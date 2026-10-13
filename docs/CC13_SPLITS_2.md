# CC13 splits 2: function gaps and splits found by cc13-gp85

Packet `cc13-splits2`, Claude worker in the cc13 lineage, 2026-10-12. This packet repairs the Ghidra
function boundary problems listed in docs/CC13_GP_0085_008F.md finding 2, using the method of
docs/CC13_SPLITS_1.md. No C++ was written. The only code change is a one-line exporter fix (see the
last section). No game was run.

Names are hypotheses, not recovered symbols. Each ledger row has the evidence prefix `cc13-splits2`.

## Method

- **Boundaries.** Each start and end was read from the disk bytes with `bsp.py disasm-raw`. The
  enclosing Ghidra function's body was read live with `get_function_by_address`.
- **Split or separate function.** A true split continues mid-stream after a CALL, with no RET or JMP
  before it. A separate function starts after INT3 padding, following a function that ends at its
  own RET or JMP.
- **References.** References were checked with `local/sp2_rel32.py`. It scans the PE on disk for
  E8/E9 rel32 and for absolute dwords. As a positive control it found the vtable slots of 0085F2F0
  and 0085F7A0.
- **Ghidra writes.** These went through `tools/ghidra_define_function.py --record
  reports/cc13_splits2_defined.json` under the write lock. Before the run I checked with
  `list_analyzers` that 'Non-Returning Functions - Discovered' is disabled. No analysis pass was
  started.
- **After the writes.**
  - The new functions were named with `ledger add-name` and annotated with
    `tools/ghidra_annotate.py --apply`. The prior state is in
    `local/ghidra-annotations-20261012T230132Z.json` in the worktree.
  - Exports were refreshed with `bsp.py ghidra export <addrs> --force`. That covered all 22 new
    functions and the four neighbours 0085F2B0, 008619C0, 008DAD30 and 008E4B00.

## Results

The two true splits were repaired first, in packet cc13-gp85 (commit `76595ac6a`, record
`reports/cc13_gp85_defined.json`, docs/CC13_GP_0085_008F.md "Split repairs"):

| true head | mid-body function deleted | new body | name |
|---|---|---|---|
| 0085F7A0 | 0085F7C0 | 0085F7A0-0085F8D0 | `BSP_WaterMine_OnWrecked` (vtable `00D0D130` +7Ch) |
| 008D9750 | 008D97A0 (reached by `JNE` at 008D978E, EBP set by the head) | 008D9750-008D98D1 | `BSP_SceneTokenizer_ConvertByUnitToken` |

The remaining items were all separate functions. Each one follows INT3 padding, and the Ghidra
function before it already ends at its own RET: 0085F2B0 at 0085F2E5, 008619C0 at 008619C6,
008DAD30 at 008DAD65 and 008E4B00 at 008E4B3E. Each was therefore defined as a new function.
Nothing was recreated or deleted, and no parent changed. The gaps held more functions than were
first reported: 00861B10 and 00861B30 were two of 13 functions in the 008619C6-00861B40 gap, and
the 008DAD65-008DADB0 gap also holds 008DADA0.

| start | end (excl.) | name | evidence | referenced |
|---|---|---|---|---|
| 0085F2F0 | 0085F30D | `BSP_WaterMine_HandleMessage` | vtable `00D0D130` +164h (dword at `00D0D294`); MT 0B5h `MT_WATERMINE_ARM` calls `BSP_WaterMine_Arm` (0085F1E0), else JMP `BSP_BombProjectile_HandleMessage` | vtable |
| 008619D0 | 008619ED | `TMPL_Record12_Construct_008619D0` | stores {dword, byte, float}, RET 0Ch | no (only the `00D7D1B0` table of mid-instruction addresses) |
| 008619F0 | 008619FB | `TMPL_Handle_Construct_008619F0` | stores one dword, RET 4 | no |
| 00861A00 | 00861A10 | `TMPL_Handle_Equals_00861A00` | `*arg == *this`, RET 4 | no |
| 00861A10 .. 00861B10 | each +11h (00861AF0: +13h) | `BSP_ArtillerySubDirectorBot_Get{MaxErrorRadius, ErrorRangeMul, MinErrorMul, ApproachMulMin, ApproachMulMax, DeviationMul, AngleChange, ErrorDistIncStartTime, ErrorDistIncFullTime}` | `[00E19994] + 28h*[ECX+14h] + 0Ch..2Ch`, the ArtillerySubDirectorBot level rows of `include/bsp/robot_config.hpp` (reader 008FCF30); 00861AF0 indexes `28h*(level+1)` for +28h | no |
| 00861B30 | 00861B3B | `BSP_GunneryTargetRecord_Destruct` | stores vtable `00D0D2FC`, JMP `BSP_CallbackOwner_Destroy` (00695870) | no |
| 008DAD70 | 008DAD74 | `BSP_ObjectiveEntry_GetField28_Unreferenced` | `MOV EAX,[ECX+28h]`; class from position only (provisional) | no |
| 008DAD80 | 008DADA0 | `BSP_ObjectiveSessionMessage_Construct` | `BSP_SessionMessage_ConstructBase(id)`, +4 = 1, vtable `00D16024` | no |
| 008DADA0 | 008DADAC | `BSP_World_HasEntitySet0_Unreferenced` | `[ECX+21A4h] != 0`, the world entity-set array (provisional class) | no |
| 008E4B40 | 008E4B5E | `BSP_PowerupSlotRecord_IsCooldownExpired` | not deferred (+14h) and now (`00F876A4`) > end (+10h) | no |
| 008E4B60 | 008E4B80 | `BSP_PowerupSlotRecord_GetReadyTime` | deferred: float(class+7Ch + now), else +0Ch | no |
| 008E4B80 | 008E4BA3 | `BSP_PowerupSlotRecord_GetEndTime` | deferred: float(class+80h + now), else +10h | no |

Most of the new functions have no code reference. They are out-of-line copies of inlined members.
The compiler emitted them, but nothing in the image calls them.

## Exporter fix

`tools/ghidra_export.py` skipped a function as "not a function in this snapshot or in Ghidra"
whenever the live `get_function_by_address` text contained "error" in any case. A function *named*
`..._GetMaxErrorRadius` therefore could not be exported: 5 of the new names failed. The test now
requires the reply to start with `Function: `. After the fix the five exported normally.

## For cc9

- The water mine's message handler (0085F2F0) and OnWrecked (0085F7A0) are now real functions, so
  their callees, exports and `scan-bytes` attribution are correct. OnWrecked draws its damage from
  RNG stream 1.
- The ArtillerySubDirectorBot level-row getters confirm the row layout in
  `include/bsp/robot_config.hpp`. They are unreferenced, so the live readers inline these loads.
- A reconstruction fragment record keyed at 008D97A0 now points at a non-function. Its function is
  008D9750.
