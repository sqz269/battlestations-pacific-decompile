#include "bsp/native_unit_wake_handoff.hpp"
#include "bsp/native_entity_pose.hpp"
#include "bsp/native_unit_wake_copy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw unit wake handoff requires MSVC Win32 assembly.
#endif

namespace bsp {
// Complete Original182/42, SHA-256
// e4929517cb6a00c4c7bbf62bb0127159641c5a26194246e75b0b4cfd89266e90.
// Only three natural CALL operands differ in Source. All three calls bind
// complete canonical raw functions; no service, snapshot or typed overlay.
__declspec(naked) void __fastcall handoff_native_unit_wake_00815e20(
    void*, void*, void*) noexcept {
    __asm {
        sub esp, 0x18
        push esi
        push edi
        mov edi, dword ptr [esp + 0x24]
        movss xmm0, dword ptr [edi + 0xfa0]
        mov esi, ecx
        cmp byte ptr [esi + 0xc8], 0
        movss dword ptr [esp + 8], xmm0
        movss xmm0, dword ptr [edi + 0xfa8]
        movss dword ptr [esp + 0x10], xmm0
        jnz old_pose
        call refresh_native_entity_pose_00414db0
    old_pose:
        cmp byte ptr [edi + 0xc8], 0
        jnz poses_ready
        mov ecx, edi
        call refresh_native_entity_pose_00414db0
    poses_ready:
        fld dword ptr [edi + 0xfc]
        add edi, 0xbd0
        fsub dword ptr [esi + 0xfc]
        push edi
        lea ecx, [esi + 0xbd0]
        fstp dword ptr [esp + 0x18]
        fld dword ptr [edi - 0xacc]
        fsub dword ptr [esi + 0x104]
        fstp dword ptr [esp + 0x20]
        fld dword ptr [esp + 0x18]
        fadd dword ptr [esp + 0xc]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [esp + 0x20]
        fadd dword ptr [esp + 0x14]
        fstp dword ptr [esp + 0x14]
        call copy_native_unit_wake_00815680
        fld dword ptr [esp + 8]
        xorps xmm0, xmm0
        fstp dword ptr [esi + 0xfa0]
        pop edi
        fld dword ptr [esp + 0xc]
        movss dword ptr [esi + 0xfa4], xmm0
        fstp dword ptr [esi + 0xfa8]
        pop esi
        add esp, 0x18
        ret 4
    }
}
} // namespace bsp
