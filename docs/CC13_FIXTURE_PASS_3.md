# cc13 fixture pass 3: Dyn leaf routines

Packet `cc13-fixture3` (2026-10-24). This pass continues docs/CC13_FIXTURE_PASS_2.md and uses the same
harness: `tools/verify_cc13_leaf_reference.py` audits the original bytes and relocations, and
`tests/cc13_leaf_fixture_tests.cpp` (CTest `cc13_leaf_differential`) runs each original body next to
its reconstruction. The input set is fixed pseudo-random, mixed with edge values (signed zeros, a
quiet NaN, denormals, FLT_MIN, FLT_MAX, infinities, 1e30) and branch boundaries. Every case runs at
both 53-bit and 24-bit x87 precision control.

## Selection

Eleven leaf routines in the Dyn band, each with a reconstruction that has no unresolved calls:

- the box and cylinder support functions (00C38470, 00C38810, 00C47AA0)
- the segment-box entry (00C412D0)
- the four polygon clips (00C313A0, 00C30D10, 00C31060, 00C309D0)
- expand node matrices (00C34CF0)
- add force at point (00C37F60)
- interpolate body transform (00C43EA0)

The audit covers 21267 bytes and 100 relocations. It adds the double constants 00D7A250 and
00D7A298 to the data list.

## Deciding precision

The deciding precision is that of the thread that calls the routine. A routine reached only from
Dyn workers is judged at 53-bit. Every other routine is judged at 24-bit (main thread), and so is
any routine with no live caller.

| Address | Routine | Thread evidence | Deciding |
| --- | --- | --- | --- |
| 00C38470 | box support (double) | called only from GJK 00C51C20/00C51EF0, reached through 00C53010 from 00C535E0 and 00C58EB0 (narrow phase, Dyn workers) | 53 |
| 00C38810 | cylinder support (double) | same GJK chain | 53 |
| 00C47AA0 | box support (float, slot 2) | only the vtable entry refers to it; no slot-2 call site was found | 24 |
| 00C412D0 | segment-box entry | 00C445B0 world ray cast; its callers 00C4DA70 and 00C55DB0 are undefined and unreferenced | 24 |
| 00C313A0, 00C30D10, 00C31060, 00C309D0 | clips | undefined chunk 00C34C60, unreferenced | 24 |
| 00C34CF0 | expand node matrices | undefined chunk 00C37530, unreferenced | 24 |
| 00C37F60 | add force at point | 0092B350 TickAdvance, a fixed-step pool job that can run on either thread | 24 |
| 00C43EA0 | interpolate body transform | 00447B80 UpdateFrame (main thread), 00929CB0, 0092F930 | 24 |

## Results (edge-value set)

| Address | Deciding | Other precision | Outcome |
| --- | --- | --- | --- |
| 00C38470 | 53: 3000/3000 | 24: 318/3000 | promoted |
| 00C38810 | 53: 2996/3000, plus 4 that differ only in NaN sign or payload | 24: 567/3000 | pinned |
| 00C47AA0 | 24: 2999/3000, plus 1 that differs only in NaN sign or payload | 53: same | pinned |
| 00C412D0 | 24: 3000/3000 | 53: 2994/3000 | promoted (remodelled) |
| 00C313A0, 00C30D10, 00C31060, 00C309D0 | 24: 2000/2000 each | 53: 2000/2000 each | promoted |
| 00C34CF0 | 24: 1000/1000 | 53: 1000/1000 | promoted |
| 00C37F60 | 24: 3000/3000 | 53: 1699/3000 | promoted (remodelled) |
| 00C43EA0 | 24: 3000/3000 | 53: 827/3000 | promoted (remodelled) |

Nine routines were promoted to `semantic_native_fixture_tested` and two were pinned. All eleven
match on every case when the edge values are removed (`BSP_FIXTURE_NO_EDGES`). Every ledger row
keeps `abi_compatible` false and its `cpp_symbol`.

## Pins

The x87 chooses which operand's NaN to propagate, and how to quiet it, by its own rules. The double
or float C++ produces a NaN in the same cases, but not always with the same sign or payload.
`tests/cc13_leaf_fixture_tests.cpp` pins those counts (00C38810: 4, 00C47AA0: 1) with the reason
"NaN sign/payload only". The rows stay at `reconstructed_build_tested`.

## Remodels

Three reconstructions used plain float or double arithmetic where the original keeps intermediates on
the x87 stack at 24-bit precision. Each now goes through the `x87_*24` helpers in
`include/bsp/cc13_x87_model.hpp`. Those helpers round to a 24-bit significand with the stack's
exponent range, which avoids float overflow on intermediates and double rounding of denormals.

- 00C412D0 `segment_box_entry_00c412d0` (src/cc13_dyn1.cpp): plane - start, the quotient by the
  direction, and the entry point. Before: 2994/3000 at 24-bit.
- 00C37F60 `body_add_force_at_point_00c37f60` (src/cc13_dyn1.cpp): the force adds, the lever, each
  cross-product term and the torque adds. Before: 2980/3000 at 24-bit.
- 00C43EA0 `interpolate_body_transform_00c43ea0` (src/game_dynamics_list.cpp): a + (b - a) * t
  rounded per op. Before: 2927/3000 at 24-bit.

No host calls any of these bsp functions, so the remodels change no live path and need no OFF
switch.
