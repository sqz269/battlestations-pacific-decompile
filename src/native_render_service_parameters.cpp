#include "bsp/native_render_service_parameters.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "Native service storage requires Win32");

void* initialize_native_render_service_parameters_00b0cd80(void* storage,
    const NativeRenderServiceParameterConstants& values) noexcept {
    // Capture only the fixed reference bindings. Each referent is read at
    // its original point below, after any preceding aliased destination store.
    const auto* const c0 = &values.actual_00ce4788;
    const auto* const c1 = &values.actual_00d5e138;
    const auto* const c2 = &values.actual_00ce3854;
    const auto* const c3 = &values.actual_00d5e134;
    const auto* const c4 = &values.actual_00ce3850;
    const auto* const c5 = &values.actual_00ce3958;
    const auto* const c6 = &values.actual_00cef1b0;
    const auto* const c7 = &values.actual_00ce54a0;
    const auto* const c8 = &values.actual_00ce7804;
    const auto* const c9 = &values.actual_00ce69c8;
    const auto* const c10 = &values.actual_00ced318;
    const auto* const c11 = &values.actual_00cf2548;
    const auto* const c12 = &values.actual_00cef0b8;
    const auto* const c13 = &values.actual_00d7a2f0;
    const auto* const c14 = &values.actual_00d5e130;
    const auto* const c15 = &values.actual_00ce3930;
    const auto* const c16 = &values.actual_00d5e12c;
    const auto* const c17 = &values.actual_00ce380c;
    const auto* const c18 = &values.actual_00ce3800;
    const auto* const c19 = &values.actual_00d7a24c;
    const auto* const c20 = &values.actual_00ce3d34;
    __asm {
        mov ecx, c0
        movss xmm0, dword ptr [ecx]
        mov ecx, c1
        movss xmm2, dword ptr [ecx]
        mov ecx, c2
        movss xmm3, dword ptr [ecx]
        mov eax, storage
        mov ecx, c3
        movss xmm1, dword ptr [ecx]
        movss dword ptr [eax], xmm0
        mov ecx, c4
        movss xmm0, dword ptr [ecx]
        movss dword ptr [eax + 04h], xmm0
        mov ecx, c5
        movss xmm0, dword ptr [ecx]
        movss dword ptr [eax + 18h], xmm2
        mov ecx, c6
        movss xmm2, dword ptr [ecx]
        movss dword ptr [eax + 30h], xmm3
        mov ecx, c7
        movss xmm3, dword ptr [ecx]
        movss dword ptr [eax + 08h], xmm0
        mov ecx, c8
        movss xmm0, dword ptr [ecx]
        movss dword ptr [eax + 1ch], xmm2
        mov ecx, c9
        movss xmm2, dword ptr [ecx]
        movss dword ptr [eax + 34h], xmm3
        mov ecx, c10
        movss xmm3, dword ptr [ecx]
        movss dword ptr [eax + 0ch], xmm0
        movss dword ptr [eax + 20h], xmm2
        mov ecx, c11
        movss xmm2, dword ptr [ecx]
        movss dword ptr [eax + 38h], xmm3
        mov ecx, c12
        movss xmm3, dword ptr [ecx]
        movss dword ptr [eax + 4ch], xmm0
        mov ecx, c13
        movss xmm0, dword ptr [ecx]
        movss dword ptr [eax + 24h], xmm2
        mov ecx, c14
        movss xmm2, dword ptr [ecx]
        movss dword ptr [eax + 3ch], xmm3
        mov ecx, c15
        movss xmm3, dword ptr [ecx]
        movss dword ptr [eax + 54h], xmm0
        movss dword ptr [eax + 58h], xmm0
        mov ecx, c16
        movss xmm0, dword ptr [ecx]
        movss dword ptr [eax + 10h], xmm1
        mov ecx, c17
        movss xmm1, dword ptr [ecx]
        movss dword ptr [eax + 28h], xmm2
        mov ecx, c18
        movss xmm2, dword ptr [ecx]
        movss dword ptr [eax + 40h], xmm3
        mov ecx, c19
        movss xmm3, dword ptr [ecx]
        movss dword ptr [eax + 5ch], xmm0
        mov ecx, c20
        movss xmm0, dword ptr [ecx]
        movss dword ptr [eax + 14h], xmm1
        movss dword ptr [eax + 2ch], xmm2
        movss dword ptr [eax + 44h], xmm3
        movss dword ptr [eax + 48h], xmm2
        movss dword ptr [eax + 50h], xmm1
        movss dword ptr [eax + 60h], xmm0
        movss dword ptr [eax + 64h], xmm3
    }
    return storage;
}

} // namespace bsp
