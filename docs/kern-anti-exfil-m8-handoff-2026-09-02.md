# Kern anti-exfil handoff — M8 after mixed-device broadcast

Date: 2026-09-02
Last updated: 2026-09-03
Scope: experimental AEXT v1, disposable Testnet3 data only
Status: M8-X01 mixed-device transaction broadcast and confirmed, with devices
cleaned up; final independent review is pending

## Resume here

1. Read this handoff, then the conformance runbook and evidence recorder linked
   below. Treat the recorder and immutable per-attempt receipts as authoritative
   if a summary here ever disagrees with them.
2. Re-check the preserved confirmation evidence for Testnet3 transaction
   `6854c031978839983e6cb2bcf064a432f126fbbbdbec7c969e9202e554442496`.
   It was broadcast once, was initially unconfirmed, and later confirmed in
   block `5,128,025`. Do not modify the immutable pre-broadcast or initial
   unconfirmed evidence.
3. Verify the post-broadcast receipt, final closed Sparrow log, and cleanup
   record described below. Obtain independent review before treating Part G as
   accepted evidence.
4. Plan the next bounded M8 case from the open-work list. Do not promote Kern to
   `REQUIRED` or relax the Testnet3-only boundary yet.

Recommended next-session order:

1. independent review of the post-broadcast receipt and confirmation artifact;
2. obtain Joe's missing review document and reproduce or rule out the reported
   quarantine-egress gap;
3. if reproduced, fix and regression-test quarantine containment before making
   further Required-policy claims;
4. implement and physically test the seamless Kern Step 1 → Step 2 transition
   as a separate lifecycle/UI change;
5. send the checkpoint update to BitcoinShooter; then
6. resume the remaining M8 matrix with a newly reviewed playbook/checkpoint.

Explorer URL:
`https://mempool.space/testnet/tx/6854c031978839983e6cb2bcf064a432f126fbbbdbec7c969e9202e554442496`

## Frozen implementation identities

| Component | Identity |
| --- | --- |
| Kern firmware source | `5180dbb603e01e33698bb388a400f92bff722d4c` |
| Kern flashed application SHA-256 | `e7df2b55d7c476a0ff67c06aebcd55e7fdf3b7027de98b58022828e85e1f893a` |
| Kern evidence/docs branch before this handoff | `codex/kern-anti-exfil-m5` at `1235d66dedd15899188262c4aa34e297f48c2cff` |
| Sparrow | `a53d9e166bb480df9f53f0bc4399545a4a1b5be8` on `codex/kern-anti-exfil-m7` |
| Sparrow M8 build | BR-2026-09-02-01; JAR SHA-256 `fae73cc86b8be4dcc5c0bbcb2669f7a36f59361e9e0d9963fe5517398a1e9f53` |
| Drongo | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| SeedSigner app | `214793df4f51466179b792420921b8cdd8d0c1ac` |
| SeedSignerOS | `0bf1dc92519906c7db265055abfb07e0ee344342` |
| SeedSigner Buildroot | `bf2a2858aa675a14b60f1f9142c65b32652609c1` |
| SeedSigner image SHA-256 | `adc2b58ae9dd57e884ec33b0e39ebf608ee8cc468d3fa7c563a1f1f808550fb3` |
| Profile | canonical `aext-v1`; temporary `AEXT_V1` is read-only compatibility input |

Any coordinator rebuild, firmware reflash, or device-image change requires a
new receipt. Do not carry a prior physical pass across changed identities.

## What is complete and independently accepted

- M1–M7 implementation checkpoints are complete. M7 established the shared,
  canonical signer/profile representation and the M8 coordinator foundations.
- M8-D/P01 attempt 02: Sparrow GUI to Kern, one-input native P2WPKH.
- M8-C/P01 attempt 01: Sparrow GUI to SeedSigner, one-input native P2WPKH.
- M8-C/P02 and M8-D/P02: the same frozen two-input native P2WPKH PSBT,
  independently completed by SeedSigner and Kern. All four signatures were
  independently verified and both results recover the exact frozen PSBT.
- M8-X01 Parts E/F attempt 02: one-input native P2WSH 2-of-3, Kern first and
  SeedSigner `b4899a09` second. Sparrow retained Kern's protected signature,
  added SeedSigner's, excluded reserve signer `2a0726f2`, and finalized at
  exactly 2-of-3. KimiK3 independently accepted all pre-broadcast evidence and
  authorized one Testnet3 broadcast.

Recorded safe failures remain evidence and must not be overwritten:

- M8-D/P01 attempt 01 exposed direct PSBT-v0 prevout-member use. Kern
  `5180dbb` fixed this by using authoritative libwally getters and added the
  exact live regression fixture.
- M8-X01 attempt 01 exposed coordinator signer-selection filtering in a mixed
  Required/Optional wallet. Sparrow `a53d9e1` fixed chooser eligibility while
  retaining Required provenance enforcement. Kern rejected the incorrectly
  addressed request before emitting M2.

## M8-X01 broadcast checkpoint

The transaction spends fixture outpoint
`61a05816882fb79f5142137d5514cbcbd76f46772977325b7bfd0f493b9079da:2`
with value 36,369 sats. It pays 11,111 sats to the recorded recipient, returns
25,055 sats to the multisig wallet, and pays a 203-sat fee.

| Item | Value |
| --- | --- |
| txid | `6854c031978839983e6cb2bcf064a432f126fbbbdbec7c969e9202e554442496` |
| wtxid | `9950542609e55c1bf719b5926e221431e797a2fe8f23e2792fa6560c8c42c9fb` |
| Raw transaction | 392 bytes; SHA-256 `fa7ef396a88d3e8f6684a51e876802a5d2ad130ff8f9f7894e39822803dfb052` |
| Broadcast count | Exactly one |
| Initial Sparrow state | `Unconfirmed`; signed by `SeedSigner 2, Kern` |
| Initial public API state | `confirmed: false` |
| Screenshot | `run/m8-evidence/M8-X01-attempt-02-broadcast-unconfirmed.png`; 102,895 bytes; SHA-256 `7bc9573ab867d5892a61a8dd2365dfe02fcff43343ca2f1a55bb4027546bd891` |
| Public response | `run/m8-evidence/M8-X01-attempt-02-mempool-space-unconfirmed.json`; 2,192 bytes; SHA-256 `ec5bb24cc07fe0abc0f22d0e95ee1b51b0a3d08b4eb35d4fcd89bafb632386ac` |
| Screenshot capture | 2026-09-02 21:36 EDT, shortly after broadcast |
| Confirmation | Block `5,128,025`; block time `2026-09-03T03:50:12Z` |
| Confirmation response | `run/m8-evidence/M8-X01-attempt-02-mempool-space-confirmed.json`; 2,319 bytes; SHA-256 `1abd88adec2b797556be7d3f704c4f1581123b4534306c8aba39008f55683e6a` |

The immutable pre-broadcast receipt is
`run/m8-evidence/M8-X01-attempt-02-receipt.md` (10,438 bytes, SHA-256
`f8b576923e61ac73bbcdfabc54b2a2fefd144206bac3bdb22dbe554065596d59`).
Do not edit it. Record broadcast and confirmation evidence in the separate
post-broadcast receipt, `run/m8-evidence/M8-X01-attempt-02-broadcast-receipt.md`
(3,810 bytes, SHA-256
`235bc43fc48e13002ce5f140bb560da6aaf4ffac9ad165d10b26058ce7444f9e`).

## Important M8-X01 evidence

- Pristine PSBT: 1,607 bytes,
  `483294e990e74f412a31719b765147ea20c403bf8b2200c4722bb4c7fb0edec0`.
- Kern intermediate PSBT: 1,715 bytes,
  `a711f49d080797333f4b05a025183509e1121d239597da67c5db0a687588d217`.
- Final signed/finalized PSBT: 1,753 bytes,
  `e36d7b26fe89ce83668e461beb5d57dd365aa85ca22f1cb4ff5213bcec549d54`.
- Final transaction file: SHA-256
  `75dcc615dad2a815ca92d2e425a5f67c43094ab0af3dc054ad3045dc319bf200`.
- Kern complete AEXS session ID:
  `2a38f80bbb2a57494d9999167853f96bd5a88a57315bdee3f5ab2ddec2a7e152`.
- SeedSigner complete AEXS session ID:
  `7644fbddbded6ae231979ce6fd2fed55552d43c6f7b3466fdd7ab830a969b1d0`.
- Independent verifier: `run/m8-evidence/verify_m8_x01_mixed.py`; its
  preserved output ends with `validation=PASS`.
- Kern serial evidence:
  `run/m8-evidence/M8-X01-attempt-02-kern-serial.log`, 9,672 bytes,
  SHA-256 `a4499d9ea5fdf2caac2072ff952d181233b38200a74f102521c9acf5dd71d40c`.
- Final closed Sparrow log:
  `run/m8-evidence/M8-X01-attempt-02-closed-sparrow.log`, 2,782 bytes,
  SHA-256 `4a8a6d04c985698c141995d4207efe841391bccbacfa538ec42f92574dadcfaf`.

See the immutable receipt for the complete 18-artifact hash inventory.

## Open M8 work

The mixed transaction is supporting evidence for P04/P07. It does not close
the entire conformance matrix. Remaining work includes:

- M8-A and M8-B reference-tool P01 baselines.
- P03 P2SH-P2WPKH in A/B/C/D.
- Clean per-cell P04 standard P2WSH and P05 P2SH-P2WSH coverage.
- Applicable P06 (multiple locally controlled slots on one input) and P07
  (multiple signatures on a multisig input) receipts beyond the mixed-device
  supporting case.
- C01–C07 continuity and user-control cases, including power-cycle stateless
  continuation, cancel/back, failure-after-reveal, and exact-session behavior.
- N01–N15 adversarial/fail-closed matrix using generated or pinned artifacts.
- At least 20 consecutive complete Kern ceremonies with fresh sessions and
  per-cycle heap, largest-block, minimum-free, stack, timing, display, and
  cleanup evidence; representative 150- and 200-byte fragment scans.
- Gate 6 only after all earlier gates pass and are reviewed: enable Kern
  `REQUIRED` capability in a separate commit, then rerun policy/persistence,
  baseline, downgrade, and ordinary-PSBT refusal checks.

Kern remains experimental, disabled by default, Testnet3-only, and Optional in
Sparrow until those gates pass.

## Priority review lead — reported containment gap

Joe reported an independently found, medium-severity containment gap in the
Sparrow quarantine UI. This severity and finding are the reporter's assessment
and have not yet been independently reproduced in this workspace. The review
document mentioned in his message was not attached or otherwise available.

Reported behavior: when a tab is quarantined read-only because a signature has
not earned provenance proof, Finalize and Broadcast are disabled, but PSBT
egress may remain available through Save, Copy, Show PSBT, payjoin, and the
file/paste import path. If confirmed, an unproven signature could leave the
quarantined tab despite the visible read-only state.

Start the next conversation by:

1. asking Joe for the missing independent-review document and exact commit/build
   identities used for reproduction;
2. reproducing every named route against the frozen Sparrow identity, while
   preserving a safe-failure receipt and using only disposable Testnet data;
3. mapping every route by which the quarantined transaction or PSBT can be
   copied, saved, shown, passed to payjoin, imported, or transferred to another
   tab/window;
4. defining one quarantine capability predicate and enforcing it at the data-
   egress/action layer, rather than relying only on disabled buttons;
5. adding regression tests for each route, including indirect file/paste and
   cross-tab paths, then obtaining independent review before resuming broader
   M8 execution.

Treat a reproduced escape as a containment blocker for claims about Required
policy enforcement. It does not invalidate the already verified cryptographic
anti-exfil tuples, but it may invalidate coordinator quarantine completeness.

## BitcoinShooter collaboration checkpoint

Follow up in
`https://github.com/FractalEncrypt/seedsigner-anti-exfil-review/issues/1`
after the post-broadcast evidence is independently accepted and the reported
containment gap has at least been reproduced or ruled out. The update should:

- report the accepted Kern and SeedSigner P01/P02 results plus the confirmed
  mixed-device 2-of-3 transaction and link its public Testnet3 txid;
- distinguish cryptographic conformance, coordinator carriage, and quarantine
  containment claims;
- summarize any Sparrow containment fix and its route-level regression tests;
- mention the mixed-policy signer-selection correction and Kern's PSBT-v0
  getter correction without overstating them as profile changes;
- ask for Shooter's latest branch/vector identities and any corresponding
  quarantine/export semantics before freezing further shared profile policy.

Do not wait until all of M8 is complete: this post-broadcast/security-triage
checkpoint is the appropriate next sync because it contains concrete new
interoperability evidence and a potentially relevant coordinator-policy issue.

## UX observations to retain

- Next-session UX task: streamline Kern's stage transition. After message 2,
  Kern currently returns to Home with no indication that the ceremony must
  continue; the user must infer that they should tap Scan for stage 2.
  Implement an explicit, seamless transition from “Step 1 of 2 complete” into
  the stage-3 scanner / “Step 2 of 2” flow, matching SeedSigner's continuous
  ceremony. Keep the retained-byte, independent-approval, no-retry, cleanup,
  and fail-closed boundaries intact, and review/test this as a discrete UI and
  lifecycle change.
- SeedSigner requested the already registered multisig descriptor again during
  M8-X01. Closing and redisplaying M1 produced the exact same package hash, so
  no session substitution occurred.
- Future safe-failure receipts should save raw M1 as a separate artifact even
  when a phase-0 AEXS already proves the request contents.
- Earlier Kern completion wording was understandable only after explanation;
  avoid combining “scan this response” and “session complete” in a way that
  makes the action order ambiguous.

## Working locations and launch command

- Kern checkout: `C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\kern-m1`
- Sparrow checkout: `C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\feasibility-sources\our-sparrow`
- Immutable evidence: `C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-evidence`
- M8-X01 Sparrow home: `C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-x01-sparrow-home`

Launch the accepted Sparrow build from PowerShell only when a reviewed next
case requires it:

```powershell
cd "C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\feasibility-sources\our-sparrow"
.\gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 run --args="--dir C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-x01-sparrow-home --network testnet"
```

Do not launch this unreleased build against the normal Sparrow home.

## Primary documents

- `docs/kern-anti-exfil-m8-conformance-runbook-2026-08-28.md`
- `docs/kern-anti-exfil-m8-evidence-recorder-2026-08-28.md`
- `docs/kern-anti-exfil-m8-first-physical-playbook-2026-08-28.md`
- `docs/kern-anti-exfil-m8-p02-mixed-multisig-playbook-2026-09-02.md`
- `docs/kern-anti-exfil-m8-x01-preparation-checkpoint-2026-09-02.md`
- `run/m8-evidence/M8-X01-attempt-02-receipt.md`
- `run/m8-evidence/M8-X01-attempt-02-broadcast-receipt.md`

## Safety state at handoff

- Test funds only; the sole authorized Testnet3 broadcast has occurred.
- Normal Sparrow data was never opened by the development build.
- Final Sparrow/device cleanup: complete. Sparrow is closed; Kern has every
  signing toggle Off and its seed unloaded; SeedSigner anti-exfil is Disabled
  and all seeds are discarded.
- Confirmation: block `5,128,025`, block time `2026-09-03T03:50:12Z`;
  independent post-broadcast evidence review pending.
