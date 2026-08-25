#include <stdio.h>
#include <string.h>

#include "anti_exfil_semantic_vectors.generated.h"
#include "core/anti_exfil/anti_exfil_signer.h"
#include "core/key.h"

static const char *TEST_MNEMONIC =
    "model ensure search plunge galaxy firm exclude brain satoshi meadow "
    "cable roast";
static const char *WRONG_MNEMONIC =
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon "
    "abandon abandon about";

static anti_exfil_message_t output;
static anti_exfil_message_t mutated;
static anti_exfil_slot_set_t scratch;
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

static int all_zero(const void *bytes, size_t bytes_len) {
  const uint8_t *p = bytes;
  for (size_t i = 0; i < bytes_len; ++i)
    if (p[i] != 0)
      return 0;
  return 1;
}

static void poison(void) {
  memset(&output, 0xa5, sizeof(output));
  memset(&scratch, 0xa5, sizeof(scratch));
}

static void expect_prepare_failure(const char *name,
                                   const anti_exfil_message_t *input,
                                   const uint8_t *psbt, size_t psbt_len,
                                   anti_exfil_result_t expected) {
  poison();
  anti_exfil_result_t result = anti_exfil_signer_prepare(
      input, psbt, psbt_len, &output, &scratch);
  CHECK(name, result == expected && all_zero(&output, sizeof(output)) &&
                  all_zero(&scratch, sizeof(scratch)));
}

static void expect_complete_failure(const char *name,
                                    const anti_exfil_message_t *input,
                                    const uint8_t *psbt, size_t psbt_len,
                                    anti_exfil_result_t expected) {
  poison();
  anti_exfil_result_t result = anti_exfil_signer_complete(
      input, psbt, psbt_len, &output, &scratch);
  CHECK(name, result == expected && all_zero(&output, sizeof(output)) &&
                  all_zero(&scratch, sizeof(scratch)));
}

int main(void) {
  printf("=== anti-exfil headless signer tests ===\n");
  CHECK("initialize key state", key_init());
  CHECK("load pinned fixture seed",
        key_load_from_mnemonic(TEST_MNEMONIC, "", true));

  poison();
  anti_exfil_result_t result = anti_exfil_signer_prepare(
      &ANTI_EXFIL_SEMANTIC_MESSAGES[0], ANTI_EXFIL_SEMANTIC_PSBT,
      ANTI_EXFIL_SEMANTIC_PSBT_LEN, &output, &scratch);
  CHECK("message 1 produces exact pinned message 2",
        result == ANTI_EXFIL_OK &&
            memcmp(&output, &ANTI_EXFIL_SEMANTIC_MESSAGES[1],
                   sizeof(output)) == 0 &&
            all_zero(&scratch, sizeof(scratch)));

  key_unload();
  CHECK("reload key after simulated process restart",
        key_init() && key_load_from_mnemonic(TEST_MNEMONIC, "", true));
  poison();
  result = anti_exfil_signer_complete(
      &ANTI_EXFIL_SEMANTIC_MESSAGES[2], ANTI_EXFIL_SEMANTIC_PSBT,
      ANTI_EXFIL_SEMANTIC_PSBT_LEN, &output, &scratch);
  CHECK("stateless message 3 produces exact pinned message 4",
        result == ANTI_EXFIL_OK &&
            memcmp(&output, &ANTI_EXFIL_SEMANTIC_MESSAGES[3],
                   sizeof(output)) == 0 &&
            all_zero(&scratch, sizeof(scratch)));

  key_unload();
  CHECK("load unrelated seed for mismatch test",
        key_init() && key_load_from_mnemonic(WRONG_MNEMONIC, "", true));
  expect_complete_failure("reject message 3 after wrong-seed reload",
                          &ANTI_EXFIL_SEMANTIC_MESSAGES[2],
                          ANTI_EXFIL_SEMANTIC_PSBT,
                          ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                          ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
  key_unload();
  CHECK("restore pinned fixture seed",
        key_init() && key_load_from_mnemonic(TEST_MNEMONIC, "", true));

  memcpy(&mutated, &ANTI_EXFIL_SEMANTIC_MESSAGES[1], sizeof(mutated));
  expect_prepare_failure("reject non-message-1 prepare", &mutated,
                         ANTI_EXFIL_SEMANTIC_PSBT,
                         ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                         ANTI_EXFIL_WRONG_STAGE);

  memcpy(&mutated, &ANTI_EXFIL_SEMANTIC_MESSAGES[0], sizeof(mutated));
  mutated.network = ANTI_EXFIL_NETWORK_SIGNET;
  poison();
  result = anti_exfil_signer_prepare(
      &mutated, ANTI_EXFIL_SEMANTIC_PSBT, ANTI_EXFIL_SEMANTIC_PSBT_LEN,
      &output, &scratch);
  CHECK("carry exact public-test network identity through signer",
        result == ANTI_EXFIL_OK &&
            output.network == ANTI_EXFIL_NETWORK_SIGNET);

  uint8_t changed_psbt[ANTI_EXFIL_SEMANTIC_PSBT_LEN];
  memcpy(changed_psbt, ANTI_EXFIL_SEMANTIC_PSBT, sizeof(changed_psbt));
  changed_psbt[10] ^= 1;
  expect_prepare_failure("reject changed frozen PSBT", 
                         &ANTI_EXFIL_SEMANTIC_MESSAGES[0], changed_psbt,
                         sizeof(changed_psbt),
                         ANTI_EXFIL_TRANSACTION_MISMATCH);

  memcpy(&mutated, &ANTI_EXFIL_SEMANTIC_MESSAGES[0], sizeof(mutated));
  mutated.slots[0].message_hash[0] ^= 1;
  expect_prepare_failure("reject coordinator-supplied sighash", &mutated,
                         ANTI_EXFIL_SEMANTIC_PSBT,
                         ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                         ANTI_EXFIL_TRANSACTION_MISMATCH);

  memcpy(&mutated, &ANTI_EXFIL_SEMANTIC_MESSAGES[2], sizeof(mutated));
  mutated.slots[0].opening[1] ^= 1;
  expect_complete_failure("reject altered accepted opening", &mutated,
                          ANTI_EXFIL_SEMANTIC_PSBT,
                          ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                          ANTI_EXFIL_OPENING_MISMATCH);

  memcpy(&mutated, &ANTI_EXFIL_SEMANTIC_MESSAGES[2], sizeof(mutated));
  mutated.slots[0].host_reveal[0] ^= 1;
  expect_complete_failure("reject reveal/commitment mismatch", &mutated,
                          ANTI_EXFIL_SEMANTIC_PSBT,
                          ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                          ANTI_EXFIL_COMMITMENT_MISMATCH);

  memcpy(&mutated, &ANTI_EXFIL_SEMANTIC_MESSAGES[2], sizeof(mutated));
  mutated.session_id[0] ^= 1;
  poison();
  result = anti_exfil_signer_complete(
      &mutated, ANTI_EXFIL_SEMANTIC_PSBT, ANTI_EXFIL_SEMANTIC_PSBT_LEN,
      &output, &scratch);
  CHECK("carry changed session unchanged in stateless completion",
        result == ANTI_EXFIL_OK &&
            memcmp(output.session_id, mutated.session_id,
                   sizeof(output.session_id)) == 0);

  expect_complete_failure("reject output/input alias",
                          (const anti_exfil_message_t *)&output,
                          ANTI_EXFIL_SEMANTIC_PSBT,
                          ANTI_EXFIL_SEMANTIC_PSBT_LEN,
                          ANTI_EXFIL_INVALID_MESSAGE);

  poison();
  result = anti_exfil_signer_prepare(
      &ANTI_EXFIL_SEMANTIC_MESSAGES[0], ANTI_EXFIL_SEMANTIC_PSBT,
      ANTI_EXFIL_SEMANTIC_PSBT_LEN, &output,
      (anti_exfil_slot_set_t *)&output);
  CHECK("reject output/scratch alias without oversized clear",
        result == ANTI_EXFIL_INVALID_MESSAGE &&
            all_zero(&output, sizeof(anti_exfil_slot_set_t)));

  key_unload();
  printf("%d passed, %d failed\n", passed, failed);
  return failed ? 1 : 0;
}
