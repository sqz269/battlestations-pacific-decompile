# Retained Application Game caller frame: bounded Source composition

This adds `bsp::game::GameNativeGameApplicationFrame` in a new header/source pair.
It composes the previously reviewed 0073E150..0073E1D9 caller fragment with existing
Game and raw-string implementations. It supplies **zero additional Original
functions**, no application startup wiring, and no Original FH3/SEH cleanup.
The basis is `CC12_GAME_RETAINED_APPLICATION_FRAME_COMPOSITION.md`, commit
44de6c4887f4bb0cffe8c4e57a9d985c8e0477fe; the current base is
af15a550d63ee226f3b72ef5520f7a9e1454f493. The paired JSON report pins both new
Source files and rechecks all 16 direct and 26 earlier dependency pins.

## Inputs and retained storage

The stable, noncopyable/nonmovable frame borrows an exclusive fresh
`GameNativeGameRuntime`, the genuine `void* volatile` Application+14 cell, the
current `char* volatile` E1AE78 duplicate-input cell, and the canonical
`NativeStringRawPoolContext`. All construction/lifetime services, actual Game
and grid cells, source tables, operation frames, strings, resources, profile,
GlobalConfig and Dyn inputs must already be correctly composed and retained.
The raw-string context must share the runtime's string/pool/manager domain.
Freshness is checked; those domain/provenance obligations cannot be proved by
this API. Runtime operations must not be invoked independently while borrowed.

The frame owns the canonical allocation's disposition, an aligned actual 8-byte
name header, and explicit operation/status fields. The input string is captured
once after allocation/zeroing and remains borrowed. The frame does not replace
or free that duplicate. Before preparation, the raw header has no observable
preimage; the diagnostic accessor returns null. The callee initializes it.

## Normal schedule

1. Call `singleton_lifetime_allocate({object,0x71a0,0x71a0})`, retain its nonnull
   result, begin trivial `NativeGameStorage` lifetime by placement **default**
   construction, and zero every 0x71A0 byte. Record parent state 27h.
2. Capture the current E1AE78 cell once and call raw
   `construct_native_string_header_0041e870` on the actual retained header.
   Only after return mark the name complete and set the observed mask to `0x2`.
3. Record parent state 28h and enter the existing runtime's one-shot constructor
   using the retained allocation and header. Its internal E188A8 publication
   remains in the existing body before Dyn initialization.
4. On constructor return, write its **returned result** into Application+14.
   Then record parent state -1 and attempt the raw-pool overload of
   `destroy_native_string_header_0041dd20` once. Mark return complete only when
   that helper returns. It leaves the header bytes untouched.
5. Retain the live Game for explicit later scalar deletion. Full deletion flags
   are forwarded; only bit 0 determines successful outer-storage disposition.

The observed mask remains `0x2` after normal name return. It is only the observed
name bit, not a claim about every bit in the Original caller's stack word. The
initial parent state is likewise unobserved until `parent_state_recorded` becomes
true at state 27h. Distinct `Name` states record preparation, completion, return
attempt, return success and external resolution. A stale header or set mask
never authorizes another cleanup. Caller/helper site anchors are diagnostic
boundaries; exact child failure progress remains in the borrowed runtime.

## Supported failures and explicit retirement

| Failure | Preserved state and resolution |
| --- | --- |
| Canonical allocation | Frame failed, runtime fresh, no allocation/header obligation. Explicit diagnostic retirement requires `no_allocation`. |
| Name preparation | Retain zeroed Game storage and exact partial header; runtime fresh, mask unset, parent state 27h. Resolve the name externally, acknowledge it, then explicitly free retained storage. |
| Game construction | Runtime/child operations retain their failure; completed name, mask 0x2 and state 28h remain. The frame has not published Application+14. Resolve graph/publication/name obligations and acknowledge the runtime before explicit outer free or disposition acknowledgment. |
| Normal name return | Frame failed but runtime remains **live**; Application+14 was written and parent state is -1. Resolve/acknowledge the name, then its first scalar deletion is allowed. Construction and name return cannot be replayed. |
| Scalar deletion or custom free | Record full flags and attempted operation, retain runtime failure, and mark outer disposition **indeterminate**. No supposedly live storage pointer is exposed. A free override may already have released the allocation before throwing. |

Each supported C++ catch records its stage/site and rethrows. It neither invokes
Original cleanup funclets nor repairs arbitrary faults. Historical failure data
survives later resolution; `failed_stage` records the most recent caught failure.
The first scalar is permitted only with a completed constructor, live runtime,
resolved name, retained allocation and no previous scalar attempt.

Successful scalar deletion with bit 0 set records the allocation as released;
the returned address is only historical data. With bit 0 clear, graph destruction
is complete and the allocation remains retained for one explicit matching free.
`free_retained_storage()` also permits never-entered construction or an externally
resolved graph already acknowledged through the runtime. It rejects indeterminate
storage even after a scalar failure: the caller must establish and resolve that
allocation's actual disposition externally before `already_released` retirement.

`acknowledge_name_resolution()` changes only the pending-name status. It performs
no header access, return, clear or helper replay. Diagnostic retirement similarly
performs no cleanup; it requires a resolved name, a nonlive qualified runtime,
and explicit proof of either no original allocation or its prior release. A
failed runtime must first have its retained graph/children resolved and be
acknowledged through its existing API. These acknowledgments are assertions of
completed external work, not automatic recovery or proofs of external behavior.

Canonical allocation/free are `std::malloc`/`std::free`. The lifetime service's
default `free_00bf6989` matches that domain; any override must honor it and actually
release on a successful bit-0-set scalar. A throwing override cannot be assumed
to retain ownership. No path rereads freed Game bytes, repeats the scalar, clears
Application+14, or destroys the borrowed runtime. The frame's destructor performs
no resource cleanup and terminates on unresolved retained state, following the
existing retained-owner **Source policy**, not recovered Original destructor EH.

## Validation and remaining boundary

Only these four owned files changed. Static checks cover the source sequence,
single call sites, full-flag forwarding, raw-header initialization boundary,
mask/state ordering, deleted copy/move operations and dependency pin equality.
Both new Source files are pinned with physical and LF-normalized hashes and Git
blob identities; 16 direct plus 26 earlier dependency pins still match the prior
audit and current base. No fresh native export/query, new test/probe, compilation,
CMake registration, ledger edit or GPR mutation was performed in this packet.
The primary integrator owns build registration, the normal Win32 build and
emitted-code review. This report does not claim any of them already passed.

The application still lacks the actual Application+14 and duplicate-cell producer
composition and fully qualified Game construction/lifetime inputs identified in
the prior audit. This reusable frame does not start a game, establish startup or
gameplay parity, or make missing services safe by substituting defaults.

## Primary integration and emitted review

The integrator registered this retained glue in bsp_game. The normal MSVC Win32 build and all three existing checks passed. All 17 emitted methods/handlers and their ordered physical relocations were reviewed. Construction retains and zeroes the full 0x71A0 allocation, captures the current duplicated name, calls the real name and Game providers, publishes the exact returned pointer, sets parent -1, and attempts the name return once. The emitted scalar method retains the full flags and marks the allocation indeterminate before dispatch; free and diagnostic paths preserve their explicit guards. Both catch funclets only record failure and rethrow.

The frame methods remain absent from the game map. This retained composition earns no additional Original function and supplies neither actual Application ownership nor a duplicated-name producer. Native parent EH/SEH/private-frame equivalence and live failure/recovery/application/gameplay execution remain open.

Evidence: `reports/cc12_world_tick_reset_frame_primary_review.json`; complete retained artifacts under `local/cc12_world_tick_reset_frame_primary`.
