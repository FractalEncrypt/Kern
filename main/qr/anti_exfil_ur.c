#include "anti_exfil_ur.h"

#include <string.h>

static anti_exfil_result_t decode_route(const char *type, const uint8_t *cbor,
                                        size_t cbor_len,
                                        anti_exfil_aext_view_t *view) {
  if (!view)
    return ANTI_EXFIL_INVALID_MESSAGE;
  memset(view, 0, sizeof(*view));
  if (!type || strcmp(type, ANTI_EXFIL_AEXT_UR_TYPE) != 0)
    return ANTI_EXFIL_INVALID_MESSAGE;
  const uint8_t *package = NULL;
  size_t package_len = 0;
  anti_exfil_result_t result = anti_exfil_aext_cbor_decode(
      cbor, cbor_len, &package, &package_len);
  if (result == ANTI_EXFIL_OK)
    result = anti_exfil_aext_decode(package, package_len, view);
  if (result != ANTI_EXFIL_OK)
    memset(view, 0, sizeof(*view));
  return result;
}

static anti_exfil_result_t validate_route(
    const char *type, const uint8_t *cbor, size_t cbor_len,
    anti_exfil_network_t expected_network, anti_exfil_stage_t expected_stage,
    anti_exfil_aext_view_t *view) {
  anti_exfil_result_t result = decode_route(type, cbor, cbor_len, view);
  if (result == ANTI_EXFIL_OK && view->message.network != expected_network)
    result = ANTI_EXFIL_TRANSACTION_MISMATCH;
  if (result == ANTI_EXFIL_OK && view->message.stage != expected_stage)
    result = ANTI_EXFIL_WRONG_STAGE;
  if (result != ANTI_EXFIL_OK)
    memset(view, 0, sizeof(*view));
  return result;
}

anti_exfil_result_t anti_exfil_ur_probe_result(const ur_result_t *result,
                                               anti_exfil_aext_view_t *view) {
  if (!result) {
    if (view)
      memset(view, 0, sizeof(*view));
    return ANTI_EXFIL_INVALID_MESSAGE;
  }
  if (!view)
    return ANTI_EXFIL_INVALID_MESSAGE;
  return decode_route(result->type, result->cbor_data, result->cbor_len, view);
}

anti_exfil_result_t anti_exfil_ur_decode_result(
    const ur_result_t *result, anti_exfil_network_t expected_network,
    anti_exfil_stage_t expected_stage, anti_exfil_aext_view_t *view) {
  if (!result) {
    if (view)
      memset(view, 0, sizeof(*view));
    return ANTI_EXFIL_INVALID_MESSAGE;
  }
  return validate_route(result->type, result->cbor_data, result->cbor_len,
                        expected_network, expected_stage, view);
}

anti_exfil_result_t anti_exfil_ur_encoder_create(
    const uint8_t *canonical_cbor, size_t cbor_len,
    anti_exfil_network_t expected_network, anti_exfil_stage_t expected_stage,
    size_t max_fragment_len, anti_exfil_aext_view_t *scratch,
    ur_encoder_t **encoder) {
  if (encoder)
    *encoder = NULL;
  if (!encoder || !scratch || max_fragment_len < ANTI_EXFIL_UR_MIN_FRAGMENT_LEN) {
    if (scratch)
      memset(scratch, 0, sizeof(*scratch));
    return ANTI_EXFIL_INVALID_MESSAGE;
  }
  if (cbor_len > ANTI_EXFIL_UR_MAX_CBOR_LEN) {
    memset(scratch, 0, sizeof(*scratch));
    return ANTI_EXFIL_SIZE_LIMIT;
  }
  anti_exfil_result_t result = validate_route(
      ANTI_EXFIL_AEXT_UR_TYPE, canonical_cbor, cbor_len, expected_network,
      expected_stage, scratch);
  if (result == ANTI_EXFIL_OK) {
    *encoder = ur_encoder_new(ANTI_EXFIL_AEXT_UR_TYPE, canonical_cbor, cbor_len,
                              max_fragment_len, 0,
                              ANTI_EXFIL_UR_MIN_FRAGMENT_LEN);
    if (!*encoder)
      result = ANTI_EXFIL_NATIVE_BACKEND;
  }
  memset(scratch, 0, sizeof(*scratch));
  return result;
}
