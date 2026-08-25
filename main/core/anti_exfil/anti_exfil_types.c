#include "anti_exfil_types.h"

const char *anti_exfil_result_name(anti_exfil_result_t result) {
  static const char *const names[] = {
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

  if ((unsigned int)result >= sizeof(names) / sizeof(names[0]))
    return "AE_UNKNOWN";
  return names[result];
}
