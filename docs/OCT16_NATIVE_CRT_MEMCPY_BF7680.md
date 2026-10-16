# Complete CRT memcpy BF7680

The ordinary `_memcpy` entry at `00BF7680` is reconstructed in its own complete
Source body. The correct library name is retained. There are 711 defined bytes,
247 instructions and 22 typed ranges. Its 869-byte physical span additionally
contains six jump tables totaling 120 bytes and 38 padding bytes. Those data and
padding bytes do not count as reconstructed instructions. Only BF7680 is owned;
vector providers, BF87 comparison, feature storage and constructor remain separate.

Native is cdecl with destination/source/DWORD count at entry ESP+4/+8/+C,
returns original destination in EAX, uses plain RET, and leaves 12-byte cleanup
to the caller. It saves EBP/EDI/ESI, leaves EBX untouched, and requires DF=0.
The Source `copy_native_crt_bytes_00bf7680` adds a fourth borrowed reference to
the actual stable canonical feature cell. Its caller removes 16 bytes. It is a
new Source interface, with no drop-in binary ABI claim.

The body performs the original unsigned destination/source/source+count direction
decision with DWORD arithmetic. Equal pointers still copy; zero follows the real
scalar return path without buffer or feature reads. Alignment prefixes, scalar
DWORD unrolls and byte tails keep their individual load/store order. Backward
bulk copying uses STD, REP MOVSD, then CLD; a fault between them can expose DF=1
and already completed writes. No validation, short-copy repair, catch or rollback
is added.

Only a forward count at least 256 reads the CURRENT actual feature cell. A
nonzero value and matching unsigned pointer low-four bits select the vector
tail. ESI/EDI/EBP are restored before the real provider receives the original
return slot and first three arguments. The Source fourth word remains for its
16-byte-cleanup caller. Scalar returns and the provider return preserve original
destination EAX. No stack realignment or alternate copy service is introduced.

These are the actual Native transfers; support rows confer no extra ownership:

| Actual Native instruction | Scope |
| --- | --- |
| `00BF76C2 JMP 00C0C82B` | Owned whole BF7680 restored-frame vector tail |
| `00C0C878 CALL 00C0C7A4` | Read-only published vector dispatcher to aligned engine |
| `00C0C8DA CALL 00C0C82B` | Read-only published vector dispatcher self-recursion |

The published vector Source is used directly. Its 227-byte/94-instruction
dispatcher computes signed 32-bit pointer remainders modulo 16. Aligned inputs
use positive 128-byte bulk through the actual SSE2 engine, then REP MOVSB tail.
Equal nonzero signed remainders copy prefix 16-r and recurse on advanced pointers.
That prefix is 1..15 for positive representations and 17..31 for negative ones;
the parent count gate supplies enough bytes. Unequal signed remainders use scalar
REP MOVSD/MOVSB. The dispatcher has its original 1Ch frame and three-word calls.
The aligned engine has 129 defined bytes/35 instructions in a 135-byte span,
including six skipped LEA padding bytes. It enters the count>>7 loop without a
short/zero guard: four MOVDQA reads/four stores, then another four reads/stores.
Its caller supplies positive multiples of 128, aligned backing and actual SSE2.

The BF87 comparison was proved across both complete range sets and every
instruction. Exactly 16 indexed table-base operands and one relative vector-tail
operand account for 68 code bytes. Thirty table-target DWORDs account for another
120 bytes; all addresses rebase by 1160h and the vector target itself is the same.
The remaining 681 raw span bytes match exactly: 643 code bytes and 38 padding.
The full 615-line Source has only the owned include/entry/library comment and
internal Native-address-label bijection relative to the fully reviewed family.
It owns all payload instructions and eight proven selector ladders. It does not
delegate to BF87 or a standard-library copy implementation.

All eight selector domains and 38 selector values were checked against actual
BF7680 table words and emitted ladder bytes. The ladders replace 16 indexed
transfers without stack touches or selector changes. They alter arithmetic flags
and omit original jump-table reads; table/code/fault-PC identity remains qualified.

The feature cell is the existing canonical mutable CRT datum exposed by
`GameNativeMutableCrtData::feature_word()`. Its PE virtual-tail zero-fill and
current Ghidra zero are initial image evidence, not runtime flag truth. The caller
must borrow that same cell with valid lifetime and feature truth; this packet
creates no global, default zero, cached feature owner or CPU startup writer.
The identified startup write stays outside this Source packet. Existing vector,
memmove, resize, canonical data and preset constructor files remain untouched.
The constructor remains Source-unauthorized; a future actual-copy caller needs
the explicit feature reference as well as its raw pool context.

Fresh proof verifies the full installed PE hash, project `C:/Users/sqz269/bsp.gpr`
and program `/battlestationspacific.exe`. Owned BF7680 and read-only providers/
comparison cover 1778 defined bytes/623 current starts in 2100 span bytes, with
all 46 encoded edges and four transfers checked. Source ownership remains the
single 711-byte ordinary body.

The default MSVC Win32 `./scripts/build.ps1` exited zero. All three existing
suites passed in 7.58 seconds; no new tests, probes or copy-body execution were
added. The existing unrelated lua_spawn duplicate-symbol LNK4006 remains in the
log. All actual CMake code was reviewed: one 988-byte/316-instruction public
function, eight plain RETs, no helpers or padding. Its sole relocation is code
REL32 at offset42 for the actual vector JMP at offset41. There are no EH sections,
literal/table data, data/debug relocations or generated cleanup paths. All four
sections, directives, compiler/debug/checksum data were read; MSVCRT/OLDNAMES stay.
Every code byte and the symbolic relocation equal the same-build BF87 reference;
the public symbol is mapped explicitly. Object/debug/checksum identities differ.

All six provider/comparison/feature-owner LF hashes match published Main. Prior
95 artifacts and the frozen constructor/closure receipts remain unchanged.
The documented domain requires readable/writable nonwrapping extents outside
active frames, actual selected SSE2 CPU/OS support, DF=0 and stable canonical
feature ownership. Source argument/frame changes, arithmetic flags, Original
table/code/fault PC, ABI, FH3/SEH, heap, runtime and gameplay remain unproved.
Static/build evidence does not establish runtime or binary-replacement identity.
