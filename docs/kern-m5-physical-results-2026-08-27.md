# Kern M5 physical results

Date: 2026-08-27

Firmware checkpoint: `bdea4fa0c915279634bfdd92bb2723e56070d056`

Board: Waveshare ESP32-P4-WIFI6-Touch-LCD-7B (`wave_7b`)

Camera detected at boot: OV5647, 1280x960 RAW10 binning path

## Environment

- Mid-morning, approximately 10:00 EST.
- Daylight through Venetian blinds at a nearby window, another window about
  15 feet away, and a skylight about 20 feet away.
- Four monitors on and one lamp about five feet away.
- Source displays at their normal brightness.
- Larger monitor and larger rendered QR scanned more reliably than a tablet.
- Best framing placed the whole QR at approximately 75% of the camera
  preview's vertical height with a clear border. Nearly 100% fill was slower;
  excessive distance also reduced reliability.

## Required scans

All four canonical public fixtures completed and reached the expected
`Protected signing` scaffold. No signing prompt, signature, or response QR was
reported.

| Stage | Maximum fragment | Time | Highest visible progress |
| ---: | ---: | ---: | ---: |
| 1 | 150 bytes | 7.2 s | about 50% |
| 3 | 150 bytes | 9.5 s | about 55% |
| 1 | 200 bytes | 5.5 s | about 50% |
| 3 | 200 bytes | 5.5 s | about 50% |

The 200-byte setting was 2.85 seconds faster on average in this four-scan
sample (5.5 seconds versus 8.35 seconds, approximately 34%). The last visible
percentage is not the decoder's final state: the scanner can receive the part
that completes fountain reconstruction and transition immediately, before the
periodic UI progress update paints 100%.

## Repeated stage-3/150-byte scans

Nine repetitions were recorded before stopping. The brief screen between the
camera and result was initially mistaken for an additional result that needed
manual transcription; it is scanner completion/teardown state and is not part
of the required evidence.

| Repetition | Time | Highest visible progress | Distance |
| ---: | ---: | ---: | ---: |
| 1 | 8.10 s | about 55% | not recorded |
| 2 | 8.80 s | about 55% | not recorded |
| 3 | 8.60 s | about 55% | not recorded |
| 4 | 9.38 s | about 55% | not recorded |
| 5 | 10.12 s | about 55% | not recorded |
| 6 | 18.30 s | about 55% | 10 in |
| 7 | 9.58 s | about 55% | 18 in |
| 8 | 8.58 s | about 55% | 15 in |
| 9 | 9.28 s | about 55% | 16 in |

Median completion time was 9.28 seconds. Mean was 10.08 seconds including the
10-inch outlier and 9.06 seconds without it. No progressive timing slowdown is
visible in this sample; the one large delay correlates with the closest
recorded distance.

The original serial monitor output was allowed to accumulate and suffered
host-side USB serial reconnect noise, so its middle section was truncated. A
subsequent clean interactive capture recorded six additional consecutive
stage-3/150-byte scans:

| Scan | `cbor` | `retained` | `heap_before` | `heap_after_copy` | `heap_after_camera_stop` | `min_free` |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 1954 | 32262 | 23141223 | 23108511 | 25818863 | 22777463 |
| 2 | 1954 | 32262 | 23261275 | 23228563 | 25818851 | 22777451 |
| 3 | 1954 | 32262 | 23290991 | 23258279 | 25818879 | 22777407 |
| 4 | 1954 | 32262 | 23305071 | 23272359 | 25818871 | 22777407 |
| 5 | 1954 | 32262 | 23290991 | 23258279 | 25818875 | 22777407 |
| 6 | 1954 | 32262 | 23229551 | 23196839 | 25818871 | 22777407 |

The owned-copy/re-decode step reduced reported free heap by exactly 32,712
bytes in every scan. The device-owned request payload is 32,262 bytes: 30,308
bytes for the request/view plus the exact 1,954-byte canonical CBOR. The
remaining observed difference includes allocator and decode overhead.

`heap_after_camera_stop` stayed within a 28-byte range (25,818,851 to
25,818,879) and did not decline across the sequence. The lifetime `min_free`
watermark fell by 56 bytes during the first three scans and then remained
exactly 22,777,407 for scans 3 through 6. This short sequence passes the heap
recovery check: there is no evidence of scan-to-scan retained-heap growth. It
does not replace a longer soak test.

## Ordinary transaction regression

- Sparrow singlesig PSBT: scanned almost immediately, reviewed and signed
  successfully, and Sparrow accepted the returned PSBT.
- Sparrow multisig PSBT: reviewed and signed successfully, and Sparrow accepted
  the returned PSBT.
- The multisig review displayed `Unproven fee: 1 of 1 input amounts are not
  backed by their previous transaction.` Kern received a witness UTXO amount
  without the full matching previous transaction. This is an explicit fee
  provenance warning in the existing Kern review path, not an anti-exfil
  failure; it does not by itself make the SegWit signature invalid.

## Camera observations and next controlled test

- A SeedQR displayed on a bright tablet did not decode until the tablet dimmed
  just before its screensaver.
- SeedSigner decoded the same displayed QR almost immediately.
- Kern decoded SeedQR more quickly once a suitable distance and border were
  found.
- The scanner `Exposure` slider was initially at approximately 40% of its
  travel. Moving it left to approximately 30% made the same displayed SeedQR
  decode effectively immediately. These are visual slider positions, not
  calibrated numeric sensor values.
- The anti-exfil branch does not modify `main/qr/scanner.c` or the camera
  pipeline relative to Kern 0.0.17. The result is more consistent with
  exposure/focus/framing sensitivity than with the added transport routing.

The successful lower-exposure result strongly favors camera tuning over a
defective sensor. Keep approximately 75% vertical QR fill and a complete quiet
border; for bright displays, begin near the successful 30% slider position and
adjust only as needed. This firmware build has the OV5647 focus-motor option
disabled, so buying an autofocus module alone would not activate focus control
without a corresponding build change.
