#include "bsp/vector_helpers.hpp"
#include <cstring>

// Genuine current Win32 CRT x87 entry. BF7030 in the installed image is the
// original CRT sqrt boundary; keep that library operation external.
extern "C" double __cdecl _CIsqrt();

namespace bsp {
namespace {
const double squared_length_cutoff = 1e-10; // CE3820: bb bd d7 d9 df 7c db 3d.

__declspec(naked) float __fastcall length_kernel(const float*) {
    __asm {
        push ecx
        fld dword ptr [ecx + 4]
        fld dword ptr [ecx]
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp]
        fld qword ptr [squared_length_cutoff]
        fld dword ptr [esp]
        fcomi st(0), st(1)
        fstp st(1)
        jbe zero_result
        call _CIsqrt
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        pop ecx
        ret
    zero_result:
        xorps xmm0, xmm0
        fstp st(0)
        movss dword ptr [esp], xmm0
        fld dword ptr [esp]
        pop ecx
        ret
    }
}
}

float length_2d_00414c60(const std::array<float, 2>& value) {
    return length_kernel(value.data());
}

void transform_point_copy_00414cd0(float* destination,
    const std::array<float, 3>& source, const CameraMatrix& matrix) {
    std::array<float, 3> transformed;
    transform_point_004142e0(source, matrix, transformed);
    // MOVSS copies in the native wrapper preserve float bits, with no x87 load
    // or additional arithmetic. Borrow raw output storage to permit matrix aliases.
    std::memcpy(destination, transformed.data(), sizeof transformed);
}
}
