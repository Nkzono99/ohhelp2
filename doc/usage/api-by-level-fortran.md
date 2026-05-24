# API by OhHelp Level (Fortran)

Language: [C mirror](api-by-level.md) | Fortran

OhHelp の level は、利用側がどこまでを自前で行い、どこからを OhHelp に
任せるかを決める境界です。このページは Fortran 版です。

v2.0 では Level 1-3 を supported scope とします。Level 4p/4s は v2.x で
対応を進める拡張であり、v2.0 の完了条件には含めません。

## 共通 API

Fortran では、従来 API と v2 facade で使う module が分かれます。
`ohhelp_f.h` の `oh_*` alias、`oh_init()`、`oh_transbound()`、default
`type(oh_particle)` 配列を使う場合は `ohhelp1` / `ohhelp2` / `ohhelp3`
module を使います。v2 context/adapter/raw-init API だけを使う場合は
`ohhelp_v2` module だけで足ります。

| 経路 | 必要な module |
| --- | --- |
| Level 1 legacy API | `use ohhelp1` |
| Level 2 legacy API | `use ohhelp2` |
| Level 3 legacy API | `use ohhelp3` |
| v2 context / adapter / raw init bridge | `use iso_c_binding`; `use ohhelp_v2` |
| legacy API と v2 facade の併用 | 対象 level の `ohhelp*`; `use ohhelp_v2` |

| API | 呼ぶ段階 | 目的 |
| --- | --- | --- |
| `oh_init()` | 初期化 | OhHelp の領域分割、通信、統計、内部配列を初期化する。 |
| `oh_transbound(currmode, stats)` | timestep 中の粒子 push 後 | 粒子移動、負荷分散、primary/secondary 遷移を実行する。 |
| `oh_neighbors()` | 初期化後 | 隣接領域情報を取得する。 |
| `oh_families()` | 初期化後 | helpand/helper family 情報を取得する。 |
| `oh_accom_mode()` | 必要時 | 現在の accommodation mode を取得する。 |
| `oh_broadcast()` | primary/secondary 同期 | family 内で primary 側データを secondary 側へ配る。 |
| `oh_all_reduce()` / `oh_reduce()` | primary/secondary 同期 | secondary 側の寄与を集約する。 |
| `oh_set_region_weights()` | `oh_init()` 後 | region ごとの計算コスト重みを設定する。 |

`currmode` には magic number ではなく、`ohhelp1` 系 module と `ohhelp_v2`
module が公開する定数を使います。

| 定数 | 値 | 意味 |
| --- | ---: | --- |
| `OH_MODE_NORMAL_PRIMARY` | 0 | primary だけを通常計算する。初期値。 |
| `OH_MODE_NORMAL_SECONDARY` | 1 | primary / secondary を通常計算する。 |
| `OH_MODE_REBALANCE_SECONDARY` | -1 | rebalance 後に secondary field broadcast が必要。 |
| `OH_MODE_ANY_PRIMARY` | 2 | primary 側で anywhere/accommodation mode を使う。 |
| `OH_MODE_ANY_SECONDARY` | 3 | secondary 側も含めて anywhere/accommodation mode を使う。 |

短い互換名として `OH_MODE_NORM_PRI` / `OH_MODE_NORM_SEC` /
`OH_MODE_REB_SEC` / `OH_MODE_ANY_PRI` / `OH_MODE_ANY_SEC` も提供します。

## Context API

Fortran では `ohhelp_v2` の opaque handle を使います。default context facade に加えて、
non-default context の初期 API も使えます。

```fortran
use iso_c_binding
use ohhelp_v2

type(oh_context_handle) :: ctx
real(c_double), target :: weights(nregions)
integer(c_int) :: currmode = OH_MODE_NORMAL_PRIMARY

ctx = oh_default_context()
call oh_context_set_region_weights(ctx, weights)
currmode = oh_context_transbound3(ctx, currmode, 0_c_int)
```

v2.x では process-global default context の解消に向けて、heap-owned context の
初期 API も追加しています。

```fortran
type(oh_context_handle) :: owned
integer(c_int) :: currmode = OH_MODE_NORMAL_PRIMARY
integer(c_int) :: ierr

call oh_context_create(owned, fortran_comm, ierr)
call oh_context_configure_particles(owned, nspec, maxfrac)
call oh_context_configure_level3(owned, c_loc(pcoord(1)), c_null_ptr, &
                                 c_loc(scoord(1,1)), nbound, &
                                 c_loc(bcond(1,1)), c_null_ptr, &
                                 c_loc(ftypes(1,1)), c_loc(cfields(1)), &
                                 c_loc(ctypes(1,1,1,1)), c_loc(fsizes(1,1,1)))
call oh_context_set_region_weights(owned, weights)
call oh_context_bind_particles(owned, particles, maxlocalp, &
                               OH_PARTICLES_BORROWED)
call oh_context_bind_particle_accounting(owned, nphgram, totalp, pbase, &
                                         OH_PARTICLES_BORROWED)
currmode = oh_context_transbound3(owned, currmode, 0_c_int)
call oh_context_destroy(owned)
```

現時点の non-default context は、species/max-fraction 設定、region weight、particle adapter、
particle buffer/accounting binding、Level 1/2 work buffer、Level 3 geometry/field
descriptor、Level 1-3 の `transbound` state を保持できます。Level 1 collective、
Level 3 mapping/field exchange も context state を使います。Fortran runtime smoke
では同一 communicator 上の 2 つの non-default context が異なる Level 3 geometry
を保持できることを 1 rank / 2 rank で確認し、C runtime smoke では context-local
adapter state の独立性も確認しています。

Level 1 collective、Level 2 injection、Level 3 mapping/field exchange も
`oh_context_*` から呼べます。field や particle を渡す API では `c_loc()` で
`type(c_ptr)` を渡します。

## v2.0 で推奨する Fortran 経路

v2.0 で安定して使う前提の Fortran 経路は、`ohhelp_f.h` と
`ohhelp1` / `ohhelp2` / `ohhelp3` module を使い、粒子配列には
`type(oh_particle)` を渡す形です。

`ohhelp_v2` は context facade と adapter handle を Fortran から触るための
低レベル bridge です。region weight、context wrapper、adapter の offset/callback
設定に加えて、任意の Fortran derived type 配列を渡す
`oh2_init_raw()` / `oh3_init_raw()` も compile-check されています。

`raw` init は、配列を `c_loc()` で渡す C interop API です。既存の
`oh_init()` alias は引き続き `type(oh_particle)` 配列向けの簡潔な経路として
残します。

## Level 1: スケジュールだけを使う

Level 1 は、粒子データを OhHelp に渡しません。利用側が粒子転送を実装し、
OhHelp から負荷分散に必要な通信・領域情報を得ます。

```fortran
#define OH_LIB_LEVEL 1
#include "ohhelp_f.h"
module sim_l1
  use ohhelp1
  implicit none
end module
```

初期化:

```fortran
call oh_init(sdid, nspec, maxfrac, nphgram, totalp, &
             rcounts, scounts, mycomm, nbor, pcoord, &
             stats, repiter, verbose)
```

timestep 中:

```fortran
call update_particle_histogram_by_user_code(nphgram)
currmode = oh_transbound(currmode, stats)
call exchange_particles_by_user_code(rcounts, scounts, mycomm)
```

## Level 2: 粒子バッファ転送まで任せる

Level 2 は、OhHelp が粒子バッファを移動します。従来 Fortran API では
`type(oh_particle)` 配列を渡します。

```fortran
#define OH_LIB_LEVEL 2
#include "ohhelp_f.h"
module sim_l2
  use ohhelp2
  implicit none
end module
```

初期化:

```fortran
maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin)
allocate(pbuf(maxlocalp))

call oh_init(sdid, nspec, maxfrac, nphgram, totalp, &
             pbuf, pbase, maxlocalp, mycomm, nbor, pcoord, &
             stats, repiter, verbose)
```

custom particle layout 用の adapter 設定は `ohhelp_v2` の handle で行えます。
粒子バッファは `type(c_ptr)` にして `oh2_init_raw()` に渡します。

```fortran
type(oh_context_handle) :: ctx
type(oh_particle_adapter_handle) :: adapter
integer(c_int) :: ierr

ctx = oh_default_context()
call oh_particle_adapter_create_byte(adapter, c_sizeof(sample_particle), ierr)
call oh_particle_adapter_use_int_fields(adapter, region_offset, species_offset)
call oh_context_set_particle_adapter(ctx, adapter)

raw_particles = c_loc(particles(1))
call oh2_init_raw(c_loc(sdid(1)), nspec, maxfrac, &
                  c_loc(nphgram(1,1,1)), c_loc(totalp(1,1)), &
                  raw_particles, c_loc(pbase(1)), maxlocalp, &
                  c_loc(mycomm), c_loc(nbor(1,1,1)), c_loc(pcoord(1)), &
                  stats, repiter, verbose)
```

injection/removal:

```fortran
type(oh_particle) :: part

part%nid = -1
call oh_inject_particle(part)
call oh_remap_injected_particle(pbuf(injected_index))
call oh_remove_injected_particle(pbuf(injected_index))
```

`ohhelp_v2` の context API では `oh_context_inject_particle_get()` が
`type(c_ptr)` を返します。

## Level 3: 場データと空間 mapping も任せる

Level 3 は Fortran PIC での標準選択です。Level 2 の粒子転送に加えて、
subdomain geometry、粒子の隣接領域 mapping、場データの境界交換を使えます。

```fortran
#define OH_LIB_LEVEL 3
#include "ohhelp_f.h"
module sim_l3
  use ohhelp3
  implicit none
end module
```

初期化:

```fortran
call oh_init(sdid, nspec, maxfrac, nphgram, totalp, &
             pbuf, pbase, maxlocalp, mycomm, nbor, pcoord, &
             sdoms, scoord, nbound, bcond, bounds, &
             ftypes, cfields, ctypes, fsizes, &
             stats, repiter, verbose)
```

粒子 mapping:

```fortran
m = oh_map_particle_to_neighbor(pbuf(p)%x, pbuf(p)%y, pbuf(p)%z, ps)
pbuf(p)%nid = m
nphgram(n+1,s) = nphgram(n+1,s) - 1
nphgram(m+1,s) = nphgram(m+1,s) + 1
```

field 同期:

```fortran
if (currmode == OH_MODE_REBALANCE_SECONDARY) then
  call oh_bcast_field(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB)
  currmode = OH_MODE_NORMAL_SECONDARY
end if

if (currmode /= OH_MODE_NORMAL_PRIMARY) then
  call oh_allreduce_field(cd(1,0,0,0,1), cd(1,0,0,0,2), FCD)
end if

call oh_exchange_borders(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB, currmode)
```

custom particle layout 用の Level 3 position fields は adapter に設定できます。
初期化は `oh3_init_raw()` で行います。

```fortran
call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                    y_offset, z_offset)
call oh_context_set_particle_adapter(ctx, adapter)

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

## Level 4p / 4s

Level 4p/4s は v2.x の継続対応対象です。v2.0 では compile coverage と
移行境界を維持しますが、custom particle layout を含む supported API としては
固めません。

## Level 選択の目安

| 状況 | 推奨 level |
| --- | --- |
| 既存の粒子通信を維持し、負荷分散だけ使いたい | Level 1 |
| 粒子通信を OhHelp に任せたい | Level 2 |
| 通常の格子 PIC で、field 境界交換も任せたい | Level 3 |
| 粒子位置・per-grid 粒子管理まで OhHelp に寄せたい | Level 4p / 4s（v2.x 対象） |
