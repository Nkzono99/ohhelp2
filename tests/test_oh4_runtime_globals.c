/* Level 4 Fortran runtime tests link the real ohhelp2 object, which owns
   gridMask/logGrid. Keep this file non-empty without defining compatibility
   globals that conflict on GCC's default -fno-common builds. */
int oh_test_oh4_runtime_globals_anchor;
