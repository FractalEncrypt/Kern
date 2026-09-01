#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anti_exfil_semantic_vectors.generated.h"
#include "core/anti_exfil/anti_exfil_slots.h"
#include "core/key.h"
#include <wally_core.h>
#include <wally_crypto.h>
#include <wally_map.h>
#include <wally_psbt.h>
#include <wally_psbt_members.h>
#include <wally_transaction.h>

static const char *TEST_MNEMONIC =
    "model ensure search plunge galaxy firm exclude brain satoshi meadow "
    "cable roast";

/* Frozen from M8-D/P01 attempt 01. This real Sparrow PSBT carries both the
 * non-witness and witness UTXO for the controlled native-SegWit input. */
static const char *LIVE_BOTH_UTXO_PSBT_BASE64 =
    "cHNidP8BAHECAAAAAdWEUo8q6BjBN8mI6jpGSGFUVCDengOob2RhCFFkL5YVAQAA"
    "AAD9////ArlsAQAAAAAAFgAUilkw2dRTpxC3xDRilnSme+xIDl+3UwEAAAAAABYA"
    "FKzBNAcJ6be6vv+SGAbwVwxlK9BYxT5OAE8BBDWHzwNXmUmVgAAAANRFa7R5gYD8"
    "4Wbha3d1QnjgfYPOBw87on6cXS32WoyqAsPFtPxB7PRTdbujUnBPUVDh9YUBtwrl"
    "4nc0OcRNGvIyEA+4gv9UAACAAQAAgAAAAIAAAQBxAgAAAAEhanmTVmopvUpzdbW4"
    "LnrhZc6rw6zHtFbumq2riPxfUwAAAAAA/f///wLfKFkbAAAAABYAFLooCAwoMx3v"
    "nZVoXQdmL3PftbDJtsACAAAAAAAWABRkIds7UvA81+JBlSlCkIRT65TTSHs6TgAB"
    "AR+2wAIAAAAAABYAFGQh2ztS8DzX4kGVKUKQhFPrlNNIAQMEAQAAACIGA5Cb4/Ep"
    "jxM3lx7WNg3ZXUVrK9osj1CHmFbgoPrTjiZwGA+4gv9UAACAAQAAgAAAAIAAAAAA"
    "AAAAAAAiAgL8QyNHjSNaENgFOvUewNI3/oVHYpVgoK1h4Hc5KpWlzBgPuIL/VAAA"
    "gAEAAIAAAACAAAAAAAEAAAAAIgICLBXv0IyCKmKH8bQrbDDsztR/HFSEw+fXLnaA"
    "owWJPScYD7iC/1QAAIABAACAAAAAgAEAAAAAAAAAAA==";

static int passed;
static int failed;

#define CHECK(name, condition)                                                 \
  do {                                                                         \
    if (condition) {                                                           \
      printf("PASS: %s\n", name);                                             \
      ++passed;                                                                \
    } else {                                                                   \
      printf("FAIL: %s\n", name);                                             \
      ++failed;                                                                \
    }                                                                          \
  } while (0)

typedef int (*mutator_t)(struct wally_psbt *psbt);

static struct wally_psbt *parse_fixture(void) {
  struct wally_psbt *psbt = NULL;
  if (wally_psbt_from_bytes(ANTI_EXFIL_SEMANTIC_PSBT,
                            ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                            WALLY_PSBT_PARSE_FLAG_STRICT, &psbt) != WALLY_OK)
    return NULL;
  return psbt;
}

static int serialize_psbt(const struct wally_psbt *psbt, uint8_t **bytes,
                          size_t *bytes_len) {
  size_t written = 0;
  *bytes = NULL;
  *bytes_len = 0;
  if (wally_psbt_get_length(psbt, 0, bytes_len) != WALLY_OK ||
      !(*bytes = malloc(*bytes_len)) ||
      wally_psbt_to_bytes(psbt, 0, *bytes, *bytes_len, &written) != WALLY_OK ||
      written != *bytes_len) {
    free(*bytes);
    *bytes = NULL;
    return 0;
  }
  return 1;
}

static int all_zero(const void *bytes, size_t bytes_len) {
  const uint8_t *p = bytes;
  for (size_t i = 0; i < bytes_len; ++i)
    if (p[i] != 0)
      return 0;
  return 1;
}

static void expect_mutation(const char *name, mutator_t mutator,
                            anti_exfil_result_t expected) {
  struct wally_psbt *psbt = parse_fixture();
  uint8_t *bytes = NULL;
  size_t bytes_len = 0;
  anti_exfil_slot_set_t out;
  int prepared = psbt && mutator(psbt) &&
                 serialize_psbt(psbt, &bytes, &bytes_len);
  memset(&out, 0xa5, sizeof(out));
  anti_exfil_result_t actual =
      prepared ? anti_exfil_slots_enumerate(bytes, bytes_len,
                                            ANTI_EXFIL_NETWORK_TESTNET4, &out)
               : ANTI_EXFIL_NATIVE_BACKEND;
  if (!prepared || actual != expected || !all_zero(&out, sizeof(out)))
    printf("  detail: prepared=%d actual=%s expected=%s cleared=%d\n",
           prepared, anti_exfil_result_name(actual),
           anti_exfil_result_name(expected), all_zero(&out, sizeof(out)));
  CHECK(name, prepared && actual == expected && all_zero(&out, sizeof(out)));
  free(bytes);
  if (psbt)
    wally_psbt_free(psbt);
}

static int unsupported_sighash(struct wally_psbt *psbt) {
  psbt->inputs[0].sighash = WALLY_SIGHASH_NONE;
  return 1;
}

static int missing_utxo(struct wally_psbt *psbt) {
  wally_tx_output_free(psbt->inputs[0].witness_utxo);
  psbt->inputs[0].witness_utxo = NULL;
  return 1;
}

static int wrong_derivation(struct wally_psbt *psbt) {
  struct wally_map_item *item = &psbt->inputs[0].keypaths.items[0];
  if (item->value_len < 4)
    return 0;
  item->value[item->value_len - 4] ^= 1;
  return 1;
}

static int broken_redeem_script(struct wally_psbt *psbt) {
  uint8_t script[34];
  size_t script_len = 0, written = 0;
  if (wally_psbt_get_input_redeem_script_len(psbt, 1, &script_len) !=
          WALLY_OK ||
      script_len > sizeof(script) ||
      wally_psbt_get_input_redeem_script(psbt, 1, script, sizeof(script),
                                         &written) != WALLY_OK ||
      written != script_len)
    return 0;
  script[2] ^= 1;
  return wally_psbt_set_input_redeem_script(psbt, 1, script, script_len) ==
         WALLY_OK;
}

static int broken_witness_script(struct wally_psbt *psbt) {
  uint8_t script[547];
  size_t script_len = 0, written = 0;
  if (wally_psbt_get_input_witness_script_len(psbt, 2, &script_len) !=
          WALLY_OK ||
      script_len > sizeof(script) ||
      wally_psbt_get_input_witness_script(psbt, 2, script, sizeof(script),
                                          &written) != WALLY_OK ||
      written != script_len)
    return 0;
  script[2] ^= 1;
  return wally_psbt_set_input_witness_script(psbt, 2, script, script_len) ==
         WALLY_OK;
}

static int taproot_metadata(struct wally_psbt *psbt) {
  return wally_psbt_set_input_taproot_internal_key(
             psbt, 0, ANTI_EXFIL_SEMANTIC_MESSAGES[0].slots[0].signer_pubkey + 1,
             32) == WALLY_OK;
}

static int mixed_legacy_input(struct wally_psbt *psbt) {
  static const uint8_t p2pkh[] = {
      0x76, 0xa9, 0x14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0,    0,    0,    0, 0, 0, 0x88, 0xac};
  struct wally_tx_output *replacement = NULL;
  const uint64_t amount = psbt->inputs[3].witness_utxo->satoshi;
  if (wally_tx_output_init_alloc(amount, p2pkh, sizeof(p2pkh),
                                 &replacement) != WALLY_OK)
    return 0;
  wally_tx_output_free(psbt->inputs[3].witness_utxo);
  psbt->inputs[3].witness_utxo = replacement;
  return 1;
}

static int finalized_selected_input(struct wally_psbt *psbt) {
  static const uint8_t final_script[] = {0x00};
  return wally_psbt_set_input_final_scriptsig(psbt, 0, final_script,
                                              sizeof(final_script)) ==
         WALLY_OK;
}

static int preexisting_signature(struct wally_psbt *psbt) {
  uint8_t der[EC_SIGNATURE_DER_MAX_LEN + 1];
  size_t der_len = 0;
  const anti_exfil_slot_t *slot = &ANTI_EXFIL_SEMANTIC_MESSAGES[3].slots[0];
  if (wally_ec_sig_to_der(slot->signature, sizeof(slot->signature), der,
                          EC_SIGNATURE_DER_MAX_LEN, &der_len) != WALLY_OK)
    return 0;
  der[der_len++] = WALLY_SIGHASH_ALL;
  return wally_psbt_add_input_signature(psbt, 0, slot->signer_pubkey,
                                        sizeof(slot->signer_pubkey), der,
                                        der_len) == WALLY_OK;
}

static void test_positive_fixture(void) {
  static const anti_exfil_script_kind_t kinds[] = {
      ANTI_EXFIL_SCRIPT_P2WPKH, ANTI_EXFIL_SCRIPT_P2SH_P2WPKH,
      ANTI_EXFIL_SCRIPT_P2WSH_MULTISIG, ANTI_EXFIL_SCRIPT_P2WSH_MULTISIG,
      ANTI_EXFIL_SCRIPT_P2SH_P2WSH_MULTISIG};
  anti_exfil_slot_set_t out;
  const anti_exfil_message_t *expected = &ANTI_EXFIL_SEMANTIC_MESSAGES[0];
  anti_exfil_result_t result = anti_exfil_slots_enumerate(
      ANTI_EXFIL_SEMANTIC_PSBT, ANTI_EXFIL_SEMANTIC_PSBT_LEN,
      ANTI_EXFIL_NETWORK_TESTNET4, &out);
  int matches = result == ANTI_EXFIL_OK && out.slot_count == 5 &&
                out.network == ANTI_EXFIL_NETWORK_TESTNET4 &&
                memcmp(out.psbt_digest, expected->psbt_digest,
                       sizeof(out.psbt_digest)) == 0;
  for (size_t i = 0; matches && i < out.slot_count; ++i) {
    const anti_exfil_signing_slot_t *actual = &out.slots[i];
    const anti_exfil_slot_t *slot = &expected->slots[i];
    const uint32_t expected_path[] = {0x80000054, 0x80000001, 0x80000000,
                                      0, (uint32_t)i};
    matches = actual->input_index == slot->input_index &&
              actual->sighash_type == slot->sighash_type &&
              actual->script_kind == kinds[i] &&
              actual->derivation_path_len == 5 &&
              memcmp(actual->derivation_path, expected_path,
                     sizeof(expected_path)) == 0 &&
              memcmp(actual->signer_pubkey, slot->signer_pubkey,
                     sizeof(actual->signer_pubkey)) == 0 &&
              memcmp(actual->message_hash, slot->message_hash,
                     sizeof(actual->message_hash)) == 0;
  }
  CHECK("canonical fixture reproduces exact slots and BIP143 hashes", matches);
}

static void test_network_identity(void) {
  const anti_exfil_network_t networks[] = {ANTI_EXFIL_NETWORK_TESTNET3,
                                           ANTI_EXFIL_NETWORK_SIGNET,
                                           ANTI_EXFIL_NETWORK_TESTNET4};
  int exact = 1;
  for (size_t i = 0; i < sizeof(networks) / sizeof(networks[0]); ++i) {
    anti_exfil_slot_set_t out;
    if (anti_exfil_slots_enumerate(ANTI_EXFIL_SEMANTIC_PSBT,
                                   ANTI_EXFIL_SEMANTIC_PSBT_LEN, networks[i],
                                   &out) != ANTI_EXFIL_OK ||
        out.network != networks[i])
      exact = 0;
  }
  CHECK("public-test networks retain exact protocol identity", exact);
}

static void test_live_both_utxo_fixture(void) {
  static const uint8_t expected_digest[ANTI_EXFIL_PSBT_DIGEST_LEN] = {
      0x4c, 0x9b, 0x55, 0xd1, 0x0d, 0x4e, 0xc0, 0x68,
      0x6a, 0x28, 0x27, 0x84, 0x31, 0x5c, 0x7f, 0xdc,
      0x44, 0xf7, 0x6d, 0x36, 0x42, 0x5a, 0xa5, 0xeb,
      0xfa, 0x00, 0x76, 0x6c, 0xc3, 0x4a, 0x6b, 0xfd};
  static const uint8_t expected_pubkey[ANTI_EXFIL_PUBKEY_LEN] = {
      0x03, 0x90, 0x9b, 0xe3, 0xf1, 0x29, 0x8f, 0x13, 0x37, 0x97, 0x1e,
      0xd6, 0x36, 0x0d, 0xd9, 0x5d, 0x45, 0x6b, 0x2b, 0xda, 0x2c, 0x8f,
      0x50, 0x87, 0x98, 0x56, 0xe0, 0xa0, 0xfa, 0xd3, 0x8e, 0x26, 0x70};
  static const uint8_t expected_hash[ANTI_EXFIL_MESSAGE_HASH_LEN] = {
      0x13, 0x47, 0xca, 0x94, 0x6a, 0xf3, 0xfe, 0x65,
      0x11, 0xc4, 0x09, 0x76, 0x62, 0x53, 0x8d, 0x9f,
      0x4e, 0x31, 0x9d, 0xa0, 0xea, 0xb6, 0x4c, 0x66,
      0x3d, 0x5e, 0x1c, 0x2e, 0x0c, 0x30, 0x03, 0xec};
  static const uint32_t expected_path[] = {
      0x80000054, 0x80000001, 0x80000000, 0, 0};
  uint8_t psbt[1024];
  size_t psbt_len = 0;
  anti_exfil_slot_set_t out;
  int decoded =
      wally_base64_to_bytes(LIVE_BOTH_UTXO_PSBT_BASE64, 0, psbt,
                            sizeof(psbt), &psbt_len) == WALLY_OK &&
      psbt_len == 559;
  anti_exfil_result_t result =
      decoded ? anti_exfil_slots_enumerate(psbt, psbt_len,
                                           ANTI_EXFIL_NETWORK_TESTNET3, &out)
              : ANTI_EXFIL_NATIVE_BACKEND;
  int matches = decoded && result == ANTI_EXFIL_OK && out.slot_count == 1 &&
                out.network == ANTI_EXFIL_NETWORK_TESTNET3 &&
                memcmp(out.psbt_digest, expected_digest,
                       sizeof(expected_digest)) == 0 &&
                out.slots[0].input_index == 0 &&
                out.slots[0].sighash_type == ANTI_EXFIL_SIGHASH_ALL &&
                out.slots[0].derivation_path_len == 5 &&
                memcmp(out.slots[0].derivation_path, expected_path,
                       sizeof(expected_path)) == 0 &&
                memcmp(out.slots[0].signer_pubkey, expected_pubkey,
                       sizeof(expected_pubkey)) == 0 &&
                memcmp(out.slots[0].message_hash, expected_hash,
                       sizeof(expected_hash)) == 0;
  if (!matches)
    printf("  detail: decoded=%d length=%zu result=%s slots=%zu\n", decoded,
           psbt_len, anti_exfil_result_name(result),
           result == ANTI_EXFIL_OK ? out.slot_count : 0);
  CHECK("accept live P2WPKH with agreeing witness and non-witness UTXOs",
        matches);
  memset(psbt, 0, sizeof(psbt));
  memset(&out, 0, sizeof(out));
}

int main(void) {
  printf("=== anti-exfil authoritative slot tests ===\n");
  CHECK("initialize key state", key_init());
  CHECK("load pinned fixture seed",
        key_load_from_mnemonic(TEST_MNEMONIC, "", true));
  test_positive_fixture();
  test_network_identity();
  test_live_both_utxo_fixture();

  expect_mutation("reject unsupported sighash atomically", unsupported_sighash,
                  ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  expect_mutation("reject missing UTXO atomically", missing_utxo,
                  ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  expect_mutation("reject wrong derivation atomically", wrong_derivation,
                  ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  expect_mutation("reject redeem-script mismatch atomically",
                  broken_redeem_script, ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  expect_mutation("reject witness-script mismatch atomically",
                  broken_witness_script, ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  expect_mutation("reject Taproot metadata atomically", taproot_metadata,
                  ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  expect_mutation("reject mixed legacy input atomically", mixed_legacy_input,
                  ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  expect_mutation("reject selected-key signature atomically",
                  preexisting_signature, ANTI_EXFIL_UNEXPECTED_RETURN_DATA);
  expect_mutation("reject finalized selected input atomically",
                  finalized_selected_input,
                  ANTI_EXFIL_UNEXPECTED_RETURN_DATA);

  uint8_t trailing[ANTI_EXFIL_SEMANTIC_PSBT_LEN + 1];
  anti_exfil_slot_set_t out;
  memcpy(trailing, ANTI_EXFIL_SEMANTIC_PSBT, ANTI_EXFIL_SEMANTIC_PSBT_LEN);
  trailing[sizeof(trailing) - 1] = 0;
  memset(&out, 0xa5, sizeof(out));
  CHECK("reject trailing PSBT bytes atomically",
        anti_exfil_slots_enumerate(trailing, sizeof(trailing),
                                   ANTI_EXFIL_NETWORK_TESTNET4, &out) ==
                ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&out, sizeof(out)));

  key_unload();
  memset(&out, 0xa5, sizeof(out));
  CHECK("reject enumeration without a loaded key",
        anti_exfil_slots_enumerate(ANTI_EXFIL_SEMANTIC_PSBT,
                                   ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                                   ANTI_EXFIL_NETWORK_TESTNET4, &out) ==
                ANTI_EXFIL_STATE_INVALID &&
            all_zero(&out, sizeof(out)));
  printf("%d passed, %d failed\n", passed, failed);
  return failed ? 1 : 0;
}
