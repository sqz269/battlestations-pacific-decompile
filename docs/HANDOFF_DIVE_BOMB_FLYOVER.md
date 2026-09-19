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

`009C86D9`'s edge to flyabove is verified from the listing (EDI is `[ESI+778h]`, established by
filtering the whole 289-instruction transition listing), so nothing upstream of the goaway is in
doubt.

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
