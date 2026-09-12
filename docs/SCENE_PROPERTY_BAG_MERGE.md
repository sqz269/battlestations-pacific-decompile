# Scene property bag merge (`008F54F0`, `008F23E0`) and the per-type value copy

Addresses: `008F54F0`, `008F23E0`, `008F4F60`, `008F0700`, `008F0340`, `008F03B0`, `008F03F0`,
`008F0640`, `008F0420`, `008F3AC0`, `008EF140`, `008EF2B0`, `008EF2F0`, `008EF360`, `008EF7F0`,
`004E6730`, `008F0DE0`, `008F38A0` (re-read), `008F5A00` group-merge site, `0046CF40` group loop,
`0046D930` merge site, `00469BF0` header-block sites, `008F67B0`, `009512F0`.

Packet `cc2_property_bag_merge`, read-only analysis. Every name below is a hypothesis, not a
recovered symbol, except `CPropTreeLibrary_Load` (`008F67B0`), whose literal is in the image.
Continues `docs/SCENE_PROPERTY_BAG.md`, which established the bag, the 38h record, the twelve type
codes and the eight reference letters; this packet closes that doc's open questions.

## How authored properties and class defaults combine

Precedence has two separate stages, and the flag means the opposite thing in each.

1. **Inside one `properties (G1, G2, ...) { ... }` block.** For a scene entity the group list is
   read by `BSP_SceneFile_ReadEntityBlock` (`0046CF40`), not by the bag parser: `0046D2C3`
   constructs the empty bag with `008F41A0` into EBX, then the loop `0046D2F4`-`0046D326` reads one
   group name per turn, looks it up with `00469B60` and merges it at `0046D30C` with
   **`keep_existing = 1`** (`0046D2F4 PUSH 0x1` survives the two intervening calls: `008D9980` takes
   no stack argument, as its zero-push site `008F5A9F` shows, and `00469B60` cleans its own). So
   among the groups the **first** one named wins a key that two of them declare. Only after the
   loop, at `0046D34E`, does `008F5A00` parse the body into that same bag, and the body's
   assignments set their keys directly, so an **authored line always beats every group default**.

   `008F5A00` has a group-merge branch of its own at `008F5AB3` that uses the opposite flag
   (`008F5AAE PUSH 0x0`, `008F5AB0 PUSH EAX`, so the source is argument 1 and the flag argument 2),
   where a later group overwrites an earlier one. It is gated on the group registry in argument 3,
   which `0046D34E` passes as 0, so an entity block never reaches it; the `.props` library loader
   `008F67B0` is what supplies a registry, for the parenthesised parent in `properties Ship(Common)`.
2. **At entity creation** (`0046D930`). `0046DB5A` clones `record->properties` (ECX = `[EBX+4]`,
   the bag the scene file produced for this entity) and `0046DB66` merges the caller's `overrides`
   argument (EDI, from `[ESP+7Ch]`) into that clone with `keep_existing = 1` (`0046DB61 PUSH 0x1`,
   `0046DB63 PUSH EDI`). So **the clone at `0046DB5A` is the authored bag** and the argument bag is
   a fallback: it can only add keys the authored record never mentioned. `docs/SCENE_ENTITY_CREATE.md`
   calls that argument `overrides`; with the flag at 1 it does not override anything.

Cross-check on the installed game: `universe/library/ship.props` declares the group `Ship(Common)`
with `Skill = E SkillLevels : SPNormal ;`, and `universe/scenes/missions/multi/scene175.scn` line
26815 opens an entity with `properties (Common, GameUnit, Ship, Command, MotherShipPlanes) {` whose
line 26845 authors `Skill = E SkillLevels : Elite ;`. The group loop puts SPNormal in the bag when
it reaches `Ship` (`Common`, named first, does not declare `Skill`) and the authored line replaces
it with Elite; stage 2 cannot undo that, because the entity's own bag is the merge destination. The
path was not executed, so this is a static reading of the files, not run-time evidence.

## The merge rule (`008F54F0`)

`__thiscall(ScenePropertyBag* dest /*ECX*/, ScenePropertyBag* source, char keep_existing)`, `RET 8`.
Argument count from the stack cleanup: `0046DB61`/`0046DB63` push two dwords and no `ADD ESP`
follows `0046DB66`; likewise at `00469D23` and `008F5AB3`. The destination is ECX (the lookup at
`008F5588` reloads the spill `008F550C` writes) and the iterator walks `[ESP+0x28]+4`, the first
stack argument.

Per source entry, in order:

| Case | Action |
| --- | --- |
| key absent from `dest` | `008F561D` clones the source record with `008F4F60`, `008F562E` copies the source's `+34h` ordinal onto the clone, `008F5636` inserts it with `008F33F0` |
| key present and the **destination** record is type 6 | `008F55F6` recurses on both sides' `+0Ch` with the same flag; `keep_existing` therefore applies at every depth |
| key present, any other type, `keep_existing == 0` | `008F560D` overwrites the destination's value through `008F0700` |
| key present, any other type, `keep_existing != 0` | nothing happens |

The type test at `008F55D0` reads the **destination's** `+04h`. A type-6 destination whose source
key is a scalar recurses into the sub-bag with a scalar's `+0Ch` as the source bag; nothing on the
covered paths produces that pair. Sub-bags merge recursively and are never replaced wholesale: a
nested block's keys are merged key by key. The clone branch is the only one that moves an ordinal,
so a key that already exists keeps the destination's insertion order even when its value is
overwritten.

## The assign-existing rule (`008F23E0`)

`__thiscall(ScenePropertyBag* dest /*ECX*/, ScenePropertyBag* source)`, `RET 4`. Register
provenance: `008F23FD MOV EDI,ECX` saves the destination, `008F243A` spills it, and `008F2493 MOV
ECX,[ESP+0x18]` reloads that spill (two pushes deeper) for the lookup; the iterator source is
`[ESP+0x30]`, the first stack argument, after the 0x2C bytes of prologue.

It is the merge with the clone branch and the flag removed, not a merge with `keep_existing = 0`:

- key absent from `dest` (`008F24A2` leaves BL = 0, `008F24D6` skips the body): **the entry is
  dropped**. Nothing is allocated and nothing is inserted.
- destination record type 6 (`008F24D8`): recurse at `008F24EF` on `[EDI+0Ch]` and the source
  record's `+0Ch`.
- otherwise: `008F2503` assigns through `008F0700`. The source always wins.

Callers: `00469D37`, inside `BSP_SceneFile_ReadHeaderBlock` (`00469BF0`), and its own recursion.
That header path shows both routines in sequence: `00469D09` parses the header's `properties` body
into the bag in ESI, `00469D23` merges the named default bag from `00469B60` with `keep_existing = 1`
(defaults fill gaps), and `00469D37` then applies the caller's bag from `[ESP+0x28]` with
assign-existing (values only, no new keys).

## The record is one class, not twelve subclasses

Correction to `docs/SCENE_PROPERTY_BAG.md`, which listed `+00h` as "vtable, one per type".
`008EF148` (type 0), `008EF2B0` (type 5), `008EF2F8` (type 8) and `008F38A0` (type 2) all store the
same pointer `00CE89D4`. Its single slot is `004E6730`, a scalar deleting destructor that forwards
through the thunk `008F0DE0` to `008F0640`, which switches on `+04h`. So the type code, not the
vtable, carries the type, and `008F0640` is the ownership map:

| Type | `008F0640` releases |
| --- | --- |
| 2 String | `free(+0Ch)` |
| 5 Reference | `free(+1Ch)`; `+08h` and `+18h` are just zeroed |
| 6 SubBag | the sub-bag's own deleting destructor with argument 1 |
| 8..0Bh arrays | `008F03F0`, which frees `+20h` and clears `+20h`/`+24h` |
| 0, 1, 3, 4, 7 | nothing; the tail only zeroes `+04h`, `+08h`, `+0Ch` and `+28h` |

`+28h` is never freed, so the enum/`LUA_S` declaration object at `+28h` is **shared, not owned**.

Ghidra's decompiler shows an early `return` immediately after the `free` in `008F0340`, `008F03B0`,
`008F03F0`, `008F0420` and `008F0640`. That is wrong. The bytes at `008F0355` are
`50 E8 2E663000 83C404 C746 04 00000000`, i.e. `PUSH EAX / CALL free / ADD ESP,4 / MOV [ESI+4],0`,
falling through to the `JZ` target `008F0365`; `008F03C4` has the same shape
(`... 83C404 C706 00000000`, then `008F03D3`). Every one of these routines frees the old storage,
nulls the field and **then** performs the copy.

## Per-type copy and assign

`008F4F60` `BSP_SceneProperty_Clone`, `__thiscall(record /*ECX*/)`, plain `RET`, returns a new
record. All twelve arms are now read; every one allocates `operator_new(0x38)`, runs one
constructor and copies `+34h` onto the result. `008F0700`
`BSP_SceneProperty_AssignValueFrom`, `__thiscall(dest /*ECX*/, source)`, `RET 4`, switches on the
**destination's** code and writes into a record that already has its type.

| Code | Clone arm (`008F4F60`) | Assign arm (`008F0700`) | Deep-copied | Shared |
| --- | --- | --- | --- | --- |
| 0 Int | `008EF140`: `+0Ch` dword | `008F071A`: `+0Ch` dword | value only | — |
| 1 Float | `008EF170`: `+0Ch` dword | `008F0726`: `+0Ch` dword | value only | — |
| 2 String | `008EF1B0(+0Ch)`, duplicates | `008F074C` frees `+0Ch`, `008F0758` duplicates `other+0Ch` | the string | — |
| 3 Bool | `008EF1F0(byte +0Ch)` | `008F0736`: one byte | value only | — |
| 4 Enum | `008EF230(+28h, +0Ch)` | `008F0790`: `+0Ch` dword only | the value | the declaration at `+28h` (clone carries it, assign leaves the destination's) |
| 5 Reference | `008EF2B0(+1Ch, +18h, +08h)` | `008F0771`: `008F0340(other+18h)` with ECX = `dest+18h` | the target name at `+1Ch` | — |
| 6 SubBag | `008F41F0` then `008EF780` | **no arm**; the switch falls to the tail | the whole sub-tree | — |
| 7 Vector3 | `008EF270(&record+0Ch)` | `008F0796`..`008F07A5`: `+0Ch`, `+10h`, `+14h` | three floats | — |
| 8 ByteArray | `008EF2F0(+20h, +24h, 1)` | `008F07B6`: `008F03B0(other+20h)` with ECX = `dest+20h` | the block at `+20h` | — |
| 9 FloatArray | `008EF360(count, +20h, 1)` after `008F51D6 008EF7F0` | same shared arm as 8 | the block | — |
| 0Ah IntArray | `008EF3E0(count, +20h, 1)`, same shape | same shared arm as 8 | the block | — |
| 0Bh V3Array | `008EF460(count, +20h, 1)`, same shape | same shared arm as 8 | the block | — |
| other | `default: return 0` | tail, no write | — | — |

Two asymmetries are real, not transcription slips:

- **The enum declaration.** The clone passes `+28h` to `008EF230`; the assign arm for code 4 writes
  only `+0Ch`, so an assigned enum keeps the destination's declaration and takes the source's
  resolved integer. Two different enum tables on the two sides would silently mix.
- **The reference kind.** The clone passes `+08h` to `008EF2B0`; `008F0340` moves exactly two
  dwords, `+18h` and `+1Ch`, so an assigned reference keeps the destination's kind and takes the
  source's target name.

The third stack argument of `008EF2F0` / `008EF360` / `008EF3E0` / `008EF460` is a copy flag:
`008EF2F0` allocates and `memcpy`s when it is non-zero and stores the caller's pointer verbatim when
it is zero. The clone passes 1 in every arm (`008F518C`, `008F51D1`), so a cloned array never shares
its block.

## Arrays: `+24h` is a byte size

Correction to `docs/SCENE_PROPERTY_BAG.md`, which listed `+24h` as "array element count".
`008EF2F0` stores its `size` argument at `+24h` and passes the same value to `operator_new` and
`memcpy`; `008F03B0` does the same on an assign; `008EF360` computes `count * 4` before storing.
`008EF7F0`, `__fastcall(record /*ECX*/)` (`008F51D4 MOV ECX,ESI` with ESI the source record), is the
inverse and the only place a count is recovered: `>> 2` for codes 9 and 0Ah, `/ 0Ch` for 0Bh, and 0
for every other code, including 8, whose count and byte size are the same number.

## The reference payload

Three fields, written together by `008F0420` (declared key), `008F3AC0` (undeclared key, which
allocates the record itself and inserts it) and the clone constructor `008EF2B0`:

| Offset | Field | Evidence |
| --- | --- | --- |
| `+08h` | `SceneReferenceKind`, 0..7 (`R`, `RLand`, `RFort`, `RShip`, `RPlane`, `RPlnShp`, `RLPlnShp`, `RPath`) | `008F0451 MOV [ESI+8],ECX` from the third stack argument; `008EF2B0` `param_1[2] = param_4`; the type-5 clone arm reads `+08h` back and passes it to `008EF2B0` |
| `+18h` | a dword the parser always passes as 0 | `008F044E MOV [ESI+0x18],EAX` from the second stack argument; both parser sites (`008F60C1`, `008F6254`) pass 0; copied by `008F0340` |
| `+1Ch` | the target name, an owned heap duplicate | `008F0423`..`008F0447`: free the old, `00438E40` duplicate the first stack argument, store; freed by `008F0640` case 5 |

`008F0420` is `__thiscall(record /*ECX*/, const char* name, dword tag, dword kind)`, `RET 0Ch` — the
argument count straight from the listing. `+08h` was `docs/SCENE_PROPERTY_BAG.md`'s "not written by
any producer read here" gap; it is written by every reference producer and is part of this payload,
not of the scalar area. The target name is stored as text and is **not** resolved at parse time:
the installed files carry empty ones (`RepairZoneArea = R "" ;` in `universe/library/ship.props`,
`ManagerCameraPos = R ""` in the training scene), and `+18h` is the natural home for a resolved
handle, but no routine read in this packet writes it with anything but 0.

## `record+2Ch`

Still unresolved, and now bounded on the writer side: `008F38A0` (type 2), `008EF2B0` (type 5),
`008EF2F0` (type 8), `008EF360` (type 9) and `008F3AC0` all store the byte 1 at `+2Ch` as their last
field, and no constructor read here stores 0. No routine read in this packet loads it: not the
clone, not the assign, not the merge, not `008F0640`, not the parser body. Provisional reading: a
per-record flag fixed at construction, plausibly "owns its storage", with no observed reader.

## `008F5A00`'s remaining callers

| Address | What it is |
| --- | --- |
| `008F67B0` | `CPropTreeLibrary_Load`, already documented in `docs/GAME_EXECUTABLE.md` milestone 2h: the `.props`/`.enums` library loader that produces the very group bags stage 1 merges. Recovered symbol (`CPropTreeLibrary::Load ` at `00D1653C`). |
| `009512F0` | A tokenizer-driven entity block reader: `BSP_SceneTokenizer_ExpectToken` for four fixed literals, a name token resolved through `BSP_EntityRegistry_FindEntityByName`, then, when the next token compares equal to `properties`, `BSP_ScenePropertyBag_Construct(0)` on a stack bag, `BSP_ScenePropertyBag_Parse(param_1, 1, 0)` into it, a hand-off to `0049CF80`, and the bag destructor `008F5410`. Read to its entry and its parse call only; `0049CF80` is unread, so what it does with the bag is open. |

## Host table

One row per native call site read in this packet. The containing function is the Ghidra body the
site falls in.

| Site | In | Callee | Name | this / args / ret | Gate |
| --- | --- | --- | --- | --- | --- |
| `008F23FF` | `008F23E0` | `00BF681B` | `operator new` (existing) | arg `0Ch`, `__cdecl`, `ADD ESP,4` at `008F2408` | — |
| `008F241A` | `008F23E0` | `00480690` | map iterator begin (existing) | ECX = the 0Ch block whose `+00h` is `source+4` | allocation succeeded |
| `008F2446` | `008F23E0` | `00484D20` | iterator key (existing) | ECX = the iterator; returns `char*` | loop |
| `008F2481` | `008F23E0` | `00BF7680` | `memcpy` (existing) | key text into the local string | key length non-zero |
| `008F249B` | `008F23E0` | `0043B8B0` | case-insensitive map find (existing) | ECX = the destination bag, args key, out-slot | loop |
| `008F24EF` | `008F23E0` | `008F23E0` | `BSP_ScenePropertyBag_AssignExistingFrom` | ECX = `dest+0Ch`, arg source record's `+0Ch`; `RET 4` | found and destination type 6 |
| `008F2503` | `008F23E0` | `008F0700` | `BSP_SceneProperty_AssignValueFrom` | ECX = the destination record, arg the source record; `RET 4` | found, not type 6 |
| `008F250E` | `008F23E0` | `0047E480` | iterator advance (existing) | ECX = the iterator | loop |
| `008F2523` | `008F23E0` | `00BF65AC` | `free` (existing) | arg the iterator | after the loop |
| `008F5099` | `008F4F60` | `008EF1B0` | string record constructor | ECX = the 38h block, arg `source+0Ch` | type 2 |
| `008F50D1` | `008F4F60` | `008EF230` | enum record constructor | ECX = the block, args `source+28h`, `source+0Ch` | type 4 |
| `008F5192` | `008F4F60` | `008EF2F0` | byte-array record constructor | ECX = the block, args `source+20h`, `source+24h`, `1` | type 8 |
| `008F51D6` | `008F4F60` | `008EF7F0` | `BSP_SceneProperty_ArrayElementCount` | ECX = the **source record**; returns `+24h` divided by the stride | types 9, 0Ah, 0Bh |
| `008F51DE` | `008F4F60` | `008EF360` | float-array record constructor | ECX = the block, args count, `source+20h`, `1` | type 9 |
| `008F074C` | `008F0700` | `004357F0` | string field release (existing) | ECX = `dest+0Ch` | type 2 |
| `008F0758` | `008F0700` | `00438E40` | `BSP_NativeString_Duplicate` (existing) | ECX = `source+0Ch` | type 2 |
| `008F0771` | `008F0700` | `008F0340` | `BSP_SceneProperty_CopyReferencePayload` | ECX = `dest+18h`, arg `source+18h`; `RET 4` | type 5 |
| `008F07B6` | `008F0700` | `008F03B0` | `BSP_SceneProperty_CopyArrayBlock` | ECX = `dest+20h`, arg `source+20h`; `RET 4` | types 8..0Bh |
| `008F0356` | `008F0340` | `00BF6989` | `free` (existing) | arg the old `+1Ch`; `ADD ESP,4` then `MOV [ESI+4],0` | old name non-null |
| `008F0367` | `008F0340` | `00438E40` | `BSP_NativeString_Duplicate` (existing) | ECX = the source name | always |
| `008F03C5` | `008F03B0` | `00BF6989` | `free` (existing) | arg the old `+20h`; then `MOV [ESI],0` | old block non-null |
| `008F03D7` | `008F03B0` | `00BF55BE` | `operator new` (existing) | arg the source byte size | always |
| `008F03E1` | `008F03B0` | `00BF7680` | `memcpy` (existing) | args new block, source block, byte size; `ADD ESP,0x10` | always |
| `008F042B` | `008F0420` | `00BF6989` | `free` (existing) | arg the old `+1Ch` | old name non-null |
| `008F043E` | `008F0420` | `00438E40` | `BSP_NativeString_Duplicate` (existing) | ECX = the name argument | always |
| `0046D2C3` | `0046CF40` | `008F41A0` | bag constructor (existing) | ECX = the 114h allocation from `0046D2A7` | entity has a property block |
| `0046D2F8` | `0046CF40` | `008D9980` | `BSP_SceneTokenizer_ReadToken` (existing) | ECX = the tokenizer, no stack argument | group loop |
| `0046D304` | `0046CF40` | `00469B60` | property-group lookup by name (existing) | ECX = `*(00E18678)`, arg the group token | group loop |
| `0046D30C` | `0046CF40` | `008F54F0` | `BSP_ScenePropertyBag_MergeFrom` | ECX = the entity's bag, args the group bag, `1`; `RET 8` | group loop |
| `0046D34E` | `0046CF40` | `008F5A00` | `BSP_ScenePropertyBag_Parse` (existing) | ECX = the entity's bag, args the tokenizer, `1`, `0` | after the group loop |
| `008F5AA9` | `008F5A00` | `00469B60` | property-group lookup by name (existing) | ECX = the library, arg the group token | group list, registry non-null |
| `008F5AB3` | `008F5A00` | `008F54F0` | `BSP_ScenePropertyBag_MergeFrom` | ECX = the block's bag, args the group bag, `0`; `RET 8` | group list, registry non-null |
| `00469D09` | `00469BF0` | `008F5A00` | `BSP_ScenePropertyBag_Parse` | ECX = the header bag | header has a `properties` body |
| `00469D1B` | `00469BF0` | `00469B60` | property-group lookup by name (existing) | ECX = `*(00E18678)`, args `00CE567C`, `1` | same |
| `00469D23` | `00469BF0` | `008F54F0` | `BSP_ScenePropertyBag_MergeFrom` | ECX = the header bag, args that group bag, `1` | same |
| `00469D37` | `00469BF0` | `008F23E0` | `BSP_ScenePropertyBag_AssignExistingFrom` | ECX = the header bag, arg `[ESP+0x28]`; `RET 4` | `EBP != 0` (`00469D2A`) |
| `0046DB5A` | `0046D930` | `008F41F0` | `BSP_ScenePropertyBag_Clone` (existing) | ECX = `record->properties` | `overrides != 0` |
| `0046DB66` | `0046D930` | `008F54F0` | `BSP_ScenePropertyBag_MergeFrom` | ECX = the clone, args `overrides`, `1`; `RET 8` | same |

## Coverage

| Routine | Coverage |
| --- | --- |
| `008F54F0` | complete, re-verified against the listing for the argument order and the flag |
| `008F23E0` | complete: `008F23E0`-`008F252D`, every branch |
| `008F4F60` | complete: all twelve arms plus the `default: return 0` tail |
| `008F0700` | complete: all twelve codes, including the missing code-6 arm |
| `008F0340`, `008F03B0`, `008F03F0` | complete, from the bytes; the decompiler's early return is a mis-model |
| `008F0420` | complete: `008F0420`-`008F0457` |
| `008F3AC0` | complete for the record it builds; its `008F33F0` insert is the existing rule |
| `008F0640` | complete as a switch; the per-type destructors it does not call are simply absent |
| `008EF2B0`, `008EF2F0`, `008EF360`, `008EF7F0` | complete |
| `008EF140`, `008EF170`, `008EF1B0`, `008EF1F0`, `008EF230`, `008EF270`, `008EF3E0`, `008EF460`, `008EF780` | analyzed as call contracts from their call sites and their stored type codes; bodies not read except `008EF140`'s first instructions |
| `009512F0` | partial: entry through the `properties` branch (`009512F0`-`00951335` approximately); `0049CF80` unread |
| `008F67B0` | not re-read; already complete in `docs/GAME_EXECUTABLE.md` |
| `0046CF40` | partial: the property-block branch `0046D2A2`-`0046D353` only; the rest of the entity block belongs to `docs/SCENE_FILE_READER.md` |

## Reconstruction

`include/bsp/scene_property_bag_merge.hpp` and `src/scene_property_bag_merge.cpp`, registered on
`bsp_core`. They reuse the record types of `include/bsp/scene_property_bag.hpp` and add only what
this packet established: the reference payload, the two per-type copy specifications, the element
count rule, the `008F0700` value assignment, the `008F23E0` bag rule, and `ScenePropertyMergeHost`
with one virtual per native call site for the two walks. Status: reconstructed and build-tested, not
ABI-compatible and not game-validated. The value assignment refuses a type-mismatched pair instead
of reinterpreting the dword the way the native arm does; that divergence is marked in both files.
One case in `tests/math_tests.cpp` covers the `008F23E0` precedence and the dropped key.

## Open questions

- `record+2Ch`: written 1 by every constructor, no reader anywhere in this packet.
- `record+18h`: copied on both a clone and an assign, but every producer read so far passes 0.
  Whatever writes a non-zero value is outside the parser and the copy paths.
- `0049CF80`, the consumer of the bag `009512F0` builds.
- Who calls `0046D930` with a non-null `overrides` bag, and what it holds. The callers are
  `BSP_Game_SpawnEntityByName` (`004C6BA0`, which forwards its arguments unchanged),
  `BSP_SceneRecord_ScatterClouds` (`004BA870`), `BSP_LuaBinding_GenerateObject` (`00944FD0`),
  `BSP_LuaBinding_Spawn` (`00944680`) and `00945450`; none of their argument setups were read.
- Whether any path pairs a type-6 destination with a non-bag source, which would hand `008F55F6`
  a scalar as a bag.
