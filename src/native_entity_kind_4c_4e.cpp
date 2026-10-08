#include "bsp/native_entity_kind_4c_4e.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These complete raw entries require MSVC Win32.
#endif

namespace bsp {

// COMPLETE[0042C010,0042C032) 34B/11 instructions.
__declspec(naked) std::uint32_t __fastcall native_entity_is_kind_0042c010(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 04Ch
        je accept_0042c010
        test eax, eax
        je accept_0042c010
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_0042c010
        xor eax, eax
        ret 4
    accept_0042c010:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[004351E0,00435207) 39B/13 instructions; not second4E 00435360.
__declspec(naked) std::uint32_t __fastcall native_entity_is_kind_004351e0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 04Eh
        je accept_004351e0
        cmp eax, 04Ch
        je accept_004351e0
        test eax, eax
        je accept_004351e0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_004351e0
        xor eax, eax
        ret 4
    accept_004351e0:
        mov eax, 1
        ret 4
    }
}

} // namespace bsp
