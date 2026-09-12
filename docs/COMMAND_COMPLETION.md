# What happens the moment a unit's command finishes

Addresses: `0071E430` `0071D810` `0071D9E0` `0071C730` `0071C130` `0071D900` `0071E550`
`00721A40` `00720850` `0071E610` `00836920` `0071F600` `0071ECF0` `007788D0` `007788B0`
`0071F3B0` `00984300` `00984800` `0097B8C0` `0097C8A0` `00968AF0` `0097E360` `00887E50`
`005457C0` `0084E010` `00E08F58`..`00E08F90` `00D09FD8` `00CFDB1C` `00D186C4` `00F8A0C4`
`00D1B770` `00D1AF24`

Packet `cc2_command_completion`, read-only analysis. Every descriptive name here is a hypothesis,
not a recovered symbol, except `InternalClearPrimaryCommand`, which `docs/COMMAND_EXECUTION.md`
recovered from a trace literal. That document owns the ten-slot queue layout, `00720850` and the
per-class arms of `00836920`; this one owns the completion path that reaches them and is written
not to repeat them. `docs/SHIP_AI_STATE_STEPS.md` owns the state steps that start the path and
`docs/MISSION_EVENTS_UPDATE.md` owns the event-channel machinery; both are cited, not restated.

## Headline

There is no "command finished" routine. Finishing is a **one-way stage counter** per command, and
everything else is a consequence of the counter reaching 2.

* An AI state step that decides its command is done calls `0071E430`, which raises the stage of
  whichever command the controller's mode says is current.
* The stage only ever rises (`0071D810`/`0071D9E0` return early when the stored stage is already
  at least the requested one). Only a clear resets it, through `0071C130`.
* Reaching stage **2** *sends a network message*, `MT_GAMEUNIT_CLEARCMD` (`5Dh`). It does not touch
  the queue. The queue advance happens when that message is **received**, in `00721A40`'s `5Dh` arm.
* So a command finishing is a round trip through the session, identical on the unit's own machine
  and on every other one, and the local controller keeps executing the command until the message
  comes back.
* The controller is never left idle with nothing to do: `00836920`'s tail re-issues a standing
  `cruise`, `stop` or `follow` the moment the queue empties.
* A mission Lua handler does run. `00984300` fires the event channel named `command` with four
  parameters, and the status parameter has exactly two values in the whole image: `start` and
  `finished`.

## 1. The end-command sequence

### `0071E430 BSP_WeaponDirector_EndCommand`

`__thiscall(controller)(void* command, char terminal)`, `RET 8` at `0071E469`, `0071E486` and
`0071E4B7`, body `0071E430-0071E4B9`, complete. The `this` pointer is `ECX` (`MOV ESI,ECX` at
`0071E431`); the two stack arguments are `[ESP+8]` and `[ESP+0Ch]` after that push.

| # | Test | Site | Effect |
| --- | --- | --- | --- |
| A | `command != 0` and `command->vtable[0Ch]()` is neither 1 nor 2 | `0071E437`-`0071E44A` | jump straight to the queue raise, ignoring the mode |
| B | `mode` (`+30h`) `== 1` | `0071E44C` | `0071D810 RaiseCommandStage(1 + (terminal != 0))` |
| C | `mode == 2` | `0071E46C` | `0071D9E0 RaiseOverrideStage(1 + (terminal != 0))` |
| D | any other mode | `0071E489` | count occupied slots from `+54h`, stride `1Ch`, up to 10; if any, `mode = 1`, `+44h = 0`, `+48h = 0` |

`vtable[0Ch]` is `int category()` (`docs/COMMAND_CLASSES.md`). A null command and categories 1 and 2
fall into the mode test; every other category skips it. Categories 1 and 2 are the target-owning
attack classes, so the rule reads: *an attack command finishing is routed by the mode, any other
command finishing always ends the queue head.*

Arm D is not an ending at all. It is the restart the controller uses when a step reports completion
while the mode is 0: it promotes the queue head and clears both the acceptance byte and the stage,
so the next frame begins the command from scratch. Nothing else in the routine writes `+44h`.

**The third argument is "terminal now", and both values occur.** The stage value is literally
`1 + (terminal != 0)` (`XOR EAX,EAX` / `CMP byte [ESP+0Ch],AL` / `SETNZ AL` / `ADD EAX,1` at
`0071E454`-`0071E45F`, and the `ECX` twin at `0071E471`).

| Call site | Caller | Command | `terminal` |
| --- | --- | --- | --- |
| `009BCB0E` | `009BCA50` | `moveonpath` `00E08F80` | 1 |
| `009C312E` | `009C3100` | `moveto` `00E08F68` | **0** |
| `009CFB47` | `009CFA80` | `moveto` `00E08F68` | **0** |
| `009E5997` | `BSP_ShipAi_MoveToPosStateStep` | `moveto` | 1 |
| `009E5C70` | `BSP_ShipAi_MoveOnPathStateStep` | `moveonpath` | 1 |
| `009E88C1` | `BSP_ShipAi_AttackMoveStateStep` | `attackmove` `00E08F78` | 1 |
| `009F3718` | `BSP_ShipAi_AttackMoveTangentSubStateStep` | `attackmove` | 1 |
| `009F7CFC`, `009F7D8A` | `009F7C90` | `0071BE40`'s current command | 1 |
| `009F83A8` | `BSP_Bot_RevalidateCurrentCommand` | `dogfight` `00E08F58` | 1 |

At `009E88C1` and `009F3718` the two argument pushes precede the virtual call that computes `ECX`
(`PUSH 1` / `PUSH 0E08F78h` / `CALL EAX` / `MOV ECX,EAX` / `CALL 0071E430`); the accessor at entity
`vtable[114h]` takes no arguments and leaves them in place, which is what makes `RET 8` balance.

### Stage 1 versus stage 2

`0071D810` and `0071D9E0` are the same body against different fields (`+48h`/`+44h` for the queue
head, `+50h`/`+4Ch` for the override):

```
if (stored >= stage) return;      // monotonic
stored = stage;
if (stage != 2) return;
if (*(00E188A8)+1FE4h == 2) return;   // this machine does not originate the message
route(0071C730(isQueue, 0), 7, 0);    // MT_GAMEUNIT_CLEARCMD
```

Stage 1 therefore only marks the command. `00836920 BSP_WeaponDirector_Step` decides when a mark
becomes an ending, in its first three arms:

| Arm | Site | Rule |
| --- | --- | --- |
| queue head | `00836985` | `queueStage == 1` **and** (`0071BE60` count > 1 **or** `unit+184h` set) -> `RaiseCommandStage(2)` |
| override | `00836993` | `overrideStage == 1` -> `RaiseOverrideStage(2)` unconditionally |
| crowding | `00836A7C` | a queued attack command whose target is nearer than `00D09FE8` ends a running movement command at stage 2 (`docs/COMMAND_EXECUTION.md`) |

So a lone `moveto` finished with `terminal = 0` **keeps running**: the ship holds position at its
destination, still owning the command, until either another command is queued behind it or the unit
is player-driven. An override command gets no such grace. This is the whole difference between the
two `0071E430` argument values, and it is why the two `009C312E`/`009CFB47` sites behave differently
from the four state-step sites.

### From stage 2 to the queue

`0071C730(u8 isQueue, u32 index)` builds the message: `BSP_SessionMessage_ConstructBase(5Dh)`,
vtable `00CFD9D8`, `+4h = 1`, `+18h`/`+1Ah`/`+1Ch` zero, `+20h = isQueue`, `+24h = index`.
`0071D810` calls it as `(1, 0)` at `0071D852` and routes at `0071D867`; `0071D9E0` calls it as
`(0, 0)`. `0071D900 BSP_WeaponDirector_SendClearCommandSlot(index)` builds the byte-identical
message with `+20h = 1` and `+24h = index` (`0071D921`, routed at `0071D961`) and is called only
from `0071E550 BSP_WeaponDirector_MakeRoomForCommand` at `0071E5AA` with `count - 1`. The stage-2
send and the make-room send are one mechanism with a different slot index.

`00721A40`'s `5Dh` arm is the receiver, and it is three lines:

| `msg+20h` | `msg+24h` | Site | Action |
| --- | --- | --- | --- |
| 0 | ignored | `00721BC5` | `0071E610 ClearOverrideCommand` |
| non-zero | negative | `00721BA8` | `00720CA0 ClearAllCommandSlots` |
| non-zero | `>= 0` | `00721BB7` | `00720850 InternalClearPrimaryCommand(index)` |

`-1` as "every slot" is `0071D880 BSP_WeaponDirector_SendClearCommands`, which is the same builder
with `+24h = 0FFFFFFFFh`.

## 2. The queue advance

`00720850` with index 0 is the advance, and `docs/COMMAND_EXECUTION.md` (its "index == 0" list)
owns the field-by-field account. What matters here is the **state the controller is left in**, and
one field that document's list does not connect to the restart:

| After `00720850(0)` | Value | Why it matters |
| --- | --- | --- |
| `+16Ch`..`+187h` | a copy of the slot-0 record made *before* the removal | the previous-command record. No reader was found in this packet; see the open questions |
| queue with 2+ commands | slots 1..n-1 shifted down, last slot cleared, `mode = 1` | the next command is the head and starts at stage 0 |
| queue with 1 command | slot 0's command zeroed, its parameters **snapped to the target's world position**, `mode = 0` | the controller is idle but the slot still records where the unit was told to be |
| `+44h`, `+48h` | both 0 | via `vtable[6Ch](1)` at `00720B56` |

That last row is the link. `vtable[6Ch]` is `00835BF0` on the derived table; it calls `0071C130`
first, and `0071C130(primary)` is nothing but `+44h = 0; +48h = 0` for a non-zero argument and
`+4Ch = 0; +50h = 0` for zero. So the promoted command starts with a clear acceptance byte, which
is exactly the condition arm 6 of `0071F290` tests before calling `vtable[78h](1)`
(`docs/DIRECTOR_UPDATE_ARMS.md`). The next frame therefore begins the new head, and
`0071F600 BSP_WeaponDirector_BeginCommandBase` sends the `start` event at `0071F79B`.

### The controller is never idle for long

`00836920`'s tail `LAB_00836DC9` runs when the queue-head stage is 2 with at most one command left,
or when the "no command" flag set at `00836941` holds:

```
if ((queueStage != 2 || 0071BE60() > 1) && !noCommandFlag) return;
if (0071BE60() <= (queueStage == 2)) {
    vtable[6Ch](1);                               // 00836E0B, reset stage + expire commanded speed
    if (007788B0(unit)) {                         // 00836E13, the controller belongs to another
        cls = follow  00E08F60;  tgt = 007788D0(unit);   // 00836E28, the controlling entity
    } else if (unit+184h || *(unit+73Ch)+28h >= 0.0f) {
        cls = cruise  00E08F70;  tgt = unit;
    } else {
        cls = stop    00E08F88;  tgt = unit;
    }
    0071ECF0 IssueCommand(cls, tgt);              // 00836E92
}
```

`*(unit+73Ch)+28h` is the commanded speed (`docs/UNIT_COMMANDED_SPEED.md`), and `00835BF0` has just
expired it, so the branch reads the value the reset left. A unit with a standing throttle keeps
cruising; one without comes to a stop; one under another entity's controller follows that entity.
`007788D0 BSP_Entity_ControllingEntity` is `[entity+284h] ? [[entity+284h]+14h] : 0`, the twin of
`007788B0`, which is `ctrl != 0 && [ctrl+14h] != entity`.

`0071ECF0` both applies the command locally through `vtable[30h]` and routes `MT_GAMEUNIT_SETCMD`
with flag 1, so the standing command is a real queued command, not a local fallback.

## 3. The Lua side of `finished`

### `00984300 BSP_MissionEvents_ReportCommand`

`__thiscall(reporter)(void* entity, CommandTarget* target, CommandClass* cls, NativeString* status)`,
`RET 10h` at `009847FB`, body `00984300-009847FD`. `this` is **not** a director: every call site does
`MOV ECX,[00F8A0C4]`, the mission event director of `docs/MISSION_EVENTS_UPDATE.md`.

Sequence:

1. `00984323`-`00984346`: enter the critical section at `reporter+24h` and bump its recursion count.
2. `00984352`: `00521EA0` resolve the target descriptor to an object.
3. `00984362`-`0098436E`: `cls->vtable[4]()` is `const char* name()`; assign it into a stack
   `NativeString` at `[ESP+20h]`.
4. `00984380`/`00984386`: clear `reporter+104h` and `+105h`.
5. `00984394`-`009843C7`: build the channel name `command` (`00D186C4`) and look it up with
   `00980150` on the map at `reporter+F8h`. **`EBP` is reassigned to the returned channel at
   `009843CE` and is not restored until `009847AF`.** Return immediately when `channel+8h`
   (`_Mysize`) is zero — a mission with no `command` event block costs one map lookup per event.
6. `00984401`-`009844AA`: box the two entity ids as `operator_new(8)` objects with vtable
   `00D1AF24` and the `u16` at `entity+174h`, using 0 for a null entity.
7. `009845B1`-`009845B6`: box the two strings.
8. `009845BD`: `0097B8C0 BSP_WarningChannel_Evaluate(channel, params)` collects the callback names
   of every subscription whose `vtable[0Ch]` accepts the four parameters.
9. `009845F0`-`009846DF`: release the boxes, then rebuild the same four values as Lua variants
   (`00884A90` for the two entities at `00984636` and `00984663`, `00884FC0` for the two strings at
   `00984690` and `009846C0`, each pushed with `006EDF00`).
10. `00984710`: for each collected name, `00887E50 BSP_MissionLuaHost_CallNamedThreadSafe` on the
    host at `*(00E188A8)+1A08h`, `__thiscall(host)(0, &nameRecord, variants, 0, -1)`.
11. `0098473F`: `MOV byte ptr [EBP+105h],1` — written to the **channel**, not the reporter.

### The parameters, in order

| # | Value | Source |
| --- | --- | --- |
| 0 | the acting unit's entity id | argument 1's `+174h` |
| 1 | the command target's entity id | `00521EA0(argument 2)`, then `+174h` |
| 2 | the command class name | `argument 3->vtable[4]()`, e.g. `moveonpath` |
| 3 | the status verb | argument 4 |

### The status vocabulary is closed

Two literals reach argument 4 in the whole image:

| Literal | Address | Sites |
| --- | --- | --- |
| `start` | `00CFDB1C` | `0071F768`, `0071F892` (both in `0071F600`, the override and queue-head begin arms) |
| `finished` | `00D09FD8` | `00836D17` (the `moveonpath` arm of the step), `0084E3BE`, `009E58E6`, `009E5BBC` |

Argument 1 is not uniform: `0071F600` passes `director+34h` (`0071F77C`, `0071F8A4`) and `00836920`
passes `director+24Ch`, the owning unit (`00836D43`). `docs/WEAPON_DIRECTOR.md` calls `+34h` the
session endpoint, but `00984300` only reads `+174h` from it and `0071F290`'s session gate reads the
same `+5Ch`/`+5Dh`/`+5Eh`/`+60h` bytes an entity carries, so both are entity-shaped. Which entity
each field is remains an open question.

### A script handler does run

`0097E360 BSP_WarningManager_ParseEventBlock` accepts the Lua event kind `command` at `0097E611`
and, at `0097E61A`-`0097E63A`, allocates `4Ch` bytes and hands them to
`0097C8A0 BSP_CommandEventSubscription_Construct`. That constructor installs vtable `00D1B770` and
builds four empty ordered containers: two `0096BA00` trees at `+0Ch` and `+1Ch` and two `004C26B0`
trees (`18h`-byte nodes, string keys) at `+2Ch` and `+3Ch`.

Slot `+0Ch` of `00D1B770` is `00968AF0 BSP_EventSubscription_MatchFourParameters`, which is what
`0097B8C0` calls. It requires the parameter vector to hold at least 1, 2, 3 and 4 entries in turn and
calls `filter->vtable[0]` on the four containers with parameters 0, 1, 2 and 3, returning 1 only when
all four answer true. The four containers line up one-for-one with the four boxed parameters, so a
mission `command` event block filters on **unit, target, command name and status**, and an empty
container must be the accept-all case.

There is therefore **no fixed `OnCommandFinished` entry point**. A mission declares a `command` event
whose filters name (for example) the ship and the status `finished`, and the callback name stored at
`subscription+4h` is the Lua function `0097B8C0` queues and `00887E50` calls. For Kortenaer arriving
at Java: `BSP_ShipAi_MoveToPosStateStep` assigns `finished` at `009E58E6`, calls `00984300` at
`009E595C`, and any `command` event block matching that unit and that status runs its Lua function
before `009E5997` ends the command.

### `00984800 BSP_MissionEvents_ReportTarget`

`__thiscall(reporter)(void* entity, void* target)`, body `00984800-00984B93`. The same shape for the
channel named `target`: lock `+24h`, clear `+104h`/`+105h`, look the channel up, return when
`channel+8h` is zero, box two entity ids, evaluate at `009849D2`, call the host at `00984ACA`. One
native producer, `0071F600` at `0071F7E8`, fired only when a command that has just started has a
target that resolves. So `target` is a *start* event, never a completion one.

## 4. Attackmove when the target dies

Three independent paths watch an attackmove, and the completion side is the same for all three.

1. **The director step.** `00836B45`'s arm is documented in `docs/COMMAND_EXECUTION.md` line 176 and
   this packet's read agrees with it. A target that no longer resolves goes to `LAB_00836BB2` and
   stage 2. A target that passes `vtable[5Ch](1Ch)` and is flagged at `+5Eh` or sits on the session's
   own side is converted: `00465080(target, 0)` becomes a `moveto` record and `0071ECF0` issues it at
   `00836BAD`, then stage 2. A live visible unit failing `005457C0` also goes to stage 2.
   `005457C0` is `__thiscall(unit)(int side)` returning `unit+54h != side && side != 2`, so it means
   "still hostile": the attackmove ends the moment its target stops being an enemy.
2. **The ship AI state step.** `BSP_ShipAi_AttackMoveStateStep` ends with `0071E430(attackmove, 1)`
   at `009E88C1`, and the tangent sub-state does the same at `009F3718`. Both are terminal, so no
   grace period applies to an attackmove.
3. **The bot.** `BSP_Bot_RevalidateCurrentCommand` re-tests the running command each tick. A category
   1 or 2 command whose target is gone or fails `BSP_Unit_AttackCommandApplies` falls to
   `LAB_009F8412`, which sets `bot+3Dh` and **does not end the command** — it is a request the bot's
   own task layer reads. `0072BCE0 BSP_GunBot_OnTargetDestroyed` is narrower still: it only fires the
   bot's `vtable[30h]` when the destroyed entity is the bot's own fire target
   (`docs/GUN_BOT_TICKS.md`), which is the weapon's target, not the command's.

**What the ship does next** is then the ordinary path: stage 2 sends `MT_GAMEUNIT_CLEARCMD(1, 0)`,
`00721A40` runs `00720850(0)`, the slot-0 record is copied to `+16Ch`, and if the attackmove was the
last queued command the controller drops to mode 0 with slot 0 snapped to the dead target's last
world position. The very next `00836920` reaches `LAB_00836DC9` and issues `cruise` or `stop`. Six
ships on one attackmove whose target dies therefore each end up holding station or cruising at their
own last commanded throttle, not reverting to anything.

**The previous-command record is not a revert source.** `+16Ch` is written by `00720850` and zeroed
by the constructor at `0072023B` (`docs/DIRECTOR_UPDATE_ARMS.md`). No reader was found in this
packet; see the open questions.

## 5. The remaining unread callees

| Address | Name | What it is |
| --- | --- | --- |
| `0071F3B0` | `BSP_RefCountedPtr_Release` | `__fastcall(void** slot)`, body `0071F3B0-0071F3D8`. `InterlockedDecrement(obj+4)`, `obj->vtable[0]()` at zero, then `*slot = 0`. Not a command routine; `0071F600` uses it at `0071F880` to drop the old slot-0 path object before committing a new `moveonpath` route |
| `007788D0` | `BSP_Entity_ControllingEntity` | `__fastcall(entity)`, body `007788D0-007788DE`. `[entity+284h] ? [[entity+284h]+14h] : 0`. Read at `00836AF2`, `00836B03`, `00836B1A` (the follow arm) and `00836E28` (the idle tail) |
| `00984800` | `BSP_MissionEvents_ReportTarget` | section 3 |
| `0084E010` | unnamed | the fourth `finished` producer, at `0084E3BE`/`0084E431`, reporting `moveonpath`. It has no Ghidra caller and reaches `0071ECF0` twice (`0084E570`, `0084E5C6`). Not read further; **coverage: none** |

## No Ghidra function

None. Every routine this packet names has a Ghidra function. `0071F290`, `00835BF0` and `0072BCE0`
are decoded from raw bytes but are already listed in `docs/DIRECTOR_TARGET_GATE.md`,
`docs/UNIT_COMMANDED_SPEED.md` and `docs/GUN_BOT_TICKS.md` and are not re-claimed here.

## Coverage

| Routine | Coverage |
| --- | --- |
| `0071E430` | complete, listing and pseudocode, all ten call sites read |
| `00984300` | complete for the control flow and the four parameters; the box and variant helpers `00884A90`, `00884FC0`, `006E0E00`, `0096DC00`, `00975EA0` are read only as far as their role in the sequence |
| `00984800` | structural only: the same shape as `00984300`, two parameters, one producer |
| `0071F3B0`, `007788D0`, `005457C0`, `0071C130`, `0071C730`, `0071D900` | complete |
| `00721A40`'s `5Dh` arm | complete |
| `00720850` | read, not re-projected; `docs/COMMAND_EXECUTION.md` owns it |
| `0097C8A0`, `00968AF0` | complete; the container element types and `vtable[0]` of the filters are not read |
| `0084E010` | none |

## Open questions

* Which entity `director+34h` is, and why the two `finished` producers disagree with `0071F600`
  about whether to report `+34h` or `+24Ch`.
* Whether anything reads the previous-command record at `+16Ch`. A targeted scan was not run; the
  claim here is only that this packet found no reader.
* `00984300` writes `channel+105h = 1` while clearing `reporter+104h`/`+105h`. The channel value is
  a `std::list` base by `docs/MISSION_EVENTS_UPDATE.md`'s reading, which makes `+105h` far outside
  it. Either the value object is larger than that document assumes or one of the two reads is wrong.
* What `bot+3Dh` and `bot+3Ch` do once `BSP_Bot_RevalidateCurrentCommand` sets them.
* `0084E010`'s owner and its vtable slot.

## Corrections to earlier documents

| Document | Was | Is |
| --- | --- | --- |
| `docs/MISSION_EVENTS_UPDATE.md` | `+104h`, `+105h`: "two bytes cleared by each channel dispatch" on the reporter | in `00984300` the clears at `00984380`/`00984386` are on the reporter (`EBP = ECX`), but the set at `0098473F` is on the channel: `EBP` is overwritten with `00980150`'s return at `009843CE` and restored only at `009847AF`, and the two undisassembled gaps at `0098472E` and `0098478D` contain no restore |
| `docs/COMMAND_EXECUTION.md` | `00984300`, `00984800`, `0071F3B0`, `007788D0`, `005457C0` listed as unread | read; sections 3 and 5 |
| `docs/CRUISE_COMMAND.md` | the `5Dh` senders are `0071E390`, `0071C7A0`, `0071C7E0`, `0071C770` | those may be further builders, but the three senders on the completion path are `0071C730` (from `0071D810` at `0071D852` and `0071D9E0`), `0071D880` and `0071D900`, and `0071E550` reaches `0071D900` at `0071E5AA`. Not a contradiction, an addition: the list was not exhaustive |
