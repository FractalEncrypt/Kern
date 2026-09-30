#!/usr/bin/env bash

# Build and run every host-test binary under ASan/LSan. Each executable is
# linked through an auditable main() marker so loader/runtime startup failures
# cannot be mistaken for product crashes. Deliberately faulty controls use the
# same compiler and sanitizer flags as the real targets.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
COMPILER="clang"
OUTPUT="$REPO_ROOT/.sanitizer-output"
REPEAT=2

while [ "$#" -gt 0 ]; do
  case "$1" in
    --compiler) COMPILER="$2"; shift 2 ;;
    --output) OUTPUT="$2"; shift 2 ;;
    --repeat) REPEAT="$2"; shift 2 ;;
    *) echo "Unknown argument: $1" >&2; exit 2 ;;
  esac
done

case "$COMPILER" in clang|gcc) ;; *) echo "Unsupported compiler: $COMPILER" >&2; exit 2 ;; esac
case "$REPEAT" in ''|*[!0-9]*|0) echo "--repeat must be a positive integer" >&2; exit 2 ;; esac

mkdir -p "$OUTPUT/raw" "$OUTPUT/build" "$OUTPUT/bin"
OUTPUT="$(cd "$OUTPUT" && pwd)"
FLAGS=(-g -O1 -fno-omit-frame-pointer -fsanitize=address)
export ASAN_OPTIONS="halt_on_error=1:detect_leaks=1:abort_on_error=0:strict_string_checks=1"
export LSAN_OPTIONS="exitcode=23"

SUMMARY="$OUTPUT/results.jsonl"
: > "$SUMMARY"
: > "$OUTPUT/compile-commands.log"
FAILURES=0

record() {
  local phase="$1" name="$2" attempt="$3" status="$4" entered="$5" classification="$6" log="$7"
  printf '{"phase":"%s","name":"%s","attempt":%s,"exit_status":%s,"main_entered":%s,"classification":"%s","log":"%s"}\n' \
    "$phase" "$name" "$attempt" "$status" "$entered" "$classification" "$log" >> "$SUMMARY"
}

classify_real() {
  local status="$1" log="$2"
  local entered=false
  grep -q 'KERN_SANITIZER_MAIN_ENTERED' "$log" && entered=true
  if [ "$entered" = false ]; then
    CLASSIFICATION=SANITIZER_INFRASTRUCTURE_STARTUP_FAILURE
  elif grep -qE 'ERROR: (Address|Leak)Sanitizer|SUMMARY: AddressSanitizer' "$log"; then
    CLASSIFICATION=PRODUCT_SANITIZER_FINDING
  elif [ "$status" -ne 0 ]; then
    CLASSIFICATION=PRODUCT_TEST_FAILURE
  else
    CLASSIFICATION=PASS
  fi
  MAIN_ENTERED="$entered"
}

{
  echo "timestamp_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "kernel=$(uname -a)"
  echo "architecture=$(uname -m)"
  echo "os_release=$(grep PRETTY_NAME /etc/os-release | cut -d= -f2-)"
  echo "vm.mmap_rnd_bits=$(cat /proc/sys/vm/mmap_rnd_bits 2>/dev/null || echo unavailable) [read-only]"
  echo "kernel.randomize_va_space=$(cat /proc/sys/kernel/randomize_va_space 2>/dev/null || echo unavailable) [read-only]"
  echo "compiler=$($COMPILER --version | head -1)"
  echo "linker=$($COMPILER -Wl,--version 2>&1 | head -1)"
  echo "make=$(make --version | head -1)"
  echo "python=$(python3 --version)"
  echo "flags=${FLAGS[*]}"
  echo "ASAN_OPTIONS=$ASAN_OPTIONS"
  echo "LSAN_OPTIONS=$LSAN_OPTIONS"
  echo "repeat=$REPEAT"
} | tee "$OUTPUT/environment.txt"

MARKER_OBJ="$OUTPUT/build/main_marker.o"
"$COMPILER" "${FLAGS[@]}" -c "$REPO_ROOT/tests/sanitizer-controls/main_marker.c" -o "$MARKER_OBJ"

WRAPPER="$OUTPUT/build/sanitizer-cc"
cat > "$WRAPPER" <<'EOF'
#!/usr/bin/env bash
set -eu
flags=(-g -O1 -fno-omit-frame-pointer -fsanitize=address)
{
  printf '%q ' "$SANITIZER_CC" "${flags[@]}" "$@"
  printf '\n'
} >> "$KERN_SANITIZER_COMMAND_LOG"
link=true
for argument in "$@"; do
  case "$argument" in -c|-E|-S) link=false ;; esac
done
if $link; then
  exec "$SANITIZER_CC" "${flags[@]}" "$@" "$KERN_SANITIZER_MARKER_OBJ" -Wl,--wrap=main
fi
exec "$SANITIZER_CC" "${flags[@]}" "$@"
EOF
chmod +x "$WRAPPER"
export SANITIZER_CC="$COMPILER"
export KERN_SANITIZER_MARKER_OBJ="$MARKER_OBJ"
export KERN_SANITIZER_COMMAND_LOG="$OUTPUT/compile-commands.log"

echo "Running sanitizer positive controls..."
for control in heap_oob use_after_free leak; do
  source="$REPO_ROOT/tests/sanitizer-controls/$control.c"
  binary="$OUTPUT/bin/$control"
  "$COMPILER" "${FLAGS[@]}" "$source" -o "$binary"
  for attempt in $(seq 1 "$REPEAT"); do
    log="$OUTPUT/raw/control-$control-attempt-$attempt.log"
    timeout --signal=KILL 30s "$binary" 64 > "$log" 2>&1
    status=$?
    entered=false
    grep -q 'KERN_SANITIZER_MAIN_ENTERED' "$log" && entered=true
    expected=false
    case "$control" in
      heap_oob) grep -q 'heap-buffer-overflow' "$log" && expected=true ;;
      use_after_free) grep -q 'heap-use-after-free' "$log" && expected=true ;;
      leak) grep -qE 'LeakSanitizer: detected memory leaks|Direct leak of' "$log" && expected=true ;;
    esac
    if [ "$entered" = false ]; then
      classification=SANITIZER_INFRASTRUCTURE_STARTUP_FAILURE
    elif [ "$status" -eq 0 ] || [ "$expected" = false ]; then
      classification=SANITIZER_POSITIVE_CONTROL_FAILURE
    else
      classification=PASS
    fi
    record control "$control" "$attempt" "$status" "$entered" "$classification" "raw/$(basename "$log")"
    if [ "$classification" != PASS ]; then FAILURES=$((FAILURES + 1)); fi
    echo "control=$control attempt=$attempt status=$status main_entered=$entered classification=$classification"
  done
done

if [ "$FAILURES" -ne 0 ]; then
  echo "Positive controls did not qualify the sanitizer runtime; real targets will not run." >&2
  exit 1
fi

SIMPLE_SUITES=(
  "deflate_codec|components/deflate_codec/test|test_deflate_codec"
  "bbqr|components/bbqr/test|test_base32 test_bbqr"
)
CORE_BINS=(
  test_derivation_path test_ss_whitelist_parse test_ss_whitelist_is_whitelisted
  test_ss_whitelist_regen test_purpose_binding test_registry_match
  test_registry_parse test_psbt_classify test_miniscript_policy test_bip322
  test_estimated_entropy test_anti_exfil_crypto test_anti_exfil_semantic
  test_anti_exfil_slots test_anti_exfil_signer test_anti_exfil_transport
  test_anti_exfil_response test_anti_exfil_continuation
)

for attempt in $(seq 1 "$REPEAT"); do
  echo "Building and running host suites: attempt $attempt/$REPEAT"
  for spec in "${SIMPLE_SUITES[@]}"; do
    IFS='|' read -r suite directory binaries <<< "$spec"
    build_log="$OUTPUT/raw/build-$suite-attempt-$attempt.log"
    make -C "$REPO_ROOT/$directory" clean >/dev/null 2>&1
    timeout --signal=KILL 600s make -C "$REPO_ROOT/$directory" all CC="$WRAPPER" > "$build_log" 2>&1
    build_status=$?
    if [ "$build_status" -ne 0 ]; then
      record build "$suite" "$attempt" "$build_status" false PRODUCT_TEST_FAILURE "raw/$(basename "$build_log")"
      FAILURES=$((FAILURES + 1))
      continue
    fi
    record build "$suite" "$attempt" 0 false PASS "raw/$(basename "$build_log")"
    for binary in $binaries; do
      log="$OUTPUT/raw/$suite-$binary-attempt-$attempt.log"
      (cd "$REPO_ROOT/$directory" && timeout --signal=KILL 120s "./$binary") > "$log" 2>&1
      status=$?
      classify_real "$status" "$log"
      symbols=$(nm "$REPO_ROOT/$directory/$binary" 2>/dev/null | grep -cE '__asan|__lsan' || true)
      printf 'instrumentation_symbols=%s\n' "$symbols" >> "$log"
      if [ "$symbols" -eq 0 ] && [ "$CLASSIFICATION" = PASS ]; then
        CLASSIFICATION=SANITIZER_INFRASTRUCTURE_STARTUP_FAILURE
        FAILURES=$((FAILURES + 1))
      elif [ "$CLASSIFICATION" != PASS ]; then
        FAILURES=$((FAILURES + 1))
      fi
      record real "$suite/$binary" "$attempt" "$status" "$MAIN_ENTERED" "$CLASSIFICATION" "raw/$(basename "$log")"
    done
  done

  build_log="$OUTPUT/raw/build-core-attempt-$attempt.log"
  make -C "$REPO_ROOT/main/core/test" clean >/dev/null 2>&1
  timeout --signal=KILL 1800s make -C "$REPO_ROOT/main/core/test" all CC="$WRAPPER" > "$build_log" 2>&1
  build_status=$?
  if [ "$build_status" -ne 0 ]; then
    record build core "$attempt" "$build_status" false PRODUCT_TEST_FAILURE "raw/$(basename "$build_log")"
    FAILURES=$((FAILURES + 1))
    continue
  fi
  record build core "$attempt" 0 false PASS "raw/$(basename "$build_log")"
  for binary in "${CORE_BINS[@]}"; do
    log="$OUTPUT/raw/core-$binary-attempt-$attempt.log"
    (cd "$REPO_ROOT/main/core/test" && timeout --signal=KILL 120s "./$binary") > "$log" 2>&1
    status=$?
    classify_real "$status" "$log"
    symbols=$(nm "$REPO_ROOT/main/core/test/$binary" 2>/dev/null | grep -cE '__asan|__lsan' || true)
    printf 'instrumentation_symbols=%s\n' "$symbols" >> "$log"
    if [ "$symbols" -eq 0 ] && [ "$CLASSIFICATION" = PASS ]; then
      CLASSIFICATION=SANITIZER_INFRASTRUCTURE_STARTUP_FAILURE
      FAILURES=$((FAILURES + 1))
    elif [ "$CLASSIFICATION" != PASS ]; then
      FAILURES=$((FAILURES + 1))
    fi
    record real "core/$binary" "$attempt" "$status" "$MAIN_ENTERED" "$CLASSIFICATION" "raw/$(basename "$log")"
  done
done

python3 - "$SUMMARY" "$OUTPUT/summary.json" "$COMPILER" "$FAILURES" <<'PY'
import json
import pathlib
import sys

records = [json.loads(line) for line in pathlib.Path(sys.argv[1]).read_text().splitlines()]
counts = {}
for record in records:
    key = record["classification"]
    counts[key] = counts.get(key, 0) + 1
summary = {
    "compiler": sys.argv[3],
    "failure_count": int(sys.argv[4]),
    "classification_counts": counts,
    "records": records,
}
pathlib.Path(sys.argv[2]).write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")
PY

echo "Sanitizer summary: $OUTPUT/summary.json"
echo "failure_count=$FAILURES"
[ "$FAILURES" -eq 0 ]
