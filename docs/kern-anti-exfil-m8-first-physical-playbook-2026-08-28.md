# M8 first physical playbook — Sparrow GUI to Kern P2WPKH

Date: 2026-08-28
Status: prepared for review; stop before Step 1
Case reserved: M8-D/P01 attempt 01

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
playbook have completed independent review.

## Required identities

- Sparrow `b99417afe4468a3caa4ccb58d7c7ff4db3fe0429`
- Drongo `54365d7f09df956e0b3e8baf035b23920073bac3`
- Sparrow JAR SHA-256
  `332cc75d2fd406963f0fe5ae91e04a9439ceaf9ea5eeabdfa1d40285a7c0d4bc`
- Kern flashed firmware receipt DR-KERN-01, SHA-256
  `b9aecca4dc0c894d8b7ff282c160aa3758cd4133c825247954ee15e59e307e1d`
- Testnet3 and `aext-v1`

If any identity differs, stop and create a new build/device receipt.

## Pre-test preparation

1. Close normal Sparrow completely.
2. Back up the existing watch-only test wallet/export needed for this test.
3. Create and use a dedicated development home:

   `C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-sparrow-home`

   Do not point this fork at `%APPDATA%\Sparrow`.
4. Use only a disposable Testnet3 wallet. The preferred first case is a
   single-signature BIP84 wallet with exactly one spendable Testnet3 input and
   one Kern-controlled signing slot.
5. Do not load a production seed into Kern or Sparrow. Sparrow receives public
   wallet material only.
6. Connect the Kern serial monitor before scanning so lifecycle/resource lines
   are captured from the start. Save a redacted copy after the attempt.
7. Set camera exposure to the previously successful position and frame the
   animated QR at roughly 75% of the Kern camera preview height.

## Launch command after review

From the Sparrow checkout:

```powershell
.\gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home run --args="--dir C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\m8-sparrow-home --network testnet"
```

Checkout:

`C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\feasibility-sources\our-sparrow`

Confirm the UI says Testnet before importing anything.

## Wallet setup after review

1. Load the selected disposable test seed on Kern.
2. Keep Kern on Testnet.
3. Export its native-SegWit account xpub/keystore QR.
4. In the isolated Sparrow instance, create or import the corresponding
   single-signature wallet using the **Kern** importer.
5. Confirm the keystore model is Kern, profile is `aext-v1`, and protected
   policy is `OPTIONAL`. It must not be mislabeled SeedSigner, Krux, or generic
   Specter DIY.
6. Confirm the wallet fingerprint, derivation, descriptor, and first receive
   address against the existing known test wallet.
7. Ensure the wallet has exactly the intended Testnet3 UTXO. Do not broadcast
   or fund a new transaction as part of this playbook.

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

1. Create a single-input native-SegWit spend to a disposable Testnet3 address.
2. Record input count, amount, fee, destination, frozen-PSBT SHA-256, and the
   Sparrow session identifier. Do not broadcast.
3. Choose Kern/protected signing. Expected: Sparrow displays an animated
   `x-btc-anti-exfil` stage-1 request, not an ordinary `crypto-psbt` signing QR.
4. Save/hash the canonical stage-1 artifact when the development tooling makes
   it available.

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
5. Capture response lifecycle/heap lines and hash the stage-2 artifact.
6. Scan message 2 into Sparrow before pressing Done on Kern if that makes
   evidence capture easier.

Stop if a signature or signed PSBT is returned, review details differ, an
unexpected policy override is required, or Back/Cancel creates a response.

### Step 3 — Sparrow creates message 3

1. Sparrow must validate the complete opening set for the exact controlled
   slot and durably advance the same session.
2. Expected: Sparrow displays an animated stage-3 host-reveal request for the
   exact frozen PSBT and session.
3. Record the session prefix and message-3 hash/length. Compare them with the
   stage-1 receipt before scanning.

Stop if Sparrow silently starts a new session, changes the PSBT, accepts an
incomplete opening set, or exposes an ordinary signed-PSBT merge path.

### Step 4 — Kern creates message 4

1. Scan the exact stage-3 QR.
2. Expected: `Protected signing — Step 2 of 2` and a fresh full transaction
   review. Verify the same transaction again.
3. Press `Create signatures` only after the independent review.
4. Expected: animated `Protected signatures`; no ordinary signed PSBT is
   exported.
5. Capture response lifecycle/heap lines and scan message 4 into Sparrow.
6. Press Done only after Sparrow has either captured the response or the
   response evidence has been retained.

Stop if Kern accepts a different network/PSBT/session/slot set, omits the
second approval, returns ordinary PSBT state, or offers automatic retry.

### Step 5 — Sparrow verifies and reconstructs

1. Sparrow must verify the ECDSA signature and S2C opening for every expected
   slot.
2. It must insert only verified signatures into its frozen original PSBT.
3. Record the final PSBT hash and signature placement.
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

The first receipt should be `M8-D/P01 attempt 01`. It remains `NOT RUN` until
the independent review requested at this checkpoint is complete.
