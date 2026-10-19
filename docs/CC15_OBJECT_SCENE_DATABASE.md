# cc15-ob4: the scene database at [00E18680] (2026-10-19)

Lane L4 of cc15's object bring-up (docs/CC15_WIRE_FRONTIER.md, rank 6). Switch:
`kSceneDatabaseCc15Bound` in `include/bsp/game_hosts_scene_contents.hpp`. Every name below is a
hypothesis, not a recovered symbol.

## The object, as the image builds it

- **Constructor and size.** 0046F350 (cdecl, no arguments): `operator new(170h)` at 0046F36B,
  0046F160 on the block at 0046F384, store to `[00E18680]` at 0046F389. A failed allocation stores 0
  (0046F3A3). Reused body: `scene_database_create_singleton_0046f350` (src/seg6_cc13.cpp), which runs
  `scene_database_construct_0046f160`. The layout is in seg6_cc13.hpp: vtable 00CE59FC, +04h byte 1,
  +08h vector, +18h hidden-record map, +24h pending vector, +34h class map (vtable 00CE55B0, +38h
  count, +3Ch 64 bucket heads), +158h set {44h, 5Eh, 1Dh, 47h, 3Dh}, +164h set {47h, 3Dh}.
- **When it is built.** BSP_Game_OnInit 004E3AA0 calls 0046F350 at 004E3E60, between the
  manager at 00E1930C (0.17/0.18 progress) and the 0.22 progress report (cc13_sweepA4.cpp:525). The
  host's model of OnInit is `FrontEndShell::game_on_init` (game_hosts_menu.cpp), which
  frontend_entry.cpp:269 runs once per process. The ON build calls
  `scene_database_create_0046f350` there.
- **When it is destroyed.** OnDestroy 004DC5C0 calls 0046F3E0 (vtable slot 0 with 1, then
  `[00E18680] = 0`). OnDestroy is not hosted: 00737F80's singleton teardown is a record in
  game_hosts.cpp. The block therefore lives to process exit. No destructor path was invented.
- **What fills it.** BSP_Game_LoadMissionScene 004DFB70 calls 004F2800 on every load: 26 x
  (`MOV ECX,[00E18680]`; push register, create, id, name; `CALL 004EE250`). 004EE250 takes ECX =
  database and adds 34h itself (004EE2A8 `LEA ECX,[EDI+34h]`). The host step is
  `MissionLoad::resolve_named_scene_objects` in game_hosts_mission_frame.cpp's
  MissionSceneLoadHost walk; the ON build calls `scene_database_register_classes_004f2800` there.
  The 26 pushes are `scene_entity_class_table()` (scene_entity_factory.cpp), in order.

## Which side is authoritative

The native class map at database+34h. The host had one owner of this state before: the static
`scene_entity_class_table()` lookup (`find_scene_entity_class_by_name` / `_by_id`). With the switch
ON, the five lookup sites in game_hosts_scene_contents.cpp go through
`scene_class_row_by_name` / `scene_class_row_by_id`:

- by name: 00468FB0 over the map (the find 0046CF40 uses for a class token), then the descriptor at
  node+8 gives the class id, and the static row is joined on that id. The join checks that create and
  register match the descriptor and throws if they do not.
- by id: 00468660 over the map (what 0046BE90 uses); null when the map has no such id.

So the static table is no longer a second owner. It is the input to 004F2800 and the source of the
metadata the descriptor does not hold (instance size, creator kind, type key). With [00E18680] null
(OnInit's call not reached), the static table still answers; with it built and 004F2800 not yet run,
the map answers null, as the image would.

## Host plumbing (labelled substitutions)

`SceneDatabaseSeg6Host` implements the Seg6Host calls these paths reach. The rest throw
`std::logic_error`.

- 00BF681B / 00BF55BE / 00BF65AC: `::operator new` / `::operator delete`.
- 00419CC0 + 00BD1510 and 0041DD40 / 0041E870: `crt_string_storage()`.
- 00438E10: `compare_insensitive_00438e10`. 00466BB0 / 00466C00: the raw iterator bodies.
- 0046AC50: `insert_native_scene_integer_set_key_0046ac50`. Its result is 0Ch (owner, node,
  inserted byte). It writes 9 bytes: a dword at +0, a dword at +4 and a byte at +8. 0046F160's result
  slot is the full 0Ch ([ESP+18h..23h], ending at the SEH link). seg6_cc13.cpp declares
  `inserted[3]` (cc13 fa5c61133, main d7bed7752), and the adapter passes out_pair straight through.
  It originally wrote into a 12-byte temp and copied 8 bytes out, a workaround for the old 8-byte
  `inserted[2]`; both are gone.
- 00468260 / 004682B0 head nodes: new(1Ch / 14h), links zeroed, colour byte 1. 0046F160 sets the
  nil byte and the self links itself.
- 004E7C00 (14h slots of the node pool 00E18514): new(14h).
- 004EE250 allocates its descriptor with 00BF681B (004EE26E). The interface method is named
  `new_descriptor_00bf55be`. It is operator new either way here.

## Rows wired

| Row | Before (USN04) | ON |
|---|---|---|
| 0046F350 at OnInit | not in the table | `Game::OnInit scene_database_create_singleton` implemented |
| 004F2800 `MissionLoad::resolve_named_scene_objects` | UNIMPLEMENTED calls=1 | done |
| 004E99B0 `SceneContents::class_creator` (NavPoint) | UNIMPLEMENTED calls=1 | implemented |

The `MissionLoad(scene-graph)::resolve_named_scene_objects` row in game_hosts_mission.cpp is the
milestone-2e inventory loop. It logs every step without running any of them, so it is not a call
site and stays as it is.

004E99B0, 004E9AE0 and 004E9C10 (NavPoint 41h, MovieCamPos 42h, MovieCamLookat 43h) share
`create_named_scene_entity_004e99b0` with a FixedEntitySpec. ON runs that body over a zeroed 1E4h
host block, with these calls:

- 006AF3D0 is the reconstructed stub (`network_id_from_name_stub_006af3d0`, id 0).
- GameEntity 00928630 and the vtable +98h placement at 004E9A80 are not hosted. They become the new
  records `SceneContents::game_entity_construct [00928630]` and
  `SceneContents::named_entity_place_vslot98 [004e9a80]`.

The image keeps what the creator writes itself: four vtables, the class id at +C4h, and the name at
+154h. The scene record stays the entity: `created` stays false, so create_units and the
markers pass see what they saw before. Only USN04's NavPoint reaches this path.

## Pairs

All USN04, `--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800
--mission-frame-seconds 0.05`, through tools/run_game.ps1, 2026-10-19.

- **OFF.** This tree (switch OFF, all three call sites in) against a build of the base a84b1ddac:
  `pair_diff` exit 0. The native table, summary lines and other lines are identical apart from noise.
- **ON against OFF.** `pair_diff` exit 1, gameplay identical: deaths 62, hits 882, damage 14522.4, the
  unit table (90 rows) and the plane death modes (62) are identical. Every moved row:
  - `+ Game::OnInit scene_database_create_singleton 0046f350` concrete, plus the note line: vtable
    00CE59FC, +04h=1, class map count 0, +158h set size 5, +164h set size 2. These are the five and
    two keys 0046F160 inserts, so the 0046AC50 path built both trees.
  - `~ MissionLoad::resolve_named_scene_objects 004f2800` UNIMPLEMENTED -> concrete, plus the note
    "registered 26 classes, class map count=26". The summary line `mission load: concrete 17 -> 18,
    records 26 -> 25` and the `mission load finished` line move with it.
  - `~ SceneContents::class_creator 004e99b0` UNIMPLEMENTED -> concrete (NavPoint `IJNRetreat`). New
    rows: `006af3d0` concrete, `00928630` and `004e9a80` UNIMPLEMENTED (one call each). Also the
    `scene named entity` note (+C4h = 41h, size 1E4h). The NavPoint class line's creator state reads
    `image 004e99b0` instead of `record 004e99b0`, with `created=0` unchanged.
  - Every lookup now goes through the native map. No class line, stock line or vehicle-class
    census/purge line moved. kVehicleClassPreloadCc15Bound is OFF on origin/main (f403f1bf4, the
    USN21 crash), so its purge lines are absent from both runs. Its census reads
    `scene_class_row_by_name` like the other sites.

## Flipped ON (cc15-flip, merged base 4cc9872bc = origin/main 2e21536b2 + the flip, 2026-10-19)

OFF = the merged base built with the switch false, ON = the same tree with it true, run through
tools/run_game.ps1 with `BSP_DEATH_TABLE=1`.

- **USN04** (the pair above): gameplay identical. Deaths 56, hits 730, damage 12958.6, 84 units; main
  moved these since the earlier pair, on both sides. The moved rows are exactly the earlier list,
  with nothing new.
- **Wins set** (ref env `BSP_GUNNERY_RNG_STREAMS=1`, `--stop-after-end 40`): `pair_diff` reports
  gameplay identical on all 12 rows. The end, the clock, the death rows, the unit table and the guard
  line match per row. These are the end results on this base:

  | Row | Orders | End (OFF = ON) |
  |---|---|---|
  | USN01 | usn01_l69_u1_a | completed 1200.05 s |
  | USN02 | usn02_l68_u2_a | failed 380.03 s (Game Over) |
  | USN12 | usn12_l68_u12_e8 | completed 1335.18 s |
  | PRCP01 | prcp01_p9_p01_k | completed 1010.23 s |
  | USN21 | s46_u21_f | completed 2906.71 s |
  | ESMP08 | esmp08_p12_e8_rb | completed 1454.44 s |
  | BSM09 | bsm09_l68_b9_d1 | EndMission (nil) 285.05 s |
  | LOMP06 | lomp06_l70_l6_c | failed 657.11 s |
  | LOMP10 | lomp10_p3_l10_b | no end |
  | BSM01 | bsm01_l69_b1_d | no end |
  | IJN18 | ijn18_l67_i18_d | no end |
  | JM06 | jm06_s45_j6_k | no end |

- Not caused by the flip (both sides alike): on this main, USN02, LOMP06, LOMP10, BSM01, IJN18 and
  JM06 lose the wins docs/WINS_TABLE.md records for these order files.
- These rows moved beyond the USN04 list, and each is explained:
  - MovieCamPos 004E9AE0 and MovieCamLookat 004E9C10 creators run on JM06 and BSM09. They use the
    same shared body, and the class lines read `image` in place of `record`.
  - The `scene named entity` note prints once per named NavPoint, MovieCamPos and MovieCamLookat.
  - The ring-scan `clear_37c` of a unit that does not scan can move. On BSM01, one PT reads
    FLT_MAX -> 12672.0. On the earlier f10dce645 pair, LOMP10's followers moved: Kashi 5444.4 ->
    10000.0 and Kasumi FLT_MAX -> 10000.0. The ShipAiSectorScan and ShipAiClearance counters that
    pair_diff already masks move with it. This is the heap-dependent searcher-bounds value of
    SHIP_AI_OPEN_ITEMS 140: the image has no initialiser, the field is diagnostic only, and the
    pair_diff mask covers only the sentinel values.

## Open items

- OnDestroy 004DC5C0 / 0046F3E0 is not hosted (with 00737F80).
- GameEntity 00928630 and placement 004E9A80 are records, so the NavPoint image is not yet an entity.
- Fixed: the 0046F160 result local was 8 bytes for a 0Ch result (cc13 fa5c61133, main d7bed7752).
- Pending at cc13: rename 004EE250's allocator seam new_descriptor_00bf55be to 00bf681b (CALL 00BF681B
  at 004EE26E), including the override at game_hosts_scene_contents.cpp:847.
