// GUI routines of segments 16, 24 and 28 (packet cc13-fe5). Evidence and ABI
// per routine are in include/bsp/frontend_seg16_24_28_cc13.hpp and
// docs/CC13_FE_5.md.

#include "bsp/frontend_seg16_24_28_cc13.hpp"

#include "bsp/band_0067_cc13.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace bsp::cc13_fe5 {

const TrivialSlot* find_trivial_slot(std::uint32_t address) noexcept {
    for (const TrivialSlot& slot : kTrivialSlots) {
        if (slot.address == address) return &slot;
    }
    return nullptr;
}

void unit_doing_icon_listener_released_0068df50(Raw listener) noexcept {
    listener.at<std::uint32_t>(0x18) = 0;
}

void weapon_group_screen_suspend_00545360(Raw screen) noexcept {
    screen.at<std::uint8_t>(0xD4) = 1;
    screen.at<std::uint8_t>(0x1C) = 0;
}

void* pointer_slot_construct_null_0054d440(Raw slot) noexcept {
    slot.at<std::uint32_t>(0) = 0;
    return slot.p;
}

// ---------------------------------------------------------------------------
// Screen 50h, HUD warnings
// ---------------------------------------------------------------------------
namespace {
constexpr std::size_t kWarnFirstGroup = 0x68;
constexpr std::size_t kWarnText = 0x50;
constexpr std::size_t kWarnIcon1 = 0x54;
constexpr std::size_t kWarnIcon2 = 0x58;
constexpr std::size_t kWarnSecondGroup = 0x60;
constexpr std::size_t kWarnThirdGroup = 0x64;
constexpr std::size_t kWarnPage = 0x70;
constexpr std::size_t kWarnLatch = 0x2C;
constexpr std::size_t kWarnPhase = 0x18;
constexpr std::size_t kWarnSample = 0x30;
constexpr std::size_t kWarnSound = 0x40;
constexpr std::size_t kWarnDt = 0x78;
}  // namespace

void warning_screen_enter_00681e60(Raw screen, Fe5Host& host) {
    host.widget_set_visible_34(screen.ptr(kWarnFirstGroup), false);
    host.widget_set_visible_34(screen.ptr(kWarnSecondGroup), false);
    host.widget_set_visible_34(screen.ptr(kWarnThirdGroup), false);
}

void warning_screen_show_wait_00681e90(Raw screen, Fe5Host& host) {
    screen.at<std::uint8_t>(kWarnLatch) = 1;
    host.widget_set_visible_34(screen.ptr(kWarnFirstGroup), true);
    host.text_set_localised_key_00abbe50(screen.ptr(kWarnText), kWarningWaitKey.data());
}

void warning_screen_hide_wait_00681ec0(Raw screen, Fe5Host& host) {
    screen.at<std::uint8_t>(kWarnLatch) = 0;
    host.widget_set_visible_34(screen.ptr(kWarnFirstGroup), false);
    host.text_set_localised_key_00abbe50(screen.ptr(kWarnText), "");  // 00CE3A0C
}

void warning_screen_reset_widgets_00681ef0(Raw screen, Fe5Host& host) {
    if (screen.at<std::uint8_t>(kWarnLatch) != 0) return;
    host.widget_set_visible_34(screen.ptr(kWarnFirstGroup), false);
    host.widget_set_alpha_4c(screen.ptr(kWarnText), 1.0f);
    host.widget_set_alpha_4c(screen.ptr(kWarnIcon1), 1.0f);
    host.widget_set_alpha_4c(screen.ptr(kWarnIcon2), 1.0f);
}

void warning_screen_exit_00682150(Raw screen, Fe5Host& host) {
    host.widget_set_visible_34(screen.ptr(kWarnFirstGroup), false);
    for (int i = 0; i < 4; ++i) {
        void*& sound = screen.ptr(kWarnSound + 4 * i);
        if (sound == nullptr) continue;
        host.sound_stop_08(sound, false);
        host.sound_handle_release(sound);
        sound = nullptr;
    }
}

void warning_screen_register_00682740(Raw screen, Fe5Host& host) {
    host.front_end_screen_register_004f71d0(screen);
    screen.ptr(kWarnPage) = host.gui_load_page_00aa5840(kWarningPage);
    host.screen_virtual_14(screen);
    screen.at<std::uint8_t>(kWarnLatch) = 0;
}

void warning_screen_layout_006823c0(Raw screen, Fe5Host& host) {
    void* page = screen.ptr(kWarnPage);
    screen.ptr(kWarnFirstGroup) = host.gui_find_child_00aa7e00(page, "first_Group");
    screen.ptr(kWarnText) = host.gui_find_child_00aa7e00(page, "warning_text");
    screen.ptr(kWarnIcon1) = host.gui_find_child_00aa7e00(page, "warning_1_Icon");
    screen.ptr(kWarnIcon2) = host.gui_find_child_00aa7e00(page, "warning_2_Icon");
    screen.at<float>(0x6C) = 1.0f;  // 00D7A24C
    screen.ptr(kWarnSecondGroup) = host.gui_find_child_00aa7e00(page, "second_Group");
    screen.ptr(kWarnThirdGroup) = host.gui_find_child_00aa7e00(page, "third_Group");
    const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    host.widget_set_colour_50(host.gui_find_child_00aa7e00(page, "warning_2_text"), white);
}

void warning_screen_play_sound_00682800(Raw screen, int alert, Fe5Host& host) {
    void*& slot = screen.ptr(kWarnSound + 4 * alert);
    if (slot != nullptr) return;
    void* const sound_class = host.ingame_gui_sound_class_00e1988c();
    void* const created =
        host.sound_create_tracked_normal(screen.ptr(kWarnSample + 4 * alert), sound_class);
    if (slot != created) {
        void* const old = slot;
        slot = created;
        if (created != nullptr) host.sound_handle_add_ref(created);
        if (old != nullptr) host.sound_handle_release(old);
    }
    if (created != nullptr) host.sound_handle_release(created);
}

float warning_pulse_alpha(float phase) noexcept {
    const float angle = static_cast<float>(static_cast<double>(phase) * kWarningPulseRate);
    const float s = static_cast<float>(std::sin(static_cast<double>(angle)));
    return static_cast<float>((static_cast<double>(s) + kWarningPulseOne) * kWarningPulseHalf *
                                  kWarningPulseSpan + kWarningPulseFloor);
}

void warning_screen_show_alert(Raw screen, int alert, Fe5Host& host) {
    float& phase = screen.at<float>(kWarnPhase + 4 * alert);
    phase = screen.at<float>(kWarnDt) + phase;
    host.widget_set_visible_34(screen.ptr(kWarnFirstGroup), true);
    host.text_set_localised_key_00abbe50(screen.ptr(kWarnText), kWarningKeys[alert].data());
    // One rounded alpha feeds all three widgets ([esp+4] is reloaded).
    const float alpha = warning_pulse_alpha(phase);
    host.widget_set_alpha_4c(screen.ptr(kWarnText), alpha);
    host.widget_set_alpha_4c(screen.ptr(kWarnIcon1), alpha);
    host.widget_set_alpha_4c(screen.ptr(kWarnIcon2), alpha);
    if (alert == 0) host.award_tracker_trigger_hint_00690fd0(kStallHint);
    warning_screen_play_sound_00682800(screen, alert, host);
}

void warning_screen_collect_children_00682ec0(Raw screen, void* out, Fe5Host& host) {
    host.push_page_slot_004d6790(out, &screen.ptr(kWarnPage));
}

bool game_near_world_edge_00681f40(Raw game, const float pos[3], float margin) noexcept {
    const double x = pos[0];
    const double z = pos[2];
    const double m = margin;
    if (static_cast<double>(game.at<float>(0x711C)) + m > x) return true;
    if (x > static_cast<double>(game.at<float>(0x7128)) - m) return true;
    if (z > static_cast<double>(game.at<float>(0x7124)) - m) return true;
    if (static_cast<double>(game.at<float>(0x7130)) + m > z) return true;
    return false;
}

// ---------------------------------------------------------------------------
// Tree helpers, front-end manager base, logo sequence
// ---------------------------------------------------------------------------
void* tree_iterator_pre_increment(Raw iterator, Fe5Host& host) {
    host.tree_iterator_increment_00683d10(iterator);
    return iterator.p;
}

TreeEqualRange tree_equal_range_006845a0(Raw tree, std::uint32_t key) noexcept {
    auto node = [](void* n) { return Raw{static_cast<std::uint8_t*>(n)}; };
    void* const head = tree.ptr(4);
    // First loop: the first node whose key is greater (upper bound).
    void* upper = head;
    for (void* n = node(head).ptr(4); node(n).at<std::uint8_t>(0x11) == 0;) {
        if (key < node(n).at<std::uint32_t>(0x0C)) {
            upper = n;
            n = node(n).ptr(0);
        } else {
            n = node(n).ptr(8);
        }
    }
    // Second loop: the first node whose key is not less (lower bound).
    void* lower = head;
    for (void* n = node(head).ptr(4); node(n).at<std::uint8_t>(0x11) == 0;) {
        if (node(n).at<std::uint32_t>(0x0C) < key) {
            n = node(n).ptr(8);
        } else {
            lower = n;
            n = node(n).ptr(0);
        }
    }
    return TreeEqualRange{tree.p, lower, tree.p, upper};
}

std::size_t tree_erase_key_00684ef0(Raw tree, std::uint32_t key, Fe5Host& host) {
    const TreeEqualRange r = tree_equal_range_006845a0(tree, key);
    const void* const range[4] = {r.lower_container, r.lower_node, r.upper_container, r.upper_node};
    const std::size_t count = host.tree_range_distance_00684340(range);
    host.tree_erase_range_00684d00(tree, range);
    return count;
}

void observer_ref_construct_006844f0(Raw ref) noexcept {
    ref.at<std::uint32_t>(0x00) = 0;
    ref.at<std::uint32_t>(0x08) = 0;
    ref.at<std::uint32_t>(0x0C) = 0;
    ref.at<std::uint32_t>(0x10) = 0;
    ref.at<std::uint32_t>(0x18) = 0;
    ref.at<std::uint8_t>(0x14) = 1;
    ref.at<std::uint32_t>(0x04) = kObserverBaseVtable;
}

void* front_end_manager_construct_base_00684e10(Raw manager, Fe5Host& host) {
    manager.at<std::uint32_t>(0x00) = kFrontEndManagerVtable;
    observer_ref_construct_006844f0(Raw{manager.p + 0x04});
    observer_ref_construct_006844f0(Raw{manager.p + 0x20});
    manager.at<std::uint8_t>(0x3C) = 0;
    host.front_end_registry_insert_00684c40(manager.p);
    return manager.p;
}

void front_end_manager_destroy_base_00684fa0(Raw manager, Fe5Host& host) {
    manager.at<std::uint32_t>(0x00) = kFrontEndManagerVtable;
    tree_erase_key_00684ef0(host.front_end_registry_00e19898(),
                            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(manager.p)), host);
    host.target_observer_destruct_0042bca0(Raw{manager.p + 0x24});
    host.target_observer_destruct_0042bca0(Raw{manager.p + 0x08});
}

void logo_sequence_on_movie_complete_00685060(Fe5Host& host) {
    const Raw logo = host.logo_sequence_global_00e198a4();
    if (logo.p != nullptr) host.logo_sequence_advance_00685070(logo);
}

void* logo_sequence_construct_006851e0(Raw logo, Fe5Host& host) {
    host.front_end_manager_construct_base_00684e10(logo);
    logo.at<std::uint32_t>(0x00) = kLogoSequenceVtable;
    for (std::size_t off : {0x44u, 0x48u, 0x4Cu, 0x54u, 0x58u, 0x5Cu, 0x68u, 0x6Cu, 0x74u}) {
        logo.at<std::uint32_t>(off) = 0;
    }
    logo.at<std::uint32_t>(0x70) = 1;
    host.set_logo_sequence_global_00e198a4(logo.p);
    return logo.p;
}

void logo_sequence_destruct_00685240(Raw logo, Fe5Host& host) {
    logo.at<std::uint32_t>(0x00) = kLogoSequenceVtable;
    host.set_logo_sequence_global_00e198a4(nullptr);
    if (logo.ptr(0x54) != nullptr) host.crt_free(logo.ptr(0x54));
    logo.ptr(0x54) = nullptr;
    logo.ptr(0x58) = nullptr;
    logo.ptr(0x5C) = nullptr;
    if (logo.ptr(0x44) != nullptr) host.native_string_range_destroy_free_00432050(logo.ptr(0x44), logo.ptr(0x48));
    logo.ptr(0x44) = nullptr;
    logo.ptr(0x48) = nullptr;
    logo.ptr(0x4C) = nullptr;
    host.front_end_manager_destroy_base_00684fa0(logo);
}

std::string replace_all(std::string text, std::string_view find, std::string_view with) {
    if (find.empty()) return text;
    std::size_t at = 0;
    while ((at = text.find(find, at)) != std::string::npos) {
        text.replace(at, find.size(), with);
        at += with.size();
    }
    return text;
}

void logo_sequence_load_table_00685300(Raw logo, Fe5Host& host) {
    host.logo_table_open_00b66270();
    std::string fmv;
    float delay = 0.0f;
    for (int index = 1; host.logo_table_entry(index, fmv, delay); ++index) {
        std::string path = replace_all(fmv, kLogoRegionToken, host.user_geography_region_00439100());
        path = replace_all(path, kLogoPlatformToken, kLogoPlatform);
        host.native_string_vector_append_00450540(Raw{logo.p + 0x40}, path);
        host.float_vector_insert_0045a120(Raw{logo.p + 0x50}, delay);
    }
    logo.at<std::uint32_t>(0x60) = 0;
    host.game_on_init_once_004dd5b0(1);
    host.logo_sequence_advance_00685070(logo);
    host.logo_table_close();
}
// ---------------------------------------------------------------------------
// Front-end managers
// ---------------------------------------------------------------------------
void interface_update_music_streams_00685c80(Raw manager, float seconds, Fe5Host& host) {
    const Raw browser = host.session_browser_00e198b8();
    if (browser.p != nullptr && browser.ptr(0x40) != nullptr &&
        Raw{static_cast<std::uint8_t*>(browser.ptr(0x40))}.at<std::uint8_t>(4) != 0) {
        return;
    }
    if (manager.ptr(0x50) == nullptr) return;
    host.sound_stream_set_gain_00a864f0(manager.ptr(0x50), host.music_gain_00f889a8());
    host.sound_stream_update_00a874d0(manager.ptr(0x50), seconds);
    if (manager.ptr(0x54) == nullptr) return;
    host.sound_stream_set_gain_00a864f0(manager.ptr(0x54), host.music_gain_00f889a8());
    host.sound_stream_update_00a874d0(manager.ptr(0x54), seconds);
}

void* main_menu_construct_00686170(Raw menu, Fe5Host& host) {
    host.front_end_manager_construct_base_00684e10(menu);
    menu.at<std::uint32_t>(0) = kMainMenuVtable;
    for (std::size_t off = 0x40; off <= 0x70; off += 4) menu.at<std::uint32_t>(off) = 0;
    menu.at<std::uint8_t>(0x74) = 0;
    return menu.p;
}

void main_menu_register_localisation_tables_00686240(Fe5Host& host) {
    host.localization_register_table_00aa0d30("FE");
    host.localization_register_table_00aa0d30("FE_pc");
}

bool multi_menu_apply_pending_interface_00687800(Raw menu, int interface_id, int argument, Fe5Host& host) {
    host.trace_004254b0("GVMultiMenu::SelectInterface() set:%s", host.interface_name_00e08cd8(interface_id));
    if (!host.front_end_manager_apply_request_00684600(menu, interface_id, argument)) return false;
    host.front_end_screens_set_level4_004f8710(host.multi_menu_map_interface_00687330(menu, interface_id));
    host.game_input_contexts_set_level4_004d8c00();
    return true;
}

std::int32_t interface_request_count_006878d0(Raw manager) noexcept {
    const std::uintptr_t first = manager.at<std::uint32_t>(0x5C);
    if (first == 0) return 0;
    return static_cast<std::int32_t>(manager.at<std::uint32_t>(0x60) - first) >> 2;
}

void front_end_manager_deactivate_and_hide_006878f0(Raw manager, Fe5Host& host) {
    host.object_virtual(manager, 0x0C);
    for (std::size_t off = 0x40; off <= 0x4C; off += 4) {
        const Raw screen{static_cast<std::uint8_t*>(manager.ptr(off))};
        if (screen.at<std::uint8_t>(5) != 0) host.object_virtual(screen, 0x1C);
        screen.at<std::uint8_t>(4) = 0;
        screen.at<std::uint8_t>(5) = 0;
        host.front_end_screen_commit_visibility_004f83b0(screen);
    }
    for (std::size_t off = 0x40; off <= 0x4C; off += 4) {
        if (manager.ptr(off) == nullptr) continue;
        host.object_virtual_int(Raw{static_cast<std::uint8_t*>(manager.ptr(off))}, 0x0C, 1);
        manager.ptr(off) = nullptr;
    }
}

void singleton_destroy_00f8894c_00687c00(Fe5Host& host) {
    void** const cell = host.singleton_cell_00f8894c();
    if (*cell == nullptr) return;
    void* const lock = host.singleton_lifetime_lock_00415350();
    if (lock != nullptr) host.critical_section_enter(lock);  // then ++count at +18h
    if (*cell != nullptr) {
        host.singleton_lifetime_unregister_00bcfca0(*cell);
        if (*cell != nullptr) host.object_virtual_int(Raw{static_cast<std::uint8_t*>(*cell)}, 0x00, 1);
        *cell = nullptr;
    }
    if (lock != nullptr) host.critical_section_leave(lock);  // --count, then leave
}

void multi_menu_close_pause_interface_006882a0(Raw menu, Fe5Host& host) {
    host.gui_manager_set_enabled_00aa0e00(false);
    host.multi_menu_pop_request_006881f0(menu);
    host.object_virtual(menu, 0x0C);
    host.object_virtual(host.ingame_interface_00e198c4(), 0x08);
}
void* online_player_info_assign_006883b0(Raw dst, Raw src, Fe5Host& host) {
    host.legacy_sbo_string_assign_whole(dst, src);
    dst.at<std::uint32_t>(0x20) = src.at<std::uint32_t>(0x20);
    dst.at<std::uint32_t>(0x24) = src.at<std::uint32_t>(0x24);
    for (std::size_t off = 0x28; off <= 0x2B; ++off) dst.at<std::uint8_t>(off) = src.at<std::uint8_t>(off);
    dst.at<std::uint32_t>(0x2C) = src.at<std::uint32_t>(0x2C);
    dst.at<std::uint8_t>(0x30) = src.at<std::uint8_t>(0x30);
    dst.at<std::uint32_t>(0x34) = src.at<std::uint32_t>(0x34);
    for (std::size_t off = 0x38; off <= 0x3B; ++off) dst.at<std::uint8_t>(off) = src.at<std::uint8_t>(off);
    return dst.p;
}

void string_list_clear_00688780(Raw list, Fe5Host& host) {
    auto raw = [](void* p) { return Raw{static_cast<std::uint8_t*>(p)}; };
    void* const head = list.ptr(4);
    void* node = raw(head).ptr(0);
    raw(head).ptr(0) = head;
    raw(list.ptr(4)).ptr(4) = list.ptr(4);
    list.at<std::uint32_t>(8) = 0;
    while (node != list.ptr(4)) {
        void* const next = raw(node).ptr(0);
        if (raw(node).at<std::uint32_t>(0x20) > 0x0F) host.crt_free(raw(node).ptr(0x0C));
        raw(node).at<std::uint32_t>(0x20) = 0x0F;
        raw(node).at<std::uint32_t>(0x1C) = 0;
        raw(node).at<std::uint8_t>(0x0C) = 0;
        host.crt_free(node);
        node = next;
    }
}

namespace {
// std::vector<T>::_Insert_n of 00688470 / 0054B080 (T = uint32_t) and
// 0054AED0 (T = uint8_t): max_size is FFFFFFFFh / sizeof(T).
template <class T>
void stl_vector_insert_n(Raw vector, T* where, std::size_t count, T value, Fe5Host& host) {
    constexpr std::size_t kMax = 0xFFFFFFFFu / sizeof(T);
    auto first = [&] { return static_cast<T*>(vector.ptr(4)); };
    auto last = [&] { return static_cast<T*>(vector.ptr(8)); };
    const std::size_t capacity = first() ? static_cast<std::size_t>(static_cast<T*>(vector.ptr(0x0C)) - first()) : 0;
    if (count == 0) return;
    const std::size_t size = first() ? static_cast<std::size_t>(last() - first()) : 0;
    // kMax - size < count throws length_error (00688310 / 0054AD60); not modelled.
    if (capacity < size + count) {
        std::size_t grown = (kMax - (capacity >> 1) < capacity) ? 0 : capacity + (capacity >> 1);
        if (grown < size + count) grown = size + count;
        auto* const block = static_cast<T*>(host.stl_allocate_bytes(grown * sizeof(T)));
        T* out = block;
        for (T* p = first(); p != where; ++p) *out++ = *p;
        for (std::size_t i = 0; i < count; ++i) *out++ = value;
        for (T* p = where; p != last(); ++p) *out++ = *p;
        if (first() != nullptr) host.crt_free(first());
        vector.ptr(0x0C) = block + grown;
        vector.ptr(8) = block + size + count;
        vector.ptr(4) = block;
        return;
    }
    T* const end = last();
    if (static_cast<std::size_t>(end - where) < count) {
        // Tail shorter than the insertion: move the tail past the new block,
        // fill the gap beyond the old end, then fill [where, old end).
        T* out = where + count;
        for (T* p = where; p != end; ++p) *out++ = *p;
        for (T* p = end; p != where + count; ++p) *p = value;
        vector.ptr(8) = end + count;
        for (T* p = where; p != end; ++p) *p = value;
        return;
    }
    T* const split = end - count;
    T* out = end;
    for (T* p = split; p != end; ++p) *out++ = *p;
    vector.ptr(8) = out;
    for (T *s = split, *d = end; s != where;) *--d = *--s;
    for (T* p = where; p != where + count; ++p) *p = value;
}
}  // namespace

void dword_vector_insert_n_00688470(Raw vector, std::uint32_t* where, std::size_t count,
                                    std::uint32_t value, Fe5Host& host) {
    stl_vector_insert_n<std::uint32_t>(vector, where, count, value, host);
}

void byte_vector_insert_n_0054aed0(Raw vector, std::uint8_t* where, std::size_t count, std::uint8_t value,
                                   Fe5Host& host) {
    stl_vector_insert_n<std::uint8_t>(vector, where, count, value, host);
}

void* multi_menu_construct_006887e0(Raw menu, Fe5Host& host) {
    host.front_end_manager_construct_base_00684e10(menu);
    menu.at<std::uint32_t>(0) = kMultiMenuVtable;
    menu.at<std::uint32_t>(0x5C) = 0;
    menu.at<std::uint32_t>(0x60) = 0;
    menu.at<std::uint32_t>(0x64) = 0;
    return menu.p;
}

void multi_menu_pop_request_up_to_00688b20(Raw menu, int interface_id, Fe5Host& host) {
    host.trace_004254b0("GVMultiMenu::PopRequestInterfaceUpTo() set:%s", host.interface_name_00e08cd8(interface_id));
    const std::int32_t current = menu.at<std::int32_t>(4);
    host.trace2_004254b0("GVMultiMenu::PushRequestInterface() req:%s set:%s",
                         host.interface_name_00e08cd8(menu.at<std::int32_t>(0x20)),
                         host.interface_name_00e08cd8(current));
    host.dword_vector_push_back_00688830(Raw{menu.p + 0x58}, menu.at<std::int32_t>(0x20));
    host.front_end_manager_push_request_004cc460(menu, current);
    while (interface_request_count_006878d0(menu) != 0) {
        const auto* top = static_cast<const std::int32_t*>(menu.ptr(0x60)) - 1;
        if (*top == interface_id) break;
        host.multi_menu_pop_request_006881f0(menu);
    }
    if (interface_request_count_006878d0(menu) == 0) {
        host.front_end_manager_push_request_004cc460(menu, 0x0F);
    } else {
        host.multi_menu_pop_request_006881f0(menu);
    }
    if (host.online_virtual_bool(0x90)) host.online_virtual_int(0x8C, -1);
    if (interface_id != 0x14) {
        if (host.online_virtual_bool(0x180)) host.online_virtual_int(0x170, 1);
        host.session_set_mode_0076fad0(0);
    }
    host.set_byte_00e08700(0);
}

void multi_menu_push_request_logged_00688ad0(Raw menu, int interface_id, Fe5Host& host) {
    host.trace2_004254b0("GVMultiMenu::PushRequestInterface() req:%s set:%s",
                         host.interface_name_00e08cd8(menu.at<std::int32_t>(0x20)),
                         host.interface_name_00e08cd8(interface_id));
    host.dword_vector_push_back_00688830(Raw{menu.p + 0x58}, menu.at<std::int32_t>(0x20));
    host.front_end_manager_push_request_004cc460(menu, interface_id);
}

void multi_menu_leave_to_main_interface_00688c70(Raw menu, Fe5Host& host) {
    auto on_stack = [&](std::int32_t id) {
        if (menu.at<std::int32_t>(0x20) == id) return true;
        const auto* first = static_cast<const std::int32_t*>(menu.ptr(0x5C));
        const std::int32_t n = interface_request_count_006878d0(menu);
        for (std::int32_t i = 0; first != nullptr && i < n; ++i) {
            if (first[i] == id) return true;
        }
        return false;
    };
    bool popped = false;
    for (std::int32_t id : {0x19, 0x12, 0x11, 0x10, 0x0F}) {
        if (on_stack(id)) {
            multi_menu_pop_request_up_to_00688b20(menu, id, host);
            popped = true;
            break;
        }
    }
    if (!popped && menu.at<std::int32_t>(0x20) == 0x14 && host.game_byte_218c() == 0) {
        host.front_end_manager_push_request_004cc460(menu, 0x0F);
    }
    if (host.ingame_interface_00e198c4().p != nullptr) host.game_request_state_004d7920(0x10);
}

void multi_menu_open_pause_interface_00689030(Raw menu, Fe5Host& host) {
    host.ingame_interface_collapse_overlays_0068ab80();
    host.object_virtual(host.ingame_interface_00e198c4(), 0x0C);
    host.gui_manager_set_enabled_00aa0e00(true);
    host.trace2_004254b0("GVMultiMenu::PushRequestInterface() req:%s set:%s",
                         host.interface_name_00e08cd8(menu.at<std::int32_t>(0x20)),
                         host.interface_name_00e08cd8(0x17));  // 00E08D34
    host.dword_vector_push_back_00688830(Raw{menu.p + 0x58}, menu.at<std::int32_t>(0x20));
    host.front_end_manager_push_request_004cc460(menu, 0x17);
    host.object_virtual(menu, 0x08);
}

void multi_menu_on_scene_exit_00688e40(Raw menu, Fe5Host& host) {
    host.file_block_begin("GVMultiMenu::SceneExit");
    for (const MultiScreenNew& s : kMultiMenuScreens) {
        const Raw screen = host.multi_screen_new(s.constructor, s.size);
        menu.ptr(s.slot) = screen.p;
        host.object_virtual(screen, 0x10);
    }
    const std::int32_t current = menu.at<std::int32_t>(0x20);
    if (current != 0x1B) {
        bool found = current == 0x14;
        const auto* first = static_cast<const std::int32_t*>(menu.ptr(0x5C));
        for (std::int32_t i = 0; !found && first != nullptr && i < interface_request_count_006878d0(menu); ++i) {
            found = first[i] == 0x14;
        }
        if (found) multi_menu_pop_request_up_to_00688b20(menu, 0x14, host);
    }
    host.object_virtual(menu, 0x08);
    host.file_block_end();
}

void multi_menu_update_online_interface_006890b0(Raw menu, Fe5Host& host) {
    const bool online = host.online_virtual_bool(0x14);
    const bool error = host.online_error_text_size_74() != 0 && menu.at<std::int32_t>(0x20) != 0x1B;
    if (error) {
        if (host.ingame_interface_00e198c4().p != nullptr) host.game_end_scene_004d7970();
        if (!host.online_virtual_bool(0x19C)) multi_menu_leave_to_main_interface_00688c70(menu, host);
        multi_menu_push_request_logged_00688ad0(menu, 0x1B, host);
        host.multi_error_screen_set_code_005d2e70(host.xenon_byte_128() != 0 ? 6 : 1);
        if (menu.at<std::uint8_t>(0x3C) == 0) host.object_virtual(menu, 0x08);
    } else if (!online && host.game_byte_218c() == 0) {
        host.multi_error_screen_set_code_005d2e70(5);
        if (menu.at<std::int32_t>(0x20) != 0x1B) multi_menu_push_request_logged_00688ad0(menu, 0x1B, host);
        if (menu.at<std::uint8_t>(0x3C) == 0 && host.ingame_interface_00e198c4().p != nullptr) {
            host.object_virtual(menu, 0x08);
        }
    }
    const Raw flag_owner{static_cast<std::uint8_t*>(menu.ptr(0x54))};
    if (host.session_mode_1fe4() == 0) {
        if (flag_owner.at<std::uint8_t>(4) != 0) flag_owner.at<std::uint8_t>(4) = 0;
    } else if (flag_owner.at<std::uint8_t>(4) == 0) {
        flag_owner.at<std::uint8_t>(4) = 1;
    }
    if (host.xenon_has_selected_user_00a3e510() && host.xenon_selected_sign_in_state_00a3ead0() == 2) return;
    if (!host.online_virtual_bool(0x198) || menu.at<std::int32_t>(0x20) == 0x1B) return;
    const auto* first = static_cast<const std::int32_t*>(menu.ptr(0x5C));
    for (std::int32_t i = 0; first != nullptr && i < interface_request_count_006878d0(menu); ++i) {
        if (first[i] == 0x1B) return;
    }
    const std::int32_t current = menu.at<std::int32_t>(0x20);
    multi_menu_pop_request_up_to_00688b20(menu, 0x0F, host);
    if (current != 0x1B) host.multi_error_screen_set_code_005d2e70(host.xenon_byte_128() != 0 ? 6 : 1);
    multi_menu_push_request_logged_00688ad0(menu, 0x1B, host);
    if (host.ingame_interface_00e198c4().p != nullptr) host.game_request_state_004d7920(0x10);
}

void* options_menu_construct_00689800(Raw menu, Fe5Host& host) {
    host.front_end_manager_construct_base_00684e10(menu);
    menu.at<std::uint32_t>(0) = kOptionsMenuVtable;
    return menu.p;
}

void attract_screen_deactivate_00689c00(Raw screen, Fe5Host& host) {
    host.front_end_manager_deactivate_00683aa0(screen);
    host.widget_set_visible_34(screen.ptr(0x40), false);
    screen.at<std::uint8_t>(0x3C) = 0;
    const Raw movie = host.movie_player_00e18d48();
    if (movie.at<std::uint8_t>(5) != 0) host.object_virtual(movie, 0x1C);
    movie.at<std::uint8_t>(4) = 0;
    movie.at<std::uint8_t>(5) = 0;
    host.front_end_screen_commit_visibility_004f83b0(movie);
    host.movie_player_stop_004f8ac0();
    screen.at<float>(0x44) = kAttractIdleSeconds;
}

void attract_screen_destruct_00689c60(Raw screen, Fe5Host& host) {
    screen.at<std::uint32_t>(0) = kAttractScreenVtable;
    host.set_attract_screen_global_00e198bc(nullptr);
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0x40));
    host.front_end_manager_destroy_base_00684fa0(screen);
}

void* attract_screen_construct_00689d90(Raw screen, Fe5Host& host) {
    host.front_end_manager_construct_base_00684e10(screen);
    screen.at<std::uint32_t>(0) = kAttractScreenVtable;
    screen.at<float>(0x44) = kAttractIdleSeconds;
    host.set_attract_screen_global_00e198bc(screen.p);
    screen.at<std::uint8_t>(0x48) = 1;
    screen.ptr(0x40) = host.gui_load_page_00aa5840("FE_attract");
    return screen.p;
}

void attract_screen_activate_00689e80(Raw screen, Fe5Host& host) {
    host.front_end_manager_activate_00684700(screen);
    host.widget_set_visible_34(screen.ptr(0x40), true);
    const Raw movie = host.movie_player_00e18d48();
    movie.at<std::uint8_t>(4) = 1;
    movie.at<std::uint8_t>(5) = 1;
    host.front_end_screen_commit_visibility_004f83b0(movie);
    host.object_virtual(movie, 0x18);
    host.movie_player_play_004f8a20(kAttractMovie, 1, kAttractMovieZ, 1);
}

void set_unit_input_contexts(const InputContextLevel& level, Fe5Host& host) {
    host.game_input_contexts_push_level_004d8a50(level.contexts.data());
}

bool ingame_interface_screen_8c_shown_0068a120(Raw iface) noexcept {
    const auto* screen = static_cast<const std::uint8_t*>(iface.ptr(0x8C));
    return screen != nullptr && screen[4] != 0;
}

void ingame_interface_update_spectate(const SpectateScreenRoutines& r, Raw iface, Fe5Host& host) {
    const Raw screen{static_cast<std::uint8_t*>(iface.ptr(r.screen_offset))};
    if (screen.at<std::uint8_t>(4) == 0) {
        host.spectate_screen_engage(screen, r.engage, host.controlled_unit_00e188d8().p);
        return;
    }
    const Raw camera = host.spectate_screen_camera(screen, r.camera);
    if (r.null_camera_returns && camera.p == nullptr) return;
    const Raw hud_root{static_cast<std::uint8_t*>(iface.ptr(0x40))};
    if (host.camera_target_unit(camera, r.target).p != nullptr) {
        Raw unit = host.camera_target_unit(camera, r.target);
        if (host.unit_is_kind_5c(unit, 2)) {
            unit = host.camera_target_unit(camera, r.target);
            if (host.unit_is_kind_5c(unit, 0x0F)) unit = Raw{static_cast<std::uint8_t*>(unit.ptr(0x9D4))};
            if (host.unit_selectable_for_player_00645160(unit)) {
                host.hud_root_set_spectated_unit_00647300(hud_root, unit);
                return;
            }
        }
    }
    host.hud_root_force_select_unit_006485a0(hud_root);
}

void* small_hud_screen_construct(const SmallHudScreenShape& shape, Raw screen, Fe5Host& host) {
    host.front_end_screen_construct_004f7180(screen);
    screen.at<std::uint32_t>(0x08) = kCallbackOwnerBaseVtable;
    screen.at<std::uint32_t>(0x0C) = 0;
    screen.at<std::uint32_t>(0x10) = 0;
    screen.at<std::uint32_t>(0x14) = 0;
    screen.at<std::uint8_t>(0x18) = 0;
    screen.at<std::uint32_t>(0x00) = shape.primary_vtable;
    screen.at<std::uint32_t>(0x08) = shape.secondary_vtable;
    for (std::uint16_t off : shape.zero_dwords) {
        if (off == 0) break;
        screen.at<std::uint32_t>(off) = 0;
    }
    for (std::uint16_t off : shape.zero_bytes) {
        if (off == 0) break;
        screen.at<std::uint8_t>(off) = 0;
    }
    if (shape.observer_link_1c) {
        screen.at<std::uint32_t>(0x20) = 0;
        screen.at<std::uint32_t>(0x24) = 0;
        screen.at<std::uint32_t>(0x28) = 0;
        screen.at<std::uint32_t>(0x30) = 0;
        screen.at<std::uint8_t>(0x2C) = 1;
        screen.at<std::uint32_t>(0x1C) = kObserverLinkBaseVtable;
    }
    return screen.p;
}

void small_hud_screen_destruct(const SmallHudScreenShape& shape, Raw screen, Fe5Host& host) {
    if (shape.observer_link_1c) host.observer_link_destroy_base_004b7ef0(Raw{screen.p + 0x1C});
    host.callback_owner_destroy_00695870(Raw{screen.p + 0x08});
    host.front_end_screen_destruct_004f71a0(screen);
}

bool ingame_interface_apply_pending_level_change_0068a9c0(Raw iface, Fe5Host& host) {
    const Raw scene = host.game_scene_1ed4();
    if (scene.p != nullptr && host.unit_is_kind_5c(scene, 0x54)) iface.at<std::uint8_t>(0xFD) = 0;
    if (iface.at<std::uint8_t>(0xFD) == 0) return false;
    auto pending = [&] {
        return iface.at<std::int32_t>(0x04) != iface.at<std::int32_t>(0x20) ||
               iface.at<std::int32_t>(0x1C) != iface.at<std::int32_t>(0x38);
    };
    if (!pending()) host.front_end_manager_push_request_004cc460(iface, 0x34);
    if (pending()) host.object_virtual_2(iface, 0x10, iface.at<std::int32_t>(0x20), iface.at<std::int32_t>(0x38));
    iface.at<std::uint8_t>(0xFD) = 0;
    return true;
}

void ingame_interface_request_scene_for_screen54_0068aa90(Raw iface, Fe5Host& host) {
    if (static_cast<const std::uint8_t*>(iface.ptr(0x54))[4] == 0) return;
    const std::int32_t none = 0;
    host.front_end_screens_set_level2_004f85d0(&none);
    host.game_input_contexts_level2_004d8ae0(&none);
    host.ingame_interface_request_scene_for_unit_0068aa40(iface, host.game_controlled_vehicle_004b4b00());
}

void ingame_interface_push_controlled_unit_request_0068aad0(Raw iface, Fe5Host& host) {
    const Raw unit = host.controlled_unit_00e188d8();
    if (host.unit_is_kind_5c(unit, 9) || host.unit_is_kind_5c(unit, 0x45)) {
        host.front_end_manager_push_request_arg_004cc460(iface, 0x32, unit.p);
    } else if (host.unit_is_kind_5c(unit, 0x46)) {
        host.front_end_manager_push_request_arg_004cc460(iface, 0x31, unit.p);
    }
}

void ingame_interface_leave_unit_request_level_0068ab30(Raw iface, Fe5Host& host) {
    const std::int32_t req = iface.at<std::int32_t>(0x20);
    if (req != 0x32 && req != 0x31) return;
    if (iface.at<std::int32_t>(0x04) != req || iface.at<std::int32_t>(0x1C) != iface.at<std::int32_t>(0x38)) return;
    host.front_end_manager_push_request_arg_004cc460(iface, 0x20, host.controlled_unit_00e188d8().p);
    if (iface.at<std::int32_t>(0x04) != iface.at<std::int32_t>(0x20) ||
        iface.at<std::int32_t>(0x1C) != iface.at<std::int32_t>(0x38)) {
        host.object_virtual_2(iface, 0x10, iface.at<std::int32_t>(0x20), iface.at<std::int32_t>(0x38));
    }
}

void ingame_interface_restore_hud_screen_set_0068ac30(Raw iface, Fe5Host& host) {
    if (host.byte_00e08880() == 0) {
        host.sound_request_queue_004c1b90().at<std::uint8_t>(0x1C) = 1;
        return;
    }
    if (static_cast<const std::uint8_t*>(iface.ptr(0x54))[4] != 0) return;
    ingame_interface_leave_unit_request_level_0068ab30(iface, host);
    const Raw hud_root{static_cast<std::uint8_t*>(iface.ptr(0x40))};
    hud_root.at<std::uint32_t>(0xEC) = hud_root.at<std::uint32_t>(0x1C);
    hud_root.ptr(0xF0) = host.controlled_unit_00e188d8().p;
    host.front_end_screens_set_level2_004f85d0(kHudLevel2Screens.data());
    host.game_input_contexts_level2_004d8ae0(kHudLevel2Contexts.data());
}

void ingame_interface_collapse_level3_when_spectating_0068b3f0(Raw iface, Fe5Host& host) {
    const Raw support{static_cast<std::uint8_t*>(iface.ptr(0xBC))};
    if (!host.support_screen_query_006529e0(support)) return;
    Raw{static_cast<std::uint8_t*>(host.ingame_interface_00e198c4().ptr(0xBC))}.at<std::uint8_t>(0x30) = 0;
    collapse_level3(iface, host);
}

void collapse_level3(Raw iface, Fe5Host& host) {
    if (host.global_vector_00e18d1c_count() == 0) return;
    const Raw global_support{static_cast<std::uint8_t*>(host.ingame_interface_00e198c4().ptr(0xBC))};
    if (global_support.at<std::uint8_t>(0x30) != 0) {
        Raw{static_cast<std::uint8_t*>(iface.ptr(0xBC))}.at<std::uint8_t>(0x0A) = 1;
    }
    const std::int32_t none = 0;
    host.front_end_screens_set_level3_004f8670(&none);
    host.game_input_contexts_level3_004d8b70(&none);
    host.ingame_interface_request_scene_for_unit_0068aa40(iface, host.game_controlled_vehicle_004b4b00());
}

void ingame_interface_back_out_one_overlay_level_0068b470(Raw iface, Fe5Host& host) {
    collapse_level3(iface, host);
    host.binoculars_reset_0051e8e0();
    if (static_cast<const std::uint8_t*>(iface.ptr(0x54))[4] == 0) {
        ingame_interface_restore_hud_screen_set_0068ac30(iface, host);
        return;
    }
    const std::int32_t none = 0;
    host.front_end_screens_set_level2_004f85d0(&none);
    host.game_input_contexts_level2_004d8ae0(&none);
    host.ingame_interface_request_scene_for_unit_0068aa40(iface, host.game_controlled_vehicle_004b4b00());
}

void ingame_interface_toggle_tactical_overlay_0068c0b0(Raw iface, void* target, Fe5Host& host) {
    const Raw support{static_cast<std::uint8_t*>(iface.ptr(0xBC))};
    if (!host.support_has_available_entries_0066ffa0(support)) {
        if (host.support_screen_query_006529e0(support)) {
            collapse_level3(iface, host);
            return;
        }
    } else if (!host.support_screen_query_00652a30(support)) {
        if (host.support_screen_query_006529e0(support)) {
            collapse_level3(iface, host);
            return;
        }
        if (host.ingame_interface_build_tactical_overlay_list_0068bc80(iface)) {
            const auto* overlay = static_cast<const std::uint8_t*>(iface.ptr(0x64));
            if (overlay[0x82] != 0 || overlay[0x81] != 0) return;
            host.front_end_screens_set_level3_004f8670(kSupportLevel3Screens.data());
            host.game_input_contexts_level3_004d8b70(host.game_byte_19c4() == 0 ? kSupportLevel3Contexts.data()
                                                                                 : kSupportLevel3ContextsAlt.data());
            host.support_set_tracked_unit_target_0065be80(support, target);
            return;
        }
    }
    host.support_play_unavailable_sound_0065f170(support);
}

void* hud_root_screen_construct_0068bf60(Raw screen, Fe5Host& host) {
    host.front_end_screen_construct_004f7180(screen);
    screen.at<std::uint32_t>(0x08) = kCallbackOwnerBaseVtable;
    screen.at<std::uint32_t>(0x0C) = 0;
    screen.at<std::uint32_t>(0x10) = 0;
    screen.at<std::uint32_t>(0x14) = 0;
    screen.at<std::uint8_t>(0x18) = 0;
    screen.at<std::uint32_t>(0x00) = kHudRootVtable;
    screen.at<std::uint32_t>(0x08) = kHudRootSecondaryVtable;
    screen.at<std::uint32_t>(0x70) = 0;
    screen.at<std::uint32_t>(0x74) = 0;
    for (std::size_t base : {0x90u, 0xA0u, 0xB0u}) {
        for (std::size_t k = 0; k < 3; ++k) screen.at<std::uint32_t>(base + 4 * k) = 0;
    }
    screen.at<std::uint8_t>(0xC2) = 0;
    screen.at<std::uint16_t>(0xC4) = 0xFFFF;
    screen.at<std::uint32_t>(0xD4) = 0;
    screen.at<std::uint32_t>(0xD8) = 0;
    screen.at<std::uint32_t>(0xDC) = 0;
    screen.at<std::uint32_t>(0xE4) = 0;
    screen.at<std::uint8_t>(0xE0) = 1;
    screen.at<std::uint32_t>(0xD0) = kObserverBaseVtable;
    for (std::size_t base = 0x108; base <= 0x168; base += 0x10) {
        for (std::size_t k = 0; k < 3; ++k) screen.at<std::uint32_t>(base + 4 * k) = 0;
    }
    return screen.p;
}

void* title_screen_construct_0068d760(Raw title, Fe5Host& host) {
    host.front_end_manager_construct_base_00684e10(title);
    title.at<std::uint32_t>(0) = kTitleScreenVtable;
    host.set_title_screen_global_00e198c8(title.p);
    title.at<std::uint32_t>(0x40) = 0;
    return title.p;
}

void title_screen_activate_and_show_0068d7b0(Raw title, Fe5Host& host) {
    host.front_end_manager_activate_00684700(title);
    const Raw screen{static_cast<std::uint8_t*>(title.ptr(0x40))};
    screen.at<std::uint8_t>(4) = 1;
    screen.at<std::uint8_t>(5) = 1;
    host.front_end_screen_commit_visibility_004f83b0(screen);
    host.object_virtual(screen, 0x18);
}

void title_screen_destruct_0068d7e0(Raw title, Fe5Host& host) {
    title.at<std::uint32_t>(0) = kTitleScreenVtable;
    host.set_title_screen_global_00e198c8(nullptr);
    if (title.ptr(0x40) != nullptr) {
        host.object_virtual_int(Raw{static_cast<std::uint8_t*>(title.ptr(0x40))}, 0x0C, 1);
        title.ptr(0x40) = nullptr;
    }
    host.front_end_manager_destroy_base_00684fa0(title);
}

void title_screen_skip_0068d8a0(Fe5Host& host) {
    host.storage_operation_reset_availability_00bd3450();
    if (host.game_dword_5e8() != 0) return;
    host.game_request_state_004d7920(4);
    host.set_game_byte_5ec(0);
}

void gui_blinker_stop_0068d9b0(Raw blinker, Fe5Host& host) {
    blinker.at<std::uint8_t>(4) = 0;
    if (blinker.ptr(0) != nullptr) host.widget_set_visible_34(blinker.ptr(0), false);
    blinker.at<float>(8) = 0.0f;
    blinker.at<float>(0x0C) = kBlinkerPeriod;
}

void gui_blinker_update_0068da10(Raw blinker, float dt, Fe5Host& host) {
    if (blinker.at<std::uint8_t>(4) == 0) return;
    const float t = static_cast<float>(static_cast<double>(blinker.at<float>(8)) - dt);
    blinker.at<float>(8) = t;
    if (!(0.0f >= t)) return;
    void* const widget = blinker.ptr(0);
    host.widget_set_visible_34(widget, !host.widget_is_visible_38(widget));
    blinker.at<float>(8) = static_cast<float>(
        static_cast<double>(blinker.at<float>(0x0C)) * kBlinkerHalf + static_cast<double>(blinker.at<float>(8)));
}

void plane_spawn_screen_enter_0060c870(Fe5Host& host) {
    warning_screen_show_wait_00681e90(Raw{static_cast<std::uint8_t*>(host.ingame_interface_00e198c4().ptr(0xC4))},
                                      host);
}

void unit_doing_icon_set_unit_0068daa0(Raw icon, void* unit, Fe5Host& host) {
    if (icon.ptr(0x18) != nullptr) host.observer_unregister_006952a0(icon.ptr(0x18), icon.p);
    icon.ptr(0x18) = unit;
    host.observer_register_00694a60(unit, icon.p);
    icon.at<std::uint32_t>(0x1C) = Raw{static_cast<std::uint8_t*>(unit)}.at<std::uint32_t>(0x538);
}

void unit_doing_icon_clear_unit_0068dad0(Raw icon, std::uint32_t value, Fe5Host& host) {
    if (icon.ptr(0x18) != nullptr) host.observer_unregister_006952a0(icon.ptr(0x18), icon.p);
    icon.at<std::uint32_t>(0x1C) = value;
    icon.ptr(0x18) = nullptr;
}

void* gui_widget_as_kind_0068db40(void* widget, Fe5Host& host) {
    if (widget == nullptr) return nullptr;
    return host.widget_is_kind_0c(widget, host.gui_widget_kind_00ab1060()) ? widget : nullptr;
}

void unit_doing_icon_set_local_position_0068db80(Raw icon, float x, float y, Fe5Host& host) {
    const float position[3] = {x, y, 0.0f};
    host.widget_set_local_position_00aa7dc0(icon.ptr(0x20), position);
}

void unit_doing_icon_set_widget_scale_0068dc50(Raw icon, float scale, Fe5Host& host) {
    const float pair[2] = {scale, scale};
    host.widget_virtual_48(icon.ptr(0x20), pair);
}

void unit_doing_icon_update_0068dc80(Raw icon, Fe5Host& host) {
    void* const doing = icon.ptr(0x24);
    const bool shown_mode = icon.at<std::uint8_t>(0x14) != 0 || host.session_mode_1fe4() != 2;
    if (shown_mode && icon.ptr(0x18) != nullptr) {
        const Raw unit{static_cast<std::uint8_t*>(icon.ptr(0x18))};
        if (!host.unit_is_squadron_flight_leader_007b8ad0(unit) && unit.ptr(0xDF4) != nullptr) {
            const Raw task{static_cast<std::uint8_t*>(unit.ptr(0xDF4))};
            void* image = nullptr;
            if (task.ptr(0x5C) != nullptr) {
                image = host.object_virtual_ret(Raw{static_cast<std::uint8_t*>(*static_cast<void**>(task.ptr(0x58)))}, 0x10);
            }
            host.widget_set_image_88(doing, image, 0, 1.0f);
            host.widget_set_visible_34(doing, true);
            return;
        }
    }
    host.widget_set_visible_34(doing, false);
}

void unit_doing_icon_bind_0068dde0(Raw icon, void* widget, Fe5Host& host) {
    icon.ptr(0x20) = widget;
    icon.ptr(0x24) = gui_widget_as_kind_0068db40(host.gui_find_child_00aa7e00(widget, "Doing_Icon"), host);
    icon.ptr(0x5C) = gui_widget_as_kind_0068db40(host.gui_find_child_00aa7e00(icon.ptr(0x20), "Ammo_Icon"), host);
    icon.at<std::uint8_t>(0x14) = 0;
    host.widget_set_visible_34(icon.ptr(0x20), false);
}

void gui_find_icon_child_0068df60(void* parent, const char* name, void** out, Fe5Host& host) {
    *out = host.gui_find_child_00aa7e00(parent, std::string(name) + "_Icon");
}
// ---------------------------------------------------------------------------
// Award tracker hint queue
// ---------------------------------------------------------------------------
void award_tracker_finish_active_hint_0068e420(Raw tracker, Fe5Host& host) {
    host.native_string_resize_0041dd40(Raw{tracker.p + 0x60}, 0, 0);
}

std::string award_tracker_copy_active_hint_name_0068e5b0(Raw tracker, Fe5Host& host) {
    return host.native_string_read(Raw{tracker.p + 0x60});
}

bool native_name_equal(const std::string& a, const std::string& b) noexcept {
    if (a.size() != b.size()) return false;
    if (a.empty()) return true;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lower = [](char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

bool award_tracker_has_hint_been_shown_0068e600(Raw tracker, const std::string& name, Fe5Host& host) {
    if (host.award_stored_hint_contains_007f8e00(name)) return true;
    void* const head = tracker.ptr(0x4C);
    for (void* node = *static_cast<void**>(head); node != head; node = *static_cast<void**>(node)) {
        if (native_name_equal(host.native_string_read(Raw{static_cast<std::uint8_t*>(node) + 8}), name)) return true;
    }
    return false;
}

void native_string_deque_pop_front_0068e780(Raw deque, Fe5Host& host) {
    if (deque.at<std::uint32_t>(0x10) == 0) return;
    const std::uint32_t offset = deque.at<std::uint32_t>(0x0C);
    auto* const block = static_cast<std::uint8_t*>(static_cast<void**>(deque.ptr(4))[offset >> 1]);
    host.native_string_release(Raw{block + (offset & 1) * 8});
    deque.at<std::uint32_t>(0x0C) = offset + 1;
    if (deque.at<std::uint32_t>(8) * 2 <= deque.at<std::uint32_t>(0x0C)) deque.at<std::uint32_t>(0x0C) = 0;
    if (--deque.at<std::uint32_t>(0x10) == 0) deque.at<std::uint32_t>(0x0C) = 0;
}

void alias_list_insert_copy_0068e850(Raw list, const void* value, Fe5Host& host) {
    host.alias_list_push_back_copy(list, value);
}

void award_tracker_clear_hint_queue_0068ea00(Raw tracker, Fe5Host& host) {
    host.pooled_string_list_clear_004d05e0(Raw{tracker.p + 0x48});
    while (tracker.at<std::uint32_t>(0x78) != 0) native_string_deque_pop_front_0068e780(Raw{tracker.p + 0x68}, host);
}

void award_tracker_touch_recent_hint_0068ea70(Raw tracker, Raw name, Fe5Host& host) {
    const Raw list{tracker.p + 0x54};
    void* const head = tracker.ptr(0x58);
    const std::string wanted = host.native_string_read(name);
    void* node = *static_cast<void**>(head);
    for (; node != head; node = *static_cast<void**>(node)) {
        if (native_name_equal(host.native_string_read(Raw{static_cast<std::uint8_t*>(node) + 8}), wanted)) break;
    }
    if (node != head) host.alias_list_erase_node(list, node);
    host.alias_list_push_back_copy(list, name.p);
    host.native_string_release(name);
}

bool award_tracker_is_active_hint_blocking_00690b00(Raw tracker, Raw name, Fe5Host& host) {
    if (name.at<std::uint32_t>(0) == 0) return false;
    return host.award_hint_definition_005ffb20(Raw{tracker.p + 0x30}, host.native_string_read(name)).at<std::uint8_t>(0x114) != 0;
}

void award_tracker_hint_map_pair_00690fb0(Raw tracker, std::uint32_t key, std::uint32_t value, Fe5Host& host) {
    host.award_hint_map_assign_00690fb0(Raw{tracker.p + 0x7C}, key, value);
}

void award_tracker_trigger_forced_hint_00692fb0(Raw tracker, std::uint32_t name, Fe5Host& host) {
    tracker.at<std::uint8_t>(4) = 1;
    host.award_tracker_trigger_hint_forced_00690fd0(tracker, name, 1);
}
// ---------------------------------------------------------------------------
// Engine dword vector
// ---------------------------------------------------------------------------
namespace {
std::uint32_t* dword_data(Raw v) { return static_cast<std::uint32_t*>(v.ptr(0)); }

// The reallocation block shared by 00693990, 00693FD0 and 00694060.
void reallocate_dwords(Raw v, std::uint32_t capacity, Fe5Host& host) {
    v.at<std::uint32_t>(8) = capacity;
    auto* const block = static_cast<std::uint32_t*>(host.operator_new(static_cast<std::size_t>(capacity) * 4));
    for (std::uint32_t i = 0; i < v.at<std::uint32_t>(4); ++i) {
        if (block + i != nullptr) block[i] = dword_data(v)[i];
    }
    if (v.ptr(0) != nullptr) host.crt_free(v.ptr(0));
    v.ptr(0) = block;
}
}  // namespace

void* native_dword_vector_copy_construct_006938d0(Raw dst, Raw src, Fe5Host& host) {
    const std::uint32_t size = src.at<std::uint32_t>(4);
    if (size == 0) {
        dst.ptr(0) = nullptr;
        dst.at<std::uint32_t>(8) = 0;
        dst.at<std::uint32_t>(4) = 0;
        return dst.p;
    }
    dst.at<std::uint32_t>(8) = size;
    dst.ptr(0) = host.operator_new(static_cast<std::size_t>(size) * 4);
    dst.at<std::uint32_t>(4) = dst.at<std::uint32_t>(8);
    for (std::uint32_t i = 0; i < dst.at<std::uint32_t>(4); ++i) {
        if (dword_data(dst) + i != nullptr) dword_data(dst)[i] = dword_data(src)[i];
    }
    return dst.p;
}

void native_dword_vector_grow_00693990(Raw vector, Fe5Host& host) {
    reallocate_dwords(vector, vector.at<std::uint32_t>(8) * 2 + 2, host);
}

void native_dword_vector_resize_00693fd0(Raw vector, std::uint32_t size, Fe5Host& host) {
    if (vector.at<std::uint32_t>(4) < size) {
        if (vector.at<std::uint32_t>(8) < size) reallocate_dwords(vector, size, host);
        for (std::uint32_t i = vector.at<std::uint32_t>(4); i < size; ++i) {
            if (dword_data(vector) + i != nullptr) dword_data(vector)[i] = 0;
        }
    }
    vector.at<std::uint32_t>(4) = size;
}

void native_dword_vector_assign_00694060(Raw dst, Raw src, Fe5Host& host) {
    if (dst.at<std::uint32_t>(4) != 0) dst.at<std::uint32_t>(4) = 0;
    if (dst.at<std::uint32_t>(8) < src.at<std::uint32_t>(4)) reallocate_dwords(dst, src.at<std::uint32_t>(4), host);
    dst.at<std::uint32_t>(4) = src.at<std::uint32_t>(4);
    for (std::uint32_t i = 0; i < src.at<std::uint32_t>(4); ++i) {
        if (dword_data(dst) + i != nullptr) dword_data(dst)[i] = dword_data(src)[i];
    }
}
// ---------------------------------------------------------------------------
// Observer bookkeeping
// ---------------------------------------------------------------------------
namespace {
struct ObserverLockScope {
    Fe5Host& host;
    void* cs;
    explicit ObserverLockScope(Fe5Host& h)
        : host(h), cs(Raw{static_cast<std::uint8_t*>(h.observer_lock_00694280())}.ptr(4)) {
        if (cs != nullptr) {
            host.critical_section_enter(cs);
            ++Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18);
        }
    }
    ~ObserverLockScope() {
        if (cs != nullptr) {
            --Raw{static_cast<std::uint8_t*>(cs)}.at<std::int32_t>(0x18);
            host.critical_section_leave(cs);
        }
    }
    ObserverLockScope(const ObserverLockScope&) = delete;
    ObserverLockScope& operator=(const ObserverLockScope&) = delete;
};
}  // namespace

void* observer_singleton_scalar_delete_00694250(Raw owner, std::uint8_t flags, Fe5Host& host) {
    host.set_observer_singleton_00e198e0(nullptr);
    owner.at<std::uint32_t>(0) = kSingletonBaseVtable;
    if (flags & 1) host.crt_free(owner.p);
    return owner.p;
}

std::uint32_t observer_find_pair_value_locked_00694b90(void* observed, void* observer, Fe5Host& host) {
    ObserverLockScope lock(host);
    const Raw pair = host.observer_find_pair_006949d0(observed, observer);
    return pair.p != nullptr ? pair.at<std::uint32_t>(0x0C) : 0;
}

void observer_lock_owner_destruct_00694e70(Raw owner, Fe5Host& host) {
    owner.at<std::uint32_t>(0) = kObserverLockOwnerVtable;
    host.critical_section_destroy_owned_0041cc80(Raw{owner.p + 4});
    host.set_observer_singleton_00e198e0(nullptr);
    owner.at<std::uint32_t>(0) = kSingletonBaseVtable;
}

std::uint32_t observer_ref_get_target_locked(Raw ref, Fe5Host& host) {
    ObserverLockScope lock(host);
    return ref.at<std::uint32_t>(8);
}

void observer_pair_destroy_00695000(Raw pair, Fe5Host& host) {
    host.observer_edge_array_erase_00694f60(Raw{static_cast<std::uint8_t*>(pair.ptr(4)) + 4}, pair.p);
    host.observer_edge_array_erase_00694f60(Raw{static_cast<std::uint8_t*>(pair.ptr(8)) + 4}, pair.p);
    host.object_virtual_int(pair, 0x00, 1);
}

void observer_pair_release_00695030(Raw pair, Fe5Host& host) {
    if (--pair.at<std::int32_t>(0x0C) == 0) observer_pair_destroy_00695000(pair, host);
}

void* move_dwords_00695230(const void* first, const void* last, void* dest, Fe5Host& host) {
    const std::ptrdiff_t count = (static_cast<const std::uint8_t*>(last) - static_cast<const std::uint8_t*>(first)) >> 2;
    const std::size_t bytes = static_cast<std::size_t>(count) * 4;
    if (count != 0) host.crt_memmove_s(dest, bytes, first, bytes);
    return static_cast<std::uint8_t*>(dest) + bytes;
}

void observer_unregister_if_registered_006956a0(void* observed, void* observer, Fe5Host& host) {
    ObserverLockScope lock(host);
    if (host.observer_is_registered_00694af0(observed, observer)) host.observer_unregister_006952a0(observed, observer);
}

void callback_owner_detach_all_if_bound_00695830(Raw owner, Fe5Host& host) {
    void* bound = nullptr;
    {
        ObserverLockScope lock(host);
        bound = owner.ptr(8);
    }
    if (bound != nullptr) host.callback_owner_detach_all_00695530(owner);
}

void observer_purge_locked_00695e40(Fe5Host& host) {
    ObserverLockScope lock(host);
    host.observer_purge_00695c50(0x100);
}
// ---------------------------------------------------------------------------
// Segment 24 plane screens
// ---------------------------------------------------------------------------
void game_submit_main_scene_render_0068a0d0(Fe5Host& host) {
    const Raw game = host.game_00e188a8();
    host.game_update_view_0078cff0(game.ptr(0x19FC), host.real_clock_00f876a4());
    host.scene_renderer_submit_00b101c0(game.ptr(0x19EC), game.ptr(0x19FC));
    host.game_after_render_submit_004c7e60();
}

void plane_effects_set_visible_00604bc0(Raw screen, bool visible, Fe5Host& host) {
    host.widget_set_visible_34(screen.ptr(0xB4), visible);
    host.widget_set_visible_34(screen.ptr(0xB8), visible);
    host.widget_set_visible_34(screen.ptr(0x0C), visible);
}

void plane_screen_hide_indicator_grid_00604c20(Raw screen, Fe5Host& host) {
    if (screen.ptr(0x88) != nullptr) host.widget_set_visible_34(screen.ptr(0x88), false);
    for (std::size_t i = 0; i < 6 * 5; ++i) {
        void* const w = screen.ptr(0x10 + 4 * i);
        if (w != nullptr) host.widget_set_visible_34(w, false);
    }
}

void plane_screen_release_plane_view_00604df0(Raw screen, Fe5Host& host) {
    screen.at<std::uint8_t>(0x104) = 0;
    if (screen.ptr(0xB8) == nullptr) return;
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0xB8))};
    const Raw iface = host.ingame_interface_00e198c4();
    if (iface.p != nullptr && ingame_interface_screen_8c_shown_0068a120(iface)) {
        host.unit_send_role_transfer_0077c470(plane, 0x1FF, 0);
        host.unit_send_role_transfer_0077c470(plane, 1, 1);
    } else {
        host.unit_send_role_transfer_0077c470(plane, 0x1FE, 0);
    }
    host.object_virtual(plane, 0x1C8);
    host.game_set_camera_near_plane_004b4940(kPlaneReleaseNearPlane);
}

void plane_screen_on_object_released(Raw screen, const void* object) noexcept {
    for (std::size_t off : {0xB0u, 0x8Cu, 0x90u}) {
        if (screen.ptr(off) == object) {
            screen.ptr(off) = nullptr;
            return;
        }
    }
}

std::uint32_t plane_screen_cockpit_view_flag_00604f30(Raw screen, Fe5Host& host) {
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0xB8))};
    if (plane.p == nullptr || host.unit_is_kind_5c(plane, 0x10) || host.unit_is_kind_5c(plane, 0x16)) return 0;
    return screen.at<std::uint32_t>(0xFC);
}

void owned_buffer_free_00604fc0(Raw cell, Fe5Host& host) {
    if (cell.ptr(0) == nullptr) return;
    host.crt_free(cell.ptr(0));
    cell.ptr(0) = nullptr;
}

void owned_object_delete(Raw cell, Fe5Host& host) {
    if (cell.ptr(0) == nullptr) return;
    host.object_virtual_int(Raw{static_cast<std::uint8_t*>(cell.ptr(0))}, 0x00, 1);
    cell.ptr(0) = nullptr;
}

void plane_screen_update_payload_icon_00605380(Raw screen, Fe5Host& host) {
    const std::int32_t frame = host.plane_screen_payload_frame_00604c80(screen);
    const bool shown = frame != -1;
    if (shown) {
        host.widget_set_frame_88(screen.ptr(0x110), frame, 0, 1.0f);
    }
    host.widget_set_visible_34(screen.ptr(0x100), shown);
    host.widget_set_visible_34(screen.ptr(0x104), shown);
    host.widget_set_visible_34(screen.ptr(0x110), shown);
}

void gauge_needle_set_value_00605410(Raw needle, float value, Fe5Host& host) {
    const float maximum = needle.at<float>(0x18);
    const float clamped = (maximum > value) ? value : maximum;
    needle.at<float>(0x1C) = clamped;
    const float angle = static_cast<float>(static_cast<double>(clamped) / needle.at<float>(0x18) *
                                               needle.at<float>(0x00) * needle.at<float>(0x14) +
                                           needle.at<float>(0x10));
    float a = static_cast<float>(host.crt_fmod_00bf857a(angle, kNeedleTwoPi));
    if (!(-kNeedlePi < static_cast<double>(a))) {
        a = static_cast<float>(static_cast<double>(a) + kNeedleTwoPi);
    } else if (static_cast<double>(a) > kNeedlePi) {
        a = static_cast<float>(static_cast<double>(a) - kNeedleTwoPi);
    }
    host.widget_set_rotation_44(needle.ptr(4), a);
}

void refcounted_ptr_release_00605600(Raw cell, Fe5Host& host) {
    if (cell.ptr(0) == nullptr) return;
    host.refcounted_release(cell.ptr(0));
    cell.ptr(0) = nullptr;
}

void plane_effects_exit_00605cc0(Raw screen, Fe5Host& host) {
    screen.at<std::uint8_t>(0x104) = 0;
    screen.at<std::uint32_t>(0xFC) = 0;
    plane_screen_release_plane_view_00604df0(screen, host);
    for (std::size_t off : {0xE8u, 0xECu}) {
        const Raw flash{static_cast<std::uint8_t*>(screen.ptr(off))};
        flash.at<std::uint8_t>(0) = 0;
        flash.at<std::uint32_t>(8) = 0;
        flash.at<std::uint32_t>(0x0C) = 0;
        flash.at<std::uint32_t>(0x10) = 0;
        flash.at<std::uint32_t>(4) = 0;
    }
    host.game_set_view_fov_004dc940(1.0f, 2);
    if (screen.ptr(0xB8) != nullptr) host.object_virtual(Raw{static_cast<std::uint8_t*>(screen.ptr(0xB8))}, 0x1C8);
    host.game_set_camera_near_plane_004b4940(kPlaneReleaseNearPlane);
    for (std::size_t off : {0x64u, 0x6Cu, 0x70u}) {
        const Raw effect{static_cast<std::uint8_t*>(screen.ptr(off))};
        if (effect.p == nullptr) continue;
        host.point_effect_stop_children_00867b10(effect);
        effect.at<std::uint8_t>(9) = 1;
        refcounted_ptr_release_00605600(Raw{screen.p + off}, host);
        screen.ptr(off) = nullptr;
    }
    host.render_set_transition_00b0d6a0(screen.at<float>(0x88), screen.at<float>(0x8C), screen.at<float>(0x90),
                                        0.0f, 0.0f, 0.0f);
}

void plane_screen_ensure_chase_camera_00605e50(Raw screen, Fe5Host& host) {
    if (screen.ptr(0x94) == nullptr) {
        const Raw camera = host.plane_chase_camera_new_00605130();
        screen.ptr(0x94) = camera.p;
        const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};  // 00D7A24C = 1.0
        host.camera_add_to_scene_00923870(camera, host.game_00e188a8().ptr(0x19CC), identity);
        host.observer_register_00694a60(screen.ptr(0x94), screen.p + 8);
    }
    const Raw camera{static_cast<std::uint8_t*>(screen.ptr(0x94))};
    host.camera_set_target_0042aaf0(camera, screen.ptr(0xB8));
    host.game_set_camera_mover_004bc410(camera.p);
    host.camera_virtual_dc(camera, 0.0f);
    screen.at<std::uint32_t>(0x98) = 0;
    if (screen.ptr(0xB8) != nullptr) host.object_virtual(Raw{static_cast<std::uint8_t*>(screen.ptr(0xB8))}, 0x1C8);
    host.game_set_camera_near_plane_004b4940(kPlaneReleaseNearPlane);
}

void* unit_as_kind_or_null(void* unit, int kind, Fe5Host& host) {
    if (unit == nullptr) return nullptr;
    return host.unit_is_kind_5c(Raw{static_cast<std::uint8_t*>(unit)}, kind) ? unit : nullptr;
}

bool input_action_enabled_00606250(std::int32_t action, Fe5Host& host) {
    const auto* table = static_cast<const std::uint8_t*>(host.input_manager_004bec00().ptr(4));
    return table[action * 0x30 + 1] != 0;
}

void plane_effects_destruct_00606040(Raw screen, Fe5Host& host) {
    screen.at<std::uint32_t>(0) = kPlaneEffectsVtable;
    screen.at<std::uint32_t>(8) = kPlaneEffectsSecondaryVtable;
    for (std::size_t off : {0x64u, 0x6Cu, 0x70u}) {
        const Raw effect{static_cast<std::uint8_t*>(screen.ptr(off))};
        if (effect.p == nullptr) continue;
        host.point_effect_stop_children_00867b10(effect);
        effect.at<std::uint8_t>(9) = 1;
    }
    host.crt_free(screen.ptr(0xE8));
    host.crt_free(screen.ptr(0xEC));
    host.target_observer_destruct_0042bca0(Raw{screen.p + 0xD0});
    for (std::size_t off = 0x78; off >= 0x60; off -= 4) refcounted_ptr_release_00605600(Raw{screen.p + off}, host);
    host.callback_owner_destroy_00695870(Raw{screen.p + 8});
    host.front_end_screen_destruct_004f71a0(screen);
}

void* plane_effects_construct_00606470(Raw screen, Fe5Host& host) {
    host.front_end_screen_construct_004f7180(screen);
    screen.at<std::uint32_t>(0x08) = kCallbackOwnerBaseVtable;
    screen.at<std::uint32_t>(0x0C) = 0;
    screen.at<std::uint32_t>(0x10) = 0;
    screen.at<std::uint32_t>(0x14) = 0;
    screen.at<std::uint8_t>(0x18) = 0;
    screen.at<std::uint32_t>(0x00) = kPlaneEffectsVtable;
    screen.at<std::uint32_t>(0x08) = kPlaneEffectsSecondaryVtable;
    for (std::size_t off = 0x60; off <= 0x78; off += 4) screen.at<std::uint32_t>(off) = 0;
    screen.at<std::uint32_t>(0x94) = 0;
    screen.at<std::uint32_t>(0x98) = 0;
    screen.at<std::uint32_t>(0xB8) = 0;
    screen.at<std::uint8_t>(0xBD) = 0;
    screen.at<std::uint32_t>(0xC4) = 0;
    screen.at<std::uint32_t>(0xC8) = 0;
    screen.at<std::uint32_t>(0xD4) = 0;
    screen.at<std::uint32_t>(0xD8) = 0;
    screen.at<std::uint32_t>(0xDC) = 0;
    screen.at<std::uint32_t>(0xE4) = 0;
    screen.at<std::uint8_t>(0xE0) = 1;
    screen.at<std::uint32_t>(0xD0) = kObserverBaseVtable;
    for (std::size_t off : {0xE8u, 0xECu}) {
        auto* const flash = static_cast<std::uint8_t*>(host.operator_new(0x14));
        if (flash != nullptr) {
            const Raw f{flash};
            f.at<std::uint8_t>(0) = 0;
            f.at<std::uint32_t>(8) = 0;
            f.at<std::uint32_t>(0x0C) = 0;
            f.at<std::uint32_t>(0x10) = 0;
            f.at<std::uint32_t>(4) = 0;
        }
        screen.ptr(off) = flash;
    }
    screen.at<std::uint8_t>(0x104) = 0;
    screen.at<std::uint8_t>(0x10C) = 0;
    screen.at<std::uint32_t>(0x108) = 0;
    return screen.p;
}

void plane_screen_update_view_fov_006068a0(Raw screen, Fe5Host& host) {
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0xB8))};
    if (plane.p != nullptr && !host.unit_is_kind_5c(plane, 0x10) && !host.unit_is_kind_5c(plane, 0x16) &&
        screen.at<std::uint32_t>(0xFC) != 0) {
        screen.at<float>(0x100) = host.game_tuning_0042e740().at<float>(0x84);
    } else {
        screen.at<float>(0x100) = 1.0f;  // 00D7A24C
    }
    host.game_set_view_fov_004dc940(screen.at<float>(0x100), 2);
}

void stl_vector_tidy_00606920(Raw vector, Fe5Host& host) {
    if (vector.ptr(4) != nullptr) host.crt_free(vector.ptr(4));
    vector.ptr(4) = nullptr;
    vector.ptr(8) = nullptr;
    vector.ptr(0x0C) = nullptr;
}

void plane_screen_destruct_00606950(Raw screen, Fe5Host& host) {
    screen.at<std::uint32_t>(0) = kPlaneScreenVtable;
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0x0C));
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0xB8));
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0xB4));
    refcounted_ptr_release_00605600(Raw{screen.p + 0x15C}, host);
    refcounted_ptr_release_00605600(Raw{screen.p + 0x158}, host);
    for (std::size_t first : {0xA8u, 0x98u}) {
        if (screen.ptr(first) != nullptr) host.crt_free(screen.ptr(first));
        screen.ptr(first) = nullptr;
        screen.ptr(first + 4) = nullptr;
        screen.ptr(first + 8) = nullptr;
    }
    host.front_end_screen_destruct_004f71a0(screen);
}

void plane_screen_register_00606a90(Raw screen, Fe5Host& host) {
    host.front_end_screen_register_004f71d0(screen);
    screen.at<std::uint8_t>(0x120) = 0;
    screen.at<std::uint32_t>(0x124) = 0;
    screen.at<std::uint8_t>(0x128) = 0;
    screen.at<std::uint32_t>(0x12C) = 0;
    void* const acquired = host.effect_handle_acquire_00871ba0(kPlaneRumbleEffect);
    void* const old = screen.ptr(0x158);
    if (old != acquired) {
        screen.ptr(0x158) = acquired;
        if (acquired != nullptr) host.sound_handle_add_ref(acquired);
        if (old != nullptr) host.refcounted_release(old);
    }
    if (acquired != nullptr) host.refcounted_release(acquired);
    refcounted_ptr_release_00605600(Raw{screen.p + 0x15C}, host);
    screen.ptr(0x15C) = nullptr;
    screen.at<std::uint8_t>(0x160) = 0;
    screen.at<float>(0x164) = 0.0f;
    host.object_virtual(screen, 0x14);
    const float* const position = host.widget_local_position_00aa67f0(screen.ptr(0xC8));
    screen.at<float>(0x13C) = 0.0f;
    screen.at<float>(0x134) = 0.0f;
    screen.at<float>(0x114) = static_cast<float>(static_cast<double>(position[1]) + kPlaneScreenLineOffset);
    screen.at<std::uint8_t>(0x08) = 0;
    screen.at<std::uint8_t>(0x140) = 0;
    screen.at<std::uint8_t>(0x130) = 0;
    screen.at<float>(0x138) = 1.0f;  // 00D7A24C
    host.widget_set_alpha_4c(screen.ptr(0xE0), 0.0f);
}

void plane_screen_apply_view_mode_00608280(Raw screen, Fe5Host& host) {
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0xB8))};
    if (plane.p != nullptr && !host.unit_is_kind_5c(plane, 0x10) && !host.unit_is_kind_5c(plane, 0x16) &&
        screen.at<std::uint32_t>(0xFC) != 0) {
        plane_screen_ensure_cockpit_camera_00607f60(screen, host);
        return;
    }
    plane_screen_ensure_chase_camera_00605e50(screen, host);
}

void owned_vector_delete_006088a0(Raw cell, Fe5Host& host) {
    if (cell.ptr(0) == nullptr) return;
    const Raw vector{static_cast<std::uint8_t*>(cell.ptr(0))};
    stl_vector_tidy_00606920(vector, host);
    host.crt_free(vector.p);
    cell.ptr(0) = nullptr;
}

void plane_screen_select_camera_00608b60(Raw screen, Fe5Host& host) {
    screen.at<std::uint8_t>(0x104) = 0;
    screen.at<std::uint32_t>(0xFC) = 0;
    plane_screen_apply_view_mode_00608280(screen, host);
    if (screen.ptr(0xB8) == nullptr) return;
    if (!host.observer_is_registered_00694af0(screen.ptr(0xB8), screen.p + 8)) {
        host.observer_register_00694a60(screen.ptr(0xB8), screen.p + 8);
    }
    host.markers_screen_set_view_centre_00637620(0.5f, 0.5f);
}

bool bit_flag_set_test_flag_00608c00(Raw object, std::uint32_t flag, Fe5Host& host) {
    return host.bit_flag_set_test_0048f930(Raw{object.p + 0xC4}, flag);
}

void* plane_screen_construct_00608e60(Raw screen, Fe5Host& host) {
    host.front_end_screen_construct_004f7180(screen);
    screen.at<std::uint32_t>(0) = kPlaneScreenVtable;
    for (std::size_t off : {0x98u, 0x9Cu, 0xA0u, 0xA8u, 0xACu, 0xB0u, 0x158u, 0x15Cu}) screen.at<std::uint32_t>(off) = 0;
    host.widget_quad_fade_construct_0064a4f0(Raw{screen.p + 0x16C});
    screen.at<std::uint32_t>(0x10C) = 0;
    screen.at<std::uint32_t>(0x108) = 0;
    return screen.p;
}

void plane_screen_collect_children_00608ef0(Raw screen, void* out, Fe5Host& host) {
    host.push_page_slot_004d6790(out, &screen.ptr(0xB4));
    host.push_page_slot_004d6790(out, &screen.ptr(0x0C));
    host.push_page_slot_004d6790(out, &screen.ptr(0xB8));
}

void plane_screen_set_plane_00609390(Raw screen, void* plane, Fe5Host& host) {
    screen.ptr(0x168) = plane;
    if (screen.at<std::uint8_t>(5) == 0) return;
    plane_screen_rebuild_widgets_00608f30(screen, host);
    static_cast<std::uint8_t*>(screen.ptr(0x108))[0x18] = 0;
    static_cast<std::uint8_t*>(screen.ptr(0x10C))[0x18] = 0;
    plane_screen_update_payload_icon_00605380(screen, host);
}

void plane_spawn_camera_on_object_released_0060c620(Raw camera, const void* object) noexcept {
    if (camera.ptr(0x374) == object) {
        camera.ptr(0x374) = nullptr;
        return;
    }
    if (camera.ptr(0x378) == object) camera.ptr(0x378) = nullptr;
}

void hud_screen40_destruct_0060c650(Raw screen, Fe5Host& host) {
    screen.at<std::uint32_t>(0) = 0x00CF48C8;
    screen.at<std::uint32_t>(8) = 0x00CF48B0;
    host.callback_owner_destroy_00695870(Raw{screen.p + 8});
    host.front_end_screen_destruct_004f71a0(screen);
}

void hud_plane_slot_register_observers_0060c6c0(Raw screen, Fe5Host& host) {
    if (screen.ptr(0x20) == nullptr) return;
    host.observer_register_00694a60(screen.ptr(0x20), screen.p + 8);
    const Raw object{static_cast<std::uint8_t*>(host.object_virtual_ret(screen, 0x28))};
    if (object.p == nullptr) return;
    host.observer_register_00694a60(object.p, screen.p + 8);
    screen.ptr(0x24) = host.air_ops_block_006bcd20(object);
}

void hud_plane_slot_unregister_observers(Raw screen, Fe5Host& host) {
    if (screen.ptr(0x20) != nullptr) {
        host.observer_unregister_006952a0(screen.ptr(0x20), screen.p + 8);
        screen.ptr(0x20) = nullptr;
    }
    if (screen.ptr(0x24) != nullptr) {
        host.observer_unregister_006952a0(Raw{static_cast<std::uint8_t*>(screen.ptr(0x24))}.ptr(0x7C), screen.p + 8);
        screen.ptr(0x24) = nullptr;
    }
    if (screen.ptr(0x1C) != nullptr) {
        host.observer_unregister_006952a0(screen.ptr(0x1C), screen.p + 8);
        screen.ptr(0x1C) = nullptr;
    }
}

void* plane_spawn_camera_construct_0060c930(Raw camera, Fe5Host& host) {
    host.camera_mover_construct_base_00432750(camera);
    camera.at<std::uint32_t>(0x000) = 0x00CF4930;
    camera.at<std::uint32_t>(0x010) = 0x00CF4914;
    camera.at<std::uint32_t>(0x024) = 0x00CF490C;
    camera.at<std::uint32_t>(0x170) = 0x00CF48F4;
    return camera.p;
}

void plane_effects_enter_00608c30(Raw screen, Fe5Host& host) {
    screen.at<std::uint8_t>(0x104) = 0;
    plane_screen_update_view_fov_006068a0(screen, host);
    plane_screen_apply_view_mode_00608280(screen, host);
    plane_screen_select_camera_00608b60(screen, host);
    screen.at<float>(0x7C) = kPlaneEffectsUnsetTime;
    std::memset(screen.p + 0x1C, 0, 0x40);
    screen.at<std::uint32_t>(0x5C) = 0;
    const Raw view{static_cast<std::uint8_t*>(host.game_00e188a8().ptr(0x19FC))};
    if (view.p != nullptr) {
        screen.at<std::uint32_t>(0x80) = view.at<std::uint32_t>(0x1C4);
        screen.at<std::uint32_t>(0x84) = view.at<std::uint32_t>(0x1C4);
    }
    host.render_get_transition_00b0cfc0(screen.at<float>(0x88), screen.at<float>(0x8C), screen.at<float>(0x90));
    const Raw tuning = host.game_tuning_0042e740();
    host.render_set_transition_00b0d6a0(tuning.at<float>(0xD8), tuning.at<float>(0xDC), tuning.at<float>(0xE0),
                                        0.0f, 0.0f, 0.0f);
}

void plane_screen_bind_plane_00608d50(Raw screen, void* plane, Fe5Host& host) {
    screen.at<std::uint8_t>(0x104) = 0;
    plane_screen_release_plane_view_00604df0(screen, host);
    if (screen.ptr(0xB8) != nullptr && host.observer_is_registered_00694af0(screen.ptr(0xB8), screen.p + 8)) {
        host.observer_unregister_006952a0(screen.ptr(0xB8), screen.p + 8);
    }
    screen.ptr(0xB8) = plane;
    plane_screen_select_camera_00608b60(screen, host);
    screen.at<std::uint32_t>(0xA8) = 0;
    plane_screen_update_view_fov_006068a0(screen, host);
    plane_screen_apply_view_mode_00608280(screen, host);
    screen.at<std::uint8_t>(0x9C) = 0;
    if (screen.ptr(0x98) != nullptr) host.cockpit_camera_reset_0042aa00(Raw{static_cast<std::uint8_t*>(screen.ptr(0x98))});
    if (screen.ptr(0x94) != nullptr) {
        const Raw chase{static_cast<std::uint8_t*>(screen.ptr(0x94))};
        chase.at<std::uint32_t>(0x3E8) = 0;
        chase.at<std::uint32_t>(0x3E4) = 0;
    }
    screen.at<std::uint8_t>(0xBD) = 0;
    screen.at<std::uint8_t>(0xBC) = 0;
    screen.at<std::uint32_t>(0x108) =
        Raw{static_cast<std::uint8_t*>(host.input_manager_004bec00().ptr(4))}.at<std::uint32_t>(0x1EB4);
    screen.at<std::uint8_t>(0x10C) = 1;
}
// ---------------------------------------------------------------------------
// Segment 16 weapon-group screen
// ---------------------------------------------------------------------------
void weapon_group_reset_orbit_camera_00545270(Raw screen, Fe5Host& host) {
    if (screen.ptr(0x20) != nullptr) host.camera_orbit_target_from_yaw_0042c940(Raw{static_cast<std::uint8_t*>(screen.ptr(0x20))}, 0.0f);
}

void weapon_group_orbit_camera_005452b0(Raw screen, float yaw, Fe5Host& host) {
    if (screen.ptr(0x2C) != nullptr) host.camera_orbit_target_from_yaw_0042c940(Raw{static_cast<std::uint8_t*>(screen.ptr(0x2C))}, yaw);
}

void weapon_group_hide_slot_widgets_005452f0(Raw screen, Fe5Host& host) {
    for (std::size_t i = 0; i < 5 * 5; ++i) {
        void* const w = screen.ptr(0x5C + 4 * i);
        if (w != nullptr) host.widget_set_visible_34(w, false);
    }
    for (std::size_t off = 0xC0; off <= 0xCC; off += 4) host.widget_set_visible_34(screen.ptr(off), false);
}

void screen2e_set_view_centre_005454b0(Raw screen, float x, float y, float z, Fe5Host& host) {
    screen.at<float>(0x48) = x;
    screen.at<float>(0x4C) = y;
    screen.at<float>(0x50) = z;
    if (host.game_byte_19c4() == 0) host.markers_screen_set_view_centre_00637620(x, y);
}

void weapon_group_entry_destruct(Raw entry, std::uint32_t vtable, Fe5Host& host) {
    entry.at<std::uint32_t>(0) = vtable;
    if (entry.ptr(0x14) != nullptr) host.observer_unregister_006952a0(entry.ptr(0x14), entry.p);
    host.callback_owner_destroy_00695870(entry);
}

const float* static_colour_red_00545750(Fe5Host& host) {
    const Raw block = host.static_colour_block_00e192f4();
    if ((block.at<std::uint32_t>(0x10) & 1) == 0) {
        block.at<std::uint32_t>(0x10) |= 1;
        block.at<float>(0x00) = 1.0f;  // 00D7A24C
        block.at<float>(0x04) = 0.0f;
        block.at<float>(0x08) = 0.0f;
        block.at<float>(0x0C) = 1.0f;
    }
    return &block.at<float>(0);
}

bool gun_is_artillery_kind_005459b0(Raw gun) noexcept {
    const std::int32_t kind = Raw{static_cast<std::uint8_t*>(gun.ptr(0x3F4))}.at<std::int32_t>(0x80);
    return kind == 2 || kind == 3 || kind == 4 || kind == 6;
}

bool gun_is_torpedo_class_launcher_005459e0(Raw gun) noexcept {
    const std::int32_t kind = Raw{static_cast<std::uint8_t*>(gun.ptr(0x3F4))}.at<std::int32_t>(0x80);
    return kind == 5 || kind == 6;
}

void weapon_group_resume_00545ac0(Raw screen, Fe5Host& host) {
    screen.at<std::uint8_t>(0xD4) = 0;
    screen.at<std::uint8_t>(0x1C) = 1;
    for (std::int32_t i = 0; i < screen.at<std::int32_t>(0xE8); ++i) {
        host.widget_set_visible_34(static_cast<void**>(screen.ptr(0xE4))[i], false);
        host.widget_set_visible_34(static_cast<void**>(screen.ptr(0xF0))[i], false);
    }
    weapon_group_hide_slot_widgets_005452f0(screen, host);
}

std::int32_t entry_array_index_of_target_00545c20(Raw array, Raw entry) noexcept {
    auto* const first = static_cast<std::uint8_t*>(array.ptr(0));
    auto* const last = first + array.at<std::uint32_t>(4) * 0x18;
    for (std::uint8_t* p = first; p < last; p += 0x18) {
        if (Raw{p}.ptr(0x14) == entry.ptr(0x14)) return static_cast<std::int32_t>((p - first) / 0x18);
    }
    return -1;
}

void ptr_array_reserve(Raw array, std::int32_t capacity, Fe5Host& host) {
    if (capacity < 1) capacity = 1;
    if (array.at<std::int32_t>(8) >= capacity) return;
    auto* const block = static_cast<void**>(host.operator_new(static_cast<std::size_t>(capacity) * 4));
    for (std::int32_t i = 0; i < array.at<std::int32_t>(4); ++i) {
        if (block + i != nullptr) block[i] = static_cast<void**>(array.ptr(0))[i];
    }
    host.crt_free(array.ptr(0));
    array.ptr(0) = block;
    array.at<std::int32_t>(8) = capacity;
}

void ptr_array_push_back(Raw array, void* value, Fe5Host& host) {
    if (array.at<std::int32_t>(4) == array.at<std::int32_t>(8)) {
        std::int32_t grown = array.at<std::int32_t>(8) * 2;
        if (grown < 2) grown = 1;
        ptr_array_reserve(array, grown, host);
    }
    void** const slot = static_cast<void**>(array.ptr(0)) + array.at<std::int32_t>(4);
    if (slot != nullptr) *slot = value;
    ++array.at<std::int32_t>(4);
}

void ptr_array_resize(Raw array, std::int32_t count, Fe5Host& host) {
    if (array.at<std::int32_t>(8) < count) ptr_array_reserve(array, count, host);
    for (std::int32_t i = array.at<std::int32_t>(4); i < count; ++i) {
        void** const slot = static_cast<void**>(array.ptr(0)) + i;
        if (slot != nullptr) *slot = nullptr;
    }
    while (count < array.at<std::int32_t>(4)) --array.at<std::int32_t>(4);
    array.at<std::int32_t>(4) = count;
}

void ptr_array_destroy(Raw array, Fe5Host& host) {
    ptr_array_resize(array, 0, host);
    host.crt_free(array.ptr(0));
}

void* observer_handle_copy_construct_00545f50(Raw handle, Raw other, Fe5Host& host) {
    handle.at<std::uint32_t>(4) = 0;
    handle.at<std::uint32_t>(8) = 0;
    handle.at<std::uint32_t>(0x0C) = 0;
    handle.at<std::uint8_t>(0x10) = 0;
    handle.at<std::uint32_t>(0) = kWeaponGroupEntryBVtable;
    handle.ptr(0x14) = other.ptr(0x14);
    handle.at<std::uint8_t>(0x10) = 1;
    if (handle.ptr(0x14) != nullptr) host.observer_register_00694a60(handle.ptr(0x14), handle.p);
    return handle.p;
}

void* weapon_group_entry_construct_005460a0(Raw handle, void* observed, Fe5Host& host) {
    handle.at<std::uint32_t>(4) = 0;
    handle.at<std::uint32_t>(8) = 0;
    handle.at<std::uint32_t>(0x0C) = 0;
    handle.at<std::uint32_t>(0) = kWeaponGroupEntryBVtable;
    handle.ptr(0x14) = observed;
    handle.at<std::uint8_t>(0x10) = 1;
    if (observed != nullptr) host.observer_register_00694a60(observed, handle.p);
    return handle.p;
}

void* weapon_group_entry_c_construct_00546310(Raw handle, void* observed, Fe5Host& host) {
    weapon_group_entry_construct_005460a0(handle, observed, host);
    handle.at<std::uint32_t>(0) = kWeaponGroupEntryCVtable;
    return handle.p;
}

void* observer_handle_set_target(Raw handle, void* observed, Fe5Host& host) {
    if (handle.ptr(0x14) == observed) return handle.p;
    if (handle.ptr(0x14) != nullptr) host.observer_unregister_006952a0(handle.ptr(0x14), handle.p);
    handle.ptr(0x14) = observed;
    if (observed != nullptr) host.observer_register_00694a60(observed, handle.p);
    return handle.p;
}

void observer_handle_on_released(Raw handle, Raw notifier, Fe5Host& host) {
    void* const released = host.object_virtual_ret(notifier, 0x04);
    if (released != handle.ptr(0x14)) return;
    handle.ptr(0x14) = nullptr;
    host.observer_unregister_006952a0(released, handle.p);
}

void entry_array_reserve_00546370(Raw array, std::int32_t capacity, Fe5Host& host) {
    if (capacity < 1) capacity = 1;
    if (array.at<std::int32_t>(8) >= capacity) return;
    auto* const block = static_cast<std::uint8_t*>(host.operator_new(static_cast<std::size_t>(capacity) * 0x18));
    auto* const old = static_cast<std::uint8_t*>(array.ptr(0));
    for (std::int32_t i = 0; i < array.at<std::int32_t>(4); ++i) {
        if (block + i * 0x18 != nullptr) {
            observer_handle_copy_construct_00545f50(Raw{block + i * 0x18}, Raw{old + i * 0x18}, host);
            Raw{block + i * 0x18}.at<std::uint32_t>(0) = kWeaponGroupEntryCVtable;
        }
    }
    for (std::int32_t i = 0; i < array.at<std::int32_t>(4); ++i) host.object_virtual_int(Raw{old + i * 0x18}, 0x00, 0);
    host.crt_free(old);
    array.ptr(0) = block;
    array.at<std::int32_t>(8) = capacity;
}

void entry_array_append_00546730(Raw array, Raw value, Fe5Host& host) {
    if (array.at<std::int32_t>(4) == array.at<std::int32_t>(8)) {
        std::int32_t grown = array.at<std::int32_t>(8) * 2;
        if (grown < 2) grown = 1;
        entry_array_reserve_00546370(array, grown, host);
    }
    auto* const slot = static_cast<std::uint8_t*>(array.ptr(0)) + array.at<std::int32_t>(4) * 0x18;
    if (slot != nullptr) {
        observer_handle_copy_construct_00545f50(Raw{slot}, value, host);
        Raw{slot}.at<std::uint32_t>(0) = kWeaponGroupEntryCVtable;
    }
    ++array.at<std::int32_t>(4);
}

void entry_array_resize_005467b0(Raw array, std::int32_t count, Fe5Host& host) {
    if (array.at<std::int32_t>(8) < count) entry_array_reserve_00546370(array, count, host);
    for (std::int32_t i = array.at<std::int32_t>(4); i < count; ++i) {
        auto* const e = static_cast<std::uint8_t*>(array.ptr(0)) + i * 0x18;
        if (e == nullptr) continue;
        const Raw entry{e};
        entry.at<std::uint32_t>(4) = 0;
        entry.at<std::uint32_t>(8) = 0;
        entry.at<std::uint32_t>(0x0C) = 0;
        entry.at<std::uint32_t>(0x14) = 0;
        entry.at<std::uint8_t>(0x10) = 1;
        entry.at<std::uint32_t>(0) = kWeaponGroupEntryCVtable;
    }
    while (count < array.at<std::int32_t>(4)) {
        --array.at<std::int32_t>(4);
        host.object_virtual_int(Raw{static_cast<std::uint8_t*>(array.ptr(0)) + array.at<std::int32_t>(4) * 0x18}, 0x00, 0);
    }
    array.at<std::int32_t>(4) = count;
}

void screen2e_clear_timed_entries_005464e0(Raw screen, Fe5Host& host) {
    for (std::int32_t i = 0; i < screen.at<std::int32_t>(0xE8); ++i) {
        void* const object = static_cast<void**>(screen.ptr(0xE4))[i];
        if (object != nullptr) host.object_virtual_int(Raw{static_cast<std::uint8_t*>(object)}, 0x04, 1);
        void* const widget = static_cast<void**>(screen.ptr(0xF0))[i];
        if (widget != nullptr) {
            host.widget_delete_timed_entry_00aa8b80(widget, 2);
            host.object_virtual_int(Raw{static_cast<std::uint8_t*>(static_cast<void**>(screen.ptr(0xF0))[i])}, 0x04, 1);
        }
    }
    ptr_array_resize(Raw{screen.p + 0xE4}, 0, host);
    ptr_array_resize(Raw{screen.p + 0xF0}, 0, host);
}

void entry_array_destroy_005472b0(Raw array, Fe5Host& host) {
    entry_array_resize_005467b0(array, 0, host);
    host.crt_free(array.ptr(0));
}

void** refptr_array_at_grow_005472d0(Raw array, std::int32_t index, Fe5Host& host) {
    if (array.at<std::int32_t>(4) <= index) host.refptr_array_resize_005471b0(array, index + 1);
    return static_cast<void**>(array.ptr(0)) + index;
}

void* screen2e_construct_005472f0(Raw screen, Fe5Host& host) {
    host.front_end_screen_construct_004f7180(screen);
    screen.at<std::uint32_t>(0x08) = kCallbackOwnerBaseVtable;
    screen.at<std::uint32_t>(0x0C) = 0;
    screen.at<std::uint32_t>(0x10) = 0;
    screen.at<std::uint32_t>(0x14) = 0;
    screen.at<std::uint8_t>(0x18) = 0;
    screen.at<std::uint32_t>(0x00) = kScreen2EVtable;
    screen.at<std::uint32_t>(0x08) = kScreen2ESecondaryVtable;
    for (std::size_t off : {0x20u, 0x24u, 0x28u, 0x30u, 0x34u, 0x38u, 0x40u}) screen.at<std::uint32_t>(off) = 0;
    screen.at<std::uint8_t>(0x3C) = 1;
    screen.at<std::uint32_t>(0x2C) = 0x00CEDDB8;
    for (std::size_t off = 0xE4; off <= 0xF8; off += 4) screen.at<std::uint32_t>(off) = 0;
    return screen.p;
}

void screen2e_destruct_00547390(Raw screen, Fe5Host& host) {
    screen.at<std::uint32_t>(0) = kScreen2EVtable;
    screen.at<std::uint32_t>(8) = kScreen2ESecondaryVtable;
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0xDC));
    host.gui_remove_and_destroy_page_00aa31f0(screen.ptr(0x58));
    ptr_array_resize(Raw{screen.p + 0xF0}, 0, host);
    host.crt_free(screen.ptr(0xF0));
    ptr_array_resize(Raw{screen.p + 0xE4}, 0, host);
    host.crt_free(screen.ptr(0xE4));
    weapon_group_entry_destruct(Raw{screen.p + 0x2C}, kWeaponGroupEntryAVtable, host);
    entry_array_destroy_005472b0(Raw{screen.p + 0x20}, host);
    host.callback_owner_destroy_00695870(Raw{screen.p + 8});
    host.front_end_screen_destruct_004f71a0(screen);
}

void screen2e_collect_children_005482d0(Raw screen, void* out, Fe5Host& host) {
    host.push_page_slot_004d6790(out, &screen.ptr(0xDC));
    host.push_page_slot_004d6790(out, &screen.ptr(0x58));
}

void weapon_group_on_observed_event_00549200(Raw listener, const void* object, std::int32_t event, Fe5Host& host) {
    if (event != 5) return;
    const std::int32_t count = listener.at<std::int32_t>(0x1C);
    auto* const entries = static_cast<std::uint8_t*>(listener.ptr(0x18));
    std::int32_t i = 0;
    while (i < count && Raw{entries + i * 0x18}.ptr(0x14) != object) ++i;
    if (i >= count) return;
    const Raw screen{listener.p - 8};
    const std::int32_t group = listener.at<std::int32_t>(0x3C);
    if (group == 0 || !host.weapon_group_any_available_00545b30(screen, group)) {
        host.weapon_group_step_00548410(screen, 1);
    }
}

void screen2e_register_005468b0(Raw screen, Fe5Host& host) {
    host.front_end_screen_register_004f71d0(screen);
    screen.ptr(0xDC) = host.gui_load_page_00aa5840("GUI_cross_gunstate");
    screen.ptr(0x58) = host.gui_load_page_00aa5840("GUI_cross_ship");
    screen.at<std::uint8_t>(0x100) = 0;
    screen.at<std::uint8_t>(0x1C) = 0;
    host.object_virtual(screen, 0x14);
    screen.at<std::uint8_t>(0xD6) = 0;
    weapon_group_hide_slot_widgets_005452f0(screen, host);
    screen.at<std::uint8_t>(0x108) = 0;
    screen.at<std::uint8_t>(0x109) = 0;
    screen.at<std::uint8_t>(0xE0) = 1;
    screen.at<std::uint32_t>(0x104) = 0;
}
// ---------------------------------------------------------------------------
// Main-menu command-bar listener
// ---------------------------------------------------------------------------
void command_bar_route(const std::array<RouteGroup, 4>& route, std::uint16_t slot, Raw listener, void* widget,
                       Fe5Host& host) {
    if (host.widget_kind_5c(widget) == 0x12) {
        for (std::size_t i = 0; i < 5; ++i) {
            if (widget == listener.ptr(0x7C + 4 * i)) {
                host.widget_press_84(widget, 1);
                widget = listener.ptr(0x2C + 4 * i);
            }
        }
    }
    for (const RouteGroup& group : route) {
        const Raw manager = host.manager_global(group.manager);
        if (manager.p == nullptr) continue;
        for (std::uint8_t i = 0; i < group.count; ++i) {
            const Raw screen{static_cast<std::uint8_t*>(manager.ptr(group.slots[i].screen))};
            if (screen.p == nullptr || screen.at<std::uint8_t>(4) == 0) continue;
            host.listener_forward(Raw{screen.p + group.slots[i].listener}, slot, widget);
            if (group.first_slot_ends_routing && i == 0) return;
            break;
        }
    }
}

void command_owner_on_widget_hover_00549dd0(Raw listener, void* widget, std::uint32_t flag, Fe5Host& host) {
    if (host.widget_kind_5c(widget) == 0x12) {
        if (static_cast<std::uint8_t>(flag) == 0) {
            for (std::size_t i = 0; i < 5; ++i) host.widget_press_84(listener.ptr(0x7C + 4 * i), 0);
        } else {
            for (std::size_t i = 0; i < 5; ++i) {
                if (widget == listener.ptr(0x7C + 4 * i)) {
                    host.widget_press_84(widget, 2);
                    widget = listener.ptr(0x2C + 4 * i);
                }
            }
        }
    }
    for (const RouteGroup& group : kCommandRoute18) {
        const Raw manager = host.manager_global(group.manager);
        if (manager.p == nullptr) continue;
        for (std::uint8_t i = 0; i < group.count; ++i) {
            const Raw screen{static_cast<std::uint8_t*>(manager.ptr(group.slots[i].screen))};
            if (screen.p == nullptr || screen.at<std::uint8_t>(4) == 0) continue;
            host.listener_forward2(Raw{screen.p + group.slots[i].listener}, 0x18, widget, flag);
            break;
        }
    }
}
// ---------------------------------------------------------------------------
// Idle-camera screen and shots
// ---------------------------------------------------------------------------
void native_string_concat_char_0054ac10(Raw self, Raw out, std::uint8_t c, Fe5Host& host) {
    out.at<std::uint32_t>(0) = 0;
    out.at<std::uint32_t>(4) = 0;
    if (out.p != self.p) host.native_string_assign(out, host.native_string_read(self));
    host.native_string_append_byte(out, c);
}

void idle_camera_screen_destruct_0054ec90(Raw screen, Fe5Host& host) {
    screen.at<std::uint32_t>(0) = 0x00CEE408;
    screen.at<std::uint32_t>(8) = 0x00CEE3F0;
    host.callback_owner_destroy_00695870(Raw{screen.p + 8});
    host.front_end_screen_destruct_004f71a0(screen);
}

void* idle_camera_screen_construct_0054f8c0(Raw screen, Fe5Host& host) {
    host.front_end_screen_construct_004f7180(screen);
    screen.at<std::uint32_t>(0x08) = kCallbackOwnerBaseVtable;
    screen.at<std::uint32_t>(0x0C) = 0;
    screen.at<std::uint32_t>(0x10) = 0;
    screen.at<std::uint32_t>(0x14) = 0;
    screen.at<std::uint8_t>(0x18) = 0;
    screen.at<std::uint32_t>(0x1C) = 0;
    screen.at<std::uint32_t>(0x00) = 0x00CEE408;
    screen.at<std::uint32_t>(0x08) = 0x00CEE3F0;
    return screen.p;
}

void idle_camera_shot_destroy_00550390(Raw shot, Fe5Host& host) {
    host.native_string_release(Raw{shot.p + 0x10});
    host.native_string_release(Raw{shot.p + 0x08});
}

void* idle_camera_shot_assign_005506c0(Raw dst, Raw src, Fe5Host& host) {
    dst.at<std::uint32_t>(0) = src.at<std::uint32_t>(0);
    dst.at<std::uint32_t>(4) = src.at<std::uint32_t>(4);
    if (dst.p != src.p) {
        host.native_string_assign(Raw{dst.p + 8}, host.native_string_read(Raw{src.p + 8}));
        host.native_string_assign(Raw{dst.p + 0x10}, host.native_string_read(Raw{src.p + 0x10}));
    }
    dst.at<std::uint8_t>(0x18) = src.at<std::uint8_t>(0x18);
    return dst.p;
}

void* idle_camera_shot_copy_construct_005507e0(Raw dst, Raw src, Fe5Host& host) {
    dst.at<std::uint32_t>(0) = src.at<std::uint32_t>(0);
    dst.at<std::uint32_t>(4) = src.at<std::uint32_t>(4);
    dst.at<std::uint32_t>(0x08) = 0;
    dst.at<std::uint32_t>(0x0C) = 0;
    if (dst.p != src.p) host.native_string_assign(Raw{dst.p + 8}, host.native_string_read(Raw{src.p + 8}));
    dst.at<std::uint32_t>(0x10) = 0;
    dst.at<std::uint32_t>(0x14) = 0;
    if (dst.p != src.p) host.native_string_assign(Raw{dst.p + 0x10}, host.native_string_read(Raw{src.p + 0x10}));
    dst.at<std::uint8_t>(0x18) = src.at<std::uint8_t>(0x18);
    return dst.p;
}

std::uint8_t* idle_camera_shot_copy(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest, Fe5Host& host) {
    for (; first != last; first += 0x1C, dest += 0x1C) idle_camera_shot_assign_005506c0(Raw{dest}, Raw{first}, host);
    return dest;
}

std::uint8_t* idle_camera_shot_copy_backward_00550930(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
                                                      Fe5Host& host) {
    const std::ptrdiff_t count = (last - first) / 0x1C;
    std::uint8_t* d = dest_end;
    while (last != first) {
        last -= 0x1C;
        d -= 0x1C;
        idle_camera_shot_assign_005506c0(Raw{d}, Raw{last}, host);
    }
    return dest_end - count * 0x1C;
}

void idle_camera_shot_placement_copy_00550990(std::uint8_t* dest, Raw src, Fe5Host& host) {
    if (dest != nullptr) idle_camera_shot_copy_construct_005507e0(Raw{dest}, src, host);
}

void idle_camera_shot_destroy_range_00550e10(std::uint8_t* first, std::uint8_t* last, Fe5Host& host) {
    for (; first != last; first += 0x1C) idle_camera_shot_destroy_00550390(Raw{first}, host);
}

void idle_camera_shot_vector_tidy_00550ef0(Raw vector, Fe5Host& host) {
    auto* const first = static_cast<std::uint8_t*>(vector.ptr(4));
    if (first != nullptr) {
        idle_camera_shot_destroy_range_00550e10(first, static_cast<std::uint8_t*>(vector.ptr(8)), host);
        host.crt_free(first);
    }
    vector.ptr(4) = nullptr;
    vector.ptr(8) = nullptr;
    vector.ptr(0x0C) = nullptr;
}
// ---------------------------------------------------------------------------
// Control-binding screen
// ---------------------------------------------------------------------------
namespace {
bool control_binding_widget_ignored(Raw screen, void* widget, Fe5Host& host) {
    if (screen.at<std::int32_t>(0x15C) == 3) return true;
    const std::int32_t kind = host.widget_kind_5c(widget);
    return kind == 0x10 || host.widget_kind_5c(widget) == 2;
}

void control_binding_refresh(void* widget, bool on, Fe5Host& host) {
    if (host.widget_kind_5c(widget) == 6) {
        host.widget_set_state_88(widget, on, 0, 1.0f);
        return;
    }
    if (host.widget_kind_5c(widget) == 0x12) host.widget_press_84(widget, on ? 1 : 0);
}
}  // namespace

void control_binding_on_widget_hover_00551bc0(Raw screen, void* widget, bool hover, Fe5Host& host) {
    if (control_binding_widget_ignored(screen, widget, host)) return;
    if (host.widget_kind_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, hover ? 0.5f : 1.0f);  // 00CE3800 / 00D7A24C
        return;
    }
    if (widget == screen.ptr(0x200) && screen.at<std::uint8_t>(0x1DC) != 0) return;
    control_binding_refresh(widget, hover, host);
}

void control_binding_on_widget_released_005539e0(Raw screen, void* widget, Fe5Host& host) {
    if (control_binding_widget_ignored(screen, widget, host)) return;
    const bool on = Raw{static_cast<std::uint8_t*>(widget)}.at<std::uint8_t>(0xD4) != 0;
    if (host.widget_kind_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, on ? 0.5f : 1.0f);
        return;
    }
    if (widget == screen.ptr(0x1F8) || widget == screen.ptr(0x1F4)) {
        host.scroller_set_auto_scroll_00683300(Raw{screen.p + 0x194}, 0, 0.0f, false);
    } else if (widget == screen.ptr(0x200)) {
        host.scroller_end_thumb_drag(Raw{screen.p + 0x194});
    }
    control_binding_refresh(widget, on, host);
}

void control_binding_open_keyboard_00553bf0(Raw screen, Fe5Host& host) {
    std::uint32_t user = 0;
    if (host.xenon_has_selected_user_00a3e510()) user = host.xenon_selected_user_index_00a3eac0();
    screen.at<std::uint16_t>(0xE6) = 0;
    for (std::size_t off = 0x44; off <= 0x5C; off += 4) screen.at<std::uint32_t>(off) = 0;
    std::memset(screen.p + 0x60, 0, 0x80);
    host.x_show_keyboard_ui_00a4d43a(user, 0, screen.p + 0xE6, nullptr, nullptr, screen.p + 0x60, 0x30, screen.p + 0x44);
    screen.at<std::uint8_t>(0xE4) = 1;
}

void* input_device_tree_max_00552c50(Raw node) noexcept {
    void* n = node.p;
    for (void* r = Raw{static_cast<std::uint8_t*>(n)}.ptr(8); Raw{static_cast<std::uint8_t*>(r)}.at<std::uint8_t>(0x99) == 0;
         r = Raw{static_cast<std::uint8_t*>(n)}.ptr(8)) {
        n = r;
    }
    return n;
}

void* input_device_tree_min_00552c70(Raw node) noexcept {
    void* n = node.p;
    for (void* l = Raw{static_cast<std::uint8_t*>(n)}.ptr(0); Raw{static_cast<std::uint8_t*>(l)}.at<std::uint8_t>(0x99) == 0;
         l = Raw{static_cast<std::uint8_t*>(n)}.ptr(0)) {
        n = l;
    }
    return n;
}

void* tree_iterator_pre_increment_via(Raw iterator, std::uint32_t routine, Fe5Host& host) {
    host.tree_iterator_increment(iterator, routine);
    return iterator.p;
}

void native_string_deque_grow_map_0068e8a0(Raw deque, std::uint32_t count, Fe5Host& host) {
    std::uint32_t map_size = deque.at<std::uint32_t>(8);
    // 0FFFFFFFh - map size < count throws (0068E7E0); not modelled.
    std::uint32_t increment = map_size >> 1;
    if (increment < 8) increment = 8;
    if (count < increment && map_size <= 0x0FFFFFFFu - increment) count = increment;
    const std::uint32_t front_block = deque.at<std::uint32_t>(0x0C) >> 1;
    auto* const old_map = static_cast<void**>(deque.ptr(4));
    auto* const new_map = static_cast<void**>(host.operator_new(static_cast<std::size_t>(map_size + count) * 4));
    void** out = new_map + front_block;
    for (std::uint32_t i = front_block; i < map_size; ++i) *out++ = old_map[i];
    if (front_block <= count) {
        for (std::uint32_t i = 0; i < front_block; ++i) *out++ = old_map[i];
        for (std::uint32_t i = front_block; i < count; ++i) *out++ = nullptr;
        for (std::uint32_t i = 0; i < front_block; ++i) new_map[i] = nullptr;
    } else {
        for (std::uint32_t i = 0; i < count; ++i) *out++ = old_map[i];
        void** low = new_map;
        for (std::uint32_t i = count; i < front_block; ++i) *low++ = old_map[i];
        for (std::uint32_t i = 0; i < count; ++i) *low++ = nullptr;
    }
    if (old_map != nullptr) host.crt_free(old_map);
    deque.at<std::uint32_t>(8) = map_size + count;
    deque.ptr(4) = new_map;
}

void native_string_deque_push_back_0068eb90(Raw deque, Raw value, Fe5Host& host) {
    const std::uint32_t next = deque.at<std::uint32_t>(0x0C) + deque.at<std::uint32_t>(0x10);
    if ((next & 1) == 0 && deque.at<std::uint32_t>(8) <= (deque.at<std::uint32_t>(0x10) + 2) >> 1) {
        native_string_deque_grow_map_0068e8a0(deque, 1, host);
    }
    std::uint32_t block = (deque.at<std::uint32_t>(0x0C) + deque.at<std::uint32_t>(0x10)) >> 1;
    if (deque.at<std::uint32_t>(8) <= block) block -= deque.at<std::uint32_t>(8);
    void** const map = static_cast<void**>(deque.ptr(4));
    if (map[block] == nullptr) map[block] = host.operator_new(0x10);
    const std::uint32_t slot = (deque.at<std::uint32_t>(0x0C) + deque.at<std::uint32_t>(0x10)) & 1;
    host.native_string_construct_copy_0044bcb0(Raw{static_cast<std::uint8_t*>(map[block]) + slot * 8}, value);
    ++deque.at<std::uint32_t>(0x10);
}

void plane_screen_exit_006088f0(Raw screen, Fe5Host& host) {
    for (std::size_t off : {0x150u, 0x154u}) {
        if (screen.ptr(off) != nullptr) {
            host.crt_free(screen.ptr(off));
            screen.ptr(off) = nullptr;
        }
    }
    owned_vector_delete_006088a0(Raw{screen.p + 0x108}, host);
    owned_vector_delete_006088a0(Raw{screen.p + 0x10C}, host);
    host.widget_set_local_y_00aa78f0(screen.ptr(0xC8), screen.at<float>(0x114));
    const Raw effect{static_cast<std::uint8_t*>(screen.ptr(0x15C))};
    if (effect.p != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        effect.at<std::uint8_t>(9) = 1;
        refcounted_ptr_release_00605600(Raw{screen.p + 0x15C}, host);
        screen.ptr(0x15C) = nullptr;
    }
    host.widget_set_visible_34(screen.ptr(0xB8), false);
    host.widget_set_visible_34(screen.ptr(0x0C), false);
    screen.at<std::uint8_t>(8) = 0;
    for (std::size_t first : {0x98u, 0xA8u}) {
        auto** const begin = static_cast<void**>(screen.ptr(first));
        auto** const end = static_cast<void**>(screen.ptr(first + 4));
        for (void** p = begin; begin != nullptr && p != end; ++p) {
            if (*p == nullptr) continue;
            host.object_virtual_int(Raw{static_cast<std::uint8_t*>(*p)}, 0x04, 1);
            *p = nullptr;
        }
        screen.ptr(first + 4) = screen.ptr(first);
    }
}

void plane_screen_enter_006093d0(Raw screen, Fe5Host& host) {
    screen.ptr(0x108) = host.hud_widget_set_new_0043dff0(screen.at<std::uint32_t>(0xF4), screen.at<std::uint32_t>(0xF8),
                                                         screen.at<std::uint32_t>(0xFC));
    auto make_needle = [&](std::size_t slot, std::size_t widget, float maximum) {
        auto* const block = static_cast<std::uint8_t*>(host.operator_new(0x20));
        if (block != nullptr) {
            const Raw n{block};
            n.at<float>(8) = kNeedleCentreX;
            n.ptr(4) = screen.ptr(widget);
            n.at<float>(0x0C) = kNeedleCentreY;
        }
        screen.ptr(slot) = block;
        const Raw n{block};
        n.at<float>(0x1C) = 0.0f;
        n.at<float>(0x10) = kNeedleOffset;
        n.at<float>(0x18) = maximum;
        n.at<float>(0x14) = kNeedleSpan;
        n.at<float>(0x00) = kPlaneEffectsUnsetTime;  // -1.0 (00D7A260)
        gauge_needle_set_value_00605410(n, 0.0f, host);
    };
    make_needle(0x150, 0xE4, host.game_tuning_0042e740().at<float>(0x684));
    make_needle(0x154, 0xE8, kNeedleSecondMaximum);
    host.widget_set_visible_34(screen.ptr(0xB8), true);
    host.widget_set_visible_34(screen.ptr(0x0C), true);
    screen.ptr(0x10C) = host.hud_widget_set_new_0043df90(screen.at<std::uint32_t>(0x100), screen.at<std::uint32_t>(0x104));
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0x168))};
    const float gauge = Raw{static_cast<std::uint8_t*>(plane.ptr(0x538))}.at<float>(0x600);
    if (gauge == 0.0f) {
        host.widget_set_visible_34(screen.ptr(0xD4), false);
        screen.at<float>(0x138) = plane.at<float>(0xE84);
    } else {
        host.widget_set_visible_34(screen.ptr(0xD4), true);
    }
    host.widget_set_visible_34(screen.ptr(0xE0), false);
    screen.at<std::uint8_t>(8) = 1;
    plane_screen_update_payload_icon_00605380(screen, host);
    plane_screen_rebuild_widgets_00608f30(screen, host);
    screen.at<std::uint8_t>(0x190) = 1;
}

void warning_screen_show_second_text_00682990(Raw screen, Raw text, const float* rgba, std::uint32_t frame,
                                              Fe5Host& host) {
    void* const group = screen.ptr(kWarnSecondGroup);
    host.widget_set_colour_50(host.gui_find_child_00aa7e00(group, "second_Text"), rgba);
    host.text_set_localised_source_00abaed0(host.gui_find_child_00aa7e00(group, "second_Text"), text);
    host.widget_set_frame_88(host.gui_find_child_00aa7e00(group, "second_1_Icon"), static_cast<std::int32_t>(frame), 0, 1.0f);
    host.widget_set_frame_88(host.gui_find_child_00aa7e00(group, "second_2_Icon"), static_cast<std::int32_t>(frame), 0, 1.0f);
    host.widget_set_visible_34(screen.ptr(kWarnSecondGroup), true);
}

void warning_screen_set_warning2_text_00682bd0(Raw screen, Raw text, Fe5Host& host) {
    host.text_set_localised_source_00abaed0(host.gui_find_child_00aa7e00(screen.ptr(kWarnThirdGroup), "warning_2_text"),
                                            text);
    host.native_string_release(text);
}

void control_binding_position_marker_00553b00(Raw screen, void* widget, Fe5Host& host) {
    void* const marker = screen.ptr(0x210);
    if (widget == nullptr) {
        host.widget_set_visible_34(marker, false);
        return;
    }
    float pos[3];
    host.widget_resolved_position_00aa6750(widget, pos);
    const float* const size = host.widget_size_00aa6740(widget);
    const float w = size[0];
    const float h = static_cast<float>(static_cast<double>(size[1]) + kMarkerHeightPad);
    const Raw wr{static_cast<std::uint8_t*>(widget)};
    const float fx = wr.at<float>(0x18);
    const float fy = wr.at<float>(0x1C);
    const float x = static_cast<float>(static_cast<double>(pos[0]) - static_cast<double>(w) * fx);
    const float y = static_cast<float>(static_cast<double>(pos[1]) - static_cast<double>(fy) * h);
    const float placed[3] = {x + 0.0f, 0.0f + y, static_cast<float>(static_cast<double>(pos[2]) + kMarkerDepthBias)};
    host.widget_set_resolved_position_00aa8240(marker, placed);
    const float marker_size[2] = {w, h};
    host.widget_set_size_58(marker, marker_size);
    host.widget_set_visible_34(marker, true);
}

void plane_effects_register_00607be0(Raw screen, Fe5Host& host) {
    host.front_end_screen_register_004f71d0(screen);
    for (std::size_t off : {0x94u, 0x98u}) {
        if (screen.ptr(off) == nullptr) continue;
        host.object_virtual_int(Raw{static_cast<std::uint8_t*>(screen.ptr(off))}, 0x00, 1);
        screen.ptr(off) = nullptr;
    }
    screen.at<float>(0xCC) = 0.0f;
    const Raw observer{screen.p + 0xD0};
    if (observer.ptr(0x14) != nullptr) {
        host.observer_unregister_006952a0(observer.ptr(0x14), observer.p);
        observer.ptr(0x14) = nullptr;
    }
    screen.ptr(0xB8) = nullptr;
    screen.at<std::uint32_t>(0xFC) = 0;
    auto swap_in = [&](std::size_t off, void* acquired) {
        void* const old = screen.ptr(off);
        if (old != acquired) {
            screen.ptr(off) = acquired;
            if (acquired != nullptr) host.sound_handle_add_ref(acquired);
            if (old != nullptr) host.refcounted_release(old);
        }
        if (acquired != nullptr) host.refcounted_release(acquired);
    };
    for (const auto& [off, name] : kPlaneEffectHandles) swap_in(off, host.effect_handle_acquire_00871ba0(name));
    swap_in(0x78, host.effect_handle_acquire_id_00870cd0(kPlaneEffectHandleId78));
}

// ---------------------------------------------------------------------------
// cc13-fe7: plane HUD widget rebuild, unit library update, idle camera shot
// vector insert (docs/CC13_FE_7.md).
// ---------------------------------------------------------------------------
namespace {
// A std::vector<widget*> {proxy, first, last, end} at screen+base: delete
// each widget (virtual +4h(1)) and null its cell, then erase(begin, end)
// (the image's memmove_s of the empty tail moves nothing; last = first).
void widget_vector_delete_all(Raw screen, std::size_t base, Fe5Host& host) {
    for (std::uint32_t i = 0;; ++i) {
        auto* const first = static_cast<void**>(screen.ptr(base + 4));
        auto* const last = static_cast<void**>(screen.ptr(base + 8));
        if (first == nullptr || i >= static_cast<std::uint32_t>(last - first)) break;
        if (first[i] != nullptr) {
            host.object_virtual_int(Raw{static_cast<std::uint8_t*>(first[i])}, 0x04, 1);
            first[i] = nullptr;
        }
    }
    screen.ptr(base + 8) = screen.ptr(base + 4);
}

// push_back: in place while last < end, else the out-of-line insert.
void widget_vector_push(Raw screen, std::size_t base, std::uint32_t insert_routine, void* widget, Fe5Host& host) {
    auto* const first = static_cast<void**>(screen.ptr(base + 4));
    auto* const last = static_cast<void**>(screen.ptr(base + 8));
    auto* const end = static_cast<void**>(screen.ptr(base + 0x0C));
    if (first != nullptr && last - first < end - first) {
        *last = widget;
        screen.ptr(base + 8) = last + 1;
    } else {
        host.widget_vector_insert_one(Raw{screen.p + base}, insert_routine, last, widget);
    }
}
}  // namespace

int plane_hud_payload_layout(std::int32_t plane_type) noexcept {
    switch (plane_type) {  // jump table 00609344
    case 0x09: return 0;
    case 0x0A: return 5;
    case 0x0B: return 1;
    case 0x0D: return 2;
    case 0x0F: return 3;
    case 0x12: return 4;
    default: return -1;
    }
}

void plane_screen_rebuild_widgets_00608f30(Raw screen, Fe5Host& host) {
    widget_vector_delete_all(screen, 0x94, host);
    widget_vector_delete_all(screen, 0xA4, host);
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0x168))};
    if (plane.p == nullptr) return;
    const int layout = plane_hud_payload_layout(plane.at<std::int32_t>(0x970));
    std::int32_t count = 0;
    if (layout == 4) {
        for (Raw child{static_cast<std::uint8_t*>(plane.ptr(0x48))}; child.p != nullptr;
             child = Raw{static_cast<std::uint8_t*>(child.ptr(0x44))}) {
            if (host.unit_is_kind_5c(child, 0x26)) {
                count = child.at<std::int32_t>(0x488);  // 006E3520
                break;
            }
            if (host.unit_is_kind_5c(child, 0x25)) {
                count = 1;
                break;
            }
        }
    } else if (layout >= 0) {
        for (Raw child{static_cast<std::uint8_t*>(plane.ptr(0x48))}; child.p != nullptr;
             child = Raw{static_cast<std::uint8_t*>(child.ptr(0x44))}) {
            if (host.unit_is_kind_5c(child, 0x25)) ++count;
        }
    }
    const bool odd = (count & 1) != 0;
    std::int32_t even_step = -1;
    for (std::int32_t i = 0; i < count; ++i, --even_step) {
        // x87: FLD qword step, FIMUL int, FADD qword centre, FSTP float (the
        // image keeps the product and sum in extended precision).
        float pos[3] = {kPlaneSlotDefaultX, kPlaneSlotY, kPlaneSlotZ};
        if (odd) {
            if (i != 0) {
                const std::int32_t half = i / 2;
                const std::int32_t k = (i & 1) != 0 ? half : -half;
                pos[0] = static_cast<float>(kPlaneSlotOddStep * k + kPlaneSlotOddCentre);
            }
        } else {
            const std::int32_t k = (i & 1) != 0 ? i + 1 : even_step;
            pos[0] = static_cast<float>(kPlaneSlotEvenStep * k + kPlaneSlotEvenCentre);
        }
        void* const front = host.gui_widget_clone_subtree_00aab4c0(screen.ptr(0x8C), 0);
        host.widget_set_local_position_00aa7dc0(front, pos);
        widget_vector_push(screen, 0x94, 0x0043DB60, front, host);
        void* const back = host.gui_widget_clone_subtree_00aab4c0(screen.ptr(0x90), 0);
        const float back_pos[3] = {pos[0] + 0.0f, pos[1] + 0.0f,
                                   static_cast<float>(pos[2] - kPlaneSlotBackDepth)};
        host.widget_set_local_position_00aa7dc0(back, back_pos);
        widget_vector_push(screen, 0xA4, 0x0052EB90, back, host);
    }
}

void unit_library_update_00681ba0(Raw screen, float /*dt*/, Fe5Host& host) {
    const std::int32_t mode = screen.at<std::int32_t>(0xE8);
    if (mode == 0) {
        const bool cursor_row = host.listbox_selected_index_00a9c920(screen.ptr(0x70)) == 0;
        const bool digit_row = host.listbox_selected_index_00a9c920(screen.ptr(0x70)) == 1;
        if (host.front_end_action_edge_or_repeat_004d92b0(0x4B)) {
            host.front_end_manager_push_request_004cc460(host.manager_global(0x00E198AC), 1);
            return;
        }
        if (host.front_end_action_edge_or_repeat_004d92b0(0x4C) || host.front_end_action_edge_or_repeat_004d92b0(0x4D)) {
            if (cursor_row) {
                screen.at<std::int32_t>(0xEC) += host.input_action_held_004c5090(0x4C) ? -1 : 1;
            } else if (digit_row) {
                const int delta = host.input_action_held_004c5090(0x4C) ? -1 : 1;
                band_0067_cc13::digit_spinner_step_0067df40(
                    screen.at<std::int32_t>(0xF0 + 4 * static_cast<std::size_t>(screen.at<std::int32_t>(0xEC))), delta);
            }
        }
    } else if (mode == 1) {
        if (host.front_end_action_edge_or_repeat_004d92b0(0x4B)) {
            host.unit_library_reset_selection_0067e580(screen);
            return;
        }
        host.front_end_action_edge_or_repeat_004d92b0(0x4A);  // result unused (00681CD3)
    }
    if (host.front_end_action_edge_or_repeat_004d92b0(0x4C)) {
        screen.at<std::int32_t>(0xE4) = -1;
    } else if (host.front_end_action_edge_or_repeat_004d92b0(0x4D)) {
        screen.at<std::int32_t>(0xE4) = 1;
    }
    if (screen.at<std::int32_t>(0xE4) == 0) return;
    void* const globals = host.lua_globals_00b67980();
    void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");  // 00CE5880
    host.lua_release_00b67700(globals);
    void* const entry = host.lua_nil_00b65f50();
    bool again = true;
    while (again) {
        const std::int32_t index = (screen.at<std::int32_t>(0x130) + 1000 + screen.at<std::int32_t>(0xE4)) % 1000;
        screen.at<std::int32_t>(0x130) = index;
        void* const row = host.lua_get_by_index_00b67720(classes, index);
        host.lua_assign_00b67690(entry, row);
        host.lua_release_00b67700(row);
        again = true;
        if (host.lua_is_table_00b661b0(entry)) {
            void* const mesh = host.lua_get_by_name_00b67800(entry, "Mesh");  // 00CE5FD0
            again = !host.lua_is_string_00b660a0(mesh);
            host.lua_release_00b67700(mesh);
        }
    }
    unit_library_load_unit_model_00680db0(screen, host);
    screen.at<std::int32_t>(0xE4) = 0;
    host.lua_release_00b67700(entry);
    host.lua_release_00b67700(classes);
}

std::uint8_t* idle_camera_shot_uninit_fill_00550dd0(std::uint8_t* dest, std::size_t count, Raw value, Fe5Host& host) {
    std::uint8_t* p = dest;
    try {
        for (; count != 0; --count, p += 0x1C) idle_camera_shot_placement_copy_00550990(p, value, host);
    } catch (...) {
        idle_camera_shot_destroy_range_00550e10(dest, p, host);  // Catch_All@00550C4B
        throw;
    }
    return p;
}

namespace {
// 00550AC0 / 00550F40: uninitialized copy of [first, last) to dest.
std::uint8_t* idle_camera_shot_uninit_copy(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest, Fe5Host& host) {
    std::uint8_t* p = dest;
    try {
        for (; first != last; first += 0x1C, p += 0x1C) idle_camera_shot_placement_copy_00550990(p, Raw{first}, host);
    } catch (...) {
        idle_camera_shot_destroy_range_00550e10(dest, p, host);
        throw;
    }
    return p;
}

// 00550A10: assign value to every shot of [first, last).
void idle_camera_shot_fill(std::uint8_t* first, std::uint8_t* last, Raw value, Fe5Host& host) {
    for (; first != last; first += 0x1C) idle_camera_shot_assign_005506c0(Raw{first}, value, host);
}

std::size_t shot_span(const void* first, const void* last) {
    return static_cast<std::size_t>(static_cast<const std::uint8_t*>(last) - static_cast<const std::uint8_t*>(first)) / 0x1C;
}
}  // namespace

void idle_camera_shot_vector_insert_n_00550fc0(Raw vector, std::uint8_t* where, std::size_t count, Raw value,
                                               Fe5Host& host) {
    constexpr std::size_t kMaxShots = 0x09249249;  // max_size() for 1Ch-byte elements
    std::uint8_t tmp_storage[0x1C];
    const Raw tmp{tmp_storage};
    idle_camera_shot_copy_construct_005507e0(tmp, value, host);
    auto first = [&] { return static_cast<std::uint8_t*>(vector.ptr(4)); };
    auto last = [&] { return static_cast<std::uint8_t*>(vector.ptr(8)); };
    auto size = [&] { return first() == nullptr ? std::size_t{0} : shot_span(first(), last()); };
    std::size_t capacity = first() == nullptr ? 0 : shot_span(first(), vector.ptr(0x0C));
    if (count != 0) {
        if (kMaxShots - size() < count) host.stl_xlen_00550b50();
        if (capacity < size() + count) {
            capacity = kMaxShots - capacity / 2 < capacity ? 0 : capacity + capacity / 2;
            if (capacity < size() + count) capacity = size() + count;  // 0054ED10 size()
            std::uint8_t* const fresh = host.idle_camera_shot_allocate_0054eef0(capacity);
            std::uint8_t* p = fresh;
            try {
                p = idle_camera_shot_uninit_copy(first(), where, fresh, host);
                p = idle_camera_shot_uninit_fill_00550dd0(p, count, tmp, host);
                idle_camera_shot_uninit_copy(where, last(), p, host);
            } catch (...) {
                idle_camera_shot_destroy_range_00550e10(fresh, p, host);  // Catch_All@00551186
                host.crt_free(fresh);
                throw;
            }
            const std::size_t new_size = count + size();
            if (first() != nullptr) {
                idle_camera_shot_destroy_range_00550e10(first(), last(), host);
                host.crt_free(first());
            }
            vector.ptr(0x0C) = fresh + capacity * 0x1C;
            vector.ptr(8) = fresh + new_size * 0x1C;
            vector.ptr(4) = fresh;
        } else if (shot_span(where, last()) < count) {
            std::uint8_t* const old_last = last();
            idle_camera_shot_uninit_copy(where, old_last, where + count * 0x1C, host);
            try {
                idle_camera_shot_uninit_fill_00550dd0(old_last, count - shot_span(where, old_last), tmp, host);
            } catch (...) {
                idle_camera_shot_destroy_range_00550e10(where + count * 0x1C, last() + count * 0x1C, host);  // 00551240
                throw;
            }
            vector.ptr(8) = old_last + count * 0x1C;
            idle_camera_shot_fill(where, last() - count * 0x1C, tmp, host);
        } else {
            std::uint8_t* const old_last = last();
            vector.ptr(8) = idle_camera_shot_uninit_copy(old_last - count * 0x1C, old_last, old_last, host);
            idle_camera_shot_copy_backward_00550930(where, old_last - count * 0x1C, old_last, host);  // 00550BC0
            idle_camera_shot_fill(where, where + count * 0x1C, tmp, host);
        }
    }
    idle_camera_shot_destroy_00550390(tmp, host);
}

void plane_screen_ensure_cockpit_camera_00607f60(Raw screen, Fe5Host& host) {
    if (screen.ptr(0x98) == nullptr) {
        const Raw built = host.plane_cockpit_camera_new_006065d0();
        screen.ptr(0x98) = built.p;
        const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};  // 00D7A24C = 1.0
        host.camera_add_to_scene_00923870(built, host.game_00e188a8().ptr(0x19CC), identity);
        host.observer_register_00694a60(screen.ptr(0x98), screen.p + 8);
    }
    if (screen.ptr(0x94) != nullptr) {
        float scratch[3];
        const float* const pos = host.camera_position_0042cd00(Raw{static_cast<std::uint8_t*>(screen.ptr(0x94))}, scratch);
        screen.at<float>(0xF0) = pos[0];
        screen.at<float>(0xF4) = pos[1];
        screen.at<float>(0xF8) = pos[2];
    }
    host.game_set_camera_mover_004bc410(screen.ptr(0x98));
    screen.ptr(0x94) = nullptr;
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0xB8))};
    const Raw model{static_cast<std::uint8_t*>(plane.ptr(0x538))};
    float* const eye_out = &screen.at<float>(0xF0);
    if (host.object_virtual_ret(plane, 0x1C0) != nullptr) {
        host.object_virtual(plane, 0x1C4);
        host.game_set_camera_near_plane_004b4940(kCockpitNearPlane);
        const Raw camera{static_cast<std::uint8_t*>(screen.ptr(0x98))};
        host.cockpit_camera_set_position_0042e030(
            camera, host.unit_local_to_world_18(plane, &model.at<float>(0x244), eye_out));
        host.cockpit_camera_reset_0042aa00(camera);
        const Raw link{camera.p + 0x500};
        void* const observed = link.ptr(0x14);
        if (observed != plane.p) {
            if (observed != nullptr) host.observer_unregister_006952a0(observed, link.p);
            link.ptr(0x14) = plane.p;
            if (plane.p != nullptr) host.observer_register_00694a60(plane.p, link.p);
        }
    } else {
        const float eye[3] = {0.0f, kCockpitFallbackEyeY,
                              static_cast<float>(model.at<float>(0xA0) * kCockpitFallbackEyeScale)};
        const Raw camera{static_cast<std::uint8_t*>(screen.ptr(0x98))};
        host.cockpit_camera_set_position_0042e030(camera, host.unit_local_to_world_18(plane, eye, eye_out));
        host.cockpit_camera_reset_0042aa00(camera);
        const Raw link{camera.p + 0x500};
        if (link.ptr(0x14) != nullptr) {
            host.observer_unregister_006952a0(link.ptr(0x14), link.p);
            link.ptr(0x14) = nullptr;
        }
    }
}

void plane_screen_update_drop_indicator_00609620(Raw screen, Fe5Host& host) {
    if (screen.ptr(0x168) == nullptr) return;
    // Locals of the image: two std::vector<bool> (0049F2D0 push_back) and a
    // std::vector<float> (0045AFD0), destroyed at the end.
    std::vector<bool> ready;
    std::vector<bool> gate;
    std::vector<float> fill;
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0x168))};
    auto fraction = [](float remaining, float full) {  // FDIV, FLD1, FSUBRP, FSTP float
        return static_cast<float>(1.0 - static_cast<double>(remaining) / full);
    };
    const std::int32_t layout = host.plane_screen_payload_frame_00604c80(screen);
    auto next = [](Raw child) { return Raw{static_cast<std::uint8_t*>(child.ptr(0x44))}; };
    if (layout == 4) {
        for (Raw child{static_cast<std::uint8_t*>(plane.ptr(0x48))}; child.p != nullptr; child = next(child)) {
            if (host.unit_is_kind_5c(child, 0x26)) {
                for (std::int32_t j = 0; j < child.at<std::int32_t>(0x488); ++j) {  // 006E3520 each pass
                    ready.push_back(child.at<std::uint8_t>(0x654 + j) != 0 || child.at<std::uint32_t>(0x514 + 4 * j) != 0);
                    gate.push_back(true);
                    fill.push_back(fraction(child.at<float>(0x5B4 + 4 * j), child.at<float>(0x48C)));
                }
                break;
            }
            if (host.unit_is_kind_5c(child, 0x25)) {
                ready.push_back(host.unit_virtual_210(child, 0x2A, 0) != 0);
                gate.push_back(host.plane_drop_attitude_gate_007cc8e0(plane, 0) != 0);
                fill.push_back(fraction(child.at<float>(0x490), child.at<float>(0x48C)));
                break;
            }
        }
    } else if (layout >= 0 && layout <= 5) {
        for (Raw child{static_cast<std::uint8_t*>(plane.ptr(0x48))}; child.p != nullptr; child = next(child)) {
            if (!host.unit_is_kind_5c(child, 0x25)) continue;
            ready.push_back(host.unit_virtual_210(child, 0x2A, 0) != 0);
            const Raw owner{static_cast<std::uint8_t*>(screen.ptr(0x168))};
            gate.push_back((host.unit_is_kind_5c(owner, 0x10) ? host.plane_level_bomb_attitude_gate_007c7750(owner, 0)
                                                              : host.plane_drop_attitude_gate_007cc8e0(owner, 0)) != 0);
            fill.push_back(fraction(child.at<float>(0x490), child.at<float>(0x48C)));
        }
    }
    auto* const front = static_cast<void**>(screen.ptr(0x98));
    if (front == nullptr) return;
    const std::int32_t count = static_cast<std::int32_t>(static_cast<void**>(screen.ptr(0x9C)) - front);
    for (std::int32_t i = 0; i < count; ++i) {
        const bool shown = screen.at<std::uint8_t>(0x190) != 0;  // re-read per call (MOVZX)
        // The image indexes through checked accessors (00BF6713 on a miss).
        void* const widget = static_cast<void**>(screen.ptr(0x98))[i];
        host.widget_set_visible_34(widget, shown);
        void* const back = static_cast<void**>(screen.ptr(0xA8))[i];
        if (ready.at(static_cast<std::size_t>(i))) {
            host.widget_set_frame_88(widget, gate.at(static_cast<std::size_t>(i)) ? 0 : 1, 0, 1.0f);
            host.widget_set_visible_34(back, false);
        } else {
            host.widget_set_frame_88(widget, 2, 0, 1.0f);
            host.widget_set_visible_34(back, shown);
            host.widget_set_quad_floats_00abe6e0(back, fill.at(static_cast<std::size_t>(i)), 0.0f, 0.0f, 1.0f);
        }
    }
}

void plane_screen_altitude_blink_00605b20(Raw screen, float altitude, Fe5Host& host) {
    const double step = kBlinkSlowBelow > altitude ? kBlinkStepLow
                        : altitude > kBlinkFastAbove ? kBlinkStepHigh
                                                     : kBlinkStepMid;
    const double phase = screen.at<float>(0x134);
    screen.at<float>(0x134) = static_cast<float>(screen.at<std::uint8_t>(0x130) != 0 ? phase - step : phase + step);
    const float now = screen.at<float>(0x134);
    if (0.0f >= now) {  // COMISS 0, phase; JB skips (a NaN phase changes nothing)
        screen.at<std::uint8_t>(0x130) = 0;
    } else if (now >= kBlinkPhaseTop) {
        screen.at<std::uint8_t>(0x130) = 1;
    }
    const Raw plane{static_cast<std::uint8_t*>(screen.ptr(0x168))};
    if (plane.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_00414db0(plane);
    if (kLowAltitude > plane.at<float>(0x100)) {
        host.widget_set_visible_34(screen.ptr(0xEC), screen.at<std::uint8_t>(0x130) != 0);
    }
}

void plane_screen_update_instruments_00609bd0(Raw screen, float dt, bool ordnance_view, bool aim_view,
                                              Fe5Host& host) {
    plane_screen_hide_indicator_grid_00604c20(screen, host);
    if (host.game_00e188a8().at<std::uint8_t>(0x61F) != 0) return;
    auto plane = [&] { return Raw{static_cast<std::uint8_t*>(screen.ptr(0x168))}; };
    auto refresh = [&](Raw unit) {
        if (unit.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_00414db0(unit);
    };
    auto show = [&](std::size_t off) { host.widget_set_visible_34(screen.ptr(off), screen.at<std::uint8_t>(0x190) != 0); };
    auto unit_at = [](Raw block, std::size_t off) { return Raw{static_cast<std::uint8_t*>(block.ptr(off))}; };

    // Quad fade +16Ch: target 1 (0064A530) while the effects screen's unit
    // ([*00E198C4+6Ch]+B8h) is neither kind 10h nor 16h and its +FCh is 1,
    // else 0 (0064A520).
    const Raw effects = unit_at(host.ingame_interface_00e198c4(), 0x6C);
    const Raw viewed = unit_at(effects, 0xB8);
    const Raw fade{screen.p + 0x16C};
    const bool fade_in = viewed.p != nullptr && !host.unit_is_kind_5c(viewed, 0x10) &&
                         !host.unit_is_kind_5c(viewed, 0x16) && effects.at<std::int32_t>(0xFC) == 1;
    fade.at<std::int32_t>(0x20) = fade_in ? 1 : 0;
    host.quad_fade_update_0064abd0(fade, dt);

    Raw p = plane();
    refresh(p);
    const float altitude = p.at<float>(0x100);
    gauge_needle_set_value_00605410(unit_at(screen, 0x150), altitude, host);
    gauge_needle_set_value_00605410(unit_at(screen, 0x154), altitude, host);
    host.widget_set_visible_34(screen.ptr(0xE8), kLowAltitude > altitude);
    host.widget_set_visible_34(screen.ptr(0xEC), kLowAltitude > altitude);
    host.widget_set_visible_34(screen.ptr(0xD4), unit_at(plane(), 0x538).at<float>(0x600) != 0.0f);

    float speed = static_cast<float>(host.unit_virtual_float(plane(), 0x204) * kSpeedToKnots);
    speed = static_cast<float>(static_cast<double>(host.game_tuning_0042e740().at<float>(0x6A0)) * speed);
    if (host.byte_00f88988() != 0) speed = static_cast<float>(speed * kKnotsToMph);
    host.digit_gauge_update_0043b370(screen.ptr(0x108), kSpeedShownBelow > speed ? speed : kSpeedShownCap, dt);
    if (plane().at<std::int32_t>(0x970) != 0) {
        const std::int32_t rounds = host.unit_count_remaining_ordnance_rounds_007c1db0(plane());
        host.digit_gauge_update_0043b370(screen.ptr(0x10C), static_cast<float>(rounds), dt);
    }
    plane_screen_update_drop_indicator_00609620(screen, host);

    // Ladder: y = +114h - float(v * v) * 0.104 (FMULP, FSTP, FMUL qword, FSUBP).
    const float v = plane().at<float>(0x9F0);
    const float v_squared = static_cast<float>(static_cast<double>(v) * v);
    host.widget_set_local_y_00aa78f0(screen.ptr(0xC8),
                                     static_cast<float>(screen.at<float>(0x114) - v_squared * kLadderScale));

    const Raw span_block = host.static_needle_span_block_00e1968c();
    if ((span_block.at<std::uint32_t>(4) & 1) == 0) {
        span_block.at<std::uint32_t>(4) |= 1;
        span_block.at<float>(0) = kThrottleNeedleSpan;
    }
    const float raw_throttle = plane().at<float>(0xE84);
    float throttle;
    if (0.0f > raw_throttle) {
        throttle = 0.0f;
    } else if (raw_throttle > 1.0f) {  // 00D7A24C
        throttle = 1.0f;
    } else {
        throttle = raw_throttle;
    }
    const double span = span_block.at<float>(0);
    host.widget_set_rotation_44(screen.ptr(0xDC), static_cast<float>(kThrottleNeedleHalf * span - throttle * span));

    // Horizon: RotY(-pi) * RotX(+C64h) * RotZ(+C68h - pi) into ([+C4h]+ECh)+38h.
    const float angle_c68 = static_cast<float>(plane().at<float>(0xC68) - kHorizonRollBias);
    const Raw horizon = unit_at(unit_at(screen, 0xC4), 0xEC);
    float rot_z[16], rot_x[16], rot_y[16], yx[16], yxz[16];
    host.matrix_rotation_z_00b64780(rot_z, angle_c68);
    host.matrix_rotation_x_00b64640(rot_x, plane().at<float>(0xC64));
    host.matrix_rotation_y_00b646e0(rot_y, kHorizonYaw);
    host.matrix_multiply_00413920(rot_y, rot_x, yx);
    host.matrix_multiply_00413920(yx, rot_z, yxz);
    host.node_set_matrix_38(horizon, yxz);

    // Aim point: position (+FCh, +100h, +104h) + 1000 * forward (+ECh, +F0h, +F4h).
    p = plane();
    refresh(p);
    const float ahead[3] = {static_cast<float>(p.at<float>(0xEC) * kAimDistance),
                            static_cast<float>(p.at<float>(0xF0) * kAimDistance),
                            static_cast<float>(kAimDistance * p.at<float>(0xF4))};
    p = plane();
    refresh(p);
    const float aim[3] = {static_cast<float>(ahead[0] + p.at<float>(0xFC)),
                          static_cast<float>(p.at<float>(0x100) + ahead[1]),
                          static_cast<float>(p.at<float>(0x104) + ahead[2])};
    const Raw target = unit_at(unit_at(host.ingame_interface_00e198c4(), 0xCC), 0x4C);
    auto relation = [&](Raw unit) {
        return host.recon_relation_if_identified_00803ce0(unit_at(plane(), 0x54), unit);
    };
    auto hostile_unmarked = [&](Raw unit) {  // relation 0, not kind 36h, not the plane itself
        return unit.p != nullptr && relation(unit) == 0 && !host.unit_is_kind_5c(unit, 0x36) && unit.p != plane().p;
    };
    bool handled = false;
    bool rows = false;  // reach 0060A31B
    std::size_t row = 0;
    bool ordnance = false;
    if (plane().p == nullptr) {
        if (ordnance_view) ordnance = true;
        else rows = aim_view;  // 0060A317
    } else if (ordnance_view) {
        ordnance = true;
    } else if (aim_view || host.unit_is_kind_5c(plane(), 0x17)) {
        float on_screen[3];
        if (host.camera_project_world_to_screen_0043a660(aim, on_screen, 1, 1) == 7) {
            if (!aim_view && host.unit_is_kind_5c(plane(), 0x17)) {
                void* const sight = screen.ptr(0x38);
                if (sight != nullptr) {
                    host.widget_set_frame_88(sight, 1, 0, 1.0f);
                    float resolved[3];
                    host.widget_resolved_position_00aa6750(sight, resolved);
                    const float pos[3] = {on_screen[0], on_screen[1], resolved[2]};
                    host.widget_set_resolved_position_00aa8240(sight, pos);
                    show(0x38);
                }
            } else {
                if (target.p != nullptr && target.at<std::uint8_t>(0x5D) == 0) {
                    const float* const from = host.unit_world_position_00427eb0(plane());
                    const float* const to = host.unit_world_position_00427eb0(target);
                    float delta[3];
                    host.vector_subtract_00427c90(to, delta, from);
                    const double d2 = host.vector_length_squared_00427e30(delta);  // compared unrounded
                    if (plane().at<float>(0x49C) > d2) {
                        if (!host.object_virtual_al(target, 0x10C)) {
                            if (relation(target) == 1) {
                                show(0x20);
                                handled = true;
                            } else if (!host.unit_is_kind_5c(target, 0x36)) {
                                show(0x14);
                            }
                        } else if (relation(target) != 1 && !host.unit_is_kind_5c(target, 0x36) &&
                                   target.p != plane().p) {
                            show(0x14);
                        }
                    }
                }
                if (aim_view && !handled) {
                    const Raw locked = unit_at(unit_at(host.ingame_interface_00e198c4(), 0xCC), 0x4C);
                    if (locked.p != nullptr && host.object_virtual_al(locked, 0x10C)) {
                        const float* const from = host.unit_world_position_00427eb0(locked);
                        const float* const to = host.unit_world_position_00427eb0(plane());
                        float delta[3];
                        host.vector_subtract_00427c90(to, delta, from);
                        const float d2 = static_cast<float>(host.vector_length_squared_00427e30(delta));  // FSTP
                        if (plane().at<float>(0x49C) > d2) {
                            show(0x20);
                            handled = true;
                        }
                    }
                }
                float pos[3];
                host.widget_resolved_position_00aa6750(screen.ptr(0x10), pos);  // unconditional
                pos[0] = on_screen[0];
                pos[1] = on_screen[1];
                for (std::size_t off : {0x10u, 0x20u, 0x18u, 0x1Cu, 0x14u}) {
                    if (screen.ptr(off) != nullptr) host.widget_set_resolved_position_00aa8240(screen.ptr(off), pos);
                }
                rows = aim_view;
            }
        }
    }
    if (ordnance) {
        // ECX = the plane for every query (null on the plane-less path, as in
        // the image).
        float impact[3] = {};
        bool project = false;
        auto take = [&](std::uint32_t routine) {
            float scratch[3];
            const float* const point = host.plane_ordnance_point(plane(), routine, scratch);
            impact[0] = point[0];
            impact[1] = point[1];
            impact[2] = point[2];
        };
        if (host.plane_weapon_query(plane(), 0x007B9320)) {
            row = 2;
            take(0x007C21D0);
            host.widget_set_frame_88(screen.ptr(0x38), plane().at<std::int32_t>(0xC4) != 0x10 ? 1 : 0, 0, 1.0f);
            if (host.plane_can_release_ordnance_007bb110(plane())) project = true;
            else handled = true;
        } else if (host.plane_weapon_query(plane(), 0x007B9400)) {
            row = 1;
            take(0x007C2450);
            project = true;
        } else if (host.plane_weapon_query(plane(), 0x007B9500)) {
            row = 4;
            take(0x007BCCF0);
            project = true;
        } else if (host.plane_weapon_query(plane(), 0x007B94F0)) {
            row = 5;
            take(0x007C21D0);
            if (hostile_unmarked(target)) show(0x78);
            project = true;
        } else if (host.plane_weapon_query(plane(), 0x007B93F0)) {
            row = 3;
            show(0x5C);
            float marker_y = kTorpedoMarkerY;
            if (altitude > kTorpedoMarkerAbove) {
                const float rise = static_cast<float>((altitude - kTorpedoMarkerBase) * kTorpedoMarkerSlope);
                const float capped = rise > kTorpedoMarkerLimit ? kTorpedoMarkerLimitF : rise;
                marker_y = static_cast<float>(kReticleOffset - (capped + kTorpedoMarkerPad));
            }
            host.widget_set_local_y_00aa78f0(screen.ptr(0x5C), marker_y);
            if (hostile_unmarked(target)) show(0x50);
        } else {
            handled = true;
        }
        if (project) {
            float on_screen[3];
            if (host.camera_project_world_to_screen_0043a660(impact, on_screen, 1, 1) != 7) {
                handled = true;
            } else {
                const std::size_t base = 0x10 + row * 0x14;
                auto place = [&](void* widget) {
                    host.widget_set_local_xy_00aa7d00(widget, static_cast<float>(kReticleOffset + on_screen[0]),
                                                      static_cast<float>(on_screen[1] + kReticleOffset));
                };
                place(screen.ptr(base));  // no null test (0060A4D6)
                for (std::size_t off : {0x08u, 0x0Cu, 0x04u, 0x10u}) {
                    if (screen.ptr(base + off) != nullptr) place(screen.ptr(base + off));
                }
                if (row == 1) {
                    const Raw locked = unit_at(unit_at(host.ingame_interface_00e198c4(), 0xCC), 0x4C);
                    if (locked.p != nullptr && locked.at<std::uint8_t>(0x5D) == 0 && plane().p != nullptr) {
                        if (relation(locked) == 1) {
                            show(0x34);
                            handled = true;
                        } else {
                            show(0x28);
                        }
                    }
                }
            }
        }
        rows = true;
    }
    if (rows) {
        if (!handled) show(0x10 + row * 0x14);
        const std::int32_t mode = host.manager_global(0x00F8A0C4).at<std::int32_t>(0x178);
        if (mode == 1 && screen.ptr(0x18 + row * 0x14) != nullptr) show(0x18 + row * 0x14);
        else if (mode == 2 && screen.ptr(0x1C + row * 0x14) != nullptr) show(0x1C + row * 0x14);
    }

    // 0060A37F: the altitude blinker while the plane's +72Ch sub-object says so.
    if (host.object_virtual_al(Raw{plane().p + 0x72C}, 0x38)) {
        p = plane();
        refresh(p);
        plane_screen_altitude_blink_00605b20(screen, p.at<float>(0x100), host);
    }
    const float previous = screen.at<float>(0x138);
    const float current = plane().at<float>(0xE84);
    if (previous > current) {
        if (!host.widget_is_visible_38(screen.ptr(0xE0))) host.widget_set_visible_34(screen.ptr(0xE0), true);
        host.plane_screen_pulse_alpha_00604cf0(screen);
    } else if (current > previous && host.widget_is_visible_38(screen.ptr(0xE0))) {
        host.widget_set_visible_34(screen.ptr(0xE0), false);
        host.widget_set_alpha_4c(screen.ptr(0xE0), 0.0f);
        screen.at<std::uint8_t>(0x140) = 0;
    }
    screen.at<float>(0x138) = plane().at<float>(0xE84);
    host.widget_set_visible_34(screen.ptr(0xBC), false);

    // Shake: stick vector (tuning +194h / +198h / +19Ch), length capped at
    // tuning +1A0h, timer +164h toggling byte +160h.
    p = plane();
    const float t194 = host.game_tuning_0042e740().at<float>(0x194);
    const float t198 = host.game_tuning_0042e740().at<float>(0x198);
    const float t19c = host.game_tuning_0042e740().at<float>(0x19C);
    float stick[3] = {
        static_cast<float>(static_cast<double>(t194) * p.at<float>(0xBE8) + static_cast<double>(p.at<float>(0xBF0)) * t198),
        static_cast<float>(static_cast<double>(t194) * p.at<float>(0xBEC) + static_cast<double>(p.at<float>(0xBF0)) * t198),
        host.float_00f8757c()};
    const Raw model = unit_at(plane(), 0x538);
    const float over = static_cast<float>(host.unit_virtual_float(plane(), 0x38) / model.at<float>(0x188) - 1.0);
    if (over > 0.0f) {
        const double add = static_cast<double>(over) * t19c;
        stick[0] = static_cast<float>(stick[0] + add);
        stick[1] = static_cast<float>(stick[1] + add);
        stick[2] = static_cast<float>(stick[2] + add);
    }
    const float cap = host.game_tuning_0042e740().at<float>(0x1A0);
    float strength = static_cast<float>(host.vector_length_0042b2f0(stick));
    if (strength > cap) strength = cap;
    const float timer = static_cast<float>(screen.at<float>(0x164) - dt);
    screen.at<float>(0x164) = timer;
    if (0.0f > timer) {
        const bool on = screen.at<std::uint8_t>(0x160) == 0;
        screen.at<std::uint8_t>(0x160) = on ? 1 : 0;
        const Raw tuning = host.game_tuning_0042e740();
        const float y0 = on ? tuning.at<float>(0x1B0) : tuning.at<float>(0x1AC);
        const float y1 = on ? tuning.at<float>(0x1B4) : tuning.at<float>(0x1A8);
        screen.at<float>(0x164) = static_cast<float>(host.clamped_interpolate_00419010(0.0f, y0, cap, y1, strength));
        const double next = screen.at<float>(0x164);
        const float high = static_cast<float>(next * host.game_tuning_0042e740().at<float>(0x1B8));
        const float low = static_cast<float>(-next * host.game_tuning_0042e740().at<float>(0x1B8));
        // ECX = 1 (0060AAA2): RNG stream 1, not the gameplay stream 0.
        screen.at<float>(0x164) =
            static_cast<float>(host.random_uniform_00bd2f10(1, low, high) + screen.at<float>(0x164));
    }
    bool shake = screen.at<std::uint8_t>(0x160) != 0 && strength > 0.0f;  // 00D7A218
    if (!shake) {
        const float threshold = static_cast<float>(host.game_tuning_0042e740().at<float>(0x1BC) * static_cast<double>(cap));
        shake = strength >= threshold;
    }
    if (shake) {
        if (screen.at<std::uint8_t>(0x160) == 0) strength = static_cast<float>(strength * kShakeHalf);
        void* const scene = host.game_00e188a8().ptr(0x1ED4);
        if (scene != nullptr) {
            host.scene_apply_0042c450(
                scene, static_cast<float>(host.game_tuning_0042e740().at<float>(0x1A4) * static_cast<double>(strength)));
        }
    } else {
        const Raw effect = unit_at(screen, 0x15C);
        if (effect.p != nullptr) {
            host.point_effect_stop_children_00867b10(effect);
            effect.at<std::uint8_t>(9) = 1;
            refcounted_ptr_release_00605600(Raw{screen.p + 0x15C}, host);  // 00484620(0)
        }
    }
    const Raw last = plane();
    host.widget_set_visible_34(screen.ptr(0x11C), last.p != nullptr && last.at<std::int32_t>(0xC30) != 0);
}

void plane_screen_load_layout_00606c60(Raw screen, Fe5Host& host) {
    screen.ptr(0x0C) = host.gui_load_page_00aa5840("GUI_cross_all");  // 00CF4800
    if (screen.ptr(0x0C) != nullptr) {
        screen.ptr(0x88) = host.gui_find_child_00aa7e00(screen.ptr(0x0C), "cross_tilt_Icon");  // 00CF47F0
        for (std::size_t row = 0; row < kPlaneReticleRows.size(); ++row) {
            void* const group = host.gui_find_child_00aa7e00(screen.ptr(0x0C), kPlaneReticleRows[row]);
            for (std::size_t slot = 0; slot < kPlaneReticleSlots.size(); ++slot) {
                void*& cell = screen.ptr(0x10 + row * 0x14 + slot * 4);
                cell = nullptr;
                if ((kPlaneReticleSlots[slot].skip_rows >> row) & 1) continue;
                cell = host.gui_find_child_00aa7e00(group, kPlaneReticleSlots[slot].name);
            }
            void* const f_icon = screen.ptr(0x14 + row * 0x14);
            if (f_icon != nullptr) {
                void* const f2 = host.gui_find_child_00aa7e00(f_icon, "cross_F2_Icon");  // 00CEDE30
                const float* const size = host.widget_size_00aa6740(f_icon);
                const float w = size[0];
                const float h = size[1];
                const float pos[3] = {static_cast<float>(w * kCrossF2Scale), static_cast<float>(kCrossF2Scale * h), 0.0f};
                host.widget_set_local_position_00aa7dc0(f2, pos);
            }
        }
        screen.ptr(0x8C) = host.gui_find_child_00aa7e00(screen.ptr(0x0C), "payload_template_Icon");     // 00CF4768
        host.widget_set_visible_34(screen.ptr(0x8C), false);
        screen.ptr(0x90) = host.gui_find_child_00aa7e00(screen.ptr(0x0C), "payload_template_Section");  // 00CF474C
        host.widget_set_visible_34(screen.ptr(0x90), false);
    }
    screen.ptr(0xB8) = host.gui_load_page_00aa5840("GUI_plane_effects");  // 00CF4738
    screen.ptr(0xBC) = host.gui_find_child_00aa7e00(screen.ptr(0xB8), "AttackRun_Text");    // 00CF4728
    screen.ptr(0x11C) = host.gui_find_child_00aa7e00(screen.ptr(0xB8), "RepairZone_Text");  // 00CF4718
    void* const top = host.gui_find_child_00aa7e00(screen.ptr(0xB8), "VillanasFelso_Icon");  // 00CF4704
    if (top != nullptr) {
        void* const bottom = host.gui_find_child_00aa7e00(screen.ptr(0xB8), "VillanasAlso_Icon");  // 00CF46F0
        void* const left = host.gui_find_child_00aa7e00(screen.ptr(0xB8), "VillanasBal_Icon");     // 00CF46DC
        void* const right = host.gui_find_child_00aa7e00(screen.ptr(0xB8), "VillanasJobb_Icon");   // 00CF46C8
        host.quad_fade_bind_0064a540(Raw{screen.p + 0x16C}, top, bottom, left, right);
    }
    screen.ptr(0xB4) = host.gui_load_page_00aa5840("GUI_plane");  // 00CF46BC
    for (const PlaneHudWidget& w : kPlaneHudWidgets) {
        void* const parent = w.parent == 0 ? screen.ptr(0xB4) : screen.ptr(w.parent);
        screen.ptr(w.offset) = host.gui_find_child_00aa7e00(parent, w.name);
    }
}

void unit_library_load_unit_model_00680db0(Raw screen, Fe5Host& host) {
    host.unit_library_release_model_parts_0067ed30(screen);
    // 004263B0 appends the decimal class index to the prefix (00CF70CC).
    host.file_block_begin("GIUnitLib::ShowSelectedVehicle_" + std::to_string(screen.at<std::int32_t>(0x130)));
    void* globals = host.lua_globals_00b67980();
    void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");  // 00CE5880
    host.lua_release_00b67700(globals);
    globals = host.lua_globals_00b67980();
    void* const devices = host.lua_get_by_name_00b67800(globals, "DeviceClass");  // 00CE44BC
    host.lua_release_00b67700(globals);
    void* const entry = host.lua_get_by_index_00b67720(classes, screen.at<std::int32_t>(0x130));
    {
        void* const mesh = host.lua_get_by_name_00b67800(entry, "Mesh");  // 00CE5FD0
        screen.ptr(0x108) = host.resource_load_with_game_factory_007188a0(host.lua_get_string_00b662b0(mesh));
        host.lua_release_00b67700(mesh);
    }
    const Raw hull_part = host.unit_part_instance_new_007135c0(screen.ptr(0x108));
    screen.ptr(0x10C) = hull_part.p;
    const Raw hull_node{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(hull_part.ptr(0x160))}.ptr(0x0C))};
    host.node_propagate_root_registration_00b6d890(hull_node, screen.ptr(0x0C));
    const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};  // 00D7A24C = 1.0
    host.node_set_matrix_34(hull_node, identity);
    host.model_groups_select_visible_00710bb0(Raw{static_cast<std::uint8_t*>(screen.ptr(0x10C))}, 0);

    void* const platforms = host.lua_get_by_name_00b67800(entry, "Platforms");  // 00CEB734
    if (host.lua_is_table_00b661b0(platforms)) {
        void* const key = host.lua_nil_00b65f50();
        void* const value = host.lua_nil_00b65f50();
        host.lua_iterate_first_00b67080(platforms, key, value);
        while (!host.lua_iterate_ended_00b66420(platforms, key)) {
            if (host.lua_is_integer_00b66a60(key)) {
                // Gun[1] must be an integer (00681182..0068127C).
                bool bad = true;
                void* const gun = host.lua_get_by_name_00b67800(value, "Gun");  // 00CEB730
                if (host.lua_is_table_00b661b0(gun)) {
                    void* const gun2 = host.lua_get_by_name_00b67800(value, "Gun");
                    void* const first = host.lua_get_by_index_00b67720(gun2, 1);
                    bad = !host.lua_is_integer_00b66a60(first);
                    host.lua_release_00b67700(first);
                    host.lua_release_00b67700(gun2);
                }
                host.lua_release_00b67700(gun);
                if (!bad) {
                    void* const gun3 = host.lua_get_by_name_00b67800(value, "Gun");
                    void* const first = host.lua_get_by_index_00b67720(gun3, 1);
                    const std::int32_t device = host.lua_get_integer_00b66290(first);
                    host.lua_release_00b67700(first);
                    host.lua_release_00b67700(gun3);
                    void* const row = host.lua_get_by_index_00b67720(devices, device);
                    void* const mesh = host.lua_get_by_name_00b67800(row, "Mesh");
                    const bool has_mesh = host.lua_is_string_00b660a0(mesh);
                    host.lua_release_00b67700(mesh);
                    host.lua_release_00b67700(row);
                    if (has_mesh) {
                        const Raw group{static_cast<std::uint8_t*>(host.resource_find_last_named_point_group_00718000(
                            screen.ptr(0x108), "slot", host.lua_get_integer_00b66290(key)))};  // 00CEB728
                        const Raw points{group.p == nullptr ? nullptr : group.p + 0x44};
                        const auto point_count = [&] {
                            return (static_cast<const std::uint8_t*>(points.ptr(8)) - static_cast<const std::uint8_t*>(points.ptr(4))) / 0x0C;
                        };
                        if (group.p != nullptr && points.ptr(4) != nullptr && point_count() > 2) {
                            float basis[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
                            const float* p0 = host.point_vector_at_004803a0(points, 0);
                            basis[12] = p0[0];
                            basis[13] = p0[1];
                            basis[14] = p0[2];
                            auto diff = [&](std::int32_t to, float out[3]) {  // FLD, FSUB, FSTP per component
                                const float* const a = host.point_vector_at_004803a0(points, 0);
                                const float* const b = host.point_vector_at_004803a0(points, to);
                                out[0] = b[0] - a[0];
                                out[1] = b[1] - a[1];
                                out[2] = b[2] - a[2];
                            };
                            diff(2, &basis[8]);
                            float side[3];
                            float ahead[3];
                            diff(1, side);
                            diff(2, ahead);
                            float up[3];
                            host.vector_cross_004fa230(up, ahead, side);
                            basis[4] = up[0];
                            basis[5] = up[1];
                            basis[6] = up[2];
                            host.matrix_orthonormalize_basis_rows_0085dc80(basis);
                            float yaw = 0.0f;
                            float pitch = 0.0f;
                            void* const rest_probe = host.lua_get_by_name_00b67800(value, "RestAngles");  // 00CEB71C
                            const bool has_rest = host.lua_is_table_00b661b0(rest_probe);
                            host.lua_release_00b67700(rest_probe);
                            if (has_rest) {
                                void* const rest = host.lua_get_by_name_00b67800(value, "RestAngles");
                                float pair[2];
                                const float* const angles = host.lua_get_vector2_00b679f0(rest, pair);
                                yaw = angles[0];
                                pitch = angles[1];
                                host.lua_release_00b67700(rest);
                            } else {
                                bool has_window = false;
                                void* const windows = host.lua_get_by_name_00b67800(value, "Windows");  // 00CEB714
                                if (host.lua_is_table_00b661b0(windows)) {
                                    void* const windows2 = host.lua_get_by_name_00b67800(value, "Windows");
                                    void* const window = host.lua_get_by_index_00b67720(windows2, 1);
                                    has_window = host.lua_is_table_00b661b0(window);
                                    host.lua_release_00b67700(window);
                                    host.lua_release_00b67700(windows2);
                                }
                                host.lua_release_00b67700(windows);
                                if (has_window) {
                                    void* const windows3 = host.lua_get_by_name_00b67800(value, "Windows");
                                    void* const window = host.lua_get_by_index_00b67720(windows3, 1);
                                    host.lua_release_00b67700(windows3);
                                    auto mean = [&](const char* low_key, const char* high_key) {
                                        void* const low = host.lua_get_by_name_00b67800(window, low_key);
                                        void* const high = host.lua_get_by_name_00b67800(window, high_key);
                                        const double high_value = host.lua_get_number_00b66270(high);
                                        const double sum = host.lua_get_number_00b66270(low) + high_value;
                                        const float result = static_cast<float>(sum * 0.5);  // 00D7A280
                                        host.lua_release_00b67700(high);
                                        host.lua_release_00b67700(low);
                                        return result;
                                    };
                                    yaw = mean("MinHorzAngle", "MaxHorzAngle");    // 00CEB704 / 00CEB6F4
                                    pitch = mean("MinVertAngle", "MaxVertAngle");  // 00CEB6E4 / 00CEB6D4
                                    host.lua_release_00b67700(window);
                                }
                            }
                            void* const device_row = host.lua_get_by_index_00b67720(devices, device);
                            void* const device_mesh = host.lua_get_by_name_00b67800(device_row, "Mesh");
                            void* const model =
                                host.resource_load_with_game_factory_007188a0(host.lua_get_string_00b662b0(device_mesh));
                            host.lua_release_00b67700(device_mesh);
                            host.lua_release_00b67700(device_row);
                            const Raw part = host.unit_part_instance_new_007135c0(model);
                            const Raw part_node{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(part.ptr(0x160))}.ptr(0x0C))};
                            host.node_propagate_root_registration_00b6d890(part_node, screen.ptr(0x0C));
                            host.node_set_parent_00b6e680(part_node, hull_node.p);
                            float rotation[16];
                            float placed[16];
                            const float* const turn = host.matrix_rotation_y_negated_0042b490(rotation, yaw);
                            host.matrix_multiply_00413920(turn, basis, placed);
                            host.node_set_matrix_38(part_node, placed);
                            void* const barrel = host.model_find_note_node_0071ad50(part.ptr(0x160), "barrel");  // 00CF70C4
                            if (barrel != nullptr) {
                                float elevation[16];
                                host.node_set_matrix_38(Raw{static_cast<std::uint8_t*>(barrel)},
                                                        host.matrix_rotation_x_negated_0042b450(elevation, pitch));
                            }
                            host.model_groups_select_visible_00710bb0(part, 0);
                            host.unit_library_model_vector_push_00680cc0(Raw{screen.p + 0x110}, model);
                            host.unit_library_part_vector_push_00680d30(Raw{screen.p + 0x120}, part.p);
                        }
                    }
                }
            }
            host.lua_iterate_next_00b67190(platforms, key, value);
        }
        host.lua_release_00b67700(value);
        host.lua_release_00b67700(key);
    }
    host.lua_release_00b67700(platforms);
    host.lua_release_00b67700(entry);
    host.lua_release_00b67700(devices);
    host.lua_release_00b67700(classes);
    host.file_block_end();
}
// Packet cc13-trivialslots: the kTrivialSlots bodies; evidence and ABI per routine
// are in the header.
std::int32_t hud_warning_screen_get_screen_id_00682140(Raw) noexcept { return 0x50; }
void multi_menu_deactivate_thunk_00687320(Raw self, Fe5Host& host) {
    host.front_end_manager_deactivate_00683aa0(self);
}
void attract_screen_virtual04no_op_00689bd0(Raw) noexcept {}
std::int32_t hud_screen_slot24_get_screen_id_0068a320(Raw) noexcept { return 0x24; }
std::int32_t hud_screen_slot46_get_screen_id_0068a400(Raw) noexcept { return 0x46; }
void hud_screen_slot46_collect_children_no_op_0068a410(Raw) noexcept {}
std::int32_t hud_screen_slot2f_get_screen_id_0068a4e0(Raw) noexcept { return 0x2F; }
std::int32_t hud_binoculars_overlay_screen_get_screen_id_0068a5d0(Raw) noexcept { return 0x4A; }
void hud_binoculars_overlay_screen_collect_children_no_op_0068a5e0(Raw) noexcept {}
std::int32_t hud_common_overlay_screen_get_screen_id_0068a8f0(Raw) noexcept { return 0x27; }
void observer_endpoint_no_op08_006935b0(Raw) noexcept {}
std::int32_t hud_plane_effects_screen_get_screen_id_00606220(Raw) noexcept { return 0x3F; }
void hud_plane_effects_screen_collect_children_no_op_00606230(Raw) noexcept {}
std::int32_t hud_plane_screen_get_screen_id_00606a80(Raw) noexcept { return 0x3E; }
std::int32_t hud_screen_slot40_get_screen_id_0060c6b0(Raw) noexcept { return 0x40; }
void hud_screen_slot40_register_0060c750(Raw self, Fe5Host& host) {
    host.front_end_screen_register_004f71d0(self);
}
void hud_screen_slot40_update_no_op_0060c760(Raw) noexcept {}
void hud_screen_slot40_enter_no_op_0060c770(Raw) noexcept {}
std::int32_t plane_spawn_camera_slot130_return_zero_0060c960(Raw) noexcept { return 0; }
std::int32_t hud_plane_spawn_screen_get_screen_id_0060cb20(Raw) noexcept { return 0x41; }
std::int32_t hud_screen2e_get_screen_id_00547370(Raw) noexcept { return 0x2E; }
void main_menu_command_owner_listener_no_op1c_00549590(Raw) noexcept {}
std::int32_t hud_hints_screen_get_screen_id_0054d660(Raw) noexcept { return 0x34; }
bool hud_hints_screen_virtual04true_0054d670(Raw) noexcept { return true; }
std::int32_t hud_idle_camera_screen_get_screen_id_0054ecf0(Raw) noexcept { return 0x2C; }
void control_binding_screen_exit_00551b90(Raw) noexcept {}
void control_binding_screen_listener_no_op1c_00551bb0(Raw) noexcept {}

// @impl-end

}  // namespace bsp::cc13_fe5
