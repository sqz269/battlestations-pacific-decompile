# Producers of a unit's ordered command fields (packet `unit_command_producers`)

Addresses: 0080DAD0, 00816A40, 0089D9D0, 0089DB70, 0089DD10, 0089DEB0, 0099B450, 0099D300,
009998A0, 009A17D0, 009D4FB0, 009D4FE0, 009DE5B0, 009EC7C0, 009F3F80, 00813CA0, and read-only
00825F20, 00818340, 0081ED40, 00822C20, 00914EF0, 009D4E30, 00811940.

Worker `agent/cc-unit-orders`, 2026-09-10. Ghidra was read-only for this packet: no renames,
comments, prototypes, function creation or saves. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; every live query verified both. Descriptive names are hypotheses,
not recovered symbols.

## What this packet settles

`docs/UNIT_FORCE_COMMANDS.md` left four writers open and proposed that all of them address the
unit "through a shifted base register" that no byte pattern can find. That premise holds for two
of the fields and is **wrong for the other two**: the load latches at `+102Ch` and `+1034h` have
ordinary writers, and the `+9A0h`/`+9A4h` pair has four of them.

| field | producer | state |
| --- | --- | --- |
| `+980h` ordered thrust | **none exists in the image** (see the scan below) | open, negative result |
| `+984h` ordered toTurn | **none exists in the image** | open, negative result |
| `+998h`/`+99Ch` script control (aircraft) | `0089D9D0`, `0089DB70` | analysed |
| `+9A0h`/`+9A4h` script control (aircraft) | `0089DD10`, `0089DEB0` | analysed |
| `+994h`/`+998h`/`+99Ch` current order (ship) | `0080DAD0` from `00816A40` | reconstructed |
| `+102Ch` turn-assist load | `009D4FB0`, inlined in `009DE5B0` and `009F3F80` | reconstructed |
| `+1034h` second load | `009D4FE0`, inlined in `009F3F80` | reconstructed |
| `+1030h` propeller load, `+1038h` boost | only the constructor `0081ED40` | open |

## `+980h` and `+984h` are never written: the scan that proves it

The previous packet scanned `movss`/`fstp`/`mov`/`lea` against `[reg+980h]` and `[reg+984h]`.
This packet widened that to every store encoding and to every shifted base the unit actually has,
over the whole image (`tools/bsp.py scan-bytes`, `.text` and unrestricted).

Store encodings tried at displacement `980h` and `984h`, all with a wildcard ModRM and again with
a SIB byte: `F3 0F 11` (movss), `F2 0F 11` (movsd), `0F 11` (movups), `0F 13` (movlps),
`66 0F D6` (movq), `0F 17` (movhps), `66 0F 13` (movlpd), `D9 /2` and `D9 /3` (fst/fstp dword),
`DD /2` and `DD /3` (fst/fstp qword), `89 /r` (mov r32), `C7 /0` (mov imm32). The only hit in any
of them is `mov [esi+980h], ebx` / `mov [esi+984h], ebx` at `004DA32B`/`004DA331` inside
`004DA2A0`, an unrelated scene loader, which the previous packet already dismissed.

The seven secondary-base subobjects of the unit (`+010h`, `+024h`, `+170h`, `+1E4h`, `+310h`,
`+38Ch`, `+72Ch`, evidence `006FE473..006FE4A9` in `docs/UNIT_INSTANCE_UPDATE.md`) give the
displacements `970h`, `95Ch`, `810h`, `79Ch`, `670h`, `5F4h`, `254h` and the corresponding `+4`
values. Every one was scanned with the same store encodings. The only ship-side hits are
`00854230` and `00857F40` writing `[esi+810h]`/`[esi+814h]`, and both are Lua class-descriptor
readers (`AirReloadTime`, `TurboStrength`), not unit code. The controller at `+1018h` gives the
negative displacement `FFFFF968h`; nothing uses it.

An address computed into a register would have to materialise `980h` as an immediate. Neither
`lea reg,[reg+980h]` in any ship or AI function, nor `add eax,980h` (`05 80 09 00 00`), nor
`add reg,980h` (`81 /0`), nor `push 980h`, nor `mov reg,980h` occurs anywhere in `.text`.
An indexed store into a float array covering `+980h` would show as a SIB store with a `09xx`
displacement; the only ones in the image are `[esp+9xxh]` stack spills inside `0062C2C0`.

So the two fields are read by at least eleven routines (`00825F20` at `00826C61`, the engine
audio `008252C0` at `00825331`, the propellers `00834E90` at `008350CE`, `00834A70`, `00822C20`
at `00823AD1`, the `_ship` dump `00818340` at `0081857C`, the session-state message `00813CA0`
at `00813D4B`/`00813D54`, the HUD `0064AE20`, `0064B870`, `0064DA40`, `00650210`, and
`00902290`) and written by nothing. The remaining explanations are a block copy over a region
that contains them, or a member function of an object embedded in the unit at an offset below
`980h` writing at a small displacement. Neither is reachable by scanning; both are follow-up work.

The constructor does not write them individually either: `0081ED40` zeroes `+9A0h`, `+9A4h`,
`+9E8h`, `+9ECh`, `+9F0h`, `+9F4h`, `+102Ch`, `+1030h`, `+1034h` and `+1038h`, but not `+980h`
or `+984h`, so their initial value comes from a bulk clear of the object.

## The player order path: `00816A40` and `0080DAD0`

`00816A40`, `void __cdecl(a, b, c)`, RET 0, body `00816A40-00816AF1`. Callers are the HUD
screens `0064B870`, `00651760` and `0067C4F0` (segments 26 and 27), so this is the path a player
order takes. It

1. builds a 20h-byte order record on the stack through `00815440(a, b, c)` (`00816A5C`),
2. calls `0080DAD0` with the record (`00816A64`),
3. if the game object's session mode at `game+1FE4h` is 2 (`00816A69`, `DAT_00E188A8`), constructs
   session message `8Eh` (`0075B430` at `00816A76`), copies the same eight dwords into it with an
   inline `rep movsd` of count 8, and sends it through `0077C2A0(&msg, 0, 0)`.

`0080DAD0`, `void __thiscall(unit, const record*)`, RET 4, body `0080DAD0-0080DB4D`. The index at
`unit+97Ch` is reloaded from memory before each write and is **not** advanced:

| write | address | effect |
| --- | --- | --- |
| slot `+00h` | 0080DAE6 | `= record[0]` |
| slot `+04h` | 0080DAF3 | `= record[1]` |
| slot `+1Ch` | 0080DB0A | `= record byte at +1Ch` |
| slot `+08h` | 0080DB1B | `= 0` |
| `unit+998h` | 0080DB34 | `= slot[+04h]` (fstp) |
| `unit+994h` | 0080DB43 | `= slot[+00h]` (fstp) |
| `unit+99Ch` | 0080DB4A | `= slot byte at +1Ch` |

The slot address is `unit + 838h + index*20h` (`imul` by 20h at `0080DADD`). The mirror is read
back out of the slot, not out of the argument, so a bound violation on the index would corrupt the
mirror as well. There is no bound check in the native code.

The two leading words move through `fld`/`fstp`, so the listing does not settle whether they are
floats or opaque 32-bit words; they are carried as floats in the reconstruction because the
mirror at `+994h`/`+998h` is what the ship-side consumers read.

## The aircraft script control block, `+998h..+9A4h`

Four Lua bindings each store one number into the block and clamp nothing:

| address | field | store | string |
| --- | --- | --- | --- |
| `0089D9D0` | `+998h` | 0089DB05, `fstp [esi+998h]` | `luaMW_PlaneSetYawCtrl failed:` |
| `0089DB70` | `+99Ch` | 0089DCA5, `fstp [esi+99Ch]` | (pitch binding, same shape) |
| `0089DD10` | `+9A0h` | 0089DE45, `fstp [esi+9A0h]` | `luaMW_PlaneSetRollCtrl` |
| `0089DEB0` | `+9A4h` | 0089DFE5, `fstp [esi+9A4h]` | `luaMW_PlaneSetPowerCtrl` |

Each is `int __fastcall(lua_State*)`, RET 0 (the tail byte at `0089DEA1` is `C3`). The body is the
standard binding frame: `BSP_NativeString_Resize`/`Assign` for the once-only `luakod` tag guarded
by bit 0 of `00F87DEC`, `FUN_00888AA0` to fetch the unit from the first argument,
`BSP_LuaObject_GetNumber` for the value, then the single `fstp` into the unit.

`+9A0h` and `+9A4h` are `helmsmanControl.thrust` and `helmsmanControl.toTurn` in the `_ship` dump
(`00818724`, `00818757`), and they are already `kUnitOffHelmsmanThrust`/`kUnitOffHelmsmanToTurn`
in `include/bsp/unit_forces.hpp`. On a ship nothing writes them except the constructor's zero at
`0081EEA2`/`0081EEA8`, so the pair is live only for units a script drives.

## The bot's intake of the control block, `0099B450` and `0099D300`

`009998A0` is the bot tick. Its order is: `0099B450`, then `0099C270`; on a true result it calls
the state vtable slot `+64h` with the delta and `0099D300` and returns. Otherwise it counts the
retarget timer at `+C2h`/`+C1h` down, calls `0099B740` when it expires, runs the state slot, then
`009FC7C0` (conditionally), `009FD0E0`, `009A17D0` and `0099D300`. Ghidra decompiles `009998A0`
and `009A17D0` with a `this` four bytes below the one it gives `0099B450` and `0099D300`; the
disassembly of `0099B450` (`mov eax,[ecx+2F0h]` at its first instruction) makes the latter pair
the correct base, and the field names below follow it.

`0099B450`, `void __fastcall(bot)`, RET 0, body `0099B450-0099B588`, seeds five plan slots from
five floats of the unit and clears each slot's "has pending" byte:

| unit float | committed | pending | pending byte | timer |
| --- | --- | --- | --- | --- |
| `+9F0h` | `+274h` | `+278h` | `+27Ch` | `+2D8h` |
| `+9E4h` | `+280h` | `+284h` | `+288h` | `+2D4h` |
| `+9ECh` | `+28Ch` | `+290h` | `+294h` | `+2CCh` |
| `+9E8h` | `+298h` | `+29Ch` | `+2A0h` | `+2D0h` |
| `+9F4h` | `+2A4h` | `+2A8h` | `+2ACh` | `+2D8h` |

`0099D300`, `void __thiscall(bot, float dt)`, RET 4, body `0099D300-0099EBAB`, is the planner. It
reads the unit's control block and, for each of `+998h`, `+99Ch` and `+9A0h` that is not exactly
zero, clamps it and writes it into the matching plan slot with the pending byte set and the
timer cleared (`0099D5D8`, `0099D63A`, `0099D69C`). `+9A4h` takes the same clamp but feeds a
heading value rather than a slot, after a call to `007B4ED0`. The clamp is

```
if (-1.0f <= v) { if (1.0f < v) v = 1.0f; }   ; 00D7A260, 00D7A24C
else             v = -1.0f;
```

so an unordered compare yields `-1.0f`. Consumers read a slot as
`pending byte ? pending : committed` (`0099E996`, `0099EABF`).

The unit this bot drives is an aircraft, not a ship: the segment's keywords are `attackrun`,
`circle`, `goaway`, `state_moveto`, the control block is written only by the `luaMW_Plane*`
bindings, and `+9F0h`/`+9F4h` are `kUnitOffBowAnchorSink`/`kUnitOffSternAnchorSink` and `+9E4h` is
`kUnitOffSteeringJam` on the *ship* layout. The five floats above are therefore aircraft-class
fields at offsets a ship uses differently, and none of them is the ship AI's order output.

## The load latches, `+102Ch` and `+1034h`

Two routines at `009D4FB0` and `009D4FE0` are out-of-line copies of one setter, one per field.
Neither has a Ghidra function: `009D4E30`'s body ends at `009D4E95` and the enclosing-candidate
heuristic mis-attributes them to it. `009D4FB0` is, byte for byte,

```
009D4FB0: d9 81 2c 10 00 00     fld   dword ptr [ecx+102Ch]
009D4FB6: d9 44 24 04           fld   dword ptr [esp+4]
009D4FBA: df f1                 fcomip st, st(1)
009D4FBC: dd d8                 fstp  st(0)
009D4FBE: 76 0e                 jbe   009D4FCE
009D4FC0: f3 0f 10 44 24 04     movss xmm0, dword ptr [esp+4]
009D4FC6: f3 0f 11 81 2c 10 00 00  movss dword ptr [ecx+102Ch], xmm0
009D4FCE: c2 04 00              ret   4
```

`void __thiscall(unit, float request)`, RET 4: the field is raised only when the request is
strictly greater, and an unordered compare takes the `jbe` and leaves it alone. `009D4FE0` is the
same with `+1034h`. Neither has any cross-reference, so both are dead out-of-line copies of an
inline function; the live sites are inlined.

The same sequence appears inlined at `009DE853` inside `009DE5B0` (`void __thiscall(this, float)`,
RET 4, body `009DE5B0-009DF117`, caller `009ED6B0`) writing `+102Ch`, and six times inside
`009F3F80` (`void __thiscall(this, float dt)`, RET 4, body `009F3F80-009F4D06`, caller
`009F4DA0`), which holds the unit at `this+3FCh`:

| address | field | condition | stored |
| --- | --- | --- | --- |
| 009F438C | `+102Ch` | below a computed level | that level |
| 009F4606 | `+1034h` | below a computed level | `1.5f` (00CE380C) |
| 009F462A | `+102Ch` | below the same level | `1.5f` |
| 009F4912 | `+1034h` | `< 1.5` (00CE3D78, double) | `1.5f` |
| 009F4A51 | `+102Ch` | `< 0.5` (00D7A280, double) | `0.5f` (00CE3800) |
| 009F4AB4 | `+1034h` | below a computed level | that level |

The exact placement of the six is from the decompilation; two were confirmed against the listing
(`009F45F8..009F4606`). The pattern is a per-frame maximum latch: several AI stages each request a
load level and the highest request for the frame survives. `00825DE0` and `00825EC0` consume
`+102Ch` and `+1038h` on the motion side, which is why `docs/UNIT_FORCE_COMMANDS.md` saw these
three routines as readers only.

`009EC7C0`, `float10 __thiscall(this, float, float, float)`, is the throttle ceiling the same AI
class uses: it folds five `BSP_Math_InterpolateClamped` stages over the tuning block `00424C40`
(fields `+6CCh`, `+6D0h`, `+6D4h`, `+6E0h`, `+6E4h`, `+6E8h`, `+6ECh`), then clamps the result
into the band around the unit's own `+980h`, then against `[unit+73Ch]+24h / this+3C4h`, and
finally negates it unless `this+364h` is set, returning `-0.625f` (00D21A78) below the floor
`-0.625` (00D21A80). It only reads `+980h`.

`+1030h` (`kUnitOffPropellerLoad`) and `+1038h` (`kUnitOffAccelerationBoost`) have no writer
besides `0081ED40`'s zero; the two hits at those displacements elsewhere (`004BC890`, `004ECA30`)
belong to other classes.

## The session state message, `00813CA0`

`00813CA0`, `msg* __thiscall(msg, unit*)`, builds session message `8Ch` and carries the ordered
pair out of the unit: `msg+40h = unit+980h`, `msg+44h = unit+984h` (`00813D4B`, `00813D54`). It
also carries `unit+6A4h..6ACh`, the heading from unit vtable `+50h`, a speed from `0092D700`, a
position from `0092D6A0`, a flooded flag from `unit+110Ch`, and two bytes. Its caller is
`00816B00`. The apply side of message `8Ch` was not found and is the most likely home of the
missing `+980h`/`+984h` write.

## Calling conventions and RET sizes established

| address | signature | RET | body |
| --- | --- | --- | --- |
| 0080DAD0 | `void __thiscall(unit, const record*)` | 4 | 0080DAD0-0080DB4D |
| 00816A40 | `void __cdecl(a, b, c)` | 0 | 00816A40-00816AF1 |
| 0089D9D0 | `int __fastcall(lua_State*)` | 0 | 0089D9D0-0089DB61 |
| 0089DB70 | `int __fastcall(lua_State*)` | 0 | 0089DB70-0089DD01 |
| 0089DD10 | `int __fastcall(lua_State*)` | 0 | 0089DD10-0089DEA1 |
| 0089DEB0 | `int __fastcall(lua_State*)` | 0 | 0089DEB0-0089E041 |
| 0099B450 | `void __fastcall(bot)` | 0 | 0099B450-0099B588 |
| 0099D300 | `void __thiscall(bot, float dt)` | 4 | 0099D300-0099EBAB |
| 009D4FB0 | `void __thiscall(unit, float)` | 4 | 009D4FB0-009D4FD0, no Ghidra function |
| 009D4FE0 | `void __thiscall(unit, float)` | 4 | 009D4FE0-009D5000, no Ghidra function |
| 009DE5B0 | `void __thiscall(this, float)` | 4 | 009DE5B0-009DF117 |
| 009F3F80 | `void __thiscall(this, float dt)` | 4 | 009F3F80-009F4D06 |

Three further listing-only accessors sit in the same gap and were read but not named:
`009D4EA0` (`movss [ecx+40h]`, flag byte `+4Ch = 1`, RET 4), `009D4EC0` (same for `+48h`),
`009D4EE0`/`009D4EF0`/`009D4F00` (`fld [ecx+44h/40h/48h]`, RET 0).

## Reconstruction

`include/bsp/unit_orders.hpp` and `src/unit_orders.cpp` carry the order queue projection and
`publish_unit_order_0080dad0`, the issue path `issue_unit_order_00816a40` over a three-method
host, the latch `raise_unit_load_latch_009d4fb0` and `latch_unit_load_to_level`, the control
clamp `clamp_unit_control_override_0099d300`, and the plan slot with
`seed_unit_plan_slot_0099b450`, `unit_plan_slot_value` and
`apply_unit_control_override_0099d300`. Offsets already present in
`include/bsp/unit_forces.hpp` and `include/bsp/unit_motion.hpp`
(`kUnitOffHelmsmanThrust`, `kUnitOffHelmsmanToTurn`, `kUnitOffTurnAssistLoad`,
`kUnitOffTurnAssistLoad2`, `kUnitOffThrottle`, `kUnitOffSteering`) are reused, not redefined.
The bound check in `publish_unit_order_0080dad0` has no native counterpart and is marked as such.

## Uncertainties

1. The order record's two leading words are copied through x87 and their type is not settled.
   The number of slots in the array at `+838h` was not recovered; the projection's eight is an
   arbitrary capacity, not a native bound.
2. `00815440`, which fills the record from `00816A40`'s three arguments, was not read, so the
   meaning of the record fields, including whether one of them is an ordered speed, is unknown.
3. The bot class at `0099B450`/`0099D300` is taken to be the aircraft pilot bot from its segment
   keywords and from the `luaMW_Plane*` writers of the block it reads. That identification is
   provisional; if it is wrong, the five unit floats it seeds from are ship fields and collide
   with `kUnitOffSteeringJam`, `kUnitOffBowAnchorSink` and `kUnitOffSternAnchorSink`.
4. Ghidra decompiles `009998A0` and `009A17D0` with a `this` four bytes below the one it gives
   `0099B450` and `0099D300`. The disassembly settles which is right for `0099B450` only; the
   other three were read through the decompiler.
5. Four of the six inlined latch sites in `009F3F80` are from the decompilation alone; the levels
   they request were not traced back to their sources.
6. `00914EF0` `BSP_BotScheduler_Update` was read only through its existing ledger record and
   callee list; it reaches `00911E80`, `00912A60` and `00914390`, none of which was opened. The
   ship AI's order output stage was not located through it.
7. Whether the ship AI writes any command field at all is open. Everything found here is either
   a player order (`00816A40`), a script control (`luaMW_Plane*`) or a load request (`009F3F80`).

## What remains

* The writer of `unit+980h`/`+984h`. The two live candidates are the apply side of session
  message `8Ch` (built by `00813CA0`) and a block copy over the `+978h..+998h` region.
* `00815440`, the order record builder, and the HUD callers `0064B870`, `00651760`, `0067C4F0`.
* The writer of `+1030h` and `+1038h`.
* The ship AI's order output stage, starting from `00914EF0`'s three unopened callees.

## Follow-up packets

* `unit_state_message_apply` — addresses `00813CA0` (read), `00816B00`, the handler of session
  message `8Ch`, `0077C2A0`; files `docs/UNIT_STATE_MESSAGE.md`. Find the receive side of the
  unit state message and settle whether it is what writes `unit+980h`/`+984h`.
* `unit_order_record` — addresses `00815440`, `00816A40` (read), `0064B870`, `00651760`,
  `0067C4F0`; files `docs/UNIT_ORDER_RECORD.md`. Recover the 20h-byte order record's fields and
  the HUD commands that build it.
* `bot_scheduler_output` — addresses `00914EF0` (read), `00911E80`, `00912A60`, `00914390`;
  files `docs/BOT_SCHEDULER_OUTPUT.md`. Follow the bot scheduler to the per-unit AI that issues
  ship orders and find where its plan leaves the AI object.
* `ai_load_requesters` — addresses `009F3F80`, `009F4DA0`, `009DE5B0`, `009ED6B0`, `009EC7C0`;
  files `docs/AI_LOAD_REQUESTS.md`. Trace the six latch levels inside `009F3F80` back to their
  sources and settle what `+102Ch` and `+1034h` mean to the motion side.

## State reached

| address | state |
| --- | --- |
| `0080DAD0` | reconstructed, build-tested |
| `00816A40` | reconstructed, build-tested (host boundary for `00815440` and the message send) |
| `009D4FB0`, `009D4FE0` | reconstructed, build-tested; no Ghidra function |
| `0099B450` | reconstructed, build-tested |
| `0099D300` | analysed; only the control-override clamp and the slot protocol reconstructed |
| `0089D9D0`, `0089DB70`, `0089DD10`, `0089DEB0` | analysed |
| `009F3F80`, `009DE5B0` | analysed; the latch step reconstructed |
| `009EC7C0`, `00813CA0`, `009998A0`, `009A17D0` | exported and analysed |
| `00825F20`, `00818340`, `0081ED40`, `00822C20`, `00914EF0`, `009D4E30` | read only |
