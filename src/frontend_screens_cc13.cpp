// Frontend and HUD menu screen reconstructions, packet cc13-fe1 (docs/CC13_FE_1.md).
#include "bsp/frontend_screens_cc13.hpp"
#include "bsp/band_0085_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/options_menu_screens.hpp"

#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace bsp::cc13_fe1 {
namespace {

template <class T>
T read_field(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
void write_field(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

std::byte* at(void* object, std::size_t offset) noexcept {
    return static_cast<std::byte*>(object) + offset;
}

const std::byte* at(const void* object, std::size_t offset) noexcept {
    return static_cast<const std::byte*>(object) + offset;
}

// Head-node reset shared by the clear and construct shapes: parent (root),
// left and right all point back at the head.
void reset_head_links(void* head) noexcept {
    write_field(head, 0x4, head);
    write_field(head, 0x0, head);
    write_field(head, 0x8, head);
}

}  // namespace

// ---------------------------------------------------------------------------
// Screen vtable leaves
// ---------------------------------------------------------------------------

std::int32_t screen_id_slot(std::uint32_t address) noexcept {
    for (const ScreenIdSlot& slot : kScreenIdSlots)
        if (slot.address == address) return slot.screen_id;
    return -1;
}

bool screen_return_true_slot(std::uint32_t address) noexcept {
    for (std::uint32_t slot : kReturnTrueSlots)
        if (slot == address) return true;
    return false;
}

std::int32_t screen_noop_slot(std::uint32_t address) noexcept {
    for (const NoOpSlot& slot : kNoOpSlots)
        if (slot.address == address) return slot.ret_bytes;
    return -1;
}

void* adjustor_thunk(std::uint32_t address, void* secondary, std::uint32_t flags,
    ScalarDeletingDtorHost& host) {
    for (const AdjustorThunk& thunk : kAdjustorThunks)
        if (thunk.address == address)
            return host.scalar_deleting_dtor(thunk.target, at(secondary, 0) - thunk.delta, flags);
    return nullptr;
}

bool collect_children(std::uint32_t address, void* screen, void* list, PointerVectorHost& host) {
    for (const ChildCollector& collector : kChildCollectors) {
        if (collector.address != address) continue;
        for (std::uint16_t offset : collector.offsets) {
            if (offset == 0) break;
            host.push_back_004d6790(list, at(screen, offset));
        }
        return true;
    }
    return false;
}

void listener_recapture_scroller_30(void* listener, ScrollerHost& host) {
    host.recapture_base_positions_006833f0(at(listener, 0x30));
}

void lobby_listener_on_layout_changed_005e10c0(void* listener, ScrollerHost& host) {
    // 005E10C3 LEA ECX,[ESI-0Ch]: the screen; the three scrollers follow in this order.
    host.lobby_build_map_preview_005d6e30(at(listener, 0) - 0xC);
    host.recapture_base_positions_006833f0(at(listener, 0x14C));
    host.recapture_base_positions_006833f0(at(listener, 0xC0));
    host.recapture_base_positions_006833f0(at(listener, 0x24));
}

void multi_main_listener_current1c_005ea700(void* listener, ScrollerHost& host) {
    host.multi_main_layout_achievement_locks_005ea030(at(listener, 0) - 0xC);
    host.recapture_base_positions_006833f0(at(listener, 0x60));
}

// ---------------------------------------------------------------------------
// Native containers
// ---------------------------------------------------------------------------

bool tree_clear(std::uint32_t address, void* tree, ContainerHost& host) {
    for (const TreeClear& entry : kTreeClears) {
        if (entry.address != address) continue;
        void* head = read_field<void*>(tree, 0x4);
        host.erase_subtree(entry.erase_subtree, read_field<void*>(head, 0x4));
        write_field(head, 0x4, head);
        write_field<std::uint32_t>(tree, 0x8, 0);
        write_field(head, 0x0, head);
        write_field(head, 0x8, head);
        return true;
    }
    return false;
}

bool tree_destroy(std::uint32_t address, void* object, ContainerHost& host) {
    for (const TreeDestroy& entry : kTreeDestroys) {
        if (entry.address != address) continue;
        void* tree = at(object, entry.tree_offset);
        void* head = read_field<void*>(tree, 0x4);
        host.erase_range(entry.erase_range, tree, read_field<void*>(head, 0x0), head);
        host.free_block(read_field<void*>(tree, 0x4));
        write_field<std::uint32_t>(tree, 0x4, 0);
        write_field<std::uint32_t>(tree, 0x8, 0);
        return true;
    }
    return false;
}

void* objective_tree_assign(void* tree, const void* source, ContainerHost& host) {
    if (tree != source) {
        void* head = read_field<void*>(tree, 0x4);
        host.erase_range(kObjectiveTreeEraseRange, tree, read_field<void*>(head, 0x0), head);
        host.copy_tree(kObjectiveTreeCopy, tree, source);
    }
    return tree;
}

void warning_tree28_construct_00583360(void* tree, ContainerHost& host) {
    void* head = host.allocate_head(0x005826B0u);
    write_field(tree, 0x4, head);
    write_field<std::uint8_t>(head, 0x19, 1);
    reset_head_links(head);
    write_field<std::uint32_t>(tree, 0x8, 0);
}

void* tree_copy_construct_00590080(void* tree, const void* source, ContainerHost& host) {
    warning_tree28_construct_00583360(tree, host);
    host.copy_tree(0x0058D930u, tree, source);
    return tree;
}

void* objective_widget_list_construct_005840b0(void* list, ContainerHost& host) {
    write_field(list, 0x4, host.allocate_head(0x00582280u));
    write_field<std::uint32_t>(list, 0x8, 0);
    return list;
}

void list_destroy_00583120(void* list, ContainerHost& host) {
    void* sentinel = read_field<void*>(list, 0x4);
    void* node = read_field<void*>(sentinel, 0x0);
    write_field(sentinel, 0x0, sentinel);
    write_field(read_field<void*>(list, 0x4), 0x4, read_field<void*>(list, 0x4));
    write_field<std::uint32_t>(list, 0x8, 0);
    while (node != read_field<void*>(list, 0x4)) {
        void* next = read_field<void*>(node, 0x0);
        host.free_block(node);
        node = next;
    }
    host.free_block(read_field<void*>(list, 0x4));
    write_field<std::uint32_t>(list, 0x4, 0);
}

void pod8_uninitialized_copy_00582ed0(const std::uint32_t* first, const std::uint32_t* last,
    std::uint32_t* dest) noexcept {
    for (; first != last; first += 2, dest += 2) {
        if (dest != nullptr) {
            dest[0] = first[0];
            dest[1] = first[1];
        }
    }
}

void* tree_buy_node(std::uint32_t address, void* left, void* parent, void* right,
    const void* value, std::uint8_t colour, ContainerHost& host) {
    for (const TreeBuyNode& entry : kTreeBuyNodes) {
        if (entry.address != address) continue;
        void* node = host.operator_new(entry.node_size);
        if (node != nullptr) {
            write_field(node, 0x0, left);
            write_field(node, 0x4, parent);
            write_field(node, 0x8, right);
            host.construct_value(entry.value_ctor, at(node, 0xC), value);
            write_field<std::uint8_t>(node, entry.colour_offset, colour);
            write_field<std::uint8_t>(node, entry.colour_offset + 1u, 0);
        }
        return node;
    }
    return nullptr;
}

bool short_vector_construct_0058a980(void* vector, std::uint32_t count, ShortVectorHost& host) {
    write_field<std::uint32_t>(vector, 0x4, 0);
    write_field<std::uint32_t>(vector, 0x8, 0);
    write_field<std::uint32_t>(vector, 0xC, 0);
    if (count == 0) return false;
    if (count > 0x7FFFFFFFu) host.throw_length_error_00589620();
    std::byte* first = static_cast<std::byte*>(host.allocate_shorts_005804b0(count));
    write_field(vector, 0x4, first);
    write_field(vector, 0x8, first);
    write_field(vector, 0xC, first + count * 2u);
    return true;
}

void pointer_array_push_back_0059fa40(void* array, const std::uint32_t* value, PointerArrayHost& host) {
    if (read_field<std::int32_t>(array, 0x4) == read_field<std::int32_t>(array, 0x8)) {
        std::int32_t capacity = read_field<std::int32_t>(array, 0x8) * 2;
        if (capacity < 2) capacity = 1;
        host.pointer_array_reserve(array, capacity);
    }
    const std::int32_t size = read_field<std::int32_t>(array, 0x4);
    // 0059FA81: the slot address is data + 4*size, tested against null (a null
    // data pointer with size 0 skips the store).
    const std::uintptr_t slot = read_field<std::uintptr_t>(array, 0x0) + static_cast<std::uintptr_t>(size) * 4u;
    if (slot != 0) write_field(reinterpret_cast<void*>(slot), 0, *value);
    write_field(array, 0x4, size + 1);
}

void owned_object_list_clear_005b7840(void* list, OwnedListHost& host) {
    while (read_field<std::int32_t>(list, 0x0) != 0) {
        void* node = read_field<void*>(list, 0x4);
        if (void* object = read_field<void*>(node, 0x8)) {
            host.delete_object(object);
            write_field<std::uint32_t>(node, 0x8, 0);
        }
        void* prev = read_field<void*>(node, 0x0);
        void* next = read_field<void*>(node, 0x4);
        if (prev == nullptr) write_field(list, 0x4, next);
        else write_field(prev, 0x4, next);
        if (next == nullptr) write_field(list, 0x8, prev);
        else write_field(next, 0x0, prev);
        write_field(list, 0x0, read_field<std::int32_t>(list, 0x0) - 1);
        host.free_block(node);
    }
}

void option_page_tidy_005f4a20(void* page, ContainerHost& host) {
    for (std::size_t base : {std::size_t{0x14}, std::size_t{0x04}}) {
        if (void* data = read_field<void*>(page, base)) host.free_block(data);
        write_field<std::uint32_t>(page, base, 0);
        write_field<std::uint32_t>(page, base + 4, 0);
        write_field<std::uint32_t>(page, base + 8, 0);
    }
}

void effect_sound_samples_tidy_005b9c40(void* vector, std::uint32_t count, SampleVectorHost& host) {
    host.resize(vector, count, 0);
    host.free_block(read_field<void*>(vector, 0x0));
}

// ---------------------------------------------------------------------------
// Native strings and small records
// ---------------------------------------------------------------------------

void native_string_destroy_at(void* object, std::size_t offset, NativeStringStorage& storage) noexcept {
    destroy_native_string_header_0041dd20(at(object, offset), storage);
}

namespace {
void* string_copy_header(void* dst, const void* src, NativeStringStorage& storage) {
    write_field<std::uint32_t>(dst, 0x0, 0);
    write_field<std::uint32_t>(dst, 0x4, 0);
    copy_native_string_header_00be0a30_fragment(dst, storage, src);
    return dst;
}
}  // namespace

void* string_byte_construct_00585f20(void* dst, const void* src, const std::uint8_t* tail,
    NativeStringStorage& storage) {
    string_copy_header(dst, src, storage);
    write_field(dst, 0x8, *tail);
    return dst;
}

void* string_byte_copy_00585fa0(void* dst, const void* src, NativeStringStorage& storage) {
    string_copy_header(dst, src, storage);
    write_field(dst, 0x8, read_field<std::uint8_t>(src, 0x8));
    return dst;
}

void* string_dword_copy_00586070(void* dst, const void* src, NativeStringStorage& storage) {
    string_copy_header(dst, src, storage);
    write_field(dst, 0x8, read_field<std::uint32_t>(src, 0x8));
    return dst;
}

void* profile_copy_mission_id_00586150(const void* profile, void* out, NativeStringStorage& storage) {
    return string_copy_header(out, at(profile, 0x68), storage);
}

void* string_pair_copy_005894f0(void* dst, const void* src, NativeStringStorage& storage) {
    string_copy_header(dst, src, storage);
    string_copy_header(at(dst, 0x8), at(src, 0x8), storage);
    write_field(dst, 0x10, read_field<std::uint32_t>(src, 0x10));
    return dst;
}

void* rec1c_construct_00589480(void* dst, std::uint32_t a, std::uint32_t b, std::uint32_t c,
    const void* string_record, std::uint8_t flag, NativeStringStorage& storage) {
    write_field(dst, 0x0, a);
    write_field(dst, 0x4, b);
    write_field(dst, 0x8, c);
    string_copy_header(at(dst, 0xC), string_record, storage);
    write_field(dst, 0x14, read_field<std::uint8_t>(string_record, 0x8));
    write_field(dst, 0x18, flag);
    write_field<std::uint8_t>(dst, 0x19, 0);
    return dst;
}

void* legacy_sbo_record_copy_00588640(void* dst, const void* src, LegacyStringHost& host) {
    write_field<std::uint32_t>(dst, 0x18, 0xF);
    write_field<std::uint32_t>(dst, 0x14, 0);
    write_field<std::uint8_t>(dst, 0x4, 0);
    host.assign_substring(dst, src, 0, 0xFFFFFFFFu);
    for (std::size_t offset : {0x20u, 0x24u, 0x28u, 0x2Cu})
        write_field(dst, offset, read_field<std::uint32_t>(src, offset));
    write_field(dst, 0x30, read_field<std::uint8_t>(src, 0x30));
    write_field(dst, 0x34, read_field<std::uint32_t>(src, 0x34));
    for (std::size_t offset : {0x38u, 0x39u, 0x3Au, 0x3Bu})
        write_field(dst, offset, read_field<std::uint8_t>(src, offset));
    return dst;
}

std::int32_t native_string_find_first_of_005b69b0(const void* string, const char* set, std::int32_t pos) noexcept {
    const char* data = read_field<const char*>(string, 0x4);
    const std::uint32_t length = read_field<std::uint32_t>(string, 0x0);
    if (data == nullptr || length == 0 || set == nullptr) return -1;
    if (pos < 0) pos = 0;
    else if (static_cast<std::uint32_t>(pos) > length) return -1;  // JA: unsigned compare
    const std::size_t span = std::strcspn(data + pos, set) + static_cast<std::uint32_t>(pos);
    if (span == length) return -1;
    return static_cast<std::int32_t>(span);
}

void record14_clear_005b7fa0(void* record) noexcept {
    for (std::size_t offset : {0x4u, 0x8u, 0xCu, 0x10u, 0x0u})
        write_field<std::uint32_t>(record, offset, 0);
}

void* observer_slot_set(void* owner, std::uint32_t target, ObserverHost& host) {
    const std::uint32_t current = read_field<std::uint32_t>(owner, 0x14);
    if (current != target) {
        if (current != 0) host.unregister_pair(owner);
        write_field(owner, 0x14, target);
        if (target != 0) host.register_pair(owner);
    }
    return owner;
}

void optional_vector3_get_or_default_0059bd20(const void* optional, const float* fallback_00f87574,
    float* out) noexcept {
    const float* source = read_field<std::uint8_t>(optional, 0x1) == 0
        ? fallback_00f87574
        : reinterpret_cast<const float*>(at(optional, 0x8));
    out[0] = source[0];
    out[1] = source[1];
    out[2] = source[2];
}

std::int32_t int_wrap_into_range_005ef4d0(std::int32_t value, std::int32_t lo, std::int32_t count) noexcept {
    if (value < lo) return count - 1;
    if (value <= count - 1) return value;
    return lo;
}

void* options_row_label_at_005f0220(void* out, const char* const* table, std::int32_t index,
    NativeStringRawPoolContext& context) {
    return construct_native_string_header_0041e870(out, context, table[index]);
}

bool session_message188_is_type_005f97a0(std::int32_t type) noexcept {
    return type == 0xBC || type == 0x49 || type == 0x46;
}

bool hud_order_screen_has_pending_order_005f9410(const void* screen) noexcept {
    return read_field<std::uint8_t>(screen, 0x156) != 0 || read_field<std::uint8_t>(screen, 0x157) != 0;
}

bool hud_order_menu_is_open_005fc620(const void* menu) noexcept {
    return read_field<std::uint8_t>(menu, 0x81) != 0 || read_field<std::uint8_t>(menu, 0x82) != 0;
}

void hud_order_screen_exit_005f9480(void* screen) noexcept {
    write_field<std::uint8_t>(screen, 0x81, 0);
    write_field<std::uint8_t>(screen, 0x82, 0);
}

std::uint32_t plane_ground_contact_owner_006049f0(const void* plane) noexcept {
    return read_field<std::uint8_t>(plane, 0xBF8) != 0 ? read_field<std::uint32_t>(plane, 0xBF4) : 0;
}

void prompt_result_set_quit_flag_005fc640(std::int32_t result, std::uint8_t& quit_00e1ae75) noexcept {
    // 005FC645 stores CL, which is 1 on this path.
    if (result == 1) quit_00e1ae75 = 1;
}

std::uint32_t unit_cast_to_squadron_005f9a30(std::uint32_t unit, EntityHost& host) {
    if (unit != 0 && host.is_kind(unit, 0x18)) return unit;
    return 0;
}

float unit_hud_health_0059ac50(std::uint32_t unit, UnitHealthHost& host) {
    if (host.is_kind(unit, 0x18)) {
        unit = read_field<std::uint32_t>(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(unit)), 0x3D0);
    } else if (host.is_kind(unit, 0x45)) {
        return host.airfield_health_fraction_006d2560(unit);
    }
    if (unit != 0) return host.unit_instance_health_00923be0(unit);
    return 0.0f;  // 0059AC93 FLDZ
}

// ---------------------------------------------------------------------------
// Widgets
// ---------------------------------------------------------------------------

namespace {
std::uint32_t widget_at(const void* object, std::size_t offset) noexcept {
    return read_field<std::uint32_t>(object, offset);
}
}  // namespace

void apply_focus_style(std::uint32_t widget, bool focused, bool has_toggle_case, WidgetHost& host) {
    // Each branch re-reads the type through +5Ch, as the native does.
    if (host.type(widget) == 3) {
        host.set_scale(widget, focused ? kFocusedScale : kUnfocusedScale);
        return;
    }
    if (host.type(widget) == 6) {
        host.set_frame_highlight(widget, focused, 0, 1.0f);
        return;
    }
    if (has_toggle_case && host.type(widget) == 0x12) host.set_toggle_highlight(widget, focused);
}

void hud_pause_on_focus_005fc650(std::uint32_t widget, bool focused, WidgetHost& host) {
    apply_focus_style(widget, focused, true, host);
}

void multi_main_listener_current18_005e6b80(const void* listener, std::uint32_t widget, bool focused,
    WidgetHost& host) {
    if (widget == widget_at(listener, 0xC0) && read_field<std::uint8_t>(listener, 0xA8) != 0) return;
    apply_focus_style(widget, focused, true, host);
}

void options_refresh_focus_005ef8d0(std::uint32_t widget, WidgetHost& host) {
    if (host.type(widget) == 2) return;
    const void* w = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(widget));
    apply_focus_style(widget, read_field<std::uint8_t>(w, 0xD4) != 0, false, host);
}

void multi_main_listener_current0c_005e6e20(void* listener, std::uint32_t widget, ScrollerInputHost& host) {
    if (widget == widget_at(listener, 0x58) || widget == widget_at(listener, 0x54))
        host.set_auto_scroll_00683300(at(listener, 0x60), 0, 0.0f, 0);
    else if (widget == widget_at(listener, 0xC0))
        host.end_thumb_drag_00683320(at(listener, 0x60));
    const void* w = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(widget));
    apply_focus_style(widget, read_field<std::uint8_t>(w, 0xD4) != 0, true, host);
}

void multi_main_set_buttons_visible_005e6b10(const void* screen, bool shown, WidgetHost& host) {
    for (std::size_t offset : {0x38u, 0x3Cu, 0x54u, 0x58u, 0x5Cu}) host.set_shown(widget_at(screen, offset), shown);
}

void hud_panel_set_visible3_005bca30(const void* panel, bool shown, WidgetHost& host) {
    for (std::size_t offset : {0x48u, 0xF0u, 0x1Cu}) host.set_shown(widget_at(panel, offset), shown);
}

void hud_narrative_show_default_005b5bd0(const void* screen, WidgetHost& host) {
    host.set_shown(widget_at(screen, 0x28), false);
    host.set_shown(widget_at(screen, 0xAC), false);
    host.set_shown(widget_at(screen, 0x24), true);
}

void hud_narrative_hide_text_005b5d40(const void* screen, WidgetHost& host) {
    host.set_shown(widget_at(screen, 0xAC), false);
}

void hud_order_hide_all_005f9490(const void* screen, WidgetHost& host) {
    for (std::size_t offset = 0xC; offset <= 0x34; offset += 4) host.set_shown(widget_at(screen, offset), false);
}

void hud_map_legend_hide_all_0059ad60(const void* legend, WidgetHost& host) {
    host.set_shown(widget_at(legend, 0x4), false);
    for (std::size_t offset = 0x8; offset < 0x28; offset += 4) host.set_shown(widget_at(legend, offset), false);
}

void hud_map_hide_unused_arrow_widgets_0059f9b0(void* screen, WidgetHost& host) {
    for (std::int32_t index = read_field<std::int32_t>(screen, 0x548) + 1;; ++index) {
        // The array size is re-read every pass (+544h may be null: size 0).
        const void* array = read_field<const void*>(screen, 0x544);
        const std::int32_t size = array == nullptr ? 0 : read_field<std::int32_t>(array, 0x4);
        if (index >= size) break;
        const void* entry = read_field<const void*>(read_field<const void*>(array, 0x0), index * 4u);
        hud_map_legend_hide_all_0059ad60(entry, host);
    }
    write_field<std::int32_t>(screen, 0x548, -1);
}

void hud_panel_set_pending_value_005bca80(void* panel, std::uint32_t value, std::uint32_t extra) noexcept {
    write_field(panel, 0x170, value);
    write_field<std::uint8_t>(panel, 0x168, 1);
    write_field(panel, 0x174, extra);
}

void hud_map_force_cursor_pos_0059ad20(void* screen, const std::uint32_t* pos) noexcept {
    write_field<std::uint8_t>(screen, 0x5F0, 1);
    write_field(screen, 0x5F4, pos[0]);
    write_field(screen, 0x5F8, pos[2]);
}

// ---------------------------------------------------------------------------
// Reference-counted handles and owned objects
// ---------------------------------------------------------------------------

void release_counted(void*& slot, RefCountHost& host) {
    if (slot != nullptr) {
        if (host.interlocked_decrement(at(slot, 0x4)) == 0) host.release_object(slot);
    }
}

void main_menu_release_mission_preview_005830a0(void* screen, RefCountHost& host) {
    void* preview = read_field<void*>(screen, 0x574);
    if (preview != nullptr) host.stop_preview(preview);
    preview = read_field<void*>(screen, 0x574);
    if (preview != nullptr) {
        release_counted(preview, host);
        write_field<std::uint32_t>(screen, 0x574, 0);
    }
    write_field<std::uint32_t>(screen, 0x574, 0);  // 005830EB stores it a second time
    void* other = read_field<void*>(screen, 0x570);
    release_counted(other, host);
    write_field<std::uint32_t>(screen, 0x570, 0);
}

void hud_map_model_ref_destruct_0059f780(void* ref, RefCountHost& host) {
    write_field<std::uint32_t>(ref, 0x0, 0x00CEFFE8u);
    void* target = read_field<void*>(ref, 0x4);
    if (target != nullptr) {
        release_counted(target, host);
        write_field<std::uint32_t>(ref, 0x4, 0);
    }
}

namespace {
void destruct_marker(void* marker, std::uint32_t vtable, std::size_t owned_offset, RefCountHost& host) {
    write_field(marker, 0x0, vtable);
    if (void* owned = read_field<void*>(marker, owned_offset)) {
        host.delete_owned(owned);
        write_field<std::uint32_t>(marker, owned_offset, 0);
    }
}
}  // namespace

void hud_minimap_marker_b_destruct_005bd690(void* marker, RefCountHost& host) {
    destruct_marker(marker, 0x00CF1158u, 0x8, host);
}

void hud_minimap_marker_destruct_005bd6c0(void* marker, RefCountHost& host) {
    destruct_marker(marker, 0x00CF1154u, 0x4, host);
}

void node_handle_release_0059bef0(void* handle, RefCountHost& host) {
    if (void* node = read_field<void*>(handle, 0x0)) {
        host.node_unlink_and_release(node);
        write_field<std::uint32_t>(handle, 0x0, 0);
    }
}

void hud_map_icon_array_clear_0059fd80(void* array_slot, IconArrayHost& host) {
    void* array = read_field<void*>(array_slot, 0x0);
    if (array != nullptr && read_field<std::int32_t>(array, 0x4) != 0) {
        host.reserve_0059e690(array_slot, 0);
        write_field<std::int32_t>(read_field<void*>(array_slot, 0x0), 0x4, 0);
    }
}

void hud_map_release_icon_array_005a09a0(void* array_slot, IconArrayHost& host) {
    if (const void* array = read_field<const void*>(array_slot, 0x0)) {
        const std::byte* first = read_field<const std::byte*>(array, 0x0);
        const std::byte* last = first + read_field<std::int32_t>(array, 0x4) * 4;
        for (const std::byte* p = first; p != last; p += 4)
            if (void* element = read_field<void*>(p, 0)) host.delete_element(element);
    }
    hud_map_icon_array_clear_0059fd80(array_slot, host);
}

void* session_message188_scalar_delete_005f97d0(void* message, std::uint8_t flags,
    void (*free_block)(void*)) {
    write_field<std::uint32_t>(message, 0x0, 0x00CE4974u);
    if ((flags & 1) != 0) free_block(message);
    return message;
}

// ---------------------------------------------------------------------------
// Screen registration, construction and destruction
// ---------------------------------------------------------------------------

void hud_objectives_register_005ee010(void* screen, ScreenLifetimeHost& host) {
    host.frontend_register_004f71d0(screen);
    write_field(screen, 0x10, host.load_page_00aa5840(kHudObjectivesPage));
    write_field<std::uint8_t>(screen, 0x64, 0);
    write_field<std::uint32_t>(screen, 0x60, 0);
    write_field<std::uint8_t>(screen, 0x0C, 0);
}

void hud_order_register_005f9ff0(void* screen, ScreenLifetimeHost& host) {
    host.frontend_register_004f71d0(screen);
    write_field(screen, 0x08, host.load_page_00aa5840(kHudOrderPage));
    host.screen_bind_layout(screen);
    write_field<std::uint32_t>(screen, 0x88, 0);
    write_field<std::uint8_t>(screen, 0x8C, 0);
    write_field<std::uint8_t>(screen, 0x5D, 0);
    write_field<std::uint8_t>(screen, 0x5E, 0);
    write_field<std::uint8_t>(screen, 0x5C, 0);
    write_field<std::uint8_t>(screen, 0x81, 0);
    write_field<std::uint8_t>(screen, 0x82, 0);
    write_field<std::uint32_t>(screen, 0x84, 0x128);
    write_field<std::uint8_t>(screen, 0x83, 1);
    write_field<std::uint32_t>(screen, 0x60, 0);
    write_field<std::uint32_t>(screen, 0x64, 0);
    write_field<std::uint32_t>(screen, 0x70, 0);
}

void hud_pause_cheat_register_00604890(void* screen, ScreenLifetimeHost& host) {
    host.frontend_register_004f71d0(screen);
    write_field(screen, 0x08, host.load_page_00aa5840(kHudPauseCheatPage));
}

void multi_main_register_005ea720(void* screen, ScreenLifetimeHost& host) {
    host.frontend_register_004f71d0(screen);
    host.screen_bind_layout(screen);
    write_field<std::uint32_t>(screen, 0xF0, 0);
    write_field<std::uint32_t>(screen, 0xE0, 0);
    write_field<std::uint32_t>(screen, 0xE4, 0);
    write_field(screen, 0xEC, host.widget_resolved_position_00aa6750(widget_at(screen, 0xDC))[1]);
    write_field<std::int32_t>(screen, 0xF4, -1);
    host.multi_main_layout_achievement_locks_005ea030(screen);
}

void multi_mode_selector_register_005ea7c0(void* screen, ScreenLifetimeHost& host) {
    host.frontend_register_004f71d0(screen);
    host.screen_bind_layout(screen);
    write_field<std::int32_t>(screen, 0xF4, -1);
    write_field<std::uint32_t>(screen, 0x10, 0);
}

namespace {
void* construct_scroller_screen(void* screen, std::uint32_t vt0, std::uint32_t vt8, std::uint32_t vtc,
    ScreenLifetimeHost& host) {
    host.frontend_construct_004f7180(screen);
    write_field(screen, 0x0, vt0);
    write_field(screen, 0x8, vt8);
    write_field(screen, 0xC, vtc);
    host.scroller_construct_00683610(at(screen, 0x6C));
    return screen;
}
}  // namespace

void* multi_main_construct_005e6d90(void* screen, ScreenLifetimeHost& host) {
    return construct_scroller_screen(screen, 0x00CF2708u, 0x00CF26F4u, 0x00CF26D4u, host);
}

void* multi_mode_selector_construct_005ea8c0(void* screen, ScreenLifetimeHost& host) {
    return construct_scroller_screen(screen, 0x00CF2B44u, 0x00CF2B30u, 0x00CF2B10u, host);
}

void* hud_objectives_construct_005ee440(void* screen, ScreenLifetimeHost& host) {
    host.frontend_construct_004f7180(screen);
    write_field<std::uint32_t>(screen, 0x0, 0x00CF2D20u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF2D0Cu);
    for (std::size_t offset : {0x44u, 0x48u, 0x4Cu, 0x54u, 0x58u, 0x5Cu}) write_field<std::uint32_t>(screen, offset, 0);
    return screen;
}

void* hud_order_construct_005f9440(void* screen, ScreenLifetimeHost& host) {
    host.frontend_construct_004f7180(screen);
    write_field<std::uint32_t>(screen, 0x0, 0x00CF3B7Cu);
    return screen;
}

void* hud_pause_cheat_construct_00604780(void* screen, ScreenLifetimeHost& host) {
    host.frontend_construct_004f7180(screen);
    write_field<std::uint32_t>(screen, 0x0, 0x00CF41A0u);
    return screen;
}

namespace {
void destroy_pages(void* screen, std::initializer_list<std::size_t> offsets, ScreenLifetimeHost& host) {
    for (std::size_t offset : offsets) host.remove_and_destroy_page(widget_at(screen, offset));
}
void free_vector(void* screen, std::size_t offset, ScreenLifetimeHost& host) {
    if (void* data = read_field<void*>(screen, offset)) host.free_block(data);
    write_field<std::uint32_t>(screen, offset, 0);
    write_field<std::uint32_t>(screen, offset + 4, 0);
    write_field<std::uint32_t>(screen, offset + 8, 0);
}
}  // namespace

void multi_main_destruct_005e70b0(void* screen, ScreenLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF2708u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF26F4u);
    write_field<std::uint32_t>(screen, 0xC, 0x00CF26D4u);
    destroy_pages(screen, {0x18, 0x1C}, host);
    write_field<std::uint32_t>(screen, 0x8, 0x00CEB0FCu);
    host.frontend_destruct(screen);
}

void multi_mode_selector_destruct_005eabc0(void* screen, ScreenLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF2B44u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF2B30u);
    write_field<std::uint32_t>(screen, 0xC, 0x00CF2B10u);
    destroy_pages(screen, {0x14, 0x18, 0x1C}, host);
    write_field<std::uint32_t>(screen, 0x8, 0x00CEB0FCu);
    host.frontend_destruct(screen);
}

void hud_objectives_destruct_005edf30(void* screen, ScreenLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF2D20u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF2D0Cu);
    destroy_pages(screen, {0x10}, host);
    free_vector(screen, 0x54, host);
    free_vector(screen, 0x44, host);
    write_field<std::uint32_t>(screen, 0x8, 0x00CEB0FCu);
    host.frontend_destruct(screen);
}

void hud_order_destruct_005f9d70(void* screen, ScreenLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF3B7Cu);
    destroy_pages(screen, {0x08}, host);
    host.frontend_destruct(screen);
}

void hud_pause_cheat_destruct_00604810(void* screen, ScreenLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF41A0u);
    destroy_pages(screen, {0x08}, host);
    host.frontend_destruct(screen);
}

// ---------------------------------------------------------------------------
// Main menu callbacks
// ---------------------------------------------------------------------------

void main_menu_on_movie_advance_005861a0(MainMenuCallbackHost& host) {
    host.advance_background_movie_005845d0(host.main_menu_screen());
}

void main_menu_on_async_text_request_done_00590e40(MainMenuCallbackHost& host) {
    host.finish_async_text_request_00590a00(host.main_menu_screen(), 1);
}

void main_menu_remove_file_store_file_00588460(const void* file, MainMenuCallbackHost& host) {
    if (void* screen = host.main_menu_screen()) host.play_test_speech_00585c50(screen, file);
    host.file_store_remove_file(file);
}

void main_menu_on_mission_detail_prompt_result_0058f540(std::int32_t result, MainMenuCallbackHost& host) {
    if (result == 0) return;
    void* screen = host.main_menu_screen();
    write_field<std::uint8_t>(screen, 0x5C, 0);
    host.build_mission_detail_page_0058c010(screen);
}

void main_menu_screen_exit_0058f560(void* screen, std::int32_t& page_00e08874, WidgetHost& widgets,
    MainMenuCallbackHost& host) {
    widgets.set_shown(widget_at(screen, 0x2F0), false);
    if (host.game_state() != 10 && page_00e08874 != 1 && page_00e08874 != 2 && page_00e08874 != 3)
        page_00e08874 = 1;
    host.clear_objective_rows_00583e50(screen);
    host.build_mission_map_icons_0058d9e0(screen, true);
}

void main_menu_cancel_new_profile_unreferenced_00580780(NewProfileTextHost& host) {
    new_profile_cancel_005807d0(host);
}

void main_menu_clear_text_editor_00583de0(void* screen, TextEditorHost& host) {
    void* text = at(screen, 0x24);
    host.resize_native_string(text, 0, false);
    // 00583DF2..00583E07: memcpy(data, 00CE3A0C, length) with the length now 0.
    const std::uint32_t list_box = widget_at(screen, 0x1B8);
    host.list_box_set_enabled_60(list_box, false);
    write_field<std::uint8_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(widget_at(screen, 0x1B8))), 0x77, 1);
    const std::uint32_t value = host.list_box_value_00425e50(widget_at(screen, 0x1B8));
    host.editor_set_active_00a966e0(at(screen, 0xC), true, value);
    host.editor_virtual_04(at(screen, 0xC));
}

void main_menu_slide_widget_005808b0(void* screen, bool move, bool active, SlideHost& host) {
    if (move) {
        const float offset = active ? 0.0f : 1000.0f;
        const float xyz[3] = {read_field<float>(screen, 0x558) + 0.0f, read_field<float>(screen, 0x55C) + 0.0f,
                              read_field<float>(screen, 0x560) + offset};
        host.set_resolved_position_00aa8240(widget_at(screen, 0x1B8), xyz);
    }
    host.set_listener_fields_00aa6bc0(widget_at(screen, 0x1B8), active ? at(screen, 0x40) : nullptr, 0);
}

std::int32_t mission_tree_mission_index_of_005c3850(void* screen, const void* name, MissionTreeLookupHost& host) {
    return host.find_mission_by_name_005c3470(screen, name);
}

// ---------------------------------------------------------------------------
// Options screen
// ---------------------------------------------------------------------------

void options_prompt_reset_defaults_005efcb0(std::int32_t result, OptionsCallbackHost& host) {
    if (host.options_manager() != nullptr && result == 2) host.reset_to_defaults_005efab0(host.options_screen());
}

void options_prompt_apply_005f5b90(std::int32_t result, OptionsCallbackHost& host) {
    if (host.options_manager() == nullptr) return;
    host.refresh_help_line_005f0cd0(host.options_screen());
    if (result != 1) return;
    void* screen = host.options_screen();  // read before ApplyAll (005F5BAE)
    host.settings_apply_all_008d5b50();
    host.build_main_list_005f54f0(screen);
}

void options_prompt_commit_005f6fb0(std::int32_t result, OptionsCallbackHost& host) {
    if (host.options_manager() == nullptr) return;
    host.refresh_help_line_005f0cd0(host.options_screen());
    if (result == 1) host.commit_changes_005f65c0(host.options_screen());
}

void options_prompt_video_confirm_005f6fe0(std::int32_t result, OptionsCallbackHost& host) {
    if (host.options_manager() == nullptr) return;
    host.refresh_help_line_005f0cd0(host.options_screen());
    if (result == 1) host.commit_changes_005f65c0(host.options_screen());
    else if (result == 0) host.revert_video_settings_005efcf0(host.options_screen());
}

void options_prompt_apply_and_save_005f01c0(std::int32_t result, std::uint8_t* settings_f889c1,
    OptionsCallbackHost& host) {
    if (result == 1) {
        host.media_manager_refresh_00a4c2d0();
        host.settings_apply_all_008d5b50();
        host.settings_save_to_profile_008d6c00(0x00BD53C0u);
        return;
    }
    // 005F01EC..005F021B: F889C5..C8 -> F889C1..C4.
    for (int i = 0; i < 4; ++i) settings_f889c1[i] = settings_f889c1[i + 4];
}

void options_apply_and_rebuild_main_list_005f5ac0(void* screen, OptionsCallbackHost& host) {
    host.settings_apply_all_008d5b50();
    host.build_main_list_005f54f0(screen);
}

void options_screen_exit_005f7020(void* screen, OptionsCallbackHost& host) {
    if (!host.game_in_mission_61f()) host.command_owner_set_help_variant_00549620(0);
    host.sync_profile_values_005f6ef0(screen);
}

void options_revert_video_settings_005efcf0(void* screen, const VideoSettingsSnapshot& live,
    OptionsCallbackHost& host) {
    write_field(screen, 0xEC, live.f88994);
    write_field(screen, 0xF0, live.f88998);
    write_field(screen, 0x150, live.f889f8);
    write_field(screen, 0xF6, live.f8899e);
    host.settings_apply_all_008d5b50();
    if (!host.ingame_interface_present()) host.xlive_on_reset_device(live.device_00f8d394 + 0x1A28u);
}

bool utf16_char_sanitize_for_name_005ef500(char16_t& c) noexcept {
    const char16_t v = c;
    const bool keep = (v >= u'a' && v <= u'z') || (v >= u'A' && v <= u'Z') || (v >= u'0' && v <= u'9') ||
                      v == u' ' || v == 0;
    if (keep) return false;
    c = u' ';
    return true;
}

namespace {
void build_options_page(void* screen, OptionsPage page, std::int32_t& page_00e19650, OptionsPageHost& host) {
    const OptionsPageInfo* info = options_page_info(page);
    page_00e19650 = static_cast<std::int32_t>(page);
    host.build_settings_page_005f24d0(screen, info->title, info->label_table, info->label_count,
        info->value_rows, info->help_table);
}
}  // namespace

void options_build_game_page_005f58f0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host) {
    page_00e19650 = static_cast<std::int32_t>(OptionsPage::Game);
    const bool many = host.edit_settings_query_008d45c0(at(screen, 0xD8)) > 1;  // SETG
    build_options_page(screen, OptionsPage::Game, page_00e19650, host);
    if (!many) {
        void* row = host.list_box_row_00a9be00(widget_at(screen, 0x88), 0);
        write_field<std::uint8_t>(row, 0x77, 1);
    }
    host.refresh_page_rows(0x005F4C90u, screen);
}

void options_build_audio_page_005f59f0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host) {
    build_options_page(screen, OptionsPage::Audio, page_00e19650, host);
    host.refresh_page_rows(0x005F4FA0u, screen);
}

void options_build_video_page_005f41e0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host) {
    build_options_page(screen, OptionsPage::Video, page_00e19650, host);
    host.refresh_page_rows(0x005F2F20u, screen);
}

void options_build_control_page_005f40f0(void* screen, std::int32_t& page_00e19650, OptionsPageHost& host) {
    build_options_page(screen, OptionsPage::Control, page_00e19650, host);
    auto* bytes = static_cast<std::uint8_t*>(screen);
    host.control_layout_button_flags_00527cb0(bytes + 0x119, bytes + 0x11A, bytes + 0x11B, bytes + 0x118);
    host.refresh_page_rows(0x005F34D0u, screen);
}

// ---------------------------------------------------------------------------
// HUD pause, order and objectives
// ---------------------------------------------------------------------------

void hud_pause_resume_005fd440(void* screen, HudPauseHost& host) {
    host.set_command_bar_byte4(0);
    host.input_set_context_level(0x1B, 0);
    host.game_set_cinematic_mode_004cd0f0(0, 0, 1);
    host.game_set_system_pause_004d95f0(false);
    host.sound_queue_set_flag_1a();
    write_field<std::uint8_t>(screen, 0x4, 0);
    host.frontend_restore_level_set_004f8390(3, reinterpret_cast<void*>(std::uintptr_t{0x00E19664u}));
    host.input_contexts_set_level_set_004d6410(3, reinterpret_cast<void*>(std::uintptr_t{0x00E19674u}));
}

void hud_pause_open_005ff0d0(void* screen, HudPauseHost& host) {
    host.input_set_context_level(0x1B, 5);
    host.sound_queue_set_flag_1a();
    write_field<std::uint8_t>(screen, 0x4, 1);
    write_field<std::uint8_t>(screen, 0x5, 1);
    host.frontend_commit_visibility_004f83b0(screen);
    host.screen_virtual_18(screen);
    host.save_level_sets();
}

void hud_pause_prompt_quit_mission_005fd610(std::int32_t result, HudPauseHost& host) {
    if (result != 1) return;
    host.stop_active_movie_a8();
    host.game_end_scene_004d7970(true);
    hud_pause_resume_005fd440(host.hud_pause_screen(), host);
}

void hud_pause_prompt_restart_005fd5b0(std::int32_t result, HudPauseHost& host) {
    if (result != 1) return;
    host.profile_clear_checkpoint_and_rewrite_00437c70();
    host.stop_active_movie_a8();
    host.game_end_scene_004d7970(false);
    host.game_request_state_004d7920(0xB);
    hud_pause_resume_005fd440(host.hud_pause_screen(), host);
}

void hud_pause_exit_005fd880(HudPauseHost& host) {
    host.frontend_frame_select_default_004c9680();
}

void hud_order_close_005f9520(void* screen, WidgetHost& widgets, HudOrderCloseHost& host) {
    write_field<std::uint8_t>(screen, 0x82, 0);
    write_field<std::uint8_t>(screen, 0x81, 0);
    widgets.set_shown(widget_at(screen, 0x8), false);
    host.weapon_group_suspend_00545360();
    host.set_ingame_68_byte190();
    host.collapse_overlays_0068ab80(false, false);
    host.leave_unit_request_level_0068ab30();
}

void hud_objectives_set_expanded_005ed5d0(void* screen, bool expanded, WidgetHost& widgets,
    HudObjectivesExpandHost& host) {
    if (read_field<std::uint8_t>(screen, 0xC) != 0) return;
    write_field<std::uint8_t>(screen, 0x70, expanded ? 1 : 0);
    widgets.set_enabled(widget_at(screen, 0x34), expanded);
    if (read_field<std::uint8_t>(screen, 0x70) != 0) {
        host.set_level3_set_004f8670(0x39, 0);
        host.input_push_level_004d8b70(0x14, 0xE, 0);
        host.counters_hide_all_005290d0();
    } else if (!host.overlay_busy_006529e0()) {
        host.collapse_overlays_0068ab80(false, true);
    }
    if (!expanded) host.counters_restore_hidden_00529350();
}

// ---------------------------------------------------------------------------
// Multiplayer menus
// ---------------------------------------------------------------------------

void lobby_on_value_changed_005e4f90(void* listener, std::int32_t value, MultiMenuHost& host) {
    if (value != read_field<std::int32_t>(listener, 0x8C) && host.session_state_1fe4() != 2)
        host.lobby_step_setting_005e1cf0(at(listener, 0) - 8, -1);
}

void multi_main_exit_005e6b60(const void* screen, MultiMenuHost& host) {
    host.list_box_clear_rows_00a9bec0(widget_at(screen, 0x20));
    host.list_box_clear_rows_00a9bec0(widget_at(screen, 0x24));
}

void multi_main_reset_flags_005ea7f0(void* screen, std::int32_t& mode_00e19610, std::int32_t& index_00e19614) noexcept {
    mode_00e19610 = 0;
    index_00e19614 = 0;
    write_field<std::uint32_t>(screen, 0xF4, 0);
}

void multi_mode_selector_exit_005ea960(std::uint8_t* object_00e19698) noexcept {
    object_00e19698[4] = 0;
}

void multi_main_start_hosting_005e6f00(const HostingFlags& flags, void* game_00e188a8, MultiMenuHost& host) {
    *flags.e08700 = 1;
    *flags.e08701 = 0;
    *flags.e08718 = 0;
    *flags.e08720 = 0;
    host.session_set_mode_0076fad0(2);
    host.push_request_interface_00688ad0(0x1A);
    // 005E6F3C re-reads the game pointer and the online object after the calls.
    const std::uint8_t online = host.online_byte_4d();
    write_field<std::uint8_t>(game_00e188a8, 0x2019, 0);
    write_field<std::uint8_t>(game_00e188a8, 0x2059, online);
    write_field<std::uint8_t>(game_00e188a8, 0x2017, 8);
    write_field<std::uint8_t>(game_00e188a8, 0x201D, 1);
    write_field<std::uint8_t>(game_00e188a8, 0x201C, 0x14);
}

void lobby_on_scene_index_received_005e5ae0(void* screen, std::uint32_t& scene_00e19594, MultiMenuHost& host) {
    const std::int32_t slot = host.session_slot_18ec();
    if (slot < 0 || slot > 7) return;
    scene_00e19594 = host.game_scene_index_60c();
    write_field<std::uint32_t>(screen, 0x10, 1);
    for (std::uint32_t routine : {0x005E3580u, 0x005D6C10u, 0x005D98E0u, 0x005D5340u}) host.lobby_refresh(routine, screen);
}

void* lobby_settings_option_table_005e1990(LobbyOptionHost& host) {
    if (void* tree = host.mission_tree()) {
        void* record = host.current_mission_record(tree);
        if (read_field<std::uint8_t>(record, 0x424) != 0) return at(record, 0x428);
    }
    return at(host.option_registry_00576b10(), 0x28);
}

// ---------------------------------------------------------------------------
// HUD narrative and map helpers
// ---------------------------------------------------------------------------

void panel_owner_stop_voice_005b68d0(void* owner, VoiceSlotHost& host) {
    write_field(owner, 0x88, 0.0f);
    write_field(owner, 0x8C, 0.0f);
    const std::int32_t index = read_field<std::int32_t>(owner, 0x70);
    if (index < 0) return;
    void* slot = at(owner, 8u + static_cast<std::uint32_t>(index) * 0x18u);
    if (host.poll_007027b0(slot)) host.stop_007026f0(slot);
}

void hud_map_set_icon_texture_0059aa10(void* icon, void* texture, float value, MapIconHost& host) {
    if (void* material = read_field<void*>(icon, 0x13C)) {
        host.material_set_texture_slot_00b189f0(material, 0, texture);
        write_field(icon, 0x140, value);
    }
}

void hud_map_icon_set_color_0059a9b0(void* icon, const float* rgba, MapIconHost& host) {
    float* colour = host.material_diffuse_color_00b179f0(read_field<void*>(icon, 0x134), 0);
    for (int i = 0; i < 4; ++i) colour[i] = rgba[i];
    if (void* second = read_field<void*>(icon, 0x138)) {
        colour = host.material_diffuse_color_00b179f0(second, 0);
        for (int i = 0; i < 4; ++i) colour[i] = rgba[i];
    }
}

void hud_map_icon_refresh_side_texture_0059d2a0(void* icon, MapIconHost& host) {
    if (read_field<std::uint32_t>(icon, 0xC) == 0) return;
    std::int32_t index = read_field<std::int32_t>(read_field<const void*>(icon, 0x4), 0x54);
    if (host.recon_relation_if_identified_00803ce0() == 3) index = 2;
    void* table = read_field<void*>(icon, 0x14);
    host.material_set_texture_slot_00b189f0(read_field<void*>(icon, 0x134), 0,
        read_field<void*>(table, static_cast<std::size_t>(index) * 4u));
    if (void* second = read_field<void*>(icon, 0x138))
        host.material_set_texture_slot_00b189f0(second, 0,
            read_field<void*>(read_field<void*>(icon, 0x14), static_cast<std::size_t>(index) * 4u));
}

void hud_map_clear_speaking_icon_005a4860(void* screen, MapIconHost& host) {
    const std::uint32_t unit = read_field<std::uint32_t>(screen, 0x52C);
    if (unit == 0) return;
    void* icon = host.find_unit_icon_005a22d0(screen, unit);
    if (icon != nullptr && read_field<std::uint32_t>(icon, 0x118) != 0) {
        if (void* widget = read_field<void*>(icon, 0x20)) {
            host.delete_owned(widget);
            write_field<std::uint32_t>(icon, 0x20, 0);
        }
        host.icon_start_blink_005a21e0(icon);
    }
    write_field(screen, 0x530, read_field<std::uint32_t>(screen, 0x52C));
    write_field<std::uint32_t>(screen, 0x52C, 0);
}

void hud_map_set_unit_highlight_005aaad0(void* screen, std::uint32_t unit, std::uint8_t value, MapIconHost& host) {
    *host.map_subscript_005a7af0(at(screen, 0x600), unit) = value;
}

// ---------------------------------------------------------------------------
// Native string records (batch 2)
// ---------------------------------------------------------------------------

void* string_record_assign(void* dst, const void* src, int strings, NativeStringStorage& storage) {
    for (int i = 0; i < strings; ++i)
        copy_native_string_header_00be0a30_fragment(at(dst, i * 8u), storage, at(src, i * 8u));
    return dst;
}

void* string_record_copy(void* dst, const void* src, int strings, NativeStringStorage& storage) {
    for (int i = 0; i < strings; ++i) string_copy_header(at(dst, i * 8u), at(src, i * 8u), storage);
    return dst;
}

void option_row_destruct_005efdc0(void* row, NativeStringStorage& storage) noexcept {
    destroy_native_string_header_0041dd20(at(row, 0x8), storage);
    destroy_native_string_header_0041dd20(at(row, 0x0), storage);
}

void* option_row_construct_005effa0(void* row, std::uint32_t a_length, const char* a_data,
    std::uint32_t b_length, const char* b_data, NativeStringStorage& storage) {
    // The by-value arguments are native headers on the stack; model them as locals.
    struct Header { std::uint32_t length; const char* data; };
    Header a{a_length, a_data};
    Header b{b_length, b_data};
    for (std::size_t offset : {0x0u, 0x4u, 0x8u, 0xCu}) write_field<std::uint32_t>(row, offset, 0);
    copy_native_string_header_00be0a30_fragment(at(row, 0x0), storage, &a);
    copy_native_string_header_00be0a30_fragment(at(row, 0x8), storage, &b);
    destroy_native_string_header_0041dd20(&a, storage);
    destroy_native_string_header_0041dd20(&b, storage);
    return row;
}

void* id_name_pair_construct_005fdca0(void* pair, const std::uint32_t* key, const void* name,
    NativeStringStorage& storage) {
    write_field(pair, 0x0, *key);
    string_copy_header(at(pair, 0x4), name, storage);
    return pair;
}

void* string_then_subobject(std::uint32_t routine, void* dst, const void* src_string, const void* arg,
    NativeStringStorage& storage, SubObjectHost& host) {
    string_copy_header(dst, src_string, storage);
    host.construct(routine, at(dst, 0x8), arg);
    return dst;
}

void* mission_score_pair_construct_005943d0(void* dst, const void* name, const void* record,
    NativeStringStorage& storage, MissionScoreHost& host) {
    string_copy_header(dst, name, storage);
    host.record_assign_00593ca0(at(dst, 0x8), record);
    write_field(dst, 0x28C, read_field<std::uint32_t>(record, 0x284));
    return dst;
}

void* mission_score_pair_copy_00594340(void* dst, const void* src, NativeStringStorage& storage,
    MissionScoreHost& host) {
    string_copy_header(dst, src, storage);
    host.record_assign_00593ca0(at(dst, 0x8), at(src, 0x8));
    write_field(dst, 0x28C, read_field<std::uint32_t>(src, 0x28C));
    return dst;
}

void mission_score_pair_destroy_005942a0(void* pair, NativeStringStorage& storage, MissionScoreHost& host) {
    host.record_destruct_00593570(at(pair, 0x8));
    destroy_native_string_header_0041dd20(pair, storage);
}

void native_string_array_remove_swap_last_005f1930(void* array, std::int32_t index, NativeStringStorage& storage) {
    if (index == -1) return;
    const std::int32_t count = read_field<std::int32_t>(array, 0x4);
    std::byte* data = read_field<std::byte*>(array, 0x0);
    if (index != count - 1)
        copy_native_string_header_00be0a30_fragment(data + index * 8, storage, data + (count - 1) * 8);
    // 005F197B re-reads data and count after the copy.
    data = read_field<std::byte*>(array, 0x0);
    destroy_native_string_header_0041dd20(data + (read_field<std::int32_t>(array, 0x4) - 1) * 8, storage);
    write_field(array, 0x4, read_field<std::int32_t>(array, 0x4) - 1);
}

void* native_string_from_unsigned_005f1750(void* out, std::uint32_t value, NativeStringStorage& storage) {
    write_field<std::uint32_t>(out, 0x0, 0);
    write_field<std::uint32_t>(out, 0x4, 0);
    char buffer[64];
    std::snprintf(buffer, sizeof buffer, "%u", value);
    struct Header { std::uint32_t length; char* data; } temp{0, nullptr};
    resize_native_string_header_0041dd40(&temp, storage, static_cast<std::uint32_t>(std::strlen(buffer)), true);
    if (temp.data != nullptr) std::memcpy(temp.data, buffer, temp.length + 1u);
    copy_native_string_header_00be0a30_fragment(out, storage, &temp);
    destroy_native_string_header_0041dd20(&temp, storage);
    return out;
}

void native_string_vector_erase_00589690(void* vector, std::uint32_t* out_iter, std::uint32_t iter_container,
    void* where, NativeStringRangeHost& host) {
    host.assign_range_0048da50(at(where, 0x8), read_field<void*>(vector, 0x8), where);
    std::byte* last = read_field<std::byte*>(vector, 0x8);
    host.destroy_range_00432050(last - 8, last);
    write_field(vector, 0x8, read_field<std::byte*>(vector, 0x8) - 8);
    out_iter[1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(where));
    out_iter[0] = iter_container;
}

void* string_int_tree_copy_subtree_0058bd40(void* tree, const void* node, void* parent, TreeCopyHost& host) {
    void* result = read_field<void*>(tree, 0x4);
    if (read_field<std::uint8_t>(node, 0x19) == 0) {
        void* head = result;
        void* copy = host.allocate_node_00504bd0(tree, head, parent, head, at(node, 0xC),
            read_field<std::uint8_t>(node, 0x18));
        if (read_field<std::uint8_t>(result, 0x19) != 0) result = copy;
        write_field(copy, 0x0, string_int_tree_copy_subtree_0058bd40(tree, read_field<const void*>(node, 0x0), copy, host));
        write_field(copy, 0x8, string_int_tree_copy_subtree_0058bd40(tree, read_field<const void*>(node, 0x8), copy, host));
    }
    return result;
}

void vector_tidy_destroy(void* vector, std::size_t element_size, void (*destroy)(void*, void*), void* context,
    ContainerHost& host) {
    std::byte* first = read_field<std::byte*>(vector, 0x4);
    if (first == nullptr) return;
    for (std::byte* p = first; p != read_field<std::byte*>(vector, 0x8); p += element_size) destroy(context, p);
    host.free_block(read_field<void*>(vector, 0x4));
    for (std::size_t offset : {0x4u, 0x8u, 0xCu}) write_field<std::uint32_t>(vector, offset, 0);
}

void debug_text_list_destruct_00590250(void* list, DebugTextListHost& host) {
    write_field<std::uint32_t>(list, 0x0, 0x00CEFC00u);
    host.clear_00544a70(list);
    if (void* first = read_field<void*>(list, 0x14)) {
        host.destroy_range_00543c20(first, read_field<void*>(list, 0x18), list);
        host.free_block(read_field<void*>(list, 0x14));
    }
    for (std::size_t offset : {0x14u, 0x18u, 0x1Cu}) write_field<std::uint32_t>(list, offset, 0);
}

void voice_observer_destruct_005b8070(void* observer, VoiceObserverHost& host) {
    host.observer_link_destroy_base_004b7ef0(at(observer, 0x18));
    host.voice_slot_destroy_005b7fc0(observer);
}
// ---------------------------------------------------------------------------
// HUD map arithmetic and lookups (batch 2)
// ---------------------------------------------------------------------------

void hud_map_animate_zoom_0059ab10(void* screen, float dt) noexcept {
    const bool closing = read_field<std::uint8_t>(screen, 0x88) != 0;
    // cc13-fixture8: main thread (005B45E6 in the HUD map update), so the FMUL and the
    // FADD/FSUBP each round to 24 bits.
    const double step = cc13x87::x87_mul24(dt, 1.5);  // FMUL qword 00CE3D78
    const double zoom = read_field<float>(screen, 0x84);
    write_field(screen, 0x84, static_cast<float>(closing ? cc13x87::x87_add24(step, zoom)     // 0059AB24
                                                         : cc13x87::x87_sub24(zoom, step)));  // 0059AB3C
    if (!closing && 0.0f > read_field<float>(screen, 0x84)) write_field(screen, 0x84, 0.0f);
    if (closing && read_field<float>(screen, 0x84) > 1.0f) write_field(screen, 0x84, 1.0f);
    if (!closing && read_field<float>(screen, 0x84) == 0.0f) write_field<std::uint8_t>(screen, 0x88, 1);
    if (read_field<std::uint8_t>(screen, 0x88) != 0 && read_field<float>(screen, 0x84) == 1.0f)
        write_field<std::uint8_t>(screen, 0x88, 0);
}

float hud_map_zoom_scale_for_mode_0059abc0(const void* screen) noexcept {
    double factor;
    switch (read_field<std::uint32_t>(screen, 0x2C)) {
    case 0: factor = 0.800000011920929; break;   // 00CE3D40
    case 1: factor = 0.699999988079071; break;   // 00CEFFA0
    case 2: factor = 0.6000000238418579; break;  // 00CEFF98
    default: factor = 0.5; break;                // 00D7A280 (case 3 and above 3)
    }
    return static_cast<float>(read_field<float>(screen, 0x5BC) * factor);
}

void game_world_bounds_center_0059c920(const void* game, float* out) noexcept {
    const float sx = read_field<float>(game, 0x7128) + read_field<float>(game, 0x711C);
    const float sy = read_field<float>(game, 0x712C) + read_field<float>(game, 0x7120);
    const float sz = read_field<float>(game, 0x7130) + read_field<float>(game, 0x7124);
    out[0] = static_cast<float>(sx * 0.5);
    out[1] = static_cast<float>(sy * 0.5);
    out[2] = static_cast<float>(0.5 * sz);
}

bool world_point_beyond_map_margin_0059c9b0(const void* game, const float* p, float margin) noexcept {
    // x87 forms each bound in 80-bit; double holds min-margin exactly unless the
    // exponents differ by more than 29.
    const double m = margin;
    if (read_field<float>(game, 0x711C) - m > p[0]) return true;
    if (p[0] > read_field<float>(game, 0x7128) + m) return true;
    if (p[2] > read_field<float>(game, 0x7124) + m) return true;
    if (read_field<float>(game, 0x7130) - m > p[2]) return true;
    return false;
}

bool hud_map_is_point_inside_map_area_0059d410(const float* p, bool wide_layout) noexcept {
    const double x = p[0];
    if (wide_layout) {
        if (!(x > -0.10000000894069672)) return false;  // 00CEFFD8
        if (!(1.1f > p[0])) return false;               // 00CE6448 (COMISS)
    } else {
        if (!(x > 0.060416667411724724)) return false;  // 00CEFFD0
        if (!(0.9395833214124044 > x)) return false;    // 00CEFFC8
    }
    const double y = p[1];
    return y > 0.05000000074505806 && 0.949999988079071 > y;  // 00D7A270, 00CEFFB0
}

std::int32_t hud_map_classify_offscreen_edge_0059a910(const void* classifier, const float* p, EdgeReferenceHost& host) {
    void* view = read_field<void*>(classifier, 0x4);
    const double x = p[0];
    const double y = p[1];
    if (static_cast<double>(host.reference_point_c4(view)[0]) < x) {  // JBE not taken
        return static_cast<double>(host.reference_point_c4(view)[2]) > y ? 0 : 2;
    }
    return static_cast<double>(host.reference_point_c4(view)[2]) > y ? 1 : 3;
}

void hud_map_icon_start_blink_005a21e0(void* icon, float now_00f876a4, BlinkHost& host) {
    write_field<std::uint32_t>(icon, 0x118, 5);
    write_field(icon, 0x110, static_cast<float>(now_00f876a4 + 0.0));  // FADD qword 00D7A258 = 0.0
    write_field(icon, 0x114, now_00f876a4);
    write_field(icon, 0x108, read_field<float>(icon, 0x100));
    write_field(icon, 0x10C, read_field<float>(icon, 0x104));
    write_field(icon, 0xF8, 0.0f);
    write_field(icon, 0xFC, 0.0f);
    host.release_widgets_005a1c60(icon, 0);
}

const float* static_white_0059adc0(std::uint32_t& guard_00e19528, float* colour_00e19518) noexcept {
    if ((guard_00e19528 & 1) == 0) {
        guard_00e19528 |= 1;
        for (int i = 0; i < 4; ++i) colour_00e19518[i] = 1.0f;  // 00D7A24C
    }
    return colour_00e19518;
}

bool award_hint_is_active_005b5d50(const void* hint) noexcept {
    // UCOMISS + LAHF/TEST 44h/JNP: equal-and-ordered is false; NaN is true.
    const float v = read_field<float>(hint, 0xC0);
    return !(v == 0.0f);
}

namespace {
void* find_unit_icon_node(void* screen, std::uint32_t unit, UnitIconMapHost& host) {
    void* map = at(screen, 0x4F4);
    const std::array<void*, 2> it = host.lower_bound_pair_005a1800(map, unit);
    void* end = read_field<void*>(map, 0x4);
    if (it[0] == nullptr || it[0] != map) host.invalid_iterator_00bf6713();
    if (it[1] == end) return nullptr;
    if (it[0] == nullptr) host.invalid_iterator_00bf6713();
    if (it[1] == read_field<void*>(it[0], 0x4)) host.invalid_iterator_00bf6713();
    return it[1];
}
}  // namespace

void* hud_map_find_unit_icon_005a22d0(void* screen, std::uint32_t unit, UnitIconMapHost& host) {
    void* node = find_unit_icon_node(screen, unit, host);
    return node == nullptr ? nullptr : read_field<void*>(node, 0x10);
}

void hud_map_refresh_unit_icon_texture_005a31a0(void* screen, std::uint32_t unit, UnitIconMapHost& host) {
    if (read_field<std::uint8_t>(screen, 0x5) == 0) return;
    if (void* node = find_unit_icon_node(screen, unit, host))
        host.refresh_side_texture_0059d2a0(read_field<void*>(node, 0x10));
}

void hud_minimap_icon_map_find_005be110(void* map, void** out_iter, const std::uint32_t* key) noexcept {
    void* head = read_field<void*>(map, 0x4);
    void* bound = head;
    void* node = read_field<void*>(head, 0x4);
    while (read_field<std::uint8_t>(node, 0x15) == 0) {
        if (read_field<std::uint32_t>(node, 0xC) < *key) {
            node = read_field<void*>(node, 0x8);
        } else {
            bound = node;
            node = read_field<void*>(node, 0x0);
        }
    }
    out_iter[0] = map;
    out_iter[1] = (bound == head || *key < read_field<std::uint32_t>(bound, 0xC)) ? head : bound;
}
// ---------------------------------------------------------------------------
// Multiplayer mode selector and lobby settings (batch 2)
// ---------------------------------------------------------------------------

namespace {
void route_mode_selector_listeners(void* screen, bool custom, ModeSelectorHost& host) {
    void* plain = custom ? nullptr : screen;
    host.set_listener_fields_00aa6bc0(widget_at(screen, 0x20), plain ? at(plain, 0xC) : nullptr, 0);
    host.list_box_set_listener_00a9ac40(widget_at(screen, 0x20), plain ? at(plain, 0x8) : nullptr);
    void* custom_owner = custom ? screen : nullptr;
    host.set_listener_fields_00aa6bc0(widget_at(screen, 0xA8), custom_owner ? at(custom_owner, 0xC) : nullptr, 0);
    host.list_box_set_listener_00a9ac40(widget_at(screen, 0xA8), custom_owner ? at(custom_owner, 0x8) : nullptr);
}
}  // namespace

void mode_selector_set_custom_mode_005ec4c0(void* screen, bool custom, ModeSelectorHost& host) {
    host.set_shown(widget_at(screen, 0x1C), custom);
    write_field<std::uint8_t>(screen, 0xE0, custom ? 1 : 0);
    host.set_enabled(widget_at(screen, 0xA8), custom);
    host.set_enabled(widget_at(screen, 0x20), !custom);
    route_mode_selector_listeners(screen, custom, host);
    if (custom) {
        host.set_shown(widget_at(screen, 0xA8), true);
        write_field<std::uint8_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(widget_at(screen, 0xA8))), 0x77, 0);
        host.list_box_select_index_00a9c7c0(widget_at(screen, 0xA8), 0);
        host.refresh_server_info_005ebdd0(screen);
    }
    host.input_update_zero();
}

void mode_selector_go_back_005eca90(void* screen, std::int32_t& index_00e19614, ModeSelectorHost& host) {
    if (read_field<std::uint8_t>(screen, 0xE0) != 0) {
        if (read_field<std::int32_t>(screen, 0xF8) != 3) return;
        host.set_shown(widget_at(screen, 0x1C), false);
        write_field<std::uint8_t>(screen, 0xE0, 0);
        host.set_enabled(widget_at(screen, 0xA8), false);
        host.set_enabled(widget_at(screen, 0x20), true);
        route_mode_selector_listeners(screen, false, host);
        host.input_update_zero();
        return;
    }
    if (host.game_byte_218c()) {
        host.clear_game_byte_218c();
        host.multi_main_refresh_online_state_005e7900();
    } else {
        host.pop_request_interface_006881f0();
    }
    index_00e19614 = 0;
}

void mode_selector_on_list_select_005ec990(void* listener, std::uint32_t list_box, std::int32_t mode_00e19610,
    std::int32_t& index_00e19614, ModeSelectorHost& host) {
    if (host.multi_menu_current_interface_20() != 0x10) return;
    index_00e19614 = host.list_box_selected_index_00a9c920(list_box);
    if (index_00e19614 > 5) return;
    if (mode_00e19610 == 0) {
        host.session_set_mode_host_00770e40();
        host.push_request_interface_00688ad0(0x14);
    } else if (mode_00e19610 == 1) {
        mode_selector_set_custom_mode_005ec4c0(at(listener, 0) - 8, true, host);
    } else if (mode_00e19610 == 2) {
        host.session_set_mode_0076fad0(2);
        host.push_request_interface_00688ad0(0x1A);
    }
    const std::uint32_t row = static_cast<std::uint32_t>(index_00e19614);
    const std::int32_t mode = row <= 5 ? kModeSelectorRowModes[row] : 0;  // JA: unsigned
    write_field(at(listener, 0) - 8, 0xEC, host.list_box_selected_index_00a9c920(list_box));
    host.mode_selector_set_mode_005d5660(mode);
    host.mode_selector_apply_005662f0(mode);
}

void mode_selector_listener_current18_005eaac0(const void* listener, std::uint32_t widget, bool focused,
    const char* empty_00e19618, WidgetHost& host) {
    const void* w = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(widget));
    if (host.type(widget) == 3) {
        const char* text = read_field<const char*>(w, 0xF0);
        if (text == nullptr) text = empty_00e19618;
        const auto c = static_cast<std::uint8_t>(*text);
        if (c != 0xA2 && c != 0xA3) return;
        host.set_scale(widget, focused ? kFocusedScale : kUnfocusedScale);
        return;
    }
    if (read_field<std::uint8_t>(listener, 0xD4) != 0) {
        if (widget != widget_at(listener, 0xA4) && widget != widget_at(listener, 0xA8)) return;
    } else if (widget == widget_at(listener, 0x90) && read_field<std::uint8_t>(listener, 0x78) != 0) {
        return;
    }
    if (host.type(widget) == 6) {
        host.set_frame_highlight(widget, focused, 0, 1.0f);
        return;
    }
    if (host.type(widget) == 0x12) host.set_toggle_highlight(widget, focused);
}

void lobby_check_settings_changed_005e50d0(void* screen, std::uint32_t* settings_00e1955c, LobbySettingsHost& host) {
    for (int i = 0; i < 14; ++i) {
        if (settings_00e1955c[i] != read_field<std::uint32_t>(screen, 0x18u + i * 4u)) {
            host.session_request_player_ready_00771dd0(0, 1);
            break;
        }
    }
    // 00E19560..00E1956C are settings[1..4] against +1Ch..+28h.
    bool changed = false;
    for (int i = 1; i <= 4; ++i) changed = changed || settings_00e1955c[i] != read_field<std::uint32_t>(screen, 0x18u + i * 4u);
    if (changed && host.menus_present() && host.multi_menu_query_00687860(0xB, 0)) host.tactical_library_leave_00515bc0();
    for (int i = 0; i < 14; ++i) settings_00e1955c[i] = read_field<std::uint32_t>(screen, 0x18u + i * 4u);
    // 0059518D: CMOVBE on an unsigned compare with 3.
    const std::uint32_t mode = settings_00e1955c[2] <= 3 ? settings_00e1955c[3] : settings_00e1955c[2];
    host.game_set_game_mode_004bc890(mode, 0);
    if (void* lobby = host.lobby_screen_if_visible()) {
        write_field<std::uint32_t>(lobby, 0x10, 1);
        host.refresh_setting_rows_005e3580(lobby);
    }
}

void lobby_step_map_index_005e1bf0(void* screen, std::int32_t delta, std::int32_t count_00f1af2c,
    std::uint32_t& map_00e19594, std::uint32_t mode_00e19564, LobbySettingsHost& host) {
    std::int32_t index = static_cast<std::int32_t>(map_00e19594);
    if (count_00f1af2c > 0) {
        for (std::int32_t tries = 0; tries < count_00f1af2c; ++tries) {
            index = (count_00f1af2c + index + delta) % count_00f1af2c;
            if (static_cast<std::uint32_t>(index) >= static_cast<std::uint32_t>(host.mission_record_count()))
                host.invalid_iterator_00bf6713();
            void* record = host.mission_record(index);
            void* table = read_field<std::uint8_t>(record, 0x424) != 0 ? at(record, 0x428)
                                                                        : at(host.option_registry_00576b10(), 0x28);
            if (host.mode_available(table, 2, mode_00e19564) == 1) break;
        }
        if (static_cast<std::uint32_t>(index) != map_00e19594) map_00e19594 = static_cast<std::uint32_t>(index);
    }
    if (delta != 0) write_field<std::uint32_t>(screen, 0x27C, 0);
}
// ---------------------------------------------------------------------------
// Main-menu async completions (batch 2)
// ---------------------------------------------------------------------------

void main_menu_finish_async_text_request_00590a00(void* screen, MainMenuAsyncHost& host, LegacyStringHost& strings) {
    if (void* online = host.online()) {
        alignas(8) std::byte local[0x40]{};
        legacy_sbo_record_copy_00588640(local, host.online_status_record_3c(online), strings);
        if (read_field<std::uint8_t>(local, 0x3B) != 0) {
            host.online_virtual_38(host.online());
            write_field<std::uint8_t>(screen, 0x500, 1);
        }
        if (read_field<std::uint32_t>(local, 0x18) >= 0x10) host.free_block(read_field<void*>(local, 0x4));
    }
    host.build_top_level_page_00584ae0(screen, 0);
}

void main_menu_on_profile_loaded_00590d80(MainMenuAsyncHost& host, LegacyStringHost& strings,
    NativeStringStorage& storage) {
    if (host.lua_reader_pending_count() != 0) {
        main_menu_finish_async_text_request_00590a00(host.main_menu_screen(), host, strings);
        return;
    }
    host.settings_apply_all_008d5b50();
    host.build_top_level_page_00584ae0(host.main_menu_screen(), 1);
    alignas(4) std::byte id[8];
    profile_copy_mission_id_00586150(host.profile_650(), id, storage);
    host.mission_tree_select_by_id_005c35d0(host.mission_tree(), id);
    destroy_native_string_header_0041dd20(id, storage);
}

void main_menu_on_play_mission_prompt_result_thunk_00592ad0(std::int32_t result, MainMenuAsyncHost& host) {
    host.on_play_mission_prompt_result_00592640(host.main_menu_screen(), result);
}
// ---------------------------------------------------------------------------
// Options and pause helpers (batch 2)
// ---------------------------------------------------------------------------

void options_find_auto_control_group_005f0390(std::uint32_t parent, void* listener, OptionsWidgetHost& host) {
    host.set_listener_fields_00aa6bc0(host.find_child_by_name_00aa7e00(parent, kAutoControlGroupName), listener, 0);
}

void options_set_row_var_text_005f0730(void* screen, std::int32_t row, const void* source, OptionsWidgetHost& host) {
    const std::uint32_t row_widget = host.list_box_row_00a9be00(widget_at(screen, 0x88), row);
    host.text_set_localised_source_00abaed0(host.find_child_by_name_00aa7e00(row_widget, kRowVarTextName), source, 1);
}

void options_on_slider_event_005f4c10(void* listener, std::uint32_t widget, OptionsWidgetHost& host) {
    if (host.input_devices_empty()) host.invalid_iterator_00bf6713();
    if (host.widget_type_5c(widget) != 2) return;
    const void* w = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(widget));
    if (read_field<std::int32_t>(w, 0xD8) == 1) return;
    const std::int32_t value = host.slider_value_from_row_005f2290(widget);
    if (value == 0) return;
    host.list_box_select_row_00a9c740(widget_at(listener, 0x7C), read_field<std::int32_t>(w, 0x70));
    host.adjust_focused_value_005f42d0(at(listener, 0) - 0xC, value);
}

void hud_pause_run_lua_command_005fd360(std::uint32_t length, char* data, LuaCommandHost& host,
    NativeStringStorage& storage) {
    if (length != 0) {
        const char* text = data != nullptr ? data : "";  // 00E19684
        const bool failed = host.lua_run_string_006b8ad0(text) != 0;
        host.log_004254b0(failed ? kLuaCommandFailed : kLuaCommandRan, text);
    }
    struct Header { std::uint32_t length; char* data; } by_value{length, data};
    destroy_native_string_header_0041dd20(&by_value, storage);
}

void lua_globals_ensure_reader_005f60f0(LuaCommandHost& host) {
    if (host.lua_reader_pending_count() != 0) return;
    host.read_keyboard_archive_from_globals();
    if (void* buffer = host.owner_buffer_30()) {
        host.free_00bf6989(buffer);
        host.clear_owner_buffer_30();
    }
    host.lua_state_owner_close_00b65e80();
    host.lua_reader_destroy_00441a20();
}
// ---------------------------------------------------------------------------
// HUD map and minimap object pools (batch 3)
// ---------------------------------------------------------------------------

void hud_map_delete_marker_objects_005a3aa0(void* screen, MapPoolHost& host) {
    for (std::uint32_t i = 0;; ++i) {
        std::byte* first = read_field<std::byte*>(screen, 0x580);
        if (first == nullptr) break;
        const auto size = static_cast<std::uint32_t>((read_field<std::byte*>(screen, 0x584) - first) / 4);
        if (i >= size) break;
        void* slot = read_field<std::byte*>(screen, 0x580) + i * 4u;
        if (void* object = read_field<void*>(slot, 0)) {
            host.delete_object(object);
            write_field<std::uint32_t>(slot, 0, 0);
        }
    }
    std::byte* first = read_field<std::byte*>(screen, 0x580);
    std::byte* last = read_field<std::byte*>(screen, 0x584);
    if (first > last) host.invalid_iterator_00bf6713();
    if (read_field<std::byte*>(screen, 0x580) > read_field<std::byte*>(screen, 0x584)) host.invalid_iterator_00bf6713();
    if (first != last) write_field(screen, 0x584, first);  // the tail [last, last) moved is empty
}

void* hud_map_acquire_arrow_widget_005a4a00(void* screen, MapPoolHost& host) {
    const std::int32_t index = read_field<std::int32_t>(screen, 0x548) + 1;
    write_field(screen, 0x548, index);
    const void* pool = read_field<const void*>(screen, 0x544);
    const std::int32_t size = pool == nullptr ? 0 : read_field<std::int32_t>(pool, 0x4);
    if (index < size) return read_field<void*>(read_field<const void*>(pool, 0x0), index * 4u);
    void* memory = host.operator_new(0x28);
    void* widget = memory != nullptr ? host.arrow_widget_construct_005a3610(memory, screen) : nullptr;
    host.pointer_array_push_back_0059fad0(at(screen, 0x544), &widget);
    return widget;
}

void hud_map_get_or_create_unit_icon_005b2cb0(void* screen, std::uint32_t unit, MapPoolHost& host) {
    void** slot = host.map_subscript_00538910(at(screen, 0x4F4), unit);
    if (*slot == nullptr) {
        void* memory = host.operator_new(0x1A0);
        *slot = memory != nullptr
            ? host.unit_marker_construct_005a6ad0(memory, unit, read_field<std::uint32_t>(screen, 0xC), at(screen, 0x20))
            : nullptr;
        host.observer_register_pair_00694a60(unit, at(screen, 0x64));
        write_field<std::uint8_t>(screen, 0x598, 1);
    }
    write_field<std::uint8_t>(*slot, 0x8, 1);
    host.update_unit_icon_005adda0(screen, *slot);
}

void* hud_minimap_screen_construct_005c0810(void* screen, MapPoolHost& host) {
    host.frontend_construct_004f7180(screen);
    write_field<std::uint32_t>(screen, 0x08, 0x00CE3CD4u);
    for (std::size_t offset : {0x0Cu, 0x10u, 0x14u}) write_field<std::uint32_t>(screen, offset, 0);
    write_field<std::uint8_t>(screen, 0x18, 0);
    write_field<std::uint32_t>(screen, 0x00, 0x00CF1404u);
    write_field<std::uint32_t>(screen, 0x08, 0x00CF13ECu);
    void* tree = at(screen, 0xF8);
    void* head = host.tree_buy_head_node_005bdd40(tree);
    write_field(tree, 0x4, head);
    write_field<std::uint8_t>(head, 0x15, 1);
    reset_head_links(head);
    write_field<std::uint32_t>(tree, 0x8, 0);
    for (std::size_t offset : {0x108u, 0x10Cu, 0x110u, 0x118u, 0x11Cu, 0x120u, 0x128u, 0x12Cu, 0x130u, 0x138u,
                               0x13Cu, 0x140u, 0x148u, 0x14Cu, 0x150u, 0x180u, 0x184u})
        write_field<std::uint32_t>(screen, offset, 0);
    return screen;
}
void lobby_register_005e1820(void* screen, LobbyRegisterHost& host) {
    host.frontend_register_004f71d0(screen);
    write_field(screen, 0x14, host.load_page_00aa5840(kLobbyPage));
    write_field(screen, 0x18, host.load_page_00aa5840(kLobbySettingsPage));
    host.screen_bind_layout(screen);
    host.load_weather_table_005e12a0(screen);
    for (std::size_t offset : {0x2F4u, 0x2F0u, 0x2FCu, 0x2F8u}) write_field<std::int32_t>(screen, offset, -1);
    for (std::size_t offset : {0x2C8u, 0x2CCu, 0x2C0u, 0x2C4u}) write_field<std::uint32_t>(screen, offset, 0);
    write_field<std::uint8_t>(screen, 0x2DC, 0);
    write_field<std::uint32_t>(screen, 0x274, 1);
}

void options_register_005f0250(void* screen, ScreenLifetimeHost& host) {
    host.frontend_register_004f71d0(screen);
    write_field(screen, 0x7C, host.load_page_00aa5840(kOptionsBackgroundPage));
    write_field(screen, 0x80, host.load_page_00aa5840(kOptionsPage));
    write_field<std::uint32_t>(screen, 0xA8, 0);
    write_field<std::uint8_t>(screen, 0x25C, 0);
    write_field<std::uint8_t>(screen, 0x78, 0);
    host.screen_bind_layout(screen);
}
void hud_minimap_screen_exit_005c0e10(void* screen, MinimapLifetimeHost& host) {
    void* tree = at(screen, 0xF8);
    void* node = read_field<void*>(read_field<void*>(tree, 0x4), 0x0);  // begin()
    for (;;) {
        void* end = read_field<void*>(tree, 0x4);
        if (node == end) break;
        if (node == read_field<void*>(tree, 0x4)) host.invalid_iterator_00bf6713();
        if (void* value = read_field<void*>(node, 0x10)) host.delete_object(value);
        host.observer_unregister_pair_006952a0(read_field<std::uint32_t>(node, 0xC), at(screen, 0x8));
        node = host.tree_next_005bcf70(node);
    }
    void* root = read_field<void*>(read_field<void*>(tree, 0x4), 0x4);
    while (read_field<std::uint8_t>(root, 0x15) == 0) {
        host.tree_erase_subtree_005bdc20(tree, read_field<void*>(root, 0x8));
        void* left = read_field<void*>(root, 0x0);
        host.free_block(root);
        root = left;
    }
    void* head = read_field<void*>(tree, 0x4);
    write_field(head, 0x4, head);
    write_field<std::uint32_t>(tree, 0x8, 0);
    write_field(head, 0x0, head);
    write_field(head, 0x8, head);
    for (std::size_t offset : {0x154u, 0x158u, 0x15Cu, 0x160u, 0x164u}) write_field<std::int32_t>(screen, offset, -1);
    host.hide_unused_markers_005c0bd0(screen);
}

void hud_minimap_screen_destruct_005c0520(void* screen, MinimapLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF1404u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF13ECu);
    destroy_pages(screen, {0x1C}, host);
    host.destroy_native_string(at(screen, 0x180));
    for (std::size_t offset : {0x148u, 0x138u, 0x128u, 0x118u, 0x108u}) free_vector(screen, offset, host);
    void* tree = at(screen, 0xF8);
    void* head = read_field<void*>(tree, 0x4);
    host.tree_erase_range_005bfac0(tree, read_field<void*>(head, 0x0), head);
    host.free_block(read_field<void*>(tree, 0x4));
    write_field<std::uint32_t>(tree, 0x4, 0);
    write_field<std::uint32_t>(tree, 0x8, 0);
    host.callback_owner_destroy_00695870(at(screen, 0x8));
    host.frontend_destruct(screen);
}
void* options_screen_construct_005f6030(void* screen, OptionsLifetimeHost& host) {
    host.frontend_construct_004f7180(screen);
    write_field<std::uint32_t>(screen, 0x0, 0x00CF3974u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF3960u);
    write_field<std::uint32_t>(screen, 0xC, 0x00CF3940u);
    for (std::size_t offset : {0xC4u, 0xCCu, 0xD0u, 0xD4u}) write_field<std::uint32_t>(screen, offset, 0);
    host.game_settings_construct_008d7710(at(screen, 0xD8));
    host.game_settings_construct_008d7710(at(screen, 0x194));
    write_field<std::uint8_t>(screen, 0x264, 0);
    write_field<std::uint8_t>(screen, 0x265, 0);
    write_field<std::uint32_t>(screen, 0x70, 10);
    write_field(screen, 0x26C, host.malloc_00bf9f1a(10));
    return screen;
}

void options_screen_destruct_005f1bb0(void* screen, OptionsLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF3974u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF3960u);
    write_field<std::uint32_t>(screen, 0xC, 0x00CF3940u);
    host.free_00bf9dc8(read_field<void*>(screen, 0x26C));
    destroy_pages(screen, {0x7C, 0x80}, host);
    host.game_settings_destroy_008d78d0(at(screen, 0x194));
    host.game_settings_destroy_008d78d0(at(screen, 0xD8));
    write_field<std::uint32_t>(screen, 0xC4, 0);
    free_vector(screen, 0xCC, host);
    write_field<std::uint32_t>(screen, 0x8, 0x00CEB0FCu);
    host.frontend_destruct(screen);
}
void* hud_pause_screen_construct_005ffcb0(void* screen, PauseLifetimeHost& host) {
    host.frontend_construct_004f7180(screen);
    write_field<std::uint32_t>(screen, 0x0, 0x00CF3EF4u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF3EE0u);
    write_field<std::uint32_t>(screen, 0xC, 0x00CF3EC0u);
    write_field(screen, 0x68, host.list_allocate_sentinel_00582280(at(screen, 0x64)));
    write_field<std::uint32_t>(screen, 0x6C, 0);
    host.scroller_construct_00683610(at(screen, 0x9C));
    void* tree = at(screen, 0x11C);
    void* head = host.tree_buy_head_node_005fd1b0(tree);
    write_field(tree, 0x4, head);
    write_field<std::uint8_t>(head, 0x19, 1);
    reset_head_links(head);
    write_field<std::uint32_t>(tree, 0x8, 0);
    write_field(screen, 0x12C, host.string_list_allocate_sentinel_004c3020(at(screen, 0x128)));
    write_field<std::uint32_t>(screen, 0x130, 0);
    return screen;
}

void hud_pause_screen_destruct_005ff980(void* screen, PauseLifetimeHost& host) {
    write_field<std::uint32_t>(screen, 0x0, 0x00CF3EF4u);
    write_field<std::uint32_t>(screen, 0x8, 0x00CF3EE0u);
    write_field<std::uint32_t>(screen, 0xC, 0x00CF3EC0u);
    destroy_pages(screen, {0x14, 0x18, 0x1C}, host);
    host.string_list_clear_storage_004d05e0(at(screen, 0x128));
    host.free_block(read_field<void*>(screen, 0x12C));
    write_field<std::uint32_t>(screen, 0x12C, 0);
    void* tree = at(screen, 0x11C);
    void* head = read_field<void*>(tree, 0x4);
    host.tree_erase_range_005fedc0(tree, read_field<void*>(head, 0x0), head);
    host.free_block(read_field<void*>(tree, 0x4));
    write_field<std::uint32_t>(tree, 0x4, 0);
    write_field<std::uint32_t>(tree, 0x8, 0);
    host.list_tidy_005822a0(at(screen, 0x64));
    write_field<std::uint32_t>(screen, 0x8, 0x00CEB0FCu);
    host.frontend_destruct(screen);
}
// ---------------------------------------------------------------------------
// Mission tree tables (batch 3)
// ---------------------------------------------------------------------------

void mission_tree_read_int_vector_005c4ad0(std::uint32_t key_kind, std::uint32_t key_value, void* vector,
    TableReaderHost& host) {
    if (read_field<std::uintptr_t>(vector, 0x4) > read_field<std::uintptr_t>(vector, 0x8)) host.invalid_iterator_00bf6713();
    if (read_field<std::uintptr_t>(vector, 0x4) > read_field<std::uintptr_t>(vector, 0x8)) host.invalid_iterator_00bf6713();
    host.erase_all_004685a0(vector);
    host.enter_04(key_kind, key_value);
    for (std::int32_t index = 1; host.has_index_14(index); ++index) {
        const std::int32_t value = host.read_int_10(index);  // lands in the key's stack slot
        std::byte* first = read_field<std::byte*>(vector, 0x4);
        std::byte* last = read_field<std::byte*>(vector, 0x8);
        const std::uintptr_t size = first == nullptr ? 0 : static_cast<std::uintptr_t>(last - first) / 4;
        if (first != nullptr && size < static_cast<std::uintptr_t>(read_field<std::byte*>(vector, 0xC) - first) / 4) {
            write_field(last, 0, value);
            write_field(vector, 0x8, last + 4);
        } else {
            if (first > read_field<std::byte*>(vector, 0x8)) host.invalid_iterator_00bf6713();
            host.insert_one_00441f30(vector, read_field<void*>(vector, 0x8), &value);
        }
    }
    host.leave_08();
}

void mission_side_block_construct_005c4be0(void* block) noexcept {
    write_field<std::uint8_t>(block, 0x0, 0);
    write_field<std::uint32_t>(block, 0x4, 0);
    write_field<std::uint32_t>(block, 0x8, 0);
    for (std::size_t k = 0; k < 20; ++k)
        for (std::size_t word = 0; word < 3; ++word) write_field<std::uint32_t>(block, 0x18 + 0x10 * k + 4 * word, 0);
}
void mission_score_record_destroy_members_00594290(void* record, MissionScoreHost& host) {
    host.record_destruct_00593570(record);
}

// ---------------------------------------------------------------------------
// Multiplayer main menu list handlers (packet cc13-fe1b)
// ---------------------------------------------------------------------------

namespace {
void show_list_help(std::uint32_t list_box, std::uint32_t table, MultiMainListHost& host) {
    host.command_bar_set_help_line_0054a0c0(host.help_text(table, host.list_box_selected_data_00a9c990(list_box)));
}
std::uint8_t list_byte(std::uint32_t list_box, std::size_t offset) {
    return read_field<std::uint8_t>(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(list_box)), offset);
}
void set_list_byte(std::uint32_t list_box, std::size_t offset, std::uint8_t value) {
    write_field(reinterpret_cast<void*>(static_cast<std::uintptr_t>(list_box)), offset, value);
}
}  // namespace

void multi_main_on_listbox_selection_changed_005e73a0(void* listener, std::uint32_t list_box,
    MultiMainListHost& host) {
    std::uint32_t table;
    if (list_box == widget_at(listener, 0x1C) && list_byte(list_box, 0x85) != 0) table = kQuickHelpTable;
    else if (list_box == widget_at(listener, 0x18) && list_byte(list_box, 0x85) != 0) table = kRankedHelpTable;
    else return;
    host.set_enabled(list_box, true);
    show_list_help(list_box, table, host);
}

void multi_main_on_listbox_activate_005e7160(void* listener, std::uint32_t list_box,
    const MultiMainHostingFlags& flags, MultiMainListHost& host) {
    const std::int32_t data = host.list_box_selected_data_00a9c990(list_box);
    void* screen = at(listener, 0) - 8;
    if (list_box == widget_at(listener, 0x1C)) {
        write_field(listener, 0xEC, host.list_box_selected_index_00a9c920(list_box));
        if (data == 0) {
            host.start_hosting_005e6f00(screen);
        } else if (data == 1) {
            *flags.e08701 = 0;
            *flags.e08700 = 0;
            *flags.e19610 = 2;
            host.push_request_interface_00688ad0(0x10);
        } else if (data == 2) {
            host.open_friends_005e6c70(screen);
        }
        return;
    }
    if (list_box != widget_at(listener, 0x18)) return;
    if (data == 0 || data == 1) {
        host.set_online_ranked_4d(data == 0);
        host.frame_set_title_005189b0(data == 0 ? kRankedTitle : kPlayerTitle);
        host.set_buttons_visible_005e6b10(screen, data == 0);
    }
    const std::uint32_t ranked = widget_at(listener, 0x18);
    host.set_enabled(ranked, false);
    host.set_shown(ranked, false);
    set_list_byte(ranked, 0x77, 1);
    const std::uint32_t quick = widget_at(listener, 0x1C);
    host.set_enabled(quick, true);
    host.set_shown(quick, true);
    set_list_byte(quick, 0x77, 0);
    show_list_help(list_box, kQuickHelpTable, host);  // 005E7312: ECX = the activated (ranked) list
    host.input_update_zero();
}
namespace {
std::uint8_t first_glyph(std::uint32_t widget, const char* empty) {
    const char* text = read_field<const char*>(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(widget)), 0xF0);
    return static_cast<std::uint8_t>(*(text != nullptr ? text : empty));
}
void press_style(std::uint32_t widget, PressHost& host) {
    if (host.type(widget) == 6) {
        host.set_frame_state_88(widget, 2, 0, 1.0f);
        return;
    }
    if (host.type(widget) == 0x12) host.set_toggle_state_84(widget, 2);
}
}  // namespace

void hud_pause_on_press_005fdb50(void* listener, std::uint32_t widget, std::int32_t& page_00e19660,
    const char* empty_00e19688, PressHost& host) {
    if (host.type(widget) == 3) {
        host.set_scale(widget, 1.0f);
        const std::uint8_t glyph = first_glyph(widget, empty_00e19688);
        if (glyph == 0xA2) {
            const std::uint32_t list_box = widget_at(listener, 0x130);
            host.list_listener_activate_04(at(listener, 0) - 4, host.list_box_value_00425e50(list_box), list_box);
            return;
        }
        if (glyph == 0xB7 || glyph == 0xB8 || glyph != 0xA3) return;
        page_00e19660 = 0;
        host.hud_pause_resume_005fd440(at(listener, 0) - 0xC);
        return;
    }
    if (widget == widget_at(listener, 0x70)) host.scroller_begin_thumb_drag_00683a40(at(listener, 0x90));
    else if (widget == widget_at(listener, 0x6C)) host.scroller_scroll_toward_end_00683700(at(listener, 0x90), 0.5f);
    else if (widget == widget_at(listener, 0x68)) host.scroller_scroll_toward_start_00683680(at(listener, 0x90), 0.5f);
    press_style(widget, host);
}

void mode_selector_on_press_005ed450(void* listener, std::uint32_t widget, const char* empty_00e19618,
    PressHost& host) {
    void* screen = at(listener, 0) - 0xC;
    if (host.type(widget) == 3) {
        const std::uint8_t glyph = first_glyph(widget, empty_00e19618);
        if (glyph == 0xA2) {
            if (read_field<std::uint8_t>(listener, 0xD4) != 0) host.mode_selector_refresh_map_list_005ec250(screen);
            else host.list_listener_activate_04(at(listener, 0) - 4, widget, widget_at(listener, 0x14));
            return;
        }
        if (glyph == 0xA3) {
            host.mode_selector_go_back_005eca90(screen);
            return;
        }
        for (std::int32_t k = 0; k < 4; ++k)
            if (widget == widget_at(listener, 0xB4u + k * 4u))
                host.list_box_select_index_00a9c7c0(widget_at(listener, 0x9C), k);
        return;
    }
    if (read_field<std::uint8_t>(listener, 0xD4) != 0) {
        if (widget == widget_at(listener, 0xA4)) host.mode_selector_step_option_005ecba0(screen, -1);
        else if (widget == widget_at(listener, 0xA8)) host.mode_selector_step_option_005ecba0(screen, 1);
        else return;
    } else if (widget == widget_at(listener, 0x28)) {
        host.scroller_set_auto_scroll_00683300(at(listener, 0x30), 2, kModeSelectorAutoScrollSpeed, 1);
    } else if (widget == widget_at(listener, 0x24)) {
        host.scroller_set_auto_scroll_00683300(at(listener, 0x30), 1, kModeSelectorAutoScrollSpeed, 1);
    } else if (widget == widget_at(listener, 0x90)) {
        host.scroller_begin_thumb_drag_00683a40(at(listener, 0x30));
    }
    press_style(widget, host);
}
void mode_selector_on_listbox_selection_changed_005ebc90(void* self, std::uint32_t list_box,
    std::int32_t& index_00e19614, ModeSelectorTextHost& host) {
    if (list_box == widget_at(self, 0xA0)) return;
    index_00e19614 = host.list_box_selected_index_00a9c920(list_box);
    host.command_bar_set_help_line_0054a0c0(host.table_string(0x00E089A4u, index_00e19614));
    if (index_00e19614 > 5) return;
    host.text_set_localised_cstring_00abbe50(widget_at(self, 0x20), host.table_string(0x00E089BCu, index_00e19614), 1);
    host.text_set_localised_cstring_00abbe50(widget_at(self, 0x24), host.table_string(0x00E0898Cu, index_00e19614), 1);
    host.set_frame_state_88(widget_at(self, 0x1C), index_00e19614 & 0xFFFF, 0, 1.0f);  // MOVZX word
    host.scroller_attach_content_00683790(at(self, 0x34), 0);
    const float size_y = host.widget_size_y_00aa6740(widget_at(self, 0x90));
    const double description = static_cast<double>(host.text_normalized_height_00ab6bd0(widget_at(self, 0x24))) * 1.5;
    const double title = host.text_normalized_height_00ab6bd0(widget_at(self, 0x20));
    host.scroller_set_scroll_range_006834a0(at(self, 0x34), static_cast<float>(title + description - size_y));
}

void hud_narrative_update_005bc920(void* screen, NarrativeUpdateHost& host) {
    const bool shown = read_field<std::int32_t>(screen, 0x54) > 0 || host.narrative_34() != 0 || host.narrative_24() != 0;
    for (std::size_t offset : {0x34u, 0x38u, 0x3Cu, 0x40u}) host.set_shown(widget_at(screen, offset), shown);
    if (!shown) {
        host.text_set_localised_cstring_00abbe50(widget_at(screen, 0x34), "", 1);  // 00CE3A0C
        host.text_set_localised_cstring_00abbe50(widget_at(screen, 0x38), "", 1);
    }
    const float dt = host.game_frame_time_21f0();
    if (0.0f >= dt) return;  // COMISS/JAE; NaN runs the steps
    host.step(0x005BBF10u, screen, dt);
    host.step(0x005BC640u, screen, dt);
    host.step(0x00735100u, host.mission_narrative_21e8(), dt);
    host.step(0x005B9800u, screen, dt);
    host.step(0x005B8C30u, screen, dt);
}
void hud_pause_show_page_005fd890(void* screen, bool ingame_page, bool list_page, PauseShowPageHost& host) {
    const std::uint32_t page = ingame_page ? host.ingame_page_widget() : widget_at(screen, 0x14);
    const std::uint32_t list_box = widget_at(screen, 0x2C);
    if (list_page) {
        host.set_shown(page, false);
        host.set_shown(widget_at(screen, 0x18), true);
        host.frame_select_default_004c9680();
        host.set_enabled(list_box, true);
        host.list_box_select_index_00a9c7c0(widget_at(screen, 0x2C), 0);
        host.list_box_set_listener_00a9ac40(widget_at(screen, 0x2C), at(screen, 0x8));
        host.list_box_virtual_80(widget_at(screen, 0x2C), true);
        return;
    }
    host.set_shown(page, true);
    host.set_shown(widget_at(screen, 0x18), false);
    host.list_box_clear_rows_00a9bec0(widget_at(screen, 0x2C));
    // The row list at +64h (sentinel +68h, size +6Ch): delete each row object,
    // then unlink everything and free the nodes.
    void* list = at(screen, 0x64);
    for (void* node = read_field<void*>(read_field<void*>(list, 0x4), 0x0); node != read_field<void*>(list, 0x4);
         node = read_field<void*>(node, 0x0)) {
        if (void* row = read_field<void*>(node, 0x8)) host.delete_owned(row);
    }
    void* sentinel = read_field<void*>(list, 0x4);
    void* node = read_field<void*>(sentinel, 0x0);
    write_field(sentinel, 0x0, sentinel);
    write_field(sentinel, 0x4, sentinel);
    write_field<std::uint32_t>(list, 0x8, 0);
    while (node != read_field<void*>(list, 0x4)) {
        void* next = read_field<void*>(node, 0x0);
        host.free_block(node);
        node = next;
    }
    if (ingame_page) {
        if (!host.ingame_page_locked_1c())
            host.frame_select_00518d60(host.game_byte_218c() ? kPausedFrame : kPauseMultiTitleFrame);
    } else {
        host.frame_select_00518d60(kPausedFrame);
    }
    write_field<std::uint8_t>(screen, 0x10, 0);
}
void options_screen_enter_005f5bd0(void* screen, std::int32_t page_00e19650, OptionsEnterHost& host) {
    if (read_field<std::uint8_t>(screen, 0x25C) == 0) host.frame_select_00518d60(6, 0, 0, kOptionsMainTitle);
    host.copy_settings_005f52c0(at(screen, 0xD8), host.live_settings_00f88980());
    const std::uint32_t page = widget_at(screen, 0x7C);
    const bool in_mission = host.game_in_mission_61f();
    write_field(screen, 0x84, host.find_child_by_name_00aa7e00(page, in_mission ? kIngameMainListboxName : kMainListboxName));
    host.set_shown(host.find_child_by_name_00aa7e00(widget_at(screen, 0x7C), in_mission ? kMainListboxName : kIngameMainListboxName), false);
    write_field(screen, 0x8C, host.find_child_by_name_00aa7e00(widget_at(screen, 0x84), kMainListboxTextName));
    host.set_shown(widget_at(screen, 0x84), true);
    host.set_enabled(widget_at(screen, 0x84), true);
    if (page_00e19650 == 1) host.build_page(0x005F58F0u, screen);
    else if (page_00e19650 == 4) host.build_page(0x005F40F0u, screen);
    else host.build_page(0x005F54F0u, screen);
    write_field(screen, 0x260, 0.1f);  // 00D7A2F0
    write_field<std::uint8_t>(screen, 0x251, 0);
    host.input_devices_virtual_0c();
}
void hud_order_screen_enter_005fad70(void* screen, HudOrderEnterHost& host) {
    for (std::size_t offset : {0x44u, 0x48u, 0x4Cu, 0x50u}) host.set_shown(widget_at(screen, offset), false);
    host.set_shown(widget_at(screen, 0x54), true);
    if (read_field<std::int32_t>(screen, 0x84) == 0xEC) {
        write_field(screen, 0x74, 0.0f);
        write_field<std::uint8_t>(screen, 0x78, 1);
        write_field(screen, 0x7C, 0.0f);
        write_field<std::uint8_t>(screen, 0x80, 1);
        if (!host.active_hint_is(kPermanentOrderHint)) host.trigger_hint_00690fd0(kPermanentOrderHint, 0);
    }
    const std::uint32_t unit = host.controlled_unit();
    const void* u = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(unit));
    auto set_mode = [&](std::size_t component) {
        const bool single = host.order_mode(read_field<std::uint32_t>(u, component)) == 1;
        write_field<std::uint8_t>(screen, 0x83, single ? 1 : 0);
        host.set_shown(widget_at(screen, 0x10), !single);
        host.set_shown(widget_at(screen, 0x0C), single);
    };
    if (host.is_kind(unit, 6)) {
        set_mode(0x738);
        host.set_shown(widget_at(screen, 0x18), true);
        host.set_shown(widget_at(screen, 0x14), false);
        host.set_shown(widget_at(screen, read_field<std::uint32_t>(u, 0x284) != 0 ? 0x2Cu : 0x28u), true);
        host.set_shown(widget_at(screen, 0x1C), true);
    } else if (host.is_kind(host.controlled_unit(), 0x18)) {
        set_mode(0x348);
        host.set_shown(widget_at(screen, 0x18), false);
        host.set_shown(widget_at(screen, 0x14), true);
        host.set_shown(widget_at(screen, 0x1C), true);
        host.set_shown(widget_at(screen, 0x34), true);
        host.set_shown(widget_at(screen, 0x30), false);
    }
}

// ---------------------------------------------------------------------------
// HudPauseScreen bodies (packet cc13-fe6)
// ---------------------------------------------------------------------------

void hud_pause_screen_enter_00603120(void* screen, std::int32_t page_00e19660, PauseScreenHost& host) {
    host.frame_select_00518d60(kPausedFrame);
    void* hud = host.hud_interface_00e198c4();
    void* order = read_field<void*>(hud, 0x64);
    if (read_field<std::uint8_t>(order, 0x81) != 0 || read_field<std::uint8_t>(order, 0x82) != 0)
        host.hud_order_close_005f9520();
    void* ship = read_field<void*>(hud, 0x78);
    if (read_field<std::uint8_t>(ship, 0x156) != 0 || read_field<std::uint8_t>(ship, 0x157) != 0)
        host.hud_ship_close_order_menu_0064a2f0();
    write_field<std::uint8_t>(read_field<void*>(hud, 0x60), 0x4, 0);

    write_field(screen, 0x13C, host.find_child_by_name_00aa7e00(widget_at(screen, 0x14), kMainListboxName));
    write_field(screen, 0x5C, host.find_child_by_name_00aa7e00(widget_at(screen, 0x13C), kFelkiBalIcon));
    write_field(screen, 0x60, host.find_child_by_name_00aa7e00(widget_at(screen, 0x13C), kFelkiJobbIcon));
    host.set_shown(widget_at(screen, 0x5C), false);
    host.set_shown(widget_at(screen, 0x60), false);
    host.set_shown(widget_at(screen, 0x18), false);
    host.list_box_clear_items_00a9bf90(widget_at(screen, 0x13C));
    const std::uint32_t row_template =
        host.find_child_by_name_00aa7e00(widget_at(screen, 0x13C), kMainListboxTextName);
    host.set_shown(row_template, false);

    void* tracker = nullptr;
    void* game = host.game_00e188a8();
    for (std::size_t i = 0; i < kPauseRowOffsets.size(); ++i) {
        const std::uint32_t row = host.clone_subtree_00aab4c0(row_template, 0);
        write_field(screen, kPauseRowOffsets[i], row);
        host.text_set_localised_cstring_00abbe50(row, kPauseRowKeys[i], 1);
        if (kPauseRowOffsets[i] == 0x15C) {
            // The singleton is fetched after the text is set.
            tracker = host.award_tracker_004e1ca0();
            host.widget_virtual_7c(row, read_field<std::int32_t>(tracker, 0x9C) == 0);
        } else if (kPauseRowOffsets[i] == 0x14C) {
            host.widget_virtual_7c(row,
                read_field<std::uint8_t>(game, 0x218C) == 0 && read_field<std::int32_t>(game, 0x1FE4) == 0);
        }
        host.list_box_insert_row_00a9d750(widget_at(screen, 0x13C), row, 0, 0);
    }
    const std::uint32_t list_box = widget_at(screen, 0x13C);
    host.set_enabled(list_box, true);
    host.list_box_select_index_00a9c7c0(list_box, page_00e19660);
    host.list_box_set_listener_00a9ac40(list_box, at(screen, 0x8));
    host.list_box_virtual_80(list_box, true);
    host.list_box_reset_highlight_00a9cf30(list_box, 1, 1);

    // FSTP dword after each step: the x87 intermediates round to float.
    const std::uint32_t tips = widget_at(screen, 0x15C);
    float offset = static_cast<float>(static_cast<double>(read_field<float>(reinterpret_cast<void*>(
                                          static_cast<std::uintptr_t>(tips)), 0x114)) / kPauseArrowWidthScale);
    offset = static_cast<float>(static_cast<double>(offset) * 0.5);
    offset = static_cast<float>(static_cast<double>(offset) + static_cast<double>(kPauseArrowPad));
    std::array<float, 3> position = host.widget_resolved_position_00aa6750(tips);
    host.widget_set_resolved_position_00aa8240(widget_at(screen, 0x5C),
        {position[0] - offset, static_cast<float>(static_cast<double>(position[1]) - kPauseArrowDrop),
            static_cast<float>(static_cast<double>(position[2]) - 0.0)});
    position = host.widget_resolved_position_00aa6750(tips);
    host.widget_set_resolved_position_00aa8240(widget_at(screen, 0x60),
        {position[0] + offset, static_cast<float>(static_cast<double>(position[1]) - kPauseArrowDrop),
            static_cast<float>(static_cast<double>(position[2]) + 0.0)});

    write_field<std::uint8_t>(host.command_bar_00e1930c(), 0x4, 1);
    host.list_box_clear_items_00a9bf90(widget_at(screen, 0x30));
    host.list_box_clear_items_00a9bf90(widget_at(screen, 0x34));
    void* set = read_field<void*>(game, 0x21A4 + 4 * static_cast<std::size_t>(read_field<std::int32_t>(game, 0x18EC)));
    void* sentinel = read_field<void*>(at(set, 0x24), 0x4);
    for (void* node = read_field<void*>(sentinel, 0x0); node != sentinel; node = read_field<void*>(node, 0x0)) {
        void* objective = read_field<void*>(node, 0x8);
        if (!cc13_band0085::objective_is_active_008da130(read_field<std::int32_t>(objective, 0x1C))) continue;
        const std::int32_t kind = read_field<std::int32_t>(objective, 0x18);
        std::uint32_t target = 0;
        if (kind == 0 && read_field<std::int32_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(
                             widget_at(screen, 0x30))), 0x104) < 3)
            target = widget_at(screen, 0x30);
        else if (kind == 1 && read_field<std::int32_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(
                                  widget_at(screen, 0x34))), 0x104) < 3)
            target = widget_at(screen, 0x34);
        if (target == 0) continue;
        const std::uint32_t row = host.clone_subtree_00aab4c0(widget_at(screen, 0x50), 0);
        write_field(screen, 0x160, row);
        host.text_set_localised_source_00abaed0(row, at(objective, 0xC), 1);
        host.list_box_insert_row_00a9d750(target, row, 0, 0);
    }
    host.set_shown(widget_at(screen, 0x1C), true);
}

void hud_pause_screen_update_00603ca0(void* screen, std::int32_t& page_00e19660, PauseScreenHost& host) {
    const bool hint_page = read_field<std::uint8_t>(screen, 0x10) != 0;
    if (hint_page) {
        host.scroller_update_00683820(read_field<void*>(screen, 0x9C), 1);
        host.update_hints_006037d0(screen);
    } else if (read_field<std::int32_t>(host.award_tracker_004e1ca0(), 0x84) == 0) {
        host.set_shown(widget_at(screen, 0x5C), false);
        host.set_shown(widget_at(screen, 0x60), false);
    } else {
        // The award tracker's std::map at +7Ch (head +80h): any entry whose string
        // list value (node+14h, count at +8h of the list) is non-empty.
        void* head = read_field<void*>(host.award_tracker_004e1ca0(), 0x80);
        bool any = false;
        for (void* node = read_field<void*>(head, 0x0);
             node != read_field<void*>(host.award_tracker_004e1ca0(), 0x80); node = host.tree_next_004b7de0(node)) {
            if (read_field<std::int32_t>(node, 0x14 + 0x8) != 0) {
                any = true;
                break;
            }
        }
        host.set_shown(widget_at(screen, 0x5C), any);
        host.set_shown(widget_at(screen, 0x60), any);
    }
    void* game = host.game_00e188a8();
    void* mode = read_field<void*>(game, 0x38);
    const bool tips_visible = (mode == nullptr || read_field<std::int32_t>(mode, 0x60) != 1) &&
                              read_field<std::int32_t>(host.award_tracker_004e1ca0(), 0x9C) != 0;
    host.list_control_set_item_visible_00a9c380(widget_at(screen, 0x13C), 1, tips_visible);

    if (host.input_was_pressed_004c43c0(0x4B)) {
        if (read_field<std::uint8_t>(screen, 0x10) != 0) {
            host.show_page_005fd890(screen, false, false);
            return;
        }
        host.resume_005fd440(screen);
        page_00e19660 = 0;
        return;
    }
    if (host.input_was_pressed_004c43c0(1)) {
        if (read_field<std::uint8_t>(screen, 0x10) == 0) {
            host.resume_005fd440(screen);
            page_00e19660 = 0;
        }
        return;
    }
    if (read_field<std::uint8_t>(screen, 0x10) != 0) {
        host.scroller_update_00683820(read_field<void*>(screen, 0x9C), 1);
        host.rebuild_command_bar_0054b530({{{0x00, "", 1}, {0x00, "", 1}, {0xB3, "globals.navigate", 1},
            {0xA3, "globals.back", 2}, {0x00, "", 1}}});
    } else {
        host.rebuild_command_bar_0054b530({{{0xA2, "globals.select", 0}, {0x00, "", 1},
            {0xB3, "globals.navigate", 1}, {0xA3, "globals.back", 2}, {0x00, "", 1}}});
        host.command_bar_set_help_line_0054a0c0("");
    }
    host.profile_hints_00427190(0x0F);
}

void hud_pause_on_listbox_activate_00604190(void* listener, std::uint32_t row, std::int32_t& page_00e19660,
    PauseScreenHost& host) {
    std::byte* screen = static_cast<std::byte*>(listener) - 0x8;
    if (row == widget_at(screen, 0x140)) {
        page_00e19660 = 0;
        host.resume_005fd440(screen);
    } else if (row == widget_at(screen, 0x14C)) {
        page_00e19660 = 2;
        write_field<std::uint8_t>(screen, 0x4, 0);
        void* points = read_field<void*>(host.hud_interface_00e198c4(), 0xE0);
        write_field<std::uint8_t>(points, 0x4, 1);
        write_field<std::uint8_t>(points, 0x5, 1);
        host.commit_visibility_004f83b0(points);
        host.screen_virtual_18(points);
        host.input_manager_update_00a92c40(0.0f);
    } else if (row == widget_at(screen, 0x148)) {
        page_00e19660 = 3;
        write_field<std::uint8_t>(screen, 0x4, 0);
        if (host.options_menu_00e198b8() == nullptr) host.options_menu_create_00689800();
        host.set_options_page_00e19650(0);
        host.options_menu_virtual_08();
    } else if (row == widget_at(screen, 0x150)) {
        host.raise_prompt_00531b00(4, kPauseRestartPrompt, 1, 0x005FD5B0u);
        host.focus_last_prompt_navigation_00530c20();
    } else if (row == widget_at(screen, 0x15C)) {
        page_00e19660 = 1;
        write_field(screen, 0x100, 0.0f);
        host.build_hint_list_00601200(screen);
        write_field<std::uint8_t>(screen, 0x10, 1);
        host.set_shown(widget_at(screen, 0x14), false);
        host.set_shown(widget_at(screen, 0x18), true);
        host.frame_select_default_004c9680();
        host.set_enabled(widget_at(screen, 0x2C), true);
        host.list_box_select_index_00a9c7c0(widget_at(screen, 0x2C), 0);
        host.list_box_set_listener_00a9ac40(widget_at(screen, 0x2C), listener);
        host.list_box_virtual_80(widget_at(screen, 0x2C), true);
        host.set_shown(widget_at(screen, 0x5C), false);
        host.set_shown(widget_at(screen, 0x60), false);
    } else if (row == widget_at(screen, 0x154)) {
        host.raise_prompt_00531b00(4, kPauseQuitPrompt, 1, 0x005FD610u);
        host.focus_last_prompt_navigation_00530c20();
    } else if (row == widget_at(screen, 0x158)) {
        host.raise_prompt_00531b00(6, kPauseExitPrompt, 1, 0x005FC640u);
        host.focus_last_prompt_navigation_00530c20();
    }

    if (read_field<std::uint8_t>(screen, 0x10) == 0) return;
    const std::uint32_t list_box = widget_at(screen, 0x2C);
    write_field(screen, 0x134,
        read_field<std::int32_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(list_box)), 0x164));
    write_field(screen, 0x138, host.list_box_selected_index_00a9c920(list_box));
    const std::int32_t hint_row = read_field<std::int32_t>(screen, 0x134) + read_field<std::int32_t>(screen, 0x138);
    const NativeString* mapped = host.hint_name_at_row_005fd220(screen, hint_row);
    if (mapped == nullptr) return;
    // The image compares against a 00426060 copy of the mapped name (released at
    // exit); the copy has no other effect, so the mapped value is compared here.
    const NativeString& name = *mapped;
    bool expanded = false;
    void* sentinel = read_field<void*>(screen, 0x12C);
    for (void* node = read_field<void*>(sentinel, 0x0); node != sentinel; node = read_field<void*>(node, 0x0)) {
        if (host.equals_insensitive_00435c40(*reinterpret_cast<const NativeString*>(at(node, 0x8)), name)) {
            expanded = true;
            break;
        }
    }
    if (expanded)
        host.expanded_hints_remove_005fe8b0(screen, *host.hint_name_at_row_005fd220(screen, hint_row));
    else
        host.expanded_hints_push_back(screen, *host.hint_name_at_row_005fd220(screen, hint_row));
    host.build_hint_list_00601200(screen);
    write_field(reinterpret_cast<void*>(static_cast<std::uintptr_t>(widget_at(screen, 0x2C))), 0x164,
        read_field<std::int32_t>(screen, 0x134));
    host.list_box_rebuild_page_00a9d870(widget_at(screen, 0x2C));
    host.list_box_select_index_00a9c7c0(widget_at(screen, 0x2C), read_field<std::int32_t>(screen, 0x138));
    host.list_box_virtual_80(widget_at(screen, 0x2C), false);
}

void hud_pause_update_hints_006037d0(void* screen, PauseHintsHost& host) {
    const std::uint32_t list_box = widget_at(screen, 0x2C);
    void* list = reinterpret_cast<void*>(static_cast<std::uintptr_t>(list_box));
    if (read_field<std::int32_t>(list, 0x154) == 0) return;
    if (!host.widget_virtual_38(
            host.find_child_by_name_00aa7e00(host.list_box_current_row_00425e50(list_box), kHintExclamationText)))
        return;
    write_field(screen, 0x134, read_field<std::int32_t>(list, 0x164));
    const std::int32_t selected = host.list_box_selected_index_00a9c920(list_box);
    const std::int32_t row = read_field<std::int32_t>(screen, 0x134) + selected;
    write_field(screen, 0x138, selected);
    if (host.hint_name_at_row_005fd220(screen, row) != nullptr) return;

    // Nearest preceding title in the map +11Ch (head +120h; key +0Ch, name +10h).
    std::int32_t title = 0;
    const NativeString* name = nullptr;
    void* head = read_field<void*>(screen, 0x120);
    for (void* node = read_field<void*>(head, 0x0); node != read_field<void*>(screen, 0x120);
         node = host.tree_next_005fce00(node)) {
        const std::int32_t key = read_field<std::int32_t>(node, 0xC);
        if (key < row && row - key <= row - title) {
            title = key;
            name = reinterpret_cast<const NativeString*>(at(node, 0x10));
        }
    }
    // The image keeps the title name in a local NativeString initialised to ""
    // (name == nullptr here); the lines come from map[title] through 005FF5D0.
    const NativeString* line =
        host.tracker_hint_line(host.hint_map_subscript_005ff5d0(screen, title), row - title);
    host.tracker_mark_hint_read_00690fb0(name, line);
    host.set_shown(host.find_child_by_name_00aa7e00(
                       host.list_box_row_00a9be00(list_box, read_field<std::int32_t>(screen, 0x138)), kHintExclamationText),
        false);
    if (read_field<std::int32_t>(screen, 0x134) <= title && host.tracker_unread_count_7c(name) == 0) {
        host.set_shown(host.find_child_by_name_00aa7e00(
                           host.list_box_row_00a9be00(list_box, title - read_field<std::int32_t>(screen, 0x134)),
                           kHintExclamationText),
            false);
    }
}

void hud_pause_register_00602280(void* screen, PauseRegisterHost& host) {
    const auto bind = [&](std::size_t parent, std::size_t store, std::string_view name) {
        write_field(screen, store, host.find_child_by_name_00aa7e00(widget_at(screen, parent), name));
    };
    host.frontend_register_004f71d0(screen);
    write_field(screen, 0x14, host.load_page_00aa5840(kPauseSinglePage));
    host.award_tracker_clear_new_hint_history_00690ca0();
    host.set_pause_page_00e19660(0);
    write_field(screen, 0x18, host.load_page_00aa5840(kPauseTipPage));
    bind(0x18, 0x20, "title_Group");
    bind(0x20, 0x44, "hint_title_Text");
    bind(0x20, 0x38, "parent_Group");
    bind(0x20, 0x3C, "child_Group");
    host.set_shown(widget_at(screen, 0x38), false);
    host.set_shown(widget_at(screen, 0x3C), false);
    host.set_shown(widget_at(screen, 0x44), false);
    bind(0x20, 0x2C, "HintTitle_Listbox");
    host.list_box_set_layout_flags_00a9ac90(widget_at(screen, 0x2C), 0, 0, 0);
    write_field<std::uint8_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(widget_at(screen, 0x2C))), 0x78, 0);
    host.widget_set_listener_fields_00aa6bc0(widget_at(screen, 0x2C), nullptr, 0);

    const std::uint32_t title_slider = host.find_child_by_name_00aa7e00(widget_at(screen, 0x20), "slider_Group");
    write_field(screen, 0x80, host.find_child_by_name_00aa7e00(title_slider, "slider_FrameBox"));
    write_field(screen, 0x84,
        host.widget_size_00aa6740(host.find_child_by_name_00aa7e00(title_slider, "back_Icon"))[1]);
    const std::array<float, 3> frame = host.widget_resolved_position_00aa6750(widget_at(screen, 0x80));
    write_field(screen, 0x88, frame[0]);
    write_field(screen, 0x8C, frame[1]);
    write_field(screen, 0x90, frame[2]);

    bind(0x18, 0x24, "info_Group");
    host.widget_set_listener_fields_00aa6bc0(widget_at(screen, 0x24), at(screen, 0xC), 0);
    bind(0x24, 0x70, "info_Clipbox");
    bind(0x70, 0x28, "info_text_group");
    bind(0x28, 0x40, "info_Text");
    bind(0x28, 0xF8, "silverline_FrameBox");
    const std::array<float, 3> text = host.widget_resolved_position_00aa6750(widget_at(screen, 0x40));
    write_field(screen, 0x104, text[0]);
    write_field(screen, 0x108, text[1]);
    write_field(screen, 0x10C, text[2]);
    const std::array<float, 2> text_size = host.widget_size_00aa6740(widget_at(screen, 0x40));
    write_field(screen, 0x110, text_size[0]);
    write_field(screen, 0x114, text_size[1]);
    bind(0x28, 0x48, "title_Text");
    const std::uint32_t info_slider = host.find_child_by_name_00aa7e00(widget_at(screen, 0x24), "slider_Group");
    write_field(screen, 0x7C, host.find_child_by_name_00aa7e00(info_slider, "slider_FrameBox"));
    host.widget_set_listener_fields_00aa6bc0(widget_at(screen, 0x7C), at(screen, 0xC), 0);
    bind(0x24, 0x74, "ScrollUp_Icon");
    host.widget_set_listener_fields_00aa6bc0(widget_at(screen, 0x74), at(screen, 0xC), 0);
    bind(0x24, 0x78, "ScrollDown_Icon");
    host.widget_set_listener_fields_00aa6bc0(widget_at(screen, 0x78), at(screen, 0xC), 0);
    host.scroller_attach_content_00683790(at(screen, 0x9C), widget_at(screen, 0x28));
    write_field(screen, 0x118,
        host.widget_size_00aa6740(host.find_child_by_name_00aa7e00(info_slider, "back_Icon"))[1]);
    const float back = host.widget_size_00aa6740(host.find_child_by_name_00aa7e00(info_slider, "back_Icon"))[1];
    const float thumb = host.widget_size_00aa6740(widget_at(screen, 0x7C))[1];
    host.scroller_attach_scroll_bar_00683380(at(screen, 0x9C), widget_at(screen, 0x7C), back - thumb,
        widget_at(screen, 0x74), widget_at(screen, 0x78));
    write_field<std::uint8_t>(screen, 0x10, 0);

    write_field(screen, 0x1C, host.load_page_00aa5840(kPauseObjectivesPage));
    bind(0x1C, 0xFC, "GUI_pause_objectives_FrameBox");
    bind(0xFC, 0x50, "objectives_listbox_Text");
    bind(0xFC, 0x30, "Primary_Listbox");
    host.list_box_set_layout_flags_00a9ac90(widget_at(screen, 0x30), 0, 0, 0);
    bind(0xFC, 0x34, "Secondary_Listbox");
    host.list_box_set_layout_flags_00a9ac90(widget_at(screen, 0x34), 0, 0, 0);
    bind(0xFC, 0x54, "primary_objectives_Text");
    bind(0xFC, 0x58, "secondary_objectives_Text");
    host.set_shown(widget_at(screen, 0x1C), false);
}

// ---------------------------------------------------------------------------
// HudObjectivesScreen bodies (packet cc13-fe6)
// ---------------------------------------------------------------------------

namespace {
void* native_widget(std::uint32_t widget) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(widget));
}
}  // namespace

void hud_objectives_screen_enter_005eea00(void* screen, HudObjectivesHost& host) {
    struct Bind {
        std::uint16_t parent;
        std::uint16_t store;
        std::string_view name;
    };
    static constexpr std::array<Bind, 11> kBinds{{
        {0x10, 0x14, "Open_Group"},                     // 00CF2E40
        {0x14, 0x18, "obj_main_Icon"},                  // 00CF2E30
        {0x14, 0x1C, "obj_text_top_Icon"},              // 00CF2E1C
        {0x14, 0x20, "obj_scale_Icon"},                 // 00CF2E0C
        {0x14, 0x24, "obj_text_bottom_Icon"},           // 00CF2DF4
        {0x14, 0x2C, "PrimaryNumbers_Text"},            // 00CF2DE0
        {0x14, 0x30, "SecondaryNumbers_Text"},          // 00CF2DC8
        {0x14, 0x34, "ObjectiveNumbers_Listbox"},       // 00CF2DAC
        {0x34, 0x38, "ObjectiveNumbersTemplate_Text"},  // 00CF2D8C
        {0x34, 0x3C, "KisPocset_Icon"},                 // 00CF2D7C
        {0x14, 0x28, "SelectedObjective_Text"},         // 00CF2D64
    }};
    for (const Bind& bind : kBinds) {
        write_field(screen, bind.store, host.find_child_by_name_00aa7e00(widget_at(screen, bind.parent), bind.name));
        if (bind.store == 0x34) {
            void* list_box = native_widget(widget_at(screen, 0x34));
            write_field(list_box, 0x12C, 0.0f);
            write_field(list_box, 0x130, 1000.0f);  // 00CE3804
            write_field(list_box, 0x134, 0.0f);
            host.list_box_set_input_filter_00a9ac70(widget_at(screen, 0x34), 0x00696540u);
            host.list_box_set_listener_00a9ac40(widget_at(screen, 0x34), at(screen, 0x8));
        }
    }
    write_field<std::uint32_t>(screen, 0x6C, 0);
    write_field<std::uint32_t>(screen, 0x68, 0);
    write_field<std::uint8_t>(screen, 0x70, 0);
    host.set_expanded_005ed5d0(screen, false);
    host.refresh_rows_005ee520(screen);
}

void hud_objectives_screen_update_005eef60(void* screen, HudObjectivesHost& host) {
    const std::uint32_t list_box = widget_at(screen, 0x34);
    const auto objectives_count = [&] {
        void* begin = read_field<void*>(screen, 0x54);
        if (begin == nullptr) return std::uint32_t{0};
        return static_cast<std::uint32_t>(
            (read_field<std::uintptr_t>(screen, 0x58) - reinterpret_cast<std::uintptr_t>(begin)) / 4);
    };
    const auto objective = [&](std::uint32_t index) {
        return read_field<void*>(read_field<void*>(screen, 0x54), 4 * static_cast<std::size_t>(index));
    };
    const auto row_widget = [&](std::uint32_t index) {
        return read_field<std::uint32_t>(read_field<void*>(screen, 0x44), 4 * static_cast<std::size_t>(index));
    };

    void* requested = read_field<void*>(screen, 0x6C);
    if (requested != nullptr) {
        if (read_field<void*>(screen, 0x68) != requested) {
            write_field(screen, 0x68, requested);
            for (std::uint32_t i = 0; i < objectives_count(); ++i)
                if (objective(i) == read_field<void*>(screen, 0x68)) write_field(screen, 0x60, i);
            host.list_box_select_index_00a9c7c0(list_box, read_field<std::uint32_t>(screen, 0x60));
        }
        write_field<std::uint32_t>(screen, 0x6C, 0);
    } else if (host.input_was_pressed_004c43c0(0x4B) || host.vector_00e18d1c_count() == 0) {
        host.set_expanded_005ed5d0(screen, false);
    }
    if (host.input_was_pressed_004c43c0(0xC8))
        host.set_expanded_005ed5d0(screen, read_field<std::uint8_t>(native_widget(widget_at(screen, 0x34)), 0x85) == 0);
    if (read_field<std::uint8_t>(screen, 0x65) == 0) host.refresh_rows_005ee520(screen);

    if (read_field<std::uint8_t>(screen, 0x70) != 0) {
        bool marker_shown = false;
        if (read_field<std::uint8_t>(screen, 0x64) == 0) {
            host.text_set_localised_cstring_00abbe50(widget_at(screen, 0x28), kNoActiveObjective, 1);
        } else {
            if (host.input_was_pressed_004c43c0(0xC9)) {
                const std::uint32_t rows = host.list_box_row_count_00a9ac50(widget_at(screen, 0x34));
                host.list_box_select_index_00a9c7c0(widget_at(screen, 0x34),
                    (host.list_box_selected_index_00a9c920(widget_at(screen, 0x34)) + 1u) % rows);
            }
            write_field(screen, 0x60, host.list_box_selected_index_00a9c920(widget_at(screen, 0x34)));
            const std::uint32_t selected = read_field<std::uint32_t>(screen, 0x60);
            host.text_set_localised_source_00abaed0(widget_at(screen, 0x28), at(objective(selected), 0xC), 1);
            host.text_set_localised_source_00abaed0(widget_at(screen, 0x28),
                at(objective(read_field<std::uint32_t>(screen, 0x60)), 0xC), 1);
            const std::array<float, 3> row =
                host.widget_resolved_position_00aa6750(row_widget(read_field<std::uint32_t>(screen, 0x60)));
            host.widget_set_resolved_position_00aa8240(widget_at(screen, 0x3C),
                {static_cast<float>(0.0 + row[0]), static_cast<float>(row[1] + 0.0),
                    static_cast<float>(row[2] - 1.0)});
            marker_shown = true;
        }
        host.set_shown(widget_at(screen, 0x3C), marker_shown);

        const std::array<float, 3> top = host.widget_resolved_position_00aa6750(widget_at(screen, 0x1C));
        const std::array<float, 2> top_size = host.widget_size_00aa6740(widget_at(screen, 0x1C));
        const float w = top_size[0];
        const float h = top_size[1];
        host.widget_set_resolved_position_00aa8240(widget_at(screen, 0x20),
            {static_cast<float>(top[0] + static_cast<double>(kPauseArrowDrop)), top[1] + h,
                static_cast<float>(top[2] + 0.0)});
        void* text = native_widget(widget_at(screen, 0x28));
        const float line_scale = static_cast<float>(
            static_cast<double>(read_field<std::int16_t>(read_field<void*>(text, 0x108), 0x14)) / kObjectivesLineScale);
        // FILD of the dword plus 2^32 when negative: the line count is unsigned.
        const float scale_height = static_cast<float>(
            static_cast<double>(line_scale) * static_cast<double>(read_field<std::uint32_t>(text, 0x110)));
        host.widget_set_size_58(widget_at(screen, 0x20),
            {host.widget_size_00aa6740(widget_at(screen, 0x20))[0], scale_height});
        const float span = static_cast<float>(static_cast<double>(h) + static_cast<double>(h) +
                                              static_cast<double>(scale_height));
        host.widget_set_resolved_position_00aa8240(widget_at(screen, 0x24),
            {top[0] + w, span + top[1], static_cast<float>(top[2] + 0.0)});
        host.widget_set_resolved_position_00aa8240(widget_at(screen, 0x28),
            {static_cast<float>(top[0] + static_cast<double>(kObjectivesTextIndent)), top[1] + h,
                static_cast<float>(top[2] - static_cast<double>(kObjectivesTextDrop))});
    }

    // Function-local static colour 00E1961C..00E19628, guard 00E1962C bit 0.
    static const std::array<float, 4> kWhite{1.0f, 1.0f, 1.0f, 1.0f};
    for (std::uint32_t i = 0; i < objectives_count(); ++i) {
        const std::int32_t state = read_field<std::int32_t>(objective(i), 0x1C);
        std::array<float, 4> colour = kWhite;
        if (state == 1)
            colour = {0.0f, kObjectiveDoneGreen, 0.0f, 1.0f};
        else if (state == 2)
            colour = {kObjectiveFailedRed, 0.0f, 0.0f, 1.0f};
        host.widget_set_colour_50(row_widget(i), colour);
    }
    host.set_shown(widget_at(screen, 0x14), read_field<std::uint8_t>(screen, 0x70) != 0);
    host.apply_highlight_pulse_006dbac0(widget_at(screen, 0x3C), read_field<std::uint8_t>(screen, 0x70) != 0, 0, 1,
        1.0f);
}

void hud_order_screen_update_005f9dd0(void* screen, float dt, HudOrderUpdateHost& host) {
    if (read_field<std::uint8_t>(screen, 0x82) == 0) return;
    if (read_field<std::uint8_t>(read_field<void*>(host.hud_interface_00e198c4(), 0x5C), 0x4) != 0) return;
    std::array<float, 4> colour = host.widget_colour_54(widget_at(screen, 0x8));
    const bool no_order = read_field<std::int32_t>(screen, 0x88) == 0;
    float step = dt;
    if (no_order) {
        if (1.0f > read_field<float>(screen, 0x70)) write_field(screen, 0x70, 1.0f);
        step = static_cast<float>(static_cast<double>(dt) * kOrderFadeRate);
    }
    const float fade = static_cast<float>(read_field<float>(screen, 0x70) + static_cast<double>(step));
    write_field(screen, 0x70, fade);
    if (fade > 0.5f)
        colour[3] = static_cast<float>(static_cast<double>(colour[3]) - kOrderFadeRate * static_cast<double>(step));
    for (int i = 0; i < 8; ++i) host.input_manager_singleton_004bec00();
    if (colour[3] <= 0.0f) {
        host.hud_order_close_005f9520(screen);
        colour[3] = 1.0f;
    }
    host.widget_set_colour_00aa85b0(widget_at(screen, 0x8), colour);

    const double delta = static_cast<double>(step) * kOrderPulseRate;
    const float pulse = read_field<std::uint8_t>(screen, 0x80) != 0
                            ? static_cast<float>(delta + read_field<float>(screen, 0x7C))
                            : static_cast<float>(read_field<float>(screen, 0x7C) - delta);
    write_field(screen, 0x7C, pulse);
    if (static_cast<double>(pulse) > static_cast<double>(kOrderPulseHigh)) write_field<std::uint8_t>(screen, 0x80, 0);
    if (kOrderPulseLow > pulse) write_field<std::uint8_t>(screen, 0x80, 1);
    if (read_field<std::int32_t>(screen, 0x88) == 0) {  // re-read by the image
        write_field(screen, 0x7C, 0.5f);
        write_field<std::uint8_t>(screen, 0x80, 1);
    }
    std::array<float, 4> pulse_colour = host.widget_colour_54(widget_at(screen, 0x44));
    pulse_colour[3] = read_field<float>(screen, 0x7C);
    for (std::size_t offset = 0x44; offset <= 0x54; offset += 4) host.widget_set_colour_50(widget_at(screen, offset), pulse_colour);
}

void multi_main_screen_update_005ea2d0(void* screen, float dt, MultiMainUpdateHost& host) {
    void* list20 = native_widget(widget_at(screen, 0x20));
    void* list24 = native_widget(widget_at(screen, 0x24));
    bool stepped = false;
    if (host.action_edge_or_repeat_004d92b0(0x4B)) {
        host.refresh_online_state_005e7900(screen);
    } else if (read_field<std::uint8_t>(list20, 0x77) == 0) {
        if (host.action_edge_or_repeat_004d92b0(0x4D)) {
            write_field<std::uint8_t>(list20, 0x77, 1);
            stepped = true;
        }
    } else if (read_field<std::uint8_t>(list24, 0x77) == 0) {
        if (host.action_edge_or_repeat_004d92b0(0x4D)) {
            write_field<std::uint8_t>(list24, 0x77, 1);
            stepped = true;
        }
    } else if (host.action_edge_or_repeat_004d92b0(0x4C)) {
        const std::int32_t column = read_field<std::int32_t>(screen, 0xE0);
        if (column != 0) {
            write_field(screen, 0xE0, column - 1);
            stepped = true;
        } else {
            void* list = host.widget_virtual_38(widget_at(screen, 0x24)) ? list24 : list20;
            write_field<std::uint8_t>(list, 0x77, 0);
            host.widget_set_uniform_scale_004fb9e0(widget_at(screen, 0xE8), 1.0f);
            host.set_shown(widget_at(screen, 0xD4), false);
        }
    } else if (host.action_edge_or_repeat_004d92b0(0x4D)) {
        const std::int32_t column = read_field<std::int32_t>(screen, 0xE0);
        if (column != 2) {
            write_field(screen, 0xE0, column + 1);
            stepped = true;
        }
    } else if (host.action_edge_or_repeat_004d92b0(0x46)) {
        const std::int32_t row = read_field<std::int32_t>(screen, 0xE4);
        if (row != 0) {
            write_field(screen, 0xE4, row - 1);
            stepped = true;
        }
    } else if (host.action_edge_or_repeat_004d92b0(0x47)) {
        const std::int32_t row = read_field<std::int32_t>(screen, 0xE4);
        if (row != read_field<std::int32_t>(screen, 0x10) - 1) {
            write_field(screen, 0xE4, row + 1);
            stepped = true;
        }
    }
    if (stepped) {
        host.update_session_list_005e9be0(screen);
        host.sound_queue_set_flag_19();
    }

    // x87: the products are exact in extended precision; one rounding per FSTP.
    const float velocity = read_field<float>(screen, 0xF0);
    if (velocity > kMultiScrollDeadZone || -kMultiScrollDeadZone > velocity) {
        if (velocity > kMultiScrollDeadZone)
            host.scroller_toward_end_00683700(at(screen, 0x6C),
                static_cast<float>(2.0 * (static_cast<double>(velocity) * dt)));
        else
            host.scroller_toward_start_00683680(at(screen, 0x6C),
                static_cast<float>(2.0 * (-static_cast<double>(velocity) * dt)));
        const double v = read_field<float>(screen, 0xF0);
        write_field(screen, 0xF0, static_cast<float>(v - 2.0 * (static_cast<double>(dt) * v)));
    }
    host.scroller_update_00683820(at(screen, 0x6C), 1);
    const bool selectable = read_field<std::uint8_t>(list24, 0x77) == 0 || read_field<std::uint8_t>(list20, 0x77) == 0;
    host.rebuild_command_bar_0054b530({{selectable ? MainMenuCommandArgument{0xA2, "globals.select", 0}
                                                   : MainMenuCommandArgument{0x00, "", 1},
        {0xB3, "globals.navigate", 1}, {0x00, "", 1}, {0xA3, "globals.back", 2}, {0x00, "", 1}}});
}

std::string lobby_settings_option_label_005e2320(std::int32_t slot, LobbyLabelHost& host) {
    void* game = host.game_00e188a8();
    const auto record = [&]() -> const void* {
        const std::int32_t index = host.mission_index_00e19594();
        const void* list = at(game, 0x5F0);
        if (read_field<std::int32_t>(list, 0x0) <= index) return nullptr;
        if (host.list_nth_node_004be940(list, index) == nullptr) return nullptr;
        if (read_field<std::uint8_t>(host.list_nth_node_004be940(list, index), 0x914) == 0) return nullptr;
        return host.list_nth_node_004be940(list, index);
    };
    if (slot == 1) {
        const void* node = record();
        return node != nullptr ? host.native_string(at(node, 0x920)) : std::string("FE.unknown_mapname");
    }
    if (slot == 0) {
        if (read_field<std::uint8_t>(game, 0x218C) == 0) {
            return std::to_string(static_cast<int>(read_field<std::int8_t>(game, 0x2018))) + "/" +
                   std::to_string(static_cast<int>(read_field<std::int8_t>(game, 0x2017)));
        }
        if (host.effective_game_mode_004bca50() <= 3) return std::to_string(2);
        int active = 0;
        for (std::size_t i = 0; i < 8; ++i) {
            const void* player = read_field<void*>(game, 0x18CC + 4 * i);
            if ((read_field<std::uint8_t>(player, 0x8) != 0 && read_field<std::uint8_t>(player, 0x9) == 0) ||
                read_field<std::uint8_t>(player, 0xA) != 0)
                ++active;
        }
        return std::to_string(active);
    }
    if (slot == 0x0D) return host.string_00e195bc();
    if (record() == nullptr) return "-";
    if (!host.option_enabled(slot, host.stored_option_00e1955c(slot))) host.step_option_005e21b0(slot);
    if (host.option_label_00575e80(slot, 0) != nullptr)
        return host.option_label_00575e80(slot, host.stored_option_00e1955c(slot));
    return std::to_string(host.option_value_00575fd0(slot, host.stored_option_00e1955c(slot)));
}

void options_build_main_list_005f54f0(void* screen, OptionsMainListHost& host) {
    write_field<std::uint8_t>(screen, 0x252, 0);
    void* game = host.game_00e188a8();
    host.command_bar_set_help_variant_00549620(read_field<std::uint8_t>(game, 0x61F));
    if (read_field<std::uint8_t>(screen, 0x78) != 0) {
        write_field<std::uint8_t>(screen, 0x78, 0);
        host.push_interface_request_004cc460(0x0E, 0);
        return;
    }
    host.copy_settings_005f52c0(at(screen, 0xD8));
    host.set_options_page_00e19650(0);
    host.control_layout_button_flags_00527cb0(reinterpret_cast<std::uint8_t*>(at(screen, 0x119)),
        reinterpret_cast<std::uint8_t*>(at(screen, 0x11A)), reinterpret_cast<std::uint8_t*>(at(screen, 0x11B)),
        reinterpret_cast<std::uint8_t*>(at(screen, 0x118)));
    const std::uint32_t list_box = widget_at(screen, 0x84);
    host.list_box_set_listener_00a9ac40(list_box, nullptr);
    host.list_box_clear_rows_00a9bec0(list_box);
    if (read_field<std::uint8_t>(host.game_00e188a8(), 0x61F) != 0)
        host.frame_select_00518d60(kOptionsMainTitle);
    else
        host.frame_set_title_005189b0(kOptionsMainTitle);
    for (std::uint32_t row = 0; row < kOptionsMainListRowCount; ++row) {
        if (row == 6 && read_field<std::uint8_t>(host.game_00e188a8(), 0x61F) != 0) continue;
        const std::uint32_t widget = host.clone_subtree_00aab4c0(widget_at(screen, 0x8C), 0);
        host.widget_set_name_2c(widget, "Elem_" + std::to_string(row & 0xFFFFu) + "_Text");
        write_field(native_widget(widget), 0xD8, row);
        host.list_box_insert_row_00a9d750(widget_at(screen, 0x84), widget, 0, 0);
        host.set_shown(widget, true);
        host.text_set_localised_cstring_00abbe50(widget, kOptionsMainList[row].label.data(), 1);
        if (row == 5) {
            void* briefing = host.briefing_screen_00e198ac();
            if (briefing != nullptr && read_field<std::uint8_t>(briefing, 0x74) != 0) host.widget_virtual_7c(widget, true);
        } else if (row == 6) {
            void* downloads = host.downloads_00f8a304();
            host.widget_virtual_7c(widget, downloads == nullptr || read_field<std::int32_t>(downloads, 0x8) == 0);
        } else if (row == 4) {
            host.widget_virtual_7c(widget, read_field<std::uint8_t>(screen, 0x188) == 0);
        }
    }
    host.list_box_reset_highlight_00a9cf30(widget_at(screen, 0x84), 1, 1);
    host.refresh_help_line_005f0cd0(screen);
    host.list_box_set_listener_00a9ac40(widget_at(screen, 0x84), at(screen, 0x8));
    host.list_box_select_index_00a9c7c0(widget_at(screen, 0x84), read_field<std::int32_t>(screen, 0xA8));
    host.list_box_virtual_80(widget_at(screen, 0x84), true);
    for (std::size_t offset : {0xA4u, 0xA0u, 0x88u, 0x254u, 0x258u}) host.set_shown(widget_at(screen, offset), false);
    host.list_box_clear_other_two_00a9cdf0(widget_at(screen, 0x84), widget_at(screen, 0x84), widget_at(screen, 0x88));
}

void options_activate_list_row_005f7c40(void* listener, std::uint32_t row, OptionsActivateHost& host) {
    std::byte* screen = static_cast<std::byte*>(listener) - 0x8;
    if (read_field<float>(screen, 0x260) != 0.0f) return;
    const std::int32_t page = host.options_page_00e19650();
    const auto finish = [&] { write_field(screen, 0x260, kOptionsTransitionTime); };
    const auto apply = [&] {
        if (host.settings_dirty_00f88a30() || read_field<std::uint8_t>(screen, 0x252) != 0) {
            if (read_field<std::uint8_t>(screen, 0x252) != 0) {
                host.raise_prompt_00531b00(0, kOptionsDlcChangedPrompt, 1, 0x005F6FB0u);
                finish();
                return;
            }
            if (host.xenon_has_selected_user_00a3e510()) {
                host.raise_prompt_00531b00(0, kOptionsSaveConfirmPrompt, 1, 0x005F6FB0u);
                finish();
                return;
            }
        }
        host.apply_and_rebuild_005f65c0(screen);
        finish();
    };
    const auto video_changed = [&] {
        return host.live_video_00f889f8() != read_field<std::int32_t>(screen, 0x150) ||
               host.live_video_00f8899e() != read_field<std::uint8_t>(screen, 0xF6);
    };
    const auto defaults = [&](std::uint32_t routine) {
        write_field<std::uint8_t>(screen, 0x250, 1);
        host.settings_reset_defaults(routine, at(screen, 0xD8));
        host.settings_apply_all_008d5b50(at(screen, 0xD8));
    };

    if (page == 0) {
        const std::int32_t data = host.list_box_selected_data_00a9c990(widget_at(screen, 0x84));
        write_field(screen, 0xA8, data);
        write_field(screen, 0x25C, read_field<std::uint8_t>(host.game_00e188a8(), 0x61F));
        switch (data) {
        case 0: host.build_page(0x005F58F0u, screen); break;
        case 1: host.build_page(0x005F59F0u, screen); break;
        case 2: host.build_page(0x005F41E0u, screen); break;
        case 3: host.push_interface_request_004cc460(0x0E, 0); break;
        case 4:
            host.control_layout_set_payload_00527c80(read_field<std::uint8_t>(screen, 0x119),
                read_field<std::uint8_t>(screen, 0x11A), read_field<std::uint8_t>(screen, 0x11B),
                read_field<std::uint8_t>(screen, 0x118));
            host.push_interface_request_004cc460(0x0D, 0);
            break;
        case 5:
            if (!host.main_menu_connection_gate_00585b40()) return;
            if (!host.main_menu_account_gate_00585810(0)) return;
            write_field<std::uint8_t>(screen, 0x25C, 0);
            host.options_reset_005efab0(screen);
            break;
        case 6: host.build_page(0x005F3B50u, screen); break;
        default: break;
        }
        if (read_field<std::uint8_t>(screen, 0x25C) != 0) host.frame_select_00518d60(-1, -1, 0, "");
        return;
    }

    if (row == widget_at(screen, 0x84)) {
        const std::int32_t index =
            host.list_box_selected_index_00a9c920(widget_at(screen, 0x84)) + read_field<std::int32_t>(screen, 0xAC);
        if (page == 1) {
            if (index == 9) apply();
            else if (index == 10) defaults(0x008D41C0u);
            else if (index == 11) host.return_to_list_005f5e60(screen);
        } else if (page == 4) {
            if (index == 6) apply();
            else if (index == 7) defaults(0x008D4820u);
        } else if (page == 2) {
            if (index == 4) apply();
            else if (index == 5) defaults(0x008D41F0u);
            else if (index == 6) host.return_to_list_005f5e60(screen);
        } else if (page == 3) {
            if (index == 0x0D) {
                const bool changed = video_changed();
                host.settings_apply_all_008d5b50(at(screen, 0xD8));
                host.confirm_changes_005f7050(screen, changed);
            } else if (index == 0x0E) {
                write_field<std::uint8_t>(screen, 0x250, 1);
                host.settings_reset_video_defaults_008d4520(at(screen, 0xD8), 0, 0);
                host.settings_apply_all_008d5b50(at(screen, 0xD8));
            } else if (index == 0x0F) {
                host.return_to_list_005f5e60(screen);
            }
        }
        return;
    }

    if (page == 1 || page == 2 || page == 4) {
        apply();
    } else if (page == 3) {
        const bool changed = video_changed();
        host.settings_apply_all_008d5b50(at(screen, 0xD8));
        if (host.hud_interface_00e198c4() == nullptr) host.xlive_on_reset_device_00a4d46a();
        host.confirm_changes_005f7050(screen, changed);
    } else if (page == 7) {
        write_field<std::uint8_t>(screen, 0x252, 1);
        apply();
    }
}

void lobby_handle_gui_event_005e4490(void* self, std::uint32_t event, LobbyEventHost& host) {
    std::byte* lobby = static_cast<std::byte*>(self) - 0xC;
    void* game = host.game_00e188a8();
    const auto focused = [&] { return widget_at(self, 0x10); };
    const auto selected_record = [&]() -> const void* {
        if (host.list_box_selected_index_00a9c920(widget_at(self, 0x9C)) <= -1) return nullptr;
        const std::int32_t sel = host.list_box_selected_index_00a9c920(widget_at(self, 0x9C));
        return read_field<void*>(game, 0x18CC + 4 * static_cast<std::size_t>(sel));
    };
    const auto local_slot = [&] {
        return read_field<void*>(game, 0x18CC + 4 * static_cast<std::size_t>(read_field<std::int32_t>(game, 0x18EC)));
    };
    const auto focus_panel = [&](std::size_t offset) {
        if (host.lobby_settings_open_005d5490(lobby)) return;
        if (focused() == widget_at(self, 0x144)) host.chat_notify_focus();
        host.lobby_set_focus_005d67e0(lobby, widget_at(self, offset));
    };

    const std::int32_t kind = host.widget_kind_5c(event);
    if (kind == 2) {
        if (host.widget_virtual_38(widget_at(self, 0xC)) &&
            read_field<std::uint8_t>(native_widget(widget_at(self, 0x21C)), 0x85) != 0)
            return;
        host.list_box_select_index_00a9c7c0(widget_at(self, 0x9C),
            read_field<std::int32_t>(native_widget(event), 0xD8));
        if (focused() == widget_at(self, 0x144)) host.chat_notify_focus();
        host.lobby_set_focus_005d67e0(lobby, widget_at(self, 0x98));
        return;
    }
    if (host.widget_kind_5c(event) == 3) {
        host.widget_virtual_4c(event, 1.0f);
        const auto* code_ptr = read_field<const std::uint8_t*>(native_widget(event), 0xF0);
        const std::uint8_t code = code_ptr != nullptr ? *code_ptr : host.default_command_00e19608();
        switch (code) {
        case 0xA2:
            if (host.lobby_settings_open_005d5490(lobby)) {
                host.lobby_apply_selected_map_005da530(lobby);
            } else if (focused() == widget_at(self, 0x14)) {
                host.lobby_apply_local_map_005d9f90(lobby);
            } else if (focused() == widget_at(self, 0x80)) {
                host.input_manager_update_00a92c40(0.0f);
                host.lobby_set_editable_005d5870(lobby, true);
            } else if (focused() == widget_at(self, 0x1C)) {
                void* brief = read_field<void*>(host.briefing_screen_00e198ac(), 0x70);
                write_field(brief, 0x98, read_field<std::int32_t>(local_slot(), 0x28));
                write_field<std::int32_t>(read_field<void*>(host.briefing_screen_00e198ac(), 0x70), 0x94, 8);
                host.push_request_interface_00688ad0(0x0B);
            } else if (focused() == widget_at(self, 0x98)) {
                const std::int32_t sel = host.list_box_selected_index_00a9c920(widget_at(self, 0x9C));
                // The listener sub-object at lobby+8 (this-4), its vtable slot +04h.
                host.lobby_listener_activate_04(static_cast<std::byte*>(self) - 0x4,
                    host.list_box_row_00a9be00(widget_at(self, 0x9C), sel));
            }
            return;
        case 0xB7: host.lobby_refresh_players_005d9660(lobby, 0, 1); return;
        case 0xB8: {
            if (host.online_virtual_198_00f8a2fc() && host.online_byte_4d_00f8a2fc() != 0) return;
            write_field(self, 0x9C, host.find_child_by_name_00aa7e00(widget_at(self, 0x8), "Players_Listbox"));
            const void* record = selected_record();
            std::int32_t level = 0;
            if ((record == nullptr || read_field<std::uint8_t>(record, 0xA) == 0) &&
                !host.game_is_standard_mode_005d4a10(lobby)) {
                host.session_request_slot_ai_enable_00771e80(
                    host.list_box_selected_index_00a9c920(widget_at(self, 0x9C)), true);
                level = 3;
            } else {
                const std::int32_t sel = host.list_box_selected_index_00a9c920(widget_at(self, 0x9C));
                const std::int32_t ai = read_field<std::int32_t>(
                    read_field<void*>(game, 0x18CC + 4 * static_cast<std::size_t>(sel)), 0x2C);
                if (ai == 3 || ai == 1) {
                    level = 4;
                } else if (ai == 4) {
                    level = 5;
                } else {
                    if (host.game_is_standard_mode_005d4a10(lobby)) return;
                    host.session_request_slot_ai_enable_00771e80(
                        host.list_box_selected_index_00a9c920(widget_at(self, 0x9C)), false);
                    return;
                }
            }
            host.session_set_slot_ai_skill_0076cc40(host.list_box_selected_index_00a9c920(widget_at(self, 0x9C)), level);
            return;
        }
        case 0xA3: host.lobby_confirm_leave_005da210(lobby); return;
        case 0xA7:
            if (focused() == widget_at(self, 0x98)) {
                const void* record = selected_record();
                const bool ai_or_empty = record == nullptr || read_field<std::uint8_t>(record, 0x9) != 0;
                const bool ready = record != nullptr && read_field<std::uint8_t>(record, 0xA) != 0;
                if (read_field<std::int32_t>(game, 0x1FE4) != 1 || record == nullptr) return;
                if (ai_or_empty) {
                    host.session_request_slot_ai_enable_00771e80(
                        host.list_box_selected_index_00a9c920(widget_at(self, 0x9C)), !ready);
                } else if (record != host.game_local_slot_record_005d5130()) {
                    host.lobby_confirm_kick_005d8530();
                }
            } else if (focused() == widget_at(self, 0x80)) {
                const void* mission = host.mission_record_005d8650(read_field<std::int32_t>(local_slot(), 0x28));
                host.briefing_set_mission_data_0051dce0(read_field<void*>(host.briefing_screen_00e198ac(), 0x64), mission);
                host.push_request_interface_00688ad0(3);
            }
            return;
        case 0xA5: {
            const void* record = selected_record();
            const bool ai_or_empty = record == nullptr || read_field<std::uint8_t>(record, 0x9) != 0;
            if (read_field<std::uint8_t>(native_widget(widget_at(self, 0x9C)), 0x85) == 0) return;
            if (record == nullptr || ai_or_empty) return;
            host.show_gamer_card(read_field<std::uint32_t>(record, 0x80), read_field<std::uint32_t>(record, 0x84));
            return;
        }
        case 0xB1: host.lobby_check_client_compatibility_005d71b0(lobby); return;
        default: return;
        }
    }

    const auto auto_scroll = [&](std::size_t scroller, std::int32_t direction) {
        host.scroller_set_auto_scroll_00683300(at(self, scroller), direction, kLobbyAutoScrollSpeed, 1);
    };
    const auto focus_list = [&] {
        if (focused() != widget_at(self, 0x80)) host.lobby_set_focus_005d67e0(lobby, widget_at(self, 0x80));
    };
    if (event == widget_at(self, 0x98)) { focus_panel(0x98); return; }
    if (event == widget_at(self, 0x14)) { focus_panel(0x14); return; }
    if (event == widget_at(self, 0x80)) { focus_panel(0x80); return; }
    if (event == widget_at(self, 0x1C)) { focus_panel(0x1C); return; }
    if (event == widget_at(self, 0x144)) {
        if (host.lobby_settings_open_005d5490(lobby)) return;
        void* menu = host.multi_menu_00e198b4();
        if (menu == nullptr || read_field<std::uint8_t>(read_field<void*>(menu, 0x54), 0x5) == 0) return;
        if (read_field<std::uint8_t>(game, 0x218C) != 0) return;
        host.lobby_set_focus_005d67e0(lobby, widget_at(self, 0x144));
        host.chat_set_mode_005d11f0(2);
        host.input_set_context_level_00a933f0(0x1E, 0);
        return;
    }
    if (event == widget_at(self, 0x90)) { focus_list(); auto_scroll(0x24, 2); }
    else if (event == widget_at(self, 0x8C)) { focus_list(); auto_scroll(0x24, 1); }
    else if (event == widget_at(self, 0x84)) { focus_list(); host.scroller_begin_thumb_drag_00683a40(at(self, 0x24)); }
    else if (event == widget_at(self, 0x130)) auto_scroll(0x14C, 2);
    else if (event == widget_at(self, 0x12C)) auto_scroll(0x14C, 1);
    else if (event == widget_at(self, 0x148)) host.scroller_begin_thumb_drag_00683a40(at(self, 0x14C));
    else if (event == widget_at(self, 0xB0)) auto_scroll(0xC0, 2);
    else if (event == widget_at(self, 0xAC)) auto_scroll(0xC0, 1);
    else if (event == widget_at(self, 0xBC)) host.scroller_begin_thumb_drag_00683a40(at(self, 0xC0));
    else if (event == widget_at(self, 0x214)) host.lobby_step_setting_005e1cf0(lobby, -1);
    else if (event == widget_at(self, 0x218)) host.lobby_step_setting_005e1cf0(lobby, 1);
    else if (event == widget_at(self, 0x138)) {
        host.chat_set_team_only_005d1000(false);
        host.widget_virtual_88(widget_at(self, 0x138), 2, 0, 1.0f);
        host.widget_virtual_88(widget_at(self, 0x13C), 0, 0, 1.0f);
        host.widget_virtual_88(widget_at(self, 0x140), 0, 0, 1.0f);
        return;
    } else if (event == widget_at(self, 0x13C) || event == widget_at(self, 0x140)) {
        host.chat_set_team_only_005d1000(true);
        host.widget_virtual_88(widget_at(self, 0x138), 0, 0, 1.0f);
        host.widget_virtual_88(widget_at(self, 0x13C), 2, 0, 1.0f);
        host.widget_virtual_88(widget_at(self, 0x140), 2, 0, 1.0f);
        return;
    } else if (host.lobby_settings_open_005d5490(lobby)) {
        return;
    }
    if (host.widget_kind_5c(event) == 6)
        host.widget_virtual_88(event, 2, 0, 1.0f);
    else if (host.widget_kind_5c(event) == 0x12)
        host.widget_virtual_84(event, 2);
}

// ---------------------------------------------------------------------------
// Packet cc13-fe9 (docs/CC13_FE_9.md)
// ---------------------------------------------------------------------------

void hud_narrative_register_005bb130(void* screen, Fe9NarrativeRegisterHost& host) {
    const auto ptr = [screen](std::size_t offset) { return read_field<void*>(screen, offset); };
    const auto find = [&host](void* parent, std::string_view name) { return host.find_child_by_name_00aa7e00(parent, name); };
    host.frontend_screen_register_004f71d0(screen);
    write_field(screen, 0x20, host.gui_load_page_00aa5840("GUI_blackout", 1, 0));
    void* const warnings = find(ptr(0x20), "Warnings_Group");
    write_field(screen, 0x2C, find(warnings, "MessageBoxTemplate_Group"));
    host.voice_slot_stop_007026f0(at(screen, 0x08));
    write_field<std::uint8_t>(screen, 0x60, 0);
    write_field(screen, 0x28, host.gui_load_page_00aa5840("GUI_subtitle", 1, 0));
    write_field(screen, 0x30, find(ptr(0x28), "textbox_main_Group"));
    write_field(screen, 0x34, find(ptr(0x30), "textbox_main_Text"));
    write_field(screen, 0x38, find(ptr(0x30), "textbox_main_Text"));
    write_field(screen, 0x3C, find(ptr(0x30), "textbox_main_FrameBox"));
    write_field(screen, 0x40, find(ptr(0x30), "textbox_main_Icon"));
    const std::array<float, 4> colour = *host.text_colour_54(ptr(0x34));
    write_field(screen, 0x44, colour);

    void* const box = host.message_boxes_reset_to_one(at(screen, 0x90));
    write_field(box, 0x00, ptr(0x30));
    write_field(box, 0x04, find(ptr(0x30), "textbox_main_FrameBox"));
    write_field(box, 0x08, find(ptr(0x30), "textbox_main_Text"));
    write_field(box, 0x10, read_field<std::array<float, 4>>(screen, 0x44));
    write_field(box, 0x0C, 0.0f);
    write_field<std::uint8_t>(box, 0x20, 1);
    write_field<std::int32_t>(box, 0x24, 0);
    write_field<std::int32_t>(box, 0x28, 0);
    write_field(box, 0x2C, 0.0f);
    write_field<std::int32_t>(box, 0x30, -1);

    void* const owner = host.lua_owner_open_run(1, "Scripts/datatables/DialogGlobals.lua", 0);
    write_field(screen, 0x78, static_cast<float>(host.lua_global_number(owner, "DialogFadeTime")));
    write_field(screen, 0x88, 0.0f);
    write_field<std::int32_t>(screen, 0x84, -1);
    write_field(screen, 0x7C, static_cast<float>(host.lua_global_number(owner, "TempCharWait")));
    write_field(screen, 0x80, static_cast<float>(host.lua_global_number(owner, "TempWaitBase")));
    write_field(screen, 0x8C, 0.0f);
    write_field<std::int32_t>(screen, 0x70, -1);
    write_field<std::int32_t>(screen, 0x74, 0);
    write_field(screen, 0xBC, find(ptr(0x20), "Blackout_Icon"));
    write_field(screen, 0x24, host.gui_load_page_00aa5840("GUI_narrative", 1, 0));
    write_field(screen, 0xAC, find(ptr(0x24), "MissionInfoTitle_Group"));
    write_field(screen, 0xB0, find(ptr(0xAC), "textbox_obj_Text"));
    write_field(screen, 0xB4, find(ptr(0xAC), "textbox_obj_FrameBox"));
    write_field(screen, 0xD8, 1.0f);
    write_field(screen, 0xD4, 0.0f);
    write_field(screen, 0xDC, 8.0f);  // 00CE3918
    write_field(screen, 0xC8, 0.0f);
    write_field(screen, 0xC4, 0.0f);
    write_field(screen, 0xC0, 0.0f);
    host.widget_set_visible_34(ptr(0xBC), 0);
    write_field<std::uint8_t>(screen, 0x04, 1);
    host.lua_owner_close_00b669a0(owner);
}

namespace {

std::string_view fe9_native_view(const void* string) {
    const char* data = read_field<const char*>(string, 4);
    return data != nullptr ? std::string_view(data, read_field<std::uint32_t>(string, 0)) : std::string_view();
}

bool fe9_equal_ci(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lower = [](char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

}  // namespace

void hud_pause_build_hint_list_00601200(void* screen, Fe9HintListHost& host) {
    const auto ptr = [screen](std::size_t offset) { return read_field<void*>(screen, offset); };
    void* const list = ptr(0x2C);
    host.list_box_clear_items_00a9bf90(list);
    void* const index_map = at(screen, 0x11C);
    void* const head = read_field<void*>(index_map, 4);
    host.index_tree_erase_005fe9e0(index_map, read_field<void*>(head, 4));
    write_field(head, 4, head);
    write_field<std::uint32_t>(index_map, 8, 0);
    write_field(head, 0, head);
    write_field(head, 8, head);

    std::int32_t index = 0;
    void* const tracker = host.award_tracker_004e1ca0();
    void* const history_head = read_field<void*>(tracker, 0x98);
    for (void* node = read_field<void*>(history_head, 0); node != history_head;
         node = host.tree_iterator_next_004b7de0(node)) {
        void* const row = host.widget_clone_subtree_00aab4c0(ptr(0x38), nullptr);
        void* const hint = host.find_child_by_name_00aa7e00(row, "hint_Text");
        host.widget_set_visible_34(host.find_child_by_name_00aa7e00(row, "felkialtojel_Text"), 0);
        const std::string key(fe9_native_view(at(node, 0x0C)));
        std::string shown = key;
        if (shown.find("AAAAAA") != std::string::npos) shown = shown.substr(6, shown.size() - 6);
        const float width = static_cast<float>(host.widget_size_00aa6740(hint)[0] - 0.020833333333333332);  // 00CF0000
        host.widget_set_size_58(hint, {width, host.widget_size_00aa6740(hint)[1]});
        host.gui_text_set_ellipsized_00abb000(hint, shown, -1.0f, 1);
        write_field(hint, 0x30, -0.03020833246409893f);  // 00CF3F80
        write_field(hint, 0x34, 0.0f);
        write_field<std::uint8_t>(hint, 0x1F0, 0);
        host.widget_set_size_58(hint, {0.1979166716337204f, host.widget_size_00aa6740(hint)[1]});  // 00CF3F7C
        host.list_box_insert_row_00a9d750(list, row, 0, 0);
        host.index_map_assign_005ff5d0(index_map, index, key);
        ++index;

        std::vector<std::string> fresh;
        if (host.new_hints_contains_005fd120(key) && host.new_hints_count_006002b0(key) != 0) {
            host.widget_set_visible_34(host.find_child_by_name_00aa7e00(row, "felkialtojel_Text"), 1);
            fresh = host.new_hints_list_005fd4b0(key);
        }
        host.widget_set_state_88(host.find_child_by_name_00aa7e00(row, "button_Icon"), 0, 0, 1.0f);
        void* const expanded = at(screen, 0x128);
        void* const expanded_head = read_field<void*>(expanded, 4);
        bool open = false;
        for (void* item = read_field<void*>(expanded_head, 0); item != expanded_head; item = read_field<void*>(item, 0)) {
            if (fe9_equal_ci(fe9_native_view(at(item, 8)), key)) {
                open = true;
                break;
            }
        }
        if (!open) continue;
        host.widget_set_state_88(host.find_child_by_name_00aa7e00(row, "button_Icon"), 1, 0, 1.0f);
        for (const std::string& sub : host.history_sub_hints_004d48a0(node)) {
            void* const sub_row = host.widget_clone_subtree_00aab4c0(ptr(0x3C), nullptr);
            void* const sub_hint = host.find_child_by_name_00aa7e00(sub_row, "hint_Text");
            host.widget_set_visible_34(host.find_child_by_name_00aa7e00(sub_row, "felkialtojel_Text"), 0);
            const HintTexts texts = host.hint_texts_005ffb20(sub);
            const float sub_width = static_cast<float>(host.widget_size_00aa6740(sub_hint)[0] - 0.016666666666666666);  // 00CF3F68
            host.widget_set_size_58(sub_hint, {sub_width, host.widget_size_00aa6740(sub_hint)[1]});
            write_field<std::uint8_t>(sub_hint, 0xFC, 0);
            const std::int32_t language = host.language_index();
            std::string_view text = sub;
            if (language == 0 && !texts.language0.empty()) text = texts.language0;
            else if (language == 1 && !texts.language1.empty()) text = texts.language1;
            else if (!texts.fallback.empty()) text = texts.fallback;
            host.gui_text_set_ellipsized_00abb000(sub_hint, text, -1.0f, 1);
            write_field(sub_hint, 0x30, -0.00937500037252903f);  // 00CF3F64
            write_field(sub_hint, 0x34, 0.0f);
            write_field<std::uint8_t>(sub_hint, 0x1F0, 0);
            host.widget_set_size_58(sub_hint, {0.18541666865348816f, host.widget_size_00aa6740(sub_hint)[1]});  // 00CF3F60
            host.list_box_insert_row_00a9d750(list, sub_row, 0, 0);
            for (const std::string& f : fresh) {
                if (fe9_equal_ci(f, sub)) {
                    host.widget_set_visible_34(host.find_child_by_name_00aa7e00(sub_row, "felkialtojel_Text"), 1);
                    break;
                }
            }
            ++index;
        }
    }

    host.list_box_enable_paging_00a9e230(list, 0x10);
    const std::int32_t rows = read_field<std::int32_t>(list, 0x154);
    void* const thumb = ptr(0x80);
    const float width = host.widget_size_00aa6740(thumb)[0];
    const float height = rows > 0x10 ? static_cast<float>(16.0 / rows * read_field<float>(screen, 0x84))  // 00CED9F8
                                     : read_field<float>(screen, 0x84);
    write_field(screen, 0x98, height);
    write_field(screen, 0x94, width);
    host.widget_set_size_58(thumb, {width, height});
}
// --- packet cc13-fe11: 005AB190 BSP_HudMapScreen_Register --------------------

void hud_map_register_setup_005ab190(void* screen, Fe11HudMapRegisterHost& host) {
    host.frontend_screen_register_004f71d0(screen);
    host.simple_3d_scene_init_0093ccc0(at(screen, 0x08));
    write_field<float>(screen, 0x84, 1.0f);
    write_field<std::uint8_t>(screen, 0x88, 0);
    const float width = static_cast<float>(static_cast<double>(host.world_bound(0x7124)) - host.world_bound(0x7130));
    const float x = static_cast<float>(static_cast<double>(width) / 0.75 * 1.100000023841858);
    const float depth = static_cast<float>(static_cast<double>(host.world_bound(0x7128)) - host.world_bound(0x711C));
    const float y = static_cast<float>(static_cast<double>(depth) * 1.100000023841858);
    const float m = x > y ? x : y;
    float* const ext = host.map_zoom_extents_00e0cf40();
    std::int32_t level;
    if (8000.0 >= m) {  // FCOMIP + JB: an unordered m takes the larger levels
        level = 2;
        ext[2] = m;
    } else if (25000.0f >= m) {
        level = 1;
        ext[1] = m;
        ext[2] = static_cast<float>(static_cast<double>(m) * 0.4000000059604645);
    } else {
        level = 0;
        ext[0] = m;
        ext[1] = static_cast<float>(0.375 * m);
        ext[2] = static_cast<float>(static_cast<double>(m) * 0.16599999368190765);
    }
    write_field<std::int32_t>(screen, 0x2C, level);
    write_field<std::int32_t>(screen, 0x5FC, level);
    const float* const crossfade = host.map_zoom_crossfade_00e0cf50();
    float* const scales = host.map_zoom_scales_00f89ae4();
    const double base = ext[3];
    for (int i = 0; i < 3; ++i) scales[i] = static_cast<float>(static_cast<double>(crossfade[i]) * ext[i] / base);
    scales[3] = static_cast<float>(static_cast<double>(crossfade[3]) * base / base);
    write_field<float>(screen, 0x1C, static_cast<float>(static_cast<double>(level) + 1.0));
    const float extent = ext[level];
    write_field<float>(screen, 0x30, extent);
    write_field<float>(screen, 0x34, static_cast<float>(0.75 * extent));
    void* const light = read_field<void*>(screen, 0x18);
    void* const owner = host.directional_light_create_shadow_owner_00a8fd30(light);
    host.directional_light_set_shadow_owner_00b7bdf0(light, owner);
    host.ref_release(owner);
    write_field(screen, 0x8C, host.gui_load_page_00aa5840("GUI_map", 1, 0));
    write_field(screen, 0x90, host.gui_load_page_00aa5840("GUI_classicons", 1, 0));
    const float pan_x = static_cast<float>((static_cast<double>(host.world_bound(0x7128)) + host.world_bound(0x711C)) * 0.5);
    const float pan_y = static_cast<float>((static_cast<double>(host.world_bound(0x7130)) + host.world_bound(0x7124)) * 0.5);
    write_field<float>(screen, 0x54C, pan_x);
    write_field<float>(screen, 0x550, pan_y);
    write_field<float>(screen, 0x554, pan_x);
    write_field<float>(screen, 0x558, read_field<float>(screen, 0x550));
    write_field<float>(screen, 0x5AC, 0.0f);
    write_field<std::uint8_t>(screen, 0x40, 0);
    write_field<float>(screen, 0x5B0, read_field<float>(screen, 0x30));
    write_field<std::uint8_t>(screen, 0x41, 0);
    write_field<float>(screen, 0x5B4, read_field<float>(screen, 0x34));
    write_field<float>(screen, 0x5E8, read_field<float>(screen, 0x54C));
    write_field<float>(screen, 0x5EC, read_field<float>(screen, 0x550));
    write_field<float>(screen, 0x5D0, 0.0f);
    write_field<float>(screen, 0x5D4, 1.0f);
}

void hud_map_register_objects_005ab519(void* screen, Fe11HudMapObjectsHost& host) {
    for (void* link = host.map_object_list_head(); link != nullptr; link = read_field<void*>(link, 4)) {
        void* const object = read_field<void*>(link, 8);
        void* const resource = host.resource_load_default_factory_00b80d70(at(object, 0x424));
        void* const instance = host.resource_instantiate_08(resource, 6, 1.0f);
        void* const root = read_field<void*>(instance, 0x0C);
        host.node_propagate_root_registration_00b6d890(root, read_field<void*>(screen, 0x0C));
        void* const inner = host.node_virtual_10(root, 6, 0);
        if (read_field<std::uint8_t>(object, 0xC8) == 0) {
            void* const parent = read_field<void*>(object, 0x3C);
            if (parent != nullptr) host.entity_pose_refresh_world_00414db0(parent);
            std::array<float, 16> world{};
            if (read_field<void*>(object, 0x3C) != nullptr) {
                world = host.matrix_multiply_00413920(reinterpret_cast<const float*>(at(object, 0x74)),
                    reinterpret_cast<const float*>(at(read_field<void*>(object, 0x3C), 0xCC)));
            } else {
                std::memcpy(world.data(), at(object, 0x74), sizeof world);
            }
            std::memcpy(at(object, 0xCC), world.data(), sizeof world);
            write_field<std::uint8_t>(object, 0xC8, 1);
            write_field<std::uint8_t>(object, 0x10C, 0);
        }
        host.node_set_world_matrix_34(root, reinterpret_cast<const float*>(at(object, 0xCC)));
        host.dword_vector_push(at(screen, 0x510), root);
        host.map_scene_setup_ocean_materials_0093d2d0(at(screen, 0x08), root, 1);
        host.map_scene_setup_ocean_materials_0093d2d0(at(screen, 0x08), inner, 0);
        for (void* child = read_field<void*>(inner, 0x34); child != nullptr; child = read_field<void*>(child, 0x3C)) {
            const char* const name = host.node_name_00b6d800(child);
            host.debug_print_004254b0("Terrain child %s", name != nullptr ? name : "");  // 00CF0A68, "" 00E1952D
        }
        host.dword_vector_push(at(screen, 0x500), inner);
        host.node_set_parent_00b6e680(inner, root);
        host.ref_release(resource);
        host.list_push_back_498(at(screen, 0x498), instance);
    }
}

void hud_map_register_quads_005ab7c5(void* screen, Fe11HudMapQuadsHost& host) {
    const float dx = static_cast<float>(static_cast<double>(host.world_bound(0x7128)) - host.world_bound(0x711C));
    const float dz = static_cast<float>(static_cast<double>(host.world_bound(0x7124)) - host.world_bound(0x7130));
    write_field<float>(screen, 0x55C, dx);
    write_field<float>(screen, 0x560, dz);
    const float cx = static_cast<float>((static_cast<double>(host.world_bound(0x7128)) + host.world_bound(0x711C)) * 0.5);
    write_field<float>(screen, 0x564, -0.0f - cx);
    const float cz = static_cast<float>(0.5 * (static_cast<double>(host.world_bound(0x7130)) + host.world_bound(0x7124)));
    write_field<float>(screen, 0x568, -0.0f - cz);
    const float padded_w = static_cast<float>(static_cast<double>(dx) + 6000.0);
    const float padded_h = static_cast<float>(static_cast<double>(dz) + 6000.0);
    for (const Fe11MapQuad& quad : kHudMapQuads) {
        float w = padded_w;
        float h = padded_h;
        if (quad.size == Fe11QuadSize::Extent) {
            w = read_field<float>(screen, 0x30);
            h = read_field<float>(screen, 0x34);
        } else if (quad.size == Fe11QuadSize::World) {
            w = dx;
            h = dz;
        }
        void* const model = host.map_scene_build_quad_0093de10(at(screen, 0x08), quad, w, h);
        if (quad.mode == 0x0D || quad.mode == 0x0E) {
            host.material_register_float2_00b18b00(host.generated_model_material(model), "cLength",
                reinterpret_cast<const float*>(at(screen, 0x55C)));
            host.material_register_float2_00b18b00(host.generated_model_material(model), "cOffset",
                reinterpret_cast<const float*>(at(screen, 0x564)));
        }
    }
    for (std::size_t i = 0; i < 0x61; ++i) {
        write_field<std::int32_t>(screen, 0x130 + i * 4, 0);
        write_field<std::int32_t>(screen, 0x2B4 + i * 4, 0);
    }
}

void hud_map_register_resources_005ac390(void* screen, Fe11HudMapResourcesHost& host) {
    for (const Fe11MapResource& model : kHudMapModels) {
        if (model.offset == 0x43C)
            for (std::uint32_t off = 0x438; off <= 0x44C; off += 4) write_field<std::int32_t>(screen, off, 0);
        if (model.offset == 0x450)
            for (std::uint32_t off = 0x450; off <= 0x460; off += 4) write_field<std::int32_t>(screen, off, 0);
        write_field(screen, model.offset, host.resource_load_default_factory_00b80d70(model.path));
    }
    for (const Fe11MapResource& texture : kHudMapTextures)
        write_field(screen, texture.offset, host.texture_manager_load_64(texture.path, 0));
    write_field<std::int32_t>(screen, 0x128, 0);
    write_field<float>(screen, 0x12C, 0.0f);
    write_field(screen, 0xF8, read_field<void*>(screen, 0xF4));
    const float config = host.global_config_2bc();  // 00432650 runs before the table read
    const float scale = static_cast<float>(
        static_cast<double>(host.map_zoom_scales_00f89ae4()[read_field<std::int32_t>(screen, 0x2C)]) * config);
    write_field<float>(screen, 0x5BC, scale);
    write_field<float>(screen, 0x5DC, scale);
}

void hud_map_register_005ab190(void* screen, Fe11HudMapHost& host) {
    hud_map_register_setup_005ab190(screen, static_cast<Fe11HudMapRegisterHost&>(host));
    hud_map_register_objects_005ab519(screen, static_cast<Fe11HudMapObjectsHost&>(host));
    hud_map_register_quads_005ab7c5(screen, static_cast<Fe11HudMapQuadsHost&>(host));
    hud_map_register_resources_005ac390(screen, static_cast<Fe11HudMapResourcesHost&>(host));
}

}  // namespace bsp::cc13_fe1