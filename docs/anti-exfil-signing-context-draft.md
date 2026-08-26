# Transport-neutral signing-context digest draft

Status: concrete proposal for joint review; not normative and not wired into
signing policy.

The shared binding should not depend on how a PSBT library orders or
re-serializes maps. This draft therefore defines a projection of the signing
context and hashes that projection with BIP340-style tagged SHA-256 using the
tag `anti-exfil/signing-context/v1`.

## Canonical preimage

All integers are unsigned big-endian. Every variable byte string is encoded as
`length:u32 || bytes`.

```text
"AESC"
version:u8 = 1
network:u8
flags:u16 = 0
unsigned_transaction:bytes
input_count:u32
for each input in transaction order:
    input_index:u32
    amount:u64
    script_pubkey:bytes
    redeem_script:bytes
    witness_script:bytes
    sighash_type:u32
slot_count:u16
for each requested slot ordered by (input_index, compressed_pubkey):
    input_index:u32
    compressed_pubkey:33 bytes
    sighash_type:u32
```

The digest is:

```text
tag_hash = SHA256("anti-exfil/signing-context/v1")
digest = SHA256(tag_hash || tag_hash || canonical_preimage)
```

The projection includes the exact unsigned transaction, every input amount and
spend script, explicit network identity, and the complete requested slot set.
It excludes anti-exfil records and unrelated proprietary fields.

BIP32 paths are deliberately excluded from the proposed shared digest. A signer
must still derive its own key from the supplied path and prove that it matches
the slot pubkey before signing. The path is local authorization evidence; the
transaction input and signer pubkey are the cross-implementation signing
identity. This avoids requiring a signer to understand other signers' path
metadata while retaining Kern's stricter local derivation check.

The exact generated preimage and digest for Kern's five-slot, four-input fixture
are in
`main/core/test/fixtures/anti_exfil/signing_context/anti-exfil-signing-context-draft-v1.json`.
Regeneration is deterministic and gated by
`generate_anti_exfil_collaboration_fixtures.py --check`.

## Deliberately unresolved

- Whether the shared spec adopts this domain tag and integer encoding.
- Whether a session may cover a subset of otherwise eligible inputs.
- Whether profiles also require a normalized base-PSBT check as defense in
  depth.
- How a future PSBT-v2 profile projects fields that are currently carried in
  the PSBT-v0 unsigned transaction.

No firmware signer or scanner decision depends on this draft yet.
