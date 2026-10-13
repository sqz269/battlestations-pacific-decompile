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
| `008568E0` | `torpedo_enter_water_008568e0` | Runs SetWeaponScale(1.0). The round **breaks up** when `\|v\| > MaxWaterHitVel` or `vy < -terminal fall speed` (+ECh). On a server the break-up kills it (00926D90(1)); on a client it runs 006E6450 instead. Otherwise 006E6450(arg) runs and a splash appears when `arg && WaterSplashEfx`. A heading still exactly 10000 takes the pose's yaw (00521370). |
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
5. Math helpers 0085C0F0..0085E8C0 (rotate about axis, orthonormalise variants, ellipsoid
   and sphere-triangle tests). Torpedo homing and debris use 0085E880 / 0085C3F0.

Functions in the segment that still have no reconstruction record (named, not reconstructed),
grouped by name prefix:

| prefix | count | addresses |
|---|---|---|
| `STL` | 62 | 00851E10, 00861E20, 00861E50, 00861EA0, 00861ED0, 00861F60, 00861F80, 00861FC0, 00862040, 008620E0, 00862110, 00862190, 00862300, 008628D0, 00862900, 00862940, 008629D0, 00862A10, 00862A70, 00862A90, 00863470, 008634A0, 008634F0, 00863500, 00863540, 00863570, 00863B10, 00863B80, 00863BC0, 00863BF0, 00863EA0, 00864090, 00864160, 00864220, 00864290, 00864300, 00864340, 00864550, 008660D0, 00866160, 008665C0, 00866660, 008668F0, 00866BD0, 00869690, 008696C0, 00869890, 00869930, 00869B70, 0086A3C0, 0086A590, 0086A5A0, 0086A9D0, 0086AB50, 0086AB60, 0086AE60, 0086AF80, 0086B6E0, 0086EF20, 0086FA00, 0086FC50, 0086FF20 |
| `BSP_FixedStringBuilder` | 2 | 00851EE0, 00852140 |
| `CG` | 56 | 00851F20, 00853010, 00853020, 00853030, 00853040, 00853380, 008533A0, 00856230, 00856250, 00856340, 00857D80, 00857D90, 00857DA0, 00857DB0, 00857E00, 00858150, 00858220, 008586E0, 0085EAE0, 0085EB00, 0085EB60, 00861BA0, 00862630, 00862750, 008627F0, 00863100, 00864660, 00864FD0, 00865CB0, 00866090, 00866280, 00866300, 00867060, 00869CF0, 0086A400, 0086B710, 0086B850, 0086B9A0, 0086BED0, 0086C240, 0086CD10, 0086CF10, 0086CF70, 0086D080, 0086E5A0, 0086E870, 0086FD70, 0086FF00, 008716E0, 00872280, 00873060, 008730E0, 00873420, 00873890, 00873A90, 00873E90 |
| `BSP_ShipClass` | 1 | 00852200 |
| `BSP_UnitInstance` | 2 | 00852210, 00857C20 |
| `BSP_SubmarineUnit` | 5 | 00852D30, 00852F10, 008530E0, 00853110, 00853590 |
| `TRIV` | 10 | 00852FB0, 00852FC0, 008561D0, 008561E0, 008561F0, 00856200, 00856210, 00858170, 0085EA30, 0086B980 |
| `BSP_VehicleClassSubmarine` | 1 | 008531A0 |
| `BSP_SubmarinePeriscopeDebris` | 1 | 008533E0 |
| `BSP_TorpedoProjectile` | 8 | 008559D0, 00855B00, 00855BB0, 00855D30, 00855F50, 008562A0, 00856360, 008563A0 |
| `BSP_ObserverLink14` | 1 | 00855FE0 |
| `Unwind@00856220` | 1 | 00856220 |
| `Unwind@00856240` | 1 | 00856240 |
| `BSP_MTorpedo` | 1 | 00856260 |
| `BSP_TorpedoBoat` | 2 | 00857CD0, 00857F00 |
| `BSP_TorpedoBoatClass` | 2 | 00857D60, 00857E20 |
| `BSP_TraceLineEffectInstance` | 4 | 008580A0, 008583A0, 00858530, 008585D0 |
| `BSP_NodeHandle` | 2 | 008580C0, 008658C0 |
| `TMPL_PointerArray` | 3 | 008580E0, 008583D0, 00858660 |
| `BSP_NoOpRet4` | 1 | 00858180 |
| `BSP_NoOpRet8` | 1 | 008581B0 |
| `BSP_ReturnFalseByte` | 1 | 008581E0 |
| `BSP_NoOp` | 1 | 00858200 |
| `BSP_NativeTraceline` | 1 | 00858330 |
| `BSP_EffectTraceLine` | 3 | 00858700, 00859240, 00859370 |
| `BSP_SessionMessage` | 5 | 00859410, 00859C80, 00859CC0, 00859FA0, 00859FE0 |
| `BSP_TurningGun` | 13 | 00859550, 008598B0, 00859920, 00859A20, 00859B10, 0085A130, 0085A3D0, 0085A8B0, 0085AB50, 0085AD00, 0085B980, 0085BA30, 0085BB50 |
| `BSP_Unit` | 1 | 00859830 |
| `FUN` | 1 | 00859950 |
| `BSP_Gun` | 2 | 0085A9A0, 0085B7D0 |
| `BSP_ReconCategory` | 2 | 0085BCE0, 0085BD50 |
| `BSP_Matrix` | 9 | 0085C0F0, 0085C220, 0085D3D0, 0085D770, 0085D920, 0085DAD0, 0085E4D0, 0085E880, 0085E8C0 |
| `BSP_Vector3f` | 1 | 0085C3F0 |
| `BSP_Ellipsoid` | 1 | 0085C5D0 |
| `BSP_Geometry` | 2 | 0085CDB0, 0085DF60 |
| `BSP_WaterMine` | 7 | 0085E9C0, 0085EA50, 0085EAC0, 0085F120, 0085F1E0, 0085F270, 0085F2B0 |
| `BSP_MWaterMine` | 1 | 0085EB20 |
| `BSP_WaterMineShot` | 1 | 0085F1A0 |
| `BSP_WaterMineClass` | 1 | 0085F310 |
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
| `BSP_WorldAtmosphere` | 2 | 008658E0, 00865900 |
| `BSP_RainDescriptor` | 3 | 00865AB0, 00865B80, 00865CF0 |
| `BSP_OwnedObject` | 1 | 00865E30 |
| `BSP_NativePointerVector` | 3 | 00865E90, 008661B0, 00866690 |
| `BSP_PointEffect` | 2 | 00865FE0, 008679D0 |
| `BSP_RefPtr` | 11 | 00866730, 00866970, 00866C30, 0086AC50, 0086AC80, 0086ACB0, 0086ACE0, 0086AFC0, 0086B080, 0086B750, 00873AB0 |
| `BSP_EffectDeletionLock` | 1 | 00866760 |
| `BSP_EffectManager` | 2 | 008669A0, 008674A0 |
| `BSP_EffectHandle` | 1 | 00866B70 |
| `BSP_EffectSound` | 4 | 00868D00, 00869E10, 0086A740, 0086FA80 |
| `BSP_EffectFlare` | 2 | 00869300, 0086BDB0 |
| `BSP_EffectThunderStorm` | 5 | 00869370, 0086F3B0, 0086FB40, 0086FBC0, 0086FC20 |
| `BSP_EffectWaterdrops` | 2 | 008693E0, 0086BDF0 |
| `BSP_EffectLight` | 2 | 00869450, 0086BE30 |
| `BSP_EffectSplash` | 2 | 008695B0, 0086BE70 |
| `BSP_Effects` | 1 | 00869D20 |
| `BSP_EffectParticle` | 2 | 00869D70, 0086BC50 |
| `BSP_NativeDwordVector` | 6 | 0086A280, 0086A910, 0086A950, 0086AA10, 0086B630, 00872E70 |
| `BSP_NativeQwordVector` | 4 | 0086A350, 0086A990, 0086AAE0, 0086AED0 |
| `BSP_ParticleComponentResourcePointers` | 2 | 0086AAA0, 0086AE90 |
| `BSP_NativeString` | 1 | 0086B290 |
| `BSP_EffectComponent` | 1 | 0086B7C0 |
| `BSP_EffectTracer` | 2 | 0086B990, 0086B9B0 |
| `BSP_EffectWaterTracer` | 5 | 0086BD10, 0086BD20, 0086BF10, 0086C120, 0086D180 |
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
| `BSP_EffectFlareInstance` | 4 | 008720A0, 008720D0, 008720F0, 00873010 |
| `BSP_EffectThunderStormInstance` | 5 | 00872100, 00872110, 00873C80, 00873EB0, 00873FF0 |
| `BSP_EffectWaterdropsInstance` | 5 | 00872120, 00872BF0, 00872C10, 00873090, 00873100 |
| `BSP_EffectNode` | 1 | 008721A0 |
| `BSP_EffectSplashInstance` | 5 | 008721C0, 00872D60, 00872DF0, 00872E30, 008739A0 |
| `BSP_NodeRef` | 3 | 00872220, 00872240, 00872260 |
| `BSP_EffectSoundInstance` | 10 | 00872630, 00872650, 00872670, 00872690, 008726C0, 00872720, 00872F40, 00872FB0, 00873AE0, 00873DE0 |
| `BSP_EffectLightInstance` | 3 | 00872CB0, 00872D30, 008738B0 |
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
| `008738B0` / `00873100` / `00873C80` | nothing (the effect components are built by src/gameplay_effect_components.cpp but not ticked) | the instance fields, the definition rows (`WaterdropCount` etc., src/gameplay_effect_scalar_components.cpp), node positions, the weather renderer 00BC1xxx | none in these bodies. The storm's strike timing lives in 00BC0510 (outside this segment, not checked) | presentation only; nothing in a gameplay pair moves |

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
