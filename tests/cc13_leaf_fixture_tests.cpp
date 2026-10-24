// Execute audited cc13 leaf routines from the original image beside their reconstructions.
// The bytes come from local/cc13_leaf_reference.hpp (tools/verify_cc13_leaf_reference.py), which
// relocates constants, BSS buffers and the few internal calls into one private copy. No game
// process, assets or original addresses are involved. One case per routine; inputs are a fixed
// pseudo-random set, compared bit for bit. The x87 control word is pinned to 53-bit precision,
// the CRT default; the game's main thread runs at 24 bits after device creation
// (docs/CC13_FPU_PRECISION.md), which these cases do not cover. The cc13-fixture2 cases
// (docs/CC13_FIXTURE_PASS_2.md) mix edge values into their inputs and run at both 53-bit and
// 24-bit precision control; the precision a reconstruction models decides.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "bsp/band_0040_cc13.hpp"
#include "bsp/band_0045_cc13.hpp"
#include "bsp/band_0050_cc13.hpp"
#include "bsp/band_0067_cc13.hpp"
#include "bsp/band_0076_cc13.hpp"
#include "bsp/band_0080_cc13.hpp"
#include "bsp/band_0090_cc13.hpp"
#include "bsp/frontend_seg12_17_cc13.hpp"
#include "bsp/ingame_gui_cc13.hpp"
#include "bsp/rocket_task_cc13.hpp"
#include "bsp/post_effects_cc13.hpp"
#include "bsp/seg6_cc13.hpp"
#include "bsp/scene_weather_cc13.hpp"
#include "bsp/engine_seg4_6_9_cc13.hpp"
#include "bsp/terrain_particles_cc13.hpp"
#include "bsp/cc13_seg59a.hpp"
#include "bsp/scene_graph2_cc13.hpp"
#include "bsp/cc13_left61.hpp"
#include "bsp/lua_spawn_cc13.hpp"
#include "bsp/cc13_seg67b.hpp"
#include "bsp/cc13_camera_controllers.hpp"
#include "bsp/cc13_seg47.hpp"
#include "bsp/sound_streaming_cc13.hpp"
#include "bsp/scene_graph_cc13.hpp"
#include "bsp/weather_render.hpp"
#include "bsp/cc13_sweepA1b.hpp"
#include "bsp/cc13_dyn1.hpp"
#include "bsp/render_fx_cc13.hpp"
#include "bsp/oct18b_nc_mid.hpp"
#include "cc13_leaf_reference.hpp"
#include <float.h>
#include <array>
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

unsigned char* image = nullptr;

template<class F> F at(unsigned offset) {
    F f{};
    void* p = image + offset;
    static_assert(sizeof(f) == sizeof(p));
    std::memcpy(&f, &p, sizeof(f));
    return f;
}

void load_image() {
    constexpr std::size_t n = sizeof(cc13_leaf_bytes);
    image = static_cast<unsigned char*>(VirtualAlloc(nullptr, n, MEM_COMMIT | MEM_RESERVE,
                                                     PAGE_EXECUTE_READWRITE));
    if (!image) throw std::runtime_error("VirtualAlloc failed");
    std::memcpy(image, cc13_leaf_bytes, n);
    const auto origin = reinterpret_cast<std::uintptr_t>(image);
    for (const auto& patch : cc13_leaf_patches) {
        const auto value = static_cast<std::uint32_t>(patch.relative
            ? patch.target - (patch.offset + 4) : origin + patch.target);
        std::memcpy(image + patch.offset, &value, 4);
    }
    FlushInstructionCache(GetCurrentProcess(), image, n);
}

std::uint32_t state = 0x13579bdfu;
std::uint32_t next() { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
float uniform(float lo, float hi) { return lo + (hi - lo) * static_cast<float>(next() >> 8) / 16777216.0f; }
template<class T> bool same(const T& a, const T& b) { return std::memcmp(&a, &b, sizeof(T)) == 0; }

struct Case {
    std::string name;
    int runs = 0, failures = 0, nan_only = 0;
    void check(bool ok) { ++runs; if (!ok) ++failures; }
    // cc13-fixture2: outputs that differ only where both sides are NaN (sign or payload).
    void check_nan_only() { ++runs; ++nan_only; }
};

// cc13-trivialslots: hosts that record the object a tail-jump or adjustor thunk hands on.
#pragma warning(push)
#pragma warning(disable : 4100 4459)
struct RecordingHudServices final : bsp::ingame_gui_cc13::HudScreenServices {
    using Raw = bsp::ingame_gui_cc13::Raw;
    void* seen = nullptr;
    void front_end_screen_construct_004f7180(Raw screen) override {}
    void front_end_screen_destruct_004f71a0(Raw screen) override {}
    void front_end_screen_register_004f71d0(Raw screen) override { seen = screen.p; }
    void front_end_screen_commit_visibility_004f83b0(Raw screen) override {}
    void screen_virtual_14(Raw screen) override {}
    void* gui_load_page_00aa5840(std::string_view name) override { return {}; }
    void gui_remove_and_destroy_page_00aa31f0(void* page) override {}
    void* gui_find_child_00aa7e00(void* parent, std::string_view name) override { return {}; }
    void widget_set_visible_34(void* widget, bool visible) override {}
    void widget_set_focus_60(void* widget, bool focus) override {}
    void widget_set_colour_50(void* widget, const float rgba[4]) override {}
    void widget_get_colour_54(void* widget, float rgba[4]) override {}
    void widget_set_float_4c(void* widget, float value) override {}
    void object_delete_04(void* object) override {}
    void listbox_set_listener_00a9ac40(void* listbox, void* listener) override {}
    void push_page_slot_004d6790(void* vector, void** slot) override {}
    void observer_register_00694a60(void* observer, void* observed) override {}
    void observer_unregister_006952a0(void* observer, void* observed) override {}
    bool observer_is_registered_00694af0(void* observer, void* observed) override { return {}; }
    bool action_edge_or_repeat_004d92b0(int action) override { return {}; }
    std::int32_t session_mode() override { return {}; }
};
struct RecordingFe3Host final : bsp::cc13_fe3::Fe3Host {
    using Raw = bsp::ingame_gui_cc13::Raw;
    void* seen = nullptr;
    void free_00bf65ac(void* block) override {}
    void free_00bf6989(void* block) override {}
    void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t count) override {}
    void front_end_screen_construct_004f7180(Raw screen) override {}
    void front_end_screen_register_004f71d0(Raw screen) override {}
    void object_delete(void* object) override {}
    void widget_virtual_48(void* widget, const float xy[2]) override {}
    void gui_set_state_texture_uv_00ab2690(void* widget, std::uint32_t state, std::uint32_t texture, const void* uv) override {}
    void gui_text_set_localised_00abbe50(void* text, const char* key, std::uint32_t flag) override {}
    void scroller_recapture_006833f0(void* scroller) override { seen = scroller; }
    std::int32_t listbox_selected_index_00a9c920(void* listbox) override { return {}; }
    void movie_player_stop_004f8ac0() override {}
    void game_cancel_leaderboard_read_004bc7f0() override {}
    void clear_frame_singleton_00e18d80() override {}
    const char* global_config_c8_name() override { return {}; }
    const char* empty_string_00e19476() override { return {}; }
    void input_settings_reload_006aa2d0() override {}
    void settings_save_to_profile_008d6c00() override {}
    void* control_binding_screen_00e198b8() override { return {}; }
    void* library_find_entry_by_name_004fbf70(void* atlas, const char* name) override { return {}; }
    void loading_job_pointers_resize_004fb4b0(void* vector, std::uint32_t size) override {}
    void award_index_list_destroy_00487110(void* list) override {}
    void library_entry_destruct_004fd1d0(void* entry) override {}
    void ref_ptr_array_resize_00504960(void* vector, std::uint32_t size) override {}
    void loading_queue_destroy_00509fd0(void* queue) override {}
    void iterator_next(std::uint32_t routine, void* iterator) override {}
    void* tree_allocate_head(std::uint32_t routine, void* tree) override { return {}; }
    void tree_destroy_subtree(std::uint32_t routine, void* tree, void* node) override {}
    void string_vector_range_destroy_0055a3a0(void* first, void* last, void* extra1, void* extra2) override {}
    void control_binding_refresh_page_0055e3a0(Raw screen, std::uint32_t argument) override {}
    void control_binding_load_selected_preset_0055f710(void* screen) override {}
};

#pragma warning(pop)

// cc13-fixture2: edge values mixed into the pseudo-random inputs (one draw in six): signed
// zeros, units, a quiet NaN, denormals, the smallest normal, huge, infinite and FLT_MAX.
float edge_or(float lo, float hi) {
    static const float edges[] = {0.0f, -0.0f, 1.0f, -1.0f, 0.5f, std::numeric_limits<float>::quiet_NaN(),
                                  1e-40f, -1e-40f, FLT_MIN, -FLT_MIN, 1e30f, -1e30f,
                                  std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(),
                                  FLT_MAX};
    static const bool plain = GetEnvironmentVariableA("BSP_FIXTURE_NO_EDGES", nullptr, 0) != 0;  // diagnosis only
    if (next() % 6 == 0 && !plain) return edges[next() % (sizeof(edges) / sizeof(edges[0]))];
    return uniform(lo, hi);
}

// Check, and with BSP_FIXTURE_VERBOSE set print the first three failing inputs of a case.
template<class... A> void check_note(Case& c, bool ok, const char* format, A... args) {
    c.check(ok);
    static const bool verbose = GetEnvironmentVariableA("BSP_FIXTURE_VERBOSE", nullptr, 0) != 0;
    if (!ok && verbose && c.failures <= 3) {
        std::printf("  %s: ", c.name.c_str());
        std::printf(format, args...);
        std::printf("\n");
    }
}

// True when a and b differ, but only in elements that are NaN on both sides.
bool nan_only_diff(const float* a, const float* b, int n) {
    bool differs = false;
    for (int k = 0; k < n; ++k) {
        if (std::memcmp(a + k, b + k, 4) == 0) continue;
        if (!(std::isnan(a[k]) && std::isnan(b[k]))) return false;
        differs = true;
    }
    return differs;
}

int first_diff(const float* a, const float* b, int n) {
    for (int k = 0; k < n; ++k) if (std::memcmp(a + k, b + k, 4) != 0) return k;
    return -1;
}

void unit3(float d[3]) {
    const float l = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    if (l > 0) for (int k = 0; k < 3; ++k) d[k] /= l;
}

// 00B75520 takes its outputs in EAX, ECX, EDX and [ESP+8] (s at [ESP+4]), RET 8.
void native_hermite(unsigned offset, float s, float* p1, float* p0, float* t1, float* t0) {
    void* fn = image + offset;
    __asm {
        push t0
        push s
        mov eax, p1
        mov ecx, p0
        mov edx, t1
        call fn
    }
}

// 00C37860 takes u, v, w in EAX, ECX, EDX and eight floats on the stack, RET 20h.
void native_barycentric(unsigned offset, float* u, float* v, float* w, const float* args) {
    void* fn = image + offset;
    __asm {
        mov esi, args
        push dword ptr [esi + 28]
        push dword ptr [esi + 24]
        push dword ptr [esi + 20]
        push dword ptr [esi + 16]
        push dword ptr [esi + 12]
        push dword ptr [esi + 8]
        push dword ptr [esi + 4]
        push dword ptr [esi]
        mov eax, u
        mov ecx, v
        mov edx, w
        call fn
    }
}

// The cc13-fixture2 routines. Runs once per precision control; the caller resets the generator
// so both passes see the same inputs.
template<class Open> void run_fixture2(Open open) {
    {   using namespace bsp::cc13fx;
        auto& c = open("00B64470 matrix from euler/scale/translation");
        auto native = at<void (__fastcall*)(float*, const float*, const float*, const float*)>(cc13_leaf_00b64470);
        for (int i = 0; i < 2000; ++i) {
            float t[3], e[3], s[3], o1[16], o2[16];
            for (int k = 0; k < 3; ++k) { t[k] = edge_or(-100, 100); e[k] = edge_or(-7, 7); s[k] = edge_or(-3, 3); }
            for (int k = 0; k < 16; ++k) o1[k] = o2[k] = static_cast<float>(k) + 0.25f;
            native(o1, t, e, s);
            compose_euler_scale_translation(o2, t, e, s);
            if (nan_only_diff(o1, o2, 16)) { c.check_nan_only(); continue; }
            const int k = first_diff(o1, o2, 16);
            check_note(c, k < 0, "e=%a,%a,%a s=%a,%a,%a [%d] %a / %a", e[0], e[1], e[2], s[0], s[1], s[2], k,
                       k < 0 ? 0.0 : o1[k], k < 0 ? 0.0 : o2[k]);
        }
        auto& f = open("00B65560 frustum planes");
        auto planes = at<void (__fastcall*)(float*, int, float, float, float, float)>(cc13_leaf_00b65560);
        for (int i = 0; i < 2000; ++i) {
            const float v = edge_or(-3.2f, 3.2f), h = edge_or(-3.2f, 3.2f), n = edge_or(0, 10), fa = edge_or(10, 5000);
            float o1[24], o2[24];
            for (int k = 0; k < 24; ++k) o1[k] = o2[k] = 9.0f;
            planes(o1, 0, v, h, n, fa);
            frustum_planes(o2, v, h, n, fa);
            const int k = first_diff(o1, o2, 24);
            check_note(f, k < 0, "v=%a h=%a n=%a f=%a [%d] %a / %a", v, h, n, fa, k, k < 0 ? 0.0 : o1[k], k < 0 ? 0.0 : o2[k]);
        }
        auto& sc = open("00B8C520 color32 scale");
        auto scale = at<void (__fastcall*)(const std::uint8_t*, int, std::uint8_t*, float)>(cc13_leaf_00b8c520);
        for (int i = 0; i < 4000; ++i) {
            std::uint8_t in[4], o1[4] = {1, 2, 3, 4}, o2[4] = {1, 2, 3, 4};
            for (auto& b : in) b = static_cast<std::uint8_t>(next());
            const float k = i < 256 ? static_cast<float>(i) / 128.0f : edge_or(-1, 3);
            scale(in, 0, o1, k);
            scale_color32(in, o2, k);
            sc.check(std::memcmp(o1, o2, 4) == 0);
        }
        auto& inv = open("00B62DD0 2x2 invert");
        auto invert = at<void (__fastcall*)(float*)>(cc13_leaf_00b62dd0);
        for (int i = 0; i < 2000; ++i) {
            float m1[4], m2[4];
            for (int k = 0; k < 4; ++k) m1[k] = m2[k] = edge_or(-10, 10);
            if (i % 40 == 0) { m1[0] = m2[0] = 2; m1[1] = m2[1] = 4; m1[2] = m2[2] = 1; m1[3] = m2[3] = 2; }  // det 0
            const float m0[4] = {m1[0], m1[1], m1[2], m1[3]};
            invert(m1);
            invert2x2(m2);
            if (nan_only_diff(m1, m2, 4)) { inv.check_nan_only(); continue; }
            const int k = first_diff(m1, m2, 4);
            check_note(inv, k < 0, "m=%a,%a,%a,%a [%d] %a / %a", m0[0], m0[1], m0[2], m0[3], k, k < 0 ? 0.0 : m1[k], k < 0 ? 0.0 : m2[k]);
        }
        auto& orth = open("00B62A10 ortho off-centre LH");
        auto ortho = at<void (__fastcall*)(float*, int, float, float, float, float, float, float)>(cc13_leaf_00b62a10);
        for (int i = 0; i < 2000; ++i) {
            const float l = edge_or(-500, 500), r = i % 50 ? edge_or(-500, 500) : l, b = edge_or(-500, 500),
                        tp = edge_or(-500, 500), zn = edge_or(0, 10), zf = edge_or(0, 1000);
            float o1[16], o2[16];
            for (int k = 0; k < 16; ++k) o1[k] = o2[k] = -3.0f;
            ortho(o1, 0, l, r, b, tp, zn, zf);
            ortho_off_center_lh(o2, l, r, b, tp, zn, zf);
            orth.check(std::memcmp(o1, o2, sizeof(o1)) == 0);
        }
    }
    {
        auto& c = open("00467050 rotation from euler");
        auto native = at<void (__fastcall*)(float*, int, float, float, float)>(cc13_leaf_00467050);
        for (int i = 0; i < 2000; ++i) {
            float m1[16], m2[16];
            for (int k = 0; k < 16; ++k) m1[k] = m2[k] = static_cast<float>(k) - 7.5f;
            const float a = edge_or(-7, 7), b = edge_or(-7, 7), d = edge_or(-7, 7);
            native(m1, 0, a, b, d);
            bsp::cc13_seg6::matrix_set_rotation_from_euler_00467050(m2, a, b, d);
            const int k = first_diff(m1, m2, 16);
            check_note(c, k < 0, "a=%a b=%a c=%a [%d] %a / %a", a, b, d, k, k < 0 ? 0.0 : m1[k], k < 0 ? 0.0 : m2[k]);
        }
    }
    {
        auto& c = open("004E5C00 linearize rgb (gamma 2.2)");
        auto native = at<void (__fastcall*)(float*)>(cc13_leaf_004e5c00);
        for (int i = 0; i < 3000; ++i) {
            float a[3], b[3];
            for (int k = 0; k < 3; ++k) a[k] = b[k] = edge_or(-2, 2);
            const float in[3] = {a[0], a[1], a[2]};
            native(a);
            bsp::scene_weather_cc13::linearize_rgb_004e5c00(b);
            const int k = first_diff(a, b, 3);
            check_note(c, k < 0, "in=%a [%d] %a / %a", k < 0 ? 0.0 : in[k], k, k < 0 ? 0.0 : a[k], k < 0 ? 0.0 : b[k]);
        }
    }
    {   using namespace bsp::cc13_sys;
        auto& c = open("00452B00 lens flare pow response");
        auto native = at<void (__fastcall*)(void*, int, float)>(cc13_leaf_00452b00);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char flare[0x20]{};
            const float k = edge_or(-4, 8), x = edge_or(-3, 3);
            std::memcpy(flare + 0x1c, &k, 4);
            native(flare, 0, x);
            float o4 = 0, o8 = 0;
            lens_flare_pow_response_00452b00(k, x, o4, o8);
            float n4 = 0;
            std::memcpy(&n4, flare + 4, 4);
            check_note(c, std::memcmp(flare + 4, &o4, 4) == 0 && std::memcmp(flare + 8, &o8, 4) == 0, "k=%a x=%a: %a / %a", k, x, n4, o4);
        }
        auto ar = [] { return std::array<float, 3>{uniform(-50, 50), uniform(-50, 50), uniform(-50, 50)}; };
        auto& pp = open("0046F7B0 project point onto plane");
        auto& cl = open("0046F550 closest point on line");
        auto& da = open("0046F6D0 distance along axis");
        using Pt = float* (__fastcall*)(float*, const float*, const float*, const float*, float*);
        auto project = at<Pt>(cc13_leaf_0046f7b0);
        auto closest = at<Pt>(cc13_leaf_0046f550);
        auto along = at<float (__fastcall*)(const float*, const float*, const float*)>(cc13_leaf_0046f6d0);
        for (int i = 0; i < 2000; ++i) {
            auto p = ar(), q = ar(), d = ar();
            unit3(d.data());
            if (i % 50 == 0) p = q;
            if (i % 70 == 1) p[0] = edge_or(-50, 50);
            float o1[3] = {5, 5, 5}, dist1 = -1, dist2 = -1;
            project(o1, p.data(), q.data(), d.data(), &dist1);
            auto o2 = project_onto_plane_0046f7b0(p, q, d, dist2);
            pp.check(std::memcmp(o1, o2.data(), 12) == 0 && same(dist1, dist2));
            closest(o1, p.data(), q.data(), d.data(), &dist1);
            o2 = closest_point_on_line_0046f550(p, q, d, dist2);
            cl.check(std::memcmp(o1, o2.data(), 12) == 0 && same(dist1, dist2));
            da.check(same(along(p.data(), q.data(), d.data()), distance_along_axis_0046f6d0(p, q, d)));
        }
    }
    {   using namespace bsp::cc13_terrain;
        auto& q = open("00AED820 float layer quantize8");
        auto quantize = at<void (__fastcall*)(const void*, int, std::uint8_t*, std::int32_t, std::int32_t)>(cc13_leaf_00aed820);
        float cells[64];
        for (auto& v : cells) v = edge_or(-0.5f, 1.5f);
        alignas(4) unsigned char block[0x40]{};
        const std::int32_t row_stride = 8;
        const float* data = cells;
        std::memcpy(block + 0x2c, &row_stride, 4);
        std::memcpy(block + 0x34, &data, 4);
        for (int i = 0; i < 1000; ++i) {
            const std::int32_t x = static_cast<std::int32_t>(next() % 8), y = static_cast<std::int32_t>(next() % 8);
            if (i % 64 == 0) for (auto& v : cells) v = edge_or(-0.5f, 1.5f);
            std::uint8_t a = 7, b = 7;
            quantize(block, 0, &a, x, y);
            terrain_float_layer_quantize8_00aed820(block, &b, x, y);
            q.check(a == b);
        }
        auto& hi = open("00AFB7F0 ballistic max height");
        auto& lo = open("00AFBA40 ballistic min height");
        auto max_n = at<float (__fastcall*)(const float*, int, const float*)>(cc13_leaf_00afb7f0);
        auto min_n = at<float (__fastcall*)(const float*, int, const float*)>(cc13_leaf_00afba40);
        for (int i = 0; i < 3000; ++i) {
            float curve[11], other[2];
            for (auto& v : curve) v = edge_or(-20, 20);
            curve[0] = edge_or(0, 5);
            other[0] = edge_or(0, 10); other[1] = edge_or(-20, 20);
            if (i % 30 == 0) curve[7] = 0;
            hi.check(same(max_n(curve, 0, other), ballistic_max_height_00afb7f0(curve, other)));
            lo.check(same(min_n(curve, 0, other), ballistic_min_height_00afba40(curve, other)));
        }
        auto& b8 = open("00ADFA90 terrain u8 block set");
        auto& b16 = open("00ADC580 terrain u16 block set");
        auto set8 = at<void (__fastcall*)(void*, int, float, std::int32_t, std::int32_t)>(cc13_leaf_00adfa90);
        auto set16 = at<void (__fastcall*)(void*, int, float, std::int32_t, std::int32_t)>(cc13_leaf_00adc580);
        for (int i = 0; i < 2000; ++i) {
            std::uint16_t d1[16]{}, d2[16]{};
            alignas(4) unsigned char k1[0x44]{}, k2[0x44]{};
            const float offset = edge_or(-1200, 200), mult = edge_or(0, 2);
            const std::int32_t stride = 4;
            for (auto* k : {k1, k2}) {
                std::memcpy(k + 0x2c, &offset, 4); std::memcpy(k + 0x30, &mult, 4); std::memcpy(k + 0x38, &stride, 4);
            }
            void* p1 = d1; void* p2 = d2;
            std::memcpy(k1 + 0x40, &p1, 4); std::memcpy(k2 + 0x40, &p2, 4);
            const float h = i % 25 == 0 ? -1000.0f : edge_or(-1100, 500);
            const std::int32_t x = static_cast<std::int32_t>(next() % 4), y = static_cast<std::int32_t>(next() % 4);
            set8(k1, 0, h, x, y);
            bsp::cc13_terrain::terrain_u8_block_set_00adfa90(k2, h, x, y);
            b8.check(std::memcmp(d1, d2, sizeof(d1)) == 0);
            set16(k1, 0, h, x, y);
            bsp::cc13_terrain::terrain_u16_block_set_00adc580(k2, h, x, y);
            b16.check(std::memcmp(d1, d2, sizeof(d1)) == 0);
        }
    }
    {
        auto& c = open("009064D0 map usage points");
        auto native = at<std::int32_t (__fastcall*)(void*, int, float)>(cc13_leaf_009064d0);
        alignas(4) static unsigned char scoring[0x1470]{};
        for (int i = 0; i < 3000; ++i) {
            const float rate = edge_or(0, 50), seconds = i < 200 ? static_cast<float>(i) * 0.25f : edge_or(-100, 4000);
            std::memcpy(scoring + 0x1468, &rate, 4);
            c.check(native(scoring, 0, seconds) == bsp::cc13_seg59a::scoring_map_usage_points_009064d0(scoring, seconds));
        }
        auto& t = open("00B8B940 truncate float to +20h");
        auto trunc = at<void (__stdcall*)(void*, float)>(cc13_leaf_00b8b940);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char object[0x24]{};
            const float v = i < 100 ? static_cast<float>(i - 50) * 0.5f : edge_or(-3e9f, 3e9f);
            trunc(object, v);
            std::int32_t field = 0, native_field = 0;
            bsp::cc13_scene2::truncate_float_to_field_20_00b8b940(field, v);
            std::memcpy(&native_field, object + 0x20, 4);
            t.check(field == native_field);
        }
    }
    {
        auto v3 = [](float lo, float hi, float* o) { for (int k = 0; k < 3; ++k) o[k] = edge_or(lo, hi); };
        auto& ss = open("00428090 segment / sphere");
        auto seg = at<bool (__fastcall*)(const float*, const float*, float, const float*)>(cc13_leaf_00428090);
        for (int i = 0; i < 3000; ++i) {
            float centre[3], start[3], end[3];
            v3(-20, 20, centre); v3(-30, 30, start); v3(-30, 30, end);
            if (i % 40 == 0) std::memcpy(end, start, 12);
            const float r = edge_or(0, 15);
            const bool n1 = seg(centre, start, r, end), n2 = bsp::cc13_left61::segment_intersects_sphere_00428090(centre, start, r, end);
            check_note(ss, n1 == n2, "c=%a,%a,%a s=%a,%a,%a e=%a,%a,%a r=%a: %d / %d", centre[0], centre[1], centre[2], start[0], start[1],
                       start[2], end[0], end[1], end[2], r, n1, n2);
        }
        auto& rs = open("00941AA0 spawn ray / sphere");
        auto ray = at<bool (__fastcall*)(const float*, const float*, const float*, float, float*, float*)>(cc13_leaf_00941aa0);
        for (int i = 0; i < 3000; ++i) {
            float start[3], dir[3], centre[3];
            v3(-30, 30, start); v3(-1, 1, dir); v3(-30, 30, centre);
            if (i % 3) unit3(dir);
            const float r = edge_or(0, 20);
            float a0 = 3, a1 = 4, b0 = 3, b1 = 4;
            const bool r1 = ray(start, dir, centre, r, &a0, &a1);
            const bool r2 = bsp::spawn_ray_sphere_00941aa0(start, dir, centre, r, b0, b1);
            const float ra[2] = {a0, a1}, rb[2] = {b0, b1};
            if (r1 == r2 && nan_only_diff(ra, rb, 2)) { rs.check_nan_only(); continue; }
            check_note(rs, r1 == r2 && same(a0, b0) && same(a1, b1), "s=%a,%a,%a d=%a,%a,%a c=%a,%a,%a r=%a: %d %a %a / %d %a %a",
                       start[0], start[1], start[2], dir[0], dir[1], dir[2], centre[0], centre[1], centre[2], r, r1, a0, a1, r2, b0, b1);
        }
        auto& r1c = open("009D7780 oriented rect distance");
        auto& r2c = open("009D7930 oriented rect 2 distance");
        using RectFn = float (__fastcall*)(const void*, int, const float*);
        auto rect1 = at<RectFn>(cc13_leaf_009d7780);
        auto rect2 = at<RectFn>(cc13_leaf_009d7930);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char rect[0x80]{};
            for (int k = 0; k < 0x80; k += 4) { const float f = edge_or(-40, 40); std::memcpy(rect + k, &f, 4); }
            for (const int axis : {0x28, 0x30, 0x4c, 0x54}) {   // unit axes as the records hold them
                float ax[2] = {uniform(-1, 1), uniform(-1, 1)};
                const float l = std::sqrt(ax[0] * ax[0] + ax[1] * ax[1]);
                if (i % 5 && l > 0) { ax[0] /= l; ax[1] /= l; }
                std::memcpy(rect + axis, ax, 8);
            }
            rect[0x68] = static_cast<unsigned char>(i & 1);
            const float point[2] = {edge_or(-60, 60), edge_or(-60, 60)};
            r1c.check(same(rect1(rect, 0, point), bsp::oriented_rect_distance_to_point_009d7780(rect, point)));
            r2c.check(same(rect2(rect, 0, point), bsp::oriented_rect2_distance_to_point_009d7930(rect, point)));
        }
        auto& s3 = open("00793730 vec3 scale to length");
        auto& s2 = open("00793800 vec2 scale to length");
        auto scale3 = at<float* (__fastcall*)(float*, const float*, float)>(cc13_leaf_00793730);
        auto scale2 = at<float* (__fastcall*)(float*, const float*, float)>(cc13_leaf_00793800);
        for (int i = 0; i < 3000; ++i) {
            std::array<float, 3> v{};
            v3(-30, 30, v.data());
            if (i % 40 == 0) v = {0, 0, 0};
            const float len = edge_or(-10, 10);
            float o[3] = {7, 7, 7};
            scale3(o, v.data(), len);
            const auto r3 = bsp::cc13cam::scale_to_length_00793730(v, len);
            s3.check(std::memcmp(o, r3.data(), 12) == 0);
            const std::array<float, 2> w{v[0], v[1]};
            float o2[2] = {7, 7};
            scale2(o2, w.data(), len);
            const auto r2 = bsp::cc13cam::scale_to_length_00793800(w, len);
            s2.check(std::memcmp(o2, r2.data(), 8) == 0);
        }
        auto& tv = open("007BB4C0 triangle vertex distance sum");
        auto sum = at<float (__fastcall*)(const float*, const float*)>(cc13_leaf_007bb4c0);
        for (int i = 0; i < 3000; ++i) {
            float a[12], b[12];
            for (int k = 0; k < 12; ++k) { a[k] = edge_or(-50, 50); b[k] = i % 30 == 0 ? a[k] : edge_or(-50, 50); }
            tv.check(same(sum(a, b), bsp::cc13_seg47::triangle_vertex_distance_sum_007bb4c0(a, b)));
        }
    }
    {
        auto& c = open("00A8B860 2D line intersection");
        auto native = at<void (__fastcall*)(float*, const float*, const float*, const float*, const float*)>(cc13_leaf_00a8b860);
        for (int i = 0; i < 3000; ++i) {
            float p0[2], p1[2], q0[2], q1[2];
            for (auto* p : {p0, p1, q0, q1}) { p[0] = edge_or(-100, 100); p[1] = edge_or(-100, 100); }
            if (i % 40 == 0) { q0[0] = p0[0]; q0[1] = p0[1]; q1[0] = p1[0]; q1[1] = p1[1]; }  // parallel
            float o1[4] = {1, 2, 3, 4}, o2[4] = {1, 2, 3, 4};
            native(o1, p0, p1, q0, q1);
            bsp::cc13_audio::intersect_lines_2d_00a8b860(o2, p0, p1, q0, q1);
            if (nan_only_diff(o1, o2, 4)) { c.check_nan_only(); continue; }
            const int k = first_diff(o1, o2, 4);
            check_note(c, k < 0, "p0=%a,%a p1=%a,%a q0=%a,%a q1=%a,%a [%d] %a / %a", p0[0], p0[1], p1[0], p1[1], q0[0], q0[1], q1[0], q1[1], k,
                       k < 0 ? 0.0 : o1[k], k < 0 ? 0.0 : o2[k]);
        }
    }
    {   using namespace bsp::cc13_scene;
        auto& bz = open("00B75580 bezier");
        auto bezier = at<double (__stdcall*)(float, float, float, float, float)>(cc13_leaf_00b75580);
        auto& hm = open("00B75520 hermite basis");
        for (int i = 0; i < 3000; ++i) {
            const float p0 = edge_or(-20, 20), p1 = edge_or(-20, 20), p2 = edge_or(-20, 20), p3 = edge_or(-20, 20),
                        s = i < 101 ? static_cast<float>(i) / 100.0f : edge_or(-0.5f, 1.5f);
            const double n1 = bezier(p0, p1, p2, p3, s), n2 = bezier_00b75580(p0, p1, p2, p3, s);
            check_note(bz, same(n1, n2), "p=%a,%a,%a,%a s=%a: %a / %a", p0, p1, p2, p3, s, n1, n2);
            HermiteBasis h1{};
            native_hermite(cc13_leaf_00b75520, s, &h1.p1, &h1.p0, &h1.t1, &h1.t0);
            const HermiteBasis h2 = hermite_basis_00b75520(s);
            hm.check(std::memcmp(&h1, &h2, sizeof(h1)) == 0);
        }
    }
    {
        auto& c = open("00BBF280 rgba lerp");
        using bsp::weather::Rgba;
        auto native = at<void (__fastcall*)(Rgba*, const Rgba*, const Rgba*, const float*)>(cc13_leaf_00bbf280);
        for (int i = 0; i < 3000; ++i) {
            const Rgba a{edge_or(0, 1), edge_or(0, 1), edge_or(0, 1), edge_or(0, 1)};
            const Rgba b{edge_or(0, 1), edge_or(0, 1), edge_or(0, 1), edge_or(0, 1)};
            const float t = edge_or(-0.5f, 1.5f);
            Rgba o{9, 9, 9, 9};
            native(&o, &a, &b, &t);
            const Rgba r = bsp::weather::sky_lerp_00bbf280(a, b, t);
            c.check(same(o, r));
        }
    }
    {
        auto& c = open("004F3DA0 2D triangle contains point");
        auto native = at<bool (__fastcall*)(const float*, const float*)>(cc13_leaf_004f3da0);
        for (int i = 0; i < 4000; ++i) {
            float tri[6], p[2];
            for (auto& v : tri) v = edge_or(-10, 10);
            p[0] = edge_or(-10, 10); p[1] = edge_or(-10, 10);
            if (i % 20 == 0) { p[0] = tri[0]; p[1] = tri[1]; }                                   // a vertex
            if (i % 20 == 1) { p[0] = (tri[0] + tri[2]) * 0.5f; p[1] = (tri[1] + tri[3]) * 0.5f; }  // an edge
            const bool n1 = native(p, tri), n2 = bsp::cc13_sweepA1b::triangle2f_contains_point_004f3da0(p, tri);
            check_note(c, n1 == n2, "p=%a,%a t=%a,%a %a,%a %a,%a: %d / %d", p[0], p[1], tri[0], tri[1], tri[2], tri[3], tri[4], tri[5], n1, n2);
        }
    }
    {
        auto& c = open("00C37860 2D barycentric");
        for (int i = 0; i < 3000; ++i) {
            float a[8];
            for (auto& v : a) v = edge_or(-10, 10);
            if (i % 40 == 0) { a[4] = a[0]; a[5] = a[1]; }  // degenerate
            float u1 = 5, v1 = 5, w1 = 5, u2 = 5, v2 = 5, w2 = 5;
            native_barycentric(cc13_leaf_00c37860, &u1, &v1, &w1, a);
            bsp::cc13_dyn1::barycentric_2d_00c37860(&u2, &v2, &w2, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
            c.check(same(u1, u2) && same(v1, v2) && same(w1, w2));
        }
    }
    {
        auto& c = open("00B478E0 vertex element type size");
        auto native = at<std::uint32_t (__fastcall*)(const std::uint32_t*)>(cc13_leaf_00b478e0);
        for (std::uint32_t type = 0; type < 18; ++type) {
            const std::uint32_t element[5] = {0, type, 0, 0, 0};
            c.check(native(element) == bsp::cc13_render1::vertex_element_type_size_00b478e0(element));
        }
    }
}

}  // namespace

int main() {
    unsigned control{};
    _controlfp_s(&control, _PC_53 | _RC_NEAR, _MCW_PC | _MCW_RC);
    load_image();
    static Case cases[200]{};
    int count = 0;
    auto open = [&](const char* name) -> Case& { cases[count].name = name; return cases[count++]; };

    {   // 00BB9F40 table fill, then 00BBA030 over that table.
        using namespace bsp::band0090;
        auto& c = open("00BB9F40 CRC-32 table");
        auto* table = reinterpret_cast<std::uint32_t*>(image + cc13_leaf_01090500);
        at<void (__fastcall*)(std::uint32_t*)>(cc13_leaf_00bb9f40)(table);
        const auto rebuilt = build_crc32_table();
        c.check(std::memcmp(table, rebuilt.data(), 1024) == 0);
        auto& u = open("00BBA030 CRC-32 update");
        auto native = at<void (__fastcall*)(std::uint32_t*, int, const std::uint8_t*, std::uint32_t)>(cc13_leaf_00bba030);
        std::uint8_t bytes[300];
        for (auto& b : bytes) b = static_cast<std::uint8_t>(next());
        for (std::uint32_t length = 0; length <= 300; length += 23) {
            std::uint32_t a = 0xffffffffu ^ length, b = a;
            native(&a, 0, bytes, length);
            crc32_update(b, bytes, length);
            u.check(a == b);
        }
    }
    {
        using namespace bsp::cc13_band0080;
        auto& c = open("008EF6A0 hex digit");
        auto digit = at<int (__fastcall*)(char)>(cc13_leaf_008ef6a0);
        for (int i = 0; i < 256; ++i)
            c.check(digit(static_cast<char>(i)) == hex_digit_value_008ef6a0(static_cast<char>(i)));
        auto& t = open("008EF6D0 BIN token byte");
        auto decode = at<std::uint8_t (__fastcall*)(const char*)>(cc13_leaf_008ef6d0);
        const char alphabet[] = "09afAFgG/:@`";
        t.check(decode(nullptr) == decode_hex_byte_token_008ef6d0(nullptr));
        for (const char x : alphabet) for (const char y : alphabet) {
            const char token[] = {x, y, 'f', 0};
            t.check(decode(token) == decode_hex_byte_token_008ef6d0(token));
        }
        auto& e = open("008EE5F0 scene text escape");
        auto escape = at<const char* (__fastcall*)(const char*, int)>(cc13_leaf_008ee5f0);
        e.check(std::string(escape(nullptr, 5)) == escape_scene_text_008ee5f0(nullptr, 5));
        const char pieces[] = "a\n\"\\ xQ";
        for (int i = 0; i < 200; ++i) {
            std::string text;
            for (int k = next() % 40; k > 0; --k) text += pieces[next() % 7];
            const int limit = static_cast<int>(next() % 45);
            e.check(std::string(escape(text.c_str(), limit)) == escape_scene_text_008ee5f0(text.c_str(), limit));
        }
        auto& s = open("0085D020 segment / triangle");
        auto hit = at<bool (__fastcall*)(const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*,
                                         const Vec3f*, Vec3f*)>(cc13_leaf_0085d020);
        for (int i = 0; i < 2000; ++i) {
            auto v = [](float lo, float hi) { return Vec3f{uniform(lo, hi), uniform(lo, hi), uniform(lo, hi)}; };
            Vec3f a = v(-5, 5), b = v(-5, 5), c3 = v(-5, 5);
            Vec3f p0 = v(-6, 6), p1 = v(-6, 6);
            if (i & 1) { p0.y = 8; p1.y = -8; a.y *= 0.1f; b.y *= 0.1f; c3.y *= 0.1f; }
            Vec3f o1{-1, -2, -3}, o2 = o1;
            const bool r1 = hit(&p0, &p1, &a, &b, &c3, &o1);
            const bool r2 = segment_intersect_triangle_0085d020(p0, p1, a, b, c3, o2);
            s.check(r1 == r2 && same(o1, o2));
            if (!(r1 == r2 && same(o1, o2)) && s.failures < 4) std::printf("seg i=%d: %d %a,%a,%a / %d %a,%a,%a\n", i, r1,o1.x,o1.y,o1.z,r2,o2.x,o2.y,o2.z);
        }
    }
    {   // 00809E10: launch speed through the record's +3Ch object, vt slot 38h.
        auto& c = open("00809E10 rocket flight time");
        static float launch = 0;
        struct Launcher { static float __fastcall speed(void*, void*) { return launch; } };
        void* vtable[16]{};
        vtable[0x38 / 4] = reinterpret_cast<void*>(&Launcher::speed);
        void* object[1] = {vtable};
        alignas(4) unsigned char rocket[0xe0]{}, record[0x320]{};
        void* object_pointer = object; void* rocket_pointer = rocket;
        std::memcpy(record + 0x3c, &object_pointer, 4);
        std::memcpy(record + 0x314, &rocket_pointer, 4);
        auto native = at<float (__fastcall*)(void*, int, float)>(cc13_leaf_00809e10);
        for (int i = 0; i < 2000; ++i) {
            launch = uniform(0, 300);
            const float vmax = uniform(launch - 60, 400), accel = uniform(1, 120), range = uniform(0, 6000);
            std::memcpy(rocket + 0xd8, &vmax, 4);
            std::memcpy(rocket + 0xdc, &accel, 4);
            const float a = native(record, 0, range);
            const float b = bsp::rocket_flight_time_00809e10(range, launch, vmax, accel);
            c.check(same(a, b));
        }
    }
    {
        using namespace bsp::band_0045_cc13;
        auto v = [] { return Vec3{uniform(-50, 50), uniform(-50, 50), uniform(-50, 50)}; };
        auto& mn = open("00481C90 component min");
        auto& mx = open("00481D40 component max");
        auto min_n = at<Vec3* (__fastcall*)(Vec3*, const Vec3*, const Vec3*)>(cc13_leaf_00481c90);
        auto max_n = at<Vec3* (__fastcall*)(Vec3*, const Vec3*, const Vec3*)>(cc13_leaf_00481d40);
        for (int i = 0; i < 500; ++i) {
            Vec3 a = v(), b = v(), out{};
            if (i % 5 == 0) b.y = a.y;
            min_n(&out, &a, &b);
            mn.check(same(out, component_min_00481c90(a, b)));
            max_n(&out, &a, &b);
            mx.check(same(out, component_max_00481d40(a, b)));
        }
        auto& pl = open("0046F440 point / line distance");
        auto distance = at<float (__fastcall*)(const Vec3*, const Vec3*, const Vec3*)>(cc13_leaf_0046f440);
        for (int i = 0; i < 2000; ++i) {
            Vec3 p = v(), q = v(), d = v();
            const float length = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
            d = {d.x / length, d.y / length, d.z / length};
            if (i % 50 == 0) p = q;
            pl.check(same(distance(&p, &q, &d), point_line_distance_0046f440(p, q, d)));
        }
        auto& rp = open("0046FC80 ray / plane");
        auto ray = at<bool (__fastcall*)(const Vec3*, const Vec3*, const Vec3*, const Vec3*, Vec3*)>(cc13_leaf_0046fc80);
        for (int i = 0; i < 2000; ++i) {
            Vec3 o = v(), d = v(), pp = v(), pn = v();
            if (i % 50 == 0) { d = {1, 0, 0}; pn = {0, 1, 0}; }
            Vec3 o1{7, 7, 7}, o2 = o1;
            const bool r1 = ray(&o, &d, &pp, &pn, &o1);
            const bool r2 = ray_plane_intersect_0046fc80(o, d, pp, pn, o2);
            rp.check(r1 == r2 && same(o1, o2));
            if (!(r1 == r2 && same(o1, o2)) && rp.failures < 3) std::printf("ray o=%a,%a,%a d=%a,%a,%a pp=%a,%a,%a pn=%a,%a,%a: %d %a,%a,%a / %d %a,%a,%a\n", o.x,o.y,o.z,d.x,d.y,d.z,pp.x,pp.y,pp.z,pn.x,pn.y,pn.z,r1,o1.x,o1.y,o1.z,r2,o2.x,o2.y,o2.z);
        }
    }
    {
        auto& c = open("00439BD0 quadratic roots");
        auto solve = at<float (__fastcall*)(float*, float*, float, float, float)>(cc13_leaf_00439bd0);
        for (int i = 0; i < 2000; ++i) {
            float a = uniform(-20, 20), b = uniform(-60, 60), q = uniform(-20, 20);
            if (a == 0) a = 1;
            float n1 = 5, n2 = 6, r1 = 5, r2 = 6;
            const float d1 = solve(&n1, &n2, a, b, q);
            const float d2 = bsp::cc13_band0040::solve_quadratic_00439bd0(a, b, q, r1, r2);
            c.check(same(d1, d2) && same(n1, r1) && same(n2, r2));
        }
    }
    {
        using namespace bsp::band_0067_cc13;
        auto v = [] { return Vec3{uniform(-80, 80), uniform(-80, 80), uniform(-80, 80)}; };
        auto& pf = open("0073E780 plane from normal and point");
        auto& pd = open("0073E7C0 plane signed distance");
        auto& va = open("00709A60 vec3 add");
        auto& vs = open("00709BB0 vec3 add scaled");
        auto from = at<void (__fastcall*)(Plane*, int, const Vec3*, const Vec3*)>(cc13_leaf_0073e780);
        auto signed_distance = at<float (__fastcall*)(const Plane*, int, const Vec3*)>(cc13_leaf_0073e7c0);
        auto add = at<Vec3* (__fastcall*)(Vec3*, const Vec3*, const Vec3*)>(cc13_leaf_00709a60);
        auto add_scaled = at<Vec3* (__fastcall*)(Vec3*, const Vec3*, const Vec3*, float)>(cc13_leaf_00709bb0);
        for (int i = 0; i < 2000; ++i) {
            const Vec3 point = v(), normal = v(), other = v();
            Plane p1{}, p2{};
            from(&p1, 0, &point, &normal);
            plane_from_normal_and_point_0073e780(p2, point, normal);
            pf.check(same(p1, p2));
            pd.check(same(signed_distance(&p1, 0, &other), plane_signed_distance_0073e7c0(p1, other)));
            Vec3 out{};
            add(&out, &point, &other);
            va.check(same(out, vec3_add_00709a60(point, other)));
            const float s = uniform(-3, 3);
            add_scaled(&out, &point, &other, s);
            vs.check(same(out, vec3_add_scaled_00709bb0(point, other, s)));
        }
    }
    {
        using namespace bsp::cc13_band0050;
        auto& c = open("00569E40 linear remap");
        auto remap = at<float (__fastcall*)(const float*, const float*, const float*, const float*, const float*)>(cc13_leaf_00569e40);
        for (int i = 0; i < 2000; ++i) {
            const float x0 = uniform(-100, 100), x1 = uniform(-100, 100), x = uniform(-150, 150),
                        y0 = uniform(-10, 10), y1 = uniform(-10, 10);
            c.check(same(remap(&x0, &x1, &x, &y0, &y1), remap_linear_00569e40(x0, x1, x, y0, y1)));
        }
        auto& mx = open("0059A840 world to map x");
        auto& my = open("0059A860 world to map y");
        auto to_x = at<float (__fastcall*)(const void*, int, float)>(cc13_leaf_0059a840);
        auto to_y = at<float (__fastcall*)(const void*, int, float)>(cc13_leaf_0059a860);
        alignas(4) unsigned char map[0x554]{};
        for (int i = 0; i < 2000; ++i) {
            HudMapProjection m;
            m.scale_x_30 = uniform(0.5f, 40); m.scale_y_34 = uniform(0.5f, 40);
            m.origin_x_54c = uniform(-30000, 30000); m.origin_y_550 = uniform(-30000, 30000);
            m.offset_x_e08890 = uniform(0, 1024); m.offset_y_e08894 = uniform(0, 768);
            std::memcpy(map + 0x30, &m.scale_x_30, 4); std::memcpy(map + 0x34, &m.scale_y_34, 4);
            std::memcpy(map + 0x54c, &m.origin_x_54c, 4); std::memcpy(map + 0x550, &m.origin_y_550, 4);
            std::memcpy(image + cc13_leaf_00e08890, &m.offset_x_e08890, 4);
            std::memcpy(image + cc13_leaf_00e08894, &m.offset_y_e08894, 4);
            const float x = uniform(-40000, 40000), y = uniform(-40000, 40000);
            mx.check(same(to_x(map, 0, x), world_to_map_x_0059a840(m, x)));
            my.check(same(to_y(map, 0, y), world_to_map_y_0059a860(m, y)));
        }
    }
    {
        using namespace bsp::band0076_cc13;
        auto& ic = open("0078E2B0 angle windows contain");
        auto& cl = open("0078FEB0 angle windows clamp");
        auto contain = at<bool (__fastcall*)(const AngleWindows*, int, float, float)>(cc13_leaf_0078e2b0);
        auto clamp = at<void (__fastcall*)(const AngleWindows*, int, float*, float*)>(cc13_leaf_0078feb0);
        for (int i = 0; i < 2000; ++i) {
            const AngleWindows w{uniform(-3.2f, 3.2f), uniform(-3.2f, 3.2f), uniform(-3.2f, 3.2f), uniform(-3.2f, 3.2f)};
            const float a = uniform(-6.5f, 6.5f), b = uniform(-6.5f, 6.5f);
            ic.check(contain(&w, 0, a, b) == movcam_angle_windows_contain_0078e2b0(w, a, b));
            float na = a, nb = b, ra = a, rb = b;
            clamp(&w, 0, &na, &nb);
            movcam_angle_windows_clamp_0078feb0(w, ra, rb);
            cl.check(same(na, ra) && same(nb, rb));
        }
    }
    {   // cc13-trivialslots: one trivial vtable slot per kind against its original bytes.
        using namespace bsp::ingame_gui_cc13;
        std::uint8_t object[0x100]{};
        auto& ri = open("00626710 ReturnInt");
        ri.check(at<std::int32_t (__fastcall*)(void*)>(cc13_leaf_00626710)(object) ==
                 rewards_screen_get_screen_id_00626710(Raw{object}));
        auto& rb = open("00636DD0 ReturnByte");
        rb.check(at<std::uint8_t (__fastcall*)(void*)>(cc13_leaf_00636dd0)(object) ==
                 (hud_scene_init_screen_occludes_lower_levels_00636dd0(Raw{object}) ? 1 : 0));
        auto& em = open("0067AF30 Empty RET 8");
        // Smoke check only: both run and return, and the object is untouched. The RET 8
        // itself rests on the disk bytes (C2 08 00) and the audit hash, not on this case.
        auto empty = at<void (__fastcall*)(void*, int, std::uint32_t, std::uint32_t)>(cc13_leaf_0067af30);
        empty(object, 0, 1, 2);
        hud_tactical_sheet_screen_on_listbox_selection_changed_no_op_0067af30(Raw{object});
        bool untouched = true;
        for (const std::uint8_t b : object) untouched = untouched && b == 0;
        em.check(untouched);
        auto& tj = open("0067B3F0 TailJump");
        RecordingHudServices hud;
        void* native = at<void* (__fastcall*)(void*)>(cc13_leaf_0067b3f0)(object);
        hud_common_overlay_screen_register_0067b3f0(Raw{object}, hud);
        tj.check(native == object && hud.seen == native);
        auto& ad = open("00509D70 Adjustor ADD ECX,64h");
        RecordingFe3Host fe3;
        void* adjusted = at<void* (__fastcall*)(void*)>(cc13_leaf_00509d70)(object);
        bsp::cc13_fe3::tactical_library_widget_listener_current1c_00509d70(Raw{object}, fe3);
        ad.check(adjusted == object + 0x64 && fe3.seen == adjusted);
    }
    // cc13-fixture2: each routine at 53-bit (the CRT default, as above) and at 24-bit precision
    // control (the game's main thread after device creation), on the same inputs.
    for (const unsigned pc : {static_cast<unsigned>(_PC_53), static_cast<unsigned>(_PC_24)}) {
        _controlfp_s(&control, pc | _RC_NEAR, _MCW_PC | _MCW_RC);
        state = 0x2468ace1u;
        const std::string tag = pc == _PC_53 ? " @53" : " @24";
        run_fixture2([&](const char* name) -> Case& { return open((std::string(name) + tag).c_str()); });
    }
    _controlfp_s(&control, _PC_53 | _RC_NEAR, _MCW_PC | _MCW_RC);
    // cc13-fixture2 cases carry a precision suffix. A routine's reconstruction models one
    // precision control: 24-bit for the three listed here, 53-bit otherwise; only that pass
    // decides. Known findings (documented in docs/CC13_FIXTURE_PASS_2.md) are reported and
    // do not fail the run; NaN sign/payload-only differences are counted apart.
    static const char* const primary24[] = {"009D7780", "009D7930", "004F3DA0"};
    static const char* const known[][2] = {
        {"004F3DA0", "float intermediates overflow where the x87 stack does not"},
    };
    int failures = 0;
    for (int i = 0; i < count; ++i) {
        const std::string& name = cases[i].name;
        const bool at24 = name.size() > 4 && name.compare(name.size() - 4, 4, " @24") == 0;
        const bool tagged = at24 || (name.size() > 4 && name.compare(name.size() - 4, 4, " @53") == 0);
        bool wants24 = false;
        for (const char* p : primary24) wants24 = wants24 || name.compare(0, 8, p) == 0;
        const bool primary = !tagged || at24 == wants24;
        const char* finding = nullptr;
        for (const auto& k : known) if (name.compare(0, 8, k[0]) == 0) finding = k[1];
        std::cout << name << ": " << cases[i].runs - cases[i].failures - cases[i].nan_only << '/' << cases[i].runs;
        if (cases[i].nan_only) std::cout << " (+" << cases[i].nan_only << " NaN sign/payload only)";
        if (!cases[i].failures) std::cout << " match";
        else if (!primary) std::cout << " mismatch (other precision, informational)";
        else if (finding) std::cout << " MISMATCH (known finding: " << finding << ')';
        else std::cout << " MISMATCH";
        std::cout << (primary && tagged ? " [primary]\n" : "\n");
        failures += cases[i].failures != 0 && primary && !finding;
    }
    std::cout << count << " cc13 leaf cases; " << failures << " with mismatches\n";
    return failures ? 1 : 0;
}
