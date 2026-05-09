#!/usr/bin/env python3
"""Prepare a qf_math release by updating metadata and rebuilding generated files."""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def run(cmd: list[str], dry_run: bool) -> None:
    print("+ " + " ".join(cmd))
    if dry_run:
        return
    subprocess.run(cmd, cwd=ROOT, check=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("version", nargs="?", help="new semantic version, e.g. 1.0.2")
    parser.add_argument("--skip-docker", action="store_true", help="skip make docker-sizes")
    parser.add_argument("--skip-peer-tests", action="store_true", help="skip upstream libfixmath/fr_math tests")
    parser.add_argument("--skip-pages", action="store_true", help="skip pages/index.html benchmark injection")
    parser.add_argument("--dry-run", action="store_true", help="print commands without running them")
    args = parser.parse_args()

    version_arg = args.version or os.environ.get("VERSION", "")
    if version_arg:
        run(["python3", "tools/qf_version.py", "update", version_arg], args.dry_run)
    else:
        run(["python3", "tools/qf_version.py", "verify"], args.dry_run)

    run(["make", "clean"], args.dry_run)
    run(["make", "test"], args.dry_run)
    run(["make", "test-lean-cpp"], args.dry_run)

    if not args.skip_peer_tests:
        run(["make", "compare-tests"], args.dry_run)
        run(["make", "compare-fr-tests"], args.dry_run)

    run(["make", "compare-github-report"], args.dry_run)
    run(["make", "benchmark-crossplatform"], args.dry_run)
    run(["make", "benchmark-arch-speed"], args.dry_run)

    if not args.skip_pages:
        run(["make", "docs-pages"], args.dry_run)

    if not args.skip_docker:
        run(["make", "docker-sizes"], args.dry_run)

    run(["python3", "tools/qf_version.py", "verify"], args.dry_run)
    run(["python3", "tools/qf_version.py", "show", "--format", "markdown"], args.dry_run)

    if args.dry_run:
        print("Dry run complete.")
    else:
        print("Release preparation complete.")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)
