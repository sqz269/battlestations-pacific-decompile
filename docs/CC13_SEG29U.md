# cc13-seg29U: segment 29's leftover functions

Segment 29 (00696470-006BEB10) had 71 functions with no reconstruction record and no library
tag (the list in the cc13 lead tree's `local/cc13_undone_seg29.txt`).

| Group | Count | Outcome |
|---|---|---|
| STL template instantiations | 32 | Tagged `stl_shape`, action `reviewed` (name kept). Bind-only. |
| `Catch_All@` EH funclets | 9 | Tagged `stl_shape`, action `reviewed`. They bind with their parents. |
| cc9 landing approach, 006AC000-006ACFFF | 3 | Not touched; cc9's lane |
| cc9 air ops, 006BA620 and up | 27 | Not touched; cc9's lane |
| Non-gameplay bodies left to reconstruct | 0 | |

Nothing was reconstructed, wired or built. The packet changes only tag rows (`config/tags/00690000.jsonl`,
`006a0000.jsonl`, `evidence.json`) and this document.

## Ownership

The lanes come from `tools/raw_ownership_cc13.py`:
- `00696470-006BA61F` is non-gameplay (ng). Every row of this list in that span is STL or a funclet.
- `006AC000-006ACFFF` is cc9 (the ship AI landing approach and command-building landing paths).
- `006BA620-006CFFFF` is cc9 (air ops).

The STL row 006ACD80 `STL_List_PushBack_006acd80` sits inside the cc9 span. It was tagged with
the rest of the STL rows, because a tag is not a reconstruction.

## Library rows (bind-only)

The 32 STL rows are the bodies of `std::_Tree`, `std::vector` and `std::list` templates. Each
one's ledger-name evidence (docs/CC13_BAND_0067_0073.md) describes the template shape. The
rows fall into these groups:
- head-node constructors: 0069DCA0, 0069E190, 0069E1C0, 006A20A0, 006A2C40, 006AD5D0, 006AFE50;
- iterator pre-decrement: 0069C570, 0069D0C0;
- tree assign (erase then copy when distinct): 006A1E30, 006A2470, 006A3870, 006A4470,
  006A46D0, 006A4C30;
- tree clear and insert: 006A1260, 006A1520, 006A41A0, 006A49F0, 006A5EF0;
- vector tidy and range helpers: 0069FB50, 006A0B50, 006A13B0, 006A1D30, 006A2200, 006A2240,
  006A2270, 006A23F0, 006A3180, 006A31B0, 006A6BB0;
- list push-back: 006ACD80.

The user ruling of 2026-10-16/17 binds library bodies by default. No game call site in this
segment needs a non-standard ABI from any of these rows, so none were reconstructed.

The nine funclets are 0069F9BE, 006A00F6, 006A1322, 006A22CB, 006A239E, 006A29A1, 006A3E9E,
006A3F27 and 006A4B2E. Each is a `catch(...)` cleanup of a neighbouring STL range or insert
body. Spot checks of 0069F9BE, 006A22CB, 006A3E9E and 006A4B2E found two patterns:
- a call to `_free` 00BF65AC, then a rethrow through `__CxxThrowException@8` 00BF6885 with null
  arguments;
- at 006A22CB, `STL_Vector_Tidy_0069e9b0` and then the same rethrow.

## cc9 rows (not touched)

Rule 1 found no faithful body on main for any of the 30 cc9 rows.

Some have **host models** in cc9's `game_hosts_units.cpp`. These are host adaptations, not
reconstructions, so they get no record here:
- 006BCA40: `landing_t_world_006bca40`.
- 006BCA80: `land_path_point_006bca80`.
- 006BCD80/006BCE60: `land_set_held_006bcd80(bool)`.
- 006BD080: the `find_landing_assignment` hook.

The other rows appear only as **host interface hooks** or comments:
- `cc13_sweepC`: 006BA670, 006BDBF0.
- `cc13_seg41_land`: 006BA670.
- `cc13_seg30`: 006BC5E0, 006BD620, 006BD800.
- `cc13_sweepC2`: 006BD230.
- `cc13_left55_seg56a`: 006BD800.
- `gameplay_seg52_cc13`: a comment on 006BC030's inline copies.
- `air_operations`: comments on 006BC5E0, 006BC6F0, 006BC730 and 006BC8E0.
- `return_to_base.hpp`: a stand-in flag for 006BC120.
- `game_hosts_ship_ai.cpp`: a record-only stand-in for 006AC370.

The 30 rows are 006AC370, 006ACB40, 006ACDC0, 006BA670, 006BC030, 006BC120, 006BC3E0,
006BC5E0, 006BC690, 006BC6F0, 006BC730, 006BC800, 006BC870, 006BC890, 006BC8E0, 006BC960,
006BCA40, 006BCA80, 006BCCE0, 006BCD80, 006BCE60, 006BCF40, 006BCFD0, 006BD080, 006BD230,
006BD620, 006BD800, 006BD9E0, 006BDAC0 and 006BDBF0. They are all cc9's to reconstruct or
assign.
