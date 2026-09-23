# The dive-bomb re-attack: what follows a completed goaway

Addresses: 009C83E0 (009C86D3-009C870B), 009C7F00, 009C82D0, 009C8A90, 009C6270, 009C6210, 009C3DA0, 007B3DC0, 0099ACD0 (0099AD16-0099AD29), 009C4A40

Packet `cc9_goaway_reattack`, branch `agent/cc9-goaway-turn`, base `main` `74891ae53`. It follows
`docs/DIVE_BOMB_GOAWAY_TURN.md`. The transition was read from the Ghidra listing (`ghidra disasm
009C83E0`), and the vtable-slot bodies with no Ghidra function were decoded with `disasm-raw`.

## 1. The edge, read

`009C83E0` (`BSP_BotTaskDiveBomb_UpdateStateTransition`, `__fastcall(task)`, `RET 4`) is the only
caller of `009C7F00`. The goaway arm:

```
009C86D3  LEA ECX,[ESI+704h]; CMP EAX,ECX; JNZ 009C84E7   ; current state is goaway (+704h)
009C86E1  CALL 009C7F00; TEST AL,AL; JZ 009C84E7         ; completion rule, else stay
009C86EE  CMP byte ptr [ESI+4C9h],0                       ; approach+D1h, the bomb-ordnance byte
009C86F7  JZ 009C8705
009C86F9  PUSH EDI; CALL 009C82D0                         ; EDI = ESI+778h (009C84ED) = flyabove
009C8705  PUSH EBX; CALL 009C82D0                         ; EBX = ESI+664h (009C8484) = done
```

EDI and EBX were resolved by filtering the whole listing for the two registers. The only
`LEA EDI` on a path that reaches `009C86D3` is `009C84ED` (`ESI+778h`). The three other `EDI`
loads, `LEA EDI,[ESI+79Ch]` at `009C853F`, `009C856B` and `009C85B8`, are inside the flyabove arm, which returns
before `009C8609`. `EBX` is loaded once, at `009C8484`, after `PUSH EBX` at `009C8483`.

**That is the whole re-attack edge.** It has no round counter, no re-attack limit, no target test
and no reset of its own. The 0.9 scale with `ctl+369h` and `[00E17BF2]` lives inside `009C7F00`
(`009C7F09`, `009C7F7A`), which the host already reconstructs. The only other way out of goaway on
this path is earlier in the same routine: `009C8495 CALL [[ESI]+1Ch]` = `009C8A90`
(`BSP_BotTaskDiveBomb_ShouldBreakOff`) sends any attacking state to done (`009C849E`), and the host
reconstructs that as `dive_bomb_should_break_off_009c8a90`. Rounds are spent only through the
ordnance byte: after the last bomb, `approach+D1h` clears and the edge takes done.

**What changes on the edge is in the two vtable slots `009C82D0` (`BSP_BotTaskDiveBomb_SetState`)
calls**: the old state's exit (slot `+8h`), then the new state's enter (slot `+4h`).

* The goaway exit, `00D20CA0` slot `+8h` = `007B3DC0`, is a bare `RET`. **Nothing is reset on the
  way out of goaway**, so `+18h`..`+2Ch` persist into the next goaway, as the host has it.
* The fly-over enter `009C6270` (`00D20D04` slot `+4h`; the vtable reads `009C7380, 009C6270,
  009C6210, 009C62B0`):
  * `009C6276 CALL 009C3DA0(approach)`, the aim-error re-draw below;
  * `unit+844h = 0` (`009C6283`);
  * `state+20h = 0` (the roll-in side, `009C628C`);
  * `+1Bh`, `+19h`, `+18h` and `+1Ch` all set to 0 (`009C628F`-`009C629E`);
  * `+1Ah = (approach+D1h == 0)` (`009C62A1`).
* `009C3DA0` (body `009C3DA0`-`009C3E9C`, `__thiscall(approach)`, named here
  `BSP_BotTaskDiveBombApproach_RedrawAimError`). Two `00BD2F10` draws symmetric about zero, from
  `[approach+14h]+30h` and `+34h` (`009C3DCB`, `009C3DF5`, the lower bound built by
  `SUBSS` from the `-0.0` at `00D7A208`), go through `009FA380` into the aim-reference
  sub-object at `approach+30h`. Then `+48h`..`+50h` = `row+58h`, `+41h = 1` (dirty),
  `+64h`..`+70h` = `row+70h`..`+7Ch`, and `approach+C8h = U(-row+2Ch, row+2Ch)` (`009C3E8C`).
  **Every re-attack re-draws the aim error.** This host draws none, which
  `docs/FOLLOWER_ATTACK_HANDOVER.md` section 6 already records as open.
* The fly-over exit `009C6210` (no Ghidra function, body `009C6210`-`009C621D`, named here
  `BSP_BotStateDiveBombFlyAbove_Exit`) sets `unit+844h = 1`.

## 2. Host against image, term by term

| image | host | status |
| --- | --- | --- |
| `009C86E1` `009C7F00` | `dive_bomb_goaway_complete_009c7f00`, fed the goaway's own `+20h` since `adb4d47cd` | same |
| `009C86EE` `approach+D1h` | `in.entry.has_bomb_ordnance_4c9 = slot.db_has_bomb_d1` | same |
| `009C86F9` -> flyabove, `009C8705` -> done | `src/dive_bomb_task.cpp` goaway case | same |
| `009C8495` `009C8A90` -> done | `dive_bomb_should_break_off_009c8a90` | same |
| goaway exit `007B3DC0` | nothing | same (`RET`) |
| goaway enter `009C4950` | `run_dive_bomb_goaway_enter_009c4950` | same (`adb4d47cd`) |
| fly-over enter `009C6270`: `009C3DA0` re-draw, `unit+844h`, `+20h`, `+1Ah` | only the `+1Ch` clear; the rest recomputed by the state feed or absent | **fly-over's hunk, not this packet's** |

**No edge or reset inside this packet's hunks is missing**, so `kDiveBombReattackBound` was not
added. A flag with nothing behind it would only make the measured pair look like a test. The
missing enter effects belong to the fly-over and are listed for its owner in section 6.

## 3. The arm's tick period, read

The dive-bomb arm is `task->vtable[64h]`, called from `BSP_PilotBot_Tick` `0099ACD0`. Gate 6
(`0099AD21`-`0099AD29`, `docs/PILOT_BOT_TICK_GATES.md`) thinks only when `bot+70h + dt >=
[00D1F39C] = 0.09f` (`FLD dword`, `FCOMIP`, `JBE`). `0099AD16` then replaces the argument with
the accumulated interval, which `0099AD75` resets. At `--mission-frame-seconds 0.05` the
accumulator reads 0.05 (no think) and then 0.10 (think), so **the arm, and the goaway tick inside
it, runs every second frame with `dt = 0.10 s`**. The host matches: `run_dive_bomb_task_arm_009c8790
(elapsed)` at `src/game_hosts_units.cpp` ~9400 is fed `unit_.pilot_think_accumulator_70`. That
corrects `docs/DIVE_BOMB_GOAWAY_TURN.md` section 6's inferred period, whose number was right but
unsupported, and its predictions' `dt = 0.05`. A bank weave is `1.5 * 3.0 / 0.1` = 45 arm ticks.

## 4. The last packet's 146 / 58 / 131 against 838, explained

These counts are not a different law. The three `#7.1` Vals were **still in goaway when the
4800-frame run ended**. Their `flyabove>goaway` hand-overs in `local/turn_usn04.log` are at arm ticks
1193, 1281 and 1208, and the arm ended at tick 1339. `1339 - 1193 = 146`, `1339 - 1281 = 58` and
`1339 - 1208 = 131`, exactly the three goaway tick counts. `#3.1|.-2`'s 838 is two goaways:
1130-1297 (167 ticks, complete at 1412 m) plus 1467 to the end of its arm at 2138 (671 ticks, a
climb from 167 m that had not finished). So **the only fully observed first goaway took 167 arm
ticks** to open from 1214 m to past 1404 m. At ~70 m/s and 0.1 s per tick, 190 m of straight
opening is about 27 ticks. The other ~140 ticks are, by inference and not by a per-tick trace, the turn: the aircraft entered goaway heading at
the target, and the heading arm allows at most 30 degrees of commanded error, so it turns at the
planner's rate through about 180 degrees before it opens range.

## 5. Predictions, written before the 9000-frame run

USN04, `--frames 9200 --press-start-frame 30 --menu-select USN04 --mission-frames 9000
--mission-frame-seconds 0.05`, on `main` `74891ae53` unchanged. It is the only run: there is no
binding, so there is no treatment. The four goaway Vals:

1. **`#7.1`, `#7.1|.-2`, `#7.1|.-3`**: first goaway from the fly-over at 1170-1220 m and ~1000 m,
   complete after about 150-200 arm ticks at `BCh > 1404` with altitude still above 900. The edge
   takes **flyabove** (both bombs still aboard), which hands to turndown within one or two ticks
   while the aircraft faces away, then the dive starts beyond 1.8 km and **aborts without a
   release** (aim error far above the 25 m gate). That leads to a second goaway at low altitude, a
   long climb back past 900 m (hundreds of ticks), and completion.
2. **`#3.1|.-2`**: the same cycle, already one step further along at arm tick 1467.
3. At 9000 frames each of the four gets **one or two more such cycles**, and **releases stay 0**
   for all four unless one dive happens to start close enough.
4. Handoff (f): each cycle is a climb and a dive inside the escorts' AA ring, so **at least one of
   the four is shot down** before the end, and more AA shots go to Vals than at 4800 frames.
5. The other eleven dive-bomb aircraft never enter goaway. They either finish in done after two
   releases or stay where the 4800-frame control left them; the extra frames move them only
   through `done`.

## 6. For `cc9_flyover_speed`: the fly-over-to-turndown hand-off after a goaway

Evidence from `local/turn_usn04.log` (`adb4d47cd`, USN04, 4800 frames), `D3A Val #3.1|.-2`:

| event | arm tick | range | altitude | bearing error |
| --- | --- | --- | --- | --- |
| `goaway>flyabove` | 1297 | 1412 m | 1022 m | - |
| `flyabove>turndown` (roll-in latch set at 1298) | 1298 | 1420 m | 1023 m | 2.9358 rad (168 deg, target behind) |
| `turndown>aimdive` | 1360 | 1882 m (aim trace 1888.3 m) | 938 m | 2.6424 rad |
| `aimdive>goaway`, no release | 1467 | 1187 m | 167 m | - |

The fly-over bank census at the latch reads `along=-1636.7 m cross=430.1 m turn_circle=1300.0 m`.
The aimdive entry reads `aim error 009C5C9B=953.36 m (gate 25.0 m)`.

What is **consistent with the image as read**: `009C67B0` sets `+19h` when the folded bearing error
exceeds 1.6 rad, so a target behind is "ready to roll in". The tick's two clears (`009C688F`:
`1.5 * sin|E| * R > 1.4 * TurnCircleRadius`, and `009C68E1`: `cos|E| * R - 120 > 0`) would not
clear it here either, because `sin|E|` is ~0.2 and `cos|E| * R` is negative. So a turndown from
behind at 1.4 km is plausibly the image's own split-S. Those clears are still unapplied in the host,
which is a gap in general, but not the fault here.

What **looks wrong**: the turndown ends (`009C7EA0`, pitch and bank test) with the aircraft **still
facing away**, 2.64 rad at aimdive entry against 2.94 at turndown entry. The bearing error fell only
0.29 rad in 62 ticks while the range opened another 462 m. A split-S that completes should leave the aircraft heading
back at the target, so the question for the turndown owner is whether the host's turndown flies the
half-loop the image commands (`009C44F0` writes bank pi through the mode-1 servo) and whether
`009C7EA0` can complete before the heading has reversed. The dive then starts 1.9 km out with a 953
m aim error and can never release.

## 7. Measurement

`local/ctl_e9000.log`: `main` `74891ae53` unchanged, built and copied to `local/binCtl`, then run
with `--frames 9200 --press-start-frame 30 --menu-select USN04 --mission-frames 9000
--mission-frame-seconds 0.05`. It has 9000 `world frame` lines. **It is the only run.** With no
binding there is no treatment, so no pair is claimed and nothing below is a before/after
difference.

**The four goaway Vals, hand-over by hand-over** (arm tick, altitude, range):

| aircraft | goaway 1 (from fly-over) | complete -> flyabove -> turndown | dive 1 | goaway 2 (climb) | dive 2 | releases |
| --- | --- | --- | --- | --- | --- | --- |
| `#3.1\|.-2` | @1130, 1047 m, 1214 m | @1297/1298, 1412/1420 m | aimdive @1360 at 1882 m, 938 m; abort @1467 at 167 m | complete @2355, 900 m, 1564 m | aimdive @2410 at 1551 m, 805 m; abort @2499 at 163 m, 680 m | 0 |
| `#7.1` | @1193, 1007 m, 1216 m | @1361/1362, 1408/1415 m | @1429 at 1831 m; abort @1533 at 166 m | complete @2441, 900 m, 1565 m | @2496 at 1551 m; abort @2585 at 164 m, 681 m | 0 |
| `#7.1\|.-2` | @1281, 963 m, 1173 m | @1451/1452, 1405/1412 m | @1517 at 1793 m; abort @1620 at 167 m | complete @2538, 900 m, 1575 m | @2593 at 1553 m; abort @2683 at 163 m, 699 m | 0 |
| `#7.1\|.-3` | @1208, 1002 m, 1208 m | @1376/1377, 1408/1416 m | @1438 at 1870 m; abort @1540 at 168 m | complete @2440, 900 m, 1573 m | @2495 at 1553 m; abort @2586 at 163 m, 681 m | 0 |

The goaway-turn census at the end: `enters` 3-4, `rerolls` 4-6, `bank` 165-260 ticks, bank
-1.200 to +1.200 rad, heading `err_max` 0.5236, `side_writes` 0, standoff 1560.0 on all four. The
bank counts are consistent with 45-tick weaves, some cut short by a state exit (4 re-rolls give
165-167 ticks, 6 give 260, against 180 and 270 for full weaves). The per-weave split was not traced.

**The cycle is exactly periodic.** The first goaway lasts 167-170 arm ticks, opening from about
1210 m to 1405-1412 m. The climb goaway lasts 888-918 ticks, from about 165 m to 900 m, and ends at
1564-1575 m, on the standoff ring. Every re-attack then spends **one** fly-over tick. Every dive is
aborted by `aimdive>goaway` at 163-168 m. `#3.1|.-2` completes a third goaway at @3390 and the three
`#7.1` are in theirs when the run ends.

| mission | this run |
| --- | --- |
| `summary mission dive-bomb task` | aircraft=24 (a second batch spawns after frame 4800), releases=22 |
| `bomb_impacts` | 22 |
| `entity_impacts` | 83 |
| `deaths` | 9: Lexington-class01, movieval\|.-3, D3A Val #1.1 and #1.1\|.-3, B5N Kate #2.1, #2.1\|.-3, #4.1\|.-2, #4.1\|.-3, #8.1\|.-2 |

**None of the four goaway Vals is hit**: taken 0 and health 220 on each, through two dives and three
or four climbs each.

**Predictions against the result.** Right: prediction 1 (the first goaway ends at `BCh > 1404` after
about 170 ticks with altitude above 900, takes flyabove, turndown within one tick, a dive from beyond
1.8 km, an abort, then a long climb); prediction 2; prediction 3 (one more full cycle each, releases 0
for all four). Wrong: prediction 4, "at least one of the four is shot down". None is hit at all. The
handoff (f) cost of the climb (`#3.1` shot down by `Northampton-class04`) does not recur on `main`
at this base. Prediction 5 holds for the original fifteen aircraft; the second batch was not
predicted.

**The second dive is the stronger evidence for section 6.** After a climb goaway the aircraft sits on
the 1560 m standoff ring. The turndown starts at 1564-1575 m, the aimdive at 1551-1553 m and 805 m,
and the dive still aborts at 163 m, 680-699 m short. So the second failure is not only "starts facing
away". A dive begun 1.55 km out at 805 m (a 27 degree line of sight) ends 680-699 m short at the
pull-out altitude. Why it is begun there is the question for the fly-over and turndown owners.

## 8. Decision

**Nothing to bind in this packet's hunks.** The re-attack edge is `approach+D1h ? flyabove : done`
on `009C7F00`, and the host has it term for term. The goaway enter and completion were bound by
`adb4d47cd`. `kDiveBombReattackBound` was not added. The re-attack runs, and it fails downstream in
the fly-over, turndown and aimdive. That is `cc9_flyover_speed`'s area, with the evidence in
section 6 and the table above. The image's missing fly-over enter effects (`009C3DA0` aim-error
re-draw, `unit+844h`, `state+20h`) belong there too.

## 9. Open

* Whether the image's turndown reverses the heading before `009C7EA0` completes (section 6), and why
  a dive begun on the 1560 m ring at ~800 m aborts. The aimdive abort flag `aimdive+18h` producer is
  the next read.
* The base of `[approach+14h]` for `009C3DA0`'s three draws (`docs/FOLLOWER_ATTACK_HANDOVER.md` 6).
* Why no AA reaches the four goaway Vals across four dives. Not read.
