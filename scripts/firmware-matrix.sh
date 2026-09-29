#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output="${1:?usage: scripts/firmware-matrix.sh OUTPUT_DIRECTORY}"
mkdir -p "$output"
cd "$repo_root"

boards=(
  wave_4b wave_35 wave_5 wave_43 crowpanel wave_7b
  wave_4b_v3 wave_35_v3 wave_5_v3 wave_43_v3 crowpanel_v3 wave_7b_v3
)

for board in "${boards[@]}"; do
  base="${board%_v3}"
  defaults="sdkconfig.defaults;sdkconfig.defaults.$base"
  if [[ "$board" == *_v3 ]]; then
    defaults="$defaults;sdkconfig.rev3"
  fi
  echo "MATRIX_START:$board"
  idf.py -B "build_$board" \
    -D "SDKCONFIG=build_$board/sdkconfig" \
    -D "SDKCONFIG_DEFAULTS=$defaults" build \
    >"$output/$board.log" 2>&1
  echo PASS >"$output/$board.result"
  echo "MATRIX_PASS:$board"
done
