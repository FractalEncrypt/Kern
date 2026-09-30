#include "pages/scan/anti_exfil_continuation.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned checks;
static unsigned failures;
static unsigned callback_a_calls;
static unsigned callback_b_calls;

static void check(const char *name, bool condition) {
  checks++;
  if (!condition) {
    failures++;
    fprintf(stderr, "FAIL: %s\n", name);
  }
}

static void callback_a(void) { callback_a_calls++; }
static void callback_b(void) { callback_b_calls++; }

static void test_restricted_routing_and_stateless_fallback(void) {
  anti_exfil_continuation_t state = {0};
  check("ordinary M3 uses generic stateless route while inactive",
        anti_exfil_continuation_route(&state, true, true,
                                      ANTI_EXFIL_STAGE_HOST_REVEAL) ==
            ANTI_EXFIL_SCAN_GENERIC);

  uint32_t generation = anti_exfil_continuation_begin(&state, callback_a);
  check("choice is not a scanner route",
        anti_exfil_continuation_route(&state, true, true,
                                      ANTI_EXFIL_STAGE_HOST_REVEAL) ==
            ANTI_EXFIL_SCAN_GENERIC);
  check("primary opens scanner",
        anti_exfil_continuation_open_scanner(&state, generation));
  check("explicit M3 is accepted",
        anti_exfil_continuation_route(&state, true, true,
                                      ANTI_EXFIL_STAGE_HOST_REVEAL) ==
            ANTI_EXFIL_SCAN_HOST_REVEAL);
  check("ordinary PSBT is rejected",
        anti_exfil_continuation_route(&state, true, false, 0) ==
            ANTI_EXFIL_SCAN_REJECT);
  check("BIP322/text is rejected",
        anti_exfil_continuation_route(&state, false, false, 0) ==
            ANTI_EXFIL_SCAN_REJECT);
  check("wrong protected stage is rejected",
        anti_exfil_continuation_route(&state, true, true,
                                      ANTI_EXFIL_STAGE_HOST_COMMIT) ==
            ANTI_EXFIL_SCAN_REJECT);
  check("malformed explicit input is rejected before stage dispatch",
        anti_exfil_continuation_route(&state, true, true, 0) ==
            ANTI_EXFIL_SCAN_REJECT);
}

static void test_back_exit_cleanup_and_stale_callbacks(void) {
  anti_exfil_continuation_t state = {0};
  uint32_t first = anti_exfil_continuation_begin(&state, callback_a);
  check("open first scanner",
        anti_exfil_continuation_open_scanner(&state, first));
  check("Back returns to choice",
        anti_exfil_continuation_scanner_back(&state) &&
            state.phase == ANTI_EXFIL_CONTINUATION_CHOICE);
  void (*return_cb)(void) = anti_exfil_continuation_exit(&state, first);
  check("Exit returns captured Home callback", return_cb == callback_a);
  if (return_cb)
    return_cb();
  check("Exit calls Home callback once", callback_a_calls == 1);
  check("Exit clears continuation",
        state.phase == ANTI_EXFIL_CONTINUATION_INACTIVE &&
            state.return_cb == NULL);
  check("M3 is stateless again after Exit",
        anti_exfil_continuation_route(&state, true, true,
                                      ANTI_EXFIL_STAGE_HOST_REVEAL) ==
            ANTI_EXFIL_SCAN_GENERIC);

  uint32_t second = anti_exfil_continuation_begin(&state, callback_b);
  check("repeat ceremony has a new generation", second != first);
  check("stale primary callback cannot open scanner",
        !anti_exfil_continuation_open_scanner(&state, first));
  check("stale Exit cannot consume new callback",
        anti_exfil_continuation_exit(&state, first) == NULL &&
            state.return_cb == callback_b);
  check("current primary opens scanner",
        anti_exfil_continuation_open_scanner(&state, second));
  anti_exfil_continuation_clear(&state);
  check("lock/reset/teardown cleanup clears callback and route",
        state.phase == ANTI_EXFIL_CONTINUATION_INACTIVE &&
            state.return_cb == NULL &&
            anti_exfil_continuation_route(&state, true, true,
                                          ANTI_EXFIL_STAGE_HOST_REVEAL) ==
                ANTI_EXFIL_SCAN_GENERIC);
  check("stale Exit after cleanup is inert",
        anti_exfil_continuation_exit(&state, second) == NULL &&
            callback_b_calls == 0);
}

static void test_repeated_ceremonies(void) {
  anti_exfil_continuation_t state = {0};
  for (unsigned i = 0; i < 1000; ++i) {
    uint32_t generation = anti_exfil_continuation_begin(&state, callback_a);
    check("repeated ceremony opens",
          anti_exfil_continuation_open_scanner(&state, generation));
    check("repeated ceremony backs out",
          anti_exfil_continuation_scanner_back(&state));
    check("repeated ceremony reopens",
          anti_exfil_continuation_open_scanner(&state, generation));
    anti_exfil_continuation_clear(&state);
  }
  check("repeat loop leaves no callback",
        state.phase == ANTI_EXFIL_CONTINUATION_INACTIVE &&
            state.return_cb == NULL);
}

int main(void) {
  test_restricted_routing_and_stateless_fallback();
  test_back_exit_cleanup_and_stale_callbacks();
  test_repeated_ceremonies();
  if (failures) {
    fprintf(stderr, "%u/%u continuation checks failed\n", failures, checks);
    return EXIT_FAILURE;
  }
  printf("PASS: %u continuation checks\n", checks);
  return EXIT_SUCCESS;
}
