# Allocation statistics startup activation readiness

The existing allocator, genuine constructor, registration and lifecycle providers
are sufficient for a real 12-byte statistics receiver. The permanent process cell
is accepted. Pending host-binding proposal `abb468ec7` supplies the retained
two-cell context and both scalar-deletion routes. After that proposal is accepted,
no additional low-level provider is identified for normal startup ownership.

Activation is not yet implemented or authorized by this packet. The remaining
work is the startup caller's allocation/cleanup state and its failure-retention
boundary. A direct constructor substitution without those boundaries could free
a published or registered receiver and later drain the stale manager graph.

This readiness packet changes only this document and its report. It performs no
C++ edit, compilation, test, game launch, Native execution, live Native address
query, installed-image read or Ghidra mutation. Names describe reconstructed
Source interfaces; no drop-in Native ABI or original switch support is implied.

## Separate frozen epochs

The host proposal is frozen at
`abb468ec7290aa8e337af22c6b2f9aa770de1385`, still pending primary comparison when
this packet began. Accepted main at start is
`5a2ba8aa30b447d1297e0d5fcf94729b8fac443d`. Each has a complete frozen domain of
4,206 tracked `src/` and `include/` files, plus named build/evidence inputs:
4,227 pending files and 4,225 accepted files in total.

During the freeze, accepted main advanced to
`06bb5f24d1290d8485d343e5d0c3aa4e5c2b0e4c`. Its full view is separately retained
using exact aliases for 4,223 unchanged inputs and new Git bytes for the two
gameplay-point files. Those changes do not alter the inspected stats startup,
constructor, lifecycle, process-cell or registration contracts. The earlier two
epochs remain immutable. No worktree resync or pending six-file edit occurred.

The pending/initial-accepted Source domains differ in the four intended
host/binding files and two independent gameplay-point files. Ten complete required
startup/provider files compare equal across the initial epochs. Complete sorted
queries over the frozen domains yield 60 / 43 stats-related lines and 41 / 41
startup failure-boundary lines. These are Source-domain findings, not claims
about untracked files or a running original game.

## Available ownership chain

| Dependency | Established Source contract |
| --- | --- |
| Allocation and free | `singleton_lifetime_allocate({SingletonAllocationKind::object,12,12})` returns raw storage through the existing malloc/new-handler/bad_alloc provider. `singleton_lifetime_free` uses the matching current Source CRT. Valid aligned DWORD accesses at offsets 0, 4 and 8 are required. No new allocator is needed. |
| Real constructor | `construct_native_allocation_stats_00be2900` calls exact base entry `construct_native_allocation_stats_base_00be2750`, then writes `D685F4`, raw `40000000` and zero. It does not allocate or free its receiver. |
| Publication and registration | The base captures the first manager's section, enters/increments, arms guard cleanup, publishes the receiver, obtains the second manager, reloads current stats, and calls genuine `BD0C30`. The first and second manager observations are not collapsed. |
| Registration wrapper | `BD0C30` validates bounds before its null argument test, appends through its actual argument slot, and adds no lock, duplicate filter or retain. Its Native/raw Source entry is ECX manager with one stack object argument and `RET 4`. |
| Process cell | Accepted `GameNativeStringProcess::allocation_stats_0109cefc()` returns the permanent zero-initialized cell by reference. The process object deliberately outlives host teardown. |
| Retained host binding | Pending `abb468e` retains the same canonical manager and stats cell, installs the required deletion context, and routes `D685E0` / `D685F4` to genuine `BE2890` / `BE2930` using the popped receiver and full flags. Primary acceptance is still a prerequisite in this readiness epoch. |
| Lifecycle | Accepted `BE27F0` uses genuine current-manager/unregister/captured-section/base-reset providers. Both scalar entries call it before testing flag bit 0 and conditionally freeing the original receiver. No failure-path free is added. |

The original constructor convention is ECX receiver, EAX receiver result and
`RET`. The retained C++ constructor APIs additionally borrow an explicit context;
the Source API is not the original register/private-frame ABI. `BE2840`, supplied
as a candidate evidence anchor, lies inside `BE27F0`; it is not a constructor
entry and is not substituted for `BE2750`.

## Existing insertion position and caller cleanup

`GameStartupHost::run_initialize_phases` currently captures the module directory,
creates local `AllocationStatsState allocation_stats{}`, calls the semantic
`construct_allocation_stats_00be2900`, and then calls
`ensure_frame_clock_0073d480`. Object-handle resolver installation follows.
The semantic projection writes a null profile, budget and zero count. It neither
allocates the real receiver nor publishes/registers it.

The existing position is therefore already identified. A future bounded Source
packet can replace this local projection with the existing 12-byte allocation
and genuine constructor using the retained host context. It must not move the
operation to a new startup phase, add a new policy switch, use current publication
as the allocation/receiver, or convert a previous successful call into a silent
first-success fallback. Core already registers the constructor and lifecycle
translation units; no new low-level implementation is required.

The saved startup prefix supplies a zero EDI only under the intervening callees'
ordinary nonvolatile-register contract. The retained 35-byte allocation window
requests 12 bytes, saves returned EAX to the caller spill, sets state zero and
conditionally invokes `BE2900` at `0073D478`. A null allocation skips construction.
The Source allocator ordinarily returns storage or throws; that does not erase
the defensive Native null branch or permit a successful empty-owner claim.

Saved state-zero metadata selects action `C86A30` and transition to -1. Its saved
body loads the current spill word, pushes it and calls free. The separately
accepted physical tail is `POP ECX; RET`. It does not read current stats
publication, clear publication, unregister or add another cleanup call. A Source
captured allocation pointer is an explicit Source identity contract; the action
does not independently prove that the Native spill still holds the original
allocation. Original OS/handler association remains conditional.

One ordering detail must remain explicit: the saved post-call sequence first
compares `[01090AB0]` at `0073D480`, then stores -1 to the sentinel at `0073D486`.
The existing Source clock helper can allocate and validate `ClockServices`
before it compares publication. A stats cleanup guard must not simply surround
that whole helper: a later clock failure would then free stats after its Native
caller state was already retired. Conversely, immediate post-constructor disarm
does not establish the saved AB0 hardware-fault window. The future implementation
must state its bounded ordinary Source retirement contract while preserving the
saved comparison/order qualification. This packet adds no extra publication read
or reordered clock construction.

## Concrete failure boundaries

| Point | Required boundary |
| --- | --- |
| Allocation throws | No receiver returned; no constructor or outer receiver free. Do not manufacture a successful owner. Any conservative retention policy for this event must be named as Source policy. |
| Allocation returns null | Skip construction; represent no actual owner. Do not register a surrogate, claim constructed state or invoke a first-success fallback. |
| Constructor escapes | Preserve its genuine guard/base cleanup. Outer cleanup uses the captured allocation subject and adds no destructor, publication clear, unregister, rollback or retry. |
| Escape after publication/registration | Those effects persist. The outer free may leave stale global/manager references. Prevent later normal/fallback drain and CRT callbacks from treating that graph as a successful owner domain. |
| Construction completes | Retire outer allocation cleanup at its stated boundary. The raw registered receiver survives the local phase and is left to canonical scalar deletion. |
| A later startup operation fails | Do not re-enter the retired stats caller free. Preserve existing subsystem retention gates and canonical drain requirements. |
| Stats destructor escapes | Do not reach scalar flag-test/free by an added catch/finally. Preserve existing raw-drain cleanup/rethrow and its limits. |

Current Source contains no statistics startup state or failed-stats retention
gate. It already has a named conservative clock policy that closes logging and
uses `std::_Exit(1)` before unsafe cleanup; this is a design precedent, not an
installed stats policy or proof of original FH3 behavior. Four existing host
boundaries invoke that clock policy: the destructor, `exit_process` before
`std::exit`/atexit, `application_shutdown`, and
`destroy_singleton_lifetime_manager`. A future stats policy must cover the
applicable boundaries before a failed graph can be drained. It must distinguish
unattempted, returned-null, constructing, completed and failed outcomes without
using receiver/publication equality as a success test.

The smallest expected Source surface is `include/bsp/game_hosts.hpp` and
`src/game_hosts.cpp`, after host-binding acceptance and explicit authorization of
the caller-state/cleanup/retention design. Existing allocator, constructor,
lifecycle, process-cell and registration implementations need no identified
change. No original-mode support, cache binding, metric predicate or allocation
report activation follows from this assessment.

## Evidence and limits

The companion report pins the complete Source domains, exact Git deltas, all
queries, eleven bounded whole-file-backed Source excerpts, retained post-call
metadata, and three complete prior evidence bundles: owner-handover readiness,
raw lifecycle Source, and pending host-binding Source. The latter retains the
earlier process-cell proof separately; pending compilation is not recast as
primary acceptance. Earlier Native tail/failure/frame qualifications remain
unchanged, including the raw forwarded-argument/parent-node association, free's
actual fault behavior, Native FH3/SEH/OS delivery and game execution.

Local evidence is under
`local/cc12_allocation_stats_startup_activation_readiness/`.
`verify_evidence.py` replays complete Git/source bytes, every query result, static
contract assertions, every payload of the three retained bundles, the final
readiness bundle and its exact two-file commit. `freeze_evidence.py` packages
the full inputs, scripts, document and report; final verification/commit receipts
hold manifest/archive pins outside the archive to avoid self-reference.

No new compiler, build, test, ABI, Native execution or gameplay credit is claimed.
This is a completed readiness assessment with a concrete next Source contract,
not startup activation.
