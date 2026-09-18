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
