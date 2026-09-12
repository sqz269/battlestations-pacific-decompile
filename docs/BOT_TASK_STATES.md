# Bot task state objects and ordnance release

Addresses: `00411E70` `00411D90` (the registry); `009C2AC0` `009C2980` `009C18C0` `009C1850`
(the shared `moveto`/`follow` states); `009A56C0` (the state setter), `009A57D0` `009A5870`
(the depth-charge transition rule), `009A66C0` (the depth-charge per-tick, no Ghidra function);
`009A36A0` `009A3770` (depth-charge `attackrun`), `009A4010` `009A3FA0` `009A48D0`
(the depth-charge release), `009A5F50` (the depth-charge approach update, partial);
`009D06D0` `009D07B0` (torpedo `attackrun`), `009D2570` (the torpedo release);
`007BBBA0` (the shared ordnance release request); `009F9980` (the approach-to-task binding).
Follow-up of `docs/BOT_TASKS.md`, which owns the task object, the thirteen classes, the 26-slot
interface and the state-name tables. Every descriptive name below is a hypothesis.

## The state class family

A state object is a small polymorphic record embedded **inside the approach controller**, which
is itself embedded in the task at `task+3F8h` (`docs/BOT_TASKS.md`). The `docs/BOT_TASKS.md`
state tables are whole-object (`task`-relative); subtract `3F8h` for the approach-relative
offsets the constructors use. `009A3090` registers depth-charge `turnto` as `ESI+3A8h` after
`ADD ESI,0x3A8` at `009A3132`, which is why the doc's table reads `+7A0h`.

Base layout, from the three constructors that build one (`009C2AC0`, `009A36A0`, `009D06D0`) and
from the five depth-charge states the approach constructor `009A4DC0` builds inline
(`009A4EC1`..`009A4FC4`):

| offset | size | written by | meaning |
| --- | --- | --- | --- |
| `+0h` | 4 | every ctor | vtable |
| `+4h` | 4 | `009C2AD4`, `009A36C6`, `009D06E6`, `009A4EC1` | the owning approach controller |
| `+8h` | 4 | zeroed | `contract: unread` |
| `+Ch` | 4 | zeroed | `contract: unread` |
| `+10h` | 4 | zeroed | `contract: unread` |
| `+14h` | 4 | zeroed | `contract: unread` |

`+18h` onwards is per-derived-class. Three shapes were read:

* `attackrun` (`009A36A0` depth-charge, `009D06D0` torpedo, identical bodies): `+18h` = `1.0f`
  (`00D7A24C`), `+1Ch` = `-(00BD2F10(0.0f, 1.0f))`, `+20h` = `0`. `+18h` is the re-roll period and
  `+1Ch` the countdown, read that way by the tick at `009A3788`/`009D07D8`. This is the same
  `(period, -phase)` pair the task base `0099C6F0` writes at `task+304h`/`+308h`, so the phase is
  a per-instance random stagger.
* `aim` (depth-charge, inline at `009A4F01`..`009A4F20`): `+18h` byte, `+19h` byte, `+1Ch` float.
  `+18h` is the "pull out now" flag the tick sets, `+1Ch` the release cooldown.
* `moveto`/`follow` (`009C2AC0`, `009C2980`): `+18h` is a second vtable (`00D20AD4` for `moveto`),
  a sub-object registered with `BSP_Observer_RegisterPair` at `009C2B0E`; `+2Ch` = the target,
  `+30h`/`+34h` = the two ranges, `+38h` = the mode word. Instances run to at least `+A0h`
  (`009A4E9C` writes `+98h` and `+9Ch` on a `009C2980`-built object).

### The interface

Seven slots, `+0h`..`+18h`, read from `00D1F64C` (depth-charge `attackrun`), `00D1F668`
(`leave`), `00D1F684` (`turnto`), `00D1F6A0` (`goaway`), `00D1F6D8` (`aim`), `00D1F6FC`
(`done` and `prepare`), `00D212C4` `00D212E0` `00D212FC` `00D21320` (the torpedo four) and
`00D20AEC` (`moveto`). `moveto`/`follow` add an eighth at `+1Ch`.

| slot | base body | established meaning | evidence |
| --- | --- | --- | --- |
| `+0h` | per class | scalar deleting destructor | Ghidra's own label on `009A47D0`, `009C3D10` |
| `+4h` | `007B3DB0` (`RET`) | **enter** | `009A56C0` calls it right after storing `+310h` |
| `+8h` | `007B3DC0` (`RET`) | **exit** | `009A56C0` calls it on the outgoing state |
| `+Ch` | overridden by all | **tick(float dt)** | `009A6741` calls it on `task->+310h`; every body is `RET 4` |
| `+10h` | `007B3DE0` (`MOV AL,1; RET`) | a predicate defaulting to true | `contract: unread`, no override read |
| `+14h` | `007B3DF0` (`RET`) | `contract: unread`; `done`/`prepare` override with `009BE590` | |
| `+18h` | `007B45E0` (`XOR AL,AL; RET`) | a predicate defaulting to false | `contract: unread` |
| `+1Ch` | `moveto` family only | **set_desired_speed(float)** | `009C18C0` calls it with the computed speed |

`007B3DB0`, `007B3DC0`, `007B3DF0` are single `RET`s and `007B3DE0`/`007B45E0` three bytes each,
read with `ghidra bytes` because Ghidra defines only `007B3DB0`.

## The registry and the transition rule

`00411E70(this, name, state)` is `__thiscall` on the approach's own container at `approach+FCh`
(`MOV ECX,EDI` with `LEA EDI,[ESI+0xFC]` at `009A309B`). Its body is one call to `00411D90`,
which is a **vector push_back of an eight-byte pair**: `+4h` begin, `+8h` end, `+Ch` capacity,
`(end - begin) >> 3` against `(capacity - begin) >> 3`. So the registry is a growable array of
`{const char* name, State* state}`, not a map.

The names are never looked up at run time: `ghidra xrefs 00D1F5C4` ("DepthCharge/attackrun")
returns one reference, the registration at `009A30DA`. Every transition below compares **state
pointers**, so the registry exists for debug display only.

`009A56C0(task, next)` is the setter, three instructions plus two indirect calls:

```
State* cur = task->+310h;
if (next != cur) {
    if (cur) cur->vtable[+8h]();   // exit
    task->+310h = next;
    next->vtable[+4h]();           // enter
}
```

The derived task constructors inline the same shape without the exit (there is no current state):
`009A52BC` stores `+310h` and `009A52C7` calls `vtable[+4h]`. `009A5870` inlines it twice more.

`0099C6F0` leaves `+310h` at `0` for `stop`, which registers no states.

### Where the tick enters

`009998A0` (the peer's `BSP_PilotBot_Update`; `this` is the **task**, proven by `0099B740(ESI)`
at `0099993C` and by `LEA ECX,[ESI+0x314]` / `LEA ECX,[ESI+0x38Ch]` at `00999970` and `00999983`,
which are the task sub-objects of `docs/BOT_TASKS.md`) calls `task->vtable[+64h](dt)` at
`009998FB` and `0099995A` — `MOV EAX,[EDX+0x64]` in both cases. **Slot `+64h`, not `+54h`, is the
per-tick entry.** That settles the first open question of `docs/BOT_TASKS.md`.

Between the two call sites `009998A0` runs the `(period, -phase)` accumulator on `task+304h`/
`+308h`: when `dt >= +308h` it adds `+304h - dt` back and runs `0099B740` (the abandon check);
otherwise it only subtracts `dt`. Slot `+64h` runs on every tick either way.

Slot `+54h` is reached from exactly two sites, `0099AE28` and `0099AEAE`, both
`MOV EAX,[EDX+0x54]; CALL EAX` with `ECX = *(bot->+58h)`, i.e. the **front task of the bot's task
vector** (`MOV ECX,[ESI+0x58]; MOV ECX,[ECX]` at `0099AE21`). There is no `CALL dword ptr [r+54h]`
anywhere in the bot segments; the only one in the image is `00C03B1B` in the CRT. Both sites sit
in an undefined region (`no_ghidra_function`); the enclosing routine writes `bot->+74h` right
after and was not read, so whether it is a push path or a per-tick pass is `contract: unread`.

### The depth-charge transition rule, `009A5870`

Called once per tick from `009A66C0` at `009A6729`, before the current state's own tick. `param_1`
is the task; `param_1[0xC4]` is `task+310h`. State pointers as whole-object offsets:
`moveto +508h`, `follow +544h`, `done +5DCh`, `attackrun +67Ch`, `goaway +6A0h`, `aim +6C4h`,
`prepare +6E4h`, `leave +784h`, `turnto +7A0h` — all nine agree with `docs/BOT_TASKS.md`.

| step | rule |
| --- | --- |
| 1 | `attacking = 009A5420(task->+310h)` (the seven non-approach states; `docs/BOT_TASKS.md` slot `+50h`) |
| 2 | `engaged = task->+46Dh != 0 \|\| (ctl->+370h == 2 && task->+48Ch != 0)` |
| 3 | `!attacking && engaged` -> `009A57D0(task)` |
| 4 | `!attacking && !engaged` -> `SetState(007B8AD0(unit) ? moveto : follow)` |
| 5 | `attacking && !engaged` -> the same `moveto`/`follow` choice, i.e. the abort back to approach |
| 6 | `attacking && engaged && ctl->+370h == 0` -> `SetState(prepare)` |
| 7 | otherwise, when the state is not already `done`: `task->vtable[+1Ch]()` (the break-off test `009A65F0`) -> `SetState(done)` |
| 8 | `prepare` -> `009A57D0(task)` |
| 9 | `attackrun` -> `task->+46Dh != 0` -> `SetState(turnto)` |
| 10 | `aim` -> `approach->+74h == 0` -> `SetState(leave)`; else `aimState->+18h != 0` -> `SetState(goaway)` |
| 11 | `leave` -> `leaveState->+18h <= 0.0f` -> `SetState(goaway)` |
| 12 | `turnto` -> `009A5610()` -> `SetState(aim)`; else `009A5390()` -> `SetState(goaway)` |
| 13 | `goaway` -> `009A53B0()` -> `SetState(task->+46Ch ? turnto : done)` |

`009A57D0(task)` is the entry chooser, `complete`:

| order | test | next state |
| --- | --- | --- |
| 1 | `ctl->+370h == 0` | `prepare` `+6E4h` |
| 2 | `task->+46Ch == 0` | `done` `+5DCh` |
| 3 | `task->+46Dh == 0` | `attackrun` `+67Ch` |
| 4 | otherwise | `turnto` `+7A0h` |

`009A5610`, `009A5390`, `009A53B0` are the three geometry predicates behind `turnto` and `goaway`;
their bodies were not read (`contract: unread`). `ctl->+370h` is a three-valued pilot-control mode
the task does not write; its producer is the motion controller, a peer's contract.

### The depth-charge per-tick, `009A66C0` (slot `+64h`)

No Ghidra function; read from the raw listing, `009A66C0`-`009A67BA` inclusive, `RET 4`.

| step | rule |
| --- | --- |
| 1 | `task->+470h = 0FFh` |
| 2 | `009A5F50(task+3F8h, dt)` — the approach update |
| 3 | `009BDE80(task+508h, approach->+3Ch + approach->+34h, approach->+3Ch + approach->+34h, task->+438h)` — refresh the `moveto` ranges |
| 4 | `009A5870(task, dt)` — the transition rule above |
| 5 | `task->+310h->vtable[+Ch](dt)` — the current state's tick |
| 6 | `task->+2E4h = task->+470h` |
| 7 | `if (task->+424h > 0 && task->+3FCh != 0 && (*(unit+72Ch))->vtable[+38h]())` and the state is none of `aim`, `attackrun`, `turnto`, `prepare`: `007BBBA0(unit)` and `task->+424h -= 1` |

Step 7 is the **manual-release passthrough**: when the unit's own device at `unit+72Ch` asks to
drop and the bot is not in a state that owns the release, the task drops anyway and spends one
round. The vtable `+38h` callee was not read, so that reading is provisional.

## The shared `moveto` and `follow` states

Both are built from the same class. `009C2AC0(this, approach, target, near, far, mode)` stores
`approach` at `+4h`, `target` at `+2Ch` (and registers an observer pair when non-null),
`near`/`far` at `+30h`/`+34h` and `mode` at `+38h`. `009C2980(this, approach, float)` is the
`follow` form. Both share the tick `009C18C0` (`00D20AEC+0Ch` and `00D20B24+0Ch`) and the speed
slot `009C1850`.

`009C18C0` decompiles with an `unaff_EBX` float; the real prototype is `__thiscall(this, float dt)`
like every other `+Ch` body. The three branches:

| step | rule |
| --- | --- |
| 1 | with a target (`+2Ch`), refresh both poses (`+C8h == 0` -> `BSP_EntityPose_RefreshWorld`) and take the planar separation `dx^2 + dz^2`; below `[00CE3820]` the speed is `0`, otherwise the square root |
| 2 | `this->vtable[+1Ch](speed)` |
| 3 | when `unit->+C25h != 0`: write nothing more |
| 4 | when `+2Ch == 0` (no target): `cmd->+2C4h = 0; cmd->+2CCh = 1; cmd->+2BCh = 0; cmd->+2D0h = 2` and, when `approach->+10h != 0`, `approach->+10h->+7Ch = 1` |
| 5 | otherwise: altitude `max(+34h + targetY, +30h)`, a `BSP_Math_InterpolateClamped` throttle over `([00D1F8D0] - targetY) / clamp(dt', ...)`, then `009FBA50(alt, +38h, ., throttle)`, `009F9E40`, `approach->+1Ch->+40h = tuning->+670h`, and `009FABE0(009A1A20(x), x)` with `x = 0099B630()` |

`009C1850(this, float)` is the speed slot: `cmd->+2B4h = 009BECD0(ctl->+3A0h, 007C47F0(), arg)`,
`cmd->+2B0h = 0`, `cmd->+2D8h = 1`.

`cmd` is `approach->+18h`, which `009F9980(approach, task)` sets to `task+4h`, the sub-object the
base task constructor builds with `0099BE30`. `009F9980` also sets `approach->+1Ch = task+314h`
and `approach->+20h = task+38Ch`. So the bot's per-tick output is the command block at `task+4h`,
and `unit+9D4h` (`approach->+0Ch`) only carries the altitude limits of `docs/BOT_TASKS.md`.
`0099D300 PilotBot_PlanControls` is called on `task+4h` at `00999907` and `009999AA`; it is the
peer's contract and the consumer of every `cmd->` write below.

Command-block offsets written by the states read here (all relative to `task+4h`):

| offset | written by | value |
| --- | --- | --- |
| `+278h`/`+27Ch` | `009A3770`, `009D07B0` | a float and a byte `1` |
| `+2A8h`/`+2ACh` | `009A3770`, `009D07B0` | `0` and a byte `1` |
| `+2B0h`/`+2B4h` | `009C1850`, `009A4010` | byte `0` and the desired speed |
| `+2BCh`/`+2C4h` | `009C18C0` | `0` when there is no target |
| `+2C0h` | `009A3770`, `009A4010`, `009D07B0` | the desired heading, an absolute wrapped angle |
| `+2C8h` | `009A4010` | an interpolated value |
| `+2CCh` | all | `1` with no target, `2` with a heading |
| `+2D0h` | `009C18C0` | `2` |
| `+2D8h` | `009C1850`, `009A3770`, `009A4010` | `1`, or `0` in `attackrun` |

## `unit+9D8h`

`007B8AD0` is three instructions: `return unit->+9D8h == 0`. Ghidra already carries the name
`BSP_Unit_HasFollowTarget` on it, which is **inverted**: it returns true when the field is null.
The two consumers agree on the reading that `+9D8h` is the unit's follow target:

* every derived task constructor picks `moveto` when it returns true and `follow` when it returns
  false (`009A52A7`: `TEST AL,AL` / `JNZ` over the `LEA ECX,[ESI+0x544]`);
* `009A5870` steps 4 and 5 make the same choice on every tick, so a follow target appearing or
  disappearing switches the approach state live.

Per `docs/BOT_TASKS.md` it also gates the whole altitude block of the ten slot `+54h` bodies. No
writer of `+9D8h` was read: `contract: unread`.

## The ordnance release

`007BBBA0(unit)` is `__fastcall(ECX = unit)`, no stack arguments, and is the single release
request for every drop weapon. 34 call sites image-wide; the bot-task ones are `0099B6CD`,
`009A20DB`, `009A4620`, `009A48EA`, `009A4909`, `009A4A89`, `009A5746`, `009A67AB` (depth-charge
and close-to-ship), `009AD2F0`, `009AE106`, `009AE97E` (drop-kamikaze), `009B5A1A`, `009B67E9`,
`009B81B6`, `009B8C38` (level bomb), `009C5777`, `009C60F1`, `009C8026`, `009C88C4` (dive bomb),
`009D258A`, `009D25B3`, `009D26F8`, `009D2938`, `009D29CB`, `009D3EB6`, `009D4956` (torpedo).

```
dev = unit->+DECh;
if (dev->+60h != 0 && dev->+64h != [00D7A24C] && unit->+9C3h[[00F876B8] * 8] != 0) {
    if (dev->+61h != 1 || dev->+64h != [00D7A24C]) { dev->+64h = [00D7A24C]; dev->+61h = 1; dev->+68h = 1; }
    if (dev->+68h != 0) dev->+11h = 1;
}
unit->+C20h += 1;
```

It sets a request flag on the device at `unit+DECh` and bumps a counter at `unit+C20h`; it does
**not** spawn anything itself. Which gun or bomb-platform class `unit+DECh` is, and where the
`docs/PROJECTILE_KINDS.md` spawn happens behind `dev->+11h`, was not read: `contract: unread`.
It is not one of the `docs/GUN_CLASS_FAMILY.md` `25h`/`26h` Fire slots at any site read here.

### The depth-charge run, `009A4010` (the `aim` tick)

`__thiscall(this, float dt)`, `RET 4`, body `009A4010`-`009A4669`. `coverage: partial` — the
heading and throttle arithmetic of the first two thirds is transcribed only as far as the
`BSP_Math_InterpolateClamped` chain; the release tail is `complete`.

| step | rule | address |
| --- | --- | --- |
| 1 | `state->+1Ch -= dt` | `009A4010` head |
| 2 | `p = approach->+60h`, the run-in parameter every interpolation is keyed on | |
| 3 | heading: `AddWrappedAngle(approach->+4Ch, offset)` then clamped to `+/- InterpolateClamped([00CEB4D4], [00CE398C], [00D1F4C8], [00D1F6F8] = 1.3089969f, p)` around the current heading, written to `cmd->+2C0h` with `cmd->+2CCh = 2` | |
| 4 | `cmd->+2C8h = InterpolateClamped([00CE77B0], [00CE54A0], [00CE397C], [00CE3814], p)` | |
| 5 | altitude `approach->+3Ch + approach->+34h` through `009FBA50` (the aim altitude of `docs/BOT_TASKS.md`, drawn between `Pilot/DepthCharge/AimAltRange/1` = 20 and `/2` = 60) | |
| 6 | speed `cmd->+2B4h`, `cmd->+2B0h = 0`, `cmd->+2D8h = 1` | |
| 7 | `state->+18h = [00CE380C] < approach->+50h` — the pull-out flag step 10 of the transition rule reads | `009A4588` |
| 8 | `if (altitude - (approach->+5Ch + approach->+58h) >= [00CE4D70]) { approach->+76h = 0; return; }` | `009A45A3`, `009A465D` |
| 9 | `approach->+76h = 1; approach->+78h = 3` — arm, and select weapon `3` | `009A45A9`, `009A45AF` |
| 10 | release when all of: `approach->+94h != 0`; `target->+5Dh == 0`; `approach->+74h != 0` (has ordnance, set by `009A5F50` from `BSP_WeaponController_HasDepthChargeOrdnance`); `state->+1Ch < 0.0f`; `009A3FA0(approach)`; and `BSP_EntityPose_GetWorldPositionRefreshed(unit)->y < approach->+3Ch + approach->+34h + [00CE3DD8]` | `009A45B8`-`009A4619` |
| 11 | `007BBBA0(unit)`, `approach->+2Ch -= 1`, `state->+1Ch = 00BD2F10([00CE3860], [00CE6448]) * [00CE65D0]` | `009A4620`-`009A4653` |

`009A3FA0(approach)` is two instructions plus the compare:
`return approach->+60h < approach->+5Ch + approach->+58h`. `approach->+58h` is the constant
`[00CE38C8]` the approach constructor `009A3390` writes; `+5Ch` and `+60h` are per-tick values
`009A5F50` maintains and were not read, so the release range is stated as "the run-in parameter
fell below the release threshold", not as a metre value: `contract: unread`.

The ECX at the release is `approach->+4h`, the unit: `MOV EAX,[ESI]; MOV ECX,[EAX+4]` at
`009A461B`-`009A461D`, and no `ADD ESP` follows the call.

### The depth-charge run, `009A48D0` (the `done`/`prepare` exit, slot `+8h`)

`done` (`+5DCh`) and `prepare` (`+6E4h`) are two instances of one class, both built by `009C2980`
and both stamped with the vtable `00D1F6FC` (`009A4E9C`, `009A4F4D`). Leaving either one drops:

```
if (state->+98h >= [00D7A218]) { 007BBBA0(state->owner->+4h); state->owner->+2Ch -= 1; }
009BDE40(state);     // the follow base's own exit
```

`state->+98h` is initialised to `[00D7A260]` (`-1.0f`) by the constructor, so the drop only
happens after something raises it; the writer was not read: `contract: unread`.

### The torpedo run

`009D06D0` (the `attackrun` constructor) and `009A36A0` are byte-for-byte the same shape, and the
ticks `009D07B0` and `009A3770` are the same routine with different constants. Neither releases.
`009D07B0`, `complete`:

| step | rule |
| --- | --- |
| 1 | `if (dt < state->+1Ch) { state->+1Ch -= dt; }` else re-roll: `state->+1Ch = (state->+18h - dt) + state->+1Ch` and draw a new lateral offset from `007F0280(unit, ...)` with the last argument `0` (the depth-charge passes `1`) |
| 2 | on a re-roll only, bias the offset by `clamp(InterpolateClamped([00CE3854], 0, [00CE3800], [00CE3854], approach->+90h / approach->+88h) * approach->+5Ch, [00D05EA4], [00CE3814])` and re-wrap |
| 3 | `cmd->+2C0h = AddWrappedAngle(approach->+94h, state->+20h)`, `cmd->+2CCh = 2`, and `approach->+60h = ` the same angle |
| 4 | altitude `approach->+78h + approach->+74h` through `009FBA50`, throttle from `InterpolateClamped([00D7A2F0], [00CF6560], [00CE7804], [00CE74F8], .)` over the height margin `[00D1F8D0] - unitY` clamped against `min(approach->+90h, [00CF0DD8])`; the second argument is `approach->+7Ch` when `approach->+134h >= [00CF3F20]` and `approach->+80h` otherwise |
| 5 | `cmd->+278h = 1.0f`, `cmd->+27Ch = 1`, `cmd->+2A8h = 0`, `cmd->+2ACh = 1` |

The torpedo drop is the same exit-slot rule as the depth charge, `009D2570` (vtable `00D21320`
slot `+8h`, the `done`/`prepare` class):

```
if (state->+98h >= [00D7A218]) { 007BBBA0(state->owner->+4h); state->owner->+2Ch -= 1; }
jmp 009BDE40;
```

`009D25A0` is its sibling with an explicit `bool` argument gating the same two lines
(`CMP byte ptr [ESP+1Ch],0` at `009D25A3`, `RET 4`), and `009D2938`, `009D29CB`, `009D26F8`,
`009D3EB6`, `009D4956` are the remaining torpedo release sites; none of the five was read.
The torpedo `aim` state (`00D212FC`, tick `009D15F0`) has no Ghidra function and was not read:
`contract: unread`. So the torpedo release **range** is not established here; only the mechanism
(the same `007BBBA0` request, one round per call) and the fact that it fires on leaving
`prepare`/`done` rather than from `attackrun`.

## Host table

One row per native call site the reconstruction models. `this`/args/ret from the calling
convention at the site; `gate` is the condition under which the site runs.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `009A30A8` and eight siblings | `00411E70` | `register_state_name` | `approach+FCh / name, state / void` | construction only |
| (in `00411E70`) | `00411D90` | not modelled (vector push_back) | `approach+FCh / pair / void` | always |
| `009A4E57` | `009C2AC0` | `construct_moveto_state` | `state / approach, target, near, far, mode / state` | construction only |
| `009A4E72`, `009A4E8F`, `009A4F40` | `009C2980` | `construct_follow_state` | `state / approach, float / state` | construction only |
| `009A4EBC` | `009A36A0` | `construct_attack_run_state` | `state / approach / state` | construction only |
| `009D2EB7` | `009D06D0` | `construct_attack_run_state` | `state / approach / state` | construction only |
| (in `009A36A0`, `009D06D0`, `0099C6F0`) | `00BD2F10` | `random_between` | `- / lo, hi / float` | every period phase |
| `009A52BC`, `009A52C7` | `state->vtable[+4h]` | `enter_state` | `state / - / void` | the constructor's initial state |
| (in `009A56C0`) | `state->vtable[+8h]` | `exit_state` | `state / - / void` | when a current state exists |
| `009A6741` | `state->vtable[+Ch]` | `tick_state` | `state / dt / void` | every tick |
| `009C18C0` tail | `this->vtable[+1Ch]` | `set_desired_speed` | `state / speed / void` | every `moveto`/`follow` tick |
| `009A52CC` | `009F9980` | `bind_approach_to_task` | `approach / task / void` | construction only |
| `009998FB`, `0099995A` | `task->vtable[+64h]` | not modelled (the peer's `009998A0`) | `task / dt / void` | every tick |
| `0099AE28`, `0099AEAE` | `task->vtable[+54h]` | not modelled (`docs/BOT_TASKS.md`) | `bot->+58h[0] / - / void` | `contract: unread` |
| `009A66DF` | `009A5F50` | `update_approach` | `approach / dt / void` | every tick, step 2 |
| `009A671A` | `009BDE80` | `refresh_moveto_ranges` | `moveto state / near, far, float / void` | every tick, step 3 |
| `009A6729` | `009A5870` | not modelled (the rule itself) | `task / dt / void` | every tick, step 4 |
| `009A6771` | `(*(unit+72Ch))->vtable[+38h]` | `manual_release_requested` | `unit+72Ch / - / bool` | step 7, when rounds remain |
| `009A67AB`, `009A4620`, `009A48EA`, `009D258A`, `009D25B3` | `007BBBA0` | `request_ordnance_release` | `unit / - / void` | the five release sites read |
| `009A45E5` | `009A3FA0` | `in_release_window` | `approach / - / bool` | the depth-charge release chain |
| `009A45FB` | `00427EB0 BSP_EntityPose_GetWorldPositionRefreshed` | `unit_world_position` | `unit / - / vec3` | the depth-charge release chain |
| `009A4646` | `00BD2F10` | `random_between` | `- / lo, hi / float` | after a depth-charge release |
| `009A52A7`, in `009A5870` | `007B8AD0` | `unit_has_no_follow_target` | `unit / - / bool` | the `moveto`/`follow` choice |
| in `009C18C0`, `009A4010`, `009D07B0` | `BSP_EntityPose_RefreshWorld` | `refresh_pose` | `unit / - / void` | when `unit->+C8h == 0` |
| in `009A4010`, `009D07B0`, `009C18C0` | `BSP_Math_InterpolateClamped` | `interpolate_clamped` | `- / x0, y0, x1, y1, t / float` | every geometry step |
| in `009A3770`, `009A4010`, `009D07B0` | `BSP_Math_AddWrappedAngle` | `add_wrapped_angle` | `- / a, b / float` | every heading step |
| in `009A4010` | `BSP_Math_SubtractWrappedAngle` | `subtract_wrapped_angle` | `- / a, b / float` | the heading clamp |
| in `009A3770`, `009A4010`, `009D07B0` | `007F0280` | `sample_offset_direction` | `unit / in, out, out, out, flag / void` | on a re-roll |
| in `009A3770`, `009A4010`, `009D07B0` | `009FBA50` | `command_altitude_and_throttle` | `approach / alt, a, b, throttle / void` | every tick |
| in `009C18C0`, `009A3770` | `0042E740 BSP_GameTuning_GetSingleton` | `game_tuning` | `- / - / void*` | each constant fetch |

## Coverage

`complete`: the state base layout and the seven-slot interface; the registry container and the
proof that names are never looked up; `009A56C0` the setter; `009A57D0` the entry chooser;
`009A5870` the depth-charge transition rule; `009A66C0` the depth-charge per-tick; the slot `+64h`
and slot `+54h` call sites; `007B8AD0` and the meaning of `unit+9D8h`; `007BBBA0` the release
request and its 34 call sites; `009A48D0` and `009D2570` the two exit-slot drops; `009A3FA0`;
`009D07B0` the torpedo `attackrun` tick; the `009A36A0`/`009D06D0` constructors; `009F9980`.

`partial`: `009C18C0` (the `moveto`/`follow` tick — the no-target branch and the speed call are
complete, the throttle arithmetic of `009C1900`-`009C1A4F` is transcribed but its constants are
not resolved to tuning rows); `009A4010` (the depth-charge `aim` tick — release tail complete,
heading and throttle partial); `009A5F50` (the depth-charge approach update — only the head at
`009A5F50`-`009A6043` and the ordnance flag were read, `009A6043`-`009A64F3` unread).

`contract: unread`: the torpedo `aim` tick `009D15F0` and the five other torpedo release sites;
every dive-bomb, level-bomb, strafe, rocket and kamikaze state body; the three depth-charge
geometry predicates `009A5610`, `009A5390`, `009A53B0`; `009BDE40`, `009BDE80`, `009BE590`,
`009BECD0`, `009FBA50`, `009FABE0`, `009F9E40`, `009A1A20`, `0099B630`, `007F0280`, `00BD2F10`;
the device at `unit+DECh` and the spawn behind `dev->+11h`; `ctl->+370h`'s producer; the writer of
`unit+9D8h`; the routine containing `0099AE28`; the `+10h`, `+14h` and `+18h` state slots.

## Corrections

* `docs/BOT_TASKS.md` open question "which slot the scheduler calls per tick" answered: `+64h`,
  from `009998A0` at `009998FB` and `0099995A`. Slot `+54h` is called on `bot->+58h[0]` from
  `0099AE28`/`0099AEAE` instead.
* Ghidra's `BSP_Unit_HasFollowTarget` on `007B8AD0` is inverted; the body returns
  `unit->+9D8h == 0`. Not renamed (this packet is read-only on the Ghidra project).
* `docs/BOT_TASKS.md` reads the tasks' output as the pilot control block at `unit+9D4h`. That is
  where the altitude limits go, but the per-tick heading, speed and weapon requests go to the
  command block at `task+4h` through `approach->+18h`, set by `009F9980`.

## Open questions

* What raises `state->+98h` on the `done`/`prepare` class, which is the only gate on the two drops
  that actually fire in the depth-charge and torpedo classes.
* Whether `cmd->+2A8h`/`+2ACh` (written `0` and `1` by both `attackrun` ticks) is a second weapon
  request or a control-mode pair. Its reader is in `0099D300`, a peer's contract.
* Why both release rules decrement `approach->+2Ch` while the manual passthrough at `009A67B0`
  decrements `task->+424h`; the two counters are different fields.
* What `approach->+5Ch` and `+60h` are per tick, which is what turns `009A3FA0` into a metre range.
