#pragma once
#include <cstdint>
namespace bsp {
// Descriptive hypothesis: two clamped inputs and a smoothed remainder.
// Raw float bits preserve MOVSS NaN payloads and explicit FLD/FSTP boundaries.
struct NativeSmoothedRemainderTriplet {
    std::uint32_t first_bits,second_bits,remainder_bits;
};
static_assert(sizeof(NativeSmoothedRemainderTriplet)==12);
struct NativeTripletSmoothingContext {
    const volatile float& upper_00d7a24c;
    const volatile double& falling_new_00ce3e20;
    const volatile double& falling_old_00d04308;
    const volatile double& rising_new_00ce4d68;
    const volatile double& rising_old_00d04310;
};
// Native ECX=triplet, two stack float values, RET8. Source adds borrowed context.
// COMISS/FCOMI unordered branches, float spills and double FMUL operands matter.
void update_native_smoothed_remainder_triplet_007862c0(NativeSmoothedRemainderTriplet*,
    std::uint32_t first_bits,std::uint32_t second_bits,const NativeTripletSmoothingContext&);
} // namespace bsp
