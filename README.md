# ohhelp2

OhHelp v2 is an independent v2 line of the OhHelp dynamic load balancing
library for MPI particle-in-cell simulations.

## About OhHelp Library Package

OhHelp library is a dynamic load balancing and scalability library that supports massively parallel particle-in-cell simulations using MPI.
Each process handles the particle calculations within the partitioned area while taking on some of the particles handled by other processes so that the overall load is balanced.
Developed by Nakashima et al. [^nakashima_2019] and used mainly in the field of plasma particle simulation, it has been shown to be effective for models such as plasma-satellite interaction and magnetospheric plasma, where the number of particles is time-varying and non-uniform.

This repository is no longer a compatibility-preserving fork of the v1 package.
The v2 line intentionally changes APIs and implementation boundaries where that
is useful for cleaner context handling, external particle layouts, and weighted
load balancing.

The current v2 line supports Level 1-3. Level 4p/4s source files are kept under
compile coverage, but the supported v2 API for Level 4 is deferred to v2.x.

Fortran users can use the Level 1-3 modules with the default `type(oh_particle)`
layout. The `ohhelp_v2` module also exposes the v2 context facade and opaque
particle adapter handles. Arbitrary Fortran particle layouts can use the
`ohhelp_v2` raw init bridge (`oh2_init_raw()` / `oh3_init_raw()`) with `c_loc()`
and a registered particle adapter.

C and Fortran both have runtime-covered non-default context paths for Level 1-3.
The Docker smoke tests create two heap-owned contexts on the same communicator,
configure Level 3 geometry, bind independent particle/accounting state, and call
`oh_context_transbound1/2/3` through C and `ohhelp_v2`.

## Install
``` toml
[dependencies]
ohhelp2 = { git = "https://github.com/Nkzono99/ohhelp2" }
```

## Usage documentation

- [OhHelp v2 Usage Guide](doc/usage/README.md)
- PIC integration lifecycle: [C](doc/usage/pic-lifecycle.md), [Fortran](doc/usage/pic-lifecycle-fortran.md)
- API by OhHelp level: [C](doc/usage/api-by-level.md), [Fortran](doc/usage/api-by-level-fortran.md)
- v2 particle layout and weighted load: [C](doc/usage/v2-particle-and-weight.md), [Fortran](doc/usage/v2-particle-and-weight-fortran.md)

## License notice

This repository contains code derived from the original OhHelp software.
Some original source files include a non-commercial-use notice.

The MIT License applies only to files or portions explicitly authored by Jin Nakazono,
unless otherwise stated. It does not override the license terms of the original OhHelp code.

Commercial use of the repository as a whole has not been confirmed.
Please contact the relevant rights holders before any commercial use.

## References
[^nakashima_2019]: Nakashima, Hiroshi, Yohei Miyake, Hideyuki Usui, and Yoshiharu Omura. 2009. “OhHelp: A Scalable Domain-Decomposing Dynamic Load Balancing for Particle-in-Cell Simulations.” In Proceedings of the 23rd International Conference on Supercomputing, 2009, Yorktown Heights, NY, USA, June 8-12, 2009, 90–99. unknown.
