#!/usr/bin/env python3
"""Generate a neutral logical/AEXT/in-PSBT mapping from pinned Jade bytes."""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
from pathlib import Path


HERE = Path(__file__).resolve().parent
ROOT = HERE / "fixtures" / "anti_exfil" / "profile_interop"
LARK = ROOT / "lark"
JADE = ROOT / "jade"
OUTPUT = JADE / "jade-neutral-profile-mapping-v1.json"

NETWORK = 4  # testnet4
SESSION_ID = bytes.fromhex("5a" * 32)
PUBKEY = bytes.fromhex("02915fd6388a83fd35f41ef8ec5004b372cb616683d3ac114d48131a62549c8d1a")
MESSAGE_HASH = bytes.fromhex("2951a65a749283810743e1cd038d201c00d3ed17895ac2c0acaac898fa319c3f")
COMMITMENT = bytes.fromhex("c6d65d57ca32496e0cc6b1cac7a9c4e3ee4fb70c93e34141e996138b4556431a")
OPENING = bytes.fromhex("02edd2346fcdb2882c866f081452b16b4528e7281a6667a7a16fd976c3c7579874")
RHO = bytes.fromhex("40d01a3c67c6f762519959eaf013f871b5d024caf70b68cc6552d0a13939493a")
SIGNATURE = bytes.fromhex(
    "8ed9294bd47bb752cc5c12b04c339a52147d15ace176f6d578bbb7db29c19472"
    "7778889569feab0a2206c45d2603174e01ade535ecbe72ef3b0b555eb163b257"
)
STAGES = (
    (1, "M1-coordinator-to-signer.psbt.b64"),
    (2, "M2-signer-to-coordinator.psbt.b64"),
    (3, "M3-coordinator-to-signer.psbt.b64"),
    (4, "M4-signer-to-coordinator.psbt.b64"),
)


def compact(raw: bytes, offset: int) -> tuple[int, int]:
    prefix = raw[offset]
    offset += 1
    if prefix < 0xfd:
        return prefix, offset
    width = {0xfd: 2, 0xfe: 4, 0xff: 8}[prefix]
    value = int.from_bytes(raw[offset:offset + width], "little")
    if value < {2: 0xfd, 4: 0x10000, 8: 0x100000000}[width]:
        raise ValueError("non-minimal compact size")
    return value, offset + width


def put_compact(value: int) -> bytes:
    if value < 0xfd:
        return bytes([value])
    if value <= 0xffff:
        return b"\xfd" + value.to_bytes(2, "little")
    if value <= 0xffffffff:
        return b"\xfe" + value.to_bytes(4, "little")
    return b"\xff" + value.to_bytes(8, "little")


def read_map(raw: bytes, offset: int) -> tuple[list[tuple[bytes, bytes]], int]:
    entries = []
    seen = set()
    while True:
        key_len, offset = compact(raw, offset)
        if key_len == 0:
            return entries, offset
        key = raw[offset:offset + key_len]
        offset += key_len
        value_len, offset = compact(raw, offset)
        value = raw[offset:offset + value_len]
        offset += value_len
        if len(key) != key_len or len(value) != value_len or key in seen:
            raise ValueError("malformed or duplicate PSBT map entry")
        seen.add(key)
        entries.append((key, value))


def write_map(entries: list[tuple[bytes, bytes]]) -> bytes:
    return b"".join(
        put_compact(len(key)) + key + put_compact(len(value)) + value
        for key, value in entries
    ) + b"\x00"


def tx_counts(tx: bytes) -> tuple[int, int]:
    offset = 4
    inputs, offset = compact(tx, offset)
    for _ in range(inputs):
        offset += 36
        length, offset = compact(tx, offset)
        offset += length + 4
    outputs, offset = compact(tx, offset)
    for _ in range(outputs):
        offset += 8
        length, offset = compact(tx, offset)
        offset += length
    if offset + 4 != len(tx):
        raise ValueError("malformed unsigned transaction")
    return inputs, outputs


def parse_psbt(raw: bytes) -> tuple[list[tuple[bytes, bytes]], list[list[tuple[bytes, bytes]]], list[list[tuple[bytes, bytes]]]]:
    if not raw.startswith(b"psbt\xff"):
        raise ValueError("invalid PSBT magic")
    global_map, offset = read_map(raw, 5)
    unsigned = [value for key, value in global_map if key == b"\x00"]
    if len(unsigned) != 1:
        raise ValueError("expected one PSBT-v0 unsigned transaction")
    input_count, output_count = tx_counts(unsigned[0])
    inputs = []
    outputs = []
    for _ in range(input_count):
        value, offset = read_map(raw, offset)
        inputs.append(value)
    for _ in range(output_count):
        value, offset = read_map(raw, offset)
        outputs.append(value)
    if offset != len(raw):
        raise ValueError("trailing PSBT bytes")
    return global_map, inputs, outputs


def serialize_psbt(global_map, inputs, outputs) -> bytes:
    return b"psbt\xff" + write_map(global_map) + b"".join(write_map(value) for value in inputs + outputs)


def ae_key(subtype: int) -> bytes:
    return b"\xfc\x02ae" + bytes([subtype]) + PUBKEY


def der_to_compact(value: bytes) -> bytes:
    if value[-1] != 1 or value[0] != 0x30 or value[1] != len(value) - 3:
        raise ValueError("unexpected partial signature encoding")
    offset = 2
    scalars = []
    for _ in range(2):
        if value[offset] != 2:
            raise ValueError("unexpected DER scalar")
        length = value[offset + 1]
        scalar = value[offset + 2:offset + 2 + length].lstrip(b"\x00")
        if not scalar or len(scalar) > 32:
            raise ValueError("invalid DER scalar length")
        scalars.append(scalar.rjust(32, b"\x00"))
        offset += 2 + length
    if offset != len(value) - 1:
        raise ValueError("trailing DER data")
    return b"".join(scalars)


def aexb(stage: int, digest: bytes) -> bytes:
    record = (0).to_bytes(4, "big") + (1).to_bytes(4, "big") + PUBKEY + MESSAGE_HASH + COMMITMENT
    if stage >= 2:
        record += OPENING
    if stage == 3:
        record += RHO
    elif stage == 4:
        record += SIGNATURE
    return (b"AEXB" + bytes([1, NETWORK, stage, 0]) + len(record).to_bytes(4, "big") +
            SESSION_ID + digest + (1).to_bytes(2, "big") + record)


def aext(message: bytes, frozen: bytes | None) -> bytes:
    stage = message[6]
    psbt = frozen or b""
    outer_digest = hashlib.sha256(psbt).digest() if psbt else bytes(32)
    return (b"AEXT" + bytes([1, NETWORK, stage, 1 if psbt else 0]) +
            len(message).to_bytes(4, "big") + len(psbt).to_bytes(4, "big") +
            outer_digest + message + psbt)


def build() -> dict:
    artifacts = []
    parsed = {}
    for stage, name in STAGES:
        text = (LARK / "transcript" / name).read_text(encoding="ascii").strip()
        raw = base64.b64decode(text, validate=True)
        maps = parse_psbt(raw)
        parsed[stage] = (raw, maps)
        artifacts.append({
            "stage": stage,
            "path": f"../lark/transcript/{name}",
            "decoded_length": len(raw),
            "decoded_sha256": hashlib.sha256(raw).hexdigest(),
        })

    # The frozen transaction is the exact stage-1 PSBT with only reserved ae records removed.
    global_map, inputs, outputs = parsed[1][1]
    stripped_inputs = [[entry for entry in scope if not entry[0].startswith(b"\xfc\x02ae")] for scope in inputs]
    frozen = serialize_psbt(global_map, stripped_inputs, outputs)
    digest = hashlib.sha256(frozen).digest()

    expected_by_stage = {1: {0}, 2: {0, 1}, 3: {0, 1, 2}, 4: {0, 1, 2}}
    expected_values = {0: COMMITMENT, 1: OPENING, 2: RHO}
    for stage, (_, (_, inputs, _)) in parsed.items():
        scope = inputs[0]
        found = {subtype: dict(scope).get(ae_key(subtype)) for subtype in range(3)}
        if {key for key, value in found.items() if value is not None} != expected_by_stage[stage]:
            raise ValueError(f"stage {stage}: unexpected ae record set")
        for subtype, value in found.items():
            if value is not None and value != expected_values[subtype]:
                raise ValueError(f"stage {stage}: ae subtype {subtype} changed")
        unknown = [key for key, _ in scope if key.startswith(b"\xfc\x02ae") and key not in {ae_key(i) for i in range(3)}]
        if unknown:
            raise ValueError(f"stage {stage}: unknown/out-of-slot ae record")

    partial = dict(parsed[4][1][1][0]).get(b"\x02" + PUBKEY)
    if partial is None or der_to_compact(partial) != SIGNATURE:
        raise ValueError("stage 4 device signature changed")

    messages = []
    for stage, _ in STAGES:
        message = aexb(stage, digest)
        package = aext(message, frozen if stage in (1, 3) else None)
        messages.append({
            "stage": stage,
            "logical_message_hex": message.hex(),
            "logical_message_length": len(message),
            "logical_message_sha256": hashlib.sha256(message).hexdigest(),
            "detached_aext_hex": package.hex(),
            "detached_aext_length": len(package),
            "detached_aext_sha256": hashlib.sha256(package).hexdigest(),
            "in_psbt_decoded_sha256": artifacts[stage - 1]["decoded_sha256"],
            "mapping": {
                "authoritative_from_coordinator": ["network", "session_id", "psbt_digest", "input_index", "sighash_type", "signer_pubkey", "message_hash", "host_commitment"],
                "lifted_from_signer": ["opening"] if stage == 2 else (["signature_compact"] if stage == 4 else []),
            },
        })

    return {
        "schema": "ae-profile-neutral-mapping-v1",
        "status": "mapping-conformance-not-independent-detached-hardware",
        "source": {
            "drongo_repository": "https://github.com/bitcoinshooter/drongo",
            "drongo_commit": "dc5d836",
            "drongo_tag": "jade-in-psbt-profile-2026-08-27",
            "recorded_profile": "in-PSBT",
            "projected_profile": "AEXT/AEXB",
        },
        "identity": {
            "network": "testnet4",
            "session_id": SESSION_ID.hex(),
            "frozen_psbt_length": len(frozen),
            "frozen_psbt_sha256": digest.hex(),
            "slot_count": 1,
            "signer_pubkey": PUBKEY.hex(),
            "message_hash": MESSAGE_HASH.hex(),
        },
        "profile_rules": {
            "logical_authority": "canonical AEXB fields",
            "in_psbt_response_allowlist": "stage 2 lifts only opening; stage 4 lifts only the selected-key partial signature",
            "returned_psbt": "never merged; transaction must match frozen transaction and verified signatures are applied to the frozen original",
            "reserved_namespace": "unknown ae subtypes and ae records outside the expected slot set are rejected",
        },
        "in_psbt_artifacts": artifacts,
        "messages": messages,
    }


def rendered() -> str:
    return json.dumps(build(), indent=2, sort_keys=True) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    value = rendered()
    if args.check:
        if not OUTPUT.exists() or OUTPUT.read_text(encoding="utf-8") != value:
            raise SystemExit("stale Jade profile mapping corpus")
    else:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(value, encoding="utf-8", newline="\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
