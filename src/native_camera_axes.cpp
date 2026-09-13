#include "bsp/native_camera_axes.hpp"
#include "bsp/native_camera_world.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera axes require MSVC Win32 x87 and SSE assembly.
#endif

namespace bsp {
// Full original 00b70ea0; EBP retains only the added context outside the
// original 30h local frame. No floating-point operation or spill is added.
__declspec(naked) float* __fastcall get_native_camera_axis_y_00b70ea0(
    void*, const NativeCameraAxesContext*) {
    __asm {
        push ebp
        mov ebp, edx
        sub esp,0x30 // 00b70ea0
        push esi // 00b70ea3
        mov esi,ecx // 00b70ea4
        test dword ptr [esi + 0x2f0],0x100 // 00b70ea6
        jnz L_00b70fcc // 00b70eb0
        test byte ptr [esi + 0x5c],0x2 // 00b70eb6
        jnz L_00b70ec1 // 00b70eba
        call refresh_native_camera_world_00b6db70 // 00b70ebc
    L_00b70ec1:
        movss xmm0,dword ptr [esi + 0x110] // 00b70ec1
        mov edx, dword ptr [ebp + 8]
        movss xmm1,dword ptr [edx] // 00b70ec9
        movss dword ptr [esp + 0x4],xmm0 // 00b70ed1
        movss xmm0,dword ptr [esi + 0x114] // 00b70ed7
        push edi // 00b70edf
        lea eax,[esp + 0x8] // 00b70ee0
        movss dword ptr [esp + 0xc],xmm0 // 00b70ee4
        movss xmm0,dword ptr [esi + 0x118] // 00b70eea
        push eax // 00b70ef2
        lea ecx,[esp + 0x18] // 00b70ef3
        movss dword ptr [esp + 0x14],xmm0 // 00b70ef7
        xorps xmm0,xmm0 // 00b70efd
        push ecx // 00b70f00
        mov edx,eax // 00b70f01
        lea ecx,[esp + 0x28] // 00b70f03
        movss dword ptr [esp + 0x1c],xmm0 // 00b70f07
        movss dword ptr [esp + 0x20],xmm1 // 00b70f0d
        movss dword ptr [esp + 0x24],xmm0 // 00b70f13
        call camera_vector_cross_004f9b30 // 00b70f19
        mov edx,eax // 00b70f1e
        lea ecx,[esp + 0x18] // 00b70f20
        call camera_vector_cross_004f9b30 // 00b70f24
        mov edx,eax // 00b70f29
        lea ecx,[esp + 0x2c] // 00b70f2b
        push ebp // added stack CRT access; shared provider RET4
        call camera_vector_normalize_00419510 // 00b70f2f
        fld dword ptr [eax] // 00b70f34
        lea edi,[esi + 0x440] // 00b70f36
        fstp dword ptr [edi] // 00b70f3c
        push edi // 00b70f3e
        fld dword ptr [eax + 0x4] // 00b70f3f
        lea edx,[esp + 0xc] // 00b70f42
        fstp dword ptr [edi + 0x4] // 00b70f46
        lea ecx,[esp + 0x30] // 00b70f49
        fld dword ptr [eax + 0x8] // 00b70f4d
        fstp dword ptr [edi + 0x8] // 00b70f50
        call camera_vector_cross_004f9b30 // 00b70f53
        mov edx,eax // 00b70f58
        lea ecx,[esp + 0x20] // 00b70f5a
        push ebp // added stack CRT access; shared provider RET4
        call camera_vector_normalize_00419510 // 00b70f5e
        fld dword ptr [eax] // 00b70f63
        fstp dword ptr [esi + 0x44c] // 00b70f65
        mov ecx,edi // 00b70f6b
        fld dword ptr [eax + 0x4] // 00b70f6d
        fstp dword ptr [esi + 0x450] // 00b70f70
        fld dword ptr [eax + 0x8] // 00b70f76
        fstp dword ptr [esi + 0x454] // 00b70f79
        mov edx, ebp
        call camera_vector_length_00419440 // 00b70f7f
        mov edx, dword ptr [ebp + 0ch]
        fld dword ptr [edx] // 00b70f84
        fcomip st(0),st(1) // 00b70f8a
        fstp st(0) // 00b70f8c
        jbe L_00b70fc1 // 00b70f8e
        xorps xmm0,xmm0 // 00b70f90
        mov edx, dword ptr [ebp + 8]
        movss xmm1,dword ptr [edx] // 00b70f93
        movss dword ptr [edi],xmm0 // 00b70f9b
        movss dword ptr [edi + 0x4],xmm1 // 00b70f9f
        movss dword ptr [edi + 0x8],xmm0 // 00b70fa4
        movss dword ptr [esi + 0x44c],xmm1 // 00b70fa9
        movss dword ptr [esi + 0x450],xmm0 // 00b70fb1
        movss dword ptr [esi + 0x454],xmm0 // 00b70fb9
    L_00b70fc1:
        or dword ptr [esi + 0x2f0],0x100 // 00b70fc1
        pop edi // 00b70fcb
    L_00b70fcc:
        lea eax,[esi + 0x440] // 00b70fcc
        pop esi // 00b70fd2
        add esp,0x30 // 00b70fd3
        pop ebp
        ret // 00b70fd6
    }
}

// Full original 00b70fe0; EBP retains only the added context outside the
// original 30h local frame. No floating-point operation or spill is added.
__declspec(naked) float* __fastcall get_native_camera_axis_x_00b70fe0(
    void*, const NativeCameraAxesContext*) {
    __asm {
        push ebp
        mov ebp, edx
        sub esp,0x30 // 00b70fe0
        push esi // 00b70fe3
        mov esi,ecx // 00b70fe4
        test dword ptr [esi + 0x2f0],0x100 // 00b70fe6
        jnz L_00b7110c // 00b70ff0
        test byte ptr [esi + 0x5c],0x2 // 00b70ff6
        jnz L_00b71001 // 00b70ffa
        call refresh_native_camera_world_00b6db70 // 00b70ffc
    L_00b71001:
        movss xmm0,dword ptr [esi + 0x110] // 00b71001
        mov edx, dword ptr [ebp + 8]
        movss xmm1,dword ptr [edx] // 00b71009
        movss dword ptr [esp + 0x4],xmm0 // 00b71011
        movss xmm0,dword ptr [esi + 0x114] // 00b71017
        push edi // 00b7101f
        lea eax,[esp + 0x8] // 00b71020
        movss dword ptr [esp + 0xc],xmm0 // 00b71024
        movss xmm0,dword ptr [esi + 0x118] // 00b7102a
        push eax // 00b71032
        lea ecx,[esp + 0x18] // 00b71033
        movss dword ptr [esp + 0x14],xmm0 // 00b71037
        xorps xmm0,xmm0 // 00b7103d
        push ecx // 00b71040
        mov edx,eax // 00b71041
        lea ecx,[esp + 0x28] // 00b71043
        movss dword ptr [esp + 0x1c],xmm0 // 00b71047
        movss dword ptr [esp + 0x20],xmm1 // 00b7104d
        movss dword ptr [esp + 0x24],xmm0 // 00b71053
        call camera_vector_cross_004f9b30 // 00b71059
        mov edx,eax // 00b7105e
        lea ecx,[esp + 0x18] // 00b71060
        call camera_vector_cross_004f9b30 // 00b71064
        mov edx,eax // 00b71069
        lea ecx,[esp + 0x2c] // 00b7106b
        push ebp // added stack CRT access; shared provider RET4
        call camera_vector_normalize_00419510 // 00b7106f
        fld dword ptr [eax] // 00b71074
        lea edi,[esi + 0x440] // 00b71076
        fstp dword ptr [edi] // 00b7107c
        push edi // 00b7107e
        fld dword ptr [eax + 0x4] // 00b7107f
        lea edx,[esp + 0xc] // 00b71082
        fstp dword ptr [edi + 0x4] // 00b71086
        lea ecx,[esp + 0x30] // 00b71089
        fld dword ptr [eax + 0x8] // 00b7108d
        fstp dword ptr [edi + 0x8] // 00b71090
        call camera_vector_cross_004f9b30 // 00b71093
        mov edx,eax // 00b71098
        lea ecx,[esp + 0x20] // 00b7109a
        push ebp // added stack CRT access; shared provider RET4
        call camera_vector_normalize_00419510 // 00b7109e
        fld dword ptr [eax] // 00b710a3
        fstp dword ptr [esi + 0x44c] // 00b710a5
        mov ecx,edi // 00b710ab
        fld dword ptr [eax + 0x4] // 00b710ad
        fstp dword ptr [esi + 0x450] // 00b710b0
        fld dword ptr [eax + 0x8] // 00b710b6
        fstp dword ptr [esi + 0x454] // 00b710b9
        mov edx, ebp
        call camera_vector_length_00419440 // 00b710bf
        mov edx, dword ptr [ebp + 0ch]
        fld dword ptr [edx] // 00b710c4
        fcomip st(0),st(1) // 00b710ca
        fstp st(0) // 00b710cc
        jbe L_00b71101 // 00b710ce
        xorps xmm0,xmm0 // 00b710d0
        mov edx, dword ptr [ebp + 8]
        movss xmm1,dword ptr [edx] // 00b710d3
        movss dword ptr [edi],xmm0 // 00b710db
        movss dword ptr [edi + 0x4],xmm1 // 00b710df
        movss dword ptr [edi + 0x8],xmm0 // 00b710e4
        movss dword ptr [esi + 0x44c],xmm1 // 00b710e9
        movss dword ptr [esi + 0x450],xmm0 // 00b710f1
        movss dword ptr [esi + 0x454],xmm0 // 00b710f9
    L_00b71101:
        or dword ptr [esi + 0x2f0],0x100 // 00b71101
        pop edi // 00b7110b
    L_00b7110c:
        lea eax,[esi + 0x44c] // 00b7110c
        pop esi // 00b71112
        add esp,0x30 // 00b71113
        pop ebp
        ret // 00b71116
    }
}

__declspec(naked) const void* __fastcall get_native_camera_context_00b6feb0(const void*) {
    __asm {
        mov eax, dword ptr [ecx + 43ch] // 00b6feb0
        ret // 00b6feb6
    }
}
} // namespace bsp
