#include "bsp/native_unit_group_set_leader.hpp"
#include "bsp/native_unit_wake_handoff.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw unit group leader assignment requires MSVC Win32 assembly.
#endif

namespace bsp {
// Whole Original36/16, SHA-256
// c8663953b174916475ec8175c41d653c92f494f2ea3ec42768d762cdab158787.
// Only the natural CALL operand at +18h relocates, directly to the complete
// canonical wake handoff. Keep its returned volatile registers and flags.
__declspec(naked) void __fastcall set_native_unit_group_leader_0070d0c0(
    void*, void*, void*, void*) noexcept {
    __asm {
        mov eax, dword ptr [esp + 8]
        push esi
        mov esi, dword ptr [esp + 8]
        cmp esi, eax
        push edi
        mov edi, ecx
        je done
        test eax, eax
        je publish
        push eax
        mov ecx, esi
        call handoff_native_unit_wake_00815e20
    publish:
        mov dword ptr [edi + 0x14], esi
    done:
        pop edi
        pop esi
        ret 8
    }
}
} // namespace bsp
