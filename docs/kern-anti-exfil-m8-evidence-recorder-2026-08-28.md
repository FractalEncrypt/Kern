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

## Setup observation SO-2026-09-01-01

- The first isolated Sparrow launch used BR-2026-08-30-01.
- The airgapped importer UI did not list Kern. The operator selected Krux for
  the matching public key and set protected signing Optional.
- Fingerprint `0fb882ff`, derivation `m/84'/1'/0'`, the tpub, and the one
  Testnet3 UTXO loaded, but the keystore model was visibly Krux.
- The operator stopped before transaction creation. `Protected QR` was never
  selected; no anti-exfil session or message was created; M8-D/P01 remains
  `NOT RUN`.
- Root cause: `Kern` existed as an importer class but was omitted from
  `HwAirgappedController`'s displayed importer lists.
- Sparrow `3b565e3` registers Kern for all applicable policy types and pins the
  registry with a focused regression test.
- Recovery: relaunch BR-2026-09-01-01 and replace the Krux keystore metadata by
  selecting Kern and rescanning the identical account QR. Descriptor/address/
  UTXO identity must remain unchanged.

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
| Sparrow build receipt reviewed | Pending delta review | BR-2026-08-30-01 was reviewed; replacement BR-2026-09-01-01 pins the importer-registry fix and rebuilt artifacts |
| Physical playbook reviewed | Pass | Seed custody, network identity, scan order, and automatic evidence confirmed |
| Kern importer-registry delta reviewed | Pending | Review Sparrow `3b565e3` plus this updated receipt before relaunching the ceremony |
| First M8-D/P01 ceremony authorized | Paused | Earlier authorization is suspended until the importer-registry delta and BR-2026-09-01-01 pass review; case remains `NOT RUN` |

The follow-up review covered Sparrow `b0463a2` and `0f9029a` plus Kern docs
`5efa6ce`. It found no protocol/validation changes, no blocking issue, and
accepted advancement from preflight to M8-D/P01 attempt 01.

Setup observation SO-2026-09-01-01 subsequently exposed a missing visible Kern
importer. Because the operator stopped before transaction creation, the earlier
review remains valid background evidence, but live authorization is paused for
the narrow Sparrow `3b565e3` delta and replacement build receipt.
