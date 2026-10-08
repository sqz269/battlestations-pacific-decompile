#include "bsp/native_entity_class03_kind.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This complete raw entry requires MSVC Win32.
#endif

namespace bsp {

// COMPLETE[00888EA0,00888EC7) 39B/13 instructions.
__declspec(naked) std::uint32_t __fastcall native_entity_class03_is_kind_00888ea0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 003h
        je accept_00888ea0
        cmp eax, 001h
        je accept_00888ea0
        test eax, eax
        je accept_00888ea0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_00888ea0
        xor eax, eax
        ret 4
    accept_00888ea0:
        mov eax, 1
        ret 4
    }
}

} // namespace bsp
