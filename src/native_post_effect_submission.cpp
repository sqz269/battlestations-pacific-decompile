#include "bsp/native_post_effect_submission.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera viewport access requires MSVC Win32.
#endif

namespace bsp {

__declspec(naked) void* __fastcall get_native_camera_viewport_00b6fde0(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 180h] // B6FDE0: actual borrowed pointer.
        ret // B6FDE6: no native stack arguments or ownership effect.
    }
}

} // namespace bsp
