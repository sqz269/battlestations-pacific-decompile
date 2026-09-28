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

## 2. The pitch channel on the torpedo run (packet `cc9_torpedo_run_pitch_profile`)

### The chain, image and host

- **The task writes the pitch target.** On the Kate's run, the host's torpedo approach (the same
  `009C18C0` glide, `009FBA50` cruise altitude and `009FB800` pitch command the image uses) sets
  `plan+2BCh` from the altitude error:
  - it climbs or drops at the class `ClimbAngle`/`DropAngle`, scaled over
    `Pilot.General.ClimbDist`/`DropDist`;
  - it writes `plan+2D0h = 2` on every exit (`009FB93F`/`009FB95F`/`009FBA41`).
- **The planner** `0099D300` then turns that target into an elevator demand. `0099E490` does it,
  a proportional law with no deadband:
  `demand = (target - (measured - sin^2(bank)*cos(pitch)*SlideRatio*YawSpd*mul)) / (PitchSpd *
  (0.9*authority + 0.1) * cos(bank) * mul)`.
- `0099BC00` slews the slots. `007BB6E0` quantises the stick, and `007DA710` produces the rate
  (section 1).

**The one host difference is the mode-2 arm `0099DC9E`-`0099DD5A`, which the host never modelled.**

```
0099DC9E  MOV EAX,[ESI+2D0h]         ; pitch mode
0099DCB2  JZ  0099DD94                ; 0: no pitch path
0099DCC8  CALL EDX                    ; unit vtable+38h, the cached speed
0099DCD0  FCOMIP ; 0099DCD4 JBE       ; [00D1F3D8] 2.7778 <= speed stays mode 2
0099DCD6  MOV [ESI+2D0h],1            ; else demote to mode 1
0099DCE7  JNZ 0099DD54                ; mode 1: measured = [ESP+10h]
0099DCF1  FLD [ECX+0C84h]             ; mode 2: held = unit+C84h
0099DCFA  FSTP [ESP+14h]
0099DD2C  CALL 007C4810               ; min control speed
0099DD35  CALL 00419010               ; inc = interp(minCtl -> DEG(5), TravelSpeed(+18Ch) -> DEG(20), speed)
0099DD42  FADD [ESP+14h]              ; sum = inc + held
0099DD46  FLD  [ESP+10h]              ; the live pitch
0099DD4C  JBE 0099DD62                ; live <= sum: measured stays held
0099DD4E  FSTP [ESI+2BCh]             ; else target = sum
0099DD5A  MOV [ESP+14h],[ESP+10h]     ; and measured = live pitch
```

- **`[ESP+10h]` is the live pitch `unit+C64h`,** stored at `0099D4DC`. `tools/stack_frame_walk.py`
  reports depth 8 there and 12 at `0099DD46`. Its walk is linear and crosses the `0099DC75` jump
  back to `0099D8F7`. The host's comment at the demand, "the native's own else branch uses the live
  pitch", reads it the same way. This is labelled as the one inference in this section.
- **`unit+C84h` is not a held attitude.** `007C18B0`, run every tick from
  `BSP_PlaneTickElement_FixedStep`, writes it at `007C1D05`: `atan2` of the velocity at
  `+AE0h..+AE8h`, vertical over horizontal. That is the **flight-path angle**. `+C88h` is its rate.
- **So in mode 2 the image steers the flight path, not the nose.** The measured angle is the
  flight-path angle unless the nose is more than `inc` (5 to 20 degrees by speed) above it. Then
  the target is capped at flight path + `inc` and the nose is measured.
- **The host measures the nose** (`unit+C64h`) in every mode, with no cap.

**The binding.** `kPlanePitchModeTwoBound` in `src/game_hosts_units.cpp` (an `Impl` constant), OFF:
- at the `0099E490` call site, the demotion at 2.7778 m/s;
- the flight-path angle from `plane_world_velocity`;
- `inc` through `plane_min_control_speed_007c4810`, `dive_bomb_interpolate_clamped_00419010` and
  the class `TravelSpeed`;
- the cap and the measured angle;
- a counter, in a new summary line: `summary unit plane pitch mode two`.

**Labelled:**
- The speed is the magnitude of `plane_world_velocity`, not `vtable[38h]`'s cached value.
- `+C84h` is recomputed at the demand, not in `007C18B0`, which gives the same angle each step.

The rate trace now also prints the pitch, the measured angle, `plan+2BCh`, the mode, the altitude
and the commanded altitude.

### Predictions, before any run (OFF: this tree's `build\` at the commit; ON: an export with the switch flipped)

- **USN04 4700/4500, Kate `#4.1|.-3` traced (`BSP_PLANE_RATE_TRACE`):**
  - Every travelling plane runs mode 2 (`009FB800` sets it), so the counter is large on ON and 0 on
    OFF.
  - A cruising plane's nose sits a few degrees above its flight path. With the flight path measured
    and the nose capped at flight path + `inc`, the elevator stops fighting the angle-of-attack
    offset.
  - Expected: the Kate's pitch stick on the run is lower and steadier; |w| on the run is lower; the
    share of run steps above 1.81 deg/s falls from about 90% (moderate confidence in the direction,
    none in the size).
  - Planes hold altitude closer to the command.
  - Gameplay moves (exit 3). Kate death times shift within the RNG coupling. Deaths stay within
    44 +- 3, and torpedo drops stay 0 to 2.
- **USN01 3200/3000:** it has planes (the Mavises and the ScoutDauntless pair), so it moves (exit 3).
  Deaths stay within 7 +- 1.
- **USN02 9200/9000:** it has no aircraft, so it is identical (exit 0 or 1).

### The pairs, and the flip

- **Commit:** OFF is `b2e64ab63`, this tree's `build\`.
- **ON:** `tools/pair_export.py --commit b2e64ab63 --flip kPlanePitchModeTwoBound=true` into
  `local\m2_on`.
- **Logs:** `local\m2_{off,on}_{usn04,usn01,usn02}.log`, with streams, the death table and
  `BSP_PLANE_RATE_TRACE` on the Kate. The 300-frame smoke `local\m2_smoke300.log` passed first.

| mission | pair_diff | mode-2 steps ON | what moved |
| --- | --- | --- | --- |
| USN04 4700/4500 | exit 3 | 70871 | Deaths 44 to 41 (three Zeros survive), hit records 707 to 670, shots 5433 to 4461, water contacts 16 to 12. Releases 4 of 16 / 4 of 19 and drops 0 are unchanged |
| USN01 3200/3000 | exit 3 | 5096 | Deaths 7 unchanged, hit records 149 to 148, shots 557 to 561 |
| USN02 9200/9000 | exit 1 | 0 | nothing (no aircraft) |

**The Kate (`#4.1|.-3`), OFF and ON:**

| window | \|w\| mean, deg/s | steps above 1.81 | \|pitch stick\| mean | \|alt - commanded\| mean | pitch - measured mean |
| --- | --- | --- | --- | --- | --- |
| straight, 20-60 s | 0.75 / 1.29 | 5% / 14% | 0.037 / 0.036 | 21.7 / 21.9 m | -0.0001 / -0.0010 rad |
| the run, last 600 steps | 4.00 / 4.07 | 75% / 74% | 0.139 / 0.187 | 462.7 / 452.5 m | 0.0002 / 0.0024 rad |

**Failed prediction.** "The Kate's run pitch activity falls" did not hold: |w| and the share above
the gate are unchanged within noise.
- The reason is in the last column. This host's plane flies with the nose almost exactly on the
  flight path (under 0.003 rad apart), so measuring the flight-path angle instead of the nose
  changes almost nothing for a Kate.
- The rule moves other aircraft: 70871 mode-2 steps, and USN04's Zeros live longer.
- The run's pitch activity is the **descent itself**. The Kate is on average 450 m above its
  commanded altitude over the last 600 steps, and `009FB800` pitches it down at the class
  `DropAngle` and levels it out at the drop height. That is the same law in the image.

**Decision: ON.** The arm is the image's, and USN02 held.

**Answer to the packet's question.**
- The image flies the torpedo run with the same pitch law as the host now does. The pitch activity
  that passes `00901C20`'s gate is the descent from cruise to drop height, which the image also
  commands.
- What the host still lacks is an angle of attack. Its plane flies with the nose on the flight
  path, so the image's version of the run may differ in how the nose moves, but not in the commanded
  descent.
- **On this evidence `kAaTargetTurnAverageBound` belongs ON, with the image's own consequence: a
  descending Kate is a turning target to the AA director and is shot down before release more
  often.** The ruling is cc9-gunnery3's.
