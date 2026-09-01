# M8 first physical playbook — Sparrow GUI to Kern P2WPKH

Date: 2026-08-28; revised 2026-09-01
Status: M8-D/P01 attempt 02 completed successfully without broadcast
Completed case: M8-D/P01 attempt 02; attempt 01 remains an immutable failure

## Goal

Complete one live, single-input native-SegWit Testnet3 ceremony between the
M7 Sparrow GUI fork and the physical Kern. Sparrow must create the real stage-1
request, durably retain the exact session, verify Kern's stage-2 openings,
create the stage-3 reveal, verify Kern's stage-4 signatures and S2C proof, and
reconstruct its original frozen PSBT without broadcasting.

This playbook is intentionally not a fixture-QR test. Do not use
`physical_qr_output` for the positive ceremony.

## Stop point for this checkpoint

Do not launch the development Sparrow instance, import a wallet, create a PSBT,
or scan stage 1 until the runbook, evidence recorder, build receipt, and this
revised playbook have completed review.

## Plain-English setup answers

- **Where does the seed go?** Load the disposable test seed into Kern. Sparrow
  remains watch-only and receives only Kern's account xpub/output descriptor.
  Never enter seed words or a SeedQR into Sparrow.
- **What is one Kern-controlled signing slot?** A slot is one transaction input
  plus one Kern-controlled public key and sighash. For this test, a
  single-signature wallet with exactly one selected input automatically gives
  one slot. You do not locate or configure it separately.
- **Testnet3 versus Testnet4:** Kern's UI groups public test networks under
  `Testnet`, but the anti-exfil message preserves the exact network identity.
  Sparrow's `--network testnet` means Testnet3, so the first run uses the one
  Testnet3 UTXO belonging to fingerprint `0fb882ff`. Testnet4 wallet history is
  isolated and will not be selected by this Sparrow instance.
- **Is the saved SeedQR enough?** It is sufficient to restore the disposable
  signer seed on Kern. It is not a backup of Sparrow labels or wallet metadata,
  but this test deliberately creates a fresh watch-only wallet in an isolated
  Sparrow home.
- **Who saves the stage artifacts?** Sparrow now records direction, stage,
  canonical package length, and SHA-256 automatically in its isolated
  `sparrow.log`. The user does not save or hash the animated QR manually.
- **When does monitoring start?** Stop before launching Sparrow and coordinate
  with Codex. Codex starts the Kern serial monitor; then the user performs the
  physical steps and reports each checkpoint.

## Required identities

- Sparrow `182bc8a7b24641e43cecf324e96eec6314f9b18b`
- Drongo `54365d7f09df956e0b3e8baf035b23920073bac3`
- Sparrow JAR SHA-256
  `c273f1e367db31ac1dd06458c9aa7a8f0ae1bd358770f7c61f33f33b7f835f35`
- Kern `5180dbb603e01e33698bb388a400f92bff722d4c`, flashed firmware receipt
  DR-KERN-02, SHA-256
  `e7df2b55d7c476a0ff67c06aebcd55e7fdf3b7027de98b58022828e85e1f893a`
- Testnet3 and `aext-v1`

If any identity differs, stop and create a new build/device receipt.

## Pre-test preparation

1. Close normal Sparrow completely.
2. Have the disposable SeedQR for fingerprint `0fb882ff` available. The normal
   Sparrow wallet is not opened or migrated during this test.
3. Confirm this dedicated development home exists and is otherwise isolated:

   `C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-sparrow-home`

   Do not point this fork at `%APPDATA%\Sparrow`.
4. Use the BIP84 account for fingerprint `0fb882ff` at `m/84'/1'/0'`. In the
   transaction editor, select only its one spendable Testnet3 UTXO. That is the
   one-input/one-Kern-slot baseline.
5. Load this disposable seed into Kern when instructed. Sparrow receives only
   Kern's public account data. Do not load any production seed into either app.
6. Ask Codex to start the Kern serial monitor before scanning. Codex will
   capture and later redact the lifecycle/resource lines.
7. Set camera exposure to the previously successful position and frame the
   animated QR at roughly 75% of the Kern camera preview height.

## Launch command after review

"From the Sparrow checkout" means: open an ordinary Windows PowerShell window,
change into the folder containing this Sparrow source branch, and then run the
build launcher. Paste these commands one at a time:

```powershell
cd "C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\feasibility-sources\our-sparrow"
.\gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 run --args="--dir C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-sparrow-home --network testnet"
```

The first command is all "from the checkout" means. Leave that PowerShell
window open while Sparrow runs because its output is supporting evidence.

Confirm the UI says Testnet before importing anything.

## Wallet setup after review

1. Restore the disposable `0fb882ff` seed on Kern from its SeedQR.
2. Keep Kern on Testnet.
3. Export its native-SegWit account xpub/keystore QR.
4. In the isolated Sparrow instance, create or import the corresponding
   single-signature wallet using the **Kern** importer.
5. Confirm the keystore model is Kern, profile is `aext-v1`, and protected
   policy is `OPTIONAL`. It must not be mislabeled SeedSigner, Krux, or generic
   Specter DIY.
6. Confirm the wallet fingerprint, derivation, descriptor, and first receive
   address against the existing known test wallet.
7. Wait for Testnet3 synchronization and confirm the intended UTXO appears.
   Testnet4 history must not appear in this instance. Do not broadcast or fund
   a new transaction as part of this playbook.

If an earlier preflight build created the same watch-only wallet as another
model, open Settings, select its keystore, choose `Replace`, select **Kern**,
and rescan the same Kern account QR. Apply only after the fingerprint,
derivation, and tpub match exactly. This changes device/profile metadata; it
must not change the descriptor, addresses, or UTXO.

## Device settings immediately before the ceremony

- Network: Testnet
- Anti-exfil signing: On
- Expected-owned signing: start Off
- Permissive signing: start Off
- Partial signing: Off for this single-signature case

If Kern's ordinary transaction policy blocks a valid known test fixture,
record the exact warning before enabling a narrowly needed override. Do not
pre-enable both overrides merely because the older synthetic M6 fixture needed
them.

## Live ceremony after review

### Step 1 — Sparrow creates message 1

1. In Sparrow, open the wallet's UTXOs tab and select only the one intended
   Testnet3 UTXO. Choose `Send Selected` and create a native-SegWit spend to a
   disposable Testnet3 receiving address. Review the amount and fee.
2. Create the transaction so Sparrow opens its transaction/PSBT view. Do not
   finalize or broadcast it.
3. At the bottom of that view, click `Protected QR` (lock icon), not the
   ordinary `Sign` button. Expected: Sparrow displays an animated
   `x-btc-anti-exfil` stage-1 request, not an ordinary `crypto-psbt` signing QR.
   Its heading must say to scan the commitment with **Kern**.
4. No manual save/hash action is required. Leave the QR open and tell Codex
   `Stage 1 QR is up`. Sparrow has already durably written the session and an
   INFO line like the following to
   `m8-sparrow-home\sparrow.log`:

   `Anti-exfil package direction=outgoing stage=HOST_COMMIT bytes=... sha256=...`

   Codex will read that line and enter it in the evidence recorder.

Stop immediately if Sparrow offers `REQUIRED` for Kern, selects another
profile, loses the session on navigation, or displays an ordinary signing QR.

### Step 2 — Kern creates message 2

1. Scan Sparrow's stage-1 QR.
2. Expected: `Protected signing — Step 1 of 2` followed by Kern's ordinary
   transaction review. Verify destination, amount, fee, input count, and owned
   input classification against Sparrow.
3. Press `Create commitments` only after the complete review.
4. Expected: animated `Nonce commitments`; Kern explicitly states the
   transaction is not signed.
5. Leave Kern's message-2 QR displayed and tell Codex `Nonce commitments QR is
   up`. Codex captures the heap/lifecycle lines.
6. On Sparrow, close/confirm the stage-1 display so Sparrow opens its
   anti-exfil scanner. Point Sparrow's camera at Kern's message-2 QR.
7. Wait until Sparrow accepts it and displays the stage-3 host-reveal QR. This
   automatically logs the incoming message-2 hash and outgoing message-3 hash.
8. Only then press Done on Kern. Record this exact order in the receipt:
   `message 2 scanned by Sparrow before Kern Done`.

Stop if a signature or signed PSBT is returned, review details differ, an
unexpected policy override is required, or Back/Cancel creates a response.

### Step 3 — Sparrow creates message 3

1. Sparrow must validate the complete opening set for the exact controlled
   slot and durably advance the same session.
2. Expected: Sparrow displays an animated stage-3 host-reveal request for the
   exact frozen PSBT and session. Its heading must say to scan the host reveal
   with **Kern**.
3. Leave Sparrow's stage-3 QR open and tell Codex `Stage 3 QR is up`. Codex
   reads the automatically logged message-2/message-3 lengths and hashes and
   checks the same durable session before you scan it.

Stop if Sparrow silently starts a new session, changes the PSBT, accepts an
incomplete opening set, or exposes an ordinary signed-PSBT merge path.

### Step 4 — Kern creates message 4

1. Scan the exact stage-3 QR.
2. Expected: `Protected signing — Step 2 of 2` and a fresh full transaction
   review. Verify the same transaction again.
3. Press `Create signatures` only after the independent review.
4. Expected: animated `Protected signatures`; no ordinary signed PSBT is
   exported.
5. Leave Kern's message-4 QR displayed and tell Codex `Protected signatures QR
   is up`. Codex captures the response heap/lifecycle lines.
6. On Sparrow, close/confirm the stage-3 display so its anti-exfil scanner
   opens. Scan Kern's message-4 QR. Sparrow automatically logs the incoming
   message-4 length and hash.
7. Wait for Sparrow's completed signed-PSBT view. Only then press Done on Kern
   and record `message 4 scanned by Sparrow before Kern Done`.

Stop if Kern accepts a different network/PSBT/session/slot set, omits the
second approval, returns ordinary PSBT state, or offers automatic retry.

### Step 5 — Sparrow verifies and reconstructs

1. Sparrow must verify the ECDSA signature and S2C opening for every expected
   slot.
2. It must insert only verified signatures into its frozen original PSBT.
3. Stop on Sparrow's completed PSBT view and tell Codex `Sparrow verification
   view is up`. Codex records the final PSBT hash, signature placement, and
   protected provenance from the isolated state/logs.
4. Confirm the transaction is not broadcast.
5. Export a public Testnet3 evidence copy only if it contains no unintended
   labels or metadata; otherwise retain hashes and redacted logs.

Pass requires the final PSBT to be valid, the expected Kern signature to be
present, all protected verification to succeed, and no signer-returned
transaction state to have been trusted.

## Cleanup

1. Save immutable attempt evidence before retrying anything.
2. Return Kern Anti-exfil, Expected-owned, Permissive, and Partial signing
   toggles to Off.
3. Exit the development Sparrow instance.
4. Preserve the isolated Sparrow home until review of the receipt completes;
   do not copy it over the normal Sparrow home.
5. Redact logs and update the M8 evidence recorder.
6. On failure, stop for analysis. Do not create a fresh challenge and label it
   an exact-session retry.

## Expected first-test receipt

Attempt 01 is an immutable `FAIL` receipt. The next receipt is
`M8-D/P01 attempt 02`; it remains `NOT RUN` until independent review accepts
Kern `5180dbb`, DR-KERN-02, and the recorded attempt-01 failure. Create a new
transaction/session for attempt 02. Do not resume, relabel, or reuse the
attempt-01 durable session.
