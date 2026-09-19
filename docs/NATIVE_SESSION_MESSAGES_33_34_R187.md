# Native session messages 33 and 34

Addresses: 00765BD0, 00765C40, 00765C50, 00765D00, 0076CD60, 0076CDD0, 00765D20, 00765D80, 00765D90, 00765DF0, 0076CED0, 0076CF00, 00768530

## Scope and layout

R187 reconstructs12 complete normal bodies (802 native bytes): both constructors, predicates, writers, readers, destructors and scalar-delete methods. Names describe behavior and are not recovered symbols. Source profiles expose five native-visible slots, followed by source-only borrowed pool/fallback bindings. Explicit interfaces do not establish whole binary ABI.

The factory allocates40h at768B32 and calls type33 constructor765BD0 at768B46. It allocates20h at768B52 and calls type34 constructor765D20 at768B66. Those instructions establish allocation sizes. The full7528-byte factory range matches live Ghidra and the PE, but these two call sites lack stored Ghidra function membership; they remain separate raw evidence.

| Type | Profile | Native payload |
| --- | --- | --- |
| 33 | D036A8 | owned header18/1C, DWORDs20/24, owned headers28/2C and30/34, qword38/3C |
| 34 | D036BC | owned header18/1C |

Constructors perform the existing base type/current-game owner selection, then mode1 and the final profile. Type33 zeroes only its three length/data pairs in ascending order. Its two scalar DWORDs and qword remain untouched. Type34 zeroes its single header. Base padding11..13 is retained. Both predicates compare the full query DWORD with their fixed tag regardless of a later typebyte change.

## Wire behavior

Type33 writes type8, owned string18, unsigned4 DWORD20, unsigned4 DWORD24, owned string28, owned string30 and contiguous qword38. The writer captures the high word3C before the low word38, then passes both to existing429180. Its reader follows the same field sequence; the existing428E90 reads64 bits into a local buffer before publishing low/high DWORDs.

Type34 transfers only type8 and owned string18. Writers take raw10h cursors; readers take18h wrappers with cursor+4. Wrapper profile, ownership and padding are retained.

Both classes use the established429AC0/429F20 codec. Writing uses the stored low length byte, reloads data after the prefix and transmits that many bytes, including embedded NULs. Null data uses actual E17669 backing. Reading consumes the complete wire payload into a256-byte stack buffer, stores through the first NUL, and uses actual resize/copy behavior. Equal-length/null-data reuse remains null. The fixture binds the captured255-byte fallback rather than inventing an empty string.

## Lifetime and unwind

Type33 destructor765C50 stampsD036A8, returns strings30,28,18 in reverse order, then stamps rootCE4974. Each nonnull header uses captured data and length+1 with actual419CC0 pool getter and BD1510 return, trailing argument1. Headers remain unchanged after returns.

FuncInfoDB8810 references the three-entry unwind mapDB87F8:

| State transition | Funclet | Cleanup |
| --- | --- | --- |
| 2 to1 | C88BC3 | header28 via41DD20 |
| 1 to0 | C88BB8 | header18 via41DD20 |
| 0 to-1 | C88BB0 | root stamp4499D0 |

Source nested __try/__finally preserves these remaining cleanups if an earlier return escapes. Type34 destructor765D90 stampsD036BC, returns its single header and stamps root. Its FuncInfoDB883C/mapDB8834 uses C88BE0 for the root stamp on unwind. Both scalar methods call the full destructor and free through actual BF65AC only when flags bit0 is set, returning captured identity after successful destruction.

Original raw handlersC88BCE/C88BE8, maps and support funclets are relocated for normal-path native comparisons with the actual CRT FH3 export. Original FH3 exception dispatch is not compared. The source uses its own compiler-generated cleanup machinery.

## Validation and evidence

- 9,888 live bytes matched the PE, covering complete new/support bodies, profiles, fallback, EH metadata and factory bytes.
- 60 direct CALL/tail-JMP rows passed ownership verification. Two factory calls remain separate raw evidence.
- Four missing functions were defined. Missing ADD ESP,4 instructions after free in both scalar bodies were repaired without changing the callee's no-return annotation. Both listings now have zero gaps.
- Confirmed names/comments preserve prior values and evidence; affected exports are refreshed.
- Strict MSVC Win32 build and all three existing CTests passed.
- 1,664 original/source pairs matched12,982,576 observation bytes:56 constructors,1,152 records,320 embedded-NUL cases,128 null-source combinations and8 freeing scalars.
- Cases cover all8 bit offsets, stored lengths0/1/31/148/149/150/255/256/511, mixed header sizes, empty/equal/different/equal-null destinations, both small-return gate states, all null-header combinations, nibble truncation, qword transport, record guards and actual pool arena/ring state.

Four additional source-only access-violation cases check each type33 unwind position and type34 cleanup. A scoped vectored exception observer restores the real private pool publication after the deliberately invalid context triggers an access violation, then continues exception search. This lets remaining finally blocks use the actual pool providers. Their resulting arena/ring states match explicit reverse-order cleanup of the remaining headers. The failed header stays retained for manual fixture cleanup, and the root stamp is verified. No fabricated allocation/getter/return result is supplied.

The fixture uses a prepublished private pool and actual singleton allocation/free. It does not inspect freed heap contents, touch peer gameplay or modify the game installation. Lazy pool recreation, allocation failure, concurrency, general aliasing, original CRT identity, original FH3 exception compatibility and whole ABI remain open.

## Follow-up packet

Type35 is allocated54h at768B72 and constructed by52AEF0 at768B86, with profileCED098. Its writer5294F0 uses owned headers and a word array at40/44/48; the raw reader target52ABE0 calls529980 while rebuilding that array. That reader and helper still require complete body/ABI recovery. Claim their functions, cleanup dependencies and profile before implementation; no type35 reconstruction is claimed here.

Full factory composition, packet recorder, network workers, ordinary startup, network exchange and gameplay validation remain open. This packet does not establish a runnable game rebuild.
