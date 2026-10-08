#include "bsp/native_entity_base_kind.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These complete raw entries require MSVC Win32.
#endif

namespace bsp {

// COMPLETE[004F1750,004F1777) 39B/13 instructions.
__declspec(naked) std::uint32_t __fastcall native_entity_base_is_kind_004f1750(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 002h
        je accept_004f1750
        cmp eax, 001h
        je accept_004f1750
        test eax, eax
        je accept_004f1750
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_004f1750
        xor eax, eax
        ret 4
    accept_004f1750:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[006D1610,006D163C) 44B/15 instructions.
__declspec(naked) std::uint32_t __fastcall native_entity_base_is_kind_006d1610(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 004h
        je accept_006d1610
        cmp eax, 002h
        je accept_006d1610
        cmp eax, 001h
        je accept_006d1610
        test eax, eax
        je accept_006d1610
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_006d1610
        xor eax, eax
        ret 4
    accept_006d1610:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[006D1650,006D1681) 49B/17 instructions.
__declspec(naked) std::uint32_t __fastcall native_entity_base_is_kind_006d1650(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 005h
        je accept_006d1650
        cmp eax, 004h
        je accept_006d1650
        cmp eax, 002h
        je accept_006d1650
        cmp eax, 001h
        je accept_006d1650
        test eax, eax
        je accept_006d1650
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_006d1650
        xor eax, eax
        ret 4
    accept_006d1650:
        mov eax, 1
        ret 4
    }
}

} // namespace bsp
