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

cc9 already covers this machinery. docs/GUN_BOT_TICKS.md (:83, :124-140, and the coverage table near :476) reads 008FBC80, 008FBDC0, 008FBEC0 and 008FEAC0 in full. include/bsp/gun_bot_ticks.hpp carries kGunBotAttachAddress, kAAGunnerBotOffGun (5Ch), kTailGunnerBotOffGun (68h), kEntityKindUnit (5) and the 008FEAC0 property layout (~:100). The bodies here reuse those constants.

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

The three ClearFireTarget overrides send the gun's vt[1E8h](0). That slot is the gun's trigger, and (0) releases it (docs/GUNNERY_OPEN_ITEMS.md ~10906 and ~14140). The cc9 host's clear does not send it; docs/SHIP_AI_OPEN_ITEMS.md lists that only for TorpedoBot and DepthChargeBot. The release is a named host hook here.

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
| `009043A0` | BSP_SpatialIndex_QuerySegmentExcluding | ledger-only | Hosted by cc9 as `ray_pick_009043a0` (src/game_hosts_hud.cpp:3244, a labelled substitution under kHudPickSegmentQueryBound; the exclusion-by-unit runs through query_segment_units_impl). docs/HUD_PICK_SEGMENT_QUERY.md section 1 reads the whole body. vt[B0h] takes no stack arguments, and the call is `0098ADD0`(from, to, excl, record, kind) (009043D4). The cc13 body was dropped. |
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
| `008F7CF0`, `008F7D80` | BSP_RandomThreadsScope_Construct / Release | reconstructed | `00BD2FE0` / `00BD3050` thread registration only; neither draws the shared RNG. 00BD2FE0, under the [01090AC0] critical section (when non-null), stores GetCurrentThreadId in the first free slot of the ten at 01090AC4 and raises the count at 01090AEC to slot+1 when that is larger; 00BD2ED0 then routes that thread's draws (00BD2F10, 00BD2F40, 00BD2FC0) to the per-slot generator at 01090AF0 + (stream + 2*slot)*9C8h instead of the shared 00E14748. Neither function seeds or draws; 00BD3050 clears the slot and, when it was the last one, trims the count (00BD308B..00BD30B4 lower [01090AEC] past trailing empty slots). 00BD2FE0 ignores ECX, so the scope argument is unused. |
| `008F7D20` | BSP_SingletonLifetime_DestroyGlobal01090AA0 | reconstructed | `00BD0400`, `_free`, clear [01090AA0]. |
| `008F67B0`, `008F6FC0`, `008F7100` | CPropTreeLibrary_Load and its folder / extension walkers | ledger-only | src/scene_traffic_groups.cpp:494 and :592. |
| `008F7DB0` | BSP_Startup_ResolveLanguage | ledger-only | src/winmain_startup.cpp (:93, :108) behind StartupHost::resolve_language. |

## Scoring band: small bodies and library rows

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `00903790`, `009037B0` | BSP_OwnedBuffer_Free_* | reconstructed | `_free` and clear. |
| `00905300` | BSP_SlotCounter_Increment | reconstructed | `score_record_increment_field170_00905300`: ++[scoring + slot*284h + 170h]. Called by the formation bindings 0077C8D0 / 0077C980 / 0077CA60. |
| `00906430` | BSP_MissionResult_CallNamedScript | reconstructed | `mission_result_call_named_script_00906430`: a non-empty name at +147Ch goes to `00887E50` (00906453) on the mission Lua runner. Its caller 00920A20 (the debrief bring-up) is still a stub in src/game_hosts_mission_result.cpp, so the host makes no such call. |
| `0090D2F0`, `0090E920` | BSP_LuaObjectHolder_Destruct / Construct | reconstructed | Vtable 00D18784 with a LuaObject vector at +4 (`00441210` tidy; `00442220` push_back, then `00B67700` on the by-value argument). |
| `009221E0`, `00922210` | BSP_WatchList_Construct / Destruct | reconstructed | Vtable 00D18C24, list at +4 (`004C3020` sentinel; `004D05E0` clear and `_free`). |
| `00922240` | BSP_WatchList_Add | reconstructed | `005FE8B0` removes equal names, then a `004CE6F0` node at the back, `004CE780`, and the by-value name is released. |

The following 45 rows are bound as library instantiations (std::sort over float, the std::_Tree
constructors, erases and finds, and the vector copy and insert helpers). Their callees are only
STL code, allocators, `_free`, `_memmove_s` and xlen throws: `009063C0`, `009076B0`, `00907750`,
`009077E0`, `00907DF0`, `00907E30`, `00907EB0`, `00907F40`, `00907F80`, `00908400`, `00908460`,
`009084A0`, `00908560`, `009085D0`, `0090C090`, `0090C1E0`, `0090C210`, `0090C240`, `0090C270`,
`0090C2E0`, `0090C310`, `0090C340`, `0090C370`, `0090C3A0`, `0090C430`, `0090D220`, `0090D9D0`,
`0090DB00`, `0090E600`, `0090F2C0`, `0090F2F0`, `0090F320`, `0090F350`, `0090F4A0`, `0090F560`,
`009102D0`, `00910370`, `00910ED0`, `00911470`, `00911DF0`, `0091CC40`, `0091CE30`, `00920DE0`,
`008FE420` (the robot registry lower_bound) and `008F6BA0` (a std::set<std::string> insert).

## Mission score record clear

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `00915760` | BSP_MissionScoreRecord_Clear | reconstructed | `mission_score_record_clear_00915760`: 25 scalar zeroes, 36 inline tree clears, two 16-bit vector erases (`00BF67A7`) and three string resets (`0041DD40`, 00915E9D). (An earlier include/bsp/scoring_bodies.hpp note said the clear leaves slot+08h and slot+250h alone; cc9 has since corrected it.) The binary zeroes both (record+08h at 0091576C, dword record+250h at 009157C5; saved at 008BC6A3/008BC6A7 and restored at 008BC6C5/008BC6D5, record = mgr+slot*284h+4). |

## Scoring events: component failure and repair

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `00913D80` | BSP_MissionScoring_RecordComponentFailure | reconstructed | `scoring_record_component_failure_00913d80`: the Counter_RUA_DBU counter (`005070C0`), the RUA_DBU award (`0050FC30`, `0090EDE0` at 00913F9E), the trace through `004254B0`, and the unit id push (`007265E0`, 009140DB) when the block's c*a + d*b exceeds [unit+370h]. Reached from the component-failure roll at 0093C0DC in normal missions. cc9's host only records the call. It touches only scoring state and the record's +21h flag, and draws no shared RNG. |
| `00914100` | BSP_MissionScoring_RecordRepairCompleted | reconstructed | `scoring_record_repair_completed_00914100`: GA_PL threshold against `00923BE0` health * 100 (0091418D), then {id, clock [00F876A4]} pushed through `0090E7C0` (00914250). cc9's host marks the 0093CAF1 jump unreached. |

## Scoring refresh and scene bag collectors

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `009150A0` | BSP_MissionScoring_ForceRefresh | reconstructed | `scoring_force_refresh_009150a0`: `00914390` (009150CC) and `0090EA00` (009150D3) on the record, then Lua `scoring.playerIndex = slot` (`00B67460`, 0091513A) when game+1FE4h != 0. |
| `008F3270` | BSP_ScenePropertyBag_CollectKeyNames | reconstructed | `scene_property_bag_collect_key_names_008f3270`: every key copied into a `0094F270` list (008F3324), sorted by `0094F1B0` (008F3348) when asked. The old name "WriteAll" was wrong. |
| `008F6E20` | BSP_ScenePropertyBag_CollectRecordsSorted | reconstructed | `scene_property_bag_collect_records_sorted_008f6e20`: the records gathered in a local vector, `008F3C90` sort by ordinal (008F6F08), copied out with `008F0010` (008F6F44). |

## Scene bag subtract, dotted path and text writers

None of these six is reached: apart from their own recursion and 008F71E0's use by 008F7550, the band scan found no direct call and no absolute-dword reference (`docs/CC13_BAND_0080_008F.md`, Unreferenced).

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008F4290` | BSP_ScenePropertyBag_SubtractEqual | reconstructed | `scene_property_bag_subtract_equal_008f4290`: a heap iterator over other, a key lookup in bag (`0043B8B0`), and removal (`004F01F0`, 008F43D9) when the values are equal (`008EF820`) or a shared sub-bag ends up empty after the recursion. The old name was "CloneIterator". |
| `008F4420` | BSP_ScenePropertyBag_SubtractSameType | reconstructed | `scene_property_bag_subtract_same_type_008f4420`: the same walk keyed on type alone. The image never calls `0047E480` in the loop (008F454E..008F4556), so a non-empty other spins on its first key. That is reproduced, not fixed. |
| `008F1A20` | BSP_SceneObject_BuildDottedPath | reconstructed | `scene_object_build_dotted_path_008f1a20`: out = the names along the [+30h]/[+110h] owner chain, joined with "." (CE3A70) by two `004261A0` concatenations per step, outermost name first. The result is "" when the object has no owner. The old name was "FormatValue". |
| `008F71E0` | BSP_ScenePropertyBag_WriteTextBlock | reconstructed | `scene_property_bag_write_text_block_008f71e0`: `" {"`, then the entries sorted through an inline `map<std::string, record*>` (`008F6BA0`), `"}"`. Sub-bags are written as `"key"` plus a nested block; other entries go through `008F1CA0`. |
| `008F7550` | BSP_ScenePropertyBag_WriteTextDiff | reconstructed | `scene_property_bag_write_text_diff_008f7550`: the 008F71E0 block with every entry equal to base's (`008F2260` + `008EF820`) left out. Shared sub-bags recurse, and a null base writes the full block. The old name was "WriteTextEntry". |
| `008F79A0` | BSP_ScenePropertyBag_RemoveMissing | reconstructed | `scene_property_bag_remove_missing_008f79a0`: collects the names that base lacks into a `vector<std::string>` (`00849170`), then erases them from other (`0043D870`) or from bag (`008F4030`). The heap iterator built for other is leaked, as in the image. The old name was "WriteFiltered". |

## Common resource precache and hull distance

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008FAFC0` | BSP_CommonResourceCache_Preload | reconstructed | `common_resource_cache_preload_008fafc0`: runs once, under an "IngamePrecache" file block (`00BE0A30`/`00BDCB30`). It fills 54h textures (`[00F8D394]`->vt[64h]), 9 sounds (`00A83FD0`) and 3Ch models (`007188A0`) from the tables at 00E0C988 / 00E0CAD8 / 00E0CB00, and reports `0057C1A0` progress at step / 153.0 after each item. It is live at title start: cc9's `game_entry.hpp` names the call at 004E5758 as `notify_title_ready`, but no body is hosted. |
| `008FF640` | BSP_Entity_HullDistanceToPoint_Provisional | reconstructed | `entity_hull_distance_to_point_008ff640`: for a null or non-ship entity, the XZ distance (zero below 1e-10). For a ship, q is moved into hull space (`00B63D50` inverse built lazily, `004142E0`) and tested against an outline from hull+0A0h / +0A4h before `00419AB0`. The operand order comes from the DC/DE bytes. The model uses 24-bit precision; see the uncertainty in the source. Unreferenced. |

## Scoring publish, objective-list message and nation sum

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `0090EA00` | BSP_MissionScoreRecord_PublishToLua | reconstructed | `score_record_publish_to_lua_0090ea00`: Lua `scoring = nil`, then `scoring = {}` (`00B67350`, `00B67580`), then the record is written into the table by cc9's `00908A30` through a `0090E920` holder (0090EB37). The holder's destructor is inlined. |
| `009105F0` | BSP_ObjectiveListsMessage_Apply | reconstructed | `objective_lists_message_apply_009105f0`: a network message from `00777850` with eight cases (jump table 009108C8): append a text to one of six string lists, drain six counts of flags and ints into the 00E196FC.. containers, or clear everything / call `00626C50`. The counts in the message are left at -1, as in the image. |
| `0090CDD0` | BSP_StringIntMap_SumNationMissions | reconstructed | `string_int_map_sum_nation_missions_0090cdd0`: sums the owner+0Ch map over "USN01".."USN14" or "IJN01".."IJN14" (`004BF370` with the `_stricmp` match), and returns 0 when a key is missing. Unreferenced. |

## XLive stats sum and Lua text printers

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `00920690` | BSP_ScoreMap_SumCampaignBestScores | reconstructed | `score_map_sum_campaign_best_scores_00920690`: *b = the sum, over the current campaign's missions (the USN/IJN prefix of game+2198h, mission list 00E19434 + 10h * USN), of the best of [+27Ch], [+278h] and [+280h] in each `00594A70` record with a non-zero +0Ch. *a stays 0. cc9 hosts the call from `00A43AF0` as `stats_lookup_00920690`, but no body. |
| `00920F00` | BSP_LuaTable_FormatSummary | reconstructed | `lua_table_format_summary_00920f00`: `{k=v,...}` with nested tables as `{...}`, and `...` once the limit is used up. The by-value table argument is destroyed inside. |
| `00921360` | BSP_LuaWatchList_FormatValues | reconstructed | `lua_watch_list_format_values_00921360`: a "name: value" line for each watched dotted path (`00B68D70`): invalid, a table summary (`00920F00`, limit 3), a light userdata (`ptr->` handle + vt[10h] name), or `0043C1C0` text. The text is released without being used. Unreferenced. |

## Score sync message

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `0090ABB0` | BSP_MissionScoreRecord_SendSync | reconstructed | `score_record_send_sync_0090abb0`: `__thiscall(ECX = record)`, RET, the sending half of `0091C650`. Its caller `0076D1B0` runs it for the eight 284h rows; `multiplayer_session_cc13.cpp:879` hosts that loop through `scoring_send_row_0090abb0` without a body. It returns unless `[game+1FE4h] == 1` and byte `[game+218Ch] == 0`. One stack message (`0075B430` type 15h, vtable `00D035B8`, +4 = 1) is refilled and sent through `0076A9F0(game+1EF0h; message, 0)` per item and is never destroyed: the EH state stays 0 at the exit, so only an exception unwind destroys it. Kind 0 sends the header (slot at +1Ch and +78h, three floats copied as bits, 22 ints). Kinds 2..8, 13h, 9 and 0Ah send the string-keyed maps at +18h..+60h, +6Ch, +1ECh and +1F8h (key text inline-copied to +7Ch, `00F899A0` for a null key, value to +188h). Kind 0Fh sends the int -> float map at +114h. Kinds 0Bh, 0Dh, 0Eh and 0Ch send the three-level trees at +B4h, +C0h, +A8h and +9Ch. Kind 10h sends the two-level tree at +CCh and kind 11h the int map at +D8h. Each inlined `00BF6713` iterator test stays at its point. There is no x87 arithmetic and no RNG. |
| `0091C650` | BSP_ScoreSyncMessage_Apply | reconstructed | `score_sync_message_apply_0091c650`: one network message (from `00777850` and `0076D220`) updates the mission score record of a slot. It has 24 kinds (jump table 0091CBD8): the header copy, the text-keyed counters (`005070C0`), an award (`0090EDE0`), the nested int-keyed maps (`0062C170`/`0062BB60`/`00625900`, `00915610`/`00911890`/`00625900`, `0090E740`), and the three message setters (`0090BDA0`/`0090BE30`/`0090BEC0`). |

## Hit damage recorder

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `00915F20` | BSP_MissionScoring_RecordHitDamage | reconstructed | `scoring_record_hit_damage_00915f20`: `__thiscall(ECX = scoring, target, hit)`, RET 8, called from `0077CE60` (unit damage attribution) and `00819A20` (kamikaze direct arm). 00819A20's direct arm is recorded, not delivered, by game_hosts_gunnery.cpp:11973 (record "KamikazeDetonate::direct_hit_00915f20"; CC13_VERIFY3 D2); cc9's 0077CE60 attribution host (kKillCreditDamageGateBound) does not call it. Returns early on game+1FE4h == 2, on the target's flag bytes +5Ch/+5Dh/+60h/+5Eh, or on a null source `[hit+4]`. A source with no `vt[108h]` record that is kind 6 adds `[hit+48h]` to the slot block's `+A0h` damage tree (`00915610`/`00911890`/`00625900`, keyed by source class, `00803510` relation, target class). Otherwise, once per record (byte +20h), with slot <= 7: the RUA_KU award (`vt[204h]` against the award's +44h vector [0], then the `+70h` award map lookup with `004B93C0` ==), a byte clear at `[[[00E198C4]+B8h]+14h]`, and for enemy targets the BW_AAM/BW_AM/BW_TM/BW_DCM/BW_RM counters (switch tables 00916970/00916958) in `+124h`, the RUA_LBU award over the `+1C0h` kill list (age below the vector [1] threshold), and the torpedo-hit count `entry+24h`. Every call then adds the `_ftol2` sum of `[hit+4Ch]`, `[hit+48h]`, `[hit+50h]` and byte `[hit+2Ch]` to the `+A0h` or `+ACh` tree keyed by `[record+14h]`, bumps `+174h` for ship-on-ship hits, and creates or updates the `+1C0h` kill-list entry (`0090C090` find, `0057E8A0` ==, one `0090E850` call per field). 24-bit x87 modelled; 53-bit noted. |

## Badge and achievement counters

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `00911E80` | BSP_MissionScoreRecord_EvaluateBadgeCounters (cc9 name, kept) | reconstructed | `score_record_evaluate_badge_counters_00911e80`: `__fastcall(ECX = record)`, RET, called from `00914EF0`. It empties the counter map at record+78h (`0058B520`, then resets the head), then rebuilds it. Under key 1 of record+B4h (`00908190`), the leaves of each inner node add their ints to the BU_* token picked by the inner key (jump table 00912A20: 1 BU_DM, 2 BU_PTM, 3 BU_BM, 4 BU_CM, 7 BU_VLB, 8 BU_VDB, 9 BU_VTB, 0Ah BU_FA, 0Bh BU_VR, 0Dh BU_SM). It copies record+120h by key, counts the record+15Ch floats above `[[game+21A0h]+1470h]` as BO_CV, and sets BO_DCM += [+168h], BO_KSS / BO_KSP = the largest value of +1A4h / +198h (from 0), BO_SC = [+174h], BO_EE = [+190h]. Each counter whose award vector [0] (`0050FC30`, +44h) is at most its count and which an inlined find (`004BF370`, `00BF7FBF`) misses in the awards map record+6Ch is granted through `0090EDE0([record+250h], key, 1, 0)` at 009129EE. Every inline checked-iterator test keeps its `00BF6713` call. |
| `00912A60` | BSP_MissionScoreRecord_EvaluateAchievementCounters (cc9 name, kept) | reconstructed | `score_record_evaluate_achievement_counters_00912a60`: `__fastcall(ECX = record)`, RET, called from `00914EF0` and `00920A20`. It empties the counter map at record+84h. It reads the thresholds RUA_CU [0], RUA_FU [0] - 1 and RUA_FU [1] from the award table, and builds two local trees: A (`00907D40` head; id -> kill times) and B (`005828E0` head; id -> int). One walk of the record+1BCh kill list, skipping entries with byte +2Bh set, sets RUA_CU, RUA_SU, GA_SH, GA_DB and GA_AO and fills A (`00910F10`/`0045AFD0`) and B (`00667EB0`). GA_PL is granted when an alive registry entity of the record+14Ch {id, time} vector is older than 5.0 (00D7A370) and the award is missing from the slot's awards map; dead entries are erased in place and the walk restarts, and a grant erases the whole vector (`0090CA30`). Each A list longer than RUA_FU [0] - 1 is sorted (`0090D220`), and any window of RUA_FU [0] kills within RUA_FU [1] sets RUA_FU. B values at or above RUA_TBU [0] set RUA_TBU. The last walk grants counters at threshold that are not yet in record+6Ch through `0090EDE0(slot, key, count, 0)` at 009139C4; the value is the count, not 1. The local trees are then erased (`0058B000`/`0090F3D0`) and their heads freed. 24-bit x87 modelled; 53-bit noted. |

## Score record merge

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `0091A080` | BSP_MissionScoreRecord_MergeInto | reconstructed | `score_record_merge_0091a080`: `__thiscall(ECX = destination, source)`, `RET 4`, no SEH frame. The only caller is `009205E0`, which merges the mission's score record into the progress map's record. cc9's `commit_mission_record_009205e0` (mission_result.cpp, ~188) models the call as `stored = record` and keeps the play count; the image accumulates. With `i = src[+8]`, the pair `+26Ch[i]` / `+278h[i]` takes `src[+0]` / `src[+1E0h]` when both stored values are lower. When `src[+0] != 0`, +4h keeps the max, +194h is ORed and +18Ch added. +0 is added, +8 copied, and the floats +0Ch, +10h and +14h become src + dst. The string-keyed int maps are added through `005070C0`: +18h, +24h, +54h, +60h, +1ECh and +1F8h each into its own map, while the +3Ch, +30h and +48h sources all go into dst+24h (the ECX at `0091A257`, `0091A2D7` and `0091A357`). The three-level trees +B4h and +C0h (`0062C170`/`0062BB60`/`00625900`) and +9Ch and +A8h (`00915610`/`00911890`/`00625900`), the +114h float map (`0090E740`), the +D8h int map, and the two-level +CCh tree (`00911890`/`00625900`) are added. The ints +1C8h..+1E8h are added. The six `00625F50` maps +204h..+240h add node+1Ch into `[result+8]`. The ints +178h..+188h are added last. Each inlined `00BF6713` test stays at its point. 24-bit x87 is modelled for the four float adds and 53-bit is noted. There is no RNG. |

## Campaign awards (partial)

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `0091D780` | BSP_MissionScoreRecord_EvaluateCampaignAwards | partial: 0091D780..0091E697 | `campaign_awards_finish_part_0091d780` covers the frame set-up and the campaign-finish awards (0091D780..0091E15E), and `campaign_awards_mode_part_0091e15f` covers CA_IC and CA_VR (0091E15F..0091E697). 0091E698..0091FFFF is not reconstructed yet: the Counter_CA_* passes, RANK and the teardown. No cc9 host models these awards; award_grant.cpp only maps the CA_* names to ids. CA_IC and CA_VR each take a mask from the award's second value (`[+48h][1]`) and add it once for every progress record whose +1A8h bits cover it. The current mission counts with bit `004BCA50()` added. The award is granted, with value = the sum, when the sum equals its first value. ABI `__fastcall(ECX = record)`, RET, SEH handler `00CA65F0`. The only caller is `00920A20`. The sub-range works on `CampaignAwardsFrame0091d780`, the image's frame bytes at the image's offsets. It builds "USN", "USN0", "IJN" and "IJN0", and tests the mission key (game+2198h) for a "USN" or "IJN" prefix (`00469840`, `00435C40`); otherwise it goes on at 0091E15F. On a campaign mission it reads the first threshold of CA_OF, CA_CJM, CA_SA and CA_CUM (`0050FC30`). It then walks USN01..USN14 and IJN01..IJN14 (`004263B0` ids, `00594A70` progress records, the "Campaign Finish %s %d%d%d" trace through `004254B0`). The current mission (`0043E9A0`, or the inline prefix walk for IJN) checks the record's difficulty and score. Any other mission must have been played and, for CA_CUM/CA_CJM, completed at the threshold's difficulty slot (+26Ch[i]). Each surviving flag grants its award through `0090EDE0(slot, key, 1, 0)`. No x87 and no RNG. |
