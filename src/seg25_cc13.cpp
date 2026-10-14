// Segment 25, cc13-seg25. Evidence and per-function notes: docs/CC13_SEG25.md.
// Status: reconstructed and Win32 build-tested only.
#include "bsp/seg25_cc13.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp::seg25_cc13 {
namespace {

constexpr std::uint32_t kGame = 0x00E188A8u;
constexpr std::uint32_t kHud = 0x00E198C4u;

void* global_ptr(Seg25Host& host, std::uint32_t address) {
    return *static_cast<void**>(host.global(address));
}

// The inlined pool release of a native-string buffer: 00419CC0 then
// 00BD1510(buffer, length + 1, 1) when the buffer is non-null.
void release_pool_block(Seg25Host& host, void* block, std::uint32_t length) {
    if (block != nullptr) {
        void* pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, block, length + 1, 1);
    }
}

// The inlined temporary: {0, null}, 0041DD40(length, 1), memcpy(length + 1).
NativeStr build(Seg25Host& host, const char* text, std::uint32_t length) {
    NativeStr s{0, nullptr};
    host.native_string_resize_0041dd40(&s, length, 1);
    if (s.data != nullptr) std::memcpy(s.data, text, s.length + 1);
    return s;
}

// The inlined native-string copy: resize to the source length, memcpy that many bytes.
void assign(Seg25Host& host, NativeStr* dst, const NativeStr* src) {
    host.native_string_resize_0041dd40(dst, src->length, 1);
    if (src->length != 0) std::memcpy(dst->data, src->data, dst->length);
}

}  // namespace

// ===========================================================================
// Plane-spawn and please-wait screens
// ===========================================================================

// 0060D130 __thiscall(screen), RET: vtables CF4A7C / CF4A64 (+8h), page +28h
// removed, then the HUD screen base destructor 0060C650.
void hud_plane_spawn_screen_destroy_0060d130(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CF4A7Cu;
    s.at<std::uint32_t>(0x8) = 0x00CF4A64u;
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0x28));
    host.hud_screen_base_destruct_0060c650(screen);
}

// 0060D1C0 __thiscall(screen), RET: register, load GUI_plane_spawn into +28h.
void hud_plane_spawn_screen_register_0060d1c0(void* screen, Seg25Host& host) {
    host.frontend_register_004f71d0(screen);
    NativeStr page = build(host, "GUI_plane_spawn", 0xF);
    raw(screen).at<std::uint32_t>(0x28) = host.gui_load_page_00aa5840(host.gui_manager_004c12b0(), &page, 1, 0);
    release_pool_block(host, page.data, page.length);
}

// 0060D280 __thiscall(screen, children), RET 4: push &+28h.
void hud_plane_spawn_screen_collect_children_0060d280(void* screen, void* children, Seg25Host& host) {
    host.vector_push_back_004d6790(children, static_cast<std::uint8_t*>(screen) + 0x28);
}

// 0060D290 __thiscall(screen), RET: 004F7180, vtable CF4AB8.
void* please_wait_screen_construct_0060d290(void* screen, Seg25Host& host) {
    host.frontend_construct_004f7180(screen);
    raw(screen).at<std::uint32_t>(0x0) = 0x00CF4AB8u;
    return screen;
}

// 0060D2B0: MOV EAX, 53h; RET.  0060D2C0: MOV AL, 1; RET.
std::int32_t please_wait_screen_get_screen_id_0060d2b0() noexcept { return 0x53; }
bool please_wait_screen_is_self_managed_0060d2c0() noexcept { return true; }

// 0060D330 __thiscall(screen), RET: vtable CF4AB8, page +8h removed, 004F71A0.
void please_wait_screen_destroy_0060d330(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CF4AB8u;
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0x8));
    host.frontend_destruct_004f71a0(screen);
}

// 0060D430 __thiscall(screen), RET: register, _PleaseWait into +8h, its
// Background_Icon into +0Ch, +10h = -1.0f (00D7A260).
void please_wait_screen_register_0060d430(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    host.frontend_register_004f71d0(screen);
    NativeStr page = build(host, "_PleaseWait", 0xB);
    s.at<std::uint32_t>(0x8) = host.gui_load_page_00aa5840(host.gui_manager_004c12b0(), &page, 1, 0);
    release_pool_block(host, page.data, page.length);
    NativeStr icon = build(host, "Background_Icon", 0xF);
    s.ptr(0xC) = host.widget_find_child_00aa7e00(s.ptr(0x8), &icon, 1);
    release_pool_block(host, icon.data, icon.length);
    s.at<float>(0x10) = -1.0f;
}

// 0060D560 __thiscall(screen, children), RET 4: push &+8h.
void please_wait_screen_collect_children_0060d560(void* screen, void* children, Seg25Host& host) {
    host.vector_push_back_004d6790(children, static_cast<std::uint8_t*>(screen) + 0x8);
}

// ===========================================================================
// Scoring screen (5Eh) and its 24h-byte rows: native strings +0h/+8h, dword
// +10h, byte +14h, dwords +18h..+20h.
// ===========================================================================

// 0060DE00 __thiscall(ECX = listener = screen+8h, two words), RET 8: Close(screen).
void hud_scoring_screen_on_listbox_activate_0060de00(void* listener, Seg25Host& host) {
    host.scoring_screen_close_0060dd10(static_cast<std::uint8_t*>(listener) - 8);
}

// 0060DE10 __fastcall(row), RET: release the strings +8h then +0h.
void hud_score_row_destroy_0060de10(void* row, Seg25Host& host) {
    Raw r = raw(row);
    release_pool_block(host, r.ptr(0xC), r.at<std::uint32_t>(0x8));
    release_pool_block(host, r.ptr(0x4), r.at<std::uint32_t>(0x0));
}

namespace {
void copy_score_row_tail(Raw d, Raw s) {
    d.at<std::uint32_t>(0x10) = s.at<std::uint32_t>(0x10);
    d.at<std::uint8_t>(0x14) = s.at<std::uint8_t>(0x14);
    d.at<std::uint32_t>(0x18) = s.at<std::uint32_t>(0x18);
    d.at<std::uint32_t>(0x1C) = s.at<std::uint32_t>(0x1C);
    d.at<std::uint32_t>(0x20) = s.at<std::uint32_t>(0x20);
}
}  // namespace

// 0060DFB0 __thiscall(this, const row*), RET 4: assignment (strings skipped when
// the same object).
void* hud_score_row_assign_0060dfb0(void* self, const void* source, Seg25Host& host) {
    Raw d = raw(self);
    Raw s = raw(const_cast<void*>(source));
    for (std::size_t off : {0x0u, 0x8u}) {
        auto* dst = reinterpret_cast<NativeStr*>(d.p + off);
        const auto* src = reinterpret_cast<const NativeStr*>(s.p + off);
        if (dst != src) assign(host, dst, src);
    }
    copy_score_row_tail(d, s);
    return self;
}

// 0060F020 __thiscall(this, const row*), RET 4: copy constructor.
void* hud_score_row_copy_construct_0060f020(void* self, const void* source, Seg25Host& host) {
    Raw d = raw(self);
    Raw s = raw(const_cast<void*>(source));
    for (std::size_t off : {0x0u, 0x8u}) {
        auto* dst = reinterpret_cast<NativeStr*>(d.p + off);
        const auto* src = reinterpret_cast<const NativeStr*>(s.p + off);
        dst->length = 0;
        dst->data = nullptr;
        if (dst != src) assign(host, dst, src);
    }
    copy_score_row_tail(d, s);
    return self;
}

// 0060FD90 __thiscall(screen), RET: HudScoringScreen constructor.
void* hud_scoring_screen_construct_0060fd90(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    host.frontend_construct_004f7180(screen);
    s.at<std::uint32_t>(0x8) = 0x00CEB0FCu;
    s.at<std::uint32_t>(0xC) = 0x00CEB110u;
    s.at<std::uint32_t>(0x0) = 0x00CF4B98u;
    s.at<std::uint32_t>(0x8) = 0x00CF4B84u;
    s.at<std::uint32_t>(0xC) = 0x00CF4B64u;
    for (std::size_t off : {0x20u, 0x24u, 0x28u, 0x30u, 0x34u, 0x38u}) s.at<std::uint32_t>(off) = 0;
    return screen;
}

// 0060FE00: MOV EAX, 5Eh; RET.
std::int32_t hud_scoring_screen_get_screen_id_0060fe00() noexcept { return 0x5E; }

// 0060FE20 __cdecl(), RET: self-managed while [game+1FE4h] == 0 (no network session).
bool hud_scoring_screen_is_self_managed_0060fe20(Seg25Host& host) {
    return raw(global_ptr(host, kGame)).at<std::int32_t>(0x1FE4) == 0;
}

// 0060FE40 __thiscall(screen, children), RET 4: push &+44h.
void hud_scoring_screen_collect_children_0060fe40(void* screen, void* children, Seg25Host& host) {
    host.vector_push_back_004d6790(children, static_cast<std::uint8_t*>(screen) + 0x44);
}

// 0060FE50 __thiscall(screen), RET: vtables; page +44h removed and zeroed; the
// row vectors +2Ch then +1Ch (0060F9E0); +8h = CEB0FC; 004F71A0.
void hud_scoring_screen_destroy_0060fe50(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CF4B98u;
    s.at<std::uint32_t>(0x8) = 0x00CF4B84u;
    s.at<std::uint32_t>(0xC) = 0x00CF4B64u;
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0x44));
    s.at<std::uint32_t>(0x44) = 0;
    host.score_row_vector_tidy_0060f9e0(s.p + 0x2C);
    host.score_row_vector_tidy_0060f9e0(s.p + 0x1C);
    s.at<std::uint32_t>(0x8) = 0x00CEB0FCu;
    host.frontend_destruct_004f71a0(screen);
}

// ===========================================================================
// Powerup-manager screen (4Fh, vtable 00CF4DEC)
// ===========================================================================

// 00611750 __fastcall(screen), RET: either slide flag +1F9h or +13Fh set.
bool powerup_manager_is_scrolling_00611750(void* screen) noexcept {
    Raw s = raw(screen);
    return s.at<std::uint8_t>(0x1F9) != 0 || s.at<std::uint8_t>(0x13F) != 0;
}

// 00611AE0 __thiscall(screen, float dt), RET 4: +208h -= 2*dt (DE E9 at 00611AF1,
// FSUBP ST1), below zero -> 0 and +20Ch cleared. Then for the four widget pairs
// +28h..+34h (alpha, 00AA8610) and +38h..+44h (vslot4C): above 0.5f the value is
// 2v - 1.0, else 1.0 - 2v (DE E1, FSUBRP ST1); vslot4C gets half of it (double
// 0.5 at 00D7A280). Each value is rounded to float once.
void powerup_manager_tick_reset_fade_00611ae0(void* screen, float dt, Seg25Host& host) {
    Raw s = raw(screen);
    const float v = static_cast<float>(static_cast<double>(s.at<float>(0x208)) - (static_cast<double>(dt) + dt));
    s.at<float>(0x208) = v;
    if (0.0f > v) {
        s.at<float>(0x208) = 0.0f;
        s.at<std::uint8_t>(0x20C) = 0;
    }
    const bool fading_out = s.at<float>(0x208) > 0.5f;   // 00CE3800
    for (int i = 0; i < 4; ++i) {
        const double twice = static_cast<double>(s.at<float>(0x208)) * 2.0;
        const double a = fading_out ? twice - 1.0 : 1.0 - twice;
        host.widget_set_alpha_recursive_00aa8610(s.ptr(0x28 + 4 * i), static_cast<float>(a));
        const double twice2 = static_cast<double>(s.at<float>(0x208)) * 2.0;
        const double b = (fading_out ? twice2 - 1.0 : 1.0 - twice2) * 0.5;
        host.widget_vslot4c(s.ptr(0x38 + 4 * i), static_cast<float>(b));
    }
}

// 00611BF0 __fastcall(screen), RET: hide widget +B0h.
void powerup_manager_hide_info_page_00611bf0(void* screen, Seg25Host& host) {
    host.widget_vslot34(raw(screen).ptr(0xB0), 0);
}

// 00612B70 __thiscall(screen, float dt), RET 4: the label slide-in while +22Ch is
// set. +230h = (float)(dt * 8.0 + +230h) (double 00CE3DB0); at >= 1 the flag
// clears, +230h = 1.0f, label +D4h is placed at (pos(+E4h).x - +254h*0.5,
// pos(+D4h).y, pos(+D4h).z) (DE E9 at 00612C36, FSUBP ST1) and shown. Then the
// label width ([+D4h]+114h / 960.0, 00CEC380) widens +254h when not smaller,
// hiding [[00E198C4]+C0h]+D4h and setting +234h = width + 0.013 (00CED090),
// +238h = size(+ECh).y. Finally +ECh gets vslot58({+234h * +230h, +238h}),
// +E4h is placed at (size(+ECh).x + +248h, +24Ch, +250h), and +24h's alpha is +230h.
void powerup_manager_tick_label_fade_00612b70(void* screen, float dt, Seg25Host& host) {
    Raw s = raw(screen);
    if (s.at<std::uint8_t>(0x22C) == 0) return;
    const float t = static_cast<float>(static_cast<double>(dt) * 8.0 + s.at<float>(0x230));
    s.at<float>(0x230) = t;
    if (t >= 1.0f) {   // JC skips when below or unordered
        s.at<std::uint8_t>(0x22C) = 0;
        s.at<float>(0x230) = 1.0f;
        float a[3], b[3], c[3];
        const float z = host.widget_resolved_position_00aa6750(s.ptr(0xD4), a)[2];
        const float y = host.widget_resolved_position_00aa6750(s.ptr(0xD4), b)[1];
        const float x0 = host.widget_resolved_position_00aa6750(s.ptr(0xE4), c)[0];
        const float pos[3] = {static_cast<float>(static_cast<double>(x0) - static_cast<double>(s.at<float>(0x254)) * 0.5), y, z};
        host.widget_set_resolved_position_00aa8240(s.ptr(0xD4), pos);
        host.widget_vslot34(s.ptr(0xD4), 1);
    }
    const float width = static_cast<float>(static_cast<double>(raw(s.ptr(0xD4)).at<float>(0x114)) / 960.0);
    if (width >= s.at<float>(0x254)) {   // JC skips when below or unordered
        s.at<float>(0x254) = width;
        host.widget_vslot34(raw(raw(global_ptr(host, kHud)).ptr(0xC0)).ptr(0xD4), 0);
        const float* size = host.widget_size_00aa6740(s.ptr(0xEC));
        s.at<float>(0x238) = size[1];
        s.at<float>(0x234) = static_cast<float>(static_cast<double>(s.at<float>(0x254)) + 0.013000000268220901);
    }
    float unused[3];
    host.widget_resolved_position_00aa6750(s.ptr(0xEC), unused);   // result not read
    const float extent[2] = {s.at<float>(0x234) * s.at<float>(0x230), s.at<float>(0x238)};
    host.widget_vslot58(s.ptr(0xEC), extent);
    const float offset = s.at<float>(0x248);
    const float py = s.at<float>(0x24C);
    const float pz = s.at<float>(0x250);
    const float* size = host.widget_size_00aa6740(s.ptr(0xEC));
    const float place[3] = {size[0] + offset, py, pz};
    host.widget_set_resolved_position_00aa8240(s.ptr(0xE4), place);
    host.widget_set_alpha_recursive_00aa8610(s.ptr(0x24), s.at<float>(0x230));
}

// 006140D0 __fastcall(string), RET: pool release of the buffer.
void native_string_destroy_storage_006140d0(NativeStr& s, Seg25Host& host) {
    release_pool_block(host, s.data, s.length);
}

// 00614EE0 __thiscall(this, const NativeStr*, const dword*), RET 8: {string, value}.
void* native_string_int_construct_00614ee0(void* self, const NativeStr* text, const std::uint32_t* value, Seg25Host& host) {
    auto* dst = static_cast<NativeStr*>(self);
    dst->length = 0;
    dst->data = nullptr;
    if (dst != text) assign(host, dst, text);
    raw(self).at<std::uint32_t>(0x8) = *value;
    return self;
}

// 00614F30 __thiscall(this, const pair*), RET 4: copy constructor.
void* native_string_int_copy_construct_00614f30(void* self, const void* source, Seg25Host& host) {
    auto* dst = static_cast<NativeStr*>(self);
    const auto* src = static_cast<const NativeStr*>(source);
    dst->length = 0;
    dst->data = nullptr;
    if (dst != src) assign(host, dst, src);
    raw(self).at<std::uint32_t>(0x8) = raw(const_cast<void*>(source)).at<std::uint32_t>(0x8);
    return self;
}

// 006161F0 __fastcall(carousel), RET: zero the carousel state.
void powerup_carousel_init_006161f0(void* carousel) noexcept {
    Raw c = raw(carousel);
    for (std::size_t off : {0x1Cu, 0x20u, 0x24u, 0x2Cu, 0x30u, 0x34u, 0x4u, 0x38u, 0x10u, 0x0u})
        c.at<std::uint32_t>(off) = 0;
    c.at<std::uint8_t>(0x3D) = 0;
    c.at<std::uint8_t>(0x3E) = 0;
    c.at<std::uint8_t>(0x3F) = 0;
    c.at<std::uint32_t>(0x40) = 0;
}

// 00618EA0 __thiscall(screen), RET: powerup-manager destructor.
void powerup_manager_destroy_00618ea0(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CF4DECu;
    s.at<std::uint32_t>(0x8) = 0x00CF4DE8u;
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0xB0));
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0x24));
    host.award_index_list_destroy_00487110(s.p + 0x264);
    host.list_tidy_00613df0(s.p + 0x258);
    // Four maps, last first: erase [leftmost, head), free the head, zero head and size.
    struct Tree { std::size_t off; std::uint32_t eraser; };
    const Tree trees[] = {{0x1D8, 0x00616060u}, {0x1CC, 0x00616920u}, {0x1C0, 0x00616060u}, {0x1B4, 0x00616920u}};
    for (const Tree& t : trees) {
        void* head = s.ptr(t.off + 4);
        std::uint8_t out_iterator[8];
        host.tree_erase_range(t.eraser, s.p + t.off, out_iterator, raw(head).ptr(0x0), head);
        host.free_00bf65ac(s.ptr(t.off + 4));
        s.ptr(t.off + 4) = nullptr;
        s.at<std::uint32_t>(t.off + 8) = 0;
    }
    // Pointer vectors (first, last, end), last first.
    for (std::size_t first : {0x1A8u, 0x198u, 0x188u, 0x178u, 0x168u, 0x158u, 0x148u, 0x12Cu, 0x11Cu}) {
        if (s.ptr(first) != nullptr) host.free_00bf65ac(s.ptr(first));
        s.ptr(first) = nullptr;
        s.ptr(first + 4) = nullptr;
        s.ptr(first + 8) = nullptr;
    }
    host.frontend_destruct_004f71a0(screen);
}

// 00619160: MOV EAX, 4Fh; RET.
std::int32_t powerup_manager_get_screen_id_00619160() noexcept { return 0x4F; }

// 00619170 __thiscall(screen, children), RET 4: push &+B0h then &+24h.
void powerup_manager_collect_children_00619170(void* screen, void* children, Seg25Host& host) {
    std::uint8_t* s = static_cast<std::uint8_t*>(screen);
    host.vector_push_back_004d6790(children, s + 0xB0);
    host.vector_push_back_004d6790(children, s + 0x24);
}

// 00619AE0 __fastcall(screen), RET: unless sliding (+1F9h) fill the rows
// (00616C70). Unless a fade is running without its reset byte (+1E4h set, +1E5h
// clear): clear +1E5h, hide the four row widgets +68h.., then for up to four
// entries of the pointer vector +188h show row i, set its frame from the map
// +1CCh at entry[(+1ECh + i) % count]+4h (a word, vslot88(w, 0, 1.0f)), and in
// mode +20h == 1 show the marker +88h+4i only for the selected row +1F0h.
// C4724: the divisor is non-zero on every path that reaches the DIV (the loop returns first).
#pragma warning(push)
#pragma warning(disable : 4724)
void powerup_manager_refresh_second_rows_00619ae0(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    if (s.at<std::uint8_t>(0x1F9) == 0) host.powerup_fill_second_rows_00616c70(screen);
    if (s.at<std::uint8_t>(0x1E4) != 0 && s.at<std::uint8_t>(0x1E5) == 0) return;
    if (s.at<std::uint8_t>(0x1E5) != 0) s.at<std::uint8_t>(0x1E5) = 0;
    for (int i = 0; i < 4; ++i) host.widget_vslot34(s.ptr(0x68 + 4 * i), 0);
    const auto count = [&]() -> std::uint32_t {
        auto* first = static_cast<std::uint8_t*>(s.ptr(0x188));
        return first == nullptr ? 0u : static_cast<std::uint32_t>((static_cast<std::uint8_t*>(s.ptr(0x18C)) - first) >> 2);
    };
    for (std::int32_t i = 0; i < 4; ++i) {
        if (s.ptr(0x188) == nullptr) return;
        if (!(static_cast<std::uint32_t>(i) < count())) return;
        host.widget_vslot34(s.ptr(0x68 + 4 * i), 1);
        const std::uint32_t index = (s.at<std::uint32_t>(0x1EC) + static_cast<std::uint32_t>(i)) % count();
        if (s.ptr(0x188) == nullptr || !(index < count())) host.invalid_parameter_00bf6713();
        void* entry = static_cast<void**>(s.ptr(0x188))[index];
        const std::uint32_t key = raw(entry).at<std::uint32_t>(0x4);
        const std::uint16_t frame = *static_cast<std::uint16_t*>(host.string_map_ci_subscript_00617870(s.p + 0x1CC, key));
        host.widget_vslot88(s.ptr(0x68 + 4 * i), frame, 0, 1.0f);
        if (s.at<std::int32_t>(0x20) == 1)
            host.widget_vslot34(s.ptr(0x88 + 4 * i), s.at<std::int32_t>(0x1F0) == i ? 1 : 0);
    }
}

#pragma warning(pop)

// 0061A480 __thiscall(screen), RET: powerup-manager constructor.
void* powerup_manager_construct_0061a480(void* screen, Seg25Host& host) {
    Raw s = raw(screen);
    host.frontend_construct_004f7180(screen);
    s.at<std::uint32_t>(0x8) = 0x00CF4C3Cu;
    s.at<std::uint32_t>(0x0) = 0x00CF4DECu;
    s.at<std::uint32_t>(0x8) = 0x00CF4DE8u;
    for (std::size_t off : {0x11Cu, 0x120u, 0x124u, 0x12Cu, 0x130u, 0x134u, 0x104u, 0x138u, 0x110u, 0x100u})
        s.at<std::uint32_t>(off) = 0;
    s.at<std::uint8_t>(0x13D) = 0;
    s.at<std::uint8_t>(0x13E) = 0;
    s.at<std::uint32_t>(0x140) = 0;
    s.at<std::uint8_t>(0x13F) = 0;
    for (std::size_t off = 0x148; off <= 0x1B0; off += 0x10) {   // seven pointer vectors
        s.at<std::uint32_t>(off) = 0;
        s.at<std::uint32_t>(off + 4) = 0;
        s.at<std::uint32_t>(off + 8) = 0;
    }
    struct Tree { std::size_t off; std::uint32_t buyer; std::size_t isnil; };
    const Tree trees[] = {{0x1B4, 0x00613AE0u, 0x19}, {0x1C0, 0x00613B30u, 0x15},
                          {0x1CC, 0x00613AE0u, 0x19}, {0x1D8, 0x00613B30u, 0x15}};
    for (const Tree& t : trees) {
        void* head = host.container_buy_head_node(t.buyer, s.p + t.off);
        s.ptr(t.off + 4) = head;
        raw(head).at<std::uint8_t>(t.isnil) = 1;
        raw(s.ptr(t.off + 4)).ptr(0x4) = s.ptr(t.off + 4);
        raw(s.ptr(t.off + 4)).ptr(0x0) = s.ptr(t.off + 4);
        raw(s.ptr(t.off + 4)).ptr(0x8) = s.ptr(t.off + 4);
        s.at<std::uint32_t>(t.off + 8) = 0;
    }
    s.ptr(0x25C) = host.container_buy_head_node(0x00613860u, s.p + 0x258);
    s.at<std::uint32_t>(0x260) = 0;
    s.ptr(0x268) = host.container_buy_head_node(0x004870F0u, s.p + 0x264);
    s.at<std::uint32_t>(0x26C) = 0;
    s.at<std::uint32_t>(0xB0) = 0;
    s.at<std::uint32_t>(0x24) = 0;
    return screen;
}

// 0061AFF0 __thiscall(screen, float dt), RET 4: while +1E4h is set and +200h
// clear: +1E8h -= dt (clamped at 0). Above 0.5f the four widgets +58h.. get
// alpha 2v - 1.0; otherwise +1F0h/+1ECh/+1F4h = 0, +1E5h = 1, 00619AE0, and the
// alpha is 1.0 - 2v (DE E1 at 0061B0B2, FSUBRP ST1). +1E4h clears when v == 0.
void powerup_manager_tick_second_fade_0061aff0(void* screen, float dt, Seg25Host& host) {
    Raw s = raw(screen);
    if (s.at<std::uint8_t>(0x1E4) == 0 || s.at<std::uint8_t>(0x200) != 0) return;
    const float v = static_cast<float>(static_cast<double>(s.at<float>(0x1E8)) - dt);
    s.at<float>(0x1E8) = v;
    if (0.0f > v) s.at<float>(0x1E8) = 0.0f;
    if (s.at<float>(0x1E8) > 0.5f) {   // 00CE3800
        for (int i = 0; i < 4; ++i)
            host.widget_set_alpha_recursive_00aa8610(s.ptr(0x58 + 4 * i),
                static_cast<float>(static_cast<double>(s.at<float>(0x1E8)) * 2.0 - 1.0));
    } else {
        s.at<std::uint32_t>(0x1F0) = 0;
        s.at<std::uint32_t>(0x1EC) = 0;
        s.at<std::uint32_t>(0x1F4) = 0;
        s.at<std::uint8_t>(0x1E5) = 1;
        powerup_manager_refresh_second_rows_00619ae0(screen, host);
        for (int i = 0; i < 4; ++i)
            host.widget_set_alpha_recursive_00aa8610(s.ptr(0x58 + 4 * i),
                static_cast<float>(1.0 - static_cast<double>(s.at<float>(0x1E8)) * 2.0));
    }
    if (s.at<float>(0x1E8) == 0.0f) s.at<std::uint8_t>(0x1E4) = 0;
}

// 0061B840 __thiscall(ECX = listener = screen+8h, word), RET 4: 0061B0F0(screen).
void powerup_manager_on_listener_refresh_0061b840(void* listener, Seg25Host& host) {
    host.powerup_rebuild_lists_0061b0f0(static_cast<std::uint8_t*>(listener) - 8);
}

// 0061C800 __thiscall(screen, widget, bool hovered), RET 8: kind 3 widgets get
// alpha 0.5f when hovered, else 1.0f. The three tab widgets +C0h/+118h/+18Ch are
// left alone while their byte +A4h/+164h/+1D8h is set. Otherwise kind 6 gets
// vslot88(hovered, 0, 1.0f) and kind 12h vslot84(hovered).
void rewards_screen_on_widget_hover_0061c800(void* screen, void* widget, bool hovered, Seg25Host& host) {
    Raw s = raw(screen);
    if (host.widget_vslot5c(widget) == 3) {
        host.widget_vslot4c(widget, hovered ? 0.5f : 1.0f);   // 00CE3800 / 00D7A24C
        return;
    }
    bool locked = false;
    if (widget == s.ptr(0xC0)) {
        locked = s.at<std::uint8_t>(0xA4) != 0;
    } else if (widget == s.ptr(0x118)) {
        locked = s.at<std::uint8_t>(0x164) != 0;
    } else if (widget == s.ptr(0x18C)) {
        locked = s.at<std::uint8_t>(0x1D8) != 0;
    }
    if (locked) return;
    if (host.widget_vslot5c(widget) == 6) {
        host.widget_vslot88(widget, hovered ? 1u : 0u, 0, 1.0f);
        return;
    }
    if (host.widget_vslot5c(widget) == 0x12) host.widget_vslot84(widget, hovered ? 1 : 0);
}

}  // namespace bsp::seg25_cc13
