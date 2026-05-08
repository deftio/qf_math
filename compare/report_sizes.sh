#!/usr/bin/env bash
# Markdown size table for compare/BENCHMARK_REPORT.md
# Usage: report_sizes.sh <qf_math.o> <fr_math.o> <lf_obj1> [lf_obj2 ...]
set -eu

dec_one() {
  size "$1" | awk 'NR==2 { print $5+0 }'
}

sum_many() {
  local f t=0
  for f in "$@"; do
    t=$((t + $(dec_one "$f")))
  done
  echo "$t"
}

if [[ $# -lt 3 ]]; then
  echo "usage: $0 qf_math.o fr_math.o lf1.o [lf2.o ...]" >&2
  exit 1
fi

QF="$1"
FR="$2"
shift 2
LF=( "$@" )

SUM_LF="$(sum_many "${LF[@]}")"
DQF="$(dec_one "$QF")"
DFR="$(dec_one "$FR")"

echo "### Library footprint (ROM-sized \`.o\` totals, decimal bytes)"
echo
echo "Each **row is one stack** you might ship on its own (or as your SDK’s math layer). **Do not add the rows together** — real firmware picks **one** primary approximate math approach (plus vendor libm snippets). The benchmark binary links several libraries only to score them in one harness, not because products bundle them all. **qf_math** is **float**; **libfixmath** / **fr_math** are **fixed-point** (different animals — see **README.md** § Float vs fixed-point)."
echo
echo "| Library | Bytes (dec) | What is counted |"
echo "| :--- | ---:|:---|"
echo "| **qf_math** | ${DQF} | Single TU \`qf_math.o\` at \`-Os\` (same as \`make lib\`). |"
echo "| **libfixmath** (bench subset) | ${SUM_LF} | Sum of fix16 + trig + sqrt + exp objects used by this compare link — see repo Makefile. |"
echo "| **fr_math** | ${DFR} | \`FR_math.c\` with \`-DFR_NO_PRINT\`, \`-Os\`. |"
echo
echo "<details><summary>libfixmath object breakdown (bench link only)</summary>"
echo
echo "| Object file | Bytes |"
echo "| :--- | ---:|"
for o in "${LF[@]}"; do
  printf '| `%s` | %s |\n' "$(basename "$o")" "$(dec_one "$o")"
done
echo
echo "</details>"
