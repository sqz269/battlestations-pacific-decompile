# CC13 _free flow-override repair (cc13-flowfix1)

Status: done 2026-10-11, apart from 8 call sites in regions other workers have leased (listed below).
This packet repaired the Ghidra listing only, with no C++ changes. Machine-readable record:
reports/cc13_free_flow_repair.json. Grew-list table: docs/CC13_FREE_FLOW_REPAIR_GREW.md.

## Problem

Ghidra dropped the code after calls to `_free` because those CALL instructions carried a
`CALL_RETURN` flow override and the bytes after them were undefined. Function bodies stopped at the
call, and the decompiler lost everything after the free. Example: 00B8D660
`Mit_PoolAllocator_480_32_Trim` lost its move-last-block-and-renumber loop at 00B8D686..00B8D6BF.

## Root cause

- `_free` 00BF9DC8 returns: it has reachable `RET`s at 00BF9E55 and 00BF9E26. Its thunks are
  00BF65AC (`jmp 00BF9DC8`) and 00BF6989 (`jmp 00BF65AC`), and Ghidra derives their no-return state
  from 00BF9DC8.
- Ghidra's analyzer "Non-Returning Functions - Discovered" marked `_free` no-return and left the
  bookmark "Non-Returning Function Found". With its "Repair Flow Damage" sub-option it then cleared
  the code after every call and set `CALL_RETURN` on each call site.
- The analyzer counts calls whose fallthrough is undefined bytes as evidence of no-return, so the
  existing gaps sustained themselves. Any auto-analysis pass, triggered by `create_function` or
  `disassemble_bytes` from any agent, could re-flag `_free` and clear its call sites across the
  whole image again.
- This happened during this packet:
  - At the start, `ghidra proto 00BF65AC` printed `void __cdecl _free(...)` and the census had 1992
    OK `_free`-family call sites.
  - The pilot then repaired 41 functions: 00B8D660 and 004010F0..0040B160. Each was handled by
    clearing the override, flow-disassembling, then deleting and re-creating the function. These
    writes set off auto-analysis.
  - Afterwards all three `_free` functions read no-return, and every thunk call site was override+gap
    again. That undid cc13-rewards1's earlier repair of 0062C2C0, and about 30 `operator_new` call
    sites fell inside the cleared code.
- `operator_new` 00BF681B and its thunk 00BF55BE were never no-return, and none of their decodable
  call sites carried an override. They were not changed.

## Analyzer option: left DISABLED on purpose

The user unchecked "Non-Returning Functions - Discovered" in the CodeBrowser Auto Analyze dialog on
2026-10-11. The GhidraMCP bridge cannot change analyzer options in GUI mode: `configure_analyzer`
exists only in the headless server, and scripts are disabled.

| option | before | now |
|---|---|---|
| Non-Returning Functions - Discovered | enabled | **disabled (deliberately left off)** |
| ... .Create Analysis Bookmarks | enabled | enabled (inert while the parent is off) |
| ... .Repair Flow Damage | enabled | enabled (inert while the parent is off) |
| Non-Returning Functions - Known | enabled | enabled (unchanged) |

It stays off for a reason. Re-enabling it makes the next auto-analysis pass evaluate `_free` call
sites again. Any remaining gap (the 8 leased sites below, or any future gap) can then re-flag `_free`
and wipe this repair across the whole image. There is no versioned backup of `C:/Users/sqz269/bsp.rep`.
Restore the old value (enabled) only after confirming that `_free` has no remaining gapped call
sites, and re-run the census afterwards.

`no_return` before/after, from local/ff1_state_before.json and local/ff1_state_after.json:

| function | before | after |
|---|---|---|
| 00BF9DC8 `_free` | true (`noreturn void __cdecl _free(void * _Memory)`) | false (`void __cdecl _free(void * _Memory)`) |
| 00BF65AC `_free` thunk | true | false |
| 00BF6989 `_free` thunk of thunk | true | false |
| 00BF681B / 00BF55BE `operator_new` | false | false |

## Procedure

All writes went through `coordination.ghidra_lock`, taken per 50 functions. Leases were claimed per
300-function chunk on the entry addresses, and functions leased by another owner were skipped. The
scripts lived in the worktree's `local/` (not committed): `ff1_census.py`, `ff1_repair.py`,
`ff1_drive.py`, `ff1_nofunc.py`, `ff1_one.py`.

1. Census: every E8/E9 rel32 site in the on-disk `.text` aimed at a target, plus absolute dwords.
   For each site, live `get_flow_metadata` gives the instruction at the site, the flow override, the
   instruction at site+5 and the containing function.
2. Per function, record the old values first: name, signature (including calling convention),
   no_return, body ranges, plate comment, and parameters plus non-phantom locals (variables were
   skipped for bodies over 1800h, because `get_function_variables` decompiles them). These go to the
   event log before any write.
3. Re-check each site: an instruction is present, the override is `CALL_RETURN`, Ghidra's bytes equal
   the disk bytes, and it is still a rel32 CALL to a `_free`-family target. Then clear the override
   to NONE and flow-disassemble from site+5.
4. If the body did not yet cover every site+5, delete the function and re-create it from its
   instructions (`create_function`, `disassemble_first=false`). Then restore the name, prototype and
   calling convention, plate comment and no_return wherever they differ, and compare against the
   recorded values.
5. Repeat the census and the passes until stable. Call sites outside any function were cleared and
   disassembled without creating a function.

Observed quirks:
- After a delete, auto-analysis sometimes re-created the function from its call references before the
  script's `create_function` ran, which then failed with "already exists". This happened for 004010F0,
  004042A0 and 004043D0. The labels kept the names, the plate comments survived, the bodies covered
  the sites, and the signatures were defaults, so nothing was lost. The script now treats "already
  exists" as an auto re-create and continues with the restore.
- `set_function_prototype` sometimes answers with non-JSON text. In 3 functions (00426520, 004324A0,
  00444760) the script flagged this, but the signature read back equal to the old one.
- `get_function_variables` on 0062C2C0 (43678 bytes) blocked the Ghidra HTTP server for about 9
  minutes. The run was stopped between functions and resumed, and large bodies now skip the variable
  read.
- Fallthrough code at 00CC2D79 (`Unwind@00cc2d70`) read as undefined again after its first repair.
  It was redone separately and held.

## Census per target

Classes:
- OK: no override, and an instruction follows the call.
- override+gap: an override, and undefined bytes follow the call.
- override-only: an override, but an instruction follows the call.
- not-code: the call bytes sit in undefined bytes.

| target | role | first census (start of packet) | after pilot damage | final |
|---|---|---|---|---|
| 00BF65AC | `_free` thunk | OK 1163, override+gap 5568, override-only 156, not-code 1108 | OK 0, override+gap 6848, not-code 1147 | **OK 7282**, override+gap 8 (leased), not-code 705 |
| 00BF6989 | `_free` thunk of thunk | OK 815, override+gap 1265, override-only 8, not-code 480 | OK 0, override+gap 2012, not-code 556 | **OK 2164**, not-code 404 |
| 00BF9DC8 | `_free` | OK 14, override+gap 112, not-code 96 | OK 7, override+gap 119, not-code 96 | **OK 194**, not-code 28 |
| 00BF681B | `operator_new` | OK 2594, not-code 45 | OK 2592, not-code 47 | OK 2597, not-code 42 |
| 00BF55BE | `operator_new` thunk | OK 1037, not-code 307 | OK 1007, not-code 337 | OK 1051, not-code 293 |

`_free`-family OK sites: 1992 at the start of the packet, 9640 at the end. The lead's target of 5623
or more OK sites was the first census's OK count over all five targets (1992 for `_free` plus 3631
for `operator_new`). At the end those five targets have 13288 OK sites. The census stayed unchanged
across the last two runs.

- All overrides seen were `CALL_RETURN`. The one remaining `jmp-override` is a tail `JMP _free` at
  00401027 in 00401010. `CALL_RETURN` on a JMP is Ghidra's tail-call model, so it was left alone.
- Absolute-dword references to 00BF9DC8 sit at 00A5D013, 00C05663 and 00C057BD. They are instruction
  operands, not flow.
- An offline scan of exported assembly found no other routine that returns normally with dropped
  fallthroughs. The remaining hits call genuinely no-return `__invoke_watson`,
  `__CxxThrowException@8`, `_abort` and `_exit`.
- The not-code hits that remain are call bytes inside code Ghidra never defined as a function. For
  example, 00404240 and 00404250 sit between 004041F0 and 004042A0, and 00410610 follows 00410600.
  They are not dropped fallthroughs of a repaired call. Defining those functions is out of this
  packet's scope, and the report lists the sites (`not_code_sites`).

Work done:
- 7224 functions were handled: 7205 in the main pass, then 54, 13 and 12 in follow-up passes, plus
  one single retry. Each follow-up pass picked up code that the previous one connected.
- 316 call sites outside any function were cleared and disassembled.
- No prototype, name, plate or no_return mismatch remained.

## Remaining (skipped: leased by another owner)

| site | containing function | lease |
|---|---|---|
| 00784083, 00784097, 0078471C, 00784756 | none | agent/cc13-net1:cc13-net1 |
| 007846E6 | 007844E0 | agent/cc13-net1:cc13-net1 |
| 00BE98CB, 00BED8DB, 00BF153B | none | agent/cc13-files1:cc13_files1_seg97 |

To fix them, the lease holder or a later pass runs the same steps: clear the override, flow-disassemble
from site+5, and re-create the owning function if its body does not cover the new code.

## Grew-list (requested by cc9)

- 7222 function bodies grew in total, 2643 of them with a new end address. The rest filled internal
  gaps. In all, 118709 bytes were added to bodies.
- 5838 of these were already truncated before this packet. The other 1384 only regained code lost
  during the pilot damage window, between the pilot and the end of the repair pass on 2026-10-11.
- "Old" is the body at the start of the repair pass (for the 41 pilot functions, the body before the
  pilot); "new" is after it.
- The table of all 840 already-truncated functions that have an owning doc is in
  docs/CC13_FREE_FLOW_REPAIR_GREW.md. Every grown function is in `grew` in the report, including those
  with no owning doc.
- Reconstructions in the owning docs may have been written from the truncated bodies. The rows for
  cc9-owned docs (GUNNERY_*, SHIP_AI*, SQUADRON_LAND_TASK), 110 functions:

| entry | name | old end | new end | old size | new size | owning doc(s) |
|---|---|---|---|---:|---:|---|
| 0046B730 | `FUN_0046b730` | 0046BD07 | 0046BD07 | 1493 | 1496 | docs/SHIP_AI_OPEN_ITEMS.md |
| 004E27E0 | `FUN_004e27e0` | 004E3A9A | 004E3A9A | 4752 | 4795 | docs/SHIP_AI_OPEN_ITEMS.md |
| 0057AC10 | `BSP_MultiSessionBrowserScreen_FinishJoin` | 0057AE4D | 0057AE4D | 563 | 574 | docs/SHIP_AI_OPEN_ITEMS.md |
| 0066DD00 | `FUN_0066dd00` | 0066DF2F | 0066DF2F | 455 | 551 | docs/SHIP_AI_OPEN_ITEMS.md |
| 0066E510 | `FUN_0066e510` | 0066E571 | 0066E587 | 98 | 120 | docs/SHIP_AI_OPEN_ITEMS.md |
| 0066E590 | `FUN_0066e590` | 0066E60E | 0066E60E | 118 | 127 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00673A10 | `FUN_00673a10` | 00674B72 | 00674B72 | 4150 | 4441 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 006B0000 | `CG_scalar_deleting_dtor_006b0000` | 006B001D | 006B001D | 27 | 30 | docs/SQUADRON_LAND_TASK.md |
| 006C06E0 | `CG_scalar_deleting_dtor_006c06e0` | 006C070F | 006C070F | 42 | 48 | docs/SHIP_AI_OPEN_ITEMS.md |
| 006C48F0 | `FUN_006c48f0` | 006C498B | 006C498B | 149 | 156 | docs/SQUADRON_LAND_TASK.md |
| 006C7210 | `FUN_006c7210` | 006C748B | 006C748B | 630 | 633 | docs/SHIP_AI_OPEN_ITEMS.md |
| 006C80C0 | `FUN_006c80c0` | 006C83A0 | 006C83A0 | 734 | 737 | docs/SHIP_AI_OPEN_ITEMS.md |
| 006CADD0 | `BSP_AirOps_LoadFromScene` | 006CC5BD | 006CC5BD | 6114 | 6126 | docs/SQUADRON_LAND_TASK.md |
| 006CE910 | `FUN_006ce910` | 006CE992 | 006CE992 | 128 | 131 | docs/SHIP_AI_OPEN_ITEMS.md |
| 006D3B10 | `FUN_006d3b10` | 006D3C0C | 006D3C0C | 247 | 253 | docs/SQUADRON_LAND_TASK.md |
| 006D40D0 | `CG_scalar_deleting_dtor_006d40d0` | 006D40ED | 006D40ED | 27 | 30 | docs/GUNNERY_OPEN_ITEMS.md |
| 006FAD70 | `FUN_006fad70` | 006FADEF | 006FAEE4 | 125 | 370 | docs/GUNNERY_OPEN_ITEMS.md |
| 00718C70 | `BSP_ModelNoteNameEquals_Predicate` | 00718CF1 | 00718CF1 | 127 | 130 | docs/GUNNERY_OPEN_ITEMS.md |
| 00719FA0 | `BSP_ModelNoteList_FindIfNameEquals` | 0071A00D | 0071A00D | 100 | 103 | docs/GUNNERY_OPEN_ITEMS.md |
| 0071AAE0 | `BSP_ModelNoteList_FindByName` | 0071AB5F | 0071AB5F | 125 | 128 | docs/GUNNERY_OPEN_ITEMS.md |
| 0071BA20 | `FUN_0071ba20` | 0071BB35 | 0071BB35 | 275 | 278 | docs/GUNNERY_OPEN_ITEMS.md |
| 00729600 | `FUN_00729600` | 00729660 | 00729660 | 94 | 97 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 0072AD20 | `CG_scalar_deleting_dtor_0072ad20` | 0072AD3D | 0072AD3D | 27 | 30 | docs/GUNNERY_OPEN_ITEMS.md |
| 0072D950 | `FUN_0072d950` | 0072DBCA | 0072DBEF | 611 | 672 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 0072E6D0 | `BSP_Gun_SetupFromDescriptor` | 0072F053 | 0072F053 | 2433 | 2436 | docs/GUNNERY_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 0072F6E0 | `BSP_Gun_TargetShotDecision` | 0072F82B | 0072F82B | 329 | 332 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 00733D50 | `FUN_00733d50` | 00733D7C | 00733D7C | 30 | 45 | docs/SQUADRON_LAND_TASK.md |
| 00733FF0 | `FUN_00733ff0` | 0073409F | 0073409F | 147 | 176 | docs/SQUADRON_LAND_TASK.md |
| 007340A0 | `FUN_007340a0` | 0073413C | 0073413C | 141 | 157 | docs/SQUADRON_LAND_TASK.md |
| 00747CC0 | `FUN_00747cc0` | 00747DBA | 00747DBA | 248 | 251 | docs/GUNNERY_OPEN_ITEMS.md |
| 0074BEC0 | `FUN_0074bec0` | 0074C273 | 0074C273 | 942 | 948 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00759120 | `BSP_MotherShipClass_BindModelData_Provisional` | 00759299 | 00759299 | 372 | 378 | docs/SQUADRON_LAND_TASK.md |
| 0076E520 | `BSP_Session_EnqueueIncomingMessage` | 0076E70C | 0076E70C | 481 | 487 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_AVOIDANCE_OWNERS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 0077BEA0 | `FUN_0077bea0` | 0077BF3B | 0077BF3B | 149 | 156 | docs/SHIP_AI_OPEN_ITEMS.md |
| 007B6810 | `CG_scalar_deleting_dtor_007b6810` | 007B682D | 007B682D | 27 | 30 | docs/SQUADRON_LAND_TASK.md |
| 007CAAD0 | `FUN_007caad0` | 007CAF08 | 007CAF08 | 1078 | 1081 | docs/GUNNERY_OPEN_ITEMS.md |
| 007D5D20 | `BSP_Plane_ReadPropertyBag` | 007D771E | 007D771E | 6643 | 6655 | docs/GUNNERY_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 007F0280 | `BSP_Bot_NearFieldUnitAvoidanceProbe` | 007F0B21 | 007F0B21 | 2201 | 2210 | docs/SQUADRON_LAND_TASK.md |
| 007F1140 | `CG_scalar_deleting_dtor_007f1140` | 007F115D | 007F115D | 27 | 30 | docs/SQUADRON_LAND_TASK.md |
| 007F1EE0 | `CG_scalar_deleting_dtor_007f1ee0` | 007F1EFD | 007F1EFD | 27 | 30 | docs/SQUADRON_LAND_TASK.md |
| 007F5A10 | `BSP_GunPlatform_SplitInsertArc` | 007F5FB8 | 007F5FB8 | 1443 | 1449 | docs/GUNNERY_OPEN_ITEMS.md |
| 007F7110 | `FUN_007f7110` | 007F72FE | 007F72FE | 492 | 495 | docs/GUNNERY_OPEN_ITEMS.md |
| 0081DE10 | `FUN_0081de10` | 0081E34E | 0081E34E | 1327 | 1333 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 0081E350 | `FUN_0081e350` | 0081E491 | 0081E491 | 319 | 322 | docs/GUNNERY_OPEN_ITEMS.md |
| 0081F980 | `BSP_ShipUnit_BindSectionPoints_Provisional` | 008206EB | 008206EB | 3433 | 3436 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_HEADING_TO_RUDDER.md, docs/SHIP_AI_NAV_BLOCK_CTOR.md, docs/SHIP_AI_TARGET_CURVE.md |
| 00824B60 | `BSP_UnitInstance_OnWrecked` | 008252B0 | 008252B0 | 1867 | 1873 | docs/GUNNERY_OPEN_ITEMS.md |
| 008485F0 | `FUN_008485f0` | 008486A9 | 008486A9 | 183 | 186 | docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00849F70 | `FUN_00849f70` | 0084A5CF | 0084A5CF | 1629 | 1632 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 0084ACB0 | `FUN_0084acb0` | 0084ADAB | 0084ADAB | 249 | 252 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 0084BAD0 | `BSP_Explosion_ApplyRadialDamage` | 0084BC5D | 0084BC5D | 395 | 398 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 0084D560 | `FUN_0084d560` | 0084D7B9 | 0084D7B9 | 589 | 602 | docs/SHIP_AI_OPEN_ITEMS.md |
| 008509F0 | `FUN_008509f0` | 0085122D | 0085122D | 2092 | 2101 | docs/GUNNERY_OPEN_ITEMS.md |
| 00853630 | `BSP_SubmarineUnit_SEntityInit` | 00853E0F | 00853E0F | 2007 | 2016 | docs/GUNNERY_OPEN_ITEMS.md |
| 00864D90 | `BSP_UnitGunneryVisibility_Test` | 00864FC9 | 00864FC9 | 567 | 570 | docs/GUNNERY_CANDIDATE_ORDER.md, docs/GUNNERY_OPEN_ITEMS.md, docs/GUNNERY_TABLES.md, docs/SHIP_AI_APPROACH_SLOT_SCORERS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 00882AC0 | `BSP_Landscape_LoadTerrain` | 00883BA5 | 00883BA5 | 4314 | 4326 | docs/GUNNERY_OPEN_ITEMS.md |
| 0088FFD0 | `FUN_0088ffd0` | 008901BC | 008901BC | 472 | 493 | docs/SHIP_AI_FORMATION.md |
| 008CDD60 | `BSP_LuaObjectives_AddUnit` | 008CE50D | 008CE50D | 1963 | 1966 | docs/SHIP_AI_OPEN_ITEMS.md |
| 008CE510 | `BSP_LuaObjectives_RemoveUnit` | 008CECBD | 008CECBD | 1963 | 1966 | docs/SHIP_AI_OPEN_ITEMS.md |
| 008ED9C0 | `FUN_008ed9c0` | 008EDA39 | 008EDA43 | 112 | 132 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00929800 | `BSP_MissionEntity_PublishLuaDead` | 00929B5D | 00929B5D | 811 | 862 | docs/SQUADRON_LAND_TASK.md |
| 0092AAE0 | `BSP_TickableGameEntity_InstallCollision` | 0092B344 | 0092B344 | 2143 | 2149 | docs/GUNNERY_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00934150 | `BSP_UnitParts_DetachPart` | 0093553A | 0093553A | 5076 | 5079 | docs/GUNNERY_OPEN_ITEMS.md |
| 00935540 | `BSP_UnitInstance_SpawnSinkingWreckDebris` | 00935C68 | 00935C68 | 1828 | 1831 | docs/GUNNERY_OPEN_ITEMS.md |
| 00935D30 | `FUN_00935d30` | 00936DBE | 00936DBE | 4236 | 4239 | docs/GUNNERY_OPEN_ITEMS.md |
| 0094B600 | `FUN_0094b600` | 0094BFED | 0094BFED | 2533 | 2536 | docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 0094BFF0 | `FUN_0094bff0` | 0094C471 | 0094C471 | 1151 | 1154 | docs/GUNNERY_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 0094C900 | `FUN_0094c900` | 0094CA06 | 0094CA06 | 254 | 263 | docs/SQUADRON_LAND_TASK.md |
| 00955EB0 | `BSP_GunCategoryList_Clear` | 00955EF5 | 00955EF5 | 62 | 70 | docs/GUNNERY_TABLES.md |
| 0095DE00 | `FUN_0095de00` | 0095E5AE | 0095E5AE | 1964 | 1967 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00963600 | `CG_vector_deleting_dtor_00963600` | 00963863 | 00963AF1 | 591 | 1266 | docs/SHIP_AI_CLASS_FIELD_0524.md |
| 00980E50 | `BSP_WarningManager_DispatchReconChannel` | 00981394 | 00981394 | 1326 | 1349 | docs/SHIP_AI_OPEN_ITEMS.md |
| 009813A0 | `BSP_WarningManager_ReportLoss` | 00982110 | 00982110 | 3391 | 3441 | docs/GUNNERY_OPEN_ITEMS.md |
| 00982120 | `FUN_00982120` | 0098253A | 0098253A | 1022 | 1051 | docs/SQUADRON_LAND_TASK.md |
| 00982C50 | `BSP_WarningManager_FireFailure` | 009832EB | 009832EB | 1669 | 1692 | docs/GUNNERY_OPEN_ITEMS.md |
| 00984300 | `BSP_MissionEvents_ReportCommand` | 009847FD | 009847FD | 1253 | 1278 | docs/SHIP_AI_STATE_STEPS.md |
| 00984EB0 | `FUN_00984eb0` | 00985243 | 00985243 | 888 | 913 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00986820 | `BSP_MissionEvents_ReportShipLanded` | 00986AF8 | 00986AF8 | 706 | 729 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00988510 | `FUN_00988510` | 009899BB | 009899BB | 5248 | 5286 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 009B2E30 | `CG_scalar_deleting_dtor_009b2e30` | 009B2E4D | 009B2E4D | 27 | 30 | docs/SQUADRON_LAND_TASK.md |
| 009B4100 | `CG_scalar_deleting_dtor_009b4100` | 009B411D | 009B411D | 27 | 30 | docs/SHIP_AI_OPEN_ITEMS.md |
| 009B4D00 | `BSP_BotStateLevelBombAttackRun_Tick` | 009B55B5 | 009B55B5 | 2224 | 2230 | docs/SQUADRON_LAND_TASK.md |
| 009B6D20 | `BSP_BotStateLevelBombMoveTo_Tick` | 009B747F | 009B747F | 1882 | 1888 | docs/SQUADRON_LAND_TASK.md |
| 009CA200 | `FUN_009ca200` | 009CA281 | 009CA281 | 127 | 130 | docs/SQUADRON_LAND_TASK.md |
| 009CC000 | `CG_scalar_deleting_dtor_009cc000` | 009CC01D | 009CC01D | 27 | 30 | docs/SQUADRON_LAND_TASK.md |
| 009DBEF0 | `CG_scalar_deleting_dtor_009dbef0` | 009DBF3D | 009DBF3D | 75 | 78 | docs/SHIP_AI_PATH_FOLLOWER.md |
| 009E46D0 | `CG_scalar_deleting_dtor_009e46d0` | 009E46ED | 009E46ED | 27 | 30 | docs/SHIP_AI_NAV_BLOCK_CTOR.md |
| 009F30F0 | `BSP_ShipAi_ApproachNestedReseed` | 009F3212 | 009F3212 | 278 | 291 | docs/SHIP_AI_OPEN_ITEMS.md, docs/SHIP_AI_RING_SCAN.md |
| 009F6060 | `FUN_009f6060` | 009F6089 | 009F6089 | 39 | 42 | docs/SHIP_AI_AVOIDANCE_REQUEST.md, docs/SHIP_AI_NAVIGATION_ARM_TAIL.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 00A07C10 | `FUN_00a07c10` | 00A07D32 | 00A07D32 | 287 | 290 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A08460 | `BSP_Ai_TargetWeight` | 00A0980E | 00A0980E | 5029 | 5032 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md, docs/SHIP_AI_SETTINGS_BLOCK.md, docs/SQUADRON_LAND_TASK.md |
| 00A0C650 | `BSP_AiGroup_ComposeAttackValue` | 00A0D1C3 | 00A0D1C3 | 2912 | 2929 | docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00A0D1D0 | `FUN_00a0d1d0` | 00A0DC07 | 00A0DC07 | 2612 | 2615 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A0F680 | `FUN_00a0f680` | 00A0F7C0 | 00A0F7C0 | 315 | 321 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 00A0F970 | `BSP_AiGroup_TargetValueAgainstGroup` | 00A0FA68 | 00A0FA68 | 243 | 249 | docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00A11070 | `FUN_00a11070` | 00A113C1 | 00A113C1 | 835 | 850 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A13B60 | `BSP_AiCommand_CloseAttackTargetPass` | 00A14D9F | 00A14DC1 | 4659 | 4706 | docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00A16B60 | `FUN_00a16b60` | 00A16B9B | 00A16BA7 | 49 | 72 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A179E0 | `BSP_AiPartyBrain_EngagementPass` | 00A18195 | 00A18195 | 1878 | 1968 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md |
| 00A21D90 | `FUN_00a21d90` | 00A21EB8 | 00A21EB8 | 294 | 297 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A236F0 | `FUN_00a236f0` | 00A2397E | 00A2397E | 652 | 655 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A250A0 | `FUN_00a250a0` | 00A25167 | 00A25167 | 194 | 200 | docs/GUNNERY_OPEN_ITEMS.md, docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00A25A30 | `FUN_00a25a30` | 00A25B87 | 00A25B87 | 313 | 344 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A25B90 | `FUN_00a25b90` | 00A25C66 | 00A25C66 | 212 | 215 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A287C0 | `FUN_00a287c0` | 00A28944 | 00A28944 | 320 | 389 | docs/SQUADRON_LAND_TASK.md |
| 00A29FD0 | `BSP_AiPlanner_CaptureThink` | 00A2B7EB | 00A2B875 | 5779 | 6300 | docs/SHIP_AI_OPEN_ITEMS.md, docs/SQUADRON_LAND_TASK.md |
| 00A2D8C0 | `CG_scalar_deleting_dtor_00a2d8c0` | 00A2D8DD | 00A2D8DD | 27 | 30 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A2E260 | `BSP_AiGroup_SplitDetachedMembers` | 00A2E49E | 00A2E4B8 | 562 | 601 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A2E4C0 | `FUN_00a2e4c0` | 00A2E6FE | 00A2E718 | 562 | 601 | docs/SHIP_AI_OPEN_ITEMS.md |
| 00A32500 | `BSP_Ai_ForcedTargetWeightInsert` | 00A327F5 | 00A327F5 | 749 | 755 | docs/SQUADRON_LAND_TASK.md |
| 00A38DA0 | `FUN_00a38da0` | 00A39041 | 00A39041 | 659 | 666 | docs/SHIP_AI_OPEN_ITEMS.md |

## Spot checks

- 00B8D660 `Mit_PoolAllocator_480_32_Trim`:
  - Its body went from 00B8D660..00B8D66C + 00B8D670..00B8D685 + 00B8D6C0..00B8D6FF (99 bytes) to
    00B8D660..00B8D66C + 00B8D670..00B8D6FF (157 bytes).
  - The decompile now shows the loop. After `_free(block)`, it moves `blocks[count-1]` into slot i,
    decrements the count, and if i < count it writes i into the moved block's 32 slot records (+1E0h,
    stride 1E4h). Then it decrements i. Afterwards it sets current = -1 and scans for the first block
    with a nonzero count.
  - This matches `MitBlockPool::trim` in src/post_effects_cc13.cpp:456. The C++ comment records the
    renumbering, and its self-assignment when i is the last block corresponds to the image skipping
    the renumber when i == count.
  - The plate comment still says "The Ghidra listing lacks 00B8D686..00B8D6BF". It was preserved as
    an existing comment and is now historical.
- 0062C2C0 `BSP_RewardsScreen_Enter`: both `_free` sites, 0063169B and 00632A54, are OK and inside
  the body (00636D78 end, 43678 bytes, the same as after cc13-rewards1's repair).
- The exports of the 2029 affected functions that already had an export folder were re-exported with
  `ghidra_export.py decompile --force` (2029 exported, 0 failed). `bsp.py snapshot --force`
  refreshes the inventory and call graph only. Spot check: exports/bsp/functions/00b8d660/assembly.txt
  now lists 00B8D686 `MOV EDX,[ESI+28h]` after the call, and decompiled.c has the +1E0h renumber loop.

Validation level: Ghidra listing state, verified live by census and flow metadata. There are no source
or behaviour changes.
