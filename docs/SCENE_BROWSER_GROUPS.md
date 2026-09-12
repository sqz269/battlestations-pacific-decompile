# Scene `SceneBrowserGroups` block (`00469E40`, `004694F0`) and the multiplayer stock queue

Addresses: `00469E40`, `004694F0`, `0046BF70` (contract, already documented), `0095C4D0`,
`0046EBD3` and `0046EBDD` call sites in `0046DF00`.

Packet `cc2_scene_traffic_groups`, read-only analysis. Names are hypotheses, not recovered
symbols. Closes `docs/SCENE_FILE_READER.md`'s "the entry layout was not read" row. Contracts:
`docs/SCENE_FILE_READER.md` for the `groups` block (`00467E10`, already reconstructed) and the
pass table, `docs/SCENE_ENTITY_FACTORY.md` for `0046BF70`'s stock walk.

## The finding

**`SceneBrowserGroups` is parsed for syntax and discarded.** Neither `00469E40` nor `004694F0`
stores a byte of it, and neither reads its `this` pointer. There is no group table, no index
table and no consumer: the block is mission-editor metadata that the shipped reader validates and
drops on the floor.

This is not the `groups` block. `groups` (`00467E10`) is a different keyword with a different
entry shape (`group { GroupName = TOKEN Index = INT }`) and is already reconstructed in
`docs/SCENE_FILE_READER.md` and `include/bsp/scene_file.hpp` as `SceneGroupEntry`. The two were
adjacent rows in that doc's tail-block table and are easy to conflate.

## Grammar

```
scenebrowsergroups := "SceneBrowserGroups" "{" browsergroup* "}"
browsergroup       := "BrowserGroup" "{" "GroupName" "=" TOKEN
                                         "GroupID"   "=" INT
                                         "Parent"    "=" TOKEN "}"
```

The key order is fixed, not a dispatch loop: `004694F0` is a straight line of eleven
`BSP_SceneTokenizer_ExpectToken` calls with two `ReadToken` and one `ReadInt` between them.

### Installed-file check

`local/census_traffic_groups.py` over all 259 `.scn` files:

| Measure | Value |
| --- | --- |
| files with a `SceneBrowserGroups` block | 249 |
| `BrowserGroup` entries | 927 |
| distinct key orders | 1, `(GroupName, GroupID, Parent)` |
| entries with a missing or extra key | 0 |
| grammar errors | 0 |

All 927 entries match the fixed order the listing requires, so the reader's strictness is never
exercised by shipped content. Observed values look like an editor tree: `GroupName` strings such
as `REPOPPNAVPOINTS`, `Decomissioned` and `AvoidZones`, `Parent` either `$SceneRoot` or a
landscape name such as `Landscape 02`.

## `00469E40` — the block reader

`__thiscall(void* this, SceneTokenizer* tok)`, `RET 4` at `00469EA4`. Called once, at `0046EBD3`
in `0046DF00` with `ECX = EDI`. Verified with `ghidra proto 0046ebd3`: body `0046DF00-0046EF62`.

| # | site | callee | name | this / args / ret | gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `00469E4F` | `008D9930` | `BSP_SceneTokenizer_ExpectToken` | ECX = tok, push `00CE56A4` `"SceneBrowserGroups"` | none |
| 2 | `00469E5B` | `008D9930` | same | push `00CE4D38` `"{"` | none |
| 3 | `00469E62` | `008D8A70` | `BSP_SceneTokenizer_PeekToken` | ECX = tok | loop head |
| 4 | `00469E6E` | `00438E10` | `BSP_CString_CompareInsensitive` | ECX = token, EDX = `00CE5654` `"BrowserGroup"` | none |
| 5 | `00469E7A` | `004694F0` | `browser_group_entry_read` | ECX = EDI (the entry `this`, never used by the callee), push tok | compare == 0 |
| 6 | `00469E81`/`00469E8D` | `008D8A70` + `00438E10` | the same peek and compare, re-tested | | after each entry |
| 7 | `00469E9D` | `008D9930` | `ExpectToken` | push `00CE4CD4` `"}"` | loop exit |

`MOV EDI,ECX` at `00469E46` is the only use of the incoming `this`, and its only consumer is the
`MOV ECX,EDI` at `00469E78` that passes it straight to `004694F0`.

## `004694F0` — the entry reader

`__thiscall(void*, SceneTokenizer* tok)`, `RET 4` at `0046960D`. `this` is never read: the first
instruction to touch `ECX` is `MOV ECX,ESI` at `00469512`, after `MOV ESI,[ESP+24h]` has loaded
the tokenizer from the stack argument.

| # | site | callee | literal | result |
| --- | --- | --- | --- | --- |
| 1 | `00469514` | `ExpectToken` | `00CE5654` `"BrowserGroup"` | - |
| 2 | `00469520` | `ExpectToken` | `00CE4D38` `"{"` | - |
| 3 | `0046952C` | `ExpectToken` | `00CE55C0` `"GroupName"` | - |
| 4 | `00469538` | `ExpectToken` | `00CE55BC` `"="` | - |
| 5 | `0046953F` | `BSP_SceneTokenizer_ReadToken` | - | char* in EAX |
| 6 | `00469549` | `BSP_NativeString_Assign` | - | into the stack string at `[ESP+10h]` |
| 7 | `0046955D` | `ExpectToken` | `00CE564C` `"GroupID"` | - |
| 8 | `00469569` | `ExpectToken` | `00CE55BC` `"="` | - |
| 9 | `00469575` | `BSP_SceneTokenizer_ReadInt` | - | written to `[ESP+24h]` |
| 10 | `00469581` | `ExpectToken` | `00CE5644` `"Parent"` | - |
| 11 | `0046958D` | `ExpectToken` | `00CE55BC` `"="` | - |
| 12 | `00469594` | `ReadToken` | - | char* in EAX |
| 13 | `0046959E` | `BSP_NativeString_Assign` | - | into the stack string at `[ESP+8h]` |
| 14 | `004695AF` | `ExpectToken` | `00CE4CD4` `"}"` | - |
| 15 | `004695CC`/`004695D3` and `004695F3`/`004695FA` | `BSP_SizedStoragePool_*` | - | both strings released |

Step 9's destination settles the `GroupID` question. At `0046956E` the frame is
`entry_ESP - 20h`, so `LEA EAX,[ESP+24h]` addresses `entry_ESP + 4` — the caller's stack slot for
the tokenizer argument, already copied into `ESI` and now dead, and popped by the `RET 4`. The
parsed integer is written into a slot that nobody reads. Ghidra names it `&stack0x00000004` for
the same reason.

Steps 6 and 13 are the only other stores, both into stack strings that steps 15 free before the
function returns. Nothing is written through `this`, through a global, or through a pointer
derived from the tokenizer. The routine is a validator.

## The multiplayer stock queue (`0046BF70` -> `0095C4D0`)

`0046BF70` `BSP_SceneEntity_RegisterMultiplayerStock` is already documented in
`docs/SCENE_ENTITY_FACTORY.md`: called from `0046CF40` at `0046D531` and `0046D56B` with the
entity property bag and the class name, it walks `PlaneStock %d` for `AirField` and
`MotherShipGen` and `NumSlots`/`Slot %d`/`Stock %d`/`AlliedList`/`JapanList` for `SpawnPoint`
and `Shipyard`, dispatching each stocked class to `0095C4D0` or `0046BE90`. This packet adds only
what `0095C4D0` does with what it is given, because that is the edge `docs/SCENE_TRAFFIC_BLOCK.md`
needs.

`0095C4D0(int class_id)`, `__fastcall`, no stack arguments, sole caller `0046BF70`:

1. `value = *(BSP_VehicleClassRegistry_GetSingleton() + 10h + class_id*4)` — the forward index
   map of `docs/VEHICLE_CLASS_DESCRIPTORS.md`.
2. Walk the intrusive list whose sentinel pointer is `00F8A0B0`, comparing `node[2]` against
   `value`; return on a hit.
3. On a miss, `004857F0(sentinel, sentinel[1], &value)` links a new node and
   `sentinel[1] = node`, `*(node[1]) = node`.

So the queue is a deduplicated set of already-mapped class ids, drained by `0095CA10` at the
pass-2 `traffic` keyword. Which stock a class came from is not recorded; only the id survives.

`0046BF70` itself is `contract: read elsewhere` — this packet read only its `0095C4D0` call and
reused `docs/SCENE_ENTITY_FACTORY.md` for the rest, so no new claim is made about the stock walk.

## Routine coverage

| Address | Name | Coverage |
| --- | --- | --- |
| `00469E40` | `BSP_SceneFile_ReadBrowserGroupsBlock` | complete (33 instructions, read in full) |
| `004694F0` | `BSP_SceneFile_ReadBrowserGroupEntry` | complete (62 instructions, read in full) |
| `0095C4D0` | `BSP_VehicleClassStockQueue_Append` | complete |
| `0046BF70` | `BSP_SceneEntity_RegisterMultiplayerStock` | `contract: documented in docs/SCENE_ENTITY_FACTORY.md`; only the `0095C4D0` edge re-read here |
| `004857F0`, `0048FC80` | - | `contract: unread` (the list link) |

## Correction to `docs/SCENE_FILE_READER.md`

That doc's tail-block table says of `00469E40`: "repeats `004694F0`, one entry per iteration; the
entry layout was not read". The entry layout is `GroupName`/`GroupID`/`Parent`, and there is no
record: the row should say the block is validated and discarded. The same table's `traffic` row
calls `0095CA10` the pass-2 skip; `docs/SCENE_TRAFFIC_BLOCK.md` corrects that.

## Open questions

1. Nothing in the shipped executable reads `SceneBrowserGroups`, so whether the editor build
   stored it cannot be settled from this image.
2. `00469E40`'s unused `this` is `EDI` at `0046EBD3`; since neither callee reads it, its identity
   was not traced.
