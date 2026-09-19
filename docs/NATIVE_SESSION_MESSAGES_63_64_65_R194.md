# Session messages 63, 64 and 65 (R194)

## Reconstructed scope

Eighteen complete normal game bodies (1,191 bytes) provide the constructors,
destructors, predicates, writers, readers and scalar destructors for factory
types 63, 64 and 65. Three translated five-slot profiles support their actual
virtual-call argument placement. Names are descriptive hypotheses.

| Type | Constructor | Destructor | Predicate | Writer | Reader | Scalar destructor | Profile / allocation |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 63 | 00763720 | 00763790 | 007637A0 | 007637C0 | 00763870 | 00763910 | 00D03568 / 30h |
| 64 | 00763930 | 007639A0 | 007639B0 | 007639D0 | 00763A80 | 00763B20 | 00D0357C / 30h |
| 65 | 0075B130 | 0075B150 | 0075B160 | 0075B180 | 0075B1C0 | 0075B200 | 00D02EAC / 20h |

The full stream factory `00768530` allocates 30h, 30h and 20h bytes respectively
and invokes these constructors. Its remaining arms and complete composition
remain open. This packet records the three raw constructor-call instructions
separately from ownership-checked calls inside the reconstructed methods.

## Storage and behavior

Types 63 and 64 have the established 18h base, signed DWORD +18h, Boolean byte
+1Ch, retained bytes +1Dh..+1Fh, three float words +20h/+24h/+28h, WORD handle
+2Ch and retained WORD +2Eh. The constructor initializes the base and leaves
the entire payload and base padding untouched. It first publishes base profile
`00D02C68`, sets the fixed type, reads the current game pointer once, and selects
owner slot `game+18CCh+index*4` only for signed index 0..7 from `game+18ECh`.
It then sets delivery=1 and the concrete profile. There are no native calls
inside these inlined 99-byte constructors.

Their writers emit the mutable type byte (8 bits), signed field (4 bits), and
Boolean. The Boolean is reloaded after its write. If nonzero, the low 12 bits
of the handle follow. Otherwise, the three position floats follow through
the actual numeric provider, with signed=1, zero=0, width=32 and maximum
`00D7A248`. Each float call reloads the maximum through x87 FLD/FSTP, then
loads/stores the value through x87 before calling `004295C0`.

Readers receive an 18h stream wrapper and operate on its cursor at +4. They
read the type, signed 4-bit field and Boolean, then the corresponding branch.
The handle branch leaves position storage untouched; the position branch
leaves the handle untouched. Each numeric read reloads its scale through
x87 and calls `004293F0`. No extra payload initialization is introduced.

Type 65 has a DWORD at +18h, flag byte +1Ch and retained bytes +1Dh..+1Fh.
Its 30-byte constructor initializes fields +8/+Ch, selected owner=null,
**type=0**, delivery=1 and profile `00D02EAC`. It neither reads the game nor
assigns type 65. Writer and reader transfer the mutable type in 8 bits,
the DWORD in 11 unsigned bits and the Boolean. A later read can supply wire
type 65; the constructor's zero is preserved.

All three predicates compare the full DWORD query against the fixed class
type or the zero-extended current type byte. The fixed comparison precedes
receiver access. There is no unconditional match for type 54.

The seven-byte destructors only stamp root profile `00CE4974`. Scalar
destructors stamp that same root, free through the actual allocation backend
if flags bit 0 is set, and return the captured object address. Their native
entry is ECX=this, stack flags, RET 4. Writers, readers and predicates also
use ECX=this and one stack argument with RET 4; constructors return EAX=this.

## Providers and evidence boundaries

Numeric contexts and the maximum-value cell are explicitly borrowed by the
source profiles, after the native five-slot prefix. They must outlive the
messages. The base, bit cursor, numeric conversion and allocation routines
are existing reconstructed providers. No new globals, unresolved-call stubs,
library implementations or runtime fallbacks are invented.

Evidence uses `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, with live bytes checked against the original PE.
Nine missing functions were defined under the shared write lock. The three
scalar destructors' returning-free flows were checked and repaired where
needed. Previous names/comments are preserved, annotations saved, exports
refreshed, and a snapshot/index refresh records the new definitions.

The strict Win32 build and three existing CTests pass. The focused original
comparison adds 5,222 cases through actual raw profiles: 84 constructor and
destructor cases, 768 mutable-type predicate cases, 4,352 wire-record cases,
and 18 scalar cases. It covers seven owner indices, four fill patterns, all
eight bit alignments, signed/unsigned width boundaries, noncanonical true
bytes, both payload branches, floating-point edge values, four x87 rounding
modes and both conversion selectors. It compares complete initialized
storage, cursor advancement, wire bytes and masked x87/MXCSR exception flags.
The inherited 1,183-case type-55 provider fixture and its source-only cleanup
fault remain part of the same run. Exact final counts, hashes and merged-build
receipts are pinned in the report.

The combined run passes **6,405 pairs and 3,847,477 matching bytes**. The final
corpus contains 20,764 live/PE bytes and 537 relocations. All 37 owned method
calls pass ownership checks. Factory calls `00768C7E`, `00768C9E` and
`00768CBE` are separately confirmed to have no stored Ghidra function ownership.

These checks establish the tested normal-call domain. They do not prove
arbitrary overlapping message/cursor storage, concurrent global mutation,
unmasked floating-point faults, original CRT/FH3 identity, full binary ABI,
the complete stream factory, startup/network composition or gameplay.

Evidence: `reports/native_session_messages63_64_65_r194.json`.
