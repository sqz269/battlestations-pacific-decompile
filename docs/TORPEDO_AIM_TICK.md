# The torpedo `aim` tick `009D15F0` and the aim-complete byte

Addresses: `009D15F0` (`009D15F0`-`009D2377`, 3464 bytes, `void __thiscall(BotStateTorpedoAim*,
float)`, `RET 4`), the aim-complete test `009D22FF`-`009D236E`, its consumer `009D31B0`
(`009D31B0`-`009D31C2`), the goaway predicate `009D3150` (`009D3150`-`009D31A5`), the
time-to-target metric `009D1500`, the interpolation helper `00419010` (`RET 14h` at `00419030`),
`00438AA0`/`00438B10` (`RET 8`), `00903860` (`RET 8`), `009FA3A0` (`RET 4`), `00427EB0` (`RET`),
`00414DB0` (`RET`), `007F0280` (`RET 18h` at `007F0B1F`), `009D1360`.

`coverage: reconstructed` for the whole body except the obstacle probe `007F0280`, which is
`coverage: contract` (see "no_ghidra_function" and "Uncertainty").

## (1) The frame accounting, and why the previous packet's negative result was wrong

`docs/TORPEDO_AIM_COMPLETE.md` concluded that two of the four slots the aim-complete test reads
have no dominating write, and that a corrected stack walk and Ghidra's decompiler disagree about
the clause-1 minuend. **Both halves of that are artefacts of a 20-byte (`0x14`) frame
misattribution, and the test is in fact a single expression on four slots that every path defines.**

The accounting here is not a linear walk. `local/depth.py` in this worktree propagates an ESP
depth along every fall-through and branch edge of the function's own graph and reports any join
whose predecessors disagree:

```
instructions=870 reached=870 unreached=0 conflicts=0
```

Zero conflicts over all 870 instructions, and the epilogue (`POP EDI`, `POP ESI`, `ADD ESP,0x64`
at `009D2372`-`009D2374`) balances exactly. The cleanups that make it balance were read from each
callee's `RET imm16`, not assumed:

| callee | RET | sites in this function |
| --- | --- | --- |
| `00419010` | `RET 0x14` | thirteen, each preceded by its own `SUB ESP,0x14` |
| `00438AA0` / `00438B10` | `RET 0x8` | six, each preceded by `SUB ESP,0x8` |
| `00903860` | `RET 0x8` | `009D16D1`, `009D2265` |
| `009FA3A0` | `RET 0x4` | `009D2027` |
| `007F0280` | `RET 0x18` | `009D1A94`, six pushes |
| `009D1500`, `009D1360`, `00414DB0`, `00427EB0` | `RET` | callee takes nothing on the stack |
| `CALL EDX` / `CALL EAX` | callee-clean `__thiscall` | pops what was pushed since the last call |

**Slot key.** `F` is the byte offset from the deepest prologue ESP (`SUB ESP,0x64` at `009D15F0`
plus `PUSH ESI` plus `PUSH EDI`). `F=0` is the saved EDI, `F=4` the saved ESI, `F=08h`-`F=6Bh` the
locals, `F=6Ch` the return address and `F=70h` the `dt` argument. `EBX` and `EBP` are pushed
mid-body at `009D1BBC`/`009D1BBD` and popped at `009D22E2`/`009D22E9`, so `009D1BBE`-`009D22E1`
runs eight bytes deeper; the fixpoint proves every path crosses that pair exactly once.

**Where the previous walk broke.** At `009D2310` the correct depth is `0x80`
(`0x6C` + the `SUB ESP,0x14` at `009D2303`), so `[ESP+0x28]` is `F=14h`. The earlier walk carried
`0x6C` there and read the same instruction as `F=28h`. `F=28h` is the commanded altitude floor
(`009D1673`/`009D16F8`), which is how "the commanded altitude floor" came to be named as the
clause-1 minuend. Ghidra's `fStack_5c` is `F=10h` (`0x6C - 0x5C`), the bearing error stored at
`009D1699`, one slot below the real operand. Neither method was reading `F=14h`.

**The four `SUB ESP,0x14` windows the handoff flagged.** There are twelve, not four, and every one
is an argument window for `00419010`, which pops it. `009D2085` and `009D217D` write
`double ptr [ESP+0x64]` inside such a window at depth `0x88`; that is `F=48h`, the function's
general-purpose scratch double (74 accesses), not an argument slot. There is no case in this
function where one `[ESP+n]` is an argument on one side of a call and a local on the other: the
argument slots are always `F` in `-1Ch`..`-4`, strictly below the frame.

## (2) The frame table for the slots that matter

| slot | meaning | writes | reaching the test |
| --- | --- | --- | --- |
| `F=0Ch` | time to target, seconds | `009D160F` (`009D1500`), `009D178A` (`x 0.9f`) | both |
| `F=10h` | the steering delta, radians | `009D1699`, `009D1850`, `009D18B7`, `009D1ADD`-`009D1B17`, `009D1B9A` | - |
| `F=14h` | range to the target point, metres | `009D1604` (`approach+90h`), `009D1782` (`x 0.9f`) | both |
| `F=18h` | \|steering delta\|, radians | `009D162D`, `009D17E0`, `009D18F5`, `009D1BB0` | `009D18F5`, `009D1BB0` |
| `F=1Ch` | the commanded heading | `009D17FC`, `009D1885`, `009D18A6`, `009D18C9`, `009D1B7F`, `009D1BD4` | - |
| `F=20h` | the unit heading, then the aim-solution envelope | `009D167B`, `009D1C71`, `009D1F99`, `009D1FF6` | - |
| `F=28h` | the commanded altitude floor | `009D1673`, `009D16F8` | - |
| `F=2Ch` | the target's aspect angle | `009D164A`, `009D16AA`, `009D1723`, `009D174C` | - |
| `F=34h` | the release-distance threshold | `009D1956`, `009D197A`, `009D1A70`, `009D1DED`, `009D2306` | **`009D1DED` only** |
| `F=40h`, `F=48h` | scratch float / scratch double | many | - |

`local/reach.py` computes the reaching definitions. For the four reads the test makes:

```
F=0x34 R 009d22ff  reached-by=009d1ded
F=0x14 R 009d2310  reached-by=009d1604 009d1782
F=0x0c R 009d2318  reached-by=009d160f 009d178a
F=0x18 R 009d2364  reached-by=009d18f5 009d1bb0
```

`009D1604`/`009D1782` and `009D160F`/`009D178A` are the same quantity with and without the 0.9
tighten, so clause 1's minuend and the interpolation parameter are single expressions. `F=34h` has
one reaching write. Only `F=18h` has two genuinely different producers, and both are the absolute
value of the same steering delta.

## (3) The aim-complete rule

```
009D22DC  speed = (approach->+134h >= 15.0) ? approach->+7Ch : approach->+80h
009D2324  half  = (float)((double)speed * 0.5)
009D2345  ramp  = InterpolateClamped(x0=0, y0=0, x1=1, y1=half, x=F0C)
009D234A  rem   = (float)((double)F14 - ramp)
009D235A  if (F34 > rem)   state->+2Ch = 1     /* JA 009D236E */
009D2368  if (F18 > F0C)   state->+2Ch = 1     /* JBE 009D2372 skips */
```

`00419010` is `clamp(y0 + (y1-y0)*(x-x0)/(x1-x0), min(y0,y1), max(y0,y1))`, returning `y0` when
`x0 == x1` (`00419026 JP` / `0041902C`). So `ramp = clamp(F0C * speed/2, 0, speed/2)`.

Nothing in the tick clears `state+2Ch`; `009D236E` only ever stores `1`.

**The operands.**

* `F0C` (`009D1500`): `t = approach->+90h / speed`, returned directly when `t <= 1.0` or when
  `speed >= 600.0f`, else `(+90h - speed)/600.0 + 1.0`. Seconds. This corrects
  `docs/TORPEDO_AIM_COMPLETE.md` section (4), which called this slot a distance.
* `F14` (`009D1604`): `approach->+90h`, the 2D range `009D3420` writes. Metres.
* `F18` (`009D18F5` or `009D1BB0`): the bit-level absolute value of the steering delta, radians.
* `F34` (`009D1DED`): `max(unit->+100h - altitude_floor, -0.5f) * max(1 - 1.6f*F18, 0.1f)`, the
  altitude above the commanded floor scaled down by how hard the aircraft is still turning.

Clause 1 therefore fires when the remaining range, minus the half-speed ramp, has fallen to the
height the aircraft still has to lose. Clause 2 fires when the steering delta in radians exceeds
the time to target in seconds, which is the "this run is not recoverable" arm.

## (4) The rest of the body, in listing order

| range | what it does |
| --- | --- |
| `009D15F0`-`009D1673` | the entry block. `F14 = +90h`, `F0C = 009D1500`, `F1C = +94h`, `F18 = +5Ch`, the unit heading from `vtable[50h]`, and the altitude floor `max(+A9h ? 30 : 5, +78h + +74h)` |
| `009D1683`-`009D16F8` | `F10 = SubtractWrappedAngle(+94h, heading)`; `00903860` ground height; the floor takes `max(..., ground + 5.0)` |
| `009D16FE`-`009D174C` | if `approach+CCh` exists and answers `vtable[5Ch](6)`, `F2C = |SubtractWrappedAngle(target heading, own heading)|`, else `F2C` keeps the `009D16AA` zero |
| `009D1750`-`009D178A` | if `unit->vtable[5Ch](10h)` or `(16h)`, both `F14` and `F0C` are multiplied by `0.9f`. `009D177E` is `DC C9` (`FMUL ST(1),ST(0)`), which is why both slots are scaled; Ghidra prints `DC C9` and `D8 C9` identically as `FMUL ST1` |
| `009D178E`-`009D18C9` | only when `+5Ch != 0`: `F18 *= Interp(1.8,3.0,5.0,0.8,t)`, clamp to `[-1.2,1.2]`, add `Interp(1.35,0,1.6,1,t) *` that to the bearing error, clamp to `[-1.4,1.4]`, and make the commanded heading |
| `009D18CD`-`009D18F5` | the `+134h >= 15.0` speed switch; `F18 = |F10|` |
| `009D190A`-`009D19A0` | `turn_room = max(obj+268h, obj+26Ch) * sin(min(F18, 1.2)) + speed + 200.0`; when that exceeds the range the tick calls `009D1360` and jumps to the `009D1BB8` join |
| `009D19AE`-`009D1B64` | the obstacle probe arm, entered only when `approach+AAh != 0` and `Interp(1.5,0.872665,3.0,0.349066,t) > F18`. `007F0280` gets an extent triple `{72t, min(0.7*72t, 150), 1.5*72t}`; a hit re-folds the commanded heading and recomputes `F18` at `009D1BB0` |
| `009D1BB8`-`009D1D39` | the throttle `min(1.2, bank_fold * time_fold * alt_fold * obj+25Ch * 1.2)` and the eleven command-block writes |
| `009D1D44`-`009D1DED` | the release threshold `F34` |
| `009D1DD6`-`009D1EE5` | the commanded pitch `clamp(-F34 / max(range - 1200, obj+188h * denom), 0.05625, 0.872665)` into `cmd+2BCh` |
| `009D1EEF`-`009D1F6B` | the ordnance fade `cmd+264h` when `t < 1.5`, and the five constant command bits |
| `009D1F75`-`009D2021` | `approach+130h = (Interp(0.5,1,1,+84h,|cos(F2C)|) * speed + 200.0 > range)` |
| `009D2027` | `009FA3A0(&state+18h, dt)` |
| `009D202C`-`009D2233` | the five-flag release chain; all five arm the `state+18h` timer at `009D2287`, the cone flag alone opens the countdown `approach+A4h` at `009D22C0`/`009D22CF` |
| `009D22D9`-`009D2377` | the speed switch again and the aim-complete test |

Argument order for `00419010` is `(x0, y0, x1, y1, x)` at `[ESP+0]`, `[ESP+4]`, `[ESP+8]`,
`[ESP+0Ch]`, `[ESP+10h]`. Every one of the thirteen call sites in this function was transcribed
against its push order.

Every `double ptr` operand this function loads is the exact widening of a float, so the
reconstruction computes in float and widens only where the listing widens.

## (5) ABI

`void __thiscall BotStateTorpedoAim::Tick(BotStateTorpedoAim* this /* ECX */, float dt)`,
`RET 4` at `009D2377`. `this+4h` is the approach record, `this+18h` the timer block `009FA3A0`
accumulates into, `this+24h` a byte the release chain reads at `009D2030`, `this+2Ch` the
aim-complete byte. `approach+4h` is the unit, `approach+8h` the aircraft description,
`approach+Ch` the pilot control block, `approach+18h` the command block, `approach+1Ch` a slot
whose `+40h` the countdown clears.

`009D31B0`: `bool __thiscall(BotStateTorpedoAim*)`, `RET`.
`return approach->+132h ? state->+2Ch : false` (`009D31B3`, `009D31BF`).

`009D3150`: `bool __thiscall(BotStateTorpedoGoAway*)`, `RET`. Returns
`approach->+90h > d` at `009D3195`, where `d = state->+24h`, scaled by `[00CE65D0]` when
`ctl->+369h` and `[00E17BF2]` are both set, and `false` outright when `approach->+132h` is clear.

## Host methods

`include/bsp/torpedo_aim_tick.hpp` declares one `TorpedoAimTickHost` method per native call site.

| host method | native | site |
| --- | --- | --- |
| `time_to_target_009d1500` | `009D1500` | `009D160A` |
| `unit_heading_vtable50` | `unit->vtable[50h]` | `009D1679` |
| `target_heading_vtable50` | `target->vtable[50h]` | `009D1721` |
| `target_is_kind_vtable5c` | `target->vtable[5Ch]` | `009D1714` |
| `unit_is_kind_vtable5c` | `unit->vtable[5Ch]` | `009D175F`, `009D176E`, `009D1E08`, `009D1E17` |
| `has_target_cc` | `approach+CCh` | `009D1701` |
| `refresh_world_pose_00414db0` | `00414DB0` | `009D16B4`, `009D1C79`, `009D1D4F`, `009D2077` |
| `ground_height_00903860` | `00903860` | `009D16D1`, `009D2265` |
| `update_run_time_009d1360` | `009D1360` | `009D19A4` |
| `sector_probe_009d1a94` | `007F0280` | `009D1A94` |
| `accumulate_timer_009fa3a0` | `009FA3A0` | `009D2027` |
| `arm_release_timer_009d2287` | the `state+18h` store | `009D2282`, `009D2287` |

`00427EB0` at `009D2253` is folded into `ground_height_00903860`: its `PUSH EAX` is not its own
argument but `00903860`'s out-pointer, which is why `00427EB0` returns with no `imm16` and the
frame still balances.

## Corrections

### Correction to `docs/TORPEDO_AIM_COMPLETE.md` (packet `cc8_torpedo_aim_complete`)

That document's central claim - that two of the four slots have no dominating write and that the
test cannot be lifted out of the tick - does not hold. Its section (3) dominance table is computed
over slot keys that are 20 bytes off for every read inside the last `SUB ESP,0x14` window, so it
grouped `009D2310` with the writes to `F=28h` and `009D22FF`, `009D2318`, `009D2364` with other
slots' writers. With the corrected keys all four slots are defined on every path (section (2)
above), and the test is the single expression in section (3) above.

Its section (5) disagreement is resolved against **both** candidates. The clause-1 minuend is
neither the commanded altitude floor (that is `F=28h`, written at `009D1673`) nor the bearing error
(that is `F=10h`, written at `009D1699`). It is `F=14h`, `approach+90h`, written at `009D1604` in
the entry block and optionally scaled by `0.9f` at `009D1782`.

Its section (4) identified the `|cos|` interpolation correctly but attributed it to the slot the
test's interpolation parameter comes from. That expression writes `F=20h` at `009D1FF6` and feeds
the aim-solution byte `approach+130h` at `009D2021`; it never reaches `009D2318`. The conclusion
drawn from it - that the second clause's right operand "is not a time" - is therefore wrong, and
`docs/TORPEDO_APPROACH_UPDATE.md`'s original reading of that operand as a time was right.

Its section (1) claim that the eight flagged sites need a cumulative 44-byte correction is correct
as far as it goes but insufficient: the twelve `00419010` windows need their `RET 14h` credited as
well, which is the missing 20 bytes at the test.

### Correction to `docs/TORPEDO_APPROACH_UPDATE.md` section (4)

That section lists the aim tick as `coverage: partial` and says the four `InterpolateClamped`
chains that fold the bank, the altitude and the time into the throttle are "transcribed only to
their constants". All thirteen `00419010` sites are transcribed here, in section (4) and in
`src/torpedo_aim_tick.cpp`. Its command-block table is otherwise confirmed, with two additions:
`009D1C5B` in that table is `009D1D39` (`approach+60h`), and `009D1F36` writes `cmd+264h` only on
the `t < 1.5` arm.

## no_ghidra_function

None. Every routine read for this packet has a Ghidra function: `009D15F0`, `009D1500`,
`009D1360`, `009D31B0`, `009D3150`, `00419010`, `00438AA0`, `00438B10`, `00903860`, `009FA3A0`,
`00427EB0`, `00414DB0`, `007F0280`.

## Validation

**Which tree these numbers are for.** This worktree's merge-base with `main` is `ea2c0b05e`. It
does not contain the ship AI goal-vector gate (`7cfe2db04`, merged as `48bd5339d`) that makes the
ring scan run, nor the moved USN02 standing census (`bda1a32b8`). Both censuses below are for the
`ea2c0b05e` baseline and must not be compared against a tree that carries those merges, where the
USN02 gunnery numbers move to `shots=853 hull=119 deaths=3 total_damage=12463.2`.

* `build-tested`: `./scripts/build.ps1`, Win32 `/W4 /WX`, clean.
* `game-validated` on USN01, 3200 frames, 3000 mission frames at 0.05 s. Before this packet the
  five ordered aircraft spent 445-535 ticks in `aim` and never left it. After, all five write
  `state+2Ch`:

| aircraft | aim ticks | first true at | clause | F34 | F14 | ramp | F18 | F0C |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Mav1 | 181 | 181 | turn | 80.55 | 1407.97 | 250.00 | 2.5165 | 2.5133 |
| Mav2 | 167 | 167 | turn | 78.04 | 1444.05 | 250.00 | 2.5755 | 2.5734 |
| Mav3 | 171 | 171 | turn | 78.90 | 1436.67 | 250.00 | 2.5612 | 2.5611 |
| Mav4 | 173 | 173 | turn | 78.86 | 1424.40 | 250.00 | 2.5461 | 2.5407 |
| Mav5 | 178 | 178 | turn | 79.69 | 1414.94 | 250.00 | 2.5305 | 2.5249 |

  Every aircraft leaves `aim` for `goaway` on the tick the byte goes true, and three of the five
  reach `prepare` once. Releases stay at 0.

* USN02, same invocation: `shots=734 hull=180 deaths=2 total_damage=18525.6`, unchanged against
  the `ea2c0b05e` baseline. Method limitation: that run was made after the source change, and the
  reference came from the packet brief rather than from a before-run in this worktree. The after
  run reproduced all four numbers exactly, so the check holds for this tree, but this packet did
  not independently measure the before state.

* The next gate, by address and value: **`009D3150`**, the goaway-done predicate. It returns
  `approach+90h > goaway_state+24h` at `009D3195`; the host binding in
  `src/game_hosts_units.cpp` still reports a constant `false`, so `009D4132` never promotes
  `goaway` to `done` or back to `aim`, `009D49A0` never reaches `prepare+98h`, and `009D2720`
  never calls `007BBBA0`.

## Uncertainty

* All five aircraft set the byte through clause 2 with `F18` around 2.5 rad, a 144 degree steering
  delta. The commanded heading the tick writes into `cmd+2C0h` is not yet consumed by the host's
  pilot binding, so the aircraft do not turn onto it; on the native the steering delta should
  shrink and clause 1 should be the one that fires. The rule is listing-backed either way, but the
  clause the host exercises is not the clause the native is likely to exercise.
* The obstacle probe `007F0280` is a contract. Its branch is entered only when `approach+AAh` is
  set; the host reports no hit, which is also what the `approach+AAh == 0` arm produces.
* `approach+8h`, the aircraft description, is not modelled: `+268h`, `+26Ch`, `+25Ch`, `+A4h`,
  `+188h` and `+1ACh` carry contract values. None of them reaches the aim-complete test; they feed
  the turn-radius escape, the throttle and the pitch.
* `approach+84h` is not in `TorpedoApproachState` yet; the host passes `1.0f`. It only shapes the
  aim-solution byte `approach+130h`.
* The `009D1500` listing leaves one x87 register live across its `009D15AD` return on the
  `speed < 600` arm. That is in the callee, not this tick, and does not change its value.
* Descriptive names are hypotheses, not recovered symbols.

## Follow-up packets

1. **`goaway` tick and `009D3150`** - the named next gate. It needs `goaway_state+24h`, which the
   goaway tick writes, and the `ctl+369h` / `[00E17BF2]` pair.
2. **The command-block consumer** - `cmd+2C0h`/`+2C8h`/`+2BCh` are written every aim tick and read
   by nothing in the host, which is why the steering delta never shrinks.
3. **`007F0280`** - the obstacle probe, 611 instructions, `RET 18h`, six arguments.
4. **`approach+8h`** - the aircraft description fields `+268h`, `+26Ch`, `+25Ch`, `+A4h`, `+188h`,
   `+1ACh`, all read by this tick.

## Correction from `docs/TORPEDO_GOAWAY_RELEASE.md` (packet `cc8_torpedo_goaway_release`)

Appended, not rewritten. The two follow-ups this document opened are answered there.

* **Follow-up 1, `goaway_state+24h`.** Its only producer is the goaway state's enter,
  `009D0D90`, which stores it at `009D0E37` as
  `UniformFloatRange(1.0, 1.15) * max(Pilot/Torpedo/SafeDist at 0042E740()+438h, 007B5BE0(target))`.
  The registrar's inline construction at `009D2ECC`-`009D2F09` skips the field and the vtable's
  other entry slot `009D0C00` is a bare `RET`, so nothing else writes it.
* **Follow-up 2, the `cmd+2C0h` consumer.** `plan+2C0h` with `plan+2CCh = 2` is the plan pair the
  pilot planner's yaw base term reads at `0099DEB8`; the aim tick writes exactly that pair at
  `009D1D16`/`009D1D1E`. The host now carries it and the yaw arm prefers it over the raw target
  bearing, which is the native's own precedence.

The Uncertainty note above, that the host exercises clause 2 while the native would likely
exercise clause 1, is measured in that document's Validation section.

## Correction, packet cc8_torpedo_steering_delta

Appended, not a rewrite of anything above.

The frame table's `F=10h` entry names the subtrahend of the delta at `009D1699` as the
unit's heading through vtable slot `50h`, and slot `50h` was recorded as
`006DFD60 BSP_UnitInstance_GetHullHeading`, `FLD [ECX+1050h]`. That is the **ship**
override. Slot `50h` has a second override:

```
0074e260  d9 81 6c 0c 00 00   fld dword ptr [ecx + 0xc6c]
0074e266  c3                  ret
```

`0074E260 BSP_PlaneUnitInstance_GetHeading` is slot `50h` of nine vtables, one of which is
`00D05F20`, the vtable `BSP_PlaneUnitInstance_Construct` (`007CFD20`) installs at `007CFD78`.
So on an aircraft the `CALL EDX` at `009D1679` returns `unit+C6Ch`, the plane heading
`007C1900` writes, not `unit+1050h`.

That matters because `unit+C6Ch` is exactly the field the pilot planner subtracts at
`0099DEB8`. The aim tick's delta and the planner's yaw heading error are the same number by
construction, so there is no convention mismatch between the bot state and the planner.

docs/TORPEDO_STEERING_DELTA.md has the vtable census and the ABI.

## Correction, packet cc8_torpedo_run_profile

Appended, not a rewrite of anything above.

`F=0Ch`, the slot `009D160F` stores from `009D1500`, is a **range ratio**, not a time to
target. `009D1500` divides `approach+90h`, a range in metres, by whichever of `approach+7Ch`
and `+80h` the 15-second switch selects, and those two are release distances:
`TorpReleaseDistNear` and `TorpReleaseDistFar` from `&PilotBotConfig.levels[idx]`, scaled by
`max(1.0, desc.MaxSpd / Pilot/Torpedo/ReferenceSpeed)`. docs/TORPEDO_RUN_PROFILE.md has the
producer at `009F9CE0`.

Every gate this document describes as interpolated over `F=0Ch` is therefore scheduled on
range measured in release distances, not on seconds: the cone at `009D21AC` and `009D21F2`,
the countdown at `009D229D`, the sector gain at `009D17D7` and the lead gain at `009D182C`.
The numbers in those interpolations are unchanged; only what the x axis means changes.


## Correction from packet `cc8_plane_pose_throttle_altitude`: `009D1BB8`-`009D1D39` is not a throttle

Appended, not rewriting the text above.

Section (4)'s row for `009D1BB8`-`009D1D39` calls the product "the throttle", and
`include/bsp/torpedo_aim_tick.hpp:244` names its output `commanded_throttle_2c8`. The command
block slot it writes at `009D1D2E` is `plan+2C8h`, and `plan+2C8h` is the per-task **bank-angle
cap**:

* `docs/PILOT_PLANNER_PITCH_ROLL.md` section (2) transcribes `0099E27B`:
  `if (plan+2C8h < pi) plan+2C4h = ClampInPlace(plan+2C4h, -plan+2C8h, +plan+2C8h)`, where
  `plan+2C4h` is the bank target. `src/plane_ai_control.cpp:517-519` already implements it, reading
  `PilotBotRollInputs::bank_limit_2c8`.
* The same doc's note 6 names the producer side: `0099B55E` resets `plan+2C8h` to `20.0f`,
  deliberately above `pi` so the clamp is inert, and task arms opt in. It lists "the seven
  task-side `plan+2C8h` writers" as open. `009D1D2E` is one of them.
* The arithmetic agrees. The product's own base factor is `desc+25Ch`, which
  `src/plane_class_fields.cpp:131` names `TurnRoll` from its writer `007D289B` and that doc's note
  7 reads as "a maximum bank angle" - 1.047198 rad on this installation's TBD Devastator, 1.22173
  on the row that doc quotes. The ceiling at `00CE3814` is 1.2, and 1.2 **radians** is 68.8
  degrees of bank; as a throttle ceiling 1.2 of full would be meaningless.

So the four folds schedule how far the aircraft may bank during the run-in: the `unit+C64h` fold
opens the cap when the nose is down, and the time and altitude folds close it to a tenth as the
aircraft gets low and close. The uncertainty this doc's own section carried about `desc+25Ch` -
whether it is `TurnRoll` or a cruise speed - is settled for `TurnRoll`.

`include/bsp/torpedo_aim_tick.hpp`'s field keeps its name here because renaming it is the aim
tick owner's call; `src/game_hosts_units.cpp` now publishes it into
`plan_state.bank_limit_2c8` with the reasoning in a comment.

Two other writes in the same block, also checked against the listing:

* `cmd+2E8h` at `009D1D02` takes `[00D06874]` = **1.4**, not `0.0`.
* `cmd+2BCh` at `009D1EDD` is the pitch target and `cmd+2D0h` at `009D1EE5` the mode `0099E3D1`
  gates the pitch law on. Its value is `clamp(-f34 / den, 0.05625, 0.872665)` where `f34` is the
  height **above** the altitude floor, so the quotient is negative for a high aircraft and the
  clamp floors it at `0.05625` rad. It is a nose-up floor and a pull-up, never a descent command.
  Nothing in this tick brings a torpedo bomber down.

## Correction, packet `cc8_torpedo_release_timer`: the commanded pitch is a DIVE, and its lower clamp is `-DEG(80)`

Section (4)'s row for `009D1DD6`-`009D1EE5` reads
`clamp(-F34 / max(range - 1200, obj+188h * denom), 0.05625, 0.872665)`.

* **was**: a lower bound of `0.05625`, which with a negative quotient makes `cmd+2BCh` a positive
  3.2-degree nose-up on every tick an aircraft is above its altitude floor - the reading that
  `docs/PLANE_POSE_THROTTLE_ALTITUDE.md` section 4 and `docs/TORPEDO_RUN_IN_DESCENT.md` section 5
  both took, and that `src/game_hosts_units.cpp` summarised as "nothing in the aim tick brings a
  torpedo bomber down".
* **is**: `clamp(-F34 / pitchDen, -1.3962634, +0.8726646)`. `00D21318` holds the float
  `0xBFB2B8C3` = `-1.3962634` = `-DEG(80)`; `0.05625` is the **double** at the same eight bytes, and
  neither of the two instructions that read it is eight-byte - `009D1EA6 FLD float ptr [00D21318]`
  and `009D1EB0 MOVSS XMM0,dword ptr [00D21318]`. The routine is the aim state's **descent**: `F34`
  is the height above the release floor, `009D1E98 FCHS` negates it, and the command eases to zero
  as the aircraft reaches the floor.
* **evidence**: the listing in `docs/TORPEDO_RELEASE_TIMER.md` section 1, and the contrast with the
  high bound one instruction later, which the compiler did emit at both widths (`00D057E0` double
  for `FLD double ptr`, `00D05B40` float for `MOVSS`).

**The constant convention this document's header follows is what catches it.** Every constant in
`include/bsp/torpedo_aim_tick.hpp` that needed a width check carries two addresses or a named site.
`kPitchClampLo` carried a bare address and no site; it was the only one that did, and the only one
that was wrong. Every other constant in the file has now been re-read from the image at both widths
and checked against its load, and they are all correct.

This also settles section (4)'s neighbour: the altitude flag at `009D20C4` wants the aircraft under
25 to 40 metres, and the pitch write at `009D1EDD` is the only thing in the whole torpedo chain that
takes it there once the in-range latch has closed. Follow-up 1 of the "Follow-up packets" list -
the cone flag and `F18` - is no longer the whole remaining distance to a drop; the altitude flag was
in front of it.
