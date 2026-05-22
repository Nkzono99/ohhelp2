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

v2.0 targets supported use of Level 1-3. Level 4p/4s source files are kept under
compile coverage, but the supported v2 API for Level 4 is deferred to v2.x.

Fortran users can use the Level 1-3 modules with the default `type(oh_particle)`
layout. The `ohhelp_v2` module also exposes the v2 context facade and opaque
particle adapter handles for custom Fortran particle layouts.

## Install
``` toml
[dependencies]
ohhelp2 = { git = "https://github.com/Nkzono99/ohhelp2" }
```

## Usage documentation

- [OhHelp v2 Usage Guide](doc/usage/README.md)
- [PIC integration lifecycle](doc/usage/pic-lifecycle.md)
- [API by OhHelp level](doc/usage/api-by-level.md)
- [v2 particle layout and weighted load](doc/usage/v2-particle-and-weight.md)

## License notice

This repository contains code derived from the original OhHelp software.
Some original source files include a non-commercial-use notice.

The MIT License applies only to files or portions explicitly authored by Jin Nakazono,
unless otherwise stated. It does not override the license terms of the original OhHelp code.

Commercial use of the repository as a whole has not been confirmed.
Please contact the relevant rights holders before any commercial use.

## References
[^nakashima_2019]: Nakashima, Hiroshi, Yohei Miyake, Hideyuki Usui, and Yoshiharu Omura. 2009. “OhHelp: A Scalable Domain-Decomposing Dynamic Load Balancing for Particle-in-Cell Simulations.” In Proceedings of the 23rd International Conference on Supercomputing, 2009, Yorktown Heights, NY, USA, June 8-12, 2009, 90–99. unknown.
