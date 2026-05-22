#!/usr/bin/env bash
set -eu

failures=0

fail() {
  echo "particle contract audit: $*" >&2
  failures=$((failures + 1))
}

check_absent() {
  pattern=$1
  shift
  matches=$(grep -En -- "$pattern" "$@" || true)
  if [ -n "$matches" ]; then
    fail "unexpected legacy particle-field access"
    echo "$matches" >&2
  fi
}

check_present() {
  pattern=$1
  file=$2
  if ! grep -Eq -- "$pattern" "$file"; then
    fail "missing documented contract '$pattern' in $file"
  fi
}

# Level 2 must stay layout-agnostic in the implementation.  Region/species
# semantics are provided by oh_particle_adapter.
check_absent '->(nid|spec)\b' src/c/ohhelp2.c

# Level 3 may touch S_particle.x/y/z only inside the default S_particle mapping
# adapter.  Custom layouts must use offset-based or callback adapters.
level3_xyz=$(grep -En -- '->(x|y|z)\b' src/c/ohhelp3.c || true)
level3_xyz_count=$(printf '%s\n' "$level3_xyz" | sed '/^$/d' | wc -l)
if [ "$level3_xyz_count" -ne 6 ]; then
  fail "unexpected number of Level-3 direct x/y/z accesses: $level3_xyz_count"
  echo "$level3_xyz" >&2
fi
check_absent '->(nid|spec)\b' src/c/ohhelp3.c

# New direct accesses must not spread outside the known migration boundary.
legacy_accesses=$(
  grep -nE -- '->(nid|spec|x|y|z)\b' \
      include/ohhelp2.h include/ohhelp3.h include/ohhelp4p.h \
      include/ohhelp4s.h src/c/oh_particle_adapter.c src/c/ohhelp2.c \
      src/c/ohhelp3.c src/c/ohhelp4p.c src/c/ohhelp4s.c \
    | grep -vE 'src/c/oh_particle_adapter.c:' \
    | grep -vE 'src/c/ohhelp3.c:' \
    | grep -vE 'src/c/ohhelp4p.c:' \
    | grep -vE 'src/c/ohhelp4s.c:' \
    | grep -vE 'include/ohhelp2.h:' \
    | grep -vE 'include/ohhelp4p.h:' \
    | grep -vE 'include/ohhelp4s.h:' \
    || true
)
if [ -n "$legacy_accesses" ]; then
  fail "legacy particle-field access escaped the documented migration boundary"
  echo "$legacy_accesses" >&2
fi

# Keep the hidden Level-4 and injection semantics explicitly documented until
# they are replaced by a v2 adapter contract.
check_present 'nid < 0' doc/design/v2-particle-contracts.md
check_present 'nid == -2' doc/design/v2-particle-contracts.md
check_present 'oh_remove_injected_particle\(\)' doc/design/v2-particle-contracts.md
check_present 'packed-grid id operations' doc/design/v2-particle-contracts.md

if [ "$failures" -ne 0 ]; then
  exit 1
fi
