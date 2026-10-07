# Native unit-group scalar lifetime

`delete_native_unit_group_0070d260` reconstructs the complete normal
`0070D260..0070D283` body (36 bytes, 12 instructions). The original function
is CFD6F8 slot zero: ECX is actual 508h group storage, the DWORD flags are on
the stack, EAX returns the original address, and `RET 4` consumes the flags.
Its existing `CG_scalar_deleting_dtor_0070d260` identity is preserved. The
primary agent repaired the missing `0070D27B ADD ESP,4` listing membership
before this packet; see `reports/cc11_unit_group_scalar_definition.json`.

The new C++ function captures the address, stores CFD6F8 through the existing
volatile observer-owner profile, calls the genuine whole
`NativeObserverLifetime::destroy_callback_owner_00695870`, frees the original
allocation through `singleton_lifetime_free` iff `flags & 1`, and returns the
captured address bits. There is no null guard, group virtual call, publisher,
new lifetime service, additional allocation, or substitute callback cleanup.
The return must remain opaque after deletion. Exceptions can propagate from
the existing base cleanup; this packet closes the ordinary path only.

The genuine base destructor stamps CE3CD4, takes an outer recursive lock and
a separately nested count-query lock, detaches nonempty registrations,
unlocks, then reloads and frees the callback array. The content exchange in
`00695530/006944C0` leaves that owner's array pointer and capacity intact.
After destruction those retained pointer bits can be dangling; count is zero
for the exercised nonempty lifecycle. Do not destroy the same group again.
The actual CF7E64 edge producer routes cleanup directly to existing
`delete_observer_edge_00693ca0`, so the production runtime's unsupported-edge
fallback is never required by this family.

Native evidence is pinned from the installed PE and a fresh read-only copy:

| Evidence | Extent | Role |
| --- | --- | --- |
| `0070D260` | 36 bytes / 12 instructions | Complete scalar wrapper |
| `00695870` | 193 bytes / 61 instructions | Existing complete callback cleanup |
| `00BF65AC` | 5 bytes / 1 instruction | JMP to historical CRT `00BF9DC8` |
| `0070DB20` | 58 bytes / 26 instructions | Allocation/constructor/publisher lineage only |
| `00CFD6F8` | first 8 bytes | Slots `0070D260`, `0070ECA0` |
| `00CF7E64` | first 4 bytes | Edge deleting slot `00693CA0` |
| `00CF4888` | 4 bytes | Constructor's actual `4479C000` bits |

Fresh read-only Ghidra queries verified `/battlestationspacific.exe` in the
existing `C:/Users/sqz269/bsp.gpr` process; scalar and base bytes match the PE.
The factory calls actual allocation, construction and publication, but the
fixture does not execute or claim that complete factory/publisher path.

One connected fixture family uses the production `GameSingletonHost` and
`GameObserverRuntime`, their actual raw manager/publication cells, published
dispatch owner, recursive lock, existing whole group constructor and existing
CF7E64 registration routines. A borrowed observed-unit prefix is initialized
by the existing partial `00925CFF` store projection; this is explicitly not a
complete unit constructor. Each group is registered twice to the same real
endpoint, producing one edge with reference count two. Original and Source
each run flags 2 and 3 on fresh allocations. The real singleton manager drains
while the observer context is still alive. No new fixture service subclass,
fake vtable, callback-body stub, or free interception is used.

The Original executes all 36 copied bytes, changing only the two natural
relative CALL operands at offsets `0A..0D` and `17..1A`: 8 designated operand
bytes and 28 retained bytes, including the profile store and repaired stack
adjustment. Its adapters invoke the same existing complete Source base
cleanup and real current-CRT free. They preserve these real operations while
recording Original calls; they do not execute the original 193-byte base or
historical CRT implementation. The copied body is RX at execution.

The completed family passed **134 checks, four cases**, including both
endpoint arrays, nonzero reference count cleanup, nested lock balance,
publication identity, full returned address bits and Original RET4 balance.
For flags 2, every byte of each retained 508h group matches its captured
preimage with only the base profile and count updated. Cross-provider
comparison normalizes only the independently allocated stale array-pointer
word; all other bytes compare directly. The complete 4F8h tail is preserved.
Flags 3 inspect external graph state and saved address bits without reading
the freed group. The actual host reports zero unimplemented providers.

The first attempt failed a fixture-only assertion that incorrectly expected
the retained array pointer/capacity to be null/zero. That executable, log,
executed body, object files and first frozen inputs remain in the unique
artifact directory. Only the fixture was corrected; the wrapper and providers
were unchanged. The successful revision uses separate inputs, objects,
executable, map, log and runtime outputs. The initial missing-Lua/zlib link
failure is also preserved. No earlier accepted fixture was replayed.

Whole new Source COFF is **41 bytes / 14 instructions**, SHA-256
`256b715a7c2f5af345dffb1d41a5073826e96cd7d5a6055f1dc45049d04a881c`.
It contains precisely the two real provider relocations, retains the address
in ESI across free, and has no post-free memory load. The linked executable's
complete body and both resolved call targets were checked. All three ABI
adapters and nine existing provider functions also have complete COFF
receipts; existing provider objects were extracted from the frozen library.

Two fresh translation units use 60 frozen repository/fixture inputs and 222
actual host includes. The real host's broad deletion dispatch requires four
support archives. The successful map retains symbols from 511 existing
translation units: 443 core, 36 game, 28 Lua and 4 zlib. This does not mean all
were dynamically executed. Exact archive members, provider commands and
2,064 recorded compiler inputs are pinned; 1,721 repository/generated inputs
are also frozen. The other 40 of 76 frozen game objects are candidates with
no retained map symbols. The original MSBuild tlogs establish build-input
provenance, rather than an instrumented original compiler read trace. Git
content comparison normalizes CRLF only; frozen bytes and SHA-256 retain the
actual primary files. The provider source tree matches the reported primary
build commit `6bf930f35` through worker base `8272aabe4`.

Current compiler/linker/librarian files, environment script, 21 searched
libraries, PE imports, embedded I386 asInvoker manifest and installed-image
before/after hashes are recorded. These are file/static-import receipts, not
runtime DLL-load traces. Nine earlier worker reports and all 917 referenced
artifacts remain unchanged. All new receipts are under
`local/cc11_unit_group_lifetime_20261007_a`; the report inventories their hashes.

This is reconstructed, standalone Win32 build-tested and bounded
Original/Source fixture-tested. The primary agent owns global build and
metadata integration. Native class replacement ABI, FH3/SEH and asynchronous
faults, allocation failures/new handlers, old CRT/SBH/heap equivalence, group
publication through virtual +5C, group callback slot +4, world integration
and live-game parity remain unbound. Current allocation and free use their
matching existing Source CRT domain; original game heap blocks are not used.
