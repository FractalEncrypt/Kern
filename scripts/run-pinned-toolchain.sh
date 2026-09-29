#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# shellcheck disable=SC1091
. "$REPO_ROOT/ci/toolchain.env"

usage() {
  cat <<'EOF'
Usage: scripts/run-pinned-toolchain.sh COMMAND [ARGUMENTS]

Commands:
  build                         Build the digest-pinned Clang toolchain image.
  sanitize-clang OUTPUT [N]     Run Clang ASan/LSan N times (default: 2).
  sanitize-gcc OUTPUT [N]       Run the GCC 11.4 continuity lane N times.
  format [--check|--print-files]
                                Run the exact-version repository formatter.
EOF
}

build_clang_image() {
  local cache="$REPO_ROOT/ci/.toolchain-cache"
  mkdir -p "$cache"
  while read -r digest size url filename; do
    case "$digest" in ''|'#'*) continue ;; esac
    local target="$cache/$filename"
    if [ ! -f "$target" ] ||
       [ "$(wc -c < "$target" | tr -d ' ')" != "$size" ] ||
       [ "$(sha256sum "$target" | cut -d' ' -f1)" != "$digest" ]; then
      rm -f "$target"
      curl --fail --location --proto '=https' --tlsv1.2 \
        --output "$target" "$url"
    fi
    printf '%s  %s\n' "$digest" "$target" | sha256sum --check --status
    [ "$(wc -c < "$target" | tr -d ' ')" = "$size" ] || {
      echo "size mismatch: $filename" >&2
      exit 1
    }
  done < "$REPO_ROOT/ci/sanitizer-inputs.lock"
  docker build --pull=false \
    --build-arg "CLANG_BASE_IMAGE=$CLANG_BASE_IMAGE" \
    --build-arg "GCC_BASE_IMAGE=$GCC_BASE_IMAGE" \
    --file "$REPO_ROOT/ci/clang18-toolchain.Dockerfile" \
    --tag "$CLANG_TOOLCHAIN_IMAGE" \
    "$REPO_ROOT"
}

run_sanitizer() {
  local image="$1"
  local compiler="$2"
  local output="$3"
  local repeats="${4:-2}"
  mkdir -p "$output"
  docker run --rm \
    --mount "type=bind,src=$REPO_ROOT,dst=/src" \
    --mount "type=bind,src=$(cd "$output" && pwd),dst=/out" \
    --workdir /src \
    "$image" \
    ./scripts/sanitizer.sh --compiler "$compiler" --output /out --repeat "$repeats"
}

case "${1:-}" in
  build)
    build_clang_image
    ;;
  sanitize-clang)
    [ "$#" -ge 2 ] || { usage; exit 2; }
    build_clang_image
    run_sanitizer "$CLANG_TOOLCHAIN_IMAGE" clang "$2" "${3:-2}"
    ;;
  sanitize-gcc)
    [ "$#" -ge 2 ] || { usage; exit 2; }
    run_sanitizer "$GCC_BASE_IMAGE" gcc "$2" "${3:-2}"
    ;;
  format)
    shift
    "$REPO_ROOT/scripts/format.sh" "$@"
    ;;
  *)
    usage
    exit 2
    ;;
esac
