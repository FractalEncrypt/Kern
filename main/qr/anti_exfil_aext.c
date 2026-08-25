#include "anti_exfil_aext.h"

#include <limits.h>
#include <string.h>
#include <wally_core.h>
#include <wally_crypto.h>

static const uint8_t AEXT_MAGIC[4] = {'A', 'E', 'X', 'T'};
static const uint8_t PSBT_MAGIC[5] = {'p', 's', 'b', 't', 0xff};

static void put_u32(uint8_t *out, uint32_t value) {
  out[0] = (uint8_t)(value >> 24);
  out[1] = (uint8_t)(value >> 16);
  out[2] = (uint8_t)(value >> 8);
  out[3] = (uint8_t)value;
}

static uint32_t get_u32(const uint8_t *in) {
  return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) |
         ((uint32_t)in[2] << 8) | in[3];
}

static int stage_has_psbt(anti_exfil_stage_t stage) {
  return stage == ANTI_EXFIL_STAGE_HOST_COMMIT ||
         stage == ANTI_EXFIL_STAGE_HOST_REVEAL;
}

static int all_zero(const uint8_t *bytes, size_t bytes_len) {
  uint8_t aggregate = 0;
  for (size_t i = 0; i < bytes_len; ++i)
    aggregate |= bytes[i];
  return aggregate == 0;
}

size_t anti_exfil_aext_encoded_len(const anti_exfil_message_t *message,
                                   size_t psbt_len) {
  const size_t message_len = anti_exfil_aexb_encoded_len(message);
  if (!message_len || psbt_len > ANTI_EXFIL_AEXT_MAX_PSBT_LEN ||
      stage_has_psbt(message->stage) != (psbt_len != 0))
    return 0;
  return ANTI_EXFIL_AEXT_HEADER_LEN + message_len + psbt_len;
}

anti_exfil_result_t anti_exfil_aext_encode(
    const anti_exfil_message_t *message, const uint8_t *psbt, size_t psbt_len,
    uint8_t *output, size_t output_capacity, size_t *output_len) {
  if (output_len)
    *output_len = 0;
  if (!message || !output || !output_len ||
      (psbt_len != 0 && !psbt) || (psbt_len == 0 && psbt))
    return ANTI_EXFIL_INVALID_MESSAGE;
  const size_t package_len = anti_exfil_aext_encoded_len(message, psbt_len);
  if (!package_len || output_capacity < package_len ||
      (psbt_len &&
       (psbt_len < sizeof(PSBT_MAGIC) ||
        memcmp(psbt, PSBT_MAGIC, sizeof(PSBT_MAGIC)) != 0)))
    return ANTI_EXFIL_INVALID_MESSAGE;

  uint8_t digest[ANTI_EXFIL_PSBT_DIGEST_LEN] = {0};
  if (psbt_len &&
      (wally_sha256(psbt, psbt_len, digest, sizeof(digest)) != WALLY_OK ||
       memcmp(digest, message->psbt_digest, sizeof(digest)) != 0)) {
    wally_bzero(digest, sizeof(digest));
    return ANTI_EXFIL_TRANSACTION_MISMATCH;
  }

  memcpy(output, AEXT_MAGIC, sizeof(AEXT_MAGIC));
  output[4] = ANTI_EXFIL_PROTOCOL_VERSION;
  output[5] = (uint8_t)message->network;
  output[6] = (uint8_t)message->stage;
  output[7] = psbt_len ? 1 : 0;
  const size_t message_len = anti_exfil_aexb_encoded_len(message);
  put_u32(output + 8, (uint32_t)message_len);
  put_u32(output + 12, (uint32_t)psbt_len);
  memcpy(output + 16, digest, sizeof(digest));
  wally_bzero(digest, sizeof(digest));
  size_t actual_message_len = 0;
  anti_exfil_result_t result = anti_exfil_aexb_encode(
      message, output + ANTI_EXFIL_AEXT_HEADER_LEN,
      output_capacity - ANTI_EXFIL_AEXT_HEADER_LEN, &actual_message_len);
  if (result != ANTI_EXFIL_OK || actual_message_len != message_len) {
    memset(output, 0, package_len);
    return result == ANTI_EXFIL_OK ? ANTI_EXFIL_INVALID_MESSAGE : result;
  }
  if (psbt_len)
    memcpy(output + ANTI_EXFIL_AEXT_HEADER_LEN + message_len, psbt, psbt_len);
  *output_len = package_len;
  return ANTI_EXFIL_OK;
}

anti_exfil_result_t anti_exfil_aext_decode(const uint8_t *encoded,
                                           size_t encoded_len,
                                           anti_exfil_aext_view_t *view) {
  if (!view)
    return ANTI_EXFIL_INVALID_MESSAGE;
  memset(view, 0, sizeof(*view));
  if (encoded_len > ANTI_EXFIL_AEXT_MAX_PACKAGE_LEN)
    return ANTI_EXFIL_SIZE_LIMIT;
  if (!encoded || encoded_len < ANTI_EXFIL_AEXT_HEADER_LEN ||
      memcmp(encoded, AEXT_MAGIC, sizeof(AEXT_MAGIC)) != 0 ||
      encoded[4] != ANTI_EXFIL_PROTOCOL_VERSION || encoded[7] > 1)
    return ANTI_EXFIL_INVALID_MESSAGE;

  const uint32_t message_len = get_u32(encoded + 8);
  const uint32_t psbt_len = get_u32(encoded + 12);
  if (message_len < ANTI_EXFIL_AEXB_HEADER_LEN ||
      message_len > ANTI_EXFIL_AEXB_MAX_LEN ||
      psbt_len > ANTI_EXFIL_AEXT_MAX_PSBT_LEN ||
      encoded_len != ANTI_EXFIL_AEXT_HEADER_LEN + (size_t)message_len +
                         (size_t)psbt_len ||
      (encoded[7] != 0) != (psbt_len != 0))
    return ANTI_EXFIL_INVALID_MESSAGE;

  anti_exfil_result_t result = anti_exfil_aexb_decode(
      encoded + ANTI_EXFIL_AEXT_HEADER_LEN, message_len, &view->message);
  if (result != ANTI_EXFIL_OK)
    return result;
  if ((uint8_t)view->message.network != encoded[5] ||
      (uint8_t)view->message.stage != encoded[6] ||
      stage_has_psbt(view->message.stage) != (psbt_len != 0)) {
    memset(view, 0, sizeof(*view));
    return ANTI_EXFIL_INVALID_MESSAGE;
  }

  const uint8_t *psbt =
      encoded + ANTI_EXFIL_AEXT_HEADER_LEN + (size_t)message_len;
  if (!psbt_len) {
    if (!all_zero(encoded + 16, ANTI_EXFIL_PSBT_DIGEST_LEN)) {
      memset(view, 0, sizeof(*view));
      return ANTI_EXFIL_TRANSACTION_MISMATCH;
    }
  } else {
    uint8_t digest[ANTI_EXFIL_PSBT_DIGEST_LEN];
    if (psbt_len < sizeof(PSBT_MAGIC) ||
        memcmp(psbt, PSBT_MAGIC, sizeof(PSBT_MAGIC)) != 0 ||
        wally_sha256(psbt, psbt_len, digest, sizeof(digest)) != WALLY_OK ||
        memcmp(digest, encoded + 16, sizeof(digest)) != 0 ||
        memcmp(digest, view->message.psbt_digest, sizeof(digest)) != 0) {
      wally_bzero(digest, sizeof(digest));
      memset(view, 0, sizeof(*view));
      return ANTI_EXFIL_TRANSACTION_MISMATCH;
    }
    wally_bzero(digest, sizeof(digest));
    view->psbt = psbt;
    view->psbt_len = psbt_len;
  }
  return ANTI_EXFIL_OK;
}

static size_t cbor_header_len(size_t package_len) {
  if (package_len < 24)
    return 1;
  if (package_len <= UINT8_MAX)
    return 2;
  if (package_len <= UINT16_MAX)
    return 3;
  if (package_len <= UINT32_MAX)
    return 5;
  return 0;
}

size_t anti_exfil_aext_cbor_encoded_len(size_t package_len) {
  const size_t header_len = cbor_header_len(package_len);
  return header_len && package_len <= ANTI_EXFIL_AEXT_MAX_PACKAGE_LEN
             ? header_len + package_len
             : 0;
}

anti_exfil_result_t anti_exfil_aext_cbor_encode(
    const uint8_t *package, size_t package_len, uint8_t *output,
    size_t output_capacity, size_t *output_len) {
  if (output_len)
    *output_len = 0;
  const size_t encoded_len = anti_exfil_aext_cbor_encoded_len(package_len);
  if (!package || !output || !output_len || !encoded_len ||
      output_capacity < encoded_len)
    return ANTI_EXFIL_INVALID_MESSAGE;
  const size_t header_len = encoded_len - package_len;
  if (header_len == 1) {
    output[0] = (uint8_t)(0x40u | package_len);
  } else if (header_len == 2) {
    output[0] = 0x58;
    output[1] = (uint8_t)package_len;
  } else if (header_len == 3) {
    output[0] = 0x59;
    output[1] = (uint8_t)(package_len >> 8);
    output[2] = (uint8_t)package_len;
  } else {
    output[0] = 0x5a;
    put_u32(output + 1, (uint32_t)package_len);
  }
  memcpy(output + header_len, package, package_len);
  *output_len = encoded_len;
  return ANTI_EXFIL_OK;
}

anti_exfil_result_t anti_exfil_aext_cbor_decode(
    const uint8_t *cbor, size_t cbor_len, const uint8_t **package,
    size_t *package_len) {
  if (package)
    *package = NULL;
  if (package_len)
    *package_len = 0;
  if (!cbor || !package || !package_len || cbor_len == 0)
    return ANTI_EXFIL_INVALID_MESSAGE;

  size_t header_len = 0;
  size_t decoded_len = 0;
  const uint8_t initial = cbor[0];
  if ((initial & 0xe0u) != 0x40u)
    return ANTI_EXFIL_INVALID_MESSAGE;
  const uint8_t additional = initial & 0x1fu;
  if (additional < 24) {
    header_len = 1;
    decoded_len = additional;
  } else if (additional == 24 && cbor_len >= 2 && cbor[1] >= 24) {
    header_len = 2;
    decoded_len = cbor[1];
  } else if (additional == 25 && cbor_len >= 3) {
    header_len = 3;
    decoded_len = ((size_t)cbor[1] << 8) | cbor[2];
    if (decoded_len <= UINT8_MAX)
      return ANTI_EXFIL_INVALID_MESSAGE;
  } else if (additional == 26 && cbor_len >= 5) {
    header_len = 5;
    decoded_len = get_u32(cbor + 1);
    if (decoded_len <= UINT16_MAX)
      return ANTI_EXFIL_INVALID_MESSAGE;
  } else {
    return ANTI_EXFIL_INVALID_MESSAGE;
  }
  if (decoded_len > ANTI_EXFIL_AEXT_MAX_PACKAGE_LEN)
    return ANTI_EXFIL_SIZE_LIMIT;
  if (decoded_len > cbor_len - header_len ||
      cbor_len != header_len + decoded_len)
    return ANTI_EXFIL_INVALID_MESSAGE;
  *package = cbor + header_len;
  *package_len = decoded_len;
  return ANTI_EXFIL_OK;
}
