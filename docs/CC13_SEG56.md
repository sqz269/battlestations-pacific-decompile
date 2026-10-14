# CC13 segment 56: mission script bindings (packet cc13-seg56)

Segment 56 (0088B410..008D3E00) is the mission Lua binding table's handlers (rows of the table
at 00E0B7B8, `config/lua_bindings.json`). Its keyword list (options, hardwarereported, vsync)
comes from string literals the segment shares with the options code; the functions themselves
are script natives. An earlier pass (cc13-sys5, `src/engine_seg56_cc13.cpp`) reconstructed 138
of them; this packet takes 68 of the remainder that had neither a tag nor a reconstruction
record.

Code: `src/options_seg56_cc13.cpp`, `include/bsp/options_seg56_cc13.hpp`, namespace
`bsp::seg56_options_cc13`. Status: reconstructed, build-tested only. Not ABI-compatible (the
natives are `int __fastcall(lua_State*)`; these take an argument reader, a result writer, an
image accessor and a host), not fixture-tested, not game-validated.

## Shared shape

Every binding opens with a dead prologue (a `luaMW_<name> failed:` string built and freed, a
one-time `luakod` guard), wraps the state (00B66C00), opens a call frame (00B679B0) and reads
argument i with 00B677E0(frame, i). Entities come from 00888AA0 (`entity_at`). The return value
is 00B66400, the count of values on the frame; it is a literal in the C++ wherever only the
binding itself pushes, and `LuaFrame::result_count()` for ClearFailure, whose virtual callee
receives the frame. No binding below tests the resolved entity for null unless its row says so,
and the reconstruction adds no such test.

Field reads and writes go through `ImageAccess` with the image offset; each call the native
makes is one `Seg56Host` method whose name carries the callee address (or the vtable slot).
Three host methods stand for more than one native call, by design:
`session_send_0076a9f0` (0075B430 + the field stores + 0076A9F0 + the base-vtable restore),
`route_message_0077c2a0` (the same with 0077C2A0), and
`unit_group_broadcast_formation_set_0077a080_0077c880` (construct, send, two `_free`s).

Evidence: the Ghidra listing of each function between its 00B66C00 and 00B66400 calls,
register sources checked in the prologue (EBP = 0 is proven by `PUSH EBP` as argument index 0;
EBX/EDI where used are cited), and the immediate bytes where the listing width mattered.

## Functions

| Address | Lua name | C++ | Behaviour and evidence |
|---|---|---|---|
| 0088C750 | BannSupportmanager | `lua_bann_support_manager` | 00A38DA0(CL=1) at 0088C838; when game+1FE4h == 1, 0075B430(D7h), +4h=1, vtable 00D035E0, byte +18h=1, 0076A9F0(game+1EF0h, &msg, 0) at 0088C883. |
| 0088C8E0 | PermitSupportmanager | `lua_permit_support_manager` | Same shape as 0088C750 with XOR CL,CL at 0088C9BE and the message flag byte 0. |
| 0088CA70 | ShowHideSupportManager | `lua_show_hide_support_manager` | 0068C0B0(ECX=[00E198C4], PUSH EBP=0) at 0088CB5D; reads no argument. |
| 0088CBB0 | SupportManagerEnable | `lua_support_manager_enable` | entity arg0, boolean arg1; true: 0065BBA0([[00E198C4]+BCh], entity) (list +1F8h remove via 0065B1F0); false: 00665EF0 (append to +1F8h). Panel read after both args. |
| 0088CD70 | IsSMVisible | `lua_is_sm_visible` | PushBoolean(006529E0([[00E198C4]+BCh])); 006529E0 is MOV AL,[ECX+4] / RET. |
| 0088D000 | UnlockAllMission | `lua_unlock_all_mission` | Empty body: opens the call frame (0088D0D9) and returns 00B66400; no argument read, no call. |
| 0088D2B0 | SetBPCsodaSquadMereteHack | `lua_set_bpc_soda_squad_merete_hack` | integer arg0, then 006529D0([[00E198C4]+BCh], int) at 0088D3D3; 006529D0 stores [ECX+2B4h]. |
| 0088F3C0 | ClearFailure | `lua_clear_failure` | entity arg0, string arg1 assigned to a native string (0041E870), then entity vtable+19Ch(&name, &frame) at 0088F51C; returns 00B66400 of the frame the callee received. |
| 0088F5A0 | ClearAllShipFailure | `lua_clear_all_ship_failure` | 0093A390(ECX = entity+A20h) at 0088F6BC. |
| 0088F710 | SetDamagedGFXLevel | `lua_set_damaged_gfx_level` | entity vtable+1B4h(integer arg1) at 0088F851. |
| 0088F8B0 | ActivateSpawnpoint | `lua_activate_spawnpoint` | ArgCount == 1: 0084CE60(entity, 1); else integer arg1 into message BFh (0075B430; +4h=1, word +18h=0, byte +1Ah=0, vtable 00D02EC0, +1Ch=arg1, byte +20h=1) and 0077C2A0(entity, &msg, 7, 0) at 0088FA42. |
| 0088FAB0 | DeactivateSpawnpoint | `lua_deactivate_spawnpoint` | As 0088F8B0 with 0084CE60(entity, 0) and message byte +20h = 0 (0088FC33), route at 0088FC49. |
| 0088FCC0 | GetDamagedGFXLevel | `lua_get_damaged_gfx_level` | PushInteger([entity+358h]) at 0088FDE1. |
| 0088FE30 | SetNumbering | `lua_set_numbering` | 008761E0(entity, integer arg1) at 0088FF68. |
| 0088FFD0 | SetFormationShape | `lua_set_formation_shape` | group = [entity+284h]; null skips everything (JZ 00890176) including arg1; else 0070EFD0(group, int arg1), 0077A080(&msg, group), 0077C880([group+14h], &msg), two _free of the message buffers. |
| 008901C0 | GetFormationShape | `lua_get_formation_shape` | PushInteger(group ? [group+500h] : EDI=-1) with group = [entity+284h] (008902D6..008902EE). |
| 00890340 | SetPlaneGears | `lua_set_plane_gears` | entity arg0, int arg1; for i < [sq+3CCh] (re-read): 007B8CE0(i<=4 ? [sq+3D0h+4i] : null, arg1, 0) at 008904A4. |
| 00890510 | GetPlaneSurelyLanded | `lua_get_plane_surely_landed` | true when byte [+904h] != 0 and [+900h] is 2 or 1 (00890629..00890646). |
| 008906A0 | GetPlaneLandingAborted | `lua_get_plane_landing_aborted` | true when byte [+904h] == 0 and [+900h] is 2 or 4; otherwise vtable+38h of the sub-object at +72Ch (008907D1..008907E0). |
| 00890840 | ForcePlaneGears | `lua_force_plane_gears` | As 00890340 with the second 007B8CE0 argument 1 (PUSH 1 at 008909A1). |
| 00890BB0 | GetShipMaxSpeed | `lua_get_ship_max_speed` | PushFloat((float)0080FC30(entity)). |
| 00890EF0 | GetShipSpeed | `lua_get_ship_speed` | PushFloat((float)entity vtable+38h()) at 0089100D..00891017. |
| 00891070 | SetForceSellableUnits | `lua_set_force_sellable_units` | boolean arg0, then 005F9470([[00E198C4]+64h], bool) at 0089117E; 005F9470 stores [ECX+8Ch]. |
| 008914F0 | BreakShip | `lua_break_ship` | 00814520(entity) when 0092BEA0(0080E490(entity)) and then 0092BE90(0080E490(entity)) are non-zero (00891606..0089162C). |
| 00892AA0 | GetNumCatapulted | `lua_get_num_catapulted` | PushInteger: 0 for a null entity or vtable+5Ch(6) false, else 00953920(entity) (00892BB6..00892BD8). |
| 00892C30 | SetCatapultStock | `lua_set_catapult_stock` | non-null entity with vtable+5Ch(5): 009539A0(entity, int arg1); arg1 read only on that path. |
| 00893240 | SetPump | `lua_set_pump` | Reads entity arg0 (008932E7) and boolean arg1 (00893317) and discards both; no other call. |
| 008938E0 | GetSubmarineAirSupply | `lua_get_submarine_air_supply` | PushFloat(float [entity+127Ch]) at 008939F6..00893A0C. |
| 00893A60 | SetSubmarineAirSupply | `lua_set_submarine_air_supply` | float [entity+127Ch] = (float)number arg1 (FSTP at 00893BA1). |
| 00893DA0 | ForceSubmarinePeriscope | `lua_force_submarine_periscope` | byte [entity+1235h] = boolean arg1 (00893ED9). |
| 00894440 | SubmarineAttack | `lua_submarine_attack` | 0077D600(entity, 00E08F78 attackmove, 0088A810(arg1), 1) at 00894597. |
| 00894F20 | SetCrewLevel | `lua_set_crew_level` | entity vtable+128h(006E6210(ECX = int arg1)) at 0089507C. |
| 008950D0 | GetCrewLevel | `lua_get_crew_level` | PushInteger(006E6270(ECX = entity vtable+12Ch())) at 008951F4. |
| 008953F0 | GetSkillLevel | `lua_get_skill_level` | PushInteger(entity vtable+12Ch()) at 00895510. |
| 00895BA0 | GetAirBaseStatus | `lua_get_air_base_status` | PushInteger(006BF5C0(006BCD20(ECX=entity, DL=0))). |
| 00896220 | GetAirBaseSlotStatus | `lua_get_air_base_slot_status` | block = 006BCD20(entity, 0) before int arg1; PushInteger([[block+4Ch] + (arg1-1)*58h + 2Ch]). |
| 00896590 | SetAirBaseSlotReady | `lua_set_air_base_slot_ready` | 006BA4E0 on [block+4Ch] + (arg1-1)*58h (006BA4E0 stores 6 at +2Ch). |
| 00896EE0 | RemoveShipyardStock | `lua_remove_shipyard_stock` | 008457E0(entity, int arg1, int arg2) at 0089705F. |
| 008978D0 | GetAirBasePlaneLimit | `lua_get_air_base_plane_limit` | PushInteger(006BA6C0(006BCD20(entity, 0))); 006BA6C0 reads [ECX+58h]. |
| 00897E40 | InferiorIsDisabled | `lua_inferior_is_disabled` | PushBoolean(byte [entity+720h]). |
| 00898320 | ClearWait | `lua_clear_wait` | byte [entity+1DCh] = 0 at 0089843A. |
| 00898610 | InitAll | `lua_init_all` | 00925F20(CL=0) at 008986F8; reads no argument. |
| 00899810 | IsFormationLeader | `lua_is_formation_leader` | PushBoolean(00778890(entity)). |
| 00899980 | IsFormationFollower | `lua_is_formation_follower` | PushBoolean(007788B0(entity)). |
| 0089ADB0 | SetBeachFight | `lua_set_beach_fight` | Reads entity arg0 and int arg1 and discards both; no store, no call. |
| 0089B0C0 | SetCaptureValue | `lua_set_capture_value` | [entity+7A4h] = int arg1 at 0089B20C. |
| 0089B6D0 | GetCommandBuildingLevel | `lua_get_command_building_level` | PushInteger([entity+770h]). |
| 0089CE60 | GetSquadronLanded | `lua_get_squadron_landed` | PushBoolean(byte [entity+3B0h]). |
| 0089D690 | PlaneSetTargetRoll | `lua_plane_set_target_roll` | float [entity+9A8h] = number arg1 (FSTP at 0089D7C5). |
| 0089D830 | PlaneSetTargetPitch | `lua_plane_set_target_pitch` | float [entity+9ACh] = number arg1 (FSTP at 0089D965). |
| 0089E220 | SquadronSetHomeBase | `lua_squadron_set_home_base` | 007F1C00(entity arg0, entity arg1) at 0089E36C. |
| 0089EA00 | SquadronSetOldStyleBombing | `lua_squadron_set_old_style_bombing` | byte [entity+3A8h] = boolean arg1. |
| 0089EBA0 | PlaneForceRelease | `lua_plane_force_release` | 007BBBA0(entity) when byte [entity + word[00F876B8]*8 + 9C0h] != 0. |
| 0089F1F0 | PlaneChangeAmmoType | `lua_plane_change_ammo_type` | 007ED690(ECX=[00E188D8], int arg0); no entity argument. |
| 0089FCB0 | SquadronSetAIRetreatEnabled | `lua_squadron_set_ai_retreat_enabled` | byte [entity+368h] = !boolean arg1 (SETZ at 0089FDE7). |
| 008A0000 | SquadronEnableVehicleAvoidance | `lua_squadron_enable_vehicle_avoidance` | [entity+3A4h] /= 2, or &= 0FDh (imm32 25 FD 00 00 00 at 008A0157 clears bits 8..31 too). |
| 008A01B0 | SquadronEnableGunfireAvoidance | `lua_squadron_enable_gunfire_avoidance` | [entity+3A4h] /= 4, or &= 0FBh (imm32 at 008A0307, clears bits 8..31 too). |
| 008A0360 | SquadronEnablePlayerTakeoff | `lua_squadron_enable_player_takeoff` | byte [entity+360h] = BL (EBX = 1 from 008A037F). |
| 008A04C0 | SquadronDisablePlayerTakeoff | `lua_squadron_disable_player_takeoff` | byte [entity+360h] = 0. |
| 008A1A00 | SquadronRestoreTravelSpeed | `lua_squadron_restore_travel_speed` | 007ED590(entity). |
| 008A1F40 | SquadronSetDropAllBombPercent | `lua_squadron_set_drop_all_bomb_percent` | float [entity+390h] = (float)number arg1. |
| 008A2A50 | NavigatorLandAttackerStop | `lua_navigator_land_attacker_stop` | byte [00E0E2FC] = !boolean arg0 (SETZ DL at 008A2B56). |
| 008A32C0 | NavigatorAllowMaxDepth | `lua_navigator_allow_max_depth` | nav = [entity+73Ch] (read before arg1); byte [nav+20h] = boolean arg1. |
| 008A3460 | NavigatorForceMoveCloseToTarget | `lua_navigator_force_move_close_to_target` | nav = [entity+73Ch]; byte [nav+21h] = !boolean arg1 (SETZ). |
| 008A5310 | PilotTorpedo | `lua_pilot_torpedo` | 0077D600(entity, 00E08F18, 0088A810(arg1), 1) at 008A5467. |
| 008AB580 | IsUnitSelectable | `lua_is_unit_selectable` | PushBoolean(00645160(ECX=entity, DL=1)). |
| 008AD8D0 | SetRepairEffectivity | `lua_set_repair_effectivity` | float [entity+A48h] = (float)number arg1. |
| 008AFD50 | Pause | `lua_pause` | 004CD0F0(ECX=[00E188A8], boolean arg0, 0, 1) at 008AFE5E. |

## Uncertainties and notes

- Names: the 15 handlers that were `FUN_` now carry `BSP_LuaBinding_<row name>` in the name
  ledger; the Lua name comes from the binding-table row and agrees with each function's
  `luaMW_<name> failed:` literal. Field meanings (support panel, gear slots, avoidance bits)
  are hypotheses from those names.
- SquadronEnableVehicleAvoidance / SquadronEnableGunfireAvoidance clear with a 32-bit
  immediate 000000FDh / 000000FBh, so disabling also zeroes bits 8..31 of +3A4h. That is
  what the bytes say; whether any higher bit is ever set is not checked here.
- SetPump and SetBeachFight read their arguments and do nothing else. UnlockAllMission does
  not even read an argument.
- GetNumCatapulted tests class 6 and SetCatapultStock class 5 through the same vtable+5Ch;
  the two ids are what the PUSHes say.
- PlaneChangeAmmoType acts on the global [00E188D8], not on an argument.
- GetPlaneLandingAborted pushes EAX after a false AL from the controller; 00B66450 compares
  only the byte (CMP byte [ESP+8]), so that is `false`.

## Skipped

Existing implementations were not re-done; the addresses with hits in `src/` or `include/`
(about 108, mostly `src/game_hosts_lua.cpp` host natives and `src/mission_lua_host.cpp`
dispatch rows) are left for a recording pass. The larger untouched handlers (HitTarget,
SetWeaponDirectorTargetEnable, IsAreaEmpty, the scoring adders, GetETA, the trigger builders)
were not attempted in this packet.

## Existing implementations recorded (cc13-seg56b)

Of the 108 untagged segment-56 addresses that already had hits in `src/` or `include/`, 66 have
a working implementation elsewhere. Each one was compared with the Ghidra listing (read-only) and
given a reconstruction record that points at the existing code. None of that code was edited.
Counts by status:
- `existing_host_implementation_defect_found`: 2
- `existing_helper_verified_not_wired`: 1
- `existing_library_function_verified_not_wired`: 6
- `existing_host_implementation_verified_against_listing`: 26
- `existing_host_substitution_labelled_verified_against_listing`: 31

Defects found in existing code (not fixed here; the files belong to other lanes):

- 0089B840 GetCapturePercentage (`src/game_hosts_script_orders.cpp`, GetCapturePercentage
  branch): the binding applies FABS (D9 E1 at 0089B966) to the 006F1F90 fraction before
  PushFloat. The host pushes the signed value, and its comment says there is no FABS.
- 008AA8F0 SetForcedReconLevel (`src/game_hosts_lua.cpp` run_set_forced_recon_level_008aa8f0):
  00805CF0 runs on the entity's own record first (008AAA76), and then on the squadron members or
  the LandConvoy elements. The host forces the members instead of the entity, so a squadron's
  own record is never forced, and this deviation is not labelled.
- 00896A90 AddAirBaseStock (`run_add_air_base_stock_00896a90`), minor: the 0095BA60 party
  preload mark (00896BF5) is neither applied nor labelled.

Not recorded:

- 008B19A0 CountdownCancel: its listing was not compared beyond the comment's addresses.
- 0088B410: an implicit struct copy with no concrete host.
- The rest had mentions only (dispatch rows, header comments, callee-only reconstructions) and
  are reconstruction candidates (the 17 done in the next section are removed):
  0088B4B0, 0088B6D0, 0088B840, 0088E320, 0088E560, 0088E790, 0088EB50, 0088EDC0, 00891D50,
  008936A0, 008959F0, 008A4F00, 008A50D0, 008A5BD0, 008A6060, 008A8720, 008AC7B0, 008AD6F0,
  008AF700, 008AFEC0, 008C8560, 008CC850, 008CFC10.

## More reconstructions (cc13-seg56b)

These 17 bindings had only dispatch rows and comments. They are now in the same files as the
first batch, with the same conventions, and are build-tested only. New host methods are listed
under "added by cc13-seg56b" in the header.

| Address | Lua name | C++ | Behaviour and evidence |
|---|---|---|---|
| 0089D9D0 | SetPlaneYawControl | `lua_set_plane_yaw_control` | entity arg0 (00888AA0), float [entity+998h] = number arg1, FSTP at 0089DB05. |
| 0089DB70 | SetPlanePitchControl | `lua_set_plane_pitch_control` | entity arg0, float [entity+99Ch] = number arg1, FSTP at 0089DCA5. |
| 0089DD10 | SetPlaneRollControl | `lua_set_plane_roll_control` | entity arg0, float [entity+9A0h] = number arg1, FSTP at 0089DE45. |
| 0089DEB0 | SetPlanePowerControl | `lua_set_plane_power_control` | entity arg0, float [entity+9A4h] = number arg1, FSTP at 0089DFE5. |
| 0089FE50 | SquadronEnableTerrainAvoidance | `lua_squadron_enable_terrain_avoidance` | [entity+3A4h] |= 1, or &= 0FEh with a 32-bit immediate (25 FE 00 00 00 at 0089FFA7, clears bits 8..31), stored at 0089FFB0. |
| 008A28B0 | SquadronSetForceRelease | `lua_squadron_set_force_release` | byte [entity+3AEh] = boolean arg1 (008A29FC). |
| 008A2710 | SetTorpedoSwimDepth | `lua_set_torpedo_swim_depth` | object = 00888D20(arg0) (the table's Ptr field, 008A280F), float [object+47Ch] = (float)number arg1 (008A2861). |
| 008AC2B0 | RemoveUntouchableUnit | `lua_remove_untouchable_unit` | entity arg0, vtable+140h() (008AC3D0), byte [result+1D4h] = 0 (008AC3D6). |
| 00897720 | SetAirBasePlaneLimit | `lua_set_air_base_plane_limit` | block = 006BCD20(entity, DL=0) before int arg1; 006BA6D0(block, n) at 00897878 (stores block+58h). |
| 008C14B0 | SetDeviceReloadTimeMul | `lua_set_device_reload_time_mul` | float [00E08178] = number arg0 (FSTP at 008C15B4). |
| 008B21A0 | SetWeather | `lua_set_weather` | string arg0 assigned into an 8-byte by-value NativeString (008B22A4..008B22AE), 00445A30 on [game+21C4h] at 008B22BF. |
| 008A9E10 | FindHiddenEntity | `lua_find_hidden_entity` | string arg0, 00469480(ECX=[00E18680], text) at 008A9F1B, PushBoolean(AL) at 008A9F3D. |
| 0089AF40 | IsLandingTrafficActive | `lua_is_landing_traffic_active` | entity arg0, vtable+210h() at 0089B060, PushBoolean at 0089B067. |
| 008AB3D0 | MultiSelectUnit | `lua_multi_select_unit` | int arg0 first, entity arg1, 0076CF30(ECX=game+1EF0h, slot, unit) at 008AB528. |
| 008A4960 | PilotCloseToShip | `lua_pilot_close_to_ship` | 0077D600(entity, 00E08FA8 closetoship, 0088A810(arg1), 1) at 008A4AB7. |
| 008A4B10 | PilotStopCloseToShip | `lua_pilot_stop_close_to_ship` | director = entity vtable+114h(); when [director+54h] == 00E08FA8 (008A4C35), 007ED430(ECX=entity, EBX=2) at 008A4C41. |
| 008CB920 | ScoringGrantUnlock | `lua_scoring_grant_unlock` | string arg0 into a stack native string, 004D0640(ECX=game+6CCh, &result, &name) at 008CBA4C, result unused. |
