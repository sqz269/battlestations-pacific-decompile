# Ocean surface rendering (packet cc13_water1)

Partition segment 92 (00B999F0-00BB3CE0) and the water part of segment 93 (00BB3DB0-00BBCCAF).
Weather1 already named 00BBCF60-00BBDEC0. Worker cc13-water1, branch `agent/cc13-water1`.

Status words follow AGENTS.md:
- **Exported** means raw pseudocode exists.
- **Reconstructed** means C++ exists in `src/water_render.cpp`.
- **Build-tested** means it compiles in the strict Win32 build.

Nothing here is **game-validated**. None of the code is wired into the host. Names are hypotheses.

## 1. Gameplay boundary finding: the water-height amplitude is not zero

`docs/OCEAN_WAVE_FIELD.md` says field+24h is "stored once, as 0.0" and that 0078C890 therefore
returns `h * 0.0`. That is wrong. The ocean per-frame update writes field+24h every render frame.

- **Caller.** 004B4820 BSP_RenderCameraUpdateJob_Run → 0068A0D0 → 0078CFF0. At the tail of 0078CFF0,
  0078D18B does `MOV ECX,[EDI+0A8h]` (EDI = the world), and 0078D19D does an unconditional
  `CALL 00BA0090`.
- **The store.** 00BA0090 sets ESI = ECX (= the ocean object) at 00BA00B7. Then:
  - 00BA0B3E..00BA0B55: `[ESI+2Ch] = [ESI+0B8h]`. This is `Waves.HeightScale`, which 0078C9B0
    stores at +B8h.
  - 00BA0B5F: `[ESI+28h] = HS * [spectrum+88h]`. This is cWaveScale.x.
  - 00BA0B6D: `[ESI+30h]` gets the same product.
  - **00BA0B90 `FLD [ESI+2Ch]`, then 00BA0B93 `FSTP float ptr [ESI+24h]`** (bytes `D9 5E 24`).
- **The grid is live.** 00BA00C0..00BA00DD calls `spectrum->vtable[+8]` with
  `(arg * 0.1 * 10.0) * [field+20h]` (TimeScale), and then calls vtable[+18h]. So the 64×64 grid
  that 0078C890 samples advances in time.

After the first in-game render frame, 0078C890 returns `grid(u,v) * HeightScale`. This
installation authors HeightScale between 0.5 and 3.0 (2.0 for usn_04 and usn_01). The census in
OCEAN_WAVE_FIELD.md missed the store even though 00BA0090 lies inside its scanned band.

This was reported to the lead (for cc9) as soon as it was found. It is reconstructed as
`ocean_wave_scale_00ba0b32` but **not wired**. `ocean_wave_field.*` and `world_ocean.*` are
untouched.

**Same object: the writer and the gameplay reader load the same pointer.** The chain is
game = `[00E188A8]`, world = `[game+19F0h]`, ocean = `[world+0A8h]`. 0078DB03 in the world
constructor is the only store to world+A8h.
- **Writer.** 0068A0D0 loads EAX = `[00E188A8]` and then ECX = `[EAX+19F0h]` (0068A0E6). It calls
  0078CFF0 (0068A0EC), which sets EDI = ECX (0078CFF8). Then 0078D18B loads ECX = `[EDI+0A8h]` and
  00BA0090 sets ESI = ECX (00BA00B7), stores `[ESI+24h]` at 00BA0B93. The time argument is
  `[00F876A4]` (0068A0D5).
- **Gameplay reader.** 0078CF20 is called with ECX = `[[00E188A8]+19F0h]`. One example is 00487C24
  then 00487C2D; the rel32 census finds the same two-load pattern at every caller whose load is
  visible, more than 40 sites. 0078CF20 sets ESI = ECX (0078CF2F) and loads ECX = `[ESI+0A8h]`
  (0078CF35). It calls 0078C890 (0078CF3E), which sets ESI = ECX (0078C894) and at 0078C92D does
  `FMUL [ESI+24h]` on the 00B960C0 grid sample. 0078CF43 reloads `[ESI+0A8h]` for the coverage mask
  00B9CF50.
- **Second reader.** 0078D1B0 (BisectWaterCrossing) does the same: 0078D294
  `MOV ECX,[ESI+0A8h]`, then 0078D2A8 `CALL 0078C890`.

So 0078C890 multiplies by the same field+24h that 00BA0B93 writes.

Two related render-side readers exist. Neither is a gameplay reader:
- **00BA2B50 BSP_OceanWaveField_SampleHorizontalDisplacement** returns the choppy (dx, dz) pair from
  00B962E0/00B96450 at `frac(k·x), frac(k·z)`. It applies no amplitude. Its only caller is the beam
  system 00BA2FF0.
- **0078C890 is also read** by 00B9F0A0 (the camera-underwater test, field+CCh), 00BA2FF0 (beam
  placement) and the tracer helpers 00BAB3F0/00BAB930.

## 2. The ocean object (world+A8h, 6BCh bytes)

**Construction and vtables.**
- 00BA6FB0 BSP_OceanWaveField_Create allocates the object. 00BA6C40 constructs it, after the base
  constructor 00BA1400.
- The base vtable is 00D63B5C. The derived vtable is 00D63E10.
- Slot 0 is shared by both: 00B9D7B0 RefreshGridTiles.
- Derived overrides:
  - slot 1: 00BA51C0 CaptureSceneColor
  - slot 3: 00BA5030 UploadHeightTexture (float16 through D3DXFloat32To16Array)
  - slot 4: 00BA6BD0 SetQuality (the base slot is the RET 4 at 00BA2460)
- Slot 5 is 00B9A3B0. It returns the 200h×200h reflection target size.

**Rendering technique: a projected grid.**
1. 00B9AE00 ComputeProjectorMatrix intersects the 12 edges of the frustum cube with the planes
   y = ±[field+B8h] (00B9A8A0 → 00B999F0). It then builds a look-at projector and writes the range
   matrix to +174h. It is not reconstructed: it is large x87 code.
2. 00B9CA00 maps uv (0,0), (1,0), (0,1), (1,1) through 00B99C80 into cCorner0..3 at +134h..+170h.
3. The vertex shader interpolates the corners across the 00BA5240 `projocean.mvfm` grid. The grid
   is 20h×40h at start (00BA5610). 00BA6BD0 changes it to 80h×100h, or to 20h×20h when the simple
   flag +F9h is set.

**Heights on the GPU** come from the same CPU spectrum:
- 00BA5030 uploads the grid as float16 into +274h/+278h. 00BA0090 swaps the two each frame.
- The height generator +2BCh is created by 00BB3C60 → 00BB3AB0 → base 00BA94F0. 00BA7530
  initialises its passes (oceanheightmapgen, oceannormalgen, oceanheightscale) and 00BA8EE0 renders
  them per frame.
- 00BB3DB0 creates its double-buffered vertex streams. 00BA4EB0 binds the current stream as
  stream 1 and the texture as slot 8.

**Other passes.**
- 00B9F0A0 renders reflection and refraction: the REFL and REFR render-queue passes, the camera at
  +C8h with a clip plane, and the fog copy.
- 00B9AB60 builds the projective texture matrices.
- 00B9DC20 recreates the reflection targets: +E4h, +E8h and the depth surface +288h. The format is
  15h, or 71h when the byte 00F8D398 is set.
- 00BA51C0 copies the back buffer into +608h. It wraps the copy in EndScene and BeginScene.

**Floating objects and coverage regions.**
- BSP_Landscape_LoadTerrain calls 00BA1370. That function adds coverage regions through 00BA0C00
  and appends the terrain model to +5F4h.
- 00B9E170 reads a texture into a byte mask.
- 00B9FE00 removes an object's nodes from +614h and its regions from +620h.
- The region containers are 00B9D1E0..00B9EFA0 and 00BA13C0/00BA13E0. All are
  `{data, count, capacity}` vectors.

**Water rings.**
- 00B9E3F0 appends a 24h-byte record to +4F8h. Its caller is the effect-component constructor
  008739A0.
- 00B9E4A0 removes a record (callers 00872D60 and 00872DF0).
- 00B9CB80 rebuilds the ring triangle stream each frame.

## 3. The authored g_Ocean block

004EB9B0 reads the scene file's `g_Ocean` group into record+990h. Offsets below are relative to
that block. The full table is the `block::` namespace in `include/bsp/water_render.hpp`, taken from
the key strings and the stores in 004EB9B0. ReflCubemap comes from 004EE450, which builds
`<ReflCubemap minus .dds>_ocean.dds` at block+210h.

0078C9B0 applies the block. ECX of each setter is `[world+A8h]`.

| Setter | Native | Block source | Object field |
|---|---|---|---|
| SetSkyReflectionData | 00B99E90 | +144h (7SkyRefl×4) | +350h |
| SetLocalReflectionTint | 00B99EC0 | +154h | +36Ch |
| SetSprayTint | 00B99F00 | +1A0h | +3DCh |
| SetReflectionTint | 00B99E60 | +164h | +37Ch |
| SetRefractionTint | 00B99F60 | +174h | +38Ch |
| SetUnderwaterRefractionTint | 00B99F90 | +184h | +39Ch |
| SetUnderwaterGlow | 00B99FC0 | +194h (3 floats) | +360h |
| (existing) BSP_SystemParameters_SetSlot3AndReciprocal | 00B9A0D0 | +38h..40h WaveDampening | slot 3 |
| (existing) SetSlot0 | 00B9A030 | +1B4h, +1B8h, +1C4h, +1C8h | +3BCh.., slot 0 |
| (existing) SetSlot1 | 00B9A130 | +1BCh, +1C0h DeepWater | slot 1 |
| (existing) SetSlot2 | 00B9A080 | +1D8h..1E0h ShoreFade | slot 2 |
| SetUnderwaterFog | 00B9DBB0 | world+84h..90h (Fog.UWFogColor, MaxDarken) | +2ACh, +3ACh |
| SetRefractionSpecularColor | 00B9A180 | +1E4h | +3FCh |
| SetUnderwaterRefractionIndex | 00B9A2C0 | +1F4h | +414h |
| SetReflectionCubemap | 00B9DB50 | +210h | slot 2 of materials +12Ch/+130h |
| SetFoam | 00B9E8B0 | Foam +0h..+24h | +84h..+ACh, foam model +120h slots 0/1 |
| SetLayerParams | 00B9A4C0 | +2Ch, +30h, +34h | +18h, +1Ch, +20h, +B0h, +B4h |
| SetNormalMapLayers | 00B9A3F0 | +260h, +270h | +44h (1/scale), +54h |
| SetNormalMapTexture | 00B9DE30 | +280h | slot 5 |
| SetWind | 00B9A450 | +4Ch × π (00CE3D28), +50h | +59Ch, cos +64h, sin +68h |

The four existing `BSP_SystemParameters_*` names were kept. Their evidence was appended with these
block sources.

## 4. Other water objects

**Beams (world+ACh).** Underwater light shafts from `g_Ocean.Beam`.
- 0078DCF0 constructs them through 00BA3A60 (vtable 00D63B88) and initialises them with 00BA3BD0
  (`beam.mshd`). The destructor is 00BA2E60.
- 00BA2FF0 updates them every frame. It respawns beams with `BSP_RandomState_NextU32` on the private
  state 00E130C0. That state is referenced only by 00BA2FF0 and the static initialiser 00CD87B0, so
  the beams never draw from the 00BD2F10 gameplay streams.

**Water particles (world+B0h).**
- 0078DBA0 constructs them through 00BA43B0 (vtable 00D63BB4) and initialises them with 00BA44F0
  (`WaterParticle.mshd`, cVolumeSize). The destructor is 00BA42A0.
- 00BA40F0 updates them every frame.

**Water tracers (wakes).** These complete the existing NATIVE_TRACER_* docs. 0086D180 creates the
shared resources:
- 00BADC70 builds the `watertracerskined.mshd` and `watertracerstatic.mshd` sections.
- 00BAD150 builds the skinned mesh, with the width profile from 00BAA0F0. 00BACBD0 builds the static
  mesh.
- 00BAE2E0 builds `waterspray.mshd` with `p_bubbles_0002.dds`.

The tracer vtable slot at 00D63FF8 is 00BABA50, defined by this packet.

**Caustics and shore waves.**
- These are procedural texture sources, registered by 00BBCB40.
- The caustics vtable is 00D64478:
  - slot 8: 00BBCBC0 SetScale (+30h, CausticsScale)
  - slot 9: 00BBCBA0 SetColor (+20h..28h, CausticsColor)
- The shore-wave vtable 00D644B4 makes both of those slots no-ops (00BBCBF0 and 00BBCBE0).
- Both share 00BBC690, which returns the descriptor at 0109E900h.

## 5. Shipped but unreachable: the coast mesh and the coastline extractor

The image contains a complete shoreline feature.

**00BB0E30 BSP_CoastMesh_Construct** (vtable 00D640E4) refers to `foamwave.dds`, `foamcoast.dds` and
`coast1.dat`. Its pieces:
- 00BB3370 initialises it (`Coast.mshd`, `CoastFoam`).
- 00BB2DF0 loads the coast distance map.
- 00BB11A0 generates the map from landscape heights when the load fails.

**00BB02C0 BSP_Coastline_Construct** (vtable 00D640DC) extracts the coastline. 00BB09F0 drives it:
- 00BAF9A0 collects the `bterrain.mshd` sections.
- 00BAF8A0 and 00BAF5C0 clip triangles against the water level.
- 00BAFAA0 chains the segments and 00BAFCE0 resamples them.

The wave-damp object 00B9EA50 (`WaveDampObject`, `oceanwavedamp.mshd`) and the spray object 00BAE900
are in the same state.

**No reference reaches them.** I checked:
- A rel32 CALL/JMP census of .text.
- An absolute-pointer census of .rdata and .data, both aligned and unaligned.
- A census of any 32-bit immediate in .text.

None of these finds a reference to 00BB0E30, 00BB3370, 00BB02C0, 00BB09F0, 00BB0070, 00B9EA50 or
00BAE900. The landscape tick does call 00BB0A90 on landscape+3D4h and 00BAEC50 on landscape+3D8h,
but behind null checks (00880013, 00880038). The only writers of those fields store zero: the
landscape constructor at 00883BD9/00883BDF and the release path at 008828B0/008828D2, inside 008826F0, where EBP is zeroed at 008826F6.

So in this image the visible shore effect is the shore-wave texture source, not this mesh. This is
a static negative, bounded by the three censuses above.

## 6. Reconstructed C++ (`src/water_render.cpp`, `include/bsp/water_render.hpp`)

All entries are reconstructed and build-tested. None is fixture-tested. The ledger carries the ABI
of each.

**Whole functions:**
- 00B999F0 `plane_intersect_segment`
- 00B9A8A0 `plane_crossing`
- 00B99BF0 `transform_homogeneous_divide` (stdcall, in place, RET 8)
- 00B99C80 `clip_ray_water_plane` (stdcall, RET 10h)
- 00B9CA00 `compute_grid_corners`
- 00B9A3F0 `set_normal_map_layers`
- 00B9A4C0 `set_layer_params`
- 00B9E3F0 `make_water_ring`. This covers the record fields only. The allocation and the append are
  not reconstructed.

**Fragments:**
- 0078C9B0's optics setter sequence (`apply_authored_optics`)
- 00BA0090 normal-map scroll (00BA0792), ocean clock (00BA00AD) and wave scale and amplitude
  (00BA0B32)
- the 00B9A450 trig stores after the spectrum call
- the 00BA5610 and 00BA6BD0 grid sizes

The x87 bodies keep float operands. The extended products are modelled in double and rounded at the
native stores, so they are not bit-exact.

## 7. Ghidra

- Defined 27 functions with `tools/ghidra_define_function.py` (`reports/cc13_water1_defined.json`).
  Each start had INT3 or RET padding before it and a vtable or call reference.
- Named 195 functions (181, then 14 cloud accessors) through the ledger and `ghidra_annotate.py`. The previous values are in
  `local/ghidra-annotations-20261011T211248Z.json`, which is ignored by git.
- Call-site rows are in `reports/cc13_water1_calls.json`. `verify_report_calls.py` checked 17 rows
  with 0 failures.
- **Cloud-manager accessors (closed at the lead's request).** The layout is from cc13-weather1.
  - 00BBCCB0 GetVector50 and 00BBCCD0 GetVector44.
  - 00BBCD30/00BBCD70/00BBCDB0 GetColour60/70/80. BSP_CloudInstance_Update reads them at
    00BBEF21, 00BBF08B and 00BBEEF3.
  - 00BBCD50/00BBCD90/00BBCDE0 SetColour60/70/80 and 00BBCE30 Set94. Their callers are in
    BSP_Game_ConstructWorld.
  - 00BBCE60 SetWindEnabled (byte +99h).
- **Uncovered code at 00BBE308..00BBE37F.** This starts with INT3 padding after
  BSP_CloudInstance_AcceptsRuntimeTypeToken (00BBE2E0..00BBE307). It holds four functions, now
  defined:
  - 00BBE310 ShowReturnFalse: SetVisibilityFactor(1.0, 0), AL = 0, RET 4.
  - 00BBE330 GetOwner184.
  - 00BBE340 GetManagerVector50: ECX = `[[this+184h]+14h]` through 00BC3030, then 00BBCCB0.
  - 00BBE360 GetManagerVector44: the same through 00BBCCD0.

  The two call sites 00BBE353 and 00BBE373 are inside 00BBE340 and 00BBE360. My first scan had
  attributed them to the type-token function. The rel32, pointer and immediate censuses find no
  reference to any of the four, so they are not reachable in this image.

## 8. Open items

- **Amplitude (cc9).** Model the amplitude store in the host's 0078C890 path: amplitude = HeightScale
  after the first render frame. This also needs the spectrum's per-frame step (vtable+8/+18h,
  segment 91) and the authored Waves keys.
- **00BA0090 time argument.** It is `0.1 * [00F876A4]`, which 0068A0D0 loads at 0068A0D5. 00B9E3F0
  stores it as a start time, so it looks like an absolute clock. The writer of 00F876A4 has not been
  traced.
- **Not reconstructed.** 00B9AE00 (the projector matrix), 00B9D7B0 (the grid tiles), 00BA2FF0 (the
  beams) and the large resource builders are large x87 or D3D bodies.
- **Seg 93.** The mpak/mpkg/msar STL instantiations 00BB3DB0..00BBBED0 that are still FUN_ were
  not named, apart from the water ones. They are container plumbing for the existing named
  providers.

## 9. Handoff

(Superseded for 00B9AE00, 00B9D7B0 and 00BA2FF0 by section 10 and its handoff 10.4; the cc13-water1 notes below are kept for history.)

- Branch `agent/cc13-water1`. The lease was released at the end of the packet.
- Next worker: the not-reconstructed list in section 8, starting with 00B9AE00 if exact grid parity
  is wanted, or the seg 93 STL naming. AGENTS.md routes x87-heavy packets to a stronger model.

**00B9AE00 BSP_OceanWaveField_ComputeProjectorMatrix**
- `__thiscall(field, camera)`. The second argument is a camera node: it is tested at +5Ch bit 2 and
  read at +1C8h/+1D8h (+72h/+76h dwords). The live decompile is about 500 lines.
- Structure, from my skim. These are the steps of Johanson's projected-grid "getMinMax"; this has
  not been verified against the listing.
  1. Widen the FOV by [00CEC160] and build a perspective matrix with BSP_Matrix_BuildPerspective.
  2. Invert the view-projection.
  3. Transform the eight clip-cube corners at ±1.
  4. Run 12 edges through 00B9A8A0, against planes at y = +[field+B8h] and -[field+B8h]. The
     negated value is formed as `-0.0 - x` from 00D7A208.
  5. Project the hits onto y = 0.
  6. Clamp the projector's elevation using cos([00CEC400]), [00D7A358], [00CED5D8] and [00D7A308].
  7. Build a look-at projector with up = (0,1,0) through BSP_Matrix_BuildLookAt.
  8. Take the x/z min/max of the transformed points into the range matrix at field+174h.
- 00B9CA00 consumes +174h. That function is already reconstructed.
- Reconstruction risk: the many FSTP spills. Use `memory/sweep-huge-functions-with-eh-state-esp-anchors`
  only if it will not decompile. It does decompile live.

**00B9D7B0 BSP_OceanWaveField_RefreshGridTiles**
- Vtable slot 0. It walks the tile grid at `[field+2B0h]` (w at +4h, h at +0h) against the 64×64
  spectrum cells: index `(x & 3Fh) + (y & 3Fh)*40h` into `[field+BCh]`.
- It compares against the thresholds +104h..+110h and calls 00B9A990 per quad.
- It ends with vtable+0Ch, which is 00BA5030, the float16 upload.
- Check x87 against the listing: the decompile shows `fVar15 = -0.0` resets.

**00BA2FF0 BSP_OceanBeams_Update**
- ECX = world+ACh. Its arguments are the camera and dt × [beam+30h] (Speed).
- Three draws from `BSP_RandomState_NextU32` with ECX = 00E130C0, at 00BA3270, 00BA32BF and
  00BA346E. Each converts the draw to [0,1) through `+[00CE3978]` and `*[00D63B80]`.
- Respawn uses position = Tex parameters +50h/+54h. Lifetime uses +CCh, +D0h, +D4h and +D8h.
- It samples 0078C890 at two points and the displacement 00BA2B50.
- **RNG rule.** If the host ever wires this, keep the private state. Never route it to the 00BD2F10
  streams.

**Seg 93 STL helpers.** Before naming them, check `lease list` and the CC12 archive/VFS docs
(NATIVE_VFS_*, MPKG, mpak packets). These helpers sit under cc12-reconstructed providers, so they
are probably cc12 territory.
- Helper scripts are in this tree's `local/wa1_*`. They are read-only PE censuses: `wa1_callers.py`,
  `wa1_ptrs.py`, `wa1_immref.py`, `wa1_dispstore.py`, `wa1_bounds.py`, `wa1_dis.py` and `wa1_vt.py`.

## 10. Ocean x87 bodies (packet cc13_water2)

Method: every block was traced against the listing with `local/wa2_sym.py` (cc13-water2 tree), a
symbolic path executor that decodes x87 register arithmetic from the opcode bytes. Ghidra prints
`DC C9` as `FMUL ST1` (meaning ST1 *= ST0), so its one-operand x87 mnemonics are ambiguous and were
not trusted. Constants were read from the PE on disk. Status for all three: reconstructed and
build-tested, not fixture-tested, not game-validated, not wired. The reconstructions round at every
native float store and use double between stores, so they are close to native but not bit-exact.

### 10.1 00B9AE00 BSP_OceanWaveField_ComputeProjectorMatrices (complete)

`bool __thiscall(field, camera)`, RET 4. AL = 1 on success. `compute_projector_matrices_00b9ae00`.
water1's eight-step outline was right in shape but wrong in several details:
- Only the four near clip corners (z = 0) go through the inverse projection (B9B157, count 4). The far
  corners are the near ones times far/near (B9B219). The ratio is spilled to a float.
- The near plane is clamped: `near = 1.5 > near ? 1.5f : near` (double [00CE3D78], float [00CE380C]).
- The fov is widened by the double [00CEC160] = (double)1.2f. The camera is moved to (0, y, 0)
  through vtable+30. Both are restored on both exits (B9C9A5, B9C9D3).
- Corners count as inside the slab when `(h + y) / (y - h) < 0`. `FDIVRP` is `DE F1`, so the
  quotient is ST0/ST1. y = (x*0 + y) + z*0 is kept in extended precision.
- Projector eye: lim = h + 7.0 ([00CED5D8]). If y < lim, the eye moves up by `lim - y`, or by
  `lim - 2y` when y < 0 (2.0 is [00D7A308]). This is a lift, not a clamp of a cosine.
- Forward vector: when |f.y| <= float(cos(1.55334306f)) ([00CEC400], 89 degrees), f.y gets -0.01 or
  +0.01 (double [00D7A358] = (double)0.01f) and f is renormalised through 00BF7030 (0 for zero length).
- Aim point: B = f, reflected as `f - (0,2,0)*d` when camera and forward lie on the same side of
  y = 0. hit = 00B999F0((0,1,0,0), pos, pos + B + 0). The aim is `hit*|f.y| + flatten(pos + 10*f)*(1 - |f.y|)`,
  10.0 = [00CE3DC0]. Then `aim - eye + eye` is stored as computed.
- view = 00B63F10(eye, aim, (0,1,0)). The points are flattened to y = 0, then each goes through view
  and then through the second perspective Pp with a 1/w divide. Each step is stored back.
- Outputs: +174h = range * inverse(view * Pp); +1B4h = inverse(+174h); +1F4h = (view * Pp) *
  00B63B30(range); +418h = view; +458h = Pp. Here range = [dx 0 0 0; 0 dy 0 0; 0 0 1 0; min_x min_y 0 1].
- No point (B9C117): restore the camera and return 0 without writing the field.
- The point array (frame -120h) has no bound check. For a convex frustum it holds at most 20 points.
  The port caps it at 32.

### 10.2 00B9D7B0 BSP_OceanWaveField_RefreshGridTiles (complete), 00B9A990 BSP_OceanGrid_AverageQuad (complete)

[field+2B0h] is not a "tile grid" but a pyramid of 0Ch-byte levels {width, height, cells}. Its count
is at [field+2B4h], and each cell is 18h bytes (x, height, z, normal[3]).
- Level 0, cell (x, y), with i = (x & 3Fh) + (y & 3Fh)*40h: x = choppy[i].x and z = choppy[i].z from
  spectrum+30h, and height = -0.0f - spectrum+20h[i].word0. Both std::vector accesses are range
  checked through 00BF6713 (_invalid_parameter_noinfo).
- Running maxima, never reset in this function: +104h = HS*|h| (NaN takes the new value), then
  +108h |x|, +10Ch |h| and +110h |z| (NaN keeps the old value).
- **The "-0.0 resets"** in the decompile are only the reload of XMM3 = [00D7A208] after the CRT call.
  They are not field resets.
- **Native defect:** level-0 normals (+0Ch..+14h) are copied from [ESP+4Ch..54h] (frame -0Ch..-4h).
  Nothing in the function writes those slots before the first loop, so they hold stack garbage. The
  port takes them as `stack_garbage`.
- Level i+1 cell (x, y) = 00B9A990 of the level-i cells (2x, 2y), (2x+1, 2y), (2x, 2y+1) and
  (2x+1, 2y+1). The scale comes from the 5-entry stack array {0, 0.2, 0.5, 0.9, 1.0}
  ([00CE54A0], [00CE3800], [00CE3860]). A count of 0 would underflow (SUB ESI,1).
- 00B9A990: centre = ((p0+p1)+p2)+p3 times double 0.25 [00D7A348]. Normal = ((p1.n+p0.n)+p2.n)+p3.n,
  lerped (005803E0) towards (0, 4.0 [00CE3D34], 0) by the scale, then normalised (00419440).
- The function ends by tail-jumping to vtable+0Ch (00BA5030), the float16 upload.
### 10.3 00BA2FF0 BSP_OceanBeams_Update (complete)

`void __thiscall(beams = world+ACh, camera, float time)`, RET 8. `update_ocean_beams_00ba2ff0`. Every
outside touch point goes through `OceanBeamHost`.
- Off when +2Ch == 0. When camera.y >= 0 it sets visibility 0 ([beams+14h], 00B6DA70) and returns
  before the clock update.
- The clock is +F0h = float(+30h * time) and the previous clock is +ECh (taken from the new clock when
  negative). delta = clock - previous.
- **Surface origin:** L = 00419510(first light's +1E0h vector) and n = -L. Then s = sqrt(1 - n.y^2),
  h = n.xz * 00419260(n.xz) * s, and ly = -sqrt(1 - (1 - n.y^2)). origin = camera - (camera.y/ly) *
  (h.x, ly, h.z), so origin.y = 0. Beams are placed around it and every quad faces it. They do not
  face the camera.
- **RNG.** Three draws come from `00BA2C20` with ECX = 00E130C0, the private Mersenne state. The two
  at BA3270/BA32BF are in the respawn path and the one at BA346E decides deaths. The caller also inlines
  the state guard (state+9C5h set and +9C4h clear: write to address 0). The port keeps the stream
  behind `next_private_random_00ba2c20`. It must never be wired to 00BD2F10.
- Respawn happens when birth < 0, or when death >= 0 and clock > death. It sets
  radius = +50h + +54h*u1, angle = float(u2 * (double)6.2831855f) and position = origin + radius*(cos,
  0, sin), then birth = clock and death = -1.
- Death scheduling applies when death < 0. The squared xz distance to the origin selects a band
  (+60h/+64h/+68h) and its rate p (+6Ch..+78h). survival = exp(log(1-p) * delta): 00BFA910 is _CIlog
  (x87 path `FLDLN2; FYL2X`) and 00BFEEEC is _CIexp (name string "exp" at 00E1670A). If u3 > survival,
  death = clock + +D0h.
- NaN (cc13-big5ad, 2026-10-17): the birth and death tests are COMISS / JC (BA3216, BA3226) and the
  fade-out test is COMISS / JC skipping (BA37AA), so an unordered birth or death counts as below zero:
  a NaN birth respawns, a NaN death schedules, and a NaN death skips the fade-out. The C++ tests are
  written as `!(x >= 0)` / `x >= 0` to take those arms; the clock, band and fade comparisons were
  already NaN-faithful.
- Quads: the perpendicular is normalize(origin - beam) rotated, or (1, 0) at distance <= (double)0.05f.
  The top corners sit at +-(+58h) on the water height 0078C890. **That is the gameplay water height,
  including the +24h amplitude** (section 1). It is read only.
- axis = normalize(normalize(dx, 64, dz) * +5Ch + (h.x, ly, h.z)), with (dx, dz) from 00BA2B50 and
  4096 = [00D63B90]. The bottom corners are top + axis * +4Ch.
- Vertices are 1Ch bytes (xyz, u, v, alpha, ARGB). u = 0 for vertices 1 and 3 (the + side). v = 1 for
  the bottom pair. Fade-in is age * +D4h while age < +CCh. Fade-out multiplies by (death - clock) * +D8h
  while that is < +D0h. Colour = 004FB850(+3Ch) for each vertex. The counts are written first:
  [beams+1Ch]+10h = 4n and +18h = 2n.
### 10.4 Handoff (cc13-water2)

- Branch `agent/cc13-water2`. Leases `cc13_water2_ocean_x87` (00B9AE00-00BA35FF) and `cc13_water2_quad`
  (00B9A990-00B9AB39) are released.
- Done: 00B9AE00, 00B9D7B0, 00B9A990 and 00BA2FF0. All four were reconstructed in full and
  build-tested. They are not fixture-tested and not wired.
- Next options: give 00BA5030 (the float16 upload that 00B9D7B0 tail-calls) and 00BA51C0/00B9A2F0
  (vtable 00D63E10 slots 1 and 2) the same treatment, or write a fixture that drives
  `compute_projector_matrices_00b9ae00` against a captured camera.
- Tooling: `local/wa2_sym.py` in the cc13-water2 tree is a one-path symbolic executor. Use `--take`
  for branches, `--esp`/`--fpu`/`--init` to start mid-function, and `--out A=arg:len` for callee
  outputs. It decodes `D8/DC/DE` register forms from the bytes. Pitfall: capstone names FCOMIP
  `fcompi`, and the tool now pops for it.