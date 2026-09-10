# Scene file reader (`0046DF00`, the `.scn` format)

Addresses: 0046df00 0046cf40 00469bf0 00467e10 00469e40 0046a9f0 0046aab0 008d9cf0 008d8a70 008d8960 008d9930 008d9980 008d9ad0 008d9b40 008f5a00 00469b60 0046c550 00413920

Packet `scene_file_reader`, worktree `agent/scn-reader`. Ghidra was read-only for this packet;
every name below is a hypothesis, not a recovered symbol.

## The format is text

`.scn` is a **plain-text, brace-structured record format**, not Lua and not binary. The files ship
uncompressed on disk under `universe/scenes/`. A file is a flat sequence of top-level blocks:

```
header { ... }
entity "Houston" (DestroyerGen) { ... }
entity ...
traffic { ... }
groups { ... }
SceneBrowserGroups { ... }
```

`0046DF00` proves this three ways: it constructs the tokenizer at `008D9CF0` with the delimiter
literal `;{}=:(,)` (00CE4F40), it dispatches the top-level loop on the literals `header`, `entity`,
`traffic`, `groups` and `SceneBrowserGroups`, and every installed file parses under the recovered
grammar (see **Validation**).

### Tokenizer (`008D9CF0` construct, `008D8A70` peek, `008D8960` consume)

The tokenizer object is `0x838` bytes. Fields established from `008D8A70` and `008D9CF0`:

| Offset | Use |
| --- | --- |
| `+0h` | vtable, `00D15FE4` |
| `+4h` | current token was quoted |
| `+5h` | current token text, `0x400` bytes |
| `+405h` | previously consumed token text, `0x400` bytes |
| `+805h` | lookahead cache valid (peek already scanned) |
| `+806h`/`+807h` | current and lookahead character |
| `+808h` | lookahead character valid |
| `+809h` | end of buffer reached |
| `+80Ah` | end of buffer at the start of this token |
| `+818h` | delimiter set pointer |
| `+81Ch` | 1-based line counter |
| `+820h`/`+824h` | file name, native `{length, pointer}` pair |
| `+828h` | VFS stream, opened through `[0109CEEC]` vtable `+4h` with mode `0x32` |
| `+82Ch` | whole-file buffer, `malloc` of the stream size |
| `+830h` | buffer size, stream vtable `+30h` |
| `+834h` | read cursor |

Scanning rules, all from `008D8A70`:

- **Whitespace** is `" \t\r\n,"` (`PTR_s___00e0c940` -> `00D15F2C`). **The comma is whitespace, not
  a delimiter**, so `properties (Common, Ship)` and `properties (Common Ship)` tokenize alike.
- **Delimiters** are the built-in default `";"` (`00CE5698`) concatenated with the caller's set. For
  `.scn` that is `;{}=:(,)`, so `; { } = : ( , )` each form a one-character token. `:` being a
  delimiter is why `Command = E CommandType : Cruise ;` splits into seven tokens.
- `//` runs to end of line, `/* */` to the matching close. `--` is **not** a comment: see the
  `--Skill = ...` lines in the shipped files under **Error handling**.
- `"` opens a quoted run that ends at the next `"` with **no escape processing**; the token carries
  the quoted flag in `+4h`. That flag is the only way to tell an empty quoted string from EOF, and
  the entity and header loops both test it.
- Anything else accumulates until whitespace or a delimiter.
- Peek is idempotent (`+805h` caches); `008D8960` copies the token to `+405h` and clears the cache.

### Parser primitives

| Address | Name | ABI | Behaviour |
| --- | --- | --- | --- |
| `008D9930` | `BSP_SceneTokenizer_ExpectToken` | `__thiscall(const char*)`, RET 4 | peek; consume **only** on a case-insensitive match; otherwise report through `008D8F70` and leave the token in place |
| `008D9980` | `BSP_SceneTokenizer_ReadToken` | `__thiscall(void)`, RET 0 | peek; consume unless it is EOF or an unquoted empty token; returns `this+5h` |
| `008D9AD0` | `BSP_SceneTokenizer_ReadInt` | `__thiscall(bool* ok)`, RET 4 | `sscanf(token, "%d")`; consume only on success |
| `008D9B40` | `BSP_SceneTokenizer_ReadFloat` | `__thiscall(bool* ok)`, RET 4 | `sscanf(token, "%f")`; consume only on success, result in ST0 |

That "consume only on success" rule is the whole error-recovery story; see **Error handling**.

## Grammar

```
file      := ( header | entity | traffic | groups | scenebrowsergroups | <ignored token> )*

header    := "header" "{" ( "uniqueID" INT ";"
                          | "NextUID"  INT ";"
                          | "properties" propsection
                          | "precache"   propsection
                          | <ignored token> )* "}"

entity    := "entity" NAME "(" CLASS ")" "{"
                "localframe" FLOAT x16 ";"
                [ "template" TOKEN ";" ]
                [ "uid" INT ";" ]
                "properties" propsection
                entity*
             "}"

propsection := [ "(" TOKEN* ")" ] "{" propbody "}"
propbody    := ( KEY "{" propbody              # nested sub-block
               | KEY "=" LETTER TOKEN* ";"     # assignment
               | KEY                           # neither: dropped, scan resumes
               )* "}"

groups    := "groups" "{" ( "group" "{" "GroupName" "=" TOKEN "Index" "=" INT "}" )* "}"
```

`traffic` and `SceneBrowserGroups` bodies are brace-balanced blocks this packet did not decode;
`004694F0` reads one `SceneBrowserGroups` entry and `009514B0` reads the `traffic` body.

Value type letters observed in the shipped files: `S F I E B R V3 RPath RFort RPlnShp IA LUA_S`.
The letter is **validated**, not dispatched on: `008F5A00` compares it against the type the property
descriptor already declares (type codes 0..0Bh, letter strings at `00D16504`..`00D16528` and
`00D162D4`), so an unknown key falls back to the integer reader.

## The three passes

`0046DF00` is `__thiscall`, `RET 0x18`, six stack arguments:

```
0046DF00(this = [00E18680], const char* scenePath, void* a2, char instantiate,
         void* sceneRecord, const char* overrideName, char registration)
```

`docs/MISSION_SCENE_LOAD.md` records all three call sites. The last two arguments select the pass:

| Pass | `instantiate` (arg 3) | `registration` (arg 6) | Caller | What it does |
| --- | --- | --- | --- | --- |
| 1 Header | 0 | 0 | `004DFB70` at `004E0xxx` | weather descriptor, `header` block only; the entity loop is skipped outright (`0046E9D5`) |
| 2 Registration | 0 | 1 | `004D4DF0`, first call | header, then entities with the registration flag; **stops at the first `traffic`, `groups` or `SceneBrowserGroups`** |
| 3 Instantiate | 1 | 0 | `004D4DF0`, second call | header, entities fully constructed, then all four tail blocks, then `0046AAB0` |

The gate is `0046E9D5: CMP byte [EBP+10h],0 ; JNZ ... ; TEST BL,BL ; JZ 0046ED14`: the entity loop
runs when either flag is set, and pass 1 falls straight through to the epilogue.

Prologue common to every pass:

1. `0046A9F0` on `this+14Ch` empties the pending deferred-reference list.
2. `new(0x838)` then `008D9CF0(tokenizer, NativeString(scenePath), ";{}=:(,)")` opens the file.
3. The scene path is split on the **last** `.`: the whole path goes to `this+13Ch/+140h`, the text
   before the dot to `this+144h/+148h`, and the text from the dot onward to the scratch buffer
   `00E18668`. An empty path leaves the stem empty (`00CE3A0C`).
4. The `Weathers` table of `SCRIPTS\datatables\Weather.lua` (`00CE59DC`) is walked. The entry whose
   name matches is scanned for `SubScenes` rows carrying `sceneFile`, `ID` and `Descriptor`.
5. When `overrideName` is null or empty, the matched descriptor's shadow keys are pushed into four
   console variables; otherwise the descriptor is applied wholesale and the shadow keys are skipped.

Epilogue: the tokenizer is destroyed through its vtable, and pass 3 only then runs `0046AAB0`.

### Terrain keys

`0046DF00` writes exactly four `g_Terrain.*` console variables, each looked up by full name through
`008F2260` on the descriptor set and written at `+0Ch`:

| Descriptor key | Console variable | Written as |
| --- | --- | --- |
| `g_StaticShadowTexture` (00CE597C) | `g_Terrain.g_StaticShadowTexture` (00CE595C) | string, `008F3370` |
| `ga_StaticShadowShotOffsetX` (00CE5940) | `g_Terrain.ga_StaticShadowShotOffsetX` (00CE5918) | float at `+0Ch` |
| `ga_StaticShadowShotOffsetZ` (00CE58FC) | `g_Terrain.ga_StaticShadowShotOffsetZ` (00CE58D4) | float at `+0Ch` |
| `ga_StaticShadowShotSize` (00CE58BC) | `g_Terrain.ga_StaticShadowShotSize` (00CE5898) | float at `+0Ch` |

There is no `Y` offset variable, and there is no terrain key inside the `.scn` file itself. The one
terrain datum the file contributes is the landscape entity's synthetic `__EXTDATAPATH` (below).

## The record the reader fills

Argument 4 is the mission scene record `docs/MISSION_SCENE_LOAD.md` describes. `0046DF00` itself
never touches it; `00469BF0` does, and only for two fields:

| Offset | Source | Site |
| --- | --- | --- |
| `+905h` | set to 1 when the header contains a `precache` block | `00469DAx` |
| `+1098h` | the `uniqueID` integer | `00469E0x` |

`+1098h` is the mission id `004E183D` publishes to `[00F8A2FC]+48h`, which independently confirms
the record identity. `+90Ch/+910h` is the scene path the caller derived and passed in as argument 1;
the reader reads it, never writes it.

Scene-database fields (`this`, the object behind `[00E18680]`):

| Offset | Use |
| --- | --- |
| `+4h` | non-zero enables the `Hidden` property test in `0046CF40` |
| `+13Ch/+140h` | scene path, native string pair |
| `+144h/+148h` | scene path with the last extension removed; the `%s` of `%s_LS_%i.trn` |
| `+14Ch` | pending deferred-reference list `{count, head, tail}`, cleared by `0046A9F0` |
| `+150h` | deferred-reference list walked by `0046AAB0` |

## The entity block (`0046CF40`)

`__thiscall`, `RET 0x58`. From the call site at `0046EB0F`:

```
0046CF40(this, Tokenizer* t, void* a2 = 0, void* a3, const char* overrideName,
         float parentFrame[16] /* by value, 0x40 bytes */, int a6 = 1, char registration)
```

Read order, with the exact keys:

1. `008D9980` -> the quoted entity name (256-byte stack buffer). `00467CF0` searches it for `_W_`
   (00CE5854) and, when found, `00425850` tests the substring; the result gates instantiation.
2. `(` `<class>` `)` — the class token is interned and resolved to a class descriptor by `00468FB0`;
   `*descriptor` is the numeric class id used later.
3. `{` then `localframe`, sixteen floats through `008D9B40`, then `;`. The basis is renormalised:
   the length of the first row is computed and rows 0..2 of the 3x3 part are scaled by it.
4. Optional `template <token> ;` (00CE5834).
5. Optional `uid <int> ;` (00CE5830).
6. `properties` (00CE568C), then the optional `( g1 g2 ... )` group list. Each name is interned by
   `00469B60` against `[00E18678]` and added to the bag by `008F54F0`. Commas are whitespace, so the
   authored `(Common, Command, GameUnit, Ship)` yields exactly four groups.
7. `{ ... }` — the property body, parsed by `008F5A00` into a `0x114`-byte bag.
8. Zero or more nested `entity` blocks. Each child's `localframe` is composed with the parent's
   world frame by `00413920` (`BSP_Matrix_Multiply4x4`) before recursing into `0046CF40`. Nesting is
   real: the shipped files reach depth 4.
9. `}`.

Synthetic property, the landscape hook (`0046D353`..`0046D398`): with a non-zero `uid`, a non-empty
scene stem at `this+144h`, and class id `0x44`, the bag gets

```
__EXTDATAPATH = sprintf("%s_LS_%i.trn", sceneStem, uid)
```

Every other entity gets `__EXTDATAPATH = ""`. `%s_LS_%i.trn` is at `00CE55D4`, `__EXTDATAPATH` at
`00CE5820`. This is how a `Landscape` entity names its terrain tile.

Properties `0046CF40` reads back out of the bag it just filled:

| Key | Literal | Use |
| --- | --- | --- |
| `Hidden` | 00CE5708 | when set (and `this+4h` non-zero) the normal instantiation is skipped and the hidden branch runs instead |
| `Type` | 00CE4780 | read on the registration pass after a successful instantiation |
| `Party` | 00CE5804 | read on the hidden branch; a `0x5C`-byte record is allocated and filled with the name and party |

### How entities are instantiated

`0046C550` is the single instantiation call, reached as
`0046C550(classFactory, name, a3, frame[16], propertyBag)` after `00468660` maps the class id to its
factory. This packet treats it as a contract over the entity/resource code it belongs to and does
not reimplement it. What the reader guarantees to it:

- the name, class descriptor, composed world frame and the fully parsed property bag,
- `__EXTDATAPATH` already present in the bag,
- the registration flag, which on pass 2 restricts work to the class ids `0x47`, `0x19`, `0x1B`,
  `0x1C`, `0x34` and `0x4D` (checked through `00469690`) plus the `0x4D`/`0x44` fallback, and on
  pass 3 lets the full construction and the `piVar1[2]` virtual run.

The pass-2 tail of `0046DF00` (`0046ED14`..`0046EF62`) re-walks the collected class ids, skipping
`0x47`, and for each one reads a template name out of the weather/descriptor table and compares it
against `CommandBuilding` (00CE5870), then `00425850` twice, before `0095C640`. The literals
`LandVehicle` (00CE5858), `LandFort` (00CE5864) and `VehicleClass` (00CE5880) sit in that same
comparison block. That tail is the least certain part of this packet.

## The tail blocks

| Keyword | Literal | Handler | ABI | Body |
| --- | --- | --- | --- | --- |
| `traffic` | 00CE5890 | `009514B0` (pass 3) / `0095CA10` (pass 2, then stop) | not read | not decoded |
| `groups` | 00CE55E4 | `00467E10` | `__thiscall(Tokenizer*)`, RET 4 | `group { GroupName = <token> Index = <int> }`, repeated |
| `SceneBrowserGroups` | 00CE56A4 | `00469E40` | `__thiscall(Tokenizer*)`, RET 4 | repeats `004694F0`, one entry per iteration; the entry layout was not read |

`00925F20(CL = 0)` runs once before the first tail block of pass 3 and once more at the end if no
tail block appeared.

## Deferred references (`0046A9F0`, `0046AAB0`)

`0046A9F0` is `__thiscall(void)`, `RET` 0, on a `{count, head, tail}` triple. **Ghidra shows it
popping a single node; the listing is a loop.** Two fall-through gaps after the `_free` call at
`00BF65AC` were dropped because that helper carries a no-return annotation:

```
0046aa17: add esp, 4                    0046aa52: add esp, 4
0046aa1a: mov dword ptr [esi+8], 0      0046aa55: cmp dword ptr [edi], 0
                                        0046aa58: jne 0x46aa00      <- the loop back edge
```

So `0046A9F0` **empties** the list. `python tools/bsp.py ghidra flow 0046a9f0` reports both gaps;
they are not repaired here because Ghidra is read-only for this packet.

`0046AAB0` is `__thiscall(void)`; it ends `POP EBX ; ADD ESP,0x20 ; JMP 0046A9F0`, a tail call, so
its effective RET is 0. It walks `this+150h`, and for each node scans the global list at `00E19A70`
comparing names case-insensitively; on a match it takes either the referenced object's position
(`+FCh/+100h/+104h`) or, when `[node+8]+0Ch` is non-zero, a named lookup through `00925A90` and the
globals `00F87574..0100F8757C`. It then clears `this+14Ch`. This is the pass that resolves the `R`,
`RPath`, `RFort` and `RPlnShp` property values written as names in the file.

## Error handling

There is no failure path. `008D9930`, `008D9AD0` and `008D9B40` report through `008D8F70` and then
**leave the offending token unconsumed**, so the next step of the grammar sees it and usually
recovers on its own. Three consequences, all visible in the shipped files:

- `template "Japan\SEA\Takao-class"` with the `;` missing: the `expect(";")` fails, `properties` is
  still the next token, and the entity parses correctly.
- `localframe ... 1.- ...`: `sscanf("%f")` converts the prefix `1.` and returns 1, so the value is
  accepted as 1.0.
- `-- Skill = E SkillLevels : Stun ;`: `--` is a normal token, not a comment. It becomes a property
  key, is followed by neither `=` nor `{`, and `008F66B9` drops it and resumes at `Skill`. **The
  line is therefore not commented out; the property is applied.** The variant `--=--SceneGroupNum =
  I 8 ;` binds `--` to the letter `--SceneGroupNum` and drops the whole line instead.

An unrecognised header key or top-level keyword is consumed and ignored. A missing file is not an
error either: `008D9CF0` leaves the buffer empty when the provider returns no stream, and every loop
terminates on the end-of-buffer flag.

## Validation against the installed game

`local/scn_ref.py` is an independent Python implementation of the tokenizer, the grammar and the
three recovery rules above, written from the recovered rules only. Run read-only over
`I:/SteamLibrary/steamapps/common/Battlestations Pacific`:

| Measure | Value |
| --- | --- |
| `.scn` files found | 259 |
| files parsed to end of input | 259 |
| files parsed with no recovered error | 250 |
| files needing the native recovery rules | 9 |
| entities parsed (including nested) | 133655 |
| entities per file, min / median / max | 0 / 329 / 2720 |
| maximum entity nesting depth | 4 |
| distinct entity classes | 23 |
| distinct property keys | 172 |
| unknown top-level keywords | 0 |
| unknown header keys | 0 |

Top-level keyword counts: `header` 259, `entity` 30727 (top level only), `traffic` 251,
`groups` 259, `SceneBrowserGroups` 249. Every file has a `header` and a `groups` block; ten have no
`SceneBrowserGroups` and eight no `traffic`. Every file carries both `uniqueID` and `NextUID`; one
(`COTP-IJN/ijn_02_force_z.scn`) carries a `precache` block.

Entity classes, by count: `LandFort` 71388, `Path` 24886, `DestroyerGen` 9634, `Cloud` 7078,
`LandingPoint` 3542, `NavPoint` 3457, `Stationary` 2233, `PlaneSquadronGen` 2091, `SpawnPoint` 1654,
`MotherShipGen` 1059, `WaterMine` 1013, `Landscape` 744, `TBoatGen` 650, `CommandBuilding` 646,
`LandingShipGen` 390, `SubmarineGen` 326, `AirField` 253, `Shipyard` 238, `MovieCamPos` 77,
`MovieCamLookat` 77, `LandConvoy` 31, `SimpleEffect` 11, `Landfort` 3 (a case variant, which the
case-insensitive compare folds).

The nine files that need the recovery rules, and why:

| File | Errors | First site |
| --- | --- | --- |
| `COTP-USN/PRCPUS/usn_09_leyte.scn` | 3 | line 11954, `template` line missing its `;` |
| `ijn/ESMP/05_philippine_sea.scn` | 4 | line 23435, entity block unclosed before `traffic` |
| `ijn/ijn_11_operation_to.scn` | 1 | line 9711, `template` line missing its `;` |
| `ijn/ijn_8_north_sol.scn` | 1 | line 42933, entity body opening on a float, not `localframe` |
| `multi/scene12.scn` | 1 | line 46101, `template` line missing its `;` |
| `multi/scene175.scn` | 4 | line 28775, entity block unclosed before `traffic` |
| `multi/scene901.scn` | 1 | line 3872, `localframe` line missing its `;` |
| `multi/scene906.scn` | 2 | line 37099, entity body opening on a float |
| `multi/scene907.scn` | 4 | line 30703, entity block unclosed before `traffic` |

None of the 259 files contains a construct the recovered grammar does not cover.

## Reconstruction

`include/bsp/scene_file.hpp` and `src/scene_file.cpp` carry:

- `bsp::SceneLexer`, the tokenizer of `008D8A70` with the recovered whitespace set, delimiter set,
  comment forms and quoted-string rule;
- `bsp::scene_scan_int` / `bsp::scene_scan_float`, the `sscanf` prefix semantics of `008D9AD0` and
  `008D9B40`;
- the document model (`SceneHeader`, `SceneEntity`, `ScenePropertyBlock`, `SceneGroupEntry`) and the
  four block parsers, each named for its native address;
- the key tables as data: `kSceneTopLevelKeys`, `kSceneHeaderKeys`, `kSceneEntityKeys`,
  `kSceneTerrainShadowKeys`, `kSceneRegistrationClassNames`;
- the record and database layout as documented offsets (`kSceneRecordPrecacheFlagOffset` = 0x905,
  `kSceneDatabaseFullPathOffset` = 0x13C, `kSceneDatabaseStemOffset` = 0x144, …). The two record
  offsets `include/bsp/mission_scene_load.hpp` already declares from the caller side
  (`kSceneRecordScenePathOffset` = 0x90C, `kSceneRecordMissionIdOffset` = 0x1098) are reused, not
  redefined;
- `bsp::SceneFileReaderHost`, one virtual per native call site outside the parser, and
  `bsp::run_scene_file_reader_0046df00`, the three-pass driver over it, in the style of
  `bsp::run_application_frame`. The VFS, resource-manager and scene-graph code the host binds to is
  reconstructed elsewhere and is not duplicated.

Deliberate divergence: the reconstruction stops after 256 recovered errors, where the native keeps
reporting. No shipped file comes near that (the worst is 4).

## State reached

| Address | Name | State |
| --- | --- | --- |
| 0046DF00 | `BSP_SceneDatabase_LoadSceneFile` | analyzed, reconstructed, build-tested, installed-file-checked |
| 0046CF40 | `BSP_SceneFile_ReadEntityBlock` | analyzed, reconstructed (parse only), installed-file-checked |
| 00469BF0 | `BSP_SceneFile_ReadHeaderBlock` | analyzed, reconstructed, installed-file-checked |
| 00467E10 | `BSP_SceneFile_ReadGroupsBlock` | analyzed, reconstructed, installed-file-checked |
| 00469E40 | `BSP_SceneFile_ReadBrowserGroupsBlock` | analyzed (entry loop only) |
| 0046A9F0 | `BSP_SceneDatabase_ClearPendingReferences` | analyzed (listing, past the Ghidra gaps) |
| 0046AAB0 | `BSP_SceneDatabase_ResolveDeferredReferences` | analyzed (shape and inputs) |
| 008D9CF0 | `BSP_SceneTokenizer_Construct` | analyzed |
| 008D8A70 | `BSP_SceneTokenizer_PeekToken` | analyzed, reconstructed, installed-file-checked |
| 008D8960 | `BSP_SceneTokenizer_ConsumeToken` | analyzed, reconstructed |
| 008D9930 | `BSP_SceneTokenizer_ExpectToken` | analyzed, reconstructed |
| 008D9980 | `BSP_SceneTokenizer_ReadToken` | analyzed, reconstructed |
| 008D9AD0 | `BSP_SceneTokenizer_ReadInt` | analyzed, reconstructed |
| 008D9B40 | `BSP_SceneTokenizer_ReadFloat` | analyzed (from its call sites and RET size) |

Every routine named here has a Ghidra function; the `no_ghidra_function` list in
`reports/scene_file_reader.json` is empty.

## Uncertainties

- The weather block (`0046DF00+0x100`..`+0x5A0`) was read for its literals and control flow, not for
  the Lua table API behind `00B65FB0`/`00B662B0`/`00B66270`/`00B67700`/`00B67800`. Which string the
  `Weathers` entry name is matched against is the weakest link: the compare at `0046E4xx` uses the
  scene path argument, but whether it is the full path or a derived name was not settled.
- `008F5A00`'s value grammar per type code (0..0Bh) was not read; the reconstruction reads a value
  as "tokens up to `;`", which is structurally right for every shipped file but does not model the
  parenthesised `V3` form or the array types `IA`/`FA`.
- `0046C550`, `00468660`, `00468FB0` and `00469690` are treated as contracts. The class ids `0x44`
  (landscape), `0x47`, `0x19`, `0x1B`, `0x1C`, `0x34` and `0x4D` are recorded but not named.
- The `SceneBrowserGroups` entry layout (`004694F0`) and the `traffic` body (`009514B0`) were not
  decoded.
- The hidden-entity record allocated at `0046D68A` is `0x5C` bytes; only its name and `Party`
  fields were identified.
- `0046A9F0` and `0046CF40` show fall-through gaps after `_free`/`operator delete` call-site
  overrides (`ghidra flow` reports 2 and 1). The `0046A9F0` gaps hide the loop back edge and are
  material; the `0046CF40` gap at `0046D8E0` is 3 bytes and was not needed.

## What remains, and follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `scene_property_bag` | 008f5a00 008f2260 008f38a0 008f54f0 008f41a0 008f3370 | docs/SCENE_PROPERTY_BAG.md, include/bsp/scene_properties.hpp | The `0x114`-byte property bag: the twelve type codes, their letters, the per-type value grammar (`V3`, `IA`, `FA`, `LUA_S`) and the descriptor lookup |
| `scene_entity_factory` | 0046c550 00468660 00468fb0 00469690 0046bf70 | docs/SCENE_ENTITY_FACTORY.md | Class name to descriptor to instance: the class-id table, the two-stage `0046C550` construction and the `Hidden`/`Party` record |
| `scene_browser_groups` | 004694f0 00469e40 00469a20 00469fc0 | docs/SCENE_BROWSER_GROUPS.md | One `SceneBrowserGroups` entry, and the group/index tables `00467E10` feeds |
| `scene_weather_descriptor` | 0046df00+0x100 008f5a00 00b69d40 00b6a020 00b65fb0 | docs/SCENE_WEATHER_DESCRIPTOR.md | The `Weathers` / `SubScenes` / `sceneFile` / `ID` / `Descriptor` walk of `SCRIPTS\datatables\Weather.lua` and which name it matches |
| `scene_traffic_block` | 009514b0 0095ca10 0095c640 00925f20 | docs/SCENE_TRAFFIC_BLOCK.md | The `traffic` body, and why pass 2 stops at it |
| `scene_deferred_refs` | 0046aab0 00925a90 00414db0 | docs/SCENE_DEFERRED_REFS.md | `this+150h`, the `00E19A70` name list, and how `R`/`RPath`/`RFort` values resolve to objects |

## Corrections from docs/SCENE_ENTITY_FACTORY.md

`0046c550` constructs nothing: it is the per-entity generation predicate (`__thiscall` on the scene database, `RET 5Ch`, 23 stack dwords), and `00468660` maps a class id to its class name, not to a factory. The real class table is `004f2800` (formerly named `BSP_Scene_ResolveNamedObjects`), which registers 26 classes through `004ee250` as 12-byte descriptors (class id, instantiate-pass creator, registration-pass creator) into the scene database hash map at +34h. The seven unidentified ids resolve to Landscape (44h), Path (47h), LandFort (1Bh), CommandBuilding (1Ch), WaterMine (34h) and SpawnPoint (4Dh); 19h is not a registered id, so the comparison at `0046d492` can never match. Wreck, CameraPath, PeriodicEffect and FreeCamPos are registered but never authored in the 259 installed files.
