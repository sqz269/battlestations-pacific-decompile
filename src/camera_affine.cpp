#include "bsp/camera_affine.hpp"
#include <cstdint>

namespace bsp {
namespace {
// Original instruction ordering and store boundaries; no calls into game memory.
const std::uint32_t one_bits = 0x3f800000u;
__declspec(naked) void __fastcall compose_kernel(float*, const float*, const float*) {
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
}

void compose_camera_affine_00b6d4d0(CameraMatrix& destination,
    const CameraMatrix& left, const CameraMatrix& right) {
    compose_kernel(destination.data(), left.data(), right.data());
}

// Value/kernel projection of004142e0; no original object-layout assumptions.
// Preserve all three outputs and the original x87 product/add/spill order even
// though00b51a20 consumes only Z. Native input XYZ is staged before any writes.
void transform_point_004142e0(const std::array<float, 3>& point,
    const CameraMatrix& matrix, std::array<float, 3>& result) {
    const float* source = point.data();
    const float* transform = matrix.data();
    float* output = result.data();
    float values[3];
    __asm {
        mov ecx, source
        fld dword ptr [ecx + 4]
        fstp dword ptr values[0]
        fld dword ptr [ecx]
        fstp dword ptr values[4]
        fld dword ptr [ecx + 8]
        fstp dword ptr values[8]
        mov ecx, transform
        mov eax, output
        fld dword ptr [ecx + 16]
        fld dword ptr values[0]
        fld st(0)
        fmulp st(2), st(0)
        fld dword ptr [ecx]
        fld dword ptr values[4]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(3)
        faddp st(1), st(0)
        fld dword ptr [ecx + 32]
        fld dword ptr values[8]
        fld st(0)
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fadd dword ptr [ecx + 48]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fmul st(0), st(3)
        fld dword ptr [ecx + 20]
        fmul st(0), st(3)
        faddp st(1), st(0)
        fld dword ptr [ecx + 36]
        fmul st(0), st(2)
        faddp st(1), st(0)
        fadd dword ptr [ecx + 52]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fmulp st(3), st(0)
        fld dword ptr [ecx + 24]
        fmulp st(2), st(0)
        fxch st(2)
        faddp st(1), st(0)
        fld dword ptr [ecx + 40]
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fadd dword ptr [ecx + 56]
        fstp dword ptr [eax + 8]
    }
}

}
