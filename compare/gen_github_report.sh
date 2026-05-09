#!/usr/bin/env bash
# Assemble compare/BENCHMARK_REPORT.md for GitHub (preamble + benchmark + sizes + footer).
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/compare/BENCHMARK_REPORT.md"
BIN="$ROOT/build/compare/benchmark_suite"
QF_O="$ROOT/build/qf_math.o"
QF_LEAN_O="$ROOT/build/qf_math_lean.o"
FR_O="$ROOT/build/compare/obj/fr_math.o"

shopt -s nullglob
LF_OBJS=( "$ROOT/build/compare/obj"/lf_*.o )

if [[ ! -x "$BIN" ]]; then
  echo "benchmark_suite missing; run: make compare-github-report from repo root (Makefile builds it)." >&2
  exit 1
fi

{
  cat "$ROOT/compare/GITHUB_REPORT_PREAMBLE.md"
  echo
  "$BIN" --markdown-body
  echo
  bash "$ROOT/compare/report_sizes.sh" "$QF_O" "$QF_LEAN_O" "$FR_O" "${LF_OBJS[@]}"
  echo
  cat "$ROOT/compare/GITHUB_REPORT_FOOTER.md"
} > "$OUT"

echo "Wrote $OUT"
