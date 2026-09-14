#include "bsp/native_particle_clock_publication_base.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-clock publication cleanup requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void __fastcall clear_native_particle_clock_publication_base_004b4f10(
    void*, void* volatile&) noexcept {
    __asm {
        mov dword ptr [edx], 0
        mov dword ptr [ecx], 0CE3818h
        ret
    }
}
} // namespace bsp
