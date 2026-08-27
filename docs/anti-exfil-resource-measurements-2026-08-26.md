# Anti-exfil transport resource checkpoint

Date: 2026-08-26

These are reproducible host measurements over the realistic four-input,
five-slot AEXT fixture. They are not camera or ESP32-P4 heap claims.

Run:

```text
cd main/core/test
make measure-anti-exfil
```

Representative results:

| Stage | Canonical CBOR bytes | Source parts at 150-byte maximum | Longest UR part | Source parts at 200-byte maximum | Longest UR part |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 1,629 | 11 | 358 chars | 9 | 420 chars |
| 2 | 819 | 6 | 332 chars | 5 | 386 chars |
| 3 | 1,954 | 14 | 340 chars | 10 | 452 chars |
| 4 | 1,139 | 8 | 344 chars | 6 | 438 chars |

The source-part count is the first complete fountain window; real scanning may
need additional mixed parts. The current scanner already reports cUR decoder
percent completion independently of anti-exfil policy.

The measured host sizes are 30,296 bytes for `anti_exfil_message_t` and 30,312
bytes for `anti_exfil_aext_view_t`. These records must remain in static or heap
storage, never an ESP-IDF task stack. The scanner handoff uses heap storage,
retains an exact copy of the canonical CBOR while its decoded PSBT view is live,
and wipes both allocations before release.

## Device measurement hook

Each recognized `x-btc-anti-exfil` scan now emits one
`ANTI_EXFIL_MEASURE` log line containing:

- canonical CBOR bytes and retained payload bytes;
- free 8-bit heap immediately before the owned copy;
- free heap after the copy and canonical re-decode;
- free heap after camera/scanner teardown; and
- the device's minimum free 8-bit heap watermark.

Capture the line for stage 1 and stage 3 at both 150- and 200-byte fragment
maxima. The simulator's heap values are fixed stubs and must not be recorded as
physical evidence. This hook measures the high-pressure scanner-to-workflow
handoff without selecting a network, stage, retry, or coverage policy.

The measurement binary can emit the actual first fountain window for every
stage, ready for QR rendering or a coordinator display harness:

```text
cd main/core/test
./measure_anti_exfil_transport --emit 150
./measure_anti_exfil_transport --emit 200
```

Each line identifies `stage`, `part/current-window-size`, and the complete UR.
Use the stage 1 and stage 3 lines for signer-side camera measurements. Stages 2
and 4 remain available to confirm that scanner ownership is carriage-neutral;
their presence does not make them acceptable signing requests.

## Physical measurement status

A clean six-scan stage-3/150-byte capture now shows stable heap recovery at the
scanner handoff. The owned-copy/re-decode delta was exactly 32,712 bytes on
every scan, `heap_after_camera_stop` remained within a 28-byte range, and the
lifetime minimum-free watermark stabilized after the third scan. Full values,
camera observations, and timing results are recorded in
`docs/kern-m5-physical-results-2026-08-27.md`.

Measurements still required include:

- largest free block before scan, after reconstruction, after PSBT parse, after
  signer output, and after cleanup (free/minimum heap at the scanner handoff is
  now logged automatically);
- QR decoder task stack high-water mark;
- decode time and extra fountain frames at 150- and 200-byte fragment maxima;
- bright/dim room scan distance, glare, display brightness, and frame rate;
- a longer repeated-ceremony soak beyond the six clean serial captures;
- one byte below, at, and above the eventual operational package limit.

Until those measurements exist, the current 256 KiB cUR reconstruction ceiling
is an implementation safety cap, not a promise that the camera workflow can
operate near that size.

The first physical camera run is recorded in
`docs/kern-m5-physical-results-2026-08-27.md`. All four canonical scans and the
ordinary singlesig/multisig regression paths completed. The clean filtered
serial capture supports a scanner-handoff heap-recovery claim for the tested
stage-3/150-byte fixture; later signer-output and operational-limit measurements
remain open.
