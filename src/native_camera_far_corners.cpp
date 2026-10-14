#include "bsp/native_camera_far_corners.hpp"
#include "bsp/native_camera_projection.hpp"
#include "bsp/native_camera_general_inverse.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/native_camera_world.hpp"
#include <cstddef>
namespace bsp {
static_assert(offsetof(NativeCameraFarCornersContext, one_00d7a24c)==0);
static_assert(offsetof(NativeCameraFarCornersContext, negative_one_00d7a260)==4);
static_assert(offsetof(NativeCameraFarCornersContext, half_00d7a280)==8);
static_assert(offsetof(NativeCameraFarCornersContext, near_00ce3800)==12);
__declspec(naked) void* __fastcall compute_native_camera_far_corners_00b0eb10(
    const NativeCameraFarCornersContext*, void*, void*, void*) {
    __asm {
        push ecx // new borrowed context; native scratch offsets unchanged
        sub esp, 013ch
        push eax
        mov eax, dword ptr [esp + 0140h]
        mov eax, dword ptr [eax + 0]
        movss xmm0,dword ptr [eax]
        pop eax
        push eax
        mov eax, dword ptr [esp + 0140h]
        mov eax, dword ptr [eax + 4]
        movss xmm1,dword ptr [eax]
        pop eax
        xorps xmm2,xmm2 // 00b0eb26
        push ebx // 00b0eb29
        push ebp // 00b0eb2a
        push esi // 00b0eb2b
        mov esi, dword ptr [esp + 0150h] // original camera slot +14C
        fld dword ptr [esi + 01d4h] // 00b0eb33
        push edi // 00b0eb39
        fstp dword ptr [esp + 020h] // 00b0eb3a
        movss dword ptr [esp + 09ch],xmm1 // 00b0eb3e
        fld dword ptr [esp + 020h] // 00b0eb47
        movss dword ptr [esp + 0a0h],xmm1 // 00b0eb4b
        push eax
        mov eax, dword ptr [esp + 0150h]
        mov eax, dword ptr [eax + 8]
        fld qword ptr [eax]
        pop eax
        movss dword ptr [esp + 0a4h],xmm2 // 00b0eb5a
        fcomip st(0),st(1) // 00b0eb63
        fstp st(0) // 00b0eb65
        movss dword ptr [esp + 0a8h],xmm1 // 00b0eb67
        movss dword ptr [esp + 0ach],xmm0 // 00b0eb70
        movss dword ptr [esp + 0b0h],xmm2 // 00b0eb79
        movss dword ptr [esp + 0b4h],xmm0 // 00b0eb82
        movss dword ptr [esp + 0b8h],xmm1 // 00b0eb8b
        movss dword ptr [esp + 0bch],xmm2 // 00b0eb94
        movss dword ptr [esp + 0c0h],xmm0 // 00b0eb9d
        movss dword ptr [esp + 0c4h],xmm0 // 00b0eba6
        movss dword ptr [esp + 0c8h],xmm2 // 00b0ebaf
        movss dword ptr [esp + 0cch],xmm1 // 00b0ebb8
        movss dword ptr [esp + 0d0h],xmm1 // 00b0ebc1
        movss dword ptr [esp + 0d4h],xmm0 // 00b0ebca
        movss dword ptr [esp + 0d8h],xmm1 // 00b0ebd3
        movss dword ptr [esp + 0dch],xmm0 // 00b0ebdc
        movss dword ptr [esp + 0e0h],xmm0 // 00b0ebe5
        movss dword ptr [esp + 0e4h],xmm0 // 00b0ebee
        movss dword ptr [esp + 0e8h],xmm1 // 00b0ebf7
        movss dword ptr [esp + 0ech],xmm0 // 00b0ec00
        movss dword ptr [esp + 0f0h],xmm0 // 00b0ec09
        movss dword ptr [esp + 0f4h],xmm0 // 00b0ec12
        movss dword ptr [esp + 0f8h],xmm0 // 00b0ec1b
        jbe loc_00b0ec30 // 00b0ec24
        push eax
        mov eax, dword ptr [esp + 0150h]
        mov eax, dword ptr [eax + 12]
        movss xmm0,dword ptr [eax]
        pop eax
        jmp loc_00b0ec36 // 00b0ec2e
    loc_00b0ec30:
        movss xmm0,dword ptr [esp + 020h] // 00b0ec30
    loc_00b0ec36:
        fld dword ptr [esi + 01d8h] // 00b0ec36
        sub esp,010h // 00b0ec3c
        fstp dword ptr [esp + 034h] // 00b0ec3f
        movss dword ptr [esp + 030h],xmm0 // 00b0ec43
        fld dword ptr [esi + 01c8h] // 00b0ec49
        lea ecx,[esp + 04ch] // 00b0ec4f
        fstp dword ptr [esp + 038h] // 00b0ec53
        fld dword ptr [esi + 01c4h] // 00b0ec57
        fstp dword ptr [esp + 020h] // 00b0ec5d
        fld dword ptr [esp + 034h] // 00b0ec61
        fstp dword ptr [esp + 0ch] // 00b0ec65
        fld dword ptr [esp + 030h] // 00b0ec69
        fstp dword ptr [esp + 08h] // 00b0ec6d
        fld dword ptr [esp + 038h] // 00b0ec71
        fstp dword ptr [esp + 04h] // 00b0ec75
        fld dword ptr [esp + 020h] // 00b0ec79
        fstp dword ptr [esp] // 00b0ec7d
        lea edx, [esp]
        call build_native_camera_projection_00b642f0
        add esp, 010h
        mov edx,eax // 00b0ec85
        lea ecx,[esp + 010ch] // 00b0ec87
        call invert_native_camera_matrix_00b632d0
        xor edi,edi // 00b0ec93
        lea ebx,[edi + 04h] // 00b0ec95
        jmp loc_00b0eca0 // 00b0ec98
    loc_00b0eca0:
        movss xmm0,dword ptr [esp + edi*01h + 09ch] // 00b0eca0
        movss xmm1,dword ptr [esp + edi*01h + 0a0h] // 00b0eca9
        movss xmm2,dword ptr [esp + edi*01h + 0a4h] // 00b0ecb2
        lea eax,[esp + 010ch] // 00b0ecbb
        push eax // 00b0ecc2
        lea ecx,[esp + 0100h] // 00b0ecc3
        movss dword ptr [esp + 030h],xmm0 // 00b0ecca
        push eax
        mov eax, dword ptr [esp + 0154h]
        mov eax, dword ptr [eax + 0]
        movss xmm0,dword ptr [eax]
        pop eax
        push ecx // 00b0ecd8
        lea ecx,[esp + 034h] // 00b0ecd9
        movss dword ptr [esp + 038h],xmm1 // 00b0ecdd
        movss dword ptr [esp + 03ch],xmm2 // 00b0ece3
        movss dword ptr [esp + 040h],xmm0 // 00b0ece9
        pop edx // original output argument; matrix remains on stack
        call transform_native_vector4_00b62d10
        fld dword ptr [eax] // 00b0ecf4
        fstp dword ptr [esp + 02ch] // 00b0ecf6
        add edi,0ch // 00b0ecfa
        sub ebx,01h // 00b0ecfd
        fld dword ptr [eax + 04h] // 00b0ed00
        fstp dword ptr [esp + 030h] // 00b0ed03
        fld dword ptr [eax + 08h] // 00b0ed07
        fstp dword ptr [esp + 034h] // 00b0ed0a
        fld dword ptr [eax + 0ch] // 00b0ed0e
        fstp dword ptr [esp + 038h] // 00b0ed11
        fld dword ptr [esp + 038h] // 00b0ed15
        fld1 // 00b0ed19
        fdivrp st(1), st(0) // 00b0ed1b
        fld dword ptr [esp + 02ch] // 00b0ed1d
        fmul st(0), st(1) // 00b0ed21
        fstp dword ptr [esp + 014h] // 00b0ed23
        fld dword ptr [esp + 030h] // 00b0ed27
        fmul st(0), st(1) // 00b0ed2b
        fstp dword ptr [esp + 018h] // 00b0ed2d
        fmul dword ptr [esp + 034h] // 00b0ed31
        fstp dword ptr [esp + 01ch] // 00b0ed35
        fld dword ptr [esp + 014h] // 00b0ed39
        fstp dword ptr [esp + edi*01h + 030h] // 00b0ed3d
        fld dword ptr [esp + 018h] // 00b0ed41
        fstp dword ptr [esp + edi*01h + 034h] // 00b0ed45
        fld dword ptr [esp + 01ch] // 00b0ed49
        fstp dword ptr [esp + edi*01h + 038h] // 00b0ed4d
        jnz loc_00b0eca0 // 00b0ed51
        fld dword ptr [esi + 01d8h] // 00b0ed57
        lea ebp,[ebx + 08h] // 00b0ed5d
        fdiv dword ptr [esp + 020h] // 00b0ed60
        lea edi,[esp + 044h] // 00b0ed64
        mov bl,02h // 00b0ed68
        fstp dword ptr [esp + 010h] // 00b0ed6a
        fld dword ptr [esp + 010h] // 00b0ed6e
        fst dword ptr [esp + 010h] // 00b0ed72
        fld dword ptr [esp + 03ch] // 00b0ed76
        fld dword ptr [esp + 010h] // 00b0ed7a
        fld st(0) // 00b0ed7e
        fmulp st(2), st(0) // 00b0ed80
        fxch // 00b0ed82
        fstp dword ptr [esp + 014h] // 00b0ed84
        fld dword ptr [esp + 040h] // 00b0ed88
        fmul st(0), st(1) // 00b0ed8c
        fstp dword ptr [esp + 018h] // 00b0ed8e
        fmul dword ptr [esp + 044h] // 00b0ed92
        fstp dword ptr [esp + 01ch] // 00b0ed96
        fld dword ptr [esp + 014h] // 00b0ed9a
        fstp dword ptr [esp + 06ch] // 00b0ed9e
        fld dword ptr [esp + 018h] // 00b0eda2
        fstp dword ptr [esp + 070h] // 00b0eda6
        fld dword ptr [esp + 01ch] // 00b0edaa
        fstp dword ptr [esp + 074h] // 00b0edae
        fst dword ptr [esp + 010h] // 00b0edb2
        fld dword ptr [esp + 048h] // 00b0edb6
        fld dword ptr [esp + 010h] // 00b0edba
        fld st(0) // 00b0edbe
        fmulp st(2), st(0) // 00b0edc0
        fxch // 00b0edc2
        fstp dword ptr [esp + 014h] // 00b0edc4
        fld dword ptr [esp + 04ch] // 00b0edc8
        fmul st(0), st(1) // 00b0edcc
        fstp dword ptr [esp + 018h] // 00b0edce
        fmul dword ptr [esp + 050h] // 00b0edd2
        fstp dword ptr [esp + 01ch] // 00b0edd6
        fld dword ptr [esp + 014h] // 00b0edda
        fstp dword ptr [esp + 078h] // 00b0edde
        fld dword ptr [esp + 018h] // 00b0ede2
        fstp dword ptr [esp + 07ch] // 00b0ede6
        fld dword ptr [esp + 01ch] // 00b0edea
        fstp dword ptr [esp + 080h] // 00b0edee
        fst dword ptr [esp + 010h] // 00b0edf5
        fld dword ptr [esp + 054h] // 00b0edf9
        fld dword ptr [esp + 010h] // 00b0edfd
        fld st(0) // 00b0ee01
        fmulp st(2), st(0) // 00b0ee03
        fxch // 00b0ee05
        fstp dword ptr [esp + 014h] // 00b0ee07
        fld dword ptr [esp + 058h] // 00b0ee0b
        fmul st(0), st(1) // 00b0ee0f
        fstp dword ptr [esp + 018h] // 00b0ee11
        fmul dword ptr [esp + 05ch] // 00b0ee15
        fstp dword ptr [esp + 01ch] // 00b0ee19
        fld dword ptr [esp + 014h] // 00b0ee1d
        fstp dword ptr [esp + 084h] // 00b0ee21
        fld dword ptr [esp + 018h] // 00b0ee28
        fstp dword ptr [esp + 088h] // 00b0ee2c
        fld dword ptr [esp + 01ch] // 00b0ee33
        fstp dword ptr [esp + 08ch] // 00b0ee37
        fstp dword ptr [esp + 010h] // 00b0ee3e
        fld dword ptr [esp + 060h] // 00b0ee42
        fld dword ptr [esp + 010h] // 00b0ee46
        fld st(0) // 00b0ee4a
        fmulp st(2), st(0) // 00b0ee4c
        fxch // 00b0ee4e
        fstp dword ptr [esp + 014h] // 00b0ee50
        fld dword ptr [esp + 064h] // 00b0ee54
        fmul st(0), st(1) // 00b0ee58
        fstp dword ptr [esp + 018h] // 00b0ee5a
        fmul dword ptr [esp + 068h] // 00b0ee5e
        fstp dword ptr [esp + 01ch] // 00b0ee62
        fld dword ptr [esp + 014h] // 00b0ee66
        fstp dword ptr [esp + 090h] // 00b0ee6a
        fld dword ptr [esp + 018h] // 00b0ee71
        fstp dword ptr [esp + 094h] // 00b0ee75
        fld dword ptr [esp + 01ch] // 00b0ee7c
        fstp dword ptr [esp + 098h] // 00b0ee80
    loc_00b0ee87:
        test byte ptr [esi + 05ch],bl // 00b0ee87
        jnz loc_00b0ee93 // 00b0ee8a
        mov ecx,esi // 00b0ee8c
        call refresh_native_camera_world_00b6db70
    loc_00b0ee93:
        fld dword ptr [edi + -04h] // 00b0ee93
        add edi,0ch // 00b0ee96
        sub ebp,01h // 00b0ee99
        fstp dword ptr [esp + 028h] // 00b0ee9c
        fld dword ptr [edi + -014h] // 00b0eea0
        fstp dword ptr [esp + 010h] // 00b0eea3
        fld dword ptr [edi + -0ch] // 00b0eea7
        fstp dword ptr [esp + 024h] // 00b0eeaa
        fld dword ptr [esp + 010h] // 00b0eeae
        fld st(0) // 00b0eeb2
        fmul dword ptr [esi + 0f0h] // 00b0eeb4
        fld dword ptr [esi + 0100h] // 00b0eeba
        fld dword ptr [esp + 028h] // 00b0eec0
        fld st(0) // 00b0eec4
        fmulp st(2), st(0) // 00b0eec6
        fxch st(2) // 00b0eec8
        faddp st(1), st(0) // 00b0eeca
        fld dword ptr [esp + 024h] // 00b0eecc
        fld st(0) // 00b0eed0
        fmul dword ptr [esi + 0110h] // 00b0eed2
        faddp st(2),st(0) // 00b0eed8
        fxch // 00b0eeda
        fstp dword ptr [esp + 014h] // 00b0eedc
        fld st(2) // 00b0eee0
        movss xmm0,dword ptr [esp + 014h] // 00b0eee2
        fmul dword ptr [esi + 0f4h] // 00b0eee8
        movss dword ptr [edi + -014h],xmm0 // 00b0eeee
        fld dword ptr [esi + 0104h] // 00b0eef3
        fmul st(0), st(3) // 00b0eef9
        faddp st(1), st(0) // 00b0eefb
        fld st(1) // 00b0eefd
        fmul dword ptr [esi + 0114h] // 00b0eeff
        faddp st(1), st(0) // 00b0ef05
        fstp dword ptr [esp + 018h] // 00b0ef07
        fld dword ptr [esi + 0f8h] // 00b0ef0b
        fmulp st(3), st(0) // 00b0ef11
        fld dword ptr [esi + 0108h] // 00b0ef13
        fmulp st(2), st(0) // 00b0ef19
        fxch st(2) // 00b0ef1b
        faddp st(1), st(0) // 00b0ef1d
        fld dword ptr [esi + 0118h] // 00b0ef1f
        fmulp st(2), st(0) // 00b0ef25
        faddp st(1), st(0) // 00b0ef27
        fstp dword ptr [esp + 01ch] // 00b0ef29
        fld dword ptr [esp + 018h] // 00b0ef2d
        fstp dword ptr [edi + -010h] // 00b0ef31
        fld dword ptr [esp + 01ch] // 00b0ef34
        fstp dword ptr [edi + -0ch] // 00b0ef38
        jnz loc_00b0ee87 // 00b0ef3b
        mov eax, dword ptr [esp + 0158h] // original output slot +154
        movss xmm0,dword ptr [esp + 06ch] // 00b0ef48
        movss xmm1,dword ptr [esp + 078h] // 00b0ef4e
        movss dword ptr [eax],xmm0 // 00b0ef54
        movss xmm0,dword ptr [esp + 070h] // 00b0ef58
        movss dword ptr [eax + 04h],xmm0 // 00b0ef5e
        movss xmm0,dword ptr [esp + 074h] // 00b0ef63
        movss dword ptr [eax + 08h],xmm0 // 00b0ef69
        push eax
        mov eax, dword ptr [esp + 0150h]
        mov eax, dword ptr [eax + 0]
        movss xmm0,dword ptr [eax]
        pop eax
        movss dword ptr [eax + 0ch],xmm0 // 00b0ef76
        movss dword ptr [eax + 010h],xmm1 // 00b0ef7b
        movss xmm1,dword ptr [esp + 07ch] // 00b0ef80
        movss dword ptr [eax + 014h],xmm1 // 00b0ef86
        movss xmm1,dword ptr [esp + 080h] // 00b0ef8b
        movss dword ptr [eax + 018h],xmm1 // 00b0ef94
        movss xmm1,dword ptr [esp + 084h] // 00b0ef99
        movss dword ptr [eax + 01ch],xmm0 // 00b0efa2
        movss dword ptr [eax + 020h],xmm1 // 00b0efa7
        movss xmm1,dword ptr [esp + 088h] // 00b0efac
        movss dword ptr [eax + 024h],xmm1 // 00b0efb5
        movss xmm1,dword ptr [esp + 08ch] // 00b0efba
        movss dword ptr [eax + 028h],xmm1 // 00b0efc3
        movss xmm1,dword ptr [esp + 090h] // 00b0efc8
        movss dword ptr [eax + 02ch],xmm0 // 00b0efd1
        pop edi // 00b0efd6
        movss dword ptr [eax + 030h],xmm1 // 00b0efd7
        movss xmm1,dword ptr [esp + 090h] // 00b0efdc
        pop esi // 00b0efe5
        movss dword ptr [eax + 034h],xmm1 // 00b0efe6
        movss xmm1,dword ptr [esp + 090h] // 00b0efeb
        pop ebp // 00b0eff4
        movss dword ptr [eax + 038h],xmm1 // 00b0eff5
        movss dword ptr [eax + 03ch],xmm0 // 00b0effa
        pop ebx // 00b0efff
        add esp,013ch // 00b0f000
        add esp, 4 // discard borrowed context
        ret 8
    }
}
} // namespace bsp
