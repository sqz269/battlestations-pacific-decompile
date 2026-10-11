#pragma once

// Ocean surface rendering: the authored g_Ocean parameters, the projected-grid
// geometry and the per-frame wave scale of the 6BCh-byte ocean object that
// 00BA6FB0 allocates (constructor 00BA6C40, base 00BA1400) and the world keeps
// at world+A8h. Packet cc13_water1, docs/CC13_WATER_RENDER.md.
//
// These are semantic C++ interfaces for MSVC Win32, not binary replacements.
// Names are hypotheses. Offsets are byte offsets into the native object or into
// the scene record's +990h environment block ("block"). The x87 routines keep
// float operands and round at the native stores, but they are not bit-exact:
// native intermediates are 80-bit unless a store spills them.
//
// Nothing here is wired into the host. In particular ocean_wave_scale_00ba0b32
// documents the per-frame amplitude store (00BA0B93) that 0078C890's water
// height reads; ship physics owns that path (cc9), not this file.

#include <cstdint>

namespace bsp::water_render {

struct Float3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct Float4 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float w{0.0f};
};

// ---------------------------------------------------------------------------
// Authored parameters.
//
// 004EB9B0 reads the scene file's "g_Ocean" property group into record+990h..
// (the block). The g_Ocean subgroups and their block offsets, from that
// reader's key strings and stores (rec+off - 990h):
namespace block {
// Foam
constexpr std::uint32_t kFoamScrollSpeed = 0x000;        // ScrollSpeed
constexpr std::uint32_t kFoamNoiseScrollSpeed = 0x004;   // NoiseScrollSpeed
constexpr std::uint32_t kFoamFadeRange = 0x008;          // FadeRange
constexpr std::uint32_t kFoamUseFoam = 0x00C;            // UseFoam (byte)
constexpr std::uint32_t kFoamHeight = 0x010;             // Height
constexpr std::uint32_t kFoamScale = 0x014;              // Scale
constexpr std::uint32_t kFoamNoiseScale = 0x018;         // NoiseScale
constexpr std::uint32_t kFoamTexture = 0x01C;            // Texture (string)
constexpr std::uint32_t kFoamNoiseTexture = 0x024;       // NoiseTexture (string)
// Waves
constexpr std::uint32_t kWavesFalloff = 0x02C;
constexpr std::uint32_t kWavesLayerScale = 0x030;
constexpr std::uint32_t kWavesTimeScale = 0x034;
constexpr std::uint32_t kWaveDampeningDepth = 0x038;     // 1WaveDampeningDepth
constexpr std::uint32_t kWaveDampeningPower = 0x03C;     // 1WaveDampeningPower
constexpr std::uint32_t kWaveDampeningScale = 0x040;     // 1WaveDampeningScale
constexpr std::uint32_t kWaveDampModel = 0x044;          // 1WaveDampModel (string)
constexpr std::uint32_t kWavesWindDirection = 0x04C;
constexpr std::uint32_t kWavesWindSpeed = 0x050;
constexpr std::uint32_t kWavesWaveHeight = 0x054;
constexpr std::uint32_t kWavesChoppyWavesFactor = 0x058;
constexpr std::uint32_t kWavesHeightScale = 0x05C;
// Fog
constexpr std::uint32_t kUWExpDistMaxdist = 0x104;
constexpr std::uint32_t kUWFogColor = 0x118;             // R,G,B then MaxDarken +124h
// Caustics
constexpr std::uint32_t kCausticsScale = 0x128;
constexpr std::uint32_t kCausticsColor = 0x12C;          // R,G,B
// Optics
constexpr std::uint32_t kSkyReflection = 0x144;          // 7SkyRefl BaseFact, PowExp, PowDiv, DesatFact
constexpr std::uint32_t kLocalReflectionTint = 0x154;    // 1LocalReflectionTint R,G,B
constexpr std::uint32_t kReflectionTint = 0x164;         // 1ReflectionTint
constexpr std::uint32_t kRefractionTint = 0x174;         // 1RefractionTint
constexpr std::uint32_t kUWRefractionTint = 0x184;       // 1UWRefractionTint
constexpr std::uint32_t kUWGlow = 0x194;                 // 8UWBaseGlow, 8UWSkyGlow, 8UWSunSpot strengths
constexpr std::uint32_t kSprayTint = 0x1A0;              // 1SprayTintColor
constexpr std::uint32_t kShadowStrength = 0x1B0;         // 9ShadowStrength
constexpr std::uint32_t kRefractionTintExponent = 0x1B4; // 2RefractionTintExponent
constexpr std::uint32_t kRefractionTintPower = 0x1B8;
constexpr std::uint32_t kDeepWaterRefractionExponent = 0x1BC;
constexpr std::uint32_t kDeepWaterRefractionTintPower = 0x1C0;
constexpr std::uint32_t kRefractionMaxDepth = 0x1C4;     // 3RefractionMaxDepth
constexpr std::uint32_t kRefractionDepthFadeRange = 0x1C8;
constexpr std::uint32_t kFresnelExponent = 0x1CC;        // 4Fresnel Exponent, Scale +1D0h, Offset +1D4h
constexpr std::uint32_t kShoreFadeDepth = 0x1D8;         // 5ShoreFade Depth, Range +1DCh, Power +1E0h
constexpr std::uint32_t kRefrSpecularColor = 0x1E4;      // RefrSpecularColor R,G,B
constexpr std::uint32_t kUnderWaterRefrIndex = 0x1F4;
constexpr std::uint32_t kOceanReflCubemap = 0x210;       // ReflCubemap + "_ocean" + ".dds" (004EE450)
constexpr std::uint32_t kReflectionMultiplier = 0x22C;   // 6ReflectionMultiplier, 6FinalMultiplier +230h
constexpr std::uint32_t kNormalMapLayerScales = 0x260;   // NormalMaps.layer0..2_Scale
constexpr std::uint32_t kNormalMapScrollSpeeds = 0x270;  // NormalMaps.layer0..2_ScrollSpeed
constexpr std::uint32_t kNormalMapTexture = 0x280;       // NormalMaps.Texture (string)
}  // namespace block

// The ocean object's optics fields that 0078C9B0 fills from the block through
// the small setters 00B99E60..00B9A2C0 (field offsets in the member names).
struct OceanOpticsFields {
    Float4 sky_reflection_350;          // 00B99E90 <- block+144h
    Float3 underwater_glow_360;         // 00B99FC0 <- block+194h
    Float4 local_reflection_tint_36c;   // 00B99EC0 <- block+154h
    Float4 reflection_tint_37c;         // 00B99E60 <- block+164h
    Float4 refraction_tint_38c;         // 00B99F60 <- block+174h
    Float4 underwater_refraction_39c;   // 00B99F90 <- block+184h
    Float4 spray_tint_3dc;              // 00B99F00 <- block+1A0h
    Float4 refraction_specular_3fc;     // 00B9A180 <- block+1E4h
    float underwater_refraction_index_414{0.0f};  // 00B9A2C0 <- block+1F4h
};

// The 0078C9B0 setter calls for the optics fields, in the native order
// (00B99E90, 00B99EC0, 00B99F00, 00B99E60, 00B99F60, 00B99F90, 00B99FC0, then
// 00B9A180 and 00B9A2C0 after the system-parameter slots). block points at
// record+990h. Each setter is a plain dword copy.
void apply_authored_optics_0078c9b0(const std::uint8_t* block, OceanOpticsFields& fields);

// 00B9A3F0 `void __thiscall(field, const float scales[4], const float speeds[4])`,
// RET 8: +44h..+50h = scales, then the first three replaced by their
// reciprocals (FDIVR from 1.0), +54h..+60h = speeds. 0078C9B0 passes
// block+260h and block+270h.
struct NormalMapLayers {
    Float4 inverse_scale_44;   // w is the authored fourth dword, not inverted
    Float4 scroll_speed_54;
};
NormalMapLayers set_normal_map_layers_00b9a3f0(const float scales[4], const float speeds[4]);

// 00BA0090 at 00BA0792..00BA0870: the three normal-map scroll offsets, from
// the wind direction (cos +64h, sin +68h), the ocean clock T = [01090494] and
// each layer's 1/scale and speed: offset_i = (cos, sin) * T * speed_i * inv_i.
struct NormalMapOffsets {
    float layer0_6c{0.0f};
    float layer0_70{0.0f};
    float layer1_74{0.0f};
    float layer1_78{0.0f};
    float layer2_7c{0.0f};
    float layer2_80{0.0f};
};
NormalMapOffsets normal_map_scroll_00ba0792(const NormalMapLayers& layers, float wind_cos_64,
                                            float wind_sin_68, float ocean_clock);

// 00BA0090 prologue: [01090494] = float(10.0 * arg), arg being 0078CFF0's
// time argument times the double 0.1 at 00D7A3A0 (0078D184).
float ocean_clock_00ba00ad(float update_argument);

// 00B9A4C0 `void __thiscall(field, float falloff, float layer_scale,
// float time_scale)`, RET 0Ch.
struct OceanLayerParams {
    float falloff_18{0.0f};
    float layer_scale_1c{0.0f};
    float time_scale_20{0.0f};
    float tile_b0{0.0f};
    float inverse_tile_b4{0.0f};
};
OceanLayerParams set_layer_params_00b9a4c0(float falloff, float layer_scale, float time_scale);

// 00B9A450 fragment 00B9A45x..00B9A4AB: after the spectrum call 00B96C10(dir,
// speed) (not reconstructed here; the spectrum is segment 91), the direction
// is stored at +59Ch with FCOS at +64h and FSIN at +68h. 0078C9B0 passes
// block+4Ch times the double pi at 00CE3D28 as dir.
struct WindTrig {
    float direction_59c{0.0f};
    float cos_64{0.0f};
    float sin_68{0.0f};
};
WindTrig wind_trig_00b9a450(float direction);

// 00BA0090 at 00BA0B32..00BA0B93: the per-frame wave scale and amplitude.
//   +2Ch = [field+B8h] (Waves.HeightScale, set by 0078C9B0)
//   +28h = +2Ch * [spectrum+88h] (cWaveScale.x), +30h = the same product
//   +24h = +2Ch, stored at 00BA0B93 after 00BA70F0/00BA8EE0/00B9F0A0
// +24h is the amplitude 0078C890 multiplies the grid sample by.
struct OceanWaveScale {
    float wave_scale_28{0.0f};
    float height_scale_2c{0.0f};
    float wave_scale_30{0.0f};
    float amplitude_24{0.0f};
};
OceanWaveScale ocean_wave_scale_00ba0b32(float height_scale_b8, float spectrum_choppy_88);

// ---------------------------------------------------------------------------
// Projected grid.

// 00B999F0 `float* __thiscall(const float plane[4], float out[3],
// const float a[3], const float b[3])`, RET 0Ch, returns out:
// t = (n.a - d) / ((n.a - d) - (n.b - d)); out = a + t * (b - a).
Float3 plane_intersect_segment_00b999f0(const Float4& plane, const Float3& a, const Float3& b);

// 00B9A8A0 `bool __thiscall(const float plane[4], const float a[3],
// const float b[3], float out[3])`, RET 0Ch: true when a.y and b.y lie
// strictly on opposite sides of plane.w and |a.y - b.y| > 1.1f ([00CE6448]),
// then out = 00B999F0(plane, a, b).
bool plane_crossing_00b9a8a0(const Float4& plane, const Float3& a, const Float3& b, Float3& out);

// 00B99BF0 `void __stdcall(float p[3], const float m[16])`, RET 8, in place:
// 00B62D10 of (p.x, p.y, p.z, 1.0), then p = xyz * (1 / w) (FLD1, FDIVRP,
// three FMULs). Returns the new p here.
Float3 transform_homogeneous_divide_00b99bf0(const Float3& point, const float matrix[16]);

// 00B99C80 `float* __stdcall(float out[4], float u, float v, const float m[16])`, RET 10h:
// a = 00B62D10((u, v, -1, 1), m), b = 00B62D10((u, v, 1, 1), m),
// t = -a.y / (b.y - a.y), out = a + t * (b - a).
Float4 clip_ray_water_plane_00b99c80(float u, float v, const float matrix[16]);

// 00B9CA00 `void __fastcall(field)`: the four cCorner constants at
// +134h/+144h/+154h/+164h from the projector range matrix at +174h at uv
// (0,0), (1,0), (0,1), (1,1), each corner's y then cleared to 0.
struct GridCorners {
    Float4 corner[4];
};
GridCorners compute_grid_corners_00b9ca00(const float projector_matrix_174[16]);

// Grid resolution of the projected mesh 00BA5240 builds ([+FCh] x [+100h]).
//   00BA5610 (initial): simple ocean 20h x 20h, else 20h x 40h.
//   00BA6BD0 (vtable slot 4, quality): simple 20h x 20h, else 80h x 100h; it
//   also stores its argument in [00E130BC].
struct GridResolution {
    std::uint32_t columns_fc{0};
    std::uint32_t rows_100{0};
};
GridResolution initial_grid_resolution_00ba5610(bool simple_ocean_f9);
GridResolution quality_grid_resolution_00ba6bd0(bool simple_ocean_f9);

// ---------------------------------------------------------------------------
// Water rings.

// 00B9E3F0 `void* __thiscall(field, const float pos[3], float a, uint32 b,
// float duration)`, RET 10h: a 24h-byte record appended to +4F8h/+4FCh.
// rate = (a - 2pi) / duration: FSUB of the double at 00CE3828 and FDIV in
// extended precision, one float rounding at 00B9E43E, stored to +1Ch and +20h.
struct WaterRingRecord {
    Float3 position_00;
    float a_0c{0.0f};
    std::uint32_t b_10{0};
    float duration_14{0.0f};
    float start_clock_18{0.0f};  // [01090494] at creation
    float rate_1c{0.0f};
    float rate_20{0.0f};
};
WaterRingRecord make_water_ring_00b9e3f0(const Float3& position, float a, std::uint32_t b,
                                         float duration, float ocean_clock);

}  // namespace bsp::water_render
