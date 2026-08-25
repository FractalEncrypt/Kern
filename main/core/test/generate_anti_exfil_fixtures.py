#!/usr/bin/env python3
"""Generate transport-neutral C records from the pinned layered JSON corpus."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
FIXTURES = HERE / "fixtures" / "anti_exfil"
CORPUS = FIXTURES / "semantic_protocol"
OUTPUT = HERE / "anti_exfil_semantic_vectors.generated.h"

SOURCES = {
    "protocol-v1-semantic-psbt-vector.json":
        "f28d572d1ae5d2060eeb52ca9814f37ce5d54258811d3af18b78c41744e23a4e",
    "protocol-v1-negative-vectors.json":
        "f5b9d3d21210173bb35da0a0de15705b3bc1d3a3d8ab42a14183c2cd7ee97599",
}
RECORD_LENGTHS = {1: 105, 2: 138, 3: 170, 4: 202}
PRESENT_FIELDS = {
    1: "0",
    2: "ANTI_EXFIL_FIELD_OPENING",
    3: "ANTI_EXFIL_FIELD_OPENING | ANTI_EXFIL_FIELD_HOST_REVEAL",
    4: "ANTI_EXFIL_FIELD_OPENING | ANTI_EXFIL_FIELD_SIGNATURE",
}
STAGES = {
    1: "ANTI_EXFIL_STAGE_HOST_COMMIT",
    2: "ANTI_EXFIL_STAGE_SIGNER_OPENINGS",
    3: "ANTI_EXFIL_STAGE_HOST_REVEAL",
    4: "ANTI_EXFIL_STAGE_SIGNER_SIGNATURES",
}
NETWORKS = {
    0: "ANTI_EXFIL_NETWORK_MAINNET",
    1: "ANTI_EXFIL_NETWORK_TESTNET3",
    2: "ANTI_EXFIL_NETWORK_REGTEST",
    3: "ANTI_EXFIL_NETWORK_SIGNET",
    4: "ANTI_EXFIL_NETWORK_TESTNET4",
}


def checked_json(name: str) -> dict:
    raw = (CORPUS / name).read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != SOURCES[name]:
        raise ValueError(f"{name}: expected SHA-256 {SOURCES[name]}, got {digest}")
    return json.loads(raw)


def check_corpus_manifest() -> None:
    manifest = json.loads((FIXTURES / "manifest.json").read_text(encoding="utf-8"))
    semantic_layers = [name for name, layer in manifest.items()
                       if layer["semantic_test_input"]]
    if semantic_layers != ["semantic_protocol"]:
        raise ValueError("only semantic_protocol may feed semantic C fixtures")
    for layer in manifest.values():
        for relative, expected in layer["files"].items():
            actual = hashlib.sha256((FIXTURES / relative).read_bytes()).hexdigest()
            if actual != expected:
                raise ValueError(f"{relative}: expected SHA-256 {expected}, got {actual}")


def parse_message(hex_text: str) -> dict:
    raw = bytes.fromhex(hex_text)
    if len(raw) < 78 or raw[:4] != b"AEXB":
        raise ValueError("fixture does not contain a complete AEXB header")
    version, network, stage, flags = raw[4:8]
    payload_len = int.from_bytes(raw[8:12], "big")
    slot_count = int.from_bytes(raw[76:78], "big")
    record_len = RECORD_LENGTHS.get(stage)
    if record_len is None or payload_len != slot_count * record_len:
        raise ValueError("fixture has non-canonical stage payload length")
    if len(raw) != 78 + payload_len:
        raise ValueError("fixture length does not match its header")

    slots = []
    for index in range(slot_count):
        record = raw[78 + index * record_len:78 + (index + 1) * record_len]
        slot = {
            "input_index": int.from_bytes(record[0:4], "big"),
            "sighash_type": int.from_bytes(record[4:8], "big"),
            "signer_pubkey": record[8:41],
            "message_hash": record[41:73],
            "host_commitment": record[73:105],
            "opening": b"",
            "host_reveal": b"",
            "signature": b"",
        }
        if stage >= 2:
            slot["opening"] = record[105:138]
        if stage == 3:
            slot["host_reveal"] = record[138:170]
        if stage == 4:
            slot["signature"] = record[138:202]
        slots.append(slot)
    return {
        "version": version,
        "network": network,
        "stage": stage,
        "flags": flags,
        "session_id": raw[12:44],
        "psbt_digest": raw[44:76],
        "slots": slots,
    }


def c_bytes(value: bytes, indent: str) -> str:
    if not value:
        return "{0}"
    chunks = [value[i:i + 8] for i in range(0, len(value), 8)]
    lines = [", ".join(f"0x{byte:02x}" for byte in chunk) for chunk in chunks]
    if len(lines) == 1:
        return "{" + lines[0] + "}"
    return "{\n" + "\n".join(indent + line + "," for line in lines) + "\n" + indent[:-2] + "}"


def c_message(message: dict, indent: str = "    ") -> str:
    stage = message["stage"]
    lines = [
        "{",
        f"{indent}.version = {message['version']},",
        f"{indent}.network = {NETWORKS[message['network']]},",
        f"{indent}.stage = {STAGES[stage]},",
        f"{indent}.flags = {message['flags']},",
        f"{indent}.session_id = {c_bytes(message['session_id'], indent + '    ')},",
        f"{indent}.psbt_digest = {c_bytes(message['psbt_digest'], indent + '    ')},",
        f"{indent}.slot_count = {len(message['slots'])},",
        f"{indent}.slots = {{",
    ]
    for slot in message["slots"]:
        slot_indent = indent + "    "
        lines.extend([
            slot_indent + "{",
            f"{slot_indent}  .input_index = {slot['input_index']},",
            f"{slot_indent}  .sighash_type = {slot['sighash_type']},",
            f"{slot_indent}  .signer_pubkey = {c_bytes(slot['signer_pubkey'], slot_indent + '      ')},",
            f"{slot_indent}  .message_hash = {c_bytes(slot['message_hash'], slot_indent + '      ')},",
            f"{slot_indent}  .host_commitment = {c_bytes(slot['host_commitment'], slot_indent + '      ')},",
        ])
        if slot["opening"]:
            lines.append(f"{slot_indent}  .opening = {c_bytes(slot['opening'], slot_indent + '      ')},")
        if slot["host_reveal"]:
            lines.append(f"{slot_indent}  .host_reveal = {c_bytes(slot['host_reveal'], slot_indent + '      ')},")
        if slot["signature"]:
            lines.append(f"{slot_indent}  .signature = {c_bytes(slot['signature'], slot_indent + '      ')},")
        lines.extend([
            f"{slot_indent}  .present_fields = {PRESENT_FIELDS[stage]},",
            slot_indent + "},",
        ])
    lines.extend([f"{indent}}},", "  }"])
    return "\n".join(lines)


def render() -> str:
    check_corpus_manifest()
    positive = checked_json("protocol-v1-semantic-psbt-vector.json")
    negative = checked_json("protocol-v1-negative-vectors.json")
    positive_messages = [parse_message(positive[f"message_{stage}_hex"])
                         for stage in range(1, 5)]
    expected_digest = bytes.fromhex(positive["psbt_sha256"])
    psbt = bytes.fromhex(positive["psbt_hex"])
    if hashlib.sha256(psbt).digest() != expected_digest:
        raise ValueError("semantic PSBT bytes do not match their pinned digest")
    if positive["slot_count"] != len(positive["slots"]):
        raise ValueError("semantic slot count does not match its slot layer")
    for stage, message in enumerate(positive_messages, start=1):
        if message["network"] != 4 or message["stage"] != stage:
            raise ValueError("semantic message has the wrong network or stage")
        if message["psbt_digest"] != expected_digest:
            raise ValueError("semantic message changed the frozen PSBT digest")
        if len(message["slots"]) != positive["slot_count"]:
            raise ValueError("semantic message changed the complete slot set")
        for slot, layered_slot in zip(message["slots"], positive["slots"]):
            if (slot["input_index"] != layered_slot["input_index"] or
                    slot["signer_pubkey"].hex() != layered_slot["signer_pubkey"] or
                    slot["message_hash"].hex() != layered_slot["message_hash"]):
                raise ValueError("semantic message disagrees with its slot layer")
    for slot, randomness in zip(positive_messages[2]["slots"],
                                positive["host_randomness"]):
        if (slot["input_index"] != randomness["input_index"] or
                slot["signer_pubkey"].hex() != randomness["signer_pubkey"] or
                slot["host_reveal"].hex() != randomness["rho"]):
            raise ValueError("message 3 disagrees with its host-randomness layer")

    negative_cases = []
    for case in negative["cases"]:
        raw = bytes.fromhex(case["message_hex"])
        if len(raw) != case["message_length"] or case["stage"] != raw[6]:
            raise ValueError(f"{case['name']}: negative vector metadata mismatch")
        if hashlib.sha256(raw).hexdigest() != case["message_sha256"]:
            raise ValueError(f"{case['name']}: negative vector digest mismatch")
        negative_cases.append((parse_message(case["message_hex"]),
                               case["expected_error"]))

    out = [
        "/* Generated by generate_anti_exfil_fixtures.py; do not edit. */",
        "#ifndef KERN_ANTI_EXFIL_SEMANTIC_VECTORS_GENERATED_H",
        "#define KERN_ANTI_EXFIL_SEMANTIC_VECTORS_GENERATED_H",
        "",
        '#include "core/anti_exfil/anti_exfil_types.h"',
        "",
        f'#define ANTI_EXFIL_SEMANTIC_SOURCE_SHA256 "{SOURCES["protocol-v1-semantic-psbt-vector.json"]}"',
        f'#define ANTI_EXFIL_NEGATIVE_SOURCE_SHA256 "{SOURCES["protocol-v1-negative-vectors.json"]}"',
        "",
        "static const anti_exfil_message_t ANTI_EXFIL_SEMANTIC_MESSAGES[] = {",
    ]
    out.extend(c_message(message) + "," for message in positive_messages)
    out.extend([
        "};",
        "",
        "static const uint8_t ANTI_EXFIL_SEMANTIC_PSBT[] = " +
        c_bytes(psbt, "    ") + ";",
        "static const size_t ANTI_EXFIL_SEMANTIC_PSBT_LEN =",
        "    sizeof(ANTI_EXFIL_SEMANTIC_PSBT);",
        "",
        "typedef struct {",
        "  anti_exfil_message_t message;",
        "  anti_exfil_result_t expected_result;",
        "} anti_exfil_negative_fixture_t;",
        "",
        "static const anti_exfil_negative_fixture_t ANTI_EXFIL_NEGATIVE_MESSAGES[] = {",
    ])
    for message, expected in negative_cases:
        out.extend([
            "  {",
            "    .message = " + c_message(message, "      ").lstrip() + ",",
            f"    .expected_result = ANTI_EXFIL_{expected},",
            "  },",
        ])
    out.extend(["};", "", "#endif // KERN_ANTI_EXFIL_SEMANTIC_VECTORS_GENERATED_H", ""])
    return "\n".join(out)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true",
                        help="fail if the checked-in generated header is stale")
    args = parser.parse_args()
    rendered = render()
    if args.check:
        if not OUTPUT.exists() or OUTPUT.read_text(encoding="utf-8") != rendered:
            raise SystemExit("generated anti-exfil fixtures are stale")
    else:
        OUTPUT.write_text(rendered, encoding="utf-8", newline="\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
