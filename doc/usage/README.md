# OhHelp v2 Usage Guide

このディレクトリは、OhHelp を PIC コードへ組み込む利用者向けの
実装ガイドです。PDF 由来の `doc/markdown/` は v1 系の詳細な
リファレンスとして残し、この usage guide では v2 で推奨する呼び出し順と
設計上の考え方をまとめます。

## 読む順番

1. [PIC integration lifecycle](pic-lifecycle.md)
   - PIC の初期化、粒子 push、電流 scatter、場の solve、負荷分散を
     どの順番で呼ぶかを説明します。
2. [API by OhHelp level](api-by-level.md)
   - `OH_LIB_LEVEL` ごとの API と、どの level を選ぶべきかを説明します。
3. [v2 particle layout and weighted load](v2-particle-and-weight.md)
   - v2 で追加・整備中の particle adapter と region weight の使い方を
     説明します。

## v2.0 の対象範囲

v2.0 では Level 1-3 を利用可能な範囲として固めます。通常の PIC 利用では
Level 3 を標準選択とし、粒子転送だけを任せたい場合は Level 2、既存の
粒子通信を維持したい場合は Level 1 を選びます。

Fortran からも Level 1-3 は利用対象です。`ohhelp_f.h` の alias と
`src/fortran/oh_mod1.F90` / `oh_mod2.F90` / `oh_mod3.F90` の module は
Docker 検証で compile-check され、`sample/sample.F90` も Level 3 の
Fortran 利用例として compile-check されます。

v2 の context facade と particle adapter は `src/fortran/oh_v2.F90` の
`ohhelp_v2` module からも使えます。Fortran 側では C 構造体を直接公開せず、
`type(oh_context_handle)` と `type(oh_particle_adapter_handle)` の opaque handle
として扱います。複数 context の完全な独立運用は引き続き v2.x の対象ですが、
default context に対する Level 1-3 操作、offset-based adapter、callback adapter
の設定は Fortran から呼べます。

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
- 現在の実装は default context を中心に移行中です。複数 context を完全に
  独立運用する API はまだ完成形ではありません。
