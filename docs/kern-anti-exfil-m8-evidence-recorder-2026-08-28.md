# Kern anti-exfil M8 evidence recorder

Date opened: 2026-08-28
Status: independent preflight review complete; no live M8 ceremony has been run

This is the append-only human-readable index. Store bulky logs, images, QR
frames, and message artifacts in a case directory and link them from the case
receipt. Do not paste secret material into this file.

## Build receipt BR-2026-08-28-01

Status: superseded before live testing by BR-2026-08-30-01. No M8 ceremony
used these artifacts.

| Item | Recorded value |
| --- | --- |
| Sparrow branch | `codex/kern-anti-exfil-m7` |
| Sparrow commit | `b99417afe4468a3caa4ccb58d7c7ff4db3fe0429` |
| Drongo commit | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark commit | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| Build command | `gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home clean assemble` |
| Result | `BUILD SUCCESSFUL` in 27 seconds, 16 tasks executed |
| Gradle | 9.1.0 |
| Java | Eclipse Adoptium Temurin 25.0.4+7-LTS |
| OS | Windows 10 amd64 |
| Release-reference JDK deviation | Upstream names Temurin 25.0.2+10; this development build used 25.0.4+7 |
| JAR | `build/libs/sparrow-2.5.4.jar`, 15,697,916 bytes |
| JAR SHA-256 | `332cc75d2fd406963f0fe5ae91e04a9439ceaf9ea5eeabdfa1d40285a7c0d4bc` |
| ZIP distribution | `build/distributions/sparrow-2.5.4.zip`, 86,872,024 bytes |
| ZIP SHA-256 | `34f9e3a6515e8ce14f0db107954cb710998a1dbdc1325cdea7c5676ea6ea3806` |
| TAR distribution | `build/distributions/sparrow-2.5.4.tar`, 93,357,568 bytes |
| TAR SHA-256 | `1d8bff7cece6e371afe0533754635fbb67f3d770e99f839d73fe6a70fea35607` |

The build is suitable for the planned development run. It is not a release or
reproducible-build claim, and no installer was created.

## Build receipt BR-2026-08-30-01

Status: superseded before live testing by BR-2026-09-01-01. A setup-only launch
revealed that Kern was absent from the importer registry; no transaction or
anti-exfil session was created with this artifact.

| Item | Recorded value |
| --- | --- |
| Sparrow branch | `codex/kern-anti-exfil-m7` |
| Sparrow commit | `0f9029a70543358f01af473aab1813ccd49143a1` |
| Drongo commit | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark commit | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| Change since BR-2026-08-28-01 | QR prompts use the selected signer model, the generic tooltip no longer names SeedSigner, and INFO logs record canonical package direction, stage, length, and SHA-256 |
| Build command | `gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 clean assemble` |
| Result | `BUILD SUCCESSFUL` in 14 seconds, 16 tasks executed |
| Gradle / Java | 9.1.0 / Eclipse Adoptium Temurin 25.0.4+7-LTS |
| JAR | `build/libs/sparrow-2.5.4.jar`, 15,698,749 bytes |
| JAR SHA-256 | `ee26ed2e0d1985166a5fdf23dd68a5a508f0ec7b29c307237aea8b0d1e22d354` |
| ZIP distribution | `build/distributions/sparrow-2.5.4.zip`, 86,872,726 bytes |
| ZIP SHA-256 | `0288c86ac76172333e4b9000b25ac58a17ac8fade5df4b1b0aba69681eb82374` |
| TAR distribution | `build/distributions/sparrow-2.5.4.tar`, 93,358,592 bytes |
| TAR SHA-256 | `193a0293cb44800a6e68232ea3248a3c08e27ad01aad16912dbcd4f1ff4b6c68` |

The canonical package hash is now automatic evidence. Sparrow logs only its
direction, stage, byte length, and SHA-256; it does not log PSBT bytes, host
randomness, openings, or signatures.

## Build receipt BR-2026-09-01-01

Status: superseded before live testing by BR-2026-09-01-02. A setup-only
replacement attempt exposed oversized Kern SVG assets that clipped the
keystore fields; the operator did not apply the replacement or create a
transaction.

| Item | Recorded value |
| --- | --- |
| Sparrow branch | `codex/kern-anti-exfil-m7` |
| Sparrow commit | `3b565e3e5ac15ddc07f6e7aeaf537308c7beb7bf` |
| Drongo commit | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark commit | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| Change since BR-2026-08-30-01 | Kern registered once in the single-HD, single-SP, and multisig airgapped importer lists; focused registry test added |
| Build command | `gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 clean assemble` |
| Result | `BUILD SUCCESSFUL` in 15 seconds, 16 tasks executed |
| Gradle / Java | 9.1.0 / Eclipse Adoptium Temurin 25.0.4+7-LTS |
| JAR | `build/libs/sparrow-2.5.4.jar`, 15,698,835 bytes |
| JAR SHA-256 | `0e0bb9c37cad079e16e7e244a130fa0625891aa072f31e448c201fc6ce52ef21` |
| ZIP distribution | `build/distributions/sparrow-2.5.4.zip`, 86,872,801 bytes |
| ZIP SHA-256 | `8e14ff2e1913474d2c6d588e4304148f5ac878da64ee20e25313abb2b326e927` |
| TAR distribution | `build/distributions/sparrow-2.5.4.tar`, 93,358,592 bytes |
| TAR SHA-256 | `d84e823af4ca68c04deb2996da984eceb6d2b2b45d8b6bde96d9f72aa02fe274` |

Focused `HwAirgappedControllerTest`, `AntiExfilQrExchangeTest`,
`KernImportTest`, `AntiExfilPolicyPersistenceTest`, and
`AntiExfilPolicySelectionTest` passed before this clean assembly.

## Build receipt BR-2026-09-01-02

Status: superseded before live testing by BR-2026-09-01-03. Physical setup
showed that changing only the SVG width and height resized the visible mark but
left its 200-unit coordinate bounds controlling JavaFX layout.

| Item | Recorded value |
| --- | --- |
| Sparrow branch | `codex/kern-anti-exfil-m7` |
| Sparrow commit | `6c01dec2a861d4a1ade082b3cdc15ac4c6d8f602` |
| Drongo commit | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark commit | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| Change since BR-2026-09-01-01 | Kern's four wallet-model SVG assets now use Sparrow's standard 50-by-50 intrinsic dimensions; focused asset regression test added |
| Build command | `gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 clean assemble` |
| Result | `BUILD SUCCESSFUL` in 24 seconds, 16 tasks executed |
| Gradle / Java | 9.1.0 / Eclipse Adoptium Temurin 25.0.4+7-LTS |
| JAR | `build/libs/sparrow-2.5.4.jar`, 15,698,843 bytes |
| JAR SHA-256 | `b41085f3cd587b267bb42380caedaf895a57120d16f58b54d2038566ab680f79` |
| ZIP distribution | `build/distributions/sparrow-2.5.4.zip`, 86,872,803 bytes |
| ZIP SHA-256 | `388d196ec7152d4025382f60410d8324aee03f8899dc49c36cb57f2e68a44c12` |
| TAR distribution | `build/distributions/sparrow-2.5.4.tar`, 93,358,592 bytes |
| TAR SHA-256 | `453a5122530703adbb65e97a9d65fe52f03ae7eaa7de56ff9fde6d9c6fe62080` |

Focused `KernWalletModelAssetTest`, `HwAirgappedControllerTest`,
`KeystoreFxmlAntiExfilTest`, `KernImportTest`, and
`AntiExfilPolicySelectionTest` passed before this clean assembly.

## Build receipt BR-2026-09-01-03

| Item | Recorded value |
| --- | --- |
| Sparrow branch | `codex/kern-anti-exfil-m7` |
| Sparrow commit | `a967c0a88eb4db5a8b85eae2e7cc8581709def50` |
| Drongo commit | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark commit | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| Change since BR-2026-09-01-02 | All four Kern SVG viewBoxes and circle geometry normalized to 50-by-50; regression test now verifies every stroked artwork bound fits that coordinate system |
| Build command | `gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 clean assemble` |
| Result | `BUILD SUCCESSFUL` in 22 seconds, 16 tasks executed |
| Gradle / Java | 9.1.0 / Eclipse Adoptium Temurin 25.0.4+7-LTS |
| JAR | `build/libs/sparrow-2.5.4.jar`, 15,698,847 bytes |
| JAR SHA-256 | `5c0c1a2405ff5d1de7111c7158a4a5b680a4ccd42d5ff81bc220244e67e16f97` |
| ZIP distribution | `build/distributions/sparrow-2.5.4.zip`, 86,872,811 bytes |
| ZIP SHA-256 | `1af4e8b9c6f7b02944b574cdf11dc52f3e9eda339310f06fa9a3911a9cea6662` |
| TAR distribution | `build/distributions/sparrow-2.5.4.tar`, 93,358,592 bytes |
| TAR SHA-256 | `10567b5a6841f4ab5c22b4759d99a0a6de772134315e816ce86eb6f5be50bc33` |

The five focused tests named in BR-2026-09-01-02 were rerun from scratch. The
asset test now parses each SVG and verifies its viewBox, three-ring mark, and
stroke-inclusive horizontal and vertical bounds.

## Setup observation SO-2026-09-01-01

- The first isolated Sparrow launch used BR-2026-08-30-01.
- The airgapped importer UI did not list Kern. The operator selected Krux for
  the matching public key and set protected signing Optional.
- Fingerprint `0fb882ff`, derivation `m/84'/1'/0'`, the tpub, and the one
  Testnet3 UTXO loaded, but the keystore model was visibly Krux.
- The operator stopped before transaction creation. `Protected QR` was never
  selected; no anti-exfil session or message was created; M8-D/P01 remains
  `NOT RUN`.
- Krux has no entry in Sparrow's anti-exfil capability registry, so the
  mislabeled keystore could not have entered a protected workflow even though
  its stored policy was Optional. The setup detour was fail-safe by
  construction as well as by the operator's stop.
- Root cause: `Kern` existed as an importer class but was omitted from
  `HwAirgappedController`'s displayed importer lists.
- Sparrow `3b565e3` registers Kern for all applicable policy types and pins the
  registry with a focused regression test.
- Recovery: relaunch BR-2026-09-01-01 and replace the Krux keystore metadata by
  selecting Kern and rescanning the identical account QR. Descriptor/address/
  UTXO identity must remain unchanged.

## Setup observation SO-2026-09-01-02

- The operator relaunched BR-2026-09-01-01 and selected the now-visible Kern
  importer for keystore replacement.
- The Kern identity and fingerprint `0fb882ff` were visible, but Kern's
  200-by-200 intrinsic SVG size expanded the fixed keystore pane and clipped
  the derivation, tpub, and Protected signing fields.
- The operator did not apply the replacement and did not create a transaction.
  No protected QR, session, or protocol message was created; M8-D/P01 remains
  `NOT RUN`.
- Sparrow `6c01dec` changes only the intrinsic dimensions of the four Kern SVG
  assets to the standard 50-by-50 size and pins all variants with a focused
  regression test.

## Setup observation SO-2026-09-01-03

- The operator chose to relaunch BR-2026-09-01-02 before obtaining its planned
  independent delta review.
- The Kern mark rendered at the intended visible size, but both the importer
  tile and keystore pane still retained approximately 200 pixels of layout
  space. The derivation, tpub, and Protected signing fields remained clipped.
- The operator again did not apply the replacement and did not create a
  transaction. No protected QR, session, or protocol message was created;
  M8-D/P01 remains `NOT RUN`.
- Root cause: JavaFX layout followed the SVG's 200-unit viewBox/geometry even
  after its width and height were reduced. Sparrow `a967c0a` normalizes both
  the coordinate system and artwork geometry and strengthens the regression
  test to check actual stroke-inclusive bounds.

## Coordinator test receipt TR-2026-08-28-01

The following focused suites passed against the pinned Sparrow/Drongo source
after the clean assembly:

- `AntiExfilPolicyPersistenceTest`
- `DbPersistenceTest`
- `KernImportTest`
- `SeedSignerImportPolicyTest`
- `KeystoreFxmlAntiExfilTest`
- `AntiExfilPolicySelectionTest`
- `AntiExfilQrExchangeTest`

Gradle reported `BUILD SUCCESSFUL` in 15 seconds. H2 emitted the previously
observed Windows shutdown warnings after temporary test directories had been
removed; no test failed. The generated stray CSV test artifact was removed and
the Sparrow working tree is clean.

## Device receipt DR-KERN-01

| Item | Recorded value |
| --- | --- |
| Device | Waveshare ESP32-P4-WIFI6-Touch-LCD-7B (`wave_7b`) |
| Camera | OV5647-compatible module; exposure adjusted during M5 physical work |
| Kern source HEAD | `9bfda18` |
| Flashed firmware source | `c360341` M6 stage-3 workflow |
| Flashed `kern.bin` SHA-256 | `b9aecca4dc0c894d8b7ff282c160aa3758cd4133c825247954ee15e59e307e1d` |
| Firmware size | 1,970,176 bytes (`0x1e1000`) |
| Port used for flash | COM6 |
| Existing physical evidence | M6 stage-1 and stage-3 checkpoints |
| M8 readiness | Awaiting independent review and first live coordinator ceremony |

Before M8-D/P01, verify the device still runs this image or record a new device
receipt after rebuilding/flashing.

## Matrix ledger

| Cell | P01 baseline | Positive suite | Continuity suite | Negative suite | Soak | Final |
| --- | --- | --- | --- | --- | --- | --- |
| M8-A CLI/SeedSigner | Not run | Not run | Not run | Not run | N/A unless selected | Open |
| M8-B CLI/Kern | Not run | Not run | Not run | Not run | Not run | Open |
| M8-C GUI/SeedSigner | Not run | Not run | Not run | Not run | N/A unless selected | Open |
| M8-D GUI/Kern | Not run | Not run | Not run | Not run | Not run | Open |

## Case receipt template

Copy this section for every attempt; never edit a failed attempt into a pass.

### `<case-id>` attempt `<nn>` — `<short name>`

| Field | Value |
| --- | --- |
| Matrix cell | |
| Result | `NOT RUN`, `PASS`, `FAIL`, or `BLOCKED` |
| Started / completed | |
| Operator / reviewer | |
| Sparrow / Drongo / Lark commits | |
| Kern firmware receipt | |
| Network / profile | `testnet` / `aext-v1` |
| Wallet alias and script class | Public alias only; no mnemonic |
| Inputs / controlled slots | |
| Descriptor registered | |
| Fragment size / display / camera | |
| Session-ID display prefix | |
| Frozen-PSBT SHA-256 | |
| M1 length / SHA-256 | |
| M2 length / SHA-256 | |
| M3 length / SHA-256 | |
| M4 length / SHA-256 | |
| Final reconstructed PSBT SHA-256 | |
| Expected outcome / reason | |
| Observed outcome / reason | |
| Signatures cryptographically verified | |
| Frozen-PSBT reconstruction only | |
| Broadcast attempted | Must be `No` |
| Cleanup state | |
| Evidence directory | |
| Follow-up | |

#### Timing and resource sample

| Phase | Elapsed | Free heap | Largest block | Lifetime minimum | Stack high-water |
| --- | ---: | ---: | ---: | ---: | ---: |
| Before scan | | | | | |
| Request retained / camera stopped | | | | | |
| Signer entry | | | | | |
| Signer complete | | | | | |
| Response ready | | | | | |
| Viewer destroyed | | | | | |
| Ceremony cleanup | | | | | |

#### Decision trace

- Stage 1 review shown:
- Stage 1 explicit approval:
- Stage 2 accepted and durably recorded by coordinator:
- Stage 3 exact-session lookup:
- Stage 3 independent review shown:
- Stage 3 explicit approval:
- Stage 4 S2C and ECDSA verification:
- Final PSBT reconstructed from frozen original:
- No ordinary fallback or returned-PSBT merge:

#### Artifacts and observations

- Logs:
- Screenshots/photos:
- Canonical messages:
- Coordinator receipt:
- Deviations or warnings:
- Reviewer disposition:

## Preflight review record

| Review item | Status | Notes |
| --- | --- | --- |
| M8 runbook reviewed | Pass | KimiK3 preflight and follow-up reviews; control-case instruction verified |
| Evidence recorder reviewed | Pass | Identity chain and superseded receipt handling verified |
| Sparrow build receipt reviewed | Pending delta review | BR-2026-09-01-01 was reviewed; replacement BR-2026-09-01-03 pins the complete Kern SVG coordinate-bound fix and rebuilt artifacts |
| Physical playbook reviewed | Pass | Seed custody, network identity, scan order, and automatic evidence confirmed |
| Kern importer-registry delta reviewed | Pass | KimiK3 approved Sparrow `3b565e3` and Kern evidence commit `3696c57`; UI-only registration change, no protocol or policy-registry delta |
| Kern asset-sizing delta reviewed | Pending | Review Sparrow `a967c0a` plus the replacement BR-2026-09-01-03 receipt before relaunching |
| First M8-D/P01 ceremony authorized | Paused | Authorization is suspended until the asset-sizing delta passes review; case remains `NOT RUN` |

The follow-up review covered Sparrow `b0463a2` and `0f9029a` plus Kern docs
`5efa6ce`. It found no protocol/validation changes, no blocking issue, and
accepted advancement from preflight to M8-D/P01 attempt 01.

Setup observation SO-2026-09-01-01 subsequently exposed a missing visible Kern
importer. Because the operator stopped before transaction creation, the earlier
review remains valid background evidence. KimiK3's delta review approved the
narrow Sparrow `3b565e3` fix and replacement build receipt, so live
authorization is reinstated with keystore replacement as a mandatory pre-step.

Setup observation SO-2026-09-01-02 then exposed oversized Kern SVG assets
before replacement was applied. Live authorization is paused again for the
narrow Sparrow `a967c0a` coordinate-bound delta and BR-2026-09-01-03 review.
