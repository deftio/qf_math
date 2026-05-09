#!/usr/bin/env bash
# Assemble compare/BENCHMARK_REPORT.md for GitHub (preamble + benchmark + sizes + footer).
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/compare/BENCHMARK_REPORT.md"
BIN="$ROOT/build/compare/benchmark_suite"
QF_O="$ROOT/build/qf_math.o"
QF_LEAN_O="$ROOT/build/qf_math_lean.o"
FR_O="$ROOT/build/compare/obj/fr_math.o"
FR_FULL_O="$ROOT/build/compare/obj/fr_math_full.o"
FR_LEAN_O="$ROOT/build/compare/obj/fr_math_lean.o"
FASTTRIG_O="$ROOT/examples/lilygo_t_display_s3_bench/.pio/build/lilygo-t-display-s3/libd57/FastTrig/FastTrig.cpp.o"
XTENSA_SIZE="${XTENSA_SIZE:-$HOME/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-size}"

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
  FASTTRIG_O="$FASTTRIG_O" XTENSA_SIZE="$XTENSA_SIZE" bash "$ROOT/compare/report_sizes.sh" "$QF_O" "$QF_LEAN_O" "$FR_O" "$FR_FULL_O" "$FR_LEAN_O" "${LF_OBJS[@]}"
  echo
  cat "$ROOT/compare/GITHUB_REPORT_FOOTER.md"
} > "$OUT"

echo "Wrote $OUT"
