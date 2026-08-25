#include "anti_exfil_semantic_vectors.generated.h"
#include "core/anti_exfil/anti_exfil_semantic.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wally_core.h>
#include <wally_crypto.h>

_Static_assert(sizeof(anti_exfil_message_t) <= 32768,
               "bounded semantic record grew beyond 32 KiB");

static void assert_reason_names(void) {
  static const char *const expected[] = {
      "AE_OK",
      "AE_INVALID_MESSAGE",
      "AE_WRONG_STAGE",
      "AE_SESSION_MISMATCH",
      "AE_TRANSACTION_MISMATCH",
      "AE_SIGNATURE_SLOT_MISMATCH",
      "AE_COMMITMENT_MISMATCH",
      "AE_OPENING_MISMATCH",
      "AE_SIGNATURE_INVALID",
      "AE_RETRY_CONFLICT",
      "AE_STATE_INVALID",
      "AE_OUTPUT_EXISTS",
      "AE_TEST_KEY_MISMATCH",
      "AE_UNEXPECTED_RETURN_DATA",
      "AE_NATIVE_BACKEND",
  };
  for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i)
    assert(strcmp(anti_exfil_result_name((anti_exfil_result_t)i),
                  expected[i]) == 0);
  assert(strcmp(anti_exfil_result_name((anti_exfil_result_t)255),
                "AE_UNKNOWN") == 0);
}

static void assert_positive_corpus(void) {
  const size_t count = sizeof(ANTI_EXFIL_SEMANTIC_MESSAGES) /
                       sizeof(ANTI_EXFIL_SEMANTIC_MESSAGES[0]);
  assert(count == 4);
  for (size_t i = 0; i < count; ++i)
    assert(anti_exfil_semantic_validate(&ANTI_EXFIL_SEMANTIC_MESSAGES[i]) ==
           ANTI_EXFIL_OK);
  for (size_t i = 1; i < count; ++i)
    assert(anti_exfil_semantic_validate_transition(
               &ANTI_EXFIL_SEMANTIC_MESSAGES[i - 1],
               &ANTI_EXFIL_SEMANTIC_MESSAGES[i]) == ANTI_EXFIL_OK);
}

static void assert_negative_corpus(void) {
  const size_t count = sizeof(ANTI_EXFIL_NEGATIVE_MESSAGES) /
                       sizeof(ANTI_EXFIL_NEGATIVE_MESSAGES[0]);
  assert(count > 0);
  for (size_t i = 0; i < count; ++i)
    assert(anti_exfil_semantic_validate(
               &ANTI_EXFIL_NEGATIVE_MESSAGES[i].message) ==
           ANTI_EXFIL_NEGATIVE_MESSAGES[i].expected_result);
}

static int compare_test_slots(const void *left, const void *right) {
  const anti_exfil_slot_t *left_slot = left;
  const anti_exfil_slot_t *right_slot = right;
  return memcmp(left_slot->signer_pubkey, right_slot->signer_pubkey,
                ANTI_EXFIL_PUBKEY_LEN);
}

static void assert_per_input_limit(void) {
  anti_exfil_message_t message = {
      .version = ANTI_EXFIL_PROTOCOL_VERSION,
      .network = ANTI_EXFIL_NETWORK_TESTNET4,
      .stage = ANTI_EXFIL_STAGE_HOST_COMMIT,
      .slot_count = ANTI_EXFIL_MAX_SLOTS_PER_INPUT + 1,
  };

  for (size_t i = 0; i < message.slot_count; ++i) {
    uint8_t private_key[EC_PRIVATE_KEY_LEN] = {0};
    private_key[EC_PRIVATE_KEY_LEN - 1] = (uint8_t)(i + 1);
    message.slots[i].sighash_type = ANTI_EXFIL_SIGHASH_ALL;
    message.slots[i].host_commitment[ANTI_EXFIL_HOST_COMMITMENT_LEN - 1] =
        (uint8_t)(i + 1);
    assert(wally_ec_public_key_from_private_key(
               private_key, sizeof(private_key), message.slots[i].signer_pubkey,
               ANTI_EXFIL_PUBKEY_LEN) == WALLY_OK);
  }
  qsort(message.slots, message.slot_count, sizeof(message.slots[0]),
        compare_test_slots);
  assert(anti_exfil_semantic_validate(&message) ==
         ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);
}

static void assert_structural_rejections(void) {
  anti_exfil_message_t changed = ANTI_EXFIL_SEMANTIC_MESSAGES[0];

  assert(anti_exfil_semantic_validate(NULL) == ANTI_EXFIL_INVALID_MESSAGE);
  changed.version = 2;
  assert(anti_exfil_semantic_validate(&changed) == ANTI_EXFIL_INVALID_MESSAGE);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[0];
  changed.stage = (anti_exfil_stage_t)0;
  assert(anti_exfil_semantic_validate(&changed) == ANTI_EXFIL_WRONG_STAGE);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[0];
  changed.slot_count = ANTI_EXFIL_MAX_SLOTS + 1;
  assert(anti_exfil_semantic_validate(&changed) == ANTI_EXFIL_INVALID_MESSAGE);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[0];
  anti_exfil_slot_t temporary = changed.slots[0];
  changed.slots[0] = changed.slots[1];
  changed.slots[1] = temporary;
  assert(anti_exfil_semantic_validate(&changed) ==
         ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[0];
  memcpy(changed.slots[1].host_commitment, changed.slots[0].host_commitment,
         ANTI_EXFIL_HOST_COMMITMENT_LEN);
  assert(anti_exfil_semantic_validate(&changed) ==
         ANTI_EXFIL_COMMITMENT_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[0];
  changed.slots[0].signature[0] = 1;
  assert(anti_exfil_semantic_validate(&changed) ==
         ANTI_EXFIL_UNEXPECTED_RETURN_DATA);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];
  changed.slots[0].present_fields = 0;
  assert(anti_exfil_semantic_validate(&changed) == ANTI_EXFIL_INVALID_MESSAGE);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];
  changed.slots[0].opening[0] = 0x04;
  assert(anti_exfil_semantic_validate(&changed) == ANTI_EXFIL_INVALID_MESSAGE);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[2];
  memcpy(changed.slots[1].host_reveal, changed.slots[0].host_reveal,
         ANTI_EXFIL_HOST_REVEAL_LEN);
  assert(anti_exfil_semantic_validate(&changed) ==
         ANTI_EXFIL_COMMITMENT_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[3];
  memset(changed.slots[0].signature, 0, ANTI_EXFIL_SIGNATURE_LEN);
  assert(anti_exfil_semantic_validate(&changed) == ANTI_EXFIL_INVALID_MESSAGE);
}

static void assert_transition_rejections(void) {
  anti_exfil_message_t changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];

  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[0],
             &ANTI_EXFIL_SEMANTIC_MESSAGES[2]) == ANTI_EXFIL_WRONG_STAGE);

  changed.session_id[0] ^= 1;
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[0], &changed) ==
         ANTI_EXFIL_SESSION_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];
  changed.network = ANTI_EXFIL_NETWORK_MAINNET;
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[0], &changed) ==
         ANTI_EXFIL_TRANSACTION_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];
  changed.psbt_digest[0] ^= 1;
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[0], &changed) ==
         ANTI_EXFIL_TRANSACTION_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];
  memcpy(changed.slots[0].signer_pubkey,
         ANTI_EXFIL_SEMANTIC_MESSAGES[1].slots[1].signer_pubkey,
         ANTI_EXFIL_PUBKEY_LEN);
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[0], &changed) ==
         ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];
  changed.slots[0].message_hash[0] ^= 1;
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[0], &changed) ==
         ANTI_EXFIL_TRANSACTION_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[1];
  changed.slots[0].host_commitment[0] ^= 1;
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[0], &changed) ==
         ANTI_EXFIL_COMMITMENT_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[2];
  memcpy(changed.slots[0].opening,
         ANTI_EXFIL_SEMANTIC_MESSAGES[2].slots[1].opening,
         ANTI_EXFIL_OPENING_LEN);
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[1], &changed) ==
         ANTI_EXFIL_OPENING_MISMATCH);

  changed = ANTI_EXFIL_SEMANTIC_MESSAGES[2];
  changed.slots[0].host_reveal[0] ^= 1;
  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[1], &changed) ==
         ANTI_EXFIL_COMMITMENT_MISMATCH);

  assert(anti_exfil_semantic_validate_transition(
             &ANTI_EXFIL_SEMANTIC_MESSAGES[2],
             &ANTI_EXFIL_SEMANTIC_MESSAGES[1]) == ANTI_EXFIL_WRONG_STAGE);
}

int main(void) {
  assert(wally_init(0) == WALLY_OK);
  assert_reason_names();
  assert_positive_corpus();
  assert_negative_corpus();
  assert_per_input_limit();
  assert_structural_rejections();
  assert_transition_rejections();
  wally_cleanup(0);
  puts("anti-exfil semantic-record tests passed");
  return 0;
}
