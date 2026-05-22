# PIC Integration Lifecycle (Fortran)

Language: [C mirror](pic-lifecycle.md) | Fortran

この文書では、Fortran の PIC コードに OhHelp を組み込むときの典型的な
呼び出し順を説明します。章立ては C 版と揃えています。

## 1. ビルド時に level と次元を選ぶ

Fortran では、`OH_DIMENSION` と `OH_LIB_LEVEL` を preprocessor で設定してから
`ohhelp_f.h` を include します。通常の v2.0 利用では Level 3 を標準選択にします。

```fortran
#define OH_DIMENSION 3
#define OH_LIB_LEVEL 3
#include "ohhelp_f.h"
module simulator
  use ohhelp3
  implicit none
end module
```

v2 の context/adapter API を使う場合は `ohhelp_v2` も使います。

```fortran
use iso_c_binding
use ohhelp_v2
```

## 2. 初期化前に利用側のデータ構造を決める

default layout では `type(oh_particle)` の配列を使います。

- 粒子種数 `nspec`
- 領域分割数 `pcoord`
- subdomain 境界 `sdoms` と座標 `scoord`
- 粒子数 histogram `nphgram`
- primary / secondary の粒子数 `totalp`
- 粒子バッファ `pbuf`
- primary / secondary の開始位置 `pbase`
- 場データの descriptor `ftypes`, `cfields`, `ctypes`, `fsizes`

custom particle layout を使う場合は、`bind(C)` な粒子型、
`type(oh_particle_adapter_handle)`、`type(c_ptr)` の粒子バッファ pointer を
用意します。初期化は `ohhelp_v2` の `oh2_init_raw()` / `oh3_init_raw()` で行います。

## 3. 初期化時の基本順序

Level 2 以上では、まず最大粒子数を見積もって粒子バッファを確保します。
Level 3 では field descriptor も `oh_init()` に渡します。

```fortran
integer :: currmode
integer :: maxlocalp
integer(kind=8) :: npmax
type(oh_particle), allocatable :: pbuf(:)

call MPI_Init(ierr)

maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin)
allocate(pbuf(maxlocalp))
allocate(nphgram(nregions, nspec, 2))
allocate(totalp(nspec, 2))

call oh_init(sdid, nspec, maxfrac, nphgram, totalp, &
             pbuf, pbase, maxlocalp, mycomm, nbor, pcoord, &
             sdoms, scoord, nbound, bcond, bounds, &
             ftypes, cfields, ctypes, fsizes, &
             stats, repiter, verbose)

allocate(eb(6, fsizes(1,1,FEB):fsizes(2,1,FEB), &
               fsizes(1,2,FEB):fsizes(2,2,FEB), &
               fsizes(1,3,FEB):fsizes(2,3,FEB), 2))
```

## 4. 最初の transbound と場の同期

初期粒子配置後、最初に `oh_transbound()` を呼びます。

```fortran
currmode = oh_transbound(0, stats)

if (currmode < 0) then
  call oh_bcast_field(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB)
  currmode = 1
end if

call oh_exchange_borders(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB, currmode)
```

## 5. timestep 内の基本ループ

粒子 push、粒子移動と負荷分散、電流 scatter、電流同期、場 solve、
場境界交換の順で呼びます。

```fortran
do step = 1, nstep
  call particle_push(pbuf(pbase(1):), nspec, totalp(:,1), &
                     eb(:,:,:,:,1), sdoms(:,:,sdid(1)), sdid(1), 0, &
                     nphgram(:,:,1))

  if (sdid(2) >= 0) then
    call particle_push(pbuf(pbase(2):), nspec, totalp(:,2), &
                       eb(:,:,:,:,2), sdoms(:,:,sdid(2)), sdid(2), 1, &
                       nphgram(:,:,2))
  end if

  currmode = oh_transbound(currmode, stats)

  if (currmode < 0) then
    call oh_bcast_field(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB)
    currmode = 1
  end if

  if (currmode /= 0) then
    call oh_allreduce_field(cd(1,0,0,0,1), cd(1,0,0,0,2), FCD)
  end if

  call oh_exchange_borders(cd(1,0,0,0,1), cd(1,0,0,0,2), FCD, currmode)
  call field_solve_e(eb(:,:,:,:,1), cd(:,:,:,:,1), sdoms(:,:,sdid(1)))
  call oh_exchange_borders(eb(1,0,0,0,1), eb(1,0,0,0,2), FEB, currmode)
end do
```

## 6. 粒子 push 中に必要な更新

Level 3 では、粒子が subdomain 境界を越えたら `oh_map_particle_to_neighbor()`
で移動先 region を決め、histogram と粒子の region id を更新します。

```fortran
if (pbuf(p)%x < xl .or. pbuf(p)%x >= xu .or. &
    pbuf(p)%y < yl .or. pbuf(p)%y >= yu .or. &
    pbuf(p)%z < zl .or. pbuf(p)%z >= zu) then
  m = oh_map_particle_to_neighbor(pbuf(p)%x, pbuf(p)%y, pbuf(p)%z, ps)
  nphgram(n+1,s) = nphgram(n+1,s) - 1
  nphgram(m+1,s) = nphgram(m+1,s) + 1
  pbuf(p)%nid = m
end if
```

custom particle layout 用には `ohhelp_v2` の adapter に region/position field を
設定し、`oh3_init_raw()` で初期化します。粒子 push 中は adapter に登録した
region field と histogram を、default `type(oh_particle)` の `nid` と同じ意味で
更新します。

## 7. 粒子 injection / removal

default `type(oh_particle)` API では、injection 領域にある particle そのものを
remap/remove に渡します。

```fortran
type(oh_particle) :: part

part%nid = -1
call oh_inject_particle(part)

! 後で操作する injected particle は pbuf 側の要素を渡す
call oh_remap_injected_particle(pbuf(injected_index))
call oh_remove_injected_particle(pbuf(injected_index))
```

`ohhelp_v2` の context API を使う場合は、C と同じく
`oh_context_inject_particle_get()` が `type(c_ptr)` を返します。

```fortran
type(c_ptr) :: injected

injected = oh_context_inject_particle_get(ctx, c_loc(part))
call oh_context_remap_injected_particle(ctx, injected)
call oh_context_remove_injected_particle(ctx, injected)
```

## 8. 統計と verbose

統計 API は C と同じ順番で呼びます。

```fortran
call oh_init_stats(key, primary_or_secondary)
call oh_stats_time(key, primary_or_secondary)
call oh_show_stats(step, currmode)
call oh_print_stats(nstep)
```
