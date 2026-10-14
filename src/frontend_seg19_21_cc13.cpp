// Segments 19 and 21, cc13-seg19-21. Evidence and per-function notes:
// docs/CC13_SEG19_21.md. Status: reconstructed and Win32 build-tested only.
#include "bsp/frontend_seg19_21_cc13.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp::frontend_seg19_21_cc13 {
namespace {

constexpr std::uint32_t kHud = 0x00E198C4u;          // HUD interface singleton
constexpr std::uint32_t kGame = 0x00E188A8u;         // game object
constexpr std::uint32_t kLoadingScreen = 0x00E194B4u;
constexpr std::uint32_t kRenderService = 0x00F8D39Cu;

void* global_ptr(Seg19_21Host& host, std::uint32_t address) {
    return *static_cast<void**>(host.global(address));
}

// The inlined pool release every native-string temporary ends with: when the
// buffer is non-null, 00419CC0 then 00BD1510(buffer, length + 1, 1).
void release_pool_block(Seg19_21Host& host, void* block, std::uint32_t length) {
    if (block != nullptr) {
        void* pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, block, length + 1, 1);
    }
}

// The inlined native-string copy of 005C9830 / 005C9CC0 / 0057CFF0: resize the
// destination to the source length, then memcpy that many bytes (no terminator).
void native_string_assign(Seg19_21Host& host, NativeStr* dst, const NativeStr* src) {
    host.native_string_resize_0041dd40(dst, src->length, 1);
    if (src->length != 0) std::memcpy(dst->data, src->data, dst->length);
}

// The critical-section bracket of the singleton lifetime manager: the section is
// [manager+10h] and the depth counter is section+18h.
void* lifetime_lock(Seg19_21Host& host) {
    void* manager = host.singleton_manager_00415350();
    void* cs = raw(manager).ptr(0x10);
    if (cs != nullptr) {
        host.enter_critical_section(cs);
        raw(cs).at<std::int32_t>(0x18) += 1;
    }
    return cs;
}
void lifetime_unlock(Seg19_21Host& host, void* cs) {
    if (cs != nullptr) {
        raw(cs).at<std::int32_t>(0x18) -= 1;
        host.leave_critical_section(cs);
    }
}

// Session records are A0h bytes; an MSVC std::wstring sits at +8h (buffer +0Ch,
// size +1Ch, capacity +20h, inline while the capacity is below 8).
void copy_session_record_tail(Raw d, Raw s) {
    for (std::size_t off = 0x24; off <= 0x44; off += 4) d.at<std::uint32_t>(off) = s.at<std::uint32_t>(off);
    d.at<std::uint8_t>(0x48) = s.at<std::uint8_t>(0x48);
    d.at<std::uint8_t>(0x49) = s.at<std::uint8_t>(0x49);
    for (std::size_t off = 0x4C; off < 0x84; off += 4) d.at<std::uint32_t>(off) = s.at<std::uint32_t>(off);  // REP MOVSD, 0Eh dwords
    for (std::size_t off = 0x88; off <= 0x9C; off += 4) d.at<std::uint32_t>(off) = s.at<std::uint32_t>(off);
}

constexpr std::uint32_t kStringNpos = 0xFFFFFFFFu;

}  // namespace

// ===========================================================================
// Native string helpers
// ===========================================================================

// The build every caller inlines: a zeroed {0, null}, 0041DD40(length, 1), and
// when a buffer came back, memcpy(buffer, text, length + 1).
NativeStr native_string_build(Seg19_21Host& host, const char* text, std::uint32_t length) {
    NativeStr s{0, nullptr};
    host.native_string_resize_0041dd40(&s, length, 1);
    if (s.data != nullptr) std::memcpy(s.data, text, s.length + 1);
    return s;
}

// 005CC750 __fastcall(ECX = string), RET: release the buffer to the sized pool.
void native_string_release_storage_005cc750(NativeStr& s, Seg19_21Host& host) {
    release_pool_block(host, s.data, s.length);
}

// ===========================================================================
// Segment 19: session browser
// ===========================================================================

// 00570A30 __thiscall(this, const record*), RET 4: copy constructor.
void* session_record_copy_construct_00570a30(void* self, const void* source, Seg19_21Host& host) {
    Raw d = raw(self);
    Raw s = raw(const_cast<void*>(source));
    d.at<std::uint32_t>(0x0) = s.at<std::uint32_t>(0x0);
    d.at<std::uint8_t>(0x4) = s.at<std::uint8_t>(0x4);
    d.at<std::uint32_t>(0x20) = 7;        // empty wstring: capacity 7, size 0, L'\0'
    d.at<std::uint32_t>(0x1C) = 0;
    d.at<std::uint16_t>(0xC) = 0;
    host.wstring_assign_0056f160(d.p + 0x8, s.p + 0x8, 0, kStringNpos);
    copy_session_record_tail(d, s);
    return self;
}

// 00571A20 __thiscall(this, const record*), RET 4: assignment.
void* session_record_assign_00571a20(void* self, const void* source, Seg19_21Host& host) {
    Raw d = raw(self);
    Raw s = raw(const_cast<void*>(source));
    d.at<std::uint32_t>(0x0) = s.at<std::uint32_t>(0x0);
    d.at<std::uint8_t>(0x4) = s.at<std::uint8_t>(0x4);
    host.wstring_assign_0056f160(d.p + 0x8, s.p + 0x8, 0, kStringNpos);
    copy_session_record_tail(d, s);
    return self;
}

// 00571AF0 __fastcall(ECX = first, EDX = last), RET 8 (two unused stack words):
// destroy each record's wstring.
void session_record_destroy_range_00571af0(void* first, void* last, Seg19_21Host& host) {
    for (std::uint8_t* p = static_cast<std::uint8_t*>(first); p != last; p += 0xA0) {
        Raw r = raw(p);
        if (r.at<std::uint32_t>(0x20) > 7) host.free_00bf65ac(r.ptr(0xC));
        r.at<std::uint32_t>(0x20) = 7;
        r.at<std::uint32_t>(0x1C) = 0;
        r.at<std::uint16_t>(0xC) = 0;
    }
}

// 005725A0 __fastcall(ECX = a, EDX = b), RET: tmp(a); a = b; b = tmp; ~tmp.
void session_record_swap_005725a0(void* a, void* b, Seg19_21Host& host) {
    alignas(4) std::uint8_t tmp[0xA0];
    session_record_copy_construct_00570a30(tmp, a, host);
    session_record_assign_00571a20(a, b, host);
    session_record_assign_00571a20(b, tmp, host);
    if (raw(tmp).at<std::uint32_t>(0x20) >= 8) host.free_00bf65ac(raw(tmp).ptr(0xC));
}

// 00573A60 __thiscall(screen), RET: MultiSessionBrowserScreen destructor.
void multi_session_browser_destruct_00573a60(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CEF134u;
    s.at<std::uint32_t>(0x8) = 0x00CEF120u;
    s.at<std::uint32_t>(0xC) = 0x00CEF100u;
    for (std::size_t off = 0x54; off < 0x54 + 9 * 4; off += 4) {  // nine owned widgets
        if (void* object = s.ptr(off)) host.delete_vslot04(object, 1);
    }
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0x28));
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0x2C));
    for (std::size_t vec : {std::size_t{0x2A0}, std::size_t{0x290}}) {  // two record vectors
        if (s.ptr(vec + 4) != nullptr) {
            session_record_destroy_range_00571af0(s.ptr(vec + 4), s.ptr(vec + 8), host);
            host.free_00bf65ac(s.ptr(vec + 4));
        }
        s.ptr(vec + 4) = nullptr;
        s.ptr(vec + 8) = nullptr;
        s.ptr(vec + 0xC) = nullptr;
    }
    // The tree at +278h (head +27Ch, size +280h): erase [leftmost, head), free the head.
    void* head = s.ptr(0x27C);
    std::uint8_t out_iterator[4];
    host.tree_erase_range_005720e0(s.p + 0x278, out_iterator, raw(head).ptr(0x0), head);
    host.free_00bf65ac(s.ptr(0x27C));
    s.ptr(0x27C) = nullptr;
    s.at<std::uint32_t>(0x280) = 0;
    s.at<std::uint32_t>(0x8) = 0x00CEB0FCu;
    host.frontend_destruct_004f71a0(screen);
}

// 00573BB0: MOV EAX, 1Ch; RET.
std::int32_t multi_session_browser_get_screen_id_00573bb0() noexcept { return 0x1C; }

// 00573BD0 __thiscall(ECX = listener = screen+0Ch), RET: recapture both
// scrollers (screen+12Ch, screen+188h), then tail-jump to 00568930(screen).
void multi_session_browser_listener_current_1c_00573bd0(void* listener, Seg19_21Host& host) {
    std::uint8_t* l = static_cast<std::uint8_t*>(listener);
    host.scroller_recapture_006833f0(l + 0x120);
    host.scroller_recapture_006833f0(l + 0x17C);
    host.session_browser_layout_filter_rows_00568930(l - 0xC);
}

// 00573C00 __thiscall(screen, children vector), RET 4: push both pages.
void multi_session_browser_collect_children_00573c00(void* screen, void* children, Seg19_21Host& host) {
    std::uint8_t* s = static_cast<std::uint8_t*>(screen);
    host.vector_push_back_004d6790(children, s + 0x28);
    host.vector_push_back_004d6790(children, s + 0x2C);
}

// 00574050 __thiscall(profile), RET: low byte of profile+A0h map["RANK"].
std::uint8_t session_filter_read_rank_setting_00574050(void* profile, Seg19_21Host& host) {
    NativeStr key = native_string_build(host, "RANK", 4);   // 00CEF15C
    void* value = host.case_insensitive_map_index_005070c0(static_cast<std::uint8_t*>(profile) + 0xA0, &key);
    const std::uint8_t rank = *static_cast<std::uint8_t*>(value);
    release_pool_block(host, key.data, key.length);
    return rank;
}

// 00574240 __thiscall(screen), RET: MultiSessionBrowserScreen constructor.
void* multi_session_browser_construct_00574240(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.frontend_construct_004f7180(screen);
    s.at<std::uint32_t>(0x8) = 0x00CEB0FCu;
    s.at<std::uint32_t>(0xC) = 0x00CEB110u;
    s.at<std::uint32_t>(0x0) = 0x00CEF134u;
    s.at<std::uint32_t>(0x8) = 0x00CEF120u;
    s.at<std::uint32_t>(0xC) = 0x00CEF100u;
    host.scroller_construct_00683610(s.p + 0x12C);
    host.scroller_construct_00683610(s.p + 0x188);
    host.scroller_construct_00683610(s.p + 0x1E4);
    void* tree = s.p + 0x278;
    void* head = host.tree_buy_head_node(0x0056A430u, tree);
    s.ptr(0x27C) = head;
    raw(head).at<std::uint8_t>(0x11) = 1;              // _Isnil
    raw(s.ptr(0x27C)).ptr(0x4) = s.ptr(0x27C);
    raw(s.ptr(0x27C)).ptr(0x0) = s.ptr(0x27C);
    raw(s.ptr(0x27C)).ptr(0x8) = s.ptr(0x27C);
    s.at<std::uint32_t>(0x280) = 0;
    for (std::size_t off : {0x294u, 0x298u, 0x29Cu, 0x2A4u, 0x2A8u, 0x2ACu, 0x28Cu, 0x2B0u})
        s.at<std::uint32_t>(off) = 0;
    s.at<float>(0x270) = 2.5f;                          // 00CEECEC
    s.at<std::int32_t>(0x274) = -1;
    // The inlined clear(): erase from the root, re-link the head, size 0.
    host.tree_erase_subtree_0056a090(tree, raw(s.ptr(0x27C)).ptr(0x4));
    raw(s.ptr(0x27C)).ptr(0x4) = s.ptr(0x27C);
    s.at<std::uint32_t>(0x280) = 0;
    raw(s.ptr(0x27C)).ptr(0x0) = s.ptr(0x27C);
    raw(s.ptr(0x27C)).ptr(0x8) = s.ptr(0x27C);
    s.at<std::uint8_t>(0x288) = 0;
    s.at<float>(0x284) = 0.0f;
    return screen;
}

// 005769E0 __thiscall(this), RET: five empty std::maps at +4h, +10h, +1Ch, +28h, +34h.
void* multi_lobby_option_registry_construct_005769e0(void* self, Seg19_21Host& host) {
    Raw r = raw(self);
    r.at<std::uint32_t>(0x0) = 0x00CEF164u;
    struct TreeInit { std::size_t off; std::uint32_t buyer; std::size_t isnil; };
    const TreeInit trees[] = {{0x4, 0x0056A620u, 0x25}, {0x10, 0x0056A670u, 0x1D}, {0x1C, 0x0056A6C0u, 0x1D},
                              {0x28, 0x0056A710u, 0x1D}, {0x34, 0x0056A670u, 0x1D}};
    for (const TreeInit& t : trees) {
        void* tree = r.p + t.off;
        void* head = host.tree_buy_head_node(t.buyer, tree);
        r.ptr(t.off + 4) = head;
        raw(head).at<std::uint8_t>(t.isnil) = 1;
        raw(r.ptr(t.off + 4)).ptr(0x4) = r.ptr(t.off + 4);
        raw(r.ptr(t.off + 4)).ptr(0x0) = r.ptr(t.off + 4);
        raw(r.ptr(t.off + 4)).ptr(0x8) = r.ptr(t.off + 4);
        r.at<std::uint32_t>(t.off + 8) = 0;
    }
    return self;
}

// 00576B10 __cdecl(), RET: the C4h-byte registry singleton at 00F8894C,
// created under the lifetime manager's lock and registered with it.
void* multi_lobby_option_registry_get_singleton_00576b10(Seg19_21Host& host) {
    void*& instance = *static_cast<void**>(host.global(0x00F8894Cu));
    if (instance == nullptr) {
        void* cs = lifetime_lock(host);
        if (instance == nullptr) {
            void* block = host.operator_new_00bf681b(0xC4);
            instance = block == nullptr ? nullptr : multi_lobby_option_registry_construct_005769e0(block, host);
            host.singleton_register_00bd0c30(host.singleton_manager_00415350(), instance);
        }
        lifetime_unlock(host, cs);
    }
    return instance;
}

// ===========================================================================
// Segment 19: loading screen
// ===========================================================================

// 0057BF10 __thiscall(screen, phase, float fraction), RET 8. The weights are the
// six floats at 00E08764. Every partial sum is stored to a float; the last term
// (weight * fraction + sum) is rounded once.
void loading_progress_compute_fraction_0057bf10(void* screen, std::int32_t phase, float fraction, Seg19_21Host& host) {
    const float* weights = static_cast<const float*>(host.global(0x00E08764u));
    float sum = 0.0f;
    std::int32_t i = 0;
    if (phase >= 4) {
        do {
            sum = weights[i] + sum;
            sum = sum + weights[i + 1];
            sum = sum + weights[i + 2];
            sum = sum + weights[i + 3];
            i += 4;
        } while (i < phase - 3);
    }
    for (; i < phase; ++i) sum = weights[i] + sum;
    const float value = static_cast<float>(static_cast<double>(weights[phase]) * fraction + sum);
    Raw s = raw(screen);
    const float previous = s.at<float>(0x30);
    s.at<float>(0x30) = previous > value ? previous : value;
    // ST0 still holds the new value (not the maximum): 00BF7420 truncates value * 128.0.
    const std::int32_t units = static_cast<std::int32_t>(static_cast<double>(value) * 128.0);
    if (!(s.at<std::int32_t>(0x2C) > units) && s.at<std::int32_t>(0x2C) != units) s.at<std::int32_t>(0x2C) = units;
}

// 0057BFF0 __thiscall(base = screen+8h), RET: publish [00E194B4] = screen and
// register base with the lifetime manager, under its lock.
void* loading_screen_register_singleton_0057bff0(void* base, Seg19_21Host& host) {
    raw(base).at<std::uint32_t>(0x0) = 0x00CEF260u;
    void* cs = lifetime_lock(host);
    void* screen = static_cast<std::uint8_t*>(base) - 8;
    *static_cast<void**>(host.global(kLoadingScreen)) = screen;
    host.singleton_register_00bd0c30(host.singleton_manager_00415350(), screen != nullptr ? base : nullptr);
    lifetime_unlock(host, cs);
    return base;
}

// 0057C090 __thiscall(base), RET: unregister and clear [00E194B4].
void loading_screen_singleton_destruct_0057c090(void* base, Seg19_21Host& host) {
    raw(base).at<std::uint32_t>(0x0) = 0x00CEF260u;
    void* cs = lifetime_lock(host);
    void*& screen = *static_cast<void**>(host.global(kLoadingScreen));
    void* registered = screen == nullptr ? nullptr : static_cast<std::uint8_t*>(screen) + 8;
    host.singleton_unregister_00bcfca0(host.singleton_manager_00415350(), registered);
    screen = nullptr;
    lifetime_unlock(host, cs);
    raw(base).at<std::uint32_t>(0x0) = 0x00CE3818u;
}

// 0057C1A0 __fastcall(ECX = phase, float fraction), RET 4.
void loading_progress_report_phase_0057c1a0(std::int32_t phase, float fraction, Seg19_21Host& host) {
    if (void* screen = global_ptr(host, kLoadingScreen))
        loading_progress_compute_fraction_0057bf10(screen, phase, fraction, host);
}

// 0057C1C0 __thiscall(screen), RET: LoadingScreen constructor.
void* loading_screen_construct_0057c1c0(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.frontend_construct_004f7180(screen);
    loading_screen_register_singleton_0057bff0(s.p + 0x8, host);
    s.at<std::uint32_t>(0x8) = 0x00CEF28Cu;
    s.at<std::uint32_t>(0x0) = 0x00CEF264u;
    s.at<std::uint32_t>(0x38) = 0;
    s.at<std::uint32_t>(0x3C) = 0;
    s.at<std::uint32_t>(0x44) = 0;
    s.at<std::uint32_t>(0x40) = 1;
    return screen;
}

// 0057C230: MOV EAX, 59h; RET.
std::int32_t loading_screen_get_screen_id_0057c230() noexcept { return 0x59; }

// 0057C2E0 __thiscall(screen), RET: LoadingScreen destructor.
void loading_screen_destruct_0057c2e0(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CEF264u;
    s.at<std::uint32_t>(0x8) = 0x00CEF28Cu;
    host.gui_remove_and_destroy_page_00aa31f0(host.gui_manager_004c12b0(), s.at<std::uint32_t>(0xC));
    raw(global_ptr(host, kGame)).at<std::uint8_t>(0x719E) = 1;
    loading_screen_singleton_destruct_0057c090(s.p + 0x8, host);
    host.frontend_destruct_004f71a0(screen);
}

// 0057CF10 __thiscall(screen, children), RET 4: push the page slot +0Ch.
void loading_screen_collect_children_0057cf10(void* screen, void* children, Seg19_21Host& host) {
    host.vector_push_back_004d6790(children, static_cast<std::uint8_t*>(screen) + 0xC);
}

// The loading configuration record: string vector +0h, native string +10h, byte +18h.
// 0057CFF0 __thiscall(ECX = source), RET: copy it into the live config at 00E08780.
void loading_screen_publish_config_0057cff0(const void* source, Seg19_21Host& host) {
    Raw src = raw(const_cast<void*>(source));
    host.string_vector_assign_00506c80(host.global(0x00E08780u), source);
    auto* name = static_cast<NativeStr*>(host.global(0x00E08790u));
    const auto* from = reinterpret_cast<const NativeStr*>(src.p + 0x10);
    if (from != name) native_string_assign(host, name, from);
    *static_cast<std::uint8_t*>(host.global(0x00E08798u)) = src.at<std::uint8_t>(0x18);
}

// 0057D060 __thiscall(ECX = source), RET: copy into the pending config at
// 00E0879C, then tail-jump to 0057CFF0 with that config as the source.
void loading_screen_set_mission_text_0057d060(const void* source, Seg19_21Host& host) {
    Raw src = raw(const_cast<void*>(source));
    void* pending = host.global(0x00E0879Cu);
    host.string_vector_assign_00506c80(pending, source);
    auto* name = static_cast<NativeStr*>(host.global(0x00E087ACu));
    const auto* from = reinterpret_cast<const NativeStr*>(src.p + 0x10);
    if (from != name) native_string_assign(host, name, from);
    *static_cast<std::uint8_t*>(host.global(0x00E087B4u)) = src.at<std::uint8_t>(0x18);
    loading_screen_publish_config_0057cff0(pending, host);
}

// 0057D3B0 __fastcall(ECX = result), RET: result 1 sets the quit byte 00E1AE75.
void main_menu_quit_confirm_callback_0057d3b0(std::int32_t result, Seg19_21Host& host) {
    if (result == 1) *static_cast<std::uint8_t*>(host.global(0x00E1AE75u)) = 1;
}

// 0057D3F0 __thiscall(screen), RET: hide the widgets +3DCh, +1B8h, +41Ch.
void loading_screen_hide_overlays_0057d3f0(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.widget_vslot34(s.ptr(0x3DC), 0);
    host.widget_vslot34(s.ptr(0x1B8), 0);
    host.widget_vslot34(s.ptr(0x41C), 0);
}

// 0057D430 __cdecl(), RET: the game state [00E08874] is 2, 3 or 0Bh.
bool game_is_loading_state_0057d430(Seg19_21Host& host) {
    const std::int32_t state = *static_cast<std::int32_t*>(host.global(0x00E08874u));
    return state == 2 || state == 3 || state == 0xB;
}

// ===========================================================================
// Segment 21: mission tree groups (34h bytes: three native strings +0h/+8h/+10h,
// dwords +18h..+20h, a mission record vector +24h) and the MissionTreeScreen
// ===========================================================================

// 005C9830 __thiscall(this, const group*), RET 4: copy constructor.
void* mission_group_copy_construct_005c9830(void* self, const void* source, Seg19_21Host& host) {
    Raw d = raw(self);
    Raw s = raw(const_cast<void*>(source));
    for (std::size_t off : {0x0u, 0x8u, 0x10u}) {
        auto* dst = reinterpret_cast<NativeStr*>(d.p + off);
        const auto* src = reinterpret_cast<const NativeStr*>(s.p + off);
        dst->length = 0;
        dst->data = nullptr;
        if (dst != src) native_string_assign(host, dst, src);
    }
    d.at<std::uint32_t>(0x18) = s.at<std::uint32_t>(0x18);
    d.at<std::uint32_t>(0x1C) = s.at<std::uint32_t>(0x1C);
    d.at<std::uint32_t>(0x20) = s.at<std::uint32_t>(0x20);
    host.mission_record_vector_copy_005c93c0(d.p + 0x24, s.p + 0x24);
    return self;
}

// 005C9B10 __fastcall(this), RET: default constructor (strings and vector pointers zero).
void mission_group_clear_005c9b10(void* self) noexcept {
    Raw g = raw(self);
    for (std::size_t off : {0x0u, 0x4u, 0x8u, 0xCu, 0x10u, 0x14u, 0x28u, 0x2Cu, 0x30u}) g.at<std::uint32_t>(off) = 0;
}

// 005C9B60 __fastcall(this), RET: destructor (vector, then the strings in reverse).
void mission_group_destroy_005c9b60(void* self, Seg19_21Host& host) {
    Raw g = raw(self);
    host.mission_record_vector_tidy_005c9370(g.p + 0x24);
    release_pool_block(host, g.ptr(0x14), g.at<std::uint32_t>(0x10));
    release_pool_block(host, g.ptr(0xC), g.at<std::uint32_t>(0x8));
    release_pool_block(host, g.ptr(0x4), g.at<std::uint32_t>(0x0));
}

// 005C9CC0 __thiscall(this, const group*), RET 4: assignment.
void* mission_group_assign_005c9cc0(void* self, const void* source, Seg19_21Host& host) {
    Raw d = raw(self);
    Raw s = raw(const_cast<void*>(source));
    for (std::size_t off : {0x0u, 0x8u, 0x10u}) {
        auto* dst = reinterpret_cast<NativeStr*>(d.p + off);
        const auto* src = reinterpret_cast<const NativeStr*>(s.p + off);
        if (dst != src) native_string_assign(host, dst, src);
    }
    d.at<std::uint32_t>(0x18) = s.at<std::uint32_t>(0x18);
    d.at<std::uint32_t>(0x1C) = s.at<std::uint32_t>(0x1C);
    d.at<std::uint32_t>(0x20) = s.at<std::uint32_t>(0x20);
    host.mission_record_vector_assign_005c9940(d.p + 0x24, s.p + 0x24);
    return self;
}

// 005CA300 __fastcall(ECX = first, EDX = count, value), RET 4: uninitialized_fill_n.
// The catch funclet (destroy the built prefix, rethrow) is not modelled.
void mission_group_uninitialized_fill_005ca300(void* first, std::uint32_t count, const void* value, Seg19_21Host& host) {
    std::uint8_t* p = static_cast<std::uint8_t*>(first);
    for (; count != 0; --count, p += 0x34) {
        if (p != nullptr) mission_group_copy_construct_005c9830(p, value, host);
    }
}

// 005CA880 __thiscall(screen), RET: MissionTreeScreen constructor.
void* mission_tree_screen_construct_005ca880(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.frontend_construct_004f7180(screen);
    s.at<std::uint32_t>(0x8) = 0x00CEB0FCu;
    s.at<std::uint32_t>(0x0) = 0x00CF170Cu;
    s.at<std::uint32_t>(0x8) = 0x00CF16F8u;
    for (std::size_t off : {0x18u, 0x1Cu, 0x20u, 0x28u, 0x2Cu, 0x30u}) s.at<std::uint32_t>(off) = 0;
    return screen;
}

// 005CA8E0: MOV EAX, 2; RET.
std::int32_t mission_tree_screen_get_screen_id_005ca8e0() noexcept { return 2; }

// 005CA900 __thiscall(screen), RET: MissionTreeScreen destructor.
void mission_tree_screen_destruct_005ca900(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CF170Cu;
    s.at<std::uint32_t>(0x8) = 0x00CF16F8u;
    host.mission_record_vector_tidy_005c9370(s.p + 0x24);
    host.mission_group_vector_tidy_005ca520(s.p + 0x14);
    s.at<std::uint32_t>(0x8) = 0x00CEB0FCu;
    host.frontend_destruct_004f71a0(screen);
}

// ===========================================================================
// Segment 21: HUD movie screens. Screen 36h (vtable 00CF1968, 98h bytes) is the
// movie-camera debug screen; the "MovCam" setters write its fields +34h..+60h,
// which 005CC770 loads from MovieCamera.lua. Screen 37h (vtable 00CF17A8, 9Ch)
// is the movie screen with the black bars; 38h (00CF17D0, 20h) the engine one.
// ===========================================================================

// 005CAF60 __thiscall(screen, float), RET 4: empty update slot.
void hud_movie_camera_debug_update_noop_005caf60(void*, float) noexcept {}

// 005CAF70 __thiscall(this, object), RET 4: forget the observed object at +28h.
void hud_movie_camera_debug_on_observed_released_005caf70(void* self, void* object) noexcept {
    if (raw(self).ptr(0x28) == object) raw(self).ptr(0x28) = nullptr;
}

// 005CAF90 / 005CAFA0 __thiscall(screen, float), RET 4.
void movie_camera_set_deviation_factor_005caf90(void* screen, float value) noexcept { raw(screen).at<float>(0x54) = value; }
void movie_camera_set_overview_height_005cafa0(void* screen, float value) noexcept { raw(screen).at<float>(0x5C) = value; }
// 005CAFB0 / 005CAFC0 __fastcall(screen), RET: back to the Lua defaults.
void movie_camera_reset_deviation_factor_005cafb0(void* screen) noexcept {
    raw(screen).at<std::uint32_t>(0x54) = raw(screen).at<std::uint32_t>(0x58);
}
void movie_camera_reset_overview_height_005cafc0(void* screen) noexcept {
    raw(screen).at<std::uint32_t>(0x5C) = raw(screen).at<std::uint32_t>(0x60);
}

// 005CAFF0 __thiscall(screen), RET: HudMovieScreen (37h) destructor.
void hud_movie_screen_destruct_005caff0(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    s.at<std::uint32_t>(0x0) = 0x00CF17A8u;
    s.at<std::uint32_t>(0x8) = 0x00CF1790u;
    if (void* object = s.ptr(0x1C)) host.entity_kill_00926d90(object, 1);
    host.callback_owner_destroy_00695870(s.p + 0x8);
    host.frontend_destruct_004f71a0(screen);
}

// 005CB070: MOV EAX, 37h; RET.
std::int32_t hud_movie_screen_get_screen_id_005cb070() noexcept { return 0x37; }

// 005CB080 __thiscall(this, object), RET 4: forget the object at +14h and clear +18h.
void hud_movie_screen_on_observed_released_005cb080(void* self, void* object) noexcept {
    Raw r = raw(self);
    if (r.ptr(0x14) == object) {
        r.ptr(0x14) = nullptr;
        r.at<std::uint8_t>(0x18) = 0;
    }
}

// 005CB0A0 __thiscall(screen), RET: HudEngineMovieScreen (38h) constructor.
void* hud_engine_movie_screen_construct_005cb0a0(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.frontend_construct_004f7180(screen);
    s.at<std::uint8_t>(0x9) = 0;
    s.at<std::uint8_t>(0x8) = 0;
    s.at<std::uint32_t>(0x0) = 0x00CF17D0u;
    s.at<float>(0xC) = 0.0f;
    return screen;
}

// 005CB0D0: MOV EAX, 38h; RET.
std::int32_t hud_engine_movie_screen_get_screen_id_005cb0d0() noexcept { return 0x38; }

// 005CB0F0 __thiscall(screen), RET: hide the two HUD info panels, start the bars
// opening (+8h = 1, +9h = 0), and clamp a negative bar position +0Ch to 0.
void hud_engine_movie_screen_show_black_bars_005cb0f0(void* screen, Seg19_21Host& host) {
    Raw hud = raw(global_ptr(host, kHud));
    host.hud_hide_info_widget_00652b60(hud.ptr(0xBC));
    host.hud_hide_info_page_00611bf0(raw(global_ptr(host, kHud)).ptr(0xC0));
    Raw s = raw(screen);
    s.at<std::uint8_t>(0x9) = 0;
    s.at<std::uint8_t>(0x8) = 1;
    if (0.0f > s.at<float>(0xC)) s.at<float>(0xC) = 0.0f;
}

// 005CB130 __thiscall(screen), RET: start the bars closing and clamp +0Ch to +1Ch.
void hud_engine_movie_screen_hide_black_bars_005cb130(void* screen) noexcept {
    Raw s = raw(screen);
    const float limit = s.at<float>(0x1C);
    s.at<std::uint8_t>(0x8) = 0;
    s.at<std::uint8_t>(0x9) = 1;
    if (s.at<float>(0xC) > limit) s.at<float>(0xC) = limit;
}

// 005CB160 __thiscall(screen), RET: like 005CB0F0 but the bars jump fully open
// (+0Ch = +1Ch) with the same flags.
void hud_engine_movie_screen_show_black_bars_now_005cb160(void* screen, Seg19_21Host& host) {
    host.hud_hide_info_widget_00652b60(raw(global_ptr(host, kHud)).ptr(0xBC));
    host.hud_hide_info_page_00611bf0(raw(global_ptr(host, kHud)).ptr(0xC0));
    Raw s = raw(screen);
    s.at<float>(0xC) = s.at<float>(0x1C);
    s.at<std::uint8_t>(0x9) = 0;
    s.at<std::uint8_t>(0x8) = 1;
}

// 005CB8B0 __cdecl(type), RET: message type 34h.
bool session_message52_is_type_005cb8b0(std::int32_t type) noexcept { return type == 0x34; }

// 005CB8C0 __thiscall(message, flags), RET 4: reset to the root vtable, free on bit 0.
void* session_message52_scalar_delete_005cb8c0(void* message, std::uint8_t flags, Seg19_21Host& host) {
    raw(message).at<std::uint32_t>(0x0) = 0x00CE4974u;
    if ((flags & 1) != 0) host.free_00bf65ac(message);
    return message;
}

// The speed setters: +34h linear and +38h angular limit, +44h acceleration time,
// +4Ch/+50h the derived accelerations (limit / time, each rounded to float).
// 005CB900 __thiscall(screen, linear, angular), RET 8.
void movie_camera_set_speed_limits_005cb900(void* screen, float linear, float angular) noexcept {
    Raw s = raw(screen);
    s.at<float>(0x34) = linear;
    s.at<float>(0x38) = angular;
    s.at<float>(0x4C) = linear / s.at<float>(0x44);
    s.at<float>(0x50) = angular / s.at<float>(0x44);
}
// 005CB940 __thiscall(screen, seconds), RET 4.
void movie_camera_set_accel_time_005cb940(void* screen, float seconds) noexcept {
    Raw s = raw(screen);
    s.at<float>(0x44) = seconds;
    s.at<float>(0x4C) = s.at<float>(0x34) / seconds;
    s.at<float>(0x50) = s.at<float>(0x38) / seconds;
}
// 005CB970 __fastcall(screen), RET: limits back to the defaults +3Ch/+40h.
void movie_camera_reset_speed_limits_005cb970(void* screen) noexcept {
    Raw s = raw(screen);
    s.at<float>(0x34) = s.at<float>(0x3C);
    s.at<float>(0x38) = s.at<float>(0x40);
    s.at<float>(0x4C) = s.at<float>(0x3C) / s.at<float>(0x44);
    s.at<float>(0x50) = s.at<float>(0x40) / s.at<float>(0x44);
}
// 005CB9C0 __fastcall(screen), RET: time back to the default +48h.
void movie_camera_reset_accel_time_005cb9c0(void* screen) noexcept {
    Raw s = raw(screen);
    const float seconds = s.at<float>(0x48);
    s.at<float>(0x44) = seconds;
    s.at<float>(0x4C) = s.at<float>(0x34) / seconds;
    s.at<float>(0x50) = s.at<float>(0x38) / seconds;
}
// 005CB9F0 / 005CBA20 __thiscall(screen, float), RET 4.
void movie_camera_set_linear_speed_limit_005cb9f0(void* screen, float value) noexcept {
    Raw s = raw(screen);
    s.at<float>(0x34) = value;
    s.at<float>(0x4C) = value / s.at<float>(0x44);
    s.at<float>(0x50) = s.at<float>(0x38) / s.at<float>(0x44);
}
void movie_camera_set_angular_speed_limit_005cba20(void* screen, float value) noexcept {
    Raw s = raw(screen);
    s.at<float>(0x38) = value;
    s.at<float>(0x4C) = s.at<float>(0x34) / s.at<float>(0x44);
    s.at<float>(0x50) = value / s.at<float>(0x44);
}

// 005CBA50 __thiscall(screen, int value), RET 4: widget +2Ch gets vslot44 with
// (float)(value * 1.5707963705062866) (FILD, FMUL double 00CE3830), a white
// colour, shown; +80h = 1.5f (00CE380C).
void movie_camera_show_dump_message_005cba50(void* screen, std::int32_t value, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.widget_vslot44(s.ptr(0x2C), static_cast<float>(static_cast<double>(value) * 1.5707963705062866));
    const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    host.widget_vslot50(s.ptr(0x2C), white);
    host.widget_vslot34(s.ptr(0x2C), 1);
    s.at<float>(0x80) = 1.5f;
}

namespace {
// 005CBC30 / 005CBCC0 tail: in a session game ([game+1FE4h] == 1) send message
// 34h (vtable 00CF17F8, +4h = 1, +18h = bars shown) through 0076A9F0.
void send_black_bars_message(Seg19_21Host& host, std::uint8_t shown) {
    void* game = global_ptr(host, kGame);
    if (raw(game).at<std::int32_t>(0x1FE4) != 1) return;
    alignas(4) std::uint8_t message[0x1C];
    host.session_message_construct_base_0075b430(message, 0x34);
    raw(message).at<std::uint32_t>(0x4) = 1;
    raw(message).at<std::uint32_t>(0x0) = 0x00CF17F8u;
    raw(message).at<std::uint8_t>(0x18) = shown;
    host.session_dispatch_0076a9f0(static_cast<std::uint8_t*>(global_ptr(host, kGame)) + 0x1EF0, message, 0);
}
}  // namespace

// 005CBC30 __thiscall(screen), RET: HudMovieScreen bars opening (+21h = 1, +22h = 0),
// clamp a negative +24h to 0, then the session message with 1.
void hud_movie_screen_show_black_bars_005cbc30(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    s.at<std::uint8_t>(0x22) = 0;
    s.at<std::uint8_t>(0x21) = 1;
    if (0.0f > s.at<float>(0x24)) s.at<float>(0x24) = 0.0f;
    send_black_bars_message(host, 1);
}

// 005CBCC0 __thiscall(screen), RET: bars closing, clamp +24h to +34h, message with 0.
void hud_movie_screen_hide_black_bars_005cbcc0(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    const float limit = s.at<float>(0x34);
    s.at<std::uint8_t>(0x21) = 0;
    s.at<std::uint8_t>(0x22) = 1;
    if (s.at<float>(0x24) > limit) s.at<float>(0x24) = limit;
    send_black_bars_message(host, 0);
}

// 005CBD90 __thiscall(screen, float seconds), RET 4: the engine movie screen's bar
// animation. Opening: +0Ch += seconds * 0.0625 (double 00CEF290), stop at +1Ch.
// Closing: +0Ch -= seconds / 20.0 (double 00CE3D88), stop below zero at -0.01f
// (00D7A300). Then both bars +10h/+14h shown while +0Ch > 0, placed at
// (-1, +0Ch, -20) and (-1, 1 - +0Ch, -20), and 00B0D0B0 gets +0Ch.
// DE E9 at 005CBDE6 is FSUBP ST1 (sum - quotient); DE E1 at 005CBE8E is FSUBRP ST1 (1 - v).
void hud_engine_movie_screen_update_black_bars_005cbd90(void* screen, float seconds, Seg19_21Host& host) {
    Raw s = raw(screen);
    if (s.at<std::uint8_t>(0x8) != 0) {
        const float v = static_cast<float>(static_cast<double>(seconds) * 0.0625 + s.at<float>(0xC));
        s.at<float>(0xC) = v;
        const float limit = s.at<float>(0x1C);
        if (v > limit) {
            s.at<std::uint8_t>(0x8) = 0;
            s.at<float>(0xC) = limit;
        }
    } else if (s.at<std::uint8_t>(0x9) != 0) {
        const float v = static_cast<float>(static_cast<double>(s.at<float>(0xC)) - static_cast<double>(seconds) / 20.0);
        s.at<float>(0xC) = v;
        if (0.0f > v) {
            s.at<std::uint8_t>(0x9) = 0;
            s.at<float>(0xC) = -0.01f;  // 00D7A300
        }
    }
    host.widget_vslot34(s.ptr(0x10), s.at<float>(0xC) > 0.0f ? 1 : 0);
    host.widget_vslot34(s.ptr(0x14), s.at<float>(0xC) > 0.0f ? 1 : 0);
    const float top[3] = {-1.0f, s.at<float>(0xC), -20.0f};
    host.widget_set_resolved_position_00aa8240(s.ptr(0x10), top);
    const float bottom[3] = {-1.0f, static_cast<float>(1.0 - static_cast<double>(s.at<float>(0xC))), -20.0f};
    host.widget_set_resolved_position_00aa8240(s.ptr(0x14), bottom);
    host.render_set_value224_00b0d0b0(global_ptr(host, kRenderService), s.at<float>(0xC));
}

// 005CC000 __thiscall(screen 36h), RET: when +30h is null, build the movie-camera
// mover (operator new 540h, memset 0, 0079F480 when non-null), place it with an
// identity matrix under [game+19CCh], install it as the game's camera mover and
// register the screen's observer +8h with it. The 00923870 call is made even
// when the allocation failed (ECX = 0), as in the image.
void movie_camera_debug_ensure_mover_005cc000(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    if (s.ptr(0x30) != nullptr) return;
    void* block = host.operator_new_00bf55be(0x540);
    std::memset(block, 0, 0x540);
    void* mover = block != nullptr ? host.movcam_construct_0079f480(block) : nullptr;
    s.ptr(0x30) = mover;
    const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    void* game = global_ptr(host, kGame);
    host.movcam_place_00923870(mover, 0, raw(game).at<std::uint32_t>(0x19CC), identity);
    host.game_set_camera_mover_004bc410(global_ptr(host, kGame), s.ptr(0x30));
    host.observer_register_pair_00694a60(s.ptr(0x30), s.p + 0x8);
}

// 005CC120 __thiscall(screen), RET: HudMovieScreen (37h) constructor.
void* hud_movie_screen_construct_005cc120(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.frontend_construct_004f7180(screen);
    s.at<std::uint32_t>(0x8) = 0x00CE3CD4u;
    s.at<std::uint32_t>(0xC) = 0;
    s.at<std::uint32_t>(0x10) = 0;
    s.at<std::uint32_t>(0x14) = 0;
    s.at<std::uint8_t>(0x18) = 0;
    s.at<std::uint32_t>(0x1C) = 0;
    s.at<std::uint8_t>(0x20) = 0;
    s.at<std::uint8_t>(0x21) = 0;
    s.at<std::uint8_t>(0x22) = 0;
    s.at<std::uint32_t>(0x0) = 0x00CF17A8u;
    s.at<std::uint32_t>(0x8) = 0x00CF1790u;
    s.at<std::uint32_t>(0x24) = 0;
    return screen;
}

// 005CC590 __cdecl(), RET: input context 3 at level 0; with a camera at
// [game+19FCh], field of view from the float at 00F889B4 and near plane 2.0f.
void movie_camera_debug_enter_005cc590(Seg19_21Host& host) {
    host.input_set_context_level_00a933f0(host.input_manager_004bec00(), 3, 0);
    if (raw(global_ptr(host, kGame)).ptr(0x19FC) != nullptr) {
        host.camera_set_fov_00b6fbb0(raw(global_ptr(host, kGame)).ptr(0x19FC), *static_cast<float*>(host.global(0x00F889B4u)));
        host.camera_set_near_plane_00b6fbf0(raw(global_ptr(host, kGame)).ptr(0x19FC), 2.0f);
    }
}

// 005CC5E0 __thiscall(screen, NativeStr text by value), RET 8: the text widget
// +24h shows the localised text in (1, 0.75, 1, 1), +7Ch = 1.5f; the callee then
// releases its by-value string.
void movie_camera_show_mode_text_005cc5e0(void* screen, NativeStr text, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.text_set_localised_source_00abaed0(s.ptr(0x24), &text, 1);
    const float colour[4] = {1.0f, 0.75f, 1.0f, 1.0f};   // 00D7A24C, 00CEE07C
    host.widget_vslot50(s.ptr(0x24), colour);
    host.widget_vslot34(s.ptr(0x24), 1);
    s.at<float>(0x7C) = 1.5f;
    release_pool_block(host, text.data, text.length);
}

// 005CC700 __thiscall(this), RET: two zero dwords, then 0078E230 on +48h.
void* vehicle_camera_array_construct_005cc700(void* self, Seg19_21Host& host) {
    Raw r = raw(self);
    r.at<std::uint32_t>(0x0) = 0;
    r.at<std::uint32_t>(0x4) = 0;
    host.vehicle_camera_limits_init_0078e230(r.p + 0x48);
    return self;
}

// 005CC770 __thiscall(screen 36h), RET: register, load GUI_moviecamera_debug into
// +1Ch, run Scripts\datatables\MovieCamera.lua and read the MovieCamera table
// into the current/default pairs (+34h/+3Ch, +38h/+40h, +44h/+48h, +54h/+58h,
// +5Ch/+60h), then derive +4Ch/+50h. The Lua objects are opaque stack buffers.
void movie_camera_debug_register_005cc770(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.frontend_register_004f71d0(screen);
    NativeStr page = native_string_build(host, "GUI_moviecamera_debug", 0x15);   // 00CF18C8
    s.at<std::uint32_t>(0x1C) = host.gui_load_page_00aa5840(host.gui_manager_004c12b0(), &page, 1, 0);
    release_pool_block(host, page.data, page.length);

    alignas(4) std::uint8_t owner[0x4C8];
    alignas(4) std::uint8_t globals[0x14];
    alignas(4) std::uint8_t table[0x14];
    alignas(4) std::uint8_t value[0x14];
    host.lua_owner_construct_00b66bd0(owner);
    host.lua_owner_open_00b6a020(owner, 1);
    NativeStr path = native_string_build(host, "Scripts\\datatables\\MovieCamera.lua", 0x22);  // 00CF18A4
    host.lua_run_script_00b69d40(owner, &path, 0);
    release_pool_block(host, path.data, path.length);
    host.lua_get_globals_00b67980(owner, globals);
    host.lua_get_by_name_00b67800(globals, table, "MovieCamera");   // 00CF1898
    host.lua_object_destruct_00b67700(globals);
    struct Field { const char* name; std::size_t current; std::size_t fallback; };
    const Field fields[] = {{"MoveSpeedDefaultLimit", 0x34, 0x3C},          // 00CF1880
                            {"RotationSpeedDefaultLimit", 0x38, 0x40},      // 00CF1864
                            {"GeneralAccelerationDefaultTime", 0x44, 0x48}, // 00CF1844
                            {"DefaultDeviationFactor", 0x54, 0x58},         // 00CF182C
                            {"OverviewModeDefaultHeight", 0x5C, 0x60}};     // 00CF1810
    for (const Field& f : fields) {
        host.lua_get_by_name_00b67800(table, value, f.name);
        const float number = static_cast<float>(host.lua_get_number_00b66270(value));
        s.at<float>(f.current) = number;
        s.at<float>(f.fallback) = number;
        host.lua_object_destruct_00b67700(value);
    }
    const float seconds = s.at<float>(0x44);
    s.at<float>(0x4C) = s.at<float>(0x34) / seconds;
    s.at<float>(0x50) = s.at<float>(0x38) / seconds;
    host.lua_object_destruct_00b67700(table);
    host.lua_owner_close_00b669a0(owner);
}

// 005CCA80 __thiscall(screen 36h), RET: bind the four widgets of page +1Ch into
// +20h..+2Ch, hide them, build the mover (005CC000), +7Ch = 0.
void movie_camera_debug_bind_widgets_005cca80(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    struct Bind { const char* name; std::uint32_t length; std::size_t slot; };
    const Bind binds[] = {{"TargetNames_Text", 0x10, 0x20},     // 00CF191C
                          {"DeckPosName_Text", 0x10, 0x24},     // 00CF1908
                          {"TempEditCursor_Icon", 0x13, 0x28},  // 00CF18F4
                          {"TempEditArrow_Icon", 0x12, 0x2C}};  // 00CF18E0
    for (const Bind& b : binds) {
        NativeStr name = native_string_build(host, b.name, b.length);
        s.ptr(b.slot) = host.widget_find_child_00aa7e00(s.ptr(0x1C), &name, 1);
        release_pool_block(host, name.data, name.length);
    }
    host.widget_vslot34(s.ptr(0x20), 0);
    host.widget_vslot34(s.ptr(0x24), 0);
    host.widget_vslot34(s.ptr(0x28), 0);
    host.widget_vslot34(s.ptr(0x2C), 0);
    movie_camera_debug_ensure_mover_005cc000(screen, host);
    s.at<float>(0x7C) = 0.0f;
}

// 005CCCB0 __thiscall(screen 37h), RET: register, clear the bar flags, load
// GUI_movie into +30h, bind up_Icon (+28h) and down_Icon (+2Ch); +34h = 0.14f
// (00CE81A4) when [[0109CF04]+0Dh] is set, else 0.18f (00CE8198).
void hud_movie_screen_register_005cccb0(void* screen, Seg19_21Host& host) {
    Raw s = raw(screen);
    host.frontend_register_004f71d0(screen);
    s.at<std::uint8_t>(0x20) = 0;
    s.at<std::uint8_t>(0x22) = 0;
    s.at<std::uint8_t>(0x21) = 0;
    NativeStr page = native_string_build(host, "GUI_movie", 9);
    s.at<std::uint32_t>(0x30) = host.gui_load_page_00aa5840(host.gui_manager_004c12b0(), &page, 1, 0);
    release_pool_block(host, page.data, page.length);
    NativeStr up = native_string_build(host, "up_Icon", 7);
    s.ptr(0x28) = host.widget_find_child_00aa7e00(s.ptr(0x30), &up, 1);
    release_pool_block(host, up.data, up.length);
    NativeStr down = native_string_build(host, "down_Icon", 9);
    s.ptr(0x2C) = host.widget_find_child_00aa7e00(s.ptr(0x30), &down, 1);
    release_pool_block(host, down.data, down.length);
    const bool wide = raw(global_ptr(host, 0x0109CF04u)).at<std::uint8_t>(0xD) != 0;
    s.at<float>(0x34) = wide ? 0.14f : 0.18f;
}

}  // namespace bsp::frontend_seg19_21_cc13
