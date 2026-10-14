#pragma once
// Segment 25 (0060CB60-0061D330), cc13-seg25: HUD plane-spawn, please-wait and
// scoring screen leftovers, and the powerup-manager screen's small routines.
// docs/CC13_SEG25.md has the evidence. Names are hypotheses, not recovered
// symbols. Objects are raw image storage (Raw::at at the image's offsets); every
// engine routine the image calls is a named host method invoked at the same
// point. The native __thiscall/__fastcall conventions are documented per
// routine, not reproduced. Nothing here runs in the game.
#include <cstddef>
#include <cstdint>

namespace bsp::seg25_cc13 {

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};
inline Raw raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }

// The eight-byte native string (dword length, dword buffer); see native_string.hpp.
struct NativeStr {
    std::uint32_t length;
    char* data;
};

struct Seg25Host {
    virtual ~Seg25Host() = default;
    // Address of an image global's storage (00E188A8, 00E198C4, ...).
    virtual void* global(std::uint32_t address) = 0;

    // --- native strings, the sized pool, CRT ---
    virtual void native_string_resize_0041dd40(NativeStr* s, std::uint32_t length, std::int32_t flag) = 0;
    // 00419CC0 (pool singleton), then 00BD1510 __thiscall(pool, block, size, 1) RET 0Ch.
    virtual void* pool_singleton_00419cc0() = 0;
    virtual void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size, std::int32_t flag) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // _invalid_parameter_noinfo (the checked-index trap).
    virtual void invalid_parameter_00bf6713() = 0;

    // --- screens and the GUI manager ---
    virtual void frontend_construct_004f7180(void* screen) = 0;
    virtual void frontend_destruct_004f71a0(void* screen) = 0;
    virtual void frontend_register_004f71d0(void* screen) = 0;
    // 0060C650 __thiscall(screen): the HUD screen base destructor (slot 40h family).
    virtual void hud_screen_base_destruct_0060c650(void* screen) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    // 00AA5840 __thiscall(manager, const NativeStr* name, 1, 0) RET 0Ch -> page.
    virtual std::uint32_t gui_load_page_00aa5840(void* manager, const NativeStr* name, std::int32_t a, std::int32_t b) = 0;
    // 00AA31F0 __thiscall(manager, page) RET 4.
    virtual void gui_remove_and_destroy_page_00aa31f0(void* manager, std::uint32_t page) = 0;
    // 00AA7E00 __thiscall(parent, const NativeStr* name, 1) RET 8 -> widget.
    virtual void* widget_find_child_00aa7e00(void* parent, const NativeStr* name, std::int32_t flag) = 0;
    // 004D6790 __thiscall(vector, const value*) RET 4: push_back.
    virtual void vector_push_back_004d6790(void* vector, const void* value) = 0;
    // 0060DD10 __thiscall(screen): HudScoringScreen Close.
    virtual void scoring_screen_close_0060dd10(void* screen) = 0;
    // 0060F9E0 __thiscall(vector): HudScoreRow vector _Tidy.
    virtual void score_row_vector_tidy_0060f9e0(void* vector) = 0;

    // --- widgets ---
    // Widget virtuals: +04h(1) deleting destructor, +34h(int) show, +4Ch(float) alpha,
    // +58h(const float*) set size, +5Ch() kind, +84h(bool), +88h(state, 0, float).
    virtual void widget_vslot04(void* widget, std::int32_t flags) = 0;
    virtual void widget_vslot34(void* widget, std::int32_t value) = 0;
    virtual void widget_vslot4c(void* widget, float value) = 0;
    virtual void widget_vslot58(void* widget, const float* values) = 0;
    virtual std::int32_t widget_vslot5c(void* widget) = 0;
    virtual void widget_vslot84(void* widget, std::int32_t value) = 0;
    virtual void widget_vslot88(void* widget, std::uint32_t state, std::int32_t zero, float alpha) = 0;
    // 00AA6740 __thiscall(widget) -> const float* (x, y) size.
    virtual const float* widget_size_00aa6740(void* widget) = 0;
    // 00AA6750 __thiscall(widget, float out[3]) RET 4 -> out.
    virtual const float* widget_resolved_position_00aa6750(void* widget, float (&out)[3]) = 0;
    // 00AA8240 __thiscall(widget, const float xyz[3]) RET 4.
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float (&xyz)[3]) = 0;
    // 00AA8610 __thiscall(widget, float) RET 4: alpha on the widget and its children.
    virtual void widget_set_alpha_recursive_00aa8610(void* widget, float alpha) = 0;

    // --- powerup manager ---
    // 00616C70 / 0061B0F0 __thiscall(screen).
    virtual void powerup_fill_second_rows_00616c70(void* screen) = 0;
    virtual void powerup_rebuild_lists_0061b0f0(void* screen) = 0;
    // 00617870 __thiscall(map, key) RET 4 -> value reference (a word read by the caller).
    virtual void* string_map_ci_subscript_00617870(void* map, std::uint32_t key) = 0;
    // Head-node buyers __thiscall(container) -> node (00613AE0, 00613B30, 00613860, 004870F0).
    virtual void* container_buy_head_node(std::uint32_t function, void* container) = 0;
    // 00487110 / 00613DF0 __thiscall(list): destroy storage / _Tidy.
    virtual void award_index_list_destroy_00487110(void* list) = 0;
    virtual void list_tidy_00613df0(void* list) = 0;
    // 00616060 / 00616920 __thiscall(tree, out, first, last) RET 0Ch: erase a range.
    virtual void tree_erase_range(std::uint32_t function, void* tree, void* out_iterator, void* first, void* last) = 0;
};

// ---- plane-spawn and please-wait screens ----
void hud_plane_spawn_screen_destroy_0060d130(void* screen, Seg25Host& host);
void hud_plane_spawn_screen_register_0060d1c0(void* screen, Seg25Host& host);
void hud_plane_spawn_screen_collect_children_0060d280(void* screen, void* children, Seg25Host& host);
void* please_wait_screen_construct_0060d290(void* screen, Seg25Host& host);
std::int32_t please_wait_screen_get_screen_id_0060d2b0() noexcept;
bool please_wait_screen_is_self_managed_0060d2c0() noexcept;
void please_wait_screen_destroy_0060d330(void* screen, Seg25Host& host);
void please_wait_screen_register_0060d430(void* screen, Seg25Host& host);
void please_wait_screen_collect_children_0060d560(void* screen, void* children, Seg25Host& host);

// ---- scoring screen and its rows ----
void hud_scoring_screen_on_listbox_activate_0060de00(void* listener, Seg25Host& host);
void hud_score_row_destroy_0060de10(void* row, Seg25Host& host);
void* hud_score_row_assign_0060dfb0(void* self, const void* source, Seg25Host& host);
void* hud_score_row_copy_construct_0060f020(void* self, const void* source, Seg25Host& host);
void* hud_scoring_screen_construct_0060fd90(void* screen, Seg25Host& host);
std::int32_t hud_scoring_screen_get_screen_id_0060fe00() noexcept;
bool hud_scoring_screen_is_self_managed_0060fe20(Seg25Host& host);
void hud_scoring_screen_collect_children_0060fe40(void* screen, void* children, Seg25Host& host);
void hud_scoring_screen_destroy_0060fe50(void* screen, Seg25Host& host);

// ---- powerup-manager screen ----
bool powerup_manager_is_scrolling_00611750(void* screen) noexcept;
void powerup_manager_tick_reset_fade_00611ae0(void* screen, float dt, Seg25Host& host);
void powerup_manager_hide_info_page_00611bf0(void* screen, Seg25Host& host);
void powerup_manager_tick_label_fade_00612b70(void* screen, float dt, Seg25Host& host);
void native_string_destroy_storage_006140d0(NativeStr& s, Seg25Host& host);
void* native_string_int_construct_00614ee0(void* self, const NativeStr* text, const std::uint32_t* value, Seg25Host& host);
void* native_string_int_copy_construct_00614f30(void* self, const void* source, Seg25Host& host);
void powerup_carousel_init_006161f0(void* carousel) noexcept;
void powerup_manager_destroy_00618ea0(void* screen, Seg25Host& host);
std::int32_t powerup_manager_get_screen_id_00619160() noexcept;
void powerup_manager_collect_children_00619170(void* screen, void* children, Seg25Host& host);
void powerup_manager_refresh_second_rows_00619ae0(void* screen, Seg25Host& host);
void* powerup_manager_construct_0061a480(void* screen, Seg25Host& host);
void powerup_manager_tick_second_fade_0061aff0(void* screen, float dt, Seg25Host& host);
void powerup_manager_on_listener_refresh_0061b840(void* listener, Seg25Host& host);
void rewards_screen_on_widget_hover_0061c800(void* screen, void* widget, bool hovered, Seg25Host& host);

} // namespace bsp::seg25_cc13
