#include "bsp/native_dyn_world_integration.hpp"
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Dyn integration requires MSVC Win32 x87/SSE assembly.
#endif
namespace bsp {
namespace {
alignas(8) const std::uint64_t constant_00d7a220=0x4059000000000000ULL;
alignas(8) const std::uint32_t constant_00d7a310=0x3727c5acU;
alignas(8) const std::uint64_t constant_00d7a390=0x3fecccccc0000000ULL;
// Existing 004011D0 float-rounding boundary; actual shared CRT ST0 service.
__declspec(naked) void sqrt_kernel(){
    __asm {
        push ebp
        mov ebp,esp
        and esp,-8
        sub esp,8
        fld dword ptr [ebp+0ch]
        push ecx
        mov ecx,dword ptr [ebp+8]
        call native_crt_sqrt_st0_00bf7030
        pop ecx
        fstp dword ptr [esp+4]
        fld dword ptr [esp+4]
        mov esp,ebp
        pop ebp
        ret 8
    }
}
// Complete native instruction schedule; comments identify instruction starts.
__declspec(naked) void velocity_kernel(){
    __asm {
        push ebp // 00c41550
        mov ebp, esp // 00c41551
        and esp, 0fffffff8h // 00c41553
        mov edx, dword ptr [esi + 0204h] // 00c41556
        sub esp, 0ech // 00c4155c
        push edi // 00c41562
        lea edi, [esi + 0208h] // 00c41563
        cmp edx, edi // 00c41569
        je l_00c41ac2 // 00c4156b
        fld dword ptr [ebp + 8] // 00c41571
        xorps xmm0, xmm0 // 00c41574
        fld1  // 00c41577
    l_00c41579:
        mov eax, dword ptr [edx + 050h] // 00c41579
        shr eax, 4 // 00c4157c
        test al, 1 // 00c4157f
        jne l_00c41ab0 // 00c41581
        mov ecx, dword ptr [edx + 4] // 00c41587
        fld dword ptr [ecx + 050h] // 00c4158a
        fmul st(0), st(2) // 00c4158d
        fstp dword ptr [esp + 8] // 00c4158f
        fld dword ptr [ecx + 038h] // 00c41593
        fld dword ptr [esp + 8] // 00c41596
        fld st(0) // 00c4159a
        fmulp st(2), st(0) // 00c4159c
        fxch st(1) // 00c4159e
        fstp dword ptr [esp + 078h] // 00c415a0
        fld dword ptr [ecx + 03ch] // 00c415a4
        fmul st(0), st(1) // 00c415a7
        fstp dword ptr [esp + 07ch] // 00c415a9
        fmul dword ptr [ecx + 040h] // 00c415ad
        fstp dword ptr [esp + 080h] // 00c415b0
        fld dword ptr [ecx] // 00c415b7
        fadd dword ptr [esp + 078h] // 00c415b9
        fstp dword ptr [esp + 060h] // 00c415bd
        mov eax, dword ptr [esp + 060h] // 00c415c1
        fld dword ptr [ecx + 4] // 00c415c5
        fadd dword ptr [esp + 07ch] // 00c415c8
        fstp dword ptr [esp + 064h] // 00c415cc
        fld dword ptr [ecx + 8] // 00c415d0
        mov dword ptr [ecx], eax // 00c415d3
        fadd dword ptr [esp + 080h] // 00c415d5
        mov eax, dword ptr [esp + 064h] // 00c415dc
        mov dword ptr [ecx + 4], eax // 00c415e0
        fstp dword ptr [esp + 068h] // 00c415e3
        mov eax, dword ptr [esp + 068h] // 00c415e7
        mov dword ptr [ecx + 8], eax // 00c415eb
        test byte ptr [edx + 050h], 4 // 00c415ee
        jne l_00c4163d // 00c415f2
        fld dword ptr [esi + 4] // 00c415f4
        fmul st(0), st(2) // 00c415f7
        fstp dword ptr [esp + 090h] // 00c415f9
        fld dword ptr [esi + 8] // 00c41600
        fmul st(0), st(2) // 00c41603
        fstp dword ptr [esp + 094h] // 00c41605
        fld dword ptr [esi + 0ch] // 00c4160c
        fmul st(0), st(2) // 00c4160f
        fstp dword ptr [esp + 098h] // 00c41611
        fld dword ptr [ecx] // 00c41618
        fadd dword ptr [esp + 090h] // 00c4161a
        fstp dword ptr [ecx] // 00c41621
        fld dword ptr [ecx + 4] // 00c41623
        fadd dword ptr [esp + 094h] // 00c41626
        fstp dword ptr [ecx + 4] // 00c4162d
        fld dword ptr [ecx + 8] // 00c41630
        fadd dword ptr [esp + 098h] // 00c41633
        fstp dword ptr [ecx + 8] // 00c4163a
    l_00c4163d:
        fld dword ptr [ecx + 0b8h] // 00c4163d
        fmul st(0), st(2) // 00c41643
        fsubp st(1), st(0) // 00c41645
        fstp dword ptr [esp + 8] // 00c41647
        fldz  // 00c4164b
        fld dword ptr [esp + 8] // 00c4164d
        fcomip st(0), st(1) // 00c41651
        fstp st(0) // 00c41653
        jbe l_00c41665 // 00c41655
        movss xmm1, dword ptr [esp + 8] // 00c41657
        movss dword ptr [esp + 0ch], xmm1 // 00c4165d
        jmp l_00c4166b // 00c41663
    l_00c41665:
        movss dword ptr [esp + 0ch], xmm0 // 00c41665
    l_00c4166b:
        fld dword ptr [ecx] // 00c4166b
        fld dword ptr [esp + 0ch] // 00c4166d
        fld st(0) // 00c41671
        fmulp st(2), st(0) // 00c41673
        fxch st(1) // 00c41675
        fstp dword ptr [ecx] // 00c41677
        fld dword ptr [ecx + 4] // 00c41679
        fmul st(0), st(1) // 00c4167c
        fstp dword ptr [ecx + 4] // 00c4167e
        fmul dword ptr [ecx + 8] // 00c41681
        fstp dword ptr [ecx + 8] // 00c41684
        movss xmm1, dword ptr [edx + 8] // 00c41687
        fld dword ptr [ecx + 054h] // 00c4168c
        movss dword ptr [esp + 03ch], xmm1 // 00c4168f
        fstp dword ptr [esp + 8] // 00c41695
        movss xmm1, dword ptr [edx + 014h] // 00c41699
        fld dword ptr [edx + 8] // 00c4169e
        movss dword ptr [esp + 040h], xmm1 // 00c416a1
        fld dword ptr [esp + 8] // 00c416a7
        movss xmm1, dword ptr [edx + 020h] // 00c416ab
        fld st(0) // 00c416b0
        movss dword ptr [esp + 044h], xmm1 // 00c416b2
        fmulp st(2), st(0) // 00c416b8
        movss xmm1, dword ptr [edx + 0ch] // 00c416ba
        fxch st(1) // 00c416bf
        movss dword ptr [esp + 048h], xmm1 // 00c416c1
        movss xmm1, dword ptr [edx + 018h] // 00c416c7
        fstp dword ptr [esp + 030h] // 00c416cc
        movss dword ptr [esp + 04ch], xmm1 // 00c416d0
        fld dword ptr [edx + 0ch] // 00c416d6
        movss xmm1, dword ptr [edx + 024h] // 00c416d9
        fmul st(0), st(1) // 00c416de
        movss dword ptr [esp + 050h], xmm1 // 00c416e0
        movss xmm1, dword ptr [edx + 010h] // 00c416e6
        movss dword ptr [esp + 054h], xmm1 // 00c416eb
        fstp dword ptr [esp + 034h] // 00c416f1
        movss xmm1, dword ptr [edx + 01ch] // 00c416f5
        movss dword ptr [esp + 058h], xmm1 // 00c416fa
        fmul dword ptr [edx + 010h] // 00c41700
        movss xmm1, dword ptr [edx + 028h] // 00c41703
        movss dword ptr [esp + 05ch], xmm1 // 00c41708
        fstp dword ptr [esp + 038h] // 00c4170e
        fld dword ptr [ecx + 058h] // 00c41712
        fstp dword ptr [esp + 8] // 00c41715
        fld dword ptr [edx + 014h] // 00c41719
        fld dword ptr [esp + 8] // 00c4171c
        fld st(0) // 00c41720
        fmulp st(2), st(0) // 00c41722
        fxch st(1) // 00c41724
        fstp dword ptr [esp + 024h] // 00c41726
        fld dword ptr [edx + 018h] // 00c4172a
        fmul st(0), st(1) // 00c4172d
        fstp dword ptr [esp + 028h] // 00c4172f
        fmul dword ptr [edx + 01ch] // 00c41733
        fstp dword ptr [esp + 02ch] // 00c41736
        fld dword ptr [ecx + 05ch] // 00c4173a
        fstp dword ptr [esp + 8] // 00c4173d
        fld dword ptr [edx + 020h] // 00c41741
        fld dword ptr [esp + 8] // 00c41744
        fld st(0) // 00c41748
        fmulp st(2), st(0) // 00c4174a
        fxch st(1) // 00c4174c
        fstp dword ptr [esp + 018h] // 00c4174e
        fld dword ptr [edx + 024h] // 00c41752
        fmul st(0), st(1) // 00c41755
        fstp dword ptr [esp + 01ch] // 00c41757
        fmul dword ptr [edx + 028h] // 00c4175b
        fstp dword ptr [esp + 020h] // 00c4175e
        fld dword ptr [esp + 040h] // 00c41762
        fld dword ptr [esp + 024h] // 00c41766
        fld st(0) // 00c4176a
        fmulp st(2), st(0) // 00c4176c
        fxch st(1) // 00c4176e
        fst qword ptr [esp + 0b8h] // 00c41770
        fld dword ptr [esp + 030h] // 00c41777
        fld st(0) // 00c4177b
        fld dword ptr [esp + 03ch] // 00c4177d
        fld st(0) // 00c41781
        fmulp st(2), st(0) // 00c41783
        fxch st(1) // 00c41785
        faddp st(3), st(0) // 00c41787
        fld dword ptr [esp + 044h] // 00c41789
        fld st(0) // 00c4178d
        fmul dword ptr [esp + 018h] // 00c4178f
        faddp st(4), st(0) // 00c41793
        fxch st(3) // 00c41795
        fstp dword ptr [ecx + 060h] // 00c41797
        fld dword ptr [esp + 028h] // 00c4179a
        fmul dword ptr [esp + 040h] // 00c4179e
        fst qword ptr [esp + 0d8h] // 00c417a2
        fld dword ptr [esp + 034h] // 00c417a9
        fmul st(0), st(2) // 00c417ad
        faddp st(1), st(0) // 00c417af
        fld dword ptr [esp + 01ch] // 00c417b1
        fmul st(0), st(4) // 00c417b5
        faddp st(1), st(0) // 00c417b7
        fstp dword ptr [ecx + 064h] // 00c417b9
        fld dword ptr [esp + 02ch] // 00c417bc
        fmul dword ptr [esp + 040h] // 00c417c0
        fst qword ptr [esp + 0e8h] // 00c417c4
        fld dword ptr [esp + 038h] // 00c417cb
        fmul st(0), st(2) // 00c417cf
        faddp st(1), st(0) // 00c417d1
        fld dword ptr [esp + 020h] // 00c417d3
        fmul st(0), st(4) // 00c417d7
        faddp st(1), st(0) // 00c417d9
        fstp dword ptr [ecx + 068h] // 00c417db
        fld dword ptr [esp + 04ch] // 00c417de
        fmulp st(4), st(0) // 00c417e2
        fxch st(3) // 00c417e4
        fst qword ptr [esp + 0d0h] // 00c417e6
        fld dword ptr [esp + 048h] // 00c417ed
        fld st(0) // 00c417f1
        fmulp st(3), st(0) // 00c417f3
        fxch st(2) // 00c417f5
        faddp st(1), st(0) // 00c417f7
        fld dword ptr [esp + 050h] // 00c417f9
        fld st(0) // 00c417fd
        fmul dword ptr [esp + 018h] // 00c417ff
        faddp st(2), st(0) // 00c41803
        fxch st(1) // 00c41805
        fstp dword ptr [ecx + 06ch] // 00c41807
        fld dword ptr [esp + 028h] // 00c4180a
        fmul dword ptr [esp + 04ch] // 00c4180e
        fst qword ptr [esp + 0c0h] // 00c41812
        fld dword ptr [esp + 034h] // 00c41819
        fmul st(0), st(3) // 00c4181d
        faddp st(1), st(0) // 00c4181f
        fld dword ptr [esp + 01ch] // 00c41821
        fmul st(0), st(2) // 00c41825
        faddp st(1), st(0) // 00c41827
        fstp dword ptr [ecx + 070h] // 00c41829
        fld dword ptr [esp + 02ch] // 00c4182c
        fmul dword ptr [esp + 04ch] // 00c41830
        fst qword ptr [esp + 0e0h] // 00c41834
        fld dword ptr [esp + 038h] // 00c4183b
        fmul st(0), st(3) // 00c4183f
        faddp st(1), st(0) // 00c41841
        fld dword ptr [esp + 020h] // 00c41843
        fmul st(0), st(2) // 00c41847
        faddp st(1), st(0) // 00c41849
        fstp dword ptr [ecx + 074h] // 00c4184b
        fld dword ptr [esp + 058h] // 00c4184e
        fmul dword ptr [esp + 024h] // 00c41852
        fst qword ptr [esp + 0b0h] // 00c41856
        fld dword ptr [esp + 054h] // 00c4185d
        fmul dword ptr [esp + 030h] // 00c41861
        faddp st(1), st(0) // 00c41865
        fld dword ptr [esp + 05ch] // 00c41867
        fmul dword ptr [esp + 018h] // 00c4186b
        faddp st(1), st(0) // 00c4186f
        fstp dword ptr [ecx + 078h] // 00c41871
        fld dword ptr [esp + 028h] // 00c41874
        fmul dword ptr [esp + 058h] // 00c41878
        fst qword ptr [esp + 0a8h] // 00c4187c
        fld dword ptr [esp + 034h] // 00c41883
        fmul dword ptr [esp + 054h] // 00c41887
        faddp st(1), st(0) // 00c4188b
        fld dword ptr [esp + 01ch] // 00c4188d
        fmul dword ptr [esp + 05ch] // 00c41891
        faddp st(1), st(0) // 00c41895
        fstp dword ptr [ecx + 07ch] // 00c41897
        fld dword ptr [esp + 02ch] // 00c4189a
        fmul dword ptr [esp + 058h] // 00c4189e
        fst qword ptr [esp + 0c8h] // 00c418a2
        fld dword ptr [esp + 038h] // 00c418a9
        fmul dword ptr [esp + 054h] // 00c418ad
        faddp st(1), st(0) // 00c418b1
        fld dword ptr [esp + 020h] // 00c418b3
        fmul dword ptr [esp + 05ch] // 00c418b7
        faddp st(1), st(0) // 00c418bb
        fstp dword ptr [ecx + 080h] // 00c418bd
        fld dword ptr [ecx + 048h] // 00c418c3
        fstp dword ptr [esp + 010h] // 00c418c6
        fld dword ptr [ecx + 044h] // 00c418ca
        fstp dword ptr [esp + 014h] // 00c418cd
        fld dword ptr [ecx + 04ch] // 00c418d1
        fstp dword ptr [esp + 8] // 00c418d4
        fld dword ptr [ecx + 06ch] // 00c418d8
        fmul dword ptr [esp + 010h] // 00c418db
        fld dword ptr [ecx + 060h] // 00c418df
        fmul dword ptr [esp + 014h] // 00c418e2
        faddp st(1), st(0) // 00c418e6
        fld dword ptr [ecx + 078h] // 00c418e8
        fmul dword ptr [esp + 8] // 00c418eb
        faddp st(1), st(0) // 00c418ef
        fstp dword ptr [esp + 06ch] // 00c418f1
        fld dword ptr [ecx + 064h] // 00c418f5
        fmul dword ptr [esp + 014h] // 00c418f8
        fld dword ptr [ecx + 070h] // 00c418fc
        fmul dword ptr [esp + 010h] // 00c418ff
        faddp st(1), st(0) // 00c41903
        fld dword ptr [ecx + 07ch] // 00c41905
        fmul dword ptr [esp + 8] // 00c41908
        faddp st(1), st(0) // 00c4190c
        fstp dword ptr [esp + 070h] // 00c4190e
        fld dword ptr [ecx + 068h] // 00c41912
        fmul dword ptr [esp + 014h] // 00c41915
        fld dword ptr [ecx + 074h] // 00c41919
        fmul dword ptr [esp + 010h] // 00c4191c
        faddp st(1), st(0) // 00c41920
        fld dword ptr [ecx + 080h] // 00c41922
        fmul dword ptr [esp + 8] // 00c41928
        faddp st(1), st(0) // 00c4192c
        fstp dword ptr [esp + 074h] // 00c4192e
        fld dword ptr [esp + 06ch] // 00c41932
        fmul st(0), st(5) // 00c41936
        fstp dword ptr [esp + 084h] // 00c41938
        fld dword ptr [esp + 070h] // 00c4193f
        fmul st(0), st(5) // 00c41943
        fstp dword ptr [esp + 088h] // 00c41945
        fld dword ptr [esp + 074h] // 00c4194c
        fmul st(0), st(5) // 00c41950
        fstp dword ptr [esp + 08ch] // 00c41952
        fld dword ptr [esp + 084h] // 00c41959
        fadd dword ptr [ecx + 0ch] // 00c41960
        fstp dword ptr [esp + 09ch] // 00c41963
        mov eax, dword ptr [esp + 09ch] // 00c4196a
        fld dword ptr [ecx + 010h] // 00c41971
        fadd dword ptr [esp + 088h] // 00c41974
        fstp dword ptr [esp + 0a0h] // 00c4197b
        fld dword ptr [ecx + 014h] // 00c41982
        mov dword ptr [ecx + 0ch], eax // 00c41985
        fadd dword ptr [esp + 08ch] // 00c41988
        mov eax, dword ptr [esp + 0a0h] // 00c4198f
        mov dword ptr [ecx + 010h], eax // 00c41996
        fstp dword ptr [esp + 0a4h] // 00c41999
        mov eax, dword ptr [esp + 0a4h] // 00c419a0
        fld dword ptr [ecx + 0bch] // 00c419a7
        mov dword ptr [ecx + 014h], eax // 00c419ad
        fmul st(0), st(5) // 00c419b0
        fld1  // 00c419b2
        fld st(0) // 00c419b4
        fsubrp st(2), st(0) // 00c419b6
        fxch st(1) // 00c419b8
        fstp dword ptr [esp + 8] // 00c419ba
        fldz  // 00c419be
        fld dword ptr [esp + 8] // 00c419c0
        fcomip st(0), st(1) // 00c419c4
        fstp st(0) // 00c419c6
        jbe l_00c419d8 // 00c419c8
        movss xmm1, dword ptr [esp + 8] // 00c419ca
        movss dword ptr [esp + 0ch], xmm1 // 00c419d0
        jmp l_00c419de // 00c419d6
    l_00c419d8:
        movss dword ptr [esp + 0ch], xmm0 // 00c419d8
    l_00c419de:
        fld dword ptr [esp + 0ch] // 00c419de
        fmul dword ptr [ecx + 0ch] // 00c419e2
        fstp dword ptr [ecx + 0ch] // 00c419e5
        fld dword ptr [esp + 0ch] // 00c419e8
        fmul dword ptr [ecx + 010h] // 00c419ec
        fstp dword ptr [ecx + 010h] // 00c419ef
        fld dword ptr [ecx + 014h] // 00c419f2
        fmul dword ptr [esp + 0ch] // 00c419f5
        fstp dword ptr [ecx + 014h] // 00c419f9
        cmp byte ptr [ecx + 0b4h], 0 // 00c419fc
        je l_00c41aa8 // 00c41a03
        fldz  // 00c41a09
        fmul st(5), st(0) // 00c41a0b
        fmul st(4), st(0) // 00c41a0d
        fld st(5) // 00c41a0f
        fadd qword ptr [esp + 0b8h] // 00c41a11
        fadd st(0), st(5) // 00c41a18
        fstp dword ptr [ecx + 060h] // 00c41a1a
        fld st(5) // 00c41a1d
        fadd qword ptr [esp + 0d8h] // 00c41a1f
        fadd st(0), st(5) // 00c41a26
        fstp dword ptr [ecx + 064h] // 00c41a28
        fxch st(5) // 00c41a2b
        fadd qword ptr [esp + 0e8h] // 00c41a2d
        faddp st(4), st(0) // 00c41a34
        fxch st(3) // 00c41a36
        fstp dword ptr [ecx + 068h] // 00c41a38
        fxch st(1) // 00c41a3b
        fmul st(0), st(3) // 00c41a3d
        fxch st(1) // 00c41a3f
        fmul st(0), st(3) // 00c41a41
        fld st(1) // 00c41a43
        fadd qword ptr [esp + 0d0h] // 00c41a45
        fadd st(0), st(1) // 00c41a4c
        fstp dword ptr [ecx + 06ch] // 00c41a4e
        fld st(1) // 00c41a51
        fadd qword ptr [esp + 0c0h] // 00c41a53
        fadd st(0), st(1) // 00c41a5a
        fstp dword ptr [ecx + 070h] // 00c41a5c
        fxch st(1) // 00c41a5f
        fadd qword ptr [esp + 0e0h] // 00c41a61
        faddp st(1), st(0) // 00c41a68
        fstp dword ptr [ecx + 074h] // 00c41a6a
        fld dword ptr [esp + 054h] // 00c41a6d
        fmul st(0), st(2) // 00c41a71
        fld dword ptr [esp + 05ch] // 00c41a73
        fmulp st(3), st(0) // 00c41a77
        fld qword ptr [esp + 0b0h] // 00c41a79
        fadd st(0), st(1) // 00c41a80
        fadd st(0), st(3) // 00c41a82
        fstp dword ptr [ecx + 078h] // 00c41a84
        fld qword ptr [esp + 0a8h] // 00c41a87
        fadd st(0), st(1) // 00c41a8e
        fadd st(0), st(3) // 00c41a90
        fstp dword ptr [ecx + 07ch] // 00c41a92
        fadd qword ptr [esp + 0c8h] // 00c41a95
        faddp st(2), st(0) // 00c41a9c
        fxch st(1) // 00c41a9e
        fstp dword ptr [ecx + 080h] // 00c41aa0
        jmp l_00c41ab0 // 00c41aa6
    l_00c41aa8:
        fstp st(4) // 00c41aa8
        fstp st(2) // 00c41aaa
        fstp st(0) // 00c41aac
        fstp st(0) // 00c41aae
    l_00c41ab0:
        mov edx, dword ptr [edx + 084h] // 00c41ab0
        cmp edx, edi // 00c41ab6
        jne l_00c41579 // 00c41ab8
        fstp st(1) // 00c41abe
        fstp st(0) // 00c41ac0
    l_00c41ac2:
        pop edi // 00c41ac2
        mov esp, ebp // 00c41ac3
        pop ebp // 00c41ac5
        ret 4 // 00c41ac6
    }
}
// Complete native instruction schedule; comments identify instruction starts.
__declspec(naked) void position_kernel(){
    __asm {
        sub esp, 0fch // 00c5b1b0
        cmp dword ptr [ebx + 0290h], 0 // 00c5b1b6
        push ebp // 00c5b1bd
        push esi // 00c5b1be
        push edi // 00c5b1bf
        je l_00c5bb1e // 00c5b1c0
        mov edi, dword ptr [ebx + 0204h] // 00c5b1c6
        lea ebp, [ebx + 0208h] // 00c5b1cc
        cmp edi, ebp // 00c5b1d2
        je l_00c5bb1e // 00c5b1d4
        fld dword ptr [esp + 010ch] // 00c5b1da
        fld qword ptr constant_00d7a220 // 00c5b1e1
    l_00c5b1e7:
        mov eax, dword ptr [edi + 050h] // 00c5b1e7
        mov esi, dword ptr [edi + 4] // 00c5b1ea
        shr eax, 4 // 00c5b1ed
        test al, 1 // 00c5b1f0
        jne l_00c5bb0c // 00c5b1f2
        fld dword ptr [esi] // 00c5b1f8
        fadd dword ptr [esi + 020h] // 00c5b1fa
        fstp dword ptr [esp + 0f0h] // 00c5b1fd
        fld dword ptr [esi + 024h] // 00c5b204
        fadd dword ptr [esi + 4] // 00c5b207
        fstp dword ptr [esp + 0f4h] // 00c5b20a
        fld dword ptr [esi + 028h] // 00c5b211
        fadd dword ptr [esi + 8] // 00c5b214
        fstp dword ptr [esp + 0f8h] // 00c5b217
        fld dword ptr [esp + 0f0h] // 00c5b21e
        fmul st(0), st(2) // 00c5b225
        fstp dword ptr [esp + 0c0h] // 00c5b227
        fld dword ptr [esp + 0f4h] // 00c5b22e
        fmul st(0), st(2) // 00c5b235
        fstp dword ptr [esp + 0c4h] // 00c5b237
        fld dword ptr [esp + 0f8h] // 00c5b23e
        fmulp st(2), st(0) // 00c5b245
        fxch st(1) // 00c5b247
        fstp dword ptr [esp + 0c8h] // 00c5b249
        fld dword ptr [edi + 02ch] // 00c5b250
        fadd dword ptr [esp + 0c0h] // 00c5b253
        fstp dword ptr [esp + 090h] // 00c5b25a
        mov ecx, dword ptr [esp + 090h] // 00c5b261
        fld dword ptr [edi + 030h] // 00c5b268
        push ecx // 00c5b26b
        fadd dword ptr [esp + 0c8h] // 00c5b26c
        fstp dword ptr [esp + 098h] // 00c5b273
        mov edx, dword ptr [esp + 098h] // 00c5b27a
        fld dword ptr [edi + 034h] // 00c5b281
        mov dword ptr [edi + 02ch], ecx // 00c5b284
        fadd dword ptr [esp + 0cch] // 00c5b287
        mov dword ptr [edi + 030h], edx // 00c5b28e
        fstp dword ptr [esp + 09ch] // 00c5b291
        mov eax, dword ptr [esp + 09ch] // 00c5b298
        mov dword ptr [edi + 034h], eax // 00c5b29f
        fld dword ptr [esi + 02ch] // 00c5b2a2
        fadd dword ptr [esi + 0ch] // 00c5b2a5
        fstp dword ptr [esp + 020h] // 00c5b2a8
        fld dword ptr [esi + 030h] // 00c5b2ac
        fadd dword ptr [esi + 010h] // 00c5b2af
        fstp dword ptr [esp + 024h] // 00c5b2b2
        fld dword ptr [esi + 034h] // 00c5b2b6
        fadd dword ptr [esi + 014h] // 00c5b2b9
        fstp dword ptr [esp + 028h] // 00c5b2bc
        fld dword ptr [esp + 020h] // 00c5b2c0
        fmul st(0), st(1) // 00c5b2c4
        fstp dword ptr [esp + 020h] // 00c5b2c6
        fld dword ptr [esp + 024h] // 00c5b2ca
        fmul st(0), st(1) // 00c5b2ce
        fstp dword ptr [esp + 024h] // 00c5b2d0
        fmul dword ptr [esp + 028h] // 00c5b2d4
        fstp dword ptr [esp + 028h] // 00c5b2d8
        fld dword ptr [esp + 020h] // 00c5b2dc
        fld dword ptr [esp + 024h] // 00c5b2e0
        fld dword ptr [esp + 028h] // 00c5b2e4
        fld st(1) // 00c5b2e8
        fmulp st(2), st(0) // 00c5b2ea
        fld st(2) // 00c5b2ec
        fmulp st(3), st(0) // 00c5b2ee
        fxch st(1) // 00c5b2f0
        faddp st(2), st(0) // 00c5b2f2
        fmul st(0), st(0) // 00c5b2f4
        faddp st(1), st(0) // 00c5b2f6
        fstp dword ptr [esp + 014h] // 00c5b2f8
        fld dword ptr [esp + 014h] // 00c5b2fc
        fstp dword ptr [esp] // 00c5b300
        push dword ptr [esp+276] // Actual CRT access; native local offsets retained.
        call sqrt_kernel // 00c5b303
        fstp dword ptr [esp + 010h] // 00c5b308
        fld dword ptr constant_00d7a310 // 00c5b30c
        fld dword ptr [esp + 010h] // 00c5b312
        fcomi st(0), st(1) // 00c5b316
        fstp st(1) // 00c5b318
        jbe l_00c5b804 // 00c5b31a
        fld dword ptr [esp + 01ch] // 00c5b320
        fdiv st(0), st(1) // 00c5b324
        fstp dword ptr [esp + 01ch] // 00c5b326
        fld dword ptr [esp + 020h] // 00c5b32a
        fdiv st(0), st(1) // 00c5b32e
        fstp dword ptr [esp + 020h] // 00c5b330
        fld dword ptr [esp + 024h] // 00c5b334
        fdiv st(0), st(1) // 00c5b338
        fstp dword ptr [esp + 024h] // 00c5b33a
        fmul dword ptr [esp + 010ch] // 00c5b33e
        fdiv qword ptr constant_00d7a220 // 00c5b345
        fstp dword ptr [esp + 018h] // 00c5b34b
        fld dword ptr [edi + 024h] // 00c5b34f
        fstp dword ptr [esp + 010h] // 00c5b352
        fld dword ptr [edi + 020h] // 00c5b356
        fstp dword ptr [esp + 0ch] // 00c5b359
        fld dword ptr [edi + 028h] // 00c5b35d
        fstp dword ptr [esp + 014h] // 00c5b360
        fld dword ptr [esp + 010h] // 00c5b364
        fld st(0) // 00c5b368
        fld dword ptr [esp + 020h] // 00c5b36a
        fld st(0) // 00c5b36e
        fmulp st(2), st(0) // 00c5b370
        fld dword ptr [esp + 0ch] // 00c5b372
        fld st(0) // 00c5b376
        fld dword ptr [esp + 01ch] // 00c5b378
        fld st(0) // 00c5b37c
        fmulp st(2), st(0) // 00c5b37e
        fxch st(4) // 00c5b380
        faddp st(1), st(0) // 00c5b382
        fld dword ptr [esp + 014h] // 00c5b384
        fld dword ptr [esp + 024h] // 00c5b388
        fld st(0) // 00c5b38c
        fmulp st(2), st(0) // 00c5b38e
        fxch st(2) // 00c5b390
        faddp st(1), st(0) // 00c5b392
        fstp dword ptr [esp + 0ch] // 00c5b394
        fld dword ptr [esp + 0ch] // 00c5b398
        fld st(0) // 00c5b39c
        fmul st(0), st(5) // 00c5b39e
        fstp dword ptr [esp + 040h] // 00c5b3a0
        fld st(3) // 00c5b3a4
        fmul st(0), st(1) // 00c5b3a6
        fstp dword ptr [esp + 044h] // 00c5b3a8
        fmul st(0), st(1) // 00c5b3ac
        fstp dword ptr [esp + 048h] // 00c5b3ae
        fld dword ptr [esp + 040h] // 00c5b3b2
        fsubp st(2), st(0) // 00c5b3b6
        fxch st(1) // 00c5b3b8
        fstp dword ptr [esp + 05ch] // 00c5b3ba
        fld dword ptr [esp + 044h] // 00c5b3be
        fsubp st(4), st(0) // 00c5b3c2
        fxch st(3) // 00c5b3c4
        fstp dword ptr [esp + 060h] // 00c5b3c6
        fld dword ptr [esp + 014h] // 00c5b3ca
        fsub dword ptr [esp + 048h] // 00c5b3ce
        fstp dword ptr [esp + 064h] // 00c5b3d2
        fld dword ptr [esp + 064h] // 00c5b3d6
        fld st(0) // 00c5b3da
        fmul st(0), st(2) // 00c5b3dc
        fld dword ptr [esp + 060h] // 00c5b3de
        fld st(0) // 00c5b3e2
        fmul st(0), st(6) // 00c5b3e4
        fsubp st(2), st(0) // 00c5b3e6
        fxch st(1) // 00c5b3e8
        fstp dword ptr [esp + 06ch] // 00c5b3ea
        fld dword ptr [esp + 05ch] // 00c5b3ee
        fld st(0) // 00c5b3f2
        fmulp st(6), st(0) // 00c5b3f4
        fld st(4) // 00c5b3f6
        fmulp st(3), st(0) // 00c5b3f8
        fxch st(5) // 00c5b3fa
        fsubrp st(2), st(0) // 00c5b3fc
        fxch st(1) // 00c5b3fe
        fstp dword ptr [esp + 070h] // 00c5b400
        fmulp st(2), st(0) // 00c5b404
        fmulp st(2), st(0) // 00c5b406
        fsubrp st(1), st(0) // 00c5b408
        fstp dword ptr [esp + 074h] // 00c5b40a
        fld dword ptr [esp + 018h] // 00c5b40e
        fsin  // 00c5b412
        fstp dword ptr [esp + 030h] // 00c5b414
        fld dword ptr [esp + 06ch] // 00c5b418
        fld dword ptr [esp + 030h] // 00c5b41c
        fld st(0) // 00c5b420
        fmulp st(2), st(0) // 00c5b422
        fxch st(1) // 00c5b424
        fstp dword ptr [esp + 0d8h] // 00c5b426
        fld dword ptr [esp + 070h] // 00c5b42d
        fmul st(0), st(1) // 00c5b431
        fstp dword ptr [esp + 0dch] // 00c5b433
        fmul dword ptr [esp + 074h] // 00c5b43a
        fstp dword ptr [esp + 0e0h] // 00c5b43e
        fld dword ptr [esp + 018h] // 00c5b445
        fcos  // 00c5b449
        fstp dword ptr [esp + 04ch] // 00c5b44b
        fld dword ptr [esp + 04ch] // 00c5b44f
        fld st(0) // 00c5b453
        fmul dword ptr [esp + 05ch] // 00c5b455
        fstp dword ptr [esp + 0a8h] // 00c5b459
        fld dword ptr [esp + 060h] // 00c5b460
        fmul st(0), st(1) // 00c5b464
        fstp dword ptr [esp + 0ach] // 00c5b466
        fmul dword ptr [esp + 064h] // 00c5b46d
        fstp dword ptr [esp + 0b0h] // 00c5b471
        fld dword ptr [esp + 0a8h] // 00c5b478
        fadd dword ptr [esp + 0d8h] // 00c5b47f
        fstp dword ptr [esp + 078h] // 00c5b486
        fld dword ptr [esp + 0ach] // 00c5b48a
        fadd dword ptr [esp + 0dch] // 00c5b491
        fstp dword ptr [esp + 07ch] // 00c5b498
        fld dword ptr [esp + 0b0h] // 00c5b49c
        fadd dword ptr [esp + 0e0h] // 00c5b4a3
        fstp dword ptr [esp + 080h] // 00c5b4aa
        fld dword ptr [esp + 078h] // 00c5b4b1
        fadd dword ptr [esp + 040h] // 00c5b4b5
        fstp dword ptr [esp + 084h] // 00c5b4b9
        mov ecx, dword ptr [esp + 084h] // 00c5b4c0
        fld dword ptr [esp + 07ch] // 00c5b4c7
        mov dword ptr [edi + 020h], ecx // 00c5b4cb
        fadd dword ptr [esp + 044h] // 00c5b4ce
        fstp dword ptr [esp + 088h] // 00c5b4d2
        mov edx, dword ptr [esp + 088h] // 00c5b4d9
        fld dword ptr [esp + 080h] // 00c5b4e0
        mov dword ptr [edi + 024h], edx // 00c5b4e7
        fadd dword ptr [esp + 048h] // 00c5b4ea
        fstp dword ptr [esp + 08ch] // 00c5b4ee
        mov eax, dword ptr [esp + 08ch] // 00c5b4f5
        mov dword ptr [edi + 028h], eax // 00c5b4fc
        fld dword ptr [edi + 018h] // 00c5b4ff
        fstp dword ptr [esp + 0ch] // 00c5b502
        fld dword ptr [edi + 014h] // 00c5b506
        fstp dword ptr [esp + 010h] // 00c5b509
        fld dword ptr [edi + 01ch] // 00c5b50d
        fstp dword ptr [esp + 014h] // 00c5b510
        fld dword ptr [esp + 0ch] // 00c5b514
        fld st(0) // 00c5b518
        fld dword ptr [esp + 020h] // 00c5b51a
        fld st(0) // 00c5b51e
        fmulp st(2), st(0) // 00c5b520
        fld dword ptr [esp + 010h] // 00c5b522
        fld st(0) // 00c5b526
        fld dword ptr [esp + 01ch] // 00c5b528
        fld st(0) // 00c5b52c
        fmulp st(2), st(0) // 00c5b52e
        fxch st(4) // 00c5b530
        faddp st(1), st(0) // 00c5b532
        fld dword ptr [esp + 014h] // 00c5b534
        fld dword ptr [esp + 024h] // 00c5b538
        fld st(0) // 00c5b53c
        fmulp st(2), st(0) // 00c5b53e
        fxch st(2) // 00c5b540
        faddp st(1), st(0) // 00c5b542
        fstp dword ptr [esp + 0ch] // 00c5b544
        fld dword ptr [esp + 0ch] // 00c5b548
        fld st(0) // 00c5b54c
        fmul st(0), st(5) // 00c5b54e
        fstp dword ptr [esp + 034h] // 00c5b550
        fld st(3) // 00c5b554
        fmul st(0), st(1) // 00c5b556
        fstp dword ptr [esp + 038h] // 00c5b558
        fmul st(0), st(1) // 00c5b55c
        fstp dword ptr [esp + 03ch] // 00c5b55e
        fld dword ptr [esp + 034h] // 00c5b562
        fsubp st(2), st(0) // 00c5b566
        fxch st(1) // 00c5b568
        fstp dword ptr [esp + 050h] // 00c5b56a
        fld dword ptr [esp + 038h] // 00c5b56e
        fsubp st(4), st(0) // 00c5b572
        fxch st(3) // 00c5b574
        fstp dword ptr [esp + 054h] // 00c5b576
        fld dword ptr [esp + 014h] // 00c5b57a
        fsub dword ptr [esp + 03ch] // 00c5b57e
        fstp dword ptr [esp + 058h] // 00c5b582
        fld dword ptr [esp + 058h] // 00c5b586
        fld st(0) // 00c5b58a
        fmul st(0), st(2) // 00c5b58c
        fld dword ptr [esp + 054h] // 00c5b58e
        fld st(0) // 00c5b592
        fmul st(0), st(6) // 00c5b594
        fsubp st(2), st(0) // 00c5b596
        fxch st(1) // 00c5b598
        fstp dword ptr [esp + 09ch] // 00c5b59a
        fld dword ptr [esp + 050h] // 00c5b5a1
        fld st(0) // 00c5b5a5
        fmulp st(6), st(0) // 00c5b5a7
        fld st(4) // 00c5b5a9
        fmulp st(3), st(0) // 00c5b5ab
        fxch st(5) // 00c5b5ad
        fsubrp st(2), st(0) // 00c5b5af
        fxch st(1) // 00c5b5b1
        fstp dword ptr [esp + 0a0h] // 00c5b5b3
        fmulp st(2), st(0) // 00c5b5ba
        fmulp st(2), st(0) // 00c5b5bc
        fsubrp st(1), st(0) // 00c5b5be
        fstp dword ptr [esp + 0a4h] // 00c5b5c0
        fld dword ptr [esp + 018h] // 00c5b5c7
        fsin  // 00c5b5cb
        fstp dword ptr [esp + 02ch] // 00c5b5cd
        fld dword ptr [esp + 09ch] // 00c5b5d1
        fld dword ptr [esp + 02ch] // 00c5b5d8
        fld st(0) // 00c5b5dc
        fmulp st(2), st(0) // 00c5b5de
        fxch st(1) // 00c5b5e0
        fstp dword ptr [esp + 0cch] // 00c5b5e2
        fld dword ptr [esp + 0a0h] // 00c5b5e9
        fmul st(0), st(1) // 00c5b5f0
        fstp dword ptr [esp + 0d0h] // 00c5b5f2
        fmul dword ptr [esp + 0a4h] // 00c5b5f9
        fstp dword ptr [esp + 0d4h] // 00c5b600
        fld dword ptr [esp + 018h] // 00c5b607
        fcos  // 00c5b60b
        fstp dword ptr [esp + 068h] // 00c5b60d
        fld dword ptr [esp + 068h] // 00c5b611
        fld st(0) // 00c5b615
        fmul dword ptr [esp + 050h] // 00c5b617
        fstp dword ptr [esp + 0b4h] // 00c5b61b
        fld dword ptr [esp + 054h] // 00c5b622
        fmul st(0), st(1) // 00c5b626
        fstp dword ptr [esp + 0b8h] // 00c5b628
        fmul dword ptr [esp + 058h] // 00c5b62f
        fstp dword ptr [esp + 0bch] // 00c5b633
        fld dword ptr [esp + 0b4h] // 00c5b63a
        fadd dword ptr [esp + 0cch] // 00c5b641
        fstp dword ptr [esp + 0e4h] // 00c5b648
        fld dword ptr [esp + 0b8h] // 00c5b64f
        fadd dword ptr [esp + 0d0h] // 00c5b656
        fstp dword ptr [esp + 0e8h] // 00c5b65d
        fld dword ptr [esp + 0bch] // 00c5b664
        fadd dword ptr [esp + 0d4h] // 00c5b66b
        fstp dword ptr [esp + 0ech] // 00c5b672
        fld dword ptr [esp + 0e4h] // 00c5b679
        fadd dword ptr [esp + 034h] // 00c5b680
        fstp dword ptr [esp + 0fch] // 00c5b684
        mov ecx, dword ptr [esp + 0fch] // 00c5b68b
        fld dword ptr [esp + 0e8h] // 00c5b692
        mov dword ptr [edi + 014h], ecx // 00c5b699
        fadd dword ptr [esp + 038h] // 00c5b69c
        fstp dword ptr [esp + 0100h] // 00c5b6a0
        mov edx, dword ptr [esp + 0100h] // 00c5b6a7
        fld dword ptr [esp + 0ech] // 00c5b6ae
        mov dword ptr [edi + 018h], edx // 00c5b6b5
        fadd dword ptr [esp + 03ch] // 00c5b6b8
        fstp dword ptr [esp + 0104h] // 00c5b6bc
        mov eax, dword ptr [esp + 0104h] // 00c5b6c3
        mov dword ptr [edi + 01ch], eax // 00c5b6ca
        fld dword ptr [edi + 024h] // 00c5b6cd
        fstp dword ptr [esp + 018h] // 00c5b6d0
        fld dword ptr [esp + 018h] // 00c5b6d4
        fld dword ptr [edi + 020h] // 00c5b6d8
        fstp dword ptr [esp + 014h] // 00c5b6db
        fld dword ptr [esp + 014h] // 00c5b6df
        push ecx // 00c5b6e3
        fld dword ptr [edi + 028h] // 00c5b6e4
        fstp dword ptr [esp + 014h] // 00c5b6e7
        fld dword ptr [esp + 014h] // 00c5b6eb
        fld st(1) // 00c5b6ef
        fmulp st(2), st(0) // 00c5b6f1
        fld st(2) // 00c5b6f3
        fmulp st(3), st(0) // 00c5b6f5
        fxch st(1) // 00c5b6f7
        faddp st(2), st(0) // 00c5b6f9
        fmul st(0), st(0) // 00c5b6fb
        faddp st(1), st(0) // 00c5b6fd
        fstp dword ptr [esp + 010h] // 00c5b6ff
        fld dword ptr [esp + 010h] // 00c5b703
        fstp dword ptr [esp] // 00c5b707
        push dword ptr [esp+276] // Actual CRT access; native local offsets retained.
        call sqrt_kernel // 00c5b70a
        fstp dword ptr [esp + 0ch] // 00c5b70f
        push ecx // 00c5b713
        fld dword ptr [esp + 018h] // 00c5b714
        fld dword ptr [esp + 010h] // 00c5b718
        fld st(0) // 00c5b71c
        fdivp st(2), st(0) // 00c5b71e
        fxch st(1) // 00c5b720
        fstp dword ptr [edi + 020h] // 00c5b722
        fld dword ptr [esp + 01ch] // 00c5b725
        fdiv st(0), st(1) // 00c5b729
        fstp dword ptr [edi + 024h] // 00c5b72b
        fdivr dword ptr [esp + 014h] // 00c5b72e
        fstp dword ptr [edi + 028h] // 00c5b732
        fld dword ptr [edi + 018h] // 00c5b735
        fmul dword ptr [edi + 028h] // 00c5b738
        fld dword ptr [edi + 01ch] // 00c5b73b
        fmul dword ptr [edi + 024h] // 00c5b73e
        fsubp st(1), st(0) // 00c5b741
        fstp dword ptr [edi + 8] // 00c5b743
        fld dword ptr [edi + 020h] // 00c5b746
        fmul dword ptr [edi + 01ch] // 00c5b749
        fld dword ptr [edi + 028h] // 00c5b74c
        fmul dword ptr [edi + 014h] // 00c5b74f
        fsubp st(1), st(0) // 00c5b752
        fstp dword ptr [edi + 0ch] // 00c5b754
        fld dword ptr [edi + 014h] // 00c5b757
        fmul dword ptr [edi + 024h] // 00c5b75a
        fld dword ptr [edi + 018h] // 00c5b75d
        fmul dword ptr [edi + 020h] // 00c5b760
        fsubp st(1), st(0) // 00c5b763
        fstp dword ptr [esp + 010h] // 00c5b765
        fld dword ptr [esp + 010h] // 00c5b769
        fst dword ptr [edi + 010h] // 00c5b76d
        fld dword ptr [edi + 0ch] // 00c5b770
        fld dword ptr [edi + 8] // 00c5b773
        fstp dword ptr [esp + 014h] // 00c5b776
        fld dword ptr [esp + 014h] // 00c5b77a
        fld st(1) // 00c5b77e
        fmulp st(2), st(0) // 00c5b780
        fmul st(0), st(0) // 00c5b782
        faddp st(1), st(0) // 00c5b784
        fld st(1) // 00c5b786
        fmulp st(2), st(0) // 00c5b788
        faddp st(1), st(0) // 00c5b78a
        fstp dword ptr [esp + 010h] // 00c5b78c
        fld dword ptr [esp + 010h] // 00c5b790
        fstp dword ptr [esp] // 00c5b794
        push dword ptr [esp+276] // Actual CRT access; native local offsets retained.
        call sqrt_kernel // 00c5b797
        fstp dword ptr [esp + 0ch] // 00c5b79c
        fld dword ptr [esp + 010h] // 00c5b7a0
        fld dword ptr [esp + 0ch] // 00c5b7a4
        fld st(0) // 00c5b7a8
        fdivp st(2), st(0) // 00c5b7aa
        fxch st(1) // 00c5b7ac
        fstp dword ptr [edi + 8] // 00c5b7ae
        fld dword ptr [edi + 0ch] // 00c5b7b1
        fdiv st(0), st(1) // 00c5b7b4
        fstp dword ptr [esp + 0ch] // 00c5b7b6
        fld dword ptr [esp + 0ch] // 00c5b7ba
        fst dword ptr [edi + 0ch] // 00c5b7be
        fld dword ptr [edi + 010h] // 00c5b7c1
        fdivrp st(2), st(0) // 00c5b7c4
        fxch st(1) // 00c5b7c6
        fstp dword ptr [esp + 0ch] // 00c5b7c8
        fld dword ptr [esp + 0ch] // 00c5b7cc
        fst dword ptr [edi + 010h] // 00c5b7d0
        fmul dword ptr [edi + 024h] // 00c5b7d3
        fld dword ptr [edi + 028h] // 00c5b7d6
        fmulp st(2), st(0) // 00c5b7d9
        fsubrp st(1), st(0) // 00c5b7db
        fstp dword ptr [edi + 014h] // 00c5b7dd
        fld dword ptr [edi + 8] // 00c5b7e0
        fmul dword ptr [edi + 028h] // 00c5b7e3
        fld dword ptr [edi + 020h] // 00c5b7e6
        fmul dword ptr [edi + 010h] // 00c5b7e9
        fsubp st(1), st(0) // 00c5b7ec
        fstp dword ptr [edi + 018h] // 00c5b7ee
        fld dword ptr [edi + 0ch] // 00c5b7f1
        fmul dword ptr [edi + 020h] // 00c5b7f4
        fld dword ptr [edi + 024h] // 00c5b7f7
        fmul dword ptr [edi + 8] // 00c5b7fa
        fsubp st(1), st(0) // 00c5b7fd
        fstp dword ptr [edi + 01ch] // 00c5b7ff
        jmp l_00c5b806 // 00c5b802
    l_00c5b804:
        fstp st(0) // 00c5b804
    l_00c5b806:
        fld dword ptr [esi + 0bch] // 00c5b806
        fld dword ptr [esp + 010ch] // 00c5b80c
        fld st(0) // 00c5b813
        fmulp st(2), st(0) // 00c5b815
        fld1  // 00c5b817
        fld st(0) // 00c5b819
        fsubrp st(3), st(0) // 00c5b81b
        fxch st(2) // 00c5b81d
        fstp dword ptr [esp + 010h] // 00c5b81f
        fldz  // 00c5b823
        fld dword ptr [esp + 010h] // 00c5b825
        fcomip st(0), st(1) // 00c5b829
        jbe l_00c5b83e // 00c5b82b
        movss xmm0, dword ptr [esp + 010h] // 00c5b82d
        movss dword ptr [esp + 028h], xmm0 // 00c5b833
        xorps xmm0, xmm0 // 00c5b839
        jmp l_00c5b847 // 00c5b83c
    l_00c5b83e:
        xorps xmm0, xmm0 // 00c5b83e
        movss dword ptr [esp + 028h], xmm0 // 00c5b841
    l_00c5b847:
        fld dword ptr [esp + 028h] // 00c5b847
        fld st(0) // 00c5b84b
        fmul dword ptr [esi + 0ch] // 00c5b84d
        fstp dword ptr [esi + 0ch] // 00c5b850
        fld dword ptr [esi + 010h] // 00c5b853
        fmul st(0), st(1) // 00c5b856
        fstp dword ptr [esi + 010h] // 00c5b858
        fmul dword ptr [esi + 014h] // 00c5b85b
        fstp dword ptr [esi + 014h] // 00c5b85e
        fld dword ptr [esi + 0b8h] // 00c5b861
        fmul st(0), st(2) // 00c5b867
        fsubp st(3), st(0) // 00c5b869
        fxch st(2) // 00c5b86b
        fstp dword ptr [esp + 010h] // 00c5b86d
        fld dword ptr [esp + 010h] // 00c5b871
        fcomip st(0), st(2) // 00c5b875
        fstp st(1) // 00c5b877
        jbe l_00c5b889 // 00c5b879
        movss xmm1, dword ptr [esp + 010h] // 00c5b87b
        movss dword ptr [esp + 028h], xmm1 // 00c5b881
        jmp l_00c5b88f // 00c5b887
    l_00c5b889:
        movss dword ptr [esp + 028h], xmm0 // 00c5b889
    l_00c5b88f:
        fld dword ptr [esi] // 00c5b88f
        fld dword ptr [esp + 028h] // 00c5b891
        fld st(0) // 00c5b895
        fmulp st(2), st(0) // 00c5b897
        fxch st(1) // 00c5b899
        fstp dword ptr [esp + 0ch] // 00c5b89b
        fld dword ptr [esp + 0ch] // 00c5b89f
        fst dword ptr [esi] // 00c5b8a3
        fld dword ptr [esi + 4] // 00c5b8a5
        fmul st(0), st(2) // 00c5b8a8
        fstp dword ptr [esp + 0ch] // 00c5b8aa
        fld dword ptr [esp + 0ch] // 00c5b8ae
        fst dword ptr [esi + 4] // 00c5b8b2
        fld dword ptr [esi + 8] // 00c5b8b5
        fmulp st(3), st(0) // 00c5b8b8
        fxch st(2) // 00c5b8ba
        fstp dword ptr [esp + 0ch] // 00c5b8bc
        fld dword ptr [esp + 0ch] // 00c5b8c0
        fst dword ptr [esi + 8] // 00c5b8c4
        fld dword ptr [esi + 010h] // 00c5b8c7
        fld dword ptr [esi + 0ch] // 00c5b8ca
        fld dword ptr [esi + 014h] // 00c5b8cd
        fld st(1) // 00c5b8d0
        fmulp st(2), st(0) // 00c5b8d2
        fld st(2) // 00c5b8d4
        fmulp st(3), st(0) // 00c5b8d6
        fxch st(1) // 00c5b8d8
        faddp st(2), st(0) // 00c5b8da
        fmul st(0), st(0) // 00c5b8dc
        faddp st(1), st(0) // 00c5b8de
        fstp dword ptr [esp + 014h] // 00c5b8e0
        fld st(1) // 00c5b8e4
        fmulp st(2), st(0) // 00c5b8e6
        fld st(2) // 00c5b8e8
        fmulp st(3), st(0) // 00c5b8ea
        fxch st(1) // 00c5b8ec
        faddp st(2), st(0) // 00c5b8ee
        fmul st(0), st(0) // 00c5b8f0
        faddp st(1), st(0) // 00c5b8f2
        fstp dword ptr [esp + 0ch] // 00c5b8f4
        fld dword ptr [esi + 018h] // 00c5b8f8
        fstp dword ptr [esp + 010h] // 00c5b8fb
        fld dword ptr [esp + 010h] // 00c5b8ff
        fld dword ptr [esp + 0ch] // 00c5b903
        fld st(1) // 00c5b907
        fmulp st(2), st(0) // 00c5b909
        fcomi st(0), st(1) // 00c5b90b
        fstp st(1) // 00c5b90d
        jbe l_00c5b978 // 00c5b90f
        fstp st(1) // 00c5b911
        push ecx // 00c5b913
        fstp dword ptr [esp] // 00c5b914
        push dword ptr [esp+276] // Actual CRT access; native local offsets retained.
        call sqrt_kernel // 00c5b917
        fstp dword ptr [esp + 0ch] // 00c5b91c
        xorps xmm0, xmm0 // 00c5b920
        fld dword ptr [esp + 010h] // 00c5b923
        fstp dword ptr [esp + 010h] // 00c5b927
        fld dword ptr [esi] // 00c5b92b
        fld dword ptr [esp + 0ch] // 00c5b92d
        fld st(0) // 00c5b931
        fdivp st(2), st(0) // 00c5b933
        fxch st(1) // 00c5b935
        fstp dword ptr [esp + 0ch] // 00c5b937
        fld dword ptr [esp + 0ch] // 00c5b93b
        fld dword ptr [esp + 010h] // 00c5b93f
        fld st(0) // 00c5b943
        fmulp st(2), st(0) // 00c5b945
        fxch st(1) // 00c5b947
        fstp dword ptr [esi] // 00c5b949
        fld dword ptr [esi + 4] // 00c5b94b
        fdiv st(0), st(2) // 00c5b94e
        fstp dword ptr [esp + 0ch] // 00c5b950
        fld dword ptr [esp + 0ch] // 00c5b954
        fmul st(0), st(1) // 00c5b958
        fstp dword ptr [esi + 4] // 00c5b95a
        fld dword ptr [esi + 8] // 00c5b95d
        fdivrp st(2), st(0) // 00c5b960
        fxch st(1) // 00c5b962
        fstp dword ptr [esp + 0ch] // 00c5b964
        fmul dword ptr [esp + 0ch] // 00c5b968
        fstp dword ptr [esi + 8] // 00c5b96c
        fld dword ptr [esp + 010ch] // 00c5b96f
        jmp l_00c5b97a // 00c5b976
    l_00c5b978:
        fstp st(0) // 00c5b978
    l_00c5b97a:
        fld dword ptr [esi + 01ch] // 00c5b97a
        fld dword ptr [esp + 014h] // 00c5b97d
        fld st(1) // 00c5b981
        fmulp st(2), st(0) // 00c5b983
        fcomip st(0), st(1) // 00c5b985
        fstp st(0) // 00c5b987
        jbe l_00c5ba23 // 00c5b989
        fstp st(0) // 00c5b98f
        push ecx // 00c5b991
        fld dword ptr [esi + 010h] // 00c5b992
        fstp dword ptr [esp + 018h] // 00c5b995
        fld dword ptr [esp + 018h] // 00c5b999
        fld dword ptr [esi + 0ch] // 00c5b99d
        fstp dword ptr [esp + 014h] // 00c5b9a0
        fld dword ptr [esp + 014h] // 00c5b9a4
        fld dword ptr [esi + 014h] // 00c5b9a8
        fstp dword ptr [esp + 01ch] // 00c5b9ab
        fld dword ptr [esp + 01ch] // 00c5b9af
        fld st(1) // 00c5b9b3
        fmulp st(2), st(0) // 00c5b9b5
        fld st(2) // 00c5b9b7
        fmulp st(3), st(0) // 00c5b9b9
        fxch st(1) // 00c5b9bb
        faddp st(2), st(0) // 00c5b9bd
        fmul st(0), st(0) // 00c5b9bf
        faddp st(1), st(0) // 00c5b9c1
        fstp dword ptr [esp + 010h] // 00c5b9c3
        fld dword ptr [esp + 010h] // 00c5b9c7
        fstp dword ptr [esp] // 00c5b9cb
        push dword ptr [esp+276] // Actual CRT access; native local offsets retained.
        call sqrt_kernel // 00c5b9ce
        fstp dword ptr [esp + 0ch] // 00c5b9d3
        xorps xmm0, xmm0 // 00c5b9d7
        fld dword ptr [esp + 010h] // 00c5b9da
        fld dword ptr [esp + 0ch] // 00c5b9de
        fld st(0) // 00c5b9e2
        fdivp st(2), st(0) // 00c5b9e4
        fxch st(1) // 00c5b9e6
        fstp dword ptr [esi + 0ch] // 00c5b9e8
        fld dword ptr [esp + 014h] // 00c5b9eb
        fdiv st(0), st(1) // 00c5b9ef
        fstp dword ptr [esi + 010h] // 00c5b9f1
        fdivr dword ptr [esp + 018h] // 00c5b9f4
        fstp dword ptr [esi + 014h] // 00c5b9f8
        fld dword ptr [esi + 01ch] // 00c5b9fb
        fstp dword ptr [esp + 0ch] // 00c5b9fe
        fld dword ptr [esp + 0ch] // 00c5ba02
        fld st(0) // 00c5ba06
        fmul dword ptr [esi + 0ch] // 00c5ba08
        fstp dword ptr [esi + 0ch] // 00c5ba0b
        fld dword ptr [esi + 010h] // 00c5ba0e
        fmul st(0), st(1) // 00c5ba11
        fstp dword ptr [esi + 010h] // 00c5ba13
        fmul dword ptr [esi + 014h] // 00c5ba16
        fstp dword ptr [esi + 014h] // 00c5ba19
        fld dword ptr [esp + 010ch] // 00c5ba1c
    l_00c5ba23:
        fld dword ptr [esi + 010h] // 00c5ba23
        fld dword ptr [esi + 0ch] // 00c5ba26
        fld dword ptr [esi + 014h] // 00c5ba29
        fld dword ptr [ebx + 040h] // 00c5ba2c
        fld st(2) // 00c5ba2f
        fmulp st(3), st(0) // 00c5ba31
        fld st(3) // 00c5ba33
        fmulp st(4), st(0) // 00c5ba35
        fxch st(2) // 00c5ba37
        faddp st(3), st(0) // 00c5ba39
        fmul st(0), st(0) // 00c5ba3b
        faddp st(2), st(0) // 00c5ba3d
        fxch st(1) // 00c5ba3f
        fstp dword ptr [esp + 0ch] // 00c5ba41
        fld dword ptr [esp + 0ch] // 00c5ba45
        fld st(1) // 00c5ba49
        fmulp st(2), st(0) // 00c5ba4b
        fcomip st(0), st(1) // 00c5ba4d
        fstp st(0) // 00c5ba4f
        ja l_00c5bac0 // 00c5ba51
        fld dword ptr [esi + 4] // 00c5ba53
        fld dword ptr [esi] // 00c5ba56
        fld dword ptr [esi + 8] // 00c5ba58
        fld dword ptr [ebx + 03ch] // 00c5ba5b
        fld st(3) // 00c5ba5e
        fmulp st(4), st(0) // 00c5ba60
        fld st(2) // 00c5ba62
        fmulp st(3), st(0) // 00c5ba64
        fxch st(3) // 00c5ba66
        faddp st(2), st(0) // 00c5ba68
        fmul st(0), st(0) // 00c5ba6a
        faddp st(1), st(0) // 00c5ba6c
        fstp dword ptr [esp + 0ch] // 00c5ba6e
        fld dword ptr [esp + 0ch] // 00c5ba72
        fld st(1) // 00c5ba76
        fmulp st(2), st(0) // 00c5ba78
        fcomip st(0), st(1) // 00c5ba7a
        fstp st(0) // 00c5ba7c
        ja l_00c5bac0 // 00c5ba7e
        add dword ptr [edi + 054h], -1 // 00c5ba80
        jns l_00c5baca // 00c5ba84
        fld dword ptr [esi] // 00c5ba86
        fld qword ptr constant_00d7a390 // 00c5ba88
        fmul st(1), st(0) // 00c5ba8e
        fxch st(1) // 00c5ba90
        fstp dword ptr [esi] // 00c5ba92
        fld dword ptr [esi + 4] // 00c5ba94
        fmul st(0), st(1) // 00c5ba97
        fstp dword ptr [esi + 4] // 00c5ba99
        fld dword ptr [esi + 8] // 00c5ba9c
        fmul st(0), st(1) // 00c5ba9f
        fstp dword ptr [esi + 8] // 00c5baa1
        fld dword ptr [esi + 0ch] // 00c5baa4
        fmul st(0), st(1) // 00c5baa7
        fstp dword ptr [esi + 0ch] // 00c5baa9
        fld dword ptr [esi + 010h] // 00c5baac
        fmul st(0), st(1) // 00c5baaf
        fstp dword ptr [esi + 010h] // 00c5bab1
        fmul dword ptr [esi + 014h] // 00c5bab4
        fstp dword ptr [esi + 014h] // 00c5bab7
        or dword ptr [edi + 050h], 2 // 00c5baba
        jmp l_00c5baca // 00c5babe
    l_00c5bac0:
        and dword ptr [edi + 050h], 0ffffffedh // 00c5bac0
        mov ecx, dword ptr [ebx + 044h] // 00c5bac4
        mov dword ptr [edi + 054h], ecx // 00c5bac7
    l_00c5baca:
        fld qword ptr constant_00d7a220 // 00c5baca
        movss dword ptr [esi + 040h], xmm0 // 00c5bad0
        movss dword ptr [esi + 03ch], xmm0 // 00c5bad5
        movss dword ptr [esi + 038h], xmm0 // 00c5bada
        movss dword ptr [esi + 04ch], xmm0 // 00c5badf
        movss dword ptr [esi + 048h], xmm0 // 00c5bae4
        movss dword ptr [esi + 044h], xmm0 // 00c5bae9
        movss dword ptr [esi + 028h], xmm0 // 00c5baee
        movss dword ptr [esi + 024h], xmm0 // 00c5baf3
        movss dword ptr [esi + 020h], xmm0 // 00c5baf8
        movss dword ptr [esi + 034h], xmm0 // 00c5bafd
        movss dword ptr [esi + 030h], xmm0 // 00c5bb02
        movss dword ptr [esi + 02ch], xmm0 // 00c5bb07
    l_00c5bb0c:
        mov edi, dword ptr [edi + 084h] // 00c5bb0c
        cmp edi, ebp // 00c5bb12
        jne l_00c5b1e7 // 00c5bb14
        fstp st(0) // 00c5bb1a
        fstp st(0) // 00c5bb1c
    l_00c5bb1e:
        pop edi // 00c5bb1e
        pop esi // 00c5bb1f
        pop ebp // 00c5bb20
        add esp, 0fch // 00c5bb21
        ret 8 // 00c5bb27
    }
}
} // namespace
void native_dyn_integrate_velocities_00c41550(void* world,float dt){
    __asm {mov esi,world}
    __asm {push dt}
    __asm {call velocity_kernel}
}
void native_dyn_integrate_positions_00c5b1b0(void* world,float dt,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;
    __asm {mov ebx,world}
    __asm {push c}
    __asm {push dt}
    __asm {call position_kernel}
}
} // namespace bsp
