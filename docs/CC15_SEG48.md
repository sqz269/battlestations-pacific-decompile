# cc15-r48a: the live unbuilt rows of segment 48 (007D1E50-007F8390)

Packet cc15-r48a, 2026-10-18. Build only: `src/cc15_seg48.cpp`, `include/bsp/cc15_seg48.hpp`
(added to `bsp_core` in `cmake/startup.cmake`). Nothing is wired into a host. Names are hypotheses,
not recovered symbols. Every built body is reconstructed and build-tested only: not ABI-compatible
(one host hook per native call, `Cc15Seg48Host`, derived from cc13's `Cc13Seg48UHost`), not
fixture-tested, not game-validated.

## Selection

The progress board's rule (`tools/progress_board.py`: `is_game`, `LIBRARY_NAME_PREFIXES`,
`referenced_functions`, crediting ledger function records and source bodies) left 52 live game rows
without a reconstruction in the segment. Despite the segment keywords (travel speed, gears, bay doors,
wings) every one of them is a plane-squadron, squadron-message or squadron-neighbour row; the plane
part bodies were already done by cc13 and cc9. Each row was checked against `src/` and `include/` by
address and by behaviour before a body was written (cc9's rule 1).

x87: the main thread runs at 24-bit precision control, so every x87 FADD/FSUB/FMUL/FDIV rounds to a
float; the bodies use float arithmetic.

## Counts

| Class | Rows |
|---|---|
| Built (`src/cc15_seg48.cpp`) | 36 |
| Ledger-only (modelled elsewhere; host or existing body cited) | 16 |
| Deferred-partial (cc9's 007D8000-007E3000 band) | 0 |
| Library / dead | 0 (excluded by the board rule before selection) |

## Built

| Address | Body | ABI | Notes |
|---|---|---|---|
| 007EE150 | plane_squadron_max_member_908 | fastcall(squadron), ST0 | A maximum from 0.0, not a minimum (renamed BSP_PlaneSquadron_MaxMember908). |
| 007EE2E0 | is_leaving_on_purpose | thiscall(squadron; byte) RET 4 | Whole body; cc9 reads only its retreat arm. |
| 007EE4E0 | detach_pilot | thiscall(squadron) | UNITEXITED (007EDF50 inlined) per member, channel 2 in mode 2 else 7. |
| 007EE720 | first_member_df4_entry | thiscall(squadron), AL | cc9 takes it as true (labelled). |
| 007EE790 | recount_active_wingmen | thiscall(squadron) | cc9 counts at the call (labelled substitution). |
| 007EE8F0 | unit_attack_command_applies | thiscall(squadron; class, target, byte allow, loadout) RET 10h | Eleven class arms, read from the listing (Ghidra's pseudocode mislabels the stack arguments). |
| 007EEE20 | target_is_home | fastcall(squadron) | |
| 007EF490 | PLANESQUADRON_CREATE construct | thiscall(message; class word, squadron, home, byte) RET 10h | The read constructor and destructor are cc13_seg48's 007EDB80 / 007EDC10. |
| 007EF590 / 007EF610 | CREATE write / read | thiscall(message; cursor or stream) RET 4 | Float +11Ch: 16 bits, max 10.0 (00CE38B8). |
| 007EF690 / 007EF6C0 | ATTACKRUN write / read | thiscall(message; cursor or stream) RET 4 | |
| 007EF8B0 | command_record_init_with_target | thiscall(record; class, descriptor*) RET 8 | |
| 007EF9D0 | plane_squadron_destruct | thiscall(squadron) | Six vtables, then 006E0860 / 00875490 / 0077E380. |
| 007EFA80 / 007EFA90 / 007EFAC0 | accessors, slot 18Ch no-op | | 007EFA90 reads [ECX+1E4h]; on the full object that is a vtable, so it is called on a secondary base (uncertain which; recon_slot_lists.hpp reads +354h through +170h). |
| 007EFB30 | raise_force_release_flag | thiscall(squadron) | |
| 007EFEB0 | slot168_command_target_check | thiscall(squadron; const char*) RET 4 | Ten command strings. |
| 007EFFB0 | promote_armed_leader | thiscall(squadron), AL | |
| 007F0030 | pilot_control_handle_message | thiscall(squadron; message) RET 4, AL | Ids 4Bh, 4Ch, 7Fh, 80h, BCh, BEh (tables 007F0204 / 007F01E8); cc9 models only BEh. |
| 007F0B30 | send_create_message | thiscall(squadron; peer) RET 4 | |
| 007F0BF0 / 007F0DC0 | PilotBotSettings save / load | thiscall(settings=squadron+37Ch; archive) RET 4 | The loader reads newattackalt before newtravelalt. |
| 007F1000 | command_record_init_with_entity | thiscall(record; class, entity) RET 8 | Records an entity (renamed BSP_CommandRecord_InitWithEntity). |
| 007F10E0 | unregister_from_world_lists | thiscall(squadron) | |
| 007F1160 | plane_squadron_save_to_archive | thiscall(squadron; writer) RET 4 | Key strings from the export. |
| 007F1C00 | set_home_air_base | thiscall(squadron; base, byte) RET 8 | |
| 007F1CA0 | return_plane_to_stock | thiscall(squadron; base, plane) RET 8 | cc9 carries the stock add, slot return and kill only. |
| 007F1D90 | select_avoid_zone_layers | thiscall(squadron; float, int) RET 8 | |
| 007F1FE0 | begin_from_creation_record | fastcall(squadron) | Kind 1 (scene) and kind 2 (CREATE message) arms; state-record bytes 00721280 leaves unwritten are zero here, stack garbage in the image. |
| 007F2BD0 | pilot_bot_settings_init | thiscall(block=squadron+37Ch), EAX | Countdowns +0h..+13h and the PilotBotSettings fields (renamed BSP_PilotBotSettings_Init). |
| 007F2E20 / 007F2EB0 / 007F2F90 | set world position / heading / matrix | thiscall(squadron; arg) RET 4 | Mode-1 ENTITY_RELOCATE messages via 0077C7B0. |
| 007F2FD0 | entity_add_plane_kill_reason | thiscall(entity; reason, count) RET 8 | PlaneKillReasons[00E0CF04[reason]] += count (renamed BSP_Entity_AddPlaneKillReason). |

Common uncertainty: exception unwinding is not modelled (hooks are assumed not to throw); archive
type-3 values carry stale upper bytes in the image and zero here.

## Ledger-only (modelled elsewhere)

| Address | Cite | What models it |
|---|---|---|
| 007DF360 | src/game_hosts_units.cpp:21464 | cc9 `target_reachable_007df360` under kTorpedoLineOfSightBound (ON, main c5be2a47a), docs/OCEAN_WAVE_FIELD.md section 87. Body 007DF360-007DF4E9 (exclusive; RET 8 at 007DF4E6), thiscall(ECX = unit+C50h neighbours; target, const float* point), AL. AL = 1 means screened: an IsKindOf(6) member other than the target lies ahead of the point toward the plane with perp^2 < ([n+538h]+A0h)^2 * 0.22 (00D06BA0); the caller 009D36F3 overwrites [+A9h] with it. The host name "target_reachable" is a misnomer for that answer. Name BSP_PlaneNeighbours_LineOfSightClear kept at cc9's request (cc15-r48b; a cc15-r48a body and rename were withdrawn). |
| 007EB0D0 | src/cc13_seg48U.cpp:694 | JMP thunk to 007E29A0, whose body is reconstructed. |
| 007EE080 | src/band_007a_cc13.cpp:10 | Type predicate as a TypeIdRule row {BBh, 47h, 46h}. |
| 007EE410 | src/game_hosts_units.cpp:9509 | cc9 retreat_leave_map_007ee410 (gate and message effects). |
| 007EE5C0 | src/game_hosts_script_orders.cpp:2120 | cc9 inline unstowed-member count. |
| 007EE670 | src/game_hosts_hud.cpp:1680 | cc9 leader-active test (+361h read clear, labelled). |
| 007EE6B0 | src/game_hosts_units.cpp:16841 | cc9 per-member break-off answers. |
| 007EE8B0 | src/game_hosts_units.cpp:16920 | cc9 override squadron_outside_map_detached_007ee8b0. |
| 007EF010 | src/game_hosts_units.cpp:15275 | cc9 launch-task tick sends the next member inline. |
| 007EF1C0 | src/game_hosts_lua.cpp:5334 | cc9 GetProperty "state" / "ammoType" (5334, 5368). |
| 007EFA70 | src/entity_mission_seg60_argon.cpp:1086 | JMP thunk to 00928860, whose body is reconstructed. |
| 007F10B0 | src/game_hosts_units.cpp:4025 | cc9 list-24 push (lists 1/2 fused, labelled). |
| 007F1B70 | src/air_operations.cpp:231 | release_squadron_slot per air-ops block (also game_hosts_lua.cpp:7909). |
| 007F1DE0 | src/game_hosts_units.cpp:39980 | cc9 launch-task record construction. |
| 007F23A0 | src/plane_formation.cpp:97 | plane_formation_station_007f23a0; host game_hosts_units.cpp:8020. |
| 007F44E0 | src/ship_sub_entity_list.cpp:48 | SubEntityProvider::SquadronPlanes append. |

## Deferred-partial (for cc9)

None. The only row in 007D8000-007E3000 is 007DF360, which cc9's host models whole (on main since
c5be2a47a), so it is ledger-only.
