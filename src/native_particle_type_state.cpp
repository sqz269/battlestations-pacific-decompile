#include "bsp/native_particle_type_state.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/random_threads.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle type state requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(offsetof(NativeParticleTypeStateAccess, random) == 0);
static_assert(offsetof(NativeParticleTypeStateAccess, zero_00d7a218) == 4);
static_assert(offsetof(NativeParticleTypeStateAccess, one_00d7a24c) == 8);
static_assert(offsetof(NativeParticleTypeStateAccess, random_scale_00d5da30) == 12);
static_assert(offsetof(NativeParticleTypeStateAccess, base_00d7a210) == 16);
static_assert(offsetof(NativeParticleTypeStateAccess, angle_scale_00d5daf8) == 20);
static_assert(offsetof(NativeParticleTypeStateAccess, floating_counter_00f8d384) == 24);
std::uint32_t __fastcall random_bridge(RandomStream stream, const NativeParticleTypeStateAccess* access) {
    return access->random->next_00bd2fc0(stream);
}
} // namespace

// Added callee-saved EBP holds borrowed access. Native's temporary EBP frame
// minimum saves this value on the stack; its draw recovers access from that save.
// Incoming argument offsets move by four; native local float spills do not.
__declspec(naked) void __fastcall initialize_native_particle_axial_state_00b059c0(
    void*, const NativeParticleTypeStateAccess*, void*, const void*) {
    __asm {
        push ebp
        mov ebp,edx
        sub esp,0x8 // 00b059c0
        push ebx // 00b059c3
        push esi // 00b059c4
        push edi // 00b059c5
        mov edi,ecx // 00b059c6
        mov eax,dword ptr [edi + 0x1c] // 00b059c8
        movss xmm0,dword ptr [eax] // 00b059cb
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b059cf
        pop edx
        lahf // 00b059d6
        test ah,0x44 // 00b059d7
        movss dword ptr [esp + 0xc],xmm0 // 00b059da
        jp l_00b059f2 // 00b059e0
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b059e2
        pop edx
        movss dword ptr [esp + 0x10],xmm0 // 00b059ea
        jmp l_00b05a15 // 00b059f0
    l_00b059f2:
        xor ecx,ecx // 00b059f2
        mov edx,ebp
        call random_bridge // 00b059f4
        mov dword ptr [esp + 0x10],eax // 00b059f9
        fild dword ptr [esp + 0x10] // 00b059fd
        fmul dword ptr [esp + 0xc] // 00b05a01
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b05a05
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05a0b
        pop edx
        fstp dword ptr [esp + 0x10] // 00b05a11
    l_00b05a15:
        mov ecx,dword ptr [edi + 0x1c] // 00b05a15
        movzx eax,word ptr [ecx + 0xa] // 00b05a18
        test ax,ax // 00b05a1c
        jnz l_00b05a2e // 00b05a1f
        movss xmm0,dword ptr [ecx + 0x4] // 00b05a21
        movss dword ptr [esp + 0xc],xmm0 // 00b05a26
        jmp l_00b05a4a // 00b05a2c
    l_00b05a2e:
        cmp ax,0x1 // 00b05a2e
        fldz // 00b05a32
        push ecx // 00b05a34
        fstp dword ptr [esp] // 00b05a35
        jnz l_00b05a41 // 00b05a38
        call evaluate_native_particle_linear_curve_00affa70 // 00b05a3a
        jmp l_00b05a46 // 00b05a3f
    l_00b05a41:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b05a41
    l_00b05a46:
        fstp dword ptr [esp + 0xc] // 00b05a46
    l_00b05a4a:
        fld dword ptr [esp + 0xc] // 00b05a4a
        mov esi,dword ptr [esp + 0x1c] // 00b05a4e
        fmul dword ptr [esp + 0x10] // 00b05a52
        fstp dword ptr [esi + 0x44] // 00b05a56
        mov ecx,dword ptr [edi + 0x2c] // 00b05a59
        movss xmm0,dword ptr [ecx] // 00b05a5c
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b05a60
        pop edx
        lahf // 00b05a67
        test ah,0x44 // 00b05a68
        movss dword ptr [esp + 0x10],xmm0 // 00b05a6b
        jp l_00b05a83 // 00b05a71
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b05a73
        pop edx
        movss dword ptr [esp + 0x1c],xmm0 // 00b05a7b
        jmp l_00b05aa6 // 00b05a81
    l_00b05a83:
        xor ecx,ecx // 00b05a83
        mov edx,ebp
        call random_bridge // 00b05a85
        mov dword ptr [esp + 0x1c],eax // 00b05a8a
        fild dword ptr [esp + 0x1c] // 00b05a8e
        fmul dword ptr [esp + 0x10] // 00b05a92
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b05a96
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05a9c
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b05aa2
    l_00b05aa6:
        fld dword ptr [esi + 0x18] // 00b05aa6
        fld dword ptr [esp + 0x1c] // 00b05aa9
        fld st(0) // 00b05aad
        fmulp st(2),st(0) // 00b05aaf
        fxch // 00b05ab1
        fstp dword ptr [esi + 0x18] // 00b05ab3
        fld dword ptr [esi + 0x1c] // 00b05ab6
        fmul st(0),st(1) // 00b05ab9
        fstp dword ptr [esi + 0x1c] // 00b05abb
        fmul dword ptr [esi + 0x20] // 00b05abe
        fstp dword ptr [esi + 0x20] // 00b05ac1
        mov edx,dword ptr [edi + 0x30] // 00b05ac4
        movss xmm0,dword ptr [edx] // 00b05ac7
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b05acb
        pop edx
        lahf // 00b05ad2
        test ah,0x44 // 00b05ad3
        movss dword ptr [esp + 0x10],xmm0 // 00b05ad6
        jp l_00b05ae8 // 00b05adc
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b05ade
        pop edx
        jmp l_00b05b11 // 00b05ae6
    l_00b05ae8:
        xor ecx,ecx // 00b05ae8
        mov edx,ebp
        call random_bridge // 00b05aea
        mov dword ptr [esp + 0x1c],eax // 00b05aef
        fild dword ptr [esp + 0x1c] // 00b05af3
        fmul dword ptr [esp + 0x10] // 00b05af7
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b05afb
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05b01
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b05b07
        movss xmm0,dword ptr [esp + 0x1c] // 00b05b0b
    l_00b05b11:
        movss dword ptr [esi + 0x48],xmm0 // 00b05b11
        mov eax,dword ptr [edi + 0x34] // 00b05b16
        movss xmm0,dword ptr [eax] // 00b05b19
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b05b1d
        pop edx
        lahf // 00b05b24
        test ah,0x44 // 00b05b25
        movss dword ptr [esp + 0x10],xmm0 // 00b05b28
        jp l_00b05b3a // 00b05b2e
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b05b30
        pop edx
        jmp l_00b05b63 // 00b05b38
    l_00b05b3a:
        xor ecx,ecx // 00b05b3a
        mov edx,ebp
        call random_bridge // 00b05b3c
        mov dword ptr [esp + 0x1c],eax // 00b05b41
        fild dword ptr [esp + 0x1c] // 00b05b45
        fmul dword ptr [esp + 0x10] // 00b05b49
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b05b4d
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05b53
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b05b59
        movss xmm0,dword ptr [esp + 0x1c] // 00b05b5d
    l_00b05b63:
        movss dword ptr [esi + 0x4c],xmm0 // 00b05b63
        mov ecx,dword ptr [edi + 0x8c] // 00b05b68
        movss xmm0,dword ptr [ecx] // 00b05b6e
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b05b72
        pop edx
        lahf // 00b05b79
        test ah,0x44 // 00b05b7a
        movss dword ptr [esp + 0x10],xmm0 // 00b05b7d
        jp l_00b05b8f // 00b05b83
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b05b85
        pop edx
        jmp l_00b05bb8 // 00b05b8d
    l_00b05b8f:
        xor ecx,ecx // 00b05b8f
        mov edx,ebp
        call random_bridge // 00b05b91
        mov dword ptr [esp + 0x1c],eax // 00b05b96
        fild dword ptr [esp + 0x1c] // 00b05b9a
        fmul dword ptr [esp + 0x10] // 00b05b9e
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b05ba2
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05ba8
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b05bae
        movss xmm0,dword ptr [esp + 0x1c] // 00b05bb2
    l_00b05bb8:
        movss dword ptr [esi + 0x58],xmm0 // 00b05bb8
        mov edx,dword ptr [edi + 0x38] // 00b05bbd
        movss xmm0,dword ptr [edx] // 00b05bc0
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b05bc4
        pop edx
        lahf // 00b05bcb
        test ah,0x44 // 00b05bcc
        movss dword ptr [esp + 0x10],xmm0 // 00b05bcf
        jp l_00b05be1 // 00b05bd5
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b05bd7
        pop edx
        jmp l_00b05c0a // 00b05bdf
    l_00b05be1:
        xor ecx,ecx // 00b05be1
        mov edx,ebp
        call random_bridge // 00b05be3
        mov dword ptr [esp + 0x1c],eax // 00b05be8
        fild dword ptr [esp + 0x1c] // 00b05bec
        fmul dword ptr [esp + 0x10] // 00b05bf0
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b05bf4
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05bfa
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b05c00
        movss xmm0,dword ptr [esp + 0x1c] // 00b05c04
    l_00b05c0a:
        movss dword ptr [esi + 0x5c],xmm0 // 00b05c0a
        mov eax,dword ptr [edi + 0x5c] // 00b05c0f
        movss xmm0,dword ptr [eax] // 00b05c12
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b05c16
        pop edx
        lahf // 00b05c1d
        test ah,0x44 // 00b05c1e
        movss dword ptr [esp + 0x10],xmm0 // 00b05c21
        jp l_00b05c33 // 00b05c27
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b05c29
        pop edx
        jmp l_00b05c5c // 00b05c31
    l_00b05c33:
        xor ecx,ecx // 00b05c33
        mov edx,ebp
        call random_bridge // 00b05c35
        mov dword ptr [esp + 0x1c],eax // 00b05c3a
        fild dword ptr [esp + 0x1c] // 00b05c3e
        fmul dword ptr [esp + 0x10] // 00b05c42
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b05c46
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05c4c
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b05c52
        movss xmm0,dword ptr [esp + 0x1c] // 00b05c56
    l_00b05c5c:
        movss dword ptr [esi + 0x3c],xmm0 // 00b05c5c
        cmp byte ptr [edi + 0x60],0x0 // 00b05c61
        jz l_00b05c87 // 00b05c65
        mov ebx,dword ptr [edi + 0x54] // 00b05c67
        push ebp // 00b05c6a
        mov ebp,dword ptr [edi + 0x50] // 00b05c6b
        xor ecx,ecx // 00b05c6e
        mov edx,dword ptr [esp]
        call random_bridge // 00b05c70
        sub ebx,ebp // 00b05c75
        add ebx,0x1 // 00b05c77
        xor edx,edx // 00b05c7a
        div ebx // 00b05c7c
        add edx,ebp // 00b05c7e
        cvtsi2ss xmm0,edx // 00b05c80
        pop ebp // 00b05c84
        jmp l_00b05c8c // 00b05c85
    l_00b05c87:
        cvtsi2ss xmm0,dword ptr [edi + 0x50] // 00b05c87
    l_00b05c8c:
        movss dword ptr [esi + 0x34],xmm0 // 00b05c8c
        movss dword ptr [esi + 0x30],xmm0 // 00b05c91
        cmp byte ptr [edi + 0x61],0x0 // 00b05c96
        jz l_00b05cc6 // 00b05c9a
        mov ebx,dword ptr [edi + 0x54] // 00b05c9c
        mov edi,dword ptr [edi + 0x50] // 00b05c9f
        xor ecx,ecx // 00b05ca2
        mov edx,ebp
        call random_bridge // 00b05ca4
        sub ebx,edi // 00b05ca9
        add ebx,0x1 // 00b05cab
        xor edx,edx // 00b05cae
        div ebx // 00b05cb0
        add edx,edi // 00b05cb2
        pop edi // 00b05cb4
        cvtsi2ss xmm0,edx // 00b05cb5
        movss dword ptr [esi + 0x38],xmm0 // 00b05cb9
        pop esi // 00b05cbe
        pop ebx // 00b05cbf
        add esp,0x8 // 00b05cc0
        pop ebp
        ret 0x8 // 00b05cc3
    l_00b05cc6:
        fld dword ptr [esi + 0x30] // 00b05cc6
        pop edi // 00b05cc9
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b05cca
        pop edx
        fstp dword ptr [esi + 0x38] // 00b05cd0
        pop esi // 00b05cd3
        pop ebx // 00b05cd4
        add esp,0x8 // 00b05cd5
        pop ebp
        ret 0x8 // 00b05cd8
    }
}

__declspec(naked) void __fastcall initialize_native_particle_floating_state_00b077d0(
    void*, const NativeParticleTypeStateAccess*, void*, const void*) {
    __asm {
        push ebp
        mov ebp,edx
        sub esp,0x8 // 00b077d0
        push ebx // 00b077d3
        push esi // 00b077d4
        push edi // 00b077d5
        mov edi,ecx // 00b077d6
        mov eax,dword ptr [edi + 0x1c] // 00b077d8
        movss xmm0,dword ptr [eax] // 00b077db
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b077df
        pop edx
        lahf // 00b077e6
        test ah,0x44 // 00b077e7
        movss dword ptr [esp + 0xc],xmm0 // 00b077ea
        jp l_00b07802 // 00b077f0
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b077f2
        pop edx
        movss dword ptr [esp + 0xc],xmm0 // 00b077fa
        jmp l_00b07825 // 00b07800
    l_00b07802:
        xor ecx,ecx // 00b07802
        mov edx,ebp
        call random_bridge // 00b07804
        mov dword ptr [esp + 0x10],eax // 00b07809
        fild dword ptr [esp + 0x10] // 00b0780d
        fmul dword ptr [esp + 0xc] // 00b07811
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b07815
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b0781b
        pop edx
        fstp dword ptr [esp + 0xc] // 00b07821
    l_00b07825:
        mov ecx,dword ptr [edi + 0x1c] // 00b07825
        fldz // 00b07828
        movzx eax,word ptr [ecx + 0xa] // 00b0782a
        test ax,ax // 00b0782e
        jnz l_00b07842 // 00b07831
        movss xmm0,dword ptr [ecx + 0x4] // 00b07833
        fstp st(0) // 00b07838
        movss dword ptr [esp + 0x10],xmm0 // 00b0783a
        jmp l_00b0785c // 00b07840
    l_00b07842:
        cmp ax,0x1 // 00b07842
        push ecx // 00b07846
        fstp dword ptr [esp] // 00b07847
        jnz l_00b07853 // 00b0784a
        call evaluate_native_particle_linear_curve_00affa70 // 00b0784c
        jmp l_00b07858 // 00b07851
    l_00b07853:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b07853
    l_00b07858:
        fstp dword ptr [esp + 0x10] // 00b07858
    l_00b0785c:
        fld dword ptr [esp + 0x10] // 00b0785c
        mov esi,dword ptr [esp + 0x1c] // 00b07860
        fmul dword ptr [esp + 0xc] // 00b07864
        fstp dword ptr [esi + 0x44] // 00b07868
        mov ecx,dword ptr [edi + 0x80] // 00b0786b
        movss xmm0,dword ptr [ecx] // 00b07871
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b07875
        pop edx
        lahf // 00b0787c
        test ah,0x44 // 00b0787d
        movss dword ptr [esp + 0x10],xmm0 // 00b07880
        jp l_00b07898 // 00b07886
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b07888
        pop edx
        movss dword ptr [esp + 0xc],xmm0 // 00b07890
        jmp l_00b078bb // 00b07896
    l_00b07898:
        xor ecx,ecx // 00b07898
        mov edx,ebp
        call random_bridge // 00b0789a
        mov dword ptr [esp + 0x1c],eax // 00b0789f
        fild dword ptr [esp + 0x1c] // 00b078a3
        fmul dword ptr [esp + 0x10] // 00b078a7
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b078ab
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b078b1
        pop edx
        fstp dword ptr [esp + 0xc] // 00b078b7
    l_00b078bb:
        mov ecx,dword ptr [edi + 0x80] // 00b078bb
        movzx eax,word ptr [ecx + 0xa] // 00b078c1
        test ax,ax // 00b078c5
        jnz l_00b078d7 // 00b078c8
        movss xmm0,dword ptr [ecx + 0x4] // 00b078ca
        movss dword ptr [esp + 0x1c],xmm0 // 00b078cf
        jmp l_00b078f3 // 00b078d5
    l_00b078d7:
        cmp ax,0x1 // 00b078d7
        fldz // 00b078db
        push ecx // 00b078dd
        fstp dword ptr [esp] // 00b078de
        jnz l_00b078ea // 00b078e1
        call evaluate_native_particle_linear_curve_00affa70 // 00b078e3
        jmp l_00b078ef // 00b078e8
    l_00b078ea:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b078ea
    l_00b078ef:
        fstp dword ptr [esp + 0x1c] // 00b078ef
    l_00b078f3:
        fld dword ptr [esp + 0x1c] // 00b078f3
        push edx
        mov edx,dword ptr [ebp+20] // borrowed current global
        fmul qword ptr [edx] // 00b078f7
        pop edx
        fmul dword ptr [esp + 0xc] // 00b078fd
        fstp dword ptr [esi + 0x50] // 00b07901
        mov edx,dword ptr [edi + 0x2c] // 00b07904
        movss xmm0,dword ptr [edx] // 00b07907
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b0790b
        pop edx
        lahf // 00b07912
        test ah,0x44 // 00b07913
        movss dword ptr [esp + 0x10],xmm0 // 00b07916
        jp l_00b0792e // 00b0791c
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b0791e
        pop edx
        movss dword ptr [esp + 0x1c],xmm0 // 00b07926
        jmp l_00b07951 // 00b0792c
    l_00b0792e:
        xor ecx,ecx // 00b0792e
        mov edx,ebp
        call random_bridge // 00b07930
        mov dword ptr [esp + 0x1c],eax // 00b07935
        fild dword ptr [esp + 0x1c] // 00b07939
        fmul dword ptr [esp + 0x10] // 00b0793d
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b07941
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b07947
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b0794d
    l_00b07951:
        fld dword ptr [esp + 0x1c] // 00b07951
        fld st(0) // 00b07955
        fmul dword ptr [esi + 0x18] // 00b07957
        fstp dword ptr [esi + 0x18] // 00b0795a
        fld dword ptr [esi + 0x1c] // 00b0795d
        fmul st(0),st(1) // 00b07960
        fstp dword ptr [esi + 0x1c] // 00b07962
        fmul dword ptr [esi + 0x20] // 00b07965
        fstp dword ptr [esi + 0x20] // 00b07968
        mov eax,dword ptr [edi + 0x30] // 00b0796b
        movss xmm0,dword ptr [eax] // 00b0796e
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b07972
        pop edx
        lahf // 00b07979
        test ah,0x44 // 00b0797a
        movss dword ptr [esp + 0x10],xmm0 // 00b0797d
        jp l_00b0798f // 00b07983
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b07985
        pop edx
        jmp l_00b079b8 // 00b0798d
    l_00b0798f:
        xor ecx,ecx // 00b0798f
        mov edx,ebp
        call random_bridge // 00b07991
        mov dword ptr [esp + 0x1c],eax // 00b07996
        fild dword ptr [esp + 0x1c] // 00b0799a
        fmul dword ptr [esp + 0x10] // 00b0799e
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b079a2
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b079a8
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b079ae
        movss xmm0,dword ptr [esp + 0x1c] // 00b079b2
    l_00b079b8:
        movss dword ptr [esi + 0x48],xmm0 // 00b079b8
        mov ecx,dword ptr [edi + 0x34] // 00b079bd
        movss xmm0,dword ptr [ecx] // 00b079c0
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b079c4
        pop edx
        lahf // 00b079cb
        test ah,0x44 // 00b079cc
        movss dword ptr [esp + 0x10],xmm0 // 00b079cf
        jp l_00b079e1 // 00b079d5
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b079d7
        pop edx
        jmp l_00b07a0a // 00b079df
    l_00b079e1:
        xor ecx,ecx // 00b079e1
        mov edx,ebp
        call random_bridge // 00b079e3
        mov dword ptr [esp + 0x1c],eax // 00b079e8
        fild dword ptr [esp + 0x1c] // 00b079ec
        fmul dword ptr [esp + 0x10] // 00b079f0
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b079f4
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b079fa
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b07a00
        movss xmm0,dword ptr [esp + 0x1c] // 00b07a04
    l_00b07a0a:
        movss dword ptr [esi + 0x4c],xmm0 // 00b07a0a
        mov edx,dword ptr [edi + 0x84] // 00b07a0f
        movss xmm0,dword ptr [edx] // 00b07a15
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b07a19
        pop edx
        lahf // 00b07a20
        test ah,0x44 // 00b07a21
        movss dword ptr [esp + 0x10],xmm0 // 00b07a24
        jp l_00b07a36 // 00b07a2a
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b07a2c
        pop edx
        jmp l_00b07a5f // 00b07a34
    l_00b07a36:
        xor ecx,ecx // 00b07a36
        mov edx,ebp
        call random_bridge // 00b07a38
        mov dword ptr [esp + 0x1c],eax // 00b07a3d
        fild dword ptr [esp + 0x1c] // 00b07a41
        fmul dword ptr [esp + 0x10] // 00b07a45
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b07a49
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b07a4f
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b07a55
        movss xmm0,dword ptr [esp + 0x1c] // 00b07a59
    l_00b07a5f:
        movss dword ptr [esi + 0x54],xmm0 // 00b07a5f
        mov eax,dword ptr [edi + 0x88] // 00b07a64
        movss xmm0,dword ptr [eax] // 00b07a6a
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b07a6e
        pop edx
        lahf // 00b07a75
        test ah,0x44 // 00b07a76
        movss dword ptr [esp + 0x10],xmm0 // 00b07a79
        jp l_00b07a8b // 00b07a7f
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b07a81
        pop edx
        jmp l_00b07ab4 // 00b07a89
    l_00b07a8b:
        xor ecx,ecx // 00b07a8b
        mov edx,ebp
        call random_bridge // 00b07a8d
        mov dword ptr [esp + 0x1c],eax // 00b07a92
        fild dword ptr [esp + 0x1c] // 00b07a96
        fmul dword ptr [esp + 0x10] // 00b07a9a
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b07a9e
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b07aa4
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b07aaa
        movss xmm0,dword ptr [esp + 0x1c] // 00b07aae
    l_00b07ab4:
        movss dword ptr [esi + 0x58],xmm0 // 00b07ab4
        mov ecx,dword ptr [edi + 0x38] // 00b07ab9
        movss xmm0,dword ptr [ecx] // 00b07abc
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b07ac0
        pop edx
        lahf // 00b07ac7
        test ah,0x44 // 00b07ac8
        movss dword ptr [esp + 0x10],xmm0 // 00b07acb
        jp l_00b07add // 00b07ad1
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b07ad3
        pop edx
        jmp l_00b07b06 // 00b07adb
    l_00b07add:
        xor ecx,ecx // 00b07add
        mov edx,ebp
        call random_bridge // 00b07adf
        mov dword ptr [esp + 0x1c],eax // 00b07ae4
        fild dword ptr [esp + 0x1c] // 00b07ae8
        fmul dword ptr [esp + 0x10] // 00b07aec
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b07af0
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b07af6
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b07afc
        movss xmm0,dword ptr [esp + 0x1c] // 00b07b00
    l_00b07b06:
        movss dword ptr [esi + 0x5c],xmm0 // 00b07b06
        mov edx,dword ptr [edi + 0x5c] // 00b07b0b
        movss xmm0,dword ptr [edx] // 00b07b0e
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b07b12
        pop edx
        lahf // 00b07b19
        test ah,0x44 // 00b07b1a
        movss dword ptr [esp + 0x10],xmm0 // 00b07b1d
        jp l_00b07b2f // 00b07b23
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b07b25
        pop edx
        jmp l_00b07b58 // 00b07b2d
    l_00b07b2f:
        xor ecx,ecx // 00b07b2f
        mov edx,ebp
        call random_bridge // 00b07b31
        mov dword ptr [esp + 0x1c],eax // 00b07b36
        fild dword ptr [esp + 0x1c] // 00b07b3a
        fmul dword ptr [esp + 0x10] // 00b07b3e
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b07b42
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b07b48
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b07b4e
        movss xmm0,dword ptr [esp + 0x1c] // 00b07b52
    l_00b07b58:
        movss dword ptr [esi + 0x3c],xmm0 // 00b07b58
        cmp byte ptr [edi + 0x60],0x0 // 00b07b5d
        jz l_00b07b83 // 00b07b61
        mov ebx,dword ptr [edi + 0x54] // 00b07b63
        push ebp // 00b07b66
        mov ebp,dword ptr [edi + 0x50] // 00b07b67
        xor ecx,ecx // 00b07b6a
        mov edx,dword ptr [esp]
        call random_bridge // 00b07b6c
        sub ebx,ebp // 00b07b71
        add ebx,0x1 // 00b07b73
        xor edx,edx // 00b07b76
        div ebx // 00b07b78
        add edx,ebp // 00b07b7a
        cvtsi2ss xmm0,edx // 00b07b7c
        pop ebp // 00b07b80
        jmp l_00b07b88 // 00b07b81
    l_00b07b83:
        cvtsi2ss xmm0,dword ptr [edi + 0x50] // 00b07b83
    l_00b07b88:
        movss dword ptr [esi + 0x34],xmm0 // 00b07b88
        movss dword ptr [esi + 0x30],xmm0 // 00b07b8d
        cmp byte ptr [edi + 0x61],0x0 // 00b07b92
        jz l_00b07bc9 // 00b07b96
        mov ebx,dword ptr [edi + 0x54] // 00b07b98
        mov edi,dword ptr [edi + 0x50] // 00b07b9b
        xor ecx,ecx // 00b07b9e
        mov edx,ebp
        call random_bridge // 00b07ba0
        sub ebx,edi // 00b07ba5
        add ebx,0x1 // 00b07ba7
        xor edx,edx // 00b07baa
        div ebx // 00b07bac
        add edx,edi // 00b07bae
        pop edi // 00b07bb0
        cvtsi2ss xmm0,edx // 00b07bb1
        movss dword ptr [esi + 0x38],xmm0 // 00b07bb5
        push edx
        mov edx,dword ptr [ebp+24] // borrowed current global
        add dword ptr [edx],0x1 // 00b07bba
        pop edx
        pop esi // 00b07bc1
        pop ebx // 00b07bc2
        add esp,0x8 // 00b07bc3
        pop ebp
        ret 0x8 // 00b07bc6
    l_00b07bc9:
        fld dword ptr [esi + 0x30] // 00b07bc9
        pop edi // 00b07bcc
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b07bcd
        pop edx
        fstp dword ptr [esi + 0x38] // 00b07bd3
        push edx
        mov edx,dword ptr [ebp+24] // borrowed current global
        add dword ptr [edx],0x1 // 00b07bd6
        pop edx
        pop esi // 00b07bdd
        pop ebx // 00b07bde
        add esp,0x8 // 00b07bdf
        pop ebp
        ret 0x8 // 00b07be2
    }
}

} // namespace bsp
