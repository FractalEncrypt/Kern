# Kern anti-exfil M8 evidence recorder

Date opened: 2026-08-28
Status: M8-X01 attempt 02 Parts E/F are independently accepted; its single
deliberate Testnet3 Part G broadcast confirmed and cleanup is complete;
independent post-broadcast review remains pending

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

Status: superseded before live testing by BR-2026-09-01-04. Final live
baseline inspection found that the global WARN logger suppressed the
anti-exfil package receipts emitted at INFO.

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

## Build receipt BR-2026-09-01-04

Status: superseded for future ceremonies by BR-2026-09-02-01 after M8-X01
attempt 01 exposed the mixed-policy signer-selection defect. The accepted
P01/P02 results produced with this build remain valid evidence.

| Item | Recorded value |
| --- | --- |
| Sparrow branch | `codex/kern-anti-exfil-m7` |
| Sparrow commit | `182bc8a7b24641e43cecf324e96eec6314f9b18b` |
| Drongo commit | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark commit | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| Change since BR-2026-09-01-03 | INFO enabled only for `AntiExfilQrExchange`; runtime-level regression assertion added so canonical package receipts cannot be silently suppressed by the global WARN threshold |
| Build command | `gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 clean assemble` |
| Result | `BUILD SUCCESSFUL` in 22 seconds, 16 tasks executed |
| Gradle / Java | 9.1.0 / Eclipse Adoptium Temurin 25.0.4+7-LTS |
| JAR | `build/libs/sparrow-2.5.4.jar`, 15,698,877 bytes |
| JAR SHA-256 | `c273f1e367db31ac1dd06458c9aa7a8f0ae1bd358770f7c61f33f33b7f835f35` |
| ZIP distribution | `build/distributions/sparrow-2.5.4.zip`, 86,872,824 bytes |
| ZIP SHA-256 | `7bb12409798a61b4bf79ce797ab375afab31a0fb2633071e1205c67895a8bdd3` |
| TAR distribution | `build/distributions/sparrow-2.5.4.tar`, 93,358,592 bytes |
| TAR SHA-256 | `bf2c23e22e85c9b49e1858291d48a18743ea68d1df9b774a700cdb2f839476b8` |

Focused `AntiExfilQrExchangeTest`, `KernWalletModelAssetTest`,
`HwAirgappedControllerTest`, `KeystoreFxmlAntiExfilTest`, `KernImportTest`, and
`AntiExfilPolicySelectionTest` passed. A forced rerun initially met Windows
dependency-JAR access errors while the Gradle-launched Sparrow process was
still open; the same focused suite then passed through the normal task graph,
and the subsequent clean assembly passed after Sparrow closed.

## Build receipt BR-2026-09-02-01

Status: built and hash-pinned; live use paused pending independent review of
the signer-selection delta and M8-X01 attempt-01 failure evidence.

| Item | Recorded value |
| --- | --- |
| Sparrow branch | `codex/kern-anti-exfil-m7` |
| Sparrow commit | `a53d9e166bb480df9f53f0bc4399545a4a1b5be8` |
| Drongo commit | `54365d7f09df956e0b3e8baf035b23920073bac3` |
| Lark commit | `ddffe556f0d1ba6a138be3b362ce74219fed0710` |
| Change since BR-2026-09-01-04 | Protected signer selection now offers every compatible protected-capable keystore regardless of Optional/Required rank; mixed-policy regression added |
| Build command | `gradlew.bat --gradle-user-home C:\Users\FractalEncrypt\Documents\SeedSigner_AntiExfil\run\gradle-home-m8 clean assemble` |
| Result | `BUILD SUCCESSFUL` in 29 seconds, 16 tasks executed |
| Gradle / Java | 9.1.0 / Eclipse Adoptium Temurin 25.0.4+7-LTS |
| JAR | `build/libs/sparrow-2.5.4.jar`, 15,698,734 bytes |
| JAR SHA-256 | `fae73cc86b8be4dcc5c0bbcb2669f7a36f59361e9e0d9963fe5517398a1e9f53` |
| ZIP distribution | `build/distributions/sparrow-2.5.4.zip`, 86,872,709 bytes |
| ZIP SHA-256 | `3e7793887628bcca75e63754f2cd184ff2bc3ac920bcfefc8baf2341d208f429` |
| TAR distribution | `build/distributions/sparrow-2.5.4.tar`, 93,358,592 bytes |
| TAR SHA-256 | `fdf9f3b6d491f432347f25d833b27f958b7f17c5f1b702f165f43620e811e053` |

The focused `AntiExfilPolicySelectionTest` and the complete root-project
`*AntiExfil*` test selection were forced from a clean test state and passed.
Required-signature provenance enforcement tests remain unchanged and passed.
This remains a development build, not a release or reproducible-build claim.

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

## Setup verification SV-2026-09-01-01

- The operator physically relaunched BR-2026-09-01-03 and repeated the
  setup-only Replace-to-Kern flow with the same account QR.
- The Kern importer tile matched the height and alignment of its neighboring
  airgapped-device tiles.
- The keystore pane showed the complete compact Kern mark, Airgapped Wallet
  (Kern) identity, fingerprint `0fb882ff`, derivation `m/84'/1'/0'`, complete
  tpub field, and Protected signing set to Optional without clipping.
- The supplied screenshots show the replacement still staged with Apply
  available. This verification did not create a transaction, protected QR,
  session, or protocol message; M8-D/P01 remains `NOT RUN` pending independent
  review and final application of the identical replacement.

## Setup observation SO-2026-09-01-04

- After KimiK3 approved the SVG delta, the operator applied the validated Kern
  replacement and confirmed the one Testnet3 UTXO was unchanged.
- The operator prepared a self-spend but stopped before transaction creation.
- Codex connected the Kern COM6 serial evidence monitor, then inspected the
  isolated Sparrow home before Stage 1. `sparrow.log` was zero bytes.
- Root cause: `AntiExfilQrExchange` emitted the canonical package receipt at
  INFO while the root Logback threshold was WARN, so the promised stage
  length/SHA-256 evidence would have been suppressed.
- Sparrow `182bc8a` adds an INFO override scoped only to
  `AntiExfilQrExchange` and a runtime effective-level regression assertion.
  No transaction, protected QR, session, or protocol message was created;
  M8-D/P01 remains `NOT RUN`.

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

## Device receipt DR-KERN-02

| Item | Recorded value |
| --- | --- |
| Device | Waveshare ESP32-P4-WIFI6-Touch-LCD-7B (`wave_7b`) |
| Kern source | `5180dbb603e01e33698bb388a400f92bff722d4c` |
| Change from DR-KERN-01 | PSBT-v0 prevout identity and output index resolved through libwally getters; exact live both-UTXO regression added |
| Build | ESP-IDF 6.0.2; ccache disabled; Ninja response files enabled for the Windows command-line limit |
| Test gates | authoritative slots 16/16; stateless signer 22/22; response bridge 21/21; full `wave_7b` firmware build pass |
| Flashed `kern.bin` SHA-256 | `e7df2b55d7c476a0ff67c06aebcd55e7fdf3b7027de98b58022828e85e1f893a` |
| Firmware size | 1,970,176 bytes (`0x1e1000`) |
| Flash result | COM6 write and on-device hash verification passed; hard reset completed |
| Ceremony-start readback | 1,970,176 application bytes read from `0x20000`; SHA-256 `e7df2b55d7c476a0ff67c06aebcd55e7fdf3b7027de98b58022828e85e1f893a`, exact match to the signed build artifact |
| Readback evidence | `run/m8-evidence/DR-KERN-02-app-readback.bin`; application partition only, no NVS/seed partition read |
| M8 readiness | KimiK3 approved `5180dbb`, `e8f3292`, DR-KERN-02, and attempt-01 evidence; COM6 readback condition passed; attempt 02 authorized with a fresh Sparrow session |

## Device receipt DR-SEEDSIGNER-01

| Item | Recorded value |
| --- | --- |
| Device | SeedSigner on Raspberry Pi Zero; operator-selected display configuration |
| SeedSigner app source | `214793df4f51466179b792420921b8cdd8d0c1ac`; tag `anti-exfil-review-v1-finalized-input-tested-2026-08-22` |
| SeedSignerOS source | `0bf1dc92519906c7db265055abfb07e0ee344342`; Buildroot `bf2a2858aa675a14b60f1f9142c65b32652609c1` |
| App overlay preparation | 130 source files copied with non-destructive `/E`; all 130 SHA-256 comparisons matched; generated `version.json` identifies `dev` / `FractalEncrypt` / `214793d` / `2026-08-22T20:36:48` |
| Build mode | Clean Pi Zero instrumented build: `--pi0 --skip-repo --anti-exfil-test --app-commit-id=214793df4f51466179b792420921b8cdd8d0c1ac`; no `--no-clean` |
| Build result | Docker Compose `build-images-1` exited with code 0 after the SeedSignerOS post-image script |
| Image | `seedsigner_os.214793df4f51466179b792420921b8cdd8d0c1ac.pi0.anti-exfil-test.img` |
| Image size / SHA-256 | 52,428,800 bytes / `adc2b58ae9dd57e884ec33b0e39ebf608ee8cc468d3fa7c563a1f1f808550fb3` |
| Independent local artifact check | Size and SHA-256 re-read from the completed image and matched the build log exactly |
| Flash / boot observation | Operator reports the new image is loaded, the ordinary UI booted, and disposable test seed `0fb882ff` is loaded |
| Signer settings | Persistent settings enabled; Testnet selected; Anti-exfil signing `Required`; display type selected |
| Boot self-test receipt | Pass: exit code `0`; `status: ok`; backend `native-secp256k1-zkp`; opening and signature vectors match; `production_fallback: false` |
| Preserved boot evidence | `run/m8-evidence/DR-SEEDSIGNER-01-selftest.exit-code`, SHA-256 `9a271f2a916b0b6ee6cecb2426f0b3206ef074578be55d9bc94f6f3fe3ab86aa`; `run/m8-evidence/DR-SEEDSIGNER-01-selftest.json`, SHA-256 `9e4e516372cafebefd670a54bbf195b5c86f130512237dabb03ecaaa6f98fcb2` |
| Ceremony status | M8-C/P01 remains `NOT RUN`; Sparrow has not been launched for this attempt and no stage-1 message has been created |

## Fixture preparation FP-2026-09-02-01

| Item | Recorded value |
| --- | --- |
| Purpose | Create two real Testnet3 P2WPKH inputs for P02 and two real native-P2WSH inputs for later multisig cases |
| Transaction | `61a05816882fb79f5142137d5514cbcbd76f46772977325b7bfd0f493b9079da` |
| Source | `15962f64510861646fa8039ede2054546148463aea88c937c118e82a8f5284d5:1`, P2WPKH, 180,406 sats |
| Outputs | vout 0 P2WSH 11,111 sats; vout 1 P2WPKH 111,444 sats; vout 2 P2WSH 36,369 sats; vout 3 P2WPKH 21,369 sats |
| Fee | 113 sats; inputs and outputs balance exactly |
| Confirmation | Confirmed in Testnet3 block 5,127,974 before the playbook was frozen |
| Classification | Setup transaction only; it predates and is not counted as any M8 P02/P04/P07 ceremony |
| Operator note | Broadcast before the detailed preparation receipt was written; no protected ceremony was active, and the confirmed outputs now provide the intended test inventory |
| Next use | vout 1 + 3 form the shared P02 input set; one P2WSH output will later form the single-input M8-X01 mixed-device case |

## Shared P02 unsigned fixture FP-M8-P02-01

| Item | Recorded value |
| --- | --- |
| Evidence file | `run/m8-evidence/M8-P02-shared-unsigned.psbt` |
| Size / SHA-256 | 586 bytes / `b034ca05e2180611fcd357400fc997838a37348eead460359b47a2f60d71dbac` |
| Immutability | Exact byte-for-byte copy of the operator-saved PSBT; destination marked read-only before either device signs |
| Network / account | Testnet3; native P2WPKH; fingerprint `0fb882ff`; account origin `m/84'/1'/0'` |
| Unsigned transaction ID | `ffa3da5bbd7672de91075d3657dbb1fd200b77fa7921134fe2913126fb2ae3ba` |
| Inputs | `61a05816...079da:1`, 111,444 sats, path `m/84'/1'/0'/1/0`; `61a05816...079da:3`, 21,369 sats, path `m/84'/1'/0'/0/1` |
| Outputs / fee | 11,111 sats recipient; 121,598 sats change; 104-sat fee; inputs and outputs balance exactly |
| Slot/signature state | Two distinct locally controlled signing slots; zero partial signatures; zero finalized inputs |
| Validation | Complete strict map parse, exact unsigned-transaction match, derivation/fingerprint checks, and transaction/value assertions passed |
| Supporting receipt | `run/m8-evidence/M8-P02-shared-unsigned-receipt.md`; setup screenshot SHA-256 `42ac08d5a3e7d0436013d5713c10bd2eb989f23ec3a2aff8d922b1b340a1159b` |
| Shared-use rule | M8-C/P02 and M8-D/P02 must each reopen this file independently and write separate signed results; neither result is broadcast |
| Ceremony state at freeze | No M1, session, response, or signature had been created |

## Matrix ledger

| Cell | P01 baseline | Positive suite | Continuity suite | Negative suite | Soak | Final |
| --- | --- | --- | --- | --- | --- | --- |
| M8-A CLI/SeedSigner | Not run | Not run | Not run | Not run | N/A unless selected | Open |
| M8-B CLI/Kern | Not run | Not run | Not run | Not run | Not run | Open |
| M8-C GUI/SeedSigner | Pass (attempt 01; independently reviewed) | P02 pass (independently reviewed) | Not run | Not run | N/A unless selected | Open |
| M8-D GUI/Kern | Pass (attempt 02; attempt 01 failure preserved) | P02 pass (independently reviewed) | Not run | Not run | Not run | Open |

## M8-D/P01 attempt 01 — live P2WPKH baseline

| Field | Value |
| --- | --- |
| Matrix cell | M8-D GUI/Kern |
| Result | `FAIL` — safe preflight rejection |
| Started / completed | 2026-09-01 12:26 EDT / 2026-09-01 |
| Coordinator | Sparrow `182bc8a7b24641e43cecf324e96eec6314f9b18b`, BR-2026-09-01-04 |
| Signer | Kern DR-KERN-01 |
| Network / profile | Testnet3 / `aext-v1` |
| Wallet / inputs / slots | public fingerprint `0fb882ff`; native P2WPKH; one input; one controlled slot |
| Frozen-PSBT SHA-256 | `4c9b55d10d4ec0686a282784315c7fdc44f76d36425aa5ebfa00766cc34a6bfd` (559 bytes) |
| M1 length / SHA-256 | 790 / `5faa5dd8b6767b7235b2eabc4b4accfabf51cfaa0f746bbb8d695bc6284a185e` |
| M2 / M3 / M4 | Not created |
| Observed outcome | Kern displayed `Protected preflight failed` / `AE_SIGNATURE_SLOT_MISMATCH` and returned home |
| Signatures cryptographically verified | N/A; no response, opening, commitment, or signature was created |
| Broadcast attempted | No |
| Root cause | PSBT contains agreeing witness and non-witness UTXOs; the anti-exfil path read PSBT-v2-style input members instead of the PSBT-v0 unsigned-transaction prevout exposed by libwally getters |
| Evidence | `run/m8-evidence/M8-D-P01-attempt-01-receipt.md`; isolated Sparrow log SHA-256 `bba2897065bd66c8938e48daefc1e7db12004619c5bdb6f5115a42e17b339e25`; serial log SHA-256 `898ace45ac2d866953dcceeffd6548e22597e22c2cf766057b2db7140efe629c` |
| Remediation | Kern `5180dbb`; exact live regression plus DR-KERN-02 build/flash receipt |
| Follow-up | Attempt 01 is immutable. Attempt 02 must create a fresh Sparrow session after independent review; do not reuse its `.aexs` or `.aexj`. |

## M8-D/P01 attempt 02 — live P2WPKH baseline

| Field | Value |
| --- | --- |
| Matrix cell | M8-D GUI/Kern |
| Result | `PASS` |
| Started / completed | 2026-09-01 16:37 EDT / 2026-09-01 16:49 EDT |
| Coordinator / signer | Sparrow `182bc8a7b24641e43cecf324e96eec6314f9b18b` / Kern DR-KERN-02 |
| Network / profile | Testnet3 / `aext-v1` |
| Wallet / inputs / slots | public fingerprint `0fb882ff`; native P2WPKH; one input; one controlled slot |
| Frozen-PSBT SHA-256 | `c88217365dba5a9d2caca02dfa7ded5f8a67c8e7d085e6ab4df30fb0c9fbbeed` (559 bytes) |
| M1 | 790 bytes / `6a88cb063845cfa63df52522b562a7e665940bb1f84b3c0dc0dea5148db4e8ee` |
| M2 | 264 bytes / `0ceba9d71c63db76bc3031089d39da13deb438f2aa4f8303dc59e896095f3347` |
| M3 | 855 bytes / `329e316c111dc5628bb1674cd360e008627baa26d9c68d0a5fd8606e2c6f08be` |
| M4 | 328 bytes / `3a91446b7e3bd60ff661d22bc6b6486f411af55e37b10f8cfdb6af484a5c0004` |
| Final reconstructed PSBT | 667 bytes / `b717c65b22ef616900750c0eea5320cf78fdcca0e598e12f34d83bf7a64b85cb` |
| Verification | Sparrow accepted complete openings, exact-session reveal, S2C proof, and ECDSA signature; AEXS phase is `COMPLETE`; one complete Kern signature displayed |
| Transaction review | input 180,406 sats; recipient 11,111; change 169,225; fee 70; reviewed independently at both stages |
| User control | explicit `Create commitments` and independent `Create signatures` approvals; M2 and M4 scanned by Sparrow before Kern Done |
| Transport boundary | Kern returned only M2/M4 protected URs; no ordinary signed PSBT returned by signer |
| Broadcast attempted | No |
| Resource result | heap recovered after both viewers; largest block/lifetime minimum stable; stack high-water 10,236 throughout both signer phases |
| Cleanup | Sparrow closed; Kern Anti-exfil and all ordinary signing toggles Off; isolated home and both attempt records preserved |
| Evidence | `run/m8-evidence/M8-D-P01-attempt-02-receipt.md`; Sparrow log `dbaf1ba0...93f84`; Kern serial log `07dd5ad8...6d8782`; two completion screenshots and signed PSBT hash-pinned in the receipt |
| UX observations | Stage-3 disclosure looks like a warning despite being informational; completion text says to scan M4 even after it has already been scanned |

## M8-C/P01 attempt 01 — live P2WPKH baseline

| Field | Value |
| --- | --- |
| Matrix cell | M8-C GUI/SeedSigner |
| Result | `PASS` pending independent review |
| Started / completed | 2026-09-02 10:06 EDT / 2026-09-02 10:24 EDT |
| Coordinator / signer | Sparrow `182bc8a7b24641e43cecf324e96eec6314f9b18b` / SeedSigner DR-SEEDSIGNER-01 |
| Network / profile | Testnet3 / `aext-v1` |
| Wallet / inputs / slots | public fingerprint `0fb882ff`; native P2WPKH; one 180,406-sat input; one controlled slot |
| Frozen PSBT | 443 bytes / `015dcae44cf373c38f41723165c6ebc8baacb08b85c6a350a476d4ad755de97f` |
| M1 | 674 bytes / `b63ee4d195384642c6d14a96885e36a58c84b63454090ac7f3d1d2b2516908e4` |
| M2 | 264 bytes / `bc4b58247ab8e6bb362183c3cd66a39cbf90cdd533ca4cd5ccf8af528c4826f3` |
| M3 | 739 bytes / `d38049b53a7c611f21764a87213f862286ebe5af28c9ce1892de21ce19311aa5` |
| M4 | 328 bytes / `2280486467839556f26e355333a50ef9a26d0428b0467a7c3d0389ec8753ccea` |
| Final reconstructed PSBT | 551 bytes / `47a6db1f3dc735e5cf6f5195f6986c28404e8770b83c7d4d54ab1d801e7618a6` |
| Verification | Sparrow completed exact-session opening/S2C/ECDSA verification; independent host parse found one controlled slot and one valid signature and reproduced the frozen PSBT after removing only that signature |
| Transaction review | recipient 11,111 sats; change 169,225; fee 70; independently reviewed at both stages with no warnings |
| User control | explicit `Create nonce commitment` and independent `Approve protected signature` actions; M2 was accepted and M3 persisted before SeedSigner's first response QR was dismissed |
| Broadcast attempted | No |
| Cleanup | Sparrow closed; SeedSigner anti-exfil Disabled; disposable seed discarded; dedicated M8-C profile and evidence preserved |
| Evidence | `run/m8-evidence/M8-C-P01-attempt-01-receipt.md`; Sparrow log `18d5ae60...56dea`; completion screenshot `666a263f...3f0f`; original and signed PSBTs preserved |
| UX observation | SeedSigner's animated response viewer has no labeled Done control; two button presses exited M4 to the main menu |

The 73-byte abort-journal SHA-256 equals M8-D attempt 01's journal because
both files encode the same deterministic empty-journal state. It is not
evidence of session reuse; the M8-C session ID, profile, frozen PSBT, and
durable session file are distinct.

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
| Sparrow build receipt reviewed | Pass | KimiK3 independently matched the BR-2026-09-01-04 JAR SHA-256 and verified the supersession and identity chain |
| Physical playbook reviewed | Pass | Seed custody, network identity, scan order, and automatic evidence confirmed |
| Kern importer-registry delta reviewed | Pass | KimiK3 approved Sparrow `3b565e3` and Kern evidence commit `3696c57`; UI-only registration change, no protocol or policy-registry delta |
| Kern asset-sizing delta reviewed | Pass | KimiK3 approved Sparrow `a967c0a` and Kern evidence commit `ff1dcbe`; assets-only correction with physical layout verification |
| Anti-exfil receipt logging delta reviewed | Pass | KimiK3 approved Sparrow `182bc8a` and Kern evidence commit `fec7348`; single-class INFO override with runtime effective-level assertion |
| M8-D/P01 attempt 01 | Fail | Safe preflight rejection recorded above; no signer response or transaction signature created |
| Anti-exfil preflight fix reviewed | Pass | KimiK3 reviewed Kern `5180dbb`, evidence commit `e8f3292`, DR-KERN-02, and the immutable attempt-01 evidence; getter fix and exact live regression accepted |
| DR-KERN-02 ceremony-start identity | Pass | COM6 application partition read back before scanning: 1,970,176 bytes, SHA-256 `e7df2b55d7c476a0ff67c06aebcd55e7fdf3b7027de98b58022828e85e1f893a`, exact artifact match |
| M8-D/P01 attempt 02 authorized | Yes | Fresh Sparrow transaction/session required; attempt-01 `.aexs` and `.aexj` remain immutable and must not be reused |
| M8-D/P01 attempt 02 outcome | Pass | Full M1-M4 ceremony, coordinator verification, signed-PSBT reconstruction, cleanup, and no-broadcast gate all passed; UX wording observations are non-blocking |
| M8-D/P01 attempt 02 evidence review | Pass | KimiK3 accepted commit `ef4bf9e`, independently matched the signed PSBT, screenshots, serial log, firmware readback, resource transcription, cleanup boundary, and immutable attempt-01 evidence; M1-M4/session hashes retained their stated evidence-boundary caveat |
| DR-SEEDSIGNER-01 image receipt | Pass | KimiK3 reviewed Kern `d19a2773`, independently matched the app/OS/Buildroot/image identity chain and both byte-exact boot receipts, and accepted the operator-versus-artifact evidence boundary |
| M8-C/P01 authorized | Yes | Fresh Sparrow transaction/session required under DR-SEEDSIGNER-01; preserve M8-D evidence and do not reuse any prior ceremony state |
| M8-C/P01 attempt 01 outcome | Pass | Full M1-M4 ceremony, independent approvals, coordinator verification, signed-PSBT reconstruction, cleanup, and no-broadcast gate passed |
| M8-C/P01 attempt 01 evidence review | Pass | KimiK3 accepted Kern `1281b54`; independently re-parsed the durable session, recomputed BIP143 and ECDSA, stripped exactly one signature to reproduce the frozen PSBT, and matched the log/screenshot/artifact hashes |
| FP-2026-09-02-01 | Pass as fixture preparation | Public Testnet3 transaction independently matched its source, four intended outputs, 113-sat fee, and confirmation in block 5,127,974; it is not counted as an M8 protocol pass |
| P02 and M8-X01 physical playbook | Pass | KimiK3 accepted Kern `efcf47d` for Parts A-C and then accepted `f259fda` as satisfying the review checkpoint before Part D. |
| M8-C/P02 attempt 01 outcome | Pass | Two-input/two-slot SeedSigner ceremony completed over FP-M8-P02-01; both signatures independently verified, signature removal reproduced the frozen PSBT exactly, cleanup completed, and broadcast was not attempted. |
| M8-C/P02 attempt 01 evidence review | Pass | KimiK3 reviewed Kern `f259fda`, re-hashed all receipted artifacts, reproduced the transcript and signature checks independently, and accepted the SeedSigner result. |
| M8-D/P02 attempt 01 outcome | Pass | The byte-identical fixture completed on Kern with two verified signatures, stable physical resource measurements, exact signed-PSBT/final-transaction reconstruction, complete cleanup, and no broadcast. |
| M8-D/P02 attempt 01 evidence review | Pass | KimiK3 reviewed Kern `f259fda`, independently matched the closed Sparrow and serial logs, durable AEXS state, resource measurements, signatures, and final transaction. |
| P02 shared-fixture comparison | Pass | Both devices used the same ordered slots and sighashes; all four low-S ECDSA signatures verify; both results recover the exact frozen PSBT and reconstruct valid complete transactions. KimiK3 independently reproduced these findings. |
| M8-X01 authorized | Yes | Part D may start under the frozen playbook. Part G broadcast remains separately gated behind review of the pristine, intermediate, and final PSBT evidence. |
| M8-X01 Part D pristine fixture | Pass | One-input native-P2WSH PSBT frozen read-only before signing: 1,607 bytes, SHA-256 `483294e990e74f412a31719b765147ea20c403bf8b2200c4722bb4c7fb0edec0`; strict independent inspection passed and Part E Kern-first signing may start. |
| M8-X01 attempt 01 | Fail safe | BR-2026-09-01-04 addressed the sole Required reserve SeedSigner instead of Optional Kern; Kern rejected the mismatched slot before M2 and no signature or response was created. Receipt: 4,776 bytes, SHA-256 `ad3f641c3b224a60816b4479d424e494a9bd5c3b8bd853ab451a8091b19331a5`. |
| Mixed-policy signer-selection fix | Pass | KimiK3 accepted Sparrow `a53d9e1`: chooser eligibility is compatibility-only while every Required provenance, software-signing, UI, and editor enforcement path remains intact. |
| BR-2026-09-02-01 | Pass | KimiK3 independently matched the JAR SHA-256, clean source identity, submodule pins, build environment, artifact sizes, and passing focused/full anti-exfil tests. |
| M8-X01 attempt 02 authorized | Yes | Use BR-2026-09-02-01, a fresh coordinator session, the corrected policy table, explicit `Kern (0fb882ff)` chooser selection, and the Kern QR-header stop gate before scanning. |
| M8-X01 attempt 02 Parts E/F | Pass | Explicit Kern-first and `b4899a09` SeedSigner-second sessions completed; Sparrow retained both protected signatures, reached exactly 2-of-3, and did not broadcast. Receipt: 10,438 bytes, SHA-256 `f8b576923e61ac73bbcdfabc54b2a2fefd144206bac3bdb22dbe554065596d59`. |
| M8-X01 independent verifier | Pass | Pristine → Kern intermediate → SeedSigner signed state and final witness reconstruction pass; both low-S ECDSA signatures verify, Kern's signature is retained exactly, and reserve `2a0726f2` is absent. KimiK3 reproduced these results with independent parser/curve code. |
| M8-X01 attempt 02 evidence review | Pass | KimiK3 hash-matched all 18 artifacts, independently decoded both complete AEXS transcripts, reconstructed the byte-exact lineage and final witness, verified both signatures, and matched the UI/resource evidence. |
| M8-X01 mixed-device signing | Pass | One Kern protected signature followed by one SeedSigner protected signature completed the intended native-P2WSH 2-of-3 transaction without the reserve signer. |
| M8-X01 Part G broadcast authorized | Yes | KimiK3 authorized exactly one Testnet3 broadcast of txid `6854c031978839983e6cb2bcf064a432f126fbbbdbec7c969e9202e554442496`; record timestamp, response, explorer URL, and confirmation height. |
| M8-X01 Part G broadcast outcome | Pass, initially unconfirmed | Sparrow broadcast exactly once and displayed the authorized txid as `Unconfirmed`, signed by `SeedSigner 2, Kern`. A public Testnet3 API returned the exact transaction with `confirmed: false`, the expected input, 25,055/11,111-sat outputs, and 203-sat fee. |
| M8-X01 post-broadcast cleanup | Pass | Sparrow closed and isolated profile preserved; Kern all signing toggles Off and seed unloaded; SeedSigner anti-exfil Disabled and all seeds discarded. |
| M8-X01 confirmation | Pass, pending review | Confirmed in Testnet3 block `5,128,025`, block hash `00000000000000b7f5b9c43ab49553c6d8b7a11f828810386d8e647ac8e292e6`, block time `2026-09-03T03:50:12Z`; separate confirmed API response preserves the update without altering initial evidence. |
| M8-X01 post-broadcast evidence review | Pending | Review `M8-X01-attempt-02-broadcast-receipt.md`, 3,810 bytes, SHA-256 `235bc43fc48e13002ce5f140bb560da6aaf4ffac9ad165d10b26058ce7444f9e`, plus its unconfirmed/confirmed responses, screenshot, final closed Sparrow log, and cleanup statement. |

The follow-up review covered Sparrow `b0463a2` and `0f9029a` plus Kern docs
`5efa6ce`. It found no protocol/validation changes, no blocking issue, and
accepted advancement from preflight to M8-D/P01 attempt 01.

Setup observation SO-2026-09-01-01 subsequently exposed a missing visible Kern
importer. Because the operator stopped before transaction creation, the earlier
review remains valid background evidence. KimiK3's delta review approved the
narrow Sparrow `3b565e3` fix and replacement build receipt, so live
authorization is reinstated with keystore replacement as a mandatory pre-step.

Setup observations SO-2026-09-01-02 and SO-2026-09-01-03 then exposed the
Kern SVG sizing and coordinate-bound defects before replacement was applied.
KimiK3's final delta review approved Sparrow `a967c0a`, independently matched
BR-2026-09-01-03, and accepted SV-2026-09-01-01. Live authorization is
reinstated with application of the already validated identical Kern
replacement as the mandatory final setup step.

Setup observation SO-2026-09-01-04 then found the canonical INFO receipts
would be suppressed by Sparrow's global WARN threshold. Because the operator
had not created the transaction, live authorization is paused without
consuming attempt 01 pending review of the scoped Sparrow `182bc8a` logging
override and BR-2026-09-01-04.

KimiK3's final receipt-logging delta review approved the class-scoped INFO
override, independently matched the BR-2026-09-01-04 JAR, and accepted the
recorded supersession chain. Live authorization is reinstated with the first
Stage-1 runtime receipt as an immediate stop gate if absent.

M8-D/P01 attempt 01 then began under the approved identities. Sparrow emitted
and durably retained canonical M1, but Kern rejected it during authoritative
slot preflight with `AE_SIGNATURE_SLOT_MISMATCH`. Investigation pinned the
failure to direct access of PSBT input prevout members that are not
authoritative for PSBT v0. No response-construction marker appeared and M2-M4
were never created. Kern `5180dbb` uses the libwally PSBT getters and adds the
exact live PSBT as a regression; DR-KERN-02 records the passing suites, signed
firmware hash, and successful flash. Attempt 02 remains unauthorized until an
independent reviewer accepts that checkpoint; this was the state before the
review recorded below.

KimiK3's delta review accepted the getter-based fix, the full live regression,
the immutable failure classification, and the session non-reuse boundary. At
ceremony start, the exact application partition was read back from COM6 and
matched DR-KERN-02's signed `kern.bin` SHA-256 byte for byte; the NVS/seed
partition was not read. All review conditions are therefore satisfied and
M8-D/P01 attempt 02 is authorized using a newly created Sparrow transaction and
session only.

Attempt 02 used a distinct M1, frozen PSBT, and durable session. Kern passed
both authoritative preflights and independent approvals, Sparrow durably
advanced M2 to M3 before Kern's first Done action, and Sparrow accepted and
verified M4 before Kern's second Done action. The coordinator reconstructed a
667-byte signed PSBT and displayed one complete Kern signature; broadcast was
not attempted. Both viewer cleanups restored heap, the old failed session and
abort journal remained hash-identical, all toggles were returned Off, and the
isolated Sparrow home was preserved. M8-D/P01 is therefore `PASS`; the broader
positive, continuity, negative, and soak gates remain open.

KimiK3's independent review accepted the `PASS` classification and confirmed
that both wording observations are non-blocking UI polish. It recommended
keeping the remaining matrix and coverage gates open and advancing the formal
baseline order to M8-C/P01, Sparrow GUI to SeedSigner.

DR-SEEDSIGNER-01 then froze the newly built final SeedSigner artifact for that
cell. The clean instrumented Pi Zero build completed successfully and its
52,428,800-byte image independently matched the build-emitted SHA-256. The
operator reports a successful flash and normal boot with the disposable seed
loaded, Testnet selected, persistent settings enabled, and anti-exfil policy
set to `Required`. The SD boot partition then yielded exit code `0` and a
byte-preserved self-test receipt naming the native secp256k1-zkp backend,
matching both opening and signature vectors, and reporting
`production_fallback: false`. M8-C/P01 remains `NOT RUN` pending independent
review of DR-SEEDSIGNER-01 before Sparrow creates message 1.

KimiK3's independent review of `d19a2773` accepted DR-SEEDSIGNER-01 and
authorized M8-C/P01. It independently resolved the app tag and commit, matched
the SeedSignerOS and Buildroot pins, re-hashed the 50 MiB image and both boot
receipts, and confirmed that operator-reported flash/settings observations are
not presented as artifact-derived facts. The build-log statements remain
process evidence and the SeedSignerOS checkout is shallow-grafted; neither
caveat blocks this physical baseline. M8-C/P01 must use a fresh Sparrow
transaction and session, and remains `NOT RUN` until its first message is
created.

M8-C/P01 attempt 01 then completed a fresh four-message session using a
separate Sparrow profile and the accepted SeedSigner image. SeedSigner showed
matching transaction values with no warning at both independent approval
stages. Sparrow accepted M2 before M3 was displayed, then accepted M4 and
showed one complete SeedSigner signature plus the final transaction; broadcast
was not attempted. A separate host-side parse verified the only partial
signature against the locally recomputed sighash and reconstructed the frozen
PSBT after removing only that signature. Sparrow was closed, SeedSigner was
returned to Disabled, and the test seed was discarded. The baseline is marked
`PASS` pending independent review; every broader M8-C suite and both remaining
reference-tool cells stay open.

KimiK3's independent review accepted M8-C/P01 as `PASS`. It matched every
preserved artifact hash and M1-M4 log receipt, decoded the AEXS version and
`COMPLETE` phase, independently recomputed the BIP143 sighash and verified the
low-S signature without relying on the project crypto libraries, and removed
exactly that signature to recover the byte-identical frozen PSBT. It also
confirmed the setup-only model correction, cleanup boundary, and SeedSigner
QR-exit observation. M2 and M4 remain hash-only package evidence because their
canonical bytes were not separately retained. The broader M8-C suites and
M8-A/M8-B remain open.

M8-C/P02 attempt 01 then reopened the exact read-only FP-M8-P02-01 fixture and
completed a fresh four-message SeedSigner session over two P2WPKH inputs and
two distinct controlled slots. Both device reviews showed the expected
transaction values without warning and required independent approval. The log
contains one byte-identical pre-reveal M1 redisplay, with no session, PSBT,
challenge, or package substitution. Sparrow accepted M4, reconstructed an
800-byte signed PSBT, and enabled—but did not use—broadcast. Independent host
verification recomputed both BIP143 sighashes, verified both low-S ECDSA
signatures, matched both final witness stacks, and recovered the 586-byte
frozen PSBT after removing only the two partial signatures. Sparrow was closed,
SeedSigner anti-exfil was disabled, and `0fb882ff` was discarded. M8-C/P02 is
`PASS` pending independent review; M8-D/P02 remains the next authorized case,
and M8-X01 remains gated behind review of both P02 receipts and their shared
comparison.

M8-D/P02 attempt 01 then reopened the byte-identical FP-M8-P02-01 fixture in
the dedicated Kern Sparrow profile. Kern showed both inputs and the same
recipient, change, and fee at two independent approval screens without a
warning. Sparrow accepted M2 before Kern's first Done action, then accepted M4,
verified two protected signatures, and reconstructed the complete result.
Independent verification recovered the frozen PSBT after removing only Kern's
two partial signatures, recomputed the same two SeedSigner-case sighashes,
verified both low-S ECDSA signatures, matched Sparrow's direct finalized PSBT
witnesses, and matched its saved final transaction byte for byte. Physical
measurements stayed stable across both rounds and both viewer cleanups. Sparrow
was closed, all Kern signing toggles were returned Off, and broadcast was not
attempted. M8-D/P02 and the pairwise comparison are `PASS` pending independent
review; M8-X01 remains unauthorized.

While resolving the P02 protocol IDs, the earlier M8-C/P01 receipt's
`d0fdef...` value was identified as Sparrow's wallet storage directory rather
than the AEXB `session_id`. The accepted receipt is preserved unchanged; a
separate dated erratum records the actual M8-C/P01 ID and both directly decoded
P02 IDs. This corrects an evidence label only and does not alter any transcript,
PSBT, signature, or result.

KimiK3's independent review of Kern `f259fda` accepted all five requested P02
findings. It re-hashed all 19 receipted files, reproduced `validation=PASS`,
and used a separate parser and pure-Python secp256k1 verifier to confirm the
same frozen PSBT, ordered slots, BIP143 sighashes, four low-S signatures, final
transactions, and expected per-session divergence. It also accepted the
session-ID erratum as an evidence-label correction with no change to protocol
bytes or prior results. M8-C/P02, M8-D/P02, and their shared comparison are now
independently accepted. M8-X01 Part D is authorized; its one Testnet3 broadcast
remains gated behind review of the pristine, intermediate, and final evidence.

M8-X01 attempt 01 then created M1 under BR-2026-09-01-04, but the QR dialog
said to scan the commitment with SeedSigner. The wallet's actual policy state
had `2a0726f2` Required while both `b4899a09` and Kern `0fb882ff` were
Optional. Sparrow's selection helper therefore removed both Optional signers
and silently selected the sole Required reserve SeedSigner. Decoding the
retained M1 confirms its only slot carries `2a0726f2`'s pubkey rather than
Kern's. Kern correctly rejected it twice with `AE_SIGNATURE_SLOT_MISMATCH`;
there is no response-construction marker and M2-M4 were never created.

The failed attempt, phase-0 AEXS, journal, logs, prompt, and wallet-policy
screenshots are frozen in `M8-X01-attempt-01-receipt.md`. Sparrow `a53d9e1`
now treats compatibility—not policy rank—as chooser eligibility. Required
still governs provenance enforcement for signatures attributable to that
keystore. The replacement build is BR-2026-09-02-01. M8-X01 attempt 02 is
paused pending independent review and must use a fresh session plus explicit
selection of `Kern (0fb882ff)`.

KimiK3 independently accepted Sparrow `a53d9e1`, Kern `88c1571`,
BR-2026-09-02-01, and the attempt-01 safe-failure classification. It confirmed
that Required enforcement remains intact, the regression exactly models the
mixed wallet, the retained phase-0 AEXS and logs prove no M2 or signer response
was created, and every replacement-build identity matches. Attempt 02 is now
authorized under the boundaries above. The reviewer noted non-blockingly that
future safe-failure receipts should preserve raw M1 as its own artifact; the
phase-0 AEXS is conclusive for this accepted attempt, so it will not be rerun.

M8-X01 attempt 02 then used BR-2026-09-02-01 and the corrected policy table.
Sparrow displayed all compatible signers; the operator explicitly selected
Kern `0fb882ff`, confirmed the Kern-labeled M1 header, and completed both
independently approved rounds. Sparrow produced a 1,715-byte intermediate with
exactly Kern's protected signature and no final/broadcast state. Removing that
signature reproduces the pristine PSBT byte for byte. Kern's heap, largest
block, lifetime minimum, and stack measurements stayed stable; both viewer
cleanups returned to the same free-heap value.

The operator next explicitly selected SeedSigner `b4899a09`. SeedSigner asked
for the already registered descriptor again, so the operator closed M1, loaded
the descriptor, and reopened the protected flow. Sparrow redisplayed the same
1,946-byte M1 with the same hash, proving no ceremony substitution. Both
SeedSigner rounds then completed with independent approvals. Sparrow retained
Kern, added SeedSigner 2, reached exactly 2-of-3, and enabled—but did not use—
View Final Transaction and Broadcast Transaction.

The pre-broadcast receipt and independent verifier preserve both completed
AEXS records, logs, journals, provenance index, pristine/intermediate/final
artifacts, and screenshots. The verifier independently recomputes the BIP143
sighash, verifies both low-S ECDSA signatures, proves the exact Kern signature
survived into the final witness, excludes the reserve signer, and reconstructs
the saved 392-byte transaction exactly. Parts E/F are `PASS` pending review.
Part G remains unauthorized.

KimiK3 then independently hash-matched all 18 pre-broadcast artifacts and
re-derived both complete transcripts, the pristine/intermediate/final lineage,
the shared BIP143 digest, both low-S ECDSA signatures, exact Kern-signature
retention, reserve-signer absence, final witness ordering, txid, and wtxid. It
also matched the screenshots and Kern resource measurements. Parts E/F and
the mixed-device signing result are accepted as `PASS`. Exactly one Testnet3
broadcast of txid
`6854c031978839983e6cb2bcf064a432f126fbbbdbec7c969e9202e554442496`
is authorized; post-broadcast evidence and cleanup remain required.

The operator then pressed Broadcast Transaction exactly once. Sparrow showed
the authorized txid as `Unconfirmed` and identified `SeedSigner 2, Kern` as its
signers. A separately saved public Testnet3 response returned the exact txid,
the intended `61a05816...:2` input, 25,055-sat change, 11,111-sat recipient,
203-sat fee, expected two-signature witness, and `confirmed: false`. The
initial screenshot and response are read-only and hash-pinned in
`M8-X01-attempt-02-broadcast-receipt.md`; their capture time is not presented
as the network's exact first-seen time.

After capture, Sparrow was fully closed and its isolated profile preserved.
Kern was left with all signing toggles Off and its seed unloaded. SeedSigner
anti-exfil was set to Disabled and all seeds were discarded. The closed
Sparrow log was frozen read-only. Part G's broadcast action and cleanup are
therefore `PASS`. A later public response confirmed the exact transaction in
Testnet3 block `5,128,025` at `2026-09-03T03:50:12Z`; the response is preserved
separately so the initial unconfirmed evidence remains unchanged. Independent
post-broadcast review remains pending. The observed Kern stage transition—returning to Home after
Step 1 without directing the user to resume Step 2—is an explicit next-session
UX/lifecycle task, not a completed M8 protocol gate.
