// Post-effect math, texture alpha fix-ups and the .mshd description tree.
// Evidence and original ABIs are in include/bsp/post_effects_cc13.hpp and
// docs/CC13_POST_EFFECTS_AND_RESOURCES.md. Names are hypotheses.

#include "bsp/post_effects_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp::cc13fx {
namespace {

// FILD of a DWORD followed by FADD 2^32 (CE3978) when the sign bit is set:
// the image converts extents as unsigned.
double unsigned_extent(std::uint32_t v) { return static_cast<double>(v); }

float f(double v) { return static_cast<float>(v); }

// FSIN / FCOS and the 24-bit precision-control model: include/bsp/cc13_x87_model.hpp.
using cc13x87::x87_sin;
using cc13x87::x87_cos;
double mul24(double a, double b) { return cc13x87::x87_mul24(a, b); }
double add24(double a, double b) { return cc13x87::x87_add24(a, b); }
double sub24(double a, double b) { return cc13x87::x87_sub24(a, b); }
double div24(double a, double b) { return cc13x87::x87_div24(a, b); }
// CE3828: the double 6.2831854820251465, i.e. (double)(float)(2*pi).
constexpr double kTwoPi = 6.2831854820251465;
constexpr float kNegZero = -0.0f;  // D7A208

void normalize3(float* v, float (*len_of)(const float*)) {
    const float len = len_of(v);
    const float scale = len > 0.0f ? f(1.0 / len) : 0.0f;
    v[0] = f(scale * v[0]);
    v[1] = f(v[1] * scale);
    v[2] = f(scale * v[2]);
}

// 00419440 BSP_Vector3f_Length: sqrt(x*x + y*y + z*z) rounded to float.
float length3(const float* v) {
    return f(std::sqrt(static_cast<double>(v[0]) * v[0] + static_cast<double>(v[1]) * v[1] +
        static_cast<double>(v[2]) * v[2]));
}

} // namespace

void premultiply_alpha_bgra(std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) {
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            std::uint8_t* p = pixels + (static_cast<std::size_t>(width) * y + x) * 4;
            const unsigned a = p[3];
            p[2] = static_cast<std::uint8_t>((p[2] * a) / 0xff);
            p[1] = static_cast<std::uint8_t>((p[1] * a) / 0xff);
            p[0] = static_cast<std::uint8_t>((p[0] * a) / 0xff);
        }
    }
}

void alpha_from_non_black_bgra(std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) {
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            std::uint8_t* p = pixels + (static_cast<std::size_t>(width) * y + x) * 4;
            p[3] = (static_cast<unsigned>(p[2]) + p[1] + p[0]) != 0 ? 0xff : 0;
        }
    }
}

void sample_offsets_downscale4x4(std::uint32_t width, std::uint32_t height, Float4 out[16]) {
    const float inv_w = f(1.0 / unsigned_extent(width));
    const float inv_h = f(1.0 / unsigned_extent(height));
    // Column x values are computed once, before the loop (XMM0..XMM3).
    const float col[4] = {f(inv_w * (0.0 - 1.5)), f(inv_w * -0.5), f(inv_w * 0.5), f(1.5 * inv_w)};
    for (int row = 0; row < 4; ++row) {
        const float y = f((row - 1.5) * inv_h);
        for (int c = 0; c < 4; ++c) {
            out[row * 4 + c].x = col[c];
            out[row * 4 + c].y = y;
        }
    }
}

void sample_offsets_avg_lum3x3(std::uint32_t width, std::uint32_t height, Float4 out[9]) {
    const float tu = f(1.0 / (unsigned_extent(width) * 3.0));
    const float tv = f(1.0 / (unsigned_extent(height) * 3.0));
    const float y0 = f(-1.0 * tv);
    const float y1 = f(tv * 0.0);
    for (int i = -1; i < 2; ++i) {
        const float x = f(i * static_cast<double>(tu));
        Float4* row = out + 3 * (i + 1);
        row[0].x = x; row[0].y = y0;
        row[1].x = x; row[1].y = y1;
        row[2].x = x; row[2].y = tv;
    }
}

float gaussian_distribution(float x, float y, float rho) {
    const float exponent = f(-(static_cast<double>(x) * x + static_cast<double>(y) * y) /
        ((static_cast<double>(rho) + rho) * rho));
    const float e = f(std::exp(static_cast<double>(exponent)));
    const float root = f(std::sqrt(static_cast<double>(f(kTwoPi * rho * rho))));
    return f(static_cast<double>(f(1.0 / root)) * e);
}

void sample_offsets_bloom15(std::uint32_t width, std::uint32_t height, Float4 offsets[15],
    Float4 weights[15], float deviation, float multiplier) {
    const float inv_w = f(1.0 / unsigned_extent(width));
    const float inv_h = f(1.0 / unsigned_extent(height));
    const float w0 = f(static_cast<double>(gaussian_distribution(0.0f, 0.0f, deviation)) * multiplier);
    weights[0] = {w0, w0, w0, 1.0f};
    offsets[0] = {0.0f, 0.0f, 0.0f, 0.0f};
    const double y_sq = 0.0 * 0.0;  // [ESP+30h], kept as a double
    const double two_rho_sq = (static_cast<double>(deviation) + deviation) * deviation;  // [ESP+38h]
    const float inv_root = f(1.0 / f(std::sqrt(static_cast<double>(f(kTwoPi * deviation * deviation)))));
    for (int i = 1; i < 8; ++i) {
        const float x = static_cast<float>(i);
        const float exponent = f(-((static_cast<double>(x) * x + y_sq) / two_rho_sq));
        const float e = f(std::exp(static_cast<double>(exponent)));
        const float w = f(static_cast<double>(f(static_cast<double>(e) * inv_root)) * multiplier);
        offsets[i] = {f(static_cast<double>(x) * inv_w), f(static_cast<double>(x) * inv_h), 0.0f, 0.0f};
        weights[i] = {w, w, w, 1.0f};
    }
    for (int i = 1; i < 8; ++i) {
        weights[i + 7] = weights[i];
        offsets[i + 7] = {kNegZero - offsets[i].x, kNegZero - offsets[i].y,
            kNegZero - offsets[i].z, kNegZero - offsets[i].w};
    }
}

int sample_offsets_gauss_blur5x5(std::uint32_t width, std::uint32_t height, Float4 offsets[13],
    Float4 weights[13], float multiplier) {
    const float inv_w = f(1.0 / unsigned_extent(width));
    const float inv_h = f(1.0 / unsigned_extent(height));
    const float inv_root = f(1.0 / f(std::sqrt(kTwoPi)));
    float total = 0.0f;
    int n = 0;
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            if (std::abs(x) + std::abs(y) > 2) continue;
            const float fx = static_cast<float>(x);
            const float fy = static_cast<float>(y);
            offsets[n] = {f(static_cast<double>(fx) * inv_w), f(static_cast<double>(fy) * inv_h), 0.0f, 0.0f};
            const float exponent = f((static_cast<double>(fy) * fy + static_cast<double>(fx) * fx) * -0.5);
            const float e = f(std::exp(static_cast<double>(exponent)));
            const float w = f(static_cast<double>(inv_root) * e);
            weights[n] = {w, w, w, w};
            total = f(static_cast<double>(total) + w);
            ++n;
        }
    }
    for (int i = 0; i < n; ++i) {
        float* c = &weights[i].x;
        for (int k = 0; k < 4; ++k) {
            c[k] = f(static_cast<double>(c[k]) / total);
            c[k] = f(static_cast<double>(c[k]) * multiplier);
        }
    }
    return n;
}

int fill_fullscreen_primitive(int vertex_count, FullscreenVertex out[4]) {
    if (vertex_count == 3) {
        out[0] = {-3.0f, -1.0f, 0.5f, 1.0f, -1.0f, 1.0f, 0.0f};
        out[1] = {-3.0f, 5.0f, 0.5f, 1.0f, -1.0f, -2.0f, 1.0f};
        out[2] = {3.0f, -1.0f, 0.5f, 1.0f, 2.0f, 1.0f, 2.0f};
        return 3;
    }
    out[0] = {-1.0f, -1.0f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f};
    out[1] = {-1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f};
    out[2] = {1.0f, -1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 2.0f};
    out[3] = {1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.0f, 3.0f};
    return 4;
}

// cc13-fixture2 follow-up: no static caller (a camera-block helper); modelled at the main
// thread's 24-bit precision control, each FMUL/FSUB/FDIV rounded to 24 significand bits.
void invert2x2(float m[4]) {
    const float m0 = m[0];
    const float det = f(sub24(mul24(m0, m[3]), mul24(m[1], m[2])));
    m[0] = f(div24(m[3], det));
    m[1] = f(div24(-static_cast<double>(m[1]), det));
    m[2] = f(div24(-static_cast<double>(m[2]), det));
    m[3] = f(div24(m0, det));
}

void orthonormalize_rows(float m[16]) {
    float* r0 = m;
    float* r1 = m + 4;
    float* r2 = m + 8;
    normalize3(r0, length3);
    const float d = f(static_cast<double>(r0[1]) * r1[1] + static_cast<double>(r0[0]) * r1[0] +
        static_cast<double>(r0[2]) * r1[2]);
    r1[0] = f(r1[0] - static_cast<double>(d) * r0[0]);
    r1[1] = f(r1[1] - static_cast<double>(d) * r0[1]);
    r1[2] = f(r1[2] - static_cast<double>(r0[2]) * d);
    normalize3(r1, length3);
    // 004F9B30: out = a x b with a = r0, b = r1.
    const float cx = f(static_cast<double>(r0[1]) * r1[2] - static_cast<double>(r0[2]) * r1[1]);
    const float cy = f(static_cast<double>(r1[0]) * r0[2] - static_cast<double>(r0[0]) * r1[2]);
    const float cz = f(static_cast<double>(r1[1]) * r0[0] - static_cast<double>(r0[1]) * r1[0]);
    r2[0] = cx; r2[1] = cy; r2[2] = cz;
    normalize3(r2, length3);
}

void compose_euler_scale_translation(float out[16], const float t[3], const float euler[3],
    const float scale[3]) {
    const float s2 = f(x87_sin(euler[2]));
    const float s1 = f(x87_sin(euler[1]));
    const float s0 = f(x87_sin(euler[0]));
    const float c2 = f(x87_cos(euler[2]));
    const float c1 = f(x87_cos(euler[1]));
    const float c0 = f(x87_cos(euler[0]));
    using D = double;
    // cc13-fixture2 (fixture-found): the image stores four pair products to float first
    // (00B6450F c2*c0, 00B64520 -c2*s0, 00B64530 s2*s0, 00B6453A -s2*c0) and three more
    // before their scale multiply (00B64569 c1*s2, 00B645C2 c1*c2, 00B645EE c1*s0,
    // 00B64610 c1*c0); the rest stays on the x87 stack. Follow-up: the callers (005ADDA0 HUD
    // map icons, 00ACBEC0 GUI model) run on the main thread, so every operation rounds to a
    // 24-bit significand (x87_*24).
    const float c2c0 = f(mul24(c2, c0)), nc2s0 = f(mul24(-D(c2), s0)), s2s0 = f(mul24(s2, s0)),
                ns2c0 = f(mul24(-D(s2), c0));
    out[3] = 0.0f;
    out[0] = f(mul24(sub24(c2c0, mul24(s1, s2s0)), scale[0]));
    out[1] = f(mul24(f(mul24(c1, s2)), scale[0]));
    out[2] = f(mul24(add24(mul24(s1, ns2c0), nc2s0), scale[0]));
    out[7] = 0.0f;
    out[4] = f(mul24(add24(mul24(s1, nc2s0), ns2c0), scale[1]));
    out[11] = 0.0f;
    out[15] = 1.0f;
    out[5] = f(mul24(f(mul24(c1, c2)), scale[1]));
    out[6] = f(mul24(sub24(s2s0, mul24(s1, c2c0)), scale[1]));
    out[8] = f(mul24(f(mul24(c1, s0)), scale[2]));
    out[9] = f(mul24(s1, scale[2]));
    out[10] = f(mul24(scale[2], f(mul24(c1, c0))));
    out[12] = t[0];
    out[13] = t[1];
    out[14] = t[2];
}

void frustum_planes(float planes[24], float vertical, float horizontal, float near_d, float far_d) {
    const float ch = f(x87_cos(horizontal));
    const float sh = f(x87_sin(horizontal));
    const float cv = f(x87_cos(vertical));
    const float sv = f(x87_sin(vertical));

    const float p[24] = {
        ch, 0.0f, sh, 0.0f,
        kNegZero - ch, 0.0f, sh, 0.0f,
        0.0f, kNegZero - cv, sv, 0.0f,
        0.0f, cv, sv, 0.0f,
        0.0f, 0.0f, 1.0f, near_d,
        0.0f, 0.0f, -1.0f, kNegZero - far_d,
    };
    for (int i = 0; i < 24; ++i) planes[i] = p[i];
}

// cc13-fixture2 follow-up: the image's callers (GUI ortho 00AA2020, shadow fits 00A8AEB0,
// 00A8BD20, 00A8ECB0) run on the main thread at 24-bit precision control, so the faithful
// model rounds each operation to a 24-bit significand. ortho_off_center_lh is live (the
// native GUI pass), so it keeps the earlier double model until kOrthoOffCenterPc24Cc13Bound
// is flipped; the fixture tests ortho_off_center_lh_pc24.
void ortho_off_center_lh_pc24(float out[16], float l, float r, float b, float t, float zn, float zf) {
    for (int i = 0; i < 16; ++i) out[i] = 0.0f;
    out[15] = 1.0f;
    out[0] = f(div24(2.0, sub24(r, l)));
    out[5] = f(div24(2.0, sub24(t, b)));
    out[10] = f(div24(1.0, sub24(zf, zn)));
    out[12] = f(div24(add24(l, r), sub24(l, r)));
    out[13] = f(div24(add24(b, t), sub24(b, t)));
    out[14] = f(div24(zn, sub24(zn, zf)));
}

void ortho_off_center_lh(float out[16], float l, float r, float b, float t, float zn, float zf) {
    if constexpr (kOrthoOffCenterPc24Cc13Bound) {
        ortho_off_center_lh_pc24(out, l, r, b, t, zn, zf);
    } else {
        for (int i = 0; i < 16; ++i) out[i] = 0.0f;
        out[15] = 1.0f;
        out[0] = f(2.0 / (static_cast<double>(r) - l));
        out[5] = f(2.0 / (static_cast<double>(t) - b));
        out[10] = f(1.0 / (static_cast<double>(zf) - zn));
        out[12] = f((static_cast<double>(l) + r) / (static_cast<double>(l) - r));
        out[13] = f((static_cast<double>(b) + t) / (static_cast<double>(b) - t));
        out[14] = f(static_cast<double>(zn) / (static_cast<double>(zn) - zf));
    }
}

std::uint32_t plane_set_classify_point(const PlaneRow* rows, int count, const float p[3]) {
    for (int i = 0; i < count; ++i) {
        const PlaneRow& q = rows[i];
        const double dist = static_cast<double>(q.c) * p[2] + static_cast<double>(q.b) * p[1] +
            static_cast<double>(q.a) * p[0];
        if (0.0 < dist - q.d) return kPlaneSetOutside;
    }
    return kPlaneSetInside;
}

std::uint32_t plane_set_classify_aabb(const PlaneRow* rows, int count, const float box[6]) {
    auto mag = [](float v) { return v <= 0.0f ? kNegZero - v : v; };
    for (int i = 0; i < count; ++i) {
        const PlaneRow& q = rows[i];
        const double center = static_cast<double>(q.c) * (static_cast<double>(box[2]) + box[5]) * 0.5 +
            static_cast<double>(q.a) * (static_cast<double>(box[0]) + box[3]) * 0.5 +
            static_cast<double>(q.b) * (static_cast<double>(box[1]) + box[4]) * 0.5;
        const double radius = static_cast<double>(mag(q.c)) * 0.5 * (static_cast<double>(box[5]) - box[2]) +
            static_cast<double>(mag(q.a)) * (static_cast<double>(box[3]) - box[0]) * 0.5 +
            static_cast<double>(mag(q.b)) * (static_cast<double>(box[4]) - box[1]) * 0.5;
        if (center - q.d < -radius) return kPlaneSetOutside;
    }
    return kPlaneSetInside;
}

int count_float4_registers(const std::vector<std::vector<ConstantDescRegisters>>& per_constant) {
    int total = 0;
    for (const auto& descs : per_constant)
        for (const auto& d : descs)
            if (d.register_set == 2) total += d.register_count;
    return total;
}

// ---------------------------------------------------------------------------
// .mshd tree

MshdCaps read_mshd_caps(const MshdTable& t) {
    MshdCaps caps;
    caps.min_vertex_shader = 0x101;
    caps.min_pixel_shader = 0x101;
    if (t.is_integer("MaxSamplers")) caps.max_samplers = t.get_integer("MaxSamplers");
    if (t.is_integer("MaxTextureRegisters")) caps.max_texture_registers = t.get_integer("MaxTextureRegisters");
    if (t.is_integer("MinVertexShaderVersion")) caps.min_vertex_shader = t.get_integer("MinVertexShaderVersion");
    if (t.is_integer("MinPixelShaderVersion")) caps.min_pixel_shader = t.get_integer("MinPixelShaderVersion");
    return caps;
}

ShaderConstantList read_mshd_constants(const MshdTable& list, const ShaderConstantLookup& find) {
    ShaderConstantList out;
    for (const auto& entry : list.entries()) {
        if (!entry.is_integer_key || !entry.value.table) continue;
        ShaderConstantRow row;  // a = 0, b = 0, count = 1, definition = null
        int position = 0;       // advances for every entry, integer key or not
        for (const auto& field : entry.value.table->entries()) {
            if (field.is_integer_key) {
                if (field.value.value_is_string) {
                    row.name = field.value.string_value;
                    row.definition = find ? find(row.name) : nullptr;
                    if (row.definition) {
                        row.dimension_8 = row.definition->dimension_8;
                        row.dimension_c = row.definition->dimension_c;
                        row.array_count = row.definition->array_count;
                    }
                } else if (field.value.value_is_integer) {
                    if (position == 1) row.dimension_8 = field.value.integer_value;
                    else if (position == 2) row.dimension_c = field.value.integer_value;
                    else if (position == 3) row.array_count = field.value.integer_value;
                }
            }
            ++position;
        }
        row.register_index = out.next_register;
        out.next_register += row.dimension_8 * row.array_count;
        out.rows.push_back(std::move(row));
    }
    return out;
}

MshdLayer read_mshd_layer(const MshdTable& t, const MshdLayerHooks& hooks) {
    MshdLayer layer;
    layer.name = t.get_string("Name");
    layer.render_queue = t.get_integer("RenderQueue");
    if (t.is_string("VertexFormat")) layer.vertex_format = t.get_string("VertexFormat");
    if (t.is_integer("Fog")) layer.fog = t.get_integer("Fog");
    if (t.is_integer("FogType")) layer.fog_type = t.get_integer("FogType");
    if (t.is_boolean("ReceiveShadows")) layer.receive_shadows = t.get_boolean("ReceiveShadows");
    if (t.is_boolean("DirectionalLightning")) layer.directional_lighting = t.get_boolean("DirectionalLightning");
    if (t.is_boolean("PointLightning")) layer.point_lighting = t.get_boolean("PointLightning");
    if (t.is_integer("DiffuseLight")) layer.diffuse_light = t.get_integer("DiffuseLight");
    if (t.is_integer("SpecularLight")) layer.specular_light = t.get_integer("SpecularLight");
    if (t.is_integer("ReservedColorRegisters"))
        layer.reserved_color_registers = t.get_integer("ReservedColorRegisters");
    if (t.is_integer("ReservedTextureRegisters"))
        layer.reserved_texture_registers = t.get_integer("ReservedTextureRegisters");
    if (t.is_boolean("PerPixelNormalize")) layer.per_pixel_normalize = t.get_boolean("PerPixelNormalize");
    // The state list is allocated either way; it is filled only from a table.
    if (t.is_table("RenderStates")) {
        layer.has_render_states = true;
        if (hooks.read_render_states) hooks.read_render_states(*t.get_table("RenderStates"));
    }
    if (t.is_table("Samplers")) {
        for (const auto& e : t.get_table("Samplers")->entries()) {
            if (!e.is_integer_key) continue;
            if (hooks.read_sampler && e.value.table) hooks.read_sampler(*e.value.table);
            ++layer.sampler_count;
        }
    }
    layer.vs_target = t.get_string("VS_Target");
    if (t.is_table("VS_Constants"))
        layer.vs_constants = read_mshd_constants(*t.get_table("VS_Constants"), hooks.find_constant);
    layer.vs_body = t.get_string("VS_Body");
    layer.ps_target = t.get_string("PS_Target");
    if (t.is_table("PS_Constants"))
        layer.ps_constants = read_mshd_constants(*t.get_table("PS_Constants"), hooks.find_constant);
    if (t.is_integer("PS_Code")) layer.ps_code = t.get_integer("PS_Code");
    layer.ps_body = t.get_string("PS_Body");
    return layer;
}

MshdRenderMode read_mshd_render_mode(const MshdTable& t, const MshdLayerHooks& hooks) {
    MshdRenderMode mode;
    mode.name = t.get_string("Name");
    mode.render_mode = t.get_integer("RenderMode");
    for (const auto& e : t.entries())
        if (e.is_integer_key && e.value.table) mode.layers.push_back(read_mshd_layer(*e.value.table, hooks));
    return mode;
}

MshdVersion read_mshd_version(const MshdTable& t, const MshdLayerHooks& hooks) {
    MshdVersion version;
    version.name = t.get_string("Name");
    if (t.is_table("Caps")) version.caps = read_mshd_caps(*t.get_table("Caps"));
    for (const auto& e : t.entries())
        if (e.is_integer_key && e.value.table)
            version.render_modes.push_back(read_mshd_render_mode(*e.value.table, hooks));
    return version;
}

MshdLodPhase read_mshd_lod_phase(const MshdTable& t, const MshdLayerHooks& hooks) {
    MshdLodPhase phase;
    phase.lod_value = t.is_nil("LodValue") ? 0.0f : f(t.get_number("LodValue"));
    for (const auto& e : t.entries())
        if (e.is_integer_key && e.value.table) phase.versions.push_back(read_mshd_version(*e.value.table, hooks));
    return phase;
}

MshdShader read_mshd_shader(const std::string& name, const MshdTable& shader_global,
    const MshdLayerHooks& hooks) {
    MshdShader shader;
    shader.name = name;
    int index = 0;
    for (const auto& e : shader_global.entries()) {
        if (!e.is_integer_key || !e.value.table) continue;
        MshdLodPhase phase = read_mshd_lod_phase(*e.value.table, hooks);
        phase.index = index++;
        shader.lod_phases.push_back(std::move(phase));
    }
    return shader;
}

int find_named_index(const std::vector<NamedIndexRecord>& records, const std::string& name) {
    for (const auto& r : records) {
        if (r.name.size() != name.size()) continue;
        bool equal = true;
        for (std::size_t i = 0; i < name.size() && equal; ++i)
            equal = std::tolower(static_cast<unsigned char>(name[i])) ==
                std::tolower(static_cast<unsigned char>(r.name[i]));
        if (equal) return r.value;
    }
    return 0x37;
}

// ---------------------------------------------------------------------------
// Segment 91

MitBlockPool::Slot MitBlockPool::allocate() {
    if (current_ == -1) {
        current_ = static_cast<int>(blocks_.size());
        Block b;
        for (int i = 0; i < kSlots; ++i) b.stack[i] = static_cast<std::uint16_t>(kSlots - 1 - i);
        b.count = kSlots;
        b.id = next_id_++;
        blocks_.push_back(b);
    }
    Block& b = blocks_[current_];
    --b.count;
    const Slot s{current_, b.stack[b.count]};
    if (b.count == 0) {
        const int from = current_ + 1;
        current_ = -1;
        for (int i = from; i < static_cast<int>(blocks_.size()); ++i) {
            if (blocks_[i].count != 0) {
                current_ = i;
                break;
            }
        }
    }
    return s;
}

void MitBlockPool::free(Slot s) {
    Block& b = blocks_.at(s.block);
    b.stack[b.count] = static_cast<std::uint16_t>(s.slot);
    ++b.count;
    if (static_cast<std::uint32_t>(s.block) < static_cast<std::uint32_t>(current_)) current_ = s.block;
}

void MitBlockPool::trim() {
    for (std::size_t i = 0; i < blocks_.size(); ++i) {
        if (blocks_[i].count != kSlots) continue;
        blocks_[i] = blocks_.back();  // slots of the moved block are renumbered to i
        blocks_.pop_back();
        --i;  // the image decrements EDI and re-tests the same index
    }
    current_ = -1;
    for (std::size_t i = 0; i < blocks_.size(); ++i) {
        if (blocks_[i].count != 0) {
            current_ = static_cast<int>(i);
            break;
        }
    }
}

float flare_centre_weight(float x, float y, float visibility) {
    if (!(visibility > 0.0f)) return 0.0f;
    const float sum = f(static_cast<double>(y) * y + static_cast<double>(x) * x);
    const float r = f(std::sqrt(static_cast<double>(sum)));
    return f((1.4142136573791504 - r) * 0.20000000298023224 + 0.05000000074505806);
}

float flare_edge_fade(float x, float y, float visibility, float limit, bool limit_flag) {
    // Unordered compares fall through like the image's COMISS/FCOMI branches.
    constexpr double lo = -0.9399999976158142, hi = 0.9399999976158142, width = 0.06000000238418579;
    if (!(0.0f < visibility) && !std::isnan(visibility)) return 0.0f;
    if (!(visibility <= limit) && !std::isnan(visibility) && !std::isnan(limit) && limit_flag) return 0.0f;
    if (!(x > -1.0f) && !std::isnan(x)) return 0.0f;
    float fade = 1.0f;
    if (lo > x) fade = f((x + hi) / width + 1.0);
    if (!(y > -1.0f) && !std::isnan(y)) return 0.0f;
    if (lo > y) fade = f(((y + hi) / width + 1.0) * fade);
    if (!(x < 1.0f) && !std::isnan(x)) return 0.0f;
    if (x > hi) fade = f((1.0 - (x - hi) / width) * fade);
    if (!(y < 1.0f) && !std::isnan(y)) return 0.0f;
    if (y > hi) fade = f((1.0 - (y - hi) / width) * fade);
    return fade;
}

bool flare_pair_test(float a_1c0, float b_1c4) {
    if (a_1c0 == 0.0f) return false;
    return b_1c4 >= a_1c0;
}

void flare_place_element(float owner_base, const float origin[2], const float dir[2], float t,
    float out[2]) {
    const float k = f(std::log(static_cast<double>(owner_base)));
    const float e = f(std::exp(static_cast<double>(f(static_cast<double>(k) * t))));
    out[0] = f(origin[0] + static_cast<double>(f(static_cast<double>(dir[0]) * e)));
    out[1] = f(origin[1] + static_cast<double>(f(static_cast<double>(e) * dir[1])));
}

void scale_color32(const std::uint8_t in[4], std::uint8_t out[4], float scale) {
    for (int i = 0; i < 4; ++i) {
        float v = f(static_cast<double>(scale) * in[i]);
        if (!(0.0f <= v)) v = 0.0f;
        else if (255.0 < v) v = 255.0f;
        out[i] = static_cast<std::uint8_t>(static_cast<int>(std::nearbyint(v)));
    }
}

void disc_fan_indices(std::uint32_t rim_count, std::vector<std::uint16_t>& out) {
    out.clear();
    for (std::uint32_t i = 0; i < rim_count; ++i) {
        out.push_back(0);
        out.push_back(static_cast<std::uint16_t>(i + 1));
        out.push_back(static_cast<std::uint16_t>((i + 1) % rim_count + 1));
    }
}

namespace {

void put_f32(unsigned char* at, float value) { std::memcpy(at, &value, 4); }
void put_u32(unsigned char* at, std::uint32_t value) { std::memcpy(at, &value, 4); }

} // namespace

void write_disc_fan_00b8b980(FlareDiscFanHost& host, const float radius_xy[2],
    std::uint32_t rim_count) {
    // 00B8B995..00B8B9B3. With the precision control at 24 bits the division
    // rounds once to float; 2pi at 00CE3828 is exactly 6.2831855f. A 53-bit
    // control would round the quotient twice (double, then the FSTP float).
    const float step = static_cast<float>(6.2831853071795862) / static_cast<float>(rim_count);
    float angle = 0.0f;

    unsigned char* vertex = host.lock_vertex_stream0_00b73260();
    // 00B8B9CD..00B8BA0B: the centre.
    put_f32(vertex + 0x00, 0.0f);
    put_f32(vertex + 0x04, 0.0f);
    put_f32(vertex + 0x08, 0.0f);
    put_f32(vertex + 0x0c, 1.0f);
    put_u32(vertex + 0x10, 0x00ffffffu);
    vertex += 0x28;
    // 00B8BA50..00B8BAD7, rim_count times (00B8BA17 skips a zero count).
    for (std::uint32_t i = 0; i < rim_count; ++i) {
        // FSIN / FCOS take the float angle; each result is stored to float
        // before the product (00B8BA56, 00B8BA68).
        const float sine = static_cast<float>(std::sin(static_cast<double>(angle)));
        const float cosine = static_cast<float>(std::cos(static_cast<double>(angle)));
        put_u32(vertex + 0x10, 0x00ffffffu);
        put_f32(vertex + 0x00, radius_xy[0] * cosine);
        put_f32(vertex + 0x04, radius_xy[1] * sine);
        put_f32(vertex + 0x08, 0.999f); // 00D63400, 3F7FBE77h
        put_f32(vertex + 0x0c, 1.0f);
        angle = step + angle; // 00B8BAAC..00B8BAC5
        put_f32(vertex + 0x18, 0.5f); // 00CE3800
        put_f32(vertex + 0x1c, 0.5f);
        vertex += 0x28;
    }
    host.unlock_vertex_stream0(); // 00B8BADE..00B8BAE5

    // 00B8BAE7..00B8BB3E: triangle i = (0, i+1, (i+1) % n + 1), DIV unsigned.
    std::uint16_t* index = host.lock_index_buffer_60();
    for (std::uint32_t i = 0; i < rim_count; ++i) {
        index[0] = 0;
        index[1] = static_cast<std::uint16_t>(i + 1);
        index[2] = static_cast<std::uint16_t>((i + 1) % rim_count + 1);
        index += 3;
    }
    host.unlock_index_buffer_60();
}

} // namespace bsp::cc13fx
