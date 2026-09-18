# The wiring lands, and neither half executes on USN01

Addresses: 009C18C0, 009C1850, 009C1B17, 0099D300, 0099D7A5, 0099D7E8, 0099D81E, 0099D87E,
0099D8CD, 0099DAC4, 0099DBBF, 009D3210, 009D4030, 009D4053, 009D41C4, 009D48DE, 009D48F6,
009FBA50, 009FB800, 007C4A44.

Packet `cc8_torpedo_glide_throttle_wiring`, owner `agent/cc8-torpedo-run-in`, on main `16cbe68a4`
merged.

Three bindings went in, all three are exercised by the build, and **neither of the two behavioural
ones executes on USN01**. The census says so directly, and the reason is a mission fact rather than
a defect in the wiring.

## 1. What was wired

| binding | native | where |
| --- | --- | --- |
| the throttle arm | `0099D300`'s four arms | `plan_yaw_0099d300()`, before its `record` |
| the desired-speed setter | `009C1850`, step 2 | the move-to branch of the state dispatch |
| the glide slope | `009C18C0` step 5 through `009FBA50`/`009FB800` | the same branch |

The glide slope uses the values `docs/TORPEDO_AIM_ALT_AND_SAFE_DIST.md` established, with **no
substitution**: the base is `approach+74h + approach+78h`, the release altitude; `rangeLow` is
`approach+7Ch` or `+80h` on the `approach+134h >= 15.0` switch, the release distances; `rangeHigh`
is `approach+90h`, the live range; the scale is the `009C1AF3` fold; and `class+518h` is
`tan(DropAngle)` from `007C4A44`.

Two labelled substitutions remain, both in the throttle path: the desired speed itself, because
`007C47F0` and `009BECD0` are unread and the row's authored `TravelSpeed` stands in; and
`plan+2B8h`, taken as `1.0` so the error stays in metres per second.

## 2. Neither half executed

**The glide census printed zero lines**, and the plane throttle is `1.00` in every sample of the
run. So the move-to branch was never taken and the throttle arm never wrote.

**Why: USN01's ordered torpedo bombers are engaged from the first arm tick.** `009D4030`'s rule
returns the approach state - move-to or follow - only when `009D3210`'s engaged test is false
(`009D41C4` when not attacking, `009D4053` when attacking). The run's own summary reads
`blocked_engaged_009d3210=0`, and the aircraft start **1490 m** from their targets. They are inside
the engage range at mission start, so the rule sends them to `attackrun` on the first tick, the
transition at `009D48DE` runs before the state tick at `009D48F6`, and the move-to tick is never
reached.

That is mission-specific, not structural. The move-to state is reachable; these aircraft simply
never occupy it. A mission whose bombers launch outside the engage range would fly the glide slope.

**And with no move-to tick, nothing raises the one-shot.** `009C18A7` is the only raiser on this
path, so `plan+2D8h` stays zero, `0099D7A5`'s test fails, and the throttle arm returns without
writing on every think. The two failures are one chain, not two.

## 3. The second reason, and a correction I have to make

The lead asked which suspect to name if the throttle never moved. The first is section 2. The
second is the one `docs/PILOT_THROTTLE_CUT_RAISER.md` flagged - **and my reading of it was wrong,
and I am withdrawing it here.**

That doc says `[ESP+0x6c]`, the multiplier at `0099DBBF`, is `|slot desired - plan+274h|` from
`0099D7E8`-`0099D81E`, and therefore zero for a slot straight out of the reset. **I drew that from
a `grep` whose output was capped at eight lines.** The full list of that slot's mentions has a
further write at **`0099D87E`**, between `0099D81E` and the increment, inside the block at
`0099D85E`-`0099D892` that takes a running minimum against `XMM4 - [ESP+0x24]`.

So the multiplier at `0099DBBF` and `0099DAC4` is **not** settled, the "no frame-time factor"
claim is not established, and the note I asked the lead to keep in the host section is withdrawn.
The host binding still computes `|desired - current|`, which is now a **labelled substitution**
rather than a reading, and it is the first thing to fix when the arm is next exercised.

This is the third time in this area that a conclusion drawn from a truncated or single-sense read
has had to be retracted. The two before were jump senses; this one was a capped `grep`. The rule
that would have caught all three is the project's own: do not conclude from the head alone.

## 4. Validation

`tools/run_game.ps1`, 3200 frames, `--mission-frames 3000` at `0.05` s, on main `16cbe68a4` merged
with this branch.

### USN01

| | this run |
| --- | --- |
| glide census lines | **0** |
| plane throttle, every sample | **1.00** |
| descent census lines (the attack-run tick) | 15 |
| commanded run-in altitude | 12.00 m |
| pitch demand | -1.0472 rad, the `DEG(60)` cap |
| water contacts | 5 |
| water-arrival speed | 141.68 to 141.91 m/s |
| torpedo drops / breakups / `swims_started` | 0 / 0 / 0 |
| torpedo hits, torpedo damage | 0, 0 |
| mission hits / kills / damage | 10 / 0 / 313.4 |
| `distance_moved` | 145308 m |
| plane arm: free flight / surface | 48185 / 11815 |

**Nothing moved that this packet could have moved.** The dive angle is the same `DEG(60)` cap, the
water-arrival speed is the same 141.7 m/s as
`docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md` measured, the aircraft still never reach the release
altitude, and no release fires. A before run was not taken because the censuses prove the added
code did not execute: a binary with the branch removed would produce the same numbers by
construction, and that is a stronger statement than a second run.

The mission's 10 hits and 313.4 damage differ from that doc's numbers because of everything that
landed on main between the two, not because of this packet.

### USN02

168 hits, 3 kills, 20721.4 damage, first hit 31.80 s, `swims_started = 44`, **zero** glide census
lines and `plane step: steps=0`. The mission carries no flying aircraft, so neither binding can
reach it, and its numbers match the last three packets' USN02 runs exactly.

The run's presenter was cut short - `frames_presented=1183` with `presents_skipped=1345` and exit
code 1 - but the mission summary is present and complete, and the protocol judges a run by its log
rather than its exit code. It is kept on that basis and flagged here.

## 5. The next gate, by address and value

**`009D3210`'s engaged test, and the engage range it uses.** USN01's bombers are inside it at
mission start, so they never occupy the move-to state and the glide slope has nothing to do. The
test is `task+484h * [00D05AC8] > task+488h` at `009D325B` plus the earlier clauses, and neither
`task+484h` nor `+488h` has been read. Until an aircraft spends ticks in move-to, the glide slope
and the throttle arm are both dead code on this mission.

Two ways forward, and they are independent:

1. **Read `009D3210`'s range fields** and check whether the host's engaged test is too permissive.
   If the native would keep these aircraft in move-to for the first part of the run, the glide
   slope starts working with no further change.
2. **Command the attack-run descent properly instead.** The attack-run tick's own `009FBA50` call
   already runs and commands 12 m from 800 m, which saturates `009FB800` at the `DEG(60)` cap. The
   glide slope's shape - a commanded altitude that falls with the remaining range - could be given
   to the attack-run branch as well, but **that would be inventing behaviour**: `009D07B0` passes a
   zero range pair, as `docs/TORPEDO_THROTTLE_CUT.md` proved. So this needs the native's own answer
   for why an attack run from 800 m does not plunge, which nothing read so far provides.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| `run_move_to_tick_009c18c0`, steps 2 and 5 | `src/game_hosts_units.cpp` | `009C1850`, `009C18C0` | binding, desired speed **substituted** |
| the throttle arm in `plan_yaw_0099d300()` | `src/game_hosts_units.cpp` | `0099D300` | binding, `plan+2B8h` and the multiplier **substituted** |

## Corrections

Appended to the doc it amends, and verified present there.

* `docs/PILOT_THROTTLE_CUT_RAISER.md`'s increment section: the `[ESP+0x6c]` multiplier claim and
  the "no frame-time factor" conclusion are withdrawn, because `0099D87E` writes that slot between
  the read I based them on and the increment.

## no_ghidra_function

None.

## Follow-up packets

1. **`009D3210`'s engaged test and its range fields**, which is why move-to never ticks here.
2. **`[ESP+0x6c]`'s real value** at `0099DBBF`, from `0099D85E`-`0099D892`.
3. **`007C47F0` and `009BECD0`**, the desired speed itself.
