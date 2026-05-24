# OhHelp v2 Usage Guide

このディレクトリは、OhHelp を PIC コードへ組み込む利用者向けの
実装ガイドです。PDF 由来の `doc/v1/markdown/` は v1 系の詳細な
リファレンスとして残し、この usage guide では v2 で推奨する呼び出し順と
設計上の考え方をまとめます。

## 読む順番

1. PIC integration lifecycle
   - [C](pic-lifecycle.md)
   - [Fortran](pic-lifecycle-fortran.md)
   - PIC の初期化、粒子 push、電流 scatter、場の solve、負荷分散を
     どの順番で呼ぶかを説明します。
2. API by OhHelp level
   - [C](api-by-level.md)
   - [Fortran](api-by-level-fortran.md)
   - `OH_LIB_LEVEL` ごとの API と、どの level を選ぶべきかを説明します。
3. v2 particle layout and weighted load
   - [C](v2-particle-and-weight.md)
   - [Fortran](v2-particle-and-weight-fortran.md)
   - v2 で追加・整備中の particle adapter と region weight の使い方を
     説明します。

## C / Fortran mirror 構成

usage guide は C 版と Fortran 版を別ページとして mirror します。各ページの
冒頭に相互リンクを置き、章立てと概念の順番はできるだけ揃えます。C 版は
`ohhelp_c.h`、`oh_context *`、`oh_particle_adapter` を直接使う例を載せます。
Fortran 版は、従来 API 用の `ohhelp_f.h` / `ohhelp1` / `ohhelp2` /
`ohhelp3` と、v2 context/adapter/raw-init 用の `ohhelp_v2` opaque handle
API を分けて説明します。`ohhelp_v2` だけで v2 facade は使えますが、
従来の `oh_init()` / `oh_transbound()` alias や `type(oh_particle)` 経路を
同じコードで使う場合は対象 level の `ohhelp*` module も併用します。

## 現在の実装ステータス

| 項目 | C | Fortran |
| --- | --- | --- |
| Level 1-3 default API | supported | supported |
| Level 1-3 context transbound | default / non-default context | `ohhelp_v2` から default / non-default context |
| Level 3 context geometry / field facade | non-default context | `ohhelp_v2` から non-default context |
| region weight | supported | supported |
| custom particle adapter | supported | `ohhelp_v2` で設定 API は利用可能 |
| 任意 layout の粒子配列を init に渡す経路 | supported | `oh2_init_raw()` / `oh3_init_raw()` |
| Level 4p/4s | v2.x 対象 | v2.x 対象 |

Fortran の v2.0 実用経路は、Level 1-3 を既存の `type(oh_particle)` 配列で使い、
必要に応じて `oh_set_region_weights()` と `ohhelp_v2` の default-context facade を
併用する形です。`ohhelp_v2` には adapter handle と callback/offset 設定 API も
あります。任意の Fortran 粒子 layout では、adapter を登録した上で
`oh2_init_raw()` / `oh3_init_raw()` に `c_loc()` で配列と粒子バッファを渡します。

## v2.0 の対象範囲

v2.0 では Level 1-3 を利用可能な範囲として固めます。通常の PIC 利用では
Level 3 を標準選択とし、粒子転送だけを任せたい場合は Level 2、既存の
粒子通信を維持したい場合は Level 1 を選びます。

Fortran からも Level 1-3 は利用対象です。`ohhelp_f.h` の alias と
`src/fortran/oh_mod1.F90` / `oh_mod2.F90` / `oh_mod3.F90` の module は
Docker 検証で compile-check され、`sample/sample.F90` も Level 3 の
Fortran 利用例として compile-check されます。

v2 の context facade と particle adapter 設定 API は `src/fortran/oh_v2.F90` の
`ohhelp_v2` module からも使えます。Fortran 側では C 構造体を直接公開せず、
`type(oh_context_handle)` と `type(oh_particle_adapter_handle)` の opaque handle
として扱います。raw init bridge では粒子バッファを `type(c_ptr)` として受け渡し、
custom adapter の stride/datatype で Level 2/3 の粒子移動を行います。heap-owned
non-default context での Level 3 geometry、mapping、field facade、Level 1-3
`transbound` は Docker runtime test で 1 rank / 2 rank ともに確認しています。
C/Fortran ともに同一 communicator 上の 2 つの non-default context を smoke-test
し、C 側では別々の Level 3 geometry と adapter state を保持できることも確認しています。

Level 4p/4s は v2.x の継続対応対象です。v2.0 では既存コードの compile
coverage と移行中の実装境界を維持しますが、custom particle layout を含む
完全対応の完了条件には含めません。

## Level ごとに章を分けるべきか

分けるべきです。OhHelp の level は単なる内部実装差ではなく、利用側が
どこまでをライブラリに任せるかを決める境界です。

- Level 1: 負荷分散スケジュールと communicator を使い、粒子転送は利用側で行う。
- Level 2: 粒子バッファの転送も OhHelp に任せる。
- Level 3: 空間分割、粒子の隣接領域 mapping、場データの境界交換も使う。
- Level 4p/4s: 位置を意識した粒子管理と per-grid 粒子分布を OhHelp 側で扱う。
  v2.0 では移行中の拡張扱いとし、v2.x で supported 対象へ引き上げます。

そのため、共通の PIC lifecycle を先に説明し、その後で level ごとの差分を
章として分ける構成が最も読みやすいです。

## 基本方針

- `ohhelp_c.h` / `ohhelp_f.h` の `oh_*` alias を使うと、選択した
  `OH_LIB_LEVEL` に対応する実体へ展開されます。
- v2 では、後方互換よりも明示的な context、外部 particle layout、
  region weight を使う負荷分散を優先します。
- 現在の実装は default context との併存期間です。non-default context は
  Level 1-3 `transbound`、Level 3 geometry/mapping/field facade まで実行でき、
  C runtime smoke では複数 context の基本的な独立性も確認しています。
