# Kern M6 stage-3 workflow checkpoint

Date: 2026-08-28

Board: Waveshare ESP32-P4-WIFI6-Touch-LCD-7B (`wave_7b`)

This checkpoint connects a protected stage-3 host-reveal request to Kern's
existing transaction review, a second independent approval, and an animated
`x-btc-anti-exfil` stage-4 protected-signature response. It does not export an
ordinary signed PSBT.

## Workflow boundary

- A recognized protected UR is consumed without fallback to ordinary PSBT,
  bytes, or text dispatch.
- The experimental Anti-exfil signing setting must be enabled, Kern must be on
  testnet, and the request must identify a public test network.
- Stage 3 passes the same non-signing authoritative preflight as stage 1. Kern
  hashes the exact retained PSBT bytes, independently enumerates every locally
  controlled signing slot, derives every controlled key, and recomputes every
  sighash.
- Preflight additionally rederives each deterministic signer opening and
  requires it to match the opening accepted by the coordinator. It hashes each
  host reveal and requires it to open the corresponding stage-1 commitment.
- The complete ordered locally controlled slot set is required. Subsets,
  reordered slots, changed pubkeys or sighashes, changed PSBT bytes,
  substituted openings, and reveals that do not open their commitments fail
  before transaction review.
- The stage-3 transaction is reviewed again through Kern's existing PSBT review
  and signing-policy UI. Stage 1 approval cannot authorize stage 3.
- Only the explicit `Create signatures` action invokes the signer. The signer
  repeats all validation at execution time before producing signatures.
- Back/cancel cannot produce a response. After any attempted-signing failure,
  the retained request is destroyed and the UI requires a new ceremony; Kern
  does not retry, change stages, or substitute a session.
- The stage-4 response preserves the request's exact network and session ID and
  is displayed as an animated protected UR titled `Protected signatures`.
- Done destroys the response viewer and reports `Step 2 of 2 complete`, states
  that no ordinary signed PSBT was exported, and instructs that any retry begin
  as a new ceremony.

## Stateless session boundary

Kern intentionally does not cache stage-1 state. A stage-3 request remains
valid after a reboot and seed reload because Kern rederives the signer openings
from the seed, message hashes, and host commitments. Consequently, Kern cannot
compare the stage-3 session ID with a remembered stage-1 scan. Exact session-ID
continuity across the two scans is a coordinator responsibility: Sparrow must
look up the durable stage-1 record by the full 32-byte session ID and refuse a
missing, completed, replaced, or retried session.

On the device, the full session ID is structurally validated, retained
unchanged, and echoed in stage 4. The review shows a short session prefix for
human orientation; that prefix is not an authentication check. Cryptographic
round continuity is independently established by the rederived signer
openings, host-reveal commitments, exact frozen PSBT, network identity, and
complete ordered slot set.

## Physical acceptance playbook

Use only the public test seed:

`model ensure search plunge galaxy firm exclude brain satoshi meadow cable roast`

The synthetic fixture may require Kern's existing Expected-owned and
Permissive overrides. These are fixture accommodations, not anti-exfil
defaults.

1. Flash this checkpoint's signed `wave_7b` firmware.
2. Load the public test seed and keep Kern on Testnet.
3. Enable Anti-exfil signing, Expected-owned signing, and Permissive signing.
4. Open
   `main/core/test/physical_qr_output/fragment-200/stage-3/index.html`.
5. Scan the animated QR. Expected: a protected transaction review headed
   `Step 2 of 2`, a session prefix beginning with `7a`, and a warning that the
   second approval creates protected signatures for every locally controlled
   slot.
6. Press Back. Expected: Kern returns without displaying a protected response;
   the serial log contains no new `response_phase=entry` marker.
7. Scan the same stage-3 QR again, review the transaction, and press
   `Create signatures`.
8. Expected: an animated QR titled `Protected signatures`. Stop here while the
   serial measurement lines are captured.
9. Press Done. Expected: `Step 2 of 2 complete`, an explicit statement that no
   ordinary signed PSBT was exported, and a statement that any retry must start
   a new ceremony.
10. Return Anti-exfil signing, Expected-owned signing, Permissive signing, and
    all other signing toggles to off.

The canonical stage-3 fixture is sufficient for the on-device UI and lifecycle
gate. The headless suite supplies precise negative evidence for incomplete
coverage, changed frozen PSBT bytes, substituted signer openings, and invalid
host reveals without asking a human tester to distinguish nearly identical QR
animations.

## Verification before physical testing

- Full host suite and collaboration corpus: passed.
- Anti-exfil signer: 22 passed, including three stage-3 preflight checks.
- Anti-exfil transport: 68 passed.
- Owned response bridge: 21 passed.
- Desktop simulator: built and linked successfully for `wave_7b`.
- Signed ESP-IDF 6.0.2 `wave_7b` firmware: built successfully.
- `kern.bin`: 1,970,176 bytes (`0x1e1000`), leaving 69% of the smallest
  application partition free.
- Pre-flash `kern.bin` SHA-256:
  `b9aecca4dc0c894d8b7ff282c160aa3758cd4133c825247954ee15e59e307e1d`.

## Physical results

The signed firmware was flashed to the physical Kern on COM6. Flash contents
passed esptool's hash verification and the device booted normally. The
canonical stage-3/200-byte fixture was then exercised with the public test seed.
It was deliberately scanned without first running stage 1 on this boot. That
is an acceptance check for stateless resumption, not a missing workflow step:
the coordinator fixture supplies the durable prior-round state while Kern
rederives the signer-side continuation from its seed and the exact request.

Observed results:

1. Stage 3 passed its non-signing cryptographic and authoritative preflight and
   entered the existing transaction review.
2. The review visibly identified `Step 2 of 2`, showed the expected session
   prefix beginning with `7a`, required review of the transaction again, and
   offered `Create signatures` rather than the ordinary `Sign` action.
3. Kern's Expected-owned and unproven-fee warnings remained visible. The
   synthetic fixture required Expected-owned and Permissive overrides, as it
   did during the stage-1 test.
4. Back was selected on the first review. Kern returned without showing a
   response. The serial trace contains the owned-request scan measurement and
   no subsequent `response_phase=entry` marker.
5. The fixture was scanned again and `Create signatures` was explicitly
   selected. Kern displayed an animated `Protected signatures` QR.
6. Done destroyed the viewer and displayed a readable `Step 2 of 2 complete`
   checkpoint with an OK button.

The ordinary-PSBT protected-mode regressions were already exercised during the
stage-1 physical checkpoint: both a static base64/text PSBT and an animated
`crypto-psbt` UR were refused before review while Anti-exfil signing was
enabled. Together with the stage-3 Back result above, the three M6 no-fallback
and no-response regressions are physically covered.

The associated scan retained 32,262 bytes for a 1,954-byte canonical CBOR
request. Free heap was 23,255,427 bytes before the owned copy, 23,222,715 after
the copy, and 25,804,531 after camera shutdown. Its lifetime minimum before
signing was 22,804,559 bytes.

All values below are ESP-IDF 8-bit-capable heap counters reported by the
physical device. The task stack high-water value was 9,452 throughout response
construction.

| Phase | Free heap | Largest block | Lifetime minimum |
| --- | ---: | ---: | ---: |
| Response entry | 25,490,535 | 23,068,672 | 22,804,559 |
| Signer records allocated | 25,443,399 | 23,068,672 | 22,804,559 |
| Signer complete | 25,443,399 | 23,068,672 | 22,804,559 |
| Signer records released | 25,489,355 | 23,068,672 | 22,804,559 |
| Response ready | 25,457,199 | 23,068,672 | 22,804,559 |
| Response destroy entry | 25,435,411 | 23,068,672 | 22,804,559 |
| Response destroyed | 25,438,043 | 23,068,672 | 22,804,559 |
| Viewer ready, before review teardown | 25,439,023 | 23,068,672 | 22,804,559 |
| Viewer destroy entry | 25,794,151 | 25,165,824 | 22,804,559 |
| Viewer destroyed | 25,846,615 | 25,165,824 | 22,804,559 |

The signer work records used 47,136 bytes and were released before response
encoding. After their release, free heap was within 1,180 bytes of response
entry. Destroying the animated viewer recovered 52,464 bytes. The largest
block remained healthy and the lifetime minimum did not decline during signing,
response construction, display, or cleanup. This is one complete physical
stage-3 lifecycle, not a long soak test.
