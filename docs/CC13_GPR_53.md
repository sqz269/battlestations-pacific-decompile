# CC13 GPR 53: segment 53 reference models (submarine depth, torpedo round, water mine)

Addresses: 00851E10-008740E0 (segment 53). Packet cc13-gpr53, approved by cc9 as a
reference-model pass: new `src/gameplay_seg53_cc13.cpp` and
`include/bsp/gameplay_seg53_cc13.hpp` only, no host edits, nothing wired, no game runs.

Status words: every model below is **reconstructed and build-tested** (MSVC Win32,
`scripts/build.ps1`). None is fixture-tested, ABI-compatible or game-validated. Names are
hypotheses. The x87 bodies were traced from the disk bytes (Capstone via `bsp.py disasm-raw`),
with the stack tracked by hand. Gameplay x87 runs at 24-bit precision
(docs/CC13_FPU_PRECISION.md), so every intermediate rounds like a float. The models keep the
listing's operand order, and every `FCOMI`/`JBE` becomes a `>` test that is false when the
compare is unordered.

## 1. Models

### 1.1 Submarine depth helpers (unit vtable 00D0BF34, class 08h)

| address | model | what it does |
|---|---|---|
| `008522C0` | `submarine_catapult_needs_surface_008522c0` | If unit+1279h is clear, false. Otherwise true when a child in the +48h/+44h list answers `vtable[5Ch](28h)` (MCatapult) and has child+490h >= 0 (signed `JGE`). |
| `00852410` | `submarine_predict_depth_after_00852410` | The y the hull will reach after `dt` seconds (section 1.2). |
| `00852740` | `submarine_is_below_band1_for_kind0e_00852740` | False for a null argument or one whose +8h is not 0Eh. Otherwise `y < float(band1 + 2.0)`. |
| `008527A0` | `submarine_is_above_deck_gun_depth_008527a0` | `y > float(band0 - 3.0)` (00D7A2B0). |
| `00852820` | `submarine_is_above_depth_charge_depth_00852820` | `y > (band1 + band0) / 3.0`, with no intermediate store. |
| `00852B50` | `submarine_depth_step_shallower_00852b50` | Defined in Ghidra by cc13-gpr53b (unreferenced). `level > 0` -> `008528B0(level - 1)`. |
| `00852B70` | `submarine_depth_step_deeper_00852b70` | Defined by cc13-gpr53b (unreferenced). `level < 3` -> `008528B0(level + 1)`. |
| `00852970` | `periscope_debris_step_00852970` | Presentation. The periscope debris piece ("PeriszkopSullyesztoDolog:", built by 008533E0): state 0 captures the matrix and the owner's velocity. State 1 integrates the position, turns the basis when `|w| dt > 0.001` (00D7A23C), sets `vy -= 2.5 dt`, and decays v and w by 0.45 and 0.51 per second. |
| `00852FF0` | `submarine_navigation_group_at_depth_00852ff0` | `0082ADA0(class)(depthLevel)`, not clamped. |

Every helper first runs the pose refresh 00414DB0 when unit+C8h is clear. The models take the
refreshed `world_y_100` as an input.

### 1.2 `00852410` PredictDepthAfter (`__thiscall(unit, float dt)`, `RET 4`, result in ST0)

The only caller is 0072C970 (ledger name `BSP_Gun_TorpedoLaunchHeading`). That function returns
this value, or -2.5 (00D0C314) at 0072CA6B, and the value becomes the order's second argument
(`007311B0`). 00856420 copies it from launch params +8h into the torpedo's `swimdepth` +47Ch
(00856673). So the ledger name "LaunchHeading" is misleading: the function returns a **swim
depth**. 0072C970 is outside this segment, so it was not renamed. The note is here for its
owner. ECX at the call (0072CA4A) is the submarine 0072C970 resolves; `dt` is
`|pos(sub) - pos(gun)| / class+E4h` (the torpedo's WaterTravelSpeed).

Stack slots after `PUSH ESI`: A `[esp+4]`, B `+8`, C `+0Ch`, D `+10h`, E `+14h`, F `+18h`, and dt `+2Ch`.

1. `dt > 30` -> 30 (00CE7630 double, 00CE38C8 float).
2. `level = clamp(depthLevel, 0, 3)`. `target = clearance(+123Ch) > band[level] ? clearance : band`.
   needAir (+1281h), or failing that 008522C0, forces `target = 0`.
3. `vy` = 0092D6A0's y: the body's linear velocity (00C31F40 on unit+1018h -> +2Ch).
   `F = sink(+1270h) - DownSpeed(+834h)` and `E = UpSpeed(+830h) - sink`, each stored as a float.
4. If `target - 2 > y` (rising): `t = min(|E - vy| / UpDownAccel(+824h), dt)`,
   `v1 = E > vy ? acc*t + vy : vy - acc*t`, `y1 = y + (vy + v1) * 0.5 * t`,
   `y2 = E * (dt - t) + y1`, and the result is `min(y2, target + 2)`.
   Else, if `y > target + 2` (sinking): the same steps with F, and the result is `max(y2, target - 2)`.
   Else the result is y.
5. When `clearance > result`, the function returns the clearance (00852709 path) and **skips**
   the next cap. Otherwise it returns `min(result, -2.5)`.

### 1.3 The MTorpedo round (entity vtable 00D0C3E8, class 2Bh, 51Ch bytes; shot +310h, vtable 00D0C378)

The "_torpedo" schema strings (00855D30, 00857780) give the per-round fields: `heading`
+46Ch, `accel` +470h, `dragfront` +474h, `dragside` +478h, `swimdepth` +47Ch,
`depthsmoothrate` +480h and `waverate` +484h.

| address | model | what it does |
|---|---|---|
| `00855A90` | `torpedo_class_derive_00855a90` | Sets +60h = `E4h * 54h * 0.6` and +ECh = `sqrt(float(2 * MaxFall * 9.81))`. Returns AL = base 006E9890 `&& +ECh > 0`. The host's `torpedo_cached_range` / `torpedo_terminal_fall_speed` give the same two fields but drop the return value. |
| `00855B30` | `torpedo_propeller_angle_00855b30` | Presentation, partial: after 006E2220, the propeller node +4A0h turns by `dt * 26.1799` rad (1500 deg/s, 00D0C318) about +4C4h. |
| `00855CC0` | `torpedo_create_message_00855cc0` | MT create message (type B8h, vtable 00D037BC): +120h heading, +124h swimdepth, +128h the target's net id (+174h) or 0. |
| `00856050` | `torpedo_construct_00856050` | Partial (field defaults). Swimdepth -2.5, +48Ch..+494h = 1.0, +514h = 1.0, and **+518h = -U(0, 1) on stream 1** (00856132; ECX = 1 from 008560C8, unchanged until the call). |
| `00856420` | `torpedo_create_00856420` | Partial. Velocity = direction * V0 (+50h); the forward row and pose are written. With launch params (argument 7): heading = +4h, target link = +20h, swimdepth = +8h. Without them: heading 10000 (unset), the link is cleared, and swimdepth stays -2.5. |
| `00857780` | `torpedo_activate_00857780` | Runs 006E2E10, and runs it again when the start position is outside `\|x\|,\|y\|,\|z\| < 50000` with `\|y\| < 1700`. Sets dragfront 0.59999943, dragside 3.0000007, accel = `float(E4h) * 0.59999943`, and depthsmoothrate = waverate = 15. A round still in the air gets SetWeaponScale(0.25); one already in water sets +49Ch = 1. Creation kind 3 (saved game) reads the "_torpedo" table; kind 2 (network) takes the message's swimdepth. The model's "bubor" point goes to +33Ch. |
| `008568E0` | `torpedo_enter_water_008568e0` | Runs SetWeaponScale(1.0). The round **breaks up** when `\|v\| > MaxWaterHitVel` or `vy < -terminal fall speed` (+ECh). On a server the break-up kills it (00926D90(1)); on a client it runs 006E6450 instead. Otherwise 006E6450(arg) runs and a splash appears when `arg && WaterSplashEfx`. A heading still exactly 10000 takes the pose's yaw (00521370), written to shot+15Ch in place. Since cc13-fix3 the 006E6450 and 00926D90 calls go through the `enter_water` and `kill_entity` hooks at the image's points (006E6450 before the splash); there is no outcome enum. |
| `00857160` | `torpedo_create_message_apply_00857160` | Copies the heading and the target link from the message. Swimdepth is not copied here (00857780 does it). |
| `00857D40` | `torpedo_boat_is_body_speed_above_3_00857d40` | `vtable[38h]() > 3.0`. |

Host pointers, checked against the listing:

- `00855F00` -> `torpedo_water_mode` (src/bomb_torpedo_tick.cpp): `2*swimdepth > y` gives 2; otherwise the in-water byte.
- `008566B0` -> `load_torpedo_class_fields` (src/projectile_kinds.cpp): the same seven keys in
  the same order. The two Homing keys default to 0 (00B66330 with FLDZ).

### 1.4 The MWaterMine round (entity vtable 00D0D130, class 34h, 470h bytes)

| address | model | what it does |
|---|---|---|
| `0085F500` | `water_mine_read_class_fields_0085f500` | Reads V0RandomFactor +D8h, DiveSpeed +DCh, WaterSplashEfx +E0h, FloatingDepth +E4h (default -3) and TriggerRadius +E8h (default 5). The host's loader dispatch only chains the bomb reader for this kind. |
| `0085EBC0` | `water_mine_activate_0085ebc0` | Runs 006E2E10 and sets weapon scale 0.25 in the air. Off a client, scales the velocity by **U(1-r, 1+r) on stream 1** (0085EC48, ECX = 1 at 0085EC26). Then shot `vtable[34h]`, and clears armed +46Ch and marker +468h. |
| `0085ECD0` | `water_mine_step_0085ecd0` | Drag is `v * (-9.81 / DiveSpeed)`; the vertical spring is `-0.25 * ((y - FloatingDepth) - 0.25 * waterHeight)`. Off a client, an unarmed mine below `FloatingDepth + 1` whose `speed^2 < 1/16` sends MT_WATERMINE_ARM (B5h). An armed mine detonates (00926D90(1)) on a ship (world+60h list, 150 m prefilter) whose half-extents box, grown by TriggerRadius, contains it. It also detonates on a depth charge (world+228h list, the list 006FD190 registers) inside TriggerRadius. |
| `0085F160` | `water_mine_hit_detonates_0085f160` | An armed mine hit by anything that is not a water mine detonates. |
| `0085F2F0` | `water_mine_message_arms_0085f2f0` | B5h -> 0085F1E0 (armed = 1, a 1ACh part instance at +468h), modelled by `water_mine_arm_0085f1e0` (section 7.2). |
| `0085F6D0` | `water_mine_enter_water_0085f6d0` | Runs SetWeaponScale(1.0), then 006E6450 and the splash. There is no break-up test. |
| `0085F7A0` | `water_mine_on_killed_0085f7a0` | For kill cause +70h == 1 only: the damage is **U(BlastDamageMin, BlastDamageMax) on stream 1** (0085F7D5), applied with radial damage 0084BAD0 and ExplWaterEfx at the water surface. Then 0077D270 runs. |

### 1.5 Other host pointers, checked

- `00852B90` -> `sensor_category_submarine_00852b90` (src/sensor_tables.cpp). The categories
  1 / 2+periscopeOut / 4 / 5 match. One precision note: the host compares the first limit
  `(band1 + band0) / 3` in double. The image computes it in x87 without a store, which at 24-bit
  precision rounds like a float. The two can differ only for a y within one float ulp of the limit.
- `00853630` -> `submarine_build_depth_bands_00853630` / `submarine_nearest_depth_band_00853630`
  (src/submarine_model.cpp), partial. The band loop 00853A90..00853AC5 matches: the default
  table is 00E0B578 and the key is computed as `-0.0 - key`. A NaN key takes the default in the
  image (`COMISS`/`JB`); the host negates it instead. The rest of SEntityInit is not modelled.

### 1.6 Follow-up pass cc13-gpr53b

**New Ghidra functions.** `tools/ghidra_define_function.py` defined these under the write lock,
with flow discovery not re-run; 'Non-Returning Functions - Discovered' stayed disabled. The
event record is reports/defined_functions.json. Names were applied with
`ghidra_annotate.py --apply`.

| address | end (excl.) | name | reached from |
|---|---|---|---|
| `008571F0` | `00857472` | `BSP_TorpedoProjectile_UpdateWakeEmitter` | torpedo vtable 00D0C3E8 slot DCh (00D0C4C4) |
| `00855B90` | `00855BAD` | `BSP_TorpedoProjectile_ShowNode` | slot 68h (00D0C450) |
| `00855BD0` | `00855C15` | `BSP_TorpedoCreateMessage_ResolveTarget_Unreferenced` | no call or data reference |
| `00852B50` | `00852B64` | `BSP_SubmarineUnit_DepthStepShallower_Unreferenced` | no reference |
| `00852B70` | `00852B85` | `BSP_SubmarineUnit_DepthStepDeeper_Unreferenced` | no reference |

To establish "no reference", I scanned the whole image for E8/E9 rel32 and absolute dwords
(local/g53_refscan.py). The positive control was 008528B0: the scan found its call sites,
including 00852B5E and 00852B7F.

**`008571F0` is the wake emitter, not homing** (`torpedo_wake_step_008571f0`). Each step adds
dt to +498h. While the water mode is non-zero and the emitter +334h exists, it places the
emitter at the "bubor" point, sets emitter+54h to the speed and writes the forward row. The one
branch on HomingVertTurnSpeed (+F4h) only manages the emitter's type-4 children: they are
retired while the torpedo is below `2*swimdepth`, and the emitter is stopped (00867B10,
00866B70) when it is shallower and has none. Presentation only, with no RNG.

**The homing guidance is `00856BB0`** (fragment `torpedo_homing_turn_00856bb0`,
`torpedo_homing_countdown_00856bb0`, `torpedo_homing_pick_target_00856c50`):

- Countdown: `dt < +518h` gives `+518h -= dt`; otherwise `+518h += +514h - dt` and a scan runs.
- Scan: needs +3BCh. It clears the target link, then walks `[008053C0([+3BCh]+54h)]+DE8h`.
  Only `IsKindOf(8)` entities (submarines) count, and the nearest by squared distance wins.
- Turns: +46Ch is set to the target yaw. With HomingVertTurnSpeed > 0, a submarine target and
  `00852820(target)` false (target submerged), the hull rotates about the right row (+74h) by
  `clamp(wrap(own_pitch - target_pitch), ±Vert*dt)`. It then rotates about the up row (+84h)
  by `clamp(-wrap(own_yaw - target_yaw), ±Horz*dt)`, and the function returns without running
  the depth and heading controllers. Otherwise it rotates about the right row by own_pitch.
- **The second turn reads the yaw pair (fixed in the host model).** Take E = ESP at 00856E6C:
  `SUB ESP,8` followed by the call to 00438B10 (itself `RET 8`, 00438B4C) leaves E, and
  0085E880 (`RET 8`) leaves E again. So 00856EE8 reads [E+10h] (target yaw) and 00856EF3 reads
  [E+14h] (own yaw): the second turn is a real bearing turn. `src/bomb_torpedo_tick.cpp`
  `torpedo_steer_00856bb0` used to feed the pitch error to it; commit 853396a79 (cc13-gpr53c)
  put the yaw pair behind `kTorpedoHomingYawPairBound` and corrected docs/TORPEDO_TICK.md, and
  6e7850ad1 switched it ON. The model is still not called by the live host (the gunnery host
  runs its own swim with no homing), so neither commit changes a run today.
- I checked the depth and heading controllers that follow (00856F83..0085714E) against the
  same file. `torpedo_depth_velocity`, the ±5 clamp (00CE3850 / 00CFBC84), `atan2(fwd.x,
  fwd.z)`, and `HeadingTurn * pi / 180 * modifier` all match.
- Use in this installation: only "Torpedo Fido Homing Airplane" (`bulletclasses.lua`
  RealisticTable[4] and the arcade twin) has HomingHorzTurnSpeed and HomingVertTurnSpeed (0.5
  each). It is the aircraft-dropped anti-submarine torpedo, and it homes only on submarines.

**`00853E10` was misnamed "Serialize"**. It is slot F0h of the submarine vtable (00D0C024), and
it reads state rather than writing it. It is now ledger and Ghidra
`BSP_SubmarineUnit_ActivateLoadState`, and `submarine_activate_00853e10` models it:

- creation kind 3 (saved game) loads the "_submarine" table (the eight recovered field names
  are unchanged);
- creation kind 1 with an "AutoAttackTarget" property issues attackmove (00E08F78);
- off a network client, a unit with DummyObjectID +6B8h >= 0 draws **U(0, 1) on stream 1**
  (008540EA) and is ordered to moveto (00E08F68) a point 250 m ahead, at heading ±0.3 rad, at
  band-1 depth;
- finally +1279h = "has an IsKindOf(28h) child". This is the flag 008522C0 tests.

Host comments in submarine_model.hpp and docs/SUBMARINE_MODEL.md still call it the save schema.

**Small models.**

- `00857C80` torpedo boat SEntityInit: +1188h = class+808h "TurboTime", +118Ch = 0, and an
  optional director flags message. No turbo tick was found in this segment.
- `00852300` / `00852350`: the HUD child counters.
- `0085EA40`: the water mine's sensor category, always 4.

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
|---|---|---|---|---|
| `00852410` predict depth | the labelled "swim depth (00852410) is not modelled" at src/game_hosts_gunnery.cpp:730 and :11781 (0072C970's value, today the default) | target hull row: world y, `bands_1200`, clearance +123Ch (submarine_model.hpp `SubmarineUnitOffsets`), depthLevel, sink +1270h, needAir; the body's linear y velocity (the dive step's `linear[1]`); class UpDownAccel/UpSpeed/DownSpeed (`SubmarineDiveInputs`); 008522C0 | none | OFF: unchanged. ON: torpedoes fired through 0072C970 (owner a submarine, Function 7) run at the predicted depth instead of -2.5. In JM06 the submarine's torpedo runs change depth. The mode-2 threshold of 00855F00 (torpedo y below `2*swimdepth`) moves with the depth, so the step mode a torpedo is in changes too. |
| `008522C0` catapult needs surface | `catapult = false` at src/game_hosts_script_orders.cpp:3500; the 008522C0 substitution at src/game_hosts_units.cpp:9529 | unit+1279h (not in the host row today); child list: catapult kind and +490h | none | ON matters only for submarines that carry seaplanes (none in JM06). Those surface while a launch is pending. |
| `00852740` / `008527A0` / `00852820` | 00852820 is already inlined at src/game_hosts_gunnery.cpp:752; the other two have no host consumer | the hull's y and bands | none | none for 00852820 (same rule). The other two are available for deck-gun and kind-0Eh gates. |
| `00852B50` / `00852B70` | the depth up/down steps (no host stub found) | depthLevel | none | none until a caller is bound |
| `00855A90` derive | `torpedo_cached_range` / `torpedo_terminal_fall_speed` (src/bomb_torpedo_tick.cpp) | class row | none | none; the same numbers |
| `00856050` / `00856420` / `00857780` torpedo create/activate | the gunnery host's torpedo spawn defaults (src/game_hosts_gunnery.cpp:502 notes the create path) | class row (V0, WaterTravelSpeed), launch params (order heading, 0072C970 depth, target), shot in-water byte | **one draw per torpedo, stream 1** (00856132), -U(0,1) into the off-scan countdown +518h | OFF: unchanged. ON: each torpedo created shifts stream-1 consumers by one draw; the first homing off-scan is phased by the draw; accel = 0.6 * WaterTravelSpeed. |
| `008568E0` water entry | src/game_hosts_gunnery.cpp:9399, which applies only the MaxWaterHitVel limit. Its comment calls the second limit the round's "own depth field", but shot+0Ch is the **vertical velocity** (shot+8h..+10h is the velocity 00856420 writes at +318h) | velocity, class MaxWaterHitVel and terminal fall speed (00855A90), heading, network flag | none | ON: torpedoes falling faster than `sqrt(2 * 9.81 * MaxFall)` break up on entry. That hits high releases by torpedo bombers (JM06 air attacks) that pass the speed limit. |
| `00857160` / `00855CC0` | network create (the host has no MT torpedo create) | state, net ids | none | none in single player |
| water mine (`0085EBC0`, `0085ECD0`, `0085F160`, `0085F6D0`, `0085F7A0`, `0085F500`) | nothing (the host has no water mine; src/game_hosts_ai.cpp:6538 lists only the class name) | class row; world water height (0078CF20); the ships list with each ship's inverse transform and class Length/Width/Height (+A0h/+A4h/+A8h); the depth-charge list | **two draws on stream 1**: activate U(1-r, 1+r) (off a client) and kill U(BlastDamageMin, Max) | OFF: unchanged. ON: missions that lay mines get rounds that sink to FloatingDepth, arm when nearly at rest, and detonate on ship boxes or depth charges. Every mine shifts stream-1 consumers by two draws. JM06 is not known to use mines (not checked). |
| `00852970` debris | nothing (presentation) | node matrix, owner velocity | none | presentation only |
| `00856BB0` homing (fragment) | `torpedo_steer_00856bb0` in src/bomb_torpedo_tick.cpp (not wired in the gunnery host: game_hosts_gunnery.cpp runs its own swim, :262) | target list (submarines), own and target pitch/yaw, the 00852820 test on the target, class Homing speeds, +514h/+518h | none here; the +518h phase comes from the 00856132 stream-1 draw | OFF: unchanged. ON: Fido torpedoes (JM06 air attacks on the player's boat) home in pitch and **bearing**, and stop depth-keeping while homed. The host model now turns by bearing too (`kTorpedoHomingYawPairBound`, ON); with that switch OFF it would turn by the pitch error. |
| `00853E10` submarine activate | the units host's submarine spawn (no submarine-side AutoAttackTarget or DummyObjectID moveto found; the host reads AutoAttackTarget for squadrons, game_hosts_lua.cpp:3966) | creation kind and record, the scene property, DummyObjectID, heading, pose, band 1 | **stream 1, one draw per dummy-object submarine** (008540EA) | OFF: unchanged. ON: saved-game loads restore air/depth/periscope; spawned subs with AutoAttackTarget get an attackmove; dummy-object subs wander 250 m off at ±17 deg. |
| `008571F0` wake | nothing (presentation) | emitter, bubble point, speed, depth | none | presentation only |

No model draws from stream 0. For comparison, the effects band's sound instance 00873AE0 draws
twice on stream 0 (00873B63, 00873C39); docs/CC13_EFFECTS.md and src/effects_cc13.cpp
already model that (presentation stream).

## 3. Findings and open questions

1. **0072C970 returns a swim depth.** See 1.2. Its ledger name `BSP_Gun_TorpedoLaunchHeading`
   should become a depth name; the owner of segment 40 decides.
2. **Horizontal homing turns by bearing, not pitch** (1.6). Fixed in src/bomb_torpedo_tick.cpp
   and docs/TORPEDO_TICK.md behind `kTorpedoHomingYawPairBound` (853396a79, ON since 6e7850ad1).
3. The host's 008568E0 comment (game_hosts_gunnery.cpp:9399) names the second limit wrongly;
   see the adoption table.
4. 00853E10 is a load, not a save (1.6). The submarine_model comments that call it the save
   schema should be updated by their owner.
5. 00857780 runs the base activate twice for a far-off start position. The packet did not
   establish why. This is faithful to the bytes.
6. Water-mine use in this installation's missions was not surveyed. The torpedo-boat turbo
   (+1188h/+118Ch, TurboTime/TurboStrength) has no consumer in this segment.
7. The 20 effects band models in src/effects_cc13.cpp (docs/CC13_EFFECTS.md) now have ledger
   records (cc13-gpr53d), each checked against the disk bytes first (section 7).

## 4. Ledger

- cc13-gpr53: 29 `add-function` records. Of these, 25 point at src/gameplay_seg53_cc13.cpp and
  4 at checked host sources (00855F00, 008566B0, 00852B90, 00853630, status
  `reconstructed_in_existing_source_checked_by_cc13_gpr53`).
- cc13-gpr53b: 6 more function records (008571F0, 00853E10, 00857C80, 00852300, 00852350,
  0085EA40) and one fragment record at 00856BB0. The fragment sits beside the existing
  bomb_torpedo_tick record, which is untouched.
- config/names: five new names (1.6), and one replacement: 00853E10
  `BSP_SubmarineUnit_Serialize` became `BSP_SubmarineUnit_ActivateLoadState`. The old values
  are in local/ghidra-annotations-*.json.

## 5. Verification

- `scripts/build.ps1`: passes in the worktree after the final merge of origin/main.
- `tools/verify_report_calls.py` on the 26 call-site rows behind sections 1-2
  (local/g53_calls.json): 26 checked, 0 failed.
- No tests were added; no game runs.

## 6. Handoff

Still open, in priority order:

1. (Done: `torpedo_steer_00856bb0`'s second turn, 853396a79 / 6e7850ad1.)
2. Segment 53 code in cc9's gunnery lane, not touched: the turning gun, gunnery AI,
   artillery-director getters and gunnery target records (0085xxxx-00864xxx). 00862CD0,
   00864880 and 00864D90 draw from the RNG.
3. `segment_box_hit_0085cdb0` (src/hit_narrowphase.cpp) has no record. It needs a check
   against the listing before a pointer record is added.
4. (Done in section 7: 00853590, 008533E0, 00853110, 008530E0, 00852D30.)
5. Math helpers: 0085C3F0, 0085E880, 0085E8C0 and the four orthonormalise variants are done
   (section 7.5). Still open: the ellipsoid test 0085C5D0, the sphere-triangle distance
   0085DF60 and the two copy wrappers 0085C0F0 / 0085C220 (no callers).

Functions in the segment that still have no reconstruction record (named, not reconstructed),
grouped by name prefix:

| prefix | count | addresses |
|---|---|---|
| `STL` | 62 | 00851E10, 00861E20, 00861E50, 00861EA0, 00861ED0, 00861F60, 00861F80, 00861FC0, 00862040, 008620E0, 00862110, 00862190, 00862300, 008628D0, 00862900, 00862940, 008629D0, 00862A10, 00862A70, 00862A90, 00863470, 008634A0, 008634F0, 00863500, 00863540, 00863570, 00863B10, 00863B80, 00863BC0, 00863BF0, 00863EA0, 00864090, 00864160, 00864220, 00864290, 00864300, 00864340, 00864550, 008660D0, 00866160, 008665C0, 00866660, 008668F0, 00866BD0, 00869690, 008696C0, 00869890, 00869930, 00869B70, 0086A3C0, 0086A590, 0086A5A0, 0086A9D0, 0086AB50, 0086AB60, 0086AE60, 0086AF80, 0086B6E0, 0086EF20, 0086FA00, 0086FC50, 0086FF20 |
| `BSP_FixedStringBuilder` | 2 | 00851EE0, 00852140 |
| `CG` | 56 | 00851F20, 00853010, 00853020, 00853030, 00853040, 00853380, 008533A0, 00856230, 00856250, 00856340, 00857D80, 00857D90, 00857DA0, 00857DB0, 00857E00, 00858150, 00858220, 008586E0, 0085EAE0, 0085EB00, 0085EB60, 00861BA0, 00862630, 00862750, 008627F0, 00863100, 00864660, 00864FD0, 00865CB0, 00866090, 00866280, 00866300, 00867060, 00869CF0, 0086A400, 0086B710, 0086B850, 0086B9A0, 0086BED0, 0086C240, 0086CD10, 0086CF10, 0086CF70, 0086D080, 0086E5A0, 0086E870, 0086FD70, 0086FF00, 008716E0, 00872280, 00873060, 008730E0, 00873420, 00873890, 00873A90, 00873E90 |
| `BSP_ShipClass` | 1 | 00852200 |
| `TRIV` | 10 | 00852FB0, 00852FC0, 008561D0, 008561E0, 008561F0, 00856200, 00856210, 00858170, 0085EA30, 0086B980 |
| `BSP_ObserverLink14` | 1 | 00855FE0 |
| `Unwind@00856220` | 1 | 00856220 |
| `Unwind@00856240` | 1 | 00856240 |
| `BSP_TorpedoBoatClass` | 2 | 00857D60, 00857E20 |
| `BSP_NodeHandle` | 2 | 008580C0, 008658C0 |
| `TMPL_PointerArray` | 2 | 008580E0, 008583D0 |
| `BSP_NoOpRet4` | 1 | 00858180 |
| `BSP_NoOpRet8` | 1 | 008581B0 |
| `BSP_ReturnFalseByte` | 1 | 008581E0 |
| `BSP_NoOp` | 1 | 00858200 |
| `BSP_NativeTraceline` | 1 | 00858330 |
| `BSP_SessionMessage` | 5 | 00859410, 00859C80, 00859CC0, 00859FA0, 00859FE0 |
| `BSP_TurningGun` | 13 | 00859550, 008598B0, 00859920, 00859A20, 00859B10, 0085A130, 0085A3D0, 0085A8B0, 0085AB50, 0085AD00, 0085B980, 0085BA30, 0085BB50 |
| `BSP_Unit` | 1 | 00859830 |
| `FUN` | 1 | 00859950 |
| `BSP_Gun` | 2 | 0085A9A0, 0085B7D0 |
| `BSP_ReconCategory` | 2 | 0085BCE0, 0085BD50 |
| `BSP_Matrix` | 3 | 0085C0F0, 0085C220, 0085E4D0 |
| `BSP_Ellipsoid` | 1 | 0085C5D0 |
| `BSP_Geometry` | 2 | 0085CDB0, 0085DF60 |
| `BSP_DebugDraw` | 13 | 0085F990, 0085FB30, 0085FB70, 0085FFF0, 00860030, 00860140, 008605C0, 00860710, 008608B0, 00860AD0, 00860BB0, 00860BE0, 00861260 |
| `TMPL_Pair8` | 1 | 00861920 |
| `BSP_UnitGunneryDirectorBridge` | 7 | 00861970, 00861980, 00861990, 008619A0, 008619B0, 008619C0, 008624C0 |
| `TMPL_Record12` | 1 | 008619D0 |
| `TMPL_Handle` | 2 | 008619F0, 00861A00 |
| `BSP_ArtillerySubDirectorBot` | 9 | 00861A10, 00861A30, 00861A50, 00861A70, 00861A90, 00861AB0, 00861AD0, 00861AF0, 00861B10 |
| `BSP_GunneryTargetRecord` | 6 | 00861B30, 00861B40, 00862660, 008626D0, 00862CD0, 00864880 |
| `BSP_FlagGridCC` | 1 | 00861C20 |
| `BSP_UnitGunneryAi` | 14 | 00861CD0, 00861D20, 00861D70, 00861DC0, 00863130, 008636A0, 00863780, 00863840, 008638B0, 00863920, 00863A80, 00864380, 00864510, 00864CA0 |
| `TMPL_Buffer` | 2 | 00862280, 00862810 |
| `TMPL_Array12` | 2 | 00862390, 00862B00 |
| `BSP_UnitGunneryCategoryGate` | 1 | 008627A0 |
| `TMPL_VectorAt48` | 1 | 00862C00 |
| `BSP_Entity` | 1 | 00863620 |
| `BSP_UnitGunneryTarget` | 1 | 00863640 |
| `Unwind@00864500` | 1 | 00864500 |
| `BSP_UnitGunneryVisibility` | 3 | 00864680, 00864BA0, 00864D90 |
| `BSP_WorldAtmosphere` | 1 | 008658E0 |
| `BSP_RainDescriptor` | 2 | 00865B80, 00865CF0 |
| `BSP_OwnedObject` | 1 | 00865E30 |
| `BSP_NativePointerVector` | 3 | 00865E90, 008661B0, 00866690 |
| `BSP_RefPtr` | 11 | 00866730, 00866970, 00866C30, 0086AC50, 0086AC80, 0086ACB0, 0086ACE0, 0086AFC0, 0086B080, 0086B750, 00873AB0 |
| `BSP_EffectDeletionLock` | 1 | 00866760 |
| `BSP_EffectManager` | 2 | 008669A0, 008674A0 |
| `BSP_Effects` | 1 | 00869D20 |
| `BSP_EffectParticle` | 2 | 00869D70, 0086BC50 |
| `BSP_NativeDwordVector` | 6 | 0086A280, 0086A910, 0086A950, 0086AA10, 0086B630, 00872E70 |
| `BSP_NativeQwordVector` | 4 | 0086A350, 0086A990, 0086AAE0, 0086AED0 |
| `BSP_ParticleComponentResourcePointers` | 2 | 0086AAA0, 0086AE90 |
| `BSP_NativeString` | 1 | 0086B290 |
| `BSP_EffectComponent` | 1 | 0086B7C0 |
| `BSP_EffectTracer` | 2 | 0086B990, 0086B9B0 |
| `BSP_EffectWaterTracer` | 1 | 0086C120 |
| `BSP_EffectShake` | 1 | 0086BD60 |
| `BSP_EffectNamedRecord14` | 1 | 0086CDB0 |
| `BSP_EffectNamedRecord14Array` | 4 | 0086CE10, 0086E670, 0086E6F0, 0086F900 |
| `BSP_EffectConstRumble` | 1 | 0086CFC0 |
| `BSP_EffectSlopeRumble` | 1 | 0086D010 |
| `BSP_EffectSquareRumble` | 1 | 0086D060 |
| `BSP_PooledStringListOwner` | 1 | 0086FC90 |
| `BSP_ParticleResourceRecord` | 1 | 0086FD90 |
| `BSP_ParticleResourceRecords` | 1 | 00871390 |
| `BSP_GameplayEffect` | 1 | 00871A90 |
| `BSP_RegisteredModelEffect` | 2 | 00871FF0, 00872770 |
| `BSP_RegisteredType4Effect` | 1 | 00872080 |
| `BSP_EffectNode` | 1 | 008721A0 |
| `BSP_NodeRef` | 3 | 00872220, 00872240, 00872260 |
| `BSP_LightningStormOwner` | 1 | 00872E80 |

## 7. Pass cc13-gpr53d

Same terms as cc13-gpr53/53b: models in `src/gameplay_seg53_cc13.cpp` and
`include/bsp/gameplay_seg53_cc13.hpp` only, nothing wired, no host edits, no game runs. Every
model below is **reconstructed and build-tested**; none is fixture-tested, ABI-compatible or
game-validated. Each was read from the disk bytes (`bsp.py disasm-raw`), with the x87 operand
forms checked against the raw opcode bytes.

### 7.1 Torpedo homing and the effects records

- The second homing turn of 00856BB0 is fixed in the host model behind
  `kTorpedoHomingYawPairBound` (853396a79, ON since 6e7850ad1); sections 1.6, 2 and 3 and the
  header now say so.
- The 20 models in src/effects_cc13.cpp got `add-function` records after each was checked
  against the bytes: 00873AE0, 00872630, 00872650, 00872670, 00872690, 008726C0, 00872720,
  00872F40, 00872FB0, 00873DE0, 008721C0, 00872DF0, 00872E30, 008720F0, 008720A0, 008720D0,
  00872120, 00872BF0, 00872D30 and 00865FE0. No discrepancy was found. The x87 forms: 008726FA
  `DE EA` FSUBP ST(2) gives `hi - lo`, 00873C55 `DE E9` FSUBP ST(1) gives `u - 2s * 0.5`.

### 7.2 Models

| address | model | what it does |
|---|---|---|
| `00852F10` | `submarine_construct_defaults_00852f10`, `kSubmarineVptrs_00852f10` | After 0081ED40(flag): eight vptrs, then +1228h = 0, air +127Ch = 1.0f, unlimitedAir/needAir/+1235h = 0, +1238h = 0, class id +C4h = 8, +1274h = 0.0f. |
| `008531A0` | `submarine_create_instance_008531a0` | new(1288h), memset of the raw pointer, 00852F10, 009553D0(descriptor), +354h = descriptor. A failed allocation is dereferenced (no guard). |
| `00852D30` | `submarine_describe_state_00852d30` | The "_submarine" save table 00853E10 reads back: `airSupply{air, needAir, unlimitedAir}`, depthLevel, periscopeState, periscopeY, periscopeOut, sullyesztoEro. Keys and tagged values pass by value; sink +4h opens a table, +0Ch writes a field, +8h closes it. |
| `00855D30` | `torpedo_write_state_00855d30` | The "_torpedo" table 00857780 reads back. It writes **swimdepth twice** (00855D8C and 00855E58). |
| `00856360` / `0085F270` | `torpedo_register_in_world_lists_00856360`, `water_mine_register_in_world_lists_0085f270` | 00928560, then push_back on world lists +30h, +210h and +21Ch (torpedo) or +288h (water mine). |
| `008530E0` / `008563A0` / `00857F00` / `0085F2B0` | `*_detach_from_world_lists_*` | Base detach (006DFFC0 for ships, 006E29C0 for the bomb family), then the first node whose payload is the entity in the class list (+78h submarine, +21Ch torpedo, +C0h torpedo boat, +288h water mine) is erased. |
| `00853110` | `submarine_destruct_00853110` | Releases the periscope effect +1228h, then the vehicle base 0081F3A0. |
| `00853590` | `submarine_on_killed_release_periscope_00853590` | Stops the periscope effect's children, flags it finished (+9), releases it; frees the debris piece +1224h and its name; tail 00819880. |
| `008533E0` | `periscope_debris_init_008533e0` | Zero velocity/angular velocity (00F87574), state 0, name `"PeriszkopSullyesztoDolog:" + owner name`, matrix copied from the periscope node. |
| `008562A0` | `kTorpedoDestructSteps_008562a0`, `kTorpedoVptrs_008562a0` | Vptrs, members +4FCh (0042BCA0) and +4E4h (004B7EF0), base 006E2A20. |
| `0085E9C0` / `00857CD0` | `kWaterMineVptrs_0085e9c0`, `kTorpedoBoatVptrs_00857cd0` | Base constructor, vptr table, class id 34h / 0Eh. Nothing else is written. |
| `0085F310` | `water_mine_create_0085f310` | The 00856420 pattern without launch params: velocity = dir * V0, forward row = dir, position, children invalidated. |
| `0085F1E0` | `water_mine_arm_0085f1e0` | armed = 1; a 1ACh part instance (007135C0) of the mine's model +464h into +468h; root registration 00B6D890. This completes the marker that 0085F2F0 left partial. |
| `0085EA50` / `0085F120` | `water_mine_model_pointer_0085ea50`, `water_mine_release_sub_objects_0085f120` | The armed part when armed, else null. Release clears +464h **before** deleting the part, then 006E1860. |
| `008738B0` | `update_effect_light_instance_008738b0` | Light position, link, intensity `max((t / duration) * peak, 0.01)`, `t -= dt` floored at 0. |
| `00873100` | `update_effect_waterdrops_instance_00873100`, `waterdrops_spawn_count_00873100` | Once: count = trunc(WaterdropCount * (1 - clamp((dist - Fade) / Fade, 0, 1))); a non-zero count creates and spawns the drops with the ten setters. Each tick: 00BC12F0(reference node, camera position, dt). |
| `00873C80` | `update_effect_thunderstorm_instance_00873c80` | Atlas tick; on a storm strike, a fire-and-forget point effect at the strike position from the world's strike definition (+19E0h). |

Checked host rows: 00856260 and 0085EB20 (the class tests) match src/entity_class_ids.cpp rows
2Bh and 34h exactly (CMP chain 2Bh/34h, 2Ah, 2, 1, 0, then [ECX+C4h]).

Not modelled: the base effect slot defaults 00858170..00858210 are bare `RET 4` / `RET 8` /
`RET` / `XOR AL,AL` leaves. Their bodies say nothing about the slots' meaning, so there is
nothing to model. The adjustor thunks 00853010..00853040 and the scalar deleting destructors
00853380 / 008533A0 are compiler-generated.

### 7.3 Adoption notes

| model | replaces (host file:line) | inputs | RNG | OFF-switch prediction |
|---|---|---|---|---|
| `00852F10` / `008531A0` | nothing; the host builds submarines in its own unit row | flag, class descriptor | none | none |
| `00852D30` / `00855D30` writers | nothing (no host save writer; 00853E10 and 00857780 are the readers) | the submarine fields; the torpedo's seven floats | none | none until saved games are written |
| list register/detach | `world_lists_unlink_unit_00928f24` at src/game_hosts_units.cpp:3044 erases the first matching node of every list, which is the same set for these leaves | the class list payloads | none | none: same unlink set |
| `00853110` / `00853590` / `008533E0` | nothing (the host has no periscope debris or periscope effect) | effect and debris pointers, the owner name, the periscope node | none | presentation only; ON adds the debris piece and the effect release on a kill |
| `0085F1E0` / `0085EA50` / `0085F120` / `0085F310` | nothing (the host has no water mine; src/game_hosts_ai.cpp:6538 lists only the name) | the mine's model +464h, the class V0, spawn position and direction | none (the mine's draws are in 0085EBC0 and 0085F7A0, section 2) | none until mines are bound |
| `008738B0` / `00873100` / `00873C80` | nothing (the effect components are built by src/gameplay_effect_components.cpp but not ticked) | the instance fields, the definition rows (`WaterdropCount` etc., src/gameplay_effect_scalar_components.cpp), node positions, the weather renderer 00BC1xxx | none in these bodies. The storm's strike timing lives in 00BC0510, whose lightning draw 00BC05A9 is on stream 1 (docs/CC13_RNG_STREAM_CENSUS.md) | presentation only; nothing in a gameplay pair moves |

### 7.4 Findings

1. **The submarine destructor does not free the periscope debris.** Only 00853590 (on kill)
   frees +1224h. A scan for disp32 1224h finds only 00853590, 00853630 and 00854650, so a
   submarine removed without being killed (mission end, for example) leaks the 6Ch piece and
   its name. The model keeps this.
2. **The torpedo state writer writes swimdepth twice.** This is harmless to the reader.
3. **00852D30 and 00853E10 are the two sides of one schema.** 00852D30 writes the eight
   "_submarine" fields that 00853E10 reads; 00855D30 writes the "_torpedo" fields that
   00857780 reads. Section 1.6's note on the misnamed "Serialize" is now complete.
4. Both create functions (008531A0, 0085F310) run the memset on the raw `operator new` result
   and dereference a null object when the allocation fails. That is the image's behaviour.

### 7.5 Rotation and orthonormalisation helpers

The torpedo homing (00856BB0) and debris step (00852970) models take these as host callbacks.
They now have models of their own:

| address | model | what it does |
|---|---|---|
| `0085C3F0` | `rotate_vector_about_axis_0085c3f0` | Rodrigues about a unit axis. `d = (a.y*v.y + a.x*v.x) + a.z*v.z`, `p = d*a`, `r = v - p`; when `rr = (r.y^2 + r.x^2) + r.z^2` is below (double)1e-4f (00D7A268) it returns v unchanged (a NaN rotates). Otherwise `((r*cos) + p) + (a x r)*sin` (004F9B30 cross), each component a float. 7 callers, including 00730160 `BSP_Gun_Fire`. |
| `0085E880` | `rotate_basis_about_axis_0085e880` | Rotates rows 2 and then 1, then runs 0085DC80 (calls the existing `orthonormalize_pose_matrix_0085dc80`). |
| `0085E8C0` | `rotate_about_axis_through_point_0085e8c0` | 0085E880, plus row 3 = pivot + R(row 3 - pivot). |
| `0085D3D0` / `0085D770` / `0085D920` / `0085DAD0` | `orthonormalize_from_row_*` | One shape: normalise A, take A's component out of B, normalise B, C = cross, normalise C. The rows are (0, 1, 2 = 0x1), (2, 0, 1 = 2x0), (1, 0, 2 = 0x1) and (1, 2, 0 = 1x2). normalise uses `inv = L > 0 ? 1/L : 0` (FCOMI/JBE; a NaN length gives 0), and 00419440 is `sqrt((x*x + y*y) + z*z)`. |

Uncertainty: FSIN/FCOS are modelled as double `sin`/`cos` rounded to float. That can differ from
the x87 result in the last bit, but no other step can, given 24-bit precision. The x87 operand
forms were read from the bytes: 0085DAF3 `DE F1` (1/L) and the cross 004F9B30 (x = a.y*b.z - a.z*b.y).

Adoption: these are pure functions with no RNG. They replace the `rotate_about_axis_0085c3f0`
callback in src/gameplay_seg61_62_cc13.cpp:562 and the `orthonormalize_0085dad0` host callback in
src/engine_seg43_45_cc13.cpp:47, and they back this file's homing and debris callbacks.
Prediction with the switch OFF: unchanged. ON: identical results except the last-bit FSIN/FCOS
case.

Verification for this pass: `scripts/build.ps1` passes. `tools/verify_report_calls.py
local/g53d_calls.json` checked 94 call rows with 0 failures. No tests were added and there were no game runs.

### 7.6 Weather, point-effect handle and sound component

| address | model | what it does |
|---|---|---|
| `00865900` | `world_atmosphere_apply_preset_00865900` | Releases the old rain emitter. With IsRain it copies speed, drop count (CVTSI2SS), min length, length span (max - min) and colour into the rain descriptor and builds the emitter (008023B0). With a camera it runs the five fog scalar setters in listing order (+D0h, +DCh, +D4h, +D8h, +E0h), the fog colour, the four directional colours, the camera clear colour and the cloud wind switch. |
| `00865AB0` | `rain_descriptor_update_00865ab0` | Every frame from BSP_Game_OnMove: with an emitter, 00801400(frame dt [game+21F0h], camera position). |
| `00866B70` | `effect_handle_request_stop_00866b70` | Under the effect manager lock: +9 = 0, and +0Ah = 1 when +8 is set. |
| `008679D0` | `point_effect_retire_child_008679d0` | Under the lock: finds the child, stops it when active (vtable+30h), appends it to the pending array +18h (00867320 retains), releases the slot and nulls it **in place**, then releases the by-value argument. 00865FE0's "all slots null" test depends on that in-place null. |
| `00869E10` | `effect_sound_any_sample_flag15_00869e10` | Whether any sample's options byte +15h is set. |
| `0086A740` | `effect_sound_admit_at_position_0086a740` | +2Ch admits. Otherwise each sample admits on 00A818D0, options +15h, or `limit > distance`, with `limit = +20h ? +10h : +0Ch * 128`. |
| `0086FA80` | `effect_sound_category_0086fa80` | 2. |

RNG: none in these bodies. **Correction for docs/CC13_SCENE_WEATHER.md** (not this packet's
file): it says the rain spawn step 00801400 draws 00BD2F10 on stream 0. The bytes say stream 1
(`00801439 MOV ECX,1`, call at 0080144F, no ECX write in between). docs/CC13_RNG_STREAM_CENSUS.md
and `rain_emitter_step_00801400` (src/engine_seg49_64_78_cc13.cpp) already say stream 1, so rain
does not shift the gameplay stream-0 sequence.

Adoption: presentation only. 00865AB0 replaces the `update_rain_descriptor_00865ab0` record-only
stub in src/game_hosts_mission_frame.cpp:1483 (cc9's file; not edited) and would drive the
existing `rain_emitter_step_00801400`. Inputs: the frame dt at game+21F0h and the camera node.
Prediction with the switch OFF: unchanged. ON: rain on the 18 IsRain scenes, with stream-1 draws
(presentation) and no stream-0 shift.

`verify_report_calls` over the pass's call rows: 113 checked, 0 failed.

### 7.7 Effect instance constructors and destructors

These complete the lifecycle around the src/effects_cc13.cpp virtuals, and use that file's storage
types (`EffectNodeInstanceStorage`, `EffectSplashInstanceStorage`, the 1Ch prefix).

| address | model | what it does |
|---|---|---|
| `00873010` | `construct_effect_flare_instance_00873010` | Prefix, type 5, payload 0. The flare object is not created here. |
| `00873090` | `construct_effect_waterdrops_instance_00873090` | Prefix, type 0, payload 0, spawn-pending byte +20h = 1. |
| `008739A0` | `construct_effect_splash_instance_008739a0` | Prefix, type 0, then the water ring 00B9E3F0(point position, def+20h/+24h/+28h). The ocean pointer is **not** null-tested here, although the destructor tests it. |
| `00873EB0` | `construct_effect_thunderstorm_instance_00873eb0` | Prefix, type 0, loads the lightning atlas, sky effect +1Ch through [010909B0] vtable+10h, storm +20h (54h, 00BC0D40) only when def+54h is set. |
| `00873FF0` | `destruct_effect_thunderstorm_instance_00873ff0` | Destroys and frees the storm, then unloads the atlas. **The sky effect +1Ch is not released.** |
| `00872C10` / `00872CB0` | `destruct_effect_waterdrops_instance_00872c10`, `destruct_effect_light_instance_00872cb0` | The same body as their Stop slots (00872BF0 / 00872D30); the models call those. |
| `00872D60` | `destruct_effect_splash_instance_00872d60` | Removes the ring only when the ocean exists. Unlike Stop 00872DF0, it leaves the timer and +1Ch alone. |

Not modelled: the light constructor 008740E0, which sits at the segment's end address and was
left out of this lease. The per-kind CreateInstance wrappers (operator new plus the constructor)
are not modelled either.

Findings: the thunderstorm destructor does not release its sky effect +1Ch, and the splash
constructor dereferences the ocean without the null test that its destructor makes. Both models
keep the image's behaviour. RNG: none. Adoption: presentation only, nothing to replace in the host
(the host does not tick effect instances). `verify_report_calls`: 127 rows, 0 failed.

### 7.8 Handoff (cc13-gpr53d)

Totals for this pass: 20 effects_cc13 records (section 7.1) plus 49 segment-53 records: 45
models in this packet's files, the two trivial thunderstorm slots 00872100 and 00872110, and
2 checked host rows (00856260, 0085EB20). All are build-tested; none is wired, fixture-tested or
game-validated. Section 6's open table now lists 337 addresses.

Open, in value order (items 1 to 3 and most of 4 were done by cc13-gpr53e; see section 7.9):

1. `BSP_EffectTraceLine_ReadLua` 00858700 reads Width (x0.5), WidthSpeed, WidthSpeedEnd,
   MaxSegmentNum (default 10), MinSegmentLength (1.0), SegmentLifeTime (1.0), OuterColor,
   AnimOnOff, AnimLoop, AnimStartFrame, AnimEndFrame (defaults to start), AnimPlaySpeed and
   TextureName (frame list, "cloud_snake.dds" fallback). The decompiler shows negative offsets
   from ECX, so the field base has to be taken from the listing before a model is written.
   After that: the trace-line instance Update 008585D0, the destructor 00858530 and
   CreateInstance 00859240.
2. The water tracer reader 0086D180 and its curves 0086C120 (presentation).
3. The light instance constructor 008740E0, at the segment end address (not leased here).
4. Small wrappers: 00855B00, 008559D0 / 0085EAC0, 00855BB0, 00855F50, 0085F1A0, 00857C20 /
   00852210 (motion controller variants, docs/UNIT_CONTROLLER_UPDATE.md), the per-kind
   CreateInstance functions (00868D00, 00869300..008695B0), 0085C0F0 / 0085C220 (no callers),
   and 0085C5D0 (no reference at all; refscan positive control 0085DC80 gave 70 hits).
5. The gunnery lane (0085xxxx-00864xxx turning gun, gunnery AI, director bridge and target
   records) and 0085DF60 / 0085CDB0, both already in cc9's gunnery host, were not touched.
6. Notes for other owners: docs/CC13_SCENE_WEATHER.md says the rain step draws on stream 0;
   the bytes say stream 1 (section 7.6). 0072C970's ledger name and the submarine_model
   "save schema" comments are as section 3 lists.

## 7.9 Pass cc13-gpr53e

Same terms: models only in `src/gameplay_seg53_cc13.cpp` and `include/bsp/gameplay_seg53_cc13.hpp`,
nothing wired, no host edits, no game runs, gunnery lane skipped. Every model is **reconstructed
and build-tested**; none is fixture-tested, ABI-compatible or game-validated. All bodies were read
from the disk bytes (`bsp.py disasm-raw`), strings and constants from the PE
(`local/g53e_peread.py`).

### 7.9.1 The trace-line ("Tracer") effect

**The ECX base of 00858700.** The "Tracer" row of 00870400 allocates A0h bytes, and 0086B8B0
builds a trace-line definition at +0 (vtable 00D0D5B0) and the effect component at +80h (vtable
00D0D590). The loader passes the component pointer to the reader slot (vtable +14h), so ECX is
allocation + 80h, and the reader's `[ESI-6Ch]`..`[ESI-4]` stores land at definition +14h..+7Ch. The
types agree with 0086B8B0's defaults (+14h = 0.1f, +20h = 3, +28h = FLT_MAX, +2Ch = 10.0f, +4 = 0,
+5 = 1). CreateInstance 00859240 confirms the base: it takes the definition in ECX and stores
`definition + 80h` as the instance's component.

| address | model | what it does |
|---|---|---|
| `00858700` | `trace_line_read_lua_00858700` | 31 keys in listing order (section 7.8 listed the first 13). Width x 0.5 -> +14h, WidthSpeed, WidthSpeedEnd, MaxSegmentNum [10], MinSegmentLength [1], SegmentLifeTime [1], OuterColor (packed BGRA), AnimOnOff / AnimLoop [false], AnimStartFrame [0], AnimEndFrame [the start], AnimPlaySpeed [0]. TextureName loads into the {data, count, capacity} array +4Ch. Then ShowBullet, BulletTailTexture ["tr_bullet.dds"], BulletHeadTexture ["tr_glow_add.dds"], TailLength [16], HeadRadius [0.8], BulletLifeTime [2], BulletFadeBegin [1.6], +7Ch = life - fade begin, TailColor [FFFFFFFFh when nil], HeadColor [87FF8032h when nil] and UVTileLength [1]. Last come the vertex format "pf43uf43ccuf44uf41.mvfm" (+40h) and its vtable+40h binding (+44h). |
| `00858660` | `trace_line_pointer_array_append_00858660` | count == capacity -> n = 2c + 2, grow when n > c. The same code is inlined four times in 00858700. |
| `00859240` / `00859370` | `trace_line_create_instance_00859240`, `..._slot_00859370` | Instance new(20h): prefix, +10h owner (EDX), +14h component, +18h = 3. Node 00AF3430(1B8h) + 00858260("TraceLine"), vtable 00D0C928, then node vtable+5Ch(definition, world root). The slot passes EDX = [ESP+4] and ECX - 80h. |
| `008585D0` | `trace_line_instance_update_008585d0` | Refreshes the owner node, then 00AF2630(node, world position, dt) and 00AF1EB0(node, dt). Sets node+194h = 1 when the owner is stopping (+9). RET 8; the second argument is unused. |
| `00858530` / `008583A0` | `..._destruct_00858530`, `..._detach_from_root_008583a0` | When node+A4h is the world root, 00B6D890(node, root). The destructor then runs 00B6DFA0 and 00BD30F0. 008583A0 is vtable +2Ch, which is the destructor's first step. |
| `008580A0` / `00858500` / `00858520` | `is_complete`, `set_19c`, `stop` | vtable +08h: node+190h == 0. +14h: node+19Ch = arg. +30h Stop: node+1A8h = 1. |

**Findings.** (1) The texture loop stops at the **first** frame with no atlas item
(00858BC7 -> 00858C81 -> 00858CA8). Later frames are never tried. The same thing happens in
`load_native_tracer_particle_textures_00b0a920`, which already models it. (2) When TextureName is
not a string, the item for "cloud_snake.dds" is appended **without** a null test, so a missing
atlas item appends a null pointer. (3) Neither allocation in 00859240 is null-tested on the
later path: a failed instance faults at 0085930C, and a failed node faults at 00859346. (4) The
vtable+40h block (00859200) has only +0 (the format) and +10h (= 1) written. Its +4h..+0Ch are
uninitialised stack.

### 7.9.2 The water tracer reader 0086D180

ECX is the component itself (C4h, vtable 00D0D5D4), so the offsets are positive. The model is
`water_tracer_read_lua_0086d180`. The section 7.8 key list (from cc13-effects1's ledger note) was
partly wrong: the bytes give ShowForward, ShowBackwards, FitToWater, Lifetime, TimeScale, Divisions,
LineDistance, LOD, FullLines, TextureRate, TextureFile, TextureFile2, AlphaScale, AlphaScaler,
WidthScale, WidthScaler, WidthOffset, BoneLength, BoneCount, Length, HeightScale, Alpha, Width,
HeightX, HeightY, FadeIn, StartTime, StartSpeed, StopTime, StopSpeed, LocalSpace, WidthWave,
WidthSpeed, HeadingLines, HeadingLength, SegmentCount, VertTessPower, HorizDivisions,
HorizTessPower, DisplacementScale and WaterSpray. Each key's field and default is in the header
comment. 0086C120 (ledger "DestroyCurves") is the destructor of the six-array WaterSpray local,
which runs in reverse field order. It is the `WaterSprayDefinition` destructor in the model.

Findings:

1. **The HeightY curve is gated on HeightX.** 0086D9C8 pushes "HeightX" for the IsTable test and
   0086DA0C pushes "HeightY" for the load. A row with HeightY but no HeightX table gets a null
   +70h curve.
2. **TimeScale is the curve scale, not a field.** It is stored in [ESP+2Ch] and passed as the
   third argument of 00869E40 for Alpha, Width, HeightX and HeightY. WidthSpeed gets 1.0 and is
   loaded with no table test.
3. Each curve field is released and replaced **even when the gate fails** (it becomes null).
   +98h, +9Ch and +C0h are zeroed at entry without a release.
4. The Divisions clamp is unsigned (`CMP / JAE`), so a negative Divisions passes unchanged. The
   Lifetime and LineDistance clamps compare against (double)0.1 and store 0.1f. A NaN passes.
5. FitToWater == 1 forces BoneCount = 40 and BoneLength = max-like(f(Length / 39.0), 3.0f). The
   NaN case keeps the quotient.
6. HeightScale is written to +60h and then overwritten by DisplacementScale. Only
   DisplacementScale reaches the 44h block.
7. **Correction for the 00BADC70 comment in include/bsp/water_weather_cc13.hpp** (another file;
   not edited). Block +0Ch is not a DWORD: it is `&component+68h`, the Width curve holder. +10h and
   +14h are &+6Ch and &+70h. Block +4h is SETNE CL over ECX = &+68h, so its upper three bytes are
   address bits. The rest map as follows: +0 BoneCount, +8 LocalSpace, +18h SegmentCount (unread),
   +1Ch VertTessPower, +20h HorizDivisions, +24h HorizTessPower, +28h..+40h WidthScaler,
   WidthOffset, WidthScale, DisplacementScale, Length, TextureRate, AlphaScaler.

### 7.9.3 The light constructor and the CreateInstance family

| address | model | what it does |
|---|---|---|
| `008740E0` | `construct_effect_light_instance_008740e0` | Prefix, type 0, tables 00D0DBF0 / 00D0DBEC. Light = 00B7BD30(1FCh), and 00B7C710 names it "dynamic_light_" + [[point+84h]+1Ch]. Then 004B62E0 sets the diffuse from def+28h, light+1C4h = specular def+38h, light+1F8h = radius when > 0.01f (else 0.01f, including NaN), and +20h = Duration. |
| `00868D00`, `00869300`, `00869370`, `008693E0`, `00869450`, `008695B0` | `effect_create_instance_00869300_family`, `kEffectInstanceKinds` | new(20h/20h/24h/24h/24h/20h) + the kind's constructor (00873AE0, 00873010, 00873EB0, 00873090, 008740E0, 008739A0), or 0. |

Finding: in the light constructor, the light is **not null-tested** after the allocation. A failed
00B7BD30 faults in 004B62E0 (00874212). The section 7.7 light Update (008738B0) also uses
light+1F8h, but as the intensity. Here the constructor writes the clamped Radius into it.

### 7.9.4 Torpedo, water-mine and unit wrappers

| address | model | what it does |
|---|---|---|
| `00855B00` | `torpedo_commit_step_pose_00855b00` | 006E1010, then copies node(+BCh)+74h to +C0h (004134F0). |
| `008559D0` / `0085EAC0` | `forward_slot_198_to_194_008559d0` | own vtable+194h(dt). |
| `00855BB0` / `00855B90` | `torpedo_set_node_visibility_00855bb0` | With +464h: 00B6DA70(model+0Ch, 0.0 or 1.0, 0). 00855B90 is not a Ghidra function. It begins after 00855B30's RET 4 and six CCh bytes (00855B8A..8F), so no record was added for it. |
| `0085F1A0` | `water_mine_place_step_pose_0085f1a0` | 006E1060(dt), then [[[+84h]+160h]+0Ch] vtable+38h(this - 370h). |
| `00855F50` | `torpedo_send_create_message_00855f50` | 00855CC0 message, 00779FC0 send to the peer, 007668E0. |
| `00857C20` / `00852210` | `unit_create_motion_controller_c_00857c20`, `..._b_00852210` | new + 00939F20 / 00939E50 -> unit+1018h. B also sets +1210h = the first body in the 00C31DC0 chain (+208h link) with 00C32450 bit 1. B does not null-test the controller. |

**Ledger correction:** the 00852210 and 00857C20 ledger notes say "operator new(0x390)", but the
PUSH immediates are **39Ch** (00852228) and **394h** (00857C37). The records added here carry the
correct sizes. The name evidence was not edited.

### 7.9.5 Adoption notes

| model | replaces (host file:line) | inputs | RNG | OFF-switch prediction |
|---|---|---|---|---|
| `00858700` | nothing: `GameplayEffectScalarComponentDispatcher::read_lua_slot_14` (src/gameplay_effect_scalar_components.cpp:273) has no 00D0D590 row and hands the Tracer component to `remaining_` | the Lua row accessors, the atlas finder [00F8C26C], the frame helpers 00AF3A20 / 00AF3B50, renderer vtable +38h / +40h | none | presentation only: unchanged with OFF; with ON, Tracer components get their fields and textures |
| `00859240` / `008585D0` / `00858530` and the slots | nothing (the host does not tick effect instances, section 7.7) | owner node position, dt, the world root | none | presentation only |
| `0086D180` | nothing (no 00D0D5D4 row in that dispatcher either) | the row accessors, `load_tracer_parameter_curve_00869e40` (src/tracer_parameter_curve_loading.cpp), `w4_water_tracer_resource_initialize_00badc70` and `w4_water_spray_resource_construct_00bae2e0` (src/water_weather_cc13.cpp) | none in the reader. 00BAE2E0 draws once per spray sub-element from 00BD2F10 **stream 0** in the image; that model already routes the draw through `W4_PresentationRandom`, so wiring must keep a presentation stream (cc9 RNG rule) | presentation only; with ON and the shared stream, every later stream-0 draw would shift, so ON must use the presentation stream |
| `008740E0` and the CreateInstance family | nothing | definition fields (`read_effect_light_00869fd0`), the point's name, light setters | none | presentation only |
| torpedo / water-mine / unit wrappers | nothing wired: the host builds torpedoes and unit controllers its own way (sections 1.3 and 2) | entity fields and host callbacks | none | none until those entities are bound |

### 7.9.6 Handoff (cc13-gpr53e)

Records added in this pass: 26 `add-function` records (00855B90 has none, as noted above). The open
table in section 6 drops 23 addresses, to **314**. Build: `scripts/build.ps1` passes.
`tools/verify_report_calls.py local/g53e_calls.json` checked 78 call rows with 0 failures. No tests
were added, there were no game runs, and no Ghidra annotations were made in this pass.

Still open, in value order:

1. The WaterTracer instance side (0086BD10, 0086BD20, 0086BF10) and the remaining effect
   constructor-side rows (0086BDB0, 0086BDF0, 0086BE30, 0086BE70 and the thunderstorm's 0086F3B0,
   0086FB40, 0086FBC0, 0086FC20).
2. The trace-line leftovers: the two unreferenced pointer-array helpers 008580E0 and 008583D0,
   and the node handle 008580C0.
3. Item 4 of section 7.8, still open: 0085C0F0 / 0085C220 (no callers) and 0085C5D0 (no
   reference).
4. The gunnery lane (unchanged; cc9's).
5. Notes for other owners: the 00BADC70 block comment (7.9.2, item 7) and the 0x390 controller
   size in the 00852210 / 00857C20 ledger names (7.9.4).

## 7.10 Pass cc13-gpr53f

Same terms as 7.9, plus the lead's request to correct the 00BADC70 comment in
`include/bsp/water_weather_cc13.hpp`, which cc13 owns and this pass leased. That comment now says
that block +0Ch is the address of the Width curve holder (component + 68h), and that +10h and +14h
are the HeightX and HeightY holders. Only the comment changed; the code is untouched. Every model is
**reconstructed and build-tested** from the disk bytes.

| address | model | what it does |
|---|---|---|
| `0086BD10` / `0086BD20` | `effect_water_tracer_category_0086bd10`, `..._restart_0086bd20` | 4; `MOV AL, 1`. |
| `0086BDB0` / `0086BDF0` / `0086BE30` / `0086BE70` / `0086FC20` | `effect_*_category_*` | flare 5; waterdrops, light, splash and thunderstorm 0. Each equals the type +18h that the kind's instance constructor writes (section 7.7). |
| `0086BF10` | `water_tracer_destruct_0086bf10` | Releases each non-null reference and nulls it, in this order: +98h, +9Ch, +A4h, +C0h, +8Ch, +70h, +6Ch, +68h, +64h (InterlockedDecrement, slot 0 at zero). Then the strings +50h and +48h, then the base 0086B7E0. |
| `0086F3B0` | `thunderstorm_read_lua_0086f3b0` | Every key is read from **row.ParamsTable**: GlowTexture -> +40h, Has3Dlightning [true] -> +54h, MinLifeTime / MaxLifeTime [0.5 / 4] -> +24h / +28h, MinScale / MaxScale [0.5 / 4] -> +2Ch / +30h, MaxSpawnCount [15] -> +38h, SpawnDelay [1] -> +3Ch (CVTSI2SS). +34h = 2.5f is a constant, not a key. The Textures array +48h is resized to 0 and refilled with {Origo[1], Origo[2] (each [0.5]), Name} records. |
| `0086FB40` / `0086FBC0` | `thunderstorm_destroy_members_0086fb40`, `thunderstorm_destruct_0086fbc0` | 0086FB40 runs with ECX = component + 24h: the texture array (resize 0, then free), then GlowTexture. The destructor sets vtable 00D0DA38, runs 0086FB40 and then the base 0086B7E0. |

Findings:

1. The water tracer reader zeroes +98h, +9Ch and +C0h without a release (7.9.2). The destructor
   does release them. The reader also overwrites +A4h (0086DF83) without a release. So a
   component that is read twice leaks the first textures, the first tracer resource and the first
   spray resource. It does not leak its curves, which the reader releases before replacing.
2. SpawnDelay is read as an integer and stored as a float, so a fractional delay in Lua is
   truncated.
3. In the Textures loop, the Name copy writes its terminator at 0086F71D before the pool
   allocation's null test, so a failed allocation faults.

Adoption: this is presentation only, with no RNG. Nothing in the host is replaced: the effect
component dispatcher in src/gameplay_effect_scalar_components.cpp:273 has no 00D0DA38 or 00D0D5D4
row. With the switch OFF nothing changes. With it ON, the thunderstorm and water tracer components
get their fields, and the strike timing is still in 00BC0510 on stream 1 (section 7.2).

Records: 11 `add-function` records. The section 6 open table now lists **303** addresses. Build:
`scripts/build.ps1` passes. `verify_report_calls` (local/g53f_calls.json) checked 20 rows with 0
failures. No tests were added and there were no game runs.

### 7.10.1 Handoff (cc13-gpr53f)

Still open: the trace-line leftovers 008580C0, 008580E0 and 008583D0 (unreferenced); 0085C0F0,
0085C220 and 0085C5D0; the gunnery lane (cc9's); and the remaining table rows (mostly STL, CG and
the trivial leaves).

## 8. cc13-ver2: verification of the torpedo, bomb and depth-charge host paths

This pass is a read-only check of the code cc9's host runs against the disk bytes, using
`disasm-raw`, `sc2_x87.py` and a byte check of every register-register x87 form quoted below. No
host file was edited, nothing was wired, there were no game runs, and the gunnery bots were
skipped. Flags were read at origin/main ea64971c5. The one new model is
`bomb_release_velocity_006e0a70_006e1f00` (src/gameplay_seg53_cc13.cpp), which is build-tested.

### 8.1 What the host runs (default flags)

| path | host code | image | verdict |
|---|---|---|---|
| Torpedo swim (kTorpedoSwimThrustBound ON, kTorpedoSwimDepthHomingBound OFF) | game_hosts_gunnery.cpp:9172-9202 | 00857480 + 00856BB0 heading turn | matches in the horizontal plane; the vertical is a labelled substitution (8.2 item 4) |
| Torpedo water entry | :9681-9776 | 008568E0, 006E6450 | first limit matches; second limit OFF (item 3) |
| Air ballistics of bombs and torpedoes | `projectile_flight_step` (src/projectile_impact.cpp:88) and the drift add at :9266 | 006E7670 position, 006E65C0 / 006E1510 velocity | formulas match; the gravity constant is wrong (item 2) |
| Bomb release | `release_bomb_drop` :13688 | 006E0A70 (vtable+190h), 006E1F00, 006E4D50 cone | the velocity choice matches; the turn basis diverges (item 1) |
| Torpedo release | `release_ordnance_drop` (:13533) | the same 006E0A70 / 006E1F00 (torpedo vtable 00D0C3E8 +190h = 006E0A70) | diverges (item 1b) |
| Depth charge in water | :8936 `depth_charge_advance_006fcd20` | 006FCD20 | matches apart from the constant and one data source (item 5) |

Checked and matching:
- **Swim drag.** In 00857480 the split is `along = (vx fx + vy fy) + vz fz`, the axial part is
  scaled by `f(1 - 0.5999994 dt)` and the lateral part by `f(1 - 3.0000007 dt)`, and the thrust
  is `f(f(f(fwd * 470h) * mod) * dt)` with mod = 1 (0085776A). These are the host's
  kTorpedoAxialDrag474 / kTorpedoLateralDrag478 (00D0C5EC / 00D0C5E8).
- **Heading turn.** 00857061..0085714E: `e = -00438B10(atan2(fx, fz), cmd)`, clamped to +/-rate,
  then times dt, and the rotation is about the up row (0085E880). In a horizontal frame this is
  the host's `nose += step`.
- **Water entry.** 008568E0 reads shot = entity + 310h, so shot+8 is velocity +318h and shot+0Ch
  is vy. The first limit is `|v| > MaxWaterHitVel` (FCOMI / JA), as in the host. 006E6450 only
  sets the water flag and the trail effects; it sets no speed.
- **Depth charge.** In 006FCD20 the velocity is `v += f(f(-k v) dt)` with `f(-k vy - g)` on y.
  The scan runs below -5, with a radius of 625. The box test is `|local| - box * 0.5 < 10`,
  using the inverse world matrix through 004142E0. The axis pairing is x with +A4h Width, y with
  +A8h Height and z with +A0h Length, and the 006FCEFA `DC C9` FMUL ST(1),ST(0) was checked from
  the bytes.
- **Bomb fall.** 006E1510 computes `vy = f(f(a.y dt) + f(vy - g dt))` and then `v += a dt` with
  a = +348h. a is the drift, and 006E1F00 zeroes its y. This is the host's flight step plus the
  :9272 drift add.

### 8.2 Divergences

1. **Bomb release basis (006E1F00 vs game_hosts_gunnery.cpp:13766-13773).** 006E1F00 passes a
   copy of the bomb's world matrix to vtable+190h, which is 006E0A70. For a plane faster than
   1.3888890f, 006E0A70 sets velocity = w = the owner's world velocity with 3.0 taken off y
   ([00E08E54], which only has reads). It also writes w into **matrix row 2** and runs 0085DC80.
   The turn is then
   `v_i = f(f(f(f(Dx r0_i) L) + f(f(Dy r1_i) L)) + f(f(r2_i Dz) L))`, with L = |w| and D =
   (t cos a, -t sin a, 1) (006E4F91 / 006E4FB1, stream 1). Its rows are that velocity-aligned
   frame, so with t = 0 the bomb keeps w exactly.
   - **Image:** `v = |w| (Dx r0' + Dy r1' + w/|w|)`.
   - **Host:** `v = |w| (Dx right + Dy up + forward_plane)`, using the plane's pose.

   So every host bomb leaves along the **nose** with speed |w|, even with no cone. The 3 m/s
   that 006E0A70 subtracts survives only in the magnitude, and any angle of attack or sideslip
   is folded in. cc13-gpr33 labelled the substitution; the effect was never stated.
   **Prediction:** with a plane flying along its nose (forward = v/|v|), a host bomb starts
   about 3 m/s slower downward than the image's. Falls of 1000 to 3000 m (fall time t of
   14-25 s) take about 0.3 s longer (dt is about 3/g). At 100 m/s the bomb lands about **25-30 m
   long**, and that is also against 007BCC80's fall time, which assumes the -3. A steep dive
   (vy about -100) moves by only a few metres. Level bombers (kind 10h) on ship targets about
   20-30 m wide are where this shows. The image model is
   `bomb_release_velocity_006e0a70_006e1f00`.
   1b. **Torpedo release.** The host's torpedo drop sets velocity = forward x
   unit_forward_speed_0092d730, with no 006E0A70 step: no world velocity and no -3 m/s. The
   image torpedo enters the water about 3 m/s faster downward, which matters for item 3 and for
   MaxWaterHitVel. The time to the water shortens by about 0.1-0.3 s, so a torpedo launched at
   80 m/s lands about 10-25 m shorter.
2. **Gravity constant.** `kProjectileGravity = 9.8103800773621` (include/bsp/projectile_impact.hpp:27)
   is also used as `kGravity` (src/bomb_torpedo_tick.cpp:30). 00CF9058 is the double 9.8100004196
   (= (double)9.81f), so the image computes with 9.81f; docs/GUN_GRAVITY_ARC.md already noted
   this, and it is still unfixed. It affects 006E65C0, 006E7670, 006E8430's 0.05 bias, 006E1510
   and 006FCD20. g/g differs by 3.9e-5, which over a 3000 m fall is about 0.12 m and 1 ms:
   negligible, but every same-binary pair carries it. 006E7670 also associates differently:
   the image computes `y = f(f(f(vy s) - (s (s g)) 0.5) + snap.y)`, the host
   `(snap + vy s) - g term`. That is a last-bit effect.
3. **Torpedo MaxFall break-up (kTorpedoMaxFallBreakupBound OFF).** 00856923..00856932 breaks the
   round up when `vy < -sqrt(2 MaxFall 9.81)`. With the realistic table's MaxFall 50 (lines 115,
   823, 1608) the limit is 31.3 m/s; with 60 (line 886) it is 34.3 m/s. A free drop crosses
   31.3 m/s from **about 50 m** (about 49.5 m with the image's -3 m/s). The host has this
   limit OFF. **Prediction:** torpedo drops from about 50-60 m and up break up in the image but
   swim in the host. Torpedo-bomber missions whose AI releases above that height score hits the
   image would not; the counter `torpedo_fall_breakups` measures it with the switch ON. The
   host's `max_fall > 0` / `hit_limit > 0` gates do not matter, because every torpedo row in
   this installation's realistic bulletclasses.lua sets both keys.
4. **Torpedo vertical (labelled).** The image keeps the entry vy (006E6450). In 00857480 the
   nose levels by `f(fy + (0 - fy)(3 dt))`, the vertical velocity is part of the lateral
   component and decays by `1 - 3 dt`, and 00856BB0's depth controller drives it to swimdepth.
   The host sets vy = 0 and holds y on the surface (:9751, :9276). The faithful 3D path exists
   behind kTorpedoSwimDepthHomingBound. **Prediction:** a host torpedo runs at the surface, not
   at -2.5 m. The hit tests use the hull box, so a hit/miss change needs a target whose box
   bottom lies between 0 and -2.5 m. That was not checked. JM06's submarine is
   the case to check with the switch ON.
5. **Depth charge (JM06).** The formula matches. Two inputs differ:
   - The host's box comes from `flat_scaled(type_id, "length"/"width"/"height", kMilliScale)`
     (:4340-4342), not from the class Lua row's +A0h..+A8h. Whether the two agree for the
     submarines was not checked; if they differ, the 10 m contact margin moves by half the
     difference.
   - The `[game+1FE4h] == 2` skip (006FCDC2) is not modelled. In single-player there is no
     effect.

   **Prediction for JM06:** contact detonations agree when the flat and class dimensions agree;
   otherwise they change only at the box edge.
6. **Heading-turn rate rounding.** The image computes `f(f(turn * (double)pi_f) / 180)` (00CE3D28 =
   3.1415927410125732, both at 24-bit precision). The host computes
   `(float)(turn * 3.14159265358979 / 180.0)`. For 127 of 360 half-degree values from 0.5 to 180
   the host rate is one ULP lower, which is negligible.

### 8.3 Records

Seven records were re-added with `--replace`, keeping their names and sources, with coverage
`partial` and the finding appended to the evidence: 006E1F00, 006E0A70, 006E65C0, 006E7670,
006E8430, 006E1510 and 006FCD20. The previous records stay in git history. The 008568E0 record
still points at `torpedo_enter_water_008568e0` (complete), and the host's OFF limit is item 3.
00857480's record (bomb_torpedo_tick.cpp) was not re-checked as a whole, because the host runs
it only with kTorpedoSwimDepthHomingBound ON.

### 8.4 Open

- Check the flat dimensions against class +A0h..+A8h for the JM06 submarines (item 5).
- Check the torpedo release heights in the torpedo-bomber missions' logs (item 3), using
  existing logs only.
- 006E4D50's remaining rack body.
- 007BCC80's fall-time prediction, against item 1.

### 8.5 cc13-ver2b: the three follow-ups

Doc and records only. No runs; I read only existing logs.

**(1) The flat dimensions equal the class row's for every submarine.** The host's `flat_scaled(...,
"length"/"width"/"height", 1000)` (game_hosts_gunnery.cpp:4340) reads `VehicleClass[id].BSPGun`,
which the host's own flatten Lua writes as `math.floor(row.Length * 1000 + 0.5)` from the same
top-level row keys that class +A0h / +A4h / +A8h hold. I compared all 9 `Type = "Submarine"`
rows of this installation's vehicleclasses.lua (2026-05-09). Kaiten 12 x 1.4 x 1.4, I400
100 x 12 x 7, Gato / Cachalot / Type B (three rows) / Type VII 100 x 8 x 4, and the midget
20 x 1.7 x 4 all give the same float both ways (`local/v2b_subdims.py`). So section 8.2 item 5's
data-source difference has **no effect** on JM06. That leaves the depth-charge path with the
gravity constant (item 2) and the single-player-irrelevant world-mode-2 skip.

**(2) MaxFall would break up none of the logged torpedo drops.** `local/v2b_drops.py` read the
`gunnery: torpedo drop N by U at H m, speed S m/s, bullet B` lines from 396 logs modified in
the last 10 days, across all checkouts' `local\`. The host prints the first four drops of each
run. That gives 254 distinct (mission, unit, height, speed, bullet) rows:

| mission | drops | release height min / median / max | over the MaxFall 50 limit | over 60 |
|---|---|---|---|---|
| JM02 | 100 | 5 / 11 / 39 m | 0 | 0 |
| JM08 | 112 | 12 / 14 / 36 m | 0 | 0 |
| USN01 | 34 | 13 / 16 / 31 m | 0 | 0 |
| USN21 | 8 | 11 / 19 / 23 m | 0 | 0 |

The test used the image's entry speed `sqrt(9 + 2 g h)`, which includes 006E0A70's -3 m/s, from
a level release. The highest release, 39 m, enters at 27.8 m/s, against a limit of 31.3 m/s.
Only a plane already sinking at about 12 m/s or more at 39 m would cross the limit, and the
logs do not carry the release vy. No log in that window shows a water-entry break-up of either
kind. **Item 3 is therefore not observable in the current AI torpedo runs.** It would matter
only for a player or script drop from above about 50 m.

**(3) 007BCC80 matches the image release and makes item 1 an aiming bias.** 007BCC80,
`__thiscall(plane, float h)`, RET 4, returns 0 when h <= 0, and otherwise:
`vy = f(V.y - 3.0)` with V = the plane's vtable+34h world velocity, and
`t = f(f(f(sqrt(f(vy^2 + 2h g))) + vy) / g)` with g = 9.81f.

That is exactly the fall from the image's release velocity (006E0A70: V with 3 taken off y;
no cone with t = 0). In the image, the predictor and the release agree. The host's
`weapon_fall_time_007bcc80` (src/dive_bomb_task.cpp:251) has the right g and the -3. It differs
in only two ways: it rounds once in double where the image rounds each x87 step at 24 bits, and
it returns 0 for a NaN height, where the image computes. Neither matters. Its callers pass the
plane's linear velocity y, as the image does.

The host's **release** does not match its own predictor. It releases at `|w| * forward_plane`
(item 1), so its bombs fall longer than predicted:
- **Level release** (V.y = 0, nose level): `dt = (sqrt(2gh) - sqrt(9 + 2gh) + 3) / g`, which is
  **0.302 s at 1000 m and 0.304 s at 3000 m**. At 100 m/s every level-bombed impact lands about
  **30 m long** of the point the AI aimed at.
- **Dive release:** with V.y / |V| = -0.87, the host's vy is about V.y - 2.3 m/s instead of
  V.y - 3. The horizontal speed is about 1.3 m/s high, so a 5 s fall lands about 5-7 m long.

Record: `weapon_fall_time_007bcc80` (007BCC80) was added as complete, checked against the bytes.

Still open: 006E4D50's remaining rack body.

### 8.6 Adoption notes for the release-velocity fixes (host file leased; not edited)

Under the HOST FIX TERMS, `src/game_hosts_gunnery.cpp` is leased by cc9 (agent/cc9-gunnery36:
cc9_g40_turning_fire and cc9_g40_torpedo_fall, checked 2026-10-13). So these notes are for cc9 to
apply, and this pass made no edit to that file. Gravity (item 2) is left to cc9's gunnery40, and
MaxFall (item 3) already has its own switch.

| fix | replaces (host file:line) | inputs | RNG | OFF / ON prediction |
|---|---|---|---|---|
| `kBombReleaseVelocityFrameBound` | release_bomb_drop, game_hosts_gunnery.cpp:13772 (`velocity[i] = vmag * (dx*right[i] + dy*up[i] + forward[i])`) | call `bomb_release_velocity_006e0a70_006e1f00(rack, owner, D)` with rack = the plane pose as a 4x4 (rows right / up / forward / origin; the mount node is unread, as today), owner = {is_plane = true, speed = 0092D730, world velocity = unit_linear_velocity}, and D = (dx, dy, 1) from the existing draws. Use `result.velocity`. | unchanged: the same draws in the same order, because only the basis changes | OFF: as today. ON: the bomb leaves along the velocity frame (velocity minus 3 m/s), so level-bomb impacts move about **30 m short**, onto the AI's 007BCC80 aim point (8.5 (3)), and dive bombs about 5-7 m. Expect hit counts to change for level bombers (kind 10h) in the missions that have them (not enumerated here); nothing else moves. |
| `kTorpedoReleaseVelocityBound` | release_ordnance_drop, game_hosts_gunnery.cpp:13557 (`velocity = forward * speed`) | the same model with D = (0, 0, 1), which leaves velocity = the plane's world velocity minus 3 m/s on y: the torpedo's vtable+190h is also 006E0A70, and its cone is t = 0 when Throw is 0 (check the bullet row's Throw; if Throw is non-zero, use the drawn D as bombs do) | none, unless Throw is non-zero | OFF: as today. ON: torpedoes reach the water about 0.1-0.3 s sooner and about 10-25 m shorter, entering about 3 m/s faster downward. The MaxFall margin at the logged 5-39 m release heights stays positive (8.5 (2)), so no new break-ups. Run lengths and hit timing shift slightly in JM02, JM08, USN01 and USN21. |

Both models are in `src/gameplay_seg53_cc13.cpp` / `include/bsp/gameplay_seg53_cc13.hpp`
(build-tested, not wired).
