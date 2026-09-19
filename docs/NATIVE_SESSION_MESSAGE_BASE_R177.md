# Native base messages and header serialization (R177)

Addresses: 00428f50, 00428ff0, 004290b0, 00429120, 00449910, 00449940,
00449960, 00449980, 004499a0, 004499c0, 004499e0, 00759cd0, 0075b430,
0075b480, 0075b4c0, 0075b860, 0075b880, 0075b8a0, 0075b8c0, 00768530

## Scope and layouts

This packet supplies 19 complete normal bodies (709 original bytes), actual
translated base-message profiles, and one message-factory dependency fragment.
Names are descriptive hypotheses. The packet does not bind the full factory or
replace its remaining class constructors and deserializers.

The base message allocation is18h: profile +0, DWORDs +4/+8/+Ch, type byte +10h,
three retained bytes +11h..13h, and selected owner pointer +14h. The factory's
18h allocation and constructor calls establish this layout. The field named
`delivery_04` in C++ retains the observed values; its full semantic role remains
a hypothesis.

Native write methods receive a **raw10h cursor** in their stack argument. Read
methods receive an **18h stream wrapper**, with the cursor at +4. Its final
ownership byte and padding are not changed by these readers. Treating these two
arguments as the same layout would write or read the wrong fields.

| Entry | Bytes | Complete normal behavior |
| --- | ---: | --- |
| 428F50 | 148 | Raw bit writer, ECX cursor, stack input/count, RET8. |
| 428FF0 / 429120 | 18 / 18 | Write requested low bits from a DWORD stack argument, RET8; byte/WORD call sites use the corresponding widths. |
| 4290B0 | 27 | Normalize only the low argument byte to0/1, write one bit, RET4. |
| 449940 / 449960 | 19 / 21 | Write/read message type byte using the distinct cursor/wrapper layouts, RET4. |
| 4499A0 / 4499C0 | 18 / 3 | Compare full DWORD argument to zero-extended type byte; return true unconditionally. Only AL is the native Boolean result. |
| 75B430 | 80 | Base constructor: +4=3, +8/+C=0, profile D02C68, type low byte, selected game owner or null. RET4/EAX message. |
| 449980 / 75B860 / 75B8A0 | 32 each | Full base construction, then profile CE4980/D02EE8/D02EFC and +4=1/0/2 respectively. RET4/EAX message. |
| 449910 / 4499E0 / 759CD0 / 75B880 / 75B8C0 | 31 each | Root/one/base/zero/two scalar deletion: stamp CE4974, free only flags bit0, return captured identity. RET4. |
| 75B480 / 75B4C0 | 53 each | Write/read common21-bit extended header: type8, sender WORD low12, normalized relay1. RET4. |

## Writers and construction

The raw writer ORs low bits into the current byte, advances, reloads the input
byte, and stores its carry in the following byte. This store occurs after every
whole byte, even when aligned and the carry is zero. The input reload matters
when input and output overlap. Partial writes mask the input, OR it into the
current byte, add the bit count and store a carry only when the signed bit
position exceeds seven. Base/length are not consulted, and no capacity clamp or
blanket clear is introduced. Physically writable backing must include the carry.

75B430 writes the message fields before loading current E188A8. It captures that
game pointer once, reads the signed selector at +18ECh, and copies game+18CCh
index0..7 to message+14h; out-of-range selectors store null. The caller supplies
the actual current-game cell. No stand-in game or null-publication fallback is
installed. Constructors preserve bytes11h..13h.

## Operational virtual profiles

CE4974 is a three-slot root table: scalar delete and two original pure-call
entries. The source table binds its scalar and the actual current CRT `_purecall`.
D02C68, CE4980, D02EE8 and D02EFC each have five slots: scalar delete, type writer,
type reader, type equality, and always true. All92 original table bytes were
verified against live Ghidra and the PE. They are separate tables, not an assumed
larger base vtable extending into its neighbor.

The translated five-slot tables contain real implementations. Ignored-EDX
fastcall adapters preserve ECX and the original stack argument/RET placement for
virtual calls. Construction still uses explicit new C++ interfaces for process
bindings. Scalar deletion uses the existing compatible allocation/free provider;
the original CRT/private exception ABI is not claimed. The two pure-call entries
are linked but their fatal path was not executed in the fixture.

The extended-header helpers operate on caller-supplied derived storage through
+1Ah. This packet does not invent the rest of that derived class or its constructor.
The factory's type1/type2 paths use the now-complete zero-mode constructor and
type reader; its complete switch and all other class-specific paths remain open.

## Ghidra and validation

Nine missing functions were byte-verified, explicitly disassembled and defined
under the Ghidra write lock, including the five scalar bodies and virtual type
methods. Their returning-free paths retain the ADD ESP4 and RET4 instructions.
All19 routines and the parent fragment retain prior annotations, saved comments
and refreshed exports. Evidence includes8,583 live/PE bytes, the92 table bytes,
the complete factory parent listing span and25 direct native call sites.

The stored-function verifier passes23 calls. Two factory sites,768592 and768756,
are disassembled CALLs to75B860 but remain outside the factory's stored Ghidra
body. Their bytes and targets match the PE; they are retained separately as raw
parent evidence, without a containment claim. Recreating the factory preserved
its name/comment/signature but did not close these two ownership gaps. The bridge
script endpoint is disabled, and the Computer Use native pipe was unavailable,
so a precise body-range repair remains open. Failed verification and repair
receipts are retained; these two sites are not counted as verified owned calls.

The strict MSVC Win32 build and all three CTests pass. One local copied-original
fixture executes all709 new bytes and254 prior reader bytes. It compares957
original/source pairs and958,952 observation bytes:

- 616 bit writers, including all alignments, zero counts, overlapping buffers,
  ignored declared capacity, carry stores and low-byte Boolean normalization.
- 112 constructors covering signed selector boundaries, valid owner slots,
  type truncation, retained padding and all four field+4 values.
- 96 calls through actual reconstructed/original five-slot tables, covering
  type round trips, full-DWORD equality, true return and retained scalar deletion.
- 128 extended-header round trips across all alignments and sender/relay limits.
- Five freeing scalar paths: successful allocated calls and returned identity
  match. Original pre-free profile/one free call are recorded; source pre-free
  state is checked on the retained branches, not read after release.

Raw profile/return/cursor pointers are checked before normalization. Source/free
calls use the existing actual allocation provider. No permanent test suite was
added. Tested and integrated evidence archives retain unchanged fixture inputs,
four relevant COFF objects and binaries; object comparison ignores only timestamps.

These checks do not establish arbitrary memory faults or aliases into cursor
fields, original allocation failure/EH behavior, whole-module binary compatibility,
full message-factory/packet-recorder/network-worker composition, ordinary startup,
network exchange or gameplay. The continuing runnable-game goal remains open.
