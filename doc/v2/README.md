# OhHelp v2 Documentation

This directory contains documentation for the independent `ohhelp2` v2 line.

For historical OhHelp manuals and v1-derived Markdown, start at
[`../v1/`](../v1/). v2 documentation focuses on the context API, particle
adapter contract, and weighted-load behavior.

- [`usage/`](usage/README.md) is the user-facing guide for C and Fortran.
- [`design/`](design/README.md) is the v2 design specification.
- [`design/index-conventions.md`](design/index-conventions.md) summarizes
  zero-based context API indices and v1-style Fortran conversion points.

The supported v2 surface covers Level 1-4. Level 4p/4s are selected with
`OH_LIB_LEVEL_4P` or `OH_LIB_LEVEL_4S` and are covered by runtime checks for
default/custom particle layouts and weighted transbound.
