#!/usr/bin/env python3
"""Generate speed-vs-libm matrices across architectures (qf_math & fr_math bridge)."""

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

IMPL_COLUMNS = ("qf_math", "fr_math")
# Matches snapshot **`libm`** column (`libm` time ÷ `libm`); repeated for readability in arch matrices.
LIBM_RATIO_BASELINE = "1.00"


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


def emit_matrix(
    out: list[str],
    *,
    title: str,
    ratio_intro: str,
    order: list[str],
    sources_with_cols: list[tuple[str, dict[str, str]]],
) -> None:
    out.append(title)
    out.append("")
    out.append(ratio_intro)
    out.append("")
    cols = ["Function", "**libm**"] + [label for label, _ in sources_with_cols]
    out.append("| " + " | ".join(cols) + " |")
    out.append("| :--- | ---: |" + " ---: |" * (len(cols) - 2))
    for fn in order:
        row = [f"`{fn}`", LIBM_RATIO_BASELINE] + [
            cols_i[1].get(fn, "---") for cols_i in sources_with_cols
        ]
        out.append("| " + " | ".join(row) + " |")
    out.append("")


def main() -> int:
    sources: list[tuple[str, Path]] = [(host_column_label(), _HOST_REPORT)] + SOURCES_STATIC
    missing = [str(path) for _label, path in sources if not path.is_file()]
    if missing:
        print("missing benchmark snapshots:\n  " + "\n  ".join(missing), file=sys.stderr)
        return 1

    parsed: list[tuple[str, Path, dict[str, tuple[list[str], dict[str, str]]]]] = []
    for label, path in sources:
        per_impl = {impl: parse_impl_speed_ratios(path, impl) for impl in IMPL_COLUMNS}
        parsed.append((label, path, per_impl))

    order: list[str] = []
    seen: set[str] = set()
    for _label, _path, per_impl in parsed:
        for fn in per_impl["qf_math"][0]:
            if fn not in seen:
                seen.add(fn)
                order.append(fn)
    for _label, _path, per_impl in parsed:
        for fn in per_impl["fr_math"][0]:
            if fn not in seen:
                seen.add(fn)
                order.append(fn)

    out: list[str] = []
    out.append("# Speed vs libm by Architecture (`qf_math` & `fr_math`)")
    out.append("")
    out.append(
        "Below: two matrices from **`### Speed vs libm`** in each snapshot (**`libm`** time ÷ implementation time)."
        " **> 1** → that implementation beats `libm` in this microbenchmark; **< 1** → `libm` wins."
    )
    out.append("")
    out.append(
        "First table: **`qf_math`**. Second: **`fr_math` (float bridge)** — bridged **`float→s32→…→float`** harness timings, "
        "not raw fixed-only throughput."
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
    for label, path, _per in parsed:
        rel = path.relative_to(OUT_MD.parent)
        out.append(f"| {label} | [{rel.name}]({rel}) · {read_generated(path)} |")
    out.append("")

    emit_matrix(
        out,
        title="## `qf_math`",
        ratio_intro=(
            "**Speed ratio** = `libm` time / **`qf_math`** time (same snapshot). "
            "**< 1** → `libm` faster than `qf_math` in this microbenchmark."
        ),
        order=order,
        sources_with_cols=[(label, per_impl["qf_math"][1]) for label, _path, per_impl in parsed],
    )
    emit_matrix(
        out,
        title="## `fr_math` (float bridge)",
        ratio_intro=(
            "**Speed ratio** = `libm` time / **`fr_math` bridged harness** time. Numbers match the "
            "**`fr_math` (float bridge)** columns in snapshots; compare to **`qf_math`** above on the "
            "same ISA for float vs bridged-fixed cost."
        ),
        order=order,
        sources_with_cols=[
            (label, per_impl["fr_math"][1]) for label, _path, per_impl in parsed
        ],
    )

    OUT_MD.write_text("\n".join(out), encoding="utf-8")
    print(f"wrote {OUT_MD}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
