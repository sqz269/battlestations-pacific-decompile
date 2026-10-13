// GUI routines of segments 16, 24 and 28 (packet cc13-fe5). Evidence and ABI
// per routine are in include/bsp/frontend_seg16_24_28_cc13.hpp and
// docs/CC13_FE_5.md.

#include "bsp/frontend_seg16_24_28_cc13.hpp"

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

void dword_vector_insert_n_00688470(Raw vector, std::uint32_t* where, std::size_t count,
                                    std::uint32_t value, Fe5Host& host) {
    auto first = [&] { return static_cast<std::uint32_t*>(vector.ptr(4)); };
    auto last = [&] { return static_cast<std::uint32_t*>(vector.ptr(8)); };
    const std::size_t capacity = first() ? static_cast<std::size_t>(static_cast<std::uint32_t*>(vector.ptr(0x0C)) - first()) : 0;
    if (count == 0) return;
    const std::size_t size = first() ? static_cast<std::size_t>(last() - first()) : 0;
    // 0x3FFFFFFF - size < count throws length_error (00688310); not modelled.
    if (capacity < size + count) {
        std::size_t grown = (0x3FFFFFFFu - (capacity >> 1) < capacity) ? 0 : capacity + (capacity >> 1);
        if (grown < size + count) grown = size + count;
        auto* const block = static_cast<std::uint32_t*>(host.stl_allocate_dwords(grown));
        std::uint32_t* out = block;
        for (std::uint32_t* p = first(); p != where; ++p) *out++ = *p;
        for (std::size_t i = 0; i < count; ++i) *out++ = value;
        for (std::uint32_t* p = where; p != last(); ++p) *out++ = *p;
        if (first() != nullptr) host.crt_free(first());
        vector.ptr(0x0C) = block + grown;
        vector.ptr(8) = block + size + count;
        vector.ptr(4) = block;
        return;
    }
    std::uint32_t* const end = last();
    if (static_cast<std::size_t>(end - where) < count) {
        // Tail shorter than the insertion: move the tail past the new block,
        // fill the gap beyond the old end, then fill [where, old end).
        std::uint32_t* out = where + count;
        for (std::uint32_t* p = where; p != end; ++p) *out++ = *p;
        for (std::uint32_t* p = end; p != where + count; ++p) *p = value;
        vector.ptr(8) = end + count;
        for (std::uint32_t* p = where; p != end; ++p) *p = value;
        return;
    }
    std::uint32_t* const split = end - count;
    std::uint32_t* out = end;
    for (std::uint32_t* p = split; p != end; ++p) *out++ = *p;
    vector.ptr(8) = out;
    for (std::uint32_t *s = split, *d = end; s != where;) *--d = *--s;
    for (std::uint32_t* p = where; p != where + count; ++p) *p = value;
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
// @impl-end

}  // namespace bsp::cc13_fe5
