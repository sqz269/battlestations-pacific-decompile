# Application pointer-vector reserve: original body and Source binding

This read-only packet renews the complete original `00735EC0..00735F1E`
reserve dependency: **95 bytes, 38 instructions**. Fresh Ghidra bytes match the
installed PE, and an independent decode matches every live instruction start.
The current actual-storage Source and the retained resize build preserve the
qualified normal allocation/copy/free/publication behavior. This adds no Source,
Original-function credit, build, test, probe, runtime or Application wiring.

Evidence: [machine-readable report](../reports/cc12_application_pointer_vector_reserve_native_readiness.json).
The worker started from published main `5db1a543115ff82ed6b42be0373ab21452ccfeed`,
which already included the requested constructor-EH publication
`c1893651fd351ea67c5f2bb3151fd06553112d1f`.

## Scope and completeness

Only `00735EC0` and these two output files were leased. Live queries verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 little-endian,
image base `00400000`; initial function count was 64,729. The body passed the
300-byte gate before full pseudocode, listing and bytes were requested.

The prototype endpoint reports a 34-instruction metric, but its body endpoints
span 95 bytes. The complete live listing and independent PE decode both have
38 contiguous instructions, including `00735F11..00735F19` after the returning
free call. The metric is not used as completeness evidence. No listing repair,
annotation, export mutation or other Ghidra change was needed or made.

The body SHA-256 is
`f4ac9801ae9e08b9b64f7a983913dc8e53f3f2afd677f03531e4d00403c9c8b6`.
The report records all bytes, instructions, five internal branch destinations,
two direct service operands and the installed PE hash. Child queries were
prototype metadata only for `00BF55BE` and `00BF6989`; no child body, handler,
caller, or table sweep was inspected.

## Original ABI and exact normal schedule

Let `H` be the actual 12-byte header in incoming `ECX`: data at `H+0`, signed
count at `H+4`, signed capacity at `H+8`. The signed request is the one incoming
stack DWORD at entry `ESP+4`. The body saves `ESI/EDI`, captures `H` in `ESI`
and request in `EDI`, and returns with `RET4`. Growth alone saves/restores
`EBX`; `EBP` is untouched. There is no established semantic `EAX` return.

| Original sites | Operation and visible order |
| --- | --- |
| `00735EC6..00735ECD` | Signed request below one becomes exactly one. |
| `00735ED2..00735ED5` | Read signed current capacity once. If capacity is at least the clamped request, return without reading count/data or invoking services. |
| `00735ED7..00735EE9` | Compute low32(request × 4), push that DWORD, call fixed `00BF55BE`, capture returned `EAX` in `EBX`, zero the index, and caller-clean four bytes. |
| `00735EEC..00735EF1` | First count read is after allocation returns. Signed count at most zero skips the copy loop; otherwise set current destination to captured fresh pointer. |
| `00735EF3..00735EFC` | Test current calculated destination. If nonzero, reload current `H.data`, read one DWORD at low32(data + index × 4), and store it at that destination. |
| `00735EFE..00735F07` | Increment index and destination with 32-bit wrap, reread current count, and repeat while signed index is below that count. |
| `00735F09..00735F11` | Reload CURRENT `H.data`, push it even when null, call fixed `00BF6989`, and caller-clean four bytes. |
| `00735F14` then `00735F16` | After free returns, publish captured fresh data pointer, then captured clamped request as capacity. |
| `00735F19..00735F1E` | Restore saved registers and return with `RET4`. No count-field write or element retirement occurs directly. |

Both service calls use a single stack DWORD with caller cleanup. The first
call consumes `EAX` as a pointer; the second has no consumed result. Metadata
names the first `operator_new` but leaves its prototype undefined; the second
is `_free(void*)` with a cdecl signature. Those names do not prove CRT,
new-handler, allocation-domain or exception identity.

The capacity gate precedes allocation and is not repeated after it. There is
no pre-allocation capture of the old data pointer or count. Allocation-side
changes therefore affect the initial count, each current source read and the
eventual free argument. Free-side changes to count remain; the two subsequent
publications overwrite data and capacity in that order. Copy stores may also
alias the header, so the absence of a direct count store does not mean count
is immutable.

Nonpositive requests are clamped to one and can allocate; request zero is not
a clear/no-op operation. Allocation bytes wrap, including zero bytes when
the clamped request is `0x40000000`. Count need not be bounded by either
capacity or request. No validation, unused-tail zeroing, element reference
operation or malformed-header repair is present.

The destination null test applies each iteration to the calculated address,
not to the allocation as a whole. With fresh pointer zero and a positive count,
iteration zero skips its read/write, but the next destination is four and can
fault. The source pointer is not independently null-tested. Reached extents,
aliases and actual object lifetime remain required qualifications.

## Current Source and retained build binding

The concrete interface remains
`reserve_native_application_pointer_vector_00735ec0(Storage&, int32_t, const Allocation&)`
in [native_shader_preload.hpp](../include/bsp/native_shader_preload.hpp) and
[native_shader_preload.cpp](../src/native_shader_preload.cpp). It borrows the
actual 12-byte header and the actual allocation binding. Source lines 26–40
use signed clamp/capacity comparisons, unsigned wrapping allocation bytes,
live volatile count/data reads, a current destination test, CURRENT-data free,
and data-then-capacity publication. No direct count repair or rollback is added.

The existing [primary resize review](../reports/cc12_native_application_pointer_vector_resize_primary_review.json)
retains the public adapter (19 bytes/8 instructions), private resize body
(80/31), fastcall reserve bridge (20/9), and concrete reserve (113/50).
This audit replayed every selected instruction from the physical retained
objects: **232 bytes/98 instructions total**. Both whole-object hashes, their
selected whole Core-library members, and the full retained Core-library hash
match that review. This is static reuse of its build evidence, not a new build.

The adapter carries the same header and allocation references; the private
resize adds allocation in `EDX`; its bridge pushes the same allocation,
request and header as the three ordinary C++ arguments. The reserve loads
the actual binding's allocation slot at offset zero and later reloads its
free slot at offset four. Its only dynamic calls are those two callbacks.
An empty named-edge graph does not resolve them to Original native services.

All four current Source/header files equal their retained build copies.
All seven frozen build inputs match the before-build manifest. Current
`CMakeLists.txt` has changed since that build; the other six selected inputs
match. Eight canonical pins stored in the primary review were replayed.
The report separately pins eight directly consumed repository files, with
physical/LF hashes, base-commit blobs and six bounded Source/tool excerpts.

The retained reserve is a normal cdecl three-argument function, distinct from
original `ECX`/stack/`RET4`. Its 113 bytes preserve the qualified storage
behavior, but are not byte-identical to the 95-byte original. For example,
the emitted Source reloads loop count before incrementing index/destination,
whereas the Original reloads after those arithmetic instructions. The Source
also reads the callback table, accesses absent from the fixed native calls.
These facts preclude a claim of exact fault, timing, concurrency or register
identity. Correct callable bindings, matching allocation domains and their
lifetimes remain explicit requirements.

## Exception and admission boundary

The complete original body has no local SEH registration or handler setup,
catch, fresh-allocation rollback, or local exceptional cleanup path. It reaches
the final two stores only after both child calls return normally. Allocation
failure/nonreturn prevents the later copy/free/publication; a copy fault may
leave earlier effects; free failure/nonreturn prevents final publication.
Child side effects and caller unwinding remain outside this packet.

The Source likewise adds no catch or `noexcept` promise. Absence of a local
handler is not proof of allocator exception behavior, original FH3/SEH,
fault propagation or cleanup equivalence. Child metadata and ordinary stack
operands do not establish those contracts.

The older R99 report records a 95-byte reserve differential with shared
allocator/free adapters, including allocator/free header mutations. Its body
hash agrees with this fresh capture. That historical probe was neither run nor
renewed here, and its preload/startup/runtime claims are not extended.

The normal reserve dependency is now freshly audited without new admission.
Actual shared Application/vector ownership and lifetime, fixed native service
identity, binary ABI, exceptional propagation, and production construction/
retirement wiring remain separate work. Source, CMake, ledgers and Ghidra were
unchanged; no tests, probes, builds or native executable calls were run.
