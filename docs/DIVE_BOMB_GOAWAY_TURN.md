# The dive-bomb goaway's evasive turn: 009C47D0, the timers and the two arms

Addresses: 009C4A40 (tick; 009C4A6D-009C4ACD, 009C4BD8-009C4BEF, 009C4CBA-009C4E63), 009C47D0, 009C4950 (enter, no Ghidra function, 009C4950-009C4A3C), 009C73A0 (constructor stores 009C74CA-009C7523), 0099B630, 009FABE0, 009FD570, 007F0280, 009D0C10, 009C3FD2, 009C8A74, 009C7A8C

Packet `cc9_goaway_turn`, branch `agent/cc9-goaway-turn`. Successor to `cc8_dive_goaway`
(`docs/DIVE_BOMB_GOAWAY.md`, the climb, and `docs/HANDOFF_DIVE_BOMB_GOAWAY.md` section (b)).
Every read below is from the Ghidra listing (`ghidra disasm`) except where a line says
`disasm-raw` (Capstone over the disk bytes, used where Ghidra has no function). Constants were
read from the image bytes at the width of the instruction that loads them.

## 1. `009C47D0` is the torpedo break-off geometry with the goaway's fields

`009C47D0` is `void __thiscall(goaway state)`, no stack argument, plain `RET` at `009C4943`, frame
`SUB ESP,3Ch` + `PUSH ESI`. Its only caller is `009C4E09`. It **decides the heading, and the side
of the turn is not its own**: it asks `009FD570` (`BSP_Bot_FlyToPointHeading`, reconstructed as
`fly_to_point_heading_009fd570`) for a heading round the target at the state's standoff, then
limits the change to 30 degrees either side of the aircraft's own heading, and stores that
absolute compass heading to `+1Ch`. So `+1Ch` is **an absolute heading in [0, 2pi)**, not an
offset and not a side. The side is `+18h`, which `009FD570` reads through a pointer and may flip.

Diffing the two listings with addresses masked gives six differing instructions and nothing else,
so `009C47D0` is `009D0C10` (`BSP_BotStateTorpedoGoAway_UpdateGeometry`) with other fields:

| | `009C47D0` (dive-bomb goaway) | `009D0C10` (torpedo goaway) |
| --- | --- | --- |
| `009FD570` arg5 range | `FLD [ECX+0BCh]` approach+BCh, planar range (`009C47D9`) | approach+90h |
| arg6 offset scale | `FLD1` (`009C47E9`) | `FLD [00CE74F8]` = 0.8 |
| arg4 `&side` | `LEA EDX,[ESI+18h]` (`009C47EB`) | `ESI+2Ch` |
| arg3 standoff | `FLD [ESI+20h]` (`009C47F9`) | `ESI+24h` |
| `007F0280` mode byte | `PUSH 1` (`009C4824`) | `PUSH 0` |
| store | `FSTP [ESI+1Ch]` (`009C493C`) | `ESI+18h` |

The call before `009FD570` is the approach's `vtable[0]` (`009C480B CALL EAX`, `EAX = [[ECX]]`),
which cleans only its own out-pointer (`RET 4`): the five other `009FD570` arguments are staged
before it and `009C480D PUSH EAX` supplies arg1, the target point. The decompiler shows this as
one six-argument call feeding another; it is not.

The rest, read at `009C4819`-`009C493C`:

* `007F0280` (`BSP_Bot_NearFieldUnitAvoidanceProbe`, ECX = approach+0Ch, arg0 = the unit) with
  half-extents **80 / 50 / 100** (`00CE5444`, `00CEB4D4`, `00CE3D08`, `MOVSS` at
  `009C4819`/`009C482C`/`009C483A`), the weights triple zeroed by `XORPS` at `009C4852`-`009C4866`,
  mode 1. Same frame block as the attack run's `009C42B8` (`docs/BOT_PROBE_007F0280.md` 0.1).
  `probe = -outA[0] * outB[1] * outB[2]` (`009C487D`-`009C4892`, x87, stored at `009C4896`).
* the error `00438B10(flyTo, unit->vtable[50h]())` (`009C48AA`), wrapped to (-pi, pi];
* clamp to **[-0.5236, 0.5236]** (`00CEC728` `FLD dword` low bound first, `00CEC724` `MOVSS`
  high, both `JBE` so an exact bound keeps the value);
* when the clamped turn and the probe have strictly opposite signs, `turn += probe * 0.69813`
  (`00D20CC0`, `FMUL qword` at `009C4910`, i.e. 40 degrees);
* `+1Ch = 00438AA0(unit->vtable[50h](), turn)` (`009C4937`, `009C493C`).

`torpedo_goaway_heading_009d0c10` implements exactly this tail, so the host reuses it; nothing
new was written for it.

## 2. The enter `009C4950`, which seeds three of the five fields

The goaway vtable `00D20CA0` reads `009C7320, 009C4950, 007B3DC0, 009C4A40, 007B3DE0, 007B3DF0`;
slot `+4h` is `009C4950`. **Ghidra has no function there** (`ghidra proto 009C4950 --brief` prints
`009c4950 ?  ?  body ?`, and `ghidra disasm` refuses the address; `lookup` gives only the nearest preceding function, `009C47D0`, whose body ends at
`009C4943`). Decoded with `disasm-raw`, body `009C4950`-`009C4A3C` (`RET` at `009C4A3C`, `INT3`
padding from `009C4A3D`), `void __thiscall(state)`:

| store | rule | address |
| --- | --- | --- |
| `+18h` side | `[00F876B0]` signed remainder by 2: zero -> `+1.0` (`00D7A24C`), else `-1.0` (`00D7A260`) | `009C4950`-`009C497E` |
| `+24h` countdown | `15.0` (`00CE5380`, `MOVSS`) | `009C4983`-`009C498B` |
| `+20h` standoff | `2 * approach+B4h` (`FADD ST0,ST0`) | `009C4990`-`009C4999` |
| | if approach+48h (the target) answers `vtable[5Ch](5)`: `max(+20h, 007B5BE0(target))` | `009C499C`-`009C49E0` |
| | `*= 00BD2F10(1.0, 1.25)` (`FLD1`, `00CF29A8` `FLD dword`) | `009C49E5`-`009C4A10` |
| | `*= 1.5` (`00CE3D78` `FMUL qword`) when `(approach+0Ch)+369h` and `[00E17BF2]` | `009C4A13`-`009C4A2E` |

`[00F876B0]` is the mission step counter (`docs/IN_MISSION_SUBSYSTEM_TICK.md`). The torpedo enter
`009D0D90` uses the same parity.

**`+28h` and `+2Ch` have no writer outside the tick.** The constructor `009C73A0` (ESI =
task+3F8h, state at `ESI+30Ch`) writes `+4h` (`009C74CA`), `+0Ch`-`+14h` (`009C74D0`-`009C74E8`),
the vtable (`009C74EE`), `+24h = 100.0` (`009C750A`), `+1Ch = 0` (`009C751B`) and
`+20h = 2 * approach+B4h` (`009C7523`); the next state's vtable follows at `+33Ch`
(`009C7547`). The enter does not write them either. Their value on the first goaway is whatever
the task allocation held; the allocation is an allocator-vtable call in `0099A170` that was not
followed. The host starts both at 0 and says so.

## 3. The timers, the re-roll and the split, read against the listing

The tick's only argument is `dt` at `[ESP+18h]` (depth 14h after `SUB ESP,8` and three pushes).

```
009C4A6D  FLD  [ECX+0BCh]            ; approach+BCh
009C4A73  FLD  [ESI+20h]
009C4A76  FSUB qword [00D7A220]      ; 100.0
009C4A7C  FCOMIP / 009C4A84 JBE      ; (+20h - 100) > BCh  ->  +24h -= dt  (009C4A86-009C4A8B)
009C4A8E  FADD [ESI+28h]             ; +28h = dt + +28h, FSTP/FLD then FST [ESI+28h] at 009C4AA1
009C4AA4  COMISS [00D7A24C] (1.0), [ECX+0C4h]; JBE    ; only while 1.0 > approach+C4h:
009C4AAD  FLD [ESI+2Ch]; FADD qword [00CE6628] (6.0); FXCH; FCOMIP; JBE
009C4ABE  MOVSS [00D7A260] (-1.0) -> [ESI+24h]        ; +28h > +2Ch + 6  ->  +24h = -1
```

**`approach+C4h` is seeded to 3600.0** (`00CFDEB0`, `MOVSS`) at `009C3FD2` in the approach
constructor `009C3EA0` and at `009C8A74` in `009C8920`, and advanced `+= dt` at `009C7A8C`-`009C7A96` (`FADD`, `FSTP`). A
search of the `009C*` export listings for `+0C4h]` and `+4BCh]` stores finds only those three,
so **the forced re-roll cannot fire on a dive bomber**; the host feeds the 3600.0 floor.

The flag byte (`009C4A43`-`009C4A68`, `00CF885C` = -5 degrees) then splits at `009C4BEF`. The
flag-1 side (nose steeper than -5 degrees) writes the wings-level pair and the throttle shaping
and returns at `009C4CB7`. The flag-0 side writes `cmd+278h = 1.0`, `+27Ch = 1`, `+2A8h = 0`,
`+2ACh = 1`, `+2D8h = 0` (`009C4CBA`-`009C4CE7`) and goes on:

```
009C4CF1  COMISS XMM0(0), [ESI+24h]; JBE 009C4D9D    ; re-roll only when 0 > +24h
009C4D13  00BD2F10(3.0 [00CE3854], 6.0 [00CE6630])   ; ECX = EBX = 1; FSTP [ESI+2Ch]
009C4D33  00BD2F10(12.0 [00CEB4B8], 15.0 [00CE5380]) ; FADD [ESI+2Ch]; FSTP [ESI+24h]
009C4D84  00419010(100.0 [00CE3D08], 1.0 FLD1, 400.0 [00CFD710], 1.5 [00CE380C], unit+100h)
009C4D89  FMUL [ESI+24h]; FSTP [ESI+24h]             ; +24h = (w + draw2) * scale(altitude)
009C4D8F  FLD [ESI+2Ch]; FMUL qword [00D7A280] (0.5); FCHS; FSTP [ESI+28h]   ; +28h = -w/2
009C4DA8  FLD [ESI+2Ch]; FCOMIP; JBE 009C4E05        ; +2Ch > +28h -> bank arm
```

All four random bounds and both scale endpoints are `FLD dword`; `00419010` takes
`(x0, y0, x1, y1, x)` on the stack (`RET 14h`), so the countdown is lengthened by 1.0 at 100 m up
to 1.5 at 400 m and above.

**The bank arm, `009C4DAF`-`009C4DF6`**: `FMUL [ESI+18h]`, `FADD ST0,ST0`, `FSTP`, then the clamp,
**low bound `00D05EA4` = -1.2 first** (`FLD dword`, `FCOMIP`, `JBE`), high bound `00CE3814` = +1.2
(`MOVSS`/`COMISS`). `cmd+2CCh = 1` (`009C4DF0`, EBX) and `cmd+2C4h = bank` (`009C4DF6`). With the
clock starting at `-w/2`, the bank sweeps from `-w * side` through zero to `+2w * side`, clamped:
**a bank reversal, a weave, lasting `1.5 * w` seconds** (4.5 to 9 s).

**The heading arm, `009C4E05`-`009C4E63`**: `CALL 009C47D0`, `cmd+2C0h = +1Ch` (`009C4E17`),
`cmd+2CCh = 2` (`009C4E1D`), then two things that are **not a speed command**, contrary to the
handoff's wording:

* `009C4E27`-`009C4E38`: `0042E740()+674h` is stored to `(approach+1Ch)+40h`. `tuning+674h` is
  `Pilot/AutoStrafeAngle/Angle_GoAway` (`src/game_tuning_singleton.cpp`, loaded at `007E9174`).
* `009C4E3B`-`009C4E58`: `009FABE0(ECX = approach+1Ch; +1Ch, 0099B630(ECX = cmd))`.
  `0099B630` (body `0099B630`-`0099B64C`, leaf): `cmd+2D0h != 0 ? cmd+2BCh : (cmd+2F0h)->+C64h`,
  i.e. the commanded pitch if pitch mode is on, else the live pitch. `009FABE0` (`RET 8`) takes
  `a = pi/2 - heading` (`FSUBR qword 00CE3830`, `+2pi` from `00CE3828` when negative), stores
  `cos a`, `tan(pitch)` (`00412E20`), `sin a` to `+68h`/`+6Ch`/`+70h` of its receiver and
  normalises the three by `00419440`'s length. **A unit direction vector on the approach's `+1Ch`
  object, beside an auto-strafe angle.** This host has no `approach+1Ch` object; not modelled.

## 4. The binding

`include/bsp/dive_bomb_goaway_turn.hpp`, `src/dive_bomb_goaway_turn.cpp` (pure rules, explicit
inputs): `dive_bomb_goaway_enter_009c4950`, `dive_bomb_goaway_timers_009c4a6d`,
`dive_bomb_goaway_reroll_009c4cf1`, `dive_bomb_goaway_turn_split_009c4dad`, and the state
struct `DiveBombGoAwayTurnState` (+18h..+2Ch). In `src/game_hosts_units.cpp`:

* the dispatch runs the enter on the edge into `kGoAway` and passes `dt` to the tick;
* the tick runs the timers on every tick, then consumes `wrote_bank_heading`: nose-down ticks
  keep the wings-level pair and return; the others run the re-roll and the split and write
  either `plan_state.bank_target_2c4` with mode 1 or `plan_heading_2c0` (written) with mode 2,
  the fields the planner's mode-1 servo and heading term already read;
* `goaway_turn_heading_009c47d0` feeds `fly_to_point_heading_009fd570` (point = the fed aim point
  used by the fly-over feed, velocity lead, standoff `+20h`, range `db_planar_bc`, scale 1.0) and
  `torpedo_goaway_heading_009d0c10`;
* the enter writes `db_goaway_travel_20`, which `009C7F00` reads: **the completion rule now sees
  the standoff instead of zero**.

Stand-ins, each labelled at its line: the three random draws at the low end (1.0, 3.0, 12.0) as
this host pins every draw; the step-counter parity from the aircraft's own arm tick count; the
first-entry `+28h`/`+2Ch` = 0; `approach+C4h` = 3600.0 (sufficient for the only test); the
`007B5BE0` target extent absent (as at the torpedo enter); `007F0280`'s probe = 0 and
`009FD570`'s obstacle list empty and world edge far. The throttle/air-brake slots and the
`approach+1Ch` direction are not modelled.

## 5. Predictions, written before the treatment run

USN04, the six D3A Vals that reach goaway in the predecessor's runs (`#3.1` and `#7.1` flights).
From the predecessor's census `approach+B8h = 2080 m = 1.6 r`, so `approach+B4h = 0.6 r = 780 m`
and **the standoff is 1560 m**; the countdown runs while `BCh < 1460 m`; completion now needs
`BCh > 1404 m` as well as `Y > 900 m`.

1. Every goaway aircraft enters with side `+1` or `-1` by arm-tick parity, standoff 1560.0.
2. The first ticks are nose-down (flag 1): wings level as today.
3. Once the nose is above -5 degrees: `clock 28h = dt > window 2Ch = 0`, so **the heading arm from
   the first flag-0 tick** and **no bank tick before the first re-roll**.
4. Heading arm: inside 1360 m `009FD570` points straight away from the target, and between 1360 and
   1860 m the offset ramps towards a tangent, so the aircraft **opens to about 1.6 km and then
   orbits the target** on the `+18h` hand. `heading err_max <= 0.5236` (the 30 degree clamp; the
   probe is zero). `side_writes = 0` (no obstacles, so `009FDC48` cannot run).
5. A re-roll needs 15 s of countdown inside 1460 m. Leaving at ~70 m/s from 300-400 m takes about
   16 s, so **zero or one re-roll per aircraft**, near arm tick `goaway entry + 300`. Each re-roll
   gives exactly **90 bank ticks** (clock -1.5 to 3.0 at 0.05 s), bank min `-1.2` and max `+1.2`.
6. Completion: the climb is unchanged, but orbiting at ~1.6 km instead of flying straight out
   costs no range (1.6 km > 1404 m) and banking costs some climb. **No second attack run inside
   4800 frames**, as in the control; goaway tick counts equal or higher.
7. AA exposure: orbiting the carrier group at 1.6 km keeps the Vals inside the escorts' AA
   envelope instead of flying away from it. **Val deaths and entity_impacts on the Vals equal or
   higher than the control**; nothing outside the dive-bomb squadrons moves except through them.

### 5.1 Revised after reading the control, before the treatment run finished

The control on `main` `dd4f591b3` is not the predecessor's world. Only four Vals enter goaway:
`D3A Val #3.1|.-2` (1 tick) and the three `#7.1` (4 ticks each, `transitions` 12-13). They enter at
946-1047 m, above the 900 m altitude gate, and the goaway completes on the next tick because the
host's `travel_20` is 0 and `009C7F00`'s range term `BCh > 0.9 * 0` is always true. Also,
`approach+B8h = 2080` is the `009C8A5E` floor, not `1.6 r`, so the 1560 m standoff above is not
established; the treatment census prints the real one. Revised:

1. The same four aircraft enter goaway, each with `enters` equal to its control goaway entries.
2. Their pitch is near level, so **no nose-down ticks**: the flag-0 side from the first tick.
3. **The goaway lasts much longer**: completion now also needs `BCh > 0.9 * standoff`, so each of
   the four stays in goaway, flying the heading arm away from the target, until the range opens.
   Goaway ticks rise from 1-4 to tens or hundreds.
4. Heading arm from the first tick (first-entry clock `dt` > window 0), `err_max <= 0.5236`,
   `side_writes = 0`. A re-roll only if 15 s pass inside `standoff - 100`; then exactly 90 bank
   ticks with min -1.2 and max +1.2.
5. The second attack run of these four is delayed: `#7.1|.-2` (0 releases in the control) and the
   others may release less or later inside 4800 frames. The other eleven dive-bomb aircraft never
   enter goaway and their task rows do not move, unless through a shared target's damage.
6. Deaths and impacts: any change is confined to the four goaway Vals and whatever their bombs hit;
   direction not predicted.

## 6. Measurement

Both runs from this worktree, same parameters as the predecessor's header:
`--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05`,
4800 `world frame` lines each.

| log | tree | what |
| --- | --- | --- |
| `local/ctl_usn04.log` | `main` `dd4f591b3`, unchanged | control |
| `local/turn_usn04.log` | this branch | the binding |

**The four goaway Vals, per aircraft** (task rows and the new census line):

| aircraft | control goaway ticks / releases | treatment goaway ticks / releases | treatment turn census |
| --- | --- | --- | --- |
| `D3A Val #3.1\|.-2` | 1 / 2 | 838 / **0** | enters 2, flag-0 809, rerolls 3, bank 122, heading 687, bank -1.200..+1.200, err_max 0.5236, side +1, side_writes 0 |
| `D3A Val #7.1` | 4 / 2 | 146 / **0** | enters 1, flag-0 133, rerolls 0, heading 133, err_max 0.5236, side +1 |
| `D3A Val #7.1\|.-2` | 4 / 0 | 58 / 0 | enters 1, flag-0 46, rerolls 0, heading 46, err_max 0.5236, side +1 |
| `D3A Val #7.1\|.-3` | 4 / 2 | 131 / **0** | enters 1, flag-0 118, rerolls 0, heading 118, err_max 0.5236, side -1 |

Standoff `+20h` = **1560.0** on all four, so `approach+B4h` = 780 m after all. The other eleven
dive-bomb aircraft's task rows are identical in both runs.

**What happened, from the hand-over census.** All four enter goaway from the fly-over **before any
dive**: `flyabove>goaway` at range 1214-1216 m and 1007-1047 m altitude (`#7.1` at arm tick 1185
control / 1193 treatment). In the control `009C7F00` completes on the next tick (`travel_20 = 0`),
the fly-over hands to turndown at range 1195 m and `#3.1|.-2` releases at 214 m. In the treatment
the goaway holds until `BCh > 0.9 * 1560 = 1404`: `#3.1|.-2` leaves it at arm tick 1297, range
1412 m, 1022 m altitude; **the fly-over then hands to turndown on the very next tick at 1420 m**,
the turndown hands to aimdive at **1882 m** range, and the dive ends in `aimdive>goaway` at 167 m
altitude and 1187 m range with no release; the second goaway (climb from 167 m) reaches 757.7 m by
the end. The three `#7.1` aircraft are still in their first goaway at the end of the run (58-146
ticks), still turning away from the target they were facing.

| mission | control | treatment |
| --- | --- | --- |
| `summary mission dive-bomb task` releases | 23 | 17 (the six lost are `#3.1\|.-2`, `#7.1`, `#7.1\|.-3`, two each) |
| `bomb_impacts` | 20 | 16 |
| `entity_impacts` | 104 | 81 |
| `deaths` | 10 | 8 |
| `total_damage` | 16473.3 | 15882.8 |

The two fewer deaths are `B5N Kate #6.1` and `#6.1|.-2`, killed by `Fletcher-class02` at 221.6 and
228.5 s in the control and alive in the treatment; `Fletcher-class02` fires 185 rounds instead of
247 and its nearest-target distance goes from 96 to 35 m. That is consistent with a goaway Val
drawing its AA, but the target-assignment trace was not read, so the mechanism is **not proven**.
The torpedo Kates' state rows move by 1-5 ticks. No Val dies in either run and every Val's health
row is unchanged. Nothing else moved in the per-unit table.

**Predictions against the result.** Right: the same four aircraft; a much longer goaway; the heading
arm from the first flag-0 tick; `err_max` exactly the 0.5236 clamp; `side_writes = 0`; bank exactly
-1.2 to +1.2 on the one aircraft that re-rolled; lost or delayed second runs. Wrong: "no nose-down
ticks" (12-29 per aircraft, the fly-over leaves them pitched down); "exactly 90 bank ticks per
re-roll" (122 over 3 re-rolls, one cut short by the state exit at arm tick 1297). The arm appears to
run about every other frame (2138-2370 arm ticks against 4800 frames), so its `dt` is about 0.1 s
and a weave is about 45 ticks, not 90. That last point is inferred from the counts, not read.

## 7. Decision

**Keep the binding.** Every rule is read from the listing, and the control's 1-tick goaway was an
artefact of the host leaving `+20h` at zero. The release loss does not come from the turn. It comes
from what follows it: the fly-over (`009C62B0`) hands to turndown within one tick while the
aircraft sits 1.4 km out facing away, and the dive starts from 1882 m. The image may do the same.
If it does not, the fault is in the fly-over's turndown edge, which packet `cc9_aimdive_response`
owns, not in this state. The predecessor's section (a2) already recorded a second pass that spends
only one or two ticks in `flyabove`.

## 8. Open

* The fly-over's turndown edge after a goaway: which of its conditions passes at 1420 m range facing
  away (`f19=1` in the hand-over census), and whether the image's fly-over turns the aircraft back
  in first.
* `Fletcher-class02`'s changed AA, and with it the two Kates: read the gunnery assignment trace.
* The first-entry `+28h`/`+2Ch` from the task allocation; `007B5BE0`'s target extent; the
  `009FD570` obstacle list (the aircraft starts next to its target ship); the `007F0280` probe.
* `009C4E27`-`009C4E58`: the approach's `+1Ch` object (auto-strafe angle and direction).

## Correction from docs/DIVE_BOMB_REATTACK.md (2026-09-22, packet `cc9_goaway_reattack`)

* Section 6's arm period, "inferred from the counts, not read", is now read. `BSP_PilotBot_Tick`
  gate 6 (`0099AD21`-`0099AD29`, `[00D1F39C]` = 0.09f) thinks only once the accumulated interval
  reaches 0.09 s, and passes that interval on as `dt`. At 0.05 s frames the goaway tick runs every
  second frame with `dt = 0.10`, and the host feeds the same accumulator. Section 5's `dt = 0.05`
  assumption was wrong: a re-roll's bank weave is 45 arm ticks, not 90, and a 15 s countdown is 150
  arm ticks, not 300.
* Section 6 calls the three `#7.1` Vals "still turning away from the target they were facing" at
  the end of the run. Only the first half is established: their goaway counts (146, 58, 131) are
  exactly the arm ticks from their goaway entry to the end of the run. The heading at the end was not
  traced.
