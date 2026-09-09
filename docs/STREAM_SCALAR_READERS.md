# Binary stream scalar readers

Read-only audit on 2026-09-09. Each live batch verified project `bsp` and program
`/battlestationspacific.exe`. No code, metadata or Ghidra annotations changed.
This resolves the scalar-reading boundary in FONT_GLYPH_SOURCE.

## Concrete virtual targets

Live memory-stream vtable00d642c0 and physical-file vtable00d691b0 contain exactly
the same four scalar-reader targets:

| Slot | Address | Requested bytes | Result |
| --- | --- | --- | --- |
| +38h | 00be4300 | 4 | DWORD in EAX |
| +3Ch | 00be4340 | 2 | Word in AX |
| +40h | 00be4320 | 2 | Word in AX |
| +44h | 00be4360 | 4 | `FLD DWORD PTR` result in x87 ST0 |

The methods use ECX as stream and take one stack argument, RET4. They obtain
stream virtual+24h and call it with destination, requested byte count and the
original argument. That argument is an **optional actual-count pointer**, not
an endian flag. In particular, font loader00ad4c30 passes null to every scalar
read, matching the ordinary stream read contract.

00be4300 was absent from the current function inventory and the normal exporter
stopped there. All four targets were recovered directly from complete raw
assembly and verified against disk. No function was created by this audit.

## Stack buffer and short reads

The readers reuse their incoming argument slot as the destination buffer. The
essential32-bit sequence is:

1. Copy original `[ESP+4]` into EDX (the optional actual-count pointer).
2. Push EDX, push requested4 or2, and pass the address of the original argument
   slot as destination to stream virtual+24h.
3. Load the scalar back from that slot and return.

There is no separate zeroing, retry, byteswap, short-read rejection or read-result
check. Initial buffer bits are the numeric value of the supplied pointer. For
the font parser's null argument, those bits start at zero. A valid underlying
physical/memory stream that writes only transferred bytes therefore leaves
unread result bytes zero on short read; complete EOF produces integer/float zero.
This is a consequence of the original null argument, not a universal stream
policy. With nonnull actual-count pointer, unread bytes retain its original
address bits. A projected interface must not claim generic zero-fill parity
outside the null-pointer call path.

The actual-count pointer is forwarded unchanged, so the underlying stream writes
the count there when provided. It is not consumed as input endian state. Physical
read00bf5030 performs one ReadFile and advances position by actual bytes; memory
read00bef590 clamps requested bytes to remaining length and advances by copied
bytes. These wrappers inherit those effects and add no cursor logic. Native
physical read-error reporting and invalid memory-stream positions remain the
underlying implementation's responsibility.

## Signedness and floating-point fidelity

00be4320 and00be4340 are byte-for-byte identical. Both load only AX, with no
MOVSX or MOVZX. Their nominal signed/unsigned type distinction cannot be inferred
from the body; downstream consumers decide how to extend the returned word.
The font parser explicitly sign-extends header height, but zero-extends the two
scaled glyph metrics. Glyph membership uses the low16 key bits as unsigned.
Do not assign a different endian or sign conversion to the two stream slots.

Integer memory loads preserve x86 little-endian byte order. Float00be4360 does
not reinterpret bytes as an integer and numerically convert: it uses x87 FLD
of the32-bit float stored in the argument slot. The font parser subsequently
uses FSTP float stores into its payload. Ordinary finite float values round-trip;
signaling NaNs, exceptions and control-word-dependent effects need explicit x87
handling before claiming bitwise native parity. The reader has no own FP control
word changes.

## Parser integration boundary

For the font loader's null-count-pointer calls, a narrow scalar adapter can
initialize a four-byte temporary to zero, perform exactly one underlying read
of2 or4 bytes, and decode the resulting low bytes. The unsigned16 key and raw
float fields can then populate the actual DAT-based glyph table, preserving
font-level scaling, duplicate handling and required fallback0091h described in
FONT_GLYPH_SOURCE. A complete-read guard may be useful in a host parser, but it
is an explicit rejection policy absent from these native readers.

Partial count headers can still request records; EOF does not automatically stop
the native outer count loop. Do not equate a safer bounded host decoder with
recovered malformed-input behavior. Native VFS selection, texture ownership and
Lua font registry loading remain separate from these scalar routines.

## Exact byte evidence

All inclusive ranges match the installed executable and live saved program.
The last two ranges contain precisely the four adjacent vtable entries.

| Range | SHA256 |
| --- | --- |
| 00be4300..00be4319 | c7a6ad72a87a8ebe2d36e3aeb3f8bac796457b74b3d9ee5c5bf4e38c92b196bd |
| 00be4320..00be433a | 45f118e1a34545f12c5066b0a15c37bc07a24eafa7b753f04db5e871fd9df8ec |
| 00be4340..00be435a | 45f118e1a34545f12c5066b0a15c37bc07a24eafa7b753f04db5e871fd9df8ec |
| 00be4360..00be4379 | 1129ea810e498622079339887610ba4548f0622b4babec424ef2d1fb92ae02e4 |
| 00d642f8..00d64307 | 30ab01e66d50c5425cc38290ddb269085e91738251518143dd81174a02f09815 |
| 00d691e8..00d691f7 | 30ab01e66d50c5425cc38290ddb269085e91738251518143dd81174a02f09815 |

No build or runtime test was performed. These byte-level findings do not prove
font rendering, malformed-file parity or full stream object ABI compatibility.
