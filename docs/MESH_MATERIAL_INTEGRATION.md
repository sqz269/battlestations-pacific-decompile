# Installed mesh material integration

The existing Win32 D3D9 diagnostic now draws the installed
`models/misc/repulogepdarabok_004.mmod` through its actual Mesh payload,
compressed vertex declaration, decode metadata, subset indices, texture and
NORMAL material shader. This advances the resource-to-render path; it is not a
runnable game rebuild or evidence of original-game visual parity.

## Resource and GPU path

`StructuredResourceRegistry` registers the actual Mesh, Note and GroupParams
decoders in the recovered nonowning parser map. Registration preserves the
native duplicate probe and second type-name call on a miss. Ordered dispatch
appends decoded payloads before closing child handles. An unsupported child
is an explicitly skipped record; it does not impersonate the native eight-byte
fallback item. The installed model has three supported Resource children and
zero skipped payloads. It retains the previous exact byte/cursor checks.

The private mesh adapters create MANAGED, usage-zero buffers from native flags
1, upload the original 192 vertex bytes and 48 INDEX16 bytes, and retain their
logical/physical owners. Device readback matches both original payloads.
INDEX32 creation is implemented from native format66h, but this installed
fixture exercises format65h only. Native CPU slabs and renderer registries are
audited separately; these adapters have new C++ ownership interfaces.

The declaration `pssn4nubn4ussn2.mvfm` has three elements and stride16.
`pack_mesh_vertex_decode_constants_00b428c0` writes its three actual 32-byte
scale/offset records to the compiled shader's reflected VS registers85..90.
The upload is checked by device constant readback. Original vertex bytes are
never decompressed on the CPU. Per-stream iteration, absent-data identity,
ordered overlaps, DWORD wrap and stream-boundary limit checks remain as
documented in `MESH_VERTEX_DECODE_BINDING.md`.

## Material and instance inputs

The subset supplies primitive4, ranges0/12/0/8, `textured.mshd`, stream0,
`repulodestroyed.tga` at material slot0, and a17-float Lighting record.
`MaterialLighting` initializes native defaults, preserves the record bits and
ignored-slot behavior, and supplies the diffuse-alpha alias used by the
building writer. Its other12 color-like values and final scalar retain indexed
names until their semantics are established.

The installed shader selects the `building` generator. Its recovered writer
produces nine float4s: three transposed world rows, three ordered light
position/radius records, and three colors with visibility/count/alpha in W.
The combined declaration preserves TEXCOORD0 on the mesh and TEXCOORD1..9 on
stream1, with total stride160. This fixture supplies the installed identity
world, no point lights, visibility1 and installed material alpha1. It uploads
the exact144-byte writer output into explicit host instance storage. Native
per-frame instance allocation and batching remain a separate dependency.

## Original compiler and shadow route

`CompiledMaterialPass` replaces the font probe's private compiler and is shared
by font and mesh diagnostics. It executes installed Lua, combines the selected
NORMAL descriptor, generates PS first, uses actual PS disassembly liveness to
pack VS interpolators, reflects constants, and builds sampler/render states.
It uses the installed `d3dx9_40.dll` and the original compiler flags: VS1200h,
PS1400h, with the native case-sensitive `shore` name exception selecting PS0.
The engine program name retains the filename stem and appends mode,T/F,3.
The successful installed model name is `textured0F3`.

Using compiler flags0 caused the installed shader's assignments to HLSL
globals to fail. Restoring the native compatibility flags compiles the source
unchanged. Host RAII/error reporting does not reproduce native diagnostics,
failure leaks, cache/owner construction or fallback shader loading.

`textured.shfx` plus `per_vertex_shadow.shfx` has two pixel samplers. Source0
appends the material reference. Source2 advances sampler-state counters without
creating a static texture record. The special effect-name lookup selects
ShadowTexture at slot1, after the base sampler. Special configuration runs after
unused-PS-state pruning; ShadowMap filter selection is an explicit caller input.
The generic special binder distinguishes no first-light shadow owner from an
existing owner returning a null texture. Only the ShadowTexture branch is
exercised by this installed model.

The mounted VFS resolves `repulodestroyed.tga` to
`models/textures/repulodestroyed.dds`. For this controlled scene, a supplied white
ShadowTexture resolves through the same VFS to `effects/white.dds`; it represents
an unoccluded input. The game's shadow-buffer generation/global owner is not
implemented or replaced by a fake renderer service.

## Verification and remaining scope

`scripts/build.ps1` passes with MSVC Win32 `/W4 /WX`; both existing CTests pass.
The reader-only and full D3D9 probes return0. The mesh draw checks:

- Original vertex, index and instance GPU bytes.
- Three nonidentity decode records and constant-register readback.
- Combined instance declaration and frequencies40000001h/80000001h.
- Actual material/shadow texture identities at slots0/1.
- Eight indexed primitives,2,499 nonblack pixels and54 distinct colors.
- Saved render target, depth target and device state restored after the draw.

Both existing font draws also pass through the shared compiler:74 and900 visible
pixels, zero pixels outside their expected bounds, and restored state.
`local/installed_mesh/render.bmp` is the inspected raw readback. Its maximum
RGB is3/7/9: the installed DDS is dark (max66/81/90), its shader applies
`pow(rgb,2.2)`, and this diagnostic uses a linear output target and explicit
ambient/directional inputs. No game tone mapping, final display conversion or
original-game color match is claimed.

Independent review found and verified fixes for an implicit ShadowMap filter
choice and an exception path that could leave the readback surface locked.
The final build/runtime checks follow those fixes. Native evidence is preserved
in seven packet audits and `mesh_material_native_identity.json`; primary review
independently matched89 unique ranges against the installed PE.

Five stale returning-free CALL_RETURN flows were repaired after restoring all
verified continuation bytes:00b22d5c,00b4af11,00b7f76c,00b7fa1d,00b45522.
The corresponding five function definitions retain previous comments; the
verified one-byte return at00b49b30 is now a function. Reviewed names/comments
are saved in Ghidra, with before values and refreshed exports recorded in
`mesh_material_annotations.json`.

The native frame instance allocator/batcher, scene-driven constants/shadows,
native resource object/cache lifetimes, complete material executor, full
application startup and required gameplay validation remain open. Multiple
streams, point lights, ShadowMap/fallback branches, INDEX32 and malformed input
variants have source/assembly and build evidence, not blanket runtime coverage.
See `reports/mesh_material_validation.json` for exact artifacts and scope.
