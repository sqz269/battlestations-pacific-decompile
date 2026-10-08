#include "bsp/native_unit_wake_fill.hpp"
#include "bsp/game_native_geometry_globals.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"

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

namespace {
// Complete 00815600 operation order. ECX is the borrowed actual wake; EDX
// supplies the genuine canonical seed address. The additional PUSH/POP save
// that address across both actual calls. The saved word is above the outgoing
// fill arguments: PUSH [ESP+4] reads it before the PUSH decrements ESP.
// Native heading storage, both callee argument positions and the original
// ESI save/restore are retained. No seed values are snapshotted or substituted.
__declspec(naked) void* __fastcall construct_wake_kernel_00815600(
    void*, const std::uint32_t*) {
    __asm {
        xorps xmm0, xmm0                            // 00815600
        push esi                                    // 00815603
        push edx                                    // Added: save live seed address.
        mov esi, ecx                                // 00815604
        mov dword ptr [esi], 0x00d09480              // 00815606
        mov ecx, 0x27                               // 0081560C
        lea eax, [esi + 0x18]                       // 00815611
    wake_construct_00815614:
        movss dword ptr [eax - 4], xmm0             // 00815614
        movss dword ptr [eax], xmm0                 // 00815619
        add eax, 0x18                               // 0081561D
        sub ecx, 1                                  // 00815620
        jns wake_construct_00815614                 // 00815623
        call create_native_tracked_critical_section_00bd1860 // 00815625
        fldz                                        // 0081562A
        push ecx                                    // 0081562C
        fstp dword ptr [esp]                        // 0081562D
        push dword ptr [esp + 4]                    // 00815630: actual seed pointer.
        mov ecx, esi                                // 00815635
        mov dword ptr [esi + 4], eax                // 00815637
        call fill_wake_kernel                      // 0081563A: actual same-TU raw body.
        pop edx                                     // Added: reload address and restore ESP.
        movss xmm0, dword ptr [edx]                 // 0081563F: fresh seed X.
        movss dword ptr [esi + 0x3d0], xmm0          // 00815647
        movss xmm0, dword ptr [edx + 4]             // 0081564F: fresh seed Y.
        movss dword ptr [esi + 0x3d4], xmm0          // 00815657
        movss xmm0, dword ptr [edx + 8]             // 0081565F: fresh seed Z.
        movss dword ptr [esi + 0x3d8], xmm0          // 00815667
        mov eax, esi                                // 0081566F
        pop esi                                     // 00815671
        ret                                         // 00815672
    }
}
} // namespace

void* construct_native_unit_wake_00815600(void* actual_wake,
    game::GameNativeGeometryGlobals& actual_geometry_globals) {
    return construct_wake_kernel_00815600(actual_wake,
        actual_geometry_globals.zero_vector_00f87574().data());
}

// Complete 00818100..0081810D: only the real Source release target relocates.
// The literal Native vptr word is retained; this creates no Source vtable or
// C++ owner and grants no permission to release the wake allocation itself.
__declspec(naked) void __fastcall destroy_native_unit_wake_00818100(void*) {
    __asm {
        mov dword ptr [ecx], 0x00d09480              // 00818100
        add ecx, 4                                  // 00818106
        jmp release_native_tracked_critical_section_0041cc80 // 00818109
    }
}

} // namespace bsp
