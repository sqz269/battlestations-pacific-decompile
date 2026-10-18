# CC13 seg59b: segment 59 gun bots (packet `cc13-seg59A`)

Grant: the cc9 gameplay lead opened segment 59 (008EE670-00922240) to cc13 on 2026-10-18 on its
gameplay terms (gunner bots, the torpedo bot, tail gunner, pilot bot). The earlier `cc13-seg59a`
pass (docs/CC13_SEG59A.md) covered the scoring band from 00905300; this pass covers the gun bots.
Code: `src/cc13_seg59b.cpp`, `include/bsp/cc13_seg59b.hpp`. Nothing is wired into bsp_game; no
game runs. Status of every reconstructed function: reconstructed, build-checked; not
fixture-tested, not ABI-compatible, not game-validated. Names are hypotheses.

## Model

Every routine takes the original bot as raw bytes and reaches the engine through `Seg59bHost`.
The property archive (save/load) is reached through four virtual-slot methods: +04h open section,
+08h close section, +0Ch write field, +10h read field; each key and value is the image's 8-byte
by-value pair `{kind, value}` (kind 0 = C string key, 2 = float: its bits when written, its
address when read).

## Torpedo bot (vtable 00D182B0)

The gameplay slots of the torpedo bot were already hosted by cc9 (docs/SHIP_AI_OPEN_ITEMS.md,
TorpedoBot slot by slot). The rows below are what was left.

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008FF260` | BSP_GunBot_ConstructTorpedoClass | reconstructed | `torpedo_bot_construct_008ff260`: `0072BBD0(bot, [00E1998C])` at 008FF269, primary vtable `00D182B0` (008FF276), observer vtable `00D18294` at +1Ch (008FF27C), +68h = 10000.0f (`00CE3D64`, MOVSS bit copy, 008FF283). |
| `008FF360` | BSP_GunBot_ReadPropertiesTorpedoClass | reconstructed | `torpedo_bot_read_properties_008ff360`, slot +48h: section "_bot" (008FF381), `008FE660` (008FF386), close (008FF392), then `thinkTimeLeft` -> +5Ch (008FF3BE) and `targetAngle` -> +60h (008FF3EA) by address, then +58h = +50h (008FF3EF). Save/load only. |
| `008FF400` | BSP_GunBot_WritePropertiesTorpedoClass | reconstructed | `torpedo_bot_write_properties_008ff400`, slot +4Ch: the writer twin through slot +0Ch with the float bits of +5Ch (008FF462) and +60h (008FF492). Save/load only. |
| `009035A0` | BSP_TorpedoBot_SetFireTarget | ledger-only | Hosted inline at the cc9 gunnery caller (src/game_hosts_gunnery.cpp, 00727F44 -> 009035A0) and `torpedo_gun_set_bot_fire_target_00727f10` (src/ship_torpedo_ver_cc13.cpp). Spot check: 00521EA0(bot+38h) vs target (009035B5 SETNZ), 006DF170 stores (009035B8), 00903420 recomputes only on a change (009035C3) with the current +6Ch flag. No defect seen. |
| `008FD620` | BSP_RobotConfig_ConstructTorpedoBot | ledger-only | Hosted: src/robot_config.cpp `construct_descriptor<TorpedoBotConfig>("TorpedoBot", 0x00d17ff8)`. Base vtable `00D17DC0`, `00438E40` name duplicate into +8, derived vtable `00D17FF8`. |
| `008FD830` | BSP_RobotConfig_DestructTorpedoBot | ledger-only | Hosted: src/robot_config.cpp `delete_robot_descriptor`. Base vtable, name freed by `00BF6989` and cleared. |

No torpedo-bot body here draws on the shared RNG `00BD2F10`. No adoption note: the host matches the
binary on every row checked.

## Gun-bot class machinery

The class names in the image's ledger ("lead", "turret", "ballistic") map to robot classes by the
descriptor global each constructor passes to `0072BBD0`: lead = AAGunnerBot `[00E19998]`, turret =
TailGunnerBot `[00E199A0]`, ballistic = AAFlakBot `[00E1999C]`. Vtable slots: +04h attach, +30h
clear fire target, +38h set fire target, +48h / +4Ch save/load reader / writer.

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008FBC80` | BSP_GunBot_Attach | reconstructed | `gun_bot_attach_008fbc80`: `00876020(ECX = parent, bot)` (008FBC8B), +50h = [parent+28h], then bot vt[18h](unit vt[12Ch]()) when the unit passes vt[5Ch](5) (008FBCBC), else vt[18h](1) (008FBCCC). Live at mission load. |
| `008FE170` | BSP_GunBot_ReadTargetTypeProperties | reconstructed | `gun_bot_read_target_type_properties_008fe170` on the bot+38h record: section "trgType", two index-keyed bytes, then "eId" (word +2, read through the archive argument's own stack slot), "e" {4, +4}, "pos" {5, +8}, "range" {2, +14h}. Save/load. |
| `008FE2A0` | BSP_GunBot_WriteTargetTypeProperties | reconstructed | `gun_bot_write_target_type_properties_008fe2a0`: the writer twin; byte values store only the low byte (008FE2CE). Save/load. |
| `008FE660` | BSP_GunBot_ReadBaseProperties | reconstructed | `gun_bot_read_base_properties_008fe660`: section "_process" with `008746F0` (008FE687), "level" into +34h (008FE6C1), then `008FE170` (008FE6C4). Save/load. |
| `008FE6D0` | BSP_GunBot_WriteBaseProperties | reconstructed | `gun_bot_write_base_properties_008fe6d0`: `00874700` (008FE6F7), "level" from +34h, then `008FE2A0` (008FE72C). Save/load. |
| `008FE740` | BSP_GunBot_ConstructLeadClass | reconstructed | `aa_gunner_bot_construct_008fe740`: `0072BBD0` (008FE749), vtables `00D180A0` / `00D18088`, +5Ch = 0. |
| `008FBDC0` | BSP_GunBot_AttachLeadClass | reconstructed | `aa_gunner_bot_attach_008fbdc0`: `008FBC80` (008FBDC8), +5Ch = +50h, +60h..+70h = 0.0f. |
| `008FE7A0` | BSP_GunBot_ClearFireTargetLeadClass | reconstructed | `aa_gunner_bot_clear_fire_target_008fe7a0`: `0072BD50` (008FE7A3), then `[bot+5Ch]`->vt[1E8h](0) when the owner slot is 8 or `00927F10` holds it (008FE7B7). |
| `008FE800` | BSP_GunBot_ReadPropertiesLeadClass | reconstructed | `aa_gunner_bot_read_properties_008fe800`: "_bot", `008FE660` (008FE826), calcTick, angleErrTick, actAngleErr, dAngleErr; +5Ch = +50h. Save/load. |
| `008FE8F0` | BSP_GunBot_WritePropertiesLeadClass | reconstructed | `aa_gunner_bot_write_properties_008fe8f0`: `008FE6D0` (008FE916) and the four fields. Save/load. |
| `008FE9F0` | BSP_GunBot_ConstructTurretClass | reconstructed | `tail_gunner_bot_construct_008fe9f0`: `0072BBD0` (008FE9F9), byte +58h = 0, +68h = 0, vtables `00D18140` / `00D18124`. |
| `008FBEC0` | BSP_GunBot_AttachTurretClass | reconstructed | `tail_gunner_bot_attach_008fbec0`: `008FBC80` (008FBEC8), +68h = +50h, byte +58h = 0, +70h/+74h = 0.0f. |
| `008FEA60` | BSP_GunBot_ClearFireTargetTurretClass | reconstructed | `tail_gunner_bot_clear_fire_target_008fea60`: `0072BD50` (008FEA63), `00927F10` (008FEA77), `[bot+68h]`->vt[1E8h](0). |
| `008FEAC0` | BSP_GunBot_ReadPropertiesTurretClass | reconstructed | `tail_gunner_bot_read_properties_008feac0`: `008FE660` (008FEAE6) and eleven fields (actFireState .. shootRange); +68h = +50h. Save/load. |
| `008FECF0` | BSP_GunBot_WritePropertiesTurretClass | reconstructed | `tail_gunner_bot_write_properties_008fecf0`: `008FE6D0` (008FED16) and the same eleven fields. Save/load. |
| `008FEFD0` | BSP_GunBot_ConstructBallisticClass | reconstructed | `aa_flak_bot_construct_008fefd0`: `0072BBD0` (008FEFD9), vtables `00D18238` / `00D18220`, +64h = 0.0f, +68h = 0. |
| `008FDBA0` | BSP_GunBot_ClearFireTargetBallisticClass | reconstructed | `aa_flak_bot_clear_fire_target_008fdba0`: `0072BD50` (008FDBA3), `00927F10` (008FDBB7), `[bot+68h]`->vt[1E8h](0). |
| `008FF060` | BSP_GunBot_ReadPropertiesBallisticClass | reconstructed | `aa_flak_bot_read_properties_008ff060`: `008FE660` (008FF086), horzAngleError, vertAngleError, distError; +68h = +50h. Save/load. |
| `008FF130` | BSP_GunBot_WritePropertiesBallisticClass | reconstructed | `aa_flak_bot_write_properties_008ff130`: `008FE6D0` (008FF156) and the three floats. Save/load. |
| `008FF4A0` | BSP_GunBot_ConstructDepthChargeClass | reconstructed | `depth_charge_bot_construct_008ff4a0`: `0072BBD0` (008FF4A9), vtables `00D18338` / `00D1831C`, +58h = 0, +60h = -1.0f (`00D7A260`). |
| `008FF560` | BSP_GunBot_ReadPropertiesDepthChargeClass | reconstructed | `depth_charge_bot_read_properties_008ff560`: `008FE660` (008FF586), thinkTimeLeft; +58h = +50h. Save/load. |
| `008FF5D0` | BSP_GunBot_WritePropertiesDepthChargeClass | reconstructed | `depth_charge_bot_write_properties_008ff5d0`: `008FE6D0` (008FF5F6), thinkTimeLeft. Save/load. |
| `008FE120` | BSP_Entity_AsClass05OrNull | reconstructed | `entity_as_kind05_or_null_008fe120`: null or vt[5Ch](5). |
| `008FE140` | BSP_Entity_AsShipOrNull | reconstructed | `entity_as_kind06_or_null_008fe140`: null or vt[5Ch](6); called by the torpedo tick 008FFF20. |
| `008FEA40` | BSP_GunBot_SetFireTargetTurretClass | ledger-only | Hosted at the cc9 gunnery caller (game_hosts_gunnery.cpp, kTailGunnerAimErrorBound): `006DF170`, then +6Ch = -1.0f. |
| `008FF040` | BSP_GunBot_AttachBallisticClass | ledger-only | Hosted with cc9's labelled substitution (kFlakErrorRerollBound): the `008FDBE0` roll (shared RNG `00BD2F10`) happens at the first flak tick instead of at attach (008FF055). |

The three ClearFireTarget overrides send the gun's vt[1E8h](0); the cc9 host's clear does not (an
open item already listed in docs/SHIP_AI_OPEN_ITEMS.md for the torpedo twin). Its meaning was not
read here either, so it is a named host hook.

## Robot descriptors

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008FB770` | BSP_RobotConfig_ReadParametersBase | reconstructed | `robot_config_read_parameters_base_008fb770`: `RET 8`, reads nothing. |
| `008FB750` | BSP_RobotConfig_ConstructBase | ledger-only | src/robot_config.cpp `construct_descriptor`. |
| `008FC660` `008FC6B0` `008FC9F0` `008FCD40` `008FCF10` `008FD350` `008FD860` | Construct PilotBot, AAFlakBot, TailGunnerBot, AAGunnerBot, ArtillerySubDirectorBot, ArtilleryGunnerBot, DepthChargeBot | ledger-only | src/robot_config.cpp `construct_descriptor` with the same name strings and derived vtables. |
| `008FC570` `008FC680` `008FC9C0` `008FCD10` `008FCEE0` `008FD320` `008FD5F0` `008FDAC0` | Destruct base and the seven classes | ledger-only | src/robot_config.cpp `delete_robot_descriptor`. |

## Scene property records and the spawn reference frame

A scene property record: +04h type (0 int, 1 float, 2 string, 3 bool byte, 4 enum, 5 reference,
6 none, 7 vec3, 8 bytes, 9 float array, 0Ah int array, 0Bh vec3 array), +0Ch value, +10h/+14h the
vec3 tail, +18h/+1Ch the reference object / name, +20h/+24h the array block / byte size, +28h the
enum declaration. The existing `native_scene_property_*` files own storage; these are the readers.

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008EF7C0` | BSP_SpawnSpec_SetField0C_Provisional | reconstructed | `scene_property_set_field0c_008ef7c0`: +0Ch = value, returns the record. |
| `008EF820` | BSP_SceneProperty_ValueEquals | reconstructed | `scene_property_value_equals_008ef820`: per type through table 008EF9FC; floats pass only when ordered-equal (FUCOMIP, JP at 008EF877); references compare objects, else names through `00438E10` (008EF8BE); type 6 is never equal. |
| `008F0B00` | BSP_SceneEnumTable_FindNameByValue | reconstructed | `scene_enum_table_find_name_008f0b00`: 40h buckets at +8, chains via node+0Ch; a null name gives `00F89450`. |
| `008F0D80` | BSP_SceneEnumDecl_FindNameByValue | reconstructed | `scene_enum_decl_find_name_008f0d80`: the same walk through `008EEB10` (008F0D8E) / `008EEB60` (008F0DA9) on decl+4. |
| `008F0DF0` | BSP_SceneProperty_GetStringValue | reconstructed | `scene_property_string_value_008f0df0`: string, enum name through `008F0D80` (008F0E08), else the +1Ch name or "". 31 callers. |
| `008F0E20` | BSP_SceneProperty_GetValueText | reconstructed | `scene_property_value_text_008f0e20`: display text through `004B7F70` (008F0E7E) into the shared buffer `00F89458`; array counts from `008EF7F0` (008F0EED). |
| `008F84A0` | BSP_SpawnRefFrame_Assign | reconstructed | `spawn_ref_frame_assign_008f84a0`: entity +14h, `00694A60` (008F84B4), matrix through `004134F0` (008F84C0); the old entity is not unregistered. |
| `008F84D0` | BSP_SpawnRefFrame_ConstructFromMatrix | reconstructed | `spawn_ref_frame_construct_from_matrix_008f84d0`: zero fields, vtable `00D16AA4`, `004134F0` (008F84F2). |
| `008F8530` | BSP_SpawnRefFrame_ConstructFromEntity | reconstructed | `spawn_ref_frame_construct_from_entity_008f8530`: entity +14h with `00694A60` (008F8574), identity matrix through `004134F0` (008F85EC). |
| `008F8610` | BSP_SpawnRefFrame_CopyConstruct | reconstructed | `spawn_ref_frame_copy_construct_008f8610`: `00694A60` (008F8656), `004134F0` (008F8662). |
| `008F86B0` | BSP_SpawnRefFrame_FreezeToMatrix | reconstructed | `spawn_ref_frame_freeze_to_matrix_008f86b0`: `00414DB0` (008F86C6) when the pose is stale, matrix from entity+CCh, +14h = 0. |
| `008F8700` | BSP_SpawnRefFrame_OnObservedReleased | reconstructed | `spawn_ref_frame_on_observed_released_008f8700`: `008F86B0` (008F8709) for the frame's own entity. |
| `008EEA80` | BSP_PropertyHash64Iterator_Next | ledger-only | Byte-identical to `008EEB60`, implemented in src/native_scene_property_hash_iterator.cpp. |
| `008EF650` | BSP_PropertyHash64Iterator_BeginCopy | ledger-only | Byte-identical to `008EEB10`, same file. |

## Library rows (bound, name only)

Recorded `library_instantiation_out_of_scope`, callees read first (only STL instantiations,
allocators, the ref-pointer release, operator new, the key transfer `00438FE0` and the element
copy `00904250`): `008F8F30`, `008F8F90`, `008F8FF0`, `008F9060`, `008F9220`, `008F9260`,
`008F92B0`, `008F9300`, `008F9360`, `008F93C0`, `008F9420`, `008F9610`, `008F9680`, `008F96D0`,
`008F9750`, `008F9980`, `008FA2A0`, `008FA740`, `008FAA00`, `008FDF30`, `008FF970`, `009045C0`,
`0090C170`, and `00904250` (the placement copy of a 64h-byte list element: FLD/FSTP float copies
of +4h..+1Ch, REP MOVSD of +20h..+5Fh, +60h).

## World save/load, the segment query, and the alias resolvers

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `00903C60` | BSP_MatrixInterpolator_ReadArchive | reconstructed | `matrix_interpolator_read_archive_00903c60`: entity, translation, rotation, time, initialMatrix (kind 7), initialTime. Save/load. |
| `00903D80` | BSP_MatrixInterpolator_WriteArchive | reconstructed | `matrix_interpolator_write_archive_00903d80`: the writer twin. Save/load. |
| `00904F70` | BSP_World_ReadArchive | reconstructed | `world_read_archive_00904f70`: one list node per existing section {1, i} (archive slot +14h): `009045C0` (00904FDD), `00904E50` (00904FE7), `00903C60` (00905026); then "stationaries" through `0084F080` (00905066). Save/load. |
| `009042B0` | BSP_World_WriteArchive | reconstructed | `world_write_archive_009042b0`: `00903D80` per node (00904324), then `0084F1A0` (00904369). Save/load. |
| `009043A0` | BSP_SpatialIndex_QuerySegmentExcluding | reconstructed | `spatial_query_segment_excluding_009043a0`: `0042E630` (009043AA), entity vt[B0h]() as the exclusion, `0098ADD0` (009043D4); a null entity tail-jumps with exclusion 0. Live in HUD picking. |
| `009052E0` | BSP_ScoreRecord_AddField18 | reconstructed | `score_record_add_field18_009052e0`: record `slot*284h`, float +18h += delta, returns its address. |
| `00900C60` .. `00900D40` (eight) | BSP_RobotConfig_ResolveAlias* | ledger-only | src/robot_config.cpp assigns each alias with `find_robot_config_00900af0` in the loader. |
| `009009A0` | STL_Tree_Construct_009009a0 | library | std::_Tree constructor, bound. |

Not done in this pass: `008FAFC0` (BSP_CommonResourceCache_Preload, 1.1 KB of title-time resource
preloading with string temporaries and an SEH frame) and `008FF640`
(BSP_Entity_HullDistanceToPoint_Provisional, no callers).

## Scene property bag adders and small scene containers

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008F37E0` | BSP_ScenePropertyBag_AddVec3 | reconstructed | `scene_property_bag_add_vec3_008f37e0`: `00BF681B`(38h), type-7 record (vtable 00CE89D4), then `008F33F0` even for a null record. |
| `008F3880` | BSP_ScenePropertyBag_AddMatrix | reconstructed | `scene_property_bag_add_matrix_008f3880`: `008F3510`(bag, key, matrix, 10h, 1). |
| `008EEBD0` | BSP_DwordArray_ConstructCapacity | reconstructed | `dword_array_construct_capacity_008eebd0`: `00BF55BE`(capacity*4, saturated). |
| `008EEC10`, `008EEC30`, `008EF630`, `008EFC30` | BSP_OwnedBuffer_Free_* | reconstructed | `owned_buffer_free_*`: `00BF65AC` (008EEC10) or `00BF6989` (the others), then clear. |
| `008EEFD0` | BSP_SceneTokenizer_PeekFlag | reconstructed | `scene_tokenizer_peek_flag_008eefd0`: `008D8A70`, then byte +4. |
| `008EEFE0` | BSP_SceneProperty_WriteBoolText | reconstructed | `scene_property_write_bool_text_008eefe0`: "true"/"false" through `0094FF10`. |
| `008EF030` | BSP_ScenePropertyReader_Destruct | reconstructed | `scene_property_reader_destruct_008ef030`: vtable 00D162CC, `00950320`, vtable 00CE4DFC. |
| `008EF730` | BSP_ByteBuffer_Assign | reconstructed | `byte_buffer_assign_008ef730`: `00BF6989` old block, then `00BF55BE` + `00BF7680` copy or a borrowed pointer. |
| `008EFBC0` | BSP_ScenePropertyPtr_CompareOrder | reconstructed | `scene_property_ptr_compare_order_008efbc0`: ordinal +34h difference. |
| `008F0010` | BSP_PointerArray_PushBackGrow | reconstructed | `pointer_array_push_back_grow_008f0010`: grow by +0Ch with `00BF55BE`, `00BF7680`, `00BF6989`. |
| `008F0600` | BSP_OwnedPolymorphic_Reset | reconstructed | `owned_polymorphic_reset_008f0600`: vtable[0](1), then clear. |
| `008F1010` | BSP_PropertyHash64_ForEach | reconstructed | `property_hash64_for_each_008f1010`: `008EF650` / `008EEA80` walk with an empty body. |
| `008F02B0`, `008F02E0` | BSP_ScenePropertyDecl_Is*Letter | ledger-only | src/scene_property_bag.cpp `scene_property_type_for_letter` (lines 83..103) folds the "E" / "LUA_S" tests into the letter parse. |
| `008F1960` | BSP_SceneReference_GetTypeLetter | ledger-only | src/scene_property_bag.cpp:70 `scene_reference_kind_letter`. |

## Scene text writers, key renames and the random-thread scope

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008EFA30` | BSP_SceneText_WriteMultiline | reconstructed | `scene_text_write_multiline_008efa30`, __stdcall(text, writer): `00BF86F0` per piece, `0094FFA0` before later pieces, quote, `008EE5F0` escape, quote; the indent at writer+8 rises after the first piece. |
| `008F08E0` | BSP_ScenePropertyBag_ResolveReferences | reconstructed | `scene_property_bag_resolve_references_008f08e0`: `00480690` / `0047E480` walk; the callback stops being called after the first failure. |
| `008F18B0` | BSP_PropertyMap_RenameKey | reconstructed | `property_map_rename_key_008f18b0`: `00435C40`, `0043B8B0` twice, bucket unlink, `0043B760`, `00425F40`. |
| `008F2AA0`, `008F2AE0` | BSP_SceneNameNode_ScalarDeletingDtor(Copy) | reconstructed | Release the name, then `0043B0A0` to pool 00E17578 / 00E17540. |
| `008F2B20` | BSP_SceneSymbol_CreateFromName | reconstructed | `00BF681B`(8), `0041E870`, `0094F270`(list, header, 1). |
| `008F2CF0` | BSP_PropertyMap_RenameKeyCString | reconstructed | Two `0041E870` temporaries, `008F18B0`(owner+4, old, new), two releases. |
| `008F4030` | BSP_PropertyMap_UnlinkKey | reconstructed | `0043B8B0`, bucket unlink, `0043D2D0`, count -1. |
| `008F2540` | BSP_ScenePropertyBag_WriteText | reconstructed | `scene_property_bag_write_text_008f2540`: the "PropTree {" debug dump; every line goes to `004254B0`, a trivial body in this build. |
| `008F1CA0` | BSP_SceneProperty_WriteText | reconstructed | `scene_property_write_text_008f1ca0`: one "name = letter value ;" line; arrays wrap 16 / 4 / 4 / 2 per line through `00439AC0`. |
| `008F7CF0`, `008F7D80` | BSP_RandomThreadsScope_Construct / Release | reconstructed | `00BD2FE0` / `00BD3050` thread registration only; neither draws the shared RNG. |
| `008F7D20` | BSP_SingletonLifetime_DestroyGlobal01090AA0 | reconstructed | `00BD0400`, `_free`, clear [01090AA0]. |
| `008F67B0`, `008F6FC0`, `008F7100` | CPropTreeLibrary_Load and its folder / extension walkers | ledger-only | src/scene_traffic_groups.cpp:494 and :592. |
| `008F7DB0` | BSP_Startup_ResolveLanguage | ledger-only | src/winmain_startup.cpp (:93, :108) behind StartupHost::resolve_language. |
