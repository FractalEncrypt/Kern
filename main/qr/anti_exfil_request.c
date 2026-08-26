#include "anti_exfil_request.h"

#include "utils/secure_mem.h"

#include <stdlib.h>
#include <string.h>

struct anti_exfil_request {
  uint8_t *canonical_cbor;
  size_t canonical_cbor_len;
  anti_exfil_aext_view_t view;
};

anti_exfil_result_t
anti_exfil_request_create(const ur_result_t *result,
                          anti_exfil_request_t **request_out) {
  if (request_out)
    *request_out = NULL;
  if (!request_out || !result || !result->cbor_data || result->cbor_len == 0)
    return ANTI_EXFIL_INVALID_MESSAGE;
  if (!result->type || strcmp(result->type, ANTI_EXFIL_AEXT_UR_TYPE) != 0)
    return ANTI_EXFIL_INVALID_MESSAGE;
  if (result->cbor_len > ANTI_EXFIL_UR_MAX_CBOR_LEN)
    return ANTI_EXFIL_SIZE_LIMIT;

  anti_exfil_request_t *request = calloc(1, sizeof(*request));
  if (!request)
    return ANTI_EXFIL_NATIVE_BACKEND;

  request->canonical_cbor = malloc(result->cbor_len);
  if (!request->canonical_cbor) {
    anti_exfil_request_destroy(&request);
    return ANTI_EXFIL_NATIVE_BACKEND;
  }
  memcpy(request->canonical_cbor, result->cbor_data, result->cbor_len);
  request->canonical_cbor_len = result->cbor_len;

  const ur_result_t owned_result = {
      .type = result->type,
      .cbor_data = request->canonical_cbor,
      .cbor_len = request->canonical_cbor_len,
  };
  const anti_exfil_result_t decoded =
      anti_exfil_ur_probe_result(&owned_result, &request->view);
  if (decoded != ANTI_EXFIL_OK) {
    anti_exfil_request_destroy(&request);
    return decoded;
  }

  *request_out = request;
  return ANTI_EXFIL_OK;
}

const anti_exfil_aext_view_t *
anti_exfil_request_view(const anti_exfil_request_t *request) {
  return request ? &request->view : NULL;
}

const uint8_t *anti_exfil_request_cbor(const anti_exfil_request_t *request,
                                       size_t *cbor_len_out) {
  if (cbor_len_out)
    *cbor_len_out = request ? request->canonical_cbor_len : 0;
  return request ? request->canonical_cbor : NULL;
}

size_t anti_exfil_request_retained_bytes(const anti_exfil_request_t *request) {
  return request ? sizeof(*request) + request->canonical_cbor_len : 0;
}

void anti_exfil_request_destroy(anti_exfil_request_t **request_ptr) {
  if (!request_ptr || !*request_ptr)
    return;
  anti_exfil_request_t *request = *request_ptr;
  if (request->canonical_cbor) {
    secure_memzero(request->canonical_cbor, request->canonical_cbor_len);
    free(request->canonical_cbor);
  }
  secure_memzero(request, sizeof(*request));
  free(request);
  *request_ptr = NULL;
}
