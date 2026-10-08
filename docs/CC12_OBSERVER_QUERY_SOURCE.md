# Observer pair query 00694AF0: complete normal-return Source

The Source query now returns whether the existing actual observer edge lookup
finds the supplied endpoint pair. It captures an outer lock section through
the real lifetime provider, retains the lookup's own nested lock acquisition,
evaluates presence, then releases the captured outer section. The declaration
and body are in `include/bsp/observer_edges.hpp` and `src/observer_edges.cpp`.
No lock service, virtual lookup, global-address substitute, or CMake entry was
added. The descriptive name is a reconstruction hypothesis.

This closes the bounded implementation proposed by
`CC12_OBSERVER_QUERY_READINESS.md`. Its installed-PE/live-listing evidence
covers every normal-return instruction of `00694AF0..00694B8C`: 157 bytes,
52 instructions, body SHA-256
`01cfbb1d6cb08ff91919ca2d6606d3bb7cade7e62ca79caabfc83d0fd661e968`.
This Source packet reused that evidence and existing providers; it did not
expand another native callee or mutate Ghidra.

## Behavior and admission

The new C++ interface is:

```cpp
bool observer_pair_registered_00694af0(
    NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner,
    NativeObserverLifetime& lifetime);
```

`ObserverEdgeGuard` captures
`lifetime.lock_owner_00694280()->section_04`. For a nonnull section, the
existing guard performs real `EnterCriticalSection`, then increments the
writable DWORD at section+18h. The existing `find_pair_006949d0` independently
obtains the current lock and searches the actual endpoint arrays under its
own guard. The return expression tests its pointer before the outer guard is
destroyed; destruction decrements the captured depth and calls real
`LeaveCriticalSection`. A null section skips that guard's OS/depth operations
and still allows lookup. There is no new null-owner recovery.

This borrows the established actual-storage admission: two live, aligned
10h-byte endpoint prefixes with valid pointer arrays at +4/+8/+Ch; counts
within capacity; and valid reached 10h-byte edges carrying actual first and
callback-owner identities at +4/+8. Empty null-data spans require zero count.
The existing lifetime context, publication cells, manager, required dispatch
owner, lock owner, section and reached allocations must remain valid through
the operation. A nonnull section must be a genuine initialized Win32 section,
not copied lock bytes. Endpoint profiles are neither called nor validated.
Querying does not change arrays, edge identities or reference counts; the
existing lazy lock getter may construct, publish and register its owner.

The original entry uses ECX=first and EDX=callback owner, no explicit stack
inputs, and plain RET. It preserves EBX/ESI/EDI and canonicalizes only AL to
0/1 after outer release; upper EAX is not a boolean contract. The Source
interface adds an explicit lifetime reference and uses the current compiler's
C++ ABI/EH. It is not a replacement for the original register entry.
`00C7E9B8` and its private FH3/SEH tables remain unexpanded. Current C++ RAII
does not establish original exception-table identity, asynchronous-fault
behavior, concurrent teardown, enclosing class lifetime, or world ownership.

## One connected Source fixture

The ignored `local/cc12_observer_query_probe.cpp` uses the current
`GameSingletonHost` and `GameObserverRuntime`, initializes the real dispatch
owner, verifies actual manager storage/publication, and obtains the real
observer lock through the existing provider. It admits two borrowed raw
endpoint prefixes surrounded by canaries. Their inert profile words are never
called; they do not claim to be constructed unit or group objects.

One connected sequence queries absent, registers through
`register_observer_pair_00694a60`, queries present, unregisters through
`unregister_pair_006952a0`, then queries absent. The registered edge is the
same live edge returned by the real lookup, has both expected endpoint
identities, and starts with reference count one. The query preserves all
16 edge bytes, both endpoint prefixes, active array entries and allocated
spare-word guards.

The fixture temporarily observes its own Win32 import slots; both wrappers
forward every call to the saved real API. For each of the three queries it
observes the following physical depth and OS recursion transitions:

| Operation | Tracked depth at API entry | OS recursion before/after |
| --- | --- | --- |
| Outer enter | 0 | 0 -> 1 |
| Nested enter | 1 | 1 -> 2 |
| Nested leave | 1 | 2 -> 1 |
| Outer leave | 0 | 1 -> 0 |

The fixture verifies the owner/section identity, restores both import slots,
tears down the endpoint arrays through the existing real providers, and drains
the actual raw singleton manager. The runtime reports two registered slots
drained, null dispatch/lock/manager publications, and the expected retained
alias. No neutral host fallback was consumed. The fixture passed; it executes
new and existing Source only, not the original observer-query entry or EH.
The null-section branch and lazy initialization specifically from inside the
query are source/evidence checked, not separately exercised by this case.

## Build and evidence identity

The worker integrated main revision
`9fd2a70008be78322b65f12f2a3f0431935d71a6`; its tested precommit merge revision
is `2c68110b9` with this two-file Source patch. `scripts/build.ps1` succeeded
for MSVC Win32 Release. All three existing CTests passed:
`reconstructed_math`, `native_math_differential`, and `tool_tests` (11.26 s).
Seed verification enabled the existing math differential test; that math
execution is separate from this Source-only observer fixture.

The fixture compiled with MSVC 19.51.36244, toolset 14.51.36231,
`/O2 /std:c++17 /EHsc /MD /W4 /WX /fp:strict`. It links current completed
project objects and libraries, includes `/MANIFEST:EMBED`, and has a verified
Win32 PE and embedded `asInvoker` manifest. Compile sealing covered 279
unique inputs, with compiler-reported headers covered and no changed or
uncovered inputs. Link sealing covered 478 inputs and all 23 searched
libraries. The final execution seal covered 2,543 files, including the
480 linked project source modules, their compiler-read dependencies and
objects, fixture inputs, toolchain backends, libraries and executable. No
pinned file changed across the final execution.

Six key `bsp_core.lib` members exactly equal the completed build objects for
observer edges/lifetime/dispatch owner and critical-section construction,
release and singleton publication. The query's current COFF code section is
127 bytes, with relocations to the actual lock getter, lookup, real Win32
enter/leave imports, and current C++ EH/security machinery. The linked map
places the query at `10005BB0`. This identifies the compiled Source provider;
it does not assert equality with the original 157-byte function.

The final evidence archive is
`local/cc12_observer_query_source_evidence_current_main_20261008/` in this
worker worktree: 147 files, 102,103,283 bytes. Its manifest SHA-256 is
`e934276d6aebd1e6b3836d0547ffb8f40d991395e3cde06f1acab447af5f236b`.
It retains the fixture, tool commands, source snapshots, logs, input seals,
link map, six provider objects, game objects and three project libraries.
The tracked JSON report records the hashes and verification boundaries.

An earlier successful build/fixture archive remains separate. The main merge
changed `cmake/startup.cmake` among the initial 30 source/build pins; the
observer/provider inputs and 279 compile inputs stayed unchanged. The final
archive uses the completed current-main build and a new pre-execution
baseline. A nested-log-redirection error initially prevented relinking and
left the prior executable; the invocation was corrected and the final check
requires nonempty coverage of the 23 searched libraries. There were four
executions of this same one case during evidence preparation, not four
independent behavioral cases or a replay of previous fixture families.

No original query execution, original-ABI compatibility, original exception
behavior, game startup, world/class construction or gameplay validation is
claimed. No tracked test framework, ledger entry, annotation or shared build
file was changed by this worker packet; integration metadata belongs to the
primary agent.
