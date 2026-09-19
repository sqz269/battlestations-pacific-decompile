# Native session messages 21 and 22 — R184

Addresses: 00764340, 007643A0, 007643C0, 009064F0, 00906800; 007643E0, 00764450, 00764470, 00906AF0, 00906C20. Parent fragment: 00768530. Existing producer inspected read-only: 0090EDE0.

## Result

Reconstruct ten complete normal bodies (2,219 native bytes), two actual five-slot profiles, and all subtype branches of their inline records. Source composes existing signed/unsigned field, string and numeric codecs. The full factory remains a dependency fragment. Names and field labels are descriptive hypotheses, not recovered symbols.

| Tag | Size/profile | Constructor | Predicate | Writer | Reader | Scalar |
| --- | --- | --- | --- | --- | --- | --- |
| 21 | 190h/D035B8 | 764340,94B | 7643A0,13B | 9064F0,716B | 906800,684B | 7643C0,31B |
| 22 | 138h/D035CC | 7643E0,99B | 764450,13B | 906AF0,261B | 906C20,277B | 764470,31B |

Each native profile orders scalar delete, write, read, fixed type predicate and always-true4499C0. Native ECX carries the object. Constructors have no stack arguments, RET and EAX identity. Other methods take one DWORD/pointer stack argument and RET4. Writers receive a raw10h cursor; readers receive the18h wrapper and use cursor+4. Source virtual adapters preserve this placement. Type21 adds borrowed numeric/maximum bindings after its five slots as SOURCE-ONLY profile metadata.

## Construction and producer evidence

Both constructors initialize the18h base using one E188A8 game capture and signed18EC index0..7 selecting owner14 from18CC, otherwise null. They retain padding11..13, then set mode1 and their final profile. Type21 additionally writes subject1C=-1 after its profile; all other payload remains untouched. Type22 retains its complete payload, including counts and tail padding.

The existing named award producer0090EDE0 has a verified768-byte body. At0090EEA0 it constructs type21 at stack+24, then writes subtype20 at+18, EBP at+1C, a stack argument at+188, and a NUL-terminated string at+7C before passing the record to772AF0. This independently confirms the string/value branch layout; no change to the producer or its downstream dispatch is claimed.

Both predicates compare the full DWORD query with their fixed tag, independent of the mutable wire typebyte. Both scalar methods stamp actual root profileCE4974, free only if flags bit0 is set and return identity. These records own no new heap payload.

## Type21 wire branches

The header always writes type8, subject1C signed6 and subtype18 signed8. The writer reloads and dispatches on the full DWORD subtype. The reader decodes subtype into a local signed DWORD, stores it, then dispatches on that captured value. A writer subtype256 therefore emits a zero low byte but takes its default branch; no artificial symmetric round-trip behavior is introduced.

| Subtype | Payload after the22-bit header |
| --- | --- |
| 0 | signed fields2C:3,30:4,34:4,78:5,68:3,6C:3,74:4,70:4; seven signed32 fields38..50; numeric32 fields28,20,24; four signed6 fields54..60; signed32 field64 |
| 1 | numeric32 field28 |
| 2..10,19,20 | byte-length string7C, then signed32 field188 |
| 11..14 | signed6 field180, signed3 field17C, signed6 field184, signed32 field188 |
| 15 | signed6 field180, numeric32 field18C |
| 16 | signed6 field180, signed3 field17C, signed32 field188 |
| 17 | signed6 field180, signed32 field188 |
| 18 | signed32 field50 |
| 21..23 | byte-length string7C |
| other full DWORD | no additional payload |

Numeric calls use flags0/1, scale from actual D7A248 and32bits. Each writer captures scale then payload with FLD/FSTP before invoking the existing codec. Readers capture only scale and decode directly into the raw float word. Consequently raw wire signaling-NaN payloads remain raw on read while typed writer loads can quiet them. The existing codec's precision, rounding and CRT-selector behavior is retained.

String storage begins7C and occupies256bytes through17B; subsequent words are17C/180/184/188/18C. The writer scans to the actual terminator and sends length modulo256 plus that many leading bytes. The reader consumes the complete prefixed byte count, including embedded NULs, then appends a terminator. No strlen-based reader truncation is added.

## Type22 wire branches and captured count

Header: type8 plus subtype18 signed4. Writer dispatch again uses the full reloaded DWORD; reader dispatch uses the sign-extended local decoded value. Subtypes0..5 carry a byte-length string at1C. Subtype7 carries the low3 bits of byte134. All other values except6 carry no further payload.

Subtype6 first serializes six DWORD fields11C..130 as signed8 values. The writer then reloads and sums their FULL DWORD values in native order12C,11C,130,120,124,128 with32-bit wrap. If the signed result is positive it writes that many bytes starting at1C, sign-extending each source byte before the unsigned DWORD writer emits8bits.

The reader decodes all six signed8 fields, then sums them in native order128,124,130,120,12C,11C. It captures that sum once. For each positive-count iteration it reads an unsigned DWORD of8bits into a local and stores its low byte at1C+index. The count is not reloaded after payload stores.

Neither path clamps the count to256 or to the nominal138h record. A long read overwrites the decoded count fields and can continue beyond the nominal record. Source retains these accesses using explicit native address arithmetic and a captured count. Its interface requires sufficient physical backing. The fixture provides owned extended backing rather than allowing out-of-allocation access. Writer full-DWORD sums and reader signed-byte sums may differ; both behaviors are preserved.

## Ghidra and native bytes

12,488 live Ghidra/PE bytes match, including all new/support bodies, full7,528-byte factory, full768-byte award producer, profiles/constants, native CRT conversion block and184bytes of dispatch tables. Type21 uses two36-byte pointer tables plus24-byte subtype maps; type22 uses two32-byte pointer tables. All34 targets were checked against reviewed new-body instruction starts before relocation, and both type21 subtype maps match.

Six absent functions were defined under the write lock. Both reader bodies cover their complete verified ranges, and both scalar listings have zero returning-free gaps. Ten bodies and one factory dependency fragment are named/commented/saved with prior values retained and affected exports refreshed.

130 CALL/tail-JMP rows pass exact stored ownership checks, including producer0090EEA0->00764340. Factory calls007689C1->00764340(size190h) and007689E4->007643E0(size138h) have verified live instructions/PE bytes but absent stored Ghidra ownership. They remain separate raw evidence; no full factory repair claim.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. One ignored local probe executes copied, relocated original bodies including their actual switch tables and compares source record, backing, cursor, virtual and floating-point observations. Reader inputs are generated by the verified original writer on both sides, with focused raw-NaN and embedded-NUL wire modifications.

12,276 pairs match66,679,616 bytes:56 constructors,2,160 general record cases,240 count cases,9,792 numeric cases,24 embedded-NUL cases and4 freeing scalar returns. All subtype branches/defaults and cursor alignments are covered, including full-DWORD versus truncated subtype distinctions, signed field boundaries, lengths0/1/31/255/256/511, nonzero backing, declared writer length0 with physical backing, integer wrap, positive counts through762, and count-field overwrite without changing the captured loop bound.

Numeric comparisons cross x87 precision fields0/2/3, four rounding modes and both CRT modes with zeros, subnormals, finite boundaries, infinities and signed quiet/signaling NaNs. x87 exception flags, stack balance, control word and MXCSR status/control are compared. Constructor padding and untouched payload are retained. Original pre-free stamps are observed before actual singleton free; freed storage is never inspected.

The report pins source/object hashes and immutable tested/integrated artifacts. No permanent tests were added. These checks do not establish arbitrary aliasing/concurrency, unmasked FP traps, original FH3/SEH or CRT identity, whole binary ABI, network exchange or gameplay. The full award producer was added to read-only byte evidence after the fixture passed; the copied fixture input was hash-checked unchanged.

## Follow-up packets

Continue remaining factory messages with constructors75E000(size50h) and766060(size24h), then the20h record initialized by75DAA0 with tag25. Preserve each codec/lifetime dependency before composing full768530, packet recorder787850 and the network workers. Ordinary startup and gameplay remain open.
