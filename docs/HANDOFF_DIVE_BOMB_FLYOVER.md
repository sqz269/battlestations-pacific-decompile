# Handoff: the fly-over's flags are closed, and the second attack run is one gate away

Packet `cc8_dive_flyover`, branch `agent/cc8-dive-flyover`. Read
`docs/DIVE_BOMB_FLYOVER_FLAGS.md` whole first - it is this packet's evidence and it is written to
be read in order. This file is only what that document does not say.

## (a) Settled, and not to be re-derived

* **`flyabove+1Bh`** is the squadron's old-style-bombing flag, latched at `009C6813`. Both of the
  210 m clamp's skips are the same scripted switch read twice, it is 0 in this installation, so the
  clamp is unconditional and `L` is `pi/2` everywhere. Four writers image-wide, three constant zero.
* **`flyabove+1Ch`** is the roll-in latch (`cos|E| * R <= 120 m` inside one turn circle). Bound, and
  **inert on USN04**: the fly-over hands over at 914-974 m along-track, seven times the gate.
* **`T`**, the dead band, has all three producers read, including `009C6911`'s
  `InterpolateClamped(0, 100 deg, TurnCircleRadius, 10 deg, R)`.
* **`BL`**'s veto at `009C64EC` is `aircraft->IsKindOf(RECON_PLANE)`; kind `14h` is the recon plane
  on two independent scans. Bound, not substituted.
* **`009C6180`** is defined and named `BSP_BotStateDiveBombFlyAbove_Construct` (provisional).
* **USN04 is deterministic run to run.** Measured: two consecutive runs, 3340 census lines each,
  zero differences. The contrary guidance is withdrawn in `docs/GAME_EXECUTABLE.md`.
* **The aimglide pull-out `+76Ch`** is bound on arm A and the aimglide is no longer terminal.

## (b) The one thing to do next, and why it is the whole prize

Six of fifteen dive bombers abort their dive (`009C5B43`), and until this packet they sat in
`aimglide` for the rest of the mission holding a live bomb. They now pull out and reach `goaway`
within about 23 ticks - and **stop there**, for 761-841 ticks, because `009C7F00` never completes.

`dive_bomb_goaway_complete_009c7f00` is already bound and names its own blocker: the aircraft must
climb back to `min(cruise_altitude_398, begin_altitude_ac + aim_point_height_50)`, about **1000 m**,
and it left the dive at about **300 m**. So the question is entirely about the goaway's climb:

1. does `dive_bomb_goaway_climb_009c4b44` actually command a climb in this host, and does the
   planner act on it? Check `db_goaway_pitch_last` and the altitude trace across the 841 goaway
   ticks in `local\pullout_after.log`;
2. if it does, 700 m of climb at a Val's rate may simply not fit in 4800 frames - in which case the
   honest answer is "the image would re-attack, this mission is too short", and the way to show it
   is a longer run rather than a code change;
3. if it does not, that is the next binding and it hands six aircraft a second attack run.

`009C86EE`'s edge to flyabove is verified from the listing (EDI is `[ESI+778h]`, established by
filtering the whole 289-instruction transition listing), so nothing upstream of the goaway is in
doubt.

**CORRECTION to what this packet first reported**, from `cc8-dive-approach`'s whole-function read of
`009C868B`-`009C870E` and re-checked here. The second-attack-run split is **`009C86EE`**, the test
of `+4C9h`; `009C86D9` is the *goaway arm's own* `CMP EAX,ECX` against `kGoAway`, i.e. the dispatch
test, not an edge. Both destinations are register-proved: `EDI = [ESI+778h]` flyabove from
`009C84ED`, and `EBX = [ESI+664h]` done from `009C8484` - which is the only `LEA EBX` in the whole
function, and `kDone` is `0x664`. The terminal-aimglide finding itself is unaffected and was
confirmed independently (`009C86B9`'s JE leaves the state alone when `009C7850` is false and
`+76Ch` is 0).

## (b2) Release accuracy: a first cut, and where it points

Routed here by the integrator off this packet's section 5 finding. Not investigated to a
conclusion - this is what one pass over the per-bomb rows in `local\pullout_after.log` establishes,
so the next reader starts from data rather than from the question.

| bomb | its own predicted impact (`009C7D71`) | vs the target at release | fall |
| --- | --- | --- | --- |
| `#3.1` | 2.3 m | 41.7 m | 3.15 s |
| `#3.1.-3` | 5.5 m | 42.3 m | 3.20 s |
| `#1.1` | 12.2 m | 25.5 m | - |
| `#1.1` | 4.4 m | **11.3 m** (a hull hit) | - |
| `#1.1.-2` | 10.0 m | 28.0 m | 3.10 s |
| `#1.1.-2` | 4.2 m | **6.1 m** (a hull hit) | 2.60 s |
| `#3.1.-2` | 21.4 m | 57.0 m | 3.25 s |
| `#1.1.-3` | 12.3 m | 25.5 m | 3.20 s |

**The bomb goes where the solution says it will.** Its impact is within 2.3-21.4 m of
`009C7D71`'s own predicted point, median about 10 m, and the actual fall is 2.6-3.25 s. So the
integration and the ballistics are broadly consistent with the predictor, and they are *not* where
the miss comes from.

**The miss against the target is consistently the larger number**, 6.1-57.0 m, and it is larger
than the prediction error on every single round. So the loss is in **where the aim point sits
relative to the target**, i.e. the lead - not in the bomb's flight. That is the half of the
integrator's question this pass can answer.

**The lead this points at, and the trap in it.** The aimdive census prints
`impact 009C7D71: tf=<fall time> range=<predicted> (live <actual>)`, and the three squadrons read
`movieval tf=3.35 s`, `#7.1 tf=7.62 s`, `#3.1 tf=10.64 s` - the accurate squadron's `tf` matches
the measured 2.6-3.25 s fall and the two inaccurate ones are 2.4x and 3.2x it. A `tf` that long
would lead the aim point far ahead of where the target actually is when the bomb arrives, which is
exactly the observed failure. **Do not publish that as the cause without checking it**: those `tf`
values are LAST-SAMPLED columns, taken after the aircraft has left the dive, so they may not be the
`tf` in force at release. The check is one census field - sample `007BCC80`'s fall time and the
predicted point at the release tick specifically - and it is cheap. If the release-tick `tf` is
also 3x, the defect is in `007BCC80 BSP_Weapon_DropFallTime` or in what the predictor feeds it; if
it is not, the lead is right and the error is in the aim point's own geometry.

The decomposition the integrator asked for (miss split along-track / cross-track) needs the
target's heading at impact, which no census currently prints; add it beside the bomb row.

## (c) Left deliberately undone, with the reason

* **Arm B of the pull-out**, `009C57D3`-`009C57FD`. Three frame slots (`[ESP+24h]`, `[ESP+28h]`,
  `[ESP+6Ch]`) are unwalked, and the tick **reuses its own argument slot `[ESP+6Ch]` as scratch**
  from `009C5295` on, so none can be named without a frame walk. `tools/x87trace.py` plus
  `tools/calleefx.py` will do it; the callee table needs ten targets (`00414DB0`, `00419010`,
  `00438B10`, `007BBBA0`, `007C1DB0`, `009C4F80`, `00BD2F10`, `00BF701A`, `00BF7030`, `00BF8490`)
  and three indirect calls at `009C51D3`, `009C5232`, `009C5278`. Known so far: `009C4F80` is
  `net +1, imm 0`; `00BD2F10` is `imm 8`; `00BF8490` is `net 0, imm 0`. The latch currently fires
  strictly **less** often than the image's, which is the safe direction.
* **The bearing error at the first fly-over tick.** One census field in the fly-over feed. The
  `cross=` column already answers the question it was for (section 8): the dive-entry error is
  per-squadron, identical within a flight, present before the fly-over's first tick, and therefore
  delivered by the attack run or the moveto rather than made by the fly-over.
* **`docs/DIVE_BOMB_TASK.md` lines 1024 and 5034-5041** are superseded by
  `docs/DIVE_BOMB_FLYOVER_FLAGS.md` sections 1 and 2. Not edited, to avoid a doc conflict while
  that file was moving; fold them at integration.

## (d) The runs, all in this worktree's `local\`

| log | binary | what it is |
| --- | --- | --- |
| `flyover_after.log` | `65c997deb` | the bank arm, the latch and the dead band bound; 4800 mission frames |
| `usn04_rebaseline.log` | `65c997deb` | 4500 mission frames, the `docs/GAME_EXECUTABLE.md` row |
| `usn04_rebaseline2.log` | `65c997deb` | the determinism control: identical to the above on all 3340 census lines |
| `pullout_after.log` | `fcd7c145e` | the aimglide pull-out; the aimglide stops being terminal |

The before side of the first pair is `cc8-dive-heading`'s
`J:\PROG\battlestations-pacific-decompile-cc8-dive-heading\local\heading_after.log` on `d6275ef49`.

## (e) Two traps this area sets

* **The 210 m clamp runs AFTER the `BL` test.** `009C650E` compares `approach+D4h` against the
  *unclamped* `C` of `009C64C9`. Feeding the altitude arm's `limit_c` there compares 675 m against
  210 m and vetoes the bank arm on every tick of every mission. This cost one build.
* **`db_aim_pull_out_18` is the AIMDIVE's `+18h`** (whole-object `+74Ch`). The aimglide's own
  `+18h` is `+76Ch` - a different object - and the host used to carry one field for both. They are
  split now; do not re-merge them.
