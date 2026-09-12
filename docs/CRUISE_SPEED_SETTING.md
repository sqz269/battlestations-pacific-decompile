# The scene-authored `StartSpeed`, and why a `Cruise` ship makes way

Addresses: 00822C20 00926110 00925F20 00823576 00823590 008235BA 008235D5 008235DC 008235F7
008F2260 0080FC30 0080D9B0 0092D770 00922E20 00774DC0 00890D30 008A3600 00835AC0 00835E17
009E1170 00CFCCFC (string) 00D0659C (string) 00D03D94 (vtable) 00D03754 (vtable)

Packet `cc_cruise_speed_setting`, worker `agent/cc-scene-spawn-speed`, 2026-09-12 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query verified both.
Descriptive names are hypotheses, not recovered symbols. The literals in the image are
`StartSpeed` and `ShipYardLaunch`; the literals in the shipped data are `StartSpeed`,
`Command`, `CommandTarget` and `Cruise`.

## Answer to the packet question

**The scene record carries the speed, on the unit entity rather than in the `Command` block, and
it reaches the order ring before the command is ever latched.**

`usn_2_java.scn` gives all fourteen of its `Cruise` units `StartSpeed = F 12.0000 ;`. The
game-unit init slot `0A0h` (`00822C20`) reads that key out of the entity's scene property bag and
seeds two things:

```
reference = 0080FC30(unit)                      ; unit+9C0h (= class[500h] MaxSpeed) * scale
ratio     = float32(StartSpeed / reference)     ; 008235BF FDIVR in double, 008235CA FSTP float
0080D9B0(unit+838h, ratio)                      ; 008235D5: ring +148h = unit+980h = ratio
axial     = float32(0080FC30(unit) * ratio)     ; 008235E1 FMUL, 008235EC FSTP float
0092D770([unit+1018h], axial)                   ; 008235F7: the hull body's axial velocity
```

The `Cruise` command from the same `.scn` is only *queued* at that point (`004E6B30` ->
`00469610`); it is resolved by `0046AAB0`, routed as a message, and latched by the director when
it becomes the current command. By then the ring already holds `ratio`, so `00835E17` latches

```
cruiseThrust         = ring +148h = ratio       ; 0.72894263 for a 12 m/s DeRuyter
cruiseIsHeading      = |ring +14Ch| < 0.01f     ; true: the seed never touches the rudder
cruiseSteerOrHeading = the unit's spawn heading ; unit->vtable[50h]()
```

and `009E1170` orders that throttle on that heading every AI step. **A `Cruise` ship that stands
still is a ship whose init never ran the `StartSpeed` arm.** `docs/CRUISE_COMMAND.md`'s reading of
the latch is right; its conclusion that "nothing in the command, the message or the scene record
carries a speed or a heading" is right about the first two and wrong about the third.

The commanded-speed pair at `*(unit+73Ch)` `+24h`/`+28h` is **not** on this path. No scene
property feeds it; its two producers are the Lua bindings `luaMW_SetShipSpeed` (`00890D30`) and
`luaMW_NavigatorMoveOnPath` (`008A3600`), already read by packet `cc_commanded_speed`
(`docs/UNIT_COMMANDED_SPEED.md`), and `usn_2_java.lua` calls neither.

## What the shipped mission authors

`I:/SteamLibrary/steamapps/common/Battlestations Pacific/universe/scenes/missions/USN/usn_2_java.scn`
is plain text. Direct counts over the file:

| Count | What |
| --- | --- |
| 34 | `entity` blocks: 32 `DestroyerGen`, 2 `NavPoint` |
| 32 | `"Command" { ... }` sub-blocks, one per unit |
| 14 | `Command = E CommandType : Cruise ;` lines; there is no other `CommandType` token in the file |
| 14 | `CommandTarget = R "" ;` lines, all in those same fourteen |
| 14 | `StartSpeed = F 12.0000 ;` lines, one on each of those fourteen units |

The other eighteen units leave the `Command` sub-block empty, and `004E6B30` queues nothing for a
sub-block with no `Command` key. `scripts/missions/usn/usn_2_java.lua` (975 lines) contains no
occurrence of `speed`, `cruise` or `throttle` in any case, which matches
`docs/GAME_EXECUTABLE.md` milestone 2n's finding that the script calls neither `SetShipSpeed` nor
`NavigatorMoveOnPath`. So the authored `StartSpeed` is the **only** speed this mission gives its
cruise ships.

`StartSpeed` is not a one-mission field: 207 of the shipped `.scn` files author it. The commonest
values are `7.71667` (434 occurrences), `5.0` (126), `6.17333`/`6.1733` (154) and `6.5` (33),
which are knots converted to m/s at 0.514444 - 15, 12 and 12.6 knots. `12.0` is 23.3 knots.

## The call path to the seed

`00822C20` has no direct caller worth the name: its four Ghidra callers (`0074BEC0`, `007593D0`,
`00853630`, `00857C80`) are themselves vtable-only overrides. It is **slot `0A0h`** of the
game-unit vtable family. Read out of `.rdata` on disk:

| Slot address | Holds | Vtable offset |
| --- | --- | --- |
| `00CF9150`, `00CFA818`, `00CFB7D8`, `00CFC470`, `00D09718` | `00822C20` | `+0A0h` |
| `00CFFAD0` | `0074BEC0` | `+0A0h` |
| `00D016D0` | `007593D0` | `+0A0h` |
| `00D0C020` | `00853630` | `+0A0h` |
| `00D0C6E8` | `00857C80` | `+0A0h` |

The vtable bases were found by walking `.rdata` backwards while the dwords stayed inside `.text`
(`00401000..00CE1136`); for the five long vtables the walk merged two adjacent tables, so the
`+0A0h` figure is the one four of the nine agree on and the other five are consistent with.

No `CALL dword ptr [reg+0A0h]` exists anywhere in `.text`, so the slot is always called as
`MOV reg,[vtbl+0A0h]` / `CALL reg`. Twenty-five such pairs exist; the one on this class family is

```
00926105  MOV ECX,dword ptr [ESI + 0x8]     ; the entity on the pending-init list node
00926108  MOV EDX,dword ptr [ECX]           ; its vtable
0092610a  MOV EAX,dword ptr [EDX + 0xa0]
00926110  CALL EAX
```

inside `BSP_SEntity_InitAll` (`00925F20-0092638A`, its own literal `SEntity::InitAll`). That
function's callers include `BSP_SceneFile_Read` (`0046DF00`), `BSP_SceneDatabase_CreateEntityByName`
(`0046D930`) and the fixed-step driver `BSP_Game_RunFixedSimulationSteps` (`00875BB0`), which is
what puts the seed at scene-load time, ahead of every command the same load queues.

## `00822C20`'s property-bag arm, read

`void __thiscall(unit)`, `RET 0`, body `00822C20-00824B57`. `ESI` is the `this` from
`00822C47 MOV ESI,ECX`. `EDI` is zero from `008234D1 XOR EDI,EDI`; filtering the whole listing for
`EDI` shows no write between there and `008235FC`, and `EDI` is callee-saved across the three
calls in between, so every `CMP ..,EDI` in the arm is a compare against zero.

Before the arm, `00822C4F`/`00822C65` set `unit+9C0h` from the class block's `MaxSpeed` at
`class+500h`, which is exactly the value `0080FC30` scales, and `00822CCB` calls `00822B70(unit, 1)`
to refill the navigator parameters. The reference speed the seed divides by is therefore set by
this same routine, a few hundred bytes earlier.

### The dispatch on `entity+0C0h`

```
0082352b  MOV EAX,[ESI + 0xc0]      ; the 12-byte reference holder
00823537  MOV ECX,[EAX + 0x4]
0082353a  SUB ECX,0x1
0082353d  JZ  0082356c              ; kind 1: the payload at +8h is a scene property bag
0082353f  SUB ECX,0x1
00823542  JNZ 00823704              ; neither 1 nor 2: skip the whole block
00823548  MOV EAX,[EAX + 0x8]       ; kind 2: a different payload, read at +0C4h/+11Ch/+128h
```

### The two finds and the value

```
0082356f  PUSH 0xd0659c             ; "ShipYardLaunch"
00823576  CALL 008f2260             ; BSP_ScenePropertyBag_Find
0082357b  CMP EAX,EDI
0082357f  MOV BL,byte ptr [EAX + 0xc]    ; the bool; BL stays at 00823527's XOR when absent
00823582  MOV EDX,[ESI + 0xc0]
00823588  MOV ECX,[EDX + 0x8]
0082358b  PUSH 0xcfccfc             ; "StartSpeed"
00823590  CALL 008f2260
00823595  CMP EAX,EDI
00823597  JZ  008235fc              ; absent: the whole seed is skipped
00823599  CMP dword ptr [EAX + 0x4],EDI
0082359c  JNZ 008235a5
0082359e  CVTSI2SS XMM0,dword ptr [EAX + 0xc]   ; type 0 (`I`)
008235a5  MOVSS XMM0,dword ptr [EAX + 0xc]      ; every other type, read as float32
```

The type test is a compare against zero, not a check for type 1. Any type other than `I` takes the
float load, including a type whose `+0Ch` is a pointer. Authored scenes only ever use `F`, so the
fall-through never bites in shipped data; the projection keeps the native's shape anyway.

### The two stores

```
008235aa  MOVSS [ESP + 0x14],XMM0   ; the authored speed
008235b0  FLD   float [ESP + 0x14]
008235b6  FSTP  double [ESP + 0x20]
008235ba  CALL  0080fc30            ; ST0 = the reference speed
008235bf  FDIVR double [ESP + 0x20] ; ST0 = authored / reference, in double
008235c3  PUSH  ECX                 ; the argument slot; ESP drops by four here
008235c4  LEA   ECX,[ESI + 0x838]   ; this = the order ring
008235ca  FSTP  float [ESP + 0x18]  ; == the old [ESP+14h]: the ratio overwrites the speed
008235ce  FLD   float [ESP + 0x18]
008235d2  FSTP  float [ESP]
008235d5  CALL  0080d9b0            ; RET 4, so ESP is back where it was
008235da  MOV   ECX,ESI
008235dc  CALL  0080fc30            ; the reference speed, fetched a second time
008235e1  FMUL  float [ESP + 0x14]  ; that slot now holds the ratio, not the speed
008235e5  PUSH  ECX
008235e6  MOV   ECX,[ESI + 0x1018]  ; this = the unit motion controller
008235ec  FSTP  float [ESP + 0x18]
008235f0  FLD   float [ESP + 0x18]
008235f4  FSTP  float [ESP]
008235f7  CALL  0092d770
```

Two details matter for reproducing it. `0080FC30` is called **twice**, not once with the result
kept. And the value `0092D770` receives is the reference speed multiplied back by the float32
ratio, not the authored speed passed through: `008235C3`'s `PUSH` shifts `ESP` by four, so
`008235CA` writes the ratio into the slot `008235AA` had used, and `008235E1` reads it back from
there after `0080D9B0`'s `RET 4` restores `ESP`. For `12.0 / 16.4622` the round trip happens to be
exact; for `7.71667` over a reference of `15.0` it is not, and the projection's one new test case
pins that.

The x87 work is read from the listing, not the pseudocode. The numerator reaches the stack as a
double and the quotient is rounded to float32 on the store; with an x87 mantissa of 53 or 64 bits
that is the correctly rounded float32 quotient, so a double divide and one narrowing cast
reproduce it.

### What the seed does **not** write

No store in `0082352B..008235FB` touches `*(unit+73Ch)`, the ring's rudder `+14Ch`, the confirmed
pair `+15Ch`/`+160h`, or `unit+0FC4h`/`unit+0FDCh`. The commanded-speed pair stays at the
constructor's `-1.0f` (`0081F273`/`0081F278`), which is why the mission reports
`commanded_speeds=0` and why the rudder stays zero and the latch keeps the heading.

## The callees, not redone

| Native | Name | Where it already lives |
| --- | --- | --- |
| `008F2260` | `BSP_ScenePropertyBag_Find` | `docs/SCENE_PROPERTY_BAG.md` |
| `0080FC30` | `BSP_UnitInstance_GetReferenceSpeed` | `bsp::unit_reference_speed_0080fc30`, `include/bsp/unit_motion.hpp` |
| `0080D9B0` | `BSP_UnitOrderRing_SetThrottleImmediate` | `bsp::set_unit_order_ring_param_a_0080d9b0`, `src/unit_state_message.cpp` |
| `0092D770` | `BSP_UnitController_SetAxialSpeed` | `bsp::unit_set_axial_speed_0092d770`, `include/bsp/unit_forces.hpp` |
| `00835AC0` / `00835E17` | the cruise latch | `bsp::cruise_command_begin_00835e17`, `src/cruise_command.cpp` |
| `009E1170` | the cruise AI step | `bsp::cruise_ordered_values_009e1170`, `src/cruise_command.cpp` |

## Host table

The methods the executable must implement, in native call order.

| Step | Site | Callee | this | args | Gate | Host method |
| --- | --- | --- | --- | --- | --- | --- |
| call the unit's init slot | `00926110` | vtable `0A0h` = `00822C20` | the entity at `[node+8h]` | none | the entity is on `BSP_SEntity_InitAll`'s pending list (site is in `00925F20`, not in this packet's main routine) | - (the executable's own entity init) |
| find `ShipYardLaunch` | `00823576` | `008F2260` | `[[unit+0C0h]+8h]` | `"ShipYardLaunch"` | `[unit+0C0h]+4h == 1` (`0082353D`) | `find_shipyard_launch_00823576` |
| find `StartSpeed` | `00823590` | `008F2260` | `[[unit+0C0h]+8h]` | `"StartSpeed"` | same arm | `find_start_speed_00823590` |
| reference speed | `008235BA` | `0080FC30` | the unit | none | the record was found (`00823597`) | `unit_reference_speed_0080fc30` |
| seed the ring throttle | `008235D5` | `0080D9B0` | `unit+838h` | `float32(StartSpeed / reference)` | same | `set_order_ring_throttle_0080d9b0` |
| reference speed, again | `008235DC` | `0080FC30` | the unit | none | same | `unit_reference_speed_0080fc30` |
| seed the hull velocity | `008235F7` | `0092D770` | `[unit+1018h]` | `float32(reference * ratio)` | same | `set_controller_axial_speed_0092d770` |

`00926110` is an indirect call, so `tools/verify_report_calls.py` reports it as `indirect` and
skips it; `reports/cruise_speed_setting.json` records it under `native_unverified` with the
`.rdata` evidence for the slot. The other six rows verify.

## Worked example

HNLMS DeRuyter, class `DeRuyter` (`VehicleClass[20]`), `MaxSpeed` `16.4622` m/s at `class+500h`,
gameplay scale `1.0`:

| Quantity | Value |
| --- | --- |
| authored `StartSpeed` | 12.0 m/s |
| `0080FC30(unit)` | 16.4622 m/s |
| `ring +148h` after `0080D9B0` | 0.7289426326751709 |
| argument to `0092D770` | 12.0 m/s |
| latched `cruiseThrust` | 0.7289426326751709 |
| latched `cruiseIsHeading` | true |
| ordered mode at `009E1265` | `Heading`, steering to the spawn heading |

The executable's stand-in for the same ships is a zero ring, which latches thrust `0` and holds
them stopped. `docs/UNIT_COMMANDED_SPEED.md`'s probe run measured the other end of the same rule:
a ring throttle of `1.0` under `--cruise` settles a DeRuyter at `16.4622` m/s, so a throttle of
`0.7289` settles it at `12.0`.

## Coverage

| Routine | Address | Coverage |
| --- | --- | --- |
| `scene_unit_start_speed_seed_arm` | `0082356C..008235FB` | complete |
| `scene_start_speed_value_00823599` | `00823599..008235AA` | complete |
| `scene_start_speed_seed_008235b0` | `008235B0..008235F7` | complete |
| `scene_cruise_ship_speed` | - | composition, not a native routine: chains `008235B0`, `00835E17` and `009E1265` in the order the game runs them |
| `00822C20` as a whole | `00822C20..00824B57` | partial: `0082352B..008235FB` read here. The prologue `00822C20..00822CD0` and the modifier-group reset `00823704..00823742` are `docs/UNIT_FORCE_COMMANDS.md`'s; the kind-2 arm `00823548..00823567`, the `CamoColor`/`CamoColorGun` block `008235FC..008236FC` and everything from `00823742` to `00824B57` are unread |
| `00925F20` `BSP_SEntity_InitAll` | `00925F20..0092638A` | partial: only the slot call `00926105..00926112`. The list walk, the gates above `009260A0` and the second loop from `00926120` are not read |
| `008F2260`, `0080FC30`, `0080D9B0`, `0092D770`, `00835AC0`, `009E1170` | - | not redone: existing reconstructions, referenced |

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/SCENE_ENTITY_CREATE.md` line 144 and `include/bsp/scene_entity_create.hpp` line 37: the 12-byte holder at `entity+0C0h` is `{vtable +0, refs +4, bag +8}` | `+4h` is a **kind tag**, not a reference count. `00922E20` (vtable `00D03D94`) stores the literal `1` next to a cloned property bag; `00774DC0` (vtable `00D03754`) stores the literal `2` next to a different payload. `00823537..00823542` dispatches on exactly `1` and `2` and reads `+8h` as a bag only for `1`; the kind-2 payload is read at `+0C4h`, `+011Ch` and `+0128h`, past the `114h`-byte end of a property bag | `00922E32`/`00922E35`; `00774DE4`/`00774DEF`; `00823537..00823567`; `kScenePropertyBagSize = 0x114` |
| `docs/CRUISE_COMMAND.md`: "Nothing in the command, the message or the scene record carries a speed or a heading" | True of the command and the message; wrong about the scene record. `StartSpeed` on the unit entity becomes the ring throttle the latch reads, and the entity's `localframe` orientation becomes the heading the latch keeps | `usn_2_java.scn` lines 1052..1373; `00823582..008235F7`; `00835AE0` |
| `docs/CRUISE_COMMAND.md`: "The producer of `+24h` is contract: unread ... no Lua binding string names it" | Already corrected by `cc_commanded_speed`: `luaMW_SetShipSpeed` `00890D30` and `luaMW_NavigatorMoveOnPath` `008A3600`. This packet adds only that **no scene property** feeds the pair | `docs/UNIT_COMMANDED_SPEED.md`; `0082352B..008235FB` has no store to `[unit+73Ch]` |
| `docs/GAME_EXECUTABLE.md` milestone 2n: "Nineteen of the 32 authored `None`" and "The thirteen ships that author `Cruise`" | The shipped file has **14** units with `Command = E CommandType : Cruise` and **18** with an empty `Command` sub-block. There is no `None` token in the file at all. The executable's counts are off by one in each direction | `usn_2_java.scn`: 32 `"Command" {`, 14 `Command = ` lines all `Cruise`, 14 `CommandTarget = R`, 32 `DestroyerGen` + 2 `NavPoint` |

The first and the fourth are **not applied**: `docs/SCENE_ENTITY_CREATE.md` and its header belong
to another packet and `docs/GAME_EXECUTABLE.md` is owned by `agent/cc-exe-2p`. They are follow-ups
below. The second and third are recorded here rather than edited into `docs/CRUISE_COMMAND.md`,
which is the parent packet's file.

## no_ghidra_function

none. Every address this packet names lies inside an existing Ghidra function: `00822C20`
(`00822C20-00824B57`), `00925F20` (`00925F20-0092638A`), `00922E20` (`00922E20-00922E41`),
`00774DC0` (`00774DC0-...`, Ghidra function present) and the four callees.

## Follow-up packets

* `scene_entity_bag_ref_kind` - apply the `+4h` kind-tag correction to
  `docs/SCENE_ENTITY_CREATE.md` and `include/bsp/scene_entity_create.hpp`, and read `00774DC0`'s
  holder: what its `+8h` payload is and who stores it at `entity+0C0h`. `00822C20`'s kind-2 arm
  reads `+0C4h` (a byte into `unit+58h`), `+011Ch` (the `ShipYardLaunch` bool) and `+0128h` (into
  `unit+115Ch`, the camo colour), which looks like a replication or saved-state record.
* `game_executable_start_speed` - for `agent/cc-exe-2p`: wire `StartSpeed` into the executable's
  unit init so the thirteen stationary ships make way, and re-count the mission's `Cruise` units
  (14, not 13) and its empty `Command` blocks (18, not 19).
* `unit_sentity_init_rest` - the rest of `00822C20` (`00823742..00824B57`) and the `CamoColor` /
  `CamoColorGun` block, plus the other four occupants of slot `0A0h` (`0074BEC0`, `007593D0`,
  `00853630`, `00857C80`) to see which of them chain the base body.
* `sentity_init_all` - `00925F20`'s list walk, the gate above `009260A0`, the `FIDIV`/`0057C1A0`
  step just before the slot call, and the second loop from `00926120`.
* `unit_autopilot_pair` - still open from `docs/CRUISE_COMMAND.md` and `docs/UNIT_COMMANDED_SPEED.md`:
  who writes `unit+0FC4h` and `unit+0FDCh`. Not touched here.

## Uncertainties

* The vtable slot is `0A0h` on the strength of four of the nine `.rdata` occurrences, whose
  enclosing tables the backward walk resolved cleanly; the other five sit in tables the walk
  merged with their neighbours and are only *consistent* with `0A0h`. A misread base would move
  the slot number without changing the call site, which is established independently by
  `00926105..00926110` reading `[[entity]+0A0h]`.
* `00926110` is the only call site of slot `0A0h` on this family that the scan found. The scan
  covered `MOV reg,[reg+disp32=0A0h]` followed by `CALL reg` within 24 bytes, plus every
  `CALL dword ptr [reg+0A0h]` (none exist). A call that spills the loaded pointer to the stack
  first would not have shown up, so "the only site" is a bounded claim.
* Whether `00822C20` runs before or after the queued `Cruise` is argued from the call graph
  (`BSP_SceneFile_Read` -> `BSP_SEntity_InitAll` -> the slot, against `004E6B30`'s queue that
  `0046AAB0` resolves and a session message delivers) and from `00822C20` being the routine that
  sets `unit+9C0h`, which every later speed consumer needs. No run log was taken: `bsp_game.exe`
  does not reach the `StartSpeed` arm, which is the point of the packet, so checklist rule 6 has
  nothing to measure here.
* The kind-2 payload at `[entity+0C0h]+8h` is `contract: unread`. Its three fields are named above
  by their consumer, not by their producer.
* `StartSpeed` is read here only on the ship path. Whether the plane, submarine and fort creators'
  own slot-`0A0h` bodies read the same key was not checked.
