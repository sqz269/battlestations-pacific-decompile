#include "bsp/water_weather_cc13.hpp"

#include <algorithm>
#include <intrin.h>
#include <new>

#include "bsp/allocator_list.hpp"
#include "bsp/native_resource_cache_leaves.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

// ================================================================================================
// Part a: 00B999F0-00BA2FFF
// ================================================================================================
// cc13-water1 helper part "a" (ocean wave field, 00B999F0-00BA2FFF). Names are hypotheses.
// x87 model: the image runs at 53-bit precision, so intermediates are double and every FSTP
// to a float slot is a float cast. None of these functions draws from 00BD2F10 or
// BSP_RandomState_NextU32.
namespace bsp::water_weather_cc13 {
namespace {

float f32_a(double v) { return static_cast<float>(v); }

// Constants read from the image.
constexpr float kHalfA = 0.5f;                  // 00CE3800 (3F000000)
constexpr float kMinusHalfA = -0.5f;            // 00CE69D0 (BF000000)
constexpr float kOneA = 1.0f;                   // 00D7A24C (3F800000)
constexpr float kMinusOneA = -1.0f;             // 00D7A260 (BF800000)
constexpr double kHalfDoubleA = 0.5;            // 00D7A280 (double 3FE0000000000000)
constexpr float kNegZeroA = -0.0f;              // 00D7A208 (80000000)
constexpr std::uint32_t kRingAngleBitsA = 0x3F860A92u;  // 00D05AAC, float pi/3
constexpr double kNearFloorA = 1.5;             // 00CE3D78 (double 3FF8000000000000)
constexpr float kNearClampA = 1.5f;             // 00CE380C (3FC00000)
constexpr const char* kTerrainTagA = "terrain";  // 00B9FA60 _strstr literal
constexpr char kPathSeparatorA = '/';           // 00CE7898 ("/")

float ring_angle_a() {
    float v;
    std::memcpy(&v, &kRingAngleBitsA, sizeof v);
    return v;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00B9A2F0 RET; 00BA2460 RET 4; 00BA2470 XOR EAX,EAX / RET 0Ch; 00BA2480 RET 4;
// 00BA2490 RET 8 (disasm-raw). Vtable slots from 00D63B5C.
void ocean_field_base_noop_00b9a2f0() {}
void ocean_field_base_set_quality_noop_00ba2460(int) {}
int ocean_field_base_return_zero_00ba2470(std::uint32_t, std::uint32_t, std::uint32_t) {
    return 0;
}
void ocean_field_base_noop4_00ba2480(std::uint32_t) {}
void ocean_field_base_noop8_00ba2490(std::uint32_t, std::uint32_t) {}

// 00B9A3B0: MOV [out], 200h; MOV [out+4], 200h; RET 4.
W1A_OceanTargetSize ocean_field_reflection_target_size_00b9a3b0() { return {0x200, 0x200}; }

// ---------------------------------------------------------------------------
// 00B9DBB0: the reference assign is skipped when the handle is unchanged; the new handle
// is incremented before the old one is decremented. The four floats are stored always.
W1A_OceanRefObject* ocean_field_set_underwater_fog_00b9dbb0(W1A_OceanUnderwaterFog& field,
                                                             W1A_OceanRefObject* fog,
                                                             const std::array<float, 4>& color) {
    W1A_OceanRefObject* destroyed = nullptr;
    W1A_OceanRefObject* old = field.fog_ref_2ac;
    if (old != fog) {
        field.fog_ref_2ac = fog;
        if (fog != nullptr) ++fog->refs_04;
        if (old != nullptr && --old->refs_04 == 0) destroyed = old;
    }
    field.fog_color_3ac = color;
    return destroyed;
}

// 00B9DD90: same assign on +BCh, then spectrum+8 = field unconditionally (the native
// dereferences the argument, so a null spectrum faults there).
W1A_OceanRefObject* ocean_field_set_spectrum_00b9dd90(W1A_OceanSpectrumLink*& spectrum_bc,
                                                       void* field,
                                                       W1A_OceanSpectrumLink& spectrum) {
    W1A_OceanRefObject* destroyed = nullptr;
    W1A_OceanSpectrumLink* old = spectrum_bc;
    if (old != &spectrum) {
        spectrum_bc = &spectrum;
        ++spectrum.ref.refs_04;
        if (old != nullptr && --old->ref.refs_04 == 0) destroyed = &old->ref;
    }
    spectrum.owner_field_08 = field;
    return destroyed;
}

// 00B9DB50 / 00B9DE30: load once, bind on +12Ch then +130h, release the loader's reference.
void ocean_field_set_reflection_cubemap_00b9db50(W1A_OceanTextureHost& host,
                                                 const std::string& name) {
    void* texture = host.load_texture_f8d394_64(name);
    host.set_texture_slot(0, 2, texture);
    host.set_texture_slot(1, 2, texture);
    if (texture != nullptr) host.release_texture(texture);
}

void ocean_field_set_normal_map_texture_00b9de30(W1A_OceanTextureHost& host,
                                                 const std::string& name) {
    void* texture = host.load_texture_f8d394_64(name);
    host.set_texture_slot(0, 5, texture);
    host.set_texture_slot(1, 5, texture);
    if (texture != nullptr) host.release_texture(texture);
}

// 00B9E8B0: argument order checked against the caller 0078CA14..0078CA52 (UseFoam +0Ch,
// FadeRange +8, ScrollSpeed +0, NoiseScrollSpeed +4, Height +10h, Scale +14h, NoiseScale
// +18h, Texture +1Ch, NoiseTexture +24h): FadeRange lands in +8Ch and ScrollSpeed in +88h.
void ocean_field_set_foam_00b9e8b0(W1A_OceanFoamState& foam, W1A_OceanFoamHost& host,
                                   const W1A_OceanFoamArgs& args) {
    foam.fade_range_8c = args.fade_range;
    foam.scroll_speed_88 = args.scroll_speed;
    foam.noise_scroll_speed_90 = args.noise_scroll_speed;
    foam.height_94 = args.height;
    foam.scale_98 = args.scale;
    foam.use_foam_84 = args.use_foam;
    foam.noise_scale_9c = args.noise_scale;
    foam.texture_a0 = args.texture;
    foam.noise_texture_a8 = args.noise_texture;
    if (!host.foam_model_present_120()) return;
    if (!foam.use_foam_84) {
        host.foam_propagate_root_00b6d890(false);   // 00B9E992
        host.foam_set_parent_null_00b6e680();       // 00B9E99F
    } else {
        host.foam_propagate_root_00b6d890(true);    // 00B9E9AA, root = field+14h
    }
    void* texture = host.load_texture_f8d394_64(foam.texture_a0);
    host.set_texture_slot(2, 0, texture);
    if (texture != nullptr) host.release_texture(texture);
    texture = host.load_texture_f8d394_64(foam.noise_texture_a8);
    host.set_texture_slot(2, 1, texture);
    if (texture != nullptr) host.release_texture(texture);
}

// ---------------------------------------------------------------------------
// 00B9DED0: linear search from the front; the last element moves into the hole, so ring
// order (and therefore 00B9CB80's vertex order) changes on every removal.
bool ocean_ptr_vector_erase_value_swap_last_00b9ded0(std::vector<void*>& v, void* value) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i] != value) continue;
        if (i != v.size() - 1) v[i] = v.back();
        v.pop_back();
        return true;
    }
    return false;
}

// 00B9E4A0: ADD ECX,4F8h; CALL 00B9DED0; free the ring if non-null (found or not).
void ocean_field_remove_water_ring_00b9e4a0(std::vector<void*>& rings_4f8, void* ring,
                                            void (*free_ring)(void*)) {
    ocean_ptr_vector_erase_value_swap_last_00b9ded0(rings_4f8, ring);
    if (ring != nullptr && free_ring != nullptr) free_ring(ring);
}

// 00B9CB80: one triangle per ring, in the XZ plane at the ring's height. R = a / cos(pi/3)
// (the circumradius of a triangle with inradius a), h = sqrt(R*R - a*a) (half the side).
std::vector<W1A_OceanRingVertex> ocean_field_update_water_ring_mesh_00b9cb80(
    const std::vector<const W1A_OceanRingRecord*>& rings_4f8, W1A_OceanRingMeshHost& host) {
    std::vector<W1A_OceanRingVertex> out;
    if (rings_4f8.empty()) {
        host.hide_ring_model_504();
        return out;
    }
    host.show_ring_model_504();
    out.reserve(rings_4f8.size() * 3);
    // 00B9CCD4..00B9CCEC: FLD float pi/3; FCOS; FSTP float (0.5f).
    const float cos_angle = f32_a(std::cos(static_cast<double>(ring_angle_a())));
    for (const W1A_OceanRingRecord* ring : rings_4f8) {
        const W1A_OceanRingRecord& r = *ring;
        const float a = r.radius_0c;
        const float big_r = f32_a(static_cast<double>(a) / cos_angle);        // 00B9CD2C
        const float h_sq = f32_a(static_cast<double>(big_r) * big_r -
                                 static_cast<double>(a) * a);                 // 00B9CD40
        const float h = f32_a(std::sqrt(static_cast<double>(h_sq)));          // 00BF7030
        const float neg_a = kNegZeroA - a;                                    // 00B9CD67 SUBSS
        const double x = r.position_00[0], y = r.position_00[1], z = r.position_00[2];
        const float scaled_b = f32_a(static_cast<double>(
                                         f32_a(static_cast<double>(r.rate_20) / r.rate_1c)) *
                                     r.b_10);                                 // 00B9CE88..CE99
        const float pos[3][3] = {
            {f32_a(x + 0.0), f32_a(y + 0.0), f32_a(z + big_r)},
            {f32_a(h + x), f32_a(y + 0.0), f32_a(z + neg_a)},
            {f32_a(-static_cast<double>(h) + x), f32_a(0.0 + y), f32_a(z + neg_a)},
        };
        for (const auto& p : pos) {
            W1A_OceanRingVertex v;
            v.position[0] = p[0];
            v.position[1] = p[1];
            v.position[2] = p[2];
            v.centre[0] = r.position_00[0];
            v.centre[1] = r.position_00[1];
            v.centre[2] = r.position_00[2];
            v.radius = a;
            v.start_clock = r.start_clock_18;
            v.scaled_b = scaled_b;
            v.duration = r.duration_14;
            out.push_back(v);
        }
    }
    const auto count = static_cast<std::uint32_t>(rings_4f8.size());
    host.set_ring_counts(count, count * 3);
    return out;
}

// ---------------------------------------------------------------------------
// 00B9AB60.
W1A_OceanMatrix ocean_projective_bias_a(float tx, float ty) {
    W1A_OceanMatrix m{};
    m[0] = kHalfA;
    m[5] = kMinusHalfA;
    m[10] = kOneA;
    m[12] = tx;
    m[13] = ty;
    m[15] = kOneA;
    return m;
}

void ocean_field_update_projective_matrices_00b9ab60(W1A_OceanProjectiveState& s,
                                                     W1A_OceanProjectiveHost& host,
                                                     std::int32_t reflection_size,
                                                     std::int32_t screen_width,
                                                     std::int32_t screen_height,
                                                     float global_010909b4) {
    // 00B9AB77..00B9AC18: one value for both translations, from the slot-5 width only.
    const float half_texel = f32_a(kHalfDoubleA / static_cast<double>(reflection_size));
    const float b = f32_a(kHalfDoubleA + half_texel);
    s.reflection_matrix_54c = host.multiply_00413920(host.camera_view_projection_00b70490(0),
                                                     ocean_projective_bias_a(b, b));
    // 00B9AC49..00B9ACFE: width from the first +80h call, height from the second.
    const float rx = f32_a(kHalfDoubleA / static_cast<double>(screen_width));
    const float ry = f32_a(kHalfDoubleA / static_cast<double>(screen_height));
    s.refraction_matrix_50c = host.multiply_00413920(
        host.camera_view_projection_00b70490(1),
        ocean_projective_bias_a(f32_a(rx + kHalfDoubleA), f32_a(ry + kHalfDoubleA)));
    // 00B9AD26..00B9ADF2 (FLD1/FDIV pairs rounded once to float).
    s.constants_58c = {s.value_40c, s.value_40c, s.value_40c, kOneA};
    s.foam_scale_5a0 = {f32_a(1.0 / s.scale_98), s.height_94, f32_a(1.0 / s.noise_scale_9c),
                        kOneA};
    s.foam_scroll_5b0 = {s.scroll_speed_88, s.noise_scroll_speed_90, s.wind_direction_59c,
                         f32_a(1.0 / s.fade_range_8c)};
    s.clock_3c = global_010909b4;
}

// ---------------------------------------------------------------------------
// 00B9D490: allocate width*height cells, copy the overlapping top-left block row by row
// (old stride = old width), free the old block. New cells are uninitialised in the image;
// here they are value-initialised. Non-positive dimensions give an empty block.
void ocean_cell_grid_resize_00b9d490(W1A_OceanCellGrid& grid, std::int32_t width,
                                     std::int32_t height) {
    const std::int32_t w = width > 0 ? width : 0;
    const std::int32_t h = height > 0 ? height : 0;
    std::vector<W1A_OceanGridCell> cells(static_cast<std::size_t>(w) * h);
    const std::int32_t copy_w = grid.width_00 < width ? grid.width_00 : width;
    const std::int32_t copy_h = grid.height_04 < height ? grid.height_04 : height;
    for (std::int32_t yy = 0; yy < copy_h; ++yy) {
        for (std::int32_t xx = 0; xx < copy_w; ++xx) {
            cells[static_cast<std::size_t>(yy) * w + xx] =
                grid.cells_08[static_cast<std::size_t>(grid.width_00) * yy + xx];
        }
    }
    grid.width_00 = width;
    grid.height_04 = height;
    grid.cells_08 = std::move(cells);
}

// 00B9E0C0: self-assign returns at once; otherwise free, zero both sizes (so the resize
// copies nothing), resize to the source size and copy every cell.
W1A_OceanCellGrid& ocean_cell_grid_assign_00b9e0c0(W1A_OceanCellGrid& dst,
                                                   const W1A_OceanCellGrid& src) {
    if (&dst == &src) return dst;
    dst.cells_08.clear();
    dst.width_00 = 0;
    dst.height_04 = 0;
    ocean_cell_grid_resize_00b9d490(dst, src.width_00, src.height_04);
    for (std::int32_t yy = 0; yy < dst.height_04; ++yy) {
        for (std::int32_t xx = 0; xx < dst.width_00; ++xx) {
            const std::size_t i = static_cast<std::size_t>(dst.width_00) * yy + xx;
            dst.cells_08[i] = src.cells_08[static_cast<std::size_t>(src.width_00) * yy + xx];
        }
    }
    return dst;
}

// ---------------------------------------------------------------------------
// 00B9FE00: +5F4h loses the first match only (00B7BED0); +614h and +620h lose every match,
// each removal restarting the search from index 0 and moving the last record into the hole.
void ocean_field_remove_floating_object_00b9fe00(std::vector<void*>& objects_5f4,
                                                 std::vector<W1A_OceanFloatNode>& nodes_614,
                                                 std::vector<W1A_OceanCoverageRecord>& regions_620,
                                                 void* object, W1A_OceanFloatHost& host) {
    ocean_ptr_vector_erase_value_swap_last_00b9ded0(objects_5f4, object);
    for (;;) {
        std::size_t i = 0;
        while (i < nodes_614.size() && nodes_614[i].owner_00 != object) ++i;
        if (i == nodes_614.size()) break;
        // 00B9FE70..00B9FEA3: both calls use the node even when it is null.
        host.node_set_parent_null_00b6e680(nodes_614[i].node_04);
        host.node_propagate_root_null_00b6d890(nodes_614[i].node_04);
        if (nodes_614[i].node_04 != nullptr) {
            host.node_unlink_and_release_00b6dfa0(nodes_614[i].node_04);
            nodes_614[i].node_04 = nullptr;
        }
        if (i != nodes_614.size() - 1) nodes_614[i] = nodes_614.back();
        nodes_614.pop_back();
    }
    for (;;) {
        std::size_t i = 0;
        while (i < regions_620.size() && regions_620[i].owner_00 != object) ++i;
        if (i == regions_620.size()) return;
        if (i != regions_620.size() - 1) {
            W1A_OceanCoverageRecord& dst = regions_620[i];
            const W1A_OceanCoverageRecord& src = regions_620.back();
            dst.owner_00 = src.owner_00;
            dst.mask_04 = src.mask_04;            // 00B9E5D0
            std::memcpy(dst.words_10, src.words_10, sizeof dst.words_10);
        }
        regions_620.pop_back();                   // last record's mask freed, count - 1
    }
}

// ---------------------------------------------------------------------------
// 00BA0090 fragments.
std::array<std::array<float, 4>, 9> ocean_field_depth_sample_kernel_00ba0090(
    std::int32_t screen_width, std::int32_t screen_height) {
    // 00BA0865..00BA0888: FILD; FLD1; FDIVRP; FSTP float.
    const float rx = f32_a(1.0 / static_cast<double>(screen_width));
    const float ry = f32_a(1.0 / static_cast<double>(screen_height));
    const float steps[3] = {kMinusOneA, 0.0f, kOneA};
    std::array<std::array<float, 4>, 9> k{};
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            auto& e = k[static_cast<std::size_t>(row * 3 + col)];
            e = {f32_a(static_cast<double>(steps[col]) * rx),
                 f32_a(static_cast<double>(steps[row]) * ry), 0.0f, 0.0f};
        }
    }
    return k;
}

void ocean_field_wave_scale_00ba0090(W1A_OceanWaveScaleState& s, float spectrum_choppy_88) {
    const float hs = s.height_scale_b8;
    s.height_scale_2c = hs;                                                       // 00BA0B55
    s.wave_scale_28 = f32_a(static_cast<double>(hs) * spectrum_choppy_88);        // 00BA0B5F
    s.wave_scale_30 = f32_a(static_cast<double>(hs) * spectrum_choppy_88);        // 00BA0B6D
}

void ocean_field_publish_amplitude_00ba0090(W1A_OceanWaveScaleState& s) {
    s.amplitude_24 = s.height_scale_2c;                                           // 00BA0B93
    void* t = s.height_target_274;
    s.height_target_274 = s.height_target_278;
    s.height_target_278 = t;
}

// ---------------------------------------------------------------------------
// 00BA2B50: u = xs - floor(xs) with xs = float(k*x), the subtraction in double (FSUBR qword
// of the saved double) and rounded once; floor = 00BF85B0 rounded to float first.
void ocean_field_sample_horizontal_displacement_00ba2b50(float layer_scale_b4,
                                                         W1A_OceanSpectrumSampler& spectrum,
                                                         float x, float z, float out[2]) {
    const float xs = f32_a(static_cast<double>(layer_scale_b4) * x);
    const float u = f32_a(static_cast<double>(xs) - f32_a(std::floor(static_cast<double>(xs))));
    const float zs = f32_a(static_cast<double>(layer_scale_b4) * z);
    const float v = f32_a(static_cast<double>(zs) - f32_a(std::floor(static_cast<double>(zs))));
    const float a = spectrum.sample_00b962e0(u, v);
    const float b = spectrum.sample_00b96450(u, v);
    out[0] = a;
    out[1] = b;
}

// ---------------------------------------------------------------------------
// 00B9F0A0 fragment.
W1A_OceanReflectionCameraSetup ocean_field_reflection_camera_00b9f0a0(
    const W1A_OceanMatrix& camera_world, float camera_near_1d4, float camera_far_1d8,
    float camera_fov_1c4, float camera_aspect_1c8, float sea_height) {
    W1A_OceanReflectionCameraSetup out;
    // 00B9F0D4 FLD 1.5; FCOMIP; JBE: near < 1.5 takes 1.5f, a NaN near is kept.
    out.near_plane = (kNearFloorA > camera_near_1d4) ? kNearClampA : camera_near_1d4;
    // 00B9F168..00B9F174: FCOMIP height vs camera y (+124h); JBE keeps the above-water path,
    // so NaN counts as above water.
    out.underwater_cc = static_cast<double>(sea_height) > static_cast<double>(camera_world[13]);
    if (out.underwater_cc) return out;
    out.far_plane = f32_a(static_cast<double>(camera_far_1d8) * kHalfDoubleA);   // 00B9F47C
    out.field_of_view = camera_fov_1c4;                                         // 00B9F4AA
    out.aspect = camera_aspect_1c8;                                             // 00B9F4C7
    // 00B9F53B..00B9F5F0: mirror in y = 0. Rows 1 and 2 and the translation lose their y,
    // and row 0 becomes cross(row1', row2') through 004F9B30 (a = EDX, b = stack).
    W1A_OceanMatrix m = camera_world;
    m[13] = kNegZeroA - m[13];
    const float ax = m[4], ay = kNegZeroA - m[5], az = m[6];
    const float bx = m[8], by = kNegZeroA - m[9], bz = m[10];
    m[0] = f32_a(static_cast<double>(ay) * bz - static_cast<double>(az) * by);
    m[1] = f32_a(static_cast<double>(bx) * az - static_cast<double>(ax) * bz);
    m[2] = f32_a(static_cast<double>(by) * ax - static_cast<double>(ay) * bx);
    m[5] = ay;
    m[9] = by;
    out.mirrored_world = m;
    return out;
}

// ---------------------------------------------------------------------------
// 00B9FA60 fragment: leaf after the last '/' (BSP_NativeString_ReverseFindAnyByte with "/"
// at 00CE7898, then BSP_NativeString_Substring), tested with _strstr(leaf, "terrain"), case-sensitive. An
// empty leaf has a null buffer and fails; strstr stops at an embedded NUL.
bool ocean_field_is_terrain_effect_00b9fa60(std::string_view effect_name) {
    std::string_view leaf = effect_name;
    const std::size_t slash = leaf.rfind(kPathSeparatorA);
    if (slash != std::string_view::npos) leaf = leaf.substr(slash + 1);
    const std::size_t nul = leaf.find('\0');
    if (nul != std::string_view::npos) leaf = leaf.substr(0, nul);
    if (leaf.empty()) return false;
    return leaf.find(kTerrainTagA) != std::string_view::npos;
}

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part b: 00BA3000-00BBBFFF
// ================================================================================================
// cc13-water1 helper b reconstructions. Evidence per function is in local/w1_b_records.jsonl;
// constants carry their image address. Hypotheses only; see the header banner.
namespace bsp::water_weather_cc13 {

namespace {

// float 00CE3978 = 4294967296.0: the unsigned-to-float fix-up after FILD of a negative dword.
float u32_to_float_b(std::uint32_t v) {
    double d = static_cast<double>(static_cast<std::int32_t>(v));
    if (static_cast<std::int32_t>(v) < 0) d += 4294967296.0;
    return static_cast<float>(d);
}

// FISTP qword under control word | 0C00h (truncate toward zero), low dword kept.
std::uint32_t chop_to_u32_b(double v) {
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(v));
}

std::uint32_t read_u32_le_b(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}

// Reflected CRC-32, polynomial EDB88320h (table 01090500 built by 00BB9F40); the update is
// 00BBA030 (crc = (crc >> 8) ^ table[(crc ^ byte) & 0FFh]), no inversion inside.
std::uint32_t crc32_update_b(std::uint32_t crc, const std::uint8_t* bytes, std::size_t count) {
    static const std::array<std::uint32_t, 256> table = [] {
        std::array<std::uint32_t, 256> t{};
        for (std::uint32_t i = 0; i < 256; ++i) {
            std::uint32_t c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1u) ? (c >> 1) ^ 0xEDB88320u : (c >> 1);
            t[i] = c;
        }
        return t;
    }();
    for (std::size_t i = 0; i < count; ++i) crc = (crc >> 8) ^ table[(crc ^ bytes[i]) & 0xFFu];
    return crc;
}

bool iequals_b(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        unsigned char ca = static_cast<unsigned char>(a[i]);
        unsigned char cb = static_cast<unsigned char>(b[i]);
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<unsigned char>(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<unsigned char>(cb - 'A' + 'a');
        if (ca != cb) return false;
    }
    return true;
}

}  // namespace

// =============================================================================================
// Ocean water particles
std::optional<float> ocean_water_particles_update_00ba40f0(OceanWaterParticlesState& s,
                                                           const W1B_Vec3& cam, float time) {
    const float one = 1.0f;  // float 00D7A24C
    s.volume_size_cc = s.volume_size_70;
    s.one_d8 = one;
    s.one_e8 = one;
    s.volume_corner_dc = {s.volume_offset_60.x + cam.x, s.volume_offset_60.y + cam.y,
                          s.volume_offset_60.z + cam.z};
    s.camera_10c = cam;
    s.scaled_time_118 = s.scale_24 * s.time_c0;  // uses +C0h before this call's update
    s.constants_ec = s.constants_40;
    if (s.enabled_20 == 0) return std::nullopt;
    s.time_c0 = time;
    // 00BA426B: COMISS 0.0, camera.y; JBE -> factor 0 (also for NaN); else FLD1.
    return (0.0f > cam.y) ? 1.0f : 0.0f;
}

// =============================================================================================
// Ocean wave field
std::vector<OceanHeightTexel> ocean_wave_field_height_texels_00ba5030(
    const std::vector<std::vector<std::array<float, 6>>>& levels, std::uint32_t width,
    std::uint32_t height, std::uint32_t level_count) {
    std::vector<OceanHeightTexel> out;
    std::uint32_t a = width;   // vtable +40h result, halved per level (00BA5183)
    std::uint32_t b = height;  // vtable +3Ch result, halved per level (00BA5187)
    for (std::uint32_t level = 0; level < level_count; ++level) {
        const auto* src = level < levels.size() ? &levels[level] : nullptr;
        std::size_t s = 0;
        std::uint32_t column = 0;  // EDI: not reset per row (00BA50A1 resets per level only)
        std::uint32_t row = 0;
        for (std::uint32_t outer = a; outer != 0; --outer) {
            for (std::uint32_t inner = b; inner != 0; --inner) {
                OceanHeightTexel t;
                t.level = level;
                t.texel_index = row * a + column;
                std::array<float, 6> v{};
                if (src && s < src->size()) v = (*src)[s];
                t.value = {v[0], v[1], v[2], v[1]};  // 00BA50F7..00BA5125
                out.push_back(t);
                column = (column + 1) & (a - 1u);
                ++s;
            }
            row = (row + 1) & (b - 1u);
        }
        a >>= 1;
        b >>= 1;
    }
    return out;
}

OceanProjectedGrid ocean_wave_field_projected_grid_00ba5240(std::uint32_t width,
                                                           std::uint32_t height) {
    OceanProjectedGrid g;
    // 00BA5389..00BA53CE: du = float(1 / u32(W-1)), dv = float(1 / u32(H-1)).
    const float du = static_cast<float>(1.0 / static_cast<double>(u32_to_float_b(width - 1u)));
    const float dv = static_cast<float>(1.0 / static_cast<double>(u32_to_float_b(height - 1u)));
    float v = 0.0f;
    for (std::uint32_t r = 0; r < height; ++r) {
        float u = 0.0f;
        for (std::uint32_t c = 0; c < width; ++c) {
            g.uvs.push_back({u, v});
            u = static_cast<float>(static_cast<double>(u) + du);
        }
        v = static_cast<float>(static_cast<double>(v) + dv);
    }
    g.index_buffer_request = height * width * 2u;  // 00BA5459..00BA545E
    // 00BA547B..00BA5501: serpentine strip, direction flips every row; 16-bit stores.
    std::int32_t dir = 1;
    for (std::uint32_t r = 1; r < height; ++r) {
        std::int32_t c = dir > 0 ? 0 : static_cast<std::int32_t>(width) - 1;
        if (c >= 0) {
            while (static_cast<std::uint32_t>(c) < width) {
                g.indices.push_back(static_cast<std::uint16_t>(
                    (static_cast<std::uint16_t>(r) - 1u) * width + static_cast<std::uint16_t>(c)));
                g.indices.push_back(static_cast<std::uint16_t>(
                    static_cast<std::uint16_t>(r) * width + static_cast<std::uint16_t>(c)));
                c += dir;
                if (c < 0) break;  // JNS
            }
        }
        g.indices.push_back(static_cast<std::uint16_t>((r - 1u) * width - dir + c));
        dir = -dir;
    }
    g.primitive_count = static_cast<std::uint32_t>(g.indices.size()) - 2u;
    return g;
}

bool ocean_wave_field_capture_scene_color_runs_00ba51c0(bool flat_f9, bool flag_cc,
                                                        bool target_present) {
    return !flat_f9 && !flag_cc && target_present;
}

std::uint32_t ocean_height_generator_current_stream_00ba7080(const OceanHeightGeneratorState& g) {
    return g.streams_78[g.current_index_84];
}

std::uint32_t ocean_height_generator_target_00ba7090(const OceanHeightGeneratorState& g,
                                                     std::uint32_t index) {
    return g.targets_68[index];
}

void ocean_height_generator_set_wave_scale_00ba70f0(OceanHeightGeneratorState& g,
                                                    const W1B_Vec3& scale) {
    g.wave_scale_100 = scale;
}

OceanViewportSize ocean_height_generator_viewport_size_00ba7120(std::int32_t width,
                                                                std::int32_t height,
                                                                std::int32_t render_scale) {
    return {render_scale * width, render_scale * height};
}

std::array<OceanViewportSize, 4> ocean_height_generator_resize_dimensions_00bb3ee0(
    std::int32_t width, std::int32_t height, std::int32_t scale) {
    return {{{scale * width, scale * height},
             {scale * width, scale * height},
             {width, height},
             {width, height}}};
}

void ocean_height_generator_readback_swap_00bb3ce0(OceanHeightGeneratorState& g) {
    std::swap(g.write_index_80, g.current_index_84);
}

std::array<float, 6> ocean_height_generator_tap_weights_00ba8ee0(OceanCrtHost& host,
                                                                 float ratio_18) {
    // 00BA9012..00BA9063: L = float(log(r)); w_i = float(exp(float(i * L))); sum in float.
    const float l = static_cast<float>(host.crt_log_00bfa910(ratio_18));
    std::array<float, 6> w{};
    float sum = 0.0f;
    for (int i = 0; i < 6; ++i) {
        const float arg = static_cast<float>(static_cast<double>(i) * l);
        w[static_cast<std::size_t>(i)] = static_cast<float>(host.crt_exp_00bfeeec(arg));
        sum = static_cast<float>(static_cast<double>(w[static_cast<std::size_t>(i)]) + sum);
    }
    for (auto& x : w) x = static_cast<float>(static_cast<double>(x) / sum);
    return w;
}

// =============================================================================================
// Skinned water tracer
std::uint32_t tracer_ptr_vector_grow_capacity_00ba9d10(std::uint32_t capacity) {
    return capacity * 2u + 2u;
}

std::uint32_t tracer_ptr_vector_push_capacity_00bac190(std::uint32_t size, std::uint32_t capacity) {
    return size == capacity ? capacity * 2u + 2u : capacity;
}

std::uint32_t tracer_parameter_curve_push_capacity_00bac2c0(std::int32_t size,
                                                            std::int32_t capacity) {
    if (size != capacity) return static_cast<std::uint32_t>(capacity);
    std::int32_t c = capacity * 2;
    if (c < 2) c = 1;
    return static_cast<std::uint32_t>(c);
}

TracerVertex tracer_build_width_profile_00baa0f0(TracerHost& host, std::uint32_t count,
                                                 float value_0, float offset, float value_14,
                                                 float value_15) {
    // Locals hold 255 entries each (2 x 3FCh bytes); count is the caller's (+10h).
    std::vector<float> xs(count), ws(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        // qword 00D7A250 = -1.0, qword 00D7A210 = 1.0.
        const float x = static_cast<float>(
            static_cast<double>(u32_to_float_b(i)) * -1.0 - 1.0);
        xs[i] = x;
        const float d = static_cast<float>(static_cast<double>(x) - offset);
        // 00BAA14B: d > 0 keeps d; else (and NaN) -0.0f (00D7A208) - d.
        const float a = (d > 0.0f) ? d : -0.0f - d;
        float w;
        if (0.1 > static_cast<double>(a)) {  // qword 00D05E28 = 0.1
            w = 1.0f;                         // float 00D7A24C
        } else {
            // 00BAA18E..00BAA1D5: e = float(offset - x); s = float(sqrt(float(e*e) + 0)).
            const float e = static_cast<float>(static_cast<double>(offset) - x);
            const float e2 = static_cast<float>(static_cast<double>(e) * e);
            const float sum = static_cast<float>(static_cast<double>(e2) + 0.0f);
            const float s = static_cast<float>(host.crt_sqrt_00bf7030(sum));
            if (s > 1.25f) {  // float 00CF29A8
                w = 0.0f;
            } else {
                w = static_cast<float>(1.0 - static_cast<double>(s) / 1.25);  // qword 00CF87C0
                if (0.0f > w) w = 0.0f;
                else if (w > 1.0f) w = 1.0f;
            }
        }
        ws[i] = w;
    }
    // Strict-greater maximum search from -1e10f (00CE4ADC), first maximum wins.
    std::uint32_t best = 0;
    float best_w = -10000000000.0f;
    for (std::uint32_t i = 0; i < count; ++i) {
        if (ws[i] > best_w) {
            best = i;
            best_w = ws[i];
        }
    }
    const std::int32_t bm1 = static_cast<std::int32_t>(best) - 1;
    const std::uint32_t lo = bm1 < 1 ? 0u : static_cast<std::uint32_t>(bm1);
    std::int32_t hi = static_cast<std::int32_t>(count) - 1;
    if (static_cast<std::int32_t>(best) + 1 < static_cast<std::int32_t>(count) - 1)
        hi = static_cast<std::int32_t>(best) + 1;
    TracerVertex v{};
    auto at = [&](std::vector<float>& t, std::int64_t k) { return k >= 0 && k < static_cast<std::int64_t>(t.size()) ? t[static_cast<std::size_t>(k)] : 0.0f; };
    v[0] = value_0;
    v[1] = static_cast<float>(static_cast<double>(offset) - at(xs, lo));
    v[2] = static_cast<float>(static_cast<double>(offset) - at(xs, best));
    v[3] = static_cast<float>(static_cast<double>(offset) - at(xs, hi));
    v[4] = static_cast<float>(static_cast<std::int32_t>(lo));
    v[5] = static_cast<float>(static_cast<std::int32_t>(best));
    v[6] = static_cast<float>(hi);
    v[15] = value_15;
    // 00BAA3D2..00BAA3E4: f = float(round(double(float(-offset)))).
    const float neg = -offset;
    const float f = static_cast<float>(host.crt_round_00bf85b0(neg));
    v[10] = f;
    const float n1 = u32_to_float_b(count - 1u);
    const float t = static_cast<float>(1.0 + f);
    v[11] = (n1 > t) ? t : n1;  // 00BAA422 FCOMIP+JBE: NaN keeps n1
    v[12] = static_cast<float>(static_cast<double>(neg) - f);
    // 00BAA443..00BAA470: UCOMISS equality pair, unordered -> 1.0.
    v[13] = (f == 0.0f && v[11] == 1.0f) ? 0.0f : 1.0f;
    v[14] = value_14;
    const double wl = at(ws, lo), wb = at(ws, best), wh = at(ws, hi);
    const double sum = (wh + wl) + wb;  // pseudocode order
    v[7] = static_cast<float>(wl / sum);
    v[8] = static_cast<float>(wb / sum);
    v[9] = static_cast<float>(wh / sum);
    return v;
}

void tracer_update_bones_00baa8f0(TracerHost& host, const TracerBoneState& st,
                                  std::vector<TracerPoint>& pts, int head) {
    const float a = -0.0f - st.sin_240;  // 00BAA903..00BAA90E
    const float c = st.cos_23c;
    const float s = st.sin_240;
    float distance = 0.0f;
    for (int i = head; i >= 0; i = pts[static_cast<std::size_t>(i)].next_28) {
        TracerPoint& p = pts[static_cast<std::size_t>(i)];
        if (st.curve_limit_4c > distance) {
            // The curve slot +8 is called with distance/limit and its ST0 result is popped
            // unused (00BAA980 FSTP ST0); the squeeze below is constant.
            const float dx = static_cast<float>(static_cast<double>(p.x_00) - st.origin_234.x);
            const float dy = static_cast<float>(static_cast<double>(p.y_04) - st.origin_234.y);
            const float lx = static_cast<float>(static_cast<double>(a) * dx + static_cast<double>(c) * dy);
            const float ly = static_cast<float>(static_cast<double>(s) * dy + static_cast<double>(c) * dx);
            // qword 00D7A270 = 0.05000000074505806.
            const float lx2 = static_cast<float>(lx - 0.05000000074505806 * lx);
            const float nx = static_cast<float>(static_cast<double>(lx2) * a + static_cast<double>(ly) * c);
            const float ny = static_cast<float>(static_cast<double>(lx2) * c + static_cast<double>(s) * ly);
            p.x_00 = static_cast<float>(static_cast<double>(st.origin_234.x) + nx);
            p.y_04 = static_cast<float>(static_cast<double>(st.origin_234.y) + ny);
        }
        distance = static_cast<float>(static_cast<double>(distance) + p.segment_length_14);
    }
    for (int i = head; i >= 0; i = pts[static_cast<std::size_t>(i)].next_28) {
        TracerPoint& p = pts[static_cast<std::size_t>(i)];
        float tx, ty;
        if (p.toward_2c < 0) {
            tx = st.origin_234.x;
            ty = st.origin_234.y;
        } else {
            tx = pts[static_cast<std::size_t>(p.toward_2c)].x_00;
            ty = pts[static_cast<std::size_t>(p.toward_2c)].y_04;
        }
        const float ex = tx - p.x_00;
        const float ey = ty - p.y_04;
        const float r = host.reciprocal_length_00419260(ex, ey);
        p.dir_x_08 = r * ex;
        p.dir_y_0c = r * ey;
        const float sq = static_cast<float>(static_cast<double>(ey) * ey + static_cast<double>(ex) * ex);
        p.segment_length_14 = static_cast<float>(host.crt_sqrt_00bf7030(sq));
    }
}

float tracer_sample_water_contact_00bab3f0(TracerHost& host, const TracerContactState& st,
                                           const std::vector<TracerPoint>& bones,
                                           const TracerVertex& v) {
    // 00BAB3FD: UCOMISS v[13], float 00D7A218 (0.0); equal -> first segment length.
    const float scale = (v[13] == 0.0f) ? st.first_segment_length : st.scale_1d8;
    auto idx = [](float f) { return static_cast<std::int32_t>(static_cast<std::int64_t>(f)); };
    const std::int32_t i_lo = idx(v[4]), i_b = idx(v[5]), i_hi = idx(v[6]);
    const std::int32_t i_f = idx(v[10]), i_m = idx(v[11]);
    auto bone = [&](std::int32_t k) -> const TracerPoint& { return bones.at(static_cast<std::size_t>(k)); };
    const float t = v[12];
    const float a_d = static_cast<float>(static_cast<double>(bone(i_f).depth_10) - st.depth_base_1c4);
    const float b_d = static_cast<float>(static_cast<double>(bone(i_m).depth_10) - st.depth_base_1c4);
    const float interp = static_cast<float>((static_cast<double>(b_d) - a_d) * t + a_d);
    const float nd = -interp;
    // 00BAB5D6..00BAB5FB: s = float(sqrt(float((+1E8h + nd) * +1E4h))).
    const float arg = static_cast<float>((static_cast<double>(st.depth_offset_1e8) + nd) * st.depth_scale_1e4);
    const float s = static_cast<float>(host.crt_sqrt_00bf7030(arg));
    // qword 00D7A328 = 4.0; FSIN of the float-rounded product.
    const float sin_arg = static_cast<float>(static_cast<double>(nd) * 4.0);
    const float sn = static_cast<float>(std::sin(static_cast<double>(sin_arg)));
    const float wf = bone(i_f).width_1c, wm = bone(i_m).width_1c;
    const float width = static_cast<float>((static_cast<double>(wm) - wf) * t + wf);
    // qword 00D7A3A0 = 0.10000000149011612.
    const float q = static_cast<float>(static_cast<double>(v[0]) * s * width +
                                       static_cast<double>(sn) * 0.10000000149011612);
    auto place = [&](const TracerPoint& b, float k) {
        float px = static_cast<float>(static_cast<double>(k) * b.dir_x_08);
        float py = static_cast<float>(static_cast<double>(k) * b.dir_y_0c);
        px = static_cast<float>(static_cast<double>(px) * scale);
        py = static_cast<float>(static_cast<double>(py) * scale);
        return W1B_Vec2{static_cast<float>(static_cast<double>(b.x_00) + px),
                        static_cast<float>(static_cast<double>(b.y_04) + py)};
    };
    const W1B_Vec2 p_lo = place(bone(i_lo), v[1]);
    const W1B_Vec2 p_b = place(bone(i_b), v[2]);
    const W1B_Vec2 p_hi = place(bone(i_hi), v[3]);
    auto perp = [](const TracerPoint& b) { return W1B_Vec2{b.dir_y_0c, -0.0f - b.dir_x_08}; };
    auto mul = [](W1B_Vec2 p, float k) {
        return W1B_Vec2{static_cast<float>(static_cast<double>(p.x) * k),
                        static_cast<float>(static_cast<double>(p.y) * k)};
    };
    auto add = [](W1B_Vec2 p, W1B_Vec2 r) {
        return W1B_Vec2{static_cast<float>(static_cast<double>(p.x) + r.x),
                        static_cast<float>(static_cast<double>(p.y) + r.y)};
    };
    const float w_lo = v[7], w_b = v[8], w_hi = v[9];
    const W1B_Vec2 n = add(add(mul(perp(bone(i_lo)), w_lo), mul(perp(bone(i_b)), w_b)),
                           mul(perp(bone(i_hi)), w_hi));
    const float r = host.reciprocal_length_00419260(n.x, n.y);
    const W1B_Vec2 nq = mul(mul(n, r), q);
    const W1B_Vec2 pos = add(add(add(mul(p_lo, w_lo), mul(p_b, w_b)), mul(p_hi, w_hi)), nq);
    // qword 00D7A280 = 0.5.
    return static_cast<float>(host.ocean_sample_height_0078c890(pos.x, pos.y) * 0.5);
}

std::vector<TracerVertex> tracer_rebuild_vertices_00bac730(TracerHost& host,
                                                           const TracerContactState& st,
                                                           const std::vector<TracerPoint>& bones,
                                                           const std::vector<TracerVertex>& src) {
    std::vector<TracerVertex> out;
    out.reserve(src.size());
    for (const TracerVertex& v : src) {
        TracerVertex d = v;
        d[14] = tracer_sample_water_contact_00bab3f0(host, st, bones, d);
        out.push_back(d);
    }
    return out;
}

std::vector<std::array<float, 8>> tracer_conform_to_water_00bab930(
    TracerHost& host, const std::vector<std::array<float, 8>>& src) {
    std::vector<std::array<float, 8>> out;
    out.reserve(src.size());
    for (const auto& v : src) {
        const W1B_Vec4 w = host.transform_point_00b62d10(v.data());
        auto d = v;
        d[1] = static_cast<float>(host.ocean_sample_height_0078c890(w.x, w.z) * 0.5 + d[1]);
        out.push_back(d);
    }
    return out;
}

int tracer_render_00baba50(TracerHost& host, TracerRenderState& st, int pass_id,
                           const std::vector<TracerPoint>& pts, int head) {
    if (pass_id != 4) return 0;
    if (!st.static_202) {
        st.out_798 = st.value_18c;
        st.header_268 = {head >= 0 ? pts[static_cast<std::size_t>(head)].segment_length_14 : 0.0f,
                         st.value_1e4, st.value_1e8, 0.0f, st.value_1d8, st.value_1ec,
                         st.value_1fc, st.value_1c4};
        st.rows.clear();
        for (int i = head; i >= 0; i = pts[static_cast<std::size_t>(i)].next_28) {
            const TracerPoint& p = pts[static_cast<std::size_t>(i)];
            TracerRenderRow row;
            row.x = p.x_00;
            row.y = p.y_04;
            const float r = host.reciprocal_length_00419260(p.dir_x_08, p.dir_y_0c);
            const float nx = static_cast<float>(static_cast<double>(p.dir_x_08) * r);
            const float ny = static_cast<float>(static_cast<double>(r) * p.dir_y_0c);
            // qword 00D7A258 = 0.0: dot with (1, 0).
            const float cs = static_cast<float>(static_cast<double>(ny) * 0.0 + nx);
            float angle;
            if (cs > 1.0f) angle = 0.0f;
            else if (-1.0f > cs) angle = 3.14159274101257324f;  // float 00D7A264
            else angle = static_cast<float>(host.crt_acos_00bf9940(cs));
            if (ny < 0.0f) angle = static_cast<float>(6.2831854820251465 - angle);  // qword 00CE3828
            row.angle = angle;
            row.value_1c = p.width_1c;
            // qword 00D7A348 = 0.25.
            const float k = static_cast<float>(0.25 / (static_cast<double>(st.value_1c4) - p.depth_10));
            float clamped = 0.0f;
            if (0.0f <= k) clamped = (k > 1.0f) ? 1.0f : k;
            row.fade = static_cast<float>((1.0 - clamped) * p.value_24);
            row.depth = static_cast<float>(static_cast<double>(p.depth_10) - st.value_1c4);
            row.width = static_cast<float>(static_cast<double>(p.value_18) * st.value_1f0);
            row.value_20 = p.value_20;
            st.rows.push_back(row);
        }
    } else {
        st.out_258 = st.value_204;
        st.out_25c = st.value_1dc;
        const float f = static_cast<float>(static_cast<double>(st.value_1f4) * st.value_1dc * st.value_208);
        st.out_260 = (0.0f <= f) ? ((f > 1.0f) ? 1.0f : f) : 0.0f;
    }
    return 1;
}

std::vector<std::uint16_t> tracer_static_mesh_indices_00bacbd0(
    const std::vector<std::uint32_t>& ring) {
    std::vector<std::uint16_t> idx;
    if (ring.size() < 2) return idx;
    std::int32_t row_a = 0;
    std::int32_t row_b = static_cast<std::int32_t>(ring[0]);
    for (std::size_t r = 0; r + 1 < ring.size(); ++r) {
        std::int32_t a = row_a, b = row_b;
        for (std::uint32_t k = ring[r + 1] - 2u + ring[r]; k != 0; --k) {
            idx.push_back(static_cast<std::uint16_t>(a));
            idx.push_back(static_cast<std::uint16_t>(b));
            idx.push_back(static_cast<std::uint16_t>(a + 1));
            const std::int32_t na = b;
            b = a + 1;
            a = na;
        }
        row_a += static_cast<std::int32_t>(ring[r]);
        row_b += static_cast<std::int32_t>(ring[r + 1]);
    }
    return idx;
}

// =============================================================================================
// Coastline extractor
CoastlineState coastline_construct_00bb02c0() {
    CoastlineState s;
    s.water_level_2c = 0.0f;
    s.tolerance_34 = 0.100000001490116119f;  // float 00D7A2F0
    return s;
}

void coastline_set_time_00baec50(CoastlineState& s, float time) { s.time_bc = time; }

bool coastline_edge_crossing_00baeb20(const CoastlineState& s, const W1B_Vec3& p,
                                      const W1B_Vec3& q, W1B_Vec3& out) {
    const float level = s.water_level_2c;
    if (p.y == level) {  // FUCOMIP, unordered counts as not equal
        out = p;
        return true;
    }
    if (q.y == level) {
        out = q;
        return true;
    }
    if (level < p.y && level < q.y) return false;
    if (p.y < level && q.y < level) return false;
    // Pseudocode reading: t = p.y / (p.y - q.y); the level is not subtracted (exact only for 0).
    const float t = static_cast<float>(static_cast<double>(p.y) / (static_cast<double>(p.y) - q.y));
    out.x = static_cast<float>(t * (static_cast<double>(q.x) - p.x) + p.x);
    out.y = level;
    out.z = static_cast<float>(p.z + (static_cast<double>(q.z) - p.z) * t);
    return true;
}

CoastPoint coast_segment_construct_00baed40(CoastHost& host, const W1B_Vec3& a,
                                            const W1B_Vec3& b) {
    CoastPoint pt;
    pt.a = a;
    pt.b = b;
    const float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    const float len = static_cast<float>(host.vector_length_00419440(dx, dy, dz));
    const float inv = (len > 0.0f) ? static_cast<float>(1.0 / len) : 0.0f;
    const W1B_Vec3 dir{static_cast<float>(static_cast<double>(inv) * dx),
                       static_cast<float>(static_cast<double>(inv) * dy),
                       static_cast<float>(static_cast<double>(inv) * dz)};
    // 004F9B30 cross(a = up (0,1,0), b = dir).
    const W1B_Vec3 up{0.0f, 1.0f, 0.0f};
    pt.normal = {up.y * dir.z - up.z * dir.y, dir.x * up.z - up.x * dir.z,
                 dir.y * up.x - up.y * dir.x};
    pt.u0_24 = 0.0f;
    pt.u1_28 = static_cast<float>(host.vector_length_00419440(b.x - a.x, b.y - a.y, b.z - a.z));
    return pt;
}

std::int32_t coast_ptr_vector_reserve_capacity_00baeeb0(std::int32_t capacity,
                                                        std::int32_t request) {
    if (request < 0x20) request = 0x20;
    return capacity < request ? request : capacity;
}
std::int32_t coast_segment_vector_reserve_capacity_00baefb0(std::int32_t capacity,
                                                            std::int32_t request) {
    if (request < 0x20) request = 0x20;
    return capacity < request ? request : capacity;
}
std::int32_t coast_point_vector_reserve_capacity_00baf450(std::int32_t capacity,
                                                          std::int32_t request) {
    if (request < 0x20) request = 0x20;
    return capacity < request ? request : capacity;
}
std::int32_t coast_segment_vector_push_capacity_00baf350(std::int32_t size, std::int32_t capacity) {
    if (size != capacity) return capacity;
    std::int32_t c = capacity * 2;
    if (c < 0x21) c = 0x20;
    return coast_segment_vector_reserve_capacity_00baefb0(capacity, c);
}
std::int32_t coast_ptr_vector_push_capacity_00baf3e0(std::int32_t size, std::int32_t capacity) {
    if (size != capacity) return capacity;
    std::int32_t c = capacity * 2;
    if (c < 0x21) c = 0x20;
    return coast_ptr_vector_reserve_capacity_00baeeb0(capacity, c);
}
std::int32_t coast_point_vector_push_capacity_00bafc00(std::int32_t size, std::int32_t capacity) {
    if (size != capacity) return capacity;
    std::int32_t c = capacity * 2;
    if (c < 0x21) c = 0x20;
    return coast_point_vector_reserve_capacity_00baf450(capacity, c);
}
void coast_ptr_vector_resize_00baf070(std::vector<std::uint32_t>& v, std::int32_t size) {
    v.resize(static_cast<std::size_t>(size < 0 ? 0 : size), 0u);  // new slots zeroed
}
std::uint32_t pak_archive_registry_slots_push_capacity_00bb4b40(std::int32_t size,
                                                                std::int32_t capacity) {
    if (size != capacity) return static_cast<std::uint32_t>(capacity);
    std::int32_t c = capacity * 2;
    if (c < 2) c = 1;
    return static_cast<std::uint32_t>(c);
}

int coastline_find_nearest_segment_00baf0c0(CoastHost& host, const CoastlineState& s,
                                            const W1B_Vec3& p, std::uint32_t& out_index,
                                            std::uint32_t exclude) {
    for (std::uint32_t i = 0; i < s.segments.size(); ++i) {
        if (i == exclude) continue;
        const CoastSegment& g = s.segments[i];
        if ((g.flags_18 & 1u) == 0) {
            const double len = host.vector_length_00419440(p.x - g.a.x, p.y - g.a.y, p.z - g.a.z);
            if (static_cast<double>(s.tolerance_34) >= len) {  // FCOMIP+JAE, NaN fails
                out_index = i;
                return 0;
            }
        }
        if ((g.flags_18 & 2u) == 0) {
            const double len = host.vector_length_00419440(p.x - g.b.x, p.y - g.b.y, p.z - g.b.z);
            if (static_cast<double>(s.tolerance_34) >= len) {
                out_index = i;
                return 1;
            }
        }
    }
    return -1;
}

void coastline_clip_triangle_00baf5c0(CoastHost& host, CoastlineState& s, const W1B_Vec3& p0,
                                      const W1B_Vec3& p1, const W1B_Vec3& p2) {
    const float level = s.water_level_2c;
    if (p0.y == level && p1.y == level) {
        s.segments.push_back({p0, p1, 0u});
        return;
    }
    if (p1.y == level && p2.y == level) {
        s.segments.push_back({p1, p2, 0u});
        return;
    }
    if (p2.y == level && p0.y == level) {
        s.segments.push_back({p2, p0, 0u});
        return;
    }
    W1B_Vec3 c[2]{};
    std::uint32_t n = coastline_edge_crossing_00baeb20(s, p0, p1, c[0]) ? 1u : 0u;
    if (coastline_edge_crossing_00baeb20(s, p1, p2, c[n])) ++n;
    if (n == 1) {
        if (!coastline_edge_crossing_00baeb20(s, p2, p0, c[1])) return;
    } else if (n != 2) {
        return;
    }
    const double len = host.vector_length_00419440(c[1].x - c[0].x, c[1].y - c[0].y,
                                                   c[1].z - c[0].z);
    if (static_cast<double>(s.tolerance_34) < len) s.segments.push_back({c[0], c[1], 0u});
}

void coastline_clip_draw_section_00baf8a0(CoastHost& host, CoastlineState& s,
                                          const std::vector<W1B_Vec3>& positions,
                                          const std::vector<std::uint16_t>& indices,
                                          std::uint32_t first, std::uint32_t triangles,
                                          float y_offset) {
    for (std::uint32_t t = 0, k = 0; t < triangles; ++t, k += 3) {
        W1B_Vec3 v[3];
        for (int j = 0; j < 3; ++j) {
            v[j] = positions.at(indices.at(first + k + static_cast<std::uint32_t>(j)));
            v[j].y = static_cast<float>(static_cast<double>(v[j].y) + y_offset);  // 00BAF929..58
        }
        coastline_clip_triangle_00baf5c0(host, s, v[0], v[1], v[2]);
    }
}

bool coastline_collect_terrain_section_matches_00baf9a0(std::string_view effect_name) {
    return iequals_b(effect_name, "bterrain.mshd");  // __stricmp, string literal in image
}

int coastline_chain_segments_00bafaa0(CoastHost& host, const CoastlineState& s,
                                      std::uint32_t& out_segment) {
    std::uint32_t start = 0;
    for (; start < s.segments.size(); ++start)
        if (s.segments[start].flags_18 == 0) break;
    if (start >= s.segments.size()) {
        out_segment = 0xFFFFFFFFu;
        return -1;
    }
    std::uint32_t cur = start;
    int end_sel = 0;
    std::uint32_t found = 0;
    int r = coastline_find_nearest_segment_00baf0c0(host, s, s.segments[start].a, found, start);
    while (r != -1 && found != start) {
        end_sel = (r == 0) ? 1 : 0;
        cur = found;
        const W1B_Vec3 probe = end_sel ? s.segments[cur].b : s.segments[cur].a;
        r = coastline_find_nearest_segment_00baf0c0(host, s, probe, found, cur);
    }
    out_segment = cur;
    return end_sel;
}

void coastline_resample_polyline_00bafce0(CoastHost& host, const CoastlineState& s,
                                          std::vector<CoastPoint>& poly, const W1B_Vec3& a,
                                          const W1B_Vec3& b, float& acc) {
    const float spacing = 10.0f;  // float 00E14458 (sole reference 00BAFD41)
    const float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    const float len = static_cast<float>(host.vector_length_00419440(dx, dy, dz));
    auto push = [&](const W1B_Vec3& p, const W1B_Vec3& q) {
        CoastPoint pt = coast_segment_construct_00baed40(host, p, q);
        pt.u0_24 = static_cast<float>(static_cast<double>(pt.u0_24) + acc);
        pt.u1_28 = static_cast<float>(static_cast<double>(s.u_scale_fc) * pt.u1_28);
        pt.u1_28 = static_cast<float>(static_cast<double>(acc) + pt.u1_28);
        acc = pt.u1_28;
        poly.push_back(pt);
    };
    if (!(len > spacing)) {  // 00BAFD49 FCOMI+JBE
        push(a, b);
        return;
    }
    const float q = static_cast<float>(static_cast<double>(len) / spacing + 1.0);
    const float rounded = static_cast<float>(host.crt_round_00bf85b0(q));
    const std::uint32_t n = chop_to_u32_b(rounded);
    const float nf = u32_to_float_b(n);
    const float sx = static_cast<float>(static_cast<double>(dx) / nf);
    const float sy = static_cast<float>(static_cast<double>(dy) / nf);
    const float sz = static_cast<float>(static_cast<double>(dz) / nf);
    auto at = [&](std::uint32_t i) {
        const float fi = u32_to_float_b(i);
        return W1B_Vec3{static_cast<float>(static_cast<double>(a.x) + static_cast<float>(static_cast<double>(fi) * sx)),
                        static_cast<float>(static_cast<double>(static_cast<float>(static_cast<double>(fi) * sy)) + a.y),
                        static_cast<float>(static_cast<double>(a.z) + static_cast<float>(static_cast<double>(fi) * sz))};
    };
    for (std::uint32_t i = 0; i < n; ++i) push(at(i), (i == n - 1u) ? b : at(i + 1u));
}

bool coastline_load_data_00bb0070(CoastStreamHost& in, CoastlineState& s) {
    if (!in.exists_08()) return false;
    s.count_30 = static_cast<std::uint32_t>(in.read_int_38());
    std::int32_t lines = in.read_int_38();
    for (; lines != 0; --lines) {
        std::int32_t n = in.read_int_38();
        std::vector<CoastPoint> poly;
        for (; n != 0; --n) {
            float f[11];
            for (float& x : f) x = in.read_float_44();
            CoastPoint p;
            p.a = {f[0], f[1], f[2]};
            p.b = {f[3], f[4], f[5]};
            p.normal = {f[6], f[7], f[8]};
            p.u0_24 = f[9];
            p.u1_28 = f[10];
            poly.push_back(p);
        }
        s.polylines.push_back(std::move(poly));
    }
    return true;
}

void coastline_build_polylines_00bb05e0(CoastHost& host, CoastlineState& s) {
    s.count_30 = 0;
    std::uint32_t seg = 0;
    int sel = coastline_chain_segments_00bafaa0(host, s, seg);
    while (seg != 0xFFFFFFFFu) {
        std::vector<CoastPoint> poly;
        float acc = 0.0f;
        {
            const CoastSegment g = s.segments[seg];
            coastline_resample_polyline_00bafce0(host, s, poly, sel == 0 ? g.a : g.b,
                                                 sel == 0 ? g.b : g.a, acc);
            s.segments[seg].flags_18 = (sel == 0) ? 2u : 1u;
        }
        std::uint32_t next = 0;
        int r;
        while ((r = coastline_find_nearest_segment_00baf0c0(host, s, poly.back().b, next,
                                                            0xFFFFFFFFu)) != -1) {
            s.segments[next].flags_18 = 3u;
            const CoastSegment g = s.segments[next];
            coastline_resample_polyline_00bafce0(host, s, poly, r == 0 ? g.a : g.b,
                                                 r == 0 ? g.b : g.a, acc);
        }
        if (poly.size() >= 2) {
            s.count_30 += static_cast<std::uint32_t>(poly.size());
            s.polylines.push_back(std::move(poly));
        }
        sel = coastline_chain_segments_00bafaa0(host, s, seg);
    }
    s.segments.clear();
}

void coastline_build_mesh_00bb07c0(CoastHost& host, CoastlineState& s) {
    for (;;) {
        std::size_t k = 0;
        const std::uint32_t max_n = s.max_points_c0;
        while (k < s.polylines.size() && s.polylines[k].size() <= max_n) ++k;
        if (k >= s.polylines.size()) return;
        const std::vector<CoastPoint> src = s.polylines[k];
        const std::uint32_t size = static_cast<std::uint32_t>(src.size());
        const float q = static_cast<float>(
            static_cast<double>(static_cast<float>(static_cast<std::int32_t>(size))) /
                u32_to_float_b(max_n) + 1.0);
        std::uint32_t chunks = chop_to_u32_b(static_cast<float>(host.crt_round_00bf85b0(q)));
        std::uint32_t start = 0;
        for (; chunks != 0; --chunks) {
            const std::uint32_t end = std::min(max_n + start, size);
            std::vector<CoastPoint> part;
            for (std::uint32_t i = start; i < end; ++i) part.push_back(src[i]);
            if (!part.empty()) s.polylines.push_back(std::move(part));
            start = end;
        }
        // Remove the original: swap with the last slot, then pop.
        if (k != s.polylines.size() - 1) s.polylines[k] = std::move(s.polylines.back());
        s.polylines.pop_back();
    }
}

std::uint32_t coastline_build_count_00bb09f0(std::uint32_t polyline_count,
                                             std::uint32_t point_count) {
    return (polyline_count + point_count) * 2u + 2u;
}

void coast_mesh_set_time_00bb0a90(float& time_120, float time) { time_120 = time; }

CoastMarchingResult coast_mesh_classify_cells_00bb11a0(const std::vector<float>& h,
                                                       std::uint32_t rows,
                                                       std::uint32_t cols) {
    CoastMarchingResult res;
    res.cells.resize(static_cast<std::size_t>(rows) * cols);
    for (std::uint32_t r = 0; r < rows; ++r) {
        for (std::uint32_t c = 0; c < cols; ++c) {
            const std::size_t i = static_cast<std::size_t>(r) * cols + c;
            const float h00 = h[i];
            const float h01 = (c == cols - 1u) ? 0.0f : h[i + 1];
            const float h10 = (r == rows - 1u) ? 0.0f : h[i + cols];
            const float h11 = (r == rows - 1u || c == cols - 1u) ? 0.0f : h[i + cols + 1];
            const bool b00 = 0.0f < h00, b01 = 0.0f < h01, b10 = 0.0f < h10, b11 = 0.0f < h11;
            const int code = b00 + 2 * (b01 + 2 * (b10 + 2 * b11));
            CoastMarchingCell& cell = res.cells[i];
            if (b00 == b01 && b00 == b10 && b00 == b11) continue;
            if (b00 != b10 && r < rows - 1u) cell.edge_down = static_cast<std::int32_t>(res.edge_vertex_count_24++);
            if (b00 != b01 && c < cols - 1u) cell.edge_right = static_cast<std::int32_t>(res.edge_vertex_count_24++);
            if (r < rows - 1u && c < cols - 1u) {
                ++res.segment_count_28;
                if (code == 6 || code == 9) ++res.segment_count_28;  // saddles
            }
        }
    }
    return res;
}

bool coast_mesh_distance_map_header_ok_00bb2df0(const std::uint8_t* magic8, std::uint32_t rows,
                                                std::uint32_t columns, std::uint32_t rows_34,
                                                std::uint32_t columns_38) {
    static const char kMagic[8] = {'M', 'i', 't', 'C', 'o', 'a', 's', 't'};  // 00E14460
    for (int i = 0; i < 8; ++i)
        if (static_cast<char>(magic8[i]) != kMagic[i]) return false;
    return rows == rows_34 && columns == columns_38;
}

std::array<float, 6> coast_mesh_decode_vertex_00bb3370(const CoastPackedVertex& v, float sx28,
                                                       float sz2c, float outer58, float inner5c) {
    // qword 00CE5F98 = 32767.0, qword 00D64148 = 1/127, qword 00D19918 = 1/256.
    const float kx = static_cast<float>(static_cast<double>(sx28) / 32767.0);
    const float kz = static_cast<float>(static_cast<double>(sz2c) / 32767.0);
    const float x = static_cast<float>(static_cast<double>(v.x) * kx);
    const float z = static_cast<float>(static_cast<double>(v.z) * kz);
    const float nx = static_cast<float>(static_cast<double>(v.nx) * 0.007874015718698502);
    const float nz = static_cast<float>(static_cast<double>(v.nz) * 0.007874015718698502);
    const float d = static_cast<float>(static_cast<double>(v.distance) * 0.00390625);
    return {static_cast<float>(x - static_cast<double>(nx) * inner5c), d,
            static_cast<float>(z - static_cast<double>(nz) * inner5c),
            static_cast<float>(static_cast<double>(outer58) * nx + x), -0.0f - d,
            static_cast<float>(static_cast<double>(nz) * outer58 + z)};
}

// =============================================================================================
// MSAR / MPKG
MsarHeader msar_read_header_00bba090(const std::uint8_t* b) {
    MsarHeader h;
    static const char kMagic[4] = {'M', 'S', 'A', 'R'};  // 00D643BC
    h.magic_ok = true;
    for (int i = 0; i < 4; ++i)
        if (b[i] != static_cast<std::uint8_t>(kMagic[i])) h.magic_ok = false;
    h.crc_04 = read_u32_le_b(b + 4);
    h.toc_size_08 = read_u32_le_b(b + 8);
    h.data_size_0c = read_u32_le_b(b + 12);
    // 00BBA144..00BBA204: initial state 61746973h (XOR 73h / XOR 617469h), bytes 8..15, NOT.
    h.header_crc_10 = ~crc32_update_b(0x61746973u, b + 8, 8);
    return h;
}

std::vector<MsarEntry> msar_read_toc_00bbb890(const std::vector<std::uint8_t>& ar) {
    std::vector<MsarEntry> out;
    if (ar.size() < 16) return out;
    const MsarHeader h = msar_read_header_00bba090(ar.data());
    const std::size_t toc_len = static_cast<std::size_t>(h.toc_size_08) + 4u;
    std::vector<std::uint8_t> toc(toc_len, 0);
    for (std::size_t i = 0; i < toc_len && 16 + i < ar.size(); ++i) toc[i] = ar[16 + i];
    // The 'TOC' (00D64234) comparison loop exits early on mismatch but its result is unused.
    if (toc_len < 6) return out;
    std::size_t p = 4;
    const std::uint32_t count = static_cast<std::uint32_t>(toc[p]) | (static_cast<std::uint32_t>(toc[p + 1]) << 8);
    p += 2;
    for (std::uint32_t e = 0; e < count; ++e) {
        p += 2;  // 00BBB946: two bytes skipped before every entry
        if (p + 14 > toc.size()) break;
        MsarEntry m;
        m.offset_14 = read_u32_le_b(&toc[p]) + h.toc_size_08 + 0x10u;
        m.packed_size_0c = read_u32_le_b(&toc[p + 4]);
        m.size_08 = read_u32_le_b(&toc[p + 8]);
        m.compressed_10 = toc[p + 12] != 0;
        p += 13;
        // 00BB5630: length prefix byte, copy to NUL, advance by prefix + 1.
        const std::uint8_t prefix = toc[p];
        ++p;
        std::size_t q = p;
        while (q < toc.size() && toc[q] != 0) ++q;
        m.name.assign(reinterpret_cast<const char*>(toc.data() + p), q - p);
        p += static_cast<std::size_t>(prefix) + 1u;
        out.push_back(std::move(m));
    }
    return out;
}

bool msar_verify_checksum_00bbbab0(const std::vector<std::uint8_t>& ar,
                                   std::uint64_t position_after, std::uint64_t length) {
    if (ar.size() < 16) return false;
    const MsarHeader h = msar_read_header_00bba090(ar.data());
    if (!h.magic_ok) return false;
    std::uint32_t crc = ~h.header_crc_10;
    std::uint32_t remaining = h.toc_size_08 + h.data_size_0c;
    std::size_t pos = 16;
    while (remaining != 0) {
        const std::uint32_t chunk = remaining < 0x10000u ? remaining : 0x10000u;
        if (pos + chunk > ar.size()) break;  // short read stops the loop
        crc = crc32_update_b(crc, ar.data() + pos, chunk);
        pos += chunk;
        remaining -= chunk;
    }
    if (remaining != 0 || ~crc != h.crc_04) return false;
    return position_after == length;  // stream slots +30h vs +20h, then ReadToc
}

bool msar_validate_archive_00bbbb90(const std::vector<std::uint8_t>& ar) {
    const std::uint64_t consumed = 16ull + (ar.size() >= 16 ? static_cast<std::uint64_t>(
        msar_read_header_00bba090(ar.data()).toc_size_08) + msar_read_header_00bba090(ar.data()).data_size_0c : 0ull);
    return msar_verify_checksum_00bbbab0(ar, consumed, ar.size());
}

bool msar_path_has_suffix_00bbbbf0(std::string_view path) {
    if (!(4 < path.size())) return false;
    return iequals_b(path.substr(path.size() - 5), ".msar");
}

MsarOpenPlan msar_open_entry_by_index_00bba500(const std::vector<MsarEntry>& entries,
                                               std::uint32_t index) {
    MsarOpenPlan p;
    if (entries.empty() || index >= entries.size()) return p;
    const MsarEntry& e = entries[index];
    p.valid = true;
    p.offset = e.offset_14;
    p.output_size = e.size_08;
    if (!e.compressed_10) {
        p.length = e.size_08;
    } else {
        p.inflate = true;
        p.length = e.packed_size_0c;
        p.inflate_input_buffer = 0x4000u;
        p.inflate_output_buffer = 0x10000u;
    }
    return p;
}

std::optional<std::uint32_t> msar_open_entry_index_00bba7c0(const std::vector<MsarEntry>& entries,
                                                            std::string_view name,
                                                            std::uint8_t mode) {
    if (mode & 1u) return std::nullopt;
    // 00BBA650 BSP_MSARFile_FindFileIndex (existing reconstruction, src/native_vfs_lookup_leaves.cpp)
    // decides the match; modelled here as an exact name compare.
    for (std::uint32_t i = 0; i < entries.size(); ++i)
        if (entries[i].name == name) return i;
    return std::nullopt;
}

std::vector<std::string> msar_enumerate_names_00bbad60(MsarNameFilterHost& filter,
                                                       const std::vector<MsarEntry>& entries,
                                                       std::string_view directory,
                                                       std::string& normalized) {
    normalized.assign(directory.data(), directory.size());
    // Native reads directory[len-1] unconditionally (out of range for an empty string).
    if (normalized.empty() || normalized.back() != '/') normalized += '/';
    std::vector<std::string> out;
    for (const MsarEntry& e : entries)
        if (filter.matches_enumeration_filter(normalized, e)) out.push_back(e.name);
    return out;
}

std::vector<std::uint8_t> mpkg_decode_stream_to_memory_00bb9180(
    const std::vector<std::uint8_t>& enc, const std::uint8_t* key, std::size_t key_size) {
    const std::uint32_t size = static_cast<std::uint32_t>(enc.size());
    std::vector<std::uint8_t> out(size);
    const std::uint32_t final_block = size / 0x2F1u;
    for (std::uint32_t i = 0; i < size; ++i) {
        const std::uint32_t block = i / 0x2F1u;
        const std::uint32_t len = (block == final_block) ? size - final_block * 0x2F1u : 0x2F1u;
        const std::uint32_t src = block * 0x2F1u - i % len - 1u + len;
        const std::size_t k = static_cast<std::size_t>(static_cast<std::int32_t>(src) % 0x219);
        out[i] = static_cast<std::uint8_t>((k < key_size ? key[k] : 0u) ^ enc[src]);
    }
    return out;
}

MemoryBlockState memory_block_allocate_00bbbcf0(std::size_t size) {
    MemoryBlockState b;
    b.storage.resize(size);
    b.data_end_04 = 0;
    b.capacity_end_08 = size;
    b.cursor_0c = 0;
    return b;
}

std::size_t memory_block_read_00bbbed0(MemoryBlockState& b, std::uint8_t* out, std::size_t n) {
    const std::size_t avail = b.data_end_04 - b.cursor_0c;
    if (avail <= n) n = avail;
    if (n) std::memcpy(out, b.storage.data() + b.cursor_0c, n);
    b.cursor_0c += n;
    return n;
}

std::uint64_t inflate_stream_rewind_00bbbde0(InflateStreamPositions& s) {
    s.produced_24 = 0;
    s.remaining_2c = s.total_14;
    s.remaining_30 = s.total_18;
    return s.base_offset_10;
}

InflateStateConfig inflate_state_create_config_00bbbe60() {
    // 00BBBE61 new 38h; zalloc/zfree/opaque (+20h..+28h) and next_in/avail_in zeroed;
    // 00BC96A0 inflateInit2_(ECX = strm, EDX = -15 raw deflate, "1.2.1" @ 00D643F8, 38h).
    return {0x38u, -15, "1.2.1", 0x38};
}

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part c: 00BBC000-00BC2FFF
// ================================================================================================
namespace bsp::water_weather_cc13 {

// ---------------------------------------------------------------------------
// Native arrays

namespace {
template <class T>
void w1c_array_resize_c(W1C_NativeArray<T>& a, std::int32_t n) {
    if (a.capacity < n) w1c_array_reserve_c(a, n);
    for (std::int32_t i = a.count; i < n; ++i) a.storage[static_cast<std::size_t>(i)] = T{};
    a.count = n;  // the native's decrement loop for n < count only lands on n
}

template <class T>
void w1c_array_push_back_c(W1C_NativeArray<T>& a, const T& v) {
    if (a.count == a.capacity) {
        std::int32_t grow = a.capacity * 2;
        if (grow < 2) grow = 1;
        w1c_array_reserve_c(a, grow);
    }
    a.storage[static_cast<std::size_t>(a.count)] = v;
    ++a.count;
}
}  // namespace

void cloud_class_vector_reserve_00bbd0b0(W1C_NativeArray<std::uint32_t>& a, std::int32_t n) { w1c_array_reserve_c(a, n); }
void cloud_instance_vector_reserve_00bbd110(W1C_NativeArray<std::uint32_t>& a, std::int32_t n) { w1c_array_reserve_c(a, n); }
void cloud_pointer_array_reserve_00bbd230(W1C_NativeArray<std::uint32_t>& a, std::int32_t n) { w1c_array_reserve_c(a, n); }
void sky_colour_strip_reserve_00bbf390(W1C_NativeArray<std::uint32_t>& a, std::int32_t n) { w1c_array_reserve_c(a, n); }
void cloud_class_vector_resize_00bbd170(W1C_NativeArray<std::uint32_t>& a, std::int32_t n) { w1c_array_resize_c(a, n); }
void cloud_instance_vector_resize_00bbd1d0(W1C_NativeArray<std::uint32_t>& a, std::int32_t n) { w1c_array_resize_c(a, n); }
void cloud_pointer_array_resize_00bbd640(W1C_NativeArray<std::uint32_t>& a, std::int32_t n) { w1c_array_resize_c(a, n); }
void cloud_class_vector_push_back_00bbd540(W1C_NativeArray<std::uint32_t>& a, std::uint32_t v) { w1c_array_push_back_c(a, v); }
void cloud_instance_vector_push_back_00bbd580(W1C_NativeArray<std::uint32_t>& a, std::uint32_t v) { w1c_array_push_back_c(a, v); }

bool cloud_pointer_vector_remove_swap_last_00bbd880(W1C_NativeArray<std::uint32_t>& a, std::uint32_t value) {
    // Unsigned count: the scan runs while begin < begin + count.
    const auto count = static_cast<std::uint32_t>(a.count);
    for (std::uint32_t i = 0; i < count; ++i) {
        if (a.storage[i] == value) {
            if (i != count - 1) a.storage[i] = a.storage[count - 1];
            a.count -= 1;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Texture sources

void caustics_texture_source_set_colour_00bbcba0(CausticsTextureSourceC& s, const std::array<float, 4>& rgba) noexcept {
    s.colour_20 = rgba;
}
void caustics_texture_source_set_scale_00bbcbc0(CausticsTextureSourceC& s, float scale) noexcept { s.scale_30 = scale; }
void shore_wave_texture_source_ignore_colour_00bbcbe0(const std::array<float, 4>&) noexcept {}
void shore_wave_texture_source_ignore_scale_00bbcbf0(float) noexcept {}

bool water_texture_source_accepts_type_token_00bbc690(const std::array<std::uint32_t, 4>& table_0109e9d4,
                                                     std::uint32_t token) noexcept {
    for (std::uint32_t t : table_0109e9d4)
        if (t == token) return true;
    return false;
}

// ---------------------------------------------------------------------------
// Cloud manager

CloudManagerC cloud_manager_construct_00bbda50(std::uint32_t owner, std::uint32_t group_node) {
    CloudManagerC m;
    m.refcount_04 = 1;
    m.owner_08 = owner;
    m.group_node_30 = group_node;
    m.field_34 = 2000.0f;                                 // 00CFFD60
    m.field_38 = {-99999.0f, -99999.0f, -99999.0f};       // 00D1A648
    m.vector_44 = {1.0f, 0.0f, 0.0f};                     // 00D7A24C
    m.vector_50 = {0.0f, 1.0f, 0.0f};
    // 00D64508 / 00D64504 / 00D64500 = 7/255, 42/255, 98/255 as floats.
    m.colour_60 = {0.027450980618596077f, 0.16470588743686676f, 0.3843137323856354f, 0.0f};
    m.colour_70 = {0.2f, 0.2f, 0.2f, 0.0f};               // 00CE54A0
    m.colour_80 = {0.8f, 0.8f, 0.8f, 0.0f};               // 00CE74F8
    m.light_94 = 0;
    m.enabled_98 = 1;
    m.wind_enabled_99 = 1;
    return m;
}

std::array<float, 3> cloud_manager_get_vector50_00bbccb0(const CloudManagerC& m) noexcept { return m.vector_50; }
std::array<float, 3> cloud_manager_get_vector44_00bbccd0(const CloudManagerC& m) noexcept { return m.vector_44; }
std::array<float, 4> cloud_manager_get_colour60_00bbcd30(const CloudManagerC& m) noexcept { return m.colour_60; }
void cloud_manager_set_colour60_00bbcd50(CloudManagerC& m, float r, float g, float b, float a) noexcept { m.colour_60 = {r, g, b, a}; }
std::array<float, 4> cloud_manager_get_colour70_00bbcd70(const CloudManagerC& m) noexcept { return m.colour_70; }
void cloud_manager_set_colour70_00bbcd90(CloudManagerC& m, float r, float g, float b, float a) noexcept { m.colour_70 = {r, g, b, a}; }
std::array<float, 4> cloud_manager_get_colour80_00bbcdb0(const CloudManagerC& m) noexcept { return m.colour_80; }
void cloud_manager_set_colour80_00bbcde0(CloudManagerC& m, float r, float g, float b, float a) noexcept { m.colour_80 = {r, g, b, a}; }
void cloud_manager_set_light94_00bbce30(CloudManagerC& m, std::uint32_t light) noexcept { m.light_94 = light; }
void cloud_manager_set_wind_enabled_00bbce60(CloudManagerC& m, std::uint8_t enabled) noexcept { m.wind_enabled_99 = enabled; }

void cloud_manager_set_enabled_00bbcfe0(CloudManagerC& m, CloudManagerHostC& host, std::uint8_t enabled) {
    m.enabled_98 = enabled;
    // 00BBCFE5..00BBD006: the group node gets (flag == 1) ? 1.0 : 0.0 (SETE + CVTSI2SS).
    host.set_visibility_factor_00b6da70(m.group_node_30, enabled == 1 ? 1.0f : 0.0f, 0);
    // 00BBD020..00BBD055: every registered node gets float(MOVZX flag), so a flag of 2
    // gives the registered nodes 2.0 while the group node gets 0.0.
    const float value = static_cast<float>(enabled);
    for (std::uint32_t i = 0; i < m.nodes_count_a0; ++i)
        host.set_visibility_factor_00b6da70(m.nodes_9c_storage[i], value, 0);
}

void cloud_manager_register_node_00bbd470(CloudManagerC& m, std::uint32_t node) {
    if (m.nodes_count_a0 == m.nodes_capacity_a4) {
        m.nodes_capacity_a4 = m.nodes_capacity_a4 * 2 + 2;
        std::vector<std::uint32_t> next(m.nodes_capacity_a4);
        for (std::uint32_t i = 0; i < m.nodes_count_a0; ++i) next[i] = m.nodes_9c_storage[i];
        m.nodes_9c_storage.swap(next);
    }
    m.nodes_9c_storage[m.nodes_count_a0] = node;
    ++m.nodes_count_a0;
}

void cloud_manager_ensure_instance_lighting_00bbd510(const CloudManagerC& m, CloudManagerHostC& host) {
    for (std::int32_t i = 0; i < m.instances_18.count; ++i)
        host.ensure_lighting_00bbe640(m.instances_18.storage[static_cast<std::size_t>(i)]);
}

std::uint32_t cloud_manager_create_cloud_class_00bbd6c0(CloudManagerC& m, CloudManagerHostC& host,
                                                        std::uint32_t atmosphere_source_0c, std::uint32_t mesh,
                                                        std::uint32_t texture, float multiplier) {
    // 00BBD6F9..00BBD717: source = [manager+8]->+0Ch (00BBDDC0).
    const std::uint32_t cls = host.construct_cloud_class_00bc4a60(atmosphere_source_0c, mesh, texture, multiplier);
    w1c_array_push_back_c(m.classes_0c, cls);  // pushed even when 0
    return cls;
}

std::uint32_t cloud_manager_create_instance_00bbd770(CloudManagerC& m, CloudManagerHostC& host, std::uint32_t arg) {
    const std::uint32_t inst = host.factory_create_instance_010909b0(m.group_node_30, arg);
    if (inst != 0) w1c_array_push_back_c(m.instances_18, inst);
    return inst;
}

void cloud_manager_release_instance_00bbd910(CloudManagerC& m, CloudManagerHostC& host, std::uint32_t instance) {
    host.unlink_and_release_00b6dfa0(instance);
    cloud_pointer_vector_remove_swap_last_00bbd880(m.instances_18, instance);
}

void cloud_manager_destruct_body_00bbd950(CloudManagerC& m, CloudManagerHostC& host) {
    for (std::int32_t i = 0; i < m.classes_0c.count; ++i) {
        std::uint32_t& c = m.classes_0c.storage[static_cast<std::size_t>(i)];
        if (c != 0) {
            host.release_ref_interlocked_c(c);
            c = 0;
        }
    }
    m.nodes_9c_storage.clear();  // _free(+9C) when non-null
    w1c_array_resize_c(m.array_24, 0);
    m.array_24.storage.clear();
    w1c_array_resize_c(m.instances_18, 0);  // instances are not released here
    m.instances_18.storage.clear();
    w1c_array_resize_c(m.classes_0c, 0);
    m.classes_0c.storage.clear();
    // then BSP_RefCounted_DestroyBase
}

// ---------------------------------------------------------------------------
// Atmosphere

float atmosphere_vector3_length_c(float x, float y, float z) noexcept {
    const float xx = static_cast<float>(static_cast<double>(x) * x);
    const float yy = static_cast<float>(static_cast<double>(y) * y);
    const double sum_xy = static_cast<double>(xx) + yy;  // stays on the x87 stack
    const float zz = static_cast<float>(static_cast<double>(z) * z);
    const float sum = static_cast<float>(sum_xy + zz);
    return static_cast<float>(std::sqrt(static_cast<double>(sum)));
}

AtmosphereC atmosphere_construct_00bbdff0(AtmosphereHostC& host, std::uint32_t source, std::uint32_t sky_arg) {
    AtmosphereC a;
    a.field_1c = 40000.0f;   // 00D64520
    a.time_of_day_20 = 0.5f; // 00CE3800
    a.field_24 = 1.0f;       // 00D7A24C
    a.blend_28 = 0.1f;       // 00D7A2F0
    // 00BBE046..00BBE0ED: v = (1.0, 0.0, 0.4f [00CE7804]); FCOMI len, 0 with JBE: a length
    // <= 0 or NaN gives inv = 0. x = inv, y = float(0.0 [00D7A258 qword] * inv),
    // z = float(inv * 0.4000000059604645 [00CE65D0 qword]).
    const float len = atmosphere_vector3_length_c(1.0f, 0.0f, 0.4f);
    const float inv = (len > 0.0f) ? static_cast<float>(1.0 / static_cast<double>(len)) : 0.0f;
    a.wind_direction_2c = {inv, static_cast<float>(0.0 * static_cast<double>(inv)),
                           static_cast<float>(static_cast<double>(inv) * 0.4000000059604645)};
    a.wind_speed_38 = 0.6f;  // 00CE3D30
    if (a.source_0c != source) {
        const std::uint32_t old = a.source_0c;
        a.source_0c = source;
        if (source != 0) host.add_ref_interlocked_c(source);
        if (old != 0) host.release_ref_interlocked_c(old);
    }
    a.sky_box_18 = host.construct_sky_box_00bbf8a0(source, sky_arg);
    a.cloud_manager_3c = host.construct_cloud_manager_00bbda50();
    return a;
}

void atmosphere_update_clouds_00bbddd0(const AtmosphereC& a, AtmosphereHostC& host, std::uint32_t arg, float dt) {
    if (a.cloud_manager_3c != 0) host.update_frame_and_instances_00bbd310(a.cloud_manager_3c, arg, dt);
}

void atmosphere_replace_sky_box_00bbdf60(AtmosphereC& a, AtmosphereHostC& host, std::uint32_t sky_arg) {
    if (a.sky_box_18 != 0) {
        host.release_ref_interlocked_c(a.sky_box_18);
        a.sky_box_18 = 0;
    }
    a.sky_box_18 = host.construct_sky_box_00bbf8a0(a.source_0c, sky_arg);
}

void atmosphere_destruct_body_00bbe1a0(AtmosphereC& a, AtmosphereHostC& host) {
    // Order: +0Ch source, +18h sky box, +10h fog receiver, +3Ch cloud manager.
    std::uint32_t* const order[] = {&a.source_0c, &a.sky_box_18, &a.fog_receiver_10, &a.cloud_manager_3c};
    for (std::uint32_t* p : order) {
        if (*p != 0) {
            host.release_ref_interlocked_c(*p);
            *p = 0;
        }
    }
    host.weak_owner_destroy_c();
}

// ---------------------------------------------------------------------------
// Cloud instance

void cloud_instance_construct_00bbe7d0(CloudInstanceC& c) noexcept {
    c.scroll_1b8 = {0.0f, 0.0f, 0.0f};
    c.field_1c4 = 1.0f;  // 00D7A24C
    c.affected_by_wind_1e8 = 1;
}

std::uint32_t cloud_instance_type_token_00bbe2b0(const CloudInstanceTypeInfoC& t) noexcept { return t.token_00; }
const char* cloud_instance_type_name_00bbe2c0(const CloudInstanceTypeInfoC& t) noexcept { return t.name_10; }

bool cloud_instance_accepts_type_token_00bbe2e0(const CloudInstanceTypeInfoC& t, std::uint32_t token) noexcept {
    if (t.token_00 == token) return true;
    for (std::uint32_t b : t.base_04)
        if (b == token) return true;
    return false;
}

void cloud_instance_init_type_info_00bbe770(CloudInstanceTypeInfoC& t, bool& initialized_0109090c,
                                            const std::array<std::uint32_t, 3>& base_01090034,
                                            std::uint32_t& type_counter, CloudInstanceHostC& host) {
    if (initialized_0109090c) return;
    initialized_0109090c = true;
    t.name_10 = "cCloudInstance";
    host.initialize_3d_object_type_info_c();
    t.base_04 = base_01090034;  // read after the base initializer ran
    t.token_00 = type_counter++;
}

bool cloud_instance_show_return_false_00bbe310(std::uint32_t self_node, CloudInstanceHostC& host) {
    host.set_visibility_factor_00b6da70(self_node, 1.0f, 0);  // FLD1
    return false;
}

std::uint32_t cloud_instance_get_class_00bbe330(const CloudInstanceC& c) noexcept { return c.cloud_class_184; }
std::array<float, 3> cloud_instance_get_manager_vector50_00bbe340(const CloudManagerC& m) noexcept { return m.vector_50; }
std::array<float, 3> cloud_instance_get_manager_vector44_00bbe360(const CloudManagerC& m) noexcept { return m.vector_44; }

void cloud_instance_upload_puffs_00bbe380(const CloudInstanceC& c, CloudInstanceHostC& host) {
    const std::uint32_t mapped = host.lock_vertices_vslot10(c.vertex_buffer_1a8);
    host.upload_puffs_00bc33b0(c.cloud_class_184, mapped);
    host.unlock_vertices_vslot14(c.vertex_buffer_1a8);
}

void cloud_instance_set_scale_00bbe470(CloudInstanceC& c, float v) noexcept { c.scale_1c8 = v; }
void cloud_instance_set_alpha_00bbe490(CloudInstanceC& c, float v) noexcept { c.alpha_1d8 = v; }
void cloud_instance_set_ambient_colour_00bbe4b0(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept { s.ambient_01090970 = {r, g, b, a}; }
void cloud_instance_set_diffuse_colour_00bbe4e0(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept { s.diffuse_01090980 = {r, g, b, a}; }
void cloud_instance_set_specular_colour_00bbe510(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept { s.specular_01090990 = {r, g, b, a}; }
void cloud_instance_set_inside_colour_00bbe540(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept { s.inside_010909a0 = {r, g, b, a}; }
void cloud_instance_set_density_00bbe570(CloudInstanceStaticsC& s, float v) noexcept { s.density_01090908 = v; }
void cloud_instance_set_blur_radius_00bbe590(CloudInstanceStaticsC& s, float v) noexcept { s.blur_radius_01090904 = v; }
void cloud_instance_set_affected_by_wind_00bbe5c0(CloudInstanceC& c, std::uint8_t v) noexcept { c.affected_by_wind_1e8 = v; }

void cloud_instance_ensure_lighting_00bbe640(CloudInstanceC& c, const CloudInstanceStaticsC& s,
                                             std::uint32_t manager_light_94, CloudInstanceHostC& host) {
    if (c.lit_1b4 != 0) return;
    // Light 0 (00BBE653..00BBE6B2): the manager's +94h object (00BC3030, 00BBCE40), its
    // +1E0h direction normalized and taken into instance space.
    const std::array<float, 3> d0 = host.to_local_direction_0042d0d0(host.light_direction_1e0(manager_light_94));
    host.compute_lighting_00bc3930(c.cloud_class_184, 0, d0, s.density_01090908, s.blur_radius_01090904,
                                   &c.lighting_out_1ac);
    // Light 1 (00BBE6B7..00BBE71C): straight down, (0, -1.0 [00D7A260], 0).
    const std::array<float, 3> d1 = host.to_local_direction_0042d0d0({0.0f, -1.0f, 0.0f});
    host.compute_lighting_00bc3930(c.cloud_class_184, 1, d1, s.density_01090908, s.blur_radius_01090904,
                                   &c.lighting_out_1b0);
    cloud_instance_upload_puffs_00bbe380(c, host);
    c.lit_1b4 = 1;
}

void cloud_instance_invalidate_lighting_00bbe760(CloudInstanceC& c, const CloudInstanceStaticsC& s,
                                                 std::uint32_t manager_light_94, CloudInstanceHostC& host) {
    c.lit_1b4 = 0;
    cloud_instance_ensure_lighting_00bbe640(c, s, manager_light_94, host);
}

// ---------------------------------------------------------------------------
// Sky box

float sky_box_rotation_radians_00bbf1a0(float degrees) noexcept {
    // 00BBF1A6..00BBF1C4: FMUL qword 00D64610 (3.141592025756836, not the double pi),
    // FDIV qword 00CE3D20 (180.0), FSTP float.
    return static_cast<float>(static_cast<double>(degrees) * 3.141592025756836 / 180.0);
}

void sky_box_set_rotation_degrees_00bbf1a0(SkyBoxHostC& host, float degrees) {
    host.set_node_rotation_y_00b646e0(sky_box_rotation_radians_00bbf1a0(degrees));
}

void sky_box_set_luminance_00bbf200(SkyBoxC& s, float v) noexcept { s.luminance_30 = v; }

// ---------------------------------------------------------------------------
// Lightning storm

void lightning_storm_destroy_00bc04c0(LightningStormHandlesC& s, std::uint32_t& active_storm_010909b4,
                                      LightningStormHostC& host) {
    active_storm_010909b4 = 0;
    for (std::size_t i = 0; i < 7; ++i) {
        if (s.handles_08[i] != 0) {
            host.release_ref_interlocked_c(s.handles_08[i]);
            s.handles_08[i] = 0;
        }
        s.slots_24[i] = 0;
    }
}

// ---------------------------------------------------------------------------
// Waterdrops

void waterdrops_construct_00bc2030(WaterdropsC& w) noexcept {
    w.drops_184 = W1C_NativeArray<WaterdropsDropC>{};
    w.min_size_194 = 0.01f;   // 00D7A238
    w.max_size_198 = 0.025f;  // 00D1FBBC
    w.min_life_19c = 0.5f;    // 00CE3800
    w.max_life_1a0 = 4.0f;    // 00CE3D34
    w.field_1c8 = 0;
    w.start_top_off_screen_1d5 = 0;
    w.apply_at_top_only_1d6 = 0;
    w.no_camera_move_1d7 = 0;
    w.gravity_1c4 = 0.95f;    // 00CED9E0
    w.apply_at_bottom_only_1d4 = 1;
}

bool waterdrops_is_finished_00bc1150(const WaterdropsC& w) noexcept { return w.live_1a8 == 0; }
void waterdrops_set_size_range_00bc1160(WaterdropsC& w, float lo, float hi) noexcept { w.min_size_194 = lo; w.max_size_198 = hi; }
void waterdrops_set_life_range_00bc1180(WaterdropsC& w, float lo, float hi) noexcept { w.min_life_19c = lo; w.max_life_1a0 = hi; }
void waterdrops_set_count_00bc11a0(WaterdropsC& w, std::int32_t n) noexcept { w.requested_190 = n; }
void waterdrops_set_gravity_00bc11b0(WaterdropsC& w, float v) noexcept { w.gravity_1c4 = v; }
void waterdrops_set_sticky_percent_00bc11d0(WaterdropsC& w, float v) noexcept { w.sticky_percent_1a4 = v; }
void waterdrops_set_apply_at_bottom_only_00bc11f0(WaterdropsC& w, std::uint8_t v) noexcept { w.apply_at_bottom_only_1d4 = v; }
void waterdrops_set_start_top_off_screen_00bc1200(WaterdropsC& w, std::uint8_t v) noexcept { w.start_top_off_screen_1d5 = v; }
void waterdrops_set_apply_at_top_only_00bc1210(WaterdropsC& w, std::uint8_t v) noexcept { w.apply_at_top_only_1d6 = v; }
void waterdrops_set_no_camera_move_00bc1220(WaterdropsC& w, std::uint8_t v) noexcept { w.no_camera_move_1d7 = v; }

bool waterdrops_accept_render_00bc1230(const WaterdropsC& w, WaterdropsHostC& host) {
    if (w.live_1a8 == 0) return false;
    host.render_vslot60();
    return true;
}

void waterdrop_vector_reserve_00bc1860(W1C_NativeArray<WaterdropsDropC>& a, std::int32_t n) { w1c_array_reserve_c(a, n); }

WaterdropsFillCountsC waterdrops_fill_vertices_00bc2340(const WaterdropsC& w, WaterdropsHostC& host,
                                                        std::vector<WaterdropsVertexC>& out) {
    WaterdropsFillCountsC counts;
    counts.locked_vertices = w.live_1a8 * 4;
    counts.element_10 = w.live_1a8 * 4;
    counts.element_18 = w.live_1a8 * 2;
    constexpr float kZ = 0.5f;  // 00CE3800
    for (std::int32_t i = 0; i < w.drops_184.count; ++i) {
        const WaterdropsDropC& d = w.drops_184.storage[static_cast<std::size_t>(i)];
        // 00BC23D0..00BC23DC: UCOMISS + LAHF/TEST AH,44h/JNP skips only an ordered 0.0.
        if (d.life_left_20 == 0.0f) continue;
        // 00BC23E2..00BC2421: n = normalize(-orient) via 00419260 (reciprocal length).
        const float ox = -d.orient_x_24;
        const float oy = -d.orient_y_28;
        const float rl = host.reciprocal_length_00419260(ox, oy);
        const float nx = rl * ox;
        const float ny = rl * oy;
        const float s = d.size_08;
        const float h = d.height_0c;
        // Each product and partial sum is FSTP'd to a float slot; double rounding of a
        // float sum or product through 53-bit is innocuous, so float arithmetic matches.
        const float hnx = h * nx, hny = h * ny;
        const float sny = s * ny, snx = s * nx;
        WaterdropsVertexC v[4];
        v[0].x = d.x_00 + (sny + hnx);   v[0].y = (-snx + hny) + d.y_04;
        v[1].x = d.x_00 + (-sny + hnx);  v[1].y = (snx + hny) + d.y_04;
        v[2].x = d.x_00 + (sny - hnx);   v[2].y = (-snx - hny) + d.y_04;
        v[3].x = d.x_00 + (-sny - hnx);  v[3].y = (snx - hny) + d.y_04;
        // 00BC26E9..00BC2739: u0 = float(variant * step) (FILD signed), u1 = float(u0 + step).
        const float step = w.variant_step_1d0;
        const float u0 = static_cast<float>(static_cast<double>(d.variant_30) * step);
        const float u1 = static_cast<float>(static_cast<double>(u0) + step);
        // 00BC2735..00BC277A: alpha byte = low byte of trunc(life_left / life_total * 204.0
        // [00D64838 qword]) with the control word forced to chop; RGB bytes 0xFF.
        const double ratio = static_cast<double>(d.life_left_20) / static_cast<double>(d.life_total_1c);
        const double scaled = ratio * 204.0;
        std::uint8_t alpha = 0;
        if (scaled > -2147483649.0 && scaled < 2147483648.0)
            alpha = static_cast<std::uint8_t>(static_cast<std::uint32_t>(static_cast<std::int32_t>(scaled)) & 0xFFu);
        // else FISTP stores the integer indefinite 80000000h, whose low byte is 0.
        const std::uint32_t colour = (static_cast<std::uint32_t>(alpha) << 24) | 0x00FFFFFFu;
        const float us[4] = {u0, u1, u0, u1};
        const float vs[4] = {0.0f, 0.0f, 1.0f, 1.0f};  // 1.0 = 00D7A24C
        for (int k = 0; k < 4; ++k) {
            v[k].z = kZ;
            v[k].colour = colour;
            v[k].u = us[k];
            v[k].v = vs[k];
            out.push_back(v[k]);
        }
    }
    return counts;
}

// ---------------------------------------------------------------------------
// Waterdrops pool

void waterdrops_pool_init_block_00bc2190(WaterdropsPoolBlockC& b, std::uint32_t block_index) noexcept {
    b.free_count_3bc0 = 0x20;
    for (std::uint32_t i = 0; i < 0x20; ++i) {
        b.free_stack_3b80[i] = static_cast<std::uint16_t>(0x1F - i);  // slot 0 pops first
        b.owner_index_1d8[i] = block_index;
    }
}

void waterdrops_pool_construct_00bc2a30(WaterdropsPoolC& p, WaterdropsPoolC*& list_head_00e188b4) {
    p.list_prev_04 = nullptr;
    p.list_next_08 = list_head_00e188b4;
    if (list_head_00e188b4 != nullptr) list_head_00e188b4->list_prev_04 = &p;
    list_head_00e188b4 = &p;
    // InitializeCriticalSection(+0Ch)
    p.lock_depth_24 = 0;
    p.blocks_28.clear();
    p.block_capacity_30 = 0;
    p.current_34 = 0xFFFFFFFFu;
    if (p.block_capacity_30 < 0x20) p.block_capacity_30 = 0x20;  // operator_new(80h)
}

WaterdropsPoolSlotC waterdrops_pool_allocate_00bc2bb0(WaterdropsPoolC& p) {
    // EnterCriticalSection(+0Ch)
    ++p.lock_depth_24;
    if (p.current_34 == 0xFFFFFFFFu) {
        p.current_34 = static_cast<std::uint32_t>(p.blocks_28.size());
        WaterdropsPoolBlockC b;  // operator_new(3BC4h); a failed allocation is not modelled
        waterdrops_pool_init_block_00bc2190(b, p.current_34);
        if (p.blocks_28.size() == p.block_capacity_30) p.block_capacity_30 = p.block_capacity_30 * 2 + 2;
        p.blocks_28.push_back(b);
    }
    WaterdropsPoolBlockC& b = p.blocks_28[p.current_34];
    --b.free_count_3bc0;
    const WaterdropsPoolSlotC result{p.current_34, b.free_stack_3b80[b.free_count_3bc0]};
    if (b.free_count_3bc0 == 0) {
        std::uint32_t next = p.current_34 + 1;
        p.current_34 = 0xFFFFFFFFu;
        for (; next < p.blocks_28.size(); ++next) {
            if (p.blocks_28[next].free_count_3bc0 != 0) {
                p.current_34 = next;
                break;
            }
        }
    }
    --p.lock_depth_24;
    // LeaveCriticalSection(+0Ch)
    return result;
}

void waterdrops_pool_free_00bc2880(WaterdropsPoolC& p, WaterdropsPoolSlotC s) {
    ++p.lock_depth_24;
    WaterdropsPoolBlockC& b = p.blocks_28[s.block];
    b.free_stack_3b80[b.free_count_3bc0] = static_cast<std::uint16_t>(s.slot);
    ++b.free_count_3bc0;
    if (s.block < p.current_34) p.current_34 = s.block;  // unsigned: FFFFFFFFh is "none"
    --p.lock_depth_24;
}

void waterdrops_pool_free_global_00bc29f0(WaterdropsPoolC& pool_010909bc, WaterdropsPoolSlotC s) {
    waterdrops_pool_free_00bc2880(pool_010909bc, s);
}

void waterdrops_pool_trim_free_blocks_00bc2b10(WaterdropsPoolC& p) {
    // Wholly free blocks are freed and replaced by the last block, whose slot trailers are
    // renumbered; the index is re-examined (the native decrements an unsigned index, which
    // wraps at 0 and comes back with the increment).
    std::uint32_t i = 0;
    while (i < p.blocks_28.size()) {
        if (p.blocks_28[i].free_count_3bc0 == 0x20) {
            p.blocks_28[i] = p.blocks_28.back();
            p.blocks_28.pop_back();
            if (i < p.blocks_28.size())
                for (std::uint32_t& o : p.blocks_28[i].owner_index_1d8) o = i;
            continue;
        }
        ++i;
    }
    p.current_34 = 0xFFFFFFFFu;
    for (std::uint32_t k = 0; k < p.blocks_28.size(); ++k) {
        if (p.blocks_28[k].free_count_3bc0 != 0) {
            p.current_34 = k;
            break;
        }
    }
}

std::int32_t waterdrops_pool_destroy_00bc27c0(WaterdropsPoolC& p, WaterdropsPoolC*& list_head_00e188b4) {
    p.blocks_28.clear();  // each block, then the pointer array
    std::int32_t leaves = 0;
    while (p.lock_depth_24 > 0) {  // LeaveCriticalSection once per outstanding depth
        --p.lock_depth_24;
        ++leaves;
    }
    // DeleteCriticalSection, then unlink from the allocator list.
    if (p.list_prev_04 == nullptr)
        list_head_00e188b4 = p.list_next_08;
    else
        p.list_prev_04->list_next_08 = p.list_next_08;
    if (p.list_next_08 != nullptr) p.list_next_08->list_prev_04 = p.list_prev_04;
    return leaves;
}

WaterdropsPoolSlotC waterdrops_create_00bc2d00(WaterdropsPoolC& pool_010909bc, WaterdropsC& object) {
    const WaterdropsPoolSlotC s = waterdrops_pool_allocate_00bc2bb0(pool_010909bc);
    waterdrops_construct_00bc2030(object);  // then vtable 00D647D0
    return s;
}

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part d: 00BC3000-00BD2E60
// ================================================================================================
// cc13-water1 helper part "d" (00BC3000-00BD2E60). Names are hypotheses, not recovered symbols.
// x87 model: the image runs at 53-bit precision, so intermediates are double and every FSTP to a
// float slot is a float cast. Random draws: only the lightning strand model (00BC84A0) and the
// unreferenced 00BC7C10 draw from 00BD2F10, both on stream 0; the models take the values from a
// host callback or as parameters.
namespace bsp::water_weather_cc13 {

namespace {

inline float f32_w1d(double v) noexcept { return static_cast<float>(v); }

float float_from_bits_w1d(std::uint32_t bits) noexcept {
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}

// The spill pattern shared by 00BC3580, 00BC84A0 (three sites) and 00419440: x*x and y*y are each
// stored to a float slot and reloaded, their sum stays in the register, z*z is stored to a float
// slot, and the total is stored to a float slot before the square root.
float sum_sq_spilled_w1d(float x, float y, float z) noexcept {
    const double xy = static_cast<double>(f32_w1d(static_cast<double>(x) * x)) +
                      static_cast<double>(f32_w1d(static_cast<double>(y) * y));
    return f32_w1d(xy + static_cast<double>(f32_w1d(static_cast<double>(z) * z)));
}

// LIBCRT_sqrt 00BF7030, result stored to a float slot; FCOMI against 0 then JBE: a length that is
// not > 0 (including NaN) gives 0, otherwise FLD1/FDIVRP and a float store.
float inv_length_or_zero_w1d(float x, float y, float z) noexcept {
    const float length = f32_w1d(std::sqrt(static_cast<double>(sum_sq_spilled_w1d(x, y, z))));
    if (length > 0.0f) return f32_w1d(1.0 / static_cast<double>(length));
    return 0.0f;
}

// FISTP QWORD under a chop control word, low dword kept. Out of range and NaN store the integer
// indefinite 8000000000000000h, whose low dword is 0.
std::uint32_t chop_i64_low32_w1d(double v) noexcept {
    if (!(v >= -9223372036854775808.0 && v < 9223372036854775808.0)) return 0u;
    return static_cast<std::uint32_t>(static_cast<std::uint64_t>(static_cast<std::int64_t>(v)));
}

// FISTP DWORD under a chop control word, or CVTTSD2SI: truncation, 80000000h when out of range.
std::int32_t chop_i32_w1d(double v) noexcept {
    if (!(v > -2147483649.0 && v < 2147483648.0)) return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(v);
}

// _stricmp 00BF7FBF for the ASCII node names compared here.
bool ascii_iequal_w1d(const char* a, const char* b) noexcept {
    for (;; ++a, ++b) {
        unsigned char ca = static_cast<unsigned char>(*a);
        unsigned char cb = static_cast<unsigned char>(*b);
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<unsigned char>(ca + 32);
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<unsigned char>(cb + 32);
        if (ca != cb) return false;
        if (ca == 0) return true;
    }
}

// 004F9B30 BSP_Vector3f_Cross(ECX = out, EDX = a, [esp+4] = b): out = a x b, each component's two
// products in the register and the difference stored to a float slot.
W1D_Vec3 cross_w1d(const W1D_Vec3& a, const W1D_Vec3& b) noexcept {
    W1D_Vec3 out;
    out.x = f32_w1d(static_cast<double>(a.y) * b.z - static_cast<double>(a.z) * b.y);
    out.y = f32_w1d(static_cast<double>(a.z) * b.x - static_cast<double>(a.x) * b.z);
    out.z = f32_w1d(static_cast<double>(a.x) * b.y - static_cast<double>(a.y) * b.x);
    return out;
}

// Remove-expired pass shared by 00BC84A0 (strands +194h with lightning segments +188h) and
// 00BC9160 (thunder timers +1ACh with thunder segments +1A0h). Indices of records whose timer is
// <= 0 (COMISS 0, t / JB: NaN is kept) are collected in ascending order, then each index is
// removed from BOTH vectors by copying the current last record over it and decrementing the
// count. Because the indices are not adjusted, a record moved into an earlier slot is never
// re-examined and an index at or past the shrunken count copies into dead storage: e.g. expired
// {1, 3} of 4 keeps old record 3 (expired) and drops old record 2. The -1 sentinel tests of the
// native never fire. The native decrements a count of 0 to FFFFFFFFh; the model stops at 0
// (unreachable while the two vectors stay parallel).
template <typename Record>
void swap_remove_at_w1d(std::vector<Record>& records, std::uint32_t index) noexcept {
    const std::size_t count = records.size();
    if (count == 0) return;
    if (index != count - 1 && index < count) records[index] = records[count - 1];
    records.pop_back();
}

void remove_expired_pairs_w1d(std::vector<LightningStrand>& timers, std::vector<LightningSegment>& segments) {
    std::vector<std::uint32_t> expired;
    for (std::uint32_t i = 0; i < timers.size(); ++i) {
        if (timers[i].timer_00 <= 0.0f) expired.push_back(i);
    }
    for (const std::uint32_t index : expired) {
        swap_remove_at_w1d(timers, index);
        swap_remove_at_w1d(segments, index);
    }
}

constexpr std::uint32_t kLightningMaxSegmentsPlusOne_w1d = 0x32;  // CMP EAX,32h / JGE on count + 1

}  // namespace

// ---------------------------------------------------------------------------------------------
// Cloud puff
// ---------------------------------------------------------------------------------------------

CloudPuffState cloud_puff_construct_00bc6430(std::uint32_t slot, const W1D_Vec3& position, float angle,
    float size) noexcept {
    CloudPuffState puff;
    puff.slot_08 = slot;
    puff.position_0c = position;
    puff.angle_18 = angle;
    puff.size_1c = size;
    return puff;
}

W1D_Vec3 cloud_puff_get_position_00bc7a70(const CloudPuffState& puff) noexcept { return puff.position_0c; }

void cloud_puff_set_sort_key_00bc7a90(CloudPuffState& puff, float key) noexcept { puff.sort_key_34 = key; }

void cloud_puff_set_shade_00bc7ad0(CloudPuffState& puff, int light, float value) noexcept {
    if (light >= 0 && light < 2) puff.shade_28[static_cast<std::size_t>(light)] = value;
}

void cloud_puff_set_light_00bc7af0(CloudPuffState& puff, int light, float value) noexcept {
    if (light >= 0 && light < 2) puff.light_20[static_cast<std::size_t>(light)] = value;
}

// 00BC7B10: FLD alpha, FMUL QWORD 00CE4B48 (255.0), FISTP DWORD under OR 0C00h, low byte stored.
void cloud_puff_set_alpha_00bc7b10(CloudPuffState& puff, float alpha) noexcept {
    const std::int32_t v = chop_i32_w1d(static_cast<double>(alpha) * 255.0);
    puff.alpha_30 = static_cast<std::uint8_t>(static_cast<std::uint32_t>(v) & 0xFFu);
}

float cloud_puff_get_light_00bc7b40(const CloudPuffState& puff, int light) noexcept {
    return (light >= 0 && light < 2) ? puff.light_20[static_cast<std::size_t>(light)] : 0.0f;
}

float cloud_puff_get_shade_00bc7b50(const CloudPuffState& puff, int light) noexcept {
    return (light >= 0 && light < 2) ? puff.shade_28[static_cast<std::size_t>(light)] : 0.0f;
}

// 00BC64C0. Per corner: size is stored to a float slot, sx = size*cx and sy = size*cy are stored
// to float slots, cos/sin of the angle (FCOS/FSIN on the float angle) are each stored to a float
// slot, x = c*sx - s*sy and y = s*sx + c*sy are evaluated in the register and stored as floats.
// Colour: EBX = alpha << 8; EBX -= chop64(shade0 * -255.0) (double 00D64950); EBX <<= 8;
// EBX -= chop64(-255.0 * shade1); EBX <<= 8 (32-bit wrap). FSIN/FCOS of the x87 and std::sin/cos
// can differ in the last extended bit; after the float store they agree for ordinary angles.
void cloud_puff_write_quad_vertices_00bc64c0(const CloudPuffState& puff,
    std::array<CloudPuffVertex, 4>& out) noexcept {
    static constexpr float kCornerX[4] = {-1.0f, 1.0f, 1.0f, -1.0f};
    static constexpr float kCornerY[4] = {1.0f, 1.0f, -1.0f, -1.0f};
    static constexpr float kU[4] = {0.0f, 1.0f, 1.0f, 0.0f};
    static constexpr float kV[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    for (std::size_t i = 0; i < 4; ++i) {
        const double size = puff.size_1c;
        const float sx = f32_w1d(size * kCornerX[i]);
        const float sy = f32_w1d(size * kCornerY[i]);
        const float c = f32_w1d(std::cos(static_cast<double>(puff.angle_18)));
        const float s = f32_w1d(std::sin(static_cast<double>(puff.angle_18)));
        CloudPuffVertex& v = out[i];
        v.offset_x_10 = f32_w1d(static_cast<double>(c) * sx - static_cast<double>(s) * sy);
        v.offset_y_14 = f32_w1d(static_cast<double>(s) * sx + static_cast<double>(c) * sy);
        v.position_00 = puff.position_0c;
        std::uint32_t colour = static_cast<std::uint32_t>(puff.alpha_30) << 8;
        colour -= chop_i64_low32_w1d(static_cast<double>(puff.shade_28[0]) * -255.0);
        colour <<= 8;
        colour -= chop_i64_low32_w1d(-255.0 * static_cast<double>(puff.shade_28[1]));
        colour <<= 8;
        v.colour_0c = colour;
        v.u_18 = kU[i];
        v.v_1c = kV[i];
    }
}

bool cloud_class_write_puff_quads_00bc33b0(const std::vector<CloudPuffState>& puffs,
    std::vector<CloudPuffVertex>& vertices) noexcept {
    for (const CloudPuffState& puff : puffs) {
        if ((static_cast<std::size_t>(puff.slot_08) + 1) * 4 > vertices.size()) return false;
    }
    for (const CloudPuffState& puff : puffs) {
        std::array<CloudPuffVertex, 4> quad;
        cloud_puff_write_quad_vertices_00bc64c0(puff, quad);
        for (std::size_t k = 0; k < 4; ++k) vertices[static_cast<std::size_t>(puff.slot_08) * 4 + k] = quad[k];
    }
    return true;
}

std::uint32_t cloud_class_puff_slot_00bc3410(const std::vector<CloudPuffState>& puffs,
    std::uint32_t index) noexcept {
    return index < puffs.size() ? puffs[index].slot_08 : 0u;
}

// 00BC3580. d: differences stored to float slots, spill pattern, sqrt stored to a float slot.
// Containment: FLD QWORD 00CEC160 (1.2000000476837158 = (double)1.2f) times the radius; FCOMIP
// then JB skips when 1.2 r < d or unordered. f = 1 - (1.2 r - d) / (r * 0.20000000298023224)
// (QWORD 00CE3D10) in the register, stored as float. Clamp: FLDZ/FCOMI/JBE keeps f when 0 <= f
// or unordered, else 0; COMISS against 1.0f (00D7A24C) then JBE keeps f unless f > 1. Minimum:
// FCOMIP/JA replaces unless f > min (NaN replaces). A second containing sphere returns +0.
float cloud_class_puff_edge_factor_00bc3580(const std::vector<CloudEdgeSphere>& spheres,
    const W1D_Vec3& point) noexcept {
    float minimum = 0.0f;
    std::uint32_t contained = 0;
    for (const CloudEdgeSphere& sphere : spheres) {
        const float dx = f32_w1d(static_cast<double>(sphere.center.x) - point.x);
        const float dy = f32_w1d(static_cast<double>(sphere.center.y) - point.y);
        const float dz = f32_w1d(static_cast<double>(sphere.center.z) - point.z);
        const float d = f32_w1d(std::sqrt(static_cast<double>(sum_sq_spilled_w1d(dx, dy, dz))));
        const double r = sphere.radius;
        const double reach = r * 1.2000000476837158;
        if (!(reach >= static_cast<double>(d))) continue;
        ++contained;
        float f = f32_w1d(1.0 - (reach - static_cast<double>(d)) / (r * 0.20000000298023224));
        if (f < 0.0f) f = 0.0f;
        if (f > 1.0f) f = 1.0f;
        if (!(f > minimum)) minimum = f;
        if (contained > 1) return 0.0f;
    }
    return minimum;
}

// ---------------------------------------------------------------------------------------------
// Cloud resource readers
// ---------------------------------------------------------------------------------------------

void cloud_class_read_resource_00bc43d0(CloudResourceReaderHost& host) {
    while (host.has_remaining_00715bf0()) {
        const char* name = host.open_child_00bea680();
        if (name != nullptr && ascii_iequal_w1d(name, "CloudSystem")) {
            host.read_cloud_system_00bc42e0();
        } else {
            host.skip_child_00be9c40();
        }
        host.release_child_00be9ed0();
    }
}

void cloud_class_read_resource_file_00bc4480(CloudResourceReaderHost& host) {
    host.read_control_word_00be9a40();
    while (host.has_remaining_00715bf0()) {
        const char* name = host.open_child_00bea680();
        if (name != nullptr && ascii_iequal_w1d(name, "Resource")) {
            host.enter_resource_child();
        } else {
            host.skip_child_00be9c40();
        }
        host.release_child_00be9ed0();
    }
}

void cloud_class_load_resource_00bc4530(CloudResourceLoadHost& host, const std::string& path) {
    void* stream = host.open_stream_vfs_vt4(path, 2);
    host.attach_reader_00bea4b0(stream);
    host.release_stream(stream);
    host.read_root_with_00bc4480();
}

// ---------------------------------------------------------------------------------------------
// Cloud render model
// ---------------------------------------------------------------------------------------------

// 00BC4D20. UCOMISS alpha, 0.0f / LAHF / TEST AH,44h / JNP exits only on equality. The distance
// is |camera - model| with the differences stored to float slots; FCOMIP 100.0 / JA exits when
// 100 > length. The dot is (y*y' + x*x') + z*z' in the register, stored as float, compared with
// FCOMIP / JA against 0.89999997615814209. Index words are the low 16 bits of slot * 4 + k.
bool cloud_render_model_update_00bc4d20(CloudRenderModelState& model, const W1D_Vec3& camera_world,
    CloudRenderModelUpdateHost& host, std::vector<std::uint16_t>& indices) {
    if (model.alpha_0ac == 0.0f) return false;
    W1D_Vec3 diff;
    diff.x = f32_w1d(static_cast<double>(camera_world.x) - model.world_position_120.x);
    diff.y = f32_w1d(static_cast<double>(camera_world.y) - model.world_position_120.y);
    diff.z = f32_w1d(static_cast<double>(camera_world.z) - model.world_position_120.z);
    const double length = host.vector_length_00419440(diff);
    if (100.0 > length) return false;
    const W1D_Vec3 dir = host.camera_dir_to_model_0042d0d0();
    const W1D_Vec3& last = model.last_view_dir_198;
    const float dot = f32_w1d((static_cast<double>(last.y) * dir.y + static_cast<double>(last.x) * dir.x) +
                              static_cast<double>(last.z) * dir.z);
    if (static_cast<double>(dot) > 0.89999997615814209) return false;
    if (host.classify_bounds_00b71530() == 2) return false;
    model.last_view_dir_198 = dir;
    model.last_eye_18c = camera_world;
    const W1D_Vec3 local_eye = host.camera_pos_to_model_004142e0(camera_world);
    host.sort_puffs_00bc3430(local_eye);
    indices.clear();
    for (std::uint32_t i = 0; i < host.puff_count_00bc3100(); ++i) {
        const std::uint32_t base = host.sorted_puff_slot_00bc3410(i) * 4u;
        indices.push_back(static_cast<std::uint16_t>(base));
        indices.push_back(static_cast<std::uint16_t>(base + 1));
        indices.push_back(static_cast<std::uint16_t>(base + 2));
        indices.push_back(static_cast<std::uint16_t>(base + 2));
        indices.push_back(static_cast<std::uint16_t>(base + 3));
        indices.push_back(static_cast<std::uint16_t>(base));
    }
    return true;
}

// ---------------------------------------------------------------------------------------------
// Fixed-block model pools
// ---------------------------------------------------------------------------------------------

CloudPoolGeometry cloud_render_model_pool_geometry_00bc53a0() noexcept {
    return CloudPoolGeometry{0x1F0u, 16u, 0x1F24u, 0x1ECu, 0x1F00u, 0x1F20u};
}

CloudPoolGeometry cloud_plane_model_pool_geometry_00bc5f10() noexcept {
    return CloudPoolGeometry{0x190u, 4u, 0x64Cu, 0x18Cu, 0x640u, 0x648u};
}

CloudPoolGeometry lightning_model_pool_geometry_00bc7310() noexcept {
    return CloudPoolGeometry{0x1C0u, 8u, 0xE14u, 0x1BCu, 0xE00u, 0xE10u};
}

void cloud_pool_init_block_00bc4b70(std::uint16_t slots_per_block, std::uint32_t page,
    std::vector<std::uint16_t>& free_stack, std::vector<std::uint32_t>& page_index,
    std::uint16_t& free_count) {
    free_count = slots_per_block;
    free_stack.assign(slots_per_block, 0);
    page_index.assign(slots_per_block, page);
    for (std::uint16_t i = 0; i < slots_per_block; ++i) {
        free_stack[i] = static_cast<std::uint16_t>(slots_per_block - 1 - i);
    }
}

CloudPagedPool::CloudPagedPool(std::uint16_t slots_per_block) noexcept : slots_per_block_(slots_per_block) {}

CloudPagedPool::Block CloudPagedPool::init_block(std::uint32_t page_index) const {
    Block block;
    cloud_pool_init_block_00bc4b70(slots_per_block_, page_index, block.free_stack, block.page_index,
        block.free_count);
    return block;
}

// 00BC53A0: with no free block, the new block's index (the current count) becomes the first-free
// block, the block array grows to 2n+2 when full, the block is appended. The slot index is popped
// from the first-free block; when that block empties, the first-free block is searched upward from
// the next index and becomes FFFFFFFFh when none is found.
CloudPagedPool::Slot CloudPagedPool::allocate() {
    if (first_free_34_ == 0xFFFFFFFFu) {
        first_free_34_ = static_cast<std::uint32_t>(blocks_.size());
        Block block = init_block(first_free_34_);
        if (blocks_.size() == capacity_30_) capacity_30_ = capacity_30_ * 2u + 2u;
        blocks_.push_back(std::move(block));
    }
    Block& block = blocks_[first_free_34_];
    block.free_count = static_cast<std::uint16_t>(block.free_count - 1);
    Slot slot{first_free_34_, block.free_stack[block.free_count]};
    if (block.free_count == 0) {
        std::uint32_t next = first_free_34_ + 1;
        first_free_34_ = 0xFFFFFFFFu;
        for (; next < blocks_.size(); ++next) {
            if (blocks_[next].free_count != 0) {
                first_free_34_ = next;
                break;
            }
        }
    }
    return slot;
}

// 00BC5070: stack[free_count] = index, ++free_count, first-free lowered by an unsigned compare.
// The free stack is directly followed by the count, so a push onto a full block (double free)
// lands on the count itself: the model reproduces that one aliasing step (count = index + 1) and
// ignores deeper overruns, which the native writes past the block.
void CloudPagedPool::free(const Slot& slot) noexcept {
    if (slot.block >= blocks_.size()) return;
    Block& block = blocks_[slot.block];
    if (block.free_count < slots_per_block_) {
        block.free_stack[block.free_count] = slot.index;
        block.free_count = static_cast<std::uint16_t>(block.free_count + 1);
    } else if (block.free_count == slots_per_block_) {
        block.free_count = static_cast<std::uint16_t>(slot.index + 1);
    } else {
        return;
    }
    if (slot.block < first_free_34_) first_free_34_ = slot.block;
}

// 00BC5300: every completely free block is released; the last block moves into its place and all
// of its slots' page indices are rewritten, and the same index is examined again (the native
// decrements an unsigned index, wrapping from 0, before the increment). Then the first-free block
// is recomputed from 0. The block-array capacity is kept.
void CloudPagedPool::trim_free_blocks() noexcept {
    std::uint32_t i = 0;
    if (!blocks_.empty()) {
        do {
            if (blocks_[i].free_count == slots_per_block_) {
                if (i + 1 != blocks_.size()) blocks_[i] = std::move(blocks_.back());
                blocks_.pop_back();
                if (i < blocks_.size()) {
                    for (std::uint32_t& page : blocks_[i].page_index) page = i;
                }
                --i;
            }
            ++i;
        } while (i < blocks_.size());
    }
    first_free_34_ = 0xFFFFFFFFu;
    for (std::uint32_t k = 0; k < blocks_.size(); ++k) {
        if (blocks_[k].free_count != 0) {
            first_free_34_ = k;
            break;
        }
    }
}

std::uint16_t CloudPagedPool::free_count(std::uint32_t block) const noexcept {
    return block < blocks_.size() ? blocks_[block].free_count : 0;
}

std::uint32_t CloudPagedPool::slot_page_index(std::uint32_t block, std::uint16_t index) const noexcept {
    if (block >= blocks_.size() || index >= blocks_[block].page_index.size()) return 0xFFFFFFFFu;
    return blocks_[block].page_index[index];
}

// ---------------------------------------------------------------------------------------------
// Lightning strand model
// ---------------------------------------------------------------------------------------------

// 00BC66B0. a = w0 * -2, b = (1 - w0) * 2, c = (w1 + 1) * -2, d = -w1 * 2, each stored to a float
// slot. Per corner p = coef * half_width and q = coef * half_height are stored as floats; the up
// part (q*0, q, q*0) is formed with FLDZ/FMUL and stored as floats; then per axis
// t = p * side (float), u = t + up (float), position = u + centre (float), centre = dir * 4500.0
// stored as floats. +14h is copied through FLD/FSTP.
void lightning_segment_write_quad_vertices_00bc66b0(const LightningSegment& segment,
    std::array<LightningVertex, 4>& out) noexcept {
    const W1D_Vec3 up{0.0f, 1.0f, 0.0f};
    const W1D_Vec3 side = cross_w1d(up, segment.dir_00);
    const double w0 = segment.anchor_x_28;
    const double w1 = segment.anchor_y_2c;
    const float a = f32_w1d(w0 * -2.0);
    const float b = f32_w1d((1.0 - w0) * 2.0);
    const float c = f32_w1d((w1 + 1.0) * -2.0);
    const float d = f32_w1d(-w1 * 2.0);
    const W1D_Vec3 centre{f32_w1d(static_cast<double>(segment.dir_00.x) * 4500.0),
        f32_w1d(static_cast<double>(segment.dir_00.y) * 4500.0),
        f32_w1d(static_cast<double>(segment.dir_00.z) * 4500.0)};
    const float coef_p[4] = {a, b, b, a};
    const float coef_q[4] = {d, d, c, c};
    const float us[4] = {segment.u0_18, segment.u1_20, segment.u1_20, segment.u0_18};
    const float vs[4] = {segment.v0_1c, segment.v0_1c, segment.v1_24, segment.v1_24};
    for (std::size_t i = 0; i < 4; ++i) {
        const float p = f32_w1d(static_cast<double>(coef_p[i]) * segment.half_width_0c);
        const float q = f32_w1d(static_cast<double>(coef_q[i]) * segment.half_height_10);
        const float q0 = f32_w1d(static_cast<double>(q) * 0.0);
        const float ux = f32_w1d(static_cast<double>(f32_w1d(static_cast<double>(p) * side.x)) + q0);
        const float uy = f32_w1d(static_cast<double>(f32_w1d(static_cast<double>(p) * side.y)) + q);
        const float uz = f32_w1d(static_cast<double>(f32_w1d(static_cast<double>(p) * side.z)) + q0);
        LightningVertex& v = out[i];
        v.position_00.x = f32_w1d(static_cast<double>(ux) + centre.x);
        v.position_00.y = f32_w1d(static_cast<double>(uy) + centre.y);
        v.position_00.z = f32_w1d(static_cast<double>(uz) + centre.z);
        v.alpha_0c = segment.alpha_14;
        v.u_10 = us[i];
        v.v_14 = vs[i];
    }
}

namespace {

void fill_element_w1d(const std::vector<LightningSegment>& segments, LightningElementFill& element) {
    element.vertices.clear();
    element.locked = false;
    element.vertex_count_10 = 0;
    element.primitive_count_18 = 0;
    if (segments.empty()) return;
    element.locked = true;
    element.vertices.resize(segments.size() * 4);
    for (std::size_t i = 0; i < segments.size(); ++i) {
        std::array<LightningVertex, 4> quad;
        lightning_segment_write_quad_vertices_00bc66b0(segments[i], quad);
        for (std::size_t k = 0; k < 4; ++k) element.vertices[i * 4 + k] = quad[k];
    }
    const std::uint32_t n = static_cast<std::uint32_t>(segments.size());
    element.primitive_count_18 = n * 2u;
    element.vertex_count_10 = n * 4u;
}

}  // namespace

void lightning_model_fill_segment_vertices_00bc6fa0(const LightningModelState& model,
    LightningElementFill& element0, LightningElementFill& element1) {
    fill_element_w1d(model.lightning_188, element0);
    fill_element_w1d(model.thunder_1a0, element1);
}

void lightning_model_age_and_remove_expired_00bc9160(LightningModelState& model, float dt) {
    remove_expired_pairs_w1d(model.thunder_1ac, model.thunder_1a0);
    for (LightningStrand& t : model.thunder_1ac) t.timer_00 = f32_w1d(static_cast<double>(t.timer_00) - dt);
}

// 00BC84A0. Order: remove expired strands with their lightning segments; remember the surviving
// count; if the burst timer is <= 0 (COMISS/JB: NaN does not fire) spawn bursts and re-arm the
// timer; age only the remembered strands (timer -= dt, paired segment alpha -= fade_rate * dt);
// burst timer -= dt. Draw order per burst (all stream 0): x U(-1,1), y U(0.02,0.06) (floats
// 00CE9BAC/00CF0A2C), z U(-1,1), segment count chop(U(0,4)) (00CE3D34), two signs
// chop(U(0,1)) == 0 ? -1 : 1; per segment while the lightning vector has room: half height
// U(def+8, def+0Ch), life U(def+0, def+4), yaw U(2 deg, 6 deg) * second sign, pitch
// U(2 deg, 4 deg) * first sign (floats 00D64AB4/00D64AB8/00D64AB0); per thunder sprite: atlas
// index U(1, def+28h), size U(0.25 def+8, 0.25 def+0Ch) (double 00D7A348), flip chop(U(0, 1000))
// (00CE3804) & 1; finally timer U(0, def+18h).
void lightning_model_update_bursts_00bc84a0(LightningModelState& model, const LightningDefinition& def,
    LightningRandomSource& rng, float dt) {
    remove_expired_pairs_w1d(model.strands_194, model.lightning_188);
    const std::size_t age_count = model.strands_194.size();

    if (model.burst_timer_1b8 <= 0.0f) {
        // FILD of the unsigned count (+4294967296.0f, 00CE3978, when negative), stored as float.
        const float max_bursts = static_cast<float>(def.max_bursts_14);
        std::uint32_t bursts = chop_i64_low32_w1d(rng.uniform_stream0_00bd2f10(0.0f, max_bursts));
        const float kTwoDeg = float_from_bits_w1d(0x3D0EFA36u);    // 00D64AB4
        const float kSixDeg = float_from_bits_w1d(0x3DD67751u);    // 00D64AB8
        const float kFourDeg = float_from_bits_w1d(0x3D8EFA36u);   // 00D64AB0
        const float kMinLifeStart = float_from_bits_w1d(0x501502F9u);  // 00CE4970 = 1.0e10f
        for (; bursts != 0; --bursts) {
            const float x = rng.uniform_stream0_00bd2f10(-1.0f, 1.0f);
            const float y = rng.uniform_stream0_00bd2f10(float_from_bits_w1d(0x3CA3D70Au),
                float_from_bits_w1d(0x3D75C28Fu));
            const float z = rng.uniform_stream0_00bd2f10(-1.0f, 1.0f);
            float inv = inv_length_or_zero_w1d(x, y, z);
            W1D_Vec3 dir{f32_w1d(static_cast<double>(inv) * x), f32_w1d(static_cast<double>(inv) * y),
                f32_w1d(static_cast<double>(inv) * z)};
            std::uint32_t segments = chop_i64_low32_w1d(rng.uniform_stream0_00bd2f10(0.0f, 4.0f));
            const float sign_a = chop_i64_low32_w1d(rng.uniform_stream0_00bd2f10(0.0f, 1.0f)) == 0 ? -1.0f : 1.0f;
            const float sign_b = chop_i64_low32_w1d(rng.uniform_stream0_00bd2f10(0.0f, 1.0f)) == 0 ? -1.0f : 1.0f;
            W1D_Vec3 sum{0.0f, 0.0f, 0.0f};
            float min_life = kMinLifeStart;
            std::uint32_t made = 0;
            for (; segments != 0; --segments) {
                if (static_cast<std::int32_t>(model.lightning_188.size()) + 1 >=
                    static_cast<std::int32_t>(kLightningMaxSegmentsPlusOne_w1d)) {
                    continue;
                }
                ++made;
                const float half_height = rng.uniform_stream0_00bd2f10(def.height_min_08, def.height_max_0c);
                LightningSegment seg;
                seg.dir_00 = dir;
                seg.half_width_0c = f32_w1d(static_cast<double>(half_height) * def.aspect_10);
                seg.half_height_10 = half_height;
                seg.alpha_14 = 1.0f;
                seg.u0_18 = 0.0f;
                seg.v0_1c = 0.0f;
                seg.u1_20 = 1.0f;
                seg.v1_24 = 1.0f;
                seg.anchor_x_28 = 0.5f;    // 00CE3800
                seg.anchor_y_2c = -0.5f;   // 00CE69D0
                model.lightning_188.push_back(seg);
                const float life = rng.uniform_stream0_00bd2f10(def.life_min_00, def.life_max_04);
                if (!(life > min_life)) min_life = life;
                model.strands_194.push_back(LightningStrand{life, life});
                const float yaw = f32_w1d(static_cast<double>(rng.uniform_stream0_00bd2f10(kTwoDeg, kSixDeg)) * sign_b);
                const float pitch = f32_w1d(static_cast<double>(rng.uniform_stream0_00bd2f10(kTwoDeg, kFourDeg)) * sign_a);
                sum.x = f32_w1d(static_cast<double>(dir.x) + sum.x);
                sum.y = f32_w1d(static_cast<double>(dir.y) + sum.y);
                sum.z = f32_w1d(static_cast<double>(sum.z) + dir.z);
                const W1D_Mat4 m = lightning_matrix_multiply_w1d(
                    lightning_matrix_multiply_w1d(lightning_matrix_rotation_z_00b64780(0.0f),
                        lightning_matrix_rotation_x_00b64640(pitch)),
                    lightning_matrix_rotation_y_00b646e0(yaw));
                const float rx = f32_w1d((static_cast<double>(dir.x) * m.m[0] + static_cast<double>(dir.y) * m.m[4]) +
                                         static_cast<double>(dir.z) * m.m[8]);
                const float ry = f32_w1d((static_cast<double>(dir.x) * m.m[1] + static_cast<double>(dir.y) * m.m[5]) +
                                         static_cast<double>(dir.z) * m.m[9]);
                const float rz = f32_w1d((static_cast<double>(dir.x) * m.m[2] + static_cast<double>(dir.y) * m.m[6]) +
                                         static_cast<double>(dir.z) * m.m[10]);
                inv = inv_length_or_zero_w1d(rx, ry, rz);
                dir = W1D_Vec3{f32_w1d(static_cast<double>(inv) * rx), f32_w1d(static_cast<double>(inv) * ry),
                    f32_w1d(static_cast<double>(inv) * rz)};
            }
            if (made <= 1) continue;
            if (static_cast<std::int32_t>(model.thunder_1a0.size()) + 1 >=
                static_cast<std::int32_t>(kLightningMaxSegmentsPlusOne_w1d)) {
                continue;
            }
            const float n = static_cast<float>(made);
            const float ax = f32_w1d(static_cast<double>(sum.x) / n);
            const float ay = f32_w1d(static_cast<double>(sum.y) / n);
            const float az = f32_w1d(static_cast<double>(sum.z) / n);
            inv = inv_length_or_zero_w1d(ax, ay, az);
            const float tx = f32_w1d(static_cast<double>(inv) * ax);
            float ty = f32_w1d(static_cast<double>(inv) * ay);
            const float tz = f32_w1d(static_cast<double>(inv) * az);
            const double height = static_cast<double>(ty) * 4500.0;  // kept as double (FST QWORD)
            if (height > 200.0) continue;                            // double 00CE4D70
            const float item_count = static_cast<float>(static_cast<std::int32_t>(def.items_24.size()));
            const std::int32_t index = chop_i32_w1d(static_cast<double>(rng.uniform_stream0_00bd2f10(1.0f, item_count)) - 1.0);
            // The native indexes def+24h without a bounds check; the model stops here instead.
            if (index < 0 || static_cast<std::size_t>(index) >= def.items_24.size()) continue;
            const LightningAtlasItem& item = def.items_24[static_cast<std::size_t>(index)];
            const float size_hi = f32_w1d(0.25 * def.height_max_0c);
            const float size_lo = f32_w1d(0.25 * def.height_min_08);
            const float size = rng.uniform_stream0_00bd2f10(size_lo, size_hi);
            const double lift = static_cast<double>(item.anchor_y_04) + 1.0;
            if (static_cast<double>(size) * (lift + lift) > height) {
                ty = f32_w1d((static_cast<double>(size) * lift) / 4500.0 + static_cast<double>(ty));
            }
            inv = inv_length_or_zero_w1d(tx, ty, tz);
            LightningSegment thunder;
            thunder.dir_00 = W1D_Vec3{f32_w1d(static_cast<double>(inv) * tx), f32_w1d(static_cast<double>(inv) * ty),
                f32_w1d(static_cast<double>(inv) * tz)};
            thunder.alpha_14 = 1.0f;
            thunder.half_height_10 = size;
            const float ratio = f32_w1d((static_cast<double>(item.tex_u1_1c) - item.tex_u0_14) /
                                        (static_cast<double>(item.tex_v1_20) - item.tex_v0_18));
            thunder.half_width_0c = f32_w1d(static_cast<double>(ratio) * size);
            const std::uint32_t flip = chop_i64_low32_w1d(rng.uniform_stream0_00bd2f10(0.0f, 1000.0f)) & 1u;
            if (flip != 0) {
                thunder.u0_18 = item.tex_u1_1c;
                thunder.v0_1c = item.tex_v0_18;
                thunder.u1_20 = item.tex_u0_14;
                thunder.v1_24 = item.tex_v1_20;
                thunder.anchor_x_28 = f32_w1d(1.0 - static_cast<double>(item.anchor_x_00));
            } else {
                thunder.u0_18 = item.tex_u0_14;
                thunder.v0_1c = item.tex_v0_18;
                thunder.u1_20 = item.tex_u1_1c;
                thunder.v1_24 = item.tex_v1_20;
                thunder.anchor_x_28 = item.anchor_x_00;
            }
            thunder.anchor_y_2c = item.anchor_y_04;
            model.thunder_1a0.push_back(thunder);
            const float thunder_time = f32_w1d(static_cast<double>(min_life) * 0.05000000074505806);  // 00D7A270
            model.thunder_1ac.push_back(LightningStrand{thunder_time, thunder_time});
        }
        model.burst_timer_1b8 = rng.uniform_stream0_00bd2f10(0.0f, def.burst_interval_max_18);
    }

    for (std::size_t i = 0; i < age_count && i < model.strands_194.size(); ++i) {
        LightningStrand& strand = model.strands_194[i];
        strand.timer_00 = f32_w1d(static_cast<double>(strand.timer_00) - dt);
        if (i < model.lightning_188.size()) {
            float& alpha = model.lightning_188[i].alpha_14;
            alpha = f32_w1d(static_cast<double>(alpha) - static_cast<double>(strand.fade_rate_04) * dt);
        }
    }
    model.burst_timer_1b8 = f32_w1d(static_cast<double>(model.burst_timer_1b8) - dt);
}

void lightning_model_tick_00bc9390(LightningModelState& model, const LightningDefinition& def,
    LightningRandomSource& rng, float dt) {
    lightning_model_age_and_remove_expired_00bc9160(model, dt);
    lightning_model_update_bursts_00bc84a0(model, def, rng, dt);
}

void lightning_model_tick_and_fill_00bc7170(LightningModelState& model, const LightningDefinition& def,
    LightningRandomSource& rng, float dt, LightningElementFill& element0, LightningElementFill& element1) {
    lightning_model_tick_00bc9390(model, def, rng, dt);
    lightning_model_fill_segment_vertices_00bc6fa0(model, element0, element1);
}

std::vector<std::uint16_t> lightning_model_static_indices_00bc7460() {
    std::vector<std::uint16_t> indices;
    indices.reserve(300);
    for (std::uint32_t i = 0; i < 200; i += 4) {
        const auto s = static_cast<std::uint16_t>(i);
        indices.push_back(s);
        indices.push_back(static_cast<std::uint16_t>(s + 1));
        indices.push_back(static_cast<std::uint16_t>(s + 2));
        indices.push_back(static_cast<std::uint16_t>(s + 2));
        indices.push_back(static_cast<std::uint16_t>(s + 3));
        indices.push_back(s);
    }
    return indices;
}

// ---------------------------------------------------------------------------------------------
// Matrices
// ---------------------------------------------------------------------------------------------

// 00B64640: [1 0 0 0; 0 c s 0; 0 (-0.0f - s) c 0; 0 0 0 1], FSIN/FCOS of the float angle stored
// as floats (-0.0f is 00D7A208).
W1D_Mat4 lightning_matrix_rotation_x_00b64640(float angle) noexcept {
    const float s = f32_w1d(std::sin(static_cast<double>(angle)));
    const float c = f32_w1d(std::cos(static_cast<double>(angle)));
    W1D_Mat4 r;
    r.m = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f, 0.0f, -0.0f - s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    return r;
}

// 00B646E0: [c 0 (-0.0f - s) 0; 0 1 0 0; s 0 c 0; 0 0 0 1].
W1D_Mat4 lightning_matrix_rotation_y_00b646e0(float angle) noexcept {
    const float s = f32_w1d(std::sin(static_cast<double>(angle)));
    const float c = f32_w1d(std::cos(static_cast<double>(angle)));
    W1D_Mat4 r;
    r.m = {c, 0.0f, -0.0f - s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, s, 0.0f, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    return r;
}

// 00B64780: [c s 0 0; (-0.0f - s) c 0 0; 0 0 1 0; 0 0 0 1].
W1D_Mat4 lightning_matrix_rotation_z_00b64780(float angle) noexcept {
    const float s = f32_w1d(std::sin(static_cast<double>(angle)));
    const float c = f32_w1d(std::cos(static_cast<double>(angle)));
    W1D_Mat4 r;
    r.m = {c, s, 0.0f, 0.0f, -0.0f - s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    return r;
}

W1D_Mat4 lightning_matrix_multiply_w1d(const W1D_Mat4& left, const W1D_Mat4& right) noexcept {
    W1D_Mat4 out;
    for (std::size_t r = 0; r < 4; ++r) {
        for (std::size_t c = 0; c < 4; ++c) {
            double sum = 0.0;
            for (std::size_t k = 0; k < 4; ++k) {
                sum += static_cast<double>(left.m[r * 4 + k]) * right.m[k * 4 + c];
            }
            out.m[r * 4 + c] = f32_w1d(sum);
        }
    }
    return out;
}

// 00BC7C10: S (diag 50, 50, 1, 1; float 00CEB4D4) and T (identity with [3][2] = 40; float
// 00CE685C) are built on the stack; the multiplies run S*R, (S*R)*T, ((S*R)*T)*W.
W1D_Mat4 scene_node_random_roll_world_00bc7c10(const W1D_Mat4& source_world, float roll) noexcept {
    W1D_Mat4 scale;
    scale.m = {50.0f, 0.0f, 0.0f, 0.0f, 0.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    W1D_Mat4 translate;
    translate.m = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 40.0f, 1.0f};
    const W1D_Mat4 sr = lightning_matrix_multiply_w1d(scale, lightning_matrix_rotation_z_00b64780(roll));
    const W1D_Mat4 srt = lightning_matrix_multiply_w1d(sr, translate);
    return lightning_matrix_multiply_w1d(srt, source_world);
}

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part e (cc13-water2b): container plumbing and small shells on native storage. docs/CC13_WATER_2B.md.
// ================================================================================================
namespace bsp::water_weather_cc13 {
namespace {

unsigned char* w2_bytes(void* p) noexcept { return static_cast<unsigned char*>(p); }
template<class T> T& w2_field(void* p, std::size_t offset) noexcept { return *reinterpret_cast<T*>(w2_bytes(p) + offset); }

std::int32_t w2_add32(std::int32_t n, std::int32_t k) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(n) + static_cast<std::uint32_t>(k));
}

// FLD dword / FSTP dword: unchanged except that a signalling NaN is quieted.
std::uint32_t w2_x87_reload_bits(std::uint32_t bits) noexcept {
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0u) bits |= 0x00400000u;
    return bits;
}

void w2_copy_element(void* destination, const void* source, std::uint32_t size, W2_Copy copy) noexcept {
    if (copy == W2_Copy::dwords) {
        std::memcpy(destination, source, size);
        return;
    }
    for (std::uint32_t k = 0; k < size; k += 4) {
        std::uint32_t bits;
        std::memcpy(&bits, static_cast<const unsigned char*>(source) + k, 4);
        bits = w2_x87_reload_bits(bits);
        std::memcpy(static_cast<unsigned char*>(destination) + k, &bits, 4);
    }
}

void w2_release_object(void* object) {
    auto* const count = reinterpret_cast<volatile long*>(w2_bytes(object) + 4);
    if (_InterlockedDecrement(count) == 0) {
        using Slot0 = void(__thiscall*)(void*);
        reinterpret_cast<Slot0>(w2_field<void*>(w2_field<void*>(object, 0), 0))(object);
    }
}

} // namespace

void w2_pod_reserve(W2_Array& array, std::int32_t requested, std::uint32_t element_size, W2_Copy copy) {
    if (requested < 1) requested = 1;
    if (array.capacity >= requested) return;
    const std::size_t bytes = static_cast<std::size_t>(static_cast<std::uint32_t>(requested) * element_size);
    auto* const fresh = static_cast<unsigned char*>(
        singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes}));
    for (std::int32_t i = 0; i < array.count; ++i) {
        unsigned char* const destination = fresh + static_cast<std::uint32_t>(i) * element_size;
        if (destination != nullptr) {
            w2_copy_element(destination, w2_bytes(array.data) + static_cast<std::uint32_t>(i) * element_size,
                            element_size, copy);
        }
    }
    singleton_lifetime_free(array.data);
    array.data = fresh;
    array.capacity = requested;
}

void w2_pod_resize(W2_Array& array, std::int32_t count, std::uint32_t element_size, W2_Copy copy) {
    if (count > array.capacity) w2_pod_reserve(array, count, element_size, copy);
    for (std::int32_t i = array.count; i < count; ++i) {
        unsigned char* const slot = w2_bytes(array.data) + static_cast<std::uint32_t>(i) * element_size;
        if (slot != nullptr) std::memset(slot, 0, element_size);
    }
    while (count < array.count) array.count = w2_add32(array.count, -1);
    array.count = count;
}

void w2_pod_push_back(W2_Array& array, const void* element, std::uint32_t element_size, W2_Copy copy) {
    if (array.count == array.capacity) {
        std::int32_t capacity = w2_add32(array.capacity, array.capacity);
        if (capacity <= 1) capacity = 1;
        w2_pod_reserve(array, capacity, element_size, copy);
    }
    unsigned char* const slot = w2_bytes(array.data) + static_cast<std::uint32_t>(array.count) * element_size;
    if (slot != nullptr) w2_copy_element(slot, element, element_size, copy);
    array.count = w2_add32(array.count, 1);
}

void w2_push_back_dword_via_00b9dfd0(W2_Array& array, const std::uint32_t* value, W2_Reserve reserve_0073eac0) {
    if (array.count == array.capacity) {
        std::int32_t capacity = w2_add32(array.capacity, array.capacity);
        if (capacity <= 1) capacity = 1;
        reserve_0073eac0(array, capacity);
    }
    auto* const slot = static_cast<std::uint32_t*>(array.data) + array.count;
    if (slot != nullptr) *slot = *value;
    array.count = w2_add32(array.count, 1);
}

void w2_pod_destroy(W2_Array& array, std::uint32_t element_size) {
    w2_pod_resize(array, 0, element_size, W2_Copy::dwords);
    singleton_lifetime_free(array.data);
}

void w2_destroy_via(W2_Array& array, W2_Resize resize) {
    resize(array, 0);
    singleton_lifetime_free(array.data);
}

W2_Array& w2_byte_vector_assign_00b9e5d0(W2_Array& self, const W2_Array& source) {
    w2_pod_resize(self, 0, 1, W2_Copy::dwords);
    w2_pod_reserve(self, source.count, 1, W2_Copy::dwords);
    for (std::int32_t i = 0; i < source.count; ++i) {
        w2_pod_push_back(self, static_cast<const unsigned char*>(source.data) + i, 1, W2_Copy::dwords);
    }
    return self;
}

void w2_clear_counting_down(W2_Array& array, W2_Reserve reserve, bool free_data) {
    if (array.capacity < 0) reserve(array, 0);
    while (array.count > 0) array.count = w2_add32(array.count, -1);
    array.count = 0;
    if (free_data) singleton_lifetime_free(array.data);
}

void w2_coast_point_vector_delete_00bb0590(W2_Array** slot, W2_Reserve reserve) {
    W2_Array* const vector = *slot;
    if (vector != nullptr) {
        w2_clear_counting_down(*vector, reserve, true);
        singleton_lifetime_free(vector);
    }
}

void w2_free_storage_at_8(void* object) noexcept {
    void*& data = w2_field<void*>(object, 8);
    if (data != nullptr) {
        singleton_lifetime_free(data);
        data = nullptr;
    }
    w2_field<std::uint32_t>(object, 4) = 0;
    w2_field<std::uint32_t>(object, 0) = 0;
}

void w2_pointer_array_grow_00bc6ca0(W2_Array& array) { engine_services_cc13::grow_pointer_array_00bf2ee0(array); }

void w2_free_data(void** slot) noexcept { engine_services_cc13::free_owned_block(slot); }

void** w2_ref_release_return_self_00b9d6c0(void** slot) {
    return engine_services_cc13::release_ref_slot_return_self_00be9b80(slot);
}

void w2_ref_release(void** slot) { engine_services_cc13::release_ref_slot(slot); }

void** w2_ref_add_00ba2b10(void** slot) {
    if (*slot != nullptr) _InterlockedIncrement(reinterpret_cast<volatile long*>(w2_bytes(*slot) + 4));
    return slot;
}

void** w2_ref_assign(void** slot, void* object) { return engine_services_cc13::assign_ref_slot_00bef1e0(slot, object); }

void w2_node_handle_release_00b9ab40(void** slot, void (*unlink_and_release)(void* node)) {
    if (*slot != nullptr) unlink_and_release(*slot);
}

void w2_release_string(void* header, NativeStringStorage& strings) noexcept {
    destroy_native_string_header_0041dd20(header, strings);
}

void w2_archive_name_record_destroy(void* record, NativeStringStorage& strings) noexcept {
    void* const data = w2_field<void*>(record, 0x18);
    if (data != nullptr) singleton_lifetime_free(data);
    w2_field<std::uint32_t>(record, 0x18) = 0;
    w2_field<std::uint32_t>(record, 0x1C) = 0;
    w2_field<std::uint32_t>(record, 0x20) = 0;
    destroy_native_string_header_0041dd20(record, strings);
}

void* w2_name_record_copy_construct_00bb64a0(void* record, const NativeString& name, NativeStringStorage& strings) {
    auto& text = w2_field<NativeString>(record, 0);
    ::new (&text) NativeString();
    text.copy_from_00be0a30_fragment(strings, name);
    w2_field<std::uint32_t>(record, 8) = 0;
    w2_field<std::uint32_t>(record, 0x0C) = 0;
    w2_field<std::uint32_t>(record, 0x10) = 0;
    return record;
}

void* w2_archive_entry_record_construct_00bba8f0(void* record, const NativeString& name, std::uint32_t a,
    std::uint32_t b, std::uint8_t flag, std::uint32_t c, NativeStringStorage& strings) {
    auto& text = w2_field<NativeString>(record, 0);
    ::new (&text) NativeString();
    text.copy_from_00be0a30_fragment(strings, name);
    w2_field<std::uint32_t>(record, 8) = a;
    w2_field<std::uint8_t>(record, 0x10) = flag;
    w2_field<std::uint32_t>(record, 0x0C) = b;
    w2_field<std::uint32_t>(record, 0x14) = c;
    return record;
}

void* w2_archive_entry_record_copy_00bba950(void* record, const void* source, NativeStringStorage& strings) {
    auto* const from = const_cast<void*>(source);
    auto& text = w2_field<NativeString>(record, 0);
    ::new (&text) NativeString();
    text.copy_from_00be0a30_fragment(strings, w2_field<NativeString>(from, 0));
    w2_field<std::uint32_t>(record, 8) = w2_field<std::uint32_t>(from, 8);
    w2_field<std::uint32_t>(record, 0x0C) = w2_field<std::uint32_t>(from, 0x0C);
    w2_field<std::uint8_t>(record, 0x10) = w2_field<std::uint8_t>(from, 0x10);
    w2_field<std::uint32_t>(record, 0x14) = w2_field<std::uint32_t>(from, 0x14);
    return record;
}

void* w2_stream_buffer_read_whole_00bb8530(void* buffer, void* stream, std::uint32_t size) {
    void* const data = singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
    w2_field<void*>(buffer, 0) = data;
    using Read = void(__thiscall*)(void*, void*, std::uint32_t, void*);
    reinterpret_cast<Read>(w2_field<void*>(w2_field<void*>(stream, 0), 0x24))(stream, data, size, w2_bytes(buffer) + 4);
    w2_field<void*>(buffer, 8) = w2_field<void*>(buffer, 0);
    return buffer;
}

void w2_free_first_word(void* object) noexcept { singleton_lifetime_free(w2_field<void*>(object, 0)); }

void* w2_memory_stream_create_00bb9100(void* backing) {
    void* const stream = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x14, 0x14});
    if (stream != nullptr) {
        w2_field<std::uint32_t>(stream, 0) = 0x00CEB130u;
        w2_field<std::uint32_t>(stream, 4) = 1;
        w2_field<std::uint32_t>(stream, 8) = 0;
        w2_field<std::uint32_t>(stream, 0x0C) = 0;
        w2_field<std::uint32_t>(stream, 0x10) = 0;
        w2_field<std::uint32_t>(stream, 0) = 0x00D64328u;
    }
    // The listing continues through a null allocation (operator new throws in practice).
    void*& held = w2_field<void*>(stream, 8);
    if (held != nullptr) {
        w2_release_object(held);
        held = nullptr;
    }
    held = backing;
    _InterlockedIncrement(reinterpret_cast<volatile long*>(w2_bytes(backing) + 4));
    const std::uint32_t begin = w2_field<std::uint32_t>(backing, 8);
    w2_field<std::uint32_t>(stream, 0x10) = begin;
    w2_field<std::uint32_t>(stream, 0x0C) = w2_field<std::uint32_t>(backing, 0x0C) + begin;
    return stream;
}

void* w2_record30_copy_00bc7f00(void* destination, const void* source) noexcept {
    w2_copy_element(destination, source, 0x30, W2_Copy::floats);
    return destination;
}

void w2_record30_remove_swap_last_00bc81d0(W2_Array& array, std::int32_t index) noexcept {
    if (index == -1) return;
    if (index != array.count - 1) {
        w2_record30_copy_00bc7f00(w2_bytes(array.data) + static_cast<std::uint32_t>(index) * 0x30u,
                                  w2_bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x30u - 0x30u);
    }
    array.count = w2_add32(array.count, -1);
}

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part f (cc13-left1): bodies left open by Part e. docs/CC13_LEFT_1.md.
// ================================================================================================
namespace bsp::water_weather_cc13 {
namespace {

unsigned char* w3_bytes(void* p) noexcept { return static_cast<unsigned char*>(p); }
const unsigned char* w3_bytes(const void* p) noexcept { return static_cast<const unsigned char*>(p); }
template<class T> T& w3_field(void* p, std::size_t offset) noexcept { return *reinterpret_cast<T*>(w3_bytes(p) + offset); }
template<class T> const T& w3_field(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const T*>(w3_bytes(p) + offset);
}

std::int32_t w3_add32(std::int32_t n, std::int32_t k) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(n) + static_cast<std::uint32_t>(k));
}

void* w3_new(std::size_t bytes) { return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes}); }

W2_Array& w3_mask(void* region) noexcept { return w3_field<W2_Array>(region, 4); }

void w3_copy_region_tail(void* self, const void* source) noexcept {
    for (std::size_t offset = 0x10; offset < 0x30; offset += 4) {
        w3_field<std::uint32_t>(self, offset) = w3_field<std::uint32_t>(source, offset);
    }
}

}  // namespace

void w3_cell_grid_resize_00b9d490(void* grid, std::int32_t width, std::int32_t height) {
    const std::uint64_t wide = static_cast<std::uint64_t>(static_cast<std::uint32_t>(width * height)) * 0x18u;
    const std::uint32_t bytes = (wide >> 32) != 0 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(wide);
    auto* const fresh = static_cast<unsigned char*>(w3_new(bytes));
    const std::int32_t old_width = w3_field<std::int32_t>(grid, 0);
    const std::int32_t columns = old_width < width ? old_width : width;
    const std::int32_t rows = w3_field<std::int32_t>(grid, 4) < height ? w3_field<std::int32_t>(grid, 4) : height;
    auto* const old = static_cast<unsigned char*>(w3_field<void*>(grid, 8));
    for (std::int32_t row = 0; row < rows; ++row) {
        for (std::int32_t column = 0; column < columns; ++column) {
            std::memcpy(fresh + static_cast<std::uint32_t>(row * width + column) * 0x18u,
                        old + static_cast<std::uint32_t>(old_width * row + column) * 0x18u, 0x18);
        }
    }
    if (old != nullptr) {
        singleton_lifetime_free(old);
        w3_field<void*>(grid, 8) = nullptr;
    }
    w3_field<std::int32_t>(grid, 0) = width;
    w3_field<std::int32_t>(grid, 4) = height;
    w3_field<void*>(grid, 8) = fresh;
}

void* w3_cell_grid_assign_00b9e0c0(void* self, const void* source) {
    if (self == source) return self;
    void*& data = w3_field<void*>(self, 8);
    if (data != nullptr) {
        singleton_lifetime_free(data);
        data = nullptr;
    }
    w3_field<std::int32_t>(self, 0) = 0;
    w3_field<std::int32_t>(self, 4) = 0;
    w3_cell_grid_resize_00b9d490(self, w3_field<std::int32_t>(source, 0), w3_field<std::int32_t>(source, 4));
    const std::int32_t width = w3_field<std::int32_t>(self, 0);
    const std::int32_t source_width = w3_field<std::int32_t>(source, 0);
    for (std::int32_t row = 0; row < w3_field<std::int32_t>(self, 4); ++row) {
        for (std::int32_t column = 0; column < width; ++column) {
            std::memcpy(w3_bytes(data) + static_cast<std::uint32_t>(width * row + column) * 0x18u,
                        w3_bytes(w3_field<void*>(source, 8)) + static_cast<std::uint32_t>(source_width * row + column) * 0x18u,
                        0x18);
        }
    }
    return self;
}

void w3_grid_vector_reserve_00b9e7a0(W2_Array& array, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (array.capacity >= requested) return;
    auto* const fresh = static_cast<unsigned char*>(w3_new(static_cast<std::uint32_t>(requested) * 0x0Cu));
    for (std::int32_t i = 0; i < array.count; ++i) {
        unsigned char* const grid = fresh + static_cast<std::uint32_t>(i) * 0x0Cu;
        if (grid != nullptr) {
            std::memset(grid, 0, 0x0C);
            w3_cell_grid_assign_00b9e0c0(grid, w3_bytes(array.data) + static_cast<std::uint32_t>(i) * 0x0Cu);
        }
    }
    for (std::int32_t i = 0; i < array.count; ++i) {
        w2_free_storage_at_8(w3_bytes(array.data) + static_cast<std::uint32_t>(i) * 0x0Cu);
    }
    singleton_lifetime_free(array.data);
    array.data = fresh;
    array.capacity = requested;
}

void w3_grid_vector_resize_00b9ef20(W2_Array& array, std::int32_t count) {
    if (array.capacity < count) w3_grid_vector_reserve_00b9e7a0(array, count);
    for (std::int32_t i = array.count; i < count; ++i) {
        unsigned char* const grid = w3_bytes(array.data) + static_cast<std::uint32_t>(i) * 0x0Cu;
        if (grid != nullptr) std::memset(grid, 0, 0x0C);
    }
    while (count < array.count) {
        array.count = w3_add32(array.count, -1);
        w2_free_storage_at_8(w3_bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x0Cu);
    }
    array.count = count;
}

void w3_coverage_region_destroy_mask_00b9e6b0(void* region) { w2_pod_destroy(w3_mask(region), 1); }

void* w3_coverage_region_assign_00b9e6f0(void* self, const void* source) {
    w3_field<std::uint32_t>(self, 0) = w3_field<std::uint32_t>(source, 0);
    w2_byte_vector_assign_00b9e5d0(w3_mask(self), w3_field<W2_Array>(source, 4));
    w3_copy_region_tail(self, source);
    return self;
}

void* w3_coverage_region_copy_construct_00b9ed40(void* self, const void* source) {
    w3_field<std::uint32_t>(self, 0) = w3_field<std::uint32_t>(source, 0);
    w3_mask(self) = W2_Array{nullptr, 0, 0};
    w2_byte_vector_assign_00b9e5d0(w3_mask(self), w3_field<W2_Array>(source, 4));
    w3_copy_region_tail(self, source);
    return self;
}

void w3_coverage_region_vector_reserve_00b9eda0(W2_Array& array, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (array.capacity >= requested) return;
    auto* const fresh = static_cast<unsigned char*>(w3_new(static_cast<std::uint32_t>(requested) * 0x30u));
    for (std::int32_t i = 0; i < array.count; ++i) {
        unsigned char* const region = fresh + static_cast<std::uint32_t>(i) * 0x30u;
        if (region != nullptr) {
            w3_coverage_region_copy_construct_00b9ed40(region, w3_bytes(array.data) + static_cast<std::uint32_t>(i) * 0x30u);
        }
    }
    for (std::int32_t i = 0; i < array.count; ++i) {
        w3_coverage_region_destroy_mask_00b9e6b0(w3_bytes(array.data) + static_cast<std::uint32_t>(i) * 0x30u);
    }
    singleton_lifetime_free(array.data);
    array.data = fresh;
    array.capacity = requested;
}

void w3_coverage_region_vector_resize_00b9efa0(W2_Array& array, std::int32_t count) {
    if (array.capacity < count) w3_coverage_region_vector_reserve_00b9eda0(array, count);
    for (std::int32_t i = array.count; i < count; ++i) {
        unsigned char* const region = w3_bytes(array.data) + static_cast<std::uint32_t>(i) * 0x30u;
        if (region != nullptr) std::memset(region, 0, 0x10);
    }
    while (count < array.count) {
        array.count = w3_add32(array.count, -1);
        w3_coverage_region_destroy_mask_00b9e6b0(w3_bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x30u);
    }
    array.count = count;
}

void w3_coverage_region_vector_push_back_00b9fff0(W2_Array& array, const void* region) {
    if (array.count == array.capacity) {
        std::int32_t capacity = w3_add32(array.capacity, array.capacity);
        if (capacity < 2) capacity = 1;
        w3_coverage_region_vector_reserve_00b9eda0(array, capacity);
    }
    unsigned char* const slot = w3_bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x30u;
    if (slot != nullptr) w3_coverage_region_copy_construct_00b9ed40(slot, region);
    array.count = w3_add32(array.count, 1);
}

void w3_coverage_region_vector_erase_at_00b9ece0(W2_Array& array, std::int32_t index) {
    if (index == -1) return;
    unsigned char* const last = w3_bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x30u - 0x30u;
    if (index != array.count - 1) {
        w3_coverage_region_assign_00b9e6f0(w3_bytes(array.data) + static_cast<std::uint32_t>(index) * 0x30u, last);
    }
    w3_coverage_region_destroy_mask_00b9e6b0(last);
    array.count = w3_add32(array.count, -1);
}

void w3_lightning_segment_vector_reserve_00bc7fa0(W2_Array& array, std::int32_t requested) {
    w2_pod_reserve(array, requested, 0x30, W2_Copy::floats);
}

void w3_lightning_segment_vector_append_00bc8150(W2_Array& array, const void* segment) {
    w2_pod_push_back(array, segment, 0x30, W2_Copy::floats);
}

namespace {

struct W3_Header {
    std::uint32_t length;
    char* data;
};

// The pooled temporary schedule shared by 00BD1BB0 / 00BD1D90 / 00BD1E80 / 00BD26B0: build the
// temporary (resize preserving, copy length + 1), append it when nonempty, release length + 1 bytes.
void* w3_append_text(void* builder, const char* text, NativeStringStorage& strings) {
    W3_Header temporary{0, nullptr};
    resize_native_string_header_0041dd40(&temporary, strings, static_cast<std::uint32_t>(std::strlen(text)), true);
    char* const data = temporary.data;
    const std::uint32_t length = temporary.length;
    if (data != nullptr) std::memcpy(data, text, length + 1u);
    if (length != 0) {
        const std::uint32_t previous = w3_field<std::uint32_t>(builder, 0);
        resize_native_string_header_0041dd40(builder, strings, length + previous, true);
        std::memcpy(w3_field<char*>(builder, 4) + previous, data, length);
    }
    if (data != nullptr) strings.release(data, length + 1u);
    return builder;
}

void w3_check_formatted(int length, std::size_t capacity) {
    if (length < 0 || static_cast<std::size_t>(length) >= capacity) {
        throw std::length_error("Native builder format exceeds its 64-byte temporary domain");
    }
}

}  // namespace

void w3_game_allocator_destroy_00bd15d0(void* allocator, volatile std::uint32_t& small_returns_disabled_01090aa4,
                                        void* volatile& published_01090aac) noexcept {
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(w3_bytes(allocator) + 0xB89A98u);
    auto& depth = *reinterpret_cast<volatile std::uint32_t*>(w3_bytes(allocator) + 0xB89AB0u);
    w3_field<std::uint32_t>(allocator, 0) = 0x00D68204u;
    small_returns_disabled_01090aa4 = 1;
    while (static_cast<std::int32_t>(depth) > 0) {
        depth = depth - 1u;
        LeaveCriticalSection(section);
    }
    DeleteCriticalSection(section);
    published_01090aac = nullptr;
    w3_field<std::uint32_t>(allocator, 0) = 0x00CE3818u;
}

TrackedCriticalSection* w3_counted_critical_section_create_00bd1890() { return critical_section_create_00bd1860(); }

void w3_event_destroy_00bd18f0(void* event) noexcept {
    w3_field<std::uint32_t>(event, 0) = 0x00D6821Cu;
    CloseHandle(w3_field<HANDLE>(event, 4));
    w3_field<std::uint32_t>(event, 0) = 0x00D68208u;
}

void* w3_string_builder_append_char_00bd1b10(void* builder, char c, NativeStringStorage& strings) {
    char* const block = strings.allocate(2);
    block[1] = 0;
    block[0] = c;
    const std::uint32_t previous = w3_field<std::uint32_t>(builder, 0);
    resize_native_string_header_0041dd40(builder, strings, previous + 1u, true);
    w3_field<char*>(builder, 4)[previous] = block[0];
    strings.release(block, 2);
    return builder;
}

void* w3_string_builder_append_float_00bd1d90(void* builder, float value, NativeStringStorage& strings,
                                              const char* null_format_01090ab4) {
    const char* format = w3_field<const char*>(builder, 0x14);
    if (format == nullptr) format = null_format_01090ab4;
    char buffer[64];
    w3_check_formatted(std::snprintf(buffer, sizeof buffer, format, static_cast<double>(value)), sizeof buffer);
    return w3_append_text(builder, buffer, strings);
}

void* w3_string_builder_append_pointer_00bd1e80(void* builder, const void* value, NativeStringStorage& strings) {
    char buffer[64];
    w3_check_formatted(std::snprintf(buffer, sizeof buffer, "0x%p", value), sizeof buffer);
    return w3_append_text(builder, buffer, strings);
}

void* w3_error_reporter_singleton_00bd2180(void*& published_0109dbdc, SingletonLifetimeDomain& domain,
                                           engine_services_cc13::EngineServiceCalls& calls) {
    if (published_0109dbdc == nullptr) {
        SystemSingletonCriticalSection* const section = domain.get_manager_00415350()->system_owner().section_10;
        if (section != nullptr) {
            singleton_enter_critical_section(*section);
            ++section->recursion_18;
        }
        if (published_0109dbdc == nullptr) {
            void* const raw = w3_new(0x18);
            published_0109dbdc = raw != nullptr ? engine_services_cc13::error_reporter_construct_00bf2c40(raw, calls)
                                                : nullptr;
            domain.get_manager_00415350()->register_object(published_0109dbdc);
        }
        if (section != nullptr) {
            --section->recursion_18;
            singleton_leave_critical_section(*section);
        }
    }
    return published_0109dbdc;
}

void* w3_native_string_copy_construct_into_00bd2390(const void* source, void* destination,
                                                    NativeStringStorage& strings) {
    w3_field<std::uint32_t>(destination, 0) = 0;
    w3_field<char*>(destination, 4) = nullptr;
    if (destination != source) {
        resize_native_string_header_0041dd40(destination, strings, w3_field<std::uint32_t>(source, 0), true);
        if (w3_field<std::uint32_t>(source, 0) != 0) {
            std::memcpy(w3_field<char*>(destination, 4), w3_field<char*>(source, 4), w3_field<std::uint32_t>(destination, 0));
        }
    }
    return destination;
}

void* w3_log_message_append_cstring_00bd26b0(void* message, const char* text, NativeStringStorage& strings) {
    w3_append_text(w3_bytes(message) + 0x20, text, strings);
    return message;
}

void* w3_deleting_destructor(void* object, std::uint32_t flags, std::uint32_t vtable, W3_Body body, W3_Body dispose) {
    if (vtable != 0) w3_field<std::uint32_t>(object, 0) = vtable;
    if (body != nullptr) body(object);
    if ((flags & 1u) != 0) dispose(object);
    return object;
}

void* w3_adjustor_thunk_minus4(void* secondary, std::uint32_t flags, void* (*target)(void* object, std::uint32_t flags)) {
    return target(w3_bytes(secondary) - 4, flags);
}

void* w3_cloud_instance_type_info_thunk_00bbe7c0(void* object, W3_Body init_type_info_00bbe770) {
    init_type_info_00bbe770(object);
    return object;
}

void w3_delete_slot_if_set(void** slot) {
    void* const object = *slot;
    if (object != nullptr) {
        using Delete = void*(__thiscall*)(void*, std::uint32_t);
        reinterpret_cast<Delete>(w3_field<void*>(w3_field<void*>(object, 0), 0))(object, 1);
        *slot = nullptr;
    }
}

void* w3_ref_counted_base_construct_00bc7b60(void* object) noexcept {
    w3_field<std::uint32_t>(object, 0) = 0x00CEB130u;
    w3_field<std::uint32_t>(object, 4) = 1;
    w3_field<std::uint32_t>(object, 0) = 0x00D649DCu;
    return object;
}

void* w3_caustics_texture_source_construct_00bbc670(void* object) noexcept {
    construct_native_texture_source_00c30470(object);
    w3_field<std::uint32_t>(object, 0) = 0x00D64478u;
    return object;
}

void* w3_shore_wave_texture_source_construct_00bbc7c0(void* object) noexcept {
    construct_native_texture_source_00c30470(object);
    w3_field<std::uint32_t>(object, 0) = 0x00D644B4u;
    return object;
}

void* w3_waterdrops_pooled_construct_00bc2160(void* object, W3_Body waterdrops_construct_00bc2030) {
    waterdrops_construct_00bc2030(object);
    w3_field<std::uint32_t>(object, 0) = 0x00D647D0u;
    return object;
}

namespace {

void* w3_construct_named_model(void* object, const char* name, NativeStringStorage& strings,
                               W3_ConstructGeneratedModel construct_00b75030, std::uint32_t vtable) {
    W3_Header temporary{0, nullptr};
    resize_native_string_header_0041dd40(&temporary, strings, static_cast<std::uint32_t>(std::strlen(name)), true);
    if (temporary.data != nullptr) std::memcpy(temporary.data, name, temporary.length + 1u);
    construct_00b75030(object, &temporary);
    if (temporary.data != nullptr) strings.release(temporary.data, temporary.length + 1u);
    w3_field<std::uint32_t>(object, 0) = vtable;
    return object;
}

}  // namespace

void* w3_cloud_plane_model_construct_base_00bc7db0(void* object, NativeStringStorage& strings,
                                                  W3_ConstructGeneratedModel construct_00b75030) {
    return w3_construct_named_model(object, "cloudplane", strings, construct_00b75030, 0x00D649E8u);
}

void* w3_lightning_model_construct_base_00bc93e0(void* object, NativeStringStorage& strings,
                                                W3_ConstructGeneratedModel construct_00b75030) {
    w3_construct_named_model(object, "Lightning", strings, construct_00b75030, 0x00D64A50u);
    for (std::size_t offset = 0x188; offset <= 0x1B4; offset += 4) w3_field<std::uint32_t>(object, offset) = 0;
    return object;
}

void* w3_memory_stream_cast_00bb8690(void* stream, void* memory_file_type_0109dba0) {
    if (stream == nullptr) return nullptr;
    using IsKindOf = bool(__thiscall*)(void*, void*);
    const bool kind = reinterpret_cast<IsKindOf>(w3_field<void*>(w3_field<void*>(stream, 0), 0x0C))(
        stream, memory_file_type_0109dba0);
    return kind ? stream : nullptr;
}

void* w3_mpkg_provider_open_entry_00bb8e70(void* provider, const void* name, std::uint32_t flags,
    void* (*open_named_entry_00bb8d60)(void* state, const void* name, std::uint32_t flags)) {
    return open_named_entry_00bb8d60(w3_field<void*>(provider, 0x14), name, flags);
}

void* w3_mpak_directory_make_member_iterator_00bb5330(void* out, const void* name, void* container, void* first,
    void* end, void* (*find_member_00bb4f40)(void* first, void* end, const void* name)) {
    w3_field<void*>(out, 4) = find_member_00bb4f40(first, end, name);
    w3_field<void*>(out, 0) = container;
    return out;
}

void w3_tracer_curve_destroy_records_00baca80(W2_Array& records, W2_Resize resize_00bac5e0) {
    w2_destroy_via(records, resize_00bac5e0);
}

void w3_model_pool_construct(W3_ModelPoolStorage& pool, AllocatorListDomain& list_00e188b4, std::uint32_t vtable) {
    list_00e188b4.prepend_base_element(*reinterpret_cast<AllocatorListElement*>(&pool));
    pool.native_vtable_00 = vtable;
    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(pool.critical_section_0c));
    pool.recursion_24 = 0;
    pool.blocks_28 = nullptr;
    pool.block_count_2c = 0;
    pool.table_capacity_30 = 0;
    pool.first_free_34 = 0xFFFFFFFFu;
    if (pool.table_capacity_30 < 0x20) {
        pool.table_capacity_30 = 0x20;
        auto** const table = static_cast<std::byte**>(w3_new(0x80));
        for (std::uint32_t i = 0; i < pool.block_count_2c; ++i) {
            if (table + i != nullptr) table[i] = pool.blocks_28[i];
        }
        if (pool.blocks_28 != nullptr) singleton_lifetime_free(pool.blocks_28);
        pool.blocks_28 = table;
    }
}

void w3_model_pool_destroy(W3_ModelPoolStorage& pool, AllocatorListDomain& list_00e188b4, std::uint32_t vtable) noexcept {
    pool.native_vtable_00 = vtable;
    for (std::uint32_t i = 0; i < pool.block_count_2c; ++i) singleton_lifetime_free(pool.blocks_28[i]);
    if (pool.blocks_28 != nullptr) singleton_lifetime_free(pool.blocks_28);
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(pool.critical_section_0c);
    while (pool.recursion_24 > 0) {
        pool.recursion_24 = w3_add32(pool.recursion_24, -1);
        LeaveCriticalSection(section);
    }
    DeleteCriticalSection(section);
    list_00e188b4.unlink_base_element_00403970(*reinterpret_cast<AllocatorListElement*>(&pool));
}

void w3_model_pool_free_thunk(void* object, void* pool, void (*pool_free)(void* pool, void* object)) {
    pool_free(pool, object);
}

namespace {

void w3_reserve_strands(W2_Array& array, std::int32_t n) { w2_pod_reserve(array, n, 8, W2_Copy::dwords); }
void w3_reserve_segments(W2_Array& array, std::int32_t n) { w2_pod_reserve(array, n, 0x30, W2_Copy::floats); }
void w3_reserve_spheres(W2_Array& array, std::int32_t n) { w2_pod_reserve(array, n, 0x10, W2_Copy::floats); }
void w3_reserve_dwords(W2_Array& array, std::int32_t n) { w2_pod_reserve(array, n, 4, W2_Copy::dwords); }

void w3_release_field(void* object, std::size_t offset) { w2_ref_release(&w3_field<void*>(object, offset)); }

}  // namespace

void w3_lightning_model_destruct_body_00bc8380(void* model, W3_Body model_destroy_00b750c0) {
    w3_field<std::uint32_t>(model, 0) = 0x00D64A50u;
    w2_clear_counting_down(w3_field<W2_Array>(model, 0x1AC), &w3_reserve_strands, true);
    w2_clear_counting_down(w3_field<W2_Array>(model, 0x1A0), &w3_reserve_segments, true);
    w2_clear_counting_down(w3_field<W2_Array>(model, 0x194), &w3_reserve_strands, true);
    w2_clear_counting_down(w3_field<W2_Array>(model, 0x188), &w3_reserve_segments, true);
    model_destroy_00b750c0(model);
}

void w3_cloud_class_destruct_body_00bc4660(void* cloud_class, NativeStringStorage& strings,
                                          W3_Body destroy_base_00bd30f0) {
    w3_field<std::uint32_t>(cloud_class, 0) = 0x00D64860u;
    W2_Array& puffs = w3_field<W2_Array>(cloud_class, 0x1C);
    for (std::int32_t i = 0; i < puffs.count; ++i) w2_ref_release(static_cast<void**>(puffs.data) + i);
    w3_release_field(cloud_class, 0x68);
    destroy_native_string_header_0041dd20(w3_bytes(cloud_class) + 0x60, strings);
    w2_clear_counting_down(w3_field<W2_Array>(cloud_class, 0x30), &w3_reserve_spheres, true);
    w2_pod_resize(puffs, 0, 4, W2_Copy::dwords);
    singleton_lifetime_free(puffs.data);
    destroy_native_string_header_0041dd20(w3_bytes(cloud_class) + 0x0C, strings);
    destroy_base_00bd30f0(cloud_class);
}

void* w3_cloud_class_construct_00bc4a60(void* cloud_class, std::uint32_t manager_18, std::uint32_t value_14,
    const void* name, const void* texture, std::uint32_t owner_08, NativeStringStorage& strings,
    W3_Body load_00bc47d0) {
    w3_field<std::uint32_t>(cloud_class, 0) = 0x00CEB130u;
    w3_field<std::uint32_t>(cloud_class, 4) = 1;
    w3_field<std::uint32_t>(cloud_class, 0) = 0x00D64860u;
    w3_field<std::uint32_t>(cloud_class, 8) = owner_08;
    w3_native_string_copy_construct_into_00bd2390(name, w3_bytes(cloud_class) + 0x0C, strings);
    w3_field<std::uint32_t>(cloud_class, 0x14) = value_14;
    w3_field<std::uint32_t>(cloud_class, 0x18) = manager_18;
    w3_field<W2_Array>(cloud_class, 0x1C) = W2_Array{nullptr, 0, 0};
    w3_field<W2_Array>(cloud_class, 0x30) = W2_Array{nullptr, 0, 0};
    w3_native_string_copy_construct_into_00bd2390(texture, w3_bytes(cloud_class) + 0x60, strings);
    w3_field<std::uint8_t>(cloud_class, 0x28) = 0;
    w3_field<std::uint8_t>(cloud_class, 0x40) = 0;
    load_00bc47d0(cloud_class);
    return cloud_class;
}

void w3_sky_box_destruct_body_00bbf6d0(void* sky_box, W3_Body destroy_base_00bd30f0) {
    w3_field<std::uint32_t>(sky_box, 0) = 0x00D64618u;
    for (std::size_t layer = 0; layer < 3; ++layer) w3_release_field(sky_box, 0x84 + layer * 0x54);
    w3_release_field(sky_box, 8);
    w3_release_field(sky_box, 0x10);
    w2_clear_counting_down(w3_field<W2_Array>(sky_box, 0x24), &w3_reserve_dwords, true);
    w2_clear_counting_down(w3_field<W2_Array>(sky_box, 0x18), &w3_reserve_dwords, true);
    destroy_base_00bd30f0(sky_box);
}

}  // namespace bsp::water_weather_cc13
