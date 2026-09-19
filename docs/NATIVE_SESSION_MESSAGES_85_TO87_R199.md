# Native session messages 85 through 87 (R199)

Addresses: 0075C020, 0075C080, 0075C190, 0075C090, 0075C360; 0075A470, 0075A4A0, 0075FDB0, 0075FE00, 0075A4C0, 0075FE70; 00521F30, 00521F60, 00521F80, 00521FA0, 00521FC0, 00522EC0. Partial parent:00768530.

## Scope and interfaces

Seventeen complete normal game bodies (1,262 bytes) and three profiles are reconstructed. Nine missing functions were defined at verified boundaries. Three scalar flows have no gaps. Names are descriptive hypotheses, not recovered symbols. No independent85 ordinary destructor is asserted; its source cleanup helper represents the root stamp inside75C360.

| Type | Size | Constructor | Predicate | Writer | Reader | Destructor | Scalar | Profile |
|---|---|---|---|---|---|---|---|---|
| 85 | 38h | 75C020 | 75C080 | 75C190 | 75C090 | not asserted | 75C360 | D02F4C |
| 86 | 24h | 75A470 | 75A4A0 | 75FDB0 | 75FE00 | 75A4C0 | 75FE70 | D02D6C |
| 87 | 20h | 521F30 | 521F60 | 521F80 | 521FA0 | 521FC0 | 522EC0 | CECCB4 |

Native receiver is ECX. Writers take one10h cursor pointer, readers one18h stream pointer whose cursor is at+4, predicates a full DWORD query and scalars DWORDflags; these methods use RET4. Constructors and ordinary destructors have no stack arguments. Five profile slots are scalar/write/read/predicate/4499C0-always-true. Source contexts follow those slots and borrow actual session, numeric and constant bindings; their lifetimes must cover all profile users.

## Message85 construction and fields

The84-byte constructor inlines base setup: delivery3, fields8/C zero, D02C68, fixed type85, one Game E188A8 capture and signed owner index+18EC. For indices0..7 it reads owner+18CC[index], writes delivery0 then owner14; otherwise it writes ownerNULL then delivery0. Both paths install D02F4C. All payload bytes18..37 remain untouched. Predicate75C080 is13 bytes and accepts only full DWORD85, ignoring the receiver and mutable type. The31-byte scalar wrapper stamps CE4974, frees only flags bit0 and returns captured identity.

Payload consists of seven float bit patterns at18,1C,20,24,28,2C,30 and Boolean34. No physical meaning is asserted from this codec alone. Writer75C190 is457 bytes; reader75C090 is241 bytes. The wire is95 bits: type8, three16-bit numerics, three10-bit numerics, one8-bit numeric and a Boolean.

| Field | Writer transformation | Numeric mode and scale |
|---|---|---|
| 18,20 | SSE COMISS clamps between -24000 and24000 | signed16, scale24000 |
| 1C | x87 add double2000, store/reload float, x87 compare/clamp to0..8000 | unsigned16, scale8000 |
| 24,28,2C | x87 load/store float | signed10, scale float pi |
| 30 | SSE COMISS selects at most1.5; unordered also selects the cap | unsigned8, scale float pi/2 |

The source uses inline instructions for the native COMISS/JBE and FCOMIP branches, preserving masked NaN behavior and SSE versus x87 exception flags. The first clamp loads its maximum even on the lower-clamp route; the third loads it only after the lower comparison. Scale loads precede final x87 argument copies. The1C addition rounds to float before comparison against a double upper bound. The reader performs numeric conversion without writer clamps and subtracts double2000 from1C before storing float.

Verified constant storage is D02F60 float8000, D02F64 float24000, D02F68 float-24000, CF0DD8 double2000, CF0AA0 double8000, D7A264 float pi (40490FDB), CE380C float1.5, CE3C64 float pi/2 (3FC90FDB). Eight spans total40 bytes match the live program and original PE. These remain borrowed inputs rather than invented process globals.

## Messages86 and87

Constructor86 (40 bytes) zeros fields8/C, owner14, mutable type10, WORD18 and bytes1A/1C, sets delivery1 and D02D6C, and retains value20/padding. It does not read Game. Its78/77-byte codecs inline type8, WORD18/12, Boolean1A, Boolean1C and unsigned value20 width2 (24 bits total).

Constructor87 (39 bytes) calls actual75B430(type87), initializes WORD18 and bytes1A/1C to zero, sets delivery1 and CECCB4, retaining padding. Its31-byte writer/reader call existing75B480/75B4C0 for type8/WORD18/12/Boolean1A, then process Boolean1C (22 bits total). Both32-byte predicates accept their own tag,73 or70 and ignore the receiver/mutable type. Ordinary destructors only stamp CE4974; scalar wrappers inline that stamp before optional free.

## Validation and limits

Strict MSVC Win32 build and three existing CTests pass. The differential fixture matches **21,189 original/source pairs and 38,226,669 bytes**. The 5,478 new cases comprise 84 constructor/cleanup cases, 768 predicates, 18 scalar cases, 512 wire cases for86/87 and 4,096 mixed SSE/x87 cases. The latter combine all eight alignments, four x87 rounding modes, both numeric conversion selectors,32 float patterns used uniformly and in mixed fields, signed zeros, denormals, infinities, quiet/signaling NaNs and adjacent float values at the clamps/offset thresholds. Float outputs, serialized bytes, cursors, retained fields, x87 and MXCSR exception flags are compared. Five inherited source-only cleanup faults remain; there is no new EH claim for these classes.

Evidence covers32,710 live Ghidra/PE bytes,36 owned CALL edges and1,045 relocations. Raw factory calls768E66/768E46/769CA4 lack stored Ghidra ownership and remain outside the36-edge total. Verified table76A3E8 maps types85..87 to768E52/768E32/769C90, allocating38h/24h/20h. Prior Ghidra names/comments are preserved and saved annotations are read back under the normal write-lock workflow.

Full factory, recorder, networking, startup and gameplay remain open. Masked comparisons do not establish unmasked fault ordering, nondefault DAZ/FTZ or x87 precision modes, arbitrary aliases, concurrent mutation, allocation failures, malformed-input safety or whole original CRT/FH3/binary ABI. Source profiles and bindings are not drop-in original binary replacements.

The next class is88, whose predicate already has a cruise-command reconstruction; reconcile that source and the caller contracts before supplying its complete profile. See reports/native_session_messages85_to87_r199.json for byte/call evidence and immutable tested/integrated artifacts.
