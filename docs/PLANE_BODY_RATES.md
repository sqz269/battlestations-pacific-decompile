# Plane body rates against the AA turn-average gate

Packet `cc9_plane_body_rate_check`, worker cc9-ships2, on main `088e94d34`. Ghidra was read-only.
It follows docs/USN04_KATE_ATTRITION.md sections 11 to 13, where `kAaTargetTurnAverageBound` stayed
OFF because 88% of USN04's AA tests passed `00901C20`'s gate, `|w|^2 > 0.001` (`00D7A23C`), which
is |w| > 1.81 deg/s.

## 1. The image's rate law, and the host's

`007DA710 BSP_PlaneFlight_ControlRateLaw` (body `007DA710-007DAFCD`) is read in full for free
flight in docs/PLANE_CONTROL_RATE_LAW.md and docs/PLANE_CONTROL_TARGETS.md. Per axis (pitch
`ctl+48h`, yaw `+4Ch`, roll `+50h`):

```
delta = target - current
rate  = (A*delta^2 + B*|delta| + C) * |accel_axis|     ; A,B,C = Dynamics.RotationFactors
if (sign(delta) != sign(current)) rate = max(rate, 1.5*|current|)   ; 007DABEA..007DAC06
cand  = current + sign(delta) * rate * step
new   = clamp(cand, min(current, target), max(current, target))
```

- The accel terms are `f1*PitchAccel`, `-f2*YawAccel` and `f1*RollAccel`:

```
007DAA7F  FLD  [EAX+1C0h]     ; PitchAccel
007DAA93  FMUL ST1            ; D8 C9: ST0 *= f1
007DAA9D  FSTP [ESP+4Ch]      ; pops, ST0 is f1 again
007DAAB3  FMUL [EAX+1BCh]     ; f1 * RollAccel
```

- The targets are the class rate speeds times the latched stick (`unit+BB0h..+BB8h`), with the yaw
  slide and the yaw-roll coupling.
- **The clamp keeps a step from passing its target**, so the law itself cannot overshoot or hunt.
  An oscillating rate needs an oscillating target, that is, an oscillating stick.
- **Host:** `plane_control_axis_step_007da710` and `plane_control_targets_007da710`
  (`src/plane_control_rate.cpp`), called from `control_step_007da710` in
  `src/game_hosts_units.cpp`. It implements this law with `step` = the mission step, applied once
  per step. There is no dt-squared term and no missing damping term. The only unread arm is the
  ground arm (`007DA542`), which a flying Kate never takes.

## 2. The measurement

A diagnostic in `control_step_007da710` prints, for the one plane named by `BSP_PLANE_RATE_TRACE`,
the latched stick, the targets, the body rates and |w| in deg/s. It prints nothing when the variable
is unset.

The run is `local\br_trace_usn04.log`: USN04 4700/4500, streams and the death table on, lockstep
0.05, `BSP_PLANE_RATE_TRACE="B5N Kate #4.1|.-3"`, the one live torpedo releaser of section 13. It
gives 2203 steps, from spawn to about 110 s after spawn. The Kate dies at 130.20 s to
Fletcher-class05's category-1 gun at 355 m.

| flight phase (s after spawn) | stick range (pitch / roll) | body rates (pitch / roll), rad/s | \|w\| mean / max, deg/s | steps above 1.81 deg/s | axis sign changes per 5 s |
| --- | --- | --- | --- | --- | --- |
| 5-10, the turn onto course | 1.00 / 0.76 | 0.11 / 0.55 | 11.8 / 32.1 | 86 of 100 | 1 to 2 |
| 20-60, straight and level | 0.01 to 0.07 / 0.01 | about 0.02 / 0.01 | 0.2 to 1.3 / 1.3 | 40 of 800 (5%) | 0 to 24 on roll at 1e-3 rad/s (noise-level) |
| 80-110, the torpedo run | up to 1.00, then 0.03 to 0.28 / up to 0.22 | 0.02 to 0.30 / up to 0.18 | 2.0 to 5.6 / 19.7 | 87 to 99 of 100 | 0 to 2 |

**Results:**
- **The host's rates do not oscillate.** On the straight leg |w| stays below the gate for 95% of
  steps. The rates follow the stick, and the axis signs barely change. The 20 to 24 roll sign
  changes are about 1e-3 rad/s dithering around zero, far below the gate.
- **The gate passes on the torpedo run because the stick commands pitch.** While descending and
  holding the drop altitude, the pilot's pitch stick sits between 0.03 and 0.28, giving 1 to 7
  deg/s of pitch rate. The AA tests happen almost only there, which is why the pass rate is 88%.
  The 0.3 deg/s heading turn is a small part of |w|. `00901C20` squares the whole body vector,
  pitch and roll included.

## 3. Verdict, and what else could make the gate pass

**No host difference in the rate law, so no binding.** Given the same stick, the image produces the
same body rates.

What decides the pass rate is the **stick the pilot commands on the torpedo run**:
- the pitch channel of the bot's approach and altitude hold: the kind-7 approach `009BEBA0`, the
  torpedo attack task and the travel and attack altitude profile;
- the roll it adds to hold course.

If the image's pilot flies the run with less pitch activity, the image's gate passes less often.
That is the next check, in the pilot command path, not the rate law.

`00901C20` has no averaging window of its own: it reads the instantaneous body rate at
`unit+AF8h..+B00h`. So a smoother sample source cannot be the explanation.
