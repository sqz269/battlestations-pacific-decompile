# CC13_PARATROOPER: the level bomber's paratrooper (31h) path in the units host

Addresses: 009B7C90, 007B9500, 007BCCF0, 007C0D90, 006E3F90, 006E4D50, 006E541F, 006E55E7, 007AB8C0, 007AB230, 007AB590.

Packet cc13-para (worker cc13-undef, branch agent/cc13-undef), 2026-10-19, at cc9's request.
This is a static read of the bytes and the exports; no game was run. Both switches are OFF, in `include/bsp/cc13_seg67b_switches.hpp`:

- `bsp::kLevelBombParatrooperDropBound` covers the mechanism. The ordnance half (a) and the release half are under this
  one switch, so (a) cannot run without the release path.
- `bsp::kLevelBombParatrooperDropDrawBound` covers the two stream-0 draws. A `static_assert` requires the
  drop switch.

## Reach

Census (corrected after rev4's review of 597dd029b; the first table said JM13 was not reached, which was wrong).
Only two vehicle classes carry a paratrooper gun: class 136 (C47 Dakota, gun 160 "US Paratrooper platform") and
class 186 (L2D Tabby, gun 161 "JP Paratrooper platform"), in both the arcade and realistic device tables
(vehicleclasses.lua `Platforms[50].Gun`). The census below covers every script under `scripts/missions` (stock,
COTP and PRCP) that spawns `Type = 136` or `186`, plus the scene-placed transports those scripts order
(`FindEntity`), and maps each script to the scenes whose `StageScript` runs it (`universe/scenes/**/*.scn`).
"Reached" needs `PilotSetTarget` on a building, which installs the level-bomb task (kind 4) and with it the
009B7C90 31h arm. A `PlaneForceRelease` reaches the release path (the per-rack issue 007C0D90 with 006E3F90,
then the rack drop) without the task.

| Script (scene) | Transport and order | Verdict |
| --- | --- | --- |
| `COTP-IJN/PRCPIJN/prcpjm08.lua` (`PRCPIJN/prcpijn_08_defend_guadalcanal.scn`, menu JM08) | `luaBomberWaveSpawned3` (:1264-1272): class 136 `PilotSetTarget(unit, Mission.CommandBuilding)` = Headquarter 01 (Japanese) | **reached** (log evidence below) |
| `COTP-IJN/PRCPIJN/jm08.lua` | the same code as prcpjm08.lua (:1176-1178, 1272) | not reached: no scene runs it (`PRCPIJN/ijn_08_defend_guadalcanal.scn` runs `IJN\JM08`) |
| `ijn/JM/jm08.lua`, `COTP-IJN/jm08.lua` | no class 136/186 | not reached |
| `ijn/JM/jm13.lua` (`ijn/JM/ijn_13.scn`, StageScript `Ijn\JM\JM13`); `COTP-IJN/jm13.lua` has the same MD5 (see the note on `COTP-IJN/ijn_13.scn` below the table) | `luaJM13DakotaSpawned` (:1953-1963) and `luaJM13Dakota2Spawned` (:2058-2059): the LevelBomber gets `PilotMoveTo(unit, Mission.CB)` then `PilotSetTarget(unit, Mission.CB)` = HQ 01. `luaJM13CheckPara` (:2652-2678, called at :784 and :805) also `PlaneForceRelease`s each Dakota plane within 300 m of the CB | **probable** (task and forced release) |
| `COTP-IJN/PRCPIJN/prcpjm13.lua` (`PRCPIJN/ijn_13.scn`, `PRCPIJN/prcpijn_13.scn`) | `DakotaSpawned` (:1399-1412): `PilotSetTarget(Mission.Dakota, Mission.CB)`, Dakota2 `PilotSetTarget(unit, Mission.CB)`; `SpawnDakotas` repeats every 150 s while HQ 01 is neutral and not captured (:1419) | **probable** |
| `COTP-IJN/PRCPIJN/jm13.lua` | `DakotaSpawned` (:1394, :1401), the same orders | not reached: no scene runs it |
| `COTP-IJN/PRCPIJN/jm11.lua` (`PRCPIJN/ijn_11_invasion_of_fiji.scn`, menu JM12) | `luaDakotaPlanesSpawned` (:1614-1619): class 136 `PilotSetTarget(unit, Mission.CommandBuildings[3])` = Headquarter 02. Spawned by `luaPlanes` 15 s after the damaged BB (hidden objective) dies (:1714-1720) | **probable** (needs the BB kill) |
| `ijn/JM/jm11.lua`, `COTP-IJN/jm11.lua` (`ijn/JM/` and `COTP-IJN/ijn_11_invasion_of_fiji.scn`) | scene transport `Mission.Paratrooper`: `PilotMoveToRange` (:2725 / :2743); the role goes to PLAYER_1 | not reached by AI |
| `ijn/JM/jm05.lua`, `COTP-IJN/jm05.lua`, `COTP-IJN/PRCPIJN/jm05.lua` (the three `ijn_05_invasion_of_port_moresby.scn`) | `luaJM5ParatroopersSpawned`: `PilotMoveTo(v, Mission.Airfields[1])` (the `PilotSetTarget` is commented out); the `PlaneForceRelease` in `luaJM5CheckParatroopers` is never called (its only call, :1206 / :1220 / :1325, is commented out) | not reached |
| `COTP-USN/def_aleutians.lua` (`COTP-USN/def_aleutians.scn`) | `luaSpawnLevelBomba` (:1107, from `luaPh1FadeOut` :934): five class 186 L2Ds, party Japanese; `luaTabbySpawned` (:1228-1250) `PilotSetTarget(unit, trg)` + `NavigatorAttackMove`, trg = `Mission.JapHQs[3]` (CB6) for "left", else `JapHQs[2]` (CB3) | **probable** |
| `multi/escort04.lua` | scene Dakotas 1-4: `PilotMoveOnPath` only | not reached |
| all other mission scripts | no class 136/186 spawn and no order on a scene transport | not reached |

UNVERIFIED script resolution: `COTP-IJN/ijn_13.scn`, `COTP-IJN/ijn_05_invasion_of_port_moresby.scn` and
`COTP-IJN/ijn_11_invasion_of_fiji.scn` name StageScript `Ijn\JM13`, `Ijn\JM05` and `Ijn\JM11`. This installation
has no loose `scripts/missions/Ijn/JM13.lua` (or `JM05.lua`, `JM11.lua`): the loose files are under
`ijn/JM/` and `COTP-IJN/`, and the install root has no script archive. How the game resolves those three names
(packed data elsewhere or another search path) was not traced, so which file those COTP-IJN scenes run, and
whether it equals the MD5-identical `ijn/JM/jm13.lua` / `COTP-IJN/jm13.lua` pair, is unverified. The JM13 verdict
stands on `ijn/JM/ijn_13.scn` and the PRCP scenes, whose StageScripts name loose files.

Not covered by this census: transports a scene places without any script order (none found among the scripts'
`FindEntity` names), and the native high-level AI (`highlvlaiglobals.lua` `Paratroopers` = 0.30/0.40, the
skirmish/campaign AI), which issues its own orders outside mission Lua.

Log evidence, from cc9-lua52 `local\l64_j8_e_on_j5.log` (menu JM08, 2026-10-17, before this packet):

- `rack bullet kinds C47 #4.1 (level bomber): [31h/0.30s]`;
- `squadron ordnance C47 #4.1: ammo type -1 -> 5`;
- `level-bomb task 009B9030 kind 4 installed C47 #4.1 -> Headquarter 01`;
- then each wingman goes `follow (LevelBomb) -> LevelBomb/goaway ... ord=0`.

The run ended about 484 ticks after the spawn. No plane reached the target, and `releases=0`.

First ON pair (cc9 physics24, exports of 677ee345f; logs in cc9-physics1 `local\`, ON `p24s3_j8.log`, OFF
`p24b_off_j8.log`). The task does reach JM08, but nothing dropped there:

- Object 154 is Headquarter 01 and resolves. Every C47 `PilotSetTarget` line reads `target_object_id=154
  target_valid=0 pos=(0,0,0)` in both runs (:1444352), and so do the B-17s'. `target_valid` is the target's
  `position_valid`, which is 0 for any object target (`game_hosts_script_orders.cpp` PilotSetTarget log); the
  task then installs on it (:1444703 `level-bomb task 009B9030 kind 4 installed C47 #4.1 -> Headquarter 01`).
- The C47s reach aim (:1509080 `moveto -> LevelBomb/aim t=1367.76 d=4322.5 alt=1283.2 ... ord=1`) but never
  release: `aim=1085 releases=0 drops=0 min_miss=20102.8` (:1833811), with min_miss between 11.7 and 24.9 km for every
  armed C47, and `summary paratrooper rounds ... drops=0 landed=0 ... damage_calls=0` (:1833696). The cause was
  the host binding of 007BCCF0. It read the world velocity where the image reads the forward row (+94h), which put
  the point about 30 km ahead. It was fixed in packet cc13-parafix; that pair predates the fix.
- JM08 HQ knife edge: ON fails at 1476.36 s, "Mission Failed - The HQ has been destroyed!" (:1817903). That is not
  paratroopers, since none dropped. After the C47 spawn every hit on Headquarter 01 in both runs is an `impact blast` of
  bullet 71 or 72 from the US ships ordered `fire=Headquarter 01` (Auilick, Macomb, Missouri, Helena, Gleaves, the
  LSTs and LSMs): OFF 10 x 71 and 115 x 72, ending at `health=62.4` of about 5950 (OFF :1936953); ON 10 x 71
  and 125 x 72, ending at 4.5 (ON :1818126). OFF survives by 62 HP, so any ON change that moves the shared stream or
  the AI (here, C47s circling in aim instead of leaving) can flip the outcome. Judge a JM08 "HQ destroyed" against
  this margin before blaming paratroopers.
- Menu JM13 in that harness runs `universe/Scenes/missions/COTP-IJN/ijn_16_ambushed_at_wake_island.scn ->
  Scripts/missions/COTP-IJN\JM16.lua` (`p24s1_j13.log:765`). `COTP-IJN/jm16.lua` spawns no class 136/186, so
  paratrooper=0 there is expected. The JM13 rows of the table above are the `ijn_13` / `prcpijn_13` scenes, which
  this menu id does not load.

## What the switch models

| Image | Host (game_hosts_units.cpp) |
| --- | --- |
| 009B7CBF-009B7CF7 +CEh also counts 007B9500 (31h) | `lb_approach_update_009b7c90`: `in.has_paratroopers = lb_holds_paratroopers_007b9500()` |
| 009B7D7D-009B7E1F impact = 007BCCF0(unit) + v x +A8h (no +A4h); 009B7EC8 y = aim y | the same function, after `level_bomb_approach_update_009b7c90` |
| 007BCCF0 t = f(y / 2.8) (2.8 = double [00D05A38]); p + fwd x t, fwd = the forward row +94h..+9Ch (row 2 of the +74h local matrix, a unit vector; 007BCD4D/007BCD61/007BCD6D), not a velocity; y = 0078CF20 sea, raised to any higher 0087FA50 Landscape height | `paratrooper_release_point_007bccf0` (host binding; the raw-memory reconstruction is `cc13_seg47.cpp`) |
| 007C0E98-007C0EC9: 007C0D90 calls 006E3F90 for a fired 31h rack, passing the point that 007BCCF0 returns and the velocity from vtable[34h] | `run_level_bomber_issue_007c0e67` and the devwalk path `run_device_walk_per_rack_007c0d90` call `lb_para_set_target_point_006e3f90` -> `gameplay_seg46_cc13::set_target_point_006e3f90` |
| 006E541F-006E5515 landing = (U(-50,50) + px, py, U(-50,50) + pz), x first, stream 0; point += f(v x toRepeatTime) | `lb_para_drop_target_006e541f` -> `gameplay_seg46_cc13::paratrooper_drop_target_006e541f`, with `ParatrooperRackBinding::uniform` as 00BD2F10 |
| 006E55E7 flag cleared when ammo <= 0 | `gameplay_seg46_cc13::paratrooper_drop_clear_006e55e7` after the host's decrement |

Labels on that table:

- 007BCCF0: the reference model is `plane_ordnance_release_point_world_007bccf0` (src/cc13_seg47.cpp:1826, with
  its own `kReleaseHeightDivisor_00d05a38`). It reads raw plane memory through `Seg47Host`, so the units host's
  slot-field binding `paratrooper_release_point_007bccf0` cannot call it and keeps the same arithmetic (the
  x, y, z sums in the same order) with its own copy of the divisor
  (`bsp::level_bomb_paratrooper::kReleaseHeightDivisor`). NOT MODELLED: 007BCCF0's first step, the pose refresh
  00414DB0 when +C8h == 0. LABELLED: the host reads `motion.position` (+FCh) as its own motion step left it; it
  keeps no separate stale world pose for 00414DB0 to refresh.
- 006E541F is wired only into the level-bomber rack tick `run_level_bomber_rack_tick_006e56f0`
  (`kPlaneLevelBombTaskBound` with `lb_level_bomber_racks()`). A flagged 31h rack that the devwalk single-rack
  path fires (`level && !lb_tick` in `run_device_walk_per_rack_007c0d90`) still has its target point set at the
  issue. Its drop, however, runs in the host's generic rack tick, and that host tick has no 006E541F block: no
  landing draw, no target, and (option 2) no round, because that tick's `release_bomb_drop` call passes no
  `ParatrooperRelease`.

## The round (option 1 alone: a recorded gap; option 2 below)

In the image, 006E4D50 releases the rack's hanging MParatrooper round. That round was built by 007AB8C0, which takes one stream-1 draw for ParachuteOpenAltitude.
The release also makes the bomb arm's four stream-1 cone and drift draws (006E4F91, 006E4FB1, 006E513C, 006E51AC). 007AB230 takes the landing target, with y from 00903860. The round then flies
under its chute (007ABB30 / 007AB360 / 007AC000). It lands into 007AB590, which adds it to the building's capturer list
(006F39B0) and spawns a soldier with a stream-0 draw (004B0F80). With `kParatrooperRoundBound` OFF the host has no
MParatrooper entity: the rack drop `lb_rack_drop_006e4d50` hands the drop to `GameGunneryHost::release_bomb_drop`
without a `ParatrooperRelease`, which takes only a general-bomb (2Ah) gun, so a 31h round spawns nothing. The
`ParatrooperRackBinding` round slots record `MParatrooper::round_not_modelled` if anything ever calls them; the
006E541F model does not.

## Option 2 (packet cc13-para2): the round itself

Switches (`include/bsp/cc13_seg67b_switches.hpp`), all OFF, each `static_assert`ing `kLevelBombParatrooperDropBound`:

| Switch | Covers |
|---|---|
| `kParatrooperRoundBound` | the round: hang, release, flight, landing, capture, bleed, kill |
| `kParatrooperConstructDrawBound` | 007AB94A, U(ParachuteOpenAltitudeMin, Max) on stream 1, once per hung round |
| `kParatrooperReleaseScatterDrawBound` | 006E4F91 / 006E4FB1 / 006E513C / 006E51AC, the four stream-1 release draws, per drop |
| `kParatrooperSoldierAnimDrawBound` | 004B102F in 004B0F80 (from 007AB6AC), U(0, length) on stream 0, per soldier |

The three draw switches also require `kParatrooperRoundBound`.

Mapping (image -> host):

| Image | Host |
|---|---|
| 006E3350 / 006E3E70 -> 007AB8C0, 007AC530 | `lb_para_hang_006e3e70` -> `RoundWorld::construct` (src/cc13_paratrooper_round.cpp) |
| 006E4D50 bomb arm (006E0A70, cone/drift, 006E1F00) for 31h | `GameGunneryHost::release_bomb_drop(..., &ParatrooperRelease)`: the plane's first 31h gun, no projectile, `Rack::paratrooper_release_006e4d50` |
| 006E541F..006E54B2 (target, 007AB230) | `lb_para_drop_target_006e541f` then `RoundWorld::drop` (`lb_para_release_006e4d50`) |
| 006E55E7 ammo > 0 -> vslot +224h | `lb_para_hang_006e3e70` after the decrement |
| callback list 00874DE0 -> 007AB590 | `RoundWorld::step`, first pass (seg46 `land_and_spawn_soldier_007ab590`) |
| wave 1, tick element +4h -> +194h 007AC000 | second pass (seg46 `step_landing_007ac000`, dt x class +5Ch) |
| wave 3, tick element +8h 007AB360 (-> +19Ch 007ABB30) | third pass (seg46 `update_chute_and_target_007ab360`) |
| +DCh 007AB0B0 | fourth pass (seg46 `anim_clock_007ab0b0`) |
| 006E1510 / 006E2220 | `bsp::bomb_air_velocity` (gravity 9.8100004196167, [00CF9058]) / position += dt x v, f32 |
| 006F2C30 kind 0 (007AC077) | `nearest_other_side_building_006f2c30` over world list 28, d2 <= (float)(+7C8h)^2 |
| 006F39B0 / 006F34B0 (+7DCh) | `RoundWorld::capturer_list_` (building handle, round id) |
| 006F6760 arm 3 (006F6BFA..006F6D5D) | ship AI capture tick: `strength[side] = (int)((float)strength[side] + CapturePower)` per capturer, via `GameUnitsHost::paratrooper_capturers_07dc` |
| building vslot +1ACh (006F1F20 -> 0095DA00) | `command_building_hit_gate_006f1f20`, then `apply_script_damage_0095da00`, as the landed remainder bleed |
| 00803510 | `party_relative_00803510` |
| 0084CB70 | `para_soldier_globals_0084cb70` over `load_soldier_globals_0084cb70` (src/soldier_traffic_cc13.cpp) |
| 007AC780 class keys | `para_class_fields` (bulletclasses.lua: CapturePower, SlowFactorClosed/Opened, Open/CloseDuration, CaptureDuration, Damage) |

LABELLED substitutions:

- All of a round's slots run once per fixed step at the head of `motion_step_00825f20`, ahead of the units' ticks; a
  round dropped in a rack tick first runs in the next step. +DCh is an entity frame slot; it runs once per step.
- The first hung round is constructed at the plane's first 007B9500 census, in census order, not at the spawn's
  006E3350. A rack the census missed hangs its round at the drop.
- The chute's fall/open/close lengths are 1.0 / OpenDuration / CloseDuration (the animation resource is not loaded).
- The class step scale +5Ch is 1.0f, the BombClass constructor's default ([00D7A24C]).
- +174h (the avoid-zone radius seed) is the host's round serial, cut to 16 bits.
- The entity kill 00926D90 runs 007AAE10 at once; the round leaves the world at the end of the step.
- The soldier is not a host entity: the round keeps a nonzero handle and the soldier's own update, fire and
  scoring slot are not modelled. The 004B0F80 draw bound is 1.0 (the SoldierAnim length is not loaded); only its
  place in stream 0 matters.
- SoldierGlobals.lua is read as text (`SoldierGlobals["Key"] = n`), not run in a private Lua state; this
  installation's file is exactly those six assignments (MaxParatrooperGradient 20, ParachuteOpenAltitudeMin 300,
  Max 400).
- Draw keys under `BSP_GUNNERY_RNG_STREAMS=1`: a round's draws key as (00F00000h + round id, 0) and a hang's
  construct draw as (00E00000h + construct serial, 0), bands no unit index reaches, so they share no key with a
  unit's own death-mode or death-delay draws. In the default mode the key is not read: stream 1 is the one shared
  generator in call order and stream 0 the separate one, so the order and values are unchanged.
- The capture tick's per-side strength is the host's two-side array; a capturer whose side is outside 0..1 is
  counted (`bad_side`) and skipped.
- Not modelled: 006F6760's scoring slot (vtable +108h / +4B8h), the chute and soldier animations, the
  landing message 71 for network sessions.

### ON-only lookups and their lifetimes (option 2)

- `unit_.lb_para_hung` / `lb_para_hung_set`: per-rack vectors on the plane's slot, sized at the first hang; live
  as long as the slot. A dead plane's hung rounds are never released (its rack tick returns first).
- `Impl::para_world` (rounds and capturer list): lives for the units host. A round is erased at the end of the step
  it is killed in; its capturer entries go in 007AAE10's 006F34B0 call before that. Building handles are slot
  index + 1, and slots are never erased, so a handle never dangles. A building that dies keeps its capturers
  until their own timers or the same-side rule end them, as in the image. Traced: +7DCh is emptied only by
  006F34B0, whose one reference is 007AAE3E in the round's own 007AAE10 OnKilled, and is freed only by the
  destructor 006F5AE0 (006F5B6E-006F5B85: 006F33A0(0) on +7DCh, then the free 00BF6989), whose one reference is
  the scalar-deleting destructor at 006F5CA3, and by two EH unwind funclets that do the same 006F33A0(0) through
  BSP_GarrisonVector_Destruct 006F3FC0: Unwind@00C83570 (`MOV ECX,[EBP-10h]; ADD ECX,7DCh; JMP 006F3FC0`), in the
  constructor 006F5610's unwind map (its handler 00C8357E is installed at 006F5613), which runs only when the
  constructor throws, and Unwind@00C835EB (the same three instructions, its JMP at 00C835F4), in the destructor
  006F5AE0's unwind map (handler 00C835F9 installed at 006F5AE3; map entry at 00DB1724), which runs only when
  the destructor throws. Those are the only two references to 006F3FC0 (rel32 scan). None of these runs on death
  or a party change, and none notifies the rounds (their +48Ch keeps the building). No death or party-change path
  touches +7DCh. Evidence: a rel32 and absolute-dword scan of the PE for 006F34B0, 006F39B0 and 006F5AE0 (one
  hit each), and every `+7DCh` access in the exported functions of 006E0000-007FFFFF (006F34B0, 006F39B0, the
  constructor 006F5610, 006F5AE0, 006F6760; 007E2A20's hits are stack offsets). The clear helper 006F35B0
  (undefined code, `PUSH 0; CALL 006F33A0; RET`) has no reference. Undefined code outside 006F35B0 was not
  scanned for `+7DCh`. The host never deletes a building slot, so it has no destructor case.
- World list 28 (the building search) and `slots[i]->row.party / motion.position / inferior_range_7c8`: rebuilt
  once per `ParaServices` (one per step or per drop), from the scene-loaded list.
- The capture target the ship AI tick reads: `paratrooper_capturers_07dc(b.unit)` copies the list per tick, so no
  reference outlives the call. The ship AI capture buildings it indexes are built lazily
  (`command_building_armour_0368` builds them on first use, as the capture tick does).
- `ship_ai->avoid_zone_runtime_mutable()`: null before `load_avoid_zone_geometry`; the round then keeps its point
  and records `MParatrooper::avoid_zone_group_absent`.
- `owner_.gunnery`: the draws, the release and AddDamage. Without it no round is released.
- `para_globals`: loaded once at the first hang; with no file, read=0 in the log and all six values are 0.

## ON-only lookups and their lifetimes

- `unit_.rack_bullet_class_per_rack`, `rack_ammo_per_rack`, `rack_authored_per_rack`: the unit's rack census
  (`plane_rack_census`, in census order). They live as long as the unit slot.
- `unit_.lb_para_rack`: per-rack state on the slot, sized to `rack_ammo_per_rack` at the first set.
- The sea height (`OceanFieldBinding` on the host) and `world_ground_height_00903860` (the scene's Landscape list,
  loaded with the mission).
- `owner_.gunnery`, which the draw takes only when the draw switch is ON.

## Draws (with both switches ON)

- 2 per drop from a flagged 31h rack: `00BD2F10(0, -50, 50)` for x and then z (006E5479, 006E549E). They are on stream 0 of
  the host's draw service, `death_mode_draw_00bd2f10`.
- Option 2, each under its own sub-switch: 1 stream-1 per hung round (007AB94A, at the first census and after each
  drop that leaves ammo); 4 stream-1 per drop (006E4F91, 006E4FB1, 006E513C, 006E51AC, in that order, before the two
  stream-0 target draws); 1 stream-0 per soldier (004B102F, at the step after landing). With a sub-switch OFF its
  record counter rises instead (`MParatrooper::construct_draw_untaken`, `MParatrooper::soldier_anim_draw_untaken`;
  the release scatter takes no cone and no drift).

## Predicted effect in JM08 with ON (draw switch OFF or ON)

- The C-47 wingmen no longer drop to goaway on install: +CEh = 1 while a 31h rack holds a round. The squadron then
  flies the level-bomb states toward Headquarter 01: moveto/follow, attackrun, aim, release.
- The aim state steers on the 007BCCF0 point, the ground y / 2.8 ahead along the plane's forward row (about 458 m
  at 1283 m), plus v x +A8h.
  It does not use the bomb's ballistic point.
- At release, each 31h rack gets its target point. Each rack tick drop decrements the rack (`rack_drops` and
  `lb_rack_drops` rise) and logs a `paratrooper drop target` line, but nothing spawns. There are no troops, no capture, and no damage to
  Headquarter 01. Once the racks are empty, +CEh = 0 and the planes go to goaway.
- With the draw switch ON, each drop also takes the two stream-0 draws. That couples to the other stream-0 consumers
  (explosion delay, engine fire) in the same run.

### With option 2 ON as well (`kParatrooperRoundBound`)

- Each drop releases a round at the plane's gun pose with the bomb release velocity. It falls ballistically,
  opens its chute at +468h above the ground (the 300..400 m draw, or 350 m without it), steers by SlowFactorOpened
  toward the 006E541F point, and lands near Headquarter 01.
- On landing, 006F2C30 takes the nearest building of another side within its InferiorRange. HQ 01 is Japanese and
  the C-47s are allied, so the round targets it. 007AB590 adds the round to the HQ's capturer list and spawns a
  soldier.
- While the round lives, 007AB360 bleeds (Damage 42 - HQ armour +368h) per second into the HQ through 006F1F20 (no
  hit while +7D8h != 0) and 0095DA00. With armour 30 that is 12 per second per round.
- The capture tick adds CapturePower 12 per capturer to the allied side's strength each tick (arm 3).
- If the HQ is neutralized (party 2) by the bleed or the capture, JM08's CheckHQ (`Mission.CommandBuilding.Party
  == 2`) fails the mission. JM08's victory is all Dakotas dead, so the paratroopers work against the defending player.
- A round whose building is or becomes the same side, a round in water, and a round landing on a slope beyond
  MaxParatrooperGradient (20 degrees) are killed (Kill(2)). After CaptureDuration (30 s) the round is killed too.

### The other reached or probable missions (ON)

Party values: 0 Allied, 1 Japanese, 2 neutral (JM08's CheckHQ fails on `Party == 2`). 006F2C30 takes the nearest
building of another side within that building's InferiorRange, neutral ones included.

- JM13 stock (`ijn/JM/jm13.lua`): HQ 01 is the player's (Japanese) capture objective (:1112). The Dakotas take the
  level-bomb task on it, and `luaJM13CheckPara` force-releases them within 300 m. They spawn in phase 2
  (`luaJM13InitDakota` from :1141), after the player has captured HQ 01 (:1112), so HQ 01 is Japanese. Paratroopers that land bleed and
  capture HQ 01 toward the Allies. **Mission can end the same way**: `luaJM13CheckRecapt` (:2134, called at :785,
  :806, :811) fails the mission ("ijn13.obj_missionfailed_recap") once `Mission.CB.Party == PARTY_ALLIED`. Separately,
  once a Dakota has dropped and reloads, `GetPlaneIsReloading` fails it ("ijn13.obj_missionfailed_para", :1155-1160)
  whether or not anything captures, so in the image a completed drop alone ends the mission. In the host that
  rule depends on whether a drop happens at all (OFF: no 31h ordnance, so no task drop; a forced drop needs
  `kLuaPlaneForceReleaseBound`) and on the host's `GetPlaneIsReloading` (packet cc13-fix16), not traced here.
- JM13 PRCP (`prcpjm13.lua`): the Dakotas fly only while HQ 01 is neutral (`Party == 2 and not Mission.Captured`,
  :1419). Paratroopers capture it toward the Allies (0). Primary 3 completes only on `Party == 1` (:1456), and an
  Allied HQ switches the respawn to bombers (`Party == 0`, :1421). No fail rule reads the HQ (`CheckForces`'
  `not Mission.CB.Party == 1` is always false in Lua), so the effect is a blocked primary 3, not a fail.
- JM11 PRCP (`PRCPIJN/jm11.lua`): the Dakotas target Headquarter 02. Primary 2 counts the Japanese-held bases
  (:1134-1140); paratroopers on a Japanese HQ 02 neutralize it, so the count drops. No fail rule reads a base's
  party, so no fail.
- def_aleutians (`COTP-USN`): Japanese L2Ds target CB3 / CB6. A round only bleeds or captures a non-Japanese
  building within range; if CB3 or CB6 is still Japanese, a round that finds no other building there is killed
  (Kill(2)). Neither base is read by a fail rule. The fail rules read CB1 (`Mission.HQ.Party == PARTY_NEUTRAL`,
  :746, only while primary 1 is active and `not Mission.DMSpawned`) and CB9 (`MainHQ.Party ~= PARTY_JAPANESE`, :880, primary 5). CB9
  could change only if a round lands within its InferiorRange; not predicted.
- Achievement: `GA_AI` "Airborne Invasion" ("islands captured by paratroopers", achievements.lua:823-829) is a
  native counter. It is presumably bumped from the capture's scoring slot (006F6760 vtable +108h / +4B8h), which is
  not modelled. UNVERIFIED.

None of these scripts registers a Lua `capture` listener; the scripts see a capture only through the building's
`Party`.

Pair grep rows for these missions are the same as JM08's (below). In addition: JM13's `luaMissionFailedNew` text
"ijn13.obj_missionfailed_recap" or "..._para", and `PlaneForceRelease 0089EBA0 calls=` in the script-orders
summary (forced drops, only under `kLuaPlaneForceReleaseBound`).

USNOS check (ships60's note "a TroopTrans within 1500 m flips a NEUTRAL building"): that rule is mission Lua,
us_osumi.lua:1100-1106, a SetParty for Mission.Troops ships within 1500 m. It does not involve paratroopers and is
separate from the native 006F6760 arm 3, which this packet models.

## Log lines for a pair

- `paratrooper target point: <unit> rack=<i> t=... point=(...)` (006E3F90; the first 4 per unit).
- `paratrooper drop target: <unit> rack=<i> t=... landing=(...) spawned=0` (006E541F; the first 6 per unit).
- `levelbomb C47 #...` summary rows: `ar=`, `aim=`, `rel=` and `drops=` go above 0 with ON, where OFF shows
  `ga=1`, `fo=`/`mv=` only and `ord=0` transitions.
- `level bomber issue: C47 ... racks=` (007C0E67) appears with ON.
- With the draw switch OFF the record counter `Rack::paratrooper_drop_jitter_draw_untaken` rises instead of
  draws.
- Option 2: `paratrooper soldier globals:` and `paratrooper class:` (once each), `paratrooper drop:`,
  `paratrooper landed:`, `paratrooper soldier:` (first 8 each), `paratrooper killed:` (first 12),
  `paratrooper drop target: ... spawned=1`, the record `Rack::paratrooper_release_006e4d50`, and the summaries
  `summary paratrooper rounds` (units host) and `summary mission capture paratroopers` (ship AI host; printed
  OFF too, with adds=0).

## JM08 pair and flip (cc9-ships64, 2026-10-27)

All six switches were paired together on 8eded9e50, plan `config/win_orders/jm08_s63_j8_v3.txt`, reference env
(`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`), 110000 frames. The logs are in cc9-ships44 `local\`: OFF
`s64_off2_j8.log`, ON `s64_on2_j8.log` (export `local\s64_paraon`, SHA-256 6AE861089D5F), and
`s64_pd_j8.txt` (pair_diff exit 3, moved).

- OFF completes at 4246.98 s, the same second as ships63's reference. It logs the guard refusal,
  helm_moveto_unselected=0 and helm_role_forced=0. An earlier OFF on 6a9c86448 (`s64_off_j8.log`) also completed
  at 4246.98 s after one D3D device reset (`present failed hr=0x88760868`, recovered, frames_presented=44409). The
  lead ruled it valid.
- ON fails at 1762.64 s with "Mission Failed - The HQ has been destroyed!".
- The death rows are identical until 1713 s, after the C47 spawn at 1571 s.
- As predicted, the C47 wings take LevelBomb attackrun/follow (ord=1) instead of goaway. C47 #4.2 and #4.5|.-IV
  enter LevelBomb/aim at 1702-1706 s, d=4440 m, alt about 1290, in_range=1. A static check by cc13 found three of
  the four C47s at min_miss 295/227/180 m, seconds from the 300 m / 85 degree release, when the HQ died 58.5 s after
  aim entry.
- Not reached: release, drop, landing, soldiers and capture. The summary has constructs=20, construct_draws=20,
  drops=0, landed=0 and damage_calls=0; capture paratroopers adds=0.
- The HQ loss is not caused by paratroopers. HQ damage in 1500-1770 s is two bullet 71 hits plus bullet 72
  (B-17): OFF 234 hits for 9416.9, ON 237 hits for 9417.2. OFF bottoms out at 9.4 hp at 1762.54 s; ON takes one
  more bomb and dies at 1762.64 s. This knife edge is moved by the 20 stream-1 construct draws and by the C47s
  loitering at about 1290 m in the AA picture.
- **Flipped ON** by the house rule: the part the run reaches matches the prediction, and a faithful fix flips even
  if it loses a win. The JM08 re-plan went to cc9-lua74; it needs the HQ to last 5-10 s longer at about 1762 s and
  then to survive the paratroopers.
- **Still unexercised in a live run:** the drop, round, landing and capture half (option 2). The first plan that
  reaches a C47 release validates it; check the "Log lines for a pair" rows above in that run.
- Deviation noted by cc13, not switched: the host's rack gate (plane_level_bomb_task.cpp:551) tests attitude only,
  while the image also tests the bay doors. The host is therefore more permissive. This is a candidate for an OFF
  switch.
