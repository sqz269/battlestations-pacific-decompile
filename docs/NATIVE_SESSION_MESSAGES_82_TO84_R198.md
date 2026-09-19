# Native session messages 82 through 84 (R198)

Addresses: 0075A3B0, 0075A3E0, 0075C900, 0075C950, 0075A400, 0075CA60; 0075C9A0, 004B5C00, 004B5C50, 004B5C70, 004B5CA0, 004BD550; 00765A20, 00765A90, 00765AB0, 00765B10, 00765B70, 00766830. Partial parent: 00768530. Reused EH root action: 00464580.

## Scope and native interfaces

Eighteen complete normal game methods (974 bytes) and three five-slot profiles are reconstructed. Ten missing functions were defined; all three scalar flows are complete after repairing84's returning-free gap. Descriptive names are hypotheses, not recovered symbols.

| Type | Size | Default constructor | Predicate | Writer | Reader | Ordinary destructor | Scalar | Profile |
|---|---|---|---|---|---|---|---|---|
| 82 | 24h | 75A3B0 | 75A3E0 | 75C900 | 75C950 | 75A400 | 75CA60 | D02D58 |
| 83 | 24h | 75C9A0 | 4B5C50 | 4B5C70 | 4B5CA0 | not asserted | 4BD550 | CE74B4 |
| 84 | 2Ch | 765A20 | 765A90 | 765AB0 | 765B10 | 765B70 | 766830 | D03694 |

Message83 additionally has value constructor004B5C00 (48 bytes). Its RET4 at4B5C2D occupies three bytes through4B5C2F; the final collection uses the complete instruction. Address4B5CD0 is an unrelated floating getter, not a destructor; it was not defined or renamed. The source83 cleanup helper represents the root stamp inside4BD550 and is not counted as an independent native method.

Native receiver is ECX. Writers take one raw10h cursor pointer; readers take one18h stream pointer with cursor at+4; predicates take a full DWORD query; scalar wrappers take DWORDflags. These and the value constructor return with RET4. Default constructors and ordinary destructors have no stack arguments. Scalar wrappers return captured receiver identity and free only if flags bit0 is set. Profiles order scalar/write/read/predicate/4499C0-always-true. The source-only trailing context pointer and all borrowed session/string publications must outlive their users.

## Construction and common behavior

Constructor82 (40 bytes) zeros fields8/C, owner14, mutable type10, WORD18 and bytes1A/1C; sets delivery1 and D02D58; leaves Boolean20 and padding unchanged. It does not read Game.

Default constructor83 (89 bytes) inlines base construction with type83: delivery3, zero fields8/C, D02C68, one capture of Game E188A8, signed index+18EC selecting owner+18CC only for0..7. It then zeros WORD18 and bytes1A/1C, sets delivery1 and CE74B4, retaining value20 and padding. Value constructor4B5C00 instead calls concrete75B430(type83), initializes the same header, copies the full stack DWORD to20 and installs the profile.

Constructor84 (98 bytes) similarly inlines base/type84/owner selection. It zeros WORD18 and byte1A, writes delivery1, zeroes byte1C, installs D03694, and zeros String20/24 and Boolean28. Other bytes remain unchanged.

Each32-byte predicate accepts only its own full DWORD tag,73 or70 and ignores both receiver and mutable type. The ordinary82 destructor and scalar82/83 wrappers stamp CE4974. All inherited bit and header providers remain concrete source implementations.

## Wire and owned-string cleanup

The common header contains mutable type8, WORD18 width12, Boolean1A and Boolean1C. Message82 adds Boolean20 (23 bits total); writer/reader are77/75 bytes. Message83 adds unsigned DWORD20 width5 (27 bits total); its44/46-byte codecs call75B480/75B4C0 for the first three fields, then handle Boolean1C and the five-bit value.

Message84's88/86-byte codecs inline the common header, serialize owned String20 through429AC0/429F20, then Boolean28. Repeated reads retain the header layout while the real string provider reuses or replaces allocation. String storage uses the actual shared pool publication, shutdown-return gate and lifetime manager; no private pool or new library implementation is introduced.

Destructor765B70 is88 bytes. It leaves the entry profile unchanged during String20/24 return through419CC0/BD1510, retains string-header bits, and stamps CE4974 afterward. There is no own-profile stamp at entry. Active EHstate0 uses handlerC88B98, infoDB87D4, mapDB87CC and actionC88B90 -> existing464580 root stamp. Sourcefinally preserves root stamping on escape. Scalar766830 calls this destructor before optional object free.

The new source-only access-violation check starts84 with a distinct entry-profile value. Its observer confirms that value remains at the failed string return, then checks the root stamp, retained header and unchanged pool state after unwind. This is source cleanup evidence, not an original FH3 exception-dispatch comparison.

## Validation and remaining dependencies

Strict MSVC Win32 build and all three existing CTests pass. The fixture matches **15,711 original/source pairs and 36,944,401 bytes**. The 2,118 new cases comprise 84 constructor/cleanup cases, 768 predicates, 18 populated scalar cases, 224 value-constructor cases, 512 wire cases for82/83 and 512 repeated string cases for84. All eight bit alignments, retained padding, owner bounds, full-value construction, noncanonical Booleans, embedded NULs, lengths0/1/148/149/150/255/256/511, pool/heap thresholds and both return-gate values are covered. Four earlier source-only fault checks remain; the84 fault is the fifth.

The collector checks31,348 live Ghidra/PE bytes,35 owned CALL edges and975 relocations. Existing7-byte root action464580 is added only as fixture support. Raw factory calls769DE4/769E04/769E24 have verified instructions but no stored Ghidra function ownership and remain separate. Table76A3DC maps82..84 to769DD0/769DF0/769E10; native allocations are24h/24h/2Ch. Prior Ghidra names/comments are preserved, mutations are locked, and saved annotations are read back.

Full factory00768530, packet recorder, networking, startup and gameplay remain open. This packet does not prove whole binary ABI, original CRT/FH3 state, allocation failure, malformed input safety, arbitrary aliases or concurrent mutation. The next factory dependencies begin at type85. See reports/native_session_messages82_to84_r198.json for the native evidence and immutable tested/integrated artifacts.
