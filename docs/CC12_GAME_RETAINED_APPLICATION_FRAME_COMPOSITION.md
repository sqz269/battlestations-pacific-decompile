# Retained application Game caller-frame composition

A small stable Source owner is feasible using the existing allocator, raw-string
helpers and `GameNativeGameRuntime`. It must accept a **fully composed retained
runtime and genuine caller cells**. It supplies neither those missing producers
nor the Game constructor/lifetime services. This packet adds Source 0 and
Original functions 0.

The smallest justified pair is
`include/bsp/game_native_game_application_frame.hpp` and
`src/game_native_game_application_frame.cpp`, defining a noncopyable/nonmovable
`bsp::game::GameNativeGameApplicationFrame`. This is a proposed API boundary,
not an implemented provider or application-readiness claim.

## Borrowed inputs and retained state

The owner can accept four existing bindings:

- A stable, fresh, exclusively used `GameNativeGameRuntime&`, already composed
  with all real constructor, lifetime and canonical Dyn services.
- `void* volatile&` naming actual Application+14, distinct from the constructor's
  internal E188A8 publication. A private mirror is insufficient.
- `char* volatile&` naming the current E1AE78-equivalent duplicated-input cell.
  The earlier bootstrap owns this duplicate; the frame only borrows it.
- The canonical `NativeStringRawPoolContext&`, over the same pool, return gate
  and manager cells as the runtime's construction/lifetime string services.

The runtime copies contexts containing borrowed references and already owns
the Game-specific Source table plus persistent construction/destruction
operations. The new frame must not duplicate those graphs, stamp another table,
or infer that runtime construction validates every borrowed context. Its public
API does not expose enough identity information to prove the caller's string
or allocator pairing; these remain explicit composition preconditions.

The new owner retains the allocation and its ownership/lifetime disposition;
an aligned actual 8-byte name header; captured borrowed input; name preparation,
return-attempt and completion status; constructor result and publication status;
current/failing stage and site; and scalar/free completion state. Keep native
parent state `-1/27h/28h` and temporary mask `0x2` as evidence markers, not as a
claim to install Original FH3. Expose stable diagnostic state and live retained
addresses while retaining all underlying runtime operations and services.

## Exact normal composition

1. Enter a one-shot allocating stage and call
   `singleton_lifetime_allocate({object, 0x71a0, 0x71a0})`. Retain its result
   immediately. This allocator is nonnull-or-throw; the native caller clears
   the block before testing for null, so a nullable successful path is invalid.
2. Begin `NativeGameStorage` lifetime with placement **default** construction in
   that block, then clear exactly all `0x71A0` bytes with `memset`. Its extent and
   trivial default construction are already asserted. Record parent state 27h.
3. Capture the current input-cell value once at `73E17E..73E188`, after the
   allocation/zeroing. Require live nonnull NUL-terminated input. Call
   `construct_native_string_header_0041e870` on the retained actual header and
   raw context. The callee zeros both fields before strlen, resize and copy;
   no invented old header or caller-stack preimage is needed.
4. Only after preparation returns, mark the name complete and set mask `0x2`.
   Record state 28h and call `runtime.construct(*allocation, actual_header)`.
   Existing Game construction initializes and copies the name into Game+7164
   through its already-bound string storage.
5. Preserve the existing constructor's internal E188A8 store before Dyn
   initialization. On successful return, publish the actual returned result to
   Application+14 at the outer caller point; do not publish the allocation early.
6. After that store, set parent state to -1 **before** attempting name return.
   Call the raw-pool overload of `destroy_native_string_header_0041dd20` once,
   and mark completion only after it returns. This overload captures data and
   length+1, obtains the current pool and returns the block without changing the
   header. The storage-interface overload has a different `noexcept` boundary.
7. Retain the live runtime/owner for explicit later scalar retirement. The
   frame does not free/replace the original duplicated input or replay creation.

The retained native fragment confirms `OR [ESP+20],2` at 73E18D and `TEST ...,2`
at 73E1AA. Here `0x2` means the second bit, not bit index 2/value 4. There is no
normal mask-clear instruction in this fragment. The critical disarm is the
state=-1 store at 73E1B2, between Application+14 publication and pooled return.
A name-complete flag or stale nonnull header cannot authorize another cleanup.

## Failures remain distinct

| Failure | Retained state and permitted resolution |
| --- | --- |
| Allocation | Runtime remains fresh; no allocation or prepared name belongs to the frame. Preserve the failure stage and explicitly retire the attempt. |
| Name preparation | Retain the zeroed Game allocation and exact partial header; the constructor has not run and mask `0x2` was not set. Resolve the raw-string state explicitly before freeing never-constructed storage. |
| Game constructor | Runtime and construction operation are failed; retain the partial graph, completed name and all contexts. Application+14 is not assigned by this frame, but E188A8 may already be published before a Dyn failure. Resolve graph/child/publication obligations before diagnostic acknowledgement and outer free. |
| Normal name return | Runtime remains live and Application+14 is already assigned; parent state is -1. Retain the name-return attempt. Do not relabel the Game constructor failed, roll back the publication, free the live owner or repeat construct. |
| Scalar or qualified custom free | Preserve the lifetime operation and exact site. A custom free that can release then throw makes ownership uncertain; no automatic retry/free is valid until disposition is resolved. |

These are supported Source C++ failure boundaries. Retention does not supply
Original FH3/SEH cleanup or repair arbitrary faults inside borrowed services.
No destructor should infer cleanup from a partial stage. The existing retained
owner policy rejects unresolved destruction rather than silently discarding it.

## Explicit retirement API

The minimal owner can expose these operations in addition to status access:

| Proposed operation | Contract |
| --- | --- |
| `construct()` | Fresh one-shot attempt only; preserve stage/state and rethrow supported failures. Never reset to fresh. |
| `scalar_delete(flags)` | First scalar only, with a live runtime and resolved name obligation. Forward full flags. |
| `free_retained_storage()` | Explicit matching free of still-owned storage only after a never-entered constructor, completed flags-0 destruction, or resolved/acknowledged partial graph; name obligation must be resolved. |
| `acknowledge_name_resolution()` | Acknowledge external resolution; perform no header return/clear, graph operation or replay. A name-return failure leaves the runtime live and can then proceed to its first scalar. |
| `acknowledge_diagnostic_retirement(disposition)` | Explicitly account for resolved name and absent/already-freed outer storage. Do not free, roll back or resume operations. A failed runtime must first be resolved/acknowledged through its existing API. |

Canonical allocation is `std::malloc`; the existing lifetime service's default
`free_00bf6989` is `std::free`. Any override requires its own matching allocator
and failure qualification. A successful scalar with bit 0 set frees the block;
its returned pointer is only historical address data. Flags 0 destroys the graph
but leaves the block owned while the runtime becomes `destroyed`. A separate
matched outer free is then required; a second scalar is forbidden. Neither
default `delete` nor an automatic `unique_ptr` deleter is appropriate.

`GameNativeGameRuntime::acknowledge_diagnostic_cleanup` accepts only a failed
runtime and requires child-operation obligations resolved; it frees nothing.
It must not be called on the live Game left by name-return failure. Diagnostic
retirement must explicitly account for an externally freed block, including an
uncertain custom-free outcome, so no stale ownership flag causes a second free.

The frame destructor performs no string return, scalar, free or publication
rollback. This bounded creation fragment proves no later Application+14 clear;
application shutdown/publication retirement still needs its own qualified policy.

## Missing application producers and verification

The inspected `GameStartupHost::application_construct` explicitly leaves
subsystem pointers through +14 unreconstructed. Initialization validates the
mode and later parses it directly. It does not compose the audited retained
E1AE78 duplicate, actual Application+14 and Game caller frame. WinMain's current
`cachedload` constant matches the native input, but cannot replace reading the
actual current duplicate at the native point. The genuine duplicate primitive
exists; its earlier production and later retirement belong outside this frame.

Fully qualified profile, tables and opaque unit/local preimages, GlobalConfig
load, resource/parser/grid preimages, constants, Dyn construction and matching
lifetime contexts remain required inputs. This proposal supplies none of them.

After refreshing to published main `24229ec42885160e4eb02afcdff939e4a94f2c78`,
16 Source files have current pins with 32 bounded excerpts; all 26 pins from the
prior caller audit still match. The retained 138-byte/39-instruction caller
fragment matches the installed PE. No body re-audit, fresh native export, Source,
Ghidra, build, test, probe or game execution occurred. Evidence:
[cc12_game_retained_application_frame_composition.json](../reports/cc12_game_retained_application_frame_composition.json).
