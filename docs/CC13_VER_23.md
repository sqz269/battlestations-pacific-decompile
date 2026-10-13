# CC13 VER 23: ship and submarine torpedo attacks, image against host (cc13-ver23)

Packet cc13-ver23 is a read-only verification for the cc13 lead. It covers surface ships and submarines
launching torpedoes at ships:

- the decision to launch: range, target choice, readiness;
- the firing solution: lead and spread;
- the run: speed, depth, arming, hit test and duds.

Aerial torpedoes are covered by `docs/CC13_VER_20.md`, and the strike release gates by `docs/CC13_VER_6.md`.
This document extends three earlier reads and does not repeat them:

- `docs/CC13_GPR_40.md` 1.2 and 1.3;
- `docs/TORPEDO_LAUNCH_GATE.md`;
- `docs/TORPEDO_SPREAD_AIM.md`.

Method:

- Every gate below was read from the disk bytes (`bsp.py disasm-raw`). Virtual call sites were found with a
  `.text` scan for `MOV reg,[reg+disp8]` followed by `CALL reg` (`local/ver23_vcall.py`,
  `local/ver23_botcall.py`). `bsp.py scan-bytes` returned nothing for `ff 52 3c`, and this image emits no
  such form.
- The x87 sites were checked with `tools/x87_dc_form_scan.py` over `00852820`, `008FFF20`-`00900990`,
  `00903420`, `00838530`, `00416CD0` and `00901BA0`. The only DC forms are three `DC C9` FMULs at `009004AD`,
  `00900543` and `0090066B`. They sit in the run-line and friendly-scan arithmetic that
  `TORPEDO_LAUNCH_GATE.md` and `TORPEDO_FRIENDLY_CROSSING.md` cover, and none of them feeds a gate in this
  document.
- The authored numbers come from this installation:
  - `scripts/datatables/robots.lua` `TorpedoBot`, mtime 2025-06-01. That is not the 2024-07-13 bulk date, so
    the file may be locally modified;
  - `scripts/datatables/shipglobals.lua` `WeaponHitAccuracy.Torpedo`, mtime 2024-07-13;
  - `classtables/arcade/bulletclasses.lua`.
- No game runs were made.

Status of the C++ added here:

- `src/ship_torpedo_ver_cc13.cpp` and `include/bsp/ship_torpedo_ver_cc13.hpp` are reconstructed and
  build-tested.
- They are not wired into the host, because `src/game_hosts_gunnery.cpp` belongs to cc9. Every host change
  below is an adoption note.
- Nothing here is game-validated.

## Divergences

| # | image | host | effect on hit rate |
| --- | --- | --- | --- |
| D1 | `009001A8`-`0090031E`: gates A and B, with the `00901BA0` attack range | no range gate beyond the category range (`game_hosts_gunnery.cpp:5419`); fires at up to the full run | the image launches only inside the authored hit-chance curve |
| D2 | `00727F44` -> `009035A0`, then `00727F56` -> `00903580`: the attack range is recomputed with the **old** fire-target flag | no `bot+68h` cache at all | which accuracy row applies (part of D1) |
| D3 | `0090088E`-`009008EB`: a launch is held when the gun-to-lead line enters a layer-5 avoid zone | no zone test on a launch (`:7574`-`:7589`, `:7713`-`:7721`) | the host wastes torpedoes into islands |
| D4 | `008FFFB2`-`0090002A`: a surface ship drops a submerged submarine target | the target is kept (`:5373`-`:5395`, `:6914`-`:6924`) | the host fires torpedoes that pass over the boat |
| D5 | `008FC010` accept: torpedo-proof landing craft, the dead byte, line of fire | range only; kind `0Eh` is refused (`:5373`-`:5378`) | small. Carried from `CC13_GPR_40.md` 1.3 |
| D6 | `0072C970` -> `00852410`: swim depth predicted for a submarine firing at a submarine | -2.5 m for every launch (`:297`-`:300`, `:8218`-`:8221`, LABELLED) | a submarine's torpedoes miss a deep boat in the host |
| D7 | `0090087C`: `[gun+3F0h]+634h` bit 2 inhibits | `inhibited = false` (`:7539`) | only under a scripted `SetGunFireEnabled`. Carried from `CC13_AUDIT_CC9_RECENT_2.md` note (b) |

### D1: the attack range (first reported in CC13_GPR_40 1.2; still unbound; quantified here)

`CC13_GPR_40.md` 1.2 modelled the TorpedoBot tick's four steps:

- the 0.2 s think timer;
- gate A: the 3D distance must be at most `min(unit+44Ch * 1.5, bot+68h + 100)`;
- the no-intercept drop at `0090025E`;
- gate B: the lead-point distance in x and z must be at most `min(unit+44Ch, bot+68h)`.

That document left `bot+68h` as an injected function. Here `bot+68h` is bound to the real curve, so its size
can be measured.

- `bot+68h` comes from `00901BA0(size, bot+6Ch)`, which calls `008387B0(7, size, R, accuracy)`.
  - `R` is `00731020`, the tube class's `classDesc+60h`, which `00855A90` sets to
    `WaterTravelSpeed * FlyTime * 0.6`.
  - `accuracy` is the TorpedoBot level row's FireTargetAccuracy (`bot+6Ch` set) or AnyTargetAccuracy (clear),
    at `00901BC0` / `00901BD3`.
- `008387B0` kind 7 is `00838530` on the Torpedo profile, times `R`. `00838530` is already reconstructed in
  `src/ship_ai_settings_block.cpp`.
- `size` is the target class Length, `[t+538h]+0A0h` (`00903464`).

This installation's Torpedo curve is the hit chance at `n * 10%` of `R`, for a small and a large target:

| R fraction | 0.1 | 0.2 | 0.3 | 0.4 | 0.5 | 0.6 | 0.7 | 0.8 | 0.9 | 1.0 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| small (100 m) | 0.6 | 0.6 | 0.5 | 0.4 | 0.3 | 0.22 | 0.15 | 0.1 | 0.05 | 0.02 |
| large (300 m) | 0.8 | 0.8 | 0.7 | 0.6 | 0.45 | 0.3 | 0.22 | 0.15 | 0.1 | 0.05 |

The resulting fraction of `R` (`local/ver23_frac.py`, a port of the reconstructed rule):

| level | fire target, 115 m | any target, 115 m | fire, 180 m | any, 180 m | fire, 250 m | any, 250 m |
| --- | --- | --- | --- | --- | --- | --- |
| Stun (0) | 0.969 | 0.954 | 0.980 | 0.970 | 0.992 | 0.988 |
| SPNormal (1) | **0.459** | **0.361** | 0.497 | 0.410 | 0.538 | 0.463 |
| SPVeteran (2) | 0.897 | 0.878 | 0.930 | 0.910 | 0.965 | 0.945 |
| MPNormal (3) | 0.938 | 0.907 | 0.960 | 0.940 | 0.983 | 0.975 |
| MPVeteran (4) | 0.923 | 0.907 | 0.950 | 0.940 | 0.979 | 0.975 |
| Elite (5) | 0.897 | 0.878 | 0.930 | 0.910 | 0.965 | 0.945 |

Which level applies:

- The host answers `GetDifficulty` with 1 (`game_hosts_mission.cpp:984`, LABELLED).
- The mission scripts map difficulty 1 to `Mission.SkillLevel = SKILL_SPNORMAL` for the enemy and
  `SkillLevelOwn = SKILL_SPVETERAN` for the player's side (for example `bsm_01_stationed_at_pearl.lua:347`).
  There are 713 `SetSkillLevel(.., Mission.SkillLevel)` calls in this installation's scripts.
- So the **enemy** surface ships and submarines launch only inside about 36 % (any target) to 46 % (fire
  target) of `R` against a destroyer.
- The player's side launches inside about 88-90 %.

| torpedo (arcade) | `R` | enemy limit, 115 m target, any / fire | host limit |
| --- | --- | --- | --- |
| Mark 15 ship, Type 95 sub, Mark 14 sub (WTS 51.444, FlyTime 60) | 1852 m | 669 / 850 m | about 1852 m |
| Type 93 Long Lance (WTS 170.444, FlyTime 60) | 6136 m | 2215 / 2816 m | about 6136 m |

What it does to hit rate:

- The designers' own curve puts the hit chance of a launch at the host's limit at 2-5 %, against at least
  35-45 % at the image's limit for SPNormal.
- The host's enemy ships fire earlier and from up to 2.8 times farther, so a torpedo misses more often.
- The tubes empty earlier; the reload is `LoadTime` plus the 0.5 s `ShipTorpedoDelay` (`GUNNERY_OPEN_ITEMS`
  356).
- In the image, the ship AI's approach (`009F1BC0`, `+12B4h = 008387B0(7, length, unit+44Ch,
  FireTargetAccuracy)`) closes to about the same distance before the bot is allowed to fire. In the host the
  gunnery launches while the approach is still closing.

The no-intercept exit is part of the same gap. The host aims at the present position (`:7033`, `lead = at`)
and can fire. For a 51.4 m/s solver speed against a ship below about 20 m/s, `008FBB00` almost always solves,
so the exit rarely fires.

The 0.2 s think period moves a launch decision by at most one think step.

### D2: the stale fire-target flag (new)

`00727F10 BSP_Gun_SetBotFireTarget(target, flag)` updates the TorpedoBot through `gun+39Ch` in two virtual
calls:

1. `00727F44`, slot 38h, `009035A0`. When the target changes it calls `00903420`, which recomputes
   `bot+68h` with the flag the bot already holds (`00903554 MOVZX ECX,[EBX+6Ch]`).
2. `00727F56`, slot 3Ch, `00903580`. When the flag differs it calls `00903420` at `0090358D` first, and only
   then stores the flag at `00903592`.

So a flag change never reaches the cached range until the next target change.

- The constructor `008FF260` seeds `bot+68h = 10000.0f`.
- ClearFireTarget `008FF2B0` writes `bot+6Ch = 0` at `008FF2DF` and does not recompute.
- So the first commanded fire target after an automatic one keeps the AnyTargetAccuracy range. For
  SPNormal against a destroyer that is `0.361 R`, not `0.459 R`.
- `torpedo_gun_set_bot_fire_target_00727f10` models this order. A host binding of D1 must compute
  `bot+68h` with the pre-update flag. The host already carries the flag (`row.target_is_fire_target`, `:5939`).

### D3: the avoid-zone run gate (new; missing from TORPEDO_LAUNCH_GATE's table)

`TORPEDO_LAUNCH_GATE.md` goes from gate 14 (the `+634h` bit) straight to gate 15 (the launch command at
`009008F3`). Between them, at `0090088E`-`009008EB`:

1. `004218E0` returns the avoid zone manager.
2. `004120D0(5)` returns the group for layer 5 or below. With no group, the launch goes ahead.
3. `00417940(group, &gun_xz, &lead_xz)` must answer true. If it does not, `009008EB` jumps to `00900974`,
   `gun->vt[1E8h](0)`, and the trigger drops.

The two points:

- `gun_xz` is `[ESP+3Ch]`, `[ESP+44h]`, the gun position from `0090011D`.
- `lead_xz` is `[ESP+70h]`, `[ESP+60h]`, the `008FBB00` lead plus the `unit+6D4h` spread from
  `009002AB`-`009002BF`.

The tests:

- `00417940` (now `BSP_AvoidZoneGroup_SegmentKeepsOut`) answers true only when every zone answers
  `00416CD0` true.
- `00416CD0` (now `BSP_AvoidZone_SegmentKeepsOut`) answers true when the segment misses the zone box
  (`0085C910` through `004F2B00`).
- Otherwise it answers false when either end is inside (`00416B50`), or when any polygon edge crosses the
  segment (`004F3730(prev, cur, p0, p1)`, starting from the last corner).

The host's gunnery launch path has no zone test at all. The ship-AI host already holds the zone table
(`game_hosts_ship_ai.cpp`, `owner_.zones`, used at `:5825`).

Effect: wherever an island or another layer-5 avoid zone lies between a torpedo ship and its lead point, the
image holds fire and the host launches. Those host torpedoes end on the landscape, as `impacts_land`
(`:9387`-`:9410`). The size depends on each mission's avoid zones and is unmeasured.

### D4: the submerged submarine target (TORPEDO_LAUNCH_GATE gate 2, "not modelled"; read in full here)

`008FFF20` `008FFFB2`-`0090002A` runs these tests in order:

1. The target answers `IsKindOf(8)`.
2. The owner is the first `IsKindOf(5)` up the `[gun+3Ch]` chain.
3. The owner is not a submarine.
4. `00852820(target)` answers false. That function returns `y > ([+1204h] + [+1200h]) / 3.0`; the divisor is
   a double at `00D7A2B0`, and `JBE` makes a NaN answer false.

When all four hold, `bot->vt[38h](0)` drops the target.

- With the host's default bands (0, -20) the threshold is -6.7 m.
- The gunnery assignment lets category 7 take a submarine, as the image's `00862820` does: `0086288C` skips
  the band test for category 7 at `008628B7` (`CMP EDI,7; JE 008628C7`), and the host mirrors that at
  `:5380`.
- In the host nothing drops it afterwards, so a destroyer keeps firing at a boat below -6.7 m.
- Those torpedoes run at swim depth -2.5 m and pass over the boat, unless the hull shape reaches up to them.
- The host has the inputs: `units.submarine_band_y` (`:5385`) and the target's y.

The model is `torpedo_bot_drops_submerged_target_008fffb2`, with `00852820`'s answer as an input. That answer
is `gameplay_seg53_cc13::submarine_is_above_depth_charge_depth_00852820`.

### D5-D7

- **D5.** `008FC010` is unchanged from `CC13_GPR_40.md` 1.3 and still unbound. Its model is
  `gameplay_seg40_59_cc13::torpedo_bot_accepts_008fc010`.
- **D6.** `0072C970` returns `00852410`'s predicted depth only when the owner and the target are both
  submarines: an IsKindOf(8) owner, a target that is IsKindOf(8) or casts to a submarine, and a torpedo class
  of kind 10.
  - Otherwise it returns `-2.5` (`00D0C314` = `C0200000`).
  - `00856420` installs it through the order's `+8h` (`00856673` -> `record+47Ch`). Only the sub-vs-sub case
    differs from the host.
  - The host counts it (`torpedo_sub_vs_sub_launches`).
- **D7.** This is the `+634h` torpedo group bit that `SetGunFireEnabled` sets (`CC13_SCENE_BROWSER.md`, mask
  4). The host's `inhibited` is constant false.

## Verified equal

| item | image | host |
| --- | --- | --- |
| class range | `00855A90`: `WTS * FlyTime * 0.6f` -> `classDesc+60h` | `bullet_engagement_range.cpp:99`-`:108`, the same double product and one float store |
| solver speed | `0090022B` `WaterTravelSpeed` (`+0E4h`), not the 0.6 swim speed | `:7026` `gun.water_travel_speed`. The image's under-lead is its own (`TORPEDO_LAUNCH_ACCURACY.md` 2-3) |
| gyro jitter | `00900830` `00BD2F10(stream 1, AngleErrMin, AngleErrMax)`; `0090083E` `00BD2FC0(1) & 1` gives +1 or -1; degrees to radians at `00900866` | `:8242`-`:8248`: the same two draws in the same order. The host's generator is its own LCG, a global substitution |
| AngleErr table | `008FD640` rows, level order Stun 0, SPNormal 1, SPVeteran 2, MPNormal 3, MPVeteran 4, Elite 5 (`robot_config.cpp:772`) | `kTorpedoAngleErr` (`:918`) = {10-20, 0-10, 0-0.5, 0-6, 0-3, 0-0.5}, equal to this installation's robots.lua |
| shallow gate `00900110` | `bot+64h` is set only when the payload answers `IsKindOf(8)` (`008FF310`); the payload is the gun (`008FBC90`), whose chain 20h -> 1Eh -> 4 -> 2 -> 1 -> 0 never reaches 8. Dead code | absent. Equal |
| default swim depth | `0072C970` -> `-2.5` | `00856050`'s `-2.5` |
| life | the shared tick's FlyTime | `:9801`-`:9805` `life * swim > range` with `range = swim * FlyTime`. Equal for a swimming round |
| arming | `006E139B`: `record+45Ch >= 0` skips the sweep and subtracts dt. A torpedo's `+45Ch` is the memset 0, so exactly one sweep is skipped. There is no arming distance and no dud draw | not checked against the host; under one metre of travel at launch speed |
| warhead and hit | `0084BC60`: blast burst, contact damage after armour (`TORPEDO_WARHEAD.md`) | as landed by cc9 |

## RNG

- D1, D3 and D4 remove host launches.
- Each launch draws twice from the gyro jitter (`Draw::torpedo_gyro`; image stream 1). So a binding shifts
  every later shared-stream draw.
- Judge any pair on per-entity tables, not on aggregates.

## Predicted mission effect (no runs; for cc9's pairs)

| mission | who launches | D1 | D3 | D4 / D6 |
| --- | --- | --- | --- | --- |
| USN02 | IJN destroyers (SPNormal) with Long Lances; the player's cruisers | IJN launch limit falls from about 6.1 km to 2.5-3.05 km against a 180 m cruiser (0.410 / 0.497 R). Fewer, later launches; per-torpedo hits up. The opening spreads at 0.6-0.8 km (`TORPEDO_SPREAD_AIM.md`) are inside the limit, so the early Houston kill should stand | depends on the islands near the run lines | - |
| JM08 | enemy destroyers | same direction | - | - |
| IJN08 | US destroyers as the enemy (SPNormal) | Mark 15 limit 0.67-0.85 km instead of 1.85 km | - | - |
| PRCP01, JM06, USN06 | submarines | the enemy subs' Mark 14 / Type 95 limit drops to 0.67-0.85 km. The sub-attack state already uses the fire-target range (`SHIP_AI_SUB_ATTACK.md` 3) | - | D4: host destroyers stop firing at boats below -6.7 m; D6: a sub firing at a submerged sub hits in the image and misses in the host |

## Adoption notes for cc9 (`src/game_hosts_gunnery.cpp`, not edited)

1. **D1 + D2.** At the category-7 block (`:7015`), keep a per-gun `TorpedoBotRangeCache`. On every
   assignment (`:5939`), call `torpedo_gun_set_bot_fire_target_00727f10`, where `range_for_flag` is
   `torpedo_attack_range_00901ba0(Torpedo profile, level row, Length, flag, gun.max_range)`. Then apply
   `torpedo_bot_tick_gates_008fff20` with that range and the unit's category-7 range as `unit+44Ch`. Put it
   behind `kTorpedoBotAttackRangeBound = false`.
   - The Torpedo profile is already loaded for the ship AI's `009F1BC0`. The level row is the TorpedoBot row
     of the gun's skill level.
2. **D3.** After the jitter, call `torpedo_launch_zone_clear_0090088e` with the ship-AI host's zone table,
   `{muzzle x, z}` and `torpedo_lead_xz`. False drops the trigger. Use a switch `kTorpedoLaunchZoneBound`.
3. **D4.** Before the bot's aim, use `torpedo_bot_drops_submerged_target_008fffb2` with `submarine_band_y`
   and the target y, and clear `gun.target_unit`. Use a switch `kTorpedoSubmergedTargetDropBound`.
4. **D6.** This needs the boat's dive state from the units host (`00852410`). It is already labelled.

## Handoff

- Ledger records:
  - `00903580`, `00416CD0`, `00417940` (functions);
  - `0090088E`, `008FFFB2`, `00901BED` (fragments);
  - names `BSP_TorpedoBot_SetFireTargetFlag`, `BSP_AvoidZone_SegmentKeepsOut`,
    `BSP_AvoidZoneGroup_SegmentKeepsOut`.
- Ghidra was not annotated in this packet. `00416CD0` and `00417940` have no Ghidra function yet.
- Open questions:
  - which avoid-zone layers each mission authors, which sets D3's reach;
  - whether a hull shape reaches a -2.5 m torpedo over a boat at periscope depth, which sets D4's reach;
  - whether AI ships ever assign a torpedo target with the fire-target flag set. The host's
    `byte_7d_fire_target_assigns_at_0` census is the place to look.
