#!/usr/bin/env bash
# Clone comparison dependencies into build/compare/third_party/ (nothing vendored in git).
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/build/compare/third_party"
mkdir -p "$DEST"

if [[ ! -f "$DEST/libfixmath/libfixmath/fix16.c" ]]; then
  echo "Cloning libfixmath (MIT) → build/compare/third_party/libfixmath ..."
  rm -rf "$DEST/libfixmath"
  git clone --depth 1 https://github.com/PetteriAimonen/libfixmath.git "$DEST/libfixmath"
fi

if [[ ! -f "$DEST/fr_math/src/FR_math.c" ]]; then
  echo "Cloning fr_math (BSD-2-Clause, deftio) → build/compare/third_party/fr_math ..."
  rm -rf "$DEST/fr_math"
  git clone --depth 1 https://github.com/deftio/fr_math.git "$DEST/fr_math"
fi
