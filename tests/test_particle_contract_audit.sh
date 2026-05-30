#!/usr/bin/env bash
set -euo pipefail

failures=0

fail() {
  echo "particle contract audit: $*" >&2
  failures=$((failures + 1))
}

check_absent() {
  pattern=$1
  shift
  matches=$(grep -En -- "$pattern" "$@" 2>/dev/null || true)
  if [ -n "$matches" ]; then
    fail "unexpected legacy particle-field access"
    echo "$matches" >&2
  fi
}

literal_pattern() {
  printf '%s' "$1" | sed \
    -e 's/\\(/(/g' \
    -e 's/\\)/)/g' \
    -e 's/\\\[/[/g' \
    -e 's/\\\]/]/g' \
    -e 's/\\\*/\*/g' \
    -e 's/\\+/+/g' \
    -e 's/\\\././g' \
    -e 's/\\&/\&/g'
}

check_present() {
  pattern=$1
  file=$2
  if grep -Eq -- "$pattern" "$file" 2>/dev/null; then
    return
  fi
  literal=$(literal_pattern "$pattern")
  if grep -Fq -- "$literal" "$file"; then
    return
  fi
  fail "missing documented contract '$pattern' in $file"
}

check_present_re() {
  pattern=$1
  file=$2
  if ! grep -Eq -- "$pattern" "$file"; then
    fail "missing documented regex contract '$pattern' in $file"
  fi
}

check_present_literal() {
  literal=$1
  file=$2
  if ! grep -Fq -- "$literal" "$file"; then
    fail "missing documented literal contract '$literal' in $file"
  fi
}

check_order() {
  file=$1
  first=$2
  second=$3
  first_line=$(grep -Fn -- "$first" "$file" | head -n 1 | cut -d: -f1 || true)
  second_line=$(grep -Fn -- "$second" "$file" | head -n 1 | cut -d: -f1 || true)
  if [ -z "$first_line" ] || [ -z "$second_line" ]; then
    fail "missing ordered contract '$first' before '$second' in $file"
    return
  fi
  if [ "$first_line" -ge "$second_line" ]; then
    fail "wrong order for '$first' before '$second' in $file"
  fi
}

check_signature() {
  name=$1
  file=$2
  expected=$3
  if ! awk -v name="$name" -v expected="$expected" '
    function canonical(s) {
      gsub(/[[:space:]]+/, " ", s)
      gsub(/^ | $/, "", s)
      gsub(/[ ]*[*][ ]*/, "* ", s)
      gsub(/[(][ ]*/, "(", s)
      gsub(/[ ][)]/, ")", s)
      gsub(/[ ]*,[ ]*/, ", ", s)
      return s
    }
    BEGIN { expected = canonical(expected) }
    index($0, name "(") { collecting = 1; sig = "" }
    collecting {
      sig = sig " " $0
      if ($0 ~ /[;{]/) {
        normalized = canonical(sig)
        pos = index(normalized, expected)
        if (pos) {
          prefix = pos == 1 ? "" : substr(normalized, pos - 1, 1)
          suffix = substr(normalized, pos + length(expected), 1)
          if ((prefix == "" || prefix == " " || prefix == "*") &&
              (suffix == ";" || suffix == " " || suffix == "{"))
            found = 1
        }
        collecting = 0
      }
    }
    END { exit found ? 0 : 1 }
  ' "$file"; then
    fail "missing expected signature for $name in $file"
  fi
}

check_signature_exact() {
  name=$1
  file=$2
  expected=$3
  if ! awk -v name="$name" -v expected="$expected" '
    function canonical(s) {
      gsub(/[[:space:]]+/, " ", s)
      gsub(/^ | $/, "", s)
      gsub(/[ ]*[*][ ]*/, "* ", s)
      gsub(/[(][ ]*/, "(", s)
      gsub(/[ ][)]/, ")", s)
      gsub(/[ ]*,[ ]*/, ", ", s)
      gsub(/[ ]*[;{][ ]*$/, "", s)
      return s
    }
    BEGIN { expected = canonical(expected) }
    index($0, name "(") { collecting = 1; sig = "" }
    collecting {
      sig = sig " " $0
      if ($0 ~ /[;{]/) {
        if (canonical(sig) == expected) found = 1
        collecting = 0
      }
    }
    END { exit found ? 0 : 1 }
  ' "$file"; then
    fail "missing exact signature for $name in $file"
  fi
}

check_function_absent() {
  name=$1
  file=$2
  pattern=$3
  matches=$(awk -v name="$name" -v pattern="$pattern" '
    index($0, name "(") { collecting = 1; started = 0; depth = 0 }
    collecting {
      if (!started && $0 ~ /;/) {
        collecting = 0
        next
      }
      if ($0 ~ pattern) print FILENAME ":" FNR ":" $0
      rest = $0
      opens = gsub(/\{/, "", rest)
      rest = $0
      closes = gsub(/\}/, "", rest)
      if (opens) started = 1
      if (started) {
        depth += opens - closes
        if (depth <= 0) collecting = 0
      }
    }
  ' "$file")
  if [ -n "$matches" ]; then
    fail "unexpected contract '$pattern' inside function $name in $file"
    echo "$matches" >&2
  fi
}

if [ -d bag_src ] && [ -n "$(find bag_src -type f -print -quit)" ]; then
  fail "bag_src must not contain source files; Level 4 sources live under src/ and include/"
fi

check_present '#ifndef OHHELP_C_H' include/ohhelp_c.h
check_present '#ifndef OH_CONFIG_H' include/oh_config.h
check_present '#ifndef OHHELP_F_H' include/ohhelp_f.h
check_present '#ifndef OH_FORTRAN_V2_H' include/oh_fortran_v2.h
check_present '#ifndef OHHELP1_H' include/ohhelp1.h
check_present '#ifndef OHHELP2_H' include/ohhelp2.h
check_present '#ifndef OHHELP3_H' include/ohhelp3.h
check_present '#ifndef OHHELP4P_H' include/ohhelp4p.h
check_present '#ifndef OHHELP4S_H' include/ohhelp4s.h
check_present '^name = "ohhelp2"' fpm.toml
check_present '^# ohhelp2' README.md
check_present 'ohhelp_v2' README.md
check_present '#include "ohhelp1.h"' include/ohhelp_c.h
check_present '#include "ohhelp2.h"' include/ohhelp_c.h
check_present '#include "ohhelp3.h"' include/ohhelp_c.h
check_absent '^(void|int)\s+oh[0-9]' include/ohhelp_c.h
check_present 'extern "C"' include/ohhelp1.h
check_present 'extern "C"' include/ohhelp2.h
check_present 'extern "C"' include/ohhelp3.h
check_present 'extern "C"' include/ohhelp4p.h
check_present 'extern "C"' include/ohhelp4s.h

for executable_script in scripts/test.sh scripts/check-v1-markdown.sh \
                         scripts/fpm-build-test.sh; do
  if [ ! -x "$executable_script" ]; then
    fail "$executable_script must be executable"
  fi
done

check_present 'set -euo pipefail' scripts/test.sh
check_present 'set -euo pipefail' scripts/check-v1-markdown.sh
check_present 'set -euo pipefail' scripts/fpm-build-test.sh
check_present 'set -euo pipefail' scripts/docker-build-test.sh
check_present 'set -euo pipefail' tests/test_particle_contract_audit.sh
check_present 'BASH_SOURCE\[0\]' scripts/test.sh
check_present 'cd "\$REPO_ROOT"' scripts/test.sh
check_present 'BASH_SOURCE\[0\]' scripts/check-v1-markdown.sh
check_present 'cd "\$REPO_ROOT"' scripts/check-v1-markdown.sh
check_present 'BASH_SOURCE\[0\]' scripts/fpm-build-test.sh
check_present 'cd "\$REPO_ROOT"' scripts/fpm-build-test.sh
check_present 'BASH_SOURCE\[0\]' scripts/docker-build-test.sh
check_present 'cd "\$REPO_ROOT"' scripts/docker-build-test.sh
check_present 'build/docker/negative/\$suite' scripts/docker-build-test.sh

check_present '#include "ohhelp1.h"' include/ohhelp2.h
check_present '#include "ohhelp2.h"' include/ohhelp3.h
check_absent '\b(MPI_UB|MPI_Type_struct)\b' src/c/oh_context.c src/c/ohhelp1.c \
  src/c/ohhelp3.c
check_present 'mem_alloc_invalid_size_error' src/c/ohhelp1.c
check_present 'esize <= 0 \|\| count < 0' src/c/ohhelp1.c
check_present '\(size_t\)count > \(\(size_t\)-1\) / \(size_t\)esize' \
  src/c/ohhelp1.c

# Level 2 must stay layout-agnostic in the implementation.  Region/species
# semantics are provided by oh_particle_adapter.
check_absent '^EXTERN ' include/ohhelp1.h
check_absent '\bfam_comm\b' include/ohhelp1.h
check_absent '\b(InjectedParticles|nOfInjections|specBase|primaryParts|secondaryBase|gridMask|logGrid|BoundaryCondition|BoundarySendBuf)\b' \
  include/ohhelp1.h include/ohhelp2.h include/ohhelp3.h \
  include/ohhelp4p.h include/ohhelp4s.h
check_absent 'struct oh_state\s*\{' include/ohhelp1.h
check_absent '\bOhDefaultState\b' include/ohhelp1.h
check_present 'struct oh_state\s*\{' src/c/oh_context_internal.h
check_present '\bOhDefaultState\b' src/c/oh_context_internal.h
check_present_literal 'void *particles;' src/c/oh_context_internal.h
check_present_literal 'void *send_buffer;' src/c/oh_context_internal.h
check_present_literal 'void **recv_buffer_bases;' src/c/oh_context_internal.h
check_present_literal 'void *level4_boundary_send_buffer;' \
  src/c/oh_context_internal.h
check_present_literal 'int *level4_send_counts;' src/c/oh_context_internal.h
check_present_literal 'int *recv_counts;' src/c/oh_context_internal.h
check_present_literal 'int *send_counts;' src/c/oh_context_internal.h
check_present_literal 'EXTERN void *Particles;' src/c/ohhelp2_internal.h
check_present_literal 'EXTERN void *SendBuf;' src/c/ohhelp2_internal.h
check_present_literal 'EXTERN void **RecvBufBases;' src/c/ohhelp2_internal.h
check_present_literal 'EXTERN void* BoundarySendBuf;' src/c/ohhelp4s_internal.h
check_present_literal 'state->level4_boundary_send_buffer = BoundarySendBuf' \
  src/c/ohhelp4s_state.h
check_present_literal 'Particles = particles;' src/c/ohhelp2.c
check_present 'SendBuf = mem_alloc\(particle_stride_state\(state\), maxlocalp, "SendBuf"\);' \
  src/c/ohhelp2.c
check_present_literal 'state->recv_buffer_bases = RecvBufBases;' src/c/ohhelp2.c
check_absent 'struct S_particle \*particles;' src/c/oh_context_internal.h
check_absent 'struct S_particle \*send_buffer;' src/c/oh_context_internal.h
check_absent 'struct S_particle \*\*recv_buffer_bases;' \
  src/c/oh_context_internal.h
check_absent 'struct S_particle \*level4_boundary_send_buffer;' \
  src/c/oh_context_internal.h
check_absent 'EXTERN struct S_particle \*(Particles|SendBuf|BoundarySendBuf)' \
  src/c/ohhelp2_internal.h src/c/ohhelp4s_internal.h
check_absent 'EXTERN struct S_particle \*\*RecvBufBases' \
  src/c/ohhelp2_internal.h
check_absent 'EXTERN struct S_particle\* BoundarySendBuf' \
  src/c/ohhelp4s_internal.h
check_absent '\b(Particles|SendBuf) = \(struct S_particle\*\)' \
  src/c/ohhelp2.c src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\(struct S_particle\s*\*\).*BoundarySendBuf' src/c/ohhelp4s.c
check_absent 'BoundarySendBuf\s*(\[|\+)' src/c/ohhelp4s.c src/c/ohhelp4s_state.h
check_absent 'struct S_(node|heap|commlist|commsched_context|comms|statscurr|statstime|statspart|statstotal|stats)\s*\{' include/ohhelp1.h
check_absent '\b(init1|mem_alloc|mem_alloc_error|errstop|local_errstop|set_total_particles|transbound1|try_primary1|try_stable1|rebalance1|build_new_comm|vprint|dprint)\s*\(' include/ohhelp1.h
check_present 'OH_MODE_NORMAL_PRIMARY' include/oh_mode.h
check_present '#include "oh_mode.h"' include/oh_context.h
check_present '#include "oh_mode.h"' include/ohhelp1.h
check_present 'OH_MODE_NORMAL_PRIMARY' src/fortran/oh_mod1.F90
check_present 'OH_MODE_NORMAL_PRIMARY' src/fortran/oh_v2.F90
check_present 'oh1_comm' include/ohhelp1.h
check_absent 'return\(accMode\)' src/c/ohhelp1.c
check_absent '\bif \(myRank==0\)' src/c/ohhelp1.c
check_present 'init1_state(struct oh_state *state' src/c/ohhelp1_internal.h
check_present 'init1_state(&OhDefaultState' src/c/ohhelp1.c
check_present 'init1_state(&OhDefaultState' src/c/ohhelp2.c
check_present 'init1_state(&OhDefaultState' src/c/ohhelp3.c
check_present 'oh_context_init_level1_state' src/c/oh_context_internal.h
check_present 'oh_context_build_grid_neighbors' src/c/oh_context_internal.h
check_present 'oh_context_apply_neighbors' src/c/oh_context_internal.h
check_present 'oh_context_init_level1_state(state' src/c/ohhelp1.c
check_present 'run_nondefault_init1_state_test' \
  tests/test_oh_context_lifecycle.c
check_present 'run_nondefault_init1_rebalance_test' \
  tests/test_oh_context_lifecycle.c
check_present 'run_nondefault_legacy_side_channel_isolation_test' \
  tests/test_oh_context_lifecycle.c
check_present 'oh_context_is_default_state(state) && Mode_PS(currmode) && FamIndex' \
  src/c/ohhelp1.c
check_present 'oh_context_is_default_state(state) && FamIndex' \
  src/c/ohhelp1.c
check_present 'oh_context_is_default_state(state) && NeighborsShadow' \
  src/c/ohhelp1.c
check_present 'rcounts == context->recv_counts' \
  tests/test_oh_context_lifecycle.c
check_present 'borrowed_context->owns_region_id == 0' \
  tests/test_oh_context_lifecycle.c
check_present 'OH_PARTICLES_BORROWED' tests/test_oh_context_lifecycle.c
check_present 'OH_PARTICLES_OWNED' tests/test_oh_context_lifecycle.c
check_present 'borrowed_nphgram\[0\] == 11' \
  tests/test_oh_context_lifecycle.c
check_present 'both NULL or both borrowed' src/c/oh_context.c
check_present 'non-default init1_state() does not support stats or verbose yet' \
  src/c/oh_context.c
check_present 'non-default init1_state() cannot borrow recv counts yet' \
  src/c/oh_context.c
check_present 'non-default init1_state() cannot borrow send counts yet' \
  src/c/oh_context.c
check_present 'stats == 0' doc/v2/design/context.md
check_present 'borrowed `rcounts` / `scounts` are not supported yet' \
  doc/v2/design/context.md
check_present '&nphgram_slot' doc/v2/design/context.md
check_present '&nphgram_slot' doc/v2/usage/api-by-level.md
check_present '&nphgram_slot' doc/v2/usage/pic-lifecycle.md
check_present '&nphgram_slot' doc/v2/usage/v2-particle-and-weight.md
check_absent 'oh_context_bind_particle_accounting\(ctx, nphgram' \
  doc/v2/design/context.md doc/v2/usage/api-by-level.md \
  doc/v2/usage/pic-lifecycle.md doc/v2/usage/v2-particle-and-weight.md
check_present 'previous context-owned output pointers as stale' \
  doc/v2/design/context.md
check_present '`rcounts` / `scounts` from the legacy raw initializer' \
  doc/v2/usage/api-by-level.md
check_absent '\binit1\(' src/c/ohhelp2.c src/c/ohhelp3.c
check_absent 'init1_state\(\) currently supports only the default context' \
  src/c/ohhelp1.c
check_absent 'collective wrappers still need to read from the context' \
  doc/v2/design/context.md
check_absent '->(nid|spec)\b' src/c/ohhelp2.c
check_absent '->(nid|spec)\b' src/c/ohhelp3.c
check_absent '->(nid|spec)\b' \
  src/c/ohhelp4_particle.h src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(part|particle|pbuf|p|q|P|src|dst)[[:space:]]*->[[:space:]]*(nid|spec|x|y|z)\b|\([[:space:]]*\*[[:space:]]*(part|particle|pbuf|p|q|P|src|dst)[[:space:]]*\)[[:space:]]*\.[[:space:]]*(nid|spec|x|y|z)\b|\b(part|particle|pbuf|p|q|P|src|dst)[[:space:]]*\[[^]]+\][[:space:]]*\.[[:space:]]*(nid|spec|x|y|z)\b' \
  src/c/ohhelp2.c src/c/ohhelp3.c src/c/ohhelp4_particle.h \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_present 'OH_nid_t region' src/c/ohhelp2.c
check_signature_exact 'state_update_injected_particle_count' src/c/ohhelp2.c \
  'static void state_update_injected_particle_count(struct oh_state *state, void *part, int delta)'
check_signature_exact 'state_particle_region' src/c/ohhelp2.c \
  'static OH_nid_t state_particle_region(struct oh_state *state, const void *part, int primary_or_secondary)'
check_signature_exact 'state_set_particle_region' src/c/ohhelp2.c \
  'static void state_set_particle_region(struct oh_state *state, void *part, OH_nid_t region, int primary_or_secondary)'
check_signature_exact 'state_mark_particle_removed' src/c/ohhelp2.c \
  'static void state_mark_particle_removed(struct oh_state *state, void *part, int primary_or_secondary)'
check_signature_exact 'state_particle_species' src/c/ohhelp2.c \
  'static int state_particle_species(struct oh_state *state, const void *part)'
check_signature_exact 'state_particle_subdomain' src/c/ohhelp2.c \
  'static int state_particle_subdomain(struct oh_state *state, void *part, int primary_or_secondary)'
check_signature_exact 'state_map_injected_particle_to_subdomain' src/c/ohhelp2.c \
  'static int state_map_injected_particle_to_subdomain(struct oh_state *state, void *part)'
check_signature_exact 'state_primarize_particle' src/c/ohhelp2.c \
  'static oh_particle_region_t state_primarize_particle(struct oh_state *state, void *part)'
check_signature_exact 'state_particle_at' src/c/ohhelp2.c \
  'static void *state_particle_at(struct oh_state *state, void *base, int index)'
check_signature_exact 'state_injected_particle_index' src/c/ohhelp2.c \
  'static int state_injected_particle_index(struct oh_state *state, const void *part)'
check_signature_exact 'state_copy_particle' src/c/ohhelp2.c \
  'static void state_copy_particle(struct oh_state *state, void *dst, const void *src)'
check_signature_exact 'state_copy_particles' src/c/ohhelp2.c \
  'static void state_copy_particles(struct oh_state *state, void *dst, const void *src, int count)'
check_signature_exact 'state_injected_particle_region_kind' src/c/ohhelp2.c \
  'static int state_injected_particle_region_kind(struct oh_state *state, void *part)'
check_absent 'static struct S_particle \*state_particle_at' src/c/ohhelp2.c
check_absent 'state_copy_particle\(struct oh_state \*state, struct S_particle \*dst' \
  src/c/ohhelp2.c
check_absent '^[[:space:]]+struct S_particle \*(particles|sendbuf|rbb|rb|part|pbuf|rbuf|sbuf)(=|;)' \
  src/c/ohhelp2.c
check_present_literal 'void *particles=state->particles' src/c/ohhelp2.c
check_present_literal 'void *sendbuf=state->send_buffer' src/c/ohhelp2.c
check_present_literal 'void *part=state_particle_at' src/c/ohhelp2.c
check_present 'state_mark_particle_removed' src/c/ohhelp2.c
check_present 'state_update_injected_particle_count' src/c/ohhelp2.c
check_present 'state_map_injected_particle_to_subdomain' src/c/ohhelp2.c
check_present 'run_injected_position_routing_test' \
  tests/test_oh_context_lifecycle.c
check_present 'run_injected_accounting_contract_test' \
  tests/test_oh_context_lifecycle.c
check_present 'context->injected_particles\[0\] == 1' \
  tests/test_oh_context_lifecycle.c
check_present 'run_particle_adapter_reset_rebind_test' \
  tests/test_oh_context_lifecycle.c
check_present 'MPI_Type_free(&copied_type)' \
  tests/test_oh_context_lifecycle.c
check_present 'copied_adapter\.region_offset = (size_t)-1' \
  tests/test_oh_context_lifecycle.c
check_present 'oh_context_set_particle_adapter(context, NULL)' \
  tests/test_oh_context_lifecycle.c
check_present 'run_position_only_adapter_context_mapping_test' \
  tests/test_oh_context_lifecycle.c
check_present 'position_adapter\.map_to_subdomain = 0' \
  tests/test_oh_context_lifecycle.c
check_present 'state_update_injected_particle_count()' \
  doc/v2/design/particle-adapter.md
check_present 'position-field helper alone does not make the physical position authoritative' \
  doc/v2/design/particle-adapter.md
check_present 'type == MPI_DATATYPE_NULL' src/c/ohhelp2.c
check_present 'oh_context_set_particle_mpi_type' include/oh_context.h
check_present 'oh_context_transbound1' include/oh_context.h
check_present 'oh_context_transbound2' include/oh_context.h
check_present 'oh_context_transbound3' include/oh_context.h
check_present 'oh_context_map_particle_to_neighbor' include/oh_context.h
check_present 'oh_context_map_particle_to_subdomain' include/oh_context.h
check_present 'oh_context_exchange_borders' include/oh_context.h
check_present 'state->use_custom_particle_adapter = 0;' src/c/ohhelp2.c
check_present 'state->custom_particle_mpi_type = MPI_DATATYPE_NULL;' \
  src/c/ohhelp2.c
check_present 'MPI_Type_dup(adapter->mpi_type, &state->custom_particle_mpi_type)' \
  src/c/ohhelp2.c
check_present 'free_owned_particle_mpi_type_state(state)' src/c/ohhelp2.c
check_present 'ownsTParticle' src/c/ohhelp2_internal.h
check_present 'ownsCustomTParticle' src/c/ohhelp2_internal.h
check_present 'free_owned_default_particle_mpi_type' src/c/ohhelp2.c
check_present 'free_owned_default_custom_particle_mpi_type' src/c/ohhelp2.c
check_present 'OhDefaultState\.owns_particle_mpi_type = ownsTParticle' \
  src/c/oh_context.c
check_present 'test_oh_default_particle_type_ownership\.c' \
  scripts/docker-build-test.sh
check_present 'test_oh2_init_guard\.c' scripts/docker-build-test.sh
check_present 'expected oh2_init with NULL pbuf slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected oh2_init with NULL pbase slot to fail' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 1 build/docker/test_oh2_init_guard oh3-valid' \
  scripts/docker-build-test.sh
check_present 'expected oh3_init with NULL pbuf slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected oh3_init with NULL pbase slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 2 raw init with NULL pbuf slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 raw init with NULL pbuf slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 2 raw init with NULL sdid to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 2 raw init with NULL nphgram to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 2 raw init with NULL totalp to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 raw init with NULL sdid to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 raw init with NULL nphgram to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 raw init with NULL totalp to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 2 raw init with negative maxfrac to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 raw init with negative maxfrac to fail' \
  scripts/docker-build-test.sh
check_present 'expected oh2_max_local_particles with negative minmargin to fail' \
  scripts/docker-build-test.sh
check_present 'expected oh2_max_local_particles oversized base capacity to fail' \
  scripts/docker-build-test.sh
check_present 'expected oh2_max_local_particles oversized margin capacity to fail' \
  scripts/docker-build-test.sh
check_present 'oh2_init() requires a particle pointer slot' src/c/ohhelp2.c
check_present 'oh2_init() requires a particle base slot' src/c/ohhelp2.c
check_present 'oh3_init() requires a particle pointer slot' src/c/ohhelp3.c
check_present 'oh3_init() requires a particle base slot' src/c/ohhelp3.c
check_present 'oh_init() requires maxfrac >= 0' src/c/ohhelp1.c
check_present 'minimum particle buffer margin (%d) should be non-negative' \
  src/c/ohhelp2.c
check_present 'if (npl>INT_MAX) mem_alloc_error("Particles", 0)' \
  src/c/ohhelp2.c
check_present 'npl>(dint)INT_MAX-margin' src/c/ohhelp2.c
check_present 'request_slots_wide = 4LL \* nnns \+ 2LL \* OH_NEIGHBORS' \
  src/c/ohhelp2.c
check_present 'request_slots_wide > INT_MAX' src/c/ohhelp2.c
check_present 'oh2_init_raw requires a particle pointer slot' \
  src/c/oh_fortran_v2.c
check_present 'oh3_init_raw requires a particle pointer slot' \
  src/c/oh_fortran_v2.c
check_present 'oh2_init_raw requires a region id array' src/c/oh_fortran_v2.c
check_present 'oh2_init_raw requires a particle histogram array' \
  src/c/oh_fortran_v2.c
check_present 'oh2_init_raw requires a total particle array' \
  src/c/oh_fortran_v2.c
check_present 'oh3_init_raw requires a region id array' src/c/oh_fortran_v2.c
check_present 'oh3_init_raw requires a particle histogram array' \
  src/c/oh_fortran_v2.c
check_present 'oh3_init_raw requires a total particle array' \
  src/c/oh_fortran_v2.c
check_present 'null-pbuf-slot' tests/test_oh2_init_guard.c
check_present 'null-pbase-slot' tests/test_oh2_init_guard.c
check_present 'oh3-null-pbuf-slot' tests/test_oh2_init_guard.c
check_present 'oh3-null-pbase-slot' tests/test_oh2_init_guard.c
check_present 'negative-maxfrac' tests/test_oh2_init_guard.c
check_present 'oh3-negative-maxfrac' tests/test_oh2_init_guard.c
check_present 'maxlocal-negative-minmargin' tests/test_oh2_init_guard.c
check_present 'maxlocal-overflow-base' tests/test_oh2_init_guard.c
check_present 'maxlocal-overflow-margin' tests/test_oh2_init_guard.c
check_present 'raw-oh2-null-pbuf-slot' tests/test_oh2_init_guard.c
check_present 'raw-oh3-null-pbuf-slot' tests/test_oh2_init_guard.c
check_present 'raw-oh2-null-sdid' tests/test_oh2_init_guard.c
check_present 'raw-oh2-negative-maxfrac' tests/test_oh2_init_guard.c
check_present 'raw-oh3-negative-maxfrac' tests/test_oh2_init_guard.c
check_present 'raw-oh2-null-nphgram' tests/test_oh2_init_guard.c
check_present 'raw-oh2-null-totalp' tests/test_oh2_init_guard.c
check_present 'raw-oh3-null-sdid' tests/test_oh2_init_guard.c
check_present 'raw-oh3-null-nphgram' tests/test_oh2_init_guard.c
check_present 'raw-oh3-null-totalp' tests/test_oh2_init_guard.c
check_present 'run_oh3_init' tests/test_oh2_init_guard.c
check_present 'oh2_init(&sdid' tests/test_oh2_init_guard.c
check_present 'oh3_init(&sdid' tests/test_oh2_init_guard.c
check_present 'ownsTParticle' tests/test_oh_default_particle_type_ownership.c
check_present 'oh2_set_particle_mpi_type(MPI_DATATYPE_NULL)' \
  tests/test_oh_default_particle_type_ownership.c
check_present 'CustomTParticle == custom_type' \
  tests/test_oh_default_particle_type_ownership.c
check_present 'duplicated_adapter_type != custom_adapter_type' \
  tests/test_oh_default_particle_type_ownership.c
check_present 'run_default_oh2_init_with_custom_adapter' \
  tests/test_oh_default_particle_type_ownership.c
check_present 'ownsCustomTParticle' tests/test_oh_default_particle_type_ownership.c
check_present 'context->owned_custom_particle_adapter.mpi_type = MPI_DATATYPE_NULL' \
  src/c/oh_context.c
check_present 'adapter->owns_mpi_type' src/c/oh_fortran_v2.c
check_present 'oh2_set_particle_adapter_state' src/c/ohhelp2_internal.h
check_present 'MPI_Type_dup(adapter->mpi_type, &CustomTParticle)' \
  src/c/ohhelp2.c
check_present 'oh2_set_particle_mpi_type_state' src/c/ohhelp2_internal.h
check_present 'oh2_set_particle_adapter_state(context, adapter)' \
  src/c/oh_context.c
check_present 'oh2_set_particle_mpi_type_state(context, type)' \
  src/c/oh_context.c
check_present 'require_particle_buffer_unbound(context, "oh_context_set_particle_adapter()")' \
  src/c/oh_context.c
check_present 'require_particle_buffer_unbound(context, "oh_context_set_particle_mpi_type()")' \
  src/c/oh_context.c
check_present 'oh3_bind_context_particle_adapter(context)' \
  src/c/oh_context.c
check_present 'test_oh_context_particle_layout_guard\.c' \
  scripts/docker-build-test.sh
check_present 'expected adapter change while particles are bound to fail' \
  scripts/docker-build-test.sh
check_present 'expected MPI type change while particles are bound to fail' \
  scripts/docker-build-test.sh
check_present 'expected mismatched particle MPI datatype extent to fail' \
  scripts/docker-build-test.sh
check_present 'expected invalid adapter destination to fail' \
  scripts/docker-build-test.sh
check_present 'expected invalid adapter species to fail' \
  scripts/docker-build-test.sh
check_present 'expected invalid adapter species failure message' \
  scripts/docker-build-test.sh
check_present 'expected active invalid adapter destination in transbound2 to fail' \
  scripts/docker-build-test.sh
check_present 'expected active transbound2 invalid adapter destination failure message' \
  scripts/docker-build-test.sh
check_present 'expected active invalid adapter destination in transbound3 to fail' \
  scripts/docker-build-test.sh
check_present 'expected active transbound3 invalid adapter destination failure message' \
  scripts/docker-build-test.sh
check_present 'expected oversized species count to fail before allocation' \
  scripts/docker-build-test.sh
check_present 'species/node count exceeds Level 2 particle accounting capacity' \
  scripts/docker-build-test.sh
check_present 'expected capacity calculation overflow to fail before addition' \
  scripts/docker-build-test.sh
check_present 'out of virtual memory for Particles' scripts/docker-build-test.sh
check_present 'expected invalid C context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'expected negative C context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'expected NaN C context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'expected infinite C context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'region weight\[0\] must be finite and greater than zero' \
  scripts/docker-build-test.sh
check_present 'expected set_total_particles without accounting to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 field operation without fields to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 border exchange without exchanges to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 field config without ctypes to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 field config failure to report missing ctypes' \
  scripts/docker-build-test.sh
check_present 'expected injected particle index overflow to fail' \
  scripts/docker-build-test.sh
check_present 'expected injected particle index overflow to report contextual overflow' \
  scripts/docker-build-test.sh
check_present 'expected finalized injected particle remap to fail' \
  scripts/docker-build-test.sh
check_present 'expected finalized injected particle remap failure to report injected copy' \
  scripts/docker-build-test.sh
check_present '\$MPICC -Iinclude -Isrc/c tests/test_oh_context_particle_layout_guard\.c' \
  scripts/docker-build-test.sh
check_present 'unconfigured-bind' tests/test_oh_context_particle_layout_guard.c
check_present 'owned-region-nonnull' tests/test_oh_context_particle_layout_guard.c
check_present 'borrowed-region-null' tests/test_oh_context_particle_layout_guard.c
check_present 'get-region-null' tests/test_oh_context_particle_layout_guard.c
check_present 'owned-particle-nonnull' tests/test_oh_context_particle_layout_guard.c
check_present 'borrowed-particle-null' tests/test_oh_context_particle_layout_guard.c
check_present 'type-extent' tests/test_oh_context_particle_layout_guard.c
check_present 'invalid-destination' tests/test_oh_context_particle_layout_guard.c
check_present 'active-invalid-destination-transbound2' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'active-invalid-destination-transbound3' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'invalid-species' tests/test_oh_context_particle_layout_guard.c
check_present 'species-index-overflow' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'capacity-add-overflow' tests/test_oh_context_particle_layout_guard.c
check_present 'zero-weight' tests/test_oh_context_particle_layout_guard.c
check_present 'negative-region-weight' tests/test_oh_context_particle_layout_guard.c
check_present 'nan-region-weight' tests/test_oh_context_particle_layout_guard.c
check_present 'inf-region-weight' tests/test_oh_context_particle_layout_guard.c
check_present 'HUGE_VAL' tests/test_oh_context_particle_layout_guard.c
check_present 'inject-index-overflow' tests/test_oh_context_particle_layout_guard.c
check_present 'remap-null-injected-pointer' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remove-null-injected-pointer' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remap-interior-injected-pointer' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remove-interior-injected-pointer' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remap-active-particle' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remove-active-particle' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remap-finalized-injected-copy' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remove-finalized-injected-copy' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remap-original-injected-source' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remove-original-injected-source' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_inject_particle_get(context' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_remap_injected_particle(context, copy)' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_remove_injected_particle(context, copy)' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_remap_injected_particle(context, &injected)' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_remove_injected_particle(context, &injected)' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'remove-finalized-injected-copy' scripts/docker-build-test.sh
check_present 'remap-null-injected-pointer' scripts/docker-build-test.sh
check_present 'remove-null-injected-pointer' scripts/docker-build-test.sh
check_present 'remap-interior-injected-pointer' scripts/docker-build-test.sh
check_present 'remove-interior-injected-pointer' scripts/docker-build-test.sh
check_present 'remap-active-particle' scripts/docker-build-test.sh
check_present 'remove-active-particle' scripts/docker-build-test.sh
check_present 'remap-original-injected-source' scripts/docker-build-test.sh
check_present 'remove-original-injected-source' scripts/docker-build-test.sh
check_present 'guard_map_invalid_destination' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'sizeof(struct S_particle) \+ 8' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'owned-accounting-nonnull' tests/test_oh_context_particle_layout_guard.c
check_present 'borrowed-accounting-null' tests/test_oh_context_particle_layout_guard.c
check_present 'invalid-region-ownership' tests/test_oh_context_particle_layout_guard.c
check_present 'invalid-particle-ownership' tests/test_oh_context_particle_layout_guard.c
check_present 'invalid-accounting-ownership' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'accounting-null-nphgram-slot' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'accounting-null-totalp-slot' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'accounting-null-pbase-slot' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'borrowed-accounting-null-pbase' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'owned-accounting-nonnull-pbase' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'set-total-unbound' tests/test_oh_context_particle_layout_guard.c
check_present 'configure-zero-species' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'configure-negative-maxfrac' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'grid-size-null' tests/test_oh_context_particle_layout_guard.c
check_present 'level3-field-unconfigured' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'level3-exchange-unconfigured' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'level3-missing-ctypes' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'reconfigure-bound' tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_bind_particles(context' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_bind_particle_accounting(context' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_set_particle_adapter(context' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'oh_context_set_particle_mpi_type(context' \
  tests/test_oh_context_particle_layout_guard.c
check_present 'owned region id binding requires a NULL array' \
  src/c/oh_context.c
check_present 'invalid region id ownership flag' src/c/oh_context.c
check_present 'expected invalid region id ownership flag to fail' \
  scripts/docker-build-test.sh
check_present 'expected owned region id binding with non-NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected borrowed region id binding with NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected region id getter with NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected zero species C context configuration to fail' \
  scripts/docker-build-test.sh
check_present 'expected negative maxfrac C context configuration to fail' \
  scripts/docker-build-test.sh
check_present 'oh_context_configure_particles() requires maxfrac >= 0' \
  src/c/oh_context.c
check_present 'particle binding requires configured particles' src/c/oh_context.c
check_present 'expected particle binding before configure to fail' \
  scripts/docker-build-test.sh
check_present 'expected owned particle binding with non-NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected invalid particle ownership flag to fail' \
  scripts/docker-build-test.sh
check_present 'expected borrowed particle binding with NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected owned accounting binding with non-NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected invalid accounting ownership flag to fail' \
  scripts/docker-build-test.sh
check_present 'expected accounting binding with NULL nphgram slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected accounting binding with NULL totalp slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected borrowed accounting binding with NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected borrowed accounting binding with NULL pbase to fail' \
  scripts/docker-build-test.sh
check_present 'expected owned accounting binding with non-NULL pbase to fail' \
  scripts/docker-build-test.sh
check_present 'expected accounting binding with NULL pbase slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected set_total_particles without accounting to fail' \
  scripts/docker-build-test.sh
check_present 'expected grid size with NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'oh_context_grid_size() requires a size array' \
  src/c/oh_context.c
check_present 'expected particle reconfigure while particles are bound to fail' \
  scripts/docker-build-test.sh
check_present 'owned particle buffer binding requires a NULL buffer' \
  src/c/ohhelp2.c
check_present 'invalid particle buffer ownership flag' src/c/ohhelp2.c
check_present 'borrowed particle buffer binding requires a non-NULL buffer' \
  src/c/ohhelp2.c
check_present 'particle accounting is not bound' src/c/ohhelp1.c
check_present 'oh_particle_adapter_validate(&state->owned_particle_adapter)' \
  src/c/ohhelp2.c
check_present 'state_require_field_type' src/c/ohhelp3.c
check_present 'requires configured fields' src/c/ohhelp3.c
check_present 'state_require_exchange_type' src/c/ohhelp3.c
check_present 'requires configured boundary exchanges' src/c/ohhelp3.c
check_present 'Level 3 field configuration requires ctypes' src/c/ohhelp3.c
check_present 'particle MPI datatype extent must match particle stride' \
  src/c/ohhelp2.c
check_present 'state_checked_particle_destination' src/c/ohhelp2.c
check_present 'outside node range \[0,%d)' src/c/ohhelp2.c
check_present 'species/node count exceeds Level 2 particle accounting capacity' \
  src/c/oh_context.c
check_present 'oh_context_validate_species_node_capacity' src/c/oh_context.c
check_present 'request_slots = 4LL \* nnns \+ 2LL \* OH_NEIGHBORS' \
  src/c/oh_context.c
check_present 'request_slots > INT_MAX' src/c/oh_context.c
check_present 'invalid allocation size for %s' src/c/oh_context.c
check_present 'oh_context_validate_species_node_capacity(nn, nspec, "oh_init()")' \
  src/c/ohhelp1.c
check_present 'per_rank>(long long)INT_MAX-margin' src/c/oh_context.c
check_present 'owned particle accounting requires NULL nphgram/totalp' \
  src/c/oh_context.c
check_present 'invalid particle accounting ownership flag' src/c/oh_context.c
check_present 'borrowed particle accounting requires non-NULL arrays' \
  src/c/oh_context.c
check_present 'test_oh_context_particle_layout_guard_fortran\.F90' \
  scripts/docker-build-test.sh
check_present 'build_fortran_dimension_guard 1' scripts/docker-build-test.sh
check_present 'build_fortran_dimension_guard 2' scripts/docker-build-test.sh
check_present 'build/docker/dim1f' scripts/docker-build-test.sh
check_present 'build/docker/dim2f' scripts/docker-build-test.sh
check_present 'expected Fortran 2D missing y coordinate to fail' \
  scripts/docker-build-test.sh
check_present 'invalid-region-ownership' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'invalid-particle-ownership' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'invalid-accounting-ownership' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'expected Fortran adapter change while particles are bound to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran MPI type change while particles are bound to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran mismatched particle MPI datatype extent to fail' \
  scripts/docker-build-test.sh
check_present 'expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard_fortran' \
  scripts/docker-build-test.sh
check_present 'expected Fortran NULL context handle to fail' \
  scripts/docker-build-test.sh
check_present 'requires an associated context handle' scripts/docker-build-test.sh
check_present 'expected Fortran owned region id binding with non-NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran invalid region id ownership flag to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran owned particle binding with non-NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'owned particle buffer binding requires a NULL buffer' \
  scripts/docker-build-test.sh
check_present 'expected Fortran invalid particle ownership flag to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran borrowed particle binding with NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'borrowed particle buffer binding requires a non-NULL buffer' \
  scripts/docker-build-test.sh
check_present 'expected Fortran owned accounting binding with non-NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'owned particle accounting requires NULL nphgram/totalp' \
  scripts/docker-build-test.sh
check_present 'expected Fortran invalid accounting ownership flag to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran borrowed accounting binding with NULL storage to fail' \
  scripts/docker-build-test.sh
check_present 'borrowed particle accounting requires non-NULL arrays' \
  scripts/docker-build-test.sh
check_present 'expected Fortran particle binding before configure to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran negative maxfrac context configuration to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran Level 3 field operation without fields to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran Level 3 border exchange without exchanges to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran particle reconfigure while particles are bound to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran missing y coordinate to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran missing z coordinate to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran NULL context region weights to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran short region weights to fail' \
  scripts/docker-build-test.sh
check_present 'requires 1 weights for this context, got 0' \
  scripts/docker-build-test.sh
check_present 'expected Fortran short grid size array to fail' \
  scripts/docker-build-test.sh
check_present 'oh_context_grid_size requires at least OH_DIMENSION elements' \
  src/fortran/oh_v2.F90
check_present 'expected Fortran negative context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran negative context region weight failure message' \
  scripts/docker-build-test.sh
check_present 'expected Fortran zero context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran NaN context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran infinite context region weight to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran finalized injected particle remap to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran finalized injected particle remove to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran original injected source remap to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran original injected source remove to fail' \
  scripts/docker-build-test.sh
check_present 'OH_PARTICLES_OWNED' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'null-context' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'null-region-weights' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'short-weights' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'default-weights-before-init' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'default-weights-before-init' scripts/docker-build-test.sh
check_present 'short-grid-size' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'empty_grid_size' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'negative-region-weight' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'zero-region-weight' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'nan-region-weight' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'inf-region-weight' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'ieee_value' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'remap-finalized-injected-copy' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'remove-finalized-injected-copy' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'remap-original-injected-source' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'remove-original-injected-source' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_inject_particle_get(context' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_remap_injected_particle(context, copy_ptr)' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_remove_injected_particle(context, copy_ptr)' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'requires an associated context handle' src/c/oh_fortran_v2.c
check_present 'require_fortran_context' src/c/oh_fortran_v2.c
check_present 'weight_count != context->n_of_nodes' src/c/oh_fortran_v2.c
check_present 'oh_fortran_context_set_region_weights(oh_context *context,' \
  include/oh_fortran_v2.h
check_present 'integer(c_int), value :: weight_count' src/fortran/oh_v2.F90
check_present 'owned-particle' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'borrowed-particle-null' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'owned-accounting' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'borrowed-accounting-null' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'level3-field-unconfigured' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'level3-exchange-unconfigured' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'configure-negative-maxfrac' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_bcast_field(context, c_loc(field' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_exchange_borders(context, c_loc(field' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'missing-y' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'missing-z' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'missing-subdomain-y' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'missing-subdomain-z' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_bind_particles(context, particles' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_bind_particle_accounting(context, nphgram_ptr' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_set_particle_adapter(context)' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_set_particle_mpi_type(context' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'type-extent' tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_map_particle_to_neighbor requires y coordinate' \
  src/fortran/oh_v2.F90
check_present 'oh_context_map_particle_to_neighbor requires z coordinate' \
  src/fortran/oh_v2.F90
check_present 'oh_context_map_particle_to_subdomain requires y coordinate' \
  src/fortran/oh_v2.F90
check_present 'oh_context_map_particle_to_subdomain requires z coordinate' \
  src/fortran/oh_v2.F90
check_present 'expected Fortran missing subdomain y coordinate to fail' \
  scripts/docker-build-test.sh
check_present 'expected Fortran missing subdomain z coordinate to fail' \
  scripts/docker-build-test.sh
check_present 'oh_context_set_particle_mpi_type(context, int(MPI_BYTE' \
  tests/test_oh_context_particle_layout_guard_fortran.F90
check_present 'oh_context_unbind_particles()' \
  doc/v2/usage/v2-particle-and-weight.md
check_present 'Set the particle adapter or particle MPI datatype before binding' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'oh_context_unbind_particles()' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'oh3_particle_adapter_use_position_fields' \
  doc/v2/design/particle-adapter.md
check_present 'oh3_particle_adapter_use_position_fields' \
  doc/v2/usage/pic-lifecycle.md
check_present 'oh3_particle_adapter_use_position_fields' \
  doc/v2/usage/v2-particle-and-weight.md
check_present 'coordinate offsets without installing Level 3 mapping callbacks' \
  doc/v2/usage/v2-particle-and-weight.md
check_present 'oh_context_configure_particles()` before binding particles' \
  doc/v2/design/context.md
check_present 'Pass `NULL` when requesting owned storage' doc/v2/design/context.md
check_present 'cannot change species count while particles are bound' \
  src/c/oh_context.c
check_present 'cannot change species count while accounting is bound' \
  src/c/oh_context.c
check_present 'cannot change species count while Level 3 is configured' \
  src/c/oh_context.c
check_present 'For owned accounting arrays, pass `NULL` pointer slots' \
  doc/v2/design/context.md
check_present 'oh_context_configure_particles() first' \
  src/c/ohhelp3.c
check_absent 'Level 3 context configuration requires particles first' \
  src/c/ohhelp3.c
check_present 'run_context_reconfigure_particles_test' \
  tests/test_oh_context_lifecycle.c
check_present 'run_owned_particle_buffer_test' \
  tests/test_oh_context_lifecycle.c
check_present 'OH_PARTICLES_OWNED)' tests/test_oh_context_lifecycle.c
check_present 'particle_buffer_ownership == OH_PARTICLES_OWNED' \
  tests/test_oh_context_lifecycle.c
check_present 'run_context_particle_mpi_type_ownership_test' \
  tests/test_oh_context_lifecycle.c
check_present 'MPI_Type_free(&particle_type)' \
  tests/test_oh_context_lifecycle.c
check_absent 'default_context_or_stop' src/c/oh_context.c
check_absent 'only the default oh_context is implemented yet' src/c/oh_context.c
check_present 'oh1_stats_time_state(struct oh_state *state' \
  src/c/ohhelp1_internal.h
check_present 'oh1_stats_time_state(state, STATS_REBALANCE' src/c/ohhelp1.c
check_present 'oh1_stats_time_state(state, STATS_TB_COMM' src/c/ohhelp2.c
check_absent 'oh1_stats_time\(STATS_' src/c/ohhelp2.c
check_present 'context_x->stats_mode == 0' tests/test_oh_context_lifecycle.c
check_present 'OH_MODE_NORMAL_PRIMARY, 1' tests/test_oh_context_lifecycle.c
check_present 'void *oh2_inject_particle_state(struct oh_state *state, void *part)' \
  src/c/ohhelp2_internal.h
check_absent '^int[[:space:]]+transbound2_state\\(' src/c/ohhelp2.c
check_absent '^void[[:space:]]+\\*oh2_inject_particle_state\\(' src/c/ohhelp2.c
check_absent '^void[[:space:]]+oh2_remap_injected_particle_state\\(' \
  src/c/ohhelp2.c
check_absent '^void[[:space:]]+oh2_remove_injected_particle_state\\(' \
  src/c/ohhelp2.c
check_present 'dint inj = (dint)state->total_parts \+ state->n_of_injections' \
  src/c/ohhelp2.c
check_present 'inj<0 \|\| inj>INT_MAX \|\| inj>=state->n_of_local_particles_limit' \
  src/c/ohhelp2.c
check_present 'oh2_errstop_injection_overflow_state' src/c/ohhelp2.c
check_present 'local particle buffer overflow on rank' src/c/ohhelp2.c
check_present 'oh2_inject_particle_state: local particle buffer overflow' \
  scripts/docker-build-test.sh
check_absent 'state->total_parts \+ state->n_of_injections\+\+' \
  src/c/ohhelp2.c
check_absent 'oh2_inject_particle_state\(context, \(struct S_particle\*\)part\)' \
  src/c/oh_context.c
check_present 'state-backed internal entry points' doc/v2/design/architecture.md
check_absent '\b(Decl_Grid_Info|Subdomain_Id|Primarize_Id)\b' include/ohhelp2.h
check_absent '\b(nidelement|subdomid|gridmask|loggrid)\b' include/ohhelp2.h
check_absent '\b(init2|transbound2|exchange_primary_particles|move_to_sendbuf_primary|set_sendbuf_disps|exchange_particles)\s*\(' include/ohhelp2.h
check_present 'int *\*totalp, void *\*pbuf' src/c/ohhelp2_internal.h
check_present 'void *raw_pbuf = pbuf' src/c/ohhelp2.c
check_absent 'void *particle_ptr = pbuf \? *pbuf : NULL' src/c/oh_fortran_v2.c
check_present 'init2(&sdid_ptr, nspec, maxfrac, &nphgram_ptr, &totalp_ptr, pbuf' \
  src/c/oh_fortran_v2.c
check_absent 'struct S_particle \*\*pbuf' \
  src/c/ohhelp2.c src/c/ohhelp2_internal.h
check_absent '\(struct S_particle\*\*\)pbuf' src/c/ohhelp2.c
check_absent '^EXTERN ' include/ohhelp2.h
check_present 'void oh2_inject_particle(void *part)' include/ohhelp2.h
check_present 'void *oh2_inject_particle_get(void *part)' include/ohhelp2.h
check_present 'oh_inject_particle_get(A1)' include/ohhelp_c.h
check_absent 'oh_inject_particle_get' include/ohhelp_f.h
check_present 'oh_set_region_weights(A1)' include/ohhelp_c.h
check_present '#define oh_set_region_weights[[:space:]]+oh1_set_region_weights' \
  include/ohhelp_f.h
check_present '#define oh_init[[:space:]]+oh3_init' include/ohhelp_f.h
check_present 'oh1_set_region_weights' src/fortran/oh_mod1.F90
check_present 'real\*8,intent(in) :: weights(\*)' src/fortran/oh_mod1.F90
check_present 'oh_context_set_region_weights' include/oh_context.h
check_present '1\.0_c_double' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'oh_inject_particle_get' tests/test_ohhelp2_header.c
check_present 'oh_inject_particle_get' tests/test_ohhelp_c_header.c
check_present 'oh_set_region_weights' tests/test_ohhelp2_header.c
check_present 'oh_set_region_weights' tests/test_ohhelp_c_header.c
check_present 'oh_set_particle_position_fields' tests/test_ohhelp3_header.c
check_present 'oh_exchange_borders' tests/test_ohhelp3_header.c
check_present 'OH_DIMENSION=1 tests/test_ohhelp3_header\.c' \
  scripts/docker-build-test.sh
check_present 'OH_DIMENSION=2 tests/test_ohhelp3_header\.c' \
  scripts/docker-build-test.sh
check_present 'OH_DIMENSION=3 tests/test_ohhelp3_header\.c' \
  scripts/docker-build-test.sh
check_present 'level3_custom_particle\.c' scripts/docker-build-test.sh
check_present 'sample/sample\.F90' scripts/docker-build-test.sh
check_present 'docker-build-test\.sh' scripts/test.sh
check_present 'git diff --check' scripts/test.sh
check_present 'test_particle_contract_audit\.sh' scripts/test.sh
check_present 'bash tests/test_particle_contract_audit\.sh' scripts/test.sh
check_present 'OH_CHECK_V1_MARKDOWN' scripts/test.sh
check_present 'bash scripts/check-v1-markdown\.sh' scripts/test.sh
check_present 'OH_CHECK_FPM' scripts/test.sh
check_present 'bash scripts/fpm-build-test\.sh' scripts/test.sh
check_present 'bash scripts/docker-build-test\.sh' scripts/test.sh
check_present '"\$FPM" build --compiler "\$FC" --c-compiler "\$CC"' \
  scripts/fpm-build-test.sh
check_absent '"\$FPM" test' scripts/fpm-build-test.sh
check_present 'setup-fpm' .github/workflows/ci.yml
check_present 'bash scripts/test\.sh' .github/workflows/ci.yml
check_present 'convert_pdfs_to_md\.py' scripts/check-v1-markdown.sh
check_present '--out-dir' scripts/check-v1-markdown.sh
check_present '--out-dir' scripts/convert_pdfs_to_md.py
check_present 'Python 3\.10 or newer' scripts/convert_pdfs_to_md.py
check_present 'MIN_PYTHON = (3, 10)' scripts/convert_pdfs_to_md.py
check_absent '^import fitz$' scripts/convert_pdfs_to_md.py
check_present 'def open_pdf' scripts/convert_pdfs_to_md.py
check_present 'fitz\.open(pdf_path)' scripts/convert_pdfs_to_md.py
check_present 'sys.version_info >= (3, 10)' scripts/check-v1-markdown.sh
check_present "find_spec\(\"fitz\"\)" scripts/check-v1-markdown.sh
check_present "Python module 'fitz'" scripts/check-v1-markdown.sh
check_present 'mktemp -d' scripts/check-v1-markdown.sh
check_absent 'uncommitted changes' scripts/check-v1-markdown.sh
check_present 'requirements-doc\.txt' scripts/check-v1-markdown.sh
check_present 'requires Python >=3\.10' requirements-doc.txt
check_present 'check-v1-markdown\.sh' README.md
check_present 'Python 3\.10 or newer' README.md
check_present 'PYTHON=python3\.11' README.md
check_present 'MPIFC' README.md
check_present 'MPICC' README.md
check_present 'MPICC=\$\{MPICC:-mpicc\}' scripts/docker-build-test.sh
check_absent '^gcc ' scripts/docker-build-test.sh
check_present 'MPICXX' README.md
check_present 'MPIRUN' README.md
check_present 'TEST_TIMEOUT' README.md
check_present 'check-v1-markdown\.sh' doc/v1/README.md
check_present 'Python 3\.10 or newer' doc/v1/README.md
check_present 'PYTHON=python3\.11' doc/v1/README.md
check_present 'check-v1-markdown\.sh' doc/v1/markdown/README.md
check_present 'Python 3\.10 or newer' doc/v1/markdown/README.md
check_present 'PYTHON=python3\.11' doc/v1/markdown/README.md
check_present 'src/c/oh_fortran_v2\.c' scripts/docker-build-test.sh
check_present 'src/fortran/oh_v2\.F90' scripts/docker-build-test.sh
check_present 'C_CORE_OBJS=' scripts/docker-build-test.sh
check_present 'F_V2_CORE_OBJS=' scripts/docker-build-test.sh
check_present 'F_RAW_INIT_OBJS=' scripts/docker-build-test.sh
check_present 'tests/test_oh_v2_fortran\.F90' scripts/docker-build-test.sh
check_present 'tests/test_oh_v2_fortran_runtime\.F90' scripts/docker-build-test.sh
check_present 'run_mpi -n 1 build/docker/test_oh_v2_fortran_runtime' \
  scripts/docker-build-test.sh
check_present 'tests/test_oh_v2_fortran_raw_init_runtime\.F90' \
  scripts/docker-build-test.sh
check_present 'TEST_OH_RAW_LEVEL=2' scripts/docker-build-test.sh
check_present 'TEST_OH_RAW_LEVEL=3' scripts/docker-build-test.sh
check_present 'TEST_OH_RAW_MYCOMM=1' scripts/docker-build-test.sh
check_present 'TEST_OH_RAW_NULL_PBUF=1' scripts/docker-build-test.sh
check_present 'TEST_OH_RAW_NULL_PBASE=1' scripts/docker-build-test.sh
check_present 'TEST_OH_RAW_NULL_PCOORD=1' scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l2' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l3' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l2_null_pbuf' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l3_null_pbuf' \
  scripts/docker-build-test.sh
check_present 'expected Level 2 raw init with NULL pbase to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 raw init with NULL pbase to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 2 raw init with NULL pcoord to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 3 raw init with NULL pcoord to fail' \
  scripts/docker-build-test.sh
check_present 'oh2_init_raw requires a particle base array' src/c/oh_fortran_v2.c
check_present 'oh3_init_raw requires a particle base array' src/c/oh_fortran_v2.c
check_present 'oh2_init_raw requires a process grid array' src/c/oh_fortran_v2.c
check_present 'oh3_init_raw requires a process grid array' src/c/oh_fortran_v2.c
check_present 'run_mpi -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_mycomm' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_mycomm_l2' \
  scripts/docker-build-test.sh
check_present 'oh2_init_raw' tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'oh3_init_raw' tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'type(oh_mycomm_v2), target :: mycomm' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'mycomm_ptr = c_loc(mycomm)' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'pbuf = c_null_ptr' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'pbase_ptr = c_null_ptr' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'c_f_pointer(pbuf, active_particles' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'oh_context_unbind_particles(context)' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'oh_context_unbind_particle_accounting(context)' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'oh_set_particle_adapter' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present 'MPI_Comm_f2c' src/c/ohhelp1.c
check_present 'MPI_COMM_WORLD' src/c/ohhelp1.c
check_present 'MPI_Init' tests/test_oh_v2_fortran_runtime.F90
check_present 'oh_particle_adapter_validate' \
  tests/test_oh_v2_fortran_runtime.F90
check_present 'c_funloc(callback_get_region)' \
  tests/test_oh_v2_fortran_runtime.F90
check_present 'oh_particle_adapter_destroy(callback_adapter)' \
  tests/test_oh_v2_fortran_runtime.F90
check_present 'oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY' \
  tests/test_oh_v2_fortran_runtime.F90
check_present 'run_mpi -n 2 build/docker/test_oh_v2_fortran_runtime' \
  scripts/docker-build-test.sh
check_present 'callback_particles(1)%region = int(1 - rank, c_int)' \
  tests/test_oh_v2_fortran_runtime.F90
check_present 'callback_particles(1)%region /= int(rank, c_int)' \
  tests/test_oh_v2_fortran_runtime.F90
check_present 'tests/test_oh_context_lifecycle\.c' scripts/docker-build-test.sh
check_present 'OH_DIMENSION=1 -Iinclude -Isrc/c tests/test_oh_context_lifecycle\.c' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh_context_lifecycle_1d' \
  scripts/docker-build-test.sh
check_present 'OH_DIMENSION=2 -Iinclude -Isrc/c tests/test_oh_context_lifecycle\.c' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh_context_lifecycle_2d' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 3 build/docker/test_oh_context_lifecycle' \
  scripts/docker-build-test.sh
check_present 'nphgram\[0\] = 3' tests/test_oh_context_lifecycle.c
check_present 'double weights\[3\] = \{4\.0, 1\.0, 1\.0\}' \
  tests/test_oh_context_lifecycle.c
check_present 'totalp\[1\] == 1' tests/test_oh_context_lifecycle.c
check_present '#if OH_DIMENSION >= 3' src/c/ohhelp3.c
check_present 'run_localized_secondary_test' tests/test_oh_context_lifecycle.c
check_present 'test_oh_context_lifecycle_posaware' scripts/docker-build-test.sh
check_present 'posaware/test_oh_context_lifecycle_fortran' \
  scripts/docker-build-test.sh
check_present 'tests/test_oh_legacy_fortran_runtime\.F90' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 1 build/docker/test_oh_legacy_fortran_runtime' \
  scripts/docker-build-test.sh
check_present 'oh3_init' tests/test_oh_legacy_fortran_runtime.F90
check_present 'oh1_fam_comm' tests/test_oh_legacy_fortran_runtime.F90
check_present_re '^run_mpi\(\)[[:space:]]*\{' scripts/docker-build-test.sh
check_present 'timeout "\$TEST_TIMEOUT" "\$MPIRUN"' scripts/docker-build-test.sh
check_present 'MPIRUN_FLAGS' scripts/docker-build-test.sh
check_present 'MPIRUN_FLAGS: "--oversubscribe"' .github/workflows/ci.yml
check_absent '^int (gridMask|logGrid);' \
  tests/test_oh4_runtime_globals.c \
  tests/test_oh4p_runtime_smoke.c \
  tests/test_oh4p_capacity_guard.c \
  tests/test_oh4s_runtime_smoke.c \
  tests/test_oh4s_capacity_guard.c
check_present 'integer,intent(inout) :: nphgram(*)' src/fortran/oh_mod1.F90
check_present 'integer,intent(inout) :: nphgram(*)' src/fortran/oh_mod2.F90
check_present 'integer,intent(inout) :: nphgram(*)' src/fortran/oh_mod3.F90
check_present 'integer,intent(inout) :: sdoms(2,OH_DIMENSION,*)' \
  src/fortran/oh_mod3.F90
check_present_re '^expect_mpi_failure_log_contains\(\)[[:space:]]*\{' \
  scripts/docker-build-test.sh
check_present_re '^expect_mpi_failure_log_cases\(\)[[:space:]]*\{' \
  scripts/docker-build-test.sh
check_present 'expect_mpi_failure_log_cases requires MESSAGE CASE EXPECTED triples' \
  scripts/docker-build-test.sh
check_present 'local status' scripts/docker-build-test.sh
check_present 'set \+e' scripts/docker-build-test.sh
check_present 'status=\$\?' scripts/docker-build-test.sh
check_present 'set -e' scripts/docker-build-test.sh
check_present '\[ "\$status" -eq 0 \]' scripts/docker-build-test.sh
check_present '\[ "\$status" -eq 124 \]' scripts/docker-build-test.sh
check_present 'command timed out' scripts/docker-build-test.sh
check_present 'Segmentation fault|Bus error|Floating point exception|core dumped' \
  scripts/docker-build-test.sh
check_present 'Assertion failed|Assertion `|AddressSanitizer|UndefinedBehaviorSanitizer' \
  scripts/docker-build-test.sh
check_present 'command crashed before the expected diagnostic path' \
  scripts/docker-build-test.sh
check_present 'expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard' \
  scripts/docker-build-test.sh
check_absent '^expect_mpi_failure\(\)' scripts/docker-build-test.sh
check_absent '^expect_mpi_failure_cases' scripts/docker-build-test.sh
check_present 'grep -Fq -- "\$expected" "\$log_file"' scripts/docker-build-test.sh
check_present 'ffree-line-length-none' scripts/docker-build-test.sh
check_present 'EPSILON=1\.0d0,MU=1\.0d0' sample/sample.F90
check_present 'particle adapter handles' README.md
check_present 'oh2_init_raw().*oh3_init_raw()' README.md
check_present 'Fortran からも Level 1-4 は利用対象' doc/v2/usage/README.md
check_present 'ohhelp_v2` module' doc/v2/usage/README.md
check_present '任意の Fortran 粒子 layout' doc/v2/usage/README.md
check_present 'oh2_init_raw().*oh3_init_raw()' doc/v2/usage/README.md
check_present 'index-conventions\.md' doc/v2/README.md
check_present 'index-conventions\.md' doc/v2/usage/README.md
check_present 'Index Conventions' doc/v2/design/README.md
check_present 'pic-lifecycle-fortran\.md' doc/v2/usage/README.md
check_present 'api-by-level-fortran\.md' doc/v2/usage/README.md
check_present 'v2-particle-and-weight-fortran\.md' doc/v2/usage/README.md
check_present 'Fortran mirror' doc/v2/usage/pic-lifecycle.md
check_present 'C mirror' doc/v2/usage/pic-lifecycle-fortran.md
check_present 'Fortran mirror' doc/v2/usage/api-by-level.md
check_present 'C mirror' doc/v2/usage/api-by-level-fortran.md
check_present 'Fortran mirror' doc/v2/usage/v2-particle-and-weight.md
check_present 'C mirror' doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'Fortran では `ohhelp_v2` の opaque handle' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'oh2_init_raw().*oh3_init_raw()' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'zero-based `ftype` / `ctype` ids' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'この legacy helper では' doc/v2/usage/api-by-level-fortran.md
check_present '変換されません' doc/v2/usage/api-by-level-fortran.md
check_present 'does not promise full historical `oh3_init` field descriptor' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'recommended migration path for existing Fortran Level 3 callers' \
  doc/v2/usage/pic-lifecycle-fortran.md
check_present 'sample/sample\.F90' \
  doc/v2/usage/README.md
check_present 'oh_particle_adapter_create_byte' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'oh_context_bind_particle_accounting(ctx, nphgram_ptr, totalp_ptr' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'particles_ptr = c_loc(particles(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'particles_ptr = c_loc(particles(1))' \
  doc/v2/usage/pic-lifecycle-fortran.md
check_present 'nphgram_ptr = c_loc(nphgram(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'totalp_ptr = c_loc(totalp(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'pbase_ptr = c_loc(pbase(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'pcoord_ptr = c_loc(pcoord(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'scoord_ptr = c_loc(scoord(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'bcond_ptr = c_loc(bcond(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'ftypes_ptr = c_loc(ftypes(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'ctypes_ptr = c_loc(ctypes(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'fsizes_ptr = c_loc(fsizes(1))' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'oh_particle_adapter_destroy(adapter)' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'oh_particle_adapter_destroy(adapter)' \
  doc/v2/usage/pic-lifecycle-fortran.md
check_present 'call oh_particle_adapter_destroy(adapter)' \
  tests/test_oh_v2_fortran_raw_init_runtime.F90
check_present '`oh_context_set_particle_adapter()` and the default `oh_set_particle_adapter()`' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'copy the adapter state they need, including the MPI datatype' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'oh_particle_adapter_make_byte_type(sizeof(struct my_particle)' \
  doc/v2/usage/api-by-level.md
check_present 'oh_particle_adapter_make_byte_type(sizeof(struct my_particle)' \
  doc/v2/usage/pic-lifecycle.md
check_present 'oh_particle_adapter_make_byte_type(sizeof(struct my_particle)' \
  doc/v2/usage/v2-particle-and-weight.md
check_absent 'oh_default_particle_adapter\(MPI_DATATYPE_NULL\)' \
  doc/v2/usage/api-by-level.md doc/v2/usage/pic-lifecycle.md \
  doc/v2/usage/v2-particle-and-weight.md
check_present 'oh_context_inject_particle_get' \
  doc/v2/usage/pic-lifecycle-fortran.md
check_present 'callback adapter' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'custom particle layout の raw init' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'raw init の lifetime / ownership contract' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'mutable accounting state' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'borrowed buffer' doc/v2/usage/v2-particle-and-weight.md
check_present 'accounting arrays' doc/v2/usage/v2-particle-and-weight.md
check_present 'Particle Buffer Ownership' doc/v2/design/context.md
check_present 'nphgram.*totalp.*pbase' doc/v2/design/context.md
check_present 'bind/unbind' doc/v2/design/context.md
check_present 'Multiple independent OhHelp instances' \
  doc/v2/design/context.md
check_present 'OH_PARTICLES_BORROWED' include/oh_particle_ownership.h
check_present 'OH_PARTICLES_OWNED' include/oh_particle_ownership.h
check_present 'Heap contexts must be configured before binding particles' \
  include/oh_context.h
check_present 'Owned storage requires particles == NULL' include/oh_context.h
check_present 'Owned region ids require' include/oh_context.h
check_present 'all three slot addresses must' include/oh_context.h
check_present 'particle accounting binding requires a pbase slot' \
  src/c/oh_context.c
check_present 'address of each pointer variable' doc/v2/design/context.md
check_present 'oh_context_bind_particles' include/oh_context.h
check_present 'oh_context_create' include/oh_context.h
check_present 'oh_context_configure_particles' include/oh_context.h
check_present 'oh_context_bind_particle_accounting' include/oh_context.h
check_present 'oh_context_bind_region_ids' include/oh_context.h
check_present 'oh_context_get_region_ids' include/oh_context.h
check_present 'oh_context_max_local_particles_for_capacity' include/oh_context.h
check_present 'oh_context_bind_particles' src/c/oh_context.c
check_present 'oh_context_create' src/c/oh_context.c
check_present 'comm == MPI_COMM_NULL' src/c/oh_context.c
check_present 'run_context_create_invalid_comm_test' \
  tests/test_oh_context_lifecycle.c
check_present 'MPI_ERR_COMM' tests/test_oh_context_lifecycle.c
check_present 'MPI_Comm_dup(comm, &created->comm)' src/c/oh_context.c
check_present 'free_context_comm(context)' src/c/oh_context.c
check_present 'MPI_Finalized(&finalized)' src/c/oh_context.c
check_present 'free_border_exchange_types' src/c/ohhelp3.c
check_present 'if \(!initialized \|\| finalized\) return' src/c/ohhelp3.c
check_present 'destroy-border-after-finalize' tests/test_oh_context_lifecycle.c
check_present 'destroy-border-after-finalize' scripts/docker-build-test.sh
check_present 'context->particle_adapter->mpi_type != particle_type' \
  tests/test_oh_context_lifecycle.c
check_present 'context->use_custom_particle_adapter' \
  tests/test_oh_context_lifecycle.c
check_present 'owns_comm' src/c/oh_context_internal.h
check_present 'context_is_default(context)' src/c/oh_context.c
check_present 'context->region_id = RegionId' src/c/oh_context.c
check_present 'context->owns_region_id = 0' src/c/oh_context.c
check_present 'test_oh_context_default_region_ids\.c' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh_context_default_region_ids' \
  scripts/docker-build-test.sh
check_present 'test_oh_context_create_lifecycle\.c' \
  scripts/docker-build-test.sh
check_present 'test_oh_context_create_lifecycle before-init' \
  scripts/docker-build-test.sh
check_present 'test_oh_context_create_lifecycle after-finalize' \
  scripts/docker-build-test.sh
check_present 'fortran-before-init' scripts/docker-build-test.sh
check_present 'fortran-after-finalize' scripts/docker-build-test.sh
check_present 'oh_fortran_context_create(0, &context)' \
  tests/test_oh_context_create_lifecycle.c
check_present 'MPI_Initialized(&mpi_initialized)' src/c/oh_context.c
check_present 'MPI_Finalized(&mpi_finalized)' src/c/oh_context.c
check_present '!mpi_initialized || mpi_finalized' src/c/oh_context.c
check_present 'bound == RegionId' tests/test_oh_context_default_region_ids.c
check_present 'SubdomainId == RegionId' \
  tests/test_oh_context_default_region_ids.c
check_present 'context->owns_region_id == 0' \
  tests/test_oh_context_default_region_ids.c
check_present 'run_context_owned_comm_test' tests/test_oh_context_lifecycle.c
check_present 'MPI_Comm_free(&app_comm)' tests/test_oh_context_lifecycle.c
check_present 'MPI_Comm_dup(MPI_COMM_WORLD, app_comm' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'MPI_Comm_free(app_comm' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'MPI_Comm_free(&app_comm)' tests/test_public_headers_cxx.cpp
check_present 'duplicate the communicator' doc/v2/design/context.md
check_present 'before `MPI_Finalize()`' doc/v2/design/context.md
check_present 'oh_context_configure_particles' src/c/oh_context.c
check_present 'oh_context_bind_particle_accounting' src/c/oh_context.c
check_present 'oh_context_bind_region_ids' src/c/oh_context.c
check_present 'oh_context_get_region_ids' src/c/oh_context.c
check_present 'oh_context_max_local_particles_for_capacity' src/c/oh_context.c
check_present 'oh_context_configure_level3_legacy' src/fortran/oh_v2.F90
check_present 'oh_fortran_context_configure_level3_legacy' \
  include/oh_fortran_v2.h
check_present 'oh_fortran_context_configure_level3_legacy' \
  src/c/oh_fortran_v2.c
check_present 'oh_context_configure_level3_legacy' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'one-based boundary IDs' doc/v2/design/context.md
check_present 'OH_LIB_LEVEL_4P' src/c/ohhelp4p.c
check_present 'OH_LIB_LEVEL_4S' src/c/ohhelp4s.c
check_present 'fpm dependency builds' doc/v2/design/level-scope.md
check_present 'heap-owned context' doc/v2/usage/api-by-level.md
check_present 'heap-owned context' doc/v2/usage/api-by-level-fortran.md
check_present 'oh_context_bind_particles' src/fortran/oh_v2.F90
check_present 'oh_context_create' src/fortran/oh_v2.F90
check_present 'oh_context_configure_particles' src/fortran/oh_v2.F90
check_present 'oh_context_bind_particle_accounting' src/fortran/oh_v2.F90
check_present 'oh_context_bind_region_ids' src/fortran/oh_v2.F90
check_present 'oh_context_get_region_ids' src/fortran/oh_v2.F90
check_present 'oh_context_max_local_particles_for_capacity' src/fortran/oh_v2.F90
check_present 'oh_context_unbind_particles' src/fortran/oh_v2.F90
check_present 'oh_context_unbind_particle_accounting' src/fortran/oh_v2.F90
check_present 'oh_fortran_context_bind_particles' include/oh_fortran_v2.h
check_present 'oh_fortran_context_create' include/oh_fortran_v2.h
check_present 'oh_fortran_context_configure_particles' include/oh_fortran_v2.h
check_present 'oh_fortran_context_bind_particle_accounting' \
  include/oh_fortran_v2.h
check_present 'oh_fortran_context_bind_region_ids' include/oh_fortran_v2.h
check_present 'oh_fortran_context_get_region_ids' include/oh_fortran_v2.h
check_present 'oh_fortran_context_max_local_particles_for_capacity' \
  include/oh_fortran_v2.h
check_present 'test_oh_fortran_v2_header\.c' scripts/docker-build-test.sh
check_present 'oh_fortran_oh2_init_raw' tests/test_oh_fortran_v2_header.c
check_present 'oh_fortran_oh3_init_raw' tests/test_oh_fortran_v2_header.c
check_present 'oh_fortran_context_bind_particles' src/c/oh_fortran_v2.c
check_present 'oh_fortran_context_create' src/c/oh_fortran_v2.c
check_present 'MPI_Initialized(&mpi_initialized)' src/c/oh_fortran_v2.c
check_present '!mpi_initialized || mpi_finalized' src/c/oh_fortran_v2.c
check_present 'oh_fortran_context_configure_particles' src/c/oh_fortran_v2.c
check_absent 'oh_context_create\(owned_context, 0_c_int, ierr\)' \
  tests/test_oh_v2_fortran.F90
check_present 'Must be called after' include/oh_context.h
check_present 'MPI_ERR_OTHER outside the MPI' include/oh_context.h
check_present 'pbuf is an inout pointer slot' include/oh_fortran_v2.h
check_present 'sdid, nphgram, totalp, pbase, and pcoord must point at real storage' \
  include/oh_fortran_v2.h
check_present 'pbuf is an inout pointer slot' src/fortran/oh_v2.F90
check_present 'requests legacy' src/fortran/oh_v2.F90
check_present 'oh_particle_adapter_set_species_base' include/oh_particle_adapter.h
check_present 'has_position_fields' include/oh_particle_adapter.h
check_present 'has_position_fields' src/c/oh_particle_adapter.c
check_present 'position_offset\[dim\]' src/c/oh_particle_adapter.c
check_present 'MPI_Finalized(&mpi_finalized)' src/c/oh_particle_adapter.c
check_present 'if (!mpi_initialized) return 0' src/c/oh_particle_adapter.c
check_present 'return MPI_ERR_OTHER' src/c/oh_particle_adapter.c
check_present 'before-init' tests/test_oh_particle_adapter.c
check_present 'after-finalize' tests/test_oh_particle_adapter.c
check_present 'assert(!oh_particle_adapter_validate(&adapter))' \
  tests/test_oh_particle_adapter.c
check_present 'oh_particle_adapter_position(&adapter, &particle, 1) == 0' \
  tests/test_oh_particle_adapter.c
check_present 'field_range_valid(adapter->stride, adapter->position_offset' \
  src/c/oh_particle_adapter.c
check_present 'adapter->map_to_neighbor == int_field_map_to_region' \
  src/c/oh_particle_adapter.c
check_present 'adapter->map_to_subdomain == int_field_map_to_region' \
  src/c/oh_particle_adapter.c
check_present 'test_oh_particle_adapter before-init' scripts/docker-build-test.sh
check_present 'test_oh_particle_adapter after-finalize' scripts/docker-build-test.sh
check_present 'mpi_active && bx\[e\]\[1\]\[d\]\[lu\]\.send\.deriv' \
  src/c/ohhelp3.c
check_present 'adapter\.set_region(&adapter, &wide_particle, -2, 0)' \
  tests/test_oh_particle_adapter.c
check_present 'oh_particle_adapter_use_single_species_integer_region' \
  tests/test_oh_particle_adapter.c
check_present 'adapter.get_region = default_adapter.get_region' \
  tests/test_oh_particle_adapter.c
check_present 'oh_particle_adapter_use_position_fields(&adapter, (size_t)-1' \
  tests/test_oh_particle_adapter.c
check_present 'sizeof(struct S_particle)' tests/test_oh_particle_adapter.c
check_present 'tests/test_oh_particle_adapter_callbacks\.c' \
  scripts/docker-build-test.sh
check_present 'OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS' \
  tests/test_oh_particle_adapter_callbacks.c
check_present 'OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING' \
  tests/test_oh_particle_adapter_callbacks.c
check_present 'oh_particle_adapter_use_single_species_integer_region' \
  tests/test_oh_particle_adapter_callbacks.c
check_present 'tests/test_oh_particle_buffer\.c' scripts/docker-build-test.sh
check_present 'adapter\.stride = sizeof(struct padded_particle)' \
  tests/test_oh_particle_buffer.c
check_present 'oh_particle_buffer_index(0, source_base, second) == -1' \
  tests/test_oh_particle_buffer.c
check_present 'oh_particle_buffer_index(&adapter, 0, second) == -1' \
  tests/test_oh_particle_buffer.c
check_present 'oh_particle_buffer_index(&adapter, source_base, 0) == -1' \
  tests/test_oh_particle_buffer.c
check_present 'oh_particle_buffer_index(&adapter, second, source_base) == -1' \
  tests/test_oh_particle_buffer.c
check_present 'adapter.stride = 0' tests/test_oh_particle_buffer.c
check_present 'oh_particle_buffer_index_bounded(&adapter, source_base, 3' \
  tests/test_oh_particle_buffer.c
check_present 'oh_particle_buffer_index_bounded(&adapter, source_base, 1' \
  tests/test_oh_particle_buffer.c
check_present 'index > (uintptr_t)INT_MAX' src/c/oh_particle_buffer.h
check_present 'oh_particle_buffer_index_bounded' src/c/oh_particle_buffer.h
check_present 'UINTPTR_MAX / (uintptr_t)stride' src/c/oh_particle_buffer.h
check_present 'UINTPTR_MAX - base_addr' src/c/oh_particle_buffer.h
check_present 'state_injected_particle_index' src/c/ohhelp2.c
check_present 'oh_particle_buffer_index_bounded(state->particle_adapter' \
  src/c/ohhelp2.c
check_present 'oh_particle_buffer_index_bounded(state->particle_adapter' \
  src/c/ohhelp4_particle.h
check_present 'if (!adapter || !base || !part) return -1' \
  src/c/oh_particle_buffer.h
check_present 'oh_particle_buffer_at' tests/test_oh_particle_buffer.c
check_present 'oh_particle_buffer_copy_n' tests/test_oh_particle_buffer.c
check_present 'field_range_valid' src/c/oh_particle_adapter.c
check_present 'sizeof(wide_particle), sizeof(wide_particle\.region)' \
  tests/test_oh_particle_adapter.c
check_present '(size_t)-1, sizeof(wide_particle\.region)' \
  tests/test_oh_particle_adapter.c
check_present 'Weights must be finite and greater than zero' \
  doc/v2/design/load-balancing.md
check_present 'tests/test_oh_load_balance\.c' scripts/docker-build-test.sh
check_present 'build/docker/test_oh_load_balance' scripts/docker-build-test.sh
check_present 'oh_region_load' tests/test_oh_load_balance.c
check_present 'oh_region_load(2, DBL_MAX) == DBL_MAX' \
  tests/test_oh_load_balance.c
check_present 'oh_load_limit(DBL_MAX, 20, 1) == DBL_MAX' \
  tests/test_oh_load_balance.c
check_present 'oh_particles_for_load' tests/test_oh_load_balance.c
check_present 'oh_weighted_transfer_count' tests/test_oh_load_balance.c
check_present 'oh_region_weight_is_valid' tests/test_oh_load_balance.c
check_present 'oh_region_weights_use_weighted_mode' tests/test_oh_load_balance.c
check_present 'Passing `NULL` resets all region' \
  doc/v2/design/load-balancing.md
check_present '`size(weights)` must match the number of regions/nodes in the context' \
  doc/v2/design/load-balancing.md
check_present 'oh_context_region_count(ctx)' doc/v2/design/load-balancing.md
check_present 'oh_context_is_level3_configured(ctx)' \
  doc/v2/design/load-balancing.md
check_present 'oh_context_region_count' include/oh_context.h
check_present 'oh_context_is_level3_configured' include/oh_context.h
check_present 'oh_context_set_region_weights_n' include/oh_context.h
check_present 'oh_context_set_region_weights_n' src/c/oh_context.c
check_present 'oh_context_set_region_weights_n(context, weights' \
  tests/test_public_headers_cxx.cpp
check_present 'short-weights-n' tests/test_oh_context_particle_layout_guard.c
check_present 'short-weights-n' scripts/docker-build-test.sh
check_present 'oh_fortran_context_region_count' include/oh_fortran_v2.h
check_present 'oh_fortran_context_is_level3_configured' include/oh_fortran_v2.h
check_present 'oh_context_region_count(context)' tests/test_oh_v2_fortran.F90
check_present 'oh_context_is_level3_configured(context)' \
  tests/test_oh_v2_fortran.F90
check_present '`size(weights)` must match the number of regions/nodes in the context' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'count-equivalent compatibility path' \
  doc/v2/design/load-balancing.md
check_present 'Weighted mode deliberately does not use the old stable secondary shortcut yet' \
  doc/v2/design/load-balancing.md
check_present 'rounds up the receiver.s load deficit' \
  doc/v2/design/load-balancing.md
check_present 'oh_fortran_particle_adapter_set_species_base' \
  include/oh_fortran_v2.h
check_present 'oh3_particle_adapter_use_neighbor_position_fields' include/ohhelp3.h
check_present 'oh3_particle_adapter_use_subdomain_position_fields' include/ohhelp3.h
check_present 'oh_fortran_particle_adapter_use_level3_neighbor_position_fields' \
  include/oh_fortran_v2.h
check_present 'oh_fortran_particle_adapter_use_level3_subdomain_position_fields' \
  include/oh_fortran_v2.h
check_present 'oh_particle_adapter_use_level3_neighbor_position_fields' \
  src/fortran/oh_v2.F90
check_present 'oh_particle_adapter_use_level3_subdomain_position_fields' \
  src/fortran/oh_v2.F90
check_present 'oh_particle_adapter_set_species_base(unwrap_adapter(adapter), 1)' \
  src/c/oh_fortran_v2.c
check_absent 'context->spec_base = 1' src/c/oh_fortran_v2.c
check_present 'single_species' src/c/ohhelp2.c
check_present 'species -= species_base' src/c/ohhelp2.c
check_present 'oh_fortran_context_bind_particle_accounting' \
  src/c/oh_fortran_v2.c
check_present 'oh_fortran_context_bind_region_ids' src/c/oh_fortran_v2.c
check_present 'oh_fortran_context_get_region_ids' src/c/oh_fortran_v2.c
check_present 'oh_fortran_context_max_local_particles_for_capacity' \
  src/c/oh_fortran_v2.c
check_present 'particle_buffer_bound' src/c/oh_context_internal.h
check_present 'particle_accounting_bound' src/c/oh_context_internal.h
check_present 'oh2_bind_particle_buffer_state' src/c/ohhelp2_internal.h
check_present 'particle buffer is not bound' src/c/ohhelp2.c
check_present 'particle accounting is not bound' src/c/ohhelp1.c
check_present 'oh3_init_raw' src/fortran/oh_v2.F90
check_present 'oh_fortran_oh3_init_raw' src/c/oh_fortran_v2.c
check_present 'oh3_init_raw' tests/test_oh_v2_fortran.F90
check_present 'oh_particle_adapter_use_level3_position_fields' \
  doc/v2/usage/api-by-level-fortran.md
check_order doc/v2/usage/api-by-level-fortran.md \
  'oh_particle_adapter_use_level3_position_fields' \
  'oh_context_set_particle_adapter'
check_present '`pbuf` は inout pointer slot' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'oh_particle_adapter_set_callbacks' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'position fields do not replace integer-region `map_to_subdomain`' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'oh_context_inject_particle_get()` が' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'module ohhelp_v2' src/fortran/oh_v2.F90
check_present 'type :: oh_context_handle' src/fortran/oh_v2.F90
check_present 'type :: oh_particle_adapter_handle' src/fortran/oh_v2.F90
check_present 'oh_particle_adapter_set_callbacks' src/fortran/oh_v2.F90
check_present 'oh_fortran_context_set_particle_adapter' src/c/oh_fortran_v2.c
check_present 'oh_fortran_particle_adapter_use_level3_position_fields' \
  src/c/oh_fortran_v2.c
check_present 'oh_fortran_particle_field_offset' src/c/oh_fortran_v2.c
check_present 'oh_set_particle_position_fields' sample/level3_custom_particle.c
check_present 'oh_particle_adapter_use_integer_fields' \
  sample/level3_custom_particle.c
check_present 'v2_context_level2_custom_particle\.c' \
  scripts/docker-build-test.sh
check_present 'v2_context_level2_custom_particle\.F90' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 1 build/docker/v2_context_level2_custom_particle' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/v2_context_level2_custom_particle' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 1 build/docker/v2_context_level2_custom_particle_fortran' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/v2_context_level2_custom_particle_fortran' \
  scripts/docker-build-test.sh
check_present 'v2 heap context と custom' doc/v2/usage/README.md
check_present 'sample/sample\.F90` は legacy' doc/v2/usage/README.md
check_present 'oh_context_create' sample/v2_context_level2_custom_particle.c
check_present 'oh_context_configure_particles' \
  sample/v2_context_level2_custom_particle.c
check_present 'oh_context_set_particle_adapter' \
  sample/v2_context_level2_custom_particle.c
check_present 'MPI_Type_free(&particle_type)' \
  sample/v2_context_level2_custom_particle.c
check_present 'oh_context_bind_particle_accounting' \
  sample/v2_context_level2_custom_particle.c
check_present 'oh_context_transbound2' \
  sample/v2_context_level2_custom_particle.c
check_present 'use ohhelp_v2' sample/v2_context_level2_custom_particle.F90
check_present 'oh_particle_field_offset' \
  sample/v2_context_level2_custom_particle.F90
check_present 'oh_particle_adapter_use_integer_fields' \
  sample/v2_context_level2_custom_particle.F90
check_present 'oh_particle_adapter_set_species_base(adapter, 1_c_int)' \
  sample/v2_context_level2_custom_particle.F90
check_present 'oh_context_set_particle_adapter' \
  sample/v2_context_level2_custom_particle.F90
check_present 'oh_particle_adapter_destroy(adapter)' \
  sample/v2_context_level2_custom_particle.F90
check_present 'oh_context_bind_particle_accounting' \
  sample/v2_context_level2_custom_particle.F90
check_present 'oh_context_transbound2' \
  sample/v2_context_level2_custom_particle.F90
check_present 'integer(c_long_long) :: region' \
  sample/v2_context_level2_custom_particle.F90
check_present 'c_sizeof(particle%region)' \
  sample/v2_context_level2_custom_particle.F90
check_present 'oh_context_set_region_weights' tests/test_oh_context_header.c
check_present 'oh_context_bind_particles' tests/test_oh_context_header.c
check_present 'oh_context_create' tests/test_oh_context_header.c
check_present 'oh_context_configure_particles' tests/test_oh_context_header.c
check_present 'oh_context_create' tests/test_oh_context_lifecycle.c
check_present 'oh_context_configure_particles' tests/test_oh_context_lifecycle.c
check_present 'oh_context_destroy' tests/test_oh_context_lifecycle.c
check_present 'run_region_weight_copy_reset_test' tests/test_oh_context_lifecycle.c
check_present 'run_region_weights_context_isolation_test' \
  tests/test_oh_context_lifecycle.c
check_present 'weightedLoadBalancing == legacy_weighted_load_balancing' \
  tests/test_oh_context_lifecycle.c
check_present 'run_weighted_load_rebalance_test' tests/test_oh_context_lifecycle.c
check_present 'run_weighted_load_level1_level2_api_test' \
  tests/test_oh_context_lifecycle.c
check_present 'run_callback_only_adapter_transbound_test' \
  tests/test_oh_context_lifecycle.c
check_present 'run_callback_only_adapter_transbound_test(rank, n, 0)' \
  tests/test_oh_context_lifecycle.c
check_present 'run_callback_only_adapter_transbound_test(rank, n, 1)' \
  tests/test_oh_context_lifecycle.c
check_present 'OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(callback_particle' \
  tests/test_oh_context_lifecycle.c
check_present 'OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING(callback_particle' \
  tests/test_oh_context_lifecycle.c
check_present 'adapter\.get_region = callback_particle_get_region;' \
  tests/test_oh_context_lifecycle.c
check_present 'adapter\.set_region = callback_particle_set_region;' \
  tests/test_oh_context_lifecycle.c
check_present 'adapter\.get_species = callback_particle_get_species;' \
  tests/test_oh_context_lifecycle.c
check_present 'adapter\.map_to_neighbor = callback_particle_map_to_neighbor;' \
  tests/test_oh_context_lifecycle.c
check_present 'adapter\.map_to_subdomain = callback_particle_map_to_subdomain' \
  tests/test_oh_context_lifecycle.c
check_present 'callback_particle_map_to_subdomain);' \
  tests/test_oh_context_lifecycle.c
check_present 'oh_context_transbound2(context, OH_MODE_NORMAL_PRIMARY, 0)' \
  tests/test_oh_context_lifecycle.c
check_present 'oh_context_transbound3(context, OH_MODE_NORMAL_PRIMARY, 0)' \
  tests/test_oh_context_lifecycle.c
check_present 'particles\[0\]\.logical_region == rank' \
  tests/test_oh_context_lifecycle.c
check_present 'particles\[0\]\.marker == 1000\.0 \+ (double)(1 - rank)' \
  tests/test_oh_context_lifecycle.c
check_present 'nodes\[rank\]\.parentid == 0' tests/test_oh_context_lifecycle.c
check_present 'nodes\[rank\]\.get\.sec == 1' tests/test_oh_context_lifecycle.c
check_present 'n_of_local_load_max == 3\.0' tests/test_oh_context_lifecycle.c
check_present 'try_stable1_state(context, OH_MODE_NORMAL_SECONDARY, 1, 0)' \
  tests/test_oh_context_lifecycle.c
check_present 'OH_MODE_REBALANCE_SECONDARY' tests/test_oh_context_lifecycle.c
check_present 'oh_context_transbound3(context, OH_MODE_REBALANCE_SECONDARY, 0)' \
  tests/test_oh_context_lifecycle.c
check_present 'OH_MODE_NORMAL_PRIMARY' tests/test_oh_context_lifecycle.c
check_present 'oh_context_bind_particles' tests/test_oh_context_lifecycle.c
check_present 'oh_context_bind_region_ids' tests/test_oh_context_lifecycle.c
check_present 'run_weighted_load_rebalance_test' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_broadcast(context_x' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_all_reduce(context_x' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_reduce(context_x' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_set_region_weights(context, weights)' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'run_region_weight_reset_behavior_test' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'call oh_context_set_region_weights(context)' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'pbase_values(3) /= 1_c_int' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'run_legacy_passive_bounds_test' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'run_injected_accounting_contract_test' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_inject_particle_get(context' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_remove_injected_particle(context' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_remap_injected_particle(context' \
  tests/test_oh_context_lifecycle_fortran.F90
check_order tests/test_oh_context_lifecycle.c \
  'oh_context_remove_injected_particle(context, copy)' \
  'oh_context_remap_injected_particle(context, copy)'
check_order tests/test_oh_context_lifecycle_fortran.F90 \
  'call oh_context_remove_injected_particle(context, copy_ptr)' \
  'call oh_context_remap_injected_particle(context, copy_ptr)'
check_present 'bounds = 1_c_int' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'mapped = oh_context_map_particle_to_neighbor' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'pbase\[2\] == 2' tests/test_oh_context_lifecycle.c
check_present 'pbase_values(3) /= 2_c_int' \
  tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_bind_particle_accounting' \
  tests/test_oh_context_header.c
check_present 'oh_context_bind_region_ids' tests/test_oh_context_header.c
check_present 'oh_context_get_region_ids' tests/test_oh_context_header.c
check_present 'oh_context_max_local_particles_for_capacity' \
  tests/test_oh_context_header.c
check_present 'oh_context_bind_particles' tests/test_oh_v2_fortran.F90
check_present 'oh_context_create' tests/test_oh_v2_fortran_runtime.F90
check_present 'oh_context_configure_particles' tests/test_oh_v2_fortran_runtime.F90
check_present 'oh_context_bind_particle_accounting' \
  tests/test_oh_v2_fortran.F90
check_present 'oh_context_bind_region_ids' tests/test_oh_v2_fortran.F90
check_present 'oh_context_get_region_ids' tests/test_oh_v2_fortran.F90
check_present 'oh_context_remove_injected_particle(context, injected)' \
  tests/test_oh_v2_fortran.F90
check_present 'oh_context_max_local_particles_for_capacity' \
  tests/test_oh_v2_fortran.F90
check_present 'oh_context_bind_region_ids' tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_get_region_ids' tests/test_oh_context_lifecycle_fortran.F90
check_present 'oh_context_transbound3' tests/test_oh_context_header.c
check_present 'test_public_headers_cxx\.cpp' scripts/docker-build-test.sh
check_present 'test_public_headers_cxx_link' scripts/docker-build-test.sh
check_present 'run_mpi -n 1 build/docker/test_public_headers_cxx_link' \
  scripts/docker-build-test.sh
check_present 'test_public_headers_cxx_4p\.o' scripts/docker-build-test.sh
check_present 'test_public_headers_cxx_4s\.o' scripts/docker-build-test.sh
check_present 'tests/test_public_header_reinclude\.c' scripts/docker-build-test.sh
check_present '#include "oh_config\.h"' tests/test_public_header_reinclude.c
check_present '#include "ohhelp_c\.h"' tests/test_public_header_reinclude.c
check_present '#include "ohhelp4p\.h"' tests/test_public_header_reinclude.c
check_present '#include "ohhelp4s\.h"' tests/test_public_header_reinclude.c
check_present 'tests/test_ohhelp_f_reinclude\.c' scripts/docker-build-test.sh
check_present '#include "ohhelp_f\.h"' tests/test_ohhelp_f_reinclude.c
check_present 'tests/test_ohhelp4p_header\.c' scripts/docker-build-test.sh
check_present 'oh_per_grid_histogram' tests/test_ohhelp4p_header.c
check_present 'oh_remove_mapped_particle' tests/test_ohhelp4p_header.c
check_present 'oh_remap_particle_to_neighbor' tests/test_ohhelp4p_header.c
check_present 'tests/test_ohhelp4s_header\.c' scripts/docker-build-test.sh
check_present 'oh_particle_buffer' tests/test_ohhelp4s_header.c
check_present 'oh_per_grid_histogram' tests/test_ohhelp4s_header.c
check_present 'OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS' \
  tests/test_public_headers_cxx.cpp
check_present 'MPI_Comm comm = MCW' tests/test_public_headers_cxx.cpp
check_present 'MPI_Init' tests/test_public_headers_cxx.cpp
check_present 'oh_context_create' tests/test_public_headers_cxx.cpp
check_present 'oh_context_transbound2' tests/test_public_headers_cxx.cpp
check_present 'oh3_particle_adapter_use_position_fields' \
  tests/test_public_headers_cxx.cpp
check_present 'test_ohhelp_f_preprocess\.F90' scripts/docker-build-test.sh
check_present 'test_ohhelp_f_pp_l1\.o' scripts/docker-build-test.sh
check_present 'test_ohhelp_f_pp_4s\.o' scripts/docker-build-test.sh
check_present '#ifndef oh_transbound' tests/test_ohhelp_f_preprocess.F90
check_present 'oh_context_inject_particle_get' tests/test_oh_context_header.c
check_present 'oh_context_map_particle_to_neighbor' tests/test_oh_context_header.c
check_present 'oh_context_exchange_borders' tests/test_oh_context_header.c
check_present 'oh_particle_adapter_use_integer_fields' include/oh_particle_adapter.h
check_present 'oh_particle_adapter_use_single_species_integer_region' \
  include/oh_particle_adapter.h
check_present 'oh_particle_adapter_use_integer_fields' \
  doc/v2/design/particle-adapter.md
check_present 'oh_particle_adapter_use_integer_fields' \
  tests/test_oh_particle_adapter.c
check_present 'oh_particle_adapter_use_integer_fields' \
  doc/v2/usage/api-by-level.md
check_present 'OH_BIG_SPACE' doc/v2/usage/v2-particle-and-weight.md
check_present 'int *\*totalp, void *\*pbuf' include/ohhelp2.h

# Level 3 should route particle coordinates through the adapter contract.
check_absent '->(x|y|z)\b' src/c/ohhelp3.c
check_absent '->(nid|spec)\b' src/c/ohhelp3.c
check_absent '^EXTERN ' include/ohhelp3.h
check_absent '\b(init3|set_field_descriptors|clear_border_exchange|map_irregular_subdomain)\s*\(' include/ohhelp3.h
check_present 'int \*\*rcounts, int \*\*scounts, void \*\*pbuf' \
  src/c/ohhelp3_internal.h
check_present 'void \*raw_pbuf = pbuf' src/c/ohhelp3.c
check_present 'init3\(&sdid_ptr, nspec, maxfrac, &nphgram_ptr, &totalp_ptr, NULL, NULL,' \
  src/c/oh_fortran_v2.c
check_absent 'struct S_particle \*\*pbuf' \
  src/c/ohhelp3.c src/c/ohhelp3_internal.h
check_absent '\(struct S_particle\*\*\)pbuf' src/c/ohhelp3.c
check_present 'void \*\*pbuf' include/ohhelp3.h
check_absent '\bParticleAdapter\.map_to_(neighbor|subdomain)\b' \
  src/c/ohhelp3.c

# Level 4 still has packed-id semantics, but implementation code must reach
# them through local helpers so the representation can later move behind the
# adapter contract.
check_present 'tests/test_oh4p_capacity_guard\.c' scripts/docker-build-test.sh
check_present 'tests/test_oh4p_runtime_smoke\.c' scripts/docker-build-test.sh
check_present 'src/c/ohhelp1\.c src/c/ohhelp2\.c src/c/ohhelp3\.c src/c/ohhelp4p\.c' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4p_runtime_smoke' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4p_runtime_smoke custom-adapter' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4p_runtime_smoke weighted-load' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4p_runtime_smoke weighted-secondary' \
  scripts/docker-build-test.sh
check_present 'tests/test_oh4p_fortran_runtime\.F90' scripts/docker-build-test.sh
check_present 'tests/test_oh4_runtime_globals\.c' scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4p_fortran_runtime' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4p_fortran_runtime weighted-secondary' \
  scripts/docker-build-test.sh
check_present 'type(oh_particle),intent(inout) :: pbuf(*)' src/fortran/oh_mod4p.F90
check_present 'integer,intent(inout) :: sdoms(2,OH_DIMENSION,*)' \
  src/fortran/oh_mod4p.F90
check_present 'integer,intent(in)    :: ctypes(3,2,nbound,*)' \
  src/fortran/oh_mod4p.F90
check_present 'tests/test_oh4s_runtime_smoke\.c' scripts/docker-build-test.sh
check_present 'src/c/ohhelp1\.c src/c/ohhelp2\.c src/c/ohhelp3\.c src/c/ohhelp4s\.c' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4s_runtime_smoke' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4s_runtime_smoke custom-adapter' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4s_runtime_smoke weighted-load' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4s_runtime_smoke weighted-secondary' \
  scripts/docker-build-test.sh
check_present 'tests/test_oh4s_fortran_runtime\.F90' scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4s_fortran_runtime' \
  scripts/docker-build-test.sh
check_present 'run_mpi -n 2 build/docker/test_oh4s_fortran_runtime weighted-secondary' \
  scripts/docker-build-test.sh
check_present 'type(oh_particle), intent(inout) :: pbuf(*)' src/fortran/oh_mod4s.F90
check_present 'integer, intent(inout) :: sdoms(2, OH_DIMENSION, *)' \
  src/fortran/oh_mod4s.F90
check_present 'integer, intent(in)    :: ctypes(3, 2, nbound, *)' \
  src/fortran/oh_mod4s.F90
check_present 'strcmp(argv\[1\], "custom-adapter")' \
  tests/test_oh4p_runtime_smoke.c
check_present 'strcmp(argv\[1\], "weighted-load")' \
  tests/test_oh4p_runtime_smoke.c
check_present 'strcmp(argv\[1\], "weighted-secondary")' \
  tests/test_oh4p_runtime_smoke.c
check_present 'run_weighted_load_path' tests/test_oh4p_runtime_smoke.c
check_present 'oh1_set_region_weights\(weights\)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_transbound\(OH_MODE_NORMAL_PRIMARY, 0\)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'OH_MODE_REBALANCE_SECONDARY' tests/test_oh4p_runtime_smoke.c
check_present 'totalp\[1\] == 1' tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_transbound\(OH_MODE_REBALANCE_SECONDARY, 0\)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_transbound\(OH_MODE_REBALANCE_SECONDARY, 0\)' \
  tests/test_oh4p_fortran_runtime.F90
check_present 'call oh4p_per_grid_histogram\(pghgram\)' \
  tests/test_oh4p_fortran_runtime.F90
check_present 'int\* allocated = NULL' src/c/ohhelp4p.c
check_absent 'level4_fail_if_weighted_secondary_transbound' \
  src/c/ohhelp4p.c
check_present 'oh2_set_particle_adapter(adapter)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh2_set_particle_adapter(NULL)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'struct level4_custom_particle' tests/test_oh4p_runtime_smoke.c
check_present 'oh_particle_adapter_use_integer_fields' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh_particle_adapter_use_position_fields' \
  tests/test_oh4p_runtime_smoke.c
check_present 'strcmp(argv\[1\], "custom-adapter")' \
  tests/test_oh4s_runtime_smoke.c
check_present 'strcmp(argv\[1\], "weighted-load")' \
  tests/test_oh4s_runtime_smoke.c
check_present 'strcmp(argv\[1\], "weighted-secondary")' \
  tests/test_oh4s_runtime_smoke.c
check_present 'run_weighted_load_path' tests/test_oh4s_runtime_smoke.c
check_present 'oh1_set_region_weights\(weights\)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_transbound\(OH_MODE_NORMAL_PRIMARY, 0\)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'OH_MODE_REBALANCE_SECONDARY' tests/test_oh4s_runtime_smoke.c
check_present 'totalp\[1\] == 2' tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_transbound\(OH_MODE_REBALANCE_SECONDARY, 0\)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_transbound\(OH_MODE_REBALANCE_SECONDARY, 0\)' \
  tests/test_oh4s_fortran_runtime.F90
check_present 'call oh4s_per_grid_histogram\(pghgram, pgindex\)' \
  tests/test_oh4s_fortran_runtime.F90
check_present 'allocated_hgram' src/c/ohhelp4s.c
check_present 'allocated_index' src/c/ohhelp4s.c
check_present 'src_boundary_condition\[d\]\[side\] - boundary_base' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'level4_fail_if_weighted_secondary_transbound' \
  src/c/ohhelp4s.c src/c/ohhelp4_internal.h
check_present 'oh2_set_particle_adapter(adapter)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh2_set_particle_adapter(NULL)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'struct level4_custom_particle' tests/test_oh4s_runtime_smoke.c
check_present 'oh_particle_adapter_use_integer_fields' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh_particle_adapter_use_position_fields' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4p_per_grid_histogram(&pghgram)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_map_particle_to_subdomain(&particles\[0\], 0, 0)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_transbound(OH_MODE_NORMAL_PRIMARY, 0)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_inject_particle(&injected, 0)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'active_particles = particles \+ maxlocalp' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_remove_mapped_particle(&active_particles\[injected_index\], 0, 0)' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4p_remap_particle_to_subdomain' \
  tests/test_oh4p_runtime_smoke.c
check_present 'active_particles\[injected_index\]\.trace_id == 1000 \+ rank' \
  tests/test_oh4p_runtime_smoke.c
check_present 'oh4s_particle_buffer(maxlocalp, &pbuf)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_per_grid_histogram(&pghgram, &pgindex)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_map_particle_to_subdomain(&particles\[0\], 0, 0)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_transbound(OH_MODE_NORMAL_PRIMARY, 0)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_inject_particle(&injected, 0)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'active_particles = particles \+ maxlocalp' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_remove_mapped_particle(&active_particles\[injected_index\], 0, 0)' \
  tests/test_oh4s_runtime_smoke.c
check_present 'oh4s_remap_particle_to_subdomain' \
  tests/test_oh4s_runtime_smoke.c
check_present 'active_particles\[injected_index\]\.trace_id == 1000 \+ rank' \
  tests/test_oh4s_runtime_smoke.c
check_present 'bcond\[6\].*1, 1, 1, 1, 1, 1' \
  tests/test_oh4s_runtime_smoke.c
check_present 'maxlocalp == 56' tests/test_oh4s_runtime_smoke.c
check_present 'cbufsize == 6' tests/test_oh4s_runtime_smoke.c
check_present 'pbase\[1\] == 2' tests/test_oh4s_runtime_smoke.c
check_present 'pbase\[2\] == 2' tests/test_oh4s_runtime_smoke.c
check_present 'totalp\[0\] == 2' tests/test_oh4s_runtime_smoke.c
check_present 'struct oh_state\* state = oh1_state()' src/c/ohhelp4p.c
check_present 'struct oh_state\* state = oh1_state()' src/c/ohhelp4s.c
check_present 'int\*\* totalp, void\*\* pbuf' src/c/ohhelp4p.c
check_present 'void\* raw_pbuf = pbuf' src/c/ohhelp4p.c
check_present 'level4_init_particle_stride()' src/c/ohhelp4p.c
check_present 'SendBuf = level4_init_particle_at(Particles, maxlocalp)' \
  src/c/ohhelp4p.c
check_absent '\(struct S_particle\*\*\)pbuf' src/c/ohhelp4p.c
check_present 'level4_send_counts' src/c/ohhelp4p.c
check_present 'level4_send_counts' src/c/ohhelp4s.c
check_present 'Level 4 send count exceeds send buffer' src/c/ohhelp4p.c
check_present 'Level 4p is a default-context API' include/ohhelp4p.h
check_present 'Level 4s is a default-context API' include/ohhelp4s.h
check_present 'intentionally no context-owned Level 4 transbound API' \
  include/oh_context.h
check_present 'default-context state bridge' doc/v2/usage/api-by-level.md
check_present 'default-context state bridge' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'default-context state bridge' doc/v2/design/level-scope.md
check_absent 'not heap-owned context support' doc/v2/design/level-scope.md
check_absent 'not a guarantee that non-uniform' \
  doc/v2/design/level-scope.md
check_present 'runtime coverage' doc/v2/design/level-scope.md
check_present 'weighted primary and secondary transbound' \
  doc/v2/design/level-scope.md
check_present 'Weighted secondary transbound' \
  doc/v2/design/level-scope.md
check_present 'supported v2 API' doc/v2/usage/api-by-level.md
check_absent 'migration smoke for scheduling only' \
  tests/test_oh4p_runtime_smoke.c tests/test_oh4s_runtime_smoke.c
check_absent 'oh_context_transbound4' include/oh_context.h src/fortran/oh_v2.F90
check_present 'expected Level 4p zero hotspot threshold to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 4p oversized hotspot threshold to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 4p oversized hotspot threshold failure message' \
  scripts/docker-build-test.sh
check_present 'expected Level 4p doubled particle capacity overflow to fail' \
  scripts/docker-build-test.sh
check_present 'out of virtual memory for Particles' scripts/docker-build-test.sh
check_present 'hotspot threshold (%d) should be greater than 0' \
  src/c/ohhelp4p.c
check_present 'hsthresh > INT_MAX / 2' src/c/ohhelp4p.c
check_present 'npl > INT_MAX / 2' src/c/ohhelp4p.c
check_present 'maxlocalp > INT_MAX / 2' src/c/ohhelp4p.c
check_present 'maxlocalp \* 2' src/c/ohhelp4p.c
check_present 'gridOverflowLimit = hsthresh * 2' src/c/ohhelp4p.c
check_present 'test_oh4s_capacity_guard\.c' scripts/docker-build-test.sh
check_present 'expected Level 4s NULL particle pointer slot to fail' \
  scripts/docker-build-test.sh
check_present 'expected Level 4s oversized maxlocalp to fail' \
  scripts/docker-build-test.sh
check_present 'compatibility shim while Level 4' src/c/ohhelp4s.c
check_present 'maxlocalp=0' src/c/ohhelp4s.c
check_present '&pbufdummyptr' src/c/ohhelp4s.c
check_present 'oh_particle_buffer_stride(state->particle_adapter)' \
  src/c/ohhelp4s.c
check_present 'oh4s_particle_buffer() requires a particle pointer slot' \
  src/c/ohhelp4s.c
check_present 'maxlocalp > INT_MAX / 2' src/c/ohhelp4s.c
check_present 'maxlocalp \* 2' src/c/ohhelp4s.c
check_present 'state->send_buffer = SendBuf =' src/c/ohhelp4s.c
check_present 'oh_particle_buffer_at(state->particle_adapter' \
  src/c/ohhelp4s.c
check_signature_exact 'state_xfer_particles4p' src/c/ohhelp4p.c \
  'static void state_xfer_particles4p(struct oh_state* state, const int trans, const int psnew, void* sbuf)'
check_signature_exact 'state_xfer_particles4s' src/c/ohhelp4s.c \
  'static void state_xfer_particles4s(struct oh_state* state, const int trans, const int psnew, const int nextmode, void* sbuf)'
check_function_absent 'count_population' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'sort_particles' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'move_and_sort_primary' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'sort_received_particles' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'move_to_sendbuf_sec4p' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'move_to_sendbuf_uw4p' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'move_to_sendbuf_dw4p' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'move_and_sort_secondary' src/c/ohhelp4p.c 'struct S_particle'
check_function_absent 'check_particle_location4p' src/c/ohhelp4p.c \
  'struct S_particle'
check_function_absent 'oh4p_map_particle_to_neighbor' src/c/ohhelp4p.c \
  'struct S_particle'
check_function_absent 'oh4p_map_particle_to_subdomain' src/c/ohhelp4p.c \
  'struct S_particle'
check_function_absent 'oh4p_inject_particle' src/c/ohhelp4p.c \
  'struct S_particle'
check_function_absent 'oh4p_remove_mapped_particle' src/c/ohhelp4p.c \
  'struct S_particle'
check_function_absent 'count_population' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'rebalance4s' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'move_to_sendbuf_4s' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'move_to_sendbuf_uw4s' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'move_to_sendbuf_dw4s' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'sort_particles' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'move_and_sort' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'sort_received_particles' src/c/ohhelp4s.c 'struct S_particle'
check_function_absent 'xfer_boundary_particles_v' src/c/ohhelp4s.c \
  'struct S_particle'
check_function_absent 'check_particle_location4s' src/c/ohhelp4s.c \
  'struct S_particle'
check_function_absent 'oh4s_map_particle_to_neighbor' src/c/ohhelp4s.c \
  'struct S_particle'
check_function_absent 'oh4s_map_particle_to_subdomain' src/c/ohhelp4s.c \
  'struct S_particle'
check_function_absent 'oh4s_inject_particle' src/c/ohhelp4s.c \
  'struct S_particle'
check_function_absent 'oh4s_remove_mapped_particle' src/c/ohhelp4s.c \
  'struct S_particle'
check_absent '\((const )?struct S_particle\*\)particle' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\(struct S_particle\*\)(\*pbuf|mem_alloc)' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\(struct S_particle\*\)(particles|mem_alloc)' src/c/ohhelp2.c
check_absent '\(struct S_particle\*\*\)' src/c/ohhelp2.c src/c/oh_context.c
check_absent '\(void\*\*\)RecvBufBases' src/c/ohhelp2.c src/c/oh_context.c
check_present 'void\* rbuf = state->recv_buffer_bases' src/c/ohhelp4p.c
check_present 'void\* rbuf = state->recv_buffer_bases' src/c/ohhelp4s.c
check_present 'static inline void\*' src/c/ohhelp4_particle.h
check_present 'level4_particle_index(struct oh_state\* state, const void\* part)' \
  src/c/ohhelp4_particle.h
check_present 'level4_particle_region(struct oh_state\* state, const void\* part' \
  src/c/ohhelp4_particle.h
check_present 'level4_particle_position(struct oh_state\* state, void\* part' \
  src/c/ohhelp4_particle.h
check_present 'level4_copy_particle(struct oh_state\* state, void\* dst, const void\* src)' \
  src/c/ohhelp4_particle.h
check_present 'level4_copy_particle_to_buffer(struct oh_state\* state, void\* base' \
  src/c/ohhelp4_particle.h
check_absent 'static inline struct S_particle\*' src/c/ohhelp4_particle.h
check_present 'zero-hotspot' tests/test_oh4p_capacity_guard.c
check_present 'overflow-hotspot' tests/test_oh4p_capacity_guard.c
check_present 'overflow-doubled-capacity' tests/test_oh4p_capacity_guard.c
check_present 'null-pbuf-slot' tests/test_oh4s_capacity_guard.c
check_present 'overflow-maxlocalp' tests/test_oh4s_capacity_guard.c
check_present 'particle species %d is outside configured range' \
  src/c/ohhelp4_particle.h
check_present 'species < 0 \|\| species >= state->n_of_species' \
  src/c/ohhelp4_particle.h
check_present 'dint inj = (dint)state->total_parts \+ state->n_of_injections' \
  src/c/ohhelp4p.c
check_present 'dint inj = (dint)state->total_parts \+ state->n_of_injections' \
  src/c/ohhelp4s.c
check_present 'inj < 0 \|\| inj > INT_MAX \|\| inj >= state->n_of_local_particles_limit' \
  src/c/ohhelp4p.c
check_present 'inj < 0 \|\| inj > INT_MAX \|\| inj >= state->n_of_local_particles_limit' \
  src/c/ohhelp4s.c
check_present 'maxdensity <= 0' src/c/ohhelp4s.c
check_present '2 * (dint)maxdensity' src/c/ohhelp4s.c
check_present 'mem_alloc_error("BoundarySendBuf", 0)' src/c/ohhelp4s.c
check_present 'mem_alloc_error("CellBuffer", 0)' src/c/ohhelp4s.c
check_absent '->(nid|spec)\b' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(part|p|P|sp)->(x|y|z)\b' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'mem_alloc\(sizeof\(struct S_particle\)' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'state->particles\s*(\[|\+)' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'state->send_buffer\s*(\[|\+)' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(Particles|SendBuf)\s*(\[|\+)' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\bsb\s*\[[^]]+\]\s*=' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'struct S_particle' src/c/oh_particle_buffer.h
check_present 'void *' src/c/oh_particle_buffer.h
check_absent '\b(p|part|sp|sb|rbuf)\s*\[[^]]+\]\.(nid|spec|x|y|z)\b' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(nidelement|subdomid|gridmask)\b' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(Grid_Position|Combine_Subdom_Pos|Primarize_Id_Only|Secondarize_Id|Secondary_Injected|Neighbor_Subdomain_Id)\b' \
  include/ohhelp4p.h include/ohhelp4s.h
check_absent '^#(undef|define) (Grid_Position|Combine_Subdom_Pos|Subdomain_Id|Primarize_Id|Primarize_Id_Only|Secondarize_Id|Secondary_Injected|Neighbor_Subdomain_Id)' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(Grid_Position|Combine_Subdom_Pos|Subdomain_Id|Primarize_Id|Primarize_Id_Only|Secondarize_Id|Secondary_Injected|Neighbor_Subdomain_Id|Local_Grid_Position)\b' \
    src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\bDecl_Grid_Info\s*\(' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'struct S_grid\* Grid = state->grid' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'int \(\*SubDomains\).*state->subdomains' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent 'int \(\*Boundaries\).*state->boundaries' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(ParticleAdapter|gridmask|loggrid)\b' \
  include/ohhelp4p.h include/ohhelp4s.h
check_absent '^EXTERN ' include/ohhelp4p.h include/ohhelp4s.h
check_present 'oh4p_map_particle_to_neighbor(void\* part' include/ohhelp4p.h
check_present 'oh4s_map_particle_to_neighbor(void\* part' include/ohhelp4s.h
check_present 'oh4s_particle_buffer(const int maxlocalp, void\*\* pbuf)' \
  include/ohhelp4s.h
check_absent '\b(try_primary2|try_stable2|rebalance2|move_to_sendbuf_secondary)\s*\(' \
  src/c/ohhelp2.c
check_absent '\b(move_to_sendbuf_primary|exchange_primary_particles|set_sendbuf_disps|exchange_particles)\s*\(' \
  src/c/ohhelp2.c src/c/ohhelp2_internal.h
check_absent '\b(set_total_particles|try_primary1|try_stable1|rebalance1|build_new_comm)\s*\(' \
  src/c/ohhelp1.c src/c/ohhelp1_internal.h
check_absent '\b(transbound1|transbound2)\s*\(' \
  src/c/ohhelp1.c src/c/ohhelp1_internal.h src/c/ohhelp2.c src/c/ohhelp2_internal.h
check_absent '\b(set_field_descriptors|clear_border_exchange)\s*\(' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(try_primary1|try_stable1|rebalance1|build_new_comm|oh1_broadcast|move_to_sendbuf_primary|exchange_primary_particles|set_sendbuf_disps|exchange_particles)\s*\(' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\boh1_stats_time\s*\(' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_present 'oh1_stats_time_state(state, STATS_TB_SORT' src/c/ohhelp4p.c
check_present 'oh1_stats_time_state(state, STATS_TB_MOVE' src/c/ohhelp4p.c
check_present 'oh1_stats_time_state(state, STATS_TB_SORT' src/c/ohhelp4s.c
check_present 'oh1_stats_time_state(state, STATS_TB_MOVE' src/c/ohhelp4s.c
check_absent '\b(exchange_xfer_amount|xfer_particles)\s*\(' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\btransbound1\s*\(' src/c/ohhelp3.c src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\btransbound2\s*\(' src/c/ohhelp3.c
check_absent '\boh1_(broadcast|reduce|all_reduce)\s*\(' src/c/ohhelp3.c
check_absent '\b(RecvCounts|SendCounts)\b' src/c/ohhelp2.c src/c/ohhelp3.c \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '\b(NOfSend|NOfRecv)\b' src/c/ohhelp4p.c src/c/ohhelp4s.c
check_absent '=\s*(Boundaries|SubDomains|Grid)\s*;' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_present 'try_primary1_state' src/c/ohhelp1_internal.h
check_present 'transbound1_state' src/c/ohhelp1_internal.h
check_present 'transbound2_state' src/c/ohhelp2_internal.h
check_present 'oh2_inject_particle_state' src/c/ohhelp2_internal.h
check_present 'oh3_transbound_state' src/c/ohhelp3_internal.h
check_present 'oh3_map_particle_to_neighbor_state' src/c/ohhelp3_internal.h
check_present 'oh3_map_particle_to_subdomain_state' src/c/ohhelp3_internal.h
check_present 'oh3_exchange_borders_state' src/c/ohhelp3_internal.h
check_present 'oh1_broadcast_state' src/c/ohhelp1_internal.h
check_present 'oh1_reduce_state' src/c/ohhelp1_internal.h
check_present 'oh1_all_reduce_state' src/c/ohhelp1_internal.h
check_present 'exchange_particles_state' src/c/ohhelp2_internal.h

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
    || true
)
if [ -n "$legacy_accesses" ]; then
  fail "legacy particle-field access escaped the documented migration boundary"
  echo "$legacy_accesses" >&2
fi

# Keep the hidden Level-4 and injection semantics explicitly documented until
# they are replaced by a v2 adapter contract.
check_present 'nid < 0' doc/v2/design/particle-adapter.md
check_present 'nid == -2' doc/v2/design/particle-adapter.md
check_present 'map_to_subdomain()' doc/v2/design/particle-adapter.md
check_present '\[0, n_of_nodes)' doc/v2/design/particle-adapter.md
check_present 'oh_remove_injected_particle()' doc/v2/design/particle-adapter.md
check_present 'oh_inject_particle_get()' doc/v2/design/particle-adapter.md
check_present '`type(c_ptr)` handle through the v2 Fortran module' \
  doc/v2/usage/pic-lifecycle.md
check_absent 'oh_context_inject_particle\(ctx, &particle\)' \
  doc/v2/usage/pic-lifecycle.md
check_absent 'oh_context_inject_particle\(ctx, part\)' \
  doc/v2/usage/v2-particle-and-weight.md
check_absent 'call oh_context_inject_particle\(ctx, particle_ptr\)' \
  doc/v2/usage/pic-lifecycle-fortran.md \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_absent 'oh_context_inject_particle\(context, particle\)' \
  tests/test_oh_context_header.c
check_present 'Typed access is recovered' \
  doc/v2/usage/v2-particle-and-weight-fortran.md
check_present 'remap_injected_particle()` is additive' \
  doc/v2/design/particle-adapter.md
check_present 'remap_injected_particle()` is additive' \
  doc/v2/usage/pic-lifecycle.md
check_present 'remap_injected_particle()` is additive' \
  doc/v2/usage/pic-lifecycle-fortran.md
check_present '二重計上' doc/v2/usage/v2-particle-and-weight.md
check_present 'remap is additive' AGENTS.md
check_present 'Legacy v1/v1-compatible sample' sample/sample.c
check_present 'Legacy v1/v1-compatible sample' sample/sample.F90
check_present 'sample/sample.c' scripts/docker-build-test.sh
check_present 'build/docker/sample_c.o' scripts/docker-build-test.sh
check_present 'make -n -f sample/samplec.mk clean' scripts/docker-build-test.sh
check_present 'make -n -f sample/samplef.mk clean' scripts/docker-build-test.sh
check_present 'default-context' sample/level3_custom_particle.c
check_present 'migration shim' sample/level3_custom_particle.c
check_present 'set_total_particles_state().*finalizes pending' \
  doc/v2/design/particle-adapter.md
check_present 'packed-grid id operations' doc/v2/design/particle-adapter.md
check_present 'adapter-strided particle storage or cursors' \
  doc/v2/design/particle-adapter.md
check_present 'migration-boundary handles for helper and MPI calls only' \
  doc/v2/design/particle-adapter.md
check_present 'level4_secondary_region_offset' src/c/ohhelp4_particle.h
check_present 'OH_LEVEL4_PARTICLE_REMOVED' src/c/ohhelp4_particle.h
check_present 'OH_LEVEL4_BOUNDARY_EXCHANGE_MARKER' src/c/ohhelp4_particle.h
check_present 'level4_mark_particle_removed' src/c/ohhelp4_particle.h
check_present 'level4_mark_boundary_exchange_particle' src/c/ohhelp4_particle.h
check_absent 'level4_set_particle_region\(.*,[[:space:]]*-[12][[:space:]]*,' \
  src/c/ohhelp4p.c src/c/ohhelp4s.c
check_present 'level4_bind_common_state' src/c/ohhelp4_state.h
check_present 'level4p_bind_state' src/c/ohhelp4p_state.h
check_present 'level4s_bind_state' src/c/ohhelp4s_state.h
check_present 'default context layout' doc/v2/design/particle-adapter.md
check_present 'primary_or_secondary' doc/v2/design/particle-adapter.md
check_present 'negative-id convention' doc/v2/design/particle-adapter.md
check_present '-(node \+ 1)' doc/v2/design/particle-adapter.md
check_present 'Neighbor_Id' doc/v2/design/particle-adapter.md
check_present 'AbsNeighbors' doc/v2/design/particle-adapter.md
check_present 'specBase = 1' doc/v2/design/particle-adapter.md
check_present 'outside configured range' src/c/ohhelp2.c
check_present 'particle species %d is outside configured range' \
  src/c/ohhelp2.c
check_present 'single_species' src/c/ohhelp4_particle.h
check_present 'species -= species_base' \
  src/c/ohhelp4_particle.h
check_present 'InjectedParticles' doc/v2/design/particle-adapter.md
check_present 'OH_BIG_SPACE' doc/v2/design/particle-adapter.md
check_present 'boundary plane thickness' doc/v2/design/particle-adapter.md
check_present 'no current `original` particle field' doc/v2/design/particle-adapter.md
check_absent '(->|\.)original\b|\boriginal_(offset|particle|particles|field)\b' \
  include/*.h src/c/*.c src/c/*.h src/fortran/*.F90
check_present '粒子ポインタを `void *`' doc/v2/usage/v2-particle-and-weight.md
check_present 'oh_context_bind_region_ids' doc/v2/design/context.md
check_present 'zero-based field descriptor index' \
  doc/v2/design/index-conventions.md
check_present 'context_ftype = ftype - 1_c_int' \
  doc/v2/design/index-conventions.md
check_present 'oh_context_configure_level3_legacy' \
  doc/v2/design/index-conventions.md
check_present 'not a full `oh3_init` field descriptor compatibility layer' \
  doc/v2/design/index-conventions.md
check_present 'does not preserve the full historical Fortran field descriptor' \
  doc/v2/design/context.md
check_present 'pbase\(2\).*secondary split' doc/v2/usage/api-by-level-fortran.md
check_present 'pbase\(3\).*total local particle count' \
  doc/v2/usage/api-by-level-fortran.md
check_present 'buffer capacity' doc/v2/usage/api-by-level-fortran.md
check_present 'oh_context_max_local_particles_for_capacity' \
  doc/v2/usage/api-by-level.md
check_present 'oh_context_bind_region_ids' doc/v2/usage/api-by-level.md
check_present 'oh_context_max_local_particles_for_capacity' \
  doc/v2/usage/pic-lifecycle-fortran.md
check_present 'ctype.*field-operation `ftype` values are zero-based' \
  doc/v2/usage/pic-lifecycle-fortran.md
check_present 'oh_context_get_region_ids' doc/v2/usage/pic-lifecycle.md
check_absent '\bp\.nid\s*=' doc/v2/usage/api-by-level.md doc/v2/usage/pic-lifecycle.md
check_present 'level4_secondary_injected' src/c/ohhelp4_particle.h
check_present 'state->field_types =' src/c/ohhelp3.c
check_present 'FieldTypes = ft' src/c/ohhelp3.c

if [ "$failures" -ne 0 ]; then
  exit 1
fi
