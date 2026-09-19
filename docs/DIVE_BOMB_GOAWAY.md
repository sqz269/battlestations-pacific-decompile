# The dive-bomb goaway: the climb the host never commanded

Packet `cc8_dive_goaway`, branch `agent/cc8-dive-goaway`. This is the state six of fifteen USN04
dive bombers reach after their dive aborts at `009C5B43`, and the state they never leave.
Predecessor: `docs/HANDOFF_DIVE_BOMB_FLYOVER.md` section (b).

## 1. `009C4A40` read whole, and the retraction it forces

**That `009C4A40` is the goaway tick is proved, not assumed.** It has no callers; its only reference
is the data reference at `00D20CAC`, i.e. slot `+Ch` of the vtable at `00D20CA0`, which is the slot
`009C884C` dispatches. `00D20CA0` is installed once, at `009C74EE` in the task constructor
`009C73A0`, as `MOV dword ptr [ESI+30Ch], 0xd20ca0`. The same constructor installs `00D20CE8` at
`[ESI+33Ch]` and `00D20CC8` at `[ESI+35Ch]`, and the known state offsets `kGoAway = 704h`,
`kAimDive = 734h`, `kAimGlide = 754h` each sit exactly `3F8h` above those three - three independent
agreements on one base. So `ESI = task + 3F8h`, `[ESI+30Ch]` is `task+704h`, and `009C4A40` is the
`kGoAway` tick.

The goaway tick is `009C4A40`-`009C4D9D`+ (273 instructions, `local\goaway_tick.lst`,
`goaway_tick2.lst`, `goaway_tick3.lst`). Its climb-out computes **two** interpolated pitch curves
and takes the larger. `00419010` is `InterpolateClamped(x0, y0, x1, y1, x)`; the five floats go on
the stack at `[ESP]`, `[ESP+4]`, `[ESP+8]`, `[ESP+0Ch]`, `[ESP+10h]` in that order, and `00419010`
cleans them (the frame is consistent only under callee cleanup: the flag stored at `[ESP+0Fh]` on
entry is read back at `[ESP+13h]` after the single `PUSH EBP` at `009C4B05`, with both `SUB ESP,14h`
blocks balanced by the callee).

**Curve A, `009C4B32`-`009C4B61`** - the one this host does not have:

| slot | address | value |
| --- | --- | --- |
| `x0` | `009C4B58` | `00CECA0C` = **-50.0** |
| `y0` | `009C4B52` | `FLDZ` = 0.0 |
| `x1` | `009C4B48` | `00CE3AE8` = 300.0 |
| `y1` | `009C4B3E` | `[EAX+1ECh]` = the class climb angle (0.1872 rad on the D3A in this installation) |
| `x`  | `009C4B36` | `[ESP+30h]` = **`ceiling - [EBP+100h]`**, the altitude DEFICIT |

`ceiling` is built at `009C4ACF`-`009C4B05` as `min(ctl+398h, approach+ACh + approach+50h)` - the
same ceiling `009C7F00` completes against - and `009C4B26 FSUB float ptr [EBP+100h]` subtracts the
aircraft's world Y from it. So curve A is **zero when the aircraft is 50 m above its ceiling and
rises to the full class climb angle once it is 300 m or more below it**. It is the climb-back-to-
cruise arm, and it is the whole reason the state can finish.

**Curve B, `009C4B8C`-`009C4BB3`** - the one this host already had:
`InterpolateClamped(60.0 [00CEB4B0], climb_angle_1ec, 300.0, 0.0, [EDI+100h])`. Full climb angle
below 60 m, **zero at and above 300 m**, interpolant the raw altitude. It is the ground-avoidance
arm, not the climb-out.

**The max**, `009C4BBC`-`009C4BD8`: `FLD [ESP+1Ch]` (curve B, stored `009C4BB8`), `FLD [ESP+14h]`
(curve A, stored `009C4B66`), `FCOMIP ST0,ST1`, `JA` takes `[ESP+14h]`. Result = `max(A, B)`, written
to `cmd+2BCh` at `009C4BE0` with mode 1 at `009C4BE8`, both unconditional.

**RETRACTION.** `include/bsp/dive_bomb_task.hpp` (the `009C4B44` block) and
`dive_bomb_goaway_climb_009c4b44` label curve A "a second curve over the same 300.0 upper endpoint
whose `y1` and interpolant were not traced" and substitute curve B for it, so that `max(A,B) = B`.
Both halves of that are now wrong: curve A is the *first* call in program order, its `y1` is the
class climb angle and its interpolant is the altitude deficit, and substituting B for it deletes
exactly the arm that climbs. The addresses in that comment are also crossed: `009C4B48`/`009C4B96`
are the two `300.0` loads, one in each curve, and `009C4BB3` is curve B's call, not curve A's.

Consequence, from the arithmetic alone: an aircraft leaving the dive at 163 m climbs on curve B to
300 m and is then commanded **0.000 rad - level flight - forever**, because curve B is zero above
300 m and curve A is absent. `009C7F00` needs `altitude > ceiling - 100` = 900 m. The state cannot
complete. That is the 761-841 goaway ticks.

### Coverage

`009C4A40`'s Ghidra body is `009C4A40`-`009C4E65`; `ghidra proto` puts `009C4B61` and `009C4E17`
both inside it, so every address quoted here belongs to this function.

| range | what | coverage |
| --- | --- | --- |
| `009C4ACF`-`009C4B05` | the ceiling | complete, bound |
| `009C4B32`-`009C4BD8` | the two climb curves and their max | complete, bound |
| `009C4A43`-`009C4A68`, `009C4BD8`/`009C4BEF` | the -5 deg nose-down flag | complete, **read but not consumed** |
| `009C4BF5`-`009C4C06` | the wings-level pair | complete, bound (ungated - see below) |
| `009C4A6D`-`009C4ACD`, `009C4CF1`-`009C4D9A` | the `+24h`/`+28h`/`+2Ch` timers | **partial: not reconstructed** |
| `009C4C0C`-`009C4CA7`, `009C4CBA`-`009C4CE7` | the throttle/air-brake slots `+278h`/`+2A8h` | **partial: not reconstructed** (this host keeps no such slots) |
| `009C4D9D`-`009C4E63` | the bank arm and the heading arm via `009C47D0` | **partial: not reconstructed** |

So `dive_bomb_goaway_climb_009c4b44` is a **partial projection of the tick**: it is the climb command
and nothing else, and the evasive-turn half of the state is left for the next packet.

## 2. Two more things the host gets wrong in this tick, both smaller

* **The nose-down gate.** `009C4A43`-`009C4A68` sets a byte from
  `COMISS xmm0, [EAX+C64h]` against `00CF885C` = **-0.0872664675 rad** (-5 deg): the byte is 1 only
  while the aircraft's pitch is *steeper nose-down than -5 deg*. `009C4BD8 CMP byte ptr [ESP+13h],0`
  / `009C4BEF JZ 009C4CBA` splits on it. The wings-level pair `cmd+2C4h = 0` / `cmd+2CCh = 1`
  (`009C4BFE`/`009C4C06`) and the throttle shaping at `009C4C0C`-`009C4CA7` are on the **taken-flag**
  side only. The host writes the wings-level pair on every goaway tick.
  On the flag-0 side `009C4CBA`-`009C4CE7` writes `cmd+278h = 1.0`, `cmd+27Ch = 1`, `cmd+2A8h = 0.0`,
  `cmd+2ACh = 1`, `cmd+2D8h = 0` - full throttle and no air brake - and then keeps going.
* **The state's own timers**, `009C4A6D`-`009C4ACD` and `009C4CF1`-`009C4D9A`: `+24h` counts down by
  `dt` while `(+20h - 100) > approach+BCh`, `+28h` accumulates `dt`, and when `approach+C4h < 1.0`
  and `+28h > +2Ch + 6.0` the tick forces `+24h = -1.0`; a negative `+24h` then re-rolls `+2Ch` and
  `+24h` through two `00BD2F10` random-range calls and an `00419010` altitude curve.

  **CORRECTION, made before this was shipped.** A first draft of this section said the timers "write
  no command slot, only the state's own fields". That is false, and the tail `009C4D9D`-`009C4E63`
  says so. The flag-0 path does not end at `009C4CE7`; it reaches `009C4D9D` and splits at
  `009C4DAD` on `+2Ch` against `+28h`:

  * `+2Ch > +28h`, `009C4DAF`-`009C4DF6`: a **bank** arm. `+28h * +18h`, doubled (`FADD ST0,ST0`),
    clamped above by `00D05EA4` and below by `00CE3814`, written to `cmd+2C4h` with `cmd+2CCh = 1`.
  * `+2Ch <= +28h`, `009C4E05`-`009C4E1D`: a **heading** arm. `009C47D0` fills the state's `+1Ch`,
    which is then written to `cmd+2C0h` with `cmd+2CCh = 2`, followed by `0042E740`, `0099B630` and
    `009FABE0` for the speed side.

  So the goaway is an **evasive turn** state, not just a climb-out: it banks for the window the
  timers define and then flies a heading `009C47D0` computes. Those timers are still **not bound in
  this packet**, and because of that the nose-down gate above is deliberately **not applied** to the
  wings-level pair in the host: gating the pair off without binding the bank and heading arms that
  replace it would leave the goaway with no lateral command at all, which is further from the image
  than the unconditional wings-level the host writes today. The flag is computed and carried on
  `DiveBombGoAwayCommand::wrote_bank_heading` as evidence, and left unconsumed. This is the next
  binding in the goaway after the climb.

## 3. Prediction, written before run 1 (census only, no behaviour change)

Run 1 adds a `divebomb ... goaway 009C4A40` census line and changes no logic. Predicted, for the six
aborted aircraft (`#3.1` x3, `#7.1` x3):

1. `pitch_cmd` last = **0.000 rad** for every one of them (all are above 300 m by the end).
2. Altitude rises from the 82-174 m hand-over to **~300 m** and then holds flat to the last tick;
   `alt_max` ~300 m, never above ~310 m.
3. `complete=0` on every tick; the ceiling reads 1000.0 m and the deficit stays ~700 m.
4. Every behavioural column matches `pullout_after.log` exactly: releases 19, bomb_impacts 18,
   total_damage 9820.7, deaths 8, `states[goaway=841/761/830/150/89/139]`.

A miss on 4 means the census is not print-only and the pair is void.

### 3a. Run 1 measured, `local\goaway_before.log`, binary with the census only

```
divebomb D3A Val #3.1     goaway 009C4A40: ticks=841 pitch last=0.0162 max=0.1098 rad | alt first=162.9 last=279.2 max=280.2 m | ceiling=1000.0 deficit=720.8 | complete_ticks=1213
divebomb D3A Val #3.1|.-2 goaway 009C4A40: ticks=761 pitch last=0.0187 max=0.1858 rad | alt first=81.6  last=276.0 max=276.9 m | ceiling=1000.0 deficit=724.0 | complete_ticks=1283
divebomb D3A Val #3.1|.-3 goaway 009C4A40: ticks=830 pitch last=0.0179 max=0.1141 rad | alt first=158.5 last=277.0 max=280.1 m | ceiling=1000.0 deficit=723.0 | complete_ticks=1224
divebomb D3A Val #7.1     goaway 009C4A40: ticks=150 pitch last=0.0546 max=0.1061 rad | alt first=171.1 last=230.0 max=230.0 m | ceiling=1000.0 deficit=770.0 | complete_ticks=1109
divebomb D3A Val #7.1|.-2 goaway 009C4A40: ticks=89  pitch last=0.0801 max=0.1138 rad | alt first=160.9 last=197.4 max=197.4 m | ceiling=1000.0 deficit=802.6 | complete_ticks=1164
divebomb D3A Val #7.1|.-3 goaway 009C4A40: ticks=139 pitch last=0.0559 max=0.1019 rad | alt first=173.9 last=228.3 max=228.3 m | ceiling=1000.0 deficit=771.7 | complete_ticks=1119
```

**Prediction 4 held exactly**: `total_damage=9820.7`, `deaths=8`, `bomb_impacts=18`, and the torpedo
summary line all identical to `pullout_after.log`. The census is print-only and the pair is valid.

**Prediction 1 missed, and the miss is the sharper result.** I predicted `pitch last = 0.000` because
I expected the aircraft to be above 300 m. It is not: it never gets there. The measured commands are
exactly curve B evaluated at the measured altitude, to three significant figures:

| aircraft | alt last | curve B = 0.1872 * (1 - (alt-60)/240) | measured `pitch last` |
| --- | --- | --- | --- |
| `#3.1` | 279.2 m | 0.01622 | **0.0162** |
| `#7.1\|.-2` | 197.4 m | 0.08004 | **0.0801** |

So curve B is confirmed by measurement as the only arm the host was running, and its shape makes
300 m an **asymptote, not a shelf**: the command decays to zero as the aircraft approaches 300 m, so
841 ticks of climbing get it to 280.2 m and it would never arrive. `pitch max` (0.10-0.19 rad) is the
command at the hand-over, where the aircraft was lowest; `pitch last` is the command at the end.

**Prediction 3 half missed, on the census rather than the sim.** Ceiling 1000.0 m and deficit
720-803 m are as predicted. `complete_ticks` is **not** 0 - but read its printing code: the counter
sits in `dive_bomb_transition_inputs`, which runs on **every tick of the arm**, not only goaway
ticks. `#3.1` has `arm_ticks=2138` and `goaway=841`, so the 1213 true ticks are essentially the 1297
non-goaway ticks, when the aircraft was at attack-run altitude and the rule trivially held. During
goaway itself, at 82-280 m against a 900 m gate, the rule was false on every one of the 841 ticks -
which is what the 841 ticks with no exit already prove. The column is left as it is so that the
before/after pair differs only by the change under test; it must be read as "ticks of the whole arm",
never as "ticks of goaway".

## 4. Prediction, written before run 2 (curve A bound)

1. `pitch_cmd` at the hand-over = curve A clamped to **0.1872 rad**, easing below that only once the
   aircraft is within 300 m of the 1000 m ceiling.
2. Altitude climbs continuously instead of levelling at 300 m.
3. Whether `009C7F00` completes inside 4800 frames is **not** predicted: `#3.1` has 841 goaway ticks
   = 42 s, and 700 m of climb at `V*sin(0.1872)` needs V >= ~89 m/s sustained, which is marginal;
   `#7.1` has only 89-150 ticks and certainly cannot. If the climb is right and the clock is short
   that is outcome (ii) and the demonstration is a longer run, not a constant change.

### 4a. Run 2 measured, `local\goaway_after.log`, curve A bound

Same binary except curve A; same `--frames 5000 --press-start-frame 30 --menu-select USN04
--mission-frames 4800 --mission-frame-seconds 0.05`.

| aircraft | goaway ticks | alt last BEFORE | alt last AFTER | `pitch max` AFTER | final deficit |
| --- | --- | --- | --- | --- | --- |
| `#3.1` | 841 | 279.2 m | **866.8 m** | 0.1872 | 133.2 m |
| `#3.1\|.-2` | 761 | 276.0 m | **749.1 m** | 0.1872 | 250.9 m |
| `#3.1\|.-3` | 830 | 277.0 m | **856.1 m** | 0.1872 | 143.9 m |
| `#7.1` | 150 | 230.0 m | **334.4 m** | 0.1872 | 665.6 m |
| `#7.1\|.-2` | 89 | 197.4 m | **248.7 m** | 0.1872 | 751.3 m |
| `#7.1\|.-3` | 139 | 228.3 m | **326.3 m** | 0.1872 | 673.7 m |

**Prediction 1 held exactly.** `pitch max = 0.1872` on all six - curve A saturates at the class climb
angle for any deficit at or above 300 m. **Prediction 2 held**: `alt last == alt max` for every one of
the six, i.e. a monotone climb to the last tick, where before the altitude asymptoted at 300 m.

Curve A is now confirmed by measurement the same way curve B was, to three significant figures:

| aircraft | final deficit | curve A = (deficit + 50)/350 * 0.1872 | measured `pitch last` |
| --- | --- | --- | --- |
| `#3.1` | 133.2 m | 0.09798 | **0.0980** |
| `#3.1\|.-2` | 250.9 m | 0.16093 | **0.1610** |

**Prediction 3: it does not complete, and it is outcome (ii) by a very small margin.** `009C7F00`
needs `altitude > ceiling - 100` = 900 m. `#3.1` reached **866.8 m, 33.2 m short**, with the command
still at 0.098 rad and decaying. `#3.1` climbed 703.9 m in 841 ticks = 42.05 s, so 16.7 m/s, which
back-solves to about 90 m/s of airspeed - the goaway's full throttle does raise it well above the
66.7 m/s at release, which my pre-run arithmetic had used and flagged as pessimistic. Extrapolating
the decaying command, `#3.1` would cross 900 m roughly 70-80 ticks (about 4 s) after the mission
ends. **No constant was touched to close that gap**; the demonstration is the longer run in section
4b. `#7.1`'s flight, with 89-150 goaway ticks, was never going to make it.

**The behavioural consequence, which is not confined to the six aircraft.** Every dive-bomb state
count, every `releases`, and `bomb_impacts=18` are identical between the two runs - the change moves
no dive-bomb transition. But the gunnery totals move:

| | before | after |
| --- | --- | --- |
| `total_damage` | 9820.7 | 10188.4 |
| `hit_records` / `hull` | 61 / 41 | 77 / 53 |
| `deaths` | 8 | 9 |
| `entity_impacts` | 46 | 60 |

The ninth death is one of the six: `D3A Val #3.1` goes from `hits 1 taken 29 health 191` and alive,
to `hits 9 taken 220 health 0`, `killed_by Northampton-class04`. The ship outcomes are unchanged -
`Lexington-class01` is still sunk by `B5N Kate #6.1|.-2`. So **binding the climb flies the aborted
bombers back up through the escorts' AA envelope and costs one of them**. That is what the image
does too, so it is a faithfulness gain rather than a regression, but it must not be reported as
free. (The `sunk_at` column's clock was not reconciled against the per-arm tick numbering, so the
timing is not quoted here; the `taken`/`health`/`killed_by` columns are unambiguous.) One knock-on
worth flagging: `#1.1`'s `dealt` moves 432 -> 492 although it never enters goaway, because the AA
assignment is a coupled simulation - a changed target set changes the whole gunnery trajectory.

### 4b. Run 3, `local\goaway_long.log`, 9000 mission frames - the outcome (ii) demonstration

Same binary as run 2, `--frames 9200 --mission-frames 9000`. **This is a demonstration, not a paired
measurement**: two run parameters changed, so nothing in it may be differenced against the pair.

**The state machine result, which is what outcome (ii) asked for, is unambiguous.** All six aborted
aircraft climb to the completion threshold and leave:

```
#3.1      goaway ticks=924  alt last=900.0 max=900.0  deficit=100.0  pitch last=0.0803
#3.1|.-2  goaway ticks=1860 alt last=899.7 max=899.7  deficit=100.3  pitch last=0.0804
#3.1|.-3  goaway ticks=1809 alt last=899.8 max=899.8  deficit=100.2  pitch last=0.0803
#7.1      goaway ticks=902  alt last=899.9 max=899.9  deficit=100.1  pitch last=0.0803
#7.1|.-2  goaway ticks=938  alt last=900.0 max=900.0  deficit=100.0  pitch last=0.0803
#7.1|.-3  goaway ticks=895  alt last=899.8 max=899.8  deficit=100.2  pitch last=0.0803
```

`alt max` is 899.7-900.0 m on every one of them and never higher, which is exactly `ceiling - 100`:
the state ends on the tick the altitude first exceeds `009C7F00`'s threshold, so no goaway tick ever
observes a higher altitude. The command at that point is still 0.0803 rad - curve A at a deficit of
100 m is `(100+50)/350 * 0.1872 = 0.08023` - so the climb is not running out, the state is finishing.

And the second attack run follows, on all six:

| | run 2 (4800 frames) | run 3 (9000 frames) |
| --- | --- | --- |
| `transitions` | 6 | 10-15 |
| `turndown` ticks | 56-57 | 111-167 |
| `aimdive` ticks | 60-66 | 105-174 |
| `releases` | 1 | **2** |
| `rounds_left` | 1 | **0** |
| ends in | `goaway` | `done` (755-1912 ticks) |

So the chain closes: goaway completes at 900 m, `009C86EE` returns an aircraft with bomb ordnance to
`flyabove`, it flies a second turndown and aimdive, and it releases its second bomb. **No constant
was changed to produce this.** The 4800-frame mission was simply about four seconds too short, as
run 2's 866.8 m predicted.

**What this run may NOT be used for.** Its mission-level numbers diverged from the pair and I did not
isolate why: `bomb_impacts=0` (against 18), `total_damage=3596.8` (against 10188.4), `deaths=5`
(against 9), `queued_hits=272` (against 77), and `Lexington-class01` is not sunk (`taken 2497` of
8000) where the pair sinks it at 123.70 s. `#3.1`'s arm also starts about 2100 frames later in
mission terms. Two candidate causes, neither checked: changing `--frames` changed the pre-mission
frame budget, and I did not pass `--instance-tag`/`--affinity-core` while another worker's runs
overlapped mine. **Quote no damage, impact or death figure from this run.** The state, tick and
release counts above are properties of the dive-bomb task's own transitions and stand on their own.

## 5. The delivered cross-track error: the attack run cannot be making it *here*

Packet item 2. `009C4220`'s heading arm, `009C42A6`-`009C4305`, read from
`local\attackrun.lst`:

```
009C42B8  CALL 007F0280          BSP_Bot_NearFieldUnitAvoidanceProbe, out-vector at [ESP+2Ch]
009C42BD  FLD [ESP+34h] / FCHS / FMUL [ESP+2Ch] / FMUL [ESP+30h]
009C42D3  FMUL 00CEC730 = 0.5235987901687622 = pi/6 = 30 deg
009C42D9  FSTP [EDI+20h]         the state's lateral offset
009C42DC  FLD [EDI+20h] ... CALL 00438AA0   BSP_Math_AddWrappedAngle(bearing, offset)
009C42FF  cmd+2C0h = result      009C4305  cmd+2CCh = 2
```

So the image's attack run does **not** steer at the aim point. It steers at the bearing **plus a
lateral avoidance offset of up to +/-30 degrees**, re-rolled on the `+1Ch` timer, and the offset is
`-probe.z * probe.x * probe.y * pi/6` from the near-field unit-avoidance probe.

In this host that probe is a **labelled substitution to zero** - `game_hosts_units.cpp`,
`in.sampler_result = 0.0f; in.sampler_ran = false`, commented "the run-in flies straight at the
target rather than weaving". `dive_bomb_attackrun_tick_009c4220` then computes
`wrapped_angle_add_00438aa0(bearing, 0)`, i.e. the bearing exactly. **The host's attack run commands
zero lateral offset for every aircraft in every squadron, so it cannot be the origin of a
per-squadron cross-track error**, and equally it is not the image's run-in.

What the log says instead (`pullout_after.log`), measured:

| | turndown entry bearing | aimdive entry bearing | cross at fly-over | attackrun ticks |
| --- | --- | --- | --- | --- |
| `movieval`, `#1.1`, `#5.1` | 0.060 rad | 0.269 rad | 109-111 m | 1144-1564 |
| `#3.1` | 0.186-0.207 rad | 0.509-0.553 rad | 409-449 m | 1073-1141 |
| `#7.1` | 0.146-0.191 rad | 0.444-0.520 rad | 330-421 m | 973-1023 |

Two things follow. First, the bearing error is **non-zero for everyone** (0.060 rad even on the
clean squadrons) and the turndown **amplifies** it by 2.7x-4.5x for everyone; the bad squadrons are
not doing something different in kind, they enter the turndown with 3x the seed. Second, all four
D3A squadrons enter the attack run with the same geometry - `moveto>attackrun@0 alt=1500/1475/1525
rng=8198-8274` for every one of the twelve - so the seed is not a different starting point either.
`#1.1` and `#5.1` are identical to the metre in every geometry column despite `#5.1`'s arm running
790 ticks fewer, and `#3.1`/`#7.1` are the pair that differs.

This host's run-in is therefore pure pursuit of the commanded target's **live position** - the aim
point here is `slots[ti]->motion.position`, refreshed every tick, with no lead and nothing frozen
(`update_dive_bomb_approach`, the labelled substitution `+BCh`/`+C0h` already carry) - against
moving carriers, and the residual is a pure-pursuit lag. **Attributing the off-bearing dive entry to
the image's attack run is not possible until `007F0280` is bound**, because the one term the image's
heading has and this host's does not is exactly a lateral term. That is the next binding in this
chain, and it is `cc8-dive-geometry`'s function (`docs/BOT_PROBE_007F0280.md`,
`docs/HANDOFF_DIVE_BOMB_PROBE.md`), not a constant to be invented here.

## 6. Arm B of the aimglide pull-out: three slots, two named

Packet item 3, `009C57D3`-`009C57FD`. The frame first, because the handoff could not fix it:
`009C5180 SUB ESP,58h` then four pushes (`EBX`, `EBP`, `ESI` at `009C5185`, `EDI` at `009C5194`)
= `0x68`, and `RET 4`. So the return address is at `[ESP+68h]` and **`[ESP+6Ch]` is the tick's single
stack argument, `dt`** - which `009C5295` onward then reuses as scratch, exactly as the handoff
warned. The frame is stable across all three indirect calls and all four `SUB ESP,14h` argument
windows: there is one `SUB ESP,58h` and one `ADD ESP,58h`, so every callee cleans its own arguments,
and the two reads at `009C5285`/`009C5289` land on the writes at `009C51DD`/`009C51F7`, which
confirms it 2-for-2.

Arm A, `009C57C4`-`009C57D1`: `COMISS [ESP+18h], 00CE380C` with `JA`, i.e. **bearing error >
1.5 rad**. Bound already.

Arm B is three ANDed conditions, all of which must hold:

1. `009C57D3`/`009C57DB` `COMISS 00CE3D30, [ESP+24h]`, `JBE` out: **`[ESP+24h]` < 0.6**.
2. `009C57E8` `COMISS xmm2(0.0), [EAX+C64h]`, `JBE` out: **the unit's pitch is negative** - any
   nose-down attitude, not the -5 deg of the goaway's own gate.
3. `009C57F1`-`009C57FD` `FLD [ESP+6Ch]`, `FLD [ESP+28h]`, `FCOMIP`, `JBE` out:
   **`[ESP+28h] > [ESP+6Ch]`**.

Named here:

* **`[ESP+28h]` is the aimglide state's `+1Ch` re-arm timer.** `009C5188 MOVSS XMM0,[ESI+1Ch]`,
  `009C5195 MOVSS [ESP+28h],XMM0`, and `009C519D`/`009C51A1` `FLD [ESP+28h]; FSUB [ESP+6Ch]` is the
  countdown by `dt` the predecessor bound. Rewritten at `009C54F7` and `009C55B0`.
* **`[ESP+6Ch]` is `dt`** at entry, but at `009C57F1` it holds the last scratch value written to the
  argument slot, not `dt`. Which write reaches `009C57F1` is path-dependent (`009C5295`, `009C52AE`,
  `009C52D7`, `009C52F0`, `009C5319`, `009C5332`, `009C5364`, `009C5372`, `009C5386`, `009C549F`,
  `009C54D1`, `009C54DF`, `009C54EC`, `009C5502`, `009C5563`) and is **not** resolved.
* **`[ESP+24h]` is a ratio of two planar lengths**: `009C534B FLD [ESP+10h]; FDIV [ESP+14h]; FSTP
  [ESP+24h]`. Each length is the guarded `sqrt(a*a + b*b)` idiom - square, add, compare against the
  `00CE3820` epsilon, `CALL 00BF7030` or zero - at `009C5285`-`009C52B6` and `009C52C7`-`009C52F8`.

A first draft of this section said `[ESP+44h]`/`[ESP+4Ch]`, length 2's operands, are read at
`009C52C7`/`009C52CB` with **no store anywhere in the function**, and treated that as the open
question. **Retracted**: it is a frame-offset artifact of reading the raw listing across the
`PUSH`+`CALL` windows. `009C521B PUSH EAX` lowers `ESP` by 4 for the call at `009C5232`, so the
stores at `009C523C`/`009C5256` - printed as `[ESP+50h]` and `[ESP+58h]` in that window - are the
normal frame's `4Ch` and `54h`. The decompilation of `009C5180` confirms both operand pairs are
assigned, and names them: there are in fact **three** guarded `sqrt` sites, not two.

| length | listing slot | operands |
| --- | --- | --- |
| 1, `[ESP+10h]` | `009C5285`-`009C52B6` | `*pfVar9 - unit+FCh` and the matching z |
| 2, `[ESP+14h]` | `009C52C7`-`009C52F8` | **`approach+D8h - unit+FCh`** and **`approach+E0h - unit+104h`** |
| 3 | after `009C52F8` | a third pair, not needed by arm B |

So length 2 is the **planar distance from the aircraft to the point the approach keeps at
`+D8h/+DCh/+E0h`** - the vector `009C7A80` rewrites every tick - and `[ESP+24h]` is
`length1 / length2`. Arm B's first condition is that this ratio has fallen below 0.6.

**Arm B still must not be bound**, but the reason has changed. It is no longer a missing store; it is
that length 1's point comes from the out-vector of the first indirect call (`009C51CC LEA ECX,
[ESP+50h]`, `009C51D3 CALL EDX`), and that virtual is not identified. Naming it names the ratio.
The third condition's `[ESP+6Ch]` is also still path-dependent scratch. Until both are settled the
under-approximation (arm A alone) fires strictly **less** often than the image's, which is the safe
direction.
