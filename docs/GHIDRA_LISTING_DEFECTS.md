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
