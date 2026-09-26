# The pilot moveto task, kind 7 (packet cc9_pilot_moveto_task)

2026-09-25. Ghidra read-only. Names are hypotheses. The escort Zeros in USN04 are ordered by the
mission script (`usn_19_coralus.lua`, `luaBombersSpawnedLex`, difficulty 1-2):
`EntityTurnToEntity(unit2, Lex)`, `UnitSetFireStance(unit2, 2)` and `PilotMoveToRange(unit2, Lex)`.
The host left all three natives unimplemented, so the Zeros held no task
(docs/STACKED_SPAWN_SEPARATION.md). The packet is split into landable parts.

## Part 1a: the order, the route and the task record

**The native.** `008A4590` `PilotMoveToRange(unit, target [, range])`:
- it reads the unit and the target (`008A466D`-`008A46C7`);
- the range goes to the descriptor's `+14h`, only when exactly three arguments are passed
  (`008A46DC`-`008A4708`; the rule is `pilot_move_to_range_008a46dc`);
- it issues command class `00E08F68` (`moveto`) through `0077D600` `BSP_Entity_IssueCommand`
  (`008A471C`-`008A472A`);
- it refreshes the world pose afterwards.

**The route.** `0099A170` `BSP_Bot_InstallCommandTask` tests `00E08F68` at `0099A223` with no target
precondition and calls the factory `009C3BE0` `BSP_BotTask_MakeMoveTo` (`0099A236`).

**The task.** `009C3BE0` allocates 0x550 bytes and calls `009C3000` `BSP_BotTaskMoveTo_Construct`:
- `BSP_BotTask_ConstructBase` (`0099C6F0`) with kind **7**;
- a sub-object at `+3F8h` (`009C2DF0`) built on `009C1C30` and holding the three states:

| offset | name (`00411E70`) | constructor |
| --- | --- | --- |
| `+47Ch` | `moveto (moveto)` (`00D20998`) | vtable `00D20A80` set inline |
| `+494h` | `follow (moveto)` (`00D20988`) | `009C2980` `BSP_BotStateFollow_Construct` |
| `+52Ch` | `circle (moveto)` (`00D20978`) | `009C25D0` |

- vtables: task `00D20B68`, sub-object `00D20B58`, `+468h` = `00D20B54`;
- `+548h` = 1.0 (`009C307C`), and `+54Ch` = -U(0, 1) (`009C3084`/`009C3089`).

**The start state** (`009C3097`-`009C30BF`):
- a flight leader (`007B8AD0`, the unit's `+9D8h`) enters `+52Ch` circle when `task+454h` is set,
  otherwise `+47Ch` moveto;
- a wingman enters `+494h` follow;
- the state is stored at `+310h` and entered through `vtable[4]`.
`task+454h` is the sub-object's `+5Ch`, which `009C1D0A` clears at construction. The only other store
is `009C3636`, in `009C3570`, which is part 2's read. So a leader starts in moveto.

**The binding.** `kPilotMoveToTaskBound` (`include/bsp/pilot_order_bindings.hpp`) gates two pieces:
- **`GameScriptOrdersHost::run_pilot_move_to_range`.**
  - It issues `00E08F68` with the range in the descriptor.
  - It installs the task through `bot_install_command_task_0099a170`.
  - It stores the class and the range on the unit.
  - It fans the order out to every squadron member, as PilotSetTarget's `007ECF80` route does.
- **The units host's `run_moveto_task_install_009c3000`.** It installs the kind-7 record and picks
  the start state. Part 1 has no tick: the record changes no flight.

**Not in part 1a (part 1b).**
- **`EntityTurnToEntity` (`008A0A10`).** It builds an orthonormal basis toward the target
  (`0085DC80`, `008A0CE3`). For a squadron (kind 18h) it re-poses each `+3D0h` member at its own
  position through `vtable[88h]` (`008A0D6D`-`008A0DE2`). It sets the spawn heading and separates
  nobody.
- **`UnitSetFireStance` (`008A6490`),** a gunner setting.
Both are read in part 1b.

### Predictions for part 1a's pair (written before the runs)

The pair is `kPilotMoveToTaskBound` OFF against ON, on current main (`9f5c9e373`), E2 9000.

| row | OFF | ON prediction |
| --- | --- | --- |
| `PilotMoveToRange` native status | UNIMPLEMENTED, 9 calls | routed; 9 calls logged |
| moveto task records installed | 0 | 2 per escort pair ordered; the leader in `moveto`, the wingman in `follow` |
| every gameplay row (deaths, hit records, releases, the Lexington, distance) | as main | identical, or within band if the issued command changes the director's current command |

**The risk.** The issued command replaces the Zero's current director command. If anything in the host
reads that command for a Zero (the command-target lookup), the Zero may begin steering. Any moved row
is examined before the verdict.

### Part 1a's pair, measured

`local\PM_OFF_9000.log` (binary `local\pm_off`) and `local\PM_ON_9000.log` (`local\pm_on`), main
`9f5c9e373` plus this commit, window 1600x900 in both.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| `PilotMoveToRange` native | concrete, 9 calls, doing nothing | concrete, 9 calls | routed | the OFF census row was wrong: `handles()` ignored the switch. Part 1b makes an OFF switch leave the native an unimplemented record |
| moveto task records | 0 | 17: each ordered leader in `moveto (moveto)`, each wingman in `follow (moveto)` | 2 per ordered pair | held: 8 escort pairs give 16, and the ninth call orders `moviefisher`, a single plane, for 1 |
| every gameplay row | - | identical, distance moved included | identical | held |

**Part 1a is committed OFF,** as the lead briefed. It changes no flight until the tick (part 2) and
the wingman state (part 3) land.

## Part 1b: EntityTurnToEntity and UnitSetFireStance

The script calls both from `luaBombersSpawnedLex` (`usn_19_coralus.lua:3086-3098` and the five
repeats): `EntityTurnToEntity` on the bombers (`unit1`) and the escorts (`unit2`) toward
`bombertrg`, then `UnitSetFireStance(unit2, 2)` on the escorts. `luaLexKillersSpawned` (1173-1176)
and the cutscene plane `moviefisher` (2184) also turn. `UnitSetFireStance(Mission.Lex,
STANCE_HOLD_FIRE)` (867) is at the mission's end.

**`008A0A10` EntityTurnToEntity**, single-player arm (`[00E188A8]+1FE4h` is 0, so the
session-message arm through `007EDEB0` is not taken):
- `d = target+FCh - entity+FCh`; `d.y` is zeroed unless a third argument is present and true
  (`008A0C0A`-`008A0C5F`);
- the matrix at `esp+34h` is rows (1,0,0), (0,1,0), `d`, position 0, `[+70h]` 1.0, then `0085DC80`
  orthonormalises it (`008A0C65`-`008A0CE3`, read from the listing);
- a squadron (`vtable[5Ch](18h)`, `007EFB00` answers 18h, 2, 1, 0) re-poses each of its `+3CCh`
  members, at most five, at the member's own `+FCh` through member `vtable[88h]`
  (`008A0D6D`-`008A0DE2`);
- a plane's `vtable[88h]` is `007C9540`: it copies the matrix to `+74h` and `+674h`, clears the two
  pose-valid bytes, and calls `(plane+310h)->vtable[0Ch]` = `007BEEE0`, which re-derives attached
  matrices. It writes no velocity.

**`008A6490` UnitSetFireStance** on a squadron: `vtable[114h]` is `007ECFD0`, the `+348h`
command block built at `007F4FE3`-`007F5009` by `0084D810` (vtable `00D0BD98`). `0071BE80` asks
that block's own predicates and stores through the shared senders:

| slot | body | rule |
| --- | --- | --- |
| `+24h` | `0084D910` | allowFire for stance 1 or 2 |
| `+28h` | `0084D930` | allowMove for stance 0, 2 or 3 |
| `+40h`/`+44h` | `0071DA50`/`0071DAD0` | send; `+64h` `0071D5D0` stores `+3Ch`, `+68h` `0071D5E0` stores `+3Dh` |

The block's defaults (`0084D862`-`0084D8A2`): Behaviour (`+364h`) 0 frees both; any other value
frees fire only for a fighter class (`[+35Ch]->vtable[18h](13h)`), move only otherwise. An
escort Zero squadron has Behaviour -1, so it starts with **allowFire 1, allowMove 0**, and stance
2 sets allowMove 1. A plane's `vtable[114h]` is `0047F180` (`XOR EAX,EAX; RET`), so on a single
plane the native does nothing.

**Readers of `+3Dh`.** `0080DC70` tests `+3Ch` and `+3Dh` together. `009F83CB` in
`BSP_Bot_RevalidateCurrentCommand` calls it on `bot+4` for a category 1 or 2 command. The block's
own `+50h` slot `0084D960` is the same test. Which object a plane bot holds at `+4` is not read
here, so whether allowMove 0 holds a Zero back in the image is open.

**The binding.** `kMissionTurnAndStanceBound`, squadron arms only:
- **`run_entity_turn_to_entity`** builds the basis from the leader's position, the stand-in for
  the squadron's `+FCh` because `007F4580` spawns every member at one matrix. It re-poses each
  member through `GameUnitsHost::set_unit_world_basis_007c9540`. A plane or ship argument is left a
  record, because those arms are unread.
- **`run_unit_set_fire_stance`** keeps the block's two bytes per squadron in the script host,
  because this host builds no `+348h` block. Nothing in the host reads them yet.
- **`handles()`** now returns false for a switch that is off, for `PilotMoveToRange` too.

### Predictions for part 1b's pair (written before the runs)

The pair is `kMissionTurnAndStanceBound` OFF against ON, with `kPilotMoveToTaskBound` OFF on both,
on main `25b5f4fb8`, E2 9000.

| row | OFF | ON prediction |
| --- | --- | --- |
| `EntityTurnToEntity` | UNIMPLEMENTED, 18 calls | concrete, 18 calls; 16 squadron turns (8 bomber, 8 escort) plus the `moviefisher` and `LexKillers` calls if they are reached, which log a record |
| `UnitSetFireStance` | UNIMPLEMENTED, 8 calls | concrete; 8 escort squadrons, allowFire 1->1, allowMove 0->1 |
| `PilotMoveToRange` | now UNIMPLEMENTED, 9 calls | the same |
| each turned member's heading at the callback | its spawn heading | level and toward the Lexington (the Town for waves 4-6) |
| bomber and escort gameplay rows (releases, hits, deaths, distance) | as main | may move within the RNG bands: a bomber's approach starts from a new heading |
| ship rows and everything not in a turned squadron | as main | within band only through RNG coupling |

### Part 1b's pair, measured

`local\TT_OFF_9000.log` (binary `local\tt_off`) and `local\TT_ON_9000.log` (`local\tt_on`), main
`25b5f4fb8` plus this change, window 1600x900 in both, the spawn callback at mission frame 500 in
both, so there is no clock offset.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| `EntityTurnToEntity` | UNIMPLEMENTED, 18 | concrete, 18; all 18 took the squadron arm; 52 member re-poses | 16 squadron turns plus records | held, and more: `movieval` and the `LexKillers` are squadrons too, so no call fell to the record arm |
| `UnitSetFireStance` | UNIMPLEMENTED, 8 | concrete, 8; every escort squadron Behaviour -1, fighter, allowFire 1->1, allowMove 0->1 | the same | held |
| `PilotMoveToRange` | UNIMPLEMENTED, 9 | UNIMPLEMENTED, 9 | the same | held; the `handles()` correction works |
| turned heading | spawn forward (±0.17, 0, 0.98), heading ±10 deg | forward about (0, 0, -1) | level, toward the Lexington | held. **The turn is about 170 degrees**: every wave spawns flying away from the fleet |
| plane deaths | 35 | 38: A6M Zero #1.2 wingman, #6.2 and its wingman added; none removed | within band | **moved beyond the band** |
| torpedo releases | 6 | 3 | within band | moved |
| dive-bomb releases | 2 | 4 | within band | moved |
| gunnery shots / hit records | 4597 / 586 | 6316 / 586 | within band | shots moved |
| plane distance moved | 1168477 | 1089997 | within band | moved |
| the Lexington's distance moved | 6616 | 3086 | within band | moved |
| ship-AI plan seeds, approach frames | 1674, 1504 | 3740, 4872 | as main | moved; the ships react to planes that now close on them |

**Verdict: kept OFF.** The pair moves gameplay well beyond the heading and the stance. That was
expected: a bomber that spawns facing the fleet reaches it about a turn earlier, and the escorts,
which hold no moveto task while part 1a is off, fly into the fleet's anti-aircraft fire.

**What the pair cannot settle.** `007C9540` writes no velocity, so in both the image and this host
a turned plane keeps its spawn velocity of about (±12, 0, 66) for its first steps. Whether the
image's flight model re-derives velocity from the body axes on the next step, and so turns the
plane at once, is not read. The switch should be judged together with parts 1a, 2 and 3 in the
final pair, not alone.

## Part 2: the tick, the approach, the state rule and the moveto state

### The read

**The tick** `009C3950` (task vtable `00D20B68` `+64h`, `009C3950`-`009C398D`, now a Ghidra
function) runs `009C3570(task+3F8h, dt)`, then `009C3310(dt)`, then the current state's
`vtable[0Ch]` (`009C3988`).

**The approach object** at `task+3F8h`. `009F9CE0` fills its head: `+4` the unit, `+8` the class
block (`unit+538h`), `+0Ch` the pilot control block (`unit+9D4h`), and `+24h` =
max(1, MaxSpd `class+188h` / ReferenceSpeed `tuning+37Ch`). `009C1C30` then sets:
- `+2Ch` = U(0, `tuning+540h` MaxAltOffset), the first of the five construction draws;
- `+44h` the target and `+48h`..`+50h` its point;
- `+54h` = 0.5 and `+58h` = -U(0, 0.5), the refresh period and its stagger;
- `+5Ch` = 0 (arrived), `+60h` = 0 (dwell), `+68h` = 50.0 (`00CEB4D4`);
- for a flight leader, `+60h` = min(planar distance to the target, TurnCircleRadius `class+268h`)
  / TravelSpeed `class+18Ch` (`009C1D32`-`009C1D9A`);
- then one refresh.

**The refresh** `009BEBA0`:
- `+2Ch` is cleared unless the control block's dirty byte `ctl+3ADh` is set;
- the point is the target's x/z, at altitude `ctl+394h` - `+2Ch`;
- `+64h` is the planar distance;
- then `009FD050(dist, point)`, and the `+28h` path leg (`009BD400`, `009FC260`).

**The approach update** `009C3570(dt)`:
1. `+58h` counts down; at zero it reloads with the 0.5 s period and refreshes.
2. When the squadron's `+348h` block holds a moveto command (`+54h` == `00E08F68`), `+6Ch` is copied
   from the block's `+6Ch`.
3. While not arrived, with `inner` = ClosingDist `tuning+370h` × `+24h`:
   - `dist < inner` sets `+60h` = -1.0 (`00D7A260`);
   - `inner <= dist < inner + TurnCircleRadius` counts `+60h` down;
   - a negative `+60h` sets `+5Ch` = 1 at `009C3636`.
4. Once arrived, approach `vtable[8]` runs every tick.

**The state rule** `009C3310(dt)`: `+54Ch` counts down and reloads from `+548h` = 1.0. At each
reload it picks follow `+494h` for a wingman, circle `+52Ch` for an arrived leader, and moveto
`+47Ch` otherwise. When the pick changes, it calls `vtable[8]` on the old state and `vtable[4]` on
the new one.

**The moveto state** `009C2430` (vtable `00D20A80`: `+4` `007B3DB0`, `+8` `007B3DC0`, `+0Ch` the
tick):
1. It raises `cmd+26Ch`, reads the point through approach `vtable[0]` = `009BE2C0`
   (`+48h`..`+50h`), and takes the planar distance.
2. The speed is `009BECD0(009C23B0(), 007C47F0(), dist)` into `cmd+2B4h`, with `+2B0h` = 0 and
   `+2D8h` = 1. `009C23B0` is `[approach+0Ch]+3A0h`, unless the target is within
   TurnCircleRadius + `+68h` and faster than `007C47F0`; then it is the target's speed.
3. When `unit+C25h` is set, it takes a four-store arm and stops.
4. Otherwise it commands the pitch with `009FB800(point.y, 1.0)` and the heading toward the point
   with `009F9E40`.
5. It sets `dir+40h` = `tuning+670h` Angle_MoveTo, then calls `009FABE0(009A1A20(p), p)` with
   p = `0099B630()`.

**The circle state** (vtable `00D20A9C`, tick `009C26D0`):
- Its constructor `009C25D0` draws U(0, 1) >= 0.5 into `+18h` (the side).
- The speed is min(`009BECD0(009C23B0(), 007C47F0(0), 0)`, TravelSpeed `class+18Ch`).
- It then calls `009FBB20(this=approach, &point, r, side, &out)` with
  r = max(`approach+6Ch`, TurnCircleRadius), followed by the same pitch and direction tail as the
  moveto state.

**`009FBB20`**, x87-heavy and not reconstructed:
- **Inputs:** ECX = the approach, the point, the radius r, the side byte and an out pointer.
- **Outputs:**
  - it writes `cmd+2C0h` (an `atan2` heading) and `cmd+2CCh` = 2;
  - through the out pointer, it writes a blended steer point on the circle of radius r;
  - it returns dist - r while dist > r, else `[00D7A208]` - (dist - r).
- **Internals:** a turn angle from `tuning+5DCh`..`+5E4h`, scaled by `unit+340h` and negated for
  the other side, `sin`/`cos`, and `004F4840`.

**The follow state** (vtable `00D20AB8`, tick `009C1FD0`, constructor `009C2980`) draws
-U(0, 0.6) into `+74h` (`00CE3D30`). It is part 3.

**The construction draws** from the shared stream, in order:

| order | where | draw |
| --- | --- | --- |
| 1 | `009C1C30` | approach `+2Ch` = U(0, MaxAltOffset) |
| 2 | `009C1C30` | approach `+58h` = -U(0, 0.5) |
| 3 | `009C2980` | follow `+74h` = -U(0, 0.6) |
| 4 | `009C25D0` | circle `+18h` = U(0, 1) >= 0.5 |
| 5 | `009C3084` | task `+54Ch` = -U(0, 1) |

**The task's `+54h` cruise profile** is `009C3650`-`009C394A` inclusive (`RET` at `009C394A`, INT3 after), with
no Ghidra function. It draws twice more: `+460h` = U(FollowDist/1,
FollowDist/2) and `+424h` (approach `+2Ch`) = U(0, TravelAltRandom). For a leader it writes
`ctl+394h` from the class tests `007B93F0` torpedo, `007B94F0` depth charge, `007B9320` bomb,
`0047B850`, and `0047B880`. A fighter takes SmallPlaneTravelAlt `tuning+35Ch` (800), or
LargePlaneTravelAlt `+360h` (1400) for a level bomber or large recon, plus 0.6 × TravelAltRandom
(`009C36DD`-`009C36F2`, `009C38FD`).

### The binding

`kMoveToTaskTickBound` depends on `kPilotMoveToTaskBound`.
- **At install:** the five draws in the image's order through the units host's shared stream, then
  the refresh and the leader's dwell.
- **Each think, after the install:** the approach update, the state rule, and the moveto state.
  - The speed reuses `moveto_speed_009c1850`, which is `009BECD0` of `[approach+0Ch]+3A0h`.
  - The pitch uses `pitch_command_009fb800` and the heading `heading_command_009f9e40`, as the
    torpedo and dogfight moveto bodies do.
- **Substitutions**, each named in the code:
  - `ctl+394h` is the fighter value from `009C3650`, computed at every refresh. That profile's
    cadence and its redraw of `+2Ch` are not bound.
  - `009C23B0`'s target-speed override is a record.
  - `009FD050`, the `+28h` path leg, the `+6Ch` block copy, approach `vtable[8]` after arrival
    and `009FABE0` are records.
  - The circle state's steer (`009FBB20`) and the follow state's tick (`009C1FD0`) are named
    records.

### Predictions for part 2's pair (written before the runs)

`kMoveToTaskTickBound` OFF against ON, both built with `kPilotMoveToTaskBound` ON and
`kMissionTurnAndStanceBound` OFF, on main `98ef34226`, E2 9000, streams on.

| row | OFF | ON prediction |
| --- | --- | --- |
| moveto task records | 17 | 17 |
| summary `moveto task` line | absent | 17 tasks; 9 leaders in moveto, 8 wingmen in follow (a record) |
| escort leader heading | as main (task-less) | toward the ordered ship at about 805 m |
| arrivals | none | early waves' leaders arrive; inner is about 150 × max(1, Zero MaxSpd / 83.3) |
| state changes | none | one moveto to circle per arrived leader |
| `BotStateMoveToCircle::steer` | absent | recorded after each arrival |
| wingmen | as OFF | as OFF except through coupling: the follow tick is a record |
| escort Zero deaths | as OFF | may rise: leaders now fly into the fleet's fire, as in part 1b |
| bomber rows and ship rows | as OFF | move within the RNG bands |

**Open, for the final all-on pair.** Does the flight model re-derive velocity from the body axes on
the step after a turn? Part 1b's `007C9540` writes no velocity.

### Part 2's pair, measured

`local\MT_OFF_9000.log` (binary `local\mt_off`) and `local\MT_ON_9000.log` (`local\mt_on`).
Both sides are built from `0e94d176b` with `kPilotMoveToTaskBound` ON and differ only in
`kMoveToTaskTickBound`. The window is 1600x900 in both, and the spawn callback runs at the same
mission frame, so there is no clock offset.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| moveto task records | 17 | 17 | 17 | held |
| `moveto task` summary | absent | 17 tasks, 49304 ticks, 14 state changes, 10 arrivals | 9 leaders in moveto, 8 wingmen in follow | held |
| arrival radius | - | inner = 150.0: the Zero's MaxSpd is 83.33 = ReferenceSpeed, so the ratio is 1 | about 150 × max(1, MaxSpd / 83.3) | held |
| arrivals | none | 8 escort leaders at 122-143 m, plus 2 wingmen promoted after their leaders died | the early waves' leaders | held, and wider: every wave arrived within 450 s |
| state changes | none | 8 moveto->circle; 4 follow->moveto from promotion (`007B8AD0` turns true) and 2 of those then arrive | one per arrived leader | held; promotion was not predicted |
| `BotStateMoveToCircle::steer` | absent | 13571 records | recorded after arrivals | held |
| plane deaths | 35 | 42: seven escort Zeros added (#2.2, #3.2 and #5.2 with their wingmen, and #7.2), none removed | may rise | held in direction; the size is well beyond the band |
| hit records | 595 | 705 | within band | moved, from the added Zero deaths |
| torpedo / dive releases | 5 / 2 | 6 / 1 | within band | held |
| the Lexington's distance moved | 6641 | 6145 | within band | held |

**What the pair shows.**
- **Leaders fly to the ordered ship.** Every escort leader, moving at about 805 m, closes to about
  130 m of it and arrives.
- **Then they leave the fight.** With the circle steer a record, nothing writes their heading after
  arrival, so they fly straight on; four end 20-31 km away.
- **Why the deaths rise.** The Zeros that died are the ones the tick took into the fleet's
  anti-aircraft fire.
- **Waiting on part 3.** The wingmen's follow tick is a record, so they still run the task-less arms
  and do not stay with their leaders; the wingmen of waves 1, 4, 6 and 8 end 31-40 km out. Part 3
  binds that state.

**Kept OFF**, as briefed. The circle steer (`009FBB20`) matters as soon as a leader arrives. The
final all-on pair should be read with that record in view.

## Part 3: the wingman's follow state, and what a squadron holds before its first order

### The read

**What the state rule hands a wingman.** `009C3310` selects `+494h`, the follow state built by
`009C2980`:
- its vtable `00D20AB8` has seven slots: `009C2A60`, `009BED80` enter, `009BDE40` exit,
  `009C1FD0` tick, `007B3DE0`, `009BE590`, `009A4860`;
- `+70h` = 0.6 (`00CE3D30`) is the search period, and `+74h` = -U(0, 0.6) its first countdown;
- `+6Ch` = `tuning+380h`.

**The enter** `009BED80` (body `009BED80`-`009BEE24`):
- `009BE150(tuning+380h)`, then it clears `+90h`, `+94h` and `+84h`;
- `+88h` = `[+6Ch]+8` and `+8Ch` = 1.0;
- on the squadron it sets `+3E4h` = 1 and calls `007ED260` `BSP_PlaneSquadron_AssignFormationIndices`;
- it takes the leader from `squadron+3D0h` into `+2Ch` and registers the observer pair;
- it clears `+85h`.

It runs when the state is first entered and at every change `009C3310` makes.

**The tick** `009C1FD0` (body `009C1FD0`-`009C2355`):
1. It sets `cmd+26Ch` = 2 (`009C1FE2`).
2. It calls `009BFD70`, the station (`009C1FEA`). If that returns false, the tick ends.
3. A release pending (`unit+C25h`) or queued (`+C20h`, `009C1FFD`-`009C2006`) takes a four-store
   arm: speed `class+190h` and pitch mode 2.
4. Otherwise it calls `009BFEE0`, the station keeping (`009C2068`), then `009BEE30(dt)`, the command
   step (`009C2077`).
5. The `+85h` trail arm (`009C207C`-) writes `unit+844h`/`+840h` from the leader's bank `+C68h`.
6. The `+74h` countdown reloads from `+70h` (`009C211D`-`009C213B`). On reload it runs a sight
   search (`009C214A`-) over the recon list (`008053C0`) for objects of kind 5 inside a forward box
   from `dir+34h` and `dir+38h`, setting `+84h`, `+80h` and `unit+914h`.
7. `+84h` selects `dir+4Ch` = `[00CE3958]` or 0.

**What a squadron holds before its first order.** `0099A170` builds a task only from the
director's current command. It returns without a task when the director or its class is null
(`0099A17F`, `0099A19E`). A freshly spawned escort or strike squadron therefore has an empty task
vector until something issues a command. `009998A0` then runs the reseed `0099B450`, `009A17D0` and
`0099D300` with no task tick. Those are the task-less planner arms `kPlannerTasklessArmsBound`
binds, and that is what they stand in for.

**Who gives the first order:**
- In USN04, the spawn callback (`PilotMoveToRange`, `PilotSetTarget`) for script-spawned waves.
- For air-ops launches, the party AI's `attackmove` (docs/AIROPS_LAUNCH_TICK.md).
- An `attackmove` that `007EEC50` cannot resolve to an attack class has `EDI` = 0 at `0099A20A`
  and takes `009C3C40` (`0099A219`). That is a kind-7 moveto variant (`009C3B10`, vtable
  `00D20BE0`) toward the command's target or point. `009BE310` falls back to the zero vector at
  `00F87574`.
  - Its class getter `+3Ch` `009C3B80` answers `00E08F78` `attackmove`, where the plain moveto's
    `009C3150` answers `00E08F68`.
  - Its tick `+64h` is the same `009C3950`.

ATTACK_COMMANDS.md's "none / null" row is this unresolved-attackmove arm.

### The binding

`kMoveToFollowBound` depends on `kMoveToTaskTickBound`. The follow state runs `009BFD70`,
`009BFEE0` and `009BEE30` through the host's existing bodies
(`place_wing_member_on_station_007f23a0`, `run_follow_law_009bfee0_009bee30`), the same ones the
torpedo prepare seam uses.
- The formation indices and the leader come from the squadron registry. That stands in for
  `007ED260` and `squadron+3D0h` at the enter.
- The release arm, the `+85h` trail arm and the sight search are named records. The `+74h`
  countdown runs.

### Predictions for part 3's pair (written before the runs)

The pair is `kMoveToFollowBound` OFF against ON. Both sides are built with
`kPilotMoveToTaskBound`, `kMissionTurnAndStanceBound` and `kMoveToTaskTickBound` ON, on main
`7d88bf1e2`, E2 9000, streams on.

| row | OFF | ON prediction |
| --- | --- | --- |
| `BotStateMoveToFollow::tick` | a record, about 20000 | concrete, the same order |
| wingman distance at the end | 30-40 km out for a surviving wing, as in part 2 | close to a live leader, following it past the ship after arrival |
| promotions (follow -> moveto) | about 4, each on a leader's death | about the same; each promoted wingman flies on to the ship and may arrive |
| escort Zero deaths | part 2's level with the turn | rise: the wingmen follow their leaders through the fleet's fire |
| bomber and ship rows | as OFF | within the RNG bands |

### Part 3's pair, measured

`local\MF_OFF_9000.log` (binary `local\mf_off`) and `local\MF_ON_9000.log` (`local\mf_on`).
Both sides are built from `162a2affe` with parts 1a, 1b and 2 ON, and differ only in
`kMoveToFollowBound`. The window is 1600x900 in both, and the spawn callback runs at the same
frame.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| `BotStateMoveToFollow::tick` | record, 16178 | concrete, 20259 | concrete, the same order | held |
| wingman at the end, beside a live leader (waves 1, 2, 4, 8) | 2.7-31.5 km, unrelated to the leader | 4-40 m from the leader (30918/30923, 26557/26547, 30415/30454, 20945/20915) | close to the leader | held |
| promotions (follow -> moveto) | 4 | 1 | about the same | **failed**: fewer leaders die while their wingman is still in follow |
| follow -> circle | 0 | 3 | not predicted | the wingman was promoted when its leader had already arrived |
| arrivals | 11 | 14 | - | wingmen now reach the ship with their leaders |
| plane deaths | 44 | 43: A6M Zero #1.2's wingman survives, and nothing is added | rise | **failed**: flat |
| hit records | 788 | 796 | within band | held |
| torpedo / dive releases | 5 / 4 | 5 / 6 | within band | held |
| the Lexington's distance moved | 5828 | 4285 | within band | held |

**What the pair shows.**
- **The follow state does its job.** Every surviving wingman holds its leader's station through the
  run, including past the ship after the leader arrives.
- **Deaths stay flat.** A wingman flying in formation is no more exposed than one flying the
  task-less arms. Part 2's added deaths came from the leaders' approach, not from the wingmen.

**Kept OFF**, pending the all-on pair.

## The all-on pair: every moveto switch ON against every one OFF

### The velocity question, answered from the image before the runs

Does the flight model rebuild velocity from the body axes on the step after `007C9540` turns a
plane? **No.**
- **Where the velocity lives.** The plane's velocity is the world vector `ctl+18h..20h`. The pose
  arm adds it straight to the position (`007D827B`, `007DA218`; docs/PLANE_ADVANCE_POSE.md).
- **What the turn writes.** `007C9540` writes only the pose (`+74h`, `+674h`), the two pose-valid
  bytes and the child chain. Its `(plane+310h)->vtable[0Ch]`, `007BEEE0`, writes only derived
  matrices and an attachment point.
- **How the next step reads it.** `007D8470` takes the unchanged world velocity through the new pose
  into body axes, integrates it (`007D8611`: `w = v + a*dt`, the resisting fold only against `w`),
  and rotates it back (docs/FLIGHT_INTEGRATOR.md).

So a plane turned by about 170 degrees slides backwards along its spawn velocity, and drag and
thrust bend it round over the following steps. The host carries `motion.linear_velocity` the same
way through `kFlightIntegratorBound`. Part 1b's behaviour is therefore the image's, and its pair
movement is not a host artefact.

**Known gap for every switch set.** The circle steer `009FBB20` is unbound (part 4). A leader that
arrives flies straight on, and its wing follows it.

### Predictions (written before the runs)

The ON side has `kPilotMoveToTaskBound`, `kMissionTurnAndStanceBound`, `kMoveToTaskTickBound` and
`kMoveToFollowBound` all ON. The OFF side has all four OFF, which is main. Both are built from
`f8b96cf44`, with streams on and one run at a time.

**E2 9000:**

| row | OFF | ON prediction |
| --- | --- | --- |
| natives | PilotMoveToRange, EntityTurnToEntity, UnitSetFireStance UNIMPLEMENTED | all concrete, 9 / 18 / 8 calls |
| moveto tasks | none | 17, about 14 arrivals, surviving wingmen within about 40 m of their leaders |
| plane deaths | about 35 | about 43, the added deaths all escort Zeros |
| torpedo / dive releases | about 5-6 / 2 | within the bands seen in the parts (3-6 / 1-6) |
| the Lexington's distance moved | about 6600 | lower, 4000-6200 |

**USN04 4700/4500 (E2 parameters, 225 s of mission):**

| row | OFF | ON prediction |
| --- | --- | --- |
| natives | as above, fewer calls | concrete where called; waves 1-4 at most |
| escort leaders | fly away from the fleet | turn toward it and close; arrivals only for the early waves |
| plane deaths | main's | up to +4, escort Zeros |
| releases | main's | within band |

### The all-on pair, measured

Binaries `local\ao_off` and `local\ao_on` are built from `6037768c6`. Streams were on, the runs
went one at a time, and the 1600x900 window line is in all four logs. The spawn callbacks run at
the same frames on both sides.

**E2 9000** (`local\AO_OFF_9000.log`, `local\AO_ON_9000.log`):

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| natives | UNIMPLEMENTED 9 / 18 / 8 | concrete 9 / 18 / 8 | concrete | held |
| moveto tasks | none | 17, 14 arrivals; surviving wingmen 4-40 m from their leaders | 17, about 14 | held |
| plane deaths | 35 | 43: eight escort Zeros added (#3.2, #5.2, #6.2, #7.2 and their wingmen), none removed | about 43, all escorts | held |
| torpedo / dive releases | 5 / 2 | 5 / 6 | within 3-6 / 1-6 | held, dive at the top of the band |
| hit records | 595 | 796 | - | the escort losses |
| the Lexington's distance moved | 6641 | 4285 | 4000-6200 | held |

The ON log's summary equals part 3's `MF_ON_9000.log`: it is the same switch set on a tree that
differs only in the doc.

**USN04 4700/4500** (`local\AO_OFF_4500.log`, `local\AO_ON_4500.log`):

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| natives | UNIMPLEMENTED 8 / 17 / 8 | concrete 8 / 17 / 8 | concrete where called | held |
| moveto tasks | none | 16 tasks, 14 arrivals: all eight waves arrive within 225 s | the early waves only | **failed**: every wave arrives |
| plane deaths | 30 | 36: six escort Zeros added (#3.2, #5.2, #6.2 and their wingmen) | up to +4 | **failed**: +6 |
| torpedo / dive releases | 5 / 2 | 5 / 5 | within band | held |
| the Lexington's distance moved | 3467 | 3481 | - | unchanged |

### Verdict: all four switches ON

`kPilotMoveToTaskBound`, `kMissionTurnAndStanceBound`, `kMoveToTaskTickBound` and
`kMoveToFollowBound` are set ON.
- **What moves, and why.** Every row that moves comes from the escorts now doing what the script
  orders: they turn toward the fleet, fly to their ship and keep formation. The extra deaths are
  escorts lost to anti-aircraft fire. The extra dive releases come from bombers that face the fleet
  from spawn.
- **Part 1b.** Its turn is the image's own, and the velocity read above shows the slide that
  follows is too.
- **What stays wrong.** After arriving, a leader and its wing fly straight on instead of circling.
  That is the circle steer `009FBB20`, part 4, and it is the next thing to bind.
