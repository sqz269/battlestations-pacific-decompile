# The model handle `class+50h`: what writes it, and why the earlier search could not find out

Addresses: `0087C640` `BSP_DamageableClass_ConstructBase`, `0095F500` (vtable slot `+20h` base),
`0082FE30` / `007D3E60` / `00759120` / `0074DA10` (its overrides), `00718000`, `00964790`
`BSP_VehicleClass_GetOrCreate`, `00962981`, `00951671`, `00957419`.

Packet `cc7_model_handle_producer`. Ghidra **read-only**: no annotation, no tagging, no write lock.
**Exported / read only** — nothing here is reconstructible yet, so no C++ was written and no tests
were added. The scans cover `battlestationspacific.exe` only; this installation carries mod
artefacts, so a loaded module writing this field is not excluded.

## Result

**The producer was not found, and this document is mostly about why — because two of the earlier
sweep's negatives were vacuous.** What *is* settled:

* `class+50h` is **null-initialised at construction**, `0087C6A3` in `BSP_DamageableClass_ConstructBase`
  (`MOV dword ptr [ESI+50h], EBX` with `EBX` zeroed at `0087C66C`).
* No other write to it exists **in any of the forms and windows listed below**.
* The bind chain that consumes it is fully mapped, and the consumer runs *after* a base call that
  already expects the field to be populated.

## Two encodings that make a negative result meaningless

`docs/PART_DAMAGE_WIRING_PLAN.md` records that the earlier sweep scanned "`MOV [reg+50h], reg`" and
"every `CALL [reg+20h]` / `MOV reg,[reg+20h]; CALL reg` form" and found no candidate. Counted over
the whole `.text` of the shipped image:

| form | occurrences image-wide |
| --- | --- |
| `MOV [reg+50h], reg` — **disp32** encoding (`89 8x 50 00 00 00`) | **0** |
| `MOV [reg+50h], reg` — disp8 encoding (`89 4x 50`) | 244 |
| `MOV [reg+50h], imm32` — disp32 | **0** |
| `MOV [reg+50h], imm32` — disp8 | 43 |
| `CALL dword ptr [reg+20h]` — direct memory-indirect | **0** |
| `MOV r,[reg+20h]` … `CALL r` | 287 |

`50h` is 80, which fits a **signed** `disp8`, so MSVC never emits the `disp32` form for it; and this
compiler never emits the memory-indirect `CALL` for a virtual, always the load-then-call pair. A
scan for either of the two zero rows returns nothing **regardless of what the code does**, so
neither can support a conclusion. This is the single most useful thing in this packet: any future
offset search on this image should count both encodings and check the total is non-zero before
reading anything into an empty result.

## What the disp8 scan actually finds, and why all three are false positives

Three `MOV [reg+50h], reg` sites lie in the vehicle-class region `00950000`-`00970000`:

| site | enclosing function | verdict |
| --- | --- | --- |
| `00951671` | `BSP_EntityIdTable_Construct` | a different object |
| `00957419` | `CG_scalar_deleting_dtor_00957360` | a different object |
| `00962981` | `BSP_VehicleClass_ReadLuaFields` | **a per-entry record, not the class** |

`00962981` is the one worth the detail, because it is inside a function the earlier sweep "ruled out
by inspection" and it looked at first like the answer. Ghidra's listing shows it is the middle of a
long field-by-field copy of a ~`5Ch`-byte record out of a stack buffer:

```
009628B0..00962961  MOVSS [ESI+10h] .. [ESI+44h]   ; fourteen consecutive floats
00962966..0096298B  MOV   [ESI+48h] .. [ESI+54h]   ; four dwords, +50h among them
00962995            MOV   byte ptr [ESI+58h], AL
```

surrounded by `00B67190` / `00B66420` / `00B67700` Lua-iteration calls and a `JZ 009625A0` loop
back-edge. `ESI` is the element being appended, not the descriptor. **The earlier exclusion of
`00960230` stands**; it was right for a reason the sweep did not state.

A method note attached to that site: reading it first with `disasm-raw` produced a mis-synced stream
showing a `MOVUPS [ESI+40h]` overlapping a `MOVSS [ESI+44h]`, which is impossible and was the clue
to re-read it in Ghidra's stored listing. Raw resync cannot establish instruction boundaries, and
here it would have turned a false positive into a reported answer.

## The forms and windows that were searched

Negative in the vehicle-class region `00950000`-`00970000` except as noted:

| form | image-wide | in window |
| --- | --- | --- |
| `MOV [reg+50h], reg` disp8 | 244 | 3, all false positives above |
| `MOV [reg+50h], imm32` disp8 | 43 | 0 |
| `LEA reg, [reg+50h]` disp8 — the out-parameter form | 119 | **0** |
| `MOVSS [reg+50h], xmm` disp8 | 89 | 0 |
| any disp32 form of the above | 0 | 0 |

The `LEA` row matters: a loader taking `&class->model` as an out-parameter is the most natural way
to write a field without a visible store, and there is no such `LEA` anywhere in the class region.

**Not searched, and therefore not excluded**: a `memcpy` / `REP MOVSD` covering the field; a write
through a SIB-indexed address (`[reg+reg*n+50h]`, which every scan above skips by design); a write
on a different object later aliased to the class; and any window outside `00950000`-`00970000`
except the two spot checks below. The field must be written somewhere — the shipped game loads
models — so one of these, or a window not scanned, holds it.

## The bind chain, fully mapped

Descriptor vtable slot `+20h` (`docs/VEHICLE_CLASS_DESCRIPTORS.md:157` records it as
`0095F500`, "not read"; line 303 says it "has no established role"). It does:

```
0095F500  base          <- 0082FE51 (ship), 007D3E81 (plane), 0074DA2E (a fourth), and the vtables
0082FE30  ship override        also called directly from 0075913D, the mothership override
007D3E60  plane override
00759120  mothership override
0074DA10  a fourth override    prologue is unmistakable, but Ghidra has no function start here
```

Every override calls the base **first**, and the base reads the field at its second instruction
after that call:

```
0095F525  CALL 00879AD0
0095F52A  MOV EAX,[EBP+50h]         ; the model
0095F52D  MOV ESI,[EAX+68h]         ; model+64h/+68h/+6Ch is a std::vector
0095F533  CMP ESI,[EAX+8]
```

so the handle is expected to be live before any bind code runs. `0082FEA9` then passes the same
field as `this` to `00718000`, a node lookup shared with `BSP_GunClass_LoadFireNodeMuzzleOffsets`
— which is the independent confirmation that `+50h` is the model and not something else.

`0082FE30` holds slot `+20h` on **eight** class vtables, not the five the plan lists: `00D1ACE4`,
`00D1AD18`, `00D1AD58`, `00D1AD98`, `00D1ADDC`, `00D1AE18`, `00D1AE58`, `00D1AE98`.

## The invoker of slot `+20h` was not found either

287 `MOV r,[reg+20h]` … `CALL r` sites image-wide. The six inside `00940000`-`00980000` all
dispatch on other object types — `BSP_Unit_BroadcastControlledAudioFlag`,
`BSP_Aim_ResolveRayToWorldPoint`, `FUN_00957BD0`, `BSP_Unit_OnDestroyed`, `FUN_00967FF0`,
`BSP_WarningManager_Report` — and none is a class descriptor.

`BSP_LuaBinding_PrepareClass` (`008C8F70`) was the most promising name among the twenty callers of
`BSP_VehicleClass_GetOrCreate`; it contains **no** slot-`+20h` dispatch in either encoding.

The twenty callers of `00964790 BSP_VehicleClass_GetOrCreate` are the natural search set for whoever
continues, because the invoker must hold a descriptor:

```
BSP_SceneFile_Read 0046DF00, BSP_TrafficRecord_ApplyProperties 0049CF80,
the ten BSP_SceneUnit_Create* creators at 004F0520-004F10B0,
BSP_CommandBuilding_RespawnGarrison 006F3660, BSP_LandConvoy_AttachAndBuildRoster 00743450,
BSP_LandingShipClass_ReadLuaFields 0074C630, BSP_ShipClass_ReadLuaFields 00831840,
BSP_Shipyard_SceneAttach 00849A30, BSP_LuaBinding_AddAirBaseStock 00896A90,
BSP_LuaBinding_PrepareClass 008C8F70, BSP_LuaBinding_AIGetTargetWeight 00A38200
```

Only `008C8F70` was checked. **The other nineteen were not**, and that is the cheapest next step.

## Exclusion list, extended

Carried from `docs/SHIP_CLASS_BIND_MODEL_DATA.md`, plus this packet:

* `009633C0`, `00960230`, `00964020` — ruled out previously by inspection; `00960230`'s exclusion is
  now confirmed by byte evidence rather than inspection.
* `0095F500`, `0082FE30`, `007D3E60`, `00759120`, `0074DA10` — all **consumers**; each reads the
  field after the base has already dereferenced it.
* `008C8F70 BSP_LuaBinding_PrepareClass` — holds a descriptor but makes no slot-`+20h` dispatch.
* `0087C640 BSP_DamageableClass_ConstructBase` — writes the field, but writes **null**.

## What a host would need, if the producer is found

Unchanged from `docs/PART_DAMAGE_WIRING_PLAN.md`: the decoded element list has to reach a unit
before `part` can be non-zero, and `src/geom_mesh_resource.cpp` already decodes real `.MMOD`
payloads but is registered nowhere in the game build. This packet does not move that; it narrows
where the missing link is and removes two false leads from the search space.

# Continuation: nothing in this image invokes slot `+20h` (packet `cc7_model_handle_producer_2`)

Same contract and caveats as above: Ghidra read-only, no C++, no tests, scans over
`battlestationspacific.exe` only, mod artefacts not excluded.

## The scanner, checked against itself first

Before using another zero count as evidence, the dispatch scanner was validated on displacements
where the answer is known:

| slot | `CALL [reg+d]` | `MOV r,[reg+d]` … `CALL r` |
| --- | --- | --- |
| `+04h` | 23 | 1498 |
| `+08h` | 36 | 1219 |
| `+10h` | 0 | 982 |
| `+20h` | 0 | 287 |
| `+5Ch` | 0 | 1963 |

The memory-indirect form is found where it exists (`+04h`, `+08h`) and is genuinely absent at the
larger displacements — `+5Ch` is the entity class test, which this project has read dozens of call
sites for, and every one is the load-then-call pair. So the `+20h` zero is a codegen property, not a
decoder bug, and the 287 is the real population.

## The twenty callers: an exact negative, not a heuristic one

Body extents taken from Ghidra rather than assumed:

```
0046DF00-0046EF62  0049CF80-0049D389  004F0520-004F05E0  004F05F0-004F06B0
004F06C0-004F0780  004F0790-004F0850  004F0860-004F0920  004F0930-004F09F0
004F0A00-004F0AC0  004F0AD0-004F0BDF  004F0FB0-004F10A8  004F10B0-004F1165
006F3660-006F38DE  00743450-00743B6F  0074C630-0074CC46  00831840-0083468A
00849A30-00849F64  00896A90-00896CB3  008C8F70-008C9343  00A38200-00A38428
```

Every one of these bodies lies inside the `entry .. entry+3000h` span that was scanned, so the scan
covered them completely. Six dispatch sites fall in those spans and **all six lie outside the
bodies** — `006F53FF` is in `FUN_006F4D10`, `00A39898` in `FUN_00A39870`, `00A3AD27` in
`FUN_00A3AD10`, `00A3AE6A` in `FUN_00A3ADF0`, and so on.

**None of the twenty functions that obtain a descriptor from `BSP_VehicleClass_GetOrCreate`
dispatches through slot `+20h`.**

## No dispatch follows a class-descriptor load anywhere in the image

The other way to hold a descriptor is `unit+538h`. Over the whole `.text`:

```
MOV r32,[reg+538h] sites                                     686
slot +20h dispatch sites                                     287
dispatches within 60h bytes after a [reg+538h] load            0
```

## All five implementations are reached only through a vtable

| implementation | class | referenced from |
| --- | --- | --- |
| `0095F500` | base | the vtables, plus the four overrides chaining to it |
| `0082FE30` | ship family | eight vtables, plus `0075913D` inside the mothership override |
| `007D3E60` | plane family | vtables only |
| `00759120` | `MotherShip` | `00D1AEDC` only — MotherShip's vtable `+20h` |
| `0074DA10` | **`MLandVehicle`** | `00D1AA38` only — `00D1AA18 + 20h`. This names the fourth override |

`0074DA10` was the flagged unknown; it is `MLandVehicle`'s bind override, it has no non-vtable
reference, and the region `0074D000`-`0074E500` contains no `+50h` write in any scanned form.

## What this adds up to

Four independent negatives — no dispatch in the factory's callers, none after a `+538h` load, none
in the class region on a descriptor, and no non-vtable reference to any implementation — say that
**nothing in `battlestationspacific.exe` calls class-descriptor vtable slot `+20h`**. On that
reading the whole bind-model chain, including the 9.9 KB `0082FE30`, is **unreachable in the
shipped image**, which is a different and more useful answer than "the producer is hiding": it says
the model does not arrive through this path at all, and `class+50h` stays at the null that
`0087C6A3` writes.

**The limits of that claim, stated exactly.** A dispatch whose descriptor arrives as a function
**parameter** would show neither a `+538h` load nor a factory call nearby, and the `60h` window is a
heuristic. Nothing here excludes that case, and it is the one form that would overturn the reading.
Ruling it out means checking the remaining 281 dispatch sites' object types, which byte scanning
cannot do — it needs the call graph.

## Also negative this pass

`MOV [reg+50h], reg/imm/LEA/MOVSS`, disp8 and disp32, in `004F0000`-`004F2000` (the ten scene
creators), `0046D000`-`0046F000` (`BSP_SceneFile_Read`) and `0074D000`-`0074E500` (the LandVehicle
override): **zero in every form and window**.

## The next step, if this is worth continuing

Two that byte scanning cannot reach and a third that it can:

1. **Who, if anyone, calls slot `+20h` with a descriptor passed in as a parameter.** Needs the call
   graph, not a scan.
2. **Whether the model arrives without this chain** — the scene database's deferred reference
   resolution is the standing candidate and has never been read.
3. **A `memcpy` / `REP MOVSD` covering `class+50h`**, still unscanned. The destination would be a
   `LEA` of a *lower* offset than `50h`, so it needs a different scan shape than the ones here.

# The deferred-reference path carries no model (packet `cc7_scene_deferred_model_refs`)

Addresses: `0046AAB0` `BSP_SceneDatabase_ResolveDeferredReferences` (body `0046AAB0`-`0046AC38`,
107 instructions, read in full), `0046A9F0`, `00925A90`, `00438E10`, `0077D600`.
Globals `00E19A70`, `00E188A8`, `00E18560`, `00F87574`.

Same contract and caveats: Ghidra read-only, no C++, no tests, `battlestationspacific.exe` only,
mod artefacts not excluded.

## Answer: no. It resolves names into commands, and touches nothing resource-shaped

The body is short enough to read entirely, and every instruction is accounted for:

```
0046AAB3  walk the pending list at sceneDb+150h                       ; node+4h is the next link
0046AAD0  rec = [node+8h] ; EDI = [rec] (the unit) ; EBP = [rec+8h]   ; the target's name,
0046AAE2      defaulting to the empty string 00E18560 when null
0046AAE7  scan the global scene-entity list [00E19A70]
0046AAFA      name = entry->vtable[4h]()
0046AB03      00438E10 BSP_CString_CompareInsensitive(EBP, name)      ; advance on mismatch
0046AB23  on a match, ESI = the entity, and one of two arms:
0046AB2F   A: [rec+0Ch] != 0 -> second name [rec+10h] (same empty default)
0046AB41      00925A90 BSP_EntityRegistry_FindEntityByName(world[+19CCh], name)
0046AB55      record = { 00F87574..7Ch (the read-only zero vector), byte 1,
0046AB84                 the found entity, word [found+174h] }
0046AB9B   B: else require ESI->vtable[8h]() false, refresh the unit's pose when
0046ABA8      unit+C8h is clear (00414DB0), and build the record from the unit's own
0046ABB7      world position unit+FCh / +100h / +104h
0046AC0B  0077D600 BSP_Entity_IssueCommand(this = EDI, ESI, &record, 1)
0046AC34  tail-jump to 0046A9F0 BSP_SceneDatabase_ClearPendingReferences
```

`unit+C8h`, `00414DB0` and `unit+FCh/+100h/+104h` are `kPoseValidByte`, the pose refresh and the
world-position triple that `include/bsp/plane_flight.hpp` already names, which is what fixes `EDI`
as a unit and the record as a **world-position command target**. In arm A the position is the
read-only **zero** vector, so the target is the entity itself rather than a point.

**There is no model handle, no `.MMOD`, no resource call, and no write to any `+50h`.** A scan of
`0046A000`-`0046B000` for `MOV [reg+50h], reg/imm`, `LEA reg,[reg+50h]` and `MOVSS [reg+50h], xmm`
in both encodings returns zero in every form — and, per the rule this document opens with, those
forms are not vacuous here: their image-wide totals are 244, 43, 119 and 89.

## What it does resolve, which is worth having anyway

This is a **second native producer of unit commands**, independent of the Lua bindings, and it runs
on the instantiate pass of `0046DF00 BSP_SceneFile_Read`. Each pending record is
`{ unit, target-name, flag, secondary-name }` and becomes one `BSP_Entity_IssueCommand` call. So
scene-authored orders reach a unit by **name**, resolved late against the scene-entity list, with
`BSP_EntityRegistry_FindEntityByName` as the second lookup — relevant to the order chain in
`docs/PILOT_ORDER_BINDINGS.md`, which lists this function as a native command caller without saying
what it resolves.

## Five negatives now stand

1. No write to `class+50h` in any scanned form or window beyond the null at `0087C6A3`.
2. No slot-`+20h` dispatch in any of the twenty callers of `BSP_VehicleClass_GetOrCreate`.
3. No slot-`+20h` dispatch within `60h` bytes of a `unit+538h` load anywhere in `.text`.
4. No non-vtable reference to any of the five slot-`+20h` implementations.
5. **The deferred-reference path carries no model.**

The standing limit is unchanged and is still the thing that would overturn (2)-(4): a dispatch whose
descriptor arrives as a **function parameter**, which byte scanning cannot see.

## Next

The `memcpy` / `REP MOVSD` scan named above is now the only byte-reachable angle left on
`class+50h`. Beyond it the question needs the call graph, or a different hypothesis entirely — that
the model reaches the **unit** rather than the class, in which case `class+50h` is a red herring and
the search should start from `docs/GEOM_MESH_RESOURCE.md`'s decoder and
`docs/GAME_RESOURCE_PARSER_REGISTRATION.md`'s registration row instead.
