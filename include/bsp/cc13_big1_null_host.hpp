#pragma once
// A Big1Host whose every hook records that it was reached and answers zero
// (null, false, 0). Generated from include/bsp/cc13_big1.hpp; an adapter
// derives from it and overrides only the hooks its body reaches, so a hook a
// body was not expected to reach shows up in unreached() instead of running.
// Not a reconstruction: no hook here models the image.

#include "bsp/cc13_big1.hpp"

#include <cstdint>

namespace bsp::cc13_big1 {

class Big1NullHost : public Big1Host {
public:
    // Called with the hook's name and the address its name carries (0 when
    // the name has none) whenever a hook that was not overridden runs.
    virtual void unreached(const char* hook, std::uint32_t address) = 0;

    void* world_find_closest_border_zone_004c7730(
        void*, const float*, std::int32_t, float*, float*) override {
        unreached("world_find_closest_border_zone_004c7730", 0x004c7730u); return {};
    }
    const float* vector3f_normalize_00419510(float*, const float*) override {
        unreached("vector3f_normalize_00419510", 0x00419510u); return {};
    }
    void crt_invalid_parameter_noinfo_00bf6713() override {
        unreached("crt_invalid_parameter_noinfo_00bf6713", 0x00bf6713u);
    }
    void crt_free_00bf65ac(void*) override { unreached("crt_free_00bf65ac", 0x00bf65acu); }
    void* set_buy_head_node_005a0300(void*) override {
        unreached("set_buy_head_node_005a0300", 0x005a0300u); return {};
    }
    void set_erase_subtree_0059fe60(void*, void*) override {
        unreached("set_erase_subtree_0059fe60", 0x0059fe60u);
    }
    void* set_find_005a18e0(void*, void*, const std::uint32_t*) override {
        unreached("set_find_005a18e0", 0x005a18e0u); return {};
    }
    void set_insert_005a59c0(void*, void*, const std::uint32_t*) override {
        unreached("set_insert_005a59c0", 0x005a59c0u);
    }
    void set_iterator_increment_0059f0e0(void*) override {
        unreached("set_iterator_increment_0059f0e0", 0x0059f0e0u);
    }
    void set_erase_range_005a6420(void*, void*, void*, void*, void*, void*) override {
        unreached("set_erase_range_005a6420", 0x005a6420u);
    }
    void list_copy_005aaaf0(void*, const void*) override {
        unreached("list_copy_005aaaf0", 0x005aaaf0u);
    }
    void support_site_build_supply_class_list_0084d560(
        void*, void*, std::int32_t, std::int32_t) override {
        unreached("support_site_build_supply_class_list_0084d560", 0x0084d560u);
    }
    void* vehicle_class_get_or_create_00964790(void*, bool) override {
        unreached("vehicle_class_get_or_create_00964790", 0x00964790u); return {};
    }
    std::uint32_t unit_hud_icon_index_0059cb90(void*, void*) override {
        unreached("unit_hud_icon_index_0059cb90", 0x0059cb90u); return {};
    }
    bool unit_has_flag_vtable5c(void*, std::int32_t) override {
        unreached("unit_has_flag_vtable5c", 0u); return {};
    }
    void* gui_widget_clone_subtree_00aab4c0(void*, void*) override {
        unreached("gui_widget_clone_subtree_00aab4c0", 0x00aab4c0u); return {};
    }
    void widget_vtable34(void*, std::uint8_t) override { unreached("widget_vtable34", 0u); }
    void widget_vtable88(void*, std::uint32_t, std::int32_t, float) override {
        unreached("widget_vtable88", 0u);
    }
    void vector_push_back_0043dca0(void*, void* const*) override {
        unreached("vector_push_back_0043dca0", 0x0043dca0u);
    }
    void* game_00e188a8() override { unreached("game_00e188a8", 0x00e188a8u); return {}; }
    void* lua_state_owner_get_globals_00b67980(void*, void*) override {
        unreached("lua_state_owner_get_globals_00b67980", 0x00b67980u); return {};
    }
    void* lua_object_get_by_name_00b67800(void*, void*, const char*) override {
        unreached("lua_object_get_by_name_00b67800", 0x00b67800u); return {};
    }
    void* lua_object_get_by_index_00b67720(void*, void*, std::int32_t) override {
        unreached("lua_object_get_by_index_00b67720", 0x00b67720u); return {};
    }
    void lua_object_construct_00b65f50(void*) override {
        unreached("lua_object_construct_00b65f50", 0x00b65f50u);
    }
    void lua_object_assign_00b67690(void*, void*) override {
        unreached("lua_object_assign_00b67690", 0x00b67690u);
    }
    void lua_object_destruct_00b67700(void*) override {
        unreached("lua_object_destruct_00b67700", 0x00b67700u);
    }
    bool lua_object_is_boolean_00b66000(void*) override {
        unreached("lua_object_is_boolean_00b66000", 0x00b66000u); return {};
    }
    std::uint8_t lua_object_get_boolean_00b66250(void*) override {
        unreached("lua_object_get_boolean_00b66250", 0x00b66250u); return {};
    }
    float hud_ship_screen45_update_dir_icon_0064aad0(void*) override {
        unreached("hud_ship_screen45_update_dir_icon_0064aad0", 0x0064aad0u); return {};
    }
    void hud_ship_screen45_update_throttle_indicator_0064a9f0(void*, float) override {
        unreached("hud_ship_screen45_update_throttle_indicator_0064a9f0", 0x0064a9f0u);
    }
    void hud_ship_screen45_update_formation_icon_0064a960(void*) override {
        unreached("hud_ship_screen45_update_formation_icon_0064a960", 0x0064a960u);
    }
    std::int32_t unit_torpedo_barrel_count_00818300(void*) override {
        unreached("unit_torpedo_barrel_count_00818300", 0x00818300u); return {};
    }
    std::int32_t unit_count_bomb_platform_children_00852350(void*) override {
        unreached("unit_count_bomb_platform_children_00852350", 0x00852350u); return {};
    }
    void lua_state_owner_construct_00b66bd0(void*) override {
        unreached("lua_state_owner_construct_00b66bd0", 0x00b66bd0u);
    }
    void lua_state_owner_open_00b6a020(void*, std::int32_t) override {
        unreached("lua_state_owner_open_00b6a020", 0x00b6a020u);
    }
    void lua_state_owner_run_script_00b69d40(void*, const void*, std::int32_t) override {
        unreached("lua_state_owner_run_script_00b69d40", 0x00b69d40u);
    }
    void lua_state_owner_close_00b669a0(void*) override {
        unreached("lua_state_owner_close_00b669a0", 0x00b669a0u);
    }
    bool lua_object_is_nil_00b65fb0(void*) override {
        unreached("lua_object_is_nil_00b65fb0", 0x00b65fb0u); return {};
    }
    std::int32_t lua_object_get_integer_00b66290(void*) override {
        unreached("lua_object_get_integer_00b66290", 0x00b66290u); return {};
    }
    const char* lua_object_get_string_00b662b0(void*) override {
        unreached("lua_object_get_string_00b662b0", 0x00b662b0u); return {};
    }
    void native_string_resize_0041dd40(void*, std::uint32_t, std::int32_t) override {
        unreached("native_string_resize_0041dd40", 0x0041dd40u);
    }
    void crt_memcpy_00bf7680(void*, const void*, std::uint32_t) override {
        unreached("crt_memcpy_00bf7680", 0x00bf7680u);
    }
    void* sized_pool_get_00419cc0() override {
        unreached("sized_pool_get_00419cc0", 0x00419cc0u); return {};
    }
    void sized_pool_return_block_00bd1510(void*, void*, std::uint32_t, std::int32_t) override {
        unreached("sized_pool_return_block_00bd1510", 0x00bd1510u);
    }
    void weather_entry_vector_push_back_005e11f0(void*, const void*) override {
        unreached("weather_entry_vector_push_back_005e11f0", 0x005e11f0u);
    }
    void lobby_map_entry_destruct_005daca0(void*) override {
        unreached("lobby_map_entry_destruct_005daca0", 0x005daca0u);
    }
    void native_string_vector_append_00450540(void*, const void*) override {
        unreached("native_string_vector_append_00450540", 0x00450540u);
    }
    void list_tidy_005a0b80(void*) override { unreached("list_tidy_005a0b80", 0x005a0b80u); }
    void* support_tree_find_0065b160(void*, void*, const std::uint32_t*) override {
        unreached("support_tree_find_0065b160", 0x0065b160u); return {};
    }
    void support_tree_iterator_increment_00654f20(void*) override {
        unreached("support_tree_iterator_increment_00654f20", 0x00654f20u);
    }
    void support_tree_erase_subtree_00659400(void*, void*) override {
        unreached("support_tree_erase_subtree_00659400", 0x00659400u);
    }
    void support_tree_erase_range_006641b0(void*, void*, void*, void*, void*, void*) override {
        unreached("support_tree_erase_range_006641b0", 0x006641b0u);
    }
    void support_tree_copy_0065ce20(void*, const void*) override {
        unreached("support_tree_copy_0065ce20", 0x0065ce20u);
    }
    void* support_list_buy_node_00659aa0(void*, void*, void*, const void*) override {
        unreached("support_list_buy_node_00659aa0", 0x00659aa0u); return {};
    }
    void support_list_incsize_00662a70(void*, std::uint32_t) override {
        unreached("support_list_incsize_00662a70", 0x00662a70u);
    }
    void object_vtable198(void*, const void*, std::int32_t, std::uint8_t*) override {
        unreached("object_vtable198", 0u);
    }
    void* unit_list_buy_head_node_004c1630(void*) override {
        unreached("unit_list_buy_head_node_004c1630", 0x004c1630u); return {};
    }
    void* unit_list_buy_node_004c2220(void*, void*, void*, const void*) override {
        unreached("unit_list_buy_node_004c2220", 0x004c2220u); return {};
    }
    void unit_list_incsize_004cee30(void*, std::uint32_t) override {
        unreached("unit_list_incsize_004cee30", 0x004cee30u);
    }
    void unit_list_assign_006681d0(void*, void*, void*, void*, void*, std::uint32_t) override {
        unreached("unit_list_assign_006681d0", 0x006681d0u);
    }
    void* entry_list_buy_node_00659d40(void*, void*, void*, const void*) override {
        unreached("entry_list_buy_node_00659d40", 0x00659d40u); return {};
    }
    void entry_list_incsize_006632d0(void*, std::uint32_t) override {
        unreached("entry_list_incsize_006632d0", 0x006632d0u);
    }
    void entry_list_push_front_006652c0(void*, const void*) override {
        unreached("entry_list_push_front_006652c0", 0x006652c0u);
    }
    void entry_list_assign_00668350(void*, void*, void*, void*, void*, std::uint32_t) override {
        unreached("entry_list_assign_00668350", 0x00668350u);
    }
    std::int32_t unit_loss_counting_slot_009ffd20(void*) override {
        unreached("unit_loss_counting_slot_009ffd20", 0x009ffd20u); return {};
    }
    bool object_vtable18(void*, std::int32_t) override {
        unreached("object_vtable18", 0u); return {};
    }
    void* scene_property_bag_find_008f2260(void*, const char*) override {
        unreached("scene_property_bag_find_008f2260", 0x008f2260u); return {};
    }
    std::int32_t game_effective_mode_004bca50(void*) override {
        unreached("game_effective_mode_004bca50", 0x004bca50u); return {};
    }
    void* global_00f89b3c() override { unreached("global_00f89b3c", 0x00f89b3cu); return {}; }
    void* hud_00e198c4() override { unreached("hud_00e198c4", 0x00e198c4u); return {}; }
    void* global_00e188d8() override { unreached("global_00e188d8", 0x00e188d8u); return {}; }
    void hud_counters_relayout_00529a30(void*) override {
        unreached("hud_counters_relayout_00529a30", 0x00529a30u);
    }
    void hud_counters_hide_under_manager_panel_00528f70(void*) override {
        unreached("hud_counters_hide_under_manager_panel_00528f70", 0x00528f70u);
    }
    void powerup_screen_refresh_prompt_widgets_00612da0(void*) override {
        unreached("powerup_screen_refresh_prompt_widgets_00612da0", 0x00612da0u);
    }
    void support_layout_widget_1c0_006567d0(void*) override {
        unreached("support_layout_widget_1c0_006567d0", 0x006567d0u);
    }
    void support_delete_slot_clones_00652a60(void*) override {
        unreached("support_delete_slot_clones_00652a60", 0x00652a60u);
    }
    void support_rebind_slots_0065d220(void*) override {
        unreached("support_rebind_slots_0065d220", 0x0065d220u);
    }
    void support_refresh_sites_0066fb00(void*) override {
        unreached("support_refresh_sites_0066fb00", 0x0066fb00u);
    }
    void support_rebuild_00668770(void*) override {
        unreached("support_rebuild_00668770", 0x00668770u);
    }
    void support_sync_cursor_iterators_0065a2b0(void*) override {
        unreached("support_sync_cursor_iterators_0065a2b0", 0x0065a2b0u);
    }
    void support_build_entries_0066a130(void*) override {
        unreached("support_build_entries_0066a130", 0x0066a130u);
    }
    void support_layout_name_label_0065bbc0(void*) override {
        unreached("support_layout_name_label_0065bbc0", 0x0065bbc0u);
    }
    void support_refresh_selection_0066d020(void*) override {
        unreached("support_refresh_selection_0066d020", 0x0066d020u);
    }
    void support_select_weapon_value_00669170(void*) override {
        unreached("support_select_weapon_value_00669170", 0x00669170u);
    }
    void support_rebuild_lists_0066a8a0(void*) override {
        unreached("support_rebuild_lists_0066a8a0", 0x0066a8a0u);
    }
    void support_rebuild_00670020(void*) override {
        unreached("support_rebuild_00670020", 0x00670020u);
    }
    void support_play_open_sound_0065f950(void*) override {
        unreached("support_play_open_sound_0065f950", 0x0065f950u);
    }
    void support_set_tracked_unit_target_0065be80(void*, void*) override {
        unreached("support_set_tracked_unit_target_0065be80", 0x0065be80u);
    }
    void* award_tracker_get_singleton_004e1ca0() override {
        unreached("award_tracker_get_singleton_004e1ca0", 0x004e1ca0u); return {};
    }
    bool award_tracker_has_hint_been_shown_0068e600(void*, const void*) override {
        unreached("award_tracker_has_hint_been_shown_0068e600", 0x0068e600u); return {};
    }
    void award_tracker_trigger_hint_00690fd0(void*, const void*, std::int32_t) override {
        unreached("award_tracker_trigger_hint_00690fd0", 0x00690fd0u);
    }
    void* native_string_assign_0041e870(void*, const char*) override {
        unreached("native_string_assign_0041e870", 0x0041e870u); return {};
    }
    void native_string_destroy_0041dd20(void*) override {
        unreached("native_string_destroy_0041dd20", 0x0041dd20u);
    }
    void* native_string_from_int_004260b0(void*, std::int32_t) override {
        unreached("native_string_from_int_004260b0", 0x004260b0u); return {};
    }
    void* native_string_concat_004261a0(const void*, void*, const void*) override {
        unreached("native_string_concat_004261a0", 0x004261a0u); return {};
    }
    void* native_string_concat_int_004263b0(const void*, void*, std::int32_t) override {
        unreached("native_string_concat_int_004263b0", 0x004263b0u); return {};
    }
    void support_list_assign_00665d20(void*, void*, void*, void*, void*, std::uint32_t) override {
        unreached("support_list_assign_00665d20", 0x00665d20u);
    }
    void gui_text_set_localised_cstring_00abbe50(void*, const char*, std::int32_t) override {
        unreached("gui_text_set_localised_cstring_00abbe50", 0x00abbe50u);
    }
    void gui_text_set_localised_source_00abaed0(void*, const void*, std::int32_t) override {
        unreached("gui_text_set_localised_source_00abaed0", 0x00abaed0u);
    }
    const char* object_vtable14_name(void*) override {
        unreached("object_vtable14_name", 0u); return {};
    }
    void** support_unit_iterator_value_00658a60(void*) override {
        unreached("support_unit_iterator_value_00658a60", 0x00658a60u); return {};
    }
    void* support_map_iterator_value_00658b50(void*) override {
        unreached("support_map_iterator_value_00658b50", 0x00658b50u); return {};
    }
    void** list_iterator_value_0059fb60(void*) override {
        unreached("list_iterator_value_0059fb60", 0x0059fb60u); return {};
    }
    void list_clear_004e7a40(void*) override { unreached("list_clear_004e7a40", 0x004e7a40u); }
    void* support_map_find_0065b0f0(void*, void*, const void*) override {
        unreached("support_map_find_0065b0f0", 0x0065b0f0u); return {};
    }
    void* support_map_subscript_00667d60(void*, const void*) override {
        unreached("support_map_subscript_00667d60", 0x00667d60u); return {};
    }
    void* object_class_device_set_009552e0(void*, std::uint32_t) override {
        unreached("object_class_device_set_009552e0", 0x009552e0u); return {};
    }
    void* device_class_resolve_00443490(std::uint32_t, std::uint8_t) override {
        unreached("device_class_resolve_00443490", 0x00443490u); return {};
    }
    void gui_set_state_texture_uv_00ab2690(void*, std::uint32_t, void*, const void*) override {
        unreached("gui_set_state_texture_uv_00ab2690", 0x00ab2690u);
    }
    double player_support_points_00946f00(std::uint32_t) override {
        unreached("player_support_points_00946f00", 0x00946f00u); return {};
    }
    std::int32_t shipyard_count_idle_slots_008434d0(void*) override {
        unreached("shipyard_count_idle_slots_008434d0", 0x008434d0u); return {};
    }
    std::int32_t air_ops_count_idle_slots_006bdc30(void*) override {
        unreached("air_ops_count_idle_slots_006bdc30", 0x006bdc30u); return {};
    }
    void frontend_screen_register_004f71d0(void*) override {
        unreached("frontend_screen_register_004f71d0", 0x004f71d0u);
    }
    void* gui_manager_get_or_create_004c12b0() override {
        unreached("gui_manager_get_or_create_004c12b0", 0x004c12b0u); return {};
    }
    void* gui_manager_load_page_00aa5840(void*, const void*, std::int32_t, std::int32_t) override {
        unreached("gui_manager_load_page_00aa5840", 0x00aa5840u); return {};
    }
    void screen_vtable14_layout(void*) override { unreached("screen_vtable14_layout", 0u); }
    std::int32_t gui_icon_add_state_00ab64c0(void*, void*, const float*) override {
        unreached("gui_icon_add_state_00ab64c0", 0x00ab64c0u); return {};
    }
    const float* gui_widget_get_size_00aa6740(void*) override {
        unreached("gui_widget_get_size_00aa6740", 0x00aa6740u); return {};
    }
    const float* gui_widget_get_resolved_position_00aa6750(void*, float*) override {
        unreached("gui_widget_get_resolved_position_00aa6750", 0x00aa6750u); return {};
    }
    void* global_00f8bbe8() override { unreached("global_00f8bbe8", 0x00f8bbe8u); return {}; }
    void** sound_sample_cache_acquire_00a83fd0(void*, void**, const void*) override {
        unreached("sound_sample_cache_acquire_00a83fd0", 0x00a83fd0u); return {};
    }
    std::int32_t interlocked_increment_00ce221c(std::int32_t*) override {
        unreached("interlocked_increment_00ce221c", 0x00ce221cu); return {};
    }
    std::int32_t interlocked_decrement_00ce2220(std::int32_t*) override {
        unreached("interlocked_decrement_00ce2220", 0x00ce2220u); return {};
    }
    void refcounted_vtable0_release(void*) override { unreached("refcounted_vtable0_release", 0u); }
    std::int32_t native_string_rfind_any_004bcb80(void*, const char*, std::uint32_t) override {
        unreached("native_string_rfind_any_004bcb80", 0x004bcb80u); return {};
    }
    void* native_string_substring_00469840(
        const void*, void*, std::uint32_t, std::uint32_t) override {
        unreached("native_string_substring_00469840", 0x00469840u); return {};
    }
    void native_string_append_00425e10(void*, const void*) override {
        unreached("native_string_append_00425e10", 0x00425e10u);
    }
    void* native_string_prefixed_0043c130(void*, const char*, const void*) override {
        unreached("native_string_prefixed_0043c130", 0x0043c130u); return {};
    }
    void gui_set_state_texture_00ab44b0(void*, std::uint32_t, const void*, const float*) override {
        unreached("gui_set_state_texture_00ab44b0", 0x00ab44b0u);
    }
    void widget_vtable58_set_size(void*, const float*) override {
        unreached("widget_vtable58_set_size", 0u);
    }
    void widget_vtable8c(void*) override { unreached("widget_vtable8c", 0u); }
    void support_tree_erase_0066cc80(void*, void*) override {
        unreached("support_tree_erase_0066cc80", 0x0066cc80u);
    }
    void observer_unregister_pair_006952a0(void*, void*) override {
        unreached("observer_unregister_pair_006952a0", 0x006952a0u);
    }
    void support_sites_build_with_icons_0066df30(void*, float*) override {
        unreached("support_sites_build_with_icons_0066df30", 0x0066df30u);
    }
    void aabb_include_point_00427d10(float*, const float*) override {
        unreached("aabb_include_point_00427d10", 0x00427d10u);
    }
    bool aabb_overlaps_0085bf20(const float*, const float*, const float*, const float*) override {
        unreached("aabb_overlaps_0085bf20", 0x0085bf20u); return {};
    }
    void group_iterator_increment_00427fe0(void*) override {
        unreached("group_iterator_increment_00427fe0", 0x00427fe0u);
    }
    void entity_pose_refresh_world_00414db0(void*) override {
        unreached("entity_pose_refresh_world_00414db0", 0x00414db0u);
    }
    const float* matrix_multiply_00413920(const void*, float*, const void*) override {
        unreached("matrix_multiply_00413920", 0x00413920u); return {};
    }
    bool segment_intersects_sphere_00428090(
        const float*, const float*, float, const float*) override {
        unreached("segment_intersects_sphere_00428090", 0x00428090u); return {};
    }
    void troop_member_kill_00480bf0(void*) override {
        unreached("troop_member_kill_00480bf0", 0x00480bf0u);
    }
    void landing_group_apply_damage_00480c20(void*, float) override {
        unreached("landing_group_apply_damage_00480c20", 0x00480c20u);
    }
    std::uint32_t random_threads_next_u32_00bd2fc0(std::uint32_t) override {
        unreached("random_threads_next_u32_00bd2fc0", 0x00bd2fc0u); return {};
    }
    void soldier_start_animation_004b0f80(void*, const void*, std::int32_t, std::int32_t) override {
        unreached("soldier_start_animation_004b0f80", 0x004b0f80u);
    }
    float random_uniform_float_00bd2f10(std::uint32_t, float, float) override {
        unreached("random_uniform_float_00bd2f10", 0x00bd2f10u); return {};
    }
    void* crt_operator_new_00bf55be(std::uint32_t) override {
        unreached("crt_operator_new_00bf55be", 0x00bf55beu); return {};
    }
    void* crt_operator_new_00bf681b(std::uint32_t) override {
        unreached("crt_operator_new_00bf681b", 0x00bf681bu); return {};
    }
    void crt_free_00bf6989(void*) override { unreached("crt_free_00bf6989", 0x00bf6989u); }
    std::int32_t path_point_count_00415870(void*) override {
        unreached("path_point_count_00415870", 0x00415870u); return {};
    }
    float path_knot_time_007b05e0(void*, std::uint32_t) override {
        unreached("path_knot_time_007b05e0", 0x007b05e0u); return {};
    }
    void camera_path_sample_world_007b03c0(void*, float, float*, float*, std::int32_t) override {
        unreached("camera_path_sample_world_007b03c0", 0x007b03c0u);
    }
    void camera_path_sample_world_linear_007afd60(void*, float, float*, float*) override {
        unreached("camera_path_sample_world_linear_007afd60", 0x007afd60u);
    }
    const float* scene_path_point_to_world_007af800(void*, float*, std::int32_t) override {
        unreached("scene_path_point_to_world_007af800", 0x007af800u); return {};
    }
    void path_points_set_point_local_007ad180(void*, std::int32_t, const float*) override {
        unreached("path_points_set_point_local_007ad180", 0x007ad180u);
    }
    void path_points_add_point_local_007ad330(void*, const float*) override {
        unreached("path_points_add_point_local_007ad330", 0x007ad330u);
    }
    void camera_path_derive_knots_007af150(void*) override {
        unreached("camera_path_derive_knots_007af150", 0x007af150u);
    }
    void* path_component_construct_007b28c0(void*, void*) override {
        unreached("path_component_construct_007b28c0", 0x007b28c0u); return {};
    }
    void* list_buy_node48_00488e30(void*, void*, void*, const void*) override {
        unreached("list_buy_node48_00488e30", 0x00488e30u); return {};
    }
    void list_incsize_00490130(void*, std::uint32_t) override {
        unreached("list_incsize_00490130", 0x00490130u);
    }
    void list_sort_0049f6a0(void*, std::uint32_t) override {
        unreached("list_sort_0049f6a0", 0x0049f6a0u);
    }
    const float* vector4f_transform_00b62d10(const float*, float*, const void*) override {
        unreached("vector4f_transform_00b62d10", 0x00b62d10u); return {};
    }
    void matrix_orthonormalize_0085dad0(float*) override {
        unreached("matrix_orthonormalize_0085dad0", 0x0085dad0u);
    }
    void matrix_copy_004134f0(float*, const float*) override {
        unreached("matrix_copy_004134f0", 0x004134f0u);
    }
    const float* unit_vtable24c_point(void*, float*, std::int32_t) override {
        unreached("unit_vtable24c_point", 0u); return {};
    }
    const float* global_vector_00f8758c() override {
        unreached("global_vector_00f8758c", 0x00f8758cu); return {};
    }
    void gui_widget_set_resolved_position_00aa8240(void*, const float*) override {
        unreached("gui_widget_set_resolved_position_00aa8240", 0x00aa8240u);
    }
    const float* unit_vtablec4_position(void*, float*) override {
        unreached("unit_vtablec4_position", 0u); return {};
    }
    const float* map_offset_00e08890() override {
        unreached("map_offset_00e08890", 0x00e08890u); return {};
    }
};

}  // namespace bsp::cc13_big1
