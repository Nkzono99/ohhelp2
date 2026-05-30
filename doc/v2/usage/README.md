# OhHelp v2 Usage Guide

この guide は、OhHelp v2 を PIC code に組み込むための利用者向け文書です。
v1 の PDF 由来資料は [`../../v1/`](../../v1/) に分離しています。v2 では
context API、particle adapter、weighted load を主役として説明します。

Fortran からも Level 1-3 は利用対象です。v2 の Fortran 利用では
`ohhelp_v2` module だけを使います。v1 style API は
[`../../v1/`](../../v1/) を入口に参照してください。

## 読む順番

1. PIC integration lifecycle
   - [C](pic-lifecycle.md)
   - [Fortran](pic-lifecycle-fortran.md)
2. API by OhHelp level
   - [C](api-by-level.md)
   - [Fortran](api-by-level-fortran.md)
3. v2 particle layout and weighted load
   - [C](v2-particle-and-weight.md)
   - [Fortran](v2-particle-and-weight-fortran.md)

Fortran migration code should also read
[`../design/index-conventions.md`](../design/index-conventions.md). The v2
context API uses zero-based field and border type ids even when called through
`ohhelp_v2`.

`sample/v2_context_level2_custom_particle.c` と
`sample/v2_context_level2_custom_particle.F90` は v2 heap context と custom
particle adapter の run-check 対象です。`sample/sample.F90` は legacy
Fortran compile-check 対象です。任意の Fortran 粒子 layout は
`ohhelp_v2` の adapter handle と raw init bridge で扱います。raw init
bridge には `oh2_init_raw()` / `oh3_init_raw()` があります。

## C / Fortran Mirror 構成

各 usage page は C 版と Fortran 版を mirror します。C 版は
`oh_context *` と `oh_particle_adapter` を直接使います。Fortran 版は
`type(oh_context_handle)` と `type(oh_particle_adapter_handle)` を使い、
`use iso_c_binding` と `use ohhelp_v2` のみを前提にします。

## 現在の実装ステータス

| 項目 | C | Fortran |
| --- | --- | --- |
| Level 1-3 context transbound | supported | `ohhelp_v2` から supported |
| Level 3 geometry / field facade | supported | `ohhelp_v2` から supported |
| region weight | supported | `ohhelp_v2` から supported |
| custom particle adapter | supported | `ohhelp_v2` から supported |
| 任意 layout の粒子配列を init に渡す経路 | supported | `oh2_init_raw()` / `oh3_init_raw()` |
| Level 4p/4s | v2.x 対象 | v2.x 対象 |

## v2.0 の対象範囲

v2.0 は Level 1-3 を利用可能な範囲として固めます。通常の PIC 利用では
Level 3 を標準選択とし、粒子転送だけを任せたい場合は Level 2、既存の
粒子通信を維持したい場合は Level 1 を選びます。

設計上の詳細は [`../design/`](../design/README.md) にまとめています。
