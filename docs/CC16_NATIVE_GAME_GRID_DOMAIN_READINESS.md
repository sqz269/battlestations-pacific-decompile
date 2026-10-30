# Raw grid retirement: actual release domains

Baseline: published Main `9e360e71494881f48b7944862b7777f69ef5ac9d`.
This read-only refinement does **not** approve the earlier three-record selector.
**Original-owner membership is neither necessary nor sufficient for valid grid
retirement.** Actual release-service compatibility matters. The default CPU and
outer frees share the canonical Source CRT domain, but terminal stream release
requires the correct canonical companions and their live retained contexts.
Arbitrary genuine renderer contexts are not proven interchangeable.

The existing 70B280 body is available. Full current-publication producer/domain
closure remains unproved, so no C++ selection is approved or added. Only this
document and its report change; no Source, registry, table patch, allocation,
provider, policy, compile, test, executable, game, DLL, Ghidra, CMake, ledger,
guard or peer action occurs. New native/vendor/ABI/activation credit is zero.

## Exact services reached by 70B280

`delete_native_game_grid_0070b280(grid,flags,c)` invokes 70B220, then calls
`c.cpu.free_00bf65ac(grid)` iff low flags byte bit0 is set. It returns historical
owner pointer bits. The helper does not consult a constructor operation or
original-owner list and does not read the original descriptor/preimage words.

70B220 writes CFD484, releases six CPU arrays through `c.cpu`, then releases
current vertex+78 and current index+7C. Its only grid-context capabilities are:

* `c.cpu.free_00bf6989` for each nonnull current pointer at+08/+0C/+10/+14/+18/+1C;
* `c.graphics.streams.geometry.actual_owners()` for each nonnull stream release;
* `c.cpu.free_00bf65ac` for the outer 84h allocation when flags bit0 requests it.

It does not directly use `c.strings`, constants, declarations, mapping/device
creation services, or the grid context's vertex/index construction contexts.
Those are construction requirements. Whole `NativeGameGridContext` pointer
identity is therefore stronger than this destructor's actual dependency set.

| Reached action | Concrete provider and domain |
| --- | --- |
| Six CPU buffer frees | Default `NativeGameGridCpuCalls::free_00bf6989` calls `std::free`. The real grid constructor CPU default allocates via canonical `singleton_lifetime_allocate` (malloc/new-handler). These default frees do not use a renderer/device identity. All frees precede the final six-field zero pass. |
| Outer grid free | Default BF65AC calls `singleton_lifetime_free`, which calls `std::free`. The parent default allocates84h via that same canonical object domain. The raw constructor itself accepts caller storage; it does not authenticate its heap origin. Custom allocation/free overrides must actually match. |
| Stream reference decrement | `release_native_render_actual_owner` decrements the actual stream+4 atomic first. A nonzero result performs no terminal lookup or COM call. |
| Stream terminal at zero | The supplied actual-owner resolver must return the SAME live canonical companion borrowing that stream+4 atomic. `NativeRenderActualOwnerRegistry::resolve_actual` is a concrete per-registry lookup; missing identity throws. This is not a process-global COM Release substitute. |
| Logical vertex terminal | `NativeLogicalVertexReference` retains its producing `NativeLogicalVertexOwnerContext`. It invokes real B4BF10, which unregisters dynamic physical/renderer entries, releases declaration/physical/base references, frees any base CPU storage, and returns the logical slot to that context's actual vertex pool. |
| Logical index terminal | `NativeLogicalIndexReference` retains its producing creation/lifetime context. It invokes real B4C1F0: unregister from actual renderer/physical lists, release current physical owner, and return the logical slot to the actual index pool. |
| Declaration terminal, if reached | Canonical declaration companions retain their real declaration pool/type-size/profile inputs and invoke existing B48CA0. The geometry producer checks a reused companion's matching terminal context; this is not an arbitrary callback or CRT-only free. |
| Physical buffer terminal, if reached | Physical destructors use actual resource-support/lifetime services, release the current stored COM object at+28 through its own Release slot, then destroy/free the native pointer-array backing. Private physical owners use canonical free; pooled physical owners return to their captured actual index/vertex pool. |
| Companion retirement | The real geometry disposal path unbinds the exact registry identity/companion and erases its metadata entry. The registry requires one live companion per identity and must remain alive through terminal callbacks. |

The raw COM Release action itself takes the actual COM object, with no separate
device argument. That fact does not remove the surrounding renderer-list,
resource-support, pooled-storage and canonical-companion obligations. Natural
grid construction creates a dynamic logical vertex (flags 1000, sharing the
renderer+1974 physical owner) and a private physical index through logical index
flags 1. Both source streams receive canonical companions in the geometry
producer. Other remaining references can defer their terminal callbacks; all
reached terminal actions still use the companion's retained services.

## What makes two release contexts equivalent

For a genuine live grid, a different context view can invoke the same helper if
its CPU/outer frees match the actual allocation domain, and its actual-owner
resolver reaches the correct live canonical companions for every stream that
reaches zero. The terminal then uses the companion's **stored producing
context**, not the other fields in the newly supplied grid context. Exact grid
context address and original constructor index are not required by the body.

This equivalence is conditional on the current fields and reached branches:

* With both stream fields null, no stream resolver/terminal is used.
* With nonnull streams whose decrements stay nonzero, the release mutates their
  actual atomics but does not resolve a terminal companion.
* When either decrement reaches zero, a resolver capable of that actual identity
  and a live correctly bound companion is required. A wrong registry cannot be
  justified by the COM object's device or the common malloc/free domain.

The concrete `GameNativeRendererApplication::Impl` owns its own
`NativeRenderActualOwnerRegistry owners`. `GameGridGraph` obtains that SAME
registry through its geometry service. Nothing inspected makes separate Impl
registries a shared process-wide directory. Their mere existence, matching
device, numeric profiles or common CRT does not prove cross-registry lookup.
Conversely, genuinely equivalent resolver views over the same canonical
companions can be valid without being the same C++ context object.

The current registry enforces actual+4 aliasing, positive admission counts and
one live companion per key. It is a real release provider, not automatic proof
that arbitrary bytes are a genuine stream or that the outer grid belongs to a
heap. Its genuine geometry producer creates companions around real source
stream creation, retaining the exact contexts and disposal paths. Both the
**caller precondition** (actual live matching-domain receiver) and the
**concrete provider** (correct canonical resolver/companions/free services) are
needed. Neither can be inferred from CFD484 or constructor-operation fields.

Do not discover compatibility by attempting a release. CPU buffers are freed
first; the stream atomic is decremented before registry lookup. A wrong resolver
can therefore fail after irreversible effects and leave an already-zero stream
obligation and stale fields. Trying another context would repeat effects.

## Actual current publishers and replacements

The bounded current Source inventory finds exactly the already established
explicit grid publications: 4DDB90 stores its three returned values into
E19B0C/08/04 after default constructor sites 4DDFF0/4DE029/4DE062. Its default
route invokes real 70BD70 with `c.grids`; the real renderer geometry registers
the created streams in its canonical registry. The two explicit CFD484 writers
are the constructor and destructor in `native_game_grid.cpp`.

The explicit clears are 4DCF90's 4DD151/169/181. It captures C before clearing
E188A8; calls with captured receiver/slot0/flags 1; clears CURRENT C after return;
then reloads current08 and04 at their own turns. Null capture skips call and
clear. Callback replacement and the native current-cell clear order remain
part of the contract. Other literal E19B0C/08/04 matches are context identity
checks/declarations and read-only `global_u32` consumers.

This verifies known explicit Source writers; it does not prove that borrowed
mutable references, callbacks, uninspected native code or future genuine
producers cannot replace a cell. It does not establish a canonical application
owner for all three publications or prove that every current replacement's
streams resolve through one renderer's registry. The existing raw constructor
is public and can operate on other valid supplied storage/context; its body
does not constrain all instances to the original three Game calls.

## Address reuse corrects the earlier selection proposal

The three completed constructor records are useful observations of their
original calls. They do not authenticate a current allocation generation.
Likewise, a retirement row saying an address was consumed describes that old
call; it cannot veto a genuinely new matching-family grid later constructed
at the same reused address and published for a later native load.

After genuine terminal retirement, existing companion disposal unbinds the
stream identity. A later real producer can bind a new live companion at a reused
identity under the source synchronization contract. There is no basis for
turning old raw pointer bits into a permanent ban on new objects. Concurrent
reuse before retirement/unbinding completes is outside that contract.

Therefore the recorded-three-owner selector is not complete family admission:
membership is not necessary for a valid new replacement and is not sufficient
to prove that an old address still denotes its original live owner. No new grid
registry, allocation, callback wrapper, table patch or rule forbidding valid
replacements is proposed to conceal this issue.

## Precise remaining gate

At each actual 4DD14F/167/17F scalar call, the real selected service must be valid
for the CURRENT captured grid's live storage generation, its requested outer
free, six current CPU allocations, and every reached stream terminal's actual
canonical companion/context. That can be a genuine matching-domain caller
precondition with an actual canonical provider; it need not be an automatically
authenticated registry entry for the outer grid. A numeric tag is not such a
precondition or provider, and a generic callable-table service cannot execute
CFD484 as code.

The source establishes a concrete provider for the known successful default
producer domain. It does not establish that every current publication or valid
replacement belongs to that domain, nor a universal equivalent resolver across
all genuine renderer instances. **Full receiver/producer/domain admission is
retained as open. No C++ is approved by this refinement.**

The report pins the bounded source dependencies, known writer findings and
prior 86-pin audit/retirement-plan evidence. It reuses the prior saved native
70B220/70B280 and parent-current-cell proof; no new native body scan or Ghidra
query occurs. Build/device evidence remains historical and domain-qualified,
not new application, heap authentication, ABI or gameplay proof.
