# Native render pointer arrays

`native_render_pointer_arrays.hpp/.cpp` implement seven complete pointer-array
functions over the caller's actual Win32 header. The header is exactly 12 bytes:
raw data pointer at00, signed count at04, signed capacity at08. Four-byte cells
contain borrowed raw identities. There is no second vector, count, allocator
field, owner wrapper, default initialization, or implicit cleanup.

| Native address; exclusive end | Bytes | C++ operation |
| --- | ---: | --- |
| `00B1C660..00B1C6BF` | 95 | `reserve_native_render_group_pointers_00b1c660` |
| `00B1C7C0..00B1C810` | 80 | `resize_native_render_group_pointers_00b1c7c0` |
| `00B1CBE0..00B1CC19` | 57 | `append_native_render_group_pointer_00b1cbe0` |
| `00B1D1F0..00B1D207` | 23 | `destroy_native_ordered_group_pointers_00b1d1f0` |
| `00B1D260..00B1D277` | 23 | `destroy_native_indexed_group_pointers_00b1d260` |
| `00B1C6C0..00B1C71F` | 95 | `reserve_native_render_command_pointers_00b1c6c0` |
| `00B1CC80..00B1CCD0` | 80 | `resize_native_render_command_pointers_00b1cc80` |

Original reserve/resize ABI: ECX=actual header, signed requested capacity/count
on the stack, RET4. Append takes the address of a pointer cell on the stack,
RET4. Destructors use ECX=header, RET. No EAX result is claimed for these bodies.
Names describe observed roles and remain hypotheses. Layout assertions and new
C++ functions are not drop-in native ABI entry points.

Reserve clamps its request to at least1 and returns without changing storage
when current capacity is sufficient. Otherwise it allocates four bytes per new
cell, copies the current live prefix in ascending order, frees the old pointer
buffer, publishes the new data pointer, then capacity. Count remains unchanged.
The copy loop retains current data/count reads. New excess capacity is unwritten;
native minimum1 is distinct from the batch entry array's minimum256. Shared
`singleton_lifetime_allocate` with `SingletonAllocationKind::pointer_slots` and
`singleton_lifetime_free` provide the existing `BF55BE`/`BF6989` allocator boundary.

Resize reserves exactly the requested count when needed. It nulls only the newly
exposed cells, decrements count when shrinking, then assigns the requested count.
Shrinking preserves stale cells and capacity. It does not destroy or release a
pointed group/command. Both destructor entries perform group resize0 then free
the pointer buffer. Count is zero afterward, but the native data/capacity words
remain unchanged, including the dangling pointer. The buffer is dead and must
not be reused or freed again until the caller establishes fresh storage.

Append grows only when count equals capacity, with new capacity max(1,2*capacity).
After reserve it reads the current destination address, obtains the input
pointer value, publishes the cell, then increments count. The source parameter
is a raw address of a four-byte pointer cell; `memcpy` reads that representation
without introducing a C++ pointer-type alias violation. An alias of the header's
own data word therefore sees the **new buffer address**, not the freed old one.
An input cell inside storage freed by reserve is invalid; the implementation
does not add a temporary value capture to make that unsupported case work.

The normal valid domain has nonnegative count/capacity, count<=capacity, valid
allocated spans, a nonnegative resize request, and signed32-fitting four-byte
allocation products and append capacity doubling. Negative reserve requests
are valid and take the minimum1 path. Addressed storage and the input cell must
survive allocator callbacks. Arbitrary concurrent mutation, arithmetic overflow,
malformed headers and null destination-slot wraparound are outside the domain;
no new clamps or corruption recovery are introduced.

Reserve/resize/append can propagate the shared allocator's exception. In the
valid domain, failure occurs before this operation publishes a replacement or
changes count. External allocator/new-handler side effects are not rolled back.
After successful allocation, valid raw pointer copies and the shared nonthrowing
free add no throwing step requiring an invented cleanup. Valid destructors do
not allocate because resize0 does not exceed a nonnegative capacity. No forced
allocation failure or native SEH/C++ exception dispatch was tested. The native
bodies have no local EH frames; caller cleanup policies remain separate.

## Evidence and verification

Every fresh guarded CLI read checked `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Seven complete code spans and two five-byte external
hook entries total 463 bytes, all matching installed executable SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The scalar-free continuations at `B1D202` and `B1D272` contain ADD ESP4, POP ESI,
RET; live Ghidra currently ends those functions before these bytes. The audit
preserves old names/comments/prototypes and proposed evidence updates for the
primary integrator. This worker made no Ghidra or ledger changes.

One ignored original/host trajectory uses actual 12-byte headers, all seven native
bodies, and the same real shared allocator. Only `BF55BE` and `BF6989` have
five-byte boundary jump hooks. The sparse image has no absolute relocations,
other pages stay inaccessible, and unused committed bytes contain INT3. There
is no full PE loader, game entry point, owner-terminal stub, or copied private
array state. The fixture's trace vector records observations only.

Fifteen phases cover negative reserve/minimum1, reserve without growth,
reallocation with header-data alias input, copied live prefix, retained stale
cells, selective growth initialization, ignored cells outside count, command
array growth, and both destructor entries. All 98 normalized observation words
match. Fresh uninitialized excess capacity has no deterministic value; the
fixture seeds those cells after allocation to check subsequent preservation.
It does not claim the original contents of newly allocated excess bytes.
After free it checks only the surviving header and never dereferences the dead
buffer. Raw borrowed identities and self-buffer pointers are normalized; this
is not absolute allocation-address equality.

The new source compiled under MSVC Win32 C++17 `/W4 /WX /EHsc /fp:strict /O2`
in the focused fixture. All eight seed byte checks and both existing repository
tests passed. Initial repository linking exposed the already-known missing
renderer-parameter registration; primary commit `48ebd1a`, cherry-picked as
`29c72e4`, resolved it before the successful `scripts/build.ps1` rerun. The primary
integrator owns registration of this new source in CMake. No tracked test was
added; local fixture/build scripts and logs are hashed in the audit.

This packet establishes actual pointer storage operations. Complete `0x44`
command construction/destruction, `0x4C` group ownership, retained scene/model/
target lifetimes, the `0x34` queue singleton and renderer publication remain
separate. No game or visual behavior was validated by this packet.
