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

check_present '#ifndef OHHELP_C_H' include/ohhelp_c.h
check_present '#ifndef OH_CONFIG_H' include/oh_config.h
check_present '#ifndef OHHELP_F_H' include/ohhelp_f.h
check_present '#ifndef OHHELP1_H' include/ohhelp1.h
check_present '#ifndef OHHELP2_H' include/ohhelp2.h
check_present '#ifndef OHHELP3_H' include/ohhelp3.h
check_present '#ifndef OHHELP4P_H' include/ohhelp4p.h
check_present '#ifndef OHHELP4S_H' include/ohhelp4s.h
check_present '#include "ohhelp1.h"' include/ohhelp_c.h
check_present '#include "ohhelp2.h"' include/ohhelp_c.h
check_present '#include "ohhelp3.h"' include/ohhelp_c.h
check_absent '^(void|int)\s+oh[0-9]' include/ohhelp_c.h

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
check_present 'void \*particles;' src/c/oh_context_internal.h
check_present 'void \*send_buffer;' src/c/oh_context_internal.h
check_present 'void \*\*recv_buffer_bases;' src/c/oh_context_internal.h
check_present 'void \*level4_boundary_send_buffer;' src/c/oh_context_internal.h
check_present 'int \*recv_counts;' src/c/oh_context_internal.h
check_present 'int \*send_counts;' src/c/oh_context_internal.h
check_absent 'struct S_particle \*particles;' src/c/oh_context_internal.h
check_absent 'struct S_particle \*send_buffer;' src/c/oh_context_internal.h
check_absent 'struct S_particle \*\*recv_buffer_bases;' \
  src/c/oh_context_internal.h
check_absent 'struct S_particle \*level4_boundary_send_buffer;' \
  src/c/oh_context_internal.h
check_absent 'struct S_(node|heap|commlist|commsched_context|comms|statscurr|statstime|statspart|statstotal|stats)\s*\{' include/ohhelp1.h
check_absent '\b(init1|mem_alloc|mem_alloc_error|errstop|local_errstop|set_total_particles|transbound1|try_primary1|try_stable1|rebalance1|build_new_comm|vprint|dprint)\s*\(' include/ohhelp1.h
check_present 'oh1_comm' include/ohhelp1.h
check_absent 'return\(accMode\)' src/c/ohhelp1.c
check_absent '->(nid|spec)\b' src/c/ohhelp2.c
check_present 'OH_nid_t region' src/c/ohhelp2.c
check_present 'state_mark_particle_removed' src/c/ohhelp2.c
check_present 'type == MPI_DATATYPE_NULL' src/c/ohhelp2.c
check_present 'oh_context_set_particle_mpi_type' include/oh_context.h
check_present 'useCustomParticleAdapter = 0;' src/c/ohhelp2.c
check_present 'CustomTParticle = MPI_DATATYPE_NULL;' src/c/ohhelp2.c
check_absent '\b(Decl_Grid_Info|Subdomain_Id|Primarize_Id)\b' include/ohhelp2.h
check_absent '\b(nidelement|subdomid|gridmask|loggrid)\b' include/ohhelp2.h
check_absent '\b(init2|transbound2|exchange_primary_particles|move_to_sendbuf_primary|set_sendbuf_disps|exchange_particles)\s*\(' include/ohhelp2.h
check_absent '^EXTERN ' include/ohhelp2.h
check_present 'void oh2_inject_particle\(void \*part\)' include/ohhelp2.h
check_present 'int \*\*totalp, void \*\*pbuf' include/ohhelp2.h

# Level 3 may touch S_particle.x/y/z only inside the default S_particle mapping
# adapter.  Custom layouts must use offset-based or callback adapters.
level3_xyz=$(grep -En -- '->(x|y|z)\b' src/c/ohhelp3.c || true)
level3_xyz_count=$(printf '%s\n' "$level3_xyz" | sed '/^$/d' | wc -l)
if [ "$level3_xyz_count" -ne 6 ]; then
  fail "unexpected number of Level-3 direct x/y/z accesses: $level3_xyz_count"
  echo "$level3_xyz" >&2
fi
check_absent '->(nid|spec)\b' src/c/ohhelp3.c
check_absent '^EXTERN ' include/ohhelp3.h
check_absent '\b(init3|set_field_descriptors|clear_border_exchange|map_irregular_subdomain)\s*\(' include/ohhelp3.h
check_present 'void \*\*pbuf' include/ohhelp3.h
check_absent '\bParticleAdapter\.map_to_(neighbor|subdomain)\b' \
  src/c/ohhelp3.c

# Level 4 still has packed-id semantics, but implementation code must reach
# them through local helpers so the representation can later move behind the
# adapter contract.
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
check_present 'void \*' src/c/oh_particle_buffer.h
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
check_present 'oh4p_map_particle_to_neighbor\(void\* part' include/ohhelp4p.h
check_present 'oh4s_map_particle_to_neighbor\(void\* part' include/ohhelp4s.h
check_present 'oh4s_particle_buffer\(const int maxlocalp, void\*\* pbuf\)' \
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
check_present 'nid < 0' doc/design/v2-particle-contracts.md
check_present 'nid == -2' doc/design/v2-particle-contracts.md
check_present 'oh_remove_injected_particle\(\)' doc/design/v2-particle-contracts.md
check_present 'packed-grid id operations' doc/design/v2-particle-contracts.md
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
check_present 'default context layout' doc/design/v2-particle-contracts.md
check_present 'primary_or_secondary' doc/design/v2-particle-contracts.md
check_present 'specBase = 1' doc/design/v2-particle-contracts.md
check_present 'InjectedParticles' doc/design/v2-particle-contracts.md
check_present 'OH_BIG_SPACE' doc/design/v2-particle-contracts.md
check_present 'boundary plane thickness' doc/design/v2-particle-contracts.md
check_present 'no current `original` particle field' doc/design/v2-particle-contracts.md
check_absent '\boriginal\b' include/*.h src/c/*.c src/c/*.h src/fortran/*.F90
check_present '粒子ポインタを `void \*`' doc/usage/v2-particle-and-weight.md
check_absent '\bp\.nid\s*=' doc/usage/api-by-level.md doc/usage/pic-lifecycle.md
check_present 'level4_secondary_injected' src/c/ohhelp4_particle.h

if [ "$failures" -ne 0 ]; then
  exit 1
fi
