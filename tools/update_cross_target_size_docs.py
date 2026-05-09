#!/usr/bin/env python3
"""Update committed Markdown snippets from the Docker size CSV."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CSV = ROOT / "build" / "docker_sizes.csv"
DEFAULT_README = ROOT / "compare" / "README.md"

BEGIN = "<!-- BEGIN_DOCKER_SIZE_SUMMARY -->"
END = "<!-- END_DOCKER_SIZE_SUMMARY -->"

SUMMARY_TARGETS = (
    "Pico 2 ARM-M33 (hard-float)",
    "RP2040 (Cortex-M0+)",
    "ESP32/S3 (Xtensa esp-elf)",
    "RISC-V 32 (rv32im)",
    "x86-64",
)


def total(row: dict[str, str], prefix: str) -> str:
    return row.get(f"{prefix}_total", "").strip()


def fmt_bytes(value: str) -> str:
    if value.isdigit():
        size = int(value)
        return f"{size / 1024.0:.1f} KB ({size} B)"
    return value or "---"


def ratio(numer: str, denom: str) -> str:
    if numer.isdigit() and denom.isdigit() and int(denom) != 0:
        return f"{int(numer) / int(denom):.2f}x"
    return "---"


def load_rows(csv_path: Path) -> list[dict[str, str]]:
    if not csv_path.exists():
        raise SystemExit(
            f"{csv_path} does not exist; run 'make docker-sizes' first"
        )

    with csv_path.open(newline="") as handle:
        return list(csv.DictReader(handle))


def build_summary(rows: list[dict[str, str]]) -> str:
    by_target = {row["target"]: row for row in rows}
    selected = [by_target[name] for name in SUMMARY_TARGETS if name in by_target]

    lines = [
        BEGIN,
        "",
        "## Cross-Target Size",
        "",
        "Generated from `build/docker_sizes.csv` by `tools/update_cross_target_size_docs.py`.",
        "The full target matrix stays in the CSV; this section keeps only the main targets we discuss most often.",
        "",
        "| Target | qf lean | qf full | fr core | qf lean / fr core |",
        "|--------|--------:|--------:|--------:|------------------:|",
    ]

    for row in selected:
        qf_lean = total(row, "qf_lean")
        qf_full = total(row, "qf_full")
        fr_core = total(row, "fr_core")
        lines.append(
            f"| {row['target']} | {fmt_bytes(qf_lean)} | {fmt_bytes(qf_full)} | "
            f"{fmt_bytes(fr_core)} | {ratio(qf_lean, fr_core)} |"
        )

    lines.extend(
        [
            "",
            "`qf lean` is `-DQF_MATH_LEAN`; `fr core` is `-DFR_CORE_ONLY`. These are object-file totals only, not final firmware flash.",
            "",
            END,
        ]
    )
    return "\n".join(lines)


def replace_block(path: Path, block: str) -> None:
    text = path.read_text()
    if BEGIN not in text or END not in text:
        raise SystemExit(f"{path} is missing {BEGIN}/{END} markers")

    before, rest = text.split(BEGIN, 1)
    _, after = rest.split(END, 1)
    path.write_text(before.rstrip() + "\n\n" + block + after)


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", type=Path, default=DEFAULT_CSV)
    parser.add_argument("--readme", type=Path, default=DEFAULT_README)
    args = parser.parse_args()

    rows = load_rows(args.csv)
    replace_block(args.readme, build_summary(rows))
    print(f"Updated {display_path(args.readme)} from {display_path(args.csv)}")


if __name__ == "__main__":
    main()
