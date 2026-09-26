#include "bsp/native_particle_clock_sink_release.hpp"
#include "bsp/native_render_context.hpp"

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

void __fastcall release_native_particle_clock_sink_004ddb40(
    void*, NativeParticleClockAtomic const volatile& actual_decrement_00ce2220,
    void* actual_sink, NativeRenderActualOwners& actual_owners) {
    void* const captured_sink = actual_sink;
    auto* const actual_count = reinterpret_cast<volatile long*>(
        static_cast<char*>(captured_sink) + 4);
    const long current_count = actual_decrement_00ce2220(actual_count);
    if (current_count == 0)
        dispatch_native_render_actual_owner_zero(actual_owners, captured_sink);
}
} // namespace bsp
