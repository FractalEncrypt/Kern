#include "anti_exfil_continuation.h"
#include <stddef.h>

static uint32_t next_generation(uint32_t generation) {
  generation++;
  return generation ? generation : 1;
}

uint32_t anti_exfil_continuation_begin(anti_exfil_continuation_t *state,
                                       void (*return_cb)(void)) {
  if (!state)
    return 0;
  state->generation = next_generation(state->generation);
  state->phase = ANTI_EXFIL_CONTINUATION_CHOICE;
  state->return_cb = return_cb;
  return state->generation;
}

void anti_exfil_continuation_clear(anti_exfil_continuation_t *state) {
  if (!state)
    return;
  state->generation = next_generation(state->generation);
  state->phase = ANTI_EXFIL_CONTINUATION_INACTIVE;
  state->return_cb = NULL;
}

bool anti_exfil_continuation_open_scanner(anti_exfil_continuation_t *state,
                                          uint32_t generation) {
  if (!state || state->phase != ANTI_EXFIL_CONTINUATION_CHOICE ||
      state->generation != generation)
    return false;
  state->phase = ANTI_EXFIL_CONTINUATION_SCANNER;
  return true;
}

bool anti_exfil_continuation_scanner_back(anti_exfil_continuation_t *state) {
  if (!state || state->phase != ANTI_EXFIL_CONTINUATION_SCANNER)
    return false;
  state->phase = ANTI_EXFIL_CONTINUATION_CHOICE;
  return true;
}

void (*anti_exfil_continuation_exit(anti_exfil_continuation_t *state,
                                    uint32_t generation))(void) {
  if (!state || state->phase == ANTI_EXFIL_CONTINUATION_INACTIVE ||
      state->generation != generation)
    return NULL;
  void (*return_cb)(void) = state->return_cb;
  anti_exfil_continuation_clear(state);
  return return_cb;
}

bool anti_exfil_continuation_scanner_active(
    const anti_exfil_continuation_t *state) {
  return state && state->phase == ANTI_EXFIL_CONTINUATION_SCANNER;
}

anti_exfil_scan_route_t
anti_exfil_continuation_route(const anti_exfil_continuation_t *state,
                              bool is_ur, bool is_explicit_anti_exfil,
                              anti_exfil_stage_t stage) {
  if (!anti_exfil_continuation_scanner_active(state))
    return ANTI_EXFIL_SCAN_GENERIC;
  if (!is_ur || !is_explicit_anti_exfil ||
      stage != ANTI_EXFIL_STAGE_HOST_REVEAL)
    return ANTI_EXFIL_SCAN_REJECT;
  return ANTI_EXFIL_SCAN_HOST_REVEAL;
}
