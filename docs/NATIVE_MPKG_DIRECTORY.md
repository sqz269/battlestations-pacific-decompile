# Actual MPKG directory storage

This packet reconstructs five complete native bodies over the original 34h archive,
0Ch byte reader, 34h parsed header and 24h entry storage. The C++ interfaces in
`native_mpkg_directory.hpp` explicitly borrow string, stream and lifetime services.
The earlier `mpkg_archive.cpp` implementation remains bounded semantic evidence;
its containers and validation policies are not used here. Descriptive names are
hypotheses, not recovered symbols. The machine-readable call evidence and pins are
in `reports/native_mpkg_directory.json`.

| Entry | Inclusive native end | Bytes | Original ABI |
| --- | --- | ---: | --- |
| BB87A0 | BB8841 | 162 | ECX archive; RET; no stable return |
| BB8850 | BB8A8D | 574 | Three stacked arguments: reader, header, kind; RET0Ch; ECX ignored |
| BB9090 | BB90FE | 111 | ECX reader; stacked output/count; EAX output; RET8 |
| BB95B0 | BB96F5 | 326 | ECX archive; stacked reader; RET4 |
| BB9700 | BB97A2 | 163 | ECX archive; RET |

The complete body total is 1,336 bytes. The primary repaired the two false-noreturn
truncations and recreated the stored scanner and loader bodies through their actual
returns. Final readback is BB87A0..BB8841 and BB9700..BB97A2. Initial truncated
exports remain in the ignored evidence folder. The worker made no Ghidra mutation.

## Storage and read order

The archive has pathname length/data at +4/+8, stream at +Ch, low file length at
+10h, entry count at +14h, prefix adjustment at +18h, EOCD offset at +1Ch,
directory size/offset at +20h/+24h, and vector base/count/capacity at +28h/+2Ch/+30h.
Reader +0 is the scratch base, +4 the reported read count, and +8 the current byte.
No additional validity, size, signature or short-read guard is introduced.

BB87A0 captures the current stream and slot30h target for length, stores only its
low DWORD, allocates `min(length,FFFFh)`, then reloads the seek owner, stored length
and slot1Ch. It reloads the read owner and slot24h after seek. The backward search
tests offsets T-4 through 1, never zero; lengths at most four skip the search.
The nearest marker wins. A miss leaves archive+1Ch untouched; a hit subtracts the
current scan distance from the current stored length. The unchecked read's return
is ignored. Allocation/seek/read failures have no added scratch rollback.

BB8850 stores the full kind DWORD. Kind zero consumes 46 bytes; every nonzero kind
consumes 30. Each little-endian word advances the current cursor before reading
its bytes from high address to low address. The parsed header contains kind +0,
signature +4, optional central version +8, words +Ah/+Ch/+Eh, DWORDs
+10h/+14h/+18h/+1Ch, name/extra words +20h/+22h, and central-only words
+24h/+26h/+28h plus DWORDs +2Ch/+30h. Padding +2Ah/+2Bh and absent local fields
remain untouched. Caller BB8A90 supplies nonzero kind; BB95B0 supplies zero.

BB9090 zeroes the actual output string, calls the existing resize with preserve=1,
fills its current nonnull data with spaces using its current length, and copies
the captured requested count. Each byte reloads reader+8 and output+4. The final
cursor advance uses the current cursor. Embedded NULs, high bytes and separators
are preserved, with no filename normalization.

BB95B0 constructs a temporary name, copies it into the local entry and captures
the entry data in EBX after resize. Temporary cleanup precedes the archive-prefix
and parsed-header reloads. Entry projection is: name length/data +0/+4; local
offset +8 = prefix + parsed+30h; flag byte +Ch = 0; provisional data offset +10h
= prefix + parsed+30h + 30 decimal + name length + extra length; method word
+14h; compressed/decoded DWORDs +18h/+1Ch; CRC +20h. Padding +Dh..Fh and
+16h..17h is untouched. The reader skips current extra and comment lengths.
Append invokes the sibling's actual BB9520 vector source at archive+28h. The final
normal return uses the captured entry block but its current length+1.

BB9700 seeks directly to stored directory offset, without prefix adjustment.
It computes wrapping stored length minus current offset, captures the read owner
before allocation, and reloads that captured owner's slot24h after allocation.
The read receives the captured allocation and count plus an uninitialized actual
count slot. Parsing ignores that result and reloads archive entry count after
each append. Scratch cleanup is armed only after read and frees current reader+0.

## Cleanup and dependency boundary

BB95B0 uses FuncInfo DFE440 and map DFE430. State 1 cleans the current temporary
string via CC4828/41DD20, then state 0 cleans the current entry string via
CC4820/BB9030. Normal temporary cleanup lowers state to 0 before calling the pool
getter. After successful append, state becomes -1 before the final captured-block
getter/return. Therefore a getter throw there preserves the appended entry and
does not return the local block. No invented rollback is added.

BB9700 uses FuncInfo DFE46C and map DFE464: state 0 invokes CC4840/BB8570 on the
current scratch reader, then -1. BB8570's stored body ends after the free call at
BB8577; its verified raw continuation is POP ECX at BB8578 and RET at BB8579.
That consumed 10-byte helper is not counted, renamed or claimed as a new routine.
Raw handlers CC4830..CC4839 and CC4848..CC4851 load the respective FuncInfo and
jump to BF6B43; they have no current Ghidra function and remain qualified raw
control-flow evidence with inclusive ends.

The required services preserve actual operator-new/free, current numeric stream
targets and canonical pool getter/return. BB9520 and its entry/vector closure come
from `native_mpkg_entry_vector.cpp`. Existing resize, overlap-capable CRT copy and
CRT fill are reused. No private array, stream, pool, registry or successful fallback
is supplied. Nested `NativeStringStorage::release` remains noexcept; explicit
wrapper cleanup getter scheduling is represented separately.

## Verification

Strict MSVC Win32 `/W4 /WX /fp:strict` build passed with
`MSBUILDDISABLENODEREUSE=1`; both existing CTests passed after seed verification.
The worker build includes the stable sibling vector source through an ignored
CMake include. No repository tests were added.

Ignored `local/mpkg-directory-ax/attempt01` passed five native/source binary pairs
and one source-only exception check, totaling 1,162 assertions. The pairs cover:
two ordered central entries with embedded NUL/high bytes; scanner four-byte and
offset-zero misses; complete central/nonzero-kind local header writes and untouched
padding; a name allocation callback replacing the current reader; and a short read
with archive stream replacement after the loader captures its read owner.

The exception case throws at the final entry pool getter after append. It observes
the owned vector entry, skipped local block return and the outer scratch free.
Remaining pool storage is reclaimed through the actual canonical singleton drain.
All actual observed CRT allocations are freed at teardown. This exception case
executes source cleanup only, never native FH3/SEH.

All five original bodies execute unchanged at +30000000. Eight external direct
targets bridge to actual string/vector/lifetime source or CRT functions; original
owned calls still reach original owned bodies. Numeric D642C0/D15AD8 profile bytes
remain unchanged. Original BEF540/BEF590/BEF600 target addresses bridge to the
actual memory-stream source. A launcher reserves only fixture-owned BE0000,
D10000 and D60000 pages before child CRT startup; the child verifies ownership,
size, type and marker before writing. No game process or unrelated memory changes.

Before the first execution, 553 physical inputs were sealed, including dependency
headers, 125 linked source/object pairs byte-equal to current archive members,
libraries, probe/launcher objects and executables, drivers and 18 native spans
(1,595 bytes). All before hashes were rechecked after execution, and all ten
comparison files plus the source-only result are covered by the after manifest.
The first fixture attempt passed; earlier preparation evidence remains retained.
Relink with `python local/mpkg-directory-ax/run.py --repo <checkout> --attempt
<fresh-absolute-directory>`; an existing attempt is never overwritten.

These are complete logical/source bodies with bounded native composition proof.
They are not original binary ABI, compiler spill identity, FH3/SEH or game proof.
Unchecked malformed memory, zero-byte CRT edge preconditions, simultaneous unwind
failures, other stream profiles, original dependency bodies behind bridges and
unlinked application code are outside the fixture claim.
