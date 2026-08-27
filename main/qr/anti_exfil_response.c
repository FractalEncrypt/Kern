#include "anti_exfil_response.h"

#include "core/anti_exfil/anti_exfil_signer.h"
#include "utils/secure_mem.h"

#include <stdlib.h>
#include <string.h>

struct anti_exfil_response {
  uint8_t *canonical_cbor;
  size_t canonical_cbor_len;
  anti_exfil_network_t network;
  anti_exfil_stage_t stage;
  ur_encoder_t *encoder;
};

static void wipe_free(void *memory, size_t memory_len) {
  if (!memory)
    return;
  secure_memzero(memory, memory_len);
  free(memory);
}

anti_exfil_result_t anti_exfil_response_create(
    const anti_exfil_request_t *request, size_t max_fragment_len,
    anti_exfil_response_t **response_out) {
  if (response_out)
    *response_out = NULL;
  if (!response_out || !request ||
      max_fragment_len < ANTI_EXFIL_UR_MIN_FRAGMENT_LEN)
    return ANTI_EXFIL_INVALID_MESSAGE;

  const anti_exfil_aext_view_t *request_view =
      anti_exfil_request_view(request);
  if (!request_view)
    return ANTI_EXFIL_INVALID_MESSAGE;
  if (request_view->message.stage != ANTI_EXFIL_STAGE_HOST_COMMIT &&
      request_view->message.stage != ANTI_EXFIL_STAGE_HOST_REVEAL)
    return ANTI_EXFIL_WRONG_STAGE;
  if (!request_view->psbt || request_view->psbt_len == 0)
    return ANTI_EXFIL_INVALID_MESSAGE;

  anti_exfil_result_t result = ANTI_EXFIL_NATIVE_BACKEND;
  anti_exfil_response_t *response = calloc(1, sizeof(*response));
  anti_exfil_message_t *signed_message = calloc(1, sizeof(*signed_message));
  anti_exfil_slot_set_t *slot_scratch = calloc(1, sizeof(*slot_scratch));
  anti_exfil_aext_view_t *encoder_scratch = NULL;
  uint8_t *package = NULL;
  size_t package_len = 0;
  uint8_t *cbor = NULL;
  size_t cbor_len = 0;

  if (!response || !signed_message || !slot_scratch)
    goto cleanup;

  if (request_view->message.stage == ANTI_EXFIL_STAGE_HOST_COMMIT) {
    result = anti_exfil_signer_prepare(
        &request_view->message, request_view->psbt, request_view->psbt_len,
        signed_message, slot_scratch);
  } else {
    result = anti_exfil_signer_complete(
        &request_view->message, request_view->psbt, request_view->psbt_len,
        signed_message, slot_scratch);
  }
  if (result != ANTI_EXFIL_OK)
    goto cleanup;

  const anti_exfil_stage_t expected_stage =
      request_view->message.stage == ANTI_EXFIL_STAGE_HOST_COMMIT
          ? ANTI_EXFIL_STAGE_SIGNER_OPENINGS
          : ANTI_EXFIL_STAGE_SIGNER_SIGNATURES;
  if (signed_message->stage != expected_stage ||
      signed_message->network != request_view->message.network) {
    result = ANTI_EXFIL_INVALID_MESSAGE;
    goto cleanup;
  }

  package_len = anti_exfil_aext_encoded_len(signed_message, 0);
  cbor_len = anti_exfil_aext_cbor_encoded_len(package_len);
  if (!package_len || !cbor_len || cbor_len > ANTI_EXFIL_UR_MAX_CBOR_LEN) {
    result = ANTI_EXFIL_SIZE_LIMIT;
    goto cleanup;
  }
  package = malloc(package_len);
  cbor = malloc(cbor_len);
  if (!package || !cbor) {
    result = ANTI_EXFIL_NATIVE_BACKEND;
    goto cleanup;
  }

  size_t encoded_len = 0;
  result = anti_exfil_aext_encode(signed_message, NULL, 0, package,
                                  package_len, &encoded_len);
  if (result != ANTI_EXFIL_OK || encoded_len != package_len) {
    if (result == ANTI_EXFIL_OK)
      result = ANTI_EXFIL_INVALID_MESSAGE;
    goto cleanup;
  }
  encoded_len = 0;
  result = anti_exfil_aext_cbor_encode(package, package_len, cbor, cbor_len,
                                       &encoded_len);
  if (result != ANTI_EXFIL_OK || encoded_len != cbor_len) {
    if (result == ANTI_EXFIL_OK)
      result = ANTI_EXFIL_INVALID_MESSAGE;
    goto cleanup;
  }

  response->network = signed_message->network;
  response->stage = signed_message->stage;
  wipe_free(package, package_len);
  package = NULL;
  wipe_free(slot_scratch, sizeof(*slot_scratch));
  slot_scratch = NULL;
  wipe_free(signed_message, sizeof(*signed_message));
  signed_message = NULL;

  encoder_scratch = calloc(1, sizeof(*encoder_scratch));
  if (!encoder_scratch) {
    result = ANTI_EXFIL_NATIVE_BACKEND;
    goto cleanup;
  }
  result = anti_exfil_ur_encoder_create(
      cbor, cbor_len, response->network, response->stage,
      max_fragment_len, encoder_scratch, &response->encoder);
  if (result != ANTI_EXFIL_OK)
    goto cleanup;

  response->canonical_cbor = cbor;
  response->canonical_cbor_len = cbor_len;
  cbor = NULL;
  *response_out = response;
  response = NULL;

cleanup:
  wipe_free(package, package_len);
  wipe_free(cbor, cbor_len);
  wipe_free(encoder_scratch, sizeof(*encoder_scratch));
  wipe_free(slot_scratch, sizeof(*slot_scratch));
  wipe_free(signed_message, sizeof(*signed_message));
  if (response)
    anti_exfil_response_destroy(&response);
  return result;
}

anti_exfil_network_t
anti_exfil_response_network(const anti_exfil_response_t *response) {
  return response ? response->network : 0;
}

anti_exfil_stage_t
anti_exfil_response_stage(const anti_exfil_response_t *response) {
  return response ? response->stage : 0;
}

const uint8_t *anti_exfil_response_cbor(const anti_exfil_response_t *response,
                                        size_t *cbor_len_out) {
  if (cbor_len_out)
    *cbor_len_out = response ? response->canonical_cbor_len : 0;
  return response ? response->canonical_cbor : NULL;
}

size_t anti_exfil_response_ur_part_count(
    const anti_exfil_response_t *response) {
  return response ? ur_encoder_seq_len(response->encoder) : 0;
}

anti_exfil_result_t anti_exfil_response_next_part(
    anti_exfil_response_t *response, char **part_out) {
  if (part_out)
    *part_out = NULL;
  if (!response || !part_out || !response->encoder)
    return ANTI_EXFIL_INVALID_MESSAGE;
  return ur_encoder_next_part(response->encoder, part_out)
             ? ANTI_EXFIL_OK
             : ANTI_EXFIL_NATIVE_BACKEND;
}

void anti_exfil_response_destroy(anti_exfil_response_t **response_ptr) {
  if (!response_ptr || !*response_ptr)
    return;
  anti_exfil_response_t *response = *response_ptr;
  ur_encoder_free(response->encoder);
  wipe_free(response->canonical_cbor, response->canonical_cbor_len);
  secure_memzero(response, sizeof(*response));
  free(response);
  *response_ptr = NULL;
}
