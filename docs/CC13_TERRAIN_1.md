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
  untouched. `00ADA240`, `00ADA420`, `00ADA890`, `00ADC6C0`, `00ADD290`, `00ADE500`,
  `00ADE820`, `00ADFD70` and `00ADFEB0` are folded into cc9's loaders and walks without a
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

## Open items

See the handoff at the end.

## Handoff

(in progress)
