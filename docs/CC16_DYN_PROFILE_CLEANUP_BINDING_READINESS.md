# Dyn profile cleanup binding readiness

This read-only audit is pinned to `605373bf732e4bd3089e8f07ddca226ab4d635a3`.
The supplier and caller sources listed in the report have no changes from
`2c25911ef30940f93ef3a56c6202198013b59b45`. Accuracy's independently admitted
math guard is ON at this new baseline; its receipts are unchanged and receive
no additional scope here. No production source, DLL, Ghidra or ledger was
changed; no build, test, fixture, probe or game was run. All body, vendor and
activation credit is zero.

The **default process allocation path has a genuine flat matching deleter**:
global unsized scalar `::operator delete(void*)`, called as a function on the
captured raw allocation. The public custom-memory path does not have a proven
flat BF65AC supplier. A qualified default-domain accessor plus explicit new
caller overloads is implementable without changing `DynProfileScopeContext` or
any existing API. It must recognize the actual default allocation tuple and
reject an unqualified custom domain; it cannot turn `memory.release` into a
flat function by casting, guessing or installing a global forwarding context.

## Current allocation and release domains

`GameNativeDynProcess`'s existing `process_memory` is exactly:

* context null;
* `allocate(void*, size_t)` calls global `::operator new(bytes)`;
* `release(void*, void*)` calls global `::operator delete(pointer)`.

The process copies the supplied `AvoidZoneDynHullMemory` into its stable owner,
engine, scene and profile contexts. Its public `dynamics().engine` exposes the
same actual memory tuple and engine/profile publication cells. The two-argument
factory permits a caller-owned allocator context and callbacks. Its first bind
wins; later calls must match context, allocate, release and CRT pointers. Thus a
default-looking name, common CRT, or a later default factory call cannot prove
the domain of an earlier custom binding.

| Consumer / native contract | Actual source memory and release |
| --- | --- |
| C57020 scope-enter cold child | `context.memory` passed to four-argument C50390. Raw 48h node and parent pointer-vector allocations use its `allocate(context, bytes)`; vector release uses its `release(context, old)`. |
| Native collision pass | Private `Context::profile_memory` points to `c.profile.memory`. Ordinary scene allocations use separate `c.memory`. `append_bridge` uses the former, not collision `allocate_00bf55be` or the scene heap. |
| Native world step | Direct `append_bridge` uses `c.owner->collision.profile.memory`. Other arrays use `collision.memory`. `enter_bridge` and `collision_bridge` dispatch through existing `NativeDynWorldStepCalls` methods and must also propagate any qualified binding. |
| Native solver mode1 | `Context::memory` is `&profile.memory`; its direct append uses that tuple. The runtime copies the scope context by value. Its supported contract requires this profile allocator to be the world's allocator; this is a precondition, not an automatic discovery. |
| C5A8B0 radix manager | Direct append uses `context.profile->memory`. Radix vectors instead use the separately supplied `DynLiveCrt`; that CRT's BF6989 callback does not prove the node's BF65AC supplier. |
| C50310 Root | `DynEngineRuntimeContext::memory.allocate` creates the 48h Root node, and engine construction also uses that context to allocate the 9Ch profile. Root currently calls normal C44000, then publishes the profile. |
| Normal profile teardown | `delete_native_dyn_profile_node_00c35400` releases the node's child vector through BF6989 and the captured raw node through BF65AC, both delegated to the supplied engine memory by the concrete default calls. Its SBO text uses the separate singleton helper. `destroy_native_dyn_engine_00c421b0` and C4DDE0 use actual engine memory, distinct from world/scene memory when contexts differ. |

Native C50390 specifically calls **BF681B** for its raw 48h allocation at
C503A9; parent-vector growth calls **BF55BE** at C503F6. The current memory
facade routes both to one caller-supplied allocate service. This is the admitted
source contract, not proof that every original allocation entry or failure mode
has been reproduced. Original vector BF6989 and raw-object BF65AC obligations
must still be named separately.

`NativeGamePhysicsLifetimeCalls::physics_free_00bf65ac` and
`NativeDynSolverMode0Calls::free_00bf65ac` are contextual member services that
delegate to `memory.release(memory.context, pointer)`. They are not flat
`DynProfilePartialNodeDelete` functions and do not independently prove the
meaning of an opaque custom allocator. The existing pointer-vector release is
likewise insufficient evidence for raw-node partial deletion.

## Actual host CRT evidence and its limits

The retained Release Win32 project explicitly uses `MultiThreadedDLL` (`/MD`).
Its final map resolves scalar new to `MSVCRT:new_scalar.obj`, unsized scalar
delete to `MSVCRT:delete_scalar.obj`, and malloc/free to the UCRT heap import
library. Installed MSVC 14.51.36231 CRT sources show scalar new's malloc and
new-handler retry, and Release scalar delete's free call. Debug delete uses
`_free_dbg`; no Debug or arbitrary module-domain equivalence follows.

These facts support the actual default **new/delete pair**. This audit does not
claim linked vendor-object byte identity, original-image CRT identity, custom
replacement-new behavior, every allocation failure, or compatibility with a
different CRT/module. Recheck the final linked functions when implementing.

The native five bytes at BF6989 are `e9 1e fc ff ff`, JMP BF65AC. This proves
that native alias, not any relationship between host callbacks with those
labels. Original code addresses are not callable host-provider pointers.

The separate `singleton_lifetime_allocate` uses `std::malloc(host_bytes)`,
`_callnewh` retry and `bad_alloc` when declined; `singleton_lifetime_free` calls
`std::free`. Profile SBO strings use that concrete pair. A flat
`singleton_lifetime_free` therefore exists for that allocation domain, but it
is not selected as the Dyn raw-node supplier merely because current Release
new/delete eventually use the same backend. The wrappers' failure behavior is
also not identical in every domain: SDK scalar new has a SIZE_MAX exception
branch that the generic helper does not reproduce.

The pending movie-owner candidate `6d4c288834c36f5ddc83da629336077389732da7`
is absent from accepted605. Its mandatory `GameMovieFactoryAllocation` has no
default implementation and requires the actual singleton allocation/free pair.
That is a relevant matched-domain model for its own allocation, not a Dyn
provider. This factual distinction was shared with the movie publication worker;
no pending source was edited or admitted here.

## Existing qualified child cleanup

The accepted five-argument C50390 takes required
`DynProfilePartialNodeDelete = void (__cdecl*)(void*)`. It captures the raw 48h
allocation before construction. Qualified C44000 first composes the real A900
current-vector cleanup; then C50390 frees that captured raw allocation through
the distinct provider and rethrows. The allocation call precedes state0. Native
state0 ends at C503D6, before parent-vector growth: a completed orphan is not
freed on later parent-growth failure, and no parent rollback is introduced.

All five examined cold-child paths still select the old four-argument C50390:
scope-enter, collision, world step, solver mode1 and C5A8B0. Their availability,
the old fixture and their existing normal teardown services do not adopt the
qualified overload automatically. The C5 four scope-leave actions remain
independent. This proposal does not expand original FH3/SEH, naked-frame,
fault, secondary-cleanup, timing or FP guarantees.

## Concrete smallest additive source proposal

First packet: **six existing files**, no new allocation policy or global.

1. In `game_native_dyn_process.hpp/.cpp`, add an accessor (proposed spelling)
   `DynProfilePartialNodeDelete profile_partial_node_delete_00bf65ac() const`.
   Under the existing startup lock/initialization contract, compare the actual
   owner memory's context, allocate and release to the existing `process_memory`
   tuple. Only an exact match returns
   `static_cast<DynProfilePartialNodeDelete>(&::operator delete)`. Any other
   tuple reports missing qualified custody; there is no default free or fallback
   through BF6989. The function pointer belongs to the same executable/CRT
   lifetime as the actual default allocator. Do not replace the process owner or
   create a second function-local singleton for a new overload.
2. In `dyn_profile_scopes.hpp/.cpp`, add a C57020 overload with the existing four
   arguments plus required `DynProfilePartialNodeDelete`. It calls the real
   five-argument C50390 on the cold path and preserves publication/cache/RDTSC
   ordering. Keep the old overload and `DynProfileScopeContext` byte layout
   unchanged: memory tuple plus profile-cell pointer, 10h on Win32. The distinct
   scope **storage** remains 14h. Do not put the provider into either layout.
3. In `cc16_dyn_radix_manager_update.hpp/.cpp`, add an overload with the existing
   six arguments plus the required matching flat provider. Its append bridge
   selects five-argument C50390. Keep the old six-argument entry normal-only.
   Extend only the private borrowed context after its existing 28h prefix;
   audit every offset and the original call-origin/unwind handling. Separate
   legacy and qualified bindings explicitly: a missing provider must never
   silently select the legacy path. No FP operation/control change is needed.

The actual supplier at a later caller is the **same** process whose
`dynamics().engine.memory` and profile cell form the scope context. Public
custom contexts, unrelated manually constructed profile contexts and another
`DynLiveCrt` must not borrow this accessor's provider. This first packet is
ready for primary source review for the recognized default domain; it does not
activate the unwired radix manager or attach a native game.

For a custom-memory process, require a genuine owner-supplied flat BF65AC with
the actual allocation-context lifetime and matching raw-allocation contract.
There is no such producer in the accepted game call sites. If its actual owner
cannot supply a flat entry, a separately reviewed context-bearing deletion
binding/overload is required; a captureless thunk with a new global context,
member-function cast or guessed `memory.release` is not a substitute. Existing
normal-only custom-memory APIs can remain valid without being promoted.

## Subsequent caller batches and stop conditions

The next coordinated batch owns `native_dyn_collision_pass.hpp/.cpp`,
`native_dyn_world_step.hpp/.cpp`, `native_dyn_solver_mode1.hpp/.cpp` and the
process owner files. Add explicitly qualified public overloads/owner construction
with the same required flat provider; preserve the old APIs and scope context.
Carry it in private contexts and the solver runtime for their full lifetimes.
Current private prefixes are collision 14h, world-step 10h, solver 14h and C5
28h; retain the existing published field offsets and review any added selector/
provider tail against emitted code.

World-step qualification must reach all three routes: its direct append bridge,
its existing `enter_profile_00c57020` dispatch and its `collision_pass_00c57070`
dispatch. Add a truthful qualified calls interface/owner implementation or
explicitly retain a normal-only nested path; do not bypass or silently discard
existing caller overrides. The solver's task table must use the qualified
execute path and retain its copied profile context/provider. Do not automatically
switch a public custom process or claim full adoption from one changed bridge.

### Root C50310 is a separate scope

Read-only original assembly/EH inspection establishes:

* raw 48h BF681B allocation at C50328, captured before state0 at C50335;
* state0 covers C44000, profile head stores, publication at C50367 and BF79F0 at
  C5036D, with no state reset before normal epilogue;
* handler CC836E -> FuncInfo E041D0, maxState1; map E041C8 is
  `{previous=-1, action=CC8363}`;
* CC8363 reads the captured allocation at `[EBP-10h]`, calls BF65AC, and returns
  without clearing profile fields or undoing publication.

Therefore Root cannot be qualified by substituting C50390 or adding only a
constructor-duration outer catch. Its separate packet owns
`dyn_engine_runtime.hpp/.cpp`: an additive Root overload with actual matching
deleter must compose qualified C44000 and retain Root's captured-allocation
scope through the evidenced publication/tail operation. If a supported C++
exception occurs there, do not clear publication/head words or free/rewrite a
different reloaded node. Current source uses `std::memset`; equivalence to the
native BF79F0/feature-cell/fault/SEH boundary is separate. A C++ catch cannot
claim to reproduce native SEH simply by having the same lexical extent.

Propagating that Root overload through C55EA0/C55F50 also requires separately
reviewing their allocation scopes and the actual construction caller; the
minimal child packet does not change them. Default `Root` is short SBO text,
which does not create a natural failing SBO allocation witness. Preserve the
actual input instead of manufacturing a longer root name to claim activation.

Before source acceptance, use strict Win32 compilation and final call/context/
object inspection plus proportionate existing qualified-cleanup evidence. Any
new focused native fixture must prove a concrete uncovered binding risk,
preserve FP/CRT state and remain fixture proof. Original ABI/FH/SEH/fault and
enclosing unwind qualifications remain. Runtime admission additionally needs
the real game/profile/manager producer and its separately frozen evidence;
neither this plan nor a flat pointer is activation.
