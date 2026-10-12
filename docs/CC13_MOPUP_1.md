# Image-wide non-gameplay raw mop-up (cc13-mopup1)

Packet: name or tag the raw functions that are not gameplay, image-wide, and hand cc9 an ownership
table that sizes its remaining naming work. "Raw" follows `tools/progress_counts.py`: the snapshot
name is `FUN_`/`thunk_FUN_`, there is no tag row and there is no ledger name.

Every name below is a hypothesis drawn from the body, its callers, strings, vtables and RTTI, not a
recovered symbol. Nothing here is reconstructed C++; no function was created or deleted in Ghidra.

## Totals

| measure | value |
|---|---|
| raw before (fresh `bsp.py snapshot --force`, 2026-10-12T15:0x, index 76282 functions) | 7213 |
| raw after (same snapshot, ledgers of this branch merged with origin/main) | 4890 |
| ledger names added (`config/names`, evidence prefix `cc13-mopup1`) | 1202 |
| tag rows added (`config/tags`, `source: docs/CC13_MOPUP_1.md`) | 1120 |
| of the 4890 left: cc9 gameplay / non-gameplay not reached / leased to cc13-sweep79 | 4238 / 176 / 476 |
| reconstructed, build-tested, fixture-tested, game-validated | none (naming and tagging only) |
| `bsp.py ghidra proto 00BF9DC8` (`_free`) before and after every bulk apply | `void __cdecl _free(void * _Memory)`, unchanged |

Names by kind: 996 `BSP_*`, 82 `thunk_*`, 61 `EHHandler@*`, 60 CRT/C++ library names
(`_free_locale`, `type_info__Name_base`, `std_basic_filebuf_seekoff_00990740`, ...), 3 `TELEMETRY_*`.
Tags by category: 702 `stl_shape`, 245 `trivial_body`, 115 `static_init`, 25 `tmpl_shape`,
21 `cg_scalar_deleting_dtor`, 12 `cg_adjustor_thunk`.

## Method

All scripts are in the worktree's `local/` with the `mopup_` prefix; the ownership map is committed
as `tools/raw_ownership_cc13.py`.

1. Census. `mopup_census.py` lists the raw functions from the index with callers, callees and
   string references; `mopup_feat.py` adds a Capstone pass over the disk image (size, first
   mnemonics, named calls and jumps, strings, globals); `mopup_vt.py` finds every raw function
   referenced from a `.rdata`/`.data` table run and resolves the RTTI name and the named
   neighbouring slots; `mopup_rtti.py` prints a vtable's slots. Live decompiles were batched with
   `mopup_dec.py` (compact, 7-12 lines each) and annotated with the census features.
2. Ownership. `tools/raw_ownership_cc13.py` (copy of `mopup_owner.py`) holds address-range rules
   taken from the triage tables and "left for cc9" lists of docs/CC13_BAND_0040_0044.md,
   CC13_BAND_0045_0048.md, CC13_BAND_004B_004F.md, CC13_BAND_0050_0059.md,
   CC13_BAND_0060_0066.md, CC13_BAND_0067_0073.md, CC13_BAND_006D_006F.md,
   CC13_BAND_0074_0078.md, CC13_BAND_0080_008F.md, CC13_BAND_0090_00BF.md,
   CC13_BAND_00A1_00A4.md and CC13_LEFTOVERS_1.md, plus segment defaults (segments 0-7 and 9-29
   non-gameplay, segment 8 cc9). Rules added by this packet while reading bodies are marked
   `mopup:` (HUD unit pick and target cycling, formation and weapon-group command logic, the
   support-manager purchases/slots/messages 00656250-00673FFF, plane state predicates, border
   zones, scene unit entities, command-building and power-up predicates). `python
   tools/raw_ownership_cc13.py table` prints the per-band counts; `list <owner> [lo hi]` lists
   addresses.
3. Mechanical classes, image-wide (gameplay bands included, because the shapes carry no game
   meaning), skipping the cc13-sweep79 (0079-007F) and cc13-effects1 (00865000-00875FFF) leases:
   - `EHHandler@xxxxxxxx` (61): `mov eax, FuncInfo; jmp ___CxxFrameHandler3`, the naming the EH
     analyzer already uses for 6780 stubs;
   - `thunk_<target>` (82 incl. later passes): a single `jmp` to a named function;
   - `trivial_body` (245): at most two instructions ending in `ret` (repo rule, evidence
     `st1_trivial`);
   - `cg_adjustor_thunk` (12): `sub ecx,N; jmp` (evidence `st1_adjustor`);
   - `static_init` (115): functions inside `.text$yc/.text$yd` [00CC893B,00CE2000) (evidence
     `e0681`);
   - `BSP_NativeString_ReleaseStorage_*` (30): only `BSP_SizedStoragePool_GetSingleton` +
     `_ReturnBlock`, at most 48 bytes; `<Container>_Tidy_*` (21): `push 0; call <...>_Resize; call
     _free`.
4. The cc13-stl1 shape pipeline over all segments 0-97 (`st1_shapes.py 0..97`, `st1_auto.py`,
   `st1_rules.py`, `st1_family.py`, `st1_build.py` dry run): 508 rows written with source
   docs/CC13_MOPUP_1.md; the 581 `stl_probable` bookmarks it proposed were not written (they hide
   raw functions behind a weak bookmark). Twelve rows were wrong and were removed again with the
   Ghidra name reset to `FUN_` (`mopup_untag.py`): the `scl96` rule tagged three AI functions
   (00A12540, 00A12A50, 00A12F90, cc9's) and four CRT `write_string` copies, and `st1_twin` tagged
   five CRT `__finally` blocks (`push N; call __unlock; pop; ret`). The CRT ones carry names now.
   Their Ghidra bookmarks ("Inventory: stl_instantiation"/"stl_shape") were left in place.
5. Manual review in bands (`mopup_b01..b07.tsv`), applying with `mopup_apply.py`: ledger rows
   through `ledger.upsert` after `coordination.check_writable`, Ghidra names through
   `ghidra_annotate.py --apply`, tags through `ghidra_tag.apply` under `ghidra_lock`. Template
   copies found by hand went in as `stl_shape` tags with the new evidence id `mop1_stl`.
6. Six names that duplicated an existing name at another address were renamed
   (`BSP_AwardTracker_DrainHintDeque`, `BSP_Game_SetUserPresenceContextForMode`,
   `BSP_HudMovieCameraDebugScreen_BindWidgets`, `BSP_MultiGameLobbyScreen_BindWidgets`,
   `BSP_NativeString_FromUnsigned_005f1750`, `BSP_NativeString_LessCaseInsensitive_0076eaa0`).

## What was named, by area

- CRT and C++ library (00BE-00C3, 0099): printf/scanf cores and their helpers (`write_string`,
  `_output_p_l_00c0917d`, `_validate_param_reuse`, `get_int64_arg`, `__check_float_string`,
  `_inc`, `_whiteout`, `_input_l_variant_*`), `type_info` name list, `_create_locale` /
  `_free_locale`, `__initstdio` / `__endstdio` / `_fcloseall` / `_flushall` / `__ioterm` /
  `__termcon`, `__except_handler3`, `_threadstartex`, the `__finally` unlock blocks;
  `basic_filebuf` seekoff/seekpos/open/close, `ctype<char>` members, `ios_base::clear`,
  `use_facet` copies, `_Locinfo`, `runtime_error`. Which of the output/input core variants is
  `_s`/`_p` is provisional.
- Engine (00A5-00BD): MPAK/MSAR archive records and their vector/range templates, inflate stream
  helpers, archive reader/writer references (00BD4EB0/00BD4F30 and their 17/16-caller factories),
  archive text writer blocks, Lua object readers/writers (vector3/2, matrix, integer strings),
  singleton lifetime clients, terrain grids and blocks, COM `Release`/refcount helpers, render
  batch pools, NativeString sort instantiation (`std::sort` with `__stricmp`), scene type-info
  `IsKindOf` lists, resource nodes, D3D9 renderer device slots, hardware probe (CPU, driver
  version, memory), download manager and content packages, XLive stats blocks.
- GUI and front end (0050-005F, 0043-004F, 0067-0068): HUD map/minimap/narrative/objectives/pause/
  order/free-camera/formation/counter screens, MovCam setters, multiplayer lobby, session
  browser, mode selector, main menu, options and control binding screens, chat lists,
  leaderboards, tactical and unit libraries, briefing, mission tree records, prompt callbacks,
  multiplayer menu interface stack (`GVMultiMenu::PopRequestInterfaceUpTo`).
- Network and session (0076-0078, 008D-008E, 00A5): voice payload bit I/O, session host info
  and DLC check, peer/slot helpers, objective sync records, packet statistics, iostdnet HTTP
  Content-Range.
- Scene/ambient (0045-0048): scene database and record leftovers listed in
  docs/CC13_BAND_0045_0048.md, class registry walkers, cloud particle arrays, traffic containers,
  polygon partition geometry, scene property bag helpers.

RNG note: 00BF0DF0 (`BSP_RandomState_NormalFloat_00bf0df0`), 00AF5A30/00AF7F70/00AF7FB0 (particle
random helpers) and 007077B0 (`BSP_FishSchoolModel_RespawnFish`) draw from `BSP_RandomThreads` /
the shared generator; nothing here wires them into the host.

## Non-gameplay left (176)

Bodies that need more than a compact decompile (large HUD map routines in 0059-005B, lobby and
setting-row builders in 005D, the terrain/foliage remainder in 00AD-00AE, shader/texture vtable
slots in 00B2-00B4 without RTTI, Lua object library 00B6-00B8, x87 geometry). The full address
list is `non_gameplay_left` in reports/cc13_mopup1_functions.json and `python
tools/raw_ownership_cc13.py list ng`.

## Ownership table for cc9

Raw functions per 64 KB band after this packet (index of 2026-10-12, snapshot 76282 functions).
"cc9" counts are the gameplay functions left raw; the segment column lists the top segments of
those functions, and the source column names the doc rule that classified them. The
cc13-sweep79 band (0079-007F, 476 raw) was still running and is counted separately.

| band | cc9 (gameplay) | non-gameplay left | leased (sweep79/effects1) | segments of the cc9 raw | source of the cc9 classification |
|---|---|---|---|---|---|
| 0041 | 44 | 0 | 0 | 1 (44) | 0040 doc: AvoidZone groups/layers/manager; 0040 doc: AvoidZone outline/segments |
| 0042 | 22 | 0 | 0 | 1 (15), 2 (7) | 0040 doc: AvoidZone groups/layers/manager; 0040 doc: plane HUD and zoom fields |
| 0043 | 9 | 3 | 0 | 3 (8), 2 (1) | 0040 doc: gun, device class, descriptors, class-id predicates; 0040 doc: water-height probe with a hit record |
| 0044 | 28 | 0 | 0 | 3 (28) | 0040 doc: gun, device class, descriptors, class-id predicates |
| 0045 | 50 | 1 | 0 | 5 (50) | 0045 doc: engine movie, movie actions, entity-command session messages |
| 0046 | 19 | 0 | 0 | 5 (13), 6 (6) | 0045 doc: engine movie, movie actions, entity-command session messages; 0045 doc: squadron auto-target ordnance latch |
| 0047 | 6 | 1 | 0 | 6 (6) | 0045 doc: path/unit helpers, attack gate 0047B850 |
| 0048 | 2 | 6 | 0 | 7 (2) | mopup: traffic runtime view gate (check with cc9, 0045 doc); mopup: traffic behaviour update (shared RNG, 0045 doc) |
| 0049 | 75 | 0 | 0 | 8 (75) | segment 8 default |
| 004A | 80 | 0 | 0 | 8 (80) | segment 8 default |
| 004B | 33 | 2 | 0 | 8 (32), 9 (1) | 004B doc: soldier registry, wreck class, explosion smoke; segment 8 default |
| 004C | 5 | 0 | 0 | 9 (5) | mopup: world border zone layout/queries; mopup: multiplayer unit selection update |
| 004D | 0 | 1 | 0 |  |  |
| 004E | 9 | 2 | 0 | 11 (9) | mopup: scene unit entities, unit list registration, JapanList stock; mopup: scene game-entity constructors |
| 004F | 22 | 1 | 0 | 11 (22) | 004B doc: spawn point, land convoy, unit owner entity |
| 0050 | 0 | 2 | 0 |  |  |
| 0052 | 16 | 0 | 0 | 14 (16) | mopup: HUD unit pick / target cycle / attack-move command logic (0050 doc: pick/command logic left for cc9) |
| 0053 | 5 | 0 | 0 | 15 (5) | mopup: formation command issue / update; mopup: formation command messages |
| 0054 | 8 | 1 | 0 | 16 (8) | mopup: weapon aim / group firing, command-building respawn; mopup: weapon group availability |
| 0056 | 0 | 2 | 0 |  |  |
| 0059 | 0 | 18 | 0 |  |  |
| 005A | 0 | 13 | 0 |  |  |
| 005B | 0 | 3 | 0 |  |  |
| 005C | 0 | 2 | 0 |  |  |
| 005D | 0 | 9 | 0 |  |  |
| 005F | 17 | 2 | 0 | 23 (17) | mopup: HUD order screen unit commands (attackmove, regroup, attack, return to base) |
| 0060 | 6 | 0 | 0 | 24 (4), 23 (2) | mopup: plane instrument/effects maths; mopup: plane state predicates (0060 doc) |
| 0062 | 0 | 2 | 0 |  |  |
| 0063 | 0 | 1 | 0 |  |  |
| 0064 | 1 | 0 | 0 | 26 (1) | mopup: unit ordnance predicate |
| 0065 | 15 | 0 | 0 | 27 (15) | mopup: support manager purchases, slots, spawns and its session messages (0060 doc: cc9 for the support-manager logic) |
| 0066 | 12 | 0 | 0 | 27 (12) | mopup: support manager purchases, slots, spawns and its session messages (0060 doc: cc9 for the support-manager logic) |
| 0067 | 5 | 0 | 0 | 27 (5) | mopup: support manager purchases, slots, spawns and its session messages (0060 doc: cc9 for the support-manager logic) |
| 006A | 8 | 0 | 0 | 29 (8) | 0067 doc: ship AI landing approach / command-building landing paths |
| 006B | 46 | 0 | 0 | 29 (29), 30 (17) | 0067 doc: air ops |
| 006C | 89 | 0 | 0 | 30 (89) | 0067 doc: air ops |
| 006D | 84 | 0 | 0 | 30 (66), 31 (10), 33 (8) | 006D doc: air-ops sites, bombs/projectiles, buildings, squadron commands, ships |
| 006E | 178 | 0 | 0 | 33 (178) | 006D doc: air-ops sites, bombs/projectiles, buildings, squadron commands, ships |
| 006F | 195 | 1 | 0 | 33 (128), 34 (67) | 006D doc: air-ops sites, bombs/projectiles, buildings, squadron commands, ships; 006D doc: cruiser constructor 006FB300 |
| 0070 | 34 | 0 | 0 | 34 (24), 36 (10) | 0067 doc: door/cargo/bomb-carrier units; 0067 doc: flak bullet, ship AI unit lists, unit part effects |
| 0071 | 54 | 0 | 0 | 38 (52), 36 (2) | 0067 doc: weapon director, command controllers; 0067 doc: flak bullet, ship AI unit lists, unit part effects |
| 0072 | 66 | 0 | 0 | 40 (42), 38 (16), 39 (8) | 0067 doc: guns, gun messages, mission countdown; 0067 doc: weapon director, command controllers |
| 0073 | 43 | 0 | 0 | 40 (40), 41 (3) | 0067 doc: guns, gun messages, mission countdown; 0067 doc: Bink movie wiring |
| 0074 | 141 | 0 | 0 | 41 (141) | 0074 doc: land units, carrier deck, session-message classes |
| 0075 | 81 | 0 | 0 | 42 (42), 41 (39) | 0074 doc: land units, carrier deck, session-message classes |
| 0076 | 33 | 0 | 0 | 42 (33) | 0074 doc: land units, carrier deck, session-message classes |
| 0077 | 45 | 0 | 0 | 43 (45) | 0074 doc: entity replication; mopup: AI command predicate |
| 0078 | 10 | 0 | 0 | 43 (7), 44 (3) | 0074 doc: entity replication; 0074 doc: plane water-surface law |
| 007A | 0 | 0 | 51 |  |  |
| 007B | 0 | 0 | 173 |  |  |
| 007C | 0 | 0 | 74 |  |  |
| 007D | 0 | 0 | 60 |  |  |
| 007E | 0 | 0 | 85 |  |  |
| 007F | 0 | 0 | 33 |  |  |
| 0080 | 156 | 0 | 0 | 49 (135), 50 (21) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0081 | 108 | 0 | 0 | 50 (74), 51 (34) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0082 | 70 | 0 | 0 | 52 (64), 51 (6) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0083 | 32 | 0 | 0 | 52 (32) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0084 | 114 | 0 | 0 | 52 (114) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0085 | 87 | 0 | 0 | 53 (79), 52 (8) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots; mopup: intercept solution maths (plane/gun bot) |
| 0086 | 40 | 0 | 0 | 53 (40) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0087 | 34 | 0 | 0 | 55 (25), 54 (9) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0088 | 99 | 0 | 0 | 55 (69), 56 (30) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots; leftovers doc: avoid-zone byte grid / shallow water |
| 0089 | 91 | 0 | 0 | 56 (91) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 008A | 70 | 0 | 0 | 56 (70) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 008B | 22 | 0 | 0 | 56 (22) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 008C | 49 | 0 | 0 | 56 (49) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 008D | 23 | 0 | 0 | 56 (23) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 008E | 76 | 0 | 0 | 58 (70), 57 (5), 59 (1) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots; mopup: command building / AI party / power-up predicates |
| 008F | 74 | 0 | 0 | 59 (74) | 0080 doc: units, weapons, Lua host and natives, power-ups, bots |
| 0090 | 75 | 0 | 0 | 59 (75) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0091 | 19 | 0 | 0 | 59 (19) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0092 | 87 | 0 | 0 | 60 (75), 59 (8), 61 (4) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0093 | 45 | 0 | 0 | 61 (45) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0094 | 66 | 0 | 0 | 61 (66) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0095 | 125 | 0 | 0 | 62 (106), 61 (19) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0096 | 92 | 0 | 0 | 64 (57), 63 (19), 62 (16) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0097 | 128 | 0 | 0 | 64 (76), 65 (52) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0098 | 4 | 1 | 0 | 65 (4) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 0099 | 60 | 11 | 0 | 67 (59), None (1) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 009A | 70 | 0 | 0 | 67 (70) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 009B | 110 | 0 | 0 | 67 (110) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 009C | 68 | 0 | 0 | 67 (68) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 009D | 67 | 0 | 0 | 67 (67) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 009E | 50 | 0 | 0 | 67 (50) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 009F | 71 | 0 | 0 | 68 (47), 69 (16), 67 (8) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 00A0 | 84 | 0 | 0 | 69 (84) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 00A1 | 134 | 0 | 0 | 70 (87), 69 (47) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 00A2 | 88 | 0 | 0 | 70 (88) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 00A3 | 19 | 0 | 0 | 70 (18), 71 (1) | 0090/00A1 docs: bots, entities, vehicle classes, tasks, AI |
| 00A9 | 0 | 2 | 0 |  |  |
| 00AC | 0 | 1 | 0 |  |  |
| 00AD | 2 | 3 | 0 | 81 (2) | terrain doc |
| 00AE | 33 | 9 | 0 | 81 (33) | terrain doc: segment line test (gameplay lane) |
| 00AF | 0 | 2 | 0 |  |  |
| 00B1 | 0 | 3 | 0 |  |  |
| 00B2 | 0 | 7 | 0 |  |  |
| 00B3 | 0 | 4 | 0 |  |  |
| 00B4 | 0 | 6 | 0 |  |  |
| 00B6 | 0 | 5 | 0 |  |  |
| 00B7 | 0 | 1 | 0 |  |  |
| 00B8 | 0 | 16 | 0 |  |  |
| 00B9 | 0 | 1 | 0 |  |  |
| 00BB | 0 | 7 | 0 |  |  |
| 00BC | 0 | 6 | 0 |  |  |
| 00BD | 0 | 2 | 0 |  |  |
| 00BE | 0 | 2 | 0 |  |  |
| 00C1 | 0 | 3 | 0 |  |  |
| 00C3 | 0 | 9 | 0 |  |  |
| 00C4 | 0 | 1 | 0 |  |  |
| 00C5 | 0 | 1 | 0 |  |  |
| **total** | **4238** | **176** | **476** | | |

Gameplay totals by area (cc9, 4238 in all): 0080-008F units, weapons, mission Lua host and natives,
power-ups, bots 1145; 0090-009F bots, game entity, vehicle classes, owner player, plane/bot tasks
1137; 0067-0078 air ops, land/sea unit classes, guns, weapon director, carrier deck, replication
1112; 00A0-00A3 AI commands, planner and controller 325; 0049-004B soldier types, land vehicle and
wreck classes 188; 0041-0048 AvoidZone, guns and device classes, engine movie, hit records 180;
004C-0066 HUD unit pick/command logic, support manager, plane predicates, scene unit entities 116;
00A4-00CE the terrain segment line test 35.

## Open questions

1. The `_output_*`/`_input_*` core variants (00C0917D, 00C09D07, 00C0AD78, 00C0BA79, 00C10440,
   00C14DE0) need a positional-parameter or secure-check discriminator to pick `_l`, `_s_l` or
   `_p_l`; 00C10440 and 00C14DE0 were not named.
2. 00BB8580 (division by 2F1h), 00BE4410 and 00C301C0 were not identified.
3. The stl_probable bookmarks proposed by the pipeline (581) were not written; a stlr-style review
   could turn part of them into tags.

## Handoff

Branch `agent/cc13-mopup1`, merged with origin/main. Scripts: worktree `local/mopup_*`. Leases
`cc13-mopup1*` released at the end. Next steps: the 176 non-gameplay functions above (most need the
listing or a vtable class map), and cc9's table.