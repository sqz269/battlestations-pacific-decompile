# Every arm of the unit's two session-message switches (packet `cc2_unit_message_arms`)

Addresses: 00821E80 0095ABE0 00878350 0075B430 0080F710 008141A0 00957450 0080DC10 00815010
0093AA90 00939FD0 0064A270 0074F440 0074E830 0074E860 00819A20 00953F60 00954000 009540D0
00954170 007BA2E0 006E0B40, and read-only 00521E30 0081F8B0 00814520 00814560 0080E440 0092BF30
00959C20 00982C50 00939F90 00939FA0 0093A470 0093A4F0 0093C120 0093C210 00983780 009832F0
006EAFE0 004404F0 0041E870 0041DD20, plus the six undefined constructors 0080F960 009532E0
008144A0 0074EAB0 0080FDD0 009D6650.

Worker `agent/cc2-unit-message-arms`, 2026-09-12 UTC. Ghidra was read-only for this packet: no
renames, comments, prototypes, function creation or saves. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Descriptive names are hypotheses, not recovered symbols,
**except the `MT_*` kind names, which are string literals in the image**. No run-time evidence:
the `bsp_game.exe` harness does not load a mission, so no arm here was observed on a frame
(`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6).

## Headline

The two switches are fully decoded and every arm is read. **34 kinds are armed** across the pair,
and **30 of them have a message-construction site inside a defined function**, so a local session
has to be able to build almost the whole table. Ten of the 30 are driven from a Lua binding, which
is what a mission script reaches.

The decisive find is the **kind name table at `00E0AB68`**: one `char*` per kind byte, holding the
game's own `MT_*` literal. That turns the whole table from hypothesis into recovered naming and
cross-checks the index arithmetic three ways against work already merged - `58h` is `MT_COMMAND`
(`docs/ENTITY_ORDER_MESSAGE.md`), `8Ch` is `MT_SHIP_SYNC` (`docs/UNIT_STATE_MESSAGE.md`) and `99h`
is `MT_SHIP_EXPLODEONEPART` (`docs/UNIT_PARTS.md`). Seven arm callees that earlier packets had
already named independently land on the matching literal: `0081F8B0
BSP_UnitInstance_SetTorpedoStock` on `96h MT_SHIP_SET_TORPEDOSTOCK`, `0092BF30
BSP_UnitController_AddHullTorque` on `93h MT_SHIP_ADDTORQUE`, `00982C50
BSP_WarningManager_FireFailure` on `7Dh MT_VEHICLE_SET_INFERIORFAILURE`, and so on.

The chain is **three levels deep, not two**. `0095ABE0`'s default does not return false: it calls
`00878350`, which answers `4Bh`, `4Ch`, `56h MT_DAMAGEDGFXLEVEL` and `D2h`, and only *its* default
returns 0.

## The message base, from its constructor

`0075B430 BSP_SessionMessage_ConstructBase`, `__thiscall(msg, kind)`, `RET 4`, body
`0075B430-0075B47F`. Coverage: complete. This is the producer, so it settles the base field
meanings (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 4).

| Offset | Written at | Value |
| --- | --- | --- |
| `+0h` | `0075B44B` | vptr `00D02C68` |
| `+4h` | `0075B436` | `3` |
| `+8h` | `0075B43D` | `0` |
| `+Ch` | `0075B444` | `0`; the send path stamps the tick here at `0076E5C6` |
| `+10h` | `0075B451` | the kind byte |
| `+14h` | `0075B470` / `0075B476` | the **sender player object** |

The sender is not a parameter. The constructor reads `[00E188A8]+18ECh` as an index, checks it
against `0..7`, and takes the pointer from the eight-entry array at `[00E188A8]+18CCh`, storing 0
when the index is out of range. That is what makes `0080F710` meaningful:

`0080F710`, `int __fastcall(msg)`, body `0080F710-0080F720`: `msg+14h ? *(msg+14h + 20h) : 8`.
Two arms use it as the acting player's control slot, and **8 is the "no player" sentinel** - the
`MT_ROLEOWNER` release path at `009540D0`/`00954170` passes that same literal 8 when it clears a
station.

## `00821E80 BSP_UnitInstance_HandleMessage`

`undefined1 __thiscall(unit /*ECX*/, msg)`, `RET 4`, body `00821E80-00822393`, vtable slot `164h`.
Coverage: **complete**, all 27 targets read. The listing is 411 consecutive instructions from
`00821E80` to `00822391` with no gap above 10 bytes.

`MOVZX EAX,[msg+10h]` / `ADD EAX,-4Bh` / `CMP EAX,55h` / `JA 0082237B`, then
`MOVZX EAX,[EAX+00822400]` / `JMP [EAX*4 + 00822394]`. The byte table holds 86 entries for kinds
`4Bh..A0h`; the target table holds 27 dwords. `ESI` is the message and `EDI` the unit throughout.

Three of the 27 targets do no unit work, and the difference between them matters:

| Target | Address | Behaviour |
| --- | --- | --- |
| `00h` | `00821EEA` | calls the base, then `MOV AL,1` at `00821EF1` - the base's answer is **discarded** |
| `01h` | `00821ED5` | the shared return-true epilogue; the base is never reached |
| `1Ah` | `0082237B` | the default; returns whatever the base returned |

### The kind table

`producer` is the function holding the `PUSH <kind>` that feeds `0075B430`. `Lua` marks a producer
that reads its arguments through the Lua object glue.

| Kind | Name (image literal) | Arm | Fields read | Effect | Producer | Coverage |
| --- | --- | --- | --- | --- | --- | --- |
| `4Bh` | `MT_ROLEOWNER` | `00821EEA` | - | base call, forced true | `008A6060` (Lua), `0077C470` (HUD), `007796F0`, `00A31C20`, `00927D20` | complete |
| `4Ch` | `MT_ROLEAVAILABLE` | `00821ED5` | - | nothing; swallowed | `008AB850 BSP_LuaBinding_SetRoleAvailable` | complete |
| `4Dh`-`69h` | `MT_MULTISELECTUNIT` .. `MT_GUN_CLEAR_DESTROYEDFAILURE` | `0082237B` | - | default | elsewhere | table complete |
| `6Ah` | `MT_SHIP_SET_EXPLOSIONFAILURE` | `00822120` | the message | `unit->vtable[220h](msg)` | `008132C0` (Lua) | arm complete |
| `6Bh` | `MT_SHIP_SET_STEERINGJAMFAILURE` | `00822120` | the message | `unit->vtable[220h](msg)` | `008132C0` (Lua) | arm complete |
| `6Ch` | `MT_SHIP_SET_ENGINEJAMFAILURE` | `00822120` | the message | `unit->vtable[220h](msg)` | **unresolved**, kind computed | arm complete |
| `6Dh` | `MT_SHIP_CLEAR_STEERINGJAMFAILURE` | `00822140` | the message | `unit->vtable[224h](msg)` | `00813660` | arm complete |
| `6Eh` | `MT_SHIP_CLEAR_ENGINEJAMFAILURE` | `00822140` | the message | `unit->vtable[224h](msg)` | `00813660` | arm complete |
| `6Fh` | `MT_SHIP_REPAIR_SETTINGS` | `00822160` | `msg+20h..+34h` | six dwords into `unit+1134h` | `0080FEC0` | complete |
| `70h` | `MT_SHIP_KAMIKAZE_DETONATE` | `0082217D` | word `+1Ch`, `&+20h` | resolve, then `00819A20` | `0080FF30` | arm complete |
| `71h`-`79h` | `MT_PARATROOPER_LANDINGPOS` .. `MT_VEHICLE_GUN_CONTROL` | `0082237B` | - | default; the base arms `79h` | elsewhere | table complete |
| `7Ah` | `MT_VEHICLE_SHIPYARD_LAUNCH` | `0082226F` | byte `+20h` | `unit+1130h = byte ? 5 : 0` | `00813830`, `009CFBA4` | complete |
| `7Bh` | `MT_VEHICLE_UNIT_LAUNCH` | `00821F05` | `+14h`, float `+20h` | `unit->vtable[208h](slot, float)` | `00891D50 BSP_LuaBinding_ShipUseCatapult`, `00651800` (HUD) | arm complete |
| `7Ch` | `MT_VEHICLE_ADD_LAUNCHED` | `00821F37` | word `+20h` | resolve, then `00957450` | `006EB9C0` | complete |
| `7Dh`-`8Dh` | `MT_VEHICLE_SET_INFERIORFAILURE` .. `MT_SHIP_GUNS_SYNC` | `0082237B` | - | default; the base arms `7Dh`-`80h`, and `8Ch` is slot `18Ch` | elsewhere | table complete |
| `8Eh` | `MT_SHIP_HELMSMAN_CONTROL` | `00821EBE` | `+14h`, `+Ch`, `+1Ch`, `+20h`, byte `+38h` | station-gated order backfill | `00816A40 BSP_UnitInstance_IssueOrder` | complete |
| `8Fh` | `MT_SHIP_AVOIDSIDE` | `00822294` | word `+1Ch`, `+20h` | resolve, narrow to class 6, `unit+740h` `vtable[28h]` | `009D66B0` | arm complete |
| `90h` | `MT_SHIP_LEAK` | `008221A7` | dword `+1Ch`, `&+20h` | `0074F440(unit+10D4h, amount, point)` | `0080FF80`, `00826F10 BSP_ShipEntity_ApplyHitRecord` | complete |
| `91h` | `MT_SHIP_LEAKCHEATWATER` | `008221ED` | - | `0074E830(unit+10D4h)` clears all flooding | `008144A0`, a virtual at slot `23Ch` | complete |
| `92h` | `MT_SHIP_SINK` | `0082220D` | dword `+1Ch`, `&+20h` | `0074E860(unit+10D4h, count, src)` | `0074EC50`, from `00824B60 BSP_UnitInstance_OnWrecked` | arm complete |
| `93h` | `MT_SHIP_ADDTORQUE` | `00822235` | `+1Ch`, `+20h`, `+24h` | `0092BF30` with `ECX = [unit+1018h]` | `0080FFD0` | complete |
| `94h` | `MT_SHIP_STARTLANDING` | `00821F61` | - | `unit->vtable[238h]()` | `0064A820`, from `009F3240 BSP_ShipAi_AttackMoveApproachSubStateStep` | arm complete |
| `95h` | `MT_SHIP_LANDINGSHIPSLAUNCHED` | `00821F80` | float `+1Ch` | `unit+1124h = float` | `008206F0` | complete |
| `96h` | `MT_SHIP_SET_TORPEDOSTOCK` | `00821F9E` | dword `+20h` | `0081F8B0 BSP_UnitInstance_SetTorpedoStock` | `0080FE10` | complete |
| `97h` | `MT_ENTITY_RELOCATE` | `0082237B` | - | default, inside the armed run | elsewhere | table complete |
| `98h` | `MT_SHIP_WRECKED` | `00821FBC` | - | `00814520 BSP_UnitInstance_OnBreakupMessage` | **unresolved**, kind computed | arm complete |
| `99h` | `MT_SHIP_EXPLODEONEPART` | `00821FF0` | `+20h` | `0080E440 DetachPart` (`docs/UNIT_PARTS.md`) | `0092CED0` | complete |
| `9Ah` | `MT_SHIP_EXPLODETOPARTS` | `00821FD6` | - | `00814560` (`docs/UNIT_DEATH_MESSAGE_AND_SINK.md`) | `00827A90 BSP_ShipInstance_OnHealthChanged` | complete |
| `9Bh` | `MT_SHIP_SECTIONFAILUREEFX` | `00822010` | `+20h`, `+24h`, byte `+28h` | `0093AA90(unit+A20h, ...)` creates or stops a point effect | `0093C520 BSP_RepairTask_RepairFailures` | complete |
| `9Ch` | `MT_SHIP_ERASEWRECK` | `0082237B` | - | default | elsewhere | table complete |
| `9Dh` | `MT_SHIP_TORPEDOGENERATE_HACK` | `008222CF` | `+1Ch`, float `+20h`, `&+24h` | spawns class `3Fh` | `009D6650`, **no reference resolves into it** | arm complete |
| `9Eh` | `MT_SHIPREPAIR_ADDDAMAGE` | `0082203D` | float `+1Ch`, byte `+20h`, dword `+24h` | the fire/leak channel switch | `0080FA50`, `0088E320 BSP_LuaBinding_SetFireDamage`, `0088E790 BSP_LuaBinding_SetWaterDamage` | complete |
| `9Fh` | `MT_SHIPREPAIR_BODYREPAIR` | `008220D7` | byte `+1Ch` | `00939FD0` writes `repairTask+45h` | `008AD330 BSP_LuaBinding_RepairEnable` | complete |
| `A0h` | `MT_SHIPREPAIR_REPAIRPRIORITY` | `008220FC` | dword `+1Ch` | `0064A270` writes `repairTask+24h` | `008AD6F0 BSP_LuaBinding_SetRepairPriority`, `0064A770` (HUD) | complete |

### Arms worth spelling out

**`6Fh` is the only inverted arm.** `00822161 MOV ECX,ESI` makes the *message* the `this` pointer
and passes the unit as the argument: `0080DC10(msg, unit)` copies six dwords from `msg+20h..+34h`
into `unit+1134h..+1148h`, the array `include/bsp/unit_fire_flooding.hpp` already calls
`kUnitRepairLevelArrayOffset`.

**`8Eh` is lag-compensated and silently gated.** `00821EC5 CMP [EDI+1B0h],EAX` / `JNZ 00821ED5`
applies the order only when `unit+1B0h` equals the sender's control slot; a mismatch still returns
true, so a helm message from the wrong station is dropped without a trace. `008141A0` then
back-dates it: `age = [00F876B0] - msg+0Ch`, clamped to `[00E0B51C]`, and
`BSP_UnitOrderRing_BackfillOrder(msg+1Ch, msg+20h, byte msg+38h, age)`. `msg+0Ch` is the tick the
send path stamped at `0076E5C6` (`docs/SESSION_MESSAGE_DISPATCH.md`).

**`90h` scales by ten.** `FILD [msg+1Ch]` with the `if (signed < 0) add 2^32` fixup at `008221B5`
against the `2^32` float at `00CE3978` is an unsigned widen, then `FMUL` by the double `10.0` at
`00CE3DC0`. `0074F440`, `RET 8`, refreshes the owner pose at `leakManager+10h` when its `+C8h`
byte is 0, transforms the message's point through the matrix at `pose+0CCh` and forwards
`(amount, transformed)` to `0074F090`.

**`9Dh` spawns through the class registry.** `007BA2E0` builds `{cos a, 0, sin a}` from
`a = [00CE3830] - heading` wrapped by `[00CE3828]`; `006EAFE0` fetches class `3Fh`;
`class->vtable[20h](0, msg+1Ch, &msg+24h, &direction, 1, 0, 0)` spawns. The result points at the
object's `+310h` subobject, so `00822317 LEA ESI,[EAX-310h]` backs up to the base. Then
`006E0B40(obj, 1)` makes it visible, `(obj+310h)->vtable[34h](00F87574, 00F87574)` zeroes two
vectors from the read-only zero vector, and `obj+32Ch = unit+180h` carries the owner across.

**`9Eh` routes by channel then by add-versus-set.** `msg+24h == 0` is fire and `== 1` is leak;
anything above 1 falls into the plain epilogue at `008220C2` and does nothing. Within a channel,
`byte msg+20h != 0` picks the accumulating setter. See the correction below: the channel-to-offset
mapping in `include/bsp/unit_fire_flooding.hpp` is inverted.

| `msg+24h` | `byte msg+20h` | Call | Field |
| --- | --- | --- | --- |
| 0 | non-zero | `0093A470 AddFireSeconds` | `repairTask+38h` |
| 0 | zero | `00939F90 SetFireSeconds` | `repairTask+38h` |
| 1 | non-zero | `0093A4F0 AddWaterSeconds` | `repairTask+34h` |
| 1 | zero | `00939FA0 SetWaterSeconds` | `repairTask+34h` |
| `> 1` | - | none | - |

## `0095ABE0 BSP_Unit_HandleMessage`

`undefined1 __thiscall(unit /*ECX*/, msg)`, `RET 4`, body `0095ABE0-0095AE1F`. Coverage:
**complete**, all 8 targets read; 177 consecutive instructions, no gap.

`ADD ECX,-4Bh` / `CMP ECX,35h` / `JA 0095AE06`, byte table at `0095AE40` (54 entries, kinds
`4Bh..9Eh`), 8 targets at `0095AE20`. `ESI` is the unit and `EAX` the message.

| Kind | Name | Arm | Effect | Producer | Coverage |
| --- | --- | --- | --- | --- | --- |
| `4Bh` | `MT_ROLEOWNER` | `0095AC1D` | the crew-role assignment, below | `008A6060` (Lua) | complete |
| `4Ch` | `MT_ROLEAVAILABLE` | `0095ACD1` | nothing | `008AB850` | complete |
| `79h` | `MT_VEHICLE_GUN_CONTROL` | `0095ACE5` | `00959C20 BSP_Unit_ApplyGunAimMessage` (`docs/GUN_BOT_TICKS.md`) | `00954A10` | arm complete |
| `7Dh` | `MT_VEHICLE_SET_INFERIORFAILURE` | `0095AD01` | `unit+720h = 1`, then a class-gated warning | `00953DC4` | complete |
| `7Eh` | `MT_VEHICLE_CLEAR_INFERIORFAILURE` | `0095AD7C` | `unit+720h = 0`, `unit+728h = 0.0f` | `00953EC4` | complete |
| `7Fh` | `MT_VEHICLE_SET_PARTY` | `0095ADA2` | `vtable[2Ch](msg+1Ch, this+58h, &out)` | `008A8930 BSP_LuaBinding_SetParty` | arm complete |
| `80h` | `MT_VEHICLE_SET_RACE` | `0095ADD4` | `vtable[2Ch](this+54h, msg+1Ch, &out)` | `008A8720` (Lua) | arm complete |
| everything else | - | `0095AE06` | `00878350(this, msg)` | - | table complete |

**`7Fh` and `80h` together fix `vtable[2Ch]`.** `docs/WORKER_VERIFICATION_CHECKLIST.md` rule 3 in
action: read alone, either arm looks like a one-argument setter. `7Fh` passes the new value first
and the existing `this+58h` second; `80h` passes the existing `this+54h` first and the new value
second. The virtual is therefore `vtable[2Ch](party, race, out)`, and `this+54h` is the party
while `this+58h` is the race.

**`7Dh` sets the latch before it checks the class.** `0095AD0A` stores `unit+720h = 1`
unconditionally. Only then does `this->vtable[5Ch](45h)` or, failing that, `this->vtable[5Ch](46h)`
gate the warning: `0041E870` builds a pooled string from the literal `"InferiorFailure"` at
`00CF0B74`, `00982C50 BSP_WarningManager_FireFailure([00F8A0C4], this, &name, float this+728h)`
fires it, `0041DD20` frees the string.

### `4Bh MT_ROLEOWNER`, the crew-role assignment

The arm at `0095AC1D` is the largest in either switch and is what puts a player at a station.

- `byte msg+20h != 0` takes the single-device path `this->vtable[144h](msg+28h)` and returns.
- otherwise bit 0 of `msg+24h` together with `msg+2Ch == 1` sets `unit+6E8h` (`0095AC53`);
- then `msg+24h` is a device-group mask and the first matching test wins:

| Test | Call | What it walks |
| --- | --- | --- |
| `mask & 4` or `mask == 8` | `00953F60` | the device list at `unit+48h` (next at `+44h`), descriptor kind `device+3F4h+80h` in `{1,5,6}` |
| `mask & 10h` | `00954000` | the same list, kind in `{2,3,4,6}` |
| `mask & 20h` | `009540D0` | the global collection `00E0A520` |
| `mask & 80h` | `00954170` | the global collection `00E0A528` |
| none | - | nothing |

`msg+28h` is the control slot being assigned and `msg+2Ch` is 1 for take and 0 for release. The
two collection walkers make the release explicit: on a take they call
`element->vtable[154h](0, msg+28h)`, and on a release `element->vtable[154h](0, 8)` followed by
`element->vtable[1E8h](0)`. That literal 8 is the same "no player" sentinel `0080F710` returns,
which is what fixes the sentinel rather than inferring it.

The two device walkers filter on a descriptor kind word whose classes were not read, so they are
named by their sets (`DeviceKindSet156`, `DeviceKindSet2346`) and not by a device name.

## `00878350`, the third level

`undefined4 __thiscall(entity, msg)`, body `00878350-0087839A`. Coverage: complete.

| Kind | Effect |
| --- | --- |
| `4Bh`, `4Ch` | return 1, no work |
| `56h MT_DAMAGEDGFXLEVEL` | `entity->vtable[1B4h](msg+20h)`, return 1 |
| `D2h` | `00877CD0(msg+1Ch)`, return 1 |
| default | **return 0** - the only false in the chain |

## Which kinds a single-player mission needs

Of the 34 armed kinds, 30 have a construction site inside a defined function. Nothing in the armed
set is network-only in the sense of having no local construction site at all; the four gaps are
tooling gaps, not evidence of a wire-only kind.

- **Lua-driven** (a mission script reaches these): `4Bh`, `4Ch`, `6Ah`, `6Bh`, `7Bh`, `7Fh`, `80h`,
  `9Eh`, `9Fh`, `A0h`.
- **HUD or interface**: `4Bh`, `7Bh`, `94h`, `A0h`.
- **Script or AI**: `8Eh`, `8Fh`, `9Bh`, plus `94h` from the attack-move approach step.
- **Engine internal**: `6Dh`, `6Eh`, `6Fh`, `70h`, `79h`, `7Ah`, `7Ch`, `7Dh`, `7Eh`, `90h`, `92h`,
  `93h`, `95h`, `96h`, `99h`, `9Ah`.
- **Producer in code Ghidra has no function for**: `91h` (`008144A0`, a virtual at slot `23Ch`) and
  `9Dh` (`009D6650`, into which no reference resolves).
- **No construction site found**: `6Ch` and `98h`, whose kind bytes are computed rather than pushed.

Method: `0075B430` stores its one stack argument at `msg+10h`, so scanning `.text` for the last
`PUSH imm` before a `CALL 0075B430` names the kind at every direct construction site. 426 sites,
25 of which compute the kind. `bsp.py ghidra xrefs` under-reports these immediates, so none of
this came from Ghidra references.

## Routines Ghidra has no function for

Six construction sites lie past the end of the nearest defined function. Each routine was bounded
from the disk bytes; five open with the same `PUSH ESI` / `PUSH <kind>` / `MOV ESI,ECX` /
`CALL 0075B430` shape, which is the per-kind derived-message constructor.

| Address | End | Name | Kind |
| --- | --- | --- | --- |
| `0080F960` | `0080F98F` | `BSP_UnitMessage_ShipyardLaunch_Construct` | `7Ah` |
| `009532E0` | `00953313` | `BSP_UnitMessage_SetInferiorFailure_Construct` | `7Dh` |
| `008144A0` | `00814514` | `BSP_UnitInstance_SendLeakCheatWater` | `91h` |
| `0074EAB0` | `0074EAFA` | `BSP_UnitMessage_ShipSink_Construct` | `92h` |
| `0080FDD0` | `0080FE03` | `BSP_UnitMessage_LandingShipsLaunched_Construct` | `95h` |
| `009D6650` | `009D66AB` | `BSP_UnitMessage_TorpedoGenerateHack_Construct` | `9Dh` |

`008144A0` is the one with a reference: its address appears in nine `.rdata` vtables (`00CF92EC`,
`00CFA9B4`, `00CFB974`, `00CFC60C`, `00CFFC6C`, `00D0186C`, `00D098B4`, `00D0C1BC`, `00D0C884`).
Taking `00821E80` at `00CF9214` as slot `164h` puts the vtable base at `00CF90B0` and `00CF92EC` at
slot `23Ch`; `00CFA9B4` against base `00CFA778` gives the same `23Ch`. The other five have no
`CALL rel32` and no aligned dword pointer resolving into them anywhere in the image. That is not
proof they are unreachable - a linear byte scan resyncs from arbitrary offsets and cannot establish
provenance - but their triggers are unresolved.

## Corrections

| Document | Was | Is | Evidence |
| --- | --- | --- | --- |
| `docs/SESSION_MESSAGE_DISPATCH.md` | `0095ABE0` "returns false for the rest (`0095AE06`)" | the kind list is right, but `0095AE06` calls `00878350(this, msg)` and returns *its* result, which is 1 for `4Bh`, `4Ch`, `56h` and `D2h`. A caller of slot `164h` sees true for three more kinds than the two switches explain. | `0095AE06 PUSH EAX` / `0095AE07 MOV ECX,ESI` / `0095AE09 CALL 0x00878350`, then the epilogue `0095AE0E-0095AE1D` with no `MOV AL`. `00878350`'s body is a four-case switch on `[msg+10h]`. The ledger record for `0095ABE0` already read "the default to the base handler `00878350`", so the dispatch doc contradicted a record already in `config/names/00950000.jsonl`. |
| `docs/SESSION_MESSAGE_DISPATCH.md` | "every other index selects target `1Ah` ... the base call" | three targets differ in how they treat the base. `00h` (`4Bh`) calls it and forces `AL = 1`; `01h` (`4Ch`) never reaches it; only `1Ah` returns its value. `4Ch` is swallowed by the unit, not handled by it. | `00821EEA PUSH ESI / CALL 0095ABE0 / 00821EF1 MOV AL,1` against `0082237B PUSH ESI / CALL 0095ABE0 / 00822381 MOV ECX,[ESP+20h]` with no `MOV AL`. |
| `docs/SESSION_MESSAGE_DISPATCH.md` | "the `4Bh` arm at `00821EEB`" | the arm entry is `00821EEA`; `00821EEB` is the `CALL` inside it. | target dword index `00h` at `00822394` is `00821EEA`, whose first instruction is `PUSH ESI`. |
| `docs/SESSION_MESSAGE_DISPATCH.md` | "\| `4Eh` destroy \| ... \|" | the route and effect are right, the label is not: `4Eh` is `MT_DEADMEAT` and `4Fh` is `MT_DESTROY`. | the name table gives index `4Eh` -> `00D02710` "MT_DEADMEAT" and `4Fh` -> `00D02704` "MT_DESTROY". The same table's `58h` is "MT_COMMAND" and `8Ch` "MT_SHIP_SYNC", matching two merged docs. |
| `include/bsp/unit_fire_flooding.hpp` | `kRepairTaskOffFireSeconds = 0x34`, `kRepairTaskOffWaterSeconds = 0x38`, `kRepairTaskOffWaterExpiredSlot = 0x3C`, `kRepairTaskOffFireExpiredSlot = 0x40`, `kSettingsOffFirePriorityDivisor = 0x3C8`, `kSettingsOffWaterPriorityDivisor = 0x3CC` | every fire/water label in that group is on the wrong offset; the routine citations are right. Fire is `+38h` with its expired slot at `+3Ch` and its divisor at `settings+3CCh`, ticked by `0093C210`. Leak/water is `+34h` with its slot at `+40h` and its divisor at `settings+3C8h`, ticked by `0093C120`. The six constants should swap their fire and water names in place. | `0093A470`, which reads and writes `+38h` (`0093A489 FLD [ESI+38h]`), calls `00983780` at `0093A484`, and `00983780`'s body carries the string immediate `"fire"`. `0093A4F0`, which reads and writes `+34h` (`0093A518`, `0093A52D`) and also writes `+40h` (`0093A530`), calls `009832F0` at `0093A513`, whose body carries `"leak"`. The one-line setters agree: `00939F90` writes `+38h` and is already `BSP_RepairTask_SetFireSeconds`; `00939FA0` writes `+34h` and is `BSP_RepairTask_SetWaterSeconds`. The tick routines group the same way. |

## Coverage

| Routine | Coverage |
| --- | --- |
| `00821E80` | complete, `00821E80-00822393`, all 27 targets |
| `0095ABE0` | complete, `0095ABE0-0095AE1F`, all 8 targets |
| `00878350` | complete |
| `0075B430` | complete |
| `0080F710`, `008141A0`, `00957450`, `0080DC10`, `00815010` | complete |
| `00939FD0`, `0064A270`, `0074E830`, `007BA2E0`, `006E0B40` | complete |
| `0093AA90` | complete for both branches; the middle argument's use not isolated |
| `0074F440` | complete, `RET 8` from `0074F484`; `0074F090` unread |
| `00953F60`, `00954000`, `009540D0`, `00954170` | complete for the walks and filters; `00729F70` and `0072D5B0` unread |
| `0074E860` | partial: the strided copy loop `0074E860-0074E8B0`; the tail unread |
| `00819A20` | partial: the entry gate `00819A20-00819A7F`; the effect body unread |
| vtable slots `144h`, `208h`, `220h`, `224h`, `238h`, `2Ch`, `1B4h`, `28h` | owners unresolved to functions |

## Open questions

- `6Ch` and `98h` have no `PUSH <kind>` site. Their producers build the kind in a register, so
  finding them needs a data-flow pass over the 25 unattributable `0075B430` sites.
- Slots `220h` and `224h` are the whole of the five SET/CLEAR failure kinds and neither owner was
  resolved. Reading the six shared unit vtables at those displacements would settle five rows at
  once.
- `00819A20` (kamikaze) and `0074E860` (leak zone load) are covered only at their heads, and both
  are on the armed path.
- The device kind words `{1,5,6}` and `{2,3,4,6}` at `device+3F4h+80h` are unidentified, so the two
  `MT_ROLEOWNER` device walkers cannot be named by device class.
- The five unreferenced constructors: whether they are dead code or reached by a mechanism the byte
  scan cannot see.
- No run-time evidence for anything here.
