#include "bsp/native_dyn_convex_simplex.hpp"
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native convex simplex reduction requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
// Consumed existing game math leaf; not a new reconstruction claim.
__declspec(naked) void cross_reference() noexcept {
    __asm {
        fld qword ptr [ecx + 010h] // 00401c20
        fmul qword ptr [edx + 8] // 00401c23
        fld qword ptr [edx + 010h] // 00401c26
        fmul qword ptr [ecx + 8] // 00401c29
        fsubp st(1), st(0) // 00401c2c
        fstp qword ptr [eax] // 00401c2e
        fld qword ptr [edx + 010h] // 00401c30
        fmul qword ptr [ecx] // 00401c33
        fld qword ptr [ecx + 010h] // 00401c35
        fmul qword ptr [edx] // 00401c38
        fsubp st(1), st(0) // 00401c3a
        fstp qword ptr [eax + 8] // 00401c3c
        fld qword ptr [ecx + 8] // 00401c3f
        fmul qword ptr [edx] // 00401c42
        fld qword ptr [ecx] // 00401c44
        fmul qword ptr [edx + 8] // 00401c46
        fsubp st(1), st(0) // 00401c49
        fstp qword ptr [eax + 010h] // 00401c4b
        ret  // 00401c4e
    }
}
// Consumed existing game math leaf; not a new reconstruction claim.
__declspec(naked) void scale_reference() noexcept {
    __asm {
        fld qword ptr [eax] // 00401cd0
        fld qword ptr [esp + 4] // 00401cd2
        fmul st(1), st(0) // 00401cd6
        fxch st(1) // 00401cd8
        fstp qword ptr [eax] // 00401cda
        fld qword ptr [eax + 8] // 00401cdc
        fmul st(0), st(1) // 00401cdf
        fstp qword ptr [eax + 8] // 00401ce1
        fmul qword ptr [eax + 010h] // 00401ce4
        fstp qword ptr [eax + 010h] // 00401ce7
        ret 8 // 00401cea
    }
}
} // namespace
// Original switch table C3F178: counts1,2,3,4 map to the labels below.
// Direct branches encode those four verified targets without native pointers.
// All subsequent x87 instructions and storage operations retain native order.
__declspec(naked) void __fastcall reduce_native_dyn_convex_simplex_00c3cc30(void*) noexcept {
    __asm {
        push esi
        mov esi,ecx
        mov eax, dword ptr [esi + 01b8h] // 00c3cc30
        add eax, -1 // 00c3cc36
        sub esp, 064h // 00c3cc39
        cmp eax, 3 // 00c3cc3c
        ja l_00c3f171 // 00c3cc3f
        cmp eax,0
        je l_00c3cc4c
        cmp eax,1
        je l_00c3cc7c
        cmp eax,2
        je l_00c3cdf6
        jmp l_00c3d5d6
    l_00c3cc4c:
        fld qword ptr [esi + 098h] // 00c3cc4c
        fchs  // 00c3cc52
        fld qword ptr [esi + 0a0h] // 00c3cc54
        fchs  // 00c3cc5a
        fld qword ptr [esi + 0a8h] // 00c3cc5c
        fchs  // 00c3cc62
        fxch st(2) // 00c3cc64
        fstp qword ptr [esi + 080h] // 00c3cc66
        fstp qword ptr [esi + 088h] // 00c3cc6c
        fstp qword ptr [esi + 090h] // 00c3cc72
        add esp, 064h // 00c3cc78
        pop esi
        ret  // 00c3cc7b
    l_00c3cc7c:
        fld qword ptr [esi + 098h] // 00c3cc7c
        fsub qword ptr [esi + 0b0h] // 00c3cc82
        fld qword ptr [esi + 0a0h] // 00c3cc88
        fsub qword ptr [esi + 0b8h] // 00c3cc8e
        fld qword ptr [esi + 0a8h] // 00c3cc94
        fsub qword ptr [esi + 0c0h] // 00c3cc9a
        fld qword ptr [esi + 0b8h] // 00c3cca0
        fmulp st(2), st(0) // 00c3cca6
        fld qword ptr [esi + 0b0h] // 00c3cca8
        fmulp st(3), st(0) // 00c3ccae
        fxch st(1) // 00c3ccb0
        faddp st(2), st(0) // 00c3ccb2
        fmul qword ptr [esi + 0c0h] // 00c3ccb4
        faddp st(1), st(0) // 00c3ccba
        fldz  // 00c3ccbc
        fxch st(1) // 00c3ccbe
        fcomip st(0), st(1) // 00c3ccc0
        fstp st(0) // 00c3ccc2
        fld qword ptr [esi + 0b0h] // 00c3ccc4
        jb l_00c3cd70 // 00c3ccca
        fstp qword ptr [esi + 098h] // 00c3ccd0
        fld qword ptr [esi + 0b8h] // 00c3ccd6
        fstp qword ptr [esi + 0a0h] // 00c3ccdc
        fld qword ptr [esi + 0c0h] // 00c3cce2
        fstp qword ptr [esi + 0a8h] // 00c3cce8
        fld qword ptr [esi + 0110h] // 00c3ccee
        fstp qword ptr [esi + 0f8h] // 00c3ccf4
        fld qword ptr [esi + 0118h] // 00c3ccfa
        fstp qword ptr [esi + 0100h] // 00c3cd00
        fld qword ptr [esi + 0120h] // 00c3cd06
        fstp qword ptr [esi + 0108h] // 00c3cd0c
        fld qword ptr [esi + 0170h] // 00c3cd12
        fstp qword ptr [esi + 0158h] // 00c3cd18
        fld qword ptr [esi + 0178h] // 00c3cd1e
        fstp qword ptr [esi + 0160h] // 00c3cd24
        fld qword ptr [esi + 0180h] // 00c3cd2a
        fstp qword ptr [esi + 0168h] // 00c3cd30
        fld qword ptr [esi + 098h] // 00c3cd36
        fchs  // 00c3cd3c
        fld qword ptr [esi + 0a0h] // 00c3cd3e
        fchs  // 00c3cd44
        fld qword ptr [esi + 0a8h] // 00c3cd46
        fchs  // 00c3cd4c
        fxch st(2) // 00c3cd4e
        fstp qword ptr [esi + 080h] // 00c3cd50
        fstp qword ptr [esi + 088h] // 00c3cd56
        fstp qword ptr [esi + 090h] // 00c3cd5c
        mov dword ptr [esi + 01b8h], 1 // 00c3cd62
        add esp, 064h // 00c3cd6c
        pop esi
        ret  // 00c3cd6f
    l_00c3cd70:
        fsub qword ptr [esi + 098h] // 00c3cd70
        fld qword ptr [esi + 0b8h] // 00c3cd76
        fsub qword ptr [esi + 0a0h] // 00c3cd7c
        fld qword ptr [esi + 0c0h] // 00c3cd82
        fsub qword ptr [esi + 0a8h] // 00c3cd88
        fld qword ptr [esi + 0a0h] // 00c3cd8e
        fmul st(0), st(2) // 00c3cd94
        fld st(3) // 00c3cd96
        fmul qword ptr [esi + 098h] // 00c3cd98
        faddp st(1), st(0) // 00c3cd9e
        fld qword ptr [esi + 0a8h] // 00c3cda0
        fmul st(0), st(2) // 00c3cda6
        faddp st(1), st(0) // 00c3cda8
        fld st(2) // 00c3cdaa
        fmul st(0), st(3) // 00c3cdac
        fld st(4) // 00c3cdae
        fmul st(0), st(5) // 00c3cdb0
        faddp st(1), st(0) // 00c3cdb2
        fld st(2) // 00c3cdb4
        fmul st(0), st(3) // 00c3cdb6
        faddp st(1), st(0) // 00c3cdb8
        fdivp st(1), st(0) // 00c3cdba
        fld st(0) // 00c3cdbc
        fmulp st(4), st(0) // 00c3cdbe
        fmul st(2), st(0) // 00c3cdc0
        fmulp st(1), st(0) // 00c3cdc2
        fxch st(2) // 00c3cdc4
        fsub qword ptr [esi + 098h] // 00c3cdc6
        fxch st(1) // 00c3cdcc
        fsub qword ptr [esi + 0a0h] // 00c3cdce
        fxch st(2) // 00c3cdd4
        fsub qword ptr [esi + 0a8h] // 00c3cdd6
        fxch st(1) // 00c3cddc
        fstp qword ptr [esi + 080h] // 00c3cdde
        fxch st(1) // 00c3cde4
        fstp qword ptr [esi + 088h] // 00c3cde6
        fstp qword ptr [esi + 090h] // 00c3cdec
        add esp, 064h // 00c3cdf2
        pop esi
        ret  // 00c3cdf5
    l_00c3cdf6:
        fld qword ptr [esi + 098h] // 00c3cdf6
        fsub qword ptr [esi + 0c8h] // 00c3cdfc
        fld qword ptr [esi + 0a0h] // 00c3ce02
        fsub qword ptr [esi + 0d0h] // 00c3ce08
        fld qword ptr [esi + 0a8h] // 00c3ce0e
        fsub qword ptr [esi + 0d8h] // 00c3ce14
        fld qword ptr [esi + 0d0h] // 00c3ce1a
        fmulp st(2), st(0) // 00c3ce20
        fld qword ptr [esi + 0c8h] // 00c3ce22
        fmulp st(3), st(0) // 00c3ce28
        fxch st(1) // 00c3ce2a
        faddp st(2), st(0) // 00c3ce2c
        fmul qword ptr [esi + 0d8h] // 00c3ce2e
        faddp st(1), st(0) // 00c3ce34
        fldz  // 00c3ce36
        fxch st(1) // 00c3ce38
        fcomip st(0), st(1) // 00c3ce3a
        jb l_00c3cf32 // 00c3ce3c
        fld qword ptr [esi + 0b0h] // 00c3ce42
        fsub qword ptr [esi + 0c8h] // 00c3ce48
        fld qword ptr [esi + 0b8h] // 00c3ce4e
        fsub qword ptr [esi + 0d0h] // 00c3ce54
        fld qword ptr [esi + 0c0h] // 00c3ce5a
        fsub qword ptr [esi + 0d8h] // 00c3ce60
        fld qword ptr [esi + 0d0h] // 00c3ce66
        fmulp st(2), st(0) // 00c3ce6c
        fld qword ptr [esi + 0c8h] // 00c3ce6e
        fmulp st(3), st(0) // 00c3ce74
        fxch st(1) // 00c3ce76
        faddp st(2), st(0) // 00c3ce78
        fmul qword ptr [esi + 0d8h] // 00c3ce7a
        faddp st(1), st(0) // 00c3ce80
        fcomip st(0), st(1) // 00c3ce82
        jb l_00c3cf32 // 00c3ce84
        fstp st(0) // 00c3ce8a
        fld qword ptr [esi + 0c8h] // 00c3ce8c
        fstp qword ptr [esi + 098h] // 00c3ce92
        fld qword ptr [esi + 0d0h] // 00c3ce98
        fstp qword ptr [esi + 0a0h] // 00c3ce9e
        fld qword ptr [esi + 0d8h] // 00c3cea4
        fstp qword ptr [esi + 0a8h] // 00c3ceaa
        fld qword ptr [esi + 0128h] // 00c3ceb0
        fstp qword ptr [esi + 0f8h] // 00c3ceb6
        fld qword ptr [esi + 0130h] // 00c3cebc
        fstp qword ptr [esi + 0100h] // 00c3cec2
        fld qword ptr [esi + 0138h] // 00c3cec8
        fstp qword ptr [esi + 0108h] // 00c3cece
        fld qword ptr [esi + 0188h] // 00c3ced4
        fstp qword ptr [esi + 0158h] // 00c3ceda
        fld qword ptr [esi + 0190h] // 00c3cee0
        fstp qword ptr [esi + 0160h] // 00c3cee6
        fld qword ptr [esi + 0198h] // 00c3ceec
        fstp qword ptr [esi + 0168h] // 00c3cef2
        fld qword ptr [esi + 098h] // 00c3cef8
        fchs  // 00c3cefe
        fld qword ptr [esi + 0a0h] // 00c3cf00
        fchs  // 00c3cf06
        fld qword ptr [esi + 0a8h] // 00c3cf08
        fchs  // 00c3cf0e
        fxch st(2) // 00c3cf10
        fstp qword ptr [esi + 080h] // 00c3cf12
        fstp qword ptr [esi + 088h] // 00c3cf18
        fstp qword ptr [esi + 090h] // 00c3cf1e
        mov dword ptr [esi + 01b8h], 1 // 00c3cf24
        add esp, 064h // 00c3cf2e
        pop esi
        ret  // 00c3cf31
    l_00c3cf32:
        fld qword ptr [esi + 0c8h] // 00c3cf32
        fsub qword ptr [esi + 098h] // 00c3cf38
        fld qword ptr [esi + 0d0h] // 00c3cf3e
        fsub qword ptr [esi + 0a0h] // 00c3cf44
        fld qword ptr [esi + 0d8h] // 00c3cf4a
        fsub qword ptr [esi + 0a8h] // 00c3cf50
        fld qword ptr [esi + 0a0h] // 00c3cf56
        fmulp st(2), st(0) // 00c3cf5c
        fxch st(2) // 00c3cf5e
        fmul qword ptr [esi + 098h] // 00c3cf60
        faddp st(1), st(0) // 00c3cf66
        fld qword ptr [esi + 0a8h] // 00c3cf68
        fmulp st(2), st(0) // 00c3cf6e
        faddp st(1), st(0) // 00c3cf70
        fxch st(1) // 00c3cf72
        fcomi st(0), st(1) // 00c3cf74
        fstp st(1) // 00c3cf76
        jb l_00c3d21c // 00c3cf78
        fld qword ptr [esi + 098h] // 00c3cf7e
        fsub qword ptr [esi + 0c8h] // 00c3cf84
        fld qword ptr [esi + 0a0h] // 00c3cf8a
        fsub qword ptr [esi + 0d0h] // 00c3cf90
        fld qword ptr [esi + 0a8h] // 00c3cf96
        fsub qword ptr [esi + 0d8h] // 00c3cf9c
        fld qword ptr [esi + 0d0h] // 00c3cfa2
        fmulp st(2), st(0) // 00c3cfa8
        fld qword ptr [esi + 0c8h] // 00c3cfaa
        fmulp st(3), st(0) // 00c3cfb0
        fxch st(1) // 00c3cfb2
        faddp st(2), st(0) // 00c3cfb4
        fmul qword ptr [esi + 0d8h] // 00c3cfb6
        faddp st(1), st(0) // 00c3cfbc
        fxch st(1) // 00c3cfbe
        fcomi st(0), st(1) // 00c3cfc0
        fstp st(1) // 00c3cfc2
        jb l_00c3d21c // 00c3cfc4
        fld qword ptr [esi + 0b0h] // 00c3cfca
        fsub qword ptr [esi + 098h] // 00c3cfd0
        fld qword ptr [esi + 0b8h] // 00c3cfd6
        fsub qword ptr [esi + 0a0h] // 00c3cfdc
        fld qword ptr [esi + 0c0h] // 00c3cfe2
        fsub qword ptr [esi + 0a8h] // 00c3cfe8
        fld qword ptr [esi + 0c8h] // 00c3cfee
        fsub qword ptr [esi + 098h] // 00c3cff4
        fld qword ptr [esi + 0d0h] // 00c3cffa
        fsub qword ptr [esi + 0a0h] // 00c3d000
        fld qword ptr [esi + 0d8h] // 00c3d006
        fsub qword ptr [esi + 0a8h] // 00c3d00c
        fst qword ptr [esp + 028h] // 00c3d012
        fld st(1) // 00c3d016
        fmul st(0), st(4) // 00c3d018
        fxch st(1) // 00c3d01a
        fmul st(0), st(5) // 00c3d01c
        fsubp st(1), st(0) // 00c3d01e
        fstp qword ptr [esp] // 00c3d020
        fld st(4) // 00c3d023
        fmul qword ptr [esp + 028h] // 00c3d025
        fld st(2) // 00c3d029
        fmulp st(4), st(0) // 00c3d02b
        fsubrp st(3), st(0) // 00c3d02d
        fxch st(1) // 00c3d02f
        fmulp st(3), st(0) // 00c3d031
        fmulp st(3), st(0) // 00c3d033
        fxch st(1) // 00c3d035
        fsubrp st(2), st(0) // 00c3d037
        fld qword ptr [esi + 0c8h] // 00c3d039
        fsub qword ptr [esi + 098h] // 00c3d03f
        fld qword ptr [esi + 0d0h] // 00c3d045
        fsub qword ptr [esi + 0a0h] // 00c3d04b
        fld qword ptr [esi + 0d8h] // 00c3d051
        fsub qword ptr [esi + 0a8h] // 00c3d057
        fld st(1) // 00c3d05d
        fmul st(0), st(5) // 00c3d05f
        fld st(1) // 00c3d061
        fmul st(0), st(5) // 00c3d063
        fsubp st(1), st(0) // 00c3d065
        fstp qword ptr [esp + 018h] // 00c3d067
        fld qword ptr [esp] // 00c3d06b
        fld st(0) // 00c3d06e
        fmulp st(2), st(0) // 00c3d070
        fld st(3) // 00c3d072
        fmulp st(6), st(0) // 00c3d074
        fxch st(1) // 00c3d076
        fsubrp st(5), st(0) // 00c3d078
        fxch st(2) // 00c3d07a
        fmulp st(3), st(0) // 00c3d07c
        fmulp st(1), st(0) // 00c3d07e
        fsubp st(1), st(0) // 00c3d080
        fld qword ptr [esi + 0a0h] // 00c3d082
        fmulp st(2), st(0) // 00c3d088
        fld qword ptr [esp + 018h] // 00c3d08a
        fmul qword ptr [esi + 098h] // 00c3d08e
        faddp st(2), st(0) // 00c3d094
        fmul qword ptr [esi + 0a8h] // 00c3d096
        faddp st(1), st(0) // 00c3d09c
        fxch st(1) // 00c3d09e
        fcomi st(0), st(1) // 00c3d0a0
        fstp st(1) // 00c3d0a2
        jbe l_00c3d21c // 00c3d0a4
        fstp st(0) // 00c3d0aa
        fld qword ptr [esi + 098h] // 00c3d0ac
        fstp qword ptr [esi + 098h] // 00c3d0b2
        fld qword ptr [esi + 0a0h] // 00c3d0b8
        fstp qword ptr [esi + 0a0h] // 00c3d0be
        fld qword ptr [esi + 0a8h] // 00c3d0c4
        fstp qword ptr [esi + 0a8h] // 00c3d0ca
        fld qword ptr [esi + 0c8h] // 00c3d0d0
        fstp qword ptr [esi + 0b0h] // 00c3d0d6
        fld qword ptr [esi + 0d0h] // 00c3d0dc
        fstp qword ptr [esi + 0b8h] // 00c3d0e2
        fld qword ptr [esi + 0d8h] // 00c3d0e8
        fstp qword ptr [esi + 0c0h] // 00c3d0ee
        fld qword ptr [esi + 0f8h] // 00c3d0f4
        fstp qword ptr [esi + 0f8h] // 00c3d0fa
        fld qword ptr [esi + 0100h] // 00c3d100
        fstp qword ptr [esi + 0100h] // 00c3d106
        fld qword ptr [esi + 0108h] // 00c3d10c
        fstp qword ptr [esi + 0108h] // 00c3d112
        fld qword ptr [esi + 0158h] // 00c3d118
        fstp qword ptr [esi + 0158h] // 00c3d11e
        fld qword ptr [esi + 0160h] // 00c3d124
        fstp qword ptr [esi + 0160h] // 00c3d12a
        fld qword ptr [esi + 0168h] // 00c3d130
        fstp qword ptr [esi + 0168h] // 00c3d136
        fld qword ptr [esi + 0128h] // 00c3d13c
        fstp qword ptr [esi + 0110h] // 00c3d142
        fld qword ptr [esi + 0130h] // 00c3d148
        fstp qword ptr [esi + 0118h] // 00c3d14e
        fld qword ptr [esi + 0138h] // 00c3d154
        fstp qword ptr [esi + 0120h] // 00c3d15a
        fld qword ptr [esi + 0188h] // 00c3d160
        fstp qword ptr [esi + 0170h] // 00c3d166
        fld qword ptr [esi + 0190h] // 00c3d16c
        fstp qword ptr [esi + 0178h] // 00c3d172
        fld qword ptr [esi + 0198h] // 00c3d178
        fstp qword ptr [esi + 0180h] // 00c3d17e
        mov dword ptr [esi + 01b8h], 2 // 00c3d184
        fld qword ptr [esi + 0b0h] // 00c3d18e
        fsub qword ptr [esi + 098h] // 00c3d194
        fld qword ptr [esi + 0b8h] // 00c3d19a
        fsub qword ptr [esi + 0a0h] // 00c3d1a0
        fld qword ptr [esi + 0c0h] // 00c3d1a6
        fsub qword ptr [esi + 0a8h] // 00c3d1ac
        fld qword ptr [esi + 0a0h] // 00c3d1b2
        fmul st(0), st(2) // 00c3d1b8
        fld qword ptr [esi + 098h] // 00c3d1ba
        fmul st(0), st(4) // 00c3d1c0
        faddp st(1), st(0) // 00c3d1c2
        fld qword ptr [esi + 0a8h] // 00c3d1c4
        fmul st(0), st(2) // 00c3d1ca
        faddp st(1), st(0) // 00c3d1cc
        fld st(2) // 00c3d1ce
        fmul st(0), st(3) // 00c3d1d0
        fld st(4) // 00c3d1d2
        fmul st(0), st(5) // 00c3d1d4
        faddp st(1), st(0) // 00c3d1d6
        fld st(2) // 00c3d1d8
        fmul st(0), st(3) // 00c3d1da
        faddp st(1), st(0) // 00c3d1dc
        fdivp st(1), st(0) // 00c3d1de
        fld st(0) // 00c3d1e0
        fmulp st(4), st(0) // 00c3d1e2
        fld st(0) // 00c3d1e4
        fmulp st(3), st(0) // 00c3d1e6
        fmulp st(1), st(0) // 00c3d1e8
        fxch st(2) // 00c3d1ea
        fsub qword ptr [esi + 098h] // 00c3d1ec
        fxch st(1) // 00c3d1f2
        fsub qword ptr [esi + 0a0h] // 00c3d1f4
        fxch st(2) // 00c3d1fa
        fsub qword ptr [esi + 0a8h] // 00c3d1fc
        fxch st(1) // 00c3d202
        fstp qword ptr [esi + 080h] // 00c3d204
        fxch st(1) // 00c3d20a
        fstp qword ptr [esi + 088h] // 00c3d20c
        fstp qword ptr [esi + 090h] // 00c3d212
        add esp, 064h // 00c3d218
        pop esi
        ret  // 00c3d21b
    l_00c3d21c:
        fld qword ptr [esi + 0c8h] // 00c3d21c
        fsub qword ptr [esi + 0b0h] // 00c3d222
        fld qword ptr [esi + 0d0h] // 00c3d228
        fsub qword ptr [esi + 0b8h] // 00c3d22e
        fld qword ptr [esi + 0d8h] // 00c3d234
        fsub qword ptr [esi + 0c0h] // 00c3d23a
        fld qword ptr [esi + 0b8h] // 00c3d240
        fmulp st(2), st(0) // 00c3d246
        fxch st(2) // 00c3d248
        fmul qword ptr [esi + 0b0h] // 00c3d24a
        faddp st(1), st(0) // 00c3d250
        fld qword ptr [esi + 0c0h] // 00c3d252
        fmulp st(2), st(0) // 00c3d258
        faddp st(1), st(0) // 00c3d25a
        fxch st(1) // 00c3d25c
        fcomi st(0), st(1) // 00c3d25e
        fstp st(1) // 00c3d260
        jb l_00c3d504 // 00c3d262
        fld qword ptr [esi + 0b0h] // 00c3d268
        fsub qword ptr [esi + 0c8h] // 00c3d26e
        fld qword ptr [esi + 0b8h] // 00c3d274
        fsub qword ptr [esi + 0d0h] // 00c3d27a
        fld qword ptr [esi + 0c0h] // 00c3d280
        fsub qword ptr [esi + 0d8h] // 00c3d286
        fld qword ptr [esi + 0d0h] // 00c3d28c
        fmulp st(2), st(0) // 00c3d292
        fxch st(2) // 00c3d294
        fmul qword ptr [esi + 0c8h] // 00c3d296
        faddp st(1), st(0) // 00c3d29c
        fld qword ptr [esi + 0d8h] // 00c3d29e
        fmulp st(2), st(0) // 00c3d2a4
        faddp st(1), st(0) // 00c3d2a6
        fxch st(1) // 00c3d2a8
        fcomi st(0), st(1) // 00c3d2aa
        fstp st(1) // 00c3d2ac
        jb l_00c3d504 // 00c3d2ae
        fld qword ptr [esi + 098h] // 00c3d2b4
        fsub qword ptr [esi + 0b0h] // 00c3d2ba
        fld qword ptr [esi + 0a0h] // 00c3d2c0
        fsub qword ptr [esi + 0b8h] // 00c3d2c6
        fld qword ptr [esi + 0a8h] // 00c3d2cc
        fsub qword ptr [esi + 0c0h] // 00c3d2d2
        fld qword ptr [esi + 0c8h] // 00c3d2d8
        fsub qword ptr [esi + 0b0h] // 00c3d2de
        fld qword ptr [esi + 0d0h] // 00c3d2e4
        fsub qword ptr [esi + 0b8h] // 00c3d2ea
        fld qword ptr [esi + 0d8h] // 00c3d2f0
        fsub qword ptr [esi + 0c0h] // 00c3d2f6
        fst qword ptr [esp + 028h] // 00c3d2fc
        fld st(1) // 00c3d300
        fmul st(0), st(4) // 00c3d302
        fxch st(1) // 00c3d304
        fmul st(0), st(5) // 00c3d306
        fsubp st(1), st(0) // 00c3d308
        fstp qword ptr [esp] // 00c3d30a
        fld st(4) // 00c3d30d
        fmul qword ptr [esp + 028h] // 00c3d30f
        fld st(2) // 00c3d313
        fmulp st(4), st(0) // 00c3d315
        fsubrp st(3), st(0) // 00c3d317
        fxch st(1) // 00c3d319
        fmulp st(3), st(0) // 00c3d31b
        fmulp st(3), st(0) // 00c3d31d
        fxch st(1) // 00c3d31f
        fsubrp st(2), st(0) // 00c3d321
        fld qword ptr [esi + 0c8h] // 00c3d323
        fsub qword ptr [esi + 0b0h] // 00c3d329
        fld qword ptr [esi + 0d0h] // 00c3d32f
        fsub qword ptr [esi + 0b8h] // 00c3d335
        fld qword ptr [esi + 0d8h] // 00c3d33b
        fsub qword ptr [esi + 0c0h] // 00c3d341
        fld st(1) // 00c3d347
        fmul st(0), st(5) // 00c3d349
        fld st(1) // 00c3d34b
        fmul st(0), st(5) // 00c3d34d
        fsubp st(1), st(0) // 00c3d34f
        fstp qword ptr [esp + 018h] // 00c3d351
        fld qword ptr [esp] // 00c3d355
        fld st(0) // 00c3d358
        fmulp st(2), st(0) // 00c3d35a
        fld st(3) // 00c3d35c
        fmulp st(6), st(0) // 00c3d35e
        fxch st(1) // 00c3d360
        fsubrp st(5), st(0) // 00c3d362
        fxch st(2) // 00c3d364
        fmulp st(3), st(0) // 00c3d366
        fmulp st(1), st(0) // 00c3d368
        fsubp st(1), st(0) // 00c3d36a
        fld qword ptr [esi + 0b8h] // 00c3d36c
        fmulp st(2), st(0) // 00c3d372
        fld qword ptr [esi + 0b0h] // 00c3d374
        fmul qword ptr [esp + 018h] // 00c3d37a
        faddp st(2), st(0) // 00c3d37e
        fmul qword ptr [esi + 0c0h] // 00c3d380
        faddp st(1), st(0) // 00c3d386
        fxch st(1) // 00c3d388
        fcomip st(0), st(1) // 00c3d38a
        fstp st(0) // 00c3d38c
        jbe l_00c3d506 // 00c3d38e
        fld qword ptr [esi + 0b0h] // 00c3d394
        fstp qword ptr [esi + 098h] // 00c3d39a
        fld qword ptr [esi + 0b8h] // 00c3d3a0
        fstp qword ptr [esi + 0a0h] // 00c3d3a6
        fld qword ptr [esi + 0c0h] // 00c3d3ac
        fstp qword ptr [esi + 0a8h] // 00c3d3b2
        fld qword ptr [esi + 0c8h] // 00c3d3b8
        fstp qword ptr [esi + 0b0h] // 00c3d3be
        fld qword ptr [esi + 0d0h] // 00c3d3c4
        fstp qword ptr [esi + 0b8h] // 00c3d3ca
        fld qword ptr [esi + 0d8h] // 00c3d3d0
        fstp qword ptr [esi + 0c0h] // 00c3d3d6
        fld qword ptr [esi + 0110h] // 00c3d3dc
        fstp qword ptr [esi + 0f8h] // 00c3d3e2
        fld qword ptr [esi + 0118h] // 00c3d3e8
        fstp qword ptr [esi + 0100h] // 00c3d3ee
        fld qword ptr [esi + 0120h] // 00c3d3f4
        fstp qword ptr [esi + 0108h] // 00c3d3fa
        fld qword ptr [esi + 0170h] // 00c3d400
        fstp qword ptr [esi + 0158h] // 00c3d406
        fld qword ptr [esi + 0178h] // 00c3d40c
        fstp qword ptr [esi + 0160h] // 00c3d412
        fld qword ptr [esi + 0180h] // 00c3d418
        fstp qword ptr [esi + 0168h] // 00c3d41e
        fld qword ptr [esi + 0128h] // 00c3d424
        fstp qword ptr [esi + 0110h] // 00c3d42a
        fld qword ptr [esi + 0130h] // 00c3d430
        fstp qword ptr [esi + 0118h] // 00c3d436
        fld qword ptr [esi + 0138h] // 00c3d43c
        fstp qword ptr [esi + 0120h] // 00c3d442
        fld qword ptr [esi + 0188h] // 00c3d448
        fstp qword ptr [esi + 0170h] // 00c3d44e
        fld qword ptr [esi + 0190h] // 00c3d454
        fstp qword ptr [esi + 0178h] // 00c3d45a
        fld qword ptr [esi + 0198h] // 00c3d460
        fstp qword ptr [esi + 0180h] // 00c3d466
        mov dword ptr [esi + 01b8h], 2 // 00c3d46c
        fld qword ptr [esi + 0b0h] // 00c3d476
        fsub qword ptr [esi + 098h] // 00c3d47c
        fld qword ptr [esi + 0b8h] // 00c3d482
        fsub qword ptr [esi + 0a0h] // 00c3d488
        fld qword ptr [esi + 0c0h] // 00c3d48e
        fsub qword ptr [esi + 0a8h] // 00c3d494
        fld qword ptr [esi + 0a0h] // 00c3d49a
        fmul st(0), st(2) // 00c3d4a0
        fld st(3) // 00c3d4a2
        fmul qword ptr [esi + 098h] // 00c3d4a4
        faddp st(1), st(0) // 00c3d4aa
        fld qword ptr [esi + 0a8h] // 00c3d4ac
        fmul st(0), st(2) // 00c3d4b2
        faddp st(1), st(0) // 00c3d4b4
        fld st(2) // 00c3d4b6
        fmul st(0), st(3) // 00c3d4b8
        fld st(4) // 00c3d4ba
        fmul st(0), st(5) // 00c3d4bc
        faddp st(1), st(0) // 00c3d4be
        fld st(2) // 00c3d4c0
        fmul st(0), st(3) // 00c3d4c2
        faddp st(1), st(0) // 00c3d4c4
        fdivp st(1), st(0) // 00c3d4c6
        fld st(0) // 00c3d4c8
        fmulp st(4), st(0) // 00c3d4ca
        fld st(0) // 00c3d4cc
        fmulp st(3), st(0) // 00c3d4ce
        fmulp st(1), st(0) // 00c3d4d0
        fxch st(2) // 00c3d4d2
        fsub qword ptr [esi + 098h] // 00c3d4d4
        fxch st(1) // 00c3d4da
        fsub qword ptr [esi + 0a0h] // 00c3d4dc
        fxch st(2) // 00c3d4e2
        fsub qword ptr [esi + 0a8h] // 00c3d4e4
        fxch st(1) // 00c3d4ea
        fstp qword ptr [esi + 080h] // 00c3d4ec
        fxch st(1) // 00c3d4f2
        fstp qword ptr [esi + 088h] // 00c3d4f4
        fstp qword ptr [esi + 090h] // 00c3d4fa
        add esp, 064h // 00c3d500
        pop esi
        ret  // 00c3d503
    l_00c3d504:
        fstp st(0) // 00c3d504
    l_00c3d506:
        fld qword ptr [esi + 0b0h] // 00c3d506
        lea ecx, [esp] // 00c3d50c
        fsub qword ptr [esi + 098h] // 00c3d50f
        lea edx, [esp + 018h] // 00c3d515
        lea eax, [esp + 048h] // 00c3d519
        fstp qword ptr [esp] // 00c3d51d
        fld qword ptr [esi + 0b8h] // 00c3d520
        fsub qword ptr [esi + 0a0h] // 00c3d526
        fstp qword ptr [esp + 8] // 00c3d52c
        fld qword ptr [esi + 0c0h] // 00c3d530
        fsub qword ptr [esi + 0a8h] // 00c3d536
        fstp qword ptr [esp + 010h] // 00c3d53c
        fld qword ptr [esi + 0c8h] // 00c3d540
        fsub qword ptr [esi + 098h] // 00c3d546
        fstp qword ptr [esp + 018h] // 00c3d54c
        fld qword ptr [esi + 0d0h] // 00c3d550
        fsub qword ptr [esi + 0a0h] // 00c3d556
        fstp qword ptr [esp + 020h] // 00c3d55c
        fld qword ptr [esi + 0d8h] // 00c3d560
        fsub qword ptr [esi + 0a8h] // 00c3d566
        fstp qword ptr [esp + 028h] // 00c3d56c
        call cross_reference // 00c3d570
        mov ecx, eax // 00c3d575
        fld qword ptr [ecx] // 00c3d577
        lea eax, [esi + 080h] // 00c3d579
        fstp qword ptr [eax] // 00c3d57f
        sub esp, 8 // 00c3d581
        fld qword ptr [ecx + 8] // 00c3d584
        fstp qword ptr [eax + 8] // 00c3d587
        fld qword ptr [ecx + 010h] // 00c3d58a
        fstp qword ptr [eax + 010h] // 00c3d58d
        fld qword ptr [eax + 8] // 00c3d590
        fld qword ptr [eax] // 00c3d593
        fld qword ptr [eax + 010h] // 00c3d595
        fld qword ptr [eax + 8] // 00c3d598
        fmul qword ptr [esi + 0a0h] // 00c3d59b
        fld qword ptr [eax] // 00c3d5a1
        fmul qword ptr [esi + 098h] // 00c3d5a3
        faddp st(1), st(0) // 00c3d5a9
        fld qword ptr [eax + 010h] // 00c3d5ab
        fmul qword ptr [esi + 0a8h] // 00c3d5ae
        faddp st(1), st(0) // 00c3d5b4
        fld st(2) // 00c3d5b6
        fmulp st(3), st(0) // 00c3d5b8
        fld st(3) // 00c3d5ba
        fmulp st(4), st(0) // 00c3d5bc
        fxch st(2) // 00c3d5be
        faddp st(3), st(0) // 00c3d5c0
        fmul st(0), st(0) // 00c3d5c2
        faddp st(2), st(0) // 00c3d5c4
        fdivrp st(1), st(0) // 00c3d5c6
        fchs  // 00c3d5c8
        fstp qword ptr [esp] // 00c3d5ca
        call scale_reference // 00c3d5cd
        add esp, 064h // 00c3d5d2
        pop esi
        ret  // 00c3d5d5
    l_00c3d5d6:
        fld qword ptr [esi + 098h] // 00c3d5d6
        fsub qword ptr [esi + 0e0h] // 00c3d5dc
        fld qword ptr [esi + 0a0h] // 00c3d5e2
        fsub qword ptr [esi + 0e8h] // 00c3d5e8
        fld qword ptr [esi + 0a8h] // 00c3d5ee
        fsub qword ptr [esi + 0f0h] // 00c3d5f4
        fld qword ptr [esi + 0e8h] // 00c3d5fa
        fmulp st(2), st(0) // 00c3d600
        fxch st(2) // 00c3d602
        fmul qword ptr [esi + 0e0h] // 00c3d604
        faddp st(1), st(0) // 00c3d60a
        fld qword ptr [esi + 0f0h] // 00c3d60c
        fmulp st(2), st(0) // 00c3d612
        faddp st(1), st(0) // 00c3d614
        fldz  // 00c3d616
        fxch st(1) // 00c3d618
        fcomip st(0), st(1) // 00c3d61a
        jb l_00c3d75a // 00c3d61c
        fld qword ptr [esi + 0b0h] // 00c3d622
        fsub qword ptr [esi + 0e0h] // 00c3d628
        fld qword ptr [esi + 0b8h] // 00c3d62e
        fsub qword ptr [esi + 0e8h] // 00c3d634
        fld qword ptr [esi + 0c0h] // 00c3d63a
        fsub qword ptr [esi + 0f0h] // 00c3d640
        fld qword ptr [esi + 0e8h] // 00c3d646
        fmulp st(2), st(0) // 00c3d64c
        fxch st(2) // 00c3d64e
        fmul qword ptr [esi + 0e0h] // 00c3d650
        faddp st(1), st(0) // 00c3d656
        fld qword ptr [esi + 0f0h] // 00c3d658
        fmulp st(2), st(0) // 00c3d65e
        faddp st(1), st(0) // 00c3d660
        fcomip st(0), st(1) // 00c3d662
        jb l_00c3d75a // 00c3d664
        fld qword ptr [esi + 0c8h] // 00c3d66a
        fsub qword ptr [esi + 0e0h] // 00c3d670
        fld qword ptr [esi + 0d0h] // 00c3d676
        fsub qword ptr [esi + 0e8h] // 00c3d67c
        fld qword ptr [esi + 0d8h] // 00c3d682
        fsub qword ptr [esi + 0f0h] // 00c3d688
        fld qword ptr [esi + 0e8h] // 00c3d68e
        fmulp st(2), st(0) // 00c3d694
        fxch st(2) // 00c3d696
        fmul qword ptr [esi + 0e0h] // 00c3d698
        faddp st(1), st(0) // 00c3d69e
        fld qword ptr [esi + 0f0h] // 00c3d6a0
        fmulp st(2), st(0) // 00c3d6a6
        faddp st(1), st(0) // 00c3d6a8
        fcomip st(0), st(1) // 00c3d6aa
        jb l_00c3d75a // 00c3d6ac
        fstp st(0) // 00c3d6b2
        fld qword ptr [esi + 0e0h] // 00c3d6b4
        fstp qword ptr [esi + 098h] // 00c3d6ba
        fld qword ptr [esi + 0e8h] // 00c3d6c0
        fstp qword ptr [esi + 0a0h] // 00c3d6c6
        fld qword ptr [esi + 0f0h] // 00c3d6cc
        fstp qword ptr [esi + 0a8h] // 00c3d6d2
        fld qword ptr [esi + 0140h] // 00c3d6d8
        fstp qword ptr [esi + 0f8h] // 00c3d6de
        fld qword ptr [esi + 0148h] // 00c3d6e4
        fstp qword ptr [esi + 0100h] // 00c3d6ea
        fld qword ptr [esi + 0150h] // 00c3d6f0
        fstp qword ptr [esi + 0108h] // 00c3d6f6
        fld qword ptr [esi + 01a0h] // 00c3d6fc
        fstp qword ptr [esi + 0158h] // 00c3d702
        fld qword ptr [esi + 01a8h] // 00c3d708
        fstp qword ptr [esi + 0160h] // 00c3d70e
        fld qword ptr [esi + 01b0h] // 00c3d714
        fstp qword ptr [esi + 0168h] // 00c3d71a
        fld qword ptr [esi + 098h] // 00c3d720
        fchs  // 00c3d726
        fld qword ptr [esi + 0a0h] // 00c3d728
        fchs  // 00c3d72e
        fld qword ptr [esi + 0a8h] // 00c3d730
        fchs  // 00c3d736
        fxch st(2) // 00c3d738
        fstp qword ptr [esi + 080h] // 00c3d73a
        fstp qword ptr [esi + 088h] // 00c3d740
        fstp qword ptr [esi + 090h] // 00c3d746
        mov dword ptr [esi + 01b8h], 1 // 00c3d74c
        add esp, 064h // 00c3d756
        pop esi
        ret  // 00c3d759
    l_00c3d75a:
        fld qword ptr [esi + 0b0h] // 00c3d75a
        fsub qword ptr [esi + 098h] // 00c3d760
        fld qword ptr [esi + 0b8h] // 00c3d766
        fsub qword ptr [esi + 0a0h] // 00c3d76c
        fld qword ptr [esi + 0c0h] // 00c3d772
        fsub qword ptr [esi + 0a8h] // 00c3d778
        fld qword ptr [esi + 0e0h] // 00c3d77e
        fsub qword ptr [esi + 098h] // 00c3d784
        fld qword ptr [esi + 0e8h] // 00c3d78a
        fsub qword ptr [esi + 0a0h] // 00c3d790
        fld qword ptr [esi + 0f0h] // 00c3d796
        fsub qword ptr [esi + 0a8h] // 00c3d79c
        fst qword ptr [esp + 028h] // 00c3d7a2
        fld st(1) // 00c3d7a6
        fmul st(0), st(4) // 00c3d7a8
        fxch st(1) // 00c3d7aa
        fmul st(0), st(5) // 00c3d7ac
        fsubp st(1), st(0) // 00c3d7ae
        fstp qword ptr [esp] // 00c3d7b0
        fld st(4) // 00c3d7b3
        fmul qword ptr [esp + 028h] // 00c3d7b5
        fld st(2) // 00c3d7b9
        fmulp st(4), st(0) // 00c3d7bb
        fsubrp st(3), st(0) // 00c3d7bd
        fxch st(2) // 00c3d7bf
        fstp qword ptr [esp + 8] // 00c3d7c1
        fmulp st(2), st(0) // 00c3d7c5
        fmulp st(2), st(0) // 00c3d7c7
        fsubrp st(1), st(0) // 00c3d7c9
        fstp qword ptr [esp + 010h] // 00c3d7cb
        fld qword ptr [esi + 0e0h] // 00c3d7cf
        fsub qword ptr [esi + 098h] // 00c3d7d5
        fld qword ptr [esi + 0e8h] // 00c3d7db
        fsub qword ptr [esi + 0a0h] // 00c3d7e1
        fld qword ptr [esi + 0f0h] // 00c3d7e7
        fsub qword ptr [esi + 0a8h] // 00c3d7ed
        fld qword ptr [esi + 0c8h] // 00c3d7f3
        fsub qword ptr [esi + 098h] // 00c3d7f9
        fld qword ptr [esi + 0d0h] // 00c3d7ff
        fsub qword ptr [esi + 0a0h] // 00c3d805
        fld qword ptr [esi + 0d8h] // 00c3d80b
        fsub qword ptr [esi + 0a8h] // 00c3d811
        fst qword ptr [esp + 040h] // 00c3d817
        fld st(1) // 00c3d81b
        fmul st(0), st(4) // 00c3d81d
        fxch st(1) // 00c3d81f
        fmul st(0), st(5) // 00c3d821
        fsubp st(1), st(0) // 00c3d823
        fstp qword ptr [esp + 018h] // 00c3d825
        fld st(4) // 00c3d829
        fmul qword ptr [esp + 040h] // 00c3d82b
        fld st(2) // 00c3d82f
        fmulp st(4), st(0) // 00c3d831
        fsubrp st(3), st(0) // 00c3d833
        fxch st(1) // 00c3d835
        fmulp st(3), st(0) // 00c3d837
        fmulp st(3), st(0) // 00c3d839
        fxch st(1) // 00c3d83b
        fsubrp st(2), st(0) // 00c3d83d
        fld qword ptr [esi + 0e0h] // 00c3d83f
        fsub qword ptr [esi + 098h] // 00c3d845
        fld qword ptr [esi + 0e8h] // 00c3d84b
        fsub qword ptr [esi + 0a0h] // 00c3d851
        fld qword ptr [esi + 0f0h] // 00c3d857
        fsub qword ptr [esi + 0a8h] // 00c3d85d
        fld qword ptr [esi + 0a0h] // 00c3d863
        fmulp st(2), st(0) // 00c3d869
        fxch st(2) // 00c3d86b
        fmul qword ptr [esi + 098h] // 00c3d86d
        faddp st(1), st(0) // 00c3d873
        fld qword ptr [esi + 0a8h] // 00c3d875
        fmulp st(2), st(0) // 00c3d87b
        faddp st(1), st(0) // 00c3d87d
        fxch st(3) // 00c3d87f
        fcomi st(0), st(3) // 00c3d881
        fstp st(3) // 00c3d883
        jb l_00c3db33 // 00c3d885
        fld qword ptr [esi + 098h] // 00c3d88b
        fsub qword ptr [esi + 0e0h] // 00c3d891
        fld qword ptr [esi + 0a0h] // 00c3d897
        fsub qword ptr [esi + 0e8h] // 00c3d89d
        fld qword ptr [esi + 0a8h] // 00c3d8a3
        fsub qword ptr [esi + 0f0h] // 00c3d8a9
        fld qword ptr [esi + 0e8h] // 00c3d8af
        fmulp st(2), st(0) // 00c3d8b5
        fxch st(2) // 00c3d8b7
        fmul qword ptr [esi + 0e0h] // 00c3d8b9
        faddp st(1), st(0) // 00c3d8bf
        fld qword ptr [esi + 0f0h] // 00c3d8c1
        fmulp st(2), st(0) // 00c3d8c7
        faddp st(1), st(0) // 00c3d8c9
        fxch st(3) // 00c3d8cb
        fcomi st(0), st(3) // 00c3d8cd
        fstp st(3) // 00c3d8cf
        jb l_00c3db33 // 00c3d8d1
        fld qword ptr [esi + 0e0h] // 00c3d8d7
        fsub qword ptr [esi + 098h] // 00c3d8dd
        fld qword ptr [esi + 0e8h] // 00c3d8e3
        fsub qword ptr [esi + 0a0h] // 00c3d8e9
        fld qword ptr [esi + 0f0h] // 00c3d8ef
        fsub qword ptr [esi + 0a8h] // 00c3d8f5
        fst qword ptr [esp + 040h] // 00c3d8fb
        fld st(1) // 00c3d8ff
        fmul qword ptr [esp + 010h] // 00c3d901
        fxch st(1) // 00c3d905
        fmul qword ptr [esp + 8] // 00c3d907
        fsubp st(1), st(0) // 00c3d90b
        fstp qword ptr [esp + 048h] // 00c3d90d
        fld qword ptr [esp] // 00c3d911
        fmul qword ptr [esp + 040h] // 00c3d914
        fld st(2) // 00c3d918
        fmul qword ptr [esp + 010h] // 00c3d91a
        fsubp st(1), st(0) // 00c3d91e
        fxch st(2) // 00c3d920
        fmul qword ptr [esp + 8] // 00c3d922
        fld qword ptr [esp] // 00c3d926
        fmulp st(2), st(0) // 00c3d929
        fsubrp st(1), st(0) // 00c3d92b
        fld qword ptr [esi + 0a0h] // 00c3d92d
        fmulp st(2), st(0) // 00c3d933
        fld qword ptr [esp + 048h] // 00c3d935
        fmul qword ptr [esi + 098h] // 00c3d939
        faddp st(2), st(0) // 00c3d93f
        fmul qword ptr [esi + 0a8h] // 00c3d941
        faddp st(1), st(0) // 00c3d947
        fxch st(3) // 00c3d949
        fcomi st(0), st(3) // 00c3d94b
        fstp st(3) // 00c3d94d
        jb l_00c3db33 // 00c3d94f
        fld qword ptr [esi + 0e0h] // 00c3d955
        fsub qword ptr [esi + 098h] // 00c3d95b
        fld qword ptr [esi + 0e8h] // 00c3d961
        fsub qword ptr [esi + 0a0h] // 00c3d967
        fld qword ptr [esi + 0f0h] // 00c3d96d
        fsub qword ptr [esi + 0a8h] // 00c3d973
        fld st(0) // 00c3d979
        fmul st(0), st(4) // 00c3d97b
        fld st(2) // 00c3d97d
        fmul st(0), st(6) // 00c3d97f
        fsubp st(1), st(0) // 00c3d981
        fld st(3) // 00c3d983
        fmulp st(6), st(0) // 00c3d985
        fld qword ptr [esp + 018h] // 00c3d987
        fmulp st(2), st(0) // 00c3d98b
        fxch st(5) // 00c3d98d
        fsubrp st(1), st(0) // 00c3d98f
        fld qword ptr [esp + 018h] // 00c3d991
        fmulp st(2), st(0) // 00c3d995
        fxch st(2) // 00c3d997
        fmulp st(3), st(0) // 00c3d999
        fsubrp st(2), st(0) // 00c3d99b
        fmul qword ptr [esi + 0a0h] // 00c3d99d
        fxch st(2) // 00c3d9a3
        fmul qword ptr [esi + 098h] // 00c3d9a5
        faddp st(2), st(0) // 00c3d9ab
        fmul qword ptr [esi + 0a8h] // 00c3d9ad
        faddp st(1), st(0) // 00c3d9b3
        fxch st(1) // 00c3d9b5
        fcomi st(0), st(1) // 00c3d9b7
        fstp st(1) // 00c3d9b9
        jb l_00c3db37 // 00c3d9bb
        fstp st(0) // 00c3d9c1
        fld qword ptr [esi + 098h] // 00c3d9c3
        fstp qword ptr [esi + 098h] // 00c3d9c9
        fld qword ptr [esi + 0a0h] // 00c3d9cf
        fstp qword ptr [esi + 0a0h] // 00c3d9d5
        fld qword ptr [esi + 0a8h] // 00c3d9db
        fstp qword ptr [esi + 0a8h] // 00c3d9e1
        fld qword ptr [esi + 0e0h] // 00c3d9e7
        fstp qword ptr [esi + 0b0h] // 00c3d9ed
        fld qword ptr [esi + 0e8h] // 00c3d9f3
        fstp qword ptr [esi + 0b8h] // 00c3d9f9
        fld qword ptr [esi + 0f0h] // 00c3d9ff
        fstp qword ptr [esi + 0c0h] // 00c3da05
        fld qword ptr [esi + 0f8h] // 00c3da0b
        fstp qword ptr [esi + 0f8h] // 00c3da11
        fld qword ptr [esi + 0100h] // 00c3da17
        fstp qword ptr [esi + 0100h] // 00c3da1d
        fld qword ptr [esi + 0108h] // 00c3da23
        fstp qword ptr [esi + 0108h] // 00c3da29
        fld qword ptr [esi + 0140h] // 00c3da2f
        fstp qword ptr [esi + 0110h] // 00c3da35
        fld qword ptr [esi + 0148h] // 00c3da3b
        fstp qword ptr [esi + 0118h] // 00c3da41
        fld qword ptr [esi + 0150h] // 00c3da47
        fstp qword ptr [esi + 0120h] // 00c3da4d
        fld qword ptr [esi + 0158h] // 00c3da53
        fstp qword ptr [esi + 0158h] // 00c3da59
        fld qword ptr [esi + 0160h] // 00c3da5f
        fstp qword ptr [esi + 0160h] // 00c3da65
        fld qword ptr [esi + 0168h] // 00c3da6b
        fstp qword ptr [esi + 0168h] // 00c3da71
        fld qword ptr [esi + 01a0h] // 00c3da77
        fstp qword ptr [esi + 0170h] // 00c3da7d
        fld qword ptr [esi + 01a8h] // 00c3da83
        fstp qword ptr [esi + 0178h] // 00c3da89
        fld qword ptr [esi + 01b0h] // 00c3da8f
        fstp qword ptr [esi + 0180h] // 00c3da95
        fld qword ptr [esi + 0b0h] // 00c3da9b
        fsub qword ptr [esi + 098h] // 00c3daa1
        fld qword ptr [esi + 0b8h] // 00c3daa7
        fsub qword ptr [esi + 0a0h] // 00c3daad
        fld qword ptr [esi + 0c0h] // 00c3dab3
        fsub qword ptr [esi + 0a8h] // 00c3dab9
        fld qword ptr [esi + 0a0h] // 00c3dabf
        fmul st(0), st(2) // 00c3dac5
        fld st(3) // 00c3dac7
        fmul qword ptr [esi + 098h] // 00c3dac9
        faddp st(1), st(0) // 00c3dacf
        fld qword ptr [esi + 0a8h] // 00c3dad1
        fmul st(0), st(2) // 00c3dad7
        faddp st(1), st(0) // 00c3dad9
        fld st(2) // 00c3dadb
        fmul st(0), st(3) // 00c3dadd
        fld st(4) // 00c3dadf
        fmul st(0), st(5) // 00c3dae1
        faddp st(1), st(0) // 00c3dae3
        fld st(2) // 00c3dae5
        fmul st(0), st(3) // 00c3dae7
        faddp st(1), st(0) // 00c3dae9
        fdivp st(1), st(0) // 00c3daeb
        fld st(0) // 00c3daed
        fmulp st(4), st(0) // 00c3daef
        fld st(0) // 00c3daf1
        fmulp st(3), st(0) // 00c3daf3
        fmulp st(1), st(0) // 00c3daf5
        fxch st(2) // 00c3daf7
        fsub qword ptr [esi + 098h] // 00c3daf9
        fxch st(1) // 00c3daff
        fsub qword ptr [esi + 0a0h] // 00c3db01
        fxch st(2) // 00c3db07
        fsub qword ptr [esi + 0a8h] // 00c3db09
        fxch st(1) // 00c3db0f
        fstp qword ptr [esi + 080h] // 00c3db11
        fxch st(1) // 00c3db17
        fstp qword ptr [esi + 088h] // 00c3db19
        fstp qword ptr [esi + 090h] // 00c3db1f
        mov dword ptr [esi + 01b8h], 2 // 00c3db25
        add esp, 064h // 00c3db2f
        pop esi
        ret  // 00c3db32
    l_00c3db33:
        fstp st(1) // 00c3db33
        fstp st(0) // 00c3db35
    l_00c3db37:
        fld qword ptr [esi + 098h] // 00c3db37
        fsub qword ptr [esi + 0b0h] // 00c3db3d
        fld qword ptr [esi + 0a0h] // 00c3db43
        fsub qword ptr [esi + 0b8h] // 00c3db49
        fld qword ptr [esi + 0a8h] // 00c3db4f
        fsub qword ptr [esi + 0c0h] // 00c3db55
        fld qword ptr [esi + 0e0h] // 00c3db5b
        fsub qword ptr [esi + 0b0h] // 00c3db61
        fld qword ptr [esi + 0e8h] // 00c3db67
        fsub qword ptr [esi + 0b8h] // 00c3db6d
        fld qword ptr [esi + 0f0h] // 00c3db73
        fsub qword ptr [esi + 0c0h] // 00c3db79
        fst qword ptr [esp + 058h] // 00c3db7f
        fld st(1) // 00c3db83
        fmul st(0), st(4) // 00c3db85
        fxch st(1) // 00c3db87
        fmul st(0), st(5) // 00c3db89
        fsubp st(1), st(0) // 00c3db8b
        fstp qword ptr [esp] // 00c3db8d
        fld qword ptr [esp + 058h] // 00c3db90
        fmul st(0), st(5) // 00c3db94
        fxch st(3) // 00c3db96
        fmul st(0), st(2) // 00c3db98
        fsubp st(3), st(0) // 00c3db9a
        fxch st(2) // 00c3db9c
        fstp qword ptr [esp + 8] // 00c3db9e
        fmulp st(2), st(0) // 00c3dba2
        fmulp st(2), st(0) // 00c3dba4
        fsubrp st(1), st(0) // 00c3dba6
        fstp qword ptr [esp + 010h] // 00c3dba8
        fld qword ptr [esi + 0e0h] // 00c3dbac
        fsub qword ptr [esi + 0b0h] // 00c3dbb2
        fld qword ptr [esi + 0e8h] // 00c3dbb8
        fsub qword ptr [esi + 0b8h] // 00c3dbbe
        fld qword ptr [esi + 0f0h] // 00c3dbc4
        fsub qword ptr [esi + 0c0h] // 00c3dbca
        fld qword ptr [esi + 0c8h] // 00c3dbd0
        fsub qword ptr [esi + 0b0h] // 00c3dbd6
        fld qword ptr [esi + 0d0h] // 00c3dbdc
        fsub qword ptr [esi + 0b8h] // 00c3dbe2
        fld qword ptr [esi + 0d8h] // 00c3dbe8
        fsub qword ptr [esi + 0c0h] // 00c3dbee
        fst qword ptr [esp + 058h] // 00c3dbf4
        fld st(1) // 00c3dbf8
        fmul st(0), st(4) // 00c3dbfa
        fxch st(1) // 00c3dbfc
        fmul st(0), st(5) // 00c3dbfe
        fsubp st(1), st(0) // 00c3dc00
        fld qword ptr [esp + 058h] // 00c3dc02
        fmul st(0), st(6) // 00c3dc06
        fxch st(4) // 00c3dc08
        fmul st(0), st(3) // 00c3dc0a
        fsubp st(4), st(0) // 00c3dc0c
        fxch st(4) // 00c3dc0e
        fmulp st(2), st(0) // 00c3dc10
        fmulp st(4), st(0) // 00c3dc12
        fsubrp st(3), st(0) // 00c3dc14
        fld qword ptr [esi + 0e0h] // 00c3dc16
        fsub qword ptr [esi + 0b0h] // 00c3dc1c
        fld qword ptr [esi + 0e8h] // 00c3dc22
        fsub qword ptr [esi + 0b8h] // 00c3dc28
        fld qword ptr [esi + 0f0h] // 00c3dc2e
        fsub qword ptr [esi + 0c0h] // 00c3dc34
        fld qword ptr [esi + 0b8h] // 00c3dc3a
        fmulp st(2), st(0) // 00c3dc40
        fld qword ptr [esi + 0b0h] // 00c3dc42
        fmulp st(3), st(0) // 00c3dc48
        fxch st(1) // 00c3dc4a
        faddp st(2), st(0) // 00c3dc4c
        fmul qword ptr [esi + 0c0h] // 00c3dc4e
        faddp st(1), st(0) // 00c3dc54
        fxch st(4) // 00c3dc56
        fcomi st(0), st(4) // 00c3dc58
        fstp st(4) // 00c3dc5a
        jb l_00c3df0e // 00c3dc5c
        fld qword ptr [esi + 0b0h] // 00c3dc62
        fsub qword ptr [esi + 0e0h] // 00c3dc68
        fld qword ptr [esi + 0b8h] // 00c3dc6e
        fsub qword ptr [esi + 0e8h] // 00c3dc74
        fld qword ptr [esi + 0c0h] // 00c3dc7a
        fsub qword ptr [esi + 0f0h] // 00c3dc80
        fld qword ptr [esi + 0e8h] // 00c3dc86
        fmulp st(2), st(0) // 00c3dc8c
        fxch st(2) // 00c3dc8e
        fmul qword ptr [esi + 0e0h] // 00c3dc90
        faddp st(1), st(0) // 00c3dc96
        fld qword ptr [esi + 0f0h] // 00c3dc98
        fmulp st(2), st(0) // 00c3dc9e
        faddp st(1), st(0) // 00c3dca0
        fxch st(4) // 00c3dca2
        fcomi st(0), st(4) // 00c3dca4
        fstp st(4) // 00c3dca6
        jb l_00c3df0e // 00c3dca8
        fld qword ptr [esi + 0e0h] // 00c3dcae
        fsub qword ptr [esi + 0b0h] // 00c3dcb4
        fld qword ptr [esi + 0e8h] // 00c3dcba
        fsub qword ptr [esi + 0b8h] // 00c3dcc0
        fst qword ptr [esp + 020h] // 00c3dcc6
        fld qword ptr [esi + 0f0h] // 00c3dcca
        fsub qword ptr [esi + 0c0h] // 00c3dcd0
        fst qword ptr [esp + 028h] // 00c3dcd6
        fxch st(1) // 00c3dcda
        fmul qword ptr [esp + 010h] // 00c3dcdc
        fxch st(1) // 00c3dce0
        fmul qword ptr [esp + 8] // 00c3dce2
        fsubp st(1), st(0) // 00c3dce6
        fld qword ptr [esp + 028h] // 00c3dce8
        fmul qword ptr [esp] // 00c3dcec
        fld qword ptr [esp + 010h] // 00c3dcef
        fmul st(0), st(3) // 00c3dcf3
        fsubp st(1), st(0) // 00c3dcf5
        fstp qword ptr [esp + 050h] // 00c3dcf7
        fld qword ptr [esp + 8] // 00c3dcfb
        fmulp st(2), st(0) // 00c3dcff
        fld qword ptr [esp + 020h] // 00c3dd01
        fmul qword ptr [esp] // 00c3dd05
        fsubp st(2), st(0) // 00c3dd08
        fld qword ptr [esi + 0b8h] // 00c3dd0a
        fmul qword ptr [esp + 050h] // 00c3dd10
        fld qword ptr [esi + 0b0h] // 00c3dd14
        fmulp st(2), st(0) // 00c3dd1a
        faddp st(1), st(0) // 00c3dd1c
        fld qword ptr [esi + 0c0h] // 00c3dd1e
        fmulp st(2), st(0) // 00c3dd24
        faddp st(1), st(0) // 00c3dd26
        fxch st(4) // 00c3dd28
        fcomi st(0), st(4) // 00c3dd2a
        fstp st(4) // 00c3dd2c
        jb l_00c3df0e // 00c3dd2e
        fld qword ptr [esi + 0e0h] // 00c3dd34
        fsub qword ptr [esi + 0b0h] // 00c3dd3a
        fld qword ptr [esi + 0e8h] // 00c3dd40
        fsub qword ptr [esi + 0b8h] // 00c3dd46
        fld qword ptr [esi + 0f0h] // 00c3dd4c
        fsub qword ptr [esi + 0c0h] // 00c3dd52
        fst qword ptr [esp + 058h] // 00c3dd58
        fmul st(0), st(3) // 00c3dd5c
        fld st(1) // 00c3dd5e
        fmul st(0), st(6) // 00c3dd60
        fsubp st(1), st(0) // 00c3dd62
        fxch st(5) // 00c3dd64
        fmul st(0), st(2) // 00c3dd66
        fld qword ptr [esp + 058h] // 00c3dd68
        fmul st(0), st(5) // 00c3dd6c
        fsubp st(1), st(0) // 00c3dd6e
        fxch st(1) // 00c3dd70
        fmulp st(4), st(0) // 00c3dd72
        fxch st(2) // 00c3dd74
        fmulp st(1), st(0) // 00c3dd76
        fsubp st(2), st(0) // 00c3dd78
        fmul qword ptr [esi + 0b8h] // 00c3dd7a
        fld qword ptr [esi + 0b0h] // 00c3dd80
        fmulp st(3), st(0) // 00c3dd86
        faddp st(2), st(0) // 00c3dd88
        fmul qword ptr [esi + 0c0h] // 00c3dd8a
        faddp st(1), st(0) // 00c3dd90
        fxch st(1) // 00c3dd92
        fcomi st(0), st(1) // 00c3dd94
        fstp st(1) // 00c3dd96
        jb l_00c3df14 // 00c3dd98
        fstp st(0) // 00c3dd9e
        fld qword ptr [esi + 0b0h] // 00c3dda0
        fstp qword ptr [esi + 098h] // 00c3dda6
        fld qword ptr [esi + 0b8h] // 00c3ddac
        fstp qword ptr [esi + 0a0h] // 00c3ddb2
        fld qword ptr [esi + 0c0h] // 00c3ddb8
        fstp qword ptr [esi + 0a8h] // 00c3ddbe
        fld qword ptr [esi + 0e0h] // 00c3ddc4
        fstp qword ptr [esi + 0b0h] // 00c3ddca
        fld qword ptr [esi + 0e8h] // 00c3ddd0
        fstp qword ptr [esi + 0b8h] // 00c3ddd6
        fld qword ptr [esi + 0f0h] // 00c3dddc
        fstp qword ptr [esi + 0c0h] // 00c3dde2
        fld qword ptr [esi + 0110h] // 00c3dde8
        fstp qword ptr [esi + 0f8h] // 00c3ddee
        fld qword ptr [esi + 0118h] // 00c3ddf4
        fstp qword ptr [esi + 0100h] // 00c3ddfa
        fld qword ptr [esi + 0120h] // 00c3de00
        fstp qword ptr [esi + 0108h] // 00c3de06
        fld qword ptr [esi + 0140h] // 00c3de0c
        fstp qword ptr [esi + 0110h] // 00c3de12
        fld qword ptr [esi + 0148h] // 00c3de18
        fstp qword ptr [esi + 0118h] // 00c3de1e
        fld qword ptr [esi + 0150h] // 00c3de24
        fstp qword ptr [esi + 0120h] // 00c3de2a
        fld qword ptr [esi + 0170h] // 00c3de30
        fstp qword ptr [esi + 0158h] // 00c3de36
        fld qword ptr [esi + 0178h] // 00c3de3c
        fstp qword ptr [esi + 0160h] // 00c3de42
        fld qword ptr [esi + 0180h] // 00c3de48
        fstp qword ptr [esi + 0168h] // 00c3de4e
        fld qword ptr [esi + 01a0h] // 00c3de54
        fstp qword ptr [esi + 0170h] // 00c3de5a
        fld qword ptr [esi + 01a8h] // 00c3de60
        fstp qword ptr [esi + 0178h] // 00c3de66
        fld qword ptr [esi + 01b0h] // 00c3de6c
        fstp qword ptr [esi + 0180h] // 00c3de72
        fld qword ptr [esi + 0b0h] // 00c3de78
        fsub qword ptr [esi + 098h] // 00c3de7e
        fld qword ptr [esi + 0b8h] // 00c3de84
        fsub qword ptr [esi + 0a0h] // 00c3de8a
        fld qword ptr [esi + 0c0h] // 00c3de90
        fsub qword ptr [esi + 0a8h] // 00c3de96
        fld qword ptr [esi + 0a0h] // 00c3de9c
        fmul st(0), st(2) // 00c3dea2
        fld st(3) // 00c3dea4
        fmul qword ptr [esi + 098h] // 00c3dea6
        faddp st(1), st(0) // 00c3deac
        fld qword ptr [esi + 0a8h] // 00c3deae
        fmul st(0), st(2) // 00c3deb4
        faddp st(1), st(0) // 00c3deb6
        fld st(2) // 00c3deb8
        fmul st(0), st(3) // 00c3deba
        fld st(4) // 00c3debc
        fmul st(0), st(5) // 00c3debe
        faddp st(1), st(0) // 00c3dec0
        fld st(2) // 00c3dec2
        fmul st(0), st(3) // 00c3dec4
        faddp st(1), st(0) // 00c3dec6
        fdivp st(1), st(0) // 00c3dec8
        fld st(0) // 00c3deca
        fmulp st(4), st(0) // 00c3decc
        fmul st(2), st(0) // 00c3dece
        fmulp st(1), st(0) // 00c3ded0
        fxch st(2) // 00c3ded2
        fsub qword ptr [esi + 098h] // 00c3ded4
        fxch st(1) // 00c3deda
        fsub qword ptr [esi + 0a0h] // 00c3dedc
        fxch st(2) // 00c3dee2
        fsub qword ptr [esi + 0a8h] // 00c3dee4
        fxch st(1) // 00c3deea
        fstp qword ptr [esi + 080h] // 00c3deec
        fxch st(1) // 00c3def2
        fstp qword ptr [esi + 088h] // 00c3def4
        fstp qword ptr [esi + 090h] // 00c3defa
        mov dword ptr [esi + 01b8h], 2 // 00c3df00
        add esp, 064h // 00c3df0a
        pop esi
        ret  // 00c3df0d
    l_00c3df0e:
        fstp st(2) // 00c3df0e
        fstp st(0) // 00c3df10
        fstp st(0) // 00c3df12
    l_00c3df14:
        fld qword ptr [esi + 098h] // 00c3df14
        fsub qword ptr [esi + 0c8h] // 00c3df1a
        fld qword ptr [esi + 0a0h] // 00c3df20
        fsub qword ptr [esi + 0d0h] // 00c3df26
        fld qword ptr [esi + 0a8h] // 00c3df2c
        fsub qword ptr [esi + 0d8h] // 00c3df32
        fld qword ptr [esi + 0e0h] // 00c3df38
        fsub qword ptr [esi + 0c8h] // 00c3df3e
        fld qword ptr [esi + 0e8h] // 00c3df44
        fsub qword ptr [esi + 0d0h] // 00c3df4a
        fld qword ptr [esi + 0f0h] // 00c3df50
        fsub qword ptr [esi + 0d8h] // 00c3df56
        fst qword ptr [esp + 058h] // 00c3df5c
        fld st(1) // 00c3df60
        fmul st(0), st(4) // 00c3df62
        fxch st(1) // 00c3df64
        fmul st(0), st(5) // 00c3df66
        fsubp st(1), st(0) // 00c3df68
        fstp qword ptr [esp] // 00c3df6a
        fld qword ptr [esp + 058h] // 00c3df6d
        fmul st(0), st(5) // 00c3df71
        fxch st(3) // 00c3df73
        fmul st(0), st(2) // 00c3df75
        fsubp st(3), st(0) // 00c3df77
        fxch st(2) // 00c3df79
        fstp qword ptr [esp + 8] // 00c3df7b
        fmulp st(2), st(0) // 00c3df7f
        fmulp st(2), st(0) // 00c3df81
        fsubrp st(1), st(0) // 00c3df83
        fstp qword ptr [esp + 010h] // 00c3df85
        fld qword ptr [esi + 0e0h] // 00c3df89
        fsub qword ptr [esi + 0c8h] // 00c3df8f
        fld qword ptr [esi + 0e8h] // 00c3df95
        fsub qword ptr [esi + 0d0h] // 00c3df9b
        fld qword ptr [esi + 0f0h] // 00c3dfa1
        fsub qword ptr [esi + 0d8h] // 00c3dfa7
        fld qword ptr [esi + 0b0h] // 00c3dfad
        fsub qword ptr [esi + 0c8h] // 00c3dfb3
        fld qword ptr [esi + 0b8h] // 00c3dfb9
        fsub qword ptr [esi + 0d0h] // 00c3dfbf
        fld qword ptr [esi + 0c0h] // 00c3dfc5
        fsub qword ptr [esi + 0d8h] // 00c3dfcb
        fst qword ptr [esp + 058h] // 00c3dfd1
        fld st(1) // 00c3dfd5
        fmul st(0), st(4) // 00c3dfd7
        fxch st(1) // 00c3dfd9
        fmul st(0), st(5) // 00c3dfdb
        fsubp st(1), st(0) // 00c3dfdd
        fld qword ptr [esp + 058h] // 00c3dfdf
        fmul st(0), st(6) // 00c3dfe3
        fxch st(4) // 00c3dfe5
        fmul st(0), st(3) // 00c3dfe7
        fsubp st(4), st(0) // 00c3dfe9
        fxch st(4) // 00c3dfeb
        fmulp st(2), st(0) // 00c3dfed
        fmulp st(4), st(0) // 00c3dfef
        fsubrp st(3), st(0) // 00c3dff1
        fld qword ptr [esi + 0e0h] // 00c3dff3
        fsub qword ptr [esi + 0c8h] // 00c3dff9
        fld qword ptr [esi + 0e8h] // 00c3dfff
        fsub qword ptr [esi + 0d0h] // 00c3e005
        fld qword ptr [esi + 0f0h] // 00c3e00b
        fsub qword ptr [esi + 0d8h] // 00c3e011
        fld qword ptr [esi + 0d0h] // 00c3e017
        fmulp st(2), st(0) // 00c3e01d
        fxch st(2) // 00c3e01f
        fmul qword ptr [esi + 0c8h] // 00c3e021
        faddp st(1), st(0) // 00c3e027
        fld qword ptr [esi + 0d8h] // 00c3e029
        fmulp st(2), st(0) // 00c3e02f
        faddp st(1), st(0) // 00c3e031
        fxch st(4) // 00c3e033
        fcomi st(0), st(4) // 00c3e035
        fstp st(4) // 00c3e037
        jb l_00c3e2f3 // 00c3e039
        fld qword ptr [esi + 0c8h] // 00c3e03f
        fsub qword ptr [esi + 0e0h] // 00c3e045
        fld qword ptr [esi + 0d0h] // 00c3e04b
        fsub qword ptr [esi + 0e8h] // 00c3e051
        fld qword ptr [esi + 0d8h] // 00c3e057
        fsub qword ptr [esi + 0f0h] // 00c3e05d
        fld qword ptr [esi + 0e8h] // 00c3e063
        fmulp st(2), st(0) // 00c3e069
        fld qword ptr [esi + 0e0h] // 00c3e06b
        fmulp st(3), st(0) // 00c3e071
        fxch st(1) // 00c3e073
        faddp st(2), st(0) // 00c3e075
        fmul qword ptr [esi + 0f0h] // 00c3e077
        faddp st(1), st(0) // 00c3e07d
        fxch st(4) // 00c3e07f
        fcomi st(0), st(4) // 00c3e081
        fstp st(4) // 00c3e083
        jb l_00c3e2f3 // 00c3e085
        fld qword ptr [esi + 0e0h] // 00c3e08b
        fsub qword ptr [esi + 0c8h] // 00c3e091
        fld qword ptr [esi + 0e8h] // 00c3e097
        fsub qword ptr [esi + 0d0h] // 00c3e09d
        fst qword ptr [esp + 020h] // 00c3e0a3
        fld qword ptr [esi + 0f0h] // 00c3e0a7
        fsub qword ptr [esi + 0d8h] // 00c3e0ad
        fst qword ptr [esp + 028h] // 00c3e0b3
        fxch st(1) // 00c3e0b7
        fmul qword ptr [esp + 010h] // 00c3e0b9
        fxch st(1) // 00c3e0bd
        fmul qword ptr [esp + 8] // 00c3e0bf
        fsubp st(1), st(0) // 00c3e0c3
        fstp qword ptr [esp + 048h] // 00c3e0c5
        fld qword ptr [esp + 028h] // 00c3e0c9
        fmul qword ptr [esp] // 00c3e0cd
        fld st(1) // 00c3e0d0
        fmul qword ptr [esp + 010h] // 00c3e0d2
        fsubp st(1), st(0) // 00c3e0d6
        fxch st(1) // 00c3e0d8
        fmul qword ptr [esp + 8] // 00c3e0da
        fld qword ptr [esp + 020h] // 00c3e0de
        fmul qword ptr [esp] // 00c3e0e2
        fsubp st(1), st(0) // 00c3e0e5
        fld qword ptr [esi + 0d0h] // 00c3e0e7
        fmulp st(2), st(0) // 00c3e0ed
        fld qword ptr [esi + 0c8h] // 00c3e0ef
        fmul qword ptr [esp + 048h] // 00c3e0f5
        faddp st(2), st(0) // 00c3e0f9
        fmul qword ptr [esi + 0d8h] // 00c3e0fb
        faddp st(1), st(0) // 00c3e101
        fxch st(4) // 00c3e103
        fcomi st(0), st(4) // 00c3e105
        fstp st(4) // 00c3e107
        jb l_00c3e2f3 // 00c3e109
        fld qword ptr [esi + 0e0h] // 00c3e10f
        fsub qword ptr [esi + 0c8h] // 00c3e115
        fld qword ptr [esi + 0e8h] // 00c3e11b
        fsub qword ptr [esi + 0d0h] // 00c3e121
        fld qword ptr [esi + 0f0h] // 00c3e127
        fsub qword ptr [esi + 0d8h] // 00c3e12d
        fst qword ptr [esp + 058h] // 00c3e133
        fmul st(0), st(3) // 00c3e137
        fld st(1) // 00c3e139
        fmul st(0), st(6) // 00c3e13b
        fsubp st(1), st(0) // 00c3e13d
        fld st(2) // 00c3e13f
        fmulp st(6), st(0) // 00c3e141
        fld qword ptr [esp + 058h] // 00c3e143
        fmul st(0), st(5) // 00c3e147
        fsubp st(6), st(0) // 00c3e149
        fxch st(1) // 00c3e14b
        fmulp st(4), st(0) // 00c3e14d
        fxch st(1) // 00c3e14f
        fmulp st(2), st(0) // 00c3e151
        fxch st(2) // 00c3e153
        fsubrp st(1), st(0) // 00c3e155
        fld qword ptr [esi + 0d0h] // 00c3e157
        fmulp st(3), st(0) // 00c3e15d
        fld qword ptr [esi + 0c8h] // 00c3e15f
        fmulp st(2), st(0) // 00c3e165
        fxch st(2) // 00c3e167
        faddp st(1), st(0) // 00c3e169
        fld qword ptr [esi + 0d8h] // 00c3e16b
        fmulp st(2), st(0) // 00c3e171
        faddp st(1), st(0) // 00c3e173
        fxch st(1) // 00c3e175
        fcomi st(0), st(1) // 00c3e177
        fstp st(1) // 00c3e179
        jb l_00c3e2f9 // 00c3e17b
        fstp st(0) // 00c3e181
        fld qword ptr [esi + 0c8h] // 00c3e183
        fstp qword ptr [esi + 098h] // 00c3e189
        fld qword ptr [esi + 0d0h] // 00c3e18f
        fstp qword ptr [esi + 0a0h] // 00c3e195
        fld qword ptr [esi + 0d8h] // 00c3e19b
        fstp qword ptr [esi + 0a8h] // 00c3e1a1
        fld qword ptr [esi + 0e0h] // 00c3e1a7
        fstp qword ptr [esi + 0b0h] // 00c3e1ad
        fld qword ptr [esi + 0e8h] // 00c3e1b3
        fstp qword ptr [esi + 0b8h] // 00c3e1b9
        fld qword ptr [esi + 0f0h] // 00c3e1bf
        fstp qword ptr [esi + 0c0h] // 00c3e1c5
        fld qword ptr [esi + 0128h] // 00c3e1cb
        fstp qword ptr [esi + 0f8h] // 00c3e1d1
        fld qword ptr [esi + 0130h] // 00c3e1d7
        fstp qword ptr [esi + 0100h] // 00c3e1dd
        fld qword ptr [esi + 0138h] // 00c3e1e3
        fstp qword ptr [esi + 0108h] // 00c3e1e9
        fld qword ptr [esi + 0140h] // 00c3e1ef
        fstp qword ptr [esi + 0110h] // 00c3e1f5
        fld qword ptr [esi + 0148h] // 00c3e1fb
        fstp qword ptr [esi + 0118h] // 00c3e201
        fld qword ptr [esi + 0150h] // 00c3e207
        fstp qword ptr [esi + 0120h] // 00c3e20d
        fld qword ptr [esi + 0188h] // 00c3e213
        fstp qword ptr [esi + 0158h] // 00c3e219
        fld qword ptr [esi + 0190h] // 00c3e21f
        fstp qword ptr [esi + 0160h] // 00c3e225
        fld qword ptr [esi + 0198h] // 00c3e22b
        fstp qword ptr [esi + 0168h] // 00c3e231
        fld qword ptr [esi + 01a0h] // 00c3e237
        fstp qword ptr [esi + 0170h] // 00c3e23d
        fld qword ptr [esi + 01a8h] // 00c3e243
        fstp qword ptr [esi + 0178h] // 00c3e249
        fld qword ptr [esi + 01b0h] // 00c3e24f
        fstp qword ptr [esi + 0180h] // 00c3e255
        fld qword ptr [esi + 0b0h] // 00c3e25b
        fsub qword ptr [esi + 098h] // 00c3e261
        fld qword ptr [esi + 0b8h] // 00c3e267
        fsub qword ptr [esi + 0a0h] // 00c3e26d
        fld qword ptr [esi + 0c0h] // 00c3e273
        fsub qword ptr [esi + 0a8h] // 00c3e279
        fld qword ptr [esi + 0a0h] // 00c3e27f
        fmul st(0), st(2) // 00c3e285
        fld qword ptr [esi + 098h] // 00c3e287
        fmul st(0), st(4) // 00c3e28d
        faddp st(1), st(0) // 00c3e28f
        fld qword ptr [esi + 0a8h] // 00c3e291
        fmul st(0), st(2) // 00c3e297
        faddp st(1), st(0) // 00c3e299
        fld st(3) // 00c3e29b
        fmul st(0), st(4) // 00c3e29d
        fld st(3) // 00c3e29f
        fmul st(0), st(4) // 00c3e2a1
        faddp st(1), st(0) // 00c3e2a3
        fld st(2) // 00c3e2a5
        fmul st(0), st(3) // 00c3e2a7
        faddp st(1), st(0) // 00c3e2a9
        fdivp st(1), st(0) // 00c3e2ab
        fld st(0) // 00c3e2ad
        fmulp st(4), st(0) // 00c3e2af
        fld st(0) // 00c3e2b1
        fmulp st(3), st(0) // 00c3e2b3
        fmulp st(1), st(0) // 00c3e2b5
        fxch st(2) // 00c3e2b7
        fsub qword ptr [esi + 098h] // 00c3e2b9
        fxch st(1) // 00c3e2bf
        fsub qword ptr [esi + 0a0h] // 00c3e2c1
        fxch st(2) // 00c3e2c7
        fsub qword ptr [esi + 0a8h] // 00c3e2c9
        fxch st(1) // 00c3e2cf
        fstp qword ptr [esi + 080h] // 00c3e2d1
        fxch st(1) // 00c3e2d7
        fstp qword ptr [esi + 088h] // 00c3e2d9
        fstp qword ptr [esi + 090h] // 00c3e2df
        mov dword ptr [esi + 01b8h], 2 // 00c3e2e5
        add esp, 064h // 00c3e2ef
        pop esi
        ret  // 00c3e2f2
    l_00c3e2f3:
        fstp st(2) // 00c3e2f3
        fstp st(0) // 00c3e2f5
        fstp st(0) // 00c3e2f7
    l_00c3e2f9:
        fld qword ptr [esi + 0e0h] // 00c3e2f9
        fsub qword ptr [esi + 098h] // 00c3e2ff
        fld qword ptr [esi + 0e8h] // 00c3e305
        fsub qword ptr [esi + 0a0h] // 00c3e30b
        fld qword ptr [esi + 0f0h] // 00c3e311
        fsub qword ptr [esi + 0a8h] // 00c3e317
        fld qword ptr [esi + 0b0h] // 00c3e31d
        fsub qword ptr [esi + 098h] // 00c3e323
        fld qword ptr [esi + 0b8h] // 00c3e329
        fsub qword ptr [esi + 0a0h] // 00c3e32f
        fst qword ptr [esp + 050h] // 00c3e335
        fld qword ptr [esi + 0c0h] // 00c3e339
        fsub qword ptr [esi + 0a8h] // 00c3e33f
        fxch st(1) // 00c3e345
        fmul st(0), st(3) // 00c3e347
        fld st(1) // 00c3e349
        fmul st(0), st(5) // 00c3e34b
        fsubp st(1), st(0) // 00c3e34d
        fld st(5) // 00c3e34f
        fmulp st(2), st(0) // 00c3e351
        fld st(2) // 00c3e353
        fmulp st(4), st(0) // 00c3e355
        fxch st(1) // 00c3e357
        fsubrp st(3), st(0) // 00c3e359
        fxch st(1) // 00c3e35b
        fmulp st(3), st(0) // 00c3e35d
        fxch st(3) // 00c3e35f
        fmul qword ptr [esp + 050h] // 00c3e361
        fsubp st(2), st(0) // 00c3e365
        fld qword ptr [esi + 0c8h] // 00c3e367
        fsub qword ptr [esi + 098h] // 00c3e36d
        fld qword ptr [esi + 0d0h] // 00c3e373
        fsub qword ptr [esi + 0a0h] // 00c3e379
        fld qword ptr [esi + 0d8h] // 00c3e37f
        fsub qword ptr [esi + 0a8h] // 00c3e385
        fstp qword ptr [esp + 058h] // 00c3e38b
        fld qword ptr [esi + 0a0h] // 00c3e38f
        fmul st(0), st(3) // 00c3e395
        fld st(5) // 00c3e397
        fmul qword ptr [esi + 098h] // 00c3e399
        faddp st(1), st(0) // 00c3e39f
        fld qword ptr [esi + 0a8h] // 00c3e3a1
        fmul st(0), st(5) // 00c3e3a7
        faddp st(1), st(0) // 00c3e3a9
        fxch st(2) // 00c3e3ab
        fmul st(0), st(5) // 00c3e3ad
        fxch st(1) // 00c3e3af
        fmul st(0), st(3) // 00c3e3b1
        faddp st(1), st(0) // 00c3e3b3
        fld qword ptr [esp + 058h] // 00c3e3b5
        fmul st(0), st(4) // 00c3e3b9
        faddp st(1), st(0) // 00c3e3bb
        fmulp st(1), st(0) // 00c3e3bd
        fxch st(4) // 00c3e3bf
        fcomi st(0), st(4) // 00c3e3c1
        fstp st(4) // 00c3e3c3
        ja l_00c3e7d9 // 00c3e3c5
        fld qword ptr [esi + 0b0h] // 00c3e3cb
        fsub qword ptr [esi + 098h] // 00c3e3d1
        fld qword ptr [esi + 0b8h] // 00c3e3d7
        fsub qword ptr [esi + 0a0h] // 00c3e3dd
        fld qword ptr [esi + 0c0h] // 00c3e3e3
        fsub qword ptr [esi + 0a8h] // 00c3e3e9
        fst qword ptr [esp + 058h] // 00c3e3ef
        fld st(1) // 00c3e3f3
        fmul st(0), st(5) // 00c3e3f5
        fxch st(1) // 00c3e3f7
        fmul st(0), st(4) // 00c3e3f9
        fsubp st(1), st(0) // 00c3e3fb
        fstp qword ptr [esp + 030h] // 00c3e3fd
        fld st(4) // 00c3e401
        fmul qword ptr [esp + 058h] // 00c3e403
        fld st(2) // 00c3e407
        fmul st(0), st(5) // 00c3e409
        fsubp st(1), st(0) // 00c3e40b
        fxch st(2) // 00c3e40d
        fmul st(0), st(3) // 00c3e40f
        fld st(5) // 00c3e411
        fmulp st(2), st(0) // 00c3e413
        fsubrp st(1), st(0) // 00c3e415
        fld qword ptr [esi + 0a0h] // 00c3e417
        fmulp st(2), st(0) // 00c3e41d
        fld qword ptr [esp + 030h] // 00c3e41f
        fmul qword ptr [esi + 098h] // 00c3e423
        faddp st(2), st(0) // 00c3e429
        fmul qword ptr [esi + 0a8h] // 00c3e42b
        faddp st(1), st(0) // 00c3e431
        fxch st(4) // 00c3e433
        fcomi st(0), st(4) // 00c3e435
        fstp st(4) // 00c3e437
        ja l_00c3e7d9 // 00c3e439
        fld qword ptr [esi + 098h] // 00c3e43f
        fsub qword ptr [esi + 0e0h] // 00c3e445
        fld qword ptr [esi + 0a0h] // 00c3e44b
        fsub qword ptr [esi + 0e8h] // 00c3e451
        fld qword ptr [esi + 0a8h] // 00c3e457
        fsub qword ptr [esi + 0f0h] // 00c3e45d
        fst qword ptr [esp + 058h] // 00c3e463
        fld st(1) // 00c3e467
        fmul st(0), st(5) // 00c3e469
        fxch st(1) // 00c3e46b
        fmul st(0), st(4) // 00c3e46d
        fsubp st(1), st(0) // 00c3e46f
        fstp qword ptr [esp + 030h] // 00c3e471
        fld st(4) // 00c3e475
        fmul qword ptr [esp + 058h] // 00c3e477
        fld st(2) // 00c3e47b
        fmulp st(5), st(0) // 00c3e47d
        fsubrp st(4), st(0) // 00c3e47f
        fxch st(1) // 00c3e481
        fmulp st(2), st(0) // 00c3e483
        fmulp st(3), st(0) // 00c3e485
        fsubrp st(2), st(0) // 00c3e487
        fmul qword ptr [esi + 0a0h] // 00c3e489
        fld qword ptr [esi + 098h] // 00c3e48f
        fmul qword ptr [esp + 030h] // 00c3e495
        faddp st(1), st(0) // 00c3e499
        fld qword ptr [esi + 0a8h] // 00c3e49b
        fmulp st(2), st(0) // 00c3e4a1
        faddp st(1), st(0) // 00c3e4a3
        fxch st(1) // 00c3e4a5
        fcomi st(0), st(1) // 00c3e4a7
        fstp st(1) // 00c3e4a9
        ja l_00c3e7df // 00c3e4ab
        fld qword ptr [esi + 098h] // 00c3e4b1
        fsub qword ptr [esi + 0b0h] // 00c3e4b7
        fld qword ptr [esi + 0a0h] // 00c3e4bd
        fsub qword ptr [esi + 0b8h] // 00c3e4c3
        fld qword ptr [esi + 0a8h] // 00c3e4c9
        fsub qword ptr [esi + 0c0h] // 00c3e4cf
        fld qword ptr [esi + 0e0h] // 00c3e4d5
        fsub qword ptr [esi + 0b0h] // 00c3e4db
        fld qword ptr [esi + 0e8h] // 00c3e4e1
        fsub qword ptr [esi + 0b8h] // 00c3e4e7
        fld qword ptr [esi + 0f0h] // 00c3e4ed
        fsub qword ptr [esi + 0c0h] // 00c3e4f3
        fst qword ptr [esp + 058h] // 00c3e4f9
        fld st(1) // 00c3e4fd
        fmul st(0), st(4) // 00c3e4ff
        fxch st(1) // 00c3e501
        fmul st(0), st(5) // 00c3e503
        fsubp st(1), st(0) // 00c3e505
        fstp qword ptr [esp + 030h] // 00c3e507
        fld st(4) // 00c3e50b
        fmul qword ptr [esp + 058h] // 00c3e50d
        fld st(2) // 00c3e511
        fmulp st(4), st(0) // 00c3e513
        fsubrp st(3), st(0) // 00c3e515
        fxch st(1) // 00c3e517
        fmulp st(3), st(0) // 00c3e519
        fmulp st(3), st(0) // 00c3e51b
        fxch st(1) // 00c3e51d
        fsubrp st(2), st(0) // 00c3e51f
        fld qword ptr [esi + 0e0h] // 00c3e521
        fsub qword ptr [esi + 0b0h] // 00c3e527
        fld qword ptr [esi + 0e8h] // 00c3e52d
        fsub qword ptr [esi + 0b8h] // 00c3e533
        fld qword ptr [esi + 0f0h] // 00c3e539
        fsub qword ptr [esi + 0c0h] // 00c3e53f
        fld st(1) // 00c3e545
        fmul st(0), st(5) // 00c3e547
        fld st(1) // 00c3e549
        fmul st(0), st(5) // 00c3e54b
        fsubp st(1), st(0) // 00c3e54d
        fstp qword ptr [esp + 048h] // 00c3e54f
        fld qword ptr [esp + 030h] // 00c3e553
        fld st(0) // 00c3e557
        fmulp st(2), st(0) // 00c3e559
        fld st(3) // 00c3e55b
        fmulp st(6), st(0) // 00c3e55d
        fxch st(1) // 00c3e55f
        fsubrp st(5), st(0) // 00c3e561
        fxch st(2) // 00c3e563
        fmulp st(3), st(0) // 00c3e565
        fmulp st(1), st(0) // 00c3e567
        fsubp st(1), st(0) // 00c3e569
        fld qword ptr [esi + 0b8h] // 00c3e56b
        fmulp st(2), st(0) // 00c3e571
        fld qword ptr [esp + 048h] // 00c3e573
        fmul qword ptr [esi + 0b0h] // 00c3e577
        faddp st(2), st(0) // 00c3e57d
        fmul qword ptr [esi + 0c0h] // 00c3e57f
        faddp st(1), st(0) // 00c3e585
        fxch st(1) // 00c3e587
        fcomi st(0), st(1) // 00c3e589
        fstp st(1) // 00c3e58b
        ja l_00c3e7df // 00c3e58d
        fstp st(0) // 00c3e593
        fld qword ptr [esi + 098h] // 00c3e595
        fstp qword ptr [esi + 098h] // 00c3e59b
        fld qword ptr [esi + 0a0h] // 00c3e5a1
        fstp qword ptr [esi + 0a0h] // 00c3e5a7
        fld qword ptr [esi + 0a8h] // 00c3e5ad
        fstp qword ptr [esi + 0a8h] // 00c3e5b3
        fld qword ptr [esi + 0b0h] // 00c3e5b9
        fstp qword ptr [esi + 0b0h] // 00c3e5bf
        fld qword ptr [esi + 0b8h] // 00c3e5c5
        fstp qword ptr [esi + 0b8h] // 00c3e5cb
        fld qword ptr [esi + 0c0h] // 00c3e5d1
        fstp qword ptr [esi + 0c0h] // 00c3e5d7
        fld qword ptr [esi + 0e0h] // 00c3e5dd
        fstp qword ptr [esi + 0c8h] // 00c3e5e3
        fld qword ptr [esi + 0e8h] // 00c3e5e9
        fstp qword ptr [esi + 0d0h] // 00c3e5ef
        fld qword ptr [esi + 0f0h] // 00c3e5f5
        fstp qword ptr [esi + 0d8h] // 00c3e5fb
        fld qword ptr [esi + 0f8h] // 00c3e601
        fstp qword ptr [esi + 0f8h] // 00c3e607
        fld qword ptr [esi + 0100h] // 00c3e60d
        fstp qword ptr [esi + 0100h] // 00c3e613
        fld qword ptr [esi + 0108h] // 00c3e619
        fstp qword ptr [esi + 0108h] // 00c3e61f
        fld qword ptr [esi + 0110h] // 00c3e625
        fstp qword ptr [esi + 0110h] // 00c3e62b
        fld qword ptr [esi + 0118h] // 00c3e631
        fstp qword ptr [esi + 0118h] // 00c3e637
        fld qword ptr [esi + 0120h] // 00c3e63d
        fstp qword ptr [esi + 0120h] // 00c3e643
        fld qword ptr [esi + 0140h] // 00c3e649
        fstp qword ptr [esi + 0128h] // 00c3e64f
        fld qword ptr [esi + 0148h] // 00c3e655
        fstp qword ptr [esi + 0130h] // 00c3e65b
        fld qword ptr [esi + 0150h] // 00c3e661
        fstp qword ptr [esi + 0138h] // 00c3e667
        fld qword ptr [esi + 0158h] // 00c3e66d
        fstp qword ptr [esi + 0158h] // 00c3e673
        fld qword ptr [esi + 0160h] // 00c3e679
        fstp qword ptr [esi + 0160h] // 00c3e67f
        fld qword ptr [esi + 0168h] // 00c3e685
        fstp qword ptr [esi + 0168h] // 00c3e68b
        fld qword ptr [esi + 0170h] // 00c3e691
        fstp qword ptr [esi + 0170h] // 00c3e697
        fld qword ptr [esi + 0178h] // 00c3e69d
        fstp qword ptr [esi + 0178h] // 00c3e6a3
        fld qword ptr [esi + 0180h] // 00c3e6a9
        fstp qword ptr [esi + 0180h] // 00c3e6af
        fld qword ptr [esi + 01a0h] // 00c3e6b5
        fstp qword ptr [esi + 0188h] // 00c3e6bb
        fld qword ptr [esi + 01a8h] // 00c3e6c1
        fstp qword ptr [esi + 0190h] // 00c3e6c7
        fld qword ptr [esi + 01b0h] // 00c3e6cd
        fstp qword ptr [esi + 0198h] // 00c3e6d3
        fld qword ptr [esi + 0b0h] // 00c3e6d9
        fsub qword ptr [esi + 098h] // 00c3e6df
        fld qword ptr [esi + 0b8h] // 00c3e6e5
        fsub qword ptr [esi + 0a0h] // 00c3e6eb
        fld qword ptr [esi + 0c0h] // 00c3e6f1
        fsub qword ptr [esi + 0a8h] // 00c3e6f7
        fld qword ptr [esi + 0c8h] // 00c3e6fd
        fsub qword ptr [esi + 098h] // 00c3e703
        fld qword ptr [esi + 0d0h] // 00c3e709
        fsub qword ptr [esi + 0a0h] // 00c3e70f
        fld qword ptr [esi + 0d8h] // 00c3e715
        fsub qword ptr [esi + 0a8h] // 00c3e71b
        fld st(1) // 00c3e721
        fmul st(0), st(4) // 00c3e723
        fld st(1) // 00c3e725
        fmul st(0), st(6) // 00c3e727
        fsubp st(1), st(0) // 00c3e729
        fld st(6) // 00c3e72b
        fmulp st(2), st(0) // 00c3e72d
        fld st(3) // 00c3e72f
        fmulp st(5), st(0) // 00c3e731
        fxch st(1) // 00c3e733
        fsubrp st(4), st(0) // 00c3e735
        fxch st(2) // 00c3e737
        fmulp st(4), st(0) // 00c3e739
        fmulp st(4), st(0) // 00c3e73b
        fxch st(2) // 00c3e73d
        fsubrp st(3), st(0) // 00c3e73f
        fxch st(1) // 00c3e741
        fstp qword ptr [esi + 080h] // 00c3e743
        fstp qword ptr [esi + 088h] // 00c3e749
        fstp qword ptr [esi + 090h] // 00c3e74f
        fld qword ptr [esi + 088h] // 00c3e755
        fld qword ptr [esi + 080h] // 00c3e75b
        fld qword ptr [esi + 090h] // 00c3e761
        fld qword ptr [esi + 088h] // 00c3e767
        fmul qword ptr [esi + 0a0h] // 00c3e76d
        fld qword ptr [esi + 098h] // 00c3e773
        fmul qword ptr [esi + 080h] // 00c3e779
        faddp st(1), st(0) // 00c3e77f
        fld qword ptr [esi + 090h] // 00c3e781
        fmul qword ptr [esi + 0a8h] // 00c3e787
        faddp st(1), st(0) // 00c3e78d
        fld st(2) // 00c3e78f
        fmulp st(3), st(0) // 00c3e791
        fld st(3) // 00c3e793
        fmulp st(4), st(0) // 00c3e795
        fxch st(2) // 00c3e797
        faddp st(3), st(0) // 00c3e799
        fmul st(0), st(0) // 00c3e79b
        faddp st(2), st(0) // 00c3e79d
        fdivrp st(1), st(0) // 00c3e79f
        fchs  // 00c3e7a1
        fld qword ptr [esi + 080h] // 00c3e7a3
        fmul st(0), st(1) // 00c3e7a9
        fstp qword ptr [esi + 080h] // 00c3e7ab
        fld st(0) // 00c3e7b1
        fmul qword ptr [esi + 088h] // 00c3e7b3
        fstp qword ptr [esi + 088h] // 00c3e7b9
        fmul qword ptr [esi + 090h] // 00c3e7bf
        fstp qword ptr [esi + 090h] // 00c3e7c5
        mov dword ptr [esi + 01b8h], 3 // 00c3e7cb
        add esp, 064h // 00c3e7d5
        pop esi
        ret  // 00c3e7d8
    l_00c3e7d9:
        fstp st(2) // 00c3e7d9
        fstp st(1) // 00c3e7db
        fstp st(0) // 00c3e7dd
    l_00c3e7df:
        fld qword ptr [esi + 0e0h] // 00c3e7df
        fsub qword ptr [esi + 098h] // 00c3e7e5
        fld qword ptr [esi + 0e8h] // 00c3e7eb
        fsub qword ptr [esi + 0a0h] // 00c3e7f1
        fld qword ptr [esi + 0f0h] // 00c3e7f7
        fsub qword ptr [esi + 0a8h] // 00c3e7fd
        fld qword ptr [esi + 0c8h] // 00c3e803
        fsub qword ptr [esi + 098h] // 00c3e809
        fld qword ptr [esi + 0d0h] // 00c3e80f
        fsub qword ptr [esi + 0a0h] // 00c3e815
        fst qword ptr [esp + 050h] // 00c3e81b
        fld qword ptr [esi + 0d8h] // 00c3e81f
        fsub qword ptr [esi + 0a8h] // 00c3e825
        fxch st(1) // 00c3e82b
        fmul st(0), st(3) // 00c3e82d
        fld st(1) // 00c3e82f
        fmul st(0), st(5) // 00c3e831
        fsubp st(1), st(0) // 00c3e833
        fld st(5) // 00c3e835
        fmulp st(2), st(0) // 00c3e837
        fld st(2) // 00c3e839
        fmulp st(4), st(0) // 00c3e83b
        fxch st(1) // 00c3e83d
        fsubrp st(3), st(0) // 00c3e83f
        fxch st(1) // 00c3e841
        fmulp st(3), st(0) // 00c3e843
        fxch st(3) // 00c3e845
        fmul qword ptr [esp + 050h] // 00c3e847
        fsubp st(2), st(0) // 00c3e84b
        fld qword ptr [esi + 0b0h] // 00c3e84d
        fsub qword ptr [esi + 098h] // 00c3e853
        fld qword ptr [esi + 0b8h] // 00c3e859
        fsub qword ptr [esi + 0a0h] // 00c3e85f
        fld qword ptr [esi + 0c0h] // 00c3e865
        fsub qword ptr [esi + 0a8h] // 00c3e86b
        fstp qword ptr [esp + 058h] // 00c3e871
        fld qword ptr [esi + 0a0h] // 00c3e875
        fmul st(0), st(3) // 00c3e87b
        fld st(5) // 00c3e87d
        fmul qword ptr [esi + 098h] // 00c3e87f
        faddp st(1), st(0) // 00c3e885
        fld qword ptr [esi + 0a8h] // 00c3e887
        fmul st(0), st(5) // 00c3e88d
        faddp st(1), st(0) // 00c3e88f
        fxch st(2) // 00c3e891
        fmul st(0), st(5) // 00c3e893
        fxch st(1) // 00c3e895
        fmul st(0), st(3) // 00c3e897
        faddp st(1), st(0) // 00c3e899
        fld qword ptr [esp + 058h] // 00c3e89b
        fmul st(0), st(4) // 00c3e89f
        faddp st(1), st(0) // 00c3e8a1
        fmulp st(1), st(0) // 00c3e8a3
        fxch st(4) // 00c3e8a5
        fcomi st(0), st(4) // 00c3e8a7
        fstp st(4) // 00c3e8a9
        ja l_00c3ecbf // 00c3e8ab
        fld qword ptr [esi + 0c8h] // 00c3e8b1
        fsub qword ptr [esi + 098h] // 00c3e8b7
        fld qword ptr [esi + 0d0h] // 00c3e8bd
        fsub qword ptr [esi + 0a0h] // 00c3e8c3
        fld qword ptr [esi + 0d8h] // 00c3e8c9
        fsub qword ptr [esi + 0a8h] // 00c3e8cf
        fst qword ptr [esp + 058h] // 00c3e8d5
        fld st(1) // 00c3e8d9
        fmul st(0), st(5) // 00c3e8db
        fxch st(1) // 00c3e8dd
        fmul st(0), st(4) // 00c3e8df
        fsubp st(1), st(0) // 00c3e8e1
        fstp qword ptr [esp + 030h] // 00c3e8e3
        fld st(4) // 00c3e8e7
        fmul qword ptr [esp + 058h] // 00c3e8e9
        fld st(2) // 00c3e8ed
        fmul st(0), st(5) // 00c3e8ef
        fsubp st(1), st(0) // 00c3e8f1
        fxch st(2) // 00c3e8f3
        fmul st(0), st(3) // 00c3e8f5
        fld st(5) // 00c3e8f7
        fmulp st(2), st(0) // 00c3e8f9
        fsubrp st(1), st(0) // 00c3e8fb
        fld qword ptr [esi + 0a0h] // 00c3e8fd
        fmulp st(2), st(0) // 00c3e903
        fld qword ptr [esp + 030h] // 00c3e905
        fmul qword ptr [esi + 098h] // 00c3e909
        faddp st(2), st(0) // 00c3e90f
        fmul qword ptr [esi + 0a8h] // 00c3e911
        faddp st(1), st(0) // 00c3e917
        fxch st(4) // 00c3e919
        fcomi st(0), st(4) // 00c3e91b
        fstp st(4) // 00c3e91d
        ja l_00c3ecbf // 00c3e91f
        fld qword ptr [esi + 098h] // 00c3e925
        fsub qword ptr [esi + 0e0h] // 00c3e92b
        fld qword ptr [esi + 0a0h] // 00c3e931
        fsub qword ptr [esi + 0e8h] // 00c3e937
        fld qword ptr [esi + 0a8h] // 00c3e93d
        fsub qword ptr [esi + 0f0h] // 00c3e943
        fst qword ptr [esp + 058h] // 00c3e949
        fld st(1) // 00c3e94d
        fmul st(0), st(5) // 00c3e94f
        fxch st(1) // 00c3e951
        fmul st(0), st(4) // 00c3e953
        fsubp st(1), st(0) // 00c3e955
        fstp qword ptr [esp + 030h] // 00c3e957
        fld st(4) // 00c3e95b
        fmul qword ptr [esp + 058h] // 00c3e95d
        fld st(2) // 00c3e961
        fmulp st(5), st(0) // 00c3e963
        fsubrp st(4), st(0) // 00c3e965
        fxch st(1) // 00c3e967
        fmulp st(2), st(0) // 00c3e969
        fmulp st(3), st(0) // 00c3e96b
        fsubrp st(2), st(0) // 00c3e96d
        fmul qword ptr [esi + 0a0h] // 00c3e96f
        fld qword ptr [esp + 030h] // 00c3e975
        fmul qword ptr [esi + 098h] // 00c3e979
        faddp st(1), st(0) // 00c3e97f
        fld qword ptr [esi + 0a8h] // 00c3e981
        fmulp st(2), st(0) // 00c3e987
        faddp st(1), st(0) // 00c3e989
        fxch st(1) // 00c3e98b
        fcomi st(0), st(1) // 00c3e98d
        fstp st(1) // 00c3e98f
        ja l_00c3ecc5 // 00c3e991
        fld qword ptr [esi + 098h] // 00c3e997
        fsub qword ptr [esi + 0c8h] // 00c3e99d
        fld qword ptr [esi + 0a0h] // 00c3e9a3
        fsub qword ptr [esi + 0d0h] // 00c3e9a9
        fld qword ptr [esi + 0a8h] // 00c3e9af
        fsub qword ptr [esi + 0d8h] // 00c3e9b5
        fld qword ptr [esi + 0e0h] // 00c3e9bb
        fsub qword ptr [esi + 0c8h] // 00c3e9c1
        fld qword ptr [esi + 0e8h] // 00c3e9c7
        fsub qword ptr [esi + 0d0h] // 00c3e9cd
        fld qword ptr [esi + 0f0h] // 00c3e9d3
        fsub qword ptr [esi + 0d8h] // 00c3e9d9
        fst qword ptr [esp + 058h] // 00c3e9df
        fld st(1) // 00c3e9e3
        fmul st(0), st(4) // 00c3e9e5
        fxch st(1) // 00c3e9e7
        fmul st(0), st(5) // 00c3e9e9
        fsubp st(1), st(0) // 00c3e9eb
        fstp qword ptr [esp + 030h] // 00c3e9ed
        fld st(4) // 00c3e9f1
        fmul qword ptr [esp + 058h] // 00c3e9f3
        fld st(2) // 00c3e9f7
        fmulp st(4), st(0) // 00c3e9f9
        fsubrp st(3), st(0) // 00c3e9fb
        fxch st(1) // 00c3e9fd
        fmulp st(3), st(0) // 00c3e9ff
        fmulp st(3), st(0) // 00c3ea01
        fxch st(1) // 00c3ea03
        fsubrp st(2), st(0) // 00c3ea05
        fld qword ptr [esi + 0e0h] // 00c3ea07
        fsub qword ptr [esi + 0c8h] // 00c3ea0d
        fld qword ptr [esi + 0e8h] // 00c3ea13
        fsub qword ptr [esi + 0d0h] // 00c3ea19
        fld qword ptr [esi + 0f0h] // 00c3ea1f
        fsub qword ptr [esi + 0d8h] // 00c3ea25
        fld st(1) // 00c3ea2b
        fmul st(0), st(5) // 00c3ea2d
        fld st(1) // 00c3ea2f
        fmul st(0), st(5) // 00c3ea31
        fsubp st(1), st(0) // 00c3ea33
        fstp qword ptr [esp + 048h] // 00c3ea35
        fld qword ptr [esp + 030h] // 00c3ea39
        fld st(0) // 00c3ea3d
        fmulp st(2), st(0) // 00c3ea3f
        fld st(3) // 00c3ea41
        fmulp st(6), st(0) // 00c3ea43
        fxch st(1) // 00c3ea45
        fsubrp st(5), st(0) // 00c3ea47
        fxch st(2) // 00c3ea49
        fmulp st(3), st(0) // 00c3ea4b
        fmulp st(1), st(0) // 00c3ea4d
        fsubp st(1), st(0) // 00c3ea4f
        fld qword ptr [esi + 0d0h] // 00c3ea51
        fmulp st(2), st(0) // 00c3ea57
        fld qword ptr [esp + 048h] // 00c3ea59
        fmul qword ptr [esi + 0c8h] // 00c3ea5d
        faddp st(2), st(0) // 00c3ea63
        fmul qword ptr [esi + 0d8h] // 00c3ea65
        faddp st(1), st(0) // 00c3ea6b
        fxch st(1) // 00c3ea6d
        fcomi st(0), st(1) // 00c3ea6f
        fstp st(1) // 00c3ea71
        ja l_00c3ecc5 // 00c3ea73
        fstp st(0) // 00c3ea79
        fld qword ptr [esi + 098h] // 00c3ea7b
        fstp qword ptr [esi + 098h] // 00c3ea81
        fld qword ptr [esi + 0a0h] // 00c3ea87
        fstp qword ptr [esi + 0a0h] // 00c3ea8d
        fld qword ptr [esi + 0a8h] // 00c3ea93
        fstp qword ptr [esi + 0a8h] // 00c3ea99
        fld qword ptr [esi + 0c8h] // 00c3ea9f
        fstp qword ptr [esi + 0b0h] // 00c3eaa5
        fld qword ptr [esi + 0d0h] // 00c3eaab
        fstp qword ptr [esi + 0b8h] // 00c3eab1
        fld qword ptr [esi + 0d8h] // 00c3eab7
        fstp qword ptr [esi + 0c0h] // 00c3eabd
        fld qword ptr [esi + 0e0h] // 00c3eac3
        fstp qword ptr [esi + 0c8h] // 00c3eac9
        fld qword ptr [esi + 0e8h] // 00c3eacf
        fstp qword ptr [esi + 0d0h] // 00c3ead5
        fld qword ptr [esi + 0f0h] // 00c3eadb
        fstp qword ptr [esi + 0d8h] // 00c3eae1
        fld qword ptr [esi + 0f8h] // 00c3eae7
        fstp qword ptr [esi + 0f8h] // 00c3eaed
        fld qword ptr [esi + 0100h] // 00c3eaf3
        fstp qword ptr [esi + 0100h] // 00c3eaf9
        fld qword ptr [esi + 0108h] // 00c3eaff
        fstp qword ptr [esi + 0108h] // 00c3eb05
        fld qword ptr [esi + 0128h] // 00c3eb0b
        fstp qword ptr [esi + 0110h] // 00c3eb11
        fld qword ptr [esi + 0130h] // 00c3eb17
        fstp qword ptr [esi + 0118h] // 00c3eb1d
        fld qword ptr [esi + 0138h] // 00c3eb23
        fstp qword ptr [esi + 0120h] // 00c3eb29
        fld qword ptr [esi + 0140h] // 00c3eb2f
        fstp qword ptr [esi + 0128h] // 00c3eb35
        fld qword ptr [esi + 0148h] // 00c3eb3b
        fstp qword ptr [esi + 0130h] // 00c3eb41
        fld qword ptr [esi + 0150h] // 00c3eb47
        fstp qword ptr [esi + 0138h] // 00c3eb4d
        fld qword ptr [esi + 0158h] // 00c3eb53
        fstp qword ptr [esi + 0158h] // 00c3eb59
        fld qword ptr [esi + 0160h] // 00c3eb5f
        fstp qword ptr [esi + 0160h] // 00c3eb65
        fld qword ptr [esi + 0168h] // 00c3eb6b
        fstp qword ptr [esi + 0168h] // 00c3eb71
        fld qword ptr [esi + 0188h] // 00c3eb77
        fstp qword ptr [esi + 0170h] // 00c3eb7d
        fld qword ptr [esi + 0190h] // 00c3eb83
        fstp qword ptr [esi + 0178h] // 00c3eb89
        fld qword ptr [esi + 0198h] // 00c3eb8f
        fstp qword ptr [esi + 0180h] // 00c3eb95
        fld qword ptr [esi + 01a0h] // 00c3eb9b
        fstp qword ptr [esi + 0188h] // 00c3eba1
        fld qword ptr [esi + 01a8h] // 00c3eba7
        fstp qword ptr [esi + 0190h] // 00c3ebad
        fld qword ptr [esi + 01b0h] // 00c3ebb3
        fstp qword ptr [esi + 0198h] // 00c3ebb9
        fld qword ptr [esi + 0b0h] // 00c3ebbf
        fsub qword ptr [esi + 098h] // 00c3ebc5
        fld qword ptr [esi + 0b8h] // 00c3ebcb
        fsub qword ptr [esi + 0a0h] // 00c3ebd1
        fld qword ptr [esi + 0c0h] // 00c3ebd7
        fsub qword ptr [esi + 0a8h] // 00c3ebdd
        fld qword ptr [esi + 0c8h] // 00c3ebe3
        fsub qword ptr [esi + 098h] // 00c3ebe9
        fld qword ptr [esi + 0d0h] // 00c3ebef
        fsub qword ptr [esi + 0a0h] // 00c3ebf5
        fld qword ptr [esi + 0d8h] // 00c3ebfb
        fsub qword ptr [esi + 0a8h] // 00c3ec01
        fld st(1) // 00c3ec07
        fmul st(0), st(4) // 00c3ec09
        fld st(1) // 00c3ec0b
        fmul st(0), st(6) // 00c3ec0d
        fsubp st(1), st(0) // 00c3ec0f
        fld st(6) // 00c3ec11
        fmulp st(2), st(0) // 00c3ec13
        fld st(3) // 00c3ec15
        fmulp st(5), st(0) // 00c3ec17
        fxch st(1) // 00c3ec19
        fsubrp st(4), st(0) // 00c3ec1b
        fxch st(2) // 00c3ec1d
        fmulp st(4), st(0) // 00c3ec1f
        fmulp st(4), st(0) // 00c3ec21
        fxch st(2) // 00c3ec23
        fsubrp st(3), st(0) // 00c3ec25
        fxch st(1) // 00c3ec27
        fstp qword ptr [esi + 080h] // 00c3ec29
        fstp qword ptr [esi + 088h] // 00c3ec2f
        fstp qword ptr [esi + 090h] // 00c3ec35
        fld qword ptr [esi + 088h] // 00c3ec3b
        fld qword ptr [esi + 080h] // 00c3ec41
        fld qword ptr [esi + 090h] // 00c3ec47
        fld qword ptr [esi + 088h] // 00c3ec4d
        fmul qword ptr [esi + 0a0h] // 00c3ec53
        fld qword ptr [esi + 098h] // 00c3ec59
        fmul qword ptr [esi + 080h] // 00c3ec5f
        faddp st(1), st(0) // 00c3ec65
        fld qword ptr [esi + 090h] // 00c3ec67
        fmul qword ptr [esi + 0a8h] // 00c3ec6d
        faddp st(1), st(0) // 00c3ec73
        fld st(2) // 00c3ec75
        fmulp st(3), st(0) // 00c3ec77
        fld st(3) // 00c3ec79
        fmulp st(4), st(0) // 00c3ec7b
        fxch st(2) // 00c3ec7d
        faddp st(3), st(0) // 00c3ec7f
        fmul st(0), st(0) // 00c3ec81
        faddp st(2), st(0) // 00c3ec83
        fdivrp st(1), st(0) // 00c3ec85
        fchs  // 00c3ec87
        fld st(0) // 00c3ec89
        fmul qword ptr [esi + 080h] // 00c3ec8b
        fstp qword ptr [esi + 080h] // 00c3ec91
        fld st(0) // 00c3ec97
        fmul qword ptr [esi + 088h] // 00c3ec99
        fstp qword ptr [esi + 088h] // 00c3ec9f
        fmul qword ptr [esi + 090h] // 00c3eca5
        fstp qword ptr [esi + 090h] // 00c3ecab
        mov dword ptr [esi + 01b8h], 3 // 00c3ecb1
        add esp, 064h // 00c3ecbb
        pop esi
        ret  // 00c3ecbe
    l_00c3ecbf:
        fstp st(2) // 00c3ecbf
        fstp st(1) // 00c3ecc1
        fstp st(0) // 00c3ecc3
    l_00c3ecc5:
        fld qword ptr [esi + 0e0h] // 00c3ecc5
        fsub qword ptr [esi + 0b0h] // 00c3eccb
        fld qword ptr [esi + 0e8h] // 00c3ecd1
        fsub qword ptr [esi + 0b8h] // 00c3ecd7
        fld qword ptr [esi + 0f0h] // 00c3ecdd
        fsub qword ptr [esi + 0c0h] // 00c3ece3
        fld qword ptr [esi + 0c8h] // 00c3ece9
        fsub qword ptr [esi + 0b0h] // 00c3ecef
        fld qword ptr [esi + 0d0h] // 00c3ecf5
        fsub qword ptr [esi + 0b8h] // 00c3ecfb
        fld qword ptr [esi + 0d8h] // 00c3ed01
        fsub qword ptr [esi + 0c0h] // 00c3ed07
        fst qword ptr [esp + 058h] // 00c3ed0d
        fld st(1) // 00c3ed11
        fmul st(0), st(4) // 00c3ed13
        fxch st(1) // 00c3ed15
        fmul st(0), st(5) // 00c3ed17
        fsubp st(1), st(0) // 00c3ed19
        fld qword ptr [esp + 058h] // 00c3ed1b
        fmul st(0), st(6) // 00c3ed1f
        fxch st(4) // 00c3ed21
        fmul st(0), st(3) // 00c3ed23
        fsubp st(4), st(0) // 00c3ed25
        fxch st(4) // 00c3ed27
        fmulp st(2), st(0) // 00c3ed29
        fmulp st(4), st(0) // 00c3ed2b
        fsubrp st(3), st(0) // 00c3ed2d
        fld qword ptr [esi + 098h] // 00c3ed2f
        fsub qword ptr [esi + 0b0h] // 00c3ed35
        fld qword ptr [esi + 0a0h] // 00c3ed3b
        fsub qword ptr [esi + 0b8h] // 00c3ed41
        fld qword ptr [esi + 0a8h] // 00c3ed47
        fsub qword ptr [esi + 0c0h] // 00c3ed4d
        fstp qword ptr [esp + 058h] // 00c3ed53
        fld qword ptr [esi + 0b8h] // 00c3ed57
        fmul st(0), st(3) // 00c3ed5d
        fld qword ptr [esi + 0b0h] // 00c3ed5f
        fmul st(0), st(5) // 00c3ed65
        faddp st(1), st(0) // 00c3ed67
        fld qword ptr [esi + 0c0h] // 00c3ed69
        fmul st(0), st(6) // 00c3ed6f
        faddp st(1), st(0) // 00c3ed71
        fxch st(2) // 00c3ed73
        fmul st(0), st(4) // 00c3ed75
        fxch st(1) // 00c3ed77
        fmul st(0), st(3) // 00c3ed79
        faddp st(1), st(0) // 00c3ed7b
        fld qword ptr [esp + 058h] // 00c3ed7d
        fmul st(0), st(5) // 00c3ed81
        faddp st(1), st(0) // 00c3ed83
        fmulp st(1), st(0) // 00c3ed85
        fxch st(4) // 00c3ed87
        fcomi st(0), st(4) // 00c3ed89
        fstp st(4) // 00c3ed8b
        ja l_00c3f169 // 00c3ed8d
        fld qword ptr [esi + 0c8h] // 00c3ed93
        fsub qword ptr [esi + 0b0h] // 00c3ed99
        fld qword ptr [esi + 0d0h] // 00c3ed9f
        fsub qword ptr [esi + 0b8h] // 00c3eda5
        fld qword ptr [esi + 0d8h] // 00c3edab
        fsub qword ptr [esi + 0c0h] // 00c3edb1
        fst qword ptr [esp + 058h] // 00c3edb7
        fld st(1) // 00c3edbb
        fmul st(0), st(6) // 00c3edbd
        fxch st(1) // 00c3edbf
        fmul st(0), st(4) // 00c3edc1
        fsubp st(1), st(0) // 00c3edc3
        fstp qword ptr [esp + 030h] // 00c3edc5
        fld qword ptr [esp + 058h] // 00c3edc9
        fmul st(0), st(4) // 00c3edcd
        fld st(5) // 00c3edcf
        fmul st(0), st(3) // 00c3edd1
        fsubp st(1), st(0) // 00c3edd3
        fld st(3) // 00c3edd5
        fmulp st(3), st(0) // 00c3edd7
        fxch st(1) // 00c3edd9
        fmul st(0), st(4) // 00c3eddb
        fsubp st(2), st(0) // 00c3eddd
        fmul qword ptr [esi + 0b8h] // 00c3eddf
        fld qword ptr [esi + 0b0h] // 00c3ede5
        fmul qword ptr [esp + 030h] // 00c3edeb
        faddp st(1), st(0) // 00c3edef
        fld qword ptr [esi + 0c0h] // 00c3edf1
        fmulp st(2), st(0) // 00c3edf7
        faddp st(1), st(0) // 00c3edf9
        fxch st(4) // 00c3edfb
        fcomi st(0), st(4) // 00c3edfd
        fstp st(4) // 00c3edff
        ja l_00c3f169 // 00c3ee01
        fld qword ptr [esi + 0b0h] // 00c3ee07
        fsub qword ptr [esi + 0e0h] // 00c3ee0d
        fld qword ptr [esi + 0b8h] // 00c3ee13
        fsub qword ptr [esi + 0e8h] // 00c3ee19
        fld qword ptr [esi + 0c0h] // 00c3ee1f
        fsub qword ptr [esi + 0f0h] // 00c3ee25
        fst qword ptr [esp + 058h] // 00c3ee2b
        fld st(1) // 00c3ee2f
        fmul st(0), st(6) // 00c3ee31
        fxch st(1) // 00c3ee33
        fmul st(0), st(4) // 00c3ee35
        fsubp st(1), st(0) // 00c3ee37
        fld qword ptr [esp + 058h] // 00c3ee39
        fmul st(0), st(5) // 00c3ee3d
        fxch st(6) // 00c3ee3f
        fmul st(0), st(3) // 00c3ee41
        fsubp st(6), st(0) // 00c3ee43
        fxch st(3) // 00c3ee45
        fmulp st(2), st(0) // 00c3ee47
        fmulp st(3), st(0) // 00c3ee49
        fsubrp st(2), st(0) // 00c3ee4b
        fld qword ptr [esi + 0b8h] // 00c3ee4d
        fmulp st(3), st(0) // 00c3ee53
        fmul qword ptr [esi + 0b0h] // 00c3ee55
        faddp st(2), st(0) // 00c3ee5b
        fmul qword ptr [esi + 0c0h] // 00c3ee5d
        faddp st(1), st(0) // 00c3ee63
        fxch st(1) // 00c3ee65
        fcomip st(0), st(1) // 00c3ee67
        fstp st(0) // 00c3ee69
        ja l_00c3f171 // 00c3ee6b
        fld qword ptr [esi + 0b0h] // 00c3ee71
        lea ecx, [esp + 018h] // 00c3ee77
        fsub qword ptr [esi + 0c8h] // 00c3ee7b
        lea edx, [esp] // 00c3ee81
        lea eax, [esp + 048h] // 00c3ee84
        fstp qword ptr [esp + 018h] // 00c3ee88
        fld qword ptr [esi + 0b8h] // 00c3ee8c
        fsub qword ptr [esi + 0d0h] // 00c3ee92
        fstp qword ptr [esp + 020h] // 00c3ee98
        fld qword ptr [esi + 0c0h] // 00c3ee9c
        fsub qword ptr [esi + 0d8h] // 00c3eea2
        fstp qword ptr [esp + 028h] // 00c3eea8
        fld qword ptr [esi + 0e0h] // 00c3eeac
        fsub qword ptr [esi + 0c8h] // 00c3eeb2
        fstp qword ptr [esp] // 00c3eeb8
        fld qword ptr [esi + 0e8h] // 00c3eebb
        fsub qword ptr [esi + 0d0h] // 00c3eec1
        fstp qword ptr [esp + 8] // 00c3eec7
        fld qword ptr [esi + 0f0h] // 00c3eecb
        fsub qword ptr [esi + 0d8h] // 00c3eed1
        fstp qword ptr [esp + 010h] // 00c3eed7
        call cross_reference // 00c3eedb
        fld qword ptr [esi + 0e0h] // 00c3eee0
        lea ecx, [esp + 048h] // 00c3eee6
        fsub qword ptr [esi + 0c8h] // 00c3eeea
        lea edx, [esp + 018h] // 00c3eef0
        lea eax, [esp + 030h] // 00c3eef4
        fstp qword ptr [esp + 018h] // 00c3eef8
        fld qword ptr [esi + 0e8h] // 00c3eefc
        fsub qword ptr [esi + 0d0h] // 00c3ef02
        fstp qword ptr [esp + 020h] // 00c3ef08
        fld qword ptr [esi + 0f0h] // 00c3ef0c
        fsub qword ptr [esi + 0d8h] // 00c3ef12
        fstp qword ptr [esp + 028h] // 00c3ef18
        call cross_reference // 00c3ef1c
        fld qword ptr [esi + 0d0h] // 00c3ef21
        fmul qword ptr [eax + 8] // 00c3ef27
        fld qword ptr [eax] // 00c3ef2a
        fmul qword ptr [esi + 0c8h] // 00c3ef2c
        faddp st(1), st(0) // 00c3ef32
        fld qword ptr [esi + 0d8h] // 00c3ef34
        fmul qword ptr [eax + 010h] // 00c3ef3a
        faddp st(1), st(0) // 00c3ef3d
        fldz  // 00c3ef3f
        fcomip st(0), st(1) // 00c3ef41
        fstp st(0) // 00c3ef43
        ja l_00c3f171 // 00c3ef45
        fld qword ptr [esi + 0b0h] // 00c3ef4b
        lea ecx, [esp + 018h] // 00c3ef51
        fstp qword ptr [esi + 098h] // 00c3ef55
        lea edx, [esp] // 00c3ef5b
        fld qword ptr [esi + 0b8h] // 00c3ef5e
        lea eax, [esp + 048h] // 00c3ef64
        fstp qword ptr [esi + 0a0h] // 00c3ef68
        fld qword ptr [esi + 0c0h] // 00c3ef6e
        fstp qword ptr [esi + 0a8h] // 00c3ef74
        fld qword ptr [esi + 0c8h] // 00c3ef7a
        fstp qword ptr [esi + 0b0h] // 00c3ef80
        fld qword ptr [esi + 0d0h] // 00c3ef86
        fstp qword ptr [esi + 0b8h] // 00c3ef8c
        fld qword ptr [esi + 0d8h] // 00c3ef92
        fstp qword ptr [esi + 0c0h] // 00c3ef98
        fld qword ptr [esi + 0e0h] // 00c3ef9e
        fstp qword ptr [esi + 0c8h] // 00c3efa4
        fld qword ptr [esi + 0e8h] // 00c3efaa
        fstp qword ptr [esi + 0d0h] // 00c3efb0
        fld qword ptr [esi + 0f0h] // 00c3efb6
        fstp qword ptr [esi + 0d8h] // 00c3efbc
        fld qword ptr [esi + 0110h] // 00c3efc2
        fstp qword ptr [esi + 0f8h] // 00c3efc8
        fld qword ptr [esi + 0118h] // 00c3efce
        fstp qword ptr [esi + 0100h] // 00c3efd4
        fld qword ptr [esi + 0120h] // 00c3efda
        fstp qword ptr [esi + 0108h] // 00c3efe0
        fld qword ptr [esi + 0128h] // 00c3efe6
        fstp qword ptr [esi + 0110h] // 00c3efec
        fld qword ptr [esi + 0130h] // 00c3eff2
        fstp qword ptr [esi + 0118h] // 00c3eff8
        fld qword ptr [esi + 0138h] // 00c3effe
        fstp qword ptr [esi + 0120h] // 00c3f004
        fld qword ptr [esi + 0140h] // 00c3f00a
        fstp qword ptr [esi + 0128h] // 00c3f010
        fld qword ptr [esi + 0148h] // 00c3f016
        fstp qword ptr [esi + 0130h] // 00c3f01c
        fld qword ptr [esi + 0150h] // 00c3f022
        fstp qword ptr [esi + 0138h] // 00c3f028
        fld qword ptr [esi + 0170h] // 00c3f02e
        fstp qword ptr [esi + 0158h] // 00c3f034
        fld qword ptr [esi + 0178h] // 00c3f03a
        fstp qword ptr [esi + 0160h] // 00c3f040
        fld qword ptr [esi + 0180h] // 00c3f046
        fstp qword ptr [esi + 0168h] // 00c3f04c
        fld qword ptr [esi + 0188h] // 00c3f052
        fstp qword ptr [esi + 0170h] // 00c3f058
        fld qword ptr [esi + 0190h] // 00c3f05e
        fstp qword ptr [esi + 0178h] // 00c3f064
        fld qword ptr [esi + 0198h] // 00c3f06a
        fstp qword ptr [esi + 0180h] // 00c3f070
        fld qword ptr [esi + 01a0h] // 00c3f076
        fstp qword ptr [esi + 0188h] // 00c3f07c
        fld qword ptr [esi + 01a8h] // 00c3f082
        fstp qword ptr [esi + 0190h] // 00c3f088
        fld qword ptr [esi + 01b0h] // 00c3f08e
        fstp qword ptr [esi + 0198h] // 00c3f094
        fld qword ptr [esi + 0b0h] // 00c3f09a
        fsub qword ptr [esi + 098h] // 00c3f0a0
        fstp qword ptr [esp + 018h] // 00c3f0a6
        fld qword ptr [esi + 0b8h] // 00c3f0aa
        fsub qword ptr [esi + 0a0h] // 00c3f0b0
        fstp qword ptr [esp + 020h] // 00c3f0b6
        fld qword ptr [esi + 0c0h] // 00c3f0ba
        fsub qword ptr [esi + 0a8h] // 00c3f0c0
        fstp qword ptr [esp + 028h] // 00c3f0c6
        fld qword ptr [esi + 0c8h] // 00c3f0ca
        fsub qword ptr [esi + 098h] // 00c3f0d0
        fstp qword ptr [esp] // 00c3f0d6
        fld qword ptr [esi + 0d0h] // 00c3f0d9
        fsub qword ptr [esi + 0a0h] // 00c3f0df
        fstp qword ptr [esp + 8] // 00c3f0e5
        fld qword ptr [esi + 0d8h] // 00c3f0e9
        fsub qword ptr [esi + 0a8h] // 00c3f0ef
        fstp qword ptr [esp + 010h] // 00c3f0f5
        call cross_reference // 00c3f0f9
        mov ecx, eax // 00c3f0fe
        fld qword ptr [ecx] // 00c3f100
        lea eax, [esi + 080h] // 00c3f102
        fstp qword ptr [eax] // 00c3f108
        fld qword ptr [ecx + 8] // 00c3f10a
        fstp qword ptr [eax + 8] // 00c3f10d
        sub esp, 8 // 00c3f110
        fld qword ptr [ecx + 010h] // 00c3f113
        fstp qword ptr [eax + 010h] // 00c3f116
        fld qword ptr [eax + 8] // 00c3f119
        fld qword ptr [eax] // 00c3f11c
        fld qword ptr [eax + 010h] // 00c3f11e
        fld qword ptr [eax + 8] // 00c3f121
        fmul qword ptr [esi + 0a0h] // 00c3f124
        fld qword ptr [eax] // 00c3f12a
        fmul qword ptr [esi + 098h] // 00c3f12c
        faddp st(1), st(0) // 00c3f132
        fld qword ptr [eax + 010h] // 00c3f134
        fmul qword ptr [esi + 0a8h] // 00c3f137
        faddp st(1), st(0) // 00c3f13d
        fld st(2) // 00c3f13f
        fmulp st(3), st(0) // 00c3f141
        fld st(3) // 00c3f143
        fmulp st(4), st(0) // 00c3f145
        fxch st(2) // 00c3f147
        faddp st(3), st(0) // 00c3f149
        fmul st(0), st(0) // 00c3f14b
        faddp st(2), st(0) // 00c3f14d
        fdivrp st(1), st(0) // 00c3f14f
        fchs  // 00c3f151
        fstp qword ptr [esp] // 00c3f153
        call scale_reference // 00c3f156
        mov dword ptr [esi + 01b8h], 3 // 00c3f15b
        add esp, 064h // 00c3f165
        pop esi
        ret  // 00c3f168
    l_00c3f169:
        fstp st(3) // 00c3f169
        fstp st(0) // 00c3f16b
        fstp st(1) // 00c3f16d
        fstp st(0) // 00c3f16f
    l_00c3f171:
        add esp, 064h // 00c3f171
        pop esi
        ret  // 00c3f174
    }
}
} // namespace bsp
