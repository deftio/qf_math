#!/usr/bin/env bash
#
# Cross-compile src/qf_math.c for common embedded/desktop triples and report
# `.text` sizes (Berkeley `size`), mirroring the fr_math docker workflow.
#
# Run inside the container (via docker/run.sh):
#   bash /src/docker/build_sizes.sh
#
# Outputs: build/docker_sizes.csv, build/docker_size_table.md (+ stdout summary)

set -euo pipefail

SRC="/src/src/qf_math.c"
INC="-I/src/src"
OUT="/src/build/docker_size_report"
TABLE="/src/build/docker_size_table.md"
CSV="/src/build/docker_sizes.csv"

mkdir -p "${OUT}"

STD_CFLAGS="-std=c99 -Wall -Wextra -Os -ffreestanding"

compile_one() {
    local label="$1"
    shift
    local cc="$1"
    shift
    local flags="$*"
    local obj="${OUT}/qf_math_${label}.o"

    if ! command -v "${cc}" >/dev/null 2>&1; then
        echo "n/a"
        return
    fi

    if ${cc} ${flags} ${INC} ${STD_CFLAGS} -c "${SRC}" -o "${obj}" 2>/dev/null; then
        local sz_cmd="size"
        local prefix="${cc%-gcc*}"
        prefix="${prefix%-gcc-*}"
        if [[ "${prefix}" != "${cc}" ]] && command -v "${prefix}-size" >/dev/null 2>&1; then
            sz_cmd="${prefix}-size"
        fi
        ${sz_cmd} --format=berkeley "${obj}" 2>/dev/null | tail -1 | awk '{print $1}'
    else
        echo "fail"
    fi
}

declare -a TARGET_NAMES TARGET_WIDTHS SIZES

add_row() {
    TARGET_NAMES+=("$1")
    TARGET_WIDTHS+=("$2")
    SIZES+=("$3")
}

echo "Cross-compiling qf_math.c (single TU, ${STD_CFLAGS})..."
echo ""

add_row "RP2040 (Cortex-M0+)" 32 \
    "$(compile_one rp2040 arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb)"

add_row "STM32 (Cortex-M4, soft-float)" 32 \
    "$(compile_one stm32_cm4 arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft)"

add_row "Cortex-M0 (Thumb-1)" 32 \
    "$(compile_one cm0 arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb)"

add_row "RISC-V 32 (rv32im)" 32 \
    "$(compile_one rv32 riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32)"

esp_text="$(compile_one esp32 xtensa-esp-elf-gcc)"
if [[ "${esp_text}" == "fail" || "${esp_text}" == "n/a" ]]; then
    esp_text="$(compile_one esp32_alt xtensa-esp32-elf-gcc)"
fi
add_row "ESP32 (Xtensa)" 32 "${esp_text}"
    "$(compile_one aarch64 aarch64-linux-gnu-gcc)"

add_row "MIPS (mipsel linux-gnu)" 32 \
    "$(compile_one mips mipsel-linux-gnu-gcc)"

add_row "PowerPC (linux-gnu)" 32 \
    "$(compile_one ppc powerpc-linux-gnu-gcc)"

add_row "68k (linux-gnu)" 32 \
    "$(compile_one m68k m68k-linux-gnu-gcc)"

add_row "MSP430" 16 \
    "$(compile_one msp430 msp430-elf-gcc -mmcu=msp430f5529)"

add_row "68HC11" 8 \
    "$(compile_one hc11 m68hc11-gcc)"

add_row "x86-32" 32 \
    "$(compile_one x86_32 gcc -m32)"

add_row "x86-64" 64 \
    "$(compile_one x86_64 gcc -m64)"

# ── CSV ───────────────────────────────────────────────────────────────

echo "target,width_bits,text_bytes" >"${CSV}"
for i in "${!TARGET_NAMES[@]}"; do
    echo "${TARGET_NAMES[$i]},${TARGET_WIDTHS[$i]},${SIZES[$i]}" >>"${CSV}"
done
echo "Wrote ${CSV}"
echo ""

fmt_size() {
    local val="$1"
    if [[ "${val}" == "n/a" || "${val}" == "fail" ]]; then
        echo "${val}"
    else
        awk -v v="${val}" 'BEGIN { printf "%.1f KB (%d B)", v/1024.0, v }'
    fi
}

# ── Markdown table (sort by width then text size) ─────────────────────

{
    echo "## qf_math cross-target code size (\`qf_math.c\` only, \`-Os -ffreestanding\`)"
    echo ""
    echo "Generated inside the Docker toolchain image (see \`docker/README.md\`)."
    echo ""
    echo "| Target | Word | Text (code) |"
    echo "|--------|-----:|------------:|"

    declare -a SORT_LINES
    for i in "${!TARGET_NAMES[@]}"; do
        local_sz="${SIZES[$i]}"
        if [[ "${local_sz}" =~ ^[0-9]+$ ]]; then
            sk="${local_sz}"
        else
            sk="999999"
        fi
        SORT_LINES+=("${TARGET_WIDTHS[$i]} ${sk} ${i}")
    done

    sorted=$(printf '%s\n' "${SORT_LINES[@]}" | sort -k1,1n -k2,2n)

    while read -r _w _sz idx; do
        echo "| ${TARGET_NAMES[$idx]} | ${TARGET_WIDTHS[$idx]} | $(fmt_size "${SIZES[$idx]}") |"
    done <<<"${sorted}"

    echo ""
    echo "Sizes are **text** section bytes for \`qf_math.o\` only (no linker step)."
    echo "Float-heavy code pulls soft-float helpers from libgcc when linking a full firmware image — flash totals will be higher than this object alone."
} | tee "${TABLE}"

# ── Cortex-M0 optimization sweep ────────────────────────────────────

echo ""
echo "### Optimization flags (Cortex-M0, arm-none-eabi-gcc)"
echo ""
echo "| Flags | Text |"
echo "|-------|-----:|"

for opt in O0 Os O2 O3; do
    obj="${OUT}/qf_math_cm0_${opt}.o"
    if arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb ${INC} -std=c99 -Wall -"${opt}" -ffreestanding \
        -c "${SRC}" -o "${obj}" 2>/dev/null; then
        text=$(arm-none-eabi-size --format=berkeley "${obj}" 2>/dev/null | tail -1 | awk '{print $1}')
        kb=$(awk -v t="${text}" 'BEGIN { printf "%.1f", t/1024.0 }')
        echo "| -${opt} | ${text} B (${kb} KB) |"
    else
        echo "| -${opt} | fail |"
    fi
done

# ── Optional: libfixmath subset on Cortex-M0 (after compare-deps on host) ──

LFM_DIR="/src/build/compare/third_party/libfixmath/libfixmath"
ARM_FLAGS="-mcpu=cortex-m0 -mthumb -std=c99 -Wall -Os -ffreestanding"

echo ""
echo "### Peer size check (Cortex-M0, \`-Os\`, optional)"
echo ""

if [[ -d "${LFM_DIR}" ]] && command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    qf_obj="${OUT}/qf_math_cmp_cm0.o"
    arm-none-eabi-gcc ${ARM_FLAGS} ${INC} -c "${SRC}" -o "${qf_obj}" 2>/dev/null
    qf_text=$(arm-none-eabi-size --format=berkeley "${qf_obj}" 2>/dev/null | tail -1 | awk '{print $1}')

    lfm_total=0
    LFM_SRCS="fix16.c fix16_sqrt.c fix16_exp.c fix16_trig.c uint32.c fract32.c"
    LFM_INC="-I${LFM_DIR}"
    for src in ${LFM_SRCS}; do
        obj="${OUT}/lfm_${src%.c}.o"
        arm-none-eabi-gcc ${ARM_FLAGS} ${LFM_INC} -c "${LFM_DIR}/${src}" -o "${obj}" 2>/dev/null || true
        text=$(arm-none-eabi-size --format=berkeley "${obj}" 2>/dev/null | tail -1 | awk '{print $1}')
        lfm_total=$((lfm_total + text))
    done

    qf_kb=$(awk -v t="${qf_text}" 'BEGIN { printf "%.1f", t/1024.0 }')
    lfm_kb=$(awk -v t="${lfm_total}" 'BEGIN { printf "%.1f", t/1024.0 }')
    echo "| Artifact | Text (sum of listed objects) |"
    echo "|----------|-----------------------------:|"
    echo "| qf_math.o | ${qf_text} B (${qf_kb} KB) |"
    echo "| libfixmath (bench subset .c files) | ${lfm_total} B (${lfm_kb} KB) |"
    echo ""
    echo "libfixmath sources are only present after \`make compare-deps\` on the host (\`build/compare/third_party/\`)."
else
    echo "(skipped — run \`make compare-deps\` first, or toolchain missing)"
fi

echo ""
echo "Markdown table: ${TABLE}"
