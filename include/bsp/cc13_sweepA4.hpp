#pragma once
// Packet cc13-sweepA4: the fourth pass over the segment 0..14 leftovers (the
// hardest bodies docs/CC13_SWEEPA3.md left), smallest first.
// docs/CC13_SWEEPA4.md lists every function with its outcome, ABI and
// uncertainty, and the ones skipped with the reason.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a SweepA4Host method named
// for its target; SweepA4Host extends SweepA3Host (and so SweepA2Host and
// SweepAHost), whose bindings and reconstructions are reused directly.
// Descriptive names are hypotheses, not recovered symbols. Nothing here is
// wired.

#include "bsp/cc13_sweepA3.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_sweepA4 {

using cc13_sweepA::Raw;

// The 84h-byte rigid body descriptor 00C5D580 reads, in its Win32 layout
// (bsp/ship_hull_body.hpp DynBodyDescriptor documents each field's reader).
// 00447510 builds it on its stack at ESP+3Ch.
struct DynBodyDescriptorRaw {
    std::uint8_t lock_torque_to_row1;   // +00h
    std::uint8_t pad_01[3];
    float mass;                         // +04h
    float inertia[3];                   // +08h
    float rows[3][3];                   // +14h, three basis rows
    float position[3];                  // +38h
    float linear_velocity[3];           // +44h
    float angular_velocity[3];          // +50h
    std::uint32_t flags;                // +5Ch
    float linear_damping;               // +60h
    float angular_damping;              // +64h
    float max_linear_speed;             // +68h
    float max_angular_speed;            // +6Ch
    void* contact_listener;             // +70h
    void* owner;                        // +74h
    void** shapes;                      // +78h, an inlined vector {data, count, capacity}
    std::uint32_t shape_count;          // +7Ch
    std::uint32_t shape_capacity;       // +80h
};
static_assert(sizeof(DynBodyDescriptorRaw) == 0x84);
static_assert(offsetof(DynBodyDescriptorRaw, position) == 0x38);
static_assert(offsetof(DynBodyDescriptorRaw, flags) == 0x5c);
static_assert(offsetof(DynBodyDescriptorRaw, shapes) == 0x78);

// The 20h record 00447510 push_backs onto list+10h (bsp/game_dynamics_list.hpp
// GameDynamicsRecord is the typed projection).
struct DynamicsRecordRaw {
    void* node;               // +00h, spec+0h
    std::uint32_t lifetime;   // +04h, spec+4h (float bits)
    std::uint32_t field_08;   // +08h, spec+30h (float bits)
    void* body;               // +0Ch
    std::uint32_t anchor[3];  // +10h, spec+24h..+2Ch (float bits)
    std::uint32_t field_1c;   // +1Ch, spec+54h (float bits)
};
static_assert(sizeof(DynamicsRecordRaw) == 0x20);

struct SweepA4Host : cc13_sweepA3::SweepA3Host {
    // --- 00447510 callees ----------------------------------------------------
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix, __thiscall(ECX = node).
    virtual void transform_refresh_world_matrix_00b6db70(void* node) = 0;
    // 00C336C0, __thiscall(ECX = out 3 rows + translation (12 floats),
    // const float m[16]*), RET 4.
    virtual void physics_pose_from_matrix_00c336c0(float* out, const void* matrix) = 0;
    // 00C5D580 Dyn_World_CreateBody, __thiscall(ECX = world, const
    // descriptor*), RET 4 -> body.
    virtual void* physics_world_create_body_00c5d580(void* world, const DynBodyDescriptorRaw* descriptor) = 0;
    // 00C31DC0, __thiscall(ECX = body) -> EAX = [body+70h] (MOV, RET).
    virtual void* physics_body_field_70_00c31dc0(void* body) = 0;
    // 00C31F60, __thiscall(ECX = body, const float lo[3]*, const float hi[3]*),
    // RET 8: copies lo to body+38h..+40h and hi to body+44h..+4Ch.
    virtual void physics_body_set_box_00c31f60(void* body, const float* lo, const float* hi) = 0;
    // 00C31F90, __thiscall(ECX = body, float first[3]*, float second[3]*),
    // RET 8: the body bounds.
    virtual void physics_body_bounds_00c31f90(void* body, float* first, float* second) = 0;
    // 00C37E50 Dyn_Body_SetLinearVelocity, __thiscall(ECX = body, const
    // float[3]*), RET 4.
    virtual void physics_body_set_linear_velocity_00c37e50(void* body, const float* velocity) = 0;
    // 00C37F40 Dyn_Body_SetMass, __thiscall(ECX = body, float), RET 4.
    virtual void physics_body_set_mass_00c37f40(void* body, float mass) = 0;
    // 00C37E70 Dyn_Body_SetInertia, __thiscall(ECX = body, const float[3]*), RET 4.
    virtual void physics_body_set_inertia_00c37e70(void* body, const float* inertia) = 0;
    // [[node]+4Ch](node, out*), RET 4 -> EAX = six floats (the node's box,
    // 00447510 reads +0h..+8h and +0Ch..+14h).
    virtual const float* vcall_node_box_4c(void* node, void* out) = 0;
    // 00447480 STL_Vector_PushBack_00447480, __thiscall(ECX = vector, const
    // 20h record*), RET 4 (vendor body).
    virtual void dynamics_record_push_back_00447480(Raw vector, const DynamicsRecordRaw* record) = 0;

    // --- 004E2200 callees (STL instantiations are vendor code, called) -------
    // A legacy string is the 1Ch-byte MSVC basic_string: +4h the 16-byte
    // buffer or the heap pointer, +14h size, +18h capacity (heap when >= 10h).
    // 00408120 BSP_LegacySboString_AssignSubstring, __thiscall(ECX = string,
    // const string* source, pos, count), RET 0Ch.
    virtual void legacy_string_assign_substring_00408120(Raw string, Raw source, std::uint32_t pos,
                                                         std::uint32_t count) = 0;
    // 004CECF0 STL_List_Erase_004cecf0, __thiscall(ECX = list, iterator* out,
    // checked iterator {container, node} by value), RET 0Ch.
    virtual void list_erase_004cecf0(Raw list, Raw out_iterator, Raw iterator_container, void* node) = 0;
    // 004BEB60 STL_LegacySboString_CompareSubstring_004beb60, __thiscall(ECX =
    // string, pos, count, const char* text, text_count), RET 10h -> EAX (0 = equal).
    virtual std::int32_t legacy_string_compare_substring_004beb60(Raw string, std::uint32_t pos,
                                                                  std::uint32_t count, const char* text,
                                                                  std::uint32_t text_count) = 0;
    // 004260B0 BSP_NativeString_FromInt, __thiscall(ECX = 8-byte string, int), RET 4.
    virtual void native_string_from_int_004260b0(Raw string, std::int32_t value) = 0;
    // 00407290 BSP_LegacySboString_ConstructFromCString, __thiscall(ECX =
    // string, const char*), RET 4 -> EAX = string.
    virtual void legacy_string_construct_from_cstring_00407290(Raw string, const char* text) = 0;
    // 004D3900 STL_List_PushBack_004d3900, __thiscall(ECX = list, const string*), RET 4.
    virtual void list_push_back_004d3900(Raw list, Raw string) = 0;
    // 004072D0 BSP_LegacySboString_Destroy, __thiscall(ECX = string).
    virtual void legacy_string_destroy_004072d0(Raw string) = 0;
    // 004D3ED0 BSP_Game_EnqueueStateRequest, __thiscall(ECX = game+5D8h, const int*), RET 4.
    virtual void game_enqueue_state_request_004d3ed0(Raw queue, const std::int32_t* request) = 0;
    // 004C2DF0 BSP_CommandLine_MatchesToken, __cdecl(const string*, const char*) -> AL.
    virtual bool command_line_matches_token_004c2df0(Raw string, const char* token) = 0;
    // 004CDBE0 STL_LegacySboString_ConstructSubstring_004cdbe0, __thiscall(ECX
    // = source, string* out, pos, count), RET 0Ch -> EAX = out.
    virtual void legacy_string_construct_substring_004cdbe0(Raw source, Raw out, std::uint32_t pos,
                                                            std::uint32_t count) = 0;
    // 004BCC00 BSP_String_LowercaseAscii, __thiscall(ECX = 8-byte string).
    virtual void string_lowercase_ascii_004bcc00(Raw string) = 0;
    // 004D0860 BSP_Game_SplitCommandToken, __cdecl(string* out, const string*
    // left, const string* right) -> EAX = out: out = left + right.
    virtual void game_split_command_token_004d0860(Raw out, Raw left, Raw right) = 0;
    // 004116A0 BSP_LegacySboString_CopyConstruct, __thiscall(ECX = string, const string*), RET 4.
    virtual void legacy_string_copy_construct_004116a0(Raw string, Raw source) = 0;
    // 004BCB10 STL_inst_004bcb10 (operator[]), __thiscall(ECX = string, index), RET 4 -> char*.
    virtual char* legacy_string_at_004bcb10(Raw string, std::uint32_t index) = 0;
    // [00CE2264] GetCurrentDirectoryA(DWORD length, char* buffer), __stdcall.
    virtual std::uint32_t get_current_directory_a_import_00ce2264(std::uint32_t length, char* buffer) = 0;
    // 004C89A0 BSP_Game_AssignConsoleString2198, __thiscall(ECX = game, const 8-byte string*), RET 4.
    virtual void game_assign_console_string_2198_004c89a0(Raw game, Raw string) = 0;
    // 004E1D70 BSP_Game_BuildSceneRecord, __thiscall(ECX = game, const char*,
    // int, int, int), RET 10h.
    virtual void game_build_scene_record_004e1d70(Raw game, const char* path, std::int32_t a, std::int32_t b,
                                                  std::int32_t c) = 0;
    // 004C6890 BSP_Game_SelectSceneRecord, __thiscall(ECX = game, int index), RET 4.
    virtual void game_select_scene_record_004c6890(Raw game, std::int32_t index) = 0;
    // 004C5E00 BSP_StorageManager_GetNewestSlotName, __thiscall(ECX = storage
    // manager, 8-byte string* out), RET 4.
    virtual void storage_manager_get_newest_slot_name_004c5e00(void* manager, Raw out) = 0;
    // 00436710 BSP_ProfileManager_ConstructEmptySaveName, __thiscall(ECX = 8-byte string) -> EAX = it.
    virtual void profile_manager_construct_empty_save_name_00436710(Raw string) = 0;
    // 00449AF0 BSP_NativeString_NotEqualCaseInsensitive, __thiscall(ECX =
    // string, const string*), RET 4 -> AL.
    virtual bool native_string_not_equal_case_insensitive_00449af0(Raw string, Raw other) = 0;
    // 007FF100 BSP_PlayerProfile_RequestRead, __thiscall(ECX = [00E188A8]+650h,
    // const 8-byte string*, continuation function), RET 8.
    virtual void player_profile_request_read_007ff100(Raw profile, Raw name, std::uint32_t continuation) = 0;

    // --- 004E3AA0 callees -----------------------------------------------------
    // 00BE0A30 BSP_FileBlock_Construct, __thiscall(ECX = 1Ch-byte block, const
    // 8-byte string*, flag), RET 8.
    virtual void file_block_construct_00be0a30(Raw block, Raw name, std::uint32_t flag) = 0;
    // 00BDCB30 BSP_FileBlock_Destroy, __thiscall(ECX = block).
    virtual void file_block_destroy_00bdcb30(Raw block) = 0;
    // 00AEFF40 BSP_TextureAtlas_LoadDirectory, __thiscall(ECX = [00F8C26C],
    // const 8-byte string*), RET 4.
    virtual void texture_atlas_load_directory_00aeff40(void* atlas, Raw directory) = 0;
    // 00AF0060 BSP_TextureAtlas_Load, __thiscall(ECX = [00F8C26C], const
    // 8-byte string*), RET 4.
    virtual void texture_atlas_load_00af0060(void* atlas, Raw path) = 0;
    // 004D3040 BSP_Game_CreateEnumRegistryAndUniverseLibrary, __thiscall(ECX = game).
    virtual void game_create_enum_registry_and_universe_library_004d3040(Raw game) = 0;
    // 00450540 STL_CheckedNativeStringVector_Append_00450540, __thiscall(ECX
    // = 10h-byte vector, const 8-byte string*), RET 4 (vendor body).
    virtual void native_string_vector_append_00450540(Raw vector, Raw string) = 0;
    // 00B6D3C0 BSP_LuaGlobals_ResetUnlessLoaded_Provisional, __thiscall(ECX =
    // the string vector).
    virtual void lua_globals_reset_unless_loaded_00b6d3c0(Raw vector) = 0;
    // 00432050 STL_NativeStringRange_Destroy_00432050, ECX = begin, EDX = end,
    // two unused stack arguments, RET 8 (vendor body).
    virtual void native_string_range_destroy_00432050(void* begin, void* end) = 0;
    // 0057BEC0 BSP_LoadingScreen_ReportProgress, __stdcall(float), RET 4.
    virtual void loading_screen_report_progress_0057bec0(float fraction) = 0;
    // [object]->vtable+10h, __thiscall(ECX = object): the init slot of the
    // five managers 004E3AA0 creates.
    virtual void vcall_manager_init_10(void* object) = 0;
    // The constructors, each __thiscall(ECX = storage) -> EAX = storage.
    virtual void* frame_stats_screen_construct_0053ba10(void* storage) = 0;   // 84h bytes
    virtual void* manager_e18db0_construct_0051f990(void* storage) = 0;     // 70h bytes
    virtual void* manager_e19900_construct_006b9450(void* storage) = 0;     // 20h bytes
    virtual void* manager_e19698_construct_0060d290(void* storage) = 0;     // 14h bytes
    virtual void* manager_e1930c_construct_0054a8d0(void* storage) = 0;     // ACh bytes
    virtual void* manager_21a0_construct_0091d640(void* storage) = 0;       // 14B8h bytes
    virtual void* manager_21d8_construct_006afe80(void* storage) = 0;       // 10h bytes
    // 0091B2E0 / 006AD180, __thiscall(ECX = the new game+21A0h / +21D8h object).
    virtual void manager_21a0_init_0091b2e0(void* object) = 0;
    virtual void manager_21d8_init_006ad180(void* object) = 0;
    // 0046F350 BSP_SceneDatabase_CreateSingleton().
    virtual void scene_database_create_singleton_0046f350() = 0;
    // 004E1CA0 () -> EAX, then 006921B0 __thiscall(ECX = it).
    virtual void* award_tracker_get_singleton_004e1ca0() = 0;
    virtual void award_tracker_reload_hint_tables_006921b0(void* object) = 0;
    // 00992A50 () creates [00F8A2FC]; [[00F8A2FC]]+4 __thiscall(ECX =
    // [00F8A2FC], game), RET 4.
    virtual void online_system_create_00992a50() = 0;
    virtual void vcall_attach_game_4(void* object, void* game) = 0;
    // 00951560 __thiscall(ECX = the static object 00F89A08 / 00F89A5C).
    virtual void entity_id_table_reset_00951560(void* object) = 0;
    // 00886900 __thiscall(ECX = [game+1A08h]).
    virtual void mission_lua_run_global_script_folders_00886900(void* object) = 0;
    // Singletons and loaders, all () with no register input.
    virtual void* game_settings_get_singleton_00424c40() = 0;
    virtual void* game_singleton_00f8a71c_get_or_create_004d6ce0() = 0;
    virtual void* soldier_globals_get_singleton_00485520() = 0;
    virtual void* game_singleton_00e19a64_get_or_create_004c1d10() = 0;
    virtual void plane_part_classes_load_globals_004a9bd0() = 0;
    virtual void ship_explosion_load_globals_004af290() = 0;
    // 004E27E0 __thiscall(ECX = game).
    virtual void game_apply_command_line_options_004e27e0(Raw game) = 0;

    // --- 004EA080 callees -----------------------------------------------------
    // 004BCA50 BSP_Game_GetEffectiveGameMode, __thiscall(ECX = game) -> EAX.
    virtual std::int32_t game_get_effective_game_mode_004bca50(Raw game) = 0;
    // 00467310 BSP_FormatToBuffer32, __cdecl(char* out, const char* format,
    // ...) with one int here: _vsprintf_s(out, 20h, format, va).
    virtual void format_to_buffer32_00467310(char* out, const char* format, std::int32_t value) = 0;
    // 0095BA60 BSP_VehicleClass_MarkPartyRequired, __fastcall(ECX = type id,
    // EDX = party).
    virtual void vehicle_class_mark_party_required_0095ba60(std::int32_t type, std::int32_t party) = 0;
    // 0046BE90 BSP_SceneEntity_RegisterHiddenStockUnit, __thiscall(ECX =
    // [00E18680], type id), RET 4.
    virtual void scene_entity_register_hidden_stock_unit_0046be90(void* scene_database, std::int32_t type) = 0;
    // 00B68D70 BSP_LuaObject_ResolveDotPath, __thiscall(ECX = object,
    // LuaObject* out, const 8-byte string* path, bool* found), RET 0Ch -> EAX = out.
    virtual cc13_sweepA::LuaObjectStorage* lua_object_resolve_dot_path_00b68d70(
        cc13_sweepA::LuaObjectStorage* object, cc13_sweepA::LuaObjectStorage* out, Raw path, bool* found) = 0;

    // --- 004507D0 callees -----------------------------------------------------
    // The reading archive's slots (+4h begin and +8h end are SweepA3Host's
    // vcall_sink_begin_4 / vcall_sink_end_8, same ABI):
    // [[archive]+0Ch](archive, key, reference, default), three TaggedValues by
    // value, RET 18h: read into the reference or store the default.
    virtual void vcall_archive_read_default_c(void* archive, cc13_sweepA3::TaggedValue key,
                                              cc13_sweepA3::TaggedValue reference,
                                              cc13_sweepA3::TaggedValue fallback) = 0;
    // [[archive]+10h](archive, key, reference), RET 10h.
    virtual void vcall_archive_read_10(void* archive, cc13_sweepA3::TaggedValue key,
                                       cc13_sweepA3::TaggedValue reference) = 0;
    // [[archive]+14h](archive, key), RET 8 -> AL.
    virtual bool vcall_archive_has_14(void* archive, cc13_sweepA3::TaggedValue key) = 0;
    // 0044EB90 STL_Vector_PushBack_0044eb90, __thiscall(ECX = vector of
    // command pointers, void* const*), RET 4 (vendor body).
    virtual void command_vector_push_back_0044eb90(Raw vector, void* const* command) = 0;
    // 0044BE50 BSP_DialogCommand_CreateFromArchive, __thiscall(ECX = archive) -> EAX.
    virtual void* dialog_command_create_from_archive_0044be50(void* archive) = 0;
    // 0044E6E0 STL_Vector_InsertOne_0044e6e0, __thiscall(ECX = vector,
    // iterator* out, checked iterator {container, pointer} by value, void*
    // const*), RET 10h (vendor body).
    virtual void command_vector_insert_0044e6e0(Raw vector, void* out_iterator, Raw iterator_container,
                                                void* where, void* const* command) = 0;
    // 00450600 STL_Vector_PushBack_00450600, __thiscall(ECX = vector of 14h
    // panel slots, const slot*), RET 4 (vendor body).
    virtual void panel_slot_vector_push_back_00450600(Raw vector, Raw slot) = 0;
    // 0044B530 BSP_PanelSequenceSlot_Destroy, __thiscall(ECX = 14h slot).
    virtual void panel_sequence_slot_destroy_0044b530(Raw slot) = 0;

    // --- 0045DE50 callees -----------------------------------------------------
    // The 31 movie action constructors 0045DE50 switches over (00465C40
    // SetTarget .. 00466370 HideEntity; the table is kMovieActionFactory in
    // src/cc13_sweepA4.cpp), each __thiscall(ECX = new storage, float
    // activate time), RET 4 -> EAX = the action. `constructor` is the
    // image address of the one the case calls.
    virtual void* movie_action_type_construct(std::uint32_t constructor, void* storage, float activate_time) = 0;

    // --- 00459020 callees (STL list iterators are vendor code, called) -------
    // 00454340 STL_inst_00454340, __thiscall(ECX = checked list iterator,
    // iterator* out, int), RET 8: postfix increment.
    virtual void list_iterator_post_increment_00454340(Raw iterator, void* out, std::int32_t unused) = 0;
    // 00454310 STL_inst_00454310, __thiscall(ECX = iterator) -> EAX = it:
    // prefix decrement (node = node+4h, both checked).
    virtual Raw list_iterator_pre_decrement_00454310(Raw iterator) = 0;
    // 004552C0 STL_inst_004552c0, __thiscall(ECX = iterator) -> EAX = &value.
    virtual void* list_iterator_deref_004552c0(Raw iterator) = 0;
    // 004552F0 STL_inst_004552f0, __thiscall(ECX = iterator, iterator* out, int), RET 8.
    virtual void list_iterator_post_step_004552f0(Raw iterator, void* out, std::int32_t unused) = 0;
    // 004581E0, __thiscall(ECX = track, float* out (3 floats), float time,
    // int flag), RET 0Ch -> EAX = out: the path position at a time.
    virtual const float* movie_track_point_at_time_004581e0(Raw track, float* out, float time, std::int32_t flag) = 0;
    // 00458160, __thiscall(ECX = track) -> ST0 (float).
    virtual float movie_track_time_00458160(Raw track) = 0;
    // 00455BC0, __thiscall(ECX = track, iterator* out, float time), RET 8.
    virtual void movie_track_find_point_00455bc0(Raw track, void* out_iterator, float time) = 0;
    // 00457FF0 STL_inst_00457ff0, __thiscall(ECX = point storage, const
    // point*), RET 4: the point copy (a native string at +4h, xyz at +0Ch).
    virtual void movie_point_copy_construct_00457ff0(void* storage, const void* source) = 0;
    // 00456D10 BSP_NativeStringAt4_DestroyStorage_00456D10, __thiscall(ECX = point).
    virtual void native_string_at4_destroy_storage_00456d10(void* object) = 0;
    // 00419510 BSP_Vector3f_Normalize, __fastcall(ECX = out, EDX = v) -> EAX = out.
    virtual const float* vector3_normalize_00419510(float* out, const float* v) = 0;
    // 00453F40 BSP_Vector3_ExactEquals, __thiscall(ECX = a, const b*), RET 4 -> AL.
    virtual bool vector3_exact_equals_00453f40(const float* a, const float* b) = 0;
    // 00412E20 BSP_Math_TangentX87Float, __stdcall(float), RET 4 -> ST0 (a float).
    virtual float math_tangent_x87_float_00412e20(float angle) = 0;
    // [[entity]+4Ch](entity, position xyz, direction xyz, {sin, cos, 0, time})
    // by value, RET 28h: places the entity.
    virtual void vcall_entity_place_4c(void* entity, const float* position, const float* direction,
                                       const float* roll_time) = 0;
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose, __thiscall(ECX = node).
    virtual void scene_node_invalidate_subtree_pose_0042ed50(void* node) = 0;
    // 004560E0 BSP_MovieTrack_RunDueActions, __thiscall(ECX = track, float time), RET 4.
    virtual void movie_track_run_due_actions_004560e0(Raw track, float time) = 0;

    // --- 00526180 callees -----------------------------------------------------
    // 00803D40 BSP_Recon_FilterUndetectedUnit, __fastcall(ECX = party, EDX = unit) -> EAX.
    virtual std::uint32_t recon_filter_undetected_unit_00803d40(void* party, void* unit) = 0;
    // 004254B0 (empty debug print), __cdecl with four and two arguments.
    virtual void debug_print4_004254b0(const char* format, std::uint32_t a1, std::uint32_t a2, std::uint32_t a3,
                                       std::uint32_t a4) = 0;
    virtual void debug_print2_004254b0(const char* format, std::uint32_t a1, std::uint32_t a2) = 0;
    // 00524000 TMPL_ObserverHandle_CopyRange_00524000, __cdecl(first, last,
    // destination, three by-value tag dwords) (template body, called).
    virtual void observer_handle_copy_range_00524000(void* first, void* last, void* destination,
                                                     std::uint32_t tag0, std::uint32_t tag1, std::uint32_t tag2) = 0;
    // 00522860, __thiscall(ECX = 8-byte {unit, float} record, unit), RET 4 ->
    // EAX = record (fills the unit and its distance from [00E188A8]+1ED4h).
    virtual void unit_distance_record_construct_00522860(Raw record, void* unit) = 0;
    // 004B7EF0 BSP_ObserverLink_DestroyBase, __thiscall(ECX = handle).
    virtual void observer_link_destroy_base_004b7ef0(Raw handle) = 0;

    // --- 00487A70 callees -----------------------------------------------------
    // 00903860 BSP_World_GroundHeightAt, __thiscall(ECX = [[00E188A8]+19CCh],
    // const float3*, float* out), RET 8 -> AL (unused here).
    virtual bool world_ground_height_at_00903860(void* world, const float* point, float* out) = 0;
    // 008685E0 BSP_PointEffect_CreateFromPosition, ECX = ref-counted out
    // pointer, EDX = manager ([[00E188A8]+19ECh]), stack (ref-counted
    // definition by value (already AddRef'd by the caller), const float3*,
    // int, int), RET 10h -> EAX = the out pointer.
    virtual void** point_effect_create_from_position_008685e0(void** out, void* manager, void* definition,
                                                              const float* point, std::int32_t a, std::int32_t b) = 0;
    // 004842C0 BSP_PointEffect_SetWorldPoint, __thiscall(ECX = effect, const float3*), RET 4.
    virtual void point_effect_set_world_point_004842c0(void* effect, const float* point) = 0;
    // 00B64640 / 00B646E0 / 00B64780 BSP_Matrix_BuildRotationX / Y / Z, ECX =
    // out 4x4, EDX = const float* angle -> EAX = out.
    virtual float* matrix_build_rotation_x_00b64640(float* out, const float* angle) = 0;
    virtual float* matrix_build_rotation_y_00b646e0(float* out, const float* angle) = 0;
    virtual float* matrix_build_rotation_z_00b64780(float* out, const float* angle) = 0;
    // [[entity]+0D8h](entity): after the world matrix is replaced.
    virtual void vcall_entity_matrix_changed_d8(void* entity) = 0;

    // --- 0051CDE0 callees -----------------------------------------------------
    // 00518DA0 BSP_FrontEndFrame_SetSelection, __fastcall(ECX = int).
    virtual void front_end_frame_set_selection_00518da0(std::int32_t selection) = 0;
    // 005189B0 BSP_FrontEndFrame_SetTitle, __thiscall(ECX = const 8-byte string*).
    virtual void front_end_frame_set_title_005189b0(Raw title) = 0;
    // 0051CC90 STL_BriefingPageMap_Index_0051cc90, __thiscall(ECX = map
    // (screen+12Ch), const key*), RET 4 -> EAX = &value (vendor body).
    virtual void** briefing_page_map_index_0051cc90(Raw map, const void* key) = 0;
    // 00AA7E00 BSP_GuiWidget_FindChildByName, __thiscall(ECX = widget, const
    // 8-byte string*, int), RET 8 -> EAX.
    virtual void* gui_widget_find_child_by_name_00aa7e00(void* widget, Raw name, std::int32_t flag) = 0;
    // 00AAA2B0 BSP_GuiWidget_FindChildByNameSuffix, __thiscall(ECX = widget,
    // const 8-byte string*), RET 4 -> EAX.
    virtual void* gui_widget_find_child_by_name_suffix_00aaa2b0(void* widget, Raw suffix) = 0;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(ECX = node,
    // float, char recurse), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float value, std::int8_t recurse) = 0;
    // 0051C5F0 STL_Vector_InsertOne_0051c5f0, __thiscall(ECX = vector of
    // widget pointers, iterator* out, checked iterator {container, pointer} by
    // value, void* const*), RET 10h (vendor body).
    virtual void widget_vector_insert_0051c5f0(Raw vector, void* out_iterator, Raw iterator_container, void* where,
                                               void* const* widget) = 0;
    // 00A9BEC0 BSP_GuiListbox_ClearRows, __thiscall(ECX = listbox).
    virtual void gui_listbox_clear_rows_00a9bec0(void* listbox) = 0;
    // [[listbox]+60h](listbox, int) and [[listbox]+80h](listbox, int), RET 4.
    virtual void vcall_listbox_60(void* listbox, std::int32_t value) = 0;
    virtual void vcall_listbox_80(void* listbox, std::int32_t value) = 0;
    // 00AAB4C0 BSP_GuiWidget_CloneSubtree, __thiscall(ECX = template widget,
    // parent clone), RET 4 -> EAX = the clone.
    virtual void* gui_widget_clone_subtree_00aab4c0(void* widget, void* parent) = 0;
    // 00A9D750 BSP_GuiListbox_InsertRow, __thiscall(ECX = listbox, row,
    // iterator pointer, after byte), RET 0Ch.
    virtual void gui_listbox_insert_row_00a9d750(void* listbox, void* row, void* position, std::int32_t after) = 0;
    // 00ABBE50 BSP_GuiText_SetLocalisedCString, __thiscall(ECX = text widget,
    // const char*, localise flag), RET 8.
    virtual void gui_text_set_localised_cstring_00abbe50(void* text, const char* value, std::int32_t localise) = 0;
    // 00A9CC50 BSP_GuiListbox_ScrollBy / 00A9C7C0 BSP_GuiListbox_SelectIndex,
    // __thiscall(ECX = listbox, int), RET 4.
    virtual void gui_listbox_scroll_by_00a9cc50(void* listbox, std::int32_t count) = 0;
    virtual void gui_listbox_select_index_00a9c7c0(void* listbox, std::int32_t index) = 0;
};

// The 1Ch-byte legacy (MSVC 2008) basic_string, storage only.
struct LegacyStringStorage {
    alignas(4) std::uint8_t bytes[0x1c];
};
// The 8-byte native string {length +0, data +4}, storage only.
struct NativeStringStorage {
    alignas(4) std::uint8_t bytes[8];
};

// 00447510 BSP_GameDynamicsList_Add, __thiscall(ECX = list ([00E188A8]+30h),
// const spec*, bool scale_velocity), RET 8.
void game_dynamics_list_add_00447510(Raw list, Raw spec, bool scale_velocity, SweepA4Host& host);

// 004E2200 BSP_Game_RunConsoleCommandQueue, __thiscall(ECX = game), plain RET.
void game_run_console_command_queue_004e2200(Raw game, SweepA4Host& host);

// 004E3AA0 BSP_Game_OnInit, __thiscall(ECX = game), plain RET.
void game_on_init_004e3aa0(Raw game, SweepA4Host& host);

// 004EA080 BSP_SceneClass_ReadStockLists, __thiscall(ECX = scene property
// bag), plain RET (the reader of class-table row 4Dh, src/scene_entity_factory.cpp).
void scene_class_read_stock_lists_004ea080(Raw bag, SweepA4Host& host);

// 004507D0 BSP_PanelSequence_ReadProperties, __thiscall(ECX = panel sequence,
// archive), RET 4.
void panel_sequence_read_properties_004507d0(Raw sequence, void* archive, SweepA4Host& host);

// 0045DE50 BSP_MovieAction_Create, __stdcall(type, float activate time,
// 8-byte string comment by value), RET 10h -> EAX = the action.
void* movie_action_create_0045de50(std::int32_t type, float activate_time, std::uint32_t comment_length,
                                  void* comment_data, SweepA4Host& host);

// 004E8BF0 BSP_SceneHeader_Assign, __thiscall(ECX = header, const header*),
// RET 4 -> EAX = header.
Raw scene_header_assign_004e8bf0(Raw header, Raw source, SweepA4Host& host);

// 00459020 BSP_MovieEntityTrack_Update, __thiscall(ECX = entity track, float
// time), RET 4 (vtable 00CE4DB8 slot +0Ch).
void movie_entity_track_update_00459020(Raw track, float time, SweepA4Host& host);

// 00526180, __thiscall(ECX = unit pick screen), plain RET: refreshes the
// target cycle list (handles at screen+C4h) in place.
void unit_pick_refresh_cycle_list_00526180(Raw screen, SweepA4Host& host);

// 00487A70, __thiscall(ECX = debris set, float dt), RET 4; held only by the
// vtable slot at 00CE6854.
void debris_set_update_00487a70(Raw set, float dt, SweepA4Host& host);

// 0051CDE0 BSP_BriefingScreen_Enter, __thiscall(ECX = briefing screen), plain
// RET (slot +18h of the vtable at 00CEC948).
void briefing_screen_enter_0051cde0(Raw screen, SweepA4Host& host);

}  // namespace bsp::cc13_sweepA4
