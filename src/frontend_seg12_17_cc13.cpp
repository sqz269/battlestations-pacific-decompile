// Segments 12 and 17 front-end reconstructions, packet cc13-fe3 (docs/CC13_FE_3.md).
#include "bsp/frontend_seg12_17_cc13.hpp"

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
            if (n.ptr(0x18) != nullptr) host.free_00bf65ac(n.ptr(0x18));
            n.ptr(0x18) = nullptr;
            n.ptr(0x1C) = nullptr;
            n.ptr(0x20) = nullptr;
            if (n.ptr(0x10) != nullptr) host.sized_pool_return_00bd1510(n.ptr(0x10), n.at<std::uint32_t>(0xC) + 1, 1);
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

} // namespace bsp::cc13_fe3
