#!/usr/bin/env python3
"""
Merge “Speed vs libm” ratio tables from compare/BENCHMARK_REPORT.md and
compare/MCU_BENCHMARK_SNAPSHOT.md → compare/BENCHMARK_CROSSPLATFORM.md
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HOST_MD = ROOT / "compare" / "BENCHMARK_REPORT.md"
MCU_MD = ROOT / "compare" / "MCU_BENCHMARK_SNAPSHOT.md"
OUT_MD = ROOT / "compare" / "BENCHMARK_CROSSPLATFORM.md"

FUNCS = ["sin", "cos", "sqrt", "ln", "exp"]
LIBS_DISPLAY = [
    ("qf_math", "**qf_math**"),
    ("libfixmath", "**libfixmath** (float bridge)"),
    ("fr_math", "**fr_math** (float bridge)"),
]


def extract_host_meta(md: str) -> str:
    m = re.search(r"\| UTC time \| ([^|]+) \|", md)
    if m:
        return m.group(1).strip()
    return "—"


def extract_mcu_generated(md: str) -> str:
    m = re.search(r"_Generated:_ \*\*([^*]+)\*\*", md)
    if m:
        return m.group(1).strip()
    return "—"


def extract_device_line(md: str) -> str:
    m = re.search(r"\| Device \| ([^|]+) \|", md)
    if m:
        return m.group(1).strip()
    return "ESP32-class MCU (see MCU snapshot)"


def extract_speed_ratio_table(md: str) -> dict[str, list[str]]:
    """Rows from the portable Speed vs libm section only (not wall-clock us or extra MCU peers)."""
    lines = md.splitlines()
    sec = None
    for i, line in enumerate(lines):
        if "### Speed vs libm" in line:
            sec = i
            break
    if sec is None:
        raise ValueError("### Speed vs libm section not found")

    start = None
    for i in range(sec, len(lines)):
        if lines[i].strip().startswith("| Library | sin |"):
            start = i
            break
    if start is None:
        raise ValueError("| Library | sin | ratio table not found after Speed vs libm")

    rows: dict[str, list[str]] = {}
    for line in lines[start + 2 :]:
        line = line.strip()
        if not line.startswith("|"):
            break
        parts = [p.strip() for p in line.split("|")[1:-1]]
        if len(parts) < 6:
            continue
        key_raw = parts[0]
        key = re.sub(r"\*+", "", key_raw).strip()
        key = key.replace("`", "").strip()
        if key.startswith("libm"):
            continue
        rows[key] = parts[1:6]
    return rows


def normalize_key(key: str) -> str:
    k = key.lower()
    if "qf_math" in k or k == "qf_math":
        return "qf_math"
    if "libfixmath" in k:
        return "libfixmath"
    if "fr_math" in k or "fr math" in k:
        return "fr_math"
    if "taylor" in k:
        return "Taylor poly"
    return key


def normalize_table(raw: dict[str, list[str]]) -> dict[str, list[str]]:
    out: dict[str, list[str]] = {}
    for k, v in raw.items():
        nk = normalize_key(k)
        out[nk] = [format_fixed6_cell(cell) for cell in v]
    return out


def format_fixed6_cell(cell: str) -> str:
    if cell.strip() == "—":
        return "—"
    try:
        value = float(cell)
    except ValueError:
        return cell
    return f"{value:.6f}"


def main() -> int:
    if not HOST_MD.is_file():
        print(f"missing {HOST_MD}", file=sys.stderr)
        return 1
    if not MCU_MD.is_file():
        print(f"missing {MCU_MD}", file=sys.stderr)
        return 1

    host_txt = HOST_MD.read_text(encoding="utf-8")
    mcu_txt = MCU_MD.read_text(encoding="utf-8")

    host_meta = extract_host_meta(host_txt)
    mcu_gen = extract_mcu_generated(mcu_txt)
    device = extract_device_line(mcu_txt)

    host_tbl = normalize_table(extract_speed_ratio_table(host_txt))
    mcu_tbl = normalize_table(extract_speed_ratio_table(mcu_txt))

    buf = []
    buf.append("# Benchmark — host vs MCU (relative)")
    buf.append("")
    buf.append(
        "Side‑by‑side **`libm` ÷ implementation** ratios from the same benchmark loops "
        "([`benchmark_core.c`](benchmark_core.c)). "
        "**Values above 1.0** mean that row beat **`sinf`/`cosf`/…** on that platform "
        "for the timed loop; **below 1.0** means slower than libm."
    )
    buf.append("")
    buf.append("## Where the numbers live")
    buf.append("")
    buf.append("| Platform | Role | Source file | Regenerate |")
    buf.append("|----------|------|-------------|------------|")
    buf.append(
        "| **POSIX host** | Apple **Darwin** arm64, desktop libm reference | "
        "[`BENCHMARK_REPORT.md`](BENCHMARK_REPORT.md) | **`make compare-github-report`** |"
    )
    buf.append(
        "| **MCU** | On‑silicon (below) | [`MCU_BENCHMARK_SNAPSHOT.md`](MCU_BENCHMARK_SNAPSHOT.md) "
        "| **`make mcu-benchmark-snapshot`** (USB + `pio` + `pyserial`) |"
    )
    buf.append(
        "| **Combined relative tables** | Host vs MCU ratios only | This file | **`make benchmark-crossplatform`** "
        "(or `python3 tools/gen_benchmark_crossplatform.py`) |"
    )
    buf.append("")
    buf.append("### Snapshot timestamps (committed)")
    buf.append("")
    buf.append(f"| Host (`BENCHMARK_REPORT`) | UTC **`{host_meta}`** |")
    buf.append(f"| MCU (`MCU_BENCHMARK_SNAPSHOT`) | **`{mcu_gen}`** · {device} |")
    buf.append("")
    buf.append(
        "> **Chip note:** the checked‑in MCU row is **Espressif ESP32‑S3** "
        "(**LilyGO T‑Display‑S3**, Arduino core). "
        "**ESP32‑S2** is a different core; to add S2 numbers, port/run the same benchmark "
        "and regenerate [`MCU_BENCHMARK_SNAPSHOT.md`](MCU_BENCHMARK_SNAPSHOT.md), then re-run this script."
    )
    buf.append("")
    buf.append("## Interpretation")
    buf.append("")
    buf.append(
        "- **Do not compare absolute microseconds** across columns — host vs MCU clocks and "
        "libc builds differ wildly."
    )
    buf.append(
        "- **Ratios vs libm** are useful *within each column*: they show whether each measured "
        "row beats the platform’s **`sinf`** / **`sqrtf`** / … **on that silicon**."
    )
    buf.append(
        "- **libfixmath** / **fr_math** rows are **bridged harness timings**: "
        "`float`→fixed→function→`float`. They are included for comparison continuity, "
        "but are not native `fix16_t` / `s32` pipeline timing."
    )
    buf.append(
        "- **ESP32-S3-only peers** such as FastTrig, ESP-DSP sqrt, and espp/math are captured in "
        "[`MCU_BENCHMARK_SNAPSHOT.md`](MCU_BENCHMARK_SNAPSHOT.md). This cross-platform report "
        "only merges rows shared by the portable host and MCU harnesses."
    )
    buf.append(
        "- **Coverage is intentionally narrow today:** this file only merges the timed `sin`, `cos`, "
        "`sqrt`, `ln`, and `exp` rows. Accuracy for `tan`, inverse trig, `log2`, `log10`, `pow2`, "
        "`pow10`, `hypot`, and wave/envelope helpers still needs a separate generated coverage table."
    )
    buf.append("")
    buf.append("All ratios are shown with six digits after the decimal point.")
    buf.append("")
    buf.append("## Speed vs libm — Host | MCU")
    buf.append("")

    for lib_key, lib_title in LIBS_DISPLAY:
        buf.append(f"### {lib_title.replace('**', '')}")
        buf.append("")
        buf.append("| Function | Host ratio | MCU ratio |")
        buf.append("| :--- | ---:| ---:|")
        hr = host_tbl.get(lib_key)
        mr = mcu_tbl.get(lib_key)
        for i, fn in enumerate(FUNCS):
            hc = hr[i] if hr and i < len(hr) else "—"
            mc = mr[i] if mr and i < len(mr) else "—"
            if hc.strip() == "—" and mc.strip() == "—":
                continue
            buf.append(f"| `{fn}` | {hc} | {mc} |")
        buf.append("")

    buf.append("---")
    buf.append("")
    buf.append(
        "<!-- Generated by tools/gen_benchmark_crossplatform.py — "
        "run: make benchmark-crossplatform -->"
    )

    OUT_MD.write_text("\n".join(buf) + "\n", encoding="utf-8")
    print(f"wrote {OUT_MD}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
