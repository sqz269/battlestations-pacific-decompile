#include "bsp/native_dyn_terrain_convex.hpp"
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native primitive dispatch requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
alignas(8) const std::uint32_t constant_00d7a208=0x80000000U;
alignas(8) const std::uint32_t constant_00d7a240=0xc47a0000U;
alignas(8) const std::uint64_t constant_00d7a250=0xbff0000000000000ULL;
// Existing float-sqrt boundary with explicit context before its float argument.
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
// Complete original instruction schedule; address comments are native starts.
__declspec(naked) void terrain_convex_kernel(){
    __asm {
        push ebp // 00c53630
        mov ebp, esp // 00c53631
        and esp, 0fffffff8h // 00c53633
        sub esp, 0104h // 00c53636
        push ebx // 00c5363c
        push esi // 00c5363d
        push edi // 00c5363e
        mov edi, dword ptr [ebp + 0ch] // 00c5363f
        cmp dword ptr [edi + 8], 5 // 00c53642
        jne l_00c53af1 // 00c53646
        mov eax, dword ptr [ebp + 010h] // 00c5364c
        fld dword ptr [edi + 038h] // 00c5364f
        fstp dword ptr [esp + 01ch] // 00c53652
        mov ebx, edi // 00c53656
        fld dword ptr [eax + 0ch] // 00c53658
        fstp dword ptr [esp + 018h] // 00c5365b
        fld dword ptr [edi + 034h] // 00c5365f
        fstp dword ptr [esp + 028h] // 00c53662
        fld dword ptr [eax] // 00c53666
        fstp dword ptr [esp + 010h] // 00c53668
        fld dword ptr [edi + 03ch] // 00c5366c
        fstp dword ptr [esp + 0ch] // 00c5366f
        fld dword ptr [eax + 018h] // 00c53673
        fstp dword ptr [esp + 020h] // 00c53676
        fld dword ptr [esp + 010h] // 00c5367a
        fld st(0) // 00c5367e
        fld dword ptr [esp + 028h] // 00c53680
        fld st(0) // 00c53684
        fmulp st(2), st(0) // 00c53686
        fld dword ptr [esp + 018h] // 00c53688
        fld st(0) // 00c5368c
        fld dword ptr [esp + 01ch] // 00c5368e
        fld st(0) // 00c53692
        fmulp st(2), st(0) // 00c53694
        fxch st(4) // 00c53696
        faddp st(1), st(0) // 00c53698
        fld dword ptr [esp + 020h] // 00c5369a
        fld st(0) // 00c5369e
        fmul dword ptr [esp + 0ch] // 00c536a0
        faddp st(2), st(0) // 00c536a4
        fxch st(1) // 00c536a6
        fstp dword ptr [esp + 054h] // 00c536a8
        fld dword ptr [eax + 010h] // 00c536ac
        fstp dword ptr [esp + 020h] // 00c536af
        fld dword ptr [eax + 4] // 00c536b3
        fstp dword ptr [esp + 018h] // 00c536b6
        fld dword ptr [eax + 01ch] // 00c536ba
        fstp dword ptr [esp + 034h] // 00c536bd
        fld dword ptr [esp + 018h] // 00c536c1
        fld st(0) // 00c536c5
        fmulp st(4), st(0) // 00c536c7
        fld dword ptr [esp + 020h] // 00c536c9
        fld st(0) // 00c536cd
        fmulp st(6), st(0) // 00c536cf
        fxch st(4) // 00c536d1
        faddp st(5), st(0) // 00c536d3
        fld dword ptr [esp + 034h] // 00c536d5
        fmul dword ptr [esp + 0ch] // 00c536d9
        faddp st(5), st(0) // 00c536dd
        fxch st(4) // 00c536df
        fstp dword ptr [esp + 058h] // 00c536e1
        fld dword ptr [eax + 014h] // 00c536e5
        fstp dword ptr [esp + 030h] // 00c536e8
        fld dword ptr [eax + 8] // 00c536ec
        fstp dword ptr [esp + 02ch] // 00c536ef
        fld dword ptr [eax + 020h] // 00c536f3
        fstp dword ptr [esp + 024h] // 00c536f6
        fld dword ptr [esp + 02ch] // 00c536fa
        fmul dword ptr [esp + 028h] // 00c536fe
        fld dword ptr [esp + 030h] // 00c53702
        fmul dword ptr [esp + 01ch] // 00c53706
        faddp st(1), st(0) // 00c5370a
        fld dword ptr [esp + 024h] // 00c5370c
        fmul dword ptr [esp + 0ch] // 00c53710
        faddp st(1), st(0) // 00c53714
        fstp dword ptr [esp + 05ch] // 00c53716
        fld dword ptr [edi + 044h] // 00c5371a
        fstp dword ptr [esp + 010h] // 00c5371d
        fld dword ptr [edi + 040h] // 00c53721
        fstp dword ptr [esp + 0ch] // 00c53724
        fld dword ptr [edi + 048h] // 00c53728
        fstp dword ptr [esp + 01ch] // 00c5372b
        fld dword ptr [esp + 0ch] // 00c5372f
        fmul st(0), st(5) // 00c53733
        fld dword ptr [esp + 010h] // 00c53735
        fmul st(0), st(3) // 00c53739
        faddp st(1), st(0) // 00c5373b
        fld dword ptr [esp + 01ch] // 00c5373d
        fmul st(0), st(2) // 00c53741
        faddp st(1), st(0) // 00c53743
        fstp dword ptr [esp + 060h] // 00c53745
        fld dword ptr [esp + 0ch] // 00c53749
        fmul st(0), st(4) // 00c5374d
        fld dword ptr [esp + 010h] // 00c5374f
        fmul st(0), st(4) // 00c53753
        faddp st(1), st(0) // 00c53755
        fld dword ptr [esp + 01ch] // 00c53757
        fmul dword ptr [esp + 034h] // 00c5375b
        faddp st(1), st(0) // 00c5375f
        fstp dword ptr [esp + 064h] // 00c53761
        fld dword ptr [esp + 0ch] // 00c53765
        fmul dword ptr [esp + 02ch] // 00c53769
        fld dword ptr [esp + 010h] // 00c5376d
        fmul dword ptr [esp + 030h] // 00c53771
        faddp st(1), st(0) // 00c53775
        fld dword ptr [esp + 01ch] // 00c53777
        fmul dword ptr [esp + 024h] // 00c5377b
        faddp st(1), st(0) // 00c5377f
        fstp dword ptr [esp + 068h] // 00c53781
        fld dword ptr [edi + 050h] // 00c53785
        fstp dword ptr [esp + 010h] // 00c53788
        fld dword ptr [edi + 04ch] // 00c5378c
        fstp dword ptr [esp + 0ch] // 00c5378f
        fld dword ptr [edi + 054h] // 00c53793
        fstp dword ptr [esp + 01ch] // 00c53796
        fld dword ptr [esp + 0ch] // 00c5379a
        fmul st(0), st(5) // 00c5379e
        fld dword ptr [esp + 010h] // 00c537a0
        fmul st(0), st(3) // 00c537a4
        faddp st(1), st(0) // 00c537a6
        fld dword ptr [esp + 01ch] // 00c537a8
        fmul st(0), st(2) // 00c537ac
        faddp st(1), st(0) // 00c537ae
        fstp dword ptr [esp + 06ch] // 00c537b0
        fld dword ptr [esp + 0ch] // 00c537b4
        fmul st(0), st(4) // 00c537b8
        fld dword ptr [esp + 010h] // 00c537ba
        fmul st(0), st(4) // 00c537be
        faddp st(1), st(0) // 00c537c0
        fld dword ptr [esp + 01ch] // 00c537c2
        fmul dword ptr [esp + 034h] // 00c537c6
        faddp st(1), st(0) // 00c537ca
        fstp dword ptr [esp + 070h] // 00c537cc
        fld dword ptr [esp + 0ch] // 00c537d0
        fmul dword ptr [esp + 02ch] // 00c537d4
        fld dword ptr [esp + 010h] // 00c537d8
        fmul dword ptr [esp + 030h] // 00c537dc
        faddp st(1), st(0) // 00c537e0
        fld dword ptr [esp + 01ch] // 00c537e2
        fmul dword ptr [esp + 024h] // 00c537e6
        faddp st(1), st(0) // 00c537ea
        fstp dword ptr [esp + 074h] // 00c537ec
        fld dword ptr [edi + 05ch] // 00c537f0
        fstp dword ptr [esp + 020h] // 00c537f3
        fld dword ptr [edi + 058h] // 00c537f7
        fstp dword ptr [esp + 018h] // 00c537fa
        fld dword ptr [edi + 060h] // 00c537fe
        fstp dword ptr [esp + 014h] // 00c53801
        fld dword ptr [esp + 018h] // 00c53805
        fld st(0) // 00c53809
        fmulp st(6), st(0) // 00c5380b
        fld dword ptr [esp + 020h] // 00c5380d
        fld st(0) // 00c53811
        fmulp st(4), st(0) // 00c53813
        fxch st(6) // 00c53815
        faddp st(3), st(0) // 00c53817
        fld dword ptr [esp + 014h] // 00c53819
        fld st(0) // 00c5381d
        fmulp st(3), st(0) // 00c5381f
        fxch st(3) // 00c53821
        faddp st(2), st(0) // 00c53823
        fld dword ptr [eax + 024h] // 00c53825
        faddp st(2), st(0) // 00c53828
        fxch st(1) // 00c5382a
        fstp dword ptr [esp + 078h] // 00c5382c
        fld st(0) // 00c53830
        fmulp st(4), st(0) // 00c53832
        fld st(4) // 00c53834
        fmulp st(3), st(0) // 00c53836
        fxch st(3) // 00c53838
        faddp st(2), st(0) // 00c5383a
        fld st(0) // 00c5383c
        fmul dword ptr [esp + 034h] // 00c5383e
        mov edi, dword ptr [ebp + 014h] // 00c53842
        mov ecx, edi // 00c53845
        add edi, 034h // 00c53847
        faddp st(2), st(0) // 00c5384a
        fld dword ptr [eax + 028h] // 00c5384c
        faddp st(2), st(0) // 00c5384f
        fxch st(1) // 00c53851
        fstp dword ptr [esp + 07ch] // 00c53853
        fld dword ptr [esp + 02ch] // 00c53857
        fmulp st(2), st(0) // 00c5385b
        fld dword ptr [esp + 030h] // 00c5385d
        fmulp st(3), st(0) // 00c53861
        fxch st(1) // 00c53863
        faddp st(2), st(0) // 00c53865
        fmul dword ptr [esp + 024h] // 00c53867
        faddp st(1), st(0) // 00c5386b
        fadd dword ptr [eax + 02ch] // 00c5386d
        mov eax, dword ptr [ebp + 018h] // 00c53870
        fstp dword ptr [esp + 080h] // 00c53873
        fld dword ptr [edi + 4] // 00c5387a
        fstp dword ptr [esp + 014h] // 00c5387d
        fld dword ptr [eax + 0ch] // 00c53881
        fstp dword ptr [esp + 020h] // 00c53884
        fld dword ptr [edi] // 00c53888
        fstp dword ptr [esp + 018h] // 00c5388a
        fld dword ptr [eax] // 00c5388e
        fstp dword ptr [esp + 0ch] // 00c53890
        fld dword ptr [edi + 8] // 00c53894
        fstp dword ptr [esp + 010h] // 00c53897
        fld dword ptr [eax + 018h] // 00c5389b
        fstp dword ptr [esp + 028h] // 00c5389e
        fld dword ptr [esp + 0ch] // 00c538a2
        fld st(0) // 00c538a6
        fld dword ptr [esp + 018h] // 00c538a8
        fld st(0) // 00c538ac
        fmulp st(2), st(0) // 00c538ae
        fld dword ptr [esp + 020h] // 00c538b0
        fld st(0) // 00c538b4
        fld dword ptr [esp + 014h] // 00c538b6
        fld st(0) // 00c538ba
        fmulp st(2), st(0) // 00c538bc
        fxch st(4) // 00c538be
        faddp st(1), st(0) // 00c538c0
        fld dword ptr [esp + 028h] // 00c538c2
        fld dword ptr [esp + 010h] // 00c538c6
        fld st(0) // 00c538ca
        fmulp st(2), st(0) // 00c538cc
        fxch st(2) // 00c538ce
        faddp st(1), st(0) // 00c538d0
        fstp dword ptr [esp + 09ch] // 00c538d2
        fld dword ptr [eax + 010h] // 00c538d9
        fstp dword ptr [esp + 010h] // 00c538dc
        fld dword ptr [eax + 4] // 00c538e0
        fstp dword ptr [esp + 0ch] // 00c538e3
        fld dword ptr [eax + 01ch] // 00c538e7
        fstp dword ptr [esp + 01ch] // 00c538ea
        fld dword ptr [esp + 0ch] // 00c538ee
        fmul st(0), st(3) // 00c538f2
        fld dword ptr [esp + 010h] // 00c538f4
        fmul st(0), st(5) // 00c538f8
        faddp st(1), st(0) // 00c538fa
        fld dword ptr [esp + 01ch] // 00c538fc
        fmul st(0), st(2) // 00c53900
        faddp st(1), st(0) // 00c53902
        fstp dword ptr [esp + 0a0h] // 00c53904
        fld dword ptr [eax + 014h] // 00c5390b
        fstp dword ptr [esp + 030h] // 00c5390e
        fld dword ptr [eax + 8] // 00c53912
        fstp dword ptr [esp + 024h] // 00c53915
        fld dword ptr [eax + 020h] // 00c53919
        fstp dword ptr [esp + 02ch] // 00c5391c
        fld dword ptr [esp + 024h] // 00c53920
        fmulp st(3), st(0) // 00c53924
        fld dword ptr [esp + 030h] // 00c53926
        fmulp st(4), st(0) // 00c5392a
        fxch st(2) // 00c5392c
        faddp st(3), st(0) // 00c5392e
        fld dword ptr [esp + 02ch] // 00c53930
        fmulp st(2), st(0) // 00c53934
        fxch st(2) // 00c53936
        faddp st(1), st(0) // 00c53938
        fstp dword ptr [esp + 0a4h] // 00c5393a
        fld dword ptr [edi + 010h] // 00c53941
        fstp dword ptr [esp + 020h] // 00c53944
        fld dword ptr [edi + 0ch] // 00c53948
        fstp dword ptr [esp + 018h] // 00c5394b
        fld dword ptr [edi + 014h] // 00c5394f
        fstp dword ptr [esp + 014h] // 00c53952
        fld dword ptr [esp + 018h] // 00c53956
        fld st(0) // 00c5395a
        fmul st(0), st(3) // 00c5395c
        fld dword ptr [esp + 020h] // 00c5395e
        fld st(0) // 00c53962
        fmul st(0), st(4) // 00c53964
        faddp st(2), st(0) // 00c53966
        fld dword ptr [esp + 014h] // 00c53968
        fld st(0) // 00c5396c
        fmul dword ptr [esp + 028h] // 00c5396e
        faddp st(3), st(0) // 00c53972
        fxch st(2) // 00c53974
        fstp dword ptr [esp + 0a8h] // 00c53976
        fld st(2) // 00c5397d
        fmul dword ptr [esp + 0ch] // 00c5397f
        fld st(1) // 00c53983
        fmul dword ptr [esp + 010h] // 00c53985
        faddp st(1), st(0) // 00c53989
        fld st(2) // 00c5398b
        fmul dword ptr [esp + 01ch] // 00c5398d
        faddp st(1), st(0) // 00c53991
        fstp dword ptr [esp + 0ach] // 00c53993
        fld dword ptr [esp + 024h] // 00c5399a
        fmulp st(3), st(0) // 00c5399e
        fmul dword ptr [esp + 030h] // 00c539a0
        faddp st(2), st(0) // 00c539a4
        fmul dword ptr [esp + 02ch] // 00c539a6
        faddp st(1), st(0) // 00c539aa
        fstp dword ptr [esp + 0b0h] // 00c539ac
        fld dword ptr [edi + 020h] // 00c539b3
        fstp dword ptr [esp + 020h] // 00c539b6
        fld dword ptr [eax + 018h] // 00c539ba
        fstp dword ptr [esp + 018h] // 00c539bd
        fld st(1) // 00c539c1
        fmul dword ptr [edi + 018h] // 00c539c3
        fld dword ptr [edi + 01ch] // 00c539c6
        fmulp st(2), st(0) // 00c539c9
        faddp st(1), st(0) // 00c539cb
        fld dword ptr [esp + 018h] // 00c539cd
        fld st(0) // 00c539d1
        fld dword ptr [esp + 020h] // 00c539d3
        fld st(0) // 00c539d7
        fmulp st(2), st(0) // 00c539d9
        fxch st(3) // 00c539db
        faddp st(1), st(0) // 00c539dd
        fstp dword ptr [esp + 0b4h] // 00c539df
        fld dword ptr [edi + 01ch] // 00c539e6
        fstp dword ptr [esp + 028h] // 00c539e9
        fld dword ptr [eax + 010h] // 00c539ed
        fstp dword ptr [esp + 0ch] // 00c539f0
        fld dword ptr [edi + 018h] // 00c539f4
        fstp dword ptr [esp + 020h] // 00c539f7
        fld dword ptr [eax + 4] // 00c539fb
        fstp dword ptr [esp + 018h] // 00c539fe
        fld dword ptr [eax + 01ch] // 00c53a02
        fstp dword ptr [esp + 010h] // 00c53a05
        fld dword ptr [esp + 018h] // 00c53a09
        fld st(0) // 00c53a0d
        fld dword ptr [esp + 020h] // 00c53a0f
        fld st(0) // 00c53a13
        fmulp st(2), st(0) // 00c53a15
        fld dword ptr [esp + 0ch] // 00c53a17
        fmul dword ptr [esp + 028h] // 00c53a1b
        faddp st(2), st(0) // 00c53a1f
        fld st(4) // 00c53a21
        fmul dword ptr [esp + 010h] // 00c53a23
        faddp st(2), st(0) // 00c53a27
        fxch st(1) // 00c53a29
        fstp dword ptr [esp + 0b8h] // 00c53a2b
        fld dword ptr [eax + 014h] // 00c53a32
        fstp dword ptr [esp + 01ch] // 00c53a35
        fld dword ptr [eax + 8] // 00c53a39
        fstp dword ptr [esp + 018h] // 00c53a3c
        fld dword ptr [eax + 020h] // 00c53a40
        fstp dword ptr [esp + 024h] // 00c53a43
        fld dword ptr [esp + 018h] // 00c53a47
        fld st(0) // 00c53a4b
        fmulp st(2), st(0) // 00c53a4d
        fld dword ptr [esp + 01ch] // 00c53a4f
        fmul dword ptr [esp + 028h] // 00c53a53
        faddp st(2), st(0) // 00c53a57
        fld dword ptr [esp + 024h] // 00c53a59
        fmulp st(5), st(0) // 00c53a5d
        fxch st(1) // 00c53a5f
        faddp st(4), st(0) // 00c53a61
        fxch st(3) // 00c53a63
        fstp dword ptr [esp + 0bch] // 00c53a65
        fld dword ptr [edi + 028h] // 00c53a6c
        fstp dword ptr [esp + 020h] // 00c53a6f
        fld dword ptr [edi + 024h] // 00c53a73
        fstp dword ptr [esp + 018h] // 00c53a76
        fld dword ptr [edi + 02ch] // 00c53a7a
        fstp dword ptr [esp + 014h] // 00c53a7d
        fld dword ptr [esp + 018h] // 00c53a81
        mov dword ptr [esp + 018h], 0 // 00c53a85
        fld st(0) // 00c53a8d
        fmulp st(5), st(0) // 00c53a8f
        fld dword ptr [eax + 0ch] // 00c53a91
        fld dword ptr [esp + 020h] // 00c53a94
        fld st(0) // 00c53a98
        fmulp st(2), st(0) // 00c53a9a
        fxch st(6) // 00c53a9c
        faddp st(1), st(0) // 00c53a9e
        fld dword ptr [esp + 014h] // 00c53aa0
        fld st(0) // 00c53aa4
        fmulp st(5), st(0) // 00c53aa6
        fxch st(1) // 00c53aa8
        faddp st(4), st(0) // 00c53aaa
        fld dword ptr [eax + 024h] // 00c53aac
        faddp st(4), st(0) // 00c53aaf
        fxch st(3) // 00c53ab1
        fstp dword ptr [esp + 0c0h] // 00c53ab3
        fld st(0) // 00c53aba
        fmulp st(2), st(0) // 00c53abc
        fld st(4) // 00c53abe
        fmul dword ptr [esp + 0ch] // 00c53ac0
        faddp st(2), st(0) // 00c53ac4
        fld st(2) // 00c53ac6
        fmul dword ptr [esp + 010h] // 00c53ac8
        faddp st(2), st(0) // 00c53acc
        fld dword ptr [eax + 028h] // 00c53ace
        faddp st(2), st(0) // 00c53ad1
        fxch st(1) // 00c53ad3
        fstp dword ptr [esp + 0c4h] // 00c53ad5
        fmulp st(2), st(0) // 00c53adc
        fld dword ptr [esp + 01ch] // 00c53ade
        fmulp st(3), st(0) // 00c53ae2
        fxch st(1) // 00c53ae4
        faddp st(2), st(0) // 00c53ae6
        fmul dword ptr [esp + 024h] // 00c53ae8
        jmp l_00c53f91 // 00c53aec
    l_00c53af1:
        mov ebx, dword ptr [ebp + 014h] // 00c53af1
        mov eax, dword ptr [ebp + 018h] // 00c53af4
        fld dword ptr [ebx + 038h] // 00c53af7
        fstp dword ptr [esp + 01ch] // 00c53afa
        fld dword ptr [eax + 0ch] // 00c53afe
        fstp dword ptr [esp + 018h] // 00c53b01
        fld dword ptr [eax] // 00c53b05
        fstp dword ptr [esp + 0ch] // 00c53b07
        fld dword ptr [ebx + 034h] // 00c53b0b
        fstp dword ptr [esp + 028h] // 00c53b0e
        fld dword ptr [ebx + 03ch] // 00c53b12
        fstp dword ptr [esp + 010h] // 00c53b15
        fld dword ptr [eax + 018h] // 00c53b19
        fstp dword ptr [esp + 020h] // 00c53b1c
        fld dword ptr [esp + 018h] // 00c53b20
        fld st(0) // 00c53b24
        fld dword ptr [esp + 01ch] // 00c53b26
        fld st(0) // 00c53b2a
        fmulp st(2), st(0) // 00c53b2c
        fld dword ptr [esp + 0ch] // 00c53b2e
        fld st(0) // 00c53b32
        fld dword ptr [esp + 028h] // 00c53b34
        fld st(0) // 00c53b38
        fmulp st(2), st(0) // 00c53b3a
        fxch st(4) // 00c53b3c
        faddp st(1), st(0) // 00c53b3e
        fld dword ptr [esp + 020h] // 00c53b40
        fld st(0) // 00c53b44
        fmul dword ptr [esp + 010h] // 00c53b46
        faddp st(2), st(0) // 00c53b4a
        fxch st(1) // 00c53b4c
        fstp dword ptr [esp + 054h] // 00c53b4e
        fld dword ptr [eax + 010h] // 00c53b52
        fstp dword ptr [esp + 020h] // 00c53b55
        fld dword ptr [eax + 4] // 00c53b59
        fstp dword ptr [esp + 018h] // 00c53b5c
        fld dword ptr [eax + 01ch] // 00c53b60
        fstp dword ptr [esp + 024h] // 00c53b63
        fld dword ptr [esp + 018h] // 00c53b67
        fld st(0) // 00c53b6b
        fmulp st(5), st(0) // 00c53b6d
        fld dword ptr [esp + 020h] // 00c53b6f
        fld st(0) // 00c53b73
        fmulp st(5), st(0) // 00c53b75
        fxch st(5) // 00c53b77
        faddp st(4), st(0) // 00c53b79
        fld dword ptr [esp + 024h] // 00c53b7b
        fmul dword ptr [esp + 010h] // 00c53b7f
        faddp st(4), st(0) // 00c53b83
        fxch st(3) // 00c53b85
        fstp dword ptr [esp + 058h] // 00c53b87
        fld dword ptr [eax + 014h] // 00c53b8b
        fstp dword ptr [esp + 02ch] // 00c53b8e
        fld dword ptr [eax + 8] // 00c53b92
        fstp dword ptr [esp + 030h] // 00c53b95
        fld dword ptr [eax + 020h] // 00c53b99
        fstp dword ptr [esp + 034h] // 00c53b9c
        fld dword ptr [esp + 030h] // 00c53ba0
        fmul dword ptr [esp + 028h] // 00c53ba4
        fld dword ptr [esp + 02ch] // 00c53ba8
        fmul dword ptr [esp + 01ch] // 00c53bac
        faddp st(1), st(0) // 00c53bb0
        fld dword ptr [esp + 034h] // 00c53bb2
        fmul dword ptr [esp + 010h] // 00c53bb6
        faddp st(1), st(0) // 00c53bba
        fstp dword ptr [esp + 05ch] // 00c53bbc
        fld dword ptr [ebx + 044h] // 00c53bc0
        fstp dword ptr [esp + 010h] // 00c53bc3
        fld dword ptr [ebx + 040h] // 00c53bc7
        fstp dword ptr [esp + 0ch] // 00c53bca
        fld dword ptr [ebx + 048h] // 00c53bce
        fstp dword ptr [esp + 01ch] // 00c53bd1
        fld dword ptr [esp + 010h] // 00c53bd5
        fmul st(0), st(5) // 00c53bd9
        fld st(2) // 00c53bdb
        fmul dword ptr [esp + 0ch] // 00c53bdd
        faddp st(1), st(0) // 00c53be1
        fld dword ptr [esp + 01ch] // 00c53be3
        fmul st(0), st(2) // 00c53be7
        faddp st(1), st(0) // 00c53be9
        fstp dword ptr [esp + 060h] // 00c53beb
        fld dword ptr [esp + 0ch] // 00c53bef
        fmul st(0), st(3) // 00c53bf3
        fld dword ptr [esp + 010h] // 00c53bf5
        fmul st(0), st(5) // 00c53bf9
        faddp st(1), st(0) // 00c53bfb
        fld dword ptr [esp + 01ch] // 00c53bfd
        fmul dword ptr [esp + 024h] // 00c53c01
        faddp st(1), st(0) // 00c53c05
        fstp dword ptr [esp + 064h] // 00c53c07
        fld dword ptr [esp + 0ch] // 00c53c0b
        fmul dword ptr [esp + 030h] // 00c53c0f
        fld dword ptr [esp + 010h] // 00c53c13
        fmul dword ptr [esp + 02ch] // 00c53c17
        faddp st(1), st(0) // 00c53c1b
        fld dword ptr [esp + 01ch] // 00c53c1d
        fmul dword ptr [esp + 034h] // 00c53c21
        faddp st(1), st(0) // 00c53c25
        fstp dword ptr [esp + 068h] // 00c53c27
        fld dword ptr [ebx + 050h] // 00c53c2b
        fstp dword ptr [esp + 010h] // 00c53c2e
        fld dword ptr [ebx + 04ch] // 00c53c32
        fstp dword ptr [esp + 0ch] // 00c53c35
        fld dword ptr [ebx + 054h] // 00c53c39
        fstp dword ptr [esp + 01ch] // 00c53c3c
        fld dword ptr [esp + 010h] // 00c53c40
        fmul st(0), st(5) // 00c53c44
        fld st(2) // 00c53c46
        fmul dword ptr [esp + 0ch] // 00c53c48
        faddp st(1), st(0) // 00c53c4c
        fld dword ptr [esp + 01ch] // 00c53c4e
        fmul st(0), st(2) // 00c53c52
        faddp st(1), st(0) // 00c53c54
        fstp dword ptr [esp + 06ch] // 00c53c56
        fld dword ptr [esp + 0ch] // 00c53c5a
        fmul st(0), st(3) // 00c53c5e
        fld dword ptr [esp + 010h] // 00c53c60
        fmul st(0), st(5) // 00c53c64
        faddp st(1), st(0) // 00c53c66
        fld dword ptr [esp + 01ch] // 00c53c68
        fmul dword ptr [esp + 024h] // 00c53c6c
        faddp st(1), st(0) // 00c53c70
        fstp dword ptr [esp + 070h] // 00c53c72
        fld dword ptr [esp + 0ch] // 00c53c76
        fmul dword ptr [esp + 030h] // 00c53c7a
        fld dword ptr [esp + 010h] // 00c53c7e
        fmul dword ptr [esp + 02ch] // 00c53c82
        faddp st(1), st(0) // 00c53c86
        fld dword ptr [esp + 01ch] // 00c53c88
        fmul dword ptr [esp + 034h] // 00c53c8c
        faddp st(1), st(0) // 00c53c90
        fstp dword ptr [esp + 074h] // 00c53c92
        fld dword ptr [ebx + 05ch] // 00c53c96
        fstp dword ptr [esp + 018h] // 00c53c99
        fld dword ptr [ebx + 058h] // 00c53c9d
        fstp dword ptr [esp + 020h] // 00c53ca0
        fld dword ptr [ebx + 060h] // 00c53ca4
        fstp dword ptr [esp + 014h] // 00c53ca7
        fld dword ptr [esp + 018h] // 00c53cab
        fld st(0) // 00c53caf
        fmulp st(6), st(0) // 00c53cb1
        fld dword ptr [esp + 020h] // 00c53cb3
        fld st(0) // 00c53cb7
        fmulp st(4), st(0) // 00c53cb9
        fxch st(6) // 00c53cbb
        faddp st(3), st(0) // 00c53cbd
        fld dword ptr [esp + 014h] // 00c53cbf
        fld st(0) // 00c53cc3
        fmulp st(3), st(0) // 00c53cc5
        fxch st(3) // 00c53cc7
        faddp st(2), st(0) // 00c53cc9
        fld dword ptr [eax + 024h] // 00c53ccb
        faddp st(2), st(0) // 00c53cce
        fxch st(1) // 00c53cd0
        fstp dword ptr [esp + 078h] // 00c53cd2
        fld st(4) // 00c53cd6
        fmulp st(3), st(0) // 00c53cd8
        fld st(0) // 00c53cda
        fmulp st(4), st(0) // 00c53cdc
        fxch st(2) // 00c53cde
        faddp st(3), st(0) // 00c53ce0
        fld st(0) // 00c53ce2
        fmul dword ptr [esp + 024h] // 00c53ce4
        mov ecx, edi // 00c53ce8
        add edi, 034h // 00c53cea
        faddp st(3), st(0) // 00c53ced
        fld dword ptr [eax + 028h] // 00c53cef
        faddp st(3), st(0) // 00c53cf2
        fxch st(2) // 00c53cf4
        fstp dword ptr [esp + 07ch] // 00c53cf6
        fld dword ptr [esp + 030h] // 00c53cfa
        fmulp st(3), st(0) // 00c53cfe
        fmul dword ptr [esp + 02ch] // 00c53d00
        faddp st(2), st(0) // 00c53d04
        fmul dword ptr [esp + 034h] // 00c53d06
        faddp st(1), st(0) // 00c53d0a
        fadd dword ptr [eax + 02ch] // 00c53d0c
        mov eax, dword ptr [ebp + 010h] // 00c53d0f
        fstp dword ptr [esp + 080h] // 00c53d12
        fld dword ptr [edi + 4] // 00c53d19
        fstp dword ptr [esp + 020h] // 00c53d1c
        fld dword ptr [eax + 0ch] // 00c53d20
        fstp dword ptr [esp + 018h] // 00c53d23
        fld dword ptr [eax] // 00c53d27
        fstp dword ptr [esp + 010h] // 00c53d29
        fld dword ptr [edi] // 00c53d2d
        fstp dword ptr [esp + 014h] // 00c53d2f
        fld dword ptr [edi + 8] // 00c53d33
        fstp dword ptr [esp + 0ch] // 00c53d36
        fld dword ptr [eax + 018h] // 00c53d3a
        fstp dword ptr [esp + 028h] // 00c53d3d
        fld dword ptr [esp + 018h] // 00c53d41
        fld st(0) // 00c53d45
        fld dword ptr [esp + 020h] // 00c53d47
        fld st(0) // 00c53d4b
        fmulp st(2), st(0) // 00c53d4d
        fld dword ptr [esp + 010h] // 00c53d4f
        fld st(0) // 00c53d53
        fld dword ptr [esp + 014h] // 00c53d55
        fld st(0) // 00c53d59
        fmulp st(2), st(0) // 00c53d5b
        fxch st(4) // 00c53d5d
        faddp st(1), st(0) // 00c53d5f
        fld dword ptr [esp + 028h] // 00c53d61
        fld dword ptr [esp + 0ch] // 00c53d65
        fld st(0) // 00c53d69
        fmulp st(2), st(0) // 00c53d6b
        fxch st(2) // 00c53d6d
        faddp st(1), st(0) // 00c53d6f
        fstp dword ptr [esp + 09ch] // 00c53d71
        fld dword ptr [eax + 010h] // 00c53d78
        fstp dword ptr [esp + 010h] // 00c53d7b
        fld dword ptr [eax + 4] // 00c53d7f
        fstp dword ptr [esp + 0ch] // 00c53d82
        fld dword ptr [eax + 01ch] // 00c53d86
        fstp dword ptr [esp + 01ch] // 00c53d89
        fld dword ptr [esp + 0ch] // 00c53d8d
        fmul st(0), st(4) // 00c53d91
        fld dword ptr [esp + 010h] // 00c53d93
        fmul st(0), st(4) // 00c53d97
        faddp st(1), st(0) // 00c53d99
        fld dword ptr [esp + 01ch] // 00c53d9b
        fmul st(0), st(2) // 00c53d9f
        faddp st(1), st(0) // 00c53da1
        fstp dword ptr [esp + 0a0h] // 00c53da3
        fld dword ptr [eax + 014h] // 00c53daa
        fstp dword ptr [esp + 030h] // 00c53dad
        fld dword ptr [eax + 8] // 00c53db1
        fstp dword ptr [esp + 024h] // 00c53db4
        fld dword ptr [eax + 020h] // 00c53db8
        fstp dword ptr [esp + 02ch] // 00c53dbb
        fld dword ptr [esp + 024h] // 00c53dbf
        fmulp st(4), st(0) // 00c53dc3
        fld dword ptr [esp + 030h] // 00c53dc5
        fmulp st(3), st(0) // 00c53dc9
        fxch st(3) // 00c53dcb
        faddp st(2), st(0) // 00c53dcd
        fld dword ptr [esp + 02ch] // 00c53dcf
        fmulp st(3), st(0) // 00c53dd3
        fxch st(1) // 00c53dd5
        faddp st(2), st(0) // 00c53dd7
        fxch st(1) // 00c53dd9
        fstp dword ptr [esp + 0a4h] // 00c53ddb
        fld dword ptr [edi + 010h] // 00c53de2
        fstp dword ptr [esp + 018h] // 00c53de5
        fld dword ptr [edi + 0ch] // 00c53de9
        fstp dword ptr [esp + 020h] // 00c53dec
        fld dword ptr [edi + 014h] // 00c53df0
        fstp dword ptr [esp + 014h] // 00c53df3
        fld dword ptr [esp + 018h] // 00c53df7
        fld st(0) // 00c53dfb
        fmul st(0), st(3) // 00c53dfd
        fld st(2) // 00c53dff
        fld dword ptr [esp + 020h] // 00c53e01
        fld st(0) // 00c53e05
        fmulp st(2), st(0) // 00c53e07
        fxch st(2) // 00c53e09
        faddp st(1), st(0) // 00c53e0b
        fld dword ptr [esp + 014h] // 00c53e0d
        fld st(0) // 00c53e11
        fmul dword ptr [esp + 028h] // 00c53e13
        faddp st(2), st(0) // 00c53e17
        fxch st(1) // 00c53e19
        fstp dword ptr [esp + 0a8h] // 00c53e1b
        fld st(1) // 00c53e22
        fmul dword ptr [esp + 0ch] // 00c53e24
        fld st(3) // 00c53e28
        fmul dword ptr [esp + 010h] // 00c53e2a
        faddp st(1), st(0) // 00c53e2e
        fld st(1) // 00c53e30
        fmul dword ptr [esp + 01ch] // 00c53e32
        faddp st(1), st(0) // 00c53e36
        fstp dword ptr [esp + 0ach] // 00c53e38
        fld dword ptr [esp + 024h] // 00c53e3f
        fmulp st(2), st(0) // 00c53e43
        fld dword ptr [esp + 030h] // 00c53e45
        fmulp st(3), st(0) // 00c53e49
        fxch st(1) // 00c53e4b
        faddp st(2), st(0) // 00c53e4d
        fmul dword ptr [esp + 02ch] // 00c53e4f
        faddp st(1), st(0) // 00c53e53
        fstp dword ptr [esp + 0b0h] // 00c53e55
        fld dword ptr [edi + 020h] // 00c53e5c
        fstp dword ptr [esp + 020h] // 00c53e5f
        fld dword ptr [eax + 018h] // 00c53e63
        fstp dword ptr [esp + 018h] // 00c53e66
        fld st(0) // 00c53e6a
        fmul dword ptr [edi + 018h] // 00c53e6c
        fld dword ptr [edi + 01ch] // 00c53e6f
        fmulp st(3), st(0) // 00c53e72
        faddp st(2), st(0) // 00c53e74
        fld dword ptr [esp + 018h] // 00c53e76
        fld st(0) // 00c53e7a
        fld dword ptr [esp + 020h] // 00c53e7c
        fld st(0) // 00c53e80
        fmulp st(2), st(0) // 00c53e82
        fxch st(4) // 00c53e84
        faddp st(1), st(0) // 00c53e86
        fstp dword ptr [esp + 0b4h] // 00c53e88
        fld dword ptr [edi + 01ch] // 00c53e8f
        fstp dword ptr [esp + 020h] // 00c53e92
        fld dword ptr [eax + 010h] // 00c53e96
        fstp dword ptr [esp + 018h] // 00c53e99
        fld dword ptr [edi + 018h] // 00c53e9d
        fstp dword ptr [esp + 028h] // 00c53ea0
        fld dword ptr [eax + 4] // 00c53ea4
        fstp dword ptr [esp + 0ch] // 00c53ea7
        fld dword ptr [eax + 01ch] // 00c53eab
        fstp dword ptr [esp + 010h] // 00c53eae
        fld dword ptr [esp + 018h] // 00c53eb2
        fld st(0) // 00c53eb6
        fld dword ptr [esp + 020h] // 00c53eb8
        fld st(0) // 00c53ebc
        fmulp st(2), st(0) // 00c53ebe
        fld dword ptr [esp + 0ch] // 00c53ec0
        fmul dword ptr [esp + 028h] // 00c53ec4
        faddp st(2), st(0) // 00c53ec8
        fld dword ptr [esp + 010h] // 00c53eca
        fmul st(0), st(6) // 00c53ece
        faddp st(2), st(0) // 00c53ed0
        fxch st(1) // 00c53ed2
        fstp dword ptr [esp + 0b8h] // 00c53ed4
        fld dword ptr [eax + 014h] // 00c53edb
        fstp dword ptr [esp + 01ch] // 00c53ede
        fld dword ptr [eax + 8] // 00c53ee2
        fstp dword ptr [esp + 018h] // 00c53ee5
        fld dword ptr [eax + 020h] // 00c53ee9
        fstp dword ptr [esp + 024h] // 00c53eec
        fld dword ptr [esp + 018h] // 00c53ef0
        fld st(0) // 00c53ef4
        fmul dword ptr [esp + 028h] // 00c53ef6
        fld dword ptr [esp + 01ch] // 00c53efa
        fmulp st(3), st(0) // 00c53efe
        faddp st(2), st(0) // 00c53f00
        fld dword ptr [esp + 024h] // 00c53f02
        fmulp st(6), st(0) // 00c53f06
        fxch st(1) // 00c53f08
        faddp st(5), st(0) // 00c53f0a
        fxch st(4) // 00c53f0c
        fstp dword ptr [esp + 0bch] // 00c53f0e
        fld dword ptr [edi + 028h] // 00c53f15
        fstp dword ptr [esp + 018h] // 00c53f18
        fld dword ptr [edi + 024h] // 00c53f1c
        fstp dword ptr [esp + 020h] // 00c53f1f
        fld dword ptr [edi + 02ch] // 00c53f23
        fstp dword ptr [esp + 014h] // 00c53f26
        fld dword ptr [esp + 018h] // 00c53f2a
        mov dword ptr [esp + 018h], 1 // 00c53f2e
        fld st(0) // 00c53f36
        fmul dword ptr [eax + 0ch] // 00c53f38
        fld dword ptr [esp + 020h] // 00c53f3b
        fld st(0) // 00c53f3f
        fmulp st(6), st(0) // 00c53f41
        fxch st(1) // 00c53f43
        faddp st(5), st(0) // 00c53f45
        fld dword ptr [esp + 014h] // 00c53f47
        fld st(0) // 00c53f4b
        fmulp st(5), st(0) // 00c53f4d
        fxch st(5) // 00c53f4f
        faddp st(4), st(0) // 00c53f51
        fld dword ptr [eax + 024h] // 00c53f53
        faddp st(4), st(0) // 00c53f56
        fxch st(3) // 00c53f58
        fstp dword ptr [esp + 0c0h] // 00c53f5a
        fld st(2) // 00c53f61
        fmul dword ptr [esp + 0ch] // 00c53f63
        fld st(1) // 00c53f67
        fmulp st(3), st(0) // 00c53f69
        faddp st(2), st(0) // 00c53f6b
        fld st(3) // 00c53f6d
        fmul dword ptr [esp + 010h] // 00c53f6f
        faddp st(2), st(0) // 00c53f73
        fld dword ptr [eax + 028h] // 00c53f75
        faddp st(2), st(0) // 00c53f78
        fxch st(1) // 00c53f7a
        fstp dword ptr [esp + 0c4h] // 00c53f7c
        fxch st(1) // 00c53f83
        fmulp st(3), st(0) // 00c53f85
        fmul dword ptr [esp + 01ch] // 00c53f87
        faddp st(2), st(0) // 00c53f8b
        fmul dword ptr [esp + 024h] // 00c53f8d
    l_00c53f91:
        faddp st(1), st(0) // 00c53f91
        fadd dword ptr [eax + 02ch] // 00c53f93
        fstp dword ptr [esp + 0c8h] // 00c53f96
        fld dword ptr [esp + 0a0h] // 00c53f9d
        fld st(0) // 00c53fa4
        fld dword ptr [esp + 058h] // 00c53fa6
        fld st(0) // 00c53faa
        fmulp st(2), st(0) // 00c53fac
        fld dword ptr [esp + 09ch] // 00c53fae
        fld st(0) // 00c53fb5
        fld dword ptr [esp + 054h] // 00c53fb7
        fld st(0) // 00c53fbb
        fmulp st(2), st(0) // 00c53fbd
        fxch st(4) // 00c53fbf
        faddp st(1), st(0) // 00c53fc1
        fld dword ptr [esp + 0a4h] // 00c53fc3
        fld dword ptr [esp + 05ch] // 00c53fca
        fld st(0) // 00c53fce
        fmulp st(2), st(0) // 00c53fd0
        fxch st(2) // 00c53fd2
        faddp st(1), st(0) // 00c53fd4
        fstp dword ptr [esp + 03ch] // 00c53fd6
        mov eax, dword ptr [esp + 03ch] // 00c53fda
        fld dword ptr [esp + 064h] // 00c53fde
        mov dword ptr [esp + 0cch], eax // 00c53fe2
        fld st(0) // 00c53fe9
        fmulp st(6), st(0) // 00c53feb
        fld dword ptr [esp + 060h] // 00c53fed
        fld st(0) // 00c53ff1
        fmulp st(4), st(0) // 00c53ff3
        fxch st(6) // 00c53ff5
        faddp st(3), st(0) // 00c53ff7
        fld dword ptr [esp + 0a4h] // 00c53ff9
        fmul dword ptr [esp + 068h] // 00c54000
        faddp st(3), st(0) // 00c54004
        fxch st(2) // 00c54006
        fstp dword ptr [esp + 040h] // 00c54008
        mov edx, dword ptr [esp + 040h] // 00c5400c
        fld dword ptr [esp + 0a0h] // 00c54010
        mov dword ptr [esp + 0d0h], edx // 00c54017
        fmul dword ptr [esp + 070h] // 00c5401e
        fld dword ptr [esp + 06ch] // 00c54022
        fmul dword ptr [esp + 09ch] // 00c54026
        faddp st(1), st(0) // 00c5402d
        fld dword ptr [esp + 0a4h] // 00c5402f
        fmul dword ptr [esp + 074h] // 00c54036
        faddp st(1), st(0) // 00c5403a
        fstp dword ptr [esp + 044h] // 00c5403c
        mov eax, dword ptr [esp + 044h] // 00c54040
        fld st(2) // 00c54044
        mov dword ptr [esp + 0d4h], eax // 00c54046
        fmul dword ptr [esp + 0ach] // 00c5404d
        fld st(4) // 00c54054
        fmul dword ptr [esp + 0a8h] // 00c54056
        faddp st(1), st(0) // 00c5405d
        fld st(1) // 00c5405f
        fmul dword ptr [esp + 0b0h] // 00c54061
        faddp st(1), st(0) // 00c54068
        fstp dword ptr [esp + 090h] // 00c5406a
        mov edx, dword ptr [esp + 090h] // 00c54071
        fld st(1) // 00c54078
        mov dword ptr [esp + 0d8h], edx // 00c5407a
        fmul dword ptr [esp + 0ach] // 00c54081
        fld st(5) // 00c54088
        fmul dword ptr [esp + 0a8h] // 00c5408a
        faddp st(1), st(0) // 00c54091
        fld dword ptr [esp + 068h] // 00c54093
        fmul dword ptr [esp + 0b0h] // 00c54097
        faddp st(1), st(0) // 00c5409e
        fstp dword ptr [esp + 094h] // 00c540a0
        mov eax, dword ptr [esp + 094h] // 00c540a7
        fld dword ptr [esp + 070h] // 00c540ae
        mov dword ptr [esp + 0dch], eax // 00c540b2
        fmul dword ptr [esp + 0ach] // 00c540b9
        fld dword ptr [esp + 06ch] // 00c540c0
        fmul dword ptr [esp + 0a8h] // 00c540c4
        faddp st(1), st(0) // 00c540cb
        fld dword ptr [esp + 074h] // 00c540cd
        fmul dword ptr [esp + 0b0h] // 00c540d1
        faddp st(1), st(0) // 00c540d8
        fstp dword ptr [esp + 098h] // 00c540da
        fld st(2) // 00c540e1
        mov edx, dword ptr [esp + 098h] // 00c540e3
        fmul dword ptr [esp + 0b8h] // 00c540ea
        mov dword ptr [esp + 0e0h], edx // 00c540f1
        fld st(4) // 00c540f8
        fmul dword ptr [esp + 0b4h] // 00c540fa
        faddp st(1), st(0) // 00c54101
        fld st(1) // 00c54103
        fmul dword ptr [esp + 0bch] // 00c54105
        faddp st(1), st(0) // 00c5410c
        fstp dword ptr [esp + 084h] // 00c5410e
        mov eax, dword ptr [esp + 084h] // 00c54115
        fld st(1) // 00c5411c
        mov dword ptr [esp + 0e4h], eax // 00c5411e
        fmul dword ptr [esp + 0b8h] // 00c54125
        fld st(5) // 00c5412c
        fmul dword ptr [esp + 0b4h] // 00c5412e
        faddp st(1), st(0) // 00c54135
        fld dword ptr [esp + 068h] // 00c54137
        fmul dword ptr [esp + 0bch] // 00c5413b
        faddp st(1), st(0) // 00c54142
        fstp dword ptr [esp + 088h] // 00c54144
        mov edx, dword ptr [esp + 088h] // 00c5414b
        fld dword ptr [esp + 070h] // 00c54152
        mov dword ptr [esp + 0e8h], edx // 00c54156
        fmul dword ptr [esp + 0b8h] // 00c5415d
        fld dword ptr [esp + 06ch] // 00c54164
        fmul dword ptr [esp + 0b4h] // 00c54168
        faddp st(1), st(0) // 00c5416f
        fld dword ptr [esp + 074h] // 00c54171
        fmul dword ptr [esp + 0bch] // 00c54175
        faddp st(1), st(0) // 00c5417c
        fstp dword ptr [esp + 08ch] // 00c5417e
        mov eax, dword ptr [esp + 08ch] // 00c54185
        fld dword ptr [esp + 0c0h] // 00c5418c
        mov dword ptr [esp + 0ech], eax // 00c54193
        fsub dword ptr [esp + 078h] // 00c5419a
        fstp dword ptr [esp + 048h] // 00c5419e
        fld dword ptr [esp + 0c4h] // 00c541a2
        fsub dword ptr [esp + 07ch] // 00c541a9
        fstp dword ptr [esp + 04ch] // 00c541ad
        fld dword ptr [esp + 0c8h] // 00c541b1
        fsub dword ptr [esp + 080h] // 00c541b8
        fstp dword ptr [esp + 050h] // 00c541bf
        fld dword ptr [esp + 048h] // 00c541c3
        fld st(0) // 00c541c7
        fmulp st(5), st(0) // 00c541c9
        fld dword ptr [esp + 04ch] // 00c541cb
        fld st(0) // 00c541cf
        fmulp st(5), st(0) // 00c541d1
        fxch st(5) // 00c541d3
        faddp st(4), st(0) // 00c541d5
        fld dword ptr [esp + 050h] // 00c541d7
        fld st(0) // 00c541db
        fmulp st(3), st(0) // 00c541dd
        fxch st(4) // 00c541df
        faddp st(2), st(0) // 00c541e1
        fxch st(1) // 00c541e3
        fstp dword ptr [esp + 048h] // 00c541e5
        mov edx, dword ptr [esp + 048h] // 00c541e9
        fld st(3) // 00c541ed
        mov dword ptr [esp + 0f0h], edx // 00c541ef
        fmulp st(2), st(0) // 00c541f6
        fld st(0) // 00c541f8
        fmulp st(5), st(0) // 00c541fa
        fxch st(1) // 00c541fc
        faddp st(4), st(0) // 00c541fe
        fld dword ptr [esp + 068h] // 00c54200
        fmul st(0), st(2) // 00c54204
        faddp st(4), st(0) // 00c54206
        fxch st(3) // 00c54208
        fstp dword ptr [esp + 04ch] // 00c5420a
        fld dword ptr [esp + 070h] // 00c5420e
        fmulp st(2), st(0) // 00c54212
        fld dword ptr [esp + 06ch] // 00c54214
        fmulp st(3), st(0) // 00c54218
        fxch st(1) // 00c5421a
        faddp st(2), st(0) // 00c5421c
        fmul dword ptr [esp + 074h] // 00c5421e
        faddp st(1), st(0) // 00c54222
        fstp dword ptr [esp + 050h] // 00c54224
        fld dword ptr [ebx + 021ch] // 00c54228
        mov edx, dword ptr [esp + 050h] // 00c5422e
        fstp dword ptr [esp + 020h] // 00c54232
        mov dword ptr [esp + 0f8h], edx // 00c54236
        fld dword ptr [esp + 03ch] // 00c5423d
        mov eax, dword ptr [esp + 04ch] // 00c54241
        fld dword ptr [esp + 020h] // 00c54245
        mov ecx, dword ptr [ecx + 0210h] // 00c54249
        fld st(0) // 00c5424f
        mov esi, dword ptr [ebp + 8] // 00c54251
        fdivp st(2), st(0) // 00c54254
        xor edx, edx // 00c54256
        mov dword ptr [esp + 0f4h], eax // 00c54258
        mov eax, dword ptr [ecx + 4] // 00c5425f
        cmp eax, edx // 00c54262
        mov dword ptr [esp + 0108h], ecx // 00c54264
        mov dword ptr [esp + 010ch], eax // 00c5426b
        fxch st(1) // 00c54272
        fstp dword ptr [esp + 0cch] // 00c54274
        fld dword ptr [esp + 090h] // 00c5427b
        fdiv st(0), st(1) // 00c54282
        fstp dword ptr [esp + 0d8h] // 00c54284
        fld dword ptr [esp + 084h] // 00c5428b
        fdiv st(0), st(1) // 00c54292
        fstp dword ptr [esp + 0e4h] // 00c54294
        fdivr dword ptr [esp + 048h] // 00c5429b
        fstp dword ptr [esp + 0f0h] // 00c5429f
        fld dword ptr [ebx + 0220h] // 00c542a6
        fstp dword ptr [esp + 020h] // 00c542ac
        fld dword ptr [esp + 0d4h] // 00c542b0
        mov dword ptr [esi], edx // 00c542b7
        fld dword ptr [esp + 020h] // 00c542b9
        fld st(0) // 00c542bd
        fdivp st(2), st(0) // 00c542bf
        mov dword ptr [esp + 020h], edx // 00c542c1
        fxch st(1) // 00c542c5
        fstp dword ptr [esp + 0d4h] // 00c542c7
        fld dword ptr [esp + 0e0h] // 00c542ce
        fdiv st(0), st(1) // 00c542d5
        fstp dword ptr [esp + 0e0h] // 00c542d7
        fld dword ptr [esp + 0ech] // 00c542de
        fdiv st(0), st(1) // 00c542e5
        fstp dword ptr [esp + 0ech] // 00c542e7
        fdivr dword ptr [esp + 0f8h] // 00c542ee
        fstp dword ptr [esp + 0f8h] // 00c542f5
        jle l_00c54952 // 00c542fc
        xorps xmm0, xmm0 // 00c54302
        movss xmm3, dword ptr constant_00d7a240 // 00c54305
        mov dword ptr [esp + 028h], edx // 00c5430d
        jmp l_00c5431a // 00c54311
    l_00c54313:
        mov ecx, dword ptr [esp + 0108h] // 00c54313
    l_00c5431a:
        mov eax, dword ptr [ecx] // 00c5431a
        mov ecx, dword ptr [esp + 028h] // 00c5431c
        fld dword ptr [ecx + eax + 4] // 00c54320
        add ecx, eax // 00c54324
        fstp dword ptr [esp + 0ch] // 00c54326
        fld dword ptr [ecx] // 00c5432a
        fstp dword ptr [esp + 014h] // 00c5432c
        fld dword ptr [ecx + 8] // 00c54330
        fstp dword ptr [esp + 024h] // 00c54333
        fld dword ptr [esp + 014h] // 00c54337
        fld st(0) // 00c5433b
        fmul dword ptr [esp + 0cch] // 00c5433d
        fld dword ptr [esp + 0ch] // 00c54344
        fld st(0) // 00c54348
        fmul dword ptr [esp + 0d8h] // 00c5434a
        faddp st(2), st(0) // 00c54351
        fld dword ptr [esp + 024h] // 00c54353
        fld st(0) // 00c54357
        fmul dword ptr [esp + 0e4h] // 00c54359
        faddp st(3), st(0) // 00c54360
        fld dword ptr [esp + 0f0h] // 00c54362
        faddp st(3), st(0) // 00c54369
        fxch st(2) // 00c5436b
        fstp dword ptr [esp + 03ch] // 00c5436d
        movss xmm2, dword ptr [esp + 03ch] // 00c54371
        comiss xmm0, xmm2 // 00c54377
        fld st(2) // 00c5437a
        fmul dword ptr [esp + 0d0h] // 00c5437c
        fld st(1) // 00c54383
        fmul dword ptr [esp + 0dch] // 00c54385
        faddp st(1), st(0) // 00c5438c
        fld st(2) // 00c5438e
        fmul dword ptr [esp + 0e8h] // 00c54390
        faddp st(1), st(0) // 00c54397
        fadd dword ptr [esp + 0f4h] // 00c54399
        fstp dword ptr [esp + 040h] // 00c543a0
        fld dword ptr [esp + 0d4h] // 00c543a4
        fmulp st(3), st(0) // 00c543ab
        fmul dword ptr [esp + 0e0h] // 00c543ad
        faddp st(2), st(0) // 00c543b4
        fmul dword ptr [esp + 0ech] // 00c543b6
        faddp st(1), st(0) // 00c543bd
        fadd dword ptr [esp + 0f8h] // 00c543bf
        fstp dword ptr [esp + 044h] // 00c543c6
        ja l_00c54935 // 00c543ca
        mov edx, dword ptr [ebx + 0214h] // 00c543d0
        fld dword ptr [esp + 03ch] // 00c543d6
        lea eax, [edx - 1] // 00c543da
        mov dword ptr [esp + 014h], eax // 00c543dd
        fild dword ptr [esp + 014h] // 00c543e1
        fxch st(1) // 00c543e5
        fcomi st(0), st(1) // 00c543e7
        fstp st(1) // 00c543e9
        ja l_00c54933 // 00c543eb
        movss xmm1, dword ptr [esp + 044h] // 00c543f1
        comiss xmm0, xmm1 // 00c543f7
        ja l_00c54933 // 00c543fa
        mov eax, dword ptr [ebx + 0218h] // 00c54400
        fld dword ptr [esp + 044h] // 00c54406
        sub eax, 1 // 00c5440a
        mov dword ptr [esp + 014h], eax // 00c5440d
        fild dword ptr [esp + 014h] // 00c54411
        fxch st(1) // 00c54415
        fcomi st(0), st(1) // 00c54417
        fstp st(1) // 00c54419
        ja l_00c54931 // 00c5441b
        cvttss2si eax, xmm2 // 00c54421
        mov dword ptr [esp + 034h], eax // 00c54425
        mov eax, dword ptr [ebx + 022ch] // 00c54429
        sub eax, 0 // 00c5442f
        cvttss2si esi, xmm1 // 00c54432
        mov dword ptr [esp + 0ch], esi // 00c54436
        je l_00c54489 // 00c5443a
        sub eax, 1 // 00c5443c
        je l_00c54449 // 00c5443f
        movss dword ptr [esp + 02ch], xmm0 // 00c54441
        jmp l_00c544a3 // 00c54447
    l_00c54449:
        mov eax, edx // 00c54449
        imul eax, esi // 00c5444b
        add eax, dword ptr [esp + 034h] // 00c5444e
        mov esi, dword ptr [ebx + 0210h] // 00c54452
        movzx eax, word ptr [esi + eax*2] // 00c54458
        _emit 066h // 00c5445c
        _emit 03dh
        _emit 0ffh
        _emit 0ffh
        jne l_00c5446a // 00c54460
        movss dword ptr [esp + 02ch], xmm3 // 00c54462
        jmp l_00c544a3 // 00c54468
    l_00c5446a:
        fld dword ptr [ebx + 0224h] // 00c5446a
        movzx eax, ax // 00c54470
        mov dword ptr [esp + 014h], eax // 00c54473
        fild dword ptr [esp + 014h] // 00c54477
        fmulp st(1), st(0) // 00c5447b
        fadd dword ptr [ebx + 0228h] // 00c5447d
        fstp dword ptr [esp + 02ch] // 00c54483
        jmp l_00c544a3 // 00c54487
    l_00c54489:
        mov eax, edx // 00c54489
        imul eax, esi // 00c5448b
        add eax, dword ptr [esp + 034h] // 00c5448e
        mov esi, dword ptr [ebx + 0210h] // 00c54492
        movss xmm1, dword ptr [esi + eax*4] // 00c54498
        movss dword ptr [esp + 02ch], xmm1 // 00c5449d
    l_00c544a3:
        mov eax, dword ptr [esp + 034h] // 00c544a3
        add eax, 1 // 00c544a7
        mov dword ptr [esp + 024h], eax // 00c544aa
        mov eax, dword ptr [ebx + 022ch] // 00c544ae
        sub eax, 0 // 00c544b4
        je l_00c54508 // 00c544b7
        sub eax, 1 // 00c544b9
        je l_00c544c6 // 00c544bc
        movss dword ptr [esp + 030h], xmm0 // 00c544be
        jmp l_00c54524 // 00c544c4
    l_00c544c6:
        mov esi, dword ptr [ebx + 0210h] // 00c544c6
        mov eax, edx // 00c544cc
        imul eax, dword ptr [esp + 0ch] // 00c544ce
        add eax, dword ptr [esp + 024h] // 00c544d3
        movzx eax, word ptr [esi + eax*2] // 00c544d7
        _emit 066h // 00c544db
        _emit 03dh
        _emit 0ffh
        _emit 0ffh
        jne l_00c544e9 // 00c544df
        movss dword ptr [esp + 030h], xmm3 // 00c544e1
        jmp l_00c54524 // 00c544e7
    l_00c544e9:
        fld dword ptr [ebx + 0224h] // 00c544e9
        movzx eax, ax // 00c544ef
        mov dword ptr [esp + 014h], eax // 00c544f2
        fild dword ptr [esp + 014h] // 00c544f6
        fmulp st(1), st(0) // 00c544fa
        fadd dword ptr [ebx + 0228h] // 00c544fc
        fstp dword ptr [esp + 030h] // 00c54502
        jmp l_00c54524 // 00c54506
    l_00c54508:
        mov esi, dword ptr [ebx + 0210h] // 00c54508
        mov eax, edx // 00c5450e
        imul eax, dword ptr [esp + 0ch] // 00c54510
        add eax, dword ptr [esp + 024h] // 00c54515
        movss xmm1, dword ptr [esi + eax*4] // 00c54519
        movss dword ptr [esp + 030h], xmm1 // 00c5451e
    l_00c54524:
        mov eax, dword ptr [esp + 0ch] // 00c54524
        mov esi, dword ptr [ebx + 022ch] // 00c54528
        add eax, 1 // 00c5452e
        sub esi, 0 // 00c54531
        je l_00c54584 // 00c54534
        sub esi, 1 // 00c54536
        je l_00c54543 // 00c54539
        movss dword ptr [esp + 010h], xmm0 // 00c5453b
        jmp l_00c5459e // 00c54541
    l_00c54543:
        mov esi, eax // 00c54543
        imul esi, edx // 00c54545
        add esi, dword ptr [esp + 034h] // 00c54548
        mov edx, dword ptr [ebx + 0210h] // 00c5454c
        movzx edx, word ptr [edx + esi*2] // 00c54552
        _emit 066h // 00c54556
        _emit 081h
        _emit 0fah
        _emit 0ffh
        _emit 0ffh
        jne l_00c54565 // 00c5455b
        movss dword ptr [esp + 010h], xmm3 // 00c5455d
        jmp l_00c5459e // 00c54563
    l_00c54565:
        fld dword ptr [ebx + 0224h] // 00c54565
        movzx edx, dx // 00c5456b
        mov dword ptr [esp + 014h], edx // 00c5456e
        fild dword ptr [esp + 014h] // 00c54572
        fmulp st(1), st(0) // 00c54576
        fadd dword ptr [ebx + 0228h] // 00c54578
        fstp dword ptr [esp + 010h] // 00c5457e
        jmp l_00c5459e // 00c54582
    l_00c54584:
        mov esi, eax // 00c54584
        imul esi, edx // 00c54586
        add esi, dword ptr [esp + 034h] // 00c54589
        mov edx, dword ptr [ebx + 0210h] // 00c5458d
        movss xmm1, dword ptr [edx + esi*4] // 00c54593
        movss dword ptr [esp + 010h], xmm1 // 00c54598
    l_00c5459e:
        mov edx, dword ptr [ebx + 022ch] // 00c5459e
        sub edx, 0 // 00c545a4
        je l_00c545f8 // 00c545a7
        sub edx, 1 // 00c545a9
        je l_00c545b6 // 00c545ac
        movss dword ptr [esp + 01ch], xmm0 // 00c545ae
        jmp l_00c54614 // 00c545b4
    l_00c545b6:
        imul eax, dword ptr [ebx + 0214h] // 00c545b6
        add eax, dword ptr [esp + 024h] // 00c545bd
        mov edx, dword ptr [ebx + 0210h] // 00c545c1
        movzx eax, word ptr [edx + eax*2] // 00c545c7
        _emit 066h // 00c545cb
        _emit 03dh
        _emit 0ffh
        _emit 0ffh
        jne l_00c545d9 // 00c545cf
        movss dword ptr [esp + 01ch], xmm3 // 00c545d1
        jmp l_00c54614 // 00c545d7
    l_00c545d9:
        fld dword ptr [ebx + 0224h] // 00c545d9
        movzx eax, ax // 00c545df
        mov dword ptr [esp + 014h], eax // 00c545e2
        fild dword ptr [esp + 014h] // 00c545e6
        fmulp st(1), st(0) // 00c545ea
        fadd dword ptr [ebx + 0228h] // 00c545ec
        fstp dword ptr [esp + 01ch] // 00c545f2
        jmp l_00c54614 // 00c545f6
    l_00c545f8:
        imul eax, dword ptr [ebx + 0214h] // 00c545f8
        add eax, dword ptr [esp + 024h] // 00c545ff
        mov edx, dword ptr [ebx + 0210h] // 00c54603
        movss xmm1, dword ptr [edx + eax*4] // 00c54609
        movss dword ptr [esp + 01ch], xmm1 // 00c5460e
    l_00c54614:
        fild dword ptr [esp + 034h] // 00c54614
        fsubp st(2), st(0) // 00c54618
        fxch st(1) // 00c5461a
        fstp dword ptr [esp + 014h] // 00c5461c
        fld dword ptr [esp + 030h] // 00c54620
        fld st(0) // 00c54624
        fld dword ptr [esp + 02ch] // 00c54626
        fld st(0) // 00c5462a
        fsubp st(2), st(0) // 00c5462c
        fld dword ptr [esp + 014h] // 00c5462e
        fld st(0) // 00c54632
        fmulp st(3), st(0) // 00c54634
        fld st(1) // 00c54636
        faddp st(3), st(0) // 00c54638
        fxch st(2) // 00c5463a
        fstp dword ptr [esp + 024h] // 00c5463c
        fld dword ptr [esp + 01ch] // 00c54640
        fld st(0) // 00c54644
        fld dword ptr [esp + 010h] // 00c54646
        fld st(0) // 00c5464a
        fsubp st(2), st(0) // 00c5464c
        fxch st(1) // 00c5464e
        fmulp st(4), st(0) // 00c54650
        fld st(0) // 00c54652
        faddp st(4), st(0) // 00c54654
        fxch st(3) // 00c54656
        fstp dword ptr [esp + 014h] // 00c54658
        fld dword ptr [esp + 014h] // 00c5465c
        fld dword ptr [esp + 024h] // 00c54660
        fld st(0) // 00c54664
        fsubp st(2), st(0) // 00c54666
        fild dword ptr [esp + 0ch] // 00c54668
        fsubp st(7), st(0) // 00c5466c
        fxch st(6) // 00c5466e
        fstp dword ptr [esp + 014h] // 00c54670
        fmul dword ptr [esp + 014h] // 00c54674
        faddp st(5), st(0) // 00c54678
        fxch st(4) // 00c5467a
        fstp dword ptr [esp + 014h] // 00c5467c
        fld dword ptr [esp + 014h] // 00c54680
        fld dword ptr [esp + 040h] // 00c54684
        fcomip st(0), st(1) // 00c54688
        fstp st(0) // 00c5468a
        ja l_00c54929 // 00c5468c
        fld dword ptr [edi + 0ch] // 00c54692
        mov esi, dword ptr [ebp + 8] // 00c54695
        fmul dword ptr [ecx + 4] // 00c54698
        mov eax, dword ptr [esi] // 00c5469b
        fld dword ptr [ecx] // 00c5469d
        lea eax, [eax + eax*8] // 00c5469f
        fmul dword ptr [edi] // 00c546a2
        lea esi, [esi + eax*4 + 4] // 00c546a4
        movss xmm2, dword ptr constant_00d7a208 // 00c546a8
        faddp st(1), st(0) // 00c546b0
        fld dword ptr [edi + 018h] // 00c546b2
        fmul dword ptr [ecx + 8] // 00c546b5
        faddp st(1), st(0) // 00c546b8
        fadd dword ptr [edi + 024h] // 00c546ba
        fstp dword ptr [esi + 0ch] // 00c546bd
        fld dword ptr [edi + 4] // 00c546c0
        fmul dword ptr [ecx] // 00c546c3
        fld dword ptr [edi + 010h] // 00c546c5
        fmul dword ptr [ecx + 4] // 00c546c8
        faddp st(1), st(0) // 00c546cb
        fld dword ptr [edi + 01ch] // 00c546cd
        fmul dword ptr [ecx + 8] // 00c546d0
        faddp st(1), st(0) // 00c546d3
        fadd dword ptr [edi + 028h] // 00c546d5
        fstp dword ptr [esi + 010h] // 00c546d8
        fld dword ptr [edi + 8] // 00c546db
        fmul dword ptr [ecx] // 00c546de
        fld dword ptr [edi + 014h] // 00c546e0
        fmul dword ptr [ecx + 4] // 00c546e3
        faddp st(1), st(0) // 00c546e6
        fld dword ptr [edi + 020h] // 00c546e8
        fmul dword ptr [ecx + 8] // 00c546eb
        faddp st(1), st(0) // 00c546ee
        fadd dword ptr [edi + 02ch] // 00c546f0
        fstp dword ptr [esi + 014h] // 00c546f3
        movss xmm1, dword ptr [ebx + 0220h] // 00c546f6
        fxch st(2) // 00c546fe
        movss xmm0, dword ptr [ebx + 021ch] // 00c54700
        fsubrp st(1), st(0) // 00c54708
        subss xmm2, xmm1 // 00c5470a
        movss dword ptr [esp + 050h], xmm2 // 00c5470e
        movss dword ptr [esp + 08ch], xmm1 // 00c54714
        fstp dword ptr [esp + 04ch] // 00c5471d
        movss dword ptr [esp + 048h], xmm0 // 00c54721
        movss dword ptr [esp + 084h], xmm0 // 00c54727
        fsubp st(1), st(0) // 00c54730
        fstp dword ptr [esp + 088h] // 00c54732
        fld dword ptr [esp + 088h] // 00c54739
        fld st(0) // 00c54740
        fld dword ptr [esp + 050h] // 00c54742
        fld st(0) // 00c54746
        fmulp st(2), st(0) // 00c54748
        fld dword ptr [esp + 08ch] // 00c5474a
        fld st(0) // 00c54751
        fld dword ptr [esp + 04ch] // 00c54753
        fld st(0) // 00c54757
        fmulp st(2), st(0) // 00c54759
        fxch st(4) // 00c5475b
        fsubrp st(1), st(0) // 00c5475d
        fstp dword ptr [esi + 018h] // 00c5475f
        fld dword ptr [esp + 048h] // 00c54762
        fld st(0) // 00c54766
        fmulp st(2), st(0) // 00c54768
        fld dword ptr [esp + 084h] // 00c5476a
        fld st(0) // 00c54771
        fmulp st(4), st(0) // 00c54773
        fxch st(2) // 00c54775
        fsubrp st(3), st(0) // 00c54777
        fxch st(2) // 00c54779
        fstp dword ptr [esi + 01ch] // 00c5477b
        fmulp st(2), st(0) // 00c5477e
        fmulp st(2), st(0) // 00c54780
        fsubrp st(1), st(0) // 00c54782
        fstp dword ptr [esi + 020h] // 00c54784
        fld dword ptr [esi + 01ch] // 00c54787
        fstp dword ptr [esp + 024h] // 00c5478a
        fld dword ptr [esp + 024h] // 00c5478e
        fld dword ptr [esi + 018h] // 00c54792
        fstp dword ptr [esp + 0ch] // 00c54795
        fld dword ptr [esp + 0ch] // 00c54799
        fld dword ptr [esi + 020h] // 00c5479d
        fstp dword ptr [esp + 034h] // 00c547a0
        push ecx // 00c547a4
        fld dword ptr [esp + 038h] // 00c547a5
        fld st(1) // 00c547a9
        fmulp st(2), st(0) // 00c547ab
        fld st(2) // 00c547ad
        fmulp st(3), st(0) // 00c547af
        fxch st(1) // 00c547b1
        faddp st(2), st(0) // 00c547b3
        fmul st(0), st(0) // 00c547b5
        faddp st(1), st(0) // 00c547b7
        fstp dword ptr [esp + 03ch] // 00c547b9
        fld dword ptr [esp + 03ch] // 00c547bd
        fstp dword ptr [esp] // 00c547c1
        push dword ptr [ebp+28] // Borrowed CRT context.
        call sqrt_kernel // 00c547c4
        fstp dword ptr [esp + 038h] // 00c547c9
        mov ecx, dword ptr [esp + 03ch] // 00c547cd
        fld dword ptr [esp + 0ch] // 00c547d1
        mov edx, dword ptr [esp + 040h] // 00c547d5
        fld dword ptr [esp + 038h] // 00c547d9
        mov eax, dword ptr [esp + 044h] // 00c547dd
        fld st(0) // 00c547e1
        fdivp st(2), st(0) // 00c547e3
        fxch st(1) // 00c547e5
        fstp dword ptr [esi + 018h] // 00c547e7
        fld dword ptr [esp + 024h] // 00c547ea
        fdiv st(0), st(1) // 00c547ee
        fstp dword ptr [esi + 01ch] // 00c547f0
        fdivr dword ptr [esp + 034h] // 00c547f3
        fstp dword ptr [esi + 020h] // 00c547f7
        mov dword ptr [esi], ecx // 00c547fa
        mov dword ptr [esi + 4], edx // 00c547fc
        mov dword ptr [esi + 8], eax // 00c547ff
        fld dword ptr [esi] // 00c54802
        fmul dword ptr [ebx + 021ch] // 00c54804
        fstp dword ptr [esi] // 00c5480a
        fld dword ptr [esi + 8] // 00c5480c
        fmul dword ptr [ebx + 021ch] // 00c5480f
        fstp dword ptr [esi + 8] // 00c54815
        fld dword ptr [esp + 014h] // 00c54818
        fsub dword ptr [esp + 040h] // 00c5481c
        fstp dword ptr [esp + 038h] // 00c54820
        fld dword ptr [esp + 038h] // 00c54824
        fld st(0) // 00c54828
        fmul dword ptr [esi + 018h] // 00c5482a
        fstp dword ptr [esp + 090h] // 00c5482d
        fld dword ptr [esi + 01ch] // 00c54834
        fmul st(0), st(1) // 00c54837
        fstp dword ptr [esp + 094h] // 00c54839
        fmul dword ptr [esi + 020h] // 00c54840
        fstp dword ptr [esp + 098h] // 00c54843
        fld dword ptr [esp + 090h] // 00c5484a
        fadd dword ptr [esi] // 00c54851
        fstp dword ptr [esi] // 00c54853
        fld dword ptr [esp + 094h] // 00c54855
        fadd dword ptr [esi + 4] // 00c5485c
        fstp dword ptr [esi + 4] // 00c5485f
        fld dword ptr [esi + 8] // 00c54862
        fadd dword ptr [esp + 098h] // 00c54865
        fstp dword ptr [esi + 8] // 00c5486c
        fld dword ptr [esi + 01ch] // 00c5486f
        fstp dword ptr [esp + 038h] // 00c54872
        fld dword ptr [esi + 018h] // 00c54876
        fstp dword ptr [esp + 014h] // 00c54879
        fld dword ptr [esi + 020h] // 00c5487d
        fstp dword ptr [esp + 0ch] // 00c54880
        fld dword ptr [esp + 060h] // 00c54884
        fld dword ptr [esp + 038h] // 00c54888
        fld st(0) // 00c5488c
        fmulp st(2), st(0) // 00c5488e
        fld dword ptr [esp + 054h] // 00c54890
        fld dword ptr [esp + 014h] // 00c54894
        fld st(0) // 00c54898
        fmulp st(2), st(0) // 00c5489a
        fxch st(3) // 00c5489c
        faddp st(1), st(0) // 00c5489e
        fld dword ptr [esp + 06ch] // 00c548a0
        fld dword ptr [esp + 0ch] // 00c548a4
        fld st(0) // 00c548a8
        fmulp st(2), st(0) // 00c548aa
        fxch st(2) // 00c548ac
        faddp st(1), st(0) // 00c548ae
        fstp dword ptr [esp + 0fch] // 00c548b0
        mov ecx, dword ptr [esp + 0fch] // 00c548b7
        fld dword ptr [esp + 058h] // 00c548be
        mov dword ptr [esi + 018h], ecx // 00c548c2
        fmul st(0), st(3) // 00c548c5
        fld dword ptr [esp + 064h] // 00c548c7
        fmul st(0), st(3) // 00c548cb
        faddp st(1), st(0) // 00c548cd
        fld dword ptr [esp + 070h] // 00c548cf
        fmul st(0), st(2) // 00c548d3
        faddp st(1), st(0) // 00c548d5
        fstp dword ptr [esp + 0100h] // 00c548d7
        mov edx, dword ptr [esp + 0100h] // 00c548de
        fld dword ptr [esp + 05ch] // 00c548e5
        mov dword ptr [esi + 01ch], edx // 00c548e9
        fmulp st(3), st(0) // 00c548ec
        fld dword ptr [esp + 068h] // 00c548ee
        fmulp st(2), st(0) // 00c548f2
        fxch st(2) // 00c548f4
        faddp st(1), st(0) // 00c548f6
        fld dword ptr [esp + 074h] // 00c548f8
        fmulp st(2), st(0) // 00c548fc
        faddp st(1), st(0) // 00c548fe
        fstp dword ptr [esp + 0104h] // 00c54900
        mov eax, dword ptr [esp + 0104h] // 00c54907
        mov dword ptr [esi + 020h], eax // 00c5490e
        mov eax, dword ptr [ebp + 8] // 00c54911
        add dword ptr [eax], 1 // 00c54914
        cmp dword ptr [eax], 8 // 00c54917
        je l_00c54952 // 00c5491a
        movss xmm3, dword ptr constant_00d7a240 // 00c5491c
        xorps xmm0, xmm0 // 00c54924
        jmp l_00c54935 // 00c54927
    l_00c54929:
        fstp st(0) // 00c54929
        fstp st(0) // 00c5492b
        fstp st(0) // 00c5492d
        jmp l_00c54933 // 00c5492f
    l_00c54931:
        fstp st(1) // 00c54931
    l_00c54933:
        fstp st(0) // 00c54933
    l_00c54935:
        mov eax, dword ptr [esp + 020h] // 00c54935
        add dword ptr [esp + 028h], 010h // 00c54939
        add eax, 1 // 00c5493e
        cmp eax, dword ptr [esp + 010ch] // 00c54941
        mov dword ptr [esp + 020h], eax // 00c54948
        jl l_00c54313 // 00c5494c
    l_00c54952:
        cmp dword ptr [esp + 018h], 0 // 00c54952
        je l_00c549b6 // 00c54957
        mov eax, dword ptr [ebp + 8] // 00c54959
        xor ecx, ecx // 00c5495c
        cmp dword ptr [eax], ecx // 00c5495e
        jle l_00c549b6 // 00c54960
        fld qword ptr constant_00d7a250 // 00c54962
        add eax, 024h // 00c54968
    l_00c5496b:
        fld dword ptr [eax - 8] // 00c5496b
        add ecx, 1 // 00c5496e
        fmul st(0), st(1) // 00c54971
        add eax, 024h // 00c54973
        fstp dword ptr [eax - 02ch] // 00c54976
        fld dword ptr [eax - 028h] // 00c54979
        fmul st(0), st(1) // 00c5497c
        fstp dword ptr [eax - 028h] // 00c5497e
        fld dword ptr [eax - 024h] // 00c54981
        fmul st(0), st(1) // 00c54984
        fstp dword ptr [eax - 024h] // 00c54986
        mov ebx, dword ptr [eax - 038h] // 00c54989
        mov edx, dword ptr [eax - 044h] // 00c5498c
        mov esi, dword ptr [eax - 040h] // 00c5498f
        mov edi, dword ptr [eax - 03ch] // 00c54992
        mov dword ptr [eax - 044h], ebx // 00c54995
        mov ebx, dword ptr [eax - 034h] // 00c54998
        mov dword ptr [eax - 040h], ebx // 00c5499b
        mov ebx, dword ptr [eax - 030h] // 00c5499e
        mov dword ptr [eax - 03ch], ebx // 00c549a1
        mov dword ptr [eax - 038h], edx // 00c549a4
        mov edx, dword ptr [ebp + 8] // 00c549a7
        mov dword ptr [eax - 034h], esi // 00c549aa
        mov dword ptr [eax - 030h], edi // 00c549ad
        cmp ecx, dword ptr [edx] // 00c549b0
        jl l_00c5496b // 00c549b2
        fstp st(0) // 00c549b4
    l_00c549b6:
        mov ecx, dword ptr [ebp + 8] // 00c549b6
        xor eax, eax // 00c549b9
        cmp dword ptr [ecx], eax // 00c549bb
        pop edi // 00c549bd
        pop esi // 00c549be
        setne al // 00c549bf
        pop ebx // 00c549c2
        mov esp, ebp // 00c549c3
        pop ebp // 00c549c5
        ret 24 // 00c549c6
    }
}
} // namespace
bool intersect_native_dyn_terrain_convex_00c53630(void* result,const void* a,const float* ma,
    const void* b,const float* mb,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;unsigned char answer;
    __asm {
        push c
        push mb
        push b
        push ma
        push a
        push result
        call terrain_convex_kernel
        mov answer,al
    }
    return answer!=0;
}
static_assert(std::is_standard_layout_v<NativeDynTerrainConvexRuntime>);
NativeDynTerrainConvexRuntime::NativeDynTerrainConvexRuntime(const CameraAxesCrtAccess& crt) noexcept
    :methods_{reinterpret_cast<std::uintptr_t>(&intersect)},crt_(crt) {}
bool __fastcall NativeDynTerrainConvexRuntime::intersect(void* p,void*,void* result,
    const void* a,const float* ma,const void* b,const float* mb){
    const auto& owner=*static_cast<const DynStaticDispatchObjectStorage*>(p);
    const auto& runtime=*static_cast<const NativeDynTerrainConvexRuntime*>(owner.vtable);
    return intersect_native_dyn_terrain_convex_00c53630(result,a,ma,b,mb,runtime.crt_);
}
} // namespace bsp
