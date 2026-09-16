# Native renderer generated model

## Scope

`create_native_renderer_generated_model_00b4c700` reconstructs the complete
462-byte normal body at `00B4C700..00B4C8CE`. It composes the repository's
current model, geometry, declaration, stream, material and section providers.
It does not add another renderer, resource manager, pool, registry, type-ID
domain or reference count.

The C++ entry is a source interface. It does not reproduce the original
register ABI, FH3/SEH frames, access violations after native null allocation,
or unmasked x87 exception behavior.

## Native schedule

The direct call order is:

1. `00B74EB0` model slot allocation and `00B75030` model construction.
2. `00B73B60` mesh slot allocation and `00B73D70` mesh construction.
3. One `FLD` of `00D7A260`, followed by `FST` and `FSTP`, then `00B75170`.
4. Current renderer slot `+38` declaration creation.
5. Current renderer slot `+5C` vertex creation.
6. Declaration release, `00B73BB0`, then vertex creator release.
7. `00535320` material creation through current renderer slot `+48`.
8. When the index count is nonzero, current renderer slot `+60` index creation
   and `00B73B70` mesh publication.
9. `00533FA0` section creation, five field stores, `00B865A0`, `00B864C0`,
   and `00B73C60`.
10. Material, section and mesh creator releases, in that order.

The renderer publication is reloaded independently before slots `+38`, `+5C`
and `+60`. The current persistent material factory performs the original
`00535320` one-time renderer capture for slot `+48`.

## Current provider composition

The model uses `NativeInstanceGeometryAccess` and its existing
`prepare_model`, `retire_failed_model` and `bind_completed_model` callbacks.
The raw overload requires null `NativeModelEnvironment::actual_names`, the
same caller model-name header address, the same AA8/AA4/AA0 raw string cells,
and the caller's real `NativeNodeRawConstants`.

`GuiNativeGeometryOwners::create_native_mesh_00b73b60` is the one new narrow
provider. Existing `create_native_mesh_and_publish` models B94710 and performs
an unrelated pair publication. Existing `create_mesh` erases completed state
after added host failures. The new helper instead uses the exact B73B60 and
B72F70 static wrappers, performs only raw-slot constructor unwind, and retains
the completed creator, owner record and any companion after host bind failure.

Sections use the existing exact `create_native_section_00533fa0` acquired-frame
helper. Vertex and index creators use the direct R37 stream-registration
overload, so this body does not need a mapping or diagnostic-lock context.

Materials use one `NativeMaterialFactoryAcquired` for the whole operation.
`create_native_material_from_effect_cache_00535320` retains its persistent
cache child across failures, and `register_native_material_creator` adds the
canonical companion without a retain, native release or rollback. The obsolete
raw-material context and overload are not used.

## Ownership and failure states

The acquired frame is caller-owned and one-shot. Its destructor performs no
native cleanup. The caller must keep it and all borrowed provider contexts alive
until recorded acquisitions are explicitly resolved.

Only two constructor allocation cleanups exist:

- Model state 0 retires host preparation, then returns only the raw model slot
  through `00B748C0`.
- Mesh state 1 returns only the raw mesh slot through `00B72F70`. The completed
  model remains live and state 0 is not chained.

After native construction, a host companion allocation or bind failure leaves
the completed creator and any companion in the corresponding acquired frame.
The outer function does not roll back earlier model, mesh, declaration, stream,
material or section effects.

Before a native terminal call, source clears the acquired creator so an
exception cannot cause a retry. On success, the returned model creator remains
outstanding. If an index was created, its creator also remains outstanding after
`00B73B70`; the original body has no balancing index decrement.

## Evidence boundary

The full native body is 169 instructions and hashes to
`f0db9d4b323b45ee0e7f9e7670cd95197a538c53217aff5178e42481ec3f605a`;
fresh live memory and PE bytes agree. The focused fixture exercises the actual
mesh/section pools, canonical companion registration, append/refcount behavior,
real terminals, and injected bind failures. It links the complete generated
model translation unit but does not call the full factory, renderer, or cold
material compiler path. Game and visual behavior remain unvalidated.
