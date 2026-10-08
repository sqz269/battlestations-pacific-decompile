#include "bsp/native_unit_wake_append_dependencies.hpp"
#include "bsp/game_native_geometry_globals.hpp"
#include "bsp/vector_helpers.hpp"
#include <cstdint>
#include <cstring>

// Genuine current Win32 CRT intrinsic entry, already used by the exact raw
// 00414C60 kernel and camera decomposition. Inline CALL passes/returns ST0;
// this declaration is not an ordinary stack-argument sqrt adapter.
extern "C" double __cdecl _CIsqrt();
extern "C" double __cdecl _CIatan2();

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

namespace {
// Exact Original read-only payloads, including promoted binary32 pi values.
const float append_distance_squared_00ce6454 = 16.0f; // 00008041
const double append_decay_00d7a348 = 0.25; // 000000000000d03f
const double append_half_pi_00ce3830 = 1.57079637050628662109375; // 00000060fb21f93f
const double append_two_pi_00ce3828 = 6.283185482025146484375; // 00000060fb211940
const double append_merge_span_00d09438 = 55.0; // 0000000000804b40
const double append_advance_00ce3938 = 50.0; // 0000000000004940

// Complete 00810190 operation sequence, with the accepted borrowed-owner
// adaptation: three added MOVs and six reset-address operand replacements.
// ECX=wake, EDX=canonical live reset cells, stack=position/heading/yaw bits.
// The saved address occupies entry ESP-20, unused by the complete Original
// direct/indirect stack-access graph. It is never a snapshot of reset values.
// Labels and address comments follow Original instruction boundaries; the
// compiled body is fully mapped in CC12_NATIVE_WAKE_APPEND_SOURCE.md.
__declspec(naked) void __fastcall append_kernel_00810190(
    void*, const std::uint32_t*, const float*, std::uint32_t, std::uint32_t) {
    __asm {
        sub esp, 0x3c // 00810190
        mov dword ptr [esp + 0x28], edx // Added: save genuine reset address (entry ESP-20).
        push ebx // 00810193
        mov ebx, dword ptr [esp + 0x44] // 00810194
        fld dword ptr [ebx] // 00810198
        push esi // 0081019A
        mov esi, ecx // 0081019B
        fadd dword ptr [esi + 0x3d0] // 0081019D
        mov eax, dword ptr [esi + 0x3c8] // 008101A3
        push edi // 008101A9
        lea edi, [esi + 0x3d0] // 008101AA
        fstp dword ptr [esp + 0x18] // 008101B0
        lea eax, [eax + eax*2] // 008101B4
        fld dword ptr [edi + 4] // 008101B7
        movss xmm0, dword ptr [esi + eax*8 + 8] // 008101BA
        fadd dword ptr [ebx + 4] // 008101C0
        lea eax, [esi + eax*8 + 8] // 008101C3
        movss dword ptr [esp + 0x30], xmm0 // 008101C7
        movss xmm0, dword ptr [eax + 8] // 008101CD
        fstp dword ptr [esp + 0x1c] // 008101D2
        movss dword ptr [esp + 0x38], xmm0 // 008101D6
        fld dword ptr [edi + 8] // 008101DC
        xorps xmm0, xmm0 // 008101DF
        fadd dword ptr [ebx + 8] // 008101E2
        movss dword ptr [esp + 0x28], xmm0 // 008101E5
        fstp dword ptr [esp + 0x20] // 008101EB
        fld dword ptr [esp + 0x18] // 008101EF
        fsub dword ptr [esp + 0x30] // 008101F3
        fstp dword ptr [esp + 0x24] // 008101F7
        fld dword ptr [esp + 0x20] // 008101FB
        fsub dword ptr [esp + 0x38] // 008101FF
        fstp dword ptr [esp + 0x2c] // 00810203
        fld dword ptr [esp + 0x24] // 00810207
        fld dword ptr [esp + 0x2c] // 0081020B
        fld st(1) // 0081020F
        fmulp st(2), st(0) // 00810211
        fldz // 00810213
        fmul st(0), st(0) // 00810215
        faddp st(2), st(0) // 00810217
        fmul st(0), st(0) // 00810219
        faddp st(1), st(0) // 0081021B
        fstp dword ptr [esp + 0x4c] // 0081021D
        fld dword ptr [esp + 0x4c] // 00810221
        fld dword ptr [append_distance_squared_00ce6454] // 00810225
        fcomip st(0), st(1) // 0081022B
        fstp st(0) // 0081022D
        ja wake_00810626 // 0081022F
        mov edx, dword ptr [esp + 0x34] // Added: reload genuine reset address (entry ESP-20).
        fld dword ptr [edx] // 00810235
        fld dword ptr [edi] // 0081023B
        fucomip st(0), st(1) // 0081023D
        fstp st(0) // 0081023F
        lahf // 00810241
        test ah, 0x44 // 00810242
        jp wake_00810271 // 00810245
        fld dword ptr [edx + 4] // 00810247
        fld dword ptr [edi + 4] // 0081024D
        fucomip st(0), st(1) // 00810250
        fstp st(0) // 00810252
        lahf // 00810254
        test ah, 0x44 // 00810255
        jp wake_00810271 // 00810258
        fld dword ptr [edx + 8] // 0081025A
        fld dword ptr [edi + 8] // 00810260
        fucomip st(0), st(1) // 00810263
        fstp st(0) // 00810265
        lahf // 00810267
        test ah, 0x44 // 00810268
        jnp wake_0081032a // 0081026B
    wake_00810271:
        mov ecx, edi // 00810271
        call native_unit_wake_length_0042b2f0 // 00810273
        fstp dword ptr [esp + 0xc] // 00810278
        fld dword ptr [esp + 0x4c] // 0081027C
        call _CIsqrt // 00810280
        fstp dword ptr [esp + 0x4c] // 00810285
        fld dword ptr [esp + 0x4c] // 00810289
        fmul qword ptr [append_decay_00d7a348] // 0081028D
        fstp dword ptr [esp + 0x4c] // 00810293
        fld dword ptr [esp + 0xc] // 00810297
        fld dword ptr [esp + 0x4c] // 0081029B
        fcomi st(0), st(1) // 0081029F
        jb wake_008102cf // 008102A1
        mov edx, dword ptr [esp + 0x34] // Added: reload genuine reset address (entry ESP-20).
        movss xmm0, dword ptr [edx] // 008102A3
        fstp st(1) // 008102AB
        movss dword ptr [edi], xmm0 // 008102AD
        fstp st(0) // 008102B1
        movss xmm0, dword ptr [edx + 4] // 008102B3
        movss dword ptr [edi + 4], xmm0 // 008102BB
        movss xmm0, dword ptr [edx + 8] // 008102C0
        movss dword ptr [edi + 8], xmm0 // 008102C8
        jmp wake_008102f3 // 008102CD
    wake_008102cf:
        fsubr st(0), st(1) // 008102CF
        fdivrp st(1), st(0) // 008102D1
        fstp dword ptr [esp + 0x4c] // 008102D3
        fld dword ptr [edi] // 008102D7
        fld dword ptr [esp + 0x4c] // 008102D9
        fld st(0) // 008102DD
        fmulp st(2), st(0) // 008102DF
        fxch st(1) // 008102E1
        fstp dword ptr [edi] // 008102E3
        fld dword ptr [edi + 4] // 008102E5
        fmul st(0), st(1) // 008102E8
        fstp dword ptr [edi + 4] // 008102EA
        fmul dword ptr [edi + 8] // 008102ED
        fstp dword ptr [edi + 8] // 008102F0
    wake_008102f3:
        fld dword ptr [ebx] // 008102F3
        xorps xmm0, xmm0 // 008102F5
        fadd dword ptr [edi] // 008102F8
        fstp dword ptr [esp + 0xc] // 008102FA
        fld dword ptr [edi + 4] // 008102FE
        fadd dword ptr [ebx + 4] // 00810301
        fstp dword ptr [esp + 0x10] // 00810304
        fld dword ptr [edi + 8] // 00810308
        fadd dword ptr [ebx + 8] // 0081030B
        fstp dword ptr [esp + 0x14] // 0081030E
        fld dword ptr [esp + 0xc] // 00810312
        fstp dword ptr [esp + 0x18] // 00810316
        fld dword ptr [esp + 0x10] // 0081031A
        fstp dword ptr [esp + 0x1c] // 0081031E
        fld dword ptr [esp + 0x14] // 00810322
        fstp dword ptr [esp + 0x20] // 00810326
    wake_0081032a:
        mov eax, dword ptr [esi + 0x3c8] // 0081032A
        test eax, eax // 00810330
        push ebp // 00810332
        jg wake_0081033c // 00810333
        mov ebp, 0x27 // 00810335
        jmp wake_00810348 // 0081033A
    wake_0081033c:
        lea ebp, [eax - 1] // 0081033C
        test ebp, ebp // 0081033F
        mov ecx, 0x27 // 00810341
        jle wake_0081034b // 00810346
    wake_00810348:
        lea ecx, [ebp - 1] // 00810348
    wake_0081034b:
        movss xmm1, dword ptr [esp + 0x1c] // 0081034B
        lea edx, [eax + eax*2] // 00810351
        movss dword ptr [esi + edx*8 + 8], xmm1 // 00810354
        movss xmm1, dword ptr [esp + 0x20] // 0081035A
        movss dword ptr [esi + edx*8 + 0xc], xmm1 // 00810360
        movss xmm1, dword ptr [esp + 0x24] // 00810366
        movss dword ptr [esi + edx*8 + 0x10], xmm1 // 0081036C
        movss xmm1, dword ptr [esp + 0x54] // 00810372
        lea eax, [esi + edx*8 + 8] // 00810378
        mov eax, dword ptr [esi + 0x3c8] // 0081037C
        add eax, 1 // 00810382
        lea eax, [eax + eax*2] // 00810385
        movss dword ptr [esi + eax*8], xmm0 // 00810388
        mov eax, dword ptr [esi + 0x3c8] // 0081038D
        lea edx, [eax + eax*2] // 00810393
        movss dword ptr [esi + edx*8 + 0x14], xmm1 // 00810396
        mov eax, dword ptr [esi + 0x3c8] // 0081039C
        movss xmm1, dword ptr [esp + 0x58] // 008103A2
        lea eax, [eax + eax*2] // 008103A8
        movss dword ptr [esi + eax*8 + 0x1c], xmm1 // 008103AB
        lea edx, [ebp + ebp*2] // 008103B1
        fld dword ptr [esi + edx*8 + 8] // 008103B5
        lea edi, [esi + edx*8] // 008103B9
        fstp dword ptr [esp + 0x10] // 008103BC
        lea eax, [ecx + ecx*2] // 008103C0
        fld dword ptr [edi + 0x10] // 008103C3
        movss xmm1, dword ptr [esi + eax*8 + 8] // 008103C6
        fstp dword ptr [esp + 0x18] // 008103CC
        lea ebx, [esi + eax*8] // 008103D0
        fld dword ptr [esp + 0x1c] // 008103D3
        movss dword ptr [esp + 0x34], xmm1 // 008103D7
        fsub dword ptr [esp + 0x10] // 008103DD
        movss xmm1, dword ptr [ebx + 0x10] // 008103E1
        lea ecx, [esp + 0x28] // 008103E6
        movss dword ptr [esp + 0x3c], xmm1 // 008103EA
        fstp dword ptr [esp + 0x40] // 008103F0
        movss dword ptr [esp + 0x2c], xmm0 // 008103F4
        fld dword ptr [esp + 0x24] // 008103FA
        fsub dword ptr [esp + 0x18] // 008103FE
        fstp dword ptr [esp + 0x48] // 00810402
        fld dword ptr [esp + 0x40] // 00810406
        fstp dword ptr [esp + 0x28] // 0081040A
        fld dword ptr [esp + 0x48] // 0081040E
        fstp dword ptr [esp + 0x30] // 00810412
        call native_unit_wake_length_0042b2f0 // 00810416
        fstp dword ptr [esp + 0x50] // 0081041B
        fld dword ptr [esp + 0x24] // 0081041F
        fsub dword ptr [esp + 0x3c] // 00810423
        fstp dword ptr [esp + 0x10] // 00810427
        fld dword ptr [esp + 0x10] // 0081042B
        fld dword ptr [esp + 0x1c] // 0081042F
        fsub dword ptr [esp + 0x34] // 00810433
        fstp dword ptr [esp + 0x10] // 00810437
        fld dword ptr [esp + 0x10] // 0081043B
        call _CIatan2 // 0081043F
        fstp dword ptr [esp + 0x10] // 00810444
        fld dword ptr [esp + 0x10] // 00810448
        fsubr qword ptr [append_half_pi_00ce3830] // 0081044C
        fstp dword ptr [esp + 0x10] // 00810452
        fld dword ptr [esp + 0x10] // 00810456
        fldz // 0081045A
        fcomip st(0), st(1) // 0081045C
        jbe wake_0081046c // 0081045E
        fadd qword ptr [append_two_pi_00ce3828] // 00810460
        fstp dword ptr [esp + 0x10] // 00810466
        jmp wake_0081046e // 0081046A
    wake_0081046c:
        fstp st(0) // 0081046C
    wake_0081046e:
        fld dword ptr [esp + 0x50] // 0081046E
        movss xmm0, dword ptr [esp + 0x10] // 00810472
        fld dword ptr [edi + 0x18] // 00810478
        movss dword ptr [edi + 0x14], xmm0 // 0081047B
        fcomip st(0), st(1) // 00810480
        fstp st(0) // 00810482
        jbe wake_0081049d // 00810484
        movss xmm0, dword ptr [esp + 0x50] // 00810486
        mov byte ptr [esi + 0x3cc], 1 // 0081048C
        movss dword ptr [edi + 0x18], xmm0 // 00810493
        jmp wake_00810585 // 00810498
    wake_0081049d:
        cmp byte ptr [esi + 0x3cc], 0 // 0081049D
        jne wake_008104b6 // 008104A4
        movss xmm0, dword ptr [esp + 0x50] // 008104A6
        movss dword ptr [edi + 0x18], xmm0 // 008104AC
        jmp wake_00810585 // 008104B1
    wake_008104b6:
        fld dword ptr [esp + 0x34] // 008104B6
        lea ecx, [esp + 0x10] // 008104BA
        fsub dword ptr [esp + 0x1c] // 008104BE
        fstp dword ptr [esp + 0x40] // 008104C2
        fld dword ptr [esp + 0x3c] // 008104C6
        fsub dword ptr [esp + 0x24] // 008104CA
        fstp dword ptr [esp + 0x48] // 008104CE
        fld dword ptr [esp + 0x40] // 008104D2
        fstp dword ptr [esp + 0x10] // 008104D6
        fld dword ptr [esp + 0x48] // 008104DA
        fstp dword ptr [esp + 0x14] // 008104DE
        call raw_length_2d_00414c60 // 008104E2
        fstp dword ptr [esp + 0x10] // 008104E7
        fld dword ptr [esp + 0x10] // 008104EB
        fld qword ptr [append_merge_span_00d09438] // 008104EF
        fcomip st(0), st(1) // 008104F5
        fstp st(0) // 008104F7
        jbe wake_00810573 // 008104F9
        movss xmm0, dword ptr [esp + 0x10] // 008104FB
        movss dword ptr [ebx + 0x18], xmm0 // 00810501
        movss xmm0, dword ptr [esp + 0x1c] // 00810506
        movss dword ptr [edi + 8], xmm0 // 0081050C
        movss xmm0, dword ptr [esp + 0x20] // 00810511
        movss dword ptr [edi + 0xc], xmm0 // 00810517
        movss xmm0, dword ptr [esp + 0x24] // 0081051C
        movss dword ptr [edi + 0x10], xmm0 // 00810522
        xorps xmm0, xmm0 // 00810527
        movss dword ptr [edi + 0x18], xmm0 // 0081052A
        movss xmm0, dword ptr [esp + 0x54] // 0081052F
        movss dword ptr [edi + 0x14], xmm0 // 00810535
        movss xmm0, dword ptr [esp + 0x58] // 0081053A
        movss dword ptr [edi + 0x1c], xmm0 // 00810540
        mov eax, dword ptr [esi + 0x3c8] // 00810545
        cmp eax, 0x27 // 0081054B
        jl wake_00810554 // 0081054E
        xor ecx, ecx // 00810550
        jmp wake_00810557 // 00810552
    wake_00810554:
        lea ecx, [eax + 1] // 00810554
    wake_00810557:
        lea ecx, [ecx + ecx*2] // 00810557
        lea edx, [esi + ecx*8 + 8] // 0081055A
        lea eax, [eax + eax*2] // 0081055E
        push edx // 00810561
        lea ecx, [esi + eax*8 + 8] // 00810562
        call copy_native_unit_wake_sample_00810160 // 00810566
        mov dword ptr [esi + 0x3c8], ebp // 0081056B
        jmp wake_00810585 // 00810571
    wake_00810573:
        movss xmm0, dword ptr [esp + 0x50] // 00810573
        movss dword ptr [edi + 0x18], xmm0 // 00810579
        mov byte ptr [esi + 0x3cc], 0 // 0081057E
    wake_00810585:
        fld qword ptr [append_advance_00ce3938] // 00810585
        pop ebp // 0081058B
        fld dword ptr [edi + 0x18] // 0081058C
        fcomip st(0), st(1) // 0081058F
        fstp st(0) // 00810591
        jbe wake_00810626 // 00810593
        cmp byte ptr [esi + 0x3cc], 0 // 00810599
        jne wake_00810626 // 008105A0
        mov eax, dword ptr [esi + 0x3c8] // 008105A6
        cmp eax, 0x27 // 008105AC
        jl wake_008105b5 // 008105AF
        xor eax, eax // 008105B1
        jmp wake_008105b8 // 008105B3
    wake_008105b5:
        add eax, 1 // 008105B5
    wake_008105b8:
        movss xmm0, dword ptr [esp + 0x18] // 008105B8
        mov dword ptr [esi + 0x3c8], eax // 008105BE
        lea ecx, [eax + eax*2] // 008105C4
        movss dword ptr [esi + ecx*8 + 8], xmm0 // 008105C7
        movss xmm0, dword ptr [esp + 0x1c] // 008105CD
        movss dword ptr [esi + ecx*8 + 0xc], xmm0 // 008105D3
        movss xmm0, dword ptr [esp + 0x20] // 008105D9
        lea eax, [esi + ecx*8 + 8] // 008105DF
        movss dword ptr [eax + 8], xmm0 // 008105E3
        mov eax, dword ptr [esi + 0x3c8] // 008105E8
        xorps xmm0, xmm0 // 008105EE
        add eax, 1 // 008105F1
        lea edx, [eax + eax*2] // 008105F4
        movss dword ptr [esi + edx*8], xmm0 // 008105F7
        mov eax, dword ptr [esi + 0x3c8] // 008105FC
        movss xmm0, dword ptr [esp + 0x50] // 00810602
        lea eax, [eax + eax*2] // 00810608
        movss dword ptr [esi + eax*8 + 0x14], xmm0 // 0081060B
        mov eax, dword ptr [esi + 0x3c8] // 00810611
        movss xmm0, dword ptr [esp + 0x54] // 00810617
        lea ecx, [eax + eax*2] // 0081061D
        movss dword ptr [esi + ecx*8 + 0x1c], xmm0 // 00810620
    wake_00810626:
        pop edi // 00810626
        pop esi // 00810627
        pop ebx // 00810628
        add esp, 0x3c // 00810629
        ret 0xc // 0081062C
    }
}
} // namespace

void append_native_unit_wake_00810190(
    void* actual_wake, const float* actual_world_position,
    float heading, float yaw_rate,
    game::GameNativeGeometryGlobals& actual_geometry_globals) {
    std::uint32_t heading_bits;
    std::uint32_t yaw_bits;
    std::memcpy(&heading_bits, &heading, sizeof(heading_bits));
    std::memcpy(&yaw_bits, &yaw_rate, sizeof(yaw_bits));
    append_kernel_00810190(
        actual_wake, actual_geometry_globals.zero_vector_00f87574().data(),
        actual_world_position, heading_bits, yaw_bits);
}

} // namespace bsp
