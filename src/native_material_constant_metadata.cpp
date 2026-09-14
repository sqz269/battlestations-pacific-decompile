#include "bsp/native_material_constant_metadata.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material constant metadata requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeMaterialStorage, parameters_80) == 0x80);

__declspec(naked) void* const* __fastcall get_native_material_parameter_table_00b17390(
    const NativeMaterialStorage*) noexcept {
    __asm {
        lea eax, [ecx + 80h]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall get_native_vertex_declaration_element_count_00b47900(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 10h]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall get_native_shader_constant_register_count_00b5b880(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 4]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall get_native_optimized_animator_type_id_00b75e50(
    const volatile std::uint32_t&) noexcept {
    __asm {
        mov eax, dword ptr [ecx]
        ret
    }
}

__declspec(naked) void* __fastcall get_native_light_shadow_map_owner_00b7aab0(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 174h]
        ret
    }
}
} // namespace bsp
