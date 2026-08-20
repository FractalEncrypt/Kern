/*
 * 64-bit cycle counts from the ESP32-P4's 32-bit counter.
 *
 * Header-only so the host test can exercise it; see main/bench/test.
 */

#ifndef KERN_BENCH_CYCLE_UNWRAP_H
#define KERN_BENCH_CYCLE_UNWRAP_H

#include <stdint.h>

/*
 * Reconstruct a 64-bit cycle count from the 32-bit counter.
 *
 * esp_cpu_get_cycle_count() is uint32_t and ESP-IDF exposes no wider version;
 * on the P4 it reads the standard RISC-V `cycle` CSR, and IDF never reads the
 * `cycleh` half on this target, so probing for it risks an illegal-instruction
 * trap. The low 32 bits below are therefore an exact hardware measurement and
 * only the wrap count is inferred.
 *
 * At 360 MHz the counter wraps every 2^32 / 360e6 = 11.93 s, so a 27 s signing
 * run wraps twice and a naive subtraction silently loses 2^33 cycles. The wrap
 * count is an integer, and picking the wrong one would take a wall-clock error
 * of half a wrap period -- almost 6 seconds -- against an esp_timer that is
 * accurate to microseconds. Measured residuals are on the order of 200 cycles
 * out of 9.7 billion.
 */
static inline uint64_t unwrap_cycles(uint32_t lo0, uint32_t lo1,
                                     int64_t elapsed_us, int cpu_mhz,
                                     int *wraps_out) {
  uint32_t delta32 = lo1 - lo0; /* exact modulo 2^32 */
  double expected = (double)elapsed_us * (double)cpu_mhz;
  double wraps = (expected - (double)delta32) / 4294967296.0;
  int64_t k = (int64_t)(wraps + 0.5);
  if (k < 0) {
    k = 0;
  }
  *wraps_out = (int)k;
  return (uint64_t)delta32 + (uint64_t)k * 4294967296ULL;
}

#endif // KERN_BENCH_CYCLE_UNWRAP_H
