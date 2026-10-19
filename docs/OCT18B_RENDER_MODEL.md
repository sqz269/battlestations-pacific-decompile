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
   command provider borrows `instance_geometry()`, `group_models()`,
   `render_services()` and submits those actual B75030 objects.
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

The ship contains 445 Subset payloads, all naming `ship.mshd`, resolved by the
native cache to `shaderfx/ship/ship.shfx`. Its first sampler is ShipMap, an ordinary
source0/Index0 reference. The current compiler already has the genuine sampler
entry-word producer; it is not necessary to invent an entry capture callback.
`prepare_sampler_entry_release` executes the single actual cached-pixel COM
release through the qualified x86 layout, and `finish_sampler_entry_release`
checks the later canonical count/slot operations before granting knowledge.
`final_states` then publishes that invocation's captured word into the fresh
shared root/mode scope before B3C018. The original B3BFD7..B3C024 assembly and
B5F9C8 constructor save were rechecked with `bsp.py show --asm`.

Read-only inspection on this turn found the installed SysWOW64 D3D9, NTDLL and
Kernel32 PE metadata exactly matching `native_sampler_entry_word_profile_20261015.inc`:
D3D9 timestamp3F41597F/size179000h, NTDLL C96C22CB/1C0000h, Kernel32
01212B3C/F0000h. All eleven recorded current-provider code spans, 672 bytes,
also match their installed DLL disk bytes exactly. This qualifies the disk
candidate, not the loaded imports/vtables/device/counters: the existing runtime
guard still checks those before publication. Source compiler mode, changed
providers or unsupported release paths retain unknown knowledge and fail at the
actual consumption boundary. See [the existing entry-word evidence](NATIVE_SAMPLER_ENTRY_WORD_CC12.md).

Healthy canonical references do not make `requires_process_retention()` true;
interrupted native/host acquisition or retirement does. The destructor separately
retains any graph lacking post-drain closure. Each drawable records its exact
companion; terminal retirement marks that record before the native slot may be
reused. An added scene-association retained reference is a separate real +04
credit and must be decremented through the actual owner after raw-root detach.
Virtual18 consumes the original logical creator only once, guarded by byte+44.

No native body was newly reconstructed or renamed by this packet. Existing
reconstructed source bodies and their ledger evidence are reused unchanged.

## Attempt04 cached-row failure follow-up

The primary's real USN04 ON attempt04 crashed before MMOD loading, after a
resource post-effect compiler sampler-scope log. The matching executable hash
was `de3c1a12bc17fe49be449e389edaca84dfe571f53c46568e9bc217427da01e68`.
Minidump exception EIP10354D54 was a read of address8 with ESI=0. Matching
executable disassembly identifies the failed instruction as `MOV EDX,[ESI+8]`
inside the cached shader path, where ESI is the null B34890 returned record.
Nearby native-site stores select B3B85F/B3BDCA. The earlier sampler-scope log
therefore does not identify the fault as an entry-word producer failure.

The existing B34890 implementation and header establish a forward-only cache
cursor; a miss never wraps or resets. An isolated cold B107F0 acquisition can
therefore violate the original shader preload request order, even if a needed
name exists earlier in the installed cache. This remains a hypothesis until the
exact missing query and cache cursor are observed; the native names/order must
be repaired, rather than replacing B34890 with a random lookup or cursor reset.

`cached_shader` now rejects a null cache record before releasing its exact query
or dereferencing row+08. The retained exception reports vertex/pixel stage,
counted key, actual record count/cursor, examined rows and the live source-mode
flags. It supplies no row, shader, fallback or compiler-null result. The existing
outer compiler marks its one-shot frame failed and preserves every prior native
effect and live name. This is an explicit Source provider-domain rejection,
distinct from the Original invalid-row access violation. Combined build and the
primary's next authorized ON run remain the validation boundary.
