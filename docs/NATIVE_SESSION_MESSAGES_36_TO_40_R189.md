# Native session messages 36 through 40

Addresses: 00764670, 007646E0, 007647A0, 00764820, 00764890, 0075DD80, 0075DE00, 0075DE10, 0075DE60, 0075DEB0, 0075F670, 0075F6F0, 0075F700, 0075F730, 0075F760, 0075F780, 0075F800, 0075F810, 0075F840, 0075F870, 0075F890, 0075F910, 0075F920, 0075F970, 0075F9B0, 00768530

## Recovered scope

R189 reconstructs 25 complete normal bodies (1,366 native bytes): five constructors, predicates, writers, readers, and scalar deletes. Names are descriptive hypotheses, not recovered symbols. The C++ interfaces and source profile bindings do not establish whole binary ABI compatibility.

| Wire type | Size | Constructor | Predicate | Writer | Reader | Scalar delete | Native profile |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 36 | 28h | 764670 | 7646E0 | 7647A0 | 764820 | 764890 | D03608 |
| 37 | 24h | 75DD80 | 75DE00 | 75DE10 | 75DE60 | 75DEB0 | D030C0 |
| 38 | 20h | 75F670 | 75F6F0 | 75F700 | 75F730 | 75F760 | D0323C |
| 39 | 20h | 75F780 | 75F800 | 75F810 | 75F840 | 75F870 | D03250 |
| 40 | 30h | 75F890 | 75F910 | 75F920 | 75F970 | 75F9B0 | D03264 |

Every constructor is 99 bytes: ECX is the record, there are no stack arguments, and EAX returns the same address. It initializes delivery to 3, clears fields 08/0C, stamps the base profile D02C68 and the fixed type byte, captures the current game through E188A8, then selects the owner from 18CC using the signed index at 18EC when it lies in 0..7. Otherwise the owner is null. It finally sets delivery to 1 and the class profile. Payload bytes from 18 onward and padding 11..13 retain their prior values. Full normalized instruction checks confirm this shared shape for all five constructors.

The 13-byte predicates compare the complete stack DWORD against their fixed type, independent of the mutable byte at offset 10. The 31-byte scalar deletes stamp root CE4974, free through BF65AC only when flags bit zero is set, and return the captured identity. They have ECX record, one stack flags argument, and RET 4. Retained scalar deletion preserves all other bytes.

## Wire behavior

All writers receive ECX record and one raw 10h cursor argument. Readers receive ECX record and one 18h stream wrapper, whose cursor starts at +4. Both return with RET 4. Every record starts with the mutable type byte encoded in eight bits.

| Type | Fields after the type byte |
| --- | --- |
| 36 | unsigned 3 bits from DWORD 18; unsigned 2 bits from DWORD 1C; numeric floats at 20 and 24 |
| 37 | signed 6 bits from DWORD 18; full 32-bit DWORDs at 1C and 20 |
| 38, 39 | one 64-bit value at 18/1C |
| 40 | 64-bit values at 18/1C and 20/24; Boolean-normalized byte 28 |

### Type 36: x87 loads before the numeric codec

Each float call reloads actual D7A248, whose verified bits are 7F7FFFFF. The writer performs an x87 FLD/FSTP of that scale, then an x87 FLD/FSTP of the field before passing five arguments to 4295C0: value, zero flag 0, signed flag 1, scale, and width 32. The source preserves this order. Signaling NaNs are quieted by the caller's load, and denormals can set the x87 exception flag even though the subsequent codec uses its unscaled 32-bit path.

The reader reloads the scale through x87 for each call to 4293F0 and supplies the actual destination address. The actual maximum-float double sentinel at D7A278 and the remaining numeric context are borrowed from the established codec. No guessed numeric defaults are introduced. The source-only profile stores borrowed references after its five visible slots; the backing must outlive the profile.

### Integer and 64-bit records

Type 37 uses the signed 428D30 reader for field 18, so bit 5 controls sign extension to the full DWORD. Its other two values use unsigned full-width readers.

The 64-bit writers load the high DWORD first and the low DWORD second, capturing both before each 429180 call. Readers invoke 428E90 directly on the field; that helper reads eight local bytes before publishing the two destination words. Types 38 and 39 have identical complete writer and reader instruction shapes. Type 40 reloads its second pair only after the first write, then reloads raw flag 28. Reading the flag canonicalizes it to zero or one and leaves bytes 29..2F untouched.

## Factory evidence and remaining special class

The complete 7,528-byte factory range matches the live program and PE. Verified allocation/constructor pairs are:

| Allocation instruction | Size | Constructor call | Target / wire type |
| --- | --- | --- | --- |
| 768B92 | 28h | 768BA6 | 764670 / 36 |
| 768BD2 | 24h | 768BE6 | 75DD80 / 37 |
| 768BF2 | 20h | 768C06 | 75F670 / 38 |
| 768C12 | 20h | 768C26 | 75F780 / 39 |
| 768C32 | 30h | 768C46 | 75F890 / 40 |

These five call instructions lack stored Ghidra function membership. They remain separate raw evidence, not ownership-checked call rows. The factory itself exists; it is not listed as a missing function.

Allocation order alone does not identify wire types. The intervening 20h allocation at 768BB2 calls 759F40 at 768BC6. That constructor writes type zero and profile D02CB8; it must not be labeled type 37 merely because it follows type 36 in the listing. Its contract and full factory dispatch remain open.

## Validation

- Fourteen missing functions were defined with prior metadata preserved. All five scalar-delete listings have zero gaps.
- 10,749 live bytes match the PE, including the new bodies, dependencies, actual profiles/constants, factory range, and fixture CRT support.
- All 65 owned direct CALL/tail-JMP rows pass verification. The five factory calls are separately recorded with their missing membership.
- Strict MSVC Win32 build and all three existing CTests pass.
- 5,814 original/source pairs match 1,607,424 observation bytes: 140 constructors, 5,184 records, 160 raw readers, 320 overlap cases, and ten freeing scalars.
- Coverage includes all eight bit offsets, nonzero output backing, invalid/valid signed owner indices, retained payload/padding, signed six-bit boundaries, full-width words, noncanonical Boolean bytes, and guarded record storage.
- Type 36 cases include signed zeros, denormals, minimum normals, maximum finite values, infinities, quiet/signaling NaNs, and ordinary values. Both conversion selectors and all four x87 rounding modes are exercised. Low exception flags from x87 and MXCSR are compared after writing and reading.
- Overlap cases use owned backing for both writer and reader aliases. Scalar release uses the actual allocation/free boundary; native root stamps are observed before release, and freed bytes are never inspected.

This proves the compared normal paths and bounded aliases. It does not prove arbitrary aliasing, unmasked floating-point exceptions, concurrent constant mutation, allocation failure, original CRT identity, full ABI/FH3 compatibility, full factory composition, startup/network exchange, or gameplay. The game installation and peer runtime are untouched.

## Follow-up packet

Recover the type-zero special class at 759F40/profile D02CB8 and continue from the factory branch after 768C4D. Check the switch table and constructors before assigning wire types. Full factory binding is still required by the packet recorder and network workers; a runnable game rebuild remains unproved.
