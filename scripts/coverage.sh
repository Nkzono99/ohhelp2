#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$REPO_ROOT"

GCOVR=${GCOVR:-gcovr}
REPORT_DIR=${REPORT_DIR:-build/coverage}

if ! command -v "$GCOVR" >/dev/null 2>&1; then
  echo "gcovr is required for coverage reports; set GCOVR or install gcovr" >&2
  exit 1
fi

rm -rf "$REPORT_DIR"
mkdir -p "$REPORT_DIR"

COVERAGE=1 bash scripts/docker-build-test.sh

"$GCOVR" --root "$REPO_ROOT" \
  --object-directory "$REPO_ROOT/build/docker" \
  --filter "$REPO_ROOT/src/" \
  --filter "$REPO_ROOT/include/" \
  --exclude "$REPO_ROOT/tests/" \
  --exclude "$REPO_ROOT/sample/" \
  --exclude "$REPO_ROOT/build/" \
  --xml-pretty --xml "$REPORT_DIR/coverage.xml" \
  --html-details "$REPORT_DIR/index.html" \
  --txt
