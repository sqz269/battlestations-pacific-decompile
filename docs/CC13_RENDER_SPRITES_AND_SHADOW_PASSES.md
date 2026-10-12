# Segments 84, 85 and 90: renderer debug draws, texture slots, layout pool, resource-tree STL (cc13-render5)

Worker cc13-render5 (Claude lineage cc13), resuming cc13-render4's packet after a power outage
(cc13-render4 left no commits, notes or leases). Names are hypotheses, not recovered symbols.
"Unreferenced" means the disk image has no rel32 CALL/JMP to the address and no absolute dword
equal to it in any section (scan: `local/r5_rel32.py` in the worker tree, rel32 over `.text`
plus aligned and unaligned dwords over every section).

Status vocabulary: *named* = ledger + Ghidra name and comment; *reconstructed* = C++ in
`src/render_sprites_cc13.cpp` / `include/bsp/render_sprites_cc13.hpp`, Win32 **build-tested
only** (`scripts/build.ps1`, both existing CTests pass). Nothing here is fixture-tested,
ABI-compatible or game-validated.

## Triage

| area | state before this packet | this packet |
|---|---|---|
| seg 84 sprite batcher / 2D sprite camera (`00B2B580`, strings `2DSprites`, `2dSpriteCamera`, `pf43cc.mvfm`, `debugshader.mshd`) | reconstructed by cc12 (`src/native_renderer_debug_records40.cpp`) | not repeated |
| seg 84 debug line / sphere consumers `00B2B270`, `00B28D00`, `00B2BB90`, record appends `00B29270`, `00B29330` | reconstructed by cc12 | not repeated |
| seg 84 renderer vtable `00D5F0A8` debug slots +34h, +8Ch, +B4h, +BCh, +C0h, +CCh, +D0h, +D8h, +DCh, +68h | unnamed | named; seven reconstructed |
| seg 84 cube/volume/2D texture and surface vtable slots (`00D61870`, `00D618B0`, `00D61948`, `00D619A0`) | partly named | 21 texture/surface names added (including the cube-texture create slot) |
| seg 85 `shadow_passtrough.shfx` loader `00B45EE0`, material pass code | named/reconstructed by cc12 | only the generator-mode loop `00B44C00`, the shared vertex stream constructor `00B4A9B0` and the 44h layout slab pool are new |
| seg 90 "node" | resource node factory / instance graph named by others; the `std::map`/`std::vector` instantiations behind `BSP_ResourceInstance_*` unnamed | 38 STL bodies named (13 of them unreferenced), not reconstructed (library templates) |
| template/COMDAT residue in all three segments | unnamed | 112 unreferenced bodies named with the `_Unreferenced` suffix, not reconstructed |

Totals: **194 ledger names** (83 referenced, 111 unreferenced), **11 reconstruction records**
(10 complete bodies, 1 partial), **1 function defined** in Ghidra (`00B488D0`, record
`reports/cc13_render5_defined_functions.json`; `00B27440` and `00B3CD30` had already been
defined by cc13-define2 when the snapshot caught up). "Non-Returning Functions - Discovered" was
confirmed disabled (bridge `list_analyzers`) before the define.

Leases: ranges `00B25DC0-00B402B0`, `00B402E0-00B4B0A0`, `00B86E40-00B8A200` and the three output
files, packet `cc13-render5`. No other lease overlapped.

## Renderer debug draws (vtable `00D5F0A8`)

The renderer vtable starts at `00D5F0A8` (cc12 docs use the same base); the device is
`renderer+1A10h`. Slot callers are virtual and were not enumerated, except +B4h (below).
All four draws share one inline state tail (`restore_debug_draw_states`): FILLMODE 2, ZENABLE 1,
ALPHABLEND 0, LIGHTING 0, CULLMODE 1, ALPHABLEND 0, ALPHATEST 0, FOG 0, SPECULAR 0, ALPHATEST 0,
ALPHABLEND 0, FILLMODE 2, stage 0 colour/alpha = SELECTARG1(TFACTOR), stage 1 off, TEXTUREFACTOR
FFFFFFFFh. FILLMODE is written as 2 (D3DFILL_WIREFRAME) in every copy of the tail; whether that is
the intended "default" is open. Every draw is skipped (state calls still made) when DWORD
`renderer+1D90h` or byte `renderer+1D8Ah` is non-zero.

| address | slot | name | ABI | evidence |
|---|---|---|---|---|
| `00B26A10` | +D0h | `BSP_D3D9Renderer_DrawDebugFrustum` | ECX renderer, stack (world matrix*, camera), RET 8 | world read at `00B26AB5 MOV EDX,[ESP+10h]` (Ghidra shows one argument); `device+150h` DrawIndexedPrimitiveUP(LINELIST, 0, 8, 12, `00E13038`, INDEX16, `0108D4D0`, 0Ch) |
| `00B26C60` | +CCh | `BSP_D3D9Renderer_DrawDebugPlaneNormals` | ECX renderer, stack (planes[6][4], camera), RET 8 | `RET 0x8` at `00B2742F`; vertices at frame+74h (`00B26CEF FSTP [ESP+74h]`..), draw at `00B27325 LEA EDX,[ESP+78h]` after one push |
| `00B27440` | +D8h | `BSP_D3D9Renderer_DrawDebugTextureQuad320` | ECX renderer, stack texture, RET 4 | `OutputDebugStringA` through `[00CE2274]` (IAT name read at `00E0587C`), strings `00D5E618` "Texture db {" and `00CE4CD4` "}" |
| `00B27780` | +DCh | `BSP_D3D9Renderer_DrawDebugTextureQuad512` | ECX renderer, stack (texture, unused), RET 8 | only `[ESP+68h]` is read; `RET 0x8` at `00B27A39` |

Vertex data:

- **Frustum box.** `00CD7A80` (a static initializer) writes `0108D4D0`: xmm0 = 0.995 (`00E13068`),
  xmm1 = -0.0 (`00D7A208`) - xmm0. Vertex i has x = bit0 ? +0.995 : -0.995, y = bit1 ? .. , and
  z = 0.995 for i >= 4; the z of vertices 0..3 is never stored and stays zero-filled. That is the
  D3D clip volume (z in [0,1]) shrunk by 0.5%, so with the caller's world = inverse view-projection
  of another camera the draw outlines that camera's frustum. Indices `00E13038`: near quad, far
  quad, four connecting edges. Colour: TEXTUREFACTOR FFF00FFFh, ZENABLE and ZWRITE on, FVF XYZ.
- **Plane normals.** For each plane {nx, ny, nz, d}: a line from d*n to (d + 400.0)*n. x87:
  `00B26D07 FLD double [00CE3D90]` (400.0), `FADD`, `FSTP float [ESP+10h]`, reload, three FMULs and
  FSTP float; the products of two floats are exact in 53 bits, so float arithmetic reproduces them
  (`tools/x87_sym.py 00b26c60` agrees). Colour 00FF0000h; identity world. The shaders are cleared
  inline (renderer+1770h/+176Ch, device+170h/+1ACh, counters +1BC0h/+1BBCh) instead of through
  `00B21D10`/`00B21C20`.
- **Texture quads.** FVF XYZRHW|TEX1 (104h), stride 18h, TRIANGLESTRIP x2: (0,0) (s,0) (0,s) (s,s)
  with z = rhw = 0.5 and uv the corner, s = 320.0 (`00D5E614`) or 512.0 (`00CE3C88`). Stage 0 =
  SELECTARG1(TEXTURE), clamp U/V, TEXCOORDINDEX 0, COLORWRITEENABLE 0Fh, solid fill. Both unbind
  stage 0 before and after the state tail.

## Debug record appends

| address | slot | name | ABI | layout |
|---|---|---|---|---|
| `00B290D0` | +B4h | `BSP_D3D9Renderer_AppendDebugLine` | ECX renderer, stack (vec3* a, vec3* b, color_a, color_b), RET 10h | two 10h records {x, y, z, colour} into renderer+1CF4h {data, count +1CF8h, capacity +1CFCh}; growth max(cap*2, 1) via `BSP_RendererRecords16_Reserve`; under the record guard (`00B25BE0`) |
| `00B29230` | +BCh | `BSP_D3D9Renderer_ClearDebugLineRecords` | ECX renderer, RET | reserve(0) when capacity < 0, count to 0 |
| `00B28AC0` | +C0h | `BSP_D3D9Renderer_AppendDebugLine2D` | ECX renderer, stack (vec2* a, vec2* b, color_a, color_b), RET 10h | two 14h records {x, y, 0.5, 0.5, colour} through `00B256F0` into renderer+1D00h (the array `00B28D00` draws) |

`00B290D0` callers found by a `MOV reg,[reg+0B4h]; CALL reg` scan: `BSP_FoliageEntryNode_DrawBox`
(`00AE0A50`, twelve per box) and `BSP_FoliageQuadNode_DebugDraw` (`00AE37F0`).

2D endpoint transform (`debug_line_2d_endpoint`), checked on both branches with
`tools/x87_sym.py 00b28ac0` (`--take 00b28b7d,00b28bb8` for the second): x is first scaled by
0.75 (`00CEE07C`) when byte `[[0109CF04]+0Dh]` is set, else by 1.0 (`00D7A24C`), and stored to a
float. Pixel mode (byte renderer+1D88h): x' = (x + 0.5) * viewport_w, y' = (y + 0.5) *
viewport_h, with 0.5 the double `00D7A280`, the viewport from renderer vt+A8h then `00B1F740`, each
int FILD'd and stored as float. Normalised mode: x' = x, y' = -0.0 - y. The aspect scale is never
applied to y.

## Texture, surface and declaration slots

| address | owner / slot | name | notes |
|---|---|---|---|
| `00B264A0` | renderer +34h | `BSP_D3D9Renderer_GetHardwareLayoutForDeclarationName` | own vt+38h (`BSP_Renderer_LoadVertexDeclaration`) then `[00F8D394]` vt+40h (`BSP_Renderer_GetOrCreateHardwareLayout`) with a 14h stack key {decl, ?, ?, ?, count = 1}; releases the declaration. Key slots 1..3 are uninitialised: provisional |
| `00B2A380` | renderer +8Ch | `BSP_D3D9Renderer_CreateRuntimeCubeTexture` | stack (edge, levels, format, flags), RET 10h; flag decode identical to `00B2A070` (`decode_runtime_texture_flags`); device+64h CreateCubeTexture; retry predicate reconstructed (`cube_texture_create_should_retry`); `00B3F2C0` pool + `00B3D650` construct; append to renderer+1B00h. Unlike the 2D path there is no render-target accounting |
| `00B31AC0` | renderer +68h | `BSP_NativeRenderer_FindCachedTexture` | twin of `BSP_NativeRenderer_LoadTexture` (+64h): `00B31B55 PUSH 0; PUSH 0` where the loader pushes allow_load = 1 (`BSP_NativeTextureCache_Load` argument 4) |
| `00B3DF40` / `00B3E080` | cube / volume slot 2 | `BSP_CubeTexture_ReloadFromFile`, `BSP_VolumeTexture_ReloadFromFile` | file through `[0109CEEC]` vt+4, memory stream, renderer vt+6Ch name notice, D3DXCreate{Cube,Volume}TextureFromFileInMemory, GetLevelDesc(0) -> +24h width, +18h format, +14h level count (volume also +2Ch) |
| `00B3FE90` | cube +34h | `BSP_CubeTexture_GetFaceSurface` | stack (level, face), RET 8; GetCubeMapSurface(face = arg 2, level = arg 1) |
| `00B3E050` / `00B3DF10` | cube +30h / 2D +50h | `BSP_CubeTexture_SaveToFile`, `BSP_D3D9Texture2D_SaveToFile` | D3DXSaveTextureToFileA(name+4 or "", format, COM, NULL) |
| `00B3D5C0` / `00B3D030` | 2D +38h / volume +30h | `BSP_D3D9Texture2D_UnlockLevel`, `BSP_VolumeTexture_LockBox` | COM+50h / COM+4Ch |
| `00B3CE40`, `00B3CF60`, `00B3D080` | 2D/cube/volume +10h | `..._GetKind` | return 0 / 1 / 2 |
| `00B3CF70`, `00B3CF80` | cube +38h/+3Ch | `BSP_CubeTexture_GetReported{Width,Height}` | +24h / +28h |
| `00B3CE20` / `00B3CD30` | surface setter / vt+24h | `BSP_D3D9Surface_{Set,Get}SampleCount` | surface+24h; setter callers `BSP_RenderService_SetMotionBlurAndSamples`, `BSP_PostEffectCapture_SetSurfaceIndex` (matches CC13_SCENE_RENDERER's "00B3CE20 sets target+24h") |
| `00B3CD70` | surface +38h | `BSP_D3D9Surface_LoadFromSurface` | D3DXLoadSurfaceFromSurface(this+2Ch, NULL, dst rect, src+2Ch, NULL, src rect, FILTER_NONE, 0) |
| `00B3F450` | | `BSP_D3D9Surface_ConstructDefault` | callers: hi-res tiles and both screenshot savers |
| `00B33AC0` | | `BSP_Renderer_SetObservedSynchronizationMode` | writes only `0108D6DD`; caller `BSP_TacticalLibraryScreen_Enter` |

## Hardware-layout slab pool (pool `[0108FD70]`)

`00B48780` allocate, `00B479C0` return (called by the layout deleting destructor `00B48C80`),
`00B47750` slab initialise. The allocator's only reference is the stub `00B488D0` (`MOV ECX,0108FD70;
JMP`), which is itself unreferenced, so allocation from this pool is unreachable in the image; the
live layout allocator `BSP_HardwareVertexLayout_AllocatePooledStorage` (`00B606F0`) uses pool
`0108FE9C`. Releases into `0108FD70` are reachable through `00B48C80` (vtable `00D61D14`). Same
scheme as the vertex-declaration pool (`00B47680`/`00B48560`, D4h slots) with 44h slots: a slab is
8C4h bytes, 32 slots whose last DWORD (+40h) holds the slab index, a u16 free stack at +880h
(initialised 31..0) and a u16 free count at +8C0h. Pool: CRITICAL_SECTION +0Ch, depth +24h, slab
pointer vector +28h/+2Ch/+30h (growth cap*2+2), first slab with a free slot +34h (-1 none).
Allocation creates a slab only when +34h is -1; a failed slab `operator new` is stored as null and
then dereferenced (no native null check). Return computes the slot index by signed division by 44h
(`IMUL 78787879h`) and lowers +34h with an unsigned compare. All three are reconstructed.

## Material and vertex-stream helpers (segment 85)

- `00B44C00 BSP_MaterialSection_ApplyInstanceGeneratorModes`, called once from
  `BSP_MaterialEffect_AttachInstanceGenerator` at `00B4534C`: when byte section+80h is set, for each
  of the count +6Ch bindings in the inline array +5Ch, resolve binding+8 through `00B1BC70`, use
  "Default" (`00CE3C5C`) when the result is empty, and call (binding+4)->vslot 1Ch with the pooled
  temporary. Reconstructed; the temporary string's lifetime is the host's.
- `00B4A9B0 BSP_D3D9LogicalVertexStream_ConstructShared` (stack vertex_count, declaration, buffer,
  byte_offset; RET 10h): vtable `00D61D6C`, +58h buffer and +68h declaration retained, +5Ch offset,
  +60h = 1, +64h count, then `BSP_Renderer_RegisterExistingVertex`. Field meanings from the getters
  `00B48CD0/CE0/D10/CF0`. Named only.

## Segment 90: resource-instance STL

The segment is the `std::map` (node 24h, key +0Ch, value +10h, isnil byte +21h, left/parent/right
+0/+4/+8) and `std::vector` of 8-byte pairs behind `BSP_ResourceInstance_PublishTypedPair` and
`BSP_ResourceInstance_Destroy`, plus the factory deleting destructors. Named `STL_*` after the
existing `STL_ResourceInstanceTypeTree_LinkNode`; not reconstructed (library templates). Several
have register arguments Ghidra shows as `__fastcall`; their prototypes were not repaired.
`00B89FE0 BSP_Resource_CreateInstance` (resource vtable `00D63228` slot 4) is the one engine
function: `operator new(3Ch)` + `BSP_ResourceInstance_Construct(this)`.

## Unreferenced residue

113 bodies in the three segments have no reference at all (111 named here, plus `00B27A40` and `00B3D5E0`, left unnamed because their owner class is unknown): push_back / clear / reserve / grow /
reallocate instantiations of the renderer's pointer arrays, tree head init/destruct copies,
COM/ref pointer releases, pooled-string releases and two CPU-copy commits
(`00B493E0`, `00B49490`). Each is named `<Owner>_<Operation>_Unreferenced[_addr]` with a one-line body
description in the ledger. They are kept COMDAT bodies, not dead engine features.

## Reconstruction coverage

| routine | C++ | coverage |
|---|---|---|
| `00B26A10` + `00CD7A80` box data | `draw_debug_frustum_00b26a10`, `build_debug_frustum_box` | complete (device and renderer calls are host methods) |
| `00B26C60` | `draw_debug_plane_normals_00b26c60`, `build_debug_plane_normal_lines` | complete; the inline shader clear is one host method |
| `00B27440`, `00B27780` | `draw_debug_texture_quad_00b27440/_00b27780`, `build_debug_texture_quad` | complete |
| shared state tail | `restore_debug_draw_states` | complete |
| `00B290D0` | `append_debug_line_00b290d0` | complete except the record-guard lock (caller's responsibility) |
| `00B28AC0` | `append_debug_line_2d_00b28ac0`, `debug_line_2d_endpoint` | complete except the record-guard lock and the viewport fetch (arguments) |
| `00B2A380` | `decode_runtime_texture_flags`, `cube_texture_create_should_retry` | partial: flag decode `00B2A3B6..00B2A472` and retry test `00B2A4A2..00B2A4BF` only; device call, recreate, pool/constructor and owner append `00B2A473..00B2A587` are not reconstructed |
| `00B47750`, `00B48780`, `00B479C0` | `initialize_layout_slab_00b47750`, `allocate_layout_slot_00b48780`, `return_layout_slot_00b479c0` | complete over raw offsets |
| `00B44C00` | `apply_instance_generator_modes_00b44c00` | complete; pooled-string temporary and its release are host-side |
## Open questions

- About 400 16-byte-aligned code starts after INT3 padding in the three ranges are still not
  Ghidra functions (for example `00B25E00 MOV EAX,ECX; RET 8` and the uninitialized-copy wrapper
  `00B87C80`, which calls `00B87640`). cc13-define2 is defining functions image-wide, so this packet
  left them to it; my scan script is `local/r5_undef.py` in the worker tree.
- The callers of the virtual debug draws (+CCh, +D0h, +D8h, +DCh, +C0h) were not found by the
  `MOV reg,[reg+disp32]; CALL reg` scan; they may be unused debug entry points.
- The trailing FILLMODE = 2 in the shared state tail.
- Trivial no-op/constant leaves the define pass created in these vtables (`00B28450`..`00B284C0`,
  `00B33E80`, `00B33F30`, `00B3C4F0`, `00B48D30`, ...) were left unnamed.

## Handoff

Packet complete for the anchored and referenced code; nothing is in flight. A follow-up could
(1) define and name the undefined aligned bodies once cc13-define2 has passed these ranges,
(2) repair the register prototypes of the seg-90 STL helpers, (3) reconstruct `00B2A380` fully
(it needs the cube-texture pool `00B3F2C0` and constructor `00B3D650`, both outside this
packet's reconstructions).

## Aligned-start sweep (2026-10-12 06:09Z, cc13-render5 follow-up packet)

The open question above (about 400 aligned code starts that were not Ghidra functions) is closed.
Record: `reports/cc13_render5_aligned_starts.json` (per-candidate plan, reason for every
rejection, post-definition verification) and the define tool's event log
`reports/cc13_render5_aligned_starts_define_log.json`. Rules follow
`docs/CC13_UNDEFINED_FUNCTION_SWEEP.md`; nothing was named or reconstructed.

| | count |
|---|---|
| 16-byte-aligned candidates in the three ranges (not an index entry, previous byte INT3/NOP padding or the bytes before end a RET / RET imm16 / JMP rel32) | 551 |
| defined | 415 (391 after padding, 24 after a terminator) |
| not defined: already inside a live Ghidra function | 117 |
| not defined: flow check failed (conditional or unconditional branch leaving the gap, register/indirect jump, undecodable, no padding after the end) | 17 |
| not defined: after-terminator start shown to be mid-function (`00B40630`, `00B49690`) | 2 |
| not defined: leased to another owner | 0 |

Method: live `get_function_by_address` at the start; Capstone recursive descent from the start,
bounded by the next index entry or candidate, with tail jumps allowed only to known entries; the
end must be followed by INT3 padding to the next 16-byte boundary or meet the bound; live
containment checked again at the middle and last byte; an after-terminator start was kept only
when the live function before it ends exactly at it, or a planned body does. "Non-Returning
Functions - Discovered" was checked disabled first; the bodies were written in six locked
batches of at most 80 with `tools/ghidra_define_function.py`. All 415 bodies were re-read and
end exactly where planned. `_free` (`00BF65AC`) still reads `void __cdecl _free(void *)`
afterwards. One forced snapshot and index rebuild followed (function count 75434 -> 75851,
including other workers' concurrent definitions).

The largest body is 122 bytes; they are the same template/COMDAT residue as the named ones
(container helpers, scalar deleting destructors, trivial vtable leaves), left `FUN_`.
