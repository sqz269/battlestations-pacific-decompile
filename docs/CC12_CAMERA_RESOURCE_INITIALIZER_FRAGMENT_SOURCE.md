# Camera-resource initializer: unbound Source fragment

`initialize_native_camera_resource_fragment_00cd8390` now expresses the
previously observed local `00CD8390..00CD83DE` schedule through an ordinary
C++ interface. It borrows every publication cell and calls the existing real
Scene initializer and shared counter getter. The fragment owns no storage and
has no production caller or CMake registration in this packet.

This is a fragment of 79 observed Native bytes / 16 instructions, not an
admitted whole original function. The last accepted metadata still records
no exact or containing function at `CD8390`. Original entry/register ABI,
CRT consumer/order, complete body ownership and Native EH remain held.

Baseline: main `0fd0169927d7650b19905ee9715d0f4d9d874374`.
The implementation adds only its header, source, this document and report.
Root performs registration and the full project build/review separately.

## Borrowed contract

`NativeCameraResourceInitializerFragmentContext` contains references to the
actual guard byte and four distinct destination words with roles:

| Role | Address label |
| --- | --- |
| Guard | `01090266` |
| Own ID, published last | `01090288` |
| Scene ID | `0109028C` |
| Root ID | `01090290` |
| Name-address word | `01090294` |

The context also supplies the actual original literal-address token
`00D6327C`, the canonical three-word Scene view `01090210`, a genuine
`NativeMeshResourceTypeIds` provider, and the same retained
`TypeIdCounterLifetime` used by that provider and its root domain. The Scene
pointer must equal `scene_types.storage().scene_01090210`; its two parent IDs
are read from the current cells, not captured at context construction.

Correct cell identities, nonconflicting aliases, stable bindings throughout
dependency calls, actual literal binding and sufficient lifetimes are caller
contracts. The fragment does not validate or synthesize a missing domain.
These address labels do not create fixed-address host mappings or establish
a complete camera descriptor layout. The name token is stored numerically;
its string is never dereferenced, copied, replaced or interpreted.

The public header and implementation leave the existing providers unchanged.
There is no private counter, cached parent-ID copy, guard reset, fallback
provider, rollback or retained production camera descriptor.

## Observed publication order

| Retained Native observation | Source operation |
| --- | --- |
| `CD8390`, `CD8397` | Return immediately when the current guard is nonzero |
| `CD839E`, `CD83A5` | Store guard `1`, then the bound original name token |
| `CD8399`, `CD83AF` | Pass canonical Scene storage to the real Scene provider |
| `CD83B4`, `CD83B9` | Load both current Scene own/root words before either destination store |
| `CD83BF`, `CD83C4` | Publish captured Scene ID, then captured root ID |
| `CD83CA`, `CD83CF` | Call the real shared counter getter, then load current DWORD+4 |
| `CD83D2`, `CD83D5`, `CD83D8` | Compute modulo-`2^32` increment, store it, then publish the captured old value to own ID |
| `CD83DE` | Return from the Source fragment; no original return-register contract is claimed |

Each current cell access is volatile and the statements preserve the observed
load/store sequence. Binding-address selection is an explicit Source input;
the original register setup is not reproduced as a binary ABI. A Source
exception from Scene leaves the guard/name writes intact. An exception from
the later counter getter also leaves both parent publications intact. A later
call sees the sticky guard and returns without repairing that state. No new
exception handler or cleanup is introduced; original EH and hardware-fault
behavior are outside this interface's claim.

## Compiler and provider evidence

The candidate and the unchanged `native_mesh_subset_loading.cpp` and
`light_type_bootstrap.cpp` each compile successfully as ordinary MSVC x86
objects. Flags come from the actual generated main `bsp_core` Release project
and its provider command log: `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict
/std:c++17`, with the same definitions and dependency include paths. Only
Source/output paths are relocated to the worker; `/sourceDependencies` and
`/Bv` retain dependency and compiler receipts. Compiler version is
`19.51.36244.0`, installed toolset directory `14.51.36231`.

The candidate's function is an 84-byte Source COMDAT in section 7, symbol
index `0x16`. Its two `REL32` records are fully indexed:

| Source section offset | Symbol index | Genuine provider definition |
| --- | --- | --- |
| `0x22` | `0x15` | `NativeMeshResourceTypeIds::initialize_scene_resource_00b869c0`, freshly compiled provider symbol `0x600` |
| `0x3F` | `0x14` | `TypeIdCounterLifetime::get_006fac20`, freshly compiled provider symbol `0x3C` |

The Scene provider also references the real shared counter and root provider;
the root definition is present at symbol `0x3F` in the freshly compiled light
object. Complete symbol records, including auxiliary entries, all sections,
all relocations, raw object files, compiler dependencies and provider Source
are retained. The candidate has an unrelated compiler-generated helper
COMDAT; its separate section is not misattributed to the fragment.

Candidate Source disassembly confirms guard/name writes at offsets `0x10` /
`0x19`, Scene call `0x21`, both parent loads `0x29` / `0x2E` before stores
`0x34` / `0x39`, counter call `0x3E`, increment publication `0x49`, and final
own-ID publication `0x4F`. The nonzero guard jumps directly to the epilogue.
These offsets describe newly compiled Source only, not original Native code.

This is object compilation and static symbol/order evidence. No linking,
probe, fixture, CTest, game execution, full project build, production binding
or binary ABI validation was performed. External provider dependencies remain
real unresolved link inputs in these isolated translation units. Their
complete records are retained; none was stubbed merely to obtain a link.

## Scope and remaining gates

The packet used only retained Native observations and current Source. It made
no Native memory/PE byte, disassembly, decompile, table, name, handler or body
query. No live Ghidra analysis was requested beyond the required initial
`brief` health/status check. No GPR, configuration, ledger, CMake or existing
provider file changed.

The prior readiness report's accidental viewing of a stored historical
Scene listing remains historical evidence with no new validation credit.
The inherited typed metadata and its runtime identity are not refreshed
here; loaded Java CodeSource remains unattested.

Production camera backing cells, application startup invocation, original
CRT consumer/order, name contents/extent, complete original function ownership,
entry ABI, Native EH and game behavior remain held. This fragment must not be
silently registered as a replacement for the unknown original whole function.

Report: [Source, compiler, dependency and complete COFF receipts](../reports/cc12_camera_resource_initializer_fragment_source.json).
