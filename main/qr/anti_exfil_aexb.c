#include "anti_exfil_aexb.h"

#include "core/anti_exfil/anti_exfil_semantic.h"
#include <string.h>

static const uint8_t AEXB_MAGIC[4] = {'A', 'E', 'X', 'B'};

static void put_u16(uint8_t *out, uint16_t value) {
  out[0] = (uint8_t)(value >> 8);
  out[1] = (uint8_t)value;
}

static void put_u32(uint8_t *out, uint32_t value) {
  out[0] = (uint8_t)(value >> 24);
  out[1] = (uint8_t)(value >> 16);
  out[2] = (uint8_t)(value >> 8);
  out[3] = (uint8_t)value;
}

static uint16_t get_u16(const uint8_t *in) {
  return (uint16_t)(((uint16_t)in[0] << 8) | in[1]);
}

static uint32_t get_u32(const uint8_t *in) {
  return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) |
         ((uint32_t)in[2] << 8) | in[3];
}

static size_t record_len(anti_exfil_stage_t stage) {
  switch (stage) {
  case ANTI_EXFIL_STAGE_HOST_COMMIT:
    return 105;
  case ANTI_EXFIL_STAGE_SIGNER_OPENINGS:
    return 138;
  case ANTI_EXFIL_STAGE_HOST_REVEAL:
    return 170;
  case ANTI_EXFIL_STAGE_SIGNER_SIGNATURES:
    return 202;
  default:
    return 0;
  }
}

size_t anti_exfil_aexb_encoded_len(const anti_exfil_message_t *message) {
  if (!message || message->slot_count > ANTI_EXFIL_MAX_SLOTS)
    return 0;
  const size_t slot_len = record_len(message->stage);
  if (!slot_len)
    return 0;
  const size_t encoded_len =
      ANTI_EXFIL_AEXB_HEADER_LEN + message->slot_count * slot_len;
  return encoded_len <= ANTI_EXFIL_AEXB_MAX_LEN ? encoded_len : 0;
}

anti_exfil_result_t anti_exfil_aexb_encode(
    const anti_exfil_message_t *message, uint8_t *output,
    size_t output_capacity, size_t *output_len) {
  if (output_len)
    *output_len = 0;
  if (!message || !output || !output_len)
    return ANTI_EXFIL_INVALID_MESSAGE;
  anti_exfil_result_t result = anti_exfil_semantic_validate(message);
  const size_t encoded_len = anti_exfil_aexb_encoded_len(message);
  if (result != ANTI_EXFIL_OK || !encoded_len || output_capacity < encoded_len)
    return result == ANTI_EXFIL_OK ? ANTI_EXFIL_INVALID_MESSAGE : result;

  const size_t slot_len = record_len(message->stage);
  memcpy(output, AEXB_MAGIC, sizeof(AEXB_MAGIC));
  output[4] = message->version;
  output[5] = (uint8_t)message->network;
  output[6] = (uint8_t)message->stage;
  output[7] = message->flags;
  put_u32(output + 8, (uint32_t)(message->slot_count * slot_len));
  memcpy(output + 12, message->session_id, ANTI_EXFIL_SESSION_ID_LEN);
  memcpy(output + 44, message->psbt_digest, ANTI_EXFIL_PSBT_DIGEST_LEN);
  put_u16(output + 76, (uint16_t)message->slot_count);

  size_t offset = ANTI_EXFIL_AEXB_HEADER_LEN;
  for (size_t i = 0; i < message->slot_count; ++i) {
    const anti_exfil_slot_t *slot = &message->slots[i];
    put_u32(output + offset, slot->input_index);
    put_u32(output + offset + 4, slot->sighash_type);
    memcpy(output + offset + 8, slot->signer_pubkey,
           ANTI_EXFIL_PUBKEY_LEN);
    memcpy(output + offset + 41, slot->message_hash,
           ANTI_EXFIL_MESSAGE_HASH_LEN);
    memcpy(output + offset + 73, slot->host_commitment,
           ANTI_EXFIL_HOST_COMMITMENT_LEN);
    if (message->stage >= ANTI_EXFIL_STAGE_SIGNER_OPENINGS)
      memcpy(output + offset + 105, slot->opening, ANTI_EXFIL_OPENING_LEN);
    if (message->stage == ANTI_EXFIL_STAGE_HOST_REVEAL)
      memcpy(output + offset + 138, slot->host_reveal,
             ANTI_EXFIL_HOST_REVEAL_LEN);
    if (message->stage == ANTI_EXFIL_STAGE_SIGNER_SIGNATURES)
      memcpy(output + offset + 138, slot->signature,
             ANTI_EXFIL_SIGNATURE_LEN);
    offset += slot_len;
  }
  *output_len = encoded_len;
  return ANTI_EXFIL_OK;
}

anti_exfil_result_t anti_exfil_aexb_decode(
    const uint8_t *encoded, size_t encoded_len, anti_exfil_message_t *message) {
  if (!message)
    return ANTI_EXFIL_INVALID_MESSAGE;
  memset(message, 0, sizeof(*message));
  if (encoded_len > ANTI_EXFIL_AEXB_MAX_LEN)
    return ANTI_EXFIL_SIZE_LIMIT;
  if (!encoded || encoded_len < ANTI_EXFIL_AEXB_HEADER_LEN ||
      memcmp(encoded, AEXB_MAGIC, sizeof(AEXB_MAGIC)) != 0)
    return ANTI_EXFIL_INVALID_MESSAGE;

  message->version = encoded[4];
  message->network = (anti_exfil_network_t)encoded[5];
  message->stage = (anti_exfil_stage_t)encoded[6];
  message->flags = encoded[7];
  const size_t slot_len = record_len(message->stage);
  const uint32_t payload_len = get_u32(encoded + 8);
  const uint16_t slot_count = get_u16(encoded + 76);
  if (!slot_len || slot_count == 0 || slot_count > ANTI_EXFIL_MAX_SLOTS ||
      payload_len != (uint32_t)slot_count * slot_len ||
      encoded_len != ANTI_EXFIL_AEXB_HEADER_LEN + payload_len) {
    const anti_exfil_result_t result =
        message->stage < ANTI_EXFIL_STAGE_HOST_COMMIT ||
                message->stage > ANTI_EXFIL_STAGE_SIGNER_SIGNATURES
            ? ANTI_EXFIL_WRONG_STAGE
            : ANTI_EXFIL_INVALID_MESSAGE;
    memset(message, 0, sizeof(*message));
    return result;
  }

  memcpy(message->session_id, encoded + 12, ANTI_EXFIL_SESSION_ID_LEN);
  memcpy(message->psbt_digest, encoded + 44, ANTI_EXFIL_PSBT_DIGEST_LEN);
  message->slot_count = slot_count;
  size_t offset = ANTI_EXFIL_AEXB_HEADER_LEN;
  for (size_t i = 0; i < message->slot_count; ++i) {
    anti_exfil_slot_t *slot = &message->slots[i];
    slot->input_index = get_u32(encoded + offset);
    slot->sighash_type = get_u32(encoded + offset + 4);
    memcpy(slot->signer_pubkey, encoded + offset + 8,
           ANTI_EXFIL_PUBKEY_LEN);
    memcpy(slot->message_hash, encoded + offset + 41,
           ANTI_EXFIL_MESSAGE_HASH_LEN);
    memcpy(slot->host_commitment, encoded + offset + 73,
           ANTI_EXFIL_HOST_COMMITMENT_LEN);
    if (message->stage >= ANTI_EXFIL_STAGE_SIGNER_OPENINGS) {
      memcpy(slot->opening, encoded + offset + 105, ANTI_EXFIL_OPENING_LEN);
      slot->present_fields |= ANTI_EXFIL_FIELD_OPENING;
    }
    if (message->stage == ANTI_EXFIL_STAGE_HOST_REVEAL) {
      memcpy(slot->host_reveal, encoded + offset + 138,
             ANTI_EXFIL_HOST_REVEAL_LEN);
      slot->present_fields |= ANTI_EXFIL_FIELD_HOST_REVEAL;
    }
    if (message->stage == ANTI_EXFIL_STAGE_SIGNER_SIGNATURES) {
      memcpy(slot->signature, encoded + offset + 138,
             ANTI_EXFIL_SIGNATURE_LEN);
      slot->present_fields |= ANTI_EXFIL_FIELD_SIGNATURE;
    }
    offset += slot_len;
  }

  anti_exfil_result_t result = anti_exfil_semantic_validate(message);
  if (result != ANTI_EXFIL_OK)
    memset(message, 0, sizeof(*message));
  return result;
}
