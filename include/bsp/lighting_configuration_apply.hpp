#pragma once

#include "bsp/system_lighting_constants.hpp"

namespace bsp {
struct ConcreteSystemAmbientLight;

// Borrowed mutable fields of the actual owner; constructing a view writes none.
struct LightingAmbientFields {
    SystemLightingWords4& ambient_18;
    SystemLightingWords4& ambient_mode3_28;
    std::array<SystemLightingWords4, 6>& ambient_cube_38;
};
LightingAmbientFields lighting_ambient_fields(ConcreteSystemAmbientLight&) noexcept;

// Bind these to the SAME NativeLightTailStorage used by the light's read view.
struct LightingDirectionalFields {
    SystemLightingWords4& diffuse_184;
    SystemLightingWords4& specular_194;
    SystemLightingWords4& base_diffuse_1a4;
    SystemLightingWords4& diffuse_mode3_1b4;
    SystemLightingWords4& base_specular_1c4;
    std::uint32_t& diffuse_scale_1d8;
    std::uint32_t& specular_scale_1dc;
    SystemLightingWords3& direction_1e0;
};

// Actual configuration fields, not a cached color record. F54 and each cube's
// fourth DWORD are scalar multipliers as well as the source alpha words.
struct LightingConfigurationFields {
    const SystemLightingWords4& ambient_f48;
    const SystemLightingWords4& ambient_mode3_f58;
    const std::array<SystemLightingWords4, 6>& ambient_cube_f68;
    const SystemLightingWords4& base_diffuse_fc8;
    const SystemLightingWords4& diffuse_mode3_fd8;
    const SystemLightingWords4& base_specular_fe8;
    const std::uint32_t& diffuse_scale_ffc;
    const std::uint32_t& specular_scale_1000;
    const std::uint32_t& second_angle_1004;
    const std::uint32_t& first_angle_1008;
};

// All globals retain their current process words across calls. A guard tests
// mask 0x1 in its low byte and ORs 0x1 into the complete DWORD when first used.
struct LightingConfigurationGlobals {
    std::uint32_t& half_guard_00e18b18;
    SystemLightingWords4& half_color_00e18b08;
    std::uint32_t& diffuse_guard_00e18b04;
    SystemLightingWords4& diffuse_color_00e18af4;
    std::uint32_t& specular_guard_00e18af0;
    SystemLightingWords4& specular_color_00e18ae0;
    const SystemLightingWords3& direction_00f8758c;
    const std::uint8_t& scale_ambient_00f88a0c;
    const std::uint32_t& half_word_00ce3800;
    const std::uint32_t& one_word_00d7a24c;
};

void set_lighting_ambient_00b7af20(LightingAmbientFields, const SystemLightingWords4&);
void set_lighting_mode3_ambient_00b7af40(LightingAmbientFields, const SystemLightingWords4&);
// Native performs no bounds check; the supported owner view requires index < 6.
void set_lighting_ambient_cube_00b7af60(
    LightingAmbientFields, const SystemLightingWords4&, std::uint32_t index);
void set_lighting_base_diffuse_004b62e0(LightingDirectionalFields, const SystemLightingWords4&);
// Inputs are the raw DWORDs passed to the native stack float parameters.
void set_lighting_diffuse_scale_00b7af90(LightingDirectionalFields, std::uint32_t);
void set_lighting_specular_scale_00b7b010(LightingDirectionalFields, std::uint32_t);
SystemLightingWords3& lighting_direction_from_angles_004b4d80(
    SystemLightingWords3& output, std::uint32_t first, std::uint32_t second);

// Native ECX ambient, EDX directional, stack optional config, RET4; new C++ ABI.
// Null preserves effective specular+194 and specular scale+1DC exactly.
void apply_lighting_configuration_004bacd0(LightingAmbientFields,
    LightingDirectionalFields, const LightingConfigurationFields*, LightingConfigurationGlobals);
} // namespace bsp
