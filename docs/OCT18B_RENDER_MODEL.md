# October 18 mission model composition

`GameNativeMissionModel` composes the existing native loading and ownership
domains for the optional fixed-camera mission diagnostic. It is included once
at file scope after `game_native_renderer_application.cpp`'s namespace closes.
It adds no direct D3D calls or alternate material, geometry or texture cache.

The application owns the renderer, VFS, raw strings, canonical geometry registry,
hardware layouts, texture/effect records, compiler passes and COM providers.
This adapter retains one `GameNativeGuiSamplerApplication` and one
`GameNativeGuiEffectApplication`, exposing the latter's exact cache to resource
initialization and mesh loading. A1 added its `native_effect_cache()` metadata
borrow. The primary supplies `mission_instance_generator_context()`, the mutable
view of the application's existing validated instance-generator context.

Integration order:

1. Only inside the enabled mission diagnostic, construct this model adapter
   with the live log, renderer, VFS, singleton host, mapped data and input owner.
   Construction binds services and creates no asset loads.
2. Enter the application's normal shader-cache create/acquire/release bracket.
   Keep the cache published across resource initialization and ship loading.
3. Activate `NativeLuaServiceBindings::Activation(model.lua_services())` while
   A1 initializes B107F0 using `effect_cache()`, `material_lifetime()` and
   `model_environment()`. This same effect cache later serves native subsets.
4. Call `load_installed_ship("models/ships/us/fletcher.mmod")`. The loader itself
   activates the same Lua binding during synchronous shader descriptor reads.
5. Attach every `drawable(i)` to the primary's retained raw scene. The ordinary
   command provider borrows `instance_geometry()`, `group_models()` and
   `profile(actual)` and submits those actual B75030 objects.
6. Release the shader binary cache after all normal acquisitions finish.
7. Retire ordinary commands and scene roots, call `retire_drawables()`, drain
   native renderer/singletons, then `after_native_renderer_drain()`.

An entered failure is one-shot. Model, reader, root/child handles, name headers,
mesh/subset/texture/compiler acquisitions and canonical companions remain alive.
The facade deliberately retains its entire metadata graph on destruction until
normal closure; the parent must also retain every borrowed application provider
when `requires_process_retention()` is true. Nothing retries a failed native call
or supplies zeroes for unknown compiler stack residue.

## Loader and drawable boundary

The graphics loader uses actual VFS BDF4C0 and captured manager slot4, the existing
BEA150/BF0430 structured reader, BEA700/BEA680 raw node traversal, and the full
B94710/B944E0/B941D0 mesh/subset chain. Native material factory, effect compiler,
texture cache, stream factories, compression metadata, vertex layouts and
B85610 instance-generator finalization all use their original shared providers.
The acquired output pair preserves its leading unknown DWORD; it is not treated
as a resource index. Resource ordinals are counted separately in file order.

This bounded visual adapter selects `Mesh` resources. It explicitly skips Aux,
ConvexObject, GeomMesh, Note and other nonselected resource payloads via BE9C40.
It does not claim full B80720/game-resource loading or create simulation/physics
associations. Each selected Mesh is associated with its existing authored
hierarchy resource ordinal using `read_mmod_hierarchy_items`, the established
source hierarchy reader. Matrices are required, parent chains are checked, and
the existing 00413920 product composes absolute authored matrices. Each resulting
independent drawable is an actual canonical B75030 Model whose local matrix is
applied by the established raw B6DB10 setter. Authored local sphere words are
published at the actual model+08 and invalidate the shared world-sphere bits.
This authored-state adapter is explicit; it is not the original full model tree.

Raw post-effect/model construction and the existing B4C8D0 instance helper need
different string interfaces. Two borrowing environment aggregates share the same
pool, type cells, node runtime, actual registry and strings. The raw view keeps
`actual_names=null`; the existing instance helper requires the canonical
`ActualNativeStringPoolStorage` view. Neither aggregate owns another native
model or duplicates its reference count. Canonical native model/stream companions
are registered once and retire through their real current native terminals.

The curated `profile()` admits Model, Node, Camera, Mesh, logical vertex, generic
and building instance-generator, generator-binding and effect tables. Unknown
current profiles fail. Native model/mesh traversal already dispatches B6E8C0,
6EF890 and the known render20 bodies directly. Optimized animator type cells at
010900FC are not supplied: static B75030 models have +174=null, and any reached
animation branch must fail explicitly until its actual provider is available.

## Evidence and validation

The active configuration was rechecked: project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; live count 76774 matched the retained snapshot.
`bsp.py show` revalidated existing B75030, B94710, B941D0 and B6DB10 exports and their
reconstruction ledgers. Their original ECX/RET contracts remain documented in
`native_model_owner.hpp` and `native_mesh_subset_loading.hpp`; this new composition
is a source API and does not replace original FH3/register ABI.

Read-only installed inspection found `models/ships/us/fletcher.mmod`, 30,631,383
bytes, with 26 Mesh, 87 Aux, 11 ConvexObject, one GeomMesh and 35 Note payloads.
The first Mesh is resource ordinal zero, 3,337,130 payload bytes. Its SHA-256 is
`a6d4d1bac3d7e13f01f3ab474d5c2f445f42dfb04cb95facc8d8f7257470d211`.
This identifies a real installed ship candidate; no graphics/game run was started
by this worker.
The packet requires the integrator's Win32 build and real USN04 ordinary-command
draw/capture evidence before claiming visible textured-ship completion.

No native body was newly reconstructed or renamed by this packet. Existing
reconstructed source bodies and their ledger evidence are reused unchanged.
