# Kern anti-exfil M8 conformance runbook

Date: 2026-08-28
Status: prepared for review; no M8 live ceremony has been run
Scope: disposable Testnet3 data and the experimental AEXT v1 profile only

## Purpose and exit gate

Milestone 8 proves that SeedSigner and Kern independently complete the same
four-stage anti-exfil ceremony under both the Sparrow reference tooling and
the Sparrow GUI fork. A pass means equivalent verified PSBT outcomes, not
byte-identical signer signatures. Every applicable negative case must stop
before an opening or signature is released, and no protected failure may fall
back to ordinary PSBT signing.

The final matrix is:

| Cell | Coordinator | Signer | Status |
| --- | --- | --- | --- |
| M8-A | Sparrow reference CLI/tooling | SeedSigner | Not run for M8 |
| M8-B | Sparrow reference CLI/tooling | Kern | Not run for M8 |
| M8-C | Sparrow GUI fork | SeedSigner | Not run for M8 |
| M8-D | Sparrow GUI fork | Kern | Not run for M8 |

Earlier development checks are supporting evidence only. They do not turn a
cell above into a pass until its M8 receipt is recorded.

## Frozen implementation identities

- Kern source: `9bfda18` on `codex/kern-anti-exfil-m5`. The physical device is
  running the M6 stage-3 firmware built from `c360341`; the later `9bfda18`
  commit adds host-side mapping corpus material and does not change firmware.
- Flashed Kern `kern.bin` SHA-256:
  `b9aecca4dc0c894d8b7ff282c160aa3758cd4133c825247954ee15e59e307e1d`.
- Sparrow: `6c01dec2a861d4a1ade082b3cdc15ac4c6d8f602`.
- Drongo: `54365d7f09df956e0b3e8baf035b23920073bac3`.
- Lark submodule: `ddffe556f0d1ba6a138be3b362ce74219fed0710`.
- Profile: canonical `aext-v1`; temporary persisted `AEXT_V1` remains a
  read-only compatibility spelling.

Rebuild, reflash, or coordinator changes require a new artifact entry in the
evidence recorder. Do not silently carry results across identities.

## Safety and policy boundaries

1. Use only disposable public-test seeds and Testnet3 wallets. Stop on any
   Mainnet indication or funded production wallet.
2. Keep Kern's feature off by default and Sparrow's Kern policy `OPTIONAL`.
   `REQUIRED` remains unavailable for Kern until the other M8 gates pass.
3. Run the development Sparrow build with a dedicated `--dir`; never open the
   normal Sparrow home with this unreleased database migration.
4. Back up watch-only test-wallet metadata before import. No private seed is
   entered into Sparrow.
5. Preserve the exact coordinator session. Do not manufacture a fresh stage 3
   after a reveal-side failure and call it a retry.
6. Never broadcast an M8 transaction. Verification and PSBT reconstruction are
   the terminal outcomes.
7. Do not record mnemonics, passphrases, private keys, host randomness `rho`,
   or unredacted production wallet data. Hash artifacts and retain public
   test-vector bytes only where necessary.

## Evidence rules

Each case gets a receipt containing:

- case ID, matrix cell, UTC/local timestamps, operator, and result;
- exact source commits, build hashes, device/board, network, and profile;
- wallet/script class, input count, controlled-slot count, and descriptor
  registration state;
- session ID and frozen-PSBT digest as a short display prefix plus a full hash
  in machine evidence when safe;
- hashes and lengths of all four canonical messages and the final PSBT;
- expected and observed result/reason code;
- screenshots or photographs at decision points;
- redacted Sparrow and Kern logs;
- scan/sign/verification timings and Kern heap/stack markers;
- cleanup state, follow-up action, and reviewer disposition.

Artifact names use:

`<case-id>_<attempt>_<stage-or-kind>_<sha256-prefix>.<ext>`

An attempt is immutable once recorded. A rerun receives a new attempt number.
Do not overwrite failed evidence with a later pass.

## Execution order

### Gate 0 — review and preflight

- Independent review accepts this runbook, the evidence recorder, the Sparrow
  build receipt, and the physical playbook.
- Re-run the focused Sparrow/Drongo suites and Kern host/corpus checks.
- Verify commits, hashes, Testnet3, isolated Sparrow home, Kern setting state,
  and the selected disposable wallet before every session.
- Confirm camera exposure and QR framing without scanning a ceremony message.

### Gate 1 — honest baseline in all four cells

Run one single-input native P2WPKH ceremony in this order:

1. M8-D: Sparrow GUI to Kern (highest-risk new integration).
2. M8-C: Sparrow GUI to SeedSigner.
3. M8-B: Sparrow reference tooling to Kern.
4. M8-A: Sparrow reference tooling to SeedSigner.

If M8-D fails, run the applicable M8-C, M8-B, and M8-A baselines as control
cases before assigning the failure to Kern, Sparrow GUI, or their integration.
Do not continue the broader M8-D suite until the baseline failure is understood.

Compare logical signing context, controlled slots, frozen PSBT, verified
signature placement, and reconstructed PSBT outcome. Transport-specific bytes
are compared only within `aext-v1`.

### Gate 2 — positive coverage

| ID | Scenario | Required cells |
| --- | --- | --- |
| P01 | Single-input native P2WPKH | A, B, C, D |
| P02 | Multi-input native P2WPKH | A, B, C, D |
| P03 | P2SH-P2WPKH | A, B, C, D |
| P04 | Registered standard P2WSH multisig | A, B, C, D |
| P05 | Registered P2SH-P2WSH multisig | A, B, C, D |
| P06 | More than one locally controlled slot on one input | Applicable cells |
| P07 | More than one signature on a multisig input | Applicable cells |

Each positive pass requires all accepted protected signatures to verify and
the coordinator to reconstruct from its frozen PSBT without trusting returned
transaction state.

### Gate 3 — continuity and user control

| ID | Scenario | Pass condition |
| --- | --- | --- |
| C01 | Direct continuation | Exact session completes after distinct stage-1 and stage-3 approvals |
| C02 | Stateless continuation | Power cycle after message 2, reload the same seed, and complete the exact session |
| C03 | Camera interruption | Rescanning the exact QR resumes decoding without substituting a session |
| C04 | Cancel before openings | No stage-2 response or opening is emitted |
| C05 | Back at stage-3 review | No stage-4 response or signature is emitted |
| C06 | Failure after reveal | Sparrow retains the exact failed session and never silently creates fresh `rho` |
| C07 | Exact-session retry | Only the explicitly permitted same-session path resumes; otherwise restart clearly |

### Gate 4 — adversarial and fail-closed cases

Use generated/pinned artifacts, not hand-edited QR screenshots. Prefer headless
injection where visually indistinguishable mutations would make human results
ambiguous.

| ID | Mutation | Required boundary |
| --- | --- | --- |
| N01 | Wrong stage | Reject before review/signing |
| N02 | Wrong network | Reject before review/signing |
| N03 | Unknown, completed, stale, or substituted session | Coordinator rejects continuation |
| N04 | Changed frozen PSBT byte | Reject before opening/signature |
| N05 | Missing, extra, duplicated, or reordered slot | Reject atomically |
| N06 | Changed pubkey, path, or fingerprint | Reject atomically |
| N07 | Changed coordinator-supplied sighash | Local recomputation rejects |
| N08 | Commitment/reveal mismatch | Reject before signature |
| N09 | Substituted signer opening | Reject before signature |
| N10 | Missing/inconsistent UTXO | Reject before opening/signature |
| N11 | Broken redeem/witness script | Reject before opening/signature |
| N12 | Unsupported script, sighash, PSBT version, or Taproot metadata | Reject before opening/signature |
| N13 | Malformed/trailing/noncanonical CBOR, AEXT, or AEXB | Reject with no fallback |
| N14 | Ordinary PSBT while Kern protected signing is enabled | Refuse before ordinary signing review |
| N15 | Ordinary returned PSBT under coordinator-required policy | Coordinator rejects it |

### Gate 5 — resource and repetition

- Run at least 20 consecutive complete Kern ceremonies with the same case
  class and fresh valid sessions.
- Record free heap, largest block, lifetime minimum, stack high-water mark,
  decode time, opening/signing time, response construction, viewer display,
  and cleanup for every repetition.
- `min_free` is a lifetime watermark and need not rise. `heap_before` and
  post-cleanup heap must stabilize rather than decline continually.
- Repeat representative scans at 150-byte and 200-byte UR fragments and record
  camera distance, exposure, ambient light, and completion time.

### Gate 6 — controlled Kern `REQUIRED` promotion

Only after Gates 0–5 pass and are reviewed:

1. Change the explicit capability registry so Kern may select `REQUIRED`.
2. Re-run registry/persistence/policy tests and the M8-D baseline.
3. Prove ordinary returned PSBT rejection and downgrade resistance.
4. Review and commit the capability change separately.

This staged change avoids treating model identity as proof of conformance.

## Immediate stop conditions

Stop the case, retain evidence, and do not retry with a new session if any of
the following occurs:

- Mainnet or an unexpected wallet/seed is shown;
- a protected request reaches ordinary signing fallback;
- any response appears after Back, Cancel, or a validation failure;
- stage 3 proceeds without Sparrow's exact durable stage-1 session;
- Sparrow merges signer-returned PSBT state instead of exact signatures;
- signature/opening/slot verification is incomplete or ambiguous;
- heap or stack behavior trends toward exhaustion;
- artifact identity differs from the session header;
- the operator cannot distinguish the expected outcome.

## M8 completion rule

M8 completes only when all applicable P, C, and N cases have immutable receipts,
the four matrix cells pass, the soak data is stable, Kern's controlled
`REQUIRED` promotion passes its final regression, and independent review finds
no unresolved blocking issue. Until then, Kern stays experimental, Testnet3
only, and unavailable for `REQUIRED` in Sparrow.
