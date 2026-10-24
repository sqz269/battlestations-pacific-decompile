#include "bsp/water_weather_cc13.hpp"

#include <algorithm>
#include <intrin.h>
#include <new>

#include "bsp/allocator_list.hpp"
#include "bsp/native_resource_cache_leaves.hpp"
#include "bsp/random.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_physical_index_tree.hpp"
#include "bsp/native_pak_registry.hpp"
#include "bsp/native_vfs_container_allocation.hpp"
#include "bsp/native_vfs_open_logging.hpp"
#include "bsp/sound_stream_runtime.hpp"
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

void ocean_wave_field_projected_grid_00ba5240(std::uint8_t* field, OceanProjectedGridHost& host) {
    const auto word = [field](std::size_t offset) -> std::uint32_t& {
        return *reinterpret_cast<std::uint32_t*>(field + offset);
    };
    const auto pointer = [](std::uint32_t value) {
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
    };
    void* const model = pointer(word(0x11C));
    host.geometry_clear_draw_sections_00b73c10(
        host.generated_model_get_geometry_00b74640(model, 0));  // 00BA5267, 00BA526E
    host.geometry_set_index_stream_00b73b70(host.generated_model_get_geometry_00b74640(model, 0),
                                            nullptr);  // 00BA527B, 00BA5282
    if (word(0x124) != 0) {
        host.ref_release(pointer(word(0x124)));  // 00BA5295..00BA52A5
        word(0x124) = 0;                         // 00BA52A7
    }
    void* const format = host.renderer_vertex_format_38("projocean.mvfm");  // 00BA52F4, 00D5EDD0
    word(0x124) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(
        host.renderer_create_vertex_buffer_5c(word(0x100) * word(0xFC), 1, format)));  // 00BA533B
    if (format != nullptr) host.ref_release(format);  // 00BA533D..00BA535B

    float* uv = host.vertex_buffer_lock_10(pointer(word(0x124)), word(0x100) * word(0xFC), 0,
                                           0);  // 00BA5378
    // 00BA5389..00BA53CE: du = float(1 / u32(W-1)), dv = float(1 / u32(H-1)).
    const float du = static_cast<float>(1.0 / static_cast<double>(u32_to_float_b(word(0xFC) - 1u)));
    const float dv = static_cast<float>(1.0 / static_cast<double>(u32_to_float_b(word(0x100) - 1u)));
    float v = 0.0f;
    for (std::uint32_t r = 0; r < word(0x100); ++r) {
        float u = 0.0f;
        for (std::uint32_t c = 0; c < word(0xFC); ++c) {
            uv[0] = u;  // 00BA53FE
            uv[1] = v;  // 00BA5404
            uv += 2;
            u = static_cast<float>(static_cast<double>(u) + du);
        }
        v = static_cast<float>(static_cast<double>(v) + dv);
    }
    host.vertex_buffer_unlock_14(pointer(word(0x124)));  // 00BA543F

    const std::uint32_t height = word(0x100);
    const std::uint32_t width = word(0xFC);
    void* const index_buffer =
        host.renderer_create_index_buffer_60(height * width * 2u, 1, 0x65);  // 00BA5466
    std::uint16_t* const indices = host.index_buffer_lock_0c(index_buffer, 0, 0, 0);  // 00BA5479
    // 00BA547B..00BA5501: serpentine strip, direction flips every row; 16-bit stores.
    std::uint32_t n = 0;
    std::int32_t dir = 1;
    for (std::uint32_t r = 1; r < height; ++r) {
        std::int32_t c = dir > 0 ? 0 : static_cast<std::int32_t>(width) - 1;
        if (c >= 0) {
            while (static_cast<std::uint32_t>(c) < width) {
                indices[n++] = static_cast<std::uint16_t>(
                    (static_cast<std::uint16_t>(r) - 1u) * width + static_cast<std::uint16_t>(c));  // 00BA54BC
                indices[n++] = static_cast<std::uint16_t>(
                    static_cast<std::uint16_t>(r) * width + static_cast<std::uint16_t>(c));  // 00BA54C8
                c += dir;
                if (c < 0) break;  // JNS
            }
        }
        indices[n++] = static_cast<std::uint16_t>((r - 1u) * width - dir + c);  // 00BA54EC
        dir = -dir;
    }
    host.index_buffer_unlock_10(index_buffer);  // 00BA550F
    host.geometry_set_index_stream_00b73b70(host.generated_model_get_geometry_00b74640(model, 0),
                                            index_buffer);  // 00BA551D, 00BA5524

    void* const block = host.draw_section_pool_allocate_00b85ee0(0x60);  // 00BA552E
    std::uint8_t* const section =
        block != nullptr ? host.draw_section_construct_00b857f0(block) : nullptr;  // 00BA5545
    // No null test before these stores (00BA5552..00BA556F).
    *reinterpret_cast<std::uint32_t*>(section + 0x08) = 5;
    *reinterpret_cast<std::uint32_t*>(section + 0x0C) = 0;
    *reinterpret_cast<std::uint32_t*>(section + 0x10) = word(0x100) * word(0xFC);
    *reinterpret_cast<std::uint32_t*>(section + 0x14) = 0;
    *reinterpret_cast<std::uint32_t*>(section + 0x18) = n - 2u;
    host.draw_section_set_material_00b864c0(section, word(0x12C));          // 00BA5583
    host.draw_section_append_vertex_stream_00b85b80(section, word(0x124));  // 00BA5591
    host.draw_section_append_vertex_stream_00b85b80(
        section,
        host.ocean_height_generator_current_stream_00ba7080(pointer(word(0x2BC))));  // 00BA559C, 00BA55A4
    host.draw_section_set_vertex_layout_00b86650(section, word(0x128));  // 00BA55B2
    host.geometry_append_draw_section_00b73c60(host.generated_model_get_geometry_00b74640(model, 0),
                                               section);  // 00BA55C0, 00BA55C7
    host.ref_release(section);       // 00BA55D2..00BA55E2
    host.ref_release(index_buffer);  // 00BA55E4..00BA55F5
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

namespace {
// The shared body of 00BAEEB0/00BAEFB0/00BAF450: floor the request at 20h (JGE), and only when
// it exceeds the capacity (JLE skips) allocate request*sizeof(T), copy the live elements dword
// by dword (each copy skipped when the destination address is null, as the image tests), free
// the old block (also when null) and store the new data and capacity. The size is unchanged.
template <class T>
void coast_vector_reserve(CoastHost& host, CoastNativeVector<T>& v, std::int32_t request) {
    if (request < 0x20) request = 0x20;
    if (v.capacity >= request) return;
    auto* const block = static_cast<T*>(
        host.operator_new_00bf55be(static_cast<std::uint32_t>(request) * sizeof(T)));
    T* dst = block;
    for (std::int32_t i = 0; i < v.size; ++i, ++dst) {
        if (dst != nullptr) std::memcpy(dst, v.data + i, sizeof(T));
    }
    host.free_00bf6989(v.data);
    v.data = block;
    v.capacity = request;
}

// The doubled request of the push helpers: capacity*2, raised to 20h unless above it (JG).
std::int32_t coast_vector_grow_request(std::int32_t capacity) {
    const std::int32_t doubled = capacity + capacity;
    return doubled > 0x20 ? doubled : 0x20;
}
} // namespace

void coast_ptr_vector_reserve_00baeeb0(CoastHost& host, CoastNativeVector<std::uint32_t>& v,
                                       std::int32_t request) {
    coast_vector_reserve(host, v, request);  // 00BAEED0 new(request*4), 00BAEEFC free
}
void coast_segment_vector_reserve_00baefb0(CoastHost& host, CoastNativeVector<CoastSegment>& v,
                                           std::int32_t request) {
    coast_vector_reserve(host, v, request);  // 00BAEFD6 new(request*1Ch), 00BAF035 free
}
void coast_point_vector_reserve_00baf450(CoastHost& host, CoastNativeVector<CoastPoint>& v,
                                         std::int32_t request) {
    coast_vector_reserve(host, v, request);  // 00BAF476 new(request*2Ch), 00BAF4F1 free
}
void coast_segment_vector_push_back_00baf350(CoastHost& host, CoastNativeVector<CoastSegment>& v,
                                             const CoastSegment& value) {
    if (v.size == v.capacity)                                                  // 00BAF36E
        coast_segment_vector_reserve_00baefb0(host, v, coast_vector_grow_request(v.capacity)); // 00BAF385
    CoastSegment* const slot = v.data + v.size;                                // 00BAF38A..00BAF398
    if (slot != nullptr) {                                                     // 00BAF39F
        slot->a = value.a;                // 00BAF3BB 00453ED0(slot, value, 0Ch, 2, 00414220)
        slot->b = value.b;
        slot->flags_18 = value.flags_18;  // 00BAF3C0
    }
    v.size += 1;                                                               // 00BAF3C7
}
void coast_ptr_vector_push_back_00baf3e0(CoastHost& host, CoastNativeVector<std::uint32_t>& v,
                                         const std::uint32_t& value) {
    if (v.size == v.capacity)                                                  // 00BAF3E6
        coast_ptr_vector_reserve_00baeeb0(host, v, coast_vector_grow_request(v.capacity));    // 00BAF3F8
    std::uint32_t* const slot = v.data + v.size;                               // 00BAF3FD..00BAF402
    if (slot != nullptr) *slot = value;                                        // 00BAF405..00BAF40F
    v.size += 1;                                                               // 00BAF411
}
void coast_point_vector_push_back_00bafc00(CoastHost& host, CoastNativeVector<CoastPoint>& v,
                                           const CoastPoint& value) {
    if (v.size == v.capacity)                                                  // 00BAFC1E
        coast_point_vector_reserve_00baf450(host, v, coast_vector_grow_request(v.capacity));  // 00BAFC35
    CoastPoint* const slot = v.data + v.size;                                  // 00BAFC3A..00BAFC40
    if (slot != nullptr) {                                                     // 00BAFC4E
        slot->a = value.a;                // 00BAFC60 00453ED0(slot, value, 0Ch, 2, 00414220)
        slot->b = value.b;
        slot->normal = value.normal;      // 00BAFC65..00BAFC74 FLD/FSTP
        slot->u0_24 = value.u0_24;        // 00BAFC77 dword moves
        slot->u1_28 = value.u1_28;
    }
    v.size += 1;                                                               // 00BAFC84
}
void coast_ptr_vector_resize_00baf070(std::vector<std::uint32_t>& v, std::int32_t size) {
    v.resize(static_cast<std::size_t>(size < 0 ? 0 : size), 0u);  // new slots zeroed
}
void pak_archive_registry_slots_push_back_00bb4b40(void* vector, const std::uint32_t* value) {
    auto* const words = static_cast<std::uint32_t*>(vector);
    std::int32_t capacity = static_cast<std::int32_t>(words[2]);
    if (static_cast<std::int32_t>(words[1]) == capacity) {          // 00BB4B46
        capacity += capacity;                                        // 00BB4B4B
        if (capacity <= 1) capacity = 1;                             // 00BB4B4D, JG
        reserve_native_pak_registry_slots_00bb46f0(vector, capacity);  // 00BB4B58
    }
    auto* const slot = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(words[0])) + words[1];
    if (slot != nullptr) *slot = *value;                             // 00BB4B65..00BB4B6F
    words[1] += 1;                                                   // 00BB4B71
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

namespace {
template<class T> T sky_construct_read_c(const void* object, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(object) + offset, sizeof value);
    return value;
}
template<class T> void sky_construct_write_c(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<unsigned char*>(object) + offset, &value, sizeof value);
}
float sky_unsigned_dimension_c(std::uint32_t value) {
    const auto signed_value = static_cast<std::int32_t>(value);
    const float unsigned_bias = 4294967296.0f; // CE3978 = 4F800000
    float result;
    __asm {
        fild signed_value
        cmp signed_value, 0
        jge sky_dimension_nonnegative
        fadd unsigned_bias
    sky_dimension_nonnegative:
        fstp result
    }
    return result;
}
void sky_texel_offsets_c(void* layer_base) {
    const float width = sky_construct_read_c<float>(layer_base, 0x74);
    const float height = sky_construct_read_c<float>(layer_base, 0x78);
    float inverse_width, inverse_height, zero_width, zero_height;
    // BBFCC9..BBFD22. DC C9 at BBFCFB multiplies ST1 by ST0, leaving
    // ST0=zero for the following multiply by inverse_height. Do not replace
    // these products with literal zero: dimensions0 produce Inf and NaN.
    __asm {
        fld height
        fld width
        fld1
        fld st(0)
        fdivrp st(2), st(0)
        fxch st(1)
        fstp inverse_width
        fdivrp st(1), st(0)
        fstp inverse_height
        fld inverse_width
        fldz
        fmul st(1), st(0)
        fxch st(1)
        fstp zero_width
        fmul inverse_height
        fstp zero_height
    }
    sky_construct_write_c(layer_base, 0x34, zero_width);
    sky_construct_write_c(layer_base, 0x3C, std::uint32_t{0});
    sky_construct_write_c(layer_base, 0x40, std::uint32_t{0});
    sky_construct_write_c(layer_base, 0x38, zero_height);
    sky_construct_write_c(layer_base, 0x44, inverse_width);
    sky_construct_write_c(layer_base, 0x48, zero_height);
    sky_construct_write_c(layer_base, 0x4C, std::uint32_t{0});
    sky_construct_write_c(layer_base, 0x50, std::uint32_t{0});
    sky_construct_write_c(layer_base, 0x54, zero_width);
    sky_construct_write_c(layer_base, 0x58, inverse_height);
    sky_construct_write_c(layer_base, 0x5C, std::uint32_t{0});
    sky_construct_write_c(layer_base, 0x60, std::uint32_t{0});
    sky_construct_write_c(layer_base, 0x64, inverse_width);
    sky_construct_write_c(layer_base, 0x68, inverse_height);
    sky_construct_write_c(layer_base, 0x6C, std::uint32_t{0});
    sky_construct_write_c(layer_base, 0x70, std::uint32_t{0});
}
}

// 00BBF8A0..00BC02BF. ECX destination, source/atmosphere/name on stack,
// RET0Ch, EAX destination. Complete normal raw-storage and host-call sequence;
// native pool/STL allocation, FH3 unwinding, unmasked FP/hardware faults and
// original binary ABI are outside this source interface. Producer writes and
// all register-sensitive node/material and x87 sites were checked in assembly.
void* sky_box_construct_00bbf8a0(void* destination, void* source, void* atmosphere,
    std::string_view name, SkyBoxConstructHostC& host) {
    static_assert(sizeof(void*) == 4);
    auto* const bytes = static_cast<unsigned char*>(destination);
    sky_construct_write_c(destination, 0, std::uint32_t{0x00CEB130});
    sky_construct_write_c(destination, 4, std::uint32_t{1});
    sky_construct_write_c(destination, 0, std::uint32_t{0x00D64618});
    sky_construct_write_c(destination, 8, static_cast<void*>(nullptr));
    sky_construct_write_c(destination, 0xC, atmosphere);
    sky_construct_write_c(destination, 0x10, static_cast<void*>(nullptr));
    for (std::size_t offset = 0x18; offset <= 0x30; offset += 4)
        sky_construct_write_c(destination, offset, std::uint32_t{0});
    // 00439750 is MOV EAX,ECX; RET, so the 4x10h array callbacks write nothing.
    for (std::size_t layer = 0; layer != 3; ++layer)
        sky_construct_write_c(destination, 0x84 + layer * 0x54, static_cast<void*>(nullptr));
    if (source) {
        sky_construct_write_c(destination, 8, source);
        _InterlockedIncrement(reinterpret_cast<volatile long*>(static_cast<unsigned char*>(source) + 4));
    }
    void* const manager = host.call_004c1400();
    void* const resource = host.call_00b80d70(manager, "sky\\skybox.mmod");
    void* const instance = host.resource_vslot_08(resource, 6, 1.0f);
    sky_construct_write_c(destination, 0x10, instance);
    if (_InterlockedDecrement(reinterpret_cast<volatile long*>(static_cast<unsigned char*>(resource) + 4)) == 0)
        host.refcounted_vslot_00(resource);
    void* const node = sky_construct_read_c<void*>(instance, 0xC);
    sky_construct_write_c(destination, 0x14, node);
    void* const captured_34 = sky_construct_read_c<void*>(node, 0x34);
    host.call_00b6d890(node, sky_construct_read_c<void*>(destination, 8));
    const char* const suffixes[]{"_A.tga", "_B.tga", "_C.tga"};
    for (std::size_t layer = 0; layer != 3; ++layer) {
        void* const texture_manager = host.global_pointer_00f8d394();
        const std::string texture_name = std::string(name) + suffixes[layer];
        void* const texture = host.texture_manager_vslot_64(texture_manager, texture_name, 0);
        const auto base = layer * 0x54;
        sky_construct_write_c(destination, base + 0x84, texture);
        const auto width = host.texture_vslot_3c(texture);
        sky_construct_write_c(destination, base + 0x74, sky_unsigned_dimension_c(width));
        const auto height = host.texture_vslot_40(sky_construct_read_c<void*>(destination, base + 0x84));
        sky_construct_write_c(destination, base + 0x78, sky_unsigned_dimension_c(height));
        sky_texel_offsets_c(bytes + base);
    }
    void* const current_node = sky_construct_read_c<void*>(destination, 0x14);
    sky_construct_write_c(current_node, 0x138, sky_construct_read_c<std::uint32_t>(current_node, 0x138) & ~3u);
    sky_construct_write_c(captured_34, 0x138, sky_construct_read_c<std::uint32_t>(captured_34, 0x138) & ~3u);
    void* const nested_34 = sky_construct_read_c<void*>(current_node, 0x34);
    void* const nested_3c = sky_construct_read_c<void*>(nested_34, 0x3C);
    sky_construct_write_c(nested_3c, 0x138, sky_construct_read_c<std::uint32_t>(nested_3c, 0x138) & ~3u);
    host.call_00b72b50(host.call_00b74640(nested_3c, 0));
    for (std::uint32_t geometry_index = 0; geometry_index < host.call_00b74650(captured_34); ++geometry_index) {
        std::array<float, 4> bounds{};
        const std::uint32_t radius_bits = 0x501502F9u; // CE4970, copied as raw float bits
        std::memcpy(&bounds[3], &radius_bits, sizeof radius_bits);
        host.call_00b72650(host.call_00b74640(captured_34, geometry_index), bounds);
        void* const geometry = host.call_00b74640(captured_34, geometry_index);
        for (std::uint32_t element_index = 0; element_index < host.call_00b72b40(geometry); ++element_index) {
            void* element = host.call_00b732c0(geometry, element_index);
            void* material = sky_construct_read_c<void*>(element, 0x20);
            void* const first_texture = sky_construct_read_c<std::int16_t>(material, 0x34) > 0
                ? sky_construct_read_c<void*>(material, 0x10) : nullptr;
            const void* const name_header = host.call_00b33e40(first_texture);
            const auto length = sky_construct_read_c<std::uint32_t>(name_header, 0);
            const char* const data = sky_construct_read_c<const char*>(name_header, 4);
            const std::string element_name = length ? std::string(data, length) : std::string();
            std::size_t layer = 0;
            if (!std::strstr(element_name.c_str(), "skyboxdummy_a")) {
                if (std::strstr(element_name.c_str(), "skyboxdummy_b")) layer = 1;
                else if (std::strstr(element_name.c_str(), "skyboxdummy_c")) layer = 2;
            }
            element = host.call_00b732c0(geometry, element_index);
            material = sky_construct_read_c<void*>(element, 0x20);
            host.call_00b18b20(material, "cLuminance", reinterpret_cast<float*>(bytes + 0x30));
            const char* const offset_names[]{"cOffset0", "cOffset1", "cOffset2", "cOffset3", "cTextureSize"};
            for (std::size_t parameter = 0; parameter != 5; ++parameter) {
                element = host.call_00b732c0(geometry, element_index);
                material = sky_construct_read_c<void*>(element, 0x20);
                host.call_00b18ac0(material, offset_names[parameter],
                    reinterpret_cast<float*>(bytes + layer * 0x54 + 0x34 + parameter * 0x10), 1);
            }
            element = host.call_00b732c0(geometry, element_index);
            host.call_00b189f0(sky_construct_read_c<void*>(element, 0x20), 0,
                sky_construct_read_c<void*>(destination, layer * 0x54 + 0x84));
        }
    }
    return destination;
}

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

void waterdrops_construct_fields(WaterdropsC& w) noexcept {
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
    waterdrops_construct_fields(object);  // the field stores of 00BC2030; then vtable 00D647D0
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

void* waterdrops_construct_00bc2030(void* object, NativeStringStorage& strings,
                                    W3_ConstructGeneratedModel construct_00b75030) {
    w3_construct_named_model(object, "waterdrops", strings, construct_00b75030, 0x00D64760u);
    w3_field<std::uint32_t>(object, 0x184) = 0;
    w3_field<std::uint32_t>(object, 0x188) = 0;
    w3_field<std::uint32_t>(object, 0x18C) = 0;
    w3_field<float>(object, 0x194) = 0.01f;   // 00D7A238
    w3_field<float>(object, 0x198) = 0.025f;  // 00D1FBBC
    w3_field<float>(object, 0x19C) = 0.5f;    // 00CE3800
    w3_field<float>(object, 0x1A0) = 4.0f;    // 00CE3D34
    w3_field<std::uint32_t>(object, 0x1C8) = 0;
    w3_field<std::uint8_t>(object, 0x1D5) = 0;
    w3_field<std::uint8_t>(object, 0x1D6) = 0;
    w3_field<std::uint8_t>(object, 0x1D7) = 0;
    w3_field<float>(object, 0x1C4) = 0.95f;   // 00CED9E0
    w3_field<std::uint8_t>(object, 0x1D4) = 1;
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

namespace {

enum class W3_Kind : std::uint8_t {
    ref, node, node_keep, node_always, free_clear, free_only, string, destroy_via, clear_vector, owner_null, parent_null
};
struct W3_Step {
    W3_Kind kind;
    std::uint16_t offset;
    W2_Resize resize = nullptr;    // destroy_via: null selects calls.bound_resize
    W2_Reserve reserve = nullptr;  // clear_vector: null selects calls.bound_reserve
};

void w3_resize_ptr4(W2_Array& a, std::int32_t n) { w2_pod_resize(a, n, 4, W2_Copy::dwords); }
void w3_resize_pair8(W2_Array& a, std::int32_t n) { w2_pod_resize(a, n, 8, W2_Copy::dwords); }

template<std::size_t N>
void w3_run_steps(void* object, const W3_Step (&steps)[N], const W3_ReleaseCalls& calls) {
    for (const W3_Step& step : steps) {
        void*& field = w3_field<void*>(object, step.offset);
        switch (step.kind) {
        case W3_Kind::ref: w2_ref_release(&field); break;
        case W3_Kind::node:
            if (field != nullptr) {
                calls.node_unlink_and_release_00b6dfa0(field);
                field = nullptr;
            }
            break;
        case W3_Kind::node_keep:
            if (field != nullptr) calls.node_unlink_and_release_00b6dfa0(field);
            break;
        case W3_Kind::node_always: calls.node_unlink_and_release_00b6dfa0(field); break;
        case W3_Kind::free_clear:
            if (field != nullptr) {
                singleton_lifetime_free(field);
                field = nullptr;
            }
            break;
        case W3_Kind::free_only:
            if (field != nullptr) singleton_lifetime_free(field);
            break;
        case W3_Kind::string: destroy_native_string_header_0041dd20(&field, *calls.strings); break;
        case W3_Kind::destroy_via:
            w2_destroy_via(w3_field<W2_Array>(object, step.offset), step.resize != nullptr ? step.resize : calls.bound_resize);
            break;
        case W3_Kind::clear_vector:
            w2_clear_counting_down(w3_field<W2_Array>(object, step.offset),
                                   step.reserve != nullptr ? step.reserve : calls.bound_reserve, true);
            break;
        case W3_Kind::owner_null: calls.material_set_parameter_owner_00b18a40(field, nullptr, nullptr); break;
        case W3_Kind::parent_null: calls.node_set_parent_00b6e680(field, nullptr); break;
        }
    }
}

using K = W3_Kind;

const W3_Step kWaveFieldBase_00ba24b0[] = {
    {K::ref, 0x610}, {K::ref, 0xBC}, {K::node, 0x600}, {K::node, 0x504}, {K::node, 0x4A0}, {K::ref, 0x4E8},
    {K::ref, 0x4F0}, {K::ref, 0x604}, {K::ref, 0x608}, {K::ref, 0x60C}, {K::node, 0x49C}, {K::ref, 0xF0},
    {K::node, 0x11C}, {K::node, 0x120}, {K::ref, 0xC0}, {K::owner_null, 0x12C}, {K::owner_null, 0x130},
    {K::ref, 0x12C}, {K::ref, 0x130}, {K::ref, 0x124}, {K::ref, 0x128}, {K::ref, 0x274}, {K::ref, 0x278},
    {K::node, 0xC8}, {K::node, 0xD0}, {K::ref, 0x4E4}, {K::ref, 0x2AC}, {K::ref, 0x2BC}, {K::ref, 0x288},
    {K::ref, 0x498}, {K::ref, 0xE4}, {K::ref, 0xE8}, {K::ref, 0x14},
    {K::destroy_via, 0x620, &w3_coverage_region_vector_resize_00b9efa0}, {K::destroy_via, 0x614, &w3_resize_pair8},
    {K::destroy_via, 0x5F4}, {K::destroy_via, 0x4F8, &w3_resize_ptr4},
    {K::destroy_via, 0x2B0, &w3_grid_vector_resize_00b9ef20}, {K::destroy_via, 0x27C, &w3_resize_ptr4},
    {K::string, 0xA8}, {K::string, 0xA0},
};
const W3_Step kBeams_00ba2e60[] = {
    {K::ref, 0x1C}, {K::ref, 0x18}, {K::free_clear, 0xE8}, {K::free_clear, 0xE4}, {K::node_keep, 0x14},
    {K::ref, 0x28}, {K::string, 0x34}, {K::ref, 0x24}, {K::ref, 0x20}, {K::ref, 0x10}, {K::ref, 0x0C},
};
const W3_Step kWaterParticles_00ba42a0[] = {
    {K::node_keep, 0x10}, {K::ref, 0x1C}, {K::string, 0x28}, {K::ref, 0x18}, {K::ref, 0x14}, {K::ref, 0x0C},
};
const W3_Step kHeightGenerator_00ba71c0[] = {
    {K::ref, 0x40}, {K::ref, 0x5C}, {K::ref, 0x2C}, {K::ref, 0x18}, {K::node, 0x20}, {K::ref, 0x1C},
    {K::node, 0x24}, {K::ref, 0x58}, {K::node, 0x50}, {K::ref, 0x4C}, {K::node, 0x54}, {K::ref, 0x28},
    {K::ref, 0x3C}, {K::ref, 0x64}, {K::ref, 0x60}, {K::node, 0x38}, {K::ref, 0x30}, {K::node, 0x34},
    {K::ref, 0x78}, {K::ref, 0x7C}, {K::ref, 0x70}, {K::ref, 0x74}, {K::ref, 0x48}, {K::ref, 0x44},
    {K::ref, 0x14}, {K::ref, 0x10}, {K::node, 0x0C}, {K::ref, 0x08}, {K::ref, 0x68}, {K::ref, 0x6C},
};
const W3_Step kTracerResource_00bac8a0[] = {
    {K::ref, 0x14}, {K::ref, 0x0C}, {K::destroy_via, 0x24}, {K::destroy_via, 0x18},
};
const W3_Step kSprayResource_00bae170[] = {{K::ref, 0x0C}, {K::ref, 0x08}};
const W3_Step kSprayObject_00bae210[] = {{K::parent_null, 0x08}, {K::node_always, 0x08}, {K::ref, 0x0C}};
const W3_Step kCoastline_00bb03d0[] = {
    {K::node, 0x28}, {K::ref, 0x20}, {K::ref, 0x24}, {K::string, 0xD4}, {K::string, 0xCC},
    {K::destroy_via, 0x14}, {K::clear_vector, 0x08},
};
const W3_Step kCoastMesh_00bb0c90[] = {
    {K::free_only, 0x44}, {K::free_only, 0x3C}, {K::string, 0x78}, {K::string, 0x70}, {K::string, 0x68},
    {K::ref, 0x20}, {K::ref, 0x1C}, {K::ref, 0x18}, {K::ref, 0x14}, {K::ref, 0x10}, {K::ref, 0x0C},
};

template<std::size_t N>
void w3_release_body(void* object, std::uint32_t vtable, const W3_Step (&steps)[N], const W3_ReleaseCalls& calls) {
    w3_field<std::uint32_t>(object, 0) = vtable;
    w3_run_steps(object, steps, calls);
    calls.base_destructor(object);
}

}  // namespace

void w3_ocean_wave_field_base_destruct_00ba24b0(void* field, const W3_ReleaseCalls& calls) {
    w3_field<std::uint32_t>(field, 0) = 0x00D63B5Cu;
    w3_field<std::uint32_t>(field, 4) = 0x00D63B54u;
    w3_run_steps(field, kWaveFieldBase_00ba24b0, calls);
    calls.base_destructor(field);
}
void w3_ocean_beams_destruct_00ba2e60(void* beams, const W3_ReleaseCalls& calls) {
    w3_release_body(beams, 0x00D63B88u, kBeams_00ba2e60, calls);
}
void w3_ocean_water_particles_destruct_00ba42a0(void* particles, const W3_ReleaseCalls& calls) {
    w3_release_body(particles, 0x00D63BB4u, kWaterParticles_00ba42a0, calls);
}
void w3_ocean_height_generator_destruct_00ba71c0(void* generator, const W3_ReleaseCalls& calls) {
    w3_release_body(generator, 0x00D63E34u, kHeightGenerator_00ba71c0, calls);
}
void w3_water_tracer_resource_destruct_00bac8a0(void* resource, const W3_ReleaseCalls& calls) {
    w3_release_body(resource, 0x00D63F94u, kTracerResource_00bac8a0, calls);
}
void w3_water_spray_resource_destruct_00bae170(void* resource, const W3_ReleaseCalls& calls) {
    w3_release_body(resource, 0x00D64094u, kSprayResource_00bae170, calls);
}
void w3_water_spray_object_destruct_00bae210(void* object, const W3_ReleaseCalls& calls) {
    w3_release_body(object, 0x00D6409Cu, kSprayObject_00bae210, calls);
}
void w3_coastline_destruct_00bb03d0(void* coastline, const W3_ReleaseCalls& calls) {
    w3_release_body(coastline, 0x00D640DCu, kCoastline_00bb03d0, calls);
}
void w3_coast_mesh_destruct_00bb0c90(void* mesh, const W3_ReleaseCalls& calls) {
    w3_release_body(mesh, 0x00D640E4u, kCoastMesh_00bb0c90, calls);
}

void w3_msar_provider_destroy_00bbb660(void* provider, const W3_ReleaseCalls& calls,
    void (*destroy_range_00bbb020)(void* first, void* last, void* vector)) {
    w3_field<std::uint32_t>(provider, 0) = 0x00D643C4u;
    w2_ref_release(&w3_field<void*>(provider, 0x14));
    void* const begin = w3_field<void*>(provider, 0x1C);
    if (begin != nullptr) {
        destroy_range_00bbb020(begin, w3_field<void*>(provider, 0x20), w3_bytes(provider) + 0x18);
        singleton_lifetime_free(w3_field<void*>(provider, 0x1C));
    }
    w3_field<void*>(provider, 0x1C) = nullptr;
    w3_field<void*>(provider, 0x20) = nullptr;
    w3_field<void*>(provider, 0x24) = nullptr;
    calls.base_destructor(provider);
}

namespace {

void w3_store_added_ref(void* object, std::size_t offset, void* const* source) {
    void* const value = *source;
    w3_field<void*>(object, offset) = value;
    if (value != nullptr) _InterlockedIncrement(reinterpret_cast<volatile long*>(w3_bytes(value) + 4));
}

void w3_apply_default_settings(void* object, std::size_t float_offset, const W3_SettingsCalls& calls) {
    alignas(4) unsigned char record[0xA0];  // inside the native 0A4h frame
    calls.construct_default_layer(record);
    calls.assign_settings(object, record);
    w3_field<float>(object, float_offset) = 0.0f;
    destroy_native_string_header_0041dd20(record + 8, *calls.strings);
}

}  // namespace

void* w3_ocean_beams_construct_00ba3a60(void* beams, void* const* a, void* const* b, const W3_SettingsCalls& calls) {
    calls.weak_owner_construct_00925490(beams);
    w3_field<std::uint32_t>(beams, 0) = 0x00D63B88u;
    w3_store_added_ref(beams, 0x0C, a);
    w3_store_added_ref(beams, 0x10, b);
    for (std::size_t offset : {0x20u, 0x24u, 0x28u, 0x34u, 0x38u, 0xE4u, 0xE8u, 0x14u}) w3_field<void*>(beams, offset) = nullptr;
    w2_ref_release(&w3_field<void*>(beams, 0x24));
    w3_field<void*>(beams, 0x24) = nullptr;
    w2_ref_release(&w3_field<void*>(beams, 0x20));
    w3_field<void*>(beams, 0x20) = nullptr;
    w3_apply_default_settings(beams, 0xF0, calls);
    return beams;
}

void* w3_ocean_water_particles_construct_00ba43b0(void* particles, void* const* a, const W3_SettingsCalls& calls) {
    calls.weak_owner_construct_00925490(particles);
    w3_field<std::uint32_t>(particles, 0) = 0x00D63BB4u;
    w3_store_added_ref(particles, 0x0C, a);
    for (std::size_t offset : {0x14u, 0x18u, 0x1Cu, 0x28u, 0x2Cu, 0x10u}) w3_field<void*>(particles, offset) = nullptr;
    w2_ref_release(&w3_field<void*>(particles, 0x18));
    w3_field<void*>(particles, 0x18) = nullptr;
    w2_ref_release(&w3_field<void*>(particles, 0x14));
    w3_field<void*>(particles, 0x14) = nullptr;
    w3_apply_default_settings(particles, 0xC0, calls);
    return particles;
}

void* w3_msar_provider_construct_00bbb5b0(void* provider, void* name, void* file_system_0109ceec,
    void (*file_provider_base_construct)(void* provider, void* name)) {
    file_provider_base_construct(provider, name);
    w3_field<std::uint32_t>(provider, 0) = 0x00D643C4u;
    w3_field<void*>(provider, 0x1C) = nullptr;
    w3_field<void*>(provider, 0x20) = nullptr;
    w3_field<void*>(provider, 0x24) = nullptr;
    using Open = void*(__thiscall*)(void*, void*, std::uint32_t);
    w3_field<void*>(provider, 0x14) = reinterpret_cast<Open>(w3_field<void*>(w3_field<void*>(file_system_0109ceec, 0), 4))(
        file_system_0109ceec, w3_bytes(provider) + 8, 2);
    return provider;
}

void* w3_ocean_wave_field_create_00ba6fb0(void* world, std::uint32_t flat,
    void* (*construct_00ba6c40)(void* field, void* world, std::uint32_t flat)) {
    void* const raw = w3_new(0x6BC);
    return raw != nullptr ? construct_00ba6c40(raw, world, flat) : nullptr;
}

void* w3_ocean_height_generator_create_00bb3c60(void* a, void* b, void* c, void* d,
    void* (*construct_00bb3ab0)(void* generator, void* a, void* b, void* c, void* d)) {
    void* const raw = w3_new(0x118);
    return raw != nullptr ? construct_00bb3ab0(raw, a, b, c, d) : nullptr;
}

void* w3_application_create_pooled2(void* (*pool_allocate)(), void* a, void* b,
    void* (*construct)(void* slot, void* a, void* b)) {
    void* const slot = pool_allocate();
    return slot != nullptr ? construct(slot, a, b) : nullptr;
}

void* w3_application_create_cloud_plane_model_00bc0350(void* a, void* b, void* c, void* d, void* (*pool_allocate_00bc6050)(),
    void* (*construct_00bc6060)(void* slot, void* a, void* b, void* c, void* d)) {
    void* const slot = pool_allocate_00bc6050();
    return slot != nullptr ? construct_00bc6060(slot, a, b, c, d) : nullptr;
}

void* w3_application_create_cloud_puff_00bc03c0(void* a, void* b, float c, float d,
    void* (*construct_00bc6430)(void* object, void* a, void* b, float c, float d)) {
    void* const raw = w3_new(0x38);
    if (raw == nullptr) return nullptr;
    const float stored_c = static_cast<float>(static_cast<double>(c));
    const float stored_d = static_cast<float>(static_cast<double>(d));
    return construct_00bc6430(raw, a, b, stored_c, stored_d);
}

namespace {

template<class R, class... A>
R w3_vcall(void* object, std::size_t slot, A... args) {
    using Fn = R(__thiscall*)(void*, A...);
    return reinterpret_cast<Fn>(w3_field<void*>(w3_field<void*>(object, 0), slot))(object, args...);
}

struct W3_Size {
    std::uint32_t width;
    std::uint32_t height;
};

W3_Size w3_back_buffer_pair(void* device) {
    W3_Size first{}, second{};
    const W3_Size* const a = w3_vcall<const W3_Size*>(device, 0x80, &first);
    const W3_Size* const b = w3_vcall<const W3_Size*>(device, 0x80, &second);
    return {a->width, b->height};
}

}  // namespace

void w3_ocean_wave_field_create_reflection_targets_00b9dc20(void* field, std::uint32_t format,
                                                            const std::uint32_t* size, void* device_00f8d394) {
    w2_ref_release(&w3_field<void*>(field, 0xE4));
    w2_ref_release(&w3_field<void*>(field, 0xE8));
    w2_ref_release(&w3_field<void*>(field, 0x288));
    const std::uint32_t depth_format = format == 0x17 ? 0x50u : 0x4Bu;
    w3_field<void*>(field, 0xE4) = w3_vcall<void*>(device_00f8d394, 0x88, size[0], size[1], 1u, format, 0x10u);
    const W3_Size colour = w3_back_buffer_pair(device_00f8d394);
    w3_field<void*>(field, 0xE8) = w3_vcall<void*>(device_00f8d394, 0x88, colour.width, colour.height, 1u, 0x71u, 0x10u);
    const W3_Size depth = w3_back_buffer_pair(device_00f8d394);
    w3_field<void*>(field, 0x288) = w3_vcall<void*>(device_00f8d394, 0x94, depth.width, depth.height, depth_format, 0u, 0.0f, 1u);
}

namespace {

// A pooled temporary built the way these bodies inline it: zero header, resize preserving, copy the
// length + 1 bytes of the literal; released with the length + 1 the header holds at release time.
class W3_Temporary {
public:
    W3_Temporary(const char* text, NativeStringStorage& strings) : strings_(strings) {
        resize_native_string_header_0041dd40(&header_, strings, static_cast<std::uint32_t>(std::strlen(text)), true);
        if (header_.data != nullptr) std::memcpy(header_.data, text, header_.length + 1u);
    }
    ~W3_Temporary() {
        if (header_.data != nullptr) strings_.release(header_.data, header_.length + 1u);
    }
    W3_Temporary(const W3_Temporary&) = delete;
    W3_Temporary& operator=(const W3_Temporary&) = delete;
    const void* get() const noexcept { return &header_; }

private:
    W3_Header header_{0, nullptr};
    NativeStringStorage& strings_;
};

void* w3_geometry(void* model) { return w3_field<void*>(model, 0x180); }                               // 00B74640
void* w3_element(void* geometry, std::uint32_t i) { return w3_field<void**>(geometry, 0x54)[i]; }     // 00B732C0
void* w3_vertex_stream(void* geometry, std::uint32_t i) { return w3_field<void*>(geometry, 0x64 + 4u * i); }  // 00B73260

}  // namespace

void* w3_ocean_wave_field_create_wave_damp_object_00b9ea50(void* field, const float* a, const float* b,
    const void* texture, void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls) {
    void* model;
    {
        W3_Temporary shader("oceanwavedamp.mshd", strings);
        W3_Temporary geometry("pf43uf42.mvfm", strings);
        W3_Temporary name("WaveDampObject", strings);
        model = calls.create_generated_model_00b4c700(name.get(), geometry.get(), shader.get(), 5, 4, 2, 0);
    }  // released in reverse construction order: name, geometry, shader (listing 00B9EB31..00B9EB9F)
    calls.node_propagate_root_00b6d890(model, w3_field<void*>(w3_field<void*>(field, 0x2BC), 8));
    void* const stream = w3_vertex_stream(w3_geometry(model), 0);
    auto* const v = w3_vcall<float*>(stream, 0x10, 0u, 0u, 0u);
    const float one = 1.0f;  // 00D7A24C
    const float zero = 0.0f;
    const float values[20] = {
        a[0], zero, a[1], zero, one,
        a[0], zero, b[1], zero, zero,
        b[0], zero, a[1], one,  one,
        b[0], zero, b[1], one,  zero,
    };
    std::memcpy(v, values, sizeof values);
    w3_vcall<void>(stream, 0x14);
    void* const loaded = w3_vcall<void*>(device_00f8d394, 0x64, texture, 0u);
    void* const element = w3_element(w3_geometry(model), 0);
    calls.material_set_texture_slot_00b189f0(w3_field<void*>(element, 0x20), 0, loaded);
    if (loaded != nullptr) {
        void* slot = loaded;
        w2_ref_release(&slot);
    }
    return model;
}

void w3_ocean_wave_field_bind_height_stream_00ba4eb0(void* field, W3_EngineCalls& calls) {
    void* const generator = w3_field<void*>(field, 0x2BC);
    if (w3_field<std::uint8_t>(field, 0x10) != 0) {
        calls.material_set_texture_slot_00b189f0(w3_field<void*>(field, 0x12C), 8, w3_field<void*>(generator, 0x44));
        calls.material_set_texture_slot_00b189f0(w3_field<void*>(field, 0x130), 8, w3_field<void*>(generator, 0x44));
        void* const material = w3_field<void*>(w3_element(w3_geometry(w3_field<void*>(field, 0x4A0)), 0), 0x20);
        calls.material_set_texture_slot_00b189f0(material, 0, w3_field<void*>(w3_field<void*>(field, 0x2BC), 0x44));
        return;
    }
    w3_vcall<void>(generator, 8);
    for (std::uint32_t i = 0; i < w3_field<std::uint32_t>(w3_geometry(w3_field<void*>(field, 0x11C)), 0x58); ++i) {
        void* const element = w3_element(w3_geometry(w3_field<void*>(field, 0x11C)), i);
        void* const height = w3_field<void*>(field, 0x2BC);
        void* const stream = w3_field<void*>(height, 0x78 + 4u * w3_field<std::uint32_t>(height, 0x84));
        calls.draw_section_set_vertex_stream_00b86500(element, 1, stream);
    }
}

void w3_ocean_height_generator_create_streams_00bb3db0(void* generator, std::int32_t width, std::int32_t height,
                                                      void* device_00f8d394, NativeStringStorage& strings) {
    w2_ref_release(&w3_field<void*>(generator, 0x78));
    w2_ref_release(&w3_field<void*>(generator, 0x7C));
    void* declaration;
    {
        W3_Temporary file("oceanheightmap.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    const std::int32_t count = width * height;
    w3_field<void*>(generator, 0x78) = w3_vcall<void*>(device_00f8d394, 0x5C, count, 1u, declaration);
    w3_field<void*>(generator, 0x7C) = w3_vcall<void*>(device_00f8d394, 0x5C, count, 1u, declaration);
    w2_release_object(declaration);  // no null test in the listing (00BB3EB3)
}

void* w3_ocean_height_generator_construct_00ba94f0(void* generator, void* owner, std::int32_t width,
    std::int32_t height, const std::uint32_t* params, const volatile std::int32_t& scale_00e14454,
    void* device_00f8d394, NativeStringStorage& strings,
    void (*initialize_passes_00ba7530)(void* generator, std::int32_t width, std::int32_t height)) {
    w3_field<std::uint32_t>(generator, 0) = 0x00CEB130u;
    w3_field<std::uint32_t>(generator, 4) = 1;
    w3_field<std::uint32_t>(generator, 0x88) = 0;
    w3_field<std::uint32_t>(generator, 0) = 0x00D63E34u;
    for (std::size_t offset : {0x08u, 0x0Cu, 0x10u, 0x14u, 0x18u, 0x1Cu, 0x2Cu, 0x30u, 0x34u, 0x38u, 0x40u, 0x44u,
                               0x48u, 0x5Cu, 0x60u, 0x64u}) {
        w3_field<std::uint32_t>(generator, offset) = 0;
    }
    for (std::size_t i = 0; i < 4; ++i) w3_field<std::uint32_t>(generator, 0x8C + 4 * i) = params[i];
    w3_field<void*>(generator, 0x88) = owner;
    const std::int32_t scaled_height = scale_00e14454 * height;
    const std::int32_t scaled_width = scale_00e14454 * width;
    for (std::size_t offset : {0x6Cu, 0x68u, 0x7Cu, 0x78u, 0x74u, 0x70u}) w3_field<std::uint32_t>(generator, offset) = 0;
    void* const device = device_00f8d394;
    w3_field<void*>(generator, 0x68) = w3_vcall<void*>(device, 0x88, scaled_width, scaled_height, 1u, 0x71u, 0x10u);
    w3_field<void*>(generator, 0x6C) = w3_vcall<void*>(device, 0x88, scale_00e14454 * width, scale_00e14454 * height, 1u, 0x15u, 0x10u);
    w3_field<void*>(generator, 0x44) = w3_vcall<void*>(device, 0x88, width, height, 1u, 0x71u, 0x10u);
    w3_field<void*>(generator, 0x48) = w3_vcall<void*>(w3_field<void*>(generator, 0x44), 0x30, 0u, 0u);
    w3_field<void*>(generator, 0x60) = w3_vcall<void*>(device, 0x88, width, height, 1u, 0x71u, 2u);
    w3_field<void*>(generator, 0x64) = w3_vcall<void*>(w3_field<void*>(generator, 0x60), 0x30, 0u, 0u);
    w3_field<void*>(generator, 0x70) = w3_vcall<void*>(w3_field<void*>(generator, 0x68), 0x30, 0u, 0u);
    w3_field<void*>(generator, 0x74) = w3_vcall<void*>(w3_field<void*>(generator, 0x6C), 0x30, 0u, 0u);
    w3_field<void*>(generator, 0x10) = w3_vcall<void*>(device, 0x88, 0x80, 0x80, 1u, 0x15u, 0x10u);
    w3_field<void*>(generator, 0x14) = w3_vcall<void*>(w3_field<void*>(generator, 0x10), 0x30, 0u, 0u);
    w3_ocean_height_generator_create_streams_00bb3db0(generator, width, height, device, strings);
    initialize_passes_00ba7530(generator, width, height);
    return generator;
}

void* w3_water_spray_object_construct_00bae900(void* object, void* parent, const void* resource,
                                               NativeStringStorage& strings, W3_EngineCalls& calls) {
    auto* const res = const_cast<void*>(resource);
    w3_field<std::uint32_t>(object, 0) = 0x00CEB130u;
    w3_field<std::uint32_t>(object, 4) = 1;
    w3_field<std::uint32_t>(object, 0) = 0x00D6409Cu;
    w3_field<void*>(object, 8) = nullptr;
    w3_field<void*>(object, 0x0C) = nullptr;
    void* const material_slot = calls.material_allocate_raw_slot_00b18780(0x110);
    void* const material = material_slot != nullptr
        ? calls.material_construct_with_effect_00b18900(material_slot, w3_field<void*>(res, 0x0C)) : nullptr;
    w3_field<void*>(object, 0x0C) = material;
    calls.material_set_parameter_owner_00b18a40(material, object, nullptr);
    calls.material_set_texture_slot_00b189f0(w3_field<void*>(object, 0x0C), 0, w3_field<void*>(res, 0x10));
    void* const mesh_slot = calls.mesh_allocate_raw_slot_00b73b60(0xBC);
    void* const mesh = mesh_slot != nullptr ? calls.mesh_construct_00b73d70(mesh_slot) : nullptr;
    calls.mesh_set_vertex_stream_00b73bb0(mesh, 0, w3_vertex_stream(w3_field<void*>(res, 8), 0));
    calls.mesh_set_index_stream_00b73b70(mesh, w3_field<void*>(w3_field<void*>(res, 8), 0x60));
    void* const section = calls.draw_section_create_00533fa0();
    w3_field<std::uint32_t>(section, 0x0C) = 0;
    w3_field<std::uint32_t>(section, 0x08) = 4;
    w3_field<std::uint32_t>(section, 0x14) = 0;
    w3_field<std::uint32_t>(section, 0x10) = w3_field<std::uint32_t>(res, 0x14);
    w3_field<std::uint32_t>(section, 0x18) = w3_field<std::uint32_t>(res, 0x18);
    calls.draw_section_rebuild_layout_00b865a0(section, mesh);
    calls.draw_section_set_material_00b864c0(section, w3_field<void*>(object, 0x0C));
    calls.mesh_append_draw_section_00b73c60(mesh, section);
    w2_release_object(section);
    void* const model_slot = calls.model_allocate_raw_slot_00b74eb0(0x184);
    void* model = nullptr;
    if (model_slot != nullptr) {
        W3_Temporary name("WaterSpray", strings);
        model = calls.generated_model_construct_00b75030(model_slot, name.get());
        w3_field<void*>(object, 8) = model;
    } else {
        w3_field<void*>(object, 8) = nullptr;
    }
    const float minus_one = -1.0f;  // 00D7A260
    calls.generated_model_set_geometry_00b75170(w3_field<void*>(object, 8), 0, mesh, minus_one, minus_one);
    calls.node_set_parent_00b6e680(w3_field<void*>(object, 8), parent);
    calls.generated_model_set_local_bounds_00b74390(w3_field<void*>(object, 8),
                                                    reinterpret_cast<const float*>(w3_bytes(res) + 0x1C));
    if (mesh != nullptr) w2_release_object(mesh);
    w3_field<float>(object, 0x10) = 0.0f;
    w3_field<float>(object, 0x14) = 0.0f;
    return object;
}

namespace {

void w3_assign_literal_no_preserve(void* header, const char* text, NativeStringStorage& strings) {
    resize_native_string_header_0041dd40(header, strings, static_cast<std::uint32_t>(std::strlen(text)), false);
    char* const data = w3_field<char*>(header, 4);
    if (data != nullptr) std::memcpy(data, text, w3_field<std::uint32_t>(header, 0));
}

}  // namespace

namespace {

void w3_attach_surface(void* targets_holder, std::size_t targets_offset, void* surface, void* depth, W3_EngineCalls& calls) {
    calls.frame_targets_set_color_00b1fab0(w3_field<void*>(targets_holder, targets_offset), 0, surface);
    calls.frame_targets_set_depth_00b1fb00(w3_field<void*>(targets_holder, targets_offset), depth);
    w2_release_object(surface);
}

}  // namespace

void* w3_ocean_wave_field_construct_00ba6c40(void* field, void* world, std::uint8_t flat,
    const volatile std::uint8_t& hdr_00f8d398, void* device_00f8d394, NativeStringStorage& strings,
    W3_EngineCalls& calls, void* (*base_construct_00ba1400)(void* field, void* world)) {
    base_construct_00ba1400(field, world);
    w3_field<std::uint8_t>(field, 0xF9) = flat;
    w3_field<std::uint32_t>(field, 0) = 0x00D63E10u;
    w3_field<std::uint32_t>(field, 4) = 0x00D63E08u;
    const std::uint32_t format = hdr_00f8d398 != 0 ? 0x71u : 0x15u;  // NEG/SBB/AND 5Ch/ADD 15h
    std::uint32_t size_out[2]{};
    const std::uint32_t* const size = calls.wave_field_reflection_target_size_00b9a3b0(field, size_out);
    w3_ocean_wave_field_create_reflection_targets_00b9dc20(field, format, size, device_00f8d394);
    calls.wave_field_initialize_render_resources_00ba5610(field);
    w3_field<void*>(field, 0x498) = w3_vcall<void*>(device_00f8d394, 0x88, 0x400u, 0x400u, 1u, 0x15u, 0x10u);
    void* const camera_slot = calls.camera_allocate_raw_slot_00b71930(0x458);
    if (camera_slot != nullptr) {
        W3_Temporary name("OceanShadowRenderCamera", strings);
        w3_field<void*>(field, 0x49C) = calls.camera_construct_00b71a80(camera_slot, name.get());
    } else {
        w3_field<void*>(field, 0x49C) = nullptr;
    }
    calls.camera_set_clear_flags_00b6fe10(w3_field<void*>(field, 0x49C), 7);
    calls.camera_set_clear_color_00b6fe50(w3_field<void*>(field, 0x49C), 0);
    void* const viewport_block = w3_new(0x34);
    void* const viewport = viewport_block != nullptr ? calls.viewport_construct_00b1f850(viewport_block) : nullptr;
    calls.camera_set_viewport_00b71990(w3_field<void*>(field, 0x49C), viewport);
    const std::uint32_t texture_height = w3_vcall<std::uint32_t>(w3_field<void*>(field, 0x498), 0x40);
    const std::uint32_t texture_width = w3_vcall<std::uint32_t>(w3_field<void*>(field, 0x498), 0x3C);
    const std::uint32_t dimensions[2] = {texture_width, texture_height};
    calls.viewport_set_dimensions_00b1f940(viewport, dimensions);
    w2_release_object(viewport);
    void* const texture = w3_field<void*>(field, 0x498);
    const std::uint32_t depth_height = w3_vcall<std::uint32_t>(texture, 0x40);
    const std::uint32_t depth_width = w3_vcall<std::uint32_t>(texture, 0x3C);
    void* const depth = w3_vcall<void*>(device_00f8d394, 0x94, depth_width, depth_height, 0x4Bu, 0u, 0.0f, 1u);
    void* const targets_block = w3_new(0x40);
    w3_field<void*>(field, 0x60C) = targets_block != nullptr ? calls.frame_targets_construct_00b1fbb0(targets_block) : nullptr;
    w3_attach_surface(field, 0x60C, w3_vcall<void*>(w3_field<void*>(field, 0x498), 0x30, 0u, 0u), depth, calls);
    if (depth != nullptr) w2_release_object(depth);
    w3_attach_surface(field, 0x604, w3_vcall<void*>(w3_field<void*>(field, 0xE4), 0x30, 0u, 0u),
                      w3_field<void*>(field, 0x288), calls);
    w3_attach_surface(field, 0x608, w3_vcall<void*>(w3_field<void*>(field, 0xE8), 0x30, 0u, 0u),
                      w3_field<void*>(field, 0x288), calls);
    return field;
}

void w3_waterdrops_build_geometry_00bc2d50(void* waterdrops, void* particle_resources_00f8c280,
    void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls) {
    void* const mesh_slot = calls.mesh_allocate_raw_slot_00b73b60(0xBC);
    void* const mesh = mesh_slot != nullptr ? calls.mesh_construct_00b73d70(mesh_slot) : nullptr;
    void* declaration;
    {
        W3_Temporary file("simplecolor.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    const std::uint32_t vertices = w3_field<std::uint32_t>(waterdrops, 0x190) * 4u;
    void* const buffer = w3_vcall<void*>(device_00f8d394, 0x5C, vertices, 0x1000u, declaration);
    w2_release_object(declaration);
    calls.mesh_set_vertex_stream_00b73bb0(mesh, 0, buffer);
    w2_release_object(buffer);
    void* const section = calls.draw_section_create_00533fa0();
    w3_field<std::uint32_t>(section, 0x08) = 4;
    for (std::size_t offset : {0x0Cu, 0x10u, 0x14u, 0x18u}) w3_field<std::uint32_t>(section, offset) = 0;
    void* material;
    {
        W3_Temporary effect("waterdrops.mshd", strings);
        material = calls.material_create_for_effect_name_00535320(effect.get());
    }
    calls.material_set_texture_slot_00b189f0(material, 0, w3_field<void*>(waterdrops, 0x1C8));
    calls.draw_section_set_material_00b864c0(section, material);
    w2_release_object(material);
    calls.draw_section_rebuild_layout_00b865a0(section, mesh);
    calls.mesh_append_draw_section_00b73c60(mesh, section);
    w2_release_object(section);
    calls.mesh_set_index_stream_00b73b70(mesh, calls.particle_index_stream_00af10a0(particle_resources_00f8c280));
    const float minus_one = -1.0f;  // 00D7A260
    calls.generated_model_set_geometry_00b75170(waterdrops, 0, mesh, minus_one, minus_one);
    float bounds[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const std::uint32_t far_bits = 0x501502F9u;  // 00CE4970, 1.0e10f
    std::memcpy(&bounds[3], &far_bits, 4);
    calls.mesh_set_bounds_00b72650(mesh, bounds);
    calls.generated_model_set_local_bounds_00b74390(waterdrops, bounds);
    w2_release_object(mesh);
}

void* w3_cloud_render_model_construct_00bc54f0(void* model, void* parent, void* cloud_class,
    std::uint32_t* image_01090904, void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls,
    W3_Body cloud_instance_construct_00bbe7d0, void* (*material_clone_construct_00b18b60)(void* slot, void* source)) {
    cloud_instance_construct_00bbe7d0(model);
    w3_field<void*>(model, 0x184) = cloud_class;
    w3_field<std::uint32_t>(model, 0) = 0x00D64878u;
    const auto global = [image_01090904](std::uint32_t address) -> std::uint32_t& {
        return image_01090904[(address - 0x01090904u) / 4u];
    };
    for (std::uint32_t address : {0x01090970u, 0x01090974u, 0x0109097Cu, 0x01090978u, 0x01090980u, 0x01090988u,
                                  0x01090984u, 0x0109098Cu, 0x01090994u, 0x01090990u, 0x01090998u, 0x010909A0u,
                                  0x0109099Cu, 0x010909A4u}) {
        global(address) = 0x3F800000u;  // 00D7A24C 1.0f
    }
    global(0x01090908u) = 0x3F000000u;  // 00CE3800 0.5f
    global(0x010909ACu) = 0x3F800000u;
    global(0x010909A8u) = 0x3F800000u;
    global(0x01090904u) = 0x40400000u;  // 00CE3854 3.0f
    w3_field<float>(model, 0x1C8) = 1.0f;
    w3_field<float>(model, 0x1D8) = 1.0f;
    calls.node_set_parent_00b6e680(model, parent);
    void* const mesh_slot = calls.mesh_allocate_raw_slot_00b73b60(0xBC);
    void* const mesh = mesh_slot != nullptr ? calls.mesh_construct_00b73d70(mesh_slot) : nullptr;
    void* declaration;
    {
        W3_Temporary file("cloud.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    const std::uint32_t puffs = w3_field<std::uint32_t>(cloud_class, 0x20);  // 00BC3100
    w3_field<void*>(model, 0x1A8) = w3_vcall<void*>(device_00f8d394, 0x5C, puffs * 4u, 1u, declaration);
    w2_release_object(declaration);
    calls.mesh_set_vertex_stream_00b73bb0(mesh, 0, w3_field<void*>(model, 0x1A8));
    w2_release_object(w3_field<void*>(model, 0x1A8));
    void* const indices = w3_vcall<void*>(device_00f8d394, 0x60, w3_field<std::uint32_t>(cloud_class, 0x20) * 6u, 1u, 0x65u);
    calls.mesh_set_index_stream_00b73b70(mesh, indices);
    w2_release_object(indices);
    void* const section = calls.draw_section_create_00533fa0();
    w3_field<std::uint32_t>(section, 0x08) = 4;
    w3_field<std::uint32_t>(section, 0x0C) = 0;
    w3_field<std::uint32_t>(section, 0x10) = w3_field<std::uint32_t>(cloud_class, 0x20) * 4u;
    w3_field<std::uint32_t>(section, 0x14) = 0;
    w3_field<std::uint32_t>(section, 0x18) = w3_field<std::uint32_t>(cloud_class, 0x20) * 2u;
    void* const material_slot = calls.material_allocate_raw_slot_00b18780(0x110);
    void* const material = material_slot != nullptr
        ? material_clone_construct_00b18b60(material_slot, w3_field<void*>(cloud_class, 0x68)) : nullptr;  // 00BC3080
    calls.draw_section_set_material_00b864c0(section, material);
    w2_release_object(material);
    calls.draw_section_rebuild_layout_00b865a0(section, mesh);
    calls.mesh_append_draw_section_00b73c60(mesh, section);
    w2_release_object(section);
    const float minus_one = -1.0f;  // 00D7A260
    calls.generated_model_set_geometry_00b75170(model, 0, mesh, minus_one, minus_one);
    float bounds[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const std::uint32_t radius_bits = w2_x87_reload_bits(w3_field<std::uint32_t>(cloud_class, 0x2C));  // 00BC3010 FLD, FSTP
    std::memcpy(&bounds[3], &radius_bits, 4);
    calls.mesh_set_bounds_00b72650(mesh, bounds);
    calls.generated_model_set_local_bounds_00b74390(model, bounds);
    w2_release_object(mesh);
    w3_field<std::uint8_t>(model, 0x188) = 0;
    for (std::size_t offset : {0x198u, 0x19Cu, 0x1A0u}) w3_field<float>(model, offset) = 0.0f;
    for (std::size_t offset : {0x18Cu, 0x190u, 0x194u}) w3_field<std::uint32_t>(model, offset) = 0x47C34F80u;  // 99999.0f
    w3_field<std::uint8_t>(model, 0x1A4) = 0;
    w3_field<std::uint8_t>(model, 0x1B4) = 0;
    return model;
}

void* w3_cloud_plane_model_construct_00bc6060(void* model, void* owner, void* parent, const void* effect,
    const void* texture, void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls,
    W3_ConstructGeneratedModel construct_00b75030) {
    w3_cloud_plane_model_construct_base_00bc7db0(model, strings, construct_00b75030);
    w3_field<std::uint32_t>(model, 0) = 0x00D648E0u;
    w3_field<float>(model, 0x184) = 0.0f;
    w3_field<float>(model, 0x188) = 0.0f;
    calls.node_set_parent_00b6e680(model, parent);
    calls.scene_node_set_visibility_factor_00b6da70(model, 0.0f, 0);
    void* const material = calls.material_create_for_effect_name_00535320(effect);
    void* const loaded = w3_vcall<void*>(device_00f8d394, 0x64, texture, 0u);
    calls.material_set_texture_slot_00b189f0(material, 0, loaded);
    w2_release_object(loaded);
    calls.material_set_parameter_owner_00b18a40(material, owner, nullptr);
    void* const mesh_slot = calls.mesh_allocate_raw_slot_00b73b60(0xBC);
    void* const mesh = mesh_slot != nullptr ? calls.mesh_construct_00b73d70(mesh_slot) : nullptr;
    const float minus_one = -1.0f;  // 00D7A260
    calls.generated_model_set_geometry_00b75170(model, 0, mesh, minus_one, minus_one);
    float bounds[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const std::uint32_t far_bits = 0x501502F9u;  // 00CE4970, 1.0e10f
    std::memcpy(&bounds[3], &far_bits, 4);
    calls.mesh_set_bounds_00b72650(mesh, bounds);
    calls.generated_model_set_local_bounds_00b74390(model, bounds);
    void* declaration;
    {
        W3_Temporary file("cloud.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    void* const vertices = w3_vcall<void*>(device_00f8d394, 0x5C, 4u, 1u, declaration);
    w2_release_object(declaration);
    void* const section = calls.draw_section_create_00533fa0();
    w3_field<std::uint32_t>(section, 0x0C) = 0;
    w3_field<std::uint32_t>(section, 0x14) = 0;
    w3_field<std::uint32_t>(section, 0x08) = 4;
    w3_field<std::uint32_t>(section, 0x10) = 4;
    w3_field<std::uint32_t>(section, 0x18) = 2;
    calls.draw_section_set_material_00b864c0(section, material);
    calls.draw_section_append_vertex_stream_00b85b80(section, vertices);
    calls.draw_section_rebuild_layout_00b865a0(section, mesh);
    calls.mesh_append_draw_section_00b73c60(mesh, section);
    w2_release_object(section);
    w2_release_object(material);
    auto* const v = w3_vcall<std::uint32_t*>(vertices, 0x10, 0u, 0u, 0u);
    constexpr std::uint32_t one = 0x3F800000u, minus = 0xBF800000u, ten = 0x41200000u, minus_ten = 0xC1200000u;
    const std::uint32_t quad[32] = {
        minus, one,   0, 0xFFFFFFFFu, minus_ten, ten,       0,   0,
        one,   one,   0, 0xFFFFFFFFu, ten,       ten,       one, 0,
        one,   minus, 0, 0xFFFFFFFFu, ten,       minus_ten, one, one,
        minus, minus, 0, 0xFFFFFFFFu, minus_ten, minus_ten, 0,   one,
    };
    std::memcpy(v, quad, sizeof quad);
    w3_vcall<void>(vertices, 0x14);
    w2_release_object(vertices);
    void* const index_buffer = w3_vcall<void*>(device_00f8d394, 0x60, 6u, 1u, 0x65u);
    calls.mesh_set_index_stream_00b73b70(mesh, index_buffer);
    auto* const indices = w3_vcall<std::uint16_t*>(index_buffer, 0x0C, 0u, 0u, 0u);
    const std::uint16_t order[6] = {0, 1, 2, 2, 3, 0};
    std::memcpy(indices, order, sizeof order);
    w3_vcall<void>(index_buffer, 0x10);
    w2_release_object(index_buffer);
    w2_release_object(mesh);
    return model;
}

void* w3_coast_mesh_construct_00bb0e30(void* mesh, void* const* a, void* const* b, void* const* c,
                                       NativeStringStorage& strings, W3_Body weak_owner_construct_00925490) {
    weak_owner_construct_00925490(mesh);
    w3_field<std::uint32_t>(mesh, 0) = 0x00D640E4u;
    w3_store_added_ref(mesh, 0x0C, a);
    w3_store_added_ref(mesh, 0x10, b);
    w3_store_added_ref(mesh, 0x14, c);
    for (std::size_t offset : {0x18u, 0x1Cu, 0x20u, 0x68u, 0x6Cu, 0x70u, 0x74u, 0x78u, 0x7Cu}) w3_field<void*>(mesh, offset) = nullptr;
    for (std::size_t offset : {0x20u, 0x1Cu, 0x18u}) {
        w2_ref_release(&w3_field<void*>(mesh, offset));
        w3_field<void*>(mesh, offset) = nullptr;
    }
    // {offset, DWORD}: the .rdata source and value of each store, in listing order.
    const std::pair<std::size_t, std::uint32_t> first[] = {
        {0x48, 0x40A00000u},  // 00CE3850 5.0
        {0x58, 0x41000000u}, {0x5C, 0x41000000u},  // 00CE3918 8.0
        {0x64, 0x3CF5C28Fu},  // 00CE3DB8 0.03
        {0x60, 0x3F800000u},  // 00D7A24C 1.0
        {0x80, 0x3F000000u}, {0x84, 0x3F000000u}, {0x88, 0x3F000000u}, {0x90, 0x3F000000u},  // 00CE3800 0.5
        {0x8C, 0x3F400000u},  // 00CEE07C 0.75
        {0x94, 0x3F000000u}, {0x98, 0x3F000000u},
        {0x44, 0}, {0x3C, 0}, {0x40, 0}, {0x4C, 4}, {0x50, 0x32}, {0x54, 0},
        {0x9C, 0x3F400000u},
    };
    for (const auto& [offset, value] : first) w3_field<std::uint32_t>(mesh, offset) = value;
    w3_assign_literal_no_preserve(w3_bytes(mesh) + 0x70, "foamwave.dds", strings);
    w3_assign_literal_no_preserve(w3_bytes(mesh) + 0x78, "foamcoast.dds", strings);
    w3_assign_literal_no_preserve(w3_bytes(mesh) + 0x68, "coast1.dat", strings);
    const std::pair<std::size_t, std::uint32_t> second[] = {
        {0xA0, 0x3F19999Au}, {0xC0, 0x3F19999Au},  // 00CE3D30 0.6
        {0xC4, 0x3DCCCCCDu}, {0xC8, 0x3DCCCCCDu},  // 00D7A2F0 0.1
        {0xA4, 0x3C6703B0u},  // 00D640F8 0.0141
        {0xCC, 0xBF4CCCCDu},  // 00D05E14 -0.8
        {0xA8, 0x43040000u},  // 00D640F0 132.0
        {0xD0, 0x3E4CCCCDu},  // 00CE54A0 0.2
        {0xAC, 0x3C23D70Au},  // 00D7A238 0.01
        {0xB0, 0x3C4985F0u},  // 00D640F4 0.0123
        {0xB8, 0x3C23D70Au},
        {0xD4, 0x3ECCCCCDu},  // 00CE7804 0.4
        {0xB4, 0x42E20000u},  // 00D640EC 113.0
        {0xBC, 0x3E99999Au},  // 00CE69C8 0.3
        {0xD8, 0}, {0x120, 0},
    };
    for (const auto& [offset, value] : second) w3_field<std::uint32_t>(mesh, offset) = value;
    return mesh;
}

void* w3_ocean_height_generator_construct_derived_00bb3ab0(void* generator, void* owner, std::int32_t width,
    std::int32_t height, const std::uint32_t* params, void* device_00f8d394, W3_EngineCalls& calls,
    void* (*construct_00ba94f0)(void* generator, void* owner, std::int32_t width, std::int32_t height,
                                const std::uint32_t* params)) {
    construct_00ba94f0(generator, owner, width, height, params);
    w3_field<std::uint32_t>(generator, 0) = 0x00D64168u;
    void* const colour = w3_field<void*>(generator, 0x70);
    const std::uint32_t surface_height = w3_vcall<std::uint32_t>(colour, 0x20);
    const std::uint32_t surface_width = w3_vcall<std::uint32_t>(colour, 0x1C);
    void* const depth = w3_vcall<void*>(device_00f8d394, 0x94, surface_width, surface_height, 0x4Bu, 0u, 0.0f, 1u);
    const std::pair<std::size_t, std::size_t> targets[] = {{0x40, 0x70}, {0x5C, 0x48}, {0x2C, 0x74}};
    for (const auto& [slot, colour_offset] : targets) {
        void* const block = w3_new(0x40);
        void* const built = block != nullptr ? calls.frame_targets_construct_00b1fbb0(block) : nullptr;
        w3_field<void*>(generator, slot) = built;
        calls.frame_targets_set_color_00b1fab0(built, 0, w3_field<void*>(generator, colour_offset));
        calls.frame_targets_set_depth_00b1fb00(w3_field<void*>(generator, slot), depth);
    }
    if (depth != nullptr) {
        void* held = depth;
        w2_ref_release(&held);
    }
    return generator;
}

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part g (cc13-left2, docs/CC13_LEFT_2.md)
// ================================================================================================
namespace bsp::water_weather_cc13 {
namespace {

struct W4_Header {
    std::uint32_t length;
    char* data;
};

void w4_release(void* header, NativeStringStorage& strings) {
    auto& h = *static_cast<W4_Header*>(header);
    if (h.data != nullptr) strings.release(h.data, h.length + 1u);
}

// 0041E870 on a raw header: zero, strlen, resize preserving, copy length + 1 into the current data.
void w4_assign_cstring(void* header, const char* text, NativeStringStorage& strings) {
    auto& h = *static_cast<W4_Header*>(header);
    h = {0, nullptr};
    resize_native_string_header_0041dd40(&h, strings, static_cast<std::uint32_t>(std::strlen(text)), true);
    if (h.data != nullptr) std::memcpy(h.data, text, h.length + 1u);
}

// Append a header's bytes to out when its length is nonzero (resize preserving, copy length bytes).
void w4_append_header(void* out, const W4_Header& source, NativeStringStorage& strings) {
    const std::uint32_t length = source.length;
    if (length == 0) return;
    auto& o = *static_cast<W4_Header*>(out);
    const std::uint32_t previous = o.length;
    resize_native_string_header_0041dd40(&o, strings, previous + length, true);
    std::memcpy(o.data + previous, source.data, length);
}

// The inlined literal append of 00BD23E0: a temporary {0, 0} resized to the literal length (preserving),
// the literal copied with its terminator, appended when nonempty, released.
void w4_append_literal(void* out, const char* text, NativeStringStorage& strings) {
    W4_Header temporary{0, nullptr};
    resize_native_string_header_0041dd40(&temporary, strings, static_cast<std::uint32_t>(std::strlen(text)), true);
    if (temporary.data != nullptr) std::memcpy(temporary.data, text, temporary.length + 1u);
    w4_append_header(out, temporary, strings);
    w4_release(&temporary, strings);
}

void w4_invalid(const SingletonLifetimeCallbacks& callbacks) { callbacks.invalid_parameter(callbacks.context); }

void* w4_head(const W4_AssertionRecord* record) noexcept { return record->tree_head; }
void* w4_tree(const W4_AssertionRecord* record) noexcept {
    return const_cast<std::uint32_t*>(&record->tree_allocator);
}
const W4_Header& w4_node_key(void* node) noexcept { return w3_field<W4_Header>(node, 0x0C); }
const W4_Header& w4_node_value(void* node) noexcept { return w3_field<W4_Header>(node, 0x14); }

// 004263B0 on raw headers: out = copy of prefix (the 00BD2390 shape), then the "%d" text of value
// (004260B0, a 52-byte buffer) appended through a temporary that is released.
void* w4_concat_int_004263b0(const void* prefix, void* out, std::int32_t value, NativeStringStorage& strings) {
    w3_native_string_copy_construct_into_00bd2390(prefix, out, strings);
    char buffer[52];
    std::snprintf(buffer, sizeof buffer, "%d", value);
    w4_append_literal(out, buffer, strings);
    return out;
}

}  // namespace

void* w4_string_tree_allocate_node_00485f50() { return allocate_native_tree_node_00bdaba0(); }

void w4_string_tree_iterator_increment_00480620(void* iterator, const SingletonLifetimeCallbacks& callbacks) {
    advance_native_physical_index_00bd9860(iterator, callbacks);
}

void w4_string_tree_erase_subtree_00494ef0(void* tree, void* root, NativeStringStorage& strings) noexcept {
    destroy_native_physical_index_subtree_00bdf7a0(tree, root, strings);
}

void* w4_string_tree_erase_iterator_00494c30(void* tree, void* output, void* owner, void* node,
                                             NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    return erase_native_physical_index_iterator_00bdfd80(tree, output, owner, node, strings, callbacks);
}

void* w4_string_tree_erase_range_0049a230(void* tree, void* output, void* first_owner, void* first_node,
                                          void* last_owner, void* last_node, NativeStringStorage& strings,
                                          const SingletonLifetimeCallbacks& callbacks) {
    return erase_native_physical_index_range_00be0c30(tree, output, first_owner, first_node, last_owner, last_node,
                                                      strings, callbacks);
}

W4_AssertionRecord* w4_assertion_record_construct_00bd29c0(W4_AssertionRecord* record, const char* expression,
                                                          NativeStringStorage& strings) {
    record->file_length = 0;
    record->file_data = nullptr;
    w4_assign_cstring(&record->expression_length, expression, strings);
    void* const head = w4_string_tree_allocate_node_00485f50();
    record->tree_head = head;
    w3_field<std::uint8_t>(head, 0x1D) = 1;
    w3_field<void*>(record->tree_head, 4) = record->tree_head;
    w3_field<void*>(record->tree_head, 0) = record->tree_head;
    w3_field<void*>(record->tree_head, 8) = record->tree_head;
    record->tree_size = 0;
    record->message_length = 0;
    record->message_data = nullptr;
    record->level = 1;
    return record;
}

void w4_assertion_record_destroy_00bd2a50(W4_AssertionRecord* record, NativeStringStorage& strings,
                                          const SingletonLifetimeCallbacks& callbacks) {
    w4_release(&record->message_length, strings);
    void* output[2]{};
    void* const tree = w4_tree(record);
    w4_string_tree_erase_range_0049a230(tree, output, tree, w3_field<void*>(record->tree_head, 0), tree,
                                        record->tree_head, strings, callbacks);
    singleton_lifetime_free(record->tree_head);
    record->tree_head = nullptr;
    record->tree_size = 0;
    w4_release(&record->expression_length, strings);
    w4_release(&record->file_length, strings);
}

void* w4_assertion_record_format_context_00bd23e0(const W4_AssertionRecord* record, void* out,
                                                  NativeStringStorage& strings,
                                                  const SingletonLifetimeCallbacks& callbacks) {
    auto& o = *static_cast<W4_Header*>(out);
    o = {0, nullptr};
    void* const tree = w4_tree(record);
    void* iterator[2]{tree, w3_field<void*>(w4_head(record), 0)};
    for (;;) {
        if (iterator[0] == nullptr || iterator[0] != tree) w4_invalid(callbacks);
        if (iterator[1] == w4_head(record)) break;
        w4_append_literal(out, "<", strings);
        if (iterator[0] == nullptr) w4_invalid(callbacks);
        if (iterator[1] == w3_field<void*>(iterator[0], 4)) w4_invalid(callbacks);
        w4_append_header(out, w4_node_key(iterator[1]), strings);
        w4_append_literal(out, "=", strings);
        if (iterator[1] == w3_field<void*>(iterator[0], 4)) w4_invalid(callbacks);
        w4_append_header(out, w4_node_value(iterator[1]), strings);
        w4_append_literal(out, "> ", strings);
        w4_string_tree_iterator_increment_00480620(iterator, callbacks);
    }
    w4_append_header(out, *reinterpret_cast<const W4_Header*>(&record->message_length), strings);
    return out;
}

void* w4_assertion_record_format_report_00bd2760(const W4_AssertionRecord* record, void* out,
                                                 NativeStringStorage& strings,
                                                 const SingletonLifetimeCallbacks& callbacks,
                                                 const char* null_format_01090ab4) {
    alignas(4) unsigned char builder[0x18];
    construct_native_log_builder_004264a0(builder, strings);
    static constexpr const char* kLevels[] = {"WARNING", "DEBUG", "ERROR", "FATAL"};
    if (record->level >= 0 && record->level <= 3) {
        append_native_log_cstring_00bd1a60(builder, kLevels[record->level], strings);
    }
    void* b = append_native_log_cstring_00bd1a60(builder, " level assertion failed\n", strings);
    b = append_native_log_cstring_00bd1a60(builder, "Expression: ", strings);
    b = append_native_log_header_00bd1a20(b, &record->expression_length, strings);
    append_native_log_cstring_00bd1a60(b, "\n", strings);
    b = append_native_log_cstring_00bd1a60(builder, "File: ", strings);
    b = append_native_log_header_00bd1a20(b, &record->file_length, strings);
    b = append_native_log_cstring_00bd1a60(b, " (", strings);
    b = append_native_log_int_00bd1bb0(b, record->line, strings, null_format_01090ab4);
    append_native_log_cstring_00bd1a60(b, ")\n", strings);
    append_native_log_header_00bd1a20(builder, &record->message_length, strings);
    void* const tree = w4_tree(record);
    void* iterator[2]{tree, w3_field<void*>(w4_head(record), 0)};
    for (;;) {
        if (iterator[0] == nullptr || iterator[0] != tree) w4_invalid(callbacks);
        if (iterator[1] == w4_head(record)) break;
        if (iterator[0] == nullptr) w4_invalid(callbacks);
        if (iterator[1] == w3_field<void*>(iterator[0], 4)) w4_invalid(callbacks);
        if (iterator[1] == w3_field<void*>(iterator[0], 4)) w4_invalid(callbacks);
        b = append_native_log_cstring_00bd1a60(builder, "\t", strings);
        b = append_native_log_header_00bd1a20(b, &w4_node_key(iterator[1]), strings);
        b = append_native_log_cstring_00bd1a60(b, ": ", strings);
        b = append_native_log_header_00bd1a20(b, &w4_node_value(iterator[1]), strings);
        append_native_log_cstring_00bd1a60(b, "\n", strings);
        w4_string_tree_iterator_increment_00480620(iterator, callbacks);
    }
    w3_native_string_copy_construct_into_00bd2390(builder, out, strings);
    destroy_native_log_builder_00425f80(builder, strings);
    return out;
}

void w4_error_reporter_fail_with_location_00bd2b30(W4_AssertionRecord* record, const W4_FailCalls& calls,
                                                   NativeStringStorage& strings,
                                                   const SingletonLifetimeCallbacks& callbacks) {
    if (calls.handler_01090ab8 != nullptr) {
        W4_Header text{};
        w4_assertion_record_format_context_00bd23e0(record, &text, strings, callbacks);
        calls.handler_01090ab8(&text);
        w4_release(&text, strings);
    } else {
        W4_Header close{}, line_label{}, file_label{}, file{}, context{};
        W4_Header with_label{}, with_file{}, with_line_label{}, with_line{}, report{};
        w4_assign_cstring(&close, ")", strings);
        w4_assign_cstring(&line_label, "\", line: ", strings);
        w4_assign_cstring(&file_label, " (file: \"", strings);
        w3_native_string_copy_construct_into_00bd2390(record, &file, strings);
        w4_assertion_record_format_context_00bd23e0(record, &context, strings, callbacks);
        concatenate_native_string_headers_004261a0(&context, &with_label, &file_label, strings);
        concatenate_native_string_headers_004261a0(&with_label, &with_file, &file, strings);
        concatenate_native_string_headers_004261a0(&with_file, &with_line_label, &line_label, strings);
        w4_concat_int_004263b0(&with_line_label, &with_line, record->line, strings);
        concatenate_native_string_headers_004261a0(&with_line, &report, &close, strings);
        calls.reporter_fail_00bf1f50(calls.reporter_singleton_00bd2180(), &report);
        for (W4_Header* h : {&report, &with_line, &with_line_label, &with_file, &with_label, &context, &file,
                             &file_label, &line_label, &close}) {
            w4_release(h, strings);
        }
    }
    w4_assertion_record_destroy_00bd2a50(record, strings, callbacks);
}

}  // namespace bsp::water_weather_cc13

namespace bsp::water_weather_cc13 {
namespace {

// Release a reference slot when non-null and clear it (the "release and clear" step before a store).
void w4_release_clear(void*& slot) {
    if (slot != nullptr) {
        w2_release_object(slot);
        slot = nullptr;
    }
}

// operator new (00BF55BE) of count * size, saturated to FFFFFFFFh when the product overflows (SETO/NEG/OR).
void* w4_new_saturated(std::uint32_t count, std::uint32_t size) {
    const std::uint64_t bytes = static_cast<std::uint64_t>(count) * size;
    return w3_new(bytes > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::size_t>(bytes));
}

// An x87 FILD of a DWORD the source treats as unsigned: the signed value, plus 2^32 (00CE3978) when negative.
double w4_unsigned_to_double(std::uint32_t v) noexcept {
    double d = static_cast<double>(static_cast<std::int32_t>(v));
    if (static_cast<std::int32_t>(v) < 0) d += 4294967296.0;
    return d;
}

// Model allocate (184h) and construct with a pooled name, the name released only when constructed.
void* w4_make_model(const char* name, NativeStringStorage& strings, W3_EngineCalls& calls) {
    void* const slot = calls.model_allocate_raw_slot_00b74eb0(0x184);
    if (slot == nullptr) return nullptr;
    W3_Temporary temporary(name, strings);
    return calls.generated_model_construct_00b75030(slot, temporary.get());
}

void* w4_make_mesh(W3_EngineCalls& calls) {
    void* const slot = calls.mesh_allocate_raw_slot_00b73b60(0xBC);
    return slot != nullptr ? calls.mesh_construct_00b73d70(slot) : nullptr;
}

// {4n, 4n+1, 4n+2, 4n+1, 4n+3, 4n+2}: the quad index order of the beams and water particles.
void w4_fill_quads_strip(std::uint16_t* p, std::uint32_t n) {
    const std::uint16_t b = static_cast<std::uint16_t>(n * 4u);
    p[0] = b;
    p[1] = static_cast<std::uint16_t>(b + 1);
    p[2] = static_cast<std::uint16_t>(b + 2);
    p[3] = static_cast<std::uint16_t>(b + 1);
    p[4] = static_cast<std::uint16_t>(b + 3);
    p[5] = static_cast<std::uint16_t>(b + 2);
}

void* w4_root_of_owner(void* object) {
    void* const owner = w3_field<void*>(object, 0x0C);
    return owner != nullptr ? w3_field<void*>(owner, 8) : nullptr;
}

}  // namespace

void* w4_water_spray_resource_construct_00bae2e0(void* resource, const void* definition, void* device_00f8d394,
                                                 NativeStringStorage& strings, W4_EngineCalls& calls,
                                                 const W4_PresentationRandom& random) {
    w3_field<std::uint32_t>(resource, 0) = 0x00CEB130u;
    w3_field<std::uint32_t>(resource, 4) = 1;
    w3_field<std::uint32_t>(resource, 0) = 0x00D64094u;
    w3_field<void*>(resource, 0x08) = nullptr;
    w3_field<void*>(resource, 0x0C) = nullptr;
    w3_field<std::uint32_t>(resource, 0x14) = 0;
    w3_field<std::uint32_t>(resource, 0x18) = 0;
    const auto counts = [definition]() { return w3_field<const std::uint32_t*>(definition, 0x3C); };
    const auto elements = [definition]() { return w3_field<std::uint32_t>(definition, 0x40); };
    std::uint32_t total = 0;
    for (std::uint32_t i = 0; i < elements(); ++i) {
        w3_field<std::uint32_t>(resource, 0x14) += counts()[i] * 4u;
        w3_field<std::uint32_t>(resource, 0x18) += counts()[i] * 2u;
        total += counts()[i];
    }
    void* declaration;
    {
        W3_Temporary file("waterspray.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    void* const vertices = w3_vcall<void*>(device_00f8d394, 0x5C, w3_field<std::uint32_t>(resource, 0x14), 1u,
                                           declaration);
    w2_release_object(declaration);
    auto* out = w3_vcall<float*>(vertices, 0x10, 0u, 0u, 0u);
    for (std::uint32_t i = 0; i < elements(); ++i) {
        const float* const p = w3_field<const float*>(definition, 0x00) + i * 2u;
        const float* const q = w3_field<const float*>(definition, 0x0C) + i * 2u;
        const float px = p[0], py = p[1], qx = q[0], qy = q[1];
        const float a = w3_field<const float*>(definition, 0x24)[i];
        const float b = w3_field<const float*>(definition, 0x30)[i];
        const float c = w3_field<const float*>(definition, 0x18)[i];
        for (std::uint32_t j = 0; j < counts()[i]; ++j) {
            // 00BD2F10(stream 0, -2pi, 2pi) in the image; a presentation draw here.
            const float r = random.uniform_range_00bd2f10(random.context, -6.28318548f, 6.28318548f);
            const float t = static_cast<float>(w4_unsigned_to_double(j) / w4_unsigned_to_double(counts()[i]));
            const float minus_c = static_cast<float>(static_cast<double>(c) * -1.0);  // FMUL QWORD 00D7A250
            const float corners[4][2] = {{minus_c, c}, {minus_c, minus_c}, {c, minus_c}, {c, c}};
            for (const auto& corner : corners) {
                const float vertex[12] = {px, 0.0f, py, corner[0], corner[1], qx, qy, a, b, 1.0f, r, t};
                std::memcpy(out, vertex, sizeof vertex);
                out += 12;
            }
        }
    }
    w3_vcall<void>(vertices, 0x14);
    void* const indices = w3_vcall<void*>(device_00f8d394, 0x60, total * 6u, 1u, 0x65u);
    auto* index = w3_vcall<std::uint16_t*>(indices, 0x0C, 0u, 0u, 0u);
    for (std::uint32_t n = 0; n < total; ++n, index += 6) {
        const std::uint16_t base = static_cast<std::uint16_t>(n * 4u);
        index[1] = static_cast<std::uint16_t>(base + 1);
        index[0] = base;
        index[2] = static_cast<std::uint16_t>(base + 2);
        index[3] = static_cast<std::uint16_t>(base + 2);
        index[4] = static_cast<std::uint16_t>(base + 3);
        index[5] = base;
    }
    w3_vcall<void>(indices, 0x10);
    void* const mesh = w4_make_mesh(calls);
    w3_field<void*>(resource, 0x08) = mesh;
    calls.mesh_set_vertex_stream_00b73bb0(mesh, 0, vertices);
    w2_release_object(vertices);
    calls.mesh_set_index_stream_00b73b70(w3_field<void*>(resource, 0x08), indices);
    w2_release_object(indices);
    {
        W3_Temporary effect("waterspray.mshd", strings);
        w3_field<void*>(resource, 0x0C) = w3_vcall<void*>(device_00f8d394, 0x48, effect.get());
    }
    w3_field<float>(resource, 0x1C) = 0.0f;
    w3_field<float>(resource, 0x20) = 0.0f;
    w3_field<float>(resource, 0x24) = 0.0f;
    w3_field<float>(resource, 0x28) = 1.0e10f;  // 00CE4970
    {
        W3_Temporary texture("p_bubbles_0002.dds", strings);
        w3_field<void*>(resource, 0x10) = w3_vcall<void*>(device_00f8d394, 0x64, texture.get(), 0u);
    }
    return resource;
}

void w4_ocean_beams_initialize_00ba3bd0(void* beams, void* device_00f8d394, NativeStringStorage& strings,
                                        W4_EngineCalls& calls) {
    const std::uint32_t n = w3_field<std::uint32_t>(beams, 0x2C);
    if (n == 0) return;
    w3_field<std::uint32_t>(beams, 0xF8) = n * 6u;
    w3_field<std::uint32_t>(beams, 0xF4) = n * 4u;
    void* const texture = w3_vcall<void*>(device_00f8d394, 0x64, w3_bytes(beams) + 0x34, 0u);
    void* material;
    {
        W3_Temporary effect("beam.mshd", strings);
        material = calls.material_create_for_effect_name_00535320(effect.get());
    }
    calls.material_set_texture_slot_00b189f0(material, 0, texture);
    if (texture != nullptr) w2_release_object(texture);
    void* declaration;
    {
        W3_Temporary file("beam.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    W4_DeclarationList list{{declaration, nullptr, nullptr, nullptr}, 1};
    w3_field<void*>(beams, 0x28) = w3_vcall<void*>(device_00f8d394, 0x40, &list);
    void* const vertices = w3_vcall<void*>(device_00f8d394, 0x5C, w3_field<std::uint32_t>(beams, 0xF4), 1u,
                                           declaration);
    w4_release_clear(w3_field<void*>(beams, 0x24));
    w3_field<void*>(beams, 0x24) = vertices;
    w2_release_object(declaration);
    void* const indices = w3_vcall<void*>(device_00f8d394, 0x60, w3_field<std::uint32_t>(beams, 0xF8), 1u, 0x65u);
    w4_release_clear(w3_field<void*>(beams, 0x20));
    w3_field<void*>(beams, 0x20) = indices;
    w3_field<void*>(beams, 0x1C) = calls.draw_section_create_00533fa0();
    calls.draw_section_set_material_00b864c0(w3_field<void*>(beams, 0x1C), material);
    w2_release_object(material);
    w3_field<std::uint32_t>(w3_field<void*>(beams, 0x1C), 0x08) = 4;
    w3_field<std::uint32_t>(w3_field<void*>(beams, 0x1C), 0x0C) = 0;
    w3_field<std::uint32_t>(w3_field<void*>(beams, 0x1C), 0x10) = w3_field<std::uint32_t>(beams, 0xF4);
    w3_field<std::uint32_t>(w3_field<void*>(beams, 0x1C), 0x14) = 0;
    w3_field<std::uint32_t>(w3_field<void*>(beams, 0x1C), 0x18) = w3_field<std::uint32_t>(beams, 0x2C) * 2u;
    w3_field<void*>(beams, 0x14) = w4_make_model("UnderwaterBeam", strings, calls);
    calls.node_propagate_root_00b6d890(w3_field<void*>(beams, 0x14), w4_root_of_owner(beams));
    w3_field<std::uint32_t>(w3_field<void*>(beams, 0x14), 0x138) &= ~3u;
    auto* index = w3_vcall<std::uint16_t*>(w3_field<void*>(beams, 0x20), 0x0C, 0u, 0u, 0u);
    for (std::uint32_t i = 0; i < w3_field<std::uint32_t>(beams, 0x2C); ++i, index += 6) w4_fill_quads_strip(index, i);
    w3_vcall<void>(w3_field<void*>(beams, 0x20), 0x10);
    w3_field<void*>(beams, 0x18) = w4_make_mesh(calls);
    calls.mesh_set_vertex_stream_00b73bb0(w3_field<void*>(beams, 0x18), 0, w3_field<void*>(beams, 0x24));
    calls.mesh_set_index_stream_00b73b70(w3_field<void*>(beams, 0x18), w3_field<void*>(beams, 0x20));
    calls.draw_section_append_vertex_stream_00b85b80(w3_field<void*>(beams, 0x1C), w3_field<void*>(beams, 0x24));
    calls.draw_section_set_vertex_layout_00b86650(w3_field<void*>(beams, 0x1C), w3_field<void*>(beams, 0x28));
    calls.draw_section_rebuild_layout_00b865a0(w3_field<void*>(beams, 0x1C), w3_field<void*>(beams, 0x18));
    calls.mesh_append_draw_section_00b73c60(w3_field<void*>(beams, 0x18), w3_field<void*>(beams, 0x1C));
    calls.generated_model_set_geometry_00b75170(w3_field<void*>(beams, 0x14), 0, w3_field<void*>(beams, 0x18),
                                                -1.0f, -1.0f);  // 00D7A260
    const std::uint32_t count = w3_field<std::uint32_t>(beams, 0x2C);
    w3_field<std::uint32_t>(beams, 0xE0) = count;
    w3_field<std::uint32_t>(beams, 0xDC) = 0;
    w3_field<void*>(beams, 0xE4) = w4_new_saturated(count, 0x14);
    w3_field<void*>(beams, 0xE8) = w4_new_saturated(w3_field<std::uint32_t>(beams, 0xE0), 4);
    for (std::uint32_t i = 0; i < w3_field<std::uint32_t>(beams, 0xE0); ++i) {
        w3_field<void**>(beams, 0xE8)[i] = w3_field<unsigned char*>(beams, 0xE4) + i * 0x14u;
    }
    for (std::uint32_t i = 0; i < w3_field<std::uint32_t>(beams, 0xE0); ++i) {
        w3_field<float>(w3_field<unsigned char*>(beams, 0xE4) + i * 0x14u, 0x0C) = -1.0f;
        w3_field<float>(w3_field<unsigned char*>(beams, 0xE4) + i * 0x14u, 0x10) = -1.0f;
    }
    const float x = w3_field<float>(beams, 0x7C);
    const float y = w3_field<float>(beams, 0x80);
    w3_field<std::uint32_t>(beams, 0xDC) = w3_field<std::uint32_t>(beams, 0xE0);
    w3_field<float>(beams, 0xCC) = x;
    w3_field<float>(beams, 0xD0) = y;
    w3_field<float>(beams, 0xD4) = x > 0.0f ? static_cast<float>(1.0 / static_cast<double>(x)) : 0.0f;
    w3_field<float>(beams, 0xD8) = y > 0.0f ? static_cast<float>(1.0 / static_cast<double>(y)) : 0.0f;
    if (0.0f > w3_field<float>(beams, 0x50)) w3_field<float>(beams, 0x50) = 0.00999999978f;  // 00D7A238
    w3_field<float>(beams, 0xF0) = -1.0f;
    w3_field<float>(beams, 0xEC) = -1.0f;
}

void w4_ocean_water_particles_initialize_00ba44f0(void* particles, RandomState& state_00e13a88,
                                                  void* device_00f8d394, NativeStringStorage& strings,
                                                  W4_EngineCalls& calls) {
    const std::uint32_t n = w3_field<std::uint32_t>(particles, 0x20);
    if (n == 0) return;
    w3_field<std::uint32_t>(particles, 0xC8) = n * 6u;
    w3_field<std::uint32_t>(particles, 0xC4) = n * 4u;
    void* const texture = w3_vcall<void*>(device_00f8d394, 0x64, w3_bytes(particles) + 0x28, 0u);
    void* material;
    {
        W3_Temporary effect("WaterParticle.mshd", strings);
        material = calls.material_create_for_effect_name_00535320(effect.get());
    }
    const std::pair<const char*, std::size_t> arrays[] = {
        {"cVolumeSize", 0xCC}, {"cVolumeCorner", 0xDC}, {"cTex1", 0xEC}, {"cTex2", 0xFC}, {"cMyTime", 0x10C}};
    for (const auto& [name, offset] : arrays) {
        W3_Temporary temporary(name, strings);
        calls.material_register_float4_array_00b18ac0(material, temporary.get(), w3_bytes(particles) + offset, 1);
    }
    {
        W3_Temporary temporary("cColor", strings);
        calls.material_register_float4_00b18aa0(material, temporary.get(), w3_bytes(particles) + 0x30);
    }
    calls.material_set_parameter_owner_00b18a40(material, particles, nullptr);
    calls.material_set_texture_slot_00b189f0(material, 0, texture);
    calls.material_set_texture_slot_00b189f0(material, 1, nullptr);
    w2_release_object(texture);
    void* declaration;
    {
        W3_Temporary file("WaterParticle.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    W4_DeclarationList list{{declaration, nullptr, nullptr, nullptr}, 1};
    w3_field<void*>(particles, 0x1C) = w3_vcall<void*>(device_00f8d394, 0x40, &list);
    void* const vertices = w3_vcall<void*>(device_00f8d394, 0x5C, w3_field<std::uint32_t>(particles, 0xC4), 1u,
                                           declaration);
    w4_release_clear(w3_field<void*>(particles, 0x18));
    w3_field<void*>(particles, 0x18) = vertices;
    const std::uint32_t bytes = w3_field<std::uint32_t>(declaration, 0xCC) * w3_field<std::uint32_t>(particles, 0xC4);
    std::memset(w3_vcall<void*>(vertices, 0x10, 0u, 0u, 0u), 0, bytes);
    w3_vcall<void>(w3_field<void*>(particles, 0x18), 0x14);
    w2_release_object(declaration);
    void* const indices = w3_vcall<void*>(device_00f8d394, 0x60, w3_field<std::uint32_t>(particles, 0xC8), 1u, 0x65u);
    w4_release_clear(w3_field<void*>(particles, 0x14));
    w3_field<void*>(particles, 0x14) = indices;
    void* const section = calls.draw_section_create_00533fa0();
    calls.draw_section_set_material_00b864c0(section, material);
    w2_release_object(material);
    w3_field<std::uint32_t>(section, 0x0C) = 0;
    w3_field<std::uint32_t>(section, 0x08) = 4;
    w3_field<std::uint32_t>(section, 0x10) = w3_field<std::uint32_t>(particles, 0xC4);
    w3_field<std::uint32_t>(section, 0x14) = 0;
    w3_field<std::uint32_t>(section, 0x18) = w3_field<std::uint32_t>(particles, 0x20) * 2u;
    w3_field<void*>(particles, 0x10) = w4_make_model("WaterParticles", strings, calls);
    calls.node_propagate_root_00b6d890(w3_field<void*>(particles, 0x10), w4_root_of_owner(particles));
    auto* index = w3_vcall<std::uint16_t*>(w3_field<void*>(particles, 0x14), 0x0C, 0u, 0u, 0u);
    for (std::uint32_t i = 0; i < w3_field<std::uint32_t>(particles, 0x20); ++i, index += 6) {
        w4_fill_quads_strip(index, i);
    }
    w3_vcall<void>(w3_field<void*>(particles, 0x14), 0x10);
    void* const mesh = w4_make_mesh(calls);
    calls.mesh_set_vertex_stream_00b73bb0(mesh, 0, w3_field<void*>(particles, 0x18));
    calls.mesh_set_index_stream_00b73b70(mesh, w3_field<void*>(particles, 0x14));
    calls.draw_section_append_vertex_stream_00b85b80(section, w3_field<void*>(particles, 0x18));
    calls.draw_section_set_vertex_layout_00b86650(section, w3_field<void*>(particles, 0x1C));
    calls.mesh_append_draw_section_00b73c60(mesh, section);
    w2_release_object(section);
    calls.generated_model_set_geometry_00b75170(w3_field<void*>(particles, 0x10), 0, mesh, -1.0f, -1.0f);
    w2_release_object(mesh);
    w3_vcall<void*>(w3_field<void*>(particles, 0x18), 0x10, 0u, 0u, 0u);
    const auto draw_byte = [&state_00e13a88]() -> std::uint8_t {
        const std::uint32_t v = random_next_u32_00ba2c20(state_00e13a88);
        const float u = static_cast<float>(w4_unsigned_to_double(v) * 2.3283064365386963e-10);  // 00D63B80
        return static_cast<std::uint8_t>(static_cast<std::int32_t>(static_cast<double>(u) * 255.0));  // chop
    };
    for (std::uint32_t i = 0; i < w3_field<std::uint32_t>(particles, 0x20); ++i) {
        const std::uint8_t b0 = draw_byte();
        const std::uint8_t b1 = draw_byte();
        const std::uint8_t b2 = draw_byte();
        for (std::uint32_t k = 0; k < 4; ++k) {
            void* const buffer = w3_field<void*>(particles, 0x18);
            const std::uint8_t b3 = static_cast<std::uint8_t>(k << 6);
            unsigned char* const vertex = w3_field<unsigned char*>(buffer, 8) +
                                          (k + i * 4u) * w3_field<std::uint32_t>(buffer, 0x0C);
            const std::int32_t packed_offset = w3_field<std::int32_t>(buffer, 0x34);
            if (packed_offset >= 0) {
                w3_field<std::uint32_t>(vertex, static_cast<std::uint32_t>(packed_offset)) =
                    b2 | (static_cast<std::uint32_t>(b1) << 8) | (static_cast<std::uint32_t>(b0) << 16) |
                    (static_cast<std::uint32_t>(b3) << 24);
            } else {
                const std::uint8_t channels[4] = {b0, b1, b2, b3};
                for (std::size_t c = 0; c < 4; ++c) {
                    w3_field<float>(vertex, w3_field<std::uint32_t>(buffer, 0x38 + c * 4)) =
                        static_cast<float>(static_cast<double>(channels[c]) / 255.0);
                }
            }
        }
    }
    w3_vcall<void>(w3_field<void*>(particles, 0x18), 0x14);
}

}  // namespace bsp::water_weather_cc13

namespace bsp::water_weather_cc13 {

void w4_water_tracer_build_skinned_mesh_00bad150(void* resource, std::uint32_t rings, float k,
    std::uint8_t skip_curves, const W2_Array* counts, void* const* curve_b, void* const* curve_a,
    void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls, const W4_CrtMath& crt,
    W4_TracerVertexWriter write_vertex_00baa0f0) {
    w3_field<std::uint32_t>(resource, 0x30) += rings;
    void* declaration;
    {
        W3_Temporary file("watertracerskined.mvfm", strings);
        declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
    }
    void* const vertices = w3_vcall<void*>(device_00f8d394, 0x5C, w3_field<std::uint32_t>(resource, 0x30), 1u,
                                           declaration);
    w2_release_object(declaration);
    float* const vertex_data = w3_vcall<float*>(vertices, 0x10, 0u, 0u, 0u);
    float* out = vertex_data;
    const auto ring_count = [counts](std::uint32_t i) { return static_cast<const std::uint32_t*>(counts->data)[i]; };
    const float step = static_cast<float>(1.0 / static_cast<double>(static_cast<std::int32_t>(rings - 1u)));
    float t = 0.0f;
    std::uint32_t triangles = 0;
    for (std::uint32_t i = 0; i < rings; ++i) {
        float e;
        if (k > 1.0f) {
            const float l = static_cast<float>(crt.log_00bfa910(t));
            const float lk = static_cast<float>(static_cast<double>(l) * k);
            e = static_cast<float>(crt.exp_00bfeeec(lk));
        } else {
            e = t;
        }
        const float points = static_cast<float>(w4_unsigned_to_double(w3_field<std::uint32_t>(resource, 0x10) - 1u));
        const float m = points > 1.0f ? points : 1.0f;
        const float w = static_cast<float>(-static_cast<double>(e) * m);
        for (std::uint32_t j = 0; j < ring_count(i); ++j) {
            const float s = static_cast<float>(
                (w4_unsigned_to_double(j) / static_cast<double>(static_cast<std::int32_t>(ring_count(i) - 1u)) - 0.5) *
                2.0);
            float product = 0.0f;
            if (skip_curves == 0) {
                const float magnitude = s > 0.0f ? s : -0.0f - s;
                const double a = w3_vcall<double>(*curve_a, 8, e);
                const double b = w3_vcall<double>(*curve_b, 8, magnitude);
                product = static_cast<float>(b * a);
            }
            if (s == 0.0f) {
                write_vertex_00baa0f0(resource, out, s, w, product, -1.0f);
                out += 16;
                write_vertex_00baa0f0(resource, out, s, w, product, 1.0f);
            } else {
                write_vertex_00baa0f0(resource, out, s, w, product, 0.0f > s ? -1.0f : 1.0f);
            }
            out += 16;
        }
        t = static_cast<float>(static_cast<double>(t) + step);
        if (i < rings - 1u) triangles += ring_count(i) + ring_count(i + 1) - 2u;
    }
    void* const indices = w3_vcall<void*>(device_00f8d394, 0x60, triangles * 3u, 1u, 0x65u);
    auto* const index_data = w3_vcall<std::uint16_t*>(indices, 0x0C, 0u, 0u, 0u);
    std::uint16_t* index = index_data;
    std::uint32_t a = 0;
    std::uint32_t b = ring_count(0) + 1u;
    for (std::uint32_t r = 0; r + 1u < rings; ++r) {
        const std::uint32_t strip = ring_count(r) + ring_count(r + 1) - 2u;
        std::uint32_t x = a, y = b, z = a + 1u;
        if (strip != 0) {
            const std::int32_t mid = static_cast<std::int32_t>(strip) / 2 - 1;
            for (std::uint32_t n = 0; n < strip; ++n) {
                index[0] = static_cast<std::uint16_t>(x);
                index[1] = static_cast<std::uint16_t>(y);
                index[2] = static_cast<std::uint16_t>(z);
                index += 3;
                const std::uint32_t old_y = y, old_z = z;
                x = old_y;
                y = old_z;
                z = old_y + 1u;
                if (n == static_cast<std::uint32_t>(mid)) {
                    x = z;
                    y = old_z + 1u;
                    z = old_y + 2u;
                }
            }
        }
        a = a + ring_count(r) + 1u;
        b = b + ring_count(r + 1) + 1u;
    }
    for (std::uint32_t q = 0; q < w3_field<std::uint32_t>(resource, 0x10); ++q) {
        std::uint32_t counted = 0, highest = 0;
        const float limit = static_cast<float>(static_cast<std::int32_t>(q));
        const std::uint16_t* triangle = index_data;
        bool stopped = false;
        for (std::uint32_t n = 0; n < triangles && !stopped; ++n, triangle += 3) {
            for (std::uint32_t c = 0; c < 3; ++c) {
                const std::uint32_t idx = triangle[c];
                if (static_cast<std::int32_t>(highest) <= static_cast<std::int32_t>(idx)) highest = idx;
                const float* const v = vertex_data + idx * 16u;
                if (v[4] > limit || v[5] > limit || v[6] > limit) {
                    stopped = true;
                    break;
                }
            }
            if (!stopped) ++counted;
        }
        w3_field<std::uint32_t*>(resource, 0x18)[q] = static_cast<std::int32_t>(counted) > 0 ? highest + 1u : 0u;
        w3_field<std::uint32_t*>(resource, 0x24)[q] = counted;
    }
    w3_vcall<void>(vertices, 0x14);
    w3_vcall<void>(indices, 0x10);
    void* const mesh = w4_make_mesh(calls);
    w3_field<void*>(resource, 0x0C) = mesh;
    calls.mesh_set_vertex_stream_00b73bb0(mesh, 0, vertices);
    w2_release_object(vertices);
    calls.mesh_set_index_stream_00b73b70(w3_field<void*>(resource, 0x0C), indices);
    w2_release_object(indices);
}

void* w4_water_tracer_resource_initialize_00badc70(void* resource, const std::uint32_t* parameters,
    void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls, const W4_CrtMath& crt,
    const W4_TracerCalls& tracer) {
    const auto word = [parameters](std::size_t offset) { return parameters[offset / 4]; };
    const auto real = [parameters](std::size_t offset) {
        float f;
        std::memcpy(&f, &parameters[offset / 4], 4);
        return f;
    };
    w3_field<std::uint32_t>(resource, 0) = 0x00CEB130u;
    w3_field<std::uint32_t>(resource, 4) = 1;
    w3_field<std::uint32_t>(resource, 0) = 0x00D63F94u;
    w3_field<float>(resource, 0x08) = real(0x34);
    w3_field<void*>(resource, 0x14) = nullptr;
    auto& first = w3_field<W2_Array>(resource, 0x18);
    auto& second = w3_field<W2_Array>(resource, 0x24);
    first = W2_Array{};
    second = W2_Array{};
    w3_field<std::uint32_t>(resource, 0x48) = word(0x0C);
    w3_field<float>(resource, 0x4C) = real(0x38);
    {
        W3_Temporary effect((word(0x08) & 0xFFu) != 0 ? "watertracerstatic.mshd" : "watertracerskined.mshd", strings);
        w3_field<void*>(resource, 0x14) = w3_vcall<void*>(device_00f8d394, 0x48, effect.get());
    }
    w3_field<std::uint32_t>(resource, 0x10) = word(0x00);
    tracer.resize_00468050(first, static_cast<std::int32_t>(word(0x00)));
    tracer.resize_00468050(second, static_cast<std::int32_t>(w3_field<std::uint32_t>(resource, 0x10)));
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(first.count); ++i) {
        static_cast<std::uint32_t*>(first.data)[i] = 0;
        static_cast<std::uint32_t*>(second.data)[i] = 0;
    }
    const std::uint32_t rings = w3_field<std::uint32_t>(resource, 0x10) * 2u - 2u;
    W2_Array local{};
    tracer.resize_00468050(local, static_cast<std::int32_t>(rings));
    w3_field<std::uint32_t>(resource, 0x30) = 0;
    if (rings != 0) {
        const double step = 1.0 / w4_unsigned_to_double(rings - 1u);
        for (std::uint32_t i = 0; i < rings; ++i) {
            const float x = static_cast<float>(w4_unsigned_to_double(i) * step);
            auto& count = static_cast<std::uint32_t*>(local.data)[i];
            const float k = real(0x24);
            if (k == 0.0f) {
                count = word(0x20);
            } else {
                const float l = static_cast<float>(crt.log_00bfa910(x));
                const float lk = static_cast<float>(static_cast<double>(l) * k);
                const float v = static_cast<float>(crt.exp_00bfeeec(lk));
                const float m = static_cast<float>((1.0 - static_cast<double>(v)) * w4_unsigned_to_double(word(0x20)));
                const float bounded = m > 3.0f ? m : 3.0f;
                float rounded = static_cast<float>(crt.floor_00bf85b0(bounded));
                const float fraction = static_cast<float>(static_cast<double>(bounded) - rounded);
                double value = rounded;
                if (std::fabs(fraction) > 0.5f) value += 1.0;  // FADD QWORD 00D7A210
                count = static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<float>(value)));  // _ftol2
            }
            w3_field<std::uint32_t>(resource, 0x30) += count;
        }
    }
    if ((word(0x08) & 0xFFu) != 0) {
        tracer.build_static_mesh_00bacbd0(resource, rings, real(0x1C), 1u, &local, word(0x0C), word(0x10), word(0x14),
                                          real(0x28), real(0x2C), real(0x30), real(0x34), real(0x38), real(0x3C),
                                          real(0x40));
    } else {
        tracer.build_skinned_mesh_00bad150(resource, rings, real(0x1C), 1u, &local,
                                           reinterpret_cast<void* const*>(static_cast<std::uintptr_t>(word(0x10))),
                                           reinterpret_cast<void* const*>(static_cast<std::uintptr_t>(word(0x14))));
    }
    void* const slot = tracer.draw_section_allocate_00b85ee0(0x60);
    void* const section = slot != nullptr ? tracer.draw_section_construct_00b857f0(slot) : nullptr;
    w3_field<std::uint32_t>(section, 0x08) = 5;
    for (std::size_t offset : {0x0Cu, 0x10u, 0x14u, 0x18u}) w3_field<std::uint32_t>(section, offset) = 0;
    calls.draw_section_rebuild_layout_00b865a0(section, w3_field<void*>(resource, 0x0C));
    calls.mesh_append_draw_section_00b73c60(w3_field<void*>(resource, 0x0C), section);
    w2_release_object(section);
    singleton_lifetime_free(local.data);  // 00BF6989
    return resource;
}

}  // namespace bsp::water_weather_cc13

namespace bsp::water_weather_cc13 {
namespace {

// The 9-float triangle written into a generated model's vertex stream 0 (00D0C5F8 50000.0f, 00D63AB8
// -50000.0f), through the stream's lock (+10h with 0, 0, 0) and unlock (+14h).
void w4_write_ocean_triangle(void* model) {
    void* const geometry = w3_field<void*>(model, 0x180);         // 00B74640
    void* const stream = w3_field<void*>(geometry, 0x64);         // 00B73260(0)
    auto* const v = w3_vcall<float*>(stream, 0x10, 0u, 0u, 0u);
    v[2] = 50000.0f;
    v[0] = 0.0f;
    v[1] = 0.0f;
    v[3] = 50000.0f;
    v[4] = 0.0f;
    v[5] = -50000.0f;
    v[6] = -50000.0f;
    v[7] = 0.0f;
    v[8] = -50000.0f;
    w3_vcall<void>(stream, 0x14);
}

// 00B4C700 with three pooled temporaries (name in ECX, geometry in EDX, shader on the stack).
void* w4_create_generated_model(const char* name, const char* geometry, const char* shader, std::uint32_t a,
                                std::uint32_t b, std::uint32_t c, std::uint32_t d, NativeStringStorage& strings,
                                W3_EngineCalls& calls) {
    W3_Temporary shader_name(shader, strings);
    W3_Temporary geometry_name(geometry, strings);
    W3_Temporary model_name(name, strings);
    return calls.create_generated_model_00b4c700(model_name.get(), geometry_name.get(), shader_name.get(), a, b, c, d);
}

// Camera with clear flags 7, a 200h x 200h viewport and a system fog owner; frame targets into `targets`.
void w4_make_water_camera(void* field, std::size_t camera_offset, std::size_t targets_offset, const char* name,
                          NativeStringStorage& strings, W4_EngineCalls& calls, const W4_WaveFieldBaseCalls& base) {
    void* camera = nullptr;
    if (void* const slot = calls.camera_allocate_raw_slot_00b71930(0x458)) {
        W3_Temporary temporary(name, strings);
        camera = calls.camera_construct_00b71a80(slot, temporary.get());
    }
    w3_field<void*>(field, camera_offset) = camera;
    calls.camera_set_clear_flags_00b6fe10(w3_field<void*>(field, camera_offset), 7);
    void* const targets = w3_new(0x40);
    w3_field<void*>(field, targets_offset) = targets != nullptr ? calls.frame_targets_construct_00b1fbb0(targets) : nullptr;
    void* const viewport_block = w3_new(0x34);
    void* const viewport = viewport_block != nullptr ? calls.viewport_construct_00b1f850(viewport_block) : nullptr;
    calls.camera_set_viewport_00b71990(w3_field<void*>(field, camera_offset), viewport);
    const std::uint32_t size[2] = {0x200, 0x200};
    calls.viewport_set_dimensions_00b1f940(viewport, size);
    w2_release_object(viewport);
    void* const fog_block = w3_new(0x94);
    void* const fog = fog_block != nullptr ? base.fog_owner_construct_00b84e50(fog_block) : nullptr;
    base.camera_set_fog_owner_00b71940(w3_field<void*>(field, camera_offset), fog);
    w2_release_object(fog);
}

}  // namespace

void* w4_ocean_wave_field_base_construct_00ba1400(void* field, void* world, void* device_00f8d394,
    void* floating_particles_00f8c274, NativeStringStorage& strings, W4_EngineCalls& calls,
    const W4_WaveFieldBaseCalls& base) {
    w3_field<std::uint32_t>(field, 0) = 0x00D63928u;
    base.weak_owner_construct_00925490(w3_bytes(field) + 4);
    w3_field<std::uint32_t>(field, 0) = 0x00D63B5Cu;
    w3_field<std::uint32_t>(field, 4) = 0x00D63B54u;
    w3_field<std::uint32_t>(field, 0x14) = 0;
    w3_field<std::uint8_t>(field, 0x84) = 0;
    for (std::size_t o : {0xA0u, 0xA4u, 0xA8u, 0xACu}) w3_field<std::uint32_t>(field, o) = 0;
    w3_field<float>(field, 0x114) = 20.0f;  // 00CE3930
    for (std::size_t o : {0xBCu, 0xC0u, 0xC4u, 0xC8u}) w3_field<std::uint32_t>(field, o) = 0;
    w3_field<std::uint8_t>(field, 0xCC) = 0;
    for (std::size_t o : {0xD0u, 0xE4u, 0xE8u, 0xF0u, 0xF4u}) w3_field<std::uint32_t>(field, o) = 0;
    w3_field<std::uint8_t>(field, 0xF8) = 1;
    w3_field<float>(field, 0x118) = 0.5f;  // 00CE3800
    for (std::size_t o : {0x11Cu, 0x120u, 0x124u, 0x12Cu, 0x130u, 0x27Cu, 0x280u, 0x284u, 0x288u, 0x2ACu, 0x2B0u,
                          0x2B4u, 0x2B8u, 0x2BCu, 0x498u, 0x49Cu, 0x4A0u, 0x4E4u, 0x4E8u, 0x4F0u, 0x4F8u, 0x4FCu,
                          0x500u, 0x504u, 0x5F4u, 0x5F8u, 0x5FCu, 0x600u, 0x604u, 0x60Cu, 0x610u, 0x614u, 0x618u,
                          0x61Cu, 0x620u, 0x624u, 0x628u}) {
        w3_field<std::uint32_t>(field, o) = 0;
    }
    w3_field<std::uint8_t>(field, 0x10) = base.check_vertex_texture_format_00b20190(device_00f8d394, 0x71);
    w3_field<float>(field, 0x66C) = 5000.0f;  // 00D1AF84
    w3_field<float>(field, 0x670) = 0.0f;
    w3_field<float>(field, 0x674) = 0.0f;
    w3_field<float>(field, 0x678) = 0.0f;
    w3_field<float>(field, 0xD4) = 0.5f;
    w3_field<float>(field, 0x20) = 1.0f;
    w3_field<float>(field, 0xD8) = 0.0500000007f;  // 00CE7638
    for (std::size_t o : {0x34u, 0x38u, 0x3Cu, 0x40u}) w3_field<float>(field, o) = 1.0f;
    w3_field<std::uint32_t>(field, 0xE0) = 0;
    w3_field<std::uint32_t>(field, 0xDC) = 0;
    const auto make_scene = [&](const char* name) -> void* {
        void* const block = w3_new(0x24);
        if (block == nullptr) return nullptr;
        W3_Temporary temporary(name, strings);
        return base.scene_construct_00b724e0(block, temporary.get());
    };
    w3_field<void*>(field, 0x14) = make_scene("FloatingObjectScene");
    base.create_floating_particles_00af0f20(floating_particles_00f8c274);
    calls.node_propagate_root_00b6d890(w3_field<void*>(floating_particles_00f8c274, 0x24), w3_field<void*>(field, 0x14));
    w4_make_water_camera(field, 0xC8, 0x604, "WaterReflection", strings, calls, base);
    w4_make_water_camera(field, 0xD0, 0x608, "WaterRefraction", strings, calls, base);
    w3_field<float>(field, 0xB0) = 300.0f;          // 00CE3AE8
    w3_field<float>(field, 0xB4) = 0.00333333341f;  // 00D63B1C
    w3_field<float>(field, 0xB8) = 1.39999998f;     // 00D06874
    w3_field<float>(field, 0x24) = 0.0f;
    void* const old_world = w3_field<void*>(field, 0xC0);
    if (old_world != world) {
        w3_field<void*>(field, 0xC0) = world;
        if (world != nullptr) _InterlockedIncrement(reinterpret_cast<volatile long*>(w3_bytes(world) + 4));
        if (old_world != nullptr) w2_release_object(old_world);
    }
    w3_field<std::uint32_t>(field, 0x278) = 0;
    w3_field<std::uint32_t>(field, 0x274) = 0;
    w3_field<void*>(field, 0x11C) = w4_make_model("OceanSurface", strings, calls);
    calls.node_propagate_root_00b6d890(w3_field<void*>(field, 0x11C), w3_field<void*>(field, 0xC0));
    w3_field<float>(field, 0x28C) = 0.0f;
    w3_field<float>(field, 0x294) = 0.0f;
    w3_field<float>(field, 0x290) = 1.0f;
    w3_field<float>(field, 0x298) = 1.0f;
    w3_field<float>(field, 0x29C) = 0.0f;
    w3_field<float>(field, 0x2A4) = 0.0f;
    w3_field<float>(field, 0x2A0) = 1.0f;
    w3_field<float>(field, 0x2A8) = 3.0f;  // 00D63924
    w3_field<void*>(field, 0x4A0) = w4_create_generated_model(
        "ShadowRender", "position.mvfm",
        w3_field<std::uint8_t>(field, 0x10) != 0 ? "oceanshadow_vsampler.mshd" : "oceanshadow.mshd", 4, 3, 1, 0,
        strings, calls);
    calls.node_propagate_root_00b6d890(w3_field<void*>(field, 0x4A0), w3_field<void*>(field, 0x14));
    const auto shadow_material = [field]() {
        return w3_field<void*>(w3_field<void**>(w3_field<void*>(w3_field<void*>(field, 0x4A0), 0x180), 0x54)[0], 0x20);
    };
    {
        W3_Temporary name("cStaticShot_Size_OffsetX_OffsetY", strings);
        calls.material_register_float4_array_00b18ac0(shadow_material(), name.get(), w3_bytes(field) + 0x66C, 1);
    }
    {
        W3_Temporary name("cCameraViewMat", strings);
        base.material_register_matrix_00b18b40(shadow_material(), name.get(), w3_bytes(field) + 0x67C);
    }
    w4_write_ocean_triangle(w3_field<void*>(field, 0x4A0));
    void* resource = nullptr;
    if (void* const block = w3_new(0x3C)) {
        W3_Temporary name("AllLights", strings);
        resource = base.scene_resource_construct_00b83c50(block, name.get());
    }
    base.scene_set_resource_00b723f0(w3_field<void*>(field, 0x14), resource);
    if (resource != nullptr) w2_release_object(resource);
    void* const lighting = base.scene_lighting_owner_00b72110(w3_field<void*>(field, 0xC0));
    void* const head = w3_field<void*>(lighting, 0x1C);
    void* const first = w3_field<void*>(head, 0);
    if (first == head) base.invalid_iterator_00bf6713();
    void* const sun_source = w3_field<void*>(first, 8);
    void* sun = nullptr;
    if (void* const slot = base.directional_light_allocate_00b7bd40(0x1EC)) {
        W3_Temporary name("TheSun", strings);
        sun = base.directional_light_construct_00b7c6b0(slot, name.get());
    }
    w3_field<void*>(field, 0x600) = sun;
    base.light_set_shadow_owner_00b7bdf0(w3_field<void*>(field, 0x600), base.light_shadow_owner_00b7aab0(sun_source));
    for (std::size_t o : {0x600u, 0x4A0u}) {
        void* const object = w3_field<void*>(field, o);
        w3_vcall<void>(object, 0x50, base.scene_lighting_owner_00b72110(w3_field<void*>(field, 0x14)), 1u);
    }
    w3_field<void*>(field, 0x120) =
        w4_create_generated_model("OceanFoam", "position.mvfm", "oceanfoam.mshd", 4, 3, 1, 0, strings, calls);
    w4_write_ocean_triangle(w3_field<void*>(field, 0x120));
    w3_field<void*>(field, 0x504) =
        w4_create_generated_model("Splashes", "pf43nf43uf44.mvfm", "oceansplash.mshd", 4, 0, 0, 0, strings, calls);
    w3_field<void*>(field, 0x4E4) = make_scene("EmptyReflection");
    for (std::size_t o : {0x104u, 0x108u, 0x10Cu, 0x110u}) w3_field<float>(field, o) = -1.0e10f;  // 00CE4ADC
    void* const velocity = w4_create_generated_model("OceanVelAndDepth", "position.mvfm", "ocean_vel_and_depth.mshd", 4,
                                                     3, 1, 0, strings, calls);
    calls.node_propagate_root_00b6d890(velocity, world);
    w4_write_ocean_triangle(velocity);
    return field;
}

}  // namespace bsp::water_weather_cc13

namespace bsp::water_weather_cc13 {

void w4_ocean_height_generator_initialize_passes_00ba7530(void* generator, std::int32_t width, std::int32_t height,
    const volatile std::int32_t& scale_00e14454, void* device_00f8d394, NativeStringStorage& strings,
    W4_EngineCalls& calls, const W4_HeightGeneratorCalls& passes) {
    const auto make_scene = [&](const char* name) -> void* {
        void* const block = w3_new(0x24);
        if (block == nullptr) return nullptr;
        W3_Temporary temporary(name, strings);
        return passes.scene_construct_00b724e0(block, temporary.get());
    };
    const auto make_camera = [&](const char* name) -> void* {
        void* const slot = calls.camera_allocate_raw_slot_00b71930(0x458);
        if (slot == nullptr) return nullptr;
        W3_Temporary temporary(name, strings);
        return calls.camera_construct_00b71a80(slot, temporary.get());
    };
    const auto set_viewport = [&](void* camera, std::uint32_t w, std::uint32_t h) {
        void* const block = w3_new(0x34);
        void* const viewport = block != nullptr ? calls.viewport_construct_00b1f850(block) : nullptr;
        const std::uint32_t size[2] = {w, h};
        calls.viewport_set_dimensions_00b1f940(viewport, size);
        calls.camera_set_viewport_00b71990(camera, viewport);
        if (viewport != nullptr) w2_release_object(viewport);
    };
    const auto make_buffer = [&](const char* declaration_file) -> void* {
        void* declaration;
        {
            W3_Temporary file(declaration_file, strings);
            declaration = w3_vcall<void*>(device_00f8d394, 0x38, file.get());
        }
        void* const buffer = w3_vcall<void*>(device_00f8d394, 0x5C, 4u, 1u, declaration);
        w2_release_object(declaration);
        return buffer;
    };
    static constexpr float kQuad[4][6] = {
        {-1.0f, -1.0f, 0.5f, 1.0f, 0.0f, 1.0f},
        {-1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f},
        {1.0f, -1.0f, 0.5f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.0f}};
    const auto fill_simple = [](void* buffer) {
        auto* const v = w3_vcall<float*>(buffer, 0x10, 0u, 0u, 0u);
        std::memcpy(v, kQuad, sizeof kQuad);
        w3_vcall<void>(buffer, 0x14);
    };
    // The common tail of each pass: mesh, material, section, model rooted under `scene`.
    const auto finish = [&](void* buffer, std::size_t material_offset, const char* effect,
                            const auto& register_constants, std::size_t model_offset, const char* model_name,
                            std::size_t scene_offset) {
        void* const mesh = w4_make_mesh(calls);
        calls.mesh_set_vertex_stream_00b73bb0(mesh, 0, buffer);
        w2_release_object(buffer);
        {
            W3_Temporary name(effect, strings);
            w3_field<void*>(generator, material_offset) = calls.material_create_for_effect_name_00535320(name.get());
        }
        calls.material_set_parameter_owner_00b18a40(w3_field<void*>(generator, material_offset), generator, nullptr);
        register_constants(w3_field<void*>(generator, material_offset));
        void* const section = calls.draw_section_create_00533fa0();
        w3_field<std::uint32_t>(section, 0x08) = 5;
        w3_field<std::uint32_t>(section, 0x0C) = 0;
        w3_field<std::uint32_t>(section, 0x10) = 4;
        w3_field<std::uint32_t>(section, 0x14) = 0;
        w3_field<std::uint32_t>(section, 0x18) = 2;
        calls.draw_section_set_material_00b864c0(section, w3_field<void*>(generator, material_offset));
        calls.draw_section_rebuild_layout_00b865a0(section, mesh);
        calls.mesh_append_draw_section_00b73c60(mesh, section);
        w3_field<void*>(generator, model_offset) = w4_make_model(model_name, strings, calls);
        calls.generated_model_set_geometry_00b75170(w3_field<void*>(generator, model_offset), 0, mesh, -1.0f, -1.0f);
        calls.node_propagate_root_00b6d890(w3_field<void*>(generator, model_offset), w3_field<void*>(generator, scene_offset));
        w2_release_object(section);
        w2_release_object(mesh);
    };
    const auto float4 = [&](void* material, const char* name, void* value) {
        W3_Temporary temporary(name, strings);
        calls.material_register_float4_array_00b18ac0(material, temporary.get(), value, 1);
    };
    const auto named = [&](void (*register_value)(void*, const void*, void*), void* material, const char* name,
                           void* value) {
        W3_Temporary temporary(name, strings);
        register_value(material, temporary.get(), value);
    };
    // 1. Height map generation.
    w3_field<void*>(generator, 0x30) = make_scene("oceanheightmapscene");
    w3_field<void*>(generator, 0x34) = make_camera("OceanHeigtmapGen");
    calls.camera_set_clear_flags_00b6fe10(w3_field<void*>(generator, 0x34), 0);
    passes.camera_set_render_mode_00b6fdf0(w3_field<void*>(generator, 0x34), 0);
    {
        void* const block = w3_new(0x34);
        void* const viewport = block != nullptr ? calls.viewport_construct_00b1f850(block) : nullptr;
        const std::uint32_t size[2] = {static_cast<std::uint32_t>(scale_00e14454 * width),
                                       static_cast<std::uint32_t>(scale_00e14454 * height)};
        calls.viewport_set_dimensions_00b1f940(viewport, size);
        calls.camera_set_viewport_00b71990(w3_field<void*>(generator, 0x34), viewport);
        if (viewport != nullptr) w2_release_object(viewport);
    }
    {
        void* const buffer = make_buffer("oceanheightmapgen.mvfm");
        fill_simple(buffer);
        finish(buffer, 0x3C, "oceanheightmapgen.mshd",
               [&](void* material) {
                   float4(material, "cCorner0", w3_bytes(generator) + 0xC0);
                   float4(material, "cCorner1", w3_bytes(generator) + 0xD0);
                   float4(material, "cCorner2", w3_bytes(generator) + 0xE0);
                   float4(material, "cCorner3", w3_bytes(generator) + 0xF0);
                   named(passes.material_register_float_00b18b20, material, "cLayerScale", w3_bytes(generator) + 0x9C);
                   named(passes.material_register_float3_00b18ae0, material, "cWaveScale", w3_bytes(generator) + 0x100);
                   named(passes.material_register_float3_00b18ae0, material, "cEyePos", w3_bytes(generator) + 0x10C);
               },
               0x38, "OceanHeightmapgen", 0x30);
    }
    // 2. Normal map generation.
    w3_field<void*>(generator, 0x1C) = make_scene("oceannormalgenscene");
    w3_field<void*>(generator, 0x24) = make_camera("OceanNormalgenCamera");
    calls.camera_set_clear_flags_00b6fe10(w3_field<void*>(generator, 0x24), 7);
    set_viewport(w3_field<void*>(generator, 0x24), static_cast<std::uint32_t>(scale_00e14454 * width),
                 static_cast<std::uint32_t>(scale_00e14454 * height));
    {
        void* const buffer = make_buffer("oceannormalmapgen.mvfm");
        auto* const v = w3_vcall<float*>(buffer, 0x10, 0u, 0u, 0u);
        const std::uint32_t sw = static_cast<std::uint32_t>(scale_00e14454 * width);
        const float dx = static_cast<float>(1.0 / w4_unsigned_to_double(sw));
        const std::uint32_t sh = static_cast<std::uint32_t>(scale_00e14454 * height);
        const float dy = static_cast<float>(1.0 / w4_unsigned_to_double(sh));
        w3_field<float>(generator, 0xB8) = dx;
        w3_field<float>(generator, 0xBC) = dy;
        for (int k = 0; k < 4; ++k) {
            const float u = kQuad[k][4], t = kQuad[k][5];
            const float vertex[14] = {kQuad[k][0], kQuad[k][1], kQuad[k][2], kQuad[k][3], u, t,
                                      static_cast<float>(static_cast<double>(u) - dx), t,
                                      static_cast<float>(static_cast<double>(u) + dx), t,
                                      u, static_cast<float>(static_cast<double>(t) - dy),
                                      u, static_cast<float>(static_cast<double>(t) + dy)};
            std::memcpy(v + k * 14, vertex, sizeof vertex);
        }
        w3_vcall<void>(buffer, 0x14);
        finish(buffer, 0x28, "oceannormalgen.mshd",
               [&](void* material) {
                   unsigned char* const corners = w3_field<unsigned char*>(generator, 0x88);
                   float4(material, "cCorner0", corners);
                   float4(material, "cCorner1", corners + 0x10);
                   float4(material, "cCorner2", corners + 0x20);
                   float4(material, "cCorner3", corners + 0x30);
                   named(passes.material_register_float2_00b18b00, material, "cInvMapSize", w3_bytes(generator) + 0xB8);
               },
               0x20, "OceanNormalGen", 0x1C);
    }
    // 3. Height scale.
    w3_field<void*>(generator, 0x4C) = make_scene("oceanHeightScalescene");
    w3_field<void*>(generator, 0x54) = make_camera("OceanHeightScaleCam");
    calls.camera_set_clear_flags_00b6fe10(w3_field<void*>(generator, 0x54), 0);
    set_viewport(w3_field<void*>(generator, 0x54), static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
    {
        void* const buffer = make_buffer("oceanheightmapgen.mvfm");
        fill_simple(buffer);
        finish(buffer, 0x58, "oceanheightscale.mshd", [](void*) {}, 0x50, "OceanHeightScale", 0x4C);
    }
    // 4. Wave damping target.
    w3_field<void*>(generator, 0x08) = make_scene("wave_damp_scene");
    w3_field<void*>(generator, 0x0C) = make_camera("wave_damp_camera");
    calls.camera_set_clear_color_00b6fe50(w3_field<void*>(generator, 0x0C), 0xFFFFFFFFu);
    calls.camera_set_clear_flags_00b6fe10(w3_field<void*>(generator, 0x0C), 1);
    {
        void* const block = w3_new(0x34);
        void* const viewport = block != nullptr ? calls.viewport_construct_00b1f850(block) : nullptr;
        const std::uint32_t size[2] = {0x80, 0x80};
        calls.viewport_set_dimensions_00b1f940(viewport, size);
        calls.camera_set_viewport_00b71990(w3_field<void*>(generator, 0x0C), viewport);
        void* const targets = w3_new(0x40);
        void* const built = targets != nullptr ? calls.frame_targets_construct_00b1fbb0(targets) : nullptr;
        w3_field<void*>(generator, 0x18) = built;
        calls.frame_targets_set_color_00b1fab0(built, 0, w3_field<void*>(generator, 0x14));
        if (viewport != nullptr) w2_release_object(viewport);
    }
    w3_field<std::uint32_t>(generator, 0x80) = 0;
    w3_field<std::uint32_t>(generator, 0x84) = 1;
}

}  // namespace bsp::water_weather_cc13
