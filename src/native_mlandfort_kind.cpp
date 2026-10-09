#include "bsp/native_mlandfort_kind.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

__declspec(naked) std::uint32_t __fastcall native_mlandfort_is_kind_006f5890(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]        // 006F5890
        cmp eax, 1bh                        // 006F5894
        jz short mlandfort_matched          // 006F5897
        cmp eax, 5                          // 006F5899
        jz short mlandfort_matched          // 006F589C
        cmp eax, 4                          // 006F589E
        jz short mlandfort_matched          // 006F58A1
        cmp eax, 2                          // 006F58A3
        jz short mlandfort_matched          // 006F58A6
        cmp eax, 1                          // 006F58A8
        jz short mlandfort_matched          // 006F58AB
        test eax, eax                       // 006F58AD: exact zero test
        jz short mlandfort_matched          // 006F58AF
        cmp eax, dword ptr [ecx + 0c4h]     // 006F58B1: sole late receiver read
        jz short mlandfort_matched          // 006F58B7
        xor eax, eax                        // 006F58B9
        ret 4                               // 006F58BB
    mlandfort_matched:
        mov eax, 1                          // 006F58BE
        ret 4                               // 006F58C3
    }
}

} // namespace bsp
