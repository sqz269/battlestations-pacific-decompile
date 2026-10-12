# Review of the `stl_probable` bookmark rows (cc13-stlreview1)

Packet cc13-stlreview1, 2026-10-12. This is a quality pass over the cc13 template tags. It reviews
every bookmark-only `stl_probable` row image-wide. It also covers two small items: the RNG stream of
the sound effect instance `00873AE0`, and a stale name in `docs/CC13_BAND_0074_0078.md`.

Status vocabulary: everything here is tag and name metadata from static evidence (exports, live
Ghidra decompiles, the disk image). Nothing is reconstructed, build-tested, or game-validated.

## 1. Counts

There were 1850 bookmark rows: 1494 from inventory rule `e0029` and 356 from the cc13-stl1 rule
`st1_scl97`. Both rules mean "calls `_invalid_parameter_noinfo` `00BF6713` with a body over 96
bytes"; `e0029` also caps the body at 256 bytes.

| decision | e0029 | st1_scl97 | total | tag result |
| --- | --- | --- | --- | --- |
| upgraded to a shape name | 588 | 10 | 598 | `stl_shape` rename row `<Shape>_<addr>`, Ghidra `FUN_` renamed |
| upgraded, existing name kept | 154 | 2 | 156 | `stl_instantiation` rename row carrying the ledger name |
| converted to reviewed (game) | 140 | 25 | 165 | `game_code` row, action `reviewed`, ledger name |
| dropped (unnamed game code) | 117 | 218 | 335 | row removed |
| kept: library-shaped, no shape name | 155 | 37 | 192 | unchanged bookmark |
| kept: uncertain | 34 | 18 | 52 | unchanged bookmark |
| deferred: inside another sweep's lease | 306 | 46 | 352 | unchanged bookmark |

So 1498 rows were reviewed and decided: 754 upgraded, 165 converted, 335 dropped, 244 kept. All
1850 decisions, with the anchors behind each drop, are in `tools/template_shapes/stlr_decisions.txt`.
`tools/template_shapes/stlr_review.py` applies that file to `config/tags`.

Deferred ranges were leased to active sweeps at the time: cc13-sweep67 (`00670000-006CFFFF`,
`00700000-0073FFFF`) and cc13-sweep9x (`00980000-0098FFFF`, `00A50000-00BFFFFF`). Their decision
lines are marked `deferred_leased` but still carry the computed verdict in the anchors column. Once
the leases are gone, run the classifier again, or apply the file with the deferred lines changed.

Ghidra changes went in under the write lock (`local/stlr-ghidra-tags-20261012T140910Z.json` and
`local/stlr-ghidra-unbookmark-20261012T140910Z.json` in the worker tree):

- 598 renames from `FUN_` to the shape name.
- 157 `already` rows: the named instances got an `Inventory: stl_instantiation` bookmark, and no
  existing name was changed.
- 1254 `Inventory: stl_probable` bookmarks deleted on every upgraded, reviewed and dropped row. Each
  old comment was `[medium] ` plus the rule text, so it can be rebuilt from `config/tags/evidence.json`.
- `_free` `00BF9DC8` kept `void __cdecl _free(void *)` before and after the apply.
- 'Non-Returning Functions - Discovered' was not touched.

## 2. Method

1. Features per row. Ledger name, and callers and callees from `local/bsp_index.sqlite`. A
   Capstone pass over the disk bytes from the entry to the next function start collects
   absolute references, classed as `.data`/`.bss` global, `.rdata` string, `.rdata`
   pointer-to-code (vtable or IAT), or `.rdata` constant.
   - The index `datarefs` table is incomplete. `008EA250` has none there, yet it loads `00D7A24C`
     and `00CE4970` with `MOVSS`, so the bytes are the source.
   - Byte ranges can run past the real end of a function: `004E6050` picked up a global from the
     following bytes. Every drop decision was therefore re-checked against the pseudocode.
2. Live decompiles of all 1850 rows, saved in the worker tree as `local/stlr_decomp/`.
   - Game anchors in the pseudocode count when they appear in the code itself: a string literal; a
     `DAT_` global in `00E0xxxx-0109xxxx`; a `this`/`param_1` field offset of `100h` or more; a call
     to a named game function that is not an element operation (string, pool, copy, destroy,
     compare) or a registry singleton accessor; a virtual call that is not a deleting destructor.
3. Pseudocode identity. The pseudocode is normalised (names, numbers and locals replaced) and
   hashed. Rows with identical text form 350 clusters. A cluster was labelled only when (a) a
   representative was read and the Dinkumware operation identified, and (b) the members' existing
   names from earlier sweeps agree.
   - Example: cluster `7bee2dc83d` holds 98 unnamed rows plus 13 named ones, among them
     `STL_ClassKeyTree_IncrementIterator_004a73a0` and `SoldierRegistry_IteratorIncrement_004AFB50`.
     Its representative `00552E90` reads `_Right` (`+8`) first, then walks `_Left` with an `_Isnil`
     byte test. That is `_Tree::const_iterator::_Inc`.
4. Decisions.
   - A row in a labelled cluster is upgraded.
   - A named row is upgraded when its name already says "container operation" and it has no
     string anchor. It is converted to reviewed otherwise.
   - An unnamed row is dropped when it has a string, a global, a large field offset, two game
     calls, or one game call plus a constant or virtual call. It is kept as uncertain when it has
     one weak anchor, and kept as library-shaped when it has none.

### 2.1 Shape labels (verified representative, then members)

| shape name prefix | evidence key | rows | representative read | consensus names in the cluster |
| --- | --- | --- | --- | --- |
| `STL_Vector_InsertOne` | `st1_vecinsertone` (new) | 115 | `005A6650`, `0043E3B0`, `00460A20`, `004206A0`, `00540AF0` | `BSP_BotStateVector_InsertOne`, `STL_CheckedWordVector_InsertOne`, `STL_HudScoreRowVector_Insert` |
| `STL_Vector_PushBack` | `st1_vecpushback` (new) | 96 | `004216B0`, `004222C0`, `00450600` (one per cluster) | `BSP_PointerVector_PushBack`, `MSVC_Vector12_PushBack`, `STL_HudScoreRowVector_PushBack` |
| `STL_TreeIter_Dec` | `st1_iterdec` | 92 | `0061D1C0` | `STL_NativeStringIntTree_DecrementIterator`, `BSP_PanelPalette_IteratorPrevious` |
| `STL_TreeIter_Inc` | `st1_iterinc` | 91 | `00552E90`, `0084EE80` | `STL_ClassKeyTree_IncrementIterator_004a73a0` |
| `STL_Tree_EraseRange` | `st1_treeeraserange` | 76 | `00441E50`: begin/end fast path through `_Erase(root)` and the head reset, else the erase(where) loop | none (all 86 unnamed) |
| `STL_Vector_Resize` | `st1_vecresize` (new) | 26 | `00421AE0`, `00822460`, `00494220` | `STL_CheckedFloatVector_Resize_004a8f10`, `BSP_RacePointerTable_Resize` |
| `STL_Map_Subscript` | `st1_mapsubscript` | 24 | `00499030`, `004A95F0`, `00575E80` | `BSP_AnimClassRegistry_Index`, `STL_Map_Subscript_006170F0` |
| `STL_Vector_EraseRange` | `st1_veceraserange` (new) | 17 | `00446EF0`, `004D17C0`, `0044F020` | none |
| `STL_Vector_CopyConstruct` | `st1_veccopyctor` (new) | 16 | `0044EF30`, `0044DC40`, `004F6330` | `STL_CheckedDwordVector_CopyConstruct_004F6210` |
| `STL_List_InsertRange` | `st1_listinsertrange` (new) | 14 | `00494AA0` | none |
| `STL_Tree_Find` | `st1_treefind` | 13 | `004C8B80` | `STL_MapNativeStringPtr_Find`, `BSP_NativeStringSet_Find` |
| `STL_Tree_InsertHint` | `st1_treeinserthint` | 6 | `004B3240` | none |
| `STL_List_Remove` | `st1_listremove` (new) | 6 | `004C4670` (head only) | `STL_ListInt_RemoveValue`, `BSP_WarningList_RemoveByValue` |
| `STL_List_Erase` | `st1_listerase` (new) | 6 | `00531F10` | `STL_ChatOverlayList_Erase`, `STL_ChatHistoryList_Erase` |

Two clusters were labelled game code and their rows dropped:

- `7670104ecd` and `346f371f88` (15 rows, `00968710` and neighbours): virtual predicate calls through
  `this+0Ch`, `+1Ch`, ... over a vector of arguments.
- `d9d5ce1440` (5 rows, `00472420` and neighbours): walks the class-registry singletons
  (`BSP_AnimClassRegistry_GetSingleton` and the others).

One cluster (`76ac0e735e`) first carried the label `STL_Tree_Assign`, because two members had that
name from cc13-sweep60 (`0062BB60`, `0062C170`, "std::map::operator="). Its representative
`00575E80` and both named members do something else:

- an `_Lbound` walk over `[this+4]` comparing `node+0Ch`;
- on a miss, a default inner tree (`STL_Tree_BuyHeadNode`, `STL_Tree_CopyConstruct`) inserted with
  `STL_Tree_InsertHint`;
- `return node+10h`.

So all three are `std::map<int, inner map>::operator[]`, which is how the host headers already model
`0062C170`/`0062BB60`/`00625900` (`include/bsp/scoring_bodies.hpp:217`). The cluster is now
`STL_Map_Subscript`. The two ledger names were replaced with `STL_Map_Subscript_0062BB60` and
`STL_Map_Subscript_0062C170` (`ledger add-name --replace`, applied with `ghidra_annotate.py --apply`;
the old plate text is kept above the update). The three shape rows (`00575E80`, `00575FD0`,
`0097EFA0`) were renamed in Ghidra to match.

Fourteen rows that sweeps had named as game functions sit in labelled template clusters; four of them are in deferred ranges. Examples:
`BSP_MissionTree_PushBackMission` (vector push_back) and `BSP_VFS_AdvanceMountIterator` (tree
iterator inc). They became `stl_instantiation` rows and kept their names.

## 3. Rule-level findings

| rule | decided rows (not deferred) | game code | template / library-shaped | uncertain |
| --- | --- | --- | --- | --- |
| `st1_scl97` (no size cap) | 310 | 243 (78%) | 49 | 18 |
| `e0029` (97-256 bytes) | 1188 | 257 (22%) | 897 | 34 |

- **`st1_scl97` is unreliable.** About four in five of its rows are game code. Without a size cap
  it catches any large game function that indexes a `std::vector` or walks a `std::map` with
  checked iterators: Lua bindings (`008CF7E0` `luaMW_GetGuns`), message senders (`008E2EE0`, the
  map ping), and screen updates. The 0080-008F set the lead prioritised: of the 65 rows, 47 were
  dropped, 6 converted, 4 upgraded, 7 kept library-shaped, 1 kept uncertain (`008DDBE0`).
- **`e0029` is usable as a hint, not as a tag.** Its stated precision of about 0.6-0.8 holds. The
  false positives are mostly game-object methods that wrap a member vector: the container sits at
  `this+774h`, `+784h` or `+794h` in the shipyard object `00843xxx-0084Axxx`. The rest are short
  loops that call one game function per element.
- **Earlier sweeps' names are not always right.** The two `STL_Tree_Assign` names above were wrong
  (section 2.1). A cluster label taken from member names alone would have spread the error to three
  more rows. Read a representative before trusting a name consensus.
- **Two wrong-name hazards seen in passing.**
  - `STL_xlen_throw_*` names (rule `e0030`) are carried by helpers whose main job is `erase(where)`,
    `_Insert` or `_Incsize`, as seen from callers: `004414B0`, `004B2E30`, `00490130`. The name
    records only the throw path.
  - Section 4.2 of `docs/CC13_RNG_STREAM_CENSUS.md` lists `00873AE0` with particles; see section 5
    below.

### 3.1 Suggested tightening of the st1 rules

Proposals for whoever next owns `tools/template_shapes/st1_build.py`; this pass did not edit it.

1. Drop the uncapped `scl_97plus` bookmark rule, or cap it at 256 bytes like `e0029`. Measured to the
   next function start, 41 of the 293 decided `st1_scl97` rows over 256 bytes (14%) were library
   shaped. Of the 1129 decided `e0029` rows at or under 256 bytes, 856 (76%) were.
2. Before writing any `stl_probable` row, reject a body that has any of these:
   - an absolute reference into `.data`/`.bss` (`>= 00E08000`), or an `.rdata` string;
   - a `[ECX+disp]` / `[this+disp]` access with `disp >= 100h`;
   - a call to a named non-library function other than element copy, destroy and compare helpers;
   - a virtual call through a member pointer that is not a deleting destructor.
   Read these from the bytes or the pseudocode. The index `datarefs` table misses many references.
3. Prefer an upgrade to a bookmark. Hash the normalised pseudocode (or the existing coarse
   signature) and match it against named or tagged instances. Most real library rows here fell into
   15 shapes; the generic `STL_inst` twin name loses that information.
4. A row that matches only by "calls `00BF6713`" should stay out of the tag set altogether. It is
   good as a sweep worklist, not as a tag.

## 4. Kept rows

- **192 library-shaped** rows have no anchor but sit in clusters too small or too varied to name
  safely; 288 of the 350 clusters are singletons. These are mostly checked iterator or vector
  helpers.
- **52 uncertain** rows have one weak anchor. Examples: one `.rdata` constant (`0041E000`,
  `00443A30`), or one call to a game function that might be an element operation
  (`00481430`/`00484A10`/`00487940`/`004A5B90` call `BSP_MissionEntity_Kill`; `00A12CD0` and
  neighbours call `BSP_EntityPose_RefreshWorld`). Most of these look like game loops over a vector.
  Leaving them as bookmarks keeps them visible to the band sweeps.

  Full list: 0041E000 0041E960 00429B00 0042A480 00443A30 004514A0 0045A320 0045DDA0 00480A90
  00481430 00484A10 00487940 004881F0 00491E70 004A5B90 004AB9A0 004BFB50 0051E050 005251C0
  00538A80 00540D50 00542620 0059AA40 005DA800 005DAE00 005DEE40 005F4FA0 005F6EF0 006FBAD0
  00797AF0 00797ED0 00797F50 0079A190 007AED10 007AF740 007B1A80 007B1FB0 00884B60 0088B120
  008DD1C0 008DDBE0 009080A0 0093B920 00947BC0 00975D00 0097DC80 00A02180 00A12580 00A12CD0
  00A12DB0 00A12EA0 00A12FD0.
- **Dropped game rows are unnamed.** They need names from the band sweeps. The 0080-008F ones (76,
  both rules) are in the decisions file. Those with the strongest anchors:
  - Lua bindings carrying `luakod` and `luaMW_<name> failed:` strings: `00892F70` HitTarget,
    `008936A0` GetDraught, `008AA8F0` SetForcedReconLevel, `008AAB90` ClearForcedReconLevel,
    `008CECC0` GetPayloads, `008CF7E0` GetGuns.
  - `008023B0`: `RainSplashes`, called from `BSP_WorldAtmosphere_ApplyPreset`.
  - `008509F0`: `DamageLevel`, `XStaticShadowFactor`.
  - `0086D180`: `Alpha`, `TextureFile`.
  - `008EDDC0` and `008EE020`: `pup_gain`, `PUM1STGET`, awards.
  - `00849F70`: `Slot %d`, `Path`, shipyard.

## 5. RNG audit: `00873AE0` (item 2)

Recorded in `docs/CC13_RNG_STREAM_CENSUS.md` section 12. `00873AE0` is
`BSP_EffectSoundInstance_Construct` (cc13-sweep8E). It draws from the **shared thread generator,
stream 0**. There is no separate generator, and ECX is not passed through: `00873B61` and `00873C26`
are each `XOR ECX,ECX` immediately before the call.

- `00873B63`: `00BD2FC0` returns a raw word, which goes through `DIV` by the sample count
  `[def+24h]` to pick the sample.
- `00873C39`: `00BD2F10` draws U(0, 2s), with s = `[def+38h]`. The result becomes
  `[inst+20h] = 1 + U(0,2s) - s`.
- Both draws are skipped when the definition has no samples (`00873B5B JE`).

`00BD2ED0` maps stream 0 to the calling thread's slot. On the main thread that is the shared
gameplay stream 0. Which thread creates effect instances was not traced. No host code references
this function.

## 6. Doc fix (item 3)

`docs/CC13_BAND_0074_0078.md` open question 4 now has a correction note. The A1 name
`BSP_MNetworkXLive_ShutdownToState2` was replaced by `BSP_MNetworkServer_StartHostingAndSetState2`
(`00A41300`, ledger confirmed), per `docs/CC13_BAND_0080_008F.md`. The original question text is
left as written.

## 7. Uncertainty

- Cluster labels rest on identical normalised pseudocode. The normaliser replaces every callee
  name with one token, so two bodies with the same control flow but different callees fall into
  one cluster. Each labelled cluster's representative was read, and the unnamed members carry no
  game anchor. Even so, a member could be a hand-written game function with the same shape.
  Confidence on the `stl_shape` rows is therefore `medium`, not `high`.
- Name-based upgrades (`stl_instantiation`) trust the earlier sweeps' names.
- Drops rest on anchors in the pseudocode. A template instance that inlines a game element's
  constructor can call a game function. Element helpers (string, pool, copy, destroy, compare,
  assign) were excluded for that reason, and single-call rows were kept rather than dropped.

## 8. Handoff

Branch `agent/cc13-stlreview1`; the commit list is in the final report.

Next steps:
1. When cc13-sweep67 and cc13-sweep9x release their ranges, decide the 352 deferred rows: re-run
   the pipeline in `tools/template_shapes/stlr/` (see the README there, and update the hard-coded
   lease exclusions) or edit the decisions file. Then run
   `python tools/template_shapes/stlr_review.py <band prefixes>` and
   `tools/template_shapes/stlr/stlr_ghidra_apply.py`.
2. Run `bsp.py snapshot` so the renames reach the exports.
3. Have the band sweeps name the dropped game rows.
4. Change `st1_build.py` per section 3.1.

## 9. Second pass (cc13-stlreview2)

Packet cc13-stlreview2, 2026-10-12, on origin/main `afa6c36e3` (cc13-sweep9x landed). Same status
vocabulary as above: tag and name metadata from static evidence, nothing reconstructed or run.

### 9.1 Scope and counts

All 677 `stl_probable` bookmark rows still present were re-featured, re-decompiled (live Ghidra,
read-only) and re-clustered with the section 2 pipeline. The 137 rows inside cc13-sweep67's lease
(`00670000-006CFFFF`, `00700000-0073FFFF`) were deferred at first and decided later (9.6). The other
540 were decided:

| origin of the row | upgraded (shape) | upgraded (name kept) | reviewed (game) | dropped | kept library | kept uncertain |
| --- | --- | --- | --- | --- | --- | --- |
| deferred by stlreview1 (0098, 00A5-00BF) | 47 | 54 | 74 | 26 | 12 | 2 |
| new since stlreview1 (cc13-sweep9x bookmarks) | 9 | 0 | 1 | 38 | 12 | 2 |
| stlreview1 `keep_library` | 46 | 0 | 0 | 14 | 132 | 0 |
| stlreview1 `keep_uncertain` | 1 | 0 | 0 | 37 | 0 | 14 |
| stlreview1 `drop_game`, resurrected (9.4) | 0 | 0 | 0 | 19 | 0 | 0 |
| total (540) | 103 | 54 | 75 | 134 | 156 | 18 |

The 62 "new" rows are 0090-00BF bookmarks that cc13-sweep9x added after stlreview1 ran.

The decision file is `tools/template_shapes/stlr2_decisions.txt` (written by
`tools/template_shapes/stlr/stlr2_final.py`). It was applied with
`python tools/template_shapes/stlr_review.py --decisions tools/template_shapes/stlr2_decisions.txt
--source docs/CC13_STL_PROBABLE_REVIEW.md`; `stlr_review.py` now takes both options.

Ghidra, under the write lock (`stlr_ghidra_apply.py`; logs `local/stlr-ghidra-tags-20261012T143127Z.json`
and `local/stlr-ghidra-unbookmark-20261012T143127Z.json` in the worker tree):

- 103 `FUN_` renames to `<Shape>_<addr>`; 54 named rows got the `Inventory: stl_instantiation`
  bookmark and kept their names.
- 366 `Inventory: stl_probable` bookmarks deleted (every upgraded, reviewed and dropped row).
- `_free` `00BF9DC8` read `void __cdecl _free(void * _Memory)` before and after;
  'Non-Returning Functions - Discovered' was not touched.

### 9.2 New cluster labels

Each representative below was read in full; members share its normalised pseudocode (section 2,
step 3). Labels were appended to `tools/template_shapes/stlr/stlr_labels_extra.txt`.

| cluster | label | representative read | what the body does |
| --- | --- | --- | --- |
| `81b50eb021`, `6a3d68c0b1` | `STL_Tree_InsertHint` | `0078BEA0` (int key), `006658D0` (ushort key) | empty-tree and begin/end fast paths, then `_Dec`/`_Inc` of the hint, key compare at `node+0Ch`, `_Insert(addLeft, where, val)`; the callee carries the stale name `STL_xlen_throw_*` (section 3 hazard) |
| `8c4a9bbc7d`, `aba6b8ad91`, `98214f377a`, `f420ed1592`, `7c1d281c35` | `STL_Vector_Resize` | `0043DAB0` (8-byte), `00495D40` (54h), `004A48D0`, `0082C560`, `0083B480` | `size()<n` -> `_Insert_n(end, n-size, val)`, else `erase(begin+n, end)` |
| `c1b8adbc94`, `2060e75afd`, `be88bcae9a`, `51bf3bf305` | `STL_Vector_EraseRange` | `005C92F0` (434h records), `004C8630`, `0050A110`, `00524DC0` (virtual element dtor) | same-container hook, copy tail down, destroy `[newEnd, end)`, store end |
| `cab4e3fbb7`, `0b84a17f3c` | `STL_Vector_CopyConstruct` | `005C93C0`, `009487D0` | zero the triple, `_Buy(n)` with max_size check, `_Ucopy` under an EH state |
| `4575e47139` | `STL_List_Erase` | `004CECF0` | `list<std::string>::erase(where)`: unlink, string `_Tidy` (cap > 0Fh frees), free node, `--size` |
| `7be01eafcd` | `STL_List_EraseRange` (new key `st1_listeraserange`) | `00489690` | begin/end pair -> `clear()`, else loop unlinking and freeing each node |
| `5d81a9a118` | `STL_Vector_Reserve` (new key `st1_vecreserve`) | `00747CC0` | max_size (`38E38E3h` for 48h elements) length_error, allocate, move, free old |
| `388fe031ae`, `5cf1f258d6`, `e669c5cad1` | `STL_Map_Subscript` | `0078C0E0`, `005A7AF0`, `00A27A00` | `_Lbound` walk, on miss `insert(hint, value_type(key, T()))`, return `node+10h` |
| `d71d29abd2` | `STL_Tree_EraseRange` | `007743E0` | begin/end pair -> `_Erase(root)` + head reset, else erase(where) loop |

Five clusters were read and labelled game code (unnamed members dropped): `d904e1bdd4`
(`0096E3A0`: virtual writer interface `+4`/`+0Ch`/`+8` over a set), `818152624d` (`00473CE0`:
`BSP_BulletVisualClassRegistry_GetSingleton` walk), `af54afaa58` (`00640860`: owner fields
`+60h`/`+64h`/`+80h` and a 18h-byte pool block), `913d7c756a` (`00A12CD0`: `+5640h`/`+5644h` and
`BSP_EntityPose_RefreshWorld`), `9ae8fad93e` (`0045D110`: removes a pointer from the vector at
owner `+68h`; its twin is `BSP_GuiManager_RemovePagePointer`).

Two named rows were moved from reviewed to upgraded after reading them: `00B14480`
`BSP_RendererRecords_Erase` is `vector::erase(first,last)` over 300-byte records
(`st1_veceraserange`), and `00B7CDF0` `BSP_DecrementNativeResourceCacheIterator` is `_Tree` iterator
`_Dec` (`st1_iterdec`). Their names did not match the container-name regex in `stlr_bucket.py`.

### 9.3 Reading the kept rows

stlreview1 kept 52 rows as uncertain and 192 as library-shaped by rule, without reading them.
`local/s2_sig.py` (worker tree) printed each in-scope row's signature, callees, largest `this` field
offset (including the `param_1[N]` indexed form, which the section 2 field regex missed), float-op
count and virtual-call count, and the outliers were read. Dropped as game code (reasons in the
anchors column of the decision file):

- 49 uncertain rows, among them `BSP_MissionEntity_Kill` loops (`00481430`, `00484A10`, `00487940`,
  `004A5B90`), float geometry (`007AED10`, `007B1FB0`, `00AEB0B0`, `00AEB1F0`, `00AEB310`, `00ADF1B0`),
  bit-stream serialisers (`00429B00`, `0042A480`), front-end and HUD teardown (`0051E050`,
  `00538A80`, `00540D50`), and Lua/GUI/input callers (`00BD8EC0`, `00AB1F60`, `005251C0`).
- 12 library-shaped rows with game anchors the rules missed: `0091A080` (4.7 KB, field `+1E8h`,
  float math), `0095F1C0`, `00741A40` and `007B2950` (`BSP_NetEntityCreateMessage` with fields
  past `100h`), the GUI widget methods `00A9BF90`, `00A9C860`, `00ACB9B0`, `00AD0680`, and
  `0043DD80`, `0045A210`, `0045B220`, `007AE6D0`.

Still kept, uncertain (18): `0041E960 0045DDA0 00480A90 004881F0 004BFB50 005DAE00 005DEE40
005F4FA0 00797F50 0079A190 007B1A80 008DD1C0 009080A0 00975D00 00B77F80 00BD23E0` (deleting
destructors and short loops with one ambiguous callee), plus `0098FC20` and `00990340`, which are
in 9.5.

### 9.4 Resurrected rows: a ledger-merge hazard

19 rows that stlreview1 dropped (`009042B0`, the 15-row `00968710` vcall cluster, `0096BF70`,
`0096CE00`, `0096ED00`) were back in `config/tags/00900000.jsonl` and `00960000.jsonl`. The merge
`044466966` (origin/main into agent/cc13-sweep9x) restored them: a union merge of JSONL shards keeps
every line from both sides, so a deleted row comes back when the other branch still has it. Their
Ghidra bookmarks had already been deleted. This pass drops them again. A duplicate seen in passing:
`00A44840` has two `stl_instantiation` rows in `config/tags/00a40000.jsonl` (stlreview1 and
cc13-sweepA1, same name); it was left alone.

Anyone deleting tag rows on a branch should expect a union merge to undo the deletion. Check
`git diff` of the shard after the merge, or record deletions somewhere the merge tool can see them.

### 9.5 Library code that is not a container template

The 0098-0099 rows that stay `keep_library` are Dinkumware iostream/locale code, not container
templates: `_Lockit`, `_Locinfo_dtor`, `_Init`, `basic_streambuf` init (`0098DE50`, `0098E4E0`,
`0098F180`, `0098FB50`, `00991150`, `00991610`), `char_traits`/string `_Chassign` growth
(`0098F9A0`, `0098FA90`), and `getc`/`fputc`/`fwrite`-driven `basic_filebuf` bodies (`0098FC20`,
`00990340`, `0098FFE0`, `00992480`). They need names from the MSVC 8 `<fstream>`/`<xlocale>`
sources, not a container shape.

### 9.6 The sweep67 rows (third run)

cc13-sweep67 landed while this pass was running (merged at `76aa016f4`) and released its lease. Its
range then held 173 rows: the 137 deferred ones and 36 new bookmarks from that sweep. The same
pipeline ran on them (`stlr2_final.py --s67`, which writes `tools/template_shapes/stlr3_decisions.txt`
and adds nine read-based drops: air-ops bodies under a critical section, `BSP_VehicleClass_*`
callers, entity destructors with fields past `200h`, and one 66-float-op body):

| origin | upgraded (shape) | upgraded (name kept) | reviewed (game) | dropped | kept library | not touched |
| --- | --- | --- | --- | --- | --- | --- |
| deferred by stlreview1 | 61 | 22 | 14 | 23 | 16 | 1 |
| new cc13-sweep67 bookmarks | 1 | 0 | 0 | 30 | 4 | 1 |

Not touched: `00675C40` (cc9's support-manager update; the common brief reserves it) and `007240E0`
(inside cc13-leftovers1's lease). No new cluster labels were needed: every multi-row cluster here
already had one. Ghidra: 62 renames, 22 `stl_instantiation` bookmarks on named rows, 151
`stl_probable` bookmarks deleted (`local/stlr-ghidra-*-20261012T145044Z.json`); `_free` unchanged.

196 `stl_probable` bookmarks remain image-wide: 176 library-shaped, 18 uncertain, and the 2 above.

### 9.7 Per-rule reliability, all passes

| rule | decided | game code | template copy | kept library | kept uncertain |
| --- | --- | --- | --- | --- | --- |
| `st1_scl97` (no size cap) | 452 | 366 (81%) | 30 (7%) | 49 | 7 |
| `e0029` (97-256 bytes) | 1494 | 391 (26%) | 965 (65%) | 127 | 11 |

Counts take each row's latest decision across the three decision files (1946 rows). The 98 bookmarks
the band sweeps added after stlreview1 (62 from cc13-sweep9x, 36 from cc13-sweep67) were 70% game
code, nearly all `st1_scl97` hits with strings or game calls (scoring awards, Lua bindings, the
hardware options text, air-ops stock). Section 3.1's proposal stands: cap or drop `scl_97plus`, and
do not bookmark on the `_invalid_parameter_noinfo` call alone.

### 9.8 Still to do

- `007240E0` once cc13-leftovers1 releases it; `00675C40` belongs to cc9.
- Name the 187 dropped game rows of this pass in the band sweeps (anchors in the decision files).
- Name the 0098-0099 iostream/locale bodies (9.5).
- Section 9.4: a union merge of tag shards undoes deletions; whoever owns `tools/merge_resolve.py`
  should decide whether it needs a tombstone for removed rows.
