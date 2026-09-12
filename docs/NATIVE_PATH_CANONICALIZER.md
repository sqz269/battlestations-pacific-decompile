# Native path canonicalization and discarded temporary lifetime

Addresses: 00BEE390, 00BDB970

These are actual-header C++ reconstructions over the existing native string and
pool services. Names are descriptive hypotheses. The verified target is
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; original installation and
executable bytes are unchanged. `reports/native_path_canonicalizer.json` records
the bytes, CALL sites, original ABI, proof artifacts and remaining boundaries.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| BEE390..BEE512, 387 bytes | Complete algorithm with explicit CRT/service bindings | ECX output header, EDX input header; EAX output; RET |
| BDB970..BDBA04, 149 bytes | Complete normal storage behavior and source exception state ordering | One C-string stack argument; incoming ECX/EDX unused; RET4; no stable result |

The earlier `canonicalize_resource_path_00bee390_fragment` uses `std::string` and
admits ASCII/NUL-free strings. It remains a bounded semantic interface for its
existing callers. This packet supplies the actual eight-byte-header path and
does not silently replace those callers or claim full manager startup.

## Scratch ownership and current inputs

BEE39A reads the initial unsigned length. Below 256, scratch is the native
260-byte stack area. At or above 256, BF55BE receives the wrapping DWORD length
plus one. The input data pointer is read after allocation, so a callback that
changes input storage is observed. Null data selects the byte string at the
supplied actual 0109DB91 address; the installed load image initializes it to zero.

The byte copy stops at the first NUL. BEE3DF then reloads the input length and
captures the end address. Traversal uses that counted end, including after
embedded NUL; later lowercase calls do not cause another input-length read.
Stack bytes beyond the copied NUL remain unspecified. No length consistency
check, ASCII filter, overflow guard or replacement backing policy is added.

Successful completion writes the final NUL and calls the existing actual
41E870 constructor. That constructor clears the output header before copying;
it does not release an earlier output allocation. Input/output identity is
permitted because scratch already contains the input. Heap scratch is released
only after construction returns. Lowercase/output-construction failures retain
heap scratch, as the original body has no cleanup state.

## Cursor and character behavior

Slash and backslash collapse into slash. A leading slash is emitted immediately
and establishes the first parent barrier. At a segment boundary, dot followed
by a separator skips two input bytes. A final dot advances input and retreats
the output cursor by one byte only when it is above the scratch base.

For a parent segment, an output cursor at the barrier appends `../` and advances
the barrier. Otherwise it retreats to the preceding slash or scratch base.
Unresolved leading parents therefore remain; absolute paths are not clamped
to a synthetic root. Ordinary bytes call BF9611 after MOVSX, including negative
values for bytes 80..FF. The caller consumes only the returned low byte.

The output can require one byte beyond the requested heap extent when an
unresolved final `..` expands to `../` plus NUL. The focused fixture supplies
padding and verifies the original request and write at that boundary. Source
keeps the native request and writes; it does not establish safe malformed-input
handling or general memory safety outside storage valid for the native accesses.

BF9611 is retained as a library boundary. Its verified 39-byte body performs
the C-locale fast path when 0109DE1C is zero, otherwise calls BF94FA with the
signed integer and a null locale argument. Runtime services require an explicit
lowercase library function, without substituting an unsigned-byte or ASCII-only
normalizer. Full locale-active BF94FA behavior is not reconstructed here.

## Discard wrapper and cleanup schedule

BDB970 constructs a source header, canonicalizes a separate output header, then
returns source and output allocations in that order. Each nonnull return
captures data first and length-plus-one second, calls the actual pool getter
again, and passes captured block, size and unused value one to BD1510. Native
RET0C consumes those three arguments; they are not arguments of getter419CC0.

FuncInfo E0047C points to the three-state map E00464:

| State | Previous | Action |
| --- | --- | --- |
| 0 | -1 | CC60D0: source header at EBP-1C, tail JMP41DD20 |
| 1 | 0 | CC60D8: output header at EBP-14, tail JMP41DD20 |
| 2 | -1 | CC60D8: output header at EBP-14, tail JMP41DD20 |

Normal code arms state0 after source construction, switches to state2 after
canonicalization, and disarms before output return. Thus canonicalization
failure releases source; a failing source-return getter releases output;
output-return failure has no further cleanup. The source implementation keeps
the getter outside the existing `NativeStringStorage::release` noexcept method
so that this established ordering can execute with a throwing getter. Actual
runtime services reuse the canonical publication/gate/lifetime domain and the
existing BD1510 implementation. No extra pool or cached getter result is used.

## Validation and limits

The strict MSVC Win32 build and both existing CTests pass. One ignored fixture
compares 559 original/source cases: path and alias cases, all byte values under
both lowercase dispatch modes, lengths255/256/257, allocation-time input
replacement, wrapping zero allocation, captured end after callback mutation,
controlled heap contents after embedded NUL, final-parent expansion, and
discard-wrapper event order. All 536 owned bytes stay unchanged at a common
translation. A separate relocated 39-byte original BF9611 body stays unchanged.

The locale-active BF94FA callee is instrumented; those cases verify argument,
dispatch and result handling, not native locale parity. The original output
constructor, heap and pool CALL targets bind established source services or
controlled fixture boundaries. Seven separately labelled source exception cases
verify cleanup and retained scratch. Original FH3 exception execution is not
performed. The unchanged original instructions are never used as exception
frames with throwing fixture callbacks.

A separate source composition creates the actual native string pool through the
canonical lifetime manager, demonstrates repeated ring reuse, produces an actual
output header and completes canonical shutdown. It does not run the manager
constructor or game. All 52 source/object/library/probe inputs were frozen after
linking and before execution. There are no new permanent tests. These C++
interfaces are not drop-in binary replacements; full application integration,
original CRT/FH3/SEH identity and gameplay validation remain unclaimed.
