#include "anti_exfil_semantic.h"

#include <stdbool.h>
#include <string.h>
#include <wally_anti_exfil.h>
#include <wally_core.h>
#include <wally_crypto.h>

static const uint8_t SECP256K1_ORDER[32] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xfe, 0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48,
    0xa0, 0x3b, 0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41,
};

static const uint8_t SECP256K1_HALF_ORDER[32] = {
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x5d, 0x57, 0x6e, 0x73, 0x57, 0xa4,
    0x50, 0x1d, 0xdf, 0xe9, 0x2f, 0x46, 0x68, 0x1b, 0x20, 0xa0,
};

static bool is_zero(const uint8_t *bytes, size_t len) {
  uint8_t aggregate = 0;
  for (size_t i = 0; i < len; ++i)
    aggregate |= bytes[i];
  return aggregate == 0;
}

static bool valid_signature(const uint8_t *signature) {
  const uint8_t *r = signature;
  const uint8_t *s = signature + 32;
  return !is_zero(r, 32) && memcmp(r, SECP256K1_ORDER, 32) < 0 &&
         !is_zero(s, 32) && memcmp(s, SECP256K1_HALF_ORDER, 32) <= 0;
}

static int compare_slot_id(const anti_exfil_slot_t *left,
                           const anti_exfil_slot_t *right) {
  if (left->input_index < right->input_index)
    return -1;
  if (left->input_index > right->input_index)
    return 1;
  return memcmp(left->signer_pubkey, right->signer_pubkey,
                ANTI_EXFIL_PUBKEY_LEN);
}

static uint8_t fields_for_stage(anti_exfil_stage_t stage) {
  switch (stage) {
  case ANTI_EXFIL_STAGE_HOST_COMMIT:
    return 0;
  case ANTI_EXFIL_STAGE_SIGNER_OPENINGS:
    return ANTI_EXFIL_FIELD_OPENING;
  case ANTI_EXFIL_STAGE_HOST_REVEAL:
    return ANTI_EXFIL_FIELD_OPENING | ANTI_EXFIL_FIELD_HOST_REVEAL;
  case ANTI_EXFIL_STAGE_SIGNER_SIGNATURES:
    return ANTI_EXFIL_FIELD_OPENING | ANTI_EXFIL_FIELD_SIGNATURE;
  default:
    return UINT8_MAX;
  }
}

anti_exfil_result_t
anti_exfil_semantic_validate(const anti_exfil_message_t *message) {
  if (!message || message->version != ANTI_EXFIL_PROTOCOL_VERSION ||
      message->network < ANTI_EXFIL_NETWORK_MAINNET ||
      message->network > ANTI_EXFIL_NETWORK_TESTNET4 || message->flags != 0 ||
      message->slot_count == 0 || message->slot_count > ANTI_EXFIL_MAX_SLOTS)
    return ANTI_EXFIL_INVALID_MESSAGE;

  const uint8_t required_fields = fields_for_stage(message->stage);
  if (required_fields == UINT8_MAX)
    return ANTI_EXFIL_WRONG_STAGE;

  size_t slots_for_input = 0;
  for (size_t i = 0; i < message->slot_count; ++i) {
    const anti_exfil_slot_t *slot = &message->slots[i];
    if (slot->sighash_type != ANTI_EXFIL_SIGHASH_ALL ||
        slot->present_fields != required_fields ||
        (slot->signer_pubkey[0] != 0x02 && slot->signer_pubkey[0] != 0x03) ||
        wally_ec_public_key_verify(slot->signer_pubkey,
                                   ANTI_EXFIL_PUBKEY_LEN) != WALLY_OK)
      return ANTI_EXFIL_INVALID_MESSAGE;

    if (i == 0 || slot->input_index != message->slots[i - 1].input_index)
      slots_for_input = 1;
    else
      ++slots_for_input;
    if (slots_for_input > ANTI_EXFIL_MAX_SLOTS_PER_INPUT)
      return ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
    if (i > 0 && compare_slot_id(&message->slots[i - 1], slot) >= 0)
      return ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;

    if ((required_fields & ANTI_EXFIL_FIELD_OPENING) != 0 &&
        wally_ec_public_key_verify(slot->opening, ANTI_EXFIL_OPENING_LEN) !=
            WALLY_OK)
      return ANTI_EXFIL_INVALID_MESSAGE;
    if ((required_fields & ANTI_EXFIL_FIELD_OPENING) == 0 &&
        !is_zero(slot->opening, ANTI_EXFIL_OPENING_LEN))
      return ANTI_EXFIL_UNEXPECTED_RETURN_DATA;
    if ((required_fields & ANTI_EXFIL_FIELD_HOST_REVEAL) == 0 &&
        !is_zero(slot->host_reveal, ANTI_EXFIL_HOST_REVEAL_LEN))
      return ANTI_EXFIL_UNEXPECTED_RETURN_DATA;
    /*
     * Structural scalar failures are malformed records (INVALID_MESSAGE), as
     * in protocol_v1_codec.py. A canonical signature that fails ECDSA/S2C
     * verification is SIGNATURE_INVALID in the Milestone 4 signer engine.
     */
    if ((required_fields & ANTI_EXFIL_FIELD_SIGNATURE) != 0 &&
        !valid_signature(slot->signature))
      return ANTI_EXFIL_INVALID_MESSAGE;
    if ((required_fields & ANTI_EXFIL_FIELD_SIGNATURE) == 0 &&
        !is_zero(slot->signature, ANTI_EXFIL_SIGNATURE_LEN))
      return ANTI_EXFIL_UNEXPECTED_RETURN_DATA;

    for (size_t j = 0; j < i; ++j) {
      const anti_exfil_slot_t *other = &message->slots[j];
      if (memcmp(slot->host_commitment, other->host_commitment,
                 ANTI_EXFIL_HOST_COMMITMENT_LEN) == 0)
        return ANTI_EXFIL_COMMITMENT_MISMATCH;
      if ((required_fields & ANTI_EXFIL_FIELD_HOST_REVEAL) != 0 &&
          memcmp(slot->host_reveal, other->host_reveal,
                 ANTI_EXFIL_HOST_REVEAL_LEN) == 0)
        return ANTI_EXFIL_COMMITMENT_MISMATCH;
      if ((required_fields & ANTI_EXFIL_FIELD_OPENING) != 0 &&
          memcmp(slot->signer_pubkey, other->signer_pubkey,
                 ANTI_EXFIL_PUBKEY_LEN) == 0 &&
          memcmp(slot->opening, other->opening, ANTI_EXFIL_OPENING_LEN) == 0)
        return ANTI_EXFIL_OPENING_MISMATCH;
    }
  }
  return ANTI_EXFIL_OK;
}

anti_exfil_result_t
anti_exfil_semantic_validate_transition(const anti_exfil_message_t *previous,
                                        const anti_exfil_message_t *next) {
  anti_exfil_result_t result = anti_exfil_semantic_validate(previous);
  if (result != ANTI_EXFIL_OK)
    return result;
  result = anti_exfil_semantic_validate(next);
  if (result != ANTI_EXFIL_OK)
    return result;

  if ((unsigned int)next->stage != (unsigned int)previous->stage + 1)
    return ANTI_EXFIL_WRONG_STAGE;
  if (next->network != previous->network)
    return ANTI_EXFIL_TRANSACTION_MISMATCH;
  if (memcmp(next->session_id, previous->session_id,
             ANTI_EXFIL_SESSION_ID_LEN) != 0)
    return ANTI_EXFIL_SESSION_MISMATCH;
  if (memcmp(next->psbt_digest, previous->psbt_digest,
             ANTI_EXFIL_PSBT_DIGEST_LEN) != 0)
    return ANTI_EXFIL_TRANSACTION_MISMATCH;
  if (next->slot_count != previous->slot_count)
    return ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;

  for (size_t i = 0; i < next->slot_count; ++i) {
    const anti_exfil_slot_t *before = &previous->slots[i];
    const anti_exfil_slot_t *after = &next->slots[i];
    if (before->input_index != after->input_index ||
        memcmp(before->signer_pubkey, after->signer_pubkey,
               ANTI_EXFIL_PUBKEY_LEN) != 0)
      return ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH;
    if (before->sighash_type != after->sighash_type ||
        memcmp(before->message_hash, after->message_hash,
               ANTI_EXFIL_MESSAGE_HASH_LEN) != 0)
      return ANTI_EXFIL_TRANSACTION_MISMATCH;
    if (memcmp(before->host_commitment, after->host_commitment,
               ANTI_EXFIL_HOST_COMMITMENT_LEN) != 0)
      return ANTI_EXFIL_COMMITMENT_MISMATCH;

    if (previous->stage >= ANTI_EXFIL_STAGE_SIGNER_OPENINGS &&
        memcmp(before->opening, after->opening, ANTI_EXFIL_OPENING_LEN) != 0)
      return ANTI_EXFIL_OPENING_MISMATCH;

    if (next->stage == ANTI_EXFIL_STAGE_HOST_REVEAL) {
      uint8_t commitment[ANTI_EXFIL_HOST_COMMITMENT_LEN];
      if (wally_ae_host_commit_from_bytes(
              after->host_reveal, ANTI_EXFIL_HOST_REVEAL_LEN, EC_FLAG_ECDSA,
              commitment, sizeof(commitment)) != WALLY_OK)
        return ANTI_EXFIL_NATIVE_BACKEND;
      if (memcmp(commitment, after->host_commitment, sizeof(commitment)) != 0)
        return ANTI_EXFIL_COMMITMENT_MISMATCH;
    }
  }
  return ANTI_EXFIL_OK;
}
