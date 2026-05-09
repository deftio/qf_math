#!/usr/bin/env python3
"""Generate qf_math speed-vs-libm matrix across captured architectures."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_MD = ROOT / "compare" / "QF_MATH_ARCH_SPEED.md"

SOURCES = [
    ("POSIX host", ROOT / "compare" / "BENCHMARK_REPORT.md"),
    ("ESP32-S3", ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT_ESP32S3.md"),
    ("Pico 2 ARM-M33", ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT_PICO2_ARM.md"),
    ("Pico 2 Hazard3 RISC-V", ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT_PICO2_RISCV.md"),
]


def find_speed_table(lines: list[str]) -> int:
    for i, line in enumerate(lines):
        if line.startswith("### Speed vs libm"):
            return i
    raise ValueError("missing Speed vs libm section")


def clean_cell(cell: str) -> str:
    return re.sub(r"[*`]", "", cell.strip())


def parse_qf_speed(path: Path) -> tuple[list[str], dict[str, str]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    sec = find_speed_table(lines)
    i = sec
    while i < len(lines) and not lines[i].strip().startswith("| Function |"):
        i += 1
    if i >= len(lines):
        raise ValueError(f"missing speed table in {path}")

    header = [clean_cell(p) for p in lines[i].split("|")[1:-1]]
    try:
        qf_col = header.index("qf_math")
    except ValueError as exc:
        raise ValueError(f"missing qf_math column in {path}") from exc

    order: list[str] = []
    speeds: dict[str, str] = {}
    for line in lines[i + 2 :]:
        line = line.strip()
        if not line.startswith("|"):
            break
        parts = [clean_cell(p) for p in line.split("|")[1:-1]]
        if len(parts) <= qf_col:
            continue
        fn = parts[0]
        val = parts[qf_col]
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
    parsed: list[tuple[str, Path, list[str], dict[str, str]]] = []
    missing = [str(path) for _label, path in SOURCES if not path.is_file()]
    if missing:
        print("missing benchmark snapshots:\n  " + "\n  ".join(missing), file=sys.stderr)
        return 1

    order: list[str] = []
    seen: set[str] = set()
    for label, path in SOURCES:
        fn_order, speeds = parse_qf_speed(path)
        parsed.append((label, path, fn_order, speeds))
        for fn in fn_order:
            if fn not in seen:
                seen.add(fn)
                order.append(fn)

    out: list[str] = []
    out.append("# qf_math Speed vs libm by Architecture")
    out.append("")
    out.append(
        "This document is generated from the benchmark reports and shows only "
        "`qf_math` speed ratios. Values are `libm time / qf_math time`; values "
        "above `1.00` mean `qf_math` was faster than that platform's `libm`."
    )
    out.append("")
    out.append(
        "`---` means the source snapshot did not contain that function row. "
        "Regenerate the corresponding MCU snapshot after changing the shared benchmark matrix."
    )
    out.append("")
    out.append("| Source | Generated |")
    out.append("| :--- | :--- |")
    for label, path, _fn_order, _speeds in parsed:
        rel = path.relative_to(OUT_MD.parent)
        out.append(f"| {label} | [{rel.name}]({rel}) · {read_generated(path)} |")
    out.append("")

    headers = ["Function"] + [label for label, _path in SOURCES]
    out.append("| " + " | ".join(headers) + " |")
    out.append("| :--- |" + " ---: |" * (len(headers) - 1))
    for fn in order:
        row = [f"`{fn}`"]
        for _label, _path, _fn_order, speeds in parsed:
            row.append(speeds.get(fn, "---"))
        out.append("| " + " | ".join(row) + " |")
    out.append("")

    OUT_MD.write_text("\n".join(out), encoding="utf-8")
    print(f"wrote {OUT_MD}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
