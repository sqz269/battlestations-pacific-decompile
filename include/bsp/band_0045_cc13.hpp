#pragma once

// Leaf helpers from the 00450000-0048FFFF band (cc13-sweep45, docs/CC13_BAND_0045_0048.md).
// New C++ interfaces, not the native register ABI. Names are hypotheses, not recovered
// symbols; each declaration states the original address and ABI.
//
// x87 model (as in bsp/engine_core_cc13.hpp): the image runs with 53-bit precision, so every
// FSTP to a float slot is a float cast and unspilled intermediates are doubles. Products of
// two floats are exact in double; the casts below sit exactly where the listings store.

#include <cstdint>
#include <string>
#include <vector>

namespace bsp::band_0045_cc13 {

struct Vec3 {
    float x{};
    float y{};
    float z{};
};

struct Vec4 {
    float x{};
    float y{};
    float z{};
    float w{};
};

// 00481C90 BSP_Vector3_ComponentMin, __fastcall(ECX out, EDX a, [ESP+4] b), RET 4, EAX = out.
// Per component FCOMIP(b, a) / JBE: b is kept unless b > a, so an unordered compare keeps b.
Vec3 component_min_00481c90(const Vec3& a, const Vec3& b) noexcept;

// 00481D40 BSP_Vector3_ComponentMax, same ABI. Per component FCOMIP(a, b) / JBE: b is kept
// unless a > b, so an unordered compare keeps b.
Vec3 component_max_00481d40(const Vec3& a, const Vec3& b) noexcept;

// 00453F40 BSP_Vector3_ExactEquals, __thiscall(ECX a, [ESP+4] b), RET 4, EAX = 0/1.
bool exact_equals_00453f40(const Vec3& a, const Vec3& b) noexcept;

// 00475DF0 BSP_Vector4_ExactEquals, __thiscall(ECX a, [ESP+4] b), RET 4, EAX = 0/1.
bool exact_equals_00475df0(const Vec4& a, const Vec4& b) noexcept;

// 0046F440 BSP_Geometry_PointLineDistance, __fastcall(ECX point, EDX line_point, [ESP+4] dir),
// RET 4, result in ST0 (a float reload). `dir` is used as given (the routine assumes unit
// length). Returns 0 unless the residual squared length is above 1e-10 (double 00CE3820).
float point_line_distance_0046f440(const Vec3& point, const Vec3& line_point,
                                   const Vec3& dir) noexcept;

// 0046FC80 BSP_Geometry_RayPlaneIntersect, __fastcall(ECX origin, EDX dir, [ESP+4]
// plane_point, [ESP+8] plane_normal, [ESP+0Ch] out), RET 0Ch, AL = hit. Returns false and
// leaves `out` alone when dot(dir, normal) == 0 (LAHF/TEST AH,44h/JNP: a NaN denominator
// proceeds and writes NaNs). Otherwise out = origin + float(dir * t), t = dot(plane_point - origin,
// normal) / dot(dir, normal), and returns true. The ray parameter is not range-checked.
bool ray_plane_intersect_0046fc80(const Vec3& origin, const Vec3& dir, const Vec3& plane_point,
                                  const Vec3& plane_normal, Vec3& out) noexcept;

// 00453120 BSP_ParseCommaSeparatedLongs, __fastcall(ECX text, EDX out, [ESP+4] max), RET 4,
// EAX = count. Each comma-separated token goes through _atol (an empty token stores 0);
// parsing stops after `max` values when max != 0. The native copies each token into a
// 32-byte stack buffer without a bound, so tokens of 32 characters or more overflow it; this
// version parses the whole token instead (divergence only where the native is undefined).
int parse_comma_separated_longs_00453120(const char* text, std::vector<std::int32_t>& out,
                                         int max);

// 00453260 BSP_Path_StripExtensionStatic, __fastcall(ECX path), EAX = the static buffer
// 00E18400. _splitpath extracts the extension (into the static 00E18500) and the path minus
// that many trailing characters is returned. The native buffers are 100h bytes with no bound;
// this version returns a std::string.
std::string strip_extension_00453260(const char* path);

}  // namespace bsp::band_0045_cc13
