# CC13 post effects, .mshd shader descriptions and render resources

Packet `cc13_render3_post_effects` (worker cc13-render3). Segments 86 (`00B4B490-00B65AC0`)
and 91 (`00B8A2B0-00B999A0`). Source: `src/post_effects_cc13.cpp`,
`include/bsp/post_effects_cc13.hpp`. Ledger shards `config/names/00b40000..00b90000.jsonl`
and the matching `config/reconstruction` shards. Call-site claims are in
`reports/cc13_render3_calls.json` (checked with `tools/verify_report_calls.py`). Functions
that had no Ghidra body are listed in `reports/cc13_render3_defined.json`.

Every name is a descriptive hypothesis, not a recovered symbol.

Status vocabulary: **exported** (Ghidra pseudocode in `exports/`), **named** (ledger and
Ghidra rename with an evidence comment), **reconstructed** (C++ in this packet),
**build-tested** (compiles in `scripts/build.ps1`, existing tests pass). Nothing here is
fixture-tested, ABI-compatible or game-validated. The C++ functions are new interfaces; the
original ABI of each routine is recorded in the header and in its ledger record.

RNG: no routine read in this packet calls the shared generator `00BD2F10`. The film-grain
and dust effect (`00B529A0`) draws no random numbers either; its dust particle count comes
from the authored dust amount (see below). The ocean spectrum in segment 91 seeds its own
local Mersenne state (`docs/OCEAN_WAVE_FIELD.md`), which this packet did not touch.

## Segment 86

Segment 86 had 252 records already (the physical vertex and index buffers, the post-effect
owner constructors and initialisers, the material and occlusion-query leaves). This packet
named the 105 remaining Ghidra functions plus three Ghidra-less vtable entries.

### Texture alpha fix-ups (reconstructed)

The particle texture loader `00B00950` dispatches on the loaded format: `15h`
(`D3DFMT_A8R8G8B8`) jumps to `00B4C6A0`, `16h` (`D3DFMT_X8R8G8B8`) to `00B4C6D0`, DXT1
(`31545844h`) and every other format return untouched. Each wrapper takes level 0 through
the texture's vt+30h getter (`00B3FD80`, retained), runs the pixel walk, then releases the
surface owner (intrusive count at +4).

| Address | Name | Effect |
|---|---|---|
| `00B4C490` | `BSP_Surface_PremultiplyAlphaBgra` | B, G, R scaled by A/255 |
| `00B4C5D0` | `BSP_Surface_AlphaFromNonBlackBgra` | A = FFh when B+G+R != 0, else 0 |

Both lock the whole surface (vt+2Ch with flags 0) and index pixels by `width*y + x`: the lock
pitch in the D3DLOCKED_RECT at `[ESP+10h]` is never read, so a padded pitch would shear the
walk. The unlock (vt+30h) runs only when the lock returned non-zero. Width is vt+1Ch and
height vt+20h of the surface owner vtable `00D619A0` (`BSP_D3D9Surface_GetWidth/Height`).

### Post-effect sample tables (reconstructed)

These are the DirectX SDK HDRLighting sample helpers, recognisable from their constants and
loop shapes, with the game's layouts. All take extents as unsigned (FILD then FADD 2^32 from
`CE3978` when negative) and write `float4` rows.

| Address | Name | Rows | Notes |
|---|---|---|---|
| `00B4CC50` | `BSP_PostEffect_SampleOffsetsDownScale4x4` | 16 | x from doubles `CE3D78` 1.5, `CEC9E0` -0.5, `D7A280` 0.5; only xy written |
| `00B4CDE0` | `BSP_PostEffect_SampleOffsetsAvgLum3x3` | 9 | tu = 1/(3w), tv = 1/(3h) with `D7A2B0` 3.0; y factors `D7A250` -1, `D7A258` 0 |
| `00B4CEA0` | `BSP_GaussianDistribution` | - | exp(-(x²+y²)/(2ρ²)) / sqrt(2πρ²); 2π is `CE3828` = (double)(float)2π |
| `00B4D500` | `BSP_PostEffect_SampleOffsetsBloom15` | 15 | offsets (i/w, i/h) for i = 1..7, mirrored with `-0.0f - v` (`D7A208`) |
| `00B4D980` | `BSP_PostEffect_SampleOffsetsGaussBlur5x5` | 13 | \|x\|+\|y\| <= 2, ρ = 1, normalised by the float total then times the multiplier |

ABIs: `00B4CC50`/`00B4CDE0` are `__stdcall`, RET 0Ch; `00B4CEA0` `__stdcall` returning ST0,
RET 0Ch; `00B4D500` `__stdcall`, RET 18h; `00B4D980` `__stdcall`, RET 14h. The x87 order was
checked against the listings (for example `00B4CCD4` is `DC C9`, FMUL ST(1),ST(0), so the
fourth column is 1.5·inv_w and the row factor is (row - 1.5)·inv_h, as the pseudocode says).
Bloom differs from the SDK in two ways: each tap moves on both axes at once (`(i/w, i/h)`),
and the inlined Gaussian keeps x² and 2ρ² as doubles before rounding the exponent to float,
while the stand-alone `00B4CEA0` rounds through float. The exp call is the CRT stub
`00BFEEEC` (SSE2-dispatching `_CIexp` shape; named by role only).

Consumers: the luminance owner `00B50050` (`BSP_LuminanceOwner_MeasureAndAdapt`) takes the
3x3 table into its first pass and the 4x4 table into the three downscale passes over
`+234h/+230h/+22Ch`, then swaps the adaptation targets `+238h/+23Ch` and draws the
adaptation pass. The distortion owner render `00B4F1C0` fills its blur offsets/weights at
`+40h/+140h` with `00B4D980(width, height, ..., 1.0)`. The bloom owner render `00B54CE0`
(Ghidra-less until this packet; vtable `D62150` slot +10h) runs a 5x5 blur into `+228h/+328h`,
then `00B4D500` twice with deviation `+438h`: after the first call it zeroes the y component
of all 16 offsets (horizontal pass), after the second the x component (vertical pass). That
is why the bloom table moves on both axes: each pass discards one.

### Full-screen primitive (reconstructed fill)

`00B4DCC0` (`BSP_PostEffect_DrawFullscreenPrimitive`, ECX pass, one unused stack word, RET 4)
is the draw every post-effect pass ends with. It sets the camera viewport to the frame colour
surface size, binds camera and viewport on the renderer `00F8D394` (vt+A0h, A4h, 98h), locks
the generated model's stream 0 for `pass+1Ch` vertices, writes the primitive and draws through
`pass+14h` material `+7Ch` effect `+9Ch` vt+8 with `pass+18h`. Vertices are seven floats:

| count | vertices (x, y, z, w, u, v, index) |
|---|---|
| 3 | (-3,-1,.5,1,-1,1,0) (-3,5,.5,1,-1,-2,1) (3,-1,.5,1,2,1,2) |
| other | (-1,-1,.5,1,0,1,0) (-1,1,.5,1,0,0,1) (1,-1,.5,1,1,1,2) (1,1,.5,1,1,0,3) |

The three-vertex form is the oversized triangle that covers clip space with u = (x+1)/2,
v = (1-y)/2. `fill_fullscreen_primitive` reproduces the table only; the lock and draw remain
host work. The distortion owner vtable `00D61F1C` slot +10h is the Ghidra-less thunk
`00B4DFB0` (`ECX = [ECX+8]; push 0; call 00B4DCC0`), slot +0Ch the bare RET `00B4EC40`.

### Distortion owner and other post-effect plumbing (named)

`00B4EF90` appends a retained (node, object) pair to the owner's `+254h` array (or, with a
null object, propagates root registration of `+3Ch`); `00B4F030` releases the pairs and
resizes the array to zero. `00B4F1C0` renders: a one-time clear of both ping-pong targets
(`+251h`), the scene pass through camera `+34h` for nodes under `+3Ch`, the blur, two
full-screen passes, the distortion objects when `+260h`, then swaps `+10h/+14h`.

`00B529A0` (`BSP_PostEffectState_RenderOldFilm`, 820 pseudocode lines, caller `00B156C0`)
recreates the `oldfilm_dust.mshd` child (`BSP_PostEffect24h_Construct`, record count
`2*round(params[3] * D7A220)`) whenever `params[3]` changes, derives an aspect value
`2*h/w` from frame colour surface 0, and drives `BSP_PostEffectGeometry_PopulateCurrentStream`
and `BSP_PostEffect_SubmitCurrentEntry`. No `00BD2F10` call appears in its callee list; it was
named, not reconstructed.

The remaining helpers are vector templates (`PushBack`/`Reserve`/`Resize` instantiations for
pointer, 8-, 0Ch-, 1Ch- and 24h-byte records), intrusive and COM release holders, and four
critical-section getters with no static reference (`00B5DF90..00B5E020`). Each has a ledger
record with its ABI.

### .mshd material-shader descriptions (reconstructed parse)

`.mshd` files are Lua. In this installation `shaderfx/common/alphachannel.mshd` (mtime
2024-07-13, the untouched bulk set) shows the shape: `DoFile("fshaders/dx9_lua.inc")`, then
a global `Shader` whose integer children are LOD phases, whose children are versions (DX8,
DX9 ...), whose children are render modes (`RM_*`), whose children are layers (passes). The
enumerations (`RM_NORMAL = 0` .. `RM_COCKPIT = 12`, `RPID_*` render queues, `FT_*`, `FM_*`,
`LIT_*`, `STYPE_*`, `TEXSRC_*`) are in `shaderfx/dx9_lua.inc`.

| Level | Reader | Object | Fields read |
|---|---|---|---|
| shader | `00B5B3F0` | 1Ch, vtable `D62200`, ctor `00B571E0` | own Lua state, global `Shader`; phase+04h = ordinal, phase+18h = root; root at owner+28h |
| LOD phase | `00B5B240` | 1Ch, vtable `D621F0` | `LodValue` (+08h, 0 when nil); version+74h = phase |
| version | `00B5AFF0` | 78h, vtable `D62264` | `Name`, `Caps` (`00B55F00` at +0Ch); mode+1Ch = version |
| render mode | `00B5ADF0` | 20h, vtable `D621FC` | `Name`, `RenderMode` (unconditional); layer+50h = mode |
| layer | `00B5A1F0` | 78h, vtable `D621F8`, ctor `00B56B90` | see below |
| caps | `00B55F00` | 44h block in the version | `MaxSamplers` +10h, `MaxTextureRegisters` +14h, `MinVertexShaderVersion` +2Ch, `MinPixelShaderVersion` +28h; both versions default 101h |
| constants | `00B59E70` | 10h list, rows 1Ch | name lookup and positional dimensions |

Layer keys in read order, with offset, gate and default (`00B56B90`):

| Key | Offset | Gate | Default |
|---|---|---|---|
| `Name` | +04h | none (string) | - |
| `RenderQueue` | +54h | none (integer) | - |
| `VertexFormat` | +70h | string | empty |
| `Fog` | +10h | integer | 0 |
| `FogType` | +14h | integer | 1 (FT_NORMAL) |
| `ReceiveShadows` | +18h | Lua boolean | false |
| `DirectionalLightning` | +19h | Lua boolean | false |
| `PointLightning` | +1Ah | Lua boolean | false |
| `DiffuseLight` | +1Ch | integer | 0 |
| `SpecularLight` | +20h | integer | 0 |
| `ReservedColorRegisters` | +5Ch | integer | 0 |
| `ReservedTextureRegisters` | +60h | integer | 0 |
| `PerPixelNormalize` | +58h | Lua boolean | false |
| `RenderStates` | +0Ch | table, via `BSP_ShaderDescriptor_ReadRenderStates` | empty list |
| `Samplers` | +24h vector | table; integer children via `BSP_ShaderStateDefinitions_ReadSampler` | empty |
| `VS_Target` | +30h | none (string) | - |
| `VS_Constants` | +64h | table, `00B59E70` | null |
| `VS_Body` | +38h | none (string) | - |
| `PS_Target` | +40h | none (string) | - |
| `PS_Constants` | +68h | table, `00B59E70` | null |
| `PS_Code` | +6Ch | integer | 1 |
| `PS_Body` | +48h | none (string) | - |

Observed behaviour worth knowing:

- The three lighting flags are read with the Lua-boolean predicate `00B66000`. The installed
  `alphachannel.mshd` writes `PointLightning = LIT_PERPIXEL` (the number 2), which fails that
  test, so the layer keeps `false`. The image ignores numeric lighting modes in that slot.
- `Name`, `VS_Target`, `VS_Body`, `PS_Target` and `PS_Body` are read without a type test; the
  three shader strings are measured with an inline strlen on the `lua_tostring` result, which
  would fault on a missing key. The reconstruction throws instead (the `MshdTable` contract).
- The version allocator writes only +04h/+08h and +50h..+70h. Without a `Caps` table the caps
  block +0Ch..+4Fh is uninitialised heap; with one, only the four fields above are set. The
  reconstruction models that as `std::optional`.
- Constants rows (`00B59E70`): each integer-keyed table entry is walked in `lua_next` order
  with a position counter that advances on every entry. A string looks up
  `BSP_ShaderConstants_FindByName` and, on a hit, seeds the row from the definition's +08h,
  +0Ch and +10h (`00B5B840/850/860`); numbers at positions 1, 2 and 3 then override those three
  (defaults 0, 0, 1). `00B58160` stores the running register `list+0Ch` in the row and adds
  `dimension_8 * array_count`. The definition table itself (`BSP_ShaderConstants_FindByName`)
  was not reconstructed; the C++ takes it as a callback.
- `00B5B3F0` has no static caller (no rel32 or absolute reference); it may be dead or reached
  through a table this packet did not find.

`00B5B8E0` (`BSP_NamedIndexTable_Find`, no static reference) is a case-insensitive
name-to-index scan over 20h records returning 37h on a miss; reconstructed as
`find_named_index`.

### Shader file helpers (named)

`00B60D30` counts float4 registers in a D3DX constant table (GetDesc, then per constant
GetConstant/GetConstantDesc, summing RegisterCount where RegisterSet == 2); the census logic
is reconstructed over pre-fetched descriptors. `00B60DC0`/`00B60E90` read a compiled shader
through the VFS (`0109CEEC`) and call CreateVertexShader (vt+16Ch) / CreatePixelShader
(vt+1A8h); `00B611C0` assembles pixel-shader source with `D3DXAssembleShader` first.

### Matrix and plane helpers (reconstructed)

| Address | Name | Notes |
|---|---|---|
| `00B62A10` | `BSP_Matrix_OrthoOffCenterLH` | D3DXMatrixOrthoOffCenterLH layout, used by the GUI order pass |
| `00B62DD0` | `BSP_Matrix2x2_InvertInPlace` | no zero test; no static reference |
| `00B62F20` | `BSP_Matrix_OrthonormalizeRows` | Gram-Schmidt rows 0-1, row2 = row0 x row1; called at `00B64CB9` |
| `00B64470` | `BSP_Matrix_ComposeEulerScaleTranslation` | `__fastcall(out, t)` + (euler, scale), RET 8; callers `005ADDA0`, `00ACBEC0` |
| `00B65140` | `BSP_PlaneSet_ClassifyPoint` | AAAh when any row has dot(n,p) - d > 0 |
| `00B65560` | `BSP_FrustumPlanes_FromAngles` | six planes from two half-angles, near and far; no static reference |
| `00B65710` | `BSP_PlaneSet_ClassifyAabb` | AAAh when dot(n,c) - d < -dot(\|n\|,e) |

The two plane-set classifiers reject on opposite sides of the plane (the point test rejects
positive distances, the box test negative ones beyond the radius). Both are reproduced as
read; which convention a caller expects was not established (the point test has no static
caller). `00B64CE0` multiplies by a matrix from `0047C2A0` and sets the translation row
(named only; argument roles provisional).

## Segment 91

Segment 91 had 126 reconstruction records (the ocean spectrum core, bone models, animation
channel readers, mesh-resource initialisers). This packet named the 142 remaining Ghidra
functions plus two Ghidra-less flare thunks, and corrected two tag names: `00B90C10` and
`00B90C20` are tagged `cg_static_dtor_stub`, but each is `MOV ECX, pool; JMP allocate` into
the 404- and 400-byte pools (`01090358`, `01090390`).

### Mit::allocator block pools (reconstructed slot order)

Ghidra carries the RTTI name `Mit::allocator_list_elem` for the base vtable, so `Mit` is the
engine's own namespace. Three pool instantiations live here, each a list element of
`00E188B4` with a critical section at +0Ch, a recursion counter +24h, a block-pointer vector
(+28h/+2Ch/+30h) and the current block +34h (-1 = none). A block holds 32 slots whose last
dword is the block index, then a uint16 free-index stack and a uint16 free count.

| Payload | Stride | Block | Init | Allocate | Free | Trim | Vtable |
|---|---|---|---|---|---|---|---|
| 480 (flare, pool `010902A8`) | 1E4h | 3CC4h | `00B8B6D0` | `00B8D700` | `00B8C3A0` | `00B8D660` | `D63410` |
| 404 (skin model, quadtree node) | 198h | 3344h | `00B8FA80` | `00B90800` | `00B90020` | `00B90760` | `D6355C` |
| 400 (bone model) | 194h | 32C4h | `00B8FB50` | `00B90AC0` | `00B90150` | `00B90A20` | `D63560` |

A fresh block's stack is `31..0`, so slot 0 is handed out first. When the current block
empties, the pool advances to the first later block with a free slot (never an earlier one);
a free lowers `current` to the freed block's index (an unsigned compare, so it also replaces
-1). Trim releases every completely free block by moving the last block into its place and
renumbering that block's slots. `MitBlockPool` reproduces exactly this order; it holds no
memory. Ghidra treats the `CALL _free` in `00B8D660` as no-return, so the listing omits the
move-and-renumber tail `00B8D686..00B8D6BF` (read from the PE with `disasm-raw`); the same
pattern is likely in the other two trims. This is a listing repair candidate, not changed here.

The flare class uses the 480-byte pool directly: `00B8D840` (Ghidra-less) is its operator new
(`MOV ECX,010902A8; JMP 00B8D700`), `00B8C6A0` (Ghidra-less) its operator delete, and the
scalar deleting destructor `00B8D9A0` frees into the same pool.

### Flares (reconstructed helpers)

The flare object (vtable `D63420`, 23 slots, all generated-model base methods except the
scalar delete) keeps the projected clip position at +1A4h/+1A8h and a visibility at +1B4h.

| Address | Name | Reconstructed as |
|---|---|---|
| `00B8B350` | `BSP_Flare_CentreWeight` | `(1.4142136 - r) * 0.2 + 0.05`, r = distance from screen centre |
| `00B8B3B0` | `BSP_Flare_EdgeFade` | linear fade in the outer 6% band of each screen edge, 0 outside |
| `00B8B670` | `BSP_Flare_PairTest` | `+1C0h != 0 && +1C4h >= +1C0h` |
| `00B8B8C0` | `BSP_FlareOwner_PlaceElement` | `base + dir * owner^t` via the CRT log/exp stubs |
| `00B8C520` | `BSP_Color32_Scale` | per-byte scale, clamp to [0,255], FISTP rounding |
| `00B8B980` | `BSP_FlareGeometry_WriteDiscFan` | index pattern only: `(0, i+1, (i+1)%n+1)` |

The x87 compare order of `00B8B3B0` was traced from the listing, including how unordered
compares fall through (a NaN position skips the range rejections, as COMISS/FCOMI branch in
the image). `00B8B8C0` uses the CRT stubs `00BFA910` and `00BFEEEC`; both have the SSE2
dispatch prologue and are taken as log and exp from the exp(log(a)*t) shape only. The flare
draw path (`BSP_Flare_UpdateFromCachedOcclusionSamples` `00B8D150`, the quad writers
`00B8C6B0/00B8C8A0/00B8CC90`, the billboard rebuild `00B8BDD0` and the geometry builder
`00B8DD60` with `flare2.mshd`, `flareocclusion.mshd` and `simplecolor2.mvfm`) is named, not
reconstructed; `docs/FLARE_QUERY_SUBMISSION.md` covers the occlusion-query side.

### Groups, skinned/bone models, quadtree nodes, resources (named)

`D634B0` is the `cGroupParamsResource` vtable (slot +18h `00B8EB20` writes its params into a
node's +184h when the node accepts type token `0109032C`); `D634F8` is `cGroup` (clone
`00B8F6F0`, render `00B8EF10` with the sphere/plane classifier, node detach `00B8F090`).
`D63590` is the bone model (clone `00B912E0`, 400-byte pool), `D635F0` the skin model (render
`00B90490`, animator step `00B8FDE0`, 404-byte pool), `D63650` the quadtree node (clone
`00B91C40`, render `00B91A50`, build `00B91D70` with the `QuadTreeNode` name). Which of
`cSkined3dObject` / `cMatrixIndexed3dObject` (type-info builders `00B90FB0` / `00B911D0`)
belongs to which vtable was not settled. Mesh-resource constructors and model factories
(`00B93020`, `00B930F0`, `00B931F0`, `00B93680..00B93A00`) are named from their vtables.

### Ocean FFT and grid indices (named)

`00B94E90`, `00B950B0` and `00B952D0` are the bit-reversal and butterfly stages behind the
inverse FFT `00B95AA0` (`docs/OCEAN_WAVE_FIELD.md`); `00B962E0..00B96960` are the spectrum
samplers (`docs/CC13_WATER_RENDER.md` covers `00B962E0/00B96450`). `00B98BE0` builds a 16-bit
index list for one grid patch with edge-stitch flags (bit 0 doubles the edge resolution), using
three edge-strip writers, and `00B99790` builds every level and flag combination. These were
named only; reconstructing them belongs with the water lane.

## Verification

- `scripts/build.ps1` passes in the worktree (bsp_core with `src/post_effects_cc13.cpp`; the
  two existing CTest suites pass).
- `tools/verify_report_calls.py reports/cc13_render3_calls.json`: 26 call rows, 0 failed.
- Constants were read from the PE with `tools/pe_const_read.py`; RET sizes from the listings.
- Not done: native differential fixtures, game runs. No new tests were added.
