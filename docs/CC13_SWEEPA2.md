# CC13 sweep A2: segments 0..14 leftovers, continued (cc13-sweepA2)

Packet cc13-sweepA2 (2026-10-16) continues docs/CC13_SWEEPA.md: it takes the
bodies sweepA listed as "skipped (context budget)", smallest first (004A0D20,
the 72 KB listing, is excluded by the lead). Source: `src/cc13_sweepA2.cpp`,
header `include/bsp/cc13_sweepA2.hpp`; nothing is wired. `SweepA2Host` extends
`cc13_sweepA::SweepAHost`, so sweepA's bindings are reused and sweepA's
reconstructions (0043D870, 00521370, 00522680, 00522860) are called directly.
Descriptive names are hypotheses, not recovered symbols. Status of every
reconstruction below: reconstructed and build-tested (MSVC Win32, compiled
into `bsp_core`); none is fixture-tested, ABI-compatible or game-validated.

Outcome counts (33 of sweepA's 67 skipped bodies closed): 28 reconstructed,
1 existing implementation recorded, 2 unreferenced recorded, 2 cc9 lane;
34 left (listed at the end). Plus one ledger name correction (004D0860).

## Method

- Each address was searched in `src/`, `include/` and `docs/` and with
  `bsp.py lookup`; none of the 28 had a reconstruction record or a body in
  `src/` (the hits were host hooks, labelled stand-ins or records).
- References: `bsp.py lookup` callers, and for a zero-caller address the PE
  census (`local/sweepA2_census.py`, every E8/E9/0F8x rel32 plus every
  absolute dword in any section; positive control 00441210 = 27 CALL + 1 JMP)
  with the call sites printed by `local/sweepA2_sites.py`.
- Bodies were read from the listing with the stack offsets tracked by hand at
  every push and callee RET. x87 register forms were taken from the bytes
  (`tools/x87_dc_form_scan.py`): two DC forms matter, DC E9 at 00522571
  (ST1 = ST1 - ST0) and DC CA at 0043ACF5 (ST2 = ST2 * ST0); 0048C6A0 has
  none. Constants were read from the PE with their width
  (`local/sweepA2_dw.py`).
- Every call the image makes happens at the same point through a named
  `SweepA2Host` / `SweepAHost` method (STL instantiations are host methods
  named by address); memcpy, memset, memmove and atol are bound directly.
- Common simplification, stated once: SEH unwind actions are not modelled
  (no reconstructed body throws itself). Shared RNG: no reconstructed body
  calls 00BD2F10 directly.

## Reconstructed

| Address | Function | Original ABI | Uncertainty |
|---|---|---|---|
| 004EE300 | `land_convoy_preload_type_classes_004ee300` | __thiscall(ECX = scene property bag), RET | SEH unwind not modelled; the second 008F2260 search runs on a null sub-bag exactly as the image does. |
| 004F01F0 | `scene_property_bag_remove_record_004f01f0` | __thiscall(ECX = bag {+4 count, +8 64 bucket heads}, record), RET 4 | None beyond the host bindings; no SEH frame. |
| 0043ABA0 | `hud_digit_icon_roll_uv_0043aba0` | __fastcall(ECX = digit index, EDX = widget, float magnitude), RET 4 | x87 extended intermediates modelled in double: the 1/12 products and the hundredths division can differ in the last float bit. |
| 0043D8F0 | `lua_table_copy_into_property_bag_0043d8f0` | __fastcall(ECX = LuaObject* table, EDX = bag), RET | SEH unwind not modelled; LuaPlus calls are host bindings. |
| 00440AC0 | `gun_class_read_lua_explosion_efx_00440ac0` | __thiscall(ECX = class, LuaObject* table), RET 4 | SEH unwind not modelled; the null-slot release (state bit 1) can never fire and is kept for fidelity. |
| 00440F50 | `device_entity_on_killed_lua_00440f50` | __thiscall(ECX = device entity), RET | SEH unwind not modelled; 008689C0 consumes the by-value handle copy (its release is the callee's). |
| 004423A0 | `device_class_registry_clear_lua_class_tables_004423a0` | no register or stack input, RET | SEH unwind not modelled; the 0043EE60 buffer is the 2Ah-byte frame area (modelled as 2Ch bytes). |
| 0045A210 | `movie_track_insert_action_sorted_0045a210` | __thiscall(ECX = track, action), RET 4 | The constant container-identity test (CMP ESI,ESI) is not modelled. |
| 0048C6A0 | `scene_entity_layer_member_0048c6a0` | __thiscall(ECX = layered entity, layer L, index i), RET 8 | 00489C40 is tagged STL_inst in the ledger but here returns a record whose +4 is a side; its body is not reconstructed. The products are exact in double for float operands and small indices. |
| 0048F670 | `named_class_read_lua_0048f670` | __thiscall(ECX = class, LuaObject* table), RET 4 | SEH unwind not modelled; MSVC std::string layout (+4 buffer, +14h size, +18h capacity) assumed from the stores. |
| 0045D230 | `movie_record_assign_0045d230` | __thiscall(ECX = destination, const source*), RET 4 | Implicit copy assignment of a game record; member types inferred from the 0041DD40 + memcpy pattern. |
| 0045F110 | `movie_record_copy_construct_0045f110` | __thiscall(ECX = destination, const source*), RET 4 | Implicit copy constructor; SEH per-member states not modelled. |
| 0045D930 | `cutscene_player_destruct_0045d930` | __thiscall(ECX = cutscene player), RET | SEH unwind not modelled; the memmove_s tail move is dead (zero length) and modelled with std::memmove. |
| 004D4720 | `scene_build_class_preload_aliases_004d4720` | __thiscall(ECX = alias list 010904E4, a std::list of pooled strings: +4 the sentinel), RET | The container-identity tests compare the constant 00F8A09C with itself and are not modelled; SEH unwind not modelled. |
| 004E1A50 | `award_tracker_destroy_members_004e1a50` | __fastcall(ECX = tracker), RET | Element types of the trees and the deque are not established (vendor erase bodies bound by address). |
| 004D7BA0 | `game_show_session_lost_message_004d7ba0` | __thiscall(ECX = game, char kicked), RET 4 | The memcpy of the cable-lost text copies the length without the terminator (the resize writes it); SEH not modelled. |
| 004D8620 | `frontend_update_active_screens_004d8620` | __fastcall(ECX = game), RET | VC8 vector layout (first at +4) assumed from the stores; SEH not modelled. |
| 0050B730 | `tactical_library_show_selected_group_0050b730` | __thiscall(ECX = tactical library screen), RET | The local vector<int> is filled but never read, as in the image; vector element type of +444h not established. |
| 004F1B60 | `scene_header_append_unquoted_names_004f1b60` | __fastcall(ECX = const char* list, EDX = owner), RET | The strcpy into the 104h-byte frame buffer is unbounded, as in the image; SEH not modelled. |
| 005213D0 | `command_building_view_bind_unit_camera_005213d0` | __thiscall(ECX = view, unit), RET 4 | SEH around the allocation not modelled; memset runs even on a failed allocation, as in the image. |
| 005224C0 | `squadron_member_nearest_screen_centre_005224c0` | ECX unused, (squadron), RET 4 | x - 0.5 and |dx|+|dy| modelled in double; a member index above 4 dereferences null as the image does. |
| 005226D0 | `unit_pick_is_command_candidate_005226d0` | ECX unused, (unit), RET 4 | ECX is unused; AL = 0 for a null unit leaves the rest of EAX to the caller (modelled as false). |
| 00522F10 | `unit_pick_consider_unit_00522f10` | __thiscall(ECX = screen, unit, void** command_out, float* best_distance, int mask, owner, void** best), RET 18h | No result; the caller reads the out pointers. |
| 00523020 | `unit_pick_find_member_target_00523020` | __thiscall(ECX = screen, void** command_out), RET 4 | Members past index 4 are offered as null and dereferenced by 00522F10, as in the image. |
| 00523130 | `unit_pick_find_command_target_00523130` | __thiscall(ECX = unit pick screen, void** command_out), RET 4 | 00522680 is inlined in the image here (005231F2..00523244) and modelled inline. |
| 00525250 | `unit_pick_issue_command_at_cursor_00525250` | __thiscall(ECX = unit pick screen), RET | The command slot is reused as the tracked-sound result, as in the image. |
| 00525F80 | `unit_pick_collect_candidates_00525f80` | __thiscall(ECX = unit pick screen), RET | SEH unwind not modelled; observer-handle vptrs are written as image addresses. |
| 005241D0 | `unit_pick_issue_command_to_target_005241d0` | __thiscall(ECX = screen, command), RET 4 | The 57h session message has no destructor call in the image; the frame storage size (40h) is taken from the frame layout. |

The comment above each function in `src/cc13_sweepA2.cpp` gives its full
behaviour, callers and constants; the ledger record's evidence repeats it.

## Existing implementation, recorded ledger-only

| Address | Implementation | Outcome |
|---|---|---|
| 004DB920 | `bsp::run_mission_device_wait` (src/mission_state_entry.cpp) | Verified against the listing: the arm split on game+1FE4h / [game+1EF0h+29Ch], the device rising edge on [00F8BBF4]+DDh, the two screen gates, 004BEC00 -> 00A92C40(0.0f), the eight-slot unbound count and the [game+624h] gate, both tails into 004DA6C0. The session event build/dispatch/teardown (0075B430, 0076A9F0, 007848F0) is one host hook. |

## Unreferenced, recorded ledger-only

- 004F3DA0 BSP_Triangle2f_ContainsPoint (113 instructions): its only CALL is at
  004F3EE6 in the undefined wrapper 004F3EE0 (MOV EDX,ECX; MOV ECX,[ESP+4];
  CALL; RET 4), which has no rel32 and no absolute-dword reference.
- 004F4880 BSP_Segment2f_ClipAgainstLine (116 instructions): its only CALL is
  at 004F4B1F in the undefined function 004F4AE0 (after 004F49F0 ends at
  004F4ACE), which has no rel32 and no absolute-dword reference.

Both are dead chains; neither was reconstructed (over the 80-instruction limit
for unreferenced bodies).

## Ledger name correction

004D0860 was named `BSP_Game_SplitCommandToken`; it is `std::operator+`
(const std::string&, const std::string&) returning std::string by value:
__cdecl, hidden result [ESP+4], left [ESP+8], right [ESP+0Ch]; a temporary
assigned from left (00408120), appended with right (004C90D0), copied into the
result (00408120) and freed. Replaced with `ledger add-name --replace`
(an STL instantiation: name only).

## Adoption notes (gameplay host files, not edited)

- 00525250 (and its 00523130 pick): src/game_hosts_mission_frame.cpp
  3337..3430 issues the player's squadron target through a LABELLED stand-in
  (the target is named by the order file instead of the screen-centre pick;
  00804350 and 005220C0 are not run). The image's pick is
  `unit_pick_find_command_target_00523130`; its flags are the whole EAX of
  vtable +5Ch(18h). LIVE only for scripted squadron-target orders; no
  00BD2F10 draw in either path.
- 0043D8F0: src/game_hosts_lua.cpp 3786 and 3830 model the merge for the
  `Equipment` and `Race` keys only; the image copies every key by type
  (table/integer/boolean/number/string, in that order). Not a deviation in
  those two keys; other authored keys are not merged by the host.
- 0043ABA0: src/game_hosts_hud.cpp 2505..2513 records the call and does not
  roll the digit UVs (the sprite bridge draws the first authored state).

## cc9 lane (skipped)

- 004C3A80 (controlled-unit selection over 00F87198 in mode 0Dh):
  docs/CC13_SEGS_0_9_11.md line 124 says it belongs to cc9.
- 004C74C0 BSP_World_BorderZoneAtPoint: docs/CC13_SYS_4.md line 213 leaves it
  to cc9 (read by the cc9 retreat chain).

## Left for a later packet

<!-- skipped-list -->
- 004DD7C0 BSP_Game_CompleteLeaderboardRead (193 instructions): not attempted in this packet (context budget; left for a later packet).
- 0051EAA0 BSP_HudBinoculars_StepInputAxes (200 instructions): not attempted in this packet (context budget; left for a later packet).
- 004E5540 BSP_Game_BeginStartupSequence (202 instructions): not attempted in this packet (context budget; left for a later packet).
- 004CCE50 BSP_Game_BeginEngineMovie (218 instructions): not attempted in this packet (context budget; left for a later packet).
- 0045ECA0 FUN_0045eca0 (229 instructions): not attempted in this packet (context budget; left for a later packet).
- 004C7AA0 FUN_004c7aa0 (231 instructions): not attempted in this packet (context budget; left for a later packet).
- 0045F600 BSP_EngineMovie_Stop (233 instructions): not attempted in this packet (context budget; left for a later packet).
- 00526720 FUN_00526720 (234 instructions): not attempted in this packet (context budget; left for a later packet).
- 004D32A0 BSP_Game_NilScriptAddedGlobals (235 instructions): not attempted in this packet (context budget; left for a later packet).
- 00487470 FUN_00487470 (240 instructions): not attempted in this packet (context budget; left for a later packet).
- 0050B440 FUN_0050b440 (241 instructions): not attempted in this packet (context budget; left for a later packet).
- 005220C0 FUN_005220c0 (257 instructions): not attempted in this packet (context budget; left for a later packet).
- 00487A70 FUN_00487a70 (260 instructions): not attempted in this packet (context budget; left for a later packet).
- 00456DD0 BSP_MovieTrack_BindEntities (263 instructions): not attempted in this packet (context budget; left for a later packet).
- 00460750 FUN_00460750 (272 instructions): not attempted in this packet (context budget; left for a later packet).
- 0046B730 FUN_0046b730 (272 instructions): not attempted in this packet (context budget; left for a later packet).
- 00526180 FUN_00526180 (274 instructions): not attempted in this packet (context budget; left for a later packet).
- 00443090 BSP_DeviceClass_ResolveFromLua (289 instructions): not attempted in this packet (context budget; left for a later packet).
- 0045D480 BSP_MovieActionType_Construct (292 instructions): not attempted in this packet (context budget; left for a later packet).
- 0044B5B0 BSP_PanelSequence_DescribeProperties (329 instructions): not attempted in this packet (context budget; left for a later packet).
- 004D2BB0 BSP_Game_DestroyWorld (348 instructions): not attempted in this packet (context budget; left for a later packet).
- 004D80D0 BSP_Game_UpdateMultiplayerInterface (359 instructions): not attempted in this packet (context budget; left for a later packet).
- 004602C0 BSP_EngineMovie_ParseFile (367 instructions): not attempted in this packet (context budget; left for a later packet).
- 00447510 BSP_GameDynamicsList_Add (377 instructions): not attempted in this packet (context budget; left for a later packet).
- 004E2200 BSP_Game_RunConsoleCommandQueue (378 instructions): not attempted in this packet (context budget; left for a later packet).
- 004E3AA0 BSP_Game_OnInit (387 instructions): not attempted in this packet (context budget; left for a later packet).
- 004EA080 BSP_SceneClass_ReadStockLists (415 instructions): not attempted in this packet (context budget; left for a later packet).
- 004507D0 BSP_PanelSequence_ReadProperties (434 instructions): not attempted in this packet (context budget; left for a later packet).
- 0045DE50 BSP_MovieAction_Create (524 instructions): not attempted in this packet (context budget; left for a later packet).
- 004E8BF0 BSP_SceneHeader_Assign (531 instructions): not attempted in this packet (context budget; left for a later packet).
- 00459020 BSP_MovieEntityTrack_Update (588 instructions): not attempted in this packet (context budget; left for a later packet).
- 0051CDE0 BSP_BriefingScreen_Enter (1269 instructions): not attempted in this packet (context budget; left for a later packet).
- 0051FA50 BSP_CheatMenuScreen_BuildLines (1679 instructions): not attempted in this packet (context budget; left for a later packet).
- 004A0D20 FUN_004a0d20 (2199 instructions): skipped by the lead (72 KB listing).
<!-- /skipped-list -->
