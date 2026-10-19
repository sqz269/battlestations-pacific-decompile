# CC13 adoption triage: defect_found, minor_deviation and host_substitution_labelled rows

This triage is read-only (packet cc13-seg51c, 2026-10-19). It ranks the ledger rows whose status
says the existing code differs from the image, so cc9 can pick its next adoption batches. No code
changed and nothing was run.

## Method

1. **Rows.** I took the last record per address across every `config/reconstruction` shard, at
   main 0e4f5fd21. 339 rows carry one of the three statuses:
   - 25 `defect_found`;
   - 78 `minor_deviation`;
   - 236 `host_substitution_labelled`.

   Rows whose source is one of cc13's own unwired files (`*_cc13.cpp`, `cc13_*`, `band_*`) are
   dropped, as are presentation-only subjects (HUD, camera, menu, scene contents, settings text,
   draws). Both lists are in Appendix C.

   Rows inside the struck ranges are also dropped, because those ranges are cc9's live packets:
   007DF000-007DFFFF, 007EE000-007F2FFF, 00836000-00837FFF and 0084E000-0084EFFF. They are
   listed under "Struck" in Appendix C. A struck function may still be cited as liveness
   evidence for another row (007F16D0 and 007F1C00 for #4 and #5).

   One exception stays ranked: 007EE7F0 (#2). cc9 granted it to cc13 on 2026-10-19 for the
   +3ECh store only.
2. **Liveness.** The evidence is the unimplemented-call census at the end of each game log. I used
   the 24 newest `*_main_ref*` and `*_off_*` logs of the last 36 hours, read-only, from the cc9
   trees (listed in Appendix C). Every census address is mapped to the function that contains it,
   and the "calls" column sums the calls over the logs. A row with no census line is not
   necessarily unreached: a host arm that records nothing leaves no line. For that reason I
   checked the callers of the defect rows by hand.
3. **Staleness.** For every ranked row I read the current host code at the cited line and the
   values of the switches it names. Rows are marked **STALE** where a later packet already fixed
   them behind a switch that is ON. In those cases only the ledger text is out of date.

"Runs" uses the log tags: `u1a`..`u1e`, `j8h`, `i08h`, `w_e8`, `w_l10`, `w_i18`, `u12e8` and
`u02a` are cc9-lua52 main reference runs; `g53_*`, `p23c_*` and `s59_*` are OFF runs in the
gunnery52, physics1 and ships44 trees.

## Tier 1: defect_found, LIVE

| # | Address | Host file and function | Defect (from the ledger, rechecked) | Switch / state | Live evidence | Runs |
|---|---|---|---|---|---|---|
| 1 | **007C0D90** (URGENT class: torpedo and level-bomb issue) | game_hosts_units.cpp ~21825, `TorpedoReleaseOrderBinding::unit_carries_droppable_device_007c0d90` (logged as `Plane::droppable_device_walk`, UNIMPLEMENTED) | The device walk is reduced to "the slot's ordnance mask holds a torpedo (or bomb, or depth charge) with rack ammo". It drops the vslot +1F0h call, the 33h early return (no release orders for that descriptor) and the per-device **stream-1 draw** for kind-10h planes with other ordnance. | Labelled in the host ("the six vtable slots are contract: unread"); kReleaseIssueStageBound and kReleaseIssueStageValsBound shape it. No switch restores the draw. | 1,858 calls in 17 logs | u1a-u1e, j8h, i08h, w_e8, w_l10, u12e8, g53 j5/u1, p23c b9, s59 u01 |
| 2 | **007EE7F0** (GRANTED to cc13 by cc9 2026-10-19 for the +3ECh store only) | game_hosts_units.cpp ~21869, `pre_issue_hook_007ee7f0` (`PilotControl::pre_issue_hook`) | The image first stores ctl+3ECh = 0. The host says "ctl+3ECh is not carried by this host". **Settled 2026-10-19 (cc13-devwalk): recorded only, as the squadron's dirty byte, which has no reader.** ctl is the squadron ([unit+9D4h]), and 007EE802 is a BYTE store. It is not the pilot task's float gunfire rest-end timer at task+3ECh (seeded at 0099C7D3). No known effect, so nothing is wired. | No switch (none needed) | 788 calls in 17 logs | same as #1 |
| 3 | **006C0DA0**, **006CDC70** | air_operations.cpp:765 / :837 (`AirOps::update_slots`, `AirOps::update`) | The slot walk counts dirty and became_ready instead of calling 00696350 NotifySlot0C, 006BD520 (slot message) and the broadcast. 006CDC70 calls only 006C0DA0; 006C5B70, 006C6540, 006C77E0, 006CD240 and 006CD810 run elsewhere or not at all. The game-state gate is not modelled. | No switch; still as described | 564,912 calls in 23 logs (every frame) | all 23 |
| 4 | **006C7490** | air_operations.cpp:526, called from game_hosts_lua.cpp:7554 and game_hosts_script_orders.cpp:2248 | It writes the slot state before building the squadron and never stores a null squadron. It omits 00922F30 and 006BF150, and adds a 006CC7B0 assign-queue push (labelled as the 007F1C00 path). | No switch | Live through its callers: 006C5050 BuildLaunchSquadron has 301 calls in 17 logs | carrier missions |
| 5 | **006C5950** | air_operations.cpp:795, called from game_hosts_units.cpp:16323 | It does not erase the squadron from the block+98h record array (006BF7F0 under the +4h lock), does not unregister the observer pair, and sends none of the per-slot 00696350 / 006BD520 / 0077C7B0. It also adds a null-squadron early return. | No switch | Live through the return-to-base path (007F16D0, 63 calls in 11 logs) | carrier missions |
| 6 | **00846D90** (+ 008436F0, 00844D60) | shipyard_production.cpp:170/157/131, called from game_hosts_gunnery.cpp ~14785-14806 | Order entry omits the observer move, the 00844610 count and the 00984EB0 stock warning, the **00844CE0 free hangar plus 00844FC0 CreateLaunchedUnit**, and the session broadcast. The other two omit only the session message A8h broadcast. | No switch. Check whether the gunnery host launches the unit elsewhere before adopting. | 24 calls in 2 logs (`Shipyard::order`) | j8h, j8h2 |
| 7 | **008C1930** OverrideHP | game_hosts_gunnery.cpp `override_hp_008c1930` | `if (!kLuaOverrideHpBound \|\| state.dead) return;` skips a dead or sinking unit, while the image still writes +36Ch and calls 00877B90. | Still present | 18 calls in 2 logs | p23c b9, w_l10 |
| 8 | **00805CF0** | recon_sensor_pass.cpp:163, called by SetForcedReconLevel | It keeps the level in a global (side, target) map and does not unregister the +28h observer. The change notify is now made by the caller under kForcedReconSyncNotifyBound (ON), so this is partly stale. | Partly fixed | Through 008AA8F0: 195 calls in 8 logs | |

**STALE defect rows** (fixed by later packets; only the ledger text needs a refresh):
- **0089B840** GetCapturePercentage: FABS is applied under `kLuaCapturePercentageFabsBound` = true (game_hosts_script_orders.hpp:528).
- **008AA8F0** SetForcedReconLevel: the entity's own record is forced first under `kForcedReconOwnRecordBound` = true (game_hosts_lua.hpp:212).

**Unreached defect rows:**
- **009C8920** dive-bomb cruise profile: no host calls it. The host's own dive-bomb cruise arm should be checked against the same text (leader +394h = tuning+4C0h and the U(0,15) stream-1 draw). This is dive-bomb, so it is cc9's priority area.
- **006C56D0** arrive squadron: no caller at all.

## Tier 2: minor_deviation, LIVE (host or wired subject)

| Address | Subject | Deviation | Calls (logs) | Verdict |
|---|---|---|---|---|
| 00864680 | `Gunnery::line_of_sight_00864680`, game_hosts_gunnery.cpp | The 0.5 s eye-point cache and the shape-based height arm are not modelled: the eye is recomputed on every call. | 248,025 (23) | Gameplay: a cached eye can differ for up to 0.5 s. Candidate. |
| 009FC1C0 / 009FC000 | LegAltitudeManager remove/register (cc13 source, reached through the units host's `leg_manager_cc13_` arm, game_hosts_units.cpp ~8013, with kLegAltitudeManagerBound ON) | The critical section is omitted. | 3,401 / 3,137 (one log) | No effect, single thread. Relabel as verified. |
| 007ECF80 | squadron broadcast to members | Null members are skipped and the count is capped (added guards). | 327 | Only an error path. Low. |
| 007C9540 | `set_unit_world_basis_007c9540`, game_hosts_units.cpp | The translation of m is not applied. Its callers pass a pure rotation. | 112 | Unobservable with the current callers. Low. |
| 0045F520 / 0045F440 | call_entry_point_if_defined | Dispatch by host test. | 69 / 50 | No difference. Relabel. |
| 009E9EB0 | sub-attack fire step | "Inherits 009E4A60's range defect" | 10,329 (5) | **STALE**: 009E4A60 is fixed under `kSubAttackOwnHalfLengthBound` (ON). The rest (double vs x87) is precision only. |

**STALE minor rows** carrying a "Note for cc9" that is now fixed:
- 009A75C0, 009A8560 and 009A7DE0: the dogfight enter draws, taken under `kDogfightEnterDrawsBound` = true (game_hosts_units.cpp:6699).
- 009C83E0 and 009C7800: the dive-bomb turn coin, drawn lazily under `kDiveTurnSideCoinBound` = true (:7881).

**Gameplay minor rows with no census line**, worth a read before adoption:
- **007BC7A0**, the player bomb/rocket release gate: the host's comment states the rocket pass as +E0h > 0 where the binary passes when +E0h <= 0. This is the player's rockets.
- **0072CDD0**: the line-of-fire landscape query starts at the unraised muzzle; an adoption note already exists.
- **007B8AE0** set skill level: the dead-unit skip is absent.
- **00827B90** failure effects: names are compared case-sensitively where the image uses stricmp.
- **009C7F00** dive goaway complete: the term is kept in double. Goaway, precision only.
- **009D0160 / 009D1360 / 009D1500** torpedo run times: added zero-divide guards only.

## Tier 3: gameplay-changing substitutions, LIVE, grouped into mechanisms

Each group is a coherent batch of 3-5 switches. Calls are census totals.

| Batch | Rows (address, calls) | Why it changes gameplay |
|---|---|---|
| **A. Air-ops slot notifications** | 006C0DA0 + 006CDC70 (Tier 1 #3), 006C5B70 release brake (564,912), 006CD240 landing queue (564,912), 006C5050 build launch squadron (301), with 006C7490 / 006C5950 (Tier 1 #4, #5) | Launch, recovery and stock bookkeeping on every carrier and airfield: brake release order, landing queue order, the squadron record array. |
| **B. Plane attack-task ticks** (URGENT class: level-bomb) | 009FC7C0 dogfight gun tick (3,890,100, UNIMPLEMENTED: the fire envelope is a simplification, missing the 007B96F0 target find, 0043F080 gate, 007BA760 fire-blocked and 007B96D0 friendly-in-line checks), 009A76E0 dogfight aim tick (73,964), 009B7C90 level-bomb approach update (190,699; the 007B9500 kind-31h paratrooper branch is not modelled), 009B5C80 level-bomb aim tick (68,636), 009C3100 moveto arrival end (22,225) | Fighter firing decisions and the level-bomb approach and aim, in every air mission. |
| **C. Plane ground and deck states** | 007C11E0 wing fold notify (955, UNIMPLEMENTED), 007CC820 enter state 1 (950), 007CA3F0 ground state from locked (929), 007C1680 five-to-four (270, UNIMPLEMENTED), 007C71E0 deck reparent / 007CB5F0 touchdown / 007CC7A0 state 2 / 007CC8B0 stock return (40 each), 007C5F60 place on launch spot (1,007), 006D2510 airfield tick advance (429,737, UNIMPLEMENTED) | The take-off and landing chain cc9 is debugging (land / park / re-takeoff). |
| **D. Squadron membership and placement** | 0077BD70 leave group on destroy (434; the director notice vtable[114h]->[5Ch] is not made), 008193A0 place at world position (37), 008A9F90 PutTo (37), 008A0A10 EntityTurnToEntity (54) | Leader succession and scripted placement. |
| **E. Lua listeners and finders** | 00988510 hit listeners (5,203; the targetDevice/fireCaused/leakCaused filters are not matched and callbacks get no argument), 009813A0 kill listeners (2,303), 00982120 exit zone (153), 00898E30 FindEntity (3,639; case-fold), 0088BF80 (14,843) | Mission scripting outcomes, which can decide a win. |
| **F. Air base and catapult natives** | 00896A90 AddAirBaseStock (23), 008963E0 SetAirBaseSlotCount (18), 00892DE0 GetCatapultStock (18), 00892860 GetLastCatapulted (213), 00893C00 SetUnlimitedAirSupply (6), 008975B0 RemoveAllAirBasePlanes (2) | Scripted air-support supply. |
| **G. Ship damage and failure** | 008198A0 set failure from message (97, UNIMPLEMENTED), 0081DE10 ship in shipyard hangar (22,192), 008227E0 damage smoke tick (131M; its draws are ON and only the effects are recorded, so it is presentation apart from RNG order) | Failure state and hangar protection. |

Not batched:
- Checkpoint natives 008ACA30 / 008ACB70 are UNIMPLEMENTED with `kMissionCheckpointBound` = false.
- 00968550 warning deadlines (564,912) has an empty evidence field.
- 0081BEB0 exit mission zone is cc13's exit-zone wiring, already in progress.

**Mislabelled rows.** These carry `host_substitution_labelled` but their evidence says "No mismatch
seen". They should be relabelled verified rather than adopted:
- 00863840, 008638B0 and 00863920, the ship gunnery-AI group accept-target functions (108,257 / 90,945 / 49,958 calls).
  ASSUMPTION, unchecked: 00863840's evidence takes vtable[4] = 00861BE0. Confirm that slot before
  relabelling 00863840 as verified.

Not a relabel: 00933BB0, 00933DA0 and 00935540, the sink effects, are a genuine presentation
substitution. The draws match, but the point effect is recorded rather than spawned. They are
presentation-only and keep their `host_substitution_labelled` status.

## Tier 4: unreached

Appendix B lists every kept row with no census line. Most of it is the standalone task files:
- dive_bomb_task.cpp, plane_retreat_task.cpp, strafe_task.cpp and torpedo_approach_update.cpp,
  whose host arms are written in game_hosts_units.cpp;
- the air-ops and shipyard helpers.

Those deviations matter only if a host arm still copies the deviating text, so they rank last.

## Suggested order for cc9

1. **B** (fighter fire envelope and level-bomb), together with Tier 1 #1 (007C0D90; #2, 007EE7F0, is
   recorded only). This is the torpedo/level-bomb issue path, so it is URGENT class.
2. **C** (ground and deck states), because cc9 is debugging land, park and re-takeoff now.
3. **A** (air-ops notifications) with Tier 1 #3-#5.
4. **E** (Lua listeners). These can change mission wins.
5. **D**, **F**, **G**, then Tier 1 #6-#8 and the Tier 2 candidates (00864680, 007BC7A0).

## Appendix A. Live substitutions not ranked above (census calls > 0)

| Address | Host function / name | Source | Calls (logs) |
|---|---|---|---|
| 008227E0 | GameGunneryHost::damage_smoke_tick_008227e0 | game_hosts_gunnery.cpp | 131208816 (23) |
| 009FC7C0 | dogfight_gun_tick_009fc7c0 | dogfight_task.cpp | 3890100 (21) |
| 006C5B70 | GameUnitsHost::Impl::release_launch_brake_006c5b70 | game_hosts_units.cpp | 564912 (23) |
| 006CD240 | GameUnitsHost::run_landing_queue_006cd240 | game_hosts_units.cpp | 564912 (23) |
| 00968550 | BSP_WarningManager_UpdateDeadlines | mission_events.cpp | 564912 (23) |
| 006D2510 | BSP_AirField_TickAdvance | game_hosts_units.cpp | 429737 (15) |
| 00954A10 | GunAimMessage79 (game_hosts_gunnery.hpp:477) | include/bsp/game_hosts_gunnery.hpp | 217278 (22) |
| 009B7C90 | level_bomb_approach_update_009b7c90 | plane_level_bomb_task.cpp | 190699 (8) |
| 00863840 | BSP_UnitGunneryAi_ArtilleryGroupAcceptsTarget | game_hosts_ship_ai.cpp | 108257 (12) |
| 008638B0 | BSP_UnitGunneryAi_DepthChargeGroupAcceptsTarget | game_hosts_ship_ai.cpp | 90945 (12) |
| 009A76E0 | dogfight_aim_tick_009a76e0 | dogfight_task.cpp | 73964 (10) |
| 009B5C80 | level_bomb_aim_tick_009b5c80 | plane_level_bomb_task.cpp | 68636 (8) |
| 00933BB0 | BSP_ShipSystems_SinkSprayEffect | game_hosts_gunnery.cpp | 62934 (18) |
| 00863920 | BSP_UnitGunneryAi_TorpedoGroupAcceptsTarget | game_hosts_ship_ai.cpp | 49958 (11) |
| 009C3100 | GameUnitsHost::Impl::moveto_arrival_end_command_009c3100 | game_hosts_units.cpp | 22225 (6) |
| 0081DE10 | GameGunneryHost::Impl::ship_in_shipyard_hangar_0081de10 | game_hosts_gunnery.cpp | 22192 (23) |
| 0088BF80 | FUN_0088bf80 | game_hosts_lua.cpp | 14843 (11) |
| 00988510 | GameMissionLuaHost::dispatch_hit_listeners_00988510 | game_hosts_lua.cpp | 5203 (23) |
| 00898E30 | BSP_LuaBinding_FindEntity | game_hosts_lua.cpp | 3639 (23) |
| 009813A0 | GameMissionLuaHost::dispatch_kill_listeners_009813a0 | game_hosts_lua.cpp | 2303 (22) |
| 00933DA0 | BSP_ShipSystems_SinkGridEffects | game_hosts_gunnery.cpp | 1811 (18) |
| 007C5F60 | GameUnitsHost::Impl::place_on_launch_spot_007c5f60 | game_hosts_units.cpp | 1007 (17) |
| 007C11E0 | GameUnitsHost::Impl::wing_fold_notify_007c11e0 | game_hosts_units.cpp | 955 (19) |
| 007CC820 | GameUnitsHost::Impl::plane_enter_state_one_007cc820 | game_hosts_units.cpp | 950 (17) |
| 007CA3F0 | GameUnitsHost::Impl::ground_state_from_locked_007ca3f0 | game_hosts_units.cpp | 929 (17) |
| 008942C0 | BSP_LuaNative_GetSubmarineOnSurface | game_hosts_lua.cpp | 720 (1) |
| 0077BD70 | GameUnitsHost::leave_group_on_destroy_0077bd70 | game_hosts_units.cpp | 434 (18) |
| 006C5050 | BSP_AirOps_BuildLaunchSquadron | game_hosts_script_orders.cpp | 301 (17) |
| 007C1680 | GameUnitsHost::Impl::plane_five_to_four_007c1680 | game_hosts_units.cpp | 270 (7) |
| 00892860 | BSP_LuaBinding_GetLastCatapulted | game_hosts_lua.cpp | 213 (3) |
| 008CD440 | BSP_LuaObjectives_Add | game_hosts_lua.cpp | 197 (23) |
| 00935540 | BSP_UnitInstance_SpawnSinkingWreckDebris | game_hosts_gunnery.cpp | 164 (18) |
| 00982120 | GameMissionLuaHost::note_exit_zone_00982120 | game_hosts_lua.cpp | 153 (3) |
| 008198A0 | BSP_ShipInstance_SetFailureFromMessage | game_hosts_gunnery.cpp | 97 (19) |
| 008996A0 | BSP_LuaBinding_IsInFormation | game_hosts_lua.cpp | 90 (4) |
| 008A0A10 | BSP_LuaBinding_EntityTurnToEntity | game_hosts_script_orders.cpp | 54 (4) |
| 006CADD0 | air_ops_load_from_scene_006cadd0 | air_operations.cpp | 53 (19) |
| 007C71E0 | GameUnitsHost::Impl::carrier_deck_reparent_007c71e0 | game_hosts_units.cpp | 40 (7) |
| 007CB5F0 | plane_on_touchdown_from_flight_007cb5f0 | game_hosts_units.cpp | 40 (7) |
| 007CC7A0 | GameUnitsHost::Impl::plane_enter_state_two_007cc7a0 | game_hosts_units.cpp | 40 (7) |
| 007CC8B0 | GameUnitsHost::Impl::plane_stock_return_c7h_007cc8b0 | game_hosts_units.cpp | 40 (7) |
| 008193A0 | GameUnitsHost::place_at_world_position_008193a0 | game_hosts_units.cpp | 37 (7) |
| 008A9F90 | BSP_LuaBinding_PutTo | game_hosts_script_orders.cpp | 37 (7) |
| 008B01B0 | BSP_LuaGame_EndScene | game_hosts_script_orders.cpp | 24 (12) |
| 00806B10 | GameMissionLuaHost::publish_recon_slot_tables_00806b10 | game_hosts_lua.cpp | 23 (23) |
| 00896A90 | BSP_LuaBinding_AddAirBaseStock | game_hosts_lua.cpp | 23 (8) |
| 00892DE0 | BSP_LuaBinding_GetCatapultStock | game_hosts_lua.cpp | 18 (1) |
| 008963E0 | BSP_LuaBinding_SetAirBaseSlotCount | game_hosts_lua.cpp | 18 (9) |
| 00899500 | BSP_LuaBinding_TrulyDead | game_hosts_lua.cpp | 15 (14) |
| 008AC140 | BSP_Lua_AddUntouchableUnit | game_hosts_lua.cpp | 14 (4) |
| 00849A30 | shipyard_scene_attach_00849a30 | shipyard_production.cpp | 13 (7) |
| 00849F70 | shipyard_complete_slots_00849f70 | shipyard_production.cpp | 13 (7) |
| 006D40F0 | GameUnitsHost::Impl::run_airfield_destruction_rule_006d40f0 | game_hosts_units.cpp | 10 (2) |
| 0088F1D0 | FUN_0088f1d0 | game_hosts_lua.cpp | 10 (3) |
| 00893F40 | BSP_LuaBinding_SetSubmarineDepthLevel | game_hosts_lua.cpp | 8 (3) |
| 0092BD70 | GameUnitsHost::set_hull_class_bit_0092bd70 | game_hosts_units.cpp | 7 (4) |
| 00893C00 | BSP_LuaBinding_SetUnlimitedAirSupply | game_hosts_lua.cpp | 6 (1) |
| 0089C740 | BSP_LuaBinding_AAEnable | game_hosts_lua.cpp | 6 (5) |
| 0081BEB0 | BSP_UnitInstance_ExitMissionZone | game_hosts_units.cpp | 5 (2) |
| 008ACA30 | BSP_LuaBinding_LoadCheckpoint | game_hosts_lua.cpp | 4 (4) |
| 0088EFF0 | FUN_0088eff0 | game_hosts_lua.cpp | 3 (2) |
| 00896CC0 | BSP_LuaBinding_AddShipyardStock | game_hosts_lua.cpp | 2 (1) |
| 008975B0 | BSP_LuaBinding_RemoveAllAirBasePlanes | game_hosts_lua.cpp | 2 (2) |
| 008A7200 | BSP_LuaBinding_NavigatorForceTorpedo | game_hosts_script_orders.cpp | 2 (1) |
| 008ACB70 | BSP_LuaBinding_SaveCheckpoint | game_hosts_lua.cpp | 1 (1) |

## Appendix B. Not in any census (no call recorded in the 24 logs)

No census line falls inside these functions. That is not proof they are unreached: a host
arm without a `done`/`unimplemented` record leaves no census line. Grouped by source file.

| Source | defect_found | minor_deviation | host_substitution_labelled |
|---|---|---|---|
| game_hosts_units.cpp | - | 007B8AE0, 007BC7A0, 009CAD00 | 004A9BD0, 006082D0, 006BED30, 006BEE40, 006C0750, 006C0840, 006C0D20, 006C4B30, 006C54C0, 006C7680, 006C8800, 006CC9F0, 006CE230, 006CED90, 006CEF80, 006CF180, 006CF190, 006CF520, 006CF730, 006CF980, 006CFFC0, 006D0050, 006D02F0, 006D0600, 006D0930, 00759120, 007B58D0, 007B9140, 007B96D0, 007BCE20, 007BCFA0, 007C16F0, 007C3C90, 007C47F0, 007C5AC0, 007C6F50, 007C7600, 007CBA50, 007DEDB0, 007E2090, 007EABC0, 00811E80, 00811F50, 00812CD0, 008160B0, 00816410, 00827FB0, 0099F1C0, 009B41C0, 009CDD50, 009CE1D0 |
| game_hosts_gunnery.cpp | - | 0072CDD0, 00731E20, 00827B90 | 006D2980, 006D3250, 0072C970, 0072DD20, 007311B0, 00813660, 008137B0, 00818110, 0081DCB0, 0085DF60, 00862660, 0098B130, 009FA7E0 |
| game_hosts_lua.cpp | - | 00949750 | 006C6630, 006C7E20, 006D0E60, 008135C0, 00895570, 008956E0, 00895850, 00896750, 008BD900, 008F8680, 0094C490, 00968710, 00980C10, 00980E50 |
| air_operations.cpp | 006C56D0, 006C5950, 006C7490 | 006BD460, 006BF230, 006C0F00 | 006BED60, 006C4F70, 006C58A0, 006C64B0, 006C65B0, 006CC7B0, 006CCDA0 |
| dive_bomb_task.cpp | 009C8920 | 009C7800, 009C7F00, 009C83E0, 009C8790, 009C8A90 | 009C4220, 009C44F0, 009C8200 |
| game_hosts_script_orders.cpp | - | 00735100 | 00734140, 00734FA0, 00894100, 008ADE00 |
| game_hosts_ship_ai.cpp | - | - | 00810DD0, 00810F60, 00812090, 0082ADA0, 0082ADC0, 00864BA0 |
| plane_retreat_task.cpp | - | 00489C40, 009C8D40, 009C9FB0, 009F9D90 | 009C9310 |
| dogfight_task.cpp | - | 007B4ED0, 009A75C0, 009A7DE0, 009A8560, 009BE3E0 | - |
| game_hosts_avoid_zones.cpp | - | 0041BAE0, 0041DF40 | 00424680 |
| weapon_director.cpp | - | - | 00835940, 008359C0, 00835A40 |
| shipyard_production.cpp | 008436F0, 00844D60 | 0084ACB0 | - |
| lua_spawn_new.cpp | - | 009439B0, 00949530 | 0094A140 |
| strafe_task.cpp | - | 00996300, 00996510, 009CCED0 | - |
| torpedo_approach_update.cpp | - | 009D0160, 009D1360, 009D1500 | - |
| native_settings_loader.cpp | - | - | 00BFB483, 00BFB636, 00BFB72B |
| torpedo_task_arm.cpp | - | - | 009D2570, 009D25A0 |
| native_lua_reader.cpp | - | 00442220 | - |
| building_pads.cpp | - | - | 006AC490 |
| collision_shapes.cpp | - | 006D1400 | - |
| airfield_taxi.cpp | - | 006D2780 | - |
| plane_wanderer.cpp | - | - | 007C4560 |
| torpedo_release_orders.cpp | - | - | 007ED3C0 |
| recon_sensor_pass.cpp | 00805CF0 | - | - |
| ship_ai_goal_vector_visibility.cpp | - | - | 00806480 |
| unit_kind_query.cpp | - | 0080F9A0 | - |
| ship_ai_bearing_rating.cpp | - | 0085A8B0 | - |
| warning_owner.cpp | - | - | 0097BBC0 |
| ship_ai_sub_attack.cpp | - | - | 009E9CE0 |
| plane_flight.cpp | - | 009F9ED0 | - |
| gun_aim_terms.cpp | - | 009F9FC0 | - |
| approach_target_ref.cpp | - | - | 009FB200 |
| ai_command_tick.cpp | - | 009FDF30 | - |
| native_game_class_cleanup.cpp | - | - | 00BF67A7 |
| native_game_settings_owner.cpp | - | - | 00BF6FF5 |
| native_scene_token_values.cpp | - | - | 00BF7533 |
| native_game_array_lifetime.cpp | - | - | 00BF7C6E |
| native_game_array_elements.cpp | - | - | 00BF7CD1 |
| native_settings_choices.cpp | - | - | 00BF7FBF |
| native_dyn_sap_lifetime.cpp | - | - | 00BF9DC8 |
| native_dyn_narrow_phase.cpp | - | - | 00BF9F1A |

## Appendix C. Dropped rows

**Struck (cc9 live packets)**, dropped by the Method rule. Each is shown with its census calls in
the 24 logs:

- 007F1F00 GameUnitsHost::Impl::base_launch_task_tick_007f1f00 (game_hosts_units.cpp), 1,226 calls (17 logs)
- 007F2920 GameUnitsHost::place_squadron_members_007f2920 (game_hosts_units.cpp), 340 calls (21 logs)
- 007F16D0 GameUnitsHost::Impl::record_return_to_base_007f16d0 (game_hosts_units.cpp), 63 calls (11 logs).
  It is still used above as liveness evidence for #5.
- 007F15F0 and 007F1940 (game_hosts_script_orders.cpp), with no census line
- 007F23A0 (plane_formation.cpp), with no census line
- 007DF4F0 and 0084E010 (game_hosts_units.cpp), with no census line (formerly in Appendix B)

**cc13's own unwired files.** I re-ran the census over the same 24 logs on 2026-10-19 and none
of these has a census line. 009FC000 and 009FC1C0 (leg_altitude_manager_cc13.cpp) were listed
here before. They are wired through the units host and live, so they now stay in Tier 2:

- band_007a_cc13.cpp: 007B83F0, 007B8D10, 007B8DA0
- band_0099_cc13.cpp: 0099C690, 009DA9D0
- cc13_seg87.cpp: 00B65AC0
- gameplay_seg48_cc13.cpp: 007E2600
- kamikaze_task_cc13.cpp: 009AB920, 009ABAB0, 009ABF60, 009AC0B0, 009AC160, 009AC190, 009AC910, 009ACFC0, 009AD180, 009AD1F0, 009AD460, 009AD480, 009AD5E0, 009ADE00, 009AE8A0, 009AE9D0, 009AF0A0, 009AF480, 009AF510
- moveonpath_task_cc13.cpp: 009BC8E0, 009BCE50, 009BD6E0
- multiplayer_session_cc13.cpp: 0043BDF0
- opportunistic_rocket_cc13.cpp: 009FCEF0
- script_natives_fix5_cc13.cpp: 006CA8E0
- soldier_traffic_cc13.cpp: 0048A010, 0048A1E0, 0048A650, 0048A6D0, 0048AEC0, 0048AF00, 0048B120, 0048C230

**Presentation-only** (HUD, camera, menu, scene contents, settings text, draws), by source:

- airfield_marker_seg31_32_argon.cpp: 006DC5A0
- game_hosts_gunnery.cpp: 00673A10
- game_hosts_hud.cpp: 005CBAF0, 005CC170, 00647300, 007A44D0, 009542B0
- game_hosts_hud_world.cpp: 004B4B00
- game_hosts_scene_contents.cpp: 006D3C10, 007F3820, 007F4580, 00ADA420, 00ADA890, 00ADE500, 00ADFD70, 00ADFEB0, 00AEADE0, 00AEAE70, 00AEAFA0, 00AEB770, 00AEB7E0, 00AEB890, 00AEBB90, 00AEC3F0
- geom_mesh_resource.cpp: 00727310
- hud_ship_screen.cpp: 0043B370
- menu_screens_2.cpp: 005161C0, 005789C0
- mission_camera.cpp: 0042DCD0, 004329D0, 005470C0
- native_settings_text.cpp: 00BF8081, 00BF838E, 00BFB08B
- native_vertex_position_read.cpp: 004768D0

Logs used (newest first, read-only; `*_main_ref*` and `*_off_*` runs from the last 36 hours):

- `battlestations-pacific-decompile-cc9-lua52\local\l69r_j8h2_main_ref.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_u1d_main_ref.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_u1e_main_ref.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_u1a_main_ref.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_u1c_main_ref.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_u1b_main_ref.log`
- `battlestations-pacific-decompile-cc9-gunnery52\local\g53_wl_off_u1.log`
- `battlestations-pacific-decompile-cc9-gunnery52\local\g53_wl_off_u4.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_i08h_main_ref.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_j8h_main_ref.log`
- `battlestations-pacific-decompile-cc9-gunnery52\local\g53_dd2_off_j5.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_w_i18_main_ref.log`
- `battlestations-pacific-decompile-cc9-ships44\local\s59_bg_off_j05.log`
- `battlestations-pacific-decompile-cc9-physics1\local\p23c_off_u21f.log`
- `battlestations-pacific-decompile-cc9-physics1\local\p23c_off_j6.log`
- `battlestations-pacific-decompile-cc9-ships44\local\s59_bg_off_u01.log`
- `battlestations-pacific-decompile-cc9-physics1\local\p23c_off_u04.log`
- `battlestations-pacific-decompile-cc9-physics1\local\p23c_off_b9.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_w_e8_main_ref.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_u12e8_main_ref.log`
- `battlestations-pacific-decompile-cc9-ships44\local\s59_sm_off_u01.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_w_l10_main_ref.log`
- `battlestations-pacific-decompile-cc9-gunnery52\local\g53_dd_off_j5.log`
- `battlestations-pacific-decompile-cc9-lua52\local\l69r_u02a_main_ref.log`
