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

## Compressed entry-to-memory composition

The runtime adapter from `ad60fb260181097ebdc6dcfc528c80e6afe832dc` enables
the existing BEF750 source implementation to consume numeric D64400 owners.
A separate fixture compares the original 415-byte BB5080 caller with its
actual source implementation. Both sides call shared actual constructor,
conversion and destructor support. The ten raw methods were independently
compared above; this is a composition check of the entry caller, not another
independent execution of every original dependency.

The same compressed payload appears twice in an archive-like byte array, at
offsets 13 and 82,140. Three successive entry opens select the first copy, the
second copy and then the first copy again through the exhausted-offset fallback.
The comparison passes 1,468 checks: three entry pairs, 33 state words, 1,023
source-call trace words and 450,369 decoded output bytes. Each output matches
the immutable uncompressed payload, starts at cursor zero and owns independent
memory backing. The underlying source returns with reference count one; after
each returned stream is released, the actual memory counters are zero.

The numeric D64400 table retains its original bytes in private read-only memory.
A private BD30E0 support trampoline invokes the already reconstructed raw
deleting destructor. The actual shared source allocator is linked from the
current core, unlike the explicitly instrumented allocator boundary in the
standalone raw fixture. Distinct fixture type IDs are borrowed through the
existing conversion context; original global descriptor initialization, the
full NativeMpakRuntime service graph and VFS open/name lookup are not claimed.

The first composition attempt failed while reserving the numeric profile page,
before target game-body execution. Its error metadata is retained; the initial
conflicting memory map and binary were not captured before relinking. Linking
the private probe at fixed image base 20000000 allows the required reservations
and passes. This changed only the probe's layout.

## Replaying a selected built checkout

Both local helpers accept `--repo <selected built checkout>` and a **new**
`--output <directory>`. Keep each helper beside its captured `verified` folder:

- `local/raw_inflate_az/replay.py`: exact current raw-stream and constructor
  objects plus stock zlib, with the original 1,305-check expectation.
- `local/compressed_entry_az/replay.py`: nine current source objects verified
  byte-for-byte against their selected core archive members, plus the core and
  stock zlib, with the original 1,468-check expectation.

They preserve captured native bytes, payload, probe logic and output expectations;
they never regenerate expected results from the candidate. Each run records
compiler `/showIncludes`, linker `/MAP` and `/VERBOSE:LIB`, executable results,
source/object/library hashes and input stability. Syntax-only source reads
observe the current include closure without rewriting selected built objects;
the preceding strict build provides their freshness check. Current merged-head
replays captured 224 raw-fixture and 235 composition-fixture include paths.
Exact candidate commits, attempt paths and manifest hashes are in the report.
The combined call-site audit passes all 27 direct rows; seven indirect rows
retain explicit contracts outside that mechanical check.

Original FH3/SEH, allocation or decoder failures, malformed-input hangs,
arbitrary stack/data aliasing, concurrent mutation and installed archive/gameplay
behavior remain unvalidated. No permanent tests were added.
