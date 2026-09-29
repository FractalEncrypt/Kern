#!/usr/bin/env python3
"""Create a Clang-Tidy-compatible copy of an ESP-IDF compile database."""

import argparse
import json
import pathlib
import shlex


UNSUPPORTED_EXACT = {
    "-fno-malloc-dce",
    "-fno-tree-switch-conversion",
    "-fstrict-volatile-bitfields",
    "-Wno-old-style-declaration",
}
UNSUPPORTED_PREFIXES = (
    "-march=",
    "-mtune=",
    "-fzero-init-padding-bits=",
    "-specs=",
)
RISCV_TOOLCHAIN = pathlib.Path(
    "/opt/esp/tools/riscv32-esp-elf/esp-15.2.0_20251204/riscv32-esp-elf"
)
ANALYSIS_ARGUMENTS = [
    "--target=riscv32-unknown-elf",
    "-march=rv32imafc",
    "-isystem",
    str(RISCV_TOOLCHAIN / "picolibc/riscv32-esp-elf/sys-include"),
    "-Wno-unused-command-line-argument",
]
FIRST_PARTY_PREFIXES = (
    "/project/main/",
    "/project/components/bbqr/",
    "/project/components/cUR/",
    "/project/components/k_quirc/",
    "/project/components/sd_card/",
    "/project/components/video/",
    "/project/components/wave_4b/",
    "/project/components/wave_35/",
    "/project/components/wave_43/",
    "/project/components/crowpanel/",
    "/project/components/wave_7b/",
)


def normalize(arguments: list[str], replace_driver: bool = True) -> list[str]:
    normalized = []
    for argument in arguments:
        if argument.startswith("@"):
            response_file = pathlib.Path(argument[1:])
            if response_file.is_file():
                normalized.extend(
                    normalize(
                        shlex.split(response_file.read_text(encoding="utf-8")),
                        replace_driver=False,
                    )
                )
                continue
        if argument in UNSUPPORTED_EXACT or argument.startswith(UNSUPPORTED_PREFIXES):
            continue
        normalized.append(argument)
    if replace_driver and normalized:
        normalized[0] = "clang"
        normalized[1:1] = ANALYSIS_ARGUMENTS
    return normalized


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=pathlib.Path)
    parser.add_argument("destination", type=pathlib.Path)
    parser.add_argument("--files-output", type=pathlib.Path)
    args = parser.parse_args()

    with args.source.open(encoding="utf-8") as source_file:
        database = json.load(source_file)

    for entry in database:
        if "arguments" in entry:
            entry["arguments"] = normalize(entry["arguments"])
        else:
            entry["command"] = shlex.join(normalize(shlex.split(entry["command"])))

    args.destination.mkdir(parents=True, exist_ok=True)
    output = args.destination / "compile_commands.json"
    with output.open("w", encoding="utf-8", newline="\n") as output_file:
        json.dump(database, output_file, indent=2)
        output_file.write("\n")

    if args.files_output:
        files = sorted(
            {
                entry["file"]
                for entry in database
                if entry["file"].endswith(".c")
                and entry["file"].startswith(FIRST_PARTY_PREFIXES)
            }
        )
        args.files_output.parent.mkdir(parents=True, exist_ok=True)
        args.files_output.write_text("".join(f"{item}\n" for item in files), encoding="utf-8")


if __name__ == "__main__":
    main()
