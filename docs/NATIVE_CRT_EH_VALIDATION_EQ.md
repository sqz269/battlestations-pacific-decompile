# Native CRT scope-handler validation and EH3 readiness (EQ)

EQ reconstructs only `_ValidateScopeTableHandlers`, C168A0..C16950 inclusive,
177 bytes. Its actual caller supplies ECX=scope index, EDI=borrowed image base,
and one stacked scope-table pointer. The naked cdecl declaration describes that
original stack word only. Ordinary C++ calls do not establish the raw registers;
no dummy EDX, extra parameter, binding, callback, image owner or wrapper is added.

The full native instruction stream is preserved, using the real accepted EK
`find_native_crt_pe_section_00c16d80` at its two direct call sites. EDI is never
rewritten; the caller chooses the actual image domain. This leaf neither uses
nor replaces the separate fixed-00400000 C16DD0 image/context cycle.

## Native reads and control flow

The prologue saves EBX/EBP, reads the original argument into EBP, clears EAX,
sets EDX=-1, compares the initial ECX with -1 and saves ESI. Initial -1 returns
1 without dereferencing the table or image; the stack argument itself is read.
Otherwise index*12 wraps as original x86 arithmetic and locates an entry of
three words: parent index, filter, handler. Handler is read first, its wrapping
32-bit image-relative address is masked with FFFFF000, and the cached page and
section are checked. A changed page outside the cached section calls the actual
EK provider with image base and page RVA. Null section or absent EXECUTE bit
20000000 returns 0. A null filter is skipped; nonzero filter follows the same
page/section schedule. The parent word is read only after these validations.

The initial cached page FFFFFFFF cannot equal an aligned page RVA, ensuring
that the first nonempty scope obtains a real section before filter cache reads.
Section end uses VirtualAddress+VirtualSize with wrapping addition and unsigned
comparisons. Neither signatures, scope counts, index signs, pointer ranges,
overflow, cycles nor memory protection are additionally validated here. A
malformed table may fault, loop, or read wrapping addresses. The leaf installs
no SEH frame and introduces no catch or rollback.

Successful native return sets EAX=1 and retains flags from the equal comparison
of index with -1. Failure sets EAX=0 using XOR; CF/OF/SF=0, ZF/PF=1, AF undefined.
EBX/EBP/ESI are restored, EDI is preserved, ECX/EDX retain their actual path
effects, and DF is unchanged. Plain RET consumes the original return word;
the caller removes its one argument. Caller-owned raw registers, real borrowed
table/header memory, lifetimes and intended native entry domain are required.

## C16960 remains incomplete

`__ValidateEH3RN`, C16960..C16D01 physical span 930 bytes, is readiness evidence
only. It creates an actual SEH4 frame with C07C90, encoded fixed scope table
E03718 and cookie E15590, writes FS:[0], and reads TIB stack bounds through
FS:[18]. Its guarded cache path and embedded filter/landing entries cannot be
replaced with ordinary C++ exception handling or a relocated synthetic scope.

It also uses process-global validation cache/count/lock/version state at
109E318..109E43B, VirtualQuery via CE2070 and GetVersionExA via CE2298. Accepted
DD already commits and zero-initializes the complete 109E000 page and retains
its actual process lifetime, and supplies the E15590 cookie page. That storage
coverage does not implement C16960's cache behavior or its native scope/frame
domain. No new cache owner is required merely because these fields lack typed
accessors. No C16960 source, dispatcher/destructor implementation, fixed-image
mapping, cache shadow, spin-lock redesign or game adoption is included.

Its native return values include 0, 1 and FFFFFFFF; VirtualQuery failure takes
the success path. Memory/query failure and cache-lock behavior must be retained
by a future complete reconstruction. Detailed original-byte/provider/path
evidence and blockers accompany the EQ report.

## Evidence boundary

Source reconstruction, current strict Win32 build, existing eight seed checks
and two math CTests, native/object/archive comparison and a never-executed
forced-link provider image are separate evidence categories. No new test is
added. Original/native/helper/exception/game/forced-image execution and actual
native caller adoption are not claimed. The base 70056465 was under primary
integration validation when this packet began; this packet does not assert
publication or gameplay validation of that base.

The final build guard covers exactly 2,704 files: recursive `.cpp`, `.c`, `.h`,
`.hpp`, `.cmake` and `.ps1` files under `src/`, `include/`, `cmake/`, `scripts/`,
plus root `CMakeLists.txt`, including the new EQ source/header. Both complete
before/after inventories match. Current compiler command/read/Fo records are
unique for each selected unit. All write records are retained: EQ has its
original shared write group and a final single-source group after a documented
comment/LF qualification rebuild; both bind the same exact object path. EK has
one shared group. No older matching write group is silently discarded.
