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
