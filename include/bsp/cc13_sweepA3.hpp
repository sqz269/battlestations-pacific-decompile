#pragma once
// Packet cc13-sweepA3: the third pass over the segment 0..14 leftovers (the
// bodies cc13-sweepA2 listed under "left over"), smallest first.
// docs/CC13_SWEEPA3.md lists every function with its outcome, ABI and
// uncertainty, and the ones skipped with the reason.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a SweepA3Host method named
// for its target; SweepA3Host extends SweepA2Host (and so SweepAHost), whose
// bindings and reconstructions are reused directly. Descriptive names are
// hypotheses, not recovered symbols. Nothing here is wired.

#include "bsp/cc13_sweepA2.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_sweepA3 {

using cc13_sweepA::LuaObjectStorage;
using cc13_sweepA::Raw;

// The 8-byte tagged value the property sinks take by value: tag 0 string
// pointer, 1 integer, 2 float bits, 3 a byte (the image leaves the upper three
// bytes of a tag-3 value as whatever the stack held).
struct TaggedValue {
    std::uint32_t tag;
    std::uint32_t value;
};

struct SweepA3Host : cc13_sweepA2::SweepA2Host {
    // --- property sink (the describe visitors) ------------------------------
    // [[sink]+4](sink, TaggedValue key by value), RET 8: open a group.
    virtual void vcall_sink_begin_4(void* sink, TaggedValue key) = 0;
    // [[sink]+8](sink): close the group.
    virtual void vcall_sink_end_8(void* sink) = 0;
    // [[sink]+0Ch](sink, TaggedValue key, TaggedValue value), RET 10h.
    virtual void vcall_sink_write_c(void* sink, TaggedValue key, TaggedValue value) = 0;
    // [[object]+0Ch](object, sink), RET 4: the element describes itself.
    virtual void vcall_describe_c(void* object, void* sink) = 0;
    // 00BD5680 BSP_TaggedValue_Equals, __thiscall(ECX = value, const
    // TaggedValue* other), RET 4, AL.
    virtual bool tagged_value_equals_00bd5680(const TaggedValue* value, const TaggedValue* other) = 0;

    // --- strings -----------------------------------------------------------
    // 0041DD20 BSP_NativeString_DestroyStorage, __thiscall(ECX = 8-byte
    // string {length +0, data +4}).
    virtual void native_string_destroy_storage_0041dd20(Raw string) = 0;
    // 004C5E60 BSP_WideString_ConstructFromCString, __thiscall(ECX = 8-byte
    // wide string, const char*), RET 4.
    virtual void wide_string_construct_from_cstring_004c5e60(Raw string, const char* text) = 0;
    // 004C5E20 BSP_WideString_Assign, __thiscall(ECX = wide string, const
    // wide string*), RET 4.
    virtual void wide_string_assign_004c5e20(Raw destination, Raw source) = 0;

    // --- leaderboards (game +7038h overlapped, +7054h results) -------------
    // 00A4D42E XGetOverlappedResult(overlapped, DWORD* result, BOOL wait),
    // XLive thunk; EAX = 0 on success.
    virtual std::uint32_t x_get_overlapped_result_00a4d42e(void* overlapped, std::uint32_t* result,
                                                           std::int32_t wait) = 0;
    // 00A4D422 XGetOverlappedExtendedError(overlapped), XLive thunk; EAX = HRESULT.
    virtual std::int32_t x_get_overlapped_extended_error_00a4d422(void* overlapped) = 0;
    // 004DCA80 BSP_Leaderboard_ProcessReadResults, __thiscall(ECX = row
    // vector (10h bytes), count), RET 4.
    virtual void leaderboard_process_read_results_004dca80(Raw rows, std::uint32_t count) = 0;
    // 004BCD20 STL instantiation, __thiscall(ECX = row vector, index), RET 4,
    // EAX = the row (vendor body).
    virtual Raw leaderboard_row_at_004bcd20(Raw rows, std::uint32_t index) = 0;
    // 00564F60 BSP_LeaderboardsScreen_OnReadComplete, __thiscall(ECX =
    // screen, const row vector*), RET 4 (reconstructed in
    // frontend_seg12_17_cc13.cpp against its own host).
    virtual void leaderboards_on_read_complete_00564f60(void* screen, Raw rows) = 0;
    // 004D9CA0 STL_Vector_TidyDestroy, __thiscall(ECX = row vector) (vendor body).
    virtual void stl_vector_tidy_destroy_004d9ca0(Raw rows) = 0;

    // --- input -------------------------------------------------------------
    // 00A92090 BSP_InputAction_HasDeviceQuery1C, __thiscall(ECX = action), AL.
    virtual bool input_action_has_device_query_00a92090(void* action) = 0;
    // 004C43C0 BSP_InputAction_WasPressedThisFrame, __thiscall(ECX =
    // [00E188A8], action id), RET 4, AL.
    virtual bool input_action_was_pressed_004c43c0(void* game, std::int32_t action) = 0;
    // 0042AC60 BSP_Math_StepTowards, __thiscall(ECX = float* state, float
    // target, float max_step), RET 8.
    virtual void math_step_towards_0042ac60(float* state, float target, float max_step) = 0;
    // [[object]+34h](object, int), RET 4: on the binoculars model [+20h].
    virtual void vcall_set_visible_34(void* object, std::int32_t value) = 0;
    // [[object]+0](object), plain RET: slot 0 called without arguments.
    virtual void vcall_slot0_void(void* object) = 0;

    // --- strings (8-byte {length +0, data +4}) -----------------------------
    // 00467CF0 BSP_NativeString_ReverseFindHeader, __thiscall(ECX = string,
    // const string* needle, position), RET 8, EAX = index.
    virtual std::uint32_t native_string_rfind_00467cf0(Raw string, Raw needle, std::uint32_t position) = 0;
    // 00469840 BSP_NativeString_Substring, __thiscall(ECX = string, string*
    // result, start, count), RET 0Ch, EAX = result.
    virtual Raw native_string_substring_00469840(Raw string, Raw result, std::uint32_t start,
                                                 std::uint32_t count) = 0;

    // --- engine movie (game +38h) -------------------------------------------
    // 00460B80 BSP_EngineMovie_Construct, __thiscall(ECX = 74h block), EAX = block.
    virtual void* engine_movie_construct_00460b80(void* block) = 0;
    // 004602C0 BSP_EngineMovie_ParseFile, __thiscall(ECX = movie, const
    // string* path), RET 4.
    virtual void engine_movie_parse_file_004602c0(void* movie, Raw path) = 0;
    // 0045DB40 BSP_EngineMovie_Start, __thiscall(ECX = movie).
    virtual void engine_movie_start_0045db40(void* movie) = 0;
    // 00B72030 BSP_Scene_BuildQuadTreesForType, __thiscall(ECX = [game+19ECh]).
    virtual void scene_build_quad_trees_00b72030(void* scene) = 0;
    // 004CC460 BSP_FrontEndManager_PushInterfaceRequest, __thiscall(ECX =
    // [00E198C4], interface id, argument), RET 8.
    virtual void frontend_push_interface_request_004cc460(void* manager, std::int32_t id,
                                                          std::int32_t argument) = 0;

    // --- scene tokenizer ------------------------------------------------------
    // 008D9930 BSP_SceneTokenizer_ExpectToken, __thiscall(ECX = tokenizer,
    // const char* token), RET 4, AL (unused by these callers).
    virtual bool scene_tokenizer_expect_008d9930(void* tokenizer, const char* token) = 0;
    // 008D9980 BSP_SceneTokenizer_ReadToken, __thiscall(ECX = tokenizer), EAX = text.
    virtual const char* scene_tokenizer_read_008d9980(void* tokenizer) = 0;
    // 008D9AD0 BSP_SceneTokenizer_ReadInt, __thiscall(ECX = tokenizer, bool*
    // ok), RET 4, EAX.
    virtual std::int32_t scene_tokenizer_read_int_008d9ad0(void* tokenizer, std::uint8_t* ok) = 0;
    // 008D9B40 BSP_SceneTokenizer_ReadFloat, __thiscall(ECX = tokenizer, bool*
    // ok), RET 4, ST0.
    virtual float scene_tokenizer_read_float_008d9b40(void* tokenizer, std::uint8_t* ok) = 0;
    // 0045B420, __thiscall(ECX = owner; float3 position, float time, float3
    // in tangent, float3 out tangent, int type, float alpha, NativeString
    // comment by value (released by the callee)), RET 38h: adds one spline
    // point (tagged stl_probable in the index; not reconstructed).
    virtual void movie_spline_add_point_0045b420(void* owner, const float* position, float time,
                                                 const float* in_tangent, const float* out_tangent,
                                                 std::int32_t type, float alpha, Raw comment) = 0;

    // --- engine movie stop (0045F600) callees ----------------------------------
    // 00A77760 BSP_StreamedDialogManager_SetAllPaused, __thiscall(ECX =
    // [00F8BBCC], int paused), RET 4.
    virtual void streamed_dialog_set_all_paused_00a77760(void* manager, std::int32_t paused) = 0;
    // 00A7A440 BSP_SoundManager_ApplyGlobalLevel, __thiscall(ECX =
    // [00F8BBD8], float level), RET 4.
    virtual void sound_manager_apply_global_level_00a7a440(void* manager, float level) = 0;
    // [[object]+4](object, float), RET 4: on [00F8BBCC].
    virtual void vcall_set_float_4(void* object, float value) = 0;
    // 00735B50 BSP_SoundFrame_ServiceInline, __thiscall(ECX = [00E1AE90]).
    virtual void sound_frame_service_inline_00735b50(void* sound_frame) = 0;
    // 0046B730, __thiscall(ECX = [00E18680], the scene database): the
    // script-entity kill pass (cc9 calls it from its engine-movie model).
    virtual void scene_database_kill_pass_0046b730(void* database) = 0;
    // 0045F520 BSP_Game_CallLuaEntryPointForced and 0045F440
    // BSP_Game_CallLuaEntryPointThreadSafe, __thiscall(ECX = [00E188A8],
    // const char* name, int), RET 8.
    virtual void game_call_lua_entry_point_forced_0045f520(void* game, const char* name, std::int32_t argument) = 0;
    virtual void game_call_lua_entry_point_thread_safe_0045f440(void* game, const char* name,
                                                                std::int32_t argument) = 0;
    // The Lua class-table resets 0045F600 calls in a row, no arguments (ECX
    // not set up by the caller).
    virtual void vehicle_class_registry_clear_lua_class_tables_0095ca70() = 0;
    virtual void bullet_classes_reload_from_lua_006e9c50() = 0;
    virtual void stationary_reset_lua_class_table_0084eb60() = 0;
    virtual void wreck_class_clear_lua_global_004b1890() = 0;
    virtual void soldier_class_clear_lua_global_004af930() = 0;
    virtual void buoy_class_reset_lua_table_00474200() = 0;
    virtual void cloud_class_reset_lua_table_00476410() = 0;
    virtual void trivial_body_004708a0() = 0;
    virtual void plane_part_class_clear_lua_global_004a7120() = 0;
    virtual void marker_classes_reset_lua_table_006d74b0() = 0;
    virtual void effects_clear_lua_table_00869d20() = 0;
    // 004C3CB0 BSP_Game_BuildLocalPlayerUnitLists, __thiscall(ECX = game).
    virtual void game_build_local_player_unit_lists_004c3cb0(void* game) = 0;
    // 00B66200 BSP_LuaObject_IsFunction, __thiscall(ECX = object), AL.
    virtual bool lua_object_is_function_00b66200(LuaObjectStorage* object) = 0;
    // 004CD7F0 BSP_Scene_DeriveShortName, __fastcall(ECX = result string,
    // EDX = const source string*, int), RET 4, EAX = result.
    virtual Raw scene_derive_short_name_004cd7f0(Raw result, Raw source, std::int32_t mode) = 0;
    // 004261A0 BSP_NativeString_Concat, __thiscall(ECX = left, string*
    // result, const string* right), RET 8, EAX = result.
    virtual Raw native_string_concat_004261a0(Raw left, Raw result, Raw right) = 0;
    // 00BE0A30 BSP_FileBlock_Construct, __thiscall(ECX = block, const
    // string* path, int), RET 8 (EDX not read); 00BDCB30
    // BSP_FileBlock_Destroy, __thiscall(ECX = block).
    virtual void file_block_construct_00be0a30(void* block, Raw path, std::int32_t mode) = 0;
    virtual void file_block_destroy_00bdcb30(void* block) = 0;
    // 004B46B0 BSP_AllocatorList_TrimAll, __thiscall(ECX = [00E188A8]).
    virtual void allocator_list_trim_all_004b46b0(void* game) = 0;

    // --- unit pick screen target cycle (00526720) ------------------------------
    // 0071C060 BSP_Unit_DirectorFlagsAllow, __fastcall(ECX = int mode, EDX =
    // entity), AL.
    virtual bool unit_director_flags_allow_0071c060(std::int32_t mode, void* entity) = 0;
    // 00526180, __thiscall(ECX = screen): refreshes the cycle list in place
    // (not reconstructed: see docs/CC13_SWEEPA3.md).
    virtual void unit_pick_refresh_cycle_list_00526180(Raw screen) = 0;
    // [[unit]+10h](unit), EAX = its display name.
    virtual const char* vcall_unit_name_10(void* unit) = 0;
    // 004254B0, __cdecl(const char* format, ...): an empty debug print body
    // (TRIV_body); three integer/pointer arguments here.
    virtual void debug_print_004254b0(const char* format, std::uint32_t a1, std::uint32_t a2,
                                      std::uint32_t a3) = 0;

    // --- scripted-name set (game +1930h) -----------------------------------------
    // 004C7F10 BSP_NativeStringSet_Find, __thiscall(ECX = set, iterator*
    // result {container, node}, const string* key), RET 8, EAX = result.
    virtual Raw native_string_set_find_004c7f10(Raw set, Raw result, Raw key) = 0;
    // 004CEC60 BSP_NativeStringSet_EraseSubtree, __thiscall(ECX = set, node), RET 4.
    virtual void native_string_set_erase_subtree_004cec60(Raw set, void* node) = 0;
    // 00427110 BSP_NativeStringVector_Resize, __thiscall(ECX = vector {data,
    // count, capacity}, count), RET 4.
    virtual void native_string_vector_resize_00427110(Raw vector, std::uint32_t count) = 0;
    // 00B66C60 BSP_LuaStateOwner_ExecuteString, __thiscall(ECX = state owner,
    // const string*), RET 4.
    virtual void lua_state_owner_execute_string_00b66c60(void* owner, Raw source) = 0;

    // --- traffic side remap (00487470) -----------------------------------------
    // 00480620 STL_TreeIter_Inc, __thiscall(ECX = iterator {container, node})
    // (vendor body).
    virtual void tree_iterator_increment_00480620(Raw iterator) = 0;
    // 004B1400 SoldierClass_GetOrLoad, __fastcall(ECX = const NativeString*
    // name), EAX = class or null.
    virtual void* soldier_class_get_or_load_004b1400(Raw name) = 0;
    // 00965480 BSP_VehicleClassRegistry_FindByName, __fastcall(ECX = const
    // NativeString* name), EAX = class or null.
    virtual void* vehicle_class_find_by_name_00965480(Raw name) = 0;
    // 0047B3C0 BSP_VehicleVisualClass_GetOrCreate, __thiscall(ECX = class), EAX.
    virtual void* vehicle_visual_class_get_or_create_0047b3c0(void* vehicle_class) = 0;

    // --- tactical library (0050B440) ----------------------------------------------
    // 00441F30 STL vector<uint32> insert-one, __thiscall(ECX = vector,
    // iterator* result, container, where, const value*), RET 10h (vendor).
    virtual void uint_vector_insert_one_00441f30(Raw vector, void* result, void* container, void* where,
                                                 const void* value) = 0;

    // --- line of sight (005220C0) -------------------------------------------------
    // 004B4B00 BSP_Game_GetControlledVehicle, no arguments (plain RET), EAX.
    virtual void* game_get_controlled_vehicle_004b4b00() = 0;
    // 009043A0, __thiscall(ECX = [game+19CCh], const float3* from, const
    // float3* to, hit record*, ignored entity, mask), RET 14h, AL = hit.
    virtual bool world_ray_test_009043a0(void* world, const float* from, const float* to, Raw hit,
                                         void* ignore, std::uint32_t mask) = 0;

    // --- movie tracks -------------------------------------------------------------
    // 00925180, __thiscall(ECX = [game+19CCh], const NativeString* name),
    // RET 4, EAX = the scene entity or null.
    virtual void* world_find_entity_by_name_00925180(void* world, Raw name) = 0;

    // --- device class factory (00443090) ----------------------------------------
    // 00441780 BSP_DeviceClassRegistry_GetSingleton, EAX = registry {vptr,
    // data +4, count +8, capacity +0Ch}.
    virtual void* device_class_registry_get_singleton_00441780() = 0;
    // 00440180 BSP_DeviceClassRegistry_ResizeSlots, __thiscall(ECX =
    // registry+4, count), RET 4.
    virtual void device_class_registry_resize_slots_00440180(Raw slots, std::uint32_t count) = 0;
    // (00B67720 GetByIndex is SweepAHost::lua_object_get_by_index_00b67720.)
    // 00B673A0, __thiscall(ECX = table, const NativeString* key, int value),
    // RET 8: sets a boolean field.
    virtual void lua_object_set_bool_00b673a0(LuaObjectStorage* table, Raw key, std::int32_t value) = 0;
    // The device class constructors, __thiscall(ECX = block), EAX = block:
    // 00442B90 (common gun descriptor; the caller then overwrites the vptr),
    // 00442EC0 single turning gun, 00442C50 bomb platform, 00442CE0
    // multibomb platform, 00442F50 depth charge launcher, 00442FF0 catapult.
    virtual void* gun_descriptor_construct_common_00442b90(void* block) = 0;
    virtual void* construct_single_turning_gun_class_00442ec0(void* block) = 0;
    virtual void* construct_bomb_platform_class_00442c50(void* block) = 0;
    virtual void* construct_multibomb_platform_class_00442ce0(void* block) = 0;
    virtual void* construct_depth_charge_launcher_class_00442f50(void* block) = 0;
    virtual void* construct_catapult_class_00442ff0(void* block) = 0;
    // [[class]+8](class, LuaObject* row), RET 4: the class reads its Lua row.
    virtual void vcall_device_class_read_lua_8(void* device_class, LuaObjectStorage* row) = 0;
    // InterlockedIncrement through the import slot [00CE221C].
    virtual void interlocked_increment_import_00ce221c(void* counter) = 0;

    // --- world teardown (004D2BB0) -----------------------------------------------
    // [[object]+0](object, flags) and [[object]+4](object, flags), RET 4:
    // the destructor slots (flags 1 = delete).
    virtual void vcall_destroy_0(void* object, std::uint32_t flags) = 0;
    virtual void vcall_destroy_4(void* object, std::uint32_t flags) = 0;
    // 0046B680 BSP_SceneDatabase_ClearHiddenRecords, __thiscall(ECX = [00E18680]).
    virtual void scene_database_clear_hidden_records_0046b680(void* database) = 0;
    // 0060D6E0 BSP_PleaseWaitScreen_ShowWithWorker, __thiscall(ECX =
    // [00E19698], int), RET 4; 0060D3E0 BSP_PleaseWaitScreen_Hide,
    // __thiscall(ECX = [00E19698]) (reached by a tail jump).
    virtual void please_wait_show_with_worker_0060d6e0(void* screen, std::int32_t mode) = 0;
    virtual void please_wait_hide_0060d3e0(void* screen) = 0;
    // 00865CF0 BSP_RainDescriptor_GetSingleton, EAX; 008658E0
    // BSP_WorldAtmosphere_ReleaseNode, __thiscall(ECX = that).
    virtual void* rain_descriptor_get_singleton_00865cf0() = 0;
    virtual void world_atmosphere_release_node_008658e0(void* descriptor) = 0;
    // 004BFDF0 BSP_Game_ClearLocalPlayerUnitLists, __thiscall(ECX = game).
    virtual void game_clear_local_player_unit_lists_004bfdf0(void* game) = 0;
    // 004254B0 with two arguments (format, 0), __cdecl (empty body).
    virtual void debug_print1_004254b0(const char* format, std::uint32_t a1) = 0;
    // 00926D90 BSP_MissionEntity_Kill, __thiscall(ECX = entity, cause), RET 4.
    virtual void mission_entity_kill_00926d90(void* entity, std::int32_t cause) = 0;
    // 009496A0, __thiscall(ECX = [00F89B3C]): the object's destructor body
    // (the caller frees the block with 00BF65AC).
    virtual void destruct_009496a0(void* object) = 0;
    // 00B0D190 BSP_RenderService_FlushListener, __thiscall(ECX = [00F8D39C]).
    virtual void render_service_flush_listener_00b0d190(void* service) = 0;
    // 00867B10 BSP_PointEffect_StopChildren, __thiscall(ECX = effect).
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // [[object]+0](object): a reference-counted object's release slot (no
    // arguments, called when InterlockedDecrement(+4) reaches 0).
    virtual void vcall_release_0(void* object) = 0;
    // 004D1100 BSP_EffectManager_GetSingleton, EAX; 008674A0 on it (ECX).
    virtual void* effect_manager_get_singleton_004d1100() = 0;
    virtual void effect_manager_clear_008674a0(void* manager) = 0;
    // 00A7B9C0, __thiscall(ECX = [00F8BBD8]).
    virtual void sound_manager_reset_00a7b9c0(void* manager) = 0;
    // [[sound manager]+4](manager, const float[16]* matrix, float3 by
    // value), RET 10h: the listener transform.
    virtual void vcall_set_listener_4(void* manager, const float* matrix, const float* position) = 0;
    // 00B6DFA0 (ECX = [game+19F8h]) and 00B72250 (ECX = [game+19ECh]):
    // their owners' teardown calls.
    virtual void teardown_00b6dfa0(void* object) = 0;
    virtual void scene_teardown_00b72250(void* scene) = 0;
    // 00AD7B90, __thiscall(ECX = [00F8C210], int), RET 4.
    virtual void gui_root_reset_00ad7b90(void* root, std::int32_t value) = 0;
    // 004C7DD0 BSP_GamePayloadLists_ClearActualStorage and 004D2B10
    // BSP_Game_DestroyWorldContainers, __thiscall(ECX = game).
    virtual void game_payload_lists_clear_004c7dd0(void* game) = 0;
    virtual void game_destroy_world_containers_004d2b10(void* game) = 0;
    // 007F8180 BSP_GameParticipant_ReleaseReferences, __thiscall(ECX = participant).
    virtual void game_participant_release_references_007f8180(void* participant) = 0;

    // --- engine movie parse (004602C0) -------------------------------------------
    // 00BDF4C0 BSP_VFS_ResolveExistingName, __thiscall(ECX = [0109CEEC],
    // string* path), RET 4, AL.
    virtual bool vfs_resolve_existing_name_00bdf4c0(void* vfs, Raw path) = 0;
    // 008D9CF0 BSP_SceneTokenizer_Construct, __thiscall(ECX = 838h block,
    // NativeString path by value (callee-owned), const char* delimiters),
    // RET 0Ch, EAX = tokenizer.
    virtual void* scene_tokenizer_construct_008d9cf0(void* block, Raw path, const char* delimiters) = 0;
    // 0045DDA0, __thiscall(ECX = movie): clears the movie before a parse.
    virtual void engine_movie_reset_0045dda0(void* movie) = 0;
    // 0045EFB0 BSP_EngineMovie_CreateTrack, __thiscall(ECX = movie,
    // NativeString name by value), RET 8, EAX = track.
    virtual void* engine_movie_create_track_0045efb0(void* movie, Raw name) = 0;
    // 0045D8A0 BSP_MovieTrack_SetEntityPath, __thiscall(ECX = track,
    // NativeString by value), RET 8.
    virtual void movie_track_set_entity_path_0045d8a0(void* track, Raw path) = 0;
    // 0045E5E0 BSP_MovieAnimation_ReadAction, __thiscall(ECX = movie,
    // tokenizer, track), RET 8.
    virtual void movie_animation_read_action_0045e5e0(void* movie, void* tokenizer, void* track) = 0;
    // 00460120 STL vector<track*> insert-one, __thiscall(ECX = vector,
    // iterator* result, container, where, const value*), RET 10h (vendor).
    virtual void track_vector_insert_one_00460120(Raw vector, void* result, void* container, void* where,
                                                  const void* value) = 0;
    // [[track]+8](track): the track finishes its setup.
    virtual void vcall_track_finish_8(void* track) = 0;
    // 0045C5B0 BSP_EngineMovie_Rewind, __thiscall(ECX = movie).
    virtual void engine_movie_rewind_0045c5b0(void* movie) = 0;
};

// ============================ segment 3 ====================================
void* device_class_resolve_from_lua_00443090(std::int32_t id, std::uint8_t retain, SweepA3Host& host);
void panel_sequence_describe_properties_0044b5b0(Raw sequence, void* sink, SweepA3Host& host);

// ============================ segment 5 ====================================
void movie_spline_read_point_0045eca0(void* tokenizer, void* owner, SweepA3Host& host);
void engine_movie_stop_0045f600(Raw movie, SweepA3Host& host);
void movie_track_bind_entities_00456dd0(Raw track, SweepA3Host& host);
void engine_movie_parse_file_004602c0(Raw movie, Raw path, SweepA3Host& host);
// The eight NativeStrings arrive by value (the callee releases them).
Raw movie_action_type_construct_0045d480(Raw record, Raw name, Raw refer_label, std::uint8_t refer_flag,
                                         Raw num1_label, std::uint8_t num1_flag, Raw num2_label,
                                         std::uint8_t num2_flag, Raw num3_label, std::uint8_t num3_flag,
                                         Raw enum_label, Raw enum_options, std::uint8_t enum_flag,
                                         Raw offset_label, std::uint8_t offset_flag, SweepA3Host& host);

// ============================ segment 7 ====================================
void traffic_runtime_remap_side_templates_00487470(Raw runtime, SweepA3Host& host);

// ============================ segment 10 ===================================
std::uint32_t game_complete_leaderboard_read_004dd7c0(Raw game, SweepA3Host& host);
void game_begin_engine_movie_004cce50(Raw game, SweepA3Host& host);
void game_nil_script_added_globals_004d32a0(Raw game, SweepA3Host& host);
void game_destroy_world_004d2bb0(Raw game, SweepA3Host& host);

// ============================ segment 12 ===================================
void tactical_library_rebuild_entry_list_0050b440(Raw screen, SweepA3Host& host);

// ============================ segment 14 ===================================
void unit_pick_cycle_target_00526720(Raw screen, SweepA3Host& host);
bool unit_line_of_sight_from_viewer_005220c0(void* unit, SweepA3Host& host);
void hud_binoculars_step_input_axes_0051eaa0(float* value, float* limit, float divisor, float dt,
                                             std::uint8_t* active, std::uint8_t* rising, std::uint8_t* falling,
                                             std::uint8_t keep_limit, SweepA3Host& host);

}  // namespace bsp::cc13_sweepA3
