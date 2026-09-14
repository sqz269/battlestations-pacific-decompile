#include "bsp/native_node_visibility_factor.hpp"
#include "bsp/native_node_construction.hpp"
#include <cstddef>

namespace bsp {
static_assert(offsetof(NativeNodeStorage, first_child_34) == 0x34);
static_assert(offsetof(NativeNodeStorage, next_sibling_3c) == 0x3c);
static_assert(offsetof(NativeNodeStorage, scalar_ac) == 0xac);

__declspec(naked) void __fastcall set_native_node_visibility_factor_00b6da70(
    NativeNodeStorage*, void*, float, std::uint32_t) {
    __asm {
        movss xmm0, dword ptr [esp + 04h] // 00b6da70
        push ebx
        mov ebx, dword ptr [esp + 0ch]
        test bl, bl
        movss dword ptr [ecx + 0ach], xmm0
        jz visibility_done
        push esi
        mov esi, dword ptr [ecx + 34h]
        test esi, esi
        jz visibility_children_done
        nop
    visibility_child: // 00b6da90
        fld dword ptr [esp + 0ch]
        push ebx
        push ecx
        mov ecx, esi
        fstp dword ptr [esp]
        call set_native_node_visibility_factor_00b6da70
        mov esi, dword ptr [esi + 3ch] // reload after recursion
        test esi, esi
        jnz visibility_child
    visibility_children_done:
        pop esi
    visibility_done:
        pop ebx
        ret 08h
    }
}
} // namespace bsp
