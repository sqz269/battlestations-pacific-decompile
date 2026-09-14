#include "bsp/native_light_environment_queries.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native light environment queries require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) const void* __fastcall get_native_light_environment_ambient_00b7aa20(
    const void*) noexcept {
    __asm {
        lea eax, [ecx + 18h]
        ret
    }
}

__declspec(naked) const void* __fastcall get_native_light_environment_mode3_ambient_00b7aa30(
    const void*) noexcept {
    __asm {
        lea eax, [ecx + 28h]
        ret
    }
}

__declspec(naked) const void* __fastcall get_native_light_environment_cube_face_00b7aa40(
    const void*, void*, std::uint32_t) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        shl eax, 4
        lea eax, [eax + ecx + 38h]
        ret 4
    }
}
} // namespace bsp
