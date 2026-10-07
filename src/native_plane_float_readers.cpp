#include "bsp/native_plane_float_readers.hpp"

namespace bsp {

// Keep the native load and return, including ST0 and ambient x87 effects.
// A C++ float temporary would introduce an additional store or conversion.
__declspec(naked) float __fastcall native_plane_heading_0074e260(
    const void*, void*) {
    __asm {
        fld dword ptr [ecx + 0xc6c] // 0074E260
        ret                       // 0074E266
    }
}

__declspec(naked) float __fastcall native_plane_cached_speed_007b8e60(
    const void*, void*) {
    __asm {
        fld dword ptr [ecx + 0xb1c] // 007B8E60
        ret                       // 007B8E66
    }
}

} // namespace bsp
