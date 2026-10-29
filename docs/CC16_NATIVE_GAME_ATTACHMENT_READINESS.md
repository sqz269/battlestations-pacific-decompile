# Native Game application attachment readiness (CC16)

This read-only audit is pinned to `605373bf732e4bd3089e8f07ddca226ab4d635a3`.
It follows the root prerequisite identified by
`3dea1fe67775660434911aaf4449ec716ffe9c83`, rather than treating an unused Dyn
method table as a selected game owner. **There is no ready activation switch.**
The real Game allocation/construction wrappers exist, but their application
composition, several mutable authorities, child dispatch admission, and full
normal shutdown are not closed. No source, guard, accuracy receipt, shared
metadata, Ghidra program, installation, save or profile was changed. New body,
vendor, original ABI and activation credit are all zero; no build, test, probe,
game or DLL call was run. Names below are source hypotheses unless stated otherwise.

## Present root and the missing edge

`GameStartupHost` already owns the genuine Source `1Ch` Application storage,
`native_application_[7]`. With `kApplicationCc15Bound=true`,
`application_construct()` calls `construct_native_application_00737970` using
the canonical `GameNativeStringProcess::application_00e1ae90()` and the shared
`01090AA0` manager. The constructor explicitly initializes its actual `+14h`
cell to null. It is not necessary to invent a second Application. The separate
`ApplicationFrameState` remains a projection; its bytes are not this storage.

A complete source-tree symbol search finds `GameNativeGameRuntime` and
`GameNativeGameApplicationFrame` only in their definitions/declarations, plus
the fixed-step pointer consumer. There is no application construction of either
wrapper and no call to `GameFixedStepHost::attach_native_game`. The pointer
starts null. At `simulate_physics_world_00c5c540`, that null path records
`FixedStepFanout::simulate_physics_world` at `00875E0C` and returns. Existing
`kGatedFanoutBodiesBound=true` and `kWorldActiveByteBound=true` do not establish
an attached raw Game: the latter controls the projected mission World byte.

`GameNativeGameApplicationFrame` is a real preparation dependency, not a
replacement class. It allocates exactly `71A0h` through the canonical object
allocator, zeroes that whole allocation, constructs the actual temporary raw
string, calls the full `004DDB90` storage constructor, publishes Application+14
after return, and releases the temporary through the same raw string domain.
`GameNativeGameRuntime` owns persistent construction/lifetime operations and a
one-slot callable primary table; it borrows the Game allocation. The process
owns the report callback table. This table closes scalar deletion, not the
other methods of the original Game table.

The runtime checks matching Game/grid publication addresses, the same Dyn
engine/world contexts, and the lifetime physics call service. Other provider
admission remains a composition precondition. A non-null `E188A8` cell alone
is insufficient: `004DDB90` publishes it **before** engine/world construction.
Attach only after the frame's completed constructor and runtime `live` phase,
with Application+14, canonical Game publication and runtime storage identical.
Each physics entry then reloads the actual Game publication and Game+18.

## Native epochs checked

All live queries used the read-only `bsp.py ghidra` wrapper, whose client verifies
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32, base `00400000`
against `config/target.json`. The saved exports were read in bounded pages;
register/stack-sensitive windows were checked in assembly. This is static
saved-target evidence, not execution or new body recovery.

| Native window | Required order / qualification |
| --- | --- |
| `0073D933..0073D94A` | ECX receives the actual initialization `mode` pointer; `00438E40` is called at `73D93C`; its EAX is stored to E1AE78 at `73D945`, then command-line parsing runs. Current startup only parses `mode`. |
| `0073E150..0073E1D9` | Allocate 71A0h at `73E155`, zero at `73E163`, capture E1AE78 at `73E17E`, construct raw name at `73E188`, call `004DDB90` at `73E1A1`, publish Application+14 at `73E1AF`, then release the temporary name. This is a fragment inside `0073D410`, not a separate native function. |
| `004DDB90..004DE266` | ECX actual Game, stack raw 8h name header, EAX Game, `RET4` at `4DE264`. Game+14 is the real Dyn engine; Game+18 is the real Dyn World; the callback owner is Game+1C. |
| `00C31A40..00C31A49` | ECX World, stack callback owner, store to World+24 at `C31A44`, `RET4` at `C31A47`. No replacement callback or independent report vector. |
| `0073E45C..0073E477` | Capture duplicate E1AE78; free through BF6989 at `73E466`; clear at `73E46E`; only then call `004E5540` on Application+14 at `73E477`. The duplicate's bytes do not have process-long ownership. |
| `00737FB6..00737FCD` | Call Game OnDestroy `004DC5C0` at `737FB9`; reload Application+14; call current primary slot0 with flags1 at `737FCB`; clear Application+14 **after** that call at `737FCD`. |
| `004DE270..004DE28D` | ECX Game, stack flags, call `004DCF90`; flags bit0 selects BF6989; EAX original Game; `RET4` at `4DE28B`. The frame must not free that storage a second time. |

The existing `duplicate_native_string_00438e40` provides the whole raw ECX/RET0
helper in the current canonical allocation domain. Reusing it closes no new
body. A future producer must retain the mutable E1AE78 authority and its actual
`mode` input across the epochs above, including interrupted initialization;
neither an unrelated command line nor an empty/default string is equivalent.

## Actual suppliers and remaining composition

| Required input | Genuine current supplier | Readiness at this baseline |
| --- | --- | --- |
| Application+14 | `GameStartupHost::native_application_[7]`, word5, initialized by `native_render_service_base.cpp` | Storage is live; no Game frame binding or publication is installed. Expose this exact cell. |
| E188A8 / settings | `GameNativeSettingsProcess::game_00e188a8()`, actual BCh settings and `profile_context()` | Available and already process retained. Share this exact Game cell between construction and lifetime. |
| Strings / allocator manager | `GameVfsHost::borrow_raw_services()`, `raw_strings()`, `GameNativeStringProcess` | Available canonical AA0/AA8/AA4 domain. Frame temporary, Game fields, profile, resources and teardown must share it. NativeData's image addresses are not mutable publication storage. |
| Raw profile at Game+650 | `GameNativePlayerProfileContext(vfs, settings_process, data)` | Concrete retained provider exists, definitions-only. It binds actual settings, live SDK/profile inputs and CE3A0C/CEF794/CEF15C. Teardown still needs a matching `NativeGameProfileLifetimeContext` with the Game call service. The projected menu profile is not this F8h storage. |
| Constructor/table constants | `GameNativeReadOnlyData::data_at()` and required reference bundles in `native_game_construction.hpp`, `native_game_embedded_state.hpp`, `native_game_array_elements.hpp` | Provider exists for adopted original data; bind the exact referenced words/floats and retain it. Do not synthesize a constant map or CPU/FP default. |
| Units/ranks | `GameNativeGameTables` -> `NativeGameTablesContext` | Real 23x16h unit rows/count and 12x97 ranks exist as an uninstantiated owner. Require its explicit `unit_frame_word_preimage` (upper24 bits untouched natively), borrow once while count0. No guessed zero. |
| Resource parsers / cleanup | `GameVfsHost::raw_game_resource_parsers_context()` and `raw_resource_manager_context()` | Available real manager and five parser publications in `GameNativeResourceApplication`; must remain through Game destruction and raw manager drain. Do not register projected parser objects. |
| Grid/renderer | `GameNativeRendererApplication::game_grid_context()` | Available actual graphics/strings/CPU services and mapped constants. Constructor requires three distinct `NativeGameGridDescriptorPreimage` pairs, words20/24; no producer is bound by startup. Raw grids store numeric CFD484, so generic current-table dispatch is not automatically admissible. |
| GlobalConfig + Lua | `GlobalConfigContext`, `NativeGlobalConfigLoadContext`; existing Lua services and settings FOV accessor | Bodies exist, but no application creates either context. Bind canonical F878E4, shared manager/strings, real effects, `lua_services_->bootstrap()`, `lua_services_->binding().files()` and `settings_host_->fov_divisor_00f889b4()`. Activate the same Lua binding around execution. Install the SAME GlobalConfig context in `native_deletion_bindings().global_configuration` before registration. |
| GlobalConfig sound | `sound_->core.samples()`, `sample_cache_context()`, `core.lifetime_bindings().global_00f8bbe8`; `NativeGlobalConfigSampleDispatch` | Actual cache cell, acquisition and component lifetime exist. Compose these, not a separate cache or raw call through D5B074. Source sample stop-slot admission is restricted; the dispatch header's admitted union/nonthrowing contract remains required. No existing application GlobalConfig sound context was found. |
| Dyn engine / World / profile | Canonical `GameNativeDynProcess::dynamics()`, `world_step()`, `contact_reports()`, `body_creation()` | Process is real and initialized at CC8950/CC89C0 from main, with actual CRT cell and shared E188B4 domain. It does not construct engine, profiler or World. Real constructor calls C55F50/C420E0 do that. Borrow matching contexts through shutdown and CRT callbacks. |
| Game child lifetime | `NativeGameCurrentVirtualCalls` and `NativeGameLifetimeContext` subcontexts | Concrete forwarding glue exists, definitions-only. It creates no tables/children. Numeric original profiles are not callable source tables; each reached raw/Source family and allocator must be admitted explicitly. No application context aggregate or adoption exists. |

The construction context also requires actual movie E18D48 and three grid
publication cells. Lifetime additionally requires E18678/E1867C/E19900/E18DB0/
E19698/E1930C/E198BC, E18E6C/E18D80/F89B34 singleton cells, class registry
E187BC plus six actual 10h vectors at E1875C, embedded flag E0AF14, and actual
Lua 10h headers at 0108FF30/40. No matching canonical application suppliers are
composed by current `game*.cpp` code. The Lua process currently exposes its real
**0Ch** x360comp/region object; that is not the two required 10h
headers. Do not reinterpret a `std::vector`, menu host or empty local cell as an
unrecovered native owner. A new sole authority requires original initialization
and producer/lifetime proof before it is eligible for attachment.

Lifetime subcontexts must all borrow the SAME `NativeGameLifetimeCalls` object:
arrays with the actual observer lifetime (`singletons_->observers().lifetime()`),
embedded, profile, singleton, Lua globals, classes and physics. The implementation
checks these identities on reached calls. Physics must use the same engine/
scene memory domains and `GameNativeDynProcess::body_creation()` for convex
retirement. BF65AC, BF6989, and SBO-text cleanup are distinct contracts; allocator
defaults cannot silently accept foreign allocations.

## Ordinary SAP2 remains the policy

The proven ordinary chain remains:

`004DDB90` -> `C55F50` engine / `C420E0` World (`48Ch`) -> `C41AD0` ->
`C38070` scene (`E8h`) -> allocate **248h** / `C36F10` **SAPBroadPhaseManager2**
-> World+444 scene / scene+AC manager. The live listing has `PUSH248h` at
`C38164`, allocation BF681B at `C38189`, and `C36F10` call at `C3819F`.
`GameNativeDynProcess` supplies its actual `NativeDynSapRuntime` table.

Attached fixed-step physics would follow `C5C540` -> `C5BB30` -> `C57070` ->
scene+AC/current manager slot+0C; the existing source preserves that sequence.
Body registration `C50470` directly creates SAP2 proxies through `C54AA0`.
`NativeGamePhysicsLifetimeCalls` removes proxies through `C4C380` and deletes
the manager through `004043D0`. These are not generic SAPRadix adapters.
`dyn_world_create_body_00c5d580` requires the actual World, complete descriptor
and shape vector, retained hull data and `DynBodyCreationContext`. Current
`game*.cpp` code supplies only the process body context, not a call creating
mission bodies on the new Game's World. Empty-world stepping would not prove
body/collision reachability. Preserve unused SAP/SAPRadix families as such;
their source completion is not a reason to change the original class policy.

## Separate parent/body obligations

1. **Full OnDestroy is not implemented by its named Lua fragment.** The complete
   `004DC5C0..004DC69A` native body is 219 bytes/67 instruction starts. It sets
   Game+5D4 to10h; calls `46F3E0`, `4DA780`, `4DA650`; conditionally calls current
   F8A2FC+8 then `992AC0`; closes the borrowed Lua owner, releases the machine,
   deletes/clears Game+1A08/+21A0/+21D8 and E18678/E1867C; then tail-jumps to
   `4A9AC0` at `4DC696`. Its eight direct callees and six indirect call sites
   are visible in the complete listing. Existing `mission_lua_teardown_004dc5c0`
   is explicitly **steps7–9 only**, not this whole parent. The root must recover/
   compose the full raw parent and its genuine child services before ordinary
   shutdown is claimed. Runtime `scalar_delete` calls the storage destructor;
   it does not add this preceding OnDestroy phase.
2. **Mission World is a separate owner.** `004DE610` has a full raw parent in
   `oct18_tail_a.cpp`, but its `WorldConstructServices` remains a required
   original-target/register/stack/image-cell provider, with no application
   implementation. `world_construct.cpp` still returns a projection with
   sentinel value1. Real `004C3080` sentinel and parent-header helpers exist;
   they do not by themselves produce the full actual 4BCh World, its callable
   callbacks/array cleanup and normal `00904C40` lifetime. That World is
   Game+19CC, distinct from the 48Ch Dyn World at Game+18.
3. **Entry/scene callbacks are not closed by the scalar table.** `004E5540` is
   recorded as an existing partial interface; `004DD5B0` has required-callee
   source contracts. Current menu/Lua/mission frames use projected Game state,
   profile and World owners. They are not direct raw Game methods merely
   because their logs carry original addresses. Full SceneLoad `004DFB70` and
   raw `004DE610`/entity-to-physics producer admission must remain separate.
4. **Failure qualifications remain.** Normal storage source and diagnostic
   retention are not original FH3/SEH/fault transport. Dyn reached profiling
   still uses the old normal-only C50390/C44000 interfaces; the distinct BF65AC
   cleanup overloads were not adopted by the process/step bridges. Existing
   source availability or a recent independent fixture does not upgrade them.

## Smallest genuine owner/attach packet

This is a future ownership proposal, **blocked until the providers above are
closed**, not a new unused interface counted as activation. Use a retained
`GameNativeGameApplication` aggregate (proposed name) in new
`include/bsp/game_native_game_application.hpp` / `src/game_native_game_application.cpp`.
It would own the complete contexts, actual canonical mutable authorities,
profile/table providers, concrete admitted lifetime service, runtime and frame;
borrow existing VFS/settings/Lua/resource/renderer/sound/Dyn/NativeData owners;
and retain all operation frames through failure or normal retirement. It must
reuse `GameNativeGameApplicationFrame`, not build a minimal substitute Game.

Required future leases/hook points are:

* `game_hosts.hpp/.cpp`: retain the aggregate before entry; alias actual
  `native_application_[5]`; restore the original mode-duplicate/publication epoch
  around current line2597; construct immediately after the native73E14B
  diagnostic checkpoint/current line3047, at the73E150 position; release/clear
  E1AE78 at73E466/46E; preserve the real Game entry schedule. If canonical
  publication access belongs in an existing process owner, coordinate that
  separately rather than making another cell.
* `game_hosts_mission.hpp/.cpp`, `game_hosts_mission_frame.hpp/.cpp`: forward the
  SAME retained runtime to each newly created mission frame, following the
  existing observer-binding pattern at mission lines2304,2667. Startup can use
  the existing `menu_->mission()` getter after menu construction (before the
  current skirmish/difficulty setters), so no menu-file edit is required for
  the present sole mission-host construction site.
  Add a forwarding borrow method to `GameMissionFrameHost` so its actual
  `Impl::fixed_step` (created line289) calls the existing `attach_native_game`.
  The mission's retained borrow must be applied on every frame replacement.
  Do not attach to a discarded fixed-step instance. Re-audit ownership if a
  future menu path starts constructing or replacing another mission host.
* Shutdown root in `game_hosts.cpp`: quiesce Lua extra-step/job/frame borrowers,
  detach before lifetime entry, call the genuine full OnDestroy at737FB9,
  then scalar-delete(flags1) at737FCB, then clear the actual Application+14 at
  `737FCD`. Keep contexts, callbacks, graphics, Lua, SDK/sound resources and raw
  manager alive until Game destruction returns. Follow with current Application
  retirement and shared manager drain; retain registered bindings through that
  drain. The frame's scalar call does not clear Application+14 itself.

Only introduce a default-OFF admission guard once these actual inputs and
normal shutdown are concrete. There is no current guard to flip. At attachment,
require completed one-shot construction and all alias/lifetime/table contracts;
on any interrupted construction, callback, Lua or teardown, preserve the entire
dependent aggregate and prohibit replay. Existing diagnostic acknowledgement
methods do not perform cleanup. An explicit root retention/exit route must run
before terminating destructors or raw-manager drain; no fabricated rollback or
returning callback may make a partial graph appear complete.

After source closure, the primary owns build/ABI/static review and matched OFF/
ON gates with actual construction/publication, fixed-step/native physics return,
body/proxy/collision and completed shutdown witnesses. No such gates or runtime
acceptance are claimed here. The accepted accuracy605 cohort is untouched.
