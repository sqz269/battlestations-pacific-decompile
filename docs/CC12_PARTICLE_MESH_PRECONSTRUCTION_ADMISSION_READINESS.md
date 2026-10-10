# Particle mesh preparation and canonical admission readiness

This Source-only packet establishes a **GO for one dormant mesh prepared-record helper**, confined to the existing `include/bsp/native_mesh_owner.hpp` and `src/native_mesh_owner.cpp`. It does **not** establish GO for a complete particle resource delegate or persistent application custodian. The existing registry can reserve the exact raw mesh key before construction. The particle constructor currently gives its delegate that key only after construction, so using the new helper from that constructor requires a separate caller change.

No C++, CMake, tests, probes, Native exports, Ghidra state, ledger or application wiring changed in this packet. Function names and proposed API names are Source descriptions, not recovered symbols. The evidence is existing reconstructed Source plus a finite lexical/excerpt replay; no additional Native or ABI equivalence is inferred.

## Frozen Source and accepted prerequisites

The frozen worker commit is `0555655489fee9c03b94bdadb0a0637e21ff4a3a`, including renderer helper commit `224a64d4b6f1907a12bed5d8dc4870b37ecaf5e8` and the synchronized Main metadata. The capture contains all 4,228 tracked paths under `src/`, `include/`, `cmake/`, `scripts/`, `tests/` and `CMakeLists.txt`, five additional Source-epoch metadata paths and 22 named prerequisite documents/reports: 4,255 actual Git/current-file rows. Ten retained real Git tree objects close those paths against the retained commit and recursive tree listing. Every blob object ID and current-file pin is replayed; current bytes agree with their committed blobs after CRLF normalization.

The accepted hook-adapter Root Source3356 epoch and accepted resource-readiness Source4249 epoch differ from this freeze in exactly `include/bsp/gui_text_native_renderer.hpp` and `src/gui_text_native_renderer.cpp`, the already committed saved-renderer helper. The renderer worker's actual staged Source4253 agrees with the freeze. At capture, Main's six Source roots also agreed. This preserves the older epochs as older inputs and proves the explicit two-file delta; it does not replace their bytes or inherit their build results. The Root renderer acceptance received later in the session does not silently add another Source or artifact input.

Retained named prerequisites include the registry prepared-entry contract; admitted particle-profile constructor; particle persistent record and its primary acceptance; hook-binding readiness; prepared hook adapter and primary acceptance; resource delegate/custody readiness and primary acceptance; and the saved-renderer composition. Their exact document/report paths and pins are in the report. Prior build, object, test and runtime fields remain declarations that this Source-only packet neither follows nor credits.

## The actual key and count timeline

| Boundary | Established current Source behavior |
| --- | --- |
| Canonical pool | `GameNativeResourcePoolProcess` owns the `0108FFF8` mesh pool in the shared allocator domain. Explicit startup binds and initializes it once; its getter rejects an incomplete startup. The getter is an available Source provider, not proof a particular runtime has called it. |
| Physical allocation | `NativeMeshPool::allocate_slot_00b73a10` returns the actual C0h slot. Its BCh payload and trailing slab word at +BCh are distinct. The returned raw pointer is the first exact key available to this factory caller. The allocation does not construct a mesh or establish its +04 atomic. |
| Native construction | `construct_native_mesh_00b73d70` is placement-only and `noexcept`. It restores the preserved payload preimage around placement, sets actual +04 to one, publishes D62D60 and performs only the recovered field stores. Minimum/maximum constants are read before placement and the LOD constant at its later store. It returns the same object. The documented original interface is ECX actual BCh payload, EAX same, RET. |
| Current particle hook | `create_mesh` allocates, returns null on the source null branch, calls B73D70, and only then calls `callees.bind_mesh(*result) noexcept`. There is no callback carrying the raw key between allocation and construction. The existing prepared particle hook adapter only forwards this hook. |
| Canonical mesh companion | `NativeMeshReference` borrows the same actual +04 through `RenderCommandReference`; it adds no count and performs no retain. Construction requires positive count, an explicit retirement callback and current D62D60 slots BD30E0/B74280. A bad callback/count throws; profile mismatch terminates. |
| Particle custody | Both `tail.mesh_1b4` and `tail.mesh_1c8` receive one creator reference. Publishing the first mesh into model geometry +180 separately retains it. Stream/section attachment has its own native retain/release behavior. Normal derived destruction releases mesh1C8 then mesh1B4, and base destruction releases +180. A mesh companion is metadata for this existing count, not another owner reference. |

The earlier mesh key cannot be guessed from capacity or from a future array position. Preparing an arbitrary number of unordered-map buckets is insufficient: insertion still needs a node for this exact key, and a duplicate key must be rejected. `prepare_binding(raw)` provides that specific capability. Preallocating unrelated Native meshes, substituting a new GUI mesh for the supplied mesh, copying layouts, or creating another registry would break the existing identities.

## Registry preparation is usable, with a precise failure boundary

`NativeRenderActualOwnerRegistry::prepare_binding` validates only a nonnull aligned opaque key, locks metadata and inserts that exact key with a null mapped reference. It neither reads +04 nor creates a Native object. It can throw for invalid input, mutex locking, allocation or an existing key. The returned move-only token keeps the mapped-value address; rehash does not invalidate that address. Pending entries count toward `size`, while `find` returns null and `resolve_actual` rejects them.

`publish` validates the token's registry, exact key, saved mapped-value address and still-null entry. It then checks that the supplied companion borrows this key's live positive +04, fills the existing value and consumes the token. It does not insert, rehash, retain, invoke a callback or call Native code. Invalid capability/alias/count terminates. **`publish` itself is not noexcept: mutex failure can throw.** A proposed noexcept composition must make that failure terminal; it cannot claim the provider never fails or convert it into recoverable raw-slot cleanup after a Native object exists. The accepted particle prepared record already uses this same explicit boundary.

`BindingAdmission::cancel` only erases its exact still-null entry. The token destructor and moves use that operation; an already consumed token is empty. Cancellation cannot remove a published entry and does not read or release Native storage. `unbind` removes an exact live companion match and is noexcept; mutex or contract failure is terminal there. The registry serializes its own map only. Resource access, companion lifetime and raw-slot reuse still need caller synchronization.

## Existing GUI custody does not supply preconstruction admission

`GuiNativeGeometryOwners::Impl` keeps each mesh entry at a stable `std::list` address. An entry holds the raw pointer, a separately allocated `unique_ptr<NativeMeshReference>` and a registration flag. Its environment and registration must share the same actual-owner domain. All entries must retire before the GUI domain destructor.

Its three current paths have different failure/publication semantics:

| Path | Sequence and entered failure consequences |
| --- | --- |
| Ordinary `create_mesh` | Allocate list metadata, allocate actual slot, treat a returned null as `bad_alloc`, construct, allocate companion, then call transactional `bind`. On a recoverable postconstruction host failure it releases the companion directly if present; otherwise it deletes the constructed mesh, then removes the entry. This is a convenience factory with rollback and cannot implement a noexcept binder for an already constructed supplied mesh. |
| Native B94710 prefix | Validate a fresh acquired frame, mark started, allocate list metadata, allocate/construct the actual mesh, write the caller pair even if null, then perform host companion allocation and canonical binding. The pair publication precedes host admission. A completed creator and its acquired metadata survive later host allocation/bind failure. |
| Native B4C700 mesh prefix | Validate/mark the fresh frame, allocate list metadata, use the existing B73B60/B73D70 path, then perform host companion allocation and canonical binding. It does not publish a caller pair. A returned null erases the list entry and returns null. Completed creator/metadata likewise survive later admission failure. |

In the two native prefixes, list allocation can throw after `started` becomes true but before physical allocation. Physical allocator failure erases the unconstructed host entry. The nested constructor-unwind source branch returns the raw slot and clears its acquired raw pointer; B73D70's supported C++ implementation is currently noexcept, so it does not produce a recoverable constructor exception. Once construction completes, `raw_slot` is cleared as bookkeeping, not as a release. `owner_record` is established before allocating the companion. Companion allocation or constructor validation can fail while the real creator remains live; if binding fails after companion creation, the acquired frame also retains that companion. Transactional bind leaves no registration on a throw. No generic catch may reinterpret these later failures as an unconstructed raw allocation.

Neither native GUI path calls `prepare_binding` before construction. Their public registration interface has bind/unbind/find callbacks, not an exact-key preparation capability. Existing GUI terminal cleanup can unbind and erase its own entry inside the companion callback; that does not provide a persistent particle custodian or authorize silently changing these factories' failure policy.

## Physical return precedes metadata retirement

`NativeMeshReference::release_zero_references` checks its bound phase and current profile, marks itself destroying, saves the callback, and calls `delete_native_mesh_00b74280(..., 1)`. B74280 first runs the real destructor and then returns the physical slot through the same pool. Only after that call returns does the companion mark itself retired and invoke its retirement callback. The documented deleting interface is ECX mesh, stack flags, EAX original address, RET4; that returned address may already designate returned storage.

The real destructor releases index, streams and sections through canonical owners, then handles strings and backing storage. Those releases and metadata dispatch may fail; the companion terminal method is noexcept, so failure is terminal rather than a fabricated successful retirement. If destruction does not finish, the source must not pretend the physical return or retirement callback happened. The pool return itself is noexcept and uses the live trailing slab word while the slot is still the pool's allocated slot.

After successful physical return, retirement must use a separately saved raw identity only as an opaque registry key. It must not inspect/reset the returned payload, reread +04/+BCh, manufacture a new count or release the mesh again. The caller must serialize allocations/reuse across the interval from physical return through removal of the old exact registry entry. Without that exclusion a new allocation can obtain the same address while the old entry is still present. The registry's metadata mutex alone does not protect this interval.

Stable host custody must survive the full terminal call and any existing resolved-companion borrower. The proposed record's callback should only unbind its saved key and mark the host record retired. Explicit disposal at proven quiescence can then destroy the already-retired inline companion without touching Native storage. Slot reuse exclusion through metadata retirement and host-object quiescence before disposal are related but distinct obligations.

## Smallest ready implementation packet

The next independent Source packet can append a noncopyable, nonmovable `NativeMeshPreparedRecord` to **only** `include/bsp/native_mesh_owner.hpp` and `src/native_mesh_owner.cpp`, plus its own evidence document/report. It needs no new translation unit, CMake edit, registry edit, GUI edit, pool edit, model-construction edit, Ghidra mutation or Native entry replacement. This is a readiness recommendation, not implementation in this packet.

Proposed API, intentionally new C++ rather than original ABI:

```cpp
NativeMeshPreparedRecord(void* actual_slot, std::size_t slot_bytes,
    NativeMeshEnvironment&, NativeRenderActualOwnerRegistry&);
NativeMeshStorage& construct_and_publish(NativeMeshConstants) noexcept;
bool retired() const noexcept;
void dispose_retired_at_quiescence() noexcept;
```

The record stores the saved opaque key, borrowed environment and same canonical registry, one registry admission, an initially disengaged inline `std::optional<NativeMeshReference>` and a host phase. Preparation validates nonnull alignment, the caller's exact C0h extent, same retained-owner domain and supplied actual D62D60 profile before preparing the exact key. It must not construct a typed mesh reference or read +04 before B73D70. Extent and initialized pool provenance are caller obligations; a size argument, nonnull pointer or matching table token does not authenticate actual allocation storage. The record supplies no default constants, string service, profiles, pool or registry.

Preparation and allocation of stable host record storage may throw, all before Native construction. A preparation failure or abandoned prepared record cancels only host admission; the caller still owns returning its raw allocation to the same pool. The record allocates and returns no physical slot itself. A null native-factory branch must bypass this nonnull helper and preserve that factory's existing null behavior.

`construct_and_publish` is the single transition out of prepared state: set a publishing phase, call the existing B73D70 constructor on the saved key with the supplied live constants, emplace the companion in the already stable record, publish the exact admitted registry entry, mark published and return the same mesh. In the valid domain, no Native callback, extra retain or host allocation is inserted between construction and publication. Invalid state/profile/count/capability or a mutex exception is terminal across the noexcept boundary. Closing construction and publication in one method avoids a prepared record being cancelled in an untracked gap after a separate caller already completed Native construction.

Before that method begins, destruction can cancel the admission without touching Native bytes. After it begins, normal destruction before terminal retirement and explicit disposal is a contract violation. The terminal callback validates its own companion/phase, unbinds the saved key and marks retired; it does not destroy the record inside a still-active terminal call. `dispose_retired_at_quiescence` then resets the retired optional companion and marks disposed. There is no new failed-Native-construction retirement API: this B73D70 implementation is noexcept and has no supported recoverable constructor failure. There is also no generic postpublication rollback, mesh deletion retry, automatic release, pending-state collection, shadow registry or extra creator reference.

## Named incomplete dependencies and GO boundary

| Dependency | Current result and remaining work |
| --- | --- |
| `NativeRenderActualOwnerRegistry` exact-key admission | Ready provider with the mutex/terminal-failure and caller-lifetime qualifications above. General capacity reservation is unnecessary and insufficient. |
| Native mesh constructor and companion | Ready Source providers for the narrow helper. Their real storage, constants, profiles, string service and canonical pool still must be supplied and kept alive. |
| Particle mesh call site in `native_particle_model_construction.cpp` / its callee contract header | Incomplete for preparation: the only mesh hook occurs after construction. A separate packet must expose/use the actual raw-key window while preserving null, allocator, state2/state8 and reference effects. Merely forwarding `bind_mesh` cannot perform throwing preparation before an already completed constructor. |
| Persistent mesh record custodian | Not present under the proposed spelling and not supplied by the registry. A future application owner must hold one stable record per live mesh, handle preparation failure/raw return, serialize slot reuse, prove quiescence and dispose retired host records. A stack temporary or registry entry alone cannot do this. |
| Full resource delegate and model record custodian | Still separate. Material and section admission, stream acquired-frame custody, real profiles/pools/string domains, complete partial-failure ownership and application shutdown remain named dependencies from accepted resource readiness. |
| Saved-renderer vertex helper | Present as the accepted dormant declaration/definition. It resolves that bounded composition only; it does not implement the resource delegate or establish mesh preparation. |
| Particle construction failure after mesh publication | Existing catch destroys emitter pointer storage and the base model, then retires particle metadata; it is not the full derived mesh-tail destructor. Do not assume every mesh creator has been released, or add broad cleanup here without the separate custody/failure contract. |

Thus GO is limited to the dormant, borrowed-slot mesh prepared record and its closed construction/publication transition. There is no GO for a complete application caller, full material/section/record custodian, arbitrary failure recovery, runtime startup or gameplay use.

## Replay and limits

The retained Source corpus is all 4,211 C/C++ source/header/include-fragment paths under the closed Source roots. Thirty-three named regular expressions record exact matches, including the sole current particle callee subclass (the forwarding prepared hook adapter), the current mesh-hook occurrences, three `prepare_binding` occurrences and the two saved-renderer helper declaration/definition occurrences. The two proposed mesh preparation spellings have zero matches only in this finite corpus; this is not a universal absence proof. Forty-six pinned excerpts and twenty ordered lexical checks make the key timing, failure and retirement claims reviewable.

The evidence ZIP retains the complete actual Git closure, Source/preimage bytes, accepted prior documents, exact query definitions/results and replay helpers. Guarded in-place and physically relocated replay both passed with identical results, recomputing Git closure, all epoch comparisons, corpus queries and ordered excerpts from the supplied ZIP alone. Each checked all 4,295 manifest members in the 4,296-member ZIP. After standard-library preload the guard allows exactly one read-only ZIP open and blocks other file opens, writes, processes, networking, new library loads and directory enumeration. Final hashes, execution receipts and live committed-file checks are in `reports/cc12_particle_mesh_preconstruction_admission_readiness.json` and the named local evidence directory.

This packet performs no compilation, tests, binary execution, Native/GPR queries or writes. Old build assertions are not fresh evidence. The recommended API is a Source composition contract, not an implemented or game-validated behavior, ABI-compatible replacement, success guarantee under allocation/mutex failure or completed custody solution.
