#include "bsp/native_entity_root_kind.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These complete raw entries require MSVC Win32.
#endif

namespace bsp {

// COMPLETE[0042B8F0,0042B90D) 29B/9 instructions.
__declspec(naked) std::uint32_t __fastcall native_entity_root_is_kind_0042b8f0(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        test eax, eax
        je accept_0042b8f0
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_0042b8f0
        xor eax, eax
        ret 4
    accept_0042b8f0:
        mov eax, 1
        ret 4
    }
}

// COMPLETE[0047F190,0047F1B2) 34B/11 instructions.
__declspec(naked) std::uint32_t __fastcall native_entity_root_is_kind_0047f190(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        cmp eax, 001h
        je accept_0047f190
        test eax, eax
        je accept_0047f190
        cmp eax, dword ptr [ecx + 0C4h]
        je accept_0047f190
        xor eax, eax
        ret 4
    accept_0047f190:
        mov eax, 1
        ret 4
    }
}

} // namespace bsp
