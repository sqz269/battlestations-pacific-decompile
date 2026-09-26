# The free-flight law below StallSpd: the mush is the image's own regime (packet cc9_freeflight_stall_law)

2026-09-25. Read-only on Ghidra. Names are hypotheses. The question was whether the host's
58.37-58.40 m/s mushing fall (docs/CLIMBOUT_SPEED_GATE.md) comes from a host divergence in the
free-flight law, term by term. It does not. Every term that acts in that regime is the image's.
The one substitution, the host's velocity integration, differs in form but does not change this
regime. `kDogfightThrottleBound` stays OFF pending an original-game check.

## 1. The regime

The two falls on record:

- **An A6M Zero,** `local\CSGT_9000.log` at 86 s: alt 4.7 m, vy -57.4, speed 57.6, pitch -0.09,
  `up_y` -0.64 (inverted), throttle 0.008.
- **The F4F sqn03,** `local\THR1_9000.log` at 339 s: vy -34.6, speed 57.7, pitch +0.79, throttle 1.0.

Both aircraft move mostly across their own nose, so the forward (body z) speed is small, far below
`StallSpd` 17.5 x `LevelFlight` 1.8.

## 2. Term by term

| term | image | class fields | host | verdict |
| --- | --- | --- | --- | --- |
| lift `dyn+20h` | `007DB875`-`007DB98D`: `(1 + aoa) * qq`, `aoa = -vy/vz` (0 when `|vz| < 0.1`), `qq = min(q^2, 1)`, `q = forward / StallSpd / LevelFlight`; clamp to +-2; times AccelCheatMul 1.5, capped at `ctl+90h`; times 9.81. No post-stall drop: lift falls with `q^2` only | `desc+184h` StallSpd | `lift_accel_007db875` (`src/plane_flight.cpp`), line for line | **image's law**. In the regime `q` is about 0, so lift is about 0 |
| gravity `dyn+2Ch` | `007DB990`-`007DBA2F`: `-1.5 * 9.81 * interp(0 -> 1, LostDragTime -> ExtraGravityMul; unit+C3Ch)` | - | `gravity_accel_007db990` | **image's law** |
| thrust `dyn+24h` | `007DB744`-`007DB80A`, `007D9050`: `Accel * throttle`, the fall cheat when the nose is down, clamped [0, 100], times `unit+CC8h` and `008E6430(6)` | `desc+164h` Accel (x AccelCheatMulMul x AccelCheatMul at `007D20F3`) | the same rule; Turbo, boost, `unit+CC8h` and `008E6430` taken as 1.0 (labelled partial before this packet) | **image's law** for a healthy, non-boosted plane |
| drag `dyn+10h` | `007DBA32`-`007DBC76`, `007D9140` (decoded in full here, section 3) | `desc+50Ch` = Accel / MaxSpd^2, `+208h` GlideRate, `+1D4h` DragPitchRatio, `+1DCh` AirBrakeDrag, `+188h` MaxSpd | the same formula (`src/game_hosts_units.cpp`, the drag block) | **image's law**; one argument source differs, section 3 |
| body damping `dyn+04h`/`08h` | `007DBD37`-`007DBE0D`: `007D92B0(forward / StallSpd)` times `-v_body * XDrag/YDrag`; vertical clamp +-`interp(0.5 -> 1, 20 -> 100; unit+908h)`. `007D92B0` returns 0 when the ratio is at or below DragRangeMin | `+170h` YDrag, `+174h` XDrag | `aero_response_curve_007d92b0`, same clamp | **image's law**. In the regime the ratio is under DragRangeMin, so damping is 0 |
| rotation `007DA710` | rates toward the pilot's targets times `007D9A70` authority, which is at least 0.25 in the air (`A = interp(3 -> 0, 6 -> 0.25; unit+908h)`). **No aerodynamic pitching moment and no nose drop** | `+1C0h` PitchAccel and the rate fields | `plane_control_targets_007da710`, `plane_control_axis_step_007da710` | **image's law** |
| velocity integration | `007D8470` `BSP_PlaneDynamics_IntegrateStep`, section 4 | - | explicit Euler: `v += a * step` (`src/game_hosts_units.cpp`, labelled "the host's, not native") | **substitution**, equivalent in this regime (section 4) |

So in the image, an aircraft moving across its own nose at a small forward speed has no lift and no
damping. Only the drag along the velocity holds it, and its speed settles where drag balances
gravity net of thrust. Nothing restores the nose: the rate law turns the airframe only toward the
pilot's targets. A pilot commanding nose-up, like aim chasing a target above, keeps it there.

**The shared terminal speed is consistent with this.** The F4F (class 101) and the A6M Zero (class
150) share Accel 7, MaxSpd 83.33, GlideRate 1 and StallSpd 17.5 in this installation's
`vehicleclasses.lua`, so they have the same drag coefficient. The balance is `v^2 * K * Accel_eff /
MaxSpd^2 = g_net`, with `K = (1 - throttle)^2 * GlideRate + 1`. For the F4F at full throttle and 45
degrees nose-up, `g_net` is gravity less the thrust's vertical share, 14.7 - 12.1 x sin 0.79, about
6.2 m/s^2. That gives about 60 m/s against the measured 57.7. This is an order-of-magnitude
consistency check, not a derivation: the Zero's inverted attitude changes its own `g_net`.

## 3. `007D9140` `BSP_PlaneFlight_DragAccel`, read in full

`__fastcall(ECX = desc)` with five stack floats, `RET 14h`. The arguments are pushed at
`007DBB37`-`007DBB5B`:
- a1 is the speed;
- a2 is `[unit+9E4h block]+0Ch`, the command throttle `unit+9F0h`;
- a3 is the pitch;
- a4 is `[unit+BB0h block]+10h`, the latched air brake;
- a5 is `[unit+BB0h block]+4h`, the latched pitch control.

```
007D9141-007D9183  c = clamp(1 - a2, 0, 1)
007D9189-007D919E  t = c^2 * desc+208h
007D91BD-007D91D2  K = (t + 1 + desc+1D4h * |a5|) * desc+50Ch
007D91E0-007D91EC  B = 1 + desc+1DCh * a4 ;  KB = K * B
007D91F4-007D923D  floor = InterpolateClamped(0, MaxSpd * [00F873F4], [00F873F8], MaxSpd * [00F873F0], a3)
007D9246-007D9266  v = max(a1, floor)
007D926C-007D9299  return -sgn(v) * v * v * KB
```

`00F873F0`/`F4`/`F8` are the tuning mirror's MaxDragSpdMul, MinDragSpdMul and MaxDragPitch
(tuning `+310h`/`+314h`/`+318h`). The host's formula is this one.

**One argument source differs.** The host passes the latched throttle `unit+BBCh`, where the image
passes the command throttle `unit+9F0h`. `007B9770` latches the second into the first, so they
differ by at most the latch's timing. With the throttle at 1.0 or near 0 in both falls, `c` is the
same in either case. This is recorded, not bound.

## 4. `007D8470`'s integration tail, read for the free-flight path

Read here: `007D8602`-`007D8E28`. The timed-hold arm (`dyn+C0h > 0`, docs/PLANE_DYN_TIMED_HOLD.md)
and `007D8E28`-`007D904E` are not re-read.

- **The folds.** Three world-to-body folds come first (`007D8487`, `007D84B2`, `007D84DE`):
  `dyn+28h` into `+1Ch`, `+10h` into `+04h`, and `+34h` into `+40h`. Free flight leaves `+34h`
  empty. Then comes the 0.001 deadband on all three (`007D8502`-`007D85E7`).
- **The previous body velocity.** `dyn+70h = dyn+64h` (`007D85EC`-`007D8608`).
- **A friction-style step, per body axis** (`007D8611`-`007D874F`):
  - `w = v + A*step`, where `A = dyn+1Ch` (lift, thrust, gravity), `b = B*step` with
    `B = dyn+04h` (damping and drag), and `c = |C|*step` with `C = dyn+40h`;
  - for `w >= 0`: `v' = max(0, w + min(b, 0) - c)`;
  - for `w < 0`: `v' = min(0, w + max(b, 0) + c)`.
  So the resisting terms act only against the motion and can stop a component but never reverse it.
- **The tail of the velocity step:**
  - `v += dyn+88h..90h` (`007D8755`-`007D8774`);
  - components under 0.01 (`00D7A238`) are zeroed (`007D8777`-`007D87C2`);
  - while the takeoff timer `dyn+C8h > 0`, the forward speed is raised to at least `dyn+CCh`
    (`007D87C7`-`007D87F1`);
  - with `dyn+A4h` set or `dyn+94h > 0`, body `vy` is zeroed when `|vz| < 1e-5` (`007D87F6`-`007D8838`).
- **No hold.** The body velocity is rotated into world `dyn+4Ch` (`007D8C6B`-`007D8CE6`). Then, only
  with byte `dyn+C4h` (the latched `unit+904h`) set and world `vy >= -0.001`, a sink term runs: for
  speeds above 3.0, body `vy -= 0.1 * speed`, and the vector is renormalised to the same speed
  (`007D8CE9`-`007D8E15`). `unit+904h` is cleared on an airborne spawn (`007C648B`) and by
  `BSP_Plane_BeginFlying` (`007C719E`), so the sink term does not run in free flight.

**Why the substitution does not change the mush.**
- The friction form differs from Euler only when a resisting term would reverse a component within
  one step, `|B*step| > |w|`. In the fall, damping is 0 and drag is about 15 m/s^2 against about
  58 m/s of speed.
- The 0.01 deadband is below anything measured.
- `dyn+C8h`, `dyn+C4h` and the takeoff flags are off in free flight.
- `dyn+88h` has no writer on this path that the host models. Its producer is not read here.

## 5. Verdict

- **No divergent term acts in the mushing fall.** Nothing is bound, and there is no pair.
- **The mush is the image's own regime** as far as the listing reaches. There is no post-stall lift
  model, no nose-drop moment, and no minimum-speed guard. A fighter the pilot holds nose-up at low
  forward speed falls at the drag-limited speed.
- **`kDogfightThrottleBound` stays OFF** pending a check on the original game. With it ON, the
  head-on direct throttle is what slows a fighter into that regime in the first place (THR1).
- **Follow-up, a separate packet.** Bind `007D8470`'s friction step and the 0.01 deadband under a
  switch. This is a general flight change measured by an E2 pair, not a stall fix. First read
  `dyn+88h`'s producer and `007D8E28`-`007D904E`.
