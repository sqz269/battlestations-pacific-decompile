# cc13-frontier: cc15's wiring frontier checked and merged into the stub census (2026-10-19)

Packet cc13-frontier, requested by cc9. This is read-only analysis. No game run, no host edit, no
Ghidra write. Inputs:

- `docs/CC15_WIRE_FRONTIER.md` (origin/main df305189a), section "Hand to cc9": 20 rows, of which
  18 are checked here, plus 19 rows the cc9 hosts restate. 007BB110 is skipped (cc13-reltimer
  has it), and so is 008E9AF0 (cc13-powerup has it).
- `docs/CC13_STUB_CENSUS.md` on `agent/cc13-stubcensus` (7ca60d538), which ranks 672 host sites
  over 11 WINS logs.

Line numbers refer to this branch after merging origin/main df305189a.

## 1. Logs

cc15's own USN04 log (cf2989718, 4800 mission frames) is no longer on disk. These logs were used:

- **USN04, current main.** `...-cc9-gunnery52\local\g53_nvoff_u4.log`. Build bd61f8f4f, an
  ancestor of origin/main. Clean checkout (`dirty=false`), exit 0, 30000 mission frames, env
  DEATH_TABLE and GUNNERY_RNG_STREAMS. This is the newest on-main OFF USN04 log; every newer
  USN04 log was built from a commit that is not on main. In the table it is `U04`.
- **The census's eleven logs** for JM08, USN04 (`p23h_off_u04`, 7f9207dc6), USN21, USN01, BSM09,
  BSM01, IJN08, USN15, USNOS, LOMP06 and JM06 (census section 1). They are read the same way:
  the exit host-method table, `<name> <address> concrete|UNIMPLEMENTED calls=N`.

cc15's 4800-frame counts scale onto the 30000-frame `U04` log. Per-frame rows match it exactly:
00C5BB30 86400 x 6.25 = 540000, 00673130 / 00A7B710 4800 against 30000, and 00644DB0 / 005BD420
4539 against 29739 (both are 261 frames short of the total). Event rows differ by build and
mission time: 0098B370 21885 against 147885, 009FABE0 5609 against 42592, 009A7650 34 against 31,
and 007C11E0 12 against 15.

## 2. The 18 frontier rows, verified

Classes are the census's: LIVE GAP, LIVE SUBSTITUTION, LIVE SWITCH OFF, MARKER and
PRESENTATION. "Standalone" means a reconstruction exists in `src/` that the host does not call.

| # | address / binary name | host site (record) | U04 calls; reach in the 11 logs | class and consequence | standalone in src/ |
|---|---|---|---|---|---|
| F1 | 009FABE0 BSP_Bot_SetDirectionFromHeadingPitch | game_hosts_units.cpp:27701 (level-bomb goaway), 37016, 37037 (retreat), 37820, 37897 (moveto); standby/line 34351/34543 are `done` while Angle_Prepare = 0 | 42,592; 11 of 11 missions (BSM01 125,348) | **LIVE SUBSTITUTION.** 009FABE0 writes the commanded direction `gun+68h` (cos a, tan pitch, sin a, normalised). 009FC7C0's steer gate (009FCCA4) tests `1 - cos(lead, +68h) < +40h`. The host gate, game_hosts_units.cpp:24824-24830, uses `1 - z/d`, which is the lead against the unit's current **forward** (labelled). The cone is Angle_MoveTo DEG(2), Angle_GoAway DEG(10) or the retreat cone (planeglobals.lua:454-455). When the gate passes, 009F9FC0 steers the plane toward the enemy lead. Steers per mission (summary `task gun cones`, steer_ticks): USNOS 4489, BSM09 1729, BSM01 1125, USN21 808, IJN08 131, USN04 10, USN01 1. This is plane heading on goaway, moveto and retreat. **Sent URGENT** (goaway class). | `bsp::cc13_seg68::bot_set_direction_from_heading_pitch_009fabe0` (src/cc13_seg68.cpp) |
| F2 | 007C11E0 BSP_Plane_NotifyFlightState | Host body `flight_state_notify_007c11e0` (game_hosts_units.cpp:16052) under `kPlaneFlightStateNotifyCc13Bound` = OFF (plane_land_chain_cc13.hpp:78). The lift-off at 16743 (inside `begin_flying_007c7110`) records it **outside** the switch. | 15; 8 of 11 (USN21 480, USNOS 113) | **LIVE, SWITCH OFF** (census row 12). The body sets the actuator channel targets +29h / +45h from +900h / +908h, and the active and fade handling. The lift-off site stays a gap even with the switch ON. With state 7, +908h = 0 or 3600, so channel b's target depends on the previous state. | host body (above); src/cc13_front3.cpp, src/cc13_seg47.cpp |
| F3 | 0080E290 BSP_Unit_AimTurningGunsToRest | game_hosts_units.cpp:12987 (role message 4Bh release, 0078023B); game_hosts_hud.cpp:1737 (SetControlledUnit 0064562D) | 1 + 1; 11 of 11 (USNOS 425 + 286, IJN08 157 + 156) | **LIVE GAP**, small. It walks the unit's devices (+48h, next +44h) and runs 0085AD00 on every kind-22h turning gun. When the player's helm role leaves a kind-6 ship, every turret of that ship is commanded to its authored rest angles, and that changes the turret angles the AI later slews from. The host already has 0085AD00's effect in the gunnery idle path (game_hosts_gunnery.cpp:8659). | `unit_aim_turning_guns_to_rest_0080e290` (src/cc13_sweepC.cpp:680) |
| F4 | 009A7650 BSP_BotTaskDogfight_SetTarget | game_hosts_units.cpp:24102 | 31; 5 of 11 | **Partial, minor.** The host stores the target (`df_target_plus_one`) and runs 007B7870 (kOpportunisticRocketBound). It does not run 009FB5E0 (target ref approach+2Ch) or 009FA350 (aim offset +28h..+30h = 0, dirty byte +41h = 1). Whether the host's dogfight path keeps a target-ref aim offset that this reset would clear was not traced. The observer pair is bookkeeping. | `bot_approach_dogfight_set_target_009a7650` (src/cc13_seg67b.cpp); 009FA350 in src/cc13_seg68.cpp |
| F5 | 008069A0 BSP_Recon_PublishGroupLevelFromMembers | game_hosts_gunnery.cpp:7731 (inside `kReconAggregatesBound`) | 1,002; 11 of 11 | **Unproven.** The host computes each squadron group's level as the maximum of its members for the triple lists. 008069A0 also writes that maximum into the **group unit's own** detection record for the side (unit+1E8h + slot*34h + 4h), releases its observer and fires the change delegate (det+2Ch vtable[0]) when the effective level moves. The host has no per-side detection record on squadron units, and the readers of that record were not surveyed. | `BSP_Recon_PublishGroupLevelFromMembers` (src/cc13_seg49.cpp) |
| F6 | 0098B370 BSP_SpatialIndex_SweepSegments | game_hosts_hud.cpp:1491 (five per camera frame); substitution comment in src/mission_camera.cpp:134-137 | 147,885; 10 of 11 | **SUBSTITUTION, presentation with one gameplay coupling.** At 0042F4DC, KeepAboveWater sweeps the probe segment through the spatial index (filter = target vtable[B0h]). A hit replaces the probe point. The host builds no unit collision, so the probe never hits. Effect: the camera pose. The player gun seat's artillery ray (00957740, game_hosts_gunnery.cpp:14155) starts at the camera position, so a moved camera moves that ray's origin. It does not call 0098B370 itself: it queries 0098ADD0 through the host's `query_segment_units_impl`. | `spatial_sweep_segments_0098b370` (src/cc13_seg62_seg65.cpp:1000) |
| F7 | 00C5BB30 Dyn_World_Substep | game_hosts_units.cpp:40630 | 540,000 (18 per frame, one per ship); 11 of 11 | **MARKER.** Mit/Dyn is game code under the project rules, not a bound library. The host runs the substep in pieces: 00C41550's velocity phase per ship (`done` at 40642); then, with `kDynWorldContactPhaseBound` = ON (hull_terrain_contact.hpp:95), the contact phase (collision pass, groups, solve) once for the world in `run_world_contact_phase` (20692); then the position phase per ship. 00C5C540's accumulator gives one substep per 0.05 s game step (world+00h = 0.05, +34h = 1; ship_motion_probe.cpp:1043-1050). SleepGroups 00C4B550 is inert because both sleep speeds are stored as zero (game_hosts_units.cpp:3345-3354). The record labels hosted work. Same verdict as census row 29. | `dyn_world_substep_00c5bb30` (src/dyn_physics_substep.cpp); `run_native_dyn_world_substep_00c5bb30` (src/native_dyn_world_step.cpp) |
| F8 | 0071FB90 BSP_DirectorPathObject_Construct | game_hosts_commands.cpp:2779 (`create_path_object` returns 0) | 52; 11 of 11 | **MARKER.** The fields that get read, +8h = 1 and +0Ch = 5, are carried as `path_follow_mode` / `path_start_mode` under `kPathObjectDefaultPairBound` = ON (game_hosts_commands.cpp:230-233). The path state lives in the host's own cursor. | `director_path_object_construct_0071fb90` (src/gameplay_seg38_cc13.cpp) |
| F9 | 00673130 support manager info panel | game_hosts_hud.cpp:560 | 30,000 (one per frame); 11 of 11 | PRESENTATION. Widget visibility (vtable +34h on +16Ch / +180h..+188h) and the `ingame.sm_support` text. | `support_manager_update_info_panel_00673130` (src/seg27_cc13.cpp:1417) |
| F10 | 00A7B710 BSP_SoundManager_SetListenerEnvironment | game_hosts_hud.cpp:731 | 30,000; 11 of 11 | PRESENTATION (audio listener environment Cockpit / Air / Underwater). | `set_sound_listener_environment_00a7b710` (src/sound_streaming_cc13.cpp:122) |
| F11 | 00644DB0 BSP_InGameHudRootScreen_PollUnitSelection | game_hosts_hud.cpp:3442 | 29,739; 11 of 11 | Input-driven. It polls actions 8Dh / 8Ch / 8Eh / 8Fh (next / previous unit). With no player input in these runs it is a no-op. | `hud_root_poll_selection` (src/hud_root_rows.cpp) |
| F12 | 005BD420 BSP_HudMinimapScreen_SelectNearbyCapturePoint | game_hosts_hud_world.cpp (`HudMinimap::prepare_frame`) | 29,739; 11 of 11 | PRESENTATION. Minimap +F4h is set to the nearest capture point of side 2 within range. No gameplay reader is known. | `select_nearby_capture_point_005bd420` (src/frontend_menus.cpp:367) |
| F13 | 005BD590 BSP_HudMinimapIconEntry_Construct | game_hosts_hud_world.cpp | 9; 11 of 11 | PRESENTATION (minimap icon widgets). | `construct_minimap_icon_entry_005bd590` (src/frontend_menus.cpp:349) |
| F14 | 00646040 BSP_InGameHudRootScreen_SetInterface | game_hosts_hud.cpp:387 | 6; 11 of 11 | PRESENTATION (HUD observer pair). | `hud_root_set_interface_00646040` (src/ingame_gui_cc13.cpp:1363) |
| F15 | 0068AB80 BSP_InGameInterface_CollapseOverlays | game_hosts_hud.cpp:470, 617 | 6; 11 of 11 | PRESENTATION / input contexts. Both overlay sets are empty in the host. | `collapse_in_game_overlays_0068ab80` (src/ingame_interface.cpp:302) |
| F16 | 00954990 BSP_Unit_BroadcastControlledAudioFlag | game_hosts_hud.cpp:1770 | 3; 11 of 11 | PRESENTATION (engine emitter mix, 0072B540). | `unit_broadcast_controlled_audio_flag_00954990` (src/gameplay_seg61_62_cc13.cpp:398) |
| F17 | 005251C0 HUD screen reset | game_hosts_hud.cpp:2161, 2260 | 1 + 1; 11 of 11 | PRESENTATION / input context levels (00A933F0). | `hud_screen_reset_selection_005251c0` (src/cc13_sweepA.cpp:2048) |
| F18 | 005B5D40 BSP_HudNarrativeScreen_HideText | game_hosts_script_orders.cpp:5664, 5717 (`unimplemented`) | 1; 11 of 11 (LOMP06 311) | PRESENTATION (narrative widget hide). | `hud_narrative_hide_text_005b5d40` (src/frontend_screens_cc13.cpp:488) |

For (a), each row was matched by address, by name and by behaviour. None of the 18 has an
existing switch that makes it a mere marker, except as stated in the table. F7 and F8 are markers
for the reasons given there. F2's body exists, but it is OFF and the lift-off site sits outside
its switch.

## 3. The 19 restated rows against the bytes

Each host restatement is a `done` row, which the table counts as concrete. Three levels of check
were used: **bytes** (disassembly of the whole body, or of the deciding block, compared with the
host); **spot** (one deciding constant or branch read from disk bytes); and **not audited** (the
body is large; existing evidence is cited).

| address | host site | check | verdict |
|---|---|---|---|
| 0085AD00 TurningGun_AimToRestAngles | game_hosts_gunnery.cpp:8659 | bytes 0085AD00-0085AD7A | **Agree.** Row = [[class+538h]+94h][gun+38Ch]. When row+94h equals FLT_MAX (double 00D7A278 = 47EFFFFFE0000000; FUCOMIP, `test ah,44h; jnp`), the body returns without calling. Otherwise it calls 0085ABA0(row+94h, row+90h). The host takes `rest_authored` (Lua `rh` present) and `rest_horz` / `rest_vert`. |
| 008FDAF0 GunBot_WorldDirectionToGunAngles | game_hosts_gunnery.cpp:8424-8465 | bytes of the body and 00521370 | **Agree, with a labelled substitution.** 00521370 is asin of the clamped y and atan2(x, z), and the horizontal value is negated (`-0.0 - h`, `kGunHorzImageSignBound`). The host uses the **hull** axes, not the gun's inverse frame (0042D0D0), and labels it ("no gun node frame"). NaN arm: the binary clamps NaN to -1, while the host's `max(-1, min(1, NaN))` gives +1. That needs a zero-length direction, which the host skips (`distance > 0`). |
| 00729BC0 Gun_BotSlotAcceptsTarget | game_hosts_gunnery.cpp:6569-6662 | bytes 00729BC0-00729C8F | **Agree on the range gate and the second ammo.** Function 6 with a plane target (IsKindOf 0Fh) selects ammo index 1 (`dp_air_ammo`). The reach is mount +FCh to target +FCh (`kAaSlotRangeOriginBound`) against ammo +60h. NaN arm: the binary's JA accepts an unordered reach, while the host's `<=` rejects it (MINOR). The binary's IsKindOf(5) with +54h == 2 reject (00729C00-00729C13) was not located at this host site. |
| 00720CD0 WeaponDirector_IssueTargetCommand | game_hosts_commands.cpp:4440-4476 | bytes 00720CD0-00720D76 | **Agree.** It runs 00720850 on each occupied slot from 9 down to 0, then vtable[60h](00E08F60 = the `follow` singleton, target {kind 1, id [target+174h], zero vector}). The host never passes a null target, which is the binary's kind 0 arm. |
| 008356F0 (director float +248h getter) | game_hosts_commands.cpp:1379 | bytes | **Agree.** The host's `cruise_fields()` bundles four one-instruction getters (008356F0 +248h, 008356C0 / D0 / E0) into one `CruiseAutopilotFields` read (+243h / +244h / +248h). The values are the same. |
| 007CC820 Plane_EnterFlightStateOne | game_hosts_units.cpp:16202-16229 | bytes 007CC820-007CC8A7 | **Agree in effect, with two differences.** (1) The host's early return on +900h == 1 is the request gate 007C214B (007C2130). The binary's 007CC820 itself still runs vtable[ACh] (the parent attach) when +900h is already 1. That is reachable only if the state moves between the request and the message dispatch. (2) The host writes +908h = 0, which 007CC820's bytes do not (labelled as inferred). The multiplayer arm (game+1FE4h == 2, 007C78A0) is n/a. |
| 0084BAD0 Explosion_ApplyRadialDamage | game_hosts_gunnery.cpp:12785 | spot 0084BE59-0084BE85 | **Agree on the spot checked.** The ignore-falloff flag is `[class+8] == 10h` (SETE 0084BE74), which is `kFlakBlastIgnoreFalloffBound`. The burst centre's back-off is double 00D7A270 = 0.05, carried at game_hosts_gunnery.cpp:1970 / 12181. The rest is not audited. |
| 00926E80 DeferredEntityEventQueue_Push | game_hosts_gunnery.cpp:11761, 12133 | ledger and host | **Agree in content; the timing is folded.** The binary queues the record under a critical section, and 009239A0 dispatches it later. The host dispatches at once, and this site carries no label for that. Not audited: how far the binary's flush trails the hit within the frame. |
| 006B8610 LuaMachine_RegisterGlobalFunction | game_hosts_lua.cpp:11403 | bytes | **Agree in effect.** The binary is `pushcclosure(fn, 0)` then `setfield(LUA_GLOBALSINDEX = -10002)`. The host pushes its trampoline with two upvalues (host binding), under the same global name. |
| 00B66200 LuaObject_IsFunction | game_hosts_lua.cpp:11562 | bytes | **Agree.** The test is `lua_type == 6` (00B6622B), and the host uses `lua_isfunction` under `kEntryPointIsFunctionBound` = ON (game_hosts_lua.cpp:332). |
| 00888D20 LuaTable_GetPtrField | game_hosts_script_orders.cpp:3095 | ledger (body read in full) | **Agree.** The field is `Ptr` (00CFAD08). The host returns its own id handle (labelled). |
| 008636A0 UnitGunneryAi_InstallPolicies, 00863A80 CreateDirectorBridge | game_hosts_gunnery.cpp:5929-5930 | host only | **Labels on the host's attach seed.** The policy objects and the 14h adapter are carried by host state: `bridge_countdown` = 0 stands in for the forced 008624C0 run, and the fire cache is false. Not byte-audited. |
| 0072BF10 Gun_CreateProjectile | game_hosts_gunnery.cpp:9698 | not audited | Large body (RET 0Ch at 0072C160). Existing record: `projectile_spawn_0072bf10`, backfilled from source. |
| 008777D0 UnitInstance_ApplyHitRecord | game_hosts_gunnery.cpp:11664 | not audited | The core damage path (`apply_hit`). Existing record: `apply_hit_record_008777d0`, backfilled from source. |
| 009DE2F0 ShipAi_RebuildHullGeometry | game_hosts_ship_ai.cpp:11782 | not audited | `semantic_build_tested_native_fixture_tested` (src/ship_ai_hull_geometry.cpp). |
| 0079A3B0 HudMovieCamera_Update | game_hosts_hud.cpp:2055 | not audited | Presentation, but it reseeds the RNG streams (census section 2). `reconstructed_partial_build_tested`. |
| 00AA7DC0 GuiWidget_SetLocalPositionAndBounds | game_hosts_hud_world.cpp:385, 988 | not audited | Presentation. A raw native body exists (src/native_gui_widget_position.cpp). |
| 007EF010 PlaneSquadron_SendNextHangarMember | game_hosts_units.cpp:16307 | not checked | It lies in the struck cc9 range 007EE000-007F2FFF. |

No restatement checked here disagrees with the bytes on a normal gameplay path. The differences
found are labelled substitutions, NaN-only arms, or the 007CC820 and 00926E80 notes above.

## 4. Merged ranking (frontier rows F1-F6 merged into the census's ranks 1-30)

Gameplay rows only. Presentation and marker rows (F7-F18 and census rows 23-30) are left out.

| merged | source | site | why it ranks here |
|---|---|---|---|
| 1 | census 1 | 009FA3A0 torpedo release timer (draw, +0Dh, 007BB110) | Torpedo AI, 5 missions, 123k records. **In build: cc13-reltimer.** |
| 2 | census 2 | 009D0E3A torpedo goaway draws | Torpedo goaway, 3 missions. Needs PilotBotConfig. |
| 3 | **F1** | 009FABE0 commanded direction +68h in the task gun steer gate | Plane heading on goaway, moveto and retreat. Up to 4489 steers per mission, in 7 of 11 missions. Sent URGENT. |
| 4 | census 8 | `kLuaUntouchableClearBound` OFF | BSM09: 5 units stay untouchable. Already built. |
| 5 | census 3-5 | Lua HasFired, AddWaterLoad, BreakShip | Objectives in BSM01, BSM09 and JM06. Standalone bodies exist. |
| 6 | census 9-11, 13 | `kDogfightFireEnvelopeBound`, `kFollowSightSearchBound`, `kDiveHitClockBound`, `kNavigatorForceTorpedoBound` (all OFF) | Built; they need pair runs. |
| 7 | census 12 = **F2** | 007C11E0, `kPlaneFlightStateNotifyCc13Bound` OFF, and the lift-off site outside it | 8 missions. The switch exists; the lift-off site needs covering. |
| 8 | **F3** | 0080E290 turrets to rest on helm release | Small but real turret state. Reaches all 11 missions (USNOS 711). |
| 9 | census 7 | 00729A80 CanFire on the level-bomb rack | Unproven. 6 missions. |
| 10 | census 14-16 | 009F9E40 moveto steer, 009BDE80 SetRanges, 007F0280 candidate walk | Substitutions on hot plane paths. |
| 11 | **F5** | 008069A0 squadron group detection publish | Unproven. Its readers need a survey first. |
| 12 | **F4** | 009A7650 target-ref reset | Minor. 5 missions. |
| 13 | census 17-22 | return-to-base arm, run profile, FireTargetAccuracy, moveto speed shaping, pilot plan term, leak timing | Unproven or minor, as the census states. |
| 14 | **F6** | 0098B370 camera collision ray | Camera pose. It reaches gameplay only through the player seat's ray origin. |

## 5. Proposed build list (LIVE gameplay rows from the frontier, smallest risk first)

Each item is new standalone wiring behind a new OFF switch, except item 2, which reuses an
existing switch. cc9 decides the files and the pairs.

1. **0080E290, `kHelmReleaseGunsToRestCc13Bound`** (OFF). At 0078023B (game_hosts_units.cpp:12987)
   and 0064562D (game_hosts_hud.cpp:1737), for a kind-6 unit, command each of its turning guns
   (category answering 22h) to its authored rest through the gunnery host's existing 0085AD00 path.
   The rest step is skipped when `rest_authored` is false. The units host needs one call into the
   gunnery host. Reach: USNOS 711 and IJN08 313 releases. Pair on USNOS and IJN08.
2. **007C11E0 at lift-off, under the existing `kPlaneFlightStateNotifyCc13Bound`.** Call
   `flight_state_notify_007c11e0(p)` at game_hosts_units.cpp:16743 under the switch, as the other
   sites do. Then pair the switch as a whole (census row 12) on USN21 and USNOS.
3. **009A7650 target-ref reset, `kDogfightSetTargetRefResetCc13Bound`** (OFF). First confirm that the
   host's dogfight approach keeps a target-ref aim offset (approach+2Ch, +28h..+30h). If it does,
   zero it and set the dirty flag at the three `df_target_plus_one` stores (24092, 24175, 25480).
   If it does not, the row is a marker; relabel it.
4. **009FABE0, `kTaskGunDesiredDirectionCc13Bound`** (OFF). Store a per-unit `gun_dir_68` at every
   009FABE0 site (level-bomb goaway 27701; retreat 37016 / 37037; moveto 37820 / 37897; land
   standby / line 34351 / 34543). Also add the dive goaway, 009C4E27-009C4E58, which the host notes
   as NOT MODELLED at game_hosts_units.cpp:30434 and which records nothing. Use `bot_set_direction_from_heading_pitch_009fabe0` with the heading
   and pitch each site already computes. Make the steer gate at 24829 use `1 - dot(lead/d, gun_dir_68)`
   in the unit's world frame, in place of `1 - z/d`. Check a NaN or zero length against
   009FABE0's `<= 0` arm (0 scale). This is a hot path, so pair it on USNOS, BSM09, BSM01 and
   USN21, and compare `steer_ticks` and plane headings by state.
5. **008069A0, `kReconGroupLevelPublishCc13Bound`** (OFF). This needs a per-side detection level on
   squadron units, plus the change delegate's consumer. First survey the readers of unit+1E8h for
   squadron (group) units. Build only if one lies on a gameplay path.
6. **0098B370, `kCameraCollisionRayCc13Bound`** (OFF). In `keep_above_water_0042f0c0`, sweep
   base -> r through the host's segment query, with the target's filter, and take the hit point.
   The effect is presentation, plus the player seat's ray origin. Lowest priority.

The torpedo rows (census 1-2) are in build or blocked on PilotBotConfig. The census's built
switches (rows 8-13) need only pairs. They are not repeated here.
