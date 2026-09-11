# Native hardware-layout fields

This packet reconstructs four routines over the actual native layout and
declaration storage. It complements `NATIVE_HARDWARE_LAYOUT_OWNER.md`; it does
not replace the existing high-level `d3d9_vertex_layout` interfaces with their
different shared-ownership and bounds contracts.

| Native entry and complete span | Original ABI | New C++ function |
| --- | --- | --- |
| `B47640..B47656` (23 bytes) | ECX record; EAX same record; RET | `initialize_native_hardware_layout_record_00b47640` |
| `B48C00..B48C70` (113 bytes) | ECX owner; EAX same owner; RET | `initialize_native_hardware_layout_base_00b48c00` |
| `B48A00..B48ACE` (207 bytes) | ECX owner; stack declaration; RET4 | `append_native_hardware_layout_stream_00b48a00` |
| `B47D60..B47D87` (40 bytes) | ECX owner; RET; EAX unspecified | `recompute_native_hardware_layout_stride_00b47d60` |

The interfaces are new MSVC Win32 C++ functions, with explicit actual-storage
pointers. Append ownership uses `NativeHardwareLayoutOwnerContext` and its
borrowed native `D61D1C` profile, type-size table, and initialized CPU pool.
Descriptive names are reconstruction hypotheses. These are not drop-in ABI
replacements.

## Established storage and order

A stream record is 0Ch bytes: actual declaration pointer at +00, frequency at
+04, and an extra DWORD at +08. `B47640` stores null, 1, and 0 in that order,
without releasing any preexisting pointer, and returns the original address.

`B48C00` stores base profile `CEB130`, reference count 1 at owner+04, and hardware
profile `D61D10`. Original CRT iterator `BF7CD1` constructs exactly four records
at owner+08 through +37 using fixed constructor `B47640` and destructor
`B483F0`. It then clears count+38 and stride+3C in that order. The derived
COM field+40 and allocation slab token+44 remain untouched. Source performs the
same fixed four record initializations; valid storage and the fixed leaf have
no C++ throwing operation.

`B48A00` first initializes a local `{null, 1, 0}` record and arms its cleanup.
For a nonnull source it publishes the local source pointer and performs a real
`InterlockedIncrement` on declaration+04. It reads current owner count,
computes `owner + 8 + uint32(count)*12`, and stores count+1 before reading that
record's old pointer. If old and new pointers differ it publishes new first,
retains new, then releases captured old. Identical pointers skip those three
operations. It writes frequency1 and extra0 to the captured record, disarms
the local cleanup, and finally releases the source's temporary reference.
Changes made during old release to the current stream pointer or owner count
are preserved; metadata still goes to the captured record.

Release means real `InterlockedDecrement` on the actual declaration. Zero
reads the declaration's current profile and slot0. The supported `D61D1C`
slot0 is `BD30E0`, which rereads the current profile and calls deleting slot1,
`B48CA0`, with flags1. Source composes the actual CPU declaration deletion
implementation, its ordinary shared free, and its actual initialized pool.
No replacement owner object, private refcount, retain/release callback, or
arbitrary virtual-profile substitute is introduced.

`B47D60` captures the initial signed count comparison before clearing stride.
For a positive initial count it reads each actual record pointer and its
declaration+CC, adds with DWORD wrapping to current stride+3C, advances by 0Ch,
and reloads current signed count for the next comparison. The source uses
ordered raw DWORD accesses to preserve storage aliases. Negative count still
clears stride and performs no declaration access.

There is no four-record capacity check, null-declaration fallback, saturation,
or rollback in append/stride. The valid input domain requires every reached
address to be valid and every actual zero-reference dispatch to use the
supported native profile. Synthetic negative-count and wrap cases exercise
these arithmetic/alias contracts; they do not show that game callers use them.

## Exception evidence

Append registers handler `CBF6F8`, `FuncInfo DF80F8`, and unwind map `DF80F0`.
Its sole state0 action `CBF6F0` forms the actual local record at `EBP-18` and
jumps to `B483F0`. If old release fails, count/new-pointer publication remains,
metadata is unwritten, and the temporary reference is released by that guard.
The normal final temporary release occurs after disarming; a failure there
does not invoke the guard again. Source uses an armed local cleanup without
a catch/rethrow boundary. A second C++ exception during cleanup terminates
during exception search, matching native FH3 behavior.

Base constructor registers handler `CBF778`, `FuncInfo DF8194`, map `DF818C`,
and action `CBF770`: owner at `EBP-10` goes to `BD30F0`, which writes `CEB130`.
`BF7CD1` uses native SEH4 setup `C07C00/C07C45` with map `E02D50`; its finally
`BF7D1E` calls `BF7C10` for the already-constructed prefix only if incomplete.
The fixed `B47640` leaf does not throw on valid writable storage. That cleanup
is audited, but no invented constructor callback failure is injected or
claimed as source-equivalent execution. Access violations/asynchronous faults
and fault recovery are outside the source contract.

## Validation and primary integration

The ignored focused fixture rechecks 23 complete byte spans against live
Ghidra in `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and
the installed PE before executing a private relocated image. The spans total
1,147 bytes, including all 383 owned bytes, both native FH3 handler/map chains,
the constructor iterator and its SEH4 machinery, and actual profile bytes.
The installed executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

All four owned bodies execute original instructions. Relocation preserves
original `B483F0`, `BD30E0`, `BD30F0`, iterator, and cleanup code. The CPU
deleting terminal adapts the actual receiver/flags into the existing
production CPU owner implementation. Real Win32 critical-section imports
are observed and forwarded; selected pool-entry throws exercise cleanup.
Original CRT FH3/SEH4 dispatch enters host runtime support with native maps.
The audit checks all loaded postimages against only the declared relocations
and boundary jumps. Code and maps are protected after setup.

Seven native/source comparisons matched 6,540 normalized DWORDs: record/base
initialization and untouched tails; changed-pointer release with current-field
mutation; identical pointer; null source; negative-count metadata/reference
alias; old-release exception with temporary zero-reference deletion; and
stride wrapping/current-stride alias plus a negative signed count. Paired
traces include complete owner, three actual CPU declarations, free-index
table/count, and pool-lock state at each observation. Native append's FH3
handler was observed once in search and once in unwind. Isolated native and
source second-cleanup failures both exited91 with identical partial field,
reference, profile, lock, and `uncaught_exceptions()==1` state.

The new source and fixture compile with MSVC Win32 `/W4 /WX /O2 /Oy- /EHsc
/fp:strict`. Link-map checks establish production providers for all four
entries, `B483F0`, `B48CA0`, and shared free. `scripts/build.ps1` passed after
all eight native seed checks; both existing CTests passed. No permanent tests
were added. Primary integration registered the source in `bsp_core` and reran
the unchanged fixture against all four providers from that library: all 6,540
DWORDs and both exit91 terminal states matched. The linked library is archived
with its hash. Ghidra now contains the exact 23-byte `B47640` leaf and all four
saved names/evidence comments. B48A00/B47D60 ledger upgrades retain the prior
shared-pointer interfaces separately. Refreshed exports and old annotations
are recorded in the audit; the primary strict build and both CTests pass.

The detailed report is `reports/native_hardware_layout_fields_audit.json`.
This is reconstructed, strict-compiled and fixture-tested behavior on bounded
actual storage. There is no game, renderer/driver, visual, concurrent-mutation,
or original binary ABI validation.
