#include "bsp/native_surface_dimensions.hpp"

namespace bsp {

__declspec(naked) std::uint32_t __fastcall get_native_surface_width_00b3cd10(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 1ch]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall get_native_surface_height_00b3cd20(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 20h]
        ret
    }
}

} // namespace bsp
