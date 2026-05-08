#!/usr/bin/env bash
# Build tests with coverage, run them, require 100% executed lines in src/qf_math.c (gcov).
# Usage: tools/check_coverage.sh [--report-only]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

CC="${CC:-gcc}"
FAIL_ON_GAP=1
if [[ "${1:-}" == "--report-only" ]]; then
  FAIL_ON_GAP=0
fi

mkdir -p build
rm -f build/qf_math_test_cov build/*.gcda build/*.gcno \
  "./src#qf_math.c.gcov" 2>/dev/null || true

"$CC" -Wall -Wextra -Os -std=c99 -I src -DQF_MATH_COVERAGE \
  -ftest-coverage -fprofile-arcs \
  test/qf_math_test.c src/qf_math.c -lm -o build/qf_math_test_cov

./build/qf_math_test_cov >/dev/null

rm -f "./src#qf_math.c.gcov"
gcov -pb build/qf_math_test_cov-qf_math.gcno >/dev/null

GCOV_FILE=""
if [[ -f "./src#qf_math.c.gcov" ]]; then
  GCOV_FILE="./src#qf_math.c.gcov"
elif [[ -f "src#qf_math.c.gcov" ]]; then
  GCOV_FILE="src#qf_math.c.gcov"
else
  echo "gcov did not produce src#qf_math.c.gcov" >&2
  exit 1
fi

echo "--- gcov line summary (src/qf_math.c) ---"
grep -v '^[[:space:]]*-:[[:space:]]*0:' "$GCOV_FILE" | grep '^[[:space:]]*[0-9]' | head -5 >/dev/null
gcov -r "$GCOV_FILE" 2>/dev/null || true

if grep -q '#####:' "$GCOV_FILE"; then
  echo ""
  echo "Uncovered lines in src/qf_math.c:"
  grep '#####:' "$GCOV_FILE" || true
  if [[ "$FAIL_ON_GAP" -eq 1 ]]; then
    exit 1
  fi
else
  echo ""
  echo "100% line coverage on src/qf_math.c (gcov)."
fi
