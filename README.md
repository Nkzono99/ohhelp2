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

The current v2 line supports Level 1-4. Level 4p/4s are available through the
Level 4 C/Fortran API with the v2 particle adapter and weighted-load
scheduling support.

Fortran v2 users should use the `ohhelp_v2` module for the Level 1-3 context
facade, opaque particle adapter handles, and raw init bridge. Arbitrary
Fortran particle layouts can use `oh2_init_raw()` / `oh3_init_raw()` with
`c_loc()` and a registered particle adapter.

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

- [OhHelp v2 Usage Guide](doc/v2/usage/README.md)
- [OhHelp v2 Design Guide](doc/v2/design/README.md)
- PIC integration lifecycle: [C](doc/v2/usage/pic-lifecycle.md), [Fortran](doc/v2/usage/pic-lifecycle-fortran.md)
- API by OhHelp level: [C](doc/v2/usage/api-by-level.md), [Fortran](doc/v2/usage/api-by-level-fortran.md)
- v2 particle layout and weighted load: [C](doc/v2/usage/v2-particle-and-weight.md), [Fortran](doc/v2/usage/v2-particle-and-weight-fortran.md)

## Development checks

Run the standard local gate from the repository root:

```sh
bash scripts/test.sh
```

This gate expects MPI C, Fortran, and C++ compiler wrappers (`mpicc` or
`MPICC`, `mpifort` or `MPIFC`, `mpic++` or `MPICXX`), `mpirun` or `MPIRUN`,
and `timeout`. MPI test timeout defaults to `60s` and can be overridden with
`TEST_TIMEOUT`.

Historical v1 Markdown is generated from PDFs with Python 3.10 or newer. Check
reproducibility separately after installing `requirements-doc.txt`:

```sh
bash scripts/check-v1-markdown.sh
```

If the default `python3` is older, pass a newer interpreter explicitly:

```sh
PYTHON=python3.11 bash scripts/check-v1-markdown.sh
```

## License notice

This repository contains code derived from the original OhHelp software.
Some original source files include a non-commercial-use notice.

Repository-level license information is summarized in [LICENSE.md](LICENSE.md).
The original OhHelp notice is reproduced in
[`LICENSES/OhHelp-NonCommercial.txt`](LICENSES/OhHelp-NonCommercial.txt), and
the MIT text for contributions by Jin Nakazono is reproduced in
[`LICENSES/MIT.txt`](LICENSES/MIT.txt).

The MIT License applies only to files or portions explicitly authored by Jin
Nakazono, unless otherwise stated. It does not override the license terms of
the original OhHelp code.

Commercial use of the repository as a whole has not been confirmed.
Please contact the relevant rights holders before any commercial use.

## References
[^nakashima_2019]: Nakashima, Hiroshi, Yohei Miyake, Hideyuki Usui, and Yoshiharu Omura. 2009. “OhHelp: A Scalable Domain-Decomposing Dynamic Load Balancing for Particle-in-Cell Simulations.” In Proceedings of the 23rd International Conference on Supercomputing, 2009, Yorktown Heights, NY, USA, June 8-12, 2009, 90–99. unknown.
