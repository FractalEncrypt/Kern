# Post-quantum signature benchmark (SLH-DSA-SHA2-128s)

Measures keygen and signing time for SLH-DSA-SHA2-128s on the ESP32-P4, using the unmodified [pq-code-package/slhdsa-c](https://github.com/pq-code-package/slhdsa-c) reference implementation. The point is to isolate the effect of the higher-clocked RISC-V core against Blockstream Research's [published ESP32-S3 numbers](https://blog.blockstream.com/hardware-wallets-post-quantum-signatures/) for Jade Plus:

| Op | Jade Plus (ESP32-S3, Xtensa LX7 @ 240 MHz) |
|---|---|
| KeyGen | 7.06 s |
| SigGen | 52.85 s (7856 B signature) |

This is a measurement harness and nothing else. It never reaches a release build, touches no wallet state, and uses only ACVP test vectors and throwaway keys.

## Reproducing

```bash
git submodule update --init --recursive
just bench wave_4b          # build with the benchmark enabled
just bench-flash wave_4b    # flash and open the monitor
```

Results print between `===== BENCH BEGIN =====` and `===== BENCH END =====`. Every line is prefixed `BENCH` or `KAT` and is grep-friendly.

Iteration count defaults to 3. For reportable numbers, set `CONFIG_KERN_BENCH_HBS_ITERATIONS=100` in `sdkconfig.bench` and rebuild — at roughly the times above that pass takes over an hour and a half.

To refresh the known-answer vectors (the generated header is committed, so this is only needed if the ACVP set moves):

```bash
python3 scripts/gen_slhdsa_kat.py
```

## What it measures

Cold mode, matching the published methodology: nothing is reused between iterations, everything is recomputed. The message is a fixed 32 bytes, identical every iteration.

Each operation is timed two ways:

- **Wall time** (`esp_timer_get_time`) — how long a user waits.
- **CPU cycles** (`esp_cpu_get_cycle_count`) — whether the RISC-V core does more or less work per operation than the Xtensa LX7. This is the result that survives any future clock change, and it is the reason the benchmark exists.

The summary also reports `cyc_per_us`. That is a contamination check, not a datum: `esp_cpu_get_cycle_count()` reads `mcycle`, which counts every cycle executed on the core including any task that preempted the measurement. If `cyc_per_us` does not land on the reported CPU clock, the sample is dirty and the numbers next to it are not comparable. The benchmark task is pinned to core 1 at priority 10 — above the LVGL task's 6, which has no core affinity — to keep this from happening.

### Rotations on the P4

The ESP32-P4 targets `rv32imafc_zicsr_zifencei_xesppie`. There is **no Zbb**, so no `rori`, so every SHA-256 rotation compiles to a shift-shift-or triple. Verified by disassembling a σ₀ expression built with this toolchain:

```
sig0:  slli a4,a0,0x19 / srli a5,a0,0x7 / slli a3,a0,0xe / add a5,a5,a4
       srli a4,a0,0x12 / add a4,a4,a3  / xor a5,a5,a4   / srli a0,a0,0x3 / xor a0,a0,a5
```

The Xtensa LX7 has a funnel shifter (`ssai`/`src`) that does a fixed-amount rotate in fewer operations. Expect the P4 to spend *more* cycles per SHA-256 block; the cycle counts are what settle it.

### Hash path

The reference `sha2_256.c`, in software. No hardware SHA accelerator.

Worth stating explicitly, because a faster-looking option is right there and is a trap. Kern's libwally routes SHA-256 through mbedTLS (`ccan_config.h` hardcodes `CCAN_CRYPTO_SHA256_USE_MBEDTLS`) and the build sets `CONFIG_MBEDTLS_HARDWARE_SHA=1`, so `wally_sha256()` really does run on the P4 SHA peripheral. But `slh_sha2.c` needs three things no standard SHA-256 API exposes: `sha2_256_copy()` for midstate cloning, `sha2_256_final_pad()` plus a raw `sha2_256_compress(void *)` over a caller-owned block, and the `sha2_256_t` layout that packs state and message block into one array. mbedTLS supplies `clone` and nothing else. Wiring the rest to IDF's low-level `esp_sha_*` API would be hardware-accelerator integration — a different experiment — and the peripheral takes a FreeRTOS mutex shared with AES on every call, which for hundreds of millions of single-block hashes may well lose to software anyway.

## Divergences from the Jade port

Compared against [DmitriiKJ/Jade @ `hash_based_signatures`](https://github.com/DmitriiKJ/Jade/tree/hash_based_signatures) (HEAD `f5c90a5e`).

**First, a correction.** That branch carries three independent schemes — `slh_dsa`, `shrincs` and `xmss`. In `main/slh_dsa/`, `sha2_256.c` and `sha2_512.c` are byte-for-byte upstream pure C: no mbedTLS, no wally, no `esp_` symbol anywhere. The substitution of device-internal hash and endianness helpers that is sometimes attributed to the SLH-DSA port belongs to **shrincs** (`main/shrincs/hash.h` typedefs `SHA256_CTX` to `mbedtls_sha256_context`; `main/shrincs/byte_order.h` maps `REVERSE32/64` onto `__builtin_bswap32/64`). For SLH-DSA, Jade kept the reference hash — so matching Jade and avoiding the accelerator are the same choice, not competing ones.

| # | Jade | Kern | Why |
|---|---|---|---|
| 1 | Vendors all 18 upstream files, including SHAKE and SHA-3 | 4 `.c` + 8 `.h`, SHA-2 only | 128s needs no SHAKE. `sha2_512.c` is still required: `slh_sha2.c` unconditionally defines the `sha2_512_*` hooks and `slh_var_s` embeds a `sha2_512_t` |
| 2 | Patches the library with a per-layer progress callback (`plat_local.h`, `slh_var.h`, `slh_dsa.h/.c`, `slh_sha2.c`) to survive its watchdog | **No patch.** Task watchdog disabled in `sdkconfig.bench` | Keeps the reference code unmodified, and keeps callback overhead out of the timed region. Feeding the watchdog between iterations would not have worked anyway — the trip happens inside a single call |
| 3 | Top-layer XMSS leaf cache, persisted as an 8208 B NVS blob | Not ported | Caching is out of scope; its absence *is* cold mode |
| 4 | Dual-core WOTS/FORS work-stealing pool (force-disabled on that branch by a `#define CONFIG_FREERTOS_UNICORE 1` placed before `#include <sdkconfig.h>`) | Not ported | Parallel execution is out of scope. This is also the only place ESP-IDF headers enter Jade's vendored library |
| 5 | Also defines a non-standard variant (`h=45, d=5, a=13, k=10`) | Standard `slh_dsa_sha2_128s` only | Only the published parameter set is being compared |
| 6 | Timed **host-side in Python over serial** (`test_shrincs.py`, a running mean of `time.time()`), including serial round-trip and progress-bar redraws | On-device `esp_timer` and `mcycle` around the primitive only | More precise — **but it makes Kern's numbers slightly faster than a like-for-like Jade measurement, so any P4 advantage is marginally overstated.** Weigh this before quoting a speedup ratio |
| 7 | No Kconfig or sdkconfig changes; always compiled in | Kconfig-gated, `default n`, with a `sdkconfig.bench` overlay | Must never reach a release build |
| 8 | ESP32-S3 @ 240 MHz | ESP32-P4 @ **360 MHz — shipping Kern, not the 400 MHz part maximum** | The useful wall-time figure is what a Kern user actually waits for. Cycle counts are clock-independent, so the core comparison is unaffected |
| 9 | Vendored from a May-2026 fork point | Submodule pinned at upstream `174c02e` | Newer, and unmodified by construction |
| 10 | Hardcoded hex secret key in the signing RPC; leaf-cache lookup left disabled behind `if (0 && ...)` | Throwaway seeds generated in the harness; no storage of any kind | No wallet state is involved |

### Comparability caveat

`CONFIG_SPIRAM_XIP_FROM_PSRAM=y` and `CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y` mean instructions are fetched from PSRAM through the 256 KB L2 cache rather than from IRAM. For a tight hash loop that is a real effect on cycle counts, and a genuine difference from a build that runs the same code out of internal RAM.

## Effect on the default build

The benchmark is gated behind `CONFIG_KERN_BENCH_HBS`, which defaults to `n`. With it off, `main/bench/` is filtered out of the source glob and `components/slhdsa-c` compiles to an empty component.

Verified rather than asserted, against `CONFIG_APP_REPRODUCIBLE_BUILD=y`: building `wave_4b` before and after these changes produces images differing in exactly 65 bytes, in two runs — `esp_app_desc_t.app_elf_sha256` at image offset `0xb0`, and the trailing checksum plus appended image SHA-256. Mask those two self-digests and the images are identical. Every loadable byte of the firmware is unchanged; the digests move only because adding any line to `main/main.c` changes the ELF's debug and symbol sections.

That is also why `bench_hbs_start()` is forward-declared inside `app_main` instead of included at the top of the file: a line added above the existing `ESP_ERROR_CHECK` calls shifts `__LINE__` in every one of them, which relocates code and moves roughly a million bytes of the image.
