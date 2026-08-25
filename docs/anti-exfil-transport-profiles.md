# Experimental anti-exfil transport profiles

Status: comparison draft for collaboration; no carriage is declared normative.

Kern's anti-exfil signer consumes one bounded, transport-neutral four-stage
record. A transport adapter may supply that record only after canonical decoding;
it does not own signing policy, slot discovery, transcript validation, or PSBT
merging. All profiles must preserve the exact stage, network, session, frozen-PSBT
digest, ordered slot set, commitments, openings, reveals, and compact signatures.

## Candidate comparison

| Property | Detached AEXT/AEXB | Proprietary in-PSBT fields | Hybrid: PSBT requests, detached responses |
| --- | --- | --- | --- |
| Current evidence | Byte-exact Sparrow/SeedSigner corpus and Kern codec | BitcoinShooter Jade/Sparrow experiment; equivalent shared corpus still needed | Architectural candidate; wire fields and corpus not yet concrete |
| Requests (messages 1 and 3) | Canonical AEXB plus the exact frozen PSBT inside AEXT | Stage fields carried as PSBT proprietary data | PSBT request carries stage fields |
| Responses (messages 2 and 4) | Detached AEXB; PSBT forbidden | Returned PSBT carries openings/signatures | Detached bounded openings/signatures; PSBT forbidden |
| Frozen-byte rule | Explicit: message 3 repeats byte-identical PSBT bytes | Must define whether the authoritative identity is original bytes or canonical PSBT semantics | Must bind the request PSBT without allowing reserialization between rounds |
| Coordinator reconstruction | Imports verified exact-slot signatures into its frozen original | Must ignore all signer-returned PSBT changes except verified expected fields/signatures | Imports verified detached signatures into its frozen original |
| Generic-combine risk | Low when `x-btc-anti-exfil` is routed before generic PSBT/text handling | Higher unless proprietary fields bypass ordinary parse/combine/sign paths | Request parser risk remains; response merge risk is low |
| QR overhead | Explicit envelope overhead; message 3 repeats the PSBT | Reuses familiar `crypto-psbt` carriage but proprietary fields increase it | Potentially lower response size while retaining PSBT tooling for requests |
| Extensibility/adoption | New experimental UR type and codec | Fits existing PSBT-centric device architecture; standard fields require coordination | More adapter complexity and two distinct carriage rules |

## Kern reference-profile decisions

- `ur:x-btc-anti-exfil` remains an experimental interoperability type, not a
  universal or registered standard.
- Only canonical CBOR containing one definite-length AEXT byte string is accepted.
- AEXT messages 1 and 3 contain the exact frozen PSBT. Messages 2 and 4 contain
  no PSBT.
- The AEXT adapter passes the original decoded PSBT byte view to the signer. It
  must never parse and reserialize those bytes before the M3 digest check.
- Wrong UR type, active network, expected stage, outer/inner identity, PSBT
  presence, length, digest, or canonical encoding fails without a fallback into
  ordinary PSBT or text handling.
- Kern's cUR decoder currently caps a reconstructed CBOR message at 256 KiB.
  The adapter exposes this as `AE_SIZE_LIMIT`; this is a provisional implementation
  ceiling below AEXT's absolute 2,000,000-byte PSBT limit. The final operational
  limit and fragment/frame settings require `wave_7b` heap, timing, and camera
  measurements before M5 is complete.

## Items to settle jointly before freezing a profile

1. Publish equivalent positive and adversarial vectors for the in-PSBT and hybrid
   candidates, mapped to the same semantic records and stable reason codes.
2. Define byte identity versus semantic PSBT identity for PSBT-carried rounds,
   including unknown proprietary fields and canonical reserialization.
3. Define which returned PSBT fields a coordinator may consume; the safe default
   is none beyond independently verified expected anti-exfil data.
4. Compare QR frame counts, scan time, peak heap, retry behavior, and implementation
   complexity on SeedSigner, Jade, and Kern.
5. Coordinate any public PSBT field, CBOR tag, or registered UR type only after the
   cross-implementation matrix passes.
