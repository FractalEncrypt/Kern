# Anti-exfil conformance corpora

The fixture layers are intentionally separate so transport bytes and physical
evidence cannot become semantic protocol inputs:

- `crypto/`: primitive signer-opening vectors;
- `complete_ecdsa/`: complete protected ECDSA tuples;
- `semantic_protocol/`: transport-neutral four-stage records and rejection
  expectations used to generate C fixtures;
- `transport/`: byte-exact AEXB/AEXT/CBOR/UR fixtures, not consumed by semantic
  tests;
- `physical_evidence/`: manifests only; device images and logs are not firmware
  test inputs.
- `profile_interop/`: pinned carriage-specific artifacts and cross-project
  evidence. These files never become transport-neutral semantic test input.
- `signing_context/`: non-normative candidate preimages and digests for joint
  specification review.

The Lark/Jade profile corpus is pinned to transcript commit
`8f53be92ca7a49659d852ca58853740ce9a1625b` and adversarial commit
`10870f77421975cba7c2de8291ce652ae45a0182`. The imported base64 files are
checked by their decoded PSBT length and SHA-256; text-file hashes in the
manifest additionally detect local drift.

`complete_ecdsa/kern-ae-ecdsa-complete-v1.json` publishes Kern's five honest
full tuples plus exact-signature and mismatched-opening negatives in the schema
accepted by Shooter's independent checker. The generated multisig artifact has
two protected partial signatures on input 2 of a real 2-of-3 P2WSH PSBT.

The shared anti-exfil reference repository is the source of record. Imported
semantic JSON is pinned by SHA-256 in `generate_anti_exfil_fixtures.py`. The
byte-exact transport fixture is separately pinned to SHA-256
`bafd399a342e1be965666d4efca970b50218a2fb2e2820c418ad64686bac1bb3`.
Regeneration must be deterministic, and `--check` must produce no diff.
