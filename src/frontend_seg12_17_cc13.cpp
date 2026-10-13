// Segments 12 and 17 front-end reconstructions, packet cc13-fe3 (docs/CC13_FE_3.md).
#include "bsp/frontend_seg12_17_cc13.hpp"

#include <cstdio>
#include <cstring>

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

} // namespace bsp::cc13_fe3
