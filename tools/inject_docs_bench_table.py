#!/usr/bin/env python3
"""Replace <!--QF_MATH_BENCH_TABLE_BEGIN--> … END in docs/pages/index.html with benchmark output."""

from __future__ import annotations

import pathlib
import re
import subprocess
import sys


def main() -> None:
    root = pathlib.Path(__file__).resolve().parent.parent
    bench = root / "build" / "qf_math_bench"
    page = root / "docs" / "pages" / "index.html"
    begin = "<!--QF_MATH_BENCH_TABLE_BEGIN-->"
    end = "<!--QF_MATH_BENCH_TABLE_END-->"

    if not bench.is_file():
        print("missing build/qf_math_bench — run: make docs-pages-deps (or make bench)", file=sys.stderr)
        sys.exit(1)
    frag = subprocess.check_output([str(bench), "--html-table"], text=True).strip()

    text = page.read_text(encoding="utf-8")
    if begin not in text or end not in text:
        print(f"{begin=} / {end=} not found in {page}", file=sys.stderr)
        sys.exit(1)

    pat = re.compile(re.escape(begin) + r".*?" + re.escape(end), re.DOTALL)
    block = pat.search(text)
    if not block:
        print(f"markers not paired in {page}", file=sys.stderr)
        sys.exit(1)

    replacement = begin + "\n" + frag + "\n" + end
    new_text = pat.sub(replacement, text, count=1)
    page.write_text(new_text, encoding="utf-8")
    print(f"Injected sweep table → {page}")


if __name__ == "__main__":
    main()
