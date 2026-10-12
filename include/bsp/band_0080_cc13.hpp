#pragma once

// Non-gameplay leaves from 00800000-008FFFFF reconstructed by cc13-sweep8E:
// the scene-text escape and hex byte decoder used by the scene property
// writer/parser, the segment-triangle test, the effect component category
// table and the objective-sync message type ids. Names are hypotheses, not
// recovered symbols. Evidence, ABIs and open questions are in
// docs/CC13_BAND_0080_008F.md. None is a drop-in binary replacement: these are
// C++ interfaces over models of the native data.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast, and the intermediates in between are modelled as double.

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace bsp::cc13_band0080 {

// 008EF6A0, __fastcall(CL = character) -> int. '0'-'9', 'a'-'f' and 'A'-'F'
// give 0..15; anything else gives 0.
int hex_digit_value_008ef6a0(char c) noexcept;

// 008EF6D0, __fastcall(ECX = const char* token) -> AL. Decodes one `BIN`
// (type 8) token of a scene property file. A null or empty token gives 0. The
// first character is a hex digit (0 when it is not one); when a second
// character exists the result is (first << 4) | digit(second), truncated to a
// byte. Characters after the second are ignored.
std::uint8_t decode_hex_byte_token_008ef6d0(const char* token) noexcept;

// 008EE5F0, __fastcall(ECX = text, EDX = max characters) -> char*. Copies at
// most `max_characters` input characters, writing newline as "\n", double
// quote as "\q" and backslash as "\\". The image writes into the static
// buffer 00F88C50 without a bound and returns it; the model returns a string.
// A null text or a zero count gives the empty string.
std::string escape_scene_text_008ee5f0(const char* text, std::int32_t max_characters);

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// 0085D020, __fastcall(ECX = p0, EDX = p1, [esp+4] a, [esp+8] b, [esp+0Ch] c,
// [esp+10h] out) -> AL, RET 10h. Segment p0..p1 against triangle a, b, c
// (Moller-Trumbore order: h = d x e2, det = e1 . h). Rejects when
// -1e-4 < det < 1e-4 (doubles 00D0D098 / 00D7A268), when u is outside [0, 1]
// (upper bound float 00D7A24C), when v < 0 or u + v > 1, and when t is
// outside [0, 1]. On a hit writes p0 + t * (p1 - p0) and returns true; on a
// miss `out` is untouched. Sole caller BSP_GeomMeshElement_TraceSegmentNode.
bool segment_intersect_triangle_0085d020(const Vec3f& p0, const Vec3f& p1, const Vec3f& a,
                                         const Vec3f& b, const Vec3f& c, Vec3f& out) noexcept;

// Slot +10h of each effect component vtable (docs/GAMEPLAY_EFFECT_COMPONENTS.md
// gives the vtables): a constant per component type.
struct EffectComponentCategory {
    std::string_view component;
    std::uint32_t vtable;
    std::uint32_t slot10_address;
    std::int32_t category;
};

inline constexpr std::array<EffectComponentCategory, 13> kEffectComponentCategories{{
    {"Particle", 0x00D0D5B4, 0x0086BC50, 1},
    {"Sound", 0x00D0DA18, 0x0086FA80, 2},
    {"Tracer", 0x00D0D590, 0x0086B990, 3},
    {"WaterTracer", 0x00D0D5D4, 0x0086BD10, 4},
    {"Flare", 0x00D0D614, 0x0086BDB0, 5},
    {"Shake", 0x00D0D5F4, 0x0086BD60, 6},
    {"ConstRumble", 0x00D0D76C, 0x0086CFC0, 6},
    {"SlopeRumble", 0x00D0D78C, 0x0086D010, 6},
    {"SquareRumble", 0x00D0D7AC, 0x0086D060, 6},
    {"ThunderStorm", 0x00D0DA38, 0x0086FC20, 0},
    {"Waterdrops", 0x00D0D634, 0x0086BDF0, 0},
    {"Light", 0x00D0D654, 0x0086BE30, 0},
    {"Splash", 0x00D0D674, 0x0086BE70, 0},
}};

// Category for a component name, or -1 for a name not in the table.
std::int32_t effect_component_category(std::string_view component) noexcept;

// Objective-sync message types (the MT_ name table at 00E0AB68), constructed
// by 008E1380 and 008DD6A0..008DDB00 and routed on receipt by 008E2390.
enum class ObjectiveSyncMessage : std::uint8_t {
    Create = 0x37,          // MT_OBJ_CREATE, SessionMessage55
    Add = 0x38,             // MT_OBJ_ADD, SessionMessage56
    AddEntity = 0x39,       // MT_OBJ_ADD_ENTITY, SessionMessage57
    RemoveEntity = 0x3A,    // MT_OBJ_REMOVE_ENTITY, SessionMessage58
    AddPosition = 0x3B,     // MT_OBJ_ADD_POSITION, SessionMessage59
    RemovePosition = 0x3C,  // MT_OBJ_REMOVE_POSITION, SessionMessage60
    Completed = 0x3D,       // MT_OBJ_COMPLETED, SessionMessage61
    Failed = 0x3E,          // MT_OBJ_FAILED, SessionMessage62
};

// True for the eight type ids 008E2390 tests (37h..3Eh).
constexpr bool is_objective_sync_message(std::uint8_t type) noexcept
{
    return type >= 0x37 && type <= 0x3E;
}

} // namespace bsp::cc13_band0080
