#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$REPO_ROOT"

git diff --check
bash tests/test_particle_contract_audit.sh
if [ "${OH_CHECK_V1_MARKDOWN:-0}" = "1" ]; then
  bash scripts/check-v1-markdown.sh
fi
if [ "${OH_CHECK_FPM:-1}" = "1" ]; then
  bash scripts/fpm-build-test.sh
fi
bash scripts/docker-build-test.sh
