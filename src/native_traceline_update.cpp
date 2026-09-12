#include "bsp/native_traceline_update.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/system_camera_axes.hpp"
#include "bsp/system_time_constants.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native traceline update requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(offsetof(NativeTracelineUpdateAccess,crt)==0);
static_assert(offsetof(NativeTracelineUpdateAccess,truncate_mode_0109eea4)==4);
static_assert(offsetof(NativeTracelineUpdateAccess,negative_zero_00d7a208)==8);
static_assert(offsetof(NativeTracelineUpdateAccess,percent_00d7a220)==12);
static_assert(offsetof(NativeTracelineUpdateAccess,direction_00e13028)==16);
static_assert(offsetof(NativeTracelineUpdateAccess,parameters)==20);
static_assert(offsetof(NativeTracelineUpdateAccess,clock_01090ab0)==24);
static_assert(offsetof(NativeTracelineUpdateAccess,clock_virtuals)==28);
static_assert(sizeof(ClockTimestamp)==16 && offsetof(ClockTimestamp,frequency)==8);
const ClockTimestamp* __fastcall interval_bridge(const NativeTracelineUpdateAccess* a) {
    FrameClock* const clock=*a->clock_01090ab0;
    return a->clock_virtuals->interval_1c(*clock);
}
} // namespace

__declspec(naked) void __fastcall set_native_traceline_point_00af2630(
    void*,const NativeTracelineUpdateAccess*,const float*,float) {
    __asm {
        push edx // added access argument to the native fastcall helper
        fld dword ptr [esp+0ch] // AF2630 original interval at ESP+8
        mov edx,dword ptr [esp+8] // AF2634 original point at ESP+4
        push ecx // AF2638
        fstp dword ptr [esp] // AF2639
        call append_native_traceline_point_00af22b0 // AF263C
        ret 8 // AF2641
    }
}

// Complete native instruction schedule; only current bindings/addresses change.
__declspec(naked) void __fastcall append_native_traceline_point_00af22b0(void*,const float*,float,const NativeTracelineUpdateAccess*) {
    __asm {
        sub esp, 0x44 // 00af22b0
        push ebx // 00af22b3
        push ebp // 00af22b4
        push esi // 00af22b5
        mov esi, ecx // 00af22b6
        mov ebx, dword ptr [esi + 0x184] // 00af22b8
        mov ecx, dword ptr [esi + 0x190] // 00af22be
        cmp ecx, dword ptr [ebx + 0x20] // 00af22c4
        mov ebp, edx // 00af22c7
        je l_00af2618 // 00af22c9
        cmp byte ptr [esi + 0x194], 0 // 00af22cf
        jne l_00af2618 // 00af22d6
        cmp byte ptr [esi + 0x195], 0 // 00af22dc
        jne l_00af2618 // 00af22e3
        test ecx, ecx // 00af22e9
        jne l_00af2363 // 00af22eb
        mov eax, dword ptr [esi + 0x18c] // 00af22ed
        mov ecx, dword ptr [esi + 0x188] // 00af22f3
        push edx
        mov edx,dword ptr [esp+0x5c]
        mov edx,dword ptr [edx+8]
        movss xmm0, dword ptr [edx] // 00af22f9
        pop edx
        subss xmm0, dword ptr [esp + 0x54] // 00af2301
        mov dword ptr [esi + 0x190], 1 // 00af2307
        fld dword ptr [ebp] // 00af2311
        lea eax, [eax + eax*4] // 00af2314
        fstp dword ptr [ecx + eax*4] // 00af2317
        lea eax, [ecx + eax*4] // 00af231a
        fld dword ptr [ebp + 4] // 00af231d
        fstp dword ptr [eax + 4] // 00af2320
        fld dword ptr [ebp + 8] // 00af2323
        fstp dword ptr [eax + 8] // 00af2326
        mov eax, dword ptr [esi + 0x18c] // 00af2329
        lea edx, [eax + eax*4] // 00af232f
        mov eax, dword ptr [esi + 0x188] // 00af2332
        movss dword ptr [eax + edx*4 + 0x10], xmm0 // 00af2338
        mov eax, dword ptr [esi + 0x18c] // 00af233e
        mov ecx, dword ptr [esi + 0x184] // 00af2344
        fld dword ptr [ecx + 0x14] // 00af234a
        lea edx, [eax + eax*4] // 00af234d
        mov eax, dword ptr [esi + 0x188] // 00af2350
        pop esi // 00af2356
        fstp dword ptr [eax + edx*4 + 0xc] // 00af2357
        pop ebp // 00af235b
        pop ebx // 00af235c
        add esp, 0x44 // 00af235d
        ret 8 // 00af2360
    l_00af2363:
        cmp ecx, 1 // 00af2363
        push edi // 00af2366
        jne l_00af2371 // 00af2367
        mov edi, dword ptr [esi + 0x18c] // 00af2369
        jmp l_00af238f // 00af236f
    l_00af2371:
        mov edi, dword ptr [ebx + 0x20] // 00af2371
        mov eax, dword ptr [esi + 0x18c] // 00af2374
        lea edx, [edi + ecx] // 00af237a
        lea eax, [edx + eax - 2] // 00af237d
        cdq  // 00af2381
        idiv edi // 00af2382
        add ecx, -1 // 00af2384
        mov dword ptr [esi + 0x190], ecx // 00af2387
        mov edi, edx // 00af238d
    l_00af238f:
        mov edx, dword ptr [esi + 0x188] // 00af238f
        lea ecx, [edi + edi*4] // 00af2395
        fld dword ptr [edx + ecx*4] // 00af2398
        lea eax, [edx + ecx*4] // 00af239b
        fstp dword ptr [esp + 0x40] // 00af239e
        movss xmm0, dword ptr [eax + 0xc] // 00af23a2
        fld dword ptr [eax + 4] // 00af23a7
        movss dword ptr [esp + 0x4c], xmm0 // 00af23aa
        fstp dword ptr [esp + 0x44] // 00af23b0
        movss xmm0, dword ptr [eax + 0x10] // 00af23b4
        fld dword ptr [eax + 8] // 00af23b9
        lea ecx, [esp + 0x28] // 00af23bc
        fstp dword ptr [esp + 0x48] // 00af23c0
        movss dword ptr [esp + 0x50], xmm0 // 00af23c4
        fld dword ptr [ebp] // 00af23ca
        fsub dword ptr [esp + 0x40] // 00af23cd
        fstp dword ptr [esp + 0x1c] // 00af23d1
        fld dword ptr [ebp + 4] // 00af23d5
        fsub dword ptr [esp + 0x44] // 00af23d8
        fstp dword ptr [esp + 0x20] // 00af23dc
        fld dword ptr [ebp + 8] // 00af23e0
        fsub dword ptr [esp + 0x48] // 00af23e3
        fstp dword ptr [esp + 0x24] // 00af23e7
        fld dword ptr [esp + 0x1c] // 00af23eb
        fstp dword ptr [esp + 0x28] // 00af23ef
        fld dword ptr [esp + 0x20] // 00af23f3
        fstp dword ptr [esp + 0x2c] // 00af23f7
        fld dword ptr [esp + 0x24] // 00af23fb
        fstp dword ptr [esp + 0x30] // 00af23ff
        mov edx,dword ptr [esp+0x5c]
        mov edx,dword ptr [edx]
        call camera_vector_length_00419440 // 00af2403
        fstp dword ptr [esp + 0x10] // 00af2408
        fld dword ptr [esp + 0x1c] // 00af240c
        fld dword ptr [esp + 0x10] // 00af2410
        fld st(0) // 00af2414
        fdivp st(2), st(0) // 00af2416
        fxch st(1) // 00af2418
        fstp dword ptr [esp + 0x28] // 00af241a
        fld dword ptr [esp + 0x20] // 00af241e
        fdiv st(0), st(1) // 00af2422
        fstp dword ptr [esp + 0x2c] // 00af2424
        fld dword ptr [esp + 0x24] // 00af2428
        fdiv st(0), st(1) // 00af242c
        fstp dword ptr [esp + 0x30] // 00af242e
        fld st(0) // 00af2432
        fdiv dword ptr [ebx + 0x24] // 00af2434
        mov ecx,dword ptr [esp+0x5c]
        mov ecx,dword ptr [ecx+4]
        call native_crt_truncate_st0_00bf7420 // 00af2437
        test eax, eax // 00af243c
        mov dword ptr [esp + 0x18], eax // 00af243e
        mov dword ptr [esp + 0x10], 0 // 00af2442
        jle l_00af2585 // 00af244a
        fld dword ptr [esp + 0x58] // 00af2450
        fchs  // 00af2454
        fsub dword ptr [esp + 0x50] // 00af2456
        fstp qword ptr [esp + 0x1c] // 00af245a
        fld dword ptr [esp + 0x30] // 00af245e
        fld dword ptr [esp + 0x2c] // 00af2462
        fld dword ptr [esp + 0x28] // 00af2466
        fld dword ptr [esp + 0x4c] // 00af246a
    l_00af246e:
        fld dword ptr [ebx + 0x24] // 00af246e
        mov eax, 1 // 00af2471
        add dword ptr [esp + 0x10], eax // 00af2476
        fimul dword ptr [esp + 0x10] // 00af247a
        add dword ptr [esi + 0x190], eax // 00af247e
        lea eax, [edi + 1] // 00af2484
        cdq  // 00af2487
        idiv dword ptr [ebx + 0x20] // 00af2488
        fstp dword ptr [esp + 0x14] // 00af248b
        fld dword ptr [esp + 0x14] // 00af248f
        fld st(0) // 00af2493
        fdiv st(0), st(6) // 00af2495
        mov ecx, dword ptr [esi + 0x188] // 00af2497
        fstp dword ptr [esp + 0x14] // 00af249d
        fld st(2) // 00af24a1
        fmul st(0), st(1) // 00af24a3
        mov edi, edx // 00af24a5
        lea eax, [edi + edi*4] // 00af24a7
        add eax, eax // 00af24aa
        add eax, eax // 00af24ac
        fstp dword ptr [esp + 0x28] // 00af24ae
        add ecx, eax // 00af24b2
        fld st(3) // 00af24b4
        fmul st(0), st(1) // 00af24b6
        fstp dword ptr [esp + 0x2c] // 00af24b8
        fmul st(0), st(4) // 00af24bc
        fstp dword ptr [esp + 0x30] // 00af24be
        fld dword ptr [esp + 0x28] // 00af24c2
        fadd dword ptr [esp + 0x40] // 00af24c6
        fstp dword ptr [esp + 0x34] // 00af24ca
        fld dword ptr [esp + 0x2c] // 00af24ce
        fadd dword ptr [esp + 0x44] // 00af24d2
        fstp dword ptr [esp + 0x38] // 00af24d6
        fld dword ptr [esp + 0x30] // 00af24da
        fadd dword ptr [esp + 0x48] // 00af24de
        fstp dword ptr [esp + 0x3c] // 00af24e2
        fld dword ptr [esp + 0x34] // 00af24e6
        fstp dword ptr [ecx] // 00af24ea
        fld dword ptr [esp + 0x38] // 00af24ec
        fstp dword ptr [ecx + 4] // 00af24f0
        fld dword ptr [esp + 0x3c] // 00af24f3
        fstp dword ptr [ecx + 8] // 00af24f7
        mov ecx, dword ptr [esi + 0x188] // 00af24fa
        fld dword ptr [esp + 0x14] // 00af2500
        fld qword ptr [esp + 0x1c] // 00af2504
        fmul st(0), st(1) // 00af2508
        fadd dword ptr [esp + 0x50] // 00af250a
        fstp dword ptr [esp + 0x14] // 00af250e
        fld dword ptr [esp + 0x14] // 00af2512
        fstp dword ptr [eax + ecx + 0x10] // 00af2516
        mov edx, dword ptr [esi + 0x184] // 00af251a
        fld dword ptr [edx + 0x14] // 00af2520
        mov ecx, dword ptr [esi + 0x188] // 00af2523
        fsub st(0), st(2) // 00af2529
        fmulp st(1), st(0) // 00af252b
        fadd st(0), st(1) // 00af252d
        fstp dword ptr [esp + 0x14] // 00af252f
        fld dword ptr [esp + 0x14] // 00af2533
        fstp dword ptr [eax + ecx + 0xc] // 00af2537
        mov ebx, dword ptr [esi + 0x184] // 00af253b
        mov eax, dword ptr [esi + 0x190] // 00af2541
        mov ecx, dword ptr [ebx + 0x20] // 00af2547
        cmp eax, ecx // 00af254a
        jne l_00af256f // 00af254c
        mov eax, dword ptr [esi + 0x18c] // 00af254e
        add eax, 1 // 00af2554
        cdq  // 00af2557
        idiv ecx // 00af2558
        mov eax, dword ptr [esi + 0x190] // 00af255a
        add eax, -1 // 00af2560
        mov dword ptr [esi + 0x190], eax // 00af2563
        mov dword ptr [esi + 0x18c], edx // 00af2569
    l_00af256f:
        mov edx, dword ptr [esp + 0x18] // 00af256f
        cmp dword ptr [esp + 0x10], edx // 00af2573
        jl l_00af246e // 00af2577
        fstp st(0) // 00af257d
        fstp st(3) // 00af257f
        fstp st(1) // 00af2581
        fstp st(0) // 00af2583
    l_00af2585:
        add dword ptr [esi + 0x190], 1 // 00af2585
        fstp st(0) // 00af258c
        mov ecx, dword ptr [esi + 0x184] // 00af258e
        fld dword ptr [ebp] // 00af2594
        lea eax, [edi + 1] // 00af2597
        cdq  // 00af259a
        idiv dword ptr [ecx + 0x20] // 00af259b
        mov ecx, dword ptr [esi + 0x188] // 00af259e
        push edx
        mov edx,dword ptr [esp+0x60]
        mov edx,dword ptr [edx+8]
        movss xmm0, dword ptr [edx] // 00af25a4
        pop edx
        subss xmm0, dword ptr [esp + 0x58] // 00af25ac
        lea eax, [edx + edx*4] // 00af25b2
        add eax, eax // 00af25b5
        add eax, eax // 00af25b7
        fstp dword ptr [ecx + eax] // 00af25b9
        add ecx, eax // 00af25bc
        fld dword ptr [ebp + 4] // 00af25be
        fstp dword ptr [ecx + 4] // 00af25c1
        fld dword ptr [ebp + 8] // 00af25c4
        fstp dword ptr [ecx + 8] // 00af25c7
        mov edx, dword ptr [esi + 0x188] // 00af25ca
        movss dword ptr [eax + edx + 0x10], xmm0 // 00af25d0
        mov ecx, dword ptr [esi + 0x184] // 00af25d6
        fld dword ptr [ecx + 0x14] // 00af25dc
        mov edx, dword ptr [esi + 0x188] // 00af25df
        fstp dword ptr [eax + edx + 0xc] // 00af25e5
        mov eax, dword ptr [esi + 0x184] // 00af25e9
        mov ecx, dword ptr [esi + 0x190] // 00af25ef
        mov edi, dword ptr [eax + 0x20] // 00af25f5
        cmp ecx, edi // 00af25f8
        jne l_00af2617 // 00af25fa
        mov eax, dword ptr [esi + 0x18c] // 00af25fc
        add eax, 1 // 00af2602
        cdq  // 00af2605
        idiv edi // 00af2606
        add ecx, -1 // 00af2608
        mov dword ptr [esi + 0x190], ecx // 00af260b
        mov dword ptr [esi + 0x18c], edx // 00af2611
    l_00af2617:
        pop edi // 00af2617
    l_00af2618:
        pop esi // 00af2618
        pop ebp // 00af2619
        pop ebx // 00af261a
        add esp, 0x44 // 00af261b
        ret 8 // 00af261e
    }
}

// Complete native instruction schedule; only current bindings/addresses change.
__declspec(naked) std::uint32_t __fastcall update_native_particle_tracer_00b0a110(void*,const NativeTracelineUpdateAccess*,void*,float,std::uint32_t,const void*,float) {
    __asm {
        push edx // borrowed access slot; native locals/spills stay at their offsets
        sub esp, 0x1c // 00b0a110
        push esi // 00b0a113
        mov esi, dword ptr [esp + 0x28] // 00b0a114
        cmp dword ptr [esi + 0x30], 0 // 00b0a118
        push edi // 00b0a11c
        mov edi, ecx // 00b0a11d
        je l_00b0a3df // 00b0a11f
        fld dword ptr [esi + 0x44] // 00b0a125
        mov ecx, dword ptr [edi + 0x2c] // 00b0a128
        push edx
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+12]
        fdiv qword ptr [edx] // 00b0a12b
        pop edx
        movzx eax, word ptr [ecx + 0xa] // 00b0a131
        test ax, ax // 00b0a135
        fstp dword ptr [esp + 0x2c] // 00b0a138
        jne l_00b0a14b // 00b0a13c
        fld dword ptr [ecx + 4] // 00b0a13e
        fld dword ptr [esp + 0x30] // 00b0a141
        fld st(0) // 00b0a145
        fmulp st(2), st(0) // 00b0a147
        jmp l_00b0a171 // 00b0a149
    l_00b0a14b:
        cmp ax, 1 // 00b0a14b
        fld dword ptr [esp + 0x30] // 00b0a14f
        push ecx // 00b0a153
        fstp dword ptr [esp] // 00b0a154
        jne l_00b0a160 // 00b0a157
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_linear_00affd20 // 00b0a159
        jmp l_00b0a165 // 00b0a15e
    l_00b0a160:
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_hermite_00affcb0 // 00b0a160
    l_00b0a165:
        fstp dword ptr [esp + 8] // 00b0a165
        fld dword ptr [esp + 8] // 00b0a169
        fld dword ptr [esp + 0x30] // 00b0a16d
    l_00b0a171:
        fld dword ptr [esp + 0x2c] // 00b0a171
        fld st(0) // 00b0a175
        fmulp st(3), st(0) // 00b0a177
        fxch st(2) // 00b0a179
        fstp dword ptr [esp + 8] // 00b0a17b
        fld dword ptr [esi + 0x18] // 00b0a17f
        fld dword ptr [esp + 8] // 00b0a182
        fld st(0) // 00b0a186
        fmulp st(2), st(0) // 00b0a188
        fxch st(1) // 00b0a18a
        fstp dword ptr [esp + 0xc] // 00b0a18c
        fld dword ptr [esi + 0x1c] // 00b0a190
        fmul st(0), st(1) // 00b0a193
        fstp dword ptr [esp + 0x10] // 00b0a195
        fmul dword ptr [esi + 0x20] // 00b0a199
        fstp dword ptr [esp + 0x14] // 00b0a19c
        fld dword ptr [esp + 0xc] // 00b0a1a0
        fstp dword ptr [esi] // 00b0a1a4
        fld dword ptr [esp + 0x10] // 00b0a1a6
        fstp dword ptr [esi + 4] // 00b0a1aa
        fld dword ptr [esp + 0x14] // 00b0a1ad
        fstp dword ptr [esi + 8] // 00b0a1b1
        mov ecx, dword ptr [edi + 0x30] // 00b0a1b4
        movzx eax, word ptr [ecx + 0xa] // 00b0a1b7
        test ax, ax // 00b0a1bb
        jne l_00b0a1c7 // 00b0a1be
        fld dword ptr [ecx + 4] // 00b0a1c0
        fmul st(0), st(1) // 00b0a1c3
        jmp l_00b0a1f1 // 00b0a1c5
    l_00b0a1c7:
        cmp ax, 1 // 00b0a1c7
        fstp st(1) // 00b0a1cb
        push ecx // 00b0a1cd
        fstp dword ptr [esp] // 00b0a1ce
        jne l_00b0a1da // 00b0a1d1
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_linear_00affd20 // 00b0a1d3
        jmp l_00b0a1df // 00b0a1d8
    l_00b0a1da:
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_hermite_00affcb0 // 00b0a1da
    l_00b0a1df:
        fstp dword ptr [esp + 8] // 00b0a1df
        fld dword ptr [esp + 8] // 00b0a1e3
        fld dword ptr [esp + 0x30] // 00b0a1e7
        fld dword ptr [esp + 0x2c] // 00b0a1eb
        fxch st(2) // 00b0a1ef
    l_00b0a1f1:
        mov eax, dword ptr [esp + 0x38] // 00b0a1f1
        fld dword ptr [eax + 0x10] // 00b0a1f5
        fstp dword ptr [esp + 0xc] // 00b0a1f8
        fld dword ptr [eax + 0x14] // 00b0a1fc
        fstp dword ptr [esp + 0x10] // 00b0a1ff
        fld dword ptr [eax + 0x18] // 00b0a203
        fstp dword ptr [esp + 0x14] // 00b0a206
        fstp dword ptr [esp + 0x38] // 00b0a20a
        fld dword ptr [esp + 0x38] // 00b0a20e
        fld st(0) // 00b0a212
        fmul dword ptr [esp + 0xc] // 00b0a214
        fstp dword ptr [esp + 0x18] // 00b0a218
        fld dword ptr [esp + 0x10] // 00b0a21c
        fmul st(0), st(1) // 00b0a220
        fstp dword ptr [esp + 0x1c] // 00b0a222
        fmul dword ptr [esp + 0x14] // 00b0a226
        fstp dword ptr [esp + 0x20] // 00b0a22a
        fld dword ptr [esp + 0x18] // 00b0a22e
        fmul st(0), st(2) // 00b0a232
        fstp dword ptr [esp + 0xc] // 00b0a234
        fld dword ptr [esp + 0x1c] // 00b0a238
        fmul st(0), st(2) // 00b0a23c
        fstp dword ptr [esp + 0x10] // 00b0a23e
        fld dword ptr [esp + 0x20] // 00b0a242
        fmul st(0), st(2) // 00b0a246
        fstp dword ptr [esp + 0x14] // 00b0a248
        fld dword ptr [esi + 0x48] // 00b0a24c
        fstp dword ptr [esp + 0x38] // 00b0a24f
        fld dword ptr [esp + 0xc] // 00b0a253
        fld dword ptr [esp + 0x38] // 00b0a257
        fld st(0) // 00b0a25b
        fmulp st(2), st(0) // 00b0a25d
        fxch st(1) // 00b0a25f
        fstp dword ptr [esp + 0x18] // 00b0a261
        fld dword ptr [esp + 0x10] // 00b0a265
        fmul st(0), st(1) // 00b0a269
        fstp dword ptr [esp + 0x1c] // 00b0a26b
        fmul dword ptr [esp + 0x14] // 00b0a26f
        fstp dword ptr [esp + 0x20] // 00b0a273
        fld dword ptr [esi] // 00b0a277
        fadd dword ptr [esp + 0x18] // 00b0a279
        fstp dword ptr [esi] // 00b0a27d
        fld dword ptr [esi + 4] // 00b0a27f
        fadd dword ptr [esp + 0x1c] // 00b0a282
        fstp dword ptr [esi + 4] // 00b0a286
        fld dword ptr [esi + 8] // 00b0a289
        fadd dword ptr [esp + 0x20] // 00b0a28c
        fstp dword ptr [esi + 8] // 00b0a290
        mov ecx, dword ptr [edi + 0x34] // 00b0a293
        movzx eax, word ptr [ecx + 0xa] // 00b0a296
        test ax, ax // 00b0a29a
        jne l_00b0a2a6 // 00b0a29d
        fld dword ptr [ecx + 4] // 00b0a29f
        fmul st(0), st(1) // 00b0a2a2
        jmp l_00b0a2d0 // 00b0a2a4
    l_00b0a2a6:
        cmp ax, 1 // 00b0a2a6
        fstp st(1) // 00b0a2aa
        push ecx // 00b0a2ac
        fstp dword ptr [esp] // 00b0a2ad
        jne l_00b0a2b9 // 00b0a2b0
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_linear_00affd20 // 00b0a2b2
        jmp l_00b0a2be // 00b0a2b7
    l_00b0a2b9:
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_hermite_00affcb0 // 00b0a2b9
    l_00b0a2be:
        fstp dword ptr [esp + 0x38] // 00b0a2be
        fld dword ptr [esp + 0x38] // 00b0a2c2
        fld dword ptr [esp + 0x30] // 00b0a2c6
        fld dword ptr [esp + 0x2c] // 00b0a2ca
        fxch st(2) // 00b0a2ce
    l_00b0a2d0:
        fmul st(0), st(2) // 00b0a2d0
        fmul dword ptr [esi + 0x4c] // 00b0a2d2
        fstp dword ptr [esp + 0x30] // 00b0a2d5
        fld dword ptr [esp + 0x30] // 00b0a2d9
        fld st(0) // 00b0a2dd
        push edx
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+16]
        fmul dword ptr [edx] // 00b0a2df
        pop edx
        fstp dword ptr [esp + 0x18] // 00b0a2e5
        fld st(0) // 00b0a2e9
        push edx
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+16]
        fmul dword ptr [edx+4] // 00b0a2eb
        pop edx
        fstp dword ptr [esp + 0x1c] // 00b0a2f1
        push edx
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+16]
        fmul dword ptr [edx+8] // 00b0a2f5
        pop edx
        fstp dword ptr [esp + 0x20] // 00b0a2fb
        fld dword ptr [esi] // 00b0a2ff
        fadd dword ptr [esp + 0x18] // 00b0a301
        fstp dword ptr [esi] // 00b0a305
        fld dword ptr [esi + 4] // 00b0a307
        fadd dword ptr [esp + 0x1c] // 00b0a30a
        fstp dword ptr [esi + 4] // 00b0a30e
        fld dword ptr [esi + 8] // 00b0a311
        fadd dword ptr [esp + 0x20] // 00b0a314
        fstp dword ptr [esi + 8] // 00b0a318
        mov ecx, dword ptr [edi + 0x48] // 00b0a31b
        test ecx, ecx // 00b0a31e
        je l_00b0a399 // 00b0a320
        movzx eax, word ptr [ecx + 0xa] // 00b0a322
        test ax, ax // 00b0a326
        jne l_00b0a330 // 00b0a329
        fmul dword ptr [ecx + 4] // 00b0a32b
        jmp l_00b0a354 // 00b0a32e
    l_00b0a330:
        cmp ax, 1 // 00b0a330
        fstp st(1) // 00b0a334
        push ecx // 00b0a336
        fstp dword ptr [esp] // 00b0a337
        jne l_00b0a343 // 00b0a33a
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_linear_00affd20 // 00b0a33c
        jmp l_00b0a348 // 00b0a341
    l_00b0a343:
        mov edx,dword ptr [esp+0x28]
        mov edx,dword ptr [edx+20]
        call integrate_native_particle_parameter_hermite_00affcb0 // 00b0a343
    l_00b0a348:
        fstp dword ptr [esp + 0x30] // 00b0a348
        fld dword ptr [esp + 0x2c] // 00b0a34c
        fld dword ptr [esp + 0x30] // 00b0a350
    l_00b0a354:
        fmulp st(1), st(0) // 00b0a354
        fstp dword ptr [esp + 0x30] // 00b0a356
        fld dword ptr [esi + 0x24] // 00b0a35a
        fld dword ptr [esp + 0x30] // 00b0a35d
        fld st(0) // 00b0a361
        fmulp st(2), st(0) // 00b0a363
        fxch st(1) // 00b0a365
        fstp dword ptr [esp + 0x18] // 00b0a367
        fld dword ptr [esi + 0x28] // 00b0a36b
        fmul st(0), st(1) // 00b0a36e
        fstp dword ptr [esp + 0x1c] // 00b0a370
        fmul dword ptr [esi + 0x2c] // 00b0a374
        fstp dword ptr [esp + 0x20] // 00b0a377
        fld dword ptr [esi] // 00b0a37b
        fadd dword ptr [esp + 0x18] // 00b0a37d
        fstp dword ptr [esi] // 00b0a381
        fld dword ptr [esi + 4] // 00b0a383
        fadd dword ptr [esp + 0x1c] // 00b0a386
        fstp dword ptr [esi + 4] // 00b0a38a
        fld dword ptr [esi + 8] // 00b0a38d
        fadd dword ptr [esp + 0x20] // 00b0a390
        fstp dword ptr [esi + 8] // 00b0a394
        jmp l_00b0a39d // 00b0a397
    l_00b0a399:
        fstp st(1) // 00b0a399
        fstp st(0) // 00b0a39b
    l_00b0a39d:
        fld dword ptr [esi] // 00b0a39d
        fadd dword ptr [esi + 0xc] // 00b0a39f
        fstp dword ptr [esi] // 00b0a3a2
        fld dword ptr [esi + 0x10] // 00b0a3a4
        fadd dword ptr [esi + 4] // 00b0a3a7
        fstp dword ptr [esi + 4] // 00b0a3aa
        fld dword ptr [esi + 0x14] // 00b0a3ad
        fadd dword ptr [esi + 8] // 00b0a3b0
        fstp dword ptr [esi + 8] // 00b0a3b3
        mov ecx,dword ptr [esp+0x24] // same current timer domain // 00b0a3b6
        call interval_bridge // 00b0a3c1
        fild qword ptr [eax] // 00b0a3c3
        fild qword ptr [eax + 8] // 00b0a3c5
        push ecx // 00b0a3c8
        mov ecx, dword ptr [esi + 0x30] // 00b0a3c9
        fdivp st(1), st(0) // 00b0a3cc
        fstp dword ptr [esp + 0x34] // 00b0a3ce
        fld dword ptr [esp + 0x34] // 00b0a3d2
        fstp dword ptr [esp] // 00b0a3d6
        push esi // 00b0a3d9
        mov edx,dword ptr [esp+0x2c]
        call set_native_traceline_point_00af2630 // 00b0a3da
    l_00b0a3df:
        pop edi // 00b0a3df
        mov eax, 1 // 00b0a3e0
        pop esi // 00b0a3e5
        add esp, 0x1c // 00b0a3e6
        add esp,4 // discard borrowed access slot
        ret 0x14 // 00b0a3e9
    }
}
} // namespace bsp
