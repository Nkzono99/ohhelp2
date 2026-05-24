#!/usr/bin/env bash
set -eu

mkdir -p build/docker
mkdir -p build/docker/posaware

run_mpi() {
  timeout 60s mpirun "$@"
}

mpicc -Iinclude -c src/c/oh_load_balance.c -o build/docker/oh_load_balance.o
mpicc -Iinclude -c src/c/oh_particle_adapter.c -o build/docker/oh_particle_adapter.o
mpicc -Iinclude -c src/c/oh_context.c -o build/docker/oh_context.o
mpicc -Iinclude -c src/c/oh_fortran_v2.c -o build/docker/oh_fortran_v2.o
mpicc -Iinclude -c src/c/ohhelp1.c -o build/docker/ohhelp1.o
mpicc -Iinclude -c src/c/ohhelp2.c -o build/docker/ohhelp2.o
mpicc -Iinclude -c src/c/ohhelp3.c -o build/docker/ohhelp3.o
mpicc -Iinclude -DOH_LIB_LEVEL_4P -c src/c/ohhelp4p.c \
  -o build/docker/ohhelp4p.o
mpicc -Iinclude -DOH_LIB_LEVEL_4S -c src/c/ohhelp4s.c \
  -o build/docker/ohhelp4s.o

gfortran -cpp -Iinclude -Jbuild/docker -c src/fortran/oh_type.F90 \
  -o build/docker/oh_type.o
gfortran -cpp -Iinclude -Jbuild/docker -c src/fortran/oh_mod1.F90 \
  -o build/docker/oh_mod1.o
gfortran -cpp -Iinclude -Jbuild/docker -c src/fortran/oh_mod2.F90 \
  -o build/docker/oh_mod2.o
gfortran -cpp -Iinclude -Jbuild/docker -c src/fortran/oh_mod3.F90 \
  -o build/docker/oh_mod3.o
gfortran -cpp -Iinclude -Jbuild/docker -c src/fortran/oh_mod4p.F90 \
  -o build/docker/oh_mod4p.o
gfortran -cpp -Iinclude -Jbuild/docker -c src/fortran/oh_mod4s.F90 \
  -o build/docker/oh_mod4s.o
gfortran -cpp -Iinclude -Jbuild/docker -c src/fortran/oh_v2.F90 \
  -o build/docker/oh_v2.o
gfortran -cpp -w -ffree-line-length-none -Iinclude -Ibuild/docker \
  -Jbuild/docker -c sample/sample.F90 \
  -o build/docker/sample_f.o
gfortran -cpp -Iinclude -Ibuild/docker -Jbuild/docker \
  -c tests/test_oh_v2_fortran.F90 \
  -o build/docker/test_oh_v2_fortran.o

mpicc -Iinclude tests/test_oh_context_header.c \
  -c -o build/docker/test_oh_context_header.o
mpicc -Iinclude -Isrc/c tests/test_oh_context_lifecycle.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c \
  -o build/docker/test_oh_context_lifecycle
run_mpi -n 1 build/docker/test_oh_context_lifecycle
run_mpi -n 2 build/docker/test_oh_context_lifecycle
mpicc -DOH_POS_AWARE -Iinclude -Isrc/c tests/test_oh_context_lifecycle.c \
  src/c/oh_load_balance.c src/c/oh_particle_adapter.c src/c/oh_context.c \
  src/c/ohhelp1.c src/c/ohhelp2.c src/c/ohhelp3.c \
  -o build/docker/test_oh_context_lifecycle_posaware
run_mpi -n 2 build/docker/test_oh_context_lifecycle_posaware
mpicc -DOH_POS_AWARE -Iinclude -c src/c/oh_load_balance.c \
  -o build/docker/posaware/oh_load_balance.o
mpicc -DOH_POS_AWARE -Iinclude -c src/c/oh_particle_adapter.c \
  -o build/docker/posaware/oh_particle_adapter.o
mpicc -DOH_POS_AWARE -Iinclude -c src/c/oh_context.c \
  -o build/docker/posaware/oh_context.o
mpicc -DOH_POS_AWARE -Iinclude -c src/c/oh_fortran_v2.c \
  -o build/docker/posaware/oh_fortran_v2.o
mpicc -DOH_POS_AWARE -Iinclude -c src/c/ohhelp1.c \
  -o build/docker/posaware/ohhelp1.o
mpicc -DOH_POS_AWARE -Iinclude -c src/c/ohhelp2.c \
  -o build/docker/posaware/ohhelp2.o
mpicc -DOH_POS_AWARE -Iinclude -c src/c/ohhelp3.c \
  -o build/docker/posaware/ohhelp3.o
gfortran -cpp -Iinclude -Jbuild/docker/posaware -c src/fortran/oh_v2.F90 \
  -o build/docker/posaware/oh_v2.o
mpifort -cpp -Iinclude -Ibuild/docker/posaware -Jbuild/docker/posaware \
  tests/test_oh_context_lifecycle_fortran.F90 \
  build/docker/posaware/oh_v2.o build/docker/posaware/oh_context.o \
  build/docker/posaware/oh_fortran_v2.o \
  build/docker/posaware/oh_particle_adapter.o \
  build/docker/posaware/oh_load_balance.o build/docker/posaware/ohhelp1.o \
  build/docker/posaware/ohhelp2.o build/docker/posaware/ohhelp3.o \
  -o build/docker/posaware/test_oh_context_lifecycle_fortran
run_mpi -n 2 build/docker/posaware/test_oh_context_lifecycle_fortran
mpifort -cpp -Iinclude -Ibuild/docker -Jbuild/docker \
  tests/test_oh_context_lifecycle_fortran.F90 \
  build/docker/oh_v2.o build/docker/oh_type.o build/docker/oh_context.o \
  build/docker/oh_fortran_v2.o build/docker/oh_particle_adapter.o \
  build/docker/oh_load_balance.o build/docker/ohhelp1.o \
  build/docker/ohhelp2.o build/docker/ohhelp3.o \
  -o build/docker/test_oh_context_lifecycle_fortran
run_mpi -n 1 build/docker/test_oh_context_lifecycle_fortran
run_mpi -n 2 build/docker/test_oh_context_lifecycle_fortran
mpicc -Iinclude tests/test_ohhelp_c_header.c \
  -c -o build/docker/test_ohhelp_c_header.o
mpicc -Iinclude tests/test_ohhelp2_header.c \
  -c -o build/docker/test_ohhelp2_header.o
mpicc -Iinclude -DOH_DIMENSION=1 tests/test_ohhelp3_header.c \
  -c -o build/docker/test_ohhelp3_header_1d.o
mpicc -Iinclude -DOH_DIMENSION=2 tests/test_ohhelp3_header.c \
  -c -o build/docker/test_ohhelp3_header_2d.o
mpicc -Iinclude -DOH_DIMENSION=3 tests/test_ohhelp3_header.c \
  -c -o build/docker/test_ohhelp3_header_3d.o
mpicc -Iinclude tests/test_ohhelp4p_header.c \
  -c -o build/docker/test_ohhelp4p_header.o
mpicc -Iinclude tests/test_ohhelp4s_header.c \
  -c -o build/docker/test_ohhelp4s_header.o
mpicc -Iinclude tests/test_public_header_reinclude.c \
  -c -o build/docker/test_public_header_reinclude.o
mpicc -Iinclude tests/test_ohhelp_f_reinclude.c \
  -c -o build/docker/test_ohhelp_f_reinclude.o
mpicc -Iinclude sample/level3_custom_particle.c \
  -c -o build/docker/level3_custom_particle.o
gcc -Iinclude tests/test_oh_load_balance.c src/c/oh_load_balance.c \
  -o build/docker/test_oh_load_balance
build/docker/test_oh_load_balance

mpicc -Iinclude tests/test_oh_particle_adapter.c src/c/oh_particle_adapter.c \
  -o build/docker/test_oh_particle_adapter
run_mpi -n 1 build/docker/test_oh_particle_adapter

mpicc -Iinclude tests/test_oh_particle_adapter_callbacks.c \
  src/c/oh_particle_adapter.c \
  -o build/docker/test_oh_particle_adapter_callbacks
build/docker/test_oh_particle_adapter_callbacks

mpicc -Iinclude -Isrc/c tests/test_oh_particle_buffer.c \
  src/c/oh_particle_adapter.c -o build/docker/test_oh_particle_buffer
run_mpi -n 1 build/docker/test_oh_particle_buffer

bash tests/test_particle_contract_audit.sh
