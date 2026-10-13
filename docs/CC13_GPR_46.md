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

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `paratrooper_drop_target_006e541f`, `set_target_point_006e3f90`, `paratrooper_drop_clear_006e55e7` | `src/game_hosts_units.cpp:23130` ("paratrooper target point 006E3F90 (007C0EC9) is not modelled"), `:23231` (`lb_rack_drop_006e4d50`, "rack+4F8h ... only the paratrooper branch sets"), `:27107`; the 31h input `:22656` (`in.has_paratroopers = false`) | release point `007BCCF0` (the host's level-bomb release point) and unit velocity; rack toRepeatTime `+494h`; rounds left `+484h`; whether the rack's round is kind 31h (`bsp::ordnance_has_paratrooper_31h`) | two stream-0 draws per paratrooper round, x then z, at the drop, after that drop's own dispersion draws (`006E513C`, `006E51AC`) | OFF: none. ON: JM05-style level bombers with Paratrooper racks get a target point. The two extra stream-0 draws per round shift every later stream-0 consumer from the first drop on. |
| `construct_007ab8c0` | none (the host creates no MParatrooper round; `game_hosts_units.cpp:22656`) | SoldierGlobals (`soldier_traffic_cc13::load_soldier_globals_0084cb70`) | one **stream-1** draw per round created | ON: stream-1 consumers (`0074A9C8` landing ship, plane tasks) shift by one draw per paratrooper. |
| `advance_in_air_007abb30`, `update_chute_and_target_007ab360`, `anim_clock_007ab0b0`, `step_landing_007ac000` | none; the round would be a new entity kind in the units host. The bomb step `006E1510` / `006E2220` would be the host's existing bomb flight. | class row `bulletclasses.lua` "Paratrooper" (SlowFactor*, CaptureDuration, Damage) and BombClass `+5Ch`; ground height and normal; avoid zones (`include/bsp/avoid_zone_boundary.hpp`); command-building side and `+368h`; MP mode | none | ON: rounds descend and steer to the jittered point. They land, or die on water or on a slope above `MaxParatrooperGradient`, then find the nearest enemy building or die. They damage the building at `(Damage - +368h)/s` and die after CaptureDuration. |
| `land_and_spawn_soldier_007ab590`, `on_killed_007aae10` | `src/game_hosts_ship_ai.cpp:572` and `:12443` (arm 3, the paratrooper list `+7DCh`, "always empty in this process") | building handle; the class SoldierClass and SoldierAnim | `004B0F80` draws U(0, duration) on **stream 0** in the image (`004B102F`). This is presentation: route it to a keyed presentation stream or keep it OFF (RNG rule). | ON: capture arm 3 of the command-building step (`006F6760`) sees capturers, so capture progress and capture reports move. |

The capture itself (`006F6760` adds `CapturePower` per capturer) is the ship-AI host's arm 3 and is not
modelled here.

## 3. Open questions

- **`flying` (`+458h`).** The only native writes are the constructor's clear (`007AB980`) and the network
  create (`006E1A35`, from message `+11Ch`). `BSP_BombProjectile_Activate` (`006E2E10`) binds it as the script
  property `{3, &+458h}` / `{0, "flying"}` under `"_bulletBase"` (`006E30D8`-`006E30F7`, `BSP_LuaReader_ReadField`
  `00BD6830`). Byte scans for stores (`C6 ?? 58 04 00 00 01`, `88 ?? 58 04 00 00` and their SIB forms,
  `C7`/`66 89`/`0F 9x`) found no other writer.
  - Without `flying`, `007AB360` holds the round in "fall" and never calls `+19Ch`, and `007AC000` never
    lands it.
  - It is not settled whether a rack-dropped round reaches `flying = 1` through that property read. A
    host bind must answer that before a JM05 prediction is trusted.
- `class+5Ch` is the BombClass step scale (CC13_FLAK_FLYTIME_BURST.md); its Lua key was not read here.
- `007AB590`'s soldier attach (`vslot +98h` with `+3Ch`/`+30h` and an identity matrix) and the chute hide are
  scene work, left to the host.
- `00BF9940` is read as `acos` from its use only (as in CC13_SOLDIER_TRAFFIC.md).
- The x87 keeps 80-bit intermediates. The models carry them in double, so a last-bit difference against the
  image is possible.

## 4. Ledger

Function records (`config/reconstruction`): `007AB8C0`, `007ABA50`, `007AC530`, `007AB230`, `007ABB30`,
`007AB360`, `007AB0B0`, `007AC000`, `007AB590`, `007AAE10`, `006E3F90`. Fragments: `006E541F`, `006E55E7`.
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
  `006E3F90`/`007BCCF0` at `007C0EC1`/`007C0EC9`, and the landing, steer and kill calls), with 0 failures. The
  row file is `local/g46_calls.json`, which is not committed.
