#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$REPO_ROOT"

FPM=${FPM:-fpm}
FC=${MPIFC:-${FC:-mpifort}}
CC=${MPICC:-${CC:-mpicc}}

if ! command -v "$FPM" >/dev/null 2>&1; then
  echo "fpm is required for the package build gate; set FPM or install fpm" >&2
  echo "set OH_CHECK_FPM=0 only for local environments without fpm" >&2
  exit 1
fi

"$FPM" build --compiler "$FC" --c-compiler "$CC"
