# Why no fighter hits a Kate: the rate law's slide term (packet cc9_kate_engagement)

2026-09-25. Ghidra read-only. Names are hypotheses. The finding to resolve (docs/FIGHTER_GUNFIRE_CONSUMER.md
section 3): fighters kill Vals and movievals, but no fighter ever damages a Kate, although the
planner orders a 12-member fighter group led by Lexington-class01_sqn01 onto Kate #6.1.

## 1. The engagement, traced

A diagnostic (`kFighterChaseTraceDiag`, committed OFF) logs every dogfight fighter with a target
every 2 s. The runs are `local\KET_9000.log` and `local\KET2_9000.log`: E2 9000,
`BSP_GUNNERY_RNG_STREAMS=1`, current main plus the diagnostic.

- **The order arrives, and the group takes it.** At 142.95 s the planner orders the group onto Kate
  #6.1's group. Every member's dogfight target switches to a Kate at 143.00 s. The hand-out works.
- **The geometry is poor but not hopeless.**
  - The Kates cruise at 72-80 m/s near 1,400 m, descending.
  - The fighters' MaxSpd is 83.33.
  - sqn01 starts 3.7-7.9 km behind.
  - sqn03 takes off from the Lexington at 150 m and has 1,250 m to climb.
- **The moveto leader slows.** Its commanded speed falls to 33-55 m/s while its wingman .-2 trails
  3 km off station. This is the image's own wingmen-wait law, `009BECD0`, read and bound in
  docs/DOGFIGHT_MOVETO.md.
- **In aim, the fighters bank and never come round.** From 164 s, sqn01|.-3 holds bank 1.228 rad
  (70 degrees) and pitch +0.19 for 26 s while its heading error to the Kate grows from 0.58 to 2.45
  rad. Its speed falls from 65 to 28 m/s, and the range opens from 1,694 to 2,141 m. The leader holds
  bank 1.091 rad with the error growing from 0.53 to 3.0 rad. Both leave aim with the Kates beyond
  2 km, and no burst ever fires at a Kate.

## 2. Why a banked fighter does not turn in this host

Three terms could turn it. The host has the first two as the image does. The third is missing.

- **The planner's yaw base term** is faded to 0 above the bank limit (`0099DFFB`, tuning `+7Ch` and
  `+80h`). At 60-70 degrees of bank it contributes nothing, as in the image.
- **The planner's turn numerator** (`0099E69B`-`0099E729`, read here).
  - It is produced only when the pitch demand saturates. Above +1 it is
    `sin^2(bank) * cos(pitch) * SlideRatio * YawSpd - k * X`, with `k` NegativePitchRatio when
    inverted, else 1. Below -1, and only when inverted, it is the same product `+ NPR * X`.
  - Otherwise it stays 0 (`0099E68D`-`0099E72F`).
  - The fighters' demand is about 0.003, so it is 0 in the image too.
  - The host holds it at 0 always. That is a separate, smaller gap: it would matter only when the
    demand saturates.
- **The rate law's slide term** (`007DA9F5`-`007DAA58`), when `007DA380`'s flag is set, which it
  always is in free flight: `yaw -= SlideRatio * YawSpd * sin(unit+C68h) * cos(unit+C64h)`.
  - `007DAA00` reads the bank at `[unit+C68h]`, `007DAA23` the pitch at `[unit+C64h]`.
  - This is the yaw a banked aircraft gets from its bank alone: it is what makes a banked plane turn.
  - **The host passed 0 for both angles.** `control_step_007da710` never set `pitch_angle_c64` or
    `bank_angle_c68` in `PlaneControlUnitState`. The header recorded that as a gap from when the host
    did not maintain the angles. The host has since maintained both, through `007C1900`, for the
    planner. So the slide term was always 0, and a banked plane got no bank-induced yaw.

## 3. The binding

`kRateLawAttitudeTermsBound` (`src/game_hosts_units.cpp`, `control_step_007da710`) passes
`unit_.plane_pitch_angle_c64` and `unit_.plane_bank_angle_c68` into the rate law's targets. Two terms
read them: the slide term, and the state-6 bank factor (`007DA766`), which is inert in free flight
(state 7). The change applies to every aircraft, not only to fighters.

## 4. Predictions, written before the pair

The pair is `kRateLawAttitudeTermsBound` OFF against ON, one tree, E2 9000,
`BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`. The OFF side is expected to match `local\KET2_9000.log`
on every behaviour row, because the only change there is the diagnostic switched off.

| row | OFF (expected) | ON prediction |
| --- | --- | --- |
| Kate death rows with fighter-gun hits (`hits c0 > 0`) | 0 | at least 1 |
| Kate deaths credited to fighter guns (`killer_cat=0`) | 0 | 0-6 |
| Kate deaths to AA (categories 1, 5, 6) | 16 | 10-16, falling |
| fighter bursts / fire ticks | as KET2 | within -30% to +150% |
| fighter kills (all victims, `killer_cat=0`) | as KET2 (5-6) | 4-15 |
| Kate / Val deaths | 16 / 16 | 12-16 / 12-16 |
| hit records | about 590 | 450-750 |
| torpedo releases / dive-bomb releases | 4-6 / 0 | 2-8 / 0 |
| Lexington moved | 5.5-7.5 km | 5.5-7.5 km |
| US fighter sea losses | 0 | 0-1 |
| plane distance moved (all aircraft) | as KET2 | within +-15% |
| mission end | none | none |

**Risks, written before the runs.** The term acts on every banked aircraft, so the Kates' and Vals'
own paths change: their turns tighten. A sign error would turn every banked aircraft the wrong way.
That would show as heading errors growing faster than OFF, and as aircraft spiralling away from
their targets. If that happens, the verdict is OFF, with the evidence.

## 5. The first pair: the slide term alone

`local\KE0_9000.log` (binary `local\ke0`, OFF) and `local\KE1_9000.log` (`local\ke1`, the slide term
ON). KE0 equals the trace run KET2 on every row.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| Kate death rows with fighter-gun hits | 0 | 4 | at least 1 | held |
| Kate deaths credited to fighter guns | 0 | 3 | 0-6 | held |
| Kate deaths to AA | 16 | 13 | 10-16, falling | held |
| fighter bursts / fire ticks | 9 / 159 | 24 / 452 | -30% to +150% | fire ticks **missed, high** (+184%) |
| fighter kills | 5 | 14 | 4-15 | held |
| Kate / Val deaths | 16 / 16 | 16 / 16 | 12-16 each | held |
| hit records | 569 | 593 | 450-750 | held |
| torpedo / dive-bomb releases | 4 / 0 | 7 / **4** | 2-8 / 0 | dive-bomb **missed** |
| Lexington moved | 5982 m | 6584 m | 5.5-7.5 km | held |
| US fighter losses | 0 | **7** (6 depth kills) | 0-1 into the sea | **missed** |
| plane distance moved | 1,087,562 m | 1,150,149 m | +-15% | held (+5.8%) |
| mission end | none | none | none | held |

**What the sea losses show.** Every Yorktown loss has the same signature before the depth kill:
- the nose is 0.35-0.47 rad down;
- the pilot holds full up stick (`live_pitch` = 1.0);
- the throttle is at 1.0.

So the planner's pitch demand is saturated. Section 2's turn numerator exists for exactly that case.
Its product `sin^2(bank) * cos(pitch) * SlideRatio * YawSpd` is the slide term's own, and the yaw
arm uses it to counter the slide term's yaw. The host held it at 0, so the slide term arrived without
the pilot's compensation, and banked fighters fell through their turns.

**Verdict for the first pair: not landed.** The slide term alone is half of one chain. The second
pair adds the other half under the same switch.

## 6. The second pair: slide term and turn numerator together

`kRateLawAttitudeTermsBound` now also does two things in the planner:
- **The numerator.** It computes the turn numerator from the pitch arm's unclamped demand. The new
  rule is `yaw_turn_numerator_gated_0099e68d` in `include/bsp/plane_ai_control.hpp`: `p - k*X` when
  the demand is above 1, `p + NPR*X` when it is below -1 and the plane is inverted, else 0.
- **The order.** It runs the yaw arm after the pitch arm, as `0099E81A` follows
  `0099E490`-`0099E739` and reads the numerator at `0099E8F6`.

A summary line counts the thinks with a positive numerator.

Predictions, written before the runs (`local\KE0b_9000.log` OFF against `local\KE1b_9000.log` ON, one
new tree):

| row | OFF (expected equal to KE0) | ON prediction |
| --- | --- | --- |
| turn numerator ticks | 0 | above 0 |
| US fighter depth kills and sea contacts | 0 | 0-1 |
| US fighter losses | 0 | 0-3 |
| Kate death rows with fighter-gun hits | 0 | at least 1 |
| Kate deaths to AA | 16 | 10-16 |
| fighter kills | 5 | 4-15 |
| fighter fire ticks | 159 | 100-500 |
| Kate / Val deaths | 16 / 16 | 12-16 each |
| hit records | 569 | 450-750 |
| torpedo / dive-bomb releases | 4 / 0 | 2-8 / 0-4 |
| Lexington moved | 5982 m | 5.5-7.5 km |
| plane distance moved | 1,087,562 m | +-15% |
| mission end | none | none |

### 6.1 The second pair, measured

`local\KE0b_9000.log` (binary `local\ke0b`) and `local\KE1b_9000.log` (`local\ke1b`). KE0b equals KE0.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| turn numerator ticks | 0 | 568 | above 0 | held |
| US fighter depth kills and sea contacts | 0 | **5** (all Yorktown: sqn02 x3, sqn04 x2) | 0-1 | **missed** |
| US fighter losses | 0 | 5 | 0-3 | **missed** |
| Kate death rows with fighter-gun hits | 0 | 4 | at least 1 | held |
| Kate deaths by category (0 fighter / 1 / 5 / 6) | 0 / 10 / 0 / 6 | 4 / 6 / 1 / 5 | AA 10-16 | AA 12, held |
| fighter kills | 5 | 17 | 4-15 | **missed, high** |
| fighter bursts / fire ticks | 9 / 159 | 23 / 492 | fire 100-500 | held |
| Kate / Val deaths | 16 / 16 | 16 / 16 | 12-16 each | held |
| hit records | 569 | 583 | 450-750 | held |
| torpedo / dive-bomb releases | 4 / 0 | 6 / 3 | 2-8 / 0-4 | held |
| Lexington moved | 5982 m | 6666 m | 5.5-7.5 km | held |
| plane distance moved | 1,087,562 m | 1,138,827 m | +-15% | held (+4.7%) |
| mission end | none | none | none | held |

**The fighters now engage the Kates.** Four Kates die to fighter guns, the AA share falls from 16 to
12, and Kate #6.1's pursuers come round in their turns. The engagement gap was the missing turn
chain: the rate law's slide term and the planner's turn numerator.

**The US fighter sea losses remain, and they decide the verdict.** With the numerator wired, 568
thinks carry a positive turn term, and five Yorktown fighters still reach the water. The terrain arm
of the pilot's avoidance pass (`0099F1C0`) is the image's only floor (docs/CLIMBOUT_SPEED_GATE.md). In
both runs it records **no tick at all for any US fighter**, including the ones that dive into the
sea. The Kates (12 of 16) and some Vals get bands.
- The counter rises only when the probe produces a band.
- The Yorktown fighters are squadron members: their `plane squadron leave` events log.
- They are in free flight: the surface census reads `state=7`.
So the arm runs for them and its probe never finds the water ahead of a diving fighter. That is the
next read, and it is a candidate host gap under every fighter, not a property of the slide term.

## 7. Verdict

- **`kRateLawAttitudeTermsBound` stays OFF**, with both halves bound and measured: the slide term
  (`007DAA3B`) and the gated turn numerator (`0099E68D`) with the image's arm order.
- **It fixes the engagement.** Fighters hit and kill Kates. But it sends five Yorktown fighters into
  the sea, because the one floor that should catch them never fires for a fighter.
- **Next packet.** Read why `0099F1C0`'s probe produces no band for a US fighter diving at the water:
  - the look-ahead geometry;
  - `class+A4h`;
  - the avoid-zone layer's coverage where the Yorktown flights fight.
  Then re-take this pair.
- **The planner's own Kate order timing** (docs/PLANNER_KATE_TARGETING.md) is unchanged by this
  packet. The order still arrives at 143 s.
