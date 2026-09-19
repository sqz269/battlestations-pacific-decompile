# The plane squadron entity (`PlaneSquadronGen`, class id `18h`)

Addresses: 007f2c60, 007ef9d0, 007efa70, 007efb00, 007f1d90, 007f2bd0, 007f31a0, 007f3500,
007f3820, 007f3970, 007f3a60, 007f4580, 004f0ad0, 00928860, 00d087c0

Packet `cc2_squadron`, read-only analysis. Every descriptive name below is a hypothesis, not a
recovered symbol. Reconstruction: `include/bsp/plane_squadron.hpp`, `src/plane_squadron.cpp`.
Report: `reports/plane_squadron.json`.

Contracts owned elsewhere, cited and not re-derived: the wave-3 tick `007F3BA0`
(`docs/TICK_ELEMENT_OVERRIDES.md`), the plane-removal rule `007F3970` and the landing walk
`008A20E0` (`docs/AIR_OPERATIONS.md`), the scene creator `004F0AD0`
(`docs/SCENE_UNIT_CREATORS.md`), the slot-39 attach `0077E830` / `00928A00`
(`docs/MISSION_ENTITY_LUA_ATTACH.md`), the class-id space (`docs/ENTITY_CLASS_IDS.md`), the pilot
bots `0099D300` / `009998A0` / `009F3F80` (`docs/UNIT_COMMAND_PRODUCERS.md`) and the bot task
vector (`docs/BOT_TASKS.md`).

## 1. Correction to the packet brief: the creator's callee list

The brief lists `0047BB70`, `004B3F10`, `00851CB0`, `004F04C0` and `008F0DF0` as creation-time
callees of `004F0AD0`. **None of them is called from that body.** `004F0AD0`'s Ghidra body is
`004F0AD0..004F0BDF` and its only calls are, in order:

| Site | Callee | What it is |
| --- | --- | --- |
| `004F0AF1` | `00BF55BE` | `operator new(0x414)` |
| `004F0B01` | `00BF79F0` | `memset(instance, 0, 0x414)` |
| `004F0B18` | `007F2C60` | the squadron constructor, `__thiscall(instance, 0)` |
| `004F0B32` | `008F2260` | `BSP_ScenePropertyBag_Find(bag, "Type")` |
| `004F0B3C` | `00964790` | `BSP_VehicleClass_GetOrCreate(Type, 1)`, result discarded |
| `004F0B41` | `004C1130` | `BSP_FrameJobPool_GetSingleton` |
| `004F0B4E` | `[[pool+4]+10h]` | the deferred-hierarchy predicate (indirect) |
| `004F0B75` | `[[squadron]+98h]` | **`007EFA70`**, the place-in-world slot (indirect) |
| `004F0B7F` | `004F03C0` | `BSP_SceneUnit_DeferHierarchyToProperties` (the other arm) |
| `004F0BA7` | `0041DD40` | `BSP_NativeString_Resize` for the name at `+154h` |
| `004F0BB8` | `00BF7680` | `memcpy` of the name |
| `004F0BC4` | `004E6B30` | `BSP_SceneUnit_ApplyCommandProperty` |

The five extra names are index artefacts, not call sites (checklist rule 2, boundary before
attribution). Three of them are near-misses worth recording so the next reader does not repeat
the lookup: `004F04C0` is a standalone helper holding **exactly the code `004F0AD0` inlines** at
`004F0B41..004F0B7F` (the pool predicate, then either `vtable[98h]` or `004F03C0`); `0047BB70` is
a one-line forwarder `(*param_1)->vtable[8h](param_2)`; `004B3F10` fetches a wreck class through
`BSP_WreckClass_GetOrCreate` and calls its `vtable[0Ch]` when `class+28h` is clear; `00851CB0`
creates and caches a `StationaryClass` Lua-backed descriptor (`operator new(0x3C)`); `008F0DF0` is
a shared property helper with 27 callers. `coverage: complete` for the call list of `004F0AD0`;
`contract: unread` for the bodies of `00851CB0` and `008F0DF0` beyond the one-line summaries.

`007EFA70` has no Ghidra function: it is the five-byte thunk `JMP 00928860`
(`BSP_GameEntity_PlaceInWorld`), decoded from the raw bytes with capstone.

## 2. The 0x414-byte object

Allocated and zeroed by the caller (`004F0AD0` at `004F0AEA`, `007F3500` at its head), then
constructed by `007F2C60` (`__thiscall(this, int party)`, `RET 4`, body `007F2C60..007F2E16`).
`007F2C84` calls the shared entity base `0077EED0` with the single argument; `007F2C9A` calls
`00875890` with `ECX = this+310h`, payload `this` and group `3` (`PUSH 0x3` at `007F2C89`).
Coverage: **complete** for the constructor.

| Offset | Constructor value | Meaning | Writers |
| --- | --- | --- | --- |
| `+0h` | `00D087C0` | primary vtable | `007F2CAD`, re-installed by the destructor `007EF9F6` |
| `+10h` | `00D087A8` | second base vtable | `007F2CB3` |
| `+24h` | `00D087A0` | ref-counted deleting-destructor vtable | `007F2CBA` |
| `+30h` | (base) | world node | `00928860` |
| `+74h` | (base) | local matrix, used as the no-parent spawn matrix | read at `007F48C9` |
| `+C0h` | (base) | the `0Ch`-byte spawn descriptor | `00922E20` (catapult), `007F4580` reads it |
| `+C4h` | `18h` | class id | `007F2DEC` |
| `+154h`/`+158h` | (base) | name length and pointer | `004F0AD0`, `007F3500`, `007F3820` |
| `+170h` | `00D0879C` | base vtable | `007F2CC1` |
| `+1E4h` | `00D08794` | base vtable | `007F2CCB` |
| `+310h` | `00D0877C` | tick registration node, group 3 | `00875890` at `007F2C9A` |
| `+34Ch` | `0` | secondary avoid-zone layer | `007F1D90` |
| `+350h` | `0` | primary avoid-zone layer | `007F1D90` |
| `+354h` | `13h` | a constant, meaning unread | `007F2DF6` |
| `+358h` | `0` (byte) | rename flag | `007F3820` |
| `+35Ch` | `0` | the plane class descriptor | `007F4783`, `007F4652`, `007F46E1` |
| `+360h` | `0` (byte) | unread | — |
| `+361h` | `0` (byte) | "exit zone already handled" | `007F31A0` |
| `+364h` | `-1` | `Behaviour` | `007F47D9`, `007F46FE` |
| `+368h` | `0` (byte) | unread | — |
| `+369h` | `1` (byte) | copied from a source squadron's `+124h` | `007F3500` |
| `+36Ah` | `0` (byte) | last-plane flag | `007F3970` (`007F3A45`, `007F3A51`) |
| `+36Ch` | `0.0f` | unread | — |
| `+370h` | `1` | cleared when the squadron aborts | `007F31A0` |
| `+378h` | `1` (byte) | unread | — |
| `+379h` | `0` (byte) | unread | — |
| `+37Ch..+38Fh` | `007F2BD0` | four countdowns plus four freeze bytes | the tick |
| `+3B0h` | `0` (byte) | "any plane ready" scratch | the tick |
| `+3B4h` | `0` | the ready plane's value | the tick |
| `+3B8h` | `0` (byte) | previous `+3B0h` | the tick |
| `+3BCh` | `0` | unread | — |
| `+3C0h` | `1.0f` | periodic period (`DAT_00D7A24C`) | the tick |
| `+3C4h` | `-1.0f` | periodic countdown (`00BD2F10(0,1.0f)` then `FCHS` at `007F2D60`) | the tick |
| `+3C8h` | `0` | `WingCount` | `007F4778` |
| `+3CCh` | `0` | live plane count | `007F4B60` (+1), `007F39ED` (-1) |
| `+3D0h..+3E0h` | `0` | **five** plane pointers | `007F4B55`, `007F3970` |
| `+3E4h` | `1` | unread | — |
| `+3E8h` | `1.0f` | morale, approaches 1 in the tick | the tick |
| `+3ECh` | (memset) | dirty byte | `007F4B6E`, `007F3A1D`, `007ED67C` |
| `+3F0h` | `00CF938C` | embedded subobject, destroyed by `006E0860` | `007F2D93` |
| `+3F4h`/`+3F8h`/`+3FCh` | `0` | subobject fields | `007F2D74`.. |
| `+400h` | `1` (byte) | subobject flag | `007F2D8C` |
| `+404h` | `0` | subobject field | `007F2D86` |
| `+408h..+413h` | (memset) | unread | — |

`DAT_00D7A24C` is `1.0f`: `007F4820` loads it into `XMM1` and `007F484E`/`007F4878`/`007F48A5`
store it on the diagonal of a stack 4x4 while `XMM0` (zeroed at `007F481D`) fills the rest.

The destructor `007EF9D0` (body `007EF9D0..007EFA62`) re-installs the five vtables, then destroys
`+3F0h` (`006E0860`), the tick node at `+310h` (`00875490`) and the base (`0077E380`).

### The vtable `00D087C0`

100 slots, `00D087C0..00D0894C`; the string `bomb` starts at `00D08950`, read from the PE image
rather than from `ghidra xrefs`. The slots this packet identified:

| Slot | Target | Role |
| --- | --- | --- |
| `+0h` | `007F1140` | scalar deleting destructor |
| `+0Ch` | `007F3820` | set name, and re-prefix every plane's name |
| `+5Ch` | `007EFB00` | `IsKindOf`, answers `18h` |
| `+7Ch` | `007F3A60` | control handover when the squadron was the player's unit |
| `+80h` | `007F3B10` | `BSP_Aircraft_OnDestroyed` |
| `+98h` | `007EFA70` -> `00928860` | place in world |
| `+9Ch` | `007F4580` | attach the Lua self table **and create the planes** |

## 3. Creation, in order

The scene loader and the catapult share one sequence. Steps 1 to 6 are `004F0AD0`; step 7 is a
later pass.

| # | Site | Step |
| --- | --- | --- |
| 1 | `004F0AEA` | `operator new(0x414)` and `memset 0` |
| 2 | `004F0B18` | `007F2C60(instance, party)` fills the table above |
| 3 | `004F0B32` | read `Type` (`00CE4780`) from the property bag |
| 4 | `004F0B3C` | `00964790(Type, 1)` for the class-cache side effect only |
| 5 | `004F0B75` or `004F0B7F` | place in world, or defer the hierarchy to the property pass |
| 6 | `004F0BA7` | copy the entity name into `+154h`/`+158h` |
| 7 | `007F4580` | the slot-39 attach: read the bag again and **create the planes** |

The catapult path (`006EC8E0`, `docs/AIR_OPERATIONS.md`) reaches the same creator at `006ECA96`
and then installs the `0Ch`-byte spawn descriptor at `squadron+C0h` (`00922E20`) before the
attach runs. That descriptor is what step 7 switches on.

## 4. When the planes are created: `007F4580`

`__fastcall(ECX = squadron)`, `RET`, Ghidra body `007F4580..007F4B94`. Coverage: **complete for
the mode-1 arm**; `contract: unread` for the bodies of `007B8A80`, `00521E30`, `00922DE0`,
`00742A70` and `009238A0`.

`007F45A7` calls the base slot-39 attach `0077E830` first, on every arm. `007F45B2` then reads
`kind = [[squadron+C0h]+4h]` and branches three ways:

| Kind | Site | What it does | Planes |
| --- | --- | --- | --- |
| `1` property bag | `007F471A` | the bag is `[squadron+C0h]+8h`; the loop below | **yes** |
| `2` source record | `007F46D1` | `+35Ch` from `[record+110h]`, `007F1D90` with `[record+11Ch]`, `+364h` from `[record+120h]` | no |
| `3` Lua table | `007F45D0` | reads `classIndex` (`00D08A24`) and `climbAngleTangent` (`00D08A10`) out of a `_planeSquadron` table (`00D08AD4`) | no |

So a squadron's planes exist only once its slot-39 attach has run with a kind-1 descriptor.
That is the same pass `docs/MISSION_ENTITY_LUA_ATTACH.md` describes, which is why a kind-3
(script-owned) squadron never grows planes from native code.

`007F1D90(squadron, slope, flag)` sets the two avoid-zone layers: `+350h` from
`BSP_AvoidZoneRegistry_SelectLayerBySlope(slope, flag)` and `+34Ch` from the same call with
`DAT_00CE380C` and `1`.

### Mode 1, in order

| # | Site | Step |
| --- | --- | --- |
| 1 | `007F471D` | `008F2260(bag, "Type" 00CE4780)`, class id = `prop+0Ch` |
| 2 | `007F4735`/`007F4754` | `WingCount` (`00CF8840`): default `3`; when present, `max(1, prop+0Ch)` |
| 3 | `007F4778` | store the wing count at `+3C8h` |
| 4 | `007F477E` | `007B8A80(classId)` -> `+35Ch` |
| 5 | `007F4794` | `PlaneParentID` (`00CFACC8`), only when the type tag `prop+4h` is `0`; `00521E30` resolves it |
| 6 | `007F47C2` | `Behaviour` (`00CFACBC`), same type-tag gate -> `+364h` |
| 7 | `007F47E1` | `007B8A80(classId)` a second time: the factory the loop drives |
| 8 | `007F4811` | per wing: `factory->vtable[28h](0)` allocates the plane |
| 9 | `007F48BA` / `007F48D2` | `plane->vtable[98h]`: with a parent, identity matrix under the parent; without, the squadron's `+74h` under `[squadron+30h]` |
| 10 | `007F48D6`/`007F48FD`/`007F491A` | `operator new(0Ch)`, `00922DE0(squadron descriptor)`, store at `plane+C0h` |
| 11 | `007F4926` or `007F49EF` | name the plane (below) |
| 12 | `007F4B43` | `plane+9D8h = squadron+3CCh` (the spawn index) |
| 13 | `007F4B49` | `plane+9D4h = squadron` (the back pointer) |
| 14 | `007F4B55` | `squadron+3D0h[count] = plane` |
| 15 | `007F4B60` | `squadron+3CCh += 1` |
| 16 | `007F4B6E` | `squadron+3ECh = 1` |

Step 14 has **no bound test**. The array is five pointers (`007F2DA3..007F2DBB` zero exactly
`+3D0h`, `+3D4h`, `+3D8h`, `+3DCh`, `+3E0h`), so a sixth wing would overwrite `+3E4h`, a seventh
`+3E8h`. The authored data is what keeps that unreachable: `universe/library/plane.props`
declares `enum PlaneWingCount { " 1"=1 .. " 5"=5 }`, and across `universe/` the shipped scenes use

| `WingCount` | authored entities |
| --- | --- |
| 1 | 63 |
| 2 | 62 |
| 3 | 872 |
| 4 | 39 |
| 5 | 530 |

The `PlaneSquadronWNavpoint` property descriptor in `plane.props` declares `Type`, `Velocity`,
`WingCount`, `SubType`, `Skill`, `Equipment`, `HomeBase` and `NoseArt`; an authored entity such as
`entity "Bomber" (PlaneSquadronGen)` in `universe/scenes/traininggrounds/usn_airtraining.scn`
omits `WingCount` entirely, so the code default of 3 is what that squadron gets even though the
descriptor's own default is `" 1"`. Only `Type`, `WingCount`, `PlaneParentID` and `Behaviour` are
read by `007F4580`; the rest are consumed by other passes.

### Plane names

| Wing count | Site | Name |
| --- | --- | --- |
| `1` | `007F4926` | `<squadron> + "|"` (`00CF100C`) |
| `> 1` | `007F49EF` | `<squadron> + "|.-" (00CF8038) + (index + 1)`, the number via `00742A70` |

`007F3820` (vtable `+0Ch`) is the other half: it sets `squadron+358h`, renames the squadron
through `0077D3F0`, then for each of the `+3CCh` planes (`007F3885 CMP EBX,4` / `007F3888 JA`: an
index above 4 yields a null pointer, a second witness for the five-slot array) cuts the plane
name at the first `|` (`_strcspn` `00BF9A50` at `007F38AF`) and calls `plane->vtable[0Ch]` with the new prefix plus the old suffix.

## 5. Death and compaction

`007F3970 BSP_Squadron_RemovePlane` is documented in `docs/AIR_OPERATIONS.md` and modelled by
`squadron_remove_plane_007f3970` in `include/bsp/air_operations.hpp`; this packet adds one fact
from the listing. The sequence is `007F3997` `007F2FD0`, memmove the tail towards the removed
slot, `007F39ED` `ADD [ESI+3CCh],-1`, `007F39FA` zero the now-unused last slot, `007F3A07` clear
`plane+9D4h`, `007F3A11` `007ED260`, `007F3A1D` set `+3ECh`, then `00926D90` and the `+36Ah`
writes at `007F3A45`/`007F3A51`.

**`plane+9D8h` is never rewritten.** The spawn index stamped at `007F4B43` is a spawn-order id,
not a live array index: after any compaction it no longer matches the plane's slot. Nothing in
this packet's range reads `+9D8h`, so the consumer is `contract: unread`.

`007F31A0` (`__thiscall(squadron, plane, arg)`) handles a plane leaving the map. When the plane
is the one at `+3D0h` (the leader) it walks the tail from `+3D4h`, refreshes each live plane's
pose, and for each plane that answers `FUN_0071C4F0(plane+FCh)` routes a session message of id
`7` and decrements a survivor count; when survivors remain and `+361h` is still clear it sets
`+361h`, clears `+370h`, computes a scaled point from the squadron's world position and
`_DAT_00CE3DF0`, and issues an entity command. Either way it ends with
`BSP_MissionEntity_Kill(4)`. Coverage: **partial** — the command payload built at `007F31A0`'s
`FUN_00468560` call and `FUN_004B4850` are unread.

`007F3A60` (vtable `+7Ch`) fires only when the squadron is the player's controlled unit
(`DAT_00E188D8`): outside four excluded game states it pushes a front-end interface request
(`0x34`) or calls `[DAT_00E198C4]->vtable[10h]`, then `BSP_Game_SetControlledUnit`, then
`0077D270`. `contract: unread` for `00565FB0` and `0077D270`.

`007F3500` builds a squadron from an existing one: `operator new(0x414)`, `memset`, `007F2C60`
with `*(u16*)(source+1Ch)`, copy the name from `source+C0h` into `+154h`/`+158h`, copy
`source+124h` into `+369h`. It creates **no** planes and has no direct caller in the call graph,
so it is reached through a vtable slot this packet did not locate: `contract: unread`.

## 6. The wave-3 tick

`007F3BA0 BSP_PlaneSquadron_TickAdvance` is fully documented in
`docs/TICK_ELEMENT_OVERRIDES.md` and reconstructed as `squadron_tick_advance_sim_007f3ba0` in
`include/bsp/tick_element_overrides.hpp`; it is **not** re-derived here and
`include/bsp/plane_squadron.hpp` deliberately declares none of its types. What this packet adds
is the provenance of the three constants that document reads out of the object: `+3C0h` and
`+3E8h` are seeded to `1.0f` and `+3C4h` to `-1.0f` by `007F2C60` (section 2), which is why the
periodic callback fires on the squadron's first step and why the morale approach starts saturated.

The tick's member loop is the only per-step contact with the planes, and it does not call a pilot
bot: `009998A0 PilotBot_Update` and `0099D300 PilotBot_PlanControls`
(`docs/UNIT_COMMAND_PRODUCERS.md`) are reached from the plane's own tick element, not from the
squadron's. No call into the bot task vector (`docs/BOT_TASKS.md`) appears anywhere in
`007F3BA0`, `007F4580` or `007F2C60`; the scheduler loop that document could not locate is not in
this packet's range either. "Pure squadron AI" is, on the evidence: four countdowns, a morale
approach, a periodic callback, one pass over the planes that raises each plane's `+2ECh` to the
squadron's, collects a readiness bit, and drops planes that report state `1`.

## 7. Host table

One row per native call site this packet models. `this`/args are read from the listing.

| Site | In | Callee | Host method | this / args | ret | Gate |
| --- | --- | --- | --- | --- | --- | --- |
| `004F0B18` | `004F0AD0` | `007F2C60` | (constructor) | squadron; `0` | squadron | allocation succeeded |
| `004F0B75` | `004F0AD0` | `[squadron+0h]+98h` = `007EFA70` -> `00928860` | `place_plane_in_world` (same slot) | squadron; parent, `[00E188A8]+19CCh`, matrix | void | pool predicate false |
| `007F2C84` | `007F2C60` | `0077EED0` | (base constructor) | squadron; party | void | always |
| `007F2C9A` | `007F2C60` | `00875890` | (tick registration) | `squadron+310h`; squadron, `3` | void | always |
| `007F2D2B` | `007F2C60` | `007F2BD0` | (timer block) | `squadron+37Ch` | void | always |
| `007F2D5B` | `007F2C60` | `00BD2F10` | (pow/negate helper) | `0.0f`, `1.0f` | `1.0f` in `ST0` | always |
| `007F45A7` | `007F4580` | `0077E830` | `attach_lua_self_base_0077e830` | squadron | void | always |
| `007F4724` | `007F4580` | `008F2260` | `read_spawn_properties_008f2260` | bag; `"Type"` | prop or `0` | kind 1 |
| `007F4742` | `007F4580` | `008F2260` | same | bag; `"WingCount"` | prop or `0` | kind 1 |
| `007F4759` | `007F4580` | `008F2260` | same | bag; `"WingCount"` | prop | key present |
| `007F479D` | `007F4580` | `008F2260` | same | bag; `"PlaneParentID"` | prop or `0` | kind 1 |
| `007F47C7` | `007F4580` | `008F2260` | same | bag; `"Behaviour"` | prop or `0` | kind 1 |
| `007F477E` | `007F4580` | `007B8A80` | `vehicle_class_for_type_007b8a80` | classId in `ECX` | class | kind 1 |
| `007F47AE` | `007F4580` | `00521E30` | `resolve_parent_entity_00521e30` | id in `ECX` | entity | key present, tag `0` |
| `007F47E1` | `007F4580` | `007B8A80` | `vehicle_class_for_type_007b8a80` | classId in `ECX` | class | kind 1 |
| `007F4811` | `007F4580` | `[class+0h]+28h` | `create_plane_instance` | class; `0` | plane | per wing |
| `007F48BA` | `007F4580` | `[plane+0h]+98h` | `place_plane_in_world` | plane; parent, `[squadron+30h]`, identity | void | parent resolved |
| `007F48D2` | `007F4580` | `[plane+0h]+98h` | `place_plane_in_world` | plane; `0`, `[squadron+30h]`, `squadron+74h` | void | no parent |
| `007F48D6` | `007F4580` | `00BF681B` | (allocator) | `0Ch` | block | per wing |
| `007F48FD` | `007F4580` | `00922DE0` | `clone_spawn_descriptor_00922de0` | block; `squadron+C0h` | descriptor | block non-null |
| `007F492F` | `007F4580` | `0041E870` | `set_plane_name` | tmp; `"|"` | string | wing count `1` |
| `007F494B` | `007F4580` | `004261A0` | `set_plane_name` | `squadron+154h`; out, tmp | string | wing count `1` |
| `007F49F8` | `007F4580` | `0041E870` | `set_plane_name` | tmp; `"|.-"` | string | wing count `> 1` |
| `007F4A13` | `007F4580` | `00742A70` | `set_plane_name` | out; index `+ 1` | string | wing count `> 1` |
| `007F4A32` | `007F4580` | `004261A0` | `set_plane_name` | `squadron+154h`; out, tmp | string | wing count `> 1` |
| `007F4A47` | `007F4580` | `004261A0` | `set_plane_name` | out; out2, number | string | wing count `> 1` |
| `007F4699` | `007F4580` | `007F1D90` | (avoid-zone layers) | squadron; slope, `0` | void | kind 3 |
| `007F46F3` | `007F4580` | `007F1D90` | (avoid-zone layers) | squadron; `[record+11Ch]`, `0` | void | kind 2 |
| `007F46DA` | `007F4580` | `007B8A80` | `vehicle_class_for_type_007b8a80` | `[record+110h]` | class | kind 2 |
| `007F3885` | `007F3820` | — | (index cap) | — | — | index `> 4` yields `0` |
| `007F3855` | `007F3820` | `0077D3F0` | (squadron rename) | squadron; name, flag | void | always |
| `007F38AF` | `007F3820` | `00BF9A50` | (`_strcspn`) | plane name, `"|"` | index | name non-empty |
| `007F391A` | `007F3820` | `[plane+0h]+0Ch` | (plane set name) | plane; new name, flag | void | per plane |
| `007EFA32` | `007EF9D0` | `006E0860` | (subobject destructor) | `squadron+3F0h` | void | always |
| `007EFA3E` | `007EF9D0` | `00875490` | (tick node destructor) | `squadron+310h` | void | always |
| `007EFA4D` | `007EF9D0` | `0077E380` | (base destructor) | squadron | void | always |

## 8. Open questions

- `+354h = 13h` and `+3E4h = 1`: written once by the constructor, no reader found in range.
- `+9D8h` (the plane's spawn index) has no located reader; the compaction leaves it stale.
- `007F3500`'s caller: it is not in the call graph and is not one of the `00D087C0` slots this
  packet identified.
- The `"|.-"` separator is verified as the three bytes `7C 2E 2D 00` at `00CF8038`, preceded by
  `"^globals.down"`. No shipped scene was searched for a plane name carrying it.
- `00851CB0`, `008F0DF0`, `00565FB0`, `0077D270`, `007F2FD0`, `007ED260`: `contract: unread`.

## 9. Ledger names added

| Address | Name | Status |
| --- | --- | --- |
| `007F4580` | `BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes` | analyzed, reconstructed (mode 1), build-tested |
| `007EF9D0` | `BSP_PlaneSquadron_Destruct` | analyzed |
| `007F3820` | `BSP_PlaneSquadron_SetName` | analyzed |
| `007F31A0` | `BSP_PlaneSquadron_OnPlaneLeftMap` | analyzed (partial) |
| `007F3A60` | `BSP_PlaneSquadron_ReleaseControlledUnit` | analyzed |
| `007F3500` | `BSP_PlaneSquadron_CloneFrom` | analyzed |
| `007F1D90` | `BSP_PlaneSquadron_SelectAvoidZoneLayers` | analyzed |
| `007F2BD0` | `BSP_PlaneSquadron_InitTimerBlock` | analyzed (call site only) |

## Handoff: why no squadron in this host has members (packet `cc8_plane_squadron_members`)

Written by the torpedo run-in stream as the brief for a fresh worker taking the host half. Nothing
below changes this document's existing readings; it adds the chain that arrives here from the
torpedo side and the state of the host.

### 1. The gate chain, as one unit

The torpedo stream reached this class from the other end: USN01 builds five kind Eh torpedo tasks
(`009D4E30`) on `Mav1..Mav5` and they never close — `aim_ticks=0`, `min == last` on every approach,
`007CE9FD` waiting 7200 frames with `issues=0`. The reason is one routine.

`007EEF30 BSP_PilotControl_IssueReleaseOrders` is a single unit of logic with the block the
disassembly labels `007EEF40`, which is **not a function**: it is the fall-through body entered
immediately after `007EEF3B CALL 007EE7F0`.

```
007eef3b: CALL 0x007ee7f0            ; RefreshArmedFraction, which WRITES ctl+374h
007eef40: FLD  float ptr [ESI + 0x374]
007eef46: FLD  float ptr [ESI + 0x390]
007eef4c: FCOMIP ST0,ST1
007eef50: JBE  0x007eef96            ; test 1: needs ctl+390h > ctl+374h, strictly
007eef54: CMP  dword ptr [ESI + 0x3cc],EBX
007eef5a: JLE  0x007eef96            ; test 2: needs the member count > 0
007eef5c: LEA  EDI,[ESI + 0x3d0]     ; the member array of this document
007eef62: CMP  byte ptr [ESI + 0x378],0x0
007eef7f: CALL 0x007bcbe0  (0x3E7)   ; per member, when 007B8AD0 says so
```

`007EE7F0 BSP_PilotControl_RefreshArmedFraction` walks the same `+3D0h` array under the same `+3CCh`
count, counts members whose `[member+5Ch]` is set, calls `007C1F60` on each, and accumulates two
floats; **when `+3CCh <= 0` it jumps to `007EE891` and stores zero into `+374h`.** So `+374h` is the
armed fraction, named by its writer. `007F3BA0 BSP_PlaneSquadron_TickAdvance` calls the same
refresh, which is what identifies the object as this class.

**`+390h`'s producer is unlocated, and that is a negative I am not claiming as absent.** A store
census of the offset over `.text` returns 39 sites and an address-of (LEA) census returns **zero**;
none of the 39 is a float store on this class. The two that look tempting —
`BSP_GameTuning_LoadFromPlaneGlobals` writing `Pilot/Follow/LargePlaneTurnMul` at tuning+390h and
`Pilot/MoveTo/SwitchNextPointTime` at tuning+374h — are the **tuning singleton**, a different object
sharing the offsets. A pure collision, and the same trap that made a census on `block+C0h` vacuous in
`docs/AIROPS_LAUNCH_TICK.md`. Neither this document nor `PLANE_SQUADRON_ENTITY.md` names `+374h` or
`+390h`, so both are unread fields of this class; `+390h` did **not** fall out of reading the spawn
tail, and finding it is left open rather than guessed.

It does not block the diagnosis, because **test 2 fails on its own**.

### 2. The root cause, in one paragraph

This host makes **one plane per `PlaneSquadronGen` scene row** and **one plane per air-ops launch**,
with no squadron object in either case, so `+3CCh` is always 0 and `+3D0h[]` always empty. Every
symptom follows: `007EE7F0` writes `+374h = 0`, `007EEF40`'s float test cannot pass against an
unset `+390h`, its count test fails regardless, no member is ever offered an arm, and the torpedo
task's issue stage waits for ever. The image's aircraft are squadrons; this host's are lone planes.

### 3. What it costs to fix: the census multiplier

From the authored data in this document's own table:

| `WingCount` | authored entities |
| --- | --- |
| 1 | 63 |
| 2 | 62 |
| **3** | **872** |
| 4 | 39 |
| **5** | **530** |

1566 authored squadrons, modal wing **3**, and an entity that omits `WingCount` gets the code default
of 3 rather than the descriptor's own default of 1. So this lands at roughly **4,800 aircraft where
the host has 1,566 units today**, and every aircraft census in every mission moves by about 3x.
That is the reason for the before columns in section 5.

### 4. Where the stand-in lives today

| place | what it does now | what it must become |
| --- | --- | --- |
| `src/game_hosts_scene_contents.cpp`, the class-creator block | a `PlaneSquadronGen` row becomes one unit through `create_plane_squadron_004f0ad0`, which allocates an instance and places it | a squadron entity that then runs the spawn tail, creating `WingCount` member planes |
| `GameUnitsHost::create_units` | one `GameUnitSlot` per created record | unchanged for the squadron, plus one slot per member plane |
| `GameScriptOrdersHost::create_air_ops_squadron_006c5050` | **one plane of the slot's class stands in for the squadron**, labelled in `docs/AIROPS_LAUNCH_TICK.md` section 7 | the same squadron entity, so the launch route and the scene route converge on one spawn tail. `LaunchSquadron(carrier, class, 3)` already passes the modal wing and `006C5050`'s bag already carries `WingCount` |
| `GameMissionLuaHost::attach_created_entity_00928a00` | gives the single unit a `thisTable` slot | the **squadron** keeps the script-facing slot; members need none, since the script never names them |
| the `squadron` key in `push_air_ops_slot_entry` | pushes the integer entity id, which the script converts with `thisTable[tostring(...)]` | unchanged. `GenerateObject` returns the table, `squadron` the integer; both conventions are recorded in `docs/USN04_STRIKE_CLASS.md` section 5 and must not be swapped again |
| `run_pilot_set_target` and the class stored beside the `0099A170` install | orders the one unit | orders the **squadron**, which `007EEF30` then walks to reach members |
| `air_ops_squadron_plane_count` | returns the recorded wing while the stand-in unit is alive | the real `+3CCh`, which the air-ops tick `006C0510`, `006BD3F0` and `006BF230` all already read as the squadron's live plane count |

The last row is worth stating twice: `+3CCh` is already load-bearing in the air-operations
reconstruction, and this packet makes it real rather than substituted.

### 5. Validation plan

1. **Same-binary before columns first**, on USN01 and USN04, because every aircraft census moves.
   A short run is enough for the unit census (`--frames 400 --mission-frames 300`); the long runs are
   for the order path.
2. Then the first run in which `007EEF30` issues release orders. Read, in order: the per-Mav approach
   rows (`ticks`, `replans`, `aim_ticks`, and whether `min < last` at last), the issue stage
   `007CE9FD` (`waiting`, `issues`, `first_issue_at_arm_tick`), the issue gate `007EEF40`
   (`ctl+390h`, `ctl+374h`, `open`), then the torpedo run-in itself: move-to, the glide census, the
   commanded-speed pair against the per-class stall speed, throttle samples, release altitude and
   speed, and drops / water-entry breakups / swims.
3. USN01's five Mavs start at `range_first_mean = 4174.3 m` against an engage distance of 2200.0 x 2.2
   = 4840 m, so they begin **inside** the threshold; USN04 remains the mission whose launched strike
   should start outside it. Both numbers move when the wing is real.

### 6. Open items this stream carries

* **`SpawnNew 0094C480`** is parked at its request-record half. The drain does not exist: `004C6BA0`
  has exactly three callers, all Lua bindings, and the manager's scan `009469F0` sums a per-party
  cost for `BSP_AiParty_AvailableResources`, so it is a reinforcement request against a party's
  resource budget that the AI planner spends. No unit results until `00A38DA0` and the two planner
  thinks are reconstructed.
* **`GenerateObject 00944FD0` is implemented and not validated by a run.** Neither USN01 nor USN04
  reaches a call in 7200 frames (`MissionPhase=1` and phase 2.5 respectively). The `Hidden` hold-back
  that feeds it *is* validated, on both missions, against a predicted count.
* **The USN01 run exited 1** after completing its full frame budget with a clean shutdown and no
  cause in the log. Next USN01 run should record `EXITCODE`, the last twenty lines, and whether the
  parent or the child returned it.
* **`DummyTargetVehicle`** is carried on this stream's list by the integrator; this packet did not
  investigate it and has nothing to add, so its provenance should be taken from whoever raised it.

## Independent re-reading of `007F4580` mode 1 (packet `cc8_plane_squadron_host`)

The handoff above was written from this document rather than from the listing, so the host packet
re-read the sites it depends on. Every line below is from
`python tools/bsp.py ghidra disasm 007f4580`, not from the decompiler. Section 4's table survives
unchanged except for the one citation corrected below; what follows is the evidence for it.

**The kind switch.** `007F45AC MOV EAX,[ESI+C0h]` / `007F45B2 MOV ECX,[EAX+4]`, then three
`SUB ECX,1`: `007F45B8 JZ 007F471A` (kind 1), `007F45C1 JZ 007F46D1` (kind 2), `007F45CA JNZ
007F4B7F` (anything but kind 3 leaves through the epilogue). So kind 3 is the fall-through at
`007F45D0`, and a descriptor kind of 4 or above spawns nothing and touches no field.

**The wing count.** `007F471A MOV EAX,[EAX+8]` is the bag. The four keys are literal pointers whose
bytes are read here rather than taken from a name: `00CE4780` is `54 79 70 65 00` = `Type`,
`00CF8840` is `WingCount`, `00CF100C` is the two bytes `7C 00` = a single `|`, `00CF8038` is
`7C 2E 2D 00` = `|.-`.

| Site | Instruction | Meaning |
| --- | --- | --- |
| `007F4732` | `MOV EDI,[EAX+0Ch]` | the class id out of the `Type` property |
| `007F473A` | `MOV dword [ESP+1Ch],3` | **the default 3**, written before the lookup |
| `007F4742` | `CALL 008F2260` | the `WingCount` lookup |
| `007F4749` | `JZ 007F4772` | key absent: the 3 stands |
| `007F4761` | `CMP EAX,1` | the authored value |
| `007F4764` | `MOV dword [ESP+18h],1` | the floor, written before the test |
| `007F476C` | `JL 007F4772` | below 1: the floor stands |
| `007F476E` | `MOV [ESP+18h],EAX` | otherwise the authored value |
| `007F4772` | `MOV EBX,[ESP+18h]` | the resolved count |
| `007F4778` | `MOV [ESI+3C8h],EBX` | `+3C8h` |

`[ESP+1Ch]` at `007F473A` and `[ESP+18h]` at `007F4772` are the same slot: the `PUSH 0CF8840h` at
`007F4735` left ESP four lower for the duration of the store. `max(1, authored)` is confirmed, and
so is the default of 3 against the descriptor own `" 1"`.

**The loop bound is the stack slot, not EBX.** `007F47E6 TEST EBX,EBX` / `007F47F4 JLE 007F4B7F`
skips the whole loop for a count at or below zero (unreachable after the floor above);
`007F47EC MOV dword [ESP+14h],0` is the index. Inside the body `007F4815 MOV EBX,EAX` makes EBX the
new plane, so the count is carried only by `[ESP+18h]`, which is what the back edge
`007F4B6A CMP EAX,[ESP+18h]` / `007F4B79 JL 007F4800` compares against — signed, from the branch
bytes. This is why `007F490B CMP [ESP+18h],EDI` (EDI = 1) is the naming test rather than a test on
a register.

**The per-wing allocation.** `007F4804` reloads the factory from `[ESP+1Ch]`, `007F4808 MOV EDX,[EAX]`
takes its vtable, `007F480C MOV EAX,[EDX+28h]`, `007F480F PUSH 0`, `007F4811 CALL EAX`. The
placement arms are `007F48AE..007F48BA` (a parent resolved: the 4x4 identity built on the stack from
`00D7A24C` at `007F4820`) and `007F48BE..007F48D2` (no parent: `PUSH 0`, `[ESI+30h]`,
`LEA ECX,[ESI+74h]`). `007F48D4 PUSH 0Ch` / `007F48D6 CALL 00BF681B`, then
`007F48F4..007F48FD 00922DE0([ESI+C0h])` and `007F491A MOV [EBX+C0h],EAX`.

**The naming.** `007F4906 MOV EDI,1` / `007F490B CMP [ESP+18h],EDI` / `007F4920 JNZ 007F49EF`. The
one-wing arm is `007F4926 PUSH 00CF100C`. The multi-wing arm is `007F49EF PUSH 00CF8038`,
`007F49FD MOV EDX,[ESP+14h]` / `007F4A01 ADD EDX,1` (the index plus one) through
`007F4A13 CALL 00742A70`, and two concatenations at `007F4A32` (the squadron name at `[ESI+154h]`
plus the separator) and `007F4A47` (plus the number). So a member of a one-wing squadron is
`<squadron>|` and of a multi-wing squadron `<squadron>|.-<i+1>`; both are confirmed from the
listing and from the literal bytes.

**The tail, verbatim.**

```
007f4b3d  MOV EAX,[ESI+3cch]
007f4b43  MOV [EBX+9d8h],EAX          ; plane+9D8h = the pre-append count
007f4b49  MOV [EBX+9d4h],ESI          ; plane+9D4h = the squadron
007f4b4f  MOV EAX,[ESI+3cch]
007f4b55  MOV [ESI+EAX*4+3d0h],EBX    ; members[count] = plane, no bound test
007f4b5c  MOV EAX,[ESP+14h]
007f4b60  ADD dword [ESI+3cch],1
007f4b67  ADD EAX,1
007f4b6a  CMP EAX,[ESP+18h]
007f4b6e  MOV byte [ESI+3ech],1       ; a BYTE store
007f4b75  MOV [ESP+14h],EAX
007f4b79  JL 007f4800
```

`+3ECh` is a byte, which `PlaneSquadronEntity::dirty` in `include/bsp/plane_squadron_entity.hpp`
already models as a `bool`. `007F4B43` reads `+3CCh` **before** the append, so the stamped spawn
index is the array slot the plane occupies at spawn time, and section 5 point stands: the
compaction in `007F3970` never rewrites it.

## Correction: the `WingCount` default is written at `007F473A`, not `007F4735`

Section 4 table and `include/bsp/plane_squadron.hpp` cite `007F4735` for the default of 3.

* **was**: `007F4735`/`007F4754` `WingCount` (`00CF8840`): default `3`.
* **is**: `007F4735` is `PUSH 0CF8840h`, the key literal. The default is the immediate store
  `007F473A MOV dword [ESP+1Ch],3`, and `007F4742` is the lookup whose null return at
  `007F4749 JZ 007F4772` is what leaves it standing.
* **evidence**: the listing rows in the table above.

The rule itself is unchanged, `squadron_resolve_wing_count_007f4747` is correct as written, so this
is a citation fix and not a behaviour change.

## `squadron+390h`: a sibling producer at `0079CD36`, and why it is not this class

The handoff records `+390h` as unlocated with "none of the 39 stores is a float store on this
class". One of the 39 is worth naming, because it is a float store to `+390h` on an object that a
squadron is resolved *into*, and a later reader would otherwise find it and adopt it.

`FUN_0079CBD0` builds a controller object and caches it at `[this+8h]`:

```
0079cbeb  CMP dword [EDI+8],0 / JNZ      ; already built
0079cbf9  MOV ESI,[EAX+1ch]              ; the entity
0079cc09  PUSH 18h / CALL vtable[5ch]    ; IsKindOf(PlaneSquadronGen)
0079cc13  MOV ESI,[ESI+3d0h]             ; a squadron becomes its FLIGHT LEADER
0079cc19  PUSH 8  / CALL vtable[5ch]
0079cc2d  PUSH 410h / CALL 00bf55be      ; a 0x410 object, not the 0x414 squadron
0079cc64  MOV [EDI+8],EAX
0079ccdf  MOV EDX,[ESI+538h] / FLD [EDX+0a0h] / FMUL qword [00ceffb0] / FSTP [ESP+0ch]
0079cceb  MOV EAX,[EDI+8]
0079ccee  CMP dword [EAX+3e8h],0 / JNZ   ; seed once
0079cd08  MOVSS [EAX+384h],XMM0          ; zero
0079cd10  MOVSS [EAX+388h],XMM0          ; zero
0079cd18  MOVSS XMM0,[ESP+0ch]
0079cd1e  MOVSS [EAX+38ch],XMM0
0079cd26  MOVSS [EAX+398h],XMM0
0079cd2e  MOVSS [EAX+394h],XMM0
0079cd36  MOVSS [EAX+390h],XMM0
```

The object written is the `0x410` allocation at `0079CC2D`, reached per **plane** (a squadron
argument has already been replaced by `members[0]` at `0079CC13`), and it is stored at `[EDI+8h]`,
where EDI is `FUN_0079CBD0` own `this`. It is therefore not reachable as `plane+9D4h`: the
exhaustive census of stores to `+9D4h` finds ten writers and all ten are in the plane/squadron band
(`007CDF6C`, `007CFE6C`, `007D68D5`, `007D694B`, `007ECE68`, `007ECE80`, `007ED0E6`, `007ED21A`,
`007F3A07`, `007F4B49`), none of them this chain. `007C0EFA MOV ECX,[EBP+9D4h]` is read from the
listing here, so `007EEF30` receiver is the squadron and nothing else.

**`squadron+390h` stays unlocated, and the negative is bounded rather than claimed.** The census
covers twelve store forms in both disp8 and disp32 encodings; it does not cover a 16-byte
`MOVAPS`/`MOVUPS` store reaching `+390h` from `+384h` or `+388h`, which is exactly the shape
`0079CD08..0079CD36` writes one lane at a time on the sibling, nor a block copy such as
`007F3500 BSP_PlaneSquadron_CloneFrom` (which could only propagate a value, never originate one).
The squadron own tick `007F3BA0` does not write it either: its only calls are `007F3C02 007EE7F0`,
`007F3C5B 007EE790`, `007F3CA0 007B8AD0`, `007F3CD9 00926D90`, `007F3CE7 007F3970` and
`007F3D1C 0077A650`, and no store in its body touches `+374h`, `+378h`, `+390h` or `+3CCh`.

Since `004F0AD0` allocates the block zeroed and the constructor never writes `+390h` (below), a
squadron that nothing else seeds carries `+390h = 0.0f`, and `007EEF4C` test `0.0 > +374h` is false
for every non-negative armed fraction. In the image something must seed it before a release order
can ever be issued; this packet has not found what.

## The constructor `007F2C60`, re-read for the fields the gate uses

`007F2C7D MOV ESI,ECX` is the whole function `this`, and `007F2CAD MOV [ESI],0D087C0h` stamps the
squadron vtable, so every store below is on the squadron.

| Site | Store | Note |
| --- | --- | --- |
| `007F2D1E` | `MOV byte [ESI+378h],1` | the force flag `007EEF62` tests is seeded **set** |
| `007F2D25` | `MOV byte [ESI+379h],BL` | |
| `007F2D9D` | `MOV [ESI+3CCh],EBX` | the member count starts at 0 |
| `007F2DA3` | `MOV [ESI+3D0h],EBX` | slot 0 of the array |
| `007F2DC1` | `MOV [ESI+370h],EDI` | the attack mode |
| `007F2E00` | `MOVSS [ESI+3E8h],XMM0` | the morale |

**Neither `+374h` nor `+390h` is written by the constructor.** `+374h` gets its value from
`007EE7F0` on the first refresh; `+390h` gets none, which is the paragraph above.
`PlaneSquadronConstructedState::flag_378h{1}` in `include/bsp/plane_squadron.hpp` already carried
the `+378h` seed; what is new is that the host own release-order binding contradicts it, which
`docs/PLANE_SQUADRON_HOST.md` records.

## Two more routes by which a plane joins a squadron

The `+9D4h` census above names two writers outside `007F4580` that are squadron attachments in their
own right, and a host that models only the spawn tail will not see the planes they carry:

* `007ED0E6` in `007ED0D0 BSP_PlaneSquadron_InsertPlaneSorted`, a sorted insert into the same array.
* `007D68D5` and `007D694B` in `007D5D20 BSP_Plane_ReadPropertyBag`, an authored **plane** entity
  taking a squadron from its own property bag, and clearing it again.

Both are `contract: unread` here. They are listed so the spawn tail is not mistaken for the only
producer of membership.
