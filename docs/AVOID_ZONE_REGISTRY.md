# The avoid-zone registry and the planes' layer sample (packet cc9_avoid_zone_registry)

Addresses: 004C17D0 00424730 00424680 00423960 004248A0 0041DF40 0041BC20 00417FA0 0041BAE0
007F1D90 007CE8FF 007CE92A 0099F9F3 009A05DF 009A0E1A 009D36A9 009D386D 009D3A42 009CFAD0
004218E0 006DFD90 0082ADA0

2026-09-26. Ghidra read-only; names are hypotheses. Switch `kAvoidZoneLayerSampleBound` in
`GameUnitsHost::Impl` (`src/game_hosts_units.cpp`), committed OFF with the predictions below.
Host: `src/game_hosts_avoid_zones.cpp` / `include/bsp/game_hosts_avoid_zones.hpp`.

## Two singletons, one of them new here

| singleton | getter | object | host |
| --- | --- | --- | --- |
| `[00E17620]` registry | `004C17D0` (`operator new(14h)`, `00424730`) | a `std::list` of `TerrainGridLayer` (28h bytes) | **new:** `GameAvoidZoneRegistry` |
| `[00E17624]` manager | `004218E0` (`operator new(78h)`, `00421500`) | the zone polygons, groups and spatial index | existing: `GameAvoidZoneRuntime`, owned by the ship AI host (not duplicated) |

The registry's layer record, the `.nav` grammar and the layer selection `0041DF40` are in
`docs/SCENE_CONTENTS_HOSTS.md` section 2; this host reuses that parse (`parse_terrain_grid_nav`,
`load_avoid_zones_004248a0`) and that selection (`select_terrain_grid_layer`).

**Construction and population.**
- `00424730` leaves one default layer in the list. It is `00424680` with `[00CE3990]` (ten
  degrees), which calls `00423960(10)`: n = 10, half extent 12000 (`[00CE3968]`), cell
  24000 / 10 (`[00CE3960]`), scale 2.0 (`[00CE3958]`), and a grid zeroed by `memset`
  (`004239C7`). Slope +14h is tan(10°) through `00412E20`.
- `004248A0` (reached from `004D5251` when the scene's `.nav` opened at `004D5232`) appends the
  file's layers without clearing. **Host divergence:** each scene load starts from the
  constructor's state. Every measured run loads one mission per process.
- The consumers fetch the singleton at `007F1D93`/`007F1DB2` (the squadron's layer selection),
  `007F5239`/`007F5255` (no Ghidra function; contract unread) and `00880CEE` (the reset
  `00423AF0`, not bound).

## The sample `0041BC20`

`float __thiscall(layer, float x, float z)`, `RET 8`.
- `00417FA0(&cell, x, z)` (`__thiscall(layer)`, `RET 0Ch`): u = (x + half) / cell and
  v = (z + half) / cell, each stored as a float. Each is clamped to [0, n − 2.0]
  (`[00D7A308]` is the double 2.0), so `ix + 1` never leaves the grid.
- `0041BAE0(ix, iz)` (`RET 8`) clamps both to [0, n − 1] and returns
  `byte[iz * n + ix] × layer+10h` as a float (`0041BB2D`-`0041BB3E`). **Correction to
  `docs/SCENE_CONTENTS_HOSTS.md`:** that doc found no reader of `+10h`. It is this fetch's
  scale, so a cell is a height in metres, byte × scale.
- The result is a bilinear blend with float roundings at `0041BCBB` (a = h00 + fu·(h10 − h00)),
  `0041BCD1` (b = h01 + fu·(h11 − h01)), the fractions `0041BC69`/`0041BCE9`, and the result
  `0041BCF5`.

## Who samples which layer

The rel32 scan of the whole `.text` finds exactly these nine calls:

| call | function | layer (ECX) | what it produces | host site |
| --- | --- | --- | --- | --- |
| `007CE8FF` | `007CE040` plane fixed step | `[ESI+6C4h]+34Ch` = squadron+34Ch (ESI = unit+310h; `007CE877` skips both calls when unit+9D4h is null) | `[ESI+6A4h]` = **unit+9B4h** = y − layer(x, z) | `avoid_surface_height`, read at every unit+9B4h consumer (vehicle avoidance, AvoidTerrain, dogfight) |
| `007CE92A` | same | same | unit+9B8h = unit+9B4h − ((y − r.y) − layer(x − r.x, z − r.z)), r from the unit's `vtable[34h]` | not bound: no host reader of unit+9B8h |
| `0099F9F3`, `009A05DF`, `009A0E1A` | `0099F1C0` AvoidTerrain | `[ESP+48h]` = squadron+34Ch (`0099F1F3`-`0099F209`; null jumps to `009A17A1`) | the probe heights | `avoid_surface_height` |
| `009D36A9`, `009D386D`, `009D3A42` | `009D3420` torpedo approach | squadron+34Ch (`docs/TORPEDO_RUN_IN_PATH.md`) | the scan and the over-land test | `terrain_height_0041bc20` |
| `009CFAD0` | `009CFA80`, the move-to (no caller in the call graph; vtable) | `[[ESI+0Ch]+350h]`, squadron+350h | - | not bound: the move-to state does not run this sample in the host |

**squadron+34Ch** is written at `007F1DCA` with `0041DF40(1.5 [00CE380C], true)`. Neither
argument belongs to the squadron, so every squadron holds the same layer. tan(1.5) = 14.1, so
the query keeps the largest slope limit below it: the file's tan(70°) layer. squadron+350h
comes from the squadron's own record (`docs/PLANE_SQUADRON.md`) and is the move-to's only.

## This installation's layer on the measured missions

All 13 `.nav` files under `universe/Scenes/missions/USN/` are one file: 172,939 bytes, md5
prefix `ec09b4bbff20` (eleven dated 2024-07-13, `usn_ormoc.nav` and `usn_sibuyan.nav` dated
2024-10-29 with the same bytes). So Marshall (USN01, `usn_1_marshall.nav`) samples this same
generic layer, not its atolls. The squadron+34Ch choice was re-checked against `0041DF40` for
the Mavis and the Devastators (packet cc9_mavis_rack_drops, docs/RELEASE_ISSUE_STAGE.md): all
three call sites of `007F1D90` query (1.5, true), so every squadron holds layer index 3.

`usn_19_coralus.nav` (USN04) and `usn_2_java.nav` (USN02) are the same file (md5 prefix
`ec09b4bbff20`, 172939 bytes, 2024-07-13). The tan(70°) layer: n = 240, cell 100 m,
scale 2.0022; 40.96 % of cells are nonzero, and the maximum byte is 200 (400.4 m). A coarse map,
rows z from −12000 (top) to +12000, columns x from −12000 to +12000, digit = max height / 50 m:

```
....0015765877771664
......00123333110...
......01123345431...
......010113455410..
.......00113344550..
.........0112345530.
...........002353420
............00325520
..............024422
.............0144432
............00245542
............01234553
............00133644
............01113424
...........011111311
...........022112321
...........122145422
..........0244245421
..........0144334433
..........0112444464
```

The scene (`usn_19_coralus.scn`) places the Lexington group at (−13000, −13000), past the grid
edge. It clamps to the zero corner at top-left, where the sample is 0.0 m. The Yorktown group
is at (13000, −13000), which clamps to the top-right corner cell, where the sample is 86.1 m;
the eastern block rises to 350-400 m just inside the grid. The Japanese carriers at
(0, 13000-14500) clamp to the bottom row at x = 0, where the sample is 0.0 m. The Coral Sea
carrier path point (8000, −8000) samples 16.0 m, and the centre (0, 0) samples 0.0 m. These
are Python samples of the file with this doc's formula.

## The binding

`GameAvoidZoneRegistry` (the new file) is filled by the scene load: one hunk in
`src/game_hosts_scene_contents.cpp`, after `load_avoid_zones_004248a0`. With the switch ON,
the units host's `avoid_surface_height` and the torpedo approach's `terrain_height_0041bc20`
return `avoid_zone_sample_0041bc20` on the registry's squadron+34Ch layer. AvoidTerrain's cell
size is min(layer+0Ch, 120). In both builds the host computes the sea stand-in and the layer at
every sample and prints a census line (`summary mission avoid-zone layer 0041BC20`).

**Substitutions (labelled):**
- unit+9B4h is sampled when a consumer reads it, not stored by the fixed step.
- It is sampled for every plane. The image leaves unit+9B4h stale when unit+9D4h is null.
- unit+9B8h (`007CE92A`) and the move-to's +350h sample are not bound.

## Predictions (written before the runs)

Same-tree pairs `local\az_off` against `local\az_on`, the switch only, streams and death table on,
one run at a time.

| row | USN04 9200/9000 and 4700/4500 | USN02 9200/9000 |
| --- | --- | --- |
| census, OFF | `registry_layers=4 scene_loads=1 squadron_34c=3 (n=240 cell=100.0 scale=2.0022 slope=2.747478)`; sea within about ±1 m; `layer_nonzero` well above 0 (planes over the eastern block and the Yorktown edge); `layer_max` 150-400 m | same census shape |
| what the layer returns where the sea stood in | 0 over the western open water, the Lexington corner and the Japanese carriers (vs the wave height, under 1 m); 86.1 m at the Yorktown group's clamped corner, and up to 400.4 m over the eastern block | - |
| unit+9B4h | ON: smaller by the layer height over the east; unchanged within 1 m in the west | - |
| AvoidTerrain rows (`tr_` census, bands) | ON: more bands and pull-ups for planes near the Yorktown group | - |
| torpedo approach | the one torpedo drop (Kate #4.1\|.-4 on Fletcher-class05, Yorktown group) may be refused or moved by the over-land test | - |
| dive releases | 3 dead-Val requests at the same times (they fly at the Lexington, in the zero corner) ± 1 | - |
| deaths, hit records | plane deaths ± 10, hit records ± 150 (the eastern attacks change) | - |
| the Lexington's movement line | ± 200 m | - |
| ship rows | move only through the plane attacks above | **identical**: the registry's construction moves no ship row, and no ship reads the sample while the probe is unbound |
| identical rows | everything before the first plane sample over a nonzero cell | every ship row; plane rows may move if a USN02 plane flies over the layer |

## The ring-scan probe's contract over the manager (for cc9-gunnery2)

Rows 9-11 of `docs/UNIMPLEMENTED_RANKING_3.md` are the three calls of `009E6640`
`BSP_ShipAi_ApproachProbeSlotClearance` that `RingScanProbeBinding` in
`src/game_hosts_ship_ai.cpp` records. The manager they reach is `[00E17624]`, which is
`GameAvoidZoneRuntime` (`include/bsp/game_avoid_zone_runtime.hpp`) in this host. None of the
three touches the registry.

| call | image | ABI | contract | over the runtime |
| --- | --- | --- | --- | --- |
| `009E66EB` `unit->vtable[218h]()` | `006DFD90` in every ship vtable at +218h: `00CF92C8`, `00CFA990`, `00CFB950`, `00CFC5E8` (`MDestroyer` `00CFC3D0` + 218h), `00CFFC48`, `00D01848`, `00D09890` (ship base `00D09678` + 218h), `00D0C860` | `__thiscall(unit)`, no arguments, `RET`; body `006DFD90`-`006DFD9D` inclusive (**no Ghidra function**; INT3 at `006DFD8D`-`006DFD8F` and from `006DFD9E`) | `ECX = [unit+538h]` (the class descriptor); `0082ADA0(0)`: `004218E0` fetches the manager, then `004120D0(manager, [desc + 0*4 + 560h])` returns the **zone group** for the class's navigation layer (class+560h is 11 for every surface class in this installation, `docs/AVOID_ZONE_ESCAPE.md`) | `runtime.group_for_layer(class_560h)` returns the 1-based group token that `004120D0` names. It throws unless `ready()`, so the binding checks `ready()` first |
| `0082ADA0` | `__thiscall(desc)(int depth_index)`, tail-jumps to `004120D0` | ECX descriptor; stack index; returns EAX group | `004120D0(manager, [desc + index*4 + 560h])`; callers `00812C90`, `00852FF0`, `00941D30`, `00A02020`, and `006DFD90` with index 0 | as above, with `[desc+560h+4*index]` |
| `0082ADC0` | `__thiscall(desc)()`, `RET` | - | `004120D0(manager, [desc+570h])`; callers `009E1950`, `009E85B0`, `009F1BC0` | `runtime.group_for_layer(class_570h)` |
| `009E673E` `00417B10` | `BSP_AvoidZoneGroup_OffsetPointSequential` | `__thiscall(group)(float2* out, const float2* point, float push, char test_containment)`, `RET 10h`, EAX = out; the probe passes (out, in, 3.0f, 1) | copies the point, then walks every zone of the group in order and pushes the running output out of each zone it lies in, by `push` | **gap:** the runtime has no method for it. The reconstruction exists (`avoid_zone_group_offset_00417b10`, `include/bsp/ship_ai_layer_selection.hpp`), and a runtime method `offset(group, point, push, test)` over the runtime's own group views is the missing piece |
| `009E6808` `0041B4E0` | a thunk, body `0041B4E0`-`0041B500` | `__thiscall(group)(const float2* toward, const float2* from, float2* running)`, `RET 0Ch`, AL = hit | forwards to `004179D0` `BSP_AvoidZoneGroup_SegmentHit(toward, from, running, &zone, &edge)`, using its own argument slots as the zone and edge outputs (discarded); `running` holds the hit point when AL is set | the runtime's `segment_point(layer, toward, from, hit)` goes through the manager's all-group `00417EF0`, not one group. The group-level twin (`avoid_zone_group_segment_hit_004179d0` on `groups[token-1]`) is the faithful binding and is not exposed yet |

Two further notes:
- **Order.** `009E6640` asks for the space, refreshes the pose when `[unit+0C8h]` is clear
  (`00414DB0`), offsets the start point with `00417B10`, then casts the probe with `0041B4E0`.
  The binding needs the runtime to be `ready()`, which it is after `load_avoid_zone_geometry`.
- **A second consumer of `0041B4E0`** is `006AC5D0` (not a ship AI routine; contract unread).

## The pairs, measured

The logs are in worktree cc9-plane-release: `local\AZ_OFF_9000.log` / `AZ_ON_9000.log`,
`AZ_OFF_4500.log` / `AZ_ON_4500.log` and `AZ_OFF_USN02.log` / `AZ_ON_USN02.log`. All six show the
1600x900 line and a module directory in that tree. The OFF binary is `f49d492e5`'s. The ON binary
is `697e8cc48`'s source with the switch set, which only moves the sea return into an `else` arm
(C4702 under /WX). The diffs ignore pointers, the harness slot lines and the refill counter.

| row | USN04 9000 OFF -> ON | USN04 4500 OFF -> ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| census | `registry_layers=4 scene_loads=1 squadron_34c=3 (n=240 cell=100.0 scale=2.0022 slope=2.747478)`; samples 2,627,020 -> 2,619,271; layer nonzero in 761,112 -> 753,859; `layer_max` 394.61 m | same shape; 1,878,897 -> 1,871,170 samples, 371,235 -> 364,018 nonzero, max 330.36 m | as written; `layer_max` 150-400 m | held |
| what the sea returned | **exactly 0.000** at every sample (`sea=[0.000 0.000]`), not ± 1 m | same | ± 1 m | **failed**, harmlessly: the stand-in was a flat 0, so the layer's value is the whole difference (mean 24-25 m) |
| terrain avoidance (0099F1C0) | ticks 1284 -> 1720, bands 3520 -> 4538; the only planes that now avoid are the Yorktown's own squadrons (`Yorktown-class01_sqn02`, `sqn04`: 26-114 terrain ticks each, `min_margin` 61-64 m) | same | more bands near the Yorktown group | held |
| torpedo approach scans (009D37AE) | Kate flights #4.1 and #8.1: clear sectors 36 -> 30-32 and 36 -> 23 of 36, home sector 0 -> 5 or 6 | same | may change | held |
| torpedo drop | identical (Kate #4.1\|.-4, 13 m, 73.1 m/s) | same | may be refused or moved | held; not moved |
| dive releases, bombs | identical: 3, 0 | same | 3 ± 1 | held |
| deaths, death table, hit records | identical | identical | ± 10, ± 150 | held; nothing moved |
| the Lexington's movement line | identical | identical | ± 200 m | held |
| plane motion | distance 838,451.16 -> 838,448.31 m; the Yorktown CAP's final ranges move by about 3 m | 609,018.97 -> 609,020.32 m | - | - |

| row | USN02 9000 OFF -> ON | prediction | verdict |
| --- | --- | --- | --- |
| registry | built (`registry_layers=4 scene_loads=1`; the scene's `.nav` parsed into 3 layers) | built | held |
| samples | 0 on both: no plane samples the layer in this run | plane rows may move | held |
| every ship row | identical | identical | held |

**Verdict: `kAvoidZoneLayerSampleBound` ON.** The planes read the scene's avoid-zone layer where
the image reads it, and its only measured consequence is the Yorktown's fighters flying their
terrain avoidance over the eastern block. No death, hit or release moved on either mission.

## Units contracts (packet cc9_units_contracts)

2026-09-26. Switch `kUnitsContractsBound`, committed OFF with the predictions below.

### unit+9B8h, `007CE92A`

- **The formula.** The second sample of the fixed step uses the same squadron+34Ch layer as
  unit+9B4h. r is the unit's `vtable[34h]` (the world velocity). The value is
  unit+9B8h = unit+9B4h − ((y − r.y) − layer(x − r.x, z − r.z)). That is the height above the
  layer now, less the height above the layer one second back along the velocity. The three
  differences are floats (`007CE8D4`, `007CE8E1`, `007CE8EE`), y is kept as a double
  (`007CE8F4`), and the store is `007CE941`.
- **Its readers** come from a disp32 scan of `B8 09 00 00` and of the unit+310h form
  `A8 06 00 00`. False hits were removed: `007C8139` is `MOV EAX,9` and `007BFFCC` is a `JE`
  offset. None of the real readers is on a path this host runs:
  - `007B9620` reads unit+9B4h and +9B8h, and has no reference anywhere (rel32 and absolute
    scans).
  - `007C2B6B` in `007C2AF0` and `007D18CE` in `007D1360` are the plane state-message arms.
    This host delivers no plane state message.
  - `007DA24F` in `007D9F60` is `007C6500`'s alternate pose branch on unit+210h.
- **The binding** stores it per plane (`plane_height_rate_9b8`) under the switch, when the
  layer sample is bound too. It skips a plane with no squadron, as `007CE87F` does. Nothing
  reads it; a census line prints its range.

### The +350h sample `009CFAD0` is the takeoff task's, not the move-to's

**Correction** to this doc's table and to `docs/COMMAND_COMPLETION.md`'s "moveto" row.
`009CFA80` is called only from `009CFD8B`, in `009CFD70`: `__thiscall(task)(float dt)`,
`RET 4`, body `009CFD70`-`009CFDC3`, **no Ghidra function**. `009CFD70` is slot +64h (the tick)
of the vtable at `00D21228`, whose string after the table is `takeoff` (`00D21290`). It calls
`009CFA80` on the sub-object at task+3F8h (`[+4h]` the unit, `[+0Ch]` the squadron). That writes
sub+2Ch = max(layer350(x, z) + sub+30h, sub+30h + sub+34h), an altitude floor, then checks
the moveto command's completion (`0071E430`, not terminal). This host has no takeoff task, so
the sample stays a named record.

### Contract 1: `GameUnitsHost::unit_part_descriptor_count(std::size_t index) const`

It returns (unit+34Ch − unit+348h) >> 2.
- `0087BCC0` sizes the part table at unit+344h to the class descriptor's +18h vector: the count
  (+20h − +1Ch) / 30h at `0087BD20`-`0087BD43`, resized at `0087BD4F`. It then stores one
  pointer per record (`0087BDBC`).
- That vector's records are `Damage.Sections`, appended by `0087CA80` (a 30h record zeroed at
  `0087CE00`, pushed at `0087CE48`).
- The host counts the class row's `Damage.Sections` entries with a Lua `pairs` walk, cached
  per class. It returns 0 past the end.

### Contract 2: list 24 and `GameUnitsHost::squadron_list_24_leader(std::size_t node) const`

- **What the image does.** `007F10B0` is slot +130h of the squadron vtable `00D087C0`
  (`00D088F0` − `00D087C0` = 130h). That is the per-class registrar the entity creation calls
  after placement. It pushes the squadron into list 1 (`00928560`), list 2 (`[+30h]+30h`) and
  list 24 (`[+30h]+138h`).
- **When.** The push happens **at squadron creation**. For a carrier strike, creation is the
  launch: `006C7490` → `006C5050` builds the squadron through `004F0AD0`, the same creator a
  `PlaneSquadronGen` scene row uses (`docs/AIROPS_LAUNCH_START.md` section 3). A scene squadron
  is pushed at scene load.
- **The host.** `world_list_size(24)` and `world_list_entry(24, i)` now answer. Each node is
  the member slot the host fuses with the squadron: `squadron_unit`, else its first member.
  `squadron_list_24_leader(node)` returns [squadron+3D0h], the current flight leader, or the
  unit count when no member is alive. The consumer should read the squadron's side and position
  through that leader.
- **Substitutions, labelled:**
  - Lists 1 and 2 are not pushed, because the fused slot is already registered as a plane.
  - The push happens at the first units step after the record has a unit, at most one frame
    after creation.
  - Nothing removes a node, so the consumer's live test is the leader.

### Predictions (written before the runs)

Same-tree pairs `local\uc_off` against `local\uc_on`, the switch only, streams on.

| row | USN04 4700/4500 | USN02 9200/9000 |
| --- | --- | --- |
| `summary mission units contracts` | OFF `list24_size=0 list24_pushes=0`, `9B8h writes=0`; ON `list24_size=21 list24_pushes=21` (the 21 squadrons the formation lines name), 9B8h writes between 100,000 and 183,221 (the plane fixed steps), range within ±400 m | no squadrons: `list24_size=0` on both; 9B8h writes 0 |
| `list6` and `with_parts>1` | printed identically on both sides | identical |
| `squadron world list 24 push` lines | ON only, 21, the first at mission start for the scene squadrons and the rest at each launch | none |
| natives | ON: `PlaneSquadron::register_in_world_lists_007f10b0` 21, `PlaneStep::height_rate_9b8_007ce92a` concrete; `UnitList::push_back` + 21 | identical |
| gameplay | identical: the proximity scan, the only list-24 reader, is not bound on this tree (`WarningManager::scan_proximity` UNIMPLEMENTED), and nothing reads unit+9B8h | identical |

### The pairs, measured

The logs of record are `local\UC3_OFF_4500.log` / `UC3_ON_4500.log` and `UC3_OFF_USN02.log` /
`UC3_ON_USN02.log` in worktree cc9-plane-release, from `b8eac497e` with the switch only and
streams on. All four show the 1600x900 line, a module directory in that tree and the final COM
release.

Two earlier pairs are superseded by these:
- The pair from `a7bb64b93` (`UC_*`) and the pair from `c374fb619` (`UC2_*`) both printed
  `with_parts>1=0` for every ship.
- The Lua chunk that counts `Damage.Sections` is about 270 bytes, and it was formatted into a
  256-byte buffer, so it never compiled. `b8eac497e` fixes the buffer.
- Their list-24 and 9B8h rows equal the rows below.

| row | USN04 4500 OFF -> ON | USN02 9000 OFF -> ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| list 24 | 0 -> 21 (`lists{... 24=21}`); `UnitList::push_back` 486 -> 507 | 0 -> 0 (no squadron) | 21 / 0 | held |
| push times | movieval at 0.00 s; the first eight strike squadrons at 25.10-26.75 s (their launches); the four US CAP squadrons at 27.05 and 30.05 s; the second wave at 105.05-106.55 s | - | at creation, launches for the strikes | held |
| 9B8h | writes 0 -> 163,705, range [−228.74, 71.53] | 0 -> 0 | 100,000-183,221, within ±400 | held |
| `list6` / `with_parts>1` | 18 / 18 on both sides | 32 / 32 on both sides | printed identically | held. It answers the scan's stand-in: every list-6 ship carries more than one part |
| natives | `PlaneSquadron::register_in_world_lists_007f10b0` 21, `PlaneStep::height_rate_9b8_007ce92a` 163,705, concrete | identical | as written | held |
| gameplay | identical: deaths, hits, releases, every per-entity and gunnery row | identical | identical | held |

The only other diff lines are the ignored refill counter, the harness slot lines, and on USN02
one pre-window sound-startup count (`fmod_calls` 131 against 132). That count is taken before the
window and the mission exist, so the switch cannot reach it.

**Verdict: `kUnitsContractsBound` ON.** List 24 answers as the image fills it, the part count
answers from the class data, and unit+9B8h is stored. None of them has a reader on this tree
until the proximity scan's consumer side is bound to `unit_part_descriptor_count` and
`squadron_list_24_leader`.
