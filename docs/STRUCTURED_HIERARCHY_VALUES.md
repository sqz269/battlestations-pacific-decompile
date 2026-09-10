# Structured hierarchy primitive values

Addresses: 00b932e0, 00b936e0, 00be9a00.

The hierarchy matrix field contains sixteen serialized float32 values, stored
in the same linear order. The sphere field contains four: center X/Y/Z and
radius. The integer helper returns a raw DWORD. None of these readers supplies
an identity matrix, expands a smaller matrix, normalizes values, or validates
the enclosing record's remaining byte count.

The complete bodies of `00b936e0`, `00b932e0`, and `00be9a00` freshly match
the installed executable. Every live batch verified project `bsp`,
`C:/Users/sqz269/bsp.gpr`, and `/battlestationspacific.exe`. Full byte spans,
hashes, ABIs, original comments, and annotation proposals are recorded in
[structured_hierarchy_values_audit.json](../reports/structured_hierarchy_values_audit.json).
The executable SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Matrix: 00b936e0

ABI: ECX points to the node-handle wrapper; EDX points to the destination
float array. There are no stack arguments, and `00b9370f` is a plain RET.

The function preserves ECX in EBX and EDX in ESI, then runs two nested loops
of four iterations each. Every iteration calls scalar node reader `00be99d0`
with the same wrapper, writes ST0 with `FSTP float ptr [ESI]` at `00b936fc`,
and increments the destination pointer by four. Thus serialized slot `i`
overwrites destination float slot `i`, for all sixteen slots `0..15`.
A complete field consumes 64 bytes and fills offsets `+00h..+3Ch`.

There is no twelve-float input, implicit final row/column, transpose, identity
initialization, or arithmetic transform in this body. The nested loop counts
do not establish a mathematical row/column convention. The consumer's
matrix interpretation and coordinate space remain separate evidence.

The earlier hierarchy-record audit found no matrix initialization before
field dispatch. When a `Matrix` field is present, this reader replaces all
sixteen slots; a repeated field overwrites them again in encounter order.
When it is absent, neither this helper nor the inspected caller supplies an
identity default. A typed consumer should represent absent matrix data
explicitly instead of claiming a recovered default.

## Sphere: 00b932e0

ABI: ECX node-handle wrapper, EDX destination, no stack arguments, plain RET.
Four explicit calls to `00be99d0` are followed by `FSTP float32` stores at
destination offsets `+00h`, `+04h`, `+08h`, and `+0Ch`. A complete field
consumes 16 bytes. There is no radius adjustment, normalization, sign check,
or finite-value test.

The existing downstream audit of `00b7d220` establishes these slots as center
X, center Y, center Z, and radius: its box calculation subtracts/adds the
fourth value from/to each of the first three. That interpretation is a
previously established consumer contract, not new arithmetic in this reader.
See [STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md](STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md)
for the caller's initial sphere, sphere-derived box, and overwrite order.

## Integer and precision boundaries

`00be9a00` takes ECX pointing to the node wrapper. It obtains the node,
passes the address of node `+20h` to `00bf0280`, and places node `+8` in ECX
as the shared reader. EAX from that call is returned unchanged by plain RET
at `00be9a0e`. The decompiler's displayed `void` result is incorrect.
The helper performs no signed conversion, index lookup, or flag decoding.

The established `00bf0280` contract calls stream virtual `+34h`, reads a
little-endian DWORD, and debits the actual count from the node budget. On a
complete read this consumes four bytes. `Parent`, `Resource`, and `Flags`
consumers may assign different meanings to the bits; this forwarding helper
does not identify those meanings.

Both float readers use the existing chain `00be99d0` -> `00bf02c0` -> stream
virtual `+44h`. That chain supplies a nonnull actual-count pointer, debits
actual bytes, and rounds through a float32 temporary before returning ST0.
The matrix/sphere stores add an explicit `FSTP float32` into their destination.
Their loops have no early exit for a short read, exhausted budget, or scalar
error. They always perform sixteen/four reads respectively on normal return.

The valid host projection uses complete transfers and the recovered x87
load/store boundary. It must not reuse the font parser's null-count,
zero-seeded short-read behavior: the native nonnull-count path can preserve
pointer bits in unwritten scalar bytes. Unusual x87 control/status and NaN
behavior, malformed-input recovery, native ABI equivalence, and game behavior
remain unproved. This packet changes no C++, tests, shared metadata, or saved
Ghidra analysis; implementation/build/probe results are separate work.
