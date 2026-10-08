#include "bsp/native_entity_kind_4d_second4e.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These complete raw entries require MSVC Win32.
#endif

namespace bsp {

// COMPLETE [004F1800,004F182C), 44B/15 instructions. SpawnPoint is provisional.
__declspec(naked) std::uint32_t __fastcall native_entity_is_kind_004f1800(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 04Dh
        je accept_004f1800
        cmp eax, 02h
        je accept_004f1800
        cmp eax, 01h
        je accept_004f1800
        test eax, eax
        je accept_004f1800
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_004f1800
        xor eax, eax
        ret 4
    accept_004f1800:
        mov eax, 1
        ret 4
    }
}

// COMPLETE [00435360,00435387), 39B/13 instructions; DISTINCT from 004351E0.
__declspec(naked) std::uint32_t __fastcall native_entity_is_kind_00435360(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 04Eh
        je accept_00435360
        cmp eax, 04Ch
        je accept_00435360
        test eax, eax
        je accept_00435360
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_00435360
        xor eax, eax
        ret 4
    accept_00435360:
        mov eax, 1
        ret 4
    }
}

} // namespace bsp
