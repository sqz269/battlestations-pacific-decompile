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
  inserted byte). seg6's 0046F160 body passes an 8-byte local (`inserted[2]`), so the adapter writes
  into its own 12-byte buffer and copies 8 bytes out. Worth a fix in seg6_cc13.cpp: the image's local
  at 0046F160 is the full 0Ch.
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

(filled in below)

## Open items

- OnDestroy 004DC5C0 / 0046F3E0 is not hosted (with 00737F80).
- GameEntity 00928630 and placement 004E9A80 are records, so the NavPoint image is not yet an entity.
- The 0046F160 `inserted[2]` local in seg6_cc13.cpp is 8 bytes for a 0Ch result.
