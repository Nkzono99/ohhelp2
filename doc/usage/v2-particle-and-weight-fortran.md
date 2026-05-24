# v2 Particle Layout and Weighted Load (Fortran)

Language: [C mirror](v2-particle-and-weight.md) | Fortran

v2 では、従来の固定 `type(oh_particle)` 前提を保ちながら、Fortran からも
context facade と particle adapter 設定 API にアクセスできるようにしています。
このページは Fortran 版です。

v2.0 で supported として扱う実用経路は、Level 1-3 を
`type(oh_particle)` 配列で使う形です。`ohhelp_v2` の adapter handle は
offset/callback を設定できる低レベル bridge です。任意 Fortran particle layout は
`oh2_init_raw()` / `oh3_init_raw()` に `c_loc()` で配列と粒子バッファを渡して
初期化します。

## Particle adapter

Fortran では `ohhelp_v2` module の opaque handle を使います。このページで扱う
context / adapter / raw init bridge だけなら、`use ohhelp3` は不要です。
従来の `oh_init()` / `oh_transbound()` alias や default `type(oh_particle)`
配列の Level 3 API も同じ translation unit で使う場合だけ、`use ohhelp3`
を併用します。

```fortran
use iso_c_binding
use ohhelp_v2

type(oh_particle_adapter_handle) :: adapter
```

adapter の実体は C 側にあり、Fortran 側では `type(c_ptr)` を直接操作しません。
通常は `oh_particle_adapter_create_byte()` で byte MPI datatype 付き adapter を
作り、field offset を設定してから context に渡します。この adapter 設定は
default context facade に反映されます。

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

`type(oh_particle)` を使う通常の Fortran Level 3 では、既存の
`oh_map_particle_to_neighbor()` を使って粒子の `nid` と histogram を更新します。

```fortran
dst = oh_map_particle_to_neighbor(p%x, p%y, p%z, ps)
p%nid = dst
nphgram(self+1,s) = nphgram(self+1,s) - 1
nphgram(dst+1,s) = nphgram(dst+1,s) + 1
```

`oh_context_map_particle_to_neighbor()` は `ohhelp_v2` の context facade から同じ
mapping を呼ぶ低レベル API です。`bind(C)` な custom particle 型を使う場合は、
adapter に登録した region field を利用側で更新します。

## custom particle layout の raw init

任意の Fortran particle layout を Level 2/3 に渡す場合は、adapter 設定後に
`type(c_ptr)` の粒子バッファ pointer を raw init bridge へ渡します。

```fortran
type, bind(C) :: pic_particle
  real(c_double) :: x, y, z
  real(c_double) :: vx, vy, vz
  integer(c_int) :: region
  integer(c_int) :: species
end type

type(pic_particle), target :: particles(maxlocalp)
type(oh_mycomm_v2), target :: mycomm
type(c_ptr) :: raw_particles

raw_particles = c_loc(particles(1))
call oh3_init_raw(c_loc(sdid(1)), nspec, maxfrac, &
                  c_loc(nphgram(1,1,1)), c_loc(totalp(1,1)), &
                  raw_particles, c_loc(pbase(1)), maxlocalp, &
                  c_loc(mycomm), c_loc(nbor(1,1,1)), c_loc(pcoord(1)), &
                  c_loc(sdoms(1,1,1)), c_loc(scoord(1,1)), nbound, &
                  c_loc(bcond(1,1)), c_loc(bounds(1,1,1)), &
                  c_loc(ftypes(1,1)), c_loc(cfields(1)), &
                  c_loc(ctypes(1,1,1,1)), c_loc(fsizes(1,1,1)), &
                  stats, repiter, verbose)
```

`raw_particles` は `intent(inout)` です。`c_null_ptr` を渡した場合、OhHelp が
adapter stride に基づいて粒子バッファを確保し、返された pointer を
`c_f_pointer()` で Fortran pointer に戻せます。利用側で配列を確保して
`c_loc(particles(1))` を渡す場合は、その配列がそのまま OhHelp の粒子バッファに
なります。

### raw init の lifetime / ownership contract

現状の `oh2_init_raw()` / `oh3_init_raw()` は、粒子バッファ pointer を
default context state に保持します。そのため、後続の `oh_transbound()` /
`oh_context_transbound2()` / `oh_context_transbound3()` は、init 時に登録された
粒子バッファを読み書きします。

同じく、`nphgram`、`totalp`、`pbase` も OhHelp が pointer を保持する
mutable accounting state です。`nphgram` は region/species histogram として
`oh_transbound()` の入力になり、transfer 後に OhHelp 側でクリア・再構成されます。
`totalp` は primary/secondary の species 別粒子数として更新され、`pbase` は
primary / secondary / total local particles の境界値として更新されます。
これらも borrowed storage なので、次の init または accounting unbind まで
deallocate、再確保、shape 変更してはいけません。

`c_loc(particles(1))` を渡す場合、その配列は user-owned / borrowed buffer です。
OhHelp は pointer を借りるだけで、配列の lifetime は利用側が管理します。
`oh_transbound()` を呼ぶ可能性がある間は、対象配列を `deallocate`、
`move_alloc`、再確保、または shape 変更してはいけません。粒子の並び、region
field、injection 領域、primary/secondary の境界は OhHelp が更新します。

`c_null_ptr` を渡す場合は、OhHelp-owned buffer として OhHelp が粒子バッファを
確保し、`raw_particles` に pointer を返します。この場合も pointer は
`oh_transbound()` が使い続けるため、利用側で勝手に解放してはいけません。

adapter handle、adapter に渡した MPI datatype、callback、callback が参照する
`user_data` 相当の状態も、粒子バッファを OhHelp に登録している間は有効である
必要があります。active な adapter を破棄する前に
`oh_context_unbind_particles()` で粒子バッファとの対応を解除してください。

v2 の context API では、粒子バッファの保持を明示する
`oh_context_bind_particles()` / `oh_context_unbind_particles()` を使えます。
`OH_PARTICLES_BORROWED` は user-owned storage を借用し、
`OH_PARTICLES_OWNED` は OhHelp が active adapter stride に基づいて確保した
storage を `unbind` 時に解放します。default context の binding を明示する API と
heap-owned non-default context の binding に使えます。Fortran runtime smoke では
2 つの non-default context の基本的な独立性も検証しています。

```fortran
raw_particles = c_loc(particles(1))
call oh_context_bind_particles(ctx, raw_particles, maxlocalp, &
                               OH_PARTICLES_BORROWED)
currmode = oh_context_transbound3(ctx, currmode, stats)
call oh_context_unbind_particles(ctx)
```

OhHelp-owned storage を使う場合は `raw_particles = c_null_ptr` と
`OH_PARTICLES_OWNED` を渡します。戻り値は `raw_particles` に書き戻されるため、
必要なら `c_f_pointer()` で Fortran pointer に戻します。

`oh_context_unbind_particles()` が解除するのは粒子バッファ binding です。
`nphgram`、`totalp`、`pbase` は matching API の
`oh_context_bind_particle_accounting()` /
`oh_context_unbind_particle_accounting()` で明示できます。

```fortran
raw_nphgram = c_loc(nphgram(1,1,1))
raw_totalp = c_loc(totalp(1,1))
raw_pbase = c_loc(pbase(1))
call oh_context_bind_particle_accounting(ctx, raw_nphgram, raw_totalp, &
                                         raw_pbase, OH_PARTICLES_BORROWED)
currmode = oh_context_transbound3(ctx, currmode, stats)
call oh_context_unbind_particle_accounting(ctx)
```

OhHelp-owned accounting storage を使う場合は 3 つの pointer を `c_null_ptr` にして
`OH_PARTICLES_OWNED` を渡します。戻り値は各 pointer に書き戻されます。
borrowed accounting array は bind 時には clear されません。従来どおり transfer 前に
`nphgram` を利用側で初期化・更新してください。

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

currmode = oh_transbound(OH_MODE_NORMAL_PRIMARY, stats)
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

- context API は default context facade に加えて、heap-owned non-default context の
  Level 1-3 `transbound`、Level 3 geometry/field descriptor、mapping、field facade
  まで対応しています。C runtime smoke では 2 つの non-default context を同一
  communicator 上で同時に保持する経路も検証しています。
- Fortran custom particle layout の adapter 設定と raw init は `bind(C)` と
  `iso_c_binding` を前提にします。
- Level 4p/4s は v2.x 対応対象で、v2.0 の supported scope には含めません。
