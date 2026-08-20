/*
 * Tests for the 32-to-64-bit cycle-counter reconstruction.
 *
 * The first on-device run reported sign cyc_per_us=42.77 against an expected
 * 360: a 27 s signing call wrapped the 32-bit counter twice and a plain
 * subtraction lost 2^33 cycles. These cases pin the fix, including the real
 * logged values that exposed it.
 */

#include <stdio.h>

#include "../cycle_unwrap.h"

#define WRAP 4294967296.0

static int failures = 0;

static void expect(const char *name, uint32_t lo0, double true_cycles, int mhz,
                   int want_wraps) {
  uint32_t lo1 = (uint32_t)(lo0 + (uint64_t)true_cycles);
  int64_t us = (int64_t)(true_cycles / mhz);
  int wraps = -1;
  uint64_t got = unwrap_cycles(lo0, lo1, us, mhz, &wraps);
  double err = (double)got - true_cycles;
  int ok = err > -2000 && err < 2000 && wraps == want_wraps;
  printf("  %-28s wraps=%d err=%.0f %s\n", name, wraps, err,
         ok ? "PASS" : "FAIL");
  if (!ok)
    failures++;
}

/* Values straight from the first on-device log. */
static void expect_logged(const char *name, uint32_t delta32, int64_t us,
                          uint64_t want) {
  int wraps = -1;
  uint64_t got = unwrap_cycles(0, delta32, us, 360, &wraps);
  int ok = got == want;
  printf("  %-28s cycles=%llu %s\n", name, (unsigned long long)got,
         ok ? "PASS" : "FAIL");
  if (!ok)
    failures++;
}

int main(void) {
  printf("cycle unwrap\n");

  /* uint32 subtraction already absorbs the first wrap; only extra spans are
   * added. */
  expect("0.5 wrap", 0, 0.5 * WRAP, 360, 0);
  expect("0.99 wrap", 0, 0.99 * WRAP, 360, 0);
  expect("1.0 wrap exactly", 0, 1.0 * WRAP, 360, 1);
  expect("1.5 wraps", 0, 1.5 * WRAP, 360, 1);
  expect("2.5 wraps", 0, 2.5 * WRAP, 360, 2);
  expect("3.9 wraps", 0, 3.9 * WRAP, 360, 3);
  expect("1.5 wraps, high start", 4290000000u, 1.5 * WRAP, 360, 1);
  expect("2.5 wraps, high start", 4290000000u, 2.5 * WRAP, 360, 2);
  expect("0.3 wrap at 240 MHz", 12345, 0.3 * WRAP, 240, 0);

  expect_logged("logged keygen (no wrap)", 1275672455u, 3543536, 1275672455ULL);
  expect_logged("logged sign 0", 1157626430u, 27076559, 9747561022ULL);
  expect_logged("logged sign 1", 1158301796u, 27078435, 9748236388ULL);
  expect_logged("logged sign 2", 1158641715u, 27079379, 9748576307ULL);

  printf(failures ? "\n%d check(s) FAILED\n" : "\nall checks passed\n",
         failures);
  return failures ? 1 : 0;
}
