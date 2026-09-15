#include "bsp/native_particle_clock_sink_release.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-clock sink release requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);

__declspec(naked) void __fastcall release_native_particle_clock_sink_004ddb40(
    void*, NativeParticleClockAtomic const volatile&, void*) {
    __asm {
        push esi
        mov esi, dword ptr [esp + 8]
        lea eax, dword ptr [esi + 4]
        push eax
        call dword ptr [edx]
        test eax, eax
        jnz complete
        mov edx, dword ptr [esi]
        mov eax, dword ptr [edx]
        mov ecx, esi
        call eax
    complete:
        pop esi
        ret 4
    }
}
} // namespace bsp
