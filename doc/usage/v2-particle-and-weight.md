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
    int  (*get_region)(const void *particle, int primary_or_secondary);
    void (*set_region)(void *particle, int region, int primary_or_secondary);
    int  (*get_species)(const void *particle);
    int  (*map_to_neighbor)(void *particle, int primary_or_secondary);
    int  (*map_to_subdomain)(void *particle, int primary_or_secondary);
} oh_particle_adapter;
```

呼び出し順:

```c
oh_particle_adapter adapter;

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

注意点:

- `stride` は粒子 1 要素の実メモリ間隔です。
- `mpi_type` の extent は `stride` と一致している必要があります。
- Level 2 では region/species と byte movement が主に必要です。
- Level 3/4 で独自粒子レイアウトを使う場合は、位置から subdomain を決める
  callback も用意してください。
- 現在の implementation は default context への adapter 設定です。
  完全な複数 context 対応は v2 の継続作業です。

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
