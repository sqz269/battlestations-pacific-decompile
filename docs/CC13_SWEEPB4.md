# cc13-sweepB4: fourth pass of the segment 15-51 leftover sweep

Packet `cc13-sweepB4`, Claude worker in the cc13 lineage, 2026-10-16. It continues
docs/CC13_SWEEPB3.md: the pool is that document's "Not reached in this pass" list, taken smallest
first by byte span (the instruction counts printed there undercount several bodies), minus
005AAC00 and 0066A8A0 (inlined std::set/std::list, set aside by sweepB3), 005A3210 and 00617470
(taken last if budget remains), 0081E350, 007A84D0, 0064AE20 (cc9), everything sweepB/B2/B3 listed
as cc9 lane, and 0059D4A0. Each function is closed with one outcome: reconstructed, unreferenced
recorded, deferred (with the reason), or not reached.

Names are hypotheses, not recovered symbols. Files: `include/bsp/cc13_sweepB4.hpp`,
`src/cc13_sweepB4.cpp` (registered in `cmake/startup.cmake` on bsp_core), this document. The host
interface `Cc13SweepB4Host` extends sweepB3's `Cc13SweepB3Host`, so the sweepB/B2/B3 hooks are
reused as they are, and three earlier reconstructions are called directly
(`cc13_sweepB2_movcam_pick_deck_point_scored_007a8c60`, `cc13_sweepB3_session_browser_fill_settings_0056d660`,
and this file's own `cc13_sweepB4_hud_counter_layout_text_pair_0052a470` from the counter tick).
No host file was edited, nothing is wired and no game was run. Ghidra was read only.

Method, per function: `bsp.py lookup` / `docs-for` and `git grep` of src/ and include/ for the
address and the name (none had an implementation; several had host hooks or a recorded host call
only); bodies read from the disk bytes with `local/sweepB4_dis.py` (Capstone; x87 lines carry
their raw bytes and the DC/DE register-register forms are decoded from the encoding, never from
listing text; absolute operands are annotated with their image values); every double constant
was checked against the image (`local/sweepB4_constcheck.py` lists each `f64_bits` literal with
its value); stack frames mapped by hand from the ESP offsets. EH frames are noted but unwinding is
not modelled. Inlined STL code inside a game function (checked-iterator `00BF6713` tests, inlined
push_back / erase(begin, end)) is reproduced; called STL instantiations are host hooks.

x87 note: as in sweepB..B3, float expressions keep the image's operation order with a float store
wherever the image stores to a float slot; intermediate x87 values are kept in double; NaN
behaviour follows the image's branch (FCOMI/COMISS + JA/JB/JBE as written in each row).

RNG: no function here draws from the shared RNG 00BD2F10 itself. 007A8DA0 calls sweepB2's
007A8C60, which does. 005987F0 seeds and draws the CRT generator (srand/_time64/rand), not
00BD2F10.

## Reconstructed in src/cc13_sweepB4.cpp

| Seg | Address | Name (hypothesis) | ABI | What the code does |
|---|---|---|---|---|
| 15 | 00529D70 | BSP_HudCounter_LayoutTimer | fastcall(entry; string a, string b) RET 10h | +9Ch = 1, widget visibility through 00AA6820 and vtable +34h, +88h/+90h = a/b (inline assign: 0041DD40 + memcpy), 00AA78D0(text, 0), texts via 00ABAED0, width = larger text scale + 0.01 to both texts and +2Ch, +30h = width, +54h = 0, the bar fraction and the 00AA6750/00AA8240 move by -0.02; a and b released. |
| 15 | 0052A100 | BSP_HudCounter_LayoutValue | fastcall(entry; string a, string b, int v0, int v1) RET 18h | As above with +10h/+14h vtable +88h(v, 0, 1.0f), the two-text scale floored at 0.1, +30h = +54h + scale, and the +2Ch width (+0Ch width * fraction + 0.035). |
| 15 | 0052A470 | BSP_HudCounter_LayoutTextPair | fastcall(entry; string a, string b) RET 10h | Texts cleared with "" then set from the ARGUMENTS; +88h/+90h copies; when +54h + scale exceeds +30h (JBE, NaN skips) the row grows and re-lays out with -0.019 / +0.073. |
| 15 | 0052A860 | BSP_HudCounter_LayoutCountdown | fastcall(entry; string text, float seconds) RET 0Ch | +9Ch = 2; scale floored at 0.04; +30h = (+54h + scale) + (+54h - +3Ch); +0.013 / +0.035 offsets; +98h = seconds; +A0h = 0 (>= 3600), 1 (>= 60), 2 otherwise and for NaN (JB). |
| 15 | 0052B080 | BSP_HudCounter_Tick | thiscall(entry; float seconds) RET 4 | Mode 2: 00528B00 unless hidden, the bytes +A5h..+A9h clear and 004B6260 false; mode 1: 00426060 copies of +90h then +88h into 0052A470; mode 0: 0052ADD0 drops null records, sums +2Ch*+34h + +30h*+38h (00939F70/80/C0/B0 getters) and +370h values (the kind-18h slot loop dereferences NULL past slot 4, as the image does), then 00528E90(sumB/+84h, sumA/+84h) or hides the row. |
| 15 | 0052BB10 | BSP_HudCounter_LayoutScore | fastcall(entry; const array* units, string text) RET 0Ch | 0052AE60(+70h, 0); per unit a stack 18h observer record (00694A60, 0052AD50, 006952A0, 00695870, vtables 00CE74FC/00CE75CC as values); +84h totals; +10h vtable +88h(target(0)+58h); text or the unit's vtable +14h name (00ABBE50); -0.02 / -0.002 / +0.07 offsets. |
| 15 | 0052C1B0 | BSP_HudCounterEntry_Construct | thiscall(entry; root, names) RET 8, EAX | Zeroes the counter fields, twelve 00AA7E00 lookups (name index order 0,1,3,2,4,5,8,10,9,11,6,7), cached 00AA67F0 / 00AA6740 values; the final vtable +34h call has no pushed argument and gets the caller's saved EDI (exposed as `caller_edi`). |
| 15 | 00534870 | BSP_Hud_UnitOrderIcon | thiscall(screen) RET | Whole body (the chain fragment was recorded before): selected unit (00A9C920, 0070D070), order icon from the class-singleton chain via 0071BE40 (unmatched = 0), name, +BCh/+C0h by unit+54h, kind-5 health bars (00923BE0; kind 6 subtracts (d*b + c*a)/hp with _ftol2 of the four +A20h getters), payload frame 00653200. |
| 15 | 00536310 | BSP_HudFormationScreen_BuildQuadModel | thiscall(screen; float size) RET 4 | The 005A2340 shape: generated model (00B74EB0, 0043C130, 00B75030), pf43.mvfm stream, +-size/2 quad, formation_gui.mshd with cCircleRadiusSquared (00B18B20) and cCirclePosition (00B18B00), 00B179F0 colour {1,1,1,1}, 00B75170, vtable +30h({0, 1400, 0}), 00B6D890. Unchecked reference drops as in the image. |
| 15 | 0053EE00 | BSP_FreeCameraEffect_Update | thiscall(effect; float dt, camera*) RET 8 | Caption timer and text, action 65h cycles the record (004C43C0), fire on action 64h or the input block, repeat timer; on fire a stack entry (0053DC70/0053DCE0) with an optional point effect (0085DC80, 00868420 with the definition passed by value after InterlockedIncrement, 004845D0, 00867790) inserted into the +14h list (0053E890, 0053E930). |
| 19 | 00570300 | BSP_MultiSessionBrowserScreen_PrepareEnter | thiscall(screen) RET | Nine rows cloned (00AAB4C0, 00A9D750), setting_name_Text from 00E086C4, scroll_right/left_Icon listeners (00AA6BC0) and +D8h indices, list-box setup (00A9CEB0, 00A9AC40), 0056D660, template hidden. |
| 20 | 005987F0 | BSP_MainMenuScreen_Enter | thiscall(screen) RET | 0076FAD0, 00518D60(0,0,0,""), 0058D9E0, CRT srand/rand shuffle of the five background movies, movies\FE_bg_0<n>.bik (00AAC870/00AAE320/00AB0EB0), page builder by [00E08874], 00549620, 005F6EF0. |
| 20 | 005A1C60 | BSP_HudMapIcon_ReleaseWidgets | thiscall(icon; char keep_first) RET 4 | Widget vectors +C8h, +D8h, +88h, +78h, +A8h, +98h, +B8h: non-null slots deleted (vtable +4h(1)) and zeroed, then cleared; +E8h deleted (slot 0 kept with keep_first, each slot zeroed twice) and not cleared; +118h = 0 without keep_first. |
| 20 | 005A2730 | BSP_HudMap_CreateConeModel | stdcall(float radius, float angle) RET 8, EAX | ACone generated model: SimpleColor.mvfm vertex stream (4Ah) and index stream (renderer +60h, 00B73B70), guialphawritediff.mshd, centre plus 73 rim points from pi/2 - angle/2 in angle/72 steps, 72 fan triangles, diffuse white with alpha 0.5 (two 00B179F0 calls). |
| 20 | 005A2C30 | BSP_HudMap_CreateSimpleColorModel | stdcall(float radius) RET 4, EAX | ACircle: the same shape with 49h vertices at 5-degree steps (double 00CF0098) and the closing triangle {0, 72, 1}. |
| 20 | 005A7550 | BSP_HudMapArrowWidget_SetWaypointPath | thiscall(arrow; map, start, path, first, curve_arg, texture) RET 18h | Local 12-byte vector of start + path points (004215D0, 0059CAC0), per segment two 0059C9B0 margin tests, curve points in map coordinates, width map+5CCh * 0.014, texture [00E08884 + 4i], 00ABC920/00ABC970/00ABDD80/00ABCA10; unused curves hidden; vector freed. |
| 20 | 005C4D40 | BSP_MissionSideBlock_Destruct | thiscall(block) RET | Twenty native-string vectors destroyed from the last (00432050, _free) and the +4h string released. |
| 22 | 005D43C0 | BSP_HudPauseMultiScreen_OnConfirm | thiscall(screen; widget, arg2) RET 8 | Row-key dispatcher (resume compared inline, the rest through 00425850): 004DB030, 00688AD0 1Fh/16h, options vtable +8h, tips (00601200, 005FD890), surrender / quit / exit prompts through 00531B00 (callbacks 005D36B0 / 005D3410 / 005D3430 passed as addresses; question texts joined with 00425E10, the willendsession line gated by 004BB8A0), notsurrender 00771F20 and 005D39A0. |
| 23 | 005E1CF0 | BSP_MultiGameLobby_StepSetting | thiscall(screen; int step) RET 4 | Map slot (005E1BF0, 0041E350 clears, swap-pair check through 005D4AF0), slot 0Dh (+2F4h wrap by 00427AC0), slot 5 steps backwards and the others forwards over the option registry (00576B10, 00575E80) with the 005E1990/005C6850/005C49C0 availability flag; +10h = 2. |
| 23 | 005E9BE0 | BSP_MultiMainMenuScreen_UpdateSessionList | thiscall(screen) RET | Award-grid cursor: checked list walk, clamp to the last node with floor-rounded column/row (00BF85B0), +F0h smallest-magnitude offset, award record (0050FC30, 0050A210, 00501FA0) into +48h/+4Ch, highlight +D4h (00AA7DC0, 00AA7910). |
| 23 | 005EBDD0 | BSP_MultiModeSelectorScreen_RefreshServerInfo | thiscall(screen) RET | Session ping as float, server name (localised only for globals.newplayer via _stricmp), connection_t_Text hidden, 00578320, two numbers, grey/sand colours by [00F8A2FC]+4Dh, 00ABB000 ellipsized names from 00E08E00. |
| 23 | 005EE520 | BSP_HudObjectivesScreen_RefreshRows | thiscall(screen) RET | Objective vector rebuilt (005EDD70, 008DA180, 008DA140, 005EE3B0) in two passes, number widgets deleted and rebuilt, rows laid out with 00AAB4C0 clones (00A9D750 for the blank row), 00A9C7C0. |
| 25 | 00616C70 | BSP_HudPowerupManagerScreen_FillSecondRows | thiscall(screen) RET | +194h erased (004685A0) and refilled from +184h (00442190); +1A4h compared (size, then elements) and set from +194h, flagging +1E4h/+1E8h on a change unless +200h; inlined push_back (00441F30) and erase with memmove_s. |
| 26 | 00637080 | BSP_HudSceneInitScreen_Enter | thiscall(screen) RET (vtable +18h) | Status text by game state, hint from the global vector 00E08784 (checked) or "", bg_Group children, frameFlag_Icon by [00E08798], loadingLogo width above 4/15, then in game 004F83B0 and vtable +18h of [00E19698]. |
| 26 | 0063C110 | BSP_HudUnitMarkerWidgets_Construct | thiscall(set; root) RET 4, EAX | vtable 00CF5758; CLOSE_Group; the six colour icons of 00E08C90 cloned four times each (00AA7E00, 00AAB4C0) and hidden; seven named children under root cloned into +68h..+80h. |
| 26 | 00652C80 | BSP_Hud_UnitIconFrame_00652C80 | thiscall(screen; class, a2, a3, char use_unit, unit) RET 14h, EAX | Icon frame from the unit's classifier (+35Ch vtable +18h) and vtable +5Ch kinds with the payload bytes +808h/+81Ah/+82Ch, the 00E08FA0 command class flagging screen+6Ch (mode 4..7) and returning 21h; or the same table on the class argument. |
| 26 | 00642C20 | BSP_InGameHudMarkers_AddCommandUnitMarker | thiscall(screen; unit) RET 4 | 00638E50 projection gate, 0063A6C0 bounds, "[clan] name" text (three 004261A0 joins) or the bare name (0041E350), colour from 00432650 +108h/+128h or the {1,1,1,1} static, marker 00640940 placed with 00AA7D00. |
| 27 | 00671D60 | BSP_HudSupportManager_ShowSelectionInfo | fastcall(screen) RET | 00670020 rebuild, by-value copies of the support-map value (005AAAF0/005A0B80) choose the name text (vtable +14h names or globals.unitclass_spawnpoint), 0065BBC0, then ingame.sm_slots|.: (008434D0 or 006BDC30) or ingame.sm_cp|.: (_ftol2 of 00946F00). |
| 46 | 007A8DA0 | BSP_MovCam_StepDeckWalk | thiscall(camera) RET | Stack list of the player's kind-5 units; 00797070 with the next unit after +390h (00793380) or the head; with no target every unit is scored but no index is kept, so the LAST unit is picked; then 0079F710 and 007A8C60; 004BF8E0. |
| 46 | 007ADD70 | BSP_EntityCommand_PathCursorFollowStep | thiscall(cursor; pos, radius, count, heading, factor, force) RET 18h, AL | Whole body including the atan2 lookahead and the multi-leg advance loop (007ADCC0 + cursor vtable +8h). For follow modes outside 1..3 the image's step is an uninitialised stack slot; the C++ uses 0 (documented divergence). |

## Unreferenced, recorded (ledger only)

- 005C4080 BSP_MissionTree_IncompleteMissionSideMask (seg 20): tools/callsite_census.py found 0
  rel32 and two absolute-dword byte matches that are not references (00AF5C29 and 00AF6A9B carry
  the displacement 5C40h of a word store/add); positive control 005C4D40 found 5. About 200
  instructions, over the limit, so not reconstructed.
- 006DE5C0 BSP_MarkerManager_DeserializeState (seg 32): 0 rel32 / absolute hits; positive control
  006DA5B0 found 4. 1104 bytes, not reconstructed.

## Adoption notes (host files not edited)

- 007ADD70: `src/game_hosts_commands.cpp` `advance_path_cursor_00836bf0` takes one leg per director
  step and states the multi-leg remainder as NOT PROJECTED (docs/HANDOFF_SHIP_DRIVE_NEXT.md (b)).
  The image's 007ADD70 scans up to the mode limit (count - index, index, or count - 2; clamped to 1
  when the count argument's low byte is 0), stops at the first point inside the radius (with the
  0 < factor < 1 atan2 lookahead narrowing the radius by |sin(00438B10(heading, a))|), and then
  advances pass + 1 times through 007ADCC0 and the cursor's vtable +8h, re-testing the
  PATH_FM_SIMPLE end before each advance. LIVE on the ship path-follow arm (00836D05) and the plane
  squadron step (0084E010). No RNG.
- 00642C20: `src/game_hosts_hud_world.cpp` only records the call
  (`HudMarkers::sweep_command_units`); the reconstruction here is the body.
- 0052BB10: `include/bsp/frontend_seg15_22_cc13.hpp` declares the hook with a `std::uint32_t value`
  first argument; the image's argument is a pointer to a {data, count} unit array.

## Deferred in this pass (no record)

- 007A87B0 BSP_MovCam_UpdateHandMode (seg 46): an x87 register-stack state machine
  (007A88E0-007A8BD9, values carried across branches on ST0..ST2 with FXCH/FCOMI/FCOMIP) choosing
  the next vehicle camera by the +504h/+508h stick intent; needs the stack tracked per edge.
- 006697E0 (seg 27): the support catalog collect is mostly inlined std::map / std::list work
  (tree find, erase-range + copy-tree assignment, list clears with _free), the same category as
  0066A8A0 that sweepB3 set aside.
- 00641910 BSP_InGameHudMarkers_AddKind1CMarker (seg 26): two paths that pass by-value
  aggregates (a 16-byte colour, a native string built in the argument area, float triples) to
  0063C620 and 0063D1E0, plus the register-passed 00638C60; left for a pass that takes those two
  callees with it.
- 005E12A0 BSP_MultiGameLobby_LoadWeatherTable (seg 23): a LuaPlus table reader with a 5A4h-byte
  frame of LuaObject temporaries; left with the other Lua data-table readers.
