# Native save compression and block framing
Addresses: 00BD48F0, 00BD49B0, 00BD4A70, 00BD4860, 00BC9610

`archive_compression.cpp` implements the archive sink behind the text writer's
null-FILE route. It uses the existing linked zlib 1.2.1 implementation and actual
owned byte buffers. `ArchiveCompressionState` projects manager+50Ch scratch/file
storage, +510h block descriptors, +520h used length and +524h mode. The storage
backend owns setup/reset and the separate write phase at +508h. These fields are
evidence labels, not the new C++ structure's layout.

`00bd49b0` is `__thiscall(ECX=manager, NativeString*)`, `RET4` at00bd4a65. It
appends the explicit string length, including embedded NUL bytes. A sum strictly
below64KB stays buffered. A sum equal to or above64KB fills the current block,
compresses it exactly once, then copies the remainder. It does not loop over
arbitrarily large inputs. A remainder exactly64KB stays pending until another
append or finalization; even an empty append flushes that full pending buffer.

`00bd48f0` is `ECX=manager, RET` at00bd49a8. Assembly at00bd4901..4934 establishes
the capacity calculation `n - trunc(n * -0.0010000000474974513) + 12`. The exact
double at00d62c70 has bitsBF50624DE0000000. The x87 control word is temporarily
changed for integer truncation and restored before allocation. The routine calls
the stock `compress` at00bc9610 with destination inECX, output length pointer
inEDX, source and size on the stack. It copies the produced bytes into a second
allocation, frees the temporary, appends the pointer/length pair through00bd4860,
then resets+520h. Correct library names such as `compress` remain intact.

`00bd4a70` is `ECX=manager, RET` at00bd4be4. It flushes nonempty pending input,
frees the scratch allocation, totals every compressed length plus four header
bytes, and allocates the final buffer. Each block is laid out as a little-endian
32-bit compressed length followed by the complete zlib stream. It XORs the first
six bytes of each framed block with `C7 04 0F 48 FE 4C`: four length bytes and
the first two zlib bytes. Remaining compressed bytes are unchanged. It frees
each copied block, erases the descriptors while retaining vector capacity, then
writes mode2. An empty stream produces no compressed block.

The `00bd4860` evidence is a value-pair append fragment, represented by an actual
owned C++ vector. Native checked-vector iterators, allocator bookkeeping and SEH
are not reconstructed by that fragment. The two complete buffer routines and
finalizer use new interfaces; they are not binary replacements.

The host rejects scratch state outside0..64KB, an append remainder larger than
64KB, impossible framing sizes and zlib failure. Native code can overrun storage
or ignore codec failure in those cases; valid native inputs retain the recovered
flow and bytes. The host does not claim matching native allocation failure/SEH
behavior or x87 status flags.

Both compressor and finalizer have misleading saved Ghidra function extents
after `_free`. Their verified raw instruction ranges are00bd48f0..00bd49a9 and
00bd4a70..00bd4be5, with exclusive ends. Tail disassembly and returning-call
overrides were saved, but the stored bodies remain incomplete. See
`reports/archive_compression_flow_repair.json` and the prior
`reports/archive_buffer_flow_repair.json`. The C++ reconstruction uses the full
disk-verified assembly; no complete stored-body repair is claimed.

Win32 Release and both existing CTests passed. One ignored native-file fixture
decoded the existing `save/0/player` and `save/0/quick` payloads, then recompressed
them through this implementation. The resulting1,689 and1,772 bytes matched the
original game files exactly. Original hashes were rechecked unchanged. The same
fixture exercised the exact-boundary and single-flush cases; an independent
Python zlib decoder recovered four blocks of65,536/65,536/65,536/17 bytes from
the generated container. These are codec/container artifact checks, not proof
that the rebuilt game accepts a newly generated profile or runs gameplay.
