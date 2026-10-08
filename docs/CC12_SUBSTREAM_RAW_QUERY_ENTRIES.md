# Raw adopted-substream query entries

Addresses: 00BF1080, 00BF10A0, 00BF10B0.

The three explicitly named `raw_*` functions in
`include/bsp/native_adopted_substream.hpp` add complete original register entry
ABIs to the existing production module. The ordinary position, length and
origin C++ interfaces, constructor, dispatch and all other behavior remain
unchanged. Names describe recovered behavior and are reconstruction hypotheses.

| Original | New Source entry | Coverage | Original ABI | Bytes |
| --- | --- | --- | --- | --- |
| 00BF1080 | `raw_position_native_adopted_substream_00bf1080` | complete | ECX backing; EDX:EAX result; RET | 13 |
| 00BF10A0 | `raw_length_native_adopted_substream_00bf10a0` | complete | ECX backing; EDX:EAX result; RET | 13 |
| 00BF10B0 | `raw_origin_native_adopted_substream_00bf10b0` | complete | ECX backing; stack origin; EDX:EAX result; RET4 | 40 |

The MSVC Win32 definitions are naked `__fastcall` functions. Their first
argument is the actual ECX receiver and their second argument occupies unused
incoming EDX. Only origin has a third argument, occupying the original stack
DWORD. No facade delegation, cache, validation branch, default caller value,
profile lookup, global, call or relocation is added to these bodies.

BF1080 loads current-low at +20h, subtracts start-low at +10h, loads
current-high at +24h and subtracts start-high at +14h with borrow. BF10A0
uses end-low/high at +18h/+1Ch in the same SUB/MOV/SBB order. Results wrap
modulo 2^64. BF10B0 returns start for origin zero, current for one, and end
for every other 32-bit value, including high-bit values. Each origin branch
has its own original RET4. The producer and existing layout are documented in
`docs/NATIVE_ADOPTED_SUBSTREAM.md`; BF1130's current Source body writes these
same six offsets.

The admitted domain is stable actual backing of at least 28h bytes with
genuinely live aligned DWORDs at +10h, +14h, +18h, +1Ch, +20h and +24h.
Every call reads those fields freshly. This is memory-domain leaf admission:
it does not establish an Original class, original vtable, D68DB0 profile,
adopted source, construction history, reference ownership, object lifetime,
archive integration or game/world call path. Fault behavior and concurrent
mutation are outside the qualification. These entries alone are not a
drop-in class replacement.

## Evidence and focused native comparison

Read-only Ghidra queries verified project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base against the current
configuration pointing to `C:/Users/sqz269/bsp.gpr`. Live body boundaries and
assembly were inspected because the pseudocode has register inputs. The
unmodified installed PE and live Ghidra spans agree for all 66 bytes.

The single new ignored component family is
`local/cc12_substream_raw_queries_20261008a/`. Its successful `fresh06` build
contains exactly three translation units: current production
`native_adopted_substream.cpp`, genuine current `singleton_lifetime.cpp`, and
the unique `probe.cpp`. It links zero BSP archives. Current production files,
actual consumed project and toolchain headers, compiler/linker backends,
recipe/parser tools, Original image, and searched system libraries are pinned
before and after the run. Frozen source copies equal the current production
inputs. The fixture uses no replacement production dispatcher or free stub.

Whole COFF function spans, their unique linked-image occurrences and the
Original bytes are compared before any Original query executes. Each body
matches literally, with no relocation masking or byte patches. All three
loaded Source bodies are checked again against the full spans before Original
code is copied unchanged to executable storage. These position-independent
Original leaves need no instruction relocation. No Original constructor,
vtable, profile dispatch or CRT/FH3 handler executes.

The fixture declares an aligned typed ten-DWORD backing, which makes all six
queried fields live, inside a 48h aggregate including two 10h guards. The first
four backing DWORDs contain arbitrary sentinels and carry no class meaning.
It mutates the same backing through four states: ordinary subtraction,
low-word borrow with 64-bit underflow, high-bit words, and another in-place
mutation. For each state it compares position, length and origins 0, 1, 2,
FFFFFFFF and 80000000: 28 Original/Source comparisons and 56 instrumented
raw calls. Incoming EDX differs between Source and Original. Each call checks
EDX:EAX, actual ECX receiver, ESP/RET cleanup, EBX/ESI/EDI/EBP, and unchanged
complete backing plus both guards. Twelve additional typed Source calls
exercise the declared fastcall signatures. Executable spans are unchanged
afterward.

The existing 98-comparison family was located in
`J:/PROG/battlestations-pacific-decompile-orch2-20260910/local/native-adopted-substream`.
All 74 report-listed artifacts there were preserved with pre/post hashes;
none were replayed or overwritten. The new family's earlier attempts stopped
before Original execution: `fresh01` before compilation because the historical
family was absent from the main checkout; `fresh02` after compilation because
the header catalog omitted extensionless standard headers; `fresh03` after
linking because the map parser initially treated repeated discarded section
pseudo-symbols as duplicate functions; `fresh04` after linking because the
PE parser returned an mmap that needed conversion to bytes for occurrence
counting; `fresh05` at the probe's initial gate because Windows text output
had converted the required LF marker to CRLF. The marker is now emitted as
literal bytes. The corrected gate requires unique membership for every selected
real function. All attempts remain available; only `fresh06` executed Original
leaves.

The production Win32 build and three existing CTests are recorded in
`reports/cc12_substream_raw_queries.json`. The initial build passed with two
registered tests; this fresh worktree lacked its generated native reference
header. The existing read-only `verify-seeds` command checked eight live/disk
spans and generated that worktree-local header, with its evidence directed
inside this ignored family. The incremental normal build then ran all three
existing tests. No new tracked tests, CMake changes,
shared metadata edits or Ghidra mutations are part of this packet. These
checks establish the complete three raw entry bodies and their stated
memory-domain ABI; startup, gameplay and complete class compatibility remain
unvalidated by this packet.

Primary integration review independently rehashed 359 admitted input pins and
64 worker artifacts, checked the whole 66-byte worker COFF/unique linked Original
bodies, and confirmed the integrated Source files match the frozen inputs.
The primary normal Release Win32 build passed all three existing CTests; its
production `native_adopted_substream.obj` also contains all 66 literal Original
bytes with zero relocations. No native fixture was replayed. These are three
additional raw entry interfaces for already reconstructed functions, not three
new Original functions or a class/startup/gameplay validation claim.
