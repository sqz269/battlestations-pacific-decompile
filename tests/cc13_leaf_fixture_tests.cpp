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
#include "bsp/game_dynamics_list.hpp"
// cc13-fixture5
#include "bsp/cc13_seg95.hpp"
#include "bsp/cc13_seg68.hpp"
#include "bsp/dummy_depth_seg34_argon.hpp"
#include "bsp/engine_seg43_45_cc13.hpp"
#include "bsp/hud_movie_camera.hpp"
#include "bsp/ship_ai_navigation.hpp"
#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/cc13_seg33a.hpp"
#pragma warning(push)
#pragma warning(disable : 4244)  // kPlaneEffectHandles builds pair<uint16_t, string_view> from int literals
#include "bsp/frontend_seg16_24_28_cc13.hpp"
#pragma warning(pop)
#include "bsp/frontend_screens_cc13.hpp"
#include "bsp/cc13_sweepA.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/multiplayer_session_cc13.hpp"
#include "bsp/unit_order_record.hpp"
#include "bsp/director_geommesh_seg38_39_argon.hpp"
#include "bsp/cc13_left55.hpp"
#include "bsp/cc13_seg33a.hpp"
#include "bsp/cc13_seg62.hpp"
#include "bsp/cc13_sweepA.hpp"
#include "bsp/engine_seg1_cc13.hpp"
#include "bsp/engine_seg43_45_cc13.hpp"
#include "bsp/frontend_screens_cc13.hpp"
#include "bsp/particle_emitters_cc13.hpp"
#include "bsp/band_006b_cc13.hpp"
#include "bsp/band_004b_cc13.hpp"
#include "bsp/engine_services_cc13.hpp"
#include "bsp/cc13_gui1.hpp"
#include "cc13_leaf_reference.hpp"
#include <float.h>
#include <array>
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
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
            ortho_off_center_lh_pc24(o2, l, r, b, tp, zn, zf);
            const int k = first_diff(o1, o2, 16);
            check_note(orth, k < 0, "l=%a r=%a b=%a t=%a n=%a f=%a [%d] %a / %a", l, r, b, tp, zn, zf, k,
                       k < 0 ? 0.0 : o1[k], k < 0 ? 0.0 : o2[k]);
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
            if (!same(n1, n2) && std::isnan(n1) && std::isnan(n2)) bz.check_nan_only();
            else check_note(bz, same(n1, n2), "p=%a,%a,%a,%a s=%a: %a / %a", p0, p1, p2, p3, s, n1, n2);
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

// cc13-fixture3: the register-ABI Dyn leaves.
// 00C412D0: EBX = segment start, EDX = box; stack (entry_out, &end), RET 8; AL.
bool native_segment_box(unsigned offset, const float* start, const float* box, float* entry,
                        const float* end) {
    void* fn = image + offset;
    unsigned char hit = 0;
    __asm {
        push end
        push entry
        mov ebx, start
        mov edx, box
        call fn
        mov hit, al
    }
    return hit != 0;
}

// 00C309D0 / 00C30D10 / 00C31060 / 00C313A0: ECX = &count, ESI = output; stack (input,
// count, bound), RET 0Ch.
void native_clip(unsigned offset, std::int32_t* count_out, float* output, const float* input,
                 std::int32_t count, float limit) {
    void* fn = image + offset;
    __asm {
        push limit
        push count
        push input
        mov ecx, count_out
        mov esi, output
        call fn
    }
}

// 00C34CF0: EBX = count; stack (output, nodes), RET 8.
void native_expand(unsigned offset, float* output, const void* const* nodes, std::int32_t count) {
    void* fn = image + offset;
    __asm {
        push nodes
        push output
        mov ebx, count
        call fn
    }
}

// A double near a float draw, so the double support routines see full 53-bit operands.
double edge_or_double(float lo, float hi) {
    const float f = edge_or(lo, hi);
    if (!std::isfinite(f) || next() % 4 == 0) return f;
    return static_cast<double>(f) * (1.0 + (static_cast<double>(next()) - 2147483648.0) * 1e-15);
}

// The cc13-fixture3 routines (docs/CC13_FIXTURE_PASS_3.md). Runs once per precision control,
// like run_fixture2.
template<class Open> void run_fixture3(Open open) {
    using namespace bsp::cc13_dyn1;
    alignas(16) static std::uint8_t shape1[0x220], shape2[0x220];
    auto fill_shape = [&] {
        std::memset(shape1, 0xab, sizeof(shape1));
        for (int k = 0; k < 3; ++k) {
            const float e = edge_or(0, 5);
            std::memcpy(shape1 + 0x210 + 4 * k, &e, 4);
        }
        std::memcpy(shape2, shape1, sizeof(shape1));
    };
    auto fill_xf = [&](float xf[12]) {
        for (int k = 0; k < 9; ++k) xf[k] = edge_or(-1.5f, 1.5f);
        for (int k = 9; k < 12; ++k) xf[k] = edge_or(-100, 100);
    };
    {
        auto& c = open("00C47AA0 box shape support (float)");
        auto native = at<void (__fastcall*)(const void*, int, float*, const float*, const float*)>(cc13_leaf_00c47aa0);
        for (int i = 0; i < 3000; ++i) {
            fill_shape();
            float xf[12], dir[3], o1[3] = {7, 7, 7}, o2[3] = {7, 7, 7};
            fill_xf(xf);
            for (auto& v : dir) v = edge_or(-2, 2);
            if (i % 25 == 0) dir[i / 25 % 3] = 0.0f;  // a zero dot picks -1
            native(shape1, 0, o1, dir, xf);
            box_shape_support_00c47aa0(shape2, o2, dir, xf);
            if (nan_only_diff(o1, o2, 3)) { c.check_nan_only(); continue; }
            check_note(c, same(o1, o2), "dir=%a,%a,%a k=%d: %a / %a", dir[0], dir[1], dir[2],
                       first_diff(o1, o2, 3), o1[0], o2[0]);
        }
    }
    auto nan_only_diff_d = [](const double* a, const double* b, int n) {
        bool differs = false;
        for (int k = 0; k < n; ++k) {
            if (std::memcmp(a + k, b + k, 8) == 0) continue;
            if (!(std::isnan(a[k]) && std::isnan(b[k]))) return false;
            differs = true;
        }
        return differs;
    };
    {
        auto& c = open("00C38470 box shape support (double)");
        auto native = at<void (__fastcall*)(const void*, int, double*, const double*, const float*)>(cc13_leaf_00c38470);
        for (int i = 0; i < 3000; ++i) {
            fill_shape();
            float xf[12];
            double dir[3], o1[3] = {7, 7, 7}, o2[3] = {7, 7, 7};
            fill_xf(xf);
            for (auto& v : dir) v = edge_or_double(-2, 2);
            if (i % 25 == 0) dir[i / 25 % 3] = 0.0;
            native(shape1, 0, o1, dir, xf);
            box_shape_support_00c38470(shape2, o2, dir, xf);
            if (nan_only_diff_d(o1, o2, 3)) { c.check_nan_only(); continue; }
            check_note(c, same(o1, o2), "dir=%a,%a,%a: %a,%a,%a / %a,%a,%a", dir[0], dir[1], dir[2],
                       o1[0], o1[1], o1[2], o2[0], o2[1], o2[2]);
        }
    }
    {
        auto& c = open("00C38810 cylinder shape support (double)");
        auto native = at<void (__fastcall*)(const void*, int, double*, const double*, const float*)>(cc13_leaf_00c38810);
        for (int i = 0; i < 3000; ++i) {
            fill_shape();
            float xf[12];
            double dir[3], o1[3] = {7, 7, 7}, o2[3] = {7, 7, 7};
            fill_xf(xf);
            for (auto& v : dir) v = edge_or_double(-2, 2);
            if (i % 25 == 0) dir[i / 25 % 3] = 0.0;
            if (i % 30 == 1) { dir[0] = xf[3] * 1e-4; dir[1] = xf[4]; dir[2] = xf[5] * 1e-4; }  // near the axis
            native(shape1, 0, o1, dir, xf);
            cylinder_shape_support_00c38810(shape2, o2, dir, xf);
            if (nan_only_diff_d(o1, o2, 3)) { c.check_nan_only(); continue; }
            check_note(c, same(o1, o2), "dir=%a,%a,%a: %a,%a,%a / %a,%a,%a", dir[0], dir[1], dir[2],
                       o1[0], o1[1], o1[2], o2[0], o2[1], o2[2]);
        }
    }
    {
        auto& c = open("00C412D0 segment / box slab entry");
        for (int i = 0; i < 3000; ++i) {
            float start[3], end[3], box[6], e1[3] = {7, 7, 7}, e2[3] = {7, 7, 7};
            for (int k = 0; k < 3; ++k) {
                const float a = edge_or(-10, 10), b = edge_or(-10, 10);
                box[k] = i % 7 ? (a < b ? a : b) : a;  // mostly ordered boxes
                box[3 + k] = i % 7 ? (a < b ? b : a) : b;
                start[k] = edge_or(-15, 15);
                end[k] = edge_or(-15, 15);
            }
            if (i % 11 == 0) for (int k = 0; k < 3; ++k) end[k] = start[k];             // degenerate
            if (i % 13 == 0) start[i % 3] = box[i % 3];                                // on a face
            const bool h1 = native_segment_box(cc13_leaf_00c412d0, start, box, e1, end);
            const bool h2 = segment_box_entry_00c412d0(start, box, e2, end);
            if (h1 == h2 && nan_only_diff(e1, e2, 3)) { c.check_nan_only(); continue; }
            check_note(c, h1 == h2 && same(e1, e2), "s=%a,%a,%a e=%a,%a,%a: %d %a / %d %a",
                       start[0], start[1], start[2], end[0], end[1], end[2], h1, e1[0], h2, e2[0]);
        }
    }
    {
        struct Clip { unsigned offset; const char* name;
                      void (*fn)(std::int32_t&, float*, const float*, std::int32_t, float); };
        const Clip clips[] = {
            {cc13_leaf_00c313a0, "00C313A0 polygon clip x at least", clip_x_at_least_00c313a0},
            {cc13_leaf_00c30d10, "00C30D10 polygon clip y at least", clip_y_at_least_00c30d10},
            {cc13_leaf_00c31060, "00C31060 polygon clip x at most", clip_x_at_most_00c31060},
            {cc13_leaf_00c309d0, "00C309D0 polygon clip y at most", clip_y_at_most_00c309d0},
        };
        for (const auto& clip : clips) {
            auto& c = open(clip.name);
            for (int i = 0; i < 2000; ++i) {
                const std::int32_t n = 1 + static_cast<std::int32_t>(next() % 7);
                float in[14], o1[28], o2[28];
                for (int k = 0; k < 2 * n; ++k) in[k] = edge_or(-10, 10);
                const float bound = i % 9 == 0 ? in[next() % (2 * n)] : edge_or(-6, 6);  // on a vertex
                for (int k = 0; k < 28; ++k) o1[k] = o2[k] = -77.0f;
                std::int32_t c1 = -1, c2 = -1;
                native_clip(clip.offset, &c1, o1, in, n, bound);
                clip.fn(c2, o2, in, n, bound);
                const bool counts = c1 == c2;
                if (counts && nan_only_diff(o1, o2, 28)) { c.check_nan_only(); continue; }
                check_note(c, counts && same(o1, o2), "n=%d bound=%a: %d / %d k=%d", n, bound, c1, c2,
                           first_diff(o1, o2, 28));
            }
        }
    }
    {
        auto& c = open("00C34CF0 expand node matrices");
        for (int i = 0; i < 1000; ++i) {
            const std::int32_t n = 1 + static_cast<std::int32_t>(next() % 4);
            alignas(16) float nodes[4][14];
            const void* ptrs[4];
            for (int j = 0; j < 4; ++j) {
                for (auto& v : nodes[j]) v = edge_or(-100, 100);
                ptrs[j] = nodes[j];  // +08h is nodes[j][2]
            }
            float o1[64], o2[64];
            for (int k = 0; k < 64; ++k) o1[k] = o2[k] = -77.0f;
            native_expand(cc13_leaf_00c34cf0, o1, ptrs, n);
            expand_node_matrices_00c34cf0(o2, ptrs, n);
            if (nan_only_diff(o1, o2, 64)) { c.check_nan_only(); continue; }
            check_note(c, same(o1, o2), "n=%d k=%d", n, first_diff(o1, o2, 64));
        }
    }
    {
        auto& c = open("00C37F60 body add force at point");
        auto native = at<void (__fastcall*)(void*, int, const float*, const float*)>(cc13_leaf_00c37f60);
        for (int i = 0; i < 3000; ++i) {
            alignas(16) float body1[0x18], body2[0x18], m1[0x18], m2[0x18];
            for (int k = 0; k < 0x18; ++k) { body1[k] = edge_or(-50, 50); m1[k] = edge_or(-50, 50); }
            std::uint32_t flags = next();
            std::memcpy(&body1[0x14], &flags, 4);  // B+50h
            std::memcpy(body2, body1, sizeof(body1));
            std::memcpy(m2, m1, sizeof(m1));
            void* p1 = m1;
            void* p2 = m2;
            std::memcpy(&body1[1], &p1, 4);  // B+4h = motion
            std::memcpy(&body2[1], &p2, 4);
            float force[3], point[3];
            for (int k = 0; k < 3; ++k) { force[k] = edge_or(-1000, 1000); point[k] = edge_or(-60, 60); }
            native(body1, 0, force, point);
            body_add_force_at_point_00c37f60(body2, force, point);
            std::memcpy(&body1[1], &p2, 4);  // compare the motion pointers as equal
            const bool exact = same(body1, body2) && same(m1, m2);
            if (!exact && (same(body1, body2) || nan_only_diff(body1, body2, 0x18)) &&
                (same(m1, m2) || nan_only_diff(m1, m2, 0x18))) {
                c.check_nan_only();
                continue;
            }
            check_note(c, exact, "f=%a,%a,%a k=%d", force[0], force[1], force[2], first_diff(m1, m2, 0x18));
        }
    }
    {
        auto& c = open("00C43EA0 interpolate body transform");
        auto native = at<void (__fastcall*)(void*, int, float*, float)>(cc13_leaf_00c43ea0);
        for (int i = 0; i < 3000; ++i) {
            alignas(16) float body[0x10], motion[0x30];
            bsp::DynamicsTransform34 previous, current;
            for (int k = 0; k < 12; ++k) { previous.m[k] = edge_or(-100, 100); current.m[k] = edge_or(-100, 100); }
            std::memset(body, 0, sizeof(body));
            std::memset(motion, 0, sizeof(motion));
            void* m = motion;
            std::memcpy(&body[1], &m, 4);                        // B+4h = motion
            std::memcpy(&body[2], current.m, sizeof(current.m));  // B+08h..34h
            std::memcpy(&motion[0x21], previous.m, sizeof(previous.m));  // M+84h..B0h
            const float alpha = i % 10 == 0 ? static_cast<float>(i % 3) * 0.5f : edge_or(-0.5f, 1.5f);
            float o1[12];
            for (auto& v : o1) v = -77.0f;
            native(body, 0, o1, alpha);
            const bsp::DynamicsTransform34 o2 = bsp::interpolate_body_transform_00c43ea0(previous, current, alpha);
            if (nan_only_diff(o1, o2.m, 12)) { c.check_nan_only(); continue; }
            check_note(c, same(o1, o2.m), "t=%a k=%d", alpha, first_diff(o1, o2.m, 12));
        }
    }
}

// cc13-fixture5: game-logic float leaves, each against its original bytes. All of them run
// on the main thread, so the 24-bit pass decides (docs/CC13_FPU_PRECISION.md).
template<class Open> void run_fixture5(Open open) {
    auto pick = [](float lo, float hi) { return edge_or(lo, hi); };
    {
        auto& c = open("00BD4BF0 matrix4x4 equals");
        auto native = at<int (__fastcall*)(const float*, int, const float*)>(cc13_leaf_00bd4bf0);
        for (int i = 0; i < 3000; ++i) {
            float a[16], b[16];
            for (int k = 0; k < 16; ++k) a[k] = b[k] = pick(-10, 10);
            if (i % 3 == 1) b[next() % 16] = pick(-10, 10);
            if (i % 3 == 2) { const int k = next() % 16; b[k] = -b[k]; }   // signed zero and NaN cases
            const bool n1 = native(a, 0, b) != 0, n2 = bsp::cc13_seg95::matrix4x4_equals_00bd4bf0(a, b);
            check_note(c, n1 == n2, "%d / %d", n1, n2);
        }
    }
    {
        auto& over = open("009FA510 world edge overrun");
        auto& near_edge = open("00681F40 game near world edge");
        auto& beyond = open("0059C9B0 world point beyond map margin");
        auto overrun = at<float (__fastcall*)(const void*, int, const float*, float)>(cc13_leaf_009fa510);
        auto near_n = at<int (__fastcall*)(const void*, int, const float*, float)>(cc13_leaf_00681f40);
        auto beyond_n = at<int (__fastcall*)(const void*, int, const float*, float)>(cc13_leaf_0059c9b0);
        static std::uint8_t game[0x7140];
        for (int i = 0; i < 4000; ++i) {
            bsp::cc13_seg68::WorldEdges e;
            e.x_min_711c = pick(-2000, -500); e.x_max_7128 = pick(500, 2000);
            e.z_max_7124 = pick(500, 2000); e.z_min_7130 = pick(-2000, -500);
            std::memcpy(game + 0x711c, &e.x_min_711c, 4); std::memcpy(game + 0x7128, &e.x_max_7128, 4);
            std::memcpy(game + 0x7124, &e.z_max_7124, 4); std::memcpy(game + 0x7130, &e.z_min_7130, 4);
            const float p[3] = {pick(-2500, 2500), pick(-50, 50), pick(-2500, 2500)};
            const float m = pick(-100, 400);
            const float r1 = overrun(game, 0, p, m), r2 = bsp::cc13_seg68::ggame_world_edge_overrun_009fa510(e, p, m);
            if (std::isnan(r1) && std::isnan(r2) && !same(r1, r2)) over.check_nan_only();
            else check_note(over, same(r1, r2), "p=%a,%a m=%a: %a / %a", p[0], p[2], m, r1, r2);
            const bool a1 = near_n(game, 0, p, m) != 0;
            const bool a2 = bsp::cc13_fe5::game_near_world_edge_00681f40(bsp::ingame_gui_cc13::Raw{game}, p, m);
            check_note(near_edge, a1 == a2, "p=%a,%a m=%a: %d / %d", p[0], p[2], m, a1, a2);
            const bool b1 = beyond_n(game, 0, p, m) != 0;
            const bool b2 = bsp::cc13_fe1::world_point_beyond_map_margin_0059c9b0(game, p, m);
            check_note(beyond, b1 == b2, "p=%a,%a m=%a: %d / %d", p[0], p[2], m, b1, b2);
        }
    }
    {
        auto& c = open("006FBCE0 elevator platform start travel");
        auto native = at<void (__fastcall*)(void*, int, std::uint32_t, float)>(cc13_leaf_006fbce0);
        for (int i = 0; i < 4000; ++i) {
            alignas(4) std::uint8_t o1[0x50]{}, o2[0x50]{};
            const float h = pick(-20, 60), m = pick(0, 10), s = pick(0.1f, 20), d = pick(0, 50);
            std::memcpy(o1 + 0x18, &h, 4); std::memcpy(o1 + 0x1c, &m, 4);
            std::memcpy(o1 + 0x44, &s, 4); std::memcpy(o1 + 0x48, &d, 4);
            std::memcpy(o2, o1, sizeof o1);
            const std::uint8_t down = static_cast<std::uint8_t>(next() & 1);
            const float t = pick(-5, 30);
            native(o1, 0, down, t);
            bsp::dummy_depth_seg34_argon::ElevatorPlatform_StartTravel_006fbce0(o2, down, t);
            float r1, r2;
            std::memcpy(&r1, o1 + 0x4c, 4); std::memcpy(&r2, o2 + 0x4c, 4);
            if (std::isnan(r1) && std::isnan(r2) && !same(r1, r2)) c.check_nan_only();
            else check_note(c, std::memcmp(o1, o2, sizeof o1) == 0, "h=%a m=%a s=%a d=%a t=%a down=%d: %a / %a",
                            h, m, s, d, t, down, r1, r2);
        }
    }
    {
        auto& c = open("0078FAF0 movie camera approach offset");
        auto native = at<void (__fastcall*)(float*, int, float)>(cc13_leaf_0078faf0);
        for (int i = 0; i < 4000; ++i) {
            std::array<float, 3> s1{pick(-50, 50), pick(-50, 50), pick(-100, 100)};
            if (i % 25 == 0) s1[1] = s1[0];
            std::array<float, 3> s2 = s1;
            const float dt = pick(0, 0.2f);
            native(s1.data(), 0, dt);
            bsp::cc13cam2::movcam_approach_0078faf0(s2, dt);
            if (nan_only_diff(s1.data(), s2.data(), 3)) { c.check_nan_only(); continue; }
            const int k = first_diff(s1.data(), s2.data(), 3);
            check_note(c, k < 0, "dt=%a [%d] %a / %a", dt, k, k < 0 ? 0.0 : s1[k], k < 0 ? 0.0 : s2[k]);
        }
        auto& e = open("0078FCF0 movie camera ease in-out quad");
        auto ease = at<float (__stdcall*)(float)>(cc13_leaf_0078fcf0);
        for (int i = 0; i < 4000; ++i) {
            const float x = i < 64 ? static_cast<float>(i) / 63.0f : pick(-0.5f, 1.5f);
            const float r1 = ease(x), r2 = bsp::movie_camera_ease_in_out_quad_0078fcf0(x);
            if (std::isnan(r1) && std::isnan(r2) && !same(r1, r2)) e.check_nan_only();
            else check_note(e, same(r1, r2), "x=%a: %a / %a", x, r1, r2);
        }
    }
    {
        auto& c = open("0080E000 unit AI order slot step");
        auto native = at<void (__fastcall*)(void*, int, float)>(cc13_leaf_0080e000);
        for (int i = 0; i < 4000; ++i) {
            alignas(4) std::uint8_t raw[0x40];
            for (auto& b : raw) b = static_cast<std::uint8_t>(next());
            const float blend = pick(-50, 50), timer = pick(-0.1f, 0.3f), target = pick(-50, 50);
            std::memcpy(raw + 0x00, &blend, 4); std::memcpy(raw + 0x04, &timer, 4); std::memcpy(raw + 0x08, &target, 4);
            for (int off : {0x0c, 0x10, 0x18, 0x1c, 0x28, 0x2c, 0x34, 0x38}) { const float f = pick(-9, 9); std::memcpy(raw + off, &f, 4); }
            raw[0x14] &= 1; raw[0x30] &= 1;
            bsp::UnitAiOrderRecord r;
            auto rd = [&](int off) { float f; std::memcpy(&f, raw + off, 4); return f; };
            auto sub = [&](int base) {
                bsp::UnitAiOrderSubRecord s;
                s.value_00 = rd(base); s.field_04 = rd(base + 4); s.field_08 = rd(base + 8); s.flag_0c = raw[base + 0xc] != 0;
                s.field_10 = rd(base + 0x10); s.field_14 = rd(base + 0x14);
                std::memcpy(&s.field_18, raw + base + 0x18, 4);
                return s;
            };
            r.blend_00 = rd(0); r.timer_04 = rd(4); r.sub_a = sub(0x08); r.sub_b = sub(0x24);
            const float dt = pick(0, 0.2f);
            std::uint8_t out[0x40];
            std::memcpy(out, raw, sizeof raw);
            native(out, 0, dt);
            bsp::unit_ai_order_slot_step_0080e000(r, dt, true);
            std::uint8_t mine[0x40];
            std::memcpy(mine, raw, sizeof raw);
            auto put = [&](int base, const bsp::UnitAiOrderSubRecord& s) {
                std::memcpy(mine + base, &s.value_00, 4); std::memcpy(mine + base + 4, &s.field_04, 4);
                std::memcpy(mine + base + 8, &s.field_08, 4); mine[base + 0xc] = s.flag_0c ? 1 : 0;
                std::memcpy(mine + base + 0x10, &s.field_10, 4); std::memcpy(mine + base + 0x14, &s.field_14, 4);
                std::memcpy(mine + base + 0x18, &s.field_18, 4);
            };
            std::memcpy(mine, &r.blend_00, 4); std::memcpy(mine + 4, &r.timer_04, 4);
            put(0x08, r.sub_a); put(0x24, r.sub_b);
            int k = -1;
            for (int b = 0; b < 0x40 && k < 0; ++b) if (out[b] != mine[b]) k = b;
            const bool nan_only = k >= 0 && nan_only_diff(reinterpret_cast<const float*>(out), reinterpret_cast<const float*>(mine), 16);
            if (nan_only) c.check_nan_only();
            else { float n0, m0; std::memcpy(&n0, out + (k & ~3), 4); std::memcpy(&m0, mine + (k & ~3), 4); check_note(c, k < 0, "blend=%a timer=%a target=%a dt=%a byte %d differs: %a / %a", blend, timer, target, dt, k, n0, m0); }
        }
    }
    {
        using namespace bsp::engine_seg11_35_66_cc13;
        auto& c = open("004F3030 segment lerp");
        auto native = at<void (__fastcall*)(const float*, int, float*, float)>(cc13_leaf_004f3030);
        for (int i = 0; i < 4000; ++i) {
            const float seg[4] = {pick(-100, 100), pick(-100, 100), pick(-100, 100), pick(-100, 100)};
            const float t = pick(-0.5f, 1.5f);
            float o1[2] = {7, 7}, o2[2] = {7, 7};
            native(seg, 0, o1, t);
            segment_lerp_004f3030(seg, o2, t);
            if (nan_only_diff(o1, o2, 2)) { c.check_nan_only(); continue; }
            const int k = first_diff(o1, o2, 2);
            check_note(c, k < 0, "t=%a [%d] %a / %a", t, k, k < 0 ? 0.0 : o1[k], k < 0 ? 0.0 : o2[k]);
        }
        auto& l = open("004F40C0 is left turn");
        auto left = at<int (__stdcall*)(float, float, float, float, float, float)>(cc13_leaf_004f40c0);
        for (int i = 0; i < 4000; ++i) {
            float v[6];
            for (auto& x : v) x = pick(-100, 100);
            if (i % 20 == 0) { v[4] = v[0] + (v[2] - v[0]) * 2; v[5] = v[1] + (v[3] - v[1]) * 2; }   // collinear
            const bool n1 = left(v[0], v[1], v[2], v[3], v[4], v[5]) != 0;
            const bool n2 = is_left_turn_004f40c0(v[0], v[1], v[2], v[3], v[4], v[5]);
            check_note(l, n1 == n2, "%a,%a %a,%a %a,%a: %d / %d", v[0], v[1], v[2], v[3], v[4], v[5], n1, n2);
        }
    }
    {
        auto& c = open("004F2FD0 circle contains point");
        auto native = at<int (__fastcall*)(const float*, int, const float*)>(cc13_leaf_004f2fd0);
        for (int i = 0; i < 4000; ++i) {
            const float circle[3] = {pick(-100, 100), pick(-100, 100), pick(0, 50)};
            float p[2] = {pick(-150, 150), pick(-150, 150)};
            if (i % 20 == 0) { p[0] = circle[0] + circle[2]; p[1] = circle[1]; }   // on the rim
            const bool n1 = native(circle, 0, p) != 0, n2 = bsp::cc13_sweepA::circle2f_contains_point_004f2fd0(circle, p);
            check_note(c, n1 == n2, "c=%a,%a r=%a p=%a,%a: %d / %d", circle[0], circle[1], circle[2], p[0], p[1], n1, n2);
        }
    }
    {
        auto& c = open("004439B0 time of day hours");
        auto native = at<void (__fastcall*)(void*, int, std::int32_t, std::int32_t, std::int32_t, std::int32_t)>(cc13_leaf_004439b0);
        for (int i = 0; i < 4000; ++i) {
            auto n = [](int span) { return static_cast<std::int32_t>(next() % (2 * span + 1)) - span; };
            const std::int32_t h = i % 4 ? n(30) : n(5000), m = i % 4 ? n(70) : n(300000),
                               s = i % 4 ? n(70) : n(10000000), ms = i % 4 ? n(1000) : n(100000000);
            alignas(4) std::uint8_t self[8]{};
            native(self, 0, h, m, s, ms);
            float r1;
            std::memcpy(&r1, self + 4, 4);
            const float r2 = bsp::cc13_band0040::time_of_day_hours_004439b0(h, m, s, ms);
            check_note(c, same(r1, r2), "%d:%d:%d.%d: %a / %a", h, m, s, ms, r1, r2);
        }
    }
    {
        auto& c = open("006E2490 projectile shot speed");
        auto native = at<float (__fastcall*)(void*)>(cc13_leaf_006e2490);
        for (int i = 0; i < 4000; ++i) {
            alignas(4) std::uint8_t shot[0x14]{};
            const float v[3] = {pick(-500, 500), pick(-500, 500), pick(-500, 500)};
            if (i % 30 == 0) { const float z = 1e-6f; std::memcpy(shot + 8, &z, 4); }
            else std::memcpy(shot + 8, v, 12);
            const float r1 = native(shot), r2 = bsp::cc13_seg33a::projectile_shot_speed_006e2490(shot);
            if (std::isnan(r1) && std::isnan(r2) && !same(r1, r2)) c.check_nan_only();
            else check_note(c, same(r1, r2), "v=%a,%a,%a: %a / %a", v[0], v[1], v[2], r1, r2);
        }
    }
    {
        auto& s = open("009B94E0 split-S roll step");
        auto& r = open("009B9680 maneuver roll completed");
        auto step = at<void (__fastcall*)(void*, int, std::uint32_t)>(cc13_leaf_009b94e0);
        auto done = at<std::uint8_t (__fastcall*)(void*)>(cc13_leaf_009b9680);
        for (int i = 0; i < 4000; ++i) {
            static std::uint8_t unit[0xc70], desc[0x1c0];
            alignas(4) std::uint8_t t1[0x404]{}, t2[0x404]{};
            const float rate = pick(0.1f, 5), span = pick(0.1f, 5), pitch = pick(-3.2f, 3.2f), bank = uniform(-3.2f, 3.2f);  // 00438B10 loops on an infinite angle
            std::memcpy(desc + 0x1a8, &rate, 4); std::memcpy(desc + 0x1bc, &span, 4);
            std::memcpy(unit + 0xc64, &pitch, 4); std::memcpy(unit + 0xc68, &bank, 4);
            void* u = unit; void* d = desc;
            std::memcpy(t1 + 0x2f4, &u, 4); std::memcpy(t1 + 0x2f8, &d, 4);
            const float goal = pick(-3.2f, 3.2f);
            std::memcpy(t1 + 0x400, &goal, 4);
            std::memcpy(t2, t1, sizeof t1);
            const std::uint8_t wrap = static_cast<std::uint8_t>(next() & 1);
            step(t1, 0, wrap);
            bsp::bot_task_split_s_roll_step_009b94e0(t2, wrap, &bsp::wrapped_angle_subtract_00438b10);
            float a1, a2;
            std::memcpy(&a1, t1 + 0x294, 4); std::memcpy(&a2, t2 + 0x294, 4);
            if (std::isnan(a1) && std::isnan(a2) && !same(a1, a2)) s.check_nan_only();
            else check_note(s, std::memcmp(t1, t2, sizeof t1) == 0, "pitch=%a bank=%a wrap=%d: %a / %a", pitch, bank, wrap, a1, a2);
            std::memcpy(t2, t1, sizeof t1);
            const bool d1 = done(t1) != 0, d2 = bsp::bot_task_maneuver_roll_completed_009b9680(t2);
            float b1, b2;
            std::memcpy(&b1, t1 + 0x2a0, 4); std::memcpy(&b2, t2 + 0x2a0, 4);
            if (d1 == d2 && std::isnan(b1) && std::isnan(b2) && !same(b1, b2)) r.check_nan_only();
            else check_note(r, d1 == d2 && std::memcmp(t1, t2, sizeof t1) == 0, "goal=%a pitch=%a bank=%a: %d %a / %d %a",
                            goal, pitch, bank, d1, b1, d2, b2);
        }
    }
    {
        auto& c = open("00786030 average peer quality");
        auto native = at<float (__fastcall*)(const void*)>(cc13_leaf_00786030);
        for (int i = 0; i < 4000; ++i) {
            alignas(4) std::uint8_t raw[0xe0]{};
            bsp::mp_cc13::PeerQualityRow rows[8]{};
            for (int k = 1; k < 8; ++k) {
                rows[k].present_00 = (next() % 3) ? 0u : next();
                rows[k].value_0c = pick(-2, 2);
                std::memcpy(raw + 0x1c * k, &rows[k].present_00, 4);
                std::memcpy(raw + 0x1c * k + 0xc, &rows[k].value_0c, 4);
            }
            const float r1 = native(raw), r2 = bsp::mp_cc13::average_peer_quality_00786030(rows);
            if (std::isnan(r1) && std::isnan(r2) && !same(r1, r2)) c.check_nan_only();
            else check_note(c, same(r1, r2), "%a / %a", r1, r2);
        }
    }
    {
        auto& c = open("00815440 build unit order record");
        auto native = at<void* (__fastcall*)(void*, int, float, float, std::uint32_t)>(cc13_leaf_00815440);
        for (int i = 0; i < 4000; ++i) {
            bsp::UnitOrderRecordStorage s1{}, s2{};
            for (auto& b : s1) b = static_cast<std::uint8_t>(next());
            s2 = s1;
            const float a = pick(-4, 4), b = pick(-4, 4);
            const std::uint8_t kind = static_cast<std::uint8_t>(next());
            void* back = native(s1.data(), 0, a, b, kind | 0xabcd0000u);
            bsp::build_unit_order_record_00815440(s2, a, b, kind);
            check_note(c, back == s1.data() && s1 == s2, "a=%a b=%a", a, b);
        }
    }
    {
        auto& c = open("007B9620 plane altitude sink predicate");
        auto native = at<std::uint8_t (__fastcall*)(void*)>(cc13_leaf_007b9620);
        for (int i = 0; i < 4000; ++i) {
            static std::uint8_t plane[0xc70], desc[0x1b0];
            void* d = desc;
            std::memcpy(plane + 0x538, &d, 4);
            const float alt = pick(-50, 400), sink = pick(-30, 10), pitch = pick(-1.6f, 1.6f), stall = pick(10, 100);
            std::memcpy(plane + 0x9b4, &alt, 4); std::memcpy(plane + 0x9b8, &sink, 4);
            std::memcpy(plane + 0xc64, &pitch, 4); std::memcpy(desc + 0x1ac, &stall, 4);
            const bool n1 = native(plane) != 0;
            const bool n2 = bsp::cc13_seg47::plane_altitude_sink_predicate_007b9620(bsp::cc13_seg47::Raw{plane});
            check_note(c, n1 == n2, "alt=%a sink=%a pitch=%a stall=%a: %d / %d", alt, sink, pitch, stall, n1, n2);
        }
    }
    {
        auto& c = open("00727C30 index of readiest barrel");
        auto native = at<std::int32_t (__fastcall*)(void*)>(cc13_leaf_00727c30);
        for (int i = 0; i < 4000; ++i) {
            alignas(4) std::uint8_t gun[0x450]{};
            float timers[24];
            const std::int32_t n = static_cast<std::int32_t>(next() % 24);
            for (int k = 0; k < 24; ++k) timers[k] = (next() % 4) ? pick(-1, 5) : static_cast<float>(next() % 3);
            float* p = timers;
            std::memcpy(gun + 0x414, &p, 4); std::memcpy(gun + 0x448, &n, 4);
            const std::int32_t r1 = native(gun);
            const std::int32_t r2 = bsp::director_geommesh_seg38_39_argon::bsp_gun_index_of_readiest_barrel_00727c30(gun);
            check_note(c, r1 == r2, "n=%d: %d / %d", n, r1, r2);
        }
    }
}
// cc13-fixture8: defined at the end of the file.
template<class Open> void run_fixture8(Open open);

}  // namespace

int main() {
    unsigned control{};
    _controlfp_s(&control, _PC_53 | _RC_NEAR, _MCW_PC | _MCW_RC);
    load_image();
    // cc13-fixture8: each fixture pass adds cases here, so overflow is a hard failure rather
    // than a silent write past the end.
    constexpr int kMaxCases = 512;
    static Case cases[kMaxCases]{};
    int count = 0;
    auto open = [&](const char* name) -> Case& {
        if (count >= kMaxCases) {
            std::printf("too many cc13 leaf cases (limit %d): raise kMaxCases\n", kMaxCases);
            std::fflush(stdout);
            std::exit(1);
        }
        cases[count].name = name;
        return cases[count++];
    };

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
        run_fixture3([&](const char* name) -> Case& { return open((std::string(name) + tag).c_str()); });
        run_fixture5([&](const char* name) -> Case& { return open((std::string(name) + tag).c_str()); });
    }
    // cc13-fixture8: the same two passes over its own input stream.
    for (const unsigned pc : {static_cast<unsigned>(_PC_53), static_cast<unsigned>(_PC_24)}) {
        _controlfp_s(&control, pc | _RC_NEAR, _MCW_PC | _MCW_RC);
        state = 0x5eed0f08u;
        const std::string tag = pc == _PC_53 ? " @53" : " @24";
        run_fixture8([&](const char* name) -> Case& { return open((std::string(name) + tag).c_str()); });
    }
    // cc13-fixture7: Dyn shape supports and transforms, and main-thread vector helpers. Each
    // routine runs at 53-bit and at 24-bit precision control on the same inputs; the deciding
    // pass is the calling thread's (primary53 below lists the worker-only routines).
    {
        // Same-bits compare of n floats: 1 equal, 0 differs, -1 differs only where both are NaN.
        const auto cmp = [](const float* a, const float* b, int n) {
            if (std::memcmp(a, b, 4 * static_cast<std::size_t>(n)) == 0) return 1;
            return nan_only_diff(a, b, n) ? -1 : 0;
        };
        const auto record = [](Case& c, int r) { if (r < 0) c.check_nan_only(); else c.check(r == 1); };
        const auto bits = [] { std::uint32_t u = next(); float f; std::memcpy(&f, &u, 4); return f; };
        const auto run_fixture7 = [&](auto open7) {
            {
                auto& c = open7("00C48080 sphere support (float)");
                auto native = at<void (__fastcall*)(const void*, int, float*, const float*, const float*)>(cc13_leaf_00c48080);
                alignas(16) unsigned char shape[0x220]{};
                for (int i = 0; i < 2000; ++i) {
                    const float r = edge_or(0, 50);
                    std::memcpy(shape + 0x210, &r, 4);
                    float dir[3], xf[12], o1[3] = {7, 7, 7}, o2[3] = {7, 7, 7};
                    for (float& v : dir) v = edge_or(-1, 1);
                    for (float& v : xf) v = edge_or(-1000, 1000);
                    native(shape, 0, o1, dir, xf);
                    bsp::cc13_dyn1::sphere_shape_support_00c48080(shape, o2, dir, xf);
                    record(c, cmp(o1, o2, 3));
                }
            }
            {
                auto& c = open7("00C38970 sphere support (double)");
                auto native = at<void (__fastcall*)(const void*, int, double*, const double*, const float*)>(cc13_leaf_00c38970);
                alignas(16) unsigned char shape[0x220]{};
                for (int i = 0; i < 2000; ++i) {
                    const float r = edge_or(0, 50);
                    std::memcpy(shape + 0x210, &r, 4);
                    double dir[3], o1[3] = {7, 7, 7}, o2[3] = {7, 7, 7};
                    float xf[12];
                    for (double& v : dir) v = static_cast<double>(edge_or(-1, 1)) + (next() % 2 ? 1e-9 : 0.0);
                    for (float& v : xf) v = edge_or(-1000, 1000);
                    native(shape, 0, o1, dir, xf);
                    bsp::cc13_dyn1::sphere_shape_support_00c38970(shape, o2, dir, xf);
                    const bool same = std::memcmp(o1, o2, sizeof(o1)) == 0;
                    bool nan = !same;
                    for (int k = 0; k < 3; ++k)
                        if (std::memcmp(&o1[k], &o2[k], 8) != 0 && !(std::isnan(o1[k]) && std::isnan(o2[k]))) nan = false;
                    if (nan) c.check_nan_only(); else c.check(same);
                }
            }
            {
                auto& c = open7("00C377F0 ccw 2D");
                auto native = at<int (__stdcall*)(float, float, float, float, float, float)>(cc13_leaf_00c377f0);
                for (int i = 0; i < 4000; ++i) {
                    float v[6];
                    for (float& x : v) x = edge_or(-100, 100);
                    if (i % 20 == 0) { v[4] = v[0] + (v[2] - v[0]) * 2.0f; v[5] = v[1] + (v[3] - v[1]) * 2.0f; }  // collinear
                    const bool n1 = native(v[0], v[1], v[2], v[3], v[4], v[5]) != 0;
                    const bool n2 = bsp::cc13_dyn1::ccw_2d_00c377f0(v[0], v[1], v[2], v[3], v[4], v[5]);
                    check_note(c, n1 == n2, "%a %a %a %a %a %a: %d / %d", v[0], v[1], v[2], v[3], v[4], v[5], n1, n2);
                }
            }
            {
                // Random bit patterns, signalling NaNs included: FLD/FSTP quiets them.
                auto& e = open7("00C33650 transform 3x4 to 4x4");
                auto expand = at<void (__fastcall*)(const float*, int, float*)>(cc13_leaf_00c33650);
                auto& f = open7("00C336C0 transform from 4x4");
                auto from = at<void (__fastcall*)(float*, int, const float*)>(cc13_leaf_00c336c0);
                for (int i = 0; i < 1000; ++i) {
                    float src[16], a1[16], a2[16], b1[12], b2[12];
                    for (float& v : src) v = i % 2 ? bits() : edge_or(-10, 10);
                    for (int k = 0; k < 16; ++k) a1[k] = a2[k] = 3.0f;
                    expand(src, 0, a1);
                    bsp::cc13_dyn1::transform_expand_3x4_to_4x4_00c33650(src, a2);
                    check_note(e, std::memcmp(a1, a2, sizeof(a1)) == 0, "[%d] %08x / %08x", first_diff(a1, a2, 16),
                               first_diff(a1, a2, 16) < 0 ? 0u : reinterpret_cast<std::uint32_t&>(a1[first_diff(a1, a2, 16)]),
                               first_diff(a1, a2, 16) < 0 ? 0u : reinterpret_cast<std::uint32_t&>(a2[first_diff(a1, a2, 16)]));
                    for (int k = 0; k < 12; ++k) b1[k] = b2[k] = 3.0f;
                    from(b1, 0, src);
                    bsp::cc13_dyn1::transform_from_4x4_00c336c0(b2, src);
                    f.check(std::memcmp(b1, b2, sizeof(b1)) == 0);
                }
            }
            {
                auto& c = open7("00C37F10 body inertia");
                auto native = at<void (__fastcall*)(const void*, int, float*)>(cc13_leaf_00c37f10);
                alignas(16) unsigned char m[0x60]{};
                void* body[2] = {nullptr, m};
                for (int i = 0; i < 2000; ++i) {
                    for (int k = 0; k < 3; ++k) { const float v = edge_or(-5, 5); std::memcpy(m + 0x54 + 4 * k, &v, 4); }
                    float o1[3] = {7, 7, 7}, o2[3] = {7, 7, 7};
                    native(body, 0, o1);
                    bsp::cc13_dyn1::body_inertia_00c37f10(body, o2);
                    record(c, cmp(o1, o2, 3));
                }
            }
            {
                auto& c = open7("00709B60 vec3 add scaled");
                auto native = at<void (__fastcall*)(float*, const float*, float)>(cc13_leaf_00709b60);
                auto& d = open7("006E0540 vec3 add scaled");
                auto native2 = at<void (__fastcall*)(float*, int, const float*, float)>(cc13_leaf_006e0540);
                for (int i = 0; i < 3000; ++i) {
                    float v[3], dv[3];
                    for (float& x : v) x = edge_or(-100, 100);
                    for (float& x : dv) x = edge_or(-10, 10);
                    const float s = edge_or(-3, 3);
                    float o1[3] = {v[0], v[1], v[2]};
                    bsp::band_0067_cc13::Vec3 o2{v[0], v[1], v[2]};
                    native(o1, dv, s);
                    bsp::band_0067_cc13::vec3_add_scaled_in_place_00709b60(o2, {dv[0], dv[1], dv[2]}, s);
                    const float r2[3] = {o2.x, o2.y, o2.z};
                    record(c, cmp(o1, r2, 3));
                    float p1[3] = {v[0], v[1], v[2]}, p2[3] = {v[0], v[1], v[2]};
                    native2(p1, 0, dv, s);
                    bsp::band_006b_cc13::vec3_add_scaled_in_place_006e0540(p2, dv, s);
                    record(d, cmp(p1, p2, 3));
                }
            }
            {
                auto& c = open7("0059C920 world bounds center");
                auto native = at<void (__fastcall*)(const void*, int, float*)>(cc13_leaf_0059c920);
                auto& d = open7("004B4850 map box center");
                auto native2 = at<void (__fastcall*)(const void*, int, float*)>(cc13_leaf_004b4850);
                static unsigned char game[0x7140]{};
                for (int i = 0; i < 3000; ++i) {
                    float f[6];
                    for (float& x : f) x = edge_or(-5000, 5000);
                    std::memcpy(game + 0x711c, f, sizeof(f));
                    float o1[3] = {7, 7, 7}, o2[3] = {7, 7, 7}, q1[3] = {7, 7, 7};
                    native(game, 0, o1);
                    bsp::cc13_fe1::game_world_bounds_center_0059c920(game, o2);
                    record(c, cmp(o1, o2, 3));
                    native2(game, 0, q1);
                    const bsp::cc13_band004b::GameMapBox box{f[0], f[1], f[2], f[3], f[4], f[5]};
                    const std::array<float, 3> q2 = bsp::cc13_band004b::game_map_box_center_004b4850(box);
                    record(d, cmp(q1, q2.data(), 3));
                }
            }
            {
                auto& a = open7("004193E0 vector3 length squared");
                auto la = at<float (__fastcall*)(const float*)>(cc13_leaf_004193e0);
                auto& b = open7("00427E30 vector3 length squared");
                auto lb = at<float (__fastcall*)(const float*)>(cc13_leaf_00427e30);
                auto& c2 = open7("004191D0 vector2 length squared");
                auto lc = at<float (__fastcall*)(const float*)>(cc13_leaf_004191d0);
                for (int i = 0; i < 3000; ++i) {
                    float v[3];
                    for (float& x : v) x = edge_or(-1000, 1000);
                    // Tiny and huge vectors: the squares leave float range on the stack.
                    if (i % 5 == 1) for (float& x : v) x = std::ldexp(x, -72);
                    if (i % 5 == 2) for (float& x : v) x = std::ldexp(x, 54);
                    float n = la(v), r = bsp::engine_seg1_cc13::vector3_length_squared_004193e0(v);
                    record(a, cmp(&n, &r, 1));
                    n = lb(v); r = bsp::cc13_left61::vector3_length_squared_00427e30(v);
                    record(b, cmp(&n, &r, 1));
                    n = lc(v); r = bsp::engine_seg1_cc13::vector2_length_squared_004191d0(v);
                    record(c2, cmp(&n, &r, 1));
                }
            }
            {
                auto& c = open7("00BEE140 scale vector4");
                auto native = at<void (__fastcall*)(float*, const float*, const float*)>(cc13_leaf_00bee140);
                auto& d = open7("004E5D20 rgba scale");
                auto native2 = at<void (__fastcall*)(float*, int, float)>(cc13_leaf_004e5d20);
                for (int i = 0; i < 3000; ++i) {
                    float v[4], o1[4] = {7, 7, 7, 7}, o2[4] = {7, 7, 7, 7};
                    for (float& x : v) x = edge_or(-100, 100);
                    const float s = edge_or(-4, 4);
                    native(o1, &s, v);
                    // The 24-bit arm (kScaleVector4Pc24Cc13Bound is still off on the live path).
                    bsp::engine_services_cc13::scale_vector4_pc24_00bee140(o2, &s, v);
                    record(c, cmp(o1, o2, 4));
                    float p1[4] = {v[0], v[1], v[2], v[3]}, p2[4] = {v[0], v[1], v[2], v[3]};
                    native2(p1, 0, s);
                    bsp::scene_weather_cc13::scale_rgba_004e5d20(p2, s);
                    record(d, cmp(p1, p2, 4));
                }
            }
            {
                auto& c = open7("00AB6BD0 text height");
                auto native = at<float (__fastcall*)(const void*)>(cc13_leaf_00ab6bd0);
                alignas(16) unsigned char ctx[0x180]{}, font[0x20]{};
                for (int i = 0; i < 3000; ++i) {
                    const int mode = static_cast<int>(next() % 3);
                    ctx[0xfc] = mode == 0 ? static_cast<unsigned char>(1 + next() % 255) : 0;
                    const float h = edge_or(-100, 1000);
                    std::memcpy(ctx + 0x178, &h, 4);
                    const std::int16_t fh = static_cast<std::int16_t>(next());
                    std::memcpy(font + 0x14, &fh, 2);
                    void* const fp = mode == 1 ? font : nullptr;
                    std::memcpy(ctx + 0x108, &fp, 4);
                    float n = native(ctx), r = bsp::cc13_gui1::text_context_get_normalized_text_height_00ab6bd0(ctx);
                    record(c, cmp(&n, &r, 1));
                }
            }
            {
                auto& c = open7("007B8140 float window sample and sum");
                auto native = at<void (__fastcall*)(void*, int, std::int32_t, std::int32_t, float)>(cc13_leaf_007b8140);
                alignas(16) float w1[24], w2[24];
                for (int i = 0; i < 3000; ++i) {
                    for (int k = 0; k < 24; ++k) w1[k] = w2[k] = edge_or(-50, 50);
                    const std::int32_t count = static_cast<std::int32_t>(next() % 22);
                    const std::int32_t index = count > 0 ? static_cast<std::int32_t>(next() % count) : 0;
                    const float sample = edge_or(-50, 50);
                    native(w1, 0, index, count, sample);
                    bsp::cc13_seg47::float_window_set_sample_and_sum_007b8140(bsp::cc13_seg47::raw(w2), index, count, sample);
                    record(c, cmp(w1, w2, 24));
                }
            }
            {
                auto& c = open7("007BA450 max3");
                auto native = at<float (__fastcall*)(const float*, const float*, const float*)>(cc13_leaf_007ba450);
                auto& d = open7("00AB0FE0 gui colour equal");
                auto native2 = at<int (__fastcall*)(const float*, int, const float*)>(cc13_leaf_00ab0fe0);
                for (int i = 0; i < 3000; ++i) {
                    float v[3];
                    for (float& x : v) x = edge_or(-10, 10);
                    if (i % 10 == 0) v[1] = v[0];
                    float n = native(&v[0], &v[1], &v[2]), r = bsp::cc13_seg47::max3_007ba450(&v[0], &v[1], &v[2]);
                    record(c, cmp(&n, &r, 1));
                    float a[4], b[4];
                    for (int k = 0; k < 4; ++k) { a[k] = edge_or(0, 1); b[k] = next() % 4 ? a[k] : edge_or(0, 1); }
                    if (i % 7 == 0) b[next() % 4] = -0.0f * a[0];
                    d.check((native2(a, 0, b) != 0) == bsp::cc13_gui1::gui_color_equal_00ab0fe0(a, b));
                }
            }
        };
        for (const unsigned pc : {static_cast<unsigned>(_PC_53), static_cast<unsigned>(_PC_24)}) {
            _controlfp_s(&control, pc | _RC_NEAR, _MCW_PC | _MCW_RC);
            state = 0x7f0c13a7u;
            const std::string tag = pc == _PC_53 ? " @53" : " @24";
            run_fixture7([&](const char* name) -> Case& { return open((std::string(name) + tag).c_str()); });
        }
        _controlfp_s(&control, _PC_53 | _RC_NEAR, _MCW_PC | _MCW_RC);
    }
    _controlfp_s(&control, _PC_53 | _RC_NEAR, _MCW_PC | _MCW_RC);
    // cc13-fixture2 cases carry a precision suffix. The pass that decides is the precision of
    // the thread the image runs the routine on (docs/CC13_FPU_PRECISION.md): 24-bit for the
    // main thread, 53-bit only for routines reached from worker threads alone (listed here).
    // The documented findings of docs/CC13_FIXTURE_PASS_2.md are pinned: a deciding pass fails
    // when its mismatch or NaN sign/payload-only count differs from the pinned one (0 for an
    // unlisted routine), so a new mismatch or a fixed one both show up.
    // cc13-fixture3: the double support mappings 00C38470 and 00C38810 are reached only from
    // the GJK routines 00C51C20 / 00C51EF0 of the narrow-phase intersectors (Dyn workers).
    static const char* const primary53[] = {"00C37860", "00C38470", "00C38810", "00C38970"};
    struct Pinned { const char* address; int failures; int nan_only; const char* finding; };
    static const Pinned pinned[] = {
        {"004F3DA0", 57, 0, "float intermediates overflow where the x87 stack does not"},
        {"00B64470", 0, 7, "NaN sign/payload only"},
        {"00B62DD0", 0, 54, "NaN sign/payload only"},
        {"00941AA0", 0, 10, "NaN sign/payload only"},
        {"00A8B860", 0, 1, "NaN sign/payload only"},
        {"00B75580", 0, 1, "NaN sign/payload only"},
        // cc13-fixture3 (docs/CC13_FIXTURE_PASS_3.md).
        {"00C47AA0", 0, 1, "NaN sign/payload only"},
        {"00C38810", 0, 4, "NaN sign/payload only"},
        // cc13-fixture5: edge inputs only (FLT_MAX, infinities, denormals); clean without them.
        {"006FBCE0", 55, 0, "float intermediates overflow where the x87 stack does not"},
        {"0078FAF0", 4, 1, "float intermediates denormalize where the x87 stack does not"},
        {"004F3030", 2, 0, "float intermediates denormalize where the x87 stack does not"},
        {"004F40C0", 34, 0, "float intermediates overflow where the x87 stack does not"},
        {"004F2FD0", 5, 0, "float intermediates underflow where the x87 stack does not"},
        {"009B94E0", 40, 0, "float intermediates overflow or denormalize where the x87 stack does not"},
    };
    int failures = 0;
    for (int i = 0; i < count; ++i) {
        const std::string& name = cases[i].name;
        const bool at24 = name.size() > 4 && name.compare(name.size() - 4, 4, " @24") == 0;
        const bool tagged = at24 || (name.size() > 4 && name.compare(name.size() - 4, 4, " @53") == 0);
        bool wants53 = false;
        for (const char* p : primary53) wants53 = wants53 || name.compare(0, 8, p) == 0;
        const bool primary = !tagged || at24 != wants53;
        // The pinned counts are for the default input set; without edge values every
        // deciding pass must match outright.
        static const bool edges = GetEnvironmentVariableA("BSP_FIXTURE_NO_EDGES", nullptr, 0) == 0;
        const Pinned* pin = nullptr;
        for (const auto& k : pinned) if (edges && name.compare(0, 8, k.address) == 0) pin = &k;
        const int want_failures = primary && pin ? pin->failures : 0;
        const int want_nan_only = primary && pin ? pin->nan_only : 0;
        const bool off = primary && (cases[i].failures != want_failures || cases[i].nan_only != want_nan_only);
        std::cout << name << ": " << cases[i].runs - cases[i].failures - cases[i].nan_only << '/' << cases[i].runs;
        if (cases[i].nan_only) std::cout << " (+" << cases[i].nan_only << " NaN sign/payload only)";
        if (!cases[i].failures && !off) std::cout << " match";
        else if (!primary) std::cout << " mismatch (other precision, informational)";
        else if (!off) std::cout << " MISMATCH (pinned finding: " << pin->finding << ')';
        else std::cout << " MISMATCH (expected " << want_failures << " mismatches, " << want_nan_only << " NaN-only)";
        std::cout << (primary && tagged ? " [primary]\n" : "\n");
        failures += off;
    }
    std::cout << count << " cc13 leaf cases; " << failures << " with mismatches\n";
    return failures ? 1 : 0;
}

namespace {
#pragma warning(push)
#pragma warning(disable : 4100 4459 4189)
// cc13-fixture8 (docs/CC13_FIXTURE_PASS_8.md): small main-thread float leaves. Runs once per
// precision control; the caller resets the generator so both passes see the same inputs.
template<class Open> void run_fixture8(Open open) {
    using bsp::cc13_seg62::Object;
    auto fbits = [](float v) { std::uint32_t u; std::memcpy(&u, &v, 4); return u; };
    auto put = [](unsigned char* base, unsigned off, auto v) { std::memcpy(base + off, &v, sizeof(v)); };
    auto get_f = [](const unsigned char* base, unsigned off) { float v; std::memcpy(&v, base + off, 4); return v; };
    {   auto& c = open("00809DE0 rocket aim lift");
        auto native = at<float (__fastcall*)(void*)>(cc13_leaf_00809de0);
        for (int i = 0; i < 2000; ++i) {
            alignas(4) unsigned char cls[0xe4]{};
            const float e0 = edge_or(-20, 20);
            put(cls, 0xe0, e0);
            const float a = native(cls), b = bsp::rocket_aim_lift_00809de0(e0);
            if (nan_only_diff(&a, &b, 1)) { c.check_nan_only(); continue; }
            check_note(c, same(a, b), "e0=%a %a / %a", e0, a, b);
        }
    }
    {   // 007B5390: [approach+D4h] is the 00809E10 record; +9Ch takes the clamped flight time.
        auto& c = open("007B5390 rocket projtime");
        static float launch = 0;
        struct Launcher { static float __fastcall speed(void*, void*) { return launch; } };
        void* vtable[16]{};
        vtable[0x38 / 4] = reinterpret_cast<void*>(&Launcher::speed);
        void* object[1] = {vtable};
        alignas(4) unsigned char rocket[0xe0]{}, record[0x320]{}, approach[0xd8]{};
        void* object_pointer = object; void* rocket_pointer = rocket; void* record_pointer = record;
        std::memcpy(record + 0x3c, &object_pointer, 4);
        std::memcpy(record + 0x314, &rocket_pointer, 4);
        std::memcpy(approach + 0xd4, &record_pointer, 4);
        auto flight = at<float (__fastcall*)(void*, int, float)>(cc13_leaf_00809e10);
        auto native = at<void (__fastcall*)(void*, int, float)>(cc13_leaf_007b5390);
        for (int i = 0; i < 2000; ++i) {
            launch = edge_or(0, 300);
            put(rocket, 0xd8, edge_or(launch - 60, 400));
            put(rocket, 0xdc, edge_or(1, 120));
            const float range = edge_or(-500, 9000);
            put(approach, 0x9c, 7.0f);
            native(approach, 0, range);
            const float a = get_f(approach, 0x9c);
            const float b = bsp::rocket_projtime_007b5390(flight(record, 0, range));
            if (nan_only_diff(&a, &b, 1)) { c.check_nan_only(); continue; }
            check_note(c, same(a, b), "range=%a %a / %a", range, a, b);
        }
    }
    {   auto& c = open("00791640 orbit input track tick");
        auto native = at<void (__fastcall*)(void*, int, float)>(cc13_leaf_00791640);
        for (int i = 0; i < 2000; ++i) {
            alignas(4) unsigned char raw[0x2c]{};
            const float d = edge_or(-400, 400);
            put(raw, 0x18, d);
            put(raw, 0x1c, 123.0f);
            native(raw, 0, 0.0f);
            bsp::cc13cam::OrbitInputModifier m;
            m.distance_18 = d;
            m.distance_rate_1c = 123.0f;
            bsp::cc13cam::orbit_input_track_tick_00791640(m);
            const float a = get_f(raw, 0x1c), b = m.distance_rate_1c;
            check_note(c, same(a, b), "d=%a %a / %a", d, a, b);
        }
    }
    {   auto& c = open("0059AB10 HUD map animate zoom");
        auto native = at<void (__fastcall*)(void*, int, float)>(cc13_leaf_0059ab10);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char s1[0x90]{}, s2[0x90]{};
            const float zoom = i % 7 == 0 ? (i % 2 ? 1.0f : 0.0f) : edge_or(-0.5f, 1.5f);
            const float dt = i % 11 == 0 ? 0.0f : edge_or(0, 0.2f);
            const std::uint8_t closing = static_cast<std::uint8_t>(next() & 1);
            put(s1, 0x84, zoom); put(s1, 0x88, closing);
            std::memcpy(s2, s1, sizeof(s1));
            native(s1, 0, dt);
            bsp::cc13_fe1::hud_map_animate_zoom_0059ab10(s2, dt);
            const float a = get_f(s1, 0x84), b = get_f(s2, 0x84);
            if (s1[0x88] == s2[0x88] && nan_only_diff(&a, &b, 1)) { c.check_nan_only(); continue; }
            check_note(c, std::memcmp(s1, s2, sizeof(s1)) == 0, "zoom=%a dt=%a closing=%d %a / %a",
                       zoom, dt, closing, a, b);
        }
    }
    {   auto& c = open("00790460 MovCam snapshot tracked fields");
        auto native = at<void (__fastcall*)(void*)>(cc13_leaf_00790460);
        static const unsigned from[8] = {0x224, 0x228, 0x22c, 0x254, 0x258, 0x2f0, 0x278, 0x2f8};
        for (int i = 0; i < 1000; ++i) {
            alignas(4) static unsigned char cam[0x390];
            for (unsigned k = 0; k < sizeof(cam); k += 4) put(cam, k, edge_or(-1e4f, 1e4f));
            bsp::cc13cam2::MovCamTrackedSnapshot in;
            in.from_224 = {fbits(get_f(cam, 0x224)), fbits(get_f(cam, 0x228)), fbits(get_f(cam, 0x22c))};
            in.from_254 = fbits(get_f(cam, 0x254)); in.from_258 = fbits(get_f(cam, 0x258));
            in.from_2f0 = fbits(get_f(cam, 0x2f0)); in.from_278 = fbits(get_f(cam, 0x278));
            in.from_2f8 = fbits(get_f(cam, 0x2f8));
            std::array<std::uint32_t, 8> out{};
            bsp::cc13cam2::movcam_snapshot_00790460(out, in);
            native(cam);
            bool ok = true;
            for (int k = 0; k < 8; ++k) ok = ok && fbits(get_f(cam, 0x370 + 4 * k)) == out[k];
            static_cast<void>(from);
            check_note(c, ok, "case %d", i);
        }
    }
    {   auto& c = open("00414130 lerp");
        auto native = at<float (__fastcall*)(const float*, const float*, const float*)>(cc13_leaf_00414130);
        for (int i = 0; i < 3000; ++i) {
            const float a = edge_or(-100, 100), b = edge_or(-100, 100), t = edge_or(-0.5f, 1.5f);
            const float o1 = native(&a, &b, &t);
            const float o2 = bsp::engine_seg1_cc13::math_lerp_00414130(&a, &b, &t);
            if (nan_only_diff(&o1, &o2, 1)) { c.check_nan_only(); continue; }
            check_note(c, same(o1, o2), "a=%a b=%a t=%a %a / %a", a, b, t, o1, o2);
        }
    }
    {   auto& c = open("009C6220 clamp symmetric");
        auto native = at<float (__fastcall*)(const float*, const float*)>(cc13_leaf_009c6220);
        for (int i = 0; i < 3000; ++i) {
            const float a = edge_or(-50, 50), b = edge_or(-50, 50);
            const float o1 = native(&a, &b), o2 = bsp::math_clamp_symmetric_009c6220(&a, &b);
            check_note(c, same(o1, o2), "a=%a b=%a %a / %a", a, b, o1, o2);
        }
    }
    {   auto& c = open("0088A0C0 scored median of three");
        auto native = at<void (__fastcall*)(bsp::cc13_left55::ScoredEntry*, bsp::cc13_left55::ScoredEntry*,
                                            bsp::cc13_left55::ScoredEntry*)>(cc13_leaf_0088a0c0);
        for (int i = 0; i < 3000; ++i) {
            bsp::cc13_left55::ScoredEntry e1[3], e2[3];
            for (int k = 0; k < 3; ++k) e1[k] = {next(), i % 9 == 0 ? static_cast<float>(next() % 3) : edge_or(-10, 10)};
            std::memcpy(e2, e1, sizeof(e1));
            native(&e1[0], &e1[1], &e1[2]);
            bsp::cc13_left55::scored_med3_0088a0c0(&e2[0], &e2[1], &e2[2]);
            check_note(c, std::memcmp(e1, e2, sizeof(e1)) == 0, "case %d", i);
        }
    }
    {   auto& c = open("0098A710 node bounds");
        auto native = at<void (__fastcall*)(void*, int, float*, float*)>(cc13_leaf_0098a710);
        for (int i = 0; i < 1000; ++i) {
            alignas(4) unsigned char node[0x124]{};
            for (unsigned k = 0x10c; k < 0x124; k += 4) put(node, k, edge_or(-1e4f, 1e4f));
            float mn1[3], mx1[3], mn2[3], mx2[3];
            native(node, 0, mn1, mx1);
            bsp::cc13_seg62::node_bounds_0098a710(static_cast<Object>(node), mn2, mx2);
            check_note(c, std::memcmp(mn1, mn2, 12) == 0 && std::memcmp(mx1, mx2, 12) == 0, "case %d", i);
        }
    }
    {   auto& c = open("00638B20 HUD marker endpoints");
        auto native = at<void (__fastcall*)(void*, int, float*, float*)>(cc13_leaf_00638b20);
        for (int i = 0; i < 1000; ++i) {
            float a[3], b[3];
            for (int k = 0; k < 3; ++k) { a[k] = edge_or(-1e4f, 1e4f); b[k] = edge_or(-1e4f, 1e4f); }
            alignas(4) unsigned char marker[0x28]{};
            const float* pa = a; const float* pb = b;
            std::memcpy(marker + 0x20, &pa, 4); std::memcpy(marker + 0x24, &pb, 4);
            float oa1[3], ob1[3], oa2[3], ob2[3];
            native(marker, 0, oa1, ob1);
            bsp::ingame_gui_cc13::hud_marker_endpoints_00638b20(bsp::ingame_gui_cc13::Raw{marker}, oa2, ob2);
            check_note(c, std::memcmp(oa1, oa2, 12) == 0 && std::memcmp(ob1, ob2, 12) == 0, "case %d", i);
        }
    }
    {   auto& c = open("0042C450 camera clamp zoom");
        auto native = at<void (__fastcall*)(void*, int, float)>(cc13_leaf_0042c450);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char m1[0x1c8]{}, m2[0x1c8]{};
            const float cur = edge_or(-10, 100), high = edge_or(0, 100), low = edge_or(-10, 100);
            put(m1, 0x1c0, cur); put(m1, 0x1c4, high);
            std::memcpy(m2, m1, sizeof(m1));
            native(m1, 0, low);
            bsp::cc13_sweepA::camera_clamp_zoom_0042c450(bsp::cc13_sweepA::Raw{m2}, low);
            check_note(c, std::memcmp(m1, m2, sizeof(m1)) == 0, "cur=%a high=%a low=%a", cur, high, low);
        }
    }
    {   auto& c = open("007B5BE0 unit largest extent");
        auto native = at<float (__fastcall*)(void*)>(cc13_leaf_007b5be0);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char unit[0x44c]{};
            for (unsigned off : {0x434u, 0x444u, 0x448u}) put(unit, off, edge_or(0, 5000));
            const float a = native(unit), b = bsp::cc13_seg47::unit_largest_extent_007b5be0(bsp::cc13_seg47::Raw{unit});
            check_note(c, same(a, b), "%a / %a", a, b);
        }
    }
    {   auto& c = open("006EB870 ratio |+DCh| / +8Ch");
        auto native = at<float (__fastcall*)(void*)>(cc13_leaf_006eb870);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char cls[0xe0]{};
            put(cls, 0x8c, edge_or(-10, 100)); put(cls, 0xdc, edge_or(-1000, 1000));
            const float a = native(cls), b = bsp::cc13_seg33a::ratio_abs_dc_over_8c_006eb870(cls);
            if (nan_only_diff(&a, &b, 1)) { c.check_nan_only(); continue; }
            check_note(c, same(a, b), "dc=%a 8c=%a %a / %a", get_f(cls, 0xdc), get_f(cls, 0x8c), a, b);
        }
    }
    {   auto& c = open("009B93F0 tight turn slot 68h");
        auto native = at<float (__fastcall*)(void*)>(cc13_leaf_009b93f0);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char inner[0x1b0]{}, task[0x2fc]{};
            void* p = inner;
            std::memcpy(task + 0x2f8, &p, 4);
            put(inner, 0x1ac, edge_or(-100, 100));
            const float a = native(task), b = bsp::bot_task_tight_turn_slot68_009b93f0(task);
            if (nan_only_diff(&a, &b, 1)) { c.check_nan_only(); continue; }
            check_note(c, same(a, b), "v=%a %a / %a", get_f(inner, 0x1ac), a, b);
        }
    }
    {   auto& c = open("00B0A3F0 tracer bounding sphere");
        auto native = at<float* (__stdcall*)(float*, const void*, int)>(cc13_leaf_00b0a3f0);
        for (int i = 0; i < 1000; ++i) {
            float state[3], s1[4] = {1, 2, 3, 4}, s2[4] = {1, 2, 3, 4};
            for (float& v : state) v = edge_or(-1e4f, 1e4f);
            const float* r1 = native(s1, state, 0);
            const float* r2 = bsp::particle_emitters::tracer_bounding_sphere_00b0a3f0(s2, state);
            check_note(c, r1 == s1 && r2 == s2 && std::memcmp(s1, s2, 16) == 0, "case %d", i);
        }
    }
    {   auto& c = open("006EA530 torpedo class scaled +E4h");
        auto native = at<float (__fastcall*)(void*)>(cc13_leaf_006ea530);
        for (int i = 0; i < 3000; ++i) {
            alignas(4) unsigned char cls[0xe8]{};
            put(cls, 0xe4, edge_or(-1000, 1000));
            const float a = native(cls), b = bsp::cc13_seg33a::torpedo_class_scaled_e4_006ea530(cls);
            if (nan_only_diff(&a, &b, 1)) { c.check_nan_only(); continue; }
            check_note(c, same(a, b), "v=%a %a / %a", get_f(cls, 0xe4), a, b);
        }
    }
}
#pragma warning(pop)
}  // namespace
