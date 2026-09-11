#include "bsp/native_camera_matrix_math.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera matrix math requires MSVC Win32 x87/SSE assembly.
#endif

namespace bsp {
namespace {
// Original read-only DWORDs D7A208 and D7A24C; equal bits, new addresses.
const std::uint32_t negative_zero_bits = 0x80000000u;
const std::uint32_t one_bits = 0x3f800000u;
}

// Complete original 00b63b30[531]. Original native ABI and
// raw alias/exception contract are documented in the header and packet report.
// Only the named read-only constant address operands are relocated.
__declspec(naked) void* __fastcall invert_native_camera_scaled_affine_00b63b30(void*, const void*) {
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

// Complete original 00413920[874]. Original native ABI and
// raw alias/exception contract are documented in the header and packet report.
// Only the named read-only constant address operands are relocated.
__declspec(naked) void* __fastcall multiply_native_camera_matrices_00413920(const void*, void*, void*, const void*) {
    __asm {
        // Original00413920: allocate0x40 scratch; load native stack arguments.
        sub esp,0x40
        mov edx,dword ptr [esp + 0x48]
        fld dword ptr [edx + 0x10]
        mov eax,dword ptr [esp + 0x44]
        fstp dword ptr [esp + 0x30]
        fld dword ptr [ecx + 0x4]
        fstp dword ptr [esp + 0x3c]
        fld dword ptr [edx]
        fstp dword ptr [esp + 0x28]
        fld dword ptr [ecx]
        fstp dword ptr [esp + 0x2c]
        fld dword ptr [edx + 0x20]
        fstp dword ptr [esp + 0x34]
        fld dword ptr [ecx + 0x8]
        fstp dword ptr [esp + 0x24]
        fld dword ptr [edx + 0x30]
        fstp dword ptr [esp + 0x38]
        fld dword ptr [ecx + 0xc]
        fstp dword ptr [esp + 0x48]
        fld dword ptr [esp + 0x3c]
        fld st(0)
        fld dword ptr [esp + 0x30]
        fld st(0)
        fmulp st(2), st(0)
        fld dword ptr [esp + 0x2c]
        fld st(0)
        fld dword ptr [esp + 0x28]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(4)
        faddp st(1), st(0)
        fld dword ptr [esp + 0x24]
        fld st(0)
        fmul dword ptr [esp + 0x34]
        faddp st(2),st(0)
        fld dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x38]
        faddp st(2),st(0)
        fxch st(1)
        fstp dword ptr [eax]
        // Original0041399b: cache right column1 before destination[1].
        fld dword ptr [edx + 0x14]
        fstp dword ptr [esp + 0x8]
        fld dword ptr [edx + 0x4]
        fstp dword ptr [esp + 0x3c]
        fld dword ptr [edx + 0x24]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [edx + 0x34]
        fstp dword ptr [esp + 0x10]
        fld dword ptr [esp + 0x3c]
        fmul st(0), st(2)
        fld dword ptr [esp + 0x8]
        fmul st(0), st(6)
        faddp st(1), st(0)
        fld dword ptr [esp + 0xc]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fld dword ptr [esp + 0x10]
        fmul dword ptr [esp + 0x48]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x4]
        // Original004139da: cache right column2 before destination[2].
        fld dword ptr [edx + 0x18]
        fstp dword ptr [esp + 0x18]
        fld dword ptr [edx + 0x8]
        fstp dword ptr [esp + 0x14]
        fld dword ptr [edx + 0x28]
        fstp dword ptr [esp + 0x1c]
        fld dword ptr [edx + 0x38]
        fstp dword ptr [esp + 0x20]
        fld dword ptr [esp + 0x14]
        fmul st(0), st(2)
        fld dword ptr [esp + 0x18]
        fmul st(0), st(6)
        faddp st(1), st(0)
        fld dword ptr [esp + 0x1c]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fld dword ptr [esp + 0x20]
        fmul dword ptr [esp + 0x48]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x8]
        // Original00413a19: cache right column3 before destination[3].
        fld dword ptr [edx + 0x1c]
        fstp dword ptr [esp + 0x28]
        fld dword ptr [edx + 0xc]
        fstp dword ptr [esp + 0x24]
        fld dword ptr [edx + 0x2c]
        fstp dword ptr [esp + 0x2c]
        fld dword ptr [edx + 0x3c]
        fstp dword ptr [esp + 0x30]
        fld dword ptr [esp + 0x24]
        fmulp st(2), st(0)
        fld dword ptr [esp + 0x28]
        fmulp st(5), st(0)
        fxch st(1)
        faddp st(4),st(0)
        fmul dword ptr [esp + 0x2c]
        faddp st(3),st(0)
        fld dword ptr [esp + 0x30]
        fmul dword ptr [esp + 0x48]
        faddp st(3),st(0)
        fxch st(2)
        fstp dword ptr [eax + 0xc]
        // Original00413a5a: cache left row1; all right fields already cached.
        fld dword ptr [ecx + 0x14]
        fstp dword ptr [esp + 0x48]
        fld dword ptr [ecx + 0x10]
        fstp dword ptr [esp + 0x4]
        fld dword ptr [ecx + 0x18]
        fstp dword ptr [esp + 0x44]
        fld dword ptr [ecx + 0x1c]
        fstp dword ptr [esp]
        fld dword ptr [esp + 0x4]
        fld st(0)
        fmul st(0), st(2)
        fld dword ptr [esp + 0x48]
        fmul st(0), st(4)
        faddp st(1), st(0)
        fld dword ptr [esp + 0x44]
        fld dword ptr [esp + 0x34]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [esp]
        fld dword ptr [esp + 0x38]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x10]
        fld dword ptr [esp + 0x3c]
        fld st(0)
        fmulp st(4), st(0)
        fld dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x8]
        faddp st(4),st(0)
        fld dword ptr [esp + 0x44]
        fmul dword ptr [esp + 0xc]
        faddp st(4),st(0)
        fld dword ptr [esp]
        fmul dword ptr [esp + 0x10]
        faddp st(4),st(0)
        fxch st(3)
        fstp dword ptr [eax + 0x14]
        fld dword ptr [esp + 0x4]
        fmul dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x18]
        faddp st(1), st(0)
        fld dword ptr [esp + 0x44]
        fmul dword ptr [esp + 0x1c]
        faddp st(1), st(0)
        fld dword ptr [esp]
        fmul dword ptr [esp + 0x20]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x18]
        fld dword ptr [esp + 0x4]
        fmul dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x28]
        faddp st(1), st(0)
        fld dword ptr [esp + 0x44]
        fmul dword ptr [esp + 0x2c]
        faddp st(1), st(0)
        fld dword ptr [esp]
        fmul dword ptr [esp + 0x30]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x1c]
        // Original00413b21: cache left row2; retain x87 right values.
        fld dword ptr [ecx + 0x24]
        fstp dword ptr [esp + 0x44]
        fld dword ptr [ecx + 0x20]
        fstp dword ptr [esp + 0x48]
        fld dword ptr [ecx + 0x28]
        fstp dword ptr [esp]
        fld dword ptr [ecx + 0x2c]
        fstp dword ptr [esp + 0x4]
        fld dword ptr [esp + 0x48]
        fmul st(0), st(4)
        fld dword ptr [esp + 0x44]
        fmul st(0), st(6)
        faddp st(1), st(0)
        fld dword ptr [esp]
        fmul st(0), st(3)
        faddp st(1), st(0)
        fld dword ptr [esp + 0x4]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x20]
        fld dword ptr [esp + 0x48]
        fmul st(0), st(3)
        fld dword ptr [esp + 0x44]
        fmul dword ptr [esp + 0x8]
        faddp st(1), st(0)
        fld dword ptr [esp]
        fmul dword ptr [esp + 0xc]
        faddp st(1), st(0)
        fld dword ptr [esp + 0x4]
        fmul dword ptr [esp + 0x10]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x24]
        fld dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x14]
        fld dword ptr [esp + 0x44]
        fmul dword ptr [esp + 0x18]
        faddp st(1), st(0)
        fld dword ptr [esp]
        fmul dword ptr [esp + 0x1c]
        faddp st(1), st(0)
        fld dword ptr [esp + 0x4]
        fmul dword ptr [esp + 0x20]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x28]
        fld dword ptr [esp + 0x48]
        fmul dword ptr [esp + 0x24]
        fld dword ptr [esp + 0x44]
        fmul dword ptr [esp + 0x28]
        faddp st(1), st(0)
        fld dword ptr [esp]
        fmul dword ptr [esp + 0x2c]
        faddp st(1), st(0)
        fld dword ptr [esp + 0x4]
        fmul dword ptr [esp + 0x30]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x2c]
        // Original00413bd2: cache left row3; consume remaining x87 values.
        fld dword ptr [ecx + 0x34]
        fstp dword ptr [esp + 0x44]
        fld dword ptr [ecx + 0x30]
        fstp dword ptr [esp + 0x48]
        fld dword ptr [ecx + 0x38]
        fstp dword ptr [esp + 0x3c]
        fld dword ptr [ecx + 0x3c]
        fstp dword ptr [esp + 0x38]
        fld dword ptr [esp + 0x48]
        fld st(0)
        fmulp st(5), st(0)
        fld dword ptr [esp + 0x44]
        fld st(0)
        fmulp st(7), st(0)
        fxch st(5)
        faddp st(6),st(0)
        fld dword ptr [esp + 0x3c]
        fld st(0)
        fmulp st(4), st(0)
        fxch st(6)
        faddp st(3),st(0)
        fld dword ptr [esp + 0x38]
        fld st(0)
        fmulp st(3), st(0)
        fxch st(3)
        faddp st(2),st(0)
        fxch st(1)
        fstp dword ptr [eax + 0x30]
        fld st(0)
        fmulp st(3), st(0)
        fld st(3)
        fmul dword ptr [esp + 0x8]
        faddp st(3),st(0)
        fld st(4)
        fmul dword ptr [esp + 0xc]
        faddp st(3),st(0)
        fld st(1)
        fmul dword ptr [esp + 0x10]
        faddp st(3),st(0)
        fxch st(2)
        fstp dword ptr [eax + 0x34]
        fld st(1)
        fmul dword ptr [esp + 0x14]
        fld st(3)
        fmul dword ptr [esp + 0x18]
        faddp st(1), st(0)
        fld st(4)
        fmul dword ptr [esp + 0x1c]
        faddp st(1), st(0)
        fld st(1)
        fmul dword ptr [esp + 0x20]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x38]
        fld dword ptr [esp + 0x24]
        fmulp st(2), st(0)
        fld dword ptr [esp + 0x28]
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2),st(0)
        fld dword ptr [esp + 0x2c]
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2),st(0)
        fmul dword ptr [esp + 0x30]
        faddp st(1), st(0)
        fstp dword ptr [eax + 0x3c]
        add esp,0x40
        ret 0x8
    }
}

// Complete original 00b6d4d0[339]. Original native ABI and
// raw alias/exception contract are documented in the header and packet report.
// Only the named read-only constant address operands are relocated.
__declspec(naked) const void* __fastcall compose_native_camera_affine_00b6d4d0(void*, const void*, const void*) {
    __asm {
        mov eax,dword ptr [esp + 0x4] // 00b6d4d0
        fld dword ptr [edx] // 00b6d4d4
        fmul dword ptr [eax] // 00b6d4d6
        xorps xmm0,xmm0 // 00b6d4d8
        fld dword ptr [edx + 0x4] // 00b6d4db
        fmul dword ptr [eax + 0x10] // 00b6d4de
        faddp st(1), st(0) // 00b6d4e1
        fld dword ptr [eax + 0x20] // 00b6d4e3
        fmul dword ptr [edx + 0x8] // 00b6d4e6
        faddp st(1), st(0) // 00b6d4e9
        fstp dword ptr [ecx] // 00b6d4eb
        fld dword ptr [eax + 0x14] // 00b6d4ed
        fmul dword ptr [edx + 0x4] // 00b6d4f0
        fld dword ptr [eax + 0x4] // 00b6d4f3
        fmul dword ptr [edx] // 00b6d4f6
        faddp st(1), st(0) // 00b6d4f8
        fld dword ptr [edx + 0x8] // 00b6d4fa
        fmul dword ptr [eax + 0x24] // 00b6d4fd
        faddp st(1), st(0) // 00b6d500
        fstp dword ptr [ecx + 0x4] // 00b6d502
        fld dword ptr [eax + 0x18] // 00b6d505
        fmul dword ptr [edx + 0x4] // 00b6d508
        fld dword ptr [eax + 0x8] // 00b6d50b
        fmul dword ptr [edx] // 00b6d50e
        faddp st(1), st(0) // 00b6d510
        fld dword ptr [edx + 0x8] // 00b6d512
        fmul dword ptr [eax + 0x28] // 00b6d515
        movss dword ptr [ecx + 0xc],xmm0 // 00b6d518
        faddp st(1), st(0) // 00b6d51d
        fstp dword ptr [ecx + 0x8] // 00b6d51f
        fld dword ptr [edx + 0x14] // 00b6d522
        fmul dword ptr [eax + 0x10] // 00b6d525
        fld dword ptr [eax] // 00b6d528
        fmul dword ptr [edx + 0x10] // 00b6d52a
        faddp st(1), st(0) // 00b6d52d
        fld dword ptr [edx + 0x18] // 00b6d52f
        fmul dword ptr [eax + 0x20] // 00b6d532
        faddp st(1), st(0) // 00b6d535
        fstp dword ptr [ecx + 0x10] // 00b6d537
        fld dword ptr [eax + 0x4] // 00b6d53a
        fmul dword ptr [edx + 0x10] // 00b6d53d
        fld dword ptr [edx + 0x14] // 00b6d540
        fmul dword ptr [eax + 0x14] // 00b6d543
        faddp st(1), st(0) // 00b6d546
        fld dword ptr [edx + 0x18] // 00b6d548
        fmul dword ptr [eax + 0x24] // 00b6d54b
        faddp st(1), st(0) // 00b6d54e
        fstp dword ptr [ecx + 0x14] // 00b6d550
        fld dword ptr [eax + 0x8] // 00b6d553
        fmul dword ptr [edx + 0x10] // 00b6d556
        fld dword ptr [edx + 0x14] // 00b6d559
        fmul dword ptr [eax + 0x18] // 00b6d55c
        faddp st(1), st(0) // 00b6d55f
        fld dword ptr [edx + 0x18] // 00b6d561
        fmul dword ptr [eax + 0x28] // 00b6d564
        movss dword ptr [ecx + 0x1c],xmm0 // 00b6d567
        faddp st(1), st(0) // 00b6d56c
        fstp dword ptr [ecx + 0x18] // 00b6d56e
        fld dword ptr [edx + 0x20] // 00b6d571
        fmul dword ptr [eax] // 00b6d574
        fld dword ptr [edx + 0x24] // 00b6d576
        fmul dword ptr [eax + 0x10] // 00b6d579
        faddp st(1), st(0) // 00b6d57c
        fld dword ptr [eax + 0x20] // 00b6d57e
        fmul dword ptr [edx + 0x28] // 00b6d581
        faddp st(1), st(0) // 00b6d584
        fstp dword ptr [ecx + 0x20] // 00b6d586
        fld dword ptr [edx + 0x24] // 00b6d589
        fmul dword ptr [eax + 0x14] // 00b6d58c
        fld dword ptr [edx + 0x20] // 00b6d58f
        fmul dword ptr [eax + 0x4] // 00b6d592
        faddp st(1), st(0) // 00b6d595
        fld dword ptr [edx + 0x28] // 00b6d597
        fmul dword ptr [eax + 0x24] // 00b6d59a
        faddp st(1), st(0) // 00b6d59d
        fstp dword ptr [ecx + 0x24] // 00b6d59f
        fld dword ptr [edx + 0x24] // 00b6d5a2
        fmul dword ptr [eax + 0x18] // 00b6d5a5
        fld dword ptr [edx + 0x20] // 00b6d5a8
        fmul dword ptr [eax + 0x8] // 00b6d5ab
        faddp st(1), st(0) // 00b6d5ae
        fld dword ptr [edx + 0x28] // 00b6d5b0
        fmul dword ptr [eax + 0x28] // 00b6d5b3
        movss dword ptr [ecx + 0x2c],xmm0 // 00b6d5b6
        movss xmm0,dword ptr [one_bits] // 00b6d5bb
        faddp st(1), st(0) // 00b6d5c3
        fstp dword ptr [ecx + 0x28] // 00b6d5c5
        fld dword ptr [edx + 0x34] // 00b6d5c8
        fmul dword ptr [eax + 0x10] // 00b6d5cb
        fld dword ptr [eax] // 00b6d5ce
        fmul dword ptr [edx + 0x30] // 00b6d5d0
        faddp st(1), st(0) // 00b6d5d3
        fld dword ptr [edx + 0x38] // 00b6d5d5
        fmul dword ptr [eax + 0x20] // 00b6d5d8
        faddp st(1), st(0) // 00b6d5db
        fadd dword ptr [eax + 0x30] // 00b6d5dd
        fstp dword ptr [ecx + 0x30] // 00b6d5e0
        fld dword ptr [eax + 0x4] // 00b6d5e3
        fmul dword ptr [edx + 0x30] // 00b6d5e6
        fld dword ptr [edx + 0x34] // 00b6d5e9
        fmul dword ptr [eax + 0x14] // 00b6d5ec
        faddp st(1), st(0) // 00b6d5ef
        fld dword ptr [edx + 0x38] // 00b6d5f1
        fmul dword ptr [eax + 0x24] // 00b6d5f4
        faddp st(1), st(0) // 00b6d5f7
        fadd dword ptr [eax + 0x34] // 00b6d5f9
        fstp dword ptr [ecx + 0x34] // 00b6d5fc
        fld dword ptr [eax + 0x8] // 00b6d5ff
        fmul dword ptr [edx + 0x30] // 00b6d602
        fld dword ptr [edx + 0x34] // 00b6d605
        fmul dword ptr [eax + 0x18] // 00b6d608
        faddp st(1), st(0) // 00b6d60b
        fld dword ptr [edx + 0x38] // 00b6d60d
        fmul dword ptr [eax + 0x28] // 00b6d610
        faddp st(1), st(0) // 00b6d613
        fadd dword ptr [eax + 0x38] // 00b6d615
        movss dword ptr [ecx + 0x3c],xmm0 // 00b6d618
        fstp dword ptr [ecx + 0x38] // 00b6d61d
        ret 0x4 // 00b6d620
    }
}
} // namespace bsp
