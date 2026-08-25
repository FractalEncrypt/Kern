#include "anti_exfil_signer.h"

#include "anti_exfil_semantic.h"
#include "../anti_exfil_crypto.h"
#include "../key.h"
#include <string.h>
#include <wally_bip32.h>

static anti_exfil_result_t fail(anti_exfil_message_t *output,
                                anti_exfil_slot_set_t *scratch,
                                anti_exfil_result_t result) {
  if (output && scratch &&
      (const void *)output == (const void *)scratch) {
    /* Contract violation: clear only the smaller object to avoid overrunning
     * caller storage through an output-sized memset. */
    memset(scratch, 0, sizeof(*scratch));
    return result;
  }
  if (output)
    memset(output, 0, sizeof(*output));
  if (scratch)
    memset(scratch, 0, sizeof(*scratch));
  return result;
}

static anti_exfil_result_t validate_arguments(
    const anti_exfil_message_t *input, const uint8_t *psbt_bytes,
    size_t psbt_bytes_len, anti_exfil_message_t *output,
    anti_exfil_slot_set_t *scratch) {
  if (!output || !scratch)
    return ANTI_EXFIL_INVALID_MESSAGE;
  if ((const void *)output == (const void *)scratch)
    return ANTI_EXFIL_INVALID_MESSAGE;
  memset(output, 0, sizeof(*output));
  memset(scratch, 0, sizeof(*scratch));
  if (!input || !psbt_bytes || psbt_bytes_len == 0 || input == output ||
      (const void *)input == (const void *)scratch)
    return ANTI_EXFIL_INVALID_MESSAGE;
  return ANTI_EXFIL_OK;
}

static anti_exfil_result_t validate_authoritative_slots(
    const anti_exfil_message_t *message, const uint8_t *psbt_bytes,
    size_t psbt_bytes_len, anti_exfil_slot_set_t *scratch) {
  anti_exfil_result_t result = anti_exfil_slots_enumerate(
      psbt_bytes, psbt_bytes_len, message->network, scratch);
  if (result != ANTI_EXFIL_OK)
    return result;
  if (scratch->network != message->network ||
      memcmp(scratch->psbt_digest, message->psbt_digest,
             ANTI_EXFIL_PSBT_DIGEST_LEN) != 0)
    return ANTI_EXFIL_TRANSACTION_MISMATCH;
  if (scratch->slot_count != message->slot_count)
    return ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
  for (size_t i = 0; i < scratch->slot_count; ++i) {
    const anti_exfil_signing_slot_t *derived = &scratch->slots[i];
    const anti_exfil_slot_t *declared = &message->slots[i];
    if (derived->input_index != declared->input_index ||
        memcmp(derived->signer_pubkey, declared->signer_pubkey,
               ANTI_EXFIL_PUBKEY_LEN) != 0)
      return ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
    if (derived->sighash_type != declared->sighash_type ||
        memcmp(derived->message_hash, declared->message_hash,
               ANTI_EXFIL_MESSAGE_HASH_LEN) != 0)
      return ANTI_EXFIL_TRANSACTION_MISMATCH;
  }
  return ANTI_EXFIL_OK;
}

static anti_exfil_result_t derive_slot_key(
    const anti_exfil_signing_slot_t *slot, struct ext_key **derived) {
  *derived = NULL;
  if (!key_get_derived_key_components(slot->derivation_path,
                                      slot->derivation_path_len, derived) ||
      !*derived ||
      memcmp((*derived)->pub_key, slot->signer_pubkey,
             ANTI_EXFIL_PUBKEY_LEN) != 0) {
    if (*derived)
      bip32_key_free(*derived);
    *derived = NULL;
    return ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
  }
  return ANTI_EXFIL_OK;
}

anti_exfil_result_t anti_exfil_signer_prepare(
    const anti_exfil_message_t *host_commit, const uint8_t *psbt_bytes,
    size_t psbt_bytes_len, anti_exfil_message_t *output,
    anti_exfil_slot_set_t *scratch) {
  anti_exfil_result_t result = validate_arguments(
      host_commit, psbt_bytes, psbt_bytes_len, output, scratch);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);
  result = anti_exfil_semantic_validate(host_commit);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);
  if (host_commit->stage != ANTI_EXFIL_STAGE_HOST_COMMIT)
    return fail(output, scratch, ANTI_EXFIL_WRONG_STAGE);
  result = validate_authoritative_slots(host_commit, psbt_bytes,
                                        psbt_bytes_len, scratch);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);

  memcpy(output, host_commit, sizeof(*output));
  output->stage = ANTI_EXFIL_STAGE_SIGNER_OPENINGS;
  for (size_t i = 0; i < output->slot_count; ++i) {
    struct ext_key *derived = NULL;
    result = derive_slot_key(&scratch->slots[i], &derived);
    if (result == ANTI_EXFIL_OK &&
        !anti_exfil_signer_commit(derived->priv_key + 1,
                                  output->slots[i].message_hash,
                                  output->slots[i].host_commitment,
                                  output->slots[i].opening))
      result = ANTI_EXFIL_NATIVE_BACKEND;
    if (derived)
      bip32_key_free(derived);
    if (result != ANTI_EXFIL_OK)
      return fail(output, scratch, result);
    output->slots[i].present_fields = ANTI_EXFIL_FIELD_OPENING;
  }
  result = anti_exfil_semantic_validate_transition(host_commit, output);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);
  memset(scratch, 0, sizeof(*scratch));
  return ANTI_EXFIL_OK;
}

anti_exfil_result_t anti_exfil_signer_complete(
    const anti_exfil_message_t *host_reveal, const uint8_t *psbt_bytes,
    size_t psbt_bytes_len, anti_exfil_message_t *output,
    anti_exfil_slot_set_t *scratch) {
  anti_exfil_result_t result = validate_arguments(
      host_reveal, psbt_bytes, psbt_bytes_len, output, scratch);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);
  result = anti_exfil_semantic_validate(host_reveal);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);
  if (host_reveal->stage != ANTI_EXFIL_STAGE_HOST_REVEAL)
    return fail(output, scratch, ANTI_EXFIL_WRONG_STAGE);
  result = validate_authoritative_slots(host_reveal, psbt_bytes,
                                        psbt_bytes_len, scratch);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);

  /* Reconstruct the deterministic message 2 in output; no cached state used. */
  memcpy(output, host_reveal, sizeof(*output));
  output->stage = ANTI_EXFIL_STAGE_SIGNER_OPENINGS;
  for (size_t i = 0; i < output->slot_count; ++i) {
    struct ext_key *derived = NULL;
    memset(output->slots[i].host_reveal, 0,
           sizeof(output->slots[i].host_reveal));
    output->slots[i].present_fields = ANTI_EXFIL_FIELD_OPENING;
    result = derive_slot_key(&scratch->slots[i], &derived);
    uint8_t opening[ANTI_EXFIL_OPENING_LEN];
    memset(opening, 0, sizeof(opening));
    if (result == ANTI_EXFIL_OK &&
        !anti_exfil_signer_commit(derived->priv_key + 1,
                                  output->slots[i].message_hash,
                                  output->slots[i].host_commitment, opening))
      result = ANTI_EXFIL_NATIVE_BACKEND;
    if (derived)
      bip32_key_free(derived);
    if (result == ANTI_EXFIL_OK &&
        memcmp(opening, host_reveal->slots[i].opening, sizeof(opening)) != 0)
      result = ANTI_EXFIL_OPENING_MISMATCH;
    memcpy(output->slots[i].opening, opening, sizeof(opening));
    memset(opening, 0, sizeof(opening));
    if (result != ANTI_EXFIL_OK)
      return fail(output, scratch, result);
  }
  result = anti_exfil_semantic_validate_transition(output, host_reveal);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);

  output->stage = ANTI_EXFIL_STAGE_SIGNER_SIGNATURES;
  for (size_t i = 0; i < output->slot_count; ++i) {
    struct ext_key *derived = NULL;
    const anti_exfil_slot_t *reveal_slot = &host_reveal->slots[i];
    anti_exfil_slot_t *response_slot = &output->slots[i];
    result = derive_slot_key(&scratch->slots[i], &derived);
    if (result == ANTI_EXFIL_OK &&
        !anti_exfil_sign(derived->priv_key + 1, response_slot->message_hash,
                         reveal_slot->host_reveal,
                         response_slot->signature))
      result = ANTI_EXFIL_NATIVE_BACKEND;
    if (derived)
      bip32_key_free(derived);
    if (result == ANTI_EXFIL_OK &&
        !anti_exfil_verify(response_slot->signer_pubkey,
                           response_slot->message_hash,
                           reveal_slot->host_reveal, response_slot->opening,
                           response_slot->signature))
      result = ANTI_EXFIL_SIGNATURE_INVALID;
    memset(response_slot->host_reveal, 0,
           sizeof(response_slot->host_reveal));
    response_slot->present_fields =
        ANTI_EXFIL_FIELD_OPENING | ANTI_EXFIL_FIELD_SIGNATURE;
    if (result != ANTI_EXFIL_OK)
      return fail(output, scratch, result);
  }
  result = anti_exfil_semantic_validate_transition(host_reveal, output);
  if (result != ANTI_EXFIL_OK)
    return fail(output, scratch, result);
  memset(scratch, 0, sizeof(*scratch));
  return ANTI_EXFIL_OK;
}
