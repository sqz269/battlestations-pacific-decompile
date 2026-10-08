#include "bsp/native_unit_wake_append_dependencies.hpp"

// Genuine current Win32 CRT intrinsic entry, already used by the exact raw
// 00414C60 kernel and camera decomposition. Inline CALL passes/returns ST0;
// this declaration is not an ordinary stack-argument sqrt adapter.
extern "C" double __cdecl _CIsqrt();

namespace bsp {
namespace {
// Original read-only CE3820 payload: bb bd d7 d9 df 7c db 3d.
const double squared_length_cutoff_00ce3820 = 1e-10;
}

// Complete 0042B2F0..0042B33C: 77 bytes / 31 instructions. Only the immutable
// cutoff address and genuine CRT call target relocate. Existing typed helpers
// stay separate; their C++ interface does not supply this raw x87 contract.
__declspec(naked) float __fastcall native_unit_wake_length_0042b2f0(
    const float*) noexcept {
    __asm {
        push ecx
        fld dword ptr [ecx + 4]
        fld dword ptr [ecx]
        fld dword ptr [ecx + 8]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp]
        fld qword ptr [squared_length_cutoff_00ce3820]
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

// Complete 00810160..0081018A: 43 bytes / 15 instructions. There are no
// external calls, relocations or globals. Preserve all six x87 load/store
// events even for self-copy or partially overlapping actual storage.
__declspec(naked) void* __fastcall copy_native_unit_wake_sample_00810160(
    void*, void*, const void*) noexcept {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [esp + 4]
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 0xc]
        fstp dword ptr [eax + 0xc]
        fld dword ptr [ecx + 0x10]
        fstp dword ptr [eax + 0x10]
        fld dword ptr [ecx + 0x14]
        fstp dword ptr [eax + 0x14]
        ret 4
    }
}

} // namespace bsp
