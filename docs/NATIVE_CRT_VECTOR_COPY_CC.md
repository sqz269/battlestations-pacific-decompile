# Native CRT vector copy source pair CC

This packet implements the complete C0C82B `__VEC_memcpy` dispatcher and
C0C7A4 aligned vector engine in `src/native_crt_vector_copy.cpp`, with the
qualified instruction interfaces in `include/bsp/native_crt_vector_copy.hpp`.
The baseline is published `06e8986477baed33aa46e05c368f65026f4c6df5`.

The dispatcher is 227 bytes / 94 instructions. The engine is 135 physical
bytes: 129 reachable instruction bytes / 35 instructions plus six skipped
padding bytes. All 362 physical bytes were freshly checked against the
installed PE and saved target. The BY discovery commit
`ee7a0e08c90e0f20f876f1dc10d64d6fc3bb01f6` and its complete 93-artifact
inventory were verified twice with SHA256/SHA512. Primary and independent CA
readiness reviews approved this complete pair; their stricter domain
qualifications are incorporated below. Correct CRT library names are retained;
the engine's descriptive C++ name is not a claimed recovered symbol.

## Complete dependency and entry contracts

Both entries are MSVC Win32 naked `__cdecl` functions with three original
stack arguments: destination, source, and DWORD byte count. They install no
C++ frame, CPU detector, flag word, heap, handler, validation or catch. The
pair's only calls are a real E8 call from C0C82B to C0C7A4 and a real E8 recursive
call back to C0C82B. No callback, generic host memcpy/memmove, imported service,
or private writable provider is used.

The dispatcher returns the original destination in EAX. It explicitly
creates its original EBP frame with 1Ch local bytes and saves EBX/ESI/EDI in
those locals. Its two calls push exactly three DWORD arguments and perform
`ADD ESP,0Ch` afterward. It restores EBX/ESI/EDI/EBP and returns with caller
cleanup. ECX/EDX and arithmetic flags are volatile. ESP is not realigned to
16 bytes; SIMD stack spill storage is not introduced.

The engine explicitly creates its original eight-byte local EBP frame,
saves/restores ESI/EDI, and preserves incoming EAX, EDX and EBX. Its C++ return
type is void, which invents no native result. ECX is the loop counter and
ends at zero on valid ordinary completion; XMM0..XMM7 are clobbered. It returns
with caller cleanup. DF is unchanged, while arithmetic flags follow the
original SHR/DEC schedule.

These are low-level native instruction interfaces. All operations require
readable/writable, nonwrapping, forward-safe extents in the actual Win32
execution domain. The dispatcher requires DF=0. Actual CPU/OS SSE2 support
must exist for the vector path. The engine requires actual 16-byte aligned
source/destination and a positive byte count divisible by 128. No synthetic
CPU mode or default initialized state is provided.

## Dispatcher schedule and recursive domain

The dispatcher preserves the native signed 32-bit pointer remainder modulo
16 instruction sequence: CDQ/XOR/SUB/AND/XOR/SUB for each pointer. The source
uses those actual register operations, avoiding an unsigned-alignment or
host-pointer-arithmetic rewrite.

- If both signed remainders are zero, compute tail=count&7Fh. When count is
  larger, call the actual engine with count-tail, then copy the tail forward
  with REP MOVSB. Aligned zero count reaches neither the engine nor a memory
  copy instruction with a nonzero count.
- If signed remainders are equal and nonzero, copy the prefix `16-remainder`
  with REP MOVSB **before** recursively calling the dispatcher on advanced
  pointers and reduced count. The prefix must not exceed the supplied count.
  It is 1..15 bytes for positive signed pointer representations and 17..31
  for negative ones. The recursive call receives aligned pointers and the
  actual remaining DWORD count. A matching low-bit alignment alone does not
  erase the original signed remainder distinction.
- If signed remainders differ, copy forward using REP MOVSD then REP MOVSB.

The original BF87E0 caller's forward direction, count>=256, current feature
word and low-bit-alignment gates remain separate evidence. This source does
not own or reproduce that outer gate. Direct short/equal-misaligned calls that
violate prefix<=count still follow the original instructions; no guard or
safe-copy policy was inserted.

## Engine schedule and padding

The engine computes ECX=count>>7 and enters the loop unconditionally. It loads
four MOVDQA vectors from source+0,10h,20h,30h and stores those four vectors,
then loads/stores four more at +40h,50h,60h,70h. It advances both pointers by
80h, decrements ECX, and loops while nonzero. It does not substitute MOVDQU,
MOVNTDQ, a runtime dispatch API, or scalar fallback.

Zero/sub-128 direct calls execute an initial 128-byte iteration and underflow
the loop count. Misaligned or unavailable SSE2 instructions and inaccessible
memory can fault, and earlier writes remain. Neither entry catches faults,
installs an exception frame, rolls back, or changes original abnormal-return
behavior. The pair must not be executed as a general-purpose checked copy.

The native engine's JMP at +18h skips `8D 9B 00 00 00 00` at +1Ah..+1Fh to its
loop at +20h. The source explicitly emits those six self-LEA padding bytes and
uses a compiled loop label. The padding is retained as physical evidence and
is not claimed to execute. Every local branch retains its native target
within the corresponding compiled body.

## Compiler and validation evidence

The retained current-compiler audit checks the complete 135/227-byte emitted
bodies, their instruction encodings, padding and local labels. The dispatcher's
engine E8 operand is at +4Eh and its recursive E8 operand at +B0h. A COFF REL32
must bind to the actual paired source definition; the recursive target must
be the dispatcher's own entry with the correct symbol/addend. An assembler-
resolved self displacement is accepted only if it proves that same target.
No original numeric code address is embedded as a callable provider.

MSVC 19.51.36244.0 emitted all 135 engine bytes exactly, including the six
skipped bytes. All 227 dispatcher bytes match outside precisely two REL32
operands: +4Eh binds to the engine's section 3/value 0 definition and +B0h
binds to the dispatcher's own section 4/value 0 definition. Both encoded
addends are zero; both transfers remain E8 calls followed by the original
12-byte caller cleanup. There are no other code relocations or differences.
The object has no writable sections or undefined external symbols; both
source symbols have exactly one definition in the unique equal archive member. The report also retains current compiler identity,
strict command/read/write tlogs, exact source/header input binding, resolved
/Fo object path, emitted object, unique equal archive member and sole source
symbol definitions. Writable sections, unresolved external provider symbols,
extra bodies and instruction differences outside reviewed call operands are
rejected by the focused static audit.

The full strict MSVC Win32 build exited 0, all eight native seed comparisons
matched, and both existing CTests passed (reconstructed_math and
native_math_differential). The two direct native call rows also passed the
live call checker. No new test or
execution of these native copy primitives is added. No Ghidra or settings
mutation, shared-owner replacement, merge or push is part of this packet.
Whole-local evidence is inventoried and independently verified twice using
both SHA256 and SHA512; the report is outside that directory to avoid a
self-referential hash.

The remaining BF87E0 scalar/table implementation, canonical 0109EEA4 feature
publication, startup order and native SEH4 probe/frame ownership remain
separate. Exact source instruction and ABI checks do not establish original
runtime placement, full caller ownership, or gameplay validation.
