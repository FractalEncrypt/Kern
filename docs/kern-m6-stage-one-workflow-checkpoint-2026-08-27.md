# Kern M6 stage-1 workflow checkpoint

Date: 2026-08-27

Board: Waveshare ESP32-P4-WIFI6-Touch-LCD-7B (`wave_7b`)

This checkpoint connects a protected stage-1 request to Kern's existing
transaction review, an explicit `Create commitments` approval, and an animated
`x-btc-anti-exfil` stage-2 response. Stage 3 remains deliberately fail-closed.

## Workflow boundary

- A recognized `x-btc-anti-exfil` scan is always consumed and never falls
  through to ordinary PSBT, bytes, or text handling.
- The experimental anti-exfil setting must be enabled and Kern must be on
  testnet.
- Stage 3 is recognized but refused before transaction review or signing.
- Stage 1 must pass a non-signing authoritative preflight. Preflight rechecks
  the exact frozen PSBT, locally derives every controlled key and sighash, and
  requires the complete ordered set of locally controlled signing slots.
- The exact retained PSBT bytes are parsed into Kern's existing review and
  policy UI. Existing Expected-owned, Permissive, and Partial signing policy is
  neither bypassed nor duplicated.
- Only the explicit `Create commitments` action invokes the signer. Backing out
  of review cannot create a response.
- The response is displayed as an animated `x-btc-anti-exfil` UR titled
  `Nonce commitments`.
- Done destroys the viewer and reports `Step 1 of 2 complete`, explicitly
  stating that the transaction is not signed.
- While anti-exfil signing is enabled, an ordinary transaction PSBT is refused
  with `Protected signing required`. BIP322 routing remains separate.

The implementation makes no retry, session substitution, coordinator merge,
or stage-3 continuation decision. Those remain outside this checkpoint.

## Physical acceptance

The signed ESP-IDF 6.0.2 firmware was flashed to the physical board on COM6.
The canonical 200-byte-fragment stage-1 fixture was exercised with the pinned
public test seed.

Observed results:

1. With Anti-exfil signing disabled, stage 1 was refused as disabled.
2. With Anti-exfil signing enabled, the stage-3 fixture was refused because
   step 2 remains disabled.
3. Stage 1 passed authoritative preflight and entered the protected transaction
   review.
4. The synthetic four-input, five-slot fixture required both Kern's existing
   Expected-owned and Permissive signing overrides. The device had zero
   registered descriptors. This is expected for the deliberately mixed,
   synthetic fixture and demonstrates that protected dispatch did not bypass
   the existing policy gate. Both overrides are test-fixture accommodations,
   not new anti-exfil defaults.
5. Explicit approval produced the animated `Nonce commitments` response.
6. Done displayed a readable full-screen completion message stating that step
   1 of 2 was complete, the transaction was not signed, and host-reveal
   continuation remained fail-closed.
7. A second stage-1 scan reached review and Back was selected instead of
   `Create commitments`. The device returned without displaying a response.
   The serial trace contains the retained-request scan measurement and no
   subsequent `response_phase=entry` marker.
8. With Anti-exfil enabled, the same PSBT encoded as a static base64/text QR was
   refused with `Protected signing required` before transaction review.
9. The same PSBT encoded as a five-part animated `crypto-psbt` UR was refused
   with the same message before transaction review.
10. Anti-exfil, Expected-owned, Permissive, and all other signing toggles were
    returned to off after testing.

The public fixture contains no funds. The temporary policy overrides were
needed only to enter review for its synthetic mixed-script inputs.

## Physical resource measurements

All values are the ESP-IDF 8-bit-capable heap counters reported by the device.
The task stack high-water value was 9,592 throughout response construction.

| Phase | Free heap | Largest block | Lifetime minimum |
| --- | ---: | ---: | ---: |
| Response entry | 25,492,623 | 23,068,672 | 22,763,963 |
| Signer records allocated | 25,445,487 | 23,068,672 | 22,763,963 |
| Signer complete | 25,445,487 | 23,068,672 | 22,763,963 |
| Signer records released | 25,491,763 | 23,068,672 | 22,763,963 |
| Response ready | 25,459,983 | 23,068,672 | 22,763,963 |
| Response destroy entry | 25,440,443 | 23,068,672 | 22,763,963 |
| Response destroyed | 25,442,379 | 23,068,672 | 22,763,963 |
| Viewer ready, before review teardown | 25,443,347 | 23,068,672 | 22,763,963 |
| Viewer destroy entry | 25,798,227 | 25,165,824 | 22,763,963 |
| Viewer destroyed | 25,848,151 | 25,165,824 | 22,763,963 |

The large signer records reduced free heap by 47,136 bytes and were released
before response encoding. After their release, free heap was within 860 bytes
of response entry. Destroying the animated viewer recovered 49,924 bytes. The
largest block remained healthy and the lifetime minimum did not decline during
the signer, response, display, or cleanup phases. This single lifecycle shows
successful cleanup; it does not replace a repeated stage-1 soak test.

The associated scan recorded 1,629 canonical CBOR bytes, 31,937 retained
request bytes, free heap 23,174,415 before the owned copy, 23,142,023 after the
copy, and 25,806,379 after camera shutdown. Its lifetime minimum before signing
was 22,766,383.

An accidental second complete stage-1 lifecycle provided a repeat cleanup
sample. It again used 47,136 bytes for the signer records, returned to within
860 bytes of response entry after releasing them, and recovered 50,220 bytes
when the viewer was destroyed. Its response-lifecycle lifetime minimum remained
22,818,491. These two clean lifecycles provide useful repeat evidence but are
not treated as a long soak test.

The ordinary-route regression pack is generated from the exact pinned
975-byte PSBT (`fa4ef7e7...d249`). The generator emits a static base64 QR and a
five-source-part, 200-byte-fragment `crypto-psbt` UR. Before rendering, the C
emitter feeds every UR source part through the independent decoder and requires
the reconstructed type and canonical CBOR bytes to match exactly.

## Verification

- Full host suite and collaboration corpus: passed.
- Anti-exfil signer: 19 passed, including complete-coverage preflight tests.
- Anti-exfil transport: 68 passed.
- Owned response bridge: 21 passed.
- Signed `wave_7b` firmware: built and flashed successfully.
- `kern.bin`: 1,970,176 bytes (`0x1e1000`), leaving 69% of the smallest
  application partition free.
- Flashed `kern.bin` SHA-256:
  `230b96a6d52f0b135f40509109a700e3f0376fc2d625b6b2a5da7bb835c08c38`.

## Next boundary

Stage 1 is complete at this checkpoint. A longer repeated lifecycle soak remains
useful during the experimental period but is no longer an untested cleanup
path. Stage 3 is the next workflow boundary. It requires its own explicit
approval screen and must preserve the agreed no-retry, exact-session,
exact-network, frozen-byte, and complete-local-slot constraints.
