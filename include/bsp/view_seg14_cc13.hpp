#pragma once
// Segment 14 (0051E7E0..00527C80), packet cc13-seg14: the binoculars HUD screen, the debug
// render-flag and unit-inspector singletons, the cheat-menu screen, HUD screen 2Fh, the small
// entries of the unit-pick screen 29h, observer-handle and 8-byte record templates, and two
// control-layout listeners. Evidence and per-function ABI: docs/CC13_SEG14.md. Names are
// hypotheses, not recovered symbols.
//
// Objects are the image's own storage addressed by byte offset. Calls into bodies that are not
// reconstructed here go through ViewSeg14Hooks at the same point with the same arguments.

#include <cstddef>
#include <cstdint>

namespace bsp {
class NativeStringStorage;
} // namespace bsp

namespace bsp::cc13_seg14 {

using Raw = void*;

struct ViewGlobals {
    std::uint8_t* const& game_00e188a8;
    std::uint8_t* const& hud_root_00e198c4;   // +4Ch binoculars screen, +50h camera driver
    std::uint8_t* const& controlled_00e188d8; // the controlled unit
    float& saved_near_plane_00e18da8;
    std::uint32_t& current_widget_00e08670;
};

struct ViewSeg14Hooks {
    virtual ~ViewSeg14Hooks() = default;
    // Virtual slots, named by slot.
    virtual void widget_show_virtual_34(Raw widget, std::uint32_t flag) = 0;
    virtual bool object_is_kind_virtual_5c(Raw object, std::uint32_t kind) = 0;
    virtual std::int32_t widget_kind_virtual_5c(Raw widget) = 0;
    virtual void widget_set_alpha_virtual_4c(Raw widget, float value) = 0;
    virtual Raw handle_target_virtual_04(Raw handle) = 0;
    virtual void handle_destroy_virtual_00(Raw handle) = 0;           // slot 0, flags 0
    // Direct callees.
    virtual void binoculars_apply_fov_0051e6e0(Raw screen) = 0;
    virtual void lens_flare_set_enabled_00452b80(Raw lens_flare_00e081a0, std::uint32_t flag) = 0;
    virtual void camera_set_near_plane_00b6fbf0(Raw camera, float value) = 0;
    virtual void unit_broadcast_controlled_part_flag_00817380(Raw unit, std::uint32_t flag) = 0;
    virtual void binoculars_update_raise_0051ef00(Raw screen, float dt) = 0;
    virtual void binoculars_turn_mover_0051f050(Raw mover, Raw state, float dt, float zoom) = 0;
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    virtual Raw gui_manager_get_or_create_004c12b0() = 0;
    virtual void gui_manager_remove_page_00aa31f0(Raw manager, Raw page) = 0;
    virtual Raw gui_manager_load_page_00aa5840(Raw manager, Raw name, std::uint32_t a, std::uint32_t b) = 0;
    virtual Raw gui_widget_find_child_00aa7e00(Raw widget, Raw name, std::uint32_t flag) = 0;
    virtual void callback_owner_destroy_00695870(Raw owner) = 0;
    virtual void vector_push_back_004d6790(Raw vector, const void* value) = 0;
    virtual Raw input_layer_004bec00(std::uint32_t id, std::uint32_t flag) = 0;
    virtual void input_layer_pop_00a933f0(Raw layer) = 0;
    virtual void game_restore_cinematic_004cd0f0(Raw game, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void cheat_member_reset_00543e00(Raw member) = 0;
    virtual void node_list_destroy_00427110(Raw list, std::uint32_t flag) = 0;
    virtual void camera_driver_zoom_005454b0(Raw driver, float a, float b, float zoom) = 0;
    virtual void camera_driver_update_005484f0(Raw driver) = 0;
    virtual void camera_driver_attach_00549260(Raw driver, Raw target, Raw extra) = 0;
    virtual bool observer_find_pair_00694af0(Raw first, Raw owner) = 0;
    virtual void observer_register_pair_00694a60(Raw first, Raw owner) = 0;
    virtual void observer_unregister_pair_006952a0(Raw first, Raw owner) = 0;
    virtual bool unit_role_is_ai_00927f10(Raw unit, std::uint32_t role) = 0;
    virtual std::uint32_t widget_index_00a9c920(Raw widget) = 0;
    virtual void free_block(Raw block) = 0; // _free 00BF65AC / 00BF6989
};

// ---- binoculars screen (screen 26h, page "GUI_binoculars") -------------------------------
void binoculars_toggle_zoom_0051e7e0(Raw screen, const ViewGlobals& globals, ViewSeg14Hooks& hooks);
void binoculars_reset_0051e8e0(Raw screen, ViewSeg14Hooks& hooks);
Raw binoculars_screen_construct_0051e950(Raw screen, ViewSeg14Hooks& hooks);
void binoculars_screen_exit_0051e9b0(Raw screen, ViewSeg14Hooks& hooks);
void binoculars_screen_destruct_0051ea20(Raw screen, ViewSeg14Hooks& hooks);
void binoculars_screen_register_0051ed60(Raw screen, ViewSeg14Hooks& hooks, NativeStringStorage& strings);
void binoculars_screen_enter_0051ee50(Raw screen, ViewSeg14Hooks& hooks, NativeStringStorage& strings);
void binoculars_screen_collect_children_0051f040(Raw screen, Raw out, ViewSeg14Hooks& hooks);
void binoculars_update_view_0051f330(Raw screen, float dt, ViewSeg14Hooks& hooks);

// ---- lazy singletons (the shared getter body of engine_seg4_6_9_cc13.cpp) ----------------------
// 0051F520 BSP_DebugRenderFlags_GetSingleton: global 010901CC, 8 bytes, constructor 00B854D0,
// registered at +0. 0051F5E0: global 00E19AAC, 24h bytes, constructor 006FB920, registered +14h.
}
namespace bsp::cc13_sys { struct LazySingletonHost; }
namespace bsp::cc13_seg14 {
void* debug_render_flags_get_0051f520(void*& global_010901cc, cc13_sys::LazySingletonHost& host);
void* unit_debug_inspector_get_0051f5e0(void*& global_00e19aac, cc13_sys::LazySingletonHost& host);

// ---- cheat menu screen (page "_CheatMenu") --------------------------------------------------
void cheat_menu_restore_input_and_cinematic_0051f7e0(Raw screen, const ViewGlobals& globals,
                                                     ViewSeg14Hooks& hooks);
// Calls the 0051F520 getter and clears the singleton's +4h byte.
void cheat_menu_screen_register_0051f830(Raw screen, void*& global_010901cc, cc13_sys::LazySingletonHost& singletons,
                                         ViewSeg14Hooks& hooks, NativeStringStorage& strings);
void cheat_menu_member_destruct_0051f910(Raw member, ViewSeg14Hooks& hooks);
Raw cheat_menu_screen_construct_0051f990(Raw screen, ViewSeg14Hooks& hooks);
bool cheat_menu_screen_is_self_managed_0051f9e0() noexcept;
void cheat_menu_screen_destruct_0051f9f0(Raw screen, ViewSeg14Hooks& hooks);

// ---- HUD screen 2Fh ---------------------------------------------------------------------------
void hud_screen_2f_register_005211b0(Raw screen, ViewSeg14Hooks& hooks);
void hud_screen_2f_update_00521240(Raw screen, float dt, const ViewGlobals& globals, ViewSeg14Hooks& hooks);
void hud_screen_2f_enter_005212a0(Raw screen, const ViewGlobals& globals, ViewSeg14Hooks& hooks);
void hud_screen_2f_on_observed_destroyed_00521310(Raw screen, Raw object) noexcept;

// ---- unit-pick screen 29h (small entries) ------------------------------------------------------
void unit_pick_screen_register_00521670(Raw screen, ViewSeg14Hooks& hooks);
void unit_pick_screen_on_observed_released_005216a0(Raw observer, Raw handle, ViewSeg14Hooks& hooks);
void unit_pick_screen_exit_00524920(Raw screen, ViewSeg14Hooks& hooks);
void unit_pick_screen_enter_00524960(Raw screen, ViewSeg14Hooks& hooks);
bool unit_role_is_ai_held_00521e70(Raw unit, std::uint32_t slot, ViewSeg14Hooks& hooks);

// ---- observer handles (18h bytes: vftable, edge slots +4h..+0Ch, byte +10h, target +14h) -------
void observer_handle_set_target_00522ee0(Raw handle, Raw target, ViewSeg14Hooks& hooks);
Raw observer_handle_construct_00523320(Raw handle, Raw target, ViewSeg14Hooks& hooks);
Raw observer_handle_copy_construct_00523480(Raw handle, const void* source, ViewSeg14Hooks& hooks);
Raw observer_handle_assign_00523b30(Raw handle, const void* source, ViewSeg14Hooks& hooks);
void observer_handle_derived_placement_copy_00524090(Raw handle, const void* source, ViewSeg14Hooks& hooks);
void observer_handle_derived_placement_copy_00524710(Raw handle, const void* source, ViewSeg14Hooks& hooks);
void observer_handle_destroy_range_00524560(Raw first, Raw last, ViewSeg14Hooks& hooks);
void observer_handle_vector_tidy_00524590(Raw vector, ViewSeg14Hooks& hooks);

// ---- lists and 8-byte records -------------------------------------------------------------------
// List {count +0, head +4, tail +8}; node {prev +0, next +4}.
Raw dlist_erase_node_00522b70(Raw list, Raw node, ViewSeg14Hooks& hooks);
void node_list_clear_005234f0(Raw list, ViewSeg14Hooks& hooks);
Raw pod8_uninitialized_copy_00522e10(const void* first, const void* last, Raw destination) noexcept;
void pod8_fill_005235a0(Raw first, Raw last, const void* value) noexcept;
void pod8_uninitialized_fill_n_00523640(Raw destination, std::uint32_t count, const void* value) noexcept;
Raw pod8_copy_backward_00523af0(const void* first, const void* last, Raw destination_end) noexcept;
Raw entity_cast_if_kind_00523540(Raw entity, ViewSeg14Hooks& hooks); // kind 1
Raw entity_cast_if_kind_00523560(Raw entity, ViewSeg14Hooks& hooks); // kind 18h
Raw entity_cast_if_kind_00523580(Raw entity, ViewSeg14Hooks& hooks); // kind 2

// ---- control-layout listeners -------------------------------------------------------------------
void control_layout_select_current_00527be0(Raw listener, Raw unused, Raw widget, const ViewGlobals& globals,
                                            ViewSeg14Hooks& hooks);
void control_layout_dim_widget_00527c30(Raw widget, bool dim, ViewSeg14Hooks& hooks);

} // namespace bsp::cc13_seg14
