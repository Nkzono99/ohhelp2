# Index Conventions

OhHelp v2 keeps the context API close to the C implementation. Fortran
`ohhelp_v2` wrappers are ISO_C_BINDING facades, not v1-style index translators,
except where a helper is explicitly named `legacy` or `raw`.

For historical v1 Fortran behavior, see [`../../v1/`](../../v1/).

## Rule Of Thumb

- Context APIs use C-style zero-based indices.
- v1-compatible wrappers may accept one-based ids, but must convert before
  calling the context API.
- Particle species base is part of the particle adapter contract, not a
  context-wide indexing mode.
- Offsets and counts are never Fortran lower-bound indices.

## Context API Indices

These are zero-based in both C and `ohhelp_v2`:

| API / data | Convention |
| --- | --- |
| `oh_context_bcast_field`, `oh_context_reduce_field`, `oh_context_allreduce_field` `ftype` | zero-based field descriptor index |
| `oh_context_exchange_borders` `ctype` | zero-based communication-field descriptor index |
| `oh_context_map_particle_to_neighbor` `ps` | `0` primary, `1` secondary |
| `oh_context_configure_level3` `bcond` / `bounds` | zero-based boundary/subdomain ids, with negative values retaining their sentinel meaning |
| `oh_context_configure_level3` `ftypes`, `cfields`, `ctypes`, `fsizes` | passed through in the C descriptor layout; no one-based correction is applied |
| `oh_context_bind_region_ids` / `oh_context_get_region_ids` `sdid` | zero-based active region ids; `sdid(2) == -1` means no active secondary region |
| `nphgram`, `totalp`, `pbase` | packed count/offset arrays |
| `oh_particle_field_offset` results | byte offsets from the start of a particle record |

For a Fortran `integer(c_int) :: pbase(3)`, `pbase(2)` is the primary/secondary
split offset and `pbase(3)` is the local particle end offset. They are offsets
into the particle buffer, not Fortran array indices.

## v1-Compatible Fortran Wrappers

Historical Fortran Level 3 field routines accepted one-based field ids. The old
entry points convert before reaching the state implementation:

```c
oh3_bcast_field_state(state, pfld, sfld, *ftype - 1);
oh3_exchange_borders_state(state, pfld, sfld, *ctype - 1, *bcast);
```

If an application provides a v1-style wrapper around `ohhelp_v2`, it should do
the same conversion:

```fortran
if (ftype <= 0_c_int) error stop "OhHelp field type id must be one-based"
context_ftype = ftype - 1_c_int
call oh_context_bcast_field(ctx, c_loc(pfld), c_loc(sfld), context_ftype)
```

The same rule applies to `ctype` for border exchange.

## Level 3 Geometry Helpers

`oh_context_configure_level3()` is the direct v2 context API. It expects the C
Level 3 descriptor representation, including zero-based boundary ids.

`oh_context_configure_level3_legacy()` is the migration helper for existing
Fortran geometry arrays. It preserves the old active-decomposition sentinel and
converts one-based `bcond` / `bounds` entries to the zero-based context
representation. It does not convert `ftype` or `ctype` values passed later to
field operation calls; those call-site ids must still be converted by a
v1-style wrapper.

`oh2_init_raw()` and `oh3_init_raw()` call the historical raw init path and
therefore follow the v1 Fortran conventions for the raw init argument list.

## Particle Species

Species ids are normalized to zero-based internal indices before OhHelp indexes
species arrays. The external particle layout declares its base through the
adapter:

- C adapter helpers default to `species_base = 0`.
- Fortran integer-field adapter helpers set `species_base = 1`.
- single-species adapter helpers ignore the species base and map every particle
  to internal species `0`.
- callback adapters keep the default base unless the application calls
  `oh_particle_adapter_set_species_base()`.

This species-base rule is intentionally separate from field type and boundary
id indexing.
