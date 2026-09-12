# The authored `Cruise` order (`00E08F70`), from the scene record to the ordered pair

Addresses: 00816E30 0071ECF0 0071E550 00835E90 0071C830 0071D880 00721A40 00721030 007216D0
0077A050 008358D0 0071E6C0 0071DB50 0071D780 0071E200 0071D6D0 00836040 0071D810 0071BE40
00835AC0 00835C70 009E1170 008356C0 008356D0 008356E0 008356F0 009DAC20 00764D00 0071C900
008A75C0 00780120 00778820 00E08F70 00CFC530 00D09F58 00CFD9EC 00CFD9D8 00D21598

Packet `cc_cruise_command`, worker `agent/cc-cruise-command`, 2026-09-11 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves.
Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query
verified both. Descriptive names are hypotheses, not recovered symbols. The literals in the
image are `cruise`, `MT_COMMAND`, `MT_GAMEUNIT_SETCMD`, `MT_GAMEUNIT_CLEARCMD`,
`luaMW_NavigatorCruise` and the three field names `cruiseIsHeading`, `cruiseSteerOrHeading`
and `cruiseThrust`.

## Answer to the packet question

**`Cruise` carries no parameters. It is a latch: when it becomes the unit's current command
it captures the standing order and the current heading, and every step afterwards it re-applies
what it captured.**

```
cruiseIsHeading      = |ring +14Ch| < 0.01f                  (00835AE0, the 0.01f at 00D7A238)
cruiseSteerOrHeading = cruiseIsHeading ? unit->vtable[50h]()  (the heading, 00835E46)
                                       : ring +14Ch           (the ordered rudder)
cruiseThrust         = ring +148h                             (the ordered throttle)
```

`ring +148h` / `+14Ch` are `unit+980h` / `unit+984h`, the live ordered pair of
`docs/UNIT_STATE_MESSAGE.md`. So a `Cruise` issued to a ship already making way at full
throttle holds it at full throttle; a `Cruise` issued to a ship whose ring is still zero
holds it **stopped**, which is what the probe measures below. Nothing in the command, the
message or the scene record carries a speed or a heading.

Both producers of a `Cruise` with no target agree exactly. The scene queue
(`00469610` at load, resolved by `0046AAB0`, call site `0046AC0B`) and the Lua binding
`008A75C0 luaMW_NavigatorCruise` (the command object pushed at `008A7710`, the call at `008A7729`) both reach `0077D600` with
command `00E08F70`, flags `1`, and a descriptor whose kind is 0 and whose position is the
read-only zero vector at `00F87574`. `008A75C0` takes one Lua argument, the entity, and
nothing else; that is the whole scripted interface.

## The path, hop by hop

```
scene `Command` property -> 004E6B30 -> 00469610 queue           docs/SCENE_DEFERRED_REFS.md
0046AAB0 name match in the registry 00E19A70 -> 0077D600         docs/ENTITY_ORDER_MESSAGE.md
0077D600 -> 007798D0 builds MT_COMMAND (58h): ordinal 16 at +20h, flags 1 at +21h
0077C2A0 routes; local delivery queues on session+24Ch; 0076C600 drains  docs/UNIT_STATE_MESSAGE.md
00780670: not category 47h/61h; category 49h widens the tick window to 1770h; not 48h
00780120: not category 4Bh/4Ch/57h/4Eh/4Fh/50h/51h/52h/97h; category 58h at 00780607
          -> entity->vtable[160h](msg) at 0078061C = 00816E30 for MDestroyer (00CFC530)
00816E30: decode the ordinal (0077A050), rebuild the descriptor, fall through to 00817334
          flags != 0 -> 0071D880 clears every slot; flags == 0 -> vtable[34h] = 00835E90 gates
          -> 0071ECF0
0071ECF0: notify the AI group (00A2BD90), vtable[30h] = 0071E550 makes room,
          0071C830 builds MT_GAMEUNIT_SETCMD (5Ch), 0077C2A0 routes it with flags 7
00780120 again: category 59h at 00780636 -> 00778820 wraps unit->vtable[114h]() (the
          weapon director) -> 00721A40
00721A40 5Ch arm: 00721030 rebuilds the descriptor, 007216D0 turns the +21h ordinal back
          into the object, vtable[60h] = 008358D0
008358D0 -> 0071E6C0: the slot push at director + 54h + i*1Ch
00835C70 (vtable[78h]) when the command becomes current: 00835E17..00835E58 -> 00835AC0
009E1170 every AI step: 009E1265..009E13B1 -> 009DBF90 / 009DFFB0 / 009E0040
```

The command travels twice as one byte. `007798D0` puts the ordinal at message `+20h` and the
flags at `+21h`; `0071C830` puts a flag at `+20h` and the ordinal at `+21h`, so the two
decoders differ only in which byte they read (`0077A050` reads `+20h`, `007216D0` reads
`+21h`). Both then walk the registry list at `00E19A70` and return the first object whose
`+4h` ordinal matches, or null for the `0FFh` "no command" ordinal.

## The two new message classes

Each message-class vtable in this family is five slots, `14h` bytes, and they are adjacent:

| Type | Name literal | vtable | dtor / serialize / deserialize / category |
| --- | --- | --- | --- |
| `5Ah` | `MT_GAMEUNIT_ATTR` `00D02658` | `00CFD9C4` | `0071E370` `0071C670` `0071C6C0` `0071C640` |
| `5Dh` | `MT_GAMEUNIT_CLEARCMD` `00D02614` | `00CFD9D8` | `0071E390` `0071C7A0` `0071C7E0` `0071C770` |
| `5Ch` | `MT_GAMEUNIT_SETCMD` `00D0262C` | `00CFD9EC` | `0071E3B0` `0071C930` `0071CA30` `0071C900` |

The names are literals in the type table at `00E0AB68`. `0071C900` answers true for `5Ch`,
`59h`, `49h` and `46h`; `00764D00` (MT_COMMAND) answers true for `58h`, `49h` and `46h`.
Neither answers `48h`, so neither takes `0077C710`, the entity-sync branch of `00780670`.

`0071D880`, `__thiscall(director)`, `RET 0`, builds `MT_GAMEUNIT_CLEARCMD` with `+04h = 1`,
`+20h = 1` and `+24h = -1` and routes it with flags 7 through the endpoint at `director+34h`.
`-1` is the "every slot" index: `00721A40`'s `5Dh` arm takes `00720CA0` for a negative index
and `00720850(index)` otherwise. So clearing the queue is a networked round trip too, not a
local write.

## The command slot

`0071E6C0` writes the command pointer with `MOV [EDI + ECX*4], EBP` at `0071E764`, where
`ECX = (i+3)*7`, i.e. `director + 54h + i*1Ch`, and assigns the descriptor at
`director + 58h + i*1Ch` through `0071DB50` at `0071E76C`. One slot is therefore

| Slot offset | Field |
| --- | --- |
| `+00h` | the command-type object address |
| `+04h`..`+1Bh` | the `0x18`-byte `SceneCommandTarget` |

which fills the `1Ch` stride `docs/WEAPON_DIRECTOR.md` recovered from `00720CD0`'s clear loop.

`director+30h` is the command mode, read by `0071BE40`: 1 means the current command is slot 0
at `director+54h`, 2 means the override pair at `director+188h`/`+18Ch`, anything else means
none. `0071E6C0` sets it to 1 at `0071E795` the first time a slot is filled, and only then.
`director+48h` is a stage counter `0071D810` raises and never lowers.

## The pure rules

### `00835AC0`, the latch

`__thiscall(director)(float rudder, float heading, float thrust)`, `RET 0Ch`.
Coverage: complete, `00835AC0-00835B35`.

```
t = (rudder > 0.0f) ? rudder : (-0.0f - rudder)      ; 00835AC6/00835AD4, -0.0f at 00D7A208
director[243h] = (0.01f > t)                         ; 00835AE0/00835AE8/00835AF8, 00D7A238
if (director[243h]) director[244h] = heading         ; 00835B06
else                director[244h] = rudder          ; 00835B25
director[248h] = thrust                              ; 00835B14 / 00835B2D
```

The `COMISS 0.01f, t` / `JBE` pair means the flag is set only on a strict `0.01f > t`, and an
unordered compare (a NaN rudder) clears it.

### `00835C70`'s `cruise` arm, what it latches

`00835E0E..00835E5D`. Reached when the newly current command is `cruise` (`00E08F70`) or
`stop` (`00E08F88`); only `cruise` runs the latch (`00835E17 CMP EDI,0xE08F70`).

```
0071D810(director, 1)                                ; 00835E12, raise the stage
unit = director[24Ch]                                ; 00835E1F
[esp+0Ch] = unit[980h]                               ; 00835E25/00835E30, the ordered throttle
[esp+14h] = unit[984h]                               ; 00835E34/00835E3B, the ordered rudder
heading   = unit->vtable[50h]()                      ; 00835E46
00835AC0(director, unit[984h], heading, unit[980h])  ; 00835E58
```

The three stack slots were tracked across `PUSH ECX` at `00835E3A`, the virtual call at
`00835E46` (a `RET 0` getter) and `SUB ESP,8` at `00835E48`; `FLD [ESP+1Ch]` at `00835E51`
lands on the `unit[984h]` slot, which fixes the argument order.

### `009E1170`, the per-step rule

The AI ship's cruise state. Its class is the vtable at `00D21598`, whose `vtable[24h]` is
`009DAC20` (`MOV EAX,0xE08F70 ; RET`), i.e. "this state's command is `cruise`"; `009F3DD0`
keeps the state in sync with `0071BE40`'s answer. `vtable[0Ch]` is the work, `009E1170`.
Coverage of the AI arm `009E1265..009E13B1`: complete.

```
throttle = director->cruiseThrust()                  ; 009E126B -> 008356F0, [director+248h]
speed    = 0092D730(unit[1018h])                     ; 009E1282
absSpeed = speed & 7FFFFFFFh                         ; 009E1297
if ( *(unit+73Ch)[28h] >= 0.0f )                     ; 009E12AC, the zero at 00D7A218
    throttle = *(unit+73Ch)[24h] / 0080FC30(unit)    ; 009E12BD..009E12D7, FDIVR in double
if (!director->cruiseIsHeading())                    ; 009E130E -> 008356C0, [director+243h]
    009DFFB0(director->cruiseSteerOrHeading())       ; 009E138C -> 008356D0, then 009E1397
else if (|throttle| > 0.05 || absSpeed > 1.0f)       ; 009E1329..009E134E, 00D7A270, 00D7A24C
    009E0040(director->cruiseSteerOrHeading())       ; 009E135C -> 008356E0, then 009E1367
else
    009DFFB0(0.0f)                                   ; 009E1350 FLDZ, then 009E1397
009DBF90(throttle)                                   ; 009E1376 / 009E13A6
```

`009DFFB0` sets a desired rudder and `009E0040` a desired heading; `009DBF90` sets the desired
throttle. All three write the AI controller block at `[state]+8`, not the ring.

The four director getters are one instruction each, and they are where the literal field names
come from (`008362A0` dumps the same three offsets under those names):

| Getter | Body | Field |
| --- | --- | --- |
| `008356C0` | `MOV AL,[ECX+243h] ; RET` | `cruiseIsHeading` |
| `008356D0` | `FLD [ECX+244h] ; RET` | `cruiseSteerOrHeading` as a rudder |
| `008356E0` | `FLD [ECX+244h] ; RET` | the same field as a heading |
| `008356F0` | `FLD [ECX+248h] ; RET` | `cruiseThrust` |

### `0071E550` and `00835E90`, the two queue gates

`0071E550` (`vtable[30h]`, `RET 8`) pushes nothing. It drops the top queued command when the
incoming command and the top one are both weapon commands (category 1 or 2), so the incoming
one has room; `0071D900(count-1)` at `0071E5AA` is the removal.

`00835E90` (`vtable[34h]`, `RET 4`, no Ghidra function) is the gate `00816E30` applies when the
message's flags byte is zero:

```
if (!0071C0C0(command)) return false                 ; 00835E99: command != 0 and a slot is free
if (count <= 0) return true                          ; 00835EB2
top = slot[count-1].command                          ; 00835EC3
if ((top == 00E08FA0 land || top == 00E08F60 follow) && count-1 > 0) return false  ; 00835ED6
if (category(top) not in {1,2}) return true          ; 00835EE1/00835EE6
return category(command) in {1,2}                    ; 00835EF4/00835EF9
```

A weapon command on top therefore accepts only another weapon command. The authored `Cruise`
never reaches this gate: its flags byte is 1, so `00816E30` takes `0071D880` instead.

## The ring is not written by the cruise state

`00825F20` refills the whole ring from two unit fields once per step, and only when a gate
byte is set:

```
008266C1: CMP byte [EDI-2AFh],0        ; unit+61h; zero skips the whole block
008266DA: MOVSS XMM0,[EDI+0CB4h]       ; unit+0FC4h
008266CE..00826703: fill every slot between the cursors with it
00826708: MOVSS [EBX+148h],XMM0        ; ring +148h, the ordered throttle
0082671C: MOVSS XMM0,[EDI+0CCCh]       ; unit+0FDCh
00826710..00826746: the same walk for the second field
0082674C: MOVSS [EBX+14Ch],XMM0        ; ring +14Ch, the ordered rudder
```

`EDI` is `unit+310h` (`00826116 LEA EBX,[EDI+528h]` gives the ring at `unit+838h`).
`00834E90 BSP_UnitInstance_UpdatePropellers` reads the same selector at `008350BD`:
`unit+61h ? unit+0FC4h : unit+980h`. So `unit+61h` chooses between an autopilot pair and the
ring's own pair, and the autopilot pair is where a cruise-driven ship's order arrives.

**The bridge from `009DBF90` / `009DFFB0` / `009E0040` to `unit+0FC4h` / `unit+0FDCh` /
`unit+61h` is `contract: unread`.** A full `.text` scan for the three displacements finds only
`0081ED40 BSP_UnitVehicleBase_Construct` (the initial store), `00834E90` (a read) and
`00825F20` (the read above); like `unit+980h`/`+984h` before it, the writer addresses them
through a base this packet did not identify.

## The commanded speed at `*(unit+73Ch)`

`unit+73Ch` is a pointer, four bytes past the weapon director at `unit+738h`, to a parameter
block `00822B70` fills from the game tuning singleton `00424C40` (`+160h..+178h` into the
block's `+0h..+18h`). Two of its fields belong to cruise:

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+24h` | a commanded speed in m/s | `009E12BD`, divided by `0080FC30`'s reference speed |
| `+28h` | its enable; `>= 0.0f` active, `-1.0f` inactive | `009E12AC`; `009E11C6` stores `-1.0f` |

`00836920` (`vtable[7Ch]`, the director's step) reads the same enable at `00836AA7..00836AC8`
and raises the command stage to 2 when it is active and the unit is not player-controlled.
**The producer of `+24h` is `contract: unread`**: no direct-displacement store to it was found
and no Lua binding string names it (`luaMW_LandConvoySetSpeed` and `luaMW_SquadronSetSpeed`
are the only `SetSpeed` bindings in the image and neither was read).

## The other arms of `009E1170`

`009E11A9..009E1262` is not the cruise rule and is read here only for the fields it writes.
Its first arm runs when `[[unit+740h]+50h]+1B0h` is neither 8 nor accepted by `00927F10`: it
clears the speed setting (`*(unit+73Ch)[28h] = -1.0f`) and calls `00835AC0` with
`(unit[998h], unit->vtable[50h](), unit[994h])`, i.e. the ring's **confirmed** pair at
`+15Ch`/`+160h` rather than the live one. Its second arm runs when `unit+184h` is set, the
player-controlled byte, and forwards the same confirmed pair to `009DFFB0` and `009DBF90`
without touching the cruise fields.

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/MOTION_DIFFERENTIAL.md` follow-up `cruise_command_object`: "what the authored `Cruise` order writes into the ring" | `Cruise` writes nothing into the ring. It writes three fields on the weapon director at `+243h`/`+244h`/`+248h`, and the ring is refilled from `unit+0FC4h`/`unit+0FDCh` by `00825F20` under the `unit+61h` gate, from a producer this packet could not find. | `00835B06`/`00835B14`/`00835AF8` are the only stores the latch makes; `00826708`/`0082674C` are the only ring writes in the motion update and their sources are `EDI+0CB4h`/`EDI+0CCCh`. |
| `docs/GAME_EXECUTABLE.md` line 2988: "What that token means is not recovered" | Recovered: the latch above. The executable's stand-in (one order-ring order of throttle 1 and rudder 0 through `00816A40`) is what the latch produces for a ship already running at throttle 1 with no rudder, so the stand-in is right for that case and wrong for a ship at rest. | `00835E17..00835E58` and the probe run below. |
| `docs/WEAPON_DIRECTOR.md`: `kDirectorVtableSlotIssueCommand = 0x58` (`00720CD0`) | Slot `58h` is the clear-and-issue helper. The plain command setter is slot `60h` = `008358D0`, which is what `00721A40` calls on the receive side and what `00720CD0` itself invokes. The existing constant is not renamed here; `kCruiseDirectorVtableSlotSetCommand` names `60h`. | `00D09F58+60h` = `008358D0`; `00721B56 MOV EDX,[EBX+0x60]`. |
| `docs/WEAPON_DIRECTOR.md`: "`0071C1E0` ... Who calls it from the session side is a contract: unread" | The session side is `00780120`'s category `59h` arm at `00780636`, which wraps the director with `00778820` and calls `00721A40`; `00721A40`'s `5Ah` arm is `director->vtable[38h]`, whose base body is `0071C1E0`. | `00780644..00780653`; `00721A81..00721A93` is the `5Ah` arm and its `vtable[38h]` call. |

## Host table

One row per native call site the sequences in `src/cruise_command.cpp` project. Indirect rows
name the vtable and slot instead of a callee address.

| Step | Site | Callee | this | args | Gate |
| --- | --- | --- | --- | --- | --- |
| ordinal to object | `00816E9C` | `0077A050` | - (`__fastcall`, `ECX` = the message) | none | always |
| clear every slot | `0081733E` | `0071D880` | `[unit+738h]` | none | `msg+21h != 0` |
| accept gate | `0081734B` | `00D09F58+vtable34` = `00835E90` | the director | the command | `msg+21h == 0` |
| issue | `0081735D` | `0071ECF0` | the director | command, the local descriptor | the gate passed |
| session entity | `0071ED19` | `00D09EC0+vtable140` | `[director+34h]` | none | always |
| class query | `0071ED32` | indirect `vtable[5Ch]` | the entity | `2` | the entity is non-null |
| remote query | `0071ED43` | `007788B0` | the entity | none | `entity+16Ch != 0` |
| AI group forward | `0071ED54` | `00A2BD90` | `entity+16Ch` | command, descriptor | not remote |
| make room | `0071ED62` | `00D09F58+vtable30` = `0071E550` | the director | command, descriptor | always |
| build SETCMD | `0071ED6C` | `0071C830` | a stack message | command, descriptor, `1` | always |
| route | `0071ED81` | `0077C2A0` | `[director+34h]` | the message, `7`, `0` | always |
| drop a slot | `0071E5AA` | `0071D900` | the director | `count-1` | both categories are 1 or 2 |
| category test | `00721AEE` | indirect `vtable[0Ch]` | the message | `5Ch` | always |
| descriptor (probe) | `00721B11` | `00721030` | the message | a local | session mode 2 |
| descriptor (again) | `00721B22` | `00721030` | the message | a local | the probe's kind is non-zero |
| resolve or drop | `00721B29` | `00521EA0` | - (`__fastcall`) | the descriptor | same |
| descriptor | `00721B47` | `00721030` | the message | a local | `msg+20h != 0` |
| ordinal to object | `00721B4F` | `007216D0` | the message | the descriptor (left on the stack) | same |
| set command | `00721B5A` | `00D09F58+vtable60` = `008358D0` | the director | command, descriptor | same |
| descriptor | `00721B6C` | `00721030` | the message | a local | `msg+20h == 0` |
| ordinal to object | `00721B74` | `007216D0` | the message | the descriptor | same |
| queue command | `00721B7C` | `0071E7F0` | the director | command, descriptor | same |
| push the slot | `008358DF` | `0071E6C0` | the director | command, descriptor | always |
| category | `008358F9` | indirect command `vtable[0Ch]` | the command | none | the push succeeded |
| resolve target | `00835907` | `00521EA0` | - | the descriptor | category 1 or 2 |
| resolve target | `00835928` | `00521EA0` | - | the descriptor | session mode 0 or 1 |
| set fire target | `00835930` | `00835860` | the director | the object, force `1` | same |
| command count | `0071E6C3` | `0071D780` | the director | none | always |
| previous target | `0071E70C` | `0071E200` | `director+3Ch+i*1Ch` | the descriptor | `i > 0` |
| allowed | `0071E72A` | `0071D6D0` | the director | command, descriptor | not a duplicate |
| normalise a self target | `0071E73C` | `00D09F58+vtable14` = `00836040` | the director | command, descriptor | allowed |
| assign the target | `0071E76C` | `0071DB50` | `director+58h+i*1Ch` | the descriptor | the push runs |
| resolve target | `0071E773` | `00521EA0` | - | the descriptor | same |
| resolve target | `0071E781` | `00521EA0` | - | the descriptor | non-null |
| observe | `0071E78A` | `00694A60` | the resolved object | `EDX` = `director+1Ch` | same |
| category | `0071E7A8` | indirect command `vtable[0Ch]` | the command | none | the command is non-null |
| echo gate | `0071E7C9` | `006E38E0` | `[00E188A8]+1EF0h` | none | `director+188h != 0` |
| echo | `0071E7D4` | `0071E2E0` | the director | none | the gate passed |
| raise the stage | `00835E12` | `0071D810` | the director | `1` | the command is `cruise` or `stop` |
| heading | `00835E46` | `00CFC3D0+vtable50` | the unit | none | the command is `cruise` |
| latch | `00835E58` | `00835AC0` | the director | rudder, heading, thrust | same |
| cruiseThrust | `009E126B` | `008356F0` | the director | none | the AI arm |
| body speed | `009E1282` | `0092D730` | `unit+1018h` | none | same |
| reference speed | `009E12CE` | `0080FC30` | the unit | none | the speed setting is enabled |
| cruiseIsHeading | `009E130E` | `008356C0` | the director | none | the AI arm |
| heading field | `009E135C` | `008356E0` | the director | none | heading arm, moving |
| hold heading | `009E1367` | `009E0040` | the state | the heading | same |
| throttle | `009E1376` | `009DBF90` | the state | the throttle | same |
| steer field | `009E138C` | `008356D0` | the director | none | the rudder arm |
| steer | `009E1397` | `009DFFB0` | the state | the rudder or zero | rudder or straight arm |
| throttle | `009E13A6` | `009DBF90` | the state | the throttle | same |
| entity command dispatch | `0078061C` | `00CFC530` = `00816E30` | the entity | the message | category `58h` |
| director wrapper | `00780649` | `00778820` | a stack word | the entity | category `59h` |
| gameunit dispatch | `00780653` | `00721A40` | the wrapper | the message | same |

### The five gates, read

Each of these entered the first pass as an unread callee and is read here, so no host method in
`include/bsp/cruise_command.hpp` carries an invented verb.

| Callee | ABI and body | What it does |
| --- | --- | --- |
| `0071C0C0` | `__thiscall(director)(command)` | false for a null command; otherwise the index of the first empty slot is below 10, i.e. there is room. It is `00835E90`'s `base_allows`. |
| `0071D780` | `__fastcall(director)`, `RET 0`, `0071D780-0071D807` | walks the slots and returns the first empty index, except that a `moveonpath` slot (`00E08F80`) whose path object at `director+1A4h+i*4` reports a non-empty point vector contributes that point count instead of 1. |
| `0071D6D0` | `__thiscall(director)(command, target)`, `0071D6D0-0071D772` | when the command's `vtable[8]` requires a target and either the descriptor's `position_valid` byte is clear or the category is 1 or 2, the target must resolve and must not carry the `+5Dh` flag. `torpedo` (`00E08F18`) and `moveonpath` (`00E08F80`) add tests through `009229F0` and `007AC9D0`, whose bodies stay unread. |
| `00836040` | `__thiscall(director)(command, target)`, `00836040-008360BC` | **not a supersede test.** When the descriptor names a target, that target resolves to the director's own endpoint at `director+34h`, and the command is `cruise` or `stop`, it rewrites the descriptor **in place** to an empty one (kind 0, id 0, object 0, the zero vector at `00F87574`, zero trailer) and returns 1. Otherwise it returns 0 and writes nothing. `0071E6C0` then stores the rewritten descriptor, so a `Cruise` aimed at the ship's own session object becomes a targetless `Cruise`. |
| `0071E200` | `__thiscall(slotTarget)(otherTarget)`, `0071E200-0071E2D9` | both descriptors resolve to the same object and their positions are within a squared distance of `1.0`, where a descriptor whose `position_valid` byte is clear contributes the zero vector at `00F87574`. |
| `007788B0` | `__fastcall(entity)`, `RET 0`, `007788B0-007788C7` | `ctrl = [entity+284h]; return ctrl != 0 && [ctrl+14h] != entity`. The AI-group forward in `0071ECF0` is skipped when it is true. |
| `006E38E0` | `__fastcall(this)`, `RET 0`, `006E38E0-006E38F7` | `x = [this+0F4h]; return x == 0 \|\| x == 1`, with `this` = `[00E188A8]+1EF0h`. |

The three AI setters were read too, and they are why the cruise step's three host methods are
named for what they write. All are `__thiscall(state)(float)`, `RET 4`, over the control block at
`[state]+8`:

| Callee | Body | Effect |
| --- | --- | --- |
| `009DFFB0` | `009DFFB0-009E0011` | sets the block's mode word `+1C4h` to 0, zeroing `+360h` and `+368h` on the switch, then stores the value clamped to `[-1,+1]` (`00D7A260`, `00D7A24C`) at `+1D4h` |
| `009E0040` | `009E0040-009E0095` | sets the same mode word to 1, the same zeroing, stores the value **unclamped** at `+1D8h` and calls `00605070` on it (body unread) |
| `009DBF90` | `009DBF90-009DBFD0` | clears `+1C8h` and `+1CCh` and stores the value clamped to `[-1,+1]` at `+1D0h` |

So `+1C4h` is the steering mode, `+1D4h` the rudder, `+1D8h` the held heading and `+1D0h` the
throttle. How that block reaches `unit+0FC4h` / `unit+0FDCh` is still the unread hop.

## Coverage

| Routine | Coverage |
| --- | --- |
| `00835AC0`, `0071E550`, `0071BE40`, `00721030`, `007216D0`, `0077A050`, `00764D00`, `0071C900`, `008356C0`, `008356D0`, `008356E0`, `008356F0`, `009DAC20` | complete |
| `0071ECF0`, `008358D0`, `0071E6C0` | complete |
| `00835E90` | complete as a body; `0071C0C0` enters as an argument and is unread |
| `0071D880`, `0071C830` | complete as builders; their receive side is `00721A40` |
| `00816E30` | partial: `00816EA6..00817330` (the `follow`, `land`, `disband`, `settarget`, `cleartarget`, `clearorders`, `moveto`, `attackmove` and `artillery` arms) is read in pseudocode but not projected. The projected path is the movement fall-through plus the tail `00817330..00817374`. |
| `00721A40` | partial: only the `5Ch` arm `00721AEE..00721B87`. The `5Ah`, `5Bh`, `5Dh`, `5Eh`, `5Fh` and `60h` arms are read in pseudocode and summarised above but not projected. |
| `00835C70` | partial: only the `cruise` arm `00835E0E..00835E5D`. `00835C70..00835E0E` and `00835E5D..00835E83` are not projected. |
| `009E1170` | partial: only the AI arm `009E1265..009E13B1`. `009E1170..009E1265` is read and summarised; `009E13B4..009E14B7` is unread. |
| `00780120` | partial: the category cascade and the two arms this packet needs (`58h` at `00780602`, `59h` at `00780631`). The `4Bh` block `00780170..007803D8` and the `4Ch`, `57h`, `4Eh`, `4Fh`, `50h`, `51h`, `52h` and `97h` arms are not read. |
| `00822B70` | partial: the tuning copy into `*(unit+73Ch)+0h..+18h`; `+24h`/`+28h` are not written there. |
| `008A75C0` | complete as an argument shape: one Lua entity argument, an empty descriptor, flags 1. |

## The probe

`src/ship_motion_probe.cpp --cruise [--cruise-frame N]` runs the hand order for `N` steps
(default 20, the count `docs/GAME_EXECUTABLE.md` milestone 2j names), then latches it with
`cruise_command_begin_00835e17` and drives every later step from
`cruise_ordered_values_009e1170`. Only the rudder arm of that rule reaches the ring; the
heading arm goes through `009E0040`, the ship AI's heading controller, which is not
reconstructed, so the probe issues a zero rudder there and says so. A ship already on its
latched heading is steered with a zero rudder either way, which is the case the mission's
straight-running ships are in; a `Cruise` latched while the ship is off its held heading is
**not** covered. The commanded-speed override is left disabled because its producer is unread.

`VehicleClass[20]` (DeRuyter class 1935, `MaxSpeed` 16.4622, `MaxRotAngle` 0.122173), 280 steps
of 0.05 s = 14.0 s, class-built hull body:

| Run | latched fields | steering arm | z (m) | heading (deg) | final speed (m/s) | peak yaw (rad/s) |
| --- | --- | --- | --- | --- | --- | --- |
| `--throttle 1 --rudder 0` (hand order) | - | - | 184.8910 | 0.0000 | 16.4622 | 0.00000 |
| `--throttle 1 --rudder 0 --cruise` | heading, 0.0, thrust 1.0 | heading | 184.8910 | 0.0000 | 16.4622 | 0.00000 |
| `--throttle 1 --rudder 0.5` (hand order) | - | - | 187.4494 | -19.9637 | 16.4548 | 0.02757 |
| `--throttle 1 --rudder 0.5 --cruise` | rudder, 0.5, thrust 1.0 | rudder | 187.4494 | -19.9637 | 16.4548 | 0.02757 |
| `--throttle 1 --rudder 0 --cruise --cruise-frame 0` | heading, 0.0, thrust **0.0** | straight | 0.0000 | 0.0000 | 0.0000 | 0.00000 |

The first four rows are the check: the latch reproduces the standing order exactly, in both
arms, to the last printed digit. The fifth is the finding. Latching at frame 0 catches the ring
before it has slewed (`slew_a` 8.0/s and `slew_b` 2.0/s from `00812F84`/`00812F8C`, so the pair
is still zero on the first step), and the ship never moves. Milestone 2j's mission run has all
32 ships moving 163 to 210 m under the authored `Cruise`, so in the shipped game something
puts a throttle on those ships before or instead of the latch, and the only candidate this
packet found is the commanded speed at `*(unit+73Ch)+24h` whose producer is unread.

## no_ghidra_function

| Start | Inclusive end | Evidence for the boundary |
| --- | --- | --- |
| `00835E90` | `00835F02` | `00835E90 PUSH ESI` after the `RET 0Ch` of `00835C70`'s neighbour at `00835E83` and eleven `INT3` fillers at `00835F05..00835F0F`; `00835EA6`, `00835EF2` and `00835F02` are its three `RET 4`. Ghidra's enclosing candidate is `00835C70`, whose body ends at `00835E83`. |
| `00764D00` | `00764D1D` | `00764D00 MOV EAX,[ESP+4]` is a vtable target (`00CFD9EC+0Ch` region's neighbour `00D03630+0Ch`); the next byte after `00764D1D RET 4` starts `00764D20`, the scalar deleting destructor the same vtable's slot `0` names. |
| `0071C900` | `0071C922` | Same shape: `0071C900` is `00CFD9EC+0Ch`; `0071C925..0071C92F` are `INT3` and `0071C930` is the serializer at `00CFD9EC+4h`. |
| `009DAC20` | `009DAC25` | `MOV EAX,0xE08F70 ; RET`, the target of `00D215BC` = vtable `00D21598 + 24h`; `009DAC26..009DAC2F` are `INT3` and `009DAC30` is the next slot's body. |

`008356C0`, `008356D0`, `008356E0`, `008356F0`, `0071BE40`, `0071D560` and `0071D580` all have
Ghidra functions already (the four `008356xx` as `TRIV_body_*`), so they are not listed here.

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `unit_autopilot_pair` | `unit+61h`, `unit+0FC4h`, `unit+0FDCh`, `009DBF90`, `009DFFB0`, `009E0040`, `00834E90` | The one hop this packet could not close. The AI controller writes a desired throttle and steering into `[state]+8` at `+1C4h`/`+1C8h`/`+360h`/`+368h`; `00825F20` reads `unit+0FC4h`/`unit+0FDCh`. Find the base register the writer uses, the way `cc_unit_orders_apply` found `unit+980h` was `ring+148h`. Without it no reconstruction can drive a ship from an AI command. |
| `cruise_speed_setting` | `unit+73Ch`, `*(unit+73Ch)+24h`/`+28h`, `00822B70`, `00836920`, `luaMW_LandConvoySetSpeed` `00895850`, `luaMW_SquadronSetSpeed` `0089F780` | The commanded speed that would make the mission's `Cruise` ships move. Its enable is read at three sites and cleared at one; nothing writes `+24h` through a direct displacement. |
| `ship_ai_state_machine` | `00D21598`, `00D215C8`, `009F3D00`, `009F3DD0`, `009E1170`, `009E14C0`, `0071BE40` | The state class family whose `vtable[24h]` returns a command object. `009F3DD0` switches state whenever the director's current command changes, so the whole `moveto`/`follow`/`stop` set follows the same shape as `cruise`. |
| `gameunit_message_arms` | `00721A40` arms `5Ah`, `5Bh`, `5Dh`, `5Eh`, `5Fh`, `60h`, and `0071E7F0` | The rest of the receiver. `5Dh` is the clear this packet needs on the send side only; `0071E7F0` is the `msg+20h == 0` queue path. |
| `entity_command_arms` | `00816E30` `00816EA6..00817330` | The nine non-movement arms of the unit's command apply, including the `00E08F78` `attackmove` block that manufactures a `1E4h`-byte throwaway entity. |

## Uncertainties

1. The heading arm of `009E1170` is projected as "hold the latched heading" and the probe
   issues a zero rudder for it. `009E0040`'s body and the controller that turns a desired
   heading into a rudder are unread, so a `Cruise` latched off-heading is not modelled.
2. Four callees still enter the projection with unread bodies: `0071E2E0` (the echo),
   `0071E7F0` (the `msg+20h == 0` queue path), `00521E70` (a party or role test that calls the
   unread `00927F10`) and `[director+34h]->vtable[140h]`, whose host method is named
   `endpoint_subject_vtable140` for that reason. `009229F0`, `007AC9D0`, `00605070` and
   `009DA4E0` are reached only from gates this packet read and are unread one level down.
3. Which `MT_GAMEUNIT_SETCMD` fields the stream routines `0071C930`/`0071CA30` put on the
   wire, and in what quantisation, was not read.
4. `[00E0AF1C]`, the default routing flags `0077C2A0` uses for the `MT_COMMAND` hop, is still
   unread at run time, so "the local session applies the authored `Cruise` in the same frame"
   is not asserted here either. The second hop overrides it with `7`, which does include the
   local bit, so only the first hop depends on the global.
5. The category labels 0/1/2/3 of `docs/SCENE_COMMAND_TYPES.md` are a reading; what this
   packet proves is that `00835E90`, `0071E550`, `008358D0` and `0071E6C0` all branch on
   "category is 1 or 2" and never on any other value.
