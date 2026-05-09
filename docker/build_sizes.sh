#!/usr/bin/env bash
#
# Cross-compile qf_math and, when compare-deps are present, fr_math for common
# embedded/desktop triples. This mirrors fr_math's docker size workflow but
# reports qf full/lean next to fr full/core so table-heavy float builds are
# visible across real toolchains.
#
# Run inside the container:
#   bash /src/docker/build_sizes.sh
#
# Outputs:
#   build/docker_sizes.csv
#   build/docker_size_report/*.o

set -euo pipefail

QF_SRC="/src/src/qf_math.c"
QF_INC="-I/src/src"
FR_SRC="/src/build/compare/third_party/fr_math/src/FR_math.c"
FR_INC="-I/src/build/compare/third_party/fr_math/src"
OUT="/src/build/docker_size_report"
CSV="/src/build/docker_sizes.csv"

mkdir -p "${OUT}"

STD_CFLAGS="-std=c99 -Wall -Os -ffreestanding"

resolve_size_tool() {
    local cc="$1"
    local prefix="${cc%-gcc*}"
    prefix="${prefix%-gcc-*}"
    if [[ "${prefix}" != "${cc}" ]] && command -v "${prefix}-size" >/dev/null 2>&1; then
        echo "${prefix}-size"
    else
        echo "size"
    fi
}

# compile_one <label> <src> <inc> <compiler> <flags...>
# Prints "text:data:bss:total" or "n/a"/"fail".
compile_one() {
    local label="$1"
    shift
    local src="$1"
    shift
    local inc="$1"
    shift
    local cc="$1"
    shift
    local flags="$*"
    local obj="${OUT}/${label}.o"

    if ! command -v "${cc}" >/dev/null 2>&1; then
        echo "n/a"
        return
    fi

    if ${cc} ${inc} ${STD_CFLAGS} ${flags} -c "${src}" -o "${obj}" 2>/dev/null; then
        local sz_cmd
        sz_cmd="$(resolve_size_tool "${cc}")"
        ${sz_cmd} --format=berkeley "${obj}" 2>/dev/null | tail -1 | awk '{print $1 ":" $2 ":" $3 ":" $4}'
    else
        echo "fail"
    fi
}

declare -a TARGET_NAMES TARGET_WIDTHS QF_LEAN QF_FULL FR_CORE FR_FULL

add_row() {
    TARGET_NAMES+=("$1")
    TARGET_WIDTHS+=("$2")
    QF_LEAN+=("$3")
    QF_FULL+=("$4")
    FR_CORE+=("$5")
    FR_FULL+=("$6")
}

build_target() {
    local name="$1"
    local width="$2"
    local tag="$3"
    local cc="$4"
    shift 4
    local flags="$*"

    local qf_lean qf_full fr_core fr_full
    qf_lean="$(compile_one "qf_math_${tag}_lean" "${QF_SRC}" "${QF_INC}" "${cc}" -DQF_MATH_LEAN ${flags})"
    qf_full="$(compile_one "qf_math_${tag}_full" "${QF_SRC}" "${QF_INC}" "${cc}" ${flags})"
    if [[ -f "${FR_SRC}" ]]; then
        fr_core="$(compile_one "fr_math_${tag}_core" "${FR_SRC}" "${FR_INC}" "${cc}" -DFR_CORE_ONLY ${flags})"
        fr_full="$(compile_one "fr_math_${tag}_full" "${FR_SRC}" "${FR_INC}" "${cc}" ${flags})"
    else
        fr_core="n/a"
        fr_full="n/a"
    fi
    add_row "${name}" "${width}" "${qf_lean}" "${qf_full}" "${fr_core}" "${fr_full}"
}

echo "Cross-compiling qf_math.c and fr_math.c (${STD_CFLAGS})..."
if [[ ! -f "${FR_SRC}" ]]; then
    echo "fr_math not found at ${FR_SRC}; run 'make compare-deps' on the host for FR columns."
fi
echo ""

build_target "RP2040 (Cortex-M0+)" 32 rp2040 arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb

build_target "Pico 2 ARM-M33 (hard-float)" 32 rp2350_m33_hf arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16

build_target "STM32 (Cortex-M4, soft-float)" 32 stm32_cm4_soft arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

build_target "Cortex-M0 (Thumb-1)" 32 cm0 arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb

build_target "RISC-V 32 (rv32im)" 32 rv32im riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32

build_target "RISC-V 32 (rv32imafc)" 32 rv32imafc riscv64-unknown-elf-gcc -march=rv32imafc -mabi=ilp32f

build_target "ESP32/S3 (Xtensa esp-elf)" 32 esp32s3_xtensa xtensa-esp-elf-gcc

build_target "ESP8266 (Xtensa LX106)" 32 esp8266_xtensa xtensa-lx106-elf-gcc

build_target "AArch64 (linux-gnu)" 64 aarch64 aarch64-linux-gnu-gcc

build_target "MIPS (mipsel linux-gnu)" 32 mipsel mipsel-linux-gnu-gcc

build_target "PowerPC (linux-gnu)" 32 ppc powerpc-linux-gnu-gcc

build_target "68k (linux-gnu)" 32 m68k m68k-linux-gnu-gcc

build_target "x86-32" 32 x86_32 gcc -m32

build_target "x86-64" 64 x86_64 gcc -m64

# ── CSV ───────────────────────────────────────────────────────────────

echo "target,width_bits,qf_lean_text,qf_lean_data,qf_lean_bss,qf_lean_total,qf_full_text,qf_full_data,qf_full_bss,qf_full_total,fr_core_text,fr_core_data,fr_core_bss,fr_core_total,fr_full_text,fr_full_data,fr_full_bss,fr_full_total" >"${CSV}"

csv_parts() {
    local val="$1"
    if [[ "${val}" =~ ^[0-9]+:[0-9]+:[0-9]+:[0-9]+$ ]]; then
        echo "${val//:/,}"
    else
        echo "${val},,,"
    fi
}

csv_cell() {
    local val="$1"
    val="${val//\"/\"\"}"
    echo "\"${val}\""
}

for i in "${!TARGET_NAMES[@]}"; do
    echo "$(csv_cell "${TARGET_NAMES[$i]}"),${TARGET_WIDTHS[$i]},$(csv_parts "${QF_LEAN[$i]}"),$(csv_parts "${QF_FULL[$i]}"),$(csv_parts "${FR_CORE[$i]}"),$(csv_parts "${FR_FULL[$i]}")" >>"${CSV}"
done
echo "Wrote ${CSV}"
echo ""

total_of() {
    local val="$1"
    if [[ "${val}" =~ ^[0-9]+:[0-9]+:[0-9]+:[0-9]+$ ]]; then
        echo "${val##*:}"
    else
        echo "${val}"
    fi
}

fmt_size() {
    local val
    val="$(total_of "$1")"
    if [[ "${val}" == "n/a" || "${val}" == "fail" || "${val}" == "compiles" ]]; then
        echo "${val}"
    else
        awk -v v="${val}" 'BEGIN { printf "%.1f KB (%d B)", v/1024.0, v }'
    fi
}

echo "Summary (object totals; full matrix is ${CSV}):"
echo ""
echo "| Target | qf lean | qf full | fr core |"
echo "|--------|--------:|--------:|--------:|"

for wanted in \
    "Pico 2 ARM-M33 (hard-float)" \
    "RP2040 (Cortex-M0+)" \
    "ESP32/S3 (Xtensa esp-elf)" \
    "RISC-V 32 (rv32im)" \
    "x86-64"; do
    for i in "${!TARGET_NAMES[@]}"; do
        if [[ "${TARGET_NAMES[$i]}" == "${wanted}" ]]; then
            echo "| ${TARGET_NAMES[$i]} | $(fmt_size "${QF_LEAN[$i]}") | $(fmt_size "${QF_FULL[$i]}") | $(fmt_size "${FR_CORE[$i]}") |"
        fi
    done
done

# ── Cortex-M0 optimization sweep ────────────────────────────────────

echo ""
echo "### Optimization flags (Cortex-M0, arm-none-eabi-gcc)"
echo ""
echo "| Flags | qf lean | qf full |"
echo "|-------|--------:|--------:|"

for opt in O0 Os O2 O3; do
    lean="$(compile_one "qf_math_cm0_${opt}_lean" "${QF_SRC}" "${QF_INC}" arm-none-eabi-gcc -DQF_MATH_LEAN -mcpu=cortex-m0 -mthumb -"${opt}")"
    full="$(compile_one "qf_math_cm0_${opt}_full" "${QF_SRC}" "${QF_INC}" arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -"${opt}")"
    echo "| -${opt} | $(fmt_size "${lean}") | $(fmt_size "${full}") |"
done

echo ""
echo "CSV: ${CSV}"
