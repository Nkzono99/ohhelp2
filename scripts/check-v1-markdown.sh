#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$REPO_ROOT"

PYTHON=${PYTHON:-python3}
TMPDIR=

cleanup() {
  if [ -n "$TMPDIR" ]; then
    rm -rf "$TMPDIR"
  fi
}
trap cleanup EXIT

if ! "$PYTHON" - <<'PY' >/dev/null 2>&1
import sys
raise SystemExit(0 if sys.version_info >= (3, 10) else 1)
PY
then
  echo "documentation conversion requires Python 3.10 or newer; set PYTHON to a newer interpreter" >&2
  exit 2
fi

if ! "$PYTHON" - <<'PY' >/dev/null 2>&1
import importlib.util
raise SystemExit(0 if importlib.util.find_spec("fitz") else 1)
PY
then
  echo "missing documentation dependency: Python module 'fitz' (PyMuPDF); install requirements-doc.txt" >&2
  exit 2
fi

TMPDIR=$(mktemp -d)
"$PYTHON" scripts/convert_pdfs_to_md.py --out-dir "$TMPDIR"

if ! diff -ru -- doc/v1/markdown "$TMPDIR"; then
  echo "doc/v1/markdown is out of date; rerun scripts/convert_pdfs_to_md.py" >&2
  exit 1
fi
