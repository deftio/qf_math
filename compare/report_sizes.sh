#!/usr/bin/env bash
# Markdown size table for compare/BENCHMARK_REPORT.md
# Usage: report_sizes.sh <qf_math.o> <qf_math_lean.o> <fr_math.o> <lf_obj1> [lf_obj2 ...]
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

if [[ $# -lt 4 ]]; then
  echo "usage: $0 qf_math.o qf_math_lean.o fr_math.o lf1.o [lf2.o ...]" >&2
  exit 1
fi

QF="$1"
QF_LEAN="$2"
FR="$3"
shift 3
LF=( "$@" )

SUM_LF="$(sum_many "${LF[@]}")"
DQF="$(dec_one "$QF")"
DQFL="$(dec_one "$QF_LEAN")"
DFR="$(dec_one "$FR")"

echo "### Library footprint (ROM-sized \`.o\` totals, decimal bytes)"
echo
echo "Each **row is one stack** you might ship on its own (or as your SDK’s math layer). **Do not add the rows together** — real firmware picks **one** primary approximate math approach (plus vendor libm snippets). The benchmark binary links several libraries only to score them in one harness, not because products bundle them all. **qf_math** is **float**; **libfixmath** / **fr_math** are **fixed-point** (different animals — see **README.md** § Float vs fixed-point)."
echo
echo "**Where these numbers come from** — see [compare/README.md](README.md) § *Footprint rows (what is actually measured)*."
echo
echo "| Library | Bytes (dec) | What is counted |"
echo "| :--- | ---:|:---|"
echo "| **qf_math** (full) | ${DQF} | Single TU \`qf_math.o\` at \`-Os\` (same as \`make lib\`). |"
echo "| **qf_math** (peer-comparable lean) | ${DQFL} | \`qf_math.o\` with \`-DQF_MATH_LEAN\`: rad/deg/BAM trig, inverse trig, \`log2\`/\`ln\`, \`pow2\`/\`exp\`, \`sqrt\`, \`hypot\`/\`hypot_fast8\` — no \`log10\`/\`pow10\`, waves, or ADSR (see \`qf_math.h\`). |"
echo "| **libfixmath** (bench subset) | ${SUM_LF} | Sum of the \`lf_*.o\` objects from **Makefile** \`LIBFIXMATH_SRCS\` (trig + sqrt + exp + fix16 core) — only what the compare harness links. |"
echo "| **fr_math** | ${DFR} | \`FR_math.c\` with \`-DFR_NO_PRINT\`, \`-Os\`. |"
echo
echo "> **Note:** There is no single **\`libm\`** row — vendor math lives in prebuilt toolchain archives; the perf tables time \`sinf\`/\`cosf\`/…, not a standalone \`.o\` total."
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
