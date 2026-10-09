# CC12 GlobalConfig current-dispatch source

`NativeGlobalConfigCurrentDispatch` now implements the two existing
`GlobalConfigEffects` callbacks with genuine current-table calls. Its declaration
is in [global_config.hpp](../include/bsp/global_config.hpp), and its two methods
are in [global_config.cpp](../src/global_config.cpp). This adds **zero additional
Original functions**: it supplies concrete source dispatch glue for the already
audited native call sites.

| Method | Fresh lookup and native call shape |
| --- | --- |
| `stop_slot_08(actual_object, flag)` | Read the table from the actual object, read byte offset 8, call `void (__thiscall*)(void*, std::uint32_t)` with the actual receiver and flag. |
| `zero_references_slot_00(actual_object)` | Read the table from the actual object, read byte offset 0, call `void (__thiscall*)(void*)` with the actual receiver and no stack flags. |

Both use the existing `read<T>` byte-copy helper and assert four-byte object
and function pointers. Each invocation reads the current table anew. Neither
method accesses its receiver after the target returns, adjusts references,
clears slots, selects a profile, installs a table, or substitutes a call site.

The existing private `release()` remains responsible for the actual
`InterlockedDecrement(object+4)`, exactly once per non-null release operation,
and invokes zero dispatch only when the result equals zero. Post-stop slot
reloads, clear-after-callback behavior, paired iteration and both reverse
cleanup passes remain unchanged. Constructor, getter, strings, vectors, scalar
cleanup and caller behavior also remain unchanged. No provider instance or
`GlobalConfigContext` wiring is added in this packet.

## Required payload contract

The [readiness audit](CC12_GLOBAL_CONFIG_CURRENT_DISPATCH_PROVIDER_READINESS.md)
still controls admission. Objects must expose genuine callable current-process
tables and an accessible aligned four-byte intrusive counter at actual `+4`.
First-array objects require zero `v+0()`; second-array objects additionally
require `v+8(flag)`. Any replacement installed during stop must independently
satisfy the same representation, reference and lifetime conditions.

The existing interface is `noexcept`, so actual callbacks must be nonthrowing.
This source change does not claim original exceptional EH/SEH equivalence.
The unknown second-array producer remains external; there is no empty-array
argument or forced `D5ABF8` profile.

Projected `SoundInstance`/`SoundLevelEntry` objects remain incompatible with
raw prefix access, and `SoundChannelRuntime::release_reference` would decrement
again. Raw `SoundSampleStorage` currently carries the numeric `D5B074` identity
and uses its separate source scalar adapter. This provider does not turn that
identity into a callable table or replace that adapter. Shared context bindings,
outer-owner scalar dispatch and actual payload/table construction remain
separate integration work.

## Evidence and validation boundary

The [source report](../reports/cc12_global_config_current_dispatch_source.json)
records both exact added blocks and before/after source pins. Removing the
single added block from each file reproduces its base Git blob byte-for-byte.
Source inspection confirms two fresh table/method loads and preserves the sole
existing `InterlockedDecrement` expression.

Physical filesystem SHA-256, LF-normalized SHA-256, exact staged-blob SHA-256
and Git blob OID are recorded separately. Both worker files are physically
LF-only, so their physical and normalized hashes currently agree. A CRLF
checkout can have different physical hashes while retaining the same normalized
content and Git blobs; physical equality is not inferred from normalization.

No worker build, test, probe, Ghidra mutation, CMake change or ledger edit was
performed. Root owns the normal Win32 build, emitted receiver/stack-call review
and integration. The preceding readiness evidence contains six native spans
totaling 473 bytes; this packet reuses that reviewed contract without claiming
another native capture, emitted-code verification or runtime result.

## Primary build and emitted review

The primary reviewed both complete compiled methods and their compiler EH handlers. Stop uses actual payload ECX and one full flags word through current table+8; zero uses actual payload ECX and current table+0 with no argument. Neither decrements or reads the payload after return. COFF spans are 69/65 bytes including five CC padding bytes each; normal code is 64/60 bytes. Compiler cookie/FH3 support is retained as an explicit runtime boundary. The five provider roots remain absent from the game map; nonthrowing payload admission and complete context/application binding remain external.

The normal MSVC Win32 build exited zero and all three existing checks passed. Seventeen bounded physical Source/recipe fingerprints were captured before the build, rechecked afterward and retained with whole objects, unique exact Core members, library, map and logs in `local/cc12_current_dispatch_primary_review`. No new tests/probes or current application runtime checks were performed. See `reports/cc12_current_dispatch_primary_review.json`.
