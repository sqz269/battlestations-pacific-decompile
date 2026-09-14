#include "bsp/native_model_group_selection.hpp"
#include "bsp/native_node_visibility_factor.hpp"
#include "bsp/native_particle_model_update.hpp"
#include <cstddef>

namespace bsp {
static_assert(sizeof(NativeModelGroupSelectionAccess) == 0x14);
static_assert(offsetof(NativeModelGroupSelectionAccess, comparison_00d7a24c) == 0x00);
static_assert(offsetof(NativeModelGroupSelectionAccess, comparison_00d7a218) == 0x04);
static_assert(offsetof(NativeModelGroupSelectionAccess, displacement_00cf81f0) == 0x08);
static_assert(offsetof(NativeModelGroupSelectionAccess, invalid_parameter_00bf6713) == 0x0c);
static_assert(offsetof(NativeModelGroupSelectionAccess, node_virtual2c) == 0x10);

// The extra entry push retains only access. Original local offsets are intact;
// incoming selected and its final reused float spill move by four bytes.
// Inline assembly retains the original exception/rounding and memory order.
__declspec(naked) void __fastcall select_native_model_group_00710bb0(
    void*, const NativeModelGroupSelectionAccess*, std::uint32_t) {
    __asm {
        push edx // borrowed access survives native EDX scratch use
        // 00710bb0
        sub esp,028h
        // 00710bb3
        push ebp
        // 00710bb4
        push esi
        // 00710bb5
        mov esi,dword ptr [esp + 038h]
        // 00710bb9
        xor edx,edx
        // 00710bbb
        cmp esi,edx
        // 00710bbd
        mov ebp,ecx
        // 00710bbf
        jz L00710be8
        // 00710bc1
        jle L00710df9
        // 00710bc7
        mov ecx,dword ptr [ebp + 016ch]
        // 00710bcd
        cmp ecx,edx
        // 00710bcf
        jz L00710df9
        // 00710bd5
        mov eax,dword ptr [ebp + 0170h]
        // 00710bdb
        sub eax,ecx
        // 00710bdd
        sar eax,04h
        // 00710be0
        cmp esi,eax
        // 00710be2
        jnc L00710df9
    L00710be8: // 00710be8
        mov eax,dword ptr [ebp + 016ch]
        // 00710bee
        cmp eax,edx
        // 00710bf0
        jz L00710df9
        // 00710bf6
        mov ecx,dword ptr [ebp + 0170h]
        // 00710bfc
        sub ecx,eax
        // 00710bfe
        sar ecx,04h
        // 00710c01
        jz L00710df9
        // 00710c07
        push ebx
        // 00710c08
        mov dword ptr [esp + 0ch],edx
        // 00710c0c
        push edi
    L00710c0d: // 00710c0d
        mov ecx,dword ptr [ebp + 016ch]
        // 00710c13
        test ecx,ecx
        // 00710c15
        mov dword ptr [esp + 014h],edx
        // 00710c19
        jz L00710d39
        // 00710c1f
        mov eax,dword ptr [ebp + 0170h]
        // 00710c25
        sub eax,ecx
        // 00710c27
        sar eax,04h
        // 00710c2a
        cmp edx,eax
        // 00710c2c
        jnc L00710d26
        // 00710c32
        cmp edx,esi
        // 00710c34
        jz L00710d19
        // 00710c3a
        test ecx,ecx
        // 00710c3c
        jz L00710c4d
        // 00710c3e
        mov eax,dword ptr [ebp + 0170h]
        // 00710c44
        sub eax,ecx
        // 00710c46
        sar eax,04h
        // 00710c49
        cmp edx,eax
        // 00710c4b
        jc L00710c52
    L00710c4d: // 00710c4d
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710c52: // 00710c52
        mov edi,dword ptr [ebp + 016ch]
        // 00710c58
        add edi,dword ptr [esp + 010h]
        // 00710c5c
        mov ebx,dword ptr [edi + 04h]
        // 00710c5f
        cmp ebx,dword ptr [edi + 08h]
        // 00710c62
        jbe L00710c70
        // 00710c64
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
        // 00710c69
        lea esp,[esp]
    L00710c70: // 00710c70
        mov esi,dword ptr [edi + 08h]
        // 00710c73
        cmp dword ptr [edi + 04h],esi
        // 00710c76
        jbe L00710c7d
        // 00710c78
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710c7d: // 00710c7d
        cmp ebx,esi
        // 00710c7f
        jz L00710d11
        // 00710c85
        cmp ebx,dword ptr [edi + 08h]
        // 00710c88
        jc L00710c8f
        // 00710c8a
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710c8f: // 00710c8f
        mov esi,dword ptr [ebx]
        // 00710c91
        movss xmm0,dword ptr [esi + 0ach]
        // 00710c99
        push eax
        mov eax, dword ptr [esp + 03ch]
        mov eax, dword ptr [eax + 00h]
        ucomiss xmm0, dword ptr [eax]
        pop eax
        // 00710ca0
        lahf
        // 00710ca1
        test ah,044h
        // 00710ca4
        jp L00710cff
        // 00710ca6
        fldz
        // 00710ca8
        push 00h
        // 00710caa
        push ecx
        // 00710cab
        fstp dword ptr [esp]
        // 00710cae
        mov ecx,esi
        // 00710cb0
        call set_native_node_visibility_factor_00b6da70
        // 00710cb5
        lea edx,[esp + 02ch]
        // 00710cb9
        push edx
        // 00710cba
        mov ecx,esi
        // 00710cbc
        call copy_native_node_local_position_00b6e0a0
        // 00710cc1
        fld dword ptr [eax + 04h]
        // 00710cc4
        push edx
        mov edx, dword ptr [esp + 03ch]
        mov edx, dword ptr [edx + 08h]
        fadd qword ptr [edx]
        pop edx
        // 00710cca
        lea ecx,[esp + 020h]
        // 00710cce
        push ecx
        // 00710ccf
        mov ecx,esi
        // 00710cd1
        fstp dword ptr [esp + 01ch]
        // 00710cd5
        fld dword ptr [eax + 08h]
        // 00710cd8
        fldz
        // 00710cda
        fadd st(1),st(0)
        // 00710cdc
        fxch
        // 00710cde
        fstp dword ptr [esp + 020h]
        // 00710ce2
        fadd dword ptr [eax]
        // 00710ce4
        mov eax,dword ptr [esi]
        // 00710ce6
        mov edx,dword ptr [eax + 02ch]
        // 00710ce9
        fstp dword ptr [esp + 024h]
        // 00710ced
        fld dword ptr [esp + 01ch]
        // 00710cf1
        fstp dword ptr [esp + 028h]
        // 00710cf5
        fld dword ptr [esp + 020h]
        // 00710cf9
        fstp dword ptr [esp + 02ch]
        // 00710cfd
        mov eax, dword ptr [esp + 03ch] // position argument is already pushed
        call dword ptr [eax + 010h] // ECX node, EDX captured target; RET4
    L00710cff: // 00710cff
        cmp ebx,dword ptr [edi + 08h]
        // 00710d02
        jc L00710d09
        // 00710d04
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710d09: // 00710d09
        add ebx,04h
        // 00710d0c
        jmp L00710c70
    L00710d11: // 00710d11
        mov edx,dword ptr [esp + 014h]
        // 00710d15
        mov esi,dword ptr [esp + 040h]
    L00710d19: // 00710d19
        add edx,01h
        // 00710d1c
        add dword ptr [esp + 010h],010h
        // 00710d21
        jmp L00710c0d
    L00710d26: // 00710d26
        test ecx,ecx
        // 00710d28
        jz L00710d39
        // 00710d2a
        mov eax,dword ptr [ebp + 0170h]
        // 00710d30
        sub eax,ecx
        // 00710d32
        sar eax,04h
        // 00710d35
        cmp esi,eax
        // 00710d37
        jc L00710d3e
    L00710d39: // 00710d39
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710d3e: // 00710d3e
        shl esi,04h
        // 00710d41
        add esi,dword ptr [ebp + 016ch]
        // 00710d47
        mov ebx,esi
        // 00710d49
        mov edi,dword ptr [ebx + 04h]
        // 00710d4c
        cmp edi,dword ptr [ebx + 08h]
        // 00710d4f
        jbe L00710d56
        // 00710d51
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710d56: // 00710d56
        mov esi,dword ptr [ebx + 08h]
        // 00710d59
        cmp dword ptr [ebx + 04h],esi
        // 00710d5c
        jbe L00710d63
        // 00710d5e
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710d63: // 00710d63
        cmp edi,esi
        // 00710d65
        jz L00710df7
        // 00710d6b
        cmp edi,dword ptr [ebx + 08h]
        // 00710d6e
        jc L00710d75
        // 00710d70
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710d75: // 00710d75
        mov esi,dword ptr [edi]
        // 00710d77
        movss xmm0,dword ptr [esi + 0ach]
        // 00710d7f
        push eax
        mov eax, dword ptr [esp + 03ch]
        mov eax, dword ptr [eax + 04h]
        ucomiss xmm0, dword ptr [eax]
        pop eax
        // 00710d86
        lahf
        // 00710d87
        test ah,044h
        // 00710d8a
        jp L00710de5
        // 00710d8c
        fld1
        // 00710d8e
        push 00h
        // 00710d90
        push ecx
        // 00710d91
        fstp dword ptr [esp]
        // 00710d94
        mov ecx,esi
        // 00710d96
        call set_native_node_visibility_factor_00b6da70
        // 00710d9b
        lea eax,[esp + 02ch]
        // 00710d9f
        push eax
        // 00710da0
        mov ecx,esi
        // 00710da2
        call copy_native_node_local_position_00b6e0a0
        // 00710da7
        fld dword ptr [eax + 04h]
        // 00710daa
        push edx
        mov edx, dword ptr [esp + 03ch]
        mov edx, dword ptr [edx + 08h]
        fsub qword ptr [edx]
        pop edx
        // 00710db0
        mov edx,dword ptr [esi]
        // 00710db2
        mov edx,dword ptr [edx + 02ch]
        // 00710db5
        mov ecx,esi
        // 00710db7
        fstp dword ptr [esp + 040h]
        // 00710dbb
        fld dword ptr [eax + 08h]
        // 00710dbe
        fldz
        // 00710dc0
        fsub st(1),st(0)
        // 00710dc2
        fxch
        // 00710dc4
        fstp dword ptr [esp + 01ch]
        // 00710dc8
        fsubr dword ptr [eax]
        // 00710dca
        lea eax,[esp + 020h]
        // 00710dce
        push eax
        // 00710dcf
        fstp dword ptr [esp + 024h]
        // 00710dd3
        fld dword ptr [esp + 044h]
        // 00710dd7
        fstp dword ptr [esp + 028h]
        // 00710ddb
        fld dword ptr [esp + 020h]
        // 00710ddf
        fstp dword ptr [esp + 02ch]
        // 00710de3
        mov eax, dword ptr [esp + 03ch] // position argument is already pushed
        call dword ptr [eax + 010h] // ECX node, EDX captured target; RET4
    L00710de5: // 00710de5
        cmp edi,dword ptr [ebx + 08h]
        // 00710de8
        jc L00710def
        // 00710dea
        mov eax, dword ptr [esp + 038h]
        call dword ptr [eax + 0ch]
    L00710def: // 00710def
        add edi,04h
        // 00710df2
        jmp L00710d56
    L00710df7: // 00710df7
        pop edi
        // 00710df8
        pop ebx
    L00710df9: // 00710df9
        pop esi
        // 00710dfa
        pop ebp
        // 00710dfb
        add esp,028h
        // 00710dfe
        pop edx // discard saved access, preserve native RET4
        ret 04h
    }
}
} // namespace bsp
