# Handoff: the goaway climbs now, and the state's other half is an evasive turn

Packet `cc8_dive_goaway`, branch `agent/cc8-dive-goaway`, commit `b7be4aca1`. Successor of
`cc8-dive-flyover`. Read `docs/DIVE_BOMB_GOAWAY.md` whole first - it is this packet's evidence and
it is written to be read in order. This file is only what that document does not say.

## (a) Settled, and not to be re-derived

* **`009C4A40` is the `kGoAway` tick**, named `BSP_BotStateDiveBombGoAway_Tick` in Ghidra and the
  ledger, proved by dispatch: the vtable install at `009C74EE` is `[ESI+30Ch]` and `ESI = task+3F8h`
  on three independent agreements (`kGoAway 704h`, `kAimDive 734h`, `kAimGlide 754h`). Body
  `009C4A40`-`009C4E65`.
* **The climb is two curves and a max**, `009C4B32`-`009C4BD8`, both now confirmed by measurement to
  three significant figures, not just read from the listing. Curve A is the climb-back-to-cruise arm
  over the altitude **deficit**; curve B is the ground-avoidance arm over the raw altitude. The old
  comment calling curve A untraced and substituting B for it is retracted in the header, the ledger
  and the doc.
* **`009C7F00`'s ceiling and `009C4A40`'s ceiling are the same expression**,
  `min(ctl+398h, approach+ACh + approach+50h)`, built at `009C4ACF`-`009C4B05` and `009C7F23`.
* **The goaway does not complete on USN04 at 4800 frames, and the reason is the clock, not the
  model.** `#3.1` reaches 866.8 m against a 900 m gate with the command still positive. Do not
  "fix" this with a constant. At 9000 frames all six complete at 899.7-900.0 m and fly a **second
  attack run**: `transitions` 6 -> 10-15, a second `turndown` and `aimdive`, `releases` 1 -> **2**,
  `rounds_left` 1 -> **0**, ending in `done`. The chain closes with no constant touched.
* **`local\goaway_long.log`'s mission-level numbers must not be quoted** - `bomb_impacts=0`,
  `total_damage=3596.8`, `deaths=5`, `Lexington-class01` unsunk - but the reason recorded here was
  wrong and is **withdrawn** (packet `cc8_dive_aim`). The run did NOT diverge: 4800 `world frame`
  lines, every `torpedo trace` to t = 133.45 s, every `dive probe` and 200 `plane` lines are
  identical to the same prefix of `goaway_after.log`, and both runs carry the launcher's `slot0`
  tag. **`GameUnitsHost::create_units` rebuilds the gunnery host unconditionally on every spawn
  batch** (`src/game_hosts_units.cpp` ~3704), discarding the summary and every in-flight round; a
  batch after frame 4800 (`aircraft=24` against 15) reset it, which is why `first_hit` is *earlier*
  at 15.10 s. Withdrawn with it: the two candidates above (`--frames` - the pre-mission budget is
  200 either way - and the missing instance tag), the "~2100 frames later" observation (a symptom of
  the same reset), and two later readings, "the summary merely under-reports" and "the run
  diverged". Fix: packet `cc8-gunnery-host`. **The state/tick/release counts used here are dive-bomb
  TASK counters on the units host, which a batch does not reset, so section (a)'s second attack run
  stands unchanged.**
* **The -5 degree nose-down flag** (`009C4A43`-`009C4A68`, `00CF885C`) is recovered and carried on
  `DiveBombGoAwayCommand::wrote_bank_heading`, deliberately **unconsumed**. See (c).

## (a2) What the second attack run actually looks like, and one free answer

From `goaway_long.log`'s state counts, comparing the 9000-frame run against the 4800-frame run:

| | 4800 | 9000 |
| --- | --- | --- |
| `attackrun` | 1073-1141 | **unchanged**, 1073-1141 |
| `flyabove` | 84-89 | **85-91** (+1 to +6 only) |
| `turndown` | 56-57 | 111-167 (doubled) |
| `aimdive` | 60-66 | 105-174 (doubled) |

So the `009C86EE` split **does** send the aircraft to `flyabove`, but the second pass spends only
**one or two ticks** there and the `attackrun` state is never re-entered at all. The aircraft is
already at 900 m and inside `approach+B8h`, so the fly-over's conditions are satisfied on arrival.
The second attack run is therefore `goaway -> flyabove (1-2 ticks) -> turndown -> aimdive -> release
-> done`, and it **enters the dive at 789-790 m** against 894.9 m for the first dive (the `dive
entry` census column is last-sampled, so this value is the second dive's). The second release is at
325.6-344.9 m with `aim error 009C5C9B` of -4.15 to 19.73 m, inside the 25.0 m gate.

**The exact completion tick and the second fly-over's entry range are NOT in the log**: the
`hand-overs` census columns are first-sample, so they still show the first pass. A successor that
wants them needs a per-pass census, not another run.

**A free answer to the leader-relative question.** `009C4A40`'s complete callee set is `00414DB0`,
`00419010`, `0042E740`, `0099B630`, `009C47D0`, `009FABE0`, `00BD2F10`. **`009F9ED0`, `009FBA50` and
`009FB800` are not among them.** The goaway commands no altitude at all - it writes a *pitch* to
`cmd+2BCh` with mode 1 and lets the planner's own pitch arm fly it. So the "leader-relative or
absolute" question does not arise here, and the aborted bombers' climb and the spent wing members'
floor in `done` are **two different commands that merely look alike**, not one missing command seen
from two states. `cc8-follow-steer` and this packet are independent.

## (b) The one thing to do next in this state

**Bind the evasive turn: the `+24h`/`+28h`/`+2Ch` timers and the two arms at `009C4D9D`-`009C4E63`.**
This is the other half of the goaway and it is currently absent.

* `009C4A6D`-`009C4ACD`: `+24h` counts down by `dt` while `(+20h - 100) > approach+BCh`; `+28h`
  accumulates `dt`; when `approach+C4h < 1.0` and `+28h > +2Ch + 6.0` the tick forces `+24h = -1.0`.
* `009C4CF1`-`009C4D9A`: a negative `+24h` re-rolls `+2Ch` and `+24h` through two
  `BSP_Random_UniformFloatRange 00BD2F10` calls (`00CE3854`/`00CE6630` and `00CEB4B8`/`00CE5380`) and
  an `00419010` curve over the altitude, then sets `+28h = -(+2Ch * 00D7A280)`.
* `009C4DAD` splits: `+2Ch > +28h` takes the **bank** arm `009C4DAF`-`009C4DF6`
  (`cmd+2C4h = clamp(2 * +28h * +18h, 00CE3814, 00D05EA4)`, `cmd+2CCh = 1`); otherwise the
  **heading** arm `009C4E05`-`009C4E1D` (`CALL 009C47D0` fills `+1Ch`, then `cmd+2C0h = +1Ch`,
  `cmd+2CCh = 2`, then `0042E740`/`0099B630`/`009FABE0` for the speed side).
* `009C47D0` is unread and is the key: it is what decides which way the aircraft turns away.

Only once those exist should `wrote_bank_heading` be consumed. Gating the wings-level pair off
without them leaves the state with no lateral command at all, which is **further** from the image
than the unconditional wings-level the host writes today. I nearly shipped that; see (e).

## (c) Left deliberately undone, with the reason

* **Item 4 of the packet brief, release accuracy, is not started.** Nothing was measured and nothing
  is claimed about it. Per the integrator, it is now narrowed: every round lands within 2.3-21.4 m of
  `009C7D71`'s own predicted point while the miss against the target is 6.1-57.0 m, so the loss is in
  the **lead**, not the ballistics - do not start at `release_bomb_drop`.
  **The read to do first is the image's producer of the approach's aim point `+4Ch/+50h/+54h`**:
  filter `009C7A80`'s whole listing for every store to those three (disp8 forms and `float ptr` x87
  stores) and trace each value back - target position, target position led by velocity times a fall
  or closure time (the fly-over uses a 3.0 s lead on relative velocity via `009FA2E0`, qword at
  `00D7A2B0`), or target position plus an authored offset. This host uses the target's **live
  position with no lead at all** (`update_dive_bomb_approach`, `slots[ti]->motion.position`), so that
  one read says whether the zero-lead aim point is faithful. Take the release-tick `tf` census field
  (`007BCC80 BSP_Weapon_DropFallTime`) in the same run as whatever that binds - the existing `tf`
  column is last-sampled, taken after the aircraft left the dive, so the 3.35 / 7.62 / 10.64 s spread
  across squadrons is **not** evidence yet.
  This now matters more than before: with the ship branch landed, the carriers move by the image's
  own rules, and the ship worker's USN01 torpedo trace shows zero-lead aim missing a moving ship by
  175-210 m.
* **Whether the formation mates sit inside the probe's box is not checked.** `007F0280` is an
  axis-aligned box of half-extents 80 x 60 x 120 m centred on the aircraft that can only see another
  *unit*, so `sampler_result = 0` is a proof over open sea and a hole only in close formation. Every
  one of these bombers flies in a three-ship flight. One line of arithmetic against
  `docs/PLANE_FORMATION.md` section 4.1 settles whether the image weaves the wingmen up to 30 degrees
  apart on the run-in where this host does not. I did not do it.
* **Arm B of the aimglide pull-out is still unbound**, but it is two slots from done rather than
  three. See `docs/DIVE_BOMB_GOAWAY.md` section 6: `[ESP+6Ch]` is proved to be the tick's `dt`
  argument, `[ESP+28h]` is the `+1Ch` re-arm timer, and `[ESP+24h]` is `length1 / length2` where
  **length 2 is the planar distance to the approach's `+D8h`/`+DCh`/`+E0h` point**. What remains:
  length 1's point comes from the out-vector of the indirect call at `009C51D3` (`LEA ECX,[ESP+50h]`
  at `009C51CC`) and that virtual is unidentified; and `[ESP+6Ch]` at `009C57F1` holds path-dependent
  scratch, not `dt`.
* **The `+278h`/`+27Ch`/`+2A8h`/`+2ACh` throttle and air-brake slots** the goaway writes on both paths
  are not modelled because this host keeps no such slots. Grep confirms: no `_278`/`_2a8` anywhere in
  `game_hosts_units.cpp`.

## (d) The runs, all in this worktree's `local\`

| log | base | binary | what it is |
| --- | --- | --- | --- |
| `goaway_before.log` | `78aefa17b` | census only, uncommitted | the strict before |
| `goaway_after.log` | `78aefa17b` | = `b7be4aca1` | curve A bound; the strict after |
| `goaway_long.log` | `78aefa17b` | = `b7be4aca1` | 9000 mission frames, the outcome (ii) demonstration - **not** a paired measurement |

**All three ran on base `78aefa17b`, which is BEFORE the ship branch landed.** `main` has since moved
to `192c2a614` and beyond, and that branch changes how USN04's carriers and escorts move (leaders
ordered, followers keeping station by formation follow). The pair stays valid **as a pair**, but none
of its absolute numbers may be carried across the merge. **A successor must merge `main` and take its
own before on the merged tree.** I did **not** merge `main` into this branch: the merge touches the
ship hunks of `src/game_hosts_units.cpp`, which I do not own, and I was too near my context limit to
resolve a conflict there safely. The branch is clean at `a029de233` for the integrator to merge.

`goaway_before.log`'s behavioural columns match the predecessor's
`J:\PROG\battlestations-pacific-decompile-cc8-dive-flyover\local\pullout_after.log` exactly
(`total_damage 9820.7`, `deaths 8`, `bomb_impacts 18`), which is what makes the census provably
print-only and the pair valid.

## (e) Three traps this area set, two of which I walked into

* **The raw listing's `[ESP+N]` shifts inside a `PUSH`+`CALL` window.** In `009C5180` this made two
  slots look as if they had no writer at all and I wrote that down as the open question before the
  decompiler showed both assigned. Any frame walk here must account for the pushes, and the cheap
  cross-check is `ghidra decompile`, which names the locals consistently across the shift.
* **A census counter takes its meaning from where it is incremented.** `complete_ticks` sits in
  `dive_bomb_transition_inputs`, which runs on every tick of the **arm**, so it reads 1109-1283 for
  aircraft whose goaway never completed once. It is left that way so the before/after pair differs
  only by the change under test. Read it as "ticks of the whole arm".
* **The flag-0 side of a two-way split is not the empty side.** `009C4BEF`'s `JZ` looks like "skip
  the command writes", and it is not: it skips *these* writes and reaches different ones 700 bytes
  later. Follow the jump target to the `RET` before calling a branch command-free.

## (f) One number the integrator should carry forward

Binding the climb costs an aircraft. `deaths` 8 -> 9, `entity_impacts` 46 -> 60, `total_damage`
9820.7 -> 10188.4, with every dive-bomb state count and `bomb_impacts` unchanged; the extra death is
`D3A Val #3.1`, one of the six, shot down by `Northampton-class04` after climbing back into the
escorts' AA. The image climbs too, so this is faithfulness rather than regression - but the aircraft
the second attack run was meant for may not survive to fly it, and that is worth knowing before the
`009C86EE` re-attack edge is judged.

## Correction from docs/DIVE_BOMB_GOAWAY_TURN.md (2026-09-22, packet `cc9_goaway_turn`)

* Section (b)'s bank clamp `clamp(2 * +28h * +18h, 00CE3814, 00D05EA4)` has its bounds the other
  way round: `00D05EA4` = -1.2 is the **low** bound, tested first at `009C4DBC`, and `00CE3814` =
  +1.2 the high one at `009C4DD8`.
* Section (b)'s "then `0042E740`/`0099B630`/`009FABE0` for the speed side" is wrong: no speed is
  commanded. `009C4E2C` stores `tuning+674h` (`Pilot/AutoStrafeAngle/Angle_GoAway`) to
  `(approach+1Ch)+40h`, and `009FABE0` writes a unit direction vector built from the heading and
  `0099B630`'s commanded pitch to `(approach+1Ch)+68h..70h`.
* Section (b) omits the enter. `009C4950` (goaway vtable slot `+4h`, no Ghidra function) seeds
  `+18h` (side), `+24h = 15.0` and `+20h` (the standoff, `2 * approach+B4h` and more). The host had
  left the goaway's `+20h` at zero, which `009C7F00`'s completion rule also reads.
