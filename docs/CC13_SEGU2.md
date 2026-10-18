# cc13-segU2: undone lists of segments 94, 90 and 39

The packet covers three lists from the cc13 lead tree:
- `local/cc13_undone_seg94.txt`: 38 rows, 00BC94D0-00BCF720.
- `local/cc13_undone_seg90.txt`: 34 rows, 00B86F90-00B89EF0.
- `local/cc13_undone_seg39.txt`: 35 rows, 007231F0-00728860.

Each address was checked against rule 1: `git grep` over src/include, the
reconstruction shards, and `bsp.py lookup`. None has a reconstruction record.
None of the 107 rows is a live game body, so nothing was reconstructed, wired or
built.

| List | Rows | What they are | Outcome |
|---|---|---|---|
| seg94 | 38 | zlib 1.2.1 (inflate/deflate/trees/crc32, compress/uncompress, zcalloc/zcfree) | Already tagged `zlib121` in `config/tags/00bc0000.jsonl` (since e986705f7, from reports/library_inventory/zlib_and_borrowed.json). Bind-only; no change. |
| seg90 | 31 | STL template bodies: the resource-instance-type `std::_Tree` (buy/link node, rotate, iterator steps, insert-unique/hint, clear, construct/destruct) and pair vectors/ranges of 8-byte elements (allocate, tidy, insert-n, copy/fill/copy-backward) | Tagged `stl_shape`, action `reviewed`, evidence `cc13segu2_stl` |
| seg90 | 3 | `Catch_All@` funclets 00B88894, 00B88A17, 00B89136 | Tagged `stl_shape`, evidence `cc13segu2_catch` |
| seg39 | 27 | STL template bodies: tree buy-node, 6-byte, 20-byte and word ranges (copy/move/fill/uninitialized), vector tidy/insert-n/assign, and the 2Ch-byte geom-mesh element vector helpers | Tagged `stl_shape`, evidence `cc13segu2_stl` |
| seg39 | 8 | `Catch_All@` funclets 00725C39, 00726097, 00726395, 00726ACA, 00726B5D, 00726E8F, 0072708A, 00727133 | Tagged `stl_shape`, evidence `cc13segu2_catch` |

The seg94 rows appeared on the undone list even though their zlib121 tags are
on main. The lead's list was probably built from an index that does not count
that tag. Rebuilding the index in this tree shows them as tagged.

## Evidence for the STL rows

The rows already carry shape names from earlier sweeps. Their callees were
checked: they call only other STL rows, operator new 00BF681B, `_free`
00BF65AC, the `xlen` throws and 00BF6713. For example:
- 00B888B0 InsertN calls only 00B86F90, 00B876C0, 00B87DA0, 00B886B0 and `_free`.
- 00B89A50 InsertHint calls only the tree iterator, link and insert rows and
  00BF6713.
- 00726EB0 InsertN calls only the geom-mesh range rows, the allocator, 00725C50
  `xlen` and the element copy-constructor 0058AAC0.

The geom-mesh helpers 00725670, 007256A0 and 00726A80 are already host hooks in
`include/bsp/director_geommesh_seg38_39_argon.hpp`. The game code that uses them
is reconstructed there, so binding them is enough.

Two funclets were read from the listing:
- 00B88A17: `_free([EBP+14h])`, then `__CxxThrowException@8(0, 0)` (00BF6885),
  which is a rethrow.
- 00727133: destroy range 00725670 over the 2Ch-byte elements, then the same
  rethrow.

Under the 2026-10-16/17 library ruling, all of these are bind-only.

## Files

- `config/tags/00b80000.jsonl`: 34 rows appended.
- `config/tags/00720000.jsonl`: 35 rows appended.
- `config/tags/evidence.json`: two new ids, `cc13segu2_stl` and `cc13segu2_catch`.
