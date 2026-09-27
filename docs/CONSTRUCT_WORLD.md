# The world object behind construct_world 004DE610: layout, fillers, readers and a binding plan

Addresses: 004DE610, 004CB030, 00481640, 00875E69; read only 009037F0, 009258F0, 00928860,
00903670, 00903610, 00904C67, 00487270, 00977990, 009EEB80, 009E66EB, 0041B4E0, 00417B10,
0041BC20, 009043A0, 0042E630, 00959450.

Packet `cc9_construct_world`, part 1 (read and plan, docs only), worker cc9-side-ai,
2026-09-26, main e93b88fb0. Ghidra was read only. Every name here is a hypothesis. "(V)" means
read in the listing for this packet. "(D)" means taken from the doc named next to it.

## 1. Summary

- **What the gated consumers need.** The world object (`game+19CCh`, `4BCh` bytes) matters to
  the fixed-step gate and to 00481640 through **one byte**, `world+4ACh`. The constructor
  004CB030 sets it to 1 (004CB098) and the destructor clears it at 00904C67. Nothing else writes
  it (a displacement scan of `.text`, V).
- **The lists already exist in the host.** The 97 per-kind lists at `world+18h` are already
  modelled by the units host (`GameUnitsHost::world_list_size/entry/head`,
  `include/bsp/game_hosts_units.hpp`). Units join them on creation through their `+130h`
  registrars. Several host comments still say "the world lists are not built". That is out of
  date for every list a registrar fills (list 6 included). It is still true for list 24, which
  no host registrar fills.
- **Two of the five named consumers do not read the world.** The ship-AI ring-scan probe
  (009E66EB, 00417B10, 0041B4E0) reads the avoid-zone manager singleton `[00E17624]`. The
  plane-side sample 0041BC20 reads a layer from the avoid-zone registry `[00E17620]`. Binding
  the world moves neither.
- **The "world grid at `[game+19CCh]+84h`" is not the world's.** 009043A0 ignores the world it
  is handed in ECX and queries `BSP_SpatialIndex_GetSingleton` 0042E630 (section 6). On the
  world object, `+84h` is the count of list 9.

## 2. What 004DE610 builds

`docs/GAME_WORLD_CONSTRUCT.md` has the whole construction order. It covers 30 steps: the scene
root, the Operator node, the light, the ocean owner, the sky, the eight channel objects at
`game+21A4h..`, the marker manager `game+21D4h` and the tail objects. This section is the
world object alone.

`004DE651` allocates `4BCh` with `00BF55BE`, `004DE664` zeroes it, then `004CB030` runs and
`004DE696` stores the object at `game+19CCh`. That is the only store of that slot besides the
teardown's clear at 004D2E0D (V, census of the 284 `[reg+19CCh]` sites). `004DE69C` then calls
`009037F0(world, 1, 1)`.

| offset | meaning | writers | readers |
| --- | --- | --- | --- |
| `+0h` | vtable `00CE7784`; slot 0 is the deleting destructor 004CB0B0 | 004CB04C | teardown 004D2BB0, and virtual calls from 004C40A0, 004D7EA0, 0046B730, 005484F0, 0089BB70 |
| `+4h` | pointer to a `0Ch` header `{head, tail, count}`, the chain of every placed entity through `entity+34h/+38h` | allocated in 009037F0 (00903822, V); appended by 009258F0 (V) | the world walk 00904BF0, the kill pass 00903670 (per frame, from 004E5382), 00903610 (per fixed step) |
| `+8h` | a second header of the same shape: root entities (parent `+3Ch` null) through `+40h/+44h` | 0090383A; 009258F0 when the parent is null (V) | the destructor walk 00904C73 (D) |
| `+Ch..+14h` | zero; shaped like one more triple | 004CB057..004CB05D | none found |
| `+18h + n*0Ch` | 97 lists `{count, head, tail}`, nodes `{prev, next, value}`. `n` is a registration-category id: a destroyer joins 1, 2, 4, 5, 6 and 7 | the entity's `vtable+130h` registrar, reached from 00928860 (below) | section 4 |
| `+4A4h` | byte = 009037F0's first argument (1), 00903802 | 009037F0 | through `[unit+30h]`: 00823B70 (SEntityInit picks 008346E0 or 00834690) and 0085A3E0 (gun setup) |
| `+4A8h` | reference-counted pointer, released by the destructor (00904DA8..00904DC8) | zeroed at 009037FC; no non-zero writer found | the destructor |
| `+4ACh` | **the world-active byte** | 1 at 004CB098; 0 at 00904C67 | section 5 |
| `+4B0h..+4B8h` | `std::list` of the Lua `AddMatrixInterpolator` records | the constructor (004CB088..004CB094); 00905080 from Lua 008ADE00 (D) | 00904600 per frame (D); the destructor 00904DCE |

Named list slots seen in readers:
- **List 1** (`+24h`): every registered entity (00928560).
- **List 5** (head `+58h`): the unit list the HUD spectate and interface walks read.
- **List 6** (`+60h..+68h`): ships.
- **List 9** (`+84h..+8Ch`): the mother ships.
- **List 24** (`+138h..+140h`): plane squadrons, registered by 007F10B0 (`ADD ECX,138h`).
- **List 28** (head `+16Ch`): the squad list many AI routines walk.
- **List 69** (`+358h..`): airfields.
- **List 71** (head `+370h`): read by the avoid-zone code at 00424D00.

`docs/UNIT_WORLD_REGISTRATION.md` has the per-creator table for 21 creators.

**Correction.** `docs/GAME_WORLD_CONSTRUCT.md`'s world table names the vector element
constructor and destructor the wrong way round. Its own closing section, "Correction from
docs/UNIT_WORLD_REGISTRATION_LIVE.md", already fixes this: the constructor is 004B7EC0 and the
destructor is 004C2D30.

## 3. Who fills it

- **At load.**
  - 004DE610 builds the empty object, and 009037F0 allocates the `+4h`/`+8h` headers.
  - The scene passes of 0046DF00 then place each created entity. The per-entity creators call
    the entity's `vtable[98h]` (006DFE40, a jump to 00928860) with the world as the node (D).
- **Placement, 00928860 (V), under a lock.**
  - If the entity's old `+30h` node differs and is non-null, it runs `vtable[134h]` (detach).
  - Then 009258F0 sets `+30h`, appends to world `+4h`, and appends to world `+8h` or to the
    parent's `+48h`.
  - Then `vtable[130h]`, the per-kind registrar. It runs 00928560 (list 1) and then 00484540
    push-backs into its own lists. The destroyer's registrar is 006FE620 (lists 2, 4, 5, 6, 7).
    The squadron's is 007F10B0 (lists 2, 24).
- **Removal.**
  - **From the lists.** Slot `134h` removes the entity. For the destroyer that is 006FE670:
    006DFFC0 takes lists 5 and 6, and 006D3620 takes list 1 through 00928570 plus lists 2 and 4.
    List 7 is removed through 004837D0 (V/D). It is reached from 00928860 and from the killed
    tail 00928C80.
  - **From the chain.** 00903670 runs once per frame (004E5382) and on demand from 004D7EA0 and
    0046B730. It marks killed entities (`+5Eh` set, `+6Ch` zero, parent absent or alive) through
    00922FD0.
  - **The grace count.** 00903610, which is ungated in every fixed step (00875EDA), counts
    `+6Ch` up to 3. It then drains the children through 009035E0 and calls `vtable[0](1)`
    (V; `docs/WORLD_DEFERRED_DESTROY.md`).
- **Per frame.** Nothing else writes the lists. 00481640 does not touch them (section 5).

## 4. Who reads it

**Census.** There are 287 sites with displacement `19CCh` in `.text`:
- 3 are the D3D9 renderer's own field (00B288B0, 00B32410, 00B32920);
- 284 are world reads;
- the reads fall in 189 Ghidra functions, plus 16 sites outside any function (listed in
  `reports/cc9_construct_world.json`);
- the only stores are 004DE696 and the teardown's.

Grouped by what they touch:
- **Lists 5, 6, 24 and 28** (`+58h`, `+60h/+64h`, `+13Ch`, `+16Ch`): about 45 functions. They
  are HUD walks (0068C1F0, 006435D0), recon 008073C0, the AI world sets 00A13B60, 00A1E250 and
  00A2E720, the warning manager, and the ship AI 009F1420 and 009ED6B0.
- **The `+4h`/`+8h` chains**: 007CE040, 008AE180, 008AE2F0, 008AE480, 008C2AC0, 008C3880,
  00951940, 00974C70.
- **World methods with the world in ECX.** Ground height 00903860 from 11 callers, the
  segment query 009043A0 from 3 (it ignores ECX), and find-by-name 00925A90 from 10.
- **Creators** that pass the world to a `vtable[98h]` placement: 33 SceneUnit, projectile and
  Lua creators, plus 6 through 00923870.

**The five consumers the ranking names.**

| consumer | reads | from | computes | host stand-in | file, owner |
| --- | --- | --- | --- | --- | --- |
| warning ScanProximity 00977990 | `MOV ECX,[EAX+19CCh]; MOV EAX,[ECX+64h]` (009779A2, list 6 head); `MOV EBP,[ECX+13Ch]` (00977A25, list 24 head) | the world | for each live ship with more than one element in `+348h..+34Ch`: enemy squadrons (00803CE0 == 1) within 2000 m (squared 4.0e6 at 00D09FE8) of `squadron+3D0h` start a point effect for 2 s (0096C070, 008687C0) | `mission_events_periodic_00977990` records `WarningManager::scan_proximity` | `src/game_hosts_mission_frame.cpp:347-352`, free |
| ship neighbour count 009EEB8B (row 19) | `MOV ECX,[EAX+19CCh]; MOV EBX,[ECX+60h]` (list 6 count); elements through `009DBBC0` on `+60h` | the world | steps the path goal back until it clears each other ship's circle, `R = max(60, (e+9C8h + self+9C8h)/2)` plus the turn allowance | returns 0; `list_element_009dbbc0` returns null | `src/game_hosts_ship_ai.cpp:5490-5497`, cc9-gunnery2 |
| ring-scan probe 009E66EB / 00417B10 / 0041B4E0 (rows 9-11) | `[[unit+AA8h]]->vtable[218h]`, which is 006DFD90 for MDestroyer: `[unit+538h]` then 0082ADA0, then 004218E0 (the avoid-zone manager `[00E17624]`) reduced to the group of layer `[[unit+538h]+560h]` | **not the world** | per-slot obstacle clearance: `OffsetPointSequential(group, (x,z), 3.0, 1)` and the segment hit 004179D0 | `probe_space_vtable_0218` returns 0; the probe returns the point; the hit returns false | `src/game_hosts_ship_ai.cpp:2886-2918`, cc9-gunnery2 |
| plane AvoidZoneLayer sample 0041BC20 (row 15) | 007CE88C `MOV EBX,[EAX+34Ch]`; 007CE8FF `MOV ECX,EBX; CALL 0041BC20` | **not the world**: `squadron+34Ch`, written at 007F1DCA from `BSP_AvoidZoneRegistry_SelectLayerBySlope` 0041DF40 on `[00E17620]` | a bilinear height sample over the layer's cells | ocean height (`ocean_water_height_0078cf20`) | `src/game_hosts_units.cpp:8333-8347` and 5178-5184, cc9-plane-release |
| entity manager 00481640 and the fan-out gate 00875E69 | `[game+19CCh]+4ACh` only | the world | section 5 | the gate is hard-coded closed; 00481640 is never reached | `src/game_hosts_mission_frame.cpp:577-580`, `src/world_entities.cpp:139`, free |

**Other `+4ACh` readers** (V, a displacement scan):
- 00874D3A in 00874D00;
- 00534BCB in the HUD unit-order icon code;
- 007D0C75, 007F3A7A, 007F3B2A;
- 00806F12 in recon;
- 0095948A and 00959537 in the unit death path 00959450;
- a getter at 0047F130 with no Ghidra function.

The host models 00959450 as `unit_death_00959450` (`src/mission_result.cpp:240, 258`, input
`world_gate_4ac`). With the byte clear it reports no kill and registers no limbo page. Nothing
in the game host calls it today.

## 5. The gate, and what 00481640 is

```
00875e69  mov eax, dword ptr [0xe188a8]
00875e6e  test eax, eax
00875e70  je 0x875ec4
00875e72  mov ecx, dword ptr [eax + 0x19cc]
00875e78  cmp byte ptr [ecx + 0x4ac], 0      ; a null world would fault here
00875e7f  je 0x875ec4
```

**It opens when the game exists and `world+4ACh` is non-zero.** Behind it are fan-out rows 9 to
13:
- 00778450 on `game+1EF0h` (0.05);
- 0077EC20;
- 00874C90;
- 00925F20 with CL = 0;
- 0076FFC0 on `game+1EF0h` (0.05, 1).

Rows 14 to 16 (00926700, 009273A0, 00903610) run either way. The host fan-out
(`src/fixed_step_fanout.cpp:52-56, 89-94`) already has this shape. Its host
(`src/game_hosts_fixed_step.cpp:293-320`) records each of the five.

**00481640** (V):

```
00481640  mov eax, dword ptr [0xe188a8]
00481645  mov edx, dword ptr [eax + 0x19cc]
0048164b  cmp byte ptr [edx + 0x4ac], 0
00481652  je 0x481666
00481654  mov ecx, dword ptr [ecx + 8]       ; ECX = game+21D0h, the TrafficConfig
0048165d  mov edx, dword ptr [eax + 4]
00481664  call edx                           ; [+8]->vtable[1](delta)
```

- **What it runs.** `[game+21D0h+8]` is the 20h object 0049D690 builds (vtable 00CE6600). Its
  slot 1 is 00487270, which walks the `std::list` at `obj+0Ch` and calls each element's slot 1.
  No unit is on that list (`docs/GAME_WORLD_ENTITIES.md`, correction from
  `UNIT_INSTANCE_UPDATE.md`), so it is the traffic-group tick. What a traffic group does was not
  read.
- **When it runs.** Once per rendered frame, from 004E52F4 in `GGame::OnMove`, with the scaled
  delta `game+21F0h`.

**Correction.** The host log line "the entity manager at game+21A0h is null"
(`src/game_hosts_mission_frame.cpp` about line 2170) names the wrong slot:
- `game+21A0h` is the mission scoring object (`docs/SIDE_AI_SCHEDULER_READERS.md`);
- the manager 00481640 runs on is `game+21D0h`, and it is not null;
- what is missing is the world byte.

## 6. Other corrections

- **The spatial index.** 009043A0 never uses its ECX. Both arms call 0042E630
  `BSP_SpatialIndex_GetSingleton` and run 0098ADD0 on the result (009043AA..009043D4, V). The
  150x150 grid at `this+84h` is the spatial index's. The comments in `src/game_hosts_hud.cpp`
  (about lines 2351 and 2365) and `docs/HUD_PICK_SEGMENT_QUERY.md` that say "the world grid
  `[game+19CCh]+84h`" should name `[0042E630()]+84h`.
- **Stale comments.** "The world lists are not built" (`src/game_hosts_mission_frame.cpp:350`,
  `src/game_hosts_ship_ai.cpp:5491`) is stale for list 6. The units host fills it, and the
  ship-AI brain pre-pass already reads it through `owner_.units.world_list_head(6)`
  (`src/game_hosts_ship_ai.cpp` about 5899-5904). List 24 is still empty, because no host
  registration schedule covers `+138h`.

## 7. Binding plan

Each part has one switch, is committed OFF with predictions, and runs same-tree pairs with the
RNG and death-table variables set.

| part | what | files, owner | moves |
| --- | --- | --- | --- |
| **2** (this packet) | A host world object: `world+4ACh = 1` when the load step `construct_world` runs, clear at teardown. It feeds `world_active` into the fixed-step driver and `world_gate.enabled` into 00481640. The five gated rows and 00481640's traffic walk stay named records. | `src/game_hosts_mission_frame.cpp`, `src/world_entities.cpp`, `include/bsp/world_entities.hpp`, `src/game_hosts_fixed_step.cpp`; all free | the record rows only. Predicted: the gate open on every step, 4 500 / 9 000 calls on each of the five rows, one `EntityManager::submanager_update` per simulated frame; gameplay identical |
| 3 | ScanProximity 00977990 over list 6 (the units host) and list 24 (squadron registrars: 007F10B0's `+138h`) | `src/game_hosts_mission_frame.cpp` (free); list 24 through the units host, cc9-plane-release, or the squadron registry as a labelled stand-in | point effects and warning counts only; no gameplay |
| 4 | The neighbour count 009EEB8B and element 009DBBC0 from `owner_.units.world_list_size/entry(6)` | `src/game_hosts_ship_ai.cpp`, cc9-gunnery2 | ship path goals: USN02 9200/9000 and USN04 pairs, per-entity positions, deaths and hit records |
| 5 | The ring-scan probe (rows 9-11): the space from the avoid-zone manager's group for layer `class_navigation_floor_0560()`, using the host's `GameAvoidZoneRuntime` and `bsp::avoid_zone_group_offset_00417b10` | `src/game_hosts_ship_ai.cpp`, cc9-gunnery2; **not a world part** | ship avoidance slots; USN02 pairs |
| 6 | AvoidZoneLayer sample 0041BC20 from the registry `[00E17620]` layer at `squadron+34Ch` | `src/game_hosts_units.cpp`, cc9-plane-release; **not a world part** | plane terrain avoidance heights; USN04 pairs |
| 7 | The five gated fan-out rows: 00778450 and 0076FFC0 (the session pump and outbound flush; single player), 0077EC20, 00874C90 and 00925F20 (pending creates, tick registrations and pending inits). Each needs its body read. | `src/game_hosts_fixed_step.cpp`, free | unknown until read; the job-wave groups at 00F876C0 stay empty until 00874C90 runs |
| 8 | The traffic-group tick 00487270 and its elements; 00959450's world-byte input once the death path is bound | `src/game_hosts_mission_frame.cpp` (free); `src/mission_result.cpp` | unknown until read |

Part 2 is safe on its own because every consumer of the byte in the host is a record today. The
other `+4ACh` readers listed in section 4 have no host binding that tests the byte.

## 8. Uncertainties

- `+Ch..+14h`, `+4A4h` and `+4A8h` have no confirmed meaning. The `4A8h` census was filtered by
  address region only.
- List 28 (`+16Ch`): the registration table puts only CommandBuilding there, while its readers
  treat it as squads. Non-unit registrars probably push into it. The filler of list 71 is
  unknown.
- 0047F130, a `+4ACh` getter, has no Ghidra function. Its callers were not found. Its extent was
  not confirmed, so it is not reported for definition.
- Slot `+218h` was checked only for MDestroyer's vtable 00CFC3D0.
- The census window follows only the register that loaded the world. Routines that copy the
  pointer (for example 00875BB0) show no field in the census table even when they read one.

## 9. Part 2: the world-active byte (`kWorldActiveByteBound`, committed OFF)

**The binding.** These changes are in `src/game_hosts_mission_frame.cpp`, which is free:
- **At load.** The `construct_world` load step sets `world.world_gate.enabled`, the host's copy
  of `[game+19CCh]+4ACh`. 004CB098 is `MOV byte ptr [ESI+4ACh],1`. The new row is
  `World::active_byte_004cb098`. The rest of 004DE610 stays the load record.
- **The two gates.** The fixed-step driver's `world_active` (00875E69..00875E7F) and
  `update_entity_manager_00481640`'s gate (0048164B) now read that byte.
- **The traffic walk.** 00481640's call is now the named record
  `TrafficConfig::group_walk_00487270`, at 00481664. It used to be "EntityManager::submanager_update"
  and bumped the unit counter; it now bumps `traffic_walks`, since no unit is on that list.
- **The five gated rows** stay the records they were (`src/game_hosts_fixed_step.cpp:293-320`).
- **Teardown.** 004D2BB0 clears the byte through the destructor (00904C67). This host runs no
  teardown, so the byte stays set until the process exits.
- **Not wired.** `unit_death_00959450`'s `world_gate_4ac` input is noted only: nothing in the
  game host calls it.
- **New summary line.** `summary mission world active_byte_bound=%d active=%d traffic_walks=%llu`.

**The other `+4ACh` readers when the byte is 1.** Each was checked for a host binding and for a
row in `local\psm_on_usn04.log`:

| site | routine | what the byte gates | reached in this host |
| --- | --- | --- | --- |
| 00874D3A | 00874D00 `BSP_Game_RunExtraFixedStep`, from the Lua Spawn, GenerateObject and LaunchAirBaseSlot bindings and 0046B730 | re-running nine fan-out rows outside the driver | no: no host method calls 00874D00 |
| 00534BCB | 00534BC0..00534BF7, **no Ghidra function**, slot 1 of vtable 00CED6E0 (installed by 00538A80 at 00538AB1) | with `this+D4h` set, calls `[this-64h]` slots 1Ch then 18h | no: the class is not built |
| 007D0C75 | 007D0B80 `BSP_Plane_HandleStateMessageKinds` | with `unit+5Eh` clear, `00498F80(unit)` on the TrafficConfig `game+21D0h` | no: the host's `plane_death_flags_007d0b80` models the flags only and has no byte test |
| 007F3A7A | 007F3A60 `BSP_PlaneSquadron_ReleaseControlledUnit` (vtable slot) | the release of the controlled squadron outside states 29h, 2Bh, 2Ch and 2Dh | no host binding |
| 007F3B2A | 007F3B10 `BSP_Aircraft_OnDestroyed` | the kill report 009813A0 | no host binding |
| 00806F12 | 00806F00..008073B9, **no Ghidra function**, slot 1 of the recon slot vtable 00D08E94 (installed by `BSP_Recon_ConstructSlot` 008050E0) | the whole body: a walk over the scanned classes 00806480 | no: the host's recon runs 008073C0 only |
| 0095948A, 00959537 | 00959450 `BSP_Unit_OnDestroyed` | the kill report (0091BDA0 or 009813A0) and the limbo page 00565FB0 | no: `unit_death_00959450` has no caller |

So no gameplay path in this host tests the byte except the two gates bound here.

**Predictions** (written before the pairs; the same tree, switch only, both variables set):

| row | USN04 4700/4500 OFF -> ON | USN02 9200/9000 OFF -> ON |
| --- | --- | --- |
| `World::active_byte_004cb098` | absent -> 1 | absent -> 1 |
| `MissionLoad::construct_world` | 1 record, unchanged | unchanged |
| `summary fixed step body` | `gate_closed_steps` 4 500 -> 0, `gated_sites_skipped` 22 500 -> 0, `records` 4 500 -> 27 000 | 9 000 -> 0, 45 000 -> 0, 9 000 -> 54 000 |
| `FixedStepFanout::pump_session`, `apply_pending_entity_creates`, `flush_tick_registrations`, `init_pending_entities`, `flush_outbound_session` | absent -> 4 500 records each | absent -> 9 000 each |
| `TrafficConfig::group_walk_00487270` | absent -> 4 500 (one per simulated frame) | absent -> 9 000 |
| `summary mission world` | `active=0 traffic_walks=0` -> `active=1 traffic_walks=4500` | -> `active=1 traffic_walks=9000` |
| `summary mission frames ... units=` | 0, unchanged | 0, unchanged |
| unimplemented total | + 5 x 4 500 + 4 500 + 1 | + 5 x 9 000 + 9 000 + 1 |
| every other native row, per-entity rows, death rows, gunnery and summary lines | identical, zero clock offset | identical |

Every consumer behind the two gates is a record, so nothing can reach a unit. The ignored
counters are `ship avoidance search refills` and `pretranslate`.

## 10. Part 2 pairs and verdict

One tree, f86198484, with `local\bin\cw_off` against `local\bin\cw_on`. The two builds differ
only by the switch. `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` were set. All four logs
show the 1600x900 fit line, the module directory in this tree and the final COM release.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| native table rows | 1,530 | 1,537 (+7, the predicted ones) | 1,431 | 1,438 (+7) |
| `World::active_byte_004cb098` | absent | 1, concrete | absent | 1 |
| the five `FixedStepFanout::*` gated rows | absent | 4,500 each, records | absent | 9,000 each |
| `TrafficConfig::group_walk_00487270` | absent | 4,500 | absent | 9,000 |
| `summary fixed step body` gate_closed / skipped / records | 4,500 / 22,500 / 4,500 | 0 / 0 / 27,000 | 9,000 / 45,000 / 9,000 | 0 / 0 / 54,000 |
| `summary mission world` active / traffic_walks | 0 / 0 | 1 / 4,500 | 0 / 0 | 1 / 9,000 |
| deaths, death rows | 41, 41 | 41, 41 equal | 22, 22 | 22, 22 equal |
| gunnery damage line | 727 hits | identical | 440 hits | identical |

**Every prediction held.** One counter moved that the predictions did not list: `fanout_sites`
went from 49,500 to 72,000 and from 99,000 to 144,000. Those are the 22,500 and 45,000 gated
sites now visited, so this is the same fact counted a second way.

A whole-log diff with heap addresses masked leaves these lines:
- the per-step fan-out lines ("world gate closed" against "open", "skipped (world gate)"
  against "record");
- the ignored `ship avoidance search` refills and `pretranslate` counters;
- the pre-mission press-start blink alpha;
- one pre-window FMOD call count in USN04 (132 against 131), before the mission.

Every per-entity row, death row, gunnery line and other summary line is identical, and the
clock offset is zero.

**Verdict: ON.** Both gates now read the image's byte. The rows behind them are named records,
waiting for parts 7 and 8.

## 11. Part 3: ScanProximity 00977990 (`kScanProximityBound`, committed OFF)

Packet `cc9_construct_world_p3`. Ghidra was read only.

**The routine** (V, 00977990..00977B8x):
1. **The ship walk.** It walks list 6 from its head (`009779A2 MOV EAX,[ECX+64h]`). A ship
   qualifies when these hold:
   - `+5Ch` is set, and `+5Dh`, `+60h` and `+5Eh` are clear;
   - the part-descriptor vector at `+348h..+34Ch` (`kUnitPartTableOffset` = 344h, the vector
     object) holds more than one entry.
2. **The squadron walk.** For each qualifying ship it walks list 24 (`00977A25 MOV
   EBP,[ECX+13Ch]`). It looks for a live squadron that meets all of these:
   - its side (`+54h`) differs from the ship's;
   - `00803CE0(ship side, squadron)` returns 1;
   - its leader `[squadron+3D0h]` is within `4.0e6` squared (`00D09FE8`, a double), which is
     2000 m. The pose is refreshed first when `+C8h` is clear.
3. **The record.** `00977B04 ADD ECX,184h; CALL 00975D00` fetches the ship's record from the
   manager's `std::map<entity, record>`. A missing record is inserted with the defaults
   -1.0e10 (`00CE4ADC`) and a null effect. The torpedo effect 00977820 uses the same map and
   record, at field `+0h`. The scan uses `+4h`, a deadline, and `+8h`, an effect.
4. **A hit.** `0096C070(record, ship)` creates a point effect from the definition at
   `manager+194h`, parented to the ship's scene node `+4A4h`. The deadline becomes
   clock + 2.0 (`00D7A308`).
5. **A miss.** When the deadline has passed and the effect is live, `00867B10` stops it,
   `effect+9 = 1`, and it is released.

**00803CE0** (V) takes the side in ECX and the unit in EDX:
- The level is `[unit+1E8h+side*34h]`: its `+8h` field when `+10h` is set, else its `+4h`.
- A level below 2 returns 3.
- Otherwise the side matters. Observer side 2 returns 0 for a neutral unit and 2 for any other.
  Any other observer side returns 0 for its own side, 2 for a neutral unit, and 1 for an enemy.
- So 1 means "an enemy, identified". The host answers the level from
  `ReconSensorPassState::level(side, unit) == identified`, through the gunnery host.

**Who is warned.** Nobody in the manager's sense. The output is a point effect on each
qualifying ship, whoever controls it. It raises no radio message, has no controlled-unit guard
and has no gameplay reader. The effect definition at `manager+194h` is loaded by 00980380
`BSP_WarningManager_LoadEventTable`.

**The binding** (`src/game_hosts_mission_frame.cpp`, which is free):
- **The walk.** It covers list 6 from the units host's registry (`world_list_size/entry(6)`),
  the four bytes through `unit_alive_and_visible`, the side through `unit_side_0054`, and the
  positions through `unit_position_00fc`.
- **The record map.** `proximity_records` holds `+4h` and `+8h` of the 00975D00 record. The
  torpedo path keeps its own map for `+0h`, so the two stay apart.
- **The relation.** `relation_00803ce0` is as above.
- **Rows and summary.** The effect start and stop are named records:
  `WarningManager::proximity_effect_0096c070` and `proximity_effect_stop_00867b10`. A summary
  line reports `proximity scan bound scans ships records hits expiries`.
- **Stand-ins:**
  - **The part test** answers true for every list-6 entity. The kinds that register id 6 are
    Destroyer, Cruiser, LandingShip, Cargo, BattleShip, Submarine, TorpedoBoat and MotherShip
    (`docs/UNIT_WORLD_REGISTRATION.md`); each is a ship class. Whether each model carries more
    than one part descriptor was not checked against the data.
  - **List 24** is an empty list, because no host registrar fills it.

**Contract for cc9-plane-release (`src/game_hosts_units.cpp`, `include/bsp/game_hosts_units.hpp`).**
Two entries would retire both stand-ins:
- `std::size_t unit_part_descriptor_count(std::size_t index) const`: `(unit+34Ch - unit+348h) >> 2`,
  the part table 0087BD4F/0087BDBC fill;
- the squadron registrar 007F10B0's push into id 24 (`ADD ECX,138h`), so that
  `world_list_size/entry(24)` answers.

**Predictions** (written before the pairs; the same tree, switch only, both variables set):

| row | USN04 4700/4500 OFF -> ON | USN02 9200/9000 OFF -> ON |
| --- | --- | --- |
| `WarningManager::scan_proximity` | UNIMPLEMENTED 55 -> concrete 55 | 111 -> concrete 111 |
| `summary mission proximity scan` ships | 0 -> between 1 and 55 x 18 (the live list-6 ships per scan) | 0 -> between 1 and 111 x 32 |
| records created | 0 -> the distinct live ships, at most 18 | 0 -> at most 32 |
| hits, expiries, the two effect rows | 0, absent | 0, absent |
| every other native row, per-entity rows, death rows, gunnery and summary lines | identical, zero clock offset | identical |

The ignored counters are `ship avoidance search refills` and `pretranslate`.

## 12. Part 3 pairs and verdict

One tree, 37ac200de, with `local\bin\sp_off` against `local\bin\sp_on`. The two builds differ
only by the switch. Both variables were set. All four logs show the fit line, the module
directory in this tree and the final COM release. The USN04 OFF run ended with a present
failure (`exit_code=1`, `presents_skipped=7`) after all 4,500 mission frames. That is the
device-lost ending the rules say not to reject, and its mission rows are complete.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| native table rows | 1,537 | 1,537 | 1,438 | 1,438 |
| `WarningManager::scan_proximity` | UNIMPLEMENTED 55 | concrete 55 | UNIMPLEMENTED 111 | concrete 111 |
| ships scanned | 0 | 990 (55 x 18) | 0 | 3,524 (under 111 x 32; ships died) |
| records created | 0 | 18 | 0 | 32 |
| hits, expiries | 0, 0 | 0, 0 | 0, 0 | 0, 0 |
| death rows, gunnery | 41, 727 hits | identical | 22, 440 hits | identical |

**Every prediction held.** A masked whole-log diff leaves only these lines:
- the scan row and its summary line;
- the ignored counters;
- the pre-mission blink alpha;
- USN04 OFF's device-lost ending lines.

**Verdict: ON.** The walk, the tests and the record map are the image's. The squadron arm stays
unreachable until list 24 exists (the contract in section 11).

## 13. Part 7: four gated fan-out bodies (`kGatedFanoutBodiesBound`, committed OFF), and the 00925F20 plan

Packet `cc9_construct_world_p7`. Ghidra was read only. `session+F4h` is `game+1FE4h`
(`docs/SESSION_MESSAGE_DISPATCH.md`). It is 0 in this single-player host, and each body's
single-player arm is what is bound.

| row | body | single-player arm (V) | binding |
| --- | --- | --- | --- |
| 9 | 00778450 `BSP_Session_PumpStep` | 0077845B sets `00F876A1`. The countdown at `+278h` runs only while above 0.0 (00778462). The session constructor arms it to -1.0 (0076EE67, `[00D7A260]`), and only the 0076FE47 arm of `BSP_Session_SetMode` writes 2.0 (`00CE3958`), so its expiry branch (delete `+188h`/`+18Ch`, re-arm) is not taken. It then calls `[00F8A2FC]->vtable[5Ch](step)` at 007784F6. With `+F4h` clear it calls **0076C600 at 00778542**, the loopback-queue drain. **Correction:** `docs/FIXED_STEP_FANOUT.md` row 9 omits this else arm. | concrete. The countdown is held at -1.0. The two calls are named records: `Session::global_object_step_00f8a2fc` (the object is not built here) and `Session::drain_loopback_queue_0076c600`. The only loopback messages this host posts are the kind-8Fh pass-side ones. The ship-AI host delivers those at the start of its controller step (`kShipPassSideMessageBound`), which is the same order: after the pump, before the world tick. |
| 10 | 0077EC20 `BSP_Replication_ApplyPendingEntityCreates` | 0077EC23 `CMP [00F871A8],0`, and `JE` to the return. The list is filled only by 0076C600's entity-create arm (00780670 category 47h). | concrete. No host path posts an entity-create message, so the count is 0. |
| 11 | 00874C90 `BSP_TickRegistry_FlushPendingGroups` | 00874C95: the head `00E0B6D8` equals the sentinel `00E0B704`, and `JE 00874CDE`, which resets the headers. The list is filled only by 00875890. | concrete over the empty list. No host path runs 00875890 (`src/game_hosts_fixed_step.cpp`, fact 2). |
| 13 | 0076FFC0 `BSP_Session_FlushOutboundStep` | 0076FFC3 `CMP [ESI+F4h],0`, then 0076FFCA `JE` to the restore of `00F876A1`. Every outbound call, and the end-of-mission request `BSP_Game_RequestState(10h)`, sits under it. | concrete: nothing to send. |

**Outputs and their readers.** In single player the only output is the loopback drain. It
delivers queued local messages to their entities through 00780670, and the host's one message
kind is already delivered by the ship AI. No other body produces anything. So nothing new can
reach gameplay.

**Predictions** (written before the pairs; the same tree, switch only, both variables set):

| row | USN04 4700/4500 OFF -> ON | USN02 9200/9000 OFF -> ON |
| --- | --- | --- |
| `FixedStepFanout::pump_session`, `apply_pending_entity_creates`, `flush_tick_registrations`, `flush_outbound_session` | UNIMPLEMENTED 4,500 each -> concrete 4,500 | 9,000 -> concrete 9,000 |
| `Session::global_object_step_00f8a2fc`, `Session::drain_loopback_queue_0076c600` | absent -> 4,500 records each | absent -> 9,000 each |
| `FixedStepFanout::init_pending_entities` | 4,500 records, unchanged | 9,000, unchanged |
| `summary fixed step body` concrete / records | 40,500 / 27,000 -> 58,500 / 9,000 | 81,000 / 54,000 -> 117,000 / 18,000 |
| every other native row, per-entity rows, death rows, gunnery and summary lines | identical, zero clock offset | identical |

### 00925F20 `SEntity_InitAll`: read and plan (not bound)

**What it does** (`__fastcall void(char)`, body 00925F20..0092638A). It is guarded by the count
`00F899D4` and walks the circular pending-entity list at `00F899D0` (node next `+0h`, prev `+4h`,
entity `+8h`):
- **Pass A**, 00925FC4..00926062: the name accessor `[vt+10h]`, progress through 0057C1A0, then
  **`[vt+9Ch]`** at 0092604E. That is the Lua `thisTable` attach, 00928A00 by default. It is the
  only call through slot 39 in the image.
- **Pass B**: **`[vt+A0h]`** at 0092610A.
- **Pass C**: **`[vt+A4h]`** at 00926194, then the spawn-descriptor start branch. With the
  descriptor `entity+C0h` of kind 2 and byte `[[+C0h]+8h]+3Ch`, it sets `entity+5Ch` and calls
  `[vt+68h]` or `[vt+6Ch]` with 0. Then it walks the `+48h`/`+44h` child chain through 00922F30
  or 00922F80 with 1.
- **Pass D**: `[vt+5Ch](2)` gates 0077F090.
- **Pass E**: it destroys `entity+C0h` through its slot 0 with 1 and nulls it.
- **The end.** The list is emptied and the count zeroed at 00926335..00926365. `00F899A5` is set
  across passes B and C.

**Callers (rel32 census, 15 sites).**
- The fixed-step row 12 at 00875EA2.
- `BSP_Game_RunExtraFixedStep` at 00874D79, reached from the Lua Spawn, GenerateObject and
  LaunchAirBaseSlot bindings.
- Synchronous calls after:
  - the scene file read: 0046EB4B, 0046EB88, 0046EBC6, 0046ED0F;
  - `BSP_SceneDatabase_CreateEntityByName` (0046DBE8, the GenerateObject path);
  - `BSP_SceneDatabase_CreateHiddenEntityAt` (0046DEDD);
  - `BSP_SpawnRequest_CreateMembers` (0094879A);
  - 00467370, 0046B730 (twice), 00898610, 0089E3C0 and 00927610.

So the fixed-step row finds work only for entities built inside a step without one of those
calls. Examples are the planes of a squadron that an air-ops launch or 007F4580 spawns during the
world tick, and projectiles that are entities.

**What the host does today.** `src/game_hosts_lua.cpp` covers pass A alone, at creation time:
- `GenerateObject` calls `attach_created_entity_00928a00` right after
  `create_unit_from_scene_record_0046db4b` (about line 2153). The image does the same through
  0046DBE8 -> 00925F20.
- `attach_wing_member_tables` (about line 2641) attaches each wing member of an air-ops squadron
  at creation. The image does it at the next row 12, in the same fixed step.

Passes B to E have no host counterpart at all. What `[vt+A0h]`, `[vt+A4h]`, the start branch and
0077F090 do per class is unread.

**Where the two differ.**
- **Order.** The image runs A over the whole list, then B, then C. The host runs A per entity as
  it is made.
- **Timing.** An air-ops wing is attached at 006C5050 in the host. The image attaches it at the
  next row 12 of that fixed step, at most one step later.
- **Coverage.** B to E are missing.

**Units it would affect in the two missions** (`local\cw_on_*.log`):
- **USN04:** the 4 air-ops launches (Lexington-class01 and Yorktown-class01, sqn01 to sqn04, 3
  wings each). The 16 `GenerateObject` squadrons and the 16 SpawnNew units already get the
  synchronous call in the image, so for them only passes B to E would add anything.
- **USN02:** no launch, no GenerateObject squadron, no SpawnNew unit, so only B to E on the
  load-time scene entities. Those were already initialised by the scene-read calls, so the row
  would find an empty list.

**Plan for the successor** (packet `cc9_seentity_initall`):
1. Read `[vt+A0h]`, `[vt+A4h]` and the start branch for the plane, squadron and ship classes.
   Read 0077F090.
2. Give the host a pending-entity list, the image's `00F899D0`. A unit enters it at creation
   (the units host's `create_units`, owned by cc9-plane-release) and leaves it at InitAll.
3. Run 00925F20 at all four call routes: fixed-step row 12 in `src/game_hosts_fixed_step.cpp`
   (free); GenerateObject and SpawnNew in `src/game_hosts_lua.cpp` (free); air-ops launch in
   `src/game_hosts_script_orders.cpp`. Retire `attach_created_entity_00928a00`'s creation-time
   call and `attach_wing_member_tables` in favour of pass A.
4. Pairs: USN04 4700/4500 and E2, where launches and GenerateObject happen. USN02 would stay
   identical. Predict the `thisTable` attach counts unchanged (pass A), and each B to E arm by
   class.

## 14. Part 7 pairs and verdict

One tree, 1d223d54c, with `local\bin\gf_off` against `local\bin\gf_on`. The two builds differ
only by the switch. Both variables were set. All four logs show the fit line and the final COM
release.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| native table rows | 1,536 | 1,538 (+2, the named records) | 1,438 | 1,440 |
| the four bound rows | UNIMPLEMENTED 4,500 each | concrete 4,500 each | UNIMPLEMENTED 9,000 | concrete 9,000 |
| `Session::global_object_step_00f8a2fc`, `Session::drain_loopback_queue_0076c600` | absent | 4,500 each | absent | 9,000 each |
| `summary fixed step body` concrete / records | 40,500 / 27,000 | 58,500 / 9,000 | 81,000 / 54,000 | 117,000 / 18,000 |
| death rows, gunnery | 41, 727 hits | identical | 22, 440 hits | identical |

**Every prediction held.** A masked whole-log diff leaves only these lines:
- the bound rows and their summary;
- the ignored counters;
- the pre-mission blink alpha;
- in USN02, the harness's launcher slot and affinity lines (the two runs took different slots).

**Verdict: ON.** Rows 9, 10, 11 and 13 run their single-player arms. Row 12, 00925F20, stays
the record for the successor packet planned in section 13.
