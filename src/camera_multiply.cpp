#include "bsp/camera_multiply.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Camera matrix multiplication requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
// Assembly-backed reconstruction, not loaded executable bytes. Original874-byte
// body SHA256:a2ae101c6235ce95cd7cffdc28deb5797b9d844dbc809e729db5ac5b134e29d9.
// Its input caches and x87 register lifetimes are part of the numerical/alias
// contract. Keep this ordering instead of replacing it with a generic dot loop.
// No calls, globals, branches or FP-control changes occur in the native body.
// ECX=left. The unused EDX argument lets fastcall place destination/right on the
// stack exactly as in the original thiscall-style matrix operator. The private
// helper returns destination in EAX; the public void API discards that pointer.
__declspec(naked) float* __fastcall multiply_native_order(
    const float*, void*, float*, const float*) {
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
}

void multiply_camera_matrices_00413920(CameraMatrix& dst,
    const CameraMatrix& left, const CameraMatrix& right) {
    multiply_native_order(left.data(), nullptr, dst.data(), right.data());
}
}
