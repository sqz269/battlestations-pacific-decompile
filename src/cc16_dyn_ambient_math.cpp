#include "bsp/cc16_dyn_ambient_math.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error cc16_dyn_ambient_math requires the MSVC Win32 inline assembler
#endif

namespace bsp::cc16_dyn_ambient_math {
namespace {

// Private raw-ABI bodies. Every instruction, including its encoding, matches the
// corresponding native span. No external data, calls, CW/MXCSR edits or synthetic
// arithmetic is introduced. The declared C++ void/cdecl type is never called by
// C++: the wrappers below supply the evidenced native stack/register contract.
__declspec(naked) void __cdecl ccw_native_body_00c377f0() noexcept {
    __asm {
        sub esp, 8
        fld dword ptr [esp + 14h]
        fld dword ptr [esp + 0Ch]
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp]          // 00C37801: rounded x1-x0
        fld dword ptr [esp + 18h]
        fld dword ptr [esp + 10h]
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 4]      // 00C37812: rounded y1-y0
        fld dword ptr [esp + 1Ch]
        fsubrp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 14h]    // 00C3781E: rounded x2-x0
        fsubr dword ptr [esp + 20h]
        fstp dword ptr [esp + 18h]    // 00C37826: rounded y2-y0
        fld dword ptr [esp + 18h]
        fmul dword ptr [esp]
        fld dword ptr [esp + 4]
        fmul dword ptr [esp + 14h]
        fsubp st(1), st(0)
        fldz
        fxch st(1)
        _emit 0DFh                   // 00C3783F: FCOMIP ST(0), ST(1)
        _emit 0F1h
        fstp st(0)
        jbe ccw_false
        mov eax, 1
        add esp, 8
        ret 18h
    ccw_false:
        xor eax, eax
        add esp, 8
        ret 18h
    }
}

__declspec(naked) void __cdecl barycentric_native_body_00c37860() noexcept {
    __asm {
        sub esp, 10h
        fld dword ptr [esp + 1Ch]
        fld dword ptr [esp + 14h]
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp]          // 00C37871: e1x
        fld dword ptr [esp + 20h]
        fld dword ptr [esp + 18h]
        fld st(0)
        fsubp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 4]      // 00C37882: e1y
        fld dword ptr [esp + 24h]
        fsub st(0), st(2)
        fstp dword ptr [esp + 1Ch]    // 00C3788C: e2x; reuses x1 slot
        fld dword ptr [esp + 28h]
        fsub st(0), st(1)
        fstp dword ptr [esp + 20h]    // 00C37896: e2y; reuses y1 slot
        fld dword ptr [esp + 2Ch]
        fsubrp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 8]      // 00C378A2: dpx
        fsubr dword ptr [esp + 30h]
        fstp dword ptr [esp + 0Ch]    // 00C378AA: dpy
        fld dword ptr [esp]
        fld st(0)
        fld dword ptr [esp + 4]
        fld st(0)
        fld st(2)
        fmulp st(3), st(0)
        _emit 0DCh                   // 00C378BD: FMUL ST(0), ST(0), DC C8
        _emit 0C8h
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 24h]    // 00C378C3: d11; reuses x2 slot
        fld dword ptr [esp + 20h]
        fld st(0)
        fmul st(0), st(2)
        fld dword ptr [esp + 1Ch]
        fld st(0)
        fmul st(0), st(5)
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 1Ch]    // 00C378DB: d12; replaces e2x
        fld st(1)
        fld st(1)
        _emit 0DCh                   // 00C378E3: FMUL ST(0), ST(0), DC C8
        _emit 0C8h
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 14h]    // 00C378EB: d22; reuses x0 slot
        fld dword ptr [esp + 8]
        fld st(0)
        fmulp st(5), st(0)
        fld dword ptr [esp + 0Ch]
        fld st(0)
        fmulp st(5), st(0)
        fxch st(5)
        faddp st(4), st(0)
        fxch st(3)
        fstp dword ptr [esp + 2Ch]    // 00C37905: d1p; reuses px slot
        fxch st(3)
        fmulp st(1), st(0)
        fxch st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp]          // 00C37913: d2p; replaces e1x
        fld dword ptr [esp + 1Ch]
        fld st(0)
        fld dword ptr [esp + 14h]
        fld st(0)
        fld dword ptr [esp + 24h]
        fld st(0)
        fmulp st(2), st(0)
        fld st(3)
        fmulp st(4), st(0)
        fxch st(1)
        fsubrp st(3), st(0)
        fxch st(2)
        fstp dword ptr [esp + 1Ch]    // 00C37934: denominator; replaces d12
        fld dword ptr [esp + 2Ch]
        fld st(0)
        fmulp st(2), st(0)
        fld dword ptr [esp]
        fld st(0)
        fmul st(0), st(5)
        fsubp st(3), st(0)
        fld dword ptr [esp + 1Ch]
        fld st(0)
        fdivp st(4), st(0)
        fxch st(3)
        fstp dword ptr [esp + 1Ch]    // 00C37953: rounded u quotient
        fld dword ptr [esp + 1Ch]
        fst dword ptr [eax]          // 00C3795B: publish u, retain it in x87
        fxch st(1)
        fmulp st(4), st(0)
        fxch st(1)
        fmulp st(4), st(0)
        fxch st(2)
        fsubrp st(3), st(0)
        fdivp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 1Ch]    // 00C3796D: rounded v quotient
        fld dword ptr [esp + 1Ch]
        fst dword ptr [ecx]          // 00C37975: publish v, retain it in x87
        fld1
        fsubrp st(2), st(0)
        fsubp st(1), st(0)
        fstp dword ptr [edx]         // 00C3797D: publish w = (1-u)-v
        add esp, 10h
        ret 20h
    }
}

}  // namespace

// Keep named C++ arguments anchored at EBP while the inline assembler pushes
// native arguments. Suppress /RTC helper calls so the wrappers themselves have
// only the evidenced raw-body call, also in Debug. Both settings are restored.
#pragma runtime_checks("", off)
#pragma optimize("y", off)

__declspec(noinline) std::uint32_t __cdecl ccw_2d_ambient_00c377f0(
    float x0, float y0, float x1, float y1, float x2, float y2) noexcept {
    std::uint32_t result;
    __asm {
        push dword ptr y2
        push dword ptr x2
        push dword ptr y1
        push dword ptr x1
        push dword ptr y0
        push dword ptr x0
        call ccw_native_body_00c377f0
        mov result, eax
    }
    return result;
}

__declspec(noinline) void __cdecl barycentric_2d_ambient_00c37860(
    float* u_out, float* v_out, float* w_out,
    float x0, float y0, float x1, float y1,
    float x2, float y2, float px, float py) noexcept {
    __asm {
        push dword ptr py
        push dword ptr px
        push dword ptr y2
        push dword ptr x2
        push dword ptr y1
        push dword ptr x1
        push dword ptr y0
        push dword ptr x0
        mov eax, u_out
        mov ecx, v_out
        mov edx, w_out
        call barycentric_native_body_00c37860
    }
}

#pragma optimize("", on)
#pragma runtime_checks("", restore)

}  // namespace bsp::cc16_dyn_ambient_math
