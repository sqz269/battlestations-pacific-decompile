// Segments 12 and 17 front-end reconstructions, packet cc13-fe3 (docs/CC13_FE_3.md).
#include "bsp/frontend_seg12_17_cc13.hpp"
#include "bsp/frontend_menus.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace bsp::cc13_fe3 {
namespace {

Raw raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }

} // namespace

const TrivialSlot* find_trivial_slot(std::uint32_t address) noexcept {
    for (const TrivialSlot& slot : kTrivialSlots) {
        if (slot.address == address) return &slot;
    }
    return nullptr;
}

// --- batch A: small bodies -------------------------------------------------

bool session_message_221_is_type_004fa2f0(std::int32_t type) noexcept { return type == 0xDD; }

void owned_object_delete_004faa90(Raw h, Fe3Host& host) {
    if (h.ptr(8) != nullptr) host.object_delete(h.ptr(8));
    h.ptr(8) = nullptr;
}

void stop_tutorial_movie_004faab0(Fe3Host& host) { host.movie_player_stop_004f8ac0(); }

void widget_set_uniform_scale_004fb9e0(void* widget, float scale, Fe3Host& host) {
    const float xy[2] = {scale, scale};
    host.widget_virtual_48(widget, xy);
}

void icon_atlas_apply_by_name_004fc070(void* atlas, const char* name, void* widget, Fe3Host& host) {
    void* const entry = host.library_find_entry_by_name_004fbf70(atlas, name);
    if (entry == nullptr) return;
    host.gui_set_state_texture_uv_00ab2690(widget, 0, raw(entry).at<std::uint32_t>(0), raw(entry).p + 4);
}

void loading_job_pointers_tidy_004fcbc0(Raw v, Fe3Host& host) {
    host.loading_job_pointers_resize_004fb4b0(v.p, 0);
    host.free_00bf6989(v.ptr(0));
}

void ref_ptr_array_destroy_00505730(Raw v, Fe3Host& host) {
    host.ref_ptr_array_resize_00504960(v.p, 0);
    host.free_00bf6989(v.ptr(0));
}

void award_index_record_destruct_004fe630(Raw r, Fe3Host& host) {
    host.award_index_list_destroy_00487110(r.p + 8);
    if (r.ptr(4) != nullptr) host.sized_pool_return_00bd1510(r.ptr(4), r.at<std::uint32_t>(0) + 1, 1);
}

void owned_library_entry_delete_00501ce0(Raw h, Fe3Host& host) {
    void* const entry = h.ptr(0);
    if (entry == nullptr) return;
    host.library_entry_destruct_004fd1d0(entry);
    host.free_00bf65ac(entry);
    h.ptr(0) = nullptr;
}

void* loading_queue_deleting_destructor_0050ace0(Raw o, std::uint8_t flags, Fe3Host& host) {
    host.loading_queue_destroy_00509fd0(o.p);
    if ((flags & 1) != 0) host.free_00bf65ac(o.p);
    return o.p;
}

void* frame_deleting_destructor_00517d60(Raw o, std::uint8_t flags, Fe3Host& host) {
    host.clear_frame_singleton_00e18d80();
    o.at<std::uint32_t>(0) = 0x00CE3818;  // vtable address in the image
    if ((flags & 1) != 0) host.free_00bf65ac(o.p);
    return o.p;
}

void in_game_screen_23_register_005176a0(Raw s, Fe3Host& host) {
    host.front_end_screen_register_004f71d0(s);
    s.at<std::uint32_t>(0x20) = 0;
}

void in_game_screen_23_on_released_005176c0(Raw o, void* released) noexcept {
    if (released == o.ptr(0x14)) {
        o.ptr(0x14) = nullptr;
        return;
    }
    if (released == o.ptr(0x18)) o.ptr(0x18) = nullptr;
}

void frame_clear_singleton_00517cf0(Raw f, Fe3Host& host) {
    host.clear_frame_singleton_00e18d80();
    f.at<std::uint32_t>(0) = 0x00CE3818;
}

void* frame_construct_00517d10(Raw f, Fe3Host& host) {
    host.front_end_screen_construct_004f7180(f);
    f.at<std::uint32_t>(8) = 0x00CEC388;
    f.at<std::uint32_t>(0) = 0x00CEC39C;
    f.at<std::uint32_t>(8) = 0x00CEC398;
    return f.p;
}

void* iterator_pre_increment(const IteratorPreIncrement& row, void* it, Fe3Host& host) {
    host.iterator_next(row.next, it);
    return it;
}

void* int_scalar_tree_map_construct_00555a40(Raw t, Fe3Host& host) {
    void* const head = host.tree_allocate_head(0x00554CD0, t.p);
    t.ptr(4) = head;
    raw(head).at<std::uint8_t>(0x1D) = 1;
    raw(t.ptr(4)).ptr(4) = t.ptr(4);
    raw(t.ptr(4)).ptr(0) = t.ptr(4);
    raw(t.ptr(4)).ptr(8) = t.ptr(4);
    t.at<std::uint32_t>(8) = 0;
    return t.p;
}

void* pooled_string_list_construct_00557b30(Raw l, Fe3Host& host) {
    l.ptr(4) = host.tree_allocate_head(0x004C3020, l.p);
    l.at<std::uint32_t>(8) = 0;
    return l.p;
}

void tree_clear(const TreeClear& row, Raw t, Fe3Host& host) {
    host.tree_destroy_subtree(row.destroy_subtree, t.p, raw(t.ptr(4)).ptr(4));
    raw(t.ptr(4)).ptr(4) = t.ptr(4);
    t.at<std::uint32_t>(8) = 0;
    raw(t.ptr(4)).ptr(0) = t.ptr(4);
    raw(t.ptr(4)).ptr(8) = t.ptr(4);
}

void string_vector_range_destroy_wrap_0055ac60(void* self, void* a, void* b, Fe3Host& host) {
    host.string_vector_range_destroy_0055a3a0(a, b, self, b);
}

void control_binding_reload_input_config_0055f980(Raw s, Fe3Host& host) {
    host.input_settings_reload_006aa2d0();
    s.at<std::int32_t>(0x16C) += 1;
    host.control_binding_refresh_page_0055e3a0(s, 0);
}

void control_binding_save_and_refresh_0055f9b0(Raw s, Fe3Host& host) {
    host.settings_save_to_profile_008d6c00();
    s.at<std::int32_t>(0x16C) += 1;
    host.control_binding_refresh_page_0055e3a0(s, 0);
}

void control_binding_on_load_preset_prompt_0055f9e0(std::int32_t result, Fe3Host& host) {
    if (result != 1) return;
    host.control_binding_load_selected_preset_0055f710(host.control_binding_screen_00e198b8());
    host.settings_save_to_profile_008d6c00();
}

void leaderboards_exit_00560fe0(Fe3Host& host) { host.game_cancel_leaderboard_read_004bc7f0(); }

std::int32_t leaderboards_selected_row_00561300(Raw s, Fe3Host& host) {
    void* const listbox = s.ptr(0x94);
    if (raw(listbox).at<std::uint8_t>(0x85) != 0) return host.listbox_selected_index_00a9c920(listbox);
    return s.at<std::int32_t>(0xB8);
}

void leaderboards_listener_recapture_00563400(Raw l, Fe3Host& host) {
    host.scroller_recapture_006833f0(l.p + 0x28);
    host.scroller_recapture_006833f0(l.p + 0xB0);
}

void* hud_limbo_construct_00565d40(Raw s, Fe3Host& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(0) = 0x00CEEBAC;
    return s.p;
}

void hud_limbo_exit_00565d70(Raw s) noexcept { s.at<std::uint32_t>(0x14) = 0; }

void limbo_show_config_message_00566050(Raw s, Fe3Host& host) {
    if (s.at<std::uint8_t>(5) == 0) return;
    const char* name = host.global_config_c8_name();
    if (name == nullptr) name = host.empty_string_00e19476();
    host.gui_text_set_localised_00abbe50(s.ptr(0xC), name, 1);
}

// --- batch B: session message 221, library records and templates -----------

namespace {

void copy_native_string(Raw to, Raw from, Fe3RecordHost& host) {
    if (to.p == from.p) return;
    host.native_string_resize_0041dd40(to.p, from.at<std::uint32_t>(0), 1);
    if (from.at<std::uint32_t>(0) != 0) std::memcpy(to.ptr(4), from.ptr(4), to.at<std::uint32_t>(0));
}

void return_string_to_pool(Raw string, Fe3RecordHost& host) {
    if (string.ptr(4) != nullptr) host.sized_pool_return_00bd1510(string.ptr(4), string.at<std::uint32_t>(0) + 1, 1);
}

} // namespace

void session_message_221_write_004fa300(Raw m, void* cursor, Fe3RecordHost& host) {
    host.bit_write_byte_00428ff0(cursor, m.at<std::uint8_t>(0x10), 8);
    host.bit_write_dword_00429070(cursor, m.at<std::uint32_t>(0x18), 6);
    host.bit_write_dword_00429070(cursor, m.at<std::uint32_t>(0x1C), 0x20);
}

void session_message_221_read_004fa340(Raw m, void* cursor, Fe3RecordHost& host) {
    void* const c = static_cast<std::uint8_t*>(cursor) + 4;
    host.bit_read_byte_00428c70(c, m.p + 0x10, 8);
    host.bit_read_dword_00428d10(c, m.p + 0x18, 6);
    host.bit_read_dword_00428d10(c, m.p + 0x1C, 0x20);
}

void* session_message_221_scalar_delete_004fa380(Raw m, std::uint8_t flags, Fe3RecordHost& host) {
    m.at<std::uint32_t>(0) = 0x00CE4974;
    if ((flags & 1) != 0) host.free_00bf65ac(m.p);
    return m.p;
}

void* atlas_scalar_delete_004fb190(Raw a, std::uint8_t flags, Fe3RecordHost& host) {
    host.clear_atlas_singleton_00e18d4c();
    a.at<std::uint32_t>(0) = 0x00CE3818;
    if ((flags & 1) != 0) host.free_00bf65ac(a.p);
    return a.p;
}

void icon_atlas_apply_by_id_004fb150(void* atlas, std::int32_t id, void* widget, Fe3RecordHost& host) {
    void* const entry = host.library_find_entry_by_id_004fb090(atlas, id);
    if (entry == nullptr) return;
    host.gui_set_state_texture_uv_00ab2690(widget, 0, raw(entry).at<std::uint32_t>(0), raw(entry).p + 4);
}

void native_string_to_upper_ascii_004fb1f0(Raw s) noexcept {
    auto* data = static_cast<std::int8_t*>(s.ptr(4));
    if (data == nullptr) return;
    for (std::uint32_t i = 0; i < s.at<std::uint32_t>(0); ++i) {
        if (data[i] >= 'a' && data[i] <= 'z') data[i] = static_cast<std::int8_t>(data[i] - 0x20);
    }
}

void loading_job_pointers_push_back_004fc0a0(Raw v, void* const* value, Fe3RecordHost& host) {
    if (v.at<std::uint32_t>(4) == v.at<std::uint32_t>(8)) {
        const std::int32_t doubled = static_cast<std::int32_t>(v.at<std::uint32_t>(8)) * 2;
        host.loading_job_pointers_reserve_004fb310(v.p, static_cast<std::uint32_t>(doubled > 1 ? doubled : 1));
    }
    void** slot = static_cast<void**>(v.ptr(0)) + v.at<std::uint32_t>(4);
    if (slot != nullptr) *slot = *value;
    v.at<std::uint32_t>(4) += 1;
}

void ref_ptr_array_push_back_00504870(Raw v, void* const* value, Fe3RecordHost& host) {
    if (v.at<std::uint32_t>(4) == v.at<std::uint32_t>(8)) {
        const std::int32_t doubled = static_cast<std::int32_t>(v.at<std::uint32_t>(8)) * 2;
        host.ref_ptr_array_reserve_005017a0(v.p, static_cast<std::uint32_t>(doubled > 1 ? doubled : 1));
    }
    void** slot = static_cast<void**>(v.ptr(0)) + v.at<std::uint32_t>(4);
    if (slot != nullptr) {
        *slot = nullptr;
        if (*value != nullptr) {
            *slot = *value;
            host.interlocked_increment(static_cast<std::uint8_t*>(*value) + 4);
        }
    }
    v.at<std::uint32_t>(4) += 1;
}

void* ref_counted_ptr_assign_004fc450(Raw h, void* object, Fe3RecordHost& host) {
    void* const old = h.ptr(0);
    if (old != nullptr) {
        if (host.interlocked_decrement(static_cast<std::uint8_t*>(old) + 4) == 0) host.ref_counted_destroy(old);
        h.ptr(0) = nullptr;
    }
    h.ptr(0) = object;
    return h.p;
}

void library_carousel_step_back_004fc720(Raw carousel, Fe3RecordHost& host) {
    if (carousel.at<std::int32_t>(0) == 0) return;
    carousel.at<std::int32_t>(0) -= 1;
    host.library_reset_order_state_004f8fa0(host.library_screen());
    if (host.listbox_selected_item_00a9ca00(host.library_screen().ptr(0xB8)) == nullptr) return;
    void* const item = host.listbox_selected_item_00a9ca00(host.library_screen().ptr(0xB8));
    const char* text = static_cast<const char*>(raw(host.item_virtual_28(item)).ptr(4));
    if (text == nullptr) text = host.empty_string_00e18d78();
    const std::int32_t id = host.atol_00bf8417(text);
    const Raw screen = host.library_screen();
    screen.at<std::uint32_t>(0x138 + id * 4) = screen.at<std::uint32_t>(0x434);
}

bool library_order_of_battle_contains_004fc840(Raw screen, std::int32_t side, std::int32_t id, Fe3RecordHost& host) {
    const Raw v = raw(screen.p + side * 0x154 + 0x14C);
    for (std::uint32_t i = 0;; ++i) {
        auto* begin = static_cast<std::uint8_t*>(v.ptr(4));
        if (begin == nullptr) return false;
        const std::uint32_t count = static_cast<std::uint32_t>(static_cast<std::uint8_t*>(v.ptr(8)) - begin) / 8;
        if (i >= count) return false;
        const char* text = static_cast<const char*>(raw(begin + i * 8).ptr(4));
        if (text == nullptr) text = host.empty_string_00e18d78();
        if (id == host.atol_00bf8417(text)) return true;
    }
}

void string_vector_destroy(const StringVectorDestroy& row, Raw r, Fe3RecordHost& host) {
    if (row.pre_zero >= 0) r.at<std::uint32_t>(static_cast<std::size_t>(row.pre_zero)) = 0;
    const std::size_t v = row.vector_offset;
    if (r.ptr(v) != nullptr) host.free_00bf65ac(r.ptr(v));
    r.ptr(v) = nullptr;
    r.ptr(v + 4) = nullptr;
    r.ptr(v + 8) = nullptr;
    return_string_to_pool(raw(r.p + row.string_offset), host);
}

void* named_value_construct_004fe660(Raw v, const void* string, const std::int32_t* number, Fe3RecordHost& host) {
    v.at<std::uint32_t>(0) = 0;
    v.ptr(4) = nullptr;
    copy_native_string(v, raw(const_cast<void*>(string)), host);
    v.at<std::int32_t>(8) = *number;
    return v.p;
}

void* named_value_copy_construct_004fe6b0(Raw v, const void* source, Fe3RecordHost& host) {
    v.at<std::uint32_t>(0) = 0;
    v.ptr(4) = nullptr;
    const Raw from = raw(const_cast<void*>(source));
    copy_native_string(v, from, host);
    v.at<std::int32_t>(8) = from.at<std::int32_t>(8);
    return v.p;
}

void loading_queue_start_work_00501620(Raw q, void* job, Fe3RecordHost& host) {
    if (q.ptr(0x1C) == nullptr) q.ptr(0x1C) = host.create_thread(nullptr, 0, 0x00501510, q.p + 4, 0, nullptr);
    if (raw(job).at<std::uint32_t>(0xC) == 0) return;
    q.ptr(8) = job;
    host.worker_virtual_04(q.ptr(0xC));
}

void* library_entry_copy_backward_00501ea0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
    Fe3RecordHost& host) {
    std::uint8_t* const dest_begin = dest_end - (last - first);
    while (last != first) {
        last -= 0x6C;
        dest_end -= 0x6C;
        host.library_entry_assign_004fe310(dest_end, last);
    }
    return dest_begin;
}

void award_record_destroy_005020a0(Raw r, Fe3RecordHost& host) {
    host.award_record_destroy_storage_00501fa0(r.p + 8);
    return_string_to_pool(r, host);
}

void* award_record_construct_list_00502110(Raw r, Fe3RecordHost& host) {
    r.at<std::uint32_t>(0) = 0;
    r.ptr(4) = nullptr;
    r.ptr(0xC) = host.list_buy_head_node_004870f0(r.p + 8);
    r.at<std::uint32_t>(0x10) = 0;
    return r.p;
}

void vector_throw_subscript_00504e30(Fe3RecordHost& host) { host.throw_vector_subscript(); }

void loading_job_delete_005060e0(Raw h, Fe3RecordHost& host) {
    void* const job = h.ptr(0);
    if (job == nullptr) return;
    host.loading_job_destroy_storage_005051a0(job);
    host.free_00bf65ac(job);
    h.ptr(0) = nullptr;
}

void* record_2c_assign_00506140(Raw to, Raw from, Fe3RecordHost& host) {
    to.at<std::uint32_t>(0) = from.at<std::uint32_t>(0);
    to.at<float>(4) = from.at<float>(4);
    to.at<std::uint32_t>(8) = from.at<std::uint32_t>(8);
    to.at<std::uint8_t>(0xC) = from.at<std::uint8_t>(0xC);
    to.at<std::uint8_t>(0xD) = from.at<std::uint8_t>(0xD);
    copy_native_string(raw(to.p + 0x10), raw(from.p + 0x10), host);
    host.pointer_vector_assign_004f60e0(to.p + 0x18, from.p + 0x18);
    to.at<std::uint32_t>(0x28) = from.at<std::uint32_t>(0x28);
    return to.p;
}

void* record_2c_copy_005061b0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest, Fe3RecordHost& host) {
    for (; first != last; first += 0x2C, dest += 0x2C) record_2c_assign_00506140(raw(dest), raw(first), host);
    return dest;
}

void* record_2c_copy_backward_00506230(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
    Fe3RecordHost& host) {
    while (first != last) {
        last -= 0x2C;
        dest_end -= 0x2C;
        record_2c_assign_00506140(raw(dest_end), raw(last), host);
    }
    return dest_end;
}

void* award_record_construct_00506270(Raw r, Fe3RecordHost& host) {
    r.at<std::uint32_t>(0) = 0;
    r.ptr(4) = nullptr;
    for (std::size_t off = 0xC; off <= 0x20; off += 4) r.at<std::uint32_t>(off) = 0;
    award_record_construct_list_00502110(raw(r.p + 0x28), host);
    r.at<std::uint32_t>(0x48) = 0;
    r.at<std::uint32_t>(0x4C) = 0;
    r.at<std::uint32_t>(0x50) = 0;
    return r.p;
}

// --- batch B (continued) ---------------------------------------------------

namespace {

void copy_native_string_t(Raw to, Raw from, Fe3TemplateHost& host) {
    if (to.p == from.p) return;
    host.native_string_resize_0041dd40(to.p, from.at<std::uint32_t>(0), 1);
    if (from.at<std::uint32_t>(0) != 0) std::memcpy(to.ptr(4), from.ptr(4), to.at<std::uint32_t>(0));
}

void link_head_to_itself(void* head) noexcept {
    raw(head).ptr(4) = head;
    raw(head).ptr(0) = head;
    raw(head).ptr(8) = head;
}

} // namespace

void loading_queue_stop_and_wait_005092e0(Raw q, Fe3TemplateHost& host) {
    host.log_004254b0("GIAchievements::StopLoadingThread()");
    if (q.at<std::uint32_t>(0x14) == 0) return;
    host.loader_virtual_18();
    raw(raw(q.ptr(0x10)).ptr(0)).at<std::uint8_t>(4) = 1;
    while (q.at<std::uint32_t>(0x14) != 0) {
        host.sleep(100);
        host.vfs_tick_providers_00bdb0b0();
        host.loading_queue_update_00509190(q);
    }
    host.loader_virtual_1c();
}

void* string_tail_copy(const StringTailCopy& row, Raw to, Raw from, Fe3TemplateHost& host) {
    to.at<std::uint32_t>(0) = 0;
    to.ptr(4) = nullptr;
    copy_native_string_t(to, from, host);
    host.value_copy(row.tail_copy, to.p + 8, from.p + 8);
    return to.p;
}

void* tree_buy_node(const TreeBuyNode& row, void* left, void* parent, void* right, const void* value,
    std::uint8_t colour, Fe3TemplateHost& host) {
    void* const node = host.operator_new_00bf681b(row.size);
    if (node == nullptr) return nullptr;
    raw(node).ptr(0) = left;
    raw(node).ptr(4) = parent;
    raw(node).ptr(8) = right;
    host.value_copy(row.value_copy, raw(node).p + 0xC, value);
    raw(node).at<std::uint8_t>(row.colour_offset) = colour;
    raw(node).at<std::uint8_t>(row.colour_offset + 1u) = 0;
    return node;
}

void* int_scalar_tree_buy_node_00554f50(void* left, void* parent, void* right, const std::uint32_t value[2],
    std::uint8_t colour, Fe3TemplateHost& host) {
    void* const node = host.operator_new_00bf681b(0x18);
    if (node == nullptr) return nullptr;
    raw(node).ptr(0) = left;
    raw(node).ptr(8) = right;
    raw(node).ptr(4) = parent;
    raw(node).at<std::uint32_t>(0xC) = value[0];
    raw(node).at<std::uint32_t>(0x10) = value[1];
    raw(node).at<std::uint8_t>(0x14) = colour;
    raw(node).at<std::uint8_t>(0x15) = 0;
    return node;
}

void library_on_listbox_selection_changed_00515b40(Raw l, void* item, void* listbox, Fe3TemplateHost& host) {
    if (item == nullptr || l.at<std::int32_t>(0x30) == 7 || listbox != l.ptr(0x54)) return;
    const auto id_of = [&] {
        const char* text = static_cast<const char*>(raw(host.item_virtual_28(item)).ptr(4));
        return host.atol_00bf8417(text != nullptr ? text : host.empty_string_00e18d78());
    };
    if (id_of() == 99999) return;
    const Raw screen = raw(l.p - 0x64);
    host.library_select_order_of_battle_side_00511bf0(screen, id_of());
    if (l.at<std::uint32_t>(0x4A4) != 0) host.library_refresh_entry_detail_00510c20(screen);
}

void in_game_screen_23_destruct_00517620(Raw s, Fe3TemplateHost& host) {
    s.at<std::uint32_t>(0) = 0x00CEC194;
    s.at<std::uint32_t>(8) = 0x00CEC17C;
    host.callback_owner_destroy_00695870(s.p + 8);
    host.front_end_screen_destruct_004f71a0(s);
}

void hud_bomber_destruct_005193d0(Raw s, Fe3TemplateHost& host) {
    s.at<std::uint32_t>(0) = 0x00CEC590;
    s.at<std::uint32_t>(8) = 0x00CEC578;
    host.gui_remove_page_00aa31f0(s.ptr(0x2C));
    host.callback_owner_destroy_00695870(s.p + 8);
    host.front_end_screen_destruct_004f71a0(s);
}

void in_game_screen_23_ensure_observer_00517730(Raw s, Fe3TemplateHost& host) {
    if (s.ptr(0x1C) == nullptr) return;
    if (!host.observer_is_registered_00694af0(s.ptr(0x1C), s.p + 8)) host.observer_register_00694a60(s.ptr(0x1C), s.p + 8);
}

void* in_game_screen_23_construct_005178b0(Raw s, Fe3TemplateHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(8) = 0x00CE3CD4;
    s.at<std::uint32_t>(0xC) = 0;
    s.at<std::uint32_t>(0x10) = 0;
    s.at<std::uint32_t>(0x14) = 0;
    s.at<std::uint8_t>(0x18) = 0;
    s.at<std::uint32_t>(0x1C) = 0;
    s.at<std::uint32_t>(0) = 0x00CEC194;
    s.at<std::uint32_t>(8) = 0x00CEC17C;
    return s.p;
}

void set_player_rank_icon_frame_00517cb0(void* widget, std::int32_t rank, Fe3TemplateHost& host) {
    std::int32_t frame = rank - 1;
    if (frame < 0) frame = 0;
    else if (frame > 0x13) frame = 0x13;
    host.widget_virtual_88(widget, frame, 0, 1.0f);
}

void frame_set_selection_00518da0(void* selection, Fe3TemplateHost& host) {
    host.set_frame_selection_00e18d84(selection);
    host.frame_apply_selection_00518c80();
}

void hud_bomber_on_released_005190a0(Raw o, void* released) noexcept {
    if (released == o.ptr(0x14)) {
        o.ptr(0x14) = nullptr;
        return;
    }
    if (released == o.ptr(0x20)) o.ptr(0x20) = nullptr;
}

void* tree_allocate_head(std::uint8_t size, std::uint8_t colour_offset, Fe3TemplateHost& host) {
    void* const head = host.operator_new_00bf681b(size);
    if (head == nullptr) return nullptr;
    raw(head).ptr(0) = nullptr;
    raw(head).ptr(4) = nullptr;
    raw(head).ptr(8) = nullptr;
    raw(head).at<std::uint8_t>(colour_offset) = 1;
    raw(head).at<std::uint8_t>(colour_offset + 1u) = 0;
    return head;
}

void* tree_construct(const TreeConstruct& row, Raw t, const void* source, Fe3TemplateHost& host) {
    void* const head = host.tree_allocate_head(row.allocate_head, t.p);
    t.ptr(4) = head;
    raw(head).at<std::uint8_t>(row.is_nil_offset) = 1;
    link_head_to_itself(t.ptr(4));
    t.at<std::uint32_t>(8) = 0;
    if (row.copy != 0) host.tree_copy(row.copy, t.p, source);
    return t.p;
}

void tree_destroy_subtree(const TreeDestroySubtree& row, void* node, Fe3TemplateHost& host) {
    while (raw(node).at<std::uint8_t>(row.is_nil_offset) == 0) {
        tree_destroy_subtree(row, raw(node).ptr(8), host);
        void* const left = raw(node).ptr(0);
        if (row.string_vector_value) {
            const Raw n = raw(node);
            if (row.pre_zero >= 0) n.at<std::uint32_t>(static_cast<std::size_t>(row.pre_zero)) = 0;
            const std::size_t v = row.vector_offset;
            if (n.ptr(v) != nullptr) host.free_00bf65ac(n.ptr(v));
            n.ptr(v) = nullptr;
            n.ptr(v + 4) = nullptr;
            n.ptr(v + 8) = nullptr;
            const std::size_t s = row.string_offset;
            if (n.ptr(s + 4) != nullptr) host.sized_pool_return_00bd1510(n.ptr(s + 4), n.at<std::uint32_t>(s) + 1, 1);
        }
        host.free_00bf65ac(node);
        node = left;
    }
}

void* tree_lower_bound(const TreeLowerBound& row, Raw t, Raw key, Fe3TemplateHost& host) {
    void* result = t.ptr(4);
    void* node = raw(t.ptr(4)).ptr(4);
    while (raw(node).at<std::uint8_t>(row.is_nil_offset) == 0) {
        const Raw n = raw(node);
        bool node_less;
        if (n.at<std::uint32_t>(0xC) == 0) {
            node_less = key.at<std::uint32_t>(0) != 0;
        } else if (key.at<std::uint32_t>(0) == 0) {
            node_less = false;
        } else {
            node_less = host.stricmp_00bf7fbf(static_cast<const char*>(n.ptr(0x10)), static_cast<const char*>(key.ptr(4))) < 0;
        }
        if (node_less) {
            node = n.ptr(8);
        } else {
            result = node;
            node = n.ptr(0);
        }
    }
    return result;
}

void native_string_erase_005558f0(Raw s, std::uint32_t pos, std::uint32_t count, Fe3TemplateHost& host) {
    const std::uint32_t length = s.at<std::uint32_t>(0);
    if (pos >= length) return;
    if (pos + count > length) count = length - pos;
    auto* data = static_cast<std::uint8_t*>(s.ptr(4)) + pos;
    std::memmove(data, data + count, length - pos - count);
    host.native_string_resize_0041dd40(s.p, s.at<std::uint32_t>(0) - count, 1);
}

bool word_vector_construct_n_00557200(Raw v, std::uint32_t n, Fe3TemplateHost& host) {
    v.ptr(4) = nullptr;
    v.ptr(8) = nullptr;
    v.ptr(0xC) = nullptr;
    if (n == 0) return false;
    if (n > 0x3FFFFFFFu) host.vector_length_error_00556c20();
    void* const storage = host.allocate_words_00553260(n);
    v.ptr(4) = storage;
    v.ptr(8) = storage;
    v.ptr(0xC) = static_cast<std::uint32_t*>(storage) + n;
    return true;
}

void pooled_string_list_destroy_00557b50(Raw l, Fe3TemplateHost& host) {
    host.pooled_string_list_clear_storage_004d05e0(l.p);
    host.free_00bf65ac(l.ptr(4));
    l.ptr(4) = nullptr;
}

void native_string_vector_placement_copy(void* to, const void* from, Fe3TemplateHost& host) {
    if (to != nullptr) host.native_string_vector_copy_00557bc0(to, from);
}

void native_string_vector_tidy_00559dc0(Raw v, Fe3TemplateHost& host) {
    if (v.ptr(4) == nullptr) return;
    host.native_string_range_destroy_00432050(v.ptr(4), v.ptr(8), v.p, v.p);
    host.free_00bf65ac(v.ptr(4));
    v.ptr(4) = nullptr;
    v.ptr(8) = nullptr;
    v.ptr(0xC) = nullptr;
}

void string_vector_vector_tidy(Raw v, void* stack_word, Fe3TemplateHost& host) {
    if (v.ptr(4) == nullptr) return;
    host.string_vector_range_destroy_0055a3a0(v.ptr(4), v.ptr(8), v.p, stack_word);
    host.free_00bf65ac(v.ptr(4));
    v.ptr(4) = nullptr;
    v.ptr(8) = nullptr;
    v.ptr(0xC) = nullptr;
}

void string_vector_range_destroy_0055a3a0(std::uint8_t* first, std::uint8_t* last, void* extra1, void* extra2,
    Fe3TemplateHost& host) {
    std::uint8_t* const first0 = first;  // the original keeps the first ECX in a scratch slot
    for (; first != last; first += 0x10) {
        const Raw e = raw(first);
        if (e.ptr(4) == nullptr) continue;
        host.native_string_range_destroy_00432050(e.ptr(4), e.ptr(8), e.p, first0);
        host.free_00bf65ac(e.ptr(4));
        e.ptr(4) = nullptr;
        e.ptr(8) = nullptr;
        e.ptr(0xC) = nullptr;
    }
    static_cast<void>(extra1);
    static_cast<void>(extra2);
}

void int_scalar_tree_clear_owner(Raw owner, Fe3TemplateHost& host) {
    const Raw t = raw(owner.p + 0x10);
    void* const head = t.ptr(4);
    std::uint32_t result[2];
    host.int_scalar_tree_erase_range_005572f0(t.p, result, raw(head).ptr(0), head);
    host.free_00bf65ac(t.ptr(4));
    t.ptr(4) = nullptr;
    t.at<std::uint32_t>(8) = 0;
}

void int_scalar_map_copy_body_0055b300(Raw m, Raw source, Fe3TemplateHost& host) {
    void* const head = m.ptr(4);
    raw(head).ptr(4) = host.int_scalar_map_copy_subtree_0055b060(m.p, raw(source.ptr(4)).ptr(4), head);
    m.at<std::uint32_t>(8) = source.at<std::uint32_t>(8);
    void* const root = raw(m.ptr(4)).ptr(4);
    if (raw(root).at<std::uint8_t>(0x1D) != 0) {
        raw(m.ptr(4)).ptr(0) = m.ptr(4);
        raw(m.ptr(4)).ptr(8) = m.ptr(4);
        return;
    }
    void* left = root;
    while (raw(raw(left).ptr(0)).at<std::uint8_t>(0x1D) == 0) left = raw(left).ptr(0);
    raw(m.ptr(4)).ptr(0) = left;
    void* right = raw(m.ptr(4)).ptr(4);
    while (raw(raw(right).ptr(8)).at<std::uint8_t>(0x1D) == 0) right = raw(right).ptr(8);
    raw(m.ptr(4)).ptr(8) = right;
}

// --- batch B (end) -----------------------------------------------------------

void input_device_pair_destroy_0055b8b0(Raw p, Fe3ScreenHost& host) {
    host.input_device_record_destroy_0055b690(p.p + 8);
    if (p.ptr(4) != nullptr) host.sized_pool_return_00bd1510(p.ptr(4), p.at<std::uint32_t>(0) + 1, 1);
}

void control_binding_on_revert_prompt_0055fa10(std::int32_t result, Fe3ScreenHost& host) {
    if (result == 1 || result == 3) {
        const Raw screen = host.control_binding_screen_00e198b8();
        if (result == 1) host.settings_save_to_profile_008d6c00();
        else host.input_settings_reload_006aa2d0();
        screen.at<std::int32_t>(0x16C) += 1;
        host.control_binding_refresh_page_0055e3a0(screen, 0);
    }
    host.control_binding_screen_00e198b8().at<float>(0x10) = host.revert_value_00ce3800();
    host.input_configuration_load_00698a10();
}

void leaderboards_on_listbox_selection_changed_00561e50(Raw l, void* listbox, Fe3ScreenHost& host) {
    const Raw screen = raw(l.p - 8);
    if (listbox == l.ptr(0x8C)) {
        host.leaderboards_move_selection_00561860(screen, l.p + 0x2C, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(listbox)),
            l.at<std::uint32_t>(0x88), 0);
        host.leaderboards_show_board_name_00561660(screen);
        return;
    }
    host.leaderboards_move_selection_00561860(screen, l.p + 0xB4, l.at<std::uint32_t>(0x114), l.at<std::uint32_t>(0x110), 0);
}

void* leaderboard_row_copy_backward_00562e60(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest_end,
    Fe3ScreenHost& host) {
    std::uint8_t* const dest_begin = dest_end - (last - first);
    while (last != first) {
        last -= 0x40;
        dest_end -= 0x40;
        host.leaderboard_row_assign_005625a0(dest_end, last);
    }
    return dest_begin;
}

void* leaderboards_construct_00563370(Raw s, Fe3ScreenHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(8) = 0x00CEB0FC;
    s.at<std::uint32_t>(0xC) = 0x00CEB110;
    s.at<std::uint32_t>(0) = 0x00CEE9CC;
    s.at<std::uint32_t>(8) = 0x00CEE9B8;
    s.at<std::uint32_t>(0xC) = 0x00CEE998;
    s.at<std::uint32_t>(0x14) = 0;
    s.at<std::uint32_t>(0x18) = 0;
    s.at<std::uint32_t>(0x1C) = 0;
    host.gui_scroller_construct_00683610(s.p + 0x34);
    host.gui_scroller_construct_00683610(s.p + 0xBC);
    return s.p;
}

void hud_limbo_update_00565e00(Raw s, float dt, Fe3ScreenHost& host) {
    host.hud_limbo_poll_respawn_input_00565d80(s);
    const float t = s.at<float>(0x18);
    if (!(t > 0.0f)) return;
    const float left = t - dt;
    s.at<float>(0x18) = left;
    if (0.0f < left || left != left) return;
    s.at<float>(0x18) = 0.0f;
    host.widget_set_visible_34(s.ptr(0x10), 1);
}

void hud_limbo_destruct_00565e60(Raw s, Fe3ScreenHost& host) {
    s.at<std::uint32_t>(0) = 0x00CEEBAC;
    host.gui_remove_page_00aa31f0(s.ptr(8));
    host.front_end_screen_destruct_004f71a0(s);
}

const float* gui_colour_static_red_00568220(Fe3ScreenHost& host) {
    float* const colour = host.static_red_00e19498();
    std::uint32_t& guard = host.static_red_guard_00e194a8();
    if ((guard & 1) == 0) {
        guard |= 1;
        colour[0] = host.one_00d7a24c();
        colour[1] = 0.0f;
        colour[2] = 0.0f;
        colour[3] = colour[0];
    }
    return colour;
}

void* ceed00_scalar_delete(Raw o, std::uint8_t flags, Fe3ScreenHost& host) {
    o.at<std::uint32_t>(0) = 0x00CEED00;
    if ((flags & 1) != 0) host.free_00bf65ac(o.p);
    return o.p;
}

// --- batch C: tactical library helpers -------------------------------------

namespace {

std::uint32_t element_count(Raw owner, std::size_t begin_at, std::uint32_t stride) noexcept {
    auto* begin = static_cast<std::uint8_t*>(owner.ptr(begin_at));
    if (begin == nullptr) return 0;
    return static_cast<std::uint32_t>(static_cast<std::uint8_t*>(owner.ptr(begin_at + 4)) - begin) / stride;
}

Raw element(Raw owner, std::size_t begin_at, std::uint32_t stride, std::uint32_t index) noexcept {
    return raw(static_cast<std::uint8_t*>(owner.ptr(begin_at)) + index * stride);
}

std::int32_t item_id(void* item, Fe3LibraryHost& host) {
    const char* text = static_cast<const char*>(raw(host.item_virtual_28(item)).ptr(4));
    return host.atol_00bf8417(text != nullptr ? text : host.empty_string_00e18d78());
}

// Shared tail of 004FC720 / 004FC7A0.
void library_carousel_apply(Fe3LibraryHost& host) {
    host.library_reset_order_state_004f8fa0(host.library_screen());
    if (host.listbox_selected_item_00a9ca00(host.library_screen().ptr(0xB8)) == nullptr) return;
    const std::int32_t id = item_id(host.listbox_selected_item_00a9ca00(host.library_screen().ptr(0xB8)), host);
    const Raw screen = host.library_screen();
    screen.at<std::uint32_t>(0x138 + id * 4) = screen.at<std::uint32_t>(0x434);
}

} // namespace

void* vector3_assign_cross_004fa230(float* out, const float* a, const float* b) noexcept {
    const double x = static_cast<double>(a[1]) * b[2] - static_cast<double>(a[2]) * b[1];
    const double y = static_cast<double>(b[0]) * a[2] - static_cast<double>(a[0]) * b[2];
    const double z = static_cast<double>(b[1]) * a[0] - static_cast<double>(a[1]) * b[0];
    out[0] = static_cast<float>(x);
    out[1] = static_cast<float>(y);
    out[2] = static_cast<float>(z);
    return out;
}

bool vehicle_class_is_heavy_cruiser_004fab40(std::int32_t index, Fe3LibraryHost& host) {
    void* const globals = host.lua_globals_00b67980();
    void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");  // 00CE5880
    host.lua_release_00b67700(globals);
    void* const entry = host.lua_get_by_index_00b67720(classes, index);
    void* const flag = host.lua_get_by_name_00b67800(entry, "HeavyCruiser");       // 00CEB14C
    const bool result = host.lua_is_boolean_00b66000(flag) && host.lua_get_boolean_00b66250(flag);
    host.lua_release_00b67700(flag);
    host.lua_release_00b67700(entry);
    host.lua_release_00b67700(classes);
    return result;
}

void library_send_player_unlock_unit_004fadc0(Raw screen, std::int32_t unit, std::int32_t extra, Fe3LibraryHost& host) {
    std::uint8_t storage[0x20] = {};
    const Raw message = raw(storage);
    const Raw g = host.game();
    const std::uint32_t player = g.at<std::uint32_t>(0x18C8);
    host.session_message_base_0075b430(message.p, 0xDD);
    message.at<std::uint32_t>(0x18) = player;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint32_t>(0) = 0x00CEB138;
    message.at<std::int32_t>(0x1C) = unit;
    if (host.game().at<std::int32_t>(0x1FE4) == 2) host.session_send_first_peer_00770af0(message.p);
    else host.session_receive_unlock_0076cb30(message.p);
    screen.at<std::int32_t>(0x454) = unit;
    screen.at<std::int32_t>(0x458) = extra;
}

bool library_academy_entry_ready_004fae70(Raw screen, std::int32_t id) noexcept {
    for (std::uint32_t i = 0; i < element_count(screen, 0x408, 0x6C); ++i) {
        const Raw e = element(screen, 0x408, 0x6C, i);
        if (e.at<std::int32_t>(0x40) == id && e.at<std::uint8_t>(0x44) != 0) return true;
    }
    return false;
}

void library_build_entry_locks_004fac50(Raw screen, Fe3LibraryHost& host) {
    for (std::uint32_t i = 0; i < element_count(screen, 0x3C8, 0x6C); ++i) {
        const Raw e = element(screen, 0x3C8, 0x6C, i);
        const char* unlock = static_cast<const char*>(e.ptr(0x34));
        const bool open = unlock == nullptr || host.stricmp_00bf7fbf(unlock, "") == 0 ||
                          host.profile_unlock_satisfied_007fc4c0(e.p + 0x30);
        e.at<std::uint8_t>(0x45) = open ? 0 : 1;
    }
}

void library_on_widget_refresh_004faf40(Raw s, void* w, Fe3LibraryHost& host) {
    if (host.widget_type_5c(w) == 3) {
        host.widget_set_float_4c(w, raw(w).at<std::uint8_t>(0xD4) != 0 ? 0.5f : 1.0f);  // 00CE3800 / 00D7A24C
        return;
    }
    bool forced = false;
    if (w == s.ptr(0x554) || w == s.ptr(0x550)) {
        host.scroller_set_auto_scroll_00683300(s.p + 0x64, 0, 0.0f, 0);
        forced = true;
    } else if (w == s.ptr(0x558)) {
        forced = true;
        host.scroller_end_thumb_drag_00683320(s.p + 0x64);
    }
    bool style = true;
    if (w == s.ptr(0x50C) || w == s.ptr(0x510)) {
        if (host.widget_state_94(w) == 3) style = forced;
    } else {
        for (std::uint32_t i = 0; i < 0x28; ++i) {
            if (w != s.ptr(0x170 + i * 4) && w != s.ptr(0x210 + i * 4)) continue;
            const bool styled_type = host.widget_type_5c(w) == 6 && host.widget_state_94(w) < 3;
            style = styled_type || forced;
            break;
        }
    }
    if (!style) return;
    const std::uint32_t focused = raw(w).at<std::uint8_t>(0xD4) != 0 ? 1u : 0u;
    if (host.widget_type_5c(w) == 6) {
        host.widget_virtual_88(w, focused, 0, 1.0f);
    } else if (host.widget_type_5c(w) == 0x12) {
        host.widget_virtual_84(w, focused);
    }
}

void* library_find_entry_by_id_004fb090(Raw l, std::int32_t id) noexcept {
    for (std::uint32_t i = 0; i < element_count(l, 0x3F8, 0x6C); ++i) {
        if (element(l, 0x3F8, 0x6C, i).at<std::int32_t>(0x48) == id) return element(l, 0x3F8, 0x6C, i).p;
    }
    return nullptr;
}

void* library_find_entry_by_name_004fbf70(Raw l, Raw name, Fe3LibraryHost& host) {
    for (std::uint32_t i = 0; i < element_count(l, 0x3E8, 0x6C); ++i) {
        const Raw key = raw(element(l, 0x3E8, 0x6C, i).p + 0x28);
        if (key.at<std::uint32_t>(0) != name.at<std::uint32_t>(0)) continue;
        if (key.at<std::uint32_t>(0) == 0 ||
            host.stricmp_00bf7fbf(static_cast<const char*>(key.ptr(4)), static_cast<const char*>(name.ptr(4))) == 0) {
            return element(l, 0x3E8, 0x6C, i).p;
        }
    }
    return nullptr;
}

void library_set_maps_available_004fbe90(Raw s, std::uint8_t check, Fe3LibraryHost& host) {
    bool found = false;
    if (check != 0) {
        const Raw first = element(s, 0x470, 0x2C, 0);
        for (std::uint32_t i = 0; !found && i < element_count(first, 0x1C, 4); ++i) {
            if (element(first, 0x1C, 4, i).at<std::uint32_t>(0) == s.at<std::uint32_t>(0x454)) found = true;
        }
    }
    if (!found) library_send_player_unlock_unit_004fadc0(s, -1, 0, host);
}

void library_carousel_step_forward_004fc7a0(Raw c, Fe3LibraryHost& host) {
    const std::int32_t last = static_cast<std::int32_t>(element_count(c, 0x14, 0x6C)) - 1;
    if (c.at<std::int32_t>(0) == last) return;
    c.at<std::int32_t>(0) += 1;
    library_carousel_apply(host);
}

void library_select_initial_entry_004fc8c0(Fe3LibraryHost& host) {
    const std::int32_t id = item_id(host.listbox_selected_item_00a9ca00(host.library_screen().ptr(0xB8)), host);
    const std::int32_t wanted = host.game().at<std::int32_t>(id != 0 ? 0x730 : 0x734);
    const Raw screen = host.library_screen();
    for (std::uint32_t i = 0; i < element_count(screen, 0x448, 0x6C); ++i) {
        if (element(screen, 0x448, 0x6C, i).at<std::int32_t>(0x48) != wanted) continue;
        host.library_screen().at<std::uint32_t>(0x434) = i;
        host.library_screen().at<float>(0x438) = static_cast<float>(static_cast<double>(static_cast<std::int32_t>(i)) + static_cast<double>(0.004f));  // 00CEB160
        return;
    }
}

bool library_is_entry_unlocked_004fc9d0(Raw s, Fe3LibraryHost& host) {
    const std::uint32_t index = s.at<std::uint32_t>(0x434);
    if (element(s, 0x448, 0x6C, index).at<std::uint8_t>(0x46) == 0) return false;
    s.at<std::uint32_t>(0x4B0) = element(s, 0x448, 0x6C, s.at<std::uint32_t>(0x434)).at<std::uint32_t>(0x48);
    static_cast<void>(item_id(host.listbox_selected_item_00a9ca00(s.ptr(0xB8)), host));
    const Raw first = element(s, 0x470, 0x2C, 0);
    const std::int32_t unit = element(first, 0x1C, 4, s.at<std::uint32_t>(0x434)).at<std::int32_t>(0);
    library_send_player_unlock_unit_004fadc0(s, unit, s.at<std::int32_t>(0x138), host);
    return true;
}

std::int32_t library_find_row_by_id_004fcb30(Raw s, std::int32_t id, Fe3LibraryHost& host) {
    for (std::uint32_t i = 0; i < element_count(s, 0x460, 0x2C); ++i) {
        if (item_id(host.listbox_item_00a9be00(s.ptr(0xB8), i), host) == id) return static_cast<std::int32_t>(i);
    }
    return -1;
}

void native_string_release_004fcee0(Raw s, Fe3LibraryHost& host) {
    if (s.ptr(4) != nullptr) host.sized_pool_return_00bd1510(s.ptr(4), s.at<std::uint32_t>(0) + 1, 1);
}

// --- batch C (continued): library entries ----------------------------------

namespace {

void pool_string(Raw s, Fe3EntryHost& host) {
    if (s.ptr(4) != nullptr) host.sized_pool_return_00bd1510(s.ptr(4), s.at<std::uint32_t>(0) + 1, 1);
}

void release_ref(Raw holder, std::size_t off, Fe3EntryHost& host) {
    void* const object = holder.ptr(off);
    if (object != nullptr && host.interlocked_decrement(static_cast<std::uint8_t*>(object) + 4) == 0) {
        host.ref_counted_destroy(object);
    }
    holder.ptr(off) = nullptr;
}

} // namespace

void* library_preview_model_construct_004fcfc0(Raw m, Fe3EntryHost& host) {
    m.at<std::uint32_t>(0) = 0x00CEB130;
    m.at<std::uint32_t>(4) = 1;
    m.at<std::uint32_t>(0) = 0x00CEB168;
    m.at<std::uint32_t>(8) = 0;
    m.at<std::uint32_t>(0xC) = 0;
    m.at<std::uint32_t>(0x10) = 0;
    m.at<std::uint32_t>(0x14) = 0;
    m.at<std::uint32_t>(0x18) = 2;
    const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    host.matrix_copy_004134f0(m.p + 0x20, identity);
    m.at<std::uint32_t>(0x1C) = 0;
    return m.p;
}

void library_preview_model_destruct_004fd080(Raw m, Fe3EntryHost& host) {
    m.at<std::uint32_t>(0) = 0x00CEB168;
    if (m.ptr(0x10) != nullptr) release_ref(m, 0x10, host);
    if (m.ptr(0x14) != nullptr) {
        host.object_delete_04(m.ptr(0x14));
        m.ptr(0x14) = nullptr;
    }
    pool_string(raw(m.p + 8), host);
    host.ref_counted_destroy_base_00bd30f0(m.p);
}

void library_entry_destruct_004fd1d0(Raw e, Fe3EntryHost& host) {
    for (std::size_t off : {0x64, 0x5C, 0x54, 0x4C, 0x38, 0x30, 0x28}) pool_string(raw(e.p + off), host);
    if (e.ptr(0x14) != nullptr) release_ref(e, 0x14, host);
    if (e.ptr(0) != nullptr) release_ref(e, 0, host);
}

bool library_find_current_entry_004fd3a0(Raw l, void** out_object, std::uint8_t out[16], Fe3EntryHost& host) {
    auto* begin = static_cast<std::uint8_t*>(l.ptr(0x3F8));
    const std::uint32_t count =
        begin == nullptr ? 0 : static_cast<std::uint32_t>(static_cast<std::uint8_t*>(l.ptr(0x3FC)) - begin) / 0x6C;
    for (std::uint32_t i = 0; i < count; ++i) {
        const Raw e = raw(begin + i * 0x6C);
        if (e.at<std::uint32_t>(0x48) != l.at<std::uint32_t>(0x454)) continue;
        void* const old = *out_object;
        void* const now = e.ptr(0);
        if (old != now) {
            *out_object = now;
            if (now != nullptr) host.interlocked_increment(static_cast<std::uint8_t*>(now) + 4);
            if (old != nullptr && host.interlocked_decrement(static_cast<std::uint8_t*>(old) + 4) == 0) {
                host.ref_counted_destroy(old);
            }
        }
        std::memcpy(out, e.p + 4, 16);
        return true;
    }
    return false;
}

// --- batch C (continued): VehicleClass Lua readers -------------------------

namespace {

void assign_c_string(Raw out, const char* text, Fe3LuaHost& host) {
    const std::uint32_t length = text != nullptr ? static_cast<std::uint32_t>(std::strlen(text)) : 0u;
    host.native_string_resize_0041dd40(out.p, length, 0);
    if (out.ptr(4) != nullptr) std::memcpy(out.ptr(4), text, out.at<std::uint32_t>(0));
}

const char* class_field_string(std::int32_t index, const char* field, Fe3LuaHost& host, void* release[3]) {
    void* const globals = host.lua_globals_00b67980();
    release[0] = host.lua_get_by_name_00b67800(globals, "VehicleClass");
    host.lua_release_00b67700(globals);
    release[1] = host.lua_get_by_index_00b67720(release[0], index);
    release[2] = host.lua_get_by_name_00b67800(release[1], field);
    return host.lua_get_string_00b662b0(release[2]);
}

void release_three(void* release[3], Fe3LuaHost& host) {
    host.lua_release_00b67700(release[2]);
    host.lua_release_00b67700(release[1]);
    host.lua_release_00b67700(release[0]);
}

} // namespace

void vehicle_class_get_type_004fd7c0(std::int32_t index, Raw out, Fe3LuaHost& host) {
    void* release[3];
    assign_c_string(out, class_field_string(index, "Type", host, release), host);  // 00CE4780
    release_three(release, host);
}

void vehicle_class_get_name_or_no_unlock_004fda50(std::int32_t index, Raw out, Fe3LuaHost& host) {
    if (index == -1) {
        assign_c_string(out, "FE.unitlib_nounlock", host);  // 00CEB184, length 13h
        return;
    }
    void* release[3];
    assign_c_string(out, class_field_string(index, "Name", host, release), host);  // 00CE8ED0
    release_three(release, host);
}

void vehicle_class_get_unlock_dlc_minimum_004fd8d0(std::int32_t index, Raw out, Fe3LuaHost& host) {
    if (index == -1) {
        assign_c_string(out, "0", host);  // 00CE43BC
        return;
    }
    void* const globals = host.lua_globals_00b67980();
    void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");
    host.lua_release_00b67700(globals);
    void* const entry = host.lua_get_by_index_00b67720(classes, index);
    void* const field = host.lua_get_by_name_00b67800(entry, "UnlockDLCMinimum");  // 00CEB170
    std::uint8_t storage[8] = {};
    const Raw temp = raw(storage);
    host.lua_string_or_default_00b685c0(field, "0", temp.p);
    if (out.p != temp.p) {
        host.native_string_resize_0041dd40(out.p, temp.at<std::uint32_t>(0), 1);
        if (temp.at<std::uint32_t>(0) != 0) std::memcpy(out.ptr(4), temp.ptr(4), out.at<std::uint32_t>(0));
    }
    if (temp.ptr(4) != nullptr) host.sized_pool_return_00bd1510(temp.ptr(4), temp.at<std::uint32_t>(0) + 1, 1);
    host.lua_release_00b67700(field);
    host.lua_release_00b67700(entry);
    host.lua_release_00b67700(classes);
}

void lua_object_read_string_field(const char* name, Raw out, void* object, Fe3LuaHost& host) {
    void* const field = host.lua_get_by_name_00b67800(object, name);
    assign_c_string(out, host.lua_get_string_00b662b0(field), host);
    host.lua_release_00b67700(field);
    host.lua_release_00b67700(object);
}

void lua_object_read_string_by_index_004fe1e0(const std::int32_t* index, Raw out, void* object, Fe3LuaHost& host) {
    void* const field = host.lua_get_by_index_00b67720(object, *index);
    assign_c_string(out, host.lua_get_string_00b662b0(field), host);
    host.lua_release_00b67700(field);
    host.lua_release_00b67700(object);
}

// --- batch C (continued): movie, loading worker, ref-pointer arrays --------

namespace {

void copy_into(Raw out, Raw from, Fe3WorkerHost& host) {
    if (out.p == from.p) return;
    host.native_string_resize_0041dd40(out.p, from.at<std::uint32_t>(0), 1);
    if (from.at<std::uint32_t>(0) != 0) std::memcpy(out.ptr(4), from.ptr(4), out.at<std::uint32_t>(0));
}

void pool_temp(Raw s, Fe3WorkerHost& host) {
    if (s.ptr(4) != nullptr) host.sized_pool_return_00bd1510(s.ptr(4), s.at<std::uint32_t>(0) + 1, 1);
}

void temp_from_c_string(Raw temp, const char* text, Fe3WorkerHost& host) {
    temp.at<std::uint32_t>(0) = 0;
    temp.ptr(4) = nullptr;
    host.native_string_resize_0041dd40(temp.p, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (temp.ptr(4) != nullptr) std::memcpy(temp.ptr(4), text, temp.at<std::uint32_t>(0) + 1);
}

} // namespace

void library_stop_movie_004fe7a0(Fe3WorkerHost& host) {
    host.movie_player_stop_004f8ac0();
    std::uint8_t storage[8];
    const Raw temp = raw(storage);
    temp_from_c_string(temp, host.global_string_00e18d78(), host);
    host.sound_stream_start_00a867b0(temp.p);
    pool_temp(temp, host);
    host.sound_stream_fade_in_00a85c20();
}

std::uint32_t loading_queue_worker_main_00501510(Raw state, Fe3WorkerHost& host) {
    host.event_virtual_08(state.ptr(8));
    while (state.at<std::uint32_t>(0) == 0) {
        const Raw request = raw(raw(state.ptr(4)).ptr(8));
        const char* name = static_cast<const char*>(request.ptr(4));
        std::uint8_t storage[8];
        const Raw temp = raw(storage);
        temp_from_c_string(temp, name != nullptr ? name : host.global_string_00e18d78(), host);
        request.ptr(0xC) = host.resource_load_007188a0(temp.p);
        pool_temp(temp, host);
        host.event_virtual_0c(state.ptr(8));
        host.event_virtual_08(state.ptr(8));
    }
    return 1;
}

void ref_ptr_array_reserve_005017a0(Raw a, std::int32_t n, Fe3WorkerHost& host) {
    if (n < 1) n = 1;
    if (a.at<std::int32_t>(8) >= n) return;
    auto** const block = static_cast<void**>(host.operator_new_00bf55be(static_cast<std::uint32_t>(n) * 4));
    auto** const old = static_cast<void**>(a.ptr(0));
    for (std::int32_t i = 0; i < a.at<std::int32_t>(4); ++i) {
        void** slot = block + i;
        if (slot == nullptr) continue;
        *slot = nullptr;
        if (old[i] != nullptr) {
            *slot = old[i];
            host.interlocked_increment(static_cast<std::uint8_t*>(old[i]) + 4);
        }
    }
    for (std::int32_t i = 0; i < a.at<std::int32_t>(4); ++i) {
        void* const object = old[i];
        if (object == nullptr) continue;
        if (host.interlocked_decrement(static_cast<std::uint8_t*>(object) + 4) == 0) host.ref_counted_destroy(object);
        old[i] = nullptr;
    }
    host.free_00bf6989(old);
    a.ptr(0) = block;
    a.at<std::int32_t>(8) = n;
}

void library_read_lua_string_field_00501d10(const std::int32_t* index, Raw out, void* object, Raw fallback,
    Fe3WorkerHost& host) {
    void* const field = host.lua_get_by_index_00b67720(object, *index);
    std::uint8_t storage[8] = {};
    const Raw temp = raw(storage);
    bool made = false;
    Raw source = fallback;
    if (!host.lua_is_nil_00b65fb0(field)) {
        host.native_string_assign_0041e870(temp.p, host.lua_get_string_00b662b0(field));
        source = temp;
        made = true;
    }
    copy_into(out, source, host);
    if (made) pool_temp(temp, host);
    host.lua_release_00b67700(field);
    host.lua_release_00b67700(object);
}

void* library_entry_copy_00501e10(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest, Fe3WorkerHost& host) {
    std::uint8_t* const end = dest + (last - first);
    for (; first != last; first += 0x6C, dest += 0x6C) host.library_entry_assign_004fe310(dest, first);
    return end;
}

// --- batch C (continued): vectors and 2Ch records ---------------------------

void ref_ptr_array_resize_00504960(Raw a, std::int32_t n, Fe3VectorHost& host) {
    if (n > a.at<std::int32_t>(8)) host.ref_ptr_array_reserve_005017a0(a.p, n);
    auto** const data = static_cast<void**>(a.ptr(0));
    for (std::int32_t i = a.at<std::int32_t>(4); i < n; ++i) {
        if (data + i != nullptr) data[i] = nullptr;
    }
    while (n < a.at<std::int32_t>(4)) {
        a.at<std::int32_t>(4) -= 1;
        void*& slot = static_cast<void**>(a.ptr(0))[a.at<std::int32_t>(4)];
        if (slot != nullptr) {
            if (host.interlocked_decrement(static_cast<std::uint8_t*>(slot) + 4) == 0) host.ref_counted_destroy(slot);
            slot = nullptr;
        }
    }
    a.at<std::int32_t>(4) = n;
}

void library_show_current_entry_00504c60(Raw s, Fe3VectorHost& host) {
    auto* entries = static_cast<std::uint8_t*>(s.ptr(0x448));
    s.at<std::uint32_t>(0x4B0) = raw(entries + s.at<std::uint32_t>(0x434) * 0x6C).at<std::uint32_t>(0x48);
    void* const item = host.listbox_selected_item_00a9ca00(s.ptr(0xB8));
    const char* text = static_cast<const char*>(raw(host.item_virtual_28(item)).ptr(4));
    static_cast<void>(host.atol_00bf8417(text != nullptr ? text : host.empty_string_00e18d78()));
    host.library_refresh_unlock_rows_00503250(s, s.at<std::uint32_t>(0x4B0));
    s.at<std::uint32_t>(0x134) = s.at<std::uint32_t>(0x4B0);
}

void record_2c_destroy_range_005052b0(std::uint8_t* first, std::uint8_t* last, Fe3VectorHost& host) {
    for (; first != last; first += 0x2C) {
        const Raw r = raw(first);
        if (r.ptr(0x1C) != nullptr) host.free_00bf65ac(r.ptr(0x1C));
        r.ptr(0x1C) = nullptr;
        r.ptr(0x20) = nullptr;
        r.ptr(0x24) = nullptr;
        if (r.ptr(0x14) != nullptr) host.sized_pool_return_00bd1510(r.ptr(0x14), r.at<std::uint32_t>(0x10) + 1, 1);
    }
}

bool dword_vector_construct_n_00505750(Raw v, std::uint32_t n, Fe3VectorHost& host) {
    v.ptr(4) = nullptr;
    v.ptr(8) = nullptr;
    v.ptr(0xC) = nullptr;
    if (n == 0) return false;
    if (n > 0x3FFFFFFFu) host.vector_length_error_00504f10();
    void* const storage = host.allocate_dwords_004f9ad0(n);
    v.ptr(4) = storage;
    v.ptr(8) = storage;
    v.ptr(0xC) = static_cast<std::uint32_t*>(storage) + n;
    return true;
}

void record_2c_clear_storage_00505cc0(Raw r) noexcept {
    r.at<std::uint32_t>(0x10) = 0;
    r.at<std::uint32_t>(0x14) = 0;
    r.at<std::uint32_t>(0x1C) = 0;
    r.at<std::uint32_t>(0x20) = 0;
    r.at<std::uint32_t>(0x24) = 0;
    r.at<std::uint32_t>(0x28) = 0;
    r.at<std::uint8_t>(0xD) = 1;
}

void* record_2c_copy_construct_00505f10(Raw to, Raw from, Fe3VectorHost& host) {
    to.at<std::uint32_t>(0) = from.at<std::uint32_t>(0);
    to.at<float>(4) = from.at<float>(4);
    to.at<std::uint32_t>(8) = from.at<std::uint32_t>(8);
    to.at<std::uint8_t>(0xC) = from.at<std::uint8_t>(0xC);
    to.at<std::uint8_t>(0xD) = from.at<std::uint8_t>(0xD);
    to.at<std::uint32_t>(0x10) = 0;
    to.ptr(0x14) = nullptr;
    if (to.p != from.p) {
        host.native_string_resize_0041dd40(to.p + 0x10, from.at<std::uint32_t>(0x10), 1);
        if (from.at<std::uint32_t>(0x10) != 0) std::memcpy(to.ptr(0x14), from.ptr(0x14), to.at<std::uint32_t>(0x10));
    }
    host.dword_vector_copy_construct_004f6210(to.p + 0x18, from.p + 0x18);
    to.at<std::uint32_t>(0x28) = from.at<std::uint32_t>(0x28);
    return to.p;
}

// --- batch C (continued): loading queue ---------------------------------------

void loading_queue_update_00509190(Raw q, Fe3QueueHost& host) {
    if (q.at<std::uint32_t>(0x14) == 0) return;
    const auto front = [&q] { return raw(static_cast<void**>(q.ptr(0x10))[0]); };
    if (front().at<std::uint8_t>(4) != 0) {
        const std::int32_t state = front().at<std::int32_t>(0);
        if (state == 0 || state == 2) host.loading_queue_retire_front_00506bf0(q);
    }
    if (q.at<std::uint32_t>(0x14) == 0) return;
    const Raw job = front();
    switch (job.at<std::int32_t>(0)) {
    case 0:
        if (host.vfs_flag_78()) {
            if (host.file_store_request_file(job.p + 0x14)) job.at<std::int32_t>(0) = 1;
            else job.at<std::uint8_t>(4) = 1;
        } else {
            job.at<std::int32_t>(0) = host.file_store_request_file(job.p + 0x14) ? 1 : 2;
        }
        return;
    case 2:
        host.loading_job_ensure_file_block_00504790(job);
        job.at<std::int32_t>(0) = 3;
        host.loading_queue_start_work_00501620(q, job);
        return;
    case 3: {
        if (host.worker_virtual_10(q.ptr(0xC))) return;
        if (job.at<std::uint8_t>(4) != 0) {
            host.release_native_resource_slot_00483850(raw(job.ptr(8)).p + 0xC);
            while (q.at<std::uint32_t>(0x14) != 0) host.loading_queue_retire_front_00506bf0(q);
            return;
        }
        if (job.ptr(0x1C) != nullptr) {
            const Raw item = raw(job.ptr(8));
            host.call_completion(job.ptr(0x1C), item.at<std::uint32_t>(0xC), item.at<std::uint32_t>(8));
        }
        host.loading_work_items_erase_00501670(job.p + 8, 0);
        if (job.at<std::uint32_t>(0xC) == 0) host.loading_queue_retire_front_00506bf0(q);
        else host.loading_queue_start_work_00501620(q, job);
        return;
    }
    default:
        return;
    }
}

void loading_queue_destroy_00509fd0(Raw q, Fe3QueueHost& host) {
    q.at<std::uint32_t>(0) = 0x00CEB198;
    host.loading_queue_stop_and_wait_005092e0(q);
    host.loading_job_pointers_resize_004fb4b0(q.p + 0x10, 0);
    host.free_00bf6989(q.ptr(0x10));
    if (q.ptr(0xC) != nullptr) {
        host.object_delete(q.ptr(0xC));
        q.ptr(0xC) = nullptr;
    }
    host.clear_queue_singleton_00e18d4c();
    q.at<std::uint32_t>(0) = 0x00CE3818;
}

// --- batch C (continued): award records, DLC list -------------------------

namespace {

void construct_string_copy(Raw to, Raw from, Fe3AwardHost& host) {
    to.at<std::uint32_t>(0) = 0;
    to.ptr(4) = nullptr;
    if (to.p == from.p) return;
    host.native_string_resize_0041dd40(to.p, from.at<std::uint32_t>(0), 1);
    if (from.at<std::uint32_t>(0) != 0) std::memcpy(to.ptr(4), from.ptr(4), to.at<std::uint32_t>(0));
}

} // namespace

void* award_record_copy_construct_0050a210(Raw to, Raw from, Fe3AwardHost& host) {
    construct_string_copy(to, from, host);
    to.at<std::uint32_t>(8) = from.at<std::uint32_t>(8);
    construct_string_copy(raw(to.p + 0xC), raw(from.p + 0xC), host);
    construct_string_copy(raw(to.p + 0x14), raw(from.p + 0x14), host);
    construct_string_copy(raw(to.p + 0x1C), raw(from.p + 0x1C), host);
    to.at<std::uint8_t>(0x24) = from.at<std::uint8_t>(0x24);
    host.string_vector_pair_copy_00509830(to.p + 0x28, from.p + 0x28);
    to.at<std::uint32_t>(0x3C) = from.at<std::uint32_t>(0x3C);
    to.at<std::uint32_t>(0x40) = from.at<std::uint32_t>(0x40);
    host.dword_vector_copy_construct_004f6210(to.p + 0x44, from.p + 0x44);
    return to.p;
}

void* award_table_construct_pair_0050ae60(Raw pair, Raw key, const void* record, Fe3AwardHost& host) {
    construct_string_copy(pair, key, host);
    host.award_record_copy_construct_0050a210(pair.p + 8, record);
    return pair.p;
}

void library_receive_dlc_list_0050a820(Raw s, const std::uint8_t* packet, Fe3AwardHost& host) {
    host.string_vector_clear_004954f0(s.p + 0x47C);
    const std::uint8_t* word = packet + 0x18;
    for (std::int32_t i = 0; i < 0xFF; ++i, word += 2) {
        std::uint16_t value;
        std::memcpy(&value, word, 2);
        if (value == 0) continue;
        char text[0x1E];
        std::snprintf(text, sizeof text, "DLC_%2d", static_cast<int>(value));  // 00CEB9F0 through _sprintf_s
        std::uint8_t storage[8] = {};
        const Raw temp = raw(storage);
        host.native_string_resize_0041dd40(temp.p, static_cast<std::uint32_t>(std::strlen(text)), 1);
        if (temp.ptr(4) != nullptr) std::memcpy(temp.ptr(4), text, temp.at<std::uint32_t>(0) + 1);
        host.string_vector_append_00450540(s.p + 0x47C, temp.p);
        if (temp.ptr(4) != nullptr) host.sized_pool_return_00bd1510(temp.ptr(4), temp.at<std::uint32_t>(0) + 1, 1);
    }
}

bool library_is_map_unlocked_0050fd10(Raw s, std::uint32_t a, std::uint32_t b, Fe3AwardHost& host) {
    host.library_open_entry_as_unlocks_0050e3c0(s, a, b, 1);
    auto* records = static_cast<std::uint8_t*>(s.ptr(0x470));
    if (records == nullptr || records == s.ptr(0x474)) return false;
    const Raw first = raw(records);
    auto* begin = static_cast<std::uint8_t*>(first.ptr(0x1C));
    if (begin == nullptr) return false;
    return static_cast<std::uint32_t>(static_cast<std::uint8_t*>(first.ptr(0x20)) - begin) / 4 > 1;
}

void library_open_order_of_battle_view_00514fb0(Raw s, Fe3AwardHost& host) {
    host.widget_set_visible_34(s.ptr(0x4FC), 1);
    host.widget_set_visible_34(s.ptr(0xA8), 1);
    host.widget_set_visible_34(s.ptr(0xC4), 0);
    raw(s.ptr(0xC4)).at<std::uint8_t>(0x77) = 1;
    raw(s.ptr(0xB8)).at<std::uint8_t>(0x77) = 0;
    host.widget_set_visible_34(s.ptr(0xB8), 1);
    host.library_refresh_entry_detail_00510c20(s);
    const bool toggle = s.at<std::int32_t>(0x90) == 1;
    s.at<std::int32_t>(0x94) = 5;
    s.at<std::int32_t>(0x98) = 99;
    if (toggle) host.library_toggle_view_00511b40(s);
    host.library_show_order_of_battle_page_004fff20(s);
    host.library_build_order_of_battle_list_00506660(s);
    host.library_reset_vehicle_class_order_00506510(s);
    host.loading_queue_stop_and_wait_005092e0();
}

// --- batch C (continued): library exit, frame placement -----------------------

void library_exit_0050b2d0(Raw s, Fe3FrameHost& host) {
    for (std::size_t off = 0x540; off <= 0x554; off += 4) {
        if (s.ptr(off) != nullptr) host.widget_set_visible_34(s.ptr(off), 0);
    }
    for (std::size_t off : {0x508, 0x50C, 0x510}) {
        if (s.ptr(off) != nullptr) {
            host.object_delete_04(s.ptr(off));
            s.ptr(off) = nullptr;
        }
    }
    for (std::size_t i = 0; i < 0x28; ++i) {
        for (std::size_t off : {0x1D8 + i * 4, 0x278 + i * 4}) {
            if (s.ptr(off) != nullptr) {
                host.object_delete_04(s.ptr(off));
                s.ptr(off) = nullptr;
            }
        }
    }
    s.at<std::uint32_t>(0x51C) = 0;
    s.at<std::uint32_t>(0x520) = 0;
    s.at<std::uint32_t>(0x524) = 0;
    s.at<std::uint32_t>(0x9C) = 0;
    s.at<std::uint32_t>(0xA0) = 2;
    host.loading_queue_stop_and_wait_005092e0();
    host.library_preview_clear_rotors_0050a950(s);
    s.at<std::int32_t>(0x4B0) = -1;
    s.at<std::int32_t>(0x4B4) = -1;
    host.scroller_attach_content_00683790(s.p + 0xCC, nullptr);
    host.renderer_set_optional_sync_00b33aa0(0);
}

void frame_offset_widget_x_00517b00(void* a, void* b, std::uint8_t b_is_anchor, Fe3FrameHost& host) {
    constexpr double kWidthScale = 960.0;           // 00CEC380
    const double gap = static_cast<double>(0.003f);  // 00CEBE08
    void* const anchor = b_is_anchor != 0 ? b : a;
    void* const moved = b_is_anchor != 0 ? a : b;
    const float* pos = host.widget_local_position_00aa67f0(anchor);
    const float half = static_cast<float>(raw(anchor).at<float>(0x114) / kWidthScale);
    float xyz[3];
    xyz[0] = b_is_anchor != 0 ? static_cast<float>(static_cast<double>(pos[0]) - half - gap)
                              : static_cast<float>(static_cast<double>(half) + pos[0] + gap);
    xyz[1] = pos[1];
    xyz[2] = pos[2];
    host.widget_set_local_position_00aa7dc0(moved, xyz);
}

void set_player_rank_widget_00517ba0(void* badge, void* icon, void* text, std::int32_t rank, Fe3FrameHost& host) {
    std::int32_t r = rank - 1;
    if (r < 0) r = 0;
    else if (r > 0x13) r = 0x13;
    host.gui_text_set_localised_00abbe50(text, host.rank_name_00e08e00(r), 1);
    host.widget_virtual_88(icon, r, 0, 1.0f);
    const float* pos = host.widget_local_position_00aa67f0(text);
    const auto left_edge = [&] {
        const float half = static_cast<float>(raw(text).at<float>(0x114) / 960.0);  // 00CEC380
        return static_cast<double>(pos[0]) - half;
    };
    float xyz[3];
    double x = left_edge();
    xyz[0] = static_cast<float>(x - host.widget_size_00aa6740(icon)[0] - static_cast<double>(0.003f));  // 00CEBE08
    xyz[1] = pos[1];
    xyz[2] = pos[2];
    host.widget_set_local_position_00aa7dc0(icon, xyz);
    x = left_edge();
    const double width = host.widget_size_00aa6740(icon)[0];
    xyz[0] = static_cast<float>(x - (width + width) - static_cast<double>(0.006f));  // 00CEC390
    xyz[1] = pos[1];
    xyz[2] = pos[2];
    host.widget_set_local_position_00aa7dc0(badge, xyz);
}

// --- batch C (continued): front-end frame pages -------------------------------

namespace {

void* title_page_child(const char* name, Fe3PageHost& host) {
    std::uint8_t storage[8] = {};
    const Raw temp = raw(storage);
    host.native_string_resize_0041dd40(temp.p, static_cast<std::uint32_t>(std::strlen(name)), 1);
    if (temp.ptr(4) != nullptr) std::memcpy(temp.ptr(4), name, temp.at<std::uint32_t>(0) + 1);
    const Raw frame = host.frame_get_or_create_004c1ac0();
    const std::int32_t index = host.frame_selection(0x00E18D8C) + host.frame_get_or_create_004c1ac0().at<std::int32_t>(0x160) * 2 + 0x4E;
    void* const child = host.gui_find_child_00aa7e00(frame.ptr(static_cast<std::size_t>(index) * 4), temp.p, 1);
    if (temp.ptr(4) != nullptr) host.sized_pool_return_00bd1510(temp.ptr(4), temp.at<std::uint32_t>(0) + 1, 1);
    return child;
}

} // namespace

void frame_destruct_00517d90(Raw f, Fe3PageHost& host) {
    f.at<std::uint32_t>(0) = 0x00CEC39C;
    f.at<std::uint32_t>(8) = 0x00CEC398;
    for (std::size_t row = 0; row < 13; ++row) {
        for (std::size_t k = 0; k < 5; ++k) {
            if (f.ptr(0xC + row * 4 + k * 0x34) != nullptr) host.gui_remove_page_00aa31f0(f.ptr(0xC + row * 4 + k * 0x34));
        }
    }
    for (std::size_t base : {0x110, 0x138}) {
        for (std::size_t row = 0; row < 2; ++row) {
            for (std::size_t k = 0; k < 5; ++k) {
                if (f.ptr(base + row * 4 + k * 8) != nullptr) host.gui_remove_page_00aa31f0(f.ptr(base + row * 4 + k * 8));
            }
        }
    }
    host.clear_frame_singleton_00e18d80();
    f.at<std::uint32_t>(8) = 0x00CE3818;
    host.front_end_screen_destruct_004f71a0(f);
}

void frame_set_title_position_005186b0(const float box[3], const float title[3], Fe3PageHost& host) {
    void* const title_text = title_page_child("title_Text", host);         // 00CEC3D8
    void* const frame_box = title_page_child("szarnyas_FrameBox", host);   // 00CEC3C4
    host.widget_set_local_position_00aa7dc0(frame_box, box);
    host.widget_set_local_position_00aa7dc0(title_text, title);
}

void frame_get_title_position_00518820(float box[3], float title[3], Fe3PageHost& host) {
    void* const title_text = title_page_child("title_Text", host);
    void* const frame_box = title_page_child("szarnyas_FrameBox", host);
    std::memcpy(box, host.widget_local_position_00aa67f0(frame_box), 12);
    std::memcpy(title, host.widget_local_position_00aa67f0(title_text), 12);
}

void frame_collect_children_00518e00(Raw f, void* out, Fe3PageHost& host) {
    const std::int32_t own = f.at<std::int32_t>(0x160);
    const std::size_t slots[3] = {
        0xC + 4 * static_cast<std::size_t>(own * 13 + host.frame_selection(0x00E18D84)),
        4 * static_cast<std::size_t>(host.frame_selection(0x00E18D88) + own * 2 + 0x44),
        4 * static_cast<std::size_t>(host.frame_selection(0x00E18D8C) + own * 2 + 0x4E),
    };
    for (std::size_t off : slots) {
        if (f.ptr(off) != nullptr) host.push_page_slot_004d6790(out, &f.ptr(off));
    }
}

// --- batch D: control binding screen and input-settings trees -------------

namespace {

void copy_string_d(Raw to, Raw from, Fe3InputHost& host) {
    to.at<std::uint32_t>(0) = 0;
    to.ptr(4) = nullptr;
    if (to.p == from.p) return;
    host.native_string_resize_0041dd40(to.p, from.at<std::uint32_t>(0), 1);
    if (from.at<std::uint32_t>(0) != 0) std::memcpy(to.ptr(4), from.ptr(4), to.at<std::uint32_t>(0));
}

void* load_named_page(const char* name, Fe3InputHost& host) {
    std::uint8_t storage[8] = {};
    const Raw temp = raw(storage);
    host.native_string_resize_0041dd40(temp.p, static_cast<std::uint32_t>(std::strlen(name)), 1);
    if (temp.ptr(4) != nullptr) std::memcpy(temp.ptr(4), name, temp.at<std::uint32_t>(0) + 1);
    void* const page = host.gui_load_page_00aa5840(temp.p, 1, 0);
    if (temp.ptr(4) != nullptr) host.sized_pool_return_00bd1510(temp.ptr(4), temp.at<std::uint32_t>(0) + 1, 1);
    return page;
}

void clear_tree_at(Raw owner, std::size_t offset, Fe3InputHost& host) {
    const Raw t = raw(owner.p + offset);
    void* const head = t.ptr(4);
    std::uint32_t result[2];
    host.int_scalar_tree_erase_range_005572f0(t.p, result, raw(head).ptr(0), head);
    host.free_00bf65ac(t.ptr(4));
    t.ptr(4) = nullptr;
    t.at<std::uint32_t>(8) = 0;
}

} // namespace

void* tree_copy_subtree(const TreeCopySubtree& row, Raw t, void* node, void* parent, Fe3InputHost& host) {
    void* const head = t.ptr(4);
    if (raw(node).at<std::uint8_t>(row.is_nil_offset) != 0) return head;
    void* const copy = host.tree_buy_node(row.buy_node, t.p, head, parent, head, raw(node).p + 0xC,
        raw(node).at<std::uint8_t>(row.colour_offset));
    raw(copy).ptr(0) = host.tree_copy_subtree(row.address, t.p, raw(node).ptr(0), copy);
    raw(copy).ptr(8) = host.tree_copy_subtree(row.address, t.p, raw(node).ptr(8), copy);
    return copy;
}

void* int_key_tree_buy_node_005550d0(void* left, void* parent, void* right, const std::uint32_t* value,
    std::uint8_t colour, Fe3InputHost& host) {
    void* const node = host.operator_new_00bf681b(0x14);
    if (node == nullptr) return nullptr;
    raw(node).ptr(0) = left;
    raw(node).ptr(4) = parent;
    raw(node).ptr(8) = right;
    raw(node).at<std::uint32_t>(0xC) = *value;
    raw(node).at<std::uint8_t>(0x10) = colour;
    raw(node).at<std::uint8_t>(0x11) = 0;
    return node;
}

void* int_scalar_map_buy_node_0055a290(void* left, void* parent, void* right, const void* value,
    std::uint8_t colour, Fe3InputHost& host) {
    void* const node = host.operator_new_00bf681b(0x20);
    if (node == nullptr) return nullptr;
    raw(node).ptr(0) = left;
    raw(node).ptr(4) = parent;
    raw(node).ptr(8) = right;
    raw(node).at<std::uint32_t>(0xC) = raw(const_cast<void*>(value)).at<std::uint32_t>(0);
    host.value_copy(0x00558570, raw(node).p + 0x10, static_cast<const std::uint8_t*>(value) + 4);
    raw(node).at<std::uint8_t>(0x1C) = colour;
    raw(node).at<std::uint8_t>(0x1D) = 0;
    return node;
}

void int_scalar_tree_assign_00555b20(Raw m, Raw source, Fe3InputHost& host) {
    void* const head = m.ptr(4);
    raw(head).ptr(4) = host.tree_copy_subtree(0x005555B0, m.p, raw(source.ptr(4)).ptr(4), head);
    m.at<std::uint32_t>(8) = source.at<std::uint32_t>(8);
    void* const root = raw(m.ptr(4)).ptr(4);
    if (raw(root).at<std::uint8_t>(0x15) != 0) {
        raw(m.ptr(4)).ptr(0) = m.ptr(4);
        raw(m.ptr(4)).ptr(8) = m.ptr(4);
        return;
    }
    void* left = root;
    while (raw(raw(left).ptr(0)).at<std::uint8_t>(0x15) == 0) left = raw(left).ptr(0);
    raw(m.ptr(4)).ptr(0) = left;
    void* right = raw(m.ptr(4)).ptr(4);
    while (raw(raw(right).ptr(8)).at<std::uint8_t>(0x15) == 0) right = raw(right).ptr(8);
    raw(m.ptr(4)).ptr(8) = right;
}

void int_scalar_tree_clear_at_4_00559610(Raw owner, Fe3InputHost& host) { clear_tree_at(owner, 4, host); }

void int_tree_tree_destroy_subtree_0055af60(void* node, Fe3InputHost& host) {
    while (raw(node).at<std::uint8_t>(0x1D) == 0) {
        int_tree_tree_destroy_subtree_0055af60(raw(node).ptr(8), host);
        void* const left = raw(node).ptr(0);
        clear_tree_at(raw(node), 0x10, host);
        host.free_00bf65ac(node);
        node = left;
    }
}

bool vector16_construct_n_00557250(Raw v, std::uint32_t n, Fe3InputHost& host) {
    v.ptr(4) = nullptr;
    v.ptr(8) = nullptr;
    v.ptr(0xC) = nullptr;
    if (n == 0) return false;
    if (n > 0x0FFFFFFFu) host.vector_length_error_00556c90();
    void* const storage = host.allocate_16_00553300(n);
    v.ptr(0xC) = static_cast<std::uint8_t*>(storage) + n * 16;
    v.ptr(4) = storage;
    v.ptr(8) = storage;
    return true;
}

void* string_dword_tail_copy(const StringDwordTailCopy& row, Raw to, Raw from, Fe3InputHost& host) {
    copy_string_d(to, from, host);
    to.at<std::uint32_t>(8) = from.at<std::uint32_t>(8);
    host.value_copy(row.tail_copy, to.p + 0xC, from.p + 0xC);
    return to.p;
}

void* string_sensitivity_pair_construct_005586b0(Raw to, Raw key, const std::uint8_t* value, Fe3InputHost& host) {
    copy_string_d(to, key, host);
    std::memcpy(to.p + 8, value, 4);
    host.value_copy(0x00557590, to.p + 0xC, value + 4);
    return to.p;
}

void* input_device_record_copy_0055b920(Raw to, Raw from, Fe3InputHost& host) {
    for (const MemberCopy& m : kInputDeviceRecordMembers) host.value_copy(m.routine, to.p + m.offset, from.p + m.offset);
    return to.p;
}

void* input_device_pair_construct_0055ba80(Raw pair, Raw key, const void* record, Fe3InputHost& host) {
    copy_string_d(pair, key, host);
    input_device_record_copy_0055b920(raw(pair.p + 8), raw(const_cast<void*>(record)), host);
    return pair.p;
}

void* control_binding_construct_00555770(Raw s, Fe3InputHost& host) {
    host.front_end_screen_construct_004f7180(s);
    s.at<std::uint32_t>(8) = 0x00CEB0FC;
    s.at<std::uint32_t>(0xC) = 0x00CEB110;
    s.at<std::uint32_t>(0) = 0x00CEE524;
    s.at<std::uint32_t>(8) = 0x00CEE510;
    s.at<std::uint32_t>(0xC) = 0x00CEE4F0;
    s.at<std::uint32_t>(0x3C) = 0;
    s.at<std::uint32_t>(0x40) = 0;
    s.at<std::uint32_t>(0x184) = 0;
    s.at<std::uint32_t>(0x188) = 0;
    host.gui_scroller_construct_00683610(s.p + 0x1A0);
    s.at<std::uint8_t>(0xE4) = 0;
    s.at<std::uint32_t>(0xE0) = 10;
    return s.p;
}

void control_binding_destruct_00555820(Raw s, Fe3InputHost& host) {
    s.at<std::uint32_t>(0) = 0x00CEE524;
    s.at<std::uint32_t>(8) = 0x00CEE510;
    s.at<std::uint32_t>(0xC) = 0x00CEE4F0;
    host.gui_remove_page_00aa31f0(s.ptr(0x14));
    host.gui_remove_page_00aa31f0(s.ptr(0x18));
    if (s.ptr(0x188) != nullptr) host.sized_pool_return_00bd1510(s.ptr(0x188), s.at<std::uint32_t>(0x184) + 1, 1);
    if (s.ptr(0x40) != nullptr) host.sized_pool_return_00bd1510(s.ptr(0x40), s.at<std::uint32_t>(0x3C) + 1, 1);
    s.at<std::uint32_t>(8) = 0x00CEB0FC;
    host.front_end_screen_destruct_004f71a0(s);
}

void control_binding_register_00555c60(Raw s, Fe3InputHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.ptr(0x14) = load_named_page("FE_controls_PC_listbox", host);  // 00CEE55C
    s.ptr(0x18) = load_named_page("FE_controls_PC", host);          // 00CEE54C
    for (std::size_t off = 0x16C; off <= 0x17C; off += 4) s.at<std::uint32_t>(off) = 0;
    s.at<std::uint32_t>(0x180) = 1;
    host.screen_virtual_14(s);
}

void leaderboards_listener_style_00560ff0(Raw l, void* w, std::uint8_t focused, Fe3InputHost& host) {
    const std::uint32_t type = host.widget_type_5c(w);
    if (type == 2) return;
    if (host.widget_type_5c(w) == 3) {
        host.widget_set_float_4c(w, focused != 0 ? 0.5f : 1.0f);  // 00CE3800 / 00D7A24C
        return;
    }
    const bool style = w == l.ptr(0xA0) && l.at<std::uint8_t>(0x70) == 0;
    if (!style) return;
    if (host.widget_type_5c(w) == 6) {
        host.widget_virtual_88(w, focused != 0 ? 1u : 0u, 0, 1.0f);
    } else if (host.widget_type_5c(w) == 0x12) {
        host.widget_virtual_84(w, focused != 0 ? 1u : 0u);
    }
}

void hud_pilot_update_binocular_view_005191b0(float dt, Fe3InputHost& host) {
    host.binoculars_update_view_0051f330(host.hud_root_00e198c4().ptr(0x4C), dt);
    void* const plane = host.hud_root_00e198c4().ptr(0x68);
    if (raw(plane).at<std::uint8_t>(5) != 0) host.plane_update_instruments_00609bd0(plane, dt, 0, 0);
    const Raw root = host.hud_root_00e198c4();
    host.screen_2e_set_view_centre_005454b0(root.ptr(0x50), 0.5f, 0.5f, raw(root.ptr(0x4C)).at<float>(0x38));
}

void hud_bomber_register_00519830(Raw s, Fe3InputHost& host) {
    host.front_end_screen_register_004f71d0(s);
    s.at<std::uint32_t>(0x20) = 0;
    s.at<std::uint32_t>(0x1C) = 0;
    s.at<std::uint32_t>(0x28) = 0;
    s.ptr(0x2C) = load_named_page("GUI_bomber", host);  // 00CEC738
}

// --- batch D (continued): leaderboards ----------------------------------------

namespace {

void board_string(Raw to, Raw from, bool construct, Fe3BoardHost& host) {
    if (construct) {
        to.at<std::uint32_t>(0) = 0;
        to.ptr(4) = nullptr;
    }
    if (to.p == from.p) return;
    host.native_string_resize_0041dd40(to.p, from.at<std::uint32_t>(0), 1);
    if (from.at<std::uint32_t>(0) != 0) std::memcpy(to.ptr(4), from.ptr(4), to.at<std::uint32_t>(0));
}

} // namespace

void leaderboards_show_board_name_00561660(Raw s, Fe3BoardHost& host) {
    void* const boards = s.ptr(0x94);
    const std::int32_t index =
        raw(boards).at<std::uint8_t>(0x85) != 0 ? host.listbox_selected_index_00a9c920(boards) : s.at<std::int32_t>(0xB8);
    host.gui_text_set_localised_source_00abaed0(s.ptr(0x9C), host.board_row_00e19468(index).p + 0x14, 1);
    s.at<float>(0x150) = 0.5f;  // 00CE3800
    s.at<std::uint32_t>(0x154) = 1;
}

void leaderboard_row_destroy_005617c0(Raw row, Fe3BoardHost& host) {
    for (std::size_t off : {0x14, 0x0C, 0x04}) {
        if (row.ptr(off + 4) != nullptr) host.sized_pool_return_00bd1510(row.ptr(off + 4), row.at<std::uint32_t>(off) + 1, 1);
    }
}

void* leaderboard_row_copy_00562490(Raw to, Raw from, bool construct, Fe3BoardHost& host) {
    to.at<std::uint32_t>(0) = from.at<std::uint32_t>(0);
    board_string(raw(to.p + 4), raw(from.p + 4), construct, host);
    board_string(raw(to.p + 0xC), raw(from.p + 0xC), construct, host);
    board_string(raw(to.p + 0x14), raw(from.p + 0x14), construct, host);
    std::memcpy(to.p + 0x1C, from.p + 0x1C, 0x24);
    return to.p;
}

void leaderboards_sync_selection_00561ad0(Raw s, std::uint8_t boards, Fe3BoardHost& host) {
    if (s.at<std::uint32_t>(0x154) != 0) return;
    if (boards != 0) {
        if (raw(s.ptr(0x94)).at<std::uint8_t>(0x85) != 0) return;
        s.at<std::int32_t>(0x148) = host.listbox_selected_index_00a9c920(s.ptr(0x11C));
        host.listbox_clear_00a9cdc0(s.ptr(0x11C));
        host.widget_set_focus_60(s.ptr(0x94), 1);
        host.listbox_select_index_00a9c7c0(s.ptr(0x94), s.at<std::int32_t>(0xB8));
    } else {
        if (raw(s.ptr(0x11C)).at<std::uint8_t>(0x85) != 0) return;
        s.at<std::int32_t>(0xB8) = host.listbox_selected_index_00a9c920(s.ptr(0x94));
        host.listbox_clear_00a9cdc0(s.ptr(0x94));
        host.widget_set_focus_60(s.ptr(0x11C), 1);
        host.listbox_select_index_00a9c7c0(s.ptr(0x11C), s.at<std::int32_t>(0x148));
    }
    host.leaderboards_move_selection_00561860(s, s.p + 0xBC, s.at<std::uint32_t>(0x11C), s.at<std::uint32_t>(0x118), 1);
    s.at<std::uint8_t>(0x8D) = boards;
    s.at<std::uint8_t>(0x115) = boards == 0 ? 1 : 0;
}

void leaderboards_return_to_menu_005629a0(Raw s, Fe3BoardHost& host) {
    host.please_wait_hide_00e19698();
    host.set_menu_return_00e08874(0xB);
    host.front_end_push_interface_request_004cc460(1, 0);
    host.frame_set_selection_00518da0(0xC);
    host.listbox_clear_rows_00a9bec0(s.ptr(0x94));
    host.scroller_attach_content_00683790(s.p + 0x34, nullptr);
    host.listbox_clear_rows_00a9bec0(s.ptr(0x11C));
    host.scroller_attach_content_00683790(s.p + 0xBC, nullptr);
}

void leaderboards_destruct_00563440(Raw s, Fe3BoardHost& host) {
    s.at<std::uint32_t>(0) = 0x00CEE9CC;
    s.at<std::uint32_t>(8) = 0x00CEE9B8;
    s.at<std::uint32_t>(0xC) = 0x00CEE998;
    host.gui_remove_page_00aa31f0(s.ptr(0x30));
    if (s.ptr(0x14) != nullptr) {
        host.wide_string_range_destroy_004d0b80(s.ptr(0x14), s.ptr(0x18), s.p + 0x10, s.p);
        host.free_00bf65ac(s.ptr(0x14));
        s.ptr(0x14) = nullptr;
        s.ptr(0x18) = nullptr;
        s.ptr(0x1C) = nullptr;
    }
    s.at<std::uint32_t>(8) = 0x00CEB0FC;
    host.front_end_screen_destruct_004f71a0(s);
}

void leaderboards_begin_read_005635f0(Raw s, Fe3BoardHost& host) {
    if (s.at<std::uint8_t>(0x7C) != 0) host.scroller_end_thumb_drag_00683320(s.p + 0x34);
    if (host.game_reading_7058()) return;
    const Raw wait = host.please_wait_screen_00e19698();
    wait.at<std::uint8_t>(4) = 1;
    wait.at<std::uint8_t>(5) = 1;
    host.front_end_commit_visibility_004f83b0(wait);
    host.screen_virtual_18(wait);
    host.please_wait_render_now_0060d570();
    s.ptr(0x20) = host.wide_string_vector_erase_004d6ea0(s.p + 0x10);
    s.at<std::uint32_t>(0x28) = s.at<std::uint32_t>(0x2C);
    const std::int32_t mode = host.leaderboard_mode_00e19430();
    const std::uint64_t xuid = mode == 2 ? host.xenon_active_xuid_00a3eb00() : 0;
    // The original pushes EDX with only its low byte set to (mode == 1); for mode 2 the upper
    // bytes are the XUID's high dword, otherwise they are whatever EDX held.
    const std::uint32_t friends_word = (static_cast<std::uint32_t>(xuid >> 32) & 0xFFFFFF00u) | (mode == 1 ? 1u : 0u);
    host.game_begin_leaderboard_read_004bc6d0(0, xuid, s.at<std::uint32_t>(0x20), s.at<std::uint32_t>(0x24), 0, friends_word);
    s.at<float>(0x15C) = 0.0f;
}

// --- batch D (continued): leaderboard reads, HUD limbo ------------------------

namespace {

std::int32_t board_index(Raw s, Fe3ReadHost& host) {
    void* const boards = s.ptr(0x94);
    return raw(boards).at<std::uint8_t>(0x85) != 0 ? host.listbox_selected_index_00a9c920(boards) : s.at<std::int32_t>(0xB8);
}

} // namespace

void leaderboards_reset_rows_00564e70(Raw s, Fe3ReadHost& host) {
    s.at<std::uint32_t>(0x148) = 0;
    host.listbox_set_listener_00a9ac40(s.ptr(0x11C), nullptr);
    host.listbox_clear_rows_00a9bec0(s.ptr(0x11C));
    host.gui_text_set_ellipsized_00abb000(s.ptr(0x138), host.board_row_00e19468(board_index(s, host)).p + 0xC, -1.0f, 1);
    s.at<std::uint32_t>(0x24) = 0;
    host.leaderboards_begin_read_005635f0(s, host.board_row_00e19468(board_index(s, host)).at<std::uint32_t>(0));
    s.at<std::uint32_t>(0x154) = 2;
}

void leaderboards_on_read_complete_00564f60(Raw s, const void* results, Fe3ReadHost& host) {
    if (s.at<std::uint8_t>(5) == 0) return;
    const std::uint32_t board = host.board_row_00e19468(board_index(s, host)).at<std::uint32_t>(0);
    if (s.at<std::uint32_t>(0x20) != board || s.at<std::uint32_t>(0x28) != s.at<std::uint32_t>(0x2C)) {
        if (host.please_wait_screen_00e19698().at<std::uint8_t>(4) != 0) host.leaderboards_begin_read_005635f0(s, board);
        return;
    }
    host.results_assign_00563760(s.p + 0x10, results);
    host.leaderboards_fill_rows_00561eb0(s);
    s.at<std::uint32_t>(0x154) = 0;
    if (host.leaderboard_mode() == 2) {
        const std::uint64_t xuid = host.xenon_active_xuid_00a3eb00();
        const Raw r = raw(const_cast<void*>(results));
        auto* begin = static_cast<std::uint8_t*>(r.ptr(4));
        const std::uint32_t count = begin == nullptr ? 0 : static_cast<std::uint32_t>(static_cast<std::uint8_t*>(r.ptr(8)) - begin) / 0x60;
        for (std::uint32_t i = 0; i < count; ++i) {
            if (raw(begin + i * 0x60).at<std::uint64_t>(0x58) == xuid) break;  // the answer is not used
        }
    }
    host.please_wait_screen_00e19698().at<std::uint8_t>(4) = 0;
    if (s.at<std::uint8_t>(0x158) != 0) {
        host.gui_set_enabled_00aa0e00(1);
        host.gui_set_pointer_visible_00aa0e50(1);
    }
}

void leaderboards_cycle_filter_005650c0(Raw s, Fe3ReadHost& host) {
    const std::int32_t mode = (host.leaderboard_mode() + 1) % 3;
    host.set_leaderboard_mode(mode);
    std::uint8_t storage[8] = {};
    const Raw temp = raw(storage);
    host.native_string_assign_0041e870(temp.p, host.filter_name_00e086b4(mode));
    host.gui_text_set_ellipsized_00abb000(s.ptr(0x134), temp.p, -1.0f, 1);  // [00D7A260]
    if (temp.ptr(4) != nullptr) host.sized_pool_return_00bd1510(temp.ptr(4), temp.at<std::uint32_t>(0) + 1, 1);
    s.at<std::uint32_t>(0x2C) = 1;
    if (raw(s.ptr(0x11C)).at<std::uint8_t>(0x85) != 0) s.at<std::uint32_t>(0x164) = 1;
    leaderboards_reset_rows_00564e70(s, host);
}

void hud_limbo_register_00565ee0(Raw s, Fe3ReadHost& host) {
    host.front_end_screen_register_004f71d0(s);
    std::uint8_t storage[8] = {};
    const Raw temp = raw(storage);
    const char* const name = "GUI_limbo";  // 00CEEBD4
    host.native_string_resize_0041dd40(temp.p, 9, 1);
    if (temp.ptr(4) != nullptr) std::memcpy(temp.ptr(4), name, 10);
    s.ptr(8) = host.gui_load_page_00aa5840(temp.p, 1, 0);
    if (temp.ptr(4) != nullptr) host.sized_pool_return_00bd1510(temp.ptr(4), temp.at<std::uint32_t>(0) + 1, 1);
}

void hud_limbo_set_state_00565fb0(Raw s, std::int32_t state, Fe3ReadHost& host) {
    s.at<std::int32_t>(0x14) = state;
    std::uint32_t offset = 0xD8;
    switch (state) {
    case 1: offset = 0xA8; break;
    case 2: offset = 0xB8; break;
    case 3: offset = 0xD0; break;
    case 4: offset = 0xC0; break;
    case 5: offset = 0xB0; break;
    default: break;
    }
    const char* text = host.global_config_string(offset);
    if (text == nullptr) text = host.empty_string_00e19476();
    if (s.at<std::uint8_t>(5) != 0) host.gui_text_set_localised_00abbe50(s.ptr(0xC), text, 1);
}

// --- packet cc13-fe6 additions ----------------------------------------------

void hud_limbo_enter_00566080(Raw screen, Fe6LimboHost& host) {
    screen.ptr(0x0C) = host.find_child_by_name_00aa7e00(screen.ptr(0x08), "Message_Text");
    screen.ptr(0x10) = host.find_child_by_name_00aa7e00(screen.ptr(0x08), "Continue_Text");
    host.widget_set_visible_34(screen.ptr(0x10), 0);
    host.hud_limbo_set_state_00565fb0(screen, screen.at<std::int32_t>(0x14));
    screen.at<float>(0x18) = kLimboContinueDelay;
    host.gamepad_force_clear_all_channels_00a95cb0();
}

void leaderboards_move_selection_00561860(Raw screen, void* scroller, void* list_box, void* clip, bool no_scroll,
    Fe6LeaderboardHost& host) {
    const bool board = list_box == screen.ptr(0x94);
    std::int32_t& first = screen.at<std::int32_t>(board ? 0xB0 : 0x140);
    std::int32_t& last = screen.at<std::int32_t>(board ? 0xB4 : 0x144);
    std::int32_t& selected = screen.at<std::int32_t>(board ? 0xB8 : 0x148);
    const Raw list{static_cast<std::uint8_t*>(list_box)};
    const Raw clip_raw{static_cast<std::uint8_t*>(clip)};
    first = list.at<std::int32_t>(0x104);
    last = 0;
    const std::array<float, 3> clip_pos = host.widget_resolved_position_00aa6750(clip);
    const std::array<float, 2> clip_size = host.widget_size_00aa6740(clip);
    const float clip_y = static_cast<float>(clip_pos[1] - clip_size[1] * clip_raw.at<float>(0x1C));
    for (std::int32_t i = 0; i < list.at<std::int32_t>(0x104); ++i) {
        void* row = host.list_box_row_00a9be00(list_box, i);
        const Raw row_raw{static_cast<std::uint8_t*>(row)};
        const std::array<float, 3> pos = host.widget_resolved_position_00aa6750(row);
        const std::array<float, 2> size = host.widget_size_00aa6740(row);
        const float row_y = static_cast<float>(pos[1] - size[1] * row_raw.at<float>(0x1C));
        // The two sums stay in extended precision; double is exact for float + float.
        const bool visible = row_y > clip_y && static_cast<double>(clip_y) + clip_size[1] >
                                                   static_cast<double>(row_y) + size[1];
        if (visible) {
            if (i < first) first = i;
            else if (last < i) last = i;
        }
        host.widget_set_enabled_60(row, visible ? 1u : 0u);
    }
    const Raw scroll{static_cast<std::uint8_t*>(scroller)};
    if (scroll.at<std::uint8_t>(0x48) == 0 && scroll.at<std::int32_t>(0x20) == 0 && !no_scroll) {
        if (host.list_box_selected_index_00a9c920(list_box) != -1 && host.list_box_selected_index_00a9c920(list_box) < first) {
            host.scroller_toward_start_00683680(scroller,
                host.widget_size_00aa6740(host.list_box_current_row_00425e50(list_box))[1]);
            return;
        }
        if (host.list_box_selected_index_00a9c920(list_box) == -1) return;
        if (host.list_box_selected_index_00a9c920(list_box) > last)
            host.scroller_toward_end_00683700(scroller,
                host.widget_size_00aa6740(host.list_box_current_row_00425e50(list_box))[1]);
        return;
    }
    std::int32_t target = 0;
    if (host.list_box_selected_index_00a9c920(list_box) != -1 && host.list_box_selected_index_00a9c920(list_box) < first) {
        target = first;
    } else {
        if (host.list_box_selected_index_00a9c920(list_box) == -1) return;
        target = last;
        if (host.list_box_selected_index_00a9c920(list_box) <= target) return;
    }
    if (target == -1) return;
    host.list_box_select_index_silently_00a9c8f0(list_box, target);
    selected = target;
}

// --- packet cc13-fe8 -------------------------------------------------------
namespace {

// A NativeString {length +0h, data +4h}; a null data pointer reads as "".
std::string_view native_view(Raw s, std::size_t off) noexcept {
    const char* data = static_cast<const char*>(s.ptr(off + 4));
    return data != nullptr ? std::string_view(data, s.at<std::uint32_t>(off)) : std::string_view();
}

// 00436630: a narrow string from the low byte of each UTF-16 unit.
std::string narrow_low_bytes_00436630(const std::uint16_t* wide) {
    std::string out;
    for (; *wide != 0; ++wide) out.push_back(static_cast<char>(*wide & 0xFF));
    return out;
}

constexpr float kTextWidthUnbounded = -1.0f;  // 00D7A260

} // namespace

void leaderboards_listener_focus_00561320(Raw listener, void* widget, Fe8LeaderboardHost& host) {
    if (host.widget_type_5c(widget) == 2) return;
    const bool focused = raw(widget).at<std::uint8_t>(0xD4) != 0;
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, focused ? 0.5f : 1.0f);  // 00CE3800 / 00D7A24C
        return;
    }
    if (widget == listener.ptr(0x98) || widget == listener.ptr(0x94)) {
        host.scroller_set_auto_scroll_00683300(listener.p + 0x28, 0, 0.0f, 0);
    } else if (widget == listener.ptr(0xA0)) {
        host.scroller_end_thumb_drag_00683320(listener.p + 0x28);
    }
    if (widget == listener.ptr(0x11C) || widget == listener.ptr(0x118)) {
        host.scroller_set_auto_scroll_00683300(listener.p + 0xB0, 0, 0.0f, 0);
    } else if (widget == listener.ptr(0x124)) {
        host.scroller_end_thumb_drag_00683320(listener.p + 0xB0);
    }
    if (host.widget_type_5c(widget) == 6) {
        host.widget_set_highlight_88(widget, focused ? 1u : 0u, 0, 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 0x12) host.widget_set_highlight_84(widget, focused ? 1u : 0u);
}

void leaderboards_build_board_list_00561be0(Raw screen, Fe8LeaderboardHost& host) {
    void* const boards = screen.ptr(0x94);
    if (boards != nullptr) {
        host.list_box_clear_rows_00a9bec0(boards);
        host.list_box_set_listener_00a9ac40(boards, nullptr);
        for (std::uint32_t i = 0; i * 0x40 < 0x480; ++i) {
            void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0x98), 0);
            void* const text = host.find_child_by_name_00aa7e00(row, "filter_Text");
            host.gui_text_set_ellipsized_00abb000(text, native_view(host.board_row_00e19468(i), 4),
                                                  kTextWidthUnbounded, 1);
            host.widget_set_visible_34(row, 1);
            host.widget_set_listener_fields_00aa6bc0(row, screen.p + 0x0C, 0);
            host.widget_set_hit_clip_from_00aa8f30(row, screen.ptr(0x90), 0);
            raw(row).at<std::uint32_t>(0xD8) = 0;
            host.list_box_insert_row_00a9d750(boards, row, 0, 0);
        }
        void* const scroller = screen.p + 0x34;
        host.scroller_attach_content_00683790(scroller, nullptr);
        // 00CEE930 = 18.0, 00CEBDD0 = 1/72; the sum is stored as a double before the subtraction.
        const double content = static_cast<double>(host.widget_size_00aa6740(screen.ptr(0x98))[1]) * 18.0 +
                               0.013888888888888888;
        host.scroller_set_scroll_range_006834a0(
            scroller, static_cast<float>(content - host.widget_size_00aa6740(screen.ptr(0x90))[1]));
        host.list_box_select_index_00a9c7c0(boards, 0);
        host.widget_set_enabled_60(boards, 1);
        raw(boards).at<std::uint8_t>(0x149) = 0;
        host.list_box_set_listener_00a9ac40(boards, screen.p + 8);
        host.list_box_set_field124_00a9ceb0(boards, 0.38749998807907104f, 0);  // 00CEE928
        raw(boards).at<float>(0x12C) = 0.010416666977107525f;                  // 00CEE924
        raw(boards).at<float>(0x130) = 0.0027777778450399637f;                 // 00CEE600
        raw(boards).at<float>(0x134) = 0.0f;
        raw(boards).at<std::uint8_t>(0x138) = 1;
        raw(boards).at<std::uint8_t>(0x14A) = 0;
    }
    host.leaderboards_show_board_name_00561660(screen);
    screen.at<std::uint32_t>(0x150) = 0;
    screen.at<std::uint32_t>(0x154) = 1;
    screen.at<std::uint8_t>(0x115) = 0;
}

void leaderboards_fill_rows_00561eb0(Raw screen, Fe8LeaderboardHost& host) {
    void* const entries = screen.ptr(0x11C);
    host.list_box_set_listener_00a9ac40(entries, nullptr);
    const auto* const begin = static_cast<std::uint8_t*>(screen.ptr(0x14));
    const auto* const end = static_cast<std::uint8_t*>(screen.ptr(0x18));
    for (std::uint32_t i = 0; begin != nullptr && i < static_cast<std::uint32_t>((end - begin) / 0x60); ++i) {
        const Raw result{const_cast<std::uint8_t*>(begin) + i * 0x60};
        void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0x120), 0);
        static const std::uint16_t kEmptyWide[1] = {0};  // 00E19474
        const auto* wide = static_cast<const std::uint16_t*>(result.ptr(0x0C));
        host.gui_text_set_ellipsized_00abb000(host.find_child_by_name_00aa7e00(row, "name_Text"),
                                              narrow_low_bytes_00436630(wide != nullptr ? wide : kEmptyWide),
                                              kTextWidthUnbounded, 0);
        host.gui_text_set_ellipsized_00abb000(host.find_child_by_name_00aa7e00(row, "rank_Text"),
                                              std::to_string(result.at<std::int32_t>(4)), kTextWidthUnbounded, 1);
        host.gui_text_set_ellipsized_00abb000(host.find_child_by_name_00aa7e00(row, "value_Text"),
                                              std::to_string(result.at<std::int32_t>(0x10)), kTextWidthUnbounded, 1);
        host.widget_set_visible_34(row, 1);
        host.widget_set_listener_fields_00aa6bc0(row, screen.p + 0x0C, 0);
        host.widget_set_hit_clip_from_00aa8f30(row, screen.ptr(0x118), 0);
        raw(row).at<std::uint32_t>(0xD8) = 1;
        host.list_box_insert_row_00a9d750(entries, row, 0, 0);
    }
    void* const scroller = screen.p + 0xBC;
    host.scroller_attach_content_00683790(scroller, nullptr);
    const std::int32_t count = raw(entries).at<std::int32_t>(0x104);
    // FMUL by the double 1.005 (00CEE950), FIMUL by the row count, stored as a double.
    const double content =
        static_cast<double>(host.widget_size_00aa6740(screen.ptr(0x120))[1]) * 1.0049999952316284 * count;
    host.scroller_set_scroll_range_006834a0(
        scroller, static_cast<float>(content - host.widget_size_00aa6740(screen.ptr(0x118))[1]));
    host.list_box_clear_00a9cdc0(entries);
    raw(entries).at<std::uint8_t>(0x149) = 0;
    host.list_box_set_listener_00a9ac40(entries, screen.p + 8);
    host.list_box_set_field124_00a9ceb0(entries, 0.4166666567325592f, 0);  // 00CEE948
    raw(entries).at<float>(0x12C) = 0.0031250000465661287f;                // 00CEE944
    raw(entries).at<float>(0x130) = 0.0027777778450399637f;                // 00CEE600
    raw(entries).at<float>(0x134) = 0.0f;
    raw(entries).at<std::uint8_t>(0x138) = 1;
    raw(entries).at<std::uint8_t>(0x14A) = 0;
    if (screen.at<std::int32_t>(0x164) != 0) {
        const std::int32_t saved_board = screen.at<std::int32_t>(0xB8);
        screen.at<std::uint32_t>(0x154) = 0;
        if (raw(entries).at<std::uint8_t>(0x85) == 0) {
            screen.at<std::int32_t>(0xB8) = host.list_box_selected_index_00a9c920(screen.ptr(0x94));
            host.list_box_clear_00a9cdc0(screen.ptr(0x94));
            host.widget_set_enabled_60(entries, 1);
            host.list_box_select_index_00a9c7c0(entries, screen.at<std::int32_t>(0x148));
            host.leaderboards_move_selection_00561860(screen, scroller, entries, screen.ptr(0x118), true);
            screen.at<std::uint8_t>(0x8D) = 0;
            screen.at<std::uint8_t>(0x115) = 1;
        }
        const bool select_last = screen.at<std::int32_t>(0x164) == 2;
        screen.at<std::int32_t>(0xB8) = saved_board;
        if (select_last) host.list_box_select_index_00a9c7c0(entries, raw(entries).at<std::int32_t>(0x104) - 1);
        screen.at<std::int32_t>(0x164) = 0;
    }
    constexpr float kDim = 0.25f;  // 00CE3868
    switch (host.leaderboard_mode_00e19430()) {
    case 0: {
        const std::int32_t first = screen.at<std::int32_t>(0x2C);
        const float down = first + 0x11 < screen.at<std::int32_t>(0x14C) ? 1.0f : kDim;
        const float up = first - 0x11 > 0 ? 1.0f : kDim;
        host.scroller_set_widget_alphas_00683330(scroller, 0.0f, up, down);
        break;
    }
    case 1:
        if (raw(entries).at<std::int32_t>(0x104) > 0x11) host.scroller_set_widget_alphas_00683330(scroller, 1.0f, 1.0f, 1.0f);
        else host.scroller_set_widget_alphas_00683330(scroller, 0.0f, kDim, kDim);
        break;
    case 2:
        host.scroller_set_widget_alphas_00683330(scroller, 0.0f, kDim, kDim);
        break;
    default:
        break;
    }
}

void leaderboards_enter_00562750(Raw screen, Fe8LeaderboardEnterHost& host) {
    host.frame_select_00518d60(0, 0, 0, "FE.main_leaderboards_title");
    host.frame_set_flag_00518b90(2);
    host.command_bar_set_help_line_0054a0c0("");
    void* const title = screen.ptr(0x13C);
    const std::string copy(native_view(raw(title), 0xF4));
    host.gui_text_set_localised_cstring_00abbe50(title, "", 1);
    host.gui_text_set_ellipsized_00abb000(title, copy, kTextWidthUnbounded, 1);
    host.gui_text_set_ellipsized_00abb000(screen.ptr(0x134), host.filter_name_00e086b4(host.leaderboard_mode_00e19430()),
                                          kTextWidthUnbounded, 1);
    screen.at<std::uint32_t>(0x20) = 0;
    host.leaderboards_build_board_list_00561be0(screen);
    screen.at<std::uint8_t>(0x158) = 0;
    screen.at<std::uint8_t>(0x160) = 0;
    screen.at<std::uint32_t>(0x14C) = 0;
    screen.at<std::uint32_t>(0x164) = 0;
    screen.at<std::uint8_t>(0x168) = 0;
    screen.at<std::uint32_t>(0x15C) = 0;
    screen.at<std::uint32_t>(0x2C) = 1;
}

namespace {

std::string lua_string_or_empty(Fe8LuaHost& host, void* table, const char* name) {
    void* const field = host.lua_get_by_name_00b67800(table, name);
    const char* const text = host.lua_get_string_00b662b0(field);
    std::string out = text != nullptr ? std::string(text) : std::string();
    host.lua_release_00b67700(field);
    return out;
}

bool lua_index_is_nil(Fe8LuaHost& host, void* table, std::int32_t index) {
    void* const item = host.lua_get_by_index_00b67720(table, index);
    const bool nil = host.lua_is_nil_00b65fb0(item);
    host.lua_release_00b67700(item);
    return nil;
}

} // namespace

void leaderboards_read_table_005638f0(Fe8BoardTableHost& host) {
    void* const owner = host.lua_owner_open_run_00b6a020(4, "Scripts/datatables/Leaderboards.lua");
    for (std::int32_t k = 0; k < 3; ++k) host.board_ids_clear_00e19434(k);
    host.board_rows_clear_00562f80();
    void* const globals = host.lua_owner_globals_00b67980(owner);
    void* const leaderboards = host.lua_get_by_name_00b67800(globals, "Leaderboards");
    host.lua_release_00b67700(globals);
    void* const unique_ids = host.lua_get_by_name_00b67800(leaderboards, "LB_UniqueID");
    for (std::int32_t k = 0; k < 3; ++k) {
        void* const ids = host.lua_get_by_index_00b67720(unique_ids, k + 1);
        for (std::int32_t j = 1; !lua_index_is_nil(host, ids, j); ++j) {
            void* const item = host.lua_get_by_index_00b67720(ids, j);
            host.board_ids_push_00441f30(k, host.lua_get_integer_00b66290(item));
            host.lua_release_00b67700(item);
        }
        host.lua_release_00b67700(ids);
    }
    void* const boards = host.lua_get_by_name_00b67800(leaderboards, "LBoards");
    for (std::int32_t i = 1; i <= 0x12; ++i) {
        Fe8BoardRow row;
        if (!lua_index_is_nil(host, boards, i)) {
            void* const entry = host.lua_get_by_index_00b67720(boards, i);
            void* const id = host.lua_get_by_name_00b67800(entry, "ID");
            row.id = host.lua_get_integer_00b66290(id);
            host.lua_release_00b67700(id);
            row.name = lua_string_or_empty(host, entry, "Name");
            row.column_name = lua_string_or_empty(host, entry, "ColumnName");
            row.table_desc = lua_string_or_empty(host, entry, "TableDesc");
            void* const columns = host.lua_get_by_name_00b67800(entry, "Columns");
            std::int32_t j = 1;
            for (; !lua_index_is_nil(host, columns, j); ++j) {
                void* const item = host.lua_get_by_index_00b67720(columns, j);
                const std::int32_t value = host.lua_get_integer_00b66290(item);
                host.lua_release_00b67700(item);
                if (j <= static_cast<std::int32_t>(row.columns.size())) row.columns[j - 1] = value;
            }
            row.column_count = j - 1;
            host.lua_release_00b67700(columns);
            host.lua_release_00b67700(entry);
        }
        host.board_rows_push_005636d0(row);
    }
    host.lua_release_00b67700(boards);
    host.lua_release_00b67700(unique_ids);
    host.lua_release_00b67700(leaderboards);
    host.lua_owner_close_00b669a0(owner);
}

void leaderboards_register_00564220(Raw screen, Fe8LeaderboardRegisterHost& host) {
    host.front_end_screen_register_004f71d0(screen);
    screen.ptr(0x30) = host.gui_load_page_00aa5840("FE_leaderboards", 1, 0);
    host.leaderboards_read_table_005638f0();
    const auto find = [&host](void* parent, std::string_view name) {
        return host.find_child_by_name_00aa7e00(parent, name);
    };
    screen.ptr(0x90) = find(screen.ptr(0x30), "clipbox_Clipbox");
    screen.ptr(0x94) = find(screen.ptr(0x90), "filters_Listbox");
    screen.ptr(0x98) = find(screen.ptr(0x94), "listbox_Group");
    host.widget_set_visible_34(screen.ptr(0x98), 0);
    void* const info = find(screen.ptr(0x30), "info_Group");
    screen.ptr(0x9C) = find(info, "info_Text");
    void* const slider = find(screen.ptr(0x30), "slider_Group");
    screen.ptr(0xA0) = find(slider, "scroll_up_Icon");
    screen.ptr(0xA4) = find(slider, "scroll_down_Icon");
    screen.ptr(0xAC) = find(slider, "slider_FrameBox");
    screen.ptr(0xA8) = find(slider, "back_Icon");
    void* const listener = screen.p + 0x0C;
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0xA0), listener, 0);
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0xA4), listener, 0);
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0xAC), listener, 0);
    host.scroller_attach_content_00683790(screen.p + 0x34, screen.ptr(0x94));
    {
        const double back = host.widget_size_00aa6740(screen.ptr(0xA8))[1];
        const float span = static_cast<float>(back - host.widget_size_00aa6740(screen.ptr(0xAC))[1]);
        host.scroller_attach_scroll_bar_00683380(screen.p + 0x34, screen.ptr(0xAC), span, screen.ptr(0xA0),
                                                 screen.ptr(0xA4));
    }
    void* const data = find(screen.ptr(0x30), "data_Group");
    screen.ptr(0x134) = find(data, "filter_Text");
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0x134), listener, 0);
    screen.ptr(0x118) = find(data, "clipbox_2_Clipbox");
    screen.ptr(0x11C) = find(screen.ptr(0x118), "data_Listbox");
    screen.ptr(0x120) = find(screen.ptr(0x11C), "listbox_Group");
    host.widget_set_visible_34(screen.ptr(0x120), 0);
    void* const misc = find(data, "data_misc_Group");
    screen.ptr(0x138) = find(misc, "value_Text");
    screen.ptr(0x13C) = find(misc, "rank_Text");
    void* const slider2 = find(data, "slider2_Group");
    screen.ptr(0x124) = find(slider2, "scroll_up2_Icon");
    screen.ptr(0x128) = find(slider2, "scroll_down2_Icon");
    screen.ptr(0x130) = find(slider2, "slider2_FrameBox");
    screen.ptr(0x12C) = find(slider2, "back_Icon");
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0x124), listener, 0);
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0x128), listener, 0);
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0x130), listener, 0);
    host.scroller_attach_content_00683790(screen.p + 0xBC, screen.ptr(0x11C));
    {
        const double back = host.widget_size_00aa6740(screen.ptr(0x12C))[1];
        const float span = static_cast<float>(back - host.widget_size_00aa6740(screen.ptr(0x130))[1]);
        host.scroller_attach_scroll_bar_00683380(screen.p + 0xBC, screen.ptr(0x130), span, screen.ptr(0x124),
                                                 screen.ptr(0x128));
    }
    screen.at<std::uint8_t>(0x117) = 0;
    screen.at<std::uint32_t>(0x20) = 0;
}

namespace {

constexpr float kLeaderboardAutoScrollSpeed = 0.004f;  // 00CEC178
constexpr std::int32_t kLeaderboardPage = 0x11;

// 00565211..0056524D / 0056538F..005653D1: focus back from the entries to the boards list.
void leaderboards_focus_boards(Raw screen, Fe8LeaderboardInputHost& host) {
    void* const entries = screen.ptr(0x11C);
    screen.at<std::int32_t>(0x148) = host.list_box_selected_index_00a9c920(entries);
    host.list_box_clear_00a9cdc0(entries);
    host.widget_set_enabled_60(screen.ptr(0x94), 1);
    host.list_box_select_index_00a9c7c0(screen.ptr(0x94), screen.at<std::int32_t>(0xB8));
    screen.at<std::uint8_t>(0x8D) = 1;
    screen.at<std::uint8_t>(0x115) = 0;
}

bool list_active(void* list_box) { return raw(list_box).at<std::uint8_t>(0x85) != 0; }

// The first byte of a widget label (+0F0h data, "" at 00E19474 when null).
std::uint8_t label_lead(void* widget) {
    const auto* const text = static_cast<const std::uint8_t*>(raw(widget).ptr(0xF0));
    return text != nullptr ? text[0] : 0;
}

void show_gamer_card(Raw screen, Fe8LeaderboardInputHost& host) {
    const Raw row = host.result_row_004bcd20(screen, host.list_box_selected_index_00a9c920(screen.ptr(0x11C)));
    host.xshow_gamer_card_ui_00a4d440(host.xenon_active_user_slot_00a3eac0(), row.at<std::uint64_t>(0x58));
}

} // namespace

void leaderboards_listener_activate_00565190(Raw listener, void* widget, Fe8LeaderboardInputHost& host) {
    const Raw screen{listener.p - 0x0C};
    screen.at<std::uint8_t>(0x158) = 1;
    if (host.widget_type_5c(widget) == 2) {
        if (raw(widget).at<std::uint32_t>(0xD8) != 0) {
            if (list_active(screen.ptr(0x94))) host.leaderboards_sync_selection_00561ad0(screen, 0);
            host.list_box_select_row_00a9c740(screen.ptr(0x11C), widget);
            return;
        }
        if (list_active(screen.ptr(0x11C)) && screen.at<std::int32_t>(0x154) == 0 && !list_active(screen.ptr(0x94))) {
            leaderboards_focus_boards(screen, host);
        }
        host.list_box_select_row_00a9c740(screen.ptr(0x94), widget);
        return;
    }
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, 1.0f);
        if (widget != screen.ptr(0x134) && label_lead(widget) != 0xA7) {
            if (label_lead(widget) == 0xA5) {
                if (!list_active(screen.ptr(0x11C))) return;
                if (host.list_box_selected_index_00a9c920(screen.ptr(0x11C)) == -1) return;
                show_gamer_card(screen, host);
                return;
            }
            if (label_lead(widget) == 0xA3) host.leaderboards_return_to_menu_005629a0(screen);
            return;
        }
        if (screen.at<std::int32_t>(0x154) != 0) return;
        host.leaderboards_cycle_filter_005650c0(screen);
        return;
    }
    void* const boards_scroller = screen.p + 0x34;
    void* const entries_scroller = screen.p + 0xBC;
    if (widget == screen.ptr(0xA4)) {
        host.scroller_set_auto_scroll_00683300(boards_scroller, 2, kLeaderboardAutoScrollSpeed, 1);
        if (screen.at<std::int32_t>(0x154) == 0 && !list_active(screen.ptr(0x94))) leaderboards_focus_boards(screen, host);
    } else if (widget == screen.ptr(0xA0)) {
        host.scroller_set_auto_scroll_00683300(boards_scroller, 1, kLeaderboardAutoScrollSpeed, 1);
        host.leaderboards_sync_selection_00561ad0(screen, 1);
    } else if (widget == screen.ptr(0xAC)) {
        host.scroller_begin_thumb_drag_00683a40(boards_scroller);
        host.leaderboards_sync_selection_00561ad0(screen, 1);
    }
    std::int32_t& first = screen.at<std::int32_t>(0x2C);
    if (widget == screen.ptr(0x128)) {
        if (host.leaderboard_mode_00e19430() != 0) {
            host.scroller_set_auto_scroll_00683300(entries_scroller, 2, kLeaderboardAutoScrollSpeed, 1);
        } else if (list_active(screen.ptr(0x11C)) && first + kLeaderboardPage < screen.at<std::int32_t>(0x14C)) {
            first += kLeaderboardPage;
            screen.at<std::int32_t>(0x154) = 1;
            screen.at<std::int32_t>(0x164) = 2;
            screen.at<std::uint8_t>(0x168) = 1;
        } else {
            screen.at<std::uint8_t>(0x168) = 0;
        }
    } else if (widget == screen.ptr(0x124)) {
        if (host.leaderboard_mode_00e19430() != 0) {
            host.scroller_set_auto_scroll_00683300(entries_scroller, 1, kLeaderboardAutoScrollSpeed, 1);
        } else if (list_active(screen.ptr(0x11C)) && first - kLeaderboardPage > 0) {
            first -= kLeaderboardPage;
            screen.at<std::int32_t>(0x154) = 1;
            screen.at<std::int32_t>(0x164) = 1;
            screen.at<std::uint8_t>(0x168) = 1;
        } else {
            screen.at<std::uint8_t>(0x168) = 0;
        }
    } else if (widget == screen.ptr(0x130)) {
        host.scroller_begin_thumb_drag_00683a40(entries_scroller);
    }
    if (host.widget_type_5c(widget) == 6) {
        host.widget_set_highlight_88(widget, 2, 0, 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 0x12) host.widget_set_highlight_84(widget, 2);
}

void leaderboards_update_00565540(Raw screen, float dt, Fe8LeaderboardUpdateHost& host) {
    if (host.front_end_action_edge_004d92b0(0x4B) || screen.at<std::uint8_t>(0x160) != 0) {
        host.leaderboards_return_to_menu_005629a0(screen);
        return;
    }
    void* const boards_scroller = screen.p + 0x34;
    void* const entries_scroller = screen.p + 0xBC;
    host.scroller_update_00683820(boards_scroller, 1);
    host.scroller_update_00683820(entries_scroller, 1);
    if (screen.at<std::uint8_t>(0x7C) != 0 || screen.at<std::int32_t>(0x54) != 0) {
        const std::int32_t board = screen.at<std::int32_t>(0xB8);
        host.leaderboards_move_selection_00561860(screen, boards_scroller, screen.ptr(0x94), screen.ptr(0x90), false);
        if (screen.at<std::int32_t>(0xB8) != board) host.leaderboards_show_board_name_00561660(screen);
    }
    if (screen.at<std::uint8_t>(0x104) != 0 || screen.at<std::int32_t>(0xDC) != 0) {
        host.leaderboards_move_selection_00561860(screen, entries_scroller, screen.ptr(0x11C), screen.ptr(0x118), false);
    }
    void* const entries = screen.ptr(0x11C);
    std::int32_t& first = screen.at<std::int32_t>(0x2C);
    if (host.front_end_action_edge_004d92b0(0x4D)) {
        if (list_active(screen.ptr(0x94)) && raw(entries).at<std::int32_t>(0x104) > 0)
            host.leaderboards_sync_selection_00561ad0(screen, 0);
    } else if (host.front_end_action_edge_004d92b0(0x4C)) {
        if (list_active(entries)) host.leaderboards_sync_selection_00561ad0(screen, 1);
    } else if (host.front_end_action_edge_004d92b0(0x50)) {
        if (screen.at<std::int32_t>(0x154) == 0) host.leaderboards_cycle_filter_005650c0(screen);
    } else if (host.front_end_action_edge_004d92b0(0x4F)) {
        if (list_active(entries) && host.list_box_selected_index_00a9c920(entries) != -1) show_gamer_card(screen, host);
    } else if (host.front_end_action_edge_004d92b0(0x46)) {
        bool paged = false;
        if (list_active(entries) && first - kLeaderboardPage > 0 && host.leaderboard_mode_00e19430() == 0 &&
            host.list_box_selected_index_00a9c920(entries) == 0) {
            first -= kLeaderboardPage;
            screen.at<std::int32_t>(0x154) = 1;
            screen.at<std::int32_t>(0x164) = 1;
            paged = true;
        }
        screen.at<std::uint8_t>(0x168) = paged ? 1 : 0;
    } else if (host.front_end_action_edge_004d92b0(0x47)) {
        bool paged = false;
        if (list_active(entries) && first + kLeaderboardPage < screen.at<std::int32_t>(0x14C) &&
            host.leaderboard_mode_00e19430() == 0 && host.list_box_selected_index_00a9c920(entries) == 0x10) {
            first += kLeaderboardPage;
            screen.at<std::int32_t>(0x154) = 1;
            screen.at<std::int32_t>(0x164) = 2;
            paged = true;
        }
        screen.at<std::uint8_t>(0x168) = paged ? 1 : 0;
    }
    const std::int32_t pending = screen.at<std::int32_t>(0x154);
    if (pending != 0) {
        float& timer = screen.at<float>(0x150);
        if (!(timer == 0.0f)) {
            timer = static_cast<float>(timer - dt);
            if (timer < 0.0f) {
                timer = 0.0f;
                first = 1;
            }
        }
        if (timer == 0.0f && pending == 1) {
            if (screen.at<std::uint8_t>(0x168) == 0 &&
                screen.at<std::int32_t>(0x20) ==
                    host.board_row_00e19468(static_cast<std::uint32_t>(host.leaderboards_selected_board_00561300(screen)))
                        .at<std::int32_t>(0)) {
                screen.at<std::int32_t>(0x154) = 0;
            } else {
                host.leaderboards_reset_rows_00564e70(screen);
            }
        }
    }
    screen.at<std::uint8_t>(0x168) = 0;
    const bool card = list_active(entries) && raw(entries).at<std::int32_t>(0x104) > 0 &&
                      host.list_box_selected_index_00a9c920(entries) != -1;
    const bool scroll = card && raw(entries).at<std::int32_t>(0x104) > kLeaderboardPage;
    host.rebuild_command_bar_0054b530({{
        {0xA7, "FE.toggle_filter", 0},
        {0xB3, "globals.navigate", 1},
        {static_cast<std::uint8_t>(card ? 0xA5 : 0), card ? "globals.showgamercard" : "", 1},
        {static_cast<std::uint8_t>(scroll ? 0xB2 : 0), scroll ? "globals.scroll_menu" : "", 1},
        {0xA3, "globals.back", 2},
    }});
    if (host.xenon_duplicate_sign_in_128()) {
        host.raise_prompt_00531b00(2, "FE_pc.multi_liveerrorduplicate", 2);
        screen.at<std::uint8_t>(0x160) = 1;
        return;
    }
    if (!host.main_menu_connection_gate_00585b40() || !host.main_menu_account_gate_00585810(0)) {
        screen.at<std::uint8_t>(0x160) = 1;
        return;
    }
    if (screen.at<std::int32_t>(0x20) == 0) return;
    const std::int32_t result = host.game_complete_leaderboard_read_004dd7c0();
    constexpr std::int32_t kEFail = static_cast<std::int32_t>(0x80004005u);
    constexpr std::int32_t kEUnexpected = static_cast<std::int32_t>(0x8000FFFFu);
    if (result == kEFail || result == kEUnexpected) {
        host.please_wait_screen_00e19698().at<std::uint8_t>(4) = 0;
        if (result == kEFail) {
            host.raise_prompt_00531b00(2, "FE.error_while_reading_leaderboard", 2);
            host.game_cancel_leaderboard_read_004bc7f0();
            screen.at<std::int32_t>(0x20) = 0;
        }
    }
    float& wait = screen.at<float>(0x15C);
    wait = static_cast<float>(wait + dt);
    if (host.please_wait_screen_00e19698().at<std::uint8_t>(4) != 0 && static_cast<double>(wait) > 30.0) {  // 00CE7630
        host.raise_prompt_00531b00(2, "FE.leaderboards_cantaccess", 2);
        host.please_wait_screen_00e19698().at<std::uint8_t>(4) = 0;
        host.game_cancel_leaderboard_read_004bc7f0();
        screen.at<std::int32_t>(0x20) = 0;
    }
}

namespace {

// The settings' device key for the screen's device index (+16Ch), and its binding tree.
const void* binding_device_key(Raw settings, Raw screen) {
    return static_cast<std::uint8_t*>(settings.ptr(0x18)) + screen.at<std::uint32_t>(0x16C) * 8;
}

} // namespace

void control_binding_show_conflict_text_0055c200(Raw screen, Fe8ControlBindingHost& host) {
    const Raw settings = host.input_settings_005547d0();
    const void* const key = binding_device_key(settings, screen);
    void* const entry = host.input_device_map_index_0055c110(settings.p + 8, key);
    void* const end = raw(entry).ptr(4);
    const std::string_view name = native_view(screen, 0x184);
    if (host.binding_tree_find_00555940(entry, name) != end) {
        host.widget_set_visible_34(screen.ptr(0x34), 0);
        return;
    }
    std::string conflict;
    const bool found =
        host.find_input_binding_conflict_006aac50(settings, key, name, screen.at<std::uint32_t>(0x180), conflict);
    host.widget_set_visible_34(screen.ptr(0x34), found ? 1u : 0u);
    if (found) {
        host.gui_text_set_localised_source_00abaed0(screen.ptr(0x34),
                                                    std::string("FE_pc.controls_conflict|.: |") + conflict, 1);
    }
}

void control_binding_refresh_selected_binding_0055c3b0(Raw screen, Fe8ControlBindingHost& host) {
    if (host.list_box_selected_index_00a9c920(screen.ptr(0x20)) < 0) return;
    const Raw settings = host.input_settings_005547d0();
    void* const entry = host.input_device_map_index_0055c110(settings.p + 8, binding_device_key(settings, screen));
    void* const main_text = host.find_child_by_name_00aa7e00(host.list_box_current_row_00425e50(screen.ptr(0x20)),
                                                             "main_Text");
    // The image copies the text's NativeString (+0F4h) into +184h; the host owns the storage.
    host.native_string_copy_0041dd40(screen.p + 0x184, native_view(raw(main_text), 0xF4));
    void* const end = raw(entry).ptr(4);
    const bool unknown = host.binding_tree_find_00555940(entry, native_view(screen, 0x184)) == end;
    screen.at<std::uint8_t>(0x18C) = unknown ? 1 : 0;
    void* const row = host.list_box_current_row_00425e50(screen.ptr(0x20));
    if (unknown) {
        screen.at<std::uint32_t>(0x180) = 1;
        screen.ptr(0x190) = host.find_child_by_name_00aa7e00(row, "main_Text");
    } else {
        screen.ptr(0x190) =
            host.find_child_by_name_00aa7e00(row, screen.at<std::uint32_t>(0x180) != 0 ? "var_2_Text" : "var_1_Text");
    }
    host.control_binding_show_conflict_text_0055c200(screen);
}

bool control_binding_scroll_to_selection_0055cec0(Raw screen, bool no_scroll, Fe8ControlBindingHost& host) {
    void* const list = screen.ptr(0x20);
    std::int32_t& first = screen.at<std::int32_t>(0x21C);
    std::int32_t& last = screen.at<std::int32_t>(0x220);
    const std::int32_t count = raw(list).at<std::int32_t>(0x104);
    first = count;
    last = 0;
    void* const clip = screen.ptr(0x218);
    const std::array<float, 3> clip_pos = host.widget_resolved_position_00aa6750(clip);
    const std::array<float, 2> clip_size = host.widget_size_00aa6740(clip);
    const float clip_y = static_cast<float>(clip_pos[1] - clip_size[1] * raw(clip).at<float>(0x1C));
    for (std::int32_t i = 0; i < count; ++i) {
        void* const row = host.list_box_row_00a9be00(list, i);
        const std::array<float, 3> pos = host.widget_resolved_position_00aa6750(row);
        const std::array<float, 2> size = host.widget_size_00aa6740(row);
        const float row_y = static_cast<float>(pos[1] - size[1] * raw(row).at<float>(0x1C));
        // Both sums stay in extended precision; double is exact for float + float.
        const bool visible =
            row_y > clip_y && static_cast<double>(clip_y) + clip_size[1] > static_cast<double>(row_y) + size[1];
        if (visible) {
            if (i < first) first = i;
            else if (last < i) last = i;
        }
        host.widget_set_enabled_60(row, visible ? 1u : 0u);
    }
    void* const scroller = screen.p + 0x1A0;
    if (screen.at<std::uint8_t>(0x1E8) == 0 && screen.at<std::int32_t>(0x1C0) == 0 && !no_scroll) {
        if (host.list_box_selected_index_00a9c920(list) != -1 && host.list_box_selected_index_00a9c920(list) < first) {
            host.scroller_toward_start_00683680(
                scroller, host.widget_size_00aa6740(host.list_box_current_row_00425e50(list))[1]);
            return true;
        }
        if (host.list_box_selected_index_00a9c920(list) == -1) return false;
        if (host.list_box_selected_index_00a9c920(list) <= last) return false;
        host.scroller_toward_end_00683700(scroller,
                                          host.widget_size_00aa6740(host.list_box_current_row_00425e50(list))[1]);
        return true;
    }
    std::int32_t target = 0;
    if (host.list_box_selected_index_00a9c920(list) != -1 && host.list_box_selected_index_00a9c920(list) < first) {
        target = first;
        while (raw(host.list_box_row_00a9be00(list, target)).at<std::uint8_t>(0x77) != 0) ++target;
    } else {
        if (host.list_box_selected_index_00a9c920(list) == -1) return false;
        target = last;
        if (host.list_box_selected_index_00a9c920(list) <= target) return false;
        while (raw(host.list_box_row_00a9be00(list, target)).at<std::uint8_t>(0x77) != 0) --target;
    }
    if (target == -1) return false;
    host.list_box_select_index_silently_00a9c8f0(list, target);
    host.control_binding_refresh_selected_binding_0055c3b0(screen);
    screen.at<std::int32_t>(0x224) = target;
    return true;
}

void control_binding_on_selection_changed_0055d180(Raw listener, void* row, void* list_box,
                                                   Fe8ControlBindingHost& host) {
    if (row == nullptr) return;
    const Raw screen{listener.p - 8};
    const std::int32_t page = screen.at<std::int32_t>(0x168);
    screen.at<std::int32_t>(0x16C + page * 4) = host.list_box_selected_index_00a9c920(list_box);
    if (page == 0) {
        host.command_bar_set_help_line_0054a0c0(std::string(native_view(raw(row), 0xF4)) + "_help");
    } else if (page == 1) {
        host.command_bar_set_help_line_0054a0c0("FE_pc.cs_loadcontrollerpreset_help");
    } else if (page == 2 && list_box == screen.ptr(0x20)) {
        host.control_binding_refresh_selected_binding_0055c3b0(screen);
        host.widget_set_enabled_60(list_box, 1);
        while (host.control_binding_scroll_to_selection_0055cec0(screen, false)) {
        }
    }
}

void control_binding_enter_0055fa80(Raw screen, Fe8ControlBindingEnterHost& host) {
    const bool in_mission = host.game_in_mission_61f();
    screen.ptr(0x1C) = host.find_child_by_name_00aa7e00(screen.ptr(0x14), in_mission ? "Ingame_Main_Listbox" : "Main_Listbox");
    host.widget_set_visible_34(
        host.find_child_by_name_00aa7e00(screen.ptr(0x14), in_mission ? "Main_Listbox" : "Ingame_Main_Listbox"), 0);
    screen.ptr(0x24) = host.find_child_by_name_00aa7e00(screen.ptr(0x1C), "MainListbox_Text");
    host.list_box_set_listener_00a9ac40(screen.ptr(0x1C), screen.p + 8);
    host.widget_set_visible_34(screen.ptr(0x1C), 1);
    host.control_binding_refresh_page_0055e3a0(screen, 0);
    screen.at<float>(0x10) = 0.25f;  // 00CE3868
    screen.at<std::uint8_t>(0xE4) = 0;
    screen.at<std::uint8_t>(0x38) = 1;
    const std::uint32_t length = screen.at<std::uint32_t>(0x3C);
    if (length != 0) {
        const std::uint32_t n = length < 0x7FFFFFFFu ? length : 0x7FFFFFFFu;
        auto* const data = static_cast<char*>(screen.ptr(0x40));
        std::memmove(data, data + n, length - n);
        host.native_string_resize_0041dd40(screen.p + 0x3C, length - n, 1);
    }
}

void control_binding_confirm_or_next_device_00560210(Raw screen, Fe8ControlBindingConfirmHost& host) {
    if (screen.at<std::uint8_t>(0x19C) != 0) {
        host.prompt_append_label_00450540("FE.opt_cancel");
        host.prompt_append_label_00450540("FE.opt_discard");
        host.prompt_append_label_00450540("globals.accept");
        host.prompt_screen_00425d10().at<std::uint32_t>(0x284) = 0;
        host.raise_prompt_00531b00(0, "FE.opt_acceptchanges", 4, 0x0055FA10);
        return;
    }
    host.input_settings_save_006aa2d0();
    screen.at<std::int32_t>(0x16C) += 1;
    host.control_binding_refresh_page_0055e3a0(screen, 0);
}

namespace {

// One library list row: clone +0ACh, find "MenuItem_Text", name the row with its index, clear
// row +77h, label (and show) the text, append the row to +0B8h.
void library_add_row(Raw screen, std::int32_t index, std::string_view label, bool show_before_label,
                     Fe8LibraryListHost& host) {
    void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0xAC), 0);
    void* const text = host.find_child_by_name_00aa7e00(row, "MenuItem_Text");
    host.widget_set_name_2c(row, std::to_string(index));
    raw(row).at<std::uint8_t>(0x77) = 0;
    if (show_before_label) host.widget_set_visible_34(text, 1);
    host.gui_text_set_ellipsized_00abb000(text, label, kTextWidthUnbounded, 1);
    if (!show_before_label) host.widget_set_visible_34(text, 1);
    host.list_box_insert_row_00a9d750(screen.ptr(0xB8), row, 0, 0);
}

// The image copies each label with strcpy_s into a 200-byte buffer (an over-long label raises the
// CRT invalid-parameter handler); the model passes the label through.
std::string_view library_label(const char* text) { return text != nullptr ? std::string_view(text) : std::string_view(); }

const char* library_table_label(Raw screen, std::size_t offset, std::int32_t i) {
    return static_cast<const char*>(screen.ptr(offset + static_cast<std::size_t>(i) * 8));
}

} // namespace

void library_build_unlocks_list_005001a0(Raw screen, Fe8LibraryListHost& host) {
    host.widget_set_visible_34(screen.ptr(0xB8), 1);
    // Record 0 of the 2Ch-record vector at +46Ch (begin +470h); its id vector is +1Ch..+20h.
    const Raw record{static_cast<std::uint8_t*>(screen.ptr(0x470))};
    const auto* const ids = static_cast<const std::int32_t*>(record.ptr(0x1C));
    const std::uint32_t id_count =
        ids == nullptr ? 0u
                       : static_cast<std::uint32_t>(static_cast<const std::int32_t*>(record.ptr(0x20)) - ids);
    std::string label;
    const std::uint32_t index = screen.at<std::uint32_t>(0x458);
    if (index < id_count && ids[index] == screen.at<std::int32_t>(0x454)) {
        screen.at<std::uint32_t>(0x138) = index;
        label = host.vehicle_class_name_or_no_unlock_004fda50(ids[index]);
    } else {
        screen.at<std::uint32_t>(0x138) = 0;
        screen.at<std::int32_t>(0x454) = -1;
        screen.at<std::uint32_t>(0x458) = 0;
        label = "FE.unitlib_nounlock";
    }
    library_add_row(screen, 0, label, false, host);
    host.list_box_reset_highlight_00a9cf30(screen.ptr(0xB8), 1, 1);
}

void library_build_movie_list_00500450(Raw screen, Fe8LibraryListHost& host) {
    host.widget_set_visible_34(screen.ptr(0xB8), 1);
    library_add_row(screen, 0, library_label(host.image_string_table(0x00E08494, 0)), false, host);
    host.list_box_reset_highlight_00a9cf30(screen.ptr(0xB8), 1, 1);
}

void library_build_nose_art_list_00500640(Raw screen, Fe8LibraryListHost& host) {
    host.widget_set_visible_34(screen.ptr(0xB8), 1);
    for (std::int32_t i = 0; i < 2; ++i) {
        const std::int32_t unlocked = screen.at<std::int32_t>(0x98);
        if (unlocked < 0x5A && unlocked != i) continue;
        library_add_row(screen, i, library_label(host.image_string_table(0x00E084F0, i)), false, host);
    }
    host.list_box_reset_highlight_00a9cf30(screen.ptr(0xB8), 1, 1);
}

void library_build_units_weapons_list_00500860(Raw screen, Fe8LibraryListHost& host) {
    host.widget_set_visible_34(screen.ptr(0xB8), 1);
    for (std::int32_t i = 0; i < 5; ++i)
        library_add_row(screen, i, library_label(library_table_label(screen, 0x31C, i)), true, host);
    host.list_box_reset_highlight_00a9cf30(screen.ptr(0xB8), 1, 1);
}

void library_build_power_up_list_00500e90(Raw screen, Fe8LibraryListHost& host) {
    host.widget_set_visible_34(screen.ptr(0xB8), 1);
    for (std::int32_t i = 0; i < 3; ++i)
        library_add_row(screen, i, library_label(library_table_label(screen, 0x344, i)), false, host);
    host.list_box_reset_highlight_00a9cf30(screen.ptr(0xB8), 1, 1);
}

void library_build_academy_list_005010c0(Raw screen, Fe8LibraryListHost& host) {
    host.widget_set_visible_34(screen.ptr(0xB8), 1);
    for (std::int32_t i = 0; i < 0x0D; ++i) {
        if (host.library_has_academy_entry_004fae70(screen, i)) continue;
        library_add_row(screen, i, library_label(library_table_label(screen, 0x35C, i)), false, host);
    }
    host.list_box_reset_highlight_00a9cf30(screen.ptr(0xB8), 1, 1);
}

void library_build_achievements_list_005012f0(Raw screen, Fe8LibraryListHost& host) {
    host.widget_set_visible_34(screen.ptr(0xB8), 1);
    for (std::int32_t i = 0; i < 6; ++i)
        library_add_row(screen, i, library_label(host.image_string_table(0x00E084D8, i)), false, host);
    host.list_box_reset_highlight_00a9cf30(screen.ptr(0xB8), 1, 1);
}

const float* library_colour_static_grey_004f9d80(Fe8LibrarySmallHost& host) {
    float* const colour = host.static_grey_00e18d64();
    std::uint32_t& guard = host.static_grey_guard_00e18d74();
    if ((guard & 1) == 0) {
        guard |= 1;
        colour[0] = 0.25f;  // 00CE3868
        colour[1] = 0.25f;
        colour[2] = 0.25f;
        colour[3] = 1.0f;   // 00D7A24C
    }
    return colour;
}

namespace {

void vehicle_class_identity_order(Raw registry) {
    for (std::uint32_t i = 0; i < 0x800; ++i) {
        registry.at<std::uint32_t>(0x10 + i * 4) = i;
        registry.at<std::uint32_t>(0x2010 + i * 4) = i;
    }
}

} // namespace

void library_reset_vehicle_class_order_00506510(Raw screen, Fe8LibrarySmallHost& host) {
    screen.at<std::uint32_t>(0x134) = 0;
    vehicle_class_identity_order(host.vehicle_class_registry_00437f50());
    host.library_reset_order_state_004f8fa0(screen);
}

void library_show_current_entry_detail_00506550(Raw screen, Fe8LibrarySmallHost& host) {
    const auto* const entries = static_cast<const std::uint8_t*>(screen.ptr(0x448));
    screen.at<std::int32_t>(0x4B0) =
        *reinterpret_cast<const std::int32_t*>(entries + screen.at<std::uint32_t>(0x434) * 0x6C + 0x48);
    const Raw name = host.widget_name_28(host.list_box_current_item_00a9ca00(screen.ptr(0xB8)));
    const char* const text = static_cast<const char*>(name.ptr(4));
    const std::int32_t index = host.atol_00bf8417(text != nullptr ? text : "");
    const auto* const records = static_cast<const std::uint8_t*>(screen.ptr(0x460));
    const std::int32_t record = *reinterpret_cast<const std::int32_t*>(records + index * 0x2C);
    const std::int32_t class_id = screen.at<std::int32_t>(0x4B0);
    if (class_id == screen.at<std::int32_t>(0x134)) return;
    host.library_refresh_unlock_rows_00503250(screen, class_id);
    screen.at<std::int32_t>(0x134) = class_id;
    vehicle_class_identity_order(host.vehicle_class_registry_00437f50());
    const Raw registry = host.vehicle_class_registry_00437f50();
    registry.at<std::int32_t>(0x10 + record * 4) = class_id;
    registry.at<std::int32_t>(0x2010 + class_id * 4) = record;
}

std::int32_t session_list_absolute_selection_005661c0(void* list_box, Fe8LibrarySmallHost& host) {
    const std::int32_t offset = raw(list_box).at<std::uint8_t>(0x148) != 0 ? raw(list_box).at<std::int32_t>(0x164) : 0;
    return host.list_box_selected_index_00a9c920(list_box) + offset;
}

void gui_icon_set_uv_rect_00568290(Raw icon, const float uv[4], Fe8LibrarySmallHost& host) {
    for (int i = 0; i < 4; ++i) icon.at<float>(0x118 + i * 4) = uv[i];
    host.widget_apply_uv_8c(icon.p);
}

namespace {

// atol of the current library item's name (vtable +28h string data, "" 00E18D78 when null).
std::int32_t library_current_item_index(void* item, Fe8LibrarySmallHost& host) {
    const char* const text = static_cast<const char*>(host.widget_name_28(item).ptr(4));
    return host.atol_00bf8417(text != nullptr ? text : "");
}

} // namespace

void library_show_unlocks_page_004ffd00(Raw screen, Fe8LibraryPageHost& host) {
    if (screen.at<std::int32_t>(0x94) != 6) return;
    void* const item = host.list_box_current_item_00a9ca00(screen.ptr(0xB8));
    if (item == nullptr) return;
    library_current_item_index(item, host);  // result unused in the image
    const char* const view = screen.at<std::int32_t>(0x90) == 1 ? "globals.2dview" : "globals.3dview";
    host.rebuild_command_bar_0054b530({{
        {0xA2, "globals.select", 0},
        {0xA7, view, 1},
        {0xB2, "globals.scroll_menu", 1},
        {0xAF, "globals.change", 1},
        {0xA3, "globals.back", 2},
    }});
    host.frame_select_00518d60(0xFFFFFFFFu, 0, 0, "FE.taclib_unlocks_title");
}

void library_show_order_of_battle_page_004fff20(Raw screen, Fe8LibraryPageHost& host) {
    if (screen.at<std::int32_t>(0x94) != 5) return;
    void* const item = host.list_box_current_item_00a9ca00(screen.ptr(0xB8));
    if (item == nullptr) return;
    const std::int32_t index = library_current_item_index(item, host);
    const Raw record{static_cast<std::uint8_t*>(screen.ptr(0x460)) + index * 0x2C};
    const auto* const ids = static_cast<const std::int32_t*>(record.ptr(0x1C));
    const std::int32_t id_count =
        ids == nullptr ? 0 : static_cast<std::int32_t>(static_cast<const std::int32_t*>(record.ptr(0x20)) - ids);
    const bool change = id_count > 0 && record.at<std::uint8_t>(0x0C) != 0;
    const char* const view = screen.at<std::int32_t>(0x90) == 1 ? "globals.2dview" : "globals.3dview";
    host.rebuild_command_bar_0054b530({{
        {0xA2, "globals.continue", 0},
        {0xA7, view, 1},
        {0xB3, "globals.navigate", 1},
        {static_cast<std::uint8_t>(change ? 0xAF : 0), change ? "globals.change" : "", 1},
        {0xA3, "globals.back", 2},
    }});
    host.frame_select_00518d60(0xFFFFFFFFu, 0, 0, "FE.taclib_orderofbattle_title");
}

void library_build_map_entries_00509de0(Raw screen, Fe8LibraryMapHost& host) {
    for (std::uint32_t i = 0;; ++i) {
        auto* const begin = static_cast<std::uint8_t*>(screen.ptr(0x3D8));
        auto* const end = static_cast<std::uint8_t*>(screen.ptr(0x3DC));
        if (begin == nullptr || i >= static_cast<std::uint32_t>((end - begin) / 0x6C)) return;
        const Raw entry{begin + i * 0x6C};
        const Raw game = host.game();
        void* const map = game.p + 0x6F0;
        void* const head = game.ptr(0x6F4);
        const Raw key{entry.p + 0x28};
        void* const node = host.string_int_map_lower_bound_004bf370(map, key.p);
        bool found = false;
        if (node != head) {
            const Raw n{static_cast<std::uint8_t*>(node)};
            std::int32_t order = 0;  // key compared with the node key (+0Ch length, +10h data)
            if (key.at<std::uint32_t>(0) == 0) order = n.at<std::uint32_t>(0x0C) != 0 ? -1 : 0;
            else if (n.at<std::uint32_t>(0x0C) != 0)
                order = host.stricmp_00bf7fbf(static_cast<const char*>(key.ptr(4)), static_cast<const char*>(n.ptr(0x10)));
            found = order >= 0;
        }
        const bool unlocked = found && host.string_int_map_index_005070c0(map, key.p) > 0;
        entry.at<std::uint8_t>(0x45) = unlocked ? 0 : 1;
    }
}

void library_filter_entries_0050d900(Raw screen, void* source, void* out, Fe8LibraryFilterHost& host) {
    host.entry_vector_clear_00505de0(out);
    for (std::uint32_t i = 0; i < host.entry_vector_size_004f92e0(source); ++i) {
        const std::int32_t side = host.entry_vector_at_005056c0(source, i).at<std::int32_t>(0x40);
        if ((side == screen.at<std::int32_t>(0x3C0) || host.entry_vector_at_005056c0(source, i).at<std::int32_t>(0x40) > 0x59) &&
            host.entry_vector_at_005056c0(source, i).at<std::uint8_t>(0x44) != 0) {
            host.entry_vector_push_back_0050ad00(out, host.entry_vector_at_005056c0(source, i));
        }
    }
    switch (screen.at<std::int32_t>(0x94)) {
    case 4:
        for (const std::size_t movie : {0x51Cu, 0x520u, 0x524u})
            if (screen.ptr(movie) != nullptr) host.gui_movie_stop_00aac870(screen.ptr(movie));
        screen.at<std::int32_t>(0x558) = -1;
        screen.at<std::int32_t>(0x55C) = -1;
        break;
    case 5:
        host.library_view5_reset_0050b730(screen);
        break;
    case 6:
        host.library_view6_reset_0050b440(screen);
        break;
    default:
        break;
    }
}

void front_end_frame_register_00517ed0(Raw screen, Fe8FrameRegisterHost& host) {
    host.front_end_screen_register_004f71d0(screen);
    screen.at<std::int32_t>(0x160) = 4;
    struct Table {
        std::size_t base;
        std::uint32_t names;
        std::int32_t columns;
    };
    for (const Table table : {Table{0x0C, 0x00E08500, 13}, Table{0x110, 0x00E08604, 2}, Table{0x138, 0x00E0862C, 2}}) {
        for (std::int32_t j = 0; j < table.columns; ++j) {
            const std::int32_t index = screen.at<std::int32_t>(0x160);
            const char* const name = host.image_string_table(table.names, index * table.columns + j);
            for (std::int32_t row = 0; row < 5; ++row) {
                void* page = nullptr;
                if (row == screen.at<std::int32_t>(0x160) && name != nullptr) page = host.gui_load_page_00aa5840(name, 1, 0);
                screen.ptr(table.base + static_cast<std::size_t>(row * table.columns + j) * 4) = page;
            }
        }
    }
}

namespace {

std::uint8_t library_label_lead(void* widget) {
    const auto* const text = static_cast<const std::uint8_t*>(raw(widget).ptr(0xF0));
    return text != nullptr ? text[0] : 0;  // "" at 00E18D7C
}

} // namespace

void library_handle_gui_event_00517300(Raw listener, void* widget, Fe8LibraryEventHost& host) {
    const Raw screen{listener.p - 0x68};
    float& timer = screen.at<float>(0x5B4);
    const std::int32_t kind = screen.at<std::int32_t>(0x94);
    if (host.widget_type_5c(widget) == 3) {
        void* const list = screen.ptr(0xC4);
        if ((widget == host.list_box_row_00a9be00(list, 0) || widget == host.list_box_row_00a9be00(list, 1) ||
             widget == host.list_box_row_00a9be00(list, 2)) &&
            timer == kTextWidthUnbounded) {  // -1.0f, 00D7A260
            std::int32_t row = 0;
            if (widget == host.list_box_row_00a9be00(list, 1)) row = 1;
            else if (widget == host.list_box_row_00a9be00(list, 2)) row = 2;
            host.list_box_select_index_00a9c7c0(list, row);
            timer = 0.5f;  // 00CE3800
            return;
        }
        host.widget_set_alpha_4c(widget, 1.0f);
        const std::uint8_t lead = library_label_lead(widget);
        if (lead == 0xA2) {
            switch (kind) {
            case 3: host.library_play_entry_movie_004ff580(screen); return;
            case 5: host.front_end_show_difficulty_select_004ff6f0(screen); return;
            case 6:
                if (host.library_is_entry_unlocked_004fc9d0(screen)) host.library_back_00515e00(screen);
                return;
            case 7:
                if (timer == kTextWidthUnbounded) timer = 0.5f;
                return;
            case 8: host.library_apply_nose_art_005161c0(screen); return;
            default: return;
            }
        }
        if (lead == 0xA7) {
            if (kind == 2 || (kind > 4 && kind < 7)) host.library_switch_view_00514c20(screen);
            return;
        }
        if (lead == 0xA3) {
            if (kind == 7) host.library_open_order_of_battle_view_00514fb0(screen);
            else host.library_back_00515e00(screen);
        }
        return;
    }
    void* const scroller = screen.p + 0xCC;
    if (widget == screen.ptr(0x5BC)) {
        host.scroller_set_auto_scroll_00683300(scroller, 2, kLeaderboardAutoScrollSpeed, 1);
    } else if (widget == screen.ptr(0x5B8)) {
        host.scroller_set_auto_scroll_00683300(scroller, 1, kLeaderboardAutoScrollSpeed, 1);
    } else if (widget == screen.ptr(0x5C0)) {
        host.scroller_begin_thumb_drag_00683a40(scroller);
    } else if (widget == screen.ptr(0x574) || widget == screen.ptr(0x578)) {
        if (host.widget_state_94(widget) == 3) return;
        host.library_step_carousel_00516010(screen, widget == screen.ptr(0x574));
    } else {
        for (std::int32_t i = 0; i < 0x28; ++i) {
            if (widget != screen.ptr(0x1D8 + i * 4) && widget != screen.ptr(0x278 + i * 4)) continue;
            const bool first_table = widget == screen.ptr(0x1D8 + i * 4);
            if (kind == 5)
                host.list_box_select_index_00a9c7c0(screen.ptr(0xB8), host.library_find_row_by_id_004fcb30(screen, i));
            if (host.widget_type_5c(widget) != 6) return;
            if (host.widget_state_94(widget) >= 3) return;
            host.library_step_carousel_00516010(screen, first_table);
            break;
        }
    }
    if (host.widget_type_5c(widget) == 6) {
        host.widget_set_highlight_88(widget, 2, 0, 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 0x12) host.widget_set_highlight_84(widget, 2);
}

void in_game_screen23_bind_unit_005178f0(Raw screen, void* unit, Fe8InGame23Host& host) {
    void* const observer = screen.p + 8;
    if (screen.ptr(0x1C) != nullptr) host.unit_send_role_transfer_0077c470(screen.ptr(0x1C), 0x1FE, 0);
    if (screen.ptr(0x1C) != nullptr && host.observer_is_pair_registered_00694af0(screen.ptr(0x1C), observer))
        host.observer_unregister_pair_006952a0(screen.ptr(0x1C), observer);
    screen.ptr(0x1C) = unit;
    if (unit != nullptr && !host.observer_is_pair_registered_00694af0(unit, observer))
        host.observer_register_pair_00694a60(screen.ptr(0x1C), observer);
    if (screen.ptr(0x20) == nullptr) {
        void* camera = host.operator_new_00bf55be(0x388);
        if (camera != nullptr) {
            std::memset(camera, 0, 0x388);
            host.camera_mover_construct_base_00432750(camera);
            const Raw c = raw(camera);
            c.at<std::uint32_t>(0x000) = 0x00CEC1F8;
            c.at<std::uint32_t>(0x010) = 0x00CEC1DC;
            c.at<std::uint32_t>(0x024) = 0x00CEC1D4;
            c.at<std::uint32_t>(0x170) = 0x00CEC1BC;
        }
        screen.ptr(0x20) = camera;
        static constexpr float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        host.camera_set_transform_00923870(camera, 0, host.game().at<std::uint32_t>(0x19CC), kIdentity);
        host.observer_register_pair_00694a60(screen.ptr(0x20), observer);
    }
    void* const unit_part = screen.ptr(0x1C) != nullptr ? static_cast<std::uint8_t*>(screen.ptr(0x1C)) + 0x72C : nullptr;
    host.airfield_camera_setup_0042cb30(screen.ptr(0x20), unit_part);
    host.game_set_camera_mover_004bc410(screen.ptr(0x20));
    screen.at<float>(0x24) = 0.5f;  // 00CE3800
}

void bomber_view_ensure_camera_005198f0(Raw screen, Fe8BomberCameraHost& host) {
    if (screen.ptr(0x28) != nullptr &&
        (raw(screen.ptr(0x28)).at<std::uint8_t>(0x60) != 0 || raw(screen.ptr(0x28)).at<std::uint8_t>(0x5D) != 0))
        screen.ptr(0x28) = nullptr;
    if (screen.ptr(0x28) == nullptr) {
        void* camera = host.operator_new_00bf55be(0x3F4);
        if (camera != nullptr) {
            std::memset(camera, 0, 0x3F4);
            host.ship_camera_construct_base_00519290(camera);
            const Raw c = raw(camera);
            c.at<std::uint32_t>(0x000) = 0x00CEC5F0;
            c.at<std::uint32_t>(0x010) = 0x00CEC5D8;
            c.at<std::uint32_t>(0x024) = 0x00CEC5D0;
            c.at<std::uint32_t>(0x170) = 0x00CEC5B8;
        }
        screen.ptr(0x28) = camera;
        static constexpr float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        host.camera_set_transform_00923870(camera, 0, host.game().at<std::uint32_t>(0x19CC), kIdentity);
        raw(screen.ptr(0x28)).at<float>(0x3EC) = -1.5533430576324463f;  // 00CEC574
        raw(screen.ptr(0x28)).at<float>(0x3F0) = 1.5533430576324463f;   // 00CEC400
        host.observer_register_pair_00694a60(screen.ptr(0x28), screen.p + 8);
    }
    void* const camera = screen.ptr(0x28);
    if (host.game().ptr(0x1ED4) != camera || screen.ptr(0x1C) != screen.ptr(0x20)) {
        host.camera_mover_set_target_0042f650(camera, screen.ptr(0x1C));
        const Raw c = raw(screen.ptr(0x28));
        if (c.at<std::uint32_t>(0x3E8) == 0) {
            c.at<float>(0x384) = 0.0f;
            c.at<float>(0x388) = 0.0f;
            for (const std::size_t off : {0x38Cu, 0x398u, 0x394u, 0x390u}) c.at<float>(off) = 30.0f;  // 00CE38C8
        }
        host.game_set_camera_mover_004bc410(screen.ptr(0x28));
        screen.ptr(0x20) = screen.ptr(0x1C);
    }
    void* const binoculars = host.binoculars_screen_00e198c4_4c();
    host.binoculars_bind_unit_0051e730(binoculars, screen.ptr(0x28));
    host.binoculars_reset_0051e8e0(host.binoculars_screen_00e198c4_4c());
}

namespace {

void entry_assign_reference(Raw to, Raw from, std::size_t off, Fe8EntryAssignHost& host) {
    void* const incoming = from.ptr(off);
    void* const old = to.ptr(off);
    if (old == incoming) return;
    to.ptr(off) = incoming;
    if (incoming != nullptr) host.interlocked_increment_00ce221c(&raw(incoming).at<long>(4));
    if (old != nullptr && host.interlocked_decrement_00ce2220(&raw(old).at<long>(4)) == 0) host.object_virtual_0(old);
}

void entry_assign_string(Raw to, Raw from, std::size_t off, Fe8EntryAssignHost& host) {
    if (to.p + off == from.p + off) return;
    const std::uint32_t length = from.at<std::uint32_t>(off);
    host.native_string_resize_0041dd40(to.p + off, length, 1);
    if (length != 0) std::memcpy(to.ptr(off + 4), from.ptr(off + 4), to.at<std::uint32_t>(off));
}

} // namespace

void* library_entry_assign_004fe310(Raw to, Raw from, Fe8EntryAssignHost& host) {
    entry_assign_reference(to, from, 0x00, host);
    for (const std::size_t off : {0x04u, 0x08u, 0x0Cu, 0x10u}) to.at<std::uint32_t>(off) = from.at<std::uint32_t>(off);
    entry_assign_reference(to, from, 0x14, host);
    for (const std::size_t off : {0x18u, 0x1Cu, 0x20u, 0x24u}) to.at<std::uint32_t>(off) = from.at<std::uint32_t>(off);
    for (const std::size_t off : {0x28u, 0x30u, 0x38u}) entry_assign_string(to, from, off, host);
    to.at<std::uint32_t>(0x40) = from.at<std::uint32_t>(0x40);
    for (const std::size_t off : {0x44u, 0x45u, 0x46u}) to.at<std::uint8_t>(off) = from.at<std::uint8_t>(off);
    to.at<std::uint32_t>(0x48) = from.at<std::uint32_t>(0x48);
    for (const std::size_t off : {0x4Cu, 0x54u, 0x5Cu, 0x64u}) entry_assign_string(to, from, off, host);
    return to.p;
}

void library_refresh_unlock_rows_00503250(Raw screen, std::int32_t class_id, Fe8UnlockRowsHost& host) {
    // The image keeps the record pointer in a stack local across rows (uninitialised before the
    // first kind-5/6 row); the model carries it the same way and stops on a row that would read
    // the uninitialised value.
    const std::int32_t* record = nullptr;
    void* const list = screen.ptr(0xB8);
    for (std::int32_t i = 0; i < raw(list).at<std::int32_t>(0x104); ++i) {
        void* const row = host.list_box_row_00a9be00(list, i);
        const char* const text = static_cast<const char*>(host.widget_name_28(row).ptr(4));
        const std::int32_t index = host.atol_00bf8417(text != nullptr ? text : "");
        if (index == -1 || index == 99999) continue;
        const std::int32_t kind = screen.at<std::int32_t>(0x94);
        if (kind == 5) record = reinterpret_cast<const std::int32_t*>(static_cast<std::uint8_t*>(screen.ptr(0x460)) + index * 0x2C);
        else if (kind == 6) record = reinterpret_cast<const std::int32_t*>(static_cast<std::uint8_t*>(screen.ptr(0x470)) + index * 0x2C);
        const bool current = row == host.list_box_current_item_00a9ca00(list);
        if (!current && record == nullptr) return;
        const std::int32_t id = current ? class_id : *record;
        host.library_find_entry_by_id_004fb090(screen, id);
        std::string label = screen.at<std::int32_t>(0x94) == 6 && id == -1 ? std::string("FE.unitlib_nounlock")
                                                                             : host.vehicle_class_name_or_no_unlock_004fda50(id);
        if (screen.at<std::int32_t>(0x94) == 5 && record != nullptr && id != *record) label = "|.* |" + label;
        host.gui_text_set_ellipsized_00abb000(host.find_child_by_name_00aa7e00(row, "MenuItem_Text"), label,
                                              kTextWidthUnbounded, 1);
    }
}

bool library_find_entry_for_side_004fd500(Raw library, Raw out_object, std::uint32_t out_data[4],
                                          Fe8LibraryLookupHost& host) {
    const Raw game = host.game();
    const Raw slot{static_cast<std::uint8_t*>(game.ptr(0x18CC + game.at<std::uint32_t>(0x18EC) * 4))};
    const std::int32_t wanted = slot.at<std::int32_t>(0x28) == 0 ? game.at<std::int32_t>(0x734) : game.at<std::int32_t>(0x730);
    auto* const begin = static_cast<std::uint8_t*>(library.ptr(0x418));
    auto* const end = static_cast<std::uint8_t*>(library.ptr(0x41C));
    for (std::uint32_t i = 0; begin != nullptr && i < static_cast<std::uint32_t>((end - begin) / 0x6C); ++i) {
        const Raw entry{begin + i * 0x6C};
        if (entry.at<std::int32_t>(0x48) != wanted) continue;
        entry_assign_reference(out_object, entry, 0, host);
        for (int k = 0; k < 4; ++k) out_data[k] = entry.at<std::uint32_t>(4 + k * 4);
        return true;
    }
    return false;
}

void library_select_order_of_battle_side_00511bf0(Raw screen, std::uint32_t side, Fe8OrderOfBattleSideHost& host) {
    const std::int32_t value = screen.at<std::int32_t>(0x138 + side * 4);
    screen.at<std::int32_t>(0x434) = value;
    // FILD then FADD qword 0.003 (00CEBE08) on x87; double carries the int exactly.
    screen.at<float>(0x438) = static_cast<float>(static_cast<long double>(value) + 0.003000000026077032L);
    screen.at<std::int32_t>(0x558) = -1;
    screen.at<std::int32_t>(0x55C) = -1;
    const std::int32_t kind = screen.at<std::int32_t>(0x94);
    if (kind == 5) {
        const Raw record{static_cast<std::uint8_t*>(screen.ptr(0x460)) + side * 0x2C};
        const auto* const ids = static_cast<const std::int32_t*>(record.ptr(0x1C));
        const bool has_ids = ids != nullptr && static_cast<const std::int32_t*>(record.ptr(0x20)) != ids;
        if (has_ids && screen.at<std::int32_t>(0x434) != 0) screen.at<std::int32_t>(0x134) = ids[screen.at<std::int32_t>(0x434) - 1];
        else screen.at<std::int32_t>(0x134) = record.at<std::int32_t>(0);
        host.library_show_order_of_battle_page_004fff20(screen);
        if (host.native_string_equals_insensitive_00425850(record.p + 0x10, "change"))
            host.command_bar_set_help_line_0054a0c0("FE.oob_changeable_help");
        else if (host.native_string_equals_insensitive_00425850(record.p + 0x10, "fixed"))
            host.command_bar_set_help_line_0054a0c0("FE.oob_fixed_help");
        else
            host.command_bar_set_help_line_0054a0c0(native_view(record, 0x10));
    }
    // 00511E08 FLD qword [00CEBDD0] = 1/72 (true double), narrowed by FSTP dword at
    // 00511E18 and later added as a float (00511ECE FADD dword).
    const float extra = kind == 4 ? static_cast<float>(0.013888888888888888) : 0.0f;  // 00CEBDD0
    if (side != screen.at<std::uint32_t>(0x3C0) && kind >= 0 && kind <= 8 && kind != 7) {
        static constexpr std::size_t kSources[9] = {0x3D4, 0x3E4, 0x3F4, 0x3C4, 0x404, 0x3F4, 0x3F4, 0, 0x414};
        screen.at<std::uint32_t>(0x3C0) = side;
        host.library_filter_entries_0050d900(screen, screen.p + kSources[kind], screen.p + 0x444);
    }
    host.library_refresh_entry_detail_00510c20(screen);
    void* const scroller = screen.p + 0xCC;
    host.scroller_attach_content_00683790(scroller, nullptr);
    const double content = static_cast<double>(host.text_normalized_height_00ab6bd0(screen.ptr(0xA8)) + extra);
    host.scroller_set_scroll_range_006834a0(scroller,
                                            static_cast<float>(content - host.widget_size_00aa6740(screen.ptr(0x5AC))[1]));
}

void control_binding_bind_widgets_00555dc0(Raw screen, Fe8ControlBindWidgetsHost& host) {
    const auto find = [&host](void* parent, std::string_view name) { return host.find_child_by_name_00aa7e00(parent, name); };
    void* const list_listener = screen.p + 8;
    void* const widget_listener = screen.p + 0x0C;
    screen.ptr(0x1C) = find(screen.ptr(0x14), "Main_Listbox");
    host.list_box_set_listener_00a9ac40(screen.ptr(0x1C), list_listener);
    screen.ptr(0x24) = find(screen.ptr(0x1C), "MainListbox_Text");
    screen.ptr(0x210) = find(screen.ptr(0x18), "column_FrameBox");
    host.widget_set_visible_34(screen.ptr(0x210), 0);
    screen.ptr(0x214) = find(screen.ptr(0x18), "listbox_FrameBox");
    host.widget_set_visible_34(screen.ptr(0x214), 0);
    screen.ptr(0x218) = find(screen.ptr(0x18), "clipbox_Clipbox");
    screen.ptr(0x20) = find(screen.ptr(0x218), "Settings_Listbox");
    host.list_box_set_listener_00a9ac40(screen.ptr(0x20), list_listener);
    {
        const Raw settings = raw(screen.ptr(0x20));
        constexpr float kStep = 0.0027777778450399637f;  // 00CEE600
        settings.at<float>(0x12C) = 0.0f;
        settings.at<float>(0x130) = kStep;
        settings.at<float>(0x134) = 0.0f;
        host.list_box_set_field124_00a9ceb0(settings.p, 0.75f, 0);  // 00CEE07C
        settings.at<float>(0x128) = kStep;
    }
    screen.ptr(0x1FC) = find(screen.ptr(0x18), "slider_Group");
    screen.ptr(0x200) = find(screen.ptr(0x1FC), "scroll_up_Icon");
    screen.ptr(0x204) = find(screen.ptr(0x1FC), "scroll_down_Icon");
    screen.ptr(0x20C) = find(screen.ptr(0x1FC), "slider_FrameBox");
    screen.ptr(0x208) = find(screen.ptr(0x1FC), "back_Icon");
    for (const std::size_t off : {0x200u, 0x204u, 0x20Cu})
        host.widget_set_listener_fields_00aa6bc0(screen.ptr(off), widget_listener, 0);
    void* const scroller = screen.p + 0x1A0;
    host.scroller_attach_content_00683790(scroller, screen.ptr(0x20));
    const double back = host.widget_size_00aa6740(screen.ptr(0x208))[1];
    host.scroller_attach_scroll_bar_00683380(scroller, screen.ptr(0x20C),
                                             static_cast<float>(back - host.widget_size_00aa6740(screen.ptr(0x20C))[1]),
                                             screen.ptr(0x200), screen.ptr(0x204));
    screen.ptr(0x28) = find(screen.ptr(0x20), "setting_Group");
    host.widget_set_visible_34(screen.ptr(0x28), 0);
    for (std::int32_t k = 0; k < 3; ++k)  // main_Text, var_1_Text, var_2_Text
        host.gui_text_set_localised_cstring_00abbe50(find(screen.ptr(0x28), host.image_string_table(0x00E086A0, k)), "", 0);
    for (std::int32_t k = 3; k < 5; ++k)  // conflict_1_Group, conflict_2_Group
        host.widget_set_visible_34(find(screen.ptr(0x28), host.image_string_table(0x00E086A0, k)), 0);
    screen.ptr(0x2C) = find(screen.ptr(0x20), "slider_Group");
    host.widget_set_visible_34(screen.ptr(0x2C), 0);
    host.widget_set_listener_fields_00aa6bc0(find(screen.ptr(0x2C), "arrow_left_Icon"), widget_listener, 0);
    host.widget_set_listener_fields_00aa6bc0(find(screen.ptr(0x2C), "arrow_right_Icon"), widget_listener, 0);
    host.options_find_auto_control_group_005f0390(screen.ptr(0x2C), widget_listener);
    screen.ptr(0x30) = find(screen.ptr(0x20), "toggle_text_Group");
    host.widget_set_visible_34(screen.ptr(0x30), 0);
    host.widget_set_listener_fields_00aa6bc0(find(screen.ptr(0x30), "arrow_left_Icon"), widget_listener, 0);
    host.widget_set_listener_fields_00aa6bc0(find(screen.ptr(0x30), "arrow_right_Icon"), widget_listener, 0);
    screen.ptr(0x34) = find(screen.ptr(0x14), "SelectedConflict_Text");
}

namespace {

constexpr float kControlVariantThreshold = 0.625f;  // 00CEE8DC

void control_select_row(Raw screen, std::uint32_t id, Fe8ControlEventHost& host) {
    host.control_binding_refresh_selected_binding_0055c3b0(screen);
    host.list_box_select_index_00a9c7c0(screen.ptr(0x20), static_cast<std::int32_t>(id & 0xFFFF));
}

} // namespace

void control_binding_handle_gui_event_00560bf0(Raw listener, void* widget, Fe8ControlEventHost& host) {
    const Raw screen{listener.p - 0x0C};
    const std::int32_t page = screen.at<std::int32_t>(0x168);
    if (page == 3) return;
    if (host.widget_type_5c(widget) == 0x10) return;
    if (host.widget_type_5c(widget) == 6 && raw(widget).ptr(0x70) != screen.ptr(0x1FC)) {
        const std::uint32_t id = raw(raw(widget).ptr(0x70)).at<std::uint32_t>(0xD8);
        if (id == 0) return;
        screen.at<std::uint32_t>(0x180) = host.gui_manager_field_5c() > kControlVariantThreshold ? 1u : 0u;
        control_select_row(screen, id, host);
        if (host.widget_child_count_98(widget) <= 1) return;
        if (host.native_string_has_prefix_at_00553c80(host.widget_virtual_28(widget, "arrow_left_Icon", 0))) {
            host.control_binding_adjust_setting_0055ccc0(screen, -1);
            return;
        }
        if (host.native_string_has_prefix_at_00553c80(host.widget_virtual_28(widget, "arrow_right_Icon", 0)))
            host.control_binding_adjust_setting_0055ccc0(screen, 1);
        return;
    }
    if (host.widget_type_5c(widget) == 2) {
        const std::uint32_t id = raw(widget).at<std::uint32_t>(0xD8);
        if (id == 0) return;
        screen.at<std::uint32_t>(0x180) = host.gui_manager_field_5c() > kControlVariantThreshold ? 1u : 0u;
        control_select_row(screen, id, host);
        return;
    }
    if (host.widget_type_5c(widget) == 3) {
        host.widget_set_alpha_4c(widget, 1.0f);
        const auto* const label = static_cast<const std::uint8_t*>(raw(widget).ptr(0xF0));
        const std::uint8_t lead = label != nullptr ? label[0] : 0;  // "" at 00E1942C
        if (lead == 0xA7) return;
        if (lead == 0xA2) {
            if (page == 0 || page == 1)
                host.list_listener_activate_04(screen.p + 8, host.list_box_current_row_00425e50(screen.ptr(0x1C)), screen.ptr(0x1C));
            return;
        }
        if (lead == 0xA3) {
            if (page == 0) {
                host.set_options_page_00e19650(0);
                host.front_end_push_interface_request_004cc460(0x0C, 0);
                if (!host.input_config_load_suppressed_00f88a30()) host.input_configuration_load_00698a10();
                for (const std::size_t off : {0x16Cu, 0x170u, 0x174u, 0x178u, 0x17Cu}) screen.at<std::uint32_t>(off) = 0;
                host.widget_set_visible_34(screen.ptr(0x210), 0);
            } else if (page == 1) {
                host.control_binding_refresh_page_0055e3a0(screen, 0);
            } else if (page == 2) {
                host.control_binding_confirm_or_next_device_00560210(screen);
            }
            return;
        }
        const std::uint32_t id = raw(widget).at<std::uint32_t>(0xD8);
        if (id == 0) return;
        if ((id & 0xFF000000u) == 0xFE000000u) screen.at<std::uint32_t>(0x180) = 0;
        else if ((id & 0xFF000000u) == 0xFF000000u) screen.at<std::uint32_t>(0x180) = 1;
        control_select_row(screen, id, host);
        host.control_binding_position_marker_00553b00(screen, screen.at<std::uint8_t>(0x18C) != 0 ? nullptr : screen.ptr(0x190));
        host.control_binding_refresh_page_0055e3a0(screen, 3);
        return;
    }
    void* const scroller = screen.p + 0x1A0;
    if (widget == screen.ptr(0x204)) host.scroller_set_auto_scroll_00683300(scroller, 2, kLeaderboardAutoScrollSpeed, 1);
    else if (widget == screen.ptr(0x200)) host.scroller_set_auto_scroll_00683300(scroller, 1, kLeaderboardAutoScrollSpeed, 1);
    else if (widget == screen.ptr(0x20C)) host.scroller_begin_thumb_drag_00683a40(scroller);
    if (host.widget_type_5c(widget) == 6) {
        host.widget_set_highlight_88(widget, 2, 0, 1.0f);
        return;
    }
    if (host.widget_type_5c(widget) == 0x12) host.widget_set_highlight_84(widget, 2);
}

// --- packet cc13-fe9 -------------------------------------------------------
namespace {

// Case-insensitive `!(key < node)` after a lower bound: an empty key orders before a non-empty node
// key; a non-empty key against an empty node key counts as found.
bool binding_key_matches(std::string_view key, Raw node) {
    const std::string_view node_key = native_view(node, 0x0C);
    if (key.empty()) return node_key.empty();
    if (node_key.empty()) return true;
    return _stricmp(std::string(key).c_str(), std::string(node_key).c_str()) >= 0;
}

} // namespace

void control_binding_update_binding_row_text_0055c5a0(Raw screen, Fe9ControlRowsHost& host) {
    const Raw settings = host.input_settings_005547d0();
    const void* const key = binding_device_key(settings, screen);
    void* const entry = host.input_device_map_index_0055c110(settings.p + 8, key);
    const Raw rows{raw(screen.ptr(0x20)).p + 0x14C};
    void* const head = rows.ptr(4);
    for (void* node = raw(head).ptr(0); node != head; node = raw(node).ptr(0)) {
        void* const row = raw(node).ptr(8);
        if (host.find_child_by_name_00aa7e00(row, "main_Text", 0) == nullptr) continue;
        const std::string name(native_view(raw(host.find_child_by_name_00aa7e00(row, "main_Text", 1)), 0xF4));
        void* const end = raw(entry).ptr(4);
        void* const found = host.binding_tree_lower_bound_005548c0(entry, name);
        if (found == end || !binding_key_matches(name, raw(found))) continue;
        void* const slot_text[2] = {host.find_child_by_name_00aa7e00(row, "var_1_Text", 1),
                                    host.find_child_by_name_00aa7e00(row, "var_2_Text", 1)};
        for (std::uint32_t slot = 0; slot < 2; ++slot) {
            host.gui_text_set_localised_source_00abaed0(
                slot_text[slot], host.input_binding_slot_text_006a9850(settings, key, name, slot), 1);
        }
        static constexpr const char* kGroup[2] = {"conflict_1_Group", "conflict_2_Group"};
        for (std::uint32_t slot = 0; slot < 2; ++slot) {
            const bool conflict = host.input_binding_has_conflict_006aac50(settings, key, name, slot);
            void* const group = host.find_child_by_name_00aa7e00(row, kGroup[slot], 1);
            host.widget_set_visible_34(group, conflict ? 1 : 0);
            if (!conflict) continue;
            const float half = static_cast<float>(raw(slot_text[slot]).at<float>(0x114) / 960.0);  // 00CEC380
            host.widget_set_local_x_00aa78d0(host.find_child_by_name_00aa7e00(group, "Left_Text", 1), -half);
        }
    }
    host.control_binding_show_conflict_text_0055c200(screen);
}

namespace {

void* control_clone_row(Raw screen, std::size_t template_offset, Fe9ControlListHost& host) {
    void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(template_offset), 0);
    host.widget_set_visible_34(row, 1);
    return row;
}

void control_attach_row(Raw screen, void* row, std::uint32_t id, Fe9ControlListHost& host) {
    host.widget_set_listener_fields_00aa6bc0(row, screen.p + 0x0C, 0);
    raw(row).at<std::uint32_t>(0xD8) = id;
    host.widget_set_hit_clip_rect_00aa8f30(row, screen.ptr(0x218), 1);
}

} // namespace

void control_binding_build_settings_list_0055d380(Raw screen, Fe9ControlListHost& host) {
    void* const list = screen.ptr(0x20);
    host.list_box_set_listener_00a9ac40(list, nullptr);
    host.widget_set_enabled_60(list, 0);
    host.list_box_clear_rows_00a9bec0(list);
    const Raw settings = host.input_settings_005547d0();
    const void* const key = binding_device_key(settings, screen);
    const Raw entry = raw(host.input_device_map_index_0055c110(settings.p + 8, key));
    const auto vector_size = [&](std::size_t off) -> std::uint32_t {
        return entry.ptr(off) == nullptr ? 0
                                         : static_cast<std::uint32_t>((entry.at<std::uintptr_t>(off + 4) -
                                                                       entry.at<std::uintptr_t>(off)) / 8);
    };
    const auto vector_name = [&](std::size_t off, std::uint32_t i) {
        return native_view(Raw{static_cast<std::uint8_t*>(entry.ptr(off)) + i * 8}, 0);
    };
    const auto row_count = [&] { return raw(list).at<std::uint32_t>(0x104); };

    for (std::uint32_t i = 0; i < vector_size(0x34); ++i) {
        void* const row = control_clone_row(screen, 0x28, host);
        control_attach_row(screen, row, i | 0xFFFF0000u, host);
        const std::string name(vector_name(0x34, i));
        void* const found = host.binding_tree_lower_bound_005548c0(entry.p, name);
        const bool known = found != entry.ptr(4) && binding_key_matches(name, raw(found));
        void* const main_text = host.find_child_by_name_00aa7e00(row, "main_Text", 1);
        host.gui_text_set_localised_source_00abaed0(main_text, known ? name : "^" + name, 1);
        if (!known) {
            raw(row).at<std::uint8_t>(0x77) = 1;
        } else {
            raw(main_text).at<std::uint8_t>(0x78) = 0;
            void* const slot_text[2] = {host.find_child_by_name_00aa7e00(row, "var_1_Text", 1),
                                        host.find_child_by_name_00aa7e00(row, "var_2_Text", 1)};
            for (std::uint32_t slot = 0; slot < 2; ++slot) {
                host.gui_text_set_localised_source_00abaed0(
                    slot_text[slot], host.input_binding_slot_text_006a9850(settings, key, name, slot), 1);
                raw(slot_text[slot]).at<std::uint8_t>(0x78) = 1;
                host.widget_set_listener_fields_00aa6bc0(slot_text[slot], screen.p + 0x0C, 0);
                raw(slot_text[slot]).at<std::uint32_t>(0xD8) = i | (slot == 0 ? 0xFE000000u : 0xFF000000u);
            }
            static constexpr const char* kGroup[2] = {"conflict_1_Group", "conflict_2_Group"};
            for (std::uint32_t slot = 0; slot < 2; ++slot) {
                const bool conflict = host.input_binding_has_conflict_006aac50(settings, key, name, slot);
                void* const group = host.find_child_by_name_00aa7e00(row, kGroup[slot], 1);
                host.widget_set_visible_34(group, conflict ? 1 : 0);
                if (!conflict) continue;
                const float half = static_cast<float>(raw(slot_text[slot]).at<float>(0x114) / 960.0);  // 00CEC380
                const float x = static_cast<float>(static_cast<double>(half) + 0.010416666666666666);   // 00CEE728
                host.widget_set_local_x_00aa78d0(host.find_child_by_name_00aa7e00(group, "Right_Text", 1), x);
            }
        }
        host.list_box_insert_row_00a9d750(list, row, 0, 0);
    }

    if (vector_size(0x44) != 0) {
        void* const spacer = control_clone_row(screen, 0x28, host);
        raw(spacer).at<std::uint8_t>(0x77) = 1;
        host.list_box_insert_row_00a9d750(list, spacer, 0, 0);
        void* header = control_clone_row(screen, 0x28, host);
        host.gui_text_set_localised_cstring_00abbe50(host.find_child_by_name_00aa7e00(header, "main_Text", 1),
                                                     "^FE_pc.cs_invert", 1);
        raw(header).at<std::uint8_t>(0x77) = 1;
        raw(header).at<std::uint8_t>(0x78) = 0;
        host.list_box_insert_row_00a9d750(list, header, 0, 0);
        std::int32_t inverts = 0;
        for (std::uint32_t j = 0; j < vector_size(0x44); ++j) {
            const std::string_view name = vector_name(0x44, j);
            const std::int32_t kind = *host.setting_map_index_0055a9a0(entry.p + 0x18, name);
            if (kind != 3 && kind != 4) continue;
            void* const row = control_clone_row(screen, 0x30, host);
            control_attach_row(screen, row, row_count() | 0xFFFF0000u, host);
            void* const main_text = host.find_child_by_name_00aa7e00(row, "main_Text", 1);
            host.gui_text_set_localised_source_00abaed0(main_text, name, 1);
            raw(main_text).at<std::uint8_t>(0x78) = 0;
            void* const var_text = host.find_child_by_name_00aa7e00(row, "var_Text", 1);
            const float value = *host.setting_value_map_index_00444be0(entry.p + 0x24, name);
            host.gui_text_set_localised_cstring_00abbe50(var_text, value > 0.0f ? "FE.opt_normal" : "FE.opt_inverted", 1);
            raw(var_text).at<std::uint8_t>(0x78) = 0;
            host.list_box_insert_row_00a9d750(list, row, 0, 0);
            ++inverts;
        }
        if (inverts > 0) {
            void* const spacer2 = control_clone_row(screen, 0x28, host);
            raw(spacer2).at<std::uint8_t>(0x77) = 1;
            host.list_box_insert_row_00a9d750(list, spacer2, 0, 0);
            header = control_clone_row(screen, 0x28, host);
            raw(header).at<std::uint8_t>(0x77) = 1;
            raw(header).at<std::uint8_t>(0x78) = 0;
            host.list_box_insert_row_00a9d750(list, header, 0, 0);
        }
        // Without invert rows the image relabels the invert header (ESI still holds it).
        host.gui_text_set_localised_cstring_00abbe50(host.find_child_by_name_00aa7e00(header, "main_Text", 1),
                                                     "^FE_pc.controls_sensitivities", 1);
        for (std::uint32_t j = 0; j < vector_size(0x44); ++j) {
            const std::string_view name = vector_name(0x44, j);
            const std::int32_t kind = *host.setting_map_index_0055a9a0(entry.p + 0x18, name);
            if (kind == 3 || kind == 4) continue;
            void* const row = control_clone_row(screen, 0x2C, host);
            control_attach_row(screen, row, row_count() | 0xFFFF0000u, host);
            const std::uint32_t index = row_count();
            raw(host.find_child_by_name_00aa7e00(row, "dodgeListboxAutoControl_Group", 1)).at<std::uint32_t>(0xD8) = index;
            void* const main_text = host.find_child_by_name_00aa7e00(row, "main_Text", 1);
            host.gui_text_set_localised_source_00abaed0(main_text, name, 1);
            raw(main_text).at<std::uint8_t>(0x78) = 0;
            host.control_set_slider_row_005f3750(
                row, control_slider_position_0055ccc0(*host.setting_value_map_index_00444be0(entry.p + 0x24, name)));
            host.list_box_insert_row_00a9d750(list, row, 0, 0);
        }
    }

    const std::int32_t last = static_cast<std::int32_t>(row_count()) - 1;
    const std::int32_t wanted = screen.at<std::int32_t>(0x174);
    host.list_box_select_index_00a9c7c0(list, wanted < last ? wanted : last);
    host.widget_virtual_80(list, 1);
    host.list_box_set_listener_00a9ac40(list, screen.p + 8);
    host.widget_set_enabled_60(list, 1);
    raw(list).at<std::uint8_t>(0x77) = 0;
    if (screen.at<std::int32_t>(0x168) == 2) host.control_binding_refresh_selected_binding_0055c3b0(screen);
    void* const scroller = screen.p + 0x1A0;
    host.scroller_attach_content_00683790(scroller, nullptr);
    const double last_y = host.widget_resolved_position_00aa6750(host.list_box_row_00a9be00(list, last))[1];
    const float span = static_cast<float>(last_y - host.widget_resolved_position_00aa6750(host.list_box_row_00a9be00(list, 0))[1]);
    const float content = static_cast<float>(
        static_cast<double>(host.widget_size_00aa6740(host.list_box_row_00a9be00(list, 0))[1]) + 0.014999999664723873 + span);
    host.scroller_set_scroll_range_006834a0(
        scroller, static_cast<float>(static_cast<double>(content) - host.widget_size_00aa6740(screen.ptr(0x218))[1]));
}

namespace {

// 0055F090..0055F1BC: the preset's display name: drop the first 8 characters ("Presets/"), then
// the first ".preset" (strstr on the remaining characters).
std::string control_preset_display_name(std::string name) {
    name.erase(0, 8);
    const std::size_t at = name.find(".preset");
    if (at != std::string::npos) name.erase(at, 7);
    return name;
}

bool control_preset_name_allowed(std::string_view name) {
    for (const char c : name) {
        const bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ';
        if (!ok) return false;
    }
    return true;
}

bool control_preset_file_valid(std::string_view path, Fe9ControlPageHost& host) {
    void* const stream = host.vfs_open_04(path, 2);
    if (stream == nullptr) return false;
    bool valid = false;
    if (host.stream_is_open_18(stream)) {
        static constexpr char kMagic[] = "CONTROL_PRESET\x01";  // 00CEE75C, 15 characters
        std::uint8_t header[0x40] = {};
        host.stream_read_24(stream, header, 15);
        valid = std::memcmp(header, kMagic, 15) == 0;
    }
    host.stream_delete_04(stream);
    return valid;
}

} // namespace

void control_binding_refresh_page_0055e3a0(Raw screen, std::int32_t page, Fe9ControlPageHost& host) {
    const Raw settings = host.input_settings_005547d0();
    host.input_manager_update_00a92c40(0.0f);
    screen.at<std::uint8_t>(0x195) = 1;
    screen.at<float>(0x198) = 0.5f;  // 00CE3800
    host.widget_virtual_84(screen.ptr(0x214), host.input_flag_61f() ? 1 : 0);
    if (page == 2) {
        host.rebuild_command_bar_0054b530({{
            {0, "", 1}, {0, "", 1}, {0xB3, "globals.navigate", 1}, {0, "", 1}, {0xA3, "globals.back", 2},
        }});
        if (!host.flag_00e19428() && host.input_backend_device_004ba6d0(2, 0) == nullptr &&
            host.input_device_has_kind2_binding_006a9ea0(settings, binding_device_key(settings, screen))) {
            host.raise_prompt_00531b00(3, "FE_pc.cs_touchcontroller", 3, 0);
            screen.at<std::int32_t>(0x168) = 4;
            return;
        }
        host.widget_set_visible_34(screen.ptr(0x214), 1);
    } else if (page == 3) {
        host.rebuild_command_bar_0054b530({{{0, "", 1}, {0, "", 1}, {0, "", 1}, {0, "", 1}, {0, "", 1}}});
        host.widget_set_visible_34(screen.ptr(0x214), 1);
    } else {
        host.rebuild_command_bar_0054b530({{
            {0xA2, "globals.select", 0}, {0, "", 1}, {0xB3, "globals.navigate", 1}, {0, "", 1}, {0xA3, "globals.back", 2},
        }});
        host.widget_set_visible_34(screen.ptr(0x214), 0);
    }

    const std::int32_t old_page = screen.at<std::int32_t>(0x168);
    const bool keep = (old_page == 3 && page == 2) || (old_page == 2 && page == 3);
    screen.at<std::int32_t>(0x168) = page;
    void* const devices = screen.ptr(0x1C);
    void* const settings_list = screen.ptr(0x20);
    host.list_box_set_listener_00a9ac40(devices, nullptr);
    host.widget_set_visible_34(devices, 0);
    host.widget_set_enabled_60(devices, 0);
    host.widget_set_visible_34(settings_list, 0);
    host.widget_set_enabled_60(settings_list, 0);
    host.command_bar_set_help_line_0054a0c0("");
    if (!keep) {
        host.list_box_set_listener_00a9ac40(settings_list, nullptr);
        host.widget_set_enabled_60(settings_list, 0);
    }
    host.widget_set_visible_34(screen.ptr(0x34), 0);

    std::uint32_t marker_visible = 0;
    bool select_devices = true;
    if (page == 0) {
        host.frame_select_00518d60(6, 0, 0, "^FE_pc.controls_categories");
        host.list_box_clear_rows_00a9bec0(devices);
        void* const header = host.widget_clone_subtree_00aab4c0(screen.ptr(0x24), 0);
        host.gui_text_set_localised_cstring_00abbe50(header, "FE_pc.controls_general", 1);
        host.list_box_insert_row_00a9d750(devices, header, 0, 0);
        const auto* first = static_cast<const std::uint8_t*>(settings.ptr(0x18));
        const std::uint32_t count =
            first == nullptr ? 0 : static_cast<std::uint32_t>((static_cast<const std::uint8_t*>(settings.ptr(0x1C)) - first) / 8);
        for (std::uint32_t i = 0; i < count; ++i) {
            void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0x24), 0);
            host.widget_virtual_7c(row, host.flag_00f88a30());
            host.gui_text_set_localised_source_00abaed0(row, native_view(Raw{const_cast<std::uint8_t*>(first) + i * 8}, 0), 1);
            host.list_box_insert_row_00a9d750(devices, row, 0, 0);
        }
        void* const load = host.widget_clone_subtree_00aab4c0(screen.ptr(0x24), 0);
        host.widget_virtual_7c(load, host.flag_00f88a30());
        raw(load).at<std::uint32_t>(0xD8) = 1;
        host.gui_text_set_localised_cstring_00abbe50(load, "FE_pc.controls_loadpreset", 1);
        host.list_box_insert_row_00a9d750(devices, load, 0, 0);
        host.widget_set_visible_34(devices, 1);
        host.widget_set_enabled_60(devices, 1);
        host.list_box_reset_highlight_00a9cf30(devices, 1, 1);
        screen.at<std::uint8_t>(0x19C) = 0;
        if (host.input_flag_61f()) host.frame_select_00518d60(0, 0, 0, "^FE_pc.controls_categories");
    } else if (page == 1) {
        host.frame_select_00518d60(6, 0, 0, "^FE_pc.controls_loadpreset");
        host.list_box_clear_rows_00a9bec0(devices);
        std::vector<std::string> order;
        void* const globals = host.lua_owner_globals_00b67980(settings.p + 0x78);
        void* const table = host.lua_get_by_name_00b67800(globals, "ControlPresetsOrder");
        host.lua_release_00b67700(globals);
        for (std::int32_t i = 1;; ++i) {
            void* const probe = host.lua_get_by_index_00b67720(table, i);
            const bool nil = host.lua_is_nil_00b65fb0(probe);
            host.lua_release_00b67700(probe);
            if (nil) break;
            void* const item = host.lua_get_by_index_00b67720(table, i);
            const char* const text = host.lua_get_string_00b662b0(item);
            order.emplace_back(text != nullptr ? text : "");
            host.lua_release_00b67700(item);
        }
        for (const std::string& name : order) {
            void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0x24), 0);
            host.gui_text_set_localised_source_00abaed0(row, name, 1);
            host.list_box_insert_row_00a9d750(devices, row, 0, 0);
        }
        std::vector<std::string> files = host.vfs_enumerate_resources_00bdd990("Presets", ".preset", 0);
        const std::int32_t limit = host.input_flag_61f() ? 7 : 10;
        for (std::size_t f = 0; f < files.size() && raw(devices).at<std::int32_t>(0x104) < limit; ++f) {
            const std::string& path = files[f];
            const std::string name = control_preset_display_name(path);
            if (name.empty() || !control_preset_name_allowed(name)) continue;
            if (!control_preset_file_valid(path, host)) continue;
            void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0x24), 0);
            host.widget_set_size_58(row, {0.3645833432674408f, host.widget_size_00aa6740(row)[1]});  // 00CEE758
            host.gui_text_set_ellipsized_00abb000(row, name, -1.0f, 1);                              // 00D7A260
            host.list_box_insert_row_00a9d750(devices, row, 0, 0);
        }
        host.widget_set_visible_34(devices, 1);
        host.widget_set_enabled_60(devices, 1);
        host.list_box_reset_highlight_00a9cf30(devices, 1, 1);
        host.lua_release_00b67700(table);
    } else if (page == 2) {
        host.input_device_map_index_0055c110(settings.p + 8, binding_device_key(settings, screen));
        if (!host.input_flag_61f()) {
            host.frame_select_00518d60(6, 0, 0, "^" + std::string(native_view(Raw{static_cast<std::uint8_t*>(
                                                        const_cast<void*>(binding_device_key(settings, screen)))}, 0)));
        }
        if (!keep) {
            host.control_binding_build_settings_list_0055d380(screen);
        } else {
            host.widget_set_enabled_60(settings_list, 1);
            raw(settings_list).at<std::uint8_t>(0x77) = 0;
            host.list_box_set_listener_00a9ac40(settings_list, screen.p + 8);
        }
        host.widget_set_visible_34(settings_list, 1);
        host.widget_set_enabled_60(settings_list, 1);
        host.widget_set_visible_34(screen.ptr(0x1FC), 1);
        select_devices = false;
    } else if (page == 3) {
        if (!host.input_flag_61f()) host.frame_select_00518d60(6, 0, 0, "^FE_pc.controls_adjust");
        host.widget_set_enabled_60(settings_list, 0);
        raw(settings_list).at<std::uint8_t>(0x77) = 1;
        host.widget_set_visible_34(settings_list, 1);
        host.widget_set_enabled_60(settings_list, 1);
        marker_visible = 1;
        select_devices = false;
        host.widget_set_visible_34(screen.ptr(0x1FC), marker_visible);
    }
    if (select_devices) {
        host.list_box_select_index_00a9c7c0(devices, screen.at<std::int32_t>(0x16C + page * 4));
        host.widget_set_enabled_60(devices, 1);
        raw(devices).at<std::uint8_t>(0x77) = 0;
        host.list_box_set_listener_00a9ac40(devices, screen.p + 8);
        host.widget_virtual_80(devices, 1);
        host.widget_set_visible_34(screen.ptr(0x1FC), marker_visible);
    }
    host.list_box_set_listener_00a9ac40(devices, screen.p + 8);
    raw(settings_list).at<std::uint8_t>(0x14A) = 0;
}

void control_binding_load_selected_preset_0055f710(Raw screen, Fe9ControlPresetHost& host) {
    if (screen.at<std::uint8_t>(0x38) != 0) {
        const std::string name(native_view(raw(host.list_box_current_row_00425e50(screen.ptr(0x1C))), 0xF4));
        const Raw settings = host.input_settings_005547d0();
        void* const globals = host.lua_owner_globals_00b67980(settings.p + 0x78);
        void* const presets = host.lua_get_by_name_00b67800(globals, "ControlPresets");
        void* const preset = host.lua_get_by_native_string_00b68100(presets, name);
        host.lua_release_00b67700(presets);
        host.lua_release_00b67700(globals);
        host.input_settings_read_keyboard_archive_006aba50(preset);
        host.lua_release_00b67700(preset);
    } else {
        void* const owner = host.lua_owner_open_run_flagged(1, native_view(screen, 0x3C), 1);
        void* const globals = host.lua_owner_globals_00b67980(owner);
        void* const preset = host.lua_get_by_name_00b67800(globals, "Preset");
        host.lua_release_00b67700(globals);
        if (!host.lua_is_nil_00b65fb0(preset)) host.input_settings_read_keyboard_archive_006aba50(preset);
        host.lua_release_00b67700(preset);
        host.lua_owner_close_00b669a0(owner);
    }
    host.control_binding_refresh_page_0055e3a0(screen, 0);
}

namespace {

bool equals_case_insensitive(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lower = [](char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

void control_leave_bindings(Raw screen, Fe9ControlListenerHost& host) {
    if (!host.input_config_load_suppressed_00f88a30()) host.input_configuration_load_00698a10();
    for (std::size_t off = 0x16C; off <= 0x17C; off += 4) screen.at<std::uint32_t>(off) = 0;
}

} // namespace

void control_binding_list_listener_activate_0055fca0(Raw listener, void* row, void* list_box,
                                                     Fe9ControlListenerHost& host) {
    const Raw screen{listener.p - 8};
    const std::int32_t page = screen.at<std::int32_t>(0x168);
    const std::int32_t selection = host.list_box_selected_index_00a9c920(list_box);
    screen.at<std::int32_t>(0x16C + page * 4) = selection;
    const Raw settings = host.input_settings_005547d0();
    if (page == 0) {
        const auto* first = static_cast<const std::uint8_t*>(settings.ptr(0x18));
        const std::uint32_t keys =
            first == nullptr ? 0 : static_cast<std::uint32_t>((static_cast<const std::uint8_t*>(settings.ptr(0x1C)) - first) / 8);
        const auto sel = static_cast<std::uint32_t>(screen.at<std::int32_t>(0x16C));
        if (sel < keys + 1) {
            if (sel == 0) {
                host.set_options_page_00e19650(4);
                host.front_end_mark_field_40();
                host.front_end_push_interface_request_004cc460(0x0C, 0);
                if (host.input_flag_61f()) host.frame_select_00518d60(-1, -1, 0, "FE.opt_title_main");
                control_leave_bindings(screen, host);
                return;
            }
            screen.at<std::uint32_t>(0x16C) = sel - 1;
            screen.at<std::uint32_t>(0x174) = 0;
            screen.at<std::uint32_t>(0x180) = 1;
            host.control_binding_refresh_page_0055e3a0(screen, 2);
            host.input_settings_006aa1c0(settings);
            if (host.input_flag_61f()) host.frame_select_default_004c9680();
            return;
        }
        if (sel == keys + 3) {
            host.front_end_push_interface_request_004cc460(0x0C, 0);
            control_leave_bindings(screen, host);
            return;
        }
        const std::uint32_t id = raw(row).at<std::uint32_t>(0xD8);
        if (id == 1) host.control_binding_refresh_page_0055e3a0(screen, 1);
        else if (id == 2) host.control_binding_open_virtual_keyboard_00553bf0(screen);
        return;
    }
    if (page != 1) return;
    const std::string name(native_view(raw(row), 0xF4));
    if (equals_case_insensitive(name, "FE.opt_cancel")) {
        host.control_binding_refresh_page_0055e3a0(screen, 0);
        return;
    }
    const std::int32_t sel = screen.at<std::int32_t>(0x16C + page * 4);
    if (sel < 0) {
        host.control_binding_refresh_page_0055e3a0(screen, 0);
        return;
    }
    if (sel == 0) {
        screen.at<std::uint8_t>(0x38) = 1;
    } else {
        screen.at<std::uint8_t>(0x38) = 0;
        char path[0x100];
        std::snprintf(path, sizeof path, "Presets/%s.preset", name.c_str());  // sprintf_s, 00CEE844
        host.native_string_assign_cstring_0041e350(screen.p + 0x3C, path);
        if (!host.vfs_exists_08(native_view(screen, 0x3C))) return;
    }
    host.raise_prompt_00531b00(0, "FE_pc.cs_loadpreset_confirm", 1, 0x0055F9E0);
    host.prompt_focus_last_navigation_00530c20();
}

void library_bind_layout_004fe870(Raw screen, Fe9LibraryLayoutHost& host) {
    const auto bind = [&](std::size_t parent, const char* name, std::size_t store) {
        screen.ptr(store) = host.find_child_by_name_00aa7e00(screen.ptr(parent), name);
    };
    bind(0x4E0, "homaly_Clipbox", 0xC8);
    for (const auto& [name, store] : {std::pair<const char*, std::size_t>{"MainOOB_Listbox", 0xBC},
                                      {"Main_Listbox", 0xC0}, {"Difficulty_Listbox", 0xC4}}) {
        bind(0x4E4, name, store);
        host.list_box_set_listener_00a9ac40(screen.ptr(store), screen.p + 0x64);
        host.list_box_set_input_filter_00a9ac70(screen.ptr(store), 0x004F90D0);
    }
    bind(0x4E4, "Fixed_Text", 0x128);
    bind(0x4E4, "Changeable_Text", 0x12C);
    bind(0x4E4, "Bonus_Text", 0x130);
    bind(0x4DC, "hajnal_Group", 0x4E8);
    bind(0x4E8, "powerups_Group", 0x4F8);
    bind(0x4E8, "achievment_Group", 0x4F4);
    bind(0x4E8, "movies_Group", 0x4F0);
    bind(0x4E8, "tutorial_Group", 0x500);
    bind(0x4E8, "units_Group", 0x4EC);
    bind(0x4E8, "noseart_Group", 0x504);
    bind(0x4E8, "arrow_left_Icon", 0x56C);
    bind(0x4E8, "arrow_right_Icon", 0x570);
    bind(0x4E8, "left_big_Icon", 0x574);
    bind(0x4E8, "right_big_Icon", 0x578);
    bind(0x4DC, "background_Group", 0x518);
    host.widget_set_visible_34(screen.ptr(0x518), 1);
    bind(0x4DC, "INFO_box_Group", 0x4FC);
    bind(0x4FC, "test_Clipbox", 0x5AC);
    bind(0x5AC, "textbox_Group", 0x514);
    bind(0x514, "content_main_Text", 0xA8);
    host.widget_set_visible_34(screen.ptr(0xA8), 0);
    void* const scroller = screen.p + 0xCC;
    host.scroller_attach_content_00683790(scroller, screen.ptr(0x514));
    bind(0x4FC, "arrow_top_scroll_Icon", 0x5B8);
    bind(0x4FC, "arrow_botton_scroll_Icon", 0x5BC);
    bind(0x4FC, "textx_scroll_Framebox", 0x5C0);
    for (const std::size_t off : {0x5C0u, 0x5B8u, 0x5BCu})
        host.widget_set_listener_fields_00aa6bc0(screen.ptr(off), screen.p + 0x68, 0);
    host.scroller_attach_scroll_bar_00683380(scroller, screen.ptr(0x5C0), 0.0902777761220932f, screen.ptr(0x5B8),
                                             screen.ptr(0x5BC));
}

namespace {

struct alignas(4) LibraryEntryBuffer {
    std::uint8_t bytes[0x6C];
};

Raw library_entry_blank(LibraryEntryBuffer& buffer) {
    std::memset(buffer.bytes, 0, sizeof buffer.bytes);
    const Raw e{buffer.bytes};
    for (const std::size_t uv : {std::size_t{4}, std::size_t{0x18}}) {
        e.at<float>(uv + 8) = 1.0f;   // 00D7A24C
        e.at<float>(uv + 12) = 1.0f;
    }
    return e;
}

// Resolve, then release the old reference, then store (the image's order).
void library_entry_set_texture(Raw entry, std::size_t ref, std::size_t uv, std::string_view name,
                               Fe9LibraryLoaderHost& host) {
    float size[2] = {};
    void* const texture = host.resolve_gui_texture_00aa2660(name, &entry.at<float>(uv), size, 1.0f);
    if (entry.ptr(ref) != nullptr) host.texture_ref_release(entry.ptr(ref));
    entry.ptr(ref) = texture;
}

std::string lua_string_or_empty(void* table, const char* name, Fe9LibraryLoaderHost& host) {
    void* const field = host.lua_get_by_name_00b67800(table, name);
    const char* const text = host.lua_get_string_00b662b0(field);
    std::string out = text != nullptr ? text : "";
    host.lua_release_00b67700(field);
    return out;
}

bool lua_index_is_nil(void* table, std::int32_t index, Fe9LibraryLoaderHost& host) {
    void* const probe = host.lua_get_by_index_00b67720(table, index);
    const bool nil = host.lua_is_nil_00b65fb0(probe);
    host.lua_release_00b67700(probe);
    return nil;
}

std::string native_string_text(Raw s) { return std::string(native_view(s, 0)); }

// The grouped loaders 0050B8E0 / 0050C760 / 0050CE70: groups[g + 1].GroupName into the name slots,
// then each non-nil item of the group appended as a fresh entry and filled in place.
template <typename Fill>
void library_load_grouped(Raw screen, const char* script, const char* table_name, std::int32_t groups,
                          std::size_t names, std::size_t vector, Fill fill, Fe9LibraryLoaderHost& host) {
    void* const owner = host.lua_owner_open_run_00b6a020(4, script);
    void* const globals = host.lua_owner_globals_00b67980(owner);
    void* const classes = host.lua_get_by_name_00b67800(globals, table_name);
    host.lua_release_00b67700(globals);
    for (std::int32_t g = 0; g < groups; ++g) {
        void* const group = host.lua_get_by_index_00b67720(classes, g + 1);
        host.native_string_assign(screen.p + names + g * 8, lua_string_or_empty(group, "GroupName", host));
        for (std::int32_t j = 1; !lua_index_is_nil(group, j, host); ++j) {
            LibraryEntryBuffer fresh;
            const Raw blank = library_entry_blank(fresh);
            void* const list = screen.p + vector;
            host.entry_vector_push_back_0050ad00(list, blank);
            host.library_entry_destruct_004fd1d0(blank);
            const Raw back{static_cast<std::uint8_t*>(raw(list).ptr(8)) - 0x6C};
            void* const item = host.lua_get_by_index_00b67720(group, j);
            fill(back, item, g);
            host.lua_release_00b67700(item);
        }
        host.lua_release_00b67700(group);
    }
    host.lua_release_00b67700(classes);
    host.lua_owner_close_00b669a0(owner);
}

} // namespace

void library_load_power_up_lib_0050b8e0(Raw screen, Fe9LibraryLoaderHost& host) {
    library_load_grouped(screen, "Scripts\\datatables\\PowerUpLib.lua", "PowerupClasses", 3, 0x340, 0x3E4,
        [&](Raw back, void* item, std::int32_t g) {
            host.lua_read_string_field("name", back.p + 0x28, item);
            LibraryEntryBuffer temp;
            const Raw strings = library_entry_blank(temp);  // two scratch NativeStrings at +28h / +30h
            host.lua_read_string_field("GUITextureBackGround", strings.p + 0x28, item);
            host.lua_read_string_field("GUITextureForeGround", strings.p + 0x30, item);
            library_entry_set_texture(back, 0x00, 0x04, native_view(strings, 0x28), host);
            library_entry_set_texture(back, 0x14, 0x18, native_view(strings, 0x30), host);
            back.at<std::int32_t>(0x40) = g;
            back.at<std::uint8_t>(0x44) = 1;
            host.lua_read_string_field("description", back.p + 0x5C, item);
            host.library_entry_destruct_004fd1d0(strings);
        },
        host);
}

void library_load_nose_art_0050bf00(Raw screen, Fe9LibraryLoaderHost& host) {
    void* const globals = host.lua_owner_globals_00b67980(host.game_lua_owner_1a0c());
    void* const table = host.lua_get_by_name_00b67800(globals, "NoseArt");
    host.lua_release_00b67700(globals);
    LibraryEntryBuffer buffer;
    const Raw entry = library_entry_blank(buffer);
    if (host.lua_is_table_00b661b0(table)) {
        for (std::int32_t i = 0; i < 1000; ++i) {
            void* const item = host.lua_get_by_index_00b67720(table, i);
            if (!host.lua_is_nil_00b65fb0(item) && host.lua_is_table_00b661b0(item)) {
                const std::string path = "models/Textures/noseart/" + lua_string_or_empty(item, "Texture", host);
                library_entry_set_texture(entry, 0x00, 0x04, path, host);
                void* const japan = host.lua_get_by_name_00b67800(item, "IsJapan");
                entry.at<std::int32_t>(0x40) = host.lua_get_integer_00b66290(japan);
                host.lua_release_00b67700(japan);
                host.native_string_assign(entry.p + 0x5C, "");
                entry.at<std::int32_t>(0x48) = i;
                entry.at<std::uint8_t>(0x44) = 1;
                host.entry_vector_push_back_0050ad00(screen.p + 0x414, entry);
            }
            host.lua_release_00b67700(item);
        }
    }
    host.library_entry_destruct_004fd1d0(entry);
    host.lua_release_00b67700(table);
}

void library_load_movie_list_0050c300(Raw screen, Fe9LibraryLoaderHost& host) {
    static constexpr const char* kImage[4] = {"JP_intro.tga", "JP_outro.tga", "US_intro.tga", "US_outro.tga"};
    static constexpr const char* kMission[4] = {"", "IJN14", "", "USN14"};
    static constexpr const char* kTitle[4] = {"FE.movie_BG_JP_intro", "FE.movie_BG_JP_outro",
                                              "FE.movie_BG_US_intro", "FE.movie_BG_US_outro"};
    LibraryEntryBuffer buffer;
    const Raw entry = library_entry_blank(buffer);
    for (int i = 0; i < 4; ++i) {
        library_entry_set_texture(entry, 0x00, 0x04, std::string("fe/movies/") + kImage[i], host);
        host.native_string_assign(entry.p + 0x30, kMission[i]);
        entry.at<std::int32_t>(0x40) = 0;
        entry.at<std::uint8_t>(0x44) = 1;
        host.native_string_assign(entry.p + 0x5C, kTitle[i]);
        host.entry_vector_push_back_0050ad00(screen.p + 0x3C4, entry);
    }
    host.library_entry_destruct_004fd1d0(entry);
}

void library_load_tutorial_lib_0050c760(Raw screen, Fe9LibraryLoaderHost& host) {
    library_load_grouped(screen, "Scripts\\datatables\\TutorialLib.lua", "TutorialLib", 13, 0x358, 0x404,
        [&](Raw back, void* item, std::int32_t g) {
            host.lua_read_string_field("Name", back.p + 0x28, item);
            void* const index = host.lua_get_by_name_00b67800(item, "Index");
            back.at<std::int32_t>(0x48) = host.lua_get_integer_00b66290(index);
            host.lua_release_00b67700(index);
            LibraryEntryBuffer temp;
            const Raw strings = library_entry_blank(temp);
            host.lua_read_string_field("Movie", strings.p + 0x28, item);
            host.native_string_assign(back.p + 0x30, native_view(strings, 0x28));
            back.at<std::uint8_t>(0x44) = 1;
            void* const command = host.lua_get_by_name_00b67800(item, "LuaCommand");
            const bool nil = host.lua_is_nil_00b65fb0(command);
            host.lua_release_00b67700(command);
            host.native_string_assign(back.p + 0x38, nil ? std::string() : lua_string_or_empty(item, "LuaCommand", host));
            back.at<std::int32_t>(0x40) = g;
            back.at<std::uint8_t>(0x44) = 1;
            host.lua_read_string_field("Description", back.p + 0x5C, item);
            host.library_entry_destruct_004fd1d0(strings);
        },
        host);
}

void library_load_unit_lib_0050ce70(Raw screen, Fe9LibraryLoaderHost& host) {
    library_load_grouped(screen, "Scripts\\datatables\\UnitLib.lua", "UnitLib", 5, 0x318, 0x3F4,
        [&](Raw back, void* item, std::int32_t g) {
            host.lua_read_string_field("Name", back.p + 0x28, item);
            void* const vehicle_class = host.lua_get_by_name_00b67800(item, "VehicleClass");
            back.at<std::int32_t>(0x48) = host.lua_get_integer_00b66290(vehicle_class);
            host.lua_release_00b67700(vehicle_class);
            LibraryEntryBuffer temp;
            const Raw strings = library_entry_blank(temp);
            host.lua_read_string_field("GUITextureBackGround", strings.p + 0x28, item);
            host.lua_read_string_field("GUITextureForeGround", strings.p + 0x30, item);
            library_entry_set_texture(back, 0x00, 0x04, "fe/units/" + native_string_text(Raw{strings.p + 0x28}), host);
            library_entry_set_texture(back, 0x14, 0x18, "common/icons/" + native_string_text(Raw{strings.p + 0x30}), host);
            back.at<std::int32_t>(0x40) = g;
            back.at<std::uint8_t>(0x44) = 1;
            back.at<std::uint8_t>(0x45) = 0;
            void* const description = host.lua_get_by_name_00b67800(item, "Description");
            host.lua_read_string_by_index_004fe1e0(1, back.p + 0x4C, description);
            host.lua_read_string_by_index_004fe1e0(2, back.p + 0x54, description);
            host.lua_read_string_by_index_004fe1e0(3, back.p + 0x5C, description);
            host.lua_read_string_field_00501d10(4, back.p + 0x64, description, "");
            host.lua_release_00b67700(description);
            host.library_entry_destruct_004fd1d0(strings);
        },
        host);
}

void library_register_0050fdb0(Raw screen, Fe9LibraryRegisterHost& host) {
    host.frontend_screen_register_004f71d0(screen);
    screen.ptr(0x4DC) = host.gui_load_page_00aa5840("FE_tactical", 1, 0);
    screen.ptr(0x4E0) = host.gui_load_page_00aa5840("FE_tactical_text", 1, 0);
    screen.ptr(0x4E4) = host.gui_load_page_00aa5840("FE_tactical_listbox", 1, 0);
    screen.ptr(0x6C) = host.resource_load_64("colourremap.tga", 0);
    screen.ptr(0x70) = host.resource_load_64("oceannormalmap04.tga", 0);
    screen.ptr(0x74) = host.resource_load_64("hdr_rcs_usn_04_defend_g_ocean.tga", 0);
    screen.ptr(0x78) = host.resource_load_64("hdr_RCS_usn_04_Defend_G.tga", 0);
    host.screen_virtual_14(screen);
    screen.at<std::int32_t>(0x4C8) = 1;
    screen.at<std::int32_t>(0x454) = -1;
    screen.at<std::int32_t>(0x458) = 0;
    screen.at<std::int32_t>(0x94) = 3;
    screen.at<std::int32_t>(0x9C) = 0;
    screen.at<std::int32_t>(0xA0) = 2;
    for (std::size_t off = 0x540; off <= 0x554; off += 4) screen.at<std::uint32_t>(off) = 0;

    void* const storage = host.operator_new_00bf681b(0x6C);
    if (storage != nullptr) {
        LibraryEntryBuffer blank;
        library_entry_blank(blank);
        std::memcpy(storage, blank.bytes, sizeof blank.bytes);
    }
    screen.ptr(0x5B0) = storage;
    const Raw locked = raw(storage);
    host.native_string_assign(locked.p + 0x28, "FE.unitlib_nounlock");
    locked.at<std::int32_t>(0x48) = -1;
    library_entry_set_texture(locked, 0x00, 0x04, "fe/lobby/unlock_disabled.tga", host);
    locked.at<std::int32_t>(0x40) = -1;
    locked.at<std::uint8_t>(0x44) = 1;
    locked.at<std::uint8_t>(0x46) = 1;
    host.native_string_assign(locked.p + 0x4C, "FE.unitlib_nounit_d1");
    host.native_string_assign(locked.p + 0x54, "FE.unitlib_nounit_d2");
    host.native_string_assign(locked.p + 0x5C, "FE.unitlib_nounit_d3");
    host.native_string_assign(locked.p + 0x64, " ");

    static constexpr std::pair<const char*, std::int32_t> kCategory[] = {
        {"RANK_", 0}, {"CA_", 1}, {"MA_", 2}, {"AO_", 2}, {"RUA_", 3}, {"GA_", 4}, {"SA_", 5}};
    for (std::uint32_t i = 0;; ++i) {
        const Raw awards = host.award_table_00e19900();
        const auto* first = static_cast<std::uint8_t*>(awards.ptr(4));
        if (first == nullptr || i >= static_cast<std::uint32_t>((static_cast<std::uint8_t*>(awards.ptr(8)) - first) / 8)) break;
        const Raw name{const_cast<std::uint8_t*>(first) + i * 8};
        LibraryEntryBuffer fresh;
        const Raw blank = library_entry_blank(fresh);
        void* const list = screen.p + 0x3D4;
        host.entry_vector_push_back_0050ad00(list, blank);
        host.library_entry_destruct_004fd1d0(blank);
        const Raw back{static_cast<std::uint8_t*>(raw(list).ptr(8)) - 0x6C};
        const Raw record = host.award_table_find_record_0050fc30(name.p);
        // The image builds the path through a NativeString and re-reads it as a C string.
        const std::string icon(native_view(record, 0x14));
        library_entry_set_texture(back, 0x14, 0x18, icon.c_str(), host);
        library_entry_set_texture(back, 0x00, 0x04, "common/whiteGui.tga", host);
        back.at<std::int32_t>(0x40) = 6;
        const std::string_view text = native_view(name, 0);
        for (const auto& [prefix, category] : kCategory) {
            const std::string_view head = text.substr(0, std::strlen(prefix));
            if (!head.empty() && equals_case_insensitive(head, prefix)) back.at<std::int32_t>(0x40) = category;
        }
        const std::string_view head = text.substr(0, 1);
        if (!head.empty() && equals_case_insensitive(head, "B")) {
            back.at<std::int32_t>(0x40) = 7;
            back.at<std::uint8_t>(0x44) = 0;
        } else {
            back.at<std::uint8_t>(0x44) = 1;
        }
        host.native_string_assign(back.p + 0x28, text);
        back.at<std::uint8_t>(0x45) = 1;
    }
    host.library_load_unit_lib_0050ce70(screen);
    host.library_load_class_tables_00504130(screen);
    host.library_load_power_up_lib_0050b8e0(screen);
    host.library_load_movie_list_0050c300(screen);
    host.library_load_tutorial_lib_0050c760(screen);
    host.library_load_nose_art_0050bf00(screen);
    host.library_preview_setup_environment_00502480(screen);
}

namespace {

std::int32_t library_view_count(Raw view) {
    const auto* first = static_cast<std::uint8_t*>(view.ptr(0x14));
    return first == nullptr ? 0 : static_cast<std::int32_t>((static_cast<std::uint8_t*>(view.ptr(0x18)) - first) / 0x6C);
}

std::uint8_t* library_view_entry(Raw view, std::int32_t index) {
    return static_cast<std::uint8_t*>(view.ptr(0x14)) + index * 0x6C;
}

std::string upper_ascii(std::string_view text) {  // 004FB1F0
    std::string out(text);
    for (char& c : out) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 32);
    }
    return out;
}

} // namespace

std::uint8_t* library_view_class_id_004fa890(Raw view) {
    return library_view_entry(view, view.at<std::int32_t>(0)) + 0x48;
}

std::uint8_t* library_view_previous_004fa8f0(Raw view) {
    return library_view_entry(view, static_cast<std::int32_t>(view.at<float>(4)) - 1);
}

std::uint8_t* library_view_at_position_004fa960(Raw view) {
    return library_view_entry(view, static_cast<std::int32_t>(view.at<float>(4)));
}

std::uint8_t* library_view_current_004fa9c0(Raw view) { return library_view_entry(view, view.at<std::int32_t>(0)); }

std::uint8_t* library_view_next_004faa00(Raw view) {
    std::int32_t index = static_cast<std::int32_t>(view.at<float>(4)) + 1;
    const std::int32_t count = library_view_count(view);
    if (static_cast<std::uint32_t>(index + 1) > static_cast<std::uint32_t>(count)) index = count - 1;
    return library_view_entry(view, index);
}

void library_refresh_entry_detail_00510c20(Raw screen, Fe9LibraryDetailHost& host) {
    if (screen.ptr(0x508) == nullptr) return;
    const Raw view{screen.p + 0x434};
    const std::int32_t count = library_view_count(view);
    if (count == 0) {
        host.sleep_00ce2230(1);
        return;
    }
    if (screen.at<std::int32_t>(0x90) == 0) {
        std::uint32_t shown[3] = {1, 1, 1};
        if (count == 1) {
            shown[0] = 0;
            shown[2] = 0;
        } else {
            const std::int32_t at = static_cast<std::int32_t>(view.at<float>(4));
            if (at == 0) shown[0] = 0;
            else if (at == count - 1) shown[2] = 0;
        }
        host.widget_set_visible_34(screen.ptr(0x508), shown[0]);
        host.widget_set_visible_34(screen.ptr(0x50C), shown[1]);
        host.widget_set_visible_34(screen.ptr(0x510), shown[2]);
        struct Card {
            std::size_t card, back, front, extra;
            std::uint8_t* (*entry)(Raw);
        };
        static constexpr Card kCards[3] = {{0x508, 0x580, 0x57C, 0x584, library_view_previous_004fa8f0},
                                           {0x50C, 0x58C, 0x588, 0x590, library_view_at_position_004fa960},
                                           {0x510, 0x598, 0x594, 0x59C, library_view_next_004faa00}};
        static constexpr float kScale[2] = {1.0000100135803223f, 1.0000100135803223f};  // 00CEBDC8
        for (const Card& c : kCards) {
            if (!host.widget_virtual_38(screen.ptr(c.card))) continue;
            const Raw e{c.entry(view)};
            if (e.ptr(0) != nullptr) {
                host.gui_set_state_texture_uv_00ab2690(screen.ptr(c.back), 0, e.ptr(0), &e.at<float>(4));
                host.widget_set_scale_recursive_00aa86b0(screen.ptr(c.back), kScale);
            }
            if (e.ptr(0x14) != nullptr) {
                host.gui_set_state_texture_uv_00ab2690(screen.ptr(c.front), 0, e.ptr(0x14), &e.at<float>(0x18));
                host.widget_set_scale_recursive_00aa86b0(screen.ptr(c.front), kScale);
                if (screen.ptr(c.extra) != nullptr) {
                    host.gui_set_state_texture_uv_00ab2690(screen.ptr(c.extra), 0, e.ptr(0x14), &e.at<float>(0x18));
                    host.widget_set_scale_recursive_00aa86b0(screen.ptr(c.extra), kScale);
                }
            }
            const bool grey = screen.at<std::int32_t>(0x94) == 6 && Raw{c.entry(view)}.at<std::uint8_t>(0x46) == 0;
            host.widget_set_colour_50(screen.ptr(c.back), grey ? host.colour_static_grey_004f9d80()
                                                               : host.colour_static_white_004f9d30());
            host.widget_set_colour_50(screen.ptr(c.front), grey ? host.colour_static_grey_004f9d80()
                                                                : host.colour_static_white_004f9d30());
        }
    }

    void* const left = screen.ptr(0x574);
    void* const right = screen.ptr(0x578);
    if (screen.at<std::uint8_t>(0x560) != 0) {
        host.widget_set_visible_34(left, 1);
        host.widget_set_listener_fields_00aa6bc0(left, screen.p + 0x68, 0);
        host.widget_set_visible_34(right, 1);
        host.widget_set_listener_fields_00aa6bc0(right, screen.p + 0x68, 0);
        const auto arrow = [&host](void* widget, bool at_end) {
            if (at_end) {
                host.widget_set_state_88(widget, 3, 0, 1.0f);
                return;
            }
            const std::int16_t state = host.widget_state_94(widget);
            if (state < 1 || state > 2) host.widget_set_state_88(widget, 0, 0, 1.0f);
        };
        const std::int32_t index = view.at<std::int32_t>(0);
        arrow(left, index == 0);
        arrow(right, index == count - 1);
    } else {
        host.widget_set_visible_34(left, 0);
        host.widget_set_listener_fields_00aa6bc0(left, nullptr, 0);
        host.widget_set_visible_34(right, 0);
        host.widget_set_listener_fields_00aa6bc0(right, nullptr, 0);
    }

    static constexpr const char* kSep = "|. |####|";  // 00CEBDBC
    void* const text = screen.ptr(0xA8);
    const Raw current{library_view_current_004fa9c0(view)};
    const std::string name = upper_ascii(native_view(current, 0x28));
    switch (screen.at<std::int32_t>(0x94)) {
    case 0: {
        const Raw record = host.award_table_find_record_0050fc30(current.p + 0x28);
        const std::string award = upper_ascii(native_view(record, 0));
        if (current.at<std::uint8_t>(0x45) != 0 && Raw{library_view_at_position_004fa960(view)}.at<std::int32_t>(0x40) == 5) {
            host.gui_text_set_localised_cstring_00abbe50(text, "FE.achievement_secret_desc", 1);
            break;
        }
        const Raw again = host.award_table_find_record_0050fc30(current.p + 0x28);
        host.gui_text_set_localised_source_00abaed0(text, award + kSep + std::string(native_view(again, 0x0C)), 1);
        break;
    }
    case 1:
    case 3:
    case 8:
        host.gui_text_set_localised_source_00abaed0(text, name + kSep + std::string(native_view(current, 0x5C)), 1);
        break;
    case 2:
    case 5:
    case 6:
        screen.at<std::uint32_t>(0x4B0) = *reinterpret_cast<std::uint32_t*>(library_view_class_id_004fa890(view));
        host.gui_text_set_localised_source_00abaed0(
            text, name + kSep + std::string(native_view(current, 0x5C)) + kSep + std::string(native_view(current, 0x64)), 1);
        if (screen.at<std::int32_t>(0x90) == 1) host.library_preview_load_vehicle_model_0050aa10(screen);
        break;
    case 4: {
        const std::string command(native_view(current, 0x38));
        if (current.at<std::uint32_t>(0x38) != 0) {
            const std::int32_t failed = host.lua_machine_run_string_006b8ad0(command.c_str(), 0, 0, 2);
            host.debug_print_004254b0(failed != 0 ? "Nem sikerult vegrehajtani a: %s lua parancsot a Tutorialban."
                                                  : "%s lua parancs vegrehajtva a Tutorialban.",
                                      command.c_str());
        }
        std::string description(native_view(current, 0x5C));
        if (!host.input_config_load_suppressed_00f88a30()) {
            const std::string key = description + "_pc";
            if (!equals_case_insensitive(host.localization_resolve_narrow(key), key)) description = key;
        }
        host.gui_text_set_localised_source_00abaed0(text, name + kSep + description, 1);
        break;
    }
    default:
        break;
    }
}

void session_browser_build_create_options_005684d0(Raw screen, Fe9SessionCreateHost& host) {
    void* const label = screen.ptr(0x94);
    void* const value = screen.ptr(0xA4);
    const float step = screen.at<float>(0x260);
    const float y0 = host.widget_resolved_position_00aa6750(label)[1];
    const float x = host.widget_resolved_position_00aa6750(label)[0];
    const float z = host.widget_resolved_position_00aa6750(label)[2];
    float y = y0;
    const auto clone = [&](void* from, std::size_t store) {
        screen.ptr(store) = host.widget_clone_subtree_00aab4c0(from, screen.ptr(0x124));
        return screen.ptr(store);
    };
    const auto place = [&](std::size_t store) { host.widget_set_resolved_position_00aa8240(screen.ptr(store), {x, y, z}); };

    clone(label, 0xC0);
    place(0xC0);
    host.gui_text_set_localised_cstring_00abbe50(screen.ptr(0xC0), "FE.multi_createrankmin", 1);
    host.widget_set_visible_34(screen.ptr(0xC0), 1);
    y = step + y;
    clone(value, 0xDC);
    place(0xDC);
    clone(label, 0xC4);
    place(0xC4);
    host.gui_text_set_localised_cstring_00abbe50(screen.ptr(0xC4), "-", 1);
    host.widget_set_visible_34(screen.ptr(0xDC), 0);
    host.widget_set_visible_34(screen.ptr(0xC4), 1);
    y = step + y;
    clone(label, 0xC8);
    place(0xC8);
    host.gui_text_set_localised_cstring_00abbe50(screen.ptr(0xC8), "FE.multi_createrankmax", 1);
    host.widget_set_visible_34(screen.ptr(0xC8), 1);
    y = step + y;
    clone(value, 0xE0);
    clone(label, 0xCC);
    place(0xE0);
    place(0xCC);
    host.gui_text_set_localised_cstring_00abbe50(screen.ptr(0xCC), "-", 1);
    host.widget_set_visible_34(screen.ptr(0xE0), 0);
    host.widget_set_visible_34(screen.ptr(0xCC), 1);
    y = step + y;
    clone(label, 0xD4);
    place(0xD4);
    host.gui_text_set_localised_cstring_00abbe50(screen.ptr(0xD4), "FE.multi_createdroprate", 1);
    host.widget_set_visible_34(screen.ptr(0xD4), host.online_system_virtual_198());
    y = step + y;
    clone(value, 0xE4);
    clone(label, 0xD8);
    place(0xE4);
    place(0xD8);
    host.gui_text_set_localised_cstring_00abbe50(screen.ptr(0xD8), "-", 1);
    host.widget_set_visible_34(screen.ptr(0xE4), 0);
    host.widget_set_visible_34(screen.ptr(0xD8), host.online_system_virtual_198());
    host.scroller_set_scroll_range_006834a0(screen.p + 0x188, static_cast<float>(static_cast<double>(y) - 0.5009999871253967));
}
namespace {

// 00502728..00502950, the stores before the fog setters.
constexpr PreviewEnvironmentWrite kPreviewEnvironmentFog[] = {
    {0x0D4, 300.0f},  // 00CE3AE8
    {0x0D8, 10000.0f},  // 00CE3D64
    {0x0DC, 0.0f},
    {0x0E0, 20.0f},  // 00CE3930
    {0x080, 0.247099996f},  // 00CEB53C
    {0x084, 0.37650001f},  // 00CEB538
    {0x1B0, 0.5f},  // 00CE3800
    {0x088, 0.545099974f},  // 00CEB534
    {0x08C, 1.0f},  // 00D7A24C
    {0x094, 0.101000004f},  // 00CEB52C
    {0x090, 0.0715000033f},  // 00CEB530
    {0x098, 0.122699998f},  // 00CEB528
    {0x09C, 1.0f},  // 00D7A24C
    {0x0A0, 0.235300004f},  // 00CEB524
    {0x0A4, 0.239199996f},  // 00CE77F0
    {0x0A8, 0.188199997f},  // 00CEB520
    {0x0AC, 1.0f},  // 00D7A24C
    {0x0B0, 0.0689999983f},  // 00CEB51C
    {0x0D0, 1.0f},  // 00D7A24C
    {0x0B4, 0.0877000019f},  // 00CEB518
    {0x0B8, 0.0988000035f},  // 00CEB514
    {0x0BC, 1.0f},  // 00D7A24C
    {0x0C0, 0.0353000015f},  // 00CEB510
    {0x0C4, 0.0588000007f},  // 00CEB50C
    {0x0C8, 0.101999998f},  // 00CEB508
    {0x0CC, 1.0f},  // 00D7A24C
};

// 00502A3C..00503001, the stores after them (the image's order, duplicates included).
constexpr PreviewEnvironmentWrite kPreviewEnvironmentBlock[] = {
    {0x260, 500.0f},  // 00CE397C
    {0x264, 250.0f},  // 00CE77B0
    {0x270, 1.0f},  // 00D7A24C
    {0x274, 1.0f},  // 00D7A24C
    {0x268, 20.0f},  // 00CE3930
    {0x278, 1.0f},  // 00D7A24C
    {0x010, 100.0f},  // 00CE3D08
    {0x018, 100.0f},  // 00CE3D08
    {0x014, 0.0199999996f},  // 00CE9BAC
    {0x000, 1.0f},  // 00D7A24C
    {0x004, 1.0f},  // 00D7A24C
    {0x008, 0.0f},
    {0x154, 0.317600012f},  // 00CEB4EC
    {0x158, 0.317600012f},  // 00CEB4EC
    {0x15C, 0.317600012f},  // 00CEB4EC
    {0x1A0, 1.0f},  // 00D7A24C
    {0x1A4, 1.0f},  // 00D7A24C
    {0x1A8, 1.0f},  // 00D7A24C
    {0x144, 0.5f},  // 00CE3800
    {0x150, 0.5f},  // 00CE3800
    {0x164, 0.509800017f},  // 00CEB4E8
    {0x148, 0.00999999978f},  // 00D7A238
    {0x168, 0.57249999f},  // 00CEB4E4
    {0x14C, 5.0f},  // 00CE3850
    {0x16C, 0.600000024f},  // 00CE3D30
    {0x174, 0.196099997f},  // 00CEB4E0
    {0x178, 0.239199996f},  // 00CE77F0
    {0x17C, 0.262699991f},  // 00CEB4DC
    {0x104, 850.0f},  // 00CEB4D8
    {0x108, 50.0f},  // 00CEB4D4
    {0x10C, -200.0f},  // 00CE77E4
    {0x110, -190.0f},  // 00CEB4D0
    {0x114, 20.0f},  // 00CE3930
    {0x118, 0.0666999966f},  // 00CEB4CC
    {0x11C, 0.0863000005f},  // 00CEB4C8
    {0x120, 0.117600001f},  // 00CEB4C4
    {0x124, 0.699999988f},  // 00CE3E18
    {0x1B0, 0.5f},  // 00CE3800
    {0x1B4, 2.0f},  // 00CE3958
    {0x1B8, 10.0f},  // 00CE38B8
    {0x1BC, 50.0f},  // 00CEB4D4
    {0x1C0, 2.29999995f},  // 00CEB4C0
    {0x1C4, 70.0f},  // 00CE77B4
    {0x1C8, 20.0f},  // 00CE3930
    {0x1E4, 0.0f},
    {0x1E8, 0.0f},
    {0x1EC, 0.627499998f},  // 00CEB4BC
    {0x1D4, 0.200000003f},  // 00CE54A0
    {0x114, 0.200000003f},  // 00CE54A0
    {0x0DC, 0.0f},
    {0x0E0, 0.0f},
    {0x0E4, -120.0f},  // 00CE77F8
    {0x04C, 0.400000006f},  // 00CE7804
    {0x054, 25.0f},  // 00CE89CC
    {0x104, 850.0f},  // 00CEB4D8
    {0x058, 1.29999995f},  // 00CEB4B4
    {0x108, 0.5f},  // 00CE3800
    {0x02C, 0.300000012f},  // 00CE69C8
    {0x1CC, 12.0f},  // 00CEB4B8
    {0x10C, -200.0f},  // 00CE77E4
    {0x030, 100.0f},  // 00CE3D08
    {0x1D0, 1.5f},  // 00CE380C
    {0x110, -190.0f},  // 00CEB4D0
    {0x034, 0.600000024f},  // 00CE3D30
    {0x1D8, 2.0f},  // 00CE3958
    {0x1DC, 2.0f},  // 00CE3958
    {0x0D4, 500.0f},  // 00CE397C
    {0x038, 60.0f},  // 00CEB4B0
    {0x1E0, 1.0f},  // 00D7A24C
    {0x22C, 1.0f},  // 00D7A24C
    {0x230, 1.0f},  // 00D7A24C
    {0x0D0, 1.0f},  // 00D7A24C
    {0x0D8, 10000.0f},  // 00CE3D64
    {0x050, 1.0f},  // 00D7A24C
    {0x05C, 1.0f},  // 00D7A24C
    {0x03C, 3.0f},  // 00CE3854
    {0x040, 1.0f},  // 00D7A24C
};

} // namespace

void library_preview_setup_environment_00502480(Raw screen, Fe9PreviewEnvironmentHost& host) {
    if (screen.ptr(0x4B8) != nullptr) return;
    void* const ocean_storage = host.operator_new_00bf681b(0xB8);
    screen.ptr(0x4B8) =
        ocean_storage != nullptr ? host.ocean_environment_construct_0078daa0(ocean_storage, screen.ptr(0x0C), 0) : nullptr;
    alignas(8) std::uint8_t storage[kPreviewEnvironmentBlockSize];
    const Raw block{storage};
    host.scene_environment_block_construct_004cb420(block.p);
    host.native_string_assign(block.p + 0x208, "hdr_RCS_usn_04_Defend_G.dds");
    host.native_string_assign(block.p + 0x210, "hdr_RCS_usn_04_Defend_G_ocean.dds");
    for (const std::size_t off : {0x218u, 0x234u, 0x23Cu, 0x244u, 0x24Cu, 0x254u}) host.native_string_assign(block.p + off, "white.tga");
    host.world_parameter_set_string_00b1b830("ReflectionCubeMap", block.p + 0x208);
    host.world_parameter_set_string_00b1b830("StaticShadowTexture", block.p + 0x254);
    for (const PreviewEnvironmentWrite& w : kPreviewEnvironmentFog) block.at<float>(w.offset) = w.value;

    void* const camera = screen.ptr(0x10);
    const auto fog = [&]() { return raw(camera).ptr(0x184); };
    host.fog_set_color_00b84c40(fog(), &block.at<float>(0x80));
    for (std::uint32_t i = 0; i < 4; ++i) host.fog_set_directional_color_00b84fa0(fog(), &block.at<float>(0x90 + i * 0x10), i);
    host.camera_set_clear_color_00b6fe50(camera, host.colour_float_rgba_to_argb_004fb850(&block.at<float>(0x80)));
    host.fog_set_scalar_68_00b84d00(fog(), block.at<float>(0xD0));
    host.fog_set_scalar_6c_00b84d10(fog(), block.at<float>(0xD4));
    host.fog_set_scalar_70_00b84d20(fog(), block.at<float>(0xD8));
    host.fog_set_scalar_78_00b84d40(fog(), block.at<float>(0xDC));
    host.fog_set_scalar_74_00b84d30(fog(), block.at<float>(0xE0));

    host.native_string_assign(block.p + 0x280, "oceannormalmap04.tga");
    host.native_string_assign(block.p + 0x1C, "white.tga");
    host.native_string_assign(block.p + 0x24, "white.tga");
    block.at<std::uint8_t>(0x0C) = 0;
    for (const PreviewEnvironmentWrite& w : kPreviewEnvironmentBlock) block.at<float>(w.offset) = w.value;
    host.native_string_assign(block.p + 0x44, "");
    for (const std::size_t rgb : {0x154u, 0x1A0u, 0x164u, 0x174u, 0x118u, 0x1E4u}) host.linearize_rgb_004e5c00(&block.at<float>(rgb));
    host.ocean_environment_apply_0078c9b0(screen.ptr(0x4B8), block.p);

    const std::string sky = host.flag_00f88a0c() ? "HDR_sky_069" : "sky_069";
    void* const atmosphere_storage = host.operator_new_00bf681b(0x40);
    screen.ptr(0x4BC) = atmosphere_storage != nullptr
                            ? host.atmosphere_construct_00bbdff0(atmosphere_storage, screen.ptr(0x0C), sky)
                            : nullptr;
    host.sky_box_set_rotation_degrees_00bbf1a0(host.atmosphere_get_sky_box_00bbddb0(screen.ptr(0x4BC)), 70.0f);
    host.sky_box_set_luminance_00bbf200(host.atmosphere_get_sky_box_00bbddb0(screen.ptr(0x4BC)), 0.800000011920929f);
    host.resource_manager_virtual_120();
    host.scene_environment_block_destruct_004c6520(block.p);
}

// --- packet cc13-fe9c ------------------------------------------------------

void library_load_downloadable_classes_005021e0(Raw screen, Fe9cLibraryDlcHost& host) {
    void* const globals = host.lua_owner_globals_00b67980(host.game_lua_owner_1a0c());
    void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");
    host.lua_release_00b67700(globals);
    for (std::uint32_t i = 0;; ++i) {
        const auto* first = static_cast<std::uint8_t*>(screen.ptr(0x3F8));
        if (first == nullptr || i >= static_cast<std::uint32_t>((static_cast<std::uint8_t*>(screen.ptr(0x3FC)) - first) / 0x6C)) break;
        const Raw entry{const_cast<std::uint8_t*>(first) + i * 0x6C};
        std::string dlc = host.vehicle_class_unlock_dlc_minimum_004fd8d0(screen, entry.at<std::int32_t>(0x48));
        // 004CAD40 with an empty replacement resumes at the removal point.
        for (std::size_t at = dlc.find("DL_Content_"); at != std::string::npos; at = dlc.find("DL_Content_", at))
            dlc.erase(at, 11);
        const std::uint32_t shift = static_cast<std::uint32_t>(std::atol(dlc.c_str()) - 1) & 0xFF;  // CL of __allshl
        const std::uint64_t mask = shift >= 64 ? 0 : (std::uint64_t{1} << shift);
        // The image re-reads the entry pointer after the call.
        Raw{static_cast<std::uint8_t*>(screen.ptr(0x3F8)) + i * 0x6C}.at<std::uint8_t>(0x46) =
            (host.owned_dlc_mask_2060() & mask) == mask ? 1 : 0;
    }
    host.lua_release_00b67700(classes);
}

void library_read_vehicle_class_rows_0050a400(void* class_ids, void* types, void* flags, Fe9cVehicleRowHost& host) {
    void* const globals = host.lua_owner_globals_00b67980(host.game_lua_owner_1a0c());
    void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");
    host.lua_release_00b67700(globals);
    void* const key = host.lua_new_object_00b65f50();
    void* const value = host.lua_new_object_00b65f50();
    host.lua_iterate_first_00b67080(classes, key, value);
    while (!host.lua_is_unbound_00b66420(key)) {
        if (host.lua_is_table_00b661b0(value)) {
            void* const unlock = host.lua_get_by_name_00b67800(value, "Unlock");
            if (host.lua_is_boolean_00b66000(unlock) && host.lua_get_boolean_00b66250(unlock)) {
                void* const id = host.lua_get_by_name_00b67800(value, "UnlockID");
                bool open = true;
                if (!host.lua_is_nil_00b65fb0(id)) {
                    if (!host.lua_is_string_00b660a0(id)) {
                        open = false;
                    } else {
                        const char* const text = host.lua_get_string_00b662b0(id);
                        const std::string name = text != nullptr ? text : "";
                        if (!name.empty()) open = host.profile_is_unlock_satisfied_007fc4c0(name);
                    }
                }
                if (open) {
                    void* const race_probe = host.lua_get_by_name_00b67800(value, "Race");
                    const bool has_race = !host.lua_is_nil_00b65fb0(race_probe);
                    host.lua_release_00b67700(race_probe);
                    if (has_race) {
                        void* const race_object = host.lua_get_by_name_00b67800(value, "Race");
                        const std::int32_t race = host.lua_get_integer_00b66290(race_object);
                        host.lua_release_00b67700(race_object);
                        if (race > 0) {
                            host.int_vector_push_back_00442190(class_ids, host.lua_get_integer_00b66290(key));
                            void* const type = host.lua_get_by_name_00b67800(value, "Type");
                            const char* const type_text = host.lua_get_string_00b662b0(type);
                            const std::string type_name = type_text != nullptr ? type_text : "";
                            host.lua_release_00b67700(type);
                            host.string_vector_append_00450540(types, type_name);
                            host.int_vector_push_back_00442190(flags, race == 1 || race == 4 ? 1 : 0);
                        }
                    }
                }
                host.lua_release_00b67700(id);
            }
            host.lua_release_00b67700(unlock);
        }
        host.lua_iterate_next_00b67190(classes, key, value);
    }
    host.lua_release_00b67700(value);
    host.lua_release_00b67700(key);
    host.lua_release_00b67700(classes);
}

void library_preview_load_vehicle_model_0050aa10(Raw screen, Fe9cPreviewLoadHost& host) {
    if (screen.at<std::int32_t>(0x4B4) == screen.at<std::int32_t>(0x4B0)) return;
    host.library_preview_clear_rotors_0050a950(screen);
    const std::int32_t class_id = screen.at<std::int32_t>(0x4B0);
    screen.at<std::int32_t>(0x4B4) = class_id;
    if (class_id == -1) return;
    host.debug_print_004254b0("+ShowSelectedVehicle", 0);
    std::string pack = "mpak/Class_own_" + std::to_string(class_id) + ".mpak";
    if (!host.vfs_resolve_existing_00bdf4c0(pack)) {
        pack = "mpak/Class_" + std::to_string(class_id) + ".mpak";
        if (!host.vfs_resolve_existing_00bdf4c0(pack)) {
            host.debug_print_004254b0("ERROR:MPAK files not found for class %d!!!", class_id);
            pack = "giachievement_model_load_temp";
        }
    }
    host.debug_print_004254b0("+pak search over", 0);
    host.loading_queue_enqueue_005055c0(pack);
    host.loading_queue_set_last_callback(0x00509FB0);
    void* const globals = host.lua_owner_globals_00b67980(host.game_lua_owner_1a0c());
    void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");
    host.lua_release_00b67700(globals);
    void* const row = host.lua_get_by_index_00b67720(classes, class_id);
    void* const storage = host.operator_new_00bf681b(0x60);
    void* const model = storage != nullptr ? host.library_preview_model_construct_004fcfc0(storage) : nullptr;
    void* const mesh = host.lua_get_by_name_00b67800(row, "Mesh");
    host.native_string_assign_cstring_0041e350(raw(model).p + 8, host.lua_get_string_00b662b0(mesh));
    host.lua_release_00b67700(mesh);
    raw(model).at<std::uint32_t>(0x18) = 0;
    raw(model).at<std::int32_t>(0x1C) = class_id;
    host.loading_queue_submit_00504d20(raw(model).p + 8, model);
    host.lua_release_00b67700(row);
    host.lua_release_00b67700(classes);
}

void library_load_class_tables_00504130(Raw screen, Fe9cPreviewSceneHost& host) {
    host.simple_3d_scene_init_0093ccc0(screen.p + 8);
    void* const camera = screen.ptr(0x10);
    host.camera_set_render_mode_and_mask_00b6fdf0(camera, 0);
    host.camera_set_clear_flags_00b6fe10(camera, 7);
    void* const lights = raw(host.scene_lighting_owner_00b72110(screen.ptr(0x0C))).ptr(0x1C);
    const Raw light = raw(raw(raw(lights).ptr(0)).ptr(8));
    const std::array<float, 3> direction =
        host.lighting_direction_from_angles_004b4d80(0.4363323152065277f, 4.363323211669922f);  // 00CEB664 / 00CEB668
    for (int i = 0; i < 3; ++i) light.at<float>(0x1E0 + i * 4) = direction[i];
    const auto diffuse = [&host] {
        std::array<float, 4> colour{1.0f, 0.800000011920929f, 0.6157000064849854f, 1.0f};  // 00CE74F8, 00CEB660
        if (host.flag_00f88a0c()) host.linearize_rgb_004e5c00(colour.data());
        return colour;
    };
    const std::array<float, 4> base = diffuse();
    host.light_set_base_diffuse_004b62e0(light.p, base.data());
    const std::array<float, 4> stored = diffuse();
    for (int i = 0; i < 4; ++i) light.at<float>(0x1C4 + i * 4) = stored[i];
    host.light_set_diffuse_scale_00b7af90(light.p, 4.0f);     // 00CE3D34
    host.light_set_specular_scale_00b7b010(light.p, 35.0f);  // 00CE4D90

    void* const ambient = raw(host.scene_lighting_owner_00b72110(screen.ptr(0x0C))).ptr(0x10);
    std::array<float, 4> colour{0.23919999599456787f, 0.243149995803833f, 0.1764499992132187f, 0.25f};
    host.linearize_rgb_004e5c00(colour.data());
    colour[3] = 1.0f;
    host.ambient_set_00b7af20(ambient, colour.data());
    std::array<std::array<float, 4>, 6> faces = kPreviewAmbientFaces;
    for (std::uint32_t i = 0; i < 6; ++i) {
        std::array<float, 4>& face = faces[i];
        face[3] = static_cast<float>(face[3] * 1.5);  // 00CE3D78
        const double s = face[3];
        std::array<float, 4> scaled{static_cast<float>(face[0] * s), static_cast<float>(face[1] * s),
                                    static_cast<float>(face[2] * s), 1.0f};
        host.linearize_rgb_004e5c00(scaled.data());
        host.ambient_set_cube_face_00b7af60(ambient, scaled.data(), i);
    }
    host.render_device_virtual_04(host.flag_00f88a04());
    host.render_resources_initialize_00b107f0(host.flag_00f88a0c() ? 1 : 0, host.flag_00f88a0d(), host.dword_00f889d8());
    host.scene_renderer_set_hdr_params_00b0e9b0(&kPreviewHdrParams);
    void* const shadow = host.directional_light_create_shadow_owner_00a8fd30(light.p);
    host.directional_light_set_shadow_owner_00b7bdf0(light.p, shadow);
    host.ref_release(shadow);
    screen.at<std::uint32_t>(0x48C) = 0;
    host.lua_run_script_00b69d40("SCRIPTS/datatables/autoload/VehicleClasses.lua", 0);
    host.lua_run_script_00b69d40("SCRIPTS/datatables/autoload/DeviceClasses.lua", 0);
    screen.at<std::uint32_t>(0x4B0) = 1;
}

void library_fill_order_of_battle_row_00500a90(Raw screen, std::int32_t index, Fe9cOrderOfBattleRowHost& host) {
    const auto record = [&] { return Raw{static_cast<std::uint8_t*>(screen.ptr(0x460)) + index * 0x2C}; };
    host.library_find_entry_by_id_004fb090(screen, record().at<std::int32_t>(0));
    const std::string name = host.vehicle_class_name_or_no_unlock_004fda50(screen, record().at<std::int32_t>(0));
    void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0xAC), nullptr);
    void* const label = host.find_child_by_name_00aa7e00(row, "MenuItem_Text");
    host.widget_set_name_2c(row, std::to_string(index));
    raw(row).at<std::uint8_t>(0x77) = 0;
    host.gui_text_set_ellipsized_00abb000(label, name, -1.0f, 1);
    host.widget_set_visible_34(label, 1);
    void* const units = host.find_child_by_name_00aa7e00(row, "NumUnits_Text");
    host.widget_set_visible_34(units, 1);
    const std::int32_t count = record().at<std::int32_t>(8);
    if (count > 0) host.gui_text_set_localised_source_00abaed0(units, "x" + std::to_string(count), 1);
    else host.gui_text_set_localised_cstring_00abbe50(units, " ", 1);
    host.list_box_insert_row_00a9d750(screen.ptr(0xB8), row, 0, 0);
    if (record().at<std::int32_t>(8) == -1) {
        raw(row).at<std::uint8_t>(0x77) = 1;
        host.gui_text_set_localised_cstring_00abbe50(units, "", 1);
        raw(screen.ptr(0xB8)).at<std::uint8_t>(0x121) = 1;
    }
}

// --- packet cc13-fe11 ------------------------------------------------------

void front_end_show_difficulty_select_004ff6f0(Raw screen, Fe11DifficultyHost& host) {
    screen.at<std::int32_t>(0x94) = 7;
    screen.at<std::int32_t>(0x98) = 99;
    for (const std::size_t off : {0x508u, 0x50Cu, 0x510u, 0x12Cu, 0x130u, 0x128u, 0x574u})
        host.widget_set_visible_34(screen.ptr(off), 0);
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0x574), nullptr, 0);
    host.widget_set_visible_34(screen.ptr(0x578), 0);
    host.widget_set_listener_fields_00aa6bc0(screen.ptr(0x578), nullptr, 0);
    host.widget_set_visible_34(screen.ptr(0x56C), 0);
    host.widget_set_visible_34(screen.ptr(0x570), 0);
    for (std::size_t i = 0; i < 0x28; ++i) {
        host.widget_set_visible_34(screen.ptr(0x1D8 + i * 4), 0);
        host.widget_set_visible_34(screen.ptr(0x278 + i * 4), 0);
    }
    host.widget_set_visible_34(screen.ptr(0xA8), 0);
    host.widget_set_visible_34(screen.ptr(0x4FC), 0);
    host.widget_set_visible_34(screen.ptr(0x518), 1);
    host.frame_select_00518d60(0, 0, 0, "FE.main_difficulty_title");
    host.command_bar_set_help_line_0054a0c0("");
    host.rebuild_command_bar_0054b530({{
        {0xA2, "FE.play", 0},
        {0xB3, "globals.navigate", 1},
        {0, "", 1},
        {0xA3, "globals.back", 2},
        {0, "", 1},
    }});
    void* const list = screen.ptr(0xC4);
    host.list_box_clear_rows_00a9bec0(list);
    host.widget_set_visible_34(screen.ptr(0xB8), 0);
    raw(screen.ptr(0xB8)).at<std::uint8_t>(0x77) = 1;
    (void)host.widget_resolved_position_00aa6750(list); // overwritten by the 0.5 pair before any read
    host.widget_set_enabled_60(list, 1);
    const float half[2]{0.5f, 0.5f};
    const char* const keys[3]{"FE.easy", "FE.normal", "FE.hard"};
    for (int i = 0; i < 3; ++i) {
        void* const row = host.widget_clone_subtree_00aab4c0(screen.ptr(0xB0), 0);
        raw(row).at<std::uint8_t>(0x78) = 1;
        host.widget_set_listener_fields_00aa6bc0(row, screen.p + 0x68, 0);
        if (i == 0) raw(row).at<std::int32_t>(0x100) = 1;
        host.widget_virtual_30(row, half);
        host.gui_text_set_localised_cstring_00abbe50(row, keys[i], 1);
        raw(row).at<std::uint8_t>(0x77) = 0;
        if (i == 2) raw(row).at<std::int32_t>(0x100) = 1;
        host.list_box_insert_row_00a9d750(list, row, 0, 0);
        if (i == 1) raw(row).at<std::int32_t>(0x100) = 1;
    }
    host.widget_set_visible_34(list, 1);
    raw(list).at<std::uint8_t>(0x77) = 0;
    host.list_box_select_index_00a9c7c0(list, 1);
    host.list_box_scroll_by_00a9cc50(list, 1);
    host.list_box_scroll_by_00a9cc50(list, 2);
    host.list_box_reset_highlight_00a9cf30(list, 1, 1);
    host.list_box_select_index_00a9c7c0(list, host.profile_difficulty_6ac());
}

void front_end_preview_draw_00503510(Raw screen, Fe11PreviewDrawHost& host) {
    using Vec3 = Fe11PreviewDrawHost::Vec3;
    using Vec4 = Fe11PreviewDrawHost::Vec4;
    screen.at<float>(0x7C) = host.preview_distance_00e084f8();
    if (screen.at<std::int32_t>(0x90) == 0) return;
    const float d = screen.at<std::uint8_t>(0x4CC) != 0 ? 50.0f : 0.0f;
    const float v = screen.at<float>(0x7C);
    const float h = static_cast<float>(static_cast<double>(v) * 0.33333331346511841 + d);
    const Vec3 target{0.0f, static_cast<float>(d + 0.0), 0.0f};
    const Vec3 up{0.0f, 1.0f, 0.0f};
    void* const camera = screen.ptr(0x10);
    // The rotated eye, divided through by w (1/w is stored to float first).
    const auto eye = [&] {
        const float angle = -static_cast<float>(static_cast<double>(host.game_time_64c()) / 3.0);
        const Vec4 p = host.vector4_transform_00b62d10({v, h, 0.0f, 1.0f}, host.rotation_y_00b646e0(angle));
        const float r = static_cast<float>(1.0 / p[3]);
        return Vec3{p[0] * r, p[1] * r, p[2] * r};
    };
    host.camera_set_view_matrix_00b71490(camera, host.look_at_00b63f10(eye(), target, up));
    host.camera_set_field_of_view_00b6fbb0(camera, 0.785398185f);
    host.camera_set_near_plane_00b6fbf0(camera, 1.0f);
    host.camera_set_far_plane_00b6fc10(camera, 10000.0f);
    void* const gui_camera = raw(raw(screen.ptr(0x4DC)).ptr(0xF0)).ptr(0x18);
    const auto project_anchor = [&](std::size_t widget) {
        void* const node = raw(screen.ptr(widget)).ptr(0x4C);
        if ((raw(node).at<std::uint8_t>(0x5C) & 2) == 0) host.transform_refresh_world_matrix_00b6db70(node);
        const Vec4 in{raw(node).at<float>(0x120), raw(node).at<float>(0x124), raw(node).at<float>(0x128), 1.0f};
        const Vec4 p = host.vector4_transform_00b62d10(in, host.camera_view_projection_00b70490(gui_camera));
        return std::array<float, 2>{static_cast<float>(static_cast<double>(p[0]) / p[3]),
            static_cast<float>(static_cast<double>(p[1]) / p[3])};
    };
    const auto a = project_anchor(0x574);
    const auto b = project_anchor(0x578);
    const float cx = static_cast<float>((static_cast<double>(b[0]) + a[0]) * 0.5);
    const float cy = static_cast<float>((static_cast<double>(b[1]) + a[1]) * 0.5);
    const Vec4 t = host.vector4_transform_00b62d10({target[0], target[1], target[2], 1.0f},
        host.camera_view_projection_00b70490(camera));
    const float depth = static_cast<float>(static_cast<double>(t[2]) / t[3]);
    const Vec4 q = host.vector4_transform_00b62d10({cx, cy, depth, 1.0f},
        host.matrix_invert_general_00b632d0(host.camera_projection_00b6fcf0(camera)));
    const float qx = static_cast<float>(static_cast<double>(q[0]) / q[3]);
    const float qy = static_cast<float>(static_cast<double>(q[1]) / q[3]);
    const auto translation = [](float x, float y, float z) {
        return Fe11PreviewDrawHost::Mat4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1};
    };
    const auto shift = translation(qx, qy, 0.0f);
    const auto look = host.look_at_00b63f10(eye(), target, up);
    host.camera_set_view_matrix_00b71490(camera, host.matrix_multiply_00413920(look, shift));
    // The image re-checks the index against the vector before each of the three reads (00BF6713 on
    // failure); the loop condition already guarantees it.
    for (std::uint32_t i = 0;; ++i) {
        auto* const first = static_cast<void**>(screen.ptr(0x428));
        if (first == nullptr || i >= static_cast<std::uint32_t>(static_cast<void**>(screen.ptr(0x42C)) - first)) break;
        host.debug_print_004254b0("Rotor %d=0x%x", i, first[i]);
        const Vec3 pos = host.node_copy_local_position_00b6e0a0(first[i]);
        const float time = host.game_time_64c();
        const float spin = -static_cast<float>(static_cast<double>(time) + time);
        void* const rotor = static_cast<void**>(screen.ptr(0x428))[i];
        host.node_set_local_matrix_38(rotor,
            host.matrix_multiply_00413920(host.rotation_z_00b64780(spin), translation(pos[0], pos[1], pos[2])));
    }
    const std::int32_t width = host.display_width_24();
    const std::int32_t height = host.display_height_28();
    const auto size = host.camera_viewport_size(camera);
    if (width != size[0] || height != host.camera_viewport_size(camera)[1]) {
        if (screen.ptr(0x4B8) != nullptr) {
            host.object_delete_04(screen.ptr(0x4B8), 1);
            screen.ptr(0x4B8) = nullptr;
        }
        host.library_preview_setup_environment_00502480(screen);
        host.scene_renderer_set_hdr_params_00b0e9b0(&kPreviewHdrParams);
        for (std::uint32_t slot = 0; slot < 4; ++slot) host.render_resources_set_remap_texture(slot, "ColourRemap.tga");
    }
    host.camera_set_aspect_ratio_00b6fbd0(camera, host.display_aspect_10());
    const std::int32_t dims[2]{host.display_width_24(), host.display_height_28()};
    host.camera_viewport_set_dimensions(camera, dims);
    void* const lighting = host.scene_lighting_owner_00b72110(screen.ptr(0xC));
    void* const head = raw(lighting).ptr(0x1C);
    void* const first_light = raw(head).ptr(0);
    if (first_light == head) host.crt_invalid_parameter_00bf6713();
    host.shadow_map_update_view_from_camera_00a8f3b0(
        host.light_shadow_map_owner_00b7aab0(raw(first_light).ptr(8), camera));
    if (screen.ptr(0x4B8) != nullptr)
        host.environment_update_0078cff0(screen.ptr(0x4B8), camera, host.game_time_64c());
    host.scene_renderer_submit_scene_render_00b101c0(screen.ptr(0xC), camera);
}

void library_build_order_of_battle_list_00506660(Raw screen, Fe11OrderOfBattleListHost& host) {
    void* const list = screen.ptr(0xB8);
    host.list_box_clear_rows_00a9bec0(list);
    host.widget_set_visible_34(list, 1);
    std::vector<std::int32_t> type2, type3, type1;
    const auto records = [&] { return static_cast<std::uint8_t*>(screen.ptr(0x460)); };
    for (std::int32_t i = 0; records() != nullptr &&
         static_cast<std::uint32_t>(i) < static_cast<std::uint32_t>(
             (static_cast<std::uint8_t*>(screen.ptr(0x464)) - records()) / 0x2C); ++i) {
        switch (raw(records() + i * 0x2C).at<std::int32_t>(0x28)) {
        case 1: type1.push_back(i); break;
        case 2: type2.push_back(i); break;
        case 3: type3.push_back(i); break;
        default: break;
        }
    }
    for (const std::int32_t i : type2) host.library_fill_order_of_battle_row_00500a90(screen, i);
    for (const std::int32_t i : type3) host.library_fill_order_of_battle_row_00500a90(screen, i);
    for (const std::int32_t i : type1) host.library_fill_order_of_battle_row_00500a90(screen, i);
    const auto n2 = static_cast<std::int32_t>(type2.size());
    const auto n3 = static_cast<std::int32_t>(type3.size());
    if (n2 != 0) host.list_box_scroll_to_item_00a9d360(list, n2, 0.023f, 0);
    if (n3 != 0) host.list_box_scroll_to_item_00a9d360(list, n2 + n3, 0.023f, 0);
    const auto place_header = [&](std::size_t header, std::int32_t first_row) {
        void* const row = host.list_box_row_00a9be00(list, first_row);
        void* const text = host.find_child_by_name_00aa7e00(row, "MenuItem_Text");
        const auto p = host.widget_resolved_position_00aa6750(text);
        const float moved[3]{static_cast<float>(p[0] - 0.029999999329447746),
            static_cast<float>(p[1] - 0.054999999701976776), static_cast<float>(p[2] - 0.0)};
        host.widget_set_resolved_position_00aa8240(screen.ptr(header), moved);
        host.widget_set_visible_34(screen.ptr(header), 1);
    };
    if (n2 != 0) place_header(0x128, 0);
    if (n3 != 0) place_header(0x12C, n2);
    if (!type1.empty()) place_header(0x130, n2 + n3);
    host.list_box_reset_highlight_00a9cf30(list, 1, 1);
}

void library_open_entry_as_unlocks_0050e3c0(Raw screen, std::uintptr_t data, std::int32_t side,
    Fe11OpenEntryHost& host) {
    host.library_records_clear_0050a110(screen);
    Fe11VehicleClassRows rows = host.library_read_vehicle_class_rows_0050a400();
    Fe11LibraryRecord record;
    record.byte_0d = 1;
    record.byte_0c = 1;
    record.id = -10;
    record.count = -1;
    std::vector<std::string> list;
    static constexpr std::uint32_t kModeOffsets[8]{0x1CC, 0x1DC, 0x1EC, 0x1FC, 0x18C, 0x1AC, 0x19C, 0x1BC};
    const std::int32_t mode = host.game_effective_mode_004bca50();
    if (mode >= 0 && mode <= 7)
        list = host.string_vector_copy_00506c80(data, static_cast<std::uint32_t>(side) * 0x154 + kModeOffsets[mode]);
    if (host.game_effective_mode_004bca50() == 4) {
        std::string wanted;
        bool heavy_wanted = true;
        switch (host.selected_class_00e1956c()) {
        case 0: wanted = "Destroyer"; break;
        case 1: wanted = "Cruiser"; heavy_wanted = false; break;
        case 2: wanted = "Cruiser"; break;
        case 3: wanted = "BattleShip"; break;
        case 4: wanted = "TorpedoBoat"; break;
        case 5: wanted = "Submarine"; break;
        case 6: wanted = "Fighter"; break;
        default: break;
        }
        for (std::size_t i = 0; i < list.size(); ++i) {
            void* const globals = host.lua_owner_globals_00b67980(host.game_lua_owner_1a0c());
            void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");
            host.lua_release_00b67700(globals);
            void* const entry = host.lua_get_by_index_00b67720(classes, host.crt_atol_00bf8417(list[i].c_str()));
            void* const type_object = host.lua_get_by_name_00b67800(entry, "Type");
            const std::string type = host.lua_get_string_00b662b0(type_object); // not null-checked in the image
            host.lua_release_00b67700(type_object);
            bool match = equals_case_insensitive(type, wanted);
            if (match && !wanted.empty() && equals_case_insensitive(wanted, "Cruiser")) {
                void* const flag = host.lua_get_by_name_00b67800(entry, "HeavyCruiser");
                const bool heavy = host.lua_is_boolean_00b66000(flag) && host.lua_get_boolean_00b66250(flag);
                host.lua_release_00b67700(flag);
                match = heavy == heavy_wanted;
            }
            if (match) {
                const std::int32_t id = host.crt_atol_00bf8417(list[i].c_str());
                list.clear();
                list.push_back(host.native_string_from_int_004260b0(id));
            }
            host.lua_release_00b67700(entry);
            host.lua_release_00b67700(classes);
            if (match) break;
        }
    }
    record.ids.push_back(-1);
    for (const std::string& listed : list) {
        const std::int32_t n = host.crt_atol_00bf8417(listed.c_str());
        const std::string type = host.vehicle_class_type_004fd7c0(screen, n);
        for (std::size_t j = 0; j < rows.types.size(); ++j) {
            if (!equals_case_insensitive(rows.types[j], type)) continue;
            const bool heavy = host.vehicle_class_is_heavy_cruiser_004fab40(screen, n);
            if (host.vehicle_class_is_heavy_cruiser_004fab40(screen, rows.ids[j]) != heavy) continue;
            if (rows.flags[j] != side) continue;
            if (host.library_find_entry_by_id_004fb090(screen, rows.ids[j]) == nullptr) continue;
            record.ids.push_back(rows.ids[j]);
            rows.ids[j] = -10;
            rows.flags[j] = -10;
            rows.types[j] = "0_0_0";
        }
    }
    host.library_records_push_0050e110(screen, record);
}

void library_open_entry_as_order_of_battle_0050ef80(Raw screen, std::uintptr_t data, std::int32_t side,
    Fe11OrderOfBattleOpenHost& host) {
    host.library_battle_records_clear_0050a110(screen);
    const Fe11VehicleClassRows rows = host.library_read_vehicle_class_rows_0050a400();
    std::vector<Fe11LibraryRecord> satisfied, locked;
    const std::uintptr_t block = data + static_cast<std::uintptr_t>(side) * 0x154;
    // Ids of the class rows sharing `type` with record id `id` (rows are left as they are).
    const auto add_related = [&](Fe11LibraryRecord& record, const std::string& type) {
        for (std::size_t j = 0; j < rows.types.size(); ++j) {
            if (!equals_case_insensitive(rows.types[j], type) || rows.flags[j] != side || rows.ids[j] == record.id)
                continue;
            if (host.library_find_entry_by_id_004fb090(screen, rows.ids[j]) != nullptr) record.ids.push_back(rows.ids[j]);
        }
    };
    const auto counts = host.string_vector_at(block + 0x170);
    const auto entries = host.string_vector_at(block + 0x160);
    const auto texts = host.string_vector_at(block + 0x180);
    for (std::size_t k = 0; k < counts.size(); ++k) {
        Fe11LibraryRecord record;
        record.byte_0d = 1;
        record.type = 1;
        record.id = host.crt_atol_00bf8417(entries[k * 2].c_str());
        if (!host.profile_unlock_satisfied_007fc4c0(entries[k * 2 + 1])) {
            record.byte_0c = 0;
            record.byte_0d = 0;
            record.count = -1;
            record.text = texts.empty() ? std::string() : texts[k];
            locked.push_back(record);
            continue;
        }
        record.byte_0c = host.library_order_of_battle_contains_004fc840(data, side, record.id) ? 1 : 0;
        record.byte_0d = 1;
        record.count = host.crt_atol_00bf8417(counts[k].c_str());
        record.text = texts.empty() ? std::string() : texts[k] + "_reached";
        const std::string type = host.vehicle_class_type_004fd7c0(screen, record.id);
        if (record.byte_0c != 0) add_related(record, type);
        satisfied.push_back(record);
    }
    const auto changes = host.string_vector_at(block + 0x130);
    const auto change_counts = host.string_vector_at(block + 0x140);
    for (std::size_t k = 0; k < changes.size(); ++k) {
        Fe11LibraryRecord record;
        record.byte_0d = 1;
        record.id = host.crt_atol_00bf8417(changes[k].c_str());
        record.byte_0c = host.library_order_of_battle_contains_004fc840(data, side, record.id) ? 1 : 0;
        record.count = host.crt_atol_00bf8417(change_counts[k].c_str());
        if (record.byte_0c == 0) {
            record.type = 2;
            record.text = "fixed";
        } else {
            add_related(record, host.vehicle_class_type_004fd7c0(screen, record.id));
            record.type = 3;
            record.text = "change";
        }
        host.library_battle_records_push_0050e110(screen, record);
    }
    for (const Fe11LibraryRecord& record : satisfied) host.library_battle_records_push_0050e110(screen, record);
    for (const Fe11LibraryRecord& record : locked) host.library_battle_records_push_0050e110(screen, record);
}

void library_update_tutorial_movie_00515050(Raw screen, float rate, Fe11CarouselStepHost& host) {
    auto* const entries = static_cast<std::uint8_t*>(screen.ptr(0x448));
    if (entries == nullptr) return;
    const auto count = static_cast<std::uint32_t>((static_cast<std::uint8_t*>(screen.ptr(0x44C)) - entries) / 0x6C);
    if (count == 0) return;
    const float target = static_cast<float>(screen.at<std::int32_t>(0x434));
    const float current = screen.at<float>(0x438);
    if (target == current) return;
    const float delta = static_cast<float>(static_cast<double>(target) - current);
    const float distance = delta > 0.0f ? delta : -0.0f - delta;
    if (distance < 0.002f) return;
    const double step = static_cast<double>(rate) * (static_cast<double>(target) - current);
    const float position = static_cast<float>(step + step + current);
    screen.at<float>(0x438) = position;
    const float f = static_cast<float>(static_cast<double>(position) - static_cast<std::int32_t>(position));
    host.library_refresh_entry_detail_00510c20(screen);
    const float spacing = screen.at<float>(0x43C);
    const float middle = static_cast<float>(static_cast<double>(screen.at<float>(0x440)) - static_cast<double>(spacing) * f);
    host.widget_set_local_x_00aa78d0(screen.ptr(0x508), static_cast<float>(static_cast<double>(middle) - spacing));
    host.widget_set_local_x_00aa78d0(screen.ptr(0x50C), middle);
    host.widget_set_local_x_00aa78d0(screen.ptr(0x510), static_cast<float>(static_cast<double>(middle) + screen.at<float>(0x43C)));
    if (f > 0.5f) {
        const float half[2]{0.5f, 0.5f};
        const float ff[2]{f, f};
        host.widget_virtual_48(screen.ptr(0x508), half);
        host.widget_virtual_48(screen.ptr(0x50C), half);
        host.widget_virtual_48(screen.ptr(0x510), ff);
    } else if (f < 0.5f && f > 0.0f) {
        host.widget_set_uniform_scale_004fb9e0(screen.ptr(0x508), 0.5f);
        host.widget_set_uniform_scale_004fb9e0(screen.ptr(0x50C), static_cast<float>(1.0 - f));
        host.widget_set_uniform_scale_004fb9e0(screen.ptr(0x510), 0.5f);
    } else if (!(f > -0.5f && f < 0.0f)) {
        host.widget_set_uniform_scale_004fb9e0(screen.ptr(0x508), 0.5f);
        host.widget_set_uniform_scale_004fb9e0(screen.ptr(0x50C), 1.0f);
        host.widget_set_uniform_scale_004fb9e0(screen.ptr(0x510), 0.5f);
    }
    host.widget_set_visible_34(screen.ptr(0xA8), 1);
    if (screen.ptr(0x508) != nullptr && screen.ptr(0x50C) != nullptr && screen.ptr(0x510) != nullptr &&
        screen.at<std::int32_t>(0x94) == 8) {
        void* frames[3];
        void* icons[3];
        for (int i = 0; i < 3; ++i) frames[i] = host.find_child_by_name_00aa7e00(screen.ptr(0x508 + i * 4), "keret_Framebox", 1);
        for (int i = 0; i < 3; ++i) icons[i] = host.find_child_by_name_00aa7e00(screen.ptr(0x508 + i * 4), "icon_big_BG_Icon", 0);
        std::array<float, 3> p[3];
        for (int i = 0; i < 3; ++i) p[i] = host.widget_resolved_position_00aa6750(icons[i]);
        for (int i = 0; i < 3; ++i) {
            const float moved[3]{p[i][0] + 0.0f, p[i][1] + 0.0f, static_cast<float>(p[i][2] + 3.0)};
            host.widget_set_resolved_position_00aa8240(frames[i], moved);
        }
    }
    const Raw view{screen.p + 0x434};
    const std::int32_t kind = screen.at<std::int32_t>(0x94);
    const auto trunc_position = [&] { return static_cast<std::int32_t>(screen.at<float>(0x438)); };
    if (kind == 0 || kind == 3) {
        const std::size_t arrows = kind == 0 ? 0x540 : 0x54C;
        if (trunc_position() - 1 > -1)
            host.widget_set_visible_34(screen.ptr(arrows), raw(library_view_previous_004fa8f0(view)).at<std::uint8_t>(0x45) != 0);
        host.widget_set_visible_34(screen.ptr(arrows + 4), raw(library_view_at_position_004fa960(view)).at<std::uint8_t>(0x45) != 0);
        const auto size = static_cast<std::uint32_t>((static_cast<std::uint8_t*>(screen.ptr(0x44C)) - entries) / 0x6C);
        if (static_cast<std::uint32_t>(static_cast<std::int64_t>(screen.at<float>(0x438)) + 1) <= size)
            host.widget_set_visible_34(screen.ptr(arrows + 8), raw(library_view_next_004faa00(view)).at<std::uint8_t>(0x45) != 0);
        if (kind == 3) return;
        if (raw(library_view_at_position_004fa960(view)).at<std::int32_t>(0x40) == 5 &&
            raw(library_view_current_004fa9c0(view)).at<std::uint8_t>(0x45) != 0)
            host.gui_text_set_localised_cstring_00abbe50(screen.ptr(0xA8), "FE.achievement_secret_desc", 1);
        return;
    }
    if (kind != 4) return;
    void* const left = screen.ptr(0x51C);
    void* const centre = screen.ptr(0x520);
    void* const right = screen.ptr(0x524);
    const auto movie_path = [&](std::int32_t index) {
        return "movies/Tutorials/" + host.native_string_text(entries + index * 0x6C + 0x30) + ".bik";
    };
    const std::int32_t index = trunc_position();
    if (index == screen.at<std::int32_t>(0x434)) {
        const std::string path = movie_path(static_cast<std::int32_t>(static_cast<std::int64_t>(screen.at<float>(0x438))));
        if (screen.at<std::int32_t>(0x558) == trunc_position() && host.gui_movie_is_playing_00aac8e0(centre)) return;
        host.gui_movie_stop_00aac870(centre);
        host.gui_movie_set_source_00aae320(centre, path, 1);
        host.gui_movie_start_00ab0eb0(centre);
        host.widget_set_visible_34(centre, 1);
        host.widget_set_visible_34(screen.ptr(0x52C), 0);
        screen.at<std::int32_t>(0x558) = trunc_position();
        host.gui_movie_set_callback_7c(centre, 0x004FAAB0);
        host.gui_movie_forward_player_slot10_00aac8c0(left);
        host.widget_set_visible_34(left, 0);
        host.widget_set_visible_34(screen.ptr(0x528), 1);
        host.gui_movie_forward_player_slot10_00aac8c0(right);
        host.widget_set_visible_34(right, 0);
        host.widget_set_visible_34(screen.ptr(0x530), 1);
    } else if (index + 1 == screen.at<std::int32_t>(0x434)) {
        const std::string path = movie_path(static_cast<std::int32_t>(static_cast<std::int64_t>(screen.at<float>(0x438))) + 1);
        if (screen.at<std::int32_t>(0x55C) == trunc_position() + 1 && host.gui_movie_is_playing_00aac8e0(right)) return;
        host.gui_movie_stop_00aac870(right);
        host.gui_movie_set_source_00aae320(right, path, 1);
        host.gui_movie_start_00ab0eb0(right);
        host.widget_set_visible_34(right, 1);
        host.widget_set_visible_34(screen.ptr(0x530), 0);
        screen.at<std::int32_t>(0x55C) = trunc_position() + 1;
        host.gui_movie_set_callback_7c(right, 0x004FAAB0);
        host.gui_movie_forward_player_slot10_00aac8c0(left);
        host.widget_set_visible_34(left, 0);
        host.widget_set_visible_34(screen.ptr(0x528), 1);
        host.gui_movie_forward_player_slot10_00aac8c0(centre);
        host.widget_set_visible_34(centre, 0);
        host.widget_set_visible_34(screen.ptr(0x52C), 1);
    } else {
        host.gui_movie_forward_player_slot10_00aac8c0(left);
        host.widget_set_visible_34(screen.ptr(0x528), 1);
        host.widget_set_visible_34(left, 0);
        host.gui_movie_forward_player_slot10_00aac8c0(centre);
        host.widget_set_visible_34(screen.ptr(0x52C), 1);
        host.widget_set_visible_34(centre, 0);
        host.gui_movie_forward_player_slot10_00aac8c0(right);
        host.widget_set_visible_34(screen.ptr(0x530), 1);
        host.widget_set_visible_34(right, 0);
    }
}

void library_preview_on_model_loaded_005072c0(Raw screen, void* resource, Raw job,
    Fe11PreviewModelLoadedHost& host) {
    using Mat4 = Fe11PreviewModelLoadedHost::Mat4;
    host.debug_print_004254b0("ModelLoadedCallback actdata: \"%s\" 0x%x,0x%x readedmodel=0x%x");
    job.ptr(0x10) = resource;
    void* const instance = host.unit_part_instance_new_007135c0(job.ptr(0x10));
    void* const root = raw(raw(instance).ptr(0x160)).ptr(0x0C);
    job.ptr(0x14) = instance;
    host.model_groups_select_visible_00710bb0(instance, 0);
    const float far_scale[3]{100000.0f, 100000.0f, 100000.0f};
    for (std::int32_t i = 0; i < 0x14; ++i) {
        const std::string name = host.format_string128_004f9a80("fizika_%02d", i);
        for (void* const node : host.node_collect_descendants_00b6f9a0(root, name)) {
            const char* const text = host.node_name_00b6d800(node);
            if (text != nullptr && std::strstr(text, "ep_fizika") != nullptr) continue;
            host.scene_node_set_visibility_00b6da70(node, 0.0f, 0);
            host.node_set_scale_2c(node, far_scale);
        }
    }
    void* const model = raw(instance).ptr(0x160);
    for (void* const node : host.model_collect_children_by_prefix_0071ba20(model, "hajobelso")) {
        host.scene_node_set_visibility_00b6da70(node, 0.0f, 0);
        host.node_set_scale_2c(node, far_scale);
    }
    static constexpr const char* kRotors[5][3]{
        {"rotor", "rotor_still", "rotor_still_dam"},
        {"rotor0", "rotor_still0", "rotor_still_dam0"},
        {"rotor1", "rotor_still1", "rotor_still_dam1"},
        {"rotor2", "rotor_still2", "rotor_still_dam2"},
        {"rotor3", "rotor_still3", "rotor_still_dam3"},
    };
    for (const auto& names : kRotors) {
        if (void* const rotor = host.model_find_note_node_0071ad50(model, names[0])) {
            host.scene_node_set_visibility_00b6da70(rotor, 1.0f, 0);
            host.rotor_vector_push(screen, rotor);
            auto* const first = static_cast<void**>(screen.ptr(0x428));
            auto* const last = static_cast<void**>(screen.ptr(0x42C));
            host.debug_print_rotor_004254b0(first == nullptr ? -1 : static_cast<std::int32_t>(last - first) - 1, last[-1]);
        }
        for (int k = 1; k < 3; ++k)
            if (void* const still = host.model_find_note_node_0071ad50(model, names[k]))
                host.scene_node_set_visibility_00b6da70(still, 0.0f, 0);
    }
    (void)host.node_collect_descendants_00b6f9a0(root, "");  // walked with bounds checks only
    host.node_propagate_root_registration_00b6d890(root, screen.ptr(0x0C));
    (void)host.crt_atol_00bf8417(host.hud_list_selected_text_00a9ca00());
    const auto vehicle_root = [&] { return raw(raw(screen.ptr(0x48C)).ptr(0x160)).ptr(0x0C); };
    const std::int32_t mode = job.at<std::int32_t>(0x18);
    if (mode == 0) {
        screen.ptr(0x48C) = instance;
        host.model_instance_set_numbering_00711be0(instance, 0);
        void* const globals = host.lua_owner_globals_00b67980(host.game_lua_owner_1a0c());
        void* const classes = host.lua_get_by_name_00b67800(globals, "VehicleClass");
        host.lua_release_00b67700(globals);
        void* const entry = host.lua_get_by_index_00b67720(classes, job.at<std::int32_t>(0x1C));
        void* const globals2 = host.lua_owner_globals_00b67980(host.game_lua_owner_1a0c());
        void* const devices = host.lua_get_by_name_00b67800(globals2, "DeviceClass");
        host.lua_release_00b67700(globals2);
        void* const type_object = host.lua_get_by_name_00b67800(entry, "Type");
        const std::string type = host.lua_string_or_default_00b685c0(type_object, "unknown");
        host.lua_release_00b67700(type_object);
        bool ship = false;
        for (const char* const name : {"MotherShip", "BattleShip", "Cruiser", "Cargo", "Destroyer", "TorpedoBoat",
                 "LandingShip", "Submarine"})
            ship = ship || equals_case_insensitive(type, name);
        screen.at<std::uint8_t>(0x4CC) = ship ? 0 : 1;
        host.debug_print_type_004254b0(type.c_str(), ship ? "Ship" : "NotShip!");
        const bool not_ship = screen.at<std::uint8_t>(0x4CC) != 0;
        job.at<float>(0x54) = not_ship ? 50.0f : 0.0f;
        const float radius = host.node_bounds_44(vehicle_root())[3];
        screen.at<float>(0x84) = static_cast<float>(static_cast<double>(radius) * (not_ship ? 3.0 : 1.5));
        if (not_ship) {
            const auto bounds = host.node_bounds_44(vehicle_root());
            const Mat4 placement{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -0.0f - bounds[0], 50.0f, -0.0f - bounds[2], 1};
            for (std::size_t i = 0; i < 16; ++i) job.at<float>(0x20 + i * 4) = placement[i];
        }
        if (equals_case_insensitive(type, "Submarine") && !type.empty())
            for (void* const node : host.node_collect_descendants_00b6f9a0(vehicle_root(), "water"))
                host.scene_node_set_visibility_00b6da70(node, 0.0f, 0);
        for (void* const node : host.node_collect_descendants_00b6f9a0(vehicle_root(), "szarny letorik"))
            host.scene_node_set_visibility_00b6da70(node, 0.0f, 0);
        void* const distance_probe = host.lua_get_by_name_00b67800(entry, "UnitlibViewDistance");
        const bool has_distance = host.lua_is_number_00b66050(distance_probe);
        host.lua_release_00b67700(distance_probe);
        if (has_distance) {
            void* const distance = host.lua_get_by_name_00b67800(entry, "UnitlibViewDistance");
            screen.at<float>(0x7C) = static_cast<float>(host.lua_get_number_00b66270(distance));
            host.lua_release_00b67700(distance);
        } else {
            screen.at<float>(0x7C) = screen.at<float>(0x84);
        }
        host.set_preview_distance_00e084f8(screen.at<float>(0x7C));
        void* const platforms = host.lua_get_by_name_00b67800(entry, "Platforms");
        if (host.lua_is_table_00b661b0(platforms)) {
            void* const key = host.lua_new_object_00b65f50();
            void* const value = host.lua_new_object_00b65f50();
            host.lua_iterate_first_00b67080(platforms, key, value);
            while (!host.lua_is_unbound_00b66420(platforms, key)) {
                if (host.lua_is_integer_00b66a60(key)) {
                    bool bad = true;
                    void* const gun = host.lua_get_by_name_00b67800(value, "Gun");
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
                                job.ptr(0x10), "slot", host.lua_get_integer_00b66290(key)))};
                            const auto* const points = group.p == nullptr ? nullptr : static_cast<const float*>(group.ptr(0x48));
                            if (points != nullptr &&
                                (static_cast<const std::uint8_t*>(group.ptr(0x4C)) - reinterpret_cast<const std::uint8_t*>(points)) / 0x0C > 2) {
                                Mat4 basis{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
                                basis[12] = points[0];
                                basis[13] = points[1];
                                basis[14] = points[2];
                                for (int c = 0; c < 3; ++c) basis[8 + c] = points[6 + c] - points[c];
                                const float side[3]{points[3] - points[0], points[4] - points[1], points[5] - points[2]};
                                const float ahead[3]{points[6] - points[0], points[7] - points[1], points[8] - points[2]};
                                const auto up = host.vector3_cross_004f9b30(ahead, side);
                                basis[4] = up[0];
                                basis[5] = up[1];
                                basis[6] = up[2];
                                host.matrix_orthonormalize_basis_rows_0085dc80(basis.data());
                                float yaw = 0.0f;
                                void* const rest_probe = host.lua_get_by_name_00b67800(value, "RestAngles");
                                const bool has_rest = host.lua_is_table_00b661b0(rest_probe);
                                host.lua_release_00b67700(rest_probe);
                                if (has_rest) {
                                    void* const rest = host.lua_get_by_name_00b67800(value, "RestAngles");
                                    yaw = host.lua_get_vector2_00b679f0(rest)[0];
                                    host.lua_release_00b67700(rest);
                                } else {
                                    bool has_window = false;
                                    void* const windows = host.lua_get_by_name_00b67800(value, "Windows");
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
                                        void* const low = host.lua_get_by_name_00b67800(window, "MinHorzAngle");
                                        void* const high = host.lua_get_by_name_00b67800(window, "MaxHorzAngle");
                                        const double high_value = host.lua_get_number_00b66270(high);
                                        yaw = static_cast<float>((host.lua_get_number_00b66270(low) + high_value) * 0.5);
                                        host.lua_release_00b67700(high);
                                        host.lua_release_00b67700(low);
                                        void* const vlow = host.lua_get_by_name_00b67800(window, "MinVertAngle");
                                        void* const vhigh = host.lua_get_by_name_00b67800(window, "MaxVertAngle");
                                        (void)host.lua_get_number_00b66270(vhigh);
                                        (void)host.lua_get_number_00b66270(vlow);
                                        host.lua_release_00b67700(vhigh);
                                        host.lua_release_00b67700(vlow);
                                        host.lua_release_00b67700(window);
                                    }
                                }
                                const Raw turret{static_cast<std::uint8_t*>(host.preview_job_new_004fcfc0())};
                                void* const device_row = host.lua_get_by_index_00b67720(devices, device);
                                void* const device_mesh = host.lua_get_by_name_00b67800(device_row, "Mesh");
                                host.native_string_assign(turret.p + 8, host.lua_get_string_00b662b0(device_mesh));
                                host.lua_release_00b67700(device_mesh);
                                host.lua_release_00b67700(device_row);
                                turret.at<std::int32_t>(0x18) = 1;
                                const Mat4 placed = host.matrix_multiply_00413920(host.rotation_y_00b646e0(-0.0f - yaw), basis);
                                for (std::size_t i = 0; i < 16; ++i) turret.at<float>(0x20 + i * 4) = placed[i];
                                host.loading_queue_submit_00504d20(turret.p + 8, turret.p);
                            }
                        }
                    }
                }
                host.lua_iterate_next_00b67190(platforms, key, value);
            }
            host.lua_release_00b67700(value);
            host.lua_release_00b67700(key);
        }
        if (screen.at<std::uint8_t>(0x4CC) != 0) {
            bool found = true;
            const float frame = host.lua_resolve_float_or_default(entry, "PartAnims.Wings.1", 0.0f, found);
            if (found) host.node_apply_animation_00b6e110(root, "default", frame);
        }
        host.lua_release_00b67700(platforms);
        host.lua_release_00b67700(devices);
        host.lua_release_00b67700(entry);
        host.lua_release_00b67700(classes);
    } else if (mode == 1) {
        host.node_set_parent_00b6e680(root, vehicle_root());
        for (void* const node : host.node_collect_descendants_00b6f9a0(vehicle_root(), "underwater"))
            host.scene_node_set_visibility_00b6da70(node, 0.0f, 0);
    }
    host.node_set_local_matrix_38(root, &job.at<float>(0x20));
    host.ref_array_push_00504870(Raw{screen.p + 0x5C4}, job.p);
    if (host.interlocked_decrement(&job.at<std::int32_t>(4)) == 0) host.object_destroy_00(job.p);
    const float grey[4]{0.5f, 0.5f, 0.5f, 1.0f};
    host.model_effect_diffuse_00b85200(vehicle_root(), "gun", grey);
    void* const vehicle_model = raw(screen.ptr(0x48C)).ptr(0x160);
    if (void* const inner = host.model_find_note_node_0071ad50(vehicle_model, "belsocockpit"))
        host.scene_node_set_visibility_00b6da70(inner, 0.0f, 0);
    if (void* const outer = host.model_find_note_node_0071ad50(raw(screen.ptr(0x48C)).ptr(0x160), "kulsocockpit"))
        host.scene_node_set_visibility_00b6da70(outer, 1.0f, 0);
}

// Packet cc13-trivialslots: the kTrivialSlots bodies; evidence and ABI per routine
// are in the header.
void gui_widget_listener_empty1c_004fa170(Raw) noexcept {}
std::int32_t tactical_library_screen_get_screen_id_00509d60(Raw) noexcept { return 0xB; }
void tactical_library_widget_listener_current1c_00509d70(Raw self, Fe3Host& host) {
    host.scroller_recapture_006833f0(self.p + 0x64);
}
std::int32_t in_game_screen23_get_screen_id_00517680(Raw) noexcept { return 0x23; }
void in_game_screen23_collect_children_00517690(Raw) noexcept {}
void in_game_screen23_update_005178a0(Raw) noexcept {}
std::int32_t front_end_frame_get_screen_id_00517d30(Raw) noexcept { return 0xC; }
bool front_end_frame_is_self_managed_00517d40(Raw) noexcept { return true; }
std::int32_t hud_bomber_screen_get_screen_id_00519450(Raw) noexcept { return 0x25; }
std::int32_t control_binding_screen_screen_id_00555800(Raw) noexcept { return 0xF; }
void leaderboards_screen_on_listbox_activate_no_op_005610d0(Raw) noexcept {}
std::int32_t leaderboards_screen_get_screen_id_005633f0(Raw) noexcept { return 0xA; }
std::int32_t hud_limbo_screen_get_screen_id_00565d60(Raw) noexcept { return 0x32; }

} // namespace bsp::cc13_fe3