# Dyn manager binding readiness at 2c25911ef

This is a read-only adoption audit of `2c25911ef30940f93ef3a56c6202198013b59b45`.
It adds no reconstructed body, binding, activation, test or gameplay credit. The
29-body source census and its builds remain the separately admitted evidence in
[final source admission](CC16_DYN_FINAL_BODY_SOURCE_ADMISSION.md). The related
accuracy ON commit `2c182d11caa173b852404e625c680a0979720fcb` is outside this
audit. No frozen accuracy input or receipt was changed.

There is **no established application producer for a SAPRadix manager**. A
complete source table would close one provider dependency, but would not make
the unused constructor or its slots an active game path. Replacing the current
scene's manager is unsafe: construction and lifetime code explicitly assume the
different `SAPBroadPhaseManager2` layout.

## Actual ownership and call paths

`game_main.cpp:1221` obtains the stable `GameNativeDynProcess` and explicitly
executes dispatch startup `CC8950`, then convex-pool startup `CC89C0`. Its `Impl`
owns the engine/profile publication cells, dispatch owners, task adapters and
profile context. Startup constructs dispatch/pool services; it does **not**
construct a replacement engine, profile or world. Those publication cells start
null and the actual game-constructor path must create their objects.

The existing construction chain is:

```text
NativeGameConstructionCalls / construct_native_game_004ddb90
  C55F50 ensure engine -> C55EA0 -> C50310 actual 9Ch profile / 48h Root node
  publish game+14 engine
  C420E0 create world -> C41AD0 actual 48Ch world
    C38070 actual E8h scene -> allocate 248h -> C36F10 SAPBroadPhaseManager2
    publish world+444 scene; scene+AC manager
  publish game+18 world; C31A40 callback owner
```

`GameNativeDynProcess` supplies `NativeDynSapRuntime::table()` to this scene
context. This is an eight-slot **host callable** table with an owning service
object, memory, calls and lifetime progress. It is not `D7A118` or `D7A13C`.

The intended simulation chain already has source implementations:

```text
GameFixedStepHost::simulate_physics_world_00c5c540
  attached GameNativeGameRuntime::simulate_physics_00875e0c
  reload actual game publication -> game+18 world
  C5C540 -> C5BB30 -> C57070
  C574C2..C574CD: scene+AC -> [manager] -> table+0C -> call slot3
```

However, `GameFixedStepHost::native_game_` is initialized null. Across current
`src`, `attach_native_game` has a definition and no invocation, and the runtime
and application-frame constructors have no application instantiation. The null
fixed-step path records the site and returns. A real application owner must
compose `NativeGameConstructionContext`, `NativeGameLifetimeContext`,
`GameNativeGameRuntime` and `GameNativeGameApplicationFrame`, execute construction,
retain their publication/lifetime services, and attach the completed runtime.
The frame requires the actual application `+14` game cell, duplicate-name
`E1AE78` cell and canonical raw-string pool. Creating only a Dyn world or
attaching an unconstructed runtime does not close that path.

These generic slots are also insufficient for replacing the concrete manager:

* `dyn_body_register_broadphase_00c50470` directly calls SAP2 `C54AA0`; its source
  explicitly excludes other manager classes.
* `dyn_body_creation.cpp::refresh_proxy` writes the SAP2 proxy representation
  before its update dispatch. The other families require their own proven raw
  handle/layout contract.
* `NativeGamePhysicsLifetimeCalls::physics_broadphase_remove` and
  `physics_delete_broadphase_004043d0` directly invoke SAP2 `C4C380`/`004043D0`.

Any alternate-manager adoption must establish compatible creation, bounds
updates, removal, pair enumeration and scalar deletion together. A vptr change
at `scene+AC`, with those concrete calls retained, can corrupt storage.

## Genuine inputs: available services and missing bindings

| Required input | Current supplier and exact limit |
| --- | --- |
| Engine/profile publication | `GameNativeDynProcess::Impl::engine_0109e9fc` / `profile_0109e9f8`; public `dynamics().engine` exposes the same pointer cells and memory. These are actual source-owner cells corresponding to native identities, not permission to dereference original mutable addresses. Require completed C55F50/C50310 publication before C5A8B0. |
| Profile tree | C50310 allocates 9Ch profile and 48h Root node, publishes after its three head words, then clears the 90h tail. Child C50390 uses the real 48h layout and SBO at +10. The process currently supplies host literal `"Root"`; this is not an image-address binding. Exact D79DC8 bytes can be borrowed through the existing data service where required. |
| Object/body lifetime | `body_creation()` returns the real convex-pool construction context after CC89C0. `NativeGamePhysicsLifetimeCalls` supplies actual source destruction composition; world/scene and engine memory services must be retained separately if distinct. These do not establish the unknown C55DF0 removed object's construction provenance. |
| CRT feature | `GameNativeCanonicalDataOwner::mutable_crt_data().feature_word()` is the actual volatile DWORD at `0109EEA4`. Main passes it into CC89C0; `crt_conversion_0109eea4()` returns that same retained reference. It is available only after startup; never replace it with a cached value or CPU feature projection. |
| Half | `GameNativeReadOnlyData::data_at(0xD7A280,8)` can provide the genuine double cell; D70000 is already in main's mapped band set. Live bytes are `000000000000e03f`. Retain the owning mapper; no host `0.5` default for the new kernels. |
| Five C5 names | Use the same verified data service: D79C80/8 `SAPSort`, D79C88/9 `SAPSweep`, D79C94/13 `SAPObjectNum`, D79CA4/19 `SAPPossiblePairNum`, D79CB8/11 `SAPPairNum`, including each NUL. Populate `RadixManagerProfileNames` with these actual borrowed pointers. No replacements are installed by this audit. |
| CRT allocation | `DynLiveCrt` requires BF681B new, BF55BE new, BF6989 free and BF9F1A malloc; there is **no process construction of this struct in current game source**. The radix owner also requires `cc16_dyn_radix_owner::Heap` / `cc16_dyn_radix_entries::Heap` and distinct BF65AC retirement. They must be bound to the allocation domain that actually produced the manager, its 24h objects and vectors. |
| Existing source heap | The default process memory uses `::operator new` / `::operator delete`; a custom first binding may replace it and cannot be rebound. Existing SAP2 services delegate to that memory. Legacy profile SBO text instead uses `singleton_lifetime_allocate/free` (malloc/new-handler/free). These are source allocator contracts, not proof of original CRT binary identity. BF65AC object deletion and BF6989 vector release remain separate contracts even when a qualified host domain implements both. |
| Raw manager and dispatch owner | No current SAPRadix producer, allocation/publication/destruction owner, complete callable table, or selected-manager policy exists. Borrowing these other services does not supply a raw manager. |

Read-only image pages are mapped non-executable. Their vtable DWORDs are native
code **tokens**, not callable host addresses. `data_at(D7A118,32)` can establish
the original table's identity and contents; passing it as the C++ binding and
calling through its words is invalid. Likewise, do not copy a `NativeDynSapRuntime`
table for a different layout or reinterpret a native code token as a C++ function.

## Why the 39-byte constructor remains partial

Live BSP proto confirms the defined body `C37210..C37236`, 13 instructions,
39 bytes; the former description “Ghidra-undefined” in CC13 comments is stale.
The original EAX receiver/result and plain RET, ECX zero, fixed D7A118 store and
ten ordered zero DWORDs +4..+28 are already recovered. There is no missing
arithmetic or allocation inside it. Its required `VtableBinding` lacks a genuine
callable provider. The receiver extent is proven only to at least 2Ch; its
original owner/allocation/selection producer is not established.

The actual table has eight slots, independently read from the live program:

| Slot | Original target | Existing source status / required transport |
| --- | --- | --- |
| 0 | C55D30 | `create_object_00c55d30`, required owner/entry heaps; native ECX + three stack words, RET0C; EAX object. |
| 1 | C40D50 | `retire_radix_object_00c40d50`, same raw manager/object, `DynLiveCrt` and distinct BF65AC; stack object, RET4. |
| 2 | 00403C40 | Real `oct18_rec::radix_noop_00403c40`, RET4, preserves original registers/flags. |
| 3 | C5A8B0 | Exact assembly-based update, required CRT/profile/feature/half/names; native ECX, no stack args/RET. Public void does not assert native incidental EAX. |
| 4 | C32C30 | `cc13_dyn1::radix_pair_count_00c32c30`, +20 count. |
| 5 | C40D70 | `cc13_dyn1::radix_first_pair_00c40d70`, +1C data if count nonzero. |
| 6 | C32C20 | `cc13_dyn1::radix_next_pair_00c32c20`, pair+8, RET4. |
| 7 | 004041F0 | Current ledger is `library_instantiation_out_of_scope`; only export exists, no callable source provider. Must qualify a real library/provider composition, not claim another game-body reconstruction. |

The DWORD at D7A138 is the next table's RTTI locator, not slot8. Slot7 assembly
releases live +1C, then +10, then +4 through BF6989, stores base-table identity
D7A0E4, conditionally frees the captured manager through BF65AC for flags&1,
and returns the manager bits with RET4. A valid provider must retain those
epochs and allocator custody. Its base table also needs a truthful identity/
callability contract; original D7A0E4 contains BF698E purecall tokens, the real
noop and base scalar target 00403C60. Do not invent RTTI or a substitute scalar
stub. If the restored word is retained only as a non-callable destroyed-object
identity, prove that consumers do not dispatch through it; otherwise bind the
real host table. Template/vendor binding remains outside new-body credit.

Current graph queries show zero callers to C37210 and C37030. C55D30 and C5A8B0
are referenced by the D7A118 data/vslot rows, not by a selected manager owner.
`--path-from-host` finds no chain within its 12-level model for C37210, C37030,
C5A8B0 or SAP slot3 C5A850. This agrees with the retained positive-controlled
raw-reference census, but does not prove absence of every runtime-computed edge.
No selector should be fabricated to turn this static limitation into activity.

## Qualified profile cleanup is not yet adopted

The real five-argument C50390 overload and four-argument C44000 overload are
present. They compose C++ partial-construction cleanup only: C44000 releases
captured current child-vector data through actual BF6989 without field clearing;
C50390 catches only constructor failure and frees the captured raw 48h allocation
through required BF65AC. Allocation and later parent growth lie outside the
outer catch; no orphan rollback or original FH3/SEH identity is implied.

Current C5A8B0 `append_bridge` still calls four-argument C50390. So do the direct
bridges in `native_dyn_collision_pass.cpp`, `native_dyn_world_step.cpp` and
`native_dyn_solver_mode1.cpp`, plus `dyn_profile_scope_enter_00c57020`.
`DynProfileScopeContext` has only memory and publication cell, with no BF65AC
member. Root C50310 still uses the normal C44000 overload. The additive cleanup
fixture and existence of overloads do not upgrade any of these callers. C5's
four real scope-leave actions remain separate; its unregistered-frame C++
transport/original FH3/SEH/fault limitations also remain.

## Smallest next packets

No packet is ready for honest **manager activation** at this baseline. The
following are bounded source steps with explicit stop conditions, not new
unused APIs presented as activation:

1. **C5 cold-profile adoption, source only.** Own
   `include/bsp/cc16_dyn_radix_manager_update.hpp` and its `.cpp`. Add a required
   genuine `DynProfilePartialNodeDelete` input, retain it in the borrowed context,
   and select the existing five-argument C50390 in `append_bridge`. Its supplier
   must be the actual 48h-node allocation owner; do not default to a null callback
   or the SBO-text free. Review emitted context offsets/call transport and the
   existing four scope cleanup actions. This is ready as a bounded explicit
   provider-contract repair; runtime adoption stays blocked on the manager owner.
2. **Existing profiler caller adoption, separately leased.** Own
   `dyn_profile_scopes.hpp/.cpp`, `game_native_dyn_process.hpp/.cpp`, then the
   three direct bridge `.cpp` files named above as one coordinated source batch.
   Require the actual BF65AC matching node memory from the process's first bind
   (including custom-memory cases), retain it through context copies, and wire
   each reached cold append explicitly. Engine Root C44000 cleanup needs its own
   original-scope review in `dyn_engine_runtime.hpp/.cpp`; do not blanket-upgrade
   C50310's enclosing exceptions from overload availability. No original ABI or
   whole-world unwind equivalence follows from this source adoption.
3. **Radix callable provider closure, blocked until slot7 is qualified.** A new
   `include/bsp/native_dyn_radix_runtime.hpp` / `src/native_dyn_radix_runtime.cpp`
   could own a stable eight-slot host table, borrowing the required raw manager,
   `DynLiveCrt`, owner/entry heaps, BF65AC, profile context, feature/half/name
   bindings and the genuine slot7/base provider. Hook C37210's actual vptr store
   through `VtableBinding`; bridge each original ECX/stack/RET contract to the
   real implementations above. This would close a source provider and preserve
   a lifetime, not prove a native selected owner. First settle the existing
   out-of-scope scalar/provider classification; no stub or vendor-body credit.
4. **Real root before switching.** First construct the missing ordinary game
   context owner and hook `GameNativeGameApplicationFrame::construct` / completed
   `GameNativeGameRuntime` into `GameFixedStepHost::attach_native_game`, using the
   actual publications, canonical strings, complete construction/lifetime
   services and process Dyn owner. This is a separate readiness/producer packet,
   not a quick manager toggle. To adopt a different manager afterward, require
   actual evidence of its native selection/owner. Coordinate
   `dyn_scene_runtime.hpp/.cpp`, `dyn_body_creation.hpp/.cpp`,
   `native_game_physics_lifetime.hpp/.cpp` and the owning process. If no genuine
   selector exists, keep the dead family as qualified source closure rather than
   changing the original game policy. SAP D7A13C additionally lacks a source
   C37030 constructor with its real pool/vector EH closure; radix C37210 does
   not construct that 1A0h family.

C320D0 is likewise not a root: its three nonrecursive callers C35390/C37B10/
C40FC0 remain metadata-only with no caller chain to a host. Its real sink,
position words, actual profile cache-slot1 denominator and D7A220 double100
must come from a recovered caller. C55DF0's only caller is separate undefined
C5B180 (call C5B183), outside C5A8B0. Preserve the removed object's unknown type
and required lifetime/body context. Ring C4D5A0 also has no graph path from a
host. None supplies the missing manager-selection producer.

## Admission gates and evidence limits

At this baseline the new Dyn entries are compiled source with no gameplay
activation; the final admission receipt records the seven recent entries and
both qualified profile overloads absent from the game map. No Dyn-manager
activation guard exists to flip. Mission ship/frame flags
do not construct this missing game/manager graph. Accuracy's independent
`kWeaponHitAccuracyNativeRangeBound` is false in this exact tree; its separate
private candidate and runtime results say nothing about Dyn activation.

Before any adopted table is admitted, require strict Win32 compile/link and
final-object call/stack/context-offset review, a focused original-machine
fixture for the genuine constructor/table/owner path, matching storage and
allocation identities/order, actual profiler publication, and mixed endpoint/
pair work through slot3. Include both masked PC24/PC53 only as explicitly
limited fixture controls and preserve the caller's FP state; never force those
controls in game code. Claimed partial-cleanup adoption needs its real allocator
failure/cleanup boundary witness. A source thunk's native stack cleanup and
callee-saved-register checks do not establish every original flags, SEH/fault,
OOM, timing, concurrency or exception-frame behavior.

Only after a proven natural owner/caller is installed should a separately
frozen guarded gameplay comparison exercise actual create/update/pair/remove/
teardown calls and completed returns, with equal inputs, process/backend/source
pins and a held-identity watchdog. Missing calls are inconclusive, not a pass.
The previous ring or accuracy fixtures/runs cannot substitute for this root.
No such fixture, game, probe, test or build was run by this audit.

Graph results were generated from this tree's fresh local index and current
source, using a copied final candidate map only for identifier resolution.
`wire_readiness.py` is a file-granular source-reference approximation, counts
behind-switch uses and omits provider/exception custody; its “wired” C38070 row
does not contradict the missing `attach_native_game` producer. Full input and
source fingerprints, graph limits, mapped table words and proposed packet
contracts are in the accompanying JSON report.
