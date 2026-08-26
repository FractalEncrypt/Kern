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
TOOLS = LARK / "tools"
LARK_TRANSCRIPT_COMMIT = "8f53be92ca7a49659d852ca58853740ce9a1625b"
LARK_NEGATIVES_COMMIT = "10870f77421975cba7c2de8291ce652ae45a0182"
TRANSCRIPT_ARTIFACTS = {
    "M1-coordinator-to-signer.psbt.b64": (455, "be411a3909771f51458d38335d2a29e38c0f2819f378dfa89467760e00693715"),
    "M2-signer-to-coordinator.psbt.b64": (528, "d8735a990c61ab47ced0223a9e265e686cc652fca1134ad68f3b2fae5632b640"),
    "M3-coordinator-to-signer.psbt.b64": (600, "538421ff6a1f48a6d928c96348ac8efaa0f5eeb70770c82d1b0e17f106f6ca03"),
    "M4-signer-to-coordinator.psbt.b64": (708, "fc4dfbd4d8b53df10271d6a85d7b65eac269c96f13e0a72226376709e8806126"),
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


def main() -> int:
    run(str(HERE / "generate_anti_exfil_collaboration_fixtures.py"), "--check")

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

    print()
    print("PASS - cross-project collaboration corpus")
    print(f"  Lark transcript: {LARK_TRANSCRIPT_COMMIT}")
    print(f"  Lark negatives:  {LARK_NEGATIVES_COMMIT} (7 artifacts)")
    print("  Lark crypto:     18 vectors through Shooter's independent checker")
    print("  Kern crypto:      7 complete tuples through Shooter's independent checker")
    print("  Kern multisig:    input 2 contains two protected partial signatures")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
