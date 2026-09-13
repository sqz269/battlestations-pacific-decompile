# Native hierarchy field cleanup

This packet reconstructs the complete290 bytes at `B7D640/B7D7D0/B88180` in
`native_resource_hierarchy_fields.hpp/.cpp`. It supplies numeric-reference
array reserve/resize and hierarchy field destruction through the existing
allocation/free and actual raw string-pool services. It does not construct,
append, return or delete a hierarchy record's88h slab slot.

The [report](../reports/native_resource_hierarchy_fields.json) records exact
live/installed-PE byte matches, original ABIs, every direct call and incoming
call, name-only unwind metadata, previous Ghidra names/comments, and validation.
The original binary remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live analysis batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe` through the repository CLI. Ghidra stayed read-only.

| Source entry | Complete native span | Bytes | Original ABI | Coverage |
| --- | --- | ---: | --- | --- |
| `reserve_native_hierarchy_reference_array_00b7d640` | `B7D640-B7D69E` | 95 | ECX0Ch header; signed capacity stack; RET4 | Complete |
| `resize_native_hierarchy_reference_array_00b7d7d0` | `B7D7D0-B7D81F` | 80 | ECX0Ch header; signed count stack; RET4 | Complete |
| `destroy_native_hierarchy_fields_00b88180` | `B88180-B881F2` | 115 | ECX record; no stack arguments; RET; FH3 | Complete body; source C++ cleanup, not native FH3 ABI |

## Numeric-reference array schedule

The native header has DWORD data/count/capacity at0/4/8. It belongs at
hierarchy record+4C and holds four-byte numeric Resource values. Producer
`B7EB90` supplies values from `BE9A00`; no pointee retain/release belongs here.
The separate resource pointer-array helpers have minimum capacity16. These
numeric helpers preserve their own minimum capacity8.

Reserve captures and clamps the signed requested capacity, compares current
signed capacity, and allocates `capacity*4` with DWORD wrap. The concrete
`singleton_lifetime_allocate` implements the existing new-handler service
reached by `BF55BE/BF681B`; no allocation callback is added. It reads current
count after allocation, then copies forward while rereading count and data.
Only a null computed destination skips a store. It frees the current old data
through `singleton_lifetime_free`, then publishes the captured new data and
capacity. Count and unused slots are unchanged.

The saved Ghidra listing omits the returning-free continuation at `B7D691-699`:
`ADD ESP,4; MOV [ESI],EBX; MOV [ESI+8],EDI; POP EBX`. These nine bytes match
the installed PE. Source includes the publication/restore semantics; the
integrator owns any Ghidra flow repair and annotation/export refresh.

Resize reserves only above current signed capacity. Starting from current
count after reserve, it zeroes each computed added slot if nonnull, reloading
data each time. It then compares requested/current count again, decrements the
actual count while shrinking, and stores the requested count unconditionally.
Removed values receive no cleanup. Negative counts/capacities are not clamped;
raw address/index arithmetic wraps as a DWORD. No overflow guard, snapshot
container, bulk copy, unused-slot initialization or added rollback is introduced.

All callers were checked: reserve is reached at `B7D7DE`, `B7ECCB`, `B7D9E9`;
resize at `B881AD`; fields destruction at `B884BC` and `B88323`. The report
preserves containing functions and stack/register provenance. Both native
numeric helpers consume exactly one stack word with RET4; allocation/free
consume one cdecl word with caller `ADD ESP,4`.

## Field order and name-only unwind

The record's owned name header is at+4/+8. Its matrix/scalars and hidden
slab ID are untouched. Destruction performs this exact sequence:

1. Arm name cleanup, then resize the numeric array at+4C to0.
2. Reload and free that array's current data. With negative capacity this can
   allocate32 bytes, publish them during reserve, then free them here.
3. Read current name data from+8 **after** array free, then disarm name cleanup.
4. If data is nonnull, read current length+4, capture length+1 with DWORD wrap,
   call the actual string-pool getter and return that captured block/size.

Normal name cleanup calls the concrete `419CC0/BD1510` providers directly to
retain the captured-data/load order. Getter `419CC0` takes no native arguments;
the three prepared words `(data,length+1,1)` belong to `BD1510`, whose RET0C
consumes them. The source provider omits only the native unused third word.
Both name and numeric headers remain stale after their storage is returned.

FH3 info `DFB9C8`, map `DFB9C0`, has one state:0→-1 through `CC2570`. That
funclet loads the saved record, adds4 and tail-calls `41DD20`. Source catch
therefore invokes the **raw** `41DD20` overload on the current name header
only while cleanup is armed, then rethrows. There is no array rollback, second
array free or record-slot return. A normal name-getter exception occurs after
disarming and does not retry name cleanup.

The existing raw `41DD20` body was inspected: it reads data once, skips the
length/getter when null, captures length+1 before resolving the actual pool,
and leaves the header untouched. `NativeStringRawPoolContext` borrows actual
publication`1090AA8`, gate`1090AA4` and manager publication`1090AA0`. Its getter
exceptions escape. The older `NativeStringStorage` overload is `noexcept`
and is deliberately not used for this path.

## Validation and limits

The strict MSVC Win32 Release build and the two existing CTests passed.
All eight native seed spans matched the installed PE, and the report verifier
passed14 direct call rows with0 failures. No
permanent test or framework was added. One ignored local capsule froze the
complete290 original bytes and its baseline executable/trace **before source
creation**. Its three original/source comparisons cover allocation-time
header mutation and post-free publication, negative initial count, and
negative-capacity destruction with name fields changed during the final free.
All three traces match. The source half additionally verifies two C++ exception
cases: allocation failure cleans only the name, and a normal name-getter
failure is not retried. Those two cases were not run through original FH3.

The capsule links a unique archive containing the exact current CMake-produced
candidate object. Its allocation/free/getter/name boundaries are intercepted
only inside the ignored harness to observe scheduling. This isolates the new
bodies; it does not replay the full CRT, allocator, string pool or lifetime
manager. Frozen original files, current source/header, CMake command/object,
archive, executable and traces are hashed. Final commit proof and all evidence
are under `local/hierarchy-fields-bf/` in this worker worktree.

These are new MSVC Win32 source interfaces, with the destructor's actual pool
context passed explicitly. They are not original ECX/RET ABI entry replacements
and do not establish asynchronous fault, native FH3, arbitrary invalid-memory,
runtime resource wiring or gameplay equivalence. No game was run.

## Follow-up boundaries

The distinct actual hierarchy owner`109022C` can reuse existing88h pool
mechanics; its canonical binding and `B87A90/CD82D0/CE0ED0` wrappers remain a
separate ready packet. `B17AF0/B88320` return/flags wrappers, raw producer and
`B87AE0` append ownership, finite primary-item deletion, manager/cache work and
`B88430` integration also remain separate. This packet neither substitutes the
material owner`F8D3E4` nor changes the sibling's pointer-array implementation.
