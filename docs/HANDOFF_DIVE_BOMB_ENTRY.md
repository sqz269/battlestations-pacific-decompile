# Handoff: the dive-entry altitude, and the race that decides dive or glide

Packet `cc8_dive_entry`, 2026-09-19. Base `52418c86b`.
**Start from the branch `agent/cc8-dive-entry`, not from `main`** - see section (b).
Addresses: `009C6E10`-`009C6F91` (the flyabove's altitude arm), `009C84FB`-`009C8515` (the
transition arm), `009C67A7`-`009C680E` (the two flags), `009C8920` (the cruise profile),
`009C5188`-`009C51A5` (the aimglide's re-arm countdown), `009C3EA0` (the approach constructor).

The packet's own question is **answered and closed**: the dive-entry altitude is the **spawn
altitude**. `SpawnNew` places every `D3A Val` at `refPos` y = 1500.0 and the scripted `movieval`
starts at 700; nothing in the task sets it and, unbound, nothing changes it. Three candidate sources
were tested against the listing and all three are refuted - an authored per-class altitude (no such
field), a `PilotBotParameters` row (the index `[[unit+DF4h]+34h]` is unmodelled and no altitude is
in the row), and the per-unit ordered cruise altitude `ctl+398h` (`009C89CE`, its only writer on this
path, draws `tuning+4CCh + uniform(0, 15)` from the single `0042E740` record, so its whole spread is
15 m). Everything below is what that answer opened up.

## (a) The hand-over census, before and after, and the race

Per-aircraft, 4800-frame USN04, the altitude and planar range at every state change. The `cmd`
column is `plane_commanded_altitude`.

**Before** (`local\entry_before.log`, `52418c86b`):

| aircraft | spawn | attackrun -> flyabove | flyabove -> turndown | turndown -> aimdive | aimdive -> |
| --- | --- | --- | --- | --- | --- |
| `movieval` x3 | 700 m, 11088 m out | 729 m @ 1093 m | 730 m @ 4 m | 651 m @ 468 m | done, 181 m |
| `D3A Val #1.1` x3 | 1500 m, 8203 m out | 1126 m @ 1095 m | 1102 m @ 3 m | 1024 m @ 473 m | aimglide, 503 m |
| `D3A Val #3.1` x3 | 1500 m, 8199 m out | 1146 m @ 1096 m | flyabove/goaway chatter | 995 m | aimglide |
| `D3A Val #5.1` x3 | 1500 m, 8203 m out | 1126 m @ 1095 m | mission ends in the flyabove | - | - |

Across the flyabove the altitude moves **+1 m** (`movieval`, 158 ticks) and **-24 m** (`#1.1`, 142
ticks), and `cmd` is frozen at the run-in's last value for the whole state: this host's flyabove
commanded no altitude at all.

**After** (`local\entry_after.log`, the same binary plus the altitude arm):

| aircraft | attackrun -> flyabove | flyabove -> ... |
| --- | --- | --- |
| `movieval` | 729 m @ 1093 m | **aimglide** at 665 m @ 849 m, after 34 ticks |
| `D3A Val #1.1` | 1126 m @ 1095 m | **aimglide** at 666 m @ 14 m, after 113 ticks |

`cmd` becomes 210 and the aircraft follow the commanded 24 degrees (`#1.1` loses 460 m in 113 ticks).

**The race.** `009C84FB` reads `+791h` to decide *when* the flyabove ends and `009C8508` reads
`+790h` to decide *into what*:

* `+791h` = `|bearing error| > 1.6 rad` **OR** `span <= 0` (`009C67A7` `77` JA, `009C67AE` `72` JC)
* `+790h` = `B > approach+D4h` = **675.0 m** (`009C67F6` `76` JBE, stored `009C680E`)

> **CORRECTION, packet `cc8_dive_heading`.** This section originally read the second `+791h` arm as
> `B <= 666.7 m` and the paragraph below built on it. **Withdrawn.** `009C65FD`'s span is
> `max(R - S, 0)` with `R` the planar **range**, not the height: `009C659D`'s `FSTP ST(0)` discards
> `B` before `009C65DB`'s `FSUBP ST(2)`, which takes `R` off the stack from `009C64EE`. 666.7 is
> where `max(B - (0.7B + 200), 0)` reaches zero, so it is an artefact of the host's swapped minuend,
> and its 8 m agreement with `approach+D4h` is a coincidence, not corroboration. The image's arm is
> **`R <= 0.7 * max(B, 100) + 200`**, a range-to-go test against a glide slope, and it has no relation
> to `+D4h`. The "two heights 8 m apart" framing below is therefore wrong, and so is the conclusion
> that the height arm must win: in `local\heading_before.log` all fifteen bombers hand over with
> `span=0` at `b` between 662 and 666 and a range between 150 m and 368 m, which is the artefact
> measured directly. See `docs/DIVE_BOMB_TASK.md`, "Packet `cc8_dive_heading`". The range is also
> taken to a three-second lead point, `aim - 3.0 * (v_own - v_target) - pos`, not to the target.

Whichever arm of `+791h` fires first decides the attack: the bearing arm means the wingover dive, the
height arm means the glide. With the altitude arm bound and the **host's** span, the height arm won
for every aircraft in this mission, and `movieval` entered the flyabove only 54 m above the crossing
the artefact produced - at 24 degrees it crossed in about two seconds with 849 m still to run.

## (b) The branch is deliberately NOT on main

`agent/cc8-dive-entry` carries `5096edcf7` (the altitude arm, the hand-over census, the before),
`a8b2d1d18` (candidate 3 refuted, the `ClimbAngle` retraction) and `d3d4910ec` (the aimglide re-arm
timer, the after). The integrator has kept it unmerged **because binding the altitude arm alone is a
regression**: `main` keeps a dive that releases and this branch does not.

Exact numbers, `movieval`, before -> after:

```
states[done=303 aimdive=53 flyabove=158 turndown=71 attackrun=1527]  releases=2  bombs_spawned=2
states[aimglide=809 flyabove=34 attackrun=1527]                      releases=0  bombs_spawned=0
```

and the mission's bomb drops go **6 -> 0**. `#1.1` likewise never reaches the turndown. The arm
itself is not in doubt: its census prints `C=210.0 band=31.5 target=210.0 ref=0.800
pitch=-0.419 rad`, which is the transcription reproducing itself, and no branch in
`009C62B0`-`009C7083` reaches either `RET 4` before `009C6F7D`, so it runs on every tick.

**Start from this branch.** Re-deriving the arm from `main` would cost the whole packet.

## (c) The three substitutions to bind TOGETHER, in the order the aircraft meets them

The arm cannot be judged alone because all three of the things that decide the race are wrong here.
Bind them in this order, **each in its own same-binary before/after window** so their effects can be
told apart, then judge the set.

1. **The dive-bomb run-in's climb angle.** `src/game_hosts_units.cpp:5595` passes a literal `0.0f`
   for `pin.class_climb_angle` on the dive-bomb attack run; every other path passes
   `slot.plane_climb_angle_1ec`. That field is **computed** at class load by `007C4BC5`-`007C4C14`
   (`007D98F0` probed at `tuning+24Ch LevelFlight * desc+184h StallSpd`, scaled by the 0.6 at
   `00CEFF98`) - `"ClimbAngle"` occurs **zero** times in this installation's `vehicleclasses.lua`, so
   it is not a data problem. With the gain at zero `009FB800`'s climb arm returns `min(0 * t, limit)`
   and the run-in can never reach its commanded 1450 m: `movieval` climbs 29 m in 1527 ticks. Every
   bomber therefore arrives at the flyabove hundreds of metres lower than the image's would, which is
   the largest single reason the height arm wins the race. Expect the flyabove entry altitude to rise
   and `movieval`'s 54 m margin with it.
2. **`approach+B4h` and `approach+B8h` from `TurnCircleRadius`.** This host seeds *both* from
   `kPilotDiveBombAttackDist` = 1100.0. The constructor draws them separately:
   `+B4h = uniform(0.6, 0.8) * classDesc+268h` (`009C3F6E`-`009C3F97`) and
   `+B8h = +BCh = uniform(1.6, 1.8) * classDesc+268h` (`009C3F9D`-`009C3FF5`, one draw stored twice).
   `classDesc+268h` is `TurnCircleRadius` (`include/bsp/plane_class_fields.hpp:174`), **1300** for
   both USN04 dive-bomber classes in this installation (`VehicleClass[108]` line 46667,
   `VehicleClass[46]` line 23623), so `+B4h` is 780-1040 m and `+B8h` is 2080-2340 m. Pin at the low
   end, as this host pins every draw. It needs `turn_circle_radius` plumbed from the plane class row
   into the slot; the loader already reads it (`src/plane_class_fields.cpp:323`). **Note the
   direction**: `+B8h` is where the flyabove *starts*, so a larger value gives it *more* run-in and
   makes the height arm win *sooner*. It is a faithfulness fix, not a fix for the race - measure it,
   do not assume it. `+B4h` also feeds the flyabove's `+1Ah` leave tolerance
   (`W = approach+B4h * 0.8 - S`), so expect `+1Ah` to move with it.
3. **The flyabove altitude arm**, already bound on this branch (`5096edcf7`), left in place.

Optional fourth, if the set still does not dive: `007F0280` at `009C42B8` is a contract, so the
run-in flies straight at its target instead of weaving. The weave changes the bearing history the
`+791h` bearing arm reads, which is the arm that has to win.

**The measure for the set**, per aircraft and never on mission totals: the bombers arrive at the
flyabove at the image's altitude and range; the bearing arm wins the race where the listing says it
should (`flyabove -> turndown`, not `-> aimglide`, in the hand-over line); `movieval` releases twice
again with its aim error near 8.06 m and its bombs landing within about 25 m of the predicted point;
and the Vals release at all. Mission totals are not deterministic run to run, and since `f14732dc4`
bombs burst on impact, so every damage figure in this stream's older logs is stale.

## (d) The aimglide re-arm timer

`009C5180` counts `state+1Ch` down by its own `dt` at the top of **every** aimglide tick, exactly as
`009C58E9` does for the aimdive:

```
009c5188  MOVSS  XMM0,dword ptr [ESI + 0x1c]
009c518d  COMISS XMM0,dword ptr [0x00d7a218]   ; the float 0.0f
009c519b  JC     0x009c51a8                    ; byte 72: below zero, skip
009c519d  FLD    float ptr [ESP + 0x28]
009c51a1  FSUB   float ptr [ESP + 0x6c]        ; the dt argument
009c51a5  FSTP   float ptr [ESI + 0x1c]
```

This host decremented `db_aim_rearm_1c` only in `dive_bomb_aimdive_inputs`, which runs only while the
state is `kAimDive`, so in the glide the timer sat at the 0.0 its enter writes and `009C5689`'s
`state+1Ch < 0` was false forever. That is also why this packet's gate census disagreed with the
table in "The aimglide release, walked with frame bases", which was measured in the glide packet's
own tree: identical geometry to the digit, a different gate stopping the chain. Fixed in
`d3d4910ec` by passing the `dt` that `read_aimglide_inputs` was already being handed.

**Measured, `local\entry_rearm.log`.** The fix works and the glide still does not fire. Gate 1 stops
being the blocker and the chain reaches the gates the glide packet originally measured:

```
movieval      calls=809 blocked[rearm=0 bearing=701 ceiling=108 lateral=0 lead_hi=0 lead_lo=0]
              passed=0 | bearing err min=0.0001 rad (gate 0.5236) | throw=871.6 m range=560.1 m
D3A Val #1.1  calls=779 blocked[rearm=0 bearing=762 ceiling=17  lateral=0 lead_hi=0 lead_lo=0]
              passed=0 | bearing err min=0.1344 rad (gate 0.5236) | throw=868.4 m range=960.1 m
```

`summary mission dive-bomb task: aircraft=15 releases=0 bombs_spawned=0`.

Three things to carry forward. **The bearing gate is satisfiable but rarely satisfied**:
`movieval`'s best is 0.0001 rad against a 0.5236 gate, yet it fails on 87 per cent of ticks, so the
aircraft is not tracking its target through the glide - the same shape the glide packet reported.
**The lateral gate is never reached**, so this packet cannot report a `|throw - range|` measurement
against the 120 m gate as its brief asked; what the numbers allow is only the observation that
`|871.6 - 560.1| = 311.5 m` and `|868.4 - 960.1| = 91.7 m` would have to pass it, and the first
would not. **Nothing reaches the lead window**, so `lead last/min = 0.00` is a lead never computed,
not a lead of zero.

So the re-arm fix is a correctness fix that unblocks the chain and changes no outcome in this
mission; it should be kept, and the remaining glide block is upstream steering and geometry, not the
release arithmetic.

## (e) One unread question, for whoever takes the bomb path next

**Does the image burst a bomb on a WATER impact?** `0084BC60`'s step 7 does not test what was struck;
this host bursts on entity impacts only (`f14732dc4`). It decides whether a near miss does damage,
which is most of what a dive bomber achieves, and it is unread.

## (f) The logs, and which binary each belongs to

All 4800-frame USN04 (`--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames
4800 --mission-frame-seconds 0.05`), in this worktree's `local\`:

| log | binary |
| --- | --- |
| `entry_before.log` | `52418c86b` plus the per-hand-over census only |
| `entry_after.log` | the same, plus the flyabove altitude arm (`5096edcf7`) |
| `entry_rearm.log` | the same, plus the aimglide re-arm countdown (`d3d4910ec`) |

`local\entry_before.log` is the baseline for all three. It is **not** interchangeable with
`cc8-dive-glide`'s `glide_after.log`: `D3A Val #3.1` does not reproduce between them (`attackrun=1098
aimdive=34 aimglide=802 transitions=21` here against `attackrun=1250 aimdive=50 aimglide=618` there),
while `movieval` and `#1.1` reproduce to the digit.
