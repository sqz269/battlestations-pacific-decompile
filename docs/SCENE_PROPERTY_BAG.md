# Scene property bag (`008F5A00`, the 114h object and its twelve type codes)

Addresses: 008f5a00 008f2260 008f33f0 008f3370 008f38a0 008f41a0 008f41f0 008f54f0
008f0700 008f4f60 0043b8b0 0048e840 008f02b0 008f02e0 0045c440 00467cc0

Packet `cc2_scene_property_bag`. Read-only analysis of `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Every name below is a hypothesis, not a recovered symbol.
`docs/SCENE_FILE_READER.md` already reconstructs the `.scn` grammar and the three passes;
this packet establishes the object those passes fill and the object they read back.

## The bag is a hash map with two extra fields

`008F41A0` is the constructor and `008F66DB` (inside the parser) and `008F41FC` are two inlined
copies of it. All three write the same fields into a 114h allocation, so the layout is settled by
three independent producers:

| Offset | Size | Meaning | Evidence |
| --- | --- | --- | --- |
| +00h | 4 | vtable, `00D16504` | `008F41A0` first store; `008F420A`; `008F66EE` |
| +04h | 4 | vtable of the embedded map, `00D162C4` | `008F41A0`; the map's `this` is `bag+4` at `008F22C9` and `008F5596` |
| +08h | 4 | map entry count | zeroed by all three; `bag+4` is the map, so this is map+04h |
| +0Ch | 256 | 64 bucket heads | `MOV ECX,0x40 / STOSD.REP` at `008F4223` and `008F6707`; `0043B8B0` indexes `map+8+bucket*4` |
| +10Ch | 4 | next insertion ordinal | `008F33F0` reads it, stores it in the record and writes back `+1` |
| +110h | 4 | owner, the constructor's only argument | `008F41A0` stores `param_2`; both inlined copies store 0 |

The map is therefore `bag+04h..bag+10Bh` (108h bytes) and the bag is that plus one vtable in front
and two dwords behind: 4 + 0x108 + 8 = 0x114. A bucket node is four dwords, from `0043B8B0`:

| Offset | Meaning |
| --- | --- |
| +00h | hash, from `0043B760` |
| +04h | key string |
| +08h | the property record |
| +0Ch | next node in the bucket |

Key comparison is **case-insensitive**: `0043B8B0` accepts a pointer-equal key or `__stricmp == 0`
after the hash matches.

## The property record is 38h bytes whatever its type

`008F4F60` allocates `operator_new(0x38)` in every one of its twelve switch arms, so the twelve
types are subclasses of one fixed-size class. `008F38A0` is the producer that fills a fresh one and
`008F0700` is the producer that assigns into an existing one; between them every offset below is
written by name.

| Offset | Meaning | Evidence |
| --- | --- | --- |
| +00h | vtable, one per type | `008F38A0` writes `00CE89D4` for the string subclass |
| +04h | type code, 0..0Bh | `008F38A0` writes 2; `008F0700` and `008F4F60` switch on it; the parser compares it at `008F5B4C`, `008F5BE4`, `008F5C9A`, `008F5D53`, `008F5DE4`, `008F5FE4`, `008F6276`, `008F6327`, `008F63ED`, `008F64C7`, `008F6593` |
| +08h | not written by `008F38A0` and not read on any path this packet covers | gap, see below |
| +0Ch | the scalar value: int, float, `char*`, bool byte, enum int, sub-bag pointer, or V3.x | `008F0700` cases 0-4 and 7; `008F5C73` stores a float; `008F2324` and `008F55F2` follow it as a sub-bag |
| +10h | V3.y | `008F0700` case 7; `008F6302` |
| +14h | V3.z | `008F0700` case 7; `008F6305` |
| +18h..+1Fh | reference payload, copied whole by `008F0340` | `008F0700` case 5 passes `other+18h`; `008F38A0` zeroes +18h and +1Ch |
| +20h | array data pointer | `008F6570`; freed at `008F63A8`, `008F647E`, `008F6552` |
| +24h | array element count | `008F656D`; `008F63C3` |
| +28h | enum/`LUA_S` declaration object | `008F5DE9` and `008F33A6` load it into ECX; `008F4F60` case 4 passes it to the type-4 constructor |
| +2Ch | byte, set to 1 by `008F38A0` | meaning unestablished, provisional |
| +30h | owning bag | `008F33F0` stores `this` |
| +34h | insertion ordinal | `008F33F0` stamps it; `008F4F60` copies it on every clone; `008F562E` copies it on a merge |

Two gaps, stated rather than guessed: `008F38A0` leaves +08h, +10h, +14h and **+28h**
uninitialised in the allocation it returns. A type-2 record never reaches the +28h read, which only
happens under `[record+4] == 4`, so the omission is not a live bug on the paths covered here, but a
record built by `008F38A0` and later retyped would read heap garbage as its declaration.

### The ordinal counter starts at the sentinel

`008F33F0` reads `bag+10Ch`, stores it in `record+34h` and writes back `value+1`, but only when
`record+34h` is already zero. The counter starts at zero, so the first record inserted into a bag
gets ordinal 0, which is also the "unstamped" sentinel: re-inserting that record re-stamps it with
whatever the counter has reached. Ordinals are therefore 0-based and the first one is not stable.

## The twelve type codes

The letter is matched against the type the declaration already carries; `008F5B50`
(`CMP [EBX+4],0 / JZ 008F5B70`) jumps straight into the integer reader when the declared type is 0,
and a letter that matches nothing falls through the whole chain into that same reader. Letter
strings are at `00D16510`..`00D1652B`, `00D162D4`, `00D162D8`, `00D16358`..`00D1638F`, `00CFAD04`
and `00CE60E4`..`00CE60EF`.

| Code | Letter | String | Value grammar after the letter | Storage |
| --- | --- | --- | --- | --- |
| 0 | `I` | `00D16528` | one integer; a non-numeric token yields 0 and is not consumed unless argument 2 is set (`008F5B92`, `008D8F10`) | +0Ch int |
| 1 | `F` | `00CFAD04` | one float, same guard through `008D8F40` (`008F5C2E`) | +0Ch float |
| 2 | `S` | `00D16524` | one string, read by `008EFB00` | +0Ch `char*`, `strdup` |
| 3 | `B` | `00CE60E4` | `true` (`00CE4378`) or `false` (`00CE4370`) | +0Ch byte, 1 only for `true` |
| 4 | `E` | `00D162D4` | `<enum> : <value>`; the `:` is `00CF00B0` | +0Ch int |
| 4 | `LUA_S` | `00D162D8` | `<table> : <name>`, same shape | +0Ch int |
| 5 | `R` and seven typed letters | see below | one token, usually a quoted name | +18h reference block |
| 6 | none | n/a | `Key { ... }`, a nested bag | +0Ch sub-bag pointer |
| 7 | `V3` | `00D16520` | three floats, each guarded by `008D8F40`; a missing component stays 0.0 | +0Ch, +10h, +14h |
| 8 | `BIN` | `00D1651C` | `<count>` then `<count>` tokens, each turned into a byte by `008EF6D0` | +20h/+24h, stride 1 |
| 9 | `FA` | `00D16518` | `<count>` then `<count>` floats | +20h/+24h, stride 4 |
| 10 | `IA` | `00D16514` | `<count>` then `<count>` integers | +20h/+24h, stride 4 |
| 11 | `V3A` | `00D16510` | `<count>` then `<count>` groups of `( x y z )` | +20h/+24h, stride 12 |

Codes 4 and 5 are the two that are not one code per letter. `E` and `LUA_S` share code 4 and are
separated by the **declaration's own** letter string at `decl+118h`: `008F02B0` accepts `"E"` and
`008F02E0` accepts `"LUA_S"`, each called with `ECX = record+28h` loaded at `008F5DE9`/`008F5E82`.
Both grammars read a token, and when the next token is `:` they read a second token **into the same
128-byte buffer at ESP+0C0h**, so the declaration half of `E Party : Allied` is discarded and only
`Allied` is stored. That token then becomes an integer through `0048E840`, a name-to-value map find
on the declaration object (`008F33A6`).

Code 5 carries a sub-kind that the parser puts in EDI before the setter. Two independent dispatch
chains, `008F5FED..008F60A1` (declared) and `008F616A..008F6231` (undeclared), map the same eight
letters to the same eight values:

| Letter | String | Kind |
| --- | --- | --- |
| `R` | `00CE60EC` | 0 |
| `RLand` | `00D1637C` | 1 |
| `RFort` | `00D16384` | 2 |
| `RShip` | `00D1636C` | 3 |
| `RPlane` | `00D16374` | 4 |
| `RPlnShp` | `00D16364` | 5 |
| `RLPlnShp` | `00D16358` | 6 |
| `RPath` | `00D1638C` | 7 |

Each of the four counted types peeks for `;` **before** reading the count (`0045C440` with
`00CE5698` at `008F635E`, `008F6424`, `008F64FC`, `008F65CA`): an authored `Vehicles = IA ;` leaves
the array empty and reads no count at all. The `V3A` allocation size is
`count * 0Ch` with a saturating overflow guard (`008F65E7`: `MUL EDX / SETO CL / NEG ECX / OR ECX,EAX`
requests FFFFFFFFh rather than wrapping).

### Which letters the shipped game actually uses

`rg` over every `.scn` under the installed game, counting ` = <letter> ` occurrences:

| Letter | Occurrences |
| --- | --- |
| V3 | 713485 |
| E | 390133 |
| F | 242886 |
| B | 215580 |
| I | 197874 |
| R | 117506 |
| RPath | 95936 |
| S | 36166 |
| RFort | 2140 |
| RPlnShp | 504 |
| LUA_S | 11 |
| IA | 5 |

`RShip`, `RPlane`, `RLand`, `RLPlnShp`, `BIN`, `FA` and `V3A` are implemented but unused by the
shipped scenes, so their grammars are read from the listing only and have no installed-file check.

## Lookup, insert, merge and clone

| Address | Name | ABI | What it does |
| --- | --- | --- | --- |
| `008F2260` | `BSP_ScenePropertyBag_Find` | `__thiscall(bag, const char* key)`, `RET 4` | Splits the key at the first `.`; finds the head in `bag+4`; with no dot returns the record or 0, with a dot recurses into `record+0Ch` **only when `record+4 == 6`** and otherwise returns 0. The split uses the shared global buffer `00F89658`, so it is not reentrant. |
| `008F33F0` | `BSP_ScenePropertyBag_Insert` | `__thiscall(bag, const char* key, record)`, `RET 8` | Inserts through `008F28F0`, stamps `record+30h = bag` and takes an ordinal when `record+34h` is 0. |
| `008F38A0` | `BSP_ScenePropertyBag_SetString` | `__thiscall(bag, const char* key, const char* value)`, `RET 8` | Allocates a 38h type-2 record, `00438E40`-duplicates the value, inserts. |
| `008F3370` | `BSP_SceneProperty_SetValueFromToken` | `__thiscall(record, const char* token)`, `RET 4` | Type 2 frees `+0Ch` and duplicates the token into it; every other type resolves the token through `0048E840` on `record+28h` and stores the integer. The decompiler's early return after the `_free` is an artifact: `008F3381` falls through to `008F3394`. |
| `008F41A0` | `BSP_ScenePropertyBag_Construct` | `__thiscall(bag, void* owner)`, `RET 4` | Zero bag with the owner at +110h. |
| `008F41F0` | `BSP_ScenePropertyBag_Clone` | `__thiscall(bag)`, `RET` (no stack args) | Allocates and constructs a fresh bag with owner 0, then walks the source map with the `00480690`/`0047E480` iterator, clones each record with `008F4F60` and re-inserts it under the source key. |
| `008F54F0` | `BSP_ScenePropertyBag_MergeFrom` | `__thiscall(dest, source, char keep_existing)` | Per source entry: absent from `dest` means clone, copy `+34h`, insert; present as a type-6 record means recurse on both `+0Ch`; present otherwise means `008F0700` overwrite, but only when `keep_existing` is 0. |
| `008F0700` | `BSP_SceneProperty_AssignValueFrom` | `__thiscall(record, other)`, `RET 4` | Switches on **the destination's** type: 0/1/4 copy `+0Ch`; 2 frees and duplicates; 3 copies the byte; 5 copies `other+18h` through `008F0340`; 7 copies `+0Ch`/`+10h`/`+14h`; 8..0Bh copy `other+20h` through `008F03B0`. Code 6 has no arm. |
| `008F4F60` | `BSP_SceneProperty_Clone` | `__thiscall(record)`, `RET` | One `operator_new(0x38)` per type arm; every arm copies `+34h` onto the clone. |

`008F41F0` is a deep copy, not a reader, and `008F54F0` is a merge, not a reader. The packet brief
described both as readers; the bodies say otherwise. The only reader keyed by a property name is
`008F2260`, and it is looked up **by key, never by slot**.

### Absent properties

`008F2260` reports an absent property by returning 0 in EAX, in three cases: the head key is not in
the bucket chain; the head key is present but is not type 6 while the path continues past a dot;
and the tail lookup itself fails. There is no out-of-band error and no default value. Every caller
therefore has to null-check, and `docs/SCENE_ENTITY_FACTORY.md` records one at `0046C550` that does
not: the byte at `+0Ch` is read without a check.

## The parse routine

`008F5A00` is `__thiscall`, `RET 0Ch`, three stack arguments. The prior ledger evidence called ECX
the tokenizer; the prologue disagrees. `008F5A25` copies ECX into EBX and spills it to `[ESP+18h]`,
from where it is reloaded as the `this` of every `008F3xxx` setter and of the recursive call, while
`008F5A1E` loads the tokenizer from the first stack argument into ESI.

```
008F5A00(this = ScenePropertyBag*, Tokenizer* tokenizer,
         char allowUntypedNumbers, void* groupRegistry)
```

Argument 3 is not a flag: `008F5A47` loads it into EDI and `008F5AA9` calls `00469B60` **on it** to
turn a group name into a bag, so it is a registry object and a null skips the group list.
Argument 2 is the byte tested at `008F5B92` and `008F5C2E`, which forces the integer and float
readers to read even when the next token does not scan as a number.

The leading `( name name ... )` list is **inheritance**: each name is resolved through
`00469B60` and the resulting bag is merged in with `keep_existing = 0` (`008F5AB3` pushes 0), so a
group's values overwrite what the bag already had and the body that follows overwrites the group.

A key followed by neither `=` nor `{` is dropped (`008F5B3D` jumps past the assignment to the `{`
test, which falls back to the `}` test at `008F6745`). A key followed by `{` reuses an existing
type-6 record's sub-bag and otherwise allocates a fresh 114h bag inline and inserts it through
`008F3B60` before recursing with argument 3 forwarded and argument 3's group flag cleared.

### Host table

One row per native call site the reconstruction models. `this`/args/ret are as established above;
`gate` names the condition under which the site is reached.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| 008F5A2E | 008D8A70 | `peek_token` | ECX = tokenizer; ret `char*` | entry |
| 008F5A59 | 008D9930 | `expect_token` | ECX = tokenizer, `"("`; ret bool | peek is `(` and argument 3 non-null |
| 008F5AA1 | 008D9980 | `read_token` | ECX = tokenizer; ret `char*` | group-list element |
| 008F5AA9 | 00469B60 | `lookup_group_bag` | ECX = argument 3, name; ret bag | group-list element |
| 008F5AB3 | 008F54F0 | merge | ECX = this, source bag, 0 | group-list element |
| 008F5AC1 | 008D9930 | `expect_token` | `")"` | group list ends |
| 008F5ACD | 008D9930 | `expect_token` | `"{"` | always |
| 008F5AF9 | 008D9980 | `read_token` | ret the key | body loop |
| 008F5B1B | 008F2260 | find | ECX = this, key; ret record or 0 | body loop |
| 008F5B9E | 008D8F10 | `next_token_is_number` | ECX = tokenizer; ret bool | type 0, argument 2 clear |
| 008F5BB2 | 008D9AD0 | `read_int` | ECX = tokenizer, `&ok`; ret int | type 0 |
| 008F5C3A | 008D8F40 | `next_token_is_number` | ECX = tokenizer; ret bool | type 1, argument 2 clear |
| 008F5C55 | 008D9B40 | `read_float` | ECX = tokenizer, `&ok`; ret ST0 | type 1 |
| 008F5C8C | 008F3770 | bag set float | ECX = this, key, float | type 1, undeclared key |
| 008F5CF6 | 008EFB00 | string token read, **contract unread** | ECX = this, tokenizer; ret `char*` | type 2 |
| 008F5D04 | 008F3370 | record set from token | ECX = record, `char*` | type 2, declared key |
| 008F5D1E | 008F38A0 | bag set string | ECX = this, key, `char*` | type 2, undeclared key |
| 008F5D45 | 008F38A0 | bag set string | ECX = this, key, `""` (`00CE3A0C`) | type 2, tokenizer exhausted |
| 008F5DD6 | 008F3940 | bag set bool | ECX = this, key, byte | type 3, undeclared key |
| 008F5DEC | 008F02B0 | declaration letter is `E` | ECX = record+28h; ret bool | type 4 |
| 008F5E6F | 008F3370 | record set from token | ECX = record, `char*` | type 4, declared key |
| 008F5E85 | 008F02E0 | declaration letter is `LUA_S` | ECX = record+28h; ret bool | type 4 |
| 008F5FA2 | 0048E960 | `lookup_enum_declaration` | ECX = `*00E1867C`, name; ret decl | type 4, undeclared key |
| 008F5FCF | 008F3A20 | bag set enum | key, decl, value token | type 4, undeclared key |
| 008F60C1 | 008F0420 | record set reference | ECX = record, token, 0, kind | type 5, declared key |
| 008F6254 | 008F3AC0 | bag set reference | ECX = this, key, token, 0, kind | type 5, undeclared key |
| 008F6319 | 008F3690 | bag set vector3 | ECX = this, key, `float[3]` | type 7, undeclared key |
| 008F638A | 008D9980 | `read_token` | one `BIN` element | type 8, count > 0 |
| 008F6391 | 008EF6D0 | `decode_byte_token` | ECX = token; ret byte | type 8, count > 0 |
| 008F6467 | 008D9B40 | `read_float` | one `FA` element | type 9, count > 0 |
| 008F6617 | 008D9930 | `expect_token` | `"("` (`00CE5850`) | type 0Bh, per element |
| 008F6653 | 008D9930 | `expect_token` | `")"` (`00CE584C`) | type 0Bh, per element |
| 008F6684 | 008F0580 | record set array | ECX = record, count, data, 0 | type 0Bh, declared key |
| 008F669A | 008F3610 | bag set vector3 array | key, data, count, 0 | type 0Bh, undeclared key |
| 008F66A6 | 008D9930 | `expect_token` | `";"` (`00CE5698`) | after every assignment |
| 008F66E0 | 00BF681B | `operator new(0x114)` | ret bag | `{` with no declared sub-bag |
| 008F672E | 008F3B60 | bag insert sub-bag | ECX = this, key, bag | `{` with no declared sub-bag |
| 008F6740 | 008F5A00 | recurse | ECX = sub-bag, tokenizer, argument 2, 0 | `{` |
| 008F22D4 | 0043B8B0 | map find | ECX = bag+4, key, `&bucket` | `008F2260` |
| 008F232B | 008F2260 | recurse | ECX = record+0Ch, tail | dotted key, head is type 6 |
| 008F552B | 00480690 | iterator begin | ECX = iterator | `008F54F0` |
| 008F5545 | 00484D20 | iterator key | ECX = iterator; ret `char*` | per source entry |
| 008F559D | 0043B8B0 | map find | ECX = dest+4, key | per source entry |
| 008F55F6 | 008F54F0 | recurse | ECX = dest sub-bag, source sub-bag, flag | destination record is type 6 |
| 008F560D | 008F0700 | assign value | ECX = dest record, source record | found, not type 6, flag 0 |
| 008F561D | 008F4F60 | clone record | ECX = source record; ret record | key absent from dest |
| 008F563B | 008F33F0 | insert | ECX = dest, key, clone | key absent from dest |
| 008F5642 | 0047E480 | iterator advance | ECX = iterator | per source entry |
| 008F41FC | 00BF681B | `operator new(0x114)` | ret bag | `008F41F0` |
| 008F4247 | 00480690 | iterator begin | ECX = iterator | `008F41F0` |
| 008F4263 | 008F4F60 | clone record | ECX = source record | per source entry |
| 008F426C | 008F33F0 | insert | ECX = clone bag, key, record | per source entry |
| 008F4275 | 0047E480 | iterator advance | ECX = iterator | per source entry |
| 008F38BD | 00BF681B | `operator new(0x38)` | ret record | `008F38A0` |
| 008F38F5 | 00438E40 | duplicate string | ECX = value; ret `char*` | allocation succeeded |
| 008F3918 | 008F33F0 | insert | ECX = bag, key, record | always |
| 008F3381 | 00BF6989 | free | old `+0Ch` | type 2, `+0Ch` non-null |
| 008F3394 | 00438E40 | duplicate string | ECX = token | type 2 |
| 008F33AA | 0048E840 | `resolve_enum_value` | ECX = record+28h, name; ret int | type != 2 |

### Where the bag is read back

| Site | Callee | Containing function | What it does |
| --- | --- | --- | --- |
| 0046C59D | 008F2260 | 0046C550 | `MultiType` (`00CE580C`) |
| 0046C5B1 | 008F2260 | 0046C550 | `Party` (`00CE5804`) |
| 0046C623 | 008F41F0 | 0046C550 | clones the bag; the clone is the **first** of `004693C0`'s six stack arguments (`RET 18h`), pushed after the other five |
| 0046C69D | 008F2260 | 0046C550 | `MultiType` again on the second pass |
| 0046C7DC | 008F2260 | 0046C550 | `MultiSiege` (`00CE57F8`) |
| 0046DB5A | 008F41F0 | 0046D930 | clones `[EBX+4]`, where `EBX = [ESI+14h]` |
| 0046DB66 | 008F54F0 | 0046D930 | merges the bag at `[ESP+7Ch]` into that clone with `keep_existing = 1`, so the clone's own values win and the argument only fills gaps |

`0046D930` belongs to packet `cc2-scene-entity-create`; the two rows above are the call mechanics
only. Which of the two bags is the authored one and which is the class default is that packet's
contract, not a claim made here.

## Reconstruction

`include/bsp/scene_property_bag.hpp` and `src/scene_property_bag.cpp`. The offset and letter tables,
the reference-kind table, the counted-type strides with the saturating size rule, the value
decoders, the `008F2260` dotted lookup, the `008F33F0` ordinal rule, the `008F54F0` merge rule and
the `008F41F0` clone rule are pure. `scene_property_bag_parse` is the `008F5A00` sequence over
`ScenePropertyBagHost`, one virtual per native call site. Status: reconstructed and build-tested,
not ABI-compatible and not game-validated. One fixture in `tests/math_tests.cpp` decodes the
installed line `Vehicles = IA 8 17 19 20 21 23 68 73 109 ;`.

Coverage is complete for `008F5A00`'s assignment body, its group list and its sub-block branch, and
for `008F2260`, `008F33F0`, `008F3370`, `008F38A0`, `008F41A0`, `008F41F0` and `008F54F0` end to
end. Partial: the string reader `008EFB00`, the reference setters `008F0420`/`008F3AC0`, the array
setters `008F0580`/`008F3610`/`008F3590`, the payload copiers `008F0340`/`008F03B0` and the byte
decoder `008EF6D0` are call contracts whose bodies this packet did not read; the reconstruction
models each as one host call and does not reproduce their storage.

## Open questions

- `record+08h` is written by no producer read here and read by no consumer read here.
- `record+2Ch` is set to 1 by `008F38A0` and never inspected on the covered paths.
- The reference payload at `record+18h..1Fh` is bounded by the zeroing in `008F38A0` and by the
  array pointer at `+20h`, but its internal shape needs `008F0420` and `008F0340`.
- `008F5A00`'s remaining callers `008F67B0` and `009512F0` were not read.
