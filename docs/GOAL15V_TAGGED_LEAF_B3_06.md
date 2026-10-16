# GOAL15V tagged B3 leaves, packet 06

Twenty-two previously deferred whole routines now have complete Source bodies:
123 Native bytes and 183 actual registered Source bytes. Eleven complete
machine entries are exact (41 bytes); eleven complete Source entries retain
explicit qualifications (82 Native / 142 Source bytes). The original inventory
remains 79 unique inspected candidates, rather than 101 distinct inspections.
After this packet it contains 75 complete Source entries / 374 Native bytes,
four deferred entries, and zero uninspected entries.

Names describe observed operations, not recovered symbols, classes or fields.
The old tags, prior five packets and existing library/consumer identities remain
unchanged. `native_tagged_leaf_b3_06.hpp` contains all exact Source prototypes.

| Native entry | Source operation | Native / Source bytes | Coverage and evidence class |
| --- | --- | --- | --- |
| `00B33E50` | read dword at +14h | 4 / 4 | complete; exact entire COMDAT |
| `00B343B0` | store byte at +74h, expose AL value | 10 / 6 | complete; qualified new input interface |
| `00B343E0` | store dword at +84h, expose EAX value | 13 / 9 | complete; qualified new input interface |
| `00B3FF30` | return ECX bits | 3 / 3 | complete; exact entire COMDAT |
| `00B3CCF0` | ECX address bits plus 10h modulo 2^32 | 4 / 4 | complete; exact entire COMDAT |
| `00B344C0` | three dword zero stores | 11 / 23 | complete; qualified first-store fault registers/flags |
| `00B344D0` | three dword zero stores | 11 / 23 | complete; qualified first-store fault registers/flags |
| `00B344E0` | three dword zero stores | 11 / 23 | complete; qualified first-store fault registers/flags |
| `00B344F0` | three dword zero stores | 11 / 23 | complete; qualified first-store fault registers/flags |
| `00B3D110` | three dword zero stores | 11 / 23 | complete; qualified first-store fault registers/flags |
| `00B3C4F0` | empty body | 1 / 3 | complete; equivalent RET 0 return-state, different encoding |
| `00B3C500` | empty body | 1 / 3 | complete; equivalent RET 0 return-state, different encoding |
| `00B3C510` | empty body | 1 / 3 | complete; equivalent RET 0 return-state, different encoding |
| `00B3DC80` | empty body | 1 / 3 | complete; equivalent RET 0 return-state, different encoding |
| `00B3D4C0` | empty body, pop 4 | 3 / 3 | complete; exact entire COMDAT |
| `00B3D4D0` | empty body, pop 4 | 3 / 3 | complete; exact entire COMDAT |
| `00B3DC90` | empty body, pop 4 | 3 / 3 | complete; exact entire COMDAT |
| `00B3D4E0` | zero byte result | 3 / 3 | complete; exact entire COMDAT |
| `00B3D500` | zero byte result | 3 / 3 | complete; exact entire COMDAT |
| `00B3E380` | ADD dword [ECX+4], -1 | 5 / 5 | complete named assembly; exact entire COMDAT |
| `00B3E500` | ADD dword [ECX+4], -1 | 5 / 5 | complete named assembly; exact entire COMDAT |
| `00B3E680` | ADD dword [ECX+4], -1 | 5 / 5 | complete named assembly; exact entire COMDAT |

## Ordinary complete C++ bodies

The getter consumes ECX and reads exactly four bytes at +14h into EAX. Its
memcpy interface needs only that valid stable readable range, without a typed
record or alignment assumption. The identity and LEA entries instead accept
uint32 address bits: no backing or null rule is added for pure arithmetic, and
addition preserves all 32-bit patterns with modulo wrap.

The byte-zero entries execute the actual XOR AL,AL and RET, preserving the full
incoming upper-EAX preimage. Only AL is a Source result. The cleanup-only bodies
execute RET 4 and read no argument cell or receiver. Their new stdcall envelope
chooses one opaque word solely to describe cleanup; a recovered semantic type,
argument value input or class is not claimed. The return address is still read.
The four genuinely empty one-byte RET bodies become three-byte RET 0 under
ordinary void cdecl compilation. Both read the same return word, advance ESP
by four, and preserve registers/flags on valid executable/return-stack backing;
their whole code-fetch extents and bytes differ. They are whole empty bodies,
not unresolved stubs or partially reconstructed functions.

Each zeroer has five actual Native instructions: XOR EAX,EAX, then three MOV
dword stores at ECX+0, +4, +8, and RET. The complete ordinary C++ uses three
four-byte memcpy stores of zero and exposes concrete EAX=0. The actual whole
Source body is MOV dword [ECX],0; XOR EAX,EAX; MOV dword [ECX+4],0;
MOV dword [ECX+8],0; RET. Thus every memory store has the same width, value,
address and order. The receipt checks all three operands independently. There
is no SSE instruction, wide/byte store, read of old backing, guard, alignment
assumption, overstore, typed record or helper. All twelve touched bytes need
writable backing, with no extra nonalias condition. Normal EAX=0, ECX/EDX
preservation and defined XOR flags agree; AF is undefined for XOR in both.
At a failed first store Native has already zeroed EAX and set XOR flags while
Source has incoming EAX/flags; Source performs its XOR after that first store.
The same store footprint does not establish fault/SEH register equivalence or
full Original ABI adoption. This is a complete qualified Source result rather
than an optional byte-emission failure being counted as no implementation.

The two setters consume Original ECX plus the byte/dword at entry ESP+4 and
RET 4. Their concrete AL/EAX retains the stored value, but the Original language
return type is unknown. Source takes meaningful base/value arguments in
ECX/EDX and plain RET. Its actual instructions store DL/EDX first and move
DL/EDX to AL/EAX afterward. Original first loads the value into AL/EAX and then
stores. Under the explicit input-value mapping, the single destination store
and normal low-byte/full-dword result agree for all input bit patterns; the
byte setter preserves upper EAX in both actual bodies. No Original EDX input,
dummy register parameter, class, nonalias rule or value validation is invented.
Original stack-cell read, cleanup, physical stack-alias relations and the
result-register state at a failed store differ from this new interface.

## Exact ADD-memory bodies

The three complete five-byte routines are literal named instructions, not raw
byte arrays: ADD dword ptr [ECX+4], -1; RET. The Source is naked void fastcall
with one actual ECX backing input, no EDX formal or invented result. Ordinary
C++ arithmetic cannot specify the required EFLAGS or guarantee one memory-RMW
instruction and its fault boundary; DEC and split load/store forms are not
substituted. Entire actual strict and registered bodies are 83 41 04 FF C3.

For old uint32 value v, the stored result is `(v-1) modulo 2^32`. ADD(-1) has
CF=(v!=0), AF=((v&15)!=0), OF=(v==80000000h), ZF=(result==0), SF=result bit31,
and PF=even parity of the low result byte. DEC preserves incoming CF and uses
the subtraction borrow rule AF=((v&15)==0); both CF and AF can therefore differ.
The preserved ADD reads/writes the same four-byte operand and keeps the actual
ADD flags and general registers, including at the instruction boundary. It is
unlocked, with no synchronization/atomicity promise. The new Source calling
envelope permits normal volatile flags/registers; these exact fixed bodies
do not prove game use, indirect caller closure, or Original C++ class/ABI identity.

## Caller, coverage and whole-object evidence

Fresh live refs identify six direct sites for the +14h getter:

| Site | Incoming Native evidence |
| --- | --- |
| `00BA6B72 CALL 00B33E50` | ECX from [ESI+274h] |
| `00BA6B87 CALL 00B33E50` | ECX reloaded from [ESI+274h] |
| `00BA6BB1 CALL 00B33E50` | ECX reloaded from [ESI+274h] |
| `00B9E206 CALL 00B33E50` | MOV ECX,ESI precedes JNZ to this site |
| `00BA508D CALL 00B33E50` | ECX from [EBX+274h] |
| `00BA5197 CALL 00B33E50` | ECX reloaded from [EBX+274h] |

The report supplies exact call-site/callee/containing-function rows and saved
live contexts, including the nonfallthrough branch setup. The three containing
ocean functions retain their existing names/Source interfaces; no new binding
is claimed. Seven data refs are verified as installed PE pointer words.
Missing other refs do not prove entries unused. None of these leaves calls
another routine or needs an incomplete provider. Tracked C/C++ coverage
searches, including headers, raw/vendor paths and both short/full hex forms,
found no pre-existing complete address entry. Index NULLs alone were not proof.

Fresh saved-project verification, typed whole bodies, live bytes and installed
PE bytes establish all 22 real Native extents. The receipt separately accounts
for every nonempty executable Source section, symbol offset zero, complete
SizeOfRawData, instructions, relocations, hashes and providers. All 22 entire
strict and actual registered CMake sections agree with each other. Source
lengths are retained for qualified entries; Native-size prefixes are never
substituted. Actual CMake tlog flags and Core archive membership are checked;
the extracted archive member's entire hash equals the actual registered object.
The full archive hash is retained, and standalone flags are labeled separately.
The registered build and both
existing CTest suites are recorded, alongside the incoming-call report checks.
No new routine tests or probes were added.

Before this packet's only registered build, packet05's actual registered
object, full Core archive, CL command/tlog, proofs, strict object and report were
copied under `local/goal15v_tagged_leaf_b3_06/prior05/` with hashes. Its manifest
pins packet05 e0bccf31fe66807ed25e9f014c77bfc4c12300d8. The current packet starts
from pinned Main 3c67d8580, merged with the official append-only conflict resolver.
No later unrelated Main changes were chased. All artifacts remain ignored in
the packet directory, including final timing through commit/release/handoff.

Inventory after packet06: 57 exact entries / 235 Native bytes and 18 qualified
entries / 139 Native bytes, totaling 75 complete entries / 374 Native bytes.
The four remaining deferrals are actual-global readers `00B343F0`/`00B34490`,
the 85-byte SSE stack/overlap copy `00B3D5E0`, and the FLD32/x87 return
`00B3FFA0`. They remain unimplemented by this packet. This completes the bounded
22-entry follow-up, without a full-band, parent adoption, installation, runtime,
Ghidra-write or drop-in replacement claim.
