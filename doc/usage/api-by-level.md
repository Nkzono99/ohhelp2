# API by OhHelp Level

OhHelp の level は、利用側がどこまでを自前で行い、どこからを OhHelp に
任せるかを決める境界です。利用側ドキュメントでは level ごとに章を分ける
方が適切です。

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

向いているケース:

- 粒子が 1 本の配列で管理されている。
- 境界を越えた粒子を OhHelp に移動してほしい。
- 場データの境界交換は既存コード側で維持したい。

初期化前:

```c
maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin);
allocate_particle_buffer(&pbuf, maxlocalp);
```

v2 で独自粒子レイアウトを使う場合は、`oh_init()` より前に adapter を設定します。

```c
oh_particle_adapter adapter = make_my_particle_adapter();
oh_set_particle_adapter(&adapter);
```

初期化:

```c
#define OH_LIB_LEVEL 2
#include "ohhelp_c.h"

oh_init(&sdid, nspec, maxfrac,
        nphgram, totalp,
        &pbuf, &pbase, maxlocalp,
        &mycomm, &nbor, pcoord,
        stats, repiter, verbose);
```

timestep 中:

```c
push_primary_and_secondary_particles(pbuf, pbase, totalp);
mark_moved_particles_with_destination_region(pbuf, nphgram);
currmode = oh_transbound(currmode, stats);
```

粒子数が変化する場合:

```c
oh_inject_particle(&new_particle);
oh_remap_injected_particle(&existing_injected_particle);
oh_remove_injected_particle(&particle_to_remove);
```

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

oh_init(&sdid, nspec, maxfrac,
        nphgram, totalp,
        &pbuf, &pbase, maxlocalp,
        &mycomm, &nbor, pcoord,
        sdoms, scoord,
        nbound, bcond, bounds,
        ftypes, cfields, ctypes, fsizes,
        stats, repiter, verbose);
```

粒子 push 中:

```c
if (particle_left_subdomain(&p, sdoms[sdid[ps]])) {
    int dst = oh_map_particle_to_neighbor(&p.x, &p.y, &p.z, ps);
    p.nid = dst;
    nphgram[species][self]--;
    nphgram[species][dst]++;
}
```

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

## Level 4p: position-aware particle management

Level 4p は、粒子位置を意識した per-grid 粒子管理を使う拡張です。
通常の Level 3 mapping より粒子側の情報を OhHelp に渡す割合が増えます。

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
```

初期化:

```c
oh_init(&sdid, nspec, maxfrac,
        totalp, &pbuf, &pbase, maxlocalp,
        &mycomm, &nbor, pcoord,
        sdoms, scoord,
        nbound, bcond, bounds,
        ftypes, cfields, ctypes, fsizes,
        stats, repiter, verbose);

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
oh_particle_buffer(maxlocalp, &pbuf);

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
| 粒子位置・per-grid 粒子管理まで OhHelp に寄せたい | Level 4p / 4s |

まず Level 3 を標準選択とし、既存コードとの統合コストが大きい場合は Level 1/2、
per-grid 粒子管理が必要な場合は Level 4p/4s を選ぶのが現実的です。
