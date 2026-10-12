# Leftovers after the cc13 band sweeps (packet cc13-leftovers1)

Worker cc13-leftovers1, 2026-10-12, base origin/main 7929dbd4a. Status words follow AGENTS.md:
**listing-read** means disk bytes or the Ghidra listing were read; **reconstructed, build-tested**
means C++ in `src/leftovers_cc13.cpp` compiles in `./scripts/build.ps1`. Nothing here is
ABI-compatible, fixture-tested or game-validated. No game runs were made.

Files: `include/bsp/leftovers_cc13.hpp`, `src/leftovers_cc13.cpp` (one line in
`cmake/startup.cmake`), `tools/progress_counts.py`, `reports/cc13_leftovers1_calls.json` (31 call
rows, `tools/verify_report_calls.py`: 0 failed), `reports/cc13_leftovers1_functions.json` (the
Ghidra function repair record), the census edits (`docs/CC13_RNG_STREAM_CENSUS.md` section 13 and
`reports/cc13_rng_stream_census.json`). Scripts are `local/lo1_*.py` in the worker tree.

## 1. RNG census update

Written up as section 13 of `docs/CC13_RNG_STREAM_CENSUS.md`. In short:
- An independent rel32 and absolute-dword rescan (positive control: 625 `00BD2F10` sites, as in
  the census) finds every one of the 759 census sites. It finds no stream draw the census lacks;
  the 24 extra sites are generator internals, thread registration and the separate generators.
- The sweeps' sites are settled from the listing: fish school `007093F4`/`00709440` (stream 0),
  marker phase `006D87E7` (**stream 1**, presentation; the report had a band-guess GAMEPLAY row),
  the dead fog-cloud field `006F1D40/80/C0` (stream 0), HUD plane instruments `0060AAD4`
  (**stream 1**; `docs/CC13_BAND_0060_0066.md` says stream 0 and is wrong), entity base ctor
  `00489FC6` (stream 0, U(-ClipDistance/4, ClipDistance/4)), paratrooper `007AB94A` (stream 1,
  gameplay), effect sound `00873AE0` (section 12).
- **Ten soldier/traffic stream-0 sites are gameplay**, not presentation as the report had them:
  `0048A7D7`, `0048A880`, `0048AD8B`, `0048B253`, `0048B2AC`, `0048B91B`, `0048B98F`, `0048BB01`,
  `0048BD9A`, `0048C1FD`. Stream 0 now counts 26 gameplay sites.

## 2. MGeomMesh triangle AABB tree (reconstructed, build-tested)

The GeomMesh resource parser `BSP_GeomMeshResourceParser_ParsePayload` (00727310) calls
`00725380` at `00727A76`. The chain is anchored there and in the vtable that ends at 00CFDBB8
next to the string "MGeomMesh" (00CFDBBC).

| native | name (hypothesis) | C++ | ABI |
| --- | --- | --- | --- |
| 00725380 | BSP_GeomMesh_BuildBoundsAndTrees | `build_mesh_trees_00725380` | __fastcall(ECX mesh) |
| 00724CE0 | BSP_GeomMeshPart_BuildTree | `build_part_tree_00724ce0` | __fastcall(ECX part) |
| 007240E0 | BSP_GeomMeshPart_BuildTreeNode | `build_tree_node_007240e0` | __thiscall(ECX part, node, start, count) RET 0Ch |
| 007230F0 | BSP_GeomMesh_RadixSortByAxis (evidence corrected) | reuses `radix_sort_records16_00722210` | __stdcall(axis, start, count) RET 0Ch |
| 00724A20 | BSP_GeomMesh_DebugDrawWireframe | not reconstructed | __thiscall(ECX mesh, matrix, rgba*) |
| 00725550 | BSP_GeomMeshComponent_DebugDraw | not reconstructed | vtable slot 00CFDBB8 |

How it works:
- **Layout.** Mesh: parts at +0Ch (2Ch bytes each), positions at +1Ch (0Ch each), triangles at
  +2Ch (three `int16` indices), bounds at +38h/+44h. Part: triangle list (u16) at +14h, node
  minima at +20h, maxima at +24h, node capacity at +28h.
- **Capacity.** The node capacity is n*4/20 + 1. The tree is heap-indexed (children 2i+1 and
  2i+2), and the arrays are indexed without a bound. `local/lo1_cap.py` checks every n < 70000:
  the largest node index always fits.
- **Records.** One 16-byte record per triangle: three keys, then the triangle number. Each key is
  the float vertex sum `(v0 + v1) + v2`, rounded to float after each add (00724EDC..00724F14),
  with its bit pattern flipped (00724F36..00724F48).
- **Sort.** The radix sort is unsigned, so the scratch buffer at [00E19BDC] is shared by every
  range.
- **Node.** Bounds are seeded with FLT_MAX / -FLT_MAX. A range of 20 or fewer triangles is a leaf.
- **Axis choice (listing 00724481..007244B8).** The extents are compared against **truncated**
  integers. `best = CVTTSS2SI(dx)`. If dy > best, then axis 1 and best = trunc(dy). If dz > best,
  then axis 2.
  - So an extent below 1.0 counts as 0. A box 0.6 wide in x and 0.4 in y splits on y.
  - This is the image's behaviour, kept as is.
- **Split.** The left child takes count/2 records. The right child takes the rest, through a loop
  in the image.
- **Write-back.** The sorted triangle numbers go back into the part's list.
- **Checked iterators.** An out-of-range index calls 00BF6713 (the CRT invalid-parameter report,
  which does not return). The C++ returns `GeomMeshBuild::invalid_parameter` at the same check.
  The vertex indices are MOVSX and compared unsigned, so a negative index fails.

**Task 3 answer (radix keys).** 007240E0 does not build the keys; 00724CE0 does. The keys are the
standard order-preserving float flip: a set sign flips every bit, a clear sign flips only the sign
bit. The unsigned dword order of the keys is therefore the float order, negatives included (-0.0
sorts just below +0.0). The image orders negative coordinates correctly. The reconstruction in
`src/band_0067_cc13.cpp` sorts the dwords it is given, which is faithful, so nothing there
changed. The new `float_radix_key_00724ce0` builds the keys. The band doc's open note ("a negative
coordinate sorts after every positive one unless the caller biases them first") is answered: the
caller biases them.

Uncertainty:
- 00725380's compare instructions were not read; the C++ follows the pseudocode's strict
  less-than.
- 007240E0's are FCOMI/JBE (00724248), so a NaN coordinate changes no bound.
- `.at()` on the node arrays stands in for the image's unchecked write. It can only throw beyond
  the range `lo1_cap.py` covered.
- 00725370 is an unreferenced `JMP 00724A20` thunk inside the `STL_inst_00725260` candidate.

## 3. Landscape patch index lists (reconstructed, build-tested)

`BSP_Landscape_ResetTerrainState` (00881A70, called by `BSP_Landscape_InitAttachTerrain` at
00883C09) fills the u16 triangle **index** list at landscape+3F4h for a 33 x 33 vertex patch.
Sweep 8E called these "vertex-element builders" and its ledger evidence said "vertex-element
lists". Both are corrected: they are index lists.

| native | name (hypothesis) | C++ |
| --- | --- | --- |
| 004E7690 | BSP_PatchIndexList_Reserve | `reserve_patch_index_list_004e7690` |
| 00880A60 | BSP_PatchIndexList_Push | `push_patch_index_00880a60` |
| 00880D10 | BSP_LandscapePatch_EmitInteriorIndices | `emit_patch_interior_00880d10` |
| 00880FF0 / 00881180 / 00881350 / 00880E50 | BSP_LandscapePatch_EmitRow0/Col32/Row32/Col0EdgeIndices | `emit_patch_edge_*` |
| 00881510 | BSP_LandscapePatch_EmitLodIndices (unreferenced; 00881A70 inlines it) | `emit_patch_lod_00881510` |
| 00881A70 | BSP_Landscape_ResetTerrainState (evidence corrected) | `reset_terrain_state_00881a70` |

- **Detail levels.** res = 2, 4, 8, 16, 32 cells per side; the vertex step is 32/res.
- **Variants.** For each res, 14 stitch variants are appended in the order
  0, 1, 2, 4, 8, 3, 6, 12, 9, 7, 14, 13, 11, 15. The two opposite-edge pairs, 5 and 10, never
  occur. Each variant emits:
  - the interior cells;
  - the four edge strips. A set stitch bit drops the edge midpoints, so the edge matches a
    neighbour of half the detail.
- **Dead arm.** The res == 1 arm of 00881A70 (14 copies of a two-triangle quad) is unreachable,
  because res starts at 2.
- **List object.** `[landscape+3F4h]` points to a block {data, size, capacity}. The data sits
  16-byte aligned inside the same allocation.
  - Growth is capacity + capacity/4, at least 32.
  - The reserve copies the old **capacity** in bytes (times 2), not the size.
- **Other fields.** 00881A70 also zeroes +40Ch, +3E8h, +3ECh, +3EDh and +3E0h, and seeds +410h =
  300.0f (00CE3AE8) and +3F0h = 9.375f (00D0E658).

Evidence:
- **Read in the listing:**
  - 00880D10, except 00880D96..00880DA4, which hold the store of col+s that 00880DE0 reloads;
  - 00880E50, from 00880E96 to 00880FAE.
- **Taken from the pseudocode:** the other three edge builders. They are integer-only and every
  pushed value is MOVZX-masked.
- **Variant order:** read from the pseudocode's call sequence by `local/lo1_combos.py`.
- **References** (`local/lo1_refs.py`, rel32 and absolute):
  - 00881510 has none.
  - Each builder has 15: one from 00881510 and 14 from 00881A70.
  - 00881350 also has one absolute dword at 00D7EFE4, which was not examined.

## 4. Not reconstructed, with reasons

- **Mesh decal trio, dead in this image.** `local/lo1_refs.py` finds no reference to 0073F6E0 or
  0073F1B0 except 0073F6E0's own recursion, and 0073EBA0's only caller is 0073F6E0. All three are
  named as hypotheses only. 0073EBA0 is about 1.5 KB of x87 code, and with no reachable caller
  the reconstruction was not worth it. The ledger evidence has the details.
  - `BSP_MeshDecal_CollectTreeTriangles` 0073F6E0, __thiscall(ECX ctx, node, query) RET 8:
    - Walks an AABB tree and appends 7Ch triangle records (cap 200h) that 0073EBA0 accepts.
    - **Per-axis test.** It descends when `!(q.min > node.max) || node.min > q.max`. For an
      ordered box this reduces to a one-sided test, `q.min <= node.max`, so the walk visits more
      nodes than a full overlap test would.
  - `BSP_MeshDecal_ClipTriangleToBox` 0073EBA0, args (tri, matrix, float, float*) RET 10h, ECX
    unused:
    - writes the face normal to tri+70h;
    - transforms the vertices into box space, z zeroed;
    - rejects a triangle wholly outside [0,1] in x or y, or wound clockwise in the box;
    - then runs the edge and vertex plane tests against the distance argument.
  - `BSP_Triangle_SphereContactTest` 0073F1B0, a triangle against a sphere (face, edge and
    centroid-corner planes). No reference.
- **00721C60 split.** 00721C6E was the `JG` target inside 00721C60 and relied on the EDI that
  00721C60 pushes, so it was a bogus function start. It was repaired under the lock with
  `tools/ghidra_define_function.py 00721c60 00721c8e --recreate --delete-inside`, which deleted
  00721C6E and recreated `BSP_AabbTree_CountNodes` with body 00721C60-00721C8D. The record is
  in `reports/cc13_leftovers1_functions.json`. The function itself is unreferenced, apart from its
  own two calls. It is reconstructed as `count_tree_nodes_00721c60` because it is three lines long.
- **00725550 and 00724A20.** These are the GeomMesh debug wireframe. They are named but not
  reconstructed: they need a debug-line host and are debug-only.
- **Gameplay, left for cc9:**
  - the avoid-zone byte-grid code `00880080`, `00880420` and `00880250`, called only from
    `00423640` (it calls `BSP_AvoidZoneManager_GroupForLayerOrBelow`): ship-AI clearance;
  - `00880220`, under `BSP_LuaBinding_SetShallowWater`.
- **Not reached.** The remaining raw functions in 00865000-00875FFF (effect-instance virtuals),
  listed by `python local/lo1_raw.py 00865000 00875FFF`: 78 with no ledger name and no tag or only an
  `stl_probable` bookmark. Of 00876000-0087FFFF (damageable), cc9's.

## 5. tools/progress_counts.py

`tools/progress_counts.py` reads `local/bsp_index.sqlite`, specifically `functions.name` (the
snapshot name), `tag_category`, `ledger_name`, `recon_*` and `thunk`. It uses the band docs'
definitions:
- **raw**: the snapshot name is `FUN_`, there is no tag row and no ledger name;
- **bookmarked**: a raw function whose only tag is an `stl_probable` bookmark.

It takes `--range lo-hi` and `--by-band`. The index keeps one tag row per function. Run
`python tools/bsp.py index --if-stale` first.

Counts after this packet (index rebuilt in the worker tree after the ledger edits; the Ghidra
snapshot was not refreshed):

| range | total | raw | bookmarked | ledger | tagged | recon |
| --- | --- | --- | --- | --- | --- | --- |
| whole image | 76265 | 7329 | 379 | 17994 | 17409 | 8317 (before this packet's 13 records) |
| 00720000 band | 368 | 76 | 5 | 156 | 137 | 34 |
| 00730000 band | 252 | 38 | 8 | 141 | 80 | 56 |
| 00880000 band | 252 | 107 | 3 | 78 | 68 | 26 |

## 6. Handoff

- Ledger: 17 names added, 2 evidence corrections (007230F0, 00881A70) and 13 function records.
  All are applied to Ghidra through `tools/ghidra_annotate.py`. `_free` (00BF9DC8) kept its
  prototype before and after.
- Open items:
  - the 00865000-00875FFF effect-instance raw functions;
  - the absolute dword at 00D7EFE4 that names 00881350;
  - a host wiring of the patch index list (the landscape render host does not consume it yet).
- The `stl_probable` bookmarks on 007240E0, 00724CE0, 00725380 and 00724A20 are wrong: these are
  not STL bodies. They stay in the tag shard, and the ledger names now take precedence.
