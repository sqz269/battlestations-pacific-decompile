#pragma once

#include <cstdint>

namespace bsp {

// Complete raw 0041A200..0041A4D8 (including its skipped three-byte padding).
// Original: ECX actual zone, one stack word (only its low flag byte is read),
// AL result, RET4. The unused EDX slot belongs only to this Source interface.
// Zone+0 is the actual DWORD pointer-array base and zone+4 the DWORD count.
// Each pointer names native record storage: x/z +0/+4, outgoing unit +8/+C,
// right-normal bisector +10/+14, outgoing length +18, signed turn +1C.
// Storage must support every actual access, including modulo-DWORD end
// calculations and repeated owner/record reloads. No guard or alias repair.
// Nonpositive/unordered total turn returns true. Positive turn returns false;
// flag!=0 reverses the pointer cells and recomputes with flag0 first, still
// returning false for the original pass. The flag test considers only byte0.
// Calls the two raw entries below and genuine current UCRT _CIatan2/_CIsqrt
// with hidden x87 operands. Current CRT policy is an external Source boundary,
// not Native BF701A/BF7030 dispatch/state/exception identity. Ambient floating
// state is not reset. See docs/GOAL15I_AVOID_ZONE_WINDING.md for exact evidence.
bool __fastcall derive_native_avoid_zone_0041a200(
    void* actual_zone, void* unused_edx, std::uint32_t reverse_flag_word);

// Complete raw 00419260..004192D4, ECX two float cells, ST0 float-spilled
// reciprocal length, RET. Source-only unused EDX adds no runtime state.
// Keeps all native square/sum/sqrt/reciprocal spills and FUCOMIP/LAHF/TEST/JP
// selection. The sqrt call goes directly to actual current UCRT _CIsqrt.
float __fastcall reciprocal_length_native_vector2_00419260(
    const float* actual_vector, void* unused_edx);

// Complete raw 00438B10..00438B7D: two stack float words, RET8, ST0 result.
// Full x87 subtraction and repeated wrap, with each original float store.
// Float-rounded pi/2pi constants; no finite-input guard or bounded loop.
// Infinities or sufficiently large finite inputs may never make progress.
float __stdcall subtract_native_wrapped_angle_00438b10(float left, float right);

} // namespace bsp
