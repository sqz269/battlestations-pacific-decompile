# The world object behind construct_world 004DE610: layout, fillers, readers and a binding plan

Addresses: 004DE610, 004CB030, 00481640, 00875E69, 00925F20, 00926BE0, 007C9770, 007F4BA0; read only 009037F0, 009258F0, 00928860,
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

**Superseded in part by section 17** (packet `cc9_sentity_init_all`): the air-ops launches are
attached synchronously at 0089E613, USN02 does call GenerateObject, and the pass bodies are read
there.

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

## 15. Part 8: the traffic walk and 00959450's world byte

Packet `cc9_construct_world_p8`. Ghidra was read only.

### The traffic groups (`kTrafficWalkBound`, committed OFF)

**What they are.** `game+21D0h` is the TrafficConfig (004A43C0), loaded by 0049D690 from
`Scripts\datatables\TrafficGlobals.lua`: its soldier and vehicle party pairs
(`docs/TRAFFIC_CONFIG.md`). `+8h` is one 20h group object, vtable 00CE6600, with a `std::list`
whose sentinel is at `+10h` and whose count is at `+14h`.
- **The walk.** 00487270, the group's slot 1 (V), goes from the sentinel's next to the
  sentinel. For each node it calls the payload `+8h`'s slot 1 with the delta. It returns at
  once when the list is empty (0048728C).
- **The producer.** The only one is the scene-traffic commit. The `traffic {}` block of a
  `.scn` is read by `BSP_SceneFile_ReadTrafficBlock` 009514B0 into records. 004A5620 then
  builds one 170h runtime per record through 004A50D0, with `[manager+8h]` as an argument, and
  keys it in the map at `manager+10h` (`docs/SCENE_TRAFFIC_BLOCK.md`).
- **Where items exist.** 251 scene files carry a traffic block, with 172 items between them.
- **The two missions.** Both blocks are empty:
  - `usn_19_coralus.scn` (USN04) line 2567: `traffic { }`;
  - `usn_2_java.scn` (USN02) line 1760.
- **Scripts.** The Lua natives that also touch `game+21D0h` (`TempAddFloatsam` 008C5B80,
  `TempAddAnim` 008C5CD0, `RemoveTempAddedStuff` 008C5E20) are not called by either mission
  script. The shared helper `commandhelpers.lua` uses `TempAddAnim` at lines 4380 and 17110, and
  neither run logs it.
- **So:** no unit and no player ship is ever on a traffic list in these missions. The walk
  visits nothing.

**The binding.** 00481640's call becomes the walk itself, done as `TrafficConfig::group_walk_00487270`,
over an empty list. **Substitution:** the host does not load the traffic block
(`SceneContents::load_traffic_block` 009514B0 stays a record), so the walk is exact only for a
scene whose block has no item. A scene with items needs the commit 004A5620 first. That belongs
to the scene-contents owner, `src/game_hosts_scene_contents.cpp`.

### 00959450 `BSP_Unit_OnDestroyed` and the world byte

**The body** (V):
1. It clears `unit+520h` and runs 00878990.
2. **The report gate.** The clock must be above 1.0 (00959468, strictly), `unit+70h` must be 1,
   which is the `KillReason` harm cause that 00929800 publishes (`docs/KILL_CREDIT.md`), and
   `world+4ACh` must be set (0095948A).
3. **The report.** It walks the `+48h`/`+44h` children and calls `vt[1E8h](0)` on each one of
   kind 20h. Then comes **009813A0 on the warning manager `[00F8A0C4]`**: for a ship or a
   squadron (kinds 6, 18h) of side 0 or 1, it builds the loss report through
   `BSP_Unit_LossCountingSlot`, the radio "we lost X" warning. That call is skipped when the
   controller at `unit+538h` answers kind 17h with `unit+C41h` set, or kind 6 with `unit+100Ah`
   set. Then **0091BDA0**, the kill credit, runs **in every reported case** (00959519).
4. **The controlled unit.** If the unit is the controlled one (004B4B00), and the byte is set
   again (00959537) and the front-end state is not 29h, 2Bh, 2Ch or 2Dh, it runs the limbo page
   00565FB0 and the limbo interface or retarget (`src/mission_result.cpp`).

**Correction to the host model.** `unit_death_00959450` had the two reports as alternatives: the
kill warning, or the credit when the flags were set. The listing runs the credit always and
skips only the warning. The model is fixed in `src/mission_result.cpp` and
`include/bsp/mission_result.hpp`. The existing test still holds.

**Who calls it in the image** (rel32 census):
- 007473E3, 0074DC29 (in 0074DB70), 007BCAD9 (`BSP_Plane_OnDestroyed` 007BCAA0) and 00825271
  (`BSP_UnitInstance_OnWrecked` 00824B60);
- four vtable slots: 00CF8C84, 00CFCDDC, 00D0B7EC and 00D1A714.
- **0091BDA0's only caller in the whole image is 00959519.**

**The host's route misses a step.** The gunnery host's death path (`src/game_hosts_gunnery.cpp`,
about lines 6780-6845) goes from `Death::entity_kill_00926d90` to `Death::unit_sink_008110f0`,
and then calls `kill_credit_record_unit_kill_0091bda0` directly. That skips 00959450's frame:
- the report gate (clock above 1.0, cause 1, the world byte, which is now 1);
- the kind-20h child pass;
- the **009813A0 loss warning**;
- the **limbo page** for the controlled unit.

In these runs the gate would pass for every credited death: the first hit lands after 90 s, and
every credited death is a harm death.

**Contract for the gunnery owner (cc9-gunnery2, `src/game_hosts_gunnery.cpp`).** Route the
sunk and wrecked unit through `bsp::unit_death_00959450` with these inputs:
- `mission_clock`;
- `unit_party_70` = 1 for a harm death;
- `world_gate_4ac` from the mission frame's `world.world_gate.enabled`;
- `is_controlled_unit`;
- the controller flags for `owner_allows_alternate_report`.

Then act on the decision:
- `reports_alternate` goes to the existing `kill_credit_record_unit_kill_0091bda0`;
- `reports_kill` goes to the warning manager's loss report 009813A0, a new host method in
  `src/game_hosts_mission_frame.cpp`, which is free;
- `registers_limbo_page` goes to the HUD's `limbo_screen_take_unit` 00565FB0.

**Predictions for that binding** (for the successor, not run here):
- the credit count stays 41 and 22;
- a new loss-report row appears for each allied or Japanese ship and squadron death;
- gameplay is identical unless a mission script reacts to the loss report.

### Predictions for `kTrafficWalkBound` (written before the pairs; the same tree, switch only, both variables set)

| row | USN04 4700/4500 OFF -> ON | USN02 9200/9000 OFF -> ON |
| --- | --- | --- |
| `TrafficConfig::group_walk_00487270` | UNIMPLEMENTED 4,500 -> concrete 4,500 | 9,000 -> concrete 9,000 |
| every other native row, per-entity rows, death rows, gunnery and summary lines | identical, zero clock offset | identical |

The `mission_result` correction has no caller, so it cannot move a row.

### Part 8 pairs and verdict

One tree, f7c11bf4e, with `local\bin\tw_off` against `local\bin\tw_on`. The two builds differ
only by the switch. Both variables were set. All four logs show the fit line and the final COM
release.

| row | USN04 OFF -> ON | USN02 OFF -> ON |
| --- | --- | --- |
| native table rows | 1,540 = 1,540 | 1,442 = 1,442 |
| `TrafficConfig::group_walk_00487270` | UNIMPLEMENTED 4,500 -> concrete 4,500 | 9,000 -> concrete 9,000 |
| death rows, gunnery | 41, 727 hits, identical | 22, 440 hits, identical |

**Every prediction held.** A masked whole-log diff leaves only the ignored counters and the
pre-mission blink. **Verdict: ON.**

## 16. Handoff (cc9-side-ai retires after this packet)

**State reached on the world object** (sections 9 to 15, all ON on main once this lands):
- **The world-active byte `+4ACh`** is set at the `construct_world` load step. It opens the
  fan-out gate and 00481640.
- **ScanProximity 00977990** is bound over list 6. The part-count test and list 24 are labelled
  stand-ins.
- **Fan-out rows 9, 10, 11 and 13** run their single-player arms. The pump's `[00F8A2FC]` step
  and its loopback drain are named records.
- **The traffic walk 00487270** runs over an empty group list. That is exact for scenes with an
  empty `traffic` block.
- **The 00959450 model** in `src/mission_result.cpp` is corrected: the credit is always run,
  and only the loss warning is gated.

**Open items for a successor, with owners as of 2026-09-27, taken from `bsp.py lease list` just
before this commit:**

| item | what | files and owners | where it is written up |
| --- | --- | --- | --- |
| SEntity_InitAll 00925F20 | a pending-entity list, the image's 00F899D0, fed at unit creation, with InitAll at the four routes: fixed-step row 12, GenerateObject/SpawnNew, the air-ops launch and RunExtraFixedStep. Passes B to E (vt+A0h, vt+A4h with the start branch, 0077F090, the descriptor destroy) must be read first for the plane, squadron and ship classes. It retires the creation-time `attach_created_entity_00928a00` and `attach_wing_member_tables` in favour of pass A. | `src/game_hosts_fixed_step.cpp`, `src/game_hosts_lua.cpp` and `src/game_hosts_script_orders.cpp` are unleased. `src/game_hosts_units.cpp`, where `create_units` is, is leased to cc9-plane-release (cc9_units_contracts). | section 13; pairs USN04 4700/4500 and E2, where launches and GenerateObject happen |
| the 00959450 route | route sunk and wrecked units through `unit_death_00959450`. That adds the loss warning 009813A0 and the limbo page 00565FB0 before the existing credit. | `src/game_hosts_gunnery.cpp` (cc9-gunnery2's file, unleased right now); a new warning-manager method in `src/game_hosts_mission_frame.cpp`, which is free | section 15, the contract |
| the scene traffic block | load `traffic {}` items (009514B0) and commit them (004A5620 -> 004A50D0), so the walk has elements in the scenes that author some (172 items over 251 files) | `src/game_hosts_scene_contents.cpp`, unleased | section 15; `docs/SCENE_TRAFFIC_BLOCK.md` |
| ScanProximity stand-ins | `unit_part_descriptor_count` and 007F10B0's push into list 24 | `src/game_hosts_units.cpp`, cc9-plane-release; the lead routed both contracts | section 11 |
| the pump's two records | the object at `[00F8A2FC]` (its `vtable[5Ch]` step) and 0076C600's drain, once more loopback kinds exist than 8Fh | `src/game_hosts_fixed_step.cpp` (free), `src/game_hosts_ship_ai.cpp` (cc9-gunnery2) | section 13 |

**Parts 4 to 6** (section 7 plan) belong to other workers:
- **Part 4, the neighbour count 009EEB8B.** It landed on main at cd9264a68, merging cc9-gunnery2.
- **Part 5, the ring-scan probe (ranking rows 9 to 11).** It is with cc9-gunnery2. It reads the
  avoid-zone manager `[00E17624]`, not the world.
- **Part 6, the AvoidZoneLayer sample 0041BC20 (row 15).** It is with cc9-plane-release. It
  reads the avoid-zone registry `[00E17620]`, not the world. Main now has
  `src/game_hosts_avoid_zones.cpp` from that line of work.

**Not done and not planned here:**
- the world's `+Ch..+14h`, `+4A4h` and `+4A8h` meanings;
- the 0047F130 getter's extent;
- list 28 and list 71 fillers;
- the other `+4ACh` readers' routines (section 9 table), none reached by the host.

## 17. Part 9: 00925F20 `SEntity_InitAll` bound (`kSEntityInitAllBound`, committed OFF)

Packet `cc9_sentity_init_all`, worker cc9-world-init, 2026-09-27, base main b7fc4773d. Ghidra was
read only. The switch is `kSEntityInitAllBound` in `include/bsp/game_hosts_fixed_step.hpp`.

### What the passes do, per class (V)

The slot bodies were read from the PE's vtables. The slot-5Ch column is the pass D gate.

| class (vtable) | A `+9Ch` | B `+A0h` | C `+A4h` | `+5Ch`(2) |
| --- | --- | --- | --- | --- |
| Battleship, Cruiser, Destroyer, Cargo (`00CF90B0`, `00CFB738`, `00CFC3D0`, `00CFA778`) | 00810F60 | 00822C20 `UnitInstance_SEntityInit` | 0081F980 `ShipUnit_BindSectionPoints` | true |
| Mothership (`00D01630`) | 00810F60 | 007593D0: 00822C20, then the air-ops deck load (docs/AIROPS_LOAD_FROM_SCENE.md) | 00758210: 0081F980, then `+11A8h = 1` | true |
| plane squadron (`00D087C0`) | 007F4580: attach, then it constructs the wing | 007F1FE0 `PlaneSquadron_BeginOrderSpeed` | 007F4BA0 (no Ghidra function): a property-bag reader over the kind-1 holder | true |
| Fighter, DiveBomber, TorpedoBomber, SmallRecon (`00D06920`, `00D19D28`, `00D1A000`, `00D0BA80`) | 007CDF20 | 007D5D20 `Plane_ReadPropertyBag` | 007C9770 (no Ghidra function), see below | true |
| Airfield (`00CF8C08`) | 006D0C80 | 006D3C10 runway reader | 006D5220 hangar and marker reader | true |

- **Plane pass C, 007C9770..007C985F.** It calls 0095E5B0, 00951F80, 007C95A0 and 007BC550. When
  `plane+900h` is 0 or 1 it disables the plane (00922F80 with 0). When `plane+9D4h` is set, it
  writes `thisTable[plane].SquadronID` = that object's `+174h` id (string `SquadronID` at
  00D05B80, 00927B40 then 00B67460). It ends with 007C5AC0(-1.0f, from 00D7A260). No script of
  USN04 or USN02 in this installation reads `SquadronID`. Three other mission scripts do.
- **The start branch** needs holder kind 2 at `+4h`. Scene creation stores the kind-1
  property-bag holder (00922E20, vtable 00D03D94). The kind-2 objects are session messages of
  vtable 00D03754. Only two routines install that vtable with kind 2: 007673B0, from
  00768530 `SessionMessage_CreateFromBitStream`, and 00774DC0, from 00774E30. 00774E30 runs only
  with session mode `+F4h` 1 or 2 (00774E4B..00774E59). **The branch cannot fire in single
  player.**
- **Pass D.** Slot 5Ch is an is-kind-of test: 006DFE90, 007DDA80 and 007EFB00 all accept 2. The
  body 0077F090 returns at 0077F0A4 unless `[00E188A8]` is set and its `+1FE4h` is 1. It is 0 in
  single player: this process's `LobbySettings` line says `game+1FE4h = 0`. **Pass D does nothing here.**
- **The enable/disable virtuals** 0077D500 and 0077D580 (slots 68h and 6Ch, shared by every class
  above) build a type-52h entity message with the byte 1 or 0 and post it through 0077C7B0.
- **Pass E** destroys the kind-1 holder too. After InitAll, `entity+C0h` is null for every class,
  so the scene property bag is gone once the passes have read it.

### Corrections to section 13 and the walker

| was | is | evidence |
| --- | --- | --- |
| the four USN04 air-ops launches are attached "at the next row 12, at most one step later" | all four launches start inside the `LaunchSquadron` call, and 0089E3C0 calls InitAll right after 006CC690, at 0089E611 `XOR CL,CL` / 0089E613 `CALL 00925F20`. Only a launch the deck tick 006CDC70 starts waits for row 12. | listing; `tw_off_usn04.log`: `LaunchSquadron ... (started)` four times |
| "USN02: no GenerateObject" | USN02 calls GenerateObject 4 times (Nachi, Sazanami, Naka, Ushio, all DestroyerGen) | `tw_off_usn02.log` native table and lines 9182..9243 |
| `sentity_init_all_00925f20` walks a snapshot, "no pass observed here adds a node" | every pass reloads the sentinel and follows `[ESI]`, so nodes appended during a pass are walked by it. The squadron's pass A (007F4580) constructs planes, and the base constructor pushes each one (00928760 `CALL 00926BE0`, the locked push_back). | 00925FC6, 00926060; 00928630's tail |
| passes B and C use the denominator and step of pass A | 00926067 re-reads the count. 0092607A/0092607E then set the step to the new count and the denominator to three times it. | the three pushes before those stores |
| the fixed-step row "only catches mid-step creations (air-ops launches ...)" | in this process no route leaves a node for row 12 on these missions. Row 12 would catch a deck-tick launch, and any unit the units host makes without a route (contract below). | this section |

### The binding

- **The pending list** is the Lua host's `pending_entities_`, the counterpart of 00F899D0. Pass A
  is the Lua host's `thisTable` attach, so the list lives with it. `push_pending_entity_00926be0`
  is the 00926BE0 push.
- **Squadrons.** `push_pending_squadron_00926be0` marks a plane squadron. Its pass A appends the
  wing units to the tail, as 007F4580's plane constructions do in the image. This process has made
  those units already, in create_units.
- **The routes** (all under the switch):

| route | native site | host site |
| --- | --- | --- |
| GenerateObject | 0046DBE8 inside 0046D930, then 00874D79 from 00945311 (list already empty) | `run_generate_object_00944fd0` in `src/game_hosts_lua.cpp` |
| SpawnNew | 0094879A, once after 009483D0's member loop | `fulfil_spawn_request_009483d0` |
| LaunchSquadron | 0089E613 | `run_launch_squadron_0089e3c0` |
| air-ops squadron creation | none (006C5050 only pushes) | `create_squadron` pushes; no attach |
| fixed-step row 12 | 00875EA2 | `GameFixedStepHost::init_pending_entities_00925f20` |

- **The pass bodies:**
  - Pass A calls `attach_created_entity_00928a00`, unchanged.
  - Passes B and C are named records: `SEntity::InitAll pass B init_slot_a0` (00926110) and
    `pass C init_slot_a4` (009261A1). Parts of them already run at creation in files this packet
    does not own. create_units runs 00822C20's StartSpeed arm and the wake-ring fill. The gunnery
    host runs 0081F980's section binding. The scene-contents host runs the carrier deck load.
  - Pass E is the named record `pass E release_spawn_holder` (00926317).
  - The start branch and pass D are exact for single player and are logged as implemented.
  - The loading-bar call 0057C1A0 is a record.
- **OFF** keeps today's creation-time attach on every route.
- **How the bound order differs from today:**
  - **SpawnNew.** A group's member squadrons are all attached before any wing plane. Today each
    squadron is followed by its own wing.
  - **Air-ops launches** are attached at the end of the `LaunchSquadron` call rather than inside
    006C5050. There is no Lua between the two points.
  - **A launch the deck tick starts** (a queued one) is attached at the next row 12. The host
    runs the deck tick 006CDC70 from the mission frame's script timers, outside the fixed step
    (`src/game_hosts_mission_frame.cpp`, `run_script_timers`). So that attach can come one frame
    later than in the image, where the tick runs inside the step. Neither mission queues a launch.
  - **GenerateObject** of a `PlaneSquadronGen` would now attach its wing too. Today it does not.
    Neither mission does this.
  - **Load time** is unchanged. The scene read's four InitAll calls (0046EB4B, 0046EB88, 0046EBC6
    and 0046ED0F) stay with the mission frame's `attach_scene_entities_00928a00`.

**Contract for the units host (cc9-plane-release, `src/game_hosts_units.cpp`):** create_units
should call `push_pending_entity_00926be0` once for each instance it constructs, which is the
00928760 push. When that lands, the pushes in the Lua routes are deleted, and the squadron's wing
append in pass A is replaced by the planes' own construction-time pushes.

### Predictions for `kSEntityInitAllBound` (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500.**
  - InitAll runs 4,512 times: 4,500 at row 12, 8 at SpawnNew and 4 at LaunchSquadron.
  - 12 calls find work. They walk 60 entities: 20 pushed squadrons plus 40 appended wing planes.
  - Every row-12 call finds an empty list.
  - The row `FixedStepFanout::init_pending_entities` goes from UNIMPLEMENTED 4,500 to concrete
    4,500, so the fixed-step summary moves 4,500 from records to concrete.
  - New rows:
    - pass A attach, 60 calls;
    - pass B, pass C and pass E records, 60 each;
    - the start branch and pass D, 60 each;
    - loading progress, 180.
  - Unchanged: `self_table_entities=86` and `wing_member_tables=40`.
  - **Gameplay identical.** That covers deaths, the per-entity death table, gunnery hit records
    and releases. Every script callback sees the same tables at the same point. The only order
    change is attach order inside a SpawnNew group, and no USN04 script iterates `thisTable`.
- **E2 = USN04 9200/9000.** Row 12 runs 9,000 times, and the other counts are the same as the
  USN04 pair. Gameplay identical.
- **USN02 9200/9000.** InitAll runs 9,008 times: 9,000 at row 12, and 4 GenerateObject calls with
  two sites each. 4 calls find work, over 4 entities with no wing. `self_table_entities=34`.
  Gameplay identical.
- **If any gameplay line moves, this binding changed something it should not have, and the
  switch stays OFF.**

### Part 9 pairs and verdict

**The first USN04 pair failed one prediction, and this packet caused it.** It ran at 8aeb67bad,
`local\bin\ia_off` against `ia_on`. Pass A appended 56 wing planes instead of 40, which gave
`self_table_entities` 102 instead of 86 and `wing_member_tables` 56 instead of 40. The squadron
append took every unit from the squadron's first unit up to the unit count at pass A. SpawnNew's
InitAll runs after the whole member loop, so for member 1 that range also held member 2 and its
wing: 8 groups, each with an extra leader and one wing plane. Nothing else moved. Deaths, hits
and releases were identical, and the only other moved line was the ignored refills counter.
70b4afc41 records each squadron's own end when it is pushed. The pairs below are from that tree.

**The pairs.** One tree, 70b4afc41: `local\bin\ib_off` against `local\bin\ib_on`, differing
only by the switch. `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` were set for every run.
All six logs show the fit line, the final COM release, and a module directory inside this tree.

| row | USN04 OFF | USN04 ON | E2 OFF | E2 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| native table rows | 1,568 | 1,576 | 1,570 | 1,578 | 1,469 | 1,477 |
| row 12, `init_pending_entities` | UNIMPLEMENTED 4,500 | concrete 4,500 | UNIMPLEMENTED 9,000 | concrete 9,000 | UNIMPLEMENTED 9,000 | concrete 9,000 |
| InitAll calls / with work / entities | 0 | 4,512 / 12 / 60 | 0 | 9,012 / 12 / 60 | 0 | 9,008 / 4 / 4 |
| wing planes appended / pushes | 0 | 40 / 20 | 0 | 40 / 20 | 0 | 0 / 4 |
| fixed-step concrete / records | 58,500 / 9,000 | 63,000 / 4,500 | 117,000 / 18,000 | 126,000 / 9,000 | 117,000 / 18,000 | 126,000 / 9,000 |
| `self_table_entities`, `wing_member_tables` | 86, 40 | identical | 86, 40 | identical | 34, - | identical |
| deaths, hit records | 41, 743 | identical | 51, 843 | identical | 22, 439 | identical |
| dive-bomb / torpedo releases | 3 / 5 | identical | 3 / 5 | identical | - | - |

- **The eight new rows** are InitAll and seven per-pass rows. Each per-pass row counts one call
  per entity; the loading-progress row counts three.
- **The masked whole-log diff** (heap pointers and thread ids masked) leaves only:
  - the rows and summaries above;
  - the per-call `INIT,ENUM` notes;
  - the ignored refills counter;
  - in E2, the sound-startup line `fmod_calls` 131 against 132. It is logged before the window
    exists, long before the first InitAll call, so it is not this switch.
- **Every prediction held** on this tree. The one failure, on the first tree, is recorded above.

**Verdict: ON.** `kSEntityInitAllBound` is set true. Passes B, C and E stay named records, and
their per-class bodies are the successor's.

## 18. Part 10: the proximity scan's two stand-ins retired (`kScanProximityUnitsEntriesBound`, committed OFF)

Packet `cc9_scan_units_entries`, worker cc9-world-init, on main 297fcf7fe. The units entries
come from cc9_units_contracts (a3c7096e1). Ghidra was read only.

**What 00977990 tests, per list-24 node (V, 00977A25..00977AF9).**
- **Where each test reads.** The walk starts at `[world+13Ch]` and steps through the node's
  `+4h`. The node's entity (`+8h`) is the squadron:
  - its own four live bytes (00977A36..00977A58);
  - its own side against the ship's (00977A5E..00977A64);
  - `00803CE0(ECX = ship side, EDX = squadron) == 1` (00977A6A..00977A74).
- **Position.** Only the position comes from the flight leader `[squadron+3D0h]` (00977A76),
  after its pose refresh 00414DB0.
- **Distance.** The squared distance is `(dx*dx + dy*dy) + dz*dz` on the x87, with each
  difference stored to a float first. The sum is stored to a float at 00977AD8 and compared
  against the double 4.0e6 at 00D09FE8. The old host code summed the other way round in float,
  which could only matter at the rounding edge.
- **The part test** is 009779E7..00977A03: `+348h` non-null and `(+34Ch - +348h) >> 2` not below
  2.

**The binding** (`src/game_hosts_mission_frame.cpp`):
- The part test becomes `unit_part_descriptor_count(ship) < 2`, which rejects the ship.
- List 24 is `world_list_entry(24, i)`. The side, live bytes and rating are read on that unit.
- The position comes from `squadron_list_24_leader(unit)`.
- **Substitution:** the image reads `[+3D0h]` unguarded, but when the host answers "none alive"
  the node is skipped and counted in `list24_no_leader`.
- The switch off keeps the two stand-ins. The summary line gains `units_entries`,
  `part_rejects`, `list24_nodes` and `list24_no_leader`.

### Predictions (written before the pairs; the same tree, switch only, both variables set)

- **Part test.** It passes for every ship: the plane-release census counts 18/18 and 32/32 ships
  with more than one section. So `part_rejects=0`, and `ships` and `records` are unchanged
  against the OFF run of the same tree.
- **USN04 4700/4500.**
  - `list24_nodes` goes from 0 to a positive count. List 24 holds up to 21 nodes, pushed at
    0.00 s, 25.1 to 30.1 s and 105.1 to 106.6 s. It is walked for each scanned ship until the
    first hit.
  - `hits` goes above 0, because Japanese strikes close to within 2 km of Allied ships and are
    identified. `expiries` may rise with them.
  - `list24_no_leader` is small, 0 until a squadron loses every plane.
- **USN02 9200/9000.** No squadron registers on list 24 in this scene, so `list24_nodes=0`,
  `hits=0` and the whole log is identical apart from the summary fields.
- **Gameplay identical on both.** The hit arm only spawns a render-side point effect and holds
  a host record that nothing else reads. Deaths, hit records and releases do not move. The new
  `WarningManager::proximity_effect_0096c070` record calls appear only when hits do.

### Part 10 pairs and verdict

One tree, 718254fe0: `local\bin\sc_off` against `local\bin\sc_on`, differing only by the
switch. Both variables were set. All four logs show the fit line, the final COM release, and a
module directory inside this tree.

| field | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| ships scanned, records | 990, 18 | 990, 18 | 3,524, 32 | 3,524, 32 |
| `part_rejects` | 0 | 0 | 0 | 0 |
| `list24_nodes` | 0 | 10,576 | 0 | 0 |
| hits, expiries | 0, 0 | 444, 16 | 0, 0 | 0, 0 |
| `list24_no_leader` | 0 | 0 | 0 | 0 |
| native table | 1,581 rows | +2 records: `proximity_effect_0096c070` 444, `proximity_effect_stop_00867b10` 16 | 1,481 rows | identical |
| deaths, hit records | 41, 743 | identical | 22, 439 | identical |

**Every prediction held.** The masked whole-log diff leaves only the summary line, the two
record rows on USN04, and the ignored refills counter.

**Verdict: ON.** `kScanProximityUnitsEntriesBound` is set true.

## 19. Part 11: the loss warning 009813A0 bound past its guard (`kLossWarningBound`, committed OFF)

Packet `cc9_loss_warning`, worker cc9-world-init, from the plan in `docs/LOSS_WARNING.md`.
Ghidra was read only.

**Read for this packet (V).**
- **00976F10 `CancelByTarget`** (00976F10..00976FF0, `RET 4`). The argument is the unit pointer:
  0098167A `PUSH EDI`, with EDI the unit since 009813BF.
  - It walks the `+E0h` pending list. A record is destroyed through its slot 0 with 1, unlinked,
    and the count at `+E8h` dropped, when its `vt[10h]()` kind is 4 and its `+80h` is the unit.
  - It then stores `now + 2.0` (`[00F876A4]` plus the double 00D7A308) under the unit in the map
    at `+15Ch` (00975C40). The readers of that map are unread.
- **The ship arm, 0098168D..009816C5.** IsKindOf 6, then three calls on manager+184h, which is
  the per-ship proximity record map the scan 00977990 keeps:
  - 00975D00 finds the record;
  - 0096AE90 stops a live effect (00867B10, effect+9 = 1), releases it and nulls it;
  - 00975E30 erases the record.

**The binding** (`include/bsp/game_hosts_mission_frame.hpp`, `src/game_hosts_mission_frame.cpp`,
and one hunk in `src/game_hosts_gunnery.cpp`). With the switch on:
- The death route calls `game_warning_report_loss_009813a0` in place of its record. That is plan
  step 5; the gunnery file was unleased, and its loss counters are unchanged.
- **Bound:** past the guard, the cancel runs over `warning.pending` and stamps the `+15Ch` map. A
  ship's proximity record is erased.
- **Named records:** the text post (`loss_text_005cf3d0`), 006E6670, the `kill` channel
  (`kill_channel_0097b8c0`) and the teardown.
- **Substitution:** no player slot is resolved, so the text key is counted by side,
  `warn_uslost` or `warn_japanlost`.
- The host keeps no channel-subscription store, so the plan's listener count cannot be given.

### Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN02 9200/9000.**
  - 22 entries: every death reaches the entry.
  - The guard passes 22 times, as `loss_side0=12 loss_side1=10`; the text keys are 12
    `warn_uslost` and 10 `warn_japanlost`.
  - `cancelled=0`: no host path queues a pending warning, so the list is always empty.
  - `proximity_erased=22`. Every ship that dies has been scanned at least once, since deaths come
    after 27 s and a scan runs every 4 s, so each has a record. A dead ship is never scanned
    again, so `records` does not move.
  - Native table: the gunnery row `WarningManager::report_loss_009813a0` (22 records) goes. The
    entry row (22 concrete) and six rows, 22 each, come in: text, cancel, erase, 006E6670, kill
    channel and teardown.
- **E2 = USN04 9200/9000.**
  - 51 entries with 0 guard passes: every loss is an aircraft, which fails IsKindOf 18h and 6.
  - The gunnery record row (51) is replaced by the entry row (51 concrete), and nothing else
    moves.
- **Both.** Gameplay identical: deaths, the per-entity death table, hit records and releases.
  Nothing the cancel or the erase touches is read by gameplay, and the Lua listeners stay
  records.

### Part 11 pairs and verdict

One tree, cd0197e41 plus the scan verdict e5bc4dc3d: `local\bin\lw_off` against
`local\bin\lw_on`, differing only by the switch. Both variables were set. All four logs show the
fit line, the final COM release, and a module directory inside this tree.

| field | USN02 OFF | USN02 ON | E2 OFF | E2 ON |
| --- | ---: | ---: | ---: | ---: |
| entries, guard passes (side 0 / 1) | record 22 | 22, 22 (12 / 10) | record 51 | 51, 0 |
| text keys `warn_uslost` / `warn_japanlost` | - | 12 / 10 | - | 0 / 0 |
| `cancelled`, `proximity_erased` | - | 0, 22 | - | 0, 0 |
| proximity `records` | 32 | **54** | unchanged | unchanged |
| native table | 1,481 | 1,487 | 1,585 | 1,585 (record row replaced by the entry row) |
| deaths, hit records | 22, 439 | identical | 51, 843 | identical |

**One prediction failed: USN02 `records` 32 -> 54.**
- **The wrong premise.** The prediction assumed a dead ship is never scanned again. In this
  process it is: `ships` scanned is 3,524 in both runs, so a sunk ship still passes the scan's
  four live bytes. Each of the 22 erased records was created again at the next scan.
- **The binding is not at fault.** It does what 0098168D..009816C5 do.
- **Open question.** Does the image's dead ship still pass `+5Ch` set and `+5Dh`/`+60h`/`+5Eh`
  clear when the next scan runs? The kill routine 00922FD0 clears `+5Ch`, but it is reached from
  00903670 `EntityWorld_FlushActivations` and a destructor, not from the death route 00959450.
  So when a sunk ship leaves the scan in the image is not established.
- **Effect.** Gameplay is unaffected: the records feed only the render-side proximity effect.

**Everything else held.** The masked whole-log diff leaves only:
- the rows and summaries above;
- the ignored refills counter;
- in E2, the pre-window sound-startup `fmod_calls` 129 against 130, which was also seen in the
  InitAll E2 pair.

**Verdict: ON.** `kLossWarningBound` is set true. The text post, 006E6670, the `kill` channel's
Lua listeners and the teardown stay named records, so USN02's own failure path runs once.

## 20. Scene traffic commit: read and plan (packet `cc9_scene_traffic_commit`, not bound)

Worker cc9-world-init. Ghidra was read only. `src/game_hosts_scene_contents.cpp` is leased to
cc9-scene-entities (cc9_landscape_terrain, until 2026-09-27T15:57Z), so nothing is bound here.
The call site is written below as a contract.

### 004A50D0, the 170h runtime's constructor (V, listing 004A50D0..004A5438, `RET 8`)

`__thiscall(this = the 170h block, record, group)`. The group is `[manager+8h]`, pushed by
004A5620.
1. **Base and identity.** 004A4B70(record, group) is the base constructor, 1.3 KB and unread.
   Then vtable 00CE68E4 and `+14Ch = group` (004A510A).
2. **The path box.** The path is `[+20h]`, and its point range is `min(+24h, +28h)` to
   `max(+24h, +28h)` (004A5110..004A512B).
   - Point i comes from 007AF800 on the path; the first one seeds both corners.
   - 00427D10 grows the box `+150h..+164h` over the rest (min at `+150h`, max at `+15Ch`).
   - The box is then widened by the extent vector at `+138h..+140h`: subtracted from the min,
     added to the max (004A519D..004A51FF).
3. **The nearest anchor.**
   - `+168h = 0`. For each child of `[+D8h]+48h` along `+44h` that answers IsKindOf 1Ch
     (004A5229), the child's world pose is refreshed and copied into `+CCh..+108h` when stale.
     That uses 00414DB0, or 00413920 with the parent `[+3Ch]+CCh`, and sets `+C8h = 1`.
   - The child's smallest squared distance to any path point is taken. The x87 order is
     `(dx*dx + dy*dy) + dz*dz`, stored to a float, with the running minimum seeded from
     00CE4970.
   - `+168h` becomes the child with the smallest distance, if that is below the seed 00CE6A04.
4. **The state machine.** `+16Ch = 0`, then 00487470; `+16Ch = 1`, then 00487470 again. It ends
   at `+16Ch = 3` (004A53FC..004A541E). 00487470 is 0.75 KB and unread.

### What the TrafficConfig walk calls

- **The runtime's vtable 00CE68E4** reads, from the PE:
  - slot 0: 004A5440, the deleting destructor;
  - slot 1: 0049A360;
  - slot 2: 004969D0;
  - slot 3: 0048A650.
- **The walk.** 00487270 calls slot 1 with the delta on each node's payload `+8h`, so a
  committed runtime is ticked through 0049A360.
- **What 0049A360 does** (0049A360..0049A404, from the pseudocode only, which drops a branch):
  - it picks a time scale against `[+18h]` and `[00F876A4]` minus the double 00CE6840, and
    stores it at `+130h`;
  - it advances `+84h` by delta times that scale;
  - it calls slot 3, 0048A650, then 00496BD0.
- **Size.** That is the traffic simulation itself (spawning and moving along the path), and it
  is unread. With 004A4B70 and 00487470 it is several packets, not one.
- **Is the runtime on the walked list?** Whether the runtime puts itself on the group's walked
  list (`group+10h`) is not established. It is either the base constructor 004A4B70 or 00487470.
  That is the first thing the next read must settle, because it decides whether committing a
  runtime makes the walk tick it.

### The measuring mission

38 scenes in this installation author 172 traffic items (`local/traffic_census.py`, the same
tokenizer walk as `docs/SCENE_TRAFFIC_BLOCK.md`). Among the mission-tree ids:
- **JM01**, "Vanilla - Attack on Pearl Harbor": `ijn/JM/ijn_01_attack_on_pearl_harbor.scn`, 5
  items, file dated 2024-07-13 like the untouched bulk. This is the recommended measuring
  mission.
- **USNRM01**: `usn/USNRM/usn_1_pearl.scn`, 5 items, but modified 2024-10-29.
- **BSM02**: `bsm/bsm_02_defense_of_the_philippines.scn`, 10 items, modified 2024-08-03.

USN04 and USN02 author an empty block, so they can only show identity.

### Contract for the scene-contents owner (`src/game_hosts_scene_contents.cpp`)

The scene-contents host keeps `SceneContents::load_traffic_block` 009514B0 as a record. The hunk,
once the runtime exists as its own reconstruction:

```cpp
// SceneContents, where 0046DF00's pass-2 `traffic` arm runs (0046EB3B..0046EBDD):
// 009514B0 parses the block into the 4Ch records (docs/SCENE_TRAFFIC_BLOCK.md), then
// 004A5620 builds one 170h runtime per record through 004A50D0 with [manager+8h].
void load_traffic_block_009514b0(const SceneTrafficBlock& block);   // the reader
void commit_traffic_runtimes_004a5620();                            // the commit
```

Both would live under a switch in that file, committed OFF. On USN04 and USN02 the pairs show
identity. JM01 is the measuring run, once 0049A360's tick is bound.

### Plan

1. Read 004A4B70 and 00487470. Settle whether the runtime joins `group+10h`.
2. Read 0049A360, 0048A650 and 00496BD0 from the listing. The pseudocode drops a branch at
   0049A3xx.
3. Put the runtime in new files, `src/scene_traffic_runtime.cpp` and its header, registered in
   `cmake/startup.cmake` (append-only), so that the lease on the scene-contents file does not
   block it.
4. Bind the commit through the contract above. Pair USN04 and USN02 for identity, and run JM01
   ON against OFF as the measuring run.

## 21. Part 12: a sunk ship's live bytes and 00903670 (`kSunkShipFlushBound`, committed OFF)

Packet `cc9_sunk_ship_flush`, worker cc9-world-init, on main fe93be072. Ghidra was read only.

### When the image's dead ship leaves each walk (V)

| step | when | site | what |
| --- | --- | --- | --- |
| 1 | the death step, at the damage (row 6 or 14) | 00958A30 -> `vt[70h](1)` = 0077D1A0 -> 00926C80 | Destroy. It does nothing when `+60h` is already set (00926CBF). Otherwise it sets **`+60h = 1`** (00926CD5) and the cause `+70h` (1, or the parent's). With the argument, it passes `vt[70h](1)` to each child whose `vt[78h](this)` agrees. It then appends to the destroy list 00F899A8 (00926D33..00926D66). |
| 2 | the same step, row 15 | 009273A0 -> `vt[74h]` = 00926390 | **`+5Dh = 1`** (0092639B) and `+60h = 1` (0092639E), then the wreck handler `vt[7Ch]` 00824B60 (docs/UNIT_DEATH_MESSAGE_AND_SINK.md) |
| 3 | every step while `+5Dh` is set | 00825F20 `UpdateShipMotion` (ECX = unit+310h, so `[EDI-2B3h]` at 008263C1 is `+5Dh`) | `sinkTime +828h` (`[EDI+518h]`) grows by the delta. After 60.0 s (double 00CE3D68), or for kind 0Eh after 20.0 s (00CE3930), the hull shapes lose flag 8 (00826410..0082643B). |
| 4 | once the hull is wholly below KillDepth | 008265EC..00826628 | `y +/- up.y * 0.5 * [[desc+228h]+A0h]` both below GameSettings `+3F4h` = `VizbeomlesDolgok.KillDepth` = **-200.0** in this installation (`shipglobals.lua` line 377) -> Kill 00926D90(1): `+5Fh`, the kill list |
| 5 | the next row 15 | 009273A0 -> 009263C0 | the removal stores `+5Dh`, `+5Eh`, `+5Fh`, `+5Ch = 0` |
| 6 | the next frame's world post-tick | 00903670 (from 004E5382 `OnMove`, 004D7EDE / 004D7EFC) | for a node with `+5Eh` set, `+6Ch` clear and no removed parent: 00922FD0 sets `+5Eh`, `+5Dh`, `+5Ch = 0`, `+6Ch = 1`, recurses into `+48h` along `+44h`, and tail-jumps `vt[84h]` (ship 00819880) |
| 7 | three expiry passes later | 00903610 | the entity is freed. That is where it leaves the world lists (contract: unread) |

- **The four-byte live test** (0043F080; 00977990's own copy) fails from **step 1**: `+60h` is
  set at the damage.
- **The neighbour walk** 009EEB8B..009EEBDA has **no** live test. It checks only the list entry,
  IsKindOf 6 and not-itself. So in the image a wreck stays in the neighbour count until it is
  freed at step 7, after sinking below -200 m. The label on `kShipAiNeighbourCountBound` ("the
  host's list 6 never drops a unit") is right about this process, but the image drops the wreck
  only at step 7, not at death.
- **Correction to `docs/UNIT_DEATH_MESSAGE_AND_SINK.md`.** `+828h` (`sinkTime`) is advanced. It is
  written through the sub-object base `unit+310h` (displacement 518h), which a search for the
  displacement `28 08 00 00` cannot see.

### What this process did before this packet

- **Ships:** no step at all. The gunnery death route kills the ship in its own state, and the
  units host keeps `+5Ch` set and `+5Dh`/`+60h` clear. The scan 00977990, the gunnery validity
  tests, the ship AI's neighbour-view flags and the recon union all read a wreck as alive.
- **Planes:** steps 1 and 2 already happen, in the units host (packet cc9_plane_death_flags).
- **The flush 00903670** ran over an activation vector nothing fills. The old reconstruction
  names `+5Eh` "spawned" and `+6Ch` "activated", but 00922FD0's stores show they are the removed
  byte and the killed flag.

### The binding

With the switch on:
- **Steps 1 and 2, for ships.** The row-15 flush (`GameStepSubsystemsHost`,
  `src/game_hosts_ready.cpp`) applies `+60h` and `+5Dh` through the units host's stores to each
  ship the gunnery host holds dead. The wreck handler is a record. **Substitution:** the gunnery
  death route cannot call Destroy (the file is leased to cc9-scene-entities). So `+60h` lands at
  row 15 instead of at the damage, which is the same step whenever the damage is applied before
  row 15.
- **Step 6.** 00903670 runs over the unit slots, from both of its host sites. The kill stores the
  bytes, and the step-subsystems expiry counter is `+6Ch`. `vt[84h]` is a record.
- **Contracts:**
  - **Gunnery death route** (for its next owner): call the Destroy entry at the kill, in place of
    the row-15 substitution.
  - **Units host** (cc9-plane-release): steps 3 and 4. `sinkTime` and the KillDepth kill in the
    ship's motion update feed the kill list. Unlink from the world lists at the free (step 7).
    Until then no unit reaches step 6, so the bound 00903670 finds nothing on these missions.

### Predictions (written before the pairs; the same tree, switch only, both variables set)

- **E2 = USN04 9200/9000.** All 51 deaths are aircraft, and planes already carry both bytes, so
  `ship_destroy_flags=0` and `kills=0`. **The whole log is identical** apart from the new rows and
  summary lines: the post-tick row `MissionCompletion::world_post_tick` goes from record to
  concrete.
- **USN02 9200/9000.**
  - `ship_destroy_flags=22` and `kills=0`.
  - Proximity `records` go from 54 back to **32**. `ships` scanned falls, because a dead ship
    leaves the scan the step it dies.
  - The run is identical up to the first death, DeRuyter at 30.25 s. After it:
    - the gunnery validity tests and the ship AI's avoidance and recon reads drop wrecks;
    - the neighbour count is **unchanged** by this binding, since its walk has no live test;
    - the setback and station-keeping rows that read the neighbour view's `+5Dh`/`+60h` flags may
      move.
  - **Bands:**
    - mission failure at 39.65 s within +/-5 s (Exeter at 35.95 s is the trigger);
    - deaths 22 +/-4, direction up or flat, since guns stop taking wrecks as live targets;
    - hit records 439 within +/-15%, direction up or flat;
    - the per-entity death rows identical up to 30.25 s.
- **If the neighbour count moves**, the binding reached something outside the prediction and the
  switch stays OFF until that is explained.

### Part 12 pairs and verdict

One tree, 50a713e32: `local\bin\ss_off` against `local\bin\ss_on`, differing only by the
switch. Both variables were set. The diff is `tools/pair_diff.py`. All four logs show the fit
line, the final COM release, and a module directory inside this tree.

**E2 = USN04 9200/9000: identical** (pair_diff exit 1, "gameplay identical"). The pair has 52
deaths and 875 hit records on both sides, and the death rows, plane death modes and unit table
are all identical. Only the two `bound` fields moved. The predicted post-tick row change did not
appear: the native table did not move at all, because `MissionCompletion::world_post_tick` was
not reached in these runs.

**USN02 9200/9000: gameplay moved** (pair_diff exit 3).

| row | OFF | ON | predicted |
| --- | ---: | ---: | --- |
| `ship_destroy_flags`, `kills` | 0, 0 | 20, 0 | 22, 0 (one per ON death: held) |
| proximity `records`, `ships` scanned | 54, 3,524 | **32**, 2,283 | 32, falling: held |
| mission failure | 39.65 s | 39.65 s | +/-5 s: held |
| first hit, first death | 30.25 s | 30.25 s | identical to 30.25 s: held |
| deaths | 22 | 20 | 22 +/-4, up or flat: **direction failed** |
| hit records | 411 | 329 | +/-15%: **failed (-20%)** |
| shots | 759 | 807 | - |
| controlled DeRuyter distance moved | 2,897 m | 316 m | not predicted |
| ship AI steps (gated) | 252,000 (0) | 152,134 (99,866) | not predicted |

**What moved, and why.** None of it was predicted: I read the scan and ship-AI consumers of the
bytes, and not the rest. Every path below is a host consumer that already gates on the bytes the
image writes in the death step.
- **The ship AI stops stepping a dead ship.** 99,866 brain steps are gated. OFF, a wreck kept
  planning and steering: the player's DeRuyter, sunk at 30.25 s, travelled 2,897 m afterwards.
- **The units host takes the +5Dh-gated wreck branch** of its ship-motion reconstruction.
  `UnitInstance::wake_setting 00424c40` is reached 99,866 times: that is 008265FA's GameSettings
  read for KillDepth.
- **The mission script retargets.** New `command target` lines, such as Yamakaze -> Alden and
  Tokitsukaze -> Exeter, appear once a target reads as dead.
- **Guns stop landing on wrecks.** Shots rise and hit records fall. The gunnery validity tests
  at 0043F080 now reject a wreck. OFF, rounds were landing on dead hulls; the per-hit share
  cannot be read from this log.
- **Deaths.** Harusame, Jintsu and Haguro survive, and Witte dies. Twelve death times move, from
  Kortenaer at 70.30 s onward.

**Verdict: ON.** Every moved path is a consumer reading the state the image has from the death
step (00926CD5, 0092639B). The OFF behaviour, with wrecks sailing, planning and absorbing rounds,
is the deviation. The failed bands are recorded above. The neighbour count's walk has no live
test, so wrecks stay in it in both builds, as in the image until the KillDepth kill.

## 22. Part 13: Destroy at the kill in the gunnery death route (`kDeathRouteDestroyBound`, committed OFF)

Packet `cc9_death_route_destroy`, worker cc9-scene-entities, on main 1c9730a29. It takes up the
section 21 contract for the gunnery death route.

**The hunk.** `GameGunneryHost::Impl::kill_unit` (`src/game_hosts_gunnery.cpp`), after the sink
record and the kill credit and before the `unit_death_00959450` model:
- For a ship (kind 6) whose `+60h` is clear, it stores `+60h = 1` through the units host
  (`store_pending_destroy_0060`). That is 00926C80's store at 00926CD5, behind its 00926CBF
  already-set test, reached as 00958A30 -> `vt[70h](1)` = 0077D1A0.
- It counts `death_route_destroys` and prints
  `summary mission gunnery death route destroys=.. bound=..`.
- **What stays with the flush.** The cause `+70h` and the destroy-list append stay with the row-15
  flush (`src/game_hosts_ready.cpp`). The flush still sets `+5Dh` through 00926390. Its
  already-delivered test (`torn_down && pending`) does not skip a ship whose `+60h` alone is set,
  so its count is unchanged.
- **Planes are untouched.** They already get both bytes in the units host
  (`kPlaneDeathFlagsBound`).

**Who reads `+60h` between the kill and row 15 in the same step?**

*In the image:*
- the damage, and so the death step, is at row 6 or row 14 (the two 00926700 drains, 00875E44 and
  00875EC4);
- a row-14 kill leaves no reader before row 15 (00875EC9);
- a row-6 kill leaves rows 7..14. There, the Lua-call drain 00888230 (00875E55) and the entity
  think pass 00929460 (00875E64) run scripts and bot thinks, which may test the four-byte gate
  0043F080. So in the image those can see the kill one flush earlier.

*In this process:*
- the kill happens inside the units host's fixed step (job wave 2), in the gunnery pass's projectile
  and damage-control steps, before `run_subsystems_00875e0c` runs rows 1..15;
- **no host consumer of `+60h` runs in that window**:
  - the gunnery pass's own `unit_alive_and_visible` users (the contact sweep at 3172 and the recon
    sensor pass at 3896) run before the projectiles in the same step, and also test the gunnery
    `dead` flag;
  - the ship AI's controller step (with the torpedo warning 00977690 at 009DA90E) and the AI host's
    step run before the gunnery pass;
  - `publish_ai_weapon_facts` reads hit points and barrels only;
  - the remaining units-host loop only stores `+60h` for planes;
  - the host's think list (row 8) and Lua-call drain (row 7) are empty;
  - the proximity scan 00977990 and the HUD read in the frame, after the step, when the flush has
    run.

### Predictions (written before the pairs; same tree, switch only, streams and the death table on)

- **USN02 9200/9000.**
  - `death route destroys` = 0 OFF and **20** ON: every death on this base is a ship.
  - `ship_destroy_flags` = 20 on both sides, unchanged in count.
  - **Gameplay identical:** 20 deaths, every death row, the unit table and the native table apart
    from the new `Death::destroy_00926c80` row (20 calls, ON only). pair_diff exit 1.
- **E2 = USN04 9200/9000.** Every death is an aircraft, so destroys are 0 on both sides and
  `ship_destroy_flags=0`. Identity on every row: 52 deaths, 875 hit records. The only difference
  is the summary's `bound` field.

### The pairs, measured

- **Builds.** One tree (`agent/cc9-scene-entities` at `f63932d2e`, which is main `1c9730a29`
  plus this packet), built twice with only the switch flipped:
  - `local\dd_off`, SHA-256 prefix `6BDE629DE937`;
  - `local\dd_on`, SHA-256 prefix `2364BEBBF34A`.
- **Logs.** `local\dd_{off,on}_{usn02,e2}.log`. Each shows the 1600x900 override and its own
  module directory, and each exited 0.

**`tools/pair_diff.py`, USN02 9200/9000: exit 1, gameplay identical.**

```
GAMEPLAY: identical
  deaths                                 20                                       20
  hit records                            329                                      329
  hull hits                              167                                      167
  damage                                 59663.8                                  59663.8
  shots                                  807                                      807
  first hit                              30.25 s                                  30.25 s
  torpedo-task releases                                                           
  dive-bomb-task releases                                                         
  torpedo drops                          0                                        0
  plane water contacts                                                            
  controlled moved                       DeRuyter 316.14                          DeRuyter 316.14
  units                                  32                                       32
  mission end                            failed at 39.65 s (Mission.EndMission) text="Game Over" e... failed at 39.65 s (Mission.EndMission) text="Game Over" e...
* host methods concrete/unimplemented    979 / 519                                980 / 519
DEATH ROWS: identical (20 rows)
PLANE DEATH MODES: identical (0 rows)
UNIT TABLE: identical (32 rows)
```

- **Destroys.** `death route destroys` is 0 OFF and 20 ON. `ship_destroy_flags` is 20 on both
  sides, as predicted.
- **Native table.** It gains `Death::destroy_00926c80` (20 calls). **Counts moved on 20 GUI text
  rows**, for example `GuiText::find_font` 1216 -> 1214, and the summary's `text` line changed.
  - The two OFF-only lines are `Unit_name_Text` and `Distance_Text`, just after
    `death row: victim=DeRuyter t=30.25`. DeRuyter is the controlled ship, killed in one hit.

**`tools/pair_diff.py`, E2 = USN04 9200/9000: exit 1, gameplay identical.**

```
GAMEPLAY: identical
  deaths                                 52                                       52
  hit records                            875                                      875
  hull hits                              345                                      345
  damage                                 13618.3                                  13618.3
  shots                                  6092                                     6092
  first hit                              93.00 s                                  93.00 s
  torpedo-task releases                  4 of 16                                  4 of 16
  dive-bomb-task releases                1 of 19                                  1 of 19
  torpedo drops                          1                                        1
  plane water contacts                   19                                       19
  controlled moved                       Lexington-class01 6017.22                Lexington-class01 6017.22
  units                                  81                                       81
  mission end                            none (Mission.EndMission never true)     none (Mission.EndMission never true)
  host methods concrete/unimplemented    1043 / 548                               1043 / 548
DEATH ROWS: identical (52 rows)
PLANE DEATH MODES: identical (52 rows)
UNIT TABLE: identical (81 rows)
```

The native table is identical, destroys are 0 on both sides, and the only difference is the
summary's `bound` field.

**Failed prediction: the window.**
- **The claim** was that no host consumer reads `+60h` between the kill and row 15.
- **What the host actually does.** The units host's fixed step, which runs the gunnery pass and so
  the kills, is `motion_step_00825f20`. That is called **after** `run_subsystems_00875e0c` in the
  same step (`src/game_hosts_mission_frame.cpp`, the `FixedStepBinding::run_step_subsystems` body),
  so a host kill lands after that step's row-15 flush.
- **The consequence.** With the switch OFF, a dead ship keeps `+60h` clear through the frame's HUD
  pass until the next step's row 15. The HUD markers' live test
  (`HudMarkers::is_alive_and_visible`, 0043F080, `src/game_hosts_hud_world.cpp`) drew DeRuyter's
  name and distance for one more frame. ON matches the image: its Destroy sets `+60h` at the
  damage, before any frame.
- **The same window** holds for any frame-level reader, such as the proximity scan 00977990 and
  the minimap. Only the marker texts moved on USN02.

**Verdict: ON.** Gameplay is identical on both missions. The one presentation difference moves
the host toward the image, and the destroy count is unchanged.


## 23. Contract: the pending-list push moves to create_units (packet `cc9_units_push_pending`, held)

2026-09-27, worker cc9-units2, on main 10b1b7043. No code. The packet is held until the list side
below lands (the lead's decision). Then the units host's push becomes the one push the image
makes: the base constructor 00928760 pushes every entity onto 00F899D0 through 00926BE0.

### Three facts (USN04 4700/4500, `local\MR_ON_USN04.log` in worktree cc9-units2)

The host makes 81 units through three callers of `GameUnitsHost::create_units`.

1. **Scene load** (`src/game_hosts_mission_frame.cpp`, the instantiate step) makes 21. They never
   go on the pending list: the mission frame's `attach_scene_entities_00928a00` stands in for the
   scene read's four InitAll calls (0046EB4B, 0046EB88, 0046EBC6, 0046ED0F). A push from
   create_units would leave them pending, and the first row 12 (00875EA2) would attach them a
   second time.
2. **The script routes** (`GameScriptOrdersHost::create_unit_from_scene_record_0046db4b` for
   GenerateObject and SpawnNew, and the air-ops squadron creation) make the other 60: 20
   squadrons (each fused with its leader plane) and 40 wing planes. Each Lua route pushes its
   entity AFTER create_units returns, and a squadron's pass A appends its wing. The summary line
   reads `pushes=20 wing_appended=40 entities=60`. A push from create_units cannot see the
   route's later push, so it would double all 60.
3. **Wing order.** The host constructs the wing planes in the create_units batch. The image
   constructs them inside the squadron's pass A (007F4580), so their pushes land at the tail after
   the squadron is attached. A construction-time push today would put them BEFORE their
   squadron's pass A, a worse order than the wing-append substitution gives.

So no guard inside `src/game_hosts_units.cpp` alone can make the push both real and single.

### The design (list side: cc9-init-passes; units side: cc9-units2)

**List side, in `src/game_hosts_lua.cpp`, first:**
- `push_pending_entity_00926be0` skips a push whose entity id is already pending or already
  attached (`init_all_attached_`).
- `push_pending_squadron_00926be0` upgrades a pending plain node with the same id: it sets
  `squadron`, `units_before` and `units_end`, and does not append.
- Pass A's wing append skips ids already pending.
- The route pushes (GenerateObject, SpawnNew, LaunchSquadron, the air-ops `create_squadron`) are
  retired.
- Wing construction moves into the squadron's pass A, so each wing plane is constructed, and
  therefore pushed, where 007F4580 constructs it.

**Units side, after that lands:** `create_units` calls `push_pending_entity_00926be0` once for
each instance it constructs, as 00928760 does. A squadron's leader goes as a plain node, and the
squadron upgrade marks it. Scene-load instances stay out until the mission frame's
`attach_scene_entities_00928a00` is retired in favour of the scene read's InitAll calls. That is
a second contract, for the mission frame's owner.

**The guard while both sides push** is the list's dedup by entity id. The image pushes once, and
the dedup keeps the list at one node per entity whichever side pushes first.

**Predictions to carry** (USN04 4700/4500, USN02 9200/9000): pushes per creation route equal to
the instances each route constructs, `self_table_entities` unchanged (86 / 34), InitAll rows
unchanged in count, identical gameplay.

## 24. The KillDepth kill and the world-list unlink (packet `cc9_sunk_ship_kill_depth`, `kSunkShipKillDepthBound`, committed OFF, ON since the pairs)

2026-09-27, worker cc9-units2, on main 10b1b7043. Ghidra was read only. This is the units-host
contract of section 21 (steps 3 and 4, and the unlink).

### What the image does (V)

| step | site | what |
| --- | --- | --- |
| sinkTime | `008263C1` `CMP [EDI-2B3h],0` (EDI = unit+310h, so unit+5Dh); `008263CE`..`008263DC` | `sinkTime` +828h (`[EDI+518h]`) += dt |
| shapes | `008263E2`..`0082645A` | past 60.0 s (double `00CE3D68`), or 20.0 s for kind 0Eh (`00CE3930`), each hull shape loses flag 8 (`00C47F60`) |
| the hull ends | `00826467`..`008265F6` | half = (float)(`[unit+538h]`+A0h × 0.5, double `00D7A280`); the ends are y (unit+100h) ± (float)(forward.y (unit+F0h) × half) |
| the test | `008265FA`, `0082660F` | `00424C40` `BSP_GameSettings_GetSingleton`, +3F4h. FCOMIP then JBE, so each end must be strictly below KillDepth |
| the kill | `00826628` | `00926D90` Kill(1): +5Fh, Destroy when +60h is clear, the kill list `00F899B4`. Then `0092BD30` on the controller (`00826633`) and `RET 4` (`00826642`): the rest of the tick does not run |
| the drain | next row 15, `009274A1`..`0092751F` | `009263C0` (skipped when +5Eh is set), then `vt[80h]` |
| on killed | ship `vt[80h]` = `00951FB0` for all five ship vtables (`00CF9130`, `00CFB7B8`, `00CFC450`, `00CFA7F8`, `00D016B0`) | +4A4h = 0, `00779AF0` tail-jumps to `00928C80`; at `00928F1C`..`00928F2C`, when unit+30h is set, `vt[134h]` |
| the unlink | `vt[134h]`: Battleship `006E0060`, Cruiser `006FB510`, Destroyer `006FE670`, Cargo `006EB460`, Mothership `00758FE0` | each calls `006DFFC0` (lists 5 and 6, heads parent+58h and +64h), which calls `006D3620` (lists 1, 2 and 4 via `00928570`, parent+28h, +34h, +4Ch), then erases from its class list (Battleship +B8h = 13, Cruiser +94h = 10, Destroyer +70h = 7, Cargo +A0h = 11, Mothership +88h = 9). Each erase (`004837D0`) takes the first node whose payload is the unit. This mirrors the +130h registrar's lists exactly |

**Correction to section 21, step 7.** The world-list unlink is not at the free. It is at the
on-killed dispatch of the next row 15 (`00928F2C`), three expiry passes before the free. The
free (`00903610`) only destroys the object. `vt[134h]` has no direct caller
(`tools/callsite_census.py 006e0060`: only the vtable entry `00CF91E4`). The one call through
`[reg+134h]` on an entity is `00928F24` (`scan-bytes '?? 34 01 00 00'`, page of 567 rows; the
other `MOV reg,[reg+134h]` hits are not entity code).

### The binding

- **In the ship motion path** (`motion_step_00825f20`, before `ship_motion_step_00825f20`),
  for a ship with +5Dh set: sinkTime, the flag-8 record past 60 s, the hull-end test, and at a
  pass the kill (+5Fh, +60h when clear). The tick then ends, as at `00826642`.
- **SUBSTITUTION, labelled:** the host's row-15 kill list cannot take a host entity
  (`copy_pending_lists_00926fa0` throws in `src/game_hosts_ready.cpp`). So the removal
  `009263C0` and the on-killed unlink `00928F24` are applied at the kill, one row 15 early.
- **SUBSTITUTION, labelled:** KillDepth is the constant −200.0, this installation's
  `scripts/datatables/shipglobals.lua` line 377 (2024-07-13), because GameSettings+3F4h is not
  loaded into this host (its reader `0083EA71` belongs to the Lua host's settings load).
- **The unlink** erases the first node of the unit in every world list, which for a ship is the
  six lists its registrar joined.
- **Then:** the mission frame's `00903670` (bound ON in section 21) finds +5Eh set and marks +6Ch;
  the expiry `00903610` releases it three passes later. `src/game_hosts_ready.cpp` now stores −1
  in the released slot's counter. The image takes the entity out of the chain, while the host's
  chain is the slot order. Without the −1 the resumed walk would meet the same slot again past 3
  and release it forever.
- **A census in both builds:** every wreck's first +5Dh time, its lowest hull end, its last y,
  and the summary `sunk ship kill depth`.

### Predictions (written before the runs)

The host's wreck has no descent model. The sink `008110F0` is a record (22 UNIMPLEMENTED calls
on USN02 in `local\MT_ON_USN02.log` of worktree cc9-plane-release), no flooding (the leak
manager +10D4h) is bound, and the hydrodynamics `009329C0` keeps every element's buoyancy
(`submerged_steps` = `element_steps`). So:

| row | USN02 9200/9000 | E2 = USN04 9200/9000 |
| --- | --- | --- |
| `wrecks` (census, both builds) | 20, the ships the row-15 flush marks, first +5Dh at their death times | 0 (every death is an aircraft) |
| lowest hull end | above −50 m on every wreck, both builds | - |
| when each wreck passes −200 m | **never** within the 450 s run | - |
| `kills`, `unlinked_nodes` | 0, 0 | 0, 0 |
| list 6 at the end | unchanged (32) | unchanged |
| ship AI neighbour count mean (`traffic setback ... mean_count`) | unchanged | unchanged |
| setback and station-keeping rows | identical | identical |
| deaths, death rows, hit records, failure time | identical (failure 39.65 s) | identical |
| pair_diff exit | 1 (the `bound` field and the `ShipMotion::sunk_hull_shape_flag8` record, which ON reaches after 60 s of sinkTime on each wreck) | 1 |

**If a wreck does pass −200 m** the prediction fails at its first row. Each such wreck then
leaves list 6 at the step it passes (list 6 falls by one per kill, with 6 unlinked nodes each),
the neighbour count's mean falls by about one per kill for the steps after it, and the
setback rows move. Deaths and the failure time stay identical, because a wreck is already dead.

### The pairs, measured

One tree, `ec440301f`: `local\sk_off` against `local\sk_on`, the switch only, both variables set.
Logs `local\SK_OFF_USN02.log` / `SK_ON_USN02.log` and `SK_OFF_E2.log` / `SK_ON_E2.log` in worktree
cc9-units2. All four show the fit line, the immediate present interval, a module directory in
this tree and the final COM release. `tools/pair_diff.py` exits 1 on both pairs: gameplay, the
death rows and the unit table are identical.

| row | USN02 OFF / ON | E2 OFF / ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| `wrecks` | 20 / 20 | 0 / 0 | 20, 0 | held |
| lowest hull end | −47.04 m (Kawakaze, from 113.15 s); Yudachi, Samidare and Murasame −0.25 m; the other 16 at 0.00 m | - | above −50 m | held |
| passes −200 m | none | - | never | held |
| `kills`, `unlinked_nodes` | 0, 0 | 0, 0 | 0, 0 | held |
| list 6 at the end | 32 / 32 | 18 / 18 | unchanged | held |
| neighbour count (`traffic setback ... mean_count`) | 31.97 both | - | unchanged | held |
| station keeping | 208 calls both | - | identical | held |
| deaths, hit records, failure | 20, 329, 39.65 s both | 52, 875, none both | identical | held |
| natives | `ShipMotion::sunk_hull_shape_flag8` 77,003 calls ON; `tests` 0 -> 99,866 | no change | the flag-8 record | held |

The first +5Dh times follow the section-21 death times by the row-15 step (DeRuyter 30.30 s, Java
32.45 s, Exeter 36.00 s), so the census reads the flushed bytes.

**Verdict: ON.** It is the image's rule, and it changes nothing today.

**Open:** host wrecks have no descent. The sink `008110F0` is a record, and neither flooding nor
the leak manager is bound. So the kill, the removal, the unlink and the expiry release have run
in no mission here, and neither has the −1 retirement in `src/game_hosts_ready.cpp`. The first
descent model to land should re-run USN02 and expect each wreck to leave list 6 as it passes
−200 m.

## 25. A wreck's descent: the leak manager and the wreck handler's sink block (packet `cc9_ship_sink_descent`, `kShipSinkDescentBound`, committed OFF, ON since the pairs)

2026-09-27, worker cc9-units2, on main ad2c11cb0. Ghidra was read only. Section 24 left every host
wreck afloat: the lowest hull end on USN02 was −47.04 m.

### How a dead ship descends in the image (V)

- **`008110F0` is not the descent.** It is `BSP_UnitInstance_Sink`, the Lua entry: it refuses on
  +5Dh or on a non-zero invincibility +150h, then Destroy (`vt[70h](1)`), zeroes +828h and +82Ch,
  and releases +740h. It reaches the same wreck handler a damage death does.
- **The water.** The leak manager at unit+10D4h is built by `0074E7B0` (from the vehicle base
  constructor at `0081F15F`): +38h = 200.0 (`00CE386C`), +3Ch = 60.0 (`00CEB4B0`). `0074F490`
  (from `00822C20` at `00823780`) sets it up:
  - count +14h = 6;
  - capacity +0Ch = (float)(`0092BEB0`(parts) / 10.0), where parts+84h = controller+84h is the
    reserve buoyancy `00937DBB`..`00937F74` (sum over the class's elements of
    coefficient × |top − base| × shape × Gravitacio / 10, minus Gravitacio × Mass);
  - rate cap +08h = settings+400h (MaxLeakPercent) × capacity, × 3.0 (`00D7A2B0`) for
    IsKindOf(0Eh), or for IsKindOf(0Ch) with Mass < 500.0 (`00CE3840`);
  - six points: sides −1 then +1, x = Width × side, y = 0, z = (Length / 2) × i − Length × 0.5;
  - +38h/+3Ch take the class DamageToDeath (+54Ch) and TimeToDeath (+550h) when both are ≥ 0;
    +34h = 2 × settings+404h (EnnyiVizEsKeszPercent) × capacity / +3Ch, and the cap is raised to it.
- **At death**, the wreck handler `00824B60` (row 15, `vt[7Ch]`) runs `00824FE5`
  `0074EC50(&unit+10D4h)`. With lobby mode ≠ 2, each leak draws U(0, 1) from `00BD2F10`
  (`ECX = 1`, `0074EC9B`). Each rate becomes `(draw + water) × 2 × cap / (total water + Σdraws) +
  rate`. The rates travel in message 92h (`MT_SHIP_SINK`) through `0077C2A0`, and its arm
  `0082220D` → `0074E860` loads them. Then `00825044` sets the inertia to 2× (`00D7A308`),
  `0082505C` the angular damping to 2.5 (`00CF87C8`), `00825074` the linear damping to 0.5
  (`00CE3800`), and +828h and +82Ch are zeroed.
- **Every hydrodynamic step** (`009329C0`, tail `00933A01`) ticks `0074F930`. Each leak takes
  min(rate, cap) × dt. While +5Dh is clear, the total is capped by (1 − health) × settings+404h ×
  capacity. +5Dh suppresses the cap, so a wreck floods without limit. The total is unit+10FCh.
  It adds to the mass (`00932B72`) and pulls down as a weight of 10 × water (`00933A3A`).
  `0074F2E0` turns each leak's water into a heeling torque about its point.
- **The descent** is the hull body's response. The element buoyancy saturates once an element is
  fully under (`00932DD5` clamps the depth to its span), so a wreck whose water weight exceeds its
  reserve goes down until the vertical drag (`L`/`N` down pair) and the linear damping balance it.
  Then the KillDepth kill of section 24 fires.
- **While alive**, leaks come from message 90h (`MT_SHIP_LEAK`, `008221A7` → `0074F440` →
  `0074F090`). That path is not bound here; see the contract below.

### What the host had

| part | host before this packet |
| --- | --- |
| hydrodynamics `009329C0`, flooding weight, mass | yes, with unit+10FCh |
| leak tick `0074F930`, heel torque `0074F2E0` | reconstructed (`bsp/unit_forces.hpp`), run over an empty list |
| leak manager init `0074F490`, reserve buoyancy controller+84h | missing |
| wreck handler `00824B60` | a record at the row-15 flush |
| leaks from hits (90h) | missing; the gunnery host's flooding (`docs/SHIP_FIRE_FLOODING.md`) is the damage-control task's water seconds at unit+A20h, a different model that damages health and never touches unit+10D4h |
| buoyancy elements | a stand-in: eight elements, draft H/2, solved so the hull displaces its weight at that draft |

### The binding

- **At creation (ON)**, for every hull with an element list: controller+84h over the host's
  element list, then `0074F490`. **Consequence of the stand-in list, labelled:** the eight
  stand-in elements have frac = 0.5 and Ship Kitevo 1.0, and coefficient × draft = Mass × 10 / 6
  each. So the sum is 13.33 × Mass, controller+84h = 3.333 × Mass and capacity = 0.3333 × Mass.
  The real list's value is unknown until the element producer is read.
- **SUBSTITUTIONS, labelled:** settings+400h = 0.02 and +404h = 0.2, from this installation's
  `shipglobals.lua` lines 383 and 381 (2024-07-13). GameSettings is not loaded into this host.
- **At the row-15 flush (ON)**, `src/game_hosts_ready.cpp` calls the units host's
  `ship_wreck_sink_00824fe5`: the redistribution, then inertia, damping and the zeroed floats.
  **SUBSTITUTION, labelled:** the six draws come from the units host's keyed release stream, key
  `<ship>#leak` (so `BSP_GUNNERY_RNG_STREAMS=1` isolates them). The 92h message is applied at
  once.
- **In the hydrodynamics (ON)**, the leak tick and the heel torque run over the six leaks. The
  health argument is 1.0, labelled: the cap reads it only while +5Dh is clear, and a live hull's
  rates are zero here.
- **The census** adds a `leaks` line per wreck and a `ship sink descent` summary, in both builds.

**Contract for the gunnery host (cc9-ships):** the image's live-hull leak is message 90h
(`008221A7`), amount = (float)(uint32)msg+1Ch × 10.0 at a hull point. When the gunnery host binds
the hit that sends it, it calls a units-host entry to be added then, `add_leak_0074f440(unit,
amount, point)`, which transforms the point and runs `0074F090`. Until then a live hull never
floods through unit+10D4h.

### Predictions (written before the runs)

Same tree, `local\sd_off` against `local\sd_on`, the switch only, both variables set.

The rough dynamics, per wreck, capacity C = Mass / 3 and cap = 0.02 × C: the inflow after the
handler is about 2 × cap = 0.0133 × Mass per second. The net downward acceleration is about
0.133 × t m/s², against a hydrodynamic drag of v + 0.3 v² (m/s²) and the linear damping of 0.5.
The quasi-steady speed then reaches about 3.3 m/s at 60 s and 3.8 m/s at 75 s, so a hull reaches
−200 m roughly 75 s after its first +5Dh time.

| row | USN02 9200/9000 | E2 = USN04 9200/9000 |
| --- | --- | --- |
| `leak_models` | 32 ON (every ship), 0 OFF | 18 ON, 0 OFF |
| `redistributions` | 20 ON, one per wreck | 0 |
| each wreck's −200 m time | first +5Dh time + 75 s, band +40..+200 s: DeRuyter ~105 s, Java ~107 s, Exeter ~111 s, Yamakaze ~127 s, Houston ~150 s, Kortenaer ~156 s, Kawakaze ~188 s, Electra ~189 s, Alden ~225 s, John1 ~234 s, Asagumo ~324 s, Perth ~341 s, Witte ~342 s, John2 ~344 s, Jupiter ~344 s, Samidare ~400 s, Yudachi ~411 s, Murasame ~436 s; Encounter (376 s) and John3 (447 s) not within 450 s | none |
| `kills`, `unlinked_nodes` | 18 kills (band 14..19), 6 nodes each | 0 |
| list 6 at the end | 32 -> 14 (band 13..18) | 18 both |
| neighbour count (`traffic setback ... mean_count`) | falls from 31.97 (band 27..31.9) | unchanged |
| expiry release | first run ever: `released` = kills; each slot released once (the −1 guard), no hang | 0 |
| setback and station-keeping rows | move after the first kill (~105 s) | identical |
| deaths, hit records | 20 ± 3, 329 ± 15 %: a live ship's avoidance loses its sunk neighbours after the first kill | identical (52, 875) |
| failure time | 39.65 s, identical (before any wreck floods far) | none, identical |
| alive ships | water 0 on every live hull (no 90h) | water 0 everywhere; gameplay identical |
| pair_diff exit | 3 if any gameplay row moves after 105 s, else 1 | 1 |

### The pairs, measured

One tree, `2e58c646c`: `local\sd_off` against `local\sd_on`, the switch only, both variables set.
Logs `local\SD_OFF_USN02.log` / `SD_ON_USN02.log` and `SD_OFF_E2.log` / `SD_ON_E2.log` in
worktree cc9-units2. All four show the fit line, the immediate present interval, a module
directory in this tree and the final COM release. `tools/pair_diff.py`: USN02 exit 3, E2 exit 1.

| row | USN02 OFF | USN02 ON | prediction | verdict |
| --- | ---: | ---: | --- | --- |
| `leak_models` / `redistributions` | 0 / 0 | 32 / 21 (one per wreck; 21 wrecks ON) | 32 / 20 | held (one more wreck) |
| time from first +5Dh to −200 m | - | 91.6..98.6 s on all 17 (DeRuyter 30.30 -> 127.90 s, Java 32.45 -> 130.95, Exeter 36.00 -> 132.05, Yamakaze 51.70 -> 149.00, Houston 74.60 -> 169.51, Kortenaer 80.80 -> 178.11, Kawakaze 111.20 -> 208.96, Electra 113.80 -> 211.11, Alden 149.80 -> 247.11, John1 158.21 -> 256.11, Asagumo 187.46 -> 285.05, Witte 266.61 -> 363.93, Jupiter 267.21 -> 365.38, Yukikaze 293.20 -> 390.48, Samidare 314.00 -> 407.52, Yudachi 326.29 -> 417.77, Murasame 356.99 -> 448.46) | +75 s, band +40..+200 | held (about 20 s slower than the estimate) |
| not under by 450 s | - | Encounter −160.95 m, Harusame −177.83, Jintsu −104.04, Haguro −35.99 (all late deaths) | Encounter, John3 | held in kind |
| `kills`, `unlinked_nodes` | 0, 0 | 17, 102 (6 each) | 18 (14..19), 6 each | held |
| list 6 at the end | 32 | 15 | 14 (13..18) | held |
| expiry `released` | 0 | 17, each slot once, no hang | = kills | held |
| neighbour `mean_count` | 31.97 | 25.48 | 27..31.9 | **failed**: fell further |
| station keeping | 208 | 209 | moves after the first kill | held |
| proximity scan records / ships | 32 / 2,283 | 32 / 2,263 | unchanged ± a few | held |
| deaths | 20 | 21 | 20 ± 3 | held |
| hit records | 329 | 354 (+7.6 %) | ± 15 % | held |
| failure | 39.65 s | 39.65 s | identical | held |
| first moved death row | - | Kawakaze 113.10 -> 111.15 s (first damage 104.05 -> 102.10) | moves only after the first kill (~105 s) | **failed**: moved before the first kill at 127.90 s |
| controlled DeRuyter distance | 316.14 m | 711.72 m | not predicted | **not predicted** |

E2 = USN04 9200/9000: identical (exit 1). The only change is `leak_models 0 -> 18`: no ship dies,
and every live hull's water stays 0.

**What moved, and why (partly unexplained).** A flooding wreck changes the battle before any kill.
Its hull goes down and heels, the linear damping 0.5 changes its drift, and the dead DeRuyter now
travels 712 m while it sinks. Rounds and neighbour views that met the floating wrecks now meet
sinking ones. Hull hits rise 167 -> 179. Which of these moved Kawakaze's first damage 2 s earlier
was not isolated. After the first kills, the unlinks take the wrecks out of list 6, and
`mean_count` falls to 25.48. Perth, John2 and John3 survive, and Yukikaze, Harusame, Jintsu and
Haguro die.

**Verdict: ON.** A wreck floods and sinks as the image's leak manager and wreck handler make it,
and the kill, the unlink and the expiry release now run. **Caveat:** the descent's rate comes from
controller+84h over the host's stand-in element list (capacity = Mass / 3), so the ~97 s descent is
a property of the stand-in. It will move when the element producer is read.
**Resolved in section 28:** with the image's list the descent is 106..117 s on USN02.

## 26. The pending-list push moves to create_units (packet `cc9_units_push_pending`, `kUnitsPendingPushBound`, committed OFF, ON since the pairs)

2026-09-27, worker cc9-units2, on main af9eab355, after cc9-init-passes' dedup list (docs/
SENTITY_INIT_ATTACH_ORDER.md section 9, 338c8b4e3) and load-time InitAll (section 10,
9db5c5290), both ON. This implements section 23's contract.

**The binding.** `create_units` calls `push_pending_entity_00926be0(index + 1, row name, row
type_id)` right after it stores each slot. That is the base constructor 00928630's push at 00928760 through 00926BE0, once per
constructed instance: a squadron's leader as a plain node, and each wing plane as a plain node.
The routes' pushes and the load walk's pushes stay. The dedup list turns them into:
- skips, for a plain push whose id is pending;
- upgrades, for a squadron push over its leader's plain node;
- a drop of the wing's plain nodes, which pass A re-appends at the tail.

A summary line `units construction push` counts the pushes.

**Order.** At load, `create_units` pushes the scene units in unit order. The load walk then
finds them pending, marks them `load_scene`, and appends the markers after them: the same order
as before. At a route, `create_units` pushes the leader and its wing. The route's squadron push
drops the wing nodes and upgrades the leader. Pass A appends the wing, as before.

**Still a contract (section 9.3, not taken here):** construct the wing from the squadron's pass A
through a units-host call the Lua host makes at `entity_attach_lua_self_vcall_9c`. It is shared
with the script-orders owner. Until then the wing is constructed in the creator batch, and its
construction pushes are dropped in favour of pass A's append.

### Predictions (written before the runs)

Same tree, `local\pp_off` against `local\pp_on`, the switch only, both variables set. OFF
matches section 10.4's ON column.

| row | USN04 4700/4500 OFF -> ON | USN02 9200/9000 OFF -> ON |
| --- | --- | --- |
| `units construction push` | 0 -> 81 (21 scene, 20 leaders, 40 wing planes) | 0 -> 32 (28 scene, 4 GenerateObject) |
| `skipped_pending` | 0 -> 21 (the load walk's unit pushes) | 0 -> 32 (28 at load, 4 at GenerateObject) |
| `squadron_upgrades` / `wing_deferred` / `wing_append_skipped` | 0 -> 20 / 40 / 0 | 0 / 0 / 0 |
| `load_dropped` / `skipped_attached` | 0 / 0 (the load attach is retired) | 0 / 0 |
| InitAll `pushes` | 46 -> 86 (81 construction + 5 markers) | 34 -> 34 (32 construction + 2 NavPoints) |
| InitAll calls / with work / entities / `wing_appended` | 4,513 / 13 / 86 / 40, unchanged | 9,009 / 5 / 34 / 0, unchanged |
| load walk pushes / mirrored | 26 / 5, unchanged | 30 / 2, unchanged |
| `self_table_entities` | 86, unchanged | 34, unchanged |
| InitAll pass rows (A..E, start branch, loading progress) | unchanged in count | unchanged |
| gameplay: deaths, hit records, death rows, unit table | identical | identical |
| pair_diff exit | 1 | 1 |

These differ from section 9.5's forecast (`load_dropped=21`/`28`, `skipped_pending=4`), which was
written before the load walk replaced the load attach.

### The pairs, measured

One tree, `7edcd3102`: `local\pp_off` against `local\pp_on`, the switch only, both variables
set. Logs `local\PP_OFF_USN04.log` / `PP_ON_USN04.log` and `PP_OFF_USN02.log` /
`PP_ON_USN02.log` in worktree cc9-units2. All four show the fit line, the immediate present
interval, a module directory in this tree and the final COM release. `tools/pair_diff.py` exits 1
on both pairs: gameplay, death rows and the unit table are identical, and the masked multiset of
other lines is empty.

| row | USN04 OFF -> ON | USN02 OFF -> ON | verdict |
| --- | --- | --- | --- |
| construction pushes (native `construct_push_00926be0` calls) | 0 -> 81 | 0 -> 32 | held |
| `skipped_pending` | 0 -> 21 | 0 -> 32 | held |
| `squadron_upgrades` / `wing_deferred` / `wing_append_skipped` | 0 -> 20 / 40 / 0 | 0 / 0 / 0 | held |
| `load_dropped` / `skipped_attached` | 0 / 0 | 0 / 0 | held |
| InitAll `pushes` | 46 -> 86 | 34 -> 34 | held |
| InitAll calls / with work / entities / wing appended | unchanged | 9,009 / 5 / 34 / 0 unchanged | held |
| load walk pushes / mirrored | unchanged | 30 / 2 unchanged | held |
| every other native row | unchanged | unchanged | held |
| deaths, hit records | 43, 788 both | 21, 354 both | held |

**Verdict: ON.** `create_units` is now a pusher, as 00928630 is in the image. The Lua routes'
pushes and the load walk's unit pushes are duplicates the dedup list skips. Retiring them is
cc9-init-passes' next step. The wing-construction contract (section 9.3) remains open.

## 27. The units host's init-pass gaps, read (packet `cc9_units_init_pass_gaps`)

2026-09-27, worker cc9-units2, on main 34191e927. Ghidra was read only. This covers the rows of
docs/SENTITY_INIT_PASSES.md section 4 that name `src/game_hosts_units.cpp`, each checked against
the image and the host before any binding. The result: no row has a gameplay effect the units
file can bind alone on the reference missions. One row is a real candidate that needs the Lua
host's pass C (below). So no switch is added.

| row | image | host | verdict |
| --- | --- | --- | --- |
| ship pass C immediate rudder `0080DA00` (`0081FD6E`) | inside `0081F980`'s holder-kind-3 branch (`SEntity_GetSavedEntityLuaData`, the entity's saved Lua table; docs/ENTITY_LIFECYCLE_TAILS.md, kind 3 = the `_entity` table), next to `camoColor`, `leakManager` and the weapon-director state | scene creation makes the kind-1 property bag | **not reached on scene creation**: a save/restore path, no gap |
| ship pass C immediate throttle | kind-1 branch: the bag's StartSpeed -> `0080D9B0` | host (`SceneStartSpeed`) | covered |
| ship pass C torpedo stock `0081F8B0` (`008201B8`, argument class+7A0h `MaxTorpedoStock`) | after the kind-1/2 branch: with fewer live torpedoes (`00810E90`) than the stock, spare unit+104Ch = stock − live; else unload (`0081DCB0`) down to the stock; then re-arm every torpedo barrel (weapon type 7) whose timer sits at the FLT_MAX sentinel (`00D7A278`) through `0072D520` | no spare field; the gunnery host re-arms tubes on its own clock, and the HUD's `00815850` gauge is a record | **contract for the gunnery owner** (below); a units-side store alone changes nothing |
| squadron avoid-zone layers `007F1D90` (pass B) and `0041DF40` (pass C, `007F524A` / `007F5268`) | pass C re-selects both: +350h by the bag's slope value (flag 0), +34Ch by (1.5, true) again | +34Ch sampled (`kAvoidZoneLayerSampleBound`); +350h's only reader is the move-to's `009CFAD0`, which the host does not run (docs/AVOID_ZONE_REGISTRY.md) | covered; the 0041DF40 census is now four call sites, all two pairs of the same queries |
| squadron pass C initial command `0077D600` (`007F4E9E`) | only when the squadron has no current command (`007F4E11`) and lobby mode != 2: if its first member's +900h is 6 (on the water) a `stop` (`00E08F88`) at its position (`007EF8F0` / `00468560`), else if it has a home base (+404h, written by `007F1C00` `SetHomeAirBase`) a `moveto` (`00E08F68`) toward that entity | the host issues each unit's authored `Command` token at load and the scripts' orders; nothing issues this default | **gameplay candidate**, not bound: it needs the squadron's pass C (the Lua host's InitAll) and the squadron's current-command state; see the plan below |
| plane pass B physics body `00C5D580` | `007D6137` | the host's plane motion has no rigid body (`plane_flight.cpp`); the ship hull body uses `00C5D580` (`dyn_body_creation.cpp`) | not a units-file row while planes fly on the host's own law |
| plane pass B flight-controller setters `007D9E80` (`007D64E5`, `007D654D`) / `007D9EE0` (`007D659B`) | the bag's `Velocity` (km/h, over the double 3.6 at `00D06588`) and `VelocitySI`, each capped by class+18Ch, and `Vel3D` set the initial speed | the host seeds the class travel speed (`007C6340`'s rule) | only a plane with its own scene bag reads them; wing planes have none, and neither reference mission authors a plane row with these keys; not bound |
| actuators `007EABC0`, neighbours `007E1E20` | built at `007D61AB`, `007D620E` | the torpedo release uses `007EABC0`'s block (`torpedo_release_spawn.cpp`); the neighbour list is the host's own squadron registry | covered in effect |
| firing-gun registration `007C74A0` (`007D7038`) | the plane joins the firing list on lastGunState +C35h | `kPilotGunfireAvoidanceBound` (the host's gunFire byte +BC9h stands in, labelled) | covered |
| the `00BD2F10` draw (`007D6247`) | U(2.0, 4.0) (`00CE3958`, `00CE3D34`) into unit+C44h | its readers are `007CBFA0` `BSP_Plane_GroundRollStep` (`007CC003`..`007CC017`), the ground-roll arm; the host counts that arm but runs no roll | render/take-off only; the draw's stream position matters only for a shared generator, and the host's streams are keyed |
| `Skill` `00927A80` (`007D65AA`) | the bag's skill | docs/PILOT_SKILL_LEVEL.md; the host's `set_skill_level_007b8ae0` | covered by the skill packet |

### Contract for the gunnery owner (cc9-ships): the torpedo stock

- **At a ship's pass C,** `0081F8B0(class+7A0h)` sets spare unit+104Ch = MaxTorpedoStock minus the
  loaded torpedo barrels (`00810E90` counts barrels whose timer is below FLT_MAX). A barrel at
  the sentinel is empty and re-armed only by `0081F8B0`, which runs from pass C and from the
  supply tick `00825450`. So the spare is the ship's whole reserve.
- **The Lua property `TorpedoStock`** (`00815870`) reads spare + loaded.
- **The host needs:** a spare per ship (this installation's `MaxTorpedoStock`, e.g. Fletcher 30,
  Fubuki 27, Mogami 18), a tube that goes to the sentinel after it fires when no spare is left,
  and the re-arm that spends the spare. The units host can store the spare and answer
  `00815850` once the gunnery host says how many barrels are loaded. The consumer, where a fired
  tube waits for a spare, is the gunnery file's `[gun+3F0h]->vtable[1F4h]/[1F8h]`
  (docs/GUN_SHOT_CADENCE.md, `unit_ammunition_provider`).

### Plan for the squadron's initial command (the one candidate)

1. **The hook.** The Lua host's pass C for a squadron node (`SEntity::InitAll pass C init_slot_a4`,
   cc9-init-passes' file) calls a units-host entry, `squadron_initial_command_007f4e11(leader)`,
   at the image's point: after pass A and B, once the node's home base is known.
2. **The test.**
   - The squadron's current command (`007F4E11`, the command controller's current entry) must be
     empty. The host keeps command rows per slot (`GameCommandRow`); "empty" is no row issued to
     the leader yet.
   - Lobby mode is 0 in single player.
3. **The two arms** through `issue_script_command`:
   - first member on the water: `stop` at its pose;
   - else, with a home base: `moveto` with the home entity as the object target.
4. **Home base.** It is set for the air-ops launches (`air_operations.cpp`, `home=` on the
   launch line), and from a squadron bag's `HomeBase` key (`00CF8820`) at pass C.
5. **Predictions to write:**
   - USN04's four air-ops launches get a `moveto` to their carrier unless their launch already
     commands them;
   - USN02's Kingfisher row (on the water) gets a `stop` unless it carries an authored command.
   - Gameplay moves only through those squadrons.

### The two entries (packet `cc9_squadron_pass_hooks`)

They are declared in `include/bsp/game_hosts_units.hpp` as no-ops that only count their calls
(summary `squadron pass hooks pass_a=N pass_c=N`). The Lua host's InitAll calls them for a
plane-squadron node. The argument is the squadron's unit index: its fused leader, id - 1.

```cpp
void GameUnitsHost::on_squadron_pass_a_construct_wing(std::size_t squadron_index);
void GameUnitsHost::on_squadron_pass_c_initial_command(std::size_t squadron_index);
```

- **Pass A** is the squadron's `007F4580`. The call goes before the Lua host's wing append. The
  entry will construct the wing there (docs/SENTITY_INIT_ATTACH_ORDER.md section 9.3).
- **Pass C** is the squadron's `007F4BA0`. The call goes at the point of `007F4E9E`, after passes
  A and B. The entry will issue the initial command described above.
- Each entry gets its own switch, committed OFF with predictions, once the calls land.

### The initial command, read for step 2 (packet `cc9_squadron_initial_command_read`)

**The image, re-read from the listing (007F4E06..007F4EA3).**
- `007F4E06` `MOV ECX,[ESI+348h]`, `007F4E0C` `CALL 0071BE40`: the squadron's own command
  controller at +348h answers its current command. A non-zero answer skips the block (`007F4E13`).
- `007F4E19`..`007F4E24`: lobby mode `[00E188A8]+1FE4h` compared with EDI (the 2 of the other
  lobby tests). Single player passes.
- `007F4E26`..`007F4E37`: `[squadron+3D0h]+900h == 6` (the first member on the water) takes the
  stop arm.
- **The stop arm** (`007F4E5D`..`007F4E97`): the squadron's OWN pose is refreshed (`00414DB0`
  when +C8h is 0). The target is built from `&squadron+FCh` (its position) through `007EF8F0`
  (with `[00E188A8]`) and `00468560`. The command class is `stop` `00E08F88`.
  **Correction** to the first read above: the position is the squadron's, not the first member's.
- **The moveto arm** (`007F4E39`..`007F4E56`): home base +404h non-null. `00465080(home, 0.0f,
  1)` builds an object target. The command class is `moveto` `00E08F68`.
- `007F4E9C`: ECX = the squadron, then `0077D600` (flags 1). The squadron's MT_COMMAND handler
  fans out to its members (`007ECF80`'s shape).

**The host's equivalents.**

| image | host |
| --- | --- |
| `0071BE40` on squadron+348h | `GameUnitsHost::director_current_command_0071be40(leader)`. The host fuses the squadron with its leader plane, so the leader's director stands in for the squadron's controller (labelled) |
| `[squadron+3D0h]+900h` | the first member's `plane_control_mode_900` (the registry's `member_units[0]`) |
| the squadron's position +FCh | the leader's pose |
| `00465080` object target | `bsp::SceneCommandTarget{kind 1, object = the carrier's unit identity, object_id = its id}`, `target_name` = the carrier's name, as `GameScriptOrdersHost::entity_issue_command` builds one |
| `0077D600` + the squadron fan-out | `issue_script_command(member, 0x00E08F68 or kCommandStop 0x00E08F88, target, 1, "squadron_pass_c", name)` once per live member, as the AI command tick's squadron arm does (`src/game_hosts_ai.cpp`, `tick_issue_moveto`) |
| home base +404h | the air-ops launch's `home=` (`air_operations.cpp`, 006C5050's `HomeBase` bag key = the owner); from a scene row, the bag's `HomeBase` read at pass C (`00CF8820`) |

**Who gets what on the reference missions.**
- **USN04: the four air-ops launches.** They are `Lexington-class01_sqn01` / `_sqn03` and
  `Yorktown-class01_sqn02` / `_sqn04`, with `home=` their carrier on the launch line. They are
  airborne at 150 m, so +900h is 7, not 6. At pass C (inside `LaunchSquadron`'s InitAll at
  `0089E613`) no command is current. The AI group's `dogfight` arrives about 29 fixed steps later
  (`order_attack` then `player command issued ... token="dogfight"`, lines 8235-8243 of
  `local\PP_ON_USN04.log`). So each gets a **moveto toward its carrier**: 4 squadrons, 12 member
  orders.
- **USN04: the 16 SpawnNew squadrons.** No home base. `usn_19_coralus.scn` (2024-08-09) carries
  `HomeBase = RFort ""` only in its two `PlaneSquadronWNavpoint` templates (byte offsets 67170
  and 67760), and no row fills it. Their planes fly, not float. So **nothing** is issued, whether
  a command is current or not.
- **USN02:** the host builds no plane squadron (`ai squadrons: this mission created no unit
  answering IsKindOf(0Fh)`). So `pass_c` is 0, and the Kingfisher named in
  docs/SENTITY_INIT_PASSES.md is not a squadron here. **Nothing** is issued.

**Predictions for the initial-command pair** (same tree, switch only, both variables set; the
pass C call landed by cc9-init-passes in both builds):

| row | USN04 4700/4500 | E2 = USN04 9200/9000 | USN02 9200/9000 |
| --- | --- | --- | --- |
| `pass_c` calls | 20 both | 20 both | 0 both |
| initial commands | 0 -> 4 moveto (12 member orders), 0 stop | the same | 0 |
| the launched Wildcats' first order | moveto toward their carrier, replaced by the AI group's `dogfight` about 1.5 s later (their current command then changes as today) | the same | - |
| deaths, hit records | 43 ± 3, 788 ± 8 %: a 1.5 s heading change on 12 fighters shifts their intercepts | 52 ± 3, 875 ± 8 % | identical |
| torpedo-task and dive-bomb-task releases | unchanged ± 1 | unchanged ± 1 | - |
| pair_diff exit | 3 if an intercept moves, else 1 | same | 1 |

### The initial command, bound (packet `cc9_squadron_initial_command`, `kSquadronInitialCommandBound`, committed OFF, ON since the pairs)

`GameUnitsHost::on_squadron_pass_c_initial_command`, called by the Lua host's squadron pass C
(27ab3a4b4), does the following:
- **The test:** it returns when the leader's director has a current command.
- **The stop arm:** a squadron whose first member's +900h is 6 gets a `stop` at the leader's
  position.
- **The moveto arm:** otherwise, a squadron with an air-ops home base gets a `moveto` toward it.
  The home base is found by searching the decks for the slot whose `launched_squadron` is this
  squadron's id.
- **The orders:** each arm issues one `issue_script_command` per live member, flags 1, source
  `squadron_pass_c`.
- **The census:** a `squadron initial command ...` note per order and a summary line.

**SUBSTITUTIONS, labelled:**
- The leader's director stands in for the squadron controller +348h.
- The stop's point is the leader's position, because `007EF8F0` and `00468560` are unread.
- The only home base the host knows is an air-ops launch's. A scene row's `HomeBase` key is not
  read. No reference row fills it.

**Predictions:** the table under "The initial command, read for step 2" above. Also
`skipped_current` + `no_home` = 16 on both USN04 pairs: the 16 SpawnNew squadrons, split by
whether their route issued an authored command before pass C. It is 0 both ways on USN02.

**The pairs, measured.** The binaries are `local\ic_off` and `local\ic_on` from `19c45d4a0`, the
switch only, both variables set. The logs are `local\IC_{OFF,ON}_{USN04,E2,USN02}.log` in worktree
cc9-units2. Every log shows the fit line, the immediate present interval, a module directory in
this tree and the final COM release.

| row | USN04 4700/4500 | E2 9200/9000 | USN02 9200/9000 | prediction | verdict |
| --- | --- | --- | --- | --- | --- |
| movetos / member orders / stops | 0 -> 4 / 12 / 0 (Lex sqn01, Town sqn02, Lex sqn03, Town sqn04, each toward its own carrier) | the same | 0 | 4 / 12 / 0; USN02 none | held |
| `skipped_current` / `no_home` | 0 / 16 | 0 / 16 | 0 / 0 | sum 16 | held |
| deaths, hit records | 43, 788 both | 52, 875 both | 21, 566 both | ± 3, ± 8 % | held (identical) |
| releases | 4 of 16, 1 of 19 both | identical | - | ± 1 | held |
| natives | the order route rows + 12 | + 12, and `initial_command_007f4e9e` 4 | unchanged | - | - |
| pair_diff exit | 1 | 1 | 1 | 1 or 3 | held |

**Verdict: ON.** The moveto toward the carrier is issued as in the image. The AI group's
`dogfight` replaces it before it changes an intercept on these missions.

## 28. The image's buoyancy element list replaces the stand-in (packet `cc9_buoyancy_elements`, `kShipBuoyancyElementsBound`, committed OFF, ON since the pairs)

2026-09-27, worker cc9-units3, on main 5b9a40d70. Ghidra was read only.

Section 25's caveat: the leak manager's capacity comes from controller+84h, the reserve buoyancy
`00937DBB..00937F74` sums over the class's element list, and the host list was an eight-element
stand-in (capacity = Mass / 3). The element list's producer was already found and reconstructed
(docs/SHIP_BUOYANCY_ELEMENTS.md, `src/ship_buoyancy_elements.cpp`) but not bound in any host.

### Where the real elements come from (V)

- **The producer is `0082D040`.** It is called once, at `0082FEE3`, inside `0082FE30`. That
  function is the ship class descriptor's model-binding virtual: slot 8 of every ship-kind
  vtable, and the ninth kind's `00759120` calls it at `0075913D`. `ghidra xrefs 0082D040`
  returns that one site.
- **The window, from disk bytes:**
  - `0082FEA9` loads the class model `[EDI+50h]`, the row's `Mesh`.
  - `0082FEBA` and `0082FECA` call `00718000`, the node lookup, with kind `EBX` = 0, for
    `"deckline"` and `"bottomline"`.
  - `0082FECF` loads `Hull.WaterLineRatio` at descriptor+71Ch.
  - `0082FED9` / `0082FEDD` add 44h to each record (its point vector).
  - `0082FEE3` calls `0082D040` with `ECX` = the descriptor.
- **The authored data in this installation:**
  - `Hull = { Segments, WaterLineRatio }` is in `scripts/datatables/autoload/vehicleclasses.lua`
    (2026-05-09, modded). 161 class blocks carry it. The DeRuyter block starts at line 13618 and
    its `Hull` table at line 13771: `Segments = 5`, `WaterLineRatio = 0.55`.
  - The two point sets are `Aux` entries in the class `.mmod`. Each is an `Identifier` (counted
    name, then the u32 kind) followed by `Points` (a byte count and float triples). 160 of the
    161 classes have both. The one without them is the Black Cat, a plane model that `0082FE30`
    never reaches.
  - The PACK3 classes (Fiji, Icarus) are not in the loose Lua file. The run log prints their
    values.
- **The list:** `2 × Hull.Segments` records.
  - The station z values span the whole Length, and x = ±Width/2.
  - Deck, keel and waterline come from the two polylines.
  - The coefficient is `5 × Mass / (Segments × draught)`.

### What the reserve becomes, in closed form

At every record, waterline − keel = (deck − keel) × (1 − r), with r = `Hull.WaterLineRatio`. So
00937C90's fraction |keel − deck| / |waterline − keel| is 1 / (1 − r) at every station. With the
Ship material's Kitevo of 1, each term is coefficient × section height × Gravitacio / 10. The sum
is 10 × Mass / (1 − r), which gives:

- controller+84h = 10 × Mass × r / (1 − r);
- the leak capacity = Mass × r / (1 − r).

Once both nodes exist, this does not depend on the model geometry. Against the stand-in's
Mass / 3:

| class (this installation) | r | capacity / Mass | stand-in | hydrodynamic saturation, × weight | time to neutral |
| --- | ---: | ---: | ---: | ---: | ---: |
| DeRuyter | 0.55 | 1.222 | 0.333 | 3.58 | 52.8 s |
| Northampton (Houston) | 0.45 | 0.818 | 0.333 | 2.56 | 47.7 s |
| York (Exeter) | 0.50 | 1.000 | 0.333 | 3.00 | 50.0 s |
| Kagero | 0.48 | 0.923 | 0.333 | 2.81 | 49.0 s |
| Shiratsuyu | 0.46 | 0.852 | 0.333 | 2.64 | 48.1 s |
| Clemson | 0.3925 | 0.646 | 0.333 | 2.18 | 45.6 s |
| Fubuki | 0.415 | 0.709 | 0.333 | 2.32 | 46.4 s |
| Kuma | 0.50 | 1.000 | 0.333 | 3.00 | 50.0 s |
| Myoko | 0.37 | 0.587 | 0.333 | 2.05 | 44.8 s |

The last two columns are this section's model of the descent, not the image:

- **Saturation.** `009329C0`'s element force is coefficient × d × (0.5 d / draught + 0.5), with
  d clamped to the section height. The stand-in hull floats already saturated: its element
  point is at y = 0 and its span is the draft, so any water sinks it at once. The real hull
  floats at its waterline. It holds (saturation − 1) × weight in reserve before its deck line
  goes under.
- **Time to neutral** = (saturation − 1) × Mass / inflow. The inflow is 2 × cap = 0.04 ×
  capacity per second after the handler's redistribution.
- **After neutral**, the net downward acceleration follows the stand-in's curve to within 3 %.
  For DeRuyter it is 10 × 0.0489 t / (3.58 + 0.0489 t), against the stand-in's
  10 × 0.0133 t / (1 + 0.0133 t).
- So each wreck's descent should take the stand-in's time plus its class's time to neutral.

### It is not only the wrecks

`009329C0` walks the element list on every step of every live hull, so the binding changes live
flotation too:

- elements at ±Width/2 give the hull a roll-restoring buoyancy it did not have;
- the stations sample the real deck and keel lines, so the trim follows the model;
- the drag scale is Mass / count × fraction, and a floating hull's fraction falls from 1 (the
  saturated stand-in) to 1 − r;
- the element count is 2 × Segments instead of 8.

The waterline samples lie within 0.15 m of the model origin on the DeRuyter (the table in
docs/SHIP_BUOYANCY_ELEMENTS.md), so the mean draft barely moves. Motion and gunnery geometry still
move from the first step, so identity on E2 is **not** expected.

### The binding

- **The switch** is `kShipBuoyancyElementsBound` in `src/game_hosts_units.cpp`, committed OFF.
- **In both builds**, the units host builds each ship class's image list once, at the
  stand-in's site in `create_units`. `class_buoyancy_list_0082fe30` does four things:
  - it reads the `Mesh` through the mounted VFS;
  - it reads the Aux point items through `read_mmod_aux_point_items_0071b3e0`, the reader
    docs/GUN_BARREL_COUNT.md uses for `"fire"`;
  - it finds the two nodes with `find_named_point_group_00718870`, which applies 00718000's rule
    (the last exact match of name and kind);
  - it runs `ship_buoyancy_build_from_model_0082fea9` and logs one `buoyancy elements class=...`
    line per class.
- **ON:** a hull whose class list was built takes it.
- **OFF, and ON for a class without both nodes or with Segments ≤ 0:** the stand-in, which stays
  reachable. ON, such a hull is counted in `fallbacks`. The original would dereference a null
  node record there; the host refuses instead.
- **Lua:** `Hull.WaterLineRatio` is a nested float. The Lua host gains
  `read_vehicle_class_nested_number`, the float twin of the nested integer reader.
  `Hull.Segments` uses the integer reader, which truncates like the CRT float-to-int at
  `00832DE8`.
- **Census:** `summary ship buoyancy elements` counts the classes, the hulls on the image list,
  the hulls on the stand-in and the fallbacks. The wreck `leaks` line gains `elements=`.
- **Nothing else changes.** The reserve, the leak manager, the handler and the kill are sections
  24 and 25's, now fed by the image's list.

### Predictions (written before the runs)

Same tree: the OFF build against the ON export, both variables set. The OFF rows are expected
close to section 25's `SD_ON` measurements. Main has moved since, with gameplay-identical
verdicts on USN02.

| row | USN02 9200/9000 | E2 = USN04 9200/9000 |
| --- | --- | --- |
| `summary ship buoyancy elements` | OFF: image 0, stand-in 32. ON: image 32, stand-in 0, fallbacks 0 | OFF: image 0, stand-in 18 (the `leak_models` count). ON: image 18, fallbacks 0 |
| `buoyancy elements` class lines | both builds, one per ship class, all `image list`; `sum_coef_draught` = 10 × Mass | the same |
| wreck `capacity` | Mass × r / (1 − r): DeRuyter 9396, Houston 9492, Exeter 10350; Kortenaer (Icarus) from the log | no wrecks |
| `elements=` on the wreck lines | 2 × Segments: DeRuyter 10, Houston 8, Kagero 10, Clemson 10 | - |
| each wreck's time from first +5Dh to −200 m | section 25's 91.6..98.6 s plus the class's time to neutral: about 140..152 s, band 115..200 s | - |
| kills within 450 s | only wrecks whose first +5Dh is before about 300 s: 13, band 9..16 (OFF about 17) | 0 |
| list 6 at the end | about 19, band 16..23 (OFF about 15) | 18 both |
| neighbour `mean_count` | above OFF (fewer and later unlinks), band OFF..OFF+4 | unchanged at 18.00 |
| hydrodynamics `element_steps` | up by the mean 2 × Segments / 8, about × 1.15, band × 1.0..1.35 (wreck removals also move it) | about × 1.3, band × 1.1..1.6 |
| live hulls | trim and roll follow the new list from the first step | the same |
| deaths | OFF ± 4 | 52 ± 6 |
| hit records | OFF ± 15 % | 875 ± 12 % |
| first moved gameplay row | before the first kill, possibly before the first death: live hulls float differently from t = 0 | anywhere |
| failure time | 39.65 s ± 2 s (it is scripted; a change means the early engagement moved) | none |
| pair_diff exit | 3 | 3 (1 only if no live motion reaches a hit or a death) |

### The pairs, measured

One tree, `2a41fc841`. The OFF build is the tree's own `build\`; the ON build is
`tools/pair_export.py --flip kShipBuoyancyElementsBound=true --out local\bu_on`. Both variables
were set. The logs are `local\BU_OFF_USN02.log` / `BU_ON_USN02.log` and `BU_OFF_E2.log` /
`BU_ON_E2.log` in worktree cc9-units3. All four show the fit line, the immediate present
interval, a module directory in this tree and the final COM release. `tools/pair_diff.py`
returned exit 3 on both pairs.

Every ship class on both missions built its list from the model: 11 classes on USN02 and 8 on
E2, with no fallbacks. `sum_coef_draught` equals 10 × Mass exactly on every class line. The PACK3
row: Kortenaer is class 265 (`models/ships/rn/tribal.mmod`, r = 0.50, Segments 5).

| row | USN02 OFF | USN02 ON | prediction | verdict |
| --- | ---: | ---: | --- | --- |
| hulls on the image list / stand-in / fallbacks | 0 / 32 / 0 | 32 / 0 / 0 | 32 / 0 / 0 | held |
| capacity: DeRuyter, Houston, Exeter | 2562.67, 3867.33, 3450.00 | 9396.44, 9492.55, 10350.00 | 9396, 9492, 10350 | held (exact) |
| `elements=`: DeRuyter, Houston, Alden | 8, 8, 8 | 10, 8, 10 | 10, 8, 10 | held |
| time from first +5Dh to −200 m | 91.50..97.95 s (19 wrecks) | 106.25..117.35 s (16 wrecks) | 140..152 s, band 115..200 | **failed**: 13 of 16 below the band, all about 35 s faster than the estimate |
| water at −200 m (DeRuyter) | 9979 (1.30 × Mass) | 39916 (5.19 × Mass) | not predicted | - |
| kills, unlinked nodes | 19, 114 | 16, 96 | 13 (9..16) | held at the band's edge |
| list 6 at the end | 13 | 16 | 19 (16..23) | held at the band's edge |
| not under by 450 s | Haguro, Jintsu | Encounter, Haguro, Jintsu, Murasame, Harusame | - | - |
| neighbour `mean_count` | 28.24 | 28.41 | OFF..OFF+4 | held |
| hydrodynamics `element_steps` | 1,803,096 | 2,232,148 (× 1.24) | × 1.0..1.35 | held |
| deaths | 21 | 21 (Asagumo survives, John1 dies) | OFF ± 4 | held |
| hit records, hull hits | 566, 287 | 596 (+5.3 %), 301 | ± 15 % | held |
| first hit | 35.80 s | 35.65 s | first moved row before any kill | held (moved before the first death) |
| failure | 39.65 s | 39.65 s | ± 2 s | held |
| controlled DeRuyter distance | 2082.34 m | 2196.68 m | not predicted | - |
| pair_diff exit | - | 3 | 3 | held |

| row | E2 OFF | E2 ON | prediction | verdict |
| --- | ---: | ---: | --- | --- |
| hulls on the image list / stand-in / fallbacks | 0 / 18 / 0 | 18 / 0 / 0 | 18 / 0 / 0 | held |
| hydrodynamics `element_steps` | 1,296,000 | 1,674,000 (× 1.29) | × 1.1..1.6 | held |
| deaths | 52 | 51 | 52 ± 6 | held |
| hit records, hull hits | 875, 345 | 894 (+2.2 %), 320 | ± 12 % | held |
| shots | 6092 | 7454 | not predicted | - |
| torpedo-task / dive-bomb-task releases | 4 of 16 / 1 of 19 | 7 of 16 / 3 of 19 | not predicted | - |
| controlled Lexington distance | 6017.22 m | 5819.94 m | not predicted | - |
| neighbour `mean_count` | 18.00 | 18.00 | unchanged | held |
| first hit | 93.00 s | 93.00 s | - | - |
| pair_diff exit | - | 3 | 3 | held |

**Why the descent estimate failed.** The model assumed that after neutral the wreck follows the
stand-in's curve, because the net buoyant acceleration matches. But the hydrodynamic drag is a
force scaled by class Mass / count (`00933018`) and divided by the total mass Mass + water
(`00932B72`). A real-list wreck carries 3 to 5 times its mass in water by the time it goes under,
against 1.3 times for the stand-in. So its drag deceleration is several times weaker, and it falls
faster once its deck line is under. The capacity and the inflow (2 × cap, DeRuyter 376 per
second) are as predicted.

**What moved on E2 without a death.** No ship dies on E2, so the change is live flotation only.
Hulls now roll on their elements at ±Width/2 and trim along their deck and keel lines. The shots
the ships fire, the AA hits on the raids and the aircraft releases all moved. The release counts
are aircraft task outcomes on a changed AA screen, not a buoyancy effect of their own. The first
hit is identical (93.00 s).

**Verdict: ON.** Every hull floats on and sinks by the element list the image builds from its
own model and class row. The capacity is exactly the closed form, with no fallback on either
mission. The descent of section 25 is now 106..117 s from first +5Dh to −200 m on USN02, not
91..98 s. The caveat in section 25 is resolved by this section.

## 29. A scene row's `HomeBase` at squadron pass C (packet `cc9_scene_home_base_key`, `kSceneHomeBaseBound`, committed OFF, ON since the pairs)

2026-09-27, worker cc9-units3, on main b4687c7a1. Ghidra was read only.

Section 27's initial command knows only the air-ops carrier as a home base. This packet reads
how the image fills squadron+404h from a scene row.

### How the image sets the home base (V)

Pass C is `007F4BA0`, the same routine whose tail issues the initial command. Before that test it
reads three keys from the squadron's property bag at +C0h. It proceeds only when the bag's type
word `[+C0h]+4` is 1 (`007F4C05`). `008F2260` is the key lookup.

| step | site | what |
| --- | --- | --- |
| `State` (`00CF8818`) | `007F4C12..007F4C3C` | an integer value (type 0) or the default 7; the flag argument is `State <= 1` |
| `HomeBase` (`00CF8820`) | `007F4C43..007F4C8A` | a value of type 5 with a non-empty name (`008F0E20`) is looked up by `00925A90`, `BSP_EntityRegistry_FindEntityByName` on `[00E188A8]+19CCh`; the result, possibly 0, is the home |
| `SpawnPoint` (`00CE56B8`) | `007F4C8C..007F4CF0` | a non-empty name is looked up the same way, **without a null check**; its `[entity+3Ch]` replaces the home and sets the flag when `006BCD20` gives it an air-operations block |
| `007F1C00` | `007F4CFA` | `(home, flag)`: store +404h behind the observer pair at +3F0h, then queue the squadron on the home's block |

`007F1C00` (`007F1C00..007F1C93`, `RET 8`):
- A null home clears +408h (`007F1C89`).
- Otherwise `006BCD20(home, 0)` (`007F1C40`) returns the block: MMothership (9) gives +1188h,
  MAirfield (45h) gives +72Ch, anything else null.
- Then `[00E188A8]+1FE4h` chooses the queue:
  - non-zero: `006CC760`, the spotting queue;
  - zero: `006CC7B0` (`007F1C69`), the campaign queue at block+74h.
- On the campaign arm, a set flag writes +408h and calls `007ED6E0`.
- **There is no null check on the block.** A home that is not an air base faults in the queue
  call.

The other callers of `007F1C00` are `007F1FE0` and `0089E220`, the `SquadronSetHomeBase` Lua
binding. Neither reference mission calls the binding: no native row in `local\BU_OFF_E2.log` or
`BU_OFF_USN02.log`. An air-ops launch's bag carries `HomeBase` = the deck owner (`006C518D` in
`006C5050`), so a launched squadron comes through the same `HomeBase` read.

### Who authors a `HomeBase` in this installation

There are 259 loose `.scn` files. Every `HomeBase` occurrence was counted:

| value | occurrences | where |
| --- | ---: | --- |
| `RFort ""` | 1512 | templates and rows, empty |
| `R ""` | 610 | empty |
| `RFort "Landscape 01\MainAirFieldEntity 01"` | 8 | IJN08 only: two `PlaneSquadronGen` rows ("Ki-43 Oscar 01", "Gekko 01") in each of four copies of the scene |

The four IJN08 copies:
- `COTP-IJN/ijn_08_defend_guadalcanal.scn`, lines 26214 and 26233, 2024-07-13;
- `ijn/JM/ijn_08_defend_guadalcanal.scn`, the same lines, 2024-07-13;
- `COTP-IJN/PRCPIJN/ijn_08_defend_guadalcanal.scn`, lines 26768 and 26787, 2024-07-13;
- `COTP-IJN/PRCPIJN/prcpijn_08_defend_guadalcanal.scn`, the same lines, 2024-08-09.

No scene authors a squadron `SpawnPoint` or `State`. `usn_19_coralus.scn` (USN04) has two
`HomeBase` lines, both `RFort ""` in its `PlaneSquadronWNavpoint` templates. `usn_2_java.scn`
(USN02) has none.

### The binding

- **Contract for the scene-contents owner (cc9-ships).** Call
  `GameUnitsHost::set_squadron_scene_home_base(squadron_index, home_base)` once per squadron
  built from a `PlaneSquadronGen` row, before its pass C. `home_base` is the row's `HomeBase`
  name as authored ("" when empty). For a row held back and created later by
  `GenerateObject` / `SpawnNew`, the key must travel on the spawn-pool entry, as `WingCount`
  already does (`src/game_hosts_scene_contents.cpp`, the held-back `PlaneSquadronGen` block).
  The entry stores the key in both builds and counts `keys`.
- **ON, at pass C before the command test** (`kSceneHomeBaseBound`, `src/game_hosts_units.cpp`,
  committed OFF), for a non-empty key:
  - it looks up an air-ops deck of that name and a unit of that name (the host's `00925A90` and
    `006BCD20`);
  - **neither:** `unresolved`; +404h stays null, as `00925A90` returning 0 leaves it;
  - **a unit but no deck:** `not_airbase`, a labelled **REFUSAL**: the image faults in
    `006CC7B0`, and the host keeps +404h null;
  - **a deck:** +404h = the name; the squadron is pushed through
    `air_ops_push_assign_queue_006cc7b0` (the campaign arm, since this process asserts a campaign
    session); `resolved` and `queue_pushes` are counted.
  - The initial command's moveto arm then prefers this home over the deck-slot search.
- **SUBSTITUTIONS, labelled:**
  - the squadron's +35Ch and +3CCh for the queue are the registry record's `Type` and its
    member count;
  - an airfield without a unit slot has no object target, so the moveto arm counts `no_home`
    and issues nothing;
  - `SpawnPoint` and `State` are not read: no scene row authors them, and the launch bag's
    values stay the air-ops path's.
- **Census:** `summary squadron scene home base` (bound, keys, resolved, unresolved,
  not_airbase, queue_pushes), in both builds.

### Predictions (written before the runs)

Same tree, the switch only, both variables set. Nothing calls the entry until the contract is
wired, and neither mission authors a non-empty `HomeBase`, so the two builds must be identical.

| row | USN04 4700/4500 | USN02 9200/9000 |
| --- | --- | --- |
| `summary squadron scene home base` | `bound` 0 -> 1; keys 0, resolved 0, the other counts 0 | the same |
| initial command `movetos` / `no_home` | unchanged (4 / 16) | unchanged (0 / 0) |
| deaths, hit records, releases, every per-entity row | identical | identical |
| native table | identical (the `set_home_air_base_007f1c00` row is never reached) | identical |
| pair_diff exit | 1 (only the summary line's `bound`) | 1 |

### The pairs, measured

One tree, `ef25d5c4f`. The OFF build is the tree's own `build\`; the ON build is
`tools/pair_export.py --flip kSceneHomeBaseBound=true --out local\bu_on`. Both variables were
set. The logs are `local\HB_{OFF,ON}_{USN04,USN02}.log` in worktree cc9-units3. All four show
the fit line, the immediate present interval, a module directory in this tree and the final COM
release.

| row | USN04 4700/4500 | USN02 9200/9000 | prediction | verdict |
| --- | --- | --- | --- | --- |
| `summary squadron scene home base` | `bound` 0 -> 1, all counts 0 | the same | the same | held |
| initial command movetos / no_home | 4 / 16 both | 0 / 0 both | unchanged | held |
| deaths, hit records | 40, 799 both | 21, 596 both | identical | held |
| releases | 7 of 16, 3 of 19 both | - | identical | held |
| death rows, unit table, native table | identical (40, 81, 1598 rows) | identical (21, 32, 1511 rows) | identical | held |
| pair_diff exit | 1 | 1 | 1 | held |

**Verdict: ON.** No reference mission authors a non-empty `HomeBase`, and nothing calls the
entry yet, so ON changes nothing today. It makes the image's path live the moment the scene
owner wires the contract. The first mission it will act on is IJN08: its two squadrons name the
airfield "Landscape 01\MainAirFieldEntity 01". **Open, for that mission:** whether the host
registers an air-ops deck under that name, and whether a unit slot carries it. If no slot
carries it, the moveto arm counts `no_home` and the squadron is queued but not ordered (the
substitution above).

## 30. The wing built in the squadron's pass A: the units and script-orders half (packet `cc9_wing_construction`, `kWingConstructionInPassABound`, committed OFF)

Worker cc9-ships, 2026-09-27, base main `f73bca10f`. The plan is in `docs/RELEASE_ISSUE_STAGE.md`,
"Handoff: cc9-units2's queue", item 1. The Lua half is in `docs/SENTITY_INIT_ATTACH_ORDER.md`
section 15.4, owned by cc9-movie-camera. Both halves flip together.

### 30.1 What the image does, and what the host did

- **The image.** A plane squadron's slot-39 attach, 007F4580, constructs each wing plane in the
  squadron's InitAll pass A, through the vehicle class's `vtable[28h]` (007CFD20 for a plane
  Type) at 007F4811. Each construction reaches 00928760 CALL 00926BE0, so the planes join the
  pending list while that pass A walks it, and the walk reaches them in the same pass.
- **The host until now.** The script-orders creator batches built the wing beside the leader, in
  the creator's own `create_units` call:
  - the air-ops launch 006C5050, `create_air_ops_squadron_006c5050`;
  - GenerateObject and SpawnNew, `create_unit_from_scene_record_0046db4b`.
  The Lua host's pass A then appended the batch's range as the wing.

### 30.2 The binding (this half)

- **The creator batches** build the leader only. The plan's wing records go to
  `GameUnitsHost::stage_squadron_wing(leader_index, records)`.
- **The registry record's member slots.** Both batches pre-register `member_units` as
  `{leader, NoUnit, ...}` in plan order, together with `member_names` and `member_spawn_index`.
- **The pass A hook,** `on_squadron_pass_a_construct_wing(leader)`, takes the staged records and
  builds them through `create_units`. That pushes each plane (00928760) while the walk is inside
  pass A. The hook then writes wing i's new index into `member_units[i]` (007F4B49's `+3D0h`).
- **Unit ids of wing planes change.**
  - A SpawnNew group constructs every member squadron's leader first. The walk's pass A then
    builds the wings squadron by squadron, so every wing plane's index and id come after all
    leaders of its group.
  - An air-ops launch's wing now follows everything built before its InitAll.
  - Everything that names a plane by index reads it from `member_units`, which the hook fills. The
    per-unit rows (gunnery, logs) are keyed by name.
- **The census:** `summary squadron wing construction bound=.. staged=.. builds=.. planes=..
  left_staged=..`.

### 30.3 Predictions (written before the pairs; the same tree, switch only, both variables set)

**This half alone** (the Lua half OFF), USN04 4700/4500:
- `staged=40 planes=40 left_staged=0`; `builds` is the number of squadrons with a wing, at most
  20 (the hook's 20 calls);
- the Lua host's `wing_appended` 40 -> 0: the route's range holds only the leader. `wing_deferred`
  (the dedup's drop of an append a construction push already queued) goes to 0 as well;
- the units host's construction pushes are unchanged in total, 40 of them now made during pass A;
- InitAll `entities=86` and `self_table_entities=86` unchanged;
- **`wing_member_tables` 40 -> 0 and `squadron_ids` 40 -> 0.** Marking the hook's pushes as wing
  nodes is the Lua half (15.4 point 2). Until it lands, a wing plane gets its slot but not
  `SquadronID`;
- gameplay: identical is expected. A plane's behaviour does not depend on its index. **The risk:**
  anything that walks units in index order, or draws RNG per index, sees the wing planes later.
  Any move is reported with its first diverging row.

**USN02 9200/9000:** no squadron is created (the hooks ran 0 times on USN02), so identical,
`staged=0`.

**The joint landing** (both halves ON), from 15.4:
- `wing_appended` 40 -> 0 and `wing_deferred` 40 -> 0;
- construction pushes 81 unchanged, with 40 during pass A;
- `entities`, `self_table_entities` and `wing_member_tables` unchanged; `squadron_ids` 40;
- identical gameplay.

### 30.4 The pairs, measured (this half alone)

- **Builds.** `tools/pair_export.py` of `212eac5a3`: `local\ri_off` (SHA-256 prefix
  `B9C6C653C435`) and `local\ri_on` (`2049997AB2E5`, `kWingConstructionInPassABound` flipped).
- **Logs.** `local\wc_{off,on}_{usn04,usn02}.log` in worktree cc9-ships. Each shows the 1600x900
  fit, the immediate present interval, its own module directory and the final COM release.

**USN02 9200/9000: pair_diff exit 1.**
- Gameplay, the death table (22 rows) and the unit table are identical.
- The native table is identical, and 0 other lines are only OFF or only ON.
- The census moved only `bound 0 -> 1`, with `staged=0`.

**USN04 4700/4500: pair_diff exit 3.** Every census prediction held:

| row | OFF | ON | predicted | held |
| --- | --- | --- | --- | --- |
| staged / builds / planes / left_staged | 0 / 0 / 0 / 0 | 40 / 20 / 40 / 0 | 40 / at most 20 / 40 / 0 | yes |
| `wing_appended` | 40 | 0 | 0 | yes |
| `wing_deferred` | 40 | 0 | 0 | yes |
| InitAll pushes, entities, `self_table_entities` | 81, 86, 86 | 81, 86, 86 | unchanged | yes |
| `wing_member_tables`, `squadron_ids` | 40, 40 | 0, 0 | 0, 0 (the Lua half's job) | yes |

**The gameplay prediction failed.**

```
  deaths                                 40                                       40
* hit records                            799                                      801
* hull hits                              306                                      308
* damage                                 11621.4                                  11662.6
* shots                                  6395                                     6388
  first hit                              93.00 s                                  93.00 s
* torpedo-task releases                  7 of 16                                  5 of 16
* dive-bomb-task releases                3 of 19                                  5 of 19
  torpedo drops                          1                                        1
DEATH ROWS: 40 -> 40 rows, 0 only ON, 0 only OFF, 12 changed
```

- **The same 40 victims die.** Twelve death rows move in range, altitude or credit.
- **The draws.** Twenty-six plane death modes change. Their draws are mostly the OFF run's values
  handed to other planes: 36 of the 40 values are shared, and 4 differ on each side.
- **The likely cause is the order, not the construction.**
  - ON, every wing plane's unit index comes after all leaders of its group. The per-step plane
    walk and the shared RNG stream (00BD2F10)
    now see the planes in a different order.
  - This is the order the image creates them in: leaders by the SpawnNew loop, then each wing in
    its squadron's pass A.
- **Not separated.** The missing `SquadronID` (the Lua half's) could also move a script path.
  The joint pair, with both halves ON, is the first measurement of the whole change. This half's
  pair shows only that the index order moves USN04's air battle within its usual bands.
- **State: committed OFF, held for the joint flip** with cc9-movie-camera's Lua half.
