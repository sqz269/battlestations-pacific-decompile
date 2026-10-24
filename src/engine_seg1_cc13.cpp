// Segment 1 engine helpers (cc13-left2). Evidence and ABIs: include/bsp/engine_seg1_cc13.hpp and
// docs/CC13_LEFT_2.md. Intermediates are double, matching the image's 53-bit x87 precision; every float
// store the image makes is a float cast here.
#include "bsp/engine_seg1_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include "bsp/native_vector2_math.hpp"

#include <cstdint>
#include <cstring>

namespace bsp::engine_seg1_cc13 {
namespace {

// FLD dword / FSTP dword: a signalling NaN comes back quiet; every other value is unchanged.
float x87_reload(float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0u) bits |= 0x00400000u;
    std::memcpy(&value, &bits, 4);
    return value;
}

}  // namespace

float math_lerp_00414130(const float* a, const float* b, const float* t) noexcept {
    const float spilled_a = *a;
    return static_cast<float>((static_cast<double>(*b) - spilled_a) * *t + spilled_a);
}

float* vector3_assign_00414220(float* destination, const float* source) noexcept {
    destination[0] = x87_reload(source[0]);
    destination[1] = x87_reload(source[1]);
    destination[2] = x87_reload(source[2]);
    return destination;
}

float* vector3_scale_to_00414260(const float* v, float* out, float s) noexcept {
    out[0] = static_cast<float>(static_cast<double>(v[0]) * s);
    out[1] = static_cast<float>(static_cast<double>(v[1]) * s);
    out[2] = static_cast<float>(static_cast<double>(s) * v[2]);
    return out;
}

float* vector3_add_to_004142a0(const float* a, float* out, const float* b) noexcept {
    for (int i = 0; i < 3; ++i) out[i] = static_cast<float>(static_cast<double>(b[i]) + a[i]);
    return out;
}

// Main thread, 24-bit precision control (cc13-fixture7): each FMUL/FADD rounds to 24 bits
// with the extended exponent before its FSTP DWORD (004191E6..004191FE).
float vector2_length_squared_004191d0(const float* v) noexcept {
    const float xx = static_cast<float>(cc13x87::x87_mul24(v[0], v[0]));
    const float yy = static_cast<float>(cc13x87::x87_mul24(v[1], v[1]));
    return static_cast<float>(cc13x87::x87_add24(xx, yy));
}

// 00419401..00419429 at 24-bit precision control (cc13-fixture7): the squares are stored to
// float, but xx + yy stays on the stack (FADDP 0041941D), so both FADDs round to 24 bits
// with the extended exponent before the final FSTP DWORD.
float vector3_length_squared_004193e0(const float* v) noexcept {
    const float xx = static_cast<float>(cc13x87::x87_mul24(v[0], v[0]));
    const float yy = static_cast<float>(cc13x87::x87_mul24(v[1], v[1]));
    const double xy = cc13x87::x87_add24(xx, yy);
    const float zz = static_cast<float>(cc13x87::x87_mul24(v[2], v[2]));
    return static_cast<float>(cc13x87::x87_add24(xy, zz));
}

float* vector2_normalize_004192e0(float* out, const float* in, const CameraAxesCrtAccess* crt) {
    const float r = native_vector2_reciprocal_length_00419260(in, crt);
    out[0] = static_cast<float>(static_cast<double>(in[0]) * r);
    out[1] = static_cast<float>(static_cast<double>(r) * in[1]);
    return out;
}

}  // namespace bsp::engine_seg1_cc13
