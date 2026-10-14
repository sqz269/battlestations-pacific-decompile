#include "bsp/view_seg14_cc13.hpp"

#include "bsp/engine_seg4_6_9_cc13.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

// Segment 14, packet cc13-seg14. Evidence, ABI and simplifications per function:
// docs/CC13_SEG14.md.
namespace bsp::cc13_seg14 {
namespace {

template <class T> T get(const void* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value);
    return value;
}
template <class T> void put(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
unsigned char* at(void* p, std::size_t offset) noexcept { return static_cast<unsigned char*>(p) + offset; }

constexpr float kOne = 1.0f;              // 00D7A24C
constexpr float kQuarter = 0.25f;         // 00CE3868
constexpr float kZero = 0.0f;             // 00D7A218
constexpr float kRaisedNearPlane = 1.5f;  // 00CE380C
constexpr float kFour = 4.0f;             // 00CE3D34
constexpr float kHalf = 0.5f;             // 00CE3800
constexpr const char* kBinocularsPage = "GUI_binoculars"; // 00CEC9E8
constexpr const char* kBinocularsModel = "Tavcso_Model";  // 00CEC9F8
constexpr const char* kCheatMenuPage = "_CheatMenu";      // 00CECA18
constexpr std::uintptr_t kLensFlare = 0x00E081A0;         // ECX of 00452B80

constexpr std::uint32_t kBinocularsVtable = 0x00CEC9B0;
constexpr std::uint32_t kBinocularsCallbackVtable = 0x00CEC998;
constexpr std::uint32_t kCallbackBaseVtable = 0x00CE3CD4;
constexpr std::uint32_t kCheatMenuVtable = 0x00CECA28;
constexpr std::uint32_t kCheatMemberVtable = 0x00CECA24;
constexpr std::uint32_t kObserverHandleVtable = 0x00CE74FC;
constexpr std::uint32_t kObserverHandleDerivedVtable = 0x00CE75CC;

// The inlined NativeString temporary: {0, 0}, 0041DD40(strlen, 1), then a _memcpy of the
// current length + 1 (the terminator included) when the buffer is non-null.
void build_string(void* tmp, const char* literal, NativeStringStorage& strings) {
    put<std::uint32_t>(tmp, 0, 0);
    put<std::uint32_t>(tmp, 4, 0);
    resize_native_string_header_0041dd40(tmp, strings, static_cast<std::uint32_t>(std::strlen(literal)), true);
    if (char* const chars = get<char*>(tmp, 4)) std::memcpy(chars, literal, get<std::uint32_t>(tmp, 0) + 1);
}
void release_string(void* tmp, NativeStringStorage& strings) noexcept {
    if (char* const chars = get<char*>(tmp, 4)) strings.release(chars, get<std::uint32_t>(tmp, 0) + 1);
}

} // namespace

// ---------------------------------------------------------------------------------------------
// Binoculars screen
// ---------------------------------------------------------------------------------------------
void binoculars_toggle_zoom_0051e7e0(Raw screen, const ViewGlobals& globals, ViewSeg14Hooks& hooks) {
    const auto mode = get<std::int32_t>(screen, 0x30);
    if (mode == 1) {
        const float zoom = get<float>(screen, 0x38);
        if (zoom == kQuarter || zoom == kOne) {
            put<float>(screen, 0x38, kOne);
            put<float>(screen, 0x28, kOne);
            put<std::int32_t>(screen, 0x30, 0);
            put<std::uint8_t>(screen, 0x35, 1);
            put<std::uint8_t>(screen, 0x2c, 0);
            put<std::uint8_t>(screen, 0x2d, 0);
            put<std::uint8_t>(screen, 0x2e, 0);
            // UCOMISS/LAHF/TEST AH,44h/JNP skips only on an ordered equal compare.
            if (!(globals.saved_near_plane_00e18da8 == kZero)) {
                void* const mover = get<void*>(screen, 0x40);
                if (mover != nullptr && get<void*>(mover, 0x1bc) != nullptr)
                    hooks.camera_set_near_plane_00b6fbf0(get<void*>(mover, 0x1bc), globals.saved_near_plane_00e18da8);
            }
        }
    } else if (mode == 0 && get<float>(screen, 0x38) == kOne) {
        put<std::int32_t>(screen, 0x30, 1);
        put<std::uint8_t>(screen, 0x34, 1);
        put<float>(screen, 0x38, kOne);
        void* const mover = get<void*>(screen, 0x40);
        if (mover != nullptr && get<void*>(mover, 0x1bc) != nullptr) {
            void* const camera = get<void*>(mover, 0x1bc);
            globals.saved_near_plane_00e18da8 = get<float>(camera, 0x1d4);
            hooks.camera_set_near_plane_00b6fbf0(camera, kRaisedNearPlane);
        }
    }
    if (hooks.object_is_kind_virtual_5c(globals.controlled_00e188d8, 6))
        hooks.unit_broadcast_controlled_part_flag_00817380(globals.controlled_00e188d8,
                                                           get<std::int32_t>(screen, 0x30) == 1 ? 1 : 0);
    hooks.binoculars_apply_fov_0051e6e0(screen); // tail JMP
}

void binoculars_reset_0051e8e0(Raw screen, ViewSeg14Hooks& hooks) {
    put<std::int32_t>(screen, 0x30, 0);
    put<float>(screen, 0x38, kOne);
    put<float>(screen, 0x28, kOne);
    hooks.binoculars_apply_fov_0051e6e0(screen);
    if (get<std::uint8_t>(screen, 5) == 0) return;
    hooks.widget_show_virtual_34(get<void*>(screen, 0x20), 0);
    hooks.lens_flare_set_enabled_00452b80(reinterpret_cast<Raw>(kLensFlare), 0);
}

Raw binoculars_screen_construct_0051e950(Raw screen, ViewSeg14Hooks& hooks) {
    hooks.front_end_screen_construct_004f7180(screen);
    put<std::uint32_t>(screen, 8, kCallbackBaseVtable);
    put<std::uint32_t>(screen, 0xc, 0);
    put<std::uint32_t>(screen, 0x10, 0);
    put<std::uint32_t>(screen, 0x14, 0);
    put<std::uint8_t>(screen, 0x18, 0);
    put<std::uint8_t>(screen, 0x4c, 0);
    put<std::uint32_t>(screen, 0, kBinocularsVtable);
    put<std::uint32_t>(screen, 8, kBinocularsCallbackVtable);
    put<float>(screen, 0x44, 0.0f);
    put<float>(screen, 0x48, 0.0f);
    return screen;
}

void binoculars_screen_exit_0051e9b0(Raw screen, ViewSeg14Hooks& hooks) {
    binoculars_reset_0051e8e0(screen, hooks); // byte-identical body
}

void binoculars_screen_destruct_0051ea20(Raw screen, ViewSeg14Hooks& hooks) {
    put<std::uint32_t>(screen, 0, kBinocularsVtable);
    put<std::uint32_t>(screen, 8, kBinocularsCallbackVtable);
    Raw const manager = hooks.gui_manager_get_or_create_004c12b0();
    hooks.gui_manager_remove_page_00aa31f0(manager, get<void*>(screen, 0x1c));
    hooks.callback_owner_destroy_00695870(at(screen, 8));
    hooks.front_end_screen_destruct_004f71a0(screen);
}

void binoculars_screen_register_0051ed60(Raw screen, ViewSeg14Hooks& hooks, NativeStringStorage& strings) {
    hooks.front_end_screen_register_004f71d0(screen);
    put<std::uint8_t>(screen, 0x2c, 0);
    put<std::uint8_t>(screen, 0x2d, 0);
    put<std::uint8_t>(screen, 0x2e, 0);
    put<float>(screen, 0x28, kOne);
    alignas(4) unsigned char name[8];
    build_string(name, kBinocularsPage, strings);
    Raw const manager = hooks.gui_manager_get_or_create_004c12b0();
    put<void*>(screen, 0x1c, hooks.gui_manager_load_page_00aa5840(manager, name, 1, 0));
    release_string(name, strings);
    put<float>(screen, 0x38, kOne);
    put<std::int32_t>(screen, 0x30, 0);
    put<std::uint32_t>(screen, 0x40, 0);
    put<std::uint8_t>(screen, 0x34, 0);
    put<std::uint8_t>(screen, 0x35, 0);
    put<std::uint8_t>(screen, 0x24, 0);
    put<float>(screen, 0x3c, kFour);
}

void binoculars_screen_enter_0051ee50(Raw screen, ViewSeg14Hooks& hooks, NativeStringStorage& strings) {
    alignas(4) unsigned char name[8];
    build_string(name, kBinocularsModel, strings);
    put<void*>(screen, 0x20, hooks.gui_widget_find_child_00aa7e00(get<void*>(screen, 0x1c), name, 1));
    release_string(name, strings);
    hooks.binoculars_apply_fov_0051e6e0(screen);
}

void binoculars_screen_collect_children_0051f040(Raw screen, Raw out, ViewSeg14Hooks& hooks) {
    hooks.vector_push_back_004d6790(out, at(screen, 0x1c));
}

void binoculars_update_view_0051f330(Raw screen, float dt, ViewSeg14Hooks& hooks) {
    hooks.binoculars_update_raise_0051ef00(screen, dt);
    hooks.binoculars_turn_mover_0051f050(get<void*>(screen, 0x40), at(screen, 0x24), dt, get<float>(screen, 0x38));
}

// ---------------------------------------------------------------------------------------------
// Lazy singletons
// ---------------------------------------------------------------------------------------------
namespace {
const cc13_sys::LazySingletonGetterRow kDebugRenderFlags{0x0051F520u, 0x010901CCu, 8u, 0x00B854D0u, 0u, 0u, 0u, 0u};
const cc13_sys::LazySingletonGetterRow kUnitDebugInspector{0x0051F5E0u, 0x00E19AACu, 0x24u, 0x006FB920u, 0x14u, 0u, 0u, 0u};
} // namespace

void* debug_render_flags_get_0051f520(void*& global_010901cc, cc13_sys::LazySingletonHost& host) {
    return cc13_sys::lazy_singleton_get(kDebugRenderFlags, global_010901cc, host);
}
void* unit_debug_inspector_get_0051f5e0(void*& global_00e19aac, cc13_sys::LazySingletonHost& host) {
    return cc13_sys::lazy_singleton_get(kUnitDebugInspector, global_00e19aac, host);
}

// ---------------------------------------------------------------------------------------------
// Cheat menu screen
// ---------------------------------------------------------------------------------------------
void cheat_menu_restore_input_and_cinematic_0051f7e0(Raw screen, const ViewGlobals& globals,
                                                     ViewSeg14Hooks& hooks) {
    if (get<std::uint8_t>(screen, 0x1a) == 0) return;
    hooks.input_layer_pop_00a933f0(hooks.input_layer_004bec00(0x1e, 0));
    hooks.game_restore_cinematic_004cd0f0(globals.game_00e188a8, get<std::uint8_t>(screen, 0x18),
                                          get<std::uint8_t>(screen, 0x19), 1);
    hooks.cheat_member_reset_00543e00(at(screen, 0x20));
    hooks.widget_show_virtual_34(get<void*>(screen, 0x1c), 0);
    put<std::uint8_t>(screen, 0x1a, 0);
}

void cheat_menu_screen_register_0051f830(Raw screen, void*& global_010901cc, cc13_sys::LazySingletonHost& singletons,
                                         ViewSeg14Hooks& hooks, NativeStringStorage& strings) {
    hooks.front_end_screen_register_004f71d0(screen);
    void* const flags = debug_render_flags_get_0051f520(global_010901cc, singletons);
    put<std::uint8_t>(flags, 4, 0);
    alignas(4) unsigned char name[8];
    build_string(name, kCheatMenuPage, strings);
    Raw const manager = hooks.gui_manager_get_or_create_004c12b0();
    put<void*>(screen, 0x1c, hooks.gui_manager_load_page_00aa5840(manager, name, 1, 0));
    release_string(name, strings);
    put<std::uint8_t>(screen, 4, 1);
}

void cheat_menu_member_destruct_0051f910(Raw member, ViewSeg14Hooks& hooks) {
    put<std::uint32_t>(member, 0, kCheatMemberVtable);
    hooks.cheat_member_reset_00543e00(member);
    hooks.node_list_destroy_00427110(at(member, 0xc), 0);
    hooks.free_block(get<void*>(member, 0xc)); // _free 00BF6989, no null test
}

Raw cheat_menu_screen_construct_0051f990(Raw screen, ViewSeg14Hooks& hooks) {
    hooks.front_end_screen_construct_004f7180(screen);
    put<std::uint32_t>(screen, 0, kCheatMenuVtable);
    put<std::uint32_t>(screen, 0x20, kCheatMemberVtable);
    for (std::size_t offset : {0x2c, 0x30, 0x34, 0x40, 0x10, 0x14}) put<std::uint32_t>(screen, offset, 0);
    put<std::uint8_t>(screen, 0x1a, 0);
    put<std::uint8_t>(screen, 0xe, 0);
    put<std::uint8_t>(screen, 0xd, 0);
    put<std::uint8_t>(screen, 0xc, 0);
    put<std::uint32_t>(screen, 8, 1);
    return screen;
}

bool cheat_menu_screen_is_self_managed_0051f9e0() noexcept { return true; }

void cheat_menu_screen_destruct_0051f9f0(Raw screen, ViewSeg14Hooks& hooks) {
    put<std::uint32_t>(screen, 0, kCheatMenuVtable);
    cheat_menu_member_destruct_0051f910(at(screen, 0x20), hooks);
    hooks.front_end_screen_destruct_004f71a0(screen);
}

// ---------------------------------------------------------------------------------------------
// HUD screen 2Fh
// ---------------------------------------------------------------------------------------------
void hud_screen_2f_register_005211b0(Raw screen, ViewSeg14Hooks& hooks) {
    hooks.front_end_screen_register_004f71d0(screen);
    put<std::uint32_t>(screen, 0x20, 0);
}

void hud_screen_2f_update_00521240(Raw screen, float dt, const ViewGlobals& globals, ViewSeg14Hooks& hooks) {
    binoculars_update_view_0051f330(get<void*>(globals.hud_root_00e198c4, 0x4c), dt, hooks);
    if (get<std::uint32_t>(screen, 0x20) != 0)
        hooks.camera_driver_zoom_005454b0(get<void*>(globals.hud_root_00e198c4, 0x50), kHalf, kHalf,
                                          get<float>(get<void*>(globals.hud_root_00e198c4, 0x4c), 0x38));
    hooks.camera_driver_update_005484f0(get<void*>(globals.hud_root_00e198c4, 0x50));
}

void hud_screen_2f_enter_005212a0(Raw screen, const ViewGlobals& globals, ViewSeg14Hooks& hooks) {
    hooks.camera_driver_attach_00549260(get<void*>(globals.hud_root_00e198c4, 0x50), get<void*>(screen, 0x1c),
                                        get<void*>(screen, 0x20));
    if (get<void*>(screen, 0x1c) == nullptr) return;
    if (hooks.observer_find_pair_00694af0(get<void*>(screen, 0x1c), at(screen, 8))) return;
    hooks.observer_register_pair_00694a60(get<void*>(screen, 0x1c), at(screen, 8)); // tail JMP
}

void hud_screen_2f_on_observed_destroyed_00521310(Raw screen, Raw object) noexcept {
    if (object == get<void*>(screen, 0x14)) {
        put<void*>(screen, 0x14, nullptr);
        return;
    }
    if (object == get<void*>(screen, 0x18)) put<void*>(screen, 0x18, nullptr);
}

// ---------------------------------------------------------------------------------------------
// Unit-pick screen 29h
// ---------------------------------------------------------------------------------------------
void unit_pick_screen_register_00521670(Raw screen, ViewSeg14Hooks& hooks) {
    hooks.front_end_screen_register_004f71d0(screen);
    put<std::uint32_t>(screen, 0xb8, 0);
}

void unit_pick_screen_on_observed_released_005216a0(Raw observer, Raw handle, ViewSeg14Hooks& hooks) {
    void* const target = hooks.handle_target_virtual_04(handle);
    if (target == nullptr || target != get<void*>(observer, 0x34)) return;
    // LEA/NEG/SBB/AND: the callback-owner pointer, null only when this is the +18h of a null screen.
    void* const owner = reinterpret_cast<std::uintptr_t>(observer) != 0x18 ? observer : nullptr;
    hooks.observer_unregister_pair_006952a0(target, owner);
    put<void*>(observer, 0x34, nullptr);
    put<std::uint32_t>(observer, 0x98, 0);
}

namespace {
void pop_pick_input_layers(Raw screen, ViewSeg14Hooks& hooks) {
    const auto mode = get<std::int32_t>(screen, 0x30);
    if (mode != 4 && mode != 5 && mode != 6) return;
    hooks.input_layer_pop_00a933f0(hooks.input_layer_004bec00(4, 1));
    hooks.input_layer_pop_00a933f0(hooks.input_layer_004bec00(0xe, 1));
}
} // namespace

void unit_pick_screen_exit_00524920(Raw screen, ViewSeg14Hooks& hooks) {
    pop_pick_input_layers(screen, hooks);
    put<std::int32_t>(screen, 0x30, 0);
}

void unit_pick_screen_enter_00524960(Raw screen, ViewSeg14Hooks& hooks) {
    pop_pick_input_layers(screen, hooks);
    put<std::int32_t>(screen, 0x30, 0);
    put<float>(screen, 0xec, kQuarter);
}

bool unit_role_is_ai_held_00521e70(Raw unit, std::uint32_t slot, ViewSeg14Hooks& hooks) {
    const auto role = get<std::uint32_t>(unit, 0x1ac + slot * 4u);
    if (role == 8) return true;
    return hooks.unit_role_is_ai_00927f10(unit, role);
}

// ---------------------------------------------------------------------------------------------
// Observer handles
// ---------------------------------------------------------------------------------------------
void observer_handle_set_target_00522ee0(Raw handle, Raw target, ViewSeg14Hooks& hooks) {
    void* const current = get<void*>(handle, 0x14);
    if (current == target) return;
    if (current != nullptr) hooks.observer_unregister_pair_006952a0(current, handle);
    put<void*>(handle, 0x14, target);
    if (target != nullptr) hooks.observer_register_pair_00694a60(target, handle);
}

Raw observer_handle_construct_00523320(Raw handle, Raw target, ViewSeg14Hooks& hooks) {
    put<std::uint32_t>(handle, 4, 0);
    put<std::uint32_t>(handle, 8, 0);
    put<std::uint32_t>(handle, 0xc, 0);
    put<std::uint32_t>(handle, 0, kObserverHandleVtable);
    put<void*>(handle, 0x14, target);
    put<std::uint8_t>(handle, 0x10, 1);
    if (target != nullptr) hooks.observer_register_pair_00694a60(target, handle);
    return handle;
}

Raw observer_handle_copy_construct_00523480(Raw handle, const void* source, ViewSeg14Hooks& hooks) {
    put<std::uint32_t>(handle, 4, 0);
    put<std::uint32_t>(handle, 8, 0);
    put<std::uint32_t>(handle, 0xc, 0);
    put<std::uint8_t>(handle, 0x10, 0);
    put<std::uint32_t>(handle, 0, kObserverHandleVtable);
    void* const target = get<void*>(source, 0x14);
    put<void*>(handle, 0x14, target);
    put<std::uint8_t>(handle, 0x10, 1);
    if (target != nullptr) hooks.observer_register_pair_00694a60(target, handle);
    return handle;
}

Raw observer_handle_assign_00523b30(Raw handle, const void* source, ViewSeg14Hooks& hooks) {
    observer_handle_set_target_00522ee0(handle, get<void*>(source, 0x14), hooks);
    return handle;
}

void observer_handle_derived_placement_copy_00524090(Raw handle, const void* source, ViewSeg14Hooks& hooks) {
    if (handle == nullptr) return;
    observer_handle_copy_construct_00523480(handle, source, hooks);
    put<std::uint32_t>(handle, 0, kObserverHandleDerivedVtable);
}

void observer_handle_derived_placement_copy_00524710(Raw handle, const void* source, ViewSeg14Hooks& hooks) {
    observer_handle_derived_placement_copy_00524090(handle, source, hooks); // same body, stack ABI
}

void observer_handle_destroy_range_00524560(Raw first, Raw last, ViewSeg14Hooks& hooks) {
    for (auto* p = static_cast<unsigned char*>(first); p != last; p += 0x18) hooks.handle_destroy_virtual_00(p);
}

void observer_handle_vector_tidy_00524590(Raw vector, ViewSeg14Hooks& hooks) {
    if (void* const first = get<void*>(vector, 4)) {
        observer_handle_destroy_range_00524560(first, get<void*>(vector, 8), hooks);
        hooks.free_block(get<void*>(vector, 4));
    }
    put<std::uint32_t>(vector, 4, 0);
    put<std::uint32_t>(vector, 8, 0);
    put<std::uint32_t>(vector, 0xc, 0);
}

// ---------------------------------------------------------------------------------------------
// Lists and 8-byte records
// ---------------------------------------------------------------------------------------------
namespace {
// Unlink `node` from {count, head, tail}; the image frees it with _free 00BF65AC.
void unlink(Raw list, Raw node) noexcept {
    void* const prev = get<void*>(node, 0);
    if (prev != nullptr) put<void*>(prev, 4, get<void*>(node, 4));
    else put<void*>(list, 4, get<void*>(node, 4));
    if (get<void*>(node, 4) != nullptr) put<void*>(get<void*>(node, 4), 0, get<void*>(node, 0));
    else put<void*>(list, 8, get<void*>(node, 0));
}
} // namespace

Raw dlist_erase_node_00522b70(Raw list, Raw node, ViewSeg14Hooks& hooks) {
    void* const next = get<void*>(node, 4);
    unlink(list, node);
    put<std::int32_t>(list, 0, get<std::int32_t>(list, 0) - 1);
    hooks.free_block(node);
    return next;
}

void node_list_clear_005234f0(Raw list, ViewSeg14Hooks& hooks) {
    while (get<std::int32_t>(list, 0) != 0) {
        void* const node = get<void*>(list, 4);
        unlink(list, node);
        put<std::int32_t>(list, 0, get<std::int32_t>(list, 0) - 1);
        hooks.free_block(node);
    }
}

Raw pod8_uninitialized_copy_00522e10(const void* first, const void* last, Raw destination) noexcept {
    auto* in = static_cast<const unsigned char*>(first);
    auto* out = static_cast<unsigned char*>(destination);
    for (; in != last; in += 8, out += 8) std::memcpy(out, in, 8);
    return out;
}

void pod8_fill_005235a0(Raw first, Raw last, const void* value) noexcept {
    for (auto* p = static_cast<unsigned char*>(first); p != last; p += 8) std::memcpy(p, value, 8);
}

void pod8_uninitialized_fill_n_00523640(Raw destination, std::uint32_t count, const void* value) noexcept {
    auto* out = static_cast<unsigned char*>(destination);
    for (; count != 0; --count, out += 8) std::memcpy(out, value, 8);
}

Raw pod8_copy_backward_00523af0(const void* first, const void* last, Raw destination_end) noexcept {
    auto* in = static_cast<const unsigned char*>(last);
    auto* out = static_cast<unsigned char*>(destination_end);
    auto* const result = out - (in - static_cast<const unsigned char*>(first)) / 8 * 8;
    while (in != first) {
        in -= 8;
        out -= 8;
        std::memcpy(out, in, 8);
    }
    return result;
}

namespace {
Raw cast_if_kind(Raw entity, std::uint32_t kind, ViewSeg14Hooks& hooks) {
    if (entity == nullptr || !hooks.object_is_kind_virtual_5c(entity, kind)) return nullptr;
    return entity;
}
} // namespace

Raw entity_cast_if_kind_00523540(Raw entity, ViewSeg14Hooks& hooks) { return cast_if_kind(entity, 1, hooks); }
Raw entity_cast_if_kind_00523560(Raw entity, ViewSeg14Hooks& hooks) { return cast_if_kind(entity, 0x18, hooks); }
Raw entity_cast_if_kind_00523580(Raw entity, ViewSeg14Hooks& hooks) { return cast_if_kind(entity, 2, hooks); }

// ---------------------------------------------------------------------------------------------
// Control-layout listeners
// ---------------------------------------------------------------------------------------------
void control_layout_select_current_00527be0(Raw listener, Raw, Raw widget, const ViewGlobals& globals,
                                            ViewSeg14Hooks& hooks) {
    globals.current_widget_00e08670 = hooks.widget_index_00a9c920(widget);
    for (std::uint32_t i = 0; i < 3; ++i)
        hooks.widget_show_virtual_34(get<void*>(listener, 0x14 + i * 4u),
                                     i == globals.current_widget_00e08670 ? 1 : 0);
}

void control_layout_dim_widget_00527c30(Raw widget, bool dim, ViewSeg14Hooks& hooks) {
    if (hooks.widget_kind_virtual_5c(widget) != 3) return;
    hooks.widget_set_alpha_virtual_4c(widget, dim ? kHalf : kOne);
}

} // namespace bsp::cc13_seg14
