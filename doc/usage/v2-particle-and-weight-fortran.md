# v2 Particle Layout and Weighted Load (Fortran)

Language: [C mirror](v2-particle-and-weight.md) | Fortran

v2 では、従来の固定 `type(oh_particle)` 前提だけでなく、Fortran 側の
custom particle layout も扱えるようにします。このページは Fortran 版です。

## Particle adapter

Fortran では `ohhelp_v2` module の opaque handle を使います。

```fortran
use iso_c_binding
use ohhelp_v2

type(oh_particle_adapter_handle) :: adapter
```

adapter の実体は C 側にあり、Fortran 側では `type(c_ptr)` を直接操作しません。
通常は `oh_particle_adapter_create_byte()` で byte MPI datatype 付き adapter を
作り、field offset を設定してから context に渡します。

```fortran
type(oh_context_handle) :: ctx
integer(c_int) :: ierr

ctx = oh_default_context()
call oh_particle_adapter_create_byte(adapter, c_sizeof(sample), ierr)
call oh_context_set_particle_adapter(ctx, adapter)
```

使い終わった adapter は破棄します。

```fortran
call oh_particle_adapter_destroy(adapter)
```

## `mpi_type` の標準的な作り方

plain data の粒子型なら、`oh_particle_adapter_create_byte()` を使います。
これは C 側で `MPI_BYTE` の contiguous/resized datatype を作り、adapter の
stride と extent を合わせます。

```fortran
type, bind(C) :: pic_particle
  real(c_double) :: x, y, z
  real(c_double) :: vx, vy, vz
  integer(c_int) :: region
  integer(c_int) :: species
end type

type(pic_particle), target :: sample
type(oh_particle_adapter_handle) :: adapter
integer(c_int) :: ierr

call oh_particle_adapter_create_byte(adapter, c_sizeof(sample), ierr)
if (ierr /= 0_c_int) stop
```

既に Fortran 側で MPI datatype を作っている場合は、
`oh_particle_adapter_set_mpi_type()` に Fortran MPI handle を渡せます。

```fortran
call oh_particle_adapter_set_mpi_type(adapter, particle_mpi_type)
```

## region/species field の標準形

`bind(C)` な粒子型を使い、`c_loc()` から offset を計算します。

```fortran
integer(c_size_t) :: region_offset, species_offset

region_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%region))
species_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%species))

call oh_particle_adapter_use_int_fields(adapter, region_offset, species_offset)
```

region/species が `integer(c_long_long)` など `integer(c_int)` 以外の場合は、
field size も渡します。

```fortran
call oh_particle_adapter_use_integer_fields( &
  adapter, region_offset, c_sizeof(sample%region), &
  species_offset, c_sizeof(sample%species))
```

single-species の粒子配列では species field を省略できます。

```fortran
call oh_particle_adapter_use_single_species_int_region(adapter, region_offset)
```

## callback adapter

offset だけでは表現できない layout では、`bind(C)` callback を `c_funloc()` で
渡します。

```fortran
call oh_particle_adapter_set_callbacks( &
  adapter, &
  c_funloc(my_get_region), &
  c_funloc(my_set_region), &
  c_funloc(my_get_species), &
  c_funloc(my_map_to_neighbor), &
  c_funloc(my_map_to_subdomain))
```

callback の interface は `ohhelp_v2` に公開されています。

```fortran
function my_get_region(adapter, particle, ps) bind(C) result(region)
  use iso_c_binding
  type(c_ptr), value :: adapter
  type(c_ptr), value :: particle
  integer(c_int), value :: ps
  integer(c_long_long) :: region
end function
```

## Level 3 mapping callback の標準形

Level 3 では、position field offset を adapter に設定すると、OhHelp が持つ
subdomain geometry を使って `map_to_neighbor` / `map_to_subdomain` を設定できます。

```fortran
integer(c_size_t) :: x_offset, y_offset, z_offset

x_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%x))
y_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%y))
z_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%z))

call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                    y_offset, z_offset)
call oh_context_set_particle_adapter(ctx, adapter)
```

粒子 push 中は、利用側の region field と histogram を更新します。

```fortran
dst = oh_context_map_particle_to_neighbor(ctx, p%x, p%y, p%z, ps)
p%region = dst
nphgram(self+1,s) = nphgram(self+1,s) - 1
nphgram(dst+1,s) = nphgram(dst+1,s) + 1
```

## `nid` と remove の扱い

default `type(oh_particle)` では `nid` が region id と remove marker を兼ねます。
custom particle layout では、adapter に登録した region field が同じ役割を持ちます。

- 通常粒子: region が負値、または Level 3/4 mapping が `-1` を返す粒子は、
  次の transfer で送受信・保持対象から外れます。
- default Fortran API: Fortran には pointer-return helper はなく、injection 後に
  remap/remove する場合は、`pbuf` の injection 領域にコピーされた
  `type(oh_particle)` 要素を渡します。
- `ohhelp_v2` context API: `oh_context_inject_particle_get()` が返す `type(c_ptr)` を
  `oh_context_remap_injected_particle()` /
  `oh_context_remove_injected_particle()` に渡します。

## 既存 `type(oh_particle)` を使う場合

既存の `type(oh_particle)` を使う場合、adapter 設定は不要です。

```fortran
type(oh_particle), allocatable :: pbuf(:)

maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin)
allocate(pbuf(maxlocalp))
call oh_init(..., pbuf, pbase, maxlocalp, ...)
```

## Region weight

Fortran では `real(c_double)` または `real*8` 配列を渡します。

```fortran
real(c_double), target :: weights(nregions)

weights(:) = 1.0_c_double
weights(hot_region) = 2.5_c_double
call oh_set_region_weights(weights)
```

`ohhelp_v2` context API でも同じ設定ができます。optional の `weights` を省略すると
default context の region weight を 1.0 に戻します。

```fortran
type(oh_context_handle) :: ctx

ctx = oh_default_context()
call oh_context_set_region_weights(ctx, weights)
call oh_context_set_region_weights(ctx)
```

## いつ重みを設定するか

基本は `oh_init()` の後、最初の `oh_transbound()` の前に設定します。

```fortran
call oh_init(...)
call oh_set_region_weights(weights)

currmode = oh_transbound(0, stats)
```

重みが時間発展で変わる場合は、次の `oh_transbound()` の前に更新します。

```fortran
do step = 1, nstep
  call update_region_cost_model(weights, step)
  call oh_set_region_weights(weights)

  call push_particles()
  currmode = oh_transbound(currmode, stats)
end do
```

## 現在の実装上の制約

- context API は default context facade です。完全な複数 context 独立運用は
  v2.x の対象です。
- Fortran custom particle layout は `bind(C)` と `iso_c_binding` を前提にします。
- Level 4p/4s は v2.x 対応対象で、v2.0 の supported scope には含めません。
