#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$REPO_ROOT"

mkdir -p build/docker
mkdir -p build/docker/oh4p-fortran
mkdir -p build/docker/oh4s-fortran
mkdir -p build/docker/posaware
mkdir -p build/docker/dim1f
mkdir -p build/docker/dim2f

MPIRUN=${MPIRUN:-mpirun}
TEST_TIMEOUT=${TEST_TIMEOUT:-60s}

run_mpi() {
  timeout "$TEST_TIMEOUT" "$MPIRUN" "$@"
}

expect_mpi_failure_log_contains() {
  local failure_message=$1
  local log_file=$2
  local expected=$3
  local missing_message=$4
  local status
  shift 4
  set +e
  run_mpi "$@" >"$log_file" 2>&1
  status=$?
  set -e
  if [ "$status" -eq 0 ]; then
    echo "$failure_message" >&2
    exit 1
  fi
  if [ "$status" -eq 124 ]; then
    echo "$failure_message: command timed out" >&2
    cat "$log_file" >&2
    exit 1
  fi
  if grep -Eq \
      'Segmentation fault|Bus error|Floating point exception|core dumped|Assertion failed|Assertion `|AddressSanitizer|UndefinedBehaviorSanitizer' \
      "$log_file"; then
    echo "$failure_message: command crashed before the expected diagnostic path" >&2
    cat "$log_file" >&2
    exit 1
  fi
  if ! grep -Fq -- "$expected" "$log_file"; then
    echo "$missing_message" >&2
    cat "$log_file" >&2
    exit 1
  fi
}

expect_mpi_failure_log_cases() {
  local binary=$1
  local suite
  local log_file
  shift
  if [ $(( $# % 3 )) -ne 0 ]; then
    echo "expect_mpi_failure_log_cases requires MESSAGE CASE EXPECTED triples" >&2
    exit 1
  fi
  suite=$(basename "$binary")
  mkdir -p "build/docker/negative/$suite"
  while [ "$#" -gt 0 ]; do
    local message=$1
    local case_name=$2
    local expected=$3
    shift 3
    log_file="build/docker/negative/$suite/${case_name}.err"
    expect_mpi_failure_log_contains \
      "$message" "$log_file" "$expected" \
      "$message did not report expected diagnostic" \
      -n 1 "$binary" "$case_name"
  done
}

build_fortran_dimension_guard() {
  local dim=$1
  local mod_flag=$2
  local dimdir=$3
  local dim_objs

  $MPICC -DOH_DIMENSION="$dim" -Iinclude -c src/c/oh_load_balance.c \
    -o "$dimdir/oh_load_balance.o"
  $MPICC -DOH_DIMENSION="$dim" -Iinclude -c src/c/oh_particle_adapter.c \
    -o "$dimdir/oh_particle_adapter.o"
  $MPICC -DOH_DIMENSION="$dim" -Iinclude -c src/c/oh_context.c \
    -o "$dimdir/oh_context.o"
  $MPICC -DOH_DIMENSION="$dim" -Iinclude -c src/c/oh_fortran_v2.c \
    -o "$dimdir/oh_fortran_v2.o"
  $MPICC -DOH_DIMENSION="$dim" -Iinclude -c src/c/ohhelp1.c \
    -o "$dimdir/ohhelp1.o"
  $MPICC -DOH_DIMENSION="$dim" -Iinclude -c src/c/ohhelp2.c \
    -o "$dimdir/ohhelp2.o"
  $MPICC -DOH_DIMENSION="$dim" -Iinclude -c src/c/ohhelp3.c \
    -o "$dimdir/ohhelp3.o"
  $FC $FC_MPI_COMPILE_FLAGS -DOH_DIMENSION="$dim" -cpp -Iinclude \
    $mod_flag -c src/fortran/oh_v2.F90 -o "$dimdir/oh_v2.o"
  dim_objs="$dimdir/oh_v2.o $dimdir/oh_context.o \
$dimdir/oh_fortran_v2.o $dimdir/oh_particle_adapter.o \
$dimdir/oh_load_balance.o $dimdir/ohhelp1.o \
$dimdir/ohhelp2.o $dimdir/ohhelp3.o"
  $FC $FC_MPI_COMPILE_FLAGS -DOH_DIMENSION="$dim" -cpp -Iinclude \
    -I"$dimdir" $mod_flag tests/test_oh_context_particle_layout_guard_fortran.F90 \
    $dim_objs -o "$dimdir/test_oh_context_particle_layout_guard_fortran"
}

MPIFC=${MPIFC:-mpifort}
MPICC=${MPICC:-mpicc}
MPICXX=${MPICXX:-mpic++}
FC=$MPIFC
FC_VERSION=$($FC --version 2>/dev/null || true)
FC_MPI_COMPILE_FLAGS=$($FC -showme:compile 2>/dev/null || true)
case "$FC_VERSION" in
  *nvfortran*|*NVFORTRAN*|*NVIDIA*)
    FC_MOD_MAIN="-module build/docker"
    FC_MOD_POSAWARE="-module build/docker/posaware"
    FC_MOD_DIM1="-module build/docker/dim1f"
    FC_MOD_DIM2="-module build/docker/dim2f"
    FC_FREE_LINE_FLAGS="-Mfree -Mextend"
    ;;
  *)
    FC_MOD_MAIN="-Jbuild/docker"
    FC_MOD_POSAWARE="-Jbuild/docker/posaware"
    FC_MOD_DIM1="-Jbuild/docker/dim1f"
    FC_MOD_DIM2="-Jbuild/docker/dim2f"
    FC_FREE_LINE_FLAGS="-ffree-line-length-none"
    ;;
esac

$MPICC -Iinclude -c src/c/oh_load_balance.c -o build/docker/oh_load_balance.o
$MPICC -Iinclude -c src/c/oh_particle_adapter.c -o build/docker/oh_particle_adapter.o
$MPICC -Iinclude -c src/c/oh_context.c -o build/docker/oh_context.o
$MPICC -Iinclude -c src/c/oh_fortran_v2.c -o build/docker/oh_fortran_v2.o
$MPICC -Iinclude -c src/c/ohhelp1.c -o build/docker/ohhelp1.o
$MPICC -Iinclude -c src/c/ohhelp2.c -o build/docker/ohhelp2.o
$MPICC -Iinclude -c src/c/ohhelp3.c -o build/docker/ohhelp3.o
C_CORE_OBJS="build/docker/oh_load_balance.o \
build/docker/oh_particle_adapter.o build/docker/oh_context.o \
build/docker/ohhelp1.o build/docker/ohhelp2.o build/docker/ohhelp3.o"
$MPICC -Iinclude -DOH_LIB_LEVEL_4P -c src/c/ohhelp4p.c \
  -o build/docker/ohhelp4p.o
$MPICC -Iinclude -DOH_LIB_LEVEL_4S -c src/c/ohhelp4s.c \
  -o build/docker/ohhelp4s.o

$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_MAIN -c src/fortran/oh_type.F90 \
  -o build/docker/oh_type.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_MAIN -c src/fortran/oh_mod1.F90 \
  -o build/docker/oh_mod1.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_MAIN -c src/fortran/oh_mod2.F90 \
  -o build/docker/oh_mod2.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_MAIN -c src/fortran/oh_mod3.F90 \
  -o build/docker/oh_mod3.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_MAIN -c src/fortran/oh_mod4p.F90 \
  -o build/docker/oh_mod4p.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_MAIN -c src/fortran/oh_mod4s.F90 \
  -o build/docker/oh_mod4s.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_MAIN -c src/fortran/oh_v2.F90 \
  -o build/docker/oh_v2.o
F_V2_CORE_OBJS="build/docker/oh_v2.o build/docker/oh_type.o \
build/docker/oh_context.o build/docker/oh_fortran_v2.o \
build/docker/oh_particle_adapter.o build/docker/oh_load_balance.o \
build/docker/ohhelp1.o build/docker/ohhelp2.o build/docker/ohhelp3.o"
F_RAW_INIT_OBJS="build/docker/oh_v2.o build/docker/oh_type.o \
build/docker/oh_mod1.o build/docker/oh_context.o \
build/docker/oh_fortran_v2.o build/docker/oh_particle_adapter.o \
build/docker/oh_load_balance.o build/docker/ohhelp1.o \
build/docker/ohhelp2.o build/docker/ohhelp3.o"
$FC $FC_MPI_COMPILE_FLAGS -cpp -w $FC_FREE_LINE_FLAGS -Iinclude -Ibuild/docker \
  $FC_MOD_MAIN -c sample/sample.F90 \
  -o build/docker/sample_f.o
$MPICC -Iinclude -c sample/sample.c \
  -o build/docker/sample_c.o
make -n -f sample/samplec.mk clean
make -n -f sample/samplef.mk clean
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  -c tests/test_oh_v2_fortran.F90 \
  -o build/docker/test_oh_v2_fortran.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_runtime.F90 \
  $F_V2_CORE_OBJS \
  -o build/docker/test_oh_v2_fortran_runtime
run_mpi -n 1 build/docker/test_oh_v2_fortran_runtime
run_mpi -n 2 build/docker/test_oh_v2_fortran_runtime
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=2 \
  -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l2
run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l2
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=2 \
  -DTEST_OH_RAW_NULL_PBUF=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l2_null_pbuf
run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l2_null_pbuf
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=2 \
  -DTEST_OH_RAW_NULL_PBASE=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l2_null_pbase
expect_mpi_failure_log_contains \
  "expected Level 2 raw init with NULL pbase to fail" \
  build/docker/raw_l2_null_pbase.err \
  "oh2_init_raw requires a particle base array" \
  "expected Level 2 raw init failure to report NULL pbase" \
  -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_l2_null_pbase
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=2 \
  -DTEST_OH_RAW_NULL_PCOORD=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l2_null_pcoord
expect_mpi_failure_log_contains \
  "expected Level 2 raw init with NULL pcoord to fail" \
  build/docker/raw_l2_null_pcoord.err \
  "oh2_init_raw requires a process grid array" \
  "expected Level 2 raw init failure to report NULL pcoord" \
  -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_l2_null_pcoord
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=3 \
  -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l3
run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l3
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=3 \
  -DTEST_OH_RAW_NULL_PBUF=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l3_null_pbuf
run_mpi -n 2 build/docker/test_oh_v2_fortran_raw_init_runtime_l3_null_pbuf
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=3 \
  -DTEST_OH_RAW_NULL_PBASE=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l3_null_pbase
expect_mpi_failure_log_contains \
  "expected Level 3 raw init with NULL pbase to fail" \
  build/docker/raw_l3_null_pbase.err \
  "oh3_init_raw requires a particle base array" \
  "expected Level 3 raw init failure to report NULL pbase" \
  -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_l3_null_pbase
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=3 \
  -DTEST_OH_RAW_NULL_PCOORD=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_l3_null_pcoord
expect_mpi_failure_log_contains \
  "expected Level 3 raw init with NULL pcoord to fail" \
  build/docker/raw_l3_null_pcoord.err \
  "oh3_init_raw requires a process grid array" \
  "expected Level 3 raw init failure to report NULL pcoord" \
  -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_l3_null_pcoord
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=3 \
  -DTEST_OH_RAW_MYCOMM=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_mycomm
run_mpi -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_mycomm
$FC $FC_MPI_COMPILE_FLAGS -cpp -DTEST_OH_RAW_LEVEL=2 \
  -DTEST_OH_RAW_MYCOMM=1 -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_v2_fortran_raw_init_runtime.F90 \
  $F_RAW_INIT_OBJS \
  -o build/docker/test_oh_v2_fortran_raw_init_runtime_mycomm_l2
run_mpi -n 1 build/docker/test_oh_v2_fortran_raw_init_runtime_mycomm_l2

$MPICC -Iinclude tests/test_oh_context_header.c \
  -c -o build/docker/test_oh_context_header.o
$MPICC -Iinclude -Isrc/c tests/test_oh_context_lifecycle.c \
  $C_CORE_OBJS \
  -o build/docker/test_oh_context_lifecycle
run_mpi -n 1 build/docker/test_oh_context_lifecycle
run_mpi -n 2 build/docker/test_oh_context_lifecycle
run_mpi -n 3 build/docker/test_oh_context_lifecycle
run_mpi -n 1 build/docker/test_oh_context_lifecycle \
  destroy-border-after-finalize
$MPICC -Iinclude -Isrc/c tests/test_oh_context_default_region_ids.c \
  $C_CORE_OBJS \
  -o build/docker/test_oh_context_default_region_ids
run_mpi -n 1 build/docker/test_oh_context_default_region_ids
run_mpi -n 2 build/docker/test_oh_context_default_region_ids
$MPICC -Iinclude -Isrc/c tests/test_oh_context_create_lifecycle.c \
  $C_CORE_OBJS build/docker/oh_fortran_v2.o \
  -o build/docker/test_oh_context_create_lifecycle
run_mpi -n 1 build/docker/test_oh_context_create_lifecycle before-init
run_mpi -n 1 build/docker/test_oh_context_create_lifecycle after-finalize
run_mpi -n 1 build/docker/test_oh_context_create_lifecycle fortran-before-init
run_mpi -n 1 build/docker/test_oh_context_create_lifecycle fortran-after-finalize
$MPICC -Iinclude -Isrc/c tests/test_oh_default_particle_type_ownership.c \
  $C_CORE_OBJS \
  -o build/docker/test_oh_default_particle_type_ownership
run_mpi -n 1 build/docker/test_oh_default_particle_type_ownership
$MPICC -Iinclude -Isrc/c tests/test_oh2_init_guard.c \
  $C_CORE_OBJS build/docker/oh_fortran_v2.o \
  -o build/docker/test_oh2_init_guard
run_mpi -n 1 build/docker/test_oh2_init_guard
run_mpi -n 1 build/docker/test_oh2_init_guard oh3-valid
expect_mpi_failure_log_cases build/docker/test_oh2_init_guard \
  "expected oh2_init with NULL pbuf slot to fail" \
  null-pbuf-slot "oh2_init() requires a particle pointer slot" \
  "expected oh2_init with NULL pbase slot to fail" \
  null-pbase-slot "oh2_init() requires a particle base slot" \
  "expected oh2_init with negative maxfrac to fail" \
  negative-maxfrac "oh_init() requires maxfrac >= 0" \
  "expected oh2_max_local_particles with negative minmargin to fail" \
  maxlocal-negative-minmargin \
  "minimum particle buffer margin (-1) should be non-negative" \
  "expected oh2_max_local_particles oversized base capacity to fail" \
  maxlocal-overflow-base "out of virtual memory for Particles" \
  "expected oh2_max_local_particles oversized margin capacity to fail" \
  maxlocal-overflow-margin "out of virtual memory for Particles" \
  "expected oh3_init with NULL pbuf slot to fail" \
  oh3-null-pbuf-slot "oh3_init() requires a particle pointer slot" \
  "expected oh3_init with NULL pbase slot to fail" \
  oh3-null-pbase-slot "oh3_init() requires a particle base slot" \
  "expected oh3_init with negative maxfrac to fail" \
  oh3-negative-maxfrac "oh_init() requires maxfrac >= 0"
expect_mpi_failure_log_cases build/docker/test_oh2_init_guard \
  "expected Level 2 raw init with NULL pbuf slot to fail" \
  raw-oh2-null-pbuf-slot "oh2_init_raw requires a particle pointer slot" \
  "expected Level 3 raw init with NULL pbuf slot to fail" \
  raw-oh3-null-pbuf-slot "oh3_init_raw requires a particle pointer slot" \
  "expected Level 2 raw init with NULL sdid to fail" \
  raw-oh2-null-sdid "oh2_init_raw requires a region id array" \
  "expected Level 2 raw init with NULL nphgram to fail" \
  raw-oh2-null-nphgram "oh2_init_raw requires a particle histogram array" \
  "expected Level 2 raw init with NULL totalp to fail" \
  raw-oh2-null-totalp "oh2_init_raw requires a total particle array" \
  "expected Level 3 raw init with NULL sdid to fail" \
  raw-oh3-null-sdid "oh3_init_raw requires a region id array" \
  "expected Level 3 raw init with NULL nphgram to fail" \
  raw-oh3-null-nphgram "oh3_init_raw requires a particle histogram array" \
  "expected Level 3 raw init with NULL totalp to fail" \
  raw-oh3-null-totalp "oh3_init_raw requires a total particle array" \
  "expected Level 2 raw init with negative maxfrac to fail" \
  raw-oh2-negative-maxfrac "oh_init() requires maxfrac >= 0" \
  "expected Level 3 raw init with negative maxfrac to fail" \
  raw-oh3-negative-maxfrac "oh_init() requires maxfrac >= 0"
$MPICC -DOH_DIMENSION=1 -Iinclude -Isrc/c tests/test_oh_context_lifecycle.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c \
  -o build/docker/test_oh_context_lifecycle_1d
run_mpi -n 1 build/docker/test_oh_context_lifecycle_1d
run_mpi -n 2 build/docker/test_oh_context_lifecycle_1d
$MPICC -DOH_DIMENSION=2 -Iinclude -Isrc/c tests/test_oh_context_lifecycle.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c \
  -o build/docker/test_oh_context_lifecycle_2d
run_mpi -n 1 build/docker/test_oh_context_lifecycle_2d
run_mpi -n 2 build/docker/test_oh_context_lifecycle_2d
$MPICC -Iinclude -Isrc/c tests/test_oh_context_particle_layout_guard.c \
  $C_CORE_OBJS -o build/docker/test_oh_context_particle_layout_guard
run_mpi -n 1 build/docker/test_oh_context_particle_layout_guard
expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard \
  "expected adapter change while particles are bound to fail" adapter \
  "cannot change particle layout while particles are bound" \
  "expected MPI type change while particles are bound to fail" type \
  "cannot change particle layout while particles are bound" \
  "expected mismatched particle MPI datatype extent to fail" type-extent \
  "particle MPI datatype extent must match particle stride" \
  "expected invalid adapter destination to fail" invalid-destination \
  "particle adapter map_to_subdomain() returned destination"
expect_mpi_failure_log_contains \
  "expected invalid adapter species to fail" \
  build/docker/invalid_species.err \
  "outside configured range" \
  "expected invalid adapter species failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard invalid-species
expect_mpi_failure_log_contains \
  "expected active invalid adapter destination in transbound2 to fail" \
  build/docker/active_invalid_destination_transbound2.err \
  "particle adapter map_to_subdomain() returned destination" \
  "expected active transbound2 invalid adapter destination failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  active-invalid-destination-transbound2
expect_mpi_failure_log_contains \
  "expected active invalid adapter destination in transbound3 to fail" \
  build/docker/active_invalid_destination_transbound3.err \
  "particle adapter map_to_subdomain() returned destination" \
  "expected active transbound3 invalid adapter destination failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  active-invalid-destination-transbound3
expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard \
  "expected oversized species count to fail before allocation" \
  species-index-overflow \
  "species/node count exceeds Level 2 particle accounting capacity" \
  "expected capacity calculation overflow to fail before addition" \
  capacity-add-overflow "out of virtual memory for Particles"
expect_mpi_failure_log_contains \
  "expected zero species C context configuration to fail" \
  build/docker/c_context_zero_species.err \
  "oh_context_configure_particles() requires nspec > 0" \
  "expected zero species C context configuration failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  configure-zero-species
expect_mpi_failure_log_contains \
  "expected negative maxfrac C context configuration to fail" \
  build/docker/c_context_negative_maxfrac.err \
  "oh_context_configure_particles() requires maxfrac >= 0" \
  "expected negative maxfrac C context configuration failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  configure-negative-maxfrac
expect_mpi_failure_log_contains \
  "expected invalid C context region weight to fail" \
  build/docker/c_context_zero_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected invalid C context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard zero-weight
expect_mpi_failure_log_contains \
  "expected negative C context region weight to fail" \
  build/docker/c_context_negative_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected negative C context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  negative-region-weight
expect_mpi_failure_log_contains \
  "expected NaN C context region weight to fail" \
  build/docker/c_context_nan_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected NaN C context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard nan-region-weight
expect_mpi_failure_log_contains \
  "expected infinite C context region weight to fail" \
  build/docker/c_context_inf_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected infinite C context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard inf-region-weight
expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard \
  "expected owned region id binding with non-NULL storage to fail" \
  owned-region-nonnull "owned region id binding requires a NULL array" \
  "expected borrowed region id binding with NULL storage to fail" \
  borrowed-region-null "borrowed region id binding requires a non-NULL array" \
  "expected invalid region id ownership flag to fail" \
  invalid-region-ownership "invalid region id ownership flag" \
  "expected region id getter with NULL storage to fail" \
  get-region-null "region id getter requires a non-NULL array" \
  "expected particle binding before configure to fail" unconfigured-bind \
  "particle binding requires configured particles" \
  "expected owned particle binding with non-NULL storage to fail" \
  owned-particle-nonnull \
  "owned particle buffer binding requires a NULL buffer" \
  "expected invalid particle ownership flag to fail" \
  invalid-particle-ownership "invalid particle buffer ownership flag" \
  "expected borrowed particle binding with NULL storage to fail" \
  borrowed-particle-null \
  "borrowed particle buffer binding requires a non-NULL buffer" \
  "expected owned accounting binding with non-NULL storage to fail" \
  owned-accounting-nonnull \
  "owned particle accounting requires NULL nphgram/totalp" \
  "expected invalid accounting ownership flag to fail" \
  invalid-accounting-ownership "invalid particle accounting ownership flag" \
  "expected accounting binding with NULL nphgram slot to fail" \
  accounting-null-nphgram-slot \
  "particle accounting binding requires nphgram and totalp" \
  "expected accounting binding with NULL totalp slot to fail" \
  accounting-null-totalp-slot \
  "particle accounting binding requires nphgram and totalp" \
  "expected borrowed accounting binding with NULL storage to fail" \
  borrowed-accounting-null \
  "borrowed particle accounting requires non-NULL arrays" \
  "expected borrowed accounting binding with NULL pbase to fail" \
  borrowed-accounting-null-pbase \
  "borrowed particle accounting requires non-NULL pbase" \
  "expected owned accounting binding with non-NULL pbase to fail" \
  owned-accounting-nonnull-pbase \
  "owned particle accounting requires NULL pbase" \
  "expected accounting binding with NULL pbase slot to fail" \
  accounting-null-pbase-slot \
  "particle accounting binding requires a pbase slot" \
  "expected set_total_particles without accounting to fail" set-total-unbound \
  "particle accounting is not bound" \
  "expected grid size with NULL storage to fail" grid-size-null \
  "oh_context_grid_size() requires a size array" \
  "expected Level 3 field operation without fields to fail" \
  level3-field-unconfigured "oh_bcast_field() requires configured fields" \
  "expected Level 3 border exchange without exchanges to fail" \
  level3-exchange-unconfigured \
  "oh_exchange_borders() requires configured boundary exchanges"
expect_mpi_failure_log_contains \
  "expected Level 3 field config without ctypes to fail" \
  build/docker/level3_missing_ctypes.err \
  "Level 3 field configuration requires ctypes" \
  "expected Level 3 field config failure to report missing ctypes" \
  -n 1 build/docker/test_oh_context_particle_layout_guard level3-missing-ctypes
expect_mpi_failure_log_contains \
  "expected injected particle index overflow to fail" \
  build/docker/inject_index_overflow.err \
  "injection causes local particle buffer overflow" \
  "expected injected particle index overflow to report buffer overflow" \
  -n 1 build/docker/test_oh_context_particle_layout_guard inject-index-overflow
expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard \
  "expected NULL injected remap pointer to fail" \
  remap-null-injected-pointer "not for injected particles" \
  "expected NULL injected remove pointer to fail" \
  remove-null-injected-pointer "not for injected particles" \
  "expected interior injected remap pointer to fail" \
  remap-interior-injected-pointer "not for injected particles" \
  "expected interior injected remove pointer to fail" \
  remove-interior-injected-pointer "not for injected particles" \
  "expected active particle remap as injected to fail" \
  remap-active-particle "not for injected particles" \
  "expected active particle remove as injected to fail" \
  remove-active-particle "not for injected particles"
expect_mpi_failure_log_contains \
  "expected finalized injected particle remap to fail" \
  build/docker/remap_finalized_injected_copy.err \
  "not for injected particles" \
  "expected finalized injected particle remap failure to report injected copy" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  remap-finalized-injected-copy
expect_mpi_failure_log_contains \
  "expected finalized injected particle remove to fail" \
  build/docker/remove_finalized_injected_copy.err \
  "not for injected particles" \
  "expected finalized injected particle remove failure to report injected copy" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  remove-finalized-injected-copy
expect_mpi_failure_log_contains \
  "expected original injected source remap to fail" \
  build/docker/remap_original_injected_source.err \
  "not for injected particles" \
  "expected original injected source remap failure to report injected copy" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  remap-original-injected-source
expect_mpi_failure_log_contains \
  "expected original injected source remove to fail" \
  build/docker/remove_original_injected_source.err \
  "not for injected particles" \
  "expected original injected source remove failure to report injected copy" \
  -n 1 build/docker/test_oh_context_particle_layout_guard \
  remove-original-injected-source
expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard \
  "expected particle reconfigure while particles are bound to fail" \
  reconfigure-bound "cannot change species count while particles are bound"
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_context_particle_layout_guard_fortran.F90 \
  $F_V2_CORE_OBJS \
  -o build/docker/test_oh_context_particle_layout_guard_fortran
run_mpi -n 1 build/docker/test_oh_context_particle_layout_guard_fortran
expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard_fortran \
  "expected Fortran NULL context handle to fail" null-context \
  "requires an associated context handle" \
  "expected Fortran NULL context particle bind to fail" \
  null-bind-particles "requires an associated context handle" \
  "expected Fortran NULL context region id bind to fail" \
  null-bind-region-ids "requires an associated context handle" \
  "expected Fortran NULL context region id getter to fail" \
  null-get-region-ids "requires an associated context handle" \
  "expected Fortran NULL context accounting bind to fail" \
  null-bind-accounting "requires an associated context handle" \
  "expected Fortran NULL context transbound3 to fail" \
  null-transbound3 "requires an associated context handle" \
  "expected Fortran NULL context grid size to fail" \
  null-grid-size "requires an associated context handle" \
  "expected Fortran NULL context injection to fail" \
  null-inject "requires an associated context handle" \
  "expected Fortran NULL context region weights to fail" null-region-weights \
  "requires an associated context handle" \
  "expected Fortran short region weights to fail" short-weights \
  "requires 1 weights for this context, got 0" \
  "expected Fortran long region weights to fail" long-weights \
  "requires 1 weights for this context, got 2" \
  "expected Fortran short grid size array to fail" short-grid-size \
  "oh_context_grid_size requires at least OH_DIMENSION elements" \
  "expected Fortran non-empty short grid size array to fail" \
  short-grid-size-nonempty \
  "oh_context_grid_size requires at least OH_DIMENSION elements" \
  "expected Fortran missing y coordinate to fail" missing-y \
  "oh_context_map_particle_to_neighbor requires y coordinate" \
  "expected Fortran missing z coordinate to fail" missing-z \
  "oh_context_map_particle_to_neighbor requires z coordinate" \
  "expected Fortran missing subdomain y coordinate to fail" \
  missing-subdomain-y \
  "oh_context_map_particle_to_subdomain requires y coordinate" \
  "expected Fortran missing subdomain z coordinate to fail" \
  missing-subdomain-z \
  "oh_context_map_particle_to_subdomain requires z coordinate" \
  "expected Fortran owned region id binding with non-NULL storage to fail" \
  owned-region "owned region id binding requires a NULL array" \
  "expected Fortran owned particle binding with non-NULL storage to fail" \
  owned-particle "owned particle buffer binding requires a NULL buffer" \
  "expected Fortran borrowed particle binding with NULL storage to fail" \
  borrowed-particle-null \
  "borrowed particle buffer binding requires a non-NULL buffer" \
  "expected Fortran owned accounting binding with non-NULL storage to fail" \
  owned-accounting "owned particle accounting requires NULL nphgram/totalp" \
  "expected Fortran borrowed accounting binding with NULL storage to fail" \
  borrowed-accounting-null \
  "borrowed particle accounting requires non-NULL arrays" \
  "expected Fortran particle binding before configure to fail" unconfigured \
  "particle binding requires configured particles" \
  "expected Fortran zero species context configuration to fail" \
  configure-zero-species \
  "oh_context_configure_particles() requires nspec > 0" \
  "expected Fortran negative maxfrac context configuration to fail" \
  configure-negative-maxfrac \
  "oh_context_configure_particles() requires maxfrac >= 0" \
  "expected Fortran borrowed region id binding with NULL storage to fail" \
  borrowed-region-null "borrowed region id binding requires a non-NULL array" \
  "expected Fortran region id getter with NULL storage to fail" \
  get-region-null "region id getter requires a non-NULL array" \
  "expected Fortran Level 3 field operation without fields to fail" \
  level3-field-unconfigured "oh_bcast_field() requires configured fields" \
  "expected Fortran Level 3 border exchange without exchanges to fail" \
  level3-exchange-unconfigured \
  "oh_exchange_borders() requires configured boundary exchanges"
expect_mpi_failure_log_contains \
  "expected Fortran short region weights to fail on two ranks" \
  build/docker/fortran_context_short_weights_n2.err \
  "requires 2 weights for this context, got 1" \
  "expected Fortran short region weights n=2 failure message" \
  -n 2 build/docker/test_oh_context_particle_layout_guard_fortran \
  short-weights-n2
expect_mpi_failure_log_cases build/docker/test_oh_context_particle_layout_guard_fortran \
  "expected Fortran adapter change while particles are bound to fail" \
  adapter "cannot change particle layout while particles are bound" \
  "expected Fortran MPI type change while particles are bound to fail" \
  type "cannot change particle layout while particles are bound" \
  "expected Fortran mismatched particle MPI datatype extent to fail" \
  type-extent "particle MPI datatype extent must match particle stride" \
  "expected Fortran invalid region id ownership flag to fail" \
  invalid-region-ownership "invalid region id ownership flag" \
  "expected Fortran invalid particle ownership flag to fail" \
  invalid-particle-ownership "invalid particle buffer ownership flag" \
  "expected Fortran invalid accounting ownership flag to fail" \
  invalid-accounting-ownership "invalid particle accounting ownership flag" \
  "expected Fortran borrowed accounting binding with NULL pbase to fail" \
  borrowed-accounting-null-pbase \
  "borrowed particle accounting requires non-NULL pbase" \
  "expected Fortran owned accounting binding with non-NULL pbase to fail" \
  owned-accounting-nonnull-pbase \
  "owned particle accounting requires NULL pbase" \
  "expected Fortran particle reconfigure while particles are bound to fail" \
  reconfigure "cannot change species count while particles are bound"
expect_mpi_failure_log_contains \
  "expected Fortran negative context region weight to fail" \
  build/docker/fortran_context_negative_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected Fortran negative context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  negative-region-weight
expect_mpi_failure_log_contains \
  "expected Fortran zero context region weight to fail" \
  build/docker/fortran_context_zero_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected Fortran zero context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  zero-region-weight
expect_mpi_failure_log_contains \
  "expected Fortran NaN context region weight to fail" \
  build/docker/fortran_context_nan_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected Fortran NaN context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  nan-region-weight
expect_mpi_failure_log_contains \
  "expected Fortran infinite context region weight to fail" \
  build/docker/fortran_context_inf_weight.err \
  "region weight[0] must be finite and greater than zero" \
  "expected Fortran infinite context region weight failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  inf-region-weight
expect_mpi_failure_log_contains \
  "expected Fortran finalized injected particle remap to fail" \
  build/docker/fortran_remap_finalized_injected_copy.err \
  "not for injected particles" \
  "expected Fortran finalized injected particle remap failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  remap-finalized-injected-copy
expect_mpi_failure_log_contains \
  "expected Fortran finalized injected particle remove to fail" \
  build/docker/fortran_remove_finalized_injected_copy.err \
  "not for injected particles" \
  "expected Fortran finalized injected particle remove failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  remove-finalized-injected-copy
expect_mpi_failure_log_contains \
  "expected Fortran original injected source remap to fail" \
  build/docker/fortran_remap_original_injected_source.err \
  "not for injected particles" \
  "expected Fortran original injected source remap failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  remap-original-injected-source
expect_mpi_failure_log_contains \
  "expected Fortran original injected source remove to fail" \
  build/docker/fortran_remove_original_injected_source.err \
  "not for injected particles" \
  "expected Fortran original injected source remove failure message" \
  -n 1 build/docker/test_oh_context_particle_layout_guard_fortran \
  remove-original-injected-source
build_fortran_dimension_guard 1 "$FC_MOD_DIM1" build/docker/dim1f
build_fortran_dimension_guard 2 "$FC_MOD_DIM2" build/docker/dim2f
expect_mpi_failure_log_contains \
  "expected Fortran 2D missing y coordinate to fail" \
  build/docker/fortran_dim2_missing_y.err \
  "oh_context_map_particle_to_neighbor requires y coordinate" \
  "expected Fortran 2D missing-y failure message" \
  -n 1 build/docker/dim2f/test_oh_context_particle_layout_guard_fortran \
  missing-y
$MPICC -DOH_LIB_LEVEL_4P -Iinclude tests/test_oh4p_capacity_guard.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c src/c/ohhelp4p.c \
  -o build/docker/test_oh4p_capacity_guard
run_mpi -n 1 build/docker/test_oh4p_capacity_guard
expect_mpi_failure_log_contains \
  "expected Level 4p zero hotspot threshold to fail" \
  build/docker/oh4p_zero_hotspot.err \
  "hotspot threshold (0) should be greater than 0" \
  "expected Level 4p zero hotspot threshold failure message" \
  -n 1 build/docker/test_oh4p_capacity_guard zero-hotspot
expect_mpi_failure_log_contains \
  "expected Level 4p oversized hotspot threshold to fail" \
  build/docker/oh4p_overflow_hotspot.err \
  "out of virtual memory for Particles" \
  "expected Level 4p oversized hotspot threshold failure message" \
  -n 1 build/docker/test_oh4p_capacity_guard overflow-hotspot
expect_mpi_failure_log_contains \
  "expected Level 4p doubled particle capacity overflow to fail" \
  build/docker/oh4p_overflow_doubled_capacity.err \
  "out of virtual memory for Particles" \
  "expected Level 4p doubled particle capacity failure message" \
  -n 1 build/docker/test_oh4p_capacity_guard overflow-doubled-capacity
$MPICC -DOH_LIB_LEVEL_4S -Iinclude tests/test_oh4s_capacity_guard.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c src/c/ohhelp4s.c \
  -o build/docker/test_oh4s_capacity_guard
run_mpi -n 1 build/docker/test_oh4s_capacity_guard
expect_mpi_failure_log_contains \
  "expected Level 4s NULL particle pointer slot to fail" \
  build/docker/oh4s_null_pbuf_slot.err \
  "oh4s_particle_buffer() requires a particle pointer slot" \
  "expected Level 4s NULL particle pointer slot failure message" \
  -n 1 build/docker/test_oh4s_capacity_guard null-pbuf-slot
expect_mpi_failure_log_contains \
  "expected Level 4s oversized maxlocalp to fail" \
  build/docker/oh4s_overflow_maxlocalp.err \
  "out of virtual memory for Particles" \
  "expected Level 4s oversized maxlocalp failure message" \
  -n 1 build/docker/test_oh4s_capacity_guard overflow-maxlocalp
$MPICC -DOH_LIB_LEVEL_4P -Iinclude tests/test_oh4p_runtime_smoke.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c src/c/ohhelp4p.c \
  -o build/docker/test_oh4p_runtime_smoke
run_mpi -n 2 build/docker/test_oh4p_runtime_smoke
run_mpi -n 2 build/docker/test_oh4p_runtime_smoke custom-adapter
run_mpi -n 2 build/docker/test_oh4p_runtime_smoke weighted-load
run_mpi -n 2 build/docker/test_oh4p_runtime_smoke weighted-secondary
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c tests/test_oh4_runtime_globals.c \
  -o build/docker/oh4p-fortran/test_oh4_runtime_globals.o
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c src/c/oh_load_balance.c \
  -o build/docker/oh4p-fortran/oh_load_balance.o
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c src/c/oh_particle_adapter.c \
  -o build/docker/oh4p-fortran/oh_particle_adapter.o
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c src/c/oh_context.c \
  -o build/docker/oh4p-fortran/oh_context.o
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c src/c/ohhelp1.c \
  -o build/docker/oh4p-fortran/ohhelp1.o
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c src/c/ohhelp2.c \
  -o build/docker/oh4p-fortran/ohhelp2.o
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c src/c/ohhelp3.c \
  -o build/docker/oh4p-fortran/ohhelp3.o
$MPICC -DOH_LIB_LEVEL_4P -Iinclude -c src/c/ohhelp4p.c \
  -o build/docker/oh4p-fortran/ohhelp4p.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -DOH_LIB_LEVEL_4P -Iinclude -Ibuild/docker \
  $FC_MOD_MAIN tests/test_oh4p_fortran_runtime.F90 \
  build/docker/oh4p-fortran/test_oh4_runtime_globals.o \
  build/docker/oh_type.o build/docker/oh_mod1.o build/docker/oh_mod2.o \
  build/docker/oh_mod3.o build/docker/oh_mod4p.o \
  build/docker/oh4p-fortran/oh_context.o \
  build/docker/oh4p-fortran/oh_particle_adapter.o \
  build/docker/oh4p-fortran/oh_load_balance.o \
  build/docker/oh4p-fortran/ohhelp1.o \
  build/docker/oh4p-fortran/ohhelp2.o \
  build/docker/oh4p-fortran/ohhelp3.o \
  build/docker/oh4p-fortran/ohhelp4p.o \
  -o build/docker/test_oh4p_fortran_runtime
run_mpi -n 2 build/docker/test_oh4p_fortran_runtime
run_mpi -n 2 build/docker/test_oh4p_fortran_runtime weighted-secondary
$MPICC -DOH_LIB_LEVEL_4S -Iinclude tests/test_oh4s_runtime_smoke.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c src/c/ohhelp4s.c \
  -o build/docker/test_oh4s_runtime_smoke
run_mpi -n 2 build/docker/test_oh4s_runtime_smoke
run_mpi -n 2 build/docker/test_oh4s_runtime_smoke custom-adapter
run_mpi -n 2 build/docker/test_oh4s_runtime_smoke weighted-load
run_mpi -n 2 build/docker/test_oh4s_runtime_smoke weighted-secondary
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c tests/test_oh4_runtime_globals.c \
  -o build/docker/oh4s-fortran/test_oh4_runtime_globals.o
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c src/c/oh_load_balance.c \
  -o build/docker/oh4s-fortran/oh_load_balance.o
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c src/c/oh_particle_adapter.c \
  -o build/docker/oh4s-fortran/oh_particle_adapter.o
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c src/c/oh_context.c \
  -o build/docker/oh4s-fortran/oh_context.o
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c src/c/ohhelp1.c \
  -o build/docker/oh4s-fortran/ohhelp1.o
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c src/c/ohhelp2.c \
  -o build/docker/oh4s-fortran/ohhelp2.o
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c src/c/ohhelp3.c \
  -o build/docker/oh4s-fortran/ohhelp3.o
$MPICC -DOH_LIB_LEVEL_4S -Iinclude -c src/c/ohhelp4s.c \
  -o build/docker/oh4s-fortran/ohhelp4s.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -DOH_LIB_LEVEL_4S -Iinclude -Ibuild/docker \
  $FC_MOD_MAIN tests/test_oh4s_fortran_runtime.F90 \
  build/docker/oh4s-fortran/test_oh4_runtime_globals.o \
  build/docker/oh_type.o build/docker/oh_mod1.o build/docker/oh_mod2.o \
  build/docker/oh_mod3.o build/docker/oh_mod4s.o \
  build/docker/oh4s-fortran/oh_context.o \
  build/docker/oh4s-fortran/oh_particle_adapter.o \
  build/docker/oh4s-fortran/oh_load_balance.o \
  build/docker/oh4s-fortran/ohhelp1.o \
  build/docker/oh4s-fortran/ohhelp2.o \
  build/docker/oh4s-fortran/ohhelp3.o \
  build/docker/oh4s-fortran/ohhelp4s.o \
  -o build/docker/test_oh4s_fortran_runtime
run_mpi -n 2 build/docker/test_oh4s_fortran_runtime
run_mpi -n 2 build/docker/test_oh4s_fortran_runtime weighted-secondary
$MPICC -DOH_POS_AWARE -Iinclude -Isrc/c tests/test_oh_context_lifecycle.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c \
  -o build/docker/test_oh_context_lifecycle_posaware
run_mpi -n 2 build/docker/test_oh_context_lifecycle_posaware
$MPICC -DOH_POS_AWARE -Iinclude -c src/c/oh_load_balance.c \
  -o build/docker/posaware/oh_load_balance.o
$MPICC -DOH_POS_AWARE -Iinclude -c src/c/oh_particle_adapter.c \
  -o build/docker/posaware/oh_particle_adapter.o
$MPICC -DOH_POS_AWARE -Iinclude -c src/c/oh_context.c \
  -o build/docker/posaware/oh_context.o
$MPICC -DOH_POS_AWARE -Iinclude -c src/c/oh_fortran_v2.c \
  -o build/docker/posaware/oh_fortran_v2.o
$MPICC -DOH_POS_AWARE -Iinclude -c src/c/ohhelp1.c \
  -o build/docker/posaware/ohhelp1.o
$MPICC -DOH_POS_AWARE -Iinclude -c src/c/ohhelp2.c \
  -o build/docker/posaware/ohhelp2.o
$MPICC -DOH_POS_AWARE -Iinclude -c src/c/ohhelp3.c \
  -o build/docker/posaware/ohhelp3.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude $FC_MOD_POSAWARE -c src/fortran/oh_v2.F90 \
  -o build/docker/posaware/oh_v2.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -Ibuild/docker/posaware $FC_MOD_POSAWARE \
  tests/test_oh_context_lifecycle_fortran.F90 \
  build/docker/posaware/oh_v2.o build/docker/posaware/oh_context.o \
  build/docker/posaware/oh_fortran_v2.o \
  build/docker/posaware/oh_particle_adapter.o \
  build/docker/posaware/oh_load_balance.o build/docker/posaware/ohhelp1.o \
  build/docker/posaware/ohhelp2.o build/docker/posaware/ohhelp3.o \
  -o build/docker/posaware/test_oh_context_lifecycle_fortran
run_mpi -n 2 build/docker/posaware/test_oh_context_lifecycle_fortran
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_context_lifecycle_fortran.F90 \
  $F_V2_CORE_OBJS \
  -o build/docker/test_oh_context_lifecycle_fortran
run_mpi -n 1 build/docker/test_oh_context_lifecycle_fortran
run_mpi -n 2 build/docker/test_oh_context_lifecycle_fortran
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  tests/test_oh_legacy_fortran_runtime.F90 \
  build/docker/oh_type.o build/docker/oh_mod1.o build/docker/oh_mod2.o \
  build/docker/oh_mod3.o build/docker/oh_context.o \
  build/docker/oh_particle_adapter.o build/docker/oh_load_balance.o \
  build/docker/ohhelp1.o build/docker/ohhelp2.o build/docker/ohhelp3.o \
  -o build/docker/test_oh_legacy_fortran_runtime
run_mpi -n 1 build/docker/test_oh_legacy_fortran_runtime
$MPICC -Iinclude tests/test_ohhelp_c_header.c \
  -c -o build/docker/test_ohhelp_c_header.o
$MPICC -Iinclude tests/test_ohhelp2_header.c \
  -c -o build/docker/test_ohhelp2_header.o
$MPICC -Iinclude -DOH_DIMENSION=1 tests/test_ohhelp3_header.c \
  -c -o build/docker/test_ohhelp3_header_1d.o
$MPICC -Iinclude -DOH_DIMENSION=2 tests/test_ohhelp3_header.c \
  -c -o build/docker/test_ohhelp3_header_2d.o
$MPICC -Iinclude -DOH_DIMENSION=3 tests/test_ohhelp3_header.c \
  -c -o build/docker/test_ohhelp3_header_3d.o
$MPICC -Iinclude tests/test_ohhelp4p_header.c \
  -c -o build/docker/test_ohhelp4p_header.o
$MPICC -Iinclude tests/test_ohhelp4s_header.c \
  -c -o build/docker/test_ohhelp4s_header.o
$MPICC -Iinclude tests/test_oh_fortran_v2_header.c \
  -c -o build/docker/test_oh_fortran_v2_header.o
$MPICC -Iinclude tests/test_public_header_reinclude.c \
  -c -o build/docker/test_public_header_reinclude.o
$MPICXX -std=c++11 -Iinclude tests/test_public_headers_cxx.cpp \
  -c -o build/docker/test_public_headers_cxx.o
$MPICXX -std=c++11 -Iinclude tests/test_public_headers_cxx.cpp \
  $C_CORE_OBJS -o build/docker/test_public_headers_cxx_link
run_mpi -n 1 build/docker/test_public_headers_cxx_link
$MPICXX -std=c++11 -Iinclude -DOH_LIB_LEVEL_4P \
  tests/test_public_headers_cxx.cpp \
  -c -o build/docker/test_public_headers_cxx_4p.o
$MPICXX -std=c++11 -Iinclude -DOH_LIB_LEVEL_4S \
  tests/test_public_headers_cxx.cpp \
  -c -o build/docker/test_public_headers_cxx_4s.o
$MPICC -Iinclude tests/test_ohhelp_f_reinclude.c \
  -c -o build/docker/test_ohhelp_f_reinclude.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -DOH_LIB_LEVEL=1 \
  -c tests/test_ohhelp_f_preprocess.F90 \
  -o build/docker/test_ohhelp_f_pp_l1.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -DOH_LIB_LEVEL=2 \
  -c tests/test_ohhelp_f_preprocess.F90 \
  -o build/docker/test_ohhelp_f_pp_l2.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -DOH_LIB_LEVEL=3 -DOH_DIMENSION=1 \
  -c tests/test_ohhelp_f_preprocess.F90 \
  -o build/docker/test_ohhelp_f_pp_l3_1d.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -DOH_LIB_LEVEL_4P \
  -c tests/test_ohhelp_f_preprocess.F90 \
  -o build/docker/test_ohhelp_f_pp_4p.o
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -DOH_LIB_LEVEL_4S \
  -c tests/test_ohhelp_f_preprocess.F90 \
  -o build/docker/test_ohhelp_f_pp_4s.o
$MPICC -Iinclude sample/level3_custom_particle.c \
  -c -o build/docker/level3_custom_particle.o
$MPICC -Iinclude sample/v2_context_level2_custom_particle.c \
  $C_CORE_OBJS -o build/docker/v2_context_level2_custom_particle
run_mpi -n 1 build/docker/v2_context_level2_custom_particle
run_mpi -n 2 build/docker/v2_context_level2_custom_particle
$FC $FC_MPI_COMPILE_FLAGS -cpp -Iinclude -Ibuild/docker $FC_MOD_MAIN \
  sample/v2_context_level2_custom_particle.F90 \
  $F_V2_CORE_OBJS \
  -o build/docker/v2_context_level2_custom_particle_fortran
run_mpi -n 1 build/docker/v2_context_level2_custom_particle_fortran
run_mpi -n 2 build/docker/v2_context_level2_custom_particle_fortran
$MPICC -Iinclude tests/test_oh_load_balance.c src/c/oh_load_balance.c \
  -o build/docker/test_oh_load_balance
build/docker/test_oh_load_balance

$MPICC -Iinclude tests/test_oh_particle_adapter.c src/c/oh_particle_adapter.c \
  -o build/docker/test_oh_particle_adapter
run_mpi -n 1 build/docker/test_oh_particle_adapter
run_mpi -n 1 build/docker/test_oh_particle_adapter before-init
run_mpi -n 1 build/docker/test_oh_particle_adapter after-finalize

$MPICC -Iinclude tests/test_oh_particle_adapter_callbacks.c \
  src/c/oh_particle_adapter.c \
  -o build/docker/test_oh_particle_adapter_callbacks
build/docker/test_oh_particle_adapter_callbacks

$MPICC -Iinclude -Isrc/c tests/test_oh_particle_buffer.c \
  src/c/oh_particle_adapter.c -o build/docker/test_oh_particle_buffer
run_mpi -n 1 build/docker/test_oh_particle_buffer

bash tests/test_particle_contract_audit.sh
