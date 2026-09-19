# The torpedo descent law, and the x87 stack error that turned it into a plunge

Packet `cc8_torpedo_descent_law`, owner `agent/cc8-plane-squadron`.

Addresses: `009FB800` (the pitch command), `009FB849`, `009FB858`, `009FB86C`, `009FB87C`,
`009FB8CC`, `009FB8F1`, `009FB96E`, `009FB97F`, `009FB997`, `009FB9B8`, `009FB9BA`, `009FB9DF`,
`009FBA06`, `009FBA1A`, `009FBA2D`, `009FBA41`; `009FBA50` (the cruise-altitude command),
`009FBA56`, `009FBA6A`, `009FBA8A`, `009FBAB1`, `009FBAC5`, `009FBAC9`, `009FBAD9`, `009FBADB`,
`009FBAE5`, `009FBAF1`, `009FBAF3`, `009FBB06`, `009FBB0C`, `009FBB13`; `009D07B0` (the torpedo
attack-run tick), `009D07E4`, `009D0951`, `009D0957`, `009D096D`, `009D09B0`, `009D09D6`,
`009D09F6`, `009D09FC`, `009D0A0E`, `009D0A26`, `009D0A63`, `009D0A68`, `009D0A81`, `009D0A92`;
`00419010` (the clamped interpolation), `009C1AF3`/`009C1B17` (the move-to tick's own call),
`009C43CD`/`009C4401` (the dive bomb's), `007C4A3F`/`007C4A44` (`class+518h`).

Constants: `00D7A208`, `00D7A210`, `00D7A218`, `00D7A280`, `00D7A2F0`, `00CF6560`, `00CE7804`,
`00CE74F8`, `00CE3938`, `00CE3D48`, `00CE3D90`, `00CEB4D4`, `00CF0DD8`, `00CF3F20`, `00CFD710`,
`00CFFD60`, `00D05AAC`, `00CE7D20`, `00CE7638`, `00D06BB4`, `00D1F8D0`.

**The finding.** USN01's five torpedo bombers do not plunge in the image. They plunge in this
host, and the cause is a single x87 stack error: `009FB800`'s second argument is `009FBA50`'s
**fourth argument**, a dimensionless ramp in `[0.35, 0.8]`, and the host was passing it the
**commanded altitude in metres**. That argument is the clamp on `t`, so 12.0 in place of 0.5 let
`DropAngle * t` reach 4.82 rad and saturate the `DEG(60)` cap at every altitude error over about
80 m - the constant `-1.0472 rad` the descent census printed from 800 m down to the water, with no
flare. `docs/ATTACK_RUN_DESCENT.md`'s "the host is faithful to the native's own chain, constant for
constant" is refuted, and the retraction is in section 6.

## 1. The image's descent law, as an equation with every constant sourced

For the torpedo attack run, per tick, with `y` the unit's world altitude (`unit+100h`) and `R` the
planar range to the target (`approach+90h`):

```
margin    = clamp(1400 - y, 50, 400)                                   009D0957, 009D096D, 009D09B0
denom     = min(R, 2000)                                               009D09D6
s         = Interp(0.1, 0.35, 0.4, 0.8, margin / denom)   in [0.35,0.8]  009D0A26, 009D0A63
low       = elapsed_134 >= 15 ? approach+7Ch : approach+80h            009D0A0E
base      = approach+74h + approach+78h                                009D0A81
gain      = tan(desc+1F0h DropAngle)              [class+518h]         007C4A3F, 007C4A44

A_raw     = base + max(R - low, 0) * s * gain                          009FBAD2-009FBAE5
cap       = min(Ceiling - 50, ctl+394h when present)                   009FBA8A,009FBAB1
A         = min(A_raw, cap)                                            009FBAF1-009FBAF7

err       = min(A, Ceiling - 50) - y                                   009FB849
x         = err * ((s + 1) * 0.5)                                      009FB858-009FB86C
x <= 0:   limit = max(DropAngle * 1.6, DEG(60))                        009FB97F, 009FB997
          t     = clamp(-x / DropDist, 0, s)                           009FB9BA, 009FB9DF
          plan+2BCh = -min(DropAngle * t, limit)                       009FBA06, 009FBA1A, 009FBA2D
x >  0:   limit = max(class+1ECh * 1.6, DEG(40))                       009FB893, 009FB8AB
          t     = clamp(x / ClimbDist, 0, s)                           009FB8CC, 009FB8F1
          plan+2BCh = min(class+1ECh * t, limit)                       009FB918, 009FB92C
plan+2D0h = 2                                                          009FBA41 / 009FB93F / 009FB95F
```

| constant | address | value | role |
| --- | --- | --- | --- |
| margin ceiling | `00D1F8D0`, a double | 1400 | the altitude the margin is measured down from |
| margin floor | `00CEB4D4` (gate `00CE3938`) | 50 | `009D096D`'s `JBE` is taken when `50 <= margin` |
| margin cap | `00CFD710` (gate `00CE3D90`) | 400 | `009D09B0`'s `JBE` is taken when `margin <= 400` |
| denom cap | `00CFFD60` (gate `00CF0DD8`) | 2000 | `009D09D6`'s `JBE` is taken when `2000 <= R` |
| the ramp | `00D7A2F0`, `00CF6560`, `00CE7804`, `00CE74F8` | 0.1, 0.35, 0.4, 0.8 | `x0, y0, x1, y1` of `00419010` |
| the release-distance switch | `00CF3F20`, a double | 15 | `009D0A0E`'s `JBE` is taken when `15 <= elapsed_134` |
| `approach+7Ch` / `+80h` | authored | `TorpReleaseDistNear` 450 / `Far` 650, times `max(1, MaxSpd / Pilot/Torpedo/ReferenceSpeed)` | the release distance |
| `approach+78h` | authored | `TorpReleaseAlt` 12 (`SPNormal`) | the release altitude |
| `approach+74h` | `009D3489` | 0 here | the altitude floor; `ctl+398h` is above the `100.0` gate |
| the span gate | `00D7A218` | 0.0f | `009FBAC9 COMISS` skips the bias when `span <= 0` |
| the ceiling margin | `00CE3938`, a double | 50 | `009FBA8A` and `009FB80F` |
| `(s + 1)` | `00D7A210`, a double | 1.0 | `009FB860 FADD` |
| `* 0.5` | `00D7A280`, a double | 0.5 | `009FB866 FMUL` |
| the negation | `00D7A208` | -0.0f | `009FBA2D MOVSS` / `009FBA35 SUBSS` |
| `DropDist` | `tuning+548h`, `Pilot/General/DropDist` | 200 | `009FB9BA`. The climb arm uses `+544h` `ClimbDist`, 130 |
| the cap scale | `00CE3D48`, a double | 1.6 | `009FB97F` |
| `DEG(60)` | `00D05AAC` | 1.0471976 | the dive cap |
| `DEG(40)` | `00CE7D20` | 0.6981317 | the climb cap |
| `DropAngle` | `desc+1F0h`, authored | 0.4014 (Mav) | both the gain and the cap of the dive arm |
| `class+1ECh` | derived, `007C4C08` | 0.1854 (Mav) | the climb gain |
| `class+518h` | derived, `007C4A44` | `tan(DropAngle)` = 0.4245 (Mav) | the slope of the bias term |

**The law is a glide slope, and it closes on itself.** `A_raw` falls with the range at
`d A / d R = s * gain = s * tan(DropAngle)`, so the flight-path angle that tracks it is
`atan(s * tan(DropAngle))`. The pitch the same `s` lets `009FB800` command, once `t` saturates, is
`DropAngle * s`. Those two are the same angle to within half a degree across the whole ramp:

| `s` | commanded slope `atan(s tan a)` | saturated demand `s a` | difference |
| --- | --- | --- | --- |
| 0.35 | 0.1477 rad (8.46 deg) | 0.1405 rad (8.05 deg) | 0.41 deg |
| 0.50 | 0.2091 rad (11.98 deg) | 0.2007 rad (11.50 deg) | 0.48 deg |
| 0.80 | 0.3273 rad (18.75 deg) | 0.3211 rad (18.40 deg) | 0.35 deg |

The aircraft is asked for exactly the pitch that flies the altitude it is being commanded, and as
the error closes `t` falls off the clamp and the demand eases linearly to zero. **That is the
flare.** With an altitude in `reference` neither half of that correspondence exists: the demand
saturates at `DEG(60)` while the commanded altitude, span dead, is a flat 12 m.

## 2. `009FB800`'s second argument is `009FBA50`'s arg3, proved three ways

`009FBA50` is `__thiscall(this, float base, float rangeLow, float rangeHigh, float scale)`,
`RET 10h`, body `009FBA50`-`009FBB1C`.

**(a) The x87 stack, instruction by instruction.**

```
009fbac5  FLD   float ptr [ESP + 0x18]      ; arg3 -> ST0
009fbac9  COMISS XMM0, [0x00d7a218]         ; span > 0 ?
009fbad2  FLD   float ptr [ESP + 0x14]      ; span  -> ST0, arg3 -> ST1
009fbad9  FMUL  ST1                         ; ST0 = span*arg3   NO POP
009fbadb  FMUL  float ptr [EAX + 0x518]
009fbae1  FADD  float ptr [ESP + 0xc]
009fbae5  FSTP  float ptr [ESP + 0xc]       ; pops the biased altitude -> ST0 = arg3
009fbae9  FLD   float ptr [ESP + 0xc]       ; ST0 = A_raw, ST1 = arg3
009fbaed  FLD   float ptr [ESP + 0x10]      ; ST0 = cap,   ST1 = A_raw, ST2 = arg3
009fbaf1  FCOMIP ST0,ST1                    ; pops cap     -> ST0 = A_raw, ST1 = arg3
009fbaf3  FSTP  ST0                         ; pops A_raw   -> ST0 = arg3
009fbb03  SUB   ESP,0x8
009fbb06  FSTP  float ptr [ESP + 0x4]       ; SECOND argument = arg3
009fbb0c  FLD   float ptr [ESP + 0x14]      ; the clamped altitude
009fbb10  FSTP  float ptr [ESP]             ; FIRST argument
009fbb13  CALL  0x009fb800
```

The `FLD` at `009FBAC5` is on both sides of the `009FBAD0 JBE`, and `009FBAD9` is `FMUL ST0,ST1`
with no pop, so `arg3` survives the bias, the clamp and the compare and is the value `009FBB06`
stores. Nothing is left on the stack at the `RET`.

**(b) The decompiler.** `exports/bsp/functions/009fba50/decompiled.c` ends
`FUN_009fb800(param_2,param_5);` - the clamped altitude and the fourth float.

**(c) The `1.6` that appears twice.** The move-to tick's own ramp at `009C1AF3` is
`Interp(0.05, 0.35, 0.4, 1.6, .)` (`00CE7638`, `00CF6560`, `00CE7804`, `00D06BB4`), so its largest
scale is **1.6**, and the dive cap at `009FB97F` is `max(DropAngle * 1.6, DEG(60))` with the same
`1.6` (`00CE3D48`). The steepest scale the move-to state can command makes `DropAngle * t` land
exactly on the first term of the cap. That is a designed relationship between the clamp on `t` and
the cap, and it only exists if `t`'s clamp is the scale.

## 3. The host's divergence, named to the line

| # | site | was | is |
| --- | --- | --- | --- |
| 1 | `src/game_hosts_units.cpp`, the `BotApproach` binding | `pin.reference = c.unclamped_altitude;` | `pin.reference = c.pitch_reference;` (`009FBB06`) |
| 2 | the same file, the dive-bomb attack run | the same | the same |
| 3 | the same file, `run_move_to_tick_009c18c0` | the same | the same |
| 4 | the torpedo attackrun call site | `command_altitude_and_throttle(nullptr, base, 0.0f, 0.0f, 0.0f)` | the four arguments `009D0951`-`009D0A92` computes |
| 5 | `include/bsp/plane_flight.hpp` | `unclamped_altitude` documented as "009FB800's second argument"; `returned_in_st0` as "the leaked arg3" | `pitch_reference` is the second argument; `unclamped_altitude` is passed nowhere |

Divergence 1 alone produces the measurement. With `reference` = 12.0:

```
x = err * ((12 + 1) * 0.5) = err * 6.5
t = clamp(|err| * 6.5 / 200, 0, 12) = clamp(|err| * 0.0325, 0, 12)
DropAngle * t >= DEG(60)  whenever  t >= 2.6089,  i.e.  |err| >= 80.3 m
```

At 800 m against a 12 m command, `|err|` is 788, `t` clamps to 12 and `DropAngle * t` is 4.817 rad
against a cap of 1.0472. The demand is `-1.0472` and stays there until the aircraft is inside 92 m,
which it never reaches alive. That is `local/tap_before_usn01.log` exactly.

Divergence 4 is why the command was 12 m from 4174 m out: with `range_low` and `range_high` both
zero the span is zero, `009FBAC9` skips the bias, and the glide slope - the only thing in the chain
that brings an aircraft **gradually** to the release altitude - never runs. The two defects are
independent and both are required for the plunge: with only the slope restored the demand would
still saturate; with only the reference restored the aircraft would be commanded to 12 m at a
shallow angle from 4 km out and arrive at the sea early.

## 4. The fix

`e5cae8ef6`.

* `include/bsp/plane_flight.hpp`, `src/plane_flight.cpp`: `PlaneCruiseAltitudeResult::pitch_reference`
  replaces the unread `returned_in_st0`, and both comment blocks carry the stack walk.
* `include/bsp/bot_task_states.hpp`, `src/bot_task_states.cpp`:
  `torpedo_attack_run_altitude_009d0951` is the new pure rule for section 1's first six lines, with
  every constant named after its address and every jump sense taken from the branch bytes.
* `src/game_hosts_units.cpp`: the three `pin.reference` sites; the torpedo attackrun call site now
  passes the four real arguments; `cin.class_gain` is `tan(DropAngle)`; the descent census prints
  `range`, `low`, `span`, `margin`, `denom`, `scale`, `gain` and the resulting slope in degrees.

## 5. Correction owed to `src/dive_bomb_task.cpp`, for `agent/cc8-dive-bomb`

`DiveBombAttackRunResult::commanded_throttle`, and the comment "the throttle, interpolated on the
margin over the clamped distance", are a misnomer. `009C43CD CALL 00419010` feeds `009C43D5 FSTP
[ESP+0xc]`, which after `009C43D2 SUB ESP,0x10` is `009FBA50`'s **arg3**, and the ramp uses the same
`00D7A2F0` = 0.1 and `00CE7804` = 0.4 as the torpedo attack run's `009D0A63`. It is the descent
scale. The throttle that state commands is the literal `1.0` at `009C4413`. The field keeps its name
for now because renaming it reaches into another lease; `src/game_hosts_units.cpp` feeds it to
`cin.scale` with that correction written at the call site.

## 6. Retractions

**`docs/ATTACK_RUN_DESCENT.md` section 1 and section 4.** Its equation reads

```
weighted = err * ((A + 1) * 0.5)
t        = clamp(-weighted / DropDist, 0, A)
```

with `A` the commanded altitude in both places. `A` is `009FB800`'s **first** argument; the value at
`009FB858 FLD [ESP+0x10]` and `009FB9D5 FLD [ESP+0x10]` is the **second**. The two are one slot
apart, and `009FB853 POP EDI` between `009FB849` and `009FB854` is what makes the same commanded
altitude appear at `[ESP+0x10]` before the pop and `[ESP+0xc]` after it. Reading `[ESP+0x10]` after
the pop as "still the altitude" is the whole error. Sections 2 and 3 of that document stand: pitch
mode 2 does not limit a dive and the nose-up floor cannot reach one. Its conclusion - "nothing in the
planner's pitch path stops a 60-degree dive, so the host is faithful" - is **withdrawn**: nothing in
the pitch path needs to stop a 60-degree dive, because in the image the attack run never commands
one.

**`docs/PLANE_FLIGHT.md`**, the `009FBA50` table, step 6, and the sentence that follows it. Both are
corrected in that document's own Correction section.

**`docs/TORPEDO_APPROACH_BLOCKER.md` section 8.5**, the open item "the pitch demand ... find its
producer", is closed by this document.

## ABI

| address | prototype | cleanup |
| --- | --- | --- |
| `009FB800` | `void __thiscall(this, float desiredAltitude, float reference)` | `RET 8`, body `009FB800`-`009FBA4F` |
| `009FBA50` | `void __thiscall(this, float base, float rangeLow, float rangeHigh, float scale)` | `RET 10h`, body `009FBA50`-`009FBB1C` |
| `009D07B0` | `void __thiscall(this, float dt)` | `RET 4`, body `009D07B0`-`009D0B0F` |
| `00419010` | `float __stdcall(float x0, float y0, float x1, float y1, float x)`, ST0 result | `RET 14h` |

`009D07B0`'s frame: `SUB ESP,0x3C` then `PUSH ESI`/`PUSH EDI`; `PUSH EBX` at `009D07FE` is balanced
by `POP EBX` at `009D08C3`, and the two `SUB ESP` pairs at `009D0868`/`009D0A23` and
`009D08EE`/`009D0907`/`009D0A68` are call frames that the `RET 14h`, `RET 8` and `RET 10h` callees
clean, so `[ESP+0xc]` at `009D09C2` is the slot `009D07E4` wrote and it holds `approach+90h`.

## Coverage

| routine | coverage |
| --- | --- |
| `009FB800` | complete, both arms |
| `009FBA50` | complete except the `ctl+394h` squadron limit at `009FBA9B`, which this host passes `has_squadron = false` |
| `009D07B0` step 4, altitude half | complete |
| `009D07B0` steps 1-3, step 4's throttle half, step 5 | partial: not run by this host, unchanged by this packet |

## Uncertainty

* `approach+7Ch`/`+80h` are `TorpReleaseDistNear` 450 and `Far` 650 scaled by `approach+24h`, per
  `include/bsp/torpedo_approach_update.hpp`; the run census now prints `low` so the product is
  measured rather than argued.
* `009D0A26` divides by `min(R, 2000)` with no zero guard. The host substitutes the ratio `0.4` when
  the denominator is zero, which the interpolation clamps to the same `0.8` an `x87` infinity would.
  A zero range cannot occur on the path that reaches it, because `009D07B0` is only ticked with a
  target.
* `class+518h = tan(DropAngle)` is `docs/BOT_TASK_STATES.md`'s reading of `007C4A3F`/`007C4A44`;
  this packet did not re-read it.
* The `ctl+398h` producer on the torpedo path is still unlocated, so `approach+74h = 0` remains
  unproven-normal. It no longer matters for the plunge: the base is the **bottom** of a slope now,
  not a target to be reached at once.

## Host methods

| method | call site | native | contract |
| --- | --- | --- | --- |
| `BotApproach::command_altitude` | `009D0A92` | `009FBA50` | read |
| `BotStateMoveTo::glide_slope` | `009C1B17` | `009FBA50` | read |

## Validation

Both runs are `--frames 3200 --press-start-frame 30 --menu-select USN01 --mission-frames 3000
--mission-frame-seconds 0.05`, on the same binary lineage and the same placement, with
`query session` = `console Active` before each and `exit_code=0 frames_presented=3199
loop_finished=1`.

| | before, `local/tap_before_usn01.log` | after, `local/descentlaw_full_usn01.log` |
| --- | --- | --- |
| `plane water contact` | all five Mavs, `alt` -4.13 / -5.05 / -5.88 / -5.05 / -0.01, `\|v\|` 140.95 to 141.12 | **the string does not appear in the log** |
| approach ticks, Mav1 | 124 | 1299 |
| min range 90h, Mav1..Mav5 | 3928.5 / 3653.5 / 3579.1 / 3968.6 / 3901.3 | **3.8 / 1.7 / 2.1 / 2.7 / 1.5** |
| aim ticks, Mav1..Mav5 | 0 / 0 / 0 / 0 / 0 | **378 / 357 / 358 / 352 / 353** |
| `aim_complete_2Ch` | the aim state never ran | **1 for all five**, first true at aim tick 267 to 283, `clause=range` |
| pitch demand at 800 m | -1.0472 (`-DEG(60)`), and unchanged to the water | **-0.0573** |
| commanded altitude at 4183 m range | 12.00 | **761.91** |
| `\|v\|` entering aim | n/a, dead at 141 m/s | 81.00 m/s |
| `release_arm_009D2287`, releases | 0, 0 | 0, 0 - see below |

Mav1's descent census after the fix, the whole attack run:

```
n=1   commanded=761.91 live_alt=800.0 demand=-0.0573 pitch= 0.0000 | range=4183.4 low=650.0 span=3533.4 margin=400.0 denom=2000.0 scale=0.5000 gain=0.4245 slope_deg=11.98
n=101 commanded=736.88 live_alt=768.3 demand=-0.0473 pitch=-0.0686 | range=4065.4 low=650.0 span=3415.4
n=151 commanded=722.89 live_alt=742.2 demand=-0.0291 pitch=-0.0846 | range=3799.5 low=450.0 span=3349.5   <- the 15 s switch
n=251 commanded=604.89 live_alt=685.2 demand=-0.1210 pitch=-0.1149 | range=3243.5 low=450.0 span=2793.5
n=351 commanded=479.55 live_alt=580.3 demand=-0.1516 pitch=-0.1496 | range=2653.0 low=450.0 span=2203.0
n=401 commanded=414.68 live_alt=519.2 demand=-0.1573 pitch=-0.1560 | range=2347.3 low=450.0 span=1897.3
```

The commanded altitude tracks the range down the slope, the aircraft tracks the command, the demand
tracks the aircraft to three decimal places, and the 15-second switch from `TorpReleaseDistFar` 650
to `Near` 450 fires at `009D0A0E` exactly as the listing says. `approach+24h` is 1.0 here - the
aim census prints `speed_80=650.0` unscaled, because `max_spd` 69.44 m/s is below
`Pilot/Torpedo/ReferenceSpeed` - so `low` is the authored row value directly. `scale` stays 0.5000
for the whole run because `margin` is pinned at its 400 cap and `denom` at its 2000 cap; the ramp
only starts moving inside 2000 m of range or below 1000 m of altitude.

**What is still not a drop, and where the next gate is.** All five aircraft now arrive over the
target, but `release_arm_009D2287` is still 0 and `releases=0`. Two things stand between here and a
torpedo, and neither is in this packet:

* `run_time_009D1360 = 0` for all five. `src/torpedo_aim_tick.cpp` calls
  `update_run_time_009d1360` only inside `009D19A0`'s `turn_room > f14_range`, and the run measures
  `F34=546.26 F14=767.24` at the first completed solution, so that gate never opens. `009D1360`
  writes `approach+98h`/`+A0h`, which `009D2287` needs.
* **The aim state commands no altitude.** `009D15F0`'s callee list contains neither `009FBA50` nor
  `009FB800` - it calls `009D1500`, `009D1360`, `00427EB0`, `00414DB0`, `00438AA0`, `00438B10`,
  `00419010`, `009FA3A0`, `00903860` and `007F0280`. So when the `2200 m` latch closes the descent
  command stops, and the aircraft hold 488.9 to 559.9 m over the target for the rest of the mission
  while `009D20B4`'s release band wants 25 to 40 m. Something between the latch and the band has to
  bring them the last 400 m, and it is not the chain this packet fixed. That is
  `docs/TORPEDO_AIM_TICK.md`'s ground.

The 60-second intermediate run `local/descentlaw_after_usn01.log` is kept as the first evidence that
the latch closes: at 1200 mission frames Mav2 and Mav3 had 38 and 51 aim ticks with the other three
still inside 2400 m and descending.
