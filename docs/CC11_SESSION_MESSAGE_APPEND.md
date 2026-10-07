# Session message pointer append (0076E470)

The complete method appends a raw message pointer to the actual session's
outgoing pointer buffer. The genuine session constructor places this owner at
game+1EF0 and publishes profile00D039CC, whose+28 data slot points to0076E470.
The three accessed fields are buffer+25C, count+260, capacity+264. They are
different from incoming storage+24C/+250/+254.

## Native evidence and interface

The original body is173 bytes at[0076E470,0076E51D), SHA-256
`f50aa693351298a653c259d97a98b5e03b9fd10b8cf1b7e776eb15877e4562ac`.
Whole live Ghidra and installed-executable bytes match. There are50 linear
instructions, including two skipped alignment instructions occupying8 bytes;
48 instructions are reachable. The primary cleared the erroneous CALL_RETURN
override at0076E4F0 and restored reachable `ADD ESP,4` at0076E4F5..F7 under
the Ghidra write lock. The worker made no Ghidra changes. The original padding
is retained and was not turned into a fabricated execution path.

Original ABI: ECX is the actual session, one raw message pointer is on the
stack, RET4. The Source function takes two explicit cdecl arguments and is
not a drop-in binary replacement. A full owner/class layout is not declared.
The native00D039F4 slot is DATA evidence, not a direct caller instruction;
an actual indirect dispatcher for the slot has not been recovered.

## Preserved behavior

The method captures capacity then reads count. Equality admits growth to
32-bit wrapping `2*capacity+2`, provided that exceeds freshly read capacity.
It publishes capacity before allocation. Multiplication of the slot count by
four saturates toFFFFFFFF on overflow. The fixed existing malloc/new-handler
allocator receives equal native/host byte requests. Failure has no invented
rollback; original private CRT/unwind compatibility remains unproven.

After allocation, it checks current buffer and count, copies pointer words
while reloading current buffer and count on each iteration, then reloads
buffer again for real free. It publishes the captured replacement only after
free returns. Finally it reloads count and buffer, stores the raw argument,
and increments current count with32-bit wrap. The new Source has no operation
callbacks, service defaults, message inspection, or destructor dispatch.

The entire221-byte/59-instruction Source COFF was reviewed, including direct
allocation/free relocations, fresh loads, publication order, and compiler
stack-cookie checks. The fixed original allocation bridge is61 bytes; free
is a5-byte tail jump to the same real service. Their whole bodies were also
reviewed. Exact disassembly and provider identities are in the report and
ignored COFF receipts.

## Connected bounded fixture

One fixture executes `A, B, C, nullptr` against the complete original and
Source, with **151 checks, zero failures**. Both worlds use actual game+1EF0
storage initialized by complete0076EDE0 Source with unmodified concrete
NativeGameEmbeddedStateCalls, actual read-only PE constants, real512-slot
incoming allocation, and real+298 OS lock. Three real heap messages use
complete existing zero/one constructors and their genuine Source profiles.
Game+18EC=-1 selects the existing null-owner constructor branch.

The sequence covers empty growth0-to2, no-growth append, growth2-to6 with
copy/free/publication, and raw null insertion. It compares every inserted
pointer identity/order; count/capacity; exact pointer retention/replacement;
whole game/session bytes outside the three accessed fields; the separate
2048-byte incoming allocation; all28 lock bytes; and every message byte.
It also compares whole normalized constructed owners after every operation.

The RX original copy relocates only two direct four-byte call operands:
allocation at native0076E4A8 and free at0076E4F1. All165 other bytes, including
padding and the returning-free cleanup, are checked before and after. The
native owner profile is never modified. This is original outer-method
execution composed with fixed existing complete Source allocation/free
providers, not an all-original private CRT proof.

Cleanup calls the real Source message profile scalar slots with flags1,
fixed free for the fixture-owned vectors, and complete0041CC80 to release
the real lock through the actual+298 field. The message scalars preserve
count4/capacity6/buffer identity. This retires fixture storage; it does not
claim a complete session destructor or manufacture consumer progress.

## Build and evidence boundaries

Ten compilation units comprise the fixture and nine fresh repository TUs,
using MSVC19.51.36244 Win32, `/O2 /W4 /WX /fp:strict /MD /EHsc`, an embedded
asInvoker manifest, and immutable copies of all three preexisting support
libraries. Compiler `/showIncludes` proves the exact21 repository headers;
all31 actual repository inputs match the immutable snapshot. Input/library
and original-image pins were checked before/after. Earlier broadcast
artifacts and owned Source hashes also remain unchanged.

The report records exact input paths/hashes, original/source/bridge COFF,
two direct native call rows, actual owner-profile data, build/run logs,
and the primary's flow-repair receipt. Failed fixture snapshot01 is retained;
the final verified build uses inputs02. Only four packet files are committed.
The primary owns CMake registration and the full-main build.

Ordinary valid bounded storage is the executable domain. Overflow scheduling
has Source/listing evidence, without huge allocations or failure injection.
There is no invalid-storage, concurrent-mutation, original exception-unwind,
ABI replacement, or game-runtime claim.0076C500 is not executed: its unchanged
slot address still requires genuine consumer progress unavailable from the
currently closed message scalar providers.
