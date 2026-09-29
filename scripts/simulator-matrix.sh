#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output="${1:?usage: scripts/simulator-matrix.sh OUTPUT_DIRECTORY}"
mkdir -p "$output"

run_board() {
  local board="$1"
  local build="$output/build-$board"
  echo "START:$board"
  if cmake -S "$repo_root/simulator" -B "$build" \
       -DCMAKE_BUILD_TYPE=Debug -DSIM_BOARD="$board" \
       >"$output/$board-configure.log" 2>&1 && \
     cmake --build "$build" --parallel "${SIM_BUILD_JOBS:-2}" \
       >"$output/$board-build.log" 2>&1 && \
     ctest --test-dir "$build" --output-on-failure \
       >"$output/$board-ctest.log" 2>&1; then
    echo PASS >"$output/$board.result"
    echo "PASS:$board"
  else
    echo FAIL >"$output/$board.result"
    echo "FAIL:$board"
    return 1
  fi
}

export repo_root output
export -f run_board
printf '%s\n' wave_4b wave_35 wave_5 wave_43 crowpanel wave_7b |
  xargs -P "${SIM_MATRIX_JOBS:-3}" -n 1 bash -c 'run_board "$1"' _
