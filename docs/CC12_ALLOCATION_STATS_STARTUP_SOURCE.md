# Allocation statistics startup Source wiring

Phase 0 now allocates the genuine 12-byte receiver and conditionally calls
`construct_native_allocation_stats_00be2900` with the retained canonical
`GameSingletonHost::allocation_stats_context()`. This replaces the semantic stack
projection at the already accepted startup position. Existing allocation/free,
constructor, registration, manager and profile-dispatch providers are unchanged.

The packet owns exactly `include/bsp/game_hosts.hpp`, `src/game_hosts.cpp`, this
document and its [report](../reports/cc12_allocation_stats_startup_source.json).
There were no Native-address leases or new Native reads. The implementation
uses the approved [primary readiness contract](CC12_ALLOCATION_STATS_STARTUP_ACTIVATION_READINESS_PRIMARY_REVIEW.md).

## Caller ownership and cleanup

The appended byte state distinguishes unattempted, allocating, allocation failure,
null skip, construction attempt, normal handover and irreversible constructor
failure. A repeated attempt is rejected. Allocation failure returns no receiver
and creates no stats retention/free obligation. A null return skips the constructor
and remains skipped. Normal success hands the captured receiver to the existing
manager/profile dispatch without testing publication for success.

The constructor catch installs failure before conditionally freeing the captured
allocation and rethrowing. It preserves all existing callee publication,
registration and cleanup effects. It adds no destructor, unregister, rollback,
publication clear, retry or fallback. A potentially dangling publication or
uncertain manager graph is contained by `exit_if_allocation_stats_failed()`.
The check follows the existing renderer/Lua/clock/VFS gates at all four boundaries:
destructor, `exit_process` before CRT exit, `application_shutdown`, and manager
destruction. It logs, closes the log, and calls `std::_Exit(1)` before host/manager/CRT
cleanup. This is an explicit conservative Source policy.

The single volatile AB0 comparison is inside the bounded caller catch. The saved
bool is passed to the subsequent clock null decision. Stats cleanup retires before
stats logging or throwing clock-context allocation/profile validation. The existing
constructed-clock validation branch remains; a second startup attempt is rejected
before allocation. The ordinary Source boundary does not establish Native hardware
fault delivery in the saved comparison-before-retirement interval.

## Frozen baseline and actual compiler inputs

Baseline `1a53da3572c0e52c907de321e4f854034dfbb42b` is the accepted Source1992 epoch:
1,992 Source inputs and four artifacts, 424 selected whole Core objects,14 selected
App objects, and all 80 actual App candidates. Both full manifests and every payload
were retained before edits. The freeze also contains all 4,212
current tracked Source/build-input Git preimages,2,493 actual compiler dependencies,
69 compiler tools and 20 link libraries.

Both Core/App Release projects and their actual CL/link command/read records are
retained. All 32 actual `game_hosts.hpp` consumers were compiled before and after
using their actual App/Core settings (`/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict`
and C++17), with output/include paths rebased to this worker. Full command lines,
dependency records, compiler version/layout reports, objects and five dumpbin
representations are retained. All 14,065 baseline dependency occurrences were frozen;
no additional dependency was discovered. The include order stayed identical, and
only the two owned Source files changed among those actual dependencies.

Later main `3ffe9406dc2af2ff09d9b2975cbe5563f23d7563` changes six documentation,
report or name-ledger files. Its full diff is retained separately and has zero
Source/compiler-input changes. It does not relabel the frozen Source1992 proof.

## Complete Source and emitted-code audit

Reversing the explicitly bounded changes recreates both complete Source preimages.
Actual MSVC layout remains 1,992 bytes. Every old field offset stays fixed;
`constructed_` remains at 1,984 and the new byte state occupies old padding at 1,985.
All 31 other header consumers have identical nonmetadata code/data/EH/relocations.
All 32 candidate objects match their normal-build objects outside metadata.

In the actual `run_initialize_phases` code section:

| Event | Section offset | Emitted evidence |
|---|---:|---|
| Actual request |178h–196h| kind3/object,12,12; genuine allocator call |
| Captured identity |19Bh,1A7h| EAX to EDI and saved spill |
| Constructor guard |1ADh–1C9h| EH state 6; null skips; original EDI plus retained context |
| AB0 observation |1D1h,1D7h| comparison and saved `SETE` result |
| Cleanup retirement |1DEh| EH state returns to 3 |
| Handover/skip |200h| phase byte written before first stats log call 206h |
| Clock decision input |34Ch–355h| saved comparison result passed to `ClockServices::ensure(bool)` |
| Allocation catch |2F25h| allocation-failed state 2, rethrow, no receiver free |
| Constructor catch |2F7Dh| failed state 6 at 2F89h before captured free 2F90h, then rethrow |

The full emitted EH data confirms separate try 4/catch 5 and try 6/catch 7 regions.
MSVC optimizes the Source conditional catch free to an unconditional free because
the only ordinary throwing path has a nonnull receiver; the null comparison path
cannot throw under `/EHsc`. Full before/after bodies, EH tables and helper sections
are retained, including all later startup code. Two generated startup lambdas have
explicit bijective name qualifications. The unchanged `kCandidates` array retains
all bytes and relocations under its changed local-scope spelling. Complete SafeSEH
membership changes only for the explicit clock function signature renames.

Complete relocation graphs start at every `game_hosts` code section and physically
index every actual header consumer plus every selected whole provider. Before:
1,030 objects,958 archive members,57,262 reachable sections,165,164 edges. After:
1,031 objects,959 archive members,57,272 reachable sections,165,192 edges. Each has
40 further whole App providers. There are zero unresolved BSP symbols; genuine
CRT/Win32/external services remain explicit frontiers. Indexes preserve every
symbol/AUX index, relocation offset/type/target, code byte and embedded switch data.

Physical inventory schema2 accounts for 3,716 occurrences,2,336 distinct COFF files
and 157,405,046 bytes with zero gaps and zero unclassified executable bytes. Ordinary
function symbols, EH/catch/unwind functions and other executable labels are distinct
inventories; symbol counts do not claim nonoverlapping function extents. All physical
aliases, both whole archives, complete code/data indexes and source/provider
manifests remain retained. The large physical inventory is hash-pinned by the report.

## Normal build and qualification limits

`./scripts/build.ps1` passed both configured checks: `reconstructed_math` and
`tool_tests`. No tests were added. Native differential testing is not configured in
this worker; no seeds were extracted or generated. Root's prior 3/3 remains separate.
The normal library/executable/map and LastTest/CL/link records are frozen with
1,057 selected/provider Core objects and all 80 App objects. The genuine BE2900 constructor and retained context accessor are now linked.
BE2750 remains a complete retained COFF definition; a separate BE2750 map symbol
is not claimed. The two
distinct scalar-delete COFF definitions share a normal linker address through ICF;
that is not a Native symbol-alias claim.

Cross-worktree comparison qualifies 1,133 of 1,137 whole objects using 400 explicit
lambda pairs and 286 RTTI spellings, without ignoring ordinary code, relocation
offset/type/edge, or EH membership. Four generated-name sets remain unassigned:
`native_input_settings_tree_insertion`, `native_input_vector_map_index`,
`game_hosts_lua`, and `game_hosts_units`. Their complete bytes/indexes/differences
are retained. Unique whole-object labeled identity is not claimed for those sets.
All 32 same-worktree consumer comparisons remain fully established.

The final bundle manifest lists every full payload and both Source outputs. Run
`python local/cc12_allocation_stats_startup_source/verify_evidence.py --commit <commit>`
from this worker for complete offline Source/Git/COFF/graph/archive/bundle replay.
`--commit-only` binds the successful full replay and exact four Git outputs without
repeating it. Neither mode compiles, runs tests, launches the game or accesses Native
analysis. The final and post-commit receipts sit next to the immutable ZIP/manifest.

Startup and gameplay remain unvalidated. Native register ABI, private FH3/spills,
hardware-fault cleanup, OS registration and cookies remain unproved. This packet
does not activate a cache/report path or establish a supported original sound
selector, and makes no low-level provider, manager layout or CMake change.
