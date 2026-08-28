#!/usr/bin/env python3
"""Run the pinned Lark/Kern anti-exfil collaboration corpus in both directions."""

from __future__ import annotations

import base64
import hashlib
import json
from pathlib import Path
import subprocess
import sys


HERE = Path(__file__).resolve().parent
FIXTURES = HERE / "fixtures" / "anti_exfil"
LARK = FIXTURES / "profile_interop" / "lark"
JADE = FIXTURES / "profile_interop" / "jade"
TOOLS = LARK / "tools"
LARK_TRANSCRIPT_COMMIT = "8f53be92ca7a49659d852ca58853740ce9a1625b"
LARK_NEGATIVES_COMMIT = "10870f77421975cba7c2de8291ce652ae45a0182"
TRANSCRIPT_ARTIFACTS = {
    "M1-coordinator-to-signer.psbt.b64": (455, "be411a3909771f51458d38335d2a29e38c0f2819f378dfa89467760e00693715"),
    "M2-signer-to-coordinator.psbt.b64": (528, "d8735a990c61ab47ced0223a9e265e686cc652fca1134ad68f3b2fae5632b640"),
    "M3-coordinator-to-signer.psbt.b64": (600, "538421ff6a1f48a6d928c96348ac8efaa0f5eeb70770c82d1b0e17f106f6ca03"),
    "M4-signer-to-coordinator.psbt.b64": (708, "fc4dfbd4d8b53df10271d6a85d7b65eac269c96f13e0a72226376709e8806126"),
}
JADE_DRONGO_COMMIT = "dc5d836"
JADE_SPARROW_COMMIT = "cb2dabb"
JADE_ARTIFACT_SHA256 = {
    "journal-c882a0c7d702.json": "6ac75b9a3b8a92aaf0c17b9cb80958023c904cefd9170f923f768fbe370b9f97",
    "session-0788daa2f02d-ec8f5ee080d6.json": "327564c0fa9ef5d8b5e5d8c83bd6455b5b92465f44127a7cf31ee758bb67b3e4",
    "session-3520b40b6569-49ec0bbdee88.json": "f88552742dd818ba990a1570b1b29f4e4b9f0000a722bd317a26f8cd97ca8db2",
    "session-c882a0c7d702-34878a88adca.json": "64197c2dbcb5ac37deab80f621cf92ef7b65a6640b525f4b721890b8bb4a6535",
    "session-c882a0c7d702-57173481ec70.json": "637663e2b3bb8cf9c25091252303db62e46db36a55b6c4266927835ed68781cb",
    "session-c882a0c7d702-65cbe0bd28fb.json": "703b5fb6ad6d387c7782790e7b90c3bfc00972c8f58275acaf61c199ec00863d",
    "session-c882a0c7d702-dbeff91be9ef.json": "2dd5195e34dd5fc1a5fa431f2e157cf43b8cec376818b946215978aad0ef097e",
}


def decode_artifact(path: Path) -> bytes:
    try:
        raw = base64.b64decode(path.read_text(encoding="ascii").strip(), validate=True)
    except Exception as exc:
        raise ValueError(f"{path}: invalid base64") from exc
    if not raw.startswith(b"psbt\xff"):
        raise ValueError(f"{path}: invalid PSBT magic")
    return raw


def run(*args: str) -> None:
    subprocess.run([sys.executable, *args], cwd=HERE, check=True)


def validate_aexb(raw: bytes, stage: int, network: int, digest: str) -> None:
    record_lengths = {1: 105, 2: 138, 3: 170, 4: 202}
    if len(raw) < 78 or raw[:4] != b"AEXB" or raw[4] != 1:
        raise ValueError("invalid pinned AEXB message")
    if raw[5] != network or raw[6] != stage or raw[7] != 0:
        raise ValueError("pinned AEXB identity changed")
    payload_len = int.from_bytes(raw[8:12], "big")
    slot_count = int.from_bytes(raw[76:78], "big")
    if raw[44:76].hex() != digest or payload_len != slot_count * record_lengths[stage]:
        raise ValueError("pinned AEXB digest or record layout changed")
    if len(raw) != 78 + payload_len:
        raise ValueError("pinned AEXB length changed")


def validate_jade_artifacts() -> tuple[int, int]:
    provenance = json.loads((JADE / "PROVENANCE.json").read_text(encoding="utf-8"))
    if provenance["drongo"]["commit"] != JADE_DRONGO_COMMIT:
        raise ValueError("pinned Drongo commit changed")
    if provenance["sparrow"]["commit"] != JADE_SPARROW_COMMIT:
        raise ValueError("pinned Sparrow commit changed")

    artifact_dir = JADE / "artifacts"
    sessions = []
    journal = None
    for name, expected_hash in JADE_ARTIFACT_SHA256.items():
        path = artifact_dir / name
        if hashlib.sha256(path.read_bytes()).hexdigest() != expected_hash:
            raise ValueError(f"{name}: pinned Jade artifact changed")
        value = json.loads(path.read_text(encoding="utf-8"))
        if name.startswith("session-"):
            sessions.append(value)
        else:
            journal = value

    complete = 0
    for session in sessions:
        if session["artifact"] != "anti-exfil-session" or session["format_version"] != 2:
            raise ValueError("unexpected Jade session artifact schema")
        if session["carriage"] != "in-psbt" or session["network"] != "testnet4":
            raise ValueError("unexpected Jade session profile identity")
        retained = [message for message in session["messages"] if message is not None]
        expected_stages = [1, 2, 3, 4] if session["phase"] == "COMPLETE" else [1, 2, 3]
        if [message["stage"] for message in retained] != expected_stages:
            raise ValueError("Jade artifact retained stages do not match its durable phase")
        identity = None
        for message in retained:
            raw = bytes.fromhex(message["hex"])
            if len(raw) != message["length"]:
                raise ValueError("Jade artifact message length changed")
            validate_aexb(raw, message["stage"], 4, session["frozen_psbt_sha256"])
            current = (message["session_id"], message["psbt_digest"], message["slot_count"])
            identity = current if identity is None else identity
            if current != identity or not message["layout_consistent"]:
                raise ValueError("Jade artifact transcript identity changed between stages")
        if session["phase"] == "COMPLETE":
            complete += 1
            if not session.get("signed_psbt_sha256"):
                raise ValueError("complete Jade artifact lacks a signed-PSBT pin")

    if complete != 1 or journal is None or journal["event_count"] != len(journal["events"]):
        raise ValueError("unexpected Jade completion/journal evidence")
    if len(journal["events"]) != 3 or any(event["reason"] != "SIGNATURE_REJECTED" for event in journal["events"]):
        raise ValueError("expected three pinned post-reveal signature rejections")
    return len(sessions), len(journal["events"])


def main() -> int:
    run(str(HERE / "generate_anti_exfil_collaboration_fixtures.py"), "--check")
    run(str(HERE / "generate_anti_exfil_profile_mapping.py"), "--check")

    provenance = json.loads((LARK / "PROVENANCE.json").read_text(encoding="utf-8"))
    if (provenance["transcript_commit"] != LARK_TRANSCRIPT_COMMIT or
            provenance["adversarial_commit"] != LARK_NEGATIVES_COMMIT):
        raise ValueError("Lark provenance commit changed")

    for name, (expected_len, expected_hash) in TRANSCRIPT_ARTIFACTS.items():
        raw = decode_artifact(LARK / "transcript" / name)
        if len(raw) != expected_len or hashlib.sha256(raw).hexdigest() != expected_hash:
            raise ValueError(f"{name}: transcript artifact changed")

    negatives = json.loads((LARK / "lark-ae-negatives-v1.json").read_text(encoding="utf-8"))
    if len(negatives["cases"]) != 7:
        raise ValueError("expected seven Lark negative cases")
    for case in negatives["cases"]:
        artifact = LARK / Path(case["artifact"]).relative_to("vectors")
        raw = decode_artifact(artifact)
        if len(raw) != case["bytes"] or hashlib.sha256(raw).hexdigest() != case["sha256"]:
            raise ValueError(f"{case['id']}: negative artifact changed")

    checker = str(TOOLS / "check_vectors.py")
    run(checker, str(LARK / "lark-ae-ecdsa-crypto-v1.json"))
    run(checker, str(FIXTURES / "complete_ecdsa" / "kern-ae-ecdsa-complete-v1.json"))
    run(str(TOOLS / "verify_transcript.py"), str(LARK / "lark-ae-semantic-transcript-v1.json"))

    multisig = json.loads((FIXTURES / "profile_interop" / "kern-multisig-two-signatures-per-input-v1.json").read_text(encoding="utf-8"))
    signed = base64.b64decode(multisig["signed_psbt_base64"], validate=True)
    if hashlib.sha256(signed).hexdigest() != multisig["signed_psbt_sha256"]:
        raise ValueError("Kern multisig signed PSBT hash mismatch")
    if multisig["partial_signature_count_by_input"][multisig["multisig_input_index"]] != 2:
        raise ValueError("Kern multisig fixture lacks two signatures on one input")

    jade_sessions, jade_rejections = validate_jade_artifacts()
    mapping = json.loads((JADE / "jade-neutral-profile-mapping-v1.json").read_text(encoding="utf-8"))
    if len(mapping["messages"]) != 4 or mapping["identity"]["message_hash"] != \
            "2951a65a749283810743e1cd038d201c00d3ed17895ac2c0acaac898fa319c3f":
        raise ValueError("neutral profile mapping identity changed")

    print()
    print("PASS - cross-project collaboration corpus")
    print(f"  Lark transcript: {LARK_TRANSCRIPT_COMMIT}")
    print(f"  Lark negatives:  {LARK_NEGATIVES_COMMIT} (7 artifacts)")
    print("  Lark crypto:     18 vectors through Shooter's independent checker")
    print("  Kern crypto:      7 complete tuples through Shooter's independent checker")
    print("  Kern multisig:    input 2 contains two protected partial signatures")
    print(f"  Jade mapping:     4 in-PSBT stages projected to canonical AEXB/AEXT")
    print(f"  Jade hardware:    {jade_sessions} sessions, 1 complete, {jade_rejections} pinned rejections")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
