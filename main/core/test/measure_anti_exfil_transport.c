#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anti_exfil_measurement_vectors.generated.h"
#include "core/anti_exfil/anti_exfil_slots.h"
#include "qr/anti_exfil_ur.h"

static anti_exfil_aext_view_t scratch;

static int emit_parts(size_t fragment_len) {
  for (size_t i = 0;
       i < sizeof(ANTI_EXFIL_MEASUREMENT_FIXTURES) /
               sizeof(ANTI_EXFIL_MEASUREMENT_FIXTURES[0]);
       ++i) {
    const anti_exfil_measurement_fixture_t *fixture =
        &ANTI_EXFIL_MEASUREMENT_FIXTURES[i];
    ur_encoder_t *encoder = NULL;
    anti_exfil_result_t result = anti_exfil_ur_encoder_create(
        fixture->cbor, fixture->cbor_len, ANTI_EXFIL_NETWORK_TESTNET4,
        (anti_exfil_stage_t)fixture->stage, fragment_len, &scratch, &encoder);
    if (result != ANTI_EXFIL_OK || !encoder)
      return 1;
    const size_t parts = ur_encoder_seq_len(encoder);
    for (size_t part = 0; part < parts; ++part) {
      char *encoded = NULL;
      if (!ur_encoder_next_part(encoder, &encoded) || !encoded) {
        ur_encoder_free(encoder);
        return 1;
      }
      printf("stage=%u part=%zu/%zu %s\n", fixture->stage, part + 1, parts,
             encoded);
      free(encoded);
    }
    ur_encoder_free(encoder);
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc == 3 && strcmp(argv[1], "--emit") == 0) {
    char *end = NULL;
    unsigned long requested = strtoul(argv[2], &end, 10);
    if (!end || *end != '\0' || requested < ANTI_EXFIL_UR_MIN_FRAGMENT_LEN ||
        requested > 4096) {
      fprintf(stderr, "invalid fragment length\n");
      return 2;
    }
    return emit_parts((size_t)requested);
  }
  if (argc != 1) {
    fprintf(stderr, "usage: %s [--emit FRAGMENT_BYTES]\n", argv[0]);
    return 2;
  }

  static const size_t fragment_lengths[] = {30, 50, 100, 150, 200, 250};
  printf("# sizeof_anti_exfil_message=%zu\n", sizeof(anti_exfil_message_t));
  printf("# sizeof_anti_exfil_slot_set=%zu\n",
         sizeof(anti_exfil_slot_set_t));
  printf("# signer_work_records=%zu\n",
         sizeof(anti_exfil_message_t) + sizeof(anti_exfil_slot_set_t));
  printf("# sizeof_anti_exfil_aext_view=%zu\n", sizeof(anti_exfil_aext_view_t));
  puts("stage,cbor_bytes,max_fragment_bytes,source_parts,max_ur_chars");
  for (size_t i = 0;
       i < sizeof(ANTI_EXFIL_MEASUREMENT_FIXTURES) /
               sizeof(ANTI_EXFIL_MEASUREMENT_FIXTURES[0]);
       ++i) {
    const anti_exfil_measurement_fixture_t *fixture =
        &ANTI_EXFIL_MEASUREMENT_FIXTURES[i];
    for (size_t j = 0;
         j < sizeof(fragment_lengths) / sizeof(fragment_lengths[0]); ++j) {
      ur_encoder_t *encoder = NULL;
      anti_exfil_result_t result = anti_exfil_ur_encoder_create(
          fixture->cbor, fixture->cbor_len, ANTI_EXFIL_NETWORK_TESTNET4,
          (anti_exfil_stage_t)fixture->stage, fragment_lengths[j], &scratch,
          &encoder);
      if (result != ANTI_EXFIL_OK || !encoder) {
        fprintf(stderr, "stage %u fragment %zu: encoder failed: %s\n",
                fixture->stage, fragment_lengths[j],
                anti_exfil_result_name(result));
        return 1;
      }
      size_t parts = ur_encoder_seq_len(encoder);
      size_t max_chars = 0;
      for (size_t part = 0; part < parts; ++part) {
        char *encoded = NULL;
        if (!ur_encoder_next_part(encoder, &encoded) || !encoded) {
          ur_encoder_free(encoder);
          return 1;
        }
        size_t chars = strlen(encoded);
        if (chars > max_chars)
          max_chars = chars;
        free(encoded);
      }
      printf("%u,%zu,%zu,%zu,%zu\n", fixture->stage, fixture->cbor_len,
             fragment_lengths[j], parts, max_chars);
      ur_encoder_free(encoder);
    }
  }
  return 0;
}
