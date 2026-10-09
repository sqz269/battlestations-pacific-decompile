#include "bsp/native_mcommandbuilding_kind.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "native command-building kind entry requires Win32");

__declspec(naked) std::uint32_t __fastcall native_mcommandbuilding_is_kind_006f58e0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]      // 006F58E0: raw stacked query
        cmp eax, 0x1c                   // 006F58E4
        jz matched                      // 006F58E7
        cmp eax, 0x1b                   // 006F58E9
        jz matched                      // 006F58EC
        cmp eax, 5                      // 006F58EE
        jz matched                      // 006F58F1
        cmp eax, 4                      // 006F58F3
        jz matched                      // 006F58F6
        cmp eax, 2                      // 006F58F8
        jz matched                      // 006F58FB
        cmp eax, 1                      // 006F58FD
        jz matched                      // 006F5900
        test eax, eax                   // 006F5902
        jz matched                      // 006F5904
        cmp eax, dword ptr [ecx + 0xc4]  // 006F5906: sole late actual read
        jz matched                      // 006F590C
        xor eax, eax                    // 006F590E
        ret 4                           // 006F5910
    matched:
        mov eax, 1                      // 006F5913
        ret 4                           // 006F5918
    }
}

} // namespace bsp
