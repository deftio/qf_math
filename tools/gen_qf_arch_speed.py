#!/usr/bin/env python3
"""Generate qf_math speed-vs-libm matrix across architectures."""

from __future__ import annotations

import platform
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_MD = ROOT / "compare" / "QF_MATH_ARCH_SPEED.md"

# Host snapshot path is fixed. Column title is parsed from § Host metadata inside
# BENCHMARK_REPORT.md so it matches the timed machine (not the CI worker that runs this script).
_HOST_REPORT = ROOT / "compare" / "BENCHMARK_REPORT.md"

SOURCES_STATIC = [
    ("ESP32-S3", ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT_ESP32S3.md"),
    ("Pico 2 ARM-M33", ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT_PICO2_ARM.md"),
    ("Pico 2 Hazard3 RISC-V", ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT_PICO2_RISCV.md"),
]

# Matches snapshot **`libm`** column (`libm` time ÷ `libm`); repeated for readability in arch matrices.
LIBM_RATIO_BASELINE = "1.00"


def source_link_label(label: str, path: Path) -> str:
    """Readable link text without underscores, which some Markdown renderers over-parse."""
    if path == _HOST_REPORT:
        return "BENCHMARK_REPORT.md"
    compact = path.stem
    compact = compact.removeprefix("MCU_BENCHMARK_")
    compact = compact.replace("_", " ")
    return f"{compact}.md"


def host_column_label_runtime_fallback() -> str:
    """Used only when BENCHMARK_REPORT has no Host metadata table."""
    sys_n = platform.system()
    mach = platform.machine()
    return f"Host bench · {sys_n} {mach} (runner machine, metadata missing)"


def host_column_label_from_report(report: Path) -> str:
    """
    Prefer uname/compiler from ### Host metadata in BENCHMARK_REPORT.md so labels
    match the checked-in timings, independent of where gen_qf_arch_speed.py runs.
    """
    lines = report.read_text(encoding="utf-8").splitlines()
    meta: dict[str, str] = {}
    in_meta = False
    for line in lines:
        stripped = line.strip()
        if stripped == "### Host metadata":
            in_meta = True
            continue
        if not in_meta:
            continue
        if stripped.startswith("###") and stripped != "### Host metadata":
            break
        if stripped.startswith("|") and "---" not in stripped:
            parts = [p.strip() for p in line.split("|")[1:-1]]
            if len(parts) >= 2 and parts[0] and parts[0] != "Field":
                meta[parts[0]] = parts[1]

    um = meta.get("uname", "").strip()
    comp = meta.get("Compiler", "").strip()
    if not um:
        return host_column_label_runtime_fallback()

    toks = um.split()
    short_plat = f"{toks[0]} {toks[1]}" if len(toks) >= 2 else um

    libm_hint = (
        "Apple libm"
        if ("Apple" in comp or "apple" in comp.lower())
        else "system libm"
    )

    # Narrow header for GitHub tables: Darwin arm64 … is enough to disambiguate
    return f"Host bench · {short_plat} ({libm_hint})"


def host_column_label() -> str:
    """Label for column backed by BENCHMARK_REPORT.md."""
    if not _HOST_REPORT.is_file():
        return host_column_label_runtime_fallback()
    return host_column_label_from_report(_HOST_REPORT)


def find_speed_table(lines: list[str]) -> int:
    for i, line in enumerate(lines):
        if line.startswith("### Speed vs libm"):
            return i
    raise ValueError("missing Speed vs libm section")


def clean_cell(cell: str) -> str:
    return re.sub(r"[*`]", "", cell.strip())


def col_index_impl(header_cells: list[str], impl_prefix: str) -> int:
    needle = impl_prefix.lower()
    for i, raw in enumerate(header_cells):
        if clean_cell(raw).lower().startswith(needle):
            return i
    raise ValueError(f"missing {impl_prefix} column (header cells: {header_cells!r})")


def parse_impl_speed_ratios(path: Path, impl_prefix: str) -> tuple[list[str], dict[str, str]]:
    """Return per-function ratios from Speed vs libm table (libm column is 1.00)."""
    lines = path.read_text(encoding="utf-8").splitlines()
    sec = find_speed_table(lines)
    i = sec
    while i < len(lines) and not lines[i].strip().startswith("| Function |"):
        i += 1
    if i >= len(lines):
        raise ValueError(f"missing speed table in {path}")

    header_raw = [p.strip() for p in lines[i].split("|")[1:-1]]
    impl_col = col_index_impl(header_raw, impl_prefix)

    order: list[str] = []
    speeds: dict[str, str] = {}
    for line in lines[i + 2 :]:
        line = line.strip()
        if not line.startswith("|"):
            break
        parts = [clean_cell(p) for p in line.split("|")[1:-1]]
        if len(parts) <= impl_col:
            continue
        fn = parts[0]
        val = parts[impl_col]
        order.append(fn)
        speeds[fn] = "---" if val in {"", "---", "—"} else f"{float(val):.2f}"
    return order, speeds


def read_generated(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    m = re.search(r"_Generated:_ \*\*([^*]+)\*\*", text)
    if m:
        return m.group(1).strip()
    m = re.search(r"\| UTC time \| ([^|]+) \|", text)
    if m:
        return m.group(1).strip()
    return "---"


def main() -> int:
    sources: list[tuple[str, Path]] = [(host_column_label(), _HOST_REPORT)] + SOURCES_STATIC
    missing = [str(path) for _label, path in sources if not path.is_file()]
    if missing:
        print("missing benchmark snapshots:\n  " + "\n  ".join(missing), file=sys.stderr)
        return 1

    parsed: list[tuple[str, Path, tuple[list[str], dict[str, str]], tuple[list[str], dict[str, str]] | None]] = []
    for label, path in sources:
        qf = parse_impl_speed_ratios(path, "qf_math")
        fr = None if path == _HOST_REPORT else parse_impl_speed_ratios(path, "fr_math")
        parsed.append((label, path, qf, fr))

    order: list[str] = []
    seen: set[str] = set()
    for _label, _path, qf, _fr in parsed:
        for fn in qf[0]:
            if fn not in seen:
                seen.add(fn)
                order.append(fn)
    for _label, _path, _qf, fr in parsed:
        if fr is None:
            continue
        for fn in fr[0]:
            if fn not in seen:
                seen.add(fn)
                order.append(fn)

    out: list[str] = []
    out.append("# `qf_math` Speed vs libm by Architecture")
    out.append("")
    out.append(
        "Matrix values come from **`### Speed vs libm`** in each snapshot (**`libm`** time ÷ implementation time)."
        " **> 1** → that implementation beats `libm` in this microbenchmark; **< 1** → `libm` wins."
    )
    out.append("")
    out.append(
        "The main architecture columns are **`qf_math`**. The three **`fr_math`** MCU columns marked **(cast to float)** "
        "use the same harness with `float` inputs/outputs around the fixed-point core (**`float→s32→…→float`**); "
        "they are not raw fixed-only throughput."
    )
    out.append("")
    out.append(
        "Host column titles are **`uname`/`Compiler`** from **`BENCHMARK_REPORT.md` § Host metadata** "
        "(not the machine that runs `make benchmark-arch-speed`)."
    )
    out.append("")
    out.append(
        "**`libm`** column (**1.00**) is the denominator baseline (`libm` ÷ `libm`); snapshots use the same first column."
    )
    out.append("")
    out.append("`---` → that snapshot has no ratio for that function row.")
    out.append("")
    out.append("| Source | Generated |")
    out.append("| :--- | :--- |")
    for label, path, _qf, _fr in parsed:
        rel = path.relative_to(OUT_MD.parent)
        out.append(f"| {label} | [{source_link_label(label, path)}]({rel}) · {read_generated(path)} |")
    out.append("")
    out.append("| Function | **libm** | Host `qf_math` | ESP32-S3 `qf_math` | ESP32-S3 `fr_math` (cast to float) | Pico ARM `qf_math` | Pico ARM `fr_math` (cast to float) | Pico RISC-V `qf_math` | Pico RISC-V `fr_math` (cast to float) |")
    out.append("| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |")

    host = parsed[0][2][1]
    esp_qf = parsed[1][2][1]
    esp_fr = parsed[1][3][1] if parsed[1][3] is not None else {}
    arm_qf = parsed[2][2][1]
    arm_fr = parsed[2][3][1] if parsed[2][3] is not None else {}
    rv_qf = parsed[3][2][1]
    rv_fr = parsed[3][3][1] if parsed[3][3] is not None else {}

    for fn in order:
        row = [
            f"`{fn}`",
            LIBM_RATIO_BASELINE,
            host.get(fn, "---"),
            esp_qf.get(fn, "---"),
            esp_fr.get(fn, "---"),
            arm_qf.get(fn, "---"),
            arm_fr.get(fn, "---"),
            rv_qf.get(fn, "---"),
            rv_fr.get(fn, "---"),
        ]
        out.append("| " + " | ".join(row) + " |")
    out.append("")

    OUT_MD.write_text("\n".join(out), encoding="utf-8")
    print(f"wrote {OUT_MD}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
