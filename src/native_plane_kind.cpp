#include "bsp/native_plane_kind.hpp"

namespace bsp {
__declspec(naked) std::uint32_t __fastcall native_plane_is_kind_0074e400(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 0074E400
        cmp eax, 0x0f
        jz matched
        cmp eax, 5
        jz matched
        cmp eax, 4
        jz matched
        cmp eax, 2
        jz matched
        cmp eax, 1
        jz matched
        test eax, eax
        jz matched
        cmp eax, dword ptr [ecx + 0xc4]
        jz matched
        xor eax, eax
        ret 4
    matched:
        mov eax, 1                       // 0074E42E
        ret 4
    }
}
} // namespace bsp
