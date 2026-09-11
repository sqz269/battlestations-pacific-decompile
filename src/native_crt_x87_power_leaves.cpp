#include "bsp/native_crt_x87_power_leaves.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT x87 power leaves require MSVC Win32 raw assembly.
#endif

namespace bsp {

// Original currently LIBCRT_unmatched_00c08390; this source name is descriptive.
// Complete original sequence. Original x87 input/output ABI is assembly-only.
__declspec(naked) void __cdecl exp2_native_crt_x87_00c08390() {
    __asm {
        fld st(0) // 00c08390
        frndint // 00c08392
        fsubr st(1), st(0) // 00c08394
        fxch st(1) // 00c08396
        fchs // 00c08398
        f2xm1 // 00c0839a
        fld1 // 00c0839c
        faddp st(1), st(0) // 00c0839e
        fscale // 00c083a0
        fstp st(1) // 00c083a2
        ret // 00c083a4
    }
}

// Original currently LIBCRT_unmatched_00bfed32; name is a bounded hypothesis.
// Preserve all original offsets, waiting FSTSWs, register widths and branches.
__declspec(naked) void __fastcall
classify_native_crt_power_exponent_00bfed32(void*, const void*) {
    __asm {
        fld st(0) // 00bfed32
        frndint // 00bfed34
        fcomp st(1) // 00bfed36
        mov cl, 0 // 00bfed38
        fstsw ax // 00bfed3a: original 9B DF E0, including WAIT
        sahf // 00bfed3d
        jnz noninteger // 00bfed3e
        // 00bfed40: original DC 0D 50 58 E1 00, FMUL qword [00E15850].
        // Sole operand adaptation: DC 8A 00 00 00 00, FMUL qword [EDX+0].
        // Explicit disp32 preserves the six-byte length and both jump offsets.
        _emit 0dch
        _emit 08ah
        _emit 0
        _emit 0
        _emit 0
        _emit 0
        inc cl // 00bfed46
        fld st(0) // 00bfed48
        frndint // 00bfed4a
        fcompp // 00bfed4c
        fstsw ax // 00bfed4e: original 9B DF E0, including WAIT
        sahf // 00bfed51
        jnz finished // 00bfed52
        inc cl // 00bfed54
    finished:
        ret // 00bfed56
    noninteger:
        fstp st(0) // 00bfed57
        ret // 00bfed59
    }
}

} // namespace bsp
