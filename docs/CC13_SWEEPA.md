# CC13 sweep A: segments 0..14 leftovers (cc13-sweepA)

Packet cc13-sweepA (2026-10-16) closes out the functions of segments 0..14
(00401010..00527C80) that the earlier cc13 waves left without a reconstruction
record: 281 in the pool (`local/cc13_poolnames.py` filter). Source:
`src/cc13_sweepA.cpp`, header `include/bsp/cc13_sweepA.hpp`; nothing is wired.
Descriptive names are hypotheses, not recovered symbols. Status of every
reconstruction below: reconstructed and build-tested (MSVC Win32, compiled into
`bsp_core`); none is fixture-tested, ABI-compatible or game-validated.

## Method

- Each address was searched in `src/` and `include/` and in `docs/`. Almost every
  hit was a reference (a host hook, a table row or a comment), not a body; the
  three real implementations found are listed under "Existing".
- References were counted from the PE bytes, not from Ghidra: every E8/E9 and
  0F 8x rel32 landing on the address plus every absolute dword in any section
  (`local/sweepA_census.py`; positive control 00441210 = 27 CALL + 1 JMP). A
  single absolute dword was checked by hand where it decides the outcome
  (00432380 is an immediate, 0043FA00's is unaligned data, the others are
  vtable slots or code pointers pushed as arguments).
- Bodies were read from the listing; x87 register forms were taken from the bytes
  (`tools/x87_dc_form_scan.py`); constants were read from the image with their
  width. Every call the image makes happens at the same point through a named
  `SweepAHost` method (CRT memcpy/memset/vsprintf_s/fmod/atan2 are bound
  directly; STL instantiations the bodies call are host methods named by address).
- Common simplification, stated once: SEH unwind actions for a throwing callee are
  not modelled (no reconstructed body throws itself). Extended-precision x87
  intermediates are modelled in double where the comment says so; those can
  differ from the image in the last float bit through double rounding.

## Reconstructed

The source comment above each function gives the ABI, the callers, every call
and the uncertainty; the ledger record carries the same text.

<!-- recon-table -->
| Address | C++ function | Original ABI (first sentence of the source comment) |
|---|---|---|
| 00412D90 | `abs_float_00412d90` | ============================ segment 0 ==================================== 00412D90, __stdcall(float), RET 4, result in ST0: clears the sign bit of the argument's dword (AND 7FFFFFFFh) and loads it. |
| 00412E80 | `pointer_array_reverse_00412e80` | 00412E80, __thiscall(ECX = {void** data +0, int count +4}), RET: swaps the dword slots from both ends towards the middle (signed compare, the data pointer reloaded for each store). |
| 00412F60 | `int_clamp_00412f60` | 00412F60, __fastcall(ECX = int* value, EDX = int* low, stack int* high), RET 4: returns *low when *value < *low, else *high when *value > *high, else *value (signed). |
| 00412F80 | `format_string_100_00412f80` | 00412F80, __cdecl(char* dst, const char* format, ...): _vsprintf_s (00BF7289) with count 64h; EAX passes its result through. |
| 00412FA0 | `int_min_00412fa0` | 00412FA0, __fastcall(ECX = int* a, EDX = int* b): EAX = *a when *a < *b, else *b (signed). |
| 0042AA00 | `camera_set_angles_0042aa00` | ============================ segment 2 ==================================== 0042AA00, __thiscall(ECX = camera, float rho, float theta, char snap), RET 0Ch. |
| 0042C450 | `camera_clamp_zoom_0042c450` | 0042C450, __thiscall(ECX = mover, float low), RET 4. |
| 0042C4C0 | `camera_mover_set_shake_0042c4c0` | 0042C4C0, __thiscall(ECX = mover, six floats), RET 18h: stores them to +1D0h..+1E4h in argument order. |
| 0042CD00 | `camera_eye_offset_0042cd00` | 0042CD00, __thiscall(ECX = camera, float* out), RET 4, EAX = out. |
| 0042E3D0 | `camera_mover_frustum_contains_point_0042e3d0` | 0042E3D0, __thiscall(ECX = mover, const float* point), RET 4, AL. |
| 004313D0 | `entity_set_name_004313d0` | 004313D0, __thiscall(ECX = entity, const NativeString* name, unused), RET 8; vtable slot +0Ch of several entity classes (00CE3B34 and five more). |
| 00432380 | `four_vector_record_construct_00432380` | 00432380, __thiscall(ECX = record), EAX = record. |
| 00435410 | `camera_mover_step_base_00435410` | 00435410, __thiscall(ECX = mover, float dt), RET 4; vtable slot (00CE3C04): forwards to 004329D0(mover, dt). |
| 00435960 | `party_rows_reserve_00435960` | 00435960, __thiscall(ECX = {3-byte rows* +0, int count +4, int capacity +8}, int capacity), RET 4. |
| 004359E0 | `party_rows_resize_004359e0` | 004359E0, __thiscall(ECX = rows, int count), RET 4. |
| 00435F40 | `lua_global_table_write_archive_00435f40` | 00435F40, __thiscall(ECX = owner), RET; reached by the tail JMP at 00436487 in 00436460. |
| 00438A00 | `profile_manager_request_save_checkpoint_00438a00` | 00438A00, __thiscall(ECX = profile manager, const LuaObject* source), RET 4. |
| 00438F90 | `cstring_key_construct_owned_00438f90` | 00438F90, __thiscall(ECX = key, const char* text), RET 4, EAX = key: key+0 = operator new(strlen(text) + 1) (00BF55BE) holding a copy of the text (copied up to and including the terminator, no null check of the block), key+4 byte = 1 (owned). |
| 00439820 | `vector3_transform_project_point_00439820` | 00439820, __fastcall(ECX = float3* out, EDX = const float3* point, matrix, float* inverse_w), RET 8, EAX = out. |
| 00439A00 | `vector2_set_random_unit_direction_00439a00` | 00439A00, __thiscall(ECX = float2* out), EAX = out. |
| 00439AC0 | `string_format_static_00439ac0` | 00439AC0, __cdecl(format, ...), EAX = 00E177E8: _vsprintf_s (00BF7289) into the 400h-byte static buffer, whose address is returned whatever the result. |
| 00439F50 | `camera_unproject_screen_ray_00439f50` | 00439F50, __fastcall(ECX = camera, EDX = float3* out_near, float sx, float sy, float3* out_far), RET 0Ch, AL. |
| 0043A290 | `camera_pick_world_point_on_horizontal_plane_0043a290` | 0043A290, __fastcall(ECX = camera, EDX = float3* out, float sx, float sy, float plane_y, float3* ray), RET 10h, AL; caller 006438E8 (HUD markers). |
| 0043A870 | `camera_project_world_to_screen_xy_0043a870` | 0043A870, __fastcall(ECX = const float3* world, EDX = float2* out, int mode, char clamp_depth), RET 8: 0043A660 into a float3 temporary, then x and y are copied out; EAX is 0043A660's result passed through (callers compare it to 7). |
| 0043B9C0 | `surface_probe_kind6_above_water_0043b9c0` | 0043B9C0, __fastcall(ECX = const float3* point), AL; caller 0049BB80 in 0049B1F0, the splash path (docs/CC13_LEFT8C.md). |
| 0043BFC0 | `pooled_string_scalar_delete_0043bfc0` | 0043BFC0, __thiscall(ECX = pooled string {length +0, data +4}, flags), RET 4, EAX = string: releases the data, and with flags bit 0 returns the header to the 00E175B0 node pool (0043B0A0). |
| 0043CFA0 | `pooled_string_ptr_delete_reset_0043cfa0` | 0043CFA0, __thiscall(ECX = PooledString** slot): a non-null string has its data released and its header returned to the 00E175B0 pool, then the slot is cleared (only in that case). |
| 0043D2D0 | `property_map_node_destroy_0043d2d0` | 0043D2D0, __thiscall(ECX = map, unused; node), RET 4. |
| 0043D4D0 | `property_map_erase_0043d4d0` | 0043D4D0, __thiscall(ECX = map {+4 count, +8 bucket heads}, const PooledString* key), RET 4, AL. |
| 0043D870 | `property_map_erase_by_name_0043d870` | 0043D870, __thiscall(ECX = owner, const char* name), RET 4, AL: a stack pooled string is built by 0041E870, 0043D4D0(owner+4, &key) erases, the key's data is released (no header return: it lives on the stack) and the erase result is returned. |
| 0043DF40 | `observer_set_construct_0043df40` | 0043DF40 / 0043DF90 / 0043DFF0 / 0043E060, __thiscall(ECX = set, one to four dwords), RET 4/8/0Ch/10h, EAX = set. |
| 0043DF90 | `observer_set_construct_0043df90` | 0043DF40 / 0043DF90 / 0043DFF0 / 0043E060, __thiscall(ECX = set, one to four dwords), RET 4/8/0Ch/10h, EAX = set. |
| 0043DFF0 | `observer_set_construct_0043dff0` | 0043DF40 / 0043DF90 / 0043DFF0 / 0043E060, __thiscall(ECX = set, one to four dwords), RET 4/8/0Ch/10h, EAX = set. |
| 0043E060 | `observer_set_construct_0043e060` | 0043DF40 / 0043DF90 / 0043DFF0 / 0043E060, __thiscall(ECX = set, one to four dwords), RET 4/8/0Ch/10h, EAX = set. |
| 0043FF40 | `gun_animation_sync_to_clock_0043ff40` | ============================ segment 3 ==================================== 0043FF40, __thiscall(ECX = gun, unused), RET 4; vtable slot (00CFDD34) and the call at 0072B33F in 0072B2D0. |
| 004407A0 | `vehicle_class_underwater_armour_default_004407a0` | 004407A0, __thiscall(ECX = class), ST0 = float +4Ch; held by 26 vtable slots (00CE44A4 .. |
| 00440CA0 | `device_class_slots_destroy_00440ca0` | 00440CA0 / 004411F0 (identical bodies), __thiscall(ECX = slot array): 00440180(array, 0), then _free(data). |
| 004411F0 | `device_class_slots_destroy_004411f0` | 00440CA0 / 004411F0 (identical bodies), __thiscall(ECX = slot array): 00440180(array, 0), then _free(data). |
| 00442170 | `device_class_registry_clear_00442170` | 00442170, no arguments: 00440180(00441780() + 4, 0). |
| 004427A0 | `gun_record_construct_004427a0` | 004427A0, __thiscall(ECX = 48h-byte gun record), EAX = record. |
| 00442840 | `gun_record_copy_construct_00442840` | 00442840, __thiscall(ECX = destination 48h-byte gun record, const record* source), RET 4, EAX = destination; only caller 00442940. |
| 00442940 | `gun_records_reserve_00442940` | 00442940, __thiscall(ECX = {48h records* +0, count +4, capacity +8}, int capacity), RET 4. |
| 00442A30 | `gun_records_resize_00442a30` | 00442A30, __thiscall(ECX = records, int count), RET 4. |
| 00442AE0 | `device_apply_scaled_damage_00442ae0` | 00442AE0, __thiscall(ECX = device, float amount), RET 4; vtable slot (00CFDE04) and the call at 00728091 in 00728000. |
| 00442B70 | `gun_records_destroy_00442b70` | 00442B70, __thiscall(ECX = records): 00442A30(records, 0), then _free(data). |
| 00442C00 | `gun_class_base20_is_kind_of_00442c00` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00442C70 | `bomb_platform_class_is_kind_of_00442c70` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00442D00 | `multiple_bomb_platform_class_is_kind_of_00442d00` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00442DB0 | `rfs_gun_class_is_kind_of_00442db0` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00442E50 | `rt_gun_class_is_kind_of_00442e50` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00442EE0 | `st_gun_class_is_kind_of_00442ee0` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00442FA0 | `depth_charge_launcher_class_is_kind_of_00442fa0` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00443010 | `catapult_class_is_kind_of_00443010` | 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class, unused; int id), RET 4, EAX = 1 when id is the class's own id or an ancestor's, else 0. |
| 00440E10 | `device_entity_set_lua_class_00440e10` | 00440E10, __thiscall(ECX = entity); vtable slot (00CFDCFC) and one call. |
| 004426C0 | `device_entity_place_in_world_lua_004426c0` | 004426C0, __thiscall(ECX = entity); vtable slot (00CFDCF4) and one call. |
| 00443490 | `device_class_resolve_and_activate_00443490` | 00443490, __fastcall(ECX = class id, DL = retain flag), EAX = class; about fifty callers. |
| 0044A3C0 | `archive_serialize_picture_field_0044a3c0` | ============================ segment 4 ==================================== 0044A3C0, __thiscall(ECX = field, archive), RET 4: vtable +10h of the archive with ({0, "picture" 00CE4994}, {1, field}) by value. |
| 0044E670 | `lua_reader_copy_construct_0044e670` | 0044E670, __thiscall(ECX = reader, const reader* source), RET 4, EAX = reader: vptr 00CE44FC, then the LuaObject vector at +4 copy-constructed from source+4 by 0044DA70. |
| 004514A0 | `panel_owner_kill_dialog_004514a0` | 004514A0, __thiscall(ECX = panel owner, PooledString name by value), RET 8; caller 008B0A5F (KillDialog). |
| 004518B0 | `panel_owner_reset_004518b0` | 004518B0, __thiscall(ECX = panel owner); caller 0045F6A3 in 0045F600. |
| 00451A90 | `panel_owner_start_dialog_from_lua_00451a90` | 00451A90, __thiscall(ECX = panel owner, PooledString name by value, LuaObject table by value), RET 1Ch; callers 008B0688 (StartDialog) and 00451F7B in 00451E50 (the session message). |
| 00451E50 | `session_message_start_dialog_apply_00451e50` | 00451E50, __thiscall(ECX = panel owner, message), RET 4; handler E1h in the session message table (multiplayer_session_cc13.cpp). |
| 004567B0 | `native_string_at4_destroy_storage_004567b0` | ============================ segment 5 ==================================== 004567B0 / 00456D10 (identical bodies), __thiscall(ECX = record): the NativeString at +4 (length +4, data +8) returns its data to the sized pool when set; nothing is cleared. |
| 00456D10 | `native_string_at4_destroy_storage_00456d10` | ============================ segment 5 ==================================== 004567B0 / 00456D10 (identical bodies), __thiscall(ECX = record): the NativeString at +4 (length +4, data +8) returns its data to the sized pool when set; nothing is cleared. |
| 0045CFC0 | `scene_record_eight_strings_destroy_0045cfc0` | 0045CFC0, __fastcall(ECX = record); 75 calls. |
| 0045C440 | `scene_token_equals_0045c440` | 0045C440, ECX = tokenizer passed through untouched to 008D8A70, stack (const char* text), RET 4, EAX = 1 when 00438E10(peeked token, text) is 0, else 0 (NEG/SBB/ADD). |
| 004643A0 | `fire_target_holder_apply_004643a0` | 004643A0, __thiscall(ECX = holder, entity), RET 4; vtable slot (00CE5500). |
| 004654D0 | `entity_command_set_submarine_depth_004654d0` | 004654D0, __thiscall(ECX = command, entity), RET 4; vtable slot (00CE5530). |
| 004684F0 | `unit_reference_construct_004684f0` | ============================ segment 6 ==================================== 004684F0, __thiscall(ECX = reference, unit), RET 4, EAX = reference. |
| 00470440 | `gameplay_modifiers_powerup_multiplier_00470440` | 00470440, __thiscall(ECX = modifiers, int index, unit), RET 8, ST0. |
| 004704B0 | `hit_record_destroy_004704b0` | 004704B0, __fastcall(ECX = hit record): _free(+3Ch) when non-null; the pointer is not cleared. |
| 004704E0 | `hit_record_owning_unit_004704e0` | 004704E0, __fastcall(ECX = hit record), EAX: 0 when +4 is null or its vtable +108h() answers null, else that object's dword +ACh. |
| 0047B7F0 | `random_in_range_0047b7f0` | 0047B7F0, __thiscall(ECX = {.., lo +38h, hi +3Ch}), ST0: 00BD2F10(stream 0, lo, hi). |
| 0047B880 | `unit_uses_small_turn_roll_limit_0047b880` | 0047B880, __thiscall(ECX = unit), AL: 1 unless vtable +5Ch answers true for kind 10h or (asked only when 10h is false) kind 16h. |
| 0047B9F0 | `blob40_greater_by_float8_0047b9f0` | 0047B9F0, __cdecl(40h-byte a by value, 40h-byte b by value), EAX: 1 when the float at a+8 is above the one at b+8 (FCOMIP/JBE: NaN gives 0). |
| 0048EE90 | `traffic_path_set_destruct_0048ee90` | ============================ segment 7 ==================================== 0048EE90, __thiscall(ECX = set); called by the deleting destructors 0048EF50 and 0048F870, by 004A02E0 and two unwind funclets. |
| 004BFED0 | `unit_is_viewed_by_player_004bfed0` | ============================ segment 9 ==================================== 004BFED0, __fastcall(ECX = unit), AL; caller 007BB326 in 007BB310 (cockpit transform). |
| 004CA760 | `world_replace_border_bounds_004ca760` | 004CA760, __thiscall(ECX = world, const float3* lo, const float3* hi), RET 8; caller 0089C2E8 (ReplaceSceneBorder). |
| 004C9800 | `scene_record_player_slot_construct_004c9800` | 004C9800, __thiscall(ECX = record), EAX = record: `eh vector constructor iterator' (00BF7CD1) over the five 38h-byte slots at +8 with constructor 004C6750 and destructor 004C6760. |
| 004C9820 | `scene_record_player_slot_destroy_004c9820` | 004C9820, __thiscall(ECX = record): `eh vector destructor iterator' (00BF7C6E) over the same five slots with 004C6760. |
| 004CAF50 | `effect_handle_vector_push_back_004caf50` | 004CAF50, __thiscall(ECX = {counted T** data, count, capacity}, T* const* handle), RET 4; one call. |
| 004CB160 | `effect_handle_vector_resize_004cb160` | 004CB160, __thiscall(ECX = counted pointer vector, int count), RET 4; six calls. |
| 004CC3F0 | `end_of_mission_movie_construct_004cc3f0` | 004CC3F0, __thiscall(ECX = record, const NativeString* name), RET 4, EAX = record; caller 004CD405 in 004CD390. |
| 004D7D50 | `game_show_game_closed_004d7d50` | ============================ segment 10 =================================== 004D7D50, __thiscall(ECX = game, byte suppress_prompt), RET 4; caller 0076D122 in 0076D0E0 (session game-closed message). |
| 004E79B0 | `movie_cam_lookat_unregister_004e79b0` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004E9670 | `landing_point_register_004e9670` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004E9690 | `landing_point_unregister_004e9690` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004F1400 | `landscape_register_004f1400` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004F1420 | `landscape_unregister_004f1420` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004F1910 | `spawn_point_register_004f1910` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004F1940 | `spawn_point_unregister_004f1940` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004F25D0 | `land_convoy_register_004f25d0` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004F2600 | `land_convoy_unregister_004f2600` | World-list registration overrides, __fastcall(ECX = entity), RET; each is a vtable slot only (no direct call). |
| 004F1390 | `landscape_get_collision_node_004f1390` | 004F1390, __fastcall(ECX = landscape), EAX: landscape+1E4h, or null for a null landscape (the base-to-member pointer adjustment); vtable slot 00CEA140. |
| 004F9D30 | `color_static_white_004f9d30` | 004F9D30, no arguments, EAX = 00E18D50: the function-local static colour {1, 1, 1, 1} (1.0f from 00D7A24C) guarded by bit 0 of 00E18D60, set on the first call. |
| 004E9520 | `landing_point_construct_body_004e9520` | 004E9520, __thiscall(ECX = entity, word id), RET 4, EAX = entity; caller 004E9D40. |
| 004E9D40 | `landing_point_create_from_scene_004e9d40` | 004E9D40, __fastcall(ECX unused, EDX = const char* name; a, b, two unused dwords), RET 10h, EAX = entity; the LandingPoint (1Dh) scene creator held at .text 004F2940 by the factory table 004F2800: size 224h, body 004E9520. |
| 004F2700 | `land_convoy_create_from_scene_004f2700` | 004F2700, __fastcall(ECX unused, EDX = const char* name; a, b, property bag, unused), RET 10h, EAX = entity; the LandConvoy (1Ah) creator held at .text 004F29E8. |
| 004EA650 | `path_create_from_scene_004ea650` | 004EA650, __fastcall(ECX unused, EDX = const char* name; a, const 40h-byte pose* b, two unused), RET 10h, EAX = entity; the Path (47h) creator held at .text 004F2A3C. |
| 004E9F80 | `water_mine_create_from_scene_004e9f80` | 004E9F80, __fastcall(ECX unused, EDX = const char* name; unused a, const pose* b, property bag, unused), RET 10h, EAX = entity; the WaterMine (34h) creator held at .text 004F29B0. |
| 004F1460 | `landscape_create_from_scene_004f1460` | 004F1460, the Landscape (44h) creator, held at .text 004F29CC; the 004E9D40 shape with size 430h and body 004F11C0. |
| 004F11C0 | `landscape_construct_004f11c0` | 004F11C0, __thiscall(ECX = entity, word id), RET 4, EAX = entity; caller 004F1460. |
| 004F1A60 | `spawn_point_create_from_scene_004f1a60` | 004F1A60, the SpawnPoint (4Dh) creator, held at .text 004F2A90; the 004E9D40 shape with size 348h and body 004F1610. |
| 004F1610 | `spawn_point_construct_body_004f1610` | 004F1610, __thiscall(ECX = entity, id), RET 4, EAX = entity; caller 004F1ACE in 004F1A60. |
| 004F19A0 | `spawn_point_destruct_004f19a0` | 004F19A0, __thiscall(ECX = entity); caller the deleting destructor 004F1A40. |
| 004F2410 | `land_convoy_construct_004f2410` | 004F2410, __thiscall(ECX = entity, id), RET 4, EAX = entity; callers 004F274B (LandConvoy creator) and 00743BBE (session message). |
| 004F2660 | `land_convoy_destruct_004f2660` | 004F2660, __thiscall(ECX = entity); caller the deleting destructor 004F2AE0. |
| 004F04C0 | `scene_unit_place_or_defer_004f04c0` | 004F04C0, __fastcall(ECX = entity, EDX = property bag; a, b), RET 8; callers 004F0C88 (Stationary), 004F0D72 (Wreck), 007011DB (dummy target). |
| 004F0BE0 | `stationary_create_from_scene_004f0be0` | 004F0BE0, __fastcall(ECX unused, EDX = const char* name; a, b, property bag, unused), RET 10h, EAX = entity or 0; the Stationary (36h) creator, held at .text 004F295C by the factory table. |
| 004F0CC0 | `wreck_create_from_scene_004f0cc0` | 004F0CC0, the Wreck (3Bh) creator, held at .text 004F2972; the same body as 004F0BE0 with the key "WreckType" (00CEA038) and the class step class = 004B3F10(&name, 0), entity = 0047BB70([[00E188A8]+21D0h], class). |
| 004F2FD0 | `circle2f_contains_point_004f2fd0` | 004F2FD0, __thiscall(ECX = circle {x, y, r}, const float2* point), RET 4, EAX; caller 004F4AEA in 004F49F0. |
| 00518F30 | `hud_weapon_group_state_is_engaged_00518f30` | ============================ segments 12..14 ============================== 00518F30, __thiscall(ECX = weapon group state), EAX: 1 when +30h == 1 or byte +34h or byte +35h is set, else 0. |
| 00519020 | `hud_pilot_screen_pulse_attached_kind25_00519020` | 00519020, __thiscall(ECX = pilot screen); callers 00519070 and 00519CFE. |
| 0051E730 | `hud_view_bind_unit_0051e730` | 0051E730, __thiscall(ECX = view, unit), RET 4; callers 00519ACF, 00521617, 0064DCAE, 0067C3CC. |
| 00519B00 | `hud_bomber_view_bind_unit_00519b00` | 00519B00, __thiscall(ECX = bomber view, unit), RET 4; caller 0068AFF5. |
| 0051C3A0 | `front_end_screen_reset_descendant_colors_0051c3a0` | 0051C3A0, ECX unused, (root element, byte flag), RET 8; one call (0051C59F). |
| 00521370 | `direction_to_pitch_yaw_00521370` | 00521370, __fastcall(ECX = const float3* direction, EDX = float* pitch, float* yaw), RET 4; 16 calls. |
| 00521F10 | `unit_pick_row_value_00521f10` | 00521F10, __thiscall(ECX = rows, int index), RET 4, EAX: with e = rows + 4 + index*34h, returns e+8 when byte e+10h is set, else e+4. |
| 00521FD0 | `unit_pick_read_position_00521fd0` | 00521FD0, __thiscall(ECX = entry, float3* out), RET 4, AL = byte +FCh; out = +F0h..+F8h. |
| 00522680 | `unit_pick_attackmove_color_00522680` | 00522680, ECX unused, (entity, arg), RET 8, EAX: 00E08F78 when the byte [[00E188A8]+19C4h] is set, the entity answers vtable +5Ch(6) and vtable +16Ch("attackmove" 00CECCA8, arg) is true; otherwise 00E08EF8. |
| 00522860 | `unit_distance_record_set_00522860` | 00522860, __thiscall(ECX = record, unit), RET 4, EAX = record; callers 00526004, 0052642B, 0052653F. |
| 005251C0 | `hud_screen_reset_selection_005251c0` | 005251C0, __thiscall(ECX = screen); callers 00647059 and 0064A0DE. |
| 00527C80 | `control_layout_screen_set_payload_00527c80` | 00527C80, __thiscall(ECX = screen, four bytes), RET 10h: +28h..+2Bh = the bytes in order, +2Ch = 1. |
<!-- /recon-table -->

## Existing implementations, recorded ledger-only

| Address | Implementation | Outcome |
|---|---|---|
| 00436530 | `counted_pointer_array_reserve_0041e490` (src/cc13_left61_seg1.cpp) | ICF twin of 0041E490: the two 84-instruction listings differ only in branch targets and the SEH handler. Verified against 00436530's listing. |
| 0045F440 | `bsp::call_entry_point_if_defined` (src/mission_lua_host.cpp), threadsafe arm | MINOR deviation, adoption note below. |
| 0045F520 | same, forced arm (00887B30 instead of 00887E50; the only difference in the listing) | MINOR deviation, adoption note below. |

Adoption note (gameplay host file, not edited): 0045F440/0045F520 test the global
with 00B66200 `LuaObject_IsFunction` (lua_type == 6, 0045F48D). The host's
`GameMissionLuaHost::global_is_defined` (src/game_hosts_lua.cpp 9573..9579) tests
`!lua_isnil`, so a non-function, non-nil global (for example `luaStageInit = 5`)
is dispatched by the host and its pcall fails, where the image skips the call.
LIVE on every mission load (luaPrecacheUnits, luaStageInitMulti, luaStageInit,
luaEngineMovieInit); no RNG involvement.

## Unreferenced, recorded ledger-only

No rel32 reference and no absolute dword anywhere in the image (or reached only
from such a function); not reconstructed:

<!-- unref-list -->
- 00413D10, 00413E70, 00413F20, 00418C50, 00418CD0, 00418D60, 00418DE0, 004199D0: TMPL_ COMDAT container instantiations, rel32/abs census 0 (CC13_LEFT61 already lists them); vendor-template shape, name only.
- 0042E460: unreferenced (census 0), 154 instructions, DC-form FMUL; not reconstructed.
- 0042EF90: unreferenced (census 0), 75 instructions, DC-form FMUL; not reconstructed.
- 00439AF0: unreferenced, 104 instructions; not reconstructed.
- 00439DB0: reached only from 0043B220 (unreferenced); x87 quadratic; not reconstructed.
- 0043B220: unreferenced; not reconstructed.
- 0043A0F0: unreferenced, 121 instructions; not reconstructed.
- 0043AD80: unreferenced, 79 instructions; not reconstructed.
- 0043B450: reached only from 0043E4D0 (unreferenced); its 004254B0 sink is a trivial body; not reconstructed.
- 0043E4D0: unreferenced, 143 instructions; not reconstructed.
- 0043DD80: unreferenced, 158 instructions; not reconstructed.
- 0044BD30: unreferenced, 101 instructions.
- 00452130: unreferenced, 174 instructions.
- 0045BBE0: unreferenced (census 0), 84 instructions; not reconstructed.
- 0045D110: unreferenced (census 0), 49 instructions; not reconstructed.
- 0048EAB0: unreferenced, 305 instructions.
- 0048EFD0: unreferenced, 156 instructions.
- 004CA860: unreferenced, 72 instructions.
- 004CA930: unreferenced, 78 instructions.
- 004D62C0: unreferenced, 91 instructions.
- 004DC020: unreferenced, 104 instructions.
- 004F3090: unreferenced, 97 instructions.
- 004F3360: unreferenced, 129 instructions (DC-form FMUL).
<!-- /unref-list -->

The template COMDATs 00413D10, 00413E70, 00413F20, 00418C50, 00418CD0, 00418D60,
00418DE0, 004199D0 (segment 1) are also unreferenced; they are template
container instantiations and get no record (docs/CC13_LEFT61.md lists them too).

## Out of scope (vendor or compiler-generated), name only

- STL instantiations: 0042BF10; 00440600 (`_Uninitialized_fill_n<LuaObject>`);
  00441210 (`vector<LuaObject>::_Tidy`); 00443C30, 00443C40, 00443E00, 00443E10,
  0044A180, 0044A190, 0044AA20, 0044AA30 (`_Tree` iterator ++/--); 00445AE0,
  0044AB00, 0044AC00, 0044B170, 0044B1D0, 0044B210, 0044B250, 0044BA90, 0044BAC0,
  0044BAF0, 0044EB00, 0044F310, 004506B0, 00450D40, 004515B0, 00452630 (std::map
  _Buynode / constructor / clear / subtree helpers of the panel maps); 004D0640
  (std::set<NativeString>::insert); 004D0860 (std::operator+ of two std::strings:
  copy a, append b, return; the ledger name SplitCommandToken is wrong); and the
  TMPL_ algorithm/vector helpers 00501A50, 00509390, 0050DA40, 0051B9E0, 0051BC10,
  0051BCB0, 00521D40, 00522CF0, 00522D60, 00523740, 005237C0, 00523810, 00523B70,
  00524000, 005240E0, 00524690, 00524770, 005247D0, 00524820, 00524A90, 00524E90,
  005253F0, 00525720.
- 0043FA00: an allocator `construct` (placement copy of a LuaObject); its one
  absolute dword (00CE397D) is unaligned data, so it is also unreferenced.
- EH catch funclets: 00444252, 004CE75C.

## cc9 lane (skipped, per the lead)

00428420, 00428800 (cc9 gunnery), 004A9BD0 (cc9 plane death modes).

## Skipped (not attempted in this packet)

<!-- skipped-list -->
- 0043ABA0 BSP_HudDigitIcon_RollUv (144 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0043D8F0 BSP_LuaTable_CopyIntoPropertyBag (129 instructions; references r3a0): not attempted in this sweep (context budget; left for a later packet).
- 00440AC0 FUN_00440ac0 (107 instructions; references r1a1[.rdata:00ce4488]): not attempted in this sweep (context budget; left for a later packet).
- 00440F50 BSP_DeviceEntity_OnKilledLua (169 instructions; references r1a1[.rdata:00cfdcd4]): not attempted in this sweep (context budget; left for a later packet).
- 004423A0 BSP_DeviceClassRegistry_ClearLuaClassTables (140 instructions; references r2a0): not attempted in this sweep (context budget; left for a later packet).
- 00443090 BSP_DeviceClass_ResolveFromLua (289 instructions; references r3a0): not attempted in this sweep (context budget; left for a later packet).
- 00447510 BSP_GameDynamicsList_Add (377 instructions; references r3a0): not attempted in this sweep (context budget; left for a later packet).
- 0044B5B0 BSP_PanelSequence_DescribeProperties (329 instructions; references r2a0): not attempted in this sweep (context budget; left for a later packet).
- 004507D0 BSP_PanelSequence_ReadProperties (434 instructions; references r3a0): not attempted in this sweep (context budget; left for a later packet).
- 00456DD0 BSP_MovieTrack_BindEntities (263 instructions; references r1a1[.rdata:00ce4dbc]): not attempted in this sweep (context budget; left for a later packet).
- 00459020 BSP_MovieEntityTrack_Update (588 instructions; references r1a1[.rdata:00ce4dc4]): not attempted in this sweep (context budget; left for a later packet).
- 0045A210 FUN_0045a210 (106 instructions; references r1a1[.text:0070c07c]): not attempted in this sweep (context budget; left for a later packet).
- 0045D230 FUN_0045d230 (173 instructions; references r4a0): not attempted in this sweep (context budget; left for a later packet).
- 0045D480 BSP_MovieActionType_Construct (292 instructions; references r32a0): not attempted in this sweep (context budget; left for a later packet).
- 0045D930 FUN_0045d930 (177 instructions; references r5a0): not attempted in this sweep (context budget; left for a later packet).
- 0045DE50 BSP_MovieAction_Create (524 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0045ECA0 FUN_0045eca0 (229 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0045F110 FUN_0045f110 (207 instructions; references r5a0): not attempted in this sweep (context budget; left for a later packet).
- 0045F600 BSP_EngineMovie_Stop (233 instructions; references r4a0): not attempted in this sweep (context budget; left for a later packet).
- 004602C0 BSP_EngineMovie_ParseFile (367 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 00460750 FUN_00460750 (272 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0046B730 FUN_0046b730 (272 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 00487470 FUN_00487470 (240 instructions; references r4a0): not attempted in this sweep (context budget; left for a later packet).
- 00487A70 FUN_00487a70 (260 instructions; references r0a1[.rdata:00ce6854]): not attempted in this sweep (context budget; left for a later packet).
- 0048C6A0 FUN_0048c6a0 (156 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0048F670 NamedClassBase_ReadLua_0048F670 (166 instructions; references r5a4[.rdata:00ce5cb8]): not attempted in this sweep (context budget; left for a later packet).
- 004A0D20 FUN_004a0d20 (2199 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004C3A80 FUN_004c3a80 (159 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004C74C0 BSP_World_BorderZoneAtPoint (204 instructions; references r4a0): not attempted in this sweep (context budget; left for a later packet).
- 004C7AA0 FUN_004c7aa0 (231 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004CCE50 BSP_Game_BeginEngineMovie (218 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004D2BB0 BSP_Game_DestroyWorld (348 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004D32A0 BSP_Game_NilScriptAddedGlobals (235 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004D4720 BSP_Scene_BuildClassPreloadAliases (83 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004D7BA0 BSP_Game_ShowSessionLostMessage (133 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004D80D0 BSP_Game_UpdateMultiplayerInterface (359 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004D8620 BSP_Frontend_UpdateActiveScreens (118 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004DB920 BSP_Game_UpdateDeviceWaitScreen (107 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004DD7C0 BSP_Game_CompleteLeaderboardRead (193 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004E1A50 BSP_AwardTracker_DestroyMembers (192 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004E2200 BSP_Game_RunConsoleCommandQueue (378 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004E3AA0 BSP_Game_OnInit (387 instructions; references r2a0): not attempted in this sweep (context budget; left for a later packet).
- 004E5540 BSP_Game_BeginStartupSequence (202 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004E8BF0 BSP_SceneHeader_Assign (531 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004EA080 BSP_SceneClass_ReadStockLists (415 instructions; references r0a1[.text:004f2a8b]): not attempted in this sweep (context budget; left for a later packet).
- 004EE300 FUN_004ee300 (105 instructions; references r0a1[.text:004f29e3]): not attempted in this sweep (context budget; left for a later packet).
- 004F01F0 FUN_004f01f0 (62 instructions; references r7a0): not attempted in this sweep (context budget; left for a later packet).
- 004F1B60 FUN_004f1b60 (168 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004F3DA0 BSP_Triangle2f_ContainsPoint (113 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 004F4880 BSP_Segment2f_ClipAgainstLine (116 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0050B440 FUN_0050b440 (241 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0050B730 FUN_0050b730 (141 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 0051CDE0 BSP_BriefingScreen_Enter (1269 instructions; references r0a1[.rdata:00cec960]): not attempted in this sweep (context budget; left for a later packet).
- 0051EAA0 BSP_HudBinoculars_StepInputAxes (200 instructions; references r4a0): not attempted in this sweep (context budget; left for a later packet).
- 0051FA50 BSP_CheatMenuScreen_BuildLines (1679 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 005213D0 BSP_HudCommandBuildingView_BindUnitCamera (161 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 005220C0 FUN_005220c0 (257 instructions; references r7a0): not attempted in this sweep (context budget; left for a later packet).
- 005224C0 FUN_005224c0 (106 instructions; references r2a0): not attempted in this sweep (context budget; left for a later packet).
- 005226D0 FUN_005226d0 (93 instructions; references r2a0): not attempted in this sweep (context budget; left for a later packet).
- 00522F10 FUN_00522f10 (89 instructions; references r2a0): not attempted in this sweep (context budget; left for a later packet).
- 00523020 FUN_00523020 (98 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 00523130 FUN_00523130 (133 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 005241D0 BSP_HudUnitPickScreen_IssueCommandToTarget (166 instructions; references r3a0): not attempted in this sweep (context budget; left for a later packet).
- 00525250 FUN_00525250 (72 instructions; references r1a1[.text:0059db6c]): not attempted in this sweep (context budget; left for a later packet).
- 00525F80 FUN_00525f80 (159 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 00526180 FUN_00526180 (274 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
- 00526720 FUN_00526720 (234 instructions; references r1a0): not attempted in this sweep (context budget; left for a later packet).
<!-- /skipped-list -->
