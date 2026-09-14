# Native loading queue work storage

This packet implements eight storage bodies in
[`native_loading_queue_work_items.cpp`](../src/native_loading_queue_work_items.cpp)
against actual Win32 headers. It provides queue pointer reserve/resize, work
record removal/reserve/resize, job storage destruction, job construction and
enqueue. It does not enable the loader state machine or worker thread.
Descriptive source names are hypotheses rather than recovered symbols.

Baseline: `96d910d50a2e98317bb45227d830544ef1727a3d`. Exact live/installed
spans, native calls, compiler cleanup tables and validation provenance are in
[`native_loading_queue_work_items_bl.json`](../reports/native_loading_queue_work_items_bl.json).

## Native storage and ABI

| Entry | Actual input / output | Behavior |
|---|---|---|
| `004FB310` | ECX pointer-vector header, stack signed capacity; RET4 | Clamp to at least one; allocate, copy, free, publish pointer/capacity |
| `004FB4B0` | ECX pointer-vector header, stack signed count; RET4 | Reserve as needed, zero added slots; shrinking never destroys jobs |
| `00501670` | ECX work-vector header, stack signed index; RET4 | Shift names and raw fields; release last name, then decrement count |
| `005018A0` | ECX work-vector header, stack signed capacity; RET4 | Allocate replacement; deep-copy names and raw fields; release old names forward and free old array |
| `005019D0` | ECX work-vector header, stack signed count; RET4 | Zero four DWORDs per added record; decrement count before each reverse-order tail release |
| `005051A0` | ECX job; RET | Release package name, destroy work names, free work array |
| `00505530` | ECX job, stack source name header; EAX job, RET4 | Initialize actual 24h job and copy package name |
| `005055C0` | ECX loader, stack source name header; RET4 | Allocate/construct job, grow when count equals capacity, append pointer |

A vector is `0Ch`: pointer `+0`, signed count `+4`, signed capacity `+8`.
The actual loader contains its job pointer-vector at `+10h`; its count is
`+14h` and capacity `+18h`. Each work record is `10h`, containing an owning
name length/pointer at `+0/+4`, raw callback context at `+8`, and raw result
at `+0Ch`. There are no resource retains/releases in these storage routines.

The `24h` job contains state DWORD `+0`, stop **byte** `+4`, untouched padding
`+5..+7`, work-vector `+8/+0C/+10`, owning package-name header `+14/+18`,
raw result callback `+1C`, and raw FileBlock `+20`. Construction writes the
callback/FileBlock zeros only after copying the name. Job storage destruction
does not operate on those last two fields or free the outer job allocation.
Released string and array pointers remain in the corresponding headers.

The C++ functions accept actual storage and a borrowed
`NativeStringRawPoolContext`. The source interface is not the original
thiscall or FH3/SEH binary ABI. Signed comparisons and wrapping DWORD byte
calculations are retained; no validity, range, capacity-overflow or null-owner
checks absent from the native code are introduced. Callers must supply valid
storage and valid removal indices.

## Allocation and cleanup evidence

Pointer reserve allocates `capacity*4`, reads the current source pointer/count
while copying, frees the current old pointer, then publishes replacement
pointer followed by capacity. Work reserve allocates `capacity*10h` before
copying any string; current source headers and raw fields are reloaded after
string resize. The explicit copy uses the current destination length after
testing current source length. Host `memmove` preserves the existing native
`BF7680` backward-overlap behavior. Zero-length copies are omitted under the
repository's existing C++ string contract.

Normal allocation uses the existing `singleton_lifetime_allocate/free`
services (`BF55BE` thunk to `BF681B`; free `BF6989`/`BF65AC`). Native and host
byte counts are identical. These services preserve malloc/new-handler retry
behavior while retaining the host CRT exception/handler identity boundary.
String storage resolves the current actual `00419CC0` pool publication on
each allocation/return through existing `0041DD40`, `BD1120` and `BD1510`.
No cached pool or semantic host string is introduced.

The four native FH3 `FuncInfo` records have magic `19930522`, `maxState=1`,
zero try blocks and one unwind-map entry from state 0 to -1:

| Body | Registration / FuncInfo / map | State 0 cleanup |
|---|---|---|
| `5018A0` | `C68CF7 / D91FC0 / D91FB8` | `C68CE0` passes the partially constructed placement and calls `401130`, a one-byte RET |
| `5051A0` | `C6918B / D925A0 / D92598` | `C69180` calls `504E10(job+8)` |
| `505530` | `C691EB / D92658 / D92650` | `C691E0` calls `504E10(job+8)` |
| `5055C0` | `C6920B / D92684 / D9267C` | `C69200` frees the allocated outer job with `BF65AC` |

Thus a work reserve failure during name copy has no effective local rollback:
it does not destroy previously copied names or free the replacement block.
After copying, state -1 also leaves old-name release failures without a local
cleanup. The implementation deliberately does not add owning temporary arrays.

The constructor's active cleanup owns the work-vector, while the package name
is still being constructed. Its cleanup does not destroy the partially copied
package name. The job destructor activates the same work-vector cleanup during
package-name release, then deactivates it **before** normal work resize/free.
The private `destroy_work_vector` composition is the exact `504E10` schedule:
`5019D0(vector,0)` followed by free of the current array pointer. It is not a
separate public reconstruction or ownership claim for that dependency.

Enqueue frees its outer allocation only if job construction unwinds. It
deactivates that cleanup before pointer-vector reserve; a later reserve failure
leaves the constructed job allocated, matching the original. Scope cleanup
preserves C++ unwind ordering; original FH3 runtime identity and SEH fault
handling are not supplied by this source interface.

## Listing membership and validation

The initial and final worker observations found decoded, installed/live-matching
returning-free continuations `004FB361..004FB369` and
`005019A4..005019B2` outside their enclosing Ghidra bodies. Their assembly
publishes new array/capacity and restores registers. Both are included in the
source and exact span evidence; the integrator owns the subsequent repair.
The internal `50196D..50196F` gap is unreachable alignment, not missing logic.

The dependency `504E10` initially omitted `504E22..504E26` after its free.
The integrator repaired and saved that body during this packet; final worker
membership confirms the continuation. The enqueue unwind funclet still has
decoded `C69209 POP ECX; C6920A RET` outside its saved body after the free call.
Its whole live/PE byte span is recorded. Worker Ghidra access was read-only,
using `bsp.py ghidra` target verification with autostart disabled.

The new source and one ignored fixture compiled as MSVC Win32 with
`/std:c++17 /EHsc /W4 /WX /permissive- /MD /O2`; the probe links with an embedded
manifest. It used a frozen, hash-checked `bsp_core.lib` and a real constructed
`8AD4A0h` native string pool. The fixture passed all eight APIs, untouched job
padding, raw context/result preservation, independent string copies, enqueue
growth, pointer/work grow/shrink, middle removal, header-preserving destruction,
and initial reserve allocation failure with unchanged existing storage.

The exact source, fixture, frozen library and evidence hashes are recorded in
the report. There is no throwing-string-getter/reentrant-mutation fixture;
those cleanup and reload schedules are supported by the listing and compiler
tables. CMake and shared runtime registration belong to the integrator.
There is no original-instruction differential, drop-in ABI, FileBlock/resource
loading, live worker or gameplay validation claim.

The next production work still needs the actual loader owner/current-front
callback, update/lifecycle routines, FileBlock behavior and resource dispatch.
The work-attachment path `504D20/4FE700/5048F0` is outside this packet.


## Correction from docs/NATIVE_LOADING_QUEUE_INTEGRATION_BL.md

Primary integration at4c83a760 registered this TU and passed the complete Win32 build and both existing CTests.4FB310 and5018A0 now own38 and104 reachable instructions through their returning-free continuations. Unreachable3-byte alignment50196D..50196F remains excluded.504E10 andC69200 own their complete10 and5 instructions respectively. Four source FH3 handlers are defined; all31 direct call rows pass. Earlier hole observations and frozen-library fixture evidence above are preserved as historical records.
