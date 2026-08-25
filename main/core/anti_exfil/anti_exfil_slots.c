#include "anti_exfil_slots.h"

#include "../key.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wally_bip32.h>
#include <wally_core.h>
#include <wally_crypto.h>
#include <wally_map.h>
#include <wally_psbt.h>
#include <wally_psbt_members.h>
#include <wally_transaction.h>

#define ANTI_EXFIL_MAX_WITNESS_SCRIPT_LEN 547
#define PSBT_IN_FINAL_SCRIPTSIG 0x07
#define PSBT_IN_TAP_MERKLE_ROOT 0x18

typedef struct {
  anti_exfil_script_kind_t kind;
  uint8_t signing_keys[ANTI_EXFIL_MAX_SLOTS_PER_INPUT][ANTI_EXFIL_PUBKEY_LEN];
  size_t signing_key_count;
  uint8_t script_code[ANTI_EXFIL_MAX_WITNESS_SCRIPT_LEN];
  size_t script_code_len;
} input_script_t;

static anti_exfil_result_t fail(anti_exfil_slot_set_t *out,
                                anti_exfil_result_t result) {
  if (out)
    memset(out, 0, sizeof(*out));
  return result;
}

static bool hash160_matches(const uint8_t *bytes, size_t bytes_len,
                            const uint8_t expected[HASH160_LEN]) {
  uint8_t digest[HASH160_LEN];
  return wally_hash160(bytes, bytes_len, digest, sizeof(digest)) == WALLY_OK &&
         memcmp(digest, expected, sizeof(digest)) == 0;
}

static bool sha256_matches(const uint8_t *bytes, size_t bytes_len,
                           const uint8_t expected[SHA256_LEN]) {
  uint8_t digest[SHA256_LEN];
  return wally_sha256(bytes, bytes_len, digest, sizeof(digest)) == WALLY_OK &&
         memcmp(digest, expected, sizeof(digest)) == 0;
}

static bool is_p2wpkh(const uint8_t *script, size_t script_len) {
  return script_len == 22 && script[0] == 0x00 && script[1] == 0x14;
}

static bool is_p2wsh(const uint8_t *script, size_t script_len) {
  return script_len == 34 && script[0] == 0x00 && script[1] == 0x20;
}

static bool is_p2sh(const uint8_t *script, size_t script_len) {
  return script_len == 23 && script[0] == 0xa9 && script[1] == 0x14 &&
         script[22] == 0x87;
}

static bool is_p2tr(const uint8_t *script, size_t script_len) {
  return script_len == 34 && script[0] == 0x51 && script[1] == 0x20;
}

static bool get_script(const struct wally_psbt *psbt, size_t input_index,
                       bool witness, uint8_t *out, size_t out_capacity,
                       size_t *out_len) {
  int result = witness
                   ? wally_psbt_get_input_witness_script_len(psbt, input_index,
                                                             out_len)
                   : wally_psbt_get_input_redeem_script_len(psbt, input_index,
                                                            out_len);
  if (result != WALLY_OK || *out_len > out_capacity)
    return false;
  if (*out_len == 0)
    return true;
  size_t written = 0;
  result = witness ? wally_psbt_get_input_witness_script(
                         psbt, input_index, out, out_capacity, &written)
                   : wally_psbt_get_input_redeem_script(
                         psbt, input_index, out, out_capacity, &written);
  return result == WALLY_OK && written == *out_len;
}

static bool has_taproot_metadata(const struct wally_psbt *psbt,
                                 size_t input_index) {
  const struct wally_psbt_input *input = &psbt->inputs[input_index];
  size_t key_sig_len = 0, internal_key_len = 0;
  if (wally_psbt_get_input_taproot_signature_len(
          psbt, input_index, &key_sig_len) != WALLY_OK ||
      wally_psbt_get_input_taproot_internal_key_len(
          psbt, input_index, &internal_key_len) != WALLY_OK)
    return true;
  return key_sig_len != 0 || internal_key_len != 0 ||
         input->taproot_leaf_signatures.num_items != 0 ||
         input->taproot_leaf_scripts.num_items != 0 ||
         input->taproot_leaf_hashes.num_items != 0 ||
         input->taproot_leaf_paths.num_items != 0 ||
         input->musig2_pubkeys.num_items != 0 ||
         input->musig2_pubnonces.num_items != 0 ||
         input->musig2_partial_sigs.num_items != 0 ||
         wally_map_get_integer(&input->psbt_fields,
                               PSBT_IN_TAP_MERKLE_ROOT) != NULL;
}

static bool is_finalized(const struct wally_psbt_input *input) {
  return input->final_witness != NULL ||
         wally_map_get_integer(&input->psbt_fields,
                               PSBT_IN_FINAL_SCRIPTSIG) != NULL;
}

static const struct wally_tx_output *
resolve_utxo(const struct wally_psbt *psbt, size_t input_index,
             struct wally_tx_output **witness_out,
             struct wally_tx **previous_out) {
  *witness_out = NULL;
  *previous_out = NULL;
  if (wally_psbt_get_input_witness_utxo_alloc(psbt, input_index, witness_out) !=
      WALLY_OK)
    *witness_out = NULL;
  if (wally_psbt_get_input_utxo_alloc(psbt, input_index, previous_out) !=
      WALLY_OK)
    *previous_out = NULL;

  if (*previous_out) {
    uint8_t actual_txid[WALLY_TXHASH_LEN];
    const struct wally_psbt_input *input = &psbt->inputs[input_index];
    if (wally_tx_get_txid(*previous_out, actual_txid, sizeof(actual_txid)) !=
            WALLY_OK ||
        memcmp(actual_txid, input->txhash, sizeof(actual_txid)) != 0 ||
        input->index >= (*previous_out)->num_outputs)
      return NULL;
    const struct wally_tx_output *previous =
        &(*previous_out)->outputs[input->index];
    if (*witness_out &&
        (previous->satoshi != (*witness_out)->satoshi ||
         previous->script_len != (*witness_out)->script_len ||
         (previous->script_len != 0 &&
          memcmp(previous->script, (*witness_out)->script,
                 previous->script_len) != 0)))
      return NULL;
    return previous;
  }
  return *witness_out;
}

static bool parse_multisig(const uint8_t *script, size_t script_len,
                           input_script_t *result) {
  if (script_len < 37 || script_len > ANTI_EXFIL_MAX_WITNESS_SCRIPT_LEN ||
      script[script_len - 1] != 0xae || script[0] < 0x51 ||
      script[0] > 0x60 || script[script_len - 2] < 0x51 ||
      script[script_len - 2] > 0x60)
    return false;

  const size_t required = script[0] - 0x50;
  const size_t declared = script[script_len - 2] - 0x50;
  size_t cursor = 1;
  while (cursor < script_len - 2) {
    if (result->signing_key_count >= ANTI_EXFIL_MAX_SLOTS_PER_INPUT ||
        script[cursor] != ANTI_EXFIL_PUBKEY_LEN ||
        cursor + 1 + ANTI_EXFIL_PUBKEY_LEN > script_len - 2)
      return false;
    uint8_t *key = result->signing_keys[result->signing_key_count];
    memcpy(key, script + cursor + 1, ANTI_EXFIL_PUBKEY_LEN);
    if ((key[0] != 0x02 && key[0] != 0x03) ||
        wally_ec_public_key_verify(key, ANTI_EXFIL_PUBKEY_LEN) != WALLY_OK)
      return false;
    for (size_t i = 0; i < result->signing_key_count; ++i)
      if (memcmp(key, result->signing_keys[i], ANTI_EXFIL_PUBKEY_LEN) == 0)
        return false;
    ++result->signing_key_count;
    cursor += 1 + ANTI_EXFIL_PUBKEY_LEN;
  }
  if (cursor != script_len - 2 || declared != result->signing_key_count ||
      required == 0 || required > declared)
    return false;
  memcpy(result->script_code, script, script_len);
  result->script_code_len = script_len;
  return true;
}

static bool find_p2wpkh_key(const struct wally_map *keypaths,
                            const uint8_t program[HASH160_LEN],
                            input_script_t *result) {
  for (size_t i = 0; i < keypaths->num_items; ++i) {
    const struct wally_map_item *item = &keypaths->items[i];
    if (item->key_len != ANTI_EXFIL_PUBKEY_LEN ||
        !hash160_matches(item->key, item->key_len, program))
      continue;
    if (result->signing_key_count != 0)
      return false;
    memcpy(result->signing_keys[0], item->key, ANTI_EXFIL_PUBKEY_LEN);
    result->signing_key_count = 1;
  }
  if (result->signing_key_count != 1)
    return false;

  const uint8_t *key_hash = program;
  result->script_code[0] = 0x76;
  result->script_code[1] = 0xa9;
  result->script_code[2] = 0x14;
  memcpy(result->script_code + 3, key_hash, HASH160_LEN);
  result->script_code[23] = 0x88;
  result->script_code[24] = 0xac;
  result->script_code_len = 25;
  return true;
}

static bool classify_script(const struct wally_psbt *psbt, size_t input_index,
                            const struct wally_tx_output *utxo,
                            input_script_t *result) {
  uint8_t redeem[34] = {0};
  uint8_t witness[ANTI_EXFIL_MAX_WITNESS_SCRIPT_LEN] = {0};
  size_t redeem_len = 0, witness_len = 0;
  memset(result, 0, sizeof(*result));
  if (!get_script(psbt, input_index, false, redeem, sizeof(redeem),
                  &redeem_len) ||
      !get_script(psbt, input_index, true, witness, sizeof(witness),
                  &witness_len))
    return false;

  const uint8_t *outer = utxo->script;
  const size_t outer_len = utxo->script_len;
  if (is_p2tr(outer, outer_len))
    return false;
  if (is_p2wpkh(outer, outer_len) && redeem_len == 0 && witness_len == 0) {
    result->kind = ANTI_EXFIL_SCRIPT_P2WPKH;
    return find_p2wpkh_key(&psbt->inputs[input_index].keypaths, outer + 2,
                           result);
  }
  if (is_p2wsh(outer, outer_len) && redeem_len == 0 && witness_len != 0 &&
      sha256_matches(witness, witness_len, outer + 2)) {
    result->kind = ANTI_EXFIL_SCRIPT_P2WSH_MULTISIG;
    return parse_multisig(witness, witness_len, result);
  }
  if (!is_p2sh(outer, outer_len) || redeem_len == 0 ||
      !hash160_matches(redeem, redeem_len, outer + 2))
    return false;
  if (is_p2wpkh(redeem, redeem_len) && witness_len == 0) {
    result->kind = ANTI_EXFIL_SCRIPT_P2SH_P2WPKH;
    return find_p2wpkh_key(&psbt->inputs[input_index].keypaths, redeem + 2,
                           result);
  }
  if (is_p2wsh(redeem, redeem_len) && witness_len != 0 &&
      sha256_matches(witness, witness_len, redeem + 2)) {
    result->kind = ANTI_EXFIL_SCRIPT_P2SH_P2WSH_MULTISIG;
    return parse_multisig(witness, witness_len, result);
  }
  return false;
}

static bool key_is_in_script(const uint8_t key[ANTI_EXFIL_PUBKEY_LEN],
                             const input_script_t *script) {
  for (size_t i = 0; i < script->signing_key_count; ++i)
    if (memcmp(key, script->signing_keys[i], ANTI_EXFIL_PUBKEY_LEN) == 0)
      return true;
  return false;
}

static int compare_slots(const void *left, const void *right) {
  const anti_exfil_signing_slot_t *a = left;
  const anti_exfil_signing_slot_t *b = right;
  if (a->input_index < b->input_index)
    return -1;
  if (a->input_index > b->input_index)
    return 1;
  return memcmp(a->signer_pubkey, b->signer_pubkey,
                ANTI_EXFIL_PUBKEY_LEN);
}

static anti_exfil_result_t enumerate_input(
    struct wally_psbt *psbt, size_t input_index,
    const uint8_t fingerprint[BIP32_KEY_FINGERPRINT_LEN],
    anti_exfil_slot_set_t *out) {
  struct wally_tx_output *witness = NULL;
  struct wally_tx *previous = NULL;
  const struct wally_tx_output *utxo =
      resolve_utxo(psbt, input_index, &witness, &previous);
  anti_exfil_result_t result = ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
  input_script_t script;

  if (is_finalized(&psbt->inputs[input_index])) {
    result = ANTI_EXFIL_UNEXPECTED_RETURN_DATA;
    goto cleanup;
  }
  if (!utxo || has_taproot_metadata(psbt, input_index) ||
      !classify_script(psbt, input_index, utxo, &script))
    goto cleanup;

  const uint32_t declared_sighash = psbt->inputs[input_index].sighash;
  if (declared_sighash != 0 && declared_sighash != ANTI_EXFIL_SIGHASH_ALL)
    goto cleanup;

  uint8_t message_hash[ANTI_EXFIL_MESSAGE_HASH_LEN];
  if (wally_tx_get_btc_signature_hash(
          psbt->tx, input_index, script.script_code, script.script_code_len,
          utxo->satoshi, WALLY_SIGHASH_ALL, WALLY_TX_FLAG_USE_WITNESS,
          message_hash, sizeof(message_hash)) != WALLY_OK) {
    result = ANTI_EXFIL_NATIVE_BACKEND;
    goto cleanup;
  }

  const struct wally_map *keypaths = &psbt->inputs[input_index].keypaths;
  for (size_t i = 0; i < keypaths->num_items; ++i) {
    const struct wally_map_item *item = &keypaths->items[i];
    uint8_t item_fingerprint[BIP32_KEY_FINGERPRINT_LEN];
    uint32_t path[ANTI_EXFIL_MAX_DERIVATION_DEPTH];
    size_t path_len = 0;
    if (item->key_len != ANTI_EXFIL_PUBKEY_LEN ||
        wally_ec_public_key_verify(item->key, item->key_len) != WALLY_OK ||
        !key_is_in_script(item->key, &script) ||
        wally_keypath_get_fingerprint(item->value, item->value_len,
                                      item_fingerprint,
                                      sizeof(item_fingerprint)) != WALLY_OK ||
        wally_keypath_get_path_len(item->value, item->value_len, &path_len) !=
            WALLY_OK ||
        path_len > ANTI_EXFIL_MAX_DERIVATION_DEPTH ||
        wally_keypath_get_path(item->value, item->value_len, path,
                               ANTI_EXFIL_MAX_DERIVATION_DEPTH,
                               &path_len) != WALLY_OK) {
      result = ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
      goto cleanup;
    }
    if (memcmp(item_fingerprint, fingerprint, sizeof(item_fingerprint)) != 0)
      continue;

    struct ext_key *derived = NULL;
    if (!key_get_derived_key_components(path, path_len, &derived) || !derived ||
        memcmp(derived->pub_key, item->key, ANTI_EXFIL_PUBKEY_LEN) != 0) {
      if (derived)
        bip32_key_free(derived);
      result = ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
      goto cleanup;
    }
    bip32_key_free(derived);

    size_t signature_index = 0;
    if (wally_map_find(&psbt->inputs[input_index].signatures, item->key,
                       item->key_len, &signature_index) != WALLY_OK) {
      result = ANTI_EXFIL_NATIVE_BACKEND;
      goto cleanup;
    }
    if (signature_index != 0) {
      result = ANTI_EXFIL_UNEXPECTED_RETURN_DATA;
      goto cleanup;
    }
    if (out->slot_count >= ANTI_EXFIL_MAX_SLOTS) {
      result = ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
      goto cleanup;
    }
    anti_exfil_signing_slot_t *slot = &out->slots[out->slot_count++];
    slot->input_index = (uint32_t)input_index;
    memcpy(slot->signer_pubkey, item->key, ANTI_EXFIL_PUBKEY_LEN);
    memcpy(slot->message_hash, message_hash, sizeof(slot->message_hash));
    slot->sighash_type = ANTI_EXFIL_SIGHASH_ALL;
    memcpy(slot->derivation_path, path, path_len * sizeof(path[0]));
    slot->derivation_path_len = path_len;
    slot->script_kind = script.kind;
  }
  result = ANTI_EXFIL_OK;

cleanup:
  if (witness)
    wally_tx_output_free(witness);
  if (previous)
    wally_tx_free(previous);
  return result;
}

anti_exfil_result_t anti_exfil_slots_enumerate(
    const uint8_t *psbt_bytes, size_t psbt_bytes_len,
    anti_exfil_network_t network, anti_exfil_slot_set_t *out) {
  if (!out)
    return ANTI_EXFIL_INVALID_MESSAGE;
  memset(out, 0, sizeof(*out));
  if (!psbt_bytes || psbt_bytes_len < 5 ||
      network < ANTI_EXFIL_NETWORK_MAINNET ||
      network > ANTI_EXFIL_NETWORK_TESTNET4)
    return fail(out, ANTI_EXFIL_INVALID_MESSAGE);
  if (!key_is_loaded())
    return fail(out, ANTI_EXFIL_STATE_INVALID);

  struct wally_psbt *psbt = NULL;
  if (wally_psbt_from_bytes(psbt_bytes, psbt_bytes_len,
                            WALLY_PSBT_PARSE_FLAG_STRICT, &psbt) != WALLY_OK ||
      !psbt)
    return fail(out, ANTI_EXFIL_INVALID_MESSAGE);

  anti_exfil_result_t result = ANTI_EXFIL_INVALID_MESSAGE;
  size_t serialized_len = 0, written = 0;
  uint8_t *serialized = NULL;
  if (psbt->version != 0 || !psbt->tx || psbt->num_inputs == 0 ||
      psbt->num_outputs == 0 || psbt->tx->num_inputs != psbt->num_inputs ||
      psbt->tx->num_outputs != psbt->num_outputs ||
      wally_psbt_get_length(psbt, 0, &serialized_len) != WALLY_OK ||
      serialized_len != psbt_bytes_len ||
      !(serialized = malloc(serialized_len)) ||
      wally_psbt_to_bytes(psbt, 0, serialized, serialized_len, &written) !=
          WALLY_OK ||
      written != psbt_bytes_len ||
      memcmp(serialized, psbt_bytes, psbt_bytes_len) != 0)
    goto cleanup;

  out->network = network;
  if (wally_sha256(psbt_bytes, psbt_bytes_len, out->psbt_digest,
                   sizeof(out->psbt_digest)) != WALLY_OK) {
    result = ANTI_EXFIL_NATIVE_BACKEND;
    goto cleanup;
  }
  uint8_t fingerprint[BIP32_KEY_FINGERPRINT_LEN];
  if (!key_get_fingerprint(fingerprint)) {
    result = ANTI_EXFIL_STATE_INVALID;
    goto cleanup;
  }
  for (size_t i = 0; i < psbt->num_inputs; ++i) {
    result = enumerate_input(psbt, i, fingerprint, out);
    if (result != ANTI_EXFIL_OK)
      goto cleanup;
  }
  if (out->slot_count == 0) {
    result = ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
    goto cleanup;
  }
  qsort(out->slots, out->slot_count, sizeof(out->slots[0]), compare_slots);
  size_t slots_for_input = 0;
  for (size_t i = 0; i < out->slot_count; ++i) {
    if (i == 0 || out->slots[i].input_index != out->slots[i - 1].input_index)
      slots_for_input = 1;
    else
      ++slots_for_input;
    if (slots_for_input > ANTI_EXFIL_MAX_SLOTS_PER_INPUT ||
        (i > 0 && compare_slots(&out->slots[i - 1], &out->slots[i]) >= 0)) {
      result = ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
      goto cleanup;
    }
  }
  result = ANTI_EXFIL_OK;

cleanup:
  if (serialized)
    free(serialized);
  wally_psbt_free(psbt);
  if (result != ANTI_EXFIL_OK)
    return fail(out, result);
  return result;
}
