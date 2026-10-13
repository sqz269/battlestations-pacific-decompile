# CC13 GPR 46: segment 46 reference models (paratroopers, path join)

Packet `cc13-gpr46` (cc13 lineage, Claude). Segment 46 is `007A49A0`-`007B2D80`. cc9 approved this
reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg46_cc13.cpp` and `include/bsp/gameplay_seg46_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

The behaviour of each function was established earlier in `docs/CC13_SOLDIER_TRAFFIC.md` section 10 and
`docs/CC13_GP_007A_007F.md`. This pass turns it into C++ models. Each comparison and each float rounding
was re-read from the disk bytes: Capstone `disasm-raw`, and the x87 traced with `sc2_x87.py`. Everything here
is **reconstructed and build-tested**. None of it is ABI-compatible, wired or game-validated.
`tools/x87_dc_form_scan.py` results are in the verification section.

## 1. Models

### 1.1 The MParatrooper entity (class id 31h)

| address | model | what it does |
| --- | --- | --- |
| `007AB8C0` | `construct_007ab8c0` | Draws `+468h = U(ParachuteOpenAltitudeMin, Max)` on **stream 1**. The call at `007AB94A` has ECX = EBX+1 with EBX = 0; lo is at `[esp]` (= globals+14h) and hi at `[esp+4]` (= +18h). Clears the state bytes, sets class id 31h at `+C4h` and `+4B8h = 8`. |
| `007ABA50` | `is_kind_of_007aba50` | Answers true for 31h, 2Ah, 2, 1, 0, or the object's own `+C4h`. The chain is 31h -> 2Ah (MBomb) -> 2 -> 1 -> 0. |
| `007AC530` | `load_animations_007ac530` | The animation part only: the fall/open/close handles and their lengths. Sets current = fall and zeroes `+4B4h` and `+494h`, then starts fall at time 0. |
| `007AB230` | `set_landing_pos_007ab230` | Sets the target `+47Ch`, with y taken from `00903860` ground height. In MP mode 1 with `[game+218Ch] == 0` it sends message 71h, and copies `[+3BCh]+180h` to `+4B8h` when `+3BCh` is set. |
| `007ABB30` | `advance_in_air_007abb30` | See 1.2. |
| `007AB360` | `update_chute_and_target_007ab360` | Sub-object slot +8h (`ECX = this+3E4h`). See 1.3. |
| `007AB0B0` | `anim_clock_007ab0b0` | Has no Ghidra function; body `007AB0B0`-`007AB1D3`. See 1.4. |
| `007AC000` | `step_landing_007ac000` | See 1.5. |
| `007AB590` | `land_and_spawn_soldier_007ab590` | Runs once as the fixed-step callback. It adds the round to the building's capturer list (`006F39B0`) and returns a soldier plan: the class, SoldierAnim, its stream-0 draw, and the facing row `building - self`. |
| `007AAE10` | `on_killed_007aae10` | Calls `0077D270`, then kills the soldier `+478h` (cause 2) and clears it, then calls `006F34B0` to remove the round from the building's capturers. |

#### 1.2 `007ABB30` AdvanceInAir (`+19Ch`)

1. **Drag.** `k = c + ((chute - 0) * (o - c)) / (1 - 0)` is stored to a float, where c = SlowFactorClosed and
   o = SlowFactorOpened. Then each axis does `v += f(f(-v * dt) * k)`.
2. **Base step.** `006E1510` (the MBomb step) runs. Nothing more happens unless `+458h` (`flying`) is set.
3. **Avoid-zone fix, once (`+488h`).** The radius is `f((word +174h % 23) * 3.0 + 80.0)`. `0041B840` returns
   the nearest avoid-zone boundary point to the target's (x, z), searched within 800. The target's y is then
   re-read from the ground.
4. **Steer.**
   - The acceleration is `a = f(f(target - pos) / 3.0)`, on x and z only.
   - The authority is `ramp = clamp(f(+494h - 2.0), 0, 1)`; time zero is `007AC530`.
   - The limit is `hi = f(interp(0.2:5, 1:2.5)(chute) * ramp)` and `lo = -0.0 - hi` (SUBSS).
   - Each axis is clamped low-test-first, so an unordered value passes through.
   - The new value is `n = f(v + a * dt)`.
5. **Lateral cap.** `M = f(interp(0.5:0.3, 1:1.8)(chute) * |vy|)`, using vy after the base step. The bound
   is `|v|` only when it is strictly larger than M, otherwise M. `n` is clamped to that bound, and vy is stored
   back unchanged.

#### 1.3 `007AB360` UpdateChuteAndTarget

- **Not flying:** chute 0, target building 0, and the fall animation is held at time 0 when the animation
  object exists.
- **Flying:**
  - `+494h += dt`.
  - In "fall", once `open altitude >= pos.y - max(ground, 0)` (FCOMI with JB, so unordered skips), it switches
    to "open" and sets chute = 1.
  - In "open", once landed or in water, it switches to "close".
  - Then, when the MP mode is not 2 and a target building exists:
    - the building's side equals the round's side: `Kill(2)`;
    - `00803510(my side, building side) == 1`: `vslot +1ACh(f(f(Damage - building+368h) * dt))`, applied only
      while the difference is positive.
  - Last, it calls `+19Ch(f(class+5Ch * dt))`.

#### 1.4 `007AB0B0` AnimClock (`+DCh`)

`t = f(dt + +4B4h)` is stored first. Then, by the current animation:
- **fall:** `fmod(t, fall length)`, through `00BF857A`.
- **open:** held at `f(open length - 0.001)`. The constant is the double `0.0010000000474974513`.
- **close:** compared unrounded with `close length - 0.001`. Past it, the clock is set to that value, chute
  becomes 0, `0048A1E0` fires on the soldier, and **the capture timer `+474h += dt`**.

Then the animation is set, with no null test (unlike `007AB3A4`), and the soldier runs its clip test and
`vslot +D8h`.

#### 1.5 `007AC000` StepLanding (`+194h`)

**Landed:** nothing happens until chute <= 0 (COMISS with JB, so unordered returns). Then:
- a bad landing gives `Kill(2)` and returns;
- with no soldier yet, it asks `006F2C30(&pos, side)` for the nearest enemy building:
  - found: `+490h = 1` and `00875A80(+3E4h, 0)`;
  - not found: `Kill(2)`, and the code falls through;
- `timer >= CaptureDuration` gives `Kill(2)`.

**Not landed:**
1. `006E2220` extrapolates the position.
2. In water: `Kill(2)` once chute <= 0.
3. When flying, with ground height g:
   - g < 0 (sea) and pos.y < 0: `+472h` in water;
   - g >= pos.y: touchdown. It sets `+470h` and the pose `+A4h = (x, g, z)`, then reads the normal
     (`009038F0`). With `dot = f((nx*0 + ny) + 0*nz)` inside [-1, 1] and `acos(dot) > MaxParatrooperGradient`,
     it sets the bad landing `+471h`.

Every kill uses cause 2 and is skipped in MP mode 2.

### 1.6 The level-bomb rack's paratrooper branch

| address | model | what it does |
| --- | --- | --- |
| `006E3F90` | `set_target_point_006e3f90` | `rack+4F8h = 1`, the point goes to `+4FCh` and the velocity to `+508h`. RET 8. Its one caller is `007C0EC9`, in `007C0D90`, on a level bomber (IsKindOf 10h) whose rack descriptor answers 31h. The point is `007BCCF0(unit)` (RET 4) and the velocity is the unit's `vslot +34h`. |
| `006E541F`-`006E5515` | `paratrooper_drop_target_006e541f` | With `rack+4F8h` set and the round IsKindOf(31h), the landing point is `(f(U(-50,50) + px), py, f(U(-50,50) + pz))`. Both draws are on **stream 0**, x first (`006E5479`) and then z (`006E549E`). `007AB230` then replaces y. After that the rack point moves by `f(v * rack+494h)` per axis. |
| `006E55E7`-`006E55FE` | `paratrooper_drop_clear_006e55e7` | When `rack+484h <= 0` (signed JLE) the flag `+4F8h` is cleared; otherwise `vslot +224h` runs. |

So a stick of paratroopers lands along the bomber's velocity, spaced by toRepeatTime, with ±50 jitter on each
of x and z.

### 1.7 Path join and path geometry

`include/bsp/ship_ai_path_cursor.hpp` labels two things as unread:
- its `ship_ai_path_nearest_index_007b1100`, which is a planar hypothesis;
- the tail of `007B11F0` (head only).

This pass reads both.

| address | model | what it does |
| --- | --- | --- |
| `007B1100` -> `007B0900` -> `007AED10` | `path_local_from_homogeneous_007b0900`, `nearest_point_index_007aed10` | `007B1100` is `007B0900(pos, 0)` on the source's path. `007B0900` transforms (pos, 1) by the path entity's inverse world matrix (`00B62D10`), then divides: `inv = f(1/w)`, `f(x*inv)`, `f(y*inv)`, `f(inv*z)`. `007AED10` is a **3-D** search. It stores the per-axis differences, stores the squared sum to float, counts a sum <= 1e-10 as 0 and otherwise takes `f(sqrt)`. The strictly smaller distance wins, so the first of equal points is kept. |
| `007B11F0` | `start_index_for_position_007b11f0` | Read whole. RET 0Ch, and the float argument is never read. With fewer than two points, or the nearest already last (forward) or first (backward), it answers the nearest. Otherwise it takes `a` = the nearest point, `b` = its neighbour in the travel direction, and on (x, z) only: `d = f(p - a)`, `u = f(f(b - a) * 00419260(b - a))`. It answers the neighbour when `f(u.z*d.z + u.x*d.x) > 0`, that is when the position is already past `a` along the leg; otherwise the nearest. |
| `007AE6D0` | `centroid_007ae6d0` | Float running sums, then `f(inv * sum)` with `inv = f(1/(double)count)`. |
| `007AE7B0` | `distance_to_polyline_007ae7b0` | One point: `0042B2F0(pos - p0)`. Otherwise the minimum over the legs of `0046FF70(pos, p[i], p[i+1])` (reusing `cc13_sys::point_segment_distance_0046ff70`), starting from FLT_MAX. |
| `007B1EC0` | `erase_points_outside_world_007b1ec0` | Read whole. The ledger calls it a clip, but it **erases**. Each point with `x < [game+711Ch]`, `x > [+7128h]`, `z > [+7124h]` or `z < [+7130h]` is removed through `0071F3E0`, and the walk then restarts at index 0. The compares are strict, so a NaN coordinate keeps its point. The caller is `0071FDE0` `BSP_DirectorPathObject_AddPoint`. |

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `paratrooper_drop_target_006e541f`, `set_target_point_006e3f90`, `paratrooper_drop_clear_006e55e7` | `src/game_hosts_units.cpp:23130` ("paratrooper target point 006E3F90 (007C0EC9) is not modelled"), `:23231` (`lb_rack_drop_006e4d50`, "rack+4F8h ... only the paratrooper branch sets"), `:27107`; the 31h input `:22656` (`in.has_paratroopers = false`) | release point `007BCCF0` (the host's level-bomb release point) and unit velocity; rack toRepeatTime `+494h`; rounds left `+484h`; whether the rack's round is kind 31h (`bsp::ordnance_has_paratrooper_31h`) | two stream-0 draws per paratrooper round, x then z, at the drop, after that drop's own dispersion draws (`006E513C`, `006E51AC`) | OFF: none. ON: JM05-style level bombers with Paratrooper racks get a target point. The two extra stream-0 draws per round shift every later stream-0 consumer from the first drop on. |
| `construct_007ab8c0` | none (the host creates no MParatrooper round; `game_hosts_units.cpp:22656`) | SoldierGlobals (`soldier_traffic_cc13::load_soldier_globals_0084cb70`) | one **stream-1** draw per round created | ON: stream-1 consumers (`0074A9C8` landing ship, plane tasks) shift by one draw per paratrooper. |
| `advance_in_air_007abb30`, `update_chute_and_target_007ab360`, `anim_clock_007ab0b0`, `step_landing_007ac000` | none; the round would be a new entity kind in the units host. The bomb step `006E1510` / `006E2220` would be the host's existing bomb flight. | class row `bulletclasses.lua` "Paratrooper" (SlowFactor*, CaptureDuration, Damage) and BombClass `+5Ch`; ground height and normal; avoid zones (`include/bsp/avoid_zone_boundary.hpp`); command-building side and `+368h`; MP mode | none | ON: rounds descend and steer to the jittered point. They land, or die on water or on a slope above `MaxParatrooperGradient`, then find the nearest enemy building or die. They damage the building at `(Damage - +368h)/s` and die after CaptureDuration. |
| `land_and_spawn_soldier_007ab590`, `on_killed_007aae10` | `src/game_hosts_ship_ai.cpp:572` and `:12443` (arm 3, the paratrooper list `+7DCh`, "always empty in this process") | building handle; the class SoldierClass and SoldierAnim | `004B0F80` draws U(0, duration) on **stream 0** in the image (`004B102F`). This is presentation: route it to a keyed presentation stream or keep it OFF (RNG rule). | ON: capture arm 3 of the command-building step (`006F6760`) sees capturers, so capture progress and capture reports move. |

| `nearest_point_index_007aed10`, `path_local_from_homogeneous_007b0900`, `start_index_for_position_007b11f0` | `src/game_hosts_commands.cpp:3142` (`ship_ai_path_nearest_index_007b1100`, the planar hypothesis, fed only x and z) and the `007B11F0` head at `:3132` | the path's points in the path entity's local frame and that entity's inverse world matrix (the authored path is a scene entity, `pathID`); the cursor's direction byte; `00419260` (`native_vector2_reciprocal_length_00419260`) | none | OFF: none. ON, two effects. First, the nearest index becomes 3-D in the path's local frame, so it moves only where the path points or the ship sit off y = 0 or the path entity is transformed. Second, the start index takes the next point in the travel direction whenever the unit is already past the nearest point along that leg, so a ship joining mid-leg stops turning back to the point behind it. Ships on move-on-path orders can therefore change course at the join; which missions move was not measured. |
| `centroid_007ae6d0`, `distance_to_polyline_007ae7b0` | none found (their callers are the world wrappers `007B0650`/`007B0710`, reached through the path interface's vtable; no host caller was traced) | path points, query position, `0042B2F0` | none | none known until a caller is bound |
| `erase_points_outside_world_007b1ec0` | none (the host has no `0071FDE0` DirectorPathObject AddPoint) | the point vector and the map box `[game+711Ch..+7130h]` (as in `band_007a_cc13`'s `007EF8F0`) | none | ON: script-added path points outside the map box are dropped rather than kept |

The capture itself (`006F6760` adds `CapturePower` per capturer) is the ship-AI host's arm 3 and is not
modelled here.

## 3. Open questions

- **`flying` (`+458h`): SETTLED (cc13-gpr46b).** The drop itself sets it. The setter is
  `006E1FA3 MOV byte ptr [EBP+148h], 1` in `006E1F00` (`BSP_BombRack_DropCone`), where EBP = round+310h
  (`006E1F05`/`006E1F0E`). The path to it, all read from bytes:
  1. `006E4D50` walks the rack's children to the first that answers IsKindOf(2Ah). The paratrooper (31h ->
     2Ah) qualifies (`006E4EC0`-`006E4EE3`).
  2. `006E520B` calls the round's `vslot +108h`. In the paratrooper vtable `00D05060` that slot is `006E27C0`
     `BSP_BombProjectile_BulletBlock`, which answers round+310h.
  3. `006E529C` calls that sub-object's `vslot +34h`. In the paratrooper's +310h vtable `00D04FF0`, slot +34h
     (`00D05024`) is `006E1F00`.

  So every rack drop, the paratrooper drop included, detaches the round, sets its release velocity and sets
  `flying = 1`. `006E1F00` also stores the water byte `+354h`: 1 when the round is above the water height,
  else 0.

  The earlier search missed this because it scanned disp32 `458h`. The store goes through the +310h
  sub-object as disp `148h`; this is the vacuous-scan trap.

  The other two writers stay as recorded:
  - `006E1A35`, the network create, copies message `+11Ch`;
  - `006E2E10` restores `flying` from a saved game. `006E3047`-`006E30F7` run only when `[+C0h]+4 == 3`, and
    read `SEntity_GetSavedEntityLuaData` -> `"_bomb"` -> `"_bulletBase"` -> `"flying"`.

  No mission Lua in this installation names `flying` or `_bulletBase` (a grep of `scripts\`, which also
  found nothing in `jm05`). A JM05 bind can therefore set `flying_458 = true` at the host's rack drop.
- `class+5Ch` is the BombClass step scale (CC13_FLAK_FLYTIME_BURST.md); its Lua key was not read here.
- `007AB590`'s soldier attach (`vslot +98h` with `+3Ch`/`+30h` and an identity matrix) and the chute hide are
  scene work, left to the host.
- `00BF9940` is read as `acos` from its use only (as in CC13_SOLDIER_TRAFFIC.md).
- The x87 keeps 80-bit intermediates. The models carry them in double, so a last-bit difference against the
  image is possible.

### 3.1 JM05's paratroopers (this installation's `scripts\missions\ijn\JM\jm05.lua`)

`luaJM5SpawnParatroopers` (`jm05.lua:3288`) spawns an allied group at `USNSpawn 01`, y = 600:
- type 136 "C47 Dakota", WingCount 3, `Equipment` 1;
- type 135 "Warhawk", WingCount 3.

`luaJM5ParatroopersSpawned` (`:3338`) gives the `LevelBomber` member `PilotMoveTo(v, Mission.Airfields[1])`,
not an attack order, and sends the fighter after it.

How the drop gets there was not traced in this pass. It is the bot's decision, through the level-bomb
command and the paratrooper option in the AI target weights (`ai_target_weights.cpp` `00A08A23`), and then
the release `007C0D90` -> `006E3F90`. `jm05.lua:2733` counts the Dakota squads, not the rounds.

## 4. Ledger

Function records (`config/reconstruction`): `007AB8C0`, `007ABA50`, `007AC530`, `007AB230`, `007ABB30`,
`007AB360`, `007AB0B0`, `007AC000`, `007AB590`, `007AAE10`, `006E3F90`, `007AED10`, `007B0900` (partial: the
divide), `007B11F0`, `007AE6D0`, `007AE7B0`, `007B1EC0`. Fragments: `006E541F`, `006E55E7`.
Names (`config/names`): `007AB0B0` `BSP_MParatrooper_AnimClock`, `007AB330` `BSP_MParatrooper_HandleMessage71`
(neither has a Ghidra function).
`006E3F90`/`006E541F`/`006E55E7` are outside segment 46. They are the rack branch cc9 asked for, read-only
there, and no other lease holds them.

## 5. Verification

- `./scripts/build.ps1` passed in this tree: the build plus ctest (2 of 2). No test was added; the models are
  host-callback reference code and cc9 binds them OFF.
- `tools/x87_dc_form_scan.py --range 007A49A0-007B2D80` finds three DC-form sites in the modelled functions:
  - `007ABB5D` `fsub st(1), st(0)` is chute - 0;
  - `007ABD90` `fdiv st(1), st(0)` is dx / 3.0;
  - `007AC253` `fmul st(1), st(0)` is nx * 0.

  All three agree with the models. `006E3F90`-`006E5620` has no DC-form site.
- `tools/verify_report_calls.py` passed 20 call-site rows (constructor RNG, drop RNG and `007AB230`,
  `006E3F90`/`007BCCF0` at `007C0EC1`/`007C0EC9`, and the landing, steer and kill calls), with 0 failures; six more rows cover the path pieces (`0046FF70`, `0042B2F0`, `00419260`, `007AED10`, `007B0900`, `00BF7030`), also with 0 failures. The
  row files are `local/g46_calls.json` and `local/g46_calls2.json`, which are not committed.

## 6. Handoff

The cc9 priorities are done: the JM05 paratrooper entity and the level-bomb paratrooper branch. The path join
was also done, because it closes an unread hypothesis in the ship-AI path code. Segment 46 still has about 60
functions with names but no record. None of them is gameplay-critical:
- **Movie camera (`007A49A0`-`007AADD0`).** These are presentation. The camera lanes (cc13-cam*) own them;
  `BSP_MovCam_*` such as `007A49A0` draw on stream 0 per docs/CC13_BAND_0079_007F.md 3.2.
- **Path networking and archives.** `007AD5C0`-`007AD7A0` (MT_PATH_SET/ADD/INSERT/DELETE),
  `007B0B00`-`007B0D30`, `007B0F00`, `007B1120`/`007B1170`, `007B2C00`, and the PATH_CREATE message `007B0BF0`/`007B2950`.
- **Path-source plumbing.** `007B19B0`/`007B1BF0`/`007B1D90` (refcounted source handles), the constructors
  `007B28C0`/`007B28F0`/`007B2AB0`, and `007B1B10`.
- **Lua float readers** `007AAE50`-`007AAFD0`, and the paratrooper class reader `007AC780`. Its field table is
  in CC13_SOLDIER_TRAFFIC.md 10.1 and was not re-read here.
- **Small path-cursor accessors.** `007ADB90` PointAtIndex (it answers zero without a path, and a negative index
  means the cursor's own), `007ADD50` (`vtable[8](007ADCC0())`), `007B1320`
  (`vtable[4](out, 007ADCC0())`) and `007AE060`.

The `flying` question is settled in section 3 (cc13-gpr46b): the rack drop sets it.
