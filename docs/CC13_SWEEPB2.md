# cc13-sweepB2: continuation of the segment 15-51 leftover sweep

Packet `cc13-sweepB2`, Claude worker in the cc13 lineage, 2026-10-16. It continues
docs/CC13_SWEEPB.md: the pool is that document's "Not reached in this pass" list, taken in the
order the lead gave (the seg 43, 45/46, 20, 26, 29 and 51 short list first, then the rest smallest
first). Each function is closed with one outcome: reconstructed, existing implementation recorded,
unreferenced recorded, cc9 lane (skipped again, no record), or skipped with a reason.

Names are hypotheses, not recovered symbols. Files: `include/bsp/cc13_sweepB2.hpp`,
`src/cc13_sweepB2.cpp` (registered in `cmake/startup.cmake` on bsp_core), this document. The host
interface `Cc13SweepB2Host` extends sweepB's `Cc13SweepBHost`, so sweepB's hooks (image globals,
CRT, native strings, sized pool, render and camera calls) are reused as they are. No host file was
edited, nothing is wired and no game was run. Ghidra was read only.

Method, per function: `bsp.py lookup` / `docs-for` and `git grep` of src/ and include/ for the
address; earlier docs checked for a cc9 assignment; bodies read from the disk bytes
(`bsp.py disasm-raw`), with every x87 register-register operand direction taken from
`tools/x87_dc_form_scan.py` (bytes), never from listing text; constants read from the image with
their width. Stack frames were mapped by hand from the ESP offsets (argument-slot reuse is noted
where the image stores into an argument slot). EH frames are noted but unwinding is not modelled.

x87 note: as in sweepB, float expressions keep the image's operation order with a float store
wherever the image stores to a float slot; exact at the main thread's 24-bit precision control.

## Reconstructed in src/cc13_sweepB2.cpp

Build-tested only (compiled in the bsp_core build); none is ABI-compatible, fixture-tested or
game-validated, and nothing is wired. Every call the image makes is a named host method at the
same point with the same arguments.

| Seg | Address | Name (hypothesis) | Original ABI | What the code does, and simplifications |
|---|---|---|---|---|
| 18 | 0056B610 | BSP_MultiSessionBrowserScreen_MoveSelection | __thiscall(screen)(scroller, listbox, clip, keep) RET 10h | Marks each list row fully inside the clip (row vslot 60h(1/0)) and records the first and last such rows at +2C4h/+2C8h; then either scrolls toward the selection (00683680 / 00683700 by the selected row's height) or, when the scroller is busy or keep is set, re-selects the crossed bound (00A9C8F0, +2CCh). |
| 19 | 005787E0 | BSP_MultiSessionBrowserScreen_OnListboxSelectionChanged | __thiscall(listener = screen+8h)(unused, list) RET 8 | Hidden list: 0056AA70. Otherwise (unless +E9h) sets the scroll arrows of the nine rows (state 2 on the selected row) through child vslot 88h; when the list is the session list: +260h = 10.0f, 00577650, 0056B610. |
| 20 | 00588A80 | BSP_MainMenu_ActivateTopLevelItem | __thiscall(screen) RET | Caches the list-box selection in 00E194C4 and dispatches: single-player / multiplayer pages, options (two state requests 6 and 14h), tactical library (Lua `luaLoadControlFunctionNames()` then its page), XShowGuideUI, XShowMarketplaceUI behind two gates, and the quit-confirm prompt (00531B00 with two named strings and an owned by-value string, then 00530C20). The body behind include/bsp/main_menu_screen.hpp's item table. |
| 20 | 005AAEF0 | BSP_InGameInterfaceScreen_Construct_005aaef0 | __thiscall(screen) RET, EAX | Base constructors, the four vtable words, three inline tree constructors and one list (head nodes from the STL allocators, called as hooks), the field zeroing, +548h = -1, +5E4h = 0.001f, +5DCh = 2.0f. |
| 20 | 005A4020 | BSP_HudMapScreen_SetNodeVisibleByName | __thiscall(screen)(name by value, show) RET 0Ch | For every node in the vectors +500h and +510h, passes a copy of the name and 1.0/0.0 to sweepB's 005A3EF0 (called directly); checked-iterator tests kept; the name is released at the end. |
| 20 | 005C51F0 | BSP_MissionSideBlock_CopyConstruct | __thiscall(dest)(source) RET 4, EAX | The copy-constructor twin of 005C5470 (string vectors through 00557BC0). |
| 20 | 005C5470 | BSP_MissionRecordHeader_Assign | __thiscall(dest)(source) RET 4, EAX | Member-wise copy: byte, native string (self-assignment guarded as in the image), two dwords, twenty string vectors through 00506C80. |
| 20 | 005C5860 | BSP_MissionTree_ReadStringVector | __thiscall(reader)(kind, key, out) RET 0Ch | Empties out (checked-iterator tests kept), enters the key, appends the strings at integer keys 1, 2, ... until one is missing, leaves the table. |
| 20 | 005C5990 | BSP_MissionTreeScreen_CollectMissingPrerequisites | __thiscall(screen)(name) RET 4 | Worklist from name over the mission tree: every prerequisite is queued, and the unsatisfied ones (007FC4C0) are inserted into the profile set at game+650h+7Ch; the local vector is destroyed with 00432050 and freed. |
| 20 | 0059D7B0 | BSP_InterfaceView_DrawGroundOrtho | __thiscall(view) RET, tail JMP 0093D250 | Look-at from (+54Ch, 10000, +550h) down to (+54Ch, 0, +550h), up (0,0,1), into camera +10h; ortho (-w/2, w/2, -h/2, h/2, 1, 15000); shadow box min (x-w, 0, z-h) / max (x+w, 4500, z+h) to 00A8E860(00B7AAB0([+18h]), [+10h], &min, &max); then 0093D250(view+8h). |
| 21 | 005CCE70 | BSP_HudEngineMovieScreen_Register | __thiscall(screen) RET | 004F71D0, loads `GUI_movie`, binds `up_Icon` / `down_Icon` (each name a temporary native string), +1Ch = 0.14f or 0.18f by [[0109CF04]+0Dh]. |
| 22 | 005D20A0 | BSP_MultiErrorScreen_LayoutButtons | __thiscall(screen) RET | Measures the shown buttons (+114h / 960), spreads them across the +2Ch widget's width with equal gaps (00AA78D0 per shown button). |
| 23 | 005EC250 | BSP_MultiModeSelectorScreen_RefreshMapList | __thiscall(screen) RET | In state 3: resets the scene record when out of range (004C6890), clears and hides the map list, commits the other screen ([00E19698]), rebuilds the command bar with five empty labels (0054B530, 15 dwords), and copies the four selector bytes into the lobby block game+1FF0h (+2Ch/+2Dh one-based). |
| 26 | 00638C60 | BSP_HudMarker_ProjectOffscreenArrow | __fastcall(world, x*)(y*, angle*) RET 8 | View-space point via 00B6FCB0 / 004142E0; vy = v.y*4/3; angle = atan2(vy, v.x); margins from 00E197C8 and 00E197D0/00E197D8 (by [[0109CF04]+0Dh]); side-edge or top/bottom-edge placement as the two branches of the image. |
| 26 | 00640A20 | BSP_InGameHudMarkers_UpdateLeadMarker | __thiscall(markers)(target) RET 4 | For an enemy target in range of the controlled gun unit, places a lead marker at the projected 00901C20 intercept point (marker pool +90h / +A0h, created with 0063E950 when needed) and fades it by the screen distance to the target (d2 * 384, capped at 1). The unit's vslot 34h is its velocity getter (RET 4); this resolves the "vtable +34h outputs" left open by docs/CC13_FE_10.md. |
| 26 | 00642960 | BSP_InGameHudMarkers_AddSquadronMemberMarkers | __thiscall(markers)(squadron, flag, a3, a4, a5) RET 14h | Unless the squadron lead is at (or within 1e-10 squared of) the controlled unit, builds a marker per member through 00642040: emphasised for flight leaders (flag set) or the selected member (flag clear). |
| 26 | 00643360 | BSP_InGameHudMarkers_AddObjectiveMarker | __thiscall(markers)(x, y, z by value, objective, record) RET 14h | Needs 004B4B00; stack hit record reset/destroyed; altitude test zeroes the copy's y; on screen: unit -> 006430C0(unit, 0, 0, 2), else 00639230(00640860(markers), x+0.002, y, +2Ch, objective, record, 1, +1Ch); off screen when objective+18h == 0: 00638C60 then 00639230(..., 0, +1Ch). |
| 26 | 0064B170 | BSP_HudDamageDirection_Register | __thiscall(widget)(point) RET 4 | Camera-local direction normalised (0042B260); pitch = 0042CF10(clamped y), yaw = atan2(x, z); the larger of the two (by magnitude) is tested against +-fov/2 (camera +1C4h) and sets one of +10h/+14h (pitch) or +1Ch/+18h (yaw) to 1.5f. |
| 26 | 0064B370 | BSP_HudShipScreen45_Activate | __fastcall(screen) RET | Resets the pipe-sight globals, shows/hides the screen's widgets by unit kind, builds the three 1Ch-byte digit gauges (0043DF90), resets the screen fields and applies the unit (0064AE20). |
| 26 | 00651370 | BSP_HudUnitView_EnsureSubmarineCamera | __thiscall(view) RET | Creates the 410h-byte submarine camera once (00BF55BE, memset, 00519290, four vtables), places it (00923870, identity), pitch limits from settings +458h/+45Ch in radians, 00694A60(EDX view+8h); then, unless already bound to this unit, 0042F6C0 setup, distance seeding from [[unit+538h]+A0h] when +3E8h is clear, 004BC410, +28h = +24h, +78h = 0. No null test after a failed allocation, as in the image. |
| 29 | 006AC030 | BSP_InputSettings_LoadForCurrentProfile | __thiscall(settings) RET | Reads `DefaultPreset` as a string, looks it up in `ControlPresets`, reads that table through a LuaReader into 006ABA50, then tears the temporaries down in the image's order. The by-value LuaObject is built in a local and handed to the 004425C0 hook (simplification). |
| 29 | 006AD600 | BSP_Game_ClearMissionLuaGlobals | no arguments, RET | Sets five Lua globals to nil (00B67350 on the globals table of game+1A0Ch): `scoring`, `thisTable`, then the three names of the .data table 00E08DF0. docs/CC13_SETTINGS_SAVEDATA.md calls it reconstructed in src/mission_load_path.cpp, but that file only lists it in a hook table. |
| 29 | 006AF780 | BSP_SceneEntity_RestoreFromSaveData | __thiscall(unused)(entity) RET 4 | Reads `parentID` (into storage that starts as the entity pointer, as the image's argument slot does) and `localFrame` from the entity's saved table and calls its vslot 98h; the only caller passes null (docs/CC13_SAVE_1.md), so the image faults there; so does this code. |
| 43 | 0076F0E0 | BSP_Session_RefreshFromOnlineSession | __thiscall(session) RET | Player name to +100h (20h bytes) from the online session's first member, else the Xenon selected user, else the game name string; then the fixed field resets (+127h = 8, +12Ch = 14h, +12Dh = 1, others 0). |
| 43 | 007700E0 | BSP_Session_UpdateClientClockScale | __thiscall(session) RET | Frame-time scale 00E0B6C8: 1.0 out of play; a client with a server peer maps the tick-rate difference through 00419010(1.5, 1.0, 3.0, 1.1, \|diff\|) and takes the reciprocal when behind. |
| 43 | 00770E50 | BSP_WideString_Substring | __thiscall(source)(out, pos, count) RET 0Ch | Negative pos counts back; clamps; builds a local wide string (004C53E0 + wcsncpy), copy-constructs out (004C8DD0) and returns the local buffer to the sized pool. |
| 43 | 00775160 | BSP_SessionHostInfo_BuildBlob | __thiscall(owner)(void**, u32*) RET 8 | Allocates and fills the 98h-byte host-info blob (name, 4-char tag, game fields, rank 00574050, Xenon fields, language 008D4870, the +18Ch+C8h object's vslot 20h fill, byte from [00F8A2FC]+4Ch); size 98h. |
| 43 | 00776230 | BSP_PeerManager_PumpMessageQueues | __fastcall(session) RET | Drains the added-peer queue (three session handlers per peer) and the removed-peer queue (host remove, then on-removed) under their locks, unlinking and freeing each node; between them, every 0.25 s of the performance timer, the server kicks timed-out peers and the client checks server silence (8.0 s). Unreachable checked-iterator calls are not reproduced; the timer quotient is formed in double. |
| 43 | 00784B10 | BSP_SessionTransport_KickPeer | __thiscall(transport)(peer, flag) RET 8 | Finds the peer in the list; MT_KICKED (14h, vtable 00D03174) serialised under the peer's lock, flushed (00783490), then the raw FADEBABEh word through vslot 2Ch. Unreachable checked-iterator calls in the search loop are not reproduced. |
| 45 | 0078DBA0 | BSP_OceanEnvironment_InitWaterParticles | __thiscall(env)(source, settings) RET 8 | Builds and frees the unused error string; deletes the old +B0h; constructs the 11Ch-byte particles with a by-value reference from source+8h (interlocked count kept balanced); optional 0078D940(settings); 00BA44F0. |
| 45 | 0078DCF0 | BSP_OceanEnvironment_InitBeams | __thiscall(env)(source, settings) RET 8 | As 0078DBA0 with two references (env+A8h's +0Ch and source+8h) passed to 00BA3A60, released source first; optional 0078D9F0; 00BA3BD0. |
| 45 | 00795D90 | MovieCameraKeyframe_ConvertAnchorSpace | __thiscall(key)(new_mode) RET 4 | Re-expresses +4Ch in the entity's derived inverse (old mode 0) or as an offset from the world translation (old 1-3 to 0), copies the result into both point slots and +58h..+60h into +64h..+6Ch; +2Ch = new mode unless an early return. |
| 46 | 007A4EF0 | BSP_MovCam_SeedAnglesFromVehicleCamera | __thiscall(camera)(target) RET 4 | Once per target: records the first vehicle camera whose +58h byte is clear and fills the per-target angle pairs with the negated basis angles (0042D2E0) of each camera's matrix (one {0, 0} pair when there is none). |
| 46 | 007A49A0 | BSP_MovCam_PickRandomDeckCamera | __thiscall(camera)(target, look_target, key) RET 0Ch | Scores each entry of the keyed camera list by a 00BD2F10(0, 1) draw, plus (with a look target) 42 when inside minus the distance; returns the best entry or -1. Same RNG note as 007A8C60. |
| 46 | 007A6710 | BSP_MovCam_EnterVehicleCamera | __thiscall(camera)(index) RET 4 | Seeds angles (007A4EF0), grows the vehicle-camera array as needed, copies a translation-only matrix to +420h, reads the stored angle pair (checked), records the index in the per-target map, shows the mode text (by-value string to 005CC5E0), +460h = 1.0f, (camera+170h) vslot 0Ch. |
| 46 | 007A7C60 | BSP_MovCam_ReceivePathPoints | __thiscall(camera)(message) RET 4 | Clears the path records, resizes +478h to 3, builds the 4Ch path record from the message and stores it in slot 2 (checked), then 007A5A00 on 005CD160's camera with the message fields and an empty local string. No null test after a failed allocation, as in the image. |
| 46 | 007A8C60 | BSP_MovCam_PickDeckPointScored | __thiscall(camera)(entity) RET 4 | Scores each vehicle camera by 42 (inside) minus distance plus a 00BD2F10(0, 1) draw; enters the best one through 007A6710 and returns its index (-1 when none). The inside byte is kept in one variable seeded with the entity pointer's low byte, as the image keeps it in the argument slot. RNG: 00BD2F10 is the shared stream; this is presentation and needs its own stream or an OFF switch if wired. |

## Unreferenced, recorded (ledger only)

- 005D63E0 BSP_GuiWidget_DrawBoundsOutline_005d63e0 (seg 22): 88 instructions, the body of 00554680
  (recorded unreferenced by sweepB). tools/callsite_census.py: 0 rel32 / absolute references;
  positive control 00554600 = 5. Not reconstructed (over the ~80-instruction limit).

## cc9 lane (skipped again, no record)

- 0077B280, 0077C160, 0077C540 (seg 43): inside the cc9 unit/formation range 00778820..0077D270.
- 0077D3F0, 0077D7F0, 0077F7B0, 0077F5E0, 0077FAD0 (seg 43): docs/CC13_GP_0071_0078.md "Names for cc9".
- 0077FE80, 00780DB0 (seg 43): docs/CC13_CAM_2.md "Gameplay state, left alone" (entity
  replication; multiplayer scoring).
- 005B83E0, 005B8810 (seg 20): 005B5C00..005BCFFF is cc9's per docs/CC13_FRONTEND_MENUS.md and
  docs/CC13_NG2.md. (005B8810 has no rel32/absolute callers; 005B83E0 has one caller, 0096BF70.)
- 006AC370, 006ACB40, 006ACDC0 (seg 29): landing-pad / landing-ship helpers left to cc9 in
  docs/CC13_SEG29.md; 006AC370 is also recorded by src/game_hosts_ship_ai.cpp.
- 00641E30 (seg 26): docs/CC13_BAND_0060_0066.md "Gameplay and cc9 areas left alone".
- 005F98F0 (seg 23): port-range test also called by BSP_AiCommand_Selling_Tick; docs/CC13_SEG23.md
  "belongs to cc9".
- 00605630 (seg 24): docs/CC13_FE_5.md "For cc9 (gameplay)".
- 0081B630, 0081B7C0, 0081BA80, 0081BFE0, 0081C1F0, 0081C550 (seg 51): docs/CC13_GP_0080_0084.md
  "For cc9". 0081C1F0 has no callers (callsite_census: 0 rel32 / absolute; positive control
  0081C980 found 7).

## Skipped with a reason (no record)

- 0059D4A0 BSP_HudMapIcon_PlaceWidget (seg 20): for anchor modes other than 0 and 1 the position
  local is never written before it is passed to 00AA8240 (docs/CC13_SEG20.md); no faithful C++.

## Not reached in this pass (no record)

The rest of docs/CC13_SWEEPB.md's list, minus everything above. Instruction counts as there.

- seg 15: 00528240 (553), 00529D70 (277), 0052A100 (271), 0052A470 (281), 0052A860 (215), 0052B080 (273), 0052BB10 (347), 0052C1B0 (368), 00534870 (241), 00536310 (382), 00536A90 (989), 00538F80 (1560), 0053BCC0 (501), 0053EE00 (310), 0053F2D0 (864)
- seg 16: 00547480 (265), 00547A20 (258)
- seg 18: 0056D660 (234)
- seg 19: 0056D9A0 (237), 0056DD80 (421), 0056F320 (1061), 00570300 (324), 00578580 (198)
- seg 20: 00583E50 (198), 00585C50 (174), 0058CE30 (243), 00593CA0 (355), 00597870 (1081), 005987F0 (259), 005A1C60 (435), 005A2340 (251), 005A2730 (399), 005A2C30 (429), 005A3210 (257), 005A3610 (174), 005A3830 (186), 005A3BF0 (229), 005A4200 (494), 005A6AD0 (734), 005A7550 (315), 005A7C20 (651), 005A8600 (678), 005A8F50 (173), 005A91E0 (405), 005A9770 (345), 005A9DB0 (868), 005AAC00 (239), 005ADDA0 (5027), 005B25D0 (424), 005B2D70 (532), 005B3470 (1316), 005B45C0 (1465), 005BE240 (804), 005C0BD0 (190), 005C4080 (259), 005C4D40 (371)
- seg 21: none left.
- seg 22: 005D18A0 (509), 005D26D0 (618), 005D43C0 (370), 005D73A0 (1155), 005DB050 (4159), 005DF510 (1391)
- seg 23: 005E0DB0 (219), 005E12A0 (359), 005E1CF0 (265), 005E2750 (630), 005E3290 (194), 005E3580 (1024), 005E7B10 (1206), 005E9BE0 (297), 005EA030 (193), 005EADC0 (1138), 005EBDD0 (331), 005EE520 (387), 005F0CD0 (676), 005F24D0 (633), 005F3B50 (421), 005F65C0 (572), 005FA0F0 (723), 006003A0 (1017)
- seg 24: 0060ABD0 (1578)
- seg 25: 006113A0 (244), 00611770 (227), 00614320 (219), 00616C70 (314), 00617470 (255), 006179D0 (1139), 006191A0 (720), 0061B0F0 (576)
- seg 26: 00621C60 (569), 00623710 (631), 00626C50 (275), 00627960 (291), 00637080 (312), 0063C110 (404), 0063C620 (838), 0063D1E0 (863), 00640D70 (615), 00641910 (340), 00642040 (641), 00642C20 (321), 0064AE20 (244), 00650450 (191), 00652C80 (348)
- seg 27: 0065BF00 (259), 0065D220 (657), 0065FBF0 (431), 00660B10 (545), 00661240 (562), 00668770 (261), 006697E0 (258), 0066A8A0 (253), 00670020 (267), 00671D60 (255), 00673A10 (259), 00674B80 (1212), 0067F4D0 (1222)
- seg 28: 0068BC80 (231)
- seg 29: 006B9450 (818)
- seg 31: 006D5220 (910)
- seg 32: 006DE5C0 (354)
- seg 43: 00776930 (1125)
- seg 44: 007884D0 (1179), 00789560 (928), 0078A430 (729)
- seg 46: 007A5110 (547), 007A5A00 (498), 007A7200 (231), 007A74F0 (469), 007A84D0 (225), 007A87B0 (345), 007A8DA0 (315), 007A91A0 (1815), 007ADD70 (252)
- seg 51: 0081E350 (144; cited by cc9's docs/GUNNERY_OPEN_ITEMS.md, check with cc9), 00822C20 (2069), 00824B60 (470)
