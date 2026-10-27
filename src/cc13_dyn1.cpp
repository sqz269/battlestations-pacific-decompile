// cc13_dyn1: reconstructions in the in-house Dyn block [00C30930,00C5DF60).
// See include/bsp/cc13_dyn1.hpp and docs/CC13_DYN1.md. Nothing here is wired.
//
// x87 note. The image computes in x87 registers at the MSVC default 53-bit precision and
// rounds to float only at each FSTP float. The C++ below computes those intermediate
// expressions in double and narrows exactly where the image stores a float, so the
// stored values match bit for bit when the inputs are finite.

#include "bsp/cc13_dyn1.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/dyn_body_creation.hpp"
#include "bsp/dyn_profile_scopes.hpp"
#include "bsp/native_dyn_narrow_phase.hpp"

#include <cmath>
#include <cstring>

namespace bsp::cc13_dyn1 {

// ---------------------------------------------------------------- body accessors

void* body_first_shape_00c31dc0(void* body) noexcept { return raw(body).ptr(0x70); }

void* body_owner_00c31ea0(void* body) noexcept { return raw(body).ptr(0x6c); }

void body_angular_velocity_00c31f20(void* body, float out[3]) noexcept {
    const Raw m = raw(body).deref(0x4);  // 00C31F20 MOV EAX,[ECX+4]
    out[0] = m.at<float>(0x0c);
    out[1] = m.at<float>(0x10);
    out[2] = m.at<float>(0x14);
}

void body_linear_velocity_00c31f40(void* body, float out[3]) noexcept {
    const Raw m = raw(body).deref(0x4);
    out[0] = m.at<float>(0x00);
    out[1] = m.at<float>(0x04);
    out[2] = m.at<float>(0x08);
}

void body_set_box_00c31f60(void* body, const float lo[3], const float hi[3]) noexcept {
    const Raw b = raw(body);
    b.at<float>(0x38) = lo[0];
    b.at<float>(0x3c) = lo[1];
    b.at<float>(0x40) = lo[2];
    b.at<float>(0x44) = hi[0];
    b.at<float>(0x48) = hi[1];
    b.at<float>(0x4c) = hi[2];
}

void body_box_00c31f90(const void* body, float lo_out[3], float hi_out[3]) noexcept {
    const Raw b = raw(body);
    lo_out[0] = b.at<float>(0x38);
    lo_out[1] = b.at<float>(0x3c);
    lo_out[2] = b.at<float>(0x40);
    hi_out[0] = b.at<float>(0x44);
    hi_out[1] = b.at<float>(0x48);
    hi_out[2] = b.at<float>(0x4c);
}

float* body_position_00c35300(void* body) noexcept {
    return reinterpret_cast<float*>(raw(body).field(0x2c));
}

void body_inertia_00c37f10(const void* body, float out[3]) noexcept {
    // FLD1 then three FDIVs by the float M+54h/+58h/+5Ch, each stored as float. No zero
    // guard: a zero inverse inertia gives +inf, as the image.
    const Raw m = raw(body).deref(0x4);
    out[0] = static_cast<float>(1.0 / m.at<float>(0x54));
    out[1] = static_cast<float>(1.0 / m.at<float>(0x58));
    out[2] = static_cast<float>(1.0 / m.at<float>(0x5c));
}

void body_add_force_at_point_00c37f60(void* body, const float force[3],
                                      const float point[3]) noexcept {
    const Raw b = raw(body);
    const Raw m = b.deref(0x4);
    using namespace cc13x87;
    // Each FADD/FSUB below rounds to a 24-bit significand before its FSTP (x87_*24); a
    // float sum taken through double can round twice when the exponents are far apart.
    const auto add = [](float a, float c) { return static_cast<float>(x87_add24(a, c)); };
    const auto sub = [](float a, float c) { return static_cast<float>(x87_sub24(a, c)); };
    // 00C37F66..00C37F83: M+38h[i] = float(M+38h[i] + force[i]), i = 0, 1, 2.
    m.at<float>(0x38) = add(m.at<float>(0x38), force[0]);
    m.at<float>(0x3c) = add(m.at<float>(0x3c), force[1]);
    m.at<float>(0x40) = add(m.at<float>(0x40), force[2]);
    // 00C37F8A..00C37FA8: the lever r = point - B+2Ch, each component stored as float.
    const float r0 = sub(point[0], b.at<float>(0x2c));
    const float r1 = sub(point[1], b.at<float>(0x30));
    const float r2 = sub(point[2], b.at<float>(0x34));
    // The motion pointer is reloaded at 00C37F9F, after the force stores.
    const Raw m2 = b.deref(0x4);
    // r x force, each component formed in registers and stored as float ([ESP+0Ch..14h]).
    // 00C37FBA..00C38003 keep both products on the x87 stack (FMULP) and subtract them
    // (FSUBRP). The caller 0092B350 is fixed-step slot +8h (wave 3), run by the frame-job
    // pool on the main thread at 24-bit precision control, or on a pool worker at 53 bits
    // (docs/CC13_FPU_PRECISION.md). Modelled at 24 bits, as the main thread and the
    // single-thread mode run it: each FMULP and the FSUBRP round to a 24-bit significand
    // with the stack's exponent range (cc13-fixture3: rounding the products to float and
    // the difference through double missed the double-rounding and overflow cases).
    // At 53 bits the products would be exact and the difference rounded once.
    const auto cross = [](float a, float b, float c, float d) {
        return static_cast<float>(x87_sub24(x87_mul24(a, b), x87_mul24(c, d)));
    };
    const float t0 = cross(r1, force[2], r2, force[1]);  // 00C37FC6/00C37FD4/00C37FD8
    const float t1 = cross(force[0], r2, r0, force[2]);  // 00C37FEA/00C37FF1/00C37FF5
    const float t2 = cross(force[1], r0, r1, force[0]);  // 00C37FFD/00C37FFF/00C38001
    m2.at<float>(0x44) = add(m2.at<float>(0x44), t0);
    m2.at<float>(0x48) = add(m2.at<float>(0x48), t1);
    m2.at<float>(0x4c) = add(m2.at<float>(0x4c), t2);
    b.at<std::uint32_t>(0x50) &= 0xffffffedu;  // the wake: clear bits 1 and 4
}

namespace {
// 00401170 BSP_Math_AbsFloat: FLD float, FABS, rounded through float.
float abs_float_00401170(float v) noexcept { return std::fabs(v); }
}  // namespace

namespace {
constexpr double kHalf_00d7a280 = 0.5;              // the double at 00D7A280
constexpr double kBoundsEpsilon_00d7a2f8 = 0.02f;   // the double at 00D7A2F8 (0.02f widened)

// The conservative box rotation the image inlines in 00C56CF0, 00C57880 and 00C57FF0:
// centre and half extent of {lo, hi}, the centre through the 3x4 row matrix m (rows
// m[0..2], m[3..5], m[6..8], translation m[9..11]) and the half extent through |m|.
// Every value the image stores as float is narrowed at the same point here.
void rotate_box(const float lo[3], const float hi[3], const float m[12], float out_min[3],
                float out_max[3]) noexcept {
    float c[3];
    float h[3];
    for (int i = 0; i < 3; ++i) {
        const float sum = static_cast<float>(static_cast<double>(lo[i]) + hi[i]);
        c[i] = static_cast<float>(sum * kHalf_00d7a280);
    }
    for (int i = 0; i < 3; ++i) {
        const float d = static_cast<float>(static_cast<double>(hi[i]) - lo[i]);
        h[i] = static_cast<float>(d * kHalf_00d7a280);
    }
    // ((c0*m[i] + c1*m[3+i]) + c2*m[6+i]) + m[9+i], one float store each.
    float wc[3];
    for (int i = 0; i < 3; ++i) {
        wc[i] = static_cast<float>(static_cast<double>(c[0]) * m[i] +
                                   static_cast<double>(c[1]) * m[3 + i] +
                                   static_cast<double>(c[2]) * m[6 + i] + m[9 + i]);
    }
    // Nine 00401170 calls in element order m0..m8, then
    // (h1*|m[3+i]| + h0*|m[i]|) + h2*|m[6+i]|, one float store each.
    float a[9];
    for (int i = 0; i < 9; ++i) a[i] = abs_float_00401170(m[i]);
    float we[3];
    for (int i = 0; i < 3; ++i) {
        we[i] = static_cast<float>(static_cast<double>(h[1]) * a[3 + i] +
                                   static_cast<double>(h[0]) * a[i] +
                                   static_cast<double>(h[2]) * a[6 + i]);
    }
    for (int i = 0; i < 3; ++i)
        out_min[i] = static_cast<float>(static_cast<double>(wc[i]) - we[i]);
    for (int i = 0; i < 3; ++i)
        out_max[i] = static_cast<float>(static_cast<double>(wc[i]) + we[i]);
}

// 00C55FC0 with ESI = body: the existing reconstruction in src/dyn_body_creation.cpp.
void recompute_body_bounds(void* body) {
    dyn_body_recompute_bounds_00c55fc0(*static_cast<DynBodyStorage*>(body));
}
}  // namespace

void body_set_transform_00c56cf0(void* body, const float xf[12], Cc13Dyn1Calls& calls) {
    const Raw b = raw(body);
    auto* m_words = reinterpret_cast<std::uint32_t*>(b.field(0x08));
    for (int i = 0; i < 12; ++i) m_words[i] = reinterpret_cast<const std::uint32_t*>(xf)[i];
    const std::uint32_t flags = b.at<std::uint32_t>(0x50);
    if (((flags >> 3) & 1u) != 0 && (b.at<std::uint8_t>(0x50) & 1u) != 0) {
        const float lo[3] = {b.at<float>(0x38), b.at<float>(0x3c), b.at<float>(0x40)};
        const float hi[3] = {b.at<float>(0x44), b.at<float>(0x48), b.at<float>(0x4c)};
        float* proxy_box = reinterpret_cast<float*>(b.deref(0x60).field(0x4));  // ESI
        void* manager_owner = b.deref(0x0).ptr(0x444);                         // EDI
        rotate_box(lo, hi, reinterpret_cast<const float*>(b.field(0x08)), proxy_box,
                   proxy_box + 3);
        calls.broadphase_update_proxy_slot2(raw(manager_owner).ptr(0xac), b.ptr(0x60));
        return;
    }
    auto* pending = reinterpret_cast<std::uint32_t*>(b.deref(0x4).field(0x84));
    for (int i = 0; i < 12; ++i) pending[i] = reinterpret_cast<const std::uint32_t*>(xf)[i];
}

// ------------------------------------------------------------- primitive shapes

void box_shape_bounds_00c57880(void* shape) {
    const Raw s = raw(shape);
    float lo[3];
    float hi[3];
    for (int i = 0; i < 3; ++i) {
        const float he = s.at<float>(0x210 + 4 * i);
        hi[i] = static_cast<float>(he + kBoundsEpsilon_00d7a2f8);
        const float neg = -he;  // FCHS, stored as float
        lo[i] = static_cast<float>(neg - kBoundsEpsilon_00d7a2f8);
    }
    rotate_box(lo, hi, reinterpret_cast<const float*>(s.field(0x34)),
               reinterpret_cast<float*>(s.field(0x0c)), reinterpret_cast<float*>(s.field(0x18)));
    recompute_body_bounds(s.ptr(0x4));
}

void cylinder_shape_bounds_00c57ff0(void* shape) {
    const Raw s = raw(shape);
    // x and z take the radius S+214h, y the half height S+210h.
    const float e[3] = {s.at<float>(0x214), s.at<float>(0x210), s.at<float>(0x214)};
    float lo[3];
    float hi[3];
    for (int i = 0; i < 3; ++i) hi[i] = static_cast<float>(e[i] + kBoundsEpsilon_00d7a2f8);
    for (int i = 0; i < 3; ++i) lo[i] = -hi[i];  // FLD hi; FCHS; FSTP float
    rotate_box(lo, hi, reinterpret_cast<const float*>(s.field(0x34)),
               reinterpret_cast<float*>(s.field(0x0c)), reinterpret_cast<float*>(s.field(0x18)));
    recompute_body_bounds(s.ptr(0x4));
}

void sphere_shape_bounds_00c58520(void* shape) {
    const Raw s = raw(shape);
    void* body = s.ptr(0x4);  // MOV ESI,[ECX+4] before the arithmetic
    const float r = static_cast<float>(s.at<float>(0x210) + kBoundsEpsilon_00d7a2f8);
    for (int i = 0; i < 3; ++i) {
        s.at<float>(0x0c + 4 * i) =
            static_cast<float>(static_cast<double>(s.at<float>(0x58 + 4 * i)) - r);
    }
    for (int i = 0; i < 3; ++i) {
        s.at<float>(0x18 + 4 * i) =
            static_cast<float>(static_cast<double>(s.at<float>(0x58 + 4 * i)) + r);
    }
    recompute_body_bounds(body);
}

namespace {
// One height sample of the terrain shape as 00C58710..00C5876C reads it: by the sample
// type S+22Ch, a float array, a uint16 array (FFFFh is the hole value, the -1000.0f at
// 00D7A240, else u * S+224h + S+228h through FILD), or 0.0 for any other type.
float terrain_sample(Raw s, std::uint32_t type, std::int32_t i) noexcept {
    if (type == 0) return static_cast<const float*>(s.ptr(0x210))[i];
    if (type == 1) {
        const std::uint16_t u = static_cast<const std::uint16_t*>(s.ptr(0x210))[i];
        if (u == 0xffffu) return -1000.0f;
        return static_cast<float>(static_cast<double>(u) * s.at<float>(0x224) +
                                  s.at<float>(0x228));
    }
    return 0.0f;
}
}  // namespace

void terrain_shape_bounds_00c58690(void* shape) {
    const Raw s = raw(shape);
    const std::int32_t nx = s.at<std::int32_t>(0x214);  // ESI
    s.at<float>(0x18) = static_cast<float>(static_cast<double>(nx - 1) * s.at<float>(0x21c));
    const std::int32_t count = s.at<std::int32_t>(0x218) * nx;
    s.at<float>(0x10) = 3.402823466e+38f;   // FLT_MAX at 00D7A248
    s.at<float>(0x0c) = 0.0f;
    s.at<float>(0x14) = 0.0f;
    s.at<float>(0x1c) = -3.402823466e+38f;  // -FLT_MAX at 00D7A244
    s.at<float>(0x20) = static_cast<float>(
        static_cast<double>(s.at<std::int32_t>(0x218) - 1) * s.at<float>(0x220));
    if (count > 0) {  // JLE skips the loop
        const std::uint32_t type = s.at<std::uint32_t>(0x22c);
        std::int32_t i = 0;
        do {
            // Minimum: keep the current value only when the sample is above it (ordered).
            const float lo_sample = terrain_sample(s, type, i);
            const float lo_cur = s.at<float>(0x10);
            s.at<float>(0x10) = (lo_sample > lo_cur) ? lo_cur : lo_sample;
            // Maximum: keep the current value only when it is above the sample (ordered).
            const float hi_sample = terrain_sample(s, type, i);
            const float hi_cur = s.at<float>(0x1c);
            s.at<float>(0x1c) = (hi_cur > hi_sample) ? hi_cur : hi_sample;
            ++i;
        } while (i < s.at<std::int32_t>(0x218) * s.at<std::int32_t>(0x214));
    }
    recompute_body_bounds(s.ptr(0x4));
}

namespace {
// The Dyn::Shape base constructor the primitive constructors inline: base vtable,
// S+204h = 0, S+4h = body, S+8h = desc+10h (type), S+24h/+28h = desc+0/+4 (floats through
// FLD/FSTP), S+2Ch/+30h = desc+8/+0Ch (group, mask), S+208h = S+20Ch = 0.
void construct_shape_base(Raw s, void* body, Raw d, std::uint32_t base_vtable) {
    s.at<std::uint32_t>(0x0) = base_vtable;
    s.at<std::uint32_t>(0x204) = 0;
    s.ptr(0x4) = body;
    s.at<std::uint32_t>(0x8) = d.at<std::uint32_t>(0x10);
    s.at<float>(0x24) = d.at<float>(0x0);
    s.at<float>(0x28) = d.at<float>(0x4);
    s.at<std::uint32_t>(0x2c) = d.at<std::uint32_t>(0x8);
    s.at<std::uint32_t>(0x30) = d.at<std::uint32_t>(0xc);
    s.at<std::uint32_t>(0x208) = 0;
    s.at<std::uint32_t>(0x20c) = 0;
}
}  // namespace

void* box_shape_construct_00c57b90(void* body, void* shape, const void* desc,
                                   std::uint32_t base_vtable, std::uint32_t box_vtable) {
    const Raw s = raw(shape);
    const Raw d = raw(desc);
    construct_shape_base(s, body, d, base_vtable);
    s.at<std::uint32_t>(0x0) = box_vtable;
    auto* xf = reinterpret_cast<std::uint32_t*>(s.field(0x34));
    for (int i = 0; i < 12; ++i) xf[i] = d.at<std::uint32_t>(0x14 + 4 * i);  // REP MOVSD
    s.at<std::uint32_t>(0x210) = d.at<std::uint32_t>(0x44);
    s.at<std::uint32_t>(0x214) = d.at<std::uint32_t>(0x48);
    s.at<std::uint32_t>(0x218) = d.at<std::uint32_t>(0x4c);
    box_shape_bounds_00c57880(shape);  // direct CALL, not through the vtable
    return shape;
}

namespace {
// 004011D0 BSP_Math_SqrtFloat: stack float, CRT sqrt at 00BF7030 in double, FSTP float.
float sqrt_float_004011d0(float v) noexcept {
    return static_cast<float>(std::sqrt(static_cast<double>(v)));
}
// The squared length as 00C582E0 forms it twice: (y*y + x*x) + z*z, stored as float.
float length_squared(float x, float y, float z) noexcept {
    return static_cast<float>(static_cast<double>(y) * y + static_cast<double>(x) * x +
                              static_cast<double>(z) * z);
}
}  // namespace

void* cylinder_shape_construct_00c582e0(const void* desc, void* shape, void* body,
                                        std::uint32_t base_vtable,
                                        std::uint32_t cylinder_vtable) {
    const Raw s = raw(shape);
    const Raw d = raw(desc);
    construct_shape_base(s, body, d, base_vtable);
    s.at<std::uint32_t>(0x0) = cylinder_vtable;
    // Row 1 (S+40h..48h) is the axis desc+20h..28h, normalised.
    s.at<std::uint32_t>(0x40) = d.at<std::uint32_t>(0x20);
    s.at<std::uint32_t>(0x44) = d.at<std::uint32_t>(0x24);
    s.at<std::uint32_t>(0x48) = d.at<std::uint32_t>(0x28);
    {
        const float ax = s.at<float>(0x40);
        const float ay = s.at<float>(0x44);
        const float az = s.at<float>(0x48);
        const float len = sqrt_float_004011d0(length_squared(ax, ay, az));
        s.at<float>(0x40) = static_cast<float>(static_cast<double>(ax) / len);
        s.at<float>(0x44) = static_cast<float>(static_cast<double>(ay) / len);
        s.at<float>(0x48) = static_cast<float>(static_cast<double>(az) / len);
    }
    const float nx = s.at<float>(0x40);
    const float ny = s.at<float>(0x44);
    const float nz = s.at<float>(0x48);
    // Row 0 = n x (0, 0, 1), with the image's literal multiplications by 0.0 (FLDZ; they
    // matter for an infinite or NaN axis), then normalised.
    s.at<float>(0x34) = static_cast<float>(static_cast<double>(ny) - static_cast<double>(nz) * 0.0);
    s.at<float>(0x38) = static_cast<float>(static_cast<double>(nz) * 0.0 - nx);
    s.at<float>(0x3c) = static_cast<float>(static_cast<double>(nx) * 0.0 -
                                           static_cast<double>(ny) * 0.0);
    {
        const float len = sqrt_float_004011d0(
            length_squared(s.at<float>(0x34), s.at<float>(0x38), s.at<float>(0x3c)));
        s.at<float>(0x34) = static_cast<float>(static_cast<double>(s.at<float>(0x34)) / len);
        s.at<float>(0x38) = static_cast<float>(static_cast<double>(s.at<float>(0x38)) / len);
        s.at<float>(0x3c) = static_cast<float>(static_cast<double>(s.at<float>(0x3c)) / len);
    }
    const float r0x = s.at<float>(0x34);
    const float r0y = s.at<float>(0x38);
    const float r0z = s.at<float>(0x3c);
    // Row 2 = row 0 x row 1.
    s.at<float>(0x4c) = static_cast<float>(static_cast<double>(nz) * r0y - static_cast<double>(ny) * r0z);
    s.at<float>(0x50) = static_cast<float>(static_cast<double>(nx) * r0z - static_cast<double>(r0x) * nz);
    s.at<float>(0x54) = static_cast<float>(static_cast<double>(ny) * r0x - static_cast<double>(nx) * r0y);
    s.at<std::uint32_t>(0x58) = d.at<std::uint32_t>(0x14);
    s.at<std::uint32_t>(0x5c) = d.at<std::uint32_t>(0x18);
    s.at<std::uint32_t>(0x60) = d.at<std::uint32_t>(0x1c);
    s.at<float>(0x214) = d.at<float>(0x30);  // radius
    s.at<float>(0x210) = d.at<float>(0x2c);  // half height
    cylinder_shape_bounds_00c57ff0(shape);
    return shape;
}

namespace {
// The sign choice every support routine makes: FCOMIP dot,0 then JBE to the -1 arm, so
// a dot that is zero or unordered picks -1 (the float 00D7A260 / the double 00D7A250).
double support_sign(double dot) noexcept { return dot > 0.0 ? 1.0 : -1.0; }
}  // namespace

void box_shape_support_00c47aa0(const void* shape, float out[3], const float dir[3],
                                const float xf[12]) noexcept {
    const Raw s = raw(shape);
    const double d0 = dir[0];
    const double d1 = dir[1];
    const double d2 = dir[2];
    // Each dot is (d0*row[0] + d1*row[1]) + d2*row[2], stored as float before the compare.
    auto dot = [&](int r) {
        return static_cast<float>(d0 * xf[3 * r] + d1 * xf[3 * r + 1] + d2 * xf[3 * r + 2]);
    };
    // Rows in the image's order: 2, 1, 0. Each extent and each scaled row is a float store.
    const float e2 = static_cast<float>(s.at<float>(0x218) * support_sign(dot(2)));
    float t2[3];
    for (int i = 0; i < 3; ++i) t2[i] = static_cast<float>(static_cast<double>(xf[6 + i]) * e2);
    const float e1 = static_cast<float>(s.at<float>(0x214) * support_sign(dot(1)));
    float t1[3];
    for (int i = 0; i < 3; ++i) t1[i] = static_cast<float>(static_cast<double>(xf[3 + i]) * e1);
    const float e0 = static_cast<float>(s.at<float>(0x210) * support_sign(dot(0)));
    float t0[3];
    for (int i = 0; i < 3; ++i) t0[i] = static_cast<float>(static_cast<double>(xf[i]) * e0);
    // ((t0 + translation) + t1) + t2, a float store after every add.
    for (int i = 0; i < 3; ++i) {
        const float a = static_cast<float>(static_cast<double>(t0[i]) + xf[9 + i]);
        const float b = static_cast<float>(static_cast<double>(a) + t1[i]);
        out[i] = static_cast<float>(static_cast<double>(b) + t2[i]);
    }
}

void box_shape_support_00c38470(const void* shape, double out[3], const double dir[3],
                                const float xf[12]) noexcept {
    const Raw s = raw(shape);
    // The same construction in double: nothing is narrowed, the scaled rows 1 and 2 are
    // spilled as doubles and the extents stay in registers.
    auto dot = [&](int r) {
        return dir[0] * xf[3 * r] + dir[1] * xf[3 * r + 1] + dir[2] * xf[3 * r + 2];
    };
    const double e2 = support_sign(dot(2)) * s.at<float>(0x218);
    double t2[3];
    for (int i = 0; i < 3; ++i) t2[i] = static_cast<double>(xf[6 + i]) * e2;
    const double e1 = support_sign(dot(1)) * s.at<float>(0x214);
    double t1[3];
    for (int i = 0; i < 3; ++i) t1[i] = static_cast<double>(xf[3 + i]) * e1;
    const double e0 = support_sign(dot(0)) * s.at<float>(0x210);
    for (int i = 0; i < 3; ++i) {
        const double t0 = static_cast<double>(xf[i]) * e0;
        out[i] = ((t0 + xf[9 + i]) + t1[i]) + t2[i];
    }
}

void sphere_shape_support_00c48080(const void* shape, float out[3], const float dir[3],
                                   const float xf[12]) noexcept {
    // Slot 2 has no call site in the image (only the vtable entry refers to it), so it is
    // judged at the main thread's 24-bit precision control, as for 00C47AA0: each FMUL and
    // FADD rounds to 24 bits with the extended exponent before its FSTP (cc13-fixture7).
    const float r = raw(shape).at<float>(0x210);  // FSTP float [ESP]
    float t[3];
    for (int i = 0; i < 3; ++i) t[i] = static_cast<float>(cc13x87::x87_mul24(dir[i], r));
    for (int i = 0; i < 3; ++i) out[i] = static_cast<float>(cc13x87::x87_add24(xf[9 + i], t[i]));
}

void sphere_shape_support_00c38970(const void* shape, double out[3], const double dir[3],
                                   const float xf[12]) noexcept {
    const double r = raw(shape).at<float>(0x210);
    for (int i = 0; i < 3; ++i) out[i] = xf[9 + i] + dir[i] * r;
}

void cylinder_shape_support_00c47ca0(const void* shape, float out[3], const float dir[3],
                                     const float xf[12]) noexcept {
    const Raw s = raw(shape);
    const double dx = dir[0];
    const double dy = dir[1];
    const double dz = dir[2];
    // The radial direction: dir projected on rows 0 and 2 (row 1 is the axis).
    const float dot2 = static_cast<float>(dy * xf[7] + dx * xf[6] + dz * xf[8]);
    float a[3];
    for (int i = 0; i < 3; ++i) a[i] = static_cast<float>(static_cast<double>(xf[6 + i]) * dot2);
    const float dot0 = static_cast<float>(dx * xf[0] + dy * xf[1] + dz * xf[2]);
    float b[3];
    for (int i = 0; i < 3; ++i) b[i] = static_cast<float>(static_cast<double>(xf[i]) * dot0);
    float p[3];
    for (int i = 0; i < 3; ++i) p[i] = static_cast<float>(static_cast<double>(b[i]) + a[i]);
    const float len2 = static_cast<float>(static_cast<double>(p[1]) * p[1] +
                                          static_cast<double>(p[0]) * p[0] +
                                          static_cast<double>(p[2]) * p[2]);
    constexpr float kEpsilon_00d7a288 = 1.0e-6f;  // bytes BD 37 86 35
    if (len2 > kEpsilon_00d7a288) {  // FCOMI; JBE to the zero arm (also unordered)
        const float len = sqrt_float_004011d0(len2);
        for (int i = 0; i < 3; ++i) p[i] = static_cast<float>(static_cast<double>(p[i]) / len);
    } else {
        p[0] = p[1] = p[2] = 0.0f;
    }
    const float radius = s.at<float>(0x214);
    float c[3];
    for (int i = 0; i < 3; ++i) c[i] = static_cast<float>(static_cast<double>(p[i]) * radius);
    // The axial end: sign of dir . row1 (zero or unordered gives -1) times S+210h.
    const float dot1 = static_cast<float>(dy * xf[4] + dx * xf[3] + dz * xf[5]);
    const float h = static_cast<float>(s.at<float>(0x210) * support_sign(dot1));
    float q[3];
    for (int i = 0; i < 3; ++i) q[i] = static_cast<float>(static_cast<double>(xf[3 + i]) * h);
    for (int i = 0; i < 3; ++i) {
        const float w = static_cast<float>(static_cast<double>(q[i]) + xf[9 + i]);
        out[i] = static_cast<float>(static_cast<double>(w) + c[i]);
    }
}

void cylinder_shape_support_00c38810(const void* shape, double out[3], const double dir[3],
                                     const float xf[12]) noexcept {
    const Raw s = raw(shape);
    // The double twin of 00C47CA0, everything in registers: no float narrowing, the CRT
    // sqrt (00BF7030) directly, and the double 1e-7 at 00D7A298 as the threshold.
    const double dot2 = dir[1] * xf[7] + dir[0] * xf[6] + dir[2] * xf[8];
    const double dot0 = dir[1] * xf[1] + dir[0] * xf[0] + dir[2] * xf[2];
    double p[3];
    for (int i = 0; i < 3; ++i) p[i] = xf[6 + i] * dot2 + xf[i] * dot0;
    const double len2 = p[0] * p[0] + p[1] * p[1] + p[2] * p[2];
    constexpr double kEpsilon_00d7a298 = 1.0e-7;
    double n[3] = {0.0, 0.0, 0.0};
    if (len2 > kEpsilon_00d7a298) {
        const double len = std::sqrt(len2);
        for (int i = 0; i < 3; ++i) n[i] = p[i] / len;
    }
    const double radius = s.at<float>(0x214);
    double c[3];
    for (int i = 0; i < 3; ++i) c[i] = n[i] * radius;
    const double dot1 = dir[0] * xf[3] + dir[1] * xf[4] + dir[2] * xf[5];
    const double h = support_sign(dot1) * s.at<float>(0x210);
    for (int i = 0; i < 3; ++i) out[i] = (xf[3 + i] * h + xf[9 + i]) + c[i];
}

void* sphere_shape_construct_00c585b0(void* body, void* shape, const void* desc,
                                      std::uint32_t base_vtable, std::uint32_t sphere_vtable) {
    const Raw s = raw(shape);
    const Raw d = raw(desc);
    construct_shape_base(s, body, d, base_vtable);
    s.at<std::uint32_t>(0x0) = sphere_vtable;
    // Identity rows and zero translation in the image's store order, then the centre.
    s.at<float>(0x54) = kOne_00d7a24c;
    s.at<float>(0x44) = kOne_00d7a24c;
    s.at<float>(0x34) = kOne_00d7a24c;
    for (std::uint32_t off : {0x50u, 0x4cu, 0x48u, 0x40u, 0x3cu, 0x38u, 0x60u, 0x5cu, 0x58u})
        s.at<float>(off) = 0.0f;
    s.at<std::uint32_t>(0x58) = d.at<std::uint32_t>(0x18);
    s.at<std::uint32_t>(0x5c) = d.at<std::uint32_t>(0x1c);
    s.at<std::uint32_t>(0x60) = d.at<std::uint32_t>(0x20);
    s.at<float>(0x210) = d.at<float>(0x14);  // the radius, FLD/FSTP float
    sphere_shape_bounds_00c58520(shape);
    return shape;
}
// --------------------------------------------------------------- shape accessors

float* shape_local_transform_00c32430(void* shape) noexcept {
    return reinterpret_cast<float*>(raw(shape).field(0x34));
}

std::uint32_t shape_group_00c32450(const void* shape) noexcept {
    return raw(shape).at<std::uint32_t>(0x2c);
}

void* shape_body_00c32490(const void* shape) noexcept { return raw(shape).ptr(0x4); }

namespace {
// The shared tail of the six mask setters: PUSH 0109ECD8; CALL [EnterCriticalSection];
// PUSH [S+4]; CALL 00C43AA0; JMP [LeaveCriticalSection] with the lock in the arg slot.
void lock_and_clear_manifolds(void* shape, Cc13Dyn1Calls& calls) {
    calls.enter_critical_section(kDynWorldLock_0109ecd8);
    calls.clear_body_manifolds_00c43aa0(raw(shape).ptr(0x4));
    calls.leave_critical_section(kDynWorldLock_0109ecd8);
}
}  // namespace

void shape_collide_mask_clear_00c47f60(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls) {
    raw(shape).at<std::uint32_t>(0x30) &= ~bits;  // NOT EAX; AND [ESI+30h],EAX
    lock_and_clear_manifolds(shape, calls);
}

void shape_collide_mask_set_00c47f90(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls) {
    raw(shape).at<std::uint32_t>(0x30) |= bits;
    lock_and_clear_manifolds(shape, calls);
}

void shape_group_clear_00c47fc0(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls) {
    raw(shape).at<std::uint32_t>(0x2c) &= ~bits;
    lock_and_clear_manifolds(shape, calls);
}

void shape_group_set_00c47ff0(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls) {
    raw(shape).at<std::uint32_t>(0x2c) |= bits;
    lock_and_clear_manifolds(shape, calls);
}

void shape_collide_mask_assign_00c48020(void* shape, std::uint32_t value, Cc13Dyn1Calls& calls) {
    raw(shape).at<std::uint32_t>(0x30) = value;
    lock_and_clear_manifolds(shape, calls);
}

void shape_group_assign_00c48050(void* shape, std::uint32_t value, Cc13Dyn1Calls& calls) {
    raw(shape).at<std::uint32_t>(0x2c) = value;
    lock_and_clear_manifolds(shape, calls);
}

// -------------------------------------------------------------------- transforms

// A float copied through FLD DWORD / FSTP DWORD: a signalling NaN comes out quiet (bit 22
// set); every other pattern, denormals included, is unchanged (cc13-fixture7).
static std::uint32_t fld_fstp_word(std::uint32_t u) noexcept {
    const bool snan = (u & 0x7f800000u) == 0x7f800000u && (u & 0x007fffffu) != 0 && (u & 0x00400000u) == 0;
    return snan ? u | 0x00400000u : u;
}

void transform_expand_3x4_to_4x4_00c33650(const float src[12], float dst[16]) noexcept {
    // FLD/FSTP copies in the image's store order (00C33650..00C336B1); the 0 words are
    // MOVSS stores of a zeroed XMM0.
    const std::uint32_t* const s0 = reinterpret_cast<const std::uint32_t*>(src);
    const auto s = [s0](int k) { return fld_fstp_word(s0[k]); };
    auto* d = reinterpret_cast<std::uint32_t*>(dst);
    d[0] = s(0);
    d[1] = s(1);
    d[3] = 0;
    d[2] = s(2);
    d[4] = s(3);
    d[5] = s(4);
    d[6] = s(5);
    d[7] = 0;
    d[8] = s(6);
    d[9] = s(7);
    d[10] = s(8);
    d[11] = 0;
    d[12] = s(9);
    d[13] = s(10);
    d[14] = s(11);
    dst[15] = kOne_00d7a24c;
}

void transform_from_4x4_00c336c0(float dst[12], const float src[16]) noexcept {
    const std::uint32_t* const s0 = reinterpret_cast<const std::uint32_t*>(src);
    const auto s = [s0](int k) { return fld_fstp_word(s0[k]); };
    auto* d = reinterpret_cast<std::uint32_t*>(dst);
    d[0] = s(0);
    d[1] = s(1);
    d[2] = s(2);
    d[3] = s(4);
    d[4] = s(5);
    d[5] = s(6);
    d[6] = s(8);
    d[7] = s(9);
    d[8] = s(10);
    d[9] = s(12);
    d[10] = s(13);
    d[11] = s(14);
}

// --------------------------------------------------------------- convex hull data

void hull_copy_vertices_00c32ce0(void* const* hull_data, float* out) noexcept {
    const Raw h = raw(*hull_data);
    const std::uint32_t count = h.at<std::uint32_t>(0x4);
    auto* d = reinterpret_cast<std::uint32_t*>(out);
    for (std::uint32_t i = 0; i < count; ++i) {
        // The vertex array pointer [H] is reloaded every iteration.
        const Raw v = raw(static_cast<std::uint8_t*>(h.ptr(0)) + 0x10u * i);
        d[0] = v.at<std::uint32_t>(0x0);
        d[1] = v.at<std::uint32_t>(0x4);
        d[2] = v.at<std::uint32_t>(0x8);
        d += 3;
    }
}

void u16_vector_assign_00c35810(void* dest_vector, const void* source, Cc13Dyn1Calls& calls) {
    const Raw v = raw(dest_vector);
    const Raw src = raw(source);
    if (v.at<std::uint32_t>(0x4) != 0) v.at<std::uint32_t>(0x4) = 0;  // 00C35810..16
    const std::uint32_t need = src.at<std::uint32_t>(0x10);
    if (v.at<std::uint32_t>(0x8) < need) {  // CMP [ESI+8],EAX; JNC skips
        v.at<std::uint32_t>(0x8) = need;
        auto* grown = static_cast<std::uint16_t*>(calls.allocate_00bf55be(need * 2u));
        // Copy the (now zero) old size; each slot written only when its address is not
        // null, as the image's placement-copy loop tests.
        std::uint16_t* slot = grown;
        for (std::uint32_t i = 0; i < v.at<std::uint32_t>(0x4); ++i, ++slot) {
            if (slot != nullptr) slot[0] = static_cast<std::uint16_t*>(v.ptr(0))[i];
        }
        if (v.ptr(0) != nullptr) calls.free_00bf6989(v.ptr(0));
        v.ptr(0) = grown;
    }
    v.at<std::uint32_t>(0x4) = src.at<std::uint32_t>(0x10);
    for (std::uint32_t i = 0; i < src.at<std::uint32_t>(0x10); ++i) {
        std::uint16_t* slot = static_cast<std::uint16_t*>(v.ptr(0)) + i;
        if (slot != nullptr) *slot = static_cast<const std::uint16_t*>(src.ptr(0xc))[i];
    }
}

void hull_copy_indices_00c37430(void* const* hull_data, void* dest_vector,
                                Cc13Dyn1Calls& calls) {
    u16_vector_assign_00c35810(dest_vector, *hull_data, calls);
}

void convex_shape_copy_indices_00c35630(const void* shape, void* dest_vector,
                                        Cc13Dyn1Calls& calls) {
    u16_vector_assign_00c35810(dest_vector, raw(shape).ptr(0x210), calls);
}

void convex_shape_copy_vertices_00c356d0(const void* shape, void* dest_vector,
                                         Cc13Dyn1Calls& calls) {
    const Raw hull = raw(shape).deref(0x210);  // EBP
    const Raw v = raw(dest_vector);
    const std::uint32_t count = hull.at<std::uint32_t>(0x4);  // EBX, read once
    if (count > v.at<std::uint32_t>(0x4) && count > v.at<std::uint32_t>(0x8)) {
        v.at<std::uint32_t>(0x8) = count;
        auto* grown = static_cast<std::uint32_t*>(calls.allocate_00bf55be(count * 12u));
        std::uint32_t* slot = grown;
        for (std::uint32_t i = 0; i < v.at<std::uint32_t>(0x4); ++i, slot += 3) {
            if (slot != nullptr) {
                const auto* old = static_cast<const std::uint32_t*>(v.ptr(0)) + 3 * i;
                slot[0] = old[0];
                slot[1] = old[1];
                slot[2] = old[2];
            }
        }
        if (v.ptr(0) != nullptr) calls.free_00bf6989(v.ptr(0));
        v.ptr(0) = grown;
    }
    v.at<std::uint32_t>(0x4) = count;
    // The copy loop has no null test and re-reads the count each pass.
    for (std::uint32_t i = 0; i < hull.at<std::uint32_t>(0x4); ++i) {
        const auto* src = static_cast<const std::uint32_t*>(hull.ptr(0)) + 4 * i;
        auto* dst = static_cast<std::uint32_t*>(v.ptr(0)) + 3 * i;
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
    }
}

// ----------------------------------------------------- EH-cleanup and small helpers

void delete_critical_section_00c30930(void* section, Cc13Dyn1Calls& calls) {
    calls.delete_critical_section(section);
}

void free_owner_buffers_00c31cc0(void* owner, Cc13Dyn1Calls& calls) {
    const Raw o = raw(owner);
    if (o.ptr(0xea74) != nullptr) calls.free_00bf65ac(o.ptr(0xea74));
    if (o.ptr(0xea6c) != nullptr) calls.free_00bf6989(o.ptr(0xea6c));
}

void free_owner_buffers_00c31d70(void* owner, Cc13Dyn1Calls& calls) {
    free_owner_buffers_00c31cc0(owner, calls);  // byte-identical body
}

void shape_restore_base_vtable_00c32420(void* shape, std::uint32_t base_vtable) noexcept {
    raw(shape).at<std::uint32_t>(0x0) = base_vtable;
}

void* zero_pair_00c32d50(void* pair) noexcept {
    raw(pair).at<std::uint32_t>(0x0) = 0;
    raw(pair).at<std::uint32_t>(0x4) = 0;
    return pair;
}

void profile_scope_leave_00c38040(const void* scope, const DynProfileScopeContext& context) {
    dyn_profile_scope_leave_00c5bcd7(*static_cast<const DynProfileScopeStorage*>(scope),
                                     context);
}

// ------------------------------------------------------ unreachable small bodies

void* sap_next_pair_00c32bb0(void* manager, const void* pair) noexcept {
    void* next = raw(pair).ptr(0xc);
    // SUB; NEG; SBB; AND: next when next != M+48h, else 0.
    return next != raw(manager).field(0x48) ? next : nullptr;
}

void* sap_first_pair_00c32bd0(const void* manager) noexcept {
    const Raw m = raw(manager);
    if (m.at<std::uint32_t>(0x58) != 0) return m.ptr(0x44);
    return nullptr;
}

std::uint32_t sap_pair_count_00c32be0(const void* manager) noexcept {
    return raw(manager).at<std::uint32_t>(0x58);
}

void* radix_next_pair_00c32c20(const void* manager, void* pair) noexcept {
    static_cast<void>(manager);  // ECX is not read
    return raw(pair).field(0x8);
}

std::uint32_t radix_pair_count_00c32c30(const void* manager) noexcept {
    return raw(manager).at<std::uint32_t>(0x20);
}

void* radix_first_pair_00c40d70(const void* manager) noexcept {
    const Raw m = raw(manager);
    if (m.at<std::uint32_t>(0x20) != 0) return m.ptr(0x1c);
    return nullptr;
}

std::uint32_t free_and_clear_pair_00c327f0(void* object, Cc13Dyn1Calls& calls) {
    const Raw o = raw(object);
    if (o.ptr(0x8) != nullptr) {
        calls.free_00bf6989(o.ptr(0x8));
        o.ptr(0x8) = nullptr;
    }
    if (o.ptr(0x14) != nullptr) {
        calls.free_00bf6989(o.ptr(0x14));
        o.ptr(0x14) = nullptr;
    }
    return 0;
}

void free_and_reset_00c32830(void* object, Cc13Dyn1Calls& calls) {
    const Raw o = raw(object);
    if (o.ptr(0x10) != nullptr) calls.free_00bf6989(o.ptr(0x10));
    o.at<std::uint32_t>(0x0) = 0;
    o.at<std::uint32_t>(0x4) = 0;
    o.at<std::uint32_t>(0xc) = 0;
    o.at<std::uint32_t>(0x10) = 0;
}

void free_if_set_00c32a60(void* const* block, Cc13Dyn1Calls& calls) {
    if (*block != nullptr) calls.free_00bf6989(*block);
}

void manifold_bind_bodies_00c36c00(void* manifold, void* first_body, void* second_body,
                                   const AvoidZoneDynHullMemory& memory,
                                   NativeDynNarrowPhaseCalls& narrow_calls) {
    const Raw m = raw(manifold);
    m.at<std::uint32_t>(0xc8) = 0;
    m.ptr(0xcc) = first_body;
    m.ptr(0xd0) = second_body;
    append_native_dyn_manifold_00c35260(first_body, manifold, memory, narrow_calls);
    append_native_dyn_manifold_00c35260(m.ptr(0xd0), manifold, memory, narrow_calls);
}

// -------------------------------------------------------------------- misc object

void object_set_vec58_and_notify_00c357b0(void* object, const float v[3],
                                          Cc13Dyn1Calls& calls) {
    const Raw o = raw(object);
    o.at<std::uint32_t>(0x58) = reinterpret_cast<const std::uint32_t*>(v)[0];
    o.at<std::uint32_t>(0x5c) = reinterpret_cast<const std::uint32_t*>(v)[1];
    o.at<std::uint32_t>(0x60) = reinterpret_cast<const std::uint32_t*>(v)[2];
    calls.object_virtual_slot0(object);  // 00C357C9 CALL [[ECX]]
}

float* object_vec58_00c357d0(void* object) noexcept {
    return reinterpret_cast<float*>(raw(object).field(0x58));
}

// ----------------------------------------------------------------------- geometry

bool ccw_2d_00c377f0(float x0, float y0, float x1, float y1, float x2, float y2) noexcept {
    // Four differences, each stored as float, then the cross in registers; FCOMIP
    // against 0 and JBE to the 0 return, so an unordered cross returns 0. The callers
    // (00C41190, 00C41230) are reached only from 00C4D5A0, which nothing in the image
    // references, so this is judged at the main thread's 24-bit precision control: each
    // FSUB/FMUL rounds to 24 bits with the extended exponent (cc13-fixture7).
    using cc13x87::x87_mul24;
    using cc13x87::x87_sub24;
    const float ax = static_cast<float>(x87_sub24(x1, x0));
    const float ay = static_cast<float>(x87_sub24(y1, y0));
    const float bx = static_cast<float>(x87_sub24(x2, x0));
    const float by = static_cast<float>(x87_sub24(y2, y0));
    const double cross = x87_sub24(x87_mul24(by, ax), x87_mul24(ay, bx));
    return cross > 0.0;
}

void barycentric_2d_00c37860(float* u_out, float* v_out, float* w_out, float x0, float y0,
                             float x1, float y1, float x2, float y2, float px,
                             float py) noexcept {
    auto f = [](double v) { return static_cast<float>(v); };
    // Edge and point offsets, each stored as float.
    const float e1x = f(static_cast<double>(x1) - x0);
    const float e1y = f(static_cast<double>(y1) - y0);
    const float e2x = f(static_cast<double>(x2) - x0);
    const float e2y = f(static_cast<double>(y2) - y0);
    const float dpx = f(static_cast<double>(px) - x0);
    const float dpy = f(static_cast<double>(py) - y0);
    // The five dot products, each one add of two exact products, stored as float.
    const float d11 = f(static_cast<double>(e1x) * e1x + static_cast<double>(e1y) * e1y);
    const float d12 = f(static_cast<double>(e2y) * e1y + static_cast<double>(e2x) * e1x);
    const float d22 = f(static_cast<double>(e2x) * e2x + static_cast<double>(e2y) * e2y);
    const float d1p = f(static_cast<double>(e1x) * dpx + static_cast<double>(e1y) * dpy);
    const float d2p = f(static_cast<double>(e2y) * dpy + static_cast<double>(e2x) * dpx);
    const float den = f(static_cast<double>(d11) * d22 - static_cast<double>(d12) * d12);
    // Numerators stay in registers; each quotient is stored as float and reloaded. No
    // zero-denominator guard, as the image.
    const float u = f((static_cast<double>(d22) * d1p - static_cast<double>(d2p) * d12) / den);
    *u_out = u;
    const float v = f((static_cast<double>(d11) * d2p - static_cast<double>(d12) * d1p) / den);
    *v_out = v;
    *w_out = f((1.0 - u) - v);
}

bool segment_box_entry_00c412d0(const float start[3], const float box[6], float entry_out[3],
                                const float end[3]) noexcept {
    // t_lo is the running entry parameter (kept in a register, updated by a float store),
    // t_hi the running exit parameter at [ESP+14h] (starts at the 1.0f at 00D7A24C).
    // Precision: the only caller is the world ray cast 00C445B0, which no Dyn worker task
    // reaches, so the routine is modelled at the main thread's 24-bit control
    // (cc13-fixture3): the FSUB(R)P of plane - start and the FDIV(R)P by d each round to a
    // 24-bit significand (x87_*24) before the quotient is stored.
    using namespace cc13x87;
    const auto slab = [](float plane, float s, float d) {
        return static_cast<float>(x87_div24(x87_sub24(plane, s), d));
    };
    float t_lo = 0.0f;
    float t_hi = kOne_00d7a24c;
    for (int i = 0; i < 3; ++i) {
        const float s = start[i];
        const float e = end[i];
        const float lo = box[i];
        const float hi = box[3 + i];
        const float d = static_cast<float>(x87_sub24(e, s));  // [ESP+18h]
        float t_enter = 0.0f;
        float t_exit;
        if (e > s) {  // FCOMIP e,s; JBE takes the other arm, so unordered goes there
            if (s > hi) return false;  // JA 00C41506
            if (lo > e) return false;  // JA 00C41519
            if (lo > s) t_enter = slab(lo, s, d);
            // JBE to the 1.0 arm: e <= hi or unordered.
            t_exit = (e > hi) ? slab(hi, s, d) : kOne_00d7a24c;
        } else {
            if (e > hi) return false;  // JA 00C41534
            if (lo > s) return false;  // JA 00C41530
            if (s > hi) t_enter = slab(hi, s, d);
            t_exit = (lo > e) ? slab(lo, s, d) : kOne_00d7a24c;
        }
        if (t_enter > t_lo) t_lo = t_enter;  // FCOMIP; JBE keeps the old value
        if (t_hi > t_exit) t_hi = t_exit;
        if (t_lo > t_hi) return false;  // JA 00C41521
    }
    // 00C41464..: d[i] = float(end[i] - start[i]); scaled = float(d[i] * t_lo);
    // out[i] = float(start[i] + scaled).
    for (int i = 0; i < 3; ++i) {
        const float d = static_cast<float>(x87_sub24(end[i], start[i]));
        const float scaled = static_cast<float>(x87_mul24(d, t_lo));
        entry_out[i] = static_cast<float>(x87_add24(start[i], scaled));
    }
    return true;
}

namespace {
// One body of 00C445B0's two list walks (00C445F0..00C44684 and 00C446A3..00C44737).
void ray_cast_body(Raw world, void* body, const float* start, float current_end[3],
                   float hit[6], void** shape_out, std::uint32_t group_mask, bool& any,
                   Cc13Dyn1Calls& calls) {
    float entry[3];  // [ESP+1Ch], written by 00C412D0 and not read here
    const float* box = reinterpret_cast<const float*>(raw(body).deref(0x60).field(0x4));
    if (!segment_box_entry_00c412d0(start, box, entry, current_end)) return;
    for (void* shape = raw(body).ptr(0x70); shape != nullptr;
         shape = raw(shape).ptr(0x208)) {
        if ((raw(shape).at<std::uint32_t>(0x2c) & group_mask) == 0) continue;
        const std::uint32_t type = raw(shape).at<std::uint32_t>(0x8);
        void* dispatcher = world.ptr(0x90 + 4u * type);
        if (dispatcher == nullptr) continue;
        if (!calls.shape_ray_test_slot0(dispatcher, hit, shape, start, current_end)) continue;
        *shape_out = shape;
        current_end[0] = hit[0];
        current_end[1] = hit[1];
        current_end[2] = hit[2];
        any = true;
    }
}
}  // namespace

bool world_ray_cast_00c445b0(const float start[3], const float end[3], void* world,
                             float point_out[3], float normal_out[3], void** shape_out,
                             std::uint32_t group_mask, Cc13Dyn1Calls& calls) {
    const Raw w = raw(world);
    float current_end[3] = {end[0], end[1], end[2]};  // [ESP+10h]
    // [ESP+28h..3Ch]. The image leaves it uninitialised; with no hit it copies those
    // stack bytes out. Zero here (the only difference, and only on the no-hit path).
    float hit[6] = {};
    bool any = false;
    // [world+A8h] is reloaded for each sentinel compare, as the image does.
    for (void* body = w.deref(0xa8).ptr(0xe0); body != w.deref(0xa8).field(0xe4);
         body = raw(body).ptr(0x84)) {
        ray_cast_body(w, body, start, current_end, hit, shape_out, group_mask, any, calls);
    }
    for (void* body = w.deref(0xa8).ptr(0x204); body != w.deref(0xa8).field(0x208);
         body = raw(body).ptr(0x84)) {
        ray_cast_body(w, body, start, current_end, hit, shape_out, group_mask, any, calls);
    }
    point_out[0] = hit[0];
    point_out[1] = hit[1];
    point_out[2] = hit[2];
    normal_out[0] = hit[3];
    normal_out[1] = hit[4];
    normal_out[2] = hit[5];
    return any;
}

}  // namespace bsp::cc13_dyn1

namespace bsp::cc13_dyn1 {
namespace {
bool nc_inside(float axis, float plane_value, bool upper) {
    std::uint8_t result;
    if (upper) {
        __asm {
            fld axis
            fld plane_value
            fcomip st(0), st(1)
            setbe result
            fstp st(0)
        }
    } else {
        __asm {
            fld plane_value
            fld axis
            fcomip st(0), st(1)
            setbe result
            fstp st(0)
        }
    }
    return result != 0;
}
float nc_clip_intersection(float old_axis, float new_axis, float old_other,
    float new_other, float plane_value) {
    float fraction, result;
    // Native code rounds the fraction to float, then evaluates the multiply
    // and add in x87 precision before the final float spill.
    __asm {
        fld plane_value
        fsub old_axis
        fld new_axis
        fsub old_axis
        fdivp st(1), st(0)
        fstp fraction
        fld new_other
        fsub old_other
        fmul fraction
        fadd old_other
        fstp result
    }
    return result;
}
void nc_clip(std::int32_t& output_count, float* output, const float* input,
    std::int32_t count, float bound, int axis, bool upper) {
    const float* previous = input + (count - 1) * 2;
    output_count = 0;
    bool previous_inside = nc_inside(previous[axis], bound, upper);
    for (std::int32_t i = 0; i < count; ++i) {
        const float* current = input + i * 2;
        const bool inside = nc_inside(current[axis], bound, upper);
        if (inside != previous_inside) {
            float* destination = output + output_count * 2;
            destination[axis] = bound;
            destination[axis ^ 1] = nc_clip_intersection(previous[axis], current[axis],
                previous[axis ^ 1], current[axis ^ 1], bound);
            ++output_count;
        }
        if (inside) {
            // Native accepted vertices use integer MOVs, preserving NaN bits.
            std::memcpy(output + output_count * 2, current, sizeof(float) * 2);
            ++output_count;
        }
        previous = current;
        previous_inside = inside;
    }
}
float nc_copy_float(float value) {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
} // namespace
void clip_y_at_most_00c309d0(std::int32_t& n, float* output,
    const float* input, std::int32_t count, float bound) {
    nc_clip(n, output, input, count, bound, 1, false);
}
void clip_y_at_least_00c30d10(std::int32_t& n, float* output,
    const float* input, std::int32_t count, float bound) {
    nc_clip(n, output, input, count, bound, 1, true);
}
void clip_x_at_most_00c31060(std::int32_t& n, float* output,
    const float* input, std::int32_t count, float bound) {
    nc_clip(n, output, input, count, bound, 0, false);
}
void clip_x_at_least_00c313a0(std::int32_t& n, float* output,
    const float* input, std::int32_t count, float bound) {
    nc_clip(n, output, input, count, bound, 0, true);
}
void expand_node_matrices_00c34cf0(float* output, const void* const* nodes, std::int32_t count) {
    for (std::int32_t i = 0; i < count; ++i) {
        const auto* source = reinterpret_cast<const float*>(
            static_cast<const std::uint8_t*>(nodes[i]) + 8);
        for (int row = 0; row != 4; ++row) {
            for (int col = 0; col != 3; ++col)
                output[i * 16 + row * 4 + col] = nc_copy_float(source[row * 3 + col]);
            output[i * 16 + row * 4 + 3] = row == 3 ? 1.0f : 0.0f;
        }
    }
}
void zero_word_00c30900(std::uint32_t* word) noexcept { *word = 0; }
} // namespace bsp::cc13_dyn1
