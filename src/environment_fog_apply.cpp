#include "bsp/environment_fog_apply.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Environment fog application requires MSVC Win32 x87 operation ordering.
#endif

namespace bsp {
namespace {
SystemFogOwner* load_owner(const SystemFogState* const& slot) noexcept {
    const auto* address = &slot;
    const SystemFogState* fields;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov fields, eax
    }
    return system_fog_owner_from_state(fields);
}

// One sequence for the native FLD -> camera+184 read -> FSTP. Do not return
// a C++ float: the callee setters take the actual spilled DWORD bits directly.
SystemFogOwner* load_scalar_and_owner(const float& source,
    const SystemFogState* const& slot, std::uint32_t& argument_bits) noexcept {
    const auto* input = &source;
    const auto* address = &slot;
    auto* output = &argument_bits;
    const SystemFogState* fields;
    __asm {
        mov eax, input
        mov edx, address
        fld dword ptr [eax]
        mov ecx, dword ptr [edx]
        mov eax, output
        fstp dword ptr [eax]
        mov fields, ecx
    }
    return system_fog_owner_from_state(fields);
}

bool missing_owner(std::string& error) {
    error = "Environment fog update requires the actual live camera fog owner";
    return false;
}
}

bool apply_environment_fog_0078d076(const EnvironmentFogFields& environment,
    const SystemFogState* const& actual_camera_fog_184, std::string& error) {
    auto* owner = load_owner(actual_camera_fog_184); //0078D076
    if (!owner) return missing_owner(error);
    set_system_fog_color_00b84c40(*owner, environment.color_34.data());
    for (std::uint32_t index = 0; index != 4; ++index) {
        owner = load_owner(actual_camera_fog_184); //0078D090, independently every turn
        if (!owner) return missing_owner(error);
        set_system_fog_directional_color_00b84fa0(*owner,
            environment.directional_colors_44[index].data(), index);
    }

    std::uint32_t bits;
    owner = load_scalar_and_owner(environment.scalars_08[0], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_68_00b84d00(*owner, bits); //0078D0A8: environment+08
    owner = load_scalar_and_owner(environment.scalars_08[1], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_6c_00b84d10(*owner, bits); //0078D0BA: +0C
    owner = load_scalar_and_owner(environment.scalars_08[2], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_70_00b84d20(*owner, bits); //0078D0CC: +10
    owner = load_scalar_and_owner(environment.scalars_08[3], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_78_00b84d40(*owner, bits); //0078D0DE: +14, before74
    owner = load_scalar_and_owner(environment.scalars_08[4], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_74_00b84d30(*owner, bits); //0078D0F0: +18
    owner = load_scalar_and_owner(environment.scalars_08[5], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_7c_00b84d50(*owner, bits); //0078D102: +1C

    owner = load_owner(actual_camera_fog_184); //0078D114
    if (!owner) return missing_owner(error);
    set_system_fog_underwater_color_00b84c70(*owner, environment.underwater_color_84.data());

    owner = load_scalar_and_owner(environment.scalars_08[9], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_80_00b84d60(*owner, bits); //0078D126: +2C
    owner = load_scalar_and_owner(environment.scalars_08[10], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_84_00b84d80(*owner, bits); //0078D138: +30
    owner = load_scalar_and_owner(environment.scalars_08[6], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_88_00b84dc0(*owner, bits); //0078D14A: +20
    owner = load_scalar_and_owner(environment.scalars_08[8], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_8c_00b84de0(*owner, bits); //0078D15C: +28, before+24
    owner = load_scalar_and_owner(environment.scalars_08[7], actual_camera_fog_184, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_90_00b84e00(*owner, bits); //0078D16E: +24
    return true;
}
}
