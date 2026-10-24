# Render D: installed unit models and live native node poses

This is host composition in `GameNativeMissionModel`, under the existing
`BSP_NATIVE_MISSION_SHIP` runtime opt-in. The switch remains OFF by default.
It does not reconstruct `GGame::Render` or supply an Original binary ABI.

## Domain and caller contract

One model component borrows the application's existing VFS, strings, node
runtime, actual-owner registry, geometry pools, material process and texture
cache. Its sampler/effect services are constructed once. Additional units and
assets do not construct another Application, effect domain or preload graph.

`load_installed_unit(logical_name, unit_index)` may run in the initial ready
state or after earlier units loaded. The caller keeps its existing native
shader/resource loading bracket around each load; no batch-finish API is
required. It returns true for a new unit and false for an already completed
unit with that exact logical name. An index cannot be rebound to another
asset, and interrupted native work cannot replay.

Exact logical names cache an asset's native meshes and authored hierarchy.
Each unit receives separate actual generated-model nodes and canonical model
companions. B75170 retains the cached geometry for each node. The cache also
keeps each mesh creator until mission retirement, permitting later spawns
without reloading an existing asset or borrowing a freed geometry identity.
Name aliases are not canonicalized into one cache entry.

`unit_loaded`, `unit_count`, `unit_drawable_count` and `drawable_unit_index`
expose loading state and the stable host-unit association. New drawable
indices append; existing indices remain stable. `section_count` counts native
sections across unique cached logical assets. `load_installed_ship` preserves
the diagnostic route as unit zero in an empty domain, and `installed_name`
returns the first logical asset name.

## Pose, visibility and lifetime

The existing source hierarchy adapter computes `authored_local * parent` with
00413920 and caches the resulting asset-relative matrix. It preserves the
authored hierarchy values intrinsically to the asset; it does not read a unit
pose into that cache or create a second simulation hierarchy. Native visual
nodes remain flattened as in the earlier Fletcher diagnostic.

`update_unit_transform(unit_index, current_matrix)` composes each drawable's
immutable asset-relative matrix with the synchronously borrowed current unit
matrix, then calls the complete actual-storage B6DB10 setter. Its temporary
matrix exists only on the stack. The native setter copies local+B0, and when
flags5C has bits A set it clears bounds bits138/30 and flags5C, notifies the
current +A0/current virtual3C, then invalidates native descendants. It never
publishes a cached world matrix directly. The primary scene owner retains
responsibility for the genuine root attachment at +A4.

`set_unit_visible(unit_index, visible)` calls B6DA70 with factor 1 or 0 and
recurse=0 for every flattened drawable. The caller rereads authoritative unit
liveness/visibility each frame; this domain stores no duplicate flag or pose.
Unknown indices return false from both update APIs. Dead units can remain
resident and hidden until whole-mission retirement. There is no per-unit
unload or host-index reuse contract.

The primary first retires commands and scene associations. `retire_drawables`
then releases model creators through their current native terminal and releases
each cached mesh creator exactly once. Healthy empty domains can also retire.
Shared renderer drain precedes `after_native_renderer_drain`. An interrupted
acquisition, pose publication or retirement retains the domain for the process;
there is no speculative rollback or destructor retry.

Per-node light collection remains the earlier null-lighting diagnostic.
Original per-unit light membership, articulated turret/bone animation, damage
model switching and arbitrary non-Mesh serialized visuals are not supplied by
this asset/affine adapter. Missing VFS data, absent authored matrices and models
without supported hierarchy-to-Mesh associations fail explicitly.

## Original evidence and ABI

The batch checked `config/target.json` (`C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`) and the live Ghidra program. Existing exported
pseudocode/listings and recovered native implementations remain the evidence;
no Ghidra names, comments or body-count credit changed.

| Address | Original interface and use |
| --- | --- |
| 00413920 | ECX left; stack destination/right; RET8. Row-major product with original x87 ordering. Used for authored and live affine composition. |
| 00B6DB10 | ECX actual node; stack source; RET4. Listing B6DB10..B6DB56 confirms copy, conditional cache clearing, +A0 notification and B6DA30 descendant invalidation. |
| 00B6DA70 | ECX actual node; stack float bits/recurse DWORD; RET8. Writes +AC and optionally traverses +34/+3C. Existing raw setter is used unchanged. |
| 00B75030 | ECX model slot; stack actual name header; EAX same slot; RET4. Existing actual model construction and companions remain canonical. |
| 00B75170 | ECX model; stack unused DWORD, geometry, float178, float17C; RET10h. Publish/retain incoming geometry before releasing old geometry. The zero DWORD here is not a LOD index. |
| 00BEA150 / 00B94710 | Structured reader construction and actual mesh loading remain the earlier native-reader path. BEA150: ECX reader, RET; B94710: ECX pair, EDX unused, stack parent handle, RET4. |

These host APIs have new C++ interfaces. Evidence for the unchanged native
bodies is also recorded in `NATIVE_TRACELINE_RENDER.md`,
`GENERATED_INSTANCE_GEOMETRY.md` and `STRUCTURED_HIERARCHY_VALUES.md`.

## LOD selection and validation limits

No LOD-zero override or substitute threshold is introduced. Original
B748E0..B74B5A (ECX model; context, float LOD, visibility, flags; RET10h)
preserves supplied LOD when notification context+A0 is nonnull. When null,
B74A0F..B74AB2 computes the existing clamped screen-coverage value from the
camera view-projection, transformed sphere and context+C camera scalar+4C.
The x87 listing was inspected rather than relying on decompiler float aliases.

B72F80..B7325A (ECX mesh; context, model, LOD, visibility, flags; RET14h)
uses mesh+50 phase count and the first serialized matching inclusive min/max
range at mesh+10. Its signed section bounds are also inclusive. With zero
phases it submits all sections. The unchanged actual native render body retains
these choices, including the no-matching-phase exit. Unsupported derived
virtual targets still fail through the existing explicit service boundaries.
Recovering arbitrary original notification-context producers or derived LOD
policies requires their actual caller/owner contracts; this component invents
neither.

Validation in this worker: bounded original/source evidence inspection and
`git diff --check`. No tests were added. The primary integrator owns compilation
and runtime checks; this worker ran no build or game. Multi-asset GPU rendering,
spawn handling, OFF identity, visual parity, Original ABI and game validation
remain separate integration gates.
