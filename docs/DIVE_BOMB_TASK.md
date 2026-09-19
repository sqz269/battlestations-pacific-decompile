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
