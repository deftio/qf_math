#!/usr/bin/env bash
# Build and run libfixmath's upstream tests_ro64 suite (artifacts under build/compare/).
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
bash "$ROOT/compare/fetch_deps.sh"
SRC="$ROOT/build/compare/third_party/libfixmath"
BUILD="$ROOT/build/compare/third_party/libfixmath-build"
echo "Configuring libfixmath CMake build in $BUILD ..."
cmake -B "$BUILD" -S "$SRC"
echo "Building tests_ro64 ..."
cmake --build "$BUILD" --target tests_ro64 -j
echo "Running tests_ro64 ..."
"$BUILD/tests_ro64"
echo "libfixmath tests_ro64 completed successfully."
