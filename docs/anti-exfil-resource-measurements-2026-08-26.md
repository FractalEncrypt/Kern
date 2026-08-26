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
storage, never an ESP-IDF task stack. The scanner classification scaffold uses
heap storage and wipes it before release.

## Physical measurements still required

- minimum free heap and largest free block before scan, after reconstruction,
  after PSBT parse, after signer output, and after cleanup;
- QR decoder task stack high-water mark;
- decode time and extra fountain frames at 150- and 200-byte fragment maxima;
- bright/dim room scan distance, glare, display brightness, and frame rate;
- twenty repeated ceremonies to check heap recovery;
- one byte below, at, and above the eventual operational package limit.

Until those measurements exist, the current 256 KiB cUR reconstruction ceiling
is an implementation safety cap, not a promise that the camera workflow can
operate near that size.
