#include "bsp/native_renderer_current_depth_surface.hpp"

namespace bsp {

__declspec(naked) void* __fastcall get_native_renderer_current_depth_surface_00b20090(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 198ch]
        ret
    }
}

} // namespace bsp
