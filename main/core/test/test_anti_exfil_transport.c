#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anti_exfil_transport_vectors.generated.h"
#include "qr/anti_exfil_aexb.h"
#include "qr/anti_exfil_aext.h"
#include "qr/anti_exfil_request.h"
#include "qr/anti_exfil_ur.h"

static anti_exfil_message_t message;
static anti_exfil_message_t decoded_again;
static anti_exfil_aext_view_t view;
static uint8_t encoded[ANTI_EXFIL_TRANSPORT_MAX_CBOR_LEN];
/* Five bytes permit a deliberately non-canonical uint32 CBOR length header. */
static uint8_t mutated[ANTI_EXFIL_TRANSPORT_MAX_CBOR_LEN + 5];
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

static int all_zero(const void *data, size_t data_len) {
  const uint8_t *bytes = data;
  for (size_t i = 0; i < data_len; ++i)
    if (bytes[i])
      return 0;
  return 1;
}

static void test_fixture(const anti_exfil_transport_fixture_t *fixture) {
  memset(&message, 0xa5, sizeof(message));
  anti_exfil_result_t result = anti_exfil_aexb_decode(
      fixture->message, fixture->message_len, &message);
  CHECK("decode canonical AEXB", result == ANTI_EXFIL_OK &&
                                     message.stage == fixture->stage);

  size_t encoded_len = 0;
  memset(encoded, 0xa5, sizeof(encoded));
  result = anti_exfil_aexb_encode(&message, encoded, sizeof(encoded),
                                  &encoded_len);
  CHECK("reproduce byte-exact AEXB",
        result == ANTI_EXFIL_OK && encoded_len == fixture->message_len &&
            memcmp(encoded, fixture->message, encoded_len) == 0);

  memset(&view, 0xa5, sizeof(view));
  result = anti_exfil_aext_decode(fixture->package, fixture->package_len, &view);
  CHECK("decode canonical AEXT",
        result == ANTI_EXFIL_OK && view.message.stage == fixture->stage &&
            memcmp(&view.message, &message, sizeof(message)) == 0 &&
            ((fixture->stage == ANTI_EXFIL_STAGE_HOST_COMMIT ||
              fixture->stage == ANTI_EXFIL_STAGE_HOST_REVEAL)
                 ? view.psbt != NULL && view.psbt_len != 0
                 : view.psbt == NULL && view.psbt_len == 0));

  encoded_len = 0;
  result = anti_exfil_aext_encode(&view.message, view.psbt, view.psbt_len,
                                  encoded, sizeof(encoded), &encoded_len);
  CHECK("reproduce byte-exact AEXT",
        result == ANTI_EXFIL_OK && encoded_len == fixture->package_len &&
            memcmp(encoded, fixture->package, encoded_len) == 0);

  const uint8_t *package = NULL;
  size_t package_len = 0;
  result = anti_exfil_aext_cbor_decode(fixture->cbor, fixture->cbor_len,
                                       &package, &package_len);
  CHECK("decode canonical CBOR byte string",
        result == ANTI_EXFIL_OK && package_len == fixture->package_len &&
            memcmp(package, fixture->package, package_len) == 0);

  encoded_len = 0;
  result = anti_exfil_aext_cbor_encode(
      fixture->package, fixture->package_len, encoded, sizeof(encoded),
      &encoded_len);
  CHECK("reproduce byte-exact canonical CBOR",
        result == ANTI_EXFIL_OK && encoded_len == fixture->cbor_len &&
            memcmp(encoded, fixture->cbor, encoded_len) == 0);

  ur_encoder_t *encoder = NULL;
  memset(&view, 0xa5, sizeof(view));
  result = anti_exfil_ur_encoder_create(
      fixture->cbor, fixture->cbor_len, ANTI_EXFIL_NETWORK_TESTNET4,
      (anti_exfil_stage_t)fixture->stage, 30, &view, &encoder);
  CHECK("create stage-restricted x-btc-anti-exfil encoder",
        result == ANTI_EXFIL_OK && encoder != NULL &&
            ur_encoder_seq_len(encoder) == fixture->ur_part_count &&
            all_zero(&view, sizeof(view)));
  int exact_parts = encoder != NULL;
  for (size_t i = 0; encoder && i < fixture->ur_part_count; ++i) {
    char *part = NULL;
    if (!ur_encoder_next_part(encoder, &part) || !part ||
        strcmp(part, fixture->ur_parts[i]) != 0)
      exact_parts = 0;
    free(part);
  }
  CHECK("reproduce complete first UR fountain window", exact_parts);
  ur_encoder_free(encoder);

  ur_decoder_t *decoder = ur_decoder_new();
  ur_decoder_state_t decoder_state = UR_DECODER_PROCESSING;
  for (size_t i = 0; decoder && i < fixture->ur_part_count; ++i)
    decoder_state = ur_decoder_receive_part(decoder, fixture->ur_parts[i]);
  ur_result_t *ur_result = decoder ? ur_decoder_get_result(decoder) : NULL;
  memset(&view, 0xa5, sizeof(view));
  result = anti_exfil_ur_decode_result(
      ur_result, ANTI_EXFIL_NETWORK_TESTNET4,
      (anti_exfil_stage_t)fixture->stage, &view);
  CHECK("decode and route complete pinned UR fountain window",
        decoder_state == UR_DECODER_OK && result == ANTI_EXFIL_OK &&
            view.message.stage == fixture->stage);

  anti_exfil_request_t *request = NULL;
  result = anti_exfil_request_create(ur_result, &request);
  size_t owned_cbor_len = 0;
  const uint8_t *owned_cbor =
      anti_exfil_request_cbor(request, &owned_cbor_len);
  const anti_exfil_aext_view_t *owned_view =
      anti_exfil_request_view(request);
  CHECK("retain byte-exact canonical CBOR outside scanner",
        result == ANTI_EXFIL_OK && request && owned_cbor &&
            owned_cbor_len == fixture->cbor_len &&
            memcmp(owned_cbor, fixture->cbor, fixture->cbor_len) == 0);
  const uintptr_t owned_start = (uintptr_t)owned_cbor;
  const uintptr_t owned_end = owned_start + owned_cbor_len;
  const uintptr_t psbt_start =
      owned_view ? (uintptr_t)owned_view->psbt : 0;
  const uintptr_t psbt_end =
      owned_view ? psbt_start + owned_view->psbt_len : 0;
  const int expects_psbt =
      fixture->stage == ANTI_EXFIL_STAGE_HOST_COMMIT ||
      fixture->stage == ANTI_EXFIL_STAGE_HOST_REVEAL;
  CHECK("owned PSBT view follows stage-neutral carriage",
        owned_view &&
            (expects_psbt ? owned_view->psbt && psbt_start >= owned_start &&
                                psbt_end <= owned_end
                          : !owned_view->psbt && owned_view->psbt_len == 0));
  ur_decoder_free(decoder);
  CHECK("owned request survives scanner decoder destruction",
        owned_cbor && owned_view &&
            owned_view->message.stage == fixture->stage &&
            memcmp(owned_cbor, fixture->cbor, fixture->cbor_len) == 0 &&
            anti_exfil_request_retained_bytes(request) >=
                owned_cbor_len + sizeof(view));
  anti_exfil_request_destroy(&request);
  CHECK("owned request destroy clears owner", request == NULL);
}

int main(void) {
  printf("=== anti-exfil AEXB/AEXT transport tests ===\n");
  for (size_t i = 0;
       i < sizeof(ANTI_EXFIL_TRANSPORT_FIXTURES) /
               sizeof(ANTI_EXFIL_TRANSPORT_FIXTURES[0]);
       ++i)
    test_fixture(&ANTI_EXFIL_TRANSPORT_FIXTURES[i]);

  const anti_exfil_transport_fixture_t *first =
      &ANTI_EXFIL_TRANSPORT_FIXTURES[0];
  memcpy(mutated, first->message, first->message_len);
  mutated[first->message_len] = 0;
  memset(&decoded_again, 0xa5, sizeof(decoded_again));
  CHECK("reject AEXB trailing data atomically",
        anti_exfil_aexb_decode(mutated, first->message_len + 1,
                               &decoded_again) == ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&decoded_again, sizeof(decoded_again)));

  memcpy(mutated, first->message, first->message_len);
  mutated[7] = 1;
  memset(&decoded_again, 0xa5, sizeof(decoded_again));
  CHECK("reject unknown AEXB flags atomically",
        anti_exfil_aexb_decode(mutated, first->message_len,
                               &decoded_again) == ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&decoded_again, sizeof(decoded_again)));

  memcpy(mutated, first->package, first->package_len);
  mutated[7] = 2;
  memset(&view, 0xa5, sizeof(view));
  CHECK("reject unknown AEXT flags atomically",
        anti_exfil_aext_decode(mutated, first->package_len, &view) ==
                ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&view, sizeof(view)));

  memcpy(mutated, first->package, first->package_len);
  mutated[16] ^= 1;
  memset(&view, 0xa5, sizeof(view));
  CHECK("reject AEXT PSBT digest mismatch atomically",
        anti_exfil_aext_decode(mutated, first->package_len, &view) ==
                ANTI_EXFIL_TRANSACTION_MISMATCH &&
            all_zero(&view, sizeof(view)));

  memcpy(mutated, first->package, first->package_len);
  mutated[6] = ANTI_EXFIL_STAGE_SIGNER_OPENINGS;
  memset(&view, 0xa5, sizeof(view));
  CHECK("reject outer and embedded stage mismatch",
        anti_exfil_aext_decode(mutated, first->package_len, &view) ==
                ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&view, sizeof(view)));

  /* Encode the 480-byte package with uint32 length instead of shortest uint16. */
  mutated[0] = 0x5a;
  mutated[1] = 0;
  mutated[2] = 0;
  mutated[3] = 1;
  mutated[4] = 0xe0;
  memcpy(mutated + 5, first->package, first->package_len);
  const uint8_t *package = (const uint8_t *)1;
  size_t package_len = 1;
  CHECK("reject non-canonical CBOR length",
        anti_exfil_aext_cbor_decode(mutated, first->package_len + 5, &package,
                                    &package_len) ==
                ANTI_EXFIL_INVALID_MESSAGE &&
            package == NULL && package_len == 0);

  memcpy(mutated, first->cbor, first->cbor_len);
  mutated[first->cbor_len] = 0;
  CHECK("reject trailing CBOR data",
        anti_exfil_aext_cbor_decode(mutated, first->cbor_len + 1, &package,
                                    &package_len) ==
            ANTI_EXFIL_INVALID_MESSAGE);

  CHECK("publish experimental UR type only",
        strcmp(ANTI_EXFIL_AEXT_UR_TYPE, "x-btc-anti-exfil") == 0);

  ur_result_t wrong_type = {
      .type = "bytes",
      .cbor_data = (uint8_t *)first->cbor,
      .cbor_len = first->cbor_len,
  };
  memset(&view, 0xa5, sizeof(view));
  CHECK("reject generic UR bytes routing",
        anti_exfil_ur_decode_result(&wrong_type, ANTI_EXFIL_NETWORK_TESTNET4,
                                    ANTI_EXFIL_STAGE_HOST_COMMIT, &view) ==
                ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&view, sizeof(view)));

  ur_result_t routed = {
      .type = ANTI_EXFIL_AEXT_UR_TYPE,
      .cbor_data = (uint8_t *)first->cbor,
      .cbor_len = first->cbor_len,
  };
  memset(&view, 0xa5, sizeof(view));
  CHECK("probe protected UR before ordinary dispatch",
        anti_exfil_ur_probe_result(&routed, &view) == ANTI_EXFIL_OK &&
            view.message.stage == ANTI_EXFIL_STAGE_HOST_COMMIT);
  memset(&view, 0xa5, sizeof(view));
  CHECK("wrong UR type cannot enter protected dispatch",
        anti_exfil_ur_probe_result(&wrong_type, &view) ==
                ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&view, sizeof(view)));

  anti_exfil_request_t *request = (anti_exfil_request_t *)(uintptr_t)1;
  anti_exfil_result_t owned_result =
      anti_exfil_request_create(&wrong_type, &request);
  CHECK("owned request rejects wrong type atomically",
        owned_result == ANTI_EXFIL_INVALID_MESSAGE && request == NULL);

  ur_result_t oversized_request = routed;
  oversized_request.cbor_len = ANTI_EXFIL_UR_MAX_CBOR_LEN + 1;
  request = (anti_exfil_request_t *)(uintptr_t)1;
  owned_result = anti_exfil_request_create(&oversized_request, &request);
  CHECK("owned request enforces size limit before allocation",
        owned_result == ANTI_EXFIL_SIZE_LIMIT && request == NULL);

  CHECK("reject wrong active network at UR route",
        anti_exfil_ur_decode_result(&routed, ANTI_EXFIL_NETWORK_SIGNET,
                                    ANTI_EXFIL_STAGE_HOST_COMMIT, &view) ==
            ANTI_EXFIL_TRANSACTION_MISMATCH);
  CHECK("reject wrong expected stage at UR route",
        anti_exfil_ur_decode_result(&routed, ANTI_EXFIL_NETWORK_TESTNET4,
                                    ANTI_EXFIL_STAGE_HOST_REVEAL, &view) ==
            ANTI_EXFIL_WRONG_STAGE);

  memset(&view, 0xa5, sizeof(view));
  ur_encoder_t *oversized_encoder = (ur_encoder_t *)1;
  CHECK("report explicit operational UR size limit",
        anti_exfil_ur_encoder_create(
            first->cbor, ANTI_EXFIL_UR_MAX_CBOR_LEN + 1,
            ANTI_EXFIL_NETWORK_TESTNET4, ANTI_EXFIL_STAGE_HOST_COMMIT, 30,
            &view, &oversized_encoder) == ANTI_EXFIL_SIZE_LIMIT &&
            oversized_encoder == NULL && all_zero(&view, sizeof(view)));

  printf("%d passed, %d failed\n", passed, failed);
  return failed ? 1 : 0;
}
