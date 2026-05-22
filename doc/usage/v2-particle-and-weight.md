# v2 Particle Layout and Weighted Load

v2 では、従来の固定 `S_particle` / `oh_particle` 前提から離れ、利用側が
粒子レイアウトと region ごとの計算コストを明示できる設計へ移行しています。

## Particle adapter

C API では `oh_particle_adapter` を使って、OhHelp が粒子をどう扱うかを
指定できます。

```c
typedef struct oh_particle_adapter {
    size_t stride;
    MPI_Datatype mpi_type;
    void *user_data;
    oh_particle_region_t (*get_region)(const oh_particle_adapter *adapter,
                                       const void *particle,
                                       int primary_or_secondary);
    void (*set_region)(const oh_particle_adapter *adapter,
                       void *particle, oh_particle_region_t region,
                       int primary_or_secondary);
    int  (*get_species)(const oh_particle_adapter *adapter,
                        const void *particle);
    oh_particle_region_t (*map_to_neighbor)(const oh_particle_adapter *adapter,
                                            void *particle,
                                            int primary_or_secondary);
    oh_particle_region_t (*map_to_subdomain)(const oh_particle_adapter *adapter,
                                             void *particle,
                                             int primary_or_secondary);
} oh_particle_adapter;
```

`oh_particle_region_t` は packed Level-4 id も保持できる wide integer 型です。
region field が `int` の通常用途でもそのまま使えます。

呼び出し順:

```c
oh_particle_adapter adapter;
MPI_Datatype my_particle_mpi_type;

oh_particle_adapter_make_byte_type(sizeof(struct my_particle),
                                   &my_particle_mpi_type);

adapter.stride = sizeof(struct my_particle);
adapter.mpi_type = my_particle_mpi_type;
adapter.get_region = my_get_region;
adapter.set_region = my_set_region;
adapter.get_species = my_get_species;
adapter.map_to_neighbor = my_map_to_neighbor;
adapter.map_to_subdomain = my_map_to_subdomain;

oh_set_particle_adapter(&adapter);
oh_init(...);
```

## `mpi_type` の標準的な作り方

`adapter.mpi_type` は、粒子 1 要素を MPI で送受信するための datatype です。
OhHelp は MPI count/displacement をこの datatype の単位で扱うため、datatype の
extent は必ず `adapter.stride` と一致させます。

粒子が pointer を含まない plain data で、padding ごと送ってよい場合は、
`oh_particle_adapter_make_byte_type()` を使うのが標準です。この helper は
`MPI_BYTE` の contiguous type を作り、`MPI_Type_create_resized()` で extent を
指定した stride に固定し、commit 済みの datatype を返します。

```c
MPI_Datatype my_particle_mpi_type;

if (oh_particle_adapter_make_byte_type(sizeof(struct my_particle),
                                       &my_particle_mpi_type) != MPI_SUCCESS) {
    abort();
}
```

送る field を明示したい場合は、利用側で `MPI_Get_address()` と
`MPI_Type_create_struct()` を使います。この場合も最後に resized type を作り、
extent を `sizeof(struct my_particle)` に合わせます。pointer や process-local
handle を含む粒子では、こちらの方式で通信対象 field を明示してください。

```c
static MPI_Datatype
make_my_particle_mpi_type(void) {
    struct my_particle sample;
    MPI_Aint base;
    MPI_Aint disp[5];
    int blocklen[5] = { 1, 1, 1, 1, 1 };
    MPI_Datatype types[5] = {
        MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_INT
    };
    MPI_Datatype raw_type;
    MPI_Datatype particle_type;

    MPI_Get_address(&sample, &base);
    MPI_Get_address(&sample.x, &disp[0]);
    MPI_Get_address(&sample.y, &disp[1]);
    MPI_Get_address(&sample.z, &disp[2]);
    MPI_Get_address(&sample.region, &disp[3]);
    MPI_Get_address(&sample.species, &disp[4]);
    for (int i = 0; i < 5; i++) {
        disp[i] -= base;
    }

    MPI_Type_create_struct(5, blocklen, disp, types, &raw_type);
    MPI_Type_create_resized(raw_type, 0,
                            (MPI_Aint)sizeof(struct my_particle),
                            &particle_type);
    MPI_Type_commit(&particle_type);
    MPI_Type_free(&raw_type);
    return particle_type;
}
```

作った datatype は、OhHelp の利用が終わった後に利用側で
`MPI_Type_free()` してください。

## region/species callback の標準形

粒子構造体に `int` の region と species field があるだけなら、callback を
手書きせずに offset 指定の標準 accessor を使えます。

```c
#include <stddef.h>

struct my_particle {
    double x, y, z;
    double vx, vy, vz;
    int region;
    int species;
};

oh_particle_adapter adapter;
MPI_Datatype my_particle_mpi_type;

oh_particle_adapter_make_byte_type(sizeof(struct my_particle),
                                   &my_particle_mpi_type);

adapter = oh_default_particle_adapter(my_particle_mpi_type);
adapter.stride = sizeof(struct my_particle);
oh_particle_adapter_use_int_fields(&adapter,
                                   offsetof(struct my_particle, region),
                                   offsetof(struct my_particle, species));
oh_set_particle_position_fields(&adapter,
                                offsetof(struct my_particle, x),
                                offsetof(struct my_particle, y),
                                offsetof(struct my_particle, z));

oh_set_particle_adapter(&adapter);
oh_init(...);
```

single-species の粒子配列なら、species field なしの helper を使えます。

```c
oh_particle_adapter_use_single_species_int_region(
    &adapter, offsetof(struct my_particle, region));
```

region/species field が `int` ではない、あるいは座標や補助データを使って mapping
したい場合は、`oh_particle_adapter.h` のマクロで型付き callback を生成できます。

```c
struct my_particle {
    double x, y, z;
    double vx, vy, vz;
    int region;
    int species;
};

OH_DEFINE_PARTICLE_ADAPTER_ACCESSORS(my_particle, struct my_particle,
                                     region, species)
OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING(my_particle, struct my_particle,
                                          region)

oh_particle_adapter adapter;
MPI_Datatype my_particle_mpi_type;

oh_particle_adapter_make_byte_type(sizeof(struct my_particle),
                                   &my_particle_mpi_type);

adapter = oh_default_particle_adapter(my_particle_mpi_type);
adapter.stride = sizeof(struct my_particle);
adapter.get_region = my_particle_get_region;
adapter.set_region = my_particle_set_region;
adapter.get_species = my_particle_get_species;
adapter.map_to_neighbor = my_particle_map_to_neighbor;
adapter.map_to_subdomain = my_particle_map_to_subdomain;

oh_set_particle_adapter(&adapter);
oh_init(...);
```

single-species の型付き callback も macro で生成できます。

```c
OH_DEFINE_PARTICLE_ADAPTER_SINGLE_SPECIES_ACCESSORS(my_particle,
                                                    struct my_particle,
                                                    region)
```

`OH_DEFINE_PARTICLE_ADAPTER_REGION_MAPPING()` は、利用側が particle push 中に
`region` field を destination region へ更新する設計向けの最小実装です。
`oh_particle_adapter_use_position_fields()` は position offset だけを設定します。
Level 3 では `oh_set_particle_position_fields()` を使うと、その offset 設定に加えて
OhHelp が持つ subdomain geometry から標準の `map_to_neighbor` /
`map_to_subdomain` を設定できます。この mapping は既存 `S_particle` の Level 3
mapping と同じく、周期境界を跨ぐ場合に粒子座標 field を wrap します。

```c
oh_set_particle_position_fields(&adapter,
                                offsetof(struct my_particle, x),
                                offsetof(struct my_particle, y),
                                offsetof(struct my_particle, z));
```

adapter に position offset を設定すると、OhHelp 内部では
`oh_particle_adapter_position()` / `oh_particle_adapter_const_position()` で
座標 field を取得します。Level 4p/4s も mapping 時の座標アクセスはこの
offset helper 経由になっています。ただし Level 4 専用の packed particle id と
per-grid 管理はまだ残っているため、custom particle layout での完全対応は
引き続き移行中です。

## `nid` と remove の扱い

既存の `S_particle` では `nid` が region id と remove marker を兼ねます。
default adapter は `nid` を region field として読み書きします。

custom particle layout では、field 名は `nid` である必要はありません。
`get_region` / `set_region`、または `oh_particle_adapter_use_int_fields()` で指定した
region field が同じ役割を持ちます。
公開ヘッダ内の packed-id helper も v2 では active adapter 経由で region を
読み書きするため、`S_particle.nid` という field 名は必須条件ではありません。

- 通常粒子: region が負値、または Level 3/4 mapping が `-1` を返す粒子は、
  次の transfer で送受信・保持対象から外れます。利用側は histogram/count も
  同じ意味に合わせて更新してください。
- injected particle: `oh_inject_particle()` 後に既に OhHelp の injection count に
  入った粒子を消す場合は、`oh_remove_injected_particle()` を明示的に呼んでください。
  region を `-1` にするだけでは、injection count の減算が行われません。
- `oh_remove_injected_particle()` は対象が injection buffer 内の粒子であることを
  検証し、count を減らした上で region を `-1` にします。

注意点:

- `stride` は粒子 1 要素の実メモリ間隔です。
- `mpi_type` の extent は `stride` と一致している必要があります。
- Level 2 では region/species と byte movement が主に必要です。
- Level 3/4 で独自粒子レイアウトを使う場合は、位置から subdomain を決める
  callback も用意してください。
- 現在の implementation は default context への adapter 設定です。
  完全な複数 context 対応は v2 の継続作業です。
- `oh_set_particle_adapter(NULL)` または
  `oh_context_set_particle_adapter(ctx, NULL)` を呼ぶと custom adapter 設定は解除され、
  次の `oh_init()` では default `S_particle` adapter と default byte MPI datatype が
  使われます。adapter に渡した MPI datatype の所有権は利用側に残るため、解除時に
  OhHelp はその datatype を free しません。

## 既存 `S_particle` を使う場合

既存の `struct S_particle` / Fortran `type(oh_particle)` をそのまま使う場合、
明示的な adapter 設定は不要です。OhHelp は default adapter を使います。

```c
struct S_particle *pbuf;
maxlocalp = oh_max_local_particles(npmax, maxfrac, minmargin);
allocate_particle_buffer(&pbuf, maxlocalp);
oh_init(..., &pbuf, &pbase, maxlocalp, ...);
```

## Region weight

粒子数だけで負荷分散すると、region ごとに粒子 1 個あたりの計算コストが違う
PIC では不均衡が残ります。v2 では region ごとに `double` / `real(8)` の
重みを設定できます。

重み付き負荷は次の意味です。

```text
region_load = local_particle_count_in_region * region_weight
target_load = total_weighted_load / number_of_nodes
```

設定例:

```c
double weights[number_of_regions];

for (int r = 0; r < number_of_regions; r++) {
    weights[r] = estimate_particle_cost_for_region(r);
}

oh_set_region_weights(weights);
```

`NULL` を渡すと、全 region の重みが `1.0` に戻り、粒子数ベースの負荷分散へ
戻ります。

```c
oh_set_region_weights(NULL);
```

Fortran では `real*8` 配列を渡します。

```fortran
real*8 :: weights(nregions)

weights(:) = 1.0d0
weights(hot_region) = 2.5d0
call oh_set_region_weights(weights)
```

## いつ重みを設定するか

基本は `oh_init()` の後、最初の `oh_transbound()` の前に設定します。

```c
oh_init(...);
oh_set_region_weights(weights);

currmode = oh_transbound(0, stats);
```

重みが時間発展で変わる場合は、次の `oh_transbound()` の前に更新します。

```c
for (int step = 0; step < nstep; step++) {
    update_region_cost_model(weights, step);
    oh_set_region_weights(weights);

    push_particles();
    currmode = oh_transbound(currmode, stats);
}
```

ただし、重みを毎 step 大きく変えると分割が過敏に変化し、通信量や secondary
再構築コストが増える可能性があります。実測時間を使う場合でも、移動平均などで
平滑化した値を使う方が安定します。

## 重みの決め方

単純な開始点:

```text
weight(region) = average_particle_push_time_in_region
               / global_average_particle_push_time
```

より実用的な例:

```text
weight(region) =
    1.0
  + alpha * field_complexity(region)
  + beta  * collision_model_cost(region)
  + gamma * boundary_condition_cost(region)
```

重要なのは絶対値ではなく比率です。全 region に同じ係数を掛けても分割結果は
基本的に変わりません。

## PIC loop との組み合わせ

```c
oh_init(...);
oh_set_region_weights(initial_weights);

currmode = oh_transbound(0, stats);

for (int step = 0; step < nstep; step++) {
    measure_or_predict_region_cost(weights);
    oh_set_region_weights(weights);

    push_particles_and_update_histograms();
    currmode = oh_transbound(currmode, stats);

    scatter_current();
    exchange_current_and_fields(currmode);
    solve_fields();
}
```

## 現在の実装上の制約

- region weight の中核 helper と Level 1 の rebalance 経路は実装済みです。
- Level 2/3/4 は `oh_state` 経由でこの情報を参照する移行中です。
- public API はまだ default context 中心です。
- Level 4 の particle adapter stride 対応は進行中で、全経路が完全に
  opaque particle layout になったわけではありません。
