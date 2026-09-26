#!/usr/bin/env bash

# Run the repository-authoritative clang-format over a deterministic source set.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# shellcheck disable=SC1091
. "$REPO_ROOT/ci/toolchain.env"

MODE=format
case "${1:-}" in
  '') ;;
  --check) MODE=check ;;
  --print-files) MODE=print ;;
  *) echo "Usage: $0 [--check|--print-files]" >&2; exit 2 ;;
esac

FORMATTER="${CLANG_FORMAT:-clang-format}"
if ! command -v "$FORMATTER" >/dev/null 2>&1; then
  echo "Required formatter not found: $FORMATTER" >&2
  echo "Run: ./scripts/run-pinned-toolchain.sh format --check" >&2
  exit 2
fi

FORMATTER_ID="$($FORMATTER --version)"
ACTUAL_VERSION="$(printf '%s\n' "$FORMATTER_ID" | sed -nE 's/.* version ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p')"
echo "Formatter: $FORMATTER_ID" >&2
if [ "$ACTUAL_VERSION" != "$CLANG_FORMAT_VERSION" ]; then
  echo "Formatter version mismatch: required $CLANG_FORMAT_VERSION, found ${ACTUAL_VERSION:-unknown}." >&2
  echo "Run: ./scripts/run-pinned-toolchain.sh format ${1:-}" >&2
  exit 2
fi

DIRS=(
  "$REPO_ROOT/main"
  "$REPO_ROOT/components/bbqr"
  "$REPO_ROOT/components/cUR"
  "$REPO_ROOT/components/deflate_codec"
  "$REPO_ROOT/components/k_quirc"
  "$REPO_ROOT/components/sd_card"
  "$REPO_ROOT/components/video"
  "$REPO_ROOT/components/wave_4b"
  "$REPO_ROOT/components/wave_35"
  "$REPO_ROOT/components/wave_43"
  "$REPO_ROOT/components/crowpanel"
  "$REPO_ROOT/components/wave_7b"
)

for directory in "${DIRS[@]}"; do
  if [ ! -d "$directory" ]; then
    echo "Required formatter source directory is missing: ${directory#"$REPO_ROOT/"}" >&2
    echo "Initialize repository submodules before formatting." >&2
    exit 2
  fi
done

mapfile -d '' FILES < <(
  find "${DIRS[@]}" -type f \( -name '*.c' -o -name '*.h' \) \
    -not -path '*/build/*' \
    -not -name 'stb_image.h' \
    -not -name '*.generated.h' \
    -print0 | LC_ALL=C sort -z
)

if [ "$MODE" = print ]; then
  for file in "${FILES[@]}"; do
    printf '%s\n' "${file#"$REPO_ROOT/"}"
  done
  exit 0
fi

echo "Declared formatter files: ${#FILES[@]}"
FAILED=0
for file in "${FILES[@]}"; do
  if [ "$MODE" = check ]; then
    if ! "$FORMATTER" --dry-run -Werror "$file" >/dev/null 2>&1; then
      printf 'needs formatting: %s\n' "${file#"$REPO_ROOT/"}" >&2
      FAILED=1
    fi
  else
    "$FORMATTER" -i "$file"
  fi
done

if [ "$FAILED" -ne 0 ]; then
  echo "Format check failed." >&2
  exit 1
fi

if [ "$MODE" = check ]; then
  echo "Format check passed."
else
  echo "Formatting complete."
fi
