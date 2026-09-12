# Actual vertex declaration loading

`native_vertex_declaration_loading.cpp` reconstructs the actual CPU declaration
loader, raw slab allocation and element append path. The result occupies `D0h`
bytes inside a `D4h` pool slot and is directly usable by actual declaration
consumers. It is not the semantic `VertexDeclaration` vector or a COM declaration.
The caller supplies the canonical `0108FD38` pool and existing string domain.

| Entry | End exclusive | Bytes | Coverage | Original ABI |
| --- | --- | ---: | --- | --- |
| `00B2DBD0` | `00B2E93B` | 3435 | Complete control flow, explicit external boundaries below | Incoming ECX unused; stacked actual name header and ignored DWORD; EAX owner/null; RET8 |
| `00B47680` | `00B476C3` | 67 | Complete | ECX raw slab; stacked slab index; EAX slab; RET4 |
| `00B47D20` | `00B47D5C` | 60 | Complete | ECX declaration; RET; no semantic result |
| `00B47D90` | `00B47DE4` | 84 | Complete | ECX actual array header; stacked record pointer; RET4 |
| `00B48330` | `00B4839E` | 110 | Complete | ECX declaration; stacked type/usage/offset; RET0Ch |
| `00B48560` | `00B4869C` | 316 | Complete | ECX actual initialized pool; EAX raw slot; RET |
| `00B488C0` | `00B488CA` | 10 | Complete | Incoming ECX ignored; MOV ECX,108FD38; tail JMP B48560 |
| `00CE0C10` | `00CE0C24` | 20 | Complete shutdown flow through existing string boundary | No arguments; reverse iterator over17 type records; RET |
| `00CE0C30` | `00CE0C44` | 20 | Complete shutdown flow through existing string boundary | No arguments; reverse iterator over8 usage records; RET |

There are **4,122 original bytes**. The two shutdown entries have no Ghidra
function definition in the worker snapshot. Their inclusive ends are `CE0C23`
and `CE0C43`; both final instructions are one-byte RET. The raw listing is
bounded before padding. Root integration owns definitions and annotation.

## Storage and allocation

The existing actual constructor/destructor module establishes profile `D61D1C`,
reference word `+04`, packed cursor `+08`, main header `+0C`, fifteen usage headers
`+18+i*0C`, and packed stride `+CC`. No duplicate declaration layout is introduced.

`B47680` is the slab producer: write WORD32 at `+1AC0`, descending WORD free
indices31..0 at `+1A80`, and the supplied slab index at each slot's `+D0`.
It leaves all payload bytes and the two trailing slab padding bytes unchanged.

`B48560` enters the real critical section at pool`+0C` and increments `+24`.
If `+34` is FFFFFFFF it publishes the current slab count, allocates `1AC4h`,
initializes the slab, and grows the pointer table when count equals capacity.
Table capacity becomes `2*capacity+2` **before** allocation; DWORD multiplication
and address arithmetic wrap. Copy uses the live slab count and table pointer.
The returning-free continuation `B485F6 ADD ESP,4` publishes the captured new
table at `B485F9`; older pseudocode omitted this continuation.

Allocation pops a WORD slot index, scans later slabs only when that slab becomes
empty, decrements recursion and leaves the lock. Allocation exceptions preserve
the held lock, published first-free/capacity and any already allocated slab.
No automatic unlock, rollback, pool initialization or allocator-list registration
is added. Final deletion uses the existing `B47950` return routine on this pool.

## Actual append and syntax

`B47D90` grows only on count==capacity, doubles capacity with DWORD wrap, clamps
the signed doubled value to1 when <=1, and copies five DWORDs in native order.
The source record is read after reserve and each source word is read immediately
before its corresponding destination store. It then increments the current count.

`B48330` builds `{offset,type,0,usage,FFFFFFFF}`. FFFFFFFF offset uses cursor`+08`.
Every call adds the live type size to cursor, including explicit offsets. It
appends main first, then usage, and recomputes stride as the sum of type sizes,
not the maximum offset+size. Counts, enums and source/destination storage receive
no new validation. A second-append exception leaves the first append intact.

The decoder copies the counted input through the existing actual-header copy
body, executes the54 native alias tests, and uses the actual native string table
records for eight usage tokens and17 type tokens. It retains native ordering,
case-insensitive CRT comparisons, substring allocation/release and live signed
counts. Type and usage tables are separate borrowed arrays; the installed layout
has a four-byte gap between the type array ending at108D674 and usages108D678.

The native input count need not equal `strlen`. In particular, counted input
containing `PF43.MVFM\0ignored` is accepted: suffix comparison checks whether
lengths are zero, then uses `_stricmp` without a length-equality test. Token
comparisons do require matching recorded lengths. The source does not impose
the semantic decoder's extra NUL prohibition or a host-size cap.

## Ownership and initialization

Guard108D6D8 bits1/2 are set before usage/type construction. On a construction
exception, completed entries unwind in reverse, the failing current entry is
not destroyed, and that table's bit is cleared. Previously completed other
tables survive. Actual `41E870` construction and native string cleanup are reused.
After each table, the supplied BF6FF5 registration boundary receives `CE0C30`
or `CE0C10`; its integer return is ignored. The application's lifetime domain
must route those tokens to the two supplied concrete shutdown routines.

The original FuncInfo at `DF5E48`,29-state map `DF5E6C`, and cleanup actions
`CBD5A0..CBD6E3` establish the exception ownership distinction. Constructor failure
returns its raw slot through `CBD6AA -> B47C00 -> B47950`. After successful
construction the decoder resets to state0: subsequent parsing/append exceptions
destroy the current local string but **leave the declaration allocated**.
Malformed syntax instead calls current virtual`+04` with flags1, without a
reference decrement, then returns null. Suffix temporaries are released first,
followed by declaration deletion and the original local-name buffer.

`NativeVertexDeclarationReference` borrows the same actual `+04` atomic. It neither
constructs nor retains storage. The caller registers exactly one companion into
its existing `NativeRenderActualOwners` domain. At zero, current profile and
slots must be `D61D1C` / `BD30E0,B48CA0`; the companion runs actual scalar deletion
and the same pool return before its retirement callback removes the binding.
There is no private owner map, extra reference word or semantic declaration.

## Verification and limits

All nine complete byte ranges match the installed PE and live Ghidra bytes;
eight existing differential seed ranges also match. The report records native
CALL/JMP sites, exact ends, source/fixture hashes and the correction to B488C0.
The production source compiled with MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`.
The existing repository build and both existing CTests pass. The initial fresh
parallel build hit a CMake `generate.stamp` timestamp-access failure; an unchanged
retry passed. CMake registration of this new module remains the integrator's
task; the private fixture compiled this production source directly.

One private fixture compares **319,960 bytes across10 original-caller checkpoints**:
all54 aliases, all17 types/eight usages, mixed case, invalid syntax, counted NUL
input, explicit-offset append, repeated usage, seven allocation-failure positions,
four-slab growth/free/reuse and allocation failure while the pool lock remains
held. It separately verifies canonical actual`+04` retain/release, scalar deletion,
slot return and companion retirement. Scratch paths and build flags are in the
report. No permanent test target was added.

The fixture executes the seven owned loader/pool/append bodies plus original
reserve/count code, with265 preimage-checked absolute relocations. Existing actual
string operations and declaration constructor/destructor are explicit source
callee adapters; CRT allocation/free, memcpy/stricmp, atexit and Win32 imports
are instrumented boundaries. A registered host trampoline enters the original
29-state exception handler. The five-byte original virtual-table lookup before
syntax-failure deletion is replaced by the existing actual deletion boundary,
so this fixture does not prove arbitrary native virtual dispatch. Shutdown loop
implementations run as source on both sides, and are byte-reviewed, not separate
original-runtime comparisons.

New C++ interfaces are not binary replacements. Native CRT identity/locale,
throwing string release or atexit, arbitrary changed profiles, hardware-fault
unwinding, secondary-cleanup-exception search, malicious stack aliases, concurrent
global mutation, pool startup/shutdown and gameplay are not established. The
source exposes unbound registration/profile errors explicitly. No executable
frame or renderer behavior is inferred from these focused storage comparisons.
