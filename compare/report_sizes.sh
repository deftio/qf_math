#!/usr/bin/env bash
# Markdown size table for compare/BENCHMARK_REPORT.md
# Usage: report_sizes.sh <qf_math.o> <qf_math_lean.o> <fr_math_bench.o> <fr_math_full.o> <fr_math_lean.o> <lf_obj1> [lf_obj2 ...]
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

maybe_dec_one() {
  if [[ -n "${1:-}" && -f "$1" ]]; then
    local out tool
    out="$(size "$1" 2>/dev/null | awk 'NR==2 { print $5+0 }' || true)"
    if [[ -n "$out" ]]; then
      echo "$out"
      return
    fi
    tool="${XTENSA_SIZE:-}"
    if [[ -z "$tool" ]]; then
      tool="$(command -v xtensa-esp32s3-elf-size 2>/dev/null || true)"
    fi
    if [[ -n "$tool" && -x "$tool" ]]; then
      out="$("$tool" "$1" 2>/dev/null | awk 'NR==2 { print $4+0 }' || true)"
      if [[ -n "$out" ]]; then
        echo "$out"
        return
      fi
    fi
    echo "---"
  else
    echo "---"
  fi
}

if [[ $# -lt 6 ]]; then
  echo "usage: $0 qf_math.o qf_math_lean.o fr_math_bench.o fr_math_full.o fr_math_lean.o lf1.o [lf2.o ...]" >&2
  exit 1
fi

QF="$1"
QF_LEAN="$2"
FR="$3"
FR_FULL="$4"
FR_LEAN="$5"
shift 5
LF=( "$@" )

SUM_LF="$(sum_many "${LF[@]}")"
DQF="$(dec_one "$QF")"
DQFL="$(dec_one "$QF_LEAN")"
DFR="$(dec_one "$FR")"
DFRF="$(dec_one "$FR_FULL")"
DFRL="$(dec_one "$FR_LEAN")"
FASTTRIG_SIZE="$(maybe_dec_one "${FASTTRIG_O:-}")"

echo "### Library footprint (ROM-sized \`.o\` totals, decimal bytes)"
echo
echo "Each **row is one compiled library/object variant** you might ship on its own (or as your SDK’s math layer). **Do not add the rows together** — real firmware picks **one** primary approximate math approach (plus vendor libm snippets). The benchmark binary links several libraries only to score them in one harness, not because products bundle them all. **qf_math** is **float**; **libfixmath** / **fr_math** are **fixed-point** (different animals — see **README.md** § Float vs fixed-point)."
echo
echo "**Where these numbers come from** — see [compare/README.md](README.md) § *Footprint rows (what is actually measured)*."
echo
echo "| Library | Variant | Bytes (dec) | What is counted |"
echo "| :--- | :--- | ---:|:---|"
echo "| **qf_math** | full | ${DQF} | Single TU \`qf_math.o\` at \`-Os\` (same as \`make lib\`). |"
echo "| **qf_math** | lean | ${DQFL} | \`qf_math.o\` with \`-DQF_MATH_LEAN\`: radian trig, inverse trig, \`log2\`/\`ln\`, \`pow2\`/\`exp\`/\`pow\`, \`sqrt\`, and \`hypot_fast8\` — no degree/BAM trig entry points, exact \`hypot\`, \`hypot_fast2\`, \`log10\`/\`pow10\`, waves, or ADSR (see \`qf_math.h\`). |"
echo "| **libfixmath** | bench subset | ${SUM_LF} | Sum of the \`lf_*.o\` objects from **Makefile** \`LIBFIXMATH_SRCS\` (trig + sqrt + exp + fix16 core) — only what the compare harness links. |"
echo "| **fr_math** | full | ${DFRF} | \`FR_math.c\` at \`-Os\` with default feature set. |"
echo "| **fr_math** | lean | ${DFRL} | \`FR_math.c\` with \`-DFR_LEAN -DFR_NO_PRINT\`: radian trig, inverse trig, log/exp, sqrt; omits degree/BAM wrappers, hypot, waves/ADSR, and print helpers. |"
echo "| **fr_math** | bench no-print | ${DFR} | \`FR_math.c\` with \`-DFR_NO_PRINT\`, \`-Os\`; this is the object linked by the portable compare harness. |"
echo "| **FastTrig** | ESP32 peer object | ${FASTTRIG_SIZE} | PlatformIO-built \`FastTrig.cpp.o\` when the LilyGO benchmark has been built; \`---\` if unavailable on this machine. |"
echo "| **ESP-DSP** | local scalar subset | --- | No standalone library object in this harness; the sqrt approximation is a tiny local wrapper in \`main.cpp\`. |"
echo "| **espp/math** | local scalar subset | --- | Header-only/local scalar subset in the LilyGO benchmark, not a separately sized library object. |"
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
