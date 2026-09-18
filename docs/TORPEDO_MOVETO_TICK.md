# The move-to tick commands a glide slope, and that is the descent the run was missing

Addresses: 009C18C0, 009C196C, 009C1999, 009C1A30, 009C1AF3, 009C1B17, 009C1B23, 009C1B2D,
009C1B45, 009C2AC0,
009C2980, 009BDE80, 009FBA50, 009FBAD2, 009F9E40, 007C4850, 007C4A27, 007C4A37, 007C4A3F,
007C4A44, 00412E20, 009D4A70, 0099B630, 009A1A20, 009FABE0, 00CE7638, 00CF6560, 00CE7804,
00D06BB4, 00D1F8D0, 00CE3938, 00CEB4D4, 00CE47A0, 00CFFD60, 00CF0DD8.

Packet `cc8_torpedo_moveto_tick`, owner `agent/cc8-torpedo-run-in`, on `75f1b6570`.

Reading only: the two files the wiring needs are held elsewhere. The wiring contract is section 5.

## 1. The headline: it is not a cruise-altitude command

The premise this packet was given is that `009C18C0` would command `Pilot/Torpedo/CruisingAlt`,
authored 500. **It does not.** `Pilot/Torpedo/CruisingAlt` goes somewhere else entirely -
`bot_task_update_cruise_profile` writes it into the pilot control block's cruise channel at
`ctl+380h` (`include/bsp/bot_tasks.hpp:69`), and no part of this tick reads it.

What `009C18C0` commands is a **glide slope**:

```
altitude = aimAlt + targetY + max(distance - task+438h, 0) * t * tan(desc+1F0h DropAngle)
```

which is the descent the whole torpedo run has been missing. An aircraft far from its target is
held high; the commanded altitude falls linearly with the distance it still has to close, on a
slope set by its own authored dive angle; and it arrives at the attack run already near the release
altitude, which is why the attack run's own 12 m command is a small final correction in the image
and a 60-degree plunge in this host.

## 2. `009C18C0` to its end

`__thiscall(this, float dt)`, Ghidra body `009C18C0`-`009C1BB3`. `this+4h` is the approach,
`this+2Ch` the target, and `this+30h`/`+34h`/`+38h` the three values section 3 covers.

| step | rule |
| --- | --- |
| 1 | with a target at `+2Ch`, refresh both poses and take the planar separation; at or below the double `[00CE3820]` the speed is `0`, else its square root. The result stays in `EBX`. |
| 2 | `this->vtable[+1Ch](speed)` - the speed slot `009C1850` |
| 3 | when `unit+0C25h != 0`: write `cmd+2BCh = 0`, `cmd+2D0h = 2`, `cmd+2C4h = 0`, `cmd+2CCh = 1` and return |
| 4 | when `+2Ch == 0` (no target): the same four writes in the other order, plus `approach+10h -> +7Ch = 1` when `approach+10h` is non-null, and return |
| 5 | otherwise the glide slope, the heading, the strafe angle and the three tail calls |

Step 5 in full:

```
base   = max(this+34h + targetY, this+30h)
margin = 1400.0 [00D1F8D0] - unit+100h ; if margin < 50.0 [00CE3938] then margin = 50.0 [00CEB4D4]
denom  = EBX - 1000.0 [00CE47A0] ; if denom < 50.0 then denom = 50.0 ; if denom >= 2000.0 [00CF0DD8]
                                   then denom = 2000.0 [00CFFD60]
t      = InterpolateClamped(0.05 [00CE7638], 0.35 [00CF6560], 0.4 [00CE7804], 1.6 [00D06BB4],
                            margin / denom)
009c1af3  CALL 00419010                 ; t
009c1b17  CALL 009fba50(base, this+38h, EBX, t)   ; rangeLow = this+38h, rangeHigh = the distance
009c1b23  CALL 009f9e40(&delta)         ; the heading to the target point
009c1b2d  FLD [EAX+0x670]               ; approach+1Ch -> +40h = Pilot/AutoStrafeAngle/Angle_MoveTo
009c1b45  CALL 0099b630 -> 009c1b50 009a1a20 -> 009c1b5b 009fabe0   ; the three tail calls, unread
```

Every constant was checked against the data section: `00CE7638` = `0.05f`, `00CF6560` = `0.35f`,
`00CE7804` = `0.4f`, `00D06BB4` = `1.6f`, and the doubles `00D1F8D0` = `1400.0`, `00CE3938` = `50.0`,
`00CE47A0` = `1000.0`, `00CF0DD8` = `2000.0`, with `00CEB4D4` = `50.0f` and `00CFFD60` = `2000.0f`
as their float twins.

**The range pair here is real**, which is the difference from the attack-run tick: `rangeLow` is
`this+38h` and `rangeHigh` is the live distance, so `span = max(distance - this+38h, 0)` is
positive for the whole approach and `009FBA50`'s `009FBAD2` term is live.

`unit+9D8h` selects which state the task starts in rather than which arm this tick takes:
`bot_task_initial_state` reads `007B8AD0`'s `unit+9D8h == 0` and picks `moveto` or `follow`
(`src/bot_tasks.cpp:114-119`). Both states share this body (`00D20AEC+0Ch` and `00D20B24+0Ch`), and
the branch inside it is on the target at `+2Ch`, not on `unit+9D8h`.

## 3. `this+30h`, `+34h`, `+38h`, and the gain

`009C2AC0(this, approach, target, near, far, mode)` stores the last three at `+30h`, `+34h` and
`+38h`, and `009BDE80(state, a, b, c)` is a three-store setter that refreshes them. Its torpedo
call site passes `(approach+3Ch + approach+34h, approach+3Ch + approach+34h, task+438h)`
(`docs/BOT_TASK_STATES.md` line 160), so:

* `+30h` and `+34h` both carry `approach+3Ch + approach+34h`, the task's **aim altitude**, which
  `docs/BOT_TASK_STATES.md` line 262 identifies for the depth charge as a draw between
  `Pilot/DepthCharge/AimAltRange/1` = 20 and `/2` = 60. Because both slots hold the same value,
  `base = max(aimAlt + targetY, aimAlt)` = `aimAlt + targetY` for any target at or above sea level.
* `+38h` carries `task+438h`, the distance the glide aims to reach the aim altitude at. The
  tuning singleton's `Pilot/Torpedo/SafeDist` at the same numeric offset `0x438` is authored **700**
  (`include/bsp/bot_tasks.hpp:265`), which is the right magnitude next to `TorpReleaseDistNear` 450
  and `TorpReleaseDistFar` 650, but the task record's `+438h` and the singleton's `+438h` are
  different objects and this packet did **not** establish the task field's producer.

**The gain is `tan(DropAngle)`.** `009FBA50`'s `class+518h` is derived at class load:

```
007c4a27  FLD  float ptr [ESI + 0x1f0]   ; desc+1F0h DropAngle
007c4a34  FSTP float ptr [ESP]
007c4a3f  CALL 00412e20                  ; BSP_Math_TangentX87Float
007c4a44  FSTP float ptr [ESI + 0x518]
```

and `00412E20` is `FLD` / `FSINCOS` / `FDIVP`, i.e. `sin/cos`, a tangent. `007C4A62` derives
`class+51Ch` the same way from `DropAngle * [00CEFFA0]`. So `span * t * class+518h` is
`horizontalDistance * tan(angle)` - a height above a straight glide path at
`atan(t * tan(DropAngle))`, which for the TBD Devastator's `DropAngle` of 0.610865 rad is between
**8.5 and 34 degrees** as `t` runs its `[0.35, 1.6]` range.

## 4. What the profile does, in numbers

For a Devastator against a ship at sea level, with the aim altitude written `A` and `Dynamics/Ceiling`
1500 so `009FBA50` clamps at 1450:

| distance to target | `span` | commanded altitude at `t = 1.0` |
| --- | --- | --- |
| 5000 m | 4300 | 1450 m (the ceiling clamp binds) |
| 2770 m | 2070 | 1450 m (the clamp stops binding here) |
| 2000 m | 1300 | `A` + 910 m |
| 1500 m | 800 | `A` + 560 m |
| 1000 m | 300 | `A` + 210 m |
| 800 m | 100 | `A` + 70 m |
| 700 m | 0 | `A` |

USN01's ordered aircraft start **1488 m** from their targets on the current tree, so the glide would
command about `A` + 550 m and walk them down from there. They would reach `attackrun` at a few tens
of metres, and the attack run's 12 m command would then be an error below the roughly 80 m at which
`009FB800` saturates - a gentle correction rather than the `DEG(60)` plunge
`docs/TORPEDO_RUN_IN_DESCENT.md` and `docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md` both measured.

## 5. The wiring contract

The two files this needs, `src/bot_task_states.cpp` and `src/game_hosts_units.cpp`, are held
elsewhere, so nothing is wired here. What a wiring needs, in order:

1. **Run the tick where the native runs it.** The state machine's per-state tick already runs for
   `aim` and `attackrun` in `src/game_hosts_units.cpp`; `moveto` and `follow` need the same
   treatment, calling a reconstruction of `009C18C0` step 5.
2. **`class+518h` is free.** `tan(desc+1F0h DropAngle)`, and `DropAngle` is already loaded onto the
   slot by `cc8_plane_pose_throttle_altitude`.
3. **`this+38h` is a labelled substitution** until the task field's producer is read. The authored
   `Pilot/Torpedo/SafeDist` 700 is the value to stand in with, named as such.
4. **The aim altitude `approach+3Ch + approach+34h` is a labelled substitution.** For the torpedo
   task its producer is unread; the depth charge's twin is a tuning draw. The torpedo release
   altitude `approach+74h + approach+78h` = 12 m is the natural stand-in and is already computed on
   the slot, but it is **not** the same field and must not be presented as one.
5. `009FBA50` and `009F9E40` are already reconstructed (`cruise_altitude_command_009fba50`,
   `heading_command_009f9e40`), and `command_altitude_and_throttle` in the units host already drives
   the first with a real base; it needs the range pair and the scale rather than the zeros the
   attack-run site correctly passes.

## 6. The two open questions

**Does any other `009FBA50` call site pass a real range pair? Yes - this one.**
`docs/TORPEDO_THROTTLE_CUT.md` established that the attack-run site passes `approach+90h` as both
arguments, so `span` is zero and the `scale` term is dead there. Here `rangeLow` is `this+38h` and
`rangeHigh` the live distance, so the term is live and the interpolation that feeds it is the glide
slope's shape. The attack-run tick's dead interpolation is the same computation used by a site where
it matters; `009A3770` and `009A4010` remain unread.

**What sets a bot plane's throttle? Still nothing, in this chain.** The plan's throttle slot is
index 0 of the five (`include/bsp/pilot_plan_slots.hpp:39`), it reaches `unit+9F0h` only through
`007BB6E0`'s `cmd+14h == 0` path at `007BB83A`, and `0099B450` reseeds it from the live value every
think. This tick writes no throttle either: its step 2 hands a **speed** to `this->vtable[+1Ch]`,
the `009C1850` slot, which is not the throttle axis. An image-wide census would need the plan
record's byte offset for slot 0's `desired`, which this packet did not establish. Labelled open.

## ABI

* `009C18C0` `BSP_BotStateMoveTo_Tick`, `void __thiscall(this, float dt)`, Ghidra body
  `009C18C0`-`009C1BB3`. The decompiler shows the distance as an `unaff_EBX` float; it is computed
  in step 1 (the square root at `009C196C`) and held in `EBX` to the `009FBA50` call.
* `009BDE80`, `void __thiscall(state, float a, float b, float c)`, three stores into `+30h`, `+34h`
  and `+38h` and nothing else.
* `00412E20` `BSP_Math_TangentX87Float`, `float(float)`, `RET 4`, body `00412E20`-`00412E30`.

## Uncertainty

* `approach+3Ch` and `approach+34h`, the aim altitude, for the **torpedo** task. The depth charge's
  producer is a tuning draw; the torpedo's is unread.
* `task+438h`. The numeric coincidence with `Pilot/Torpedo/SafeDist` 700 is suggestive and is not
  evidence: they are offsets into different objects.
* The three tail calls `0099B630` (009C1B45), `009A1A20` (009C1B50) and `009FABE0` (009C1B5B), and the `this->vtable[+1Ch]` speed slot
  `009C1850`'s body.
* Which tick runs `009BDE80`'s refresh, and how often.
* `approach+1Ch -> +40h = tuning+670h`: `Pilot/AutoStrafeAngle/Angle_MoveTo` is an angle, and its
  reader is unread.

## Host methods

**None.** Reading only; the wiring contract is section 5.

## Corrections

Appended to the docs they amend, and verified present there.

* `docs/BOT_TASK_STATES.md` "The shared `moveto` and `follow` states". Its constructor line names
  `+30h`/`+34h` "near/far" and `+38h` "mode"; the tick uses `+30h` and `+34h` as altitudes and
  `+38h` as the distance `009FBA50`'s `rangeLow` takes, and its step table stops before step 5.
* `docs/TORPEDO_THROTTLE_CUT.md`'s first open question, answered here.
* `docs/PLANE_FLIGHT.md`'s `009FBA50` section, which records `class+518h` as a per-class gain
  without a producer. It is `tan(desc+1F0h DropAngle)`, derived at `007C4A44`.

## no_ghidra_function

None. `009C18C0`, `009BDE80`, `009C2AC0`, `009C2980`, `00412E20` and `007C4850` all have Ghidra
functions.

## Validation

No run: this packet changes no behaviour, and the two files a wiring needs are held elsewhere. The
numbers section 4 predicts are against `docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md`'s measured
baseline, where the ordered aircraft start 1488 m from their targets and enter `attackrun` at their
800 m spawn altitude.

## Follow-up packets

1. **Wire the tick** once the files free, to section 5's contract, and measure.
2. **`approach+3Ch`/`+34h` for the torpedo task**, which would retire the aim-altitude
   substitution.
3. **`task+438h`'s producer.**
4. **The plan throttle slot's writers**, image-wide, which needs the plan record's slot-0 offset.
