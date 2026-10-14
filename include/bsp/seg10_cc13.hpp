#pragma once

// Segment 10 (004D0FC0..004E79B0): GGame helpers around the frame and the
// session (state requests, pause, presence, input-context levels, camera
// fov), lazy singletons and their records, scene-entity constructors and
// world-list registration, and small container helpers. Evidence and
// per-function notes: docs/CC13_SEG10.md. Every name is a hypothesis.
//
// Same model as segments 5 and 7: objects are byte pointers, every field is
// read or written at the image's offset, every call goes through a named
// Seg10Host method at the same point, and globals are read through the host.

#include <cstdarg>
#include <cstdint>

namespace bsp::cc13_seg10 {

static_assert(sizeof(void*) == 4, "segment 10 bodies use the Win32 object layout");

using Object = std::uint8_t*;

template <class T>
T& native_at(Object base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

struct Seg10Host {
    virtual ~Seg10Host() = default;

    // Globals.
    virtual Object game_00e188a8() = 0;
    virtual Object mission_00e198c4() = 0;          // [00E198C4]
    virtual Object multi_menu_00e198b4() = 0;       // [00E198B4]
    virtual Object movie_player_00e18d48() = 0;     // [00E18D48]
    virtual Object xlive_00f8abe8() = 0;            // [00F8ABE8]
    virtual std::uint32_t pending_invite_00e198ac() = 0;  // [00E198AC]
    virtual void set_accepted_invite_00e19880(std::uint32_t value) = 0;
    virtual void set_prompt_status_00f1b038(std::int32_t value) = 0;
    virtual float fov_scale_00f889b4() = 0;         // float [00F889B4] (.bss)
    virtual Object display_0109cf04() = 0;          // [0109CF04]
    virtual std::uint32_t resolution_00f88994() = 0;  // dword [00F88994]
    virtual std::uint32_t resolution_00f88998() = 0;  // dword [00F88998]
    // The lazy singleton slots 00F8A71C, 00F8998C, 00E198D0, the enum
    // registry 00E1867C and the universe library 00E18678.
    virtual Object global_slot(std::uint32_t address) = 0;
    virtual void set_global_slot(std::uint32_t address, Object value) = 0;
    virtual Object static_object(std::uint32_t address) = 0;  // the object stored at an image address (00E18A60)

    // CRT and imports.
    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void crt_free_00bf65ac(void* memory) = 0;
    virtual Object operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void crt_free_00bf6989(void* memory) = 0;
    virtual void crt_memcpy_00bf7680(void* dest, const void* src, std::uint32_t bytes) = 0;
    virtual void crt_vsprintf_s_00bf7289(char* dest, std::uint32_t size, const char* format, va_list args) = 0;
    virtual void enter_critical_section_import_00ce2218(Object section) = 0;
    virtual void leave_critical_section_import_00ce2210(Object section) = 0;
    virtual void x_user_set_context_00a4d3f2(std::uint32_t slot, std::uint32_t context, std::uint32_t value) = 0;

    // Virtual calls.
    virtual bool vcall_38_bool(Object object) = 0;  // +38h, no argument, AL

    // Direct calls (ECX first where the image uses one).
    virtual Object singleton_manager_00415350() = 0;
    virtual void singleton_register_00bd0c30(Object manager, Object instance) = 0;
    virtual Object sized_pool_singleton_00419cc0(void* data, std::uint32_t bytes, std::int32_t flag) = 0;
    virtual void sized_pool_return_block_00bd1510(Object pool, void* data, std::uint32_t bytes, std::int32_t flag) = 0;
    virtual void native_string_construct_0041e870(Object native_string, const char* text) = 0;
    virtual void small_string_list_clear_004cf3f0(Object list) = 0;
    virtual void reference_range_release_004cc760(Object first, Object last, Object owner, Object allocator) = 0;
    virtual void pooled_string_list_clear_004d05e0(Object list) = 0;
    virtual Object pooled_string_list_sentinel_004c3020(Object list) = 0;
    virtual void race_records_delete_all_007ffd60(Object game) = 0;
    virtual void bullet_class_cache_clear_006ea1a0() = 0;
    virtual void reference_slots_resize_004cb220(Object slots, std::int32_t count) = 0;
    virtual void device_class_registry_clear_00442170() = 0;
    virtual void vehicle_class_registry_clear_0095ba30() = 0;
    virtual std::int32_t* state_queue_front_004c80a0(Object queue) = 0;
    virtual Object vehicle_class_registry_00437f50() = 0;
    virtual void destroy_006f7b60() = 0;
    virtual void stationary_class_map_destroy_00850930() = 0;
    virtual void wreck_class_registry_clear_004b3e60() = 0;
    virtual void soldier_registry_destroy_004b1540() = 0;
    virtual void buoy_class_registry_delete_00475860() = 0;
    virtual void cloud_class_registry_delete_00479ac0() = 0;
    virtual void anim_class_registry_delete_00472420() = 0;
    virtual void vehicle_visual_registry_delete_0047b4e0() = 0;
    virtual void robot_registry_clear_00900bb0() = 0;
    virtual Object property_tree_library_construct_008f5670(Object memory) = 0;
    virtual void property_tree_library_load_folder_008f7100(Object library, const char* folder) = 0;
    virtual void squadron_free_attack_targets_load_009f89f0(Object record) = 0;
    virtual void bit_cursor_read_unsigned_byte_00428c70(Object cursor, Object out, std::int32_t bits) = 0;
    virtual void bit_cursor_read_qword_00428e90(Object cursor, Object out) = 0;
    virtual void bit_cursor_read_signed_dword_00428d30(Object cursor, Object out, std::int32_t bits) = 0;
    virtual void bit_cursor_read_boolean_00428d70(Object cursor, Object out) = 0;
    virtual Object list_buy_node40_004c6090(Object list, Object next, Object prev, Object value) = 0;
    virtual void list_inc_size_004cf800(Object list, std::int32_t count) = 0;
    virtual void game_end_scene_004d7970(Object game, std::int32_t arg) = 0;
    virtual void multi_menu_leave_to_main_00688c70(Object menu) = 0;
    virtual float* input_repeat_map_subscript_004d6900(Object map, const std::int32_t* key) = 0;
    virtual Object input_manager_004bec00() = 0;
    virtual void ref_ptr_range_release_004ce600(Object first, Object last, Object owner, Object allocator) = 0;
    virtual void ref_ptr_range_release_004ce5c0(Object first, Object last, Object owner, Object allocator) = 0;
    virtual void ref_ptr_range_release_004ce580(Object first, Object last, Object owner, Object allocator) = 0;
    virtual void string_alias_record_destroy_004d45a0(Object record) = 0;
    virtual void session_broadcast_game_closed_007728b0(Object session, std::int32_t arg) = 0;
    virtual void leaderboard_process_004dc2c0(Object ecx, Object arg, Object by_value_block_60) = 0;
    virtual Object tree_buy_head_004c2a80(Object tree) = 0;
    virtual Object tree_buy_head_00443e20(Object tree) = 0;
    virtual Object tree_buy_head_004c2ad0(Object tree) = 0;
    virtual void camera_path_component_destroy_007b1be0(Object component) = 0;
    virtual void game_entity_destruct_009287b0(Object entity) = 0;
    virtual void collision_node_init_fields_004e6480(Object node) = 0;
    virtual void game_entity_register_in_parent_00928560(Object entity) = 0;
    virtual void game_entity_unregister_base_00928570(Object entity) = 0;
    virtual void unit_list_push_back_00484540(Object list, Object entity) = 0;
    virtual void unit_list_erase_node_004837d0(Object list, Object node) = 0;
    virtual void warning_dispatch_player_channel_00985920(Object manager, std::uint32_t player, Object text) = 0;
    virtual void native_string_set_insert_004d0640(Object set, Object result, Object value) = 0;
    virtual void native_string_range_destroy_00432050(Object first, Object last, Object owner, Object allocator) = 0;
    virtual void pointer_vector_assign_004d3740(Object vector, Object source) = 0;
    virtual void input_contexts_apply_levels_004c4300(Object game, std::int32_t level) = 0;
    virtual void input_manager_set_context_level_00a933f0(Object manager, std::int32_t index, std::int32_t level) = 0;
    virtual void enqueue_state_request_004d3ed0(Object queue, const std::int32_t* state) = 0;
    virtual void session_begin_mission_reload_00772610(Object session) = 0;
    virtual void drain_state_requests_004c88a0(Object game) = 0;
    virtual bool is_sign_in_state_valid_0067c8f0() = 0;
    virtual void xlive_restart_signin_00a3e6a0(Object xlive) = 0;
    virtual bool xenon_has_selected_user_00a3e510(Object xlive) = 0;
    virtual std::uint32_t xenon_active_user_slot_00a3eac0(Object xlive) = 0;
    virtual std::uint32_t xenon_accepted_invite_info_00a3e460(Object xlive) = 0;
    virtual std::uint32_t xenon_find_slot_for_invitee_00a3e470(Object xlive) = 0;
    virtual bool movie_player_is_playing_004f89a0(Object player) = 0;
    virtual void movie_player_stop_004f8ac0(Object player) = 0;
    virtual void multi_menu_pop_request_interface_up_to_00688b20(Object menu, std::int32_t level) = 0;
    virtual void game_accept_pending_invite_0067d6e0() = 0;
    virtual void game_raise_multiplayer_menu_004bfc70(Object game) = 0;
    virtual void vector_push_back_004d6840(Object vector, const std::int32_t* value) = 0;
    virtual void input_manager_update_00a92c40(Object manager, float seconds) = 0;
    virtual void game_set_cinematic_mode_004cd0f0(Object game, std::int32_t a, std::int32_t b, std::int32_t c) = 0;
    virtual void game_set_system_pause_004d95f0(Object game, std::int32_t paused) = 0;
    virtual void hud_pause_screen_resume_005fd440(Object screen) = 0;
    virtual void hud_pause_screen_open_005ff0d0(Object screen) = 0;
    virtual void multi_menu_open_pause_00689030(Object menu) = 0;
    virtual void multi_menu_close_pause_006882a0(Object menu) = 0;
    virtual Object global_config_00432650() = 0;
    virtual void camera_set_fov_00b6fbb0(Object camera, float fov) = 0;
    virtual void camera_set_aspect_00b6fbd0(Object camera, float aspect) = 0;
    virtual void camera_set_clear_flags_00b6fe10(Object camera, std::int32_t flags) = 0;
    virtual Object camera_get_viewport_00b6fde0(Object camera) = 0;
    virtual void viewport_set_dimensions_00b1f940(Object viewport, const std::uint32_t* size) = 0;
    virtual void camera_set_far_plane_00b6fc10(Object camera, float far_plane) = 0;
    virtual void game_entity_construct_00928630(Object entity, std::int32_t a, std::uint32_t b) = 0;
    virtual void camera_path_component_construct_007b28f0(Object component, Object owner) = 0;
    virtual void free_camera_set_matrix_0053d700(Object camera) = 0;
};

// Container helpers.
void small_string_node_list_tidy_004d1720(Seg10Host& host, Object list);
void small_string_node_list_tidy_004d38e0(Seg10Host& host, Object list);
void reference_pointer_vector_tidy_004d1890(Seg10Host& host, Object vector);
void reference_pointer_vector_tidy_004d3a30(Seg10Host& host, Object vector);
void pooled_string_list_destruct_004d28a0(Seg10Host& host, Object list);
Object pooled_string_list_construct_004d4830(Seg10Host& host, Object list);
void two_vector_record_construct_004d4db0(Object record);
void string_alias_array_pop_back_004d9e60(Seg10Host& host, Object array);
void shared_array1c_reserve_004e7730(Seg10Host& host, Object holder, std::int32_t capacity);
void shared_array4_reserve_004e7820(Seg10Host& host, Object holder, std::int32_t capacity);
void simple_array_tidy_004e77f0(Seg10Host& host, Object array);

// Game world and records.
void game_destroy_world_containers_004d2b10(Seg10Host& host, Object game);
void game_create_enum_registry_and_universe_library_004d3040(Seg10Host& host);
Object game_record_ce78fc_construct_004d4a20(Seg10Host& host, Object record);
void game_record_ce78fc_destruct_004d4ab0(Seg10Host& host, Object record);
void game_record_ce7aac_construct_004d9d10(Object record);
void game_record_ce7aac_destruct_004d9d60(Seg10Host& host, Object record);
Object game_singleton_f8a71c_004d6ce0(Seg10Host& host);
Object game_singleton_f8998c_004dc200(Seg10Host& host);
Object award_tracker_singleton_004e1ca0(Seg10Host& host);
Object award_tracker_construct_004e18e0(Seg10Host& host, Object tracker);
void session_message_e4_read_004d4bb0(Seg10Host& host, Object message, Object stream);
void game_notify_player_left_004d0fc0(Seg10Host& host, Object game, Object record);
void game_clear_pending_mission_name_004d2a80(Seg10Host& host, Object holder);

// Prompt callbacks, state requests and presence.
void prompt_leave_session_004d7b10(Seg10Host& host);
void prompt_leave_session_004d7b50(Seg10Host& host);
void prompt_set_presence_004db240(Seg10Host& host, std::int32_t result);
void prompt_close_hosted_game_004db260(Seg10Host& host, std::int32_t result);
void game_request_state_004d7920(Seg10Host& host, Object game, std::int32_t state);
void game_request_mission_reload_004d7940(Seg10Host& host, Object game);
void game_request_return_to_title_004d7f90(Seg10Host& host, Object game);
void game_set_user_presence_context_004d8000(Seg10Host& host, Object game);
void leaderboard_process_read_results_004dca80(Seg10Host& host, Object ecx, Object arg);

// Input.
float input_repeat_take_axis_value_004d9420(Seg10Host& host, Object game, std::int32_t action);
bool input_repeat_action_fired_004d9480(Seg10Host& host, Object game, std::int32_t action);
void input_contexts_set_level_set_004d6410(Seg10Host& host, Object game, std::int32_t level, Object ids);
// 004D8AE0 / 004D8B70: cdecl (game, id, id, ..., 0); `ids` is the
// zero-terminated argument block after the game.
void input_contexts_push_level2_004d8ae0(Seg10Host& host, Object game, const std::int32_t* ids);
void input_contexts_push_level3_004d8b70(Seg10Host& host, Object game, const std::int32_t* ids);
void game_toggle_pause_menu_004db030(Seg10Host& host, Object game);

// Camera.
void game_set_view_fov_004dc940(Seg10Host& host, Object game, float scale, std::int32_t index);
void game_apply_camera_display_settings_004dcdf0(Seg10Host& host, Object game);

// Scene entities.
Object camera_path_construct_004e5850(Seg10Host& host, Object entity, std::uint32_t network_id);
void camera_path_destruct_004e58e0(Seg10Host& host, Object entity);
Object nav_point_construct_004e5980(Seg10Host& host, Object entity, std::uint32_t network_id);
std::uint32_t free_camera_set_matrix_return_zero_004e5bb0(Seg10Host& host, std::uint32_t a1, Object camera,
                                                         std::uint32_t a3, std::uint32_t a4);
Object scene_command_record_copy_construct_004e5b20(Object dest, Object source);
Object collision_node_record_construct_004e6540(Seg10Host& host, Object node, std::uint32_t value);
Object scene_object_record_create_004e68a0(Seg10Host& host);
void nav_point_register_004e78d0(Seg10Host& host, Object entity);
void nav_point_unregister_004e78f0(Seg10Host& host, Object entity);
void movie_cam_pos_register_004e7930(Seg10Host& host, Object entity);
void movie_cam_pos_unregister_004e7950(Seg10Host& host, Object entity);
void movie_cam_lookat_register_004e7990(Seg10Host& host, Object entity);
// 004E60E0 cdecl(dest, format, ...): _vsprintf_s(dest, 40h, format, the
// argument block).
void format_string64_004e60e0(Seg10Host& host, char* dest, const char* format, ...);

} // namespace bsp::cc13_seg10
