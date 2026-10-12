// Execute audited cc13 leaf routines from the original image beside their reconstructions.
// The bytes come from local/cc13_leaf_reference.hpp (tools/verify_cc13_leaf_reference.py), which
// relocates constants, BSS buffers and the few internal calls into one private copy. No game
// process, assets or original addresses are involved. One case per routine; inputs are a fixed
// pseudo-random set, compared bit for bit. The x87 control word is pinned to 53-bit precision,
// the CRT default; the game's main thread runs at 24 bits after device creation
// (docs/CC13_FPU_PRECISION.md), which these cases do not cover.
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
#include "bsp/rocket_task_cc13.hpp"
#include "cc13_leaf_reference.hpp"
#include <float.h>
#include <cmath>
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
    const char* name;
    int runs = 0, failures = 0;
    void check(bool ok) { ++runs; if (!ok) ++failures; }
};

}  // namespace

int main() {
    unsigned control{};
    _controlfp_s(&control, _PC_53 | _RC_NEAR, _MCW_PC | _MCW_RC);
    load_image();
    Case cases[24]{};
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
    int failures = 0;
    for (int i = 0; i < count; ++i) {
        std::cout << cases[i].name << ": " << cases[i].runs - cases[i].failures << '/' << cases[i].runs
                  << (cases[i].failures ? " MISMATCH\n" : " match\n");
        failures += cases[i].failures != 0;
    }
    std::cout << count << " cc13 leaf cases; " << failures << " with mismatches\n";
    return failures ? 1 : 0;
}
