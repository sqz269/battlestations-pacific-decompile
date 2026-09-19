# Native session messages 94 through 97 (R202)

Addresses: 0075B060, 0075B090, 007640D0, 00764160, 0075B0C0, 00764200; 0075B100, 0071CC20, 0071CC70, 0071CCA0, 0071CD30, 0071CDC0, 0071E3F0; 0075B0D0, 0071CB50, 0071CB80, 0071CBB0, 0071CBE0, 0071CC10, 0071E3D0; 0075A290, 0075A2C0, 0075C700, 0075C760, 0075CA00; partial factory 00768530.

## Scope and shared header

Twenty-five complete normal game bodies (1,401 bytes) and four five-slot profiles are reconstructed using existing base/header, bit, numeric and allocation providers. No library code was newly ported. Names remain descriptive hypotheses.

| Type | Allocation | Factory constructor | Profile | Payload |
|---|---|---|---|---|
| 94 | 24h | 0075B060 | 00D02E98 | WORD handle20, mode22, flag23 |
| 95 | 30h | 0075B100 | 00CFDA14 | float words20/24/28, presence2C, padding2D..2F |
| 96 | 24h | 0075B0D0 | 00CFDA00 | DWORD value20 |
| 97 | 28h | 0075A290 | 00D02D1C | DWORD values20/24 |

Each factory constructor is 40 bytes: zero fields08/0C, owner14, mutable type10, sender18 and flags1A/1C; set delivery1 and the class profile; retain all payload and padding. They do not read Game. Predicates94/95/96 accept their fixed type plus 89, 73, 70; predicate97 accepts 97, 73, 70 and does not accept 89. Queries use the full DWORD and ignore receiver/mutable type.

Ordinary cleanup bodies94/95/96 stamp rootCE4974 only. All scalar destructors test flag bit0, stamp the root before concrete free and return captured this. There is no separately asserted ordinary native97 destructor: the source helper represents the inline scalar stamp; adjacent0075C7C0 is another writer and is left alone. Slot4 reuses004499C0.

## Message94: optional handle with explicit clearing

Writer007640D0 (142 bytes, ECX=this, raw cursor on stack, RET4) writes common fields: type10/8 bits, sender18/12, booleans1A/1C. It caches presence from full WORD20!=0. If present, it writes boolean22, reloads mode22 and handle20, and writes the handle at12 bits when mode is nonzero or13 bits when zero. Boolean23 is always written. Presence is decided before truncation: a nonzero handle can serialize as zero while still carrying a present mode.

Reader00764160 (152 bytes, ECX=this,18h stream/cursor+4, RET4) reads presence into a local. A present handle reads mode22 and then WORD20 with the same width choice. **An absent handle clears both mode22 and WORD20.** Boolean23 is read on either branch. Repeated-read coverage checks the clearing behavior with previously populated storage and noncanonical outgoing mode bytes.

## Message95: optional vector and x87 value copies

Writer0071CCA0 (139 bytes) and reader0071CD30 (132 bytes) use the existing75B480/75B4C0 common-header providers, then boolean1C and presence2C. When present, all three floats20/24/28 use signed numeric32 and max-finite scaleD7A248, each through its own004295C0/004293F0 call. Each writer scale is loaded/stored through x87 before its corresponding value. The reader publishes each float directly and sequentially.

**Absence clears presence2C but retains the three old vector words.** Padding2D..2F is retained. This differs from message94's explicit handle clearing.

Value constructor0071CC20 (69 bytes, vector pointer and flag on stack, RET8) calls concrete0075B430(type95), capturing Game's selected owner. It zeroes sender/flags, sets delivery1 and its profile, and performs three individual x87 FLD/FSTP copies before storing the raw presence byte. The vector is copied even when the incoming presence byte is zero. Signaling-NaN quieting and FP status are preserved. The factory constructor retains the payload instead.

## Messages96 and97

Message96 writer0071CBB0 (44 bytes) and reader0071CBE0 (46 bytes) use the common-header providers, boolean1C and unsigned DWORD20/8. Value constructor0071CB50 (48 bytes, RET4) calls concrete0075B430(type96), captures the selected owner, zeroes sender/flags, stores the complete incoming DWORD20, then sets delivery1 and its profile. It does not truncate the constructor argument to the wire width.

Message97 writer0075C700 (91 bytes) and reader0075C760 (90 bytes) use direct common-field bit calls, then unsigned DWORD20/3 and DWORD24/3. Receiver padding and unrelated fields are retained.

## Validation and factory evidence

Strict MSVC Win 32 build and three existing CTests pass. The differential fixture matches **35,447 original/source pairs and 42,722,017 bytes**. New coverage is 5,256 cases: 16 default-constructor/cleanup, 1,024 predicates, 24 scalar, 512 repeated optional-handle cases, 512 narrow wire cases, 2,048 repeated optional-vector cases, 896 x87 vector value constructors and 224 integer value constructors. Coverage includes all eight alignments, four x87 rounding modes, both numeric conversion selectors, 16 distinct float patterns used in 32 uniform/mixed vector variants, signed zeros, denormals, infinities and NaNs, noncanonical flag bytes, handle truncation, valid/invalid Game owner indices, retained fields, raw-storage guards and x87/MXCSR exception flags. Five source-only cleanup faults are inherited; no new EH claim is made.

The collector compares 36,710 live Ghidra/PE bytes, 53 owned CALL edges and 1,231 fixture relocations. Fourteen missing functions are defined; four scalar flows have no gaps. Table0076A40C maps94..97 to00769D50/D70/D90/DB0, with allocations24h/30h/24h/28h. Raw factory CALLs00769D64/769D84/769DA4/769DC4 have verified bytes but no stored Ghidra function membership, and remain separate from the 53 owned edges. Prior Ghidra annotations are preserved, saved under the shared write lock, read back and exported. The immutable artifact archive includes 29 relevant COFF objects.

## Limits and follow-up

Source profiles carry a borrowed context after the five native slots; whole original binary ABI is unproved. Original FH3, unmasked FP fault ordering, nondefault DAZ/FTZ/precision, arbitrary aliases, concurrent mutation, malformed input and allocation failure remain open. Build and fixture results do not establish game parity.

The observed type 98 table entry points to shared default route0076A277; do not infer a safe null/rejection. Type 99 points to00768E72 and needs constructor/profile recovery. Full factory, recorder, networking, startup and gameplay validation remain open.
