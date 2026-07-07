#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$REPO_ROOT"

GCOVR=${GCOVR:-gcovr}
REPORT_DIR=${REPORT_DIR:-build/coverage}
COVERAGE_MIN_LINE=${COVERAGE_MIN_LINE:-85}

if ! command -v "$GCOVR" >/dev/null 2>&1; then
  echo "gcovr is required for coverage reports; set GCOVR or install gcovr" >&2
  exit 1
fi

rm -rf "$REPORT_DIR" build/docker
rm -f ./*.mod ./*.smod
mkdir -p "$REPORT_DIR"

COVERAGE=1 bash scripts/docker-build-test.sh

COMMON_GCOVR_ARGS=(
  --root "$REPO_ROOT"
  --object-directory "$REPO_ROOT/build/docker"
  --exclude "$REPO_ROOT/tests/"
  --exclude "$REPO_ROOT/sample/"
  --exclude "$REPO_ROOT/build/"
  --merge-mode-functions merge-use-line-min
)

mkdir -p "$REPORT_DIR/full"

"$GCOVR" "${COMMON_GCOVR_ARGS[@]}" \
  --filter "$REPO_ROOT/src/" \
  --filter "$REPO_ROOT/include/" \
  --xml-pretty --xml "$REPORT_DIR/full/coverage.xml" \
  --html-details "$REPORT_DIR/full/index.html" \
  --txt "$REPORT_DIR/full/coverage.txt"

echo "Full legacy-inclusive coverage report:"
cat "$REPORT_DIR/full/coverage.txt"

"$GCOVR" "${COMMON_GCOVR_ARGS[@]}" \
  --filter "$REPO_ROOT/src/c/oh_.*" \
  --filter "$REPO_ROOT/src/fortran/oh_v2.F90" \
  --filter "$REPO_ROOT/include/oh_particle_adapter.h" \
  --fail-under-line "$COVERAGE_MIN_LINE" \
  --xml-pretty --xml "$REPORT_DIR/coverage.xml" \
  --html-details "$REPORT_DIR/index.html" \
  --txt
