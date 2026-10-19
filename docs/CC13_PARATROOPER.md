# CC13_PARATROOPER: the level bomber's paratrooper (31h) path in the units host

Addresses: 009B7C90, 007B9500, 007BCCF0, 007C0D90, 006E3F90, 006E4D50, 006E541F, 006E55E7, 007AB8C0, 007AB230, 007AB590.

Packet cc13-para (worker cc13-undef, branch agent/cc13-undef), 2026-10-19, at cc9's request.
This is a static read of the bytes and the exports; no game was run. Both switches are OFF, in `include/bsp/cc13_seg67b_switches.hpp`:

- `bsp::kLevelBombParatrooperDropBound` covers the mechanism. The ordnance half (a) and the release half are under this
  one switch, so (a) cannot run without the release path.
- `bsp::kLevelBombParatrooperDropDrawBound` covers the two stream-0 draws. A `static_assert` requires the
  drop switch.

## Reach

| Mission | Script | C-47 order | 009B7C90 31h arm |
| --- | --- | --- | --- |
| JM08 (menu JM08 = `COTP-IJN/PRCPIJN/PRCPJM08.lua`) | `luaBomberWaveSpawned3`: class 136 gets `PilotSetTarget(unit, Mission.CommandBuilding)` | level-bomb task (kind 4) on Headquarter 01 | **reached** |
| JM05 | `luaJM5ParatroopersSpawned` (jm05.lua:3338): `PilotMoveTo` (the `PilotSetTarget` at :3351 is commented out); the drop is `PlaneForceRelease` (:3388) | moveto | not reached |
| JM11 | jm11.lua:2725 `PilotMoveToRange`; the role goes to PLAYER_1 | moveto | not reached by AI |
| JM13 | jm13.lua:3250 `PilotMoveTo`, then `PlaneForceRelease` (:2661, :2674, :3270) | moveto | not reached |

Log evidence, from cc9-lua52 `local\l64_j8_e_on_j5.log` (menu JM08, 2026-10-17, before this packet):

- `rack bullet kinds C47 #4.1 (level bomber): [31h/0.30s]`;
- `squadron ordnance C47 #4.1: ammo type -1 -> 5`;
- `level-bomb task 009B9030 kind 4 installed C47 #4.1 -> Headquarter 01`;
- then each wingman goes `follow (LevelBomb) -> LevelBomb/goaway ... ord=0`.

The run ended about 484 ticks after the spawn. No plane reached the target, and `releases=0`.

## What the switch models

| Image | Host (game_hosts_units.cpp) |
| --- | --- |
| 009B7CBF-009B7CF7 +CEh also counts 007B9500 (31h) | `lb_approach_update_009b7c90`: `in.has_paratroopers = lb_holds_paratroopers_007b9500()` |
| 009B7D7D-009B7E1F impact = 007BCCF0(unit) + v x +A8h (no +A4h); 009B7EC8 y = aim y | the same function, after `level_bomb_approach_update_009b7c90` |
| 007BCCF0 t = f(y / 2.8); p + v x t; y = 0078CF20 sea, raised to any higher 0087FA50 Landscape height | `paratrooper_release_point_007bccf0` (host binding; the raw-memory reconstruction is `cc13_seg47.cpp`) |
| 007C0E98-007C0EC9: 007C0D90 calls 006E3F90 for a fired 31h rack, passing the point that 007BCCF0 returns and the velocity from vtable[34h] | `run_level_bomber_issue_007c0e67` and the devwalk path `run_device_walk_per_rack_007c0d90` call `lb_para_set_target_point_006e3f90` -> `gameplay_seg46_cc13::set_target_point_006e3f90` |
| 006E541F-006E5515 landing = (U(-50,50) + px, py, U(-50,50) + pz), x first, stream 0; point += f(v x toRepeatTime) | `lb_para_drop_target_006e541f` -> `gameplay_seg46_cc13::paratrooper_drop_target_006e541f`, with `ParatrooperRackBinding::uniform` as 00BD2F10 |
| 006E55E7 flag cleared when ammo <= 0 | `gameplay_seg46_cc13::paratrooper_drop_clear_006e55e7` after the host's decrement |

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
  until their own timers or the same-side rule end them. UNCHECKED: whether the image clears +7DCh when the
  building entity dies; the host does not.
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
- The aim state steers on the 007BCCF0 point, the ground under the plane projected along its velocity, plus v x +A8h.
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
