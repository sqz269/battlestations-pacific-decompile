# Ship station keeping: 009ED6B0's station arm and 009F4DA0's brain+3ADh arm

Packet `cc9_station_keeping` (2026-09-23). Offsets are relative to blk = brain+8h unless written
as brain+N. Names are hypotheses. The reconstruction is `src/ship_ai_station_keeping.cpp`, and
the host binding sits behind `kShipStationKeepingBound` in `src/game_hosts_ship_ai.cpp`.

## What the image does

1. **Request, 009DA3B0.** The follower step 009E1610 ends by copying a nineteen-byte request into
   blk+38Ch..+3A6h: the direction x and z, the heading +394h, the leader speed +398h, the zero at
   +39Ch, the radius +3A0h, the making-way byte +3A4h, enable +3A5h (= brain+3ADh, always 1)
   and suppress +3A6h (= the out-of-station latch). So the arm is live exactly while a follower
   is inside its station latch.
2. **Pre-pass clear, 009F145E.** 009F1420 writes `MOV byte [brain+3ADh],0` every think, so the
   enable lasts only for thinks where the follow step published it.
3. **The station arm, 009EDA28..009EE57B.** It runs when blk+3A5h is set and blk+3A6h clear
   (009EDA34 / 009EDA41) and leaves through JMP 009EF206 with AL = 0. In order:
   - the braking distance b = 0.5 * ref * ref / Retardation (unit class +508h);
   - along = d . dir and across = d x dir, with d the station point blk+1DCh/+1E0h less the hull
     position blk+184h/+188h;
   - herr = wrap(request heading - unit heading);
   - the ahead/astern latch on blk+35Ch from the making-way byte, gated by blk+360h < 0; a flip
     sets +384h = 0, +360h = 1.0 and +374h = -1.0;
   - v = deadband(clamp(leader speed / ref, -1, 1), 0.1, span 0.9), and the along target
     2*v*b + along;
   - blk+388h, the 80/100 degree cone; the steer limit from acos and atan + 10 degrees;
   - blk+38Ah, close to the station line (10 degrees and 2 widths to enter, 15 degrees and
     3 widths to stay);
   - **blk+39Ch, the throttle command**: base = leader speed / ref; along shifted by the radius;
     band = max(length / 3, 50); outside the band, base + (along -/+ band) / b; zero when
     |value| < 0.05; then one of six output branches writes blk+324h (heading target), sets
     blk+32Ch = 1500 and clamps blk+39Ch through the cosine of the steer term.
4. **009F4DA0's brain+3ADh arm, 009F4F8C..009F5020, follower branch only.** v = brain+3A4h
   (= blk+39Ch). Ahead: v < 0 gives limit344 = -v and brain+374h (= blk+36Ch, the escape byte) = 1,
   else limit344 = v and the byte 0. Otherwise v > 0 gives limit344 = v and 1, else -v and 0. Then
   limit344 = min(limit344, s) with s = clamp((ceiling + 6.70421028) / ref, 1.0, 1.25).
5. **The drive, 009F443F..009F448A.** When blk+36Ch disagrees with the committed direction the
   throttle is capped at limit344. With a follower going ahead and a positive command the byte
   is 0, which is that mismatch, so the follower's throttle becomes min(ceiling, blk+39Ch).

## Correction to docs/SHIP_FORMATION_SPEED.md

That document said the request's zero at brain+3A4h would set the limit to +-0. It does not:
the station arm overwrites blk+39Ch with the computed command on the same think, before
009F4DA0 reads it.

## Host before this packet

The request was recorded and dropped (`ShipAiFollow::publish_station_request`, UNIMPLEMENTED,
1200 calls on USN01 3000 and 10056 on USN04 4500 in the formation-speed treatment logs), so the
gate never opened (`station_keeping=0`) and every follower kept limit344 = 1.0 and steered by the
navigation goal alone.

## Predictions, written before any run

Pairs are same-tree builds that differ only in `kShipStationKeepingBound`, with
`BSP_GUNNERY_RNG_STREAMS=1` on both sides.

1. Station requests equal the control's publish counts (1200 USN01, 10056 USN04); arm runs are
   at most that and near it, since followers spawn on their stations.
2. Followers' limit344 leaves 1.0: USN01 Convoy2..6 follow Convoy1, whose own limit is 0.5879,
   so their commands sit near that ratio, inside [0, 1.25]. Escort followers (Northampton,
   Dunlap, SaltLakeCity, Ralph, McCall, Blue) take values near their leaders' speed ratios.
3. Followers stop outrunning their leaders: the formation column residuals shrink and fewer
   followers fall out of the station latch, so requests with suppress set become rarer.
4. Leaders and `role=none` ships are unchanged up to position coupling.
5. The escape byte now reaches 0 on followers each station think; the drive's request-2 path
   can fire when the committed direction disagrees with the latch. Expect no reversing on a
   steady column; any reversing is a finding.
6. Gunnery rows move only through positions; with split RNG streams, small moves only.
7. Unimplemented: `ShipAiFollow::publish_station_request` and `ShipAi::station_keeping_arm`
   become concrete, which lowers the unimplemented call total by their counts.

## First pair: inert, and why

The first USN01 pair (skC/skT) ran 1200 enabled requests, all from Dunlap and SaltLakeCity, and
**zero** arm runs and zero 009F4DA0 station steps. Two things kept the arm shut:

- Dunlap followed Enterprise from about 8270 m away, so its station latch was out and every request
  carried suppress = 1. That part is faithful: the arm runs only inside the station latch.
- 009ED73F (the direct-control arm at the head of 009ED6B0) clears blk+3A5h unless 007788B0
  answers true, and the host's binding answered a recorded false. 007788B0 is
  `g = [unit+284h]; g && [g+14h] != unit` (listing 007788B0..007788C7). So the enable never
  survived to the gate or to 009F4DA0, even for a follower in station. **This was the missing
  term.** It is now bound behind the same switch.

Two more writers of the request block run on the same controller step and need no host change.
009DDBC0 (009F51AE, after each replan) copies blk+38Ch..+3A6h to blk+3A8h..+3C2h and the goal span
blk+1C4h..+1F0h to +1F4h..+220h. 009DA0D0 (009F51B7, every hold think) copies both back. So the
request, enable included, lasts until the next replan's pre-pass clear. The host's fields already
persist that long because nothing else writes them, and the arm overwrites +39Ch before
009F4DA0 reads it.

**Consequence the second pair tests.** 009F4DA0's brain+3ADh branch does not look at suppress.
An out-of-station follower keeps enable = 1 and blk+39Ch = 0 (the request's zero, restored every
hold think), so limit344 = 0 and the escape byte = 0. Whether that stops the ship depends on the
drive's mismatch branch.

Predictions for the second pair, written before it ran:

1. Station steps (009F4DA0) for every follower that runs the follow state. limit344 = 0 while out
   of station, and blk+39Ch-derived values inside it.
2. Out-of-station followers whose committed direction disagrees with the escape byte get their
   throttle capped at 0, so they slow or stop instead of closing on their station. If they do,
   that is the image's behaviour under this host's state mix. It is not a host error unless the
   follow state is reached where the image would not reach it.
3. The arm runs only for followers inside the station latch. On USN01 that may be none.

## Secondary reads

**0082E850's descriptor test.** The body is `class+520h`, times `[00424C40()+438h]` unless
`descriptor->vtable[18h](0Eh)` answers true (0082E866, JNZ 0082E87D skips the multiply). A byte
scan for `8B 44 24 04 83 F8 0E` finds three predicates: 0075D350 (a session message type, not a
descriptor), 00857DC0 (the torpedo-boat instance, kinds 0Eh/6/5/4/2/1) and 00963E90, the
torpedo-boat class descriptor predicate (0Eh/6/5/4), whose only reference is its vtable slot
00D1AE90. The base ship class 009635E0 (vtable 00D1ACC4, slot 00D1ACDC) answers 6/5/4 only, and
the landing ship class 00963C80 answers 0Ch/6/5/4. So only torpedo boats skip the multiplier, and
every other ship's turn radius is class+520h * 2.0. The host always multiplies. A faithful fix
is one call, `units.unit_is_kind_of(unit, 0x0E)`, which dispatches the instance vtable; its
0Eh chain matches the descriptor's on 6/5/4. That substitution was not applied or run here.

**settings+438h = 2.0.** The getter at 0083D104 reads `Navigator.TurnMultipliers.
TurnMultiplierMaxSpeed[2]` (docs/GAMEPLAY_SETTINGS.md). This installation's
`scripts/datatables/shipglobals.lua` (mtime 2024-07-13) sets it at line 342 as `{ 1.0, 2.0 }`,
whose Hungarian comment reads "at which throttle setting and by how much the vehicleclass turn
radius is multiplied; first the throttle, second the multiplier". Line 400 assigns the same key in
`ShipGlobals["Fordulas"]`, a different table, also to `{ 1.0, 2.0 }`. The host's 2.0 matches the
file, and it stays a labelled constant because the settings object is not loaded.

## Second pair: results (USN01 3200/3000, `BSP_GUNNERY_RNG_STREAMS=1`)

Control `build/win32/skC` (switch off), treatment `build/win32/skT2` (switch on), same tree.

**The limit is not zero while out of station.** The path branch 009EE5BA pins blk+39Ch at 1.25
(00CF29A8, `kShipAiPathPickSpeedScale`) whenever the station arm does not run. So an
out-of-station follower's limit is min(1.25, s) = s, and the drive hands the ceiling
max(1.0, s). The zero predicted above was wrong: the follower catches up at up to s times its
reference, which caps it at the group's slowest MaxSpeed + 6.70421028. The station arm, run
inside the latch, then sets the station-keeping command.

| row | control | treatment |
|---|---|---|
| station arm runs (Dunlap / SaltLakeCity) | 0 / 0 | 5 / 5 |
| 009F4DA0 station steps (each) | 0 | 3000 |
| Dunlap limit344 | 1.0 | 0.5000 .. 1.2174 |
| SaltLakeCity limit344 | 1.0 | 0.0000 .. 1.2500 |
| Dunlap / SaltLakeCity throttle, step 10 onward | 1.000 / 1.000 | 1.217 / 1.250 |
| arm-run throttle39c (Dunlap) | - | -5.0110 .. 0.5000 |
| navigate-mode steps | 10395 | 9702 |
| gunnery assigns | 451 | 1335 |
| gunnery shots / hits | 263 / 126 | 329 / 153 |
| deaths / total damage | 5 / 4467.9 | 5 / 4467.9 |
| unimplemented calls | 2704228 | 2703105 |

Against the predictions:

1. Station steps on every follow-state follower: **met** (Dunlap and SaltLakeCity, the only two
   in the follow state on USN01; Convoy2..6 and the Enterprise escorts never ran it).
2. A zero cap on out-of-station followers: **wrong**, for the 009EE5BA reason above. The effect
   is the opposite: followers run at 1.217 and 1.25 instead of 1.0.
3. The arm runs only inside the latch: **met**, 10 runs in total. Each follower was in its latch
   for five thinks, and SaltLakeCity's command there was 0 (limit344 = 0 for those steps).

The same five aircraft die on both sides (Mav1..5). Mav5, Mav1 and Mav4 die at 105.85, 106.20 and
111.15 s instead of 62.85, 76.95 and 65.25 s: the escorts sit elsewhere, so the AA engages later.
With split RNG streams those moves come from positions, not from draws.

The direct unimplemented delta is `ShipAiFollow::publish_station_request` (1200 calls) becoming
concrete. 007788B0 in the direct-control binding is reached only while the flag is set, so the
control never calls it. The rest of the total's change follows from the moved positions.

USN04: the second-pair treatment died at the renderer init request at 12:26 (0xC0000005,
session rdp-tcp#0 active), before any ship AI ran. Its control on another slot ran normally. Per
the standing instruction it was not retried. The first USN04 pair (the inert build) ran 10248
station requests on each side and no arm runs.
