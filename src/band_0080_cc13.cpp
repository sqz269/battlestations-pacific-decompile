#include "bsp/band_0080_cc13.hpp"

namespace bsp::cc13_band0080 {

int hex_digit_value_008ef6a0(char c) noexcept
{
    const auto u = static_cast<unsigned char>(c);
    // 008EF6A0: three unsigned range tests, SUB 30h / 57h / 37h.
    if (static_cast<unsigned char>(u - 0x30) < 10) {
        return u - 0x30;
    }
    if (static_cast<unsigned char>(u - 0x61) < 6) {
        return u - 0x57;
    }
    if (static_cast<unsigned char>(u - 0x41) < 6) {
        return u - 0x37;
    }
    return 0;
}

std::uint8_t decode_hex_byte_token_008ef6d0(const char* token) noexcept
{
    if (token == nullptr || token[0] == '\0') {
        return 0;
    }
    // 008EF6E0..008EF710 inline the digit test for the first character.
    auto value = static_cast<std::uint8_t>(hex_digit_value_008ef6a0(token[0]));
    if (token[1] != '\0') {
        // 008EF712: MOV CL,[ECX+1]; CALL 008EF6A0; SHL BL,4; OR BL,AL.
        value = static_cast<std::uint8_t>((value << 4) | hex_digit_value_008ef6a0(token[1]));
    }
    return value;
}

std::string escape_scene_text_008ee5f0(const char* text, std::int32_t max_characters)
{
    std::string out;
    if (text == nullptr || max_characters == 0) {
        return out;
    }
    for (; *text != '\0' && max_characters != 0; ++text, --max_characters) {
        switch (*text) {
        case '\n': out += "\\n"; break;
        case '"': out += "\\q"; break;
        case '\\': out += "\\\\"; break;
        default: out += *text; break;
        }
    }
    return out;
}

bool segment_intersect_triangle_0085d020(const Vec3f& p0, const Vec3f& p1, const Vec3f& a,
                                         const Vec3f& b, const Vec3f& c, Vec3f& out) noexcept
{
    const float dx = p1.x - p0.x;
    const float dy = p1.y - p0.y;
    const float dz = p1.z - p0.z;
    const float e1x = b.x - a.x;
    const float e1y = b.y - a.y;
    const float e1z = b.z - a.z;
    const float e2x = c.x - a.x;
    const float e2y = c.y - a.y;
    const float e2z = c.z - a.z;
    // h = d x e2
    const float hx = static_cast<float>(double(dy) * e2z - double(dz) * e2y);
    const float hy = static_cast<float>(double(e2x) * dz - double(dx) * e2z);
    const float hz = static_cast<float>(double(e2y) * dx - double(dy) * e2x);
    // 0085D10B..0085D129: (hx*e1x + hy*e1y) + hz*e1z, stored to a float slot before the two FCOMI
    // tests. The summation order and the float stores of each dot product below were corrected
    // after the native fixture (docs/CC13_FIXTURE_PASS_1.md) showed last-place differences.
    const float det = static_cast<float>(double(hx) * e1x + double(hy) * e1y + double(hz) * e1z);
    if (double(det) > -0.0000999999974737875 && double(det) < 0.0000999999974737875) {
        return false;
    }
    const float inv = static_cast<float>(1.0 / det);
    const float sx = p0.x - a.x;
    const float sy = p0.y - a.y;
    const float sz = p0.z - a.z;
    // 0085D184..0085D1B0: (hy*sy + hx*sx) + hz*sz stored as a float, times inv, stored again.
    const float u_dot = static_cast<float>(double(hy) * sy + double(hx) * sx + double(hz) * sz);
    const float u = static_cast<float>(double(u_dot) * inv);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    // q = s x e1
    const float qx = static_cast<float>(double(sy) * e1z - double(sz) * e1y);
    const float qy = static_cast<float>(double(sz) * e1x - double(e1z) * sx);
    const float qz = static_cast<float>(double(e1y) * sx - double(sy) * e1x);
    // 0085D215..0085D23F: (qy*dy + qx*dx) + qz*dz stored as a float, times inv, stored again.
    const float v_dot = static_cast<float>(double(qy) * dy + double(qx) * dx + double(qz) * dz);
    const float v = static_cast<float>(double(v_dot) * inv);
    if (v < 0.0f || 1.0 < double(v) + u) {
        return false;
    }
    // 0085D269..0085D28D: (qy*e2y + qx*e2x) + qz*e2z stored as a float, times inv, stored again.
    const float t_dot = static_cast<float>(double(qy) * e2y + double(qx) * e2x + double(qz) * e2z);
    const float t = static_cast<float>(double(t_dot) * inv);
    if (t < 0.0f || t > 1.0f) {
        return false;
    }
    out.x = p0.x + t * dx;
    out.y = p0.y + t * dy;
    out.z = p0.z + t * dz;
    return true;
}

std::int32_t effect_component_category(std::string_view component) noexcept
{
    for (const auto& row : kEffectComponentCategories) {
        if (row.component == component) {
            return row.category;
        }
    }
    return -1;
}

} // namespace bsp::cc13_band0080
