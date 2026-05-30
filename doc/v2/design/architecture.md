# Architecture

OhHelp v2 is the independent `ohhelp2` line. The supported v2 surface is built
around explicit context objects, user-owned particle layouts, and weighted load
balancing.

For historical v1 behavior and the original PDF-derived manuals, see
[`../../v1/`](../../v1/). v2 documentation does not treat v1 signatures as the
primary user interface.

## Public Layers

The C v2 API is centered on:

- `oh_context` from `include/oh_context.h`
- `oh_particle_adapter` from `include/oh_particle_adapter.h`
- mode constants from `include/oh_mode.h`
- particle ownership constants from `include/oh_particle_ownership.h`

The Fortran v2 API is centered on one module:

```fortran
use iso_c_binding
use ohhelp_v2
```

v2 Fortran documentation uses `ohhelp_v2` only. Other module families are
covered only by the historical v1 material.

## Context Boundary

An `oh_context` owns the OhHelp runtime state for one logical library instance:
communicator state, particle configuration, particle adapter, region weights,
particle buffers, particle accounting arrays, Level 3 geometry descriptors, and
communication work buffers.

The default context exists for migration and simple single-instance programs.
New code should prefer heap-owned context objects created with
`oh_context_create()` or, from Fortran, `oh_context_create()`.

The implementation keeps state-backed internal entry points for Level 1-3 so
that public wrappers and explicit context calls use the same execution path.
That is the guardrail for reducing process-wide mutable globals.

## Particle Boundary

v2 must not require the application to store particles as `S_particle` or
`type(oh_particle)`. OhHelp sees particles through an adapter contract:

- byte stride,
- MPI datatype or byte-copy fallback,
- region getter/setter,
- species getter,
- optional position fields or mapping callbacks.

The adapter is the only supported boundary for user particle layout semantics.
See [Particle Adapter](particle-adapter.md).

## Load Boundary

v2 balancing is weighted-load based. Particle count is still the integer unit
that moves across ranks, but the scheduling metric is:

```text
region load = local particle count in the region * region weight
target load = global weighted load / number of nodes
```

See [Weighted Load Balancing](load-balancing.md).

## Supported Scope

v2 focuses on Level 1-4:

- Level 1: communicator and rebalance schedule.
- Level 2: particle buffer transfer.
- Level 3: geometry mapping and field-border exchange.
- Level 4p/4s: packed-grid particle mapping and border exchange.

Level 4p/4s use the v2 particle adapter boundary for default and custom
particle layouts and are covered by runtime checks for weighted primary and
secondary transbound.
