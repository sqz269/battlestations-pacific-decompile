# Ordered material constant builder

`src/material_constant_builder.cpp` reconstructs the ordered body of
`00B42350` (`BSP_MaterialPass_BuildShaderConstants`) using retained typed owners.
Native inputs are ECX pass, stack entry and override, followed by RET8; the
override argument has no body read. The new C++ interfaces and checked failures
are not the original object layout or calling convention.

The builder patches the actual shared VS/PS word banks without clearing or
resizing them. Its order is dynamic sources, material parameters, bone matrices,
skin matrices, stream decode constants, world matrices, visibility/LOD, inverse
world, diffuse, ShadowTexture, ShadowMap, and point lights. Metadata, owner
bindings and counters are read at their native stages. The full system prefix
builder `00B46A70`, source factories and native object lifecycles remain separate.

## Native behavior retained

- Dynamic sources use the pass's four owned binding slots and live count. The
  renderer is captured before virtual+2C, the slot is read after that callback,
  and the source is reloaded before virtual+30. The two words immediately before
  the float banks remain opaque live DWORD references. Current compiled source
  kinds 0/2 have no dynamic sources; kinds 1/3 are still rejected explicitly.
- Parameters come from the material's actual borrowed table at native+80..FC
  and live count+100. VS then PS execute for each record, with live record/count
  reloads. VS uses the last matching reflected RegisterCount. Two matrix rows
  capture eight raw DWORDs; three/four rows copy sequentially through x87. PS
  always copies four words. Earlier effects survive a later checked failure.
- Native `_memcpy` at `00BF7680` includes a backward overlap path at `00BF7844`
  (STD/REP MOVSD/CLD). Raw copies therefore use `memmove`. A zero-byte copy does
  not require a source or destination address. Matrix overlap preserves the
  original sequential reads, stores and x87 exceptional behavior.
- Bone matrices use count+188 and pointer array+184 with stride4, refresh each
  transform's world bit2, and write three transposed rows. Skin's accepted
  animator path reads palette+190 after its type predicate; an accepted null
  palette skips fallback. The fallback reloads the register without a second FF
  gate and composes inverse-bind * bone-world * model-inverse in native order.
  Optimized animator type DWORD `010900FC` is required only for an actual
  predicate call. Its initialization wrapper at `00CD8000` was inspected as raw
  installed bytes; it was not created as a new Ghidra function.
- Decode constants read actual `LogicalVertexStream` owned compressed bytes,
  declaration and element count. Mesh GPU creation attaches parsed records;
  generated streams retain constructor-null records. Identity values are 1/0,
  per-record copies use x87 immediately, and signed limits/DWORD remaining-count
  wrap follow the complete native fragment. See
  [the decode audit](../reports/material_decode_live_owners_review.json).
- World VS and PS both use the VS row count. Unsupported row counts still
  refresh the world cache. Inverse-world uses metadata index1 and the actual
  `CameraTransform::view` cache; it runs between visibility/LOD and diffuse.
  See [transform](MATERIAL_TRANSFORM_CONSTANTS.md) and
  [scalar](MATERIAL_SCALAR_CONSTANTS.md) evidence for raw/x87 copy distinctions.
- Shadow callbacks retain the native late getter/renderer/slot schedule. The
  first light and optional shadow owner must be real. The native pass+84
  fallback is an explicit retained input, whose factory remains unported.
- Point constants borrow the same model+164 list used by instance packing and
  lifetime unlinking. VS register45 gates the branch; min(count,4) is written
  before reloading register46. Four position/radius and color pairs are the
  maximum. No PS writes or unused-tail clearing occur. See
  [point-light evidence](MATERIAL_POINT_LIGHT_CONSTANTS.md).

## Material and dispatch integration

`MaterialCloneState` now owns `MaterialParameterBindings`. Clones inherit the
actual effect/shared metadata but start with an empty parameter table. Compiling
another mode updates shared selectors without discarding registrations.
`assign_compiled_material_effect_00b19210_fragment` binds that effect and metadata,
clears registrations even for the same effect, supports null clearing, and sets
the actual parameter-dirty byte B4 to 1. Construction leaves B4 zero. Native
intrusive references and pool allocation are not reproduced by these host types.

Material-entry dispatch receives the builder's HRESULT separately from invalid
host binding errors. A failed texture COM call is remembered while later builder
stages, both constant uploads, material callback, draw and statistics continue.
The dispatcher reports the accumulated failure after those effects.

The installed-mesh probe calls this builder for both queued entries using their
actual clones, transforms, camera mode, model lifetime and GPU streams. It
poisons only the six decode registers before each call and checks their rebuilt
values. The host scene still supplies its existing controlled system constants
and explicit white ShadowTexture. Bone/skin/LOD/directional-shadow owners are
unbound in this scene and fail if selected; they are not fabricated empty owners.

## Verification and limits

The [integration audit](../reports/material_constant_builder_audit.json) records
19 Ghidra/installed-PE range matches (7,415 bytes), original names/signatures and
comment preimages, strict Win32 build and existing CTest results, worker native
fixtures, and the installed D3D9 probe. Native fixtures cover 384 parameter words,
five bone/skin scenarios, matrix/scalar/light writes, the complete decode loop,
and 21 selector scenarios plus one COM-failure continuation case. The worker
parameter and bone reports identify the library tested before the later
dispatcher-only HRESULT interface change; their numerical source was unchanged.

The probe produced 2,499 visible pixels and 54 colors with state restoration.
Its captured mesh image is very dark. This proves the exercised installed-asset
host path; no comparison against an original-game frame was performed. Exported,
reconstructed, build-tested and fixture-tested status does not establish binary
ABI compatibility, visual parity or a runnable rebuilt game.

The next bounded work is documented in
[the system-prefix plan](../reports/material_system_builder_next.json): separate
camera matrix, fog, and coupled scene-light/shadow fragments. Shared camera axes,
timer/singleton behavior, final uploads and actual owner construction remain
primary integration work. Scene lighting and shadow stay together because they
share the same captured first light.
