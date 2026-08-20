#include "bench_hbs.h"

#include "cycle_unwrap.h"
#include "kat_slhdsa_sha2_128s.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <esp_cpu.h>
#include <esp_heap_caps.h>
#include <esp_private/esp_clk.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "slh_dsa.h"

#ifndef KERN_BENCH_GIT_REV
#define KERN_BENCH_GIT_REV "unknown"
#endif
#ifndef KERN_BENCH_SLHDSA_REV
#define KERN_BENCH_SLHDSA_REV "unknown"
#endif

/*
 * The RISC-V core Kern runs on. Recorded because it is the answer to the
 * question this benchmark exists to ask: there is no Zbb here, so no rori, and
 * every SHA-256 rotation costs a shift-shift-or triple rather than a single
 * instruction. Verified by disassembling a rotation built with this toolchain.
 */
#define BENCH_ISA                                                              \
  "rv32imafc_zicsr_zifencei_xesppie (no Zbb: rotations are shift-shift-or)"

#if defined(__OPTIMIZE_SIZE__)
#define BENCH_OPT "-Os"
#elif defined(__OPTIMIZE__)
#define BENCH_OPT "-O2 (COMPILER_OPTIMIZATION_PERF)"
#else
#define BENCH_OPT "-O0/-Og"
#endif

#define BENCH_ITERATIONS CONFIG_KERN_BENCH_HBS_ITERATIONS
/*
 * Measured high-water mark on wave_4b across the KAT gate and both timed
 * operations is 4584 bytes, so this is roughly 3.5x headroom. It started at
 * 64 KB; the harness still prints the high-water mark every run, so the figure
 * stays measured rather than assumed. Smaller also matters because internal
 * DRAM is tight after camera/ISP init on some boards.
 */
#define BENCH_TASK_STACK (16 * 1024)
#define BENCH_TASK_CORE 1
/* Above the LVGL task's priority 6, which has no core affinity and would
 * otherwise preempt a timed region and inflate the cycle count. */
#define BENCH_TASK_PRIO 10

#define BENCH_MSG_LEN 32

typedef struct {
  int64_t wall_us;
  uint64_t cycles;
  int wraps;
} sample_t;

typedef struct {
  int64_t wall_min, wall_max, wall_sum;
  uint64_t cyc_min, cyc_max, cyc_sum;
  int n;
} stats_t;

static void stats_init(stats_t *s) {
  memset(s, 0, sizeof(*s));
  s->wall_min = INT64_MAX;
  s->cyc_min = UINT64_MAX;
}

static void stats_add(stats_t *s, const sample_t *x) {
  if (x->wall_us < s->wall_min)
    s->wall_min = x->wall_us;
  if (x->wall_us > s->wall_max)
    s->wall_max = x->wall_us;
  s->wall_sum += x->wall_us;
  if (x->cycles < s->cyc_min)
    s->cyc_min = x->cycles;
  if (x->cycles > s->cyc_max)
    s->cyc_max = x->cycles;
  s->cyc_sum += x->cycles;
  s->n++;
}

static void report(const char *op, const stats_t *s, int cpu_mhz) {
  int64_t wall_mean = s->wall_sum / s->n;
  uint64_t cyc_mean = s->cyc_sum / (uint64_t)s->n;
  printf("BENCH %s wall_us min=%" PRId64 " mean=%" PRId64 " max=%" PRId64 "\n",
         op, s->wall_min, wall_mean, s->wall_max);
  printf("BENCH %s cycles  min=%" PRIu64 " mean=%" PRIu64 " max=%" PRIu64 "\n",
         op, s->cyc_min, cyc_mean, s->cyc_max);
  printf("BENCH %s wall_s  mean=%.3f\n", op, (double)wall_mean / 1e6);
  /*
   * Integrity check on the cycle count, not a preemption check. The cycle CSR
   * and esp_timer both advance while another task runs on this core, so
   * same-core preemption leaves this ratio at the CPU clock; what it does catch
   * is a mis-reconstructed wrap, a clock that is not what was configured, or a
   * core that stalled. Evidence against preemption is the min/max spread above:
   * iterations agreeing to a fraction of a percent did not share the core.
   */
  printf("BENCH %s cyc_per_us mean=%.2f (expect %d)\n", op,
         (double)cyc_mean / (double)wall_mean, cpu_mhz);
}

static bool run_kats(const slh_param_t *prm) {
  static uint8_t sk[64];
  static uint8_t pk[32];
  static uint8_t sig[7856];
  bool ok = true;

  if (slh_sk_sz(prm) != sizeof(sk) || slh_pk_sz(prm) != sizeof(pk) ||
      slh_sig_sz(prm) != sizeof(sig)) {
    printf("KAT FAIL: size mismatch sk=%u pk=%u sig=%u\n",
           (unsigned)slh_sk_sz(prm), (unsigned)slh_pk_sz(prm),
           (unsigned)slh_sig_sz(prm));
    return false;
  }

  for (int i = 0; i < KAT_KEYGEN_COUNT; i++) {
    const kat_keygen_t *t = &kat_keygen[i];
    memset(sk, 0, sizeof(sk));
    memset(pk, 0, sizeof(pk));
    if (slh_keygen_internal(sk, pk, t->sk_seed, t->sk_prf, t->pk_seed, prm) !=
        0) {
      printf("KAT FAIL: keygen tcId %d returned error\n", t->tc_id);
      ok = false;
      continue;
    }
    bool pass =
        memcmp(sk, t->sk, t->sk_len) == 0 && memcmp(pk, t->pk, t->pk_len) == 0;
    printf("KAT keyGen tcId %d: %s\n", t->tc_id, pass ? "PASS" : "FAIL");
    ok = ok && pass;
  }

  /* slh_sign_internal, deterministic (addrnd = NULL). */
  memset(sig, 0, sizeof(sig));
  size_t n = slh_sign_internal(sig, kat_sg_int0_msg, sizeof(kat_sg_int0_msg),
                               kat_sg_int0_sk, NULL, prm);
  bool pass =
      n == sizeof(kat_sg_int0_sig) && memcmp(sig, kat_sg_int0_sig, n) == 0;
  printf("KAT sigGen internal: %s\n", pass ? "PASS" : "FAIL");
  ok = ok && pass;

  /* slh_sign, external pure interface with a context string. */
  memset(sig, 0, sizeof(sig));
  n = slh_sign(sig, kat_sg_ext0_msg, sizeof(kat_sg_ext0_msg), kat_sg_ext0_ctx,
               sizeof(kat_sg_ext0_ctx), kat_sg_ext0_sk, NULL, prm);
  pass = n == sizeof(kat_sg_ext0_sig) && memcmp(sig, kat_sg_ext0_sig, n) == 0;
  printf("KAT sigGen external: %s\n", pass ? "PASS" : "FAIL");
  ok = ok && pass;

  return ok;
}

static void bench_task(void *arg) {
  (void)arg;
  const slh_param_t *prm = &slh_dsa_sha2_128s;

  /* Throwaway seeds. Never a user key -- this build never touches wallet state.
   */
  static const uint8_t sk_seed[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                      0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                                      0x0c, 0x0d, 0x0e, 0x0f};
  static const uint8_t sk_prf[16] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                                     0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
                                     0x1c, 0x1d, 0x1e, 0x1f};
  static const uint8_t pk_seed[16] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                                      0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
                                      0x2c, 0x2d, 0x2e, 0x2f};
  /* Empty context, but a real pointer: the hash hooks pass it straight to
   * sha2_256_update, and a NULL source is undefined behaviour even at length 0.
   */
  static const uint8_t empty_ctx[1] = {0};
  static uint8_t msg[BENCH_MSG_LEN];
  for (int i = 0; i < BENCH_MSG_LEN; i++)
    msg[i] = (uint8_t)i;

  static uint8_t sk[64];
  static uint8_t pk[32];
  static uint8_t sig[7856];

  printf("\n===== BENCH BEGIN =====\n");
  printf("BENCH scheme     %s\n", slh_alg_id(prm));
  const int cpu_mhz = esp_clk_cpu_freq() / 1000000;
  printf("BENCH cpu_mhz    %d (configured %d)\n", cpu_mhz,
         CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);
  printf("BENCH isa        %s\n", BENCH_ISA);
  printf("BENCH opt        %s\n", BENCH_OPT);
  printf("BENCH hash_path  reference sha2_256.c (software, no accelerator)\n");
  printf("BENCH kern_rev   %s\n", KERN_BENCH_GIT_REV);
  printf("BENCH slhdsa_rev %s\n", KERN_BENCH_SLHDSA_REV);
  printf("BENCH iterations %d\n", BENCH_ITERATIONS);
  printf("BENCH mode       cold (no state reused between iterations)\n");

  if (!run_kats(prm)) {
    printf("BENCH ABORTED: known-answer tests failed; timings would be "
           "meaningless\n");
    printf("===== BENCH END =====\n");
    vTaskDelete(NULL);
    return;
  }
  printf("BENCH kats       PASS\n");

  stats_t kg, sg;
  stats_init(&kg);
  stats_init(&sg);

  for (int i = 0; i < BENCH_ITERATIONS; i++) {
    sample_t s;
    int64_t t0 = esp_timer_get_time();
    uint32_t c0 = esp_cpu_get_cycle_count();
    int rc = slh_keygen_internal(sk, pk, sk_seed, sk_prf, pk_seed, prm);
    uint32_t c1 = esp_cpu_get_cycle_count();
    int64_t t1 = esp_timer_get_time();
    if (rc != 0) {
      printf("BENCH keygen iteration %d failed\n", i);
      break;
    }
    s.wall_us = t1 - t0;
    s.cycles = unwrap_cycles(c0, c1, s.wall_us, cpu_mhz, &s.wraps);
    stats_add(&kg, &s);
    printf("BENCH iter keygen %d wall_us=%" PRId64 " cycles=%" PRIu64
           " wraps=%d\n",
           i, s.wall_us, s.cycles, s.wraps);

    t0 = esp_timer_get_time();
    c0 = esp_cpu_get_cycle_count();
    size_t n = slh_sign(sig, msg, sizeof(msg), empty_ctx, 0, sk, NULL, prm);
    c1 = esp_cpu_get_cycle_count();
    t1 = esp_timer_get_time();
    if (n != slh_sig_sz(prm)) {
      printf("BENCH sign iteration %d produced %u bytes\n", i, (unsigned)n);
      break;
    }
    s.wall_us = t1 - t0;
    s.cycles = unwrap_cycles(c0, c1, s.wall_us, cpu_mhz, &s.wraps);
    stats_add(&sg, &s);
    printf("BENCH iter sign %d wall_us=%" PRId64 " cycles=%" PRIu64
           " wraps=%d\n",
           i, s.wall_us, s.cycles, s.wraps);
  }

  if (kg.n > 0)
    report("keygen", &kg, cpu_mhz);
  if (sg.n > 0)
    report("sign", &sg, cpu_mhz);
  printf("BENCH sig_bytes  %u\n", (unsigned)slh_sig_sz(prm));
  /* ESP-IDF returns this in bytes, unlike vanilla FreeRTOS which returns words.
   */
  printf("BENCH stack_hwm  %u bytes unused of %d\n",
         (unsigned)uxTaskGetStackHighWaterMark(NULL), BENCH_TASK_STACK);
  printf("===== BENCH END =====\n");

  vTaskDelete(NULL);
}

void bench_hbs_start(void) {
  /*
   * Internal DRAM first: a PSRAM stack costs cache misses that land straight in
   * the cycle counts this benchmark exists to report. It may not fit -- the ISP
   * pipeline holds enough internal DRAM after camera init that even a 32 KB
   * stack fails elsewhere in this codebase (see qr/scanner.c) -- so fall back
   * rather than silently not running, and say so, because the fallback changes
   * what the numbers mean.
   */
  printf("BENCH free_internal %u bytes, largest block %u\n",
         (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
         (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));

  if (xTaskCreatePinnedToCore(bench_task, "bench_hbs", BENCH_TASK_STACK, NULL,
                              BENCH_TASK_PRIO, NULL,
                              BENCH_TASK_CORE) == pdPASS) {
    return;
  }

  printf(
      "BENCH WARNING: %d byte internal stack unavailable; retrying in PSRAM.\n",
      BENCH_TASK_STACK);
  printf("BENCH WARNING: a PSRAM stack inflates cycle counts -- treat results "
         "as indicative.\n");
  if (xTaskCreatePinnedToCoreWithCaps(
          bench_task, "bench_hbs", BENCH_TASK_STACK, NULL, BENCH_TASK_PRIO,
          NULL, BENCH_TASK_CORE, MALLOC_CAP_SPIRAM) != pdPASS) {
    printf("BENCH ABORTED: could not create the benchmark task\n");
  }
}
