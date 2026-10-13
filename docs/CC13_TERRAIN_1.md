# Segments 80-82: terrain, foliage and the particle band (cc13-terrain1)

Segments 80-82 are `00AD5590..00B1BD20`: the foliage system and its type table, the terrain
height field with its grids, blocks, tiles and quadtree, the bush and tree groups, texture
atlases, the particle band (emitters, parameters, tracers, lights) and the tail of the scene
renderer. Names are descriptive hypotheses, not recovered symbols.

Status words: *reconstructed* (C++ in `src/terrain_particles_cc13.cpp`, declarations in
`include/bsp/terrain_particles_cc13.hpp`, namespace `bsp::cc13_terrain`), *build-tested*
(`scripts/build.ps1` passes). Nothing here is fixture-tested, ABI-compatible or
game-validated. Objects are addressed in their image layout (byte offsets on `void*`, Win32
pointers); every image routine a body calls is a virtual of a host interface named by the
address it stands for, so nothing is stubbed and no global is invented (image globals are
`TerrainHost::read_global` / `write_global`). x87 expressions are evaluated in double and
rounded to float wherever the native code stores a float; the order of operations was read
from capstone over the disk bytes (`local/tr1_x87.py`, `local/tr1_dis.py`), never from
Ghidra's listing text.

## Triage

At `origin/main` (index of 2026-10-12) the range held 327 functions with a reviewed ledger
name, no reconstruction record and no tag (173 had no export; exported with
`bsp.py ghidra export --force`). An address grep over `src/` and `include/` found:

* cc9's terrain host (`src/game_hosts_scene_contents.cpp`, `SceneTerrainHeightField` and the
  quadtree/segment walk) already reconstructs 18 of them without records. Their records are
  backfilled here (status `reconstructed_ledger_backfilled_from_source`); the code is
  untouched. `00ADA240`, `00ADA420`, `00ADA890`, `00ADD290`, `00ADE500`, `00ADE820`,
  `00ADFD70` and `00ADFEB0` are folded into cc9's loaders and walks without a
  separable function and keep no record.
* `src/particle_emitters_cc13.cpp` (6) and `src/scene_renderer_cc13.cpp` (12, plus the ten
  std::sort helpers of the stats overlay) are backfilled the same way.
* The remaining bodies are new here.

Shared-shape records (scope `shared_shape_model` / `table_model`) point at one function or
table entry: the native bodies are identical after relocation or differ only in the element
size, a vtable or a global named in the table.

## Corrections to ledger names

| Address | Ledger name | Finding |
| --- | --- | --- |
| 00AD56C0 | `BSP_TerrainRenderObject_SetField3C_Provisional` | ECX is the foliage system at both callers (00ADAC90 loads [00F8C210]; 00ADBF80 uses height field +5Ch): foliage system +3Ch, the value new groups receive (00AD8C60 / 00AD8DD0 read +3Ch). |
| 00AD9AC0 | `BSP_FoliageSystem_LoadTypes` | the A8h foliage system constructor; the FoliageTypes.lua parse is 00AE84E0 (ECX = [00F8C264]). |
| 00AD97D0 | `BSP_FoliageTypeRecordVector_Destroy` | the 18h FoliageTypeGroup destructor: record vector at +08 (00AD9520 called with ECX = group+8, 00AD97ED), then the name string +00/+04. |
| 00AD9880 | `BSP_FoliageTypeRecordVector_EraseAll` | group range destroy, ECX = first, EDX = last, 18h stride (00AD989B..00AD9914). |
| 00ADB160 | `BSP_TerrainByteLayer_SampleClamped` | writes: the clamped cell becomes max(cell, value). |
| 00ADF6B0 | `BSP_TerrainRenderBlock_Construct` | the 38h tile (vtable 00D5D418): +1Ch block, +20h second block, +24h node, +28h..+34h ref slots. |
| 00ADF7E0 | `BSP_TerrainBlockU8_Destroy` | the destructor of that tile. |
| 00ADB020 etc. | `TMPL_PointerArray_Clear_*` | the layout is the {width, height, cells} grid of 00AED690. |
| 00AE0250 | `BSP_TerrainHeightField_WriteNodeChunk` | ECX is a tile: the NODE chunk writes tile +0Ch/+10h and the tile's block. |
| 00AE0710 / 00AE2C80 | `BSP_TerrainRenderObject_SetFlag40/38` | bush / tree group enabled setters (+40h with mesh +2Ch; +38h with mesh +28h), the per-element calls of 00AD71C0. |
| 00AE9B50 | `BSP_TerrainTileGrid_GetTileOrigin` | copies the tile's height range +14h/+18h (default -1000 / 1000). |
| 00ADAC90 | `BSP_TerrainRenderObject_LoadFoliageDefault` | height field: +5Ch = foliage system, its +3Ch = field+0Ch, then 00AD92C0(field). |
| 00AE22D0 | `BSP_BushGroupModel_RegisterDiffuseFactor` | ECX is a bush group (materials +3Ch, +38h). |
| 00AED820 | `BSP_TerrainFloatLayer_GetQuantized8` | truncates (chop-mode FISTP), does not round. |
| 00B1B980 | `BSP_LuaGlobals_GetField` | a float4 global with the default (1, 1, 1, 1). |

The ledger names are left as they are; these rows are for the next naming pass.

## Segment 80: foliage

Foliage system (A8h, [00F8C210]): `+14h..+20h` vector of NativeStrings, `+24h` / `+30h`
pointer arrays (tree / bush lists), `+3Ch` group parameter, `+40h` node, `+48h` pointer array
of 48h ref objects, `+54h..+60h` vector<BushGroup*>, `+64h..+70h` vector<TreeGroup*>,
`+7Ch/+84h/+8Ch/+94h` texture and atlas names (docs/CC13_FOLIAGE_TERRAIN_SETUP.md), `+9Ch/+A0h`
atlas-loaded flags, `+A4h` ref-counted bush effect.

* `00AD7B90` remove landscape: bush groups whose landscape name matches (equal length then
  `__stricmp`; empty equals empty) and whose +14h is set are detached from +30h (00AE1E00,
  ECX = group), destructed (00AE1C80), freed and erased; trees likewise with +08h, +24h,
  00AE4F30, 00AE4C20; ref objects with +44h == 0 are deleted and removed; then each loaded
  atlas is unloaded (00AEFA30) and its names cleared. The landscape-null path skips the group
  loops but still runs the atlas part.
* `00AD8C60` / `00AD8DD0` find-or-create: trees match landscape and type, bushes the landscape
  only; new groups are 3Ch / 70h allocations, appended through `_Insert_n(last, 1)` when full.
  The bush form takes its two NativeStrings by value and releases them on both paths.
* `00AD9050` / `00AD91C0` loaders: the argument order was taken from the stack arithmetic at
  00AD90C0..00AD912E (bushes: 00AD8DD0(landscape, a, b), 00AE2450(group, stream, count, a, b),
  `a` the first string read) and 00AD922C..00AD9255 (trees: 00AD7030(model), 00AD8C60(landscape,
  type, x), 00AE5870(group, stream, v)).
* `00AD9570` destructor order: +A4h, the ref objects, node +40h, the type table through
  [00F8C264] vtable +0(1), strings +94h/+8Ch/+84h/+7Ch, vectors +64h/+54h, arrays +48h/+30h/+24h,
  the string vector +18h (00432050), base 00AD69F0.
* Singleton bases (00AD6810/00AD68B0, 00AD6950/00AD69F0, 00AD9D00/00AD9DA0/00AD9E90,
  00AF1140/00AF11E0): one table. 00AF1140/00AF11E0 do not bump the lock depth counter.

## Segment 81: height field, grids, blocks and tiles

* Slots added beside cc9's: `00ADA0D0` (grid to world x/z), `00ADA720` (world bounds; the
  maximum corner omits the +80h/+84h origin, as in the bytes), `00ADA810` (world to cell,
  `_ftol2` truncation), `00ADA990` (returns slot48 + slot50 at the same grid point:
  00ADAA0B / 00ADAA2A / 00ADAA2C FADD), `00ADB550` (byte height: byte / 255.0 * 60.0, doubles
  00CE4B48 / 00CE3D68), `00ADB620` (bilinear over slot 4Ch; call order h11, h01, h10, h00).
* Height-range scans `00ADB280` / `00ADBD00` share one loop (FCOMI; an unordered compare takes
  the tile value); sentinels 100000/-100000 and 1e10/-1000.
* Grids {w, h, cells}: seven resize bodies (element sizes 1, 2, 4, 0Ch; two fill forms that
  write *fill into every non-overlap cell, 00ADAE26..00ADAE4C) and ten clear bodies.
* Blocks: base 00AED9A0 (+18h/+1Ch = 1e10/-1e10, a zero-filled byte grid at +20h) and the
  float / U16 / U8 derivations; destructors by table. `00AED020` is cc9's `tile_range_00aed020`.
* Tiles: 00ADF6B0 / 00ADF7E0 / 00ADF8B0 (SetBlock re-reads the range through block slot 0Ch:
  lo at the first pointer, hi at the second, 00ADF8F8..00ADF915).

## Segment 82: the particle band and the renderer tail

* RNG RULE. `00AF5A30` (variation factor: `1 + spread * (int32)draw * 2^-31`, doubles
  00D5DA30 / 00D7A210), `00AF7F70` (fraction times x) and `00AF7FB0` (random sign: `-1` when
  the fraction is below 0.5 or unordered) each make one image stream-0 draw (ECX = 0 before
  00BD2FC0 / 00BD2F40). The draws are `ParticleHost` callbacks; nothing is wired into a game
  host, so a host must route them to a presentation stream or keep them off. The backfilled
  `random_int_inclusive_00b057c0` and `advance_texture_frame_00b0c8b0` (particle_emitters_cc13)
  already take a `ParticleRandom` the same way.
* Named pointer lists (14h: NativeString + pointer array) and their arrays: `00AFF290`'s miss
  path builds a 14h temporary with operator new and 00AFEF30, pushes a copy (00AFF190) and
  never destroys or frees the temporary (00AFF30C..00AFF37B), so the image leaks 14h plus the
  copied name and value array per new name. The model keeps the leak.
* `00B00950`: format 16h (X8R8G8B8) goes to 00B4C6D0 (alpha from non-black), not to the
  premultiply routine the ledger mentions; FOURCC formats at or above 'DXT1' are skipped.
* `00B0D720`: `1/w` stays in an x87 register (00B0D782 FLD1 / FDIVRP) and multiplies the
  three components.
* `00B1B980` reads a float4 global by name with the default (1, 1, 1, 1); the ledger name
  (`BSP_LuaGlobals_GetField`) undersells it.
* `00B199A0` returns AL = 1 for the two type ids at 00F8D428 / 00F8D42C; the "return
  00F8D400" in the decompile is AL = 0 (low byte of that address).
* `00B05400` is a bare `RET 4`.
* The backfilled stats-overlay sort (`00B148E0` and its nine helpers) is library-equivalent:
  `scene_renderer_cc13` sorts with std::sort and the image's comparator, so equal keys may
  order differently.

## Verification notes

* Every x87 body (`00ADA0D0`, `00ADA720`, `00ADA810`, `00ADA990`, `00ADB550`, `00ADB620`,
  `00AED730`, `00AED820`, `00AF5A30`, `00AF7F70`, `00AF7FB0`, `00AEA900`, `00B0D720`) was
  read from capstone over the disk bytes; no DC-form FMUL occurs in them (the reg-reg forms
  are FSUBP/FADDP/FMULP/FDIVRP, read with their operand order).
* Argument counts come from the RET immediates (`local/tr1_ret.py`) and the push order at
  the call sites quoted above.
* Stack-slot provenance for 00AD9050, 00ADB550 and 00ADB620 was traced by hand from the
  pushes (recorded in the comments of the header).
* `tools/verify_report_calls.py` takes a JSON call report; this packet has none, so it was
  not run. `scripts/build.ps1` passes at every commit.

## Open items

Not reconstructed (no record), in rough order of value:

| Address | Ledger name | Why it is open |
| --- | --- | --- |
| 00AE2450 / 00AE5870 | BushGroup_Initialise / TreeGroup_Initialise | large stream readers that build vertex streams and generated models; need the vertex format and model calls mapped |
| 00AE5160 / 00AE1E10 | TreeGroupType_Construct / BushGroupType_Construct | large constructors (material and texture setup) |
| 00AE1A10 | BushCell_CullAndEmitLod | recursive cull with draw-section emission; terrain_render.hpp models its plane test only |
| 00AE07C0, 00AE12F0, 00AE13D0, 00AE15D0, 00AE16F0, 00AE17D0, 00AE1870 | BushGroupModelPool_* | the Mit fixed-block pool family (18Ch elements, 8 per block); existing pool models (e.g. directional_light_pool) are per-class and own real CRITICAL_SECTIONs, so this wants the same treatment |
| 00AE1FB0 | BushGroupModel_BuildBushObject | canonical model slot + generated model + far plane |
| 00AEF280, 00AEFA30, 00AEFF40, 00AEF090 | atlas register / unload / load directory / registry destruct | VFS and atlas parse plumbing (texture_atlas.cpp reconstructs the parse 00AEEAF0) |
| 00AF0F20 | FoliageGroupManager_CreateFloatingParticles | generated-model creation with ParticleFloating.mshd / .mvfm |
| 00AFB7F0 / 00AFBA40 | ParticleBallistic_Max/MinHeight | long x87 bodies (sqrt solve); started at 00AFB7F0, not finished |
| 00AFDCF0 | NativeParticleEmitter_UpdateTransform | matrix composition through four helpers |
| 00ADB6F0, 00ADB980, 00ADC1C0, 00ADC930, 00ADE0E0 | height-field stamp, get-or-create tile, TGA dump, visibility-group builders | large terrain bodies with x87 |

Folded into cc9's loaders without a separable reconstruction (no record by design):
00ADA240, 00ADA420, 00ADA890, 00ADD290, 00ADE500, 00ADE820, 00ADFD70, 00ADFEB0.

## Handoff

Packet state at the final commit: 293 of the 327 targets have reconstruction records
(247 new in `src/terrain_particles_cc13.cpp`, 46 backfilled for cc9's terrain host and for
`particle_emitters_cc13` / `scene_renderer_cc13`); 26 bodies are open (table above) and 8
are cc9-folded. Build passes. Ledger names were not changed; the corrections table lists the
renames a naming pass should apply (00AD56C0, 00AD9AC0, 00AD97D0, 00AD9880, 00ADB160,
00ADF6B0, 00ADF7E0, 00AE0250, 00AE0710, 00AE2C80, 00AE9B50, 00ADAC90, 00AE22D0, 00B1B980).

To continue: the worker scripts are in this tree's `local/` (`tr1_rec.py` holds every record
and is re-runnable; `tr1_left.py` lists what is open; `tr1_live.py` decompiles live when the
shared exports are being regenerated; `tr1_x87.py` / `tr1_dis.py` read x87 from the bytes).
Start with the two group initialisers (00AE2450 / 00AE5870): they are the spine of the foliage
load path that 00AD9050 / 00AD91C0 now reach through `FoliageLoadHost`.

## Pass 2 (cc13-terrain2)

- **Bush model pool** (00AE07C0, 00AE12F0, 00AE13D0, 00AE15D0, 00AE16F0, 00AE17D0,
  00AE1870): one more instance of the Mit slab pool engine shared with the GUI widget pools.
  The traits are in `include/bsp/terrain_bush_pool_cc13.hpp` and the instance in
  `src/native_material_pools.cpp`. Each member was compared with capstone against the GUI
  Widget pool, with immediates masked. They are identical apart from these constants:
  slot 18Ch, 8 slots, slab C74h, vtable 00D5D4FC, global 00F8C228, and the stride
  division. The allocate stub is 00AE1A00, the CRT initializer 00CD77D0 and the exit stub
  00CE0B60.
- **Ballistic height extrema** (00AFB7F0 max, 00AFBA40 min): the x87 stack was traced by
  hand from the listing, and the results are in `src/terrain_particles_cc13.cpp`.
  - The curve is y = ((a u + b) u + c) u + d; its stationary points are the roots of
    3a u^2 + 2b u + c, with disc = 4b^2 - 12ac.
  - A root must lie strictly between x0 and other.x, and the sign test is 6a u + 2b.
  - The fallback is the larger y (max) or the smaller y (min) of the two points; NaN
    picks other.y in both.

Still open: the group initialisers and type constructors, 00AE1A10, 00AE1FB0, the atlas
group, 00AF0F20, 00AFDCF0 (needs its register arguments read from the listing), and the
five terrain bodies.

## Continued by cc13-left1

docs/CC13_LEFT_1.md reconstructs 11 of the 17 left by Pass 2: the atlas group, `00AF0F20`,
`00AE1E10`, `00AE1A10`, `00AE1FB0`, `00ADC1C0`, `00ADB6F0` and `00ADB980`. Still open: `00AFDCF0`,
`00AE5160`, `00ADE0E0`, `00ADC930`, `00AE5870` and `00AE2450`.
