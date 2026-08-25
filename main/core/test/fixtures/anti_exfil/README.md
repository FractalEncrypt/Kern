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

The shared anti-exfil reference repository is the source of record. Imported
semantic JSON is pinned by SHA-256 in `generate_anti_exfil_fixtures.py`. The
byte-exact transport fixture is separately pinned to SHA-256
`bafd399a342e1be965666d4efca970b50218a2fb2e2820c418ad64686bac1bb3`.
Regeneration must be deterministic, and `--check` must produce no diff.
