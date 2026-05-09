#!/usr/bin/env python3
"""
Merge benchmark matrices from compare/BENCHMARK_REPORT.md (host) and
compare/MCU_BENCHMARK_SNAPSHOT_ESP32S3.md (ESP32-S3 MCU) →
compare/BENCHMARK_CROSSPLATFORM.md

Output uses the same wide single-table style as MCU_BENCHMARK_SNAPSHOT_ESP32S3.md:
one matrix per Accuracy, Wall-clock, Speed vs libm — each with paired
Host | ESP32-S3 columns per library.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HOST_MD = ROOT / "compare" / "BENCHMARK_REPORT.md"
MCU_MD = ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT_ESP32S3.md"
OUT_MD = ROOT / "compare" / "BENCHMARK_CROSSPLATFORM.md"
MCU_LABEL = "ESP32-S3"

# Column order in source Markdown tables (must match bench_emit_markdown_tables).
LIB_ORDER_KEYS = [
    "libm",
    "qf_math",
    "libfixmath",
    "fr_math",
    "FastTrig",
    "ESP-DSP",
    "espp/math",
]

LIB_SHORT = {
    "libm": "libm",
    "qf_math": "qf_math",
    "libfixmath": "libfixmath",
    "fr_math": "fr_math",
    "FastTrig": "FastTrig",
    "ESP-DSP": "ESP-DSP",
    "espp/math": "espp/math",
}


def normalize_lib_header(header: str) -> str | None:
    k = header.lower().strip()
    k = re.sub(r"[`*]", "", k)
    if k == "libm":
        return "libm"
    if "qf_math" in k or k.startswith("qf"):
        return "qf_math"
    if "libfixmath" in k:
        return "libfixmath"
    if "fr_math" in k or "fr math" in k:
        return "fr_math"
    if "fasttrig" in k:
        return "FastTrig"
    if "esp-dsp" in k:
        return "ESP-DSP"
    if "espp" in k:
        return "espp/math"
    return None


def find_section_line(lines: list[str], title_substr: str) -> int | None:
    for i, line in enumerate(lines):
        if line.startswith("### ") and title_substr in line:
            return i
    return None


def parse_function_table(
    lines: list[str], start_idx: int, has_metric: bool, decimals: int = 6
) -> tuple[list[str], dict[str, dict[str, str]], dict[str, str] | None]:
    """
    Parse a markdown table starting at the first '| Function |' after start_idx.
    Returns (function_names_in_order, lib_to_fn_to_cell, metrics_by_fn or None).
    """
    i = start_idx
    while i < len(lines) and not lines[i].strip().startswith("| Function |"):
        i += 1
    if i >= len(lines):
        raise ValueError("No | Function | table found")

    header = [p.strip() for p in lines[i].split("|")[1:-1]]
    if has_metric:
        if len(header) < 3 or not header[1].lower().startswith("metric"):
            raise ValueError("Expected Function | Metric | ...")
        lib_headers = header[2:]
    else:
        lib_headers = header[1:]

    lib_keys: list[str | None] = [normalize_lib_header(h) for h in lib_headers]

    data: dict[str, dict[str, str]] = {k: {} for k in LIB_ORDER_KEYS}
    metrics: dict[str, str] | None = {} if has_metric else None
    order: list[str] = []

    for line in lines[i + 2 :]:
        line = line.strip()
        if not line.startswith("|"):
            break
        parts = [p.strip() for p in line.split("|")[1:-1]]
        if len(parts) < 2:
            continue
        fn = parts[0].replace("`", "").strip()
        order.append(fn)
        p = 1
        if has_metric and metrics is not None:
            metrics[fn] = parts[1]
            p = 2
        vals = parts[p:]
        for j, key in enumerate(lib_keys):
            if j >= len(vals) or key is None:
                continue
            if key in data:
                data[key][fn] = format_cell(vals[j], decimals)
    return order, data, metrics


def format_cell(s: str, decimals: int = 6) -> str:
    t = re.sub(r"[*`]", "", s.strip())
    if t in {"—", "---", ""}:
        return "---"
    try:
        return f"{float(t):.{decimals}f}"
    except ValueError:
        return t


def extract_tables(md: str, section_substr: str, has_metric: bool, decimals: int = 6):
    lines = md.splitlines()
    sec = find_section_line(lines, section_substr)
    if sec is None:
        return None
    return parse_function_table(lines, sec, has_metric, decimals)


def extract_markdown_section(md: str, title_substr: str) -> list[str]:
    lines = md.splitlines()
    sec = find_section_line(lines, title_substr)
    if sec is None:
        return []

    out: list[str] = []
    for line in lines[sec:]:
        if line == "---" and out:
            break
        out.append(line)
    while out and out[-1] == "":
        out.pop()
    return out


def merge_orders(
    a: list[str] | None, b: list[str] | None
) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for src in (a or [], b or []):
        for f in src:
            if f not in seen:
                seen.add(f)
                out.append(f)
    return out


def emit_unified_table(
    title: str,
    footnote: str | None,
    fn_order: list[str],
    host_d: dict[str, dict[str, str]],
    mcu_d: dict[str, dict[str, str]],
    metrics: dict[str, str] | None,
) -> list[str]:
    out: list[str] = []
    out.append(f"### {title}")
    out.append("")
    if footnote:
        out.append(footnote)
        out.append("")
    hdr = ["Function"]
    if metrics is not None:
        hdr.append("Metric")
    for k in LIB_ORDER_KEYS:
        label = LIB_SHORT[k]
        hdr.append(f"{label} (host)")
        hdr.append(f"{label} ({MCU_LABEL})")
    out.append("| " + " | ".join(hdr) + " |")
    sep_parts = []
    for idx, _h in enumerate(hdr):
        if idx == 0:
            sep_parts.append(":---")  # Function
        elif idx == 1 and metrics is not None:
            sep_parts.append(":---")  # Metric
        else:
            sep_parts.append("---:")
    out.append("| " + " | ".join(sep_parts) + " |")
    for fn in fn_order:
        row = [f"`{fn}`"]
        if metrics is not None:
            row.append(metrics.get(fn, "—"))
        for k in LIB_ORDER_KEYS:
            hc = host_d.get(k, {}).get(fn, "---")
            mc = mcu_d.get(k, {}).get(fn, "---")
            row.append(hc)
            row.append(mc)
        out.append("| " + " | ".join(row) + " |")
    out.append("")
    return out


def main() -> int:
    if not HOST_MD.is_file():
        print(f"missing {HOST_MD}", file=sys.stderr)
        return 1
    if not MCU_MD.is_file():
        print(f"missing {MCU_MD}", file=sys.stderr)
        return 1

    host_txt = HOST_MD.read_text(encoding="utf-8")
    mcu_txt = MCU_MD.read_text(encoding="utf-8")

    host_meta = ""
    m = re.search(r"\| UTC time \| ([^|]+) \|", host_txt)
    if m:
        host_meta = m.group(1).strip()

    mcu_gen = ""
    m = re.search(r"_Generated:_ \*\*([^*]+)\*\*", mcu_txt)
    if m:
        mcu_gen = m.group(1).strip()

    device = "MCU (see snapshot)"
    m = re.search(r"\| Device \| ([^|]+) \|", mcu_txt)
    if m:
        device = m.group(1).strip()

    host_acc = extract_tables(host_txt, "### Accuracy", True)
    mcu_acc = extract_tables(mcu_txt, "### Accuracy", True)
    host_mse = extract_tables(host_txt, "### Accuracy — mean squared error", True)
    mcu_mse = extract_tables(mcu_txt, "### Accuracy — mean squared error", True)
    host_wall = extract_tables(host_txt, "### Wall-clock", False)
    mcu_wall = extract_tables(mcu_txt, "### Wall-clock", False)
    host_spd = extract_tables(host_txt, "### Speed vs libm", False, 2)
    mcu_spd = extract_tables(mcu_txt, "### Speed vs libm", False, 2)
    footprint = extract_markdown_section(host_txt, "### Library footprint")

    if not host_spd or not mcu_spd:
        print("Could not parse Speed vs libm tables", file=sys.stderr)
        return 1

    _, host_spd_data, _ = host_spd
    _, mcu_spd_data, _ = mcu_spd
    fn_speed = merge_orders(host_spd[0], mcu_spd[0])

    buf: list[str] = []
    buf.append("# Benchmark — host vs ESP32-S3 (combined tables)")
    buf.append("")
    buf.append(
        "Side-by-side **POSIX host** ([`BENCHMARK_REPORT.md`](BENCHMARK_REPORT.md)) and "
        "**ESP32-S3 MCU** ([`MCU_BENCHMARK_SNAPSHOT_ESP32S3.md`](MCU_BENCHMARK_SNAPSHOT_ESP32S3.md)) for the same "
        "[`benchmark_core.c`](benchmark_core.c) loops. "
        "Each library has **two columns**: **(host)** then **(ESP32-S3)**."
    )
    buf.append("")
    buf.append("## Sources & timestamps")
    buf.append("")
    buf.append("| | |")
    buf.append("|-|-|")
    buf.append(f"| **Host** | UTC **`{host_meta}`** — `make compare-github-report` |")
    buf.append(f"| **ESP32-S3** | **`{mcu_gen}`** · {device} — `make mcu-benchmark-snapshot` |")
    buf.append("")
    buf.append(
        "> **Do not compare absolute microseconds** across host vs ESP32-S3. "
        "**Speed vs libm** ratios (`libm` time ÷ implementation time) are comparable "
        "*within* each column; > 1.0 means faster than that platform’s `sinf`/`sqrtf`/… for that loop."
    )
    buf.append("")
    buf.append("## Interpretation")
    buf.append("")
    buf.append(
        "- **libfixmath** / **fr_math** use **float bridges** in these tables (host and MCU harness); "
        "native `fix16_t` / radix `s32` calls are usually faster."
    )
    buf.append(
        "- **ESP32-only** peers (**FastTrig**, **ESP-DSP**, **espp/math**) have `---` in **(host)** "
        "columns when the POSIX bench does not build that implementation."
    )
    buf.append(
        "- `---` means the row was unsupported or not present in the captured ESP32-S3 snapshot. "
        "Refresh `MCU_BENCHMARK_SNAPSHOT_ESP32S3.md` after changing the shared benchmark row set."
    )
    buf.append("")
    buf.append(
        "Numeric formatting matches the snapshots (six digits after the decimal where applicable). "
        "Unsupported cells: `---`."
    )
    buf.append("")

    if host_acc and mcu_acc:
        host_order, host_acc_data, host_met = host_acc
        mcu_order, mcu_acc_data, mcu_met = mcu_acc
        fn_acc = merge_orders(host_order, mcu_order)
        met = host_met or mcu_met or {}
        buf.extend(
            emit_unified_table(
                "Accuracy",
                "Same metric meanings as the snapshots: `abs %FS`, `abs`, `abs rad`, `rel %` as in the `Metric` column.",
                fn_acc,
                host_acc_data,
                mcu_acc_data,
                met,
            )
        )

    if host_mse and mcu_mse:
        host_order, host_mse_data, host_met = host_mse
        mcu_order, mcu_mse_data, mcu_met = mcu_mse
        fn_mse = merge_orders(host_order, mcu_order)
        met = host_met or mcu_met or {}
        buf.extend(
            emit_unified_table(
                "Accuracy — mean squared error",
                "MSE uses the same metric units as the peak-error table, squared.",
                fn_mse,
                host_mse_data,
                mcu_mse_data,
                met,
            )
        )

    if host_wall and mcu_wall:
        host_order, host_w_data, _ = host_wall
        mcu_order, mcu_w_data, _ = mcu_wall
        fn_w = merge_orders(host_order, mcu_order)
        buf.extend(
            emit_unified_table(
                "Wall-clock time (microseconds)",
                "Total microseconds for the benchmark loop on each platform (normalized loop shape per snapshot metadata).",
                fn_w,
                host_w_data,
                mcu_w_data,
                None,
            )
        )

    buf.extend(
        emit_unified_table(
            "Speed vs libm (ratio)",
            "`libm` time ÷ implementation time on that platform. **> 1.0** = faster than platform libm for that timed loop. Ratios are rounded to two decimal places.",
            fn_speed,
            host_spd_data,
            mcu_spd_data,
            None,
        )
    )

    if footprint:
        buf.append("## Compiled Code Size")
        buf.append("")
        buf.append(
            "These object-size rows come from the host comparison build plus any ESP32 peer objects "
            "available from the LilyGO PlatformIO build. They are library/variant footprints, not "
            "firmware totals."
        )
        buf.append("")
        buf.extend(footprint)
        buf.append("")

    buf.append("---")
    buf.append("")
    buf.append(
        "<!-- Generated by tools/gen_benchmark_crossplatform.py — run: make benchmark-crossplatform -->"
    )

    OUT_MD.write_text("\n".join(buf) + "\n", encoding="utf-8")
    print(f"wrote {OUT_MD}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
