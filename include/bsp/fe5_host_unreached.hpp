#pragma once
// A base for host adapters over bsp::cc13_fe5::Fe5Host (and its
// MainMenuLocalisationHost base), include/bsp/frontend_seg16_24_28_cc13.hpp.
//
// Fe5Host carries 290 engine services as pure virtuals and no game host
// implements it, so every Fe5Host body was unreachable from a host root. This
// class overrides every one of them with a loud failure; an adapter derives from
// it and overrides only the services its own path reaches, each mapped onto the
// host's real objects. A service the adapter does not override throws
// std::logic_error naming the service, so a path the adapter did not read fails
// at once instead of running on an invented value.
//
// Written mechanically from the declarations in frontend_seg16_24_28_cc13.hpp
// (goal 3 chain 233): one override per pure virtual, same signature, in the
// header's order. It is a C++ adapter base, not an image type: no address, no
// image layout. Keep it in step with Fe5Host; a missing override fails to
// compile only where an adapter is instantiated.

#include "bsp/frontend_seg16_24_28_cc13.hpp"

#include <stdexcept>
#include <string>

namespace bsp::cc13_fe5 {

[[noreturn]] inline void fe5_unreached(const char* service) {
    throw std::logic_error(std::string("Fe5Host service not provided by this adapter: ") + service);
}

// Each body is the one macro below rather than a written-out block: these
// overrides only fail, so nothing reading the sources for implementations of
// a service (tools/wire_readiness.py among them) should take one for an
// implementation of a same-named reconstructed function.
#define BSP_FE5_UNREACHED { ::bsp::cc13_fe5::fe5_unreached(__func__); }
#pragma warning(push)
#pragma warning(disable : 4100)  // the parameter names document the services
class Fe5HostUnreached : public Fe5Host {
public:
    void localization_register_table_00aa0d30(std::string_view table) override BSP_FE5_UNREACHED
    void widget_set_visible_34(void* widget, bool visible) override BSP_FE5_UNREACHED
    void widget_set_alpha_4c(void* widget, float alpha) override BSP_FE5_UNREACHED
    void widget_set_colour_50(void* widget, const float rgba[4]) override BSP_FE5_UNREACHED
    void text_set_localised_key_00abbe50(void* text, const char* key) override BSP_FE5_UNREACHED
    void* gui_find_child_00aa7e00(void* parent, std::string_view name) override BSP_FE5_UNREACHED
    void* gui_load_page_00aa5840(std::string_view name) override BSP_FE5_UNREACHED
    void front_end_screen_register_004f71d0(Raw screen) override BSP_FE5_UNREACHED
    void screen_virtual_14(Raw screen) override BSP_FE5_UNREACHED
    void push_page_slot_004d6790(void* vector, void** slot) override BSP_FE5_UNREACHED
    void sound_stop_08(void* handle, bool immediately) override BSP_FE5_UNREACHED
    void sound_handle_release(void* handle) override BSP_FE5_UNREACHED
    void sound_handle_add_ref(void* handle) override BSP_FE5_UNREACHED
    void* ingame_gui_sound_class_00e1988c() override BSP_FE5_UNREACHED
    void* sound_create_tracked_normal(void* sample, void* sound_class) override BSP_FE5_UNREACHED
    void award_tracker_trigger_hint_00690fd0(std::string_view hint) override BSP_FE5_UNREACHED
    void tree_iterator_increment_00683d10(Raw iterator) override BSP_FE5_UNREACHED
    void front_end_registry_insert_00684c40(void* manager) override BSP_FE5_UNREACHED
    Raw front_end_registry_00e19898() override BSP_FE5_UNREACHED
    std::size_t tree_range_distance_00684340(const void* const range[4]) override BSP_FE5_UNREACHED
    void tree_erase_range_00684d00(Raw tree, const void* const range[4]) override BSP_FE5_UNREACHED
    void target_observer_destruct_0042bca0(Raw observer) override BSP_FE5_UNREACHED
    Raw logo_sequence_global_00e198a4() override BSP_FE5_UNREACHED
    void set_logo_sequence_global_00e198a4(void* logo) override BSP_FE5_UNREACHED
    void logo_sequence_advance_00685070(Raw logo) override BSP_FE5_UNREACHED
    void native_string_range_destroy_free_00432050(void* first, void* last) override BSP_FE5_UNREACHED
    void crt_free(void* block) override BSP_FE5_UNREACHED
    void front_end_manager_destroy_base_00684fa0(Raw manager) override BSP_FE5_UNREACHED
    void front_end_manager_construct_base_00684e10(Raw manager) override BSP_FE5_UNREACHED
    void logo_table_open_00b66270() override BSP_FE5_UNREACHED
    bool logo_table_entry(int index, std::string& fmv, float& delay) override BSP_FE5_UNREACHED
    void logo_table_close() override BSP_FE5_UNREACHED
    const char* user_geography_region_00439100() override BSP_FE5_UNREACHED
    void native_string_vector_append_00450540(Raw vector, const std::string& value) override BSP_FE5_UNREACHED
    void float_vector_insert_0045a120(Raw vector, float value) override BSP_FE5_UNREACHED
    void game_on_init_once_004dd5b0(int flag) override BSP_FE5_UNREACHED
    void sound_stream_set_gain_00a864f0(void* stream, float gain) override BSP_FE5_UNREACHED
    void sound_stream_update_00a874d0(void* stream, float seconds) override BSP_FE5_UNREACHED
    float music_gain_00f889a8() override BSP_FE5_UNREACHED
    Raw session_browser_00e198b8() override BSP_FE5_UNREACHED
    void trace_004254b0(const char* format, const char* argument) override BSP_FE5_UNREACHED
    const char* interface_name_00e08cd8(int interface_id) override BSP_FE5_UNREACHED
    bool front_end_manager_apply_request_00684600(Raw manager, int interface_id, int argument) override BSP_FE5_UNREACHED
    int multi_menu_map_interface_00687330(Raw manager, int interface_id) override BSP_FE5_UNREACHED
    void front_end_screens_set_level4_004f8710(int screen_set) override BSP_FE5_UNREACHED
    void game_input_contexts_set_level4_004d8c00() override BSP_FE5_UNREACHED
    void gui_manager_set_enabled_00aa0e00(bool enabled) override BSP_FE5_UNREACHED
    void multi_menu_pop_request_006881f0(Raw manager) override BSP_FE5_UNREACHED
    void object_virtual(Raw object, std::uint16_t slot) override BSP_FE5_UNREACHED
    void object_virtual_int(Raw object, std::uint16_t slot, int argument) override BSP_FE5_UNREACHED
    Raw ingame_interface_00e198c4() override BSP_FE5_UNREACHED
    void front_end_screen_commit_visibility_004f83b0(Raw screen) override BSP_FE5_UNREACHED
    void* singleton_lifetime_lock_00415350() override BSP_FE5_UNREACHED
    void critical_section_enter(void* lock) override BSP_FE5_UNREACHED
    void critical_section_leave(void* lock) override BSP_FE5_UNREACHED
    void singleton_lifetime_unregister_00bcfca0(void* object) override BSP_FE5_UNREACHED
    void** singleton_cell_00f8894c() override BSP_FE5_UNREACHED
    void legacy_sbo_string_assign_whole(Raw dst, Raw src) override BSP_FE5_UNREACHED
    void* stl_allocate_bytes(std::size_t bytes) override BSP_FE5_UNREACHED
    void dword_vector_push_back_00688830(Raw vector, std::int32_t value) override BSP_FE5_UNREACHED
    void front_end_manager_push_request_004cc460(Raw manager, int interface_id) override BSP_FE5_UNREACHED
    void trace2_004254b0(const char* format, const char* a, const char* b) override BSP_FE5_UNREACHED
    bool online_virtual_bool(std::uint16_t slot) override BSP_FE5_UNREACHED
    void online_virtual_int(std::uint16_t slot, int argument) override BSP_FE5_UNREACHED
    void session_set_mode_0076fad0(int mode) override BSP_FE5_UNREACHED
    void set_byte_00e08700(std::uint8_t value) override BSP_FE5_UNREACHED
    void game_request_state_004d7920(int state) override BSP_FE5_UNREACHED
    std::uint8_t game_byte_218c() override BSP_FE5_UNREACHED
    void ingame_interface_collapse_overlays_0068ab80() override BSP_FE5_UNREACHED
    void file_block_begin(std::string_view name) override BSP_FE5_UNREACHED
    void file_block_end() override BSP_FE5_UNREACHED
    Raw multi_screen_new(std::uint32_t constructor, std::uint32_t size) override BSP_FE5_UNREACHED
    std::size_t online_error_text_size_74() override BSP_FE5_UNREACHED
    void game_end_scene_004d7970() override BSP_FE5_UNREACHED
    void multi_error_screen_set_code_005d2e70(int code) override BSP_FE5_UNREACHED
    std::uint8_t xenon_byte_128() override BSP_FE5_UNREACHED
    bool xenon_has_selected_user_00a3e510() override BSP_FE5_UNREACHED
    int xenon_selected_sign_in_state_00a3ead0() override BSP_FE5_UNREACHED
    std::int32_t session_mode_1fe4() override BSP_FE5_UNREACHED
    void front_end_manager_activate_00684700(Raw manager) override BSP_FE5_UNREACHED
    void front_end_manager_deactivate_00683aa0(Raw manager) override BSP_FE5_UNREACHED
    Raw movie_player_00e18d48() override BSP_FE5_UNREACHED
    void movie_player_play_004f8a20(std::string_view path, int flag, float z, int loop) override BSP_FE5_UNREACHED
    void movie_player_stop_004f8ac0() override BSP_FE5_UNREACHED
    void set_attract_screen_global_00e198bc(void* screen) override BSP_FE5_UNREACHED
    void gui_remove_and_destroy_page_00aa31f0(void* page) override BSP_FE5_UNREACHED
    void game_input_contexts_push_level_004d8a50(const std::int32_t contexts[6]) override BSP_FE5_UNREACHED
    void spectate_screen_engage(Raw screen, std::uint32_t routine, void* unit) override BSP_FE5_UNREACHED
    Raw spectate_screen_camera(Raw screen, std::uint32_t routine) override BSP_FE5_UNREACHED
    Raw camera_target_unit(Raw camera, std::uint32_t routine) override BSP_FE5_UNREACHED
    bool unit_is_kind_5c(Raw unit, int kind) override BSP_FE5_UNREACHED
    bool unit_selectable_for_player_00645160(Raw unit) override BSP_FE5_UNREACHED
    void hud_root_set_spectated_unit_00647300(Raw hud_root, Raw unit) override BSP_FE5_UNREACHED
    void hud_root_force_select_unit_006485a0(Raw hud_root) override BSP_FE5_UNREACHED
    Raw controlled_unit_00e188d8() override BSP_FE5_UNREACHED
    void front_end_screen_construct_004f7180(Raw screen) override BSP_FE5_UNREACHED
    void front_end_screen_destruct_004f71a0(Raw screen) override BSP_FE5_UNREACHED
    void callback_owner_destroy_00695870(Raw owner) override BSP_FE5_UNREACHED
    void observer_link_destroy_base_004b7ef0(Raw link) override BSP_FE5_UNREACHED
    void object_virtual_2(Raw object, std::uint16_t slot, std::int32_t a, std::int32_t b) override BSP_FE5_UNREACHED
    void front_end_manager_push_request_arg_004cc460(Raw manager, int interface_id, void* argument) override BSP_FE5_UNREACHED
    Raw game_scene_1ed4() override BSP_FE5_UNREACHED
    void front_end_screens_set_level2_004f85d0(const std::int32_t* screens) override BSP_FE5_UNREACHED
    void front_end_screens_set_level3_004f8670(const std::int32_t* screens) override BSP_FE5_UNREACHED
    void game_input_contexts_level2_004d8ae0(const std::int32_t* contexts) override BSP_FE5_UNREACHED
    void game_input_contexts_level3_004d8b70(const std::int32_t* contexts) override BSP_FE5_UNREACHED
    void* game_controlled_vehicle_004b4b00() override BSP_FE5_UNREACHED
    void ingame_interface_request_scene_for_unit_0068aa40(Raw iface, void* unit) override BSP_FE5_UNREACHED
    std::uint8_t byte_00e08880() override BSP_FE5_UNREACHED
    Raw sound_request_queue_004c1b90() override BSP_FE5_UNREACHED
    bool support_screen_query_006529e0(Raw screen) override BSP_FE5_UNREACHED
    std::size_t global_vector_00e18d1c_count() override BSP_FE5_UNREACHED
    void binoculars_reset_0051e8e0() override BSP_FE5_UNREACHED
    bool support_has_available_entries_0066ffa0(Raw screen) override BSP_FE5_UNREACHED
    bool support_screen_query_00652a30(Raw screen) override BSP_FE5_UNREACHED
    void support_play_unavailable_sound_0065f170(Raw screen) override BSP_FE5_UNREACHED
    void support_set_tracked_unit_target_0065be80(Raw screen, void* target) override BSP_FE5_UNREACHED
    bool ingame_interface_build_tactical_overlay_list_0068bc80(Raw iface) override BSP_FE5_UNREACHED
    std::uint8_t game_byte_19c4() override BSP_FE5_UNREACHED
    void set_title_screen_global_00e198c8(void* screen) override BSP_FE5_UNREACHED
    void storage_operation_reset_availability_00bd3450() override BSP_FE5_UNREACHED
    std::int32_t game_dword_5e8() override BSP_FE5_UNREACHED
    void set_game_byte_5ec(std::uint8_t value) override BSP_FE5_UNREACHED
    bool widget_is_visible_38(void* widget) override BSP_FE5_UNREACHED
    bool widget_is_kind_0c(void* widget, void* kind) override BSP_FE5_UNREACHED
    void* gui_widget_kind_00ab1060() override BSP_FE5_UNREACHED
    void widget_set_local_position_00aa7dc0(void* widget, const float xyz[3]) override BSP_FE5_UNREACHED
    void widget_virtual_48(void* widget, const float pair[2]) override BSP_FE5_UNREACHED
    void widget_set_image_88(void* widget, void* image, int flag, float alpha) override BSP_FE5_UNREACHED
    bool unit_is_squadron_flight_leader_007b8ad0(Raw unit) override BSP_FE5_UNREACHED
    void* object_virtual_ret(Raw object, std::uint16_t slot) override BSP_FE5_UNREACHED
    void observer_register_00694a60(void* observed, void* observer_member) override BSP_FE5_UNREACHED
    void observer_unregister_006952a0(void* observed, void* observer_member) override BSP_FE5_UNREACHED
    void native_string_resize_0041dd40(Raw string, std::size_t size, int flag) override BSP_FE5_UNREACHED
    void native_string_release(Raw string) override BSP_FE5_UNREACHED
    std::string native_string_read(Raw string) override BSP_FE5_UNREACHED
    void native_string_assign(Raw string, std::string_view value) override BSP_FE5_UNREACHED
    bool award_stored_hint_contains_007f8e00(const std::string& name) override BSP_FE5_UNREACHED
    Raw award_hint_definition_005ffb20(Raw definitions, const std::string& name) override BSP_FE5_UNREACHED
    void award_hint_map_assign_00690fb0(Raw map, std::uint32_t key, std::uint32_t value) override BSP_FE5_UNREACHED
    void award_tracker_trigger_hint_forced_00690fd0(Raw tracker, std::uint32_t name, int forced) override BSP_FE5_UNREACHED
    void pooled_string_list_clear_004d05e0(Raw list) override BSP_FE5_UNREACHED
    void alias_list_erase_node(Raw list, void* node) override BSP_FE5_UNREACHED
    void alias_list_push_back_copy(Raw list, const void* value) override BSP_FE5_UNREACHED
    void* operator_new(std::size_t bytes) override BSP_FE5_UNREACHED
    void* observer_lock_00694280() override BSP_FE5_UNREACHED
    Raw observer_find_pair_006949d0(void* observed, void* observer) override BSP_FE5_UNREACHED
    bool observer_is_registered_00694af0(void* observed, void* observer) override BSP_FE5_UNREACHED
    void observer_edge_array_erase_00694f60(Raw edges, void* pair) override BSP_FE5_UNREACHED
    void callback_owner_detach_all_00695530(Raw owner) override BSP_FE5_UNREACHED
    void observer_purge_00695c50(int budget) override BSP_FE5_UNREACHED
    void critical_section_destroy_owned_0041cc80(Raw cell) override BSP_FE5_UNREACHED
    void set_observer_singleton_00e198e0(void* value) override BSP_FE5_UNREACHED
    void crt_memmove_s(void* dst, std::size_t dst_size, const void* src, std::size_t count) override BSP_FE5_UNREACHED
    void game_update_view_0078cff0(void* view, float clock) override BSP_FE5_UNREACHED
    void scene_renderer_submit_00b101c0(void* scene, void* view) override BSP_FE5_UNREACHED
    void game_after_render_submit_004c7e60() override BSP_FE5_UNREACHED
    Raw game_00e188a8() override BSP_FE5_UNREACHED
    void unit_send_role_transfer_0077c470(Raw unit, std::uint32_t mask, int take) override BSP_FE5_UNREACHED
    void game_set_camera_near_plane_004b4940(float near_plane) override BSP_FE5_UNREACHED
    float real_clock_00f876a4() override BSP_FE5_UNREACHED
    std::int32_t plane_screen_payload_frame_00604c80(Raw screen) override BSP_FE5_UNREACHED
    void widget_set_frame_88(void* widget, std::int32_t frame, int flag, float alpha) override BSP_FE5_UNREACHED
    void refcounted_release(void* object) override BSP_FE5_UNREACHED
    void widget_set_rotation_44(void* widget, float radians) override BSP_FE5_UNREACHED
    double crt_fmod_00bf857a(double x, double y) override BSP_FE5_UNREACHED
    void game_set_view_fov_004dc940(float fov, int flag) override BSP_FE5_UNREACHED
    void point_effect_stop_children_00867b10(Raw effect) override BSP_FE5_UNREACHED
    void render_set_transition_00b0d6a0(float a, float b, float c, float x, float y, float z) override BSP_FE5_UNREACHED
    Raw plane_chase_camera_new_00605130() override BSP_FE5_UNREACHED
    void camera_add_to_scene_00923870(Raw camera, void* scene, const float matrix[16]) override BSP_FE5_UNREACHED
    void camera_set_target_0042aaf0(Raw camera, void* target) override BSP_FE5_UNREACHED
    void game_set_camera_mover_004bc410(void* camera) override BSP_FE5_UNREACHED
    void camera_virtual_dc(Raw camera, float value) override BSP_FE5_UNREACHED
    Raw input_manager_004bec00() override BSP_FE5_UNREACHED
    Raw game_tuning_0042e740() override BSP_FE5_UNREACHED
    void* effect_handle_acquire_00871ba0(std::string_view name) override BSP_FE5_UNREACHED
    const float* widget_local_position_00aa67f0(void* widget) override BSP_FE5_UNREACHED
    void markers_screen_set_view_centre_00637620(float x, float y) override BSP_FE5_UNREACHED
    bool bit_flag_set_test_0048f930(Raw flags, std::uint32_t flag) override BSP_FE5_UNREACHED
    void widget_quad_fade_construct_0064a4f0(Raw fade) override BSP_FE5_UNREACHED
    void* air_ops_block_006bcd20(Raw object) override BSP_FE5_UNREACHED
    void camera_mover_construct_base_00432750(Raw camera) override BSP_FE5_UNREACHED
    void render_get_transition_00b0cfc0(float& a, float& b, float& c) override BSP_FE5_UNREACHED
    void cockpit_camera_reset_0042aa00(Raw camera) override BSP_FE5_UNREACHED
    void camera_orbit_target_from_yaw_0042c940(Raw camera, float yaw) override BSP_FE5_UNREACHED
    Raw static_colour_block_00e192f4() override BSP_FE5_UNREACHED
    void widget_delete_timed_entry_00aa8b80(void* widget, int index) override BSP_FE5_UNREACHED
    void refptr_array_resize_005471b0(Raw array, std::int32_t count) override BSP_FE5_UNREACHED
    void weapon_group_step_00548410(Raw screen, int forward) override BSP_FE5_UNREACHED
    bool weapon_group_any_available_00545b30(Raw screen, std::int32_t group) override BSP_FE5_UNREACHED
    Raw manager_global(std::uint32_t address) override BSP_FE5_UNREACHED
    std::int32_t widget_kind_5c(void* widget) override BSP_FE5_UNREACHED
    void widget_press_84(void* widget, int flag) override BSP_FE5_UNREACHED
    void listener_forward(Raw listener, std::uint16_t slot, void* widget) override BSP_FE5_UNREACHED
    void listener_forward2(Raw listener, std::uint16_t slot, void* widget, std::uint32_t flag) override BSP_FE5_UNREACHED
    void native_string_append_byte(Raw string, std::uint8_t value) override BSP_FE5_UNREACHED
    std::uint32_t xenon_selected_user_index_00a3eac0() override BSP_FE5_UNREACHED
    void x_show_keyboard_ui_00a4d43a(std::uint32_t user, std::uint32_t flags, const void* default_text, const void* title, const void* description, void* result, std::uint32_t result_chars, void* overlapped) override BSP_FE5_UNREACHED
    void scroller_set_auto_scroll_00683300(Raw scroller, int direction, float step, bool override_input) override BSP_FE5_UNREACHED
    void scroller_end_thumb_drag(Raw scroller) override BSP_FE5_UNREACHED
    void widget_set_state_88(void* widget, bool on, int flag, float alpha) override BSP_FE5_UNREACHED
    void tree_iterator_increment(Raw iterator, std::uint32_t routine) override BSP_FE5_UNREACHED
    void native_string_construct_copy_0044bcb0(Raw slot, Raw src) override BSP_FE5_UNREACHED
    void widget_set_local_y_00aa78f0(void* widget, float y) override BSP_FE5_UNREACHED
    void* hud_widget_set_new_0043dff0(std::uint32_t a, std::uint32_t b, std::uint32_t c) override BSP_FE5_UNREACHED
    void* hud_widget_set_new_0043df90(std::uint32_t a, std::uint32_t b) override BSP_FE5_UNREACHED
    void text_set_localised_source_00abaed0(void* text, Raw source) override BSP_FE5_UNREACHED
    void widget_resolved_position_00aa6750(void* widget, float xyz[3]) override BSP_FE5_UNREACHED
    const float* widget_size_00aa6740(void* widget) override BSP_FE5_UNREACHED
    void widget_set_resolved_position_00aa8240(void* widget, const float xyz[3]) override BSP_FE5_UNREACHED
    void widget_set_size_58(void* widget, const float size[2]) override BSP_FE5_UNREACHED
    void* effect_handle_acquire_id_00870cd0(std::uint32_t id) override BSP_FE5_UNREACHED
    void* gui_widget_clone_subtree_00aab4c0(void* widget, int flag) override BSP_FE5_UNREACHED
    void widget_vector_insert_one(Raw vector, std::uint32_t routine, void* where, void* value) override BSP_FE5_UNREACHED
    bool front_end_action_edge_or_repeat_004d92b0(int action) override BSP_FE5_UNREACHED
    bool input_action_held_004c5090(int action) override BSP_FE5_UNREACHED
    std::int32_t listbox_selected_index_00a9c920(void* listbox) override BSP_FE5_UNREACHED
    void* lua_globals_00b67980() override BSP_FE5_UNREACHED
    void* lua_get_by_name_00b67800(void* table, const char* name) override BSP_FE5_UNREACHED
    void* lua_get_by_index_00b67720(void* table, std::int32_t index) override BSP_FE5_UNREACHED
    void* lua_nil_00b65f50() override BSP_FE5_UNREACHED
    void lua_assign_00b67690(void* dst, void* src) override BSP_FE5_UNREACHED
    bool lua_is_table_00b661b0(void* object) override BSP_FE5_UNREACHED
    bool lua_is_string_00b660a0(void* object) override BSP_FE5_UNREACHED
    void lua_release_00b67700(void* object) override BSP_FE5_UNREACHED
    void unit_library_reset_selection_0067e580(Raw screen) override BSP_FE5_UNREACHED
    std::uint8_t* idle_camera_shot_allocate_0054eef0(std::size_t count) override BSP_FE5_UNREACHED
    void stl_xlen_00550b50() override BSP_FE5_UNREACHED
    Raw plane_cockpit_camera_new_006065d0() override BSP_FE5_UNREACHED
    const float* camera_position_0042cd00(Raw camera, float out[3]) override BSP_FE5_UNREACHED
    void cockpit_camera_set_position_0042e030(Raw camera, const float* position) override BSP_FE5_UNREACHED
    const float* unit_local_to_world_18(Raw unit, const float local[3], float out[3]) override BSP_FE5_UNREACHED
    std::uint32_t unit_virtual_210(Raw unit, std::int32_t a, std::int32_t b) override BSP_FE5_UNREACHED
    std::uint32_t plane_level_bomb_attitude_gate_007c7750(Raw plane, std::int32_t arg) override BSP_FE5_UNREACHED
    std::uint32_t plane_drop_attitude_gate_007cc8e0(Raw plane, std::int32_t arg) override BSP_FE5_UNREACHED
    void widget_set_quad_floats_00abe6e0(void* widget, float a, float b, float c, float d) override BSP_FE5_UNREACHED
    void quad_fade_update_0064abd0(Raw fade, float dt) override BSP_FE5_UNREACHED
    void entity_pose_refresh_00414db0(Raw unit) override BSP_FE5_UNREACHED
    void digit_gauge_update_0043b370(void* gauge, float value, float dt) override BSP_FE5_UNREACHED
    std::int32_t unit_count_remaining_ordnance_rounds_007c1db0(Raw plane) override BSP_FE5_UNREACHED
    double unit_virtual_float(Raw unit, std::uint16_t slot) override BSP_FE5_UNREACHED
    bool object_virtual_al(Raw object, std::uint16_t slot) override BSP_FE5_UNREACHED
    std::uint8_t byte_00f88988() override BSP_FE5_UNREACHED
    float float_00f8757c() override BSP_FE5_UNREACHED
    Raw static_needle_span_block_00e1968c() override BSP_FE5_UNREACHED
    void matrix_rotation_z_00b64780(float out[16], float angle) override BSP_FE5_UNREACHED
    void matrix_rotation_x_00b64640(float out[16], float angle) override BSP_FE5_UNREACHED
    void matrix_rotation_y_00b646e0(float out[16], float angle) override BSP_FE5_UNREACHED
    void matrix_multiply_00413920(const float a[16], const float b[16], float out[16]) override BSP_FE5_UNREACHED
    void node_set_matrix_38(Raw node, const float matrix[16]) override BSP_FE5_UNREACHED
    const float* unit_world_position_00427eb0(Raw unit) override BSP_FE5_UNREACHED
    void vector_subtract_00427c90(const float* a, float out[3], const float* b) override BSP_FE5_UNREACHED
    double vector_length_squared_00427e30(const float v[3]) override BSP_FE5_UNREACHED
    std::int32_t recon_relation_if_identified_00803ce0(Raw recon, Raw unit) override BSP_FE5_UNREACHED
    std::int32_t camera_project_world_to_screen_0043a660(const float world[3], float out[3], int a, int b) override BSP_FE5_UNREACHED
    void widget_set_local_xy_00aa7d00(void* widget, float x, float y) override BSP_FE5_UNREACHED
    bool plane_weapon_query(Raw plane, std::uint32_t routine) override BSP_FE5_UNREACHED
    const float* plane_ordnance_point(Raw plane, std::uint32_t routine, float out[3]) override BSP_FE5_UNREACHED
    bool plane_can_release_ordnance_007bb110(Raw plane) override BSP_FE5_UNREACHED
    void plane_screen_pulse_alpha_00604cf0(Raw screen) override BSP_FE5_UNREACHED
    double vector_length_0042b2f0(const float v[3]) override BSP_FE5_UNREACHED
    double clamped_interpolate_00419010(float x0, float y0, float x1, float y1, float x) override BSP_FE5_UNREACHED
    double random_uniform_00bd2f10(int stream, float lo, float hi) override BSP_FE5_UNREACHED
    void scene_apply_0042c450(void* scene, float amount) override BSP_FE5_UNREACHED
    void quad_fade_bind_0064a540(Raw fade, void* top, void* bottom, void* left, void* right) override BSP_FE5_UNREACHED
    bool lua_is_integer_00b66a60(void* object) override BSP_FE5_UNREACHED
    std::int32_t lua_get_integer_00b66290(void* object) override BSP_FE5_UNREACHED
    const char* lua_get_string_00b662b0(void* object) override BSP_FE5_UNREACHED
    double lua_get_number_00b66270(void* object) override BSP_FE5_UNREACHED
    const float* lua_get_vector2_00b679f0(void* object, float out[2]) override BSP_FE5_UNREACHED
    void lua_iterate_first_00b67080(void* table, void* key, void* value) override BSP_FE5_UNREACHED
    void lua_iterate_next_00b67190(void* table, void* key, void* value) override BSP_FE5_UNREACHED
    bool lua_iterate_ended_00b66420(void* table, void* key) override BSP_FE5_UNREACHED
    void unit_library_release_model_parts_0067ed30(Raw screen) override BSP_FE5_UNREACHED
    void* resource_load_with_game_factory_007188a0(std::string_view name) override BSP_FE5_UNREACHED
    Raw unit_part_instance_new_007135c0(void* model) override BSP_FE5_UNREACHED
    void node_propagate_root_registration_00b6d890(Raw node, void* root) override BSP_FE5_UNREACHED
    void node_set_parent_00b6e680(Raw node, void* parent) override BSP_FE5_UNREACHED
    void node_set_matrix_34(Raw node, const float matrix[16]) override BSP_FE5_UNREACHED
    void model_groups_select_visible_00710bb0(Raw part, std::int32_t group) override BSP_FE5_UNREACHED
    void* model_find_note_node_0071ad50(void* model, std::string_view name) override BSP_FE5_UNREACHED
    const float* matrix_rotation_y_negated_0042b490(float out[16], float angle) override BSP_FE5_UNREACHED
    const float* matrix_rotation_x_negated_0042b450(float out[16], float angle) override BSP_FE5_UNREACHED
    void* resource_find_last_named_point_group_00718000(void* model, std::string_view name, std::int32_t index) override BSP_FE5_UNREACHED
    const float* point_vector_at_004803a0(Raw vector, std::int32_t index) override BSP_FE5_UNREACHED
    void vector_cross_004fa230(float out[3], const float* edx_operand, const float* stack_operand) override BSP_FE5_UNREACHED
    void matrix_orthonormalize_basis_rows_0085dc80(float matrix[16]) override BSP_FE5_UNREACHED
    void unit_library_model_vector_push_00680cc0(Raw vector, void* model) override BSP_FE5_UNREACHED
    void unit_library_part_vector_push_00680d30(Raw vector, void* part) override BSP_FE5_UNREACHED
};
#pragma warning(pop)
#undef BSP_FE5_UNREACHED

}  // namespace bsp::cc13_fe5
