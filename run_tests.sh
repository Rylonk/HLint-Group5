#!/usr/bin/env bash
# Runs every tests/cases/*.hl through HLInt and compares what it prints on the
# screen (stdout) with the matching .expected file.  Also checks the two
# report files for PROG3.  Run from the project folder:  make test
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/HLInt"
[ -x "$BIN" ] || { echo "Build first:  make"; exit 1; }

WORK="$(mktemp -d)"; trap 'rm -rf "$WORK"' EXIT
pass=0; fail=0

check() {  # name expected actual
    if [ "$2" == "$3" ]; then pass=$((pass+1)); printf '  PASS  %s\n' "$1"
    else fail=$((fail+1)); printf '  FAIL  %s\n    expected: %q\n    actual:   %q\n' "$1" "$2" "$3"; fi
}

for src in "$ROOT"/tests/cases/*.hl; do
    name="$(basename "$src" .hl)"
    actual="$(cd "$WORK" && "$BIN" "$src" 2>/dev/null)"
    check "$name" "$(cat "${src%.hl}.expected")" "$actual"
done

(cd "$WORK" && "$BIN" "$ROOT/samples/PROG3.HL" >/dev/null 2>&1)
check "PROG3 -> NOSPACES.TXT" "$(cat "$ROOT/tests/expected_files/PROG3_NOSPACES.TXT")" "$(cat "$WORK/NOSPACES.TXT")"
check "PROG3 -> RES_SYM.TXT"  "$(cat "$ROOT/tests/expected_files/PROG3_RES_SYM.TXT")"  "$(cat "$WORK/RES_SYM.TXT")"

echo; echo "Passed: $pass   Failed: $fail"
[ "$fail" -eq 0 ]
