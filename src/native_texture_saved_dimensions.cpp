#include "bsp/native_texture_saved_dimensions.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture saved dimensions require MSVC Win32.
#endif

namespace bsp {

__declspec(naked) std::uint32_t __fastcall
get_native_texture_saved_width_00b3ce70(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 34h]
        ret
    }
}

__declspec(naked) std::uint32_t __fastcall
get_native_texture_saved_height_00b3ce80(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 38h]
        ret
    }
}

} // namespace bsp
