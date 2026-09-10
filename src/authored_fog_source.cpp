#include "bsp/authored_fog_source.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Authored fog copying requires MSVC Win32 x87 operation ordering.
#endif

namespace bsp {
namespace {
void copy_words(void* destination, const void* source, std::size_t count) noexcept {
    auto* output = static_cast<unsigned char*>(destination);
    const auto* input = static_cast<const unsigned char*>(source);
    for (std::size_t i = 0; i != count; ++i) {
        __asm {
            mov eax, input
            mov edx, output
            mov ecx, dword ptr [eax]
            mov dword ptr [edx], ecx
        }
        input += 4;
        output += 4;
    }
}

void copy_scalar(float& destination, const void* source) noexcept {
    auto* output = &destination;
    __asm {
        mov eax, source
        mov edx, output
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}

// 0078CB91..0078CB9F: the first owner capture belongs after this store,
// before the following authored scalar load, not beside the first setter.
SystemFogOwner* copy_first_scalar_and_capture(float& destination,
    const void* source, const SystemFogState* const& slot) noexcept {
    auto* output = &destination;
    const auto* address = &slot;
    const SystemFogState* fields;
    __asm {
        mov eax, source
        mov edx, output
        mov ecx, address
        fld dword ptr [eax]
        fstp dword ptr [edx]
        mov eax, dword ptr [ecx]
        mov fields, eax
    }
    return system_fog_owner_from_state(fields);
}

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

SystemFogOwner* load_scalar_and_owner(const float& source,
    const SystemFogState* const& slot, std::uint32_t& bits) noexcept {
    const auto* input = &source;
    const auto* address = &slot;
    auto* output = &bits;
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
    error = "Authored fog copy requires the actual live environment private owner";
    return false;
}
} // namespace

bool copy_authored_fog_0078caa4(const void* actual_authored_block,
    std::size_t readable_bytes, const MutableEnvironmentFogFields& environment,
    const SystemFogState* const& actual_environment_private_b4, std::string& error) {
    if (!actual_authored_block || readable_bytes < kAuthoredFogMinimumReadableBytes) {
        error = "Authored fog source requires at least 128h readable bytes";
        return false;
    }
    const auto* source = static_cast<const unsigned char*>(actual_authored_block);
    copy_words(environment.color_34.data(), source + 0x80, 4); // 0078CAA4
    copy_words(environment.underwater_color_84.data(), source + 0x118, 4); // 0078CACB
    for (std::size_t index = 0; index != 4; ++index)
        copy_words(environment.directional_colors_44[index].data(),
            source + 0x90 + index * 0x10, 4); // 0078CAFB..0078CB90

    auto* owner = copy_first_scalar_and_capture(environment.scalars_08[0],
        source + 0xd0, actual_environment_private_b4);
    copy_scalar(environment.scalars_08[1], source + 0xd4);
    copy_scalar(environment.scalars_08[2], source + 0xd8);
    copy_scalar(environment.scalars_08[3], source + 0xdc);
    copy_scalar(environment.scalars_08[4], source + 0xe0);
    copy_scalar(environment.scalars_08[5], source + 0xe4);
    copy_scalar(environment.scalars_08[9], source + 0x104);
    copy_scalar(environment.scalars_08[10], source + 0x108);
    copy_scalar(environment.scalars_08[6], source + 0x10c);
    copy_scalar(environment.scalars_08[8], source + 0x110);
    copy_scalar(environment.scalars_08[7], source + 0x114);

    if (!owner) return missing_owner(error); // captured at CB9A, consumed at CBFB
    set_system_fog_color_00b84c40(*owner, environment.color_34.data());
    for (std::uint32_t index = 0; index != 4; ++index) {
        owner = load_owner(actual_environment_private_b4); // 0078CC02 each iteration
        if (!owner) return missing_owner(error);
        set_system_fog_directional_color_00b84fa0(*owner,
            environment.directional_colors_44[index].data(), index);
    }

    std::uint32_t bits;
    owner = load_scalar_and_owner(environment.scalars_08[4], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_74_00b84d30(*owner, bits); // CC1A: +18, before+14
    owner = load_scalar_and_owner(environment.scalars_08[3], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_78_00b84d40(*owner, bits); // CC2C: +14
    owner = load_scalar_and_owner(environment.scalars_08[0], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_68_00b84d00(*owner, bits); // CC3E: +08
    owner = load_scalar_and_owner(environment.scalars_08[1], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_6c_00b84d10(*owner, bits); // CC50: +0C
    owner = load_scalar_and_owner(environment.scalars_08[2], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_70_00b84d20(*owner, bits); // CC62: +10
    owner = load_scalar_and_owner(environment.scalars_08[5], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_7c_00b84d50(*owner, bits); // CC74: +1C
    owner = load_scalar_and_owner(environment.scalars_08[9], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_80_00b84d60(*owner, bits); // CC86: +2C
    owner = load_scalar_and_owner(environment.scalars_08[10], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_84_00b84d80(*owner, bits); // CC98: +30
    owner = load_scalar_and_owner(environment.scalars_08[6], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_88_00b84dc0(*owner, bits); // CCAA: +20
    owner = load_scalar_and_owner(environment.scalars_08[8], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_8c_00b84de0(*owner, bits); // CCBC: +28
    owner = load_scalar_and_owner(environment.scalars_08[7], actual_environment_private_b4, bits);
    if (!owner) return missing_owner(error);
    set_system_fog_scalar_90_00b84e00(*owner, bits); // CCCE: +24
    return true;
}

} // namespace bsp
