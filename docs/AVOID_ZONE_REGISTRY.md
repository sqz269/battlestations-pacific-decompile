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
