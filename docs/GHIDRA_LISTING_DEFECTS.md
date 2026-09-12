# Ghidra listing defects the bridge cannot repair

Addresses: 004e4a40, 00643c0c, 004c9800

The GhidraMCP bridge offers `disassemble_bytes`, `create_function`, `delete_function` and
`clear_instruction_flow_override`, but no way to clear code units, clear a fall-through override or
set a function body. Three defects survive every bridge-side attempt and are repaired by
`tools/ghidra_scripts/RepairListingDefects.java`, run from Ghidra's Script Manager (see the header of
that file). Until it runs, `python tools/bsp.py show`/`ghidra decompile` on these routines is
incomplete and `tools/verify_report_calls.py` reports call sites inside them as "in no Ghidra
function"; read them from the disk bytes (`disasm-raw`) and the docs that already did.

| Start | Inclusive end | Defect | Evidence |
| --- | --- | --- | --- |
| 004e4a40 | 004e5537 | `BSP_Game_OnMove` (GGame::OnMove) has an 8-byte body: flow stops after `MOV EAX,FS:[0]` at 004e4a42 although 004e4a48 disassembles and no flow override is set (`reports/onmove_function_repair.json`: deleted and re-created twice, body unchanged). The routine runs to the `ret 4` at 004e5535 (745 instructions, two RETs, docs/GAME_ON_MOVE_MAP.md). | `python tools/bsp.py ghidra proto 004e4a40 --brief` -> `body 004e4a40 - 004e4a47` |
| 00643c0c | 00643c18 | one-byte-late decode inside `BSP_InGameHudMarkersScreen_Update` 006435d0 (`TEST AL,0xd4` at 00643c0d instead of the instruction at 00643c0c); re-disassembly did not replace the code units (`reports/hud_markers_listing_resync.json`). | `python tools/bsp.py ghidra proto 00643c20 --brief` -> no function |
| 004c9800 | 004c981d | `BSP_SceneRecordPlayerSlot_Construct` sits under a defined data item, so `create_function` refuses ("entryPoint may not be created on defined data"); the bytes are code (`push esi; push 004c6760; push 004c6750; ...`). | `reports/scene_records_function_definitions.json` |
| 004ceca1 | 004cecab | eleven bytes Ghidra skips inside `BSP_NativeStringSet_EraseSubtree` 004cec60 (`83 c4 04 80 7e 15 00 8b fe 74 c5`: the erase loop's back edge, `ADD ESP,4 / CMP byte [ESI+15h],0 / MOV EDI,ESI / JZ`), found by packet cc2_mission_load_hosts; an unnamed row for the repair script |
| 00643c1c | 00643c68 | a hole that stayed inside `BSP_InGameHudMarkersScreen_Update` 006435d0 after the first repair: the body is 006435d0-00643d96 but the units 00643c1c..00643c68 (the target-group member loop of docs/HUD_CENTRAL_UPDATES.md step 13) are not instructions; found by packet cc_hud_minimap (docs/HUD_MARKERS_RUNTIME.md). Added to the script's RANGES; re-run the script. | `python tools/bsp.py ghidra flow 006435d0` reports no gap because the hole is not after a CALL |

After the script runs and the program is saved: `python tools/bsp.py snapshot --force`, `python
tools/bsp.py index`, then `python tools/ghidra_flow_repair.py 004e4a40 006435d0` to close any
`_free` gaps the fresh disassembly exposes, and `python tools/ghidra_annotate.py --apply
--addresses 004c9800` to apply the deferred name. Add new rows here and in the script's `RANGES`
table when a bridge-side define fails for the same reasons.

## Repair record

Run once on 2026-09-11 in Ghidra 12.0.4 from the Script Manager (after the import fix in
commit 67323d93: `FlowOverride` is `ghidra.program.model.listing.FlowOverride`). Console output:
`004e4a40: BSP_Game_OnMove body 004e4a40 - 004e5537`, `00643c0c: re-bodied
BSP_InGameHudMarkersScreen_Update to 006435d0 - 00643d96`, `004c9800:
BSP_SceneRecordPlayerSlot_Construct body 004c9800 - 004c981d`; program saved, then
`snapshot --force`, `index`, `ghidra_flow_repair.py 004e4a40 006435d0` (three gaps after
unconditional `JMP`s in 004e4a40, padding, left alone; none in 006435d0) and
`ghidra_annotate.py --apply --addresses 004c9800`. `verify_report_calls.py
reports/game_executable_milestone_2f.json` went from 51 failures to 96 rows checked, 0 failed.
The three rows above are repaired; the table stays as the record of what the bridge cannot do.

### Second run, 2026-09-12

Run again from the Script Manager after the `00643c1c..00643c68` row was added: the hole inside
`BSP_InGameHudMarkersScreen_Update` now decodes (28 instructions, the target-group member loop,
`00643C1C: JLE 00643C69` onward), the body stays `006435d0-00643d96`,
`ghidra_flow_repair.py 006435d0` reports 0 gaps, and `snapshot --force` / `index` were refreshed
(62924 functions). All four rows are repaired.

## Repair run 3 (2026-09-12, RepairListingDefects.java, Ghidra 12.0.4)

- `004ceca1..004cecab`: repaired. `BSP_NativeStringSet_EraseSubtree` now owns `004cec60 - 004cecb1`;
  the hole decodes as `ADD ESP,4 / CMP byte ptr [ESI+15h],0 / MOV EDI,ESI / JZ 004cec71` (the erase
  loop back edge). `tools/ghidra_flow_repair.py 004cec60` reports 34 instructions, 0 gaps.
- `00643c1c..00643c68`: NOT repaired. The bytes were disassembled but the script printed "no function
  owns the range": `BSP_InGameHudMarkersScreen_Update` owns `00643c18` (the four-byte
  `MOV [ESP+34h],EAX`) and `00643c69` (the `JLE` target) but not `00643c19..00643c68`, so the
  one-byte-back anchor at `00643c1b` found no owner. The script now walks back up to 64 bytes to
  the nearest owned address, adds the range from the byte after it through the end of the last
  decoded instruction, and skips rows an earlier run already repaired. One more run is queued.

## Repair run 4 (2026-09-12, RepairListingDefects.java, Ghidra 12.0.4)

- `00643c1c..00643c68`: repaired. The backward-walking anchor found `BSP_InGameHudMarkersScreen_Update`
  at `00643c18` and the body now owns `00643c19..00643c68` as well (`00643c19`, `00643c1c` and
  `00643c68` all resolve to the function); `tools/ghidra_flow_repair.py 006435d0` reports 565
  instructions, 0 gaps. The other four rows printed "already repaired" and were left untouched.
  No listing defects remain queued.

## Repair run 5 (2026-09-12, RepairListingDefects.java, Ghidra 12.0.4)

- `0081f56c..0081f8ad`: repaired. `BSP_UnitVehicleBase_Destruct` now owns `0081f3a0 - 0081f8ad`
  (392 instructions, 0 flow gaps), so the level-5 release steps 11 to 23 of
  docs/UNIT_DESTRUCTOR_LEVELS.md are inside the stored body.
- `0077e442..0077e490`: repaired. `BSP_UnitOwnerEntity_Destruct` owns `0077e380 - 0077e490`
  (67 instructions, 0 gaps).
- `004dd123..004dd5a6`: repaired. `CG_vector_deleting_dtor_004dcf90`, the game destructor, owns
  `004dcf90 - 004dd5a6` (464 instructions, 0 gaps), the tail docs/GAMEPLAY_LOOSE_ENDS_1.md found.
- The five earlier rows printed "already repaired" and were left untouched. No listing defects
  remain queued.
