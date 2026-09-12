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
