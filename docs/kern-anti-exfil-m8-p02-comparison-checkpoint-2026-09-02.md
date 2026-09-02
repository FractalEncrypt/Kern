# M8 P02 shared-fixture comparison checkpoint

Date: 2026-09-02
Status: M8-C/P02 and M8-D/P02 pass locally; independent review pending
Scope: Sparrow GUI with SeedSigner and Kern, two-input native P2WPKH

## Frozen input

Both ceremonies independently reopened the same read-only Testnet3 PSBT:

- size: 586 bytes;
- SHA-256:
  `b034ca05e2180611fcd357400fc997838a37348eead460359b47a2f60d71dbac`;
- unsigned txid:
  `ffa3da5bbd7672de91075d3657dbb1fd200b77fa7921134fe2913126fb2ae3ba`;
- inputs: `61a05816...079da:1` (111,444 sats) and `:3` (21,369
  sats);
- outputs: 11,111 sats recipient and 121,598 sats change;
- fee: 104 sats; and
- controlled signer: `0fb882ff`, with one distinct BIP84 child key per
  input.

The fixture was frozen before either device ceremony. Each attempt created its
own session and neither consumed the other's signed result.

## Results

| Property | SeedSigner M8-C/P02 | Kern M8-D/P02 |
| --- | --- | --- |
| Result | Pass pending review | Pass pending review |
| Protocol session ID | `fd028c19...09bb44a` | `d2357ecc...4cb4e7e` |
| M1 bytes / SHA-256 | 922 / `fc325607...6a7823` | 922 / `441a68a5...0d64e4` |
| M2 bytes / SHA-256 | 402 / `3058208f...a77e7f` | 402 / `69a29da3...c8d502` |
| M3 bytes / SHA-256 | 1,052 / `a7a0bba6...b9a78` | 1,052 / `9db5fbb0...fc3974` |
| M4 bytes / SHA-256 | 530 / `1b64d94e...d63590` | 530 / `6f638afb...cfabab` |
| Signed PSBT | 800 bytes / `2c7bb496...e96209` | 801 bytes / `6594cd5d...4b9bd4` |
| DER lengths | 71, 71 | 71, 72 |
| Final txid | `ffa3da5b...ae3ba` | `ffa3da5b...ae3ba` |
| Final wtxid | `e51f6916...82fcb2` | `d95d0483...c15d30` |
| Broadcast | No | No |
| Cleanup | Sparrow closed; signer disabled/seed discarded | Sparrow closed; all signer toggles Off |

Different sessions, host randomness, signatures, signed-PSBT hashes, and
wtxids are expected. The comparison gate is semantic and cryptographic, not
byte-identical signature output.

## Independent pairwise verification

The local verifier `run/m8-evidence/verify_m8_p02_pair.py`, SHA-256
`8e9e5adbb6a151353f7b1bfb5ff3bfdd2cbe030151cb0c81e037b479473551a1`,
performed these checks without using either project crypto wrapper:

1. parsed both signed PSBTs and the frozen PSBT with strict CompactSize and
   unique-map-key checks;
2. removed only input partial signatures and recovered the exact frozen PSBT
   from each result;
3. found the same two ordered pubkeys and recomputed the same two BIP143
   sighashes in both results;
4. decoded all four DER signatures, enforced low-S, and independently verified
   secp256k1 ECDSA;
5. reconstructed both complete witness transactions;
6. matched the SeedSigner transaction to its saved final hex;
7. matched Kern's direct finalized-PSBT witnesses to its session PSBT; and
8. matched Kern's saved final transaction byte-for-byte to the independent
   reconstruction.

Shared sighashes:

- input 0:
  `210465912d441418bf7a25a2d80064ccced428fb26d2cd5cfcc3d7170c992405`;
- input 1:
  `b56f019fa0883c88ef2c75335662ab3450ba54a7d045099431c840cee090a05c`.

The verifier exits successfully with `validation=PASS`.

## Evidence receipts

The mutable test workspace holds the detailed, read-only evidence records. The
tracked checkpoint pins them so review cannot silently follow a later local
edit:

| Evidence file | Bytes | SHA-256 |
| --- | ---: | --- |
| `M8-C-P02-attempt-01-receipt.md` | 6,325 | `018eb0f1baa5319558aaf1bf9776acd15fe643c855b2eff81633fbe75a755bb4` |
| `M8-D-P02-attempt-01-receipt.md` | 6,713 | `ab167a1c1e781d26d0af7f58e989d6aeca1acfa8a7496a941ba06616cbd0b976` |
| `M8-session-id-label-erratum-2026-09-02.md` | 1,321 | `8201c5eb832d79f706eb2572a1a399b00914b3ea30a0a4d14c974ed6694a8c2b` |
| `verify_m8_p02_pair.py` | 11,165 | `8e9e5adbb6a151353f7b1bfb5ff3bfdd2cbe030151cb0c81e037b479473551a1` |

## Approval and continuity evidence

Both devices displayed the complete transaction twice and required distinct
approval actions before M2 and M4. In each case Sparrow accepted M2 and
persisted/emitted M3 before the signer response viewer was dismissed. Neither
device returned an ordinary signed PSBT. Sparrow verified M4 and reconstructed
the signed result before enabling broadcast.

SeedSigner's log contains one byte-identical M1 redisplay before M2. It retained
the same protocol session, frozen PSBT, host challenge, and package hash; no
post-reveal retry occurred. Kern's serial evidence additionally shows stable
heap, largest-block, minimum-watermark, and stack measurements across both
rounds, with cleanup after both viewers.

Both no-abort journals have SHA-256
`ae4d30335fa0144bc7d5d47acf97d1ffac60c52a0af74cd34d012049451d65d6`.
Neither transaction was broadcast.

## Session-ID evidence erratum

The accepted M8-C/P01 receipt called Sparrow's first session-storage directory
`d0fdef...` the coordinator session ID. That directory is the wallet storage
component; the protocol session ID resides inside each AEXB header. The
accepted receipt remains immutable. The local
`M8-session-id-label-erratum-2026-09-02.md` records the actual P01 ID and the
directly decoded P02 IDs. No protocol bytes or prior verification result are
changed by this label correction.

## Review gate

Requested independent findings:

1. accept or reject M8-C/P02 as a two-input/two-slot SeedSigner pass;
2. accept or reject M8-D/P02 as the equivalent Kern pass;
3. confirm the pair shares the exact unsigned fixture, ordered slot set, and
   sighashes while producing independently valid device signatures;
4. confirm the session-ID erratum is accurate and appropriately preserves the
   accepted P01 receipt; and
5. authorize M8-X01 only if all preceding findings pass.

Until that review is accepted, the native-P2WSH mixed Kern-then-SeedSigner
transaction and its one deliberate Testnet3 broadcast remain prohibited.
