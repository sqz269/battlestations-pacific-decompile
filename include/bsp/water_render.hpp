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

#include "bsp/camera_projection.hpp"

namespace bsp {
struct CameraAxesCrtAccess;
}

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

// 00B9AE00 `bool __thiscall(field, camera)`, RET 4: the projected-grid
// projector (packet cc13_water2). Called once per frame from 00BA0090.
// Verified instruction by instruction against the listing (x87 stack traced
// from the opcode bytes); docs/CC13_WATER_RENDER.md section 10.
//
// The camera is the render camera node. Every touch point the native makes is
// one method here, called in native order:
//   refresh_world_if_dirty  TEST [cam+5Ch],2 / JNZ / CALL 00B6DB70 (6 sites)
//   set_position_vtable_30  CALL [[cam]+30h] with a pointer to three floats
//   set_field_of_view       00B6FBB0, which stores cam+1C4h
// The reads are cam+1C4h fov, +1C8h aspect, +1D4h near, +1D8h far, +110h the
// world forward row, +120h the world position row and +F0h the world matrix.
class ProjectorCamera {
public:
    virtual ~ProjectorCamera() = default;
    virtual void refresh_world_if_dirty() = 0;
    virtual float field_of_view_1c4() const = 0;
    virtual float aspect_1c8() const = 0;
    virtual float near_plane_1d4() const = 0;
    virtual float far_plane_1d8() const = 0;
    virtual Float3 forward_110() const = 0;
    virtual Float3 position_120() const = 0;
    virtual const CameraMatrix& world_f0() const = 0;
    virtual void set_position_vtable_30(const Float3& position) = 0;
    virtual void set_field_of_view_00b6fbb0(float fov) = 0;
};

// The five matrices 00B9AE00 stores into the field (row-major, row vectors).
//   +174h range * inverse(view * projection)   (00B9CA00 reads it)
//   +1B4h inverse(+174h), from the stored +174h words
//   +1F4h (view * projection) * affine_inverse(range)   (00B63B30)
//   +418h the projector view (00B63F10 look-at)
//   +458h the projector projection (00B642F0, the camera's own parameters)
// range = [dx 0 0 0; 0 dy 0 0; 0 0 1 0; min_x min_y 0 1] over the projected
// x/y extent of the clipped points.
struct ProjectorMatrices {
    CameraMatrix range_174{};
    CameraMatrix range_inverse_1b4{};
    CameraMatrix view_projection_range_1f4{};
    CameraMatrix view_418{};
    CameraMatrix projection_458{};
};

// Steps (all addresses in 00B9AE00):
//  1. B9AE25: save fov and position; fov *= (double)1.2f [00CEC160]; move the
//     camera to (0, y, 0) through vtable+30.
//  2. B9B00F: near = max(near, 1.5) ([00CE3D78] double compare, [00CE380C]
//     store); P = perspective(fov, aspect, near, far); invP = 00B632D0(P).
//  3. B9B157: the four near-plane clip corners (+-1, +-1, 0) through invP with
//     a 1/w divide; the far corners are the near ones times far/near (B9B219).
//  4. B9B382: all eight to world space through cam+F0h (004142E0).
//  5. B9B3D0: the 12 cube edges against the planes y = +h and y = -h
//     (00B9A8A0); h = field+B8h, -h formed as -0.0f - h.
//  6. B9B492: corners strictly inside the slab, (h + y) / (y - h) < 0.
//  7. B9B5B8: Pp = perspective again; projector position P = camera position
//     raised to at least h + 7 [00CED5D8] (mirrored when below y = 0).
//  8. B9B75E: forward nudged off the horizontal by 0.01 [00D7A358] and
//     renormalised (00BF7030) when |forward.y| <= cos(89 deg) [00CEC400].
//  9. B9BA2A: aim = blend of the forward ray's water hit (00B999F0, reflected
//     when camera and forward are on the same side) and the camera position
//     + 10 [00CE3DC0] * forward flattened to y = 0, by |forward.y|.
// 10. B9BD41: view = look-at(P, aim, up (0,1,0)).
// 11. B9BD46: points flattened to y = 0, projected by view then Pp with 1/w,
//     x/y min/max, range matrix, outputs above.
// 12. B9C9A5 / B9C9D3: restore position and fov. Returns false (AL = 0) with
//     no field write when no point survives (B9C117).
// The native keeps 80-bit intermediates between spills; this rounds at every
// native float store and uses double between them, so it is close but not
// bit-exact. The point list holds 32 entries (native S-0x120 array: at most
// 6 + 6 plane hits plus 8 corners occur for a convex frustum).
bool compute_projector_matrices_00b9ae00(ProjectorCamera& camera, float displacement_height_b8,
                                         const CameraAxesCrtAccess& crt, ProjectorMatrices& out);

// ---------------------------------------------------------------------------
// Height-tile pyramid (packet cc13_water2).

// One 18h-byte cell. Level 0 holds the spectrum sample; coarser levels hold
// the 00B9A990 average of a 2x2 block of the finer level.
struct OceanGridCell {
    float x{0.0f};       // +00h choppy x
    float height{0.0f};  // +04h height (level 0: -0.0f - spectrum height)
    float z{0.0f};       // +08h choppy z
    float normal[3]{};   // +0Ch
};

// One 0Ch-byte pyramid level; the array is [field+2B0h], its count [field+2B4h].
struct OceanGridLevel {
    std::uint32_t width{0};   // +0h
    std::uint32_t height{0};  // +4h
    OceanGridCell* cells{nullptr};  // +8h, width * height cells
};

// 00B9A990 `void __thiscall(OceanGridCell* out, const OceanGridCell* p0,
// p1, p2, p3, float scale)`, RET 14h (packet cc13_water2, verified against
// the listing):
//   out.xyz = (((p0 + p1) + p2) + p3) * 0.25 (double [00D7A348]),
//     each partial sum spilled.
//   n = ((p1.n + p0.n) + p2.n) + p3.n, spilled per component;
//   m = 005803E0 lerp(n, (0, 4.0 [00CE3D34], 0), scale);
//   len = 00419440(m); inv = len > 0 ? 1/len : 0 (XORPS); out.n = m * inv.
OceanGridCell average_grid_quad_00b9a990(const OceanGridCell& p0, const OceanGridCell& p1,
                                         const OceanGridCell& p2, const OceanGridCell& p3, float scale,
                                         const CameraAxesCrtAccess& crt);

// What 00B9D7B0 reads and writes on the ocean field (ECX).
//   height_scale_b8   [field+B8h], Waves.HeightScale
//   choppy_30         [[field+BCh]+30h..+34h], 8-byte (x, z) entries
//   heights_20        [[field+BCh]+20h..+24h], 8-byte entries, word 0 read
//   levels / level_count  [field+2B0h] / [field+2B4h]
//   extents_104       [field+104h..+110h], running maxima, never reset here
struct OceanGridTilesField {
    float height_scale_b8{0.0f};
    const float* choppy_30{nullptr};
    std::uint32_t choppy_count{0};
    const float* heights_20{nullptr};
    std::uint32_t heights_count{0};
    OceanGridLevel* levels{nullptr};
    std::uint32_t level_count{0};
    float extents_104[4]{};  // +104h scaled |h| max, +108h |x|, +10Ch |h|, +110h |z|
};

// 00B9D7B0 `void __fastcall(field)`, vtable 00D63E10 slot 0 (packet
// cc13_water2, verified against the listing).
//  * B9D7F0..B9DA07: for each level-0 cell (x, y), i = (x & 3Fh) + (y & 3Fh)*40h
//    into the 64x64 spectrum vectors. Both vectors are range checked first
//    (00BF6713 = _invalid_parameter_noinfo); the handler is
//    `invalid_parameter`. The native then reads the element anyway; this port
//    skips the cell instead when the handler returns.
//  * cell = (choppy.x, -0.0f - height, choppy.z). The "-0.0 resets" in the
//    decompile are only the XMM3 reload of [00D7A208] after the CRT call.
//  * |v| is v > 0 ? v : -0.0f - v (COMISS / SUBSS).
//    +104h = old > float(HS * |h|) ? old : that (NaN takes the new value)
//    +108h, +10Ch, +110h = new > old ? new : old (NaN keeps the old value)
//  * Cell words +0Ch..+14h are written from three never-initialised stack
//    slots ([ESP+4Ch..54h] = frame -0Ch..-4h); the caller supplies them as
//    `stack_garbage`. They are not overwritten at level 0.
//  * B9DA0D..B9DB2F: for level i = 0 .. count-2 the coarser level i+1 cell
//    (x, y) = 00B9A990 of level i cells (2x, 2y), (2x+1, 2y), (2x, 2y+1),
//    (2x+1, 2y+1), scale = {0, 0.2 [00CE54A0], 0.5 [00CE3800],
//    0.9 [00CE3860], 1.0}[i] (a 5-entry stack array; more levels would read
//    past it).
//  * B9DB35: tail jump to vtable slot 3 (00BA5030, the float16 upload),
//    the `upload` callback here.
void refresh_grid_tiles_00b9d7b0(OceanGridTilesField& field, const float stack_garbage[3],
                                 const CameraAxesCrtAccess& crt, void (*invalid_parameter)(),
                                 void (*upload)(void* context), void* upload_context);

// ---------------------------------------------------------------------------
// Underwater light beams (packet cc13_water2).

// One beam, pointed to by [beams+E8h][i].
struct OceanBeam {
    float position[3]{};   // +00h
    float birth_0c{0.0f};  // +0Ch, < 0 means "respawn now"
    float death_10{0.0f};  // +10h, < 0 means "no death scheduled yet"
};

// One 1Ch-byte vertex of the locked beam vertex buffer.
struct OceanBeamVertex {
    float x, y, z;
    float u, v;
    float alpha;
    std::uint32_t argb;
};

// The beam system object (world+ACh). Offsets as in the native object; the
// names are hypotheses from use.
struct OceanBeamSystem {
    std::uint32_t enabled_2c{0};
    float speed_30{0.0f};
    float color_3c[4]{};
    float length_4c{0.0f};        // bottom vertices = top + axis * this
    float radius_base_50{0.0f};   // spawn radius = base + range * u
    float radius_range_54{0.0f};
    float half_width_58{0.0f};
    float axis_scale_5c{0.0f};    // weight of the displacement normal in the axis
    float death_band_60[3]{};     // squared-distance bands +60h/+64h/+68h
    float death_rate_6c[4]{};     // per-band death probabilities +6Ch..+78h
    float fade_in_time_cc{0.0f};
    float fade_out_time_d0{0.0f};
    float fade_in_rate_d4{0.0f};
    float fade_out_rate_d8{0.0f};
    std::uint32_t count_dc{0};
    OceanBeam** beams_e8{nullptr};
    float previous_clock_ec{0.0f};
    float clock_f0{0.0f};
};

// Everything 00BA2FF0 reaches outside the beam object, in native order.
class OceanBeamHost {
public:
    virtual ~OceanBeamHost() = default;
    virtual void refresh_camera_world_if_dirty() = 0;          // cam+5Ch bit 2 -> 00B6DB70
    virtual Float3 camera_position_120() = 0;
    virtual void camera_trivial_00b6fc60() = 0;                // native returns camera address + 1ACh; this call ignores it
    virtual void set_visibility_factor_00b6da70(float value) = 0;  // [beams+14h], recurse = 0
    // The first entry of the light list [[beams+14h]+170h]+18h; its +1E0h
    // vector. false = empty list (the native calls 00BF6713 and goes on).
    virtual bool first_light_vector(Float3& raw) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BA2C20 with ECX = 00E130C0, the private Mersenne state. The caller's
    // inlined guard (state+9C5h set and +9C4h clear -> write to address 0) is
    // the host's job too. Must never be the gameplay stream 00BD2F10.
    virtual std::uint32_t next_private_random_00ba2c20() = 0;
    virtual float water_height_0078c890(float x, float z) = 0;       // ECX = [[beams+10h]+8] - 4
    virtual void horizontal_displacement_00ba2b50(float x, float z, float out[2]) = 0;
    virtual void set_draw_counts(std::uint32_t vertices_10, std::uint32_t primitives_18) = 0;  // [beams+1Ch]
    virtual OceanBeamVertex* lock_vertices() = 0;                    // [[beams+24h]]+10h (0, 0, 0)
    virtual void unlock_vertices() = 0;                              // [[beams+24h]]+14h
    virtual std::uint32_t color_argb_004fb850(const float rgba[4]) = 0;
};

// 00BA2FF0 `void __thiscall(beams, camera, float time)`, RET 8, called from
// 0078D02C (packet cc13_water2, verified against the listing).
//  * Off when enabled_2c == 0. Camera at or above y = 0: visibility 0, return.
//  * clock_f0 = float(speed * time); previous = old clock, or the new clock
//    when the old one is negative; frame delta = clock - previous.
//  * L = normalize(light) (00419510). n = (-0.0-L.x, -L.y, -0.0-L.z);
//    s = sqrt(1 - n.y^2); h = n.xz * 00419260(n.xz) * s; ly = -sqrt(1 - (1 - n.y^2)).
//    origin = camera - (camera.y / ly) * (h.x, ly, h.z): where the light ray
//    through the camera leaves the surface.
//  * Per beam: birth < 0, or death >= 0 and death < clock -> respawn with
//    two draws: radius = base + range * u1, angle = u2 * (double)6.2831855f
//    [00CE3828], position = origin + radius * (cos, 0 [00D7A258], sin).
//    birth = clock, death = -1. u = (float)(draw (+2^32 if negative)) *
//    2^-32 [00CE3978]/[00D63B80].
//    death < 0 -> one draw u3; with d2 = |beam.xz - origin.xz|^2 and p the
//    band rate (+6Ch if d2 < +60h, +70h if < +64h, +74h if < +68h, else +78h),
//    survival = exp(log(1 - p) * delta) (00BFA910 = _CIlog, 00BFEEEC =
//    _CIexp); u3 > survival -> death = clock + fade_out_time.
//  * Draw counts 4n / 2n; n == 0 -> visibility 0. Otherwise each beam is a
//    quad facing the surface origin: perpendicular = normalize(origin - beam)
//    rotated (or (1, 0) when the distance <= (double)0.05f [00D7A270]),
//    top corners at +-half_width on the water height (0078C890), axis =
//    normalize(normalize(dx, 64 [00D638F0], dz) * axis_scale + (h.x, ly, h.z))
//    with (dx, dz) = 00BA2B50, bottom = top + axis * length. Vertex i: right
//    corner for i = 1, 3 (u = 0), left for 0, 2 (u = 1); v = 0 for i < 2, else
//    1 and the bottom position. alpha = fade-in * fade-out.
// Presentation only: it reads the gameplay water height but changes nothing
// outside the beam object; it never draws from 00BD2F10.
void update_ocean_beams_00ba2ff0(OceanBeamSystem& beams, OceanBeamHost& host, float time,
                                 const CameraAxesCrtAccess& crt);

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
    float b_10{0.0f};  // a float: 00B9CE96 FMUL dword [ring+10h]
    float duration_14{0.0f};
    float start_clock_18{0.0f};  // [01090494] at creation
    float rate_1c{0.0f};
    float rate_20{0.0f};
};
WaterRingRecord make_water_ring_00b9e3f0(const Float3& position, float a, float b,
                                         float duration, float ocean_clock);

}  // namespace bsp::water_render
