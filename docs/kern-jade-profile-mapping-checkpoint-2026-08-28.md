# Kern/Jade profile-mapping checkpoint

Date: 2026-08-28

This checkpoint pins Shooter's reviewed Jade in-PSBT profile and Sparrow
hardware evidence, then projects the same recorded logical ceremony into
Kern's canonical AEXB/AEXT carriage. It is a mapping-conformance result, not a
claim that Jade emitted detached transport bytes.

## Upstream identities

- Drongo commit `dc5d836`, annotated tag
  `jade-in-psbt-profile-2026-08-27`.
- Sparrow commit `cb2dabb`, annotated tag
  `jade-in-psbt-hardware-2026-08-27`.

`PROVENANCE.json` pins every upstream Git blob. The four Drongo PSBT resources
are byte-identical to the earlier Lark transcript already vendored under
`profile_interop/lark/transcript`, so this checkpoint references those bytes
instead of duplicating them. All seven new Sparrow JSON objects are stored
byte-for-byte and pinned by SHA-256.

## Executable neutral mapping

`jade-neutral-profile-mapping-v1.json` contains, for each of the four stages:

- the exact recorded in-PSBT artifact identity;
- a canonical logical AEXB record including the independently computed BIP143
  message hash;
- the canonical AEXT projection of that same logical record;
- an explicit split between coordinator-authoritative fields and the one
  signer-returned field permitted at stages 2 and 4.

The generator independently parses the PSBT maps and rejects malformed or
duplicate entries. It requires the exact reserved-namespace record set at each
stage, extracts the selected-key DER signature at stage 4, converts it to the
same compact signature carried by AEXB, and reconstructs the frozen PSBT by
removing only the known `ae` records. Unknown `ae` subtypes or records for a
different slot cannot enter the generated projection.

The AEXT projection follows Kern's detached profile: stages 1 and 3 contain the
frozen PSBT; stages 2 and 4 contain no PSBT. Thus both carriage encodings map to
one pinned logical transcript while preserving the different trust boundaries.

## Hardware and durability evidence

The imported Sparrow set contains six durable sessions:

- one complete four-stage Testnet4 Jade ceremony with a pinned signed-PSBT
  digest;
- five sessions durably stopped at `OPENINGS_ACCEPTED` after stage 3;
- one abort journal with three `SIGNATURE_REJECTED` post-reveal events.

The checker validates exact artifact hashes, schema/profile identity, retained
stage sequence, AEXB byte layout, stable session/digest/slot identity, and the
complete-session signed-PSBT pin. The existing independent Python verifier
continues to validate the recorded Jade signature's S2C relation and the Kern
complete tuples.

Run the checkpoint with:

```text
python3 main/core/test/check_anti_exfil_collaboration.py
```

## Honest boundary

This proves that the reviewed in-PSBT mapping and the detached profile can be
represented as the same logical four-stage protocol without dropping the
message hash or weakening field authority. It does not claim a second detached
hardware recording of this Jade ceremony, nor a mixed-device transaction.

With this checkpoint green, the next work item is M7: persist explicit signer
identity and anti-exfil profile/capability in Sparrow so profile selection is
never inferred merely from a broad device model.
