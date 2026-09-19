# The dive-bomb task (packet `cc8_dive_bomb_task`)

Addresses: `009C8790`, `009C83E0`, `009C8310`, `009C82D0`, `009C7910`, `009C7A80`, `009C8200`,
`009C7FF0`, `009C88E0`, `009C8920`, `009C8A90`, `009C8C70`, `009C7710`, `009C73A0`, `009C58D0`,
`009C5870`, `009C58A0`, `009C5180`, `009C4F00`, `009C62B0`, `009C44F0`, `009C4A40`, `009C4220`,
`009C7270`, `009C7240`, `009C7260`, `009C7800`, `009C7EA0`, `009C7850`, `009C7F00`, `007C1DB0`,
`007BBBA0`, vtables `00D20E18`, `00D20D28`, `00D20CA0`, `00D20CE8`, `00D20CC8`, `00D20D04`,
`00D20C84`, `00D20C68`.

The dive bomber has **ten** states, not the torpedo's seven, and its release is in the aim states,
not in `done`/`prepare`. The torpedo's exit-slot drop has no counterpart here.

## (1) `009C8790`, the per-tick arm

Task vtable `00D20E18`, slot `+64h` (the dword at `00D20E7C`). **No Ghidra function.**
Raw listing `009C8790`-`009C88D5` inclusive, `INT3` padding from `009C88D6`.

ABI: `void __thiscall(BotTaskDiveBomb* task, float dt)`, `RET 4` at `009C88D3`.

| step | rule | address |
| --- | --- | --- |
| 1 | `task->+4C4h = 0FFh`, the plan-step scratch (the torpedo uses `+49Ch`) | `009C87A3` |
| 2 | `diving` = the state is one of `aimdive +734h`, `aimglide +754h`, `flyabove +778h`, `turndown +79Ch` | `009C8794`-`009C87D7` |
| 3 | `009C7A80(task+3F8h, dt, diving)` - the approach update | `009C87EA` |
| 4 | `009BDE80(task+4F0h, task+4A4h - [00D7A220], task+4A4h, task+4ACh)` - refresh the `moveto` ranges. `[00D7A220]` is the **double** `100.0` | `009C87EF`-`009C8825` |
| 5 | `009C83E0(task, dt)` - the transition rule, before the state's own tick | `009C8834` |
| 6 | `task->+310h->vtable[+Ch](dt)` - the current state's tick | `009C883D`-`009C884C` |
| 7 | `task->+2E4h = task->+4C4h` | `009C8855`-`009C885B` |
| 8 | when `task->+424h > 0`, `task->+3FCh != 0`, the object **embedded at** `unit+72Ch` answers its `vtable[+38h]`, and the state is none of `aimdive`, `aimglide`, `attackrun +7BCh`, `flyabove`, `turndown`, `prepare +5C4h`: `007BBBA0(unit)` and `task->+424h -= 1` | `009C884E`-`009C88C9` |

Step 8 is the manual passthrough, the same shape as the torpedo's `009D4956`. The receiver is
embedded, not pointed to: `009C8873 LEA ECX,[EAX+72Ch]` while `009C886D` loads the vtable from
`[EAX+72Ch]`, exactly as `docs/TORPEDO_TASK_ARM.md` records for `009D491D`.

Step 3's callee is `RET 8`. That is forced by the frame: `009C87EF` stores `task+4A4h` into the
slot at `E0+8` and `009C880C` reads it back as `[ESP+14h]` after a `SUB ESP,0Ch`, which only lines
up if `009C7A80` cleaned its two pushed arguments. Step 4's callee is `RET 0Ch` by the same
argument: `009C882A FLD [ESP+10h]` is `dt` only with `ESP` restored.

## (2) The ten states

`009C73A0`, the approach constructor, registers all ten by name through
`BSP_BotStateRegistry_Add` at `009C7680`-`009C76C6`. The names are the image's own strings:

| state | task offset | approach offset | registry name | vtable | tick |
| --- | --- | --- | --- | --- | --- |
| `moveto` | `+4F0h` | `+F8h` | `moveto (DiveBomb)` | - | the shared `009C2AC0` family |
| `follow` | `+52Ch` | `+134h` | `follow (DiveBomb)` | - | the shared `009C2980` family |
| `prepare` | `+5C4h` | `+1CCh` | `DiveBomb/prepare` | `00D20D28` | `009C7270` |
| `done` | `+664h` | `+26Ch` | `DiveBomb/done` | `00D20D28` | `009C7270` |
| `goaway` | `+704h` | `+30Ch` | `DiveBomb/goaway` | `00D20CA0` | `009C4A40` |
| `aimdive` | `+734h` | `+33Ch` | `DiveBomb/aimdive` | `00D20CE8` | `009C58D0` |
| `aimglide` | `+754h` | `+35Ch` | `DiveBomb/aimglide` | `00D20CC8` | `009C5180` |
| `flyabove` | `+778h` | `+380h` | `DiveBomb/flyabove` | `00D20D04` | `009C62B0` |
| `turndown` | `+79Ch` | `+3A4h` | `DiveBomb/turndown` | `00D20C84` | `009C44F0` |
| `attackrun` | `+7BCh` | `+3C4h` | `DiveBomb/attackrun` | `00D20C68` | `009C4220` |

Every state holds its owner (the approach) at `+4h`: `009C73A0` writes `param_1[0xc4] = param_1`
at the goaway slot and the same pattern at every other state. That is why the transition rule can
read `approach+D1h` through `*(char*)(aimdive->+4h + 0D1h)` at `009C8664`.

`prepare` and `done` share **one** vtable `00D20D28`, exactly as the torpedo's do. `009C7240`, the
shared enter, writes `state+9Ch = 0` and `state+98h = [00D7A260] = -1.0f` before tail-jumping to
`009BED80`; the registrar seeds the same two fields at `009C7452` and `009C74B8`. The tick
`009C7270`-`009C727F` calls `009C1FD0` at `009C7278` and returns at `009C727D`; it is a call and
return, not a tail jump.

The initial state comes from the task constructor `009C7710`: `BSP_Unit_LacksFollowTarget(unit)`
picks `moveto +4F0h`, otherwise `follow +52Ch`.

## (3) `009C83E0`, the transition rule

Body `009C83E0`-`009C8785`, `__fastcall(task)`, `RET 4` (the `dt` the arm pushes at `009C8831` is
never read). It runs **before** the state tick, so every flag it reads was written by the previous
frame's tick or by `009C7A80` earlier in the same arm.

`attacking` is `009C7910(task, state)`, a function here rather than the torpedo's inlined compares:
`009C7910`-`009C796B`, eight `LEA`/`CMP` pairs over `aimdive`, `aimglide`, `done`, `flyabove`,
`goaway`, `prepare`, `attackrun`, `turndown`. Everything but `moveto` and `follow` is attacking.

`engaged` is inlined at `009C83F8`-`009C8417`:
`task->+4C8h != 0 || (ctl->+370h == 2 && task->+440h != 0)`. `ctl` is `task+404h` = `approach+0Ch`
= `unit+9D4h`; `task+440h` is the latched target `docs/BOT_TASKS.md` gives for this class.

| step | rule | address |
| --- | --- | --- |
| 1 | `!attacking && engaged` -> `009C8310(task)`, the entry chooser | `009C83FF` |
| 2 | `!attacking && !engaged` -> `SetState(007B8AD0(unit) ? moveto : follow)` | `009C8419`-`009C845E` |
| 3 | `attacking && !engaged` -> the same `moveto`/`follow` choice, the abort back to approach | `009C8483` |
| 4 | `attacking && engaged && ctl->+370h == 0` -> `SetState(prepare)` | `009C8467`-`009C84F5` |
| 5 | state is `done` -> nothing further | `009C84FD` |
| 6 | `task->vtable[+1Ch]()` (`009C8A90`) -> `SetState(done)` | `009C8506`-`009C8514` |
| 7 | `prepare` -> `009C8310(task)` | `009C8530` |
| 8 | `attackrun` -> `task->+4C8h != 0` -> `SetState(flyabove)` | `009C853B`-`009C8545` |
| 9 | `flyabove` -> see below | `009C854D`-`009C85FE` |
| 10 | `turndown` -> `009C7EA0()` -> `SetState(aimdive)` | `009C8609`-`009C8629` |
| 11 | `aimdive` -> `aimdive+19h == 0` -> `SetState(aimglide)`; else `approach+D1h == 0` -> `SetState(goaway)`; else `aimdive+18h != 0` -> `SetState(goaway)` | `009C8634`-`009C8682` |
| 12 | `aimglide` -> `009C7850()` -> `SetState(goaway)`; else `aimglide+18h != 0` -> `SetState(goaway)` | `009C868D`-`009C86BF` |
| 13 | `goaway` -> `009C7F00()` -> `SetState(task->+4C9h ? flyabove : done)` | `009C86D3`-`009C86F7` |

Step 9, the roll-in, is the one branch with arithmetic:

```
if (flyabove->+19h != 0) {
    if (flyabove->+18h == 0)  SetState(aimglide);            // 009C8557
    else {
        side = flyabove->+20h;                               // 009C8563
        if (side == 0) {
            if (unit->+C68h > [00CE398C])      side = +1;    // 009C856B
            else if (unit->+C68h < [00D1FBC0]) side = -1;    // 009C8574
            else { BSP_Random_UniformFloatRange(1, 0.0, [00D05B54] = 1024.0);
                   side = (rand() >= 0x200) ? +1 : -1; }     // 009C858D-009C85B6
        }
        009C7800(turndown, side);                            // 009C85C1
        SetState(turndown);
    }
} else if (flyabove->+1Ah != 0) SetState(goaway);            // 009C85F5
```

`009C7800(turndown, sign)` writes `turndown->+18h = UniformFloatRange(1, [00CE74F8] = 0.8, 1.0) *
(sign >= 0 ? 1.0f : -1.0f)`: a randomised roll magnitude carrying the chosen side. `ECX` is the
turndown state, from `009C856B`/`009C85B8 LEA EDI,[ESI+79Ch]` then `MOV ECX,EDI`.

`009C82D0` is the `SetState` helper: exit slot `+8h`, store to `task+310h`, enter slot `+4h`.
`009C83E0` and `009C8310` both inline the same triple in places and call it in others.

### `009C8310`, the entry chooser

Body `009C8310`-`009C83D2`. The analogue of the torpedo's `009D3F60`.

| order | test | next state | address |
| --- | --- | --- | --- |
| 1 | `ctl->+370h == 0` | `prepare` `+5C4h` | `009C8319` |
| 2 | `task->+4C9h == 0 && (ctl->+369h == 0 \|\| [00E17BF2] == 0)` | `done` `+664h` | `009C834F`-`009C8371` |
| 3 | `task->+4C8h != 0` | `flyabove` `+778h` | `009C8379` |
| 4 | otherwise | `attackrun` `+7BCh` | `009C83AE` |

That is the torpedo's chooser with `+529h` -> `+4C8h`, `+52Ah` -> `+4C9h` and `aim` -> `flyabove`.

### The two flags, and where they come from

`task+4C8h` and `task+4C9h` are approach-relative: `4C8h - 3F8h = D0h` and `4C9h - 3F8h = D1h`,
and `009C8664` reads the second one through the state's owner pointer as `(aimdive->+4h)->+D1h`.
Both are written by the approach update, section (4). This is the same relationship the torpedo has
between `task+529h`/`+52Ah` and `approach+131h`/`+132h`.

## (4) `009C7A80`, the approach update

Body `009C7A80`-`009C7E9E`, `__thiscall(approach, float dt, bool diving)`, `RET 8`.
**Coverage: partial** - the tail `009C7C5B`-`009C7E9E` is transcribed for its outputs only.

| output | rule | address |
| --- | --- | --- |
| `approach+C4h` | `+= dt` | `009C7A8C` |
| `approach+ACh` | `= ctl->+398h`, the cruise profile's second altitude | `009C7A96` |
| `approach+A8h` | `= min(approach+A8h, ctl->+39Ch - approach+A8h * [00D7A270])`, the double `0.05` | `009C7A9E`-`009C7AB4` |
| `approach+D1h` | `= BSP_WeaponController_HasGeneralBombOrdnance` | `009C7AFE` |
| `approach+D0h` | `= 0` and return when `approach+48h`, the target, is null | `009C7B0A` |
| `approach+BCh` | the planar distance `sqrt(dx^2 + dz^2)` to the target, `0` below the `1e-10` at `00CE3820` | `009C7B4F`-`009C7B80` |
| `approach+C0h` | `[00CE3830] - atan2(...)`, the double `pi/2`, `+= [00CE3828]` (`2pi`) when negative | `009C7B8A`-`009C7BB0` |
| `approach+D0h` | the in-range hysteresis, below | `009C7BFB`-`009C7C31` |
| `approach+D8h`/`+DCh`/`+E0h` | the lead point the `moveto` and the aim states steer at | `009C7D4A`-`009C7E60` |
| `approach+74h` | a scalar clamped into `[0, [00CE7630]]`, else `[00CE38C8]`, else `0` | `009C7E74`-`009C7E9A` |

The in-range latch is a hysteresis, not a compare:

```
if (approach->+D0h == 0)                     threshold = approach->+B8h;
else if (ctl->+369h == 0 || [00E17BF2] == 0) threshold = approach->+B8h + [00D7A220];  // 100.0
else                                         { approach->+D0h = 1; ... }
approach->+D0h = !(threshold <= approach->+BCh);
```

It arms inside `approach+B8h`, disarms only past `approach+B8h + 100`, and while the control flag
and the global both answer it never disarms at all. `009C7C3B`-`009C7C57` then ANDs it with a
second planar test when there are **no** bombs left.

`BSP_WeaponController_HasGeneralBombOrdnance` `007B9320` is the `divebomb` ordnance test
`docs/ATTACK_COMMANDS.md` already records: weapon-device kind `2Ah` accepted only when the same
descriptor answers none of `2Ch`, `31h`, `2Bh`, `33h`, `2Dh`. **So the bomb family is the residual
one**: a bomb slot that has its own command class (level bomb, rocket, torpedo, depth charge,
drop-kamikaze) does not count, and what is left is what the dive bomber drops.

**So every input of the dive-bomb state machine is an output of `009C7A80`**, the same finding
`docs/TORPEDO_TASK_ARM.md` reaches for `009D3420`.

## (5) The four release sites

`tools/callsite_census.py 007bbba0` is exhaustive and finds exactly four in this class:

| site | where it lives | on the ordered-attack path? |
| --- | --- | --- |
| `009C60F1` | `009C58D0`, the **aimdive** tick | yes - the dive-bombing release, one bomb per gate |
| `009C5777` | `009C5180`, the **aimglide** tick | yes - the glide-bombing release, a whole salvo |
| `009C8026` | `009C7FF0`, task vtable slot `+58h` | not per tick; a task-teardown path |
| `009C88C4` | `009C8790` step 8, the manual passthrough | yes, but only outside the six refused states |

`009C88C4` is **not** in `009C83E0`. The census names `FUN_009c83e0` for it because that is the
nearest preceding Ghidra function and its body ends at `009C8785`; the site is inside the arm,
which Ghidra never defined.

### `009C60F1`, the aimdive release

The aimdive tick `009C58D0`-`009C6161`, `__thiscall(state, float dt)`, `RET 4`. It sets
`state+19h = 1` at `009C58DE` and counts `state+1Ch` down by `dt` at `009C58E9`.

Three gates, all at `009C60A9`-`009C60EC`:

| gate | rule |
| --- | --- |
| below the floor | `approach->+A8h > pose->+100h` (the world Y). `FCOMPI` at `009C60B5`, `JBE` skips |
| re-armed | `!(0.0f < state->+1Ch)`. `COMISS xmm0(0.0), [ESI+1Ch]` / `JB` at `009C60BB` |
| on aim | `\|error\| < [00CE3880]`, the **double** `25.0`. The fold is `[00D7A208] = -0.0f` minus the value |

Then `007BBBA0(unit)` at `009C60F1`, `approach->+2Ch -= 1` at `009C60FB`, and
`state->+1Ch = BSP_Random_UniformFloatRange(1, [00CE3800] = 0.5, 1.0)` at `009C6114`.

`009C6131`-`009C6154` runs on every path: `state->+18h = 1` when
`approach->+A8h * [00D7A280] > pose->+100h`, the double `0.5`. That is the **pull-out edge** the
transition rule reads at `009C8677`: the dive ends at half the drop altitude.

The dive also has an abort, `009C5AFD`-`009C5B48`, which clears **both** `state+19h` and
`state+18h` and returns early. It needs `approach->+D4h + approach->+50h > slant`,
`pose->+C64h > [00D20338] = -1.0471976` and `slant * [00CE3DC8] + [00CE3DD8] > aimPointDistance`
(the doubles `0.3` and `150.0`). Clearing `+19h` is what sends the state to `aimglide` next frame.

The enter and exit slots carry the dive-brake flag: `009C5870` writes `state+1Ch = 0.0f`,
`state+18h = (approach->+D1h == 0)` and **`unit->+844h = 0`**; `009C58A0` restores
`unit->+844h = 1`. Both are `no_ghidra_function`.

### `009C5777`, the aimglide release

The aimglide tick `009C5180`-`009C580B`. Four gates at `009C5693`-`009C5755`:

| gate | rule |
| --- | --- |
| shallow | `[00CEC724] = 0.5235988 (30 deg) > diveAngle` |
| height | `heightAbove + [00CE3938] (50.0, double) > heightLimit` |
| lateral | `[00D1F3F8] (120.0, double) > \|lateralA - lateralB\|` |
| lead | `lead + acc > acc + [00D7A370] (5.0)` and `-(lead + acc) * [00D7A2B0] (3.0) > acc + 5.0`, where `lead = lateralB - cos(diveAngle) * lateralA` |

Then the **salvo**: `count = min(007C1DB0(unit), cap)` at `009C575D`-`009C5766`, and the loop
`009C5771`-`009C5784` calls `007BBBA0(unit)` once per round with one `approach->+2Ch -= 1` each.
`009C579E` sets `state->+1Ch = UniformFloatRange(1, [00CE54A0] = 0.2, [00CE3800] = 0.5)` and
`009C57B8` advances `state->+20h += approach->+A4h * [00D04690]` (the double `0.35`).

`007C1DB0` is the round counter: it walks the device list at `unit+48h` and sums `006E3500` over
every device whose `vtable[+5Ch]` answers `25h`. The HUD's unit rows call the same routine
(`00648C20`), so it is the bomb count the player sees. The aimglide **enter** `009C4F00` calls it
too, which is where the `cap` is latched.

**Coverage: partial.** The four frame slots the gates read (`[ESP+10h]`, `[ESP+14h]`, `[ESP+18h]`,
`[ESP+20h]`) were not traced to their producers. The gates, their constants and the loop are
transcribed; the geometry behind them is not.

## (6) What the two aim states command

`cmd` is `approach->+18h` = `task+4h`, the command block `docs/BOT_TASK_STATES.md` owns, and the
five slots are the ones `docs/PILOT_CONTROLS.md` rows 0..4 and `docs/PILOT_COMMAND_PATH.md` name:

| slot | live / desired / active | unit field | mode word |
| --- | --- | --- | --- |
| 0 | `+274h` / `+278h` / `+27Ch` | `+9F0h` throttle | `+2D8h` |
| 1 | `+280h` / `+284h` / `+288h` | `+9E4h` `yawInput` | `+2D4h` |
| 2 | `+28Ch` / `+290h` / `+294h` | `+9ECh` `rollInput` | `+2CCh` |
| 3 | `+298h` / `+29Ch` / `+2A0h` | `+9E8h` `pitchInput` | `+2D0h` |
| 4 | `+2A4h` / `+2A8h` / `+2ACh` | `+9F4h` `airBrakeInput` | `+2D8h` |

The two aim states use the block in **opposite ways**, and that is the whole difference between a
dive and a glide:

| state | writes | reading |
| --- | --- | --- |
| `aimdive` `009C58D0` | `+29Ch`/`+2A0h`/`+2D0h`, `+290h`/`+294h`/`+2CCh`, `+284h`/`+288h`/`+2D4h`, `+278h`/`+27Ch`, `+2A8h`/`+2ACh`, `+2D8h` (`009C5CFA`, `009C5DA3`, `009C5E5D`, `009C605F`, `009C6071`) | every axis raw, with mode `0`: the dive flies the aircraft directly |
| `aimglide` `009C5180` | `+2C4h`, `+2C0h`, `+2BCh` with modes `+2CCh`/`+2D0h`/`+2D4h`, plus `+278h`/`+27Ch` and `+2A8h`/`+2ACh` (`009C5400`, `009C5442`, `009C55D7`, `009C5663`) | the autopilot fields: a commanded heading and altitude |

`goaway` `009C4A40` and `turndown` `009C44F0` both use the autopilot fields; `turndown` also writes
the pitch slot at `009C4694` and the speed flag `cmd->+2B0h = 0` at `009C4518`.
`approach->+CCh` is the weapon selector: `aimdive` writes `0` at `009C58D9`, `flyabove` writes `3`
at `009C6690` (the counterpart of the torpedo's `approach->+A4h = 3`).

## (7) `009C8200`, the arming entry, and why nothing spends it

Task vtable `00D20E18` slot `+24h`, body `009C8200`-`009C8255`, `__fastcall(task) -> bool`, `RET 0`.
Ghidra already carries `BSP_BotTask_AccumulateManualReleaseRequest` on it from packet
`cc8_torpedo_first_release_authority`; this packet appended the dive-bomb reading.

Gated on `task->+4C9h`, the unit, and `(unit+72Ch)->vtable[+38h]`. On a pass it does
`task->+424h += 1` at `009C822C` and, when the state is `prepare`, writes
`task->+65Ch = [00CE3850] = 5.0f` at `009C8248`. `65Ch - 5C4h = 98h`: that is the same
`prepare+98h` release countdown the torpedo's `009D49A0` raises.

**Nothing in this class spends it.** The dive-bomb `done`/`prepare` tick `009C7270` is five
instructions - it pushes `dt`, **calls** the follow base `009C1FD0` at `009C7278` and returns with
`RET 4` at `009C727D`, so the body runs to `009C727F` inclusive - and the exit `009C7260` is a bare
`JMP 009BDE40`. The torpedo's `009D2720` release logic and its `009D2570` exit drop have no
counterparts here. `prepare+98h` is written and never read in the dive-bomb path.

Unlike `009D49A0`, every path of `009C8200` leaves `AL = 1` (`009C8227 MOV EAX,1` feeds both the
`ADD` and the return; the refusal path sets `AL = 1` explicitly at `009C8252`), so it never stops
`BSP_PilotBot_Tick`'s walk over the task vector.

## (8) The cruise profile and the break-off

Both already tabulated in `docs/BOT_TASKS.md` and confirmed here.

`009C8920` (slot `+54h`) is the ten-class shape: base `0099B660`, the `007B8AD0` gate, then
`ctl->+394h = Pilot/DiveBomb/CruisingAlt` (1300), `ctl->+398h = BeginAltRange/1` (1000) and - one
of only two classes that do - `ctl->+39Ch = [00CE4C04] = 9999.0f`, each with its own
`+38Ch`/`+38Dh`/`+38Eh` override byte and `+3A9h`/`+3AAh`/`+3ABh` one-shot, then the
`AttackDist` (1100) clamp on `this->+43Ch` and the tail `0099B740`.

`009C8A90` (slot `+1Ch`, Ghidra `BSP_BotTaskDiveBomb_ShouldBreakOff`) is the four-class shape with
latched target `+440h`, the class extra `this->+4C9h == 0` and `Pilot/DiveBomb/SafeDist` = 100.
Note the class extra: the dive bomber breaks off on **distance** only once it is out of bombs.

## (9) Which command installs kind 8

`009C8C70`, the factory. `docs/ATTACK_COMMANDS.md` gives the gate: command class `divebomb`
`00E08F20`, chosen at `007EE9C5`, requires a surface target, the unit **not** `IsKindOf(10h)`,
general bomb ordnance (`007ED7E0` -> `007B9320`) and the target not `IsKindOf(0Eh)`. The selection
order tries `levelbomb` first, and `levelbomb` needs `IsKindOf(10h)`, so exactly one of the two
applies to any aircraft. The task's target must answer `IsKindOf(2)`.

## ABI summary

| address | ABI | evidence |
| --- | --- | --- |
| `009C8790` | `void __thiscall(task, float dt)`, `RET 4` | `009C88D3 RET 4`; the caller's `PUSH ECX`/`FSTP [ESP]` |
| `009C83E0` | `void __fastcall(task)`, `RET 4` | `009C845E`, `009C8584`; the arm pushes a `dt` the body never reads |
| `009C8310` | `void __fastcall(task)`, `RET 0` | `009C8378`, and the tail `JMP EDX` at `009C834D` |
| `009C82D0` | `void __thiscall(task, state)`, `RET 4` | the one pushed pointer at every call site |
| `009C7910` | `bool __thiscall(task, state)`, `RET 4` | `009C7966`, `009C796B` |
| `009C7A80` | `void __thiscall(approach, float dt, bool diving)`, `RET 8` | the arm's frame at `009C87EF`/`009C880C` |
| `009C8200` | `bool __fastcall(task)`, `RET 0` | `009C8251`, `009C8255` |
| `009C7FF0` | `void __fastcall(task)`, `RET 0` | `009C8057`, and the tail `JMP EDX` at `009C8055` |
| `009C58D0` | `void __thiscall(state, float dt)`, `RET 4` | `009C5B51`, `009C615F` |
| `009C5180` | `void __thiscall(state, float dt)`, `RET 4` | the vtable slot and the arm's call shape |
| `009C7270` | `void __thiscall(state, float dt)`, `RET 4` | `009C727D` |
| `009C7800` | `void __thiscall(turndown, int sign)` | the one pushed `EAX` at `009C856E`/`009C85BE` |
| `007C1DB0` | `int __fastcall(unit)` | no pushed argument at `009C575D` |

## Host methods

One row per native call site the reconstruction models. The host lives in
`include/bsp/dive_bomb_task.hpp`; `src/dive_bomb_task.cpp` is the sequence.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `009C87A3` | - | `set_plan_step_scratch` | `task / 0FFh / void` | always |
| `009C87EA` | `009C7A80` | `update_dive_bomb_approach_009c7a80` | `task+3F8h / dt, diving / void` | always |
| `009C8825` | `009BDE80` | `refresh_move_to_ranges` | `task+4F0h / near, far, speed / void` | always |
| `009C8834` | `009C83E0` | `read_transition_inputs` + `set_dive_bomb_state` | `task / dt / void` | always |
| `009C85C1` | `009C7800` | `write_turn_direction` | `turndown / roll / void` | flyabove -> turndown |
| `009C884C` | `state->vtable[0Ch]` | `tick_state` | `state / dt / void` | always |
| `009C885B` | - | `commit_plan_step_result` | `task / - / void` | always |
| `009C887C` | `(unit+72Ch)->vtable[38h]` | `manual_release_requested` | `unit+72Ch / - / bool` | `task+424h > 0` |
| `009C88C4` | `007BBBA0` | `request_ordnance_release` | `unit / - / void` | step 8 of the arm |
| `009C60F1` | `007BBBA0` | `request_ordnance_release` | `unit / - / void` | the aimdive gate |
| `009C5777` | `007BBBA0` | `request_ordnance_release` | `unit / - / void` | the aimglide salvo, once per round |
| `009C575D` | `007C1DB0` | `rounds_remaining_007c1db0` | `unit / - / int` | the aimglide salvo |
| `009C6114`, `009C579E`, `009C859E`, `009C7839` | `00BD2F10` | `uniform_between` | `- / low, high / float` | the four draws |
| `009C60FB`, `009C577F` | - | `spend_round` | `approach / - / void` | every release |
| `009C7AFE` | `007B9320` | `approach_has_bomb_ordnance` | `controller / - / bool` | the approach update |

The consumer of `007BBBA0` is already reconstructed: `docs/TORPEDO_RELEASE_SPAWN.md` and
`src/torpedo_release_spawn.cpp` own the request, the actuator block and the gun path. This packet
adds no new contract there.

## Contract for the held files

`src/bot_task_states.cpp`, `include/bsp/bot_task_states.hpp` and `src/game_hosts_units.cpp` were
held by another worker for this window, so the host wiring is not in this commit. What it needs:

1. `include/bsp/bot_task_states.hpp`: **no change**. `DiveBombTaskHost` derives from the existing
   `BotTaskStateHost` and adds its own virtuals in `include/bsp/dive_bomb_task.hpp`.
2. `src/game_hosts_units.cpp`: the unit host already implements `release_ordnance_007bbba0`
   (`docs/TORPEDO_RELEASE_SPAWN.md` "Host methods"). The dive-bomb host reuses it unchanged; the
   only new binding is `rounds_remaining_007c1db0(unit)`, which should sum the unit's ordnance
   devices the way `007C1DB0` does, and `uniform_between`, which already exists for the torpedo.
3. The per-ordered-aircraft census the packet asks for needs one counter per state
   (`DiveBombState`), the dive entry altitude and angle, the release altitude, speed and range, the
   bombs released and the `007C1DB0` count at each salvo. `dive_bomb_task_arm_009c8790` returns all
   of those in `DiveBombArmTickResult`; the host only has to accumulate them.

## Corrections

### Correction to `docs/BOT_TASKS.md` (packet `cc2_bot_tasks`)

That doc's registrar table gives the `divebomb` `009C2AC0` arguments as
`approach->+A8h - [00D7A220]`, then `approach->+A8h`, with the fifth `contract: unread`. That is
right for the **constructor** `009C73A0` (`009C7460`), where the fifth argument is
`approach->+B4h`. The **per-tick** refresh in the arm uses a different pair: `009C87EF`-`009C8825`
passes `task+4A4h - [00D7A220]`, `task+4A4h` and `task+4ACh`, i.e. `approach+ACh` and
`approach+B4h`. `approach+ACh` is `ctl->+398h`, which `009C7A80` copies in every tick, so the
moveto range tracks the live `BeginAltRange` rather than the constructor's snapshot.

The same doc's slot `+24h` row is `contract: unread` for every class. For `divebomb` it is
`009C8200`, and section (7) above records what it does and why nothing in this class spends the
countdown it raises.

### Correction to `docs/BOT_TASK_STATES.md` (packet `cc7_bot_task_states`)

That doc's open question asks whether `cmd->+2A8h`/`+2ACh` is a second weapon channel. It is not:
`docs/PILOT_CONTROLS.md` row 4 pairs `+2A4h`/`+2A8h`/`+2ACh` with `unit+9F4h`, and
`docs/PILOT_COMMAND_PATH.md` names `unit+9F4h` `airBrakeInput`. The dive-bomb `aimdive` tick writes
it at `009C6071` in the same breath as the throttle at `009C605F`, which is what an air brake on a
dive is for. This packet did not re-read `009A3770` or `009D07B0`, so the claim is about the field,
not about those two sites.

## `no_ghidra_function`

| start | end (inclusive) | name |
| --- | --- | --- |
| `009C8790` | `009C88D5` | `BSP_BotTaskDiveBomb_TickArm` |
| `009C88E0` | `009C8916` | `BSP_BotTaskDiveBomb_GetTargetPoint` (task vtable slot `+Ch`) |
| `009C5870` | `009C5892` | `BSP_BotStateDiveBombAimDive_Enter` |
| `009C58A0` | `009C58AD` | `BSP_BotStateDiveBombAimDive_Exit` |
| `009C7240` | `009C725B` | `BSP_BotStateDiveBombDone_Enter` |
| `009C7260` | `009C7264` | `BSP_BotStateDiveBombDone_Exit` |
| `009C7270` | `009C727F` | `BSP_BotStateDiveBombDone_Tick` |
| `009C62B0` | unread | the flyabove tick, `00D20D04` slot `+Ch`; censused for its writes only |

## Validation

`./scripts/build.ps1` (MSVC Win32, `/W4 /WX`) succeeds before and after merging `main`;
`ctest --test-dir build/win32 -C Release` passes both existing suites. No test was added.

`tools/verify_report_calls.py reports/dive_bomb_task.json`: 27 call rows checked, 0 failed.

Ghidra was not mutated. The ledger carries 21 names from this packet.

## Follow-up packets

1. **The aimdive aim error.** `009C5C92` calls `BSP_Math_InterpolateClamped` with three pushed
   floats and the result becomes both the pitch command and the release gate at `009C60C1`. Until
   its units are established, whether the `25.0` at `00CE3880` is a real cone or a formality is
   open. That is the single highest-value unknown left in this class.
2. **The aimglide geometry.** The four frame slots behind its four gates.
3. **The flyabove tick `009C62B0`.** It owns `+18h`, `+19h`, `+1Ah` and `+20h`, which decide roll-in
   versus glide versus break-off. No Ghidra function.
4. **`approach+D4h` and `approach+B8h`.** The two ranges the dive abort and the in-range latch key
   on; neither producer was found in the part of `009C7A80` that was read.

## Addendum: `009C8920` read to its end

Body `009C8920`-`009C8A8B`, `INT3` from `009C8A8C`. Beyond the ten-class shape:

* Step 6 writes the third altitude through a base register: `009C8A06`-`009C8A0E`
  `MOVSS [EAX+20h], [00CE4C04]` with `EAX` holding `ctl+37Ch`, i.e. `ctl->+39Ch = 9999.0f`.
* Step 7's clamp target is `task+4B0h`, **not** the `+43Ch` `docs/BOT_TASKS.md` gives for the depth
  charge: `009C8A1B` calls `0042E740` for the tuning singleton, `009C8A20`-`009C8A26` computes
  `tuning+4C4h * task+41Ch`, and `009C8A5E` stores `max(that, task+4B0h)` back to `task+4B0h`.
* `009C8A74` writes `task+4BCh = [00CFDEB0]`.
* `009C8A7C` runs the approach update a **second** time, `009C7A80(task+3F8h, 0.0f, false)`:
  `009C8A58 PUSH 0` is the `diving` bool and `009C8A56 FLDZ`/`009C8A5B FSTP [ESP]` the `dt`. This is
  an independent confirmation of the two-argument, `RET 8` ABI section (1) derives from the arm's
  frame.
* `009C8A87` is the tail jump to `0099B740 BSP_BotTask_AbandonIfStale`.

## Addendum: the flyabove tick's three flags

`009C62B0`, vtable `00D20D04` slot `+Ch`, **no Ghidra function**, and this packet did not find its
inclusive end: `coverage: partial`. What it does establish is who writes the three bytes the
transition rule's step 9 branches on, all `state`-relative with `ESI` = the state:

| address | write |
| --- | --- |
| `009C659F` | `+18h = 0` |
| `009C66E3` | `+1Ah = 1` |
| `009C66E7` | `+19h = 0` |
| `009C66F2` | `+1Ah = 0` |
| `009C6A30` | `+19h = +18h`, gated on `[00CF180C] > cos(...) * [ESP+28h]` at `009C6A27` |
| `009C6690` | `approach->+CCh = 3`, the weapon selector |

So `+19h` is the roll-in permission and it is a **copy of** `+18h` taken once the aircraft's
geometry over the target closes; `+1Ah` is the separate break-off request that sends the state to
`goaway` when `+19h` never arms. The transition rule reads `+19h` first, then `+18h`, then `+1Ah`,
which is exactly that order.

## Addendum: the aimdive release gate is 25 metres, not an angle

The follow-up this doc listed first is settled. `009C59BA`-`009C5C9B` computes the quantity the
aimdive tick both steers on and releases on, and every term in it is metres.

| step | address | value |
| --- | --- | --- |
| the height above the target | `009C59BA`-`009C59D6` | `aircraft.y` (a **qword** store of `pose+100h` at `009C59C2`) minus `approach->vtable[0]()`'s `.y`; the `FSUBR` at `009C59D2` makes the memory operand the minuend |
| the x window | `009C5BEE`-`009C5BF7`, `009C5C27`-`009C5C38` | `x0 = approach->+A8h + [00D7A220]` (the double `100.0`), `x1 = approach->+ACh + approach->+50h` |
| the lead | `009C5C49` | `BSP_Math_InterpolateClamped(x0, 0.0, x1, (approach->+14h)->+5Ch, height)` |
| the along-track miss | `009C5C4E` | `FSUBR [ESP+28h]`, so `cos(bearingError) * planarDistance - lead`. `[ESP+28h]` was written at `009C5C14` as `cos(bearingError) * planarDistance`; the bearing comes from `BSP_Math_SubtractWrappedAngle` at `009C5AF1` and the distance from the `sqrt` at `009C5A40` |
| the gain | `009C5C92` | `BSP_Math_InterpolateClamped(x0, 1.0, x1, (approach->+14h)->+60h, height)`, dimensionless |
| the error | `009C5C97`-`009C5C9B` | `gain * alongTrack`, stored in the slot the release gate and the pitch law both read |

`00419010 BSP_Math_InterpolateClamped` takes **five** stack floats and is `RET 14h`; both call sites
open their window with `SUB ESP,14h` (`009C5BFD`, `009C5C52`), which is what makes the five-argument
reading the only one that balances.

So `[00CE3880]` is a **25-metre along-track window**, a real gate, and the pitch command at
`009C5CAB`/`009C5CD0` scales the same metres by the class descriptor's `+64h` (positive error) or
`+68h` (negative) and clamps to `+/-1`. The `009C5D08` sign test that picks between those two
scales reads the same slot, which is the independent confirmation that it is signed and not an
absolute value.

Still unread: the two y endpoints `(approach->+14h)->+5Ch` and `+60h`, and the producers of
`approach+D4h` and `approach+B8h`.

## Validation: the IJN01 before-run

`local/ijn01_before.log`, tree `9cff2668d` (`main` at `0c50b0a88`), 3000 mission frames at 0.05 s.

| measure | value |
| --- | --- |
| ordered aircraft | 33: 8 `A7M`, 9 `JudySpawn`, 12 `JillSpawn`, `Warhawk1`, `Dauntless1`, 2 `B-17` |
| units carrying general bomb ordnance (`007B9320`'s predicate) | 27 of 80 units with guns |
| `BOMBPLATFORM` guns | 44, **0 shots** |
| torpedo task | aircraft 6, releases 0 |

The nine `JudySpawn` aircraft are the dive-bomber population and `Dauntless1` is a tenth. No
dive-bomb instrumentation exists in this tree, so the task's own state counts, dive entry altitude,
release altitude and bomb count are **not measured**: that is what the wiring commit adds.

### The next gate, and why it is not the torpedo's

The same run reports the torpedo's blocker: `unit+C58h` at `0099AF53` in `BSP_PilotBot_Tick` is 0 on
every one of 9000 ticks, so the arming loop at `0099AF81` never offers a task its `vtable[+24h]`,
`prepare+98h` stays at `-1.0` and `009D2720` never reaches `007BBBA0`.

**That gate cannot block the dive bomb's own release.** The dive bomb's two live release sites,
`009C60F1` and `009C5777`, are inside state ticks the arm reaches through `state->vtable[+Ch]` at
`009C884C`, not through the arming loop. `009C8200` feeds only `task+424h` (the manual passthrough
at `009C88C4` and the teardown at `009C8026`) and `prepare+98h`, which nothing in this class spends.
So the dive bomber's predicted gate is upstream of that: the transition rule has to reach `aimdive`
or `aimglide`, which needs `engaged` at `009C83F8` to answer, which needs `approach+D0h` set by the
in-range latch at `009C7C31`, which needs `approach+BCh < approach+B8h`. The torpedo aircraft in
this run sit in `goaway` with their engage pair at zero, so the dive bomb's first measurement should
be `approach+B8h` and `approach+BCh` per ordered Judy.

## Validation: the wiring, and the two gates the runs expose

`src/game_hosts_units.cpp` runs the kind 8 task on the pilot think, right after the torpedo arm,
for every ordered aircraft whose class carries general bomb ordnance and no torpedo. The release
binding is the torpedo's own `release_ordnance_007bbba0`, so a drop goes down the same chain to the
gun spawn `0072F830`; `rounds_remaining_007c1db0` is the only new one.

| run | commit | aircraft on the task | states | releases | bombs spawned |
| --- | --- | --- | --- | --- | --- |
| IJN01 after, `local/ijn01_after.log` | `86ef889c4` | 27 | `flyabove` 1, `turndown` 1499 each | 0 | 0 |
| USN01 after, `local/usn01_after.log` | `86ef889c4` + the attitude fix | 5 | the same | 0 | 0 |

IJN01's 27 are every general-bomb carrier except the six Jills the torpedo task already took, which
matches the run's own `general_bomb=27`. USN01's five are `ScoutDauntless`, `ConSBD1..3` and
`KatSBD`. Every other IJN01 summary is bit-identical to the before-run, including
`plane motion distance_moved=347370.56 m` and `pilot attack ordered=33`, so the wiring moved nothing
else.

### Gate 1: the commanded target is coincident, so `approach+BCh` is 0

Every dive bomber reports `approach+BCh = 0.0 m` against `approach+B8h = 1100.0 m`. That is not the
task's doing: the pilot-attack census, which resolves the same `command_target_plus_one` over all
three axes, independently reports `range 0.0 -> 0.0 m` for all 33 ordered IJN01 aircraft and for
each of the five USN01 dive bombers. The approach geometry is degenerate before the task sees it.

With a zero range the machine still runs, and runs the way the rules say it should: the latch
`approach+D0h` arms at `009C7C31` because `1100 > 0`, `engaged` answers at `009C83F8`, the entry
chooser takes the `flyabove` arm at `009C8379` because the latch is set, and the roll-in at
`009C8563` fires on the first tick.

### Gate 2: `009C7EA0` never completes, so the state stays in `turndown`

All 1499 remaining ticks sit in `turndown`. `009C7EA0` returns 0 while
`pose+C64h >= [00D1F98C] = -1.2999999523162842` and `pose+C64h >= -1.0`, and the measured
`pose+C64h` is about `0.027 rad` (the run's own `final_pitch_mean`). Nothing ever rolls the
aircraft past the gate because the producer that would, the turndown tick `009C44F0`
(vtable `00D20C84` slot `+Ch`, censused here for its command-block writes only), is not bound.

**So the next gate is `009C7EA0` at `009C8613`, value `pose+C64h = 0.027 rad` against
`-1.2999999523162842` at `00D1F98C`, and the work that opens it is binding `009C44F0`.**

### A reversed pair, found by the run and fixed

The first IJN01 after-run passed `009C7EA0` its two pose angles in axis-name order rather than
offset order: `plane_bank_angle_c68` where the body reads `pose+C64h`. Fixed at the call site, and
the reconstruction's parameters renamed to `attitude_c64`/`attitude_c68` so the names carry the
offsets rather than a claim about which axis each holds. At the attitudes both runs measured, with
both angles near zero and above `-1.0`, either ordering returns the same answer, so the IJN01
census above stands; USN01 was run with the fix in and shows the identical pattern.

### The substitutions in the wiring, each labelled at its address

| what | why | where |
| --- | --- | --- |
| `approach+A8h = 800` | `009C3ED8` draws it from `(approach+14h)->+38h..+3Ch`, an unread class record. 800 is `BeginAltRange/1 - (BeginAltRange/2 - /1)`, built from the two rows the seed itself reads, and it satisfies the interpolation window's `approach+A8h + 100 < approach+ACh` | `009C3ED8` |
| `approach+D4h`, `+50h`, `(approach+14h)->+5Ch`, `->+60h` all zero or one | no producer read. The lead stays 0 and the gain 1, so the aim error is the bare along-track miss | `009C5C20`, `009C5C69` |
| the flyabove tick's three flags | `009C62B0` has no Ghidra function; the host stands in with the planar range against `Pilot/DiveBomb/SafeDist` | `009C62B0` |
| two rounds per aircraft | `006E3500`'s per-device count is unread; this is the cap the aimglide salvo loop clamps against | `007C1DB0` |

### USN02 after: the wiring is provably inert

`local/usn02_after.log`. The mission carries no aircraft, and the run's own line is
`summary mission pilot attack: no unit was ever ordered at a target the yaw arm could plan for`.
The dive-bomb census prints nothing at all: zero `divebomb` lines and no
`summary mission dive-bomb task`. That is identity by construction rather than by comparison, since
`run_dive_bomb_task_arm_009c8790` returns before doing anything when `command_target_plus_one` is 0,
which is the first test in its body. The gunnery, damage and world-unit summaries are the standing
USN02 ones (`queued_hits=168 deaths=3 total_damage=20721.4`, `world units=32`).

An earlier USN02 attempt was refused rather than run: `tools/run_game.ps1` gave up after 900 s with
the machine-wide lock held by `cc8-ai-squadron`. That attempt produced no log and is discarded.

## `009C44F0`, the turndown tick (packet `cc8_dive_bomb_turndown`)

Vtable `00D20C84` slot `+Ch`, body `009C44F0`-`009C4736`, `__thiscall(state, float dt)`, `RET 4`.
The `dt` is never read. `ESI` is the state, `[ESI+4]` the approach, `approach->+18h` the command
block. Every jump sense below was read from the branch byte: `009C45BB` `76` `JBE`, `009C465B` `76`
`JBE`, `009C4687` `76` `JBE`.

### The speed command, before any branch

| address | write |
| --- | --- |
| `009C44FD` | `approach->+CCh = 0`, the weapon selector |
| `009C4512` | `cmd->+2B4h = 007C47F0(approach->+8h)` = `tuning+24Ch` `Dynamics/SpdMultipliers/LevelFlight` (1.8) x `classDesc+184h` `StallSpd` (17.5), the **level-flight speed**, about 31.5 m/s on a default class |
| `009C4518` | `cmd->+2B0h = 0` |
| `009C4524` | `cmd->+2D8h = 1` |

`approach+8h` is the plane class descriptor: `009C7A94` reads its `+188h` `MaxSpd` through the same
pointer. Both halves of the product are already named, `docs/GAME_TUNING_SINGLETON.md` row `+24Ch`
and `docs/PLANE_FLIGHT.md` row `+184h`, and `docs/PLANE_GROUND_OPS.md` step 6 forms the identical
product at `007CBD7F`, so the turndown asks for exactly the speed the ground-ops water check calls
level flight.

That answers the throttle question directly: the turndown **does not touch** the throttle slot
`plan+278h`/`+27Ch`. It writes the desired-speed pair, and `docs/PILOT_THROTTLE_CUT_RAISER.md`
shows `(+2B4h, +2D8h)` is one command, "here is the speed I want, act on it once", with `+2D8h` the
one-shot `BSP_PilotBot_PlanControls` spends. `009C4512`-`009C4524` is the same three-instruction
shape that doc records at `009C189A`-`009C18A7`, so the turndown is one of the states that raises
the one-shot.

### The bank, wrapped and folded

`009C4530`-`009C4575`: `00BF857A` with `ST(1)` = `pose+C68h` and `ST(0)` = the `2pi` at `00CE3828`,
so `fmod(bank, 2pi)`, then the pair of compares that pulls it into `(-pi, pi]` using the doubles
`-pi` at `00CE3D18` and `+pi` at `00CE3D28`. `009C457D`-`009C45A5` folds it to `|bank|` with the
usual `-0.0f` subtract at `00D7A208`. `009C45BD` then forms `pi - |bank|`, the angle still to roll
through to inverted, floored at zero by `009C45C7`-`009C45DD`.

### The two arms, on `state+1Ch`

**Not latched** (`009C45A9` `CMP byte [ESI+1Ch],0`, `JNZ`):

| test | arm |
| --- | --- |
| `\|bank\| < [00CE3D40] = 0.8 rad` (45.8 deg) | roll: `cmd->+290h = InterpolateClamped(30 deg, 1.0, 0.0, 0.0, pi - \|bank\|) * state->+18h`, `cmd->+294h = 1`, `cmd->+2CCh = 0`. At these banks the interpolation clamps at `1.0`, so the command is the full signed magnitude `009C7800` drew |
| otherwise | `009C4637`: hand the roll axis back, `cmd->+2C4h = [00D7A264] = pi`, `cmd->+2CCh = 1` |

Then on both arms: `009C4654` latches `state+1Ch = 1` once `|bank| > [00D1FED0] = 2.618 rad`
(150 deg), and `009C4660`-`009C4687` splits on `|pose+C64h|` against `[00CE398C] = 20 deg`. Inside
the band it writes `cmd->+29Ch = 0`, `cmd->+2A0h = 1`, `cmd->+2D0h = 0`; at or above it writes
`cmd->+2BCh = 0` with `cmd->+2D0h = 2`.

**Latched** (`009C46C9`-`009C4736`): release the roll the same way, then pull the nose down.
`cmd->+29Ch = InterpolateClamped(30 deg, 0.0, [00D0CBA0] = 3 deg, 1.0, pi - |bank|)`, with
`cmd->+2A0h = 1` and `cmd->+2D0h = 0`. The closer to inverted, the more nose-down stick: zero at
30 degrees from inverted, full at 3 degrees.

### Why it never reaches `009C7EA0`'s window

`009C7EA0` ends the turndown only when `pose+C64h` falls below `-1.3` at `00D1F98C`. The tick's own
pitch command is the only thing that would take it there, and the pitch arm is gated behind
`state+1Ch`, which is gated behind `|bank| > 150 degrees`, which is gated behind the roll command
at `cmd->+290h` actually rolling the aircraft. The host binds none of that yet, which is exactly
what the after-runs measured.

## Gate 1, read: the dive-bomb task does not exist in either mission

`approach+BCh`'s producer is `009C7B4F`-`009C7B80` inside the approach update `009C7A80`: the planar
distance between the point `approach->vtable[0]` returns and the pose's world position, with an
early `approach+D0h = 0` and return at `009C7B0A` when the latched target `approach+48h` is null.
The target position is **not** a command-block field. The arm never dereferences one: it goes
through `approach->vtable[0]`, and the object is whatever `approach+48h` holds.

What the runs show is upstream of all of that:

| run | `PilotSetTarget` orders | command classes chosen |
| --- | --- | --- |
| USN01 | 5, all to `Mav1`..`Mav5` | `00E08F18` torpedo, every one |
| IJN01 | 0 | none |

**No aircraft in either mission ever receives the divebomb class `00E08F20`.** The dive bombers'
authored order is `artillery`, which `0046AAB0` resolves to `attackmove` `00E08F78` outright
(`docs/ATTACK_COMMANDS.md`). `attackmove` never reaches `007EEC50`'s ordnance chooser, so
`0099A170` builds no attack task at all, and nothing latches `approach+48h`. The five aircraft that
do report a real range, `Mav1`..`Mav5` at about 4259 m, are the ones holding a `PilotSetTarget`
order, and they got the torpedo task, not this one.

So the 27 IJN01 and 5 USN01 installs in the after-runs are the **host's**, not the image's.

### The fix location

`run_dive_bomb_task_arm_009c8790` in `src/game_hosts_units.cpp` gates on
`command_target_plus_one != 0` plus the ordnance test. It should gate on the divebomb command class
having actually been chosen, the analogue of the `PilotSetTarget task: 0099A170` record the torpedo
path already emits. With that gate no aircraft in IJN01 or USN01 runs the task, which is the
image's own behaviour. Reaching a bomb release needs a mission that issues a `PilotSetTarget` to a
bomb-carrying aircraft that does not answer `IsKindOf(10h)`, or the `--order` injection
`src/game_hosts.cpp` provides.

**Closed.** The non-zero `command_target_plus_one` comes from the aircraft's authored **`moveto`**
row, not from an attack order. IJN01's authored-token census reports `moveto` x142 and
`summary mission commands` reports `resolved=142`, an exact match; the `artillery` x30 rows resolve
to `attackmove`, carry no target and print `current=0`, so `store_unit_command_target` skips them.
USN01 is the same shape with `resolved=34`.

So a dive bomber's commanded target is a **movement** target, and it is co-located with the
aircraft, which is why both the pilot-attack range and `approach+BCh` difference to exactly `0.0`.
That is the third reason the loose host gate was wrong: `command_target_plus_one != 0` is true for
any unit with a `moveto`, which is most of the mission.

## The class gate, and the host contract for it

`include/bsp/dive_bomb_task.hpp` now carries `kDiveBombCommandClass` = `00E08F20` and
`dive_bomb_task_installed_for_class`. The host edit that uses them is three lines:

1. `GameUnitSlot` gains `unsigned int attack_command_class{0};`.
2. `GameUnitsHost` gains `void store_unit_attack_command_class(std::size_t index, unsigned int c)`,
   the same shape as `store_unit_command_target`.
3. `GameScriptOrdersHost::run_pilot_set_target` calls it right beside
   `bsp::bot_install_command_task_0099a170`, where `chosen` is already in hand
   (`src/game_hosts_script_orders.cpp` around the `PilotSetTarget task:` line).
4. `run_dive_bomb_task_arm_009c8790` replaces its `command_target_plus_one != 0` and ordnance tests
   with `bsp::dive_bomb_task_installed_for_class(unit_.attack_command_class)`.

With that gate **IJN01 and USN01 both install zero dive-bomb tasks**, and that is the correct
result: neither mission ever hands an aircraft the divebomb class. Saying so plainly is the point.
The 27 and 5 installs the earlier census reported were the loose gate, not the game.

## The `--order` injection cannot express a `PilotSetTarget`

Read only; `src/game_hosts.cpp`, `src/game_hosts_mission_frame.cpp` and
`src/game_hosts_script_orders.cpp` are the Codex orchestrator's and were not edited.

`--order <token>:<target> --order-unit <name> --order-frame N` reaches
`GameUnitsHost::issue_player_command` (`src/game_hosts_units.cpp`), which resolves the token against
the **scene-command registry** the way `0046AAB0` does and places a command row. `settarget` is a
registry row (`src/entity_orders.cpp`, class `00E08EF8`), so the switch does place an order.

What it does not do is run the chooser. `007EEC50` and the `0099A170` install live only in
`GameScriptOrdersHost::run_pilot_set_target` (`src/game_hosts_script_orders.cpp`), reached from the
Lua binding `PilotSetTarget` `008A4C90` that a mission script calls. That is where the run's
`PilotSetTarget choose: 007EEC50 -> ...` and `PilotSetTarget task: 0099A170 -> 1` lines come from,
and nothing on the command line drives it.

**What the injection lacks, exactly:** a route from a command-line switch to
`GameScriptOrdersHost::run_pilot_set_target`, or to its tail `bsp::attack_command_choose` plus
`bsp::bot_install_command_task_0099a170`. Either a new switch such as
`--pilot-set-target <unit>:<target>`, or an `--order` token routed through the script-orders host
instead of the scene-command registry, would do it. Both are edits to files this packet does not
own, so this is a request to the Codex side rather than a change here.

Without it, no run of IJN01 or USN01 can exercise the dive-bomb state machine past `turndown`,
because neither mission's script ever calls `PilotSetTarget` on a bomb-carrying aircraft: IJN01
makes no `PilotSetTarget` call at all, and USN01's five all name `Mav1`..`Mav5`, which are torpedo
armed and take the torpedo class.

## The class gate and the turndown binding, landed

`src/game_hosts_units.cpp` now gates the arm on `dive_bomb_task_installed_for_class` and runs
`009C44F0` through `dive_bomb_turndown_tick_009c44f0` whenever the state is `turndown`. The class
reaches the unit from `GameScriptOrdersHost::run_pilot_set_target`, which stores it beside the
`0099A170` install through the new `GameUnitsHost::store_unit_attack_command_class`. The turndown
census prints the tick count, the roll and pitch write counts, the tick the `state+1Ch` latch fired
on, and the last bank, roll, pitch and commanded speed.

One labelled substitution in the binding: `007C47F0`'s two inputs are read and named, but this host
builds no plane class descriptor, so the product uses the tuning default `1.8` and the authored
default `StallSpd` `17.5` rather than the unit's own class row. `009C1850`'s move-to setter in the
same file still substitutes the row's `TravelSpeed` and labels `007C47F0` unread; now that the
routine is read, that site can take the real product too. Left alone here because it belongs to the
packet that wrote it.

### An intermittent startup crash, wrongly called a `main` regression

Two IJN01 runs at 13:27:01 and 13:27:37 exited `0xC0000005` with a 107-line log ending at
`Phase 5 online_manager_initialize`. This doc first recorded that as a regression on `main`.
**It was wrong**, and `docs/GAME_EXECUTABLE.md` carries the full table and the method errors.

The short of it: four builds and four 300-frame runs in a throwaway detached tree, at `4668b0e96`,
`82986e455`, `cccf31e11` and `main`'s tip `78af19721`, all exited `0` with `frames_presented=299`,
and two further clean runs in this worktree at its own HEAD confirm it. The 107-line signature is
the discriminator: the `cc8-ai-squadron` worker hit the identical one the same day on a tree
containing none of the suspects, and a retry cleared it both times. The two crashes here were 36
seconds apart, so under a rule that a step fails only on two consecutive crashes with a clean
environment between them, that step never failed.

### The identity run, taken

`local/ijn01_gate.log`, IJN01, 3000 mission frames at 0.05 s, with the class gate and the turndown
binding in. It was run to confirm identity, not to look for movement: the result was known in
advance from the order census, and the point is that the new gate costs nothing.

**Zero dive-bomb lines.** No `divebomb` per-aircraft row and no `summary mission dive-bomb task`,
because no IJN01 aircraft ever receives the divebomb class. The 27 installs the loose gate produced
are gone.

Every other summary is unchanged against both earlier IJN01 runs:

| measure | before | after (loose gate) | after (class gate) |
| --- | --- | --- | --- |
| `plane motion distance_moved` | 347370.56 m | 347370.56 m | 347370.56 m |
| `pilot attack ordered` | 33 | 33 | 33 |
| `pilot attack final_pitch_mean` | 0.027 rad | 0.027 rad | 0.027 rad |
| `torpedo task` | 6 aircraft, 0 releases | 6, 0 | 6, 0 |
| `gunnery ordnance general_bomb` | 27 | 27 | 27 |
| `fixed steps` | 3000 at 0.05 s | 3000 | 3000 |

## The attack-run tick `009C4220`, and the state walk it unlocked

Vtable `00D20C68` slot `+Ch`, Ghidra body `009C4220`-`009C447D`, `__thiscall(state, float dt)`,
`RET 4`. `EDI` is the state, `ESI` is `state+4h`, and `[ESI]` is the approach. It is the
`009D07B0`/`009A3770` shape `docs/BOT_TASK_STATES.md` tabulates, with dive-bomb constants.

| step | rule | address |
| --- | --- | --- |
| 1 | `dt < state+1Ch` keeps the countdown, `state+1Ch -= dt`; otherwise re-roll | `009C424A`-`009C4255`, `009C4333` |
| 2 | the re-roll adds rather than resets: `state+1Ch = (state+18h - dt) + state+1Ch` | `009C427B` |
| 3 | a new lateral offset from `007F0280(ctl, pose, ...)`, last argument **1** (the torpedo passes 0), negated and scaled by the double `0.5235988` at `00CEC730` | `009C42B8`-`009C42D9` |
| 4 | `cmd->+2C0h = AddWrappedAngle(approach->+C0h, state->+20h)`, `cmd->+2CCh = 2` | `009C42FF`, `009C4305` |
| 5 | `d = min(approach->+BCh, 2000.0)` | `009C4311`-`009C4342` |
| 6 | `m = max(1400.0 - altitude, 50.0)` | `009C435B`-`009C438B` |
| 7 | throttle `= InterpolateClamped(0.1, 0.4, 0.35, 1.0, m / d)` | `009C4397`-`009C43CD` |
| 8 | `009FBA50(approach->+ACh + approach->+50h, approach->+B4h, ., throttle)` | `009C43ED`-`009C4401` |
| 9 | `cmd->+278h = 1.0f`, `+27Ch = 1`, `+2A8h = 0.0f`, `+2ACh = 1`, `+2D8h = 0` | `009C4413`-`009C4434` |
| 10 | `approach->+1Ch->+40h = tuning+670h`, then `009A1A20(cmd, 0099B630(cmd))` and `009FABE0(approach->+1Ch, .)` | `009C443E`-`009C4471` |

Jump senses from the branch bytes: `009C424F` `0F 82` `JC`, `009C4329` `76` `JBE`, `009C4379` `76`
`JBE`.

### The run: the machine walks four states

`local/usn04_long.log`, USN04, 4800 mission frames. Environment before the run: two `bsp_game`
processes and the lock held by `cc8-ai-squadron`, so the launcher queued and ran.

| measure | value |
| --- | --- |
| arm ticks | 2370 |
| transitions | 3 |
| states | `attackrun` 1480, `flyabove` 144, `turndown` 746 |
| the latch `approach+D0h` | **closed at tick 1481** |
| attackrun re-rolls | 149 |
| commanded heading | 3.2247 rad |
| commanded throttle | 1.000 |
| altitude base | 1000.0 m |
| range, per sample | 11044 11031 11003 10964 10919 10871 10823 10771 10715 10657 10595 10531 10464 10396 10328 10259 |
| turndown ticks | 746, with **746 roll writes and 746 pitch writes** |
| bank reached | **0.1364 rad** |
| roll commanded | -0.8000 |
| releases | 0 |

So the run-in works. The aircraft closes from 11044 m, the latch arms at tick 1481, the entry
chooser takes `flyabove`, the roll-in fires, and `turndown` gets 746 live ticks with its roll and
pitch commands written every one of them. That is the whole chain this packet reconstructed,
running on live inputs for the first time.

### `009C7EA0`'s window is not met, and the reason is the think order

The turndown latch needs `|bank| > 150 degrees` at `009C4654`, and the bank reaches
**0.1364 rad, 7.8 degrees**, despite 746 roll commands of `-0.8`.

The roll never develops because **the planner's own roll arm overwrites the slot**. The think order
is: reset the plan, run the task arm, then `plan_yaw_0099d300`, then `pilot_plan_roll_0099e2ba`,
which writes `plan_slots[kPilotSlotRoll]` at the site labelled `0099E3AE`. The turndown writes the
same slot earlier in the same think, so its command is discarded before the slot is evaluated.

**That is the next gate, and it is not another unread tick.** It is a question about the image:

> `0099E2BA`'s roll arm is unconditional in this host. In the image it writes the same slot the
> turndown writes, so either it is gated on something the turndown clears, most likely the mode word
> `cmd->+2CCh` which the turndown sets to `0` while the planner's own path uses `2`, or the native
> turndown's roll is equally overwritten and the bank comes from somewhere else. Reading `0099E2BA`'s
> entry condition decides it, and nothing downstream can be trusted until it does.

Until then the dive-bomb chain is complete up to the roll-in and stops there.

### One labelled substitution in the binding

`007F0280` at `009C42B8` is a contract, so the host's run-in flies straight at the target rather
than weaving. Its three float arguments are the `80.0` at `00CE5444`, the `60.0` at `00CEB4B0` and
the `120.0` at `00D05804`, which the listing pushes; the body was not read.

## The roll-arm gate: a task's roll does survive, and its pitch was never at risk

`BSP_PilotBot_PlanControls` is one routine, `0099D300`-`0099EBAB`, so `0099E2BA` is a region inside
it rather than a function. Read whole from the planner's load of the mode word to the slot write.

### Roll, slot 2, mode `cmd+2CCh`: the gate exists

| address | instruction | effect |
| --- | --- | --- |
| `0099DDBE` | `MOV ECX,[ESI+2CCh]` | loads the mode word **the task wrote** |
| `0099DE8A` | `CMP ECX,2` | |
| `0099DE8D` | `JNZ 0099E26E` | anything but 2 jumps **past** the planner's own mode write |
| `0099E264` | `MOV [ESI+2CCh],1` | reached only on the mode-2 path |
| `0099E26E` | `CMP [ESI+2CCh],1` | on the jumped-to path this still holds the **task's** value |
| `0099E275` | `JNZ 0099E3BF` | skips the entire roll arm |
| `0099E39D` | `FSTP [ESI+290h]` | the planner's roll desired, reached only when the compare passes |
| `0099E3AE` | `MOV byte [ESI+294h],1` | its active byte |
| `0099E3B5` | `MOV [ESI+2CCh],0` | the mode is consumed |

`ECX` is unambiguous: filtering the whole range `0099DDBE`-`0099DE8D` for the register gives exactly
three lines, the load, a `TEST` and the `CMP ECX,2`, with no intervening write.

Two more jumps reach `0099E26E` the same way, `0099DE63` and `0099DE6F`, both early exits inside the
mode-2 region, so the compare at `0099E26E` is live on four paths.

**So the rule is:** `cmd+2CCh == 2`, a commanded heading, lets the planner compute and write the
bank. `== 1` also lets it through. **Anything else, including the `0` the turndown writes at
`009C462F`, skips the arm and leaves the task's `cmd+290h` standing.**

The native turndown's roll is therefore **not** overwritten. Mine was, because this host's roll arm
was unconditional.

### Pitch, slot 3, mode `cmd+2D0h`: there was never an overwrite to gate

The planner does not write the pitch slot in the arm region at all. An exhaustive census of
`+298h`, `+29Ch` and `+2A0h` across `0099D300`-`0099EBAB` finds writes only at `0099D36F`/`0099D377`
and `0099D679`/`0099D681`, both in the early reset, and the arm region only **reads** them, at
`0099E3ED`, `0099E3F8` and `0099E402`. The `0099E3BF` `TEST`/`JNZ` on `cmd+2D0h` selects between two
demand computations and both converge; it does not skip a write.

`0099E68D` is not a slot write either: it is `MOVSS XMM0,[ESP+44h]`, a stack read inside the pitch
computation.

So a task's pitch command already survived, and only the roll needed the gate.

### The host

`src/game_hosts_units.cpp` now applies the same condition around
`pilot_plan_roll_0099e2ba`'s slot write, and the turndown binding sets the mode word to `0` when it
commands the roll (`009C462F`) and to `1` when it hands the axis back (`009C464E`).

### The gated run, and what it moved

`local/usn04_gate.log`, USN04, 4800 mission frames, with the roll gate applied.

| measure | before the gate | with the gate |
| --- | --- | --- |
| bank reached | 0.1364 rad, 7.8 deg | **0.6072 rad, 34.8 deg** |
| turndown roll writes | 746 of 746 | 446 of 746 |
| final `approach+BCh` | 5077.7 m, opening | **206.5 m** |
| latch `approach+D0h` at the end | 0 | **1** |
| state walk | `attackrun` 1480, `flyabove` 144, `turndown` 746 | the same |
| releases | 0 | 0 |

The roll command now survives, the bank grows four and a half times, and the aircraft holds its
target instead of flying past it: `206.5 m` at the end against `5077.7 m` before.

`009C7EA0`'s window is still not met. It needs `pose+C64h` past `-1.3 rad` and the bank reaches
`0.6072 rad`, so the turndown does not end and nothing reaches `aimdive`.

### The hand-over, the last piece of the same gate

The 300 ticks where the turndown stopped writing the roll are the answer. `009C45BB` hands the axis
back once `|bank| >= 0.8 rad` (45.8 deg), and `009C4646`/`009C464E` write `cmd->+2C4h = pi` and
`cmd->+2CCh = 1`. Mode `1` **passes** the planner's compare at `0099E26E`, so the planner's roll
servo runs, and because the jump also skipped `0099E25C MOVSS [ESI+2C4h]` the servo drives toward
**the `pi` the turndown just wrote**.

That is the design: the turndown rolls to 45.8 degrees under its own command, then hands the
planner a bank target of 180 degrees and lets its servo carry the aircraft the rest of the way to
inverted, where the 150-degree latch at `009C4654` closes.

The host was missing both halves: it wrote the planner's own bank target unconditionally, and the
turndown binding set the mode without the target. Both are now gated at `0099E25C` and written at
the hand-over.

**The confirming run is BLOCKED, and not on anything in this repository.** The machine's
remote-desktop session is disconnected, so it has no audio endpoint, FMOD's output init fails and
the executable cannot reach a window at all; `docs/GAME_EXECUTABLE.md` carries the signature and the
evidence. The hand-over change is reasoned from the listing and committed, and it stays **unverified**
until a session is connected and one USN04 run can be taken.

### The hand-over run is blocked by a startup failure, twice, with a clean environment

```
EXITCODE=1
startup failed: FMOD bank raw-length output unavailable: path=sound/gui/error.fsb
  bytes=2688 mode=2634 create_result=78 length_result=37 bank_returned=0
summary window_created=0 device_created=0 device_hr=0x80004005 frames_presented=0
```

Two consecutive runs, `Get-Process bsp_game` empty and no lock file before the second, so under the
rule of `docs/GAME_EXECUTABLE.md`'s intermittent-crash section this is a **failing step**, not a
stray. It is a different signature from the 107-line renderer crash: the window is never created and
the failure is in the FMOD bank load, before anything this packet touches.

The last run that worked from this tree, `local/usn04_gate.log`, was on the pre-merge build. The
merge that followed brought `main` up several commits. **This packet does not name a culprit**: the
lesson from the earlier bisect is that a failure in one tree is not evidence about a commit until a
fresh tree at the suspect commit reproduces it. The measured result above stands on the run that
completed; the hand-over remains unverified.

## `009C62B0`, the flyabove tick: defined, bounded, and its flags censused

Ghidra had no function here. `tools/ghidra_define_function.py 009c62b0 009c7086` defined one over
**`009C62B0`-`009C7085` inclusive, 3542 bytes**, with `INT3` padding from `009C7086`. The end is
two exits sharing one epilogue: `RET 4` at `009C706C` and at `009C7083`, both after
`ADD ESP,88h`. It is the largest routine in the class, half again the aimdive tick.

`ESI` is the state and `EDI` is `&state->approach`.

### The four flags, with their writers

| flag | writer | rule |
| --- | --- | --- |
| `+18h` | `009C659F` | `0` |
| `+18h` | `009C680E` | `AL`, where `009C67EA`-`009C67F8` set `1` when `approach->+D4h` exceeds the frame value in `ST1` and `009C6808` clears it |
| `+19h` | `009C66E7` | `0` |
| `+19h` | `009C67B0` | `1`, reached by `009C67A3` `FCOMIP`/`JA` or by `009C67A9` `COMISS`/`JC` falling through |
| `+19h` | `009C6826` | `[ESP+43h]` |
| `+19h` | `009C6A30` | **copied from `+18h`**, gated on `[00CF180C] > cos(...) * [ESP+28h]` at `009C6A27` |
| `+1Ah` | `009C66E3` | `1` |
| `+1Ah` | `009C66F2`, `009C6822` | `0` |
| `+1Bh` | `009C6813` | `DL`, only when `+1Ah` is set. **A fourth flag** the transition rule does not read |

`009C6690` writes `approach->+CCh = 3`, the weapon selector, as already recorded.

So the shape is confirmed: `+18h` is the can-dive decision and it is a **range** test on
`approach->+D4h`; `+19h` is the roll-in permission and its main writer copies `+18h` once the
over-target geometry closes; `+1Ah` is the separate break-off request.

**`coverage: partial`.** The frame slots behind the two compares, `ST1` at `009C67F2` and
`[ESP+30h]` at `009C67A9`, were not traced to their producers, so the host substitution for these
three flags **stands** and is not yet replaced. Replacing it needs those two traces, which is a
packet rather than a tail: this routine is 3542 bytes and a CFG fixpoint over it is the same kind of
work the aim tick took.

### Why the other two substitutions are also not closed here

`approach+A8h` comes from `009C3ED8`'s `Random((approach+14h)->+38h, (approach+14h)->+3Ch)`, and
`006E3500` is the per-device round count. Both are small reads on their own, but the record at
`approach+14h` has no producer yet and the device list behind `006E3500` is the gunnery host's, so
each is a trace rather than a transcription. They are listed in "Follow-up packets" unchanged.

### `+18h` recovered: the can-dive flag is a height test

Traced. `[ESP+38h]`, the value `009C67F2` compares `approach->+D4h` against, has exactly one writer
before that read, `009C6493`, and it is built at `009C647D`-`009C6482`: the virtual call at
`009C647B` returns the target point, `FLD [EAX+4h]` takes its **y**, and `FSUBR qword [ESP+10h]`
subtracts it from the aircraft's own height. That is the same quantity the aimdive tick forms at
`009C59D6`.

`009C67F2 FCOMIP ST0,ST1` with `ST0` the height and `ST1` the range, and `009C67F6 JBE` taking the
`XOR EAX,EAX` arm, so:

```
flyabove->+18h = (heightAboveTarget > approach->+D4h)
```

**The aircraft may dive once it is higher above its target than the release range.** That is the
rule the host was standing in for with a planar-range test against `SafeDist`, and it is now
`dive_bomb_flyabove_can_dive_009c680e` in `src/dive_bomb_task.cpp`.

### `+19h` not recovered, and the reason is worth recording

`[ESP+30h]`, the slot `009C67A9` compares against zero to decide whether to set the roll-in
permission, **has no writer anywhere in this function before that read**. An exhaustive scan of the
defined body for the slot finds a read at `009C6647`, this compare at `009C67A9`, and writes only at
`009C6853`, `009C69A5` and `009C6C65`, all **after** it. No `LEA` of the slot is passed to any
callee either, so it is not an out-parameter this reading found.

So either a callee writes it through a pointer formed in a way this scan missed, or the slot is
genuinely uninitialised on the path that reaches `009C67A9`. Both are worth knowing and neither is
worth guessing, so the host's stand-in for `+19h` and `+1Ah` **stands**.

The host binding for `+18h` is not switched over yet either: `src/game_hosts_units.cpp` is leased to
`cc8-torpedo-run-in`. The pure rule is in place and the one-line swap is the next edit when the file
frees.

## `approach+A8h`: the rule is recovered, the record is not

`009C3F1C`-`009C3F2E`, inside the approach seed `009C3EA0`:

```
EAX = approach->+14h                      ; 009C3EFD
FLD [EAX+3Ch]  -> arg2                    ; 009C3F1C
FLD [EAX+38h]  -> arg1                    ; 009C3F23
CALL BSP_Random_UniformFloatRange, ECX=1  ; 009C3F29
FSTP [ESI+A8h]                            ; 009C3F2E
```

So **`approach->+A8h = Uniform((approach->+14h)->+38h, (approach->+14h)->+3Ch)`**, a per-aircraft
random dive floor drawn once at construction between two bounds carried by the record at
`approach+14h`. The next field is the contrast: `009C3F34`-`009C3F3F` takes `approach->+ACh`
straight from `tuning+4CCh`, `Pilot/DiveBomb/BeginAltRange/1`.

**The record is not identified.** `approach+14h` is read at `009C3EFD` with no writer in this
function: the head calls `0042E740` for `tuning+4D8h` `Pilot/DiveBomb/ReferenceSpeed` and hands it
to the base approach constructor `009F9CE0` at `009C3ED5`, so the field is written there. It is the
same record the aimdive interpolations read `+5Ch` and `+60h` from, so one trace into `009F9CE0`
would settle three unknowns at once.

Until then the host keeps its labelled substitution of `800` for `approach+A8h`, and the
`Follow-up packets` entry becomes specific: **trace `approach+14h` to its producer in `009F9CE0`**.

`006E3500`'s per-device round count was not reached in this packet and stays untouched.

## `+19h` recovered: the roll-in fires once the target is behind the wing line

The earlier reading said this slot had no writer. **That was the ESP-depth trap**, and
`tools/frame_slot_census.py` with the call cleanups supplied finds it at once: frame slot **K=104**
groups `[ESP+30h]` at `009C67A9` with `[ESP+44h]` at `009C65FD`, the same slot at two depths. A
naive grep for the literal offset cannot see that, which is exactly what
`docs/WORKER_VERIFICATION_CHECKLIST.md` means by tracing values rather than offsets.

The chain to the first and decisive condition:

| address | step |
| --- | --- |
| `009C673F`-`009C674F` | wrap the angle into `[0, 2pi)` by adding the `2pi` at `00CE3828` when it is negative |
| `009C6765` | `BSP_Math_SubtractWrappedAngle(other, that)`, giving the bearing error |
| `009C676A` | store it |
| `009C6774`-`009C678A` | fold it to its absolute value with the usual `-0.0f` subtract |
| `009C6796` | store the folded error |
| `009C67A3` | `FCOMIP` it against the **double `1.600000023841858`** at `00CE3D48` |
| `009C67A7` | `JA` sets `flyabove+19h = 1` |

So:

```
flyabove->+19h = 1   when |bearingError| > 1.6 rad  (91.7 degrees)
                or   when the clamped slot K=104 is not positive
```

**1.6 radians is 91.7 degrees: the target is behind the wing line.** That is precisely when a dive
bomber rolls in, and it explains the state's name: `flyabove` flies over the target and waits until
it has passed before committing.

The second arm is `009C67A9` `COMISS`/`009C67AE` `JC`, and the slot it tests is `max(x, 0)` from
`009C65E3`-`009C65FD`, so the arm is `x <= 0`. **`x`'s own producer is one level further back and is
not established**, so `dive_bomb_flyabove_roll_in_009c67b0` takes it as a caller-supplied input and
the host still substitutes for that half.

### The census run that found it

```
python tools/frame_slot_census.py 009c62b0 --pop 009c62cf=4 --pop 009c6342=4 --pop 009c63bf=0 \
  --pop 009c6404=0 --pop 009c647b=4 --pop 009c64ec=4 --pop 009c6705=4 --pop 009c6728=0 \
  --pop 009c6b3a=24
```

Without the cleanups the walker refuses after the first indirect call and the depths are not
comparable; the pops are the four-byte `RET 4` of each virtual call, zero for the two `00BF701A`
x87 helpers, and twenty-four for `007F0280`, which the attackrun tick calls with six pushes.

## `009F9CE0`: the record is a difficulty-level row, and four unknowns are named

`BSP_BotApproach_ConstructSpeedReference`, body `009F9CE0`-`009F9D77`,
`__thiscall(approach, unit, float referenceSpeed)`, `RET 8`. It seeds the whole approach head:

| field | source | address |
| --- | --- | --- |
| `approach+0h` | vtable `00D21C74` | `009F9CE4` |
| `approach+4h` | the **unit** | `009F9CEA` |
| `approach+8h` | `unit->+538h`, the **plane class descriptor** | `009F9CF3` |
| `approach+Ch` | `unit->+9D4h`, **`ctl`** | `009F9CFC` |
| `approach+10h` | `unit->+DF4h`, the bot object | `009F9D05` |
| `approach+14h` | `[00F8A30C] + (unit->+DF4h)->+34h * 248h + 0Ch` | `009F9D22` |
| `approach+18h`, `+1Ch`, `+20h` | `0` | `009F9D27`-`009F9D2D` |
| `approach+24h` | `max(classDesc->+188h MaxSpd / referenceSpeed, 1.0)` | `009F9D37`-`009F9D61` |
| `approach+28h` | `-1.0f` at `00D7A260` | `009F9D6E` |

Every one of those confirms a field this packet had been reading positionally: `+8h` really is the
class descriptor whose `+184h` is `StallSpd` and `+188h` `MaxSpd`, and `+Ch` really is `ctl`.

**`approach+24h` is `task+41Ch`** (`3F8h + 24h = 41Ch`), the speed ratio
`docs/BOT_TASKS.md` records the cruise profile multiplying `AttackDist` by. It is
`max(MaxSpd / Pilot/<class>/ReferenceSpeed, 1.0)`, and for the dive bomber the reference is
`tuning+4D8h` `Pilot/DiveBomb/ReferenceSpeed` KMH(280), which `009C3EC4` loads and passes in.

### The record, and the four fields

`docs/TORPEDO_RUN_PROFILE.md` already read this exact `LEA`: `approach+14h` is
`&PilotBotConfig.levels[(unit->+DF4h)->+34h]`, a `PilotBotParameters` row of `248h` bytes, and the
index is a **difficulty or skill level**, not a class. `include/bsp/robot_config.hpp` carries the
struct with names taken from the Lua keys, and its member suffixes are `PilotBotConfig`-relative,
so a row offset `N` is the member whose suffix is `N + 0Ch`.

That names all four fields this packet was substituting for:

| read | row offset | member |
| --- | --- | --- |
| `(approach+14h)->+38h`, the dive-floor low bound | `38h` | **`dive_bomb_release_alt_1_044`** |
| `(approach+14h)->+3Ch`, the dive-floor high bound | `3Ch` | **`dive_bomb_release_alt_2_048`** |
| `(approach+14h)->+5Ch`, the aimdive lead endpoint | `5Ch` | **`dive_bomb_aim_prec_dist_068`** |
| `(approach+14h)->+60h`, the aimdive gain endpoint | `60h` | **`dive_bomb_aim_prec_mul_06c`** |

So:

* **`approach+A8h`, the dive release floor, is `Uniform(DiveBombReleaseAlt1, DiveBombReleaseAlt2)`**,
  drawn once per aircraft from two **authored** altitudes that vary with the difficulty level. The
  host's substituted `800` can be replaced by the real pair the moment the config rows are loaded.
* **The aimdive aim error is the AI's authored aiming imprecision.** Its lead interpolates to
  `DiveBombAimPrecDist` and its gain to `DiveBombAimPrecMul`, both difficulty-scaled. That is why
  the release gate is a 25-metre window: the whole quantity is a deliberate miss distance, and a
  harder difficulty tightens it.

This corrects this doc's earlier phrasing. The dive floor is not "a per-aircraft random draw" in the
sense of being arbitrary: it is a draw between two authored, difficulty-scaled altitudes, and the
aim error is authored imprecision rather than a geometric residue.

## The hand-over run: no change, and the reason is a gap in the planner reconstruction

`local/usn04_hand.log`, USN04, 4800 mission frames. Preconditions recorded: `query session` shows
session 1 **Active** at the console, `Get-Process bsp_game` empty, no lock file. `EXITCODE=0`.

The census is **bit-identical** to the run before the hand-over:

| measure | gated run | hand-over run |
| --- | --- | --- |
| states | `attackrun` 1480, `flyabove` 144, `turndown` 746 | the same |
| bank reached | 0.6072 rad | **0.6072 rad** |
| turndown roll writes | 446 of 746 | **446 of 746** |
| latch tick | 1481 | 1481 |
| final `approach+BCh` | 206.5 m | 206.5 m |
| `009C7EA0` window | not met | **not met** |
| releases | 0 | 0 |

### Why: `pilot_plan_roll_0099e2ba` reconstructs only the mode-2 arm

The image's roll region has **two** entries, and the reconstruction has one.

* The **mode-2** path, `0099DE93`-`0099E25C`, computes the bank target from the heading error and
  writes it to `cmd+2C4h` at `0099E25C`. That is what `pilot_plan_roll_0099e2ba` models:
  `PilotBotRollInputs` has a heading error, a bank, a pitch error and the tuning, and **no bank-target
  input at all**.
* The **mode-1** path, jumped to at `0099E26E`, does not compute a target. It **servos** `cmd+2C8h`
  toward whatever `cmd+2C4h` already holds: `0099E27B FLD [ESI+2C8h]`, the wrap against the `+pi` at
  `00CE3D28` at `0099E28F`, then `0099E2A5 LEA ECX,[ESI+2C4h]` and `0099E2B5 CALL 00415690`, through
  to the roll write at `0099E39D`.

So writing `pi` into the host's `bank_target_2c4` on the hand-over changes nothing: the host still
runs the mode-2 computation, and the field it was told to aim at is one nothing reads. **The
reasoning behind the hand-over stands and the code that would act on it does not exist yet.**

**The next gate, by address:** reconstruct the servo arm `0099E26E`-`0099E3AE` in
`src/plane_ai_control.cpp`, with a bank-target input, and call it on the mode-1 path instead of the
mode-2 computation. Until then the turndown can hand the planner a 180-degree target and the host
will keep flying its own.

## `0099E26E`-`0099E39D`, the servo arm

The other entry to the roll region, reconstructed as `pilot_roll_servo_0099e26e` in
`src/plane_ai_control.cpp`. It computes no target and servos toward whatever `plan+2C4h` holds.

| address | step |
| --- | --- |
| `0099E2C5`-`0099E2CE` | `BSP_Math_SubtractWrappedAngle(plan+2C4h, measured bank)` |
| `0099E2D3` | scale by `[ESP+28h]` |
| `0099E2E1`-`0099E301` | fold to the absolute value with the usual `-0.0f` subtract |
| `0099E30B`-`0099E314` | inside the band `EBX+40h`, multiply by the gain `EBX+44h` |
| `0099E319`-`0099E337` | outside it, add or subtract the constant rate `EBX+48h` by the error's sign, and set `plan+2ECh` |
| `0099E344`-`0099E367` | the rate limit from `desc+1A8h`, `desc+1BCh` and `EBX[0]` |
| `0099E373`-`0099E390` | `InterpolateClamped(-limit, 1.0, +limit, -1.0, [ESP+2Ch])`, a **falling** map |
| `0099E39D` | `plan+290h`, the roll command |

Jump senses from the branch bytes: `0099E2EB` `76` `JBE`, `0099E312` `76` `JBE`, `0099E330` `76`
`JBE`.

**It is not called yet.** The call site is `plan_yaw_0099d300()` in `src/game_hosts_units.cpp`,
which `cc8-torpedo-run-in` holds until 07:57. Until that one line lands the host still runs the
mode-2 computation on both paths and the turndown's `pi` target goes nowhere.

`coverage: partial`: `[ESP+28h]`, `[ESP+2Ch]` and the `EBX` tuning block are taken as inputs rather
than traced, so the function is faithful in shape and sourced in its constants, not in its feeds.

## The authored values, quoted

From this installation's `scripts/datatables/robots.lua` (mtime 13 Jul 2024), the `SPNormal` row:

```
["DiveBombReleaseAlt"] = { 350, 450 }, -- M -- regi tipusu, leboritos bumbazasnal a bomba
                                       --      oldasi magassag valahol a ketto kozott
["DiveBombAimPrecDist"] = 70.0,        -- F -- tavolrol ennyivel melle celoz, aztan
                                       --      kozeledve egyre pontosabban
["DiveBombAimPrecMul"]  = 0.3,         -- F -- celzasi pontossag szorzo. minel kisebb,
                                       --      annal jobb
```

The release altitude is "somewhere between the two", which is the uniform draw at `009C3F29`. The
aim-precision distance is "from far away it aims this much beside the target, then gets more
accurate as it closes", and the multiplier is "the aiming accuracy scale, the smaller the better".
So the whole aim-error chain, and the 25-metre gate at `00CE3880`, is authored imprecision in the
authors' own words.

## The servo run, `local\usn04_fa.log` (17:19:38, 5000 frames / 4800 mission frames)

The run that carried the servo call on the mode-1 roll path. It moved the chain three states
further than the run before it, and it named the gate that stops it.

| measure | before (`usn04_dive.log`) | this run (`usn04_fa.log`) |
| --- | --- | --- |
| aircraft on the task | 1, `movieval` | 1, `movieval` |
| arm ticks | 2370 | 2370 |
| transitions | 2 | 3 |
| attackrun ticks (`009C4220`) | 1480 | 1480 |
| flyabove ticks (`009C62B0`) | 144 | 159 |
| turndown ticks (`009C44F0`) | 746 | 731 |
| in-range latch `approach+D0h` (`009C7C31`) | never | closed at arm tick 1481 |
| `|bank|` reached against the 150 deg latch at `009C4654` | 34.8 deg (0.6074 rad) | **179.8 deg (3.1381 rad)** |
| turndown latch `state+1Ch` | never | **set at arm tick 1673** |
| turndown roll writes / pitch writes | - | 15 / 731 |
| release floor drawn (`approach+A8h`) | 350.0 m | 350.0 m |
| `approach+B8h` (AttackDist floor, `009C8A5E`) | 1100.0 m | 1100.0 m |
| `approach+BCh` at the end | - | 4437.5 m |
| rounds | 2 | 2, none spent |
| `009C7EA0` window (`pose+C64h` under -1.3) | never met | **never met** |
| releases / bombs spawned | 0 / 0 | 0 / 0 |

The state walk this run, read off the tick counts and the two latch ticks (159 + 731 + 1480 = 2370):

```
attackrun  arm ticks    1 .. 1480   closes from 11050 m to roughly 3800 m
                                    latch approach+D0h closes at 1481
flyabove   arm ticks 1481 .. 1640   159 ticks, +19h fires
turndown   arm ticks 1641 .. 2370   731 ticks; bank latch at 1673, then stalled
```

`attackrun` heading 3.2240 rad, throttle 1.000, `alt_base` 1000.0 m, 149 rerolls; the range
samples fall 11050 -> 10265 over the first 160 arm ticks, about 98 m/s of closure. The gate line
reports 2021 arm ticks without the latch and 0 without bombs, so the latch held for 349 ticks and
then re-opened as the aircraft overflew: `approach+BCh` is back out at 4437.5 m at the summary.

### The new gate: the planner's pitch arm, `0099E3BF`-`0099E3D1`

The chain now stops inside `turndown`, at the transition `009C8620` guarded by `009C7EA0`. With
`|bank|` at 3.1381 rad the folded-bank arm of that test is already satisfied (3.1381 > the 2.356
at `00D20E80`), so the whole test reduces to `pose+C64h < -1.0`: the nose has to come down past
57 degrees. It never did, although the turndown wrote its full nose-down deflection on all 731
ticks (`pitch_writes=731`, `pitch=1.0000`).

It is **an unbound branch in the planner**, not an unread body and not a contract input. The
listing:

```
0099e3bf  MOV  ECX,dword ptr [ESI + 0x2d0]
0099e3c5  FLD  float ptr [ESP + 0x20]
0099e3c9  TEST ECX,ECX
0099e3cb  MOVSS dword ptr [ESP + 0x10],XMM2
0099e3d1  JNE  0x0099e490            ; the pitch arm runs only when +2D0h != 0
0099e3d7  ...                        ; mode 0: reads cmd+2ECh against 00E0E2F4,
0099e483  JMP  0x0099e756            ; writes only cmd+2ECh, then jumps PAST the arm
```

`plan_yaw_0099d300()` ran `pilot_pitch_demand_0099e490` unconditionally and overwrote the pitch
slot every think, so the turndown's deflection never reached the elevator. The pitch mode was
already on the plan state - `pilot_plan_slots.hpp` declares `pitch_mode_2d0`, reset to 2 at
`0099B54E` - but nothing read it and nothing but the reset wrote it.

This is the pitch twin of the roll gate on `+2CCh`. The turndown writes the zero that passes it,
in both of its pitch arms, with `EBP` zeroed at `009C44FB`:

```
009c469c  MOV dword ptr [EAX + 0x2d0],EBP    ; before the latch, beside +29Ch/+2A0h
009c472a  MOV dword ptr [ESI + 0x2d0],EBP    ; after the latch, beside +29Ch/+2A0h
```

Bound in this packet: the gate at `0099E3BF`, the two turndown writes, and the explicit `= 2` the
run-in's `009FB800` chain leaves behind. The mode-0 branch `0099E3D7`-`0099E483` is NOT modelled -
its only writes are `cmd+2ECh` from `00E0E2F4`/`00E0E2F0`, and this host keeps no `+2ECh`.

### The two servo contracts, re-traced

The packet named `[ESP+28h]`, `[ESP+2Ch]` and the `EBX` tuning block as the first suspects if the
bank stalled. The bank did not stall, and tracing them anyway retired one of the three.

`[ESP+2Ch]` at `0099E36B` is **not** an incoming slot. `0099E34A SUB ESP,0x14` sits between the
demand's store and that read:

```
0099e340  FSTP  float ptr [ESP + 0x18]   ; ESP = E0-8   -> frame slot E0+0x10
0099e34a  SUB   ESP,0x14                 ; ESP = E0-0x1C
0099e36b  FLD   float ptr [ESP + 0x2c]   ; ESP = E0-0x1C -> frame slot E0+0x10
```

Same slot. The fifth argument to `BSP_Math_InterpolateClamped` is the demand the arm just computed.
`[ESP+28h]` at `0099E2D3` is read before the adjustment (E0+0x20) and is still a contract, as is
the `EBX` block and the limit `0099E344`-`0099E367` builds from `desc+1A8h`, `desc+1BCh`, `EBX[0]`.

## Correction to "`0099E26E`-`0099E39D`, the servo arm": two readings, and the split

The servo arm is now two pure functions, `pilot_roll_bank_demand_0099e2ba` (`0099E2BA`-`0099E33A`)
and `pilot_roll_rate_limited_0099e344` (`0099E344`-`0099E39D`), so the host calls each once. The
image computes both in one region; the split is at the `0099E340` store, which is the region's own
hand-off from the demand to the map.

Splitting it turned up two errors in the single-function shape. They cancelled under the
substituted tuning, so **the mode-1 roll command this host produces is unchanged** and the
179.8-degree bank in `usn04_fa.log` stands.

| was | is | evidence |
| --- | --- | --- |
| `interpolant` is a contract input, "[ESP+2Ch] at `0099E36B`" | it is the demand `0099E340` stores; the host no longer calls the arm twice to feed it | `0099E34A SUB ESP,0x14` between the two references makes `[ESP+18h]` and `[ESP+2Ch]` the same frame slot |
| the demand arms act on the folded magnitude | they act on the SIGNED scaled error; the fold feeds only the band compare and the sign test | `0099E2E7 FCOMI ST0,ST1` then `0099E2E9 FSTP ST1` leave the signed error on the stack; `0099E310 FSTP ST0` discards the folded value before `0099E314`/`0099E332`/`0099E337` |

Why they cancel: with the substituted tuning (`band_40` 0, `gain_44` 1, `rate_48` 0) the band
compare always takes the rate arm, and the rate arm with a zero rate returns the signed error
unchanged - which is exactly what the host was passing as the interpolant. The two errors only
diverge once a real `EBX` tuning block is recovered, and then the corrected shape is the one that
holds: the demand keeps the error's sign, so the falling map still rolls both ways.

One side effect is now reported and not modelled: `0099E328 MOVSS [ESI+2ECh],XMM1` with
`00E0E2F4`, on the rate arm. `cmd+2ECh` is the same word the mode-0 pitch branch at `0099E3D7`
reads and writes; this host keeps no `+2ECh`.

## `approach+D4h` recovered: the dive-entry height, set once by the constructor

`009C3EA0` is the approach's constructor - it writes the vtable `00D20C48` at `009C3EE2`, draws
`+A8h` at `009C3F2E` and `+ACh` at `009C3F3F`, and tail-calls `009C3DA0` at `009C4083`. Its last
act before that tail call is `+D4h`:

```
009c3ffb  FLD   float ptr [ESI + 0xa8]         ; the drawn release altitude
009c4001  FSTP  float ptr [ESP + 0x20]
009c4005  FLD   float ptr [ESI + 0xac]         ; the begin altitude
009c400b  FLD   float ptr [ESP + 0x20]
009c400f  FLD   ST0
009c4011  FADDP ST2,ST0                        ; ST1 = +ACh + +A8h
009c4013  FXCH                                 ; ST0 = the sum, ST1 = +A8h
009c4015  FMUL  double ptr [0x00d7a280]        ; 0.5
009c401b  FSTP  float ptr [ESP + 0x24]         ; S24 = (+ACh + +A8h) * 0.5
009c401f  FADD  double ptr [0x00cf8850]        ; 250.0, on the +A8h copy
009c4025  FSTP  float ptr [ESP + 0x20]         ; S20 = +A8h + 250.0
009c4031  FCOMIP ST0,ST1
009c4035  JBE   0x009c403f                     ; byte `76`
009c4045  MOVSS dword ptr [ESI + 0xd4],XMM0
```

`+D4h = max(+A8h + 250.0, (+ACh + +A8h) * 0.5)`. `ESI` is `this` from `009C3EB8 MOV ESI,ECX` and
is never reassigned in the body, so both reads are the approach's own fields; the only `LEA` off it,
`009C3EDF LEA ECX,[ESI+30h]`, lands in `ECX`.

With this installation's `SPNormal` draw (`+A8h` 350.0) and the begin altitude 1000.0 that is
**675.0 m**: `max(600.0, 675.0)`. So `009C680E`'s can-dive test is a real height gate - the
aircraft must be 675 m above its target before `flyabove` will roll it in - and not the "above the
target" the zero reduced it to. Bound in `game_hosts_units.cpp` through
`dive_bomb_dive_entry_height_009c4045`.

### `approach+50h` is still open, and the obvious candidate is not it

A `+50h` store census over `.text` turns up eight stores in the `009Cxxxx` range. The one that
looks like the approach, `009C3E2E MOVSS [ESI+50h],XMM0` in `009C3DA0`, is **not** `approach+50h`:

```
009c3dad  MOV EDI,ECX          ; EDI is `this`, the approach
009c3daf  MOV ESI,[EDI + 0x14]
009c3e11  LEA ESI,[EDI + 0x30] ; ESI is rebased 0x30 past `this`
009c3e2e  MOVSS [ESI + 0x50],XMM0
```

That store lands on `approach+80h`, along with `+7Ch` and `+78h` from the same value
(`[[EDI+14h]+58h]`). No writer through the approach base exists in the dive-bomb range, so `+50h`
keeps its labelled zero, and the next reader should look for a sub-object base or a block copy
rather than repeating the offset census.

## The run after the pitch gate, `local\usn04_pitchgate.log` and `local\usn04_d4h.log`

Same arguments as `usn04_fa.log`, so the three runs compare directly.

| measure | `usn04_fa` | `usn04_pitchgate` | `usn04_d4h` |
| --- | --- | --- | --- |
| transitions | 3 | 4 | 4 |
| attackrun ticks | 1480 | 1480 | 1480 |
| flyabove ticks | 159 | 159 | 159 |
| turndown ticks | 731 | **67** | 67 |
| aimdive ticks | 0 | **664** | 664 |
| `|bank|` last | 3.1381 | 3.1261 | 3.1261 |
| turndown latch tick | 1673 | 1673 | 1673 |
| dive entry altitude | - | 638.9 m | 638.9 m |
| releases / bombs | 0 / 0 | 0 / 0 | 0 / 0 |

Binding the pitch gate is what moved it: the turndown now completes in 67 ticks instead of
stalling for 731, and the chain reaches `aimdive`, a state it had never entered.

`pose_c64_min=-0.9594` is the last sample taken while `turndown` still owned the tick; the arm
runs every 0.09 s while the pose refreshes every frame, so the angle crossed the -1.0 the
`009C7EA0` window needs between that sample and the next arm call.

Binding `approach+D4h` at 675.0 m changed nothing in the walk, which is the expected result: the
aircraft is above `alt_base` 1000.0 m when `flyabove` makes the can-dive decision and only falls to
638.9 m by the time `aimdive` takes over. The gate is now real rather than vacuous, and it will
bite on a mission that orders a lower approach.

### The gate now: the 25 m aim window at `00CE3880`

The release is blocked by the aim error, not by a state gate. The census added for this run keeps
the closest the error came to the window while an aim state owned the tick:

```
aim error 009C5C9B=-3706.33 m (gate 00CE3880 = 25.0 m) closest=359.50 m at range=444.2 m alt=631.2 m
```

359.50 m against a 25.0 m window, at 444 m of planar range. The last sample, -3706.33 m, is taken
after the overfly and says nothing about the dive. The error's open inputs are `approach+50h`
(above) and the two interpolation endpoints `(approach+14h)->+5Ch/+60h`, which the host substitutes
from this installation's `robots.lua` row (`DiveBombAimPrecDist` 70.0, `DiveBombAimPrecMul` 0.3).
An authored miss of 70 x 0.3 = 21 m at close range is inside the window, so a 359 m error means an
input or a term of `009C5C9B` is wrong, not that the aircraft aimed badly. That is the next packet.

### `flyabove+19h`, second arm: the reaching writer, and how far back it goes

The second arm at `009C67A9` is `COMISS XMM0,[ESP+30h]` with `XMM0` zeroed at `009C67A0`, and
`009C67AE JC` skips the flag when the slot is positive - so the arm fires on `slot <= 0`.

A literal-offset grep would not find its producer: the flyabove tick has nine call sites the frame
walker cannot account for. `tools/frame_slot_census.py 009c62b0` with the cleanups

```
--pop 009c62cf=4 --pop 009c6342=4 --pop 009c63bf=0 --pop 009c6404=0 --pop 009c647b=4
--pop 009c64ec=4 --pop 009c6705=4 --pop 009c6728=0 --pop 009c6b3a=12
```

(4 for each `CALL EDX`/`CALL EAX` virtual with one pushed argument - the walker's "156 bytes" at
`009C62CF` is the `SUB ESP,0x88` and four register pushes, not a cleanup - and 0 for the `00BF701A`
x87 helpers) puts the read in **frame slot K=104** with the reaching write at `009C65FD`, which
carries the literal offset `[ESP+44h]` because `009C65FA SUB ESP,0x14` sits between them.

That write is the `max(x, 0)` the earlier note assumed:

```
009c65c1  FLD   [ESP+0x10]                 ; x0
009c65c5  FMUL  double ptr [0x00ceffa0]    ; 0.7
009c65cb  FADD  double ptr [0x00ce4d70]    ; 200.0
009c65d1  FSTP  [ESP+0x10]                 ; S = x0 * 0.7 + 200.0
009c65d5  FLD   [ESP+0x10]                 ; T is already in ST0 from further back
009c65db  FSUBP ST2,ST0                    ; T - S
009c65e9  FCOMIP ST0,ST1                   ; against zero
009c65ed  JBE   0x009c65f4                 ; byte `76`
009c65fd  MOVSS [ESP+0x44],XMM0            ; max(T - S, 0)
```

`x0` itself is chosen at `009C65BB` between the constant at `00CE3D08` and `[ESP+38h]`, by the
compare at `009C659B` against the 100.0 at `00D7A220`. `T` is still on the x87 stack from further
back. So `+19h` stays a labelled substitution, but the trace is now two levels deeper and the next
reader starts at `009C659B` with the slot key and the cleanups above rather than repeating them.

## `flyabove+1Ah`: the leave rule, and what it shares with `+19h`

`+1Ah` is written at `009C66E3` (set) and `009C66F2` / `009C6822` (clear). The set is one compare:

```
009c66d5  FLD    float ptr [ESP + 0x10]    ; a, the bearing tolerance
009c66d9  FLD    float ptr [ESP + 0x2c]    ; b, the folded bearing error
009c66dd  FCOMIP ST0,ST1
009c66e1  JBE    0x009c66f0                ; byte `76`
009c66e3  MOV    byte ptr [ESI + 0x1a],0x1 ; leave, and
009c66e7  MOV    byte ptr [ESI + 0x19],0x0 ; clear the roll-in in the same breath
```

So the aircraft leaves `flyabove` when `b > a`. Both operands were resolved with
`tools/frame_slot_census.py 009c62b0` and the cleanups recorded above, not by literal offset.

**`b`, slot K=108, written once at `009C6453`**: the `-0.0f` fold (`009C642F JBE`) of the
`00438B10` wrapped-angle result from the call at `009C641C`. A bearing error.

**`a`, written at `009C6643`**: the return of `BSP_Math_InterpolateClamped` at `009C663E`, whose
five arguments the window opened by `009C65FA SUB ESP,0x14` fills:

| arg | site | value |
| --- | --- | --- |
| x0 | `009C6639` | 0.0 |
| y0 | `009C662F` | `00CE398C` = 0.34906587 rad, **20 degrees** |
| x1 | `009C662B` | `W` |
| y1 | `009C660B` | `00D7A264` = pi, **180 degrees** |
| x | `009C6603`/`009C6607` | **`max(T - S, 0)`** |

`W = approach+B4h * 0.8 (00CE3D40) - S`, from `009C6615 FLD [EBP+0xB4]`, `009C661B FMUL` and the
`009C6621 FSUBRP`. `EBP` is the approach: the last write before it is `009C655A MOV EBP,[EDI]`,
and `009C6562 FMUL [EBP+0xA8]` reads the same `+A8h` the constructor draws.

The tolerance therefore opens from 20 degrees at `x = 0` to 180 degrees at `x = W`. At 180 degrees
no folded error can exceed it, so **the leave only fires while `x` is small** - the aircraft gives
up the roll-in when it is close in and the target has swung more than about 20 degrees off.

### The two flags share one quantity

`x` at `009C6603` is the **same frame slot K=104** the `+19h` second arm reads at `009C67A9`, and
the same write at `009C65FD` reaches both. So `+19h`'s `max(x, 0) <= 0` and `+1Ah`'s interpolation
are two readings of one value, and closing it closes both flags at once. The `+19h` arm's jump
sense is confirmed: `009C67AE` is the byte `72`, JC, so a positive slot skips the flag.

### What is left, to the instruction

`S = x0 * 0.7 (00CEFFA0) + 200.0 (00CE4D70)` with `x0` the selection at `009C65BB`, which the slot
census confirms is the write that reaches `009C65C1`:

```
009c658d  FLD    double ptr [0x00d7a220]   ; 100.0
009c659b  FCOMIP ST0,ST1
009c65a9  JBE    0x009c65b5
009c65ab  MOVSS  XMM0,dword ptr [0x00ce3d08]  ; 100.0
009c65b5  MOVSS  XMM0,dword ptr [ESP + 0x38]
009c65bb  MOVSS  dword ptr [ESP + 0x10],XMM0
```

Both arms of that select are a 100.0 floor on `[ESP+38h]` against whatever `ST1` holds. Note that
`009C6568` writes the same slot earlier with `(approach+14h)->+40h * approach+A8h`, and that value
is **not** `x0`: `009C65BB` overwrites it, and the product only feeds the compare at `009C6578`
against the 1.1 at `00CE3DF0`.

`T` and the `ST1` at `009C659B` both arrive on the x87 stack from the branchy merge at `009C6532`,
where two arms pop a value (`009C6528`, `009C652E`) and one does not. That merge is where the next
reader starts; everything between it and `009C66E3` is now named.

## `006E3500`, the per-device round count: the body, and why the accessor is a separate packet

The body is nine instructions and is now read:

```
006e3500  PUSH ESI
006e3501  MOV  ESI,ECX                       ; ECX is the device; no stacked argument
006e3503  MOV  EAX,dword ptr [ESI]
006e3505  MOV  EDX,dword ptr [EAX + 0x21c]
006e350b  PUSH 0x2a                          ; the ordnance kind 2Ah
006e350d  CALL EDX                           ; callee-clean, no ADD ESP follows
006e350f  ADD  EAX,dword ptr [ESI + 0x484]
006e3515  POP  ESI
006e3516  RET                                ; RET 0, the count in EAX
```

`006E3500(device) = device->vtable[+21Ch](2Ah) + device->+484h`. `2Ah` is the ordnance kind
`docs/ORDNANCE_KIND_IDENTITY.md` already tracks - the one `007B9320` requires and `006EA2A0`
accepts alongside `29h`. So the count is "how many of kind 2Ah this device holds" plus a second
term at `+484h`, which `007C1DB0` then sums over every class-25h device of the unit.

### Why this packet stops here

Routing it through the gunnery host's process-wide accessor pattern - the one
`include/bsp/game_hosts_ai.hpp` sets out for `GameAiWeaponFacts`, where the owning host publishes
a table and the reading host holds no pointer to it - needs a producer that does not exist yet.
`src/game_hosts_gunnery.cpp` models an ordnance **mask** per unit (`gun.ordnance`, published by
`store_unit_ordnance` at its load pass) and nothing per device: no magazine count, no `+484h`
equivalent, and no class-25h device rows to hang them on. The accessor would therefore be the
second half of a packet whose first half is "give the gunnery host a per-device round count", and
that half edits `src/game_hosts_gunnery.cpp`, which this stream does not own.

It is also not on the critical path: the run census reads `rounds=2 rounds_left=2`, the salvo cap
is never reached because no release is issued, and the gate is the 25 m aim window. The substituted
two rounds stay labelled where they are, and the shape the accessor should take is recorded above
so the packet that owns the gunnery host can take it whole.

## The 340 m: a missing steering command, and `009C58D0`'s aim

The aim census said the release was blocked by a 359.50 m miss against a 25.0 m window. It is not
the aim error's arithmetic and it is not a unit error. **Nothing was steering the aircraft.**

`tick_state` was an empty override at both dive-bomb binding sites, so across 664 live aimdive
ticks the task issued no roll and no pitch. With no task command the pitch mode stayed at the 2 the
reset leaves at `0099B54E`, the planner's own arm at `0099E490` levelled the aircraft, and it flew
straight past its target.

### The arithmetic, which clears the aim error

At the closest sample - `|error|` 359.50 m, range 444.2 m, altitude 631.2 m - the reconstructed
model gives `x0 = 350 + 100 = 450`, `x1 = 1000 + 0 = 1000`, `t = (631.2 - 450) / 550 = 0.3295`,
`lead = 23.06`, `gain = 0.7694`. Solving `gain * (cos(bearing) * 444.2 - 23.06) = ±359.50`:

| branch | required `cos(bearing)` | verdict |
| --- | --- | --- |
| `+359.50` | 1.1038 | impossible |
| `-359.50` | -0.99998 | the target dead astern |

So at the moment the error came closest to the window, the aircraft had already passed the target
and the along-track term was running negative. The closest planar range in the whole dive was
444.2 m: that is the miss distance, not an aiming error.

### `approach->vtable[0]`, the lead's first suspect, is clean

Slot 0 of the vtable `00D20C48` that the constructor writes at `009C3EE2` is `009C40A0`, which
Ghidra has no function for:

```
009c40a0  MOV  EAX,dword ptr [ESP + 4]
009c40a4  FLD  dword ptr [ECX + 0x4c]   / FSTP [EAX]
009c40a9  FLD  dword ptr [ECX + 0x50]   / FSTP [EAX + 4]
009c40af  FLD  dword ptr [ECX + 0x54]   / FSTP [EAX + 8]
009c40b5  RET  4
```

It returns the approach's own aim point, not a moveto row. The host draws its bearing and planar
distance from the commanded target the same way `009C7B4F`-`009C7BB0` does, so the inputs agree
stage for stage.

### Correction: `approach+50h` is not a range

It is **component 1 of that aim point** - its vertical component. The aimdive tick takes `out[0]`
and `out[2]` as the horizontal pair for its `atan2` and its planar distance, and subtracts `out[1]`
from the aircraft's Y to get the height above the aim point. That also makes the aim error's
`x1 = approach+ACh + approach+50h` coherent: altitude plus altitude, not altitude plus range.

`009C8D40` writes all three offsets - `009C8D9B`, `009C8F27`, `009C8F2E`, with `009C8D45 MOV
ESI,ECX` making `ESI` the approach - and is reached from `009C91A0`, `009C91B0` and `009C9FB0`, not
from the arm. Whether it is the only writer is not established. The host's 0.0 substitution stays
numerically right for a sea-level target, so this is a naming correction, not a numeric one: the
field `db_extra_range_50` and the input `extra_range_50` are misnamed.

### `009C5C9F`-`009C5DB2`, the steering, now bound

```
009c5c9f  FLDZ                              ; the sign of the aim error
009c5ca5  FCOMI ST0,ST1
009c5ca9  JBE   0x009c5cd0                  ; byte `76`
009c5cab  FMUL  float ptr [EBP + 0x64]      ; positive arm, clamped at +1 (009C5CBC)
009c5cd0  FMUL  float ptr [EBP + 0x68]      ; negative arm, clamped at -1 (009C5CE5)
009c5cfa  MOVSS dword ptr [EDI + 0x29c],XMM0
009c5d15  MOV   byte ptr [EDI + 0x2a0],0x1
009c5d1c  MOV   dword ptr [EDI + 0x2d0],EBX ; EBX = 0 from 009C58DE
...
009c5d8e  CALL  BSP_Math_InterpolateClamped ; (-0.4, 1.0, 0.4, -1.0, bearing error)
009c5da3  MOVSS dword ptr [EAX + 0x290],XMM0
009c5dab  MOV   byte ptr [EAX + 0x294],0x1
009c5db2  MOV   dword ptr [EAX + 0x2cc],EBX
```

Both writes carry the mode that survives the planner, and that is the whole point of the pair:
`cmd+2D0h = 0` is the value the pitch gate at `0099E3BF` lets through, and `cmd+2CCh = 0` is
neither 2 nor 1, so the roll arm at `0099E26E` is skipped and the task's `+290h` reaches the stick.
The gate bound in the previous packet is what makes this tick effective.

`EBP` is `(approach+14h)`: `009C5CAB` and `009C5CD0` read `+64h` and `+68h` of the **same
difficulty-row record** whose `+5Ch` and `+60h` the aim error already uses. Neither has a producer
read, so both are substituted at 1.0 and labelled; at that gain the clamp bites on any error past a
metre and the pitch is bang-bang on the sign of the aim error rather than proportional to it.

The wide roll arm `009C5D24`-`009C5D31` is named, not bound: it swaps in a second bearing error and
the 0.5 band at `00CE3800` when the aim error is positive and a folded angle is inside the 60
degrees at `00D05AAC`. The folded angle's frame slot has no writer at its corrected key in
`tools/frame_slot_census.py 009c58d0` with the cleanups `--pop 009c594c=4 --pop 009c5988=4 --pop
009c59cd=4 --pop 009c5a66=0 --pop 009c5ab4=0 --pop 009c5df1=4 --pop 009c5ede=0`.

Jump senses, all from the bytes: `009C5CA9`, `009C5CBC`, `009C5CE5`, `009C5D22` and `009C5D31` are
each `76`, JBE.

### The two pitch gains recovered, and the record identified for certain

`009F9CE0` is what sets `approach+14h`, and it settles what the record is:

```
009f9d08  MOV  EDX,dword ptr [EAX + 0xdf4]
009f9d0e  MOV  EDX,dword ptr [EDX + 0x34]      ; the difficulty index
009f9d11  IMUL EDX,EDX,0x248                   ; the row stride
009f9d18  MOV  ESI,dword ptr [0x00f8a30c]      ; the table base
009f9d1e  LEA  EDX,[EDX + ESI*0x1 + 0xc]
009f9d22  MOV  dword ptr [ECX + 0x14],EDX      ; approach+14h
```

`approach+14h` is a 0x248-stride robots row viewed `0xCh` in. So the four fields the dive-bomb
chain reads off it map to row offsets exactly:

| read | row offset | `include/bsp/robot_config.hpp` | this installation's `SPNormal` |
| --- | --- | --- | --- |
| `->+5Ch` | `+68h` | `dive_bomb_aim_prec_dist_068` | `DiveBombAimPrecDist` 70.0 |
| `->+60h` | `+6Ch` | `dive_bomb_aim_prec_mul_06c` | `DiveBombAimPrecMul` 0.3 |
| `->+64h` | `+70h` | `dive_bomb_aim_prec_pull_plus_070` | `DiveBombAimPrecPullPlus` **0.018** |
| `->+68h` | `+74h` | `dive_bomb_aim_prec_pull_minus_074` | `DiveBombAimPrecPullMinus` **0.025** |

The first two were a substitution picked by name; the `0xCh` offset makes them a proof. The last
two are the aimdive tick's pitch gains, and the row's own Hungarian comments name them for what
they are: "tavolsagtol fuggoen mennyire huzza a pitch-t, ha nem pontos a celzas", how much it pulls
the pitch when the aim is not precise, and its push twin.

So the pitch is proportional, not bang-bang: `clamp(error * 0.018, -1, +1)` on the positive side
and `clamp(error * 0.025, -1, +1)` on the negative. It saturates past about 55 m of error and eases
off inside that, which is the behaviour a 25 m release window needs.

### A second name to watch: `approach+D8h`/`+DCh`/`+E0h`

`include/bsp/dive_bomb_task.hpp` called these `kAimPointX/Y/Z`, "the computed lead point". They
are not the aim point - `009C40A0` hands that out from `+4Ch`/`+50h`/`+54h`. The constructor fills
`+D8h`..`+E0h` at `009C4065`, `009C4071` and `009C407D` from `EDI+FCh/+100h/+104h`, `EDI` being its
stacked argument from `009C3ECA`, and the aimdive tick subtracts `+D8h` and `+E0h` from the aim
point before its first `atan2`. That reads as a latched REFERENCE position the aim point is
measured against, not a lead. Which entity `EDI` is has not been established, so the constants keep
their names with a PROVISIONAL note rather than being renamed on a guess.

The `+50h` constant is renamed, because that one is settled: `kExtraRange` is now `kAimPointHeight`,
with `kAimPointEast` (`+4Ch`) and `kAimPointNorth` (`+54h`) beside it, and the field and input
`db_extra_range_50` / `extra_range_50` are `db_aim_point_height_50` / `aim_point_height_50`.

### No fall time, and no ballistic lead

The packet asked whether the bomb's fall time appears in the miss-distance term. It does not.
`009C5C9B`'s whole chain is `gain * (cos(bearing) * planar_distance - lead)`, where `lead` is
`InterpolateClamped(+A8h + 100, 0, +ACh + +50h, ->+5Ch, height)` - an **authored imprecision**
interpolated over height, `DiveBombAimPrecDist`, whose robots.lua comment says in so many words
"from far away it aims this much beside the target, then gets more accurate as it closes". There is
no gravity term, no time of flight and no target velocity anywhere in `009C58D0`-`009C6161`: the
aircraft's own velocity does not enter the aim error either. The only ballistic-looking work is in
the aim-point updater `009C8D40`, which is a separate object's job and is not bound here.

## The aim run `local\usn04_aim.log`: inconclusive, and why

Same arguments again. **The steering was not exercised.** The state walk never entered `aimdive`:

| measure | `usn04_d4h` | `usn04_aim` |
| --- | --- | --- |
| arm ticks | 2370 | 2370 |
| transitions | 4 | 3 |
| attackrun | 1480 | **240** |
| flyabove | 159 | **1** |
| turndown | 67 | **0** |
| aimdive | 664 | **0** |
| aimglide | 0 | **2129** |
| latch closed at | tick 1481 | tick 241 |
| `approach+BCh` at the end | 4631.1 m | **0.0 m** |

So this run says nothing about `009C5C9F`-`009C5DB2`, in either direction. What it does show is a
failure upstream of it.

### The geometry collapses, and 0.0 m is the tell

`approach+BCh` is not "small", it is **exactly** 0.0. The producer only writes that through its
epsilon branch, `d2 <= 1e-10` at `00CE3820`: the aircraft's position and its target's are the same
point. An aircraft passing over a ship gives a small non-zero distance, never that.

The timing says the same thing. The run-in's range samples are identical to the earlier runs for
their whole span - 11050 m down to 10265 m over the first 160 arm ticks - and then the latch closes
at tick 241, which needs the planar range under `approach+B8h` = 1100 m. That is at least 9165 m in
81 arm ticks, 7.3 s, or 1250 m/s, against a commanded speed of 34.5 m/s. Nothing flew that. One of
the two endpoints was re-resolved.

### The mechanism, and where it lives

`GameGunneryHost::Impl::refresh_command_targets` in `src/game_hosts_gunnery.cpp` resolves one
target per unit from the command rows:

```cpp
for (const GameCommandRow& command : command_rows) {
    if (!command.current || command.target_token.empty()) continue;
    ...
    command_target_by_unit[command.unit_index] = found->second;   // last writer wins
}
```

A unit with several current rows keeps whichever comes last in the vector, and the census in this
run shows `movieval` carrying **three** current rows - `stop` (director idle tail), `moveto`
(ai_command_tick) and `divebomb` (script:PilotSetTarget). The whole map is rebuilt whenever
`command_rows.size()` changes, so a row appearing mid-mission can take the dive bomber's target
away from it. A token that resolves back to `movieval` gives exactly the observed 0.0.

This is the same class as this stream's first bug, a target point resolved to the wrong row, and it
is not in the dive-bomb chain. **The fix location is `refresh_command_targets` in
`src/game_hosts_gunnery.cpp`**, which this stream does not own, so it is named and not touched.

### What changed between the runs, stated as a window and not as a commit

`usn04_d4h` ran at `6b8907c3f`, before this packet merged `main`; `usn04_aim` ran at `f5b279eb5`,
after. The merge brought `src/game_hosts_script_orders.cpp` (new, +187), `src/game_hosts_lua.cpp`
(+117), `src/air_operations.cpp` (+228) and `src/game_hosts_scene_contents.cpp` (+6) - the files
that add and drive command rows. That is the window. No commit is named here: a repro on a fresh
detached tree at the suspect commit has not been run, and the run lock is currently arbitrated to
another worker, so it could not be.

## `T` closed: both flyabove flags are one height test

> **WITHDRAWN in part, packet `cc8_dive_heading`.** `T`, the x87 value arriving at the merge
> `009C6532` and consumed by `009C65DB`'s `FSUBP ST(2)`, is **not** the height `B`. It is `R`, the
> planar range, pushed at `009C64EE` and still on the stack after `009C659D`'s `FSTP ST(0)` has
> discarded `B`. The span is `max(R - S, 0)`, the two flags are therefore **not** "one height test",
> and the `B <= 666.7 m` below - including the "same gate expressed twice" at 666.7 against
> `approach+D4h`'s 675.0 - is an artefact of the swapped minuend rather than a number in the image.
> The threshold `S = 0.7 * max(B, 100) + 200` is unaffected and still comes from the height. The
> forward stack walk with every push and pop accounted is in "Packet `cc8_dive_heading`" below; so is
> the measurement of the artefact in `local\heading_before.log`.

The operand the last two packets left open - `T`, the x87 value arriving at the merge `009C6532` -
is resolved, and with it `+19h`'s second arm and `+1Ah` together.

### The merge is stack-balanced, which is why `T` survives it

Three arms reach `009C6532`. After `009C64EE`, `009C64F4` and `009C64F8` the stack is
`{C, B, A}` from `[ESP+3Ch]`, `[ESP+38h]` and `[ESP+28h]`:

| path | branch | pops | stack at `009C6532` |
| --- | --- | --- | --- |
| kind test true | `009C64FC` `75` JNZ to `009C6530` | none | `{C, B, A}` |
| `+D4h > C` | `009C6510` `77` JA to `009C652E` | `FSTP ST0` | `{C, B, A}` |
| `+B4h <= A` | `009C651A` `76` JBE to `009C6528` | `FSTP ST0` | `{C, B, A}` |
| otherwise | `009C6522` `72` JC | `FSTP ST2` at `009C6520` | `{C, B, A}` |

Every arm balances. Then `009C656C`-`009C657A` pushes the product
`(approach+14h)->+40h * approach+A8h`, multiplies by the 1.1 at `00CE3DF0`, compares and pops
twice (`009C657C` `76` JBE), leaving `{B, A}`. So the value that `009C65D5`'s `FSUBP` subtracts `S`
from - the `T` of the earlier note - is **`B`, the float at `[ESP+38h]`**, not `C`.

### `B` is the height above the aim point

Frame slot K=96 has exactly one writer, `009C6493`, and `B` is its only product:

```
009c646d  FSTP  double ptr [ESP + 0x10]   ; the aircraft's Y, promoted
009c647b  CALL  EDX                       ; ECX = the approach: vtable[0], 009C40A0
009c647d  FLD   float ptr [EAX + 0x4]     ; out[1] - approach+50h
009c6482  FSUBR double ptr [ESP + 0x10]
009c6493  FSTP  float ptr [ESP + 0x38]    ; B = aircraftY - aimPointY
```

`out[1]` is the aim point's vertical component, which is the `approach+50h` this packet corrected.
The same slot is read at `009C67C7`, the first argument of
`dive_bomb_flyabove_can_dive_009c680e`, so the can-dive test and both flags key on **one** height.

### The two flags, complete

```
B = height above the aim point                                    009C6493
S = max(B, 100.0) * 0.7 + 200.0        00D7A220/00CE3D08, 00CEFFA0, 00CE4D70
x = max(B - S, 0)                                          009C65D5-009C65FD
```

`009C65A9` is the byte `76`, JBE, so the floor takes `[ESP+38h]` when 100.0 is the smaller: `x0` is
`max(B, 100.0)`, and the `009C6568` product is a different occupant of that slot, as recorded.

* **`+19h`, second arm** (`009C67A9`, `009C67AE` byte `72` JC): fires on `x <= 0`, i.e. `B <= S`.
  For `B >= 100` that is `0.3B <= 200`, so **`B <= 666.7 m`**.
* **`+1Ah`** (`009C66E3`, `009C66E1` byte `76` JBE): fires on
  `|bearing error| > InterpolateClamped(0, 20 deg, W, pi, x)` with `W = approach+B4h * 0.8 - S`.

The two numbers corroborate each other and the packet before: `approach+D4h`, the can-dive height,
is `max(+A8h + 250, (+ACh + +A8h) * 0.5)` = **675.0 m** with this installation's row, and the
roll-in arm flips at **666.7 m**. Both are the same gate expressed twice - the aircraft rolls in
and may dive at essentially the same height - and both read the same `B`. Nothing here was fitted
to that agreement; it fell out of two independent traces.

Worked at the run's own geometry, `B = 638.9 m`: `S = 647.2`, `x = 0`, so the tolerance
`a` is its floor of 20 degrees and `W = 1100 * 0.8 - 647.2 = 232.8 m`. Below 667 m the roll-in arm
is already satisfied on height alone, which is why `flyabove` has never needed its bearing test in
any run of this stream.

## `0071EBF0`: the command-target rule, and what the host had instead

The host's `refresh_command_targets` took "the last current row wins". The image does something
narrower, and the difference is the whole bug.

```
0071ebf4  MOV  EAX,[EBP + 0x30]
0071ebfa  CMP  EAX,0x1
0071ebfe  JNZ  0x0071ecd8                  ; mode 2 -> this+18Ch; anything else -> the static
0071ec06  LEA  ECX,[EBP + 0x54]            ; the unit's own slot array
0071ec10  CMP  [ECX],EBX / JZ              ; stop at the first NULL
0071ec17  ADD  ECX,0x1c                    ; ten entries, stride 1Ch
0071ec1f  LEA  ESI,[EAX + -0x1]            ; the LAST occupied entry
0071ec36  MOV  ECX,[EDI]
0071ec3e  MOV  EAX,[EDX + 0xc] / CALL EAX  ; entry->vtable[+0Ch]
0071ec43  CMP  EAX,0x1 / JZ 0x0071ecc6     ; accept
0071ec48  CMP  EAX,0x2 / JZ 0x0071ecc6     ; accept
0071ec4d  SUB  ESI,0x1 / SUB EDI,0x1c      ; otherwise step BACKWARD
0071ec55  JGE  0x0071ec36
0071ec57  ...                              ; none answered -> the static at 00E19BB4
0071ecd1  LEA  EAX,[EBP + ECX*0x4 + 0x58]  ; the accepted slot's target field
```

So it is **the most recent command of an accepting category**, walking back over the others, and on
failure a neutral static record - never another unit.

Both fields are already on `GameCommandRow`: `slot_index` is the entry `0071E6C0` pushed and
`category` is what `vtable[+0Ch]` answers. The host ignored the category and ordered by vector
position, so a later row of any kind took the target.

### The blast radius, counted

This is not a dive-bomb bug. The command tables in `local\usn04_aim.log` carry:

| category | commands | rows |
| --- | --- | --- |
| 1, accepted | `attackmove` | 48 |
| 2, accepted | `divebomb` | 1 |
| 3, stepped over | `stop` 158, `moveto` 30, `cruise` 10 | 198 |

Every unit with an `attackmove` also carries `stop` and `moveto` rows, and the carrier launches now
add rows mid-mission, which is what rebuilt the map and let the category-3 rows win. So the wrong
rule was re-pointing the attack target of any of those units, and the dive bomber is simply where a
census made it visible: `approach+BCh` at exactly 0.0 m, the `d2 <= 1e-10` branch at `00CE3820`.

**Were the torpedo stream's bombers exposed?** Not in these runs, for a reason that has nothing to
do with the fix: USN04 builds no torpedo task at all - the summary line reads "no ordered aircraft
carries torpedo ordnance (kind 2Bh), so 0099A170 builds no kind Eh task". The moment that stream's
strike class gives an aircraft a category-1 or -2 row, it was exposed exactly as the dive bomber
was, because the rule is per-unit and category-blind, not task-specific.

### The fix

`refresh_command_targets` now walks each unit's accepting rows by descending `slot_index` (vector
position breaking ties, and unpushed rows ordered behind pushed ones, which is the best standing
this host has for them), takes the highest, and only then resolves that one row's token. Resolving
second is deliberate: the image returns the accepted slot's target field whatever it holds, so an
accepting row naming nothing leaves the unit with no target rather than falling through to an older
row. The cache key gains the count of current rows, so a row flipping current without the vector
growing re-resolves too; both halves are O(commands), not the O(commands x units) the name match
costs.

## The two flyabove flags, bound

> **CORRECTED, packet `cc8_dive_heading`.** The binding below is right about `S` and about the shape
> of both flags, and wrong about one operand: `dive_bomb_flyabove_span_009c65fd`'s minuend is the
> planar **range** `R`, not the height `B`, so `x = max(R - S, 0)` and the "`B <= 666.7 m`" this
> section derives does not exist in the image. `009C65DB`'s `FSUBP ST(2)` reaches past `B`, which
> `009C659D` has already popped, to the `R` pushed at `009C64EE`. The range is also measured to a
> three-second lead point, `aim - 3.0 * (v_own - v_target) - pos`, not to the target. Both are bound
> in "Packet `cc8_dive_heading`" below, which also measures the artefact this section produced.

With `T` closed, `+19h`'s second arm and `+1Ah` are no longer stand-ins.
`dive_bomb_flyabove_span_009c65fd` computes the pair once - `S = max(B, 100) * 0.7 + 200` and
`x = max(B - S, 0)` - and the host feeds it to all three flags, which is what the image does through
one frame slot. `+19h` gets the real `x` instead of the substituted 1.0, so its second arm fires at
`B <= 666.7 m`; `+1Ah` is `dive_bomb_flyabove_leave_009c66e3`, the 20-degrees-to-pi tolerance over
`approach+B4h * 0.8 - S`, in place of "leave when out of bombs".

`009C40A0` is now a defined function in Ghidra, `dive_bomb_approach_aim_point_009c40a0`, taking the
reviewed ledger name; body `009C40A0`-`009C40B7`, 22 bytes.

## The wide roll arm closed: the band is picked by attitude

`009C5D24`-`009C5D31` chose between two `InterpolateClamped` bands for the aimdive roll, and the
`fStack_44` its second test compares against 60 degrees had no writer at its corrected slot key.
The key was wrong, not the slot: `tools/frame_slot_census.py` puts the write at K=56 and the read at
K=-32, a drift of 100 bytes, and the tool's own docstring names the cause - an argument window
opened by `SUB ESP,imm` and closed by the callee's `RET imm16` rather than by an `ADD ESP,imm`,
which `--pop` does not cover.

Settled directly instead. `009C5D2C` is not inside any argument window - the nearest `SUB ESP,0x14`
is at `009C5D37`, after it - and the tick's frame is one fixed block opened by `SUB ESP,0x48` and
closed by `ADD ESP,0x48` before both `RET 4`s, so `ESP` there is at the base depth. The only writes
to that physical slot before it are:

```
009c590c  MOVSS XMM0,dword ptr [EDX + 0xc68]   ; pose+C68h, the bank
009c5914  COMISS XMM0,XMM1                     ; XMM1 = 0, XORPS at 009C58D3
009c5917  JBE   0x009c5921
009c5919  MOVSS dword ptr [ESP + 0x20],XMM0    ; bank
009c5929  SUBSS XMM1,XMM0                      ; -0.0 - bank
009c592d  MOVSS dword ptr [ESP + 0x20],XMM1
```

So it is **`|pose+C68h|`**, the folded bank, and the arm selection is:

| condition | band | endpoints |
| --- | --- | --- |
| `aim error > 0` **and** `|bank| < 60 deg` (`00D05AAC`) | wide | `+/- 0.5`, `00CE3800` |
| otherwise | tight | `+/- 0.4`, `00CE7804` / `00D1F400` |

Both tests are `76`, JBE: `009C5D22` on the error's sign and `009C5D31` on the bank. The rule reads
sensibly - while the aircraft is still near wings-level and short of its aim point the roll is
gentler, and once banked over or past the point it tightens.

The band selection is now bound. What is still a contract is the *other* operand: the image
interpolates the wide arm over a second bearing error, drawn against the latched reference at
`approach+D8h`/`+E0h`, and this host keeps one bearing. So the arm that runs is right and the value
it runs on is the single bearing, which is labelled at the call site.

## `approach+D8h`/`+DCh`/`+E0h` settled: the run-in origin, not a lead point

The provisional note is resolved, and the name was wrong twice over. These three are the
**aircraft's own world position, latched once at task construction**.

The chain that names the constructor's `EDI`:

```
009c73c5  PUSH EBP                        ; second argument
009c73c8  PUSH EAX                        ; FIRST argument
009c73cd  CALL 0x009c3ea0
...
009c3eca  MOV  EDI,dword ptr [ESP + 0x20] ; past seven prologue pushes -> that EAX
009c3ed2  PUSH EDI                        ; 009F9CE0's first argument
009c3ed5  CALL 0x009f9ce0
009f9ce0  MOV  EAX,dword ptr [ESP + 0x4]
009f9cea  MOV  dword ptr [ECX + 0x4],EAX  ; approach+4h, the unit
```

`approach+4h` is the unit everywhere else in the class - it is the entity whose `+C8h` pose flag
and `+FCh` position the approach reads - so `EDI` is the aircraft, and `009C405D`-`009C407D` copies
its `+FCh`/`+100h`/`+104h` into `+D8h`/`+DCh`/`+E0h`.

So the aimdive tick's **first** bearing, the one it takes after subtracting `+D8h` and `+E0h` from
the aim point, is the bearing **along the attack run as it was set up** - a fixed reference line
from where the aircraft was when the task was built to the aim point. The second bearing, taken
against the aircraft's live position, is the one the aim error uses. That is why the wide roll arm
exists at all: near wings-level and short of the aim point the roll follows the set-up line, and
once banked over or past it the roll follows the live bearing.

`dive_bomb_approach_off::kAimPointX/Y/Z` are renamed `kRunInOriginX/Y/Z`. Nothing referenced them,
so this is a header-only correction.

This also finishes the aimdive steering's last contract in principle: the wide arm's interpolant is
`SubtractWrappedAngle(heading, bearing(aimPoint - runInOrigin))`. This host does not latch a run-in
origin, so it still passes the live bearing to both arms, labelled at the call site - but the value
is now named rather than unknown.

## `local\usn04_target.log`: the command-target fix works, and the chain runs end to end

| measure | `usn04_aim` (broken target) | `usn04_target` (fixed) |
| --- | --- | --- |
| arm ticks | 2370 | 2109 |
| transitions | 3 | **7** |
| attackrun | 240 | 1527 |
| flyabove | 1 | 159 |
| turndown | 0 | 67 |
| aimdive | 0 | **318** |
| aimglide | 2129 | 37 |
| goaway | 0 | **1** |
| latch closed at | tick 241 (bogus) | tick 1528 |
| `approach+BCh` at the end | **0.0 m** | 1866.0 m |
| turndown latch | never | tick 1719, `|bank|` 3.1256 rad |
| `009C58D0` steer ticks | 0 | **318** |
| closest miss vs the 25.0 m window | 359.50 m | **347.40 m** |
| release altitude vs the {350, 450} floor | none | none; `approach+A8h` drew 350.0 |
| releases / bombs / rounds left | 0 / 0 / 2 | 0 / 0 / 2 |

The target no longer collapses: `approach+BCh` ends at a sane 1866.0 m instead of exactly 0.0, the
run-in takes its full 1527 ticks, and the walk reaches four states it had never reached together -
`aimdive`, `aimglide` and `goaway` in one sortie, seven transitions. The aimdive steering bound last
packet ran for all 318 of its ticks.

**No release, and the aim window is still the gate.** The closest the error came was 347.40 m
against 25.0 m - twelve metres better than before, which is noise, not progress.

### Why the steering did not help, to the metre

The arithmetic that cleared the aim error last packet says the same thing about this run. At the
closest sample - `|error|` 347.40 m, range 443.3 m, altitude 654.0 m - the model gives `x0 = 450`,
`x1 = 1000`, `t = 0.3709`, `lead = 25.96`, `gain = 0.7404`, so
`|cos(bearing) * 443.3 - 25.96| = 469.2`. The positive branch needs `cos = 1.116`, impossible; the
negative one gives **`cos(bearing) = -0.99977`**. The target was dead astern at the best moment of
the whole dive, exactly as in the previous run.

The last aimdive tick agrees: `bearing=2.8765 rad`, 164.8 degrees, with both commands saturated
(`pitch=-1.000`, `roll=-1.000`). So the steering is issuing full deflection the whole time and the
aircraft is not getting its nose onto the target.

That leaves two possibilities the endpoint numbers cannot separate:

1. the dive **starts** with the target already behind - the run-in closes to 1100 m, then `flyabove`
   (159 ticks) and `turndown` (67 ticks) burn 226 ticks, about 20 s, and a split-S reverses the
   heading, so `aimdive` may begin past the aim point; or
2. the dive starts pointed correctly and the roll is **too slow** to hold the nose on over 318
   ticks.

These have different fixes, so this packet adds a trace rather than guessing: the aim states now
record their entry range and bearing, their closest range, and range/error/bank/bearing every 30
ticks. `flyabove B=3.4 m span=0.0 m` at the end says the aircraft finished at sea level, so the
`approach+A8h` release floor of 350.0 m was reachable - the altitude gate is not what is blocking.

## The trace settles it: the dive starts pointing away, and `009C62B0` is why

`local\usn04_trace.log`, same walk as `usn04_target.log` (2109 arm ticks, seven transitions,
`aimdive` 318). The trace the last commit added:

```
aim trace: entry range=443.3 m bearing=-3.1411 rad | closest range=443.3 m | ticks=317
range/error/bank/bearing per 30 ticks:
  639/-625/2.98/2.93    945/-879/-3.03/-2.77   1177/-1088/1.75/2.75
  1320/-1285/-0.89/3.05 1431/-944/2.10/2.37    1493/-1142/-1.28/2.65
  1545/-1245/-0.98/2.77 1616/-1495/-0.75/2.86  1688/-1639/-0.55/2.90
  1752/-1697/-0.38/2.89
```

Three readings, and together they close the question the last run left open.

1. **The dive begins with the target 179.9 degrees behind it** - entry bearing -3.1411 rad - at
   443.3 m.
2. **443.3 m is the closest the aircraft ever gets.** The entry range is the minimum; the range
   rises monotonically to 1752 m over 317 ticks. The aircraft never turns back.
3. The bearing never falls below 2.37 rad, and the bank thrashes: +2.98, -3.03, +1.75, -0.89,
   +2.10, -1.28, -0.98, -0.75, -0.38. That is the falling roll map's limit cycle - the command
   clamps to -1, the aircraft rolls past inverted, the bearing's sign flips, the command flips.

So the earlier hypothesis pair resolves cleanly: **the dive starts past the aim point**, and the
roll is not "too slow" - it never had a chance, because nothing ever pointed the aircraft at its
target. The aim error inside the dive is a symptom.

### The gate, by address: `009C62B0` is unbound

The flyabove tick has no host binding. `tick_state` is still an empty override, and the wrapper
dispatches only `kAttackRun`, `kTurnDown` and `kAimDive` - exactly the omission that cost 664
aimdive ticks two packets ago, one state earlier. A command census over its body finds what it
would have issued:

| site | write |
| --- | --- |
| `009C69B1` / `009C69B9` | `cmd+2C4h` the bank target, with the mode `cmd+2CCh` |
| `009C6DE7` / `009C6DEF` | `cmd+2C0h` the **commanded heading**, with `cmd+2CCh` |
| `009C6F89` / `009C6F91` | `cmd+2BCh` the altitude, with the pitch mode `cmd+2D0h` |
| `009C6FF1` / `009C6FFB` | `cmd+2D8h` = 1 and `cmd+2B4h`, the air brake and desired speed |

A commanded heading, an altitude hold, a bank target and a speed: this is the state that flies the
aircraft into position over its target before the wingover. Without it the aircraft holds whatever
the run-in left and simply carries on for 159 ticks, about 14 s, overflying the target; the
turndown's split-S then reverses the heading and hands `aimdive` an aircraft pointed 180 degrees
the wrong way at 443 m.

That is one packet's work on its own - the body is 949 instructions with heading, altitude and
speed arms - so it is named here and not started.

### A second release path, also blocked, also by address

`aimglide` ran 37 ticks in these runs and issues its own salvo at `009C5777`. Its first two gates
are `009C569B`, the flight path shallower than 30 degrees, and `009C56AE`,
`height_above + 50.0 > height_limit`. The host supplies `height_above` as the raw altitude and
`height_limit` as `approach+ACh` = 1000.0, so `009C56AE` demands more than 950 m and the aircraft
is at 654 m by then. Both of those inputs sit inside the PARTIAL that
`dive_bomb_aimglide_inputs` labels - four frame slots behind `009C5693`-`009C5755` were never
traced - so the aimglide release is closed by a substitution, not by a recovered rule.

## The state-dispatch inventory: which ticks still run nothing

`tick_state` is an empty override at both dive-bomb binding sites; every state tick this host runs
is dispatched explicitly from the arm wrapper. After this packet that is four of ten.

| state | task offset | tick | dispatched? |
| --- | --- | --- | --- |
| `kMoveTo` | `4F0h` | `009C1FD0` (the follow base) | **no** |
| `kFollow` | `52Ch` | `009C1FD0` (the follow base) | **no** |
| `kPrepare` | `5C4h` | `009C7270` `BSP_BotStateDiveBombDone_Tick` | **no** |
| `kDone` | `664h` | `009C7270` `BSP_BotStateDiveBombDone_Tick` | **no** |
| `kGoAway` | `704h` | `009C4A40`, body `009C4A40`-`009C4E65` | **no** |
| `kAimDive` | `734h` | `009C58D0` | yes, steering only |
| `kAimGlide` | `754h` | `009C5180`, body `009C5180`-`009C580B` | **no** |
| `kFlyAbove` | `778h` | `009C62B0` | yes, heading only (this packet) |
| `kTurnDown` | `79Ch` | `009C44F0` | yes |
| `kAttackRun` | `7BCh` | `009C4220` | yes |

There is **no `kRollIn` state**: the registrar `009C73A0` names exactly ten through
`BSP_BotStateRegistry_Add` at `009C7680`-`009C76C6`, and the enum above is all of them. The
roll-in is a *flag*, `flyabove+19h`, not a state.

So the next empty overrides, found by reading rather than by a run, are **`009C5180`** (aimglide,
1675 bytes - it owns the second release site `009C5777`, which ran 37 ticks in `usn04_target.log`)
and **`009C4A40`** (goaway, 1061 bytes). `009C7270` is ten bytes and tail-calls `009C1FD0`.
`009C7240` and `009C7260` are the done state's enter and exit, not ticks.

## `009C62B0`'s command side, and `009C56AE` recovered

### The four command arms, read

| site | write | condition |
| --- | --- | --- |
| `009C69B1` / `009C69B9` | `cmd+2C4h` = 0.0, `cmd+2CCh` = 1 | a wings-level bank target handed to the planner's servo |
| `009C6DE7` / `009C6DEF` | `cmd+2C0h` = the heading, `cmd+2CCh` = 2 | `state+1Ch == 0` (`009C6DCD`, `009C6DDA` byte `75` JNZ) |
| `009C6F89` / `009C6F91` | `cmd+2BCh`, `cmd+2D0h` = EDX | the arm that does **not** call `009FB800` at `009C6F7D` |
| `009C6FEA` / `009C6FF1` / `009C6FFB` | `cmd+2B0h` = 0, `cmd+2D8h` = 1, `cmd+2B4h` | the speed is `something + approach+A4h` (`009C6FE1`) |

Only the heading arm is bound, and the packet says so rather than inventing the rest. The reason is
the same one that bit the aimdive: `tools/frame_slot_census.py` cannot be trusted in this body. It
flags nine call sites it cannot account for, and the gap its own docstring names - an argument
window opened by `SUB ESP,imm` and closed by the callee's `RET imm16` - already puts the write at
`009C6336` and the read at `009C6DC1` four bytes apart on paper when both name `[ESP+54h]`.

So the heading's **value** is a labelled substitution: the image writes
`AddWrappedAngle(base, clamp(delta, -L, +L))` - the call at `009C6DC8`, the clamp built at
`009C6D7E`-`009C6DB0` from `[ESP+1Ch]`, `[ESP+34h]` and the negation at `009C6D7E` - and the host
commands the bearing to the aim point instead. What is **not** substituted is the part the trace
indicts: that a heading is commanded at all, with mode 2, which is the planner arm the run-in
already uses and `pilot_plan_roll_0099e2ba` already models.

### `009C56AE`: both producers recovered, and the gate was inverted

The aimglide release's second gate was reconstructed as `height_above + 50 > height_limit`, with
the host feeding raw altitude against `approach+ACh` = 1000.0 - so it demanded the aircraft be above
950 m. The operands are the other way round:

```
009c56a6  FLD  float ptr [ESP + 0x20]        ; loaded FIRST -> ST1
009c56aa  FLD  float ptr [ESP + 0x1c]        ; loaded second -> ST0
009c56ae  FADD double ptr [0x00ce3938]       ; + 50.0
009c56b4  FCOMIP ST0,ST1                     ; ([ESP+1Ch] + 50) vs [ESP+20h]
009c56b8  JBE  0x009c57c4
```

Both producers:

* **`[ESP+20h]`**, written at `009C5281`, is the **height above the aim point** - the same
  construction as the flyabove's `B`: `009C5278` calls `approach->vtable[0]`, `009C527A` takes its
  `out[1]`, `009C527D` `FSUBR` subtracts it from the aircraft's Y.
* **`[ESP+1Ch]`**, written at `009C5493`, is `(approach+14h)->+40h * approach+A8h`
  (`009C548A`/`009C548D`). `approach+14h` is the 0x248-stride robots row viewed `0xCh` in, so
  `->+40h` is row `+4Ch`, `dive_bomb_new_release_mul_04c`.

This installation's `SPNormal` row authors `DiveBombNewReleaseMul = 0.6`, and the authors' own
comment settles what it is: *"ha nem leboritott manoverrel bombaz, csak siman rarepulve, akkor a
fenti ReleaseAlt erteket ennyivel megszorozva hasznalja"* - if it bombs **without** the wingover
manoeuvre, just flying straight at it, it uses the ReleaseAlt value multiplied by this.

So the gate is a **ceiling, not a floor**: `0.6 * 350.0 + 50.0 = 260.0 m`. The glide release opens
below 260 m, and the reconstruction had it demanding more than 950 m - which is why `aimglide` ran
37 ticks in `usn04_target.log` and released nothing.

### A split worth naming: `cmd+2CCh` is two fields in this host

`cmd+2CCh` is one word in the image. This host has two:

* `GameUnitSlot::plan_heading_mode_2cc` (`src/game_hosts_units.cpp:595`), which is the field the
  planner's gate reads at `0099DE8A`/`0099E275` and which every task tick writes; and
* `PilotPlanState::heading_mode_2cc` (`include/bsp/pilot_plan_slots.hpp:90`), which
  `pilot_reset_plan_0099b450` sets to **1** every think, transcribing `0099B548`.

The reset therefore never reaches the gate. In the image `0099B548` re-arms mode 1 on every pilot
think, so a state that writes no mode still gets the **servo** arm, holding whatever bank target
`cmd+2C4h` already carries. In this host the field persists between thinks instead, and the
planner's own mode-2 arm ends by writing 0 to it at `0099E3B5` - so after one think a state that
writes no mode gets **neither** roll arm and the aircraft simply stops banking.

That is the shape of the flyabove drift, and it is why binding the heading arm is not merely
cosmetic: re-arming mode 2 every flyabove tick is what keeps the planner banking. It is also a
planner-wide difference, not a dive-bomb one - every bot state that writes no roll mode is affected -
so it is named here and **not** changed in this packet: the fix is one word in the reset path, but
it moves the default roll behaviour of every planned aircraft and needs its own run.

### Both aimglide lead gates are mis-transcribed, and that is why it never fires

Reading `009C5704`-`009C5755` to interpret this run turned up the same class of error as the
ceiling, twice more. The listing:

```
009c5725  FSUB  float ptr [ESP + 0x2c]       ; lead = lateral_b - cos(angle) * lateral_a
009c5729  FLD   float ptr [ESP + 0x1c]       ; travel = state+20h
009c572f  FADDP ST2,ST0                      ; ST1 = lead + travel
009c5733  FADD  double ptr [0x00d7a370]      ; + 5.0
009c5743  FCOMI ST0,ST1                      ; travel  vs  (lead + travel + 5.0)
009c5745  JBE   0x009c57c0                   ; byte `76`
009c5747  FCHS                               ; -travel   (ST0 survived the FCOMI)
009c5749  FMUL  double ptr [0x00d7a2b0]      ; * 3.0
009c5751  FCOMIP ST0,ST1                     ; (lead + travel + 5.0)  vs  (-travel * 3.0)
009c5755  JBE   0x009c57c4                   ; byte `76`
```

| gate | the image proceeds when | the reconstruction requires |
| --- | --- | --- |
| `009C5745` | `travel > lead + travel + 5.0`, i.e. **`lead < -5.0`** | `lead > 5.0` |
| `009C5755` | `lead + travel + 5.0 > -travel * 3.0`, i.e. `lead > -4*travel - 5.0` | `-(lead + travel) * 3.0 > travel + 5.0` |

Both are wrong, and the first has the **sign of the lead backwards**. The image's two gates are
satisfiable together - `-4*travel - 5.0 < lead < -5.0`, a window that is non-empty whenever the
travel accumulator `state+20h` is positive - whereas the reconstruction's pair is mutually
exclusive at `travel = 0`, which is what the host substitutes. So the aimglide salvo at `009C5777`
could never fire in this host, at any altitude, independently of the ceiling corrected above.

`00D7A370` is 5.0 and `00D7A2B0` is 3.0, both qwords; both branch bytes are `76`, JBE.

Not changed in this commit: a run is in flight on the current build, and the fix wants
`state+20h`'s own producer read as well. It is the first thing to do after this run lands.

## `local\usn04_flyabove.log`: the heading arm works, and the gate moves to the turndown

`009C62B0 ticks=159 heading writes=159` - the arm ran on every flyabove tick - and the geometry it
produces is the point:

| measure | `usn04_target` | `usn04_flyabove` |
| --- | --- | --- |
| flyabove ticks / heading writes | 159 / 0 | 159 / **159** |
| **range when the turndown starts** | not measured | **3.8 m** |
| bearing when the turndown starts | not measured | 2.9100 rad |
| turndown ticks | 67 | 71 |
| aimdive entry range | 443.3 m | 472.6 m |
| aimdive entry bearing | -3.1411 rad | 3.1219 rad |
| closest range in the aim states | 443.3 m | 472.6 m |
| closest miss vs the 25.0 m window | 347.40 m | 374.99 m |
| releases / bombs / rounds left | 0 / 0 / 2 | 0 / 0 / 2 |

**The flyabove now flies the aircraft to 3.8 m of its aim point.** That is the binding working: with
no tick it held the run-in heading and overflew; with the heading arm re-arming mode 2 every tick
the planner banks it right over the target.

**The gate has moved one state earlier than the aim window.** The turndown begins directly overhead
and ends 472.6 m away pointing 178.9 degrees off, because `009C7EA0` exits on **attitude alone** -
`pose+C64h < -1.0` with `|bank|` past 2.356 - and carries no heading or range term at all. 71 ticks
is 6.4 s, and at the run's speed that is the 472 m. So the wingover spends the whole approach the
flyabove just bought, and `aimdive` inherits an aircraft pointing away exactly as before; the aim
trace still shows the range rising monotonically, 660 to 1842 m.

The miss is 374.99 m, slightly worse than the previous run's 347.40 m, which is noise on the same
geometry: in both, the aimdive entry range **is** the closest range.

So the next gate is `009C7EA0` and the turndown's pull rate, not the aim error and not the flyabove.
The dive-bomb chain now reaches the right place and leaves it too slowly.

## The pull-through: what the laws command, and where the divergence is not

### (b) The bearing error is built from the nose, not the velocity

`docs/PLANE_ATTITUDE_ANGLES.md` section 2: `unit+C6Ch` is `atan2` of the pose's **forward axis**
after the pitch is rotated out of it - `007C195F` crosses `fwd` with world up, `007C1A14` rotates
the matrix about that axis by the pitch, and `007C1A21` takes `atan2(U.m22, U.m20)`, wrapped at
`007C1AAD`. So the heading is the **nose's horizontal projection**. The aimdive's bearing error is
`SubtractWrappedAngle(that heading, bearing to the aim point)`; the velocity never enters it. The
aim error's `cos(bearing) * range` is therefore an along-**nose** projection, not along-track.

### (c) The attitude integration is not the divergence

`include/bsp/plane_advance_pose.hpp` reconstructs `007C6500` as axis-angle rotations of the pose
matrix (`RotationAxisAngle`) followed by `orthonormalize_up_first_0085dad0`. A matrix advanced that
way has no Euler singularity in its state and carries an aircraft through the vertical; the
singularity lives only in the **derived** angles, and both sides guard it identically - the image
skips the heading and bank writes when `|fwd x up|` falls under the 1.0842e-10 at `00CE3820` or
`|cross|` under the 0.001f at `00D7A23C` (`007C19A9`, `007C19D4`), and `refresh_attitude_007c1900`
does the same. So the host can fly an aircraft through the vertical, and this is **not** where it
diverges.

### (a) What the laws command for an inverted aircraft with the target astern

The turndown hands over at exactly the attitude `009C7EA0` demands: `pose+C64h` crossing -1.0 rad,
57.3 degrees nose-down, with `|bank|` past 2.356 - inverted. At that instant the target is astern,
so `009C5C97`'s aim error is strongly negative, `009C5CA9`'s `76` JBE picks the negative arm, and
`009C5CD0` scales it by `DiveBombAimPrecPullMinus` = 0.025. With an error of hundreds of metres the
clamp at `009C5CE5` saturates:

* **pitch = -1.0**, full **forward** stick. `+1` is back stick, which is what the turndown uses to
  pull the nose down while inverted; `-1` is its opposite, so at 57 degrees nose-down inverted it
  **raises** the nose and aborts the pull-through.
* **roll** clamps to `-1` through the falling map, and the bank thrashes as the bearing's sign flips
  each time the aircraft rolls past inverted.

The trace is exactly that picture: `pose_c64_min` never passes -0.98, the heading never reverses,
and the range rises monotonically from 660 m to 1842 m.

### The honest answer: the image commands the same push

Nothing here is a transcription error. `009C5C97`, `009C5CA9` and `009C5CD0` are read from the
listing and the gains are the authored row; the same arithmetic on the image's side gives the same
`-1.0` for a target astern. So **the image overshoots on this pass too** - the first dive is a miss
by construction once the wingover has put the target behind.

What the image does about it is the part this host has never reached: `009C86D9` sends `goaway`
back to `flyabove` **when bombs remain**, so the aircraft climbs back over the target and tries
again. Our runs stop with `goaway=1` tick, `rounds_left=2` and `transitions=7`: the aircraft was set
up to go round and the mission window ended. That is a falsifiable prediction - a longer run should
show a second `flyabove`/`turndown`/`aimdive` circuit - and it is the next thing to test, not
another law to re-read.

## `cmd+2CCh` made one word: a same-build before and after

The reset now copies `PilotPlanState::heading_mode_2cc` into the field the roll gate reads, so
`0099B548`'s per-think re-arm reaches `0099DE8A`/`0099E275` as it does in the image. Both runs of
each pair are the same build, differing only by that copy.

| measure | USN01 before | USN01 after | USN04 before | USN04 after |
| --- | --- | --- | --- | --- |
| ordered aircraft | 5 | 5 | 1 | 1 |
| `range_last_mean` | 3858.8 m | **3806.2 m** | 1934.9 m | 1934.1 m |
| `closed_mean` | 315.6 m | **368.1 m** | 9175.0 m | 9175.8 m |
| `worst_closed` | 280.3 m | **330.7 m** | 9175.0 m | 9175.8 m |
| `heading_error_last_mean` | 0.016 rad | **0.005 rad** | 2.815 rad | 2.805 rad |
| dive-bomb walk | - | - | 2118 ticks, 7 transitions | 2117 ticks, 7 transitions |

**USN01 moves and USN04 does not, and both are the expected result.** USN01's five torpedo aircraft
fly states that often write no roll mode, so before the fix they fell to mode 0 after the planner's
own arm zeroed it at `0099E3B5` and simply stopped banking; with the reset reaching the gate they
get the servo arm every think and close 368.1 m instead of 315.6 m, worst-case 330.7 m instead of
280.3 m, and finish three times better aligned - 0.005 rad against 0.016.

USN04 is unchanged to within a tick because every dive-bomb state now writes a mode explicitly -
attackrun 2, turndown 0 then 1, aimdive 0, flyabove 2 - so the reset's default never applies there.
That is the control: a planner-wide change that improves the mission whose states leave the mode
alone and does not disturb the one whose states do not.

### `state+20h` recovered, and the glide path's three corrections applied

`state+20h`, the aimglide travel accumulator, has two writers and both are now read.

```
009c4f44  FLD   double ptr [0x00d7a370]   ; 5.0
009c4f4a  FCOMIP ST0,ST1                  ; 5.0 vs the enter's argument
009c4f4e  JBE   0x009c4f62                ; byte `76`
009c4f50  MOVSS XMM0,dword ptr [0x00ce3850] ; 5.0
009c4f58  MOVSS dword ptr [ESI + 0x20],XMM0
009c4f62  MOVSS XMM0,dword ptr [ESP + 0x4]
009c4f68  MOVSS dword ptr [ESI + 0x20],XMM0
```

The aimglide enter `009C4F00` seeds it to **`max(arg, 5.0)`** - never zero - and the tick
accumulates into it at `009C57B8`/`009C57BB`. The host seeded 0.0, which closed the glide release
by itself: the two lead gates are satisfiable only while the accumulator is positive.

With all three transcription errors corrected the glide path's gates now read:

| gate | rule |
| --- | --- |
| `009C569B` | flight path shallower than 30 degrees |
| `009C56A6`-`009C56B8` | height above the aim point **below** `0.6 * 350.0 + 50.0` = 260.0 m |
| `009C56C2`-`009C56FE` | lateral offset inside 120.0 |
| `009C5743` | `lead < -5.0` |
| `009C5751` | `lead > -4*travel - 5.0` |

which is a coherent window - `-4*travel - 5.0 < lead < -5.0`, non-empty because the accumulator
starts at 5.0 - where before the pair was mutually exclusive and the ceiling was a floor demanding
950 m. The salvo at `009C5777` could not fire under any of those; it can now.

The tick `009C5180` itself is still not dispatched. Binding it whole is the next packet: what is
corrected here is the release rule it owns and the accumulator that feeds it, so the release can
fire the moment the tick runs.

### `kAimGlide` dispatched: `009C5180`'s heading arm

The glide tick's command census has the same four-arm shape as the flyabove's:

| site | write | register |
| --- | --- | --- |
| `009C5400` / `009C5408` | `cmd+2C4h` bank target, `cmd+2CCh` = EBX | EBX = 1, the servo |
| `009C5442` / `009C5450` | `cmd+2C0h` heading, `cmd+2CCh` = EBP | EBP = 2, the planner's own arm |
| `009C55D7` / `009C55DF` | `cmd+2BCh` altitude, `cmd+2D0h` = EBP | EBP = 2 |
| `009C567F` | `cmd+2D8h` = 0 | |

`EBP` is the 2 that `009C53DD` loads and `EBX` the 1 that `009C53E2`'s `LEA EBX,[EBP-1]` takes from
it. A sibling arm at `009C5414` commands the **yaw slot** directly instead - `cmd+284h`, `+288h` and
`+2D4h` = 0 - and is not bound.

Only the heading arm is bound, on the same terms as the flyabove's and with the same labelled
substitution for its value (the image reads it from `[ESP+6Ch]` at `009C5435`). That is the arm
that steers, and the flyabove's equivalent measurably worked - it put the aircraft 3.8 m from the
aim point.

So `kAimGlide` now dispatches, and with the three transcription corrections and the `state+20h`
seed the release at `009C5777` has a reachable window for the first time: angle under 30 degrees,
height above the aim point under 260 m, lateral inside 120, and
`-4*travel - 5.0 < lead < -5.0`.

## The three slot runs, and a retraction

Three runs on slots 0-2, all on a tree **without** main's `67e8ac821`, so they compare directly with
`usn04_after.log` and `usn01_after.log`.

| measure | `usn01_after` | `usn01_glide` | `usn04_after` | `usn04_glide` | `usn04_circuit2` |
| --- | --- | --- | --- | --- | --- |
| mission frames | 4800 | 4800 | 4800 | 4800 | **8800** |
| `closed_mean` | 368.1 m | **368.1 m** | - | - | - |
| `worst_closed` | 330.7 m | **330.7 m** | - | - | - |
| `heading_error_last_mean` | 0.005 rad | **0.005 rad** | - | - | - |
| dive-bomb arm ticks | - | - | 2117 | 2118 | **2118** |
| transitions | - | - | 7 | 7 | 7 |
| aimdive / aimglide / goaway | - | - | 318 / 41 / 1 | 318 / 42 / 1 | 318 / 42 / 1 |
| releases | - | - | 0 | 0 | 0 |

USN01 reproduces the control exactly, so the `kAimGlide` dispatch costs the torpedo planner nothing.
USN04 is unchanged too - the glide corrections and the dispatch do not move the walk, because
`aimglide`'s 42 ticks happen after the aircraft is already low and out of position.

### Retraction: there is no second circuit, and the window was never the reason

The last packet predicted that a longer run would show `009C86D9` sending `goaway` back to
`flyabove` for a second circuit, and that our 4800-frame runs simply ended first. **That is wrong.**
The 8800-frame run stops at **2118 arm ticks, the same as the 4800-frame run** - 190 s of task in
both - and the log says why:

```
plane water contact: unit=movieval alt=-1.12 water=0.00 |v|=43.72 state 7 -> 6
  (007CB7F0 tail 007CB92C); the free-flight gate 0074E210 is now false
```

**The dive bomber flies into the sea.** Once the flight state leaves 7 the pilot think stops, the
task stops being armed, and no amount of mission time changes anything. The chain does not run out
of window; it runs out of aircraft.

### What that makes the gate: `009C4A40`, the goaway tick

The walk is `aimdive` 318 -> **`goaway` 1 tick** -> water. So the pull-out edge works: it is computed
at `009C6131`-`009C6154` and `apply_aimdive_result` does store it back into `db_aim_pull_out_18`,
`009C8677` reads it and the state does leave `aimdive`. What happens next is nothing, because
**`kGoAway`'s tick `009C4A40` is one of the six this host still dispatches nothing for**. The state
that exists to climb the aircraft away from its dive issues no command, so the aircraft holds its
dive attitude and ditches one tick later.

That also retracts the milder claim that "the image overshoots this pass too". The image's dive
bomber does not fly into the water; it pulls out, goes round, and `009C86D9` gives it the second
circuit with its two remaining rounds. The missing piece is not a law in the aimdive - it is the
goaway tick, at `009C4A40`, body `009C4A40`-`009C4E65`, 1061 bytes.

## `009C4A40`, the goaway tick: the climb-out bound

### The command census, with EBX the 1 that `009C4A5C` loads

| site | write | mode |
| --- | --- | --- |
| `009C4BE0` / `009C4BE8` | `cmd+2BCh`, the pitch target | `cmd+2D0h` = **1** |
| `009C4BFE` / `009C4C06` | `cmd+2C4h` = 0.0 (`XORPS`) | `cmd+2CCh` = **1** |
| `009C4CA7`, `009C4CE7` | `cmd+2D8h` = 0 | |
| `009C4DF0` / `009C4DF6` | a second bank-target arm | `cmd+2CCh` = 1 |
| `009C4E17` / `009C4E1D` | `cmd+2C0h`, a heading | `cmd+2CCh` = 2 |

Both of the climb-out's modes are **1**, and that is the whole shape: hand the planner a pitch
target and let its own arm at `0099E490` fly it, and hand the roll servo a wings-level target so the
aircraft rolls upright out of the inverted dive. The mode-1 pair is exactly what the two gates bound
in the previous packets pass - `0099E3BF` for the pitch, `0099E26E` for the roll.

### The pitch target

```
009c4ba0  FLD   float ptr [ECX + 0x1ec]      ; (approach+8h)->+1ECh, the climb angle
009c4baa  FLD   float ptr [0x00ceb4b0]       ; 60.0
009c4b96  FLD   float ptr [0x00ce3ae8]       ; 300.0
009c4b90  FLDZ
009c4b8c  FSTP  float ptr [ESP + 0x10]       ; [EDI+100h], the aircraft's own Y
009c4bb3  CALL  BSP_Math_InterpolateClamped
009c4bc4  FCOMIP ST0,ST1                     ; against the 009C4B61 curve
009c4bc8  JA    0x009c4bd2                   ; byte `77`, so the larger wins
```

`InterpolateClamped(60.0, climbAngle, 300.0, 0.0, altitude)`: the full climb angle below 60 m,
easing to level by 300 m, and the larger of it and a second curve over the same upper endpoint whose
`y1` and interpolant were not traced. The second is supplied as the same curve and labelled, so the
command is never weaker than the image's.

`(approach+8h)->+1ECh` needs no substitution at all: this host already carries it as
`plane_climb_angle_1ec`, filled from the Lua row as 0.6 of the sustainable climb angle
`007D98F0` returns.

### A correction the binding turned up: `009C7F00` was reading another state's field

`goaway_complete` was fed `db_glide_travel_20` - the **aimglide** state's `+20h`. `009C7F00` is the
goaway state's own completion rule and reads the goaway state's `+20h`; they are different objects.
Once the aimglide seed was recovered last packet as `max(arg, 5.0)`, that conflation made
`planar > 5.0 * 0.9` true at once, which is the **one-tick goaway** in every run so far. The goaway
now has its own accumulator. The rule stays the labelled PARTIAL it was, but it is no longer fed a
value belonging to another state.

## `009C7F00` read whole: the half that was missing

The first goaway run changed nothing - 2118 arm ticks, a one-tick goaway, water contact at
-1.12 m - because the completion rule still answered true on the state's first tick. It was
modelled from its first condition only. Read whole:

```
009c7f03  FLD  float ptr [ECX + 0x20]        ; the goaway state's own +20h
009c7f09  FLD  double ptr [0x00d7a390]       ; 0.9
009c7f12  FMUL ST1                           ; term = state+20h * 0.9
009c7f23  FLD  float ptr [EAX + 0xac]        ; approach+ACh
009c7f29  FADD float ptr [EAX + 0x50]        ; + approach+50h, the aim point's height
009c7f38  FCOMIP ST0,ST1 / JBE               ; ceiling = min(ctl+398h, that sum)
009c7f51..009c7f7a                           ; all three flags set -> term *= 0.9 again;
                                             ; the first two set without the ordnance -> false
009c7f8d  FCOMIP ST0,ST1 / JBE 0x009c7fd1    ; approach+BCh must exceed the term
009c7fb2  FSUB double ptr [0x00d7a220]       ; ceiling - 100.0
009c7fc2  FCOMIP ST0,ST1 / JBE 0x009c7fd1    ; and the aircraft's Y must exceed that
009c7fc8  MOV  EAX,0x1
```

So `goaway_complete` is **two** conditions, and the second is the whole point of the state:

| condition | site |
| --- | --- |
| `approach+BCh > state+20h * 0.9` (again * 0.9 with the three flags) | `009C7F8D`, byte `76` |
| **aircraft Y > min(ctl+398h, +ACh + +50h) - 100.0** | `009C7FC2`, byte `76` |

The aircraft must have **both** opened the range and climbed back to within 100 m of its cruise
altitude. With `approach+ACh` at 1000.0 and the aim point at sea level that is **900 m**, so an
aircraft coming out of a dive at 650 m stays in goaway and keeps climbing - which is exactly what
the climb-out arm bound above it exists to do. Modelling only the first condition made the state
end instantly no matter what the tick commanded.

### A planner reading worth checking, from the torpedo side

cc8-plane-squadron reports USN01's Mavs flying into the water holding a pitch demand of exactly
`-pi/3` while `climb_1ec` is printed and not flown. The pitch floor they are hitting is
`plane_ai_control.cpp:343`, `floor_target = pitch_turn_max_pitch - 2.5 * (1 - q)`, whose comment
says it "can only RAISE the target - which is the whole of what stops a bot flying into the sea".

That cannot be true with the authored value. `PitchTurnMaxPitch` is `DEG(06)` = 0.10472 rad, so at
`q = 0` the floor is `0.10472 - 2.5` = **-2.395 rad** - below anything an aircraft can fly, hence
inert. And the row's own comment describes an increment, not an absolute: *"max ennyi fokkal a
tenyleges target pitch-nel nagyobb target pitch-t akar elerni. emiatt jobban fogja huzni a pitch
kontrollt, es jobban kanyarodik"* - at most this many degrees **greater** target pitch than the
actual one, so it pulls harder and turns better.

So `0099E4DC`-`0099E512` is probably a delta on the target rather than an absolute floor. That is
the planner, shared with the torpedo and squadron streams, so it is named here and not changed.

## The same zero-span defect is in the dive-bomb attackrun

cc8-plane-squadron found that the torpedo attackrun hands `009FBA50` a zero range pair, killing the
glide bias. The dive-bomb attackrun does the same thing, and the listing shows what the image
passes instead.

`009C43D2 SUB ESP,0x10` opens the window and the four floats go in at:

```
009c43f3  FLD [EDI+0xac] / FADD [EDI+0x50]   -> [ESP]      base = approach+ACh + approach+50h
009c43e3  FLD [EDI+0xb4]                     -> [ESP+4]    approach+B4h
009c43db  FLD [ESP+0x54]                     -> [ESP+8]    the frame value 009C4317 stored
009c43cd  CALL 00419010                      -> [ESP+0xC]  an InterpolateClamped result
009c4401  CALL 0x009fba50
```

The two range arguments are **`approach+B4h` and a separately computed frame value** - two different
numbers. This host passes `in.attack_distance_b4` for both:

```cpp
cin.range_low = in.attack_distance_b4;
cin.range_high = in.attack_distance_b4;
```

so `span = max(high - low, 0)` is zero and the bias `span * scale * class+518h` vanishes, leaving
the bare base. That is precisely the defect on the torpedo side, in a second call site. The base is
wrong too: the image's is `approach+ACh + approach+50h`, the same sum `009C7F00`'s ceiling uses,
while this host passes `r.commanded_altitude_base`.

`[ESP+44h]` is the tick's own `dt` slot at entry - the prologue is `SUB ESP,0x38` plus two pushes,
so entry `[ESP+4]` is `[ESP+44h]` - reused as scratch once `dt` is consumed; the value the call
reads is the one `009C4317` stores, which is one level further back and not yet traced.

This is a real candidate for why the dive bomber's altitude profile is wrong through the whole
run-in, and it is upstream of everything the last four packets bound. It needs
`src/game_hosts_units.cpp`, which is currently released, so it is recorded here and will be taken
with its own window rather than by quietly re-claiming the file.

## Correction to the `PitchTurnMaxPitch` note above: it is a max, not a delta

I wrote that `0099E4DC`-`0099E512` is "probably a delta on the target, not a floor", inferring it
from the authored row comment. **That is wrong**, and cc8-plane-squadron refuted it from the
listing:

```
0099e4dc  FMUL  double ptr [0x00ce3de0]
0099e4e2  FSUBP                           ; floor = pitch_turn_max_pitch - 2.5*(1-q), an ABSOLUTE
0099e4e8  FLD   float ptr [ESI + 0x2bc]   ; the existing target
0099e4fa  FCOMIP ST0,ST1                  ; floor vs target
0099e4fe  JBE   0x0099e508                ; floor <= target -> keep the target
0099e500  MOVSS XMM0,dword ptr [ESP+0x40] ; else take the floor
0099e512  MOVSS dword ptr [ESI + 0x2bc],XMM0
```

`FSUBP` builds the floor as an absolute and `FCOMIP`/`JBE` selects the larger of it and `cmd+2BCh`.
There is **no `FADD` against the target anywhere in the block**, so it is a max and
`src/plane_ai_control.cpp` has the shape right. Reading the Hungarian row as a delta was my
inference, not evidence, and an increment would have been a behaviour change in the wrong
direction. The row fits the max reading too: raising the target in a hard bank *is* what makes it
pull harder and turn better.

### What does stand: the floor is inert in level flight, and one comment says otherwise

With `PitchTurnMaxPitch` = `DEG(06)` = 0.10472 rad the floor is `0.10472 - 2.5*(1 - q)`, which at
`q = 0` is **-2.395 rad** - below anything an aircraft can fly - and rises to +0.10472 only as `q`
approaches 1. As a max that is a coherent design: hold the nose at least six degrees up in a hard
bank, do not interfere in level flight.

So the defect is one sentence of a comment, not the code. `src/plane_ai_control.cpp:339-342` says
the floor "is applied unconditionally on every pass of the law, and it can only RAISE the target -
which is the whole of what stops a bot flying into the sea". The first half is right; the last
clause is not. It is inert in level flight by construction, so it stops nothing in a straight-line
descent - which is exactly what USN01's Mavs and this dive bomber both do. That file is not held
here; the one-sentence fix belongs to whoever next holds it.

### And it does not support the zero-span finding either

Stated plainly so the two are not read as mutual support: `0099E490` consumes a pitch target and
`009FB800` produces one. The zero range pair is a fault in the **producer**, and a floor that
worked perfectly would only cap how steeply an aircraft obeyed a command it should never have been
given. The two findings are independent, and the zero range pair is the one that matters.

## Baseline: which logs may share a table

Two trees are in play across the streams and their numbers must not be merged.

| side | tree | carries `67e8ac821`? |
| --- | --- | --- |
| this stream's runs, `usn01_*`, `usn04_*` | `794922056` and later on `agent/cc8-dive-bomb` | **no** |
| `local/tap_before_usn01.log` (plane-squadron) | `b882aa1d4` | **yes**, plus the AI weight model and the squadron wing |

`67e8ac821` holds back every authored object whose block sets `Hidden = B true`, so USN04 creates 19
units at load instead of 53. Every aircraft census moves with it. A planner-wide before/after - the
zero range pair is the one worth measuring - therefore needs **both** columns on one side: either a
fresh before taken here, or this stream's runs re-taken on a merged tree. Not one column from each.

Recorded because the two `before` logs look interchangeable by name and are not.

### The `009FBA50` call fully determined: the second range is the live planar distance

The one argument left unnamed above is read. `009C4311 FLD [ECX+0xbc]` / `009C4317 FSTP [ESP+44h]`
puts **`approach+BCh`, the live planar range to the target**, into the slot `009C43DB` later loads
into `[ESP+8]`. So the whole call at `009C4401` is:

```
009FBA50( base       = approach+ACh + approach+50h    009C43F3
          range_low  = approach+B4h                   009C43E3   the attack distance
          range_high = approach+BCh                   009C43DB   the LIVE planar range
          throttle   = the 009C43CD InterpolateClamped result )
```

`span = max(range_high - range_low, 0)` is therefore **the distance still to close**: about 9900 m
at 11 km out, shrinking to zero as the aircraft reaches its attack distance. That is the glide
slope - hold high while far out, come down to the base as you close.

This host passes `attack_distance_b4` for both, so `span` is identically zero and the aircraft is
commanded to the bare base altitude from 11 km out. It descends at once instead of gliding down as
it closes, which is why its whole altitude profile is wrong before any of the states this stream has
bound get a say.

The fix is two lines at the call site in `run_dive_bomb_attackrun_tick_009c4220`:

```cpp
cin.base_altitude = in.begin_altitude_ac + in.aim_point_height_50;  // 009C43F3
cin.range_low     = in.attack_distance_b4;                          // 009C43E3
cin.range_high    = in.planar_distance_bc;                          // 009C43DB
```

Both are now read from the image rather than inferred. It needs `src/game_hosts_units.cpp`, which
this stream has released, so it waits for a declared window.

## `usn04_goaway2.log`: the completion rule works, and the ditch survives it

| measure | `usn04_goaway` | `usn04_goaway2` |
| --- | --- | --- |
| arm ticks | 2118 | 2117 |
| transitions | 7 | **5** |
| goaway ticks | **1** | **43** |
| aimglide ticks | 42 | 0 |
| flyabove / turndown / aimdive | 159 / 71 / 318 | 158 / 71 / 318 |
| water contact | -1.12 m, 43.72 m/s | -0.36 m, 44.92 m/s |
| releases | 0 | 0 |

Reading `009C7F00` whole did what it should: **goaway now runs 43 ticks instead of 1**, the climb-out
arm bound beside it gets to act, and the state no longer ends on its first tick. The completion rule
was the fault and it is fixed.

The aircraft still ditches. 43 ticks is 3.9 s, and nothing recovers a dive-bomber that entered its
dive at 650 m with the target astern and spent 318 ticks descending. That is not a goaway fault;
it is the profile the aircraft arrived with.

## Three defects at one call site, `009C4401`

With the whole call now read, the host's binding was wrong in three separate ways, and they
compound:

| was | is | evidence |
| --- | --- | --- |
| `range_low = range_high = attack_distance_b4`, so `span` was identically 0 | `range_low` = `approach+B4h`, `range_high` = `approach+BCh`, the live planar range | `009C43E3` and `009C43DB`, the latter fed by `009C4311`/`009C4317` |
| `base_altitude = r.commanded_altitude_base` | `approach+ACh + approach+50h` | `009C43F3` |
| `plane_desired_speed_2b4 = commanded_throttle` | **nothing** - a census over `009C4220`-`009C447D` finds no `cmd+2B4h` write at all | the absence itself |

The first is the one that matters. `span = max(high - low, 0)` is the distance **still to close** -
about 9900 m at 11 km out, zero at the attack distance - so the bias `span * scale * class+518h`
is the glide slope. Forced to zero, the aircraft is commanded to the bare base from 11 km out and
descends immediately instead of gliding down as it closes. Every state this stream has bound was
compensating for a command that should never have been given.

The third is a consequence of the second's misnaming: `commanded_throttle` was never a throttle, so
wiring it to a desired speed invented a command the state does not issue. Renamed to
`descent_scale` here, with `kThrottleRatioLow`/`AtLow`/`RatioHigh`/`AtHigh` renamed to match, on
cc8-torpedo-descent's listing evidence: `009C43CD`'s result is `009FBA50`'s arg3 and then
`009FB800`'s arg2, the clamp on `t` in both of its arms.

**Baseline: everything from here is measured after `67e8ac821`** and cannot share a table with the
runs above. `local\usn04_span.log` is the first on the new side.

## Constant-width audit, prompted by the torpedo aim tick's `kPitchClampLo`

cc8-torpedo-descent found `00D21318` carried in a header as 0.05625 - the **double** at those eight
bytes - while both loads are four-byte, and the float is `0xBFB2B8C3` = -1.3962634 = -DEG(80). A
sign flip and a factor of twenty-five, from a constant that carried a bare address and no width.

`include/bsp/dive_bomb_task.hpp` was audited for the same pattern: constants whose comment gives an
address but no width word and no load site. Thirty-eight matched, of which **two are declared
`double`** - the only ones where a width error could flip a value rather than merely be untidy:

| constant | address | load | verdict |
| --- | --- | --- | --- |
| `kFlyAboveRollInBearing` = 1.600000023841858 | `00CE3D48` | `009C6790 FLD double ptr` | correct |
| `kRollHandOver` = 0.800000011920929 | `00CE3D40` | `009C45B3 FLD double ptr` | correct |

Both verified against the listing, and both now carry the width and the site. `00CE3D48` is the
example they cite - its float is -1.084202e-19 - so had this one been declared `float` the flyabove
roll-in would have compared a bearing against a denormal and fired on every tick.

The remaining thirty-six are declared `float` and are consistent with `MOVSS`/`FLD float ptr` loads
at the sites this stream has read, but they were **not** individually re-verified in this pass and
should not be read as audited. The rule worth carrying forward is the one their packet demonstrates:
a constant's comment gives the address **and** the width **and** a load site, because the address
alone does not determine the value.

## The width sweep completed, and a `1.0f` stand-in found by their second rule

### The sweep: 63 checked, 0 mismatched

The earlier audit verified two constants by hand and left thirty-six declared but unverified.
cc8-torpedo-descent's suggestion - script it and print both widths - closes that honestly. The
script reads eight bytes at each declared address out of the PE, computes the `float` and the
`double` there, and flags any line whose declared value does not match its declared width.

**Every constant in `include/bsp/dive_bomb_task.hpp` matches: 63 checked, 0 mismatched.** That is
the whole header, not just the thirty-eight bare-address ones, so the "not re-verified" caveat on
the earlier audit is now discharged rather than merely narrowed.

### `speed_ratio_41c`: a `1.0f` stand-in that is not inert

Their second rule - grep the block for `= 1.0f;` on anything named after a class offset, because a
harmless-looking stand-in is only harmless until it lands in a denominator - turns up one in the
dive-bomb block, `src/game_hosts_units.cpp:4198`:

```cpp
in.speed_ratio_41c = 1.0f;     // task+41Ch
```

It is **not** a denominator, so it does not blow up the way their `pitch_scale_188` did:

```cpp
// 009C8A1B-009C8A5E: task+4B0h = max(task+4B0h, tuning+4C4h * task+41Ch)
const float wanted = in.attack_distance * in.speed_ratio_41c;
```

But it is not inert either. It scales the in-range latch threshold `approach+B8h` directly, and the
census reports that threshold as exactly 1100.0 m - the bare `Pilot/DiveBomb/AttackDist` - which is
the value a 1.0 stand-in produces and tells us nothing about the real one. `task+41Ch` is a speed
ratio whose producer this stream has not read; if it is anything but 1.0, the latch closes at a
different range, the flyabove starts somewhere else, and every geometry number in the tables above
shifts with it.

Recorded as a labelled stand-in with its consequence rather than fixed: the fix needs `task+41Ch`'s
producer, which is a read this packet has not done.

## `tools/const_width_sweep.py`: a repo-wide constant-width sweep

Promoted from the scratchpad script to a tracked tool. It reads eight bytes at each declared image
address out of the PE, computes the `float` and the `double` there, and flags any
`inline constexpr float|double kName = VALUE;  // 00XXXXXX` whose declared value matches neither its
declared width. `--all` sweeps every header under `include/bsp`; `--full` prints a line per constant.

**697 constants in 1727 headers, 122 mismatched.** They fall into two very different classes and
should be routed differently.

### Class A, 102: the value matches the OTHER width

`kRampHalf declared float=0.5 but float=0 double=0.5` is the pattern - the address holds a qword and
the header declares `float`. This is exactly the `kPitchClampLo` family. It is **harmless if the load
is `FLD double ptr` and wrong if the load is four-byte**, and the sweep cannot tell which: only the
listing can. Every one needs its load site read before anything is changed.

### Class B, 20: the value matches NEITHER width

These are the ones worth reading first, because the declared value is not at that address in either
interpretation:

```
cruise_command.hpp:153/154/155/156   kCruiseHeading*Epsilon, kCruiseSpeedSettingInactive
gui_widget_scene.hpp:45/46           kGuiVisibleFactor, kGuiHiddenFactor
gun_bot_ticks.hpp:156/170            kGunBotFixedStep, kAAGunnerBotSpanAtSkillZero
hud_updates.hpp:334                  kHudMinimapXDivisor  declared 1024, double there is 0.000976562
pilot_controls.hpp:84                kPilotPitchHalfRange declared 0.5236, float there is 0.523599
ship_ai_attackmove_substates.hpp:408 kAttackMoveLeadScaleNear declared 1, float there is 0.174533
ship_ai_goal_vector.hpp:93           kShipAiGoalKeepLengthSq
ship_ai_nav_block_ctor.hpp:37        kShipAiNavBlockThrottleFull
ship_ai_obstacle_tables.hpp:159      kShipAiDangerClearanceMin
submarine_model.hpp:218              kSubCrushTickSeconds
unit_commanded_speed.hpp:113         kDirectorWeaponTargetAbandonDistanceSq
unit_controller.hpp:270              kUnitEffectGateAstern
unit_damage.hpp:49                   kUnitInvincibleOff
unit_death_sink.hpp:70               kWreckAnchorLateralDivisor declared 2, double there is 2.5
world_entity_update.hpp:83           kMatrixInterpolatorPhaseCeiling
```

Two are worth naming as likely real: `kHudMinimapXDivisor` declares 1024 where the address holds
**1/1024** as a double - the reciprocal, which is what a divide-by-multiply would store - and
`kWreckAnchorLateralDivisor` declares 2 where the double is **2.5**.

And one is a false positive worth recording so the tool is not over-trusted:
`kPilotPitchHalfRange` declares 0.5236 against a float of 0.523599, a four-significant-figure
declaration rather than a width error. Several of the "declared 1 or 0, address holds garbage" rows
are likely constants whose comment carries a **code** address (a load site) rather than the data
address, which the sweep cannot distinguish.

**Nothing outside this stream's own headers has been changed.** `include/bsp/dive_bomb_task.hpp` is
clean at 63/0. The rest is listed for routing to its owners, with the caveat that a Class A row is
not a defect until its load width is read and a Class B row may be a site address or a rounded
declaration.

## `--load-sites`: Class A split, and the danger list is four rows

The sweep's Class A covered two opposite cases. `kPitchClampLo` itself printed as Class A - declared
`float` 0.05625, the double at the address 0.05625, the float -1.396 - so "matches the other width"
hid both the harmless case (every load is 8 bytes; the header's C++ type is merely narrower than the
image's) and the catastrophic one (a load is 4 bytes; the declared VALUE is simply wrong).

`--load-sites` decides it mechanically: for each mismatched constant it finds every instruction with
an absolute `[disp32]` operand naming that address and reports the operand width.

### The scan had to be built the way the project's own notes prescribe

A linear Capstone sweep from the `.text` start found **none** of the three loads this stream had read
by hand - `009C6790`, `009C45B3`, `009C43C4` - although it reported 3, 1 and 3 sites at those
addresses. It desyncs on inline data and every "site" it had was an artefact. Disassembling from each
**known function start** up to the next, out of `local/bsp_index.sqlite`, finds all three at the
right widths and raises the site counts to 15, 56 and 168. The tool refuses to run without that
index rather than silently falling back.

### The result

**697 constants, 122 mismatched: 101 A-harmless, 1 A-WRONG, 4 Class B, 16 unreferenced by the scan.**

So the overwhelming majority are a narrow C++ type over an 8-byte image value - real, but cosmetic.
The danger list is four rows:

| class | header:line | constant | declared | loads read |
| --- | --- | --- | --- | --- |
| **A-WRONG** | `torpedo_aim_tick.hpp:77` | `kPitchClampLo` | `float` 0.05625 | **m32 -> -1.39626** |
| B | `hud_updates.hpp:334` | `kHudMinimapXDivisor` | `float` 1024 | m64 -> 0.000976562 |
| B | `ship_ai_attackmove_substates.hpp:408` | `kAttackMoveLeadScaleNear` | `float` 1 | **m32 -> 0.174533** |
| B | `unit_death_sink.hpp:70` | `kWreckAnchorLateralDivisor` | `double` 2 | m64 -> 2.5 |
| B | `pilot_controls.hpp:84` | `kPilotPitchHalfRange` | `float` 0.5236 | m32 -> 0.523599 |

The last is the false positive predicted earlier and now confirmed by its loads: a
four-significant-figure declaration, not a width error. The other three are real, and two are
recognisable at sight - `0.174533` is `DEG(10)` declared as 1, and `0.000976562` is `1/1024`, the
reciprocal a divide-by-multiply would store.

**The single A-WRONG is `kPitchClampLo`, which is the bug cc8-torpedo-descent found by hand.** This
tree still carries `0.05625f`, so the tool found it blind, in a tree where it was still live, having
been told nothing about it - and found no other constant in 697 that is wrong in the same way. That
is the validation the sweep needed; a tool that found nothing would have proved nothing.

Sixteen rows are unreferenced by the scan and are reported as their own class, not as safe: an
address with no referencing instruction may simply be one the function-start sweep did not reach.

Nothing outside this stream's headers was changed. The full table, including all 101 A-harmless
rows with their sites, is written to `local/output/const_load_widths.txt`.

## Correction: `plane_drop_angle` is NOT zero, and the span fix's inertness is still unexplained

I proposed that the `009FBA50` span fix came out inert because `class_gain = tan(plane_drop_angle)`
is zero for this class, and then nearly confirmed it from a bad search. `Select-String -List` returns
the **first match per file**, so a search for `DropAngle` over `vehicleclasses.lua` returned exactly
one row - a `["Type"] = "Submarine"` class - and I read that as "no aircraft class has DropAngle".

There are **74** of them (my first count of 176 summed a truncated grouping and is retracted). The counts by value start 33 at 0.698132, 9 at 0.383972, 9 at 0.523599,
and the torpedo stream's Mavs report 0.4014, which is one of the others. So aircraft do carry a drop
angle, `plane_drop_angle` is very probably non-zero for the dive bomber too, and `class_gain` is not
the explanation.

That is the vacuous-negative trap this project's own notes name - an empty or near-empty search
result proves nothing until the pattern is known to occur - and I walked into it while holding a
tool built specifically to stop people trusting unverified readings.

**So the span fix's inertness is open, not explained.** What is established: `usn04_span.log` is
identical to `usn04_goaway2.log` to the digit, so correcting the range pair, the composed base and
the phantom `cmd+2B4h` write changed nothing observable. The cheap next step is instrumentation
rather than inference - log `span`, `class_gain`, `scale` and `c.clamped_altitude` from the attackrun
tick for one run, and see which term is dead - and that is one build and one run, against a guess
that has already been wrong once.

### Class B row 1 settled: the address was wrong, not the value

`ship_ai_attackmove_substates.hpp:408` `kAttackMoveLeadScaleNear` declared 1.0 against an m32 load
of 0.174533. The lead's hypothesis was right: an interpolation's `x0` = DEG(10) cited on the line of
its `y0` = 1.0.

Three pieces of evidence, none of them the value:

* `00CE3990` is already cited **correctly** two lines down, on `kAttackMoveLeadScaleNearAngle =
  0.17453293f` - the same address, the same value, the right name.
* The `Far` pair is internally consistent - `kAttackMoveLeadScaleFar = 0.5f // 00CE3800` is a true
  address-value pair - so only the `Near` line is off.
* **The four instructions that load `00CE3990` are in `FUN_00424730`,
  `BSP_Plane_HandleStateMessageKinds`, `BSP_TurningGun_StepAim` and one undefined body.** None is
  ship-AI attackmove code, so the region this header documents never loads that address at all.

So the comment is corrected and **the value is untouched**: no behaviour change, and no USN02
before/after needed, which is the outcome the "value or address?" question exists to reach. The
header now sweeps 47/0.

### Class B rows 2 and 3: what the sweep proves, and the one question left in each

Both need their own consumer's site before anything is changed, and the sweep has already narrowed
each to a single question.

**`hud_updates.hpp:334` `kHudMinimapXDivisor`**, declared 1024, m64 loads read 1/1024. Its sibling
settles the shape: `kHudMinimapYDivisor` declares 768 and the m64 there **is** 768, loaded by
`005411CA FDIV` and `0054121A FDIV` - while the X constant is loaded by `005411E0 FMUL` and
`00541230 FMUL`, adjacent sites in the same function. So the image **divides by 768 and multiplies
by 1/1024**, and the X row is wrong in name, value and use together. The open question is not what
the image does but where the consumer lives: making constant and use agree means editing a `src/`
file this stream does not hold.

**`unit_death_sink.hpp:70` `kWreckAnchorLateralDivisor`**, declared `double` 2, m64 loads read 2.5.
Its four sites are `004343DD`, `0067CFCD`, `007C5DAE`, `008250F3`, none of them obviously
death-sink code - so this may be the same fault as row 1, a correct value beside a wrong address,
rather than a wrong value. Deciding it needs the wreck-anchor site itself, and changing the value on
the assumption would move the sink geometry on an inference of exactly the kind row 1 disproved.

`kPilotPitchHalfRange` is recorded in the tool's own doc as a **confirmed false positive**: declared
0.5236 against an m32 0.523599, a four-significant-figure declaration. The 101 A-harmless rows are
left alone.

### Class B rows 2 and 3 settled, and a retraction on row 3's diagnosis

**Row 2, `kHudMinimapXDivisor`** - corrected to the image's form. The two axes are not symmetrical:
`00CEDAE8` holds the qword 1/1024 and every site **multiplies** by it (`005411E0` FMUL,
`00541230` FMUL, `005C1C68` FMUL), while `00CE42B0` holds the qword 768 and its sites **divide**
(`005411CA` FDIV, `0054121A` FDIV) - adjacent instructions in one function. The constant is now
`kHudMinimapXScale = 0.0009765625` and both consumers multiply, at `src/hud_minimap.cpp:100` and
`src/hud_updates.cpp:419`. As the lead noted, this is fidelity of form and not a behaviour bug:
dividing by 1024 and multiplying by 1/1024 agree to the last bit, both being exact powers of two.

**Row 3, `kWreckAnchorLateralDivisor`** - corrected 2.0 to 2.5, and **my earlier diagnosis of it was
wrong**. I wrote that its four sites were "none of them obviously death-sink code" and that it might
be row 1 again. One of them, `008250F3`, is squarely inside this header's own documented region
`008250F0..008251CD` - I read the list and did not check it against the region printed six lines
above the constant. The region loads the address twice, `008250F3` and `0082515B`, both
`FLD double ptr`, feeding the FDIV pairs at `00825116`/`00825131` and `00825161`/`00825179`. The
qword there is `00 00 00 00 00 00 04 40` = 2.5. So the address is right and the value was simply
wrong - the opposite of row 1.

This one **does** change behaviour: it widens the wreck anchor's lateral divisor by a quarter. No
before/after run was taken for it, and that is recorded rather than glossed.

### The repo after all three

`checked 696, mismatched 119: A-WRONG 1, B 1, A-unreferenced 1, B-unreferenced 15, A-harmless 101`

The remaining A-WRONG is `kPitchClampLo`, already fixed on `agent/cc8-plane-squadron` and not yet in
this tree. The remaining B is `kPilotPitchHalfRange`, the confirmed four-significant-figure false
positive. So every real row the sweep found is now either fixed or fixed elsewhere.

## `007F0280`: three corrections from the probe survey, and my stand-in is a hole

cc8-torpedo-descent's survey (`docs/BOT_PROBE_007F0280.md`, body **not** read and marked so) carries
three things that correct this doc or the brief:

* **Eighteen callers, not sixteen**, exhaustive over rel32 - and `009FD570` is **not** one of them,
  although it is reached through the same `009D0C10` geometry.
* **`RET 0x18` at `007F0B1F` says SIX stack arguments**, against `docs/BOT_TASK_STATES.md` row 356's
  five. Checklist rule 7: the count is the cleanup, never the pushes anyone listed. The row is the
  thing to doubt.
* **The extent triple is per-caller and sometimes computed.** This stream passes 80, 60, 120 with a
  final **1** (`009C4258`, `009C4268`, `009C4287`); the torpedo goaway passes 60, 50, 90 with a
  final **0**; the aim tick passes `{72t, min(0.7*72t, 150), 1.5*72t}`. So a pure function that
  hard-codes the triple would be wrong for fifteen of eighteen sites, and that final argument is a
  per-caller **mode** - this stream's 1 against the torpedo states' 0 is not noise.

### The part that corrects this stream, and it is not a quibble

Their zero stand-in and this one are **not the same kind of thing**, and their "inert is faithful"
does not cover mine.

At `009D0CBC` the caller zeroes the three out-slots itself immediately before the call
(`009D0C96`-`009D0CAA`, XORPS/MOVSS) and the only use is a strict sign test on `-p[0]*p[1]*p[2]`, so
a no-hit answer cannot fire it: zero there is exactly what the image would compute, and that
stand-in should stay.

Here at `009C42B8` the result feeds `lateral_offset_20 = -sampler_result * ...`, so zero is **not** a
no-hit answer - it is a claim that the probe always returns zero, and it is precisely why the run-in
flies straight at its target instead of weaving. The label on it has been honest about the
behaviour ("the run-in flies straight rather than weaving") but described it as a contract, which
undersells it: it is a hole with a known shape, and it is upstream of the whole approach geometry
this stream has been measuring.

That matters for the inert-span question now under instrumentation. The run-in's commanded heading
is `bearing + lateral_offset_20` (`009C42DC`-`009C4305`), so with the offset pinned at zero the
approach path is not the image's, whatever `009FBA50` computes for its altitude. Both are upstream
of the dive, and only one of them is currently being measured.

### Checked rather than inherited: the offset does reach the heading, and stays zero

cc8-torpedo-descent was careful to say their "yours is a hole" was an argument about the shape of a
zero substitution and **not** a reading of this stream's heading chain, which they had not done, and
asked for it to be checked here. Checked, at `src/dive_bomb_task.cpp:395-419`:

```cpp
out.lateral_offset_20 = in.lateral_offset_20;          // 399, carried on entry
...
} else {                                               // the re-roll arm only
    out.lateral_offset_20 = -in.sampler_result * kLateralOffsetScale;   // 412
}
// 009C42DC-009C4305, run on BOTH arms:
out.commanded_heading_2c0 =
    wrapped_angle_add_00438aa0(in.target_bearing_c0, out.lateral_offset_20);  // 418
```

Two facts, and they compound rather than cancel:

* the offset reaches the commanded heading on **every** path, not some - the heading line is outside
  the `if`, and the comment at `009C42DC` already said "run on both arms";
* it is only **recomputed** on the re-roll arm, and it is **carried** otherwise, so a zero written
  once persists until the next re-roll writes zero again.

The census reports `rerolls=153` over the run-in, so the substitution pins the offset to zero 153
times and it is zero in between. The commanded heading is therefore the bare bearing to the target
for the whole approach - the aircraft flies straight at the target and never weaves - which is what
the label always claimed behaviourally and is now established from this side rather than argued from
the shape of the substitution.

So their conclusion stands and is now independently confirmed, which is the right standing for it:
they were right not to claim a reading they had not made, and the check was three lines away.

## `007F0280` scoped: the six-argument ABI confirmed from both ends

Ownership moved to this stream, because the zero substitution is a proof at the torpedo goaway site
and a hole here. Before reading the body, its ABI is confirmed independently of the handover note,
from the prologue and the epilogue:

```
007f0280  PUSH -1 / PUSH 0xc8f35b / MOV EAX,FS:[0] / PUSH EAX   ; SEH frame
007f0295  SUB  ESP,0x104
007f029b  PUSH EBP / PUSH ESI                                    ; 280 bytes pushed
007f029d  MOV  ESI,dword ptr [ESP + 0x11c]                       ; = [ESP+284] = arg0
...
007f0b19  ADD  ESP,0x110
007f0b1f  RET  0x18
```

`RET 0x18` is **24 bytes, six stack arguments**, and the prologue agrees: 280 bytes pushed puts the
return address at `[ESP+280]` and the first argument at `[ESP+284]` = `[ESP+11Ch]`, which is exactly
what `007F029D` reads. Two independent ends of the frame giving the same count is worth more than
either alone, and it stands against `docs/BOT_TASK_STATES.md` row 356's five - that row is the thing
to doubt, and it is not this stream's to edit.

Body `007F0280`-`007F0B21`, 2209 bytes, SEH-registered, 0x110 of frame. Per the project's own notes
that is a body to script rather than decompile, with ESP anchored on the SEH state stores and
back-propagated - the decompiler's frame reasoning is least trustworthy exactly where an SEH
registration sits, which the handover doc also says of its own contents.

Queued behind the `usn04_terms` run and the `task+41Ch` binding. `009FD570`, which `009C47D0` also
calls, belongs to cc8-flyto-solver and will not be transcribed here.

## `007F0280`'s frame walked, and the argument shape from both sides

The body is scripted rather than decompiled, with the depth walked forward from the prologue and
every `[ESP+n]` access resolved to a corrected slot. The walk validates against the hand
computation: `007F029D`'s `[ESP+11Ch]` resolves to slot -4, which is `entry+4`, the first stacked
argument, exactly as the `RET 18h` arithmetic predicted.

### The six slots, from inside the callee

| arg | slot | first read | width |
| --- | --- | --- | --- |
| 0 | -4 | `007F029D` `[ESP+11Ch]` | m32 |
| 1 | -8 | `007F02E9` `[ESP+124h]` | m32 |
| 2 | -12 | `007F02B8` `[ESP+128h]` | m32 |
| 3 | -16 | `007F02CD` `[ESP+12Ch]` | m32 |
| 4 | -20 | `007F038D` `[ESP+138h]` | **m8, a CMP** |
| 5 | -24 | - | no direct `[ESP+n]` read |

The differing displacements for a uniform 4-byte argument list are the depth changing between the
reads, which is the whole reason for anchoring rather than grepping literals: `[ESP+124h]` and
`[ESP+128h]` are consecutive arguments read four bytes apart in the frame and four apart in the
displacement only because the depth happened to be equal there.

Two things worth having before the body: **arg4 is read as a byte**, by a `CMP`, which is the shape
of a mode or flag rather than a float; and **arg5 is never read through `[ESP+n]`** in the whole
2209 bytes, so it is either taken through a pointer or genuinely unused - a question for the body,
and one the caller survey could not have answered.

### And from the caller side, at this stream's own site

```
009c4260  PUSH 0x1                         ; pushed FIRST, so the LAST argument
009c4276  PUSH ECX   ; LEA ECX,[ESP+0x14]  ; an out-pointer
009c4280  PUSH EDX   ; LEA EDX,[ESP+0x30]  ; an out-pointer
009c4293  PUSH ECX   ; LEA ECX,[ESP+0x40]  ; an out-pointer
009c4262/009c4281/009c4294  MOVSS [ESP+20h]/[ESP+2Ch]/[ESP+34h]   ; the 80, 60, 120
```

So the three extents are **stored into the argument window, not pushed**, while the three
out-pointers and the mode are pushed - which is why a reader counting pushes gets a different answer
from a reader counting the cleanup, and why row 356 had five. The mapping of which push lands in
which of the six slots is the next step and belongs to the body read, not to inference from this
side.

## `usn04_terms.log`: every input is alive, the output is pinned

Eight samples, one per 120 attackrun ticks, identical on all three aircraft
(`movieval`, `|.-2`, `|.-3`):

```
span / gain / scale / base / clamped
9715 / 0.577 / 0.963 / 1000 / 1450
8930 / 0.577 / 0.965 / 1000 / 1450
8100 / 0.577 / 0.965 / 1000 / 1450
7268 / 0.577 / 0.965 / 1000 / 1450
6435 / 0.577 / 0.965 / 1000 / 1450
5602 / 0.577 / 0.965 / 1000 / 1450
4769 / 0.577 / 0.965 / 1000 / 1450
3936 / 0.577 / 0.965 / 1000 / 1450
```

| term | over ticks 120-960 | verdict |
| --- | --- | --- |
| `span` | 9715 -> 3936, monotone | **alive** - the fix works; it is the distance still to close |
| `class_gain` | 0.577 throughout | **alive**, and `atan(0.577)` = 30 deg, so this class's `DropAngle` is 0.5236 |
| `scale` | 0.963 then 0.965 | alive, near-constant |
| `base_altitude` | 1000 throughout | constant by construction: `+ACh` 1000 + `+50h` 0 |
| `clamped_altitude` | **1450 throughout** | **DEAD** |

**The dead term is the output, not an input.** The range pair fix did exactly what it was supposed
to - span is live and falling - and the commanded altitude still never moves, because
`base + span * scale * gain` is 1000 + 9715 x 0.965 x 0.577 = **6410 m** at the first sample and
still 3190 m at the last, so the result saturates on its ceiling at 1450 on every single tick of the
run-in. That is why correcting the arguments was byte-identical: the command was pinned before and
is pinned after.

So the bias is roughly four times too large, and the clamp has been hiding it. The next question is
the bias's own form - `span * scale * class+518h` - and in particular whether `class+518h` is
`tan(DropAngle)` at all, since 0.577 against a 9.7 km span is what produces a 5.4 km bias.

### What this log cannot clear

It cannot clear the altitude chain as a whole. It shows which term is pinned and that the inputs
reaching `009FBA50` are live; it says nothing about whether `009FB800` downstream, or the pitch arm
after it, does the right thing with a saturated altitude. And the approach **path** is still not the
image's - the lateral offset is pinned at zero across all 153 re-rolls - so these numbers describe a
run-in flying straight at its target rather than weaving.

### `approach+A8h`, still a substituted 350.0

It enters **none** of the five. `base` is `+ACh + +50h`; `span` is `+BCh - +B4h`; `gain` is
`tan(plane_drop_angle)` off the Lua row; `scale` is the `009C43CD` interpolation over the height
margin against the clamped distance. The substituted 350.0 reaches the release floor and the
`009C4045` dive-entry height, not this chain.

### Correction to the saturation reading above

"The bias is roughly four times too large and the clamp has been hiding it" is **withdrawn**.
`base + distance x tan(angle)` clamped to a ceiling **is** a glide-slope law, and at 9.7 km it is
supposed to sit on the ceiling: it only comes off when `span x scale x gain < 450`, i.e. inside
about 810 m of span, and `approach+B8h` = 1100 ends the attack run outside that. Given the same
inputs the image may fly the whole run-in at the ceiling too. **The law is not shown wrong** - what
is shown is that in this regime it cannot explain the ditch.

Both settling facts are named:

* the **1450** is the authored `Dynamics/Ceiling` = **1500** from this installation's
  `scripts/datatables/planeglobals.lua` ("ez a plafon. ennyi meter folott minden repulogep atesik")
  with a 50 m margin in the clamp - a separate authored altitude, **not** `base + 450`. The exact
  form of that 50 m margin is the one detail not read.
* **`class+518h` is written as `tan(DropAngle)`**: `include/bsp/bot_task_states.hpp:330` records
  `gain = tan(desc+1F0h DropAngle)` at the store site `007C4A3F`/`007C4A44`. The measured 0.577 is
  `tan(30 deg)`, so this class's authored `DropAngle` is 0.5236.

`docs/HANDOFF_DIVE_BOMB_PROBE.md` carries this stream's state for a cold reader.

---

# `009C4F80`, the heading the aim states steer on, and why the bomber ditched

Packet `cc8_dive_geometry`, owner `agent/cc8-dive-bomb`. Listing read plus one instrumented USN04
run per side, same binary apart from the change under test.

## 1. The turndown is a split-S, and this host already flew it

`009C44F0` was read whole (153 instructions) and every constant taken at the width of the
instruction that loads it. With `a = |wrap(pose+C68h)|`, the bank magnitude in `[0, pi]`:

| range | what it writes | evidence |
| --- | --- | --- |
| `state+1Ch` clear, `a < 0.8` | `cmd+290h = InterpolateClamped(30deg, 1, 0, 0, pi-a) * state+18h`, `cmd+294h = 1`, `cmd+2CCh = 0` - a roll **rate** | `009C45B9` FCOMIP / `009C45BB` JBE; `009C45EA`-`009C462F` |
| `state+1Ch` clear, `a >= 0.8` | `cmd+2C4h = pi` (`00D7A264`), `cmd+2CCh = 1` - a roll **angle** demand of 180 degrees | `009C4637`-`009C464E` |
| either arm | latch `state+1Ch` once `a > 2.618` (150 deg, `00D1FED0`) | `009C4654` COMISS / `009C465B` JBE |
| either arm, on pitch | `|pose+C64h| < 0.3491` (20 deg): `cmd+29Ch = 0`, `cmd+2A0h = 1`, `cmd+2D0h = 0`; otherwise `cmd+2BCh = 0`, `cmd+2D0h = 2` | `009C4660`-`009C46AA` |
| `state+1Ch` set | hold `cmd+2C4h = pi`, `cmd+2CCh = 1`, and pull: `cmd+29Ch = InterpolateClamped(30deg, 0, 3deg, 1, pi-a)`, `cmd+2A0h = 1`, `cmd+2D0h = 0` | `009C46C9`-`009C4736` |

Roll to inverted, then pull through: a split-S. `009C7EA0` agrees - its second arm ends the state at
`pose+C64h < -1.0` only when `|pose+C68h| > 2.356`, i.e. **while the aircraft is still inverted**
(disassembled at `009C7EA0`-`009C7EFB`; `009C7EDB` `JA`, `009C7EE8` `JBE`, `009C7EF1` `JA`).

The per-tick trace (`local\usn04_geo1.log`, ticks 1686-1756) shows this host flying exactly that:

```
state      tick  pitch_c64 bank_c68 heading_c6c    alt    range  bearing  roll  pitch
turndown   1686    -0.0001  -0.2792  -3.0193     729.6      3.8   2.9100 -0.800  0.000
turndown   1710     0.1119   1.3521  -2.6239     730.7    168.1   2.9274 -1.000  0.000
turndown   1722     0.1226   2.6022  -2.6233     737.2    247.5   2.9876 -0.606  0.000
turndown   1740    -0.2827   3.0805  -2.6783     732.1    365.5   3.0836 -0.071  0.981
turndown   1756    -0.9795   3.1252  -2.7049     658.5    462.8   3.1195 -0.016  1.000
```

**So the 470 m and the "target 178.9 degrees behind" are not the defect.** They are what a split-S
looks like halfway through: the roll phase flies level and straight, and the heading does not
reverse until the pull passes the vertical, which `009C7EA0` deliberately does not wait for.

## 2. `009C4F80`: the aim states do not steer on `pose+C6Ch`

`float __thiscall(state)`, body `009C4F80`-`009C5177`. Two callers, exhaustive over rel32
(`tools/callsite_census.py`): `009C51A8` in the aimglide tick and `009C5935` in the aimdive tick.
Its result is arg0 of the `00438B10` at `009C5AA3` and of the one at `009C5AF1`, with the bearing to
the target as arg1 - so **the roll interpolant's x is `(aim heading - bearing)`**, not
`(bearing - heading)`. Slots, not pushes: `009C5A95 SUB ESP,8`, `009C5A98 FSTP [ESP+4]` is the
bearing, `009C5A9C FLD [ESP+18h]` reads the slot `009C593A` wrote and `009C5AA0 FSTP [ESP]` makes it
arg0.

The routine itself, with `a = |pose+C68h|` folded at `009C4F90`-`009C4FB1`:

* **`pose+C64h > -0.6981`** (`00CE7D1C`, -40 degrees; `009C4FBF` COMISS, `009C4FC6` `76` JBE):
  `h = pose->vtable[50h]()`, the raw heading - the same virtual
  `unit_set_heading_target_00811960` calls `heading_virtual_0050`. Then `009C4FDF` FCOMIP against
  the pi/2 at `00CE3830` with `009C4FE3` `76` JBE: **a bank strictly past pi/2 adds the pi at
  `00D7A264`** through `00438AA0` (`009C4FE9`-`009C4FFD`). Otherwise `009C516C` returns `h` as it is.
* **`pose+C64h <= -0.6981`**: the Euler heading is ill-conditioned, so `009C504E` transforms
  `(0, 100, 0)` (`00CE3D08`) by the pose at `+CCh` through `0042D0D0` and takes
  `pi/2 - atan2(out.z, out.x)`, wrapped into `[0, 2pi)` by `009C5068`-`009C507C`. `0042D0D0` is the
  row-vector form - `out.x = in.x*m[0] + in.y*m[4] + in.z*m[8]`, read off `0042D0F2`-`0042D116` -
  so `(0,100,0)` selects **pose row 1, the body +Y axis**, and the 100 is irrelevant to an `atan2`.

The two arms agree, which is what makes the reading safe: rolling 180 degrees about the forward axis
negates body +Y, so the bearing of its horizontal projection is the heading plus pi exactly when the
aircraft is inverted. **`009C4F80` returns the heading of the lift vector - the direction the
aircraft turns toward - not the direction its nose points.**

## 3. Why that is the ditch

The turndown hands the aim states an inverted aircraft by construction. So for the whole dive
`009C4F80` differs from `pose+C6Ch` by pi, and this host had **both** the source and the subtraction
order wrong: it passed `wrapped_angle_subtract_00438B10(bearing_c0, heading_c6c)`.

At the first aimdive tick (1757) the host's value is `3.1210`; the image's is
`wrap(H + pi - B) = +0.0206` rad. The default band is 0.4 rad (`00CE7804`), so the image's roll stick
is about `-0.05` - hold the bank, keep pulling - while this host's saturated at the far end of
`InterpolateClamped(-0.4, 1, 0.4, -1, x)` and flipped sign every time the error crossed pi. The
trace shows the result: bank `3.13 -> -2.64 -> 3.10 -> 2.47 -> -2.79 -> -1.44`, range `468 -> 1037`,
altitude `651 -> 273`. A barrel roll into the sea.

`src/dive_bomb_task.cpp` now carries `dive_bomb_aim_heading_009c4f80` and the aimdive host passes
`wrapped_angle_subtract_00438b10(aim_heading, db_bearing_c0)`.

**SUBSTITUTION, labelled:** the shallow arm's `pose->vtable[50h]` is stood in for by the cached
Euler heading `pose+C6Ch` that `007C1900` writes. The steep arm is exact - `unit.motion.pose_row1`
is the same row `plane_attitude_angles_007c1900` reads `m[8]`/`m[9]`/`m[10]` out of.

## 4. What this does NOT fix, and is not claimed to

The pitch half of the aimdive. `009C5BD4` COMISS `pose+C64h` against `00CEC728` = `-0.5236`
(-30 degrees) with `009C5BDB` `JA`: **above 30 degrees nose-down the image writes
`cmd+29Ch = -1.0` outright** at `009C5CEF`, and only below that does the aim-error arm at `009C5C9F`
run. This host implements neither the gate nor the aim error - `db_aim_error_last` stands in for the
unread block `009C5B01`-`009C5C9B`, and it is what put the pitch command at `-1.0` for all 318
aimdive ticks in the before run, one tick after the turndown had it at `+1.0`.

That block, and the wide roll arm's second bearing (the one drawn against the latched
`approach+D8h/+E0h` at `009C594C`, which this host does not compute separately), are the next thing
to read.

---

# `009C5BD4`, the -30 degree pitch gate, and what `[ESP+5Ch]` carries past it

Packet `cc8_dive_aim_error`, owner `agent/cc8-dive-bomb`. Everything in sections 1-3 below is read
from the image on disk; section 4 is the run.

## 1. The x87 question `docs/HANDOFF_DIVE_BOMB_AIM_ERROR.md` posed is answered: there is no divergence

Two independent scripted walks now agree. `local/x87_walk.py` (the retiring packet's) and
`local/x87trace.py` with `local/t.ps1` (this one's, written without reading the other) both
propagate x87 depth and ESP over every CFG edge of `009C58D0`-`009C6161` from the entry, and both
report **no depth conflict at any join in the whole body**, ending at `009C615F RET 4` with an empty
x87 stack. The hand trace in the handoff had dropped `009C5B99 FSTP ST0`.

Closing the walk needed five callee effects, each taken from the callee's own tail rather than
assumed:

| callee | x87 | ESP | evidence |
| --- | --- | --- | --- |
| `00419010` InterpolateClamped | +1 | +14h | `00419030 c2 14 00` RET 14h, `0041902C FLD [ESP+8]` |
| `00438AA0` / `00438B10` | +1 | +8 | `00438ADC` / `00438B4C` `c2 08 00` |
| `009C4F80` aim heading | +1 | 0 | `009C5175 c3`, float left in ST0 |
| `00BF701A` atan2 | **-1** | 0 | `FLD dz; FLD dx; CALL; FSTP` at `009C5A5E`-`009C5A6B`: two in, one out |
| `00BF7030` sqrt | 0 | 0 | `009C5A40`'s call path and the `009C5A53` zero path rejoin at equal depth |
| `[EDX+38h]` at `009C5EDE` | **+1** | 0 | the only assignment that closes the join at `009C5F19` |

The two things the walk settles about the block itself:

* **The `00438AA0(aim heading, pi)` at `009C5BB1` is dead.** It is pushed at depth 0 and popped by
  `009C5BBA FSTP ST0` on the path that reaches it; it is never stored. Its argument is the base
  frame's `[ESP+10h]` - `009C5BAA FLD [ESP+18h]` sits at `fb=0x60`, after `009C5BA3 SUB ESP,8`.
  So the `|default roll error| > pi/2 && range > height*0.1` test at `009C5B7D`-`009C5B9B` has no
  effect on any state, and neither arm of it needs reconstructing. MSVC cannot elide a call to an
  externally linked function whose result is dead, which is why it is still in the image.
* **The `FSIN` at `009C5BC0` is dead too.** It writes `[ESP+28h]`, and `009C5C14` overwrites that
  slot with `cos(wide roll error) * [ESP+5Ch]` before any read.

## 2. The gate reads the other way round from the handoff's summary

`009C5BCC MOVSS XMM0,[ECX+C64h]`, `009C5BD4 COMISS XMM0,[00CEC728]`, `009C5BDB 0f 87` **JA** to
`009C5CEF`. `00CEC728` is a float and is **-0.5235987901687622** (`00CEC724` next door is the
positive one). `pose+C64h` is negative nose-down, so the jump is taken when the pitch is *above*
-30 degrees: **the SHALLOW arm gets `cmd+29Ch = -1.0`, and the aim error is computed only when the
aircraft is already steeper than 30 degrees nose-down.**

`docs/HANDOFF_DIVE_BOMB_AIM_ERROR.md` section 3 and the final section of this document both said
"above 30 degrees nose-down the image writes `cmd+29Ch = -1.0` outright". **That is withdrawn**; the
branch byte is the same, the sense of the constant is not.

`-1.0` is the push end of the stick: `009C46C9`-`009C4736` drives `cmd+29Ch` to **+1** as the
turndown's bank approaches inverted, and that arm is the split-S pull-through.

## 3. `[ESP+5Ch]` on the gated arm is the latched planar distance, not the aim error

The slot is the incoming `float dt` parameter home, reused. Enumerating **every** access to it over
the whole body (16 sites) settles what the two later readers see:

* last writer before the gate: `009C5A4D FSTP [ESP+5Ch]` (or `009C5A58 MOVSS` on the `1e-10` floor
  arm) - the second `sqrt`, over `[ESP+40h]`/`[ESP+48h]`, which `009C5959`/`009C596F` filled from the
  **latched** `approach+D8h`/`+E0h` point;
* every other writer between `009C5BDB` and `009C5D08` is inside the range the jump skips
  (`009C5C30`, `009C5C55`, `009C5C79` are InterpolateClamped argument spills; `009C5C9B` is the aim
  error itself);
* the readers are `009C5D08` (the roll band test) and `009C60C1` (the 25 m release window).

So on the shallow arm the image tests a **distance in metres** against the 25.0 at `00CE3880`, and
takes the wide roll band whenever that distance is positive. Reconstructed here as
`DiveBombAimDiveSteerInputs::planar_distance_slot_5c` and `DiveBombAimDiveSteerResult::error_slot_5c`
rather than hidden, because a host that keeps its aim error in a member and reads it unconditionally
gets a different release decision on every shallow tick.

Whether the original C++ intended this or left a local uninitialised on that path cannot be settled
from the image. The data flow can, and is what a faithful host has to reproduce.

## 4. The aim error block was already reconstructed, and this walk re-derives it unchanged

`dive_bomb_aim_error_009c5c9b` (`src/dive_bomb_task.cpp`) predates this packet. Walked independently
with the `SUB ESP,0x14` displacements resolved mechanically (`local/t.ps1` prints
`base [ESP+nn]` next to every ESP-relative operand), the block is:

```
x0 = approach+A8h + 100.0            ; 009C5BEE, 009C5BF7 FADD m64 [00D7A220] = 100.0
x1 = approach+ACh + approach+50h     ; 009C5C27-009C5C38
lead = InterpolateClamped(x0, 0.0, x1, row+5Ch, height)          ; 009C5C49
along = cos(wide roll error) * latched planar distance - lead    ; 009C5C04-009C5C14, 009C5C4E FSUBR
gain = InterpolateClamped(x0, 1.0, x1, row+60h, height)          ; 009C5C92
error = gain * along                                             ; 009C5C97, stored 009C5C9B
```

which is what that function already computes, term for term. The handoff's "unread block" framing is
withdrawn in the document itself.

## 5. Measured: the gate regulates the dive angle, and the abort is what ends it

`local\usn04_gate1.log` against `local\usn04_geo3.log`, same binary apart from the gate.

```
geo3 (no gate)                          gate1 (gate bound)
1788  pitch -0.766  cmd +1.000          1788  pitch -0.766  cmd +1.000
1794  pitch -0.504  cmd +1.000          1794  pitch -0.504  cmd -1.000   <- gate fires
1800  pitch -0.242  cmd +1.000          1800  pitch -0.401  cmd -1.000
1806  pitch  +0.02  cmd +1.000          1806  pitch -0.534  cmd +1.000   <- and releases
1808  pitch +0.107                      closest range 209.3 (geo3: 204.1)
```

Measure 1 and measure 2 of the handoff are met: `pitch_cmd` stops being a constant, and
`pose+C64h` stops walking back up past zero. It is a bang-bang regulator and it holds the dive at
the gate's own angle, about -0.52 rad. That is the authored dive angle of this class from the other
end as well: `class+518h` is `tan(DropAngle)` and its measured 0.577 is `tan(30 deg)`.

`releases` is still 0 and the state counts are unchanged (`aimdive=52`, then `aimglide=562`),
because what ends the dive is the abort, not the pitch: see section 6.

## 6. Two things the release still needs, and neither is the pitch

**Measured after the abort-height fix, and it changed no state count.** `local\usn04_abort1.log`
returns the same `states[aimdive=52 aimglide=562 flyabove=158 turndown=71 attackrun=1527]` and
`releases=0 bombs_spawned=0` as `gate1` and `geo3`, with no `plane water contact` line anywhere
(that census prints `plane water contact: unit=... alt=-0.36 ...` when an aircraft ditches, so its
absence in a log that ran to clean shutdown is the no-ditch evidence).

**What the successor should check first**, because it is what those three runs say together: the
counts are identical across three materially different pitch profiles - `geo3` climbed out of the
dive, `gate1` held it at 30 degrees, `abort1` did the same with a corrected abort geometry - and a
geometric abort cannot produce the same 52 ticks under all three. So what ends the aimdive in this
host is very probably **not** `009C5B43` at all. Read the aimdive -> aimglide transition owner
(`009C8650`) and establish whether it reads `state+19h`, and whether anything else clears it,
before attributing another tick count to the dive geometry.

**The abort's height input** was a range. Corrected in this packet (`7d5c667ec`) with the
derivation in that commit; with a range in both operands `009C5B3E` reduced to
`0.3*range + 150 > range`, an abort at any range under 214 m whatever the altitude, and both runs
lose the dive just inside that (204.1 m and 209.3 m). The run measuring the correction is
`local\usn04_abort1.log`.

**`[ESP+5Ch]` is not the current range, and this host's substitution for it is a HOLE.** The two
`approach->vtable[0]` calls differ their result against two different points, and the register that
picks them changes between them:

* `009C593E MOV EDI,[ESI+4]` makes EDI the approach, so `009C5950 FSUB [EDI+0D8h]` differences the
  aim point against `approach+D8h`. `009C405D`-`009C407D` in the constructor stores `unit+FCh`,
  `+100h`, `+104h` there: **the aircraft's own position at dive entry**, latched once.
* `009C5966 MOV EDI,[EBP+4]` then makes EDI the unit, so `009C598C FSUB [EDI+0FCh]` and
  `009C5999 FSUB [EDI+104h]` difference the aim point against the aircraft's **current** position.
  EDI is written five times before `009C5BEB` and this is the only reload between the two
  differences, which is what settles it.

So `[ESP+1Ch]` (first `sqrt`) is the live aircraft-to-aim-point range, and `[ESP+5Ch]` (second
`sqrt`, the one the aim error multiplies `cos` into) is the **dive-entry-point-to-aim-point range**,
a constant for the whole dive. Likewise `[ESP+18h]`, the wide roll error, is measured from the
latched entry point and `[ESP+24h]`, the default one, from the aircraft.

This host passes `db_planar_bc`, the live range, for both. That is why the measured aim error tracks
the range exactly (`range/error` = `433/433` in both runs) and never approaches the 25 m window.
Whether the image's own quantity ever does is **not established here**: with `D_entry` fixed at
472.6 m and the lead interpolating to 0 at low altitude, `cos(...) * 472.6 - lead` has no obvious
zero either, so either `approach+D8h` is re-latched by something outside `009C58D0` or the release
this mission needs is the aimglide's at `009C5777`, not this one. That is the next question, and it
is a reading question, not a tuning one.

**`approach+A8h` = 350.0 is a proof of the range and a hole in the draw.** The release's first gate
is `approach+A8h > pose+100h`, so it depends on it directly. `009C3F23` draws it uniformly, and this
installation's `scripts/datatables/robots.lua` SPNormal row authors
`DiveBombReleaseAlt = { 350, 450 }`; the host pins the low end and the difficulty index is
unmodelled. Pinning the low end is the conservative direction - it demands a lower aircraft before
releasing, so it can suppress a release the image would make, never cause one it would not.

## 7. The open pitch question is resolved, and it was the missing gate

The seam the handoff flagged - "a full push RAISING the nose because the aircraft is inverted" -
is **not** a frame mismatch. `009C58DE XOR EBX,EBX` is the **only** write to EBX in the whole body
(filtered over the complete listing; the two `POP EBX` are epilogues), so `009C5D1C MOV [EDI+2D0h],
EBX` writes **0**: the aimdive clears the pitch-hold mode and `cmd+29Ch` is a raw body-frame
elevator demand, with `009C5D15` setting `cmd+2A0h = 1`. This host does the same - every aimdive
row of the trace carries `mode_2d0 0` - so image and host apply it in the same frame.

What produced the symptom was the unbound gate. `009C5C9F`'s positive arm clamps at **+1.0**, and
the aim error is positive and large for the whole dive, so an upright aircraft got full pull and
flew out of its dive. The gate is exactly what replaces that: once shallower than 30 degrees
nose-down it overrides with `-1.0`, a push. `local\usn04_gate1.log` shows the override firing at
tick 1794 and the dive angle holding instead of climbing out. So the answer to "body frame, or is
the lead far larger than the substituted 70.0" is **neither**: no constant needed changing, and none
was changed.

## 8. Attribution of the commits in this stream

`9daf9dad5` (packet `cc8_dive_geometry`) staged `src/game_hosts_units.cpp` whole and so contains,
besides its own two hunks, the host-side half of this packet's gate binding near line 4897 - which
references `planar_distance_slot_5c`, a member added in `d0955a4cb`. That commit therefore does not
build on its own; **`d0955a4cb` is the first commit at which the gate binding is complete and the
branch builds.** `local\usn04_geo3.log` measures the bearing fix ONLY: its author verified at tick
1806 that the gate was not in the binary it ran. The history is left as it is.

The x87 walk and the `009C5AF1` argument order were each established twice, by two scripts written
independently (`local\x87_walk.py` and `local\x87trace.py`), which is why both are stated here
without hedging.

## `approach+D8h` is the predicted bomb impact point, rewritten every tick

**This withdraws three claims in this document.** In order of how wrong they were:

1. "`approach+D8h`/`+DCh`/`+E0h` are the aircraft's own world position, **latched once at task
   construction**" (the section that renamed `kAimPointX/Y/Z` to `kRunInOriginX/Y/Z`). Wrong: the
   constructor's store at `009C405D`-`009C407D` is one of **three** writers, and it is overwritten
   on the first approach-update tick.
2. The same section's "the **second** bearing, taken against the aircraft's live position, is the
   one the aim error uses". Wrong, and the correction in section 6 of the aim-error walk is right:
   the aim error uses the bearing taken against `+D8h`/`+E0h`.
3. Section 6's own "the aircraft's own position **at dive entry**, latched once, constant for the
   dive". Wrong in the same way. There is no latch to bind.

### The census

An `fstp`/`mov` scan for `[reg+0D8h]` over `009C3E00`-`009CA000` (the whole class) returns three
stores, not one:

| site | function | what it stores |
| --- | --- | --- |
| `009C4065` | `FUN_009C3EA0`, the constructor | `unit+FCh`/`+100h`/`+104h`, the raw position |
| `009C7D31` | `009C7A80`, the approach update | the same raw position |
| `009C7E13` | `009C7A80`, the approach update | the point below |

`009C7A80` runs on **every** arm tick, before the transition and before the state tick (section (1)
step 3), so the constructor's value survives exactly zero ticks. The two update arms are chosen at
`009C7D04`-`009C7D12`:

```
009c7d04  cmp byte ptr [esp + 0x38], 0     ; the `diving` argument
009c7d09  jne 0x9c7d71                     ; -> the second arm
009c7d0b  cmp byte ptr [esi + 0xd0], 0     ; approach+D0h, the in-range latch
009c7d12  jne 0x9c7d71                     ; -> the second arm
009c7d27  fld dword ptr [edi + 0xfc]       ; else the raw unit position
```

`[ESP+38h]` is the third argument, confirmed from both ends. The callee's frame is `SUB ESP,28h`
plus `PUSH ESI`/`PUSH EDI` (the epilogue at `009C7D64`-`009C7D6E` pops exactly those two and adds
`28h`), so `[ESP+38h]` is `entry+8`, the first of the two pushes; and at the call site `009C87DF
PUSH EDX` pushes the `diving` byte `009C87D2` had just set, before `PUSH ECX`/`FSTP [ESP]` puts
`dt` in the second slot. `RET 8` closes it. The four states `009C8794`-`009C87D7` calls `diving`
include `aimdive` and `aimglide`, so **during a dive the second arm always runs**.

### The second arm, `009C7D71`-`009C7E33`

```
h    = unit.y - aimPoint.y                      ; 009C7B49 stores it negated, 009C7D88 negates it
                                                ;   back ([00D7A208] is -0.0f, so this is a negate,
                                                ;   not a subtraction from a constant)
tf   = 007BCC80(unit, h) + 0.1                  ; 009C7D94, then the qword 0.1 at 00D7A3A0
v    = unit->vtable[+34h]()                     ; 009C7DB0, through [ESI+4] - the UNIT's vtable
+D8h = unit.x + tf * v.x                        ; 009C7DDC / 009C7DC8 / 009C7E13
+DCh = unit.y + 0.0f, then aimPoint.y           ; 009C7DF1 ([00D7A258] = 0.0f), then 009C7E33
+E0h = unit.z + tf * v.z                        ; 009C7E01 / 009C7DCF / 009C7E27
```

`007BCC80` is `RET 4`, `__thiscall(unit, float height)`, body `007BCC80`-`007BCCEB`, and it is the
**time of flight of a dropped body**:

```
007bcc86  height <= 0  ->  0.0
007bcc95  vy = unit->vtable[+34h]().y - [00E08E54]     ; 00E08E54 = 3.0f, in .data
007bccb0  d  = vy*vy + 2*height*[00CF9058]             ; 00CF9058 = 9.81, qword
007bccca  t  = (sqrt(d) + vy) / [00CF9058]
```

which is the positive root of `h = -vy*t + g*t^2/2` with `vy` the world-Y velocity, upward
positive: `vy = 0` gives `sqrt(2h/g)` and a descending aircraft gets a shorter fall. That same
function is what identifies `unit->vtable[+34h]`: it reads `.y` of the slot's result as a vertical
velocity inside a free-fall solution, so the slot is the unit's **velocity** getter. The slot has no
recovered name and the identification rests on that use plus the dimensions of `unit.x + tf * v.x`;
`007B3DF0`, which occupies the same index of the *approach*'s vtable `00D20C48`, is a bare `RET` and
is not this call - `009C7D9F MOV ECX,[ESI+4]` rebases to the unit first.

`007BCC80` is shared: `009D139D` in the torpedo approach update calls it for the drop lead, where
`docs/TORPEDO_APPROACH_UPDATE.md` carries `fall_time` as an unimplemented host contract. It is
reconstructed in this packet as `weapon_fall_time_007bcc80`; nothing on the torpedo side is changed.

### What that makes the aim error

`approach+D8h`/`+E0h` is the **predicted impact point of a bomb released this tick** - the aircraft
advanced by its own velocity over the bomb's time of flight - and `+DCh` is the aim point's own Y.
The aimdive tick differences the aim point against it at `009C5950`/`009C5960`, and that difference
is the input of the second `sqrt` (`[ESP+5Ch]`) and of the bearing at `009C5AF1` (`[ESP+18h]`),
which are the aim error's two geometric inputs. So

```
error = gain(h) * ( cos(aimHeading - bearing(aimPoint - impactPoint)) * |aimPoint - impactPoint|
                    - lead(h) )
```

and the 25.0 at `00CE3880` is a **25-metre CCIP window**: release when the predicted impact point
is within 25 m, along track, of the aim point. That is why this host, which passed the live
aircraft-to-target range to both, measured `range/error = 433/433` and never approached the gate -
the substituted quantity has no reason to fall to 25 m, and the real one does so by construction.

It also withdraws this document's "no gravity term, no time of flight and no target velocity
anywhere in `009C58D0`-`009C6161`". That sentence is true of the tick's own body and was read
correctly; the ballistic work is one level up, in the approach update, and reaches the tick through
`approach+D8h`.

### The roll arms take different bearings, and that was a real defect

Walked with frame bases (`local\t.ps1`), so the `SUB ESP,14h` at `009C5D37`/`009C5D64` is not read
as a displacement:

| arm | slot | quantity |
| --- | --- | --- |
| `009C5D33`, wide band | `[ESP+18h]` | bearing error from the **impact point**, `009C5AF1` |
| `009C5D60`, default band | `[ESP+24h]` | bearing error from the **aircraft**, `009C5AA3` |

`DiveBombAimDiveSteerInputs` carried one `bearing_error` and both arms used it. It now carries both,
and `dive_bomb_aimdive_steer_009c5c9f` picks by `used_wide_band`, the same predicate that picks the
band constant.

---

## The aimglide release, walked with frame bases: three vectors, and the salvo can fire

Packet `cc8_dive_glide`. Addresses `009C5180`-`009C580B`, `009C4F00`-`009C4F71`, `009C3F16`.

**This withdraws four claims.**

1. `docs/HANDOFF_DIVE_BOMB_AIMGLIDE_AND_SPAWN.md`'s hypothesis that `lateral_a` is the throw and
   `lateral_b` is the **miss**. The throw half is right; `lateral_b` is the **range to the aim
   point**. The miss is not a release input at all.
2. `kGlideDiveAngleLimit`'s name and every comment calling `[ESP+18h]` a dive angle or a flight
   path. It is a horizontal **bearing error**.
3. `src/game_hosts_units.cpp`'s "the argument's own producer is `009C4F00`'s caller, unread".
   `009C4F00` takes no stack argument; it builds the seed itself.
4. The header's "the three range terms are frame slots whose producers this packet did not trace"
   and the `partial` coverage that went with it.

### The frame, which is why the literal displacements mislead

`009C5180` is `SUB ESP,58h` then `PUSH EBX` / `PUSH EBP` / `PUSH ESI` / `PUSH EDI`, so the canonical
frame sits `0x68` below entry: `[ESP+68h]` is the return address and **`[ESP+6Ch]` is the `dt`
argument**, which the body then reuses as scratch from `009C5295` onward. One `PUSH EAX` at
`009C521B` is what makes `009C521C`'s literal `[ESP+48h]` the frame slot `44h` and `009C522E`'s
literal `[ESP+50h]` the frame slot `4Ch` - the stack reuse the handoff warned about, and the reason
`009C523C`'s literal `[ESP+50h]` is a *different* slot from `009C522E`'s.

### Three planar vectors, not two

| built at | vector | components land in | magnitude |
| --- | --- | --- | --- |
| `009C51D5`-`009C51F7` | `aimPoint - unit` | `[ESP+38h]`, `[ESP+40h]` | `[ESP+10h]`, `009C52B6` |
| `009C5207`-`009C522E` | `impactPoint - unit`, the **throw** | `[ESP+44h]`, `[ESP+4Ch]` | `[ESP+14h]`, `009C52F8` |
| `009C5234`-`009C5256` | `aimPoint - impactPoint`, the **miss** | `[ESP+50h]`, `[ESP+58h]` | `[ESP+1Ch]`, `009C533A` |

The handoff transcribed the second and third and missed the first, which is why its hypothesis had
to put the miss where the range belongs. `impactPoint` is `approach+D8h`/`+E0h`, the predicted bomb
impact point `009C7D71` rewrites every tick.

**The miss never reaches the release.** Its only consumer is `009C53D8`'s 140.0 (`00D04A24`)
command-arm split, and `009C5493` overwrites `[ESP+1Ch]` with the release ceiling
`(approach+14h)->+40h * approach+A8h` at the join of both command arms, which dominates every gate.

### The six gates, in order

| # | site | test | constant |
| --- | --- | --- | --- |
| 1 | `009C5689` | `state+1Ch < 0`, the rearm countdown | XMM2 = 0, `009C562D` |
| 2 | `009C569B` | `|bearing error| < pi/6` | `00CEC724` |
| 3 | `009C56A6`-`009C56B8` | `ceiling + 50.0 > height above aim point` | `00CE3938` |
| 4 | `009C56BE`-`009C56FE` | `|throw - range| < 120.0` | `00D1F3F8` |
| 5 | `009C5743` | `lead < -5.0` | `00D7A370` |
| 6 | `009C5755` | `lead > -4*travel - 5.0` | `00D7A2B0` = 3.0, plus the bare travel |

Gate 1 was **missing from the host entirely**: the reconstruction began at `009C569B`, so a
satisfied geometry would have released on every tick instead of once per 0.2-0.5 s draw. XMM2 is
zeroed once, at `009C562D`, and every jump into `009C562D`-`009C5689` starts inside that block, so
the load dominates the compare. The same dominance argument settles gate 4's abs: `009C56E4`'s
`SUBSS XMM4` is a negate, XMM4 being loaded once in the whole body at `009C561B` from the `-0.0f` at
`00D7A208`.

`[ESP+18h]` is built at `009C5357`-`009C53CA`: `FLD [ESP+40h]` / `FLD [ESP+38h]` -> `atan2` ->
`pi/2 -` that (`00CE3830`), `+2pi` when negative (`00CE3828`) - **the same wrap `009C7B8A`-`009C7BB0`
builds for `approach+C0h`** - then `00438B10` against `009C4F80`'s aim heading, then abs. So it is
the horizontal angle between the line of sight to the aim point and the commanded aim heading. The
aimdive's aim error takes its bearing from the *impact point* (`009C5AF1`); the aimglide takes it
from the *aircraft*. Both feed `009C4F80`.

### What the lead actually is

`009C5715`-`009C5725`: `lead = range - cos(bearing error) * throw`. That is the **along-track
shortfall of the predicted impact point against the target**, and it goes negative when the throw
overruns. With gates 5 and 6 the window is

```
-4*travel - 5.0  <  lead  <  -5.0
```

a deliberate overshoot of 5 m up to `4*travel + 5` m - which is how a stick of bombs is walked
through a target rather than aimed at it. Two equal inputs made `lead = range * (1 - cos)`, never
negative, so `009C5777` could not fire at any altitude, angle or travel.

### `travel` (`state+20h`) and `approach+A4h`, both recovered

`009C4F00` is `__thiscall(state)` with **no stack argument** - `SUB ESP,8` at entry, `ADD ESP,8` and
a bare `RET` at both exits. It zeroes `state+18h` and `state+1Ch`, then seeds:

```
009c4f15  fld  [approach+0A4h]
009c4f22  call 007c1db0            ; BSP_Unit_CountRemainingOrdnanceRounds
009c4f27  sub  eax, 1
009c4f2e  fild / 009c4f32 fmul qword 00CED0D8 (0.07) / 009c4f38 fmul the +A4h value
009c4f44..009c4f4e                 ; against the qword 5.0 at 00D7A370
009c4f50  [esi+20h] = 5.0f (00CE3850)   |   009c4f62  [esi+20h] = the product
```

so `state+20h = max((rounds - 1) * 0.07 * approach+A4h, 5.0)`.

A Capstone scan of `009C3E00`-`009CA000` for `[reg+0A4h]` returns six touches and **one writer**,
`009C3F16` in the constructor `FUN_009C3EA0`: `MOV ECX,[ESI+8]` (the plane class descriptor), `FLD
[ECX+188h]` (MaxSpd, already named in this repo), `FMUL qword 00CEFFB0` (0.95), `FSTP [ESI+0A4h]`.
So **`approach+A4h = 0.95 * MaxSpd`**, set once and never rewritten, and `009C57A9` adds
`approach+A4h * 0.35` into `state+20h` after every salvo.

For the USN04 Vals - MaxSpd 69.44, two rounds - the seed is `1 * 0.07 * 0.95 * 69.44 = 4.62`, under
the floor, so the flat 5.0 the host used happened to be right. That is a coincidence, not the rule,
and the formula is bound now.

### One more literal

`009C53DD MOV EBP,2` and `009C53E2 LEA EBX,[EBP-1]` sit on the straight-line path before
`009C53E5`'s branch, so both command arms carry EBP=2 and EBX=1 into `009C5762`'s cap and
`009C5782`'s step. The salvo is **at most two rounds**, from a literal immediate - not the
`kDiveBombCarriedRoundsSubstitute` the host happened to share with it.

### The measurement: `local\glide_before.log` / `local\glide_after.log`, same binary apart from the change

Two 4800-frame USN04 runs, before taken at `e19129b34` in this tree rather than reused from the
retired worker's.

**Nothing else moved.** Every dive-bomb figure is identical to the digit: `movieval` `arm_ticks=2112`,
`states[done=303 aimdive=53 flyabove=158 turndown=71 attackrun=1527]`, `releases=2`, release
`alt=278.9 m speed=66.7 m/s range=365.0 m`, `aim error closest=8.06 m`; the Vals `aimglide=630`/`626`;
three `plane water contact` lines in both. The binding changes what the aimglide *asks*, not what the
aircraft do, which is what a before/after on this chain should show.

**The salvo still does not fire, and the reason has moved.** Per `D3A Val #1.1`, 630 aimglide calls:

| gate | blocked |
| --- | --- |
| 1, `009C5689` rearm | 0 |
| 2, `009C569B` bearing | **523** |
| 3, `009C56B8` ceiling | **107** |
| 4, `009C56FE` lateral | 0 |
| 5/6, the lead window | 0 |
| passed | 0 |

`#3.1` is the same shape: 626 calls, 520 and 106. **The chain never reaches the lead**, so
`lead last`/`min` are both 0.00 - not a lead of zero, a lead never computed. That is worth stating
plainly: this packet did **not** make the salvo fire, and nothing here should be read as claiming it.

What it did is move the defect out of the release arithmetic. Before, the pair of lead gates was
mutually exclusive at any geometry, so `009C5777` was unreachable by construction. Now the chain is
well formed and the block is upstream geometry, which the same line measures:

* `throw=748.8 m` against `range=201.4 m`. `|throw - range| = 547 m` is 4.6x the 120 m gate 4
  allows, so even with the ceiling open the lateral gate would refuse. A 748.8 m throw at ~70 m/s is
  a fall time near 10.7 s, i.e. a release height near 560 m - the Vals are gliding far too high for
  the geometry the release wants.
* `bearing err min=0.0008 rad` against the 0.5236 gate: the bearing gate is satisfiable and is met at
  its best, but fails on 83% of ticks, so the aircraft is not tracking the target for most of the
  glide.
* `travel=5.00`, which is the seed formula's own prediction confirmed in the run:
  `(2-1) * 0.07 * 0.95 * 69.44 = 4.62`, under the 5.0 floor, so `max` takes the floor.

Both the ceiling block and the 748.8 m throw point at the same thing as item 2 of this packet - the
Vals enter their dive from 1024.3 m against `movieval`'s 650.9 m - and not at the release chain.

## Item 2, first half: `approach+ACh` is a GLOBAL, so it is not what makes the Vals dive high

Packet `cc8_dive_glide`. Read from the constructor `FUN_009C3EA0`, immediately after the
`approach+A4h`/`+A8h` pair above:

```
009c3f34  call 0042e740                 ; BSP_GameTuning_GetSingleton
009c3f39  fld  dword ptr [eax + 0x4cc]
009c3f3f  fstp dword ptr [esi + 0xac]   ; approach+ACh = tuning+4CCh
009c3f45  call 0042e740                 ; -> EBP
009c3f4c  call 0042e740                 ; -> EAX
009c3f51  fld  dword ptr [ebp + 0x4d0]
009c3f57  fsub dword ptr [eax + 0x4cc]  ; tuning+4D0h - tuning+4CCh
009c3f68  fstp dword ptr [esi + 0xb0]   ; approach+B0h = that span
```

So the BeginAltRange pair is **a base and a span taken from the game tuning singleton**, not from a
`PilotBotParameters` row and not from the plane class. `0042E740` takes no argument: there is one
such record in the process.

**That rules out the obvious explanation for the Vals' dive entry.** The run agrees: the install
line prints `approach+ACh=1000.0 (BeginAltRange/1)` identically for all twelve dive bombers,
`movieval` and `D3A Val` alike. Whatever makes `D3A Val` enter its aimdive at 1024.3 m while
`movieval` enters at 650.9 m from an almost identical entry range, it is **not** `approach+ACh`, and
it is not a per-class or per-row altitude. The next candidates are where the aircraft actually *is*
when the flyabove hands over - the flyabove/turndown path - and `approach+D4h`, not this field.

Two neighbours fall out of the same read and are recorded here because they were in the listing:

* `approach+B4h = uniform(00CE3D30, 00CE74F8) * (classDesc+268h)` - `009C3F6E`-`009C3F97`.
* `approach+B8h = approach+BCh = uniform(00D06BB4, 00CF4848) * (classDesc+268h)` - `009C3F9D`-`009C3FF5`,
  one draw stored twice (`009C3FE8 FST`, `009C3FF5 FSTP`). `+B8h` is the attack distance the run
  prints as 1100.0, and `classDesc+268h` is the per-class scale on it. NOT verified against the
  authored rows in this packet; the arithmetic is transcribed, the identification of `+268h` is not.

### Item 2, second half: the row index, read from `009F9CE0`, and one correction

`FUN_009C3EA0`'s first call is `009C3ED5 CALL 009F9CE0` with the approach in ECX, the unit pushed,
and the float `tuning+4D8h` (`009C3EBF`-`009C3ECF`) pushed under it. `009F9CE0` fills the base:

```
009f9cea  [approach+04h] = unit
009f9cf3  [approach+08h] = unit+538h        ; the plane class descriptor
009f9cfc  [approach+0Ch] = unit+9D4h
009f9d05  [approach+10h] = unit+DF4h
009f9d08  mov edx, [eax + 0xdf4]
009f9d0e  mov edx, [edx + 0x34]             ; THE ROW INDEX
009f9d11  imul edx, edx, 0x248
009f9d18  mov esi, dword ptr [0xf8a30c]     ; the TABLE BASE, loaded from that address
009f9d1e  lea edx, [edx + esi + 0xc]
009f9d22  [approach+14h] = that
009f9d37  [approach+24h] = max(classDesc+188h MaxSpd / the pushed float, 1.0)
```

**Correction to the handoff's form `approach+14h = 00F8A30C + index*248h + 0Ch`.** `009F9D18` is
`MOV ESI,[0xF8A30C]`, a load: `00F8A30C` holds a **pointer** to the table, it is not the table's
first byte. The row is `[00F8A30C] + index*248h + 0Ch`.

The index is `[[unit+DF4h]+34h]`, which is what `src/game_hosts_units.cpp` already records at its
`&PilotBotConfig.levels[...]` comments and already labels unmodelled. This read confirms it from the
listing rather than adding to it. **The answer to "which row does a spawned AI aircraft draw" is
therefore: this host cannot know.** It would need `unit+DF4h` - the object the index lives on - and
nothing in this reconstruction creates or fills it; `approach+10h` is that same pointer, so modelling
one gives the other.

Also settled in passing: the divisor of `approach+24h`'s speed ratio, which this document carried as
an unnamed `referenceSpeed`, is **`tuning+4D8h`** from the same `0042E740` singleton (`009C3EBF`).

**So neither half of item 2 explains the Vals.** `approach+ACh` is global and identical for all
twelve; the row is SPNormal for all twelve by this host's own substitution. The 1024.3 m against
650.9 m difference in aimdive entry height has to come from where the aircraft is when the flyabove
hands over, not from an authored altitude - which is where the next packet should start.

## Item 3: the bomb spawns, and the predicted impact point is good to 17.8-24.5 m

Packet `cc8_dive_glide`. `local\bomb_spawn2.log`, 4800-frame USN04.

**Every earlier `bombs_spawned=0` in this stream was structurally zero, and said nothing about the
release.** The summary column was printing `slot->torpedo_drops_spawned` - the TORPEDO drop's
counter, which a dive bomber can never raise, because the only thing that increments it is
`release_ordnance_drop`'s torpedo-row path. So the column could not have shown a bomb however many
the task spawned, and no run that printed it was evidence about bombing. It now prints the bomb
field, with the torpedo value kept beside it so an older log can be compared against a newer one.

**The selection predicate was the whole defect.** `GameGunneryHost::release_ordnance_drop` selects
`swim_speed > 0` rows and clears kind `2Bh`; a dive bomber carries kind `2Ah`, so it found nothing.
The new `release_bomb_drop` selects on `ordnance_has_general_bomb_2ah` - kind `2Ah` excluding
`2Ch`/`31h`/`2Bh`/`33h`/`2Dh`, the set `007B9320` tests - and reuses the same `0072F830` spawn
unchanged. Result: `bomb_drops=6 refusals=0`, one bomb per counted release, `bullet 77`.

**Predicted against actual**, which is what this packet owed. `approach+D8h`/`+E0h` at the release
tick is the point `009C7D71` computed for a bomb dropped then:

| bomb | flight | vs predicted | vs target at release | died |
| --- | --- | --- | --- | --- |
| `movieval|.-3` | 3.45 s | **24.5 m** | 16.5 m | entity sweep, y = 8 |
| `movieval|.-2` | 2.15 s | **17.8 m** | 26.4 m | sea surface |
| `movieval|.-3` | 2.15 s | **17.8 m** | 26.4 m | sea surface |

The CCIP solution and the round it predicts agree to **17.8-24.5 m**, inside the 25 m window
`00CE3880` gates the release on. That is the binding validated by the round rather than by the
steering it feeds.

**Three of the six died 0.05 s after release**, at 276 m and 184 m, on the entity sweep. The cause is
visible in the log and is **not** the spawn: the three `movieval` are **co-located in this host** -
their entire dive-bomb census is identical to the digit, and two bombs with *different owners* record
the same impact `-12936 -0 -12930` at the same 2.15 s - so a round leaving one aircraft's origin
starts inside a mate's sweep volume. That is the release-geometry SUBSTITUTION showing its cost: the
round leaves from the plane's centre because the native mount node and the platform's release slot
are unread. The co-location is established; that a mount node offset would clear it is NOT, and no
hull box was measured.

**`bay 007BBBA0: accepted=1 refused=1` per aircraft**, which is the listing's own latch made visible:
`007BBBB2` leaves when channel C is already at the top, so the first release opens the bay and the
second is refused. The spawn is therefore driven by the counted rounds, not by the latch - tying it
to `accepted` would drop the second round of every salvo.

**A regression caught and removed before this run.** The first attempt
(`local\bomb_spawn.log`) also decremented `dive_bomb_rounds_remaining` on each spawn. The task's own
`spend_round` already does, so the stock went 2 -> 0 on the first bomb, `db_has_bomb_d1` went false,
and `movieval` fell from `releases=2 aimdive=53 arm_ticks=2112` to `releases=1 aimdive=38
arm_ticks=2370` - the task completing early and the aircraft no longer ditching. With the second
decrement out, every figure is back to the baseline: `arm_ticks=2112`, `releases=2`,
`states[done=303 aimdive=53 flyabove=158 turndown=71 attackrun=1527]`, `bombs_spawned=2`.

Mission effect: damage `1552.5 -> 2660.3`, deaths `3 -> 6`, hull hits `42 -> 45`. Three of those
deaths are the co-location artifact above, so the damage figure is **not** a clean measure of what
bombing is worth in this mission and should not be quoted as one.

**No kind `2Ah` loadout clear**, deliberately. The torpedo side clears `2Bh` because one drop is its
whole loadout; a bomber's is a salvo out of a per-device stock this host does not model (`006E3500`
unread). Clearing `2Ah` would make `009C7AFE`'s `HasGeneralBombOrdnance` false after the first bomb
and take the aimdive's second release with it - the same shape as the regression above.

### RETRACTION: `approach+ACh` is NOT the global. The constructor's store survives zero ticks

Packet `cc8_dive_glide` withdraws the section "Item 2, first half: `approach+ACh` is a GLOBAL, so it
is not what makes the Vals dive high" above, and the matching ledger evidence at `009C3F3F`. The
section's listing is accurate and its conclusion is wrong, for a reason this document has already
recorded once about `approach+D8h`: **I scanned for one displacement and assumed the constructor was
authoritative.** I scanned `[reg+0A4h]`, read the neighbouring stores while I was there, and never
scanned `[reg+0ACh]`.

Scanning it returns **two** writers in `009C3E00`-`009CA000`, not one:

| site | function | what it stores |
| --- | --- | --- |
| `009C3F3F` | `FUN_009C3EA0`, the constructor | `tuning+4CCh`, from `0042E740` |
| `009C7AAD` | `009C7A80`, the approach update | `ctl+398h` |

`009C7A80` runs on **every** arm tick, before the transition and before the state tick, so the
constructor's value survives exactly zero ticks - the identical trap `approach+D8h` set:

```
009c7aa4  mov eax, dword ptr [esi + 0xc]    ; approach+Ch, which 009F9CFC set to unit+9D4h
009c7aa7  fld dword ptr [eax + 0x398]       ; ctl+398h, BeginAltRange/1
009c7aad  fstp dword ptr [esi + 0xac]       ; approach+ACh, rewritten this tick
009c7ab3  fld dword ptr [eax + 0x39c]       ; ctl+39Ch, BeginAltRange/2
009c7ab9  fld dword ptr [esi + 0xa8]        ; approach+A8h
009c7abf  fmul qword ptr [0xd7a270]         ; 0.05
009c7ac5  fsubp st(1)                       ; ctl+39Ch - 0.05 * approach+A8h
```

`docs/HANDOFF_DIVE_BOMB_AIMGLIDE_AND_SPAWN.md` section 1a had this right - `approach+ACh` is
`ctl+398h` - and this packet's earlier section contradicted it from a partial scan.

**What the retraction costs, stated plainly.** `approach+Ch` is `unit+9D4h`, a **per-unit**
controller, so `ctl+398h` *can* differ between `movieval` and `D3A Val`. The claim "it is global,
therefore not the cause" is withdrawn on both halves: the field is not global, and it is **once again
a live candidate** for the 1024.3 m against 650.9 m difference - candidate 1 in section 1a, which is
where it should have stayed.

**And the run evidence I cited does not say what I said it said.** `approach+ACh=1000.0` prints
identically for all twelve dive bombers because it is *this host's own pinned substitute* on its
install line, not a reading of the image's `ctl+398h`. A host substitution being constant is not
evidence that the image's field is. That is the "check what a summary column actually reads" rule,
and I broke it in the same packet in which I caught the `bombs_spawned` column doing the same thing.

**What still stands from that section**, re-checked rather than assumed: `approach+B0h` has exactly
one writer in the range, `009C3F68`, so it is `tuning+4D0h - tuning+4CCh`; `approach+B4h` and
`approach+B8h`/`+BCh` are as transcribed; the `009F9CE0` row-index reading and the `[00F8A30C]`
pointer correction are unaffected, as is `tuning+4D8h` as the speed-ratio divisor.

**So item 2's first candidate is open, not closed**, and the next reader should start at `ctl+398h`:
who writes it, and whether a spawned Val's controller carries a different `Pilot/DiveBomb/BeginAltRange/1`
from the scripted `movieval`'s.
## Item 2 of `cc8_dive_entry`: `approach+B4h`/`+B8h` verified, and `classDesc+268h` named

Packet `cc8_dive_entry`. The bullets in "Item 2, first half" flagged the `classDesc+268h` scale as
transcribed but not verified. Both halves are now checked, and the transcription **stands**.

### The register, filtered rather than glanced at

The multiply that matters is `009C3F86 FMUL float ptr [EBP + 0x268]`, and the instruction three
lines below it is `009C3F8C MOV EBP,[ESI+8]` - so a window opened at `009C3F6E` shows EBP being
loaded with the class descriptor *after* the multiply that uses it, which is exactly the trap the
"register provenance" rule is about. Filtering the **whole** constructor listing for EBP gives every
write:

```
009c3eb6  PUSH EBP
009c3f4a  MOV EBP,EAX          ; the 0042E740 tuning singleton
009c3f5d  MOV EBP,[ESI + 0x8]  ; the plane class descriptor  <- dominates 009C3F86
009c3f8c  MOV EBP,[ESI + 0x8]  ; reloaded, dominates 009C3FB5
009c4090  POP EBP
```

`009C3F5D` is the only write between the tuning load and `009C3F86`, so **both** multiplies take
`classDesc+268h`. The doc was right and the risk was real: with only `009C3F6E` onward in view the
natural reading is `tuning+268h`.

### `classDesc+268h` is TurnCircleRadius

`include/bsp/plane_class_fields.hpp:174` already names `+268h` `kTurnCircleRadius`, recovered
elsewhere in this repo from `007D2xxx`. So the two draws are multiples of the aircraft's own turn
circle, which is what an attack-distance field should key on.

### The constants, at the width of the loading instruction

`00CE3D30` = 0.6, `00CE74F8` = 0.8, `00D06BB4` = 1.6, `00CF4848` = 1.8, all floats loaded by
`FLD float ptr`. The push order is argument 2 first (`FSTP [ESP+4]`) then argument 1 (`FSTP [ESP]`),
so the pairs are `uniform(0.6, 0.8)` and `uniform(1.6, 1.8)` as written.

### This installation's authored rows

`scripts/datatables/autoload/vehicleclasses.lua` (mtime 2026-05-09, the locally modified file):

| class | row | `TurnCircleRadius` | `approach+B4h` = `uniform(0.6,0.8)*r` | `approach+B8h` = `uniform(1.6,1.8)*r` |
| --- | --- | --- | --- | --- |
| SBD Dauntless | `VehicleClass[108]`, line 46353 | 1300 (line 46667) | 780 - 1040 m | 2080 - 2340 m |
| D3A Val | `VehicleClass[46]`, line 23399 | 1300 (line 23623) | 780 - 1040 m | 2080 - 2340 m |

The `VehicleClass[46]` comment in this installation reads `-- Kamikaze D3A Val`; it is the row the
brief names and the only D3A Val row quoted here.

**A defect falls out, and it is NOT fixed in this packet.** `src/game_hosts_units.cpp` seeds *both*
`db_in_range_b8` and `db_attack_dist_b4` from `kPilotDiveBombAttackDist` = 1100.0, labelled as
`009C8A5E`'s floor. Against the authored rows that is wrong twice over: the two fields are not the
same quantity in the image (`+B4h` is about 0.44 of `+B8h`), and `+B8h`'s own floor
`max(draw, AttackDist * task+41Ch)` would take the **draw**, 2080 m at the low end of the convention,
not the 1100 m floor. `+B8h` is the range at which `009C7C31` latches and the aircraft leaves the
run-in, so this host starts its flyabove at half the distance the authored rows ask for. Binding it
needs `turn_circle_radius` plumbed from the plane class row into the slot, which is another hunk of
`src/game_hosts_units.cpp`; it is recorded here with its numbers rather than changed under this
lease.

## Item 1: the flyabove commands an altitude, this host commanded none, and the entry height is the spawn height

Packet `cc8_dive_entry`. Addresses `009C6E10`-`009C6F91`, `009C84FB`-`009C8515`, `009C67A7`-`009C680E`.

### What decides the hand-over, confirmed at the transition rule

`009C83E0`'s flyabove arm, read from the listing rather than from the state table:

```
009c84fb  CMP byte ptr [ESI + 0x791],0 ; the flyabove's +19h, the roll-in flag
009c8502  JZ  0x009c85e8               ; not ready -> stay (or +792h -> goaway)
009c8508  CMP byte ptr [ESI + 0x790],0 ; the flyabove's +18h, the can-dive flag
009c850f  JZ  0x009c85d4               ; -> aimglide
009c8515  ...                          ; otherwise -> +79Ch, the turndown
```

So **`+19h` decides *when* the flyabove ends and `+18h` decides *what it ends into*.** Both key on
`B`, the height above the aim point that `009C6493` writes:

* `+19h` (`009C67A7` `77` JA, `009C67AE` `72` JC) is `|bearing error| > 1.6` **OR** `x <= 0`, and
  `x <= 0` is `B <= 666.7 m` on this installation.
* `+18h` (`009C67F6` `76` JBE, stored at `009C680E`) is `B > approach+D4h` = **675.0 m**.

The two height arms are within 8 m of each other and point opposite ways, so **the only route from
the flyabove into the dive is the bearing arm**: the aircraft has to still be above 675 m at the
moment the target passes 91.7 degrees off its nose. Where it is when that happens is the whole
question, and it is why this packet is about the altitude and not about the gates.

### The altitude arm the host did not have

`009C62B0` never calls `009FBA50` - a call census over the whole body `009C62B0`-`009C7083` finds
`00419010` x8, `00438AA0` x3, `00438B10` x3, `00414DB0` x2, `00BF701A` x2, and one each of
`0042E740`, `007C4810`, `007F0280`, `0099B630`, `009FA2E0`, `009FABE0`, `009FB800`, `00BF7030`. It
calls `009FB800` **directly**, at `009C6F7D`, and builds both arguments itself. The walk, with every
jump sense from the branch byte (`009C657C` `76`, `009C6E26` `76`, `009C6E5B` `76`, `009C6E91` `76`,
`009C6EAF` `76`, `009C6EC3` `76`, `009C6EF5` `77`, `009C6F15` `76`, `009C6F21` `76`, `009C6F51` `76`):

```
C      = approach+0Ch ? ctl+398h : approach+ACh + approach+50h     009C64A8 / 009C64B2
R      = (approach+14h)->+40h * approach+A8h                       009C655F-009C6568
C     := R   when   C > 1.1 * R                                    009C657C
base   = approach+ACh + approach+50h                               009C6E48-009C6E51
err    = B - min(approach+ACh, C)                                  009C6E1C-009C6E44
target = min(base, C)                                              009C6E55-009C6E71   -> 009FB800 arg1
band   = min(0.15 * C, approach+B0h)                               009C6E77-009C6EA1
A      = the planar distance to the aim point                      009C6379-009C63B1

err <  0     -> 009FB800(target, min(3 * -err / max(A, 1), 1.0))
err <= band  -> NO 009FB800: cmd+2BCh = 0.0 with cmd+2D0h = 2, a dead band
otherwise    -> 009FB800(target, min(2 * A / max(A, 1), 0.8))
```

`R` is the same product `009C548A`/`009C548D` builds for the aimglide's release ceiling -
`DiveBombNewReleaseMul * DiveBombReleaseAlt` - which this installation's `robots.lua` SPNormal row
authors at `0.6` (line 569) and `{350, 450}` (line 568), so with the pinned low end **R = 210.0 m**.
Any cruise altitude over 231 m is therefore clamped to 210, and the flyabove's target altitude is
`min(1000 + aimY, 210) = 210 m`: **the state flies the aircraft down toward the glide release
altitude while it runs in over the target.** On the dive arm `A` is a planar range in metres and is
never under 1, so `2 * A / A` is exactly 2 and the 0.8 cap at `00CE74F8` takes it; the reference is
0.8 at every geometry this mission produces.

`approach+ACh` is not the constructor's `tuning+4CCh` at run time: `009C7AA4`-`009C7AAD` rewrites it
from `[approach+0Ch]+398h` on **every** approach update, so the two arms of `C` are the same ordered
cruise altitude and differ only by `approach+50h`. That reconciles "Item 2, first half" (the
constructor seed is a global) with the handoff's "`approach+ACh` is `ctl+398h`" (the per-tick
rewrite). Both are true, of different moments.

**What this host had instead: nothing.** `run_dive_bomb_flyabove_tick_009c62b0` bound only the
heading arm, so a dive bomber in the flyabove kept whatever `plan_state.pitch_target_2bc` the run-in
had last written, for the whole state.

### Measured, before: the dive-entry altitude is the SPAWN altitude, carried through

`local\entry_before.log`, 4800-frame USN04 at `52418c86b` plus the per-hand-over census this packet
adds. One line per aircraft, the altitude and range at every state change:

| aircraft | spawn | attackrun -> flyabove | flyabove -> turndown | turndown -> aimdive | aimdive -> |
| --- | --- | --- | --- | --- | --- |
| `movieval` x3 | **700 m**, 11088 m out | 729 m @ 1093 m | 730 m @ 4 m | 651 m @ 468 m | done, 181 m |
| `D3A Val #1.1` x3 | **1500 m**, 8203 m out | 1126 m @ 1095 m | 1102 m @ 3 m | 1024 m @ 473 m | aimglide, 503 m |
| `D3A Val #3.1` x3 | **1500 m**, 8199 m out | 1146 m @ 1096 m | flyabove/goaway chatter | 995 m | aimglide |
| `D3A Val #5.1` x3 | **1500 m**, 8203 m out | 1126 m @ 1095 m | mission ends in the flyabove | - | - |

Three things fall out of that table, and they answer the packet's question.

1. **The entry altitude is the spawn altitude.** `SpawnNew` places every `D3A Val` at `refPos`
   y = **1500.0** (the four `0094C480` lines in the log), and the scripted `movieval` starts at
   **700**. The 374 m difference at aimdive entry - 1024.3 against 650.9 - is what is left of an
   800 m difference at spawn after the run-in has bled some of it off. Nothing in the task sets it.
2. **Candidate (a) is refuted.** The Vals are not spawned close: they start **8.2 km** from their
   target and fly a 1257-tick run-in. They are spawned *high*, not near.
3. **Candidate (b) is confirmed, and it is the whole flyabove.** Across the flyabove the altitude
   moves by **+1 m** (`movieval`, 158 ticks) and **-24 m** (`#1.1`, 142 ticks). The `cmd` column -
   `plane_commanded_altitude` - is frozen at the run-in's last value for the entire state, because
   nothing writes it. The state whose job is to put the aircraft over its target at a dive height
   does not touch the altitude at all.

A fourth, recorded because it bounds what the run-in can do: both classes are commanded 1450 m (the
`Dynamics/Ceiling - 50` clamp) through most of the run-in and neither reaches it. `movieval` climbs
29 m in 1527 ticks. The cause is named in the next section, and it is **not** the authored data.

## Correction: the aimglide's re-arm timer is never counted down, so gate 1 refuses every call

Packet `cc8_dive_entry`. This withdraws the gate table in "The aimglide release, walked with frame
bases" **as a description of `main`**. That table - `blocked[rearm=0 bearing=523 ceiling=107
lateral=0 lead_hi=0 lead_lo=0]` for `D3A Val #1.1` - was measured in the glide packet's own tree. On
`52418c86b`, `local\entry_before.log` measures the same aircraft at

```
calls=630 blocked[rearm=629 bearing=0 ceiling=1 lateral=0 lead_hi=0 lead_lo=0] passed=0
throw=748.8 m range=201.4 m travel=5.00 bearing err min=0.0008 rad
```

The geometry is identical to the digit; only the gate that stops the chain has moved. `#3.1` is the
same shape: `calls=802 blocked[rearm=801 bearing=0 ceiling=1 ...]`.

**The cause is in the host, and the listing settles it.** `009C5180` counts `state+1Ch` down by its
own `dt` at the very top of every aimglide tick, exactly as `009C58E9` does for the aimdive:

```
009c5188  MOVSS  XMM0,dword ptr [ESI + 0x1c]
009c518d  COMISS XMM0,dword ptr [0x00d7a218]   ; 0.0f
009c519b  JC     0x009c51a8                    ; byte 72: below zero -> do not decrement
009c519d  FLD    float ptr [ESP + 0x28]
009c51a1  FSUB   float ptr [ESP + 0x6c]        ; the dt argument
009c51a5  FSTP   float ptr [ESI + 0x1c]
```

This host decremented `db_aim_rearm_1c` only in `dive_bomb_aimdive_inputs`, which
`src/dive_bomb_task.cpp` calls only while the state is `kAimDive`. The aimglide's own enter clears
the timer to 0.0 (`009C4F0C`/`009C4F10`), and nothing then moves it, so `009C5689`'s `state+1Ch < 0`
is false forever and the release chain never reaches the bearing gate. `read_aimglide_inputs` was
already being handed the `dt` and discarding it.

Fixed by passing that `dt` through and applying the same `if (timer >= 0) timer -= dt` the aimdive
uses. The gate table above should be re-measured after this, not quoted.

## Retraction: `class+1ECh` is not an authored row, and the dive-bomb path is what zeroes it

Packet `cc8_dive_entry`, prompted by the integrator against the torpedo stream's own census. **This
withdraws the claim "`class+1ECh ClimbAngle` is zero on every shipped row, so an aircraft under this
host's pilot can descend but cannot climb"**, which this packet's first commit message carries. It
is wrong twice over, and `include/bsp/plane_flight.hpp`'s comment `// class+1ECh, zero for every
shipped row` is the source of the error.

1. **`ClimbAngle` is not authored at all.** `"ClimbAngle"` occurs **zero** times in this
   installation's `scripts/datatables/autoload/vehicleclasses.lua`. The field is **computed** at
   class load: `007C4BC5`-`007C4C14` probes `007D98F0` at `tuning+24Ch LevelFlight * desc+184h
   StallSpd` and `007C4C0E` scales the answer by the double 0.6 at `00CEFF98` into `desc+1ECh`.
   `src/game_hosts_units.cpp` already models exactly that (`plane_climb_angle_1e4` then
   `_1ec = _1e4 * 0.6`), so the value is live for every plane class, dive bombers included.
2. **The zero is this host's dive-bomb binding, not the data.** `pin.class_climb_angle` is fed
   `slot.plane_climb_angle_1ec` on the torpedo and general plane paths and a literal `0.0f` on the
   dive-bomb attack-run path. That is why the torpedo stream's census prints
   `climb_1ec=0.1854` for a `B5N Kate` - `local\entry_before.log` prints the same 0.1854 in this
   packet's own run - while a dive bomber commanded 1450 m holds its altitude.

So the correct statement is: **the dive-bomb run-in cannot climb because its own binding passes a
zero climb gain to `009FB800`**, and the fix is one field, not a data problem. It is named here and
**not** changed alongside the altitude arm: restoring it lets the run-in climb toward the 1450 m
ceiling clamp, which moves the dive-entry altitude the other way and needs its own before/after.

## Candidate 3 settled: `ctl+398h` is per unit, but every unit draws it from one process-wide row

Packet `cc8_dive_entry`, raised by the integrator after the `approach+ACh` retraction (`294f5a9de`).
The retraction is right that `approach+ACh` is rewritten per tick from `[approach+0Ch]+398h`
(`009C7AA4`-`009C7AAD`) and is therefore a **per-unit** field, so the question "can a spawned `D3A
Val`'s controller carry a different `BeginAltRange` from the scripted `movieval`'s" is a real one.
It is answered **no**, and the answer is in the writer.

An exhaustive store census over the whole image for offset `0x398` - `tools/store_census.py 0x398`,
which covers disp8 and disp32 and the `MOV`/`MOVSS`/`FST`/`FSTP` forms - returns 40 sites. Exactly
one of them is on this path: `009C89CE` inside `009C8920 BSP_BotTaskDiveBomb_UpdateCruiseProfile`
(`00939E83` in `BSP_UnitController_ConstructVariantB` is the block's own constructor default).

```
009c8977  CALL 0042e740              ; the tuning singleton -> EBP (009C8996 MOV EBP,EAX)
009c897c  FLD  float ptr [00CE5380]  ; 15.0   -> argument 2 at [ESP+4]
009c8982  MOV  EDI,[ESI + 0x404]     ; the pilot control block
009c8994  FLDZ                       ; 0.0    -> argument 1 at [ESP]
009c899b  CALL 00BD2F10              ; uniform(0.0, 15.0)
009c89a0  FADD float ptr [EBP+0x4CC] ; + tuning+4CCh, Pilot/DiveBomb/BeginAltRange/1
009c89ce  MOVSS [EDI + 0x398],XMM0
009c89d6  MOV  byte ptr [EDI+0x3AD],1
```

So **`ctl+398h` = `BeginAltRange/1` plus a uniform 0-15 m jitter**, and `0042E740` takes no argument:
there is one tuning record in the process. The field is per unit, as the retraction says, but its
*source* is process-wide and its *spread* is 15 m. It cannot produce the 374 m difference in dive
entry, let alone the 800 m difference at spawn, and it does not differ between a scripted unit and a
`SpawnNew` one - `009C8920` is the dive-bomb task's own cruise update and runs for both.

This host pins `approach+ACh` at `BeginAltRange/1` = 1000.0, which is the low end of what the draw
can produce, consistent with the convention for `approach+A8h`. The gap to the image is at most 15 m.

**Candidate 3 is therefore refuted, and the measured answer in the section above stands unchanged:
the dive-entry altitude is the spawn altitude.** Three candidates have now been tested against the
listing and the run - an authored per-class altitude (no such field), a `PilotBotParameters` row (the
row index is unmodelled and the altitude is not in it), and the per-unit ordered cruise altitude
(one process-wide row plus 15 m) - and what is left is where the aircraft is put and what the task
does about it, which is nothing.

### Measured, after: the arm works exactly as transcribed, and it removes the dive

`local\entry_after.log`, the same binary as `local\entry_before.log` apart from binding
`009C6E10`-`009C6F91`. The census the arm prints is the transcription checking itself:

```
movieval      flyabove altitude 009C6E10: calls=34  level_arm=0 | C=210.0 band=31.5
              target=210.0 ref=0.800 | err first=519.5 last=458.5 | pitch=-0.419 rad
D3A Val #1.1  flyabove altitude 009C6E10: calls=113 level_arm=0 | C=210.0 band=31.5
              target=210.0 ref=0.800 | err first=916.1 last=460.8 | pitch=-0.419 rad
```

`C` clamps to 210.0 as read, the dead band is `min(0.15 * 210, 200)` = 31.5 and is never entered,
the reference is the predicted flat 0.8, and `009FB800` returns `-min(DropAngle * 0.8, ...)` =
**-0.419 rad**, a 24-degree descent. The aircraft follow it: the commanded altitude stops being
frozen (`cmd` 1000 -> **210**) and the hand-overs move.

| aircraft | attackrun -> flyabove | flyabove -> ... | before |
| --- | --- | --- | --- |
| `movieval` | 729 m @ 1093 m | **aimglide** at 665 m @ 849 m, 34 ticks | turndown at 730 m @ 4 m |
| `D3A Val #1.1` | 1126 m @ 1095 m | **aimglide** at 666 m @ 14 m, 113 ticks | turndown at 1102 m @ 3 m |

`#1.1` loses **460 m** in 113 ticks, which is the commanded 24 degrees at its own speed. So the arm
is bound correctly and it does what the listing says.

**And that removes the wingover dive entirely.** Both aircraft now cross `B = 666.7 m` - the
`x <= 0` arm of `+19h` - *before* the bearing arm fires, so the flyabove ends on the height arm with
`+18h` (`B > 675`) necessarily false, and `009C850F` routes every dive bomber to the **aimglide**.
`movieval` goes from `states[done=303 aimdive=53 flyabove=158 turndown=71 attackrun=1527]
releases=2 bombs_spawned=2` to `states[aimglide=809 flyabove=34 attackrun=1527] releases=0
bombs_spawned=0`, and the mission's bomb drops go 6 -> **0**. `#1.1` likewise never reaches the
turndown.

**That is a regression against the brief's guard, and it is reported as one rather than tuned away.**
Three things are established by it, and none of them is "the transcription is wrong":

* The altitude arm runs on **every** flyabove tick. No branch in `009C62B0`-`009C7083` reaches the
  epilogue before the arm: the only jumps to the two `RET 4` sites are `009C702B`'s, which is past
  `009C6F7D`.
* `movieval` enters the flyabove only **54 m** above the 666.7 m leave threshold, so at the
  commanded 24 degrees it crosses in about two seconds with 849 m still to run. **An aircraft
  entering the flyabove below roughly 700 m cannot reach the turndown once the altitude arm is
  bound**, whatever its bearing does.
* The dive-versus-glide choice is therefore a **race between the two `+19h` arms**: bearing first
  means the wingover, height first means the glide. Nothing in the flyabove favours the bearing arm
  at this host's geometry.

**What is NOT established**, and must not be read into this: that the image's dive bombers glide.
The race depends on `approach+B8h` (where the flyabove starts, which this host holds at 1100 m
against the authored 2080-2340 m), on the ordered cruise altitude, and on how fast the real flight
model follows a 24-degree demand. Two of those three are substitutions in this host. What the run
does establish is that binding this arm **alone** is not an improvement, because the aimglide it
hands every aircraft to cannot release at all - `blocked[rearm=809 ...] passed=0` for `movieval`,
`779` for `#1.1` - which is the re-arm timer defect recorded in the section above.

## Packet `cc8_dive_race`: the run-in climb angle, the two attack distances, and who wins the race

Packet `cc8_dive_race`, 2026-09-19, on `agent/cc8-dive-race` = `main` `f4eb153de` merged with the
held-back `agent/cc8-dive-entry`. Three 4800-frame USN04 runs on the same tree, each differing from
the one before it by one binding, so the two substitutions can be told apart:

| window | log | binary |
| --- | --- | --- |
| A, baseline | `local\race_base.log` | the merge, unchanged |
| B, climb angle | `local\race_climb.log` | A plus `pin.class_climb_angle`, plus three census fields |
| C, attack distances | `local\race_dist.log` | B plus `approach+B4h`/`+B8h` from `TurnCircleRadius` |

Window A reproduces `cc8_dive_entry`'s `local\entry_rearm.log` **to the digit** for the two aircraft
that packet tabulated - `movieval` `calls=809 blocked[rearm=0 bearing=701 ceiling=108] passed=0`,
`throw=871.6 m range=560.1 m`; `D3A Val #1.1` `calls=779 blocked[rearm=0 bearing=762 ceiling=17]`,
`throw=868.4 m range=960.1 m` - so the merge changed nothing in this path and the three windows are
comparable with that packet's.

### What the race actually is, closed from the transition rule

`009C84FB`-`009C8515` and the two flag derivations already in this document leave exactly one route
into the wingover dive, and it is worth stating as a closed form because it is what every number
below is measured against:

* `+791h` (roll-in) = `|bearing error| > 1.6 rad` **OR** `B <= 666.7 m`
* `+790h` (can-dive) = `B > approach+D4h` = 675.0 m
* `turndown` needs **both**; `+791h` without `+790h` is the `aimglide`

The height arm of `+791h` fires only at `B <= 666.7`, which is below the 675.0 the can-dive flag
demands, so **the height arm can never produce a dive**. The only door into the turndown is the
*bearing* arm firing while the aircraft is still above 675 m - that is, the bomber has to still be
high when it passes over its target. Everything in this packet is about that one question.

There is a **third** exit and this packet is the first to record it as a competitor: `009C85F5`
sends the aircraft to `goaway` on `flyabove->+1Ah`, which is checked when `+19h` is still clear.
`009C66E1` is a `JBE` whose fall-through at `009C66E3` sets `+1Ah = 1` and clears `+19h = 0`, so
`+1Ah` fires when its quantity *exceeds* its tolerance. In window A that arm took **six of the
fifteen aircraft** - `#3.1` and `#7.1`, all three ships each - at 695-732 m and 44-69 m of range,
i.e. above the can-dive height and almost over the target. They did not lose the race to the height
gate; they were taken out of it.

### Window A, the baseline, per aircraft

`alt`/`rng` at each hand-over; `cmd` is `plane_commanded_altitude`. Wing members are no longer
co-located since tonight's formation fix, so the three ships of a squadron are listed separately
rather than averaged.

| aircraft | spawn | attackrun -> flyabove | flyabove -> ? |
| --- | --- | --- | --- |
| `movieval` | 700 m @ 11088 m | 729 m @ 1093 m, cmd 1000 | **aimglide** 665 m @ 849 m |
| `movieval\|.-2` | 675 m @ 11176 m | 705 m @ 1100 m, cmd 1004 | **aimglide** 665 m @ 917 m |
| `movieval\|.-3` | 725 m @ 11076 m | 755 m @ 1094 m, cmd 1001 | **aimglide** 667 m @ 794 m |
| `D3A Val #1.1` | 1500 m @ 8203 m | 1126 m @ 1095 m, cmd 1002 | **aimglide** 666 m @ 14 m |
| `#1.1\|.-2` | 1475 m @ 8274 m | 1127 m @ 1097 m, cmd 1002 | **aimglide** 662 m @ 6 m |
| `#1.1\|.-3` | 1525 m @ 8274 m | 1126 m @ 1095 m, cmd 1001 | **aimglide** 666 m @ 13 m |
| `D3A Val #3.1` | 1500 m @ 8198 m | 1143 m @ 1099 m, cmd 1003 | **goaway** 715 m @ 57 m |
| `#3.1\|.-2` | 1475 m @ 8269 m | 1135 m @ 1094 m, cmd 1001 | **goaway** 695 m @ 69 m |
| `#3.1\|.-3` | 1525 m @ 8269 m | 1143 m @ 1100 m, cmd 1004 | **goaway** 715 m @ 60 m |
| `D3A Val #5.1` x3 | 1500/1475/1525 m @ ~8250 m | 1126/1127/1126 m @ ~1096 m | still in the flyabove at frame 4800 |
| `D3A Val #7.1` | 1500 m @ 8199 m | 1154 m @ 1096 m, cmd 1002 | **goaway** 732 m @ 44 m |
| `#7.1\|.-2` | 1475 m @ 8269 m | 1142 m @ 1097 m, cmd 1003 | **goaway** 704 m @ 61 m |
| `#7.1\|.-3` | 1525 m @ 8269 m | 1153 m @ 1094 m, cmd 1001 | **goaway** 731 m @ 46 m |

`summary mission dive-bomb task: aircraft=15 releases=0 bombs_spawned=0`. **Nobody reaches the
turndown**, so the bearing arm wins for no aircraft at all.

Two things in that table are worth naming before any binding is judged.

**The run-in commands 1000 m, not 1450 m, and the 1000 is a stand-in.** `alt_base=1000.0 m` in the
attackrun census is `cin.base_altitude = db_begin_alt_ac + db_aim_point_height_50`, and
`db_begin_alt_ac` is `kPilotDiveBombBeginAltRange1 = 1000.0f`, this host's pin for `approach+ACh`,
which the retraction above establishes is the per-unit `ctl+398h` `Pilot/DiveBomb/BeginAltRange/1`.
`cmd` at the hand-over is 1000-1004, so the range term of `009FBA50` contributes 0-4 m there and the
whole commanded altitude *is* the pin. The handoff's "commanded 1450 m" is the `009FB800` ceiling
clamp, which nothing in this mission reaches. **So the ordered cruise altitude is not a background
substitution in this race; it is the single number that sets where the fly-over begins.**

**`approach+B4h` is an input to that altitude as well as to the attack geometry.**
`cin.range_low = attack_distance_b4` and `cin.range_high = planar_distance_bc`, so `009FBA50`'s span
is `range - B4h`. With `+B4h` and `+B8h` both pinned to 1100 the span is exactly zero at the moment
the run-in hands over, which is why `cmd` equals the base to within 4 m. That coupling is why
window C is not only a change of geometry.

### Window B: the climb angle, and why the literal zero could not have been a transcription

`src/game_hosts_units.cpp` passed a literal `0.0f` for `pin.class_climb_angle` at both dive-bomb
call sites. Read from the listing, `009FB800` cannot take a climb angle from anyone:

```
009fb800  PUSH ECX / PUSH ESI                ; one local slot, then the frame
009fb96b  RET 0x8                            ; TWO dword arguments, no more
009fb819  FLD  float ptr [ESP + 0xc]         ; arg1, the desired altitude
009fb85c  FLD  float ptr [ESP + 0x10]        ; arg2, the reference
009fb882  MOV  ECX,dword ptr [ESI]           ; this->[0]
009fb884  MOV  EDX,dword ptr [ECX + 0x4]     ;   ->+4, the unit
009fb887  MOV  EAX,dword ptr [EDX + 0x538]   ;     ->+538h, the class descriptor
009fb88d  FLD  float ptr [EAX + 0x1ec]       ; the CLIMB gain, fetched here
009fb979  FLD  float ptr [EDX + 0x1f0]       ; the DIVE gain, same chain
```

Both angles are fetched inside the routine off the same three-hop chain; the two stack arguments are
the altitude and the reference and nothing else. The one caller this stream had already read,
`009FBB13`, agrees: `SUB ESP,8`, `FSTP [ESP+4]`, `FSTP [ESP]`, `MOV ECX,ESI`, and the wrapper's own
`RET 0x10`. **No caller passes an angle, so no caller can pass zero**, and the dive-bomb run-in
reads exactly the `desc+1ECh` every other path reads. The literal was a host defect, not a
transcription of a dive-bomb-specific argument, and the same defect sat at the flyabove's own
`009FB800` call added by `cc8_dive_entry`. Both are now `unit_.plane_climb_angle_1ec`.

The field is not zero and is not argued from a source constant: the attackrun census prints
`climb_1ec=0.1872 rad` (10.7 deg) for **both** classes, computed at class load by
`max_sustainable_climb_angle_007d98f0` as this document records.

**Measured, A -> B.** The binding moves exactly the aircraft the listing says it should and no
others: the climb arm of `009FB800` runs only when the aircraft is *below* its command, and only
`movieval` is.

| aircraft | flyabove entry, A -> B | attackrun ticks | flyabove exit, A -> B |
| --- | --- | --- | --- |
| `movieval` | 729 -> **1127 m** | 1527 -> 1657 | aimglide 665 m @ 849 m -> aimglide 663 m @ **7 m** |
| `movieval\|.-2` | 705 -> **1125 m** | 1539 -> 1677 | aimglide 665 m @ 917 m -> aimglide 665 m @ **10 m** |
| `movieval\|.-3` | 755 -> **1126 m** | 1526 -> 1650 | aimglide 667 m @ 794 m -> aimglide 667 m @ **15 m** |
| `D3A Val #1.1` x3 | unchanged | unchanged | unchanged |
| `#3.1`, `#5.1`, `#7.1` x3 | unchanged | unchanged | unchanged |

`movieval` gains 398 m of fly-over entry altitude, reaches its commanded 1000 m instead of drifting
29 m above its spawn, and its exit moves from 849 m short of the target to **7 m** - it now flies the
whole fly-over and crosses the height gate essentially over the aim point, which is where `#1.1` has
always crossed it. Its flyabove lasts 34 -> 114 ticks and its glide shortens: `calls=809 -> 599`,
`ceiling` blocks `108 -> 1`, glide range `560.1 -> 158.5 m`.

It is still not a dive. `releases=0 bombs_spawned=0` across the mission, unchanged.

**Two corrections that fall out of window B.** `db_attackrun_throttle_last`, printed as
`throttle=` in the attackrun census, is assigned `r.descent_scale` - it is the descent scale, not a
throttle, and its move from `1.000` to `0.748` is that scale responding to the new geometry, not a
throttle change. And the `+1Ah` tolerance recorded in "The two flags, complete" as
`InterpolateClamped(0, 20 deg, W, pi, x)` does not match the constants at the `009C666F` interpolate
call, which are `(0.0, 0.5236, 200.0, 0.0, x)` - 0.5236 rad is 30 deg, not 20, and the sequence
descends to zero rather than rising to pi. There are two `00419010` calls in that chain
(`009C663E` and `009C666F`) and I did not filter EBP across the whole 949-instruction listing, so I
am **not** withdrawing the formula, only flagging that its endpoints do not reproduce and that the
goaway arm deserves a re-read by whoever owns it - it decided six of fifteen aircraft in window A.

### Window C: `approach+B4h` and `+B8h` from `TurnCircleRadius`

The two fields were both seeded from `kPilotDiveBombAttackDist = 1100.0`. The constructor listing,
re-read for this packet rather than carried over, settles both the register and the pairs:

```
009c3f45  CALL 0042E740            ; the tuning singleton
009c3f4a  MOV  EBP,EAX
009c3f5d  MOV  EBP,dword ptr [ESI + 0x8]   ; the plane class descriptor
009c3f68  FSTP float ptr [ESI + 0xb0]      ; +B0h = tuning+4D0h - tuning+4CCh
009c3f6e  FLD  [00CE74F8] -> [ESP+4]       ; 0.8
009c3f78  FLD  [00CE3D30] -> [ESP]         ; 0.6
009c3f81  CALL 00BD2F10                    ; uniform(0.6, 0.8)
009c3f86  FMUL float ptr [EBP + 0x268]
009c3f97  FSTP float ptr [ESI + 0xb4]      ; +B4h
009c3f8c  MOV  EBP,dword ptr [ESI + 0x8]   ; reloaded for the second draw
009c3f9d  FLD  [00CF4848] -> [ESP+4]       ; 1.8
009c3fa7  FLD  [00D06BB4] -> [ESP]         ; 1.6
009c3fb0  CALL 00BD2F10                    ; uniform(1.6, 1.8)
009c3fb5  FMUL float ptr [EBP + 0x268]
009c3fe8  FST  float ptr [ESI + 0xb8]      ; +B8h, no pop
009c3ff5  FSTP float ptr [ESI + 0xbc]      ; +BCh, the same draw
```

`009C3F5D` is the only write to EBP between the tuning load and `009C3F86`, and `009C3F8C` reloads
the same pointer before `009C3FB5`, so both multiplies take `classDesc+268h` and neither takes
`tuning+268h`; all four constants are `FLD float ptr` and read 0.6, 0.8, 1.6, 1.8 at that width.
Pinned at the low end of each draw, as this host pins every draw. `009C8A5E`'s floor
`max(itself, AttackDist * task+41Ch)` = 1100 m is then inert, which is how the 1100 came to stand in
for both fields in the first place. The attackrun census prints the result rather than the source
constant: `b4=780.0 b8=2080.0`, i.e. 0.6 and 1.6 times this installation's `TurnCircleRadius` 1300.

**Measured, B -> C.** `+B8h` is where the fly-over starts and `+B4h` is `009FBA50`'s `range_low`, so
both the geometry and the run-in's commanded altitude move:

| | window B | window C |
| --- | --- | --- |
| fly-over starts at | 1091-1098 m | **2073-2080 m** |
| run-in commands | 1000-1003 m | **1300-1302 m** |
| fly-over entry altitude | 1125-1154 m | **1393-1403 m**, all fifteen |
| fly-over exit | aimglide 663-667 m @ 7-15 m (`movieval`), 662-666 m @ 6-14 m (`#1.1`), **goaway** for `#3.1`/`#7.1` | **aimglide** 662-666 m @ 150-368 m, all fifteen |
| `goaway` exits | 6 of 15 | **0 of 15** |
| releases / bombs | 0 / 0 | 0 / 0 |

Binding `+B4h` **removes the `+1Ah` goaway exit from this mission entirely**: the six aircraft of
`#3.1` and `#7.1` that were taken out of the race at 695-732 m now fly the whole fly-over, and
`#5.1`, which used to still be in the state at frame 4800, now completes it. That is the one
unambiguous gain of the window, and it is a consequence of `+B4h` feeding the goaway tolerance, not
of the attack geometry.

Against that, the aircraft now has 2080 m of fly-over to descend through instead of 1095 m, and the
extra 268 m of entry altitude does not pay for it.

### The race, in the one arithmetic all three windows obey

This host's response to the fly-over's commanded 210 m is the same in every window: the descent
across the state, altitude lost over planar range covered, is

```
A  #1.1      (1126 - 666) / (1095 - 14)  = 0.426
B  movieval  (1127 - 663) / (1098 - 7)   = 0.425
C  movieval  (1394 - 665) / (2079 - 368) = 0.426
```

so to still be above the can-dive height `approach+D4h` = 675.0 m when it passes over its target, a
bomber must enter the fly-over above `675 + 0.426 * L`, where `L` is the fly-over's own length. The
whole packet reduces to that one comparison:

| window | aircraft | `L` | needed entry | actual entry | short by |
| --- | --- | --- | --- | --- | --- |
| A | `movieval` | 1093 m | 1141 m | 729 m | **412 m** |
| A | `D3A Val #1.1` | 1095 m | 1141 m | 1126 m | **15 m** |
| B | `movieval` | 1098 m | 1143 m | 1127 m | **16 m** |
| C | every aircraft | 2078 m | 1560 m | 1395 m | **165 m** |

The climb angle closed `movieval`'s gap from 412 m to 16 m; the attack distances, which are the
faithful values, reopened it to 165 m for everyone. **The bearing arm won for no aircraft in any
window**, so this set does not restore the dive and the branch is not mergeable on its own measure.

### What is still a substitution, and which one is now the whole question

`approach+ACh`, the ordered cruise altitude `ctl+398h` = `Pilot/DiveBomb/BeginAltRange/1`, pinned at
`kPilotDiveBombBeginAltRange1 = 1000.0f`. It is not one lever among several - it is the only one
left that moves the entry altitude, because:

* it *is* the run-in's commanded altitude (`cin.base_altitude = db_begin_alt_ac + db_aim_point_height_50`,
  and window C measures `cmd = +ACh + 302` with the span term the bound `+B4h`/`+B8h` now produce,
  and an entry altitude of `+ACh + 395` once the aircraft's tracking lag is included);
* it sets the can-dive height itself, `approach+D4h = max(+A8h + 250, (+ACh + +A8h) * 0.5)`, which is
  675.0 m only because `+ACh` is 1000 here.

Putting the measured `+395` entry lag and the measured 0.426 descent into the can-dive height gives
the condition for the wingover to exist at all, with `+B8h` at its bound 2080 m and `+A8h` pinned at
350 m:

```
+ACh + 395  >  max(600, (+ACh + 350) / 2) + 0.426 * 2080
            ->  +ACh > 1332 m
```

**This is an extrapolation from three runs of this host, not a reading of the image**: it assumes the
0.426 descent and the 395 m lag hold as `+ACh` moves, and both are this reconstruction's flight
model rather than recovered behaviour. What it does establish is where the next reader should go.
At the pinned 1000 the dive is arithmetically unreachable; somewhere above about 1330 it is
comfortable; and `ctl+398h` is per-unit, which is exactly the freedom needed for the scripted
`movieval` and a spawned `D3A Val` to behave differently. `cc8_dive_glide`'s retraction already
pointed here - "the next reader should start at `ctl+398h`: who writes it" - and this packet turns
that from a loose end into the load-bearing unknown.

Two further substitutions are named for completeness and are **not** candidates on these numbers.
`approach+A8h` is pinned at 350 by the integrator's decision rather than drawn from `uniform(350, 450)`;
at 450 the can-dive height would be 700 m and the gap would widen, not close. And the aimglide's own
steering remains blocked - `bearing` stops 590-610 of ~640 calls in window C, best error 0.0016 rad
against a 0.5236 rad gate, `passed=0` - so even the aircraft that do reach the glide cannot release,
which is the defect `cc8_dive_entry` recorded and this packet reproduces unchanged.

### Status of the branch

`agent/cc8-dive-race` carries `agent/cc8-dive-entry` plus this packet's two bindings. Both bindings
are faithful and both are improvements in their own terms - the climb angle is proved from the
listing to be a host defect that could not have been a transcription, and the attack distances
remove six spurious `goaway` exits. Neither restores a release, and the set as a whole leaves the
mission at `releases=0 bombs_spawned=0` against `main`'s six drops. **The branch remains not
mergeable on the dive's own measure**, and the reason is now a single named number rather than three.

### RETRACTION, same packet: `approach+ACh` cannot be the lever, because the image never puts it there

Packet `cc8_dive_race` withdraws the closing paragraph of the section above - "Putting the measured
`+395` entry lag and the measured 0.426 descent into the can-dive height gives ... `+ACh > 1332 m`"
- as an answer. The arithmetic stands; what it names does not exist. I extrapolated a required value
for `approach+ACh` without first reading what produces it, which is the producer-before-consumer
rule this document has now broken three times.

`009C8920` `BSP_BotTaskDiveBomb_UpdateCruiseProfile` is the writer, and it is bounded:

```
009c8977  CALL 0042E740                  ; the tuning singleton -> EAX
009c897c  FLD  float ptr [0x00CE5380]    ; 15.0
009c8988  SUB  ESP,0x8
009c898b  FSTP float ptr [ESP + 0x4]     ; arg2 = 15.0
009c8994  FLDZ
009c8998  FSTP float ptr [ESP]           ; arg1 = 0.0
009c899b  CALL 00BD2F10                  ; uniform(0.0, 15.0)
009c8996  MOV  EBP,EAX                   ; the singleton, and the ONLY write to EBP
009c89a0  FADD float ptr [EBP + 0x4cc]   ;   between 009C8977 and here
009c89ad  FSTP float ptr [ESP + 0x10]
009c89ce  MOVSS dword ptr [EDI + 0x398],XMM0
```

so `ctl+398h = tuning+4CCh + uniform(0, 15)`. `00CE5380` is 15.0 read at the `FLD float ptr` width.
`tuning+4CCh` is `Pilot/DiveBomb/BeginAltRange/1`, which `docs/GAME_TUNING_SINGLETON.md` row `+4cc`
gives as **1000** in this installation (producer `007E9FE2`). The field has no other producer that
could raise it: `00939E83`, in `BSP_UnitController_ConstructVariantB`, **zeroes** it
(`00939E77 XORPS XMM0,XMM0` then the store, with `+394h` zeroed beside it at `00939E8B`). A scan of
the whole image for `F3 0F 11 ?? 98 03 00 00` returns **19** `MOVSS` stores to some `+398h`;
`009C89CE` is the only one inside the dive-bomb task and `00939E83` the only other one on the unit
controller, the remaining seventeen being mission-tree, HUD, shadow and ship-AI objects. The
`D9 ?? 98 03 00 00` x87 form returns only ship-AI and shader sites. The pattern demonstrably occurs
elsewhere, so neither negative is vacuous. **Not checked**, and the one worth checking:
`0079CD26` in `BSP_FlightLeaderControlBlock_GetOrCreate_Provisional`, which I assumed is a different
struct on its name alone. (The first reading of this scan said "twelve" from the tail of capped
output; the count is 19.)

**So `approach+ACh` is 1000 to 1015 m and this host's pinned `kPilotDiveBombBeginAltRange1 = 1000.0f`
is faithful to within 15 m.** The condition `+ACh > 1332` is not a target for a future packet; it is
a proof that *no* value of this field reaches the dive, since the authored maximum, 1015, leaves the
bomber 150 m short by the same arithmetic.

**A naming correction that caused it.** `009C8920` writes *two* altitudes and this document has been
calling both "the ordered cruise altitude". `009C8964 FLD [EAX+4C0h]` / `009C896E FSTP [ECX+18h]`
puts `Pilot/DiveBomb/CruisingAlt` = 1300 into `ctl+394h`, while `009C89CE` puts
`BeginAltRange/1 + uniform(0,15)` into `ctl+398h`. Only `+398h` reaches `approach+ACh`
(`009C7AA7 FLD [EAX+398h]` / `009C7AAD FSTP [ESI+0ACh]`). The 1300 is a real authored cruise
altitude that this path never uses.

### What is left, and it is inside the arm that has to win

With the entry altitude, the two attack distances, the climb angle, the 210 m fly-over target and
the two height gates all now either bound or shown faithful, the only substitution still standing
between the run-in and the wingover is **the fly-over's commanded heading itself** -
`009C6DC8`'s `AddWrappedAngle(base, clamp(delta, -L, +L))`, with the clamp built at
`009C6D7E`-`009C6DB0`, which `src/game_hosts_units.cpp` replaces with the bearing to the aim point
and labels as a substitution.

This packet adds one listing observation that makes that substitution look decisive rather than
cosmetic. `009C6CEC`-`009C6D3B` is an arm nobody has written up:

```
009c6cec  FLD  float ptr [ESP + 0x10]
009c6cf0  FSUB float ptr [ESP + 0x74]        ; a heading difference
009c6cfe  FCOMIP ST0,ST1                     ; ... taken to its absolute value
009c6d14  SUBSS XMM0,dword ptr [ESP + 0x10]  ;     through the -0.0 at 00D7A208
009c6d1a  COMISS XMM0,dword ptr [0x00D7A238] ; against 0.01 (m32)
009c6d21  JBE  0x009c6d29
009c6d23  MOV  byte ptr [ESI + 0x19],0x0     ; NOT converged -> clear the roll-in flag
009c6d29  COMISS XMM1,XMM2                   ; converged -> choose the side instead
009c6d2e  OR   EAX,0xffffffff / 009c6d36 MOV EAX,1
009c6d31  MOV  dword ptr [ESI + 0x20],EAX    ; flyabove+20h, which 009C8563 reads
```

The threshold is **0.01 rad**, and the flag it clears is the same `+19h` the transition rule reads,
and the field it sets on the other branch is the roll side `flyabove+20h` that `009C8563` consumes
when the turndown is entered. Read with the substituted heading, that says the image's fly-over is
"**turn onto a computed heading, and roll in once you are on it to within 0.01 rad**", not "fly at
the aim point until the bearing swings past 1.6 rad" - and it is the second reading that this
host's substitution forces, because a bomber commanded straight at its target is converged on that
heading throughout and reaches a large bearing error only by flying over the target, which is
exactly where every aircraft in windows A, B and C ran out of altitude.

**This is a hypothesis, and it is the only one this packet leaves.** Not established: every write to
`flyabove+19h` (there are at least two more, at `009C67A9` and `009C66E7`); the producers of
`[ESP+10h]` and `[ESP+74h]` at `009C6CEC`, which I did not trace back to their writes; and the whole
of `009C6D7E`-`009C6DB0`, where `base`, `delta` and the limit `L` are built. Anyone taking it should
start there and should not assume, as I did once already in this packet, that a number can be
extrapolated before its producer is read.

### Check 1: the authored numbers, and what their own comments say the two attacks are

Packet `cc8_dive_race`. The tuning rows the dive-bomb approach reads are authored in
`scripts/datatables/planeglobals.lua`, mtime **2024-10-29 12:54:18**, one of only two files in
`scripts/datatables` at that date (29 of the 35 are the 2024-07-13 bulk):

```lua
["DiveBomb"] = -- divebomb parancs parameterei. a tobbi, skill fuggo parameter a Robots.Lua-ban
{
    ["AttackDist"]  = 1100,             -- ilyen tavolsagbol bomlik fel a formacio, es kezdenek onalloan tamadni
    ["CruisingAlt"] = 1300,             -- utazo magassag (meter)
    ["BeginAltRange"] = { 1000, 1200 }, -- e ket magassag kozott kezdi meg a leboritast
    ["SafeDist"] = 100,
    ["MoveOnCruisingAlt"] = true,
    ["ReferenceSpeed"] = KMH(280)
}
```

So `tuning+4CCh` = **1000** and `tuning+4D0h` = **1200**, and with `009C89CE`'s
`tuning+4CCh + uniform(0, 15)` the per-squadron `ctl+398h`, hence `approach+ACh`, is **1000 to
1015 m** for scripted and spawned aircraft alike. `kPilotDiveBombBeginAltRange1 = 1000.0f` is
therefore not a stand-in to be replaced but a **proof**, and so are `AttackDist` 1100 (`tuning+4C4h`,
the floor that was mistaken for `+B4h`/`+B8h`) and `ReferenceSpeed` KMH(280) (`tuning+4D8h`).

**The release band is not a mod value.** `scripts/datatables/robots.lua` has mtime 2025-06-01
16:03:10 with `robots__.lua.bak` at 16:03:09 beside it, so that file *was* rewritten by a tool in
this installation. Both copies carry `DiveBombReleaseAlt = { 350, 450 }` and
`DiveBombNewReleaseMul = 0.6` identically, at the same lines 568 and 569. Uncertainty, stated: the
`.bak` is the state before *that* rewrite, not proven to be retail.

**And their comments name the two attacks, which is the finding.** The authored Hungarian
distinguishes two dive-bomb modes by name, and this reconstruction's two states are exactly them:

| row | comment | reading |
| --- | --- | --- |
| `DiveBombReleaseAlt = {350, 450}` | "regi tipusu, **leboritos** bumbazasnal a bomba oldasi magassag valahol a ketto kozott" | old-type, **roll-over** bombing: release somewhere between the two. This is `approach+A8h`, and it belongs to the **wingover** - `turndown` + `aimdive`. |
| `DiveBombNewReleaseMul = 0.6` | "ha **nem leboritott** manoverrel bombaz, **csak siman rarepulve**, akkor a fenti ReleaseAlt erteket ennyivel megszorozva hasznalja" | if it bombs **not** with the roll-over manoeuvre, **just plainly flying at it**: use ReleaseAlt times this. This is the `aimglide`'s release altitude, 210 m. |
| `BeginAltRange = {1000, 1200}` | "e ket magassag kozott **kezdi meg a leboritast**" | between these two altitudes it **begins the roll-over**. |

### So the image does NOT glide by design, and the arithmetic indicts a different term

The reframing this check was asked to test - that with this installation's numbers the image itself
never takes the wingover and its dive bombers glide-bomb - is **not supported, and the authored data
says the opposite.** `BeginAltRange`'s own comment is that the roll-over *begins* in the 1000-1200 m
band, and an aircraft at 1000 m clears the can-dive flag `+790h` (`B > approach+D4h` = 675 m) with
325 m to spare. The `0.6` is not the normal case; its comment marks it as the multiplier for the
attack that is explicitly *not* the roll-over.

What the arithmetic actually indicts is the fly-over's **commanded altitude**, not its entry
altitude. The altitude arm computes `C` from the begin altitude and then, at `009C655F`-`009C657C`,
replaces it with `R = NewReleaseMul * ReleaseAlt` = 210 m whenever `C > 1.1 * R` - that is, it
replaces the **roll-over's** begin altitude with the **glide's** release altitude, on the path whose
own data says the roll-over starts here. A bomber that holds 1000 m across a 2080 m fly-over passes
its target above 675 m and rolls in; a bomber commanded to 210 m arrives at roughly 114 m and cannot.
All three of this packet's windows measured the second.

**The next question is therefore whether the `C := R` clamp at `009C657C` is guarded**, and not the
aimglide's steering. The prediction worth one read: something upstream of `009C655F` selects the
glide ceiling only for an aircraft already committed to the glide - a `+790h`, a `+D1h`, or the
`approach+0Ch` test at `009C64A6` doing more than choosing between two sources of `C`.
**Not established.** The walk recorded in "The altitude arm the host did not have" shows no such
guard, and it covered a 949-instruction body; a negative from it is not yet a proof. If there is no
guard, then the contradiction is between the image's code and its own data table, and the
interesting question becomes which of the two the shipped game actually honours.

**On the descent rate, what is proved and what is not.** The 0.426 used in this packet's arithmetic
is not merely a host artefact: `009C6E10`'s dive branch commands `009FB800` with `ref = 0.8` at every
geometry this mission produces, `009FB800` returns `-min(DropAngle * t, limit)` with `t` clamped to
that reference, and the census prints the command as -0.419 rad = 24.0 deg, against which this host
*achieves* 23.1 deg - it tracks the commanded pitch to within a degree. So the angle in the
arithmetic is the **image's commanded** angle. What is **not** determinable from the listing is
whether the image's own flight model tracks that command as closely as this one does, and therefore
whether the image's entry lag matches the +395 m measured here.

### Successor brief for `cc8_dive_race` (cold start)

Start in a **fresh worktree from `agent/cc8-dive-race`**, not from `main`: the branch is
`main f4eb153de` + `agent/cc8-dive-entry` + this packet, and it is **held unmerged by the
integrator's decision** until something releases. Its commits are `6faaaf733` (the merge, which
resolved one `docs/DIVE_BOMB_TASK.md` conflict by keeping both appended sections), `dc12a4b81` (the
two bindings), `77693c324` (the `+ACh` retraction), `837b54c73` (the authored rows).

**The three measurement logs**, all 4800-frame USN04, all with the clean shutdown line, in this
worktree's `local\`. They are before/after windows on the same tree, one binding apart:

| log | binary | headline |
| --- | --- | --- |
| `race_base.log` | the merge, unchanged | reproduces `cc8-dive-entry`'s `entry_rearm.log` to the digit |
| `race_climb.log` | + the climb angle | `movieval` entry 729 -> 1127 m, exit 849 -> 7 m |
| `race_dist.log` | + `+B4h`/`+B8h` | fly-over starts at 2079 m, all fifteen enter at ~1395 m, `goaway` 6/15 -> 0/15 |

All three end `aircraft=15 releases=0 bombs_spawned=0`.

**Settled, do not re-derive.** `009FB800` takes two floats (`009FB96B RET 0x8`) and fetches both the
climb gain `desc+1ECh` and `DropAngle` `desc+1F0h` itself through `[[ESI]+4]+538h`, so no caller
passes an angle. `approach+B4h` = `uniform(0.6,0.8) * desc+268h` and `+B8h` = `+BCh` =
`uniform(1.6,1.8) * desc+268h`, `009C3F5D` carrying the descriptor. `approach+ACh` = 1000 to 1015 m
and the host's pin is faithful. The height arm of `+791h` fires at `B <= 666.7` and the turndown
needs `B > 675`, so **only the bearing arm can produce a dive.** This host's fly-over descent is
0.426, against a commanded 24.0 deg it tracks to within a degree.

**Two open reads, in the order I would take them.**

1. **Is the `C := R` clamp at `009C657C` guarded?** The fly-over commands 210 m - the *glide's*
   release altitude - on the path whose own data table says the *roll-over* begins at 1000 m. Read
   `009C6491`-`009C657C` for a guard upstream of `009C655F` (a `+790h`, a `+D1h`, or the
   `approach+0Ch` test at `009C64A6` doing more than choosing between two sources of `C`). The
   earlier walk records no guard but covered a 949-instruction body, so that negative is not a proof.
   If there is a guard, binding it is probably the whole packet.
2. **The fly-over's commanded heading**, `009C6DC8`'s `AddWrappedAngle(base, clamp(delta, -L, +L))`,
   which `src/game_hosts_units.cpp` replaces with the bearing to the aim point, labelled.
   `009C6CEC`-`009C6D3B` clears `flyabove+19h` until a heading difference is within the 0.01 at
   `00D7A238`, and on the other branch writes the roll side `flyabove+20h = +/-1` that `009C8563`
   consumes on entering the turndown - which reads as "turn onto a heading, then roll in".
   **Unread:** the other `+19h` writers `009C67A9` and `009C66E7`, the producers of `[ESP+10h]` and
   `[ESP+74h]` at `009C6CEC`, and all of `009C6D7E`-`009C6DB0` where `base`, `delta` and `L` are built.

Both are hypotheses. Neither has been measured.

**Boundaries.** `src/game_hosts_units.cpp` is split by hunk: the dive-bomb attack-state inputs and
binding sites, the approach constructor values and the dive-bomb instrumentation are this packet's;
the transition law, the break-off evaluation and `dive_bomb_transition_inputs`'
`control_mode_370` belong to `cc8-done-state`, whose work lands on `main`, so expect `movieval`'s
post-release states to change when `main` is next merged in - **re-run the baseline on your own tree
rather than comparing against the logs above.** `src/dive_bomb_task.cpp` and
`include/bsp/dive_bomb_task.hpp` are split the same way. Predecessor trees are readable read-only:
`...-cc8-dive-glide\local\g.ps1` (a frame-base walk of the aimglide `009C5180`) and
`...-cc8-dive-bomb\local\t.ps1` (the aimdive `009C58D0`).

**Still owed elsewhere.** The image bursts a bomb on a water or terrain impact - `0084BF00` passes
`1` for an entity at `0084C314` and `2` at `0084C3CF` (`0084C286 MOV EDI,2`), gated per class by
`classDesc+74h ExplWaterHit`, with `0084BC60` step 7's radial explosion needing
`classDesc+6Ch != 0`. This host bursts on entity impacts only; the integrator has queued it.

## Packet `cc8_dive_heading`: the fly-over walked whole, and the span is a range, not a height

`009C62B0`-`009C7083`, all 949 instructions, walked from the listing with x87 depth and ESP tracked
over every edge (`local/f.ps1` driving `local/x87trace.py`, re-seeded from `cc8-dive-glide`'s aimglide
walk). Frame base **0x98**: `SUB ESP,88h` at `009C62B0` plus `PUSH EBX/EBP/ESI` and the `PUSH EDI` at
`009C62C6`. Every callee effect is read from that callee's own tail rather than assumed:

| callee | `RET` | x87 | read at |
| --- | --- | --- | --- |
| `009FB800` `BSP_PilotBot_CommandPitchFromAltitude` | **8** | void | `009FB94B`, `009FB96B`, `009FBA4D` |
| `007F0280` `BSP_Bot_NearFieldUnitAvoidanceProbe` | 18h | void | `007F0B1F` |
| `009FA2E0` | 4 | none | `009FA346` |
| `009FABE0` | 8 | none | `009FACAE` |
| `007C4810`, `0099B630` | 0 | float in ST0 | `007C4829` `FLD [ESP]`; `0099B639`/`0099B646` `FLD` |
| `00419010`, `00438AA0`, `00438B10` | 14h, 8, 8 | float in ST0 | as the aimglide walk |

`009FB800`'s `RET` is worth naming: a linear scan finds a `RET 10h` at `009FBB1A`, but `009FBB13`
`CALL 009FB800` sits three instructions above it, so that epilogue belongs to a **wrapper**, not to
`009FB800`. Reading the first `RET` a scan reaches would have put eight bytes of phantom cleanup into
every frame after `009C6F7D`.

Five of the six indirect calls push one out-pointer and clean four bytes. `009C6404` pushes **nothing**
and returns a float in ST0; with it modelled as `0:4` the walk reported forty-odd negative-depth
faults. With the table above the walk reports **zero join conflicts, zero unknown call targets, zero
notes, and both `RET`s at depth 0** over the whole body.

`ESI` is written exactly once, `009C62B9 MOV ESI,ECX`, and restored by the two epilogues, so every
`[ESI+n]` in the body is a fly-above field. This also settles the frame worry left in
"`009C62B0`'s command side": `009C6336` writes base `[ESP+50h]` at `fb=0x9c` and `009C6DC1` reads base
`[ESP+4Ch]` at `fb=0xa0`. Two different slots four bytes apart, exactly as that section suspected but
could not resolve.

### `009C65FD`'s span is `max(R - S, 0)`, and `R` is the planar range

This is the correction the packet exists for. `docs/HANDOFF_DIVE_BOMB_ENTRY.md` (a), "The two flyabove
flags, bound" and `src/dive_bomb_task.cpp` all had the span as `max(B - S, 0)` with `B` the height
above the aim point. The x87 stack says otherwise, walked forward from `009C64EE` with every push and
pop accounted:

```
009c64ee FLD [ESP+28h]      ST0=R
009c64f4 FLD [ESP+38h]      ST0=B ST1=R
009c64f8 FLD [ESP+3Ch]      ST0=X ST1=B ST2=R    ; all four BL paths rejoin at 009C6532 with this
009c6578 FCOMPI ST(1) pop / 009c657a FSTP ST(0)  -> ST0=B ST1=R
009c659b FCOMPI ST(1) pop / 009c659d FSTP ST(0)  -> ST0=R      ; B is discarded HERE
009c65d1 FSTP [ESP+10h]     ST0=R                ; [ESP+10h] = S
009c65d5 FLD  [ESP+10h]     ST0=S ST1=R
009c65d9 FLD  ST(0)         ST0=S ST1=S ST2=R
009c65db FSUBP ST(2)        ST2 = R - S
```

`[ESP+28h]` is the planar distance, written **once** at `009C63A6` from the `00BF7030` square root at
`009C6399`, and never overwritten anywhere in the 949 instructions (census: two writes, `009C63A6` and
the zero at `009C63B1`, and nine reads). `[ESP+38h]`, the height `B`, is consumed by `009C659B`'s
compare and dropped by `009C659D` before the subtraction ever happens. The threshold still comes from
the height - `S = 0.7 * max(B, 100) + 200`, `00D7A220`/`00CE3D08`/`00CEFFA0`/`00CE4D70` - so the
function takes **two different quantities**, which is what made the single-argument reconstruction
look plausible.

**Where "666.7 m" came from.** With the height as the minuend, `max(B - (0.7B + 200), 0)` is zero at
exactly `B = 666.7`, and that number was then read back as a height gate and compared with
`approach+D4h` = 675.0 m, eight metres away, which looked like corroboration. It is not a number in
the image. The image's second `+19h` arm is **`R <= 0.7 * max(B, 100) + 200`**: a range-to-go test
against a glide slope, with no relation to `+D4h`.

That is why the height arm won the race for every aircraft in `cc8_dive_race`'s three windows. In
window C `movieval` enters the fly-over at `alt=1394 rng=2079` and hands over at `alt=665 rng=368`;
with the range test and the measured 0.426 descent the arm fires where `R = 0.7*alt(R) + 200`, which
is `R = 792 m` at `alt = 846 m` - **171 m above `+D4h`, so `+18h` is 1 and the hand-over is the
turndown, not the glide.**

One trap for the next reader: base`[ESP+30h]` holds the span only until `009C6853`, where the bank arm
overwrites it with `classDesc+268h * 1.4` (`TurnCircleRadius` times the qword 1.4 at `00D045F0`). Every
read of `[ESP+30h]` from `009C6881` onwards is a turn radius, not a span. The `+19h` test at
`009C67A9` is before that write and does read the span.

### The commanded heading: `C + clamp(turn, +/-L)`, and the turn is a dead-banded bearing error

```
C   base[ESP+4Ch]  the aircraft's heading, written ONCE at 009C6406 from the 009C6404 vtable[50h] float
A   base[ESP+44h]  the desired heading
    009C63FE  A = base[ESP+3Ch] = wrap(pi/2 - atan2(z,x)), the bearing; +2pi at 009C63E0 (00CE3828)
    009C6A9F  A = AddWrappedAngle(C, base[ESP+1Ch])   when 009C6A46 `76` JBE is NOT taken
    009C6D59  A = AddWrappedAngle(A, base[ESP+20h])   the avoidance increment
L   base[ESP+34h]  0.1745329 (10 deg, 00CE3990) at 009C6497, then 1.5707964 (pi/2, 00CE3C64) at
                   009C65A3 and again at 009C682C. Which one is live at 009C6D78 is a path
                   question: 009C6544's `75` JNE, taken when flyabove+1Bh != 0, skips 009C65A3,
                   and 009C682C runs only on the BL != 0 arm - so L is 10 degrees exactly when
                   `+1Bh != 0 && BL == 0`, and pi/2 otherwise. The 10-degree rate limit is the
                   corner, not the common case.
delta = SubtractWrappedAngle(A, C)                    009C6D6F -> base[ESP+1Ch]
009C6D78-009C6DB0  clamp(delta, -L, +L); 009C6D7E FCHS builds -L; 009C6D8E and 009C6DA0 both `76` JBE
009C6DC8  AddWrappedAngle(C, clamped) -> base[ESP+44h] -> cmd+2C0h at 009C6DE7, mode 2 at 009C6DEF
```

The turn at `009C6A37`-`009C6A7F` is a **symmetric dead-band on the signed bearing error**, of
half-width `T` = base`[ESP+20h]`. `base[ESP+1Ch]` is the bearing error
`E = SubtractWrappedAngle(bearing, heading)` written at `009C641C` and not rewritten between there and
`009C6A37`; and `XMM0` is **0.0** on every path that reaches `009C6A43` - the four writes that can
precede it, `009C67BC`, `009C67FF`, `009C690E` and `009C699C`, are all `XORPS XMM0,XMM0`, and nothing
non-zero is written to `XMM0` between `009C699C` and `009C6A43`. So

```
009c6a43  T <= 0            -> skip the whole arm; A stays the raw bearing of 009C63FE
009c6a48  E > 0             -> A = C + max(E - T, 0)
          E <= 0            -> A = C + min(E + T, 0)
```

`T` has **three producers and only one of them is read**, which is the honest state of this arm:

* `009C6674`, `T = InterpolateClamped(0.0, 0.5235988, 200.0, 0.0, span)` - 30 degrees at span 0
  falling linearly to 0 at span >= 200 m. This is what reaches `009C6A43` on the paths that leave the
  body early, i.e. the `BL == 0` jump at `009C67C1` and the two `009C6A35` jumps.
* `009C64A0`, `T = 0.0`, on the `flyabove+1Bh != 0` path where `009C6544`'s `75` JNE skips `009C6674`
  altogether. On that path the dead-band is inert and `A` is the bearing.
* `009C6893` and `009C6911`, inside the bank arm, which is the **common** path in this mission because
  `BL` is 1 while `R > approach+B4h`. `009C6911`'s value is
  `InterpolateClamped(?, 00CEDD00, classDesc+268h TurnCircleRadius, 0.1745329, ?)`; its first and
  fifth arguments come off the x87 stack from `009C68DD` and are **NOT traced**. `009C6893`'s
  producer is likewise unread.

So the statement that can be made from the listing is the shape, not the value: the fly-over commands
`C + clamp(deadband(E, T), +/-L)`, and this host's `A = bearing` is exactly right whenever `T <= 0` and
approximate otherwise. What it is **not** is an independent heading with a convergence test, which is
what `cc8_dive_race`'s closing hypothesis proposed. The heading is therefore not the lever, and the
packet does not bind it: two of its three inputs are untraced and the third, the avoidance increment,
runs through the unbound `007F0280`.

### A sixth flag: `flyabove+1Ch` is written, and this host's contract says it never is

`009C6919 MOV byte ptr [ESI+1Ch],1`, reached from `009C68D4`'s `76` JBE inside the bank arm, and
`009C691F`/`009C6923` then branch on it: `+1Ch == 0` goes to the dead-band at `009C6A37`, `+1Ch != 0`
carries on into `009C6929`. The same byte is what `009C6DCD` tests and `009C6DDA`'s `75` JNZ uses to
**skip the heading write entirely**. `include/bsp/dive_bomb_task.hpp` records
`suppress_heading_1c` as "a contract: this host keeps no flyabove `+1Ch`, so it never suppresses" -
that is now a known hole rather than a contract, and it is the first thing to read after this packet.

### `009C6CEC`'s 0.01 is inside the avoidance arm, and the earlier hypothesis is withdrawn

`cc8_dive_race`'s closing section proposed that `009C6CEC`-`009C6D3B` means "turn onto a computed
heading, and roll in once you are on it to within 0.01 rad". It does not, and the producers say so.

`009C6B3A` calls `007F0280` `BSP_Bot_NearFieldUnitAvoidanceProbe` with six pushes matching its
`RET 18h`: `arg1 = [EAX+4]`, `arg2 = &base[ESP+5Ch]`, `arg3 = &base[ESP+68h]`, `arg4 = &base[ESP+74h]`,
`arg5 = &base[ESP+80h]`, `arg6 = 1`, `ECX = [EAX+0Ch]`. `arg2`'s three floats are the probe extents and
**`flyabove+18h` picks the set**: `(60, 70, 90)` when `+18h != 0` and `(80, 70, 140)` when it is zero
(`009C6AA7`-`009C6B12`; the run-in site at `009C4260` uses `(80, 60, 120)`).

`009C6B45`-`009C6B75` takes `|out3[0]|` by masking the sign bit and compares it with the **double 0.05**
at `00D7A270`; on `<=` the code jumps to `009C6D40` with `base[ESP+20h] = 0.0`, i.e. no avoidance turn.
Otherwise `009C6B86` stores `-out3[0]` into `base[ESP+10h]`, and the slot census shows the next write to
that slot is `009C6CD8`, the ternary itself. So at `009C6CEC`:

* `base[ESP+10h] = (out3[0] > 0) ? out3[0] : -out3[0]` = `|out3[0]|`. The `009C6CE0`/`009C6CE6` pair is a
  genuine load-store no-op; the ternary is `fabs` emitted a second time.
* `base[ESP+74h] = out4[0]`, another output of the same probe.

The test is `| |out3[0]| - out4[0] | <= 0.01` (`00D7A238`, float, `009C6D1A` `COMISS`, `009C6D21` `76`
JBE), and the arm is gated at `009C6CAE`-`009C6CC8` on `+19h != 0 && +18h != 0 && +20h == 0`. It is a
once-only latch that clears `+19h` while an avoidance turn is still converging and, when it has,
latches the roll side `+20h = (out3[0] < 0) ? +1 : -1` (`009C6D2C` `72` JB). Nothing in it is a heading
and nothing in it names the aim point.

### The `+1Ah` tolerance does reproduce; the check was reading the wrong call

The hand-over brief flagged `InterpolateClamped(0, 20 deg, W, pi, x)` as not reproducing "from the
constants at the `009C666F` call, which read `(0.0, 0.5236, 200.0, 0.0)`". There are **two** `00419010`
calls in that block and the tolerance is the one at **`009C663E`**: `[ESP+0] = 0.0` (`009C6639` FLDZ),
`[ESP+4] = 00CE398C = 0.3490659` (20 degrees), `[ESP+8] = approach+B4h * 0.8 - S` (`009C6615` FLD
`[EBP+B4h]`, `009C661B` FMUL qword `00CE3D40` = 0.8, `009C6621` FSUBRP), `[ESP+Ch] = 00D7A264 = pi`,
`[ESP+10h] = span`. `009C666F` is the dead-band's `T` above, whose result goes to `base[ESP+20h]`.
The write-up stands unchanged; nothing is retracted there.

### Every write to the fly-above flags, with its condition

`EDI = ESI+4` and `[EDI]` is the approach; `EBP = [[EDI]+4]` is the entity. `B` = base`[ESP+38h]`,
`R` = base`[ESP+28h]`, `E` = base`[ESP+2Ch]` = `|SubtractWrappedAngle(bearing, heading)|` folded at
`009C6425`-`009C6453`, `X` = base`[ESP+3Ch]` re-defined at `009C64C9` as `(approach+0Ch)->+398h` when
`approach+0Ch != 0` and `approach+ACh + approach+50h` otherwise.

| site | write | condition |
| --- | --- | --- |
| `009C659F` | `+18h = 0` | unconditional on the main path |
| `009C680E` | `+18h = (B > approach+D4h)` | `BL != 0`; `009C67BF` `74` JE skips to `009C6A37` |
| `009C66E3` | `+1Ah = 1` | `E > T1`, `T1 = InterpolateClamped(0, 20 deg, B4h*0.8 - S, pi, span)` |
| `009C66E7` | `+19h = 0` | the same edge |
| `009C66F2` | `+1Ah = 0` | `E <= T1` |
| `009C6822` | `+1Ah = 0` | the `+18h` recompute path |
| `009C67B0` | `+19h = 1` | `E > 1.6` (`009C67A7` `77` JA, qword `00CE3D48`) **or** `span <= 0` (`009C67A9` `COMISS` 0.0 against `[ESP+30h]`, `009C67AE` `72` JB skips) |
| `009C6826` | `+19h = base[ESP+43h]` | the **entry value**, saved at `009C6540`; on the `BL != 0` path this discards `009C67B0`'s set |
| `009C6A30` | `+19h = +18h` | `009C6A2B` `76` JBE not taken |
| `009C688F`, `009C68E1` | `+19h = 0` | inside the bank arm, `009C688D` and `009C68D4` both `76` JBE not taken |
| `009C6D23` | `+19h = 0` | the avoidance arm above |
| `009C6D31` / `009C6D3B` | `+20h = -1` / `+1` | converged, on the sign of `out3[0]` |

`BL`, at `009C64F2`-`009C6530`, is
`(al == 0) && (approach+D4h <= X) && ((approach+B4h <= R) || (B >= approach+D4h))`, with `al` from the
`vtable[5Ch](0x14)` query at `009C64EC`. A **fifth flag** turns up: `009C6532` `CMP [ESI+1Bh],0` with
`009C6544` `75` JNE jumps the whole leave and roll-in evaluation, and `+1Bh` is written at `009C6813`.
Nobody has written `+1Bh` up and this packet does not either.

### What is still a hole

`R` and the bearing are measured to a **lead point**, not to the target:
`(dx,dz) = vtable[34h]([[ESI+4]+4]) - 009FA2E0([ESI+4]+30h)` and then
`(x,z) = vtable[0]([ESI+4]) - 3.0*(dx,dz) - entity_pos`, the 3.0 being the qword at `00D7A2B0`
(`009C6320`, `009C6328`). `009FA2E0` is itself a forwarder to the **same** `vtable[34h]`, on
`[(approach+30h)+14h]` (`009FA2F7` `MOV EDX,[EAX+34h]`), so the two terms are one quantity taken on two
objects. `vtable[0]` on the approach is the aim point, the getter `009C5278` also uses. `vtable[34h]`
is **not identified**, so the `3.0 *` term is unread and this host measures both `R` and the bearing to
the command target's position instead. That substitution is unchanged by this packet and it moves `R`
and the bearing together.

### `R` and the bearing are taken to a three-second lead point, and `009FA2E0` is read not assumed

`009C62D1`-`009C63E6` does not measure to the aim point. It measures to where the aim point will be
relative to the aircraft in three seconds:

```
009c62cf  vtable[34h] on [[ESI+4]+4]       v_own       (out.x -> [ESP+10h], out.z -> [ESP+18h])
009c62e8  009FA2E0 on approach+30h         v_target    (overwrites [ESP+50h]..[ESP+58h])
009c62ed  009c6305                         (dx,dz) = v_own - v_target
009c6320  009c6328                         the qword 3.0 at 00D7A2B0
009c6336  009c633e                         [ESP+50h] = 3*dx, [ESP+58h] = 3*dz
009c6342  vtable[0] on the approach        the aim point
009c6346  009c6351                         aim - 3*(dx,dz)
009c635d  009c636b                         less the entity pose +FCh / +104h
009c6385  009c63a6                         R = sqrt(x*x + z*z), 1e-10 floor at 00CE3820
009c63bf  009c63e6                         bearing = wrap(pi/2 - atan2(z,x)), +2pi at 00CE3828
```

In three seconds the aircraft moves `3*v_own` and the aim point, which tracks the target, moves
`3*v_target`, so `aim - pos - 3*(v_own - v_target)` is exactly the predicted separation.
`vtable[34h]` is the velocity getter - the same slot `009C7D71` multiplies by `007BCC80`'s fall time
to build the predicted impact point.

`009FA2E0` is read rather than taken on the name. It is a three-way forwarder to the **same**
`vtable[34h]`: on `[approach+44h]` when that is non-null (`009FA2EE`-`009FA2FB`), otherwise on
`[approach+48h]` (`009FA303`-`009FA313`), otherwise it fills the out vector from the three globals at
`00F87574`/`78`/`7C` (`009FA31B`-`009FA345`). Its `this` is `approach+30h`, so the two displacements
are `approach+44h` and `approach+48h`, the approach's own target handles.

Bound together with the span, because they are one vector and they move together: the range, the
`+19h` arm, the `+1Ah` arm and the heading command all read these same two frame slots.

### The `C := 210 m` clamp at `009C657C` is NOT guarded, but two paths skip it, and one of them is named

The clamp is `009C657C`'s `76` JBE falling through to `009C6580`-`009C6589`: when
`X > 1.1 * (row+40h * approach+A8h)` = `1.1 * 210` = 231, base`[ESP+3Ch]` is replaced by 210. base
`[ESP+3Ch]` is the commanded altitude `C` - written at `009C64C9` from `(approach+0Ch)->+398h`, or
`approach+ACh + approach+50h` when there is no controller, and read at `009C6E1C`, `009C6E28` and
`009C6E5D` inside the altitude arm.

Filtering the whole listing for every branch that can reach or skip `009C6589`, there is **no mode
byte, no `+790h`, no `+D1h`, no ordnance test and no per-class flag** between `009C6491` and
`009C657C`. `009C64A6`'s `74` JE does only what the previous packet said - choose between two sources
of `C`. So the clamp is unconditional on the main path, and the image does put the glide's release
altitude on an aircraft whose data table says the roll-over begins at 1000 m.

Two branches skip it entirely, and both are worth more than the clamp:

* `009C6544`'s `75` JNE on `flyabove+1Bh` jumps to `009C67B6`, past `009C654A`-`009C67B5` - the whole
  altitude target, the leave test and the roll-in test. `+1Bh` is written at `009C6813`.
* `009C6554`'s `0F85` JNE, taken when base`[ESP+27h] != 0` **and** `BL != 0`, jumps to `009C67FF` and
  then back to `009C67D1`, again past the clamp and past the `+1Ah`/`+19h` tests. base`[ESP+27h]` is
  **`ctl+3A8h`**, a byte on the unit controller read at `009C64D1` (`MOV AL,[EAX+3A8h]` with
  `EAX = approach+0Ch`) and zeroed at `009C64DD` when there is no controller. `009C67D1` reloads it
  and its `75` JNE sends the same aircraft straight into the `+18h` recompute.

On either skip path `C` keeps the cruise altitude `009C64C9` wrote and the altitude arm commands
about 1000 m instead of 210 m - which is exactly the "holds the begin altitude across the fly-over
and rolls in" behaviour the authored comments describe. **`ctl+3A8h` is unread**, and it is the
single address to take next if the glide-versus-roll-over question is reopened. This packet does not
bind it.

### Measured: the before run, and the artefact seen directly

`local\heading_before.log`, 4800-frame USN04 on `dbda05ead` (the census commit, no rule change). It
reproduces `cc8-dive-race`'s `race_dist.log` **to the digit** on every aircraft that appears in both,
so the merged tree is a valid baseline. All fifteen dive bombers behave identically:

| aircraft | attackrun -> flyabove | flyabove -> ... |
| --- | --- | --- |
| `movieval` x3 | `alt 1393-1394 rng 2073-2079 span 218 f18=1` | **aimglide** `alt 664-665 rng 361-368 span 0 b 664-665 f18=0 f19=1` |
| `D3A Val #1.1` x3 | `alt 1395 rng 2078-2080 span 218-219 f18=1` | **aimglide** `alt 665 rng 364-366 span 0 f18=0 f19=1` |
| `D3A Val #3.1` x3 | `alt 1398-1400 rng 2077-2078 span 220 f18=1` | **aimglide** `alt 662-663 rng 207-272 span 0 f18=0 f19=1` |
| `D3A Val #5.1` x3 | `alt 1395 rng 2078-2080 span 218-219 f18=1` | **aimglide** `alt 665 rng 364-366 span 0 f18=0 f19=1` |
| `D3A Val #7.1` x3 | `alt 1400-1403 rng 2073-2077 span 220-221 f18=1` | **aimglide** `alt 664-666 rng 150-233 span 0 f18=0 f19=1` |

`summary mission dive-bomb task: aircraft=15 releases=0 bombs_spawned=0`.

The `span` column is the artefact measured rather than argued. At the fly-over entry `b=1394` gives
`S = 0.7*1394 + 200 = 1175.8` and the logged `span = 218`, which is `1394 - 1175.8` - the height
minus the threshold, to the metre. At the hand-over every aircraft reads `span=0` with `b` between
662 and 666, i.e. the crossing at 666.7, and `f18=0` because `b < approach+D4h = 675`. The range at
those hand-overs runs from 150 m to 368 m, three of the five formations well inside where the image's
range test would have fired. **Nothing in this mission ever left the fly-over on the bearing arm, and
nothing left it on a range.**
