# Native bit cursor readers (R176)

Addresses: 00428b80, 00428bb0, 00428c70, 00428c80, 00428cb0, 00428cd0,
00428d10, 00428d30, 00428d70, 00428da0, 00428df0, 00428e30, 00428e50,
00428e90, 00787850, 00768530

## Implemented scope

Fourteen complete normal bodies (715 original bytes) reconstruct the cursor used
by session-message parsing and network packet statistics. Descriptive names are
hypotheses. The new source signatures do not preserve the original binary ABI.

The actual cursor is 10h bytes: backing pointer +0, byte length +4, current pointer
+8, signed bit position +Ch. Native callers pass the cursor in ECX. The packet
recorder constructs it inside a stream at +4; the factory explicitly adds four
at 768552 before passing it to the byte reader and rewind helper.

| Entry | Bytes | Original contract |
| --- | ---: | --- |
| 428B80 | 41 | Stack bit count, RET4. Rewind whole bytes, subtract low three bits, borrow one byte when signed bit position becomes negative. |
| 428BB0 | 184 | Stack destination/count, RET8. Read little-endian bits and advance the existing cursor. |
| 428C70 / 428C80 | 16 / 47 | Zero an output byte, read requested bits, optionally sign-extend using the native byte mask. RET8. |
| 428CB0 / 428CD0 | 18 / 63 | Zero an output WORD, read bits, optionally sign-extend the WORD. RET8. |
| 428D10 / 428D30 | 19 / 51 | Zero an output DWORD, read bits, optionally sign-extend the DWORD. RET8. |
| 428D70 | 36 | Read one bit to a zeroed local byte and store whether it equals one. One output, RET4. |
| 428DA0 | 75 | Read byte length, allocate length+1, publish pointer, read characters, reload output pointer and append NUL. One output, RET4. |
| 428DF0 | 59 | Read byte length and characters into caller storage, append NUL. One output, RET4. |
| 428E30 | 18 | A second zero-WORD/read-bits wrapper, with no arithmetic conversion. RET8. |
| 428E50 | 51 | Stack destination/count/width, RET0C. Unsigned count loop, zero/read each WORD and advance destination by two. |
| 428E90 | 37 | Read 64 bits into local storage before publishing two output DWORDs. One output, RET4. |

## Native details preserved

For full bytes, 428BB0 writes the low shifted byte before advancing the cursor.
If the signed bit position is positive, it reads the following byte and combines
the carry **without consulting the declared length**. For a partial byte it first
writes masked low bits, then reads a carry only if current+1 is below base+length.
It advances by the requested bit count even when that carry is absent. Length is
not an all-purpose bounds check, and this reconstruction adds no clamp.

The implementation preserves byte-width x86 shifts, signed bit-position tests,
DWORD address arithmetic, destination writes before cursor/carry reads, and
zero-count behavior. Typed wrappers clear their entire output width even for
zero bits; the raw reader does not write for zero bits. The 64-bit wrapper retains
its local temporary so an overlapping destination cannot corrupt an ongoing read.

The allocating string reader uses the existing `singleton_lifetime_allocate`
malloc/new-handler loop corresponding to BF55BE -> BF681B. It allocates exactly
length+1 and leaves release to its caller via `singleton_lifetime_free`. Successful
allocation was exercised; the original CRT identity, private exception objects,
allocation failure paths and new-handler behavior were not proved here.

## Parent evidence and Ghidra repair

At 768558 the message factory reads an unsigned type byte; at 768561 it rewinds
eight bits. Constructors and deserializers subsequently process that same type
byte. The full factory and its class-specific dependencies remain unreconstructed.

The recorder's 787920..787969 block constructs a borrowed bitstream and calls the
WORD reader at 787955 for 16 bits and at 787965 for three bits. Its later parsing
uses the full session-message factory. The recorder also uses clock-derived time
buckets and per-direction/type statistics; it is not a generic text-log boundary.
Its complete composition, the network workers and their constructor remain open.

428CD0 was absent as a Ghidra function. Its complete 63-byte body was verified
against the original PE, disassembled and defined under the write lock. The
function ends in RET8 at 428D0C; existing adjacent bodies were preserved. All
fourteen routines and the two parent dependency fragments retain saved comments,
prior annotation values and refreshed exports.

## Validation and limits

The strict MSVC Win32 build and all three existing CTests pass. One local fixture
executes all 715 copied original bytes and compares 2,064 original/source pairs,
with 1,826,760 matching observation bytes. It covers all eight alignments, raw
reads and rewind, signed/unsigned widths including zero, boolean and 64-bit reads,
WORD arrays, declared-end carry behavior, overlapping source/destination bytes,
and both string readers at lengths 0, 1, 5 and 255. Pointer identities and backed
ranges are checked before normalizing the observation stream. Both allocation
paths use the existing actual allocation provider, with original request sizes
checked. No permanent test suite was added.

Evidence includes 817 live/PE-matched bytes and 19 direct call/tail-jump rows.
Tested and integrated archives retain the source, library, executable, two relevant
COFF objects and unchanged fixture. Object comparisons ignore only the COFF build
timestamp. Inputs still require valid native cursor state, suitable destination
widths and sufficient physically backed bytes for the native unchecked accesses.
Original hardware faults, arbitrary cursor-field aliases, binary ABI/FH3/SEH,
ordinary application startup, network exchange and gameplay are not established.
