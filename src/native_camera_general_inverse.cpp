#include "bsp/native_camera_general_inverse.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native general camera inversion requires MSVC Win32 x87/SSE assembly.
#endif

namespace bsp {
namespace {
// Verified immutable original .rdata words; only these data addresses relocate.
const std::uint32_t camera_inverse_negative_zero_d7a208 = 0x80000000u;
const std::uint32_t camera_inverse_positive_one_d7a24c = 0x3f800000u;
}
// Full B632D0..B63B28. Promote the existing private instruction schedule
// to a raw pointer entry; preserve native source snapshot and pivot order.
__declspec(naked) void* __fastcall invert_native_camera_matrix_00b632d0(void*, const void*) {
    __asm {
        sub esp, 0xb0 // 00b632d0
        xorps xmm0, xmm0 // 00b632d6
        push ebx // 00b632d9
        mov ebx, ecx // 00b632da
        push ebp // 00b632dc
        movss xmm1, dword ptr [camera_inverse_positive_one_d7a24c] // 00b632dd
        push esi // 00b632e5
        movss xmm5, dword ptr [camera_inverse_negative_zero_d7a208] // 00b632e6
        mov esi, edx // 00b632ee
        lea edx, [esp + 0x50] // 00b632f0
        mov eax, edx // 00b632f4
        mov dword ptr [esp + 0x4c], eax // 00b632f6
        mov eax, 0x2c // 00b632fa
        sub eax, ebx // 00b632ff
        mov dword ptr [esp + 0x98], eax // 00b63301
        lea eax, [esp + 0x54] // 00b63308
        sub eax, ebx // 00b6330c
        mov dword ptr [esp + 0xb8], eax // 00b6330e
        lea eax, [esp + 0x58] // 00b63315
        sub eax, ebx // 00b63319
        mov dword ptr [esp + 0x94], eax // 00b6331b
        push edi // 00b63322
        mov eax, 0xfffffffc // 00b63323
        sub eax, ebx // 00b63328
        mov ecx, 0x10 // 00b6332a
        lea edi, [esp + 0x54] // 00b6332f
        rep movsd  // 00b63333
        mov dword ptr [esp + 0xb4], eax // 00b63335
        xor edi, edi // 00b6333c
        mov ebp, edx // 00b6333e
        sub ebp, ebx // 00b63340
        mov eax, 4 // 00b63342
        mov esi, edx // 00b63347
        lea ecx, [ebx + 4] // 00b63349
        sub eax, esi // 00b6334c
        movss dword ptr [ebx], xmm1 // 00b6334e
        movss dword ptr [ecx], xmm0 // 00b63352
        movss dword ptr [ebx + 8], xmm0 // 00b63356
        movss dword ptr [ebx + 0xc], xmm0 // 00b6335b
        movss dword ptr [ebx + 0x10], xmm0 // 00b63360
        movss dword ptr [ebx + 0x14], xmm1 // 00b63365
        movss dword ptr [ebx + 0x18], xmm0 // 00b6336a
        movss dword ptr [ebx + 0x1c], xmm0 // 00b6336f
        movss dword ptr [ebx + 0x20], xmm0 // 00b63374
        movss dword ptr [ebx + 0x24], xmm0 // 00b63379
        movss dword ptr [ebx + 0x28], xmm1 // 00b6337e
        movss dword ptr [ebx + 0x2c], xmm0 // 00b63383
        movss dword ptr [ebx + 0x30], xmm0 // 00b63388
        movss dword ptr [ebx + 0x34], xmm0 // 00b6338d
        movss dword ptr [ebx + 0x38], xmm0 // 00b63392
        movss dword ptr [ebx + 0x3c], xmm1 // 00b63397
        mov dword ptr [esp + 0x20], edi // 00b6339c
        mov dword ptr [esp + 0x1c], edi // 00b633a0
        mov dword ptr [esp + 0x24], edx // 00b633a4
        mov dword ptr [esp + 0x4c], 3 // 00b633a8
        mov dword ptr [esp + 0x94], ebp // 00b633b0
        mov dword ptr [esp + 0xac], eax // 00b633b7
        jmp L_00b633c4 // 00b633be
L_00b633c0:
        mov edi, dword ptr [esp + 0x20] // 00b633c0
L_00b633c4:
        cmp dword ptr [esp + 0x4c], 4 // 00b633c4
        lea esi, [edi + 1] // 00b633c9
        mov dword ptr [esp + 0xa4], esi // 00b633cc
        jl L_00b635b0 // 00b633d3
        mov eax, dword ptr [esp + 0x9c] // 00b633d9
        mov ebp, dword ptr [esp + 0x1c] // 00b633e0
        mov edx, dword ptr [esp + 0xb4] // 00b633e4
        add eax, ecx // 00b633eb
        mov dword ptr [esp + 0x48], eax // 00b633ed
        mov eax, dword ptr [esp + 0xac] // 00b633f1
        add eax, ebp // 00b633f8
        mov ebp, dword ptr [esp + 0x48] // 00b633fa
        add edx, ecx // 00b633fe
        add eax, dword ptr [esp + 0x20] // 00b63400
        lea eax, [esp + eax + 0x54] // 00b63404
        lea eax, [esp + eax*4 + 0x64] // 00b63408
        mov dword ptr [esp + 0x10], eax // 00b6340c
L_00b63410:
        mov eax, dword ptr [esp + 0x10] // 00b63410
        movss xmm1, dword ptr [eax - 0x10] // 00b63414
        comiss xmm1, xmm0 // 00b63419
        jbe L_00b63426 // 00b6341c
        movss dword ptr [esp + 0x14], xmm1 // 00b6341e
        jmp L_00b63433 // 00b63424
L_00b63426:
        movaps xmm2, xmm5 // 00b63426
        subss xmm2, xmm1 // 00b63429
        movss dword ptr [esp + 0x14], xmm2 // 00b6342d
L_00b63433:
        mov eax, dword ptr [esp + 0x1c] // 00b63433
        add eax, edx // 00b63437
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63439
        comiss xmm1, xmm0 // 00b6343f
        lea eax, [esp + eax + 0x54] // 00b63442
        jbe L_00b63450 // 00b63446
        movss dword ptr [esp + 0x18], xmm1 // 00b63448
        jmp L_00b6345d // 00b6344e
L_00b63450:
        movaps xmm2, xmm5 // 00b63450
        subss xmm2, xmm1 // 00b63453
        movss dword ptr [esp + 0x18], xmm2 // 00b63457
L_00b6345d:
        fld dword ptr [esp + 0x18] // 00b6345d
        fld dword ptr [esp + 0x14] // 00b63461
        fcomip st(0), st(1) // 00b63465
        fstp st(0) // 00b63467
        jbe L_00b63470 // 00b63469
        mov edi, esi // 00b6346b
        lea edx, [ebp - 0x20] // 00b6346d
L_00b63470:
        mov eax, dword ptr [esp + 0x10] // 00b63470
        movss xmm1, dword ptr [eax] // 00b63474
        comiss xmm1, xmm0 // 00b63478
        jbe L_00b63485 // 00b6347b
        movss dword ptr [esp + 0x14], xmm1 // 00b6347d
        jmp L_00b63492 // 00b63483
L_00b63485:
        movaps xmm2, xmm5 // 00b63485
        subss xmm2, xmm1 // 00b63488
        movss dword ptr [esp + 0x14], xmm2 // 00b6348c
L_00b63492:
        mov eax, dword ptr [esp + 0x1c] // 00b63492
        add eax, edx // 00b63496
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63498
        comiss xmm1, xmm0 // 00b6349e
        lea eax, [esp + eax + 0x54] // 00b634a1
        jbe L_00b634af // 00b634a5
        movss dword ptr [esp + 0x18], xmm1 // 00b634a7
        jmp L_00b634bc // 00b634ad
L_00b634af:
        movaps xmm2, xmm5 // 00b634af
        subss xmm2, xmm1 // 00b634b2
        movss dword ptr [esp + 0x18], xmm2 // 00b634b6
L_00b634bc:
        fld dword ptr [esp + 0x18] // 00b634bc
        fld dword ptr [esp + 0x14] // 00b634c0
        fcomip st(0), st(1) // 00b634c4
        fstp st(0) // 00b634c6
        jbe L_00b634d0 // 00b634c8
        lea edi, [esi + 1] // 00b634ca
        lea edx, [ebp - 0x10] // 00b634cd
L_00b634d0:
        mov eax, dword ptr [esp + 0x10] // 00b634d0
        movss xmm1, dword ptr [eax + 0x10] // 00b634d4
        comiss xmm1, xmm0 // 00b634d9
        jbe L_00b634e6 // 00b634dc
        movss dword ptr [esp + 0x14], xmm1 // 00b634de
        jmp L_00b634f3 // 00b634e4
L_00b634e6:
        movaps xmm2, xmm5 // 00b634e6
        subss xmm2, xmm1 // 00b634e9
        movss dword ptr [esp + 0x14], xmm2 // 00b634ed
L_00b634f3:
        mov eax, dword ptr [esp + 0x1c] // 00b634f3
        add eax, edx // 00b634f7
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b634f9
        comiss xmm1, xmm0 // 00b634ff
        lea eax, [esp + eax + 0x54] // 00b63502
        jbe L_00b63510 // 00b63506
        movss dword ptr [esp + 0x18], xmm1 // 00b63508
        jmp L_00b6351d // 00b6350e
L_00b63510:
        movaps xmm2, xmm5 // 00b63510
        subss xmm2, xmm1 // 00b63513
        movss dword ptr [esp + 0x18], xmm2 // 00b63517
L_00b6351d:
        fld dword ptr [esp + 0x18] // 00b6351d
        fld dword ptr [esp + 0x14] // 00b63521
        fcomip st(0), st(1) // 00b63525
        fstp st(0) // 00b63527
        jbe L_00b63530 // 00b63529
        lea edi, [esi + 2] // 00b6352b
        mov edx, ebp // 00b6352e
L_00b63530:
        mov eax, dword ptr [esp + 0x10] // 00b63530
        movss xmm1, dword ptr [eax + 0x20] // 00b63534
        comiss xmm1, xmm0 // 00b63539
        jbe L_00b63546 // 00b6353c
        movss dword ptr [esp + 0x14], xmm1 // 00b6353e
        jmp L_00b63553 // 00b63544
L_00b63546:
        movaps xmm2, xmm5 // 00b63546
        subss xmm2, xmm1 // 00b63549
        movss dword ptr [esp + 0x14], xmm2 // 00b6354d
L_00b63553:
        mov eax, dword ptr [esp + 0x1c] // 00b63553
        add eax, edx // 00b63557
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63559
        comiss xmm1, xmm0 // 00b6355f
        lea eax, [esp + eax + 0x54] // 00b63562
        jbe L_00b63570 // 00b63566
        movss dword ptr [esp + 0x18], xmm1 // 00b63568
        jmp L_00b6357d // 00b6356e
L_00b63570:
        movaps xmm2, xmm5 // 00b63570
        subss xmm2, xmm1 // 00b63573
        movss dword ptr [esp + 0x18], xmm2 // 00b63577
L_00b6357d:
        fld dword ptr [esp + 0x18] // 00b6357d
        fld dword ptr [esp + 0x14] // 00b63581
        fcomip st(0), st(1) // 00b63585
        fstp st(0) // 00b63587
        jbe L_00b63591 // 00b63589
        lea edi, [esi + 3] // 00b6358b
        lea edx, [ebp + 0x10] // 00b6358e
L_00b63591:
        add dword ptr [esp + 0x10], 0x40 // 00b63591
        add ebp, 0x40 // 00b63596
        add esi, 4 // 00b63599
        cmp ebp, 0x30 // 00b6359c
        jl L_00b63410 // 00b6359f
        mov ebp, dword ptr [esp + 0x94] // 00b635a5
        mov edx, dword ptr [esp + 0x24] // 00b635ac
L_00b635b0:
        cmp esi, 4 // 00b635b0
        jge L_00b6365b // 00b635b3
        mov ebp, edi // 00b635b9
        mov eax, esi // 00b635bb
        shl ebp, 4 // 00b635bd
        shl eax, 4 // 00b635c0
        mov dword ptr [esp + 0x10], eax // 00b635c3
        mov eax, dword ptr [esp + 0x20] // 00b635c7
        lea eax, [eax + esi*4] // 00b635cb
        lea eax, [esp + eax*4 + 0x54] // 00b635ce
        mov dword ptr [esp + 0x24], eax // 00b635d2
        jmp L_00b635e0 // 00b635d6
        // Eight unreachable original alignment bytes at B635D8..B635DF.
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 090h
L_00b635e0:
        mov eax, dword ptr [esp + 0x24] // 00b635e0
        movss xmm1, dword ptr [eax] // 00b635e4
        comiss xmm1, xmm0 // 00b635e8
        jbe L_00b635f5 // 00b635eb
        movss dword ptr [esp + 0x14], xmm1 // 00b635ed
        jmp L_00b63602 // 00b635f3
L_00b635f5:
        movaps xmm2, xmm5 // 00b635f5
        subss xmm2, xmm1 // 00b635f8
        movss dword ptr [esp + 0x14], xmm2 // 00b635fc
L_00b63602:
        mov eax, dword ptr [esp + 0x1c] // 00b63602
        add eax, ebp // 00b63606
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63608
        comiss xmm1, xmm0 // 00b6360e
        lea eax, [esp + eax + 0x54] // 00b63611
        jbe L_00b6361f // 00b63615
        movss dword ptr [esp + 0x18], xmm1 // 00b63617
        jmp L_00b6362c // 00b6361d
L_00b6361f:
        movaps xmm2, xmm5 // 00b6361f
        subss xmm2, xmm1 // 00b63622
        movss dword ptr [esp + 0x18], xmm2 // 00b63626
L_00b6362c:
        fld dword ptr [esp + 0x18] // 00b6362c
        fld dword ptr [esp + 0x14] // 00b63630
        fcomip st(0), st(1) // 00b63634
        fstp st(0) // 00b63636
        jbe L_00b63640 // 00b63638
        mov ebp, dword ptr [esp + 0x10] // 00b6363a
        mov edi, esi // 00b6363e
L_00b63640:
        add dword ptr [esp + 0x10], 0x10 // 00b63640
        add dword ptr [esp + 0x24], 0x10 // 00b63645
        add esi, 1 // 00b6364a
        cmp dword ptr [esp + 0x10], 0x40 // 00b6364d
        jl L_00b635e0 // 00b63652
        mov ebp, dword ptr [esp + 0x94] // 00b63654
L_00b6365b:
        fld dword ptr [edx] // 00b6365b
        mov esi, dword ptr [esp + 0xbc] // 00b6365d
        shl edi, 4 // 00b63664
        cmp dword ptr [esp + 0x20], 0 // 00b63667
        mov eax, edi // 00b6366c
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b6366e
        fstp dword ptr [esp + eax + 0x54] // 00b63674
        movss xmm2, dword ptr [esp + eax + 0x58] // 00b63678
        fld dword ptr [ecx + ebp] // 00b6367e
        fstp dword ptr [esp + eax + 0x58] // 00b63681
        movss xmm3, dword ptr [esp + eax + 0x5c] // 00b63685
        movss xmm4, dword ptr [esp + eax + 0x60] // 00b6368b
        fld dword ptr [esi + ecx] // 00b63691
        fstp dword ptr [esp + eax + 0x5c] // 00b63694
        mov edi, dword ptr [esp + 0x98] // 00b63698
        fld dword ptr [edi + ecx] // 00b6369f
        fstp dword ptr [esp + eax + 0x60] // 00b636a2
        movss dword ptr [edi + ecx], xmm4 // 00b636a6
        fld dword ptr [ecx - 4] // 00b636ab
        movss xmm4, dword ptr [eax + ebx + 0xc] // 00b636ae
        movss dword ptr [ecx + ebp], xmm2 // 00b636b4
        movss xmm2, dword ptr [eax + ebx + 4] // 00b636b9
        movss dword ptr [edx], xmm1 // 00b636bf
        movss xmm1, dword ptr [eax + ebx] // 00b636c3
        fstp dword ptr [eax + ebx] // 00b636c8
        fld dword ptr [ecx] // 00b636cb
        movss dword ptr [esi + ecx], xmm3 // 00b636cd
        movss xmm3, dword ptr [eax + ebx + 8] // 00b636d2
        lea edi, [eax + ebx + 4] // 00b636d8
        mov dword ptr [esp + 0x48], edi // 00b636dc
        lea edi, [eax + ebx + 8] // 00b636e0
        mov dword ptr [esp + 0x24], edi // 00b636e4
        lea edi, [eax + ebx + 0xc] // 00b636e8
        mov eax, dword ptr [esp + 0x48] // 00b636ec
        fstp dword ptr [eax] // 00b636f0
        mov eax, dword ptr [esp + 0x24] // 00b636f2
        fld dword ptr [ecx + 4] // 00b636f6
        fstp dword ptr [eax] // 00b636f9
        mov eax, dword ptr [esp + 0x50] // 00b636fb
        fld dword ptr [ecx + 8] // 00b636ff
        fstp dword ptr [edi] // 00b63702
        movss dword ptr [ecx - 4], xmm1 // 00b63704
        fld dword ptr [eax] // 00b63709
        movss dword ptr [ecx], xmm2 // 00b6370b
        fstp dword ptr [esp + 0x48] // 00b6370f
        movss dword ptr [ecx + 4], xmm3 // 00b63713
        movss dword ptr [ecx + 8], xmm4 // 00b63718
        fld dword ptr [ecx - 4] // 00b6371d
        fld dword ptr [esp + 0x48] // 00b63720
        mov edi, dword ptr [esp + 0x98] // 00b63724
        fld st(0) // 00b6372b
        fdivp st(2), st(0) // 00b6372d
        fxch st(1) // 00b6372f
        fstp dword ptr [ecx - 4] // 00b63731
        fld dword ptr [ecx] // 00b63734
        fdiv st(0), st(1) // 00b63736
        fstp dword ptr [ecx] // 00b63738
        fld dword ptr [ecx + 4] // 00b6373a
        fdiv st(0), st(1) // 00b6373d
        fstp dword ptr [ecx + 4] // 00b6373f
        fld dword ptr [ecx + 8] // 00b63742
        fdiv st(0), st(1) // 00b63745
        fstp dword ptr [ecx + 8] // 00b63747
        fld dword ptr [edx] // 00b6374a
        fdiv st(0), st(1) // 00b6374c
        fstp dword ptr [edx] // 00b6374e
        fld dword ptr [ecx + ebp] // 00b63750
        fdiv st(0), st(1) // 00b63753
        fstp dword ptr [ecx + ebp] // 00b63755
        fld dword ptr [esi + ecx] // 00b63758
        fdiv st(0), st(1) // 00b6375b
        fstp dword ptr [esi + ecx] // 00b6375d
        fdivr dword ptr [edi + ecx] // 00b63760
        fstp dword ptr [edi + ecx] // 00b63763
        je L_00b63839 // 00b63766
        mov eax, dword ptr [esp + 0x1c] // 00b6376c
        movss xmm1, dword ptr [esp + eax + 0x54] // 00b63770
        ucomiss xmm1, xmm0 // 00b63776
        lahf  // 00b63779
        test ah, 0x44 // 00b6377a
        movss dword ptr [esp + 0xb0], xmm1 // 00b6377d
        jnp L_00b63839 // 00b63786
        fld dword ptr [esp + 0xb0] // 00b6378c
        fstp dword ptr [esp + 0x10] // 00b63793
        fld dword ptr [ecx - 4] // 00b63797
        fld dword ptr [esp + 0x10] // 00b6379a
        fld st(0) // 00b6379e
        fmulp st(2), st(0) // 00b637a0
        fxch st(1) // 00b637a2
        fstp dword ptr [esp + 0x38] // 00b637a4
        fld dword ptr [ecx] // 00b637a8
        fmul st(0), st(1) // 00b637aa
        fstp dword ptr [esp + 0x3c] // 00b637ac
        fld dword ptr [ecx + 4] // 00b637b0
        fmul st(0), st(1) // 00b637b3
        fstp dword ptr [esp + 0x40] // 00b637b5
        fld dword ptr [ecx + 8] // 00b637b9
        fmul st(0), st(1) // 00b637bc
        fstp dword ptr [esp + 0x44] // 00b637be
        fld dword ptr [ebx] // 00b637c2
        fsub dword ptr [esp + 0x38] // 00b637c4
        fstp dword ptr [ebx] // 00b637c8
        fld dword ptr [ebx + 4] // 00b637ca
        fsub dword ptr [esp + 0x3c] // 00b637cd
        fstp dword ptr [ebx + 4] // 00b637d1
        fld dword ptr [ebx + 8] // 00b637d4
        fsub dword ptr [esp + 0x40] // 00b637d7
        fstp dword ptr [ebx + 8] // 00b637db
        fld dword ptr [ebx + 0xc] // 00b637de
        fsub dword ptr [esp + 0x44] // 00b637e1
        fstp dword ptr [ebx + 0xc] // 00b637e5
        fld dword ptr [edx] // 00b637e8
        fmul st(0), st(1) // 00b637ea
        fstp dword ptr [esp + 0x28] // 00b637ec
        fld st(0) // 00b637f0
        fmul dword ptr [ecx + ebp] // 00b637f2
        fstp dword ptr [esp + 0x2c] // 00b637f5
        fld dword ptr [esi + ecx] // 00b637f9
        fmul st(0), st(1) // 00b637fc
        fstp dword ptr [esp + 0x30] // 00b637fe
        fmul dword ptr [edi + ecx] // 00b63802
        fstp dword ptr [esp + 0x34] // 00b63805
        fld dword ptr [esp + 0x54] // 00b63809
        fsub dword ptr [esp + 0x28] // 00b6380d
        fstp dword ptr [esp + 0x54] // 00b63811
        fld dword ptr [esp + 0x58] // 00b63815
        fsub dword ptr [esp + 0x2c] // 00b63819
        fstp dword ptr [esp + 0x58] // 00b6381d
        fld dword ptr [esp + 0x5c] // 00b63821
        fsub dword ptr [esp + 0x30] // 00b63825
        fstp dword ptr [esp + 0x5c] // 00b63829
        fld dword ptr [esp + 0x60] // 00b6382d
        fsub dword ptr [esp + 0x34] // 00b63831
        fstp dword ptr [esp + 0x60] // 00b63835
L_00b63839:
        cmp dword ptr [esp + 0x20], 1 // 00b63839
        je L_00b63913 // 00b6383e
        mov eax, dword ptr [esp + 0x1c] // 00b63844
        movss xmm1, dword ptr [esp + eax + 0x64] // 00b63848
        ucomiss xmm1, xmm0 // 00b6384e
        lahf  // 00b63851
        test ah, 0x44 // 00b63852
        movss dword ptr [esp + 0xa8], xmm1 // 00b63855
        jnp L_00b63913 // 00b6385e
        fld dword ptr [esp + 0xa8] // 00b63864
        fstp dword ptr [esp + 0x10] // 00b6386b
        fld dword ptr [ecx - 4] // 00b6386f
        fld dword ptr [esp + 0x10] // 00b63872
        fld st(0) // 00b63876
        fmulp st(2), st(0) // 00b63878
        fxch st(1) // 00b6387a
        fstp dword ptr [esp + 0x38] // 00b6387c
        fld dword ptr [ecx] // 00b63880
        fmul st(0), st(1) // 00b63882
        fstp dword ptr [esp + 0x3c] // 00b63884
        fld dword ptr [ecx + 4] // 00b63888
        fmul st(0), st(1) // 00b6388b
        fstp dword ptr [esp + 0x40] // 00b6388d
        fld dword ptr [ecx + 8] // 00b63891
        fmul st(0), st(1) // 00b63894
        fstp dword ptr [esp + 0x44] // 00b63896
        fld dword ptr [ebx + 0x10] // 00b6389a
        fsub dword ptr [esp + 0x38] // 00b6389d
        fstp dword ptr [ebx + 0x10] // 00b638a1
        fld dword ptr [ebx + 0x14] // 00b638a4
        fsub dword ptr [esp + 0x3c] // 00b638a7
        fstp dword ptr [ebx + 0x14] // 00b638ab
        fld dword ptr [ebx + 0x18] // 00b638ae
        fsub dword ptr [esp + 0x40] // 00b638b1
        fstp dword ptr [ebx + 0x18] // 00b638b5
        fld dword ptr [ebx + 0x1c] // 00b638b8
        fsub dword ptr [esp + 0x44] // 00b638bb
        fstp dword ptr [ebx + 0x1c] // 00b638bf
        fld dword ptr [edx] // 00b638c2
        fmul st(0), st(1) // 00b638c4
        fstp dword ptr [esp + 0x28] // 00b638c6
        fld st(0) // 00b638ca
        fmul dword ptr [ecx + ebp] // 00b638cc
        fstp dword ptr [esp + 0x2c] // 00b638cf
        fld dword ptr [esi + ecx] // 00b638d3
        fmul st(0), st(1) // 00b638d6
        fstp dword ptr [esp + 0x30] // 00b638d8
        fmul dword ptr [edi + ecx] // 00b638dc
        fstp dword ptr [esp + 0x34] // 00b638df
        fld dword ptr [esp + 0x64] // 00b638e3
        fsub dword ptr [esp + 0x28] // 00b638e7
        fstp dword ptr [esp + 0x64] // 00b638eb
        fld dword ptr [esp + 0x68] // 00b638ef
        fsub dword ptr [esp + 0x2c] // 00b638f3
        fstp dword ptr [esp + 0x68] // 00b638f7
        fld dword ptr [esp + 0x6c] // 00b638fb
        fsub dword ptr [esp + 0x30] // 00b638ff
        fstp dword ptr [esp + 0x6c] // 00b63903
        fld dword ptr [esp + 0x70] // 00b63907
        fsub dword ptr [esp + 0x34] // 00b6390b
        fstp dword ptr [esp + 0x70] // 00b6390f
L_00b63913:
        cmp dword ptr [esp + 0x20], 2 // 00b63913
        je L_00b639f3 // 00b63918
        mov eax, dword ptr [esp + 0x1c] // 00b6391e
        movss xmm1, dword ptr [esp + eax + 0x74] // 00b63922
        ucomiss xmm1, xmm0 // 00b63928
        lahf  // 00b6392b
        test ah, 0x44 // 00b6392c
        movss dword ptr [esp + 0xb8], xmm1 // 00b6392f
        jnp L_00b639f3 // 00b63938
        fld dword ptr [esp + 0xb8] // 00b6393e
        fstp dword ptr [esp + 0x10] // 00b63945
        fld dword ptr [ecx - 4] // 00b63949
        fld dword ptr [esp + 0x10] // 00b6394c
        fld st(0) // 00b63950
        fmulp st(2), st(0) // 00b63952
        fxch st(1) // 00b63954
        fstp dword ptr [esp + 0x38] // 00b63956
        fld dword ptr [ecx] // 00b6395a
        fmul st(0), st(1) // 00b6395c
        fstp dword ptr [esp + 0x3c] // 00b6395e
        fld dword ptr [ecx + 4] // 00b63962
        fmul st(0), st(1) // 00b63965
        fstp dword ptr [esp + 0x40] // 00b63967
        fld dword ptr [ecx + 8] // 00b6396b
        fmul st(0), st(1) // 00b6396e
        fstp dword ptr [esp + 0x44] // 00b63970
        fld dword ptr [ebx + 0x20] // 00b63974
        fsub dword ptr [esp + 0x38] // 00b63977
        fstp dword ptr [ebx + 0x20] // 00b6397b
        fld dword ptr [ebx + 0x24] // 00b6397e
        fsub dword ptr [esp + 0x3c] // 00b63981
        fstp dword ptr [ebx + 0x24] // 00b63985
        fld dword ptr [ebx + 0x28] // 00b63988
        fsub dword ptr [esp + 0x40] // 00b6398b
        fstp dword ptr [ebx + 0x28] // 00b6398f
        fld dword ptr [ebx + 0x2c] // 00b63992
        fsub dword ptr [esp + 0x44] // 00b63995
        fstp dword ptr [ebx + 0x2c] // 00b63999
        fld dword ptr [edx] // 00b6399c
        fmul st(0), st(1) // 00b6399e
        fstp dword ptr [esp + 0x28] // 00b639a0
        fld st(0) // 00b639a4
        fmul dword ptr [ecx + ebp] // 00b639a6
        fstp dword ptr [esp + 0x2c] // 00b639a9
        fld dword ptr [esi + ecx] // 00b639ad
        fmul st(0), st(1) // 00b639b0
        fstp dword ptr [esp + 0x30] // 00b639b2
        fmul dword ptr [edi + ecx] // 00b639b6
        fstp dword ptr [esp + 0x34] // 00b639b9
        fld dword ptr [esp + 0x74] // 00b639bd
        fsub dword ptr [esp + 0x28] // 00b639c1
        fstp dword ptr [esp + 0x74] // 00b639c5
        fld dword ptr [esp + 0x78] // 00b639c9
        fsub dword ptr [esp + 0x2c] // 00b639cd
        fstp dword ptr [esp + 0x78] // 00b639d1
        fld dword ptr [esp + 0x7c] // 00b639d5
        fsub dword ptr [esp + 0x30] // 00b639d9
        fstp dword ptr [esp + 0x7c] // 00b639dd
        fld dword ptr [esp + 0x80] // 00b639e1
        fsub dword ptr [esp + 0x34] // 00b639e8
        fstp dword ptr [esp + 0x80] // 00b639ec
L_00b639f3:
        cmp dword ptr [esp + 0x20], 3 // 00b639f3
        je L_00b63ae8 // 00b639f8
        mov eax, dword ptr [esp + 0x1c] // 00b639fe
        movss xmm1, dword ptr [esp + eax + 0x84] // 00b63a02
        ucomiss xmm1, xmm0 // 00b63a0b
        lahf  // 00b63a0e
        test ah, 0x44 // 00b63a0f
        movss dword ptr [esp + 0xa0], xmm1 // 00b63a12
        jnp L_00b63ae8 // 00b63a1b
        fld dword ptr [esp + 0xa0] // 00b63a21
        fstp dword ptr [esp + 0x10] // 00b63a28
        fld dword ptr [ecx - 4] // 00b63a2c
        fld dword ptr [esp + 0x10] // 00b63a2f
        fld st(0) // 00b63a33
        fmulp st(2), st(0) // 00b63a35
        fxch st(1) // 00b63a37
        fstp dword ptr [esp + 0x38] // 00b63a39
        fld dword ptr [ecx] // 00b63a3d
        fmul st(0), st(1) // 00b63a3f
        fstp dword ptr [esp + 0x3c] // 00b63a41
        fld dword ptr [ecx + 4] // 00b63a45
        fmul st(0), st(1) // 00b63a48
        fstp dword ptr [esp + 0x40] // 00b63a4a
        fld dword ptr [ecx + 8] // 00b63a4e
        fmul st(0), st(1) // 00b63a51
        fstp dword ptr [esp + 0x44] // 00b63a53
        fld dword ptr [ebx + 0x30] // 00b63a57
        fsub dword ptr [esp + 0x38] // 00b63a5a
        fstp dword ptr [ebx + 0x30] // 00b63a5e
        fld dword ptr [ebx + 0x34] // 00b63a61
        fsub dword ptr [esp + 0x3c] // 00b63a64
        fstp dword ptr [ebx + 0x34] // 00b63a68
        fld dword ptr [ebx + 0x38] // 00b63a6b
        fsub dword ptr [esp + 0x40] // 00b63a6e
        fstp dword ptr [ebx + 0x38] // 00b63a72
        fld dword ptr [ebx + 0x3c] // 00b63a75
        fsub dword ptr [esp + 0x44] // 00b63a78
        fstp dword ptr [ebx + 0x3c] // 00b63a7c
        fld dword ptr [edx] // 00b63a7f
        fmul st(0), st(1) // 00b63a81
        fstp dword ptr [esp + 0x28] // 00b63a83
        fld st(0) // 00b63a87
        fmul dword ptr [ecx + ebp] // 00b63a89
        fstp dword ptr [esp + 0x2c] // 00b63a8c
        fld dword ptr [esi + ecx] // 00b63a90
        fmul st(0), st(1) // 00b63a93
        fstp dword ptr [esp + 0x30] // 00b63a95
        fmul dword ptr [edi + ecx] // 00b63a99
        fstp dword ptr [esp + 0x34] // 00b63a9c
        fld dword ptr [esp + 0x84] // 00b63aa0
        fsub dword ptr [esp + 0x28] // 00b63aa7
        fstp dword ptr [esp + 0x84] // 00b63aab
        fld dword ptr [esp + 0x88] // 00b63ab2
        fsub dword ptr [esp + 0x2c] // 00b63ab9
        fstp dword ptr [esp + 0x88] // 00b63abd
        fld dword ptr [esp + 0x8c] // 00b63ac4
        fsub dword ptr [esp + 0x30] // 00b63acb
        fstp dword ptr [esp + 0x8c] // 00b63acf
        fld dword ptr [esp + 0x90] // 00b63ad6
        fsub dword ptr [esp + 0x34] // 00b63add
        fstp dword ptr [esp + 0x90] // 00b63ae1
L_00b63ae8:
        mov eax, dword ptr [esp + 0xa4] // 00b63ae8
        add dword ptr [esp + 0x50], 0x14 // 00b63aef
        add dword ptr [esp + 0x1c], 4 // 00b63af4
        mov dword ptr [esp + 0x20], eax // 00b63af9
        mov eax, dword ptr [esp + 0x4c] // 00b63afd
        sub eax, 1 // 00b63b01
        add edx, 0x10 // 00b63b04
        add ecx, 0x10 // 00b63b07
        cmp eax, -1 // 00b63b0a
        mov dword ptr [esp + 0x4c], eax // 00b63b0d
        mov dword ptr [esp + 0x24], edx // 00b63b11
        jg L_00b633c0 // 00b63b15
        pop edi // 00b63b1b
        pop esi // 00b63b1c
        pop ebp // 00b63b1d
        mov eax, ebx // 00b63b1e
        pop ebx // 00b63b20
        add esp, 0xb0 // 00b63b21
        ret  // 00b63b27
    }
}
} // namespace bsp
