# Level Scope

OhHelp levels describe how much of a PIC integration is delegated to the
library. The v2 public target covers Level 1-4.

For v1 level descriptions, see [`../../v1/`](../../v1/).

## Level 1

Level 1 manages the communicator and load-balance schedule. The application
keeps ownership of particle movement.

Use Level 1 when the simulation already has its own particle exchange logic but
wants OhHelp's scheduling and communicator support.

## Level 2

Level 2 adds particle buffer transfer. The application provides a particle
adapter and accounting arrays; OhHelp moves particles according to region and
species information.

Use Level 2 when particle transfer can be expressed as movement of fixed-stride
particle elements.

## Level 3

Level 3 adds geometry mapping and field-border exchange. The context stores
subdomain geometry, boundary descriptors, and field descriptors.

Use Level 3 for standard PIC integrations where OhHelp should handle particle
mapping across subdomains and field halo exchange.

## Level 4p/4s

Level 4p/4s are supported v2 APIs through the Level 4 C/Fortran entry points.
They remain implemented through the default-context state bridge, but the
bridge is part of the supported Level 4 surface rather than a temporary
compile-only path.

Adapter helpers and `struct oh_state` fields keep packed-grid ids, species, and
coordinates behind the particle adapter boundary. The runtime coverage includes
default particle storage, custom adapter storage, injected-particle accounting,
and weighted primary and secondary transbound for Level 4p/4s.

Weighted secondary transbound is part of the supported Level 4 path. Transfer
schedules still move integer particle counts, while the balancing decision uses
per-region weights when configured.

The fpm package may compile all C translation units even when a downstream
project only uses Level 1-3. The Level 4 source bodies are therefore enabled
only when `OH_LIB_LEVEL_4P` or `OH_LIB_LEVEL_4S` is explicitly defined; default
fpm dependency builds do not require downstream users to provide a Level 4
macro.
