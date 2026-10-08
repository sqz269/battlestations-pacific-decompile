#include "bsp/native_unit_wake_fill.hpp"

namespace bsp {
namespace {
// Exact immutable Original payloads. The two angle doubles are promoted
// binary32 values, not the nearest binary64 mathematical pi constants.
const unsigned __int64 half_pi_bits = 0x3ff921fb60000000ull; // CE3830
const unsigned __int64 two_pi_bits = 0x401921fb60000000ull;  // CE3828
const unsigned __int64 step_double_bits = 0x4049000000000000ull; // CE3938
const unsigned int segment_float_bits = 0x42480000u; // D09290

// Dummy EDX keeps position/heading in the original two stack slots. All
// 85 instructions are preserved; notably 0081008C encodes DC C9 and writes
// ST(1), not ST(0). The native kernel has no calls or mutable globals.
__declspec(naked) void __fastcall fill_wake_kernel(void*, void*,
    const float*, float) {
    __asm {
        sub esp, 0x24                                                     // 00810020
        fld dword ptr [esp + 0x2c]                                        // 00810023
        mov byte ptr [ecx + 0x3cc], 0                                     // 00810027
        fsubr qword ptr [half_pi_bits]                                    // 0081002e
        mov dword ptr [ecx + 0x3c8], 0x27                                 // 00810034
        fstp dword ptr [esp]                                              // 0081003e
        fld dword ptr [esp]                                               // 00810041
        fldz                                                              // 00810044
        fcomip st(0), st(1)                                               // 00810046
        jbe native_00810055                                               // 00810048
        fadd qword ptr [two_pi_bits]                                      // 0081004a
        fstp dword ptr [esp]                                              // 00810050
        jmp native_00810057                                               // 00810053
    native_00810055:
        fstp st(0)                                                        // 00810055
    native_00810057:
        push esi                                                          // 00810057
        fld dword ptr [esp + 4]                                           // 00810058
        fcos                                                              // 0081005c
        fstp dword ptr [esp + 8]                                          // 0081005e
        fld dword ptr [esp + 8]                                           // 00810062
        fstp dword ptr [esp + 0x1c]                                       // 00810066
        fld dword ptr [esp + 4]                                           // 0081006a
        fsin                                                              // 0081006e
        fstp dword ptr [esp + 0xc]                                        // 00810070
        fld dword ptr [esp + 0x1c]                                        // 00810074
        mov eax, dword ptr [esp + 0x2c]                                   // 00810078
        fld qword ptr [step_double_bits]                                  // 0081007c
        movss xmm0, dword ptr [eax]                                       // 00810082
        mov edx, dword ptr [ecx + 0x3c8]                                  // 00810086
        fmul st(1), st(0)                                                 // 0081008c
        movss xmm2, dword ptr [esp + 0x30]                                // 0081008e
        fxch st(1)                                                        // 00810094
        xorps xmm1, xmm1                                                  // 00810096
        fstp dword ptr [esp + 0x1c]                                       // 00810099
        movss dword ptr [esp + 0x10], xmm0                                // 0081009d
        fldz                                                              // 008100a3
        movss xmm0, dword ptr [eax + 4]                                   // 008100a5
        fmul st(0), st(1)                                                 // 008100aa
        movss dword ptr [esp + 0x14], xmm0                                // 008100ac
        movss xmm0, dword ptr [eax + 8]                                   // 008100b2
        movss dword ptr [esp + 0x18], xmm0                                // 008100b7
        xor esi, esi                                                      // 008100bd
        fstp dword ptr [esp + 0x20]                                       // 008100bf
        fmul dword ptr [esp + 0xc]                                        // 008100c3
        fstp dword ptr [esp + 0x24]                                       // 008100c7
        fld dword ptr [esp + 0x24]                                        // 008100cb
        fld dword ptr [esp + 0x20]                                        // 008100cf
        fld dword ptr [esp + 0x1c]                                        // 008100d3
    native_008100d7:
        test esi, esi                                                     // 008100d7
        lea eax, [edx + edx*2]                                            // 008100d9
        lea eax, [ecx + eax*8]                                            // 008100dc
        movss dword ptr [eax + 0x14], xmm2                                // 008100df
        jne native_008100eb                                               // 008100e4
        movaps xmm0, xmm1                                                 // 008100e6
        jmp native_008100f3                                               // 008100e9
    native_008100eb:
        movss xmm0, dword ptr [segment_float_bits]                        // 008100eb
    native_008100f3:
        test edx, edx                                                     // 008100f3
        fld dword ptr [esp + 0x10]                                        // 008100f5
        fsub st(0), st(1)                                                 // 008100f9
        movss dword ptr [eax + 0x18], xmm0                                // 008100fb
        movss xmm0, dword ptr [esp + 0x10]                                // 00810100
        movss dword ptr [eax + 8], xmm0                                   // 00810106
        movss xmm0, dword ptr [esp + 0x14]                                // 0081010b
        fstp dword ptr [esp + 0x10]                                       // 00810111
        fld dword ptr [esp + 0x14]                                        // 00810115
        movss dword ptr [eax + 0xc], xmm0                                 // 00810119
        movss xmm0, dword ptr [esp + 0x18]                                // 0081011e
        fsub st(0), st(2)                                                 // 00810124
        movss dword ptr [eax + 0x10], xmm0                                // 00810126
        fstp dword ptr [esp + 0x14]                                       // 0081012b
        fld dword ptr [esp + 0x18]                                        // 0081012f
        fsub st(0), st(3)                                                 // 00810133
        fstp dword ptr [esp + 0x18]                                       // 00810135
        jne native_00810142                                               // 00810139
        mov edx, 0x27                                                     // 0081013b
        jmp native_00810145                                               // 00810140
    native_00810142:
        sub edx, 1                                                        // 00810142
    native_00810145:
        add esi, 1                                                        // 00810145
        cmp esi, 0x28                                                     // 00810148
        jl native_008100d7                                                // 0081014b
        fstp st(2)                                                        // 0081014d
        pop esi                                                           // 0081014f
        fstp st(0)                                                        // 00810150
        fstp st(0)                                                        // 00810152
        add esp, 0x24                                                     // 00810154
        ret 8                                                             // 00810157
    }
}
} // namespace

void fill_native_unit_wake_00810020(void* actual_wake,
    const float* position, float heading) {
    fill_wake_kernel(actual_wake, nullptr, position, heading);
}

} // namespace bsp
