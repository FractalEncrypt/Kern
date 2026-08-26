# Kern/Lark anti-exfil collaboration checkpoint

Date: 2026-08-26

This checkpoint distinguishes cryptographic, semantic, and carriage evidence.
Passing a profile parser is not reported as transport-neutral protocol
conformance.

## Pinned external evidence

- Lark/Jade four-stage transcript: commit
  `8f53be92ca7a49659d852ca58853740ce9a1625b`.
- Lark/Jade seven-case adversarial set: commit
  `10870f77421975cba7c2de8291ce652ae45a0182`.
- Exact-signature verifier correction is included transitively through commit
  `ba07107569690354cb2d5d5ea23fb3e55f710ce4`.

All four transcript PSBTs and seven negative PSBTs are checked by decoded byte
length and SHA-256. Local text encodings are separately pinned in
`manifest.json`.

## Results reproduced in Kern

| Check | Result | Scope |
| --- | --- | --- |
| Shooter crypto corpus | PASS: 18 vectors and four tweak boundaries | S2C relation only; Lark does not publish pubkey/message-hash tuples for these cases |
| Shooter recorded transcript | PASS | Stage discipline, commitment/opening stability, entropy opening, low-S, slot identity, and S2C relation |
| Shooter adversarial artifacts | PASS: seven decoded artifacts match their declared hashes and lengths | Artifact integrity; Lark reports two cases not yet rejected by its implementation |
| Kern complete crypto corpus through Shooter's independent checker | PASS: seven complete tuples | Five honest tuples, one exact-signature cross-key negative, one mismatched-opening negative; ECDSA and S2C both checked |
| Kern multisig artifact | PASS | Canonical PSBT-v0 input 2 is 2-of-3 P2WSH and contains two protected partial signatures |

Run the complete checkpoint with:

```text
python3 main/core/test/check_anti_exfil_collaboration.py
```

The fixture freshness gate runs this command before Kern's anti-exfil C tests.

## What is not yet a cross-implementation pass

Lark's semantic mapping does not carry the locally computed BIP143 message hash,
PSBT-v0 frozen-byte digest, exact network identity, or a transport-neutral
session identifier required by Kern's internal record. It therefore cannot be
fed directly to Kern's M2 semantic validator without inventing fields.

The imported in-PSBT negatives are coordinator-response cases. Kern's detached
AEXT signer does not consume a returned PSBT, so those artifacts are pinned as
profile evidence rather than mislabeled as Kern signer inputs. Equivalent
detached semantic mutations remain covered by Kern's existing C suite.

The next cross-project step is for Lark/Jade to run
`complete_ecdsa/kern-ae-ecdsa-complete-v1.json` and the generated two-signature
multisig PSBT, then record its actual accept/reject results in the joint matrix.
