# Level Scope

OhHelp levels describe how much of a PIC integration is delegated to the
library. v2.0 supports Level 1-3 as the public target. Level 4p/4s are v2.x
work.

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

Level 4p/4s are not v2.0 supported APIs. They remain under compile coverage and
internal migration guardrails. Full custom-layout support for their packed-grid
semantics is a v2.x target.
