#!/usr/bin/env python3

"""Prove that a formatting delta preserves Clang's non-trivia C tokens."""

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys


TOKEN_RE = re.compile(
    rb"(?ms)^([a-z_]+) '(.*?)'\s+.*?Loc=<.*?>\r?$"
)


def tokens(clang: str, path: Path) -> list[tuple[str, str]]:
    process = subprocess.run(
        [clang, "-cc1", "-dump-raw-tokens", str(path)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    output = process.stdout + process.stderr
    parsed = []
    for match in TOKEN_RE.finditer(output):
        kind = match.group(1).decode("ascii")
        if kind in {"unknown", "comment"}:
            continue
        spelling = match.group(2).decode("utf-8", errors="surrogateescape")
        parsed.append((kind, spelling))
    if not parsed:
        raise RuntimeError(f"Clang produced no tokens for {path}")
    return parsed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("before_root", type=Path)
    parser.add_argument("after_root", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--clang", default="clang")
    parser.add_argument("--json-output", type=Path)
    args = parser.parse_args()

    paths = [
        Path(line.strip())
        for line in args.manifest.read_text(encoding="utf-8").splitlines()
        if line.strip()
    ]
    results = []
    mismatches = 0
    for relative in paths:
        before = tokens(args.clang, args.before_root / relative)
        after = tokens(args.clang, args.after_root / relative)
        match = before == after
        first_difference = None
        if not match:
            mismatches += 1
            for index, (before_token, after_token) in enumerate(zip(before, after)):
                if before_token != after_token:
                    first_difference = {
                        "index": index,
                        "before": before_token,
                        "after": after_token,
                    }
                    break
            if first_difference is None:
                first_difference = {"index": min(len(before), len(after))}
        result = {
            "path": relative.as_posix(),
            "before_token_count": len(before),
            "after_token_count": len(after),
            "match": match,
        }
        if first_difference is not None:
            result["first_difference"] = first_difference
        results.append(result)

    summary = {
        "clang": subprocess.run(
            [args.clang, "--version"],
            text=True,
            stdout=subprocess.PIPE,
            check=True,
        ).stdout.splitlines()[0],
        "file_count": len(results),
        "mismatch_count": mismatches,
        "files": results,
    }
    rendered = json.dumps(summary, indent=2, sort_keys=True) + "\n"
    if args.json_output:
        args.json_output.write_text(rendered, encoding="utf-8")
    else:
        sys.stdout.write(rendered)
    return 1 if mismatches else 0


if __name__ == "__main__":
    raise SystemExit(main())
