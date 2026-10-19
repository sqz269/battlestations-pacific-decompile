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

## Recorded gap: the round

In the image, 006E4D50 releases the rack's hanging MParatrooper round. That round was built by 007AB8C0, which takes one stream-1 draw for ParachuteOpenAltitude.
The release also makes the two stream-1 dispersion draws at 006E4F91 and 006E4FB1. 007AB230 takes the landing target, with y from 00903860. The round then flies
under its chute (007ABB30 / 007AB360 / 007AC000). It lands into 007AB590, which adds it to the building's capturer list
(006F39B0) and spawns a soldier with a stream-0 draw (004B0F80). The host has no MParatrooper entity. Its rack
drop `lb_rack_drop_006e4d50` hands the drop to `GameGunneryHost::release_bomb_drop`, which takes only a
general-bomb (2Ah) gun, so a 31h round spawns nothing. That function is unchanged. The binding's round slots record
`MParatrooper::round_not_modelled` if anything ever calls them; the 006E541F model does not.

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
- Not taken (the round is a gap): the drop's two stream-1 dispersion draws, the round's
  constructor draw, and the soldier's stream-0 draw.

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

## Log lines for a pair

- `paratrooper target point: <unit> rack=<i> t=... point=(...)` (006E3F90; the first 4 per unit).
- `paratrooper drop target: <unit> rack=<i> t=... landing=(...) spawned=0` (006E541F; the first 6 per unit).
- `levelbomb C47 #...` summary rows: `ar=`, `aim=`, `rel=` and `drops=` go above 0 with ON, where OFF shows
  `ga=1`, `fo=`/`mv=` only and `ord=0` transitions.
- `level bomber issue: C47 ... racks=` (007C0E67) appears with ON.
- With the draw switch OFF the record counter `Rack::paratrooper_drop_jitter_draw_untaken` rises instead of
  draws.
