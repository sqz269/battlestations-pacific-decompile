#pragma once

// Leaves from the cc13-ng2 sweep of the remaining non-gameplay raw functions (docs/CC13_NG2.md).
// New C++ interfaces over the native arithmetic, not drop-in replacements for the native register
// or stack ABI. Names are hypotheses, not recovered symbols.

#include <cstdint>

namespace bsp::ng2_cc13 {

// MPKG block length: the archive bytes are stored in reversed 2F1h-byte blocks.
inline constexpr std::uint32_t kMpkgBlockLength = 0x2F1u;

// 00BB8580 BSP_MPKG_ReversedSourceIndex. Native ABI: ECX index (divided unsigned), EDX size
// (divided signed), EAX result, RET. The block of `index` has length 2F1h, or size - (size/2F1h)*2F1h
// when it is the final block; the result is block*2F1h + length - 1 - index % length. Like the
// inline copies in BSP_MPKG_OpenDecodedArchive 00BB9920 and 00BB9180, the final block uses the
// GLOBAL index modulo its short length. A final block of length 0 is a native divide fault; the
// caller must not pass index >= size when size is a multiple of 2F1h (returns 0 here instead).
std::uint32_t mpkg_reversed_source_index_00bb8580(std::uint32_t index, std::int32_t size) noexcept;

// 0059AA40 BSP_Math_PiecewiseLerp4. Native ABI: __stdcall(k0, k1, k2, k3, t), x87, RET 14h,
// result in ST0 rounded to float. f = t - (float)floor(t); t > 3.999 -> k3; t > 2.999 -> lerp(k2,
// k3, f); t > 1.999 -> lerp(k1, k2, f); otherwise (NaN included) lerp(k0, k1, f), each lerp as
// (1-f)*a + f*b. The native sum is formed in the x87 80-bit stack and rounded once to float; this
// forms it in double (both products are exact) and rounds to float, which can differ only on a
// double-rounding tie.
float piecewise_lerp4_0059aa40(float k0, float k1, float k2, float k3, float t) noexcept;

}  // namespace bsp::ng2_cc13
