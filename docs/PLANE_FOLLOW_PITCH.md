# The follow law's pitch: 009F9ED0 read and bound

Addresses: 009F9ED0 (whole body 009F9ED0-009F9F7C), 009BEE30 (call site 009BFC21 and its
arguments 009BFBBE-009BFC1E), 0099D300 (the pitch-mode gate 0099E3BF-0099E3D1).

Packet `cc9_follow_pitch`. Follows `docs/PLANE_FOLLOW_SPEED.md`. That packet found that the
follow-only members of the E2 configuration die in a climb stall under the host's `009FB800`
stand-in for this routine.

## 1. What 009F9ED0 computes

`__thiscall(this, float a, float d)`, `RET 8`. Ghidra's body is `009F9ED0`-`009F9F7C` and it has
no locals beyond the two argument slots it reuses.

```
009F9ED0  FLD  [ESP+8]            ; d, the second argument
009F9ED4  PUSH ESI
009F9ED5  FLD  [ESP+8]            ; a, the first argument (ESP moved by the push)
009F9EDB  CALL 00BF701A           ; _CIatan2, x87 arguments: ST1 = d, ST0 = a -> atan2(d, a)
009F9EE8  FSUBR double [00CE3830] ; t = pi/2 - atan2(d, a), stored as a float over d's slot
009F9EF8  0 > t  ->  t += double [00CE3828] (2pi), float store
009F9F24  t > double [00CE3D28] (pi)  ->  t -= 2pi, float store
009F9F3C  ECX = [this]; EAX = [ECX+8]; cap = [EAX+1E4h]   (float, stored over a's slot)
009F9F59  result = cap > t ? t : cap                        (an UPPER cap only)
009F9F65  ECX = [ECX+18h]
009F9F68  MOVSS [ECX+2BCh], result
009F9F70  MOV dword [ECX+2D0h], 2
009F9F7A  RET 8                   ; EAX = &desc+1E4h, incidental
```

Constants, read from the image at their load widths: all three are doubles,
`[00CE3830]` = 1.5707963705062866, `[00CE3828]` = 6.2831854820251465 and `[00CE3D28]` =
3.1415927410125732. These are pi/2, 2pi and pi rounded to float, then widened.

So for `d > 0` the result is **`min(atan(a / d), desc+1E4h)`**: the elevation angle of a point
`a` metres above the aircraft at horizontal distance `d`, never steeper than the class's climb
cap. There is no lower cap.

* **`this`.** At `009BFC0A MOV ECX,EDI`, `EDI = state+4`, so `[this]` is the approach.
  `approach+8` is the class descriptor and `approach+18h` the pilot plan, the same two the
  whole follow step uses.
* **`desc+1E4h`.** Its producer is the class loader: `007C4BC5` sets `desc+1E4h =
  007D98F0(desc, LevelFlight * desc+184h StallSpd)`, the bisection for the steepest climb the
  aircraft can hold at that speed (`include/bsp/plane_flight.hpp`, the `007D98F0` note). The host
  already carries it as `plane_climb_angle_1e4`. `desc+1ECh` = 0.6 × it is `009FB800`'s gain.
* **`plan+2D0h = 2`.** This is the pitch mode that the planner's gate `0099E3BF`-`0099E3D1`
  tests: non-zero runs the pitch arm `0099E490`-`0099E739` toward `plan+2BCh`. It is already
  modelled at `src/game_hosts_units.cpp` ("THE PITCH-MODE GATE").

**The fly-to call's arguments** (`009BFBBE`-`009BFC1E`):

* `a = cmdAlt - unit+100h`, the blended commanded altitude of `docs/PLANE_FOLLOW_LAW.md` §5.2
  minus the member's world Y.
* `d = max(dist, block+00h)`: `009BFBD0 FCOMIP` against `FollowedPointDist` (250 m in this
  installation).

The host's fly-to rule already computes both, as `altitude_error` and `command_distance`.

**The callers.** `tools/callsite_census.py` finds four call sites, not three:

* `009BFC21` in `009BEE30`, the fly-to arm;
* `009A78BB` in `009A76E0`;
* `009CAC2D` in `009CA870`;
* `007B4D3D` in `007B4980`-`007B4D7F`. This routine has no Ghidra function: `ghidra proto
  007B4D3D` finds none, and Ghidra's `007B48E0` ends at `007B497E` with INT3 at `007B497F`. It is
  referenced only from `.rdata 00D057C4`. The census tool labels this call `007B48E0`.

The last three are unnamed and their bodies were not read.

## 2. Why the host's stand-in was bang-bang

The fly-to binding turned `cmdAlt` into a pitch through `pitch_command_009fb800` and passed
`cmdAlt` itself as `reference`. `009FB800`'s `reference` is a dimensionless ramp: the torpedo
run's `009D0A63` feeds it values in [0.35, 0.8]. It is both a multiplier on the error and the
ceiling on `t`. Fed an altitude of hundreds of metres, `t = err * (ref+1)/2 / ClimbDist` passes
the cap for any error above a few metres. The demand then sits at `max(desc+1ECh*1.6, DEG(40))`
= 0.698 climbing or `max(DropAngle*1.6, DEG(60))` = 1.047 diving. That is what
`docs/PLANE_FOLLOW_SPEED.md` measured: 308 of 516 trace rows at +0.698 and 187 at -1.047.

The binding also never wrote `plan+2D0h`. The planner's pitch arm therefore ran or did not run
depending on whatever state last wrote the mode.

## 3. The reconstruction and the binding

* `bsp::pitch_command_to_point_009f9ed0(altitude_error, distance, class_climb_angle_1e4)` in
  `include/bsp/plane_flight.hpp` / `src/plane_flight.cpp`: a pure function beside
  `heading_command_009f9e40`, with the image's float stores and double constants.
* `run_follow_law_009bfee0_009bee30` now sets `plane_commanded_pitch` and
  `plan_state.pitch_target_2bc` from it with `cmd.altitude_error` and `cmd.command_distance`,
  and sets `plan_state.pitch_mode_2d0 = 2`. This is behind `kPlaneFollowFlyToPitch` (default
  true). The `009FB800` path is kept in the `else` branch for the same-binary pair.

## 4. Predictions, written before runs B2 and C2

Control A' was already running when this section was written.

* **B2 against A'.** In the default configuration nothing enters `follow` (`follow law` rows =
  0 in A and B), so the binding never runs. Prediction: identical on every summary and per-unit
  row.
* **C2 against D (the E2 configuration without the stores or the pitch):**
  1. The follow-only members no longer stall. The commanded pitch is a real elevation angle
     over at least 250 m, capped at a climb the class can hold at LevelFlight * StallSpd.
     Their speed stays near the seed (above about 50 m/s in the trace), and most of the eight
     no longer drown in `follow`.
  2. They close on their leaders. More of them reach the 2080 m latch and the fly-over, so
     releases rise above D's 20. They cannot exceed 30 plus the eight members' 16 bombs.
  3. The two one-tick `done` placement drownings of handover section 11 (`#3.1|.-2`,
     `#7.1|.-2`) and E1's `done` descents are unaffected, because this law does not run in
     `done`. Water contacts therefore do not fall below A's 16 plus those two.
  4. The `#3.1|.-2` / `#7.1|.-2` transitions: no confident prediction. They are driven by the
     fly-over and goaway arms, which this packet does not touch.
* **Falsifier.** If the trace still shows the pitch pinned at a cap while speed decays, the
  climb cap `desc+1E4h` is not what keeps the aircraft flying, and the planner's pitch arm
  (`0099E490`, whose mode-2 hold of `unit+C84h` the host does not model) is next.

## 5. Runs

All runs use USN04 with `--frames 9200 --press-start-frame 30 --menu-select USN04
--mission-frames 9000 --mission-frame-seconds 0.05`, each from its own copied binary.

| run | configuration | binary | log | releases | water contacts | `#3.1\|.-2` / `#7.1\|.-2` transitions | `follow law` rows |
| --- | --- | --- | --- | --- | --- | --- | --- |
| A' | main | `local\binA2` | `local\A2_default.log` | 30 | 16 | 7 / 13 | 0 |
| B2 | main + pitch | `local\binB2` | `local\B2_default_pitch.log` | 30 | 16 | 7 / 13 | 0 |
| D | E2 configuration, no stores, no pitch (packet `cc9_follow_speed`) | `local\binD` | `local\D_e2cfg_nostores.log` | 20 | 24 | 23 / 12 | 39 |
| C2 | E2 configuration + stores + pitch, trace every 25 ticks | `local\binC2` | `local\C2_e2cfg_pitch.log` | 28 | 16 | 12 / 14 | 48 |

* **A' equals A.** The digest is identical to `local\A_default_nostores_digest.txt`, so the two
  packets merged since then are neutral on USN04.
* **B2 equals A'.** The digest, all 84 `summary mission` rows (refills excluded) and every
  `follow law`, `water contact`, `release census`, `db aim exit` and `divebomb` row are
  identical. The landing build's `.text` is byte-identical to `local\binB2`. Prediction B2 holds.
* **C2 against D:**
  1. **No member drowns in `follow`.** The six late-squadron members survive to the end of the
     mission (`arm_ticks` 809/779/689, equal to their leaders'). The `movieval` pair now
     attacks and releases 2 bombs each. Over all 699 trace rows the pitch command is
     continuous: 153 rows at 0.00, the rest small, and none pinned at +0.698 or -1.047.
     Speed does not collapse, but it is not held near the seed either. The late members climb
     at 0.28-0.31 rad, which is `desc+1E4h`'s regime, at 38-40 m/s against a 66.7 m/s seed,
     trailing a leader that climbs faster. Prediction 1 holds for the stall and misses its
     50 m/s threshold.
  2. **Releases rise from 20 to 28.** Every `D3A Val` member and both `movieval` members
     release 2. The only member that releases nothing is `#3.1|.-2`. Prediction 2 holds.
  3. **Water contacts fall from 24 to 16, and the set of units equals A's exactly.** Twelve are
     the `done` descents and torpedo aircraft of E1. `#7.1|.-2` drowns in `done` at 69.02 m/s,
     not by the one-tick placement snap. `#3.1|.-2` is the exception. It enters the dive,
     leaves `aimdive` for `goaway` at 166 m without releasing (aim error -91.2 m against the
     25 m gate), and drowns 24 ticks later at 124.5 m/s. Prediction 3 missed on the snap: that
     path is not reached, because the members no longer arrive at `done` the way E2's did.
  4. **Transitions:** `#3.1|.-2` 12 (D 23, A' 7) and `#7.1|.-2` 14 (D 12, A' 13; its leader
     `#7.1` also has 14). Every other wing member shows 4.

## 6. Decision

* **The pitch binding lands**, `kPlaneFollowFlyToPitch = true`. B2 is identical to A', and
  the image's routine replaces a stand-in that ran bang-bang.
* **The E2 configuration does not land.** C2 against A' passes section 8's first criterion:
  the water-contact set is the same 16 units. It fails the other two. Releases are 28 against
  30. The transitions are 12 and 14 against the 4-7 range, though A' itself has 13 for
  `#7.1|.-2`. Both E2 edits are reverted again. The trace row stays compiled out.
* **What the members still lack.** It is no longer anything in `follow`. Two pieces remain:
  1. `#3.1|.-2`'s dive. Its aim error is outside the release gate, and it cannot pull out of a
     166 m breakoff. That is the aimdive and goaway arms, owned this round by
     `cc9_aimdive_response` and `cc9_goaway_turn`.
  2. The late members' climb rate. They hold `desc+1E4h`-limited climbs at about 40 m/s,
     behind a leader climbing to cruise.
  The HOLD arm rule is **not** the missing piece yet. It applies only within 100 m of the
  station, and these members are climbing to it.
* The fourth caller of `009F9ED0` (`007B4980`) and the callers `009A76E0` and `009CA870` still
  use whatever their own states do; none of them was touched.
