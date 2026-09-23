# What happens when a follower LEAVES follow to attack (packet `cc8_follow_attack`)

Worker `cc8-follow-attack`, 2026-09-19, branch `agent/cc8-follow-attack`, base `0920f88e8`
(main `17bbbfc36` + the merge of `agent/cc8-follow-enter`'s handoff commit `d8019b8cc`).
Fifth on the plane squadron FOLLOW chain. `docs/PLANE_FOLLOW_ENTER.md` is the packet before this
one; its section 7 is the run this packet re-reads.

## 1. Run D was not truncated. Two of the six flew into the sea

`docs/PLANE_FOLLOW_ENTER.md` section 7 reads run D's six 0-release wing members as
"they hold formation for 1100-1550 ticks ... and the run ends before the approach sequence can
complete", and the pin's comment in `src/game_hosts_units.cpp` says "they are lost to holding
station until there is no mission left to fly". **Both are wrong**, and the predecessor's own log
falsifies them. Re-read of `...-cc8-follow-enter\local\D_mode_fed.log` against
`B_fed_placement_on.log`:

**Run B has zero `plane water contact` lines. Run D has four.** Two are the dive-bomb wing members
in question:

```
plane water contact: unit=D3A Val #1.1|.-2 alt=-1.66 water=0.00 |v|=48.38 state 7 -> 6
plane water contact: unit=movieval|.-2     alt=-1.34 water=0.00 |v|=49.11 state 7 -> 6
```

A second instrument agrees: the last `flyabove altitude 009C6E10` sample for those two units is
`err last=-209.4` and `-207.8` against `target=210.0`, i.e. an altitude of **0.6 m and 2.2 m**.
Their arms stop at 1812 and 1400 ticks because the aircraft died, not because the mission ended -
their squadron-mates' arms run to 2370 and 2149.

So landing criterion (b), "no new water contact", **FAILS** in run D. That was not reported.

**Only two of the four are this packet's mechanism**, and the distinction matters. The other two are
`D3A Val #3.1`, a flight **leader**, and `Yorktown-class01_sqn02|.-2`, a **torpedo** wing member.
The leader released both its bombs (`releases=2`) and its only long post-attack state is `done`
(786 ticks); a flight leader is never placed on a station, so its contact cannot be the section 7
mechanism and is more likely the `done`-state descent that `docs/BOMBER_AFTER_TASK.md` already
records. Attributing all four to one cause would be wrong.

### The six, one at a time

| unit | states in D | why it released nothing |
| --- | --- | --- |
| `movieval\|.-2` | `follow=1546 flyabove=266` | **water contact** at arm tick 1812 |
| `D3A Val #1.1\|.-2` | `follow=1134 flyabove=266` | **water contact** at arm tick 1400 |
| `D3A Val #3.1\|.-2` | `follow=969 goaway=1071 flyabove=98` | **aborted** the fly-over at 904 m (`err last=694.1`) on the `009C66E3` edge, not on roll-in, then sat in `goaway` for 1071 ticks |
| `D3A Val #7.1\|.-2` | `follow=1070 goaway=186 flyabove=83` | `transitions=32`, thrashing |
| `D3A Val #5.1\|.-2` | `follow=1134 flyabove=215` | truncated; whole squadron's arm ends at 1349 |
| `D3A Val #5.1\|.-3` | `follow=1133 aimdive=52 flyabove=109 turndown=55` | truncated mid-attack |

Only the `#5.1` pair is the mission window, and **that pair released 0 in run B as well**, so it is
not a loss this change caused. Four of the six are new failures that more mission time cannot undo.

And `#5.1|.-2` was not going to recover either: its last fly-over altitude sample is
`calls=215 ... err last=68.1 ... pitch=-0.161 rad`, i.e. 278 m and still nose-down, 215 ticks into a
fly-over the others leave in 86-129. It was truncated on the same trajectory as the two that
drowned, not short of a release.

### The loss is entirely the wing members, and it is exactly nine

Per-unit dive-bomb releases, B -> D. Five `.-2` members go `2,2,2,0,0` -> `0,0,0,0,0`; three `.-3`
members go `2 -> 1`; every leader is unchanged. Total `-9`, which is the whole `35 -> 26`.

The runs' own summary lines decompose it and confirm the hand tally: `summary mission dive-bomb
task ... releases=` is **23** in A, **23** in B and **14** in D, while `summary mission gunnery
torpedo_drop drops=` is **12** in all three. So the `-9` is entirely dive-bomb, the torpedo side is
untouched by the mode line, and the "releases" figure in `docs/PLANE_FOLLOW_ENTER.md`'s table is
those two summed.

## 2. What the pin actually does: it is the only thing that makes `attackrun` reachable

In run B **every** dive bomber flies `attackrun` for 978-1564 ticks. In run D **no aircraft enters
`attackrun` at all** - leaders go `moveto -> flyabove`, members go `follow -> flyabove`.

That is forced by the chooser. `009C8310` returns `kAttackRun` only on its last arm, reached when
the task is `engaged` with the in-range latch **clear**; `009C83F8`'s `engaged` is
`latch || (mode == 2 && has target)`. With mode 1, `engaged == latch`, so `engaged` implies the
latch is set and the chooser always takes `kFlyAbove` first. **`attackrun` is unreachable for any
aircraft whose squadron mode is 1.** Pinning the mode to 2 is what makes it reachable, and that -
not "standing in for the missing follow state" - is what the pin has been buying.

## 3. And mode 1 is the image's value for these aircraft

`docs/TORPEDO_ATTACK_MODE.md` section 1 already carries the complete writer census of `ctl+370h`,
over every store form with `007ED3F4`/`007ED43C` as positive controls. Mode **2** has exactly two
producers image-wide:

* `008A4C41`, inside `008A4B10 BSP_LuaBinding_PilotStopCloseToShip`. `tools/callsite_census.py
  007ED430` returns `total 1`, so that is the only call site of the raising setter.
* pilot-control message `BCh` with a non-zero payload, at `007F0068`. Its producer is `unread`;
  `docs/TORPEDO_ATTACK_MODE.md` section 2 states no site in the image builds the message with an
  immediate `BCh`.

An AI-ordered bomber has no `closetoship` task (section 3), so nothing lowers or raises it: the
flight leader's `0099B740` tail sets it to **1** on the first cruise think and it stays there.

**So the fed line is faithful and the pin is the substitution**, not the other way round. The pin
cannot be defended as the image's behaviour; it can only be defended as a stand-in that scores
better. Answering the question the brief put: there is **no** image path that sends an AI wing
member to `attackrun` in this mission.

## 4. Why an aircraft that enters the fly-over too steep can never leave it

`009C83E0`'s `kFlyAbove` arm leaves only on `flyabove_ready_791`, the `+19h` roll-in permission
`009C67B0` computes. Its two arms are `|bearing error| > 1.6 rad` (`00CE3D48`) and `span <= 0`,
where `009C65FD` builds

```
span = max(R - (0.7 * max(B, 100) + 200), 0)
```

with `R` the three-second lead range and `B` the height above the aim point. The three constants
are read at the width of their loading instructions: `0.7` is the **qword** at `00CEFFA0`, `200.0`
the qword at `00CE4D70`, `100.0` the float at `00CE3D08`, and the bearing `1.6` the qword at
`00CE3D48` (the float at those bytes is `-1.08e-19`, so the width is load-bearing there). Differentiate along
the flight path, with closing rate `Rdot` and descent rate `Bdot` both positive:

```
d(span)/dt = -Rdot + 0.7 * Bdot
```

so **the span shrinks only while the flight-path angle is shallower than `atan(1/0.7) = 55.0
degrees`.** Steeper than that, descending makes the threshold recede faster than the aircraft
closes, the span grows, and roll-in can never be earned - and `kFlyAbove` has no other exit.

Measured, both endpoints instrumented rather than derived. `movieval|.-2` left follow at
`alt=1340 rng=2073 span=862` and hit the water 1405.7 m short of the aim point (its last
`approach+BCh`): 1339 m down for 667 m closed, a mean path angle of **63 degrees**. Its span at
death is `1405.7 - 270 = ~1136`, up from 862. The aircraft that work are at 12-22 degrees and leave
in 86-129 ticks.

## 5. The entry geometry, and why this packet does NOT yet blame the follow station

Span at the fly-over entry, every dive bomber in both runs:

| entered `flyabove` from | span | altitude |
| --- | --- | --- |
| `attackrun` (run B, all 15) | 672-678 | ~1393 |
| `moveto` (run D leaders) | 690 | 1360 |
| `follow`, slot `.-3` (run D) | 762 | 1382 |
| `follow`, slot `.-2` (run D) | 862-865 | 1340 |

`movieval|.-2` enters at the same planar `rng=2073` in both runs, so the extra 190 m of span is
lead-range geometry - the bearing the station leaves it on - and not distance to go. The `.-2`
station is on the unfavourable side and carries ~100 m more span than `.-3`.

**This is a thin margin, not a demonstrated defect.** `.-3` reaches `span = 0` in 109 ticks at
~5.8 m/tick; 100 m of extra span is ~17 ticks, and the fly-over's descent steepens past the
55-degree line somewhere near there. So the honest statement is: the follower arrives with less
margin than any other entry, and on the unfavourable side that margin is gone.

Section 7 then reads the station law itself and **clears it**: the geometry is the image's, and the
margin is spent by how this host reaches the station rather than by where the station is.

## 6. An unmodelled contract on this packet's own addresses

`009C6270`, the fly-over ENTER, opens with `CALL 009C3DA0` on `[ESI+4]`, the approach. `009C3DA0`
(`009C3DA0`-`009C3E9A`, `__thiscall(approach)`, its other caller being the task seed `009C3EA0`) is
**three `BSP_Random_UniformFloatRange` draws**: `009C3DCB` and `009C3DF5` from the pilot config row
at `[approach+14h]+30h` and `+34h`, written through `009FA380` into the approach's `+30h`..`+3Ch`,
and `009C3E8C` from `+2Ch` into `approach+C8h`.

**Which config fields those are is NOT settled, and this packet does not settle it.** The names in
`include/bsp/robot_config.hpp` encode image offsets, so `[row+30h]`/`+34h`/`+2Ch` taken at face
value are `torp_target_point_select_prec_030`, `torp_throw_mul_034` and `torp_targetv_error_02c` -
**torpedo** parameters inside a dive-bomb function. The host's own comment at `009C3EA0` instead
reads row `+38h`/`+3Ch` as `dive_bomb_release_alt_1_044`/`_2_048`, which asserts a `+0Ch` shift
between `[approach+14h]` and the row base those names are keyed to; under that shift the three
draws are `dive_bomb_calc_target_pos_error_038`, `targeth_error_03c` and `targetv_error_040`, which
is what a dive-bomb aim-error draw ought to read. One of the two readings is wrong. Settling it
belongs to whoever owns `009F9D22` and the approach constructor, not to this packet.
**`contract: unread` - the base of `[approach+14h]`.**

What is settled either way is the shape: **the fly-over enter re-draws three random approach
parameters every time it is entered**, so in the image each aircraft enters the fly-over with its
own draw. This host draws none: `db_aim_point_height_50` is a labelled `0.0f`
substitution and nothing re-seeds on the enter. The enter's other unmodelled effects are the
`unit+844h` byte clear (`009C6283`) and `state+20h` (`009C628C`); the host models only the `+1Ch`
clear. `contract: unread` - the consumers of `approach+30h`..`+3Ch` and `approach+C8h`.

This does not explain the drowning - a zero error is the same for every aircraft - and it is
recorded as a contract for the next reader, not as this packet's cause.

## 7. The station is the image's. The way this host reaches it is not, and that is the defect

The station itself is not to blame. `007F23A0` shape 1 is reconstructed in
`src/plane_formation.cpp`: each component is `pair_number * displacement * morale`, with the
odd-index mirror at `007F285F`-`007F28B9`. The run's own census prints
`seat1 index=1 local=(-60.0 -25.0 70.0) disp=(60.0 25.0 70.0)`, so seat 1 sits 60 m left, **25 m
below** and 70 m behind, and seat 2 60 m right, **25 m above** and 70 m behind. That ±25 m is
exactly the entry-altitude split measured in section 5 (leader 1360, `.-2` 1340, `.-3` 1382). The
geometry is authored and faithful.

**What is not faithful is how the member gets there.** `place_wing_member_on_station_007f23a0`
ends with

```
for (int i = 0; i < 3; ++i) unit.motion.position[i] = station.world[i];
publish_pose(unit);
```

and that is its whole effect: it writes `motion.position`. Meanwhile the flight model integrates
two independent state variables (`src/game_hosts_units.cpp`, the plane step):

```
unit_.plane_world_velocity[i] += world_accel[i] * step;
unit_.motion.position[i]      += unit_.plane_world_velocity[i] * step;
```

`plane_world_velocity` has exactly two assignment sites in the file, both in the one-time seed
(`plane_velocity_seeded`); nothing anywhere re-derives it from a position delta. So for the
1100-1550 ticks a wing member spends in follow, **its position is overwritten every tick and its
velocity is never corrected to match.** The member appears to fly the leader's track at ~8 m/tick
because it is being teleported along it; its own velocity is free to drift anywhere the
accelerations take it, and nothing constrains it, because the teleport hides the consequence.

At the follow -> fly-over hand-over the teleport stops and the aircraft is released with that
velocity. That is one mechanism for both failure modes in section 1:

* released with too much downward and too little forward velocity -> path angle past the 55-degree
  line of section 4 -> the span grows, roll-in is never earned, and the fly-over's descent carries
  it into the sea (`movieval|.-2`, `D3A Val #1.1|.-2`);
* released off-axis -> the bearing error exceeds the `009C66E3` tolerance, which tightens to 20
  degrees as the span closes -> `flyabove_leave_792` fires, `009C85F5` sends it to `goaway`, and it
  re-enters the fly-over and aborts again: `D3A Val #3.1|.-2` (`transitions=6`, `goaway=1071`) and
  `D3A Val #7.1|.-2` (`transitions=32`) are in a fly-over/go-away limit cycle, not a stall.

It also explains the two things section 5 could not: why the leader (never teleported) and the
members diverge at all, and why `.-2` and `.-3` diverge with no principled difference between them
- an unconstrained variable does not need a reason to differ.

**Two controls already in the data, and they point the same way.**

* Run B's dive-bomb wing members were placed **once** (`once=true`, the member's first step) and
  then flew their own `attackrun`: position and velocity consistent throughout, and all fifteen
  aircraft completed. Run D's are placed **every tick** for 1100-1550 ticks.
* Run B's **torpedo** wing members were placed every tick for ~1000 follow ticks and came to no
  harm - because they never left follow. Torpedo drops are 12 in both runs. So being teleported is
  harmless while it continues; the damage appears at the **moment it stops**, which is exactly
  where a mechanism about an inconsistent released velocity predicts it.

So the blocker run D found is **not** downstream of follow in the image's law. It is this host's
placement substitution leaking an inconsistent velocity into the fly-over. That inverts the
predecessor's queue: run E - placement off, the follow law flying the member - is not a later
nice-to-have, it is the experiment that tests the actual cause.

## 8. Prediction for the 9000-frame pair, written before the runs

Same base `0920f88e8`, USN04, `--frames 9200 --press-start-frame 30 --menu-select USN04
--mission-frames 9000 --mission-frame-seconds 0.05`.

* `E1_pinned_9000.log` - the tree as it stands: mode pinned to 2, placement on.
* `E2_fed_noplace_9000.log` - two lines changed: `in.engaged.control_mode_370` fed from
  `slot.db_attack_mode_370`, and the **dive-bomb** follow tick calling the placement with
  `apply_position=false` so the follow law flies the member. The torpedo seam keeps placement,
  because the law is not wired into it and turning it off there would leave those members with no
  station-keeping at all.

E2 was chosen after section 7 was read and **before either run finished**; the original plan was
"fed alone at 9000", which run D has already measured at 4800 and which section 7 says would only
re-take D's result at greater length.

**What E2 is not, stated before it runs.** E2 is not "the image's follow". The follow law as wired
here writes `plan_heading_2c0` (via `009F9E40`) and `plane_commanded_pitch`/`plane_commanded_altitude`
(via `009FB800`) and **commands no speed**: the image's speed and station-holding live in the HOLD
arm `009BEE56`-`009BF9E5`, 997 unreconstructed instructions. So E2 is "heading and pitch commands
instead of a teleport". That asymmetry cuts one way only:

* If E2 removes the drowning and the limit cycle, section 7 is confirmed - the teleport's
  unconstrained velocity was the cause, and it was cured by the weakest possible replacement.
* If E2 is **worse**, that does **not** vindicate the teleport. A member with no speed control
  cannot hold station, so a worse E2 is consistent with either "the teleport was right" or "the
  missing HOLD arm is what the law still needs". Distinguishing those two needs the HOLD arm read,
  and this packet will say so rather than choose.

1. **E2's dive-bomb wing members reach `done` and release.** If section 7 is right, a member flown
   by the law rather than teleported enters the fly-over with a velocity consistent with its track,
   its path angle stays under 55 degrees, and it rolls in like its leader. I expect
   `states[follow=... flyabove=~100 turndown=~55 aimdive=~60 done=...]` and `releases >= 1` for the
   members that drowned in D.
2. **No dive-bomb wing-member water contact in E2.** This is the sharpest test of section 7. If a
   member still drowns with placement off, the velocity mechanism is not the cause and section 7
   is withdrawn.
3. **The fly-over/go-away limit cycle disappears.** `D3A Val #3.1|.-2` and `#7.1|.-2` come back
   with `transitions` in the normal 4-7 range instead of 6 and 32, and `goaway` in the tens rather
   than 1071 and 186.
4. **E2's total releases reach or beat E1's.** This is landing criterion (a). If they do, the mode
   can be un-pinned *together with* placement off for dive-bomb followers, and I will say so as a
   package rather than as a verdict on the one line.
5. **`follow law` lines stay > 0** (criterion e) and **mutual torpedo kills stay 0** (criterion d);
   the torpedo seam is untouched by both changes.

**A third outcome, and it is not a pass.** With no speed command the law may simply let a member
lag, so that its own range never closes to `approach+B8h` = 2080 m, its latch never sets, and it
stays in `follow` for the whole run: `states[follow=<everything>]` with no `flyabove` at all. That
would satisfy prediction 2 trivially - no fly-over, so no fly-over descent, so no water contact -
while releasing nothing. **If E2 shows that, it is a null, not a confirmation**, and it says the
HOLD arm of section 10 item 0 is the next read rather than that section 7 is right.

**Falsifier for section 7:** any dive-bomb wing member that still flies into the water in E2, or a
`.-2` member that still holds the fly-over past ~150 ticks. Either one says the velocity the
teleport leaves behind is not what loses these aircraft, and section 7 comes out.

**Falsifier for the packet's verdict on the pin:** if E2's releases reach or beat E1's with no new
water contact, the pin is not load-bearing once placement is off, and the honest recommendation is
to un-pin and turn dive-bomb follow placement off in the same change - not to keep the pin.

## 9. Measured

### E1, the pinned baseline at 9000 frames: it drowns sixteen aircraft

`local/E1_pinned_9000.log`, base `0920f88e8`, the tree exactly as it stands (mode pinned to `2`,
placement on). It reproduces run B's behaviour first: every dive bomber enters the fly-over from
`attackrun` at `alt` 1388-1401, `rng` 2073-2080, `span` 646-700, and all fifteen of B's aircraft
release 2 bombs each. `follow law` is **0**, as it must be - with the pin nothing enters follow.

**Then the mission keeps running, and the `done` state flies them all into the sea.** Every
dive-bomb `done` row is a descent that does not stop:

```
movieval|.-2      done ... ticks=950  placed=950  alt 278.5 -> 0.1
D3A Val #1.1|.-2  done ... ticks=1025 placed=1025 alt 274.5 -> 0.0
D3A Val #3.1|.-2  done ... ticks=965  placed=965  alt 208.8 -> 0.1
D3A Val #7.1|.-2  done ... ticks=918  placed=918  alt 277.0 -> 0.1
```

and the log carries **sixteen** `plane water contact` lines - five dive-bomb leaders, five dive-bomb
wing members, and six torpedo aircraft from `Lexington-class01_sqn01` and `Yorktown-class01_sqn02`.

**This reframes the whole comparison, and it is the single most important thing this packet
measured.** Run D's four water contacts at 4800 frames looked like a regression the pin did not
have. At 9000 frames the *pinned* configuration has sixteen. The pin was never the safe
configuration; 4800 frames was simply short enough to end the mission before the `done`-state
descent finished. Landing criterion (b), "no new water contact", cannot be judged against a 4800-
frame pinned baseline, and criterion (c), "a spent wing member in `done` stays inside the
leader-relative altitude band", **fails in the pinned baseline itself** - every wing member in
`done` ends between 0.0 and 30.2 m.

Prediction 2 of section 8 is confirmed in its own terms: the pinned run does gain releases from the
longer mission, **23 -> 30** dive-bomb releases (`summary mission dive-bomb task`, `aircraft=24`
rather than 15, because three more squadrons - `Yorktown-class01_sqn08`, `Zuiho-class01_sqn09`,
`Yorktown-class01_sqn13` - spawn later and sit in `attackrun` to the end). So E2's bar is **30**.

**A measurement discipline note, and I nearly broke it.** `summary mission gunnery torpedo_drop
drops=` reads **0** in E1 against 12 in A, B and D. That is not a collapse in torpedo releases: the
gunnery totals are since-the-last-`create_units` counts until `cc8-gunnery-host` lands its fix, and
a later `create_units` in the longer mission reset them. Every release figure in this section is
the dive-bomb **task** row, which lives on the units host and survives. No torpedo release number
from any 9000-frame run in this packet is usable.

### E2: launched, not read. Where it is and how to judge it

This packet closed on context before E2 finished. **The run was launched and its digest is written
automatically**, so the next reader inherits a measurement, not a to-do:

* `local/E2_fed_noplace_9000.log` - the run, on a binary built from base `0920f88e8` plus exactly
  two changes: `in.engaged.control_mode_370` fed from `slot.db_attack_mode_370`, and the dive-bomb
  follow tick calling `place_wing_member_on_station_007f23a0` with `/*apply_position=*/false`.
* `local/E2_digest.txt` - the reduced form, written by `local/e2_pipeline.ps1` when the run exits.
* `local/E1_digest.txt` - the same reduction of the pinned baseline, to compare against.

**Both changes are reverted in the tree**, because E2 was unread when the packet closed; the
`apply_position` parameter stays, defaulted to `true`, so it is inert. Re-applying them is two
edits, and both sites carry a comment naming E2.

**E2 does not test the velocity seed.** The integrator asked, before E2 was read, whether a wing
member that starts in `follow` and never flies `moveto` is seeded sensibly at all - because if it is
not, turning the teleport off measures the seed rather than the law. It is. The seed is in the
**unit-creation** path, not in any task state, so it runs for every plane regardless of which state
its task constructs into: `plane_world_velocity` is set to the class `TravelSpeed` (desc+`18Ch`,
141.666672 m/s for four rows in this installation) along the aircraft's **own forward axis**
`motion.pose_row2`, not a world axis. So every member of every squadron begins with the same
sensible forward airspeed along its spawn heading in E1 and E2 alike.

That sharpens the contrast rather than weakening it. With placement **on**, the seed is sensible and
then the velocity drifts free of the displayed motion for the 1100-1550 ticks the teleport is
running. With placement **off**, the same seed applies and the velocity is the thing that actually
moves the aircraft, so it cannot diverge from its own track. The two runs share the seed and differ
only in whether the velocity has to mean anything.

Judge E2 against section 8's pre-registered predictions, and in this order:

1. **Is it the NULL?** If the `.-2` members show `states[follow=<everything>]` with little or no
   `flyabove`, the law simply let them lag, their latch never set, and the run says nothing about
   section 7. That is not a pass. Go read the HOLD arm (item 0 below).
2. **Water contact.** Compare only against E1's **sixteen**, never against run D's four at 4800.
   The interesting number is dive-bomb wing members that drown *in the fly-over* (a descent from
   ~1340 m), not in `done` (a descent from ~280 m) - E1 shows `done` drowns everything given time,
   and that is a different defect.
3. **Dive-bomb releases against E1's 30**, from the `summary mission dive-bomb task` row only. The
   `torpedo_drop` figure in any 9000-frame log is a since-last-`create_units` count and is unusable.
4. **The limit cycle.** `D3A Val #3.1|.-2` and `#7.1|.-2` should come back with `transitions` in the
   normal 4-7 range instead of 6 and 32.

## 10. What the next reader should do, in order

0. **Read the HOLD arm `009BEE56`-`009BF9E5`.** It was item 3 on the last handoff and it is item 0
   now, because section 8's caveat cannot be resolved without it: the follow law this host wires
   commands heading and pitch and **no speed**, so "the law flies the member" is not yet a fair
   test of the image's follow. The gate is `009BEE49 CMP byte ptr [ESI+85h],0` / `009BEE50 JZ
   009BF9EA` - a member in good position takes the hold arm, one out of position jumps to the
   fly-to arm at `009BF9EA`. The arm opens with the lazy pose-matrix refresh pattern (`00414DB0`
   then `00B63D50` guarded by the `+10Ch` dirty byte) applied twice, then `0042D0D0` and x87. 997
   instructions. Call census in `docs/HANDOFF_PLANE_FOLLOW_REGIMES.md`.
1. **Wire the follow law into the torpedo follow seam.** `src/game_hosts_units.cpp:5600`,
   `follow_base_tick_009c1fd0`, calls placement and nothing else. Wiring it is not a one-liner: the
   two seams live in **different nested classes**, so the law helper
   `run_follow_law_009bfee0_009bee30` has to be hoisted out of the dive-bomb arm class onto `Impl`
   (or made a free function taking the slot, the station and the leader) before the torpedo seam
   can call it. Until it is, dive-bomb follow placement can be turned off and torpedo follow
   placement cannot - which is exactly the asymmetry E2 is built around.
2. **The fly-over's own arms.** `009C62B0` is `PARTIAL` by its own comment: only the heading is
   bound; the bank target, the altitude and the desired speed are unbound. Section 4's 55-degree
   trap is a property of the image's `009C65FD`/`009C67B0` pair and is real, but whether the image
   ever *reaches* that angle depends on arms this host does not command.
3. **`009C6270`'s unmodelled enter effects**, section 6: the `009C3DA0` aim-error draw, the
   `unit+844h` clear and `state+20h`. `009C3DA0` deserves a Ghidra name and a ledger record; this
   packet did **not** write one, because `009C3DA0` and `009C6270` are outside its lease and
   AGENTS.md puts Ghidra writes behind the lease. It is owed.
4. **Phase A `009C0251`-`009C0EE0`** and the **`009C1552` subtree**, both still untouched.

## 11. E2 read (packet cc9_follow_package)

Inputs, read-only, in the `cc8-follow-attack` tree: `local/E1_digest.txt`, `local/E2_digest.txt` and
`local/E2_fed_noplace_9000.log` (9000 mission frames, same reduction as E1). Every figure below was
re-read from the digests and checked against the log's own rows; none is inherited.

**Verdict: E2 is a FAIL, not the null.** The package (feed `control_mode_370` from
`slot.db_attack_mode_370` and turn dive-bomb follow placement off) must not land. Both changes stay
reverted in main.

| rule | E1 (pinned, placement on) | E2 (fed, placement off) |
| --- | --- | --- |
| 1. null? | nothing enters `follow` | not the null: all 8 wing members of the four `D3A Val` squadrons reach `flyabove` via `follow>flyabove` at `rng` 2070-2079; the other 8 wing members (`movieval` and the three late squadrons) never leave `follow` and drown there |
| 2. water contacts | 16 | **24** |
| 3. releases (`summary mission dive-bomb task`) | 30 | **20** |
| 4. `D3A Val #3.1\|.-2` / `#7.1\|.-2` transitions | 7 / 13 | **23 / 12** |

**Rule 2 in detail.** E2's contact set contains all sixteen of E1's units plus eight more:
`movieval|.-3`, `D3A Val #3.1|.-3`, and both wing members of the three late squadrons
(`Yorktown-class01_sqn08`, `Zuiho-class01_sqn09`, `Yorktown-class01_sqn13`). No member drowns
**in the fly-over**: the fly-over altitude rows end with `err last` 62-690 m and no contact. The
drownings are in two other places.

* **In `follow`, eight members, none of which ever left it.** `movieval|.-2` also drowned in E1, but
  in `done` after 950 ticks; in E2 it drowns in `follow` after 349. The other seven are new.

  | unit | follow ticks | spawn seed m/s | ownY at n=1 m | cmdalt at n=1 m | |v| at contact m/s | range closed m (leader) |
  | --- | --- | --- | --- | --- | --- | --- |
  | movieval\|.-2 | 349 | 66.67 | 675.0 | 710.0 | 47.19 | 330 (10842) |
  | movieval\|.-3 | 509 | 66.67 | 725.0 | 725.4 | 51.63 | 1158 (10842) |
  | Yorktown-class01_sqn08\|.-2 | 330 | 69.44 | 150.1 | 160.1 | 57.86 | 1087 (3984) |
  | Yorktown-class01_sqn08\|.-3 | 307 | 69.44 | 150.1 | 160.1 | 57.64 | 971 (3984) |
  | Zuiho-class01_sqn09\|.-2 | 215 | 66.67 | 125.0 | 160.0 | 46.81 | -87 (2615) |
  | Zuiho-class01_sqn09\|.-3 | 291 | 66.67 | 175.0 | 175.3 | 47.53 | 189 (2615) |
  | Yorktown-class01_sqn13\|.-2 | 333 | 69.44 | 150.1 | 160.1 | 57.88 | 1097 (3190) |
  | Yorktown-class01_sqn13\|.-3 | 307 | 69.44 | 150.1 | 160.1 | 57.66 | 974 (3190) |

  Seed is the `plane spawn` row's `seed` magnitude; `ownY`/`cmdalt` are the `follow law` row at
  `n=1`; range closed is the `ordered` row at mission end, with the squadron leader's figure in
  brackets. Every member drowns 11-20 m/s **below** its own spawn seed, and every member closes about a
  third or less of its leader's range. The members are lagging their leaders and sinking while the
  law commands a climb or a hold. The one mid-flight sample in the log says the same:
  `movieval|.-3 n=401 R=776.3 ... cmdalt=1160.7 ownY=492.3 spd=103.88`: 776 m behind the station,
  233 m below its spawn height, commanded 668 m above where it is.

  **What the log cannot show.** The brief asked for altitude and speed over each member's last
  ~100 ticks. The log has no per-tick plane rows for units in `follow`; the `follow law` row prints
  at `n=1` and `n=401` only (39 rows in the whole run). The descent profile is therefore bounded by
  the two samples and the contact row, not traced. A re-run that wants it needs a per-tick follow
  row for these eight units.

* **On entering `done`, two members, at dive speed.** `D3A Val #3.1|.-2` (`aimdive -> done` at
  tick 2402, alt 326.9) and `#7.1|.-2` (`aimglide -> done` at tick 2872, alt 236.9) each log their
  water contact on the very next line at alt -18.01 and -35.77, |v| 139.61 and 133.82, with
  `done ... ticks=1 placed=1`. A one-tick fall of 273-345 m is not flight; it is `done`'s own
  placement (still on in E2) moving the member. `#7.1`'s leader had already drowned (log line
  64733, before the member's `done` at 78477). This is the `done` defect of section 9, reached
  faster, not a fly-over drowning.

**Rule 4.** `#3.1|.-2` went from 7 transitions to 23 (`goaway=1025`, `turndown=115`, `aimdive=102`)
and `#7.1|.-2` from 13 to 12 (`goaway=1330`). Neither is in the 4-7 range. Two further members,
`#1.1|.-3` and `#5.1|.-3`, cycle 18 and 13 times, spend 2442 and 1753 ticks in `goaway`, and release
nothing. The limit cycle is not cured; it spreads.

**Against section 8's falsifiers.** Section 7's falsifier ("any dive-bomb wing member that still
flies into the water in E2") fires, but not in the form it was written for: no member drowns in
the fly-over, so the velocity-left-behind-by-the-teleport story is neither confirmed nor refuted by
the fly-over. The pin falsifier does not fire: releases fall from 30 to 20 and contacts rise from 16
to 24.

**Decision.** Do not land the package. The eight follow-only drownings are the direct symptom of
section 10 item 0: this host's follow law commands heading and pitch and **no speed**, so a member
that must fly its own track bleeds speed from its seed, falls behind, and sinks. The next step is
the HOLD arm read (`docs/PLANE_FOLLOW_HOLD_ARM.md`), and specifically its speed command, before any
further placement-off experiment.

**Where the "no speed" of section 8 actually is, checked in this packet.** The host's fly-to
binding (`run_follow_law_009bfee0_009bee30`) does compute the image's speed and store it in
`plane_desired_speed_2b4`; that write has been in the tree since `6fe098150`, so E2's binary had
it. What it does not do is the image's next two stores:

```
009BFD0F  FSTP float ptr [EBX+2B4h]     ; desired speed          <- host writes this
009BFD15  MOV  byte ptr [EBX+2B0h],0                            <- host does not
009BFD1C  MOV  dword ptr [EBX+2D8h],1   ; speed-demand mode      <- host does not
```

The host's `0099D300` throttle rule (`pilot_plan_throttle_0099d300`) enters its demand arm only
when `+2D8h` is 1, or when the flight state is 5; an airborne plane is in state 7. A wing member
that constructs straight into `follow` never flies a state that raises `+2D8h`, so its desired
speed is computed and never read. "Commands no speed" is true of the host, but the gap in the fly-to
arm is two missing stores, not an unread law. It is not fixed here: the brief forbids wiring
the follow seams without a same-binary control run, and the HOLD arm, which a member on station
takes instead, is still the larger unread piece.

## 12. The fly-to speed stores, measured (packet cc9_follow_speed, 2026-09-22)

`docs/PLANE_FOLLOW_SPEED.md` has the stores, their readers, the predictions, the runs and the
decision. In short:

* The host now performs the fly-to arm's `009BFD15` (`plan+2B0h = 0`) and `009BFD1C`
  (`plan+2D8h = 1`). On the default configuration this is identical to main (runs A and B, USN04
  at 9000 frames): 30 releases, 16 water contacts, every summary and per-unit row equal.
* In the E2 configuration the stores change nothing (runs D and C are identical). A log-only
  trace shows why. The follow-only members already fly at full throttle, and the stores only
  confirm it. They drown in a **climb stall**. The host's pitch substitute (`009FB800` with the
  commanded altitude as its own reference, standing in for the unread `009F9ED0`) holds the
  class climb angle, 0.698 rad, while speed decays from about 67 to 20 m/s. Then they fall.
* So section 11's "the law commands no speed" was true of the host, but it is not what drowns
  the members. The E2 configuration stays reverted. The next read is `009F9ED0`.

## 13. The fly-to pitch through 009F9ED0 (packet cc9_follow_pitch, 2026-09-22)

`docs/PLANE_FOLLOW_PITCH.md` has the read, the runs and the decision.

* `009F9ED0` is `min(pi/2 - atan2(dist, altErr), desc+1E4h)`: the elevation angle of the
  steer altitude over at least `FollowedPointDist`, capped at the class's sustainable climb.
  It is stored to `plan+2BCh` with the pitch mode `plan+2D0h = 2`. The host's stand-in fed
  `009FB800` an altitude as its dimensionless `reference`, which made it bang-bang, and never
  wrote the mode.
* Bound behind `kPlaneFollowFlyToPitch = true`. On the default configuration it is identical
  to main (A' and B2).
* In the E2 configuration (C2) no member drowns in `follow` any more. Releases go from 20 to
  28 and water contacts from 24 to 16, the same 16 units as the pinned baseline. The E2
  configuration still does not land, because releases are 28 against 30 and `#3.1|.-2` /
  `#7.1|.-2` cycle 12 / 14 times. What remains is `#3.1|.-2`'s dive: an aim error outside the
  gate, then a 166 m breakoff it cannot pull out of. That belongs to the aimdive and goaway
  arms.

## 14. Phase A of the follow geometry (packet cc9_follow_phase_a, 2026-09-22)

`docs/PLANE_FOLLOW_PHASE_A.md` reads Phase A (`009C0251`-`009C0EE0`) at block level. It is an
intercept planner: for each heading quadrant it plans a rejoin turn of radius
`TravelSpeed / (TurnMul * classDesc+270h)`, then iterates the arcs until they fit.
* `p` is where the member ends along-track relative to its moving station.
* `e` is its cross-track error there.
* `e > 0.05 p` or `p < 0` selects the `009C1328` lead-in, otherwise lead pursuit.
* A failed arc test selects abeam by the side of the track.
* `base-0Ch` leaves as the manoeuvre time, so the dispatch's lead distance is the leader's
  travel during it.

Nothing was bound: the four quadrant planners need a per-path verified transcription first.
Main at `0af2f50eb` is neutral on USN04 (control A3 equals B2).

## 15. The six dogfight fighters (packet cc9_dogfight_task, 2026-09-23)

USN04's six fighters (`Lexington-class01_sqn01` ×3, `Yorktown-class01_sqn02` ×3) carry a
scene-issued `dogfight` order. Until this packet the host built no task for them, and on main
`0899b3bb2` four of them drowned at |v| 82. `docs/DOGFIGHT_TASK.md` maps the kind-2 task (eight
states, the arm `009AB1C0`, the transitions `009AAFA0`) and binds a skeleton: the leaders in
moveto (a labelled stand-in that holds CruisingAlt 1400 through `009F9ED0`), the wing members on
the generic follow tick. No fighter drowns now (water contacts 11 -> 7), and the dive-bomb rows
are unchanged. The engaged states are not bound.
