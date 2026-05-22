# API by OhHelp Level

OhHelp の level は、利用側がどこまでを自前で行い、どこからを OhHelp に
任せるかを決める境界です。利用側ドキュメントでは level ごとに章を分ける
方が適切です。

v2.0 では Level 1-3 を supported scope とします。Level 4p/4s は v2.x で
対応を進める拡張であり、v2.0 の完了条件には含めません。

## 共通 API

全 level で使える代表的な API です。`ohhelp_c.h` / `ohhelp_f.h` を使うと
`oh_*` alias で呼べます。

| API | 呼ぶ段階 | 目的 |
| --- | --- | --- |
| `oh_init()` | 初期化 | OhHelp の領域分割、通信、統計、内部配列を初期化する。 |
| `oh_transbound(currmode, stats)` | timestep 中の粒子 push 後 | 粒子移動、負荷分散、primary/secondary 遷移を実行する。 |
| `oh_neighbors()` | 初期化後 | 隣接領域情報を取得する。 |
| `oh_families()` | 初期化後 | helpand/helper family 情報を取得する。 |
| `oh_accom_mode()` | 必要時 | 現在の accommodation mode を取得する。 |
| `oh_broadcast()` | primary/secondary 同期 | family 内で primary 側データを secondary 側へ配る。 |
| `oh_all_reduce()` / `oh_reduce()` | primary/secondary 同期 | secondary 側の寄与を集約する。 |
| `oh_set_region_weights()` | `oh_init()` 後、通常は最初の `oh_transbound()` 前 | region ごとの計算コスト重みを設定する。 |

## Context API

v2.0 では C の `oh_default_context()` で現在の default OhHelp instance を
`oh_context *` として取得できます。Fortran では `ohhelp_v2` module の
`oh_default_context()` が `type(oh_context_handle)` を返します。複数 context の
独立運用は v2.x 以降の対象ですが、Level 1-3 の主要操作には
context-facing wrapper を用意しています。

代表例:

```c
oh_context *ctx = oh_default_context();

oh_context_set_region_weights(ctx, weights);
oh_context_transbound3(ctx, currmode, stats);
oh_context_map_particle_to_neighbor(ctx, &p.x, &p.y, &p.z, ps);
oh_context_bcast_field(ctx, eb_primary, eb_secondary, field_type_eb);
oh_context_exchange_borders(ctx, eb_primary, eb_secondary, field_type_eb,
                            currmode);
```

Level 1 の collective は `oh_context_broadcast()` /
`oh_context_all_reduce()` / `oh_context_reduce()`、Level 2 の注入粒子操作は
`oh_context_inject_particle_get()` /
`oh_context_remap_injected_particle()` /
`oh_context_remove_injected_particle()` から同じ default context state を使えます。
Level 3 の座標 mapping は `oh_context_map_particle_to_neighbor()` と
`oh_context_map_particle_to_subdomain()` から使えます。

Fortran 例:

```fortran
use iso_c_binding
use ohhelp_v2

type(oh_context_handle) :: ctx
real(c_double), target :: weights(nregions)
integer(c_int) :: currmode

ctx = oh_default_context()
call oh_context_set_region_weights(ctx, weights)
currmode = oh_context_transbound3(ctx, currmode, 0_c_int)
```

## Level 1: スケジュールだけを使う

Level 1 は、粒子データを OhHelp に渡しません。利用側が粒子転送を実装し、
OhHelp から負荷分散に必要な通信・領域情報を得ます。

向いているケース:

- 既存コードに独自の粒子転送がすでにある。
- 粒子が構造体配列ではなく、複数配列や特殊なデータ構造で管理されている。
- まずは負荷分散ロジックだけを導入したい。

初期化の形:

```c
#define OH_LIB_LEVEL 1
#include "ohhelp_c.h"

oh_init(&sdid, nspec, maxfrac,
        nphgram, totalp,
        rcounts, scounts,
        &mycomm, &nbor, pcoord,
        stats, repiter, verbose);
```

timestep 中:

```c
update_particle_histogram_by_user_code(nphgram);
currmode = oh_transbound(currmode, stats);
exchange_particles_by_user_code(rcounts, scounts, mycomm);
```

Level 1 では `oh_transbound()` の結果をもとに、実際の粒子 pack/send/recv/unpack
は利用側が行います。

## Level 2: 粒子バッファ転送まで任せる

Level 2 は、OhHelp が粒子バッファを移動します。利用側は粒子の region id と
species 情報が OhHelp から読めるようにします。

C API で粒子バッファを OhHelp に渡すときは、`void *raw_pbuf` で受け渡しし、
初期化後に利用側の粒子型へ戻します。これは custom particle layout でも
`struct S_particle` でも同じです。
Fortran の従来 API では `type(oh_particle)` の配列を渡します。custom particle
layout を使う場合は `ohhelp_v2` の opaque adapter handle を設定します。

向いているケース:

- 粒子が 1 本の配列で管理されている。
- 境界を越えた粒子を OhHelp に移動してほしい。
- 場データの境界交換は既存コード側で維持したい。

初期化前:

```c
maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin);
allocate_particle_buffer(&pbuf, maxlocalp);
raw_pbuf = pbuf;
```

v2 で独自粒子レイアウトを使う場合は、`oh_init()` より前に adapter を設定します。

```c
oh_particle_adapter adapter = make_my_particle_adapter();
oh_set_particle_adapter(&adapter);
```

region/species が整数 field で表現されている場合は、callback を手書きせずに
offset helper を使えます。`int` field なら `oh_particle_adapter_use_int_fields()`、
`long` / `long long` field や `OH_BIG_SPACE` を想定する region field なら
`oh_particle_adapter_use_integer_fields()` を使います。

```c
MPI_Datatype my_particle_mpi_type;

oh_particle_adapter_make_byte_type(sizeof(struct my_particle),
                                   &my_particle_mpi_type);
adapter = oh_default_particle_adapter(my_particle_mpi_type);
adapter.stride = sizeof(struct my_particle);
oh_particle_adapter_use_integer_fields(
    &adapter,
    offsetof(struct my_particle, region), sizeof(((struct my_particle*)0)->region),
    offsetof(struct my_particle, species), sizeof(((struct my_particle*)0)->species));
```

custom adapter を使わない状態に戻す場合は、次の `oh_init()` より前に
`oh_set_particle_adapter(NULL)` を呼びます。これは default `S_particle` adapter と
default byte MPI datatype へ戻す操作です。
MPI datatype 設定だけを default byte datatype に戻す場合は
`oh_set_particle_mpi_type(MPI_DATATYPE_NULL)` を使います。

初期化:

```c
#define OH_LIB_LEVEL 2
#include "ohhelp_c.h"

oh_init(&sdid, nspec, maxfrac,
        nphgram, totalp,
        &raw_pbuf, &pbase, maxlocalp,
        &mycomm, &nbor, pcoord,
        stats, repiter, verbose);
pbuf = raw_pbuf;
```

timestep 中:

```c
push_primary_and_secondary_particles(pbuf, pbase, totalp);
mark_moved_particles_with_destination_region(pbuf, nphgram);
currmode = oh_transbound(currmode, stats);
```

粒子数が変化する場合:

```c
set_destination_region(&new_particle, -1);
struct my_particle *pinj = oh_inject_particle_get(&new_particle);

set_destination_region(pinj, new_region);
oh_remap_injected_particle(pinj);
oh_remove_injected_particle(pinj);
```

`oh_remap_injected_particle()` / `oh_remove_injected_particle()` には、
`oh_inject_particle_get()` が返す particle buffer 内ポインタを渡します。
後続操作が不要なら `oh_inject_particle(&new_particle)` だけで注入できます。
`oh_remap_injected_particle()` は、負の region で注入した粒子、または
`oh_remove_injected_particle()` で一度 count を取り消した injected particle を、
現在の region/species で再計上する API です。

Fortran には C の `oh_inject_particle_get()` 相当の pointer-return helper は
ありません。`pbuf` の injection 領域にコピーされた `type(oh_particle)` 要素を
後続の `oh_remap_injected_particle()` /
`oh_remove_injected_particle()` に渡します。

`ohhelp_v2` の context API を使う場合は、C と同じく
`oh_context_inject_particle_get()` が `type(c_ptr)` を返します。custom particle
layout の injected copy を後で remap/remove する場合は、この pointer を保持します。

## Level 3: 場データと空間 mapping も任せる

Level 3 は、PIC コードで最も標準的な選択です。Level 2 の粒子転送に加えて、
subdomain geometry、粒子の隣接領域 mapping、場データの境界交換を使えます。

向いているケース:

- 格子 field を持つ通常の PIC。
- 粒子位置から隣接 subdomain を判断したい。
- 電磁場や電流密度の ghost/border exchange を OhHelp に任せたい。

初期化:

```c
#define OH_LIB_LEVEL 3
#include "ohhelp_c.h"

oh_set_particle_position_fields(&adapter,
                                offsetof(struct my_particle, x),
                                offsetof(struct my_particle, y),
                                offsetof(struct my_particle, z));
oh_set_particle_adapter(&adapter);

oh_init(&sdid, nspec, maxfrac,
        nphgram, totalp,
        &raw_pbuf, &pbase, maxlocalp,
        &mycomm, &nbor, pcoord,
        sdoms, scoord,
        nbound, bcond, bounds,
        ftypes, cfields, ctypes, fsizes,
        stats, repiter, verbose);
pbuf = raw_pbuf;
```

粒子 push 中:

```c
if (particle_left_subdomain(&p, sdoms[sdid[ps]])) {
    int dst = (int)adapter.map_to_neighbor(&adapter, &p, ps);
    adapter.set_region(&adapter, &p, dst, ps);
    nphgram[species][self]--;
    nphgram[species][dst]++;
}
```

`oh_set_particle_position_fields()` を使った adapter では、`map_to_neighbor` と
`map_to_subdomain` は OhHelp が持つ Level 3 の subdomain geometry を使う
標準実装になります。既存 `S_particle` を使う場合は `oh_init()` 内で同等の
default mapping が自動設定されます。

場データ同期:

```c
if (currmode < 0) {
    oh_bcast_field(eb_primary, eb_secondary, field_type_eb);
    currmode = 1;
}

if (currmode != 0) {
    oh_allreduce_field(j_primary, j_secondary, field_type_current);
}

oh_exchange_borders(eb_primary, eb_secondary, field_type_eb, currmode);
oh_exchange_borders(j_primary, j_secondary, field_type_current, currmode);
```

`ftypes`, `cfields`, `ctypes`, `fsizes` は、各 field の要素数、ghost 幅、
通信対象範囲を OhHelp に伝える descriptor です。

custom particle layout で Level 3 を使う最小の C 側セットアップ例は
`sample/level3_custom_particle.c` にあります。このサンプルは Docker 検証で
compile-check されます。

Fortran の Level 3 利用例は `sample/sample.F90` にあり、default
`type(oh_particle)` layout、`oh_map_particle_to_neighbor()`、field collective、
border exchange の compile coverage に含めています。

Fortran custom particle layout の最小セットアップは次の形です。

```fortran
use iso_c_binding
use ohhelp_v2

type, bind(C) :: pic_particle
  real(c_double) :: x, y, z
  real(c_double) :: vx, vy, vz
  integer(c_int) :: region
  integer(c_int) :: species
end type

type(pic_particle), target :: sample
type(oh_context_handle) :: ctx
type(oh_particle_adapter_handle) :: adapter
integer(c_size_t) :: region_offset, species_offset
integer(c_size_t) :: x_offset, y_offset, z_offset
integer(c_int) :: ierr

ctx = oh_default_context()
call oh_particle_adapter_create_byte(adapter, c_sizeof(sample), ierr)

region_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%region))
species_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%species))
x_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%x))
y_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%y))
z_offset = oh_particle_field_offset(c_loc(sample), c_loc(sample%z))

call oh_particle_adapter_use_int_fields(adapter, region_offset, species_offset)
call oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                    y_offset, z_offset)
call oh_context_set_particle_adapter(ctx, adapter)
```

## Level 4p: position-aware particle management

Level 4p は、粒子位置を意識した per-grid 粒子管理を使う拡張です。
通常の Level 3 mapping より粒子側の情報を OhHelp に渡す割合が増えます。
v2.0 では移行中の拡張扱いです。compile coverage は維持しますが、
custom particle layout を含む supported API としては v2.x で固めます。

有効化:

```c
#define OH_LIB_LEVEL_4P
#include "ohhelp_c.h"
```

初期化前:

```c
oh_set_particle_adapter(&adapter);  /* custom layout を使う場合 */
maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin, hsthresh);
allocate_particle_buffer(&pbuf, maxlocalp);
raw_pbuf = pbuf;
```

初期化:

```c
oh_init(&sdid, nspec, maxfrac,
        totalp, &raw_pbuf, &pbase, maxlocalp,
        &mycomm, &nbor, pcoord,
        sdoms, scoord,
        nbound, bcond, bounds,
        ftypes, cfields, ctypes, fsizes,
        stats, repiter, verbose);
pbuf = raw_pbuf;

oh_per_grid_histogram(&pghgram);
```

粒子 mapping:

```c
int dst = oh_map_particle_to_neighbor(&particle, primary_or_secondary, species);
```

injection / removal:

```c
if (!oh_inject_particle(&particle, primary_or_secondary)) {
    handle_failed_injection();
}

oh_remove_mapped_particle(&particle, primary_or_secondary, species);
```

## Level 4s: structured per-grid particle management

Level 4s は、Level 4p と同じく position-aware ですが、per-grid index をより
明示的に扱う拡張です。Level 4p と Level 4s は同時には使いません。
v2.0 では移行中の拡張扱いです。per-grid index と packed-id semantics の
整理は v2.x の対象です。

有効化:

```c
#define OH_LIB_LEVEL_4S
#include "ohhelp_c.h"
```

初期化:

```c
int maxlocalp;
int cbufsize;

oh_set_particle_adapter(&adapter);  /* custom layout を使う場合 */
oh_init(&sdid, nspec, maxfrac,
        npmax, minmargin, maxdensity,
        totalp, pbase,
        &maxlocalp, &cbufsize,
        &mycomm, &nbor, pcoord,
        sdoms, scoord,
        nbound, bcond, bounds,
        ftypes, cfields, ctypes, fsizes,
        zbound,
        stats, repiter, verbose);

allocate_particle_buffer(&pbuf, maxlocalp);
raw_pbuf = pbuf;
oh_particle_buffer(maxlocalp, &raw_pbuf);
pbuf = raw_pbuf;

oh_per_grid_histogram(&pghgram, &pgindex);
```

Level 4s では境界粒子や per-grid index を使った処理が多いため、粒子配列と
histogram/index の整合性を保つことが重要です。

追加の境界データ交換:

```c
oh_exchange_border_data(buf, sbuf, rbuf, mpi_type);
```

## Level 選択の目安

| 状況 | 推奨 level |
| --- | --- |
| 既存の粒子通信を維持し、負荷分散だけ使いたい | Level 1 |
| 粒子通信を OhHelp に任せたい | Level 2 |
| 通常の格子 PIC で、field 境界交換も任せたい | Level 3 |
| 粒子位置・per-grid 粒子管理まで OhHelp に寄せたい | Level 4p / 4s（v2.x 対象） |

まず Level 3 を標準選択とし、既存コードとの統合コストが大きい場合は Level 1/2
を選ぶのが v2.0 の現実的な使い方です。per-grid 粒子管理が必要な場合は
Level 4p/4s の v2.x 対応を待つか、現行の移行中 API として使う前提で検証してください。
