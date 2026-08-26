#!/usr/bin/env python3
"""Generate collaboration corpora from Kern's pinned five-slot transcript.

The generated files are review artifacts, not a frozen protocol. They publish
complete ECDSA/S2C tuples, a signed multisig PSBT with two signatures on one
input, and a concrete transport-neutral signing-context proposal.
"""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
from pathlib import Path


HERE = Path(__file__).resolve().parent
FIXTURES = HERE / "fixtures" / "anti_exfil"
SOURCE = FIXTURES / "semantic_protocol" / "protocol-v1-semantic-psbt-vector.json"
COMPLETE_OUTPUT = FIXTURES / "complete_ecdsa" / "kern-ae-ecdsa-complete-v1.json"
MULTISIG_OUTPUT = FIXTURES / "profile_interop" / "kern-multisig-two-signatures-per-input-v1.json"
CONTEXT_OUTPUT = FIXTURES / "signing_context" / "anti-exfil-signing-context-draft-v1.json"
MEASUREMENT_HEADER = HERE / "anti_exfil_measurement_vectors.generated.h"

SOURCE_SHA256 = "f28d572d1ae5d2060eeb52ca9814f37ce5d54258811d3af18b78c41744e23a4e"
TAG = b"anti-exfil/signing-context/v1"
RECORD_LENGTHS = {1: 105, 2: 138, 3: 170, 4: 202}
NETWORKS = {"mainnet": 0, "testnet3": 1, "regtest": 2, "signet": 3, "testnet4": 4}


def read_varint(raw: bytes, offset: int) -> tuple[int, int]:
    if offset >= len(raw):
        raise ValueError("truncated compact size")
    prefix = raw[offset]
    offset += 1
    if prefix < 0xFD:
        return prefix, offset
    widths = {0xFD: 2, 0xFE: 4, 0xFF: 8}
    width = widths[prefix]
    if offset + width > len(raw):
        raise ValueError("truncated compact size payload")
    value = int.from_bytes(raw[offset:offset + width], "little")
    minimum = {2: 0xFD, 4: 0x10000, 8: 0x100000000}[width]
    if value < minimum:
        raise ValueError("non-minimal compact size")
    return value, offset + width


def write_varint(value: int) -> bytes:
    if value < 0xFD:
        return bytes([value])
    if value <= 0xFFFF:
        return b"\xfd" + value.to_bytes(2, "little")
    if value <= 0xFFFFFFFF:
        return b"\xfe" + value.to_bytes(4, "little")
    return b"\xff" + value.to_bytes(8, "little")


def read_map(raw: bytes, offset: int) -> tuple[list[tuple[bytes, bytes]], int]:
    entries: list[tuple[bytes, bytes]] = []
    seen: set[bytes] = set()
    while True:
        key_len, offset = read_varint(raw, offset)
        if key_len == 0:
            return entries, offset
        if offset + key_len > len(raw):
            raise ValueError("truncated PSBT key")
        key = raw[offset:offset + key_len]
        offset += key_len
        value_len, offset = read_varint(raw, offset)
        if offset + value_len > len(raw):
            raise ValueError("truncated PSBT value")
        value = raw[offset:offset + value_len]
        offset += value_len
        if key in seen:
            raise ValueError("duplicate PSBT key")
        seen.add(key)
        entries.append((key, value))


def write_map(entries: list[tuple[bytes, bytes]]) -> bytes:
    out = bytearray()
    for key, value in sorted(entries, key=lambda item: item[0]):
        out += write_varint(len(key)) + key + write_varint(len(value)) + value
    return bytes(out) + b"\x00"


def tx_counts(unsigned_tx: bytes) -> tuple[int, int]:
    if len(unsigned_tx) < 10:
        raise ValueError("truncated unsigned transaction")
    offset = 4
    input_count, offset = read_varint(unsigned_tx, offset)
    for _ in range(input_count):
        if offset + 36 > len(unsigned_tx):
            raise ValueError("truncated transaction input")
        offset += 36
        script_len, offset = read_varint(unsigned_tx, offset)
        offset += script_len + 4
        if offset > len(unsigned_tx):
            raise ValueError("truncated transaction input payload")
    output_count, offset = read_varint(unsigned_tx, offset)
    for _ in range(output_count):
        if offset + 8 > len(unsigned_tx):
            raise ValueError("truncated transaction output")
        offset += 8
        script_len, offset = read_varint(unsigned_tx, offset)
        offset += script_len
        if offset > len(unsigned_tx):
            raise ValueError("truncated transaction output payload")
    if offset + 4 != len(unsigned_tx):
        raise ValueError("unsigned transaction has trailing data")
    return input_count, output_count


def parse_psbt(raw: bytes) -> tuple[
    list[tuple[bytes, bytes]],
    list[list[tuple[bytes, bytes]]],
    list[list[tuple[bytes, bytes]]],
]:
    if not raw.startswith(b"psbt\xff"):
        raise ValueError("invalid PSBT magic")
    globals_map, offset = read_map(raw, 5)
    unsigned = [value for key, value in globals_map if key == b"\x00"]
    if len(unsigned) != 1:
        raise ValueError("PSBT v0 requires one unsigned transaction")
    input_count, output_count = tx_counts(unsigned[0])
    inputs = []
    outputs = []
    for _ in range(input_count):
        scope, offset = read_map(raw, offset)
        inputs.append(scope)
    for _ in range(output_count):
        scope, offset = read_map(raw, offset)
        outputs.append(scope)
    if offset != len(raw):
        raise ValueError("PSBT has trailing bytes")
    return globals_map, inputs, outputs


def serialize_psbt(globals_map, inputs, outputs) -> bytes:
    return b"psbt\xff" + write_map(globals_map) + b"".join(write_map(item) for item in inputs + outputs)


def parse_aexb(hex_text: str) -> dict:
    raw = bytes.fromhex(hex_text)
    if len(raw) < 78 or raw[:4] != b"AEXB":
        raise ValueError("invalid AEXB message")
    stage = raw[6]
    record_len = RECORD_LENGTHS.get(stage)
    slot_count = int.from_bytes(raw[76:78], "big")
    payload_len = int.from_bytes(raw[8:12], "big")
    if not record_len or payload_len != record_len * slot_count or len(raw) != 78 + payload_len:
        raise ValueError("invalid AEXB record lengths")
    slots = []
    for index in range(slot_count):
        record = raw[78 + index * record_len:78 + (index + 1) * record_len]
        slots.append({
            "input_index": int.from_bytes(record[0:4], "big"),
            "sighash_type": int.from_bytes(record[4:8], "big"),
            "signer_pubkey": record[8:41],
            "message_hash": record[41:73],
            "host_commitment": record[73:105],
            "opening": record[105:138] if stage >= 2 else b"",
            "host_entropy": record[138:170] if stage == 3 else b"",
            "signature": record[138:202] if stage == 4 else b"",
        })
    return {"stage": stage, "network": raw[5], "slots": slots}


def compact_to_der(compact: bytes) -> bytes:
    if len(compact) != 64:
        raise ValueError("compact signature must be 64 bytes")
    encoded = []
    for scalar in (compact[:32], compact[32:]):
        scalar = scalar.lstrip(b"\x00") or b"\x00"
        if scalar[0] & 0x80:
            scalar = b"\x00" + scalar
        encoded.append(b"\x02" + bytes([len(scalar)]) + scalar)
    body = b"".join(encoded)
    return b"\x30" + bytes([len(body)]) + body


def tagged_hash(tag: bytes, message: bytes) -> bytes:
    digest = hashlib.sha256(tag).digest()
    return hashlib.sha256(digest + digest + message).digest()


def field(entries: list[tuple[bytes, bytes]], key_type: int) -> bytes:
    values = [value for key, value in entries if key == bytes([key_type])]
    if len(values) > 1:
        raise ValueError(f"duplicate singleton PSBT field {key_type:#x}")
    return values[0] if values else b""


def txout(value: bytes) -> tuple[int, bytes]:
    if len(value) < 9:
        raise ValueError("truncated witness UTXO")
    amount = int.from_bytes(value[:8], "little")
    script_len, offset = read_varint(value, 8)
    if offset + script_len != len(value):
        raise ValueError("invalid witness UTXO length")
    return amount, value[offset:]


def u32(value: int) -> bytes:
    return value.to_bytes(4, "big")


def lp(value: bytes) -> bytes:
    return u32(len(value)) + value


def load_source() -> dict:
    raw = SOURCE.read_bytes()
    actual = hashlib.sha256(raw).hexdigest()
    if actual != SOURCE_SHA256:
        raise ValueError(f"source fixture hash changed: {actual}")
    return json.loads(raw)


def complete_corpus(source: dict) -> dict:
    reveal = parse_aexb(source["message_3_hex"])
    signatures = parse_aexb(source["message_4_hex"])
    if [(s["input_index"], s["signer_pubkey"]) for s in reveal["slots"]] != [
        (s["input_index"], s["signer_pubkey"]) for s in signatures["slots"]
    ]:
        raise ValueError("M3/M4 slot identities differ")
    vectors = []
    for index, (before, after) in enumerate(zip(reveal["slots"], signatures["slots"], strict=True)):
        vectors.append({
            "id": f"kern-multiscript-{index}",
            "class": "honest_signer",
            "der_sig": compact_to_der(after["signature"]).hex(),
            "signature_compact": after["signature"].hex(),
            "signer_commitment": after["opening"].hex(),
            "host_entropy": before["host_entropy"].hex(),
            "host_commitment": after["host_commitment"].hex(),
            "pubkey": after["signer_pubkey"].hex(),
            "message_hash": after["message_hash"].hex(),
            "expected_s2c": True,
            "expected_ecdsa": True,
            "expected_combined": True,
            "reason": "exact Kern/libwally signature from the five-slot PSBT-v0 transcript",
        })
    mismatched = dict(vectors[0])
    mismatched.update({
        "id": "kern-exact-signature-cross-key",
        "class": "cross_key_signature",
        "pubkey": vectors[1]["pubkey"],
        "expected_ecdsa": False,
        "expected_combined": False,
        "reason": "the S2C opening is valid but the exact signature is checked against another slot's key",
    })
    vectors.append(mismatched)
    mismatched_pairing = dict(vectors[0])
    mismatched_pairing.update({
        "id": "kern-opening-from-another-slot",
        "class": "mismatched_pairing",
        "signer_commitment": vectors[1]["signer_commitment"],
        "expected_s2c": False,
        "expected_ecdsa": True,
        "expected_combined": False,
        "reason": "the exact ECDSA signature is valid but the signer opening belongs to another slot",
    })
    vectors.append(mismatched_pairing)
    return {
        "schema": "ae-ecdsa-crypto-v1",
        "source": "Kern five-slot PSBT-v0 M3/M4 fixture; generated from pinned AEXB records",
        "source_sha256": SOURCE_SHA256,
        "curve": "secp256k1",
        "construction": {
            "host_commitment": "tagged_hash(\"s2c/ecdsa/data\", host_entropy)",
            "tweak": "tagged_hash(\"s2c/ecdsa/point\", signer_commitment || host_entropy)",
            "accept_if": "sig.r == (signer_commitment + tweak*G).x mod n",
            "tweak_out_of_range": "rejected, not reduced mod n",
        },
        "encoding_rules": {
            "signature": "strict DER; signature_compact carries the identical r,s pair",
            "low_s_required": True,
            "scalars_in_range": "0 < r,s < n",
        },
        "notes": [
            "Five accepting tuples carry pubkey and message_hash, so both ECDSA and S2C are independently checkable.",
            "The cross-key case pins exact-signature verification rather than accepting any valid signature beside a valid opening.",
        ],
        "tweak_boundary": [
            {"id": "tweak-in-range", "tweak32": "00" * 31 + "01", "acceptable": True},
            {"id": "tweak-zero", "tweak32": "00" * 32, "acceptable": False},
            {"id": "tweak-at-order", "tweak32": "fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141", "acceptable": False},
            {"id": "tweak-max", "tweak32": "ff" * 32, "acceptable": False},
        ],
        "vectors": vectors,
    }


def signed_multisig_fixture(source: dict) -> dict:
    original = bytes.fromhex(source["psbt_hex"])
    globals_map, inputs, outputs = parse_psbt(original)
    signatures = parse_aexb(source["message_4_hex"])["slots"]
    for slot in signatures:
        key = b"\x02" + slot["signer_pubkey"]
        if any(existing == key for existing, _ in inputs[slot["input_index"]]):
            raise ValueError("fixture already contains a selected-key signature")
        inputs[slot["input_index"]].append((key, compact_to_der(slot["signature"]) + b"\x01"))
    signed = serialize_psbt(globals_map, inputs, outputs)
    actual = hashlib.sha256(signed).hexdigest()
    if actual != source["signed_psbt_sha256"]:
        raise ValueError(f"signed PSBT reconstruction changed: {actual}")
    counts = [sum(1 for key, _ in item if key[:1] == b"\x02") for item in inputs]
    if counts[2] != 2:
        raise ValueError("multisig input does not carry two partial signatures")
    return {
        "schema": "ae-multisig-interoperability-v1",
        "source_sha256": SOURCE_SHA256,
        "network": source["network"],
        "description": "Real ECDSA signatures on a canonical 2-of-3 P2WSH input; input 2 carries two independently verifiable protected signatures.",
        "original_psbt_base64": base64.b64encode(original).decode(),
        "original_psbt_sha256": hashlib.sha256(original).hexdigest(),
        "signed_psbt_base64": base64.b64encode(signed).decode(),
        "signed_psbt_sha256": actual,
        "partial_signature_count_by_input": counts,
        "multisig_input_index": 2,
        "multisig_threshold": 2,
        "multisig_key_count": 3,
        "protected_signer_pubkeys": [slot["signer_pubkey"].hex() for slot in signatures if slot["input_index"] == 2],
        "message_3_hex": source["message_3_hex"],
        "message_4_hex": source["message_4_hex"],
    }


def signing_context_fixture(source: dict) -> dict:
    raw = bytes.fromhex(source["psbt_hex"])
    globals_map, inputs, _ = parse_psbt(raw)
    unsigned_tx = next(value for key, value in globals_map if key == b"\x00")
    message = parse_aexb(source["message_1_hex"])
    preimage = bytearray(b"AESC")
    preimage += b"\x01" + bytes([NETWORKS[source["network"]]]) + b"\x00\x00"
    preimage += lp(unsigned_tx) + u32(len(inputs))
    input_records = []
    for index, scope in enumerate(inputs):
        witness_utxo = field(scope, 0x01)
        if not witness_utxo:
            raise ValueError("draft vector currently requires witness UTXOs")
        amount, script_pubkey = txout(witness_utxo)
        redeem_script = field(scope, 0x04)
        witness_script = field(scope, 0x05)
        sighash_raw = field(scope, 0x03)
        sighash = int.from_bytes(sighash_raw, "little") if sighash_raw else 1
        preimage += u32(index) + amount.to_bytes(8, "big")
        preimage += lp(script_pubkey) + lp(redeem_script) + lp(witness_script) + u32(sighash)
        input_records.append({
            "input_index": index,
            "amount": amount,
            "script_pubkey": script_pubkey.hex(),
            "redeem_script": redeem_script.hex(),
            "witness_script": witness_script.hex(),
            "sighash_type": sighash,
        })
    preimage += len(message["slots"]).to_bytes(2, "big")
    slot_records = []
    for slot in message["slots"]:
        preimage += u32(slot["input_index"]) + slot["signer_pubkey"] + u32(slot["sighash_type"])
        slot_records.append({
            "input_index": slot["input_index"],
            "signer_pubkey": slot["signer_pubkey"].hex(),
            "sighash_type": slot["sighash_type"],
        })
    digest = tagged_hash(TAG, bytes(preimage))
    return {
        "schema": "ae-signing-context-draft-v1",
        "status": "proposal-not-normative",
        "source_sha256": SOURCE_SHA256,
        "domain_tag": TAG.decode(),
        "digest_algorithm": "BIP340-style tagged SHA-256",
        "digest": digest.hex(),
        "preimage_hex": bytes(preimage).hex(),
        "encoding": {
            "integers": "unsigned big-endian",
            "byte_strings": "u32 length followed by exact bytes",
            "header": "AESC || version:u8 || network:u8 || flags:u16",
            "body": "unsigned_tx || all input spend contexts || ordered requested slots",
            "slot_order": "input_index ascending, then compressed signer pubkey lexicographic",
        },
        "policy_decisions_for_review": [
            "Includes the exact unsigned transaction and every input's amount and scripts.",
            "Includes network identity and each requested slot's input, pubkey, and sighash.",
            "Excludes BIP32 derivation paths: devices still verify their own path-to-pubkey mapping locally, but path metadata is not part of signing identity.",
            "Excludes anti-exfil and unknown proprietary fields; response allowlisting is a separate invariant.",
        ],
        "unsigned_tx": unsigned_tx.hex(),
        "inputs": input_records,
        "slots": slot_records,
    }


def rendered(value: dict) -> str:
    return json.dumps(value, indent=2, sort_keys=True) + "\n"


def cbor_bytes(value: bytes) -> bytes:
    length = len(value)
    if length < 24:
        return bytes([0x40 | length]) + value
    if length <= 0xFF:
        return b"\x58" + bytes([length]) + value
    if length <= 0xFFFF:
        return b"\x59" + length.to_bytes(2, "big") + value
    return b"\x5a" + length.to_bytes(4, "big") + value


def c_bytes(value: bytes) -> str:
    return ", ".join(f"0x{item:02x}" for item in value)


def measurement_header(source: dict) -> str:
    lines = [
        "/* Generated by generate_anti_exfil_collaboration_fixtures.py. */",
        "#ifndef KERN_ANTI_EXFIL_MEASUREMENT_VECTORS_GENERATED_H",
        "#define KERN_ANTI_EXFIL_MEASUREMENT_VECTORS_GENERATED_H",
        "",
        "#include <stddef.h>",
        "#include <stdint.h>",
        "",
        "typedef struct { uint8_t stage; const uint8_t *cbor; size_t cbor_len; } anti_exfil_measurement_fixture_t;",
        "",
    ]
    fixtures = []
    for item in source["aext_packages"]:
        stage = item["stage"]
        cbor = cbor_bytes(bytes.fromhex(item["package_hex"]))
        lines.append(f"static const uint8_t ANTI_EXFIL_MEASUREMENT_CBOR_{stage}[] = {{{c_bytes(cbor)}}};")
        fixtures.append((stage, len(cbor)))
    lines.extend(["", "static const anti_exfil_measurement_fixture_t ANTI_EXFIL_MEASUREMENT_FIXTURES[] = {"])
    for stage, _ in fixtures:
        lines.append(f"  {{{stage}, ANTI_EXFIL_MEASUREMENT_CBOR_{stage}, sizeof(ANTI_EXFIL_MEASUREMENT_CBOR_{stage})}},")
    lines.extend(["};", "", "#endif", ""])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    source = load_source()
    outputs = {
        COMPLETE_OUTPUT: rendered(complete_corpus(source)),
        MULTISIG_OUTPUT: rendered(signed_multisig_fixture(source)),
        CONTEXT_OUTPUT: rendered(signing_context_fixture(source)),
        MEASUREMENT_HEADER: measurement_header(source),
    }
    if args.check:
        stale = [str(path) for path, text in outputs.items() if not path.exists() or path.read_text(encoding="utf-8") != text]
        if stale:
            raise SystemExit("stale collaboration fixture(s): " + ", ".join(stale))
        print("anti-exfil collaboration fixtures are fresh")
        return 0
    for path, text in outputs.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8", newline="\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
