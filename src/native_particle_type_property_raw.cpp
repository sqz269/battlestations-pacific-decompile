#include "bsp/native_particle_type_property_raw.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_particle_type_resources.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle common properties require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word word(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
const char* text(const void* header) noexcept { return reinterpret_cast<const char*>(word(header)); }
void return_captured(const char* captured, NativeStringRawPoolContext& strings) {
    if (captured) release_native_pooled_text_bytes_00aee1e0(const_cast<char*>(captured), strings);
}
void unwind(NativeParticleTypePropertyRawAcquired& a, NativeStringRawPoolContext& strings) noexcept {
    static constexpr int previous[]{-1, -1, 1, -1, -1};
    try {
        while (a.unwind_state >= 0) {
            const int state = a.unwind_state;
            a.unwind_state = previous[state];
            void* const header = state == 2 || state == 3
                ? static_cast<void*>(&a.native_local10) : static_cast<void*>(&a.native_argument28);
            destroy_native_pooled_text_00aee2a0(header, strings);
        }
    } catch (...) { std::terminate(); }
}
Word capture_shader(void* definition, NativeParticleTypePropertyRawContext& c) {
    const volatile Word* table = nullptr;
    switch (word(definition)) {
    case 0x00d5dd18u: table = c.sprite_profile_00d5dd18; break;
    case 0x00d5dcc0u: table = c.axial_profile_00d5dcc0; break;
    case 0x00d5dcecu: table = c.floating_profile_00d5dcec; break;
    case 0x00d5db00u: table = c.object_profile_00d5db00; break;
    case 0x00d5e048u: table = c.tracer_profile_00d5e048; break;
    default: throw std::invalid_argument("Unknown current particle definition shader profile");
    }
    if (!table) throw std::invalid_argument("Missing current particle definition profile cells");
    return table[0x10 / 4];
}
struct Property {
    const char* name;
    Word offset, token0_site, compare_site, token1_site, value_site, clear_site;
};
constexpr Property properties[]{
    {"Texture",0,0x00b015eb,0x00b015f8,0x00b0163d,0x00b0164f,0x00b01689},
    {"Layer",0,0x00b016ac,0x00b016b9,0x00b01702,0x00b0176c,0x00b017a9},
    {"Shader",0,0x00b017cc,0x00b017d9,0x00b0181e,0x00b01835,0x00b01843},
    {"Stops",0x28,0x00b01868,0x00b01875,0x00b01898,0x00b018a0,0x00b018b4},
    {"RandomRotationDirection",0x29,0x00b018d3,0x00b018e0,0x00b01903,0x00b0190b,0x00b0191f},
    {"EmitLight",0x64,0x00b0193e,0x00b0194a,0x00b01967,0x00b0196f,0x00b01983},
    {"Distort",0x65,0x00b019a2,0x00b019ae,0x00b019cb,0x00b019d3,0x00b019e7},
    {"AnimOnOff",0x4c,0x00b01a08,0x00b01a14,0x00b01a31,0x00b01a39,0x00b01a4d},
    {"AnimRndStartFrame",0x60,0x00b01a6c,0x00b01a78,0x00b01a95,0x00b01a9d,0x00b01ab1},
    {"AnimRandomPlay",0x61,0x00b01ad0,0x00b01adc,0x00b01af9,0x00b01b01,0x00b01b15},
    {"AnimLoop",0x62,0x00b01b36,0x00b01b42,0x00b01b5f,0x00b01b67,0x00b01b7b},
    {"AnimFade",0x63,0x00b01b9a,0x00b01ba6,0x00b01bc3,0x00b01bcb,0x00b01bdf}
};
constexpr Word key_returns[]{0x00b0162b,0x00b016ec,0x00b0180c,0x00b01886,
    0x00b018f1,0x00b01955,0x00b019b9,0x00b01a1f,0x00b01a83,0x00b01ae7,0x00b01b4d,0x00b01bb1};
} // namespace

bool load_native_particle_type_property_00b015c0(void* definition, const void* suffix,
    NativeParticleTypePropertyRawContext& c, NativeParticleTypePropertyRawAcquired& a) {
    using Phase = NativeParticleTypePropertyRawAcquired::Phase;
    if (a.phase != Phase::fresh)
        throw std::logic_error("Particle property operation cannot replay");
    a.phase = Phase::running;
    auto& strings = c.strings;
    try {
        for (unsigned i = 0; i != 12; ++i) {
            const auto& p = properties[i];
            a.native_site = p.token0_site;
            void* const key = get_native_pooled_text_token_00aee3c0(suffix, &a.native_argument28, 0, strings);
            a.native_site = p.compare_site;
            const bool match = i < 5 ? _stricmp(text(key), p.name) == 0
                : equal_native_pooled_text_00aedf80(key, p.name);
            a.native_site = key_returns[i];
            if (i < 3) return_captured(text(&a.native_argument28), strings);
            else destroy_native_pooled_text_00aee2a0(&a.native_argument28, strings);
            if (!match) continue;

            void* const value_header = i == 11 ? static_cast<void*>(&a.native_local10)
                : static_cast<void*>(&a.native_argument28);
            a.native_site = p.token1_site;
            void* const returned = get_native_pooled_text_token_00aee3c0(suffix, value_header, 1, strings);
            const char* const value = text(returned);
            if (i == 0) {
                a.unwind_state = 0;
                a.native_site = p.value_site;
                if (!c.texture) throw std::invalid_argument("Texture property requires its actual provider context");
                (void)load_native_particle_type_texture_00b01350(definition, value, *c.texture, a.texture);
                const char* const captured = text(&a.native_argument28);
                a.unwind_state = -1;
                a.native_site = p.clear_site;
                return_captured(captured, strings);
            } else if (i == 1) {
                a.unwind_state = 1;
                a.native_site = 0x00b01716;
                construct_native_pooled_text_00af5660(&a.native_local10, value, strings);
                const char* const token = text(&a.native_argument28);
                a.unwind_state = 3;
                a.native_site = 0x00b01749;
                return_captured(token, strings);
                const char* const captured = text(&a.native_local10); // Saved ESI before child calls.
                a.captured_layer = captured;
                a.native_local14 = reinterpret_cast<Word>(a.consumed_name8h);
                a.native_argument28 = 0; // B0175C, before by-value construction.
                a.native_site = 0x00b01764;
                construct_native_string_header_0041e870(a.consumed_name8h, strings, captured);
                void* const owner = reinterpret_cast<void*>(word(at(definition, 0x14)));
                a.native_site = p.value_site;
                const auto found = find_native_particle_layer_00af4360(owner, a.consumed_name8h, strings);
                *static_cast<volatile Word*>(at(definition, 0x74)) = static_cast<Word>(found);
                a.unwind_state = -1;
                a.native_site = p.clear_site;
                return_captured(captured, strings);
            } else if (i == 2) {
                a.native_site = 0x00b01828;
                const Word target = capture_shader(definition, c);
                a.unwind_state = 4;
                a.native_site = p.value_site;
                if (!dispatch_known_native_particle_shader(definition, target, value, strings))
                    throw std::invalid_argument("Unknown captured particle shader slot10 target");
                a.unwind_state = -1;
                a.native_site = p.clear_site;
                destroy_native_pooled_text_00aee2a0(&a.native_argument28, strings);
            } else {
                a.native_site = p.value_site;
                const auto parsed = std::atol(value); // Win32 signed LONG, original CRT boundary.
                *static_cast<volatile std::uint8_t*>(at(definition, p.offset)) = parsed > 0 ? 1u : 0u;
                a.native_site = p.clear_site;
                destroy_native_pooled_text_00aee2a0(value_header, strings);
            }
            a.phase = Phase::complete;
            return true;
        }
        a.phase = Phase::complete;
        return false;
    } catch (...) {
        a.phase = Phase::failed;
        unwind(a, strings);
        throw;
    }
}
} // namespace bsp
