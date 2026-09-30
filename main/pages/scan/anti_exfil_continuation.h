#ifndef ANTI_EXFIL_CONTINUATION_H
#define ANTI_EXFIL_CONTINUATION_H

#include "../../core/anti_exfil/anti_exfil_types.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  ANTI_EXFIL_CONTINUATION_INACTIVE = 0,
  ANTI_EXFIL_CONTINUATION_CHOICE,
  ANTI_EXFIL_CONTINUATION_SCANNER,
} anti_exfil_continuation_phase_t;

typedef enum {
  ANTI_EXFIL_SCAN_GENERIC = 0,
  ANTI_EXFIL_SCAN_HOST_REVEAL,
  ANTI_EXFIL_SCAN_REJECT,
} anti_exfil_scan_route_t;

typedef struct {
  anti_exfil_continuation_phase_t phase;
  uint32_t generation;
  void (*return_cb)(void);
} anti_exfil_continuation_t;

uint32_t anti_exfil_continuation_begin(anti_exfil_continuation_t *state,
                                       void (*return_cb)(void));
void anti_exfil_continuation_clear(anti_exfil_continuation_t *state);
bool anti_exfil_continuation_open_scanner(anti_exfil_continuation_t *state,
                                          uint32_t generation);
bool anti_exfil_continuation_scanner_back(anti_exfil_continuation_t *state);
void (*anti_exfil_continuation_exit(anti_exfil_continuation_t *state,
                                    uint32_t generation))(void);
bool anti_exfil_continuation_scanner_active(
    const anti_exfil_continuation_t *state);
anti_exfil_scan_route_t
anti_exfil_continuation_route(const anti_exfil_continuation_t *state,
                              bool is_ur, bool is_explicit_anti_exfil,
                              anti_exfil_stage_t stage);

#endif // ANTI_EXFIL_CONTINUATION_H
