# Cockpit registry admission contract, BH

Addresses: none (host registration design). Native context: `00B3C800`,
`00B71A80`, `00B71990`, and `00B71FE0`; no new body or ABI credit.

This is an implementation-ready design for reserving one future association in
each existing canonical registry. It is not an implemented admission provider or
a claim that cockpit construction is ready. Worker source pin:
`58145d5526c832581d6ef11edd95f8f8db7ecdfb`, branch
`agent/orch3-cockpit-registry-admission-bh`, reviewed 2026-09-13. The separate BG
constructor review at `ff8e720f4e0f01aca0760a51d0b2fae3ae721369` supplies the native
composition boundary in `docs/NATIVE_COCKPIT_CONSTRUCTION_COMPOSITION_BG.md`,
especially lines 101-117 and 147-199. That commit was read in its worker checkout;
it is not an ancestor of this worker pin. No live Ghidra batch was needed or run.

## Current source and the failure to remove

| Source at the worker pin | Established behavior |
| --- | --- |
| `include/bsp/scene_attachment.hpp:120-145`; `src/scene_attachment.cpp:269-281` | Stable borrowed scene binding; one raw hierarchy runtime; unique transform and key; rebinding the same binding is idempotent. `push_back` can allocate. |
| `include/bsp/generated_model_lifetime.hpp:41-61`; `src/generated_model_lifetime.cpp:42-63` | Canonical lifetime dispatch vector; scene/transform identity equality and duplicate-transform rejection. Binding allocates through `push_back`; lookup does not create a companion. |
| `src/native_camera_owner.cpp:148-174` | Placement preparation binds the external scene association before the native constructor. Failure removes typed preparation; abandoned preparation forgets the binding. |
| `src/native_camera_reference.cpp:18-33`; header lines 17-34 | The stable reference binds only after the camera is live, borrows actual +04 without retaining, and cannot be destroyed before terminal retirement. |
| `src/native_node_construction.cpp:315-320`; `src/native_node_destruction.cpp:60-74` | Camera attachment key comes from the actual node address; node destruction requires the same scene runtime as the lifetime runtime. |
| `src/native_camera_reference.cpp:68-83`; `src/native_camera_owner.cpp:117-133,368-372` | Terminal deletion ends native storage and forgets the scene association; lifetime unbind follows pool return, then disposal may destroy both companions. |
| `src/native_camera_pool.cpp:26-40,149-170` | Raw-slot return uses actual Win32 section entry/exit and scalar/free-index writes. Its helpers use casts, arithmetic and memcpy; no allocator or host callback is invoked. |
| `src/scene_attachment.cpp:283-311`; `src/generated_model_lifetime.cpp:50-63,239-241` | Scene unbind requires prior detach; forget compares companion identity without reading dead backing. Lifetime unbind compares interface identity and terminates if absent. Existing unbound lookup failures are explicit. |
| `src/native_cockpit_helper_lifetime.cpp:13-17` | Helper member release finds the actual key through the existing lifetime runtime and fails when it is unbound. |

Allocating companion memory ahead of time does not remove either vector growth.
Calling `vector::reserve` alone does not protect capacity from another bind during
a native callback. The mechanism below accounts for every outstanding credit in
both admitted and existing bind paths. It changes no scene membership list,
native refcount, native storage size, or registry identity.

## Minimal API and representation

Implement the following shape separately within `SceneAttachmentRuntime` and
`GeneratedModelLifetimeRuntime`; use their existing node type for `Node` below.
These are two small nested types, not a shared template or new registry.

```cpp
class BindingAdmission final {
public:
    BindingAdmission() noexcept;                       // empty
    BindingAdmission(const BindingAdmission&) = delete;
    BindingAdmission& operator=(const BindingAdmission&) = delete;
    BindingAdmission(BindingAdmission&&) noexcept;
    BindingAdmission& operator=(BindingAdmission&&) noexcept;
    ~BindingAdmission();                              // calls cancel(), noexcept
    void cancel() noexcept;
    explicit operator bool() const noexcept;
private:
    friend class Runtime;                            // enclosing concrete class
    explicit BindingAdmission(Runtime&) noexcept;
    Runtime* runtime_{};                              // the only token state
};
[[nodiscard]] BindingAdmission reserve_binding();     // preparation; may allocate
void bind(Node&, BindingAdmission&&);                 // no reserve on success
void bind(Node&);                                     // existing API retained
```

Add `std::size_t pending_bindings_{}` beside each existing node vector. The token
contains no pointer to a node, raw allocation, vector element, pending-list entry,
or destruction callback. Its private constructor does not increment the counter;
only successful `reserve_binding()` creates a credit. Runtime classes explicitly
delete copy/move construction and assignment: a token and raw hierarchy binding
name one stable runtime address. Ordinary existing construction and bind/unbind
call sites retain their signatures and behavior. Previously implicit runtime
copying is not a supported identity-preserving operation; the implementation
build must catch any repository caller relying on it and resolve that caller
explicitly rather than copying pending credits.

Add a nonthrowing runtime destructor that terminates if `pending_bindings_ != 0`.
This detects runtime-before-token teardown while runtime storage still exists.
It does not visit bindings or implicitly detach them. The pre-existing requirement
that runtime and registered companions outlive their uses remains mandatory.
Do not add a separately allocated control block merely to permit dangling tokens.

## Accounting and transitions

For either node vector let `L = size`, `P = pending_bindings_`, `C = capacity`.
At every externally observable operation boundary, `L + P <= C`. Each active
token accounts for exactly one unit of `P`; no unowned credits exist.

| Operation | Required sequence and resulting state |
| --- | --- |
| Reserve | Check `L + P + 1` against `max_size()` without overflowing. Reserve at least that capacity if needed. Only after success increment `P`, then return the nonthrowing token. `L` is unchanged. |
| Existing bind, fresh identity | Run its existing validation first. Ensure capacity for `L + P + 1`, then append. `L += 1`, `P` unchanged. Never use a reserved unit merely because `size() < capacity()`. |
| Admitted bind, fresh identity | Verify active token belongs to this runtime and run all validation. No reserve, resize, temporary container, allocator callback, or user callback follows validation. Append the pointer within capacity; decrement `P`; empty the token. `L += 1`, `P -= 1`. |
| Scene bind, same binding already present | Preserve existing idempotence. An admitted call cancels its unused credit and returns successfully; the existing overload just returns. No extra entry. |
| Cancel | If active, decrement that runtime's `P` exactly once and clear the token. Empty-token cancel is a no-op. `L` and native state are unchanged. Never shrink capacity. |
| Move construct | Transfer the runtime pointer and empty the source; counters unchanged. |
| Move assign | Self-move is a no-op. Otherwise cancel the destination's old credit, then transfer and empty the source. Both steps are nonthrowing, including moves between runtimes of the same type. |
| Unbind / forget | Preserve existing semantics; erase only a live entry so `L` decreases. `P` is untouched. These operations do not reactivate a consumed token. |

The append proof is local: active token implies `P >= 1`, so `L < C`; the existing
`std::vector<Node*>` with its standard allocator can append one pointer without
allocation or a throwing element copy. Only then is the credit consumed. There
must be no callback/reentry point between this append and decrement. Pointer
erasure does not invalidate tokens because tokens do not address vector elements.
Future shrink/swap/clear code must preserve the invariant; this packet introduces
none. The attachment-link vector in `GeneratedModelLifetimeRuntime` is separate
and receives no credits or behavior change.

Capacity allocation failure and checked-length failure happen during preparation
or an ordinary bind. They leave `L`, `P`, existing bindings, and existing tokens
unchanged. Capacity already increased by an earlier successful reservation need
not be rolled back. When a preparation ticket reserves scene then lifetime and
the second reserve fails, the first token's destructor cancels its credit; no
native events have begun. Returning/moving the successful token cannot throw.

## Identity checks and supported no-allocation domain

A credit reserves capacity, not an as-yet-unknown camera key. The raw pool slot
does not exist at preflight time. No identity is installed until a fully live
host binding object exists at its stable address, and no provisional key is used
for lookup. Two outstanding attempts can hold credits in the supported
serialized domain; neither token is interchangeable with the other's owner by
implicit fallback. Its runtime identity is fixed until cancellation/consumption.

Scene admitted bind performs the existing raw-key/runtime validation before the
existing uniqueness scan. It preserves duplicate-binding idempotence and rejects
another binding with the same transform or actual key. It publishes
`hierarchy_runtime_` only after the successful append, as the current code does.
It must not read `node.scene` or any other uninitialized native word at admission.

Lifetime admitted bind keeps the existing scene/transform equality and duplicate
transform rejection. Additionally, this new overload requires that
`scenes.resolve_key(node.scene_attachment().pointer_key)` is that exact attachment
and rejects an existing lifetime entry with the same key. This prevents a stale
or diagnostic alias from shadowing the admitted camera in `find_actual_node`.
These stricter admission checks do not silently tighten the old overload; the
old overload retains its validation and gains only capacity accounting. The
actual camera already has the required scene association before its reference
is created. No other registry or fabricated companion satisfies these checks.

An empty/moved/consumed token, a token for another runtime, a duplicate lifetime,
an unbound/mismatched scene, or an invalid raw identity is a contract error.
Keep explicit exceptions and existing unbound lookup/terminate behavior; do not
turn them into a successful no-op. Validate before any mutation, so a rejected
admitted bind leaves its token active and all registries unchanged. The caller
can cancel it or correct the input. Check token activity and runtime identity
before reading node identities. The rvalue-reference parameter aliases the
caller's token: `std::move(token)` alone does not consume it. Only the successful
commit (or successful idempotent scene cancellation) empties it; a diagnostic
exception, including allocation failure while constructing that exception,
precedes registry mutation and leaves the credit active. The admission overload is deliberately not
`noexcept`: constructing a diagnostic exception can itself allocate. The claim
is **successful admission with valid preconditions is nonallocating**; cancellation,
token destruction and moves are unconditionally nonallocating/nonthrowing within
their lifetime contract. This does not assert that a native operation or its
provider cannot throw, or replace native unwind handling with token RAII.

Use one owning thread/serialized execution domain for both runtimes and their
tokens. Thread migration, concurrent readers/writers, and mutation from another
thread are unsupported here. Same-thread callbacks between completed registry
operations may reserve, bind, unbind, forget, or consume their own prepared
credits. Registry mutations and their validation scans are not reentrant:
identity getters must be stable, nonallocating leaf accessors, as the camera's
header lines 33-34 are. Allocator/new-handler callbacks must not reenter that
registry while its reserve is running. No vector iterator survives a completed
operation. This is not a claim of arbitrary callback or allocator reentrancy.

An ordinary reentrant bind preserves other tokens but can allocate to do so.
To promise an entire admitted native sequence has no extra host registration
allocation, every callback that adds a binding in that sequence needs its own
credit prepared beforehand. Bounded nesting reserves the complete number before
native events; unbounded/reentrant demand is outside the supported domain.
The registry never steals a token from a shared queue or borrows another attempt's
credit. Moving a token explicitly transfers responsibility, not object identity.

## Camera composition and lifetime handoff

Preflight allocates persistent owner/reference storage and obtains one scene and
one lifetime credit, with real runtime/provider bindings. A ticket may own the
two tokens but must not become the lifetime owner of a published camera. The
shared node destruction runtime still checks scene/lifetime runtime equality.

Later narrow overloads are required in the existing camera owner/reference pair:

```cpp
NativeCameraOwner(void*, std::size_t, NativeCameraEnvironment&,
                  SceneAttachmentRuntime::BindingAdmission&&);
NativeCameraReference(NativeCameraOwner&, NativeCameraCompanionDisposal,
                      GeneratedModelLifetimeRuntime::BindingAdmission&&);
```

The owner overload uses the same preparation and catch cleanup, replacing only
its scene bind with admitted bind. Validate the token's runtime before placement
preparation where practical; slot-derived identity validation necessarily follows
the actual allocation. After raw allocation, placement construction consumes the
scene credit before native camera construction. If native construction fails,
the existing camera cleanup/phase handling forgets that consumed association;
cancel the still-unused lifetime credit and end abandoned host preparation.
The native constructor caller retains its exact raw-slot-return obligation.

After camera construction succeeds, placement-construct its stable reference
with the lifetime credit before helper+0C publication. Retain the existing live
phase, positive actual count, disposal callback and current-profile checks. Those
checks are not removed by admission; valid providers must make them succeed.
Do not route the new overload through the old allocating constructor first.
Do not retain, zero, or initialize +04 to make registration work.

Callbacks during camera construction cannot borrow a lifetime interface that is
not yet bound. Existing missing-binding checks stay effective. A provider that
needs such an early borrow needs an additional proven contract; credits do not
make it valid. This packet also does not resolve arbitrary changes to helper+0C:
later operations must resolve the freshly loaded key through the canonical
lifetime runtime and require the concrete camera adapter/domain.

Once both tokens are consumed, destroying the ticket changes no bindings.
A later helper failure must leave its published camera and companions alive as
required by the BG state0 contract. Helper retirement cannot free a camera still
retained by a queue. Terminal camera release keeps the current order: native
cleanup forgets scene association, pool return ends raw-slot ownership, lifetime
unbind compares the still-live interface, then disposal may reclaim companions.
Cancel neither token as a substitute for either unbind operation.

No token inspects backing after native destruction. Scene forgetting needs the
attachment and transform companion to remain alive; lifetime unbind needs the
interface pointer until it is removed. Pool return precedes lifetime unbind, but
the concrete return provider (`native_camera_pool.cpp:149-170`) invokes only
actual Win32 critical-section entry/exit, arithmetic and free-index/count stores;
its helpers at lines 26-40 are casts/memcpy/arithmetic. It neither allocates nor
invokes a host callback. Thus the supported owning thread cannot reenter between
slot return and the immediately following lifetime unbind. Ordinary reuse after
terminal release is supported and needs no new delay or rejection policy.
Concurrent mutation and artificial hooks interposing in Win32 calls are already
outside this registry domain; they do not justify narrowing the concrete provider.
The admitted duplicate-key check still rejects a real stale alias. This packet
does not reorder native return or add a second generation/identity registry.

The parallel viewport design owns a separate persistent view mechanism. Its
reported preparation requirement is two stable view slots per attempt, with
identity bound after each successful viewport construction and before permitted
publication/borrow. A viewport may survive camera-construction failure; node
token cancellation cannot retire it. Runtime/view borrow and terminal retirement
proofs remain with that packet. Neither its view slots nor these two node credits
replace the persistent companion/provider ownership contract.

## Bounded implementation packet and acceptance

First own only `include/bsp/scene_attachment.hpp`, `src/scene_attachment.cpp`,
`include/bsp/generated_model_lifetime.hpp`, and `src/generated_model_lifetime.cpp`
plus its evidence artifacts. Implement the nested tokens, pending counters,
stable-runtime restrictions, teardown guard, shared current validation, stricter
admitted lifetime identity validation, and ordinary-bind accounting. No CMake,
ledger, Ghidra, native-body, attachment-link, or unrelated registry changes.

After integrator review, a separate small packet owns the two camera headers and
two camera sources for the overloads above. The persistent cockpit provider and
viewport lifecycle integration remain separate coordinated work. API presence
alone closes none of those remaining ownership or native-EH requirements.

This design adds no test. The implementation should run `scripts/build.ps1` and
the existing applicable checks. If existing coverage cannot establish credit
isolation, add only one focused regression scenario: reserve two credits, force
ordinary binds beyond the original spare capacity, move/cancel one credit, then
disable host allocations while consuming the survivor into the canonical
registry. Verify ordinary binds did not steal its space and lookup returns the
same object; cleanup follows the existing explicit unbind rules. Apply that
scenario to the two concrete registry types without introducing a framework.
Static review separately checks overflow, failure-before-counter-change,
move-assignment cancellation, idempotent scene behavior and rejected-token state.

Verification here is source/evidence review, JSON parse, owned-file diff checks,
and design consistency only. There are no numeric native call rows to reverify,
no newly reconstructed functions, no build or fixture run, and no original ABI,
FH3/SEH, concurrency, executable-path or game-validation claim.
