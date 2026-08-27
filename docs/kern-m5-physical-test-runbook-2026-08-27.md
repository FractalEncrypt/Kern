# Kern M5 physical test runbook

Date: 2026-08-27

Firmware checkpoint: `bdea4fa0c915279634bfdd92bb2723e56070d056`

Board: Waveshare ESP32-P4-WIFI6-Touch-LCD-7B (`wave_7b`)

This run measures transport and scanner behavior only. The protected workflow
must recognize and retain the exact request, report resource data, then stop at
the explicit integration-pending screen. It must not sign or choose any M6
network, retry, binding, or slot-coverage policy.

All supplied QR packs contain public test vectors, not wallet secrets. A key
must be loaded only because Kern currently exposes its scan page from the
loaded-wallet UI; it does not need to match these fixtures.

## Setup

1. Connect the device by USB and start an ESP-IDF serial monitor on its port.
2. Load any disposable test mnemonic and open Kern's scanner.
3. Open `main/core/test/physical_qr_output/index.html` on a computer display.
   From the WSL directory used to generate the packs, this command opens it in
   the Windows default browser:

   ```text
   explorer.exe "$(wslpath -w "$PWD/physical_qr_output/index.html")"
   ```

   Alternatively, double-click that `index.html` in Windows Explorer or use
   the local link in the test handoff. The index contains four links. Opening
   one starts its ordered QR animation automatically; the controls below the
   QR adjust speed or pause/step through frames.
4. Use full-screen mode, begin at 700 ms per frame, and set display brightness
   high enough to avoid glare. Keep the entire QR and quiet border in view.

Record the room lighting, display brightness, approximate scan distance, frame
delay, time to complete, highest progress seen before completion, and whether
extra cycles beyond the first fountain window were needed.

## Required four scans

Run these in order, dismissing the result screen and reopening the scanner
between scans:

| Test | QR pack | First-window parts | Expected result |
| --- | --- | ---: | --- |
| A | Stage 1, 150-byte | 11 | `Protected signing`; stage 1, 5 slots, 975 PSBT bytes |
| B | Stage 3, 150-byte | 14 | `Protected signing`; stage 3, 5 slots, 975 PSBT bytes |
| C | Stage 1, 200-byte | 9 | `Protected signing`; stage 1, 5 slots, 975 PSBT bytes |
| D | Stage 3, 200-byte | 10 | `Protected signing`; stage 3, 5 slots, 975 PSBT bytes |

For each row, open the named pack in the browser, scan its animated QR once,
and record the result. The expected result is shown **on Kern's display** after
the scan completes. The full detail text ends with `Protected review
integration is pending.` No transaction approval screen, signature, or
response QR should appear.

For every completed scan, capture the single serial line tagged
`ANTI_EXFIL_MEASURE`. Expected invariant values are:

| Stage | `cbor` | `retained` |
| ---: | ---: | ---: |
| 1 | 1629 | 31937 |
| 3 | 1954 | 32262 |

On this 32-bit target, `retained` includes the 30,308-byte owning request/view
as well as the exact canonical CBOR allocation; it is therefore intentionally
larger than `cbor`. The heap fields are measurements, not predetermined pass
values. A failure is a crash/reset, allocation error, malformed-request error
for these known-good packs, a retained value that differs from the table,
ordinary PSBT/text fallback, or any signing action.

## Repeat and regression passes

After the first four scans succeed:

1. Repeat the stage-3/150-byte scan 20 times. Record all measurement lines and
   note whether `heap_before` and `heap_after_camera_stop` stabilize rather than
   decline continually. `min_free` is a lifetime watermark and is not expected
   to rise between repetitions.
2. In Sparrow, create one ordinary testnet singlesig PSBT for a wallet whose
   key is loaded on Kern. Export the ordinary `crypto-psbt` QR with protected
   signing disabled. Kern should follow its pre-existing transaction review and
   signing flow, and Sparrow should accept the returned signed PSBT.
3. Repeat with an ordinary testnet multisig PSBT in which Kern controls one
   signer. Again, this is a regression test of the existing path, not an
   anti-exfil exchange.

SeedSigner is not needed as the source of these requests: both SeedSigner and
Kern are signing devices. Sparrow is the coordinator that displays the unsigned
PSBT to Kern. Existing SeedSigner wallets may still be used to construct the
testnet multisig wallet and complete its other signatures.

Do not use mainnet funds. Stop and retain the serial log if the device resets,
falls into the ordinary parser for an anti-exfil QR, or shows a signing prompt
for any supplied M5 fixture.

## Regenerating the QR packs

From WSL, where `qrencode` is available:

```text
cd /mnt/c/Users/FractalEncrypt/Documents/SeedSigner_AntiExfil/kern-m1/main/core/test
python3 generate_anti_exfil_physical_qrs.py
```

The generator builds the existing measurement emitter, uses only its exact UR
output, renders each part with `qrencode`, and writes a manifest containing the
SHA-256 of every ordered UR sequence. Generated images are intentionally
gitignored; the generator and corpus remain the reproducible sources.
