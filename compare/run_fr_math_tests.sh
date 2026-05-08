#!/usr/bin/env bash
# Run fr_math's upstream `make test` against the clone under build/compare/third_party/fr_math.
# Build outputs stay inside that tree (still under build/).
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
bash "$ROOT/compare/fetch_deps.sh"
FR="$ROOT/build/compare/third_party/fr_math"
echo "Running fr_math tests (make -f makefile test) in $FR ..."
make -C "$FR" -f makefile test
