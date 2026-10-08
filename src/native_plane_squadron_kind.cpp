#include "bsp/native_plane_squadron_kind.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "native squadron kind entry requires Win32");

__declspec(naked) std::uint32_t __fastcall native_plane_squadron_is_kind_007efb00(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 007EFB00
        cmp eax, 0x18
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]   // 007EFB17: sole receiver read
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 007EFB24
        ret 4
    }
}

} // namespace bsp
