# PIC Integration Lifecycle

この文書では、PIC コードに OhHelp を組み込むときの典型的な呼び出し順を
説明します。例は C 風の擬似コードですが、Fortran でも同じ順序です。

## 1. ビルド時に level と次元を選ぶ

`OH_DIMENSION` と `OH_LIB_LEVEL` は、ヘッダを include する前に決めます。
Level 4 を使う場合は `OH_LIB_LEVEL_4P` または `OH_LIB_LEVEL_4S` を使います。

```c
#define OH_DIMENSION 3
#define OH_LIB_LEVEL 3
#include "ohhelp_c.h"
```

Level 4p の例:

```c
#define OH_DIMENSION 3
#define OH_LIB_LEVEL_4P
#include "ohhelp_c.h"
```

## 2. 初期化前に利用側のデータ構造を決める

PIC 側では、少なくとも次の情報を用意します。

- 粒子種数 `nspec`
- 領域分割数 `pcoord`
- subdomain 境界 `sdoms` と座標 `scoord`
- 粒子数 histogram `nphgram`
- primary / secondary の粒子数 `totalp`
- 粒子バッファ `pbuf`
- primary / secondary の開始位置 `pbase`
- 場データの descriptor `ftypes`, `cfields`, `ctypes`, `fsizes`

OhHelp は primary 領域だけでなく、必要に応じて secondary 領域も担当します。
そのため、粒子と場の計算は primary / secondary の 2 系統を扱えるようにします。

## 3. 初期化時の基本順序

Level 2 以上では、まず最大粒子数を見積もって粒子バッファを確保します。
Level 3 以上では、場データの境界交換 descriptor も `oh_init()` に渡します。

```c
int currmode;
int maxlocalp;
void *raw_pbuf;

MPI_Init(&argc, &argv);

maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin);
allocate_particle_buffer(&pbuf, maxlocalp);
raw_pbuf = pbuf;
allocate_histograms(&nphgram, nspec, node_count);
allocate_total_counts(&totalp, nspec);

oh_init(&sdid, nspec, maxfrac,
        nphgram, totalp,
        &raw_pbuf, &pbase, maxlocalp,
        &mycomm, &nbor, pcoord,
        sdoms, scoord,
        nbound, bcond, bounds,
        ftypes, cfields, ctypes, fsizes,
        stats, repiter, verbose);
pbuf = raw_pbuf;

allocate_fields_from_fsizes(fsizes, &field_primary, &field_secondary);
initialize_particles(pbuf, nphgram);
initialize_fields(field_primary);
```

Level 4s は `oh_init()` が `maxlocalp` と communication buffer size を返し、
その後に `oh_particle_buffer()` で粒子バッファを渡す形です。詳細は
[API by OhHelp level](api-by-level.md) を参照してください。

## 4. 最初の transbound と場の同期

初期粒子配置後、最初に `oh_transbound()` を呼びます。戻り値 `currmode` は
現在の計算モードを表します。

```c
currmode = oh_transbound(0, stats);

if (currmode < 0) {
    oh_bcast_field(field_primary, field_secondary, field_type_eb);
    currmode = 1;
}

oh_exchange_borders(field_primary, field_secondary, field_type_eb, currmode);
```

`currmode < 0` は、OhHelp が secondary 側の場を broadcast する必要がある
状態を表します。利用側では broadcast 後に `currmode = 1` として扱います。

## 5. timestep 内の基本ループ

典型的な PIC ループでは、粒子 push、粒子移動と負荷分散、電流 scatter、
電流同期、場 solve、場境界交換の順で呼びます。

```c
for (int step = 0; step < nstep; step++) {
    particle_push(pbuf + pbase[0],
                  totalp[0],
                  field_primary,
                  sdoms[sdid[0]],
                  0,
                  nphgram[0]);

    if (sdid[1] >= 0) {
        particle_push(pbuf + pbase[1],
                      totalp[1],
                      field_secondary,
                      sdoms[sdid[1]],
                      1,
                      nphgram[1]);
    }

    currmode = oh_transbound(currmode, stats);

    if (currmode < 0) {
        oh_bcast_field(field_primary, field_secondary, field_type_eb);
        currmode = 1;
    }

    current_scatter(pbuf + pbase[0],
                    totalp[0],
                    current_primary,
                    sdoms[sdid[0]]);

    if (sdid[1] >= 0) {
        current_scatter(pbuf + pbase[1],
                        totalp[1],
                        current_secondary,
                        sdoms[sdid[1]]);
    }

    if (currmode != 0) {
        oh_allreduce_field(current_primary, current_secondary, field_type_current);
    }

    oh_exchange_borders(current_primary, current_secondary,
                        field_type_current, currmode);

    field_solve(field_primary, current_primary, sdoms[sdid[0]]);
    if (sdid[1] >= 0) {
        field_solve(field_secondary, current_secondary, sdoms[sdid[1]]);
    }

    oh_exchange_borders(field_primary, field_secondary,
                        field_type_eb, currmode);
}
```

## 6. 粒子 push 中に必要な更新

粒子が subdomain 境界を越えた場合、利用側は「どの隣接領域へ移動するか」を
OhHelp に問い合わせ、粒子の region id と histogram を更新します。

Level 3 の例:

```c
if (particle_is_outside_subdomain(&p, sdom)) {
    int old_region = self_region_index;
    int dst = oh_map_particle_to_neighbor(&p.x, &p.y, &p.z, primary_or_secondary);

    nphgram[species][old_region]--;
    nphgram[species][dst]++;
    /* default S_particle の例。custom particle では adapter の region field を更新する。 */
    p.nid = dst;
}
```

Level 4p/4s では粒子構造体そのものを渡します。

```c
if (particle_is_outside_subdomain(&p, sdom)) {
    int dst = oh_map_particle_to_neighbor(&p, primary_or_secondary, species);
    /* Level 4 mapping updates the packed particle id through OhHelp's rules. */
}
```

## 7. 粒子 injection / removal

外部境界、衝突モデル、粒子生成モデルなどで粒子数が変わる場合は、
OhHelp の injection/removal API を使います。

Level 2/3:

```c
struct S_particle p = make_new_particle();
oh_inject_particle(&p);

/* 既に injection 済みの粒子を移動した場合 */
oh_remap_injected_particle(&p);

/* 削除する場合 */
oh_remove_injected_particle(&p);
```

Level 4p/4s:

```c
struct S_particle p = make_new_particle();
int ok = oh_inject_particle(&p, primary_or_secondary);

if (!ok) {
    handle_injection_failure();
}
```

## 8. 統計と verbose

`stats` を有効にして `oh_init()` / `oh_transbound()` に渡すと、OhHelp 側の
負荷分散・通信の統計を取りやすくなります。

```c
oh_init_stats(key, primary_or_secondary);
oh_stats_time(key, primary_or_secondary);
oh_show_stats(step, currmode);
oh_print_stats(nstep);
```

開発中は `verbose` を有効にして、分割・通信の変化を確認してください。
