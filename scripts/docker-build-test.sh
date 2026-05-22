#!/usr/bin/env bash
set -eu

mkdir -p build/docker

mpicc -Iinclude -c src/c/oh_load_balance.c -o build/docker/oh_load_balance.o
mpicc -Iinclude -c src/c/oh_particle_adapter.c -o build/docker/oh_particle_adapter.o
mpicc -Iinclude -c src/c/oh_context.c -o build/docker/oh_context.o
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

gcc -Iinclude tests/test_oh_context_header.c \
  -c -o build/docker/test_oh_context_header.o
mpicc -Iinclude tests/test_ohhelp_c_header.c \
  -c -o build/docker/test_ohhelp_c_header.o
mpicc -Iinclude tests/test_ohhelp4p_header.c \
  -c -o build/docker/test_ohhelp4p_header.o
mpicc -Iinclude tests/test_ohhelp4s_header.c \
  -c -o build/docker/test_ohhelp4s_header.o
gcc -Iinclude tests/test_oh_load_balance.c src/c/oh_load_balance.c \
  -o build/docker/test_oh_load_balance
build/docker/test_oh_load_balance

mpicc -Iinclude tests/test_oh_particle_adapter.c src/c/oh_particle_adapter.c \
  -o build/docker/test_oh_particle_adapter
mpirun -n 1 build/docker/test_oh_particle_adapter

mpicc -Iinclude tests/test_oh_particle_adapter_callbacks.c \
  src/c/oh_particle_adapter.c \
  -o build/docker/test_oh_particle_adapter_callbacks
build/docker/test_oh_particle_adapter_callbacks

mpicc -Iinclude -Isrc/c tests/test_oh_particle_buffer.c \
  src/c/oh_particle_adapter.c -o build/docker/test_oh_particle_buffer
mpirun -n 1 build/docker/test_oh_particle_buffer
