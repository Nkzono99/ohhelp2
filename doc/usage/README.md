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

## Level ごとに章を分けるべきか

分けるべきです。OhHelp の level は単なる内部実装差ではなく、利用側が
どこまでをライブラリに任せるかを決める境界です。

- Level 1: 負荷分散スケジュールと communicator を使い、粒子転送は利用側で行う。
- Level 2: 粒子バッファの転送も OhHelp に任せる。
- Level 3: 空間分割、粒子の隣接領域 mapping、場データの境界交換も使う。
- Level 4p/4s: 位置を意識した粒子管理と per-grid 粒子分布を OhHelp 側で扱う。

そのため、共通の PIC lifecycle を先に説明し、その後で level ごとの差分を
章として分ける構成が最も読みやすいです。

## 基本方針

- `ohhelp_c.h` / `ohhelp_f.h` の `oh_*` alias を使うと、選択した
  `OH_LIB_LEVEL` に対応する実体へ展開されます。
- v2 では、後方互換よりも明示的な context、外部 particle layout、
  region weight を使う負荷分散を優先します。
- 現在の実装は default context を中心に移行中です。複数 context を完全に
  独立運用する API はまだ完成形ではありません。
