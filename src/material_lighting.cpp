#include "bsp/material_lighting.hpp"
#include <cstring>

namespace bsp {
MaterialLightingValues initialize_lighting_record_00b17840() noexcept {
    return {1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 10.0f};
}

MaterialLighting::MaterialLighting() noexcept
    : values_(initialize_lighting_record_00b17840()) {}

void MaterialLighting::set_lighting_record_00b179d0(std::uint32_t,
    const MaterialLightingValues& values) noexcept {
    flag_10c_ = true;
    // Native REP MOVSD preserves every bit, including signed zero/NaN payloads.
    // Identical source/destination is also a valid native no-op copy.
    if (&values != &values_)
        std::memcpy(values_.data(), values.data(), 17 * sizeof(float));
}

float* MaterialLighting::diffuse_color_00b179f0(std::uint32_t) noexcept {
    return values_.data();
}

const float* MaterialLighting::diffuse_color_00b179f0(std::uint32_t) const noexcept {
    return values_.data();
}
}
