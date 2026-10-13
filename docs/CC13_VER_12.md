# CC13 verification 12: the Lua script-order natives

Packet cc13-ver12 (Claude, 2026-10-13). This is a static verification pass and makes no game runs.
Campaign missions create no AI group (docs/CC13_FIX_2.md, cc13-fix2b), so every campaign plan reaches its
units through the natives below. Each native's image body (a handler of the script binding table at
00E0B7B8) was compared, from the bytes (`bsp.py disasm-raw`), with the host body that serves it in a run.

The natives were prioritised in three ways:

- the native call summaries in cc9's `lead_ai2_on_*` logs (IJN18, LOMP10, USN12, USN15; 2026-10-13 06:22-06:31);
- the call counts in this installation's `scripts/` tree (`local\v12_luacalls.py`);
- the win order files in config/win_orders.

Those order files are harness player commands, mostly `launch`/`attack`/`order`/`select` (`local\v12_verbs.py`).
They do not call the Lua natives. What the natives drive is the AI side and the scripted allies.

Status words: **checked** means that what the host does in a run was compared to the bytes. A **model** in
src/script_orders_ver_cc13.cpp is reconstructed and build-tested only, and is not wired. Nothing here is
game-validated.

## Verdicts

The call counts are from cc9's logs (IJN18, LOMP10, USN12 and USN15 together), then from the scripts.

| Native | Image | Host (switch) | Calls (logs / scripts) | Verdict |
|---|---|---|---|---|
| PilotSetTarget | 008A4C90 | game_hosts_script_orders.cpp:2058 | 1195 / 1065 | partial: no live gates |
| LaunchSquadron | 0089E3C0 | game_hosts_lua.cpp:2901 | 223 / 212 | match |
| NavigatorSetTorpedoEvasion | 008A3CD0 | lua_binding_navigator.cpp:158 (kNavigatorAvoidanceDeliveryBound) | 161 / 812 | match |
| NavigatorSetAvoidLandCollision | 008A3B10 | lua_binding_navigator.cpp:153 | 161 / 732 | match |
| JoinFormation | 00899D10 | lua_binding_navigator.cpp:257 (kFormationJoinLoopbackBound) | 150 / 1447 | match |
| TorpedoEnable | 0089C8F0 | game_hosts_script_orders.cpp:3407 (kShipDirectorEnablesBound) | 84 / 1095 | match |
| NavigatorAttackMove | 008A30D0 | lua_binding_navigator.cpp:220 | 40 / 1149 | match |
| SetFireTarget | 0089A8B0 | game_hosts_script_orders.cpp:3350 (kScriptFireTargetBound) | 36 / 42 | match (ships) |
| SetShipMaxSpeed | 00890A10 | game_hosts_script_orders.cpp:3332 | 26 / 260 | match |
| SquadronSetSpeed | 0089F780 | game_hosts_lua.cpp:5921 | 24 / 102 | match |
| UnitSetFireStance | 008A6490 | game_hosts_script_orders.cpp:2398 | 17 / 281 | partial: fire-target release |
| SquadronSetTravelAlt / AttackAlt | 0089F550 / 008A22B0 | game_hosts_lua.cpp:6401 / 6424 | 16 / 428, 236 | match |
| NavigatorMoveToPos / MoveToRange / DirectMoveToRange | 008A2BC0 / 008A2F20 / 008A2D70 | lua_binding_navigator.cpp:226 | 15 / 767 | match |
| NavigatorMoveOnPath | 008A3600 | lua_binding_navigator.cpp:87 | 2 / 735 | match (receiver route labelled) |
| SetShipSpeed | 00890D30 | game_hosts_lua.cpp:5814 | 2 / 325 | match |
| NavigatorSetAvoidShipCollision | 008A3970 | lua_binding_navigator.cpp:163 | - / 79 | match |
| NavigatorEnable | 008A7060 | game_hosts_script_orders.cpp:2565 | - / 67 | match |
| PilotMoveTo / MoveToRange / MoveOnPath | 008A4150 / 008A4590 / 008A3E70 | game_hosts_script_orders.cpp:3315 / 2201 / 3367 | - / 287, 278, 86 | match |
| PilotRetreat / PilotLand | 008A4300 / 008A47B0 | game_hosts_script_orders.cpp:2531 / 2490 | - / 143, 66 | match |
| LeaveFormation | 00899EB0 | game_hosts_lua.cpp:6609 | - / 141 | match (ordering note) |
| IsInFormation / GetFormationLeader | 008996A0 / 00899AF0 | game_hosts_lua.cpp:6472 / 6569 | -, 226 / 140, 128 | match |
| SetSelectedUnit | 008AB260 | game_hosts_lua.cpp:576 | - / 908 | match |
| ShipSetTorpedoStock | 0089EEE0 | game_hosts_script_orders.cpp:3644 | - / 143 | match (deferred a step, labelled) |
| UnitHoldFire | 008A6AC0 | game_hosts_script_orders.cpp:2452 | - / 136 | partial: permanent without its release |
| GetSquadronPlanes | 0089CC50 | game_hosts_script_orders.cpp:3517 | - / 63 | partial: empty once wing 0 dies |
| UnitFreeAttack / UnitFreeFire | 008A67E0 / 008A6950 | not hosted | - / 74, 54 | missing |
| NavigatorStop | 008A7400 | not hosted | PRCP01 67 / 140 | missing |
| NavigatorCruise | 008A75C0 | not hosted | - / 10 | missing |
| DisbandFormation / PutFormationTo | 0089A020 / 008AA260 | not hosted | - / 27, 23 | missing |
| RemoveAllAirBasePlanes / LaunchAirBaseSlot | 008975B0 / 00896750 | not hosted | - / 81, 15 | missing |
| StartLanding2 / SquadronSetHomeBase / SquadronSetTravelSpeed | 0089AC30 / 0089E220 / 008A0630 | not hosted | - / 62, 20, 35 | missing |
| LandConvoyStart / Stop / SetSpeed | 00895570 / 008956E0 / 00895850 | not hosted | - / 15 | missing (usn_12_indochina only) |
| GetSquadronPlane, PlaneReloadBombPlatforms, ReloadTorpedoes, SquadronEnableGunfireAvoidance | 0089CFE0, 0089F080, 008C3CE0, 008A01B0 | not hosted | - / 18, 28, 14, 8 | missing |
| SquadronSetReleaseAlt, SquadronForceGunnerMode | 008A24E0, 0089F950 | game_hosts_lua.cpp, OFF | - / 12, 10 | match when ON |
| AISetTargetWeight | 00A38430 | not hosted in the game | - / 422 | multiplayer and skirmish only |

`GroupOrder` is not a native. It is absent from the binding table and from the image's strings, and its only
callers, in commandhelpers.lua luaShipRetreat, are dead.

## Divergences

### 1. UnitFreeAttack and UnitFreeFire are not hosted, so every hold is permanent

**Image.** Each native reads argument 0 through 00888AA0 and takes the director from vt[114h]. It then calls one of:

- 0071BF70 (UnitFreeAttack), which pushes stance 2 at 0071BF7B;
- 0071BF20 (UnitFreeFire), which pushes stance 1 at 0071BF2B.

UnitHoldFire's 0071BED0 pushes 0. All three ask vt[24h] and vt[28h], then send 5Ah sub-kinds 0 and 1 through vt[40h] and vt[44h].

The ship director's predicates are:

- fire, 0071D560: stance 1 or 2;
- move, 0071D580: stance 2 or 3.

The squadron's +348h block has its own predicates:

- fire, 0084D910: stance 1 or 2;
- move, 0084D930: stance 0, 2 or 3.

**Host.** Both names appear only in the mission_lua_host.cpp name table, at lines 232 and 233. unit_weapons.cpp:260
`lua_unit_free_fire_008a6950` has no caller. UnitHoldFire is live (`kScriptedOrderNativesBound`, `kShipFireStanceBound`), and
nothing else writes the gunnery host's `director_stance_by_unit` row. So after a hold, the ship never fires again; a squadron
held through `squadron_permissions_` is likewise never released.

**Default stance.** 008363E0, the director base constructor, writes 1 to +3Ch and +3Dh (0083647B/7E) unless the owner is
kind 0Bh or 9. 008366D0, the ship director constructor, does not write them again. A ship therefore starts with fire 1 and
move 1, which the host's unset row reproduces.

UnitFreeFire on a ship that was never held is therefore not a no-op: it clears +3Dh. That has two readers:

- 009F5610 at 009F5613, called only from AutoTargetTick at 009F5E15, skips the director's own target pick.
- 008624CB copies +3Dh into gunnery+7Ch, which gates torpedo category 7 at 008651B4.

**Where it matters.**

- In this installation there are 288 hold/free calls in 70 files.
- These rows hold and then free:
  - COTP-USN/PRCPUS usn_02_battle_of_cape_esperance: holds at 10 and 14, frees with UnitFreeAttack at 841, 864, 1263, 1387 and 1452.
  - usn_10_battle_of_capeengano: holds at 64-70, frees with UnitFreeFire at 927-933.
  - LOMP08: holds at 440 and 1021, frees at 677, 680 and 1072.
  - COTP-IJN jm05, jm12 and jm13.
  - BSM02, BSM04, BSM10 and BSM11.
- Of the win rows, only PRCP01 (PRCPUS usn_03_battle_of_santa_cruz) calls any of them.
  - At 1447-1448 it runs UnitFreeFire on every own unit; its hold at 1369 is inside a `--[[` comment.
  - At 938 it runs UnitFreeFire on the strike planes once the Hornet is dead.
- USN19 only holds.

### 2. NavigatorStop is not hosted

**Image.** Argument 0 goes through 00888AA0 at 008A74FE. There is no null test. The descriptor is all zero apart from the vector
read from 00F87574..7C (008A7516-008A7563). The native then calls 0077D600(entity, stop 00E08F88, &descriptor, 1) at
008A7569 and pushes nothing.

**Host.** NavigatorStop is a name row only, and the PRCP01 log records `UNIMPLEMENTED calls=67`.

**Where it matters.** There are 140 calls in 58 files, including commandhelpers.lua:6882 luaEnableNavigator(e, false).

In PRCP01, 66 of the 67 calls come from luaUS03CarrierMoveHack: a stop followed at once by NavigatorMoveOnPath, every 5 s. The
next order masks those. Usn_03:1918 NavigatorStop(Mission.Hornet) in luaUS03AddHornetObj has no following order. In the image
the Hornet takes `stop` when the Hornet-kill objective starts; in the host it keeps sailing.

### 3. UnitHoldFire and UnitSetFireStance do not release the fire target

00836210, the 5Ah sub-kind-0 receiver, stores +3Ch. When fire is forbidden it also releases the director's fire target at
+238h (0083621B-00836230). The host stores only `allow_fire` (game_hosts_gunnery.cpp:13488) and labels the release NOT DONE
at game_hosts_script_orders.cpp:2418.

This is rare. Most scripts clear the target with `SetFireTarget(x, nil)` first. The file is cc9's gunnery lane, so this is
an adoption note only.

### 4. PilotSetTarget skips the native's live tests

**Image.** The native tests the unit at 008A4DA8-008A4DD2, before argument 1 is read, and tests the resolved target at
008A4E70-008A4E8A. Each test is [+5Ch] != 0 && [+5Dh] == 0 && [+60h] == 0 && [+5Eh] == 0. A position-only target resolves
to null, and the native refuses it.

The second 007EEC50 call at 008A4E99 has the same three arguments. 007EEC50 writes nothing, so the second call repeats the
first answer.

**Host.** run_pilot_set_target issues whenever 007EEC50's model chooses a class, and that model checks only target +5Dh. The
host kill funnel does set +60h (game_hosts_units.cpp:17384, 00926CD5). So a sinking ship stays a valid target in the host
where the native refuses it.

**Measured.** In IJN18 `lead_nav_on`, 3 of the 1183 calls target a ship whose classify line shows y < -3 m. The impact is low.
The faithful body is already modelled in pilot_order_bindings.cpp `pilot_set_target_008a4c90`.

### 5. GetSquadronPlanes empties once the wing-0 plane dies

The image lists the +174h id strings of `[sq+3D0h]` for i < +3CCh, so the survivors are still listed after a death. The
host finds the squadron with `find_by_member_unit` (game_hosts_script_orders.cpp:3531). The squadron is fused into its
wing-0 plane, and that plane is erased from `member_units` when it dies, so the call then answers `{}`.

`find_by_member_or_departed_unit` exists for this case. The scripts affected are jm13, LOMP08, bsm_04 and the siege maps.
Address 0089CC50 is leased to cc13-fix5, so it has no ledger record here.

### Labelled residuals (not divergences of the native)

- NavigatorMoveOnPath and PilotMoveOnPath deliver `moveonpath` through the host's 0077D600 path with flags 0. The image instead
  routes a 5Bh message whose receiver (00721A9D) calls director vt[60h] 008358D0. Whether both leave the same slot state was
  not traced.
- LaunchSquadron takes argument 3 when argc >= 4; the image takes it only when argc == 4.
- LeaveFormation runs at once while JoinFormation is queued. A join followed by a leave in the same frame therefore ends in
  the group in the host, and alone in the image.

## Adoption notes

Two fixes were applied in this packet, while it held the lease on game_hosts_script_orders.cpp after cc9-lua52 released it.
Both switches are OFF. With a switch OFF, `handles()` answers false and the native stays an unimplemented record, so the run
is byte-identical.

- **9ad5a9bbf `kLuaUnitFreeFireBound`** binds UnitFreeAttack and UnitFreeFire through `run_unit_free_fire`, using
  UnitHoldFire's route with stance 2 or stance 1. It does not bind unit_weapons.cpp's `lua_unit_free_fire_008a6950`,
  because that function runs over `UnitWeaponHost` and no game host implements that interface.
- **fce82e529 `kLuaNavigatorStopBound`** binds NavigatorStop through `run_navigator_stop`, which calls
  `entity_issue_command(entity, kCommandStop, zeroed descriptor, 1)`.

The predictions are in the commit messages and are summarised here:

- **PRCP01.** At 1447-1448, the own ships lose +3Dh. They stop picking their own targets and stop firing torpedoes; guns
  still fire at targets set by script or command.
- **PRCP01.** At 1918, the Hornet halts when the Hornet-kill objective starts.
- **Hold->free.** The PRCPUS usn_02_battle_of_cape_esperance pair lets the Japanese ships fire again after
  UnitFreeAttack.

Adoption notes for files this packet does not own:

- **game_hosts_gunnery.cpp (cc9 gunnery lane).** `set_director_fire_stance_0071be80` should release the director fire target
  (+238h) when the stance forbids fire (00836210 at 0083621B-00836230).
- **game_hosts_script_orders.cpp `run_pilot_set_target`.** Gate on `units_.unit_alive_and_visible(unit)` before argument 1
  is read, and on the target object after the chooser, as `bsp::pilot_set_target_issues_008a4c90` models it.
- **game_hosts_script_orders.cpp:3531, GetSquadronPlanes.** Use `find_by_member_or_departed_unit`. The address is leased
  to cc13-fix5.
- **Unhosted natives.** These have the same shape as NavigatorStop and should get their own switches:
  - NavigatorCruise: `cruise` 00E08F70, a zeroed descriptor and flags 1, at 008A7729.
  - DisbandFormation 0077CA60: a 77h message to every member.
  - LandConvoyStart, LandConvoyStop and LandConvoySetSpeed: 00741120/00741130 write [convoy+3A8h], and 00741140 writes
    [+368h]. The host already models both fields.
  - RemoveAllAirBasePlanes: 006C8840.
  - StartLanding2: vt[238h]; the result is pushed.

## Ledger

Each checked native has a reconstruction record named `ver12_check_<Native>`, pointing at its host body:

- status `host_checked_against_bytes`, coverage `full`, for a match;
- coverage `partial`, with the divergence, for a divergence or a native that is not hosted.

## Handoff

Status:

- The table is checked against the bytes. The two fixes are build-tested and ctest passes; nothing is game-validated.
- The three helper reports that the table draws on, labelled v12a, v12b and v12c, were read-only. They were spot-checked
  against these bytes:
  - 008A4DA8-008A4EAC;
  - 008A74DD-008A7569;
  - 0071BED0-0071BF7D;
  - 0071D560/0071D580;
  - 0084D910/0084D930.

Not done:

- The receivers below the natives were not traced. These are:
  - the 5Bh path route against the host's 0077D600 flags-0 route;
  - 006CC690;
  - 0081F8B0;
  - 007F1C00.
- PutFormationTo's body past the call shape was not read.
- NavigatorCruise, DisbandFormation, LandConvoy*, RemoveAllAirBasePlanes and StartLanding2 are still unhosted.

Next steps:

- cc9 pairs PRCP01 and PRCPUS usn_02 with each switch ON.
- The next packet should host NavigatorCruise and DisbandFormation, which are the cheapest. After them, RemoveAllAirBasePlanes,
  which is called on the JM08 and PRCP01 rows.
