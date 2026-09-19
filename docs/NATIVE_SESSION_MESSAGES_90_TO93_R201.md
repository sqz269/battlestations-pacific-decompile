# Native session messages 90 through 93 (R201)

Addresses: 0075AF60, 0071C640, 0071C670, 0071C6C0, 0071C720, 0071E370; 0075AF90, 0075AFF0, 00763FC0, 00764030, 0075B020, 007640B0; 00764F90, 0071C830, 0071C900, 0071C930, 0071CA30, 0071CB40, 0071E3B0; 0075B030, 0071C770, 0071C7A0, 0071C7E0, 0071C820, 0071E390; partial factory 00768530.

## Scope and shared behavior

Twenty-four complete normal game bodies (1,631 bytes) and four profiles are reconstructed. The existing 37-byte `gameunit_set_command_message_is_category_0071c900` predicate and `SceneCommandTarget` are reused. No peer-owned source changed and no library code was newly ported. Names are descriptive hypotheses.

| Type | Allocation | Factory constructor | Profile | Additional payload |
|---|---|---|---|---|
| 90 | 28h | 0075AF60 | 00CFD9C4 | DWORD selector20 and value24 |
| 91 | 2Ch | 0075AF90 | 00D02E84 | WORD20, padding22..23, DWORD24/28 |
| 92 | 3Ch | 00764F90 | 00CFD9EC | flag20, command21, padding22..23, target24..3B |
| 93 | 28h | 0075B030 | 00CFD9D8 | flag20, padding21..23, signed DWORD24 |

All factory constructors zero the extended header's fields08/0C, owner14, mutable type10, sender18 and flags1A/1C; set delivery1 and their profile; and do not read Game. Message91 additionally sets DWORD24=1 and DWORD28=5. Message92 copies the borrowed default vector at F87574/78/7C with MOVSS and zeroes target kind/position-valid, handle, cached object and trailing float. Other payload and padding bytes are retained.

Each category predicate accepts its fixed type plus 89, 73, 70, independently of receiver or mutable type. The reused message92 predicate ends at 0071C924; RET4 at 0071C922 is three bytes. Its existing source function record is preserved with a separate reuse fragment. Each ordinary cleanup stamps rootCE4974 only; each scalar cleanup tests flag bit0, stamps the root before concrete free, and returns captured this. The final profile slot reuses 004499C0.

## Wire contracts

The common fields are mutable type10/8 bits, sender18/12 and booleans1A/1C. Messages90/92/93 call the existing 0075B480/0075B4C0 common-header providers; message91 uses direct bit-provider calls.

- **90:** Write selector20 as unsigned4, then value24 with width9 only when the full outgoing selector equals2, otherwise width1. The reader first decodes the four-bit selector, stores it, and chooses the value width from that decoded value. For example, outgoing18 serializes selector2 but takes the writer's one-bit branch; the reader takes nine bits. The reconstruction preserves this asymmetry.
- **91:** WORD20 uses16 bits, DWORD24 uses unsigned2, and DWORD28 uses unsigned4. The reader obtains both DWORD values in locals before publishing either; bytes22..23 remain unchanged.
- **92:** Boolean20 precedes command byte21/8, reversing the semantic order used by message88. Target kind24 gates handle26/13; position-valid25 gates three signed numeric floats at24 bits with scale22,000 (CF9360). COMISS against borrowed1.0 (D7A24C) controls the trailing float: values below one and unordered values omit it; other values use signed32 with max-finite scale. Numeric-array providers are00429790/004294F0. Local vectors use MOVSS copies. Reader presence bits are independent locals; absent fields retain old flags, handle, cached object, vector and trailing float. A present handle sets kind=1 and clears the cached object pointer. No writer threshold is added to reading.
- **93:** Boolean20 followed by signed DWORD24/5. Narrow signed decoding and payload padding are preserved.

## Message92 value constructor

The separately named 0071C830 body (125 bytes, ECX=this, three stack arguments, RET0Ch) is now reconstructed concretely. It calls existing 0075B430 with type92 and captures Game's selected owner, then stores caller flag20, delivery1, its profile and zero header flags. It copies the target's two leading WORDs and object pointer, followed by **four x87 FLD/FSTP float copies**. These differ from the default constructor's MOVSS bit copies: signaling NaNs can be quieted and FP status changes. A nonnull command pointer contributes the low byte of DWORD at+4; null produces commandFF. The existing Ghidra name `BSP_GameUnitSetCommandMessage_Construct` is retained.

## Evidence and validation

Strict MSVC Win32 build and all three existing CTests pass. The fixture matches **30,191 original/source pairs and 41,180,705 bytes**. The 6,564 new cases comprise 140 default-constructor/cleanup cases, 1,024 predicates, 24 scalar cases, 1,536 narrow wire cases, 2,048 repeated target-wire cases and 1,792 x87 value constructors. Coverage includes all eight wire alignments, four rounding modes, both numeric conversion selectors, 32 floating-point patterns including NaNs/infinities and values adjacent to1.0 and22,000, full-versus-truncated selectors, noncanonical flags, invalid/valid Game owner indices, null/nonnull commands, retained fields, raw-storage guards and x87/MXCSR exception flags. Five source-only cleanup faults are inherited; this packet makes no new EH claim.

The collector compares 35,229 live Ghidra/PE bytes, 56 owned CALL edges and 1,159 fixture relocations. Twelve missing functions are defined and four scalar flows have no gaps. Table0076A3FC maps90..93 to00769CD0/CF0/D10/D30. Factory CALLs00769CE4/769D04/769D24/769D44 have verified bytes but no stored Ghidra function membership; they remain separate from owned edges. Prior annotations are preserved, saved under the shared write lock, read back and exported. The immutable evidence archive captures 28 relevant COFF objects.

## Limits and next dependencies

Profiles use a borrowed source-only context after their five native slots. Whole binary ABI, original FH3, unmasked FP fault ordering, nondefault DAZ/FTZ/precision modes, arbitrary aliases, concurrent mutation, malformed input and allocation failure remain unproved. Build and fixture evidence do not establish game parity. The full factory, recorder, networking, startup and gameplay remain open.

Factory entries 94/95 allocate 24h/30h and call 0075B060/0075B100. Recover their profiles and method bodies next; do not infer their payload contracts from adjacent classes.
