#include "bsp/native_particle_record_clear.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-record clear requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void __fastcall clear_native_particle_record_array_004dcaa0(
    NativeResourceRecordVectorStorage&, NativeParticleRecordResizeContext&) {
    __asm {
        push 0
        call resize_native_particle_record_array_004dc410
        ret
    }
}

} // namespace bsp
