// cc13-ng2 leaves (docs/CC13_NG2.md). Evidence per function in include/bsp/ng2_cc13.hpp.

#include "bsp/ng2_cc13.hpp"

#include <cmath>

namespace bsp::ng2_cc13 {

std::uint32_t mpkg_reversed_source_index_00bb8580(std::uint32_t index, std::int32_t size) noexcept {
    // 00BB8585..00BB85A2: MUL by AE10FFA9h, SHR 9 (unsigned /2F1h) for ECX; IMUL by 57087FD5h,
    // SAR 8 plus sign fix (signed /2F1h) for EDX; equality compare only.
    const std::uint32_t block = index / kMpkgBlockLength;
    const std::int32_t final_block = size / static_cast<std::int32_t>(kMpkgBlockLength);
    std::uint32_t length = kMpkgBlockLength;
    if (block == static_cast<std::uint32_t>(final_block)) {
        // 00BB85AB: EDI - final_block*2F1h.
        length = static_cast<std::uint32_t>(size) -
                 static_cast<std::uint32_t>(final_block) * kMpkgBlockLength;
    }
    if (length == 0u) {
        return 0u;  // native DIV EBX faults here (00BB85BF)
    }
    // 00BB85B5..00BB85C4: DIV of the global index, then block*2F1h - rem + length - 1.
    return block * kMpkgBlockLength - index % length + length - 1u;
}

float piecewise_lerp4_0059aa40(float k0, float k1, float k2, float k3, float t) noexcept {
    // 0059AA43..0059AA64: floor (00BF85B0) of t as a double, stored to float, subtracted from
    // the double copy of t and stored to float.
    const float floor_t = static_cast<float>(std::floor(static_cast<double>(t)));
    const float f = static_cast<float>(static_cast<double>(t) - static_cast<double>(floor_t));
    const double tt = static_cast<double>(t);
    const double one_minus = 1.0 - static_cast<double>(f);
    // Thresholds are the doubles at 00CEFF90, 00CEFF88 and 00CEFF80; FCOMI/JBE sends NaN onward.
    if (tt > 3.999) {
        return k3;
    }
    if (tt > 2.999) {
        return static_cast<float>(one_minus * k2 + static_cast<double>(k3) * f);
    }
    if (tt > 1.999) {
        return static_cast<float>(one_minus * k1 + static_cast<double>(k2) * f);
    }
    return static_cast<float>(one_minus * k0 + static_cast<double>(k1) * f);
}

}  // namespace bsp::ng2_cc13
