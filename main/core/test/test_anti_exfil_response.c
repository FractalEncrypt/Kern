#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anti_exfil_measurement_vectors.generated.h"
#include "anti_exfil_semantic_vectors.generated.h"
#include "core/key.h"
#include "qr/anti_exfil_response.h"

static const char *TEST_MNEMONIC =
    "model ensure search plunge galaxy firm exclude brain satoshi meadow "
    "cable roast";
static const char *WRONG_MNEMONIC =
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon "
    "abandon abandon about";

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

static const anti_exfil_measurement_fixture_t *fixture_for_stage(uint8_t stage) {
  for (size_t i = 0;
       i < sizeof(ANTI_EXFIL_MEASUREMENT_FIXTURES) /
               sizeof(ANTI_EXFIL_MEASUREMENT_FIXTURES[0]);
       ++i) {
    if (ANTI_EXFIL_MEASUREMENT_FIXTURES[i].stage == stage)
      return &ANTI_EXFIL_MEASUREMENT_FIXTURES[i];
  }
  return NULL;
}

static anti_exfil_result_t create_request(
    uint8_t stage, anti_exfil_request_t **request_out) {
  const anti_exfil_measurement_fixture_t *fixture = fixture_for_stage(stage);
  if (!fixture)
    return ANTI_EXFIL_INVALID_MESSAGE;
  const ur_result_t result = {
      .type = ANTI_EXFIL_AEXT_UR_TYPE,
      .cbor_data = (uint8_t *)fixture->cbor,
      .cbor_len = fixture->cbor_len,
  };
  return anti_exfil_request_create(&result, request_out);
}

static void test_round(uint8_t request_stage, uint8_t response_stage,
                       size_t expected_parts) {
  const anti_exfil_measurement_fixture_t *expected =
      fixture_for_stage(response_stage);
  anti_exfil_request_t *request = NULL;
  anti_exfil_response_t *response = NULL;
  anti_exfil_result_t result = create_request(request_stage, &request);
  CHECK("create owned signer-side request", result == ANTI_EXFIL_OK && request);

  result = anti_exfil_response_create(request, 150, &response);
  size_t cbor_len = 0;
  const uint8_t *cbor = anti_exfil_response_cbor(response, &cbor_len);
  CHECK("owned request produces exact canonical response",
        result == ANTI_EXFIL_OK && response && expected && cbor &&
            cbor_len == expected->cbor_len &&
            memcmp(cbor, expected->cbor, cbor_len) == 0 &&
            anti_exfil_response_network(response) ==
                ANTI_EXFIL_NETWORK_TESTNET4 &&
            anti_exfil_response_stage(response) == response_stage);
  CHECK("response uses expected first fountain window",
        anti_exfil_response_ur_part_count(response) == expected_parts);

  ur_decoder_t *decoder = ur_decoder_new();
  ur_decoder_state_t state = UR_DECODER_PROCESSING;
  for (size_t i = 0; response && decoder && i < expected_parts; ++i) {
    char *part = NULL;
    result = anti_exfil_response_next_part(response, &part);
    if (result != ANTI_EXFIL_OK || !part) {
      state = UR_DECODER_ERROR_INVALID_PART;
      free(part);
      break;
    }
    state = ur_decoder_receive_part(decoder, part);
    free(part);
  }
  ur_result_t *decoded = decoder ? ur_decoder_get_result(decoder) : NULL;
  anti_exfil_aext_view_t decoded_view;
  memset(&decoded_view, 0xa5, sizeof(decoded_view));
  result = anti_exfil_ur_decode_result(
      decoded, ANTI_EXFIL_NETWORK_TESTNET4,
      (anti_exfil_stage_t)response_stage, &decoded_view);
  CHECK("response UR window reconstructs exact semantic message",
        state == UR_DECODER_OK && result == ANTI_EXFIL_OK &&
            memcmp(&decoded_view.message,
                   &ANTI_EXFIL_SEMANTIC_MESSAGES[response_stage - 1],
                   sizeof(decoded_view.message)) == 0 &&
            decoded_view.psbt == NULL && decoded_view.psbt_len == 0);
  ur_decoder_free(decoder);

  const anti_exfil_aext_view_t *borrowed = anti_exfil_request_view(request);
  CHECK("response creation does not consume retained request",
        borrowed && borrowed->message.stage == request_stage &&
            borrowed->psbt && borrowed->psbt_len == ANTI_EXFIL_SEMANTIC_PSBT_LEN);
  anti_exfil_response_destroy(&response);
  anti_exfil_request_destroy(&request);
  CHECK("response and request owners clear on destroy",
        response == NULL && request == NULL);
}

int main(void) {
  printf("=== anti-exfil owned response bridge tests ===\n");
  CHECK("initialize key state", key_init());
  CHECK("load pinned fixture seed",
        key_load_from_mnemonic(TEST_MNEMONIC, "", true));

  test_round(ANTI_EXFIL_STAGE_HOST_COMMIT,
             ANTI_EXFIL_STAGE_SIGNER_OPENINGS, 6);
  test_round(ANTI_EXFIL_STAGE_HOST_REVEAL,
             ANTI_EXFIL_STAGE_SIGNER_SIGNATURES, 8);

  anti_exfil_request_t *request = NULL;
  anti_exfil_response_t *response = (anti_exfil_response_t *)(uintptr_t)1;
  CHECK("create coordinator-response fixture as owned request",
        create_request(ANTI_EXFIL_STAGE_SIGNER_OPENINGS, &request) ==
                ANTI_EXFIL_OK &&
            request);
  anti_exfil_result_t result =
      anti_exfil_response_create(request, 150, &response);
  CHECK("reject message 2 as signer input atomically",
        result == ANTI_EXFIL_WRONG_STAGE && response == NULL);
  anti_exfil_request_destroy(&request);

  CHECK("create message 1 for fragment-bound rejection",
        create_request(ANTI_EXFIL_STAGE_HOST_COMMIT, &request) ==
                ANTI_EXFIL_OK &&
            request);
  response = (anti_exfil_response_t *)(uintptr_t)1;
  result = anti_exfil_response_create(
      request, ANTI_EXFIL_UR_MIN_FRAGMENT_LEN - 1, &response);
  CHECK("reject undersized response fragments atomically",
        result == ANTI_EXFIL_INVALID_MESSAGE && response == NULL);
  anti_exfil_request_destroy(&request);

  key_unload();
  CHECK("load unrelated seed",
        key_init() && key_load_from_mnemonic(WRONG_MNEMONIC, "", true));
  CHECK("create message 1 for wrong-seed rejection",
        create_request(ANTI_EXFIL_STAGE_HOST_COMMIT, &request) ==
                ANTI_EXFIL_OK &&
            request);
  response = (anti_exfil_response_t *)(uintptr_t)1;
  result = anti_exfil_response_create(request, 150, &response);
  CHECK("reject wrong seed without response",
        result == ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH && response == NULL);
  anti_exfil_request_destroy(&request);

  key_unload();
  printf("%d passed, %d failed\n", passed, failed);
  return failed ? 1 : 0;
}
