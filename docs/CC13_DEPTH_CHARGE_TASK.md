# The plane depth-charge task, reconstructed (packet cc13-depthcharge1)

Addresses: 009A6970, 009A5240, 009A4DC0, 009A3390, 009A31A0, 009A3290, 009A3300, 009A35A0, 009A36A0, 009A3750,
009A3770, 009A39C0, 009A39F0, 009A3AE0, 009A3B70, 009A3C60, 009A3CF0, 009A3E70, 009A3FA0, 009A3FE0, 009A4010,
009A4890, 009A48D0, 009A4900, 009A4940, 009A5330, 009A5390, 009A53B0, 009A5420, 009A54B0, 009A54C0, 009A54D0,
009A5500, 009A5610, 009A56C0, 009A5710, 009A5780, 009A57D0, 009A5870, 009A5AF0, 009A5D00, 009A5D50, 009A5D80,
009A5DB0, 009A5F10, 009A5F50, 009A6500, 009A65F0, 009A66C0, 009A6820, 009A68C0, 009A69E0, 007B5350, 009FB5E0,
009A19B0.

Files: `include/bsp/depth_charge_task_cc13.hpp`, `src/depth_charge_task_cc13.cpp`, `reports/cc13_depth_charge_task.json`.

## 0. Short answer

- **What it is.** The task a plane flies when 007EEC50 picks the `depthcharge` class (`00E08F38`): the plane carries
  ordnance kind `2Ch` and the target is a submarine (`IsKindOf(8)`). Kind 1, `7BCh` bytes, nine states
  (docs/BOT_TASK_STATES.md names them).
- **Status.** Reconstructed and build-tested (`./scripts/build.ps1`, 2 of 2 tests pass). It is not host-bound, not
  fixture-tested and not game-validated. `kDepthChargeTaskBound` is declared OFF and nothing calls the new code.
- **Interface.** Same pattern as the rocket and kamikaze tasks:
  - one value per unit (`DepthChargeTask`);
  - pure per-state functions;
  - a host interface (`DepthChargeTaskHost`, reads and sinks);
  - a keyed RNG interface (`DepthChargeRandom`, `#dc<key>`, never stream 0).

  The driver calls are `depth_charge_task_construct_009a5240`, `depth_charge_task_arm_009a66c0` (each think) and
  `depth_charge_cruise_profile_009a6500`.
- **Cadence.** The cruise profile (slot `+54h`) runs on **activation only**, when the task becomes the head of the
  bot's task list (docs/CC13_TASK_PROFILE_CADENCE.md). The per-think entry is slot `+64h`.
- **Reuse.** The rule `009A5870`, its entry chooser `009A57D0`, the attack-state set `009A5420`, the release
  window `009A3FA0` and the state names come from `include/bsp/bot_task_states.hpp` (packet cc2_bot_task_states).
  They were re-read against the listing in this packet and are unchanged. Other helpers reused:
  - `weapon_fall_time_007bcc80` (dive_bomb_task.hpp);
  - `approach_target_ref_construct_009fb200`;
  - `clamped_interpolate_00419010`, `wrapped_angle_add_00438aa0` and `wrapped_angle_subtract_00438b10`;
  - `heading_angle_00414eb0` and `length_2d_00414c60`.
- **Glue.** The units-host diff in section 7 was applied in this tree and built with both host files recompiled. It was
  then reverted, and it passes `git apply --check` against origin/main `b571eaa62`. It also feeds the chooser's
  `has_depth_charge_ordnance` behind the same switch.
- **Reach.** USN21 (`usn_sibuyan.lua:721-739`) is the one confirmed script order. IJN12, BSM11 and nine more missions
  have co-presence only (section 8).

## 1. Entry points

- **Factory.** `0099A170`'s `depthcharge` arm calls `BSP_BotTask_MakeDepthCharge` `009A6970` at `0099A462`.
  `009A6970` runs `operator_new(7BCh)` and then `009A5240(bot, target)`. The load-path factory `009A69E0` (over
  `009A6820`) has no code reference.
- **Constructor.** `009A5240`, `RET 8`:
  - task base `0099C6F0(bot, 1)` at `009A5266`;
  - approach and states `009A4DC0` at `009A5281`;
  - vtables `00D1F738` (task), `00D1F734` (`+3F8h`) and `00D1F730` (`+4F4h`);
  - `+310h` = moveto (`+508h`) when `007B8AD0(unit)`, else follow (`+544h`), entered through its `vt[4]`;
  - `009F9980` (bind) at `009A52CC`.
- **The approach.** `009A3390(unit, target)`:
  - `009F9CE0` with Pilot/DepthCharge/ReferenceSpeed (`+4B8h`);
  - four draws;
  - `009FB200` on `approach+7Ch` (the target ref);
  - `009A31A0`, `009A3300` and `009A3290`.

  `009A4DC0` then builds the registry and the nine states, and `009A3090` registers their names.
- **Per think.** Task vtable slot `+64h` = `009A66C0` (section 3).

## 2. The object

`approach+N` is relative to `task+3F8h`. The task's `+46Ch/+46Dh/+46Eh` are `approach+74h/+75h/+76h`, and `task+48Ch` is
`approach+94h`. That names the inputs bot_task_states.hpp calls `flag_46C`/`flag_46D`: **has depth charges** and **in
range**.

| field | writer | meaning |
| --- | --- | --- |
| `+24h` | `009F9CE0` | `max(1, MaxSpd / Pilot/DepthCharge/ReferenceSpeed)` |
| `+2Ch` | ctor 0; `009A5D00` +1; every release -1 | the drop balance (bot_task_states.hpp's `kRoundsRemaining` is a misnomer) |
| `+30h` | `009A33F8` | `U(0.9, 1.1) * 007C47F0` (LevelFlight * StallSpd), the run speed |
| `+34h` | ctor `U(0, AimAltRange/2 - /1)`; `009A5F74` 0 while `ctl+3ADh` is clear | aim-altitude spread |
| `+38h` | ctor `U(ManeuverAltRange/1, /2)`; reset to `/1` with `+34h`; capped by `ctl+394h` | manoeuvre altitude |
| `+3Ch` | `AimAltRange/1`, capped by `ctl+394h` | aim-altitude base; `+3Ch + +34h` is the aim altitude |
| `+40h` | `U(1.1, 1.3) * TurnCircleRadius + FlyAboveDist * +24h` | fly-above distance |
| `+44h` | `1.5 * +40h`; the profile raises it to `AttackDist * +24h` | attack range |
| `+48h` | `009A5F50` | planar distance to the aim point |
| `+4Ch` / `+50h` | `009A5F50` | bearing to the aim point; bearing minus the unit heading |
| `+54h` | 3600, `+= dt` each update; slot `+2Ch` zeroes it | elapsed time |
| `+58h` | ctor 30.0, then `009A3290` | half the stick length: `0.5 * max(rounds * 0.4, 0.5) * +30h` |
| `+5Ch` | `009A5F50` | release bias, grows while the plane is inside `+58h` |
| `+60h` | `009A5F50` | run-in distance (along track), the key every interpolation reads |
| `+64h` | `009A3300` | fall lead: `(0.15 - min(aim y, 0) * 0.02) + 007BCC80(unit, unit y)` |
| `+68h` | `009A5F50` | off-track angle of the release point |
| `+6Ch` | `009A31A0` | projtime error `U(-CalcTargetPosError, +)` |
| `+70h` | `009A5F50` | target-lost timer |
| `+74h` / `+75h` / `+76h` | update / update / aim, prepare and goaway | has depth charges / in range / arm request |
| `+78h` | `FFh` per think, 3 in leave, turnto, aim and the gates | weapon selector, copied to `task+2E4h` |
| `+7Ch` | `009FB200` | target ref (sub+18h target, sub+1Ch world point, sub+44h projtime) |
| `+F0h` | update | release point (y forced to 0 with ordnance aboard) |

The states:

| state | task offset | vtable | ctor seeds | enter | tick |
| --- | --- | --- | --- | --- | --- |
| moveto / follow | `+508h` / `+544h` | host | host | host | host |
| done / prepare | `+5DCh` / `+6E4h` | `00D1F6FC` | `+98h` = -1, `+9Ch` = 0 | `009A4890` | `009A4940` |
| attackrun | `+67Ch` | `00D1F64C` | `+18h` 1, `+1Ch` `-U(0,1)`, `+20h` 0 | `009A3750` | `009A3770` |
| goaway | `+6A0h` | `00D1F6A0` | `+18h..+20h` unwritten | `009A3C60` | `009A3E70` |
| aim | `+6C4h` | `00D1F6D8` | 0 | `009A3FE0` | `009A4010` |
| leave | `+784h` | `00D1F668` | `+18h` 5.0 | `009A39C0` | `009A39F0` |
| turnto | `+7A0h` | `00D1F684` | `+18h` 0 | `009A3AE0` | `009A3B70` |

The exits are `007B3DC0` (`RET`) except done and prepare, which use `009A48D0` (a drop when the gate `+98h >= 0`,
then `009BDE40`). The goaway constructor leaves `+18h..+20h` unwritten. The reconstruction zeroes them, and its
enter writes all three before any tick reads them.

## 3. Per think: `009A66C0` (slot `+64h`)

Read whole from the listing. Same order as docs/BOT_TASK_STATES.md, with the fields named:

1. `approach+78h = FFh`.
2. `009A5F50(dt)` (section 4).
3. `009BDE80(moveto, aimAlt, aimAlt, +40h)`.
4. The rule `009A5870`.
5. The current state's `vt[0Ch](dt)`.
6. `task+2E4h = approach+78h`.
7. The manual-release passthrough: when `+2Ch > 0`, the unit device asks (`(unit+72Ch)->vt[38h]`) and the state is
   none of aim, attackrun, turnto or prepare, it calls `007BBBA0` and sets `+2Ch -= 1`.

**The rule.** The table in docs/BOT_TASK_STATES.md is correct. The reconstruction evaluates `vt[1Ch]` (`009A65F0`),
`009A5610`, `009A5390` and `009A53B0` only on the branch that reads them, so their host calls happen at the image's
cadence.

**The predicates** (the earlier docs mark these bodies `contract: unread`):

- **turnto -> aim**, `009A5610`: `max(|009A19B0(plan)|, 0.5) > |+50h|`. `009A19B0` returns `0099D0A0(plan,
  [plan+2F0h]+C68h)`, or 0 with no plane.
- **turnto -> goaway**, `009A5390`: `+40h * 0.5 > +48h` (too close to turn in).
- **goaway done**, `009A53B0`: `+48h > standoff +1Ch`. Under `ctl+369h && [00E17BF2]` it needs depth charges aboard
  and the threshold is `standoff - 200`.
- **break-off**, `009A65F0`:
  - false when the base `0099C230` says so;
  - true when the target is gone or `+5Dh`;
  - false under the alt loadout, or in an attack state with depth charges;
  - otherwise true when `|aim point - unit|` (`0042B2F0`, 3-D) `>= SafeDist * +24h`.

**The state ticks:**

| state | writes |
| --- | --- |
| attackrun `009A3770` | every period: `+1Ch` refills, and `+20h = -a.x * b.y * b.z * 30 deg` from `007F0280` (extents 80/60/120, zero weights, mode 1, no `|a.x|` gate). Then heading `AddWrapped(+4Ch, +20h)`, `009FBA50(aimAlt, +40h, +48h, 0.9)`, throttle 0.98, `gun+40h = Angle_MoveTo`, `009FABE0(009A1A20(plan), 0099B630(plan))` |
| leave `009A39F0` | `+78h = 3`, `+18h -= dt`, `009FB800(2000, 1.5)`, bank 0, throttle 1.0. Enter: `+18h = U(4, 7)` |
| turnto `009A3B70` | `+78h = 3`, bank `+18h`, speed `class+190h` (TravelSpeed * NewTravelSpeedMul), `009FBA50(+38h, +40h * 1.4, +48h, 0.6)`. Enter: `+18h = TurnRoll * (+50h > 0 ? -1 : 1) * (kind 10h/16h ? 1.1 : 1.3)` |
| goaway `009A3E70` | throttle 1.0, `009FB800(+18h, 1.0)`, heading `009A3CF0`, `gun+40h = Angle_GoAway`, then `009FABE0` with a **second** `009A3CF0` call. Enter: side from `[00F876B0]` parity, `+18h = has DC ? +38h : ctl+394h`, `+76h = 0`, standoff `U(200, 400) + TurnCircleRadius` |
| aim `009A4010` | section 5. Enter: reset, `009A31A0` (a new aim offset, 3 draws plus the pick), `009A3290` |
| done / prepare `009A4940` | the follow tick `009C1FD0`, then the salvo gate below. Enter `009A4890`: `+98h = -1`, `009BED80`, `ctl+3E4h = 4`, `007ED260`, `+8Ch = 1` |

`009A3CF0` (goaway heading):

1. The fly-to solver `009FD570(point, unit, standoff, &side, +48h, 1.0)` gives `h0`. It writes the side back on each
   of the two calls.
2. The probe `007F0280` (extents 60/50/80, mode 1) gives `u = -a.x * b.y * b.z`.
3. `e = Sub(h0, heading)` is clamped to `+-25 deg` (`00D1F6C0`).
4. When `e` and `u` have opposite signs, `e += u * 30 deg`.
5. The heading is `AddWrapped(heading, e)`.

**The done/prepare salvo** (`009A4940`):

- While the gate `+98h > 0`:
  - `+78h = 3`, `gun+40h = 0`, `+76h = 1`, `+98h -= dt`;
  - `+98h = -1` when `+60h > 400`;
  - **drop** when `+98h <= 0`, or inside the release window, or `+68h > 1.5`. A drop is `007BBBA0`, `+2Ch -= 1`,
    `+9Ch -= 1`, the gate closed at `+9Ch <= 0`, `009A3290`, and the gate set to 0.3 s for the next round;
  - otherwise pitch target 0 (mode 2) and heading `unit + clamp(+50h, -0.3, 0.3)`.
- With the gate closed:
  - `gun+40h = Angle_Prepare`, and `+76h` copies the leader's byte `leader+9C1h + [00F876B8] * 8`;
  - below 400 m the follow speed factor is `+8Ch = Interp(1.1, 1.0, 0.4, 0.0, d / (Interp(250, 1, 400, 0.01, ly) *
    Interp(0.5, 1, 1.5, 0.2, |hdg err|) * (+24h * FlyAboveDist + TurnCircleRadius)))`;
  - above 400 m `+8Ch = 1`.
- The gate opens only through `009A5330` (from slot `+24h` `009A5D00`, the unit's drop request): `+9Ch += 1` and
  `+98h = U(1.5, 3.0)` when the gate is closed. The salvo is the player-commanded drop. An AI wingman reaches the
  release only through aim.

## 4. The approach update `009A5F50`

Read whole from the listing (BOT_TASK_STATES had `009A6043-009A64F3` unread). The steps:

1. **Altitude caps.** Without `ctl+3ADh`: `+34h = 0` and `+38h = ManeuverAltRange/1`. Then `+3Ch = min(+3Ch,
   ctl+394h)` and `+38h = min(+38h, ctl+394h)`, both FCOMIP with `JBE` taking the ctl value.
2. **Ordnance.** `+74h = 007B94F0(unit, 0)`, the kind-2Ch test.
3. **Target.** No target (sub+18h): `+75h = 0`, `+68h = 0`, `+54h += dt`, return. Then the spotted count
   `target + ctl+54h * 34h + 1E8h`:
   - `>= 1`: `009FADA0(dt)` and `+70h = 0`;
   - otherwise, inside 1500 m (`00CED724`): `+70h += dt`. Past Pilot/DepthCharge/TargetLostTime it calls
     `009FB5E0(ref, 0)`, which drops the target, and goes to the no-target exit;
   - beyond 1500 m the last point is kept.
4. **Geometry** on `vt[0]` (`009A35A0` = sub+1Ch):
   - `+48h` = planar length (`00414C60`);
   - `+4Ch` = heading (`00414EB0`);
   - `+50h = Sub(+4Ch, vt[50h])`;
   - `+75h = (+44h + 100 > +48h) || (+75h && 007B58D0)`.
5. **No depth charges:**
   - projtime 0, release point = unit position, `+58h = 0`, `+68h = 0`;
   - under the alt loadout, keep `+75h`;
   - otherwise `+75h = leader-in-range && +44h > +48h`. Leader-in-range is true for the leader itself, else
     `+44h >` the planar distance from `ctl+3D0h` to the aim point.
6. **With depth charges:**
   - `009A3300` sets `+64h`;
   - projtime = `clamp(min(20, +48h / max(1, vt[38h])) + +6Ch, 0, 30)` (`007B5350`);
   - release point = `unit + vt[34h] velocity * +64h`, with y set to 0;
   - `+68h = |Sub(vt[50h], bearing from the release point)|`;
   - beyond `max(60, +58h)`: `+60h = +48h`, `+5Ch = 0`;
   - otherwise `+60h = cos(+68h) * d`. When `+58h > +60h`, then `+60h = +58h - 1` and `+5Ch += +58h * dt * 0.1`;
     else `+5Ch = 0`.
7. `+54h += dt` on every exit except the head returns.

`009A5F50`'s pseudocode in the export garbles the 3-vector arithmetic. The table above is from the listing.
`009A3300`'s pseudocode is also wrong: it adds the unit height, but the `POP EDI` at `009A336D` shifts the frame, so
the addend is the `007BCC80` result. The reconstruction follows the listing.

## 5. The aim tick `009A4010` and the release

The heading arithmetic, read whole (bot_task_states.hpp had only the release tail):

- `A = Interp(80, 0.8, 300, 2.0, r) * 100` and `B = Interp(50, 20 deg, 350, 75 deg, r)`, with `r = +60h`.
- The probe `007F0280` uses extents `(A, 150, 250)`, weights `(0.4A, 0, 0)` and mode 1. When `|a.x| > 0.05`:
  - `u = -a.x * b.y * b.z` with a dead band of `+-0.5` (`o = 2(u - 0.5)` or `2(u + 0.5)`);
  - `o *= 15 deg * Interp(100, 0.25, 350, 1.0, r)`;
  - the speed term is `w = -a.z * b.x * b.y`.
- Heading: `AddWrapped(h, clamp(Sub(AddWrapped(+4Ch, o), h), -B, B))`, mode 2, and `plan+2C8h = Interp(250, 0.2,
  500, 1.2, r)`.
- Altitude: `009FBA50(aimAlt, +40h, +48h, Interp(0.01, 0.8, 0.35, 1.8, g))`, with `g = (y - (aimAlt - 30)) / max(r -
  (+58h + +5Ch + 150), 1)`.
- Speed:
  - `Interp(0, Interp(100, +30h, 450, class+190h, r), 1, 007C47F0, v) + 15 * w`;
  - `v = Interp(aimAlt + 80, 0, aimAlt + 180, 1, y) * Interp(0.15, 0, 0.7, 2, g) * Interp(450, 1.2, 1000, 0, r)`.
- **Pull out:** `+18h = +50h > 1.5` (signed, so only one side).
- **Arm** (`+76h = 1`, `+78h = 3`) when `r - (+58h + +5Ch) < 200`.
- **Release** (`007BBBA0`, `+2Ch -= 1`, `+1Ch = U(0.9, 1.1) * 0.4`). All of these must hold:
  - the target is set and clear of `+5Dh`;
  - depth charges are aboard;
  - `+1Ch < 0`;
  - `009A3FA0` (`r < +58h + +5Ch`);
  - `y < aimAlt + 150`.

The release is the shared drop request; the depth-charge projectile's scan and burst are already hosted in
`src/bomb_torpedo_tick.cpp`.

## 6. Random draws (all `00BD2F10`, `ECX = 1`)

`local/dc1_calls.py 009a3090 009a6a60 00bd2f10` finds 12 rel32 call sites in the window. The scope doc's list
omitted `009A4646`. The rel32 scan is not vacuous: the same script finds the three callers of `009A31A0`.

| id | site | function | distribution | key | when |
| --- | --- | --- | --- | --- | --- |
| `kRunSpeed30` | `009A33F8` | `009A3390` | `U(0.9, 1.1)` | `#dc30` | install |
| `kAimAltSpread34` | `009A3440` | `009A3390` | `U(0, AimAltRange/2 - /1)` | `#dc34` | install |
| `kManeuverAlt38` | `009A346F` | `009A3390` | `U(ManeuverAltRange/1, /2)` | `#dc38` | install |
| `kFlyAbove40` | `009A34AA` | `009A3390` | `U(1.1, 1.3)` | `#dc40` | install |
| `kBiasX` | `009A31D1` | `009A31A0` | `U(-TargetHError, +)` | `#dcbx` | install, aim enter |
| `kBiasZ` | `009A3201` | `009A31A0` | `U(-TargetVError, +)` | `#dcbz` | install, aim enter |
| `kProjError6C` | `009A3281` | `009A31A0` | `U(-CalcTargetPosError, +)` | `#dc6c` | install, aim enter |
| `kAttackRunCd1C` | `009A36FA` | `009A36A0` | `-U(0, 1)` | `#dcar1c` | install |
| `kLeaveTimer18` | `009A39DE` | `009A39C0` | `U(4, 7)` | `#dclv18` | leave enter |
| `kGoAwayStandoff1C` | `009A3CD3` | `009A3C60` | `U(200, 400)` | `#dcga1c` | goaway enter |
| `kAimCooldown1C` | `009A4646` | `009A4010` | `U(0.9, 1.1) * 0.4` | `#dcaim1c` | after each release |
| `kPrepareGate98` | `009A5360` | `009A5330` | `U(1.5, 3.0)` | `#dcpr98` | drop request in prepare |

Outside the window, and the host's:

- the target ref's first timer `009FB2E7` (`#dcref60`, between `#dc40` and `#dcbx`);
- the `009FA260` pick (between `#dcbz` and `#dc6c`);
- the task base `0099C6F0`;
- the follow constructors.

**Order at install:** `#dc30`, `#dc34`, `#dc38`, `#dc40`, [`009FB2E7`], `#dcbx`, `#dcbz`, [pick], `#dc6c`,
[follow ctors], `#dcar1c`.

## 7. Glue for the units host (suggested, NOT applied)

The diff below is against origin/main `b571eaa62`. It was applied in this tree, and `./scripts/build.ps1` passed with
both host files recompiled. It was then reverted, and `git apply --check` on a clean origin/main checkout passes. The
pieces:

- **`GameUnitSlot`** gains the task value and two counters.
- **The arm dispatch** gains `run_depth_charge_task_arm_009a66c0`, behind `bsp::kDepthChargeTaskBound`.
- **The chooser.** `choose_attack_class_007eec50` in `src/game_hosts_script_orders.cpp` feeds `has_depth_charge_ordnance`
  (`set.contains(0x2c)`, Type "DepthCharge") behind the same switch. They must land together. With the feed alone, a
  float plane ordered against a submarine would take a class no arm flies.
- **The adapter** maps each host call onto what the strafe arm already reads:
  - the aim point (`aim_point_009fada0`);
  - liveness (`df_slot_live`);
  - the near-field probe (squadron candidates);
  - `st_cruise_pitch_009fba50`, `pitch_command_009fb800`, `st_speed`;
  - the throttle slots and the generic follow tick.

  Substitutions, labelled in the code:
  - the robots rows come from this installation's robots.lua;
  - `ctl+394h` is the script altitude or CruisingAlt;
  - the leader byte `+9C1h` reads 0;
  - the fly-to solver is replaced by a straight heading;
  - `009A19B0` reads 0;
  - no device drop request;
  - the gun direction is not carried.
- **Install** is per class and target, as strafe does. The activation profile runs once at install.

```diff
diff --git a/src/game_hosts_script_orders.cpp b/src/game_hosts_script_orders.cpp
index 7186a98a6..5c6531983 100644
--- a/src/game_hosts_script_orders.cpp
+++ b/src/game_hosts_script_orders.cpp
@@ -7,6 +7,7 @@
 #include "bsp/game_hosts_script_orders.hpp"
 namespace bsp::game { void harness_note_mission_end() noexcept; }   // src/game_hosts_mission.cpp
 #include "bsp/strafe_task.hpp"
+#include "bsp/depth_charge_task_cc13.hpp"
 #include "bsp/game_hosts_commands.hpp"
 
 #include "bsp/air_operations.hpp"
@@ -1421,6 +1422,9 @@ std::uint32_t GameScriptOrdersHost::choose_attack_class_007eec50(std::size_t uni
     in.has_general_bomb_ordnance = bsp::ordnance_has_general_bomb_2ah(set);
     in.has_drop_kamikaze_ordnance = bsp::ordnance_has_drop_kamikaze_2fh(set);
     in.has_torpedo_ordnance = bsp::ordnance_has_torpedo_2bh(set);
+    // Packet cc13-depthcharge1: 007EDA10 -> 007B94F0, kind 2Ch (Type
+    // "DepthCharge"). Lands with the task, never apart from it.
+    if constexpr (bsp::kDepthChargeTaskBound) in.has_depth_charge_ordnance = set.contains(0x2c);
     // Packet cc9_kamikaze_ship_blocked: 007EEB64..007EEB8A. 00827F70 on the
     // target's class, then 00604A50 on the slot-0 plane (the ordered unit, as
     // the self queries above take it): kind 17h with PilotFires clear.
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..c5a1e1da4 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -63,6 +63,8 @@
 #include "bsp/torpedo_task_arm.hpp"
 #include "bsp/attack_commands.hpp"
 #include "bsp/strafe_task.hpp"
+#include "bsp/depth_charge_task_cc13.hpp"
+#include "bsp/ordnance_kinds.hpp"
 
 #include "bsp/game_hosts.hpp"
 #include "bsp/game_observer_runtime.hpp"
@@ -533,6 +535,13 @@ struct GameUnitSlot {
     // Packet cc9_dogfight_task: the dogfight task (kind 2), installed when
     // 007EEC50 chose class 00E08F58. docs/DOGFIGHT_TASK.md.
     bool dogfight_task_installed{false};
+    // Packet cc13-depthcharge1: the depth-charge task 009A5240, behind
+    // bsp::kDepthChargeTaskBound. docs/CC13_DEPTH_CHARGE_TASK.md 7.
+    bool depth_charge_task_installed{false};
+    std::size_t dc_target_plus_one{0};
+    bsp::DepthChargeTask dc{};
+    unsigned long long dc_transitions{0};
+    unsigned long long dc_releases{0};
     // Packet cc9_strafe_arm (cc9-lua30): the strafe task (kind 0Ah, 6FCh bytes),
     // installed when 007EEC50 chose class 00E08F40, behind bsp::kStrafeTaskBound.
     // Approach fields by their 009CA4A0 offsets (approach = task+3F8h), state
@@ -22379,6 +22388,297 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         owner_.done("BotTaskStrafe::arm", 0x009cd170u);
                     }
 
+                    // Packet cc13-depthcharge1 (bsp::kDepthChargeTaskBound, committed
+                    // OFF): the depth-charge task, kind 1, 009A6970 -> 009A5240 and
+                    // the arm 009A66C0. src/depth_charge_task_cc13.cpp holds the task;
+                    // this adapter maps its host calls onto what the strafe arm
+                    // already reads. Every SUBSTITUTION is labelled.
+                    // docs/CC13_DEPTH_CHARGE_TASK.md 7.
+                    void run_depth_charge_task_arm_009a66c0(float dt) {
+                        if (unit_.attack_command_class != bsp::kAttackCmdDepthCharge) {
+                            unit_.depth_charge_task_installed = false;
+                            return;
+                        }
+                        if (unit_.command_target_plus_one == 0) return;
+                        struct Adapter final : bsp::DepthChargeTaskHost, bsp::DepthChargeRandom {
+                            PlaneBinding& a;
+                            float moveto_near{0.0f}, moveto_far{0.0f}, moveto_speed{0.0f};
+                            explicit Adapter(PlaneBinding& arm) : a(arm) {}
+                            float uniform_00bd2f10(const char* key, float lo, float hi) override {
+                                return a.owner_.release_altitude_draw_00bd2f10(
+                                    a.unit_.row.name + "#dc" + key, lo, hi);
+                            }
+                            bsp::DepthChargeTuning tuning() override {
+                                bsp::DepthChargeTuning t;
+                                if (a.owner_.lua.plane_globals_loaded()) {
+                                    const bsp::GameTuningBlock& g = a.owner_.lua.plane_globals();
+                                    t.aim_alt_range_1_494 = g.pilot_depth_charge_aim_alt_range_1;
+                                    t.aim_alt_range_2_498 = g.pilot_depth_charge_aim_alt_range_2;
+                                    t.maneuver_alt_range_1_49c = g.pilot_depth_charge_maneuver_alt_range_1;
+                                    t.maneuver_alt_range_2_4a0 = g.pilot_depth_charge_maneuver_alt_range_2;
+                                    t.cruising_alt_4a4 = g.pilot_depth_charge_cruising_alt;
+                                    t.fly_above_dist_4a8 = g.pilot_depth_charge_fly_above_dist;
+                                    t.attack_dist_4ac = g.pilot_depth_charge_attack_dist;
+                                    t.safe_dist_4b0 = g.pilot_depth_charge_safe_dist;
+                                    t.reference_speed_4b8 = g.pilot_depth_charge_reference_speed;
+                                    t.target_lost_time_4bc = g.pilot_depth_charge_target_lost_time;
+                                }
+                                t.angle_prepare_66c = a.owner_.strafe_cone_tuning(0x66C);
+                                t.angle_move_to_670 = a.owner_.strafe_cone_tuning(0x670);
+                                t.angle_go_away_674 = a.owner_.strafe_cone_tuning(0x674);
+                                return t;
+                            }
+                            bsp::DepthChargeRow robot_row() override {
+                                // SUBSTITUTION, labelled, as the strafe rows: this
+                                // installation's robots.lua (mtime 2025-06-01) PilotBot
+                                // DepthCharge* by line: Stun :1286-1290, SPNormal :596-600,
+                                // SPVeteran :734-738, MPNormal :872-876, MPVeteran
+                                // :1010-1014, Elite :1148-1152.
+                                static constexpr bsp::DepthChargeRow kRows[6] = {
+                                    {100.0f, 100.0f, 20.0f, 200.0f, 1.0f},   // Stun
+                                    {15.0f, 30.0f, 4.0f, 200.0f, 1.0f},      // SPNormal
+                                    {0.0f, 0.0f, 2.0f, 200.0f, 0.4f},        // SPVeteran
+                                    {15.0f, 40.0f, 5.0f, 200.0f, 1.0f},      // MPNormal
+                                    {0.0f, 10.0f, 1.0f, 200.0f, 0.5f},       // MPVeteran
+                                    {0.0f, 0.0f, 2.0f, 200.0f, 0.4f},        // Elite
+                                };
+                                const int i = a.unit_.pilot_skill_index;
+                                return (!kSkillLevelBound || i < 0 || i > 5) ? kRows[1] : kRows[i];
+                            }
+                            bsp::DepthChargeClass class_block() override {
+                                bsp::DepthChargeClass c;
+                                c.max_spd_188 = a.unit_.plane_max_spd;
+                                c.travel_speed_190 = a.owner_.retreat_squadron_speed_3a0(a.unit_);
+                                c.turn_roll_25c = a.unit_.plane_class_turn_roll;
+                                c.turn_circle_radius_268 = a.unit_.plane_turn_circle_radius;
+                                c.level_flight_speed = a.owner_.bot_desired_speed_007c47f0(a.unit_);
+                                return c;
+                            }
+                            bsp::DepthChargeControlView control() override {
+                                bsp::DepthChargeControlView v;
+                                v.attack_mode_370 = static_cast<int>(a.unit_.st_attack_mode_370);
+                                // ctl+394h. SUBSTITUTION, labelled: the script's travel
+                                // altitude when one is set, else what the activation
+                                // profile 009A6500 stores there (Pilot/DepthCharge/
+                                // CruisingAlt); ctl+3ADh is then set either way.
+                                v.alt_394 = tuning().cruising_alt_4a4;
+                                const GameUnitSlot* sq = a.owner_.squadron_slot_of(a.unit_.process_index);
+                                if (sq != nullptr && sq->sq_alt_set) v.alt_394 = sq->sq_alt_394;
+                                v.flag_3ad = true;
+                                v.alt_loadout = false;   // [00E17BF2] = 0 in single player
+                                return v;
+                            }
+                            bsp::DepthChargeLeaderView leader() override {
+                                bsp::DepthChargeLeaderView v;
+                                const bsp::PlaneSquadronHostRecord* sq =
+                                    bsp::plane_squadron_registry().find_by_member_unit(a.unit_.process_index);
+                                if (sq == nullptr) {
+                                    v.present = true;
+                                    v.is_self = true;
+                                } else {
+                                    for (const std::size_t m : sq->member_units) {
+                                        if (m == bsp::kPlaneSquadronNoUnit || m >= a.owner_.slots.size()) continue;
+                                        if (!a.owner_.unit_is_flight_leader_007b8ad0(m)) continue;
+                                        const GameUnitSlot& l = *a.owner_.slots[m];
+                                        v.present = true;
+                                        v.is_self = m == a.unit_.process_index;
+                                        v.position = {l.motion.position[0], l.motion.position[1],
+                                                      l.motion.position[2]};
+                                        v.heading_50 = l.plane_heading_c6c;
+                                        break;
+                                    }
+                                }
+                                v.arm_9c1 = 0;   // SUBSTITUTION, labelled: leader+9C1h is not hosted
+                                return v;
+                            }
+                            std::array<float, 3> unit_position() override {
+                                return {a.unit_.motion.position[0], a.unit_.motion.position[1],
+                                        a.unit_.motion.position[2]};
+                            }
+                            std::array<float, 3> unit_velocity_34() override {
+                                return {a.unit_.plane_world_velocity[0], a.unit_.plane_world_velocity[1],
+                                        a.unit_.plane_world_velocity[2]};
+                            }
+                            float unit_speed_38() override { return a.st_speed_vt38(); }
+                            float unit_heading_50() override { return a.unit_.plane_heading_c6c; }
+                            bool bomber_kind() override { return a.st_bomber_kind(); }
+                            bool flight_leader_007b8ad0() override {
+                                return a.owner_.unit_is_flight_leader_007b8ad0(a.unit_.process_index);
+                            }
+                            bool has_depth_charge_ordnance_007b94f0() override {
+                                // SUBSTITUTION, labelled: the slot's static mask.
+                                return bsp::OrdnanceKindSet{a.unit_.ordnance_mask}.contains(0x2c);
+                            }
+                            int remaining_rounds_007c1db0() override {
+                                return a.owner_.dive_bomb_rounds_remaining(a.unit_);
+                            }
+                            int parity_f876b0() override {
+                                return static_cast<int>(a.unit_.process_index);   // LABELLED stand-in
+                            }
+                            int target_spotted_count_1e8() override {
+                                const GameUnitSlot* t = a.st_target();   // LABELLED: seen while live
+                                return t != nullptr && df_slot_live(*t) ? 1 : 0;
+                            }
+                            bool target_marked_5d() override {
+                                const GameUnitSlot* t = a.st_target();
+                                return t == nullptr || !df_slot_live(*t);
+                            }
+                            bool update_target_ref_009fada0(bsp::ApproachTargetRefState&, float,
+                                                            std::array<float, 3>& point) override {
+                                const GameUnitSlot* t = a.st_target();
+                                if (t == nullptr) return false;
+                                float p[3] = {t->motion.position[0], t->motion.position[1],
+                                              t->motion.position[2]};
+                                a.owner_.aim_point_009fada0(a.unit_, *t, a.unit_.command_target_plus_one, p);
+                                point = {p[0], p[1], p[2]};
+                                return df_slot_live(*t);
+                            }
+                            void pick_target_ref_009fa260(bsp::ApproachTargetRefState&) override {}
+                            void clear_target_ref_009fb5e0(bsp::ApproachTargetRefState&) override {}
+                            bsp::NearFieldProbeResult near_field_probe_007f0280(const float e[3],
+                                                                                const float w[3]) override {
+                                // SUBSTITUTION, labelled: the squadron candidates (mode 0).
+                                return a.nf_probe_squadron_007f0280(e, w);
+                            }
+                            float fly_to_heading_009fd570(const std::array<float, 3>& point, float,
+                                                          float&, float) override {
+                                // SUBSTITUTION, labelled: straight at the point.
+                                return bsp::heading_command_009f9e40(point[0], point[2],
+                                    a.unit_.motion.position[0], a.unit_.motion.position[2]);
+                            }
+                            float heading_diff_009a19b0() override { return 0.0f; }   // LABELLED
+                            bool break_off_base_0099c230() override { return true; }  // AI aircraft
+                            bool device_drop_requested() override { return false; }   // no device
+                            void set_moveto_ranges_009bde80(float n, float f, float s) override {
+                                moveto_near = n;
+                                moveto_far = f;
+                                moveto_speed = s;
+                            }
+                            void enter_formation_state(bsp::DepthChargeState) override {}   // LABELLED, as strafe
+                            void exit_formation_state(bsp::DepthChargeState) override {}
+                            void tick_formation_state(bsp::DepthChargeState s, float, float) override {
+                                if (s != bsp::DepthChargeState::kMoveTo) {
+                                    a.st_follow_tick_009c1fd0();   // the generic follow tick
+                                    return;
+                                }
+                                // 009C18C0 with the depth-charge ranges 009A671A stored.
+                                const GameUnitSlot* t = a.st_target();
+                                float sep = 0.0f;
+                                if (t != nullptr) {
+                                    const double dx = static_cast<double>(t->motion.position[0]) -
+                                                      a.unit_.motion.position[0];
+                                    const double dz = static_cast<double>(t->motion.position[2]) -
+                                                      a.unit_.motion.position[2];
+                                    const double d2 = dx * dx + dz * dz;
+                                    sep = d2 > 1e-10 ? static_cast<float>(std::sqrt(d2)) : 0.0f;
+                                }
+                                a.st_speed(a.owner_.bot_desired_speed_007c47f0(a.unit_));
+                                if (t == nullptr) return;
+                                bsp::MoveToGlideInputs gin;
+                                gin.near_range_30 = moveto_near;
+                                gin.far_range_34 = moveto_far;
+                                gin.speed_range_38 = moveto_speed;
+                                gin.target_world_y = t->motion.position[1];
+                                gin.unit_world_y = a.unit_.motion.position[1];
+                                gin.planar_distance = sep;
+                                const bsp::MoveToGlideCommand g = bsp::move_to_glide_009c18c0(gin);
+                                a.st_cruise_pitch_009fba50(g.base, g.range_low, g.range_high, g.scale);
+                                a.unit_.plan_heading_2c0 = bsp::heading_command_009f9e40(
+                                    t->motion.position[0], t->motion.position[2],
+                                    a.unit_.motion.position[0], a.unit_.motion.position[2]);
+                                a.unit_.plan_heading_2c0_written = true;
+                                a.unit_.plan_heading_mode_2cc = 2;
+                            }
+                            void set_control_3e4(int) override {}               // LABELLED
+                            void assign_formation_indices_007ed260() override {}  // LABELLED
+                            void apply_plan(const bsp::DepthChargePlanCommand& c) override {
+                                GameUnitSlot& u = a.unit_;
+                                if (c.writes_heading_2c0) {
+                                    u.plan_heading_2c0 = c.heading_2c0;
+                                    u.plan_heading_2c0_written = true;
+                                    u.plan_heading_mode_2cc = 2;
+                                }
+                                if (c.writes_bank_2c4) {
+                                    u.plan_state.bank_target_2c4 = c.bank_2c4;
+                                    u.plan_heading_mode_2cc = 1;
+                                    u.plan_heading_2c0_written = false;
+                                }
+                                if (c.writes_turn_2c8) u.plan_state.bank_limit_2c8 = c.turn_2c8;
+                                if (c.writes_pitch_2bc) {
+                                    u.plan_state.pitch_target_2bc = c.pitch_2bc;
+                                    u.plan_state.pitch_mode_2d0 = c.pitch_mode_2d0;
+                                }
+                                if (c.writes_speed_2b4) a.st_speed(c.speed_2b4);
+                                if (c.writes_throttle) {
+                                    u.plan_slots[bsp::kPilotSlotThrottle].desired = c.throttle_278;
+                                    u.plan_slots[bsp::kPilotSlotThrottle].active = 1;
+                                    u.plan_slots[bsp::kPilotSlotAirBrake].desired = 0.0f;
+                                    u.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
+                                    u.plane_air_brake_mode_2d8 = 0;
+                                }
+                                if (c.calls_cruise_altitude_009fba50) {
+                                    a.st_cruise_pitch_009fba50(c.cruise_base, c.cruise_low,
+                                                               c.cruise_high, c.cruise_scale);
+                                }
+                                if (c.calls_pitch_command_009fb800) {
+                                    bsp::PlanePitchCommandInputs pin;
+                                    pin.desired_altitude = c.pitch_altitude;
+                                    pin.reference = c.pitch_reference;
+                                    pin.unit_world_y = u.motion.position[1];
+                                    if (a.owner_.lua.plane_globals_loaded()) {
+                                        const bsp::GameTuningBlock& gt = a.owner_.lua.plane_globals();
+                                        pin.ceiling = gt.dynamics_ceiling;
+                                        pin.climb_dist = gt.pilot_general_climb_dist;
+                                        pin.drop_dist = gt.pilot_general_drop_dist;
+                                    }
+                                    pin.class_climb_angle = u.plane_climb_angle_1ec;
+                                    pin.class_drop_angle = u.plane_drop_angle;
+                                    u.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
+                                    u.plan_state.pitch_target_2bc = u.plane_commanded_pitch;
+                                    u.plan_state.pitch_mode_2d0 = 2;
+                                }
+                                if (c.writes_cone_40) u.gun_cone_40 = c.cone_40;
+                                // 009FABE0 (the gun direction): not carried, as strafe.
+                            }
+                            void release_ordnance_007bbba0() override {
+                                a.owner_.release_ordnance_007bbba0(a.unit_);
+                            }
+                            void apply_cruise_profile_009a6500(const bsp::DepthChargeCruiseProfileWrite&) override {
+                                // LABELLED: control() already answers the profile's value.
+                            }
+                        };
+                        {
+                            // 0099993C: 0099B740 before the arm. LABELLED: squadron+370h
+                            // is kept on the strafe arm's field (one class at a time).
+                            bsp::PilotAttackModeInputs in;
+                            in.has_control_block_2fc = true;
+                            in.has_unit_2f4 = true;
+                            in.unit_is_flight_lead =
+                                owner_.unit_is_flight_leader_007b8ad0(unit_.process_index);
+                            in.task_authorises_38h = true;
+                            unit_.st_attack_mode_370 =
+                                bsp::pilot_attack_mode_0099b740(unit_.st_attack_mode_370, in);
+                        }
+                        Adapter host{*this};
+                        if (!unit_.depth_charge_task_installed ||
+                            unit_.dc_target_plus_one != unit_.command_target_plus_one) {
+                            // SUBSTITUTION as strafe: one task per class and target.
+                            bsp::depth_charge_task_construct_009a5240(unit_.dc, host, host);
+                            // 009A6500 on activation (docs/CC13_TASK_PROFILE_CADENCE.md).
+                            bsp::DepthChargeCruiseProfileInputs pin;
+                            pin.ctl_380 = -1.0f;
+                            pin.ctl_37c = -1.0f;
+                            bsp::depth_charge_cruise_profile_009a6500(unit_.dc, host, pin);
+                            unit_.depth_charge_task_installed = true;
+                            unit_.dc_target_plus_one = unit_.command_target_plus_one;
+                        }
+                        const bsp::DepthChargeArmResult r =
+                            bsp::depth_charge_task_arm_009a66c0(unit_.dc, host, host, dt);
+                        unit_.dc_transitions += static_cast<unsigned long long>(r.transitions);
+                        unit_.dc_releases += static_cast<unsigned long long>(r.releases);
+                        owner_.done("BotTaskDepthCharge::arm", 0x009a66c0u);
+                    }
+
                     void run_dive_bomb_task_arm_009c8790(float dt) {
                         // 0099A170 builds a task from the class 007EEC50
                         // chose, so the only correct test is that the class IS
@@ -29994,6 +30294,10 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             if constexpr (bsp::kStrafeTaskBound) {
                                 run_strafe_task_arm_009cd170(elapsed);
                             }
+                            if constexpr (bsp::kDepthChargeTaskBound) {
+                                // Packet cc13-depthcharge1: vtable[64h] 009A66C0.
+                                run_depth_charge_task_arm_009a66c0(elapsed);
+                            }
                             if constexpr (bsp::kPilotMoveToTaskBound) {
                                 run_moveto_task_install_009c3000();
                                 if constexpr (bsp::kMoveToTaskTickBound) {
```

## 8. Which missions it reaches

From docs/CC13_OMITTED_TASKS_SCOPE.md 2.3-2.4. The task needs a depth-charge-armed plane and a submarine target. By
default only the three float planes are armed: Kingfisher, Pete and Jake. **No mission on cc9's run list reaches it.**

- **USN21, "USN15 Battle of the Sibuyan Sea".** `usn_sibuyan.lua` (mtime 2024-10-29) is the one confirmed script
  order. `luaJakeTime` (`:721-739`) is scheduled every 60 s by `luaDelay(luaJakeTime, 60)` at `:701` and `:743`. In
  mission phase 1 it does three things:
  - it generates a `JakeTmpl` (set 2, armed);
  - it puts the Jake 1500 m east of a random live player submarine, at 500 m altitude (`luaPickRnd` is the script's
    own random, not stream 1);
  - it calls `PilotSetTarget(jake, subm)`.

  The player's primary objective is to sink Atago and Maya with the submarines, so this task is the threat to it.
  Today the chooser never sees `has_depth_charge_ordnance`, so these Jakes never fly an attack.
- **IJN12 Attack on the Panama Canal.** Three Kingfishers on the default (armed) set and Catalinas on set 2 share the
  scene with the player's I400, I501 and I601. No script order exists, so any attack would come from the AI's own
  targeting (not traced).
- **BSM11 Endgame at Midway.** Two Catalinas on set 2 share the scene with the Japanese I-25. The secondary objective
  is "Destroy the attacking Japanese submarine!".
- **Scene co-presence only:** BSM03, CHG07, CHG08, JM01, JM14, PRCP04, PRCP07, USN11 and IJN09.

Turning the switch ON changes nothing on cc9's current run list. The pair to run is USN21.

## 9. Uncertainty and open items

- **Not validated.** Build-tested only. No pair has run with the switch ON. The precision follows the listing (x87
  extended carried as double, float at each FSTP). The main thread's 24-bit control word can move the last bit
  (docs/CC13_FPU_PRECISION.md).
- **Slot `+20h` (`009A5780`).** It returns 0 while the target has `+5Ch` set, `+5Dh/+5Eh/+60h` clear, depth charges
  aboard and `+60h < CancelTargetDist`. Its caller (the base `007B40F0` slot) was not read. robots.lua's comment for
  DepthChargeCancelTargetDist: "if the target was closer than this when it vanished from sight (dived), we still try
  to drop the bomb blind". So the 0 probably means "keep attacking". Marked provisional.
- **`009A4900` and `009A3160`.** Neither has a code reference (`local/dc1_calls.py`: no rel32, no absolute dword).
  Both are named but not reconstructed.
- **The pull-out test** `+50h > 1.5` is signed, so it fires only when the target is more than 1.5 rad to one side. This
  is the listing's own `COMISS` (`009A4572`); it is not an abs.
- **The leader byte** `leader+9C1h + [00F876B8] * 8` (`009A4B0B`) is an array indexed by a word global. Neither has a
  known writer. The glue reads 0.
- **Host contracts left open:**
  - the follow base enter/exit/tick (`009BED80`, `009BDE40`, `009C1FD0`) and their use of `+8Ch`;
  - the moveto state;
  - `009FADA0`, `009FA260`, `009FB5E0`;
  - `007F0280` mode 1 (the glue uses the squadron candidates);
  - `009FD570` (the glue flies straight at the point);
  - `0099C230`, `0099B740`, `007ED260`.
- **Load path and presentation.** `009A6820`, `009A5000`, `009A35D0` and `009A69E0` (load path) and `009A5AF0`,
  `009A68C0` (debug draw and text) are named, not reconstructed.

**Corrections to earlier docs:**

- **docs/BOT_TASK_STATES.md `bot_approach_off`.** Four fields are misnamed:
  - `+2Ch` is the drop balance, not rounds remaining;
  - `+50h` is the relative bearing, not a "pull out metric";
  - `+58h` is half the stick length (`009A3290`), not "the constant 00CE38C8";
  - `+5Ch` is written by `009A5F50` (section 4).

  Its `flag_46C`/`flag_46D` are has-depth-charges and in-range.
- **docs/CC13_OMITTED_TASKS_SCOPE.md 2.1.** The RNG list misses `009A4646` (12 sites, not 11).
- **The tuning +670h label.** Rocket's header labels tuning `+670h` as the attack-run cone. The singleton names it
  Pilot/AutoStrafeAngle/Angle_MoveTo (game_tuning_singleton.hpp). The depth-charge attack run uses it, the goaway uses
  `+674h` Angle_GoAway and the gated prepare uses `+66Ch` Angle_Prepare.

## 10. Ledger and Ghidra

- **Names.** `bsp.py ledger add-name` recorded 39 new names. That is the window's unnamed work functions with a read
  body, plus `007B5350`, `009FB5E0` and `009A19B0` outside it. Evidence was appended to 9 existing names. No existing
  name was replaced.
- **Reconstruction records.** `bsp.py ledger add-function` recorded 33 functions, all `reconstructed_build_tested`,
  each with its coverage and the ABI from the listing's `RET`.
- **Ghidra.** `tools/ghidra_annotate.py --apply` ran on 42 addresses with functions. The first attempt stopped at
  `009A39F0` (no function), so the true old values of the addresses it had already applied are in
  `local/ghidra-annotations-20261012T103304Z.json`. The rest are in `...103324Z.json`.

  The following have no Ghidra function, so their names live in the ledger only: `009A39F0`, `009A3B70`, `009A3E70`
  (the leave, turnto and goaway ticks), `009A5F10`, `009A54B0` and `009A54C0`. No function was created or resized.
  'Non-Returning Functions - Discovered' was not touched.
- **Checks.**
  - `tools/verify_report_calls.py reports/cc13_depth_charge_task.json`: 101 call rows, 0 failed.
  - `tools/const_width_sweep.py include/bsp/depth_charge_task_cc13.hpp`: 11 constants, 0 mismatched.
  - `--all --load-sites` flags nothing in this header.

## 11. Handoff

- **Done.** The window's call spine is reconstructed and build-tested: construct, arm, approach update, rule, the
  nine states and the profile. The host glue diff passes `git apply --check`.
- **Next for cc9.** Apply section 7 with `kDepthChargeTaskBound` ON and run USN21. Watch three things:
  - the chooser log line (it should show `00e08f38` for the Jakes);
  - `BotTaskDepthCharge::arm`;
  - `dc_releases` against the submarines' damage.

  Then replace the labelled substitutions that matter on the run:
  - the fly-to solver in goaway;
  - the mode-1 probe candidates;
  - the `009A19B0` heading diff, which gates turnto -> aim (with 0 the gate is `0.5 > |+50h|`).
- **Unread, if a run needs them:** the caller of slot `+20h`, the follow base's use of `+8Ch`, and the leader byte
  `+9C1h`.
