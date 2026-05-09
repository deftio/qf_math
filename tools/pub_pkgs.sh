#!/usr/bin/env bash
# Publish qf_math to the package registries used by this repo.
#
# Credentials are expected to already be configured locally:
#   - PlatformIO: pio account login
#   - Espressif:  compote registry login --default-namespace deftio
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

DRY_RUN=0
CONFIRM=0
DO_PIO=1
DO_IDF=1
RUN_TESTS=0
PIO_OWNER="${PIO_OWNER:-}"
IDF_NAMESPACE="${IDF_NAMESPACE:-deftio}"
IDF_PROFILE="${IDF_PROFILE:-}"

usage() {
  cat <<'EOF'
Usage: tools/pub_pkgs.sh [options]

Publish qf_math to PlatformIO and Espressif's component registry.

Options:
  --dry-run              Print commands without running them
  --yes                  Do not prompt before publishing
  --pio-only             Publish only to PlatformIO
  --idf-only             Publish only to Espressif Component Registry
  --pio-owner NAME       Pass --owner NAME to pio pkg publish
  --idf-namespace NAME   Pass --namespace NAME to compote (default: deftio)
  --idf-profile NAME     Pass --profile NAME to compote
  --run-tests            Run make test and make test-lean-cpp first
  -h, --help             Show this help

Environment overrides:
  PIO_OWNER              PlatformIO owner name
  IDF_NAMESPACE          Espressif component namespace (default: deftio)
  IDF_PROFILE            Espressif compote profile

Examples:
  tools/pub_pkgs.sh --dry-run
  tools/pub_pkgs.sh --yes
  tools/pub_pkgs.sh --idf-only --idf-profile default
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run)
      DRY_RUN=1
      ;;
    --yes)
      CONFIRM=1
      ;;
    --pio-only)
      DO_PIO=1
      DO_IDF=0
      ;;
    --idf-only)
      DO_PIO=0
      DO_IDF=1
      ;;
    --pio-owner)
      PIO_OWNER="${2:?missing value for --pio-owner}"
      shift
      ;;
    --idf-namespace)
      IDF_NAMESPACE="${2:?missing value for --idf-namespace}"
      shift
      ;;
    --idf-profile)
      IDF_PROFILE="${2:?missing value for --idf-profile}"
      shift
      ;;
    --run-tests)
      RUN_TESTS=1
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
  shift
done

run() {
  printf '+'
  printf ' %q' "$@"
  printf '\n'
  if [[ "$DRY_RUN" -eq 0 ]]; then
    "$@"
  fi
}

need_file() {
  if [[ ! -f "$1" ]]; then
    echo "missing required file: $1" >&2
    exit 1
  fi
}

need_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "missing required command: $1" >&2
    exit 1
  fi
}

need_file "library.json"
need_file "idf_component.yml"
need_file "src/qf_math.h"

eval "$(python3 tools/qf_version.py show --format shell)"
echo "qf_math ${QF_VERSION} (${QF_TAG})"

run python3 tools/qf_version.py verify

if [[ "$RUN_TESTS" -eq 1 ]]; then
  run make test
  run make test-lean-cpp
fi

targets=()
if [[ "$DO_PIO" -eq 1 ]]; then
  need_cmd pio
  targets+=("PlatformIO")
fi
if [[ "$DO_IDF" -eq 1 ]]; then
  need_cmd compote
  targets+=("Espressif")
fi

if [[ "${#targets[@]}" -eq 0 ]]; then
  echo "nothing to publish" >&2
  exit 1
fi

echo "Targets: ${targets[*]}"

if [[ "$DRY_RUN" -eq 0 && "$CONFIRM" -eq 0 ]]; then
  expected="publish ${QF_VERSION}"
  printf "Type '%s' to publish: " "$expected"
  read -r answer
  if [[ "$answer" != "$expected" ]]; then
    echo "aborted"
    exit 1
  fi
fi

if [[ "$DO_PIO" -eq 1 ]]; then
  pio_cmd=(pio pkg publish . --type library --no-interactive)
  if [[ -n "$PIO_OWNER" ]]; then
    pio_cmd+=(--owner "$PIO_OWNER")
  fi
  run "${pio_cmd[@]}"
fi

if [[ "$DO_IDF" -eq 1 ]]; then
  idf_cmd=(compote component upload --name qf_math)
  if [[ -n "$IDF_NAMESPACE" ]]; then
    idf_cmd+=(--namespace "$IDF_NAMESPACE")
  fi
  if [[ -n "$IDF_PROFILE" ]]; then
    idf_cmd+=(--profile "$IDF_PROFILE")
  fi
  run "${idf_cmd[@]}"
fi

echo "publish helper complete"
