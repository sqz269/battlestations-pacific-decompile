// Leaf helpers from the 00450000-0048FFFF band (cc13-sweep45). Evidence and uncertainty are in
// docs/CC13_BAND_0045_0048.md; the per-function ABI notes are in the header.

#include "bsp/band_0045_cc13.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace bsp::band_0045_cc13 {
namespace {

float f32(double value) noexcept { return static_cast<float>(value); }

// 00481C90/00481D40 per-component step: keep `cand` unless `other` wins strictly.
float keep_unless_greater(float cand, float other) noexcept { return cand > other ? other : cand; }

}  // namespace

Vec3 component_min_00481c90(const Vec3& a, const Vec3& b) noexcept {
    // FLD a; FLD b; FCOMIP -> compares b with a; JBE keeps b, else a.
    return {keep_unless_greater(b.x, a.x), keep_unless_greater(b.y, a.y),
            keep_unless_greater(b.z, a.z)};
}

Vec3 component_max_00481d40(const Vec3& a, const Vec3& b) noexcept {
    // FLD b; FLD a; FCOMIP -> compares a with b; JBE keeps b, else a.
    const auto pick = [](float av, float bv) { return av > bv ? av : bv; };
    return {pick(a.x, b.x), pick(a.y, b.y), pick(a.z, b.z)};
}

bool exact_equals_00453f40(const Vec3& a, const Vec3& b) noexcept {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool exact_equals_00475df0(const Vec4& a, const Vec4& b) noexcept {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

float point_line_distance_0046f440(const Vec3& point, const Vec3& line_point,
                                   const Vec3& dir) noexcept {
    // 0046F443..0046F45F: the offset is stored as floats.
    const float dx = f32(static_cast<double>(point.x) - line_point.x);
    const float dy = f32(static_cast<double>(point.y) - line_point.y);
    const float dz = f32(static_cast<double>(point.z) - line_point.z);
    // 0046F476..0046F4A3: (dir.x*dx + dir.y*dy) + dir.z*dz, stored as a float.
    const float dot = f32(static_cast<double>(dir.x) * dx + static_cast<double>(dir.y) * dy +
                          static_cast<double>(dir.z) * dz);
    // 0046F4A7..0046F4E3: projection components, then the residual, each stored.
    const float px = f32(static_cast<double>(dir.x) * dot);
    const float py = f32(static_cast<double>(dir.y) * dot);
    const float pz = f32(static_cast<double>(dot) * dir.z);
    const float rx = f32(static_cast<double>(dx) - px);
    const float ry = f32(static_cast<double>(dy) - py);
    const float rz = f32(static_cast<double>(dz) - pz);
    // 0046F4E7..0046F503: (ry*ry + rx*rx) + rz*rz, stored.
    const float squared = f32(static_cast<double>(ry) * ry + static_cast<double>(rx) * rx +
                              static_cast<double>(rz) * rz);
    // 0046F507: FCOMI against the double 1e-10 at 00CE3820, JBE to the zero return.
    if (!(static_cast<double>(squared) > 1e-10)) {
        return 0.0f;
    }
    return f32(std::sqrt(static_cast<double>(squared)));  // 00BF7030, result stored as float
}

bool ray_plane_intersect_0046fc80(const Vec3& origin, const Vec3& dir, const Vec3& plane_point,
                                  const Vec3& plane_normal, Vec3& out) noexcept {
    // 0046FCAE..0046FCD3: (n.x*dir.x + n.y*dir.y) + n.z*dir.z, stored.
    const float denom = f32(static_cast<double>(plane_normal.x) * dir.x +
                            static_cast<double>(plane_normal.y) * dir.y +
                            static_cast<double>(plane_normal.z) * dir.z);
    // 0046FCD6..0046FCE3: FUCOMIP with 0; equal (ZF without PF) returns 0.
    if (denom == 0.0f) {
        return false;
    }
    const float ox = f32(static_cast<double>(plane_point.x) - origin.x);
    const float oy = f32(static_cast<double>(plane_point.y) - origin.y);
    const float oz = f32(static_cast<double>(plane_point.z) - origin.z);
    // 0046FD37..0046FD53: (n.x*ox + n.y*oy) + n.z*oz stored, then divided and stored again.
    const float numer = f32(static_cast<double>(plane_normal.x) * ox +
                            static_cast<double>(plane_normal.y) * oy +
                            static_cast<double>(plane_normal.z) * oz);
    const float t = f32(static_cast<double>(numer) / denom);
    // 0046FD62..0046FD9A: each dir*t is stored as a float first (FSTP [ESP+14h/18h/1Ch]), then
    // added to the origin component kept on the stack. The fixture (docs/CC13_FIXTURE_PASS_1.md)
    // caught the earlier unrounded product.
    const float sx = f32(static_cast<double>(dir.x) * t);
    const float sy = f32(static_cast<double>(dir.y) * t);
    const float sz = f32(static_cast<double>(dir.z) * t);
    out.x = f32(static_cast<double>(origin.x) + sx);
    out.y = f32(static_cast<double>(origin.y) + sy);
    out.z = f32(static_cast<double>(origin.z) + sz);
    return true;
}

int parse_comma_separated_longs_00453120(const char* text, std::vector<std::int32_t>& out,
                                         int max) {
    int count = 0;
    for (;;) {
        // _strchr(text, ',') falling back to _strchr(text, 0), which never returns null.
        const char* end = std::strchr(text, ',');
        if (end == nullptr) {
            end = text + std::strlen(text);
        }
        if (max != 0 && count >= max) {
            return count;
        }
        const std::size_t size = static_cast<std::size_t>(end - text);
        std::int32_t value = 0;
        if (size != 0) {
            const std::string token(text, size);
            value = static_cast<std::int32_t>(std::atol(token.c_str()));
        }
        out.push_back(value);
        ++count;
        if (*end == '\0') {
            return count;
        }
        text = end + 1;
    }
}

std::string strip_extension_00453260(const char* path) {
    const std::size_t length = std::strlen(path);
    if (length == 0) {
        return {};
    }
    // _splitpath's extension: from the last '.' after the last path separator to the end.
    std::size_t extension = 0;
    for (std::size_t i = length; i-- > 0;) {
        const char c = path[i];
        if (c == '\\' || c == '/' || c == ':') {
            break;
        }
        if (c == '.') {
            extension = length - i;
            break;
        }
    }
    return std::string(path, length - extension);
}

}  // namespace bsp::band_0045_cc13
