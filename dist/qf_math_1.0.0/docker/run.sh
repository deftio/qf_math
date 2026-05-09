#!/usr/bin/env bash
#
# Build the toolchain image (if needed) and run the cross-target size report.
#
# Usage:
#   ./docker/run.sh              # build image + run docker/build_sizes.sh
#   ./docker/run.sh --rebuild    # force image rebuild first
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
IMAGE_NAME="qf-math-sizes"

REBUILD=0
for arg in "$@"; do
    case "$arg" in
        --rebuild) REBUILD=1 ;;
        -h | --help)
            sed -n '3,11p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            exit 2
            ;;
    esac
done

if [[ "${REBUILD}" == "1" ]] || ! docker image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
    echo "Building Docker image '${IMAGE_NAME}'..."
    docker build --platform linux/amd64 -t "${IMAGE_NAME}" "${SCRIPT_DIR}"
    echo ""
fi

docker run --platform linux/amd64 --rm \
    -v "${PROJECT_ROOT}:/src" \
    "${IMAGE_NAME}" \
    bash /src/docker/build_sizes.sh
