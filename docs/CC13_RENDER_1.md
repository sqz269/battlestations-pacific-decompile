# Segments 84-86: renderer helpers, resource wrappers and post effects (cc13-render1)

Segments 84-86 are `00B25DC0..00B65AC0`: the D3D9 renderer object helpers, the texture and
surface wrappers, the shader builder and binary cache, the slab pools of the resource
layer, the distortion / luminance / bloom / old-film post effects, the mshd shader
description tree and the shader file loaders. Earlier cc13 passes (render3, render5, the
scene renderer) and cc12 covered the call spine; this pass took the residue: every
function with a reviewed ledger name and neither a reconstruction record nor a tag.
Names are descriptive hypotheses, not recovered symbols.

Status words: *reconstructed* (C++ in `src/render_fx_cc13.cpp`, declarations in
`include/bsp/render_fx_cc13.hpp`, namespace `bsp::cc13_render1`), *build-tested*
(`scripts/build.ps1` passes). Nothing here is fixture-tested, ABI-compatible or
game-validated. These are new C++ interfaces over host data: every image routine a body
calls is reached through an abstract host interface or a callable named by its address,
so nothing is stubbed. The sample-table helpers (00B4CC50, 00B4CDE0, 00B4D500, 00B4D980)
and the texture alpha fix-ups are called through their existing reconstructions in
`bsp/post_effects_cc13.hpp`; the HLSL helper texts reuse the literals in
`src/shader_*_literals.inc` after a byte comparison against the PE.

RNG: nothing in this pass draws from 00BD2F10. The old-film effect (00B529A0) draws from
stream 0 in the image (00BD2F10 through BSP_RandomThreads_NextU32); it is not
reconstructed here, and nothing here is wired into a host.

## Triage

At `origin/main` b9506ce61 the range held 453 functions with a reviewed ledger name, no
reconstruction record and no tag. 332 of them had no decompile export; `bsp.py ghidra
export` filled those first. A normalised-body grouping (callee names, addresses and hex
immediates masked) reduced the 453 to 218 distinct bodies, 65 of them shared by two or
more instantiations. Where the masked constants differ inside a group (slab slot sizes,
field offsets, element strides) each row was re-read from its own body or bytes.

Result: **452 of 453** have a reconstruction record (`reconstructed_build_tested`, three
of them backfills for bodies the scene-renderer pass reconstructed without records, see
below). 308 records point at rows of the per-address table `kRows`
(scope `table_model`): container, slab, reference-count, accessor and trivial shapes whose
native bodies differ only in stride, offset or the named callee. 141 point at individual
models. One body remains: `00B529A0` (old film).

## Corrections to earlier names and ABIs

| Address | Old | New | Evidence |
| --- | --- | --- | --- |
| 00B40D70, 00B40DA0, 00B40DD0, 00B40E50 | `*_PushBack_<addr>` | `*_Resize_<addr>` | reserve(n) when capacity < n, count stepped down to n, count = n; no element is copied in |
| 00B34860 | `BSP_ShaderDescriptor_FindFirstFlaggedSampler_Unreferenced` | `..._CountUnflaggedSamplers_Unreferenced` | the loop runs over all +C8h cells and counts the ones whose byte +0Ch is zero; no early exit (00B34875..00B34884) |
| 00B4F1C0 | ledger ABI `__fastcall` | `__thiscall(owner, camera)`, RET 4 | `RET 0x4` at 00B4F52E; the distortion-object entries take that camera ([ESP+28h] at 00B4F4D4) |
| 00B2B270 | decompile showed no stack argument | `__thiscall(renderer, camera)`, RET 4 | `RET 0x4` at 00B2B573; camera = [ESP+78h] at 00B2B49E, passed to 00B46A70 (EDX) and 00B51A20 |

Ledger and Ghidra were updated together for the five renames (`tools/ghidra_annotate.py
--apply`; prior values in `local/ghidra-annotations-20261013T003211Z.json` of the worker
tree). The ABI notes are carried in the reconstruction records.

## Shared shapes (table `kRows`)

- **Engine arrays** `{data, count, capacity}`: clear through the element resize (18 rows),
  destroy = resize(0) + `_free` (14), count-down clear of POD records (9, including
  00B29230 on renderer +1CF4h), reallocate-exact / grow (2n+2) / reserve-at-least
  (unsigned compare) / push-back-grow on pointer cells (9 + 9 + 9 + 9), push-back through
  a named reserve (24) with the capacity rules double/min 1, +5/min 10, +4/min 6 and
  double/min 256 (00B51C20), reserve-min-one (4), resize with zero fill (6), FFh fill
  (00B34A50) or no fill (4), element and end addresses, construction, pop-back with a
  named element destructor, free-data with and without clearing the pointer.
- **32-slot slabs**: every slab pool in segment 85 lays a slab out as 32 slots of S bytes,
  the slab index in each slot's last dword, a u16 free-index stack at 32*S and its count at
  32*S + 40h. S per pool: D3D9 surface 38h, 2D texture 54h, cube texture 34h, material pass
  8Ch, vertex declaration D4h, hardware layout 44h, physical buffer 30h, logical vertex
  stream 78h, logical index stream 28h (the 0700h/0740h, 0A80h/0AC0h, 0680h/06C0h,
  1180h/11C0h, 1A80h/1AC0h, 0880h/08C0h, 0600h/0640h, 0F00h/0F40h, 0500h/0540h pairs in
  the bodies). Pop, push, empty, full and stamp rows for each (45 rows).
- **Reference counting**: engine ref-counted objects keep the count at +04h and delete
  through vtable slot 0; the assign shape (`__fastcall(ECX slot, EDX source*)`) is a no-op
  when unchanged, otherwise store, retain new, release old. COM wrappers use AddRef
  (+04h) / Release (+08h); the "reference count probe" returns AddRef() - 1 after a
  balancing Release.
- **Accessors and trivial bodies**: field getters (+24h sample count, +24h/+28h cube width
  and height, logical stream +10h/+14h/+18h/+60h), locked getters (critical-section
  pointer at +48h or +4Ch, depth word at cs+18h), no-op slots, texture kinds (2D 0, cube 1,
  volume 2), forwarding unwind thunks.

## Class layouts used

- **Debug line records**: renderer +1CF4h {data, +1CF8h count, +1CFCh capacity} of 16-byte
  {x, y, z, colour}; the generated model at +19E0h ("debugshader.mshd", "pf43cc.mvfm",
  "DebugLines", kind 2).
- **D3D9 surface wrapper** (vtable 00D619A0, slot 38h): +08h/+0Ch = [00D7A24C] (1.0f),
  +24h sample count, +2Ch IDirect3DSurface9, byte +30h. **Texture wrappers**: +08h file
  name, +10h COM texture, +14h level count, +18h format, +1Ch word passed to new surface
  wrappers, cube +24h edge, volume +24h/+28h/+2Ch width/height/depth (from the level-0
  descriptor read at 00B3E144..00B3E158).
- **Logical texture base** (vtable 00D5F1F4): +08h name, +10h, +1Ch flags (bit 25 read by
  00B33E90), +20h id from [0108D6E8].
- **Shader reflection owner** (vtable 00D61810): byte maps at +08h and +3Eh (36h bytes
  each, FFh = unused), byte +74h, +78h..+84h.
- **Logical vertex stream view** (vtable 00D61D6C): +58h physical buffer (retained), +5Ch
  byte offset, +60h = 1, +64h count, +68h owner (retained), +6Ch CPU copy.
- **Post-effect pass**: +08h frame targets, +0Ch camera, +14h material, +1Ch render entry,
  +24h/+28h extent; tone-map parameters +210h..+220h (00B548C0).
- **Distortion owner**: +10h/+14h ping-pong holders, +1Ch blur pass, +20h composite pass,
  +34h scene camera, +38h targets, +3Ch scene root, +40h/+140h GaussBlur5x5 offsets and
  weights, +244h, +250h/+251h clear bytes, +254h records {node clone, object clone},
  +260h objects-enabled byte.
- **Luminance owner**: +10h sample offsets, +210h clear pass, +214h average pass,
  +218h/+21Ch/+220h downscale passes, +224h adaptation pass, +228h measured luminance,
  +22Ch/+230h/+234h downscale sources, +238h/+23Ch adaptation ping-pong, +240h the word
  argument, +248h readback surface, +24Ch clear-once byte.
- **Bloom owner**: +08h blur pass, +10h/+14h GaussBlur source size, +18h/+1Ch/+20h holders,
  +24h first pass, +28h/+128h Bloom15 offsets/weights, +228h/+328h GaussBlur5x5 tables,
  +438h deviation.
- **mshd tree**: the layouts of `bsp/post_effects_cc13.hpp`; the layer constructor
  00B56B90 writes exactly the defaults of `cc13fx::MshdLayer` (FogType +14h = 1, PS_Code
  +6Ch = 1) and leaves +50h, +54h and bytes +59h..+5Bh unwritten.

No `{2,&field}` / `{0,"name"}` property tables reference these classes in the range; the
mshd field names come from the existing Lua reader reconstruction.

## Findings worth knowing

- **Luminance half texel** (00B50050): before each of the four measurement draws every one
  of the 16 sample offsets gets x += f(0.5/w), y += f(0.5/h), z += 0.0f, w += 0.0f with w
  and h the DESTINATION pass's colour surface 0 (FILD + 2^32 for negative). 0.5 is the
  double at 00D7A280; both quotients are stored as floats before the adds; the 0.0f adds
  turn -0.0 into +0.0. The first pass reads its source texture from the pass material's
  first slot and dereferences null when the material word +34h is below 1.
- **Bloom passes** (00B54CE0): the horizontal pass clears every offset y, the vertical pass
  every x (16 slots cleared, 15 used); the vertical Bloom15 takes the width of the +1Ch
  texture and the height of the +18h texture; renderer targets are reset (vt+98h(0))
  before the second blur only.
- **Distortion** (00B4F1C0): the one-time clear runs with +244h temporarily 1.0e9f
  (00D217E8); scene models under +3Ch are prepared with vt+58h(camera, 1.0) and drawn into
  the +10h holder; the distortion objects are drawn with the CALLER's camera and their
  object clone in the geometry slot; the holders swap before the objects are released.
- **Luminance readback** (00B51000): locks the readback surface read-only and unlocks it
  without reading anything; the debug use was compiled out.
- **Cube LockRect** (00B3D6D0) forwards its arguments as (2, 3, 1, 4, 5): the wrapper
  takes the level first and the face second.
- **00B3CDA0** (StretchFrom) takes the device from the destination surface, stretches the
  whole source onto {0, 0, width, height} of the destination with D3DTEXF_LINEAR.
- **00B264A0** passes a 14h record to [00F8D394] vt+40h with only words 0 and 4 written;
  words 1..3 are whatever the stack held.
- **00B61F90** copies the eight element words with FLD/FSTP float pairs (bit-exact except
  for signalling NaNs).

## Records not created

| Address | Reason |
| --- | --- |
| 00B529A0 | Old film: an 820-line x87 body with three child rebuilds (dust count from params[3], scratch/line lists from params[8] and params[13]), stack-copied uninitialised records and draws from the shared generator; needs a block-by-block listing trace. |

Backfilled records (reconstructed earlier, `reconstructed_ledger_backfilled_from_source`):
00B33A10 `release_texture_source_frames_00b33a10`, 00B3C960 `cockpit_begin_frame_00b3c960`
and 00B3C6E0 `cockpit_draw_00b3c6e0` in `src/scene_renderer_cc13.cpp`
(docs/CC13_SCENE_RENDERER.md). 00B2B270 was listed as already reconstructed in
`bsp/render_sprites_cc13.hpp`; no source carries it (cc12's files cover 00B28D00, 00B2B580
and 00B2BB90), so it is reconstructed here.

## Open questions

- The class names of the distortion, luminance and bloom owners are not anchored by
  strings in the range; the names follow their use.
- The record words 1..3 passed by 00B264A0 to vt+40h: whether the callee reads them.
- 00B51000's locked readback: whether another build reads the 1x1 luminance there.
