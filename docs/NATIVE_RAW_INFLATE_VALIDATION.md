# Independent actual raw inflater validation

Addresses: 00bbbdc0, 00bbbdd0, 00bbbe50, 00bbbe10, 00bbbf00, 00bbc060, 00bbc140, 00bbc1c0, 00bbc320, 00bbc3e0

The independent review found no behavioral discrepancy between the ten actual
raw-stream implementations from source commit `62e979c3` and their native
assembly. Strict MSVC Win32 Release compilation and both existing CTests pass.
The focused original/source fixture passes 1,305 checks. This packet adds
verification evidence, not another reconstruction count or game-validation claim.

## Body and ABI review

| Entry | Bytes | Original interface | Review coverage / exercised behavior |
| --- | ---: | --- | --- |
| BBBDC0 | 4 | ECX owner; AL byte+9; RET | complete / nonboolean 82h byte |
| BBBDD0 | 6 | ECX owner; EDX:EAX unsigned length+18; RET | complete / payload length |
| BBBE50 | 6 | ECX owner; EDX:EAX unsigned position+24; RET | complete / initial position |
| BBBE10 | 50 | ECX owner; incidental source-seek EAX; RET | complete / far rewind with stale buffers |
| BBBF00 | 257 | ECX owner; no stable return; RET | complete / short source reads, multiple blocks, zero input capacity |
| BBC060 | 210 | ECX owner; low/high/origin stack; EAX incidental; RET0C | complete / equal, forward, in-buffer and far backward, EOF, wrapping offsets |
| BBC140 | 118 | ECX owner; destination/request/count stack; EAX count pointer; RET0C | complete / zero, partial, cross-block and EOF reads |
| BBC1C0 | 3 | RET0C only | complete / null owner and untouched count output |
| BBC320 | 186 | ECX owner; RET | complete / retained and zero-reference source, callback replacement, resource release order |
| BBC3E0 | 30 | ECX owner; flags stack; EAX captured owner; RET4 | complete / flags 0 and 3 |

The ten bodies total 870 bytes. The consumed actual constructor BBC1D0 contributes
another 334 bytes to original execution; its reconstruction is already recorded
by `NATIVE_MPAK_ENTRY.md`. These counts exclude all CRT/zlib and probe support.
Every copied span was compared with live Ghidra bytes and the installed PE.
Workers performed no Ghidra mutations. The primary repaired the destructor body
to include BBC381..BBC3D9 before the final call-site audit.

The buffer header is begin/end/capacity/current at 0/4/8/C. Constructor fields +4
and +C start equal, so constructor-only evidence cannot distinguish their roles.
The prior constructor document's labels were corrected without changing code.

Refill preserves the native actual-count local initialized by `PUSH ECX` to the
owner bits. The source-read callback observes that initial value and then the
previous count on subsequent reads in the same refill. Reset calls stock zlib
reset, seeks the current source and resets three counters; it retains both
buffer cursors and extents. Seek ignores the high offset word and uses unsigned
wrapping low-word targets. No malformed-input progress guard was added.

Destructor FuncInfo DFE930 points to the single-entry unwind map DFE928. Its
action CC4B00 jumps to BB86E0 and restores only stream/reference base profiles.
Normal destruction clears the current source field after the captured source's
zero-reference callback, then releases the current decoder and captured input
and output buffer owners. Original FH3 exception execution remains outside the
fixture.

## Focused generated-payload fixture

One deterministic 150,123-byte payload compresses to 82,120 bytes of raw DEFLATE.
The file-like source has a 13-byte prefix and caps positive reads at 733 bytes,
forcing input refills and crossing the 64KiB output boundary. Python zlib only
generates the input; both compared sides decode with the fetched stock 1.2.1
library. The generated input and all copied native code are frozen locally.

The fixture compares 365 normalized state words, 854 source-call trace words,
12 allocation-release events and 70,087 output bytes. It independently checks
output against the original uncompressed payload for each meaningful read.
Two stream lifetimes exercise one payload:

- The first reads 37 then 70,000 bytes, rewinds 20 bytes within the current output
  block and reads 33. A far seek to zero preserves stale buffer cursors; the next
  16 bytes equal payload offset 70,050 while logical position advances from zero.
- The second seeks with low `FFFFFFFF` and origin 3 to decoded-length-minus-one,
  reads the final byte, then checks EOF, an absolute `FFFFFFFF` target and a
  zero relative seek. All are bounded by the native EOF condition.

Additional checks exercise zero-length read, equal-position seek, a zero-capacity
input buffer's early refill return, write's untouched output count, nonboolean
open byte 82h, real source reference changes, current source-field clearing after
a terminal callback replaces it, and deletion flags 0/3. All allocations from
the explicit shared test allocation boundary are released in the same order.

The linker map identifies the two actual built source object files and shows
allocation/free bindings in the probe object. Native code runs in private
VirtualAlloc copies with only external call/IAT/version operands redirected.
The probe uses a shared successful CRT allocation contract and stock zlib
adapters on both sides; this validates the game body calls and storage effects,
not the original CRT/zlib machine code or global allocator identity. Exact
source files, objects, linked zlib archive, generated inputs, probe source,
executable and linker map are frozen under `local/raw_inflate_az/verified`.
Immediate headers are recorded; a complete compiler dependency closure is not
claimed.

Compressed entry-to-memory composition will be recorded after the runtime
adapter is available. Original FH3/SEH, allocation or decoder failures,
malformed-input hangs, arbitrary stack/data aliasing, concurrent mutation and
installed archive/gameplay behavior remain unvalidated. No permanent tests were
added.
