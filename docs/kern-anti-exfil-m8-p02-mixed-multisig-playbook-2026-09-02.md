# M8 P02 and mixed-device multisig physical playbook

Date: 2026-09-02
Status: P02 pair independently accepted; M8-X01 Part D authorized, not started
Scope: disposable Testnet3 funds and `aext-v1` only

## Purpose

This cycle runs:

1. M8-C/P02: Sparrow GUI to SeedSigner, two-input native P2WPKH;
2. M8-D/P02: Sparrow GUI to Kern over the byte-identical unsigned PSBT; and
3. M8-X01: one native P2WSH 2-of-3 transaction signed first by Kern and then
   by SeedSigner, followed by one deliberate Testnet3 broadcast.

M8-X01 is additional mixed-device integration evidence. It does not replace
the clean per-cell P04 matrix cases, and it does not prove P06 because each
device controls only one slot.

## Frozen setup transaction

Fixture transaction:
`61a05816882fb79f5142137d5514cbcbd76f46772977325b7bfd0f493b9079da`.

It is confirmed in Testnet3 block 5,127,974 and spent the previous 180,406-sat
P2WPKH output. Its 113-sat fee and four outputs balance exactly:

| vout | Script | Value | Intended use |
| ---: | --- | ---: | --- |
| 0 | P2WSH | 11,111 sats | funded multisig fixture |
| 1 | P2WPKH | 111,444 sats | `0fb882ff` P02 input |
| 2 | P2WSH | 36,369 sats | funded multisig fixture |
| 3 | P2WPKH | 21,369 sats | `0fb882ff` P02 input |

This transaction is fixture preparation, not an M8 protocol case. Do not
spend these outputs outside this playbook.

## Frozen identities

- Sparrow: `a53d9e166bb480df9f53f0bc4399545a4a1b5be8`
- Drongo: `54365d7f09df956e0b3e8baf035b23920073bac3`
- Kern: DR-KERN-02
- SeedSigner: DR-SEEDSIGNER-01
- Single-sig account: `0fb882ff`, `m/84'/1'/0'`
- Multisig: native SegWit 2-of-3, `wsh(sortedmulti(...))`, account paths
  `m/48'/1'/0'/2'`, descriptor checksum `47y6998m`
- M8-X01 roles:
  - Kern: `0fb882ff`
  - SeedSigner: `b4899a09`
  - unused reserve cosigner: `2a0726f2`

Changing a role, device image, coordinator build, descriptor, transaction, or
network requires a new receipt. Never load private seed material into Sparrow.

## Universal stop conditions

Stop without approving, scanning another stage, or broadcasting if:

- any screen says Mainnet, Signet, Testnet4, or Regtest;
- fingerprint, derivation, descriptor checksum, input, output, change, or fee
  differs from the frozen case;
- a stage is mislabeled or an ordinary-PSBT flow replaces protected signing;
- any M2/M4 is emitted before its independent device approval;
- Sparrow does not log the exact outgoing/incoming stage receipt;
- Sparrow substitutes a new session after an error;
- a signer returns an ordinary signed PSBT instead of protected M2/M4; or
- Broadcast is enabled before the required signatures verify.

## Part A — create the shared P02 unsigned PSBT

Use the accepted M8-C SeedSigner Sparrow profile. Confirm the wallet is the
native-P2WPKH `0fb882ff` wallet and now shows both fixture UTXOs.

1. In **UTXOs**, select both `61a05816...:1` and `61a05816...:3` using coin
   control. No other input is permitted.
2. Create a Testnet3 self-spend to a fresh receive address from the same
   account. Use 11,111 sats for the recipient and let Sparrow calculate change
   and fee.
3. Before starting any signer flow, verify total input is 132,813 sats and
   record recipient address, change, fee rate, absolute fee, locktime, and
   unsigned transaction ID.
4. Save the unsigned PSBT as
   `run/m8-evidence/M8-P02-shared-unsigned.psbt`.
5. Hash it and make a read-only evidence copy before either device signs.

The exact saved file—not a recreated transaction—is the shared input to both
P02 ceremonies. Neither P02 result is broadcast.

## Part B — M8-C/P02 with SeedSigner

1. Use the dedicated M8-C Sparrow profile. Set its SeedSigner keystore policy
   to `Required`.
2. Boot DR-SEEDSIGNER-01, confirm Version shows fork `FractalEncrypt` and
   commit `214793d`, choose Testnet, set anti-exfil to `Required`, and load
   `0fb882ff`.
3. Open the frozen shared unsigned PSBT. Confirm two inputs and two controlled
   slots before signing.
4. Start the Sparrow log monitor, then create M1. Freeze its stage, length,
   SHA-256, session ID, and frozen-PSBT digest before scanning.
5. On SeedSigner, review both inputs, recipient, change, and fee. Approve
   **Create nonce commitment** only after every value matches.
6. Scan M2 into Sparrow before dismissing SeedSigner's response QR. Freeze M2
   and the newly emitted M3 receipts.
7. Choose **Scan host reveal**, scan M3, and repeat the complete review.
   Independently approve **Approve protected signature**.
8. Scan M4 into Sparrow. Require two verified signatures, one for each input,
   both belonging to `0fb882ff`.
9. Save the reconstructed result as
   `run/m8-evidence/M8-C-P02-attempt-01-signed.psbt`. Do not broadcast.
10. Close Sparrow, disable SeedSigner anti-exfil, discard the seed, and hash
    the closed log, AEXS/AEXJ, signed PSBT, and screenshots.

## Part C — M8-D/P02 with Kern

1. Relaunch the accepted M8-D Sparrow profile and open the byte-identical
   `M8-P02-shared-unsigned.psbt`; do not reuse the SeedSigner-signed result.
2. Confirm the Kern keystore is `0fb882ff`, `m/84'/1'/0'`, with protected
   signing `Optional` in Sparrow.
3. Load `0fb882ff` in DR-KERN-02; Testnet and Kern anti-exfil On; ordinary
   signing overrides Off.
4. Start the Kern serial and Sparrow log monitors before creating M1.
5. Complete the two independently approved Kern stages exactly as in
   M8-D/P01. Both stages must show the same two inputs, recipient, change, and
   fee as Part B.
6. Sparrow must verify two Kern signatures and reconstruct from the same
   frozen PSBT. Save
   `run/m8-evidence/M8-D-P02-attempt-01-signed.psbt`; do not broadcast.
7. Close Sparrow, return every Kern signing toggle Off, and freeze all logs,
   session files, PSBTs, screenshots, and resource measurements.

P02 comparison requires the same frozen unsigned PSBT, the same two ordered
controlled slots and sighashes, equivalent signature placement, and valid
reconstruction. ECDSA signatures and transport packages need not be
byte-identical because each ceremony uses fresh host entropy and its own
session.

## Review checkpoint before M8-X01

Do not start the mixed multisig transaction until an independent reviewer has
accepted both P02 receipts and the shared-PSBT comparison. This prevents a
mixed-device success from obscuring a per-device multi-input defect.

Gate satisfied: KimiK3 accepted both P02 receipts, the shared-fixture
comparison, and the session-ID erratum in its review of Kern `f259fda`.
M8-X01 preparation is authorized. Part G broadcast authorization remains a
separate later gate.

Attempt-01 addendum: BR-2026-09-01-04 filtered the Optional Kern signer from
the chooser because the wallet contained a Required reserve SeedSigner. Kern
correctly rejected the resulting SeedSigner-addressed M1 before M2. KimiK3
accepted this safe failure, Sparrow's narrow `a53d9e1` chooser fix, and
BR-2026-09-02-01. Attempt 02 is authorized with a fresh session, the exact
policy table below, explicit `Kern (0fb882ff)` selection, and the QR-header
stop gate in Part E. Do not reuse attempt-01 coordinator state.

## Part D — prepare M8-X01 native-P2WSH transaction

Close every normal or M8 Sparrow window. Use this new, dedicated profile only:

`C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-x01-sparrow-home`

Launch it from PowerShell with:

```powershell
cd "C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\feasibility-sources\our-sparrow"
.\gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 run --args="--dir C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-x01-sparrow-home --network testnet"
```

Import the exact 2-of-3 descriptor into that profile. Do not create a
transaction until the wallet has synchronized, the sole intended input is
visible, and every item below has been checked.
Verify all three fingerprints and `m/48'/1'/0'/2'` origins. Assign the models
and policies exactly as follows:

| Fingerprint | Sparrow model | Policy | Role |
| --- | --- | --- | --- |
| `0fb882ff` | Kern | Optional | first protected signature |
| `b4899a09` | SeedSigner | Required | second protected signature |
| `2a0726f2` | SeedSigner | Required | reserve; do not sign M8-X01 |

Register the exact wallet descriptor on both physical devices before signing.
Use exactly `61a05816...:2`, the 36,369-sat P2WSH fixture output, as the input
so the case isolates two signatures on one input and retains vout 0 for later
testing. Create a self-spend back to this same multisig wallet, record every
value, and save/hash the pristine unsigned PSBT before either signer acts.

## Part E — Kern signs first

1. Load `0fb882ff` in Kern and verify the registered descriptor, Testnet, and
   anti-exfil On.
2. Start both monitors and initiate protected signing. In Sparrow's signer
   chooser, explicitly select `Kern (0fb882ff)`. Before scanning anything,
   require the QR dialog header to say `scan this commitment with Kern`; if it
   names SeedSigner or any other device, close the dialog and stop the attempt.
3. Complete M1 through M4 with independent stage-1 and stage-3 approvals.
4. Sparrow must show exactly one verified Kern partial signature and must not
   consider the 2-of-3 transaction complete.
5. Save/hash the intermediate PSBT. Do not broadcast and do not recreate the
   transaction.

## Part F — SeedSigner signs second

1. Keep Sparrow on the exact intermediate transaction. Load `b4899a09` in
   DR-SEEDSIGNER-01, confirm Testnet and anti-exfil `Required`, and verify the
   same registered descriptor.
2. Initiate protected signing and explicitly select
   `SeedSigner 2 (b4899a09)` (or the equivalent label with that exact
   fingerprint) in Sparrow's signer chooser. Before scanning anything, require
   the QR dialog header to name SeedSigner. Its fresh M1 legitimately freezes
   a PSBT already containing Kern's partial signature.
3. Complete M1 through M4 with both independent SeedSigner approvals.
4. Sparrow must retain the Kern signature, verify the new SeedSigner
   opening/S2C/ECDSA evidence, show exactly two signatures on the input, and
   finalize the 2-of-3 transaction without the reserve signer.
5. Save/hash the final signed PSBT and screenshot. Verify input, output,
   change, fee, both signer fingerprints, signature count, and final txid.

## Part G — deliberate Testnet3 broadcast gate

Broadcast only after all Part E/F evidence has been copied and hashed and a
review confirms:

- Testnet3 network identity;
- the pristine, intermediate, and final PSBT relationship;
- one protected signature from Kern and one from SeedSigner;
- both ECDSA signatures and protected provenance verify;
- no signature or output was added, removed, or substituted unexpectedly;
- the transaction is final with exactly the intended 2-of-3 threshold; and
- the displayed recipient, change, and fee match both device reviews.

After explicit operator confirmation, press Broadcast once. Record Sparrow's
result, txid, first-seen time, explorer URL, and eventual confirmation height.
Then close Sparrow, return both devices' anti-exfil settings to Disabled/Off,
discard loaded seeds, and preserve the isolated profile.

## Exit state

A successful cycle advances M8-C/P02 and M8-D/P02 after their independent
review and records M8-X01 as supporting mixed-device P04/P07 evidence. P03,
clean per-cell P04, P05, P06, remaining continuity/negative suites, M8-A,
M8-B, and the Kern soak/Required-promotion gates remain open.
