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
