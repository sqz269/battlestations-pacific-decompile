#include "bsp/native_unit_list_erase.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This literal native entry requires MSVC Win32.
#endif

namespace bsp {

// Original complete 004837D0..0048381F SHA-256:
// 0909c4202ea2816202a70c2f3dc821e0512f88069d0377378aa1adb85f3f1fc2
// All 71 bytes outside CALL operands 43..46 and 66..69 remain literal.
// Both CALLs use the actual unchanged canonical CDECL free, with the actual
// node already on its stack. No adapter, fake node, guard or alternate heap.
// The successor is retained in ESI BEFORE free; no node is read after free.
__declspec(naked) void* __fastcall erase_native_unit_list_004837d0(
    void*, void*, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov edx, dword ptr [eax]
        test edx, edx
        push esi
        jz L_no_previous
        mov esi, dword ptr [eax + 4]
        mov dword ptr [edx + 4], esi
        jmp L_previous_done
    L_no_previous:
        mov edx, dword ptr [eax + 4]
        mov dword ptr [ecx + 4], edx
    L_previous_done:
        // Keep the original fresh next/previous reads and mutation order.
        cmp dword ptr [eax + 4], 0
        mov edx, dword ptr [eax]
        jz L_no_successor
        mov esi, dword ptr [eax + 4]
        mov dword ptr [esi], edx
        add dword ptr [ecx], -1
        push eax
        call singleton_lifetime_free
        // The final flags are those of this stack ADD, not the count update.
        add esp, 4
        mov eax, esi
        pop esi
        ret 4
    L_no_successor:
        add dword ptr [ecx], -1
        push eax
        xor esi, esi
        mov dword ptr [ecx + 8], edx
        call singleton_lifetime_free
        add esp, 4
        mov eax, esi
        pop esi
        ret 4
    }
}

}  // namespace bsp
