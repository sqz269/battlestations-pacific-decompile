#include "bsp/camera_inverse.hpp"
#include <cstdint>

namespace bsp {
namespace {
// Assembly-grounded kernel: original register/stack schedule preserves every
// float32 spill and the x87/SSE boundary. No original addresses are executed.
const std::uint32_t negative_zero_bits = 0x80000000u;
__declspec(naked) float* __fastcall inverse_kernel(float*, const float*) {
    __asm {
        sub esp,0x34 // 00b63b30
        push esi // 00b63b33
        mov eax,ecx // 00b63b34
        push edi // 00b63b36
        mov ecx,0x10 // 00b63b37
        mov esi,edx // 00b63b3c
        mov edi,eax // 00b63b3e
        rep movsd // 00b63b40
        fld dword ptr [edx + 0x4] // 00b63b42
        fstp dword ptr [esp + 0x8] // 00b63b45
        fld dword ptr [edx + 0x8] // 00b63b49
        fstp dword ptr [esp + 0x10] // 00b63b4c
        fld dword ptr [esp + 0x8] // 00b63b50
        movss xmm0,dword ptr [edx + 0x20] // 00b63b54
        movss dword ptr [esp + 0x24],xmm0 // 00b63b59
        movss xmm0,dword ptr [edx + 0x24] // 00b63b5f
        movss dword ptr [esp + 0x20],xmm0 // 00b63b64
        movss xmm0,dword ptr [edx + 0x28] // 00b63b6a
        movss dword ptr [esp + 0x28],xmm0 // 00b63b6f
        movss xmm0,dword ptr [edx + 0x10] // 00b63b75
        movss dword ptr [esp + 0x18],xmm0 // 00b63b7a
        movss xmm0,dword ptr [edx + 0x14] // 00b63b80
        movss dword ptr [esp + 0x14],xmm0 // 00b63b85
        movss xmm0,dword ptr [edx + 0x18] // 00b63b8b
        movss dword ptr [esp + 0x1c],xmm0 // 00b63b90
        movss xmm0,dword ptr [edx] // 00b63b96
        movss dword ptr [esp + 0xc],xmm0 // 00b63b9a
        fld dword ptr [esp + 0xc] // 00b63ba0
        movss dword ptr [esp + 0x2c],xmm0 // 00b63ba4
        fld dword ptr [esp + 0x10] // 00b63baa
        fld st(1) // 00b63bae
        fmulp st(2), st(0) // 00b63bb0
        fxch st(1) // 00b63bb2
        fstp dword ptr [esp + 0x10] // 00b63bb4
        fld dword ptr [esp + 0x10] // 00b63bb8
        fld st(2) // 00b63bbc
        fmulp st(3), st(0) // 00b63bbe
        fxch st(2) // 00b63bc0
        fstp dword ptr [esp + 0x10] // 00b63bc2
        fld dword ptr [esp + 0x10] // 00b63bc6
        faddp st(2),st(0) // 00b63bca
        fmul st(0), st(0) // 00b63bcc
        fstp dword ptr [esp + 0x10] // 00b63bce
        fadd dword ptr [esp + 0x10] // 00b63bd2
        fstp dword ptr [esp + 0x30] // 00b63bd6
        fld dword ptr [esp + 0x14] // 00b63bda
        fld dword ptr [esp + 0x18] // 00b63bde
        fld dword ptr [esp + 0x1c] // 00b63be2
        fld st(1) // 00b63be6
        fmulp st(2), st(0) // 00b63be8
        fxch st(1) // 00b63bea
        fstp dword ptr [esp + 0x1c] // 00b63bec
        fld dword ptr [esp + 0x1c] // 00b63bf0
        fld st(2) // 00b63bf4
        fmulp st(3), st(0) // 00b63bf6
        fxch st(2) // 00b63bf8
        fstp dword ptr [esp + 0x1c] // 00b63bfa
        fld dword ptr [esp + 0x1c] // 00b63bfe
        faddp st(2),st(0) // 00b63c02
        fmul st(0), st(0) // 00b63c04
        fstp dword ptr [esp + 0x1c] // 00b63c06
        fadd dword ptr [esp + 0x1c] // 00b63c0a
        fstp dword ptr [esp + 0x34] // 00b63c0e
        fld dword ptr [esp + 0x20] // 00b63c12
        fld dword ptr [esp + 0x24] // 00b63c16
        fld dword ptr [esp + 0x28] // 00b63c1a
        fld st(1) // 00b63c1e
        fmulp st(2), st(0) // 00b63c20
        fxch st(1) // 00b63c22
        fstp dword ptr [esp + 0x28] // 00b63c24
        fld dword ptr [esp + 0x28] // 00b63c28
        fld st(2) // 00b63c2c
        fmulp st(3), st(0) // 00b63c2e
        fxch st(2) // 00b63c30
        fstp dword ptr [esp + 0x28] // 00b63c32
        fld dword ptr [esp + 0x28] // 00b63c36
        faddp st(2),st(0) // 00b63c3a
        fmul st(0), st(0) // 00b63c3c
        fstp dword ptr [esp + 0x28] // 00b63c3e
        fadd dword ptr [esp + 0x28] // 00b63c42
        fstp dword ptr [esp + 0x38] // 00b63c46
        fld dword ptr [esp + 0x2c] // 00b63c4a
        fld dword ptr [esp + 0x30] // 00b63c4e
        movss xmm0,dword ptr [negative_zero_bits] // 00b63c52
        fld st(0) // 00b63c5a
        movaps xmm1,xmm0 // 00b63c5c
        fdivp st(2),st(0) // 00b63c5f
        movaps xmm2,xmm0 // 00b63c61
        pop edi // 00b63c64
        pop esi // 00b63c65
        fxch st(1) // 00b63c66
        fstp dword ptr [eax] // 00b63c68
        fld dword ptr [edx + 0x4] // 00b63c6a
        fdiv st(0),st(1) // 00b63c6d
        fstp dword ptr [eax + 0x10] // 00b63c6f
        fdivr dword ptr [edx + 0x8] // 00b63c72
        fstp dword ptr [eax + 0x20] // 00b63c75
        fld dword ptr [edx + 0x10] // 00b63c78
        fld dword ptr [esp + 0x2c] // 00b63c7b
        fld st(0) // 00b63c7f
        fdivp st(2),st(0) // 00b63c81
        fxch st(1) // 00b63c83
        fstp dword ptr [eax + 0x4] // 00b63c85
        fld dword ptr [edx + 0x14] // 00b63c88
        fdiv st(0),st(1) // 00b63c8b
        fstp dword ptr [eax + 0x14] // 00b63c8d
        fdivr dword ptr [edx + 0x18] // 00b63c90
        fstp dword ptr [eax + 0x24] // 00b63c93
        fld dword ptr [edx + 0x20] // 00b63c96
        fld dword ptr [esp + 0x30] // 00b63c99
        fld st(0) // 00b63c9d
        fdivp st(2),st(0) // 00b63c9f
        fxch st(1) // 00b63ca1
        fstp dword ptr [eax + 0x8] // 00b63ca3
        fld dword ptr [edx + 0x24] // 00b63ca6
        fdiv st(0),st(1) // 00b63ca9
        fstp dword ptr [eax + 0x18] // 00b63cab
        fdivr dword ptr [edx + 0x28] // 00b63cae
        fstp dword ptr [esp + 0x24] // 00b63cb1
        fld dword ptr [esp + 0x24] // 00b63cb5
        fst dword ptr [eax + 0x28] // 00b63cb9
        fld dword ptr [eax + 0x34] // 00b63cbc
        fstp dword ptr [esp + 0x20] // 00b63cbf
        fld dword ptr [eax + 0x30] // 00b63cc3
        fstp dword ptr [esp + 0x24] // 00b63cc6
        fld dword ptr [eax + 0x38] // 00b63cca
        fstp dword ptr [esp + 0x1c] // 00b63ccd
        fld dword ptr [esp + 0x24] // 00b63cd1
        fld st(0) // 00b63cd5
        fmul dword ptr [eax] // 00b63cd7
        fld dword ptr [esp + 0x20] // 00b63cd9
        fld st(0) // 00b63cdd
        fmul dword ptr [eax + 0x10] // 00b63cdf
        faddp st(2),st(0) // 00b63ce2
        fld dword ptr [esp + 0x1c] // 00b63ce4
        fld st(0) // 00b63ce8
        fmul dword ptr [eax + 0x20] // 00b63cea
        faddp st(3),st(0) // 00b63ced
        fxch st(2) // 00b63cef
        fstp dword ptr [esp + 0x28] // 00b63cf1
        fld st(2) // 00b63cf5
        subss xmm1,dword ptr [esp + 0x28] // 00b63cf7
        fmul dword ptr [eax + 0x4] // 00b63cfd
        fld st(1) // 00b63d00
        fmul dword ptr [eax + 0x14] // 00b63d02
        faddp st(1), st(0) // 00b63d05
        fld st(2) // 00b63d07
        fmul dword ptr [eax + 0x24] // 00b63d09
        faddp st(1), st(0) // 00b63d0c
        fstp dword ptr [esp + 0x2c] // 00b63d0e
        fld dword ptr [eax + 0x8] // 00b63d12
        subss xmm2,dword ptr [esp + 0x2c] // 00b63d15
        fmulp st(3), st(0) // 00b63d1b
        fmul dword ptr [eax + 0x18] // 00b63d1d
        movss dword ptr [eax + 0x30],xmm1 // 00b63d20
        movss dword ptr [eax + 0x34],xmm2 // 00b63d25
        faddp st(2),st(0) // 00b63d2a
        fmulp st(2), st(0) // 00b63d2c
        faddp st(1), st(0) // 00b63d2e
        fstp dword ptr [esp + 0x30] // 00b63d30
        subss xmm0,dword ptr [esp + 0x30] // 00b63d34
        movss dword ptr [eax + 0x38],xmm0 // 00b63d3a
        add esp,0x34 // 00b63d3f
        ret // 00b63d42
    }
}
}

void invert_camera_affine_00b63b30(CameraMatrix& destination, const CameraMatrix& source) {
    inverse_kernel(destination.data(), source.data());
}
}
