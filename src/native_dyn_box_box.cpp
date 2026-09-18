#include "bsp/native_dyn_box_box.hpp"
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native box-box collision requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
alignas(16) const unsigned char constant_00d7a208[]={0,0,0,128};
alignas(16) const unsigned char constant_00d7a218[]={0,0,0,0};
alignas(16) const unsigned char constant_00d7a244[]={255,255,127,255};
alignas(16) const unsigned char constant_00d7a24c[]={0,0,128,63};
alignas(16) const unsigned char constant_00d7a260[]={0,0,128,191};
alignas(16) const unsigned char constant_00d7a268[]={0,0,0,224,226,54,26,63};
alignas(16) const unsigned char constant_00d7a280[]={0,0,0,0,0,0,224,63};
alignas(16) const unsigned char constant_00d7a300[]={10,215,35,188};
alignas(16) const unsigned char constant_00d7a308[]={0,0,0,0,0,0,0,64};
alignas(16) const unsigned char constant_00d7a310[]={172,197,39,55};
alignas(16) const unsigned char constant_00d7a318[]={0,0,0,224,77,98,80,63};
// Consumed float-sqrt boundary: preserve native F32 store/reload and alignment.
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
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void abs_kernel(){
    __asm {
        push ebp // 00401170
        mov ebp, esp // 00401171
        and esp, 0fffffff8h // 00401173
        sub esp, 8 // 00401176
        fld dword ptr [ebp + 8] // 00401179
        fabs  // 0040117c
        fstp dword ptr [esp + 4] // 0040117e
        fld dword ptr [esp + 4] // 00401182
        mov esp, ebp // 00401186
        pop ebp // 00401188
        ret 4 // 00401189
    }
}
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void clip_33710_kernel(){
    __asm {
        sub esp, 014h // 00c33710
        mov eax, dword ptr [esp + 01ch] // 00c33713
        fld dword ptr [esp + 020h] // 00c33717
        mov edx, dword ptr [esp + 018h] // 00c3371b
        push ebp // 00c3371f
        mov ebp, dword ptr [esp + 028h] // 00c33720
        push edi // 00c33724
        lea eax, [eax + eax*2] // 00c33725
        lea eax, [edx + eax*4 - 0ch] // 00c33728
        mov edi, 3 // 00c3372c
        sub edi, ebx // 00c33731
        mov dword ptr [ecx], 0 // 00c33733
        fld dword ptr [eax + ebp*4] // 00c33739
        sub edi, ebp // 00c3373c
        fcomip st(0), st(1) // 00c3373e
        mov dword ptr [esp + 0ch], eax // 00c33740
        mov dword ptr [esp + 010h], 0 // 00c33744
        ja l_00c33756 // 00c3374c
        mov dword ptr [esp + 010h], 1 // 00c3374e
    l_00c33756:
        mov eax, dword ptr [esp + 024h] // 00c33756
        cmp eax, 4 // 00c3375a
        movss xmm0, dword ptr [esp + 028h] // 00c3375d
        mov dword ptr [esp + 018h], 0 // 00c33763
        jl l_00c33b7d // 00c3376b
        mov eax, edx // 00c33771
        add eax, 018h // 00c33773
        mov dword ptr [esp + 020h], eax // 00c33776
        mov eax, dword ptr [esp + 024h] // 00c3377a
        add eax, -4 // 00c3377e
        shr eax, 2 // 00c33781
        add eax, 1 // 00c33784
        mov dword ptr [esp + 014h], eax // 00c33787
        add eax, eax // 00c3378b
        add eax, eax // 00c3378d
        mov dword ptr [esp + 018h], eax // 00c3378f
        jmp l_00c33799 // 00c33793
    l_00c33795:
        mov ebp, dword ptr [esp + 02ch] // 00c33795
    l_00c33799:
        fld dword ptr [edx + ebp*4] // 00c33799
        fstp dword ptr [esp + 8] // 00c3379c
        fld dword ptr [esp + 8] // 00c337a0
        fcomi st(0), st(1) // 00c337a4
        jbe l_00c337ac // 00c337a6
        xor eax, eax // 00c337a8
        jmp l_00c337b1 // 00c337aa
    l_00c337ac:
        mov eax, 1 // 00c337ac
    l_00c337b1:
        mov dword ptr [esp + 028h], eax // 00c337b1
        xor eax, dword ptr [esp + 010h] // 00c337b5
        je l_00c33854 // 00c337b9
        mov ebp, dword ptr [esp + 02ch] // 00c337bf
        mov eax, dword ptr [esp + 0ch] // 00c337c3
        fld dword ptr [eax + ebp*4] // 00c337c7
        mov eax, dword ptr [ecx] // 00c337ca
        fstp dword ptr [esp + 010h] // 00c337cc
        lea eax, [ebp + eax*2] // 00c337d0
        fld st(1) // 00c337d4
        add eax, dword ptr [ecx] // 00c337d6
        fld dword ptr [esp + 010h] // 00c337d8
        fld st(0) // 00c337dc
        movss dword ptr [esi + eax*4], xmm0 // 00c337de
        fsubp st(2), st(0) // 00c337e3
        mov eax, dword ptr [esp + 0ch] // 00c337e5
        fsubp st(2), st(0) // 00c337e9
        fdivrp st(1), st(0) // 00c337eb
        fstp dword ptr [esp + 8] // 00c337ed
        fld dword ptr [eax + ebx*4] // 00c337f1
        fstp dword ptr [esp + 010h] // 00c337f4
        fld dword ptr [edx + ebx*4] // 00c337f8
        mov eax, dword ptr [ecx] // 00c337fb
        fld dword ptr [esp + 010h] // 00c337fd
        lea ebp, [ebx + eax*2] // 00c33801
        fld st(0) // 00c33804
        add ebp, eax // 00c33806
        fsubp st(2), st(0) // 00c33808
        mov eax, dword ptr [esp + 0ch] // 00c3380a
        fld dword ptr [esp + 8] // 00c3380e
        fld st(0) // 00c33812
        fmulp st(3), st(0) // 00c33814
        fxch st(2) // 00c33816
        faddp st(1), st(0) // 00c33818
        fstp dword ptr [esp + 010h] // 00c3381a
        fld dword ptr [esp + 010h] // 00c3381e
        fstp dword ptr [esi + ebp*4] // 00c33822
        fld dword ptr [eax + edi*4] // 00c33825
        mov eax, dword ptr [ecx] // 00c33828
        fstp dword ptr [esp + 010h] // 00c3382a
        lea ebp, [edi + eax*2] // 00c3382e
        fld dword ptr [edx + edi*4] // 00c33831
        add ebp, eax // 00c33834
        fld dword ptr [esp + 010h] // 00c33836
        fld st(0) // 00c3383a
        fsubp st(2), st(0) // 00c3383c
        fxch st(1) // 00c3383e
        fmulp st(2), st(0) // 00c33840
        faddp st(1), st(0) // 00c33842
        fstp dword ptr [esp + 010h] // 00c33844
        fld dword ptr [esp + 010h] // 00c33848
        fstp dword ptr [esi + ebp*4] // 00c3384c
        add dword ptr [ecx], 1 // 00c3384f
        jmp l_00c33856 // 00c33852
    l_00c33854:
        fstp st(0) // 00c33854
    l_00c33856:
        cmp dword ptr [esp + 028h], 1 // 00c33856
        jne l_00c33878 // 00c3385b
        mov eax, dword ptr [ecx] // 00c3385d
        mov ebp, dword ptr [edx] // 00c3385f
        lea eax, [eax + eax*2] // 00c33861
        lea eax, [esi + eax*4] // 00c33864
        mov dword ptr [eax], ebp // 00c33867
        mov ebp, dword ptr [edx + 4] // 00c33869
        mov dword ptr [eax + 4], ebp // 00c3386c
        mov ebp, dword ptr [edx + 8] // 00c3386f
        mov dword ptr [eax + 8], ebp // 00c33872
        add dword ptr [ecx], 1 // 00c33875
    l_00c33878:
        mov eax, dword ptr [esp + 02ch] // 00c33878
        fld dword ptr [edx + eax*4 + 0ch] // 00c3387c
        mov ebp, dword ptr [esp + 028h] // 00c33880
        fstp dword ptr [esp + 028h] // 00c33884
        fld dword ptr [esp + 028h] // 00c33888
        fcomi st(0), st(1) // 00c3388c
        jbe l_00c33894 // 00c3388e
        xor eax, eax // 00c33890
        jmp l_00c33899 // 00c33892
    l_00c33894:
        mov eax, 1 // 00c33894
    l_00c33899:
        mov dword ptr [esp + 028h], eax // 00c33899
        xor eax, ebp // 00c3389d
        je l_00c33930 // 00c3389f
        mov ebp, dword ptr [esp + 02ch] // 00c338a5
        fld dword ptr [edx + ebp*4] // 00c338a9
        mov eax, dword ptr [ecx] // 00c338ac
        fstp dword ptr [esp + 8] // 00c338ae
        lea eax, [ebp + eax*2] // 00c338b2
        fld st(1) // 00c338b6
        add eax, dword ptr [ecx] // 00c338b8
        fld dword ptr [esp + 8] // 00c338ba
        fld st(0) // 00c338be
        movss dword ptr [esi + eax*4], xmm0 // 00c338c0
        fsubp st(2), st(0) // 00c338c5
        mov eax, dword ptr [ecx] // 00c338c7
        lea ebp, [ebx + eax*2] // 00c338c9
        add ebp, eax // 00c338cc
        fsubp st(2), st(0) // 00c338ce
        fdivrp st(1), st(0) // 00c338d0
        fstp dword ptr [esp + 8] // 00c338d2
        fld dword ptr [edx + ebx*4] // 00c338d6
        fstp dword ptr [esp + 010h] // 00c338d9
        fld dword ptr [edx + ebx*4 + 0ch] // 00c338dd
        fld dword ptr [esp + 010h] // 00c338e1
        fld st(0) // 00c338e5
        fsubp st(2), st(0) // 00c338e7
        fld dword ptr [esp + 8] // 00c338e9
        fld st(0) // 00c338ed
        fmulp st(3), st(0) // 00c338ef
        fxch st(2) // 00c338f1
        faddp st(1), st(0) // 00c338f3
        fstp dword ptr [esp + 010h] // 00c338f5
        fld dword ptr [esp + 010h] // 00c338f9
        fstp dword ptr [esi + ebp*4] // 00c338fd
        mov eax, dword ptr [ecx] // 00c33900
        fld dword ptr [edx + edi*4] // 00c33902
        lea ebp, [edi + eax*2] // 00c33905
        fstp dword ptr [esp + 010h] // 00c33908
        add ebp, eax // 00c3390c
        fld dword ptr [edx + edi*4 + 0ch] // 00c3390e
        fld dword ptr [esp + 010h] // 00c33912
        fld st(0) // 00c33916
        fsubp st(2), st(0) // 00c33918
        fxch st(1) // 00c3391a
        fmulp st(2), st(0) // 00c3391c
        faddp st(1), st(0) // 00c3391e
        fstp dword ptr [esp + 010h] // 00c33920
        fld dword ptr [esp + 010h] // 00c33924
        fstp dword ptr [esi + ebp*4] // 00c33928
        add dword ptr [ecx], 1 // 00c3392b
        jmp l_00c33932 // 00c3392e
    l_00c33930:
        fstp st(0) // 00c33930
    l_00c33932:
        cmp dword ptr [esp + 028h], 1 // 00c33932
        jne l_00c33962 // 00c33937
        mov eax, dword ptr [ecx] // 00c33939
        mov ebp, dword ptr [esp + 020h] // 00c3393b
        mov ebp, dword ptr [ebp - 0ch] // 00c3393f
        lea eax, [eax + eax*2] // 00c33942
        mov dword ptr [esi + eax*4], ebp // 00c33945
        mov ebp, dword ptr [esp + 020h] // 00c33948
        mov ebp, dword ptr [ebp - 8] // 00c3394c
        lea eax, [esi + eax*4] // 00c3394f
        mov dword ptr [eax + 4], ebp // 00c33952
        mov ebp, dword ptr [esp + 020h] // 00c33955
        mov ebp, dword ptr [ebp - 4] // 00c33959
        mov dword ptr [eax + 8], ebp // 00c3395c
        add dword ptr [ecx], 1 // 00c3395f
    l_00c33962:
        mov eax, dword ptr [esp + 02ch] // 00c33962
        fld dword ptr [edx + eax*4 + 018h] // 00c33966
        mov ebp, dword ptr [esp + 028h] // 00c3396a
        fstp dword ptr [esp + 028h] // 00c3396e
        fld dword ptr [esp + 028h] // 00c33972
        fcomi st(0), st(1) // 00c33976
        jbe l_00c3397e // 00c33978
        xor eax, eax // 00c3397a
        jmp l_00c33983 // 00c3397c
    l_00c3397e:
        mov eax, 1 // 00c3397e
    l_00c33983:
        mov dword ptr [esp + 028h], eax // 00c33983
        xor eax, ebp // 00c33987
        je l_00c33a29 // 00c33989
        mov ebp, dword ptr [esp + 02ch] // 00c3398f
        mov eax, dword ptr [esp + 020h] // 00c33993
        fld dword ptr [eax + ebp*4 - 0ch] // 00c33997
        mov eax, dword ptr [ecx] // 00c3399b
        fstp dword ptr [esp + 010h] // 00c3399d
        lea eax, [ebp + eax*2] // 00c339a1
        fld st(1) // 00c339a5
        add eax, dword ptr [ecx] // 00c339a7
        fld dword ptr [esp + 010h] // 00c339a9
        fld st(0) // 00c339ad
        movss dword ptr [esi + eax*4], xmm0 // 00c339af
        fsubp st(2), st(0) // 00c339b4
        mov eax, dword ptr [esp + 020h] // 00c339b6
        fsubp st(2), st(0) // 00c339ba
        fdivrp st(1), st(0) // 00c339bc
        fstp dword ptr [esp + 8] // 00c339be
        fld dword ptr [eax + ebx*4 - 0ch] // 00c339c2
        fstp dword ptr [esp + 010h] // 00c339c6
        fld dword ptr [edx + ebx*4 + 018h] // 00c339ca
        mov eax, dword ptr [ecx] // 00c339ce
        fld dword ptr [esp + 010h] // 00c339d0
        lea ebp, [ebx + eax*2] // 00c339d4
        fld st(0) // 00c339d7
        add ebp, eax // 00c339d9
        fsubp st(2), st(0) // 00c339db
        mov eax, dword ptr [esp + 020h] // 00c339dd
        fld dword ptr [esp + 8] // 00c339e1
        fld st(0) // 00c339e5
        fmulp st(3), st(0) // 00c339e7
        fxch st(2) // 00c339e9
        faddp st(1), st(0) // 00c339eb
        fstp dword ptr [esp + 010h] // 00c339ed
        fld dword ptr [esp + 010h] // 00c339f1
        fstp dword ptr [esi + ebp*4] // 00c339f5
        fld dword ptr [eax + edi*4 - 0ch] // 00c339f8
        mov eax, dword ptr [ecx] // 00c339fc
        fstp dword ptr [esp + 010h] // 00c339fe
        lea ebp, [edi + eax*2] // 00c33a02
        fld dword ptr [edx + edi*4 + 018h] // 00c33a05
        add ebp, eax // 00c33a09
        fld dword ptr [esp + 010h] // 00c33a0b
        fld st(0) // 00c33a0f
        fsubp st(2), st(0) // 00c33a11
        fxch st(1) // 00c33a13
        fmulp st(2), st(0) // 00c33a15
        faddp st(1), st(0) // 00c33a17
        fstp dword ptr [esp + 010h] // 00c33a19
        fld dword ptr [esp + 010h] // 00c33a1d
        fstp dword ptr [esi + ebp*4] // 00c33a21
        add dword ptr [ecx], 1 // 00c33a24
        jmp l_00c33a2b // 00c33a27
    l_00c33a29:
        fstp st(0) // 00c33a29
    l_00c33a2b:
        cmp dword ptr [esp + 028h], 1 // 00c33a2b
        jne l_00c33a5b // 00c33a30
        mov eax, dword ptr [ecx] // 00c33a32
        mov ebp, dword ptr [esp + 020h] // 00c33a34
        mov ebp, dword ptr [ebp] // 00c33a38
        lea eax, [eax + eax*2] // 00c33a3b
        mov dword ptr [esi + eax*4], ebp // 00c33a3e
        mov ebp, dword ptr [esp + 020h] // 00c33a41
        mov ebp, dword ptr [ebp + 4] // 00c33a45
        lea eax, [esi + eax*4] // 00c33a48
        mov dword ptr [eax + 4], ebp // 00c33a4b
        mov ebp, dword ptr [esp + 020h] // 00c33a4e
        mov ebp, dword ptr [ebp + 8] // 00c33a52
        mov dword ptr [eax + 8], ebp // 00c33a55
        add dword ptr [ecx], 1 // 00c33a58
    l_00c33a5b:
        mov eax, dword ptr [esp + 02ch] // 00c33a5b
        fld dword ptr [edx + eax*4 + 024h] // 00c33a5f
        mov ebp, dword ptr [esp + 028h] // 00c33a63
        fstp dword ptr [esp + 028h] // 00c33a67
        fld dword ptr [esp + 028h] // 00c33a6b
        fcomi st(0), st(1) // 00c33a6f
        jbe l_00c33a77 // 00c33a71
        xor eax, eax // 00c33a73
        jmp l_00c33a7c // 00c33a75
    l_00c33a77:
        mov eax, 1 // 00c33a77
    l_00c33a7c:
        mov dword ptr [esp + 028h], eax // 00c33a7c
        xor eax, ebp // 00c33a80
        je l_00c33b1f // 00c33a82
        mov ebp, dword ptr [esp + 02ch] // 00c33a88
        mov eax, dword ptr [esp + 020h] // 00c33a8c
        fld dword ptr [eax + ebp*4] // 00c33a90
        mov eax, dword ptr [ecx] // 00c33a93
        fstp dword ptr [esp + 010h] // 00c33a95
        lea eax, [ebp + eax*2] // 00c33a99
        fld st(1) // 00c33a9d
        add eax, dword ptr [ecx] // 00c33a9f
        fld dword ptr [esp + 010h] // 00c33aa1
        fld st(0) // 00c33aa5
        movss dword ptr [esi + eax*4], xmm0 // 00c33aa7
        fsubp st(2), st(0) // 00c33aac
        mov eax, dword ptr [esp + 020h] // 00c33aae
        fsubp st(2), st(0) // 00c33ab2
        fdivrp st(1), st(0) // 00c33ab4
        fstp dword ptr [esp + 8] // 00c33ab6
        fld dword ptr [eax + ebx*4] // 00c33aba
        fstp dword ptr [esp + 010h] // 00c33abd
        fld dword ptr [edx + ebx*4 + 024h] // 00c33ac1
        mov eax, dword ptr [ecx] // 00c33ac5
        fld dword ptr [esp + 010h] // 00c33ac7
        lea ebp, [ebx + eax*2] // 00c33acb
        fld st(0) // 00c33ace
        add ebp, eax // 00c33ad0
        fsubp st(2), st(0) // 00c33ad2
        mov eax, dword ptr [esp + 020h] // 00c33ad4
        fld dword ptr [esp + 8] // 00c33ad8
        fld st(0) // 00c33adc
        fmulp st(3), st(0) // 00c33ade
        fxch st(2) // 00c33ae0
        faddp st(1), st(0) // 00c33ae2
        fstp dword ptr [esp + 010h] // 00c33ae4
        fld dword ptr [esp + 010h] // 00c33ae8
        fstp dword ptr [esi + ebp*4] // 00c33aec
        fld dword ptr [eax + edi*4] // 00c33aef
        mov eax, dword ptr [ecx] // 00c33af2
        fstp dword ptr [esp + 010h] // 00c33af4
        lea ebp, [edi + eax*2] // 00c33af8
        fld dword ptr [edx + edi*4 + 024h] // 00c33afb
        add ebp, eax // 00c33aff
        fld dword ptr [esp + 010h] // 00c33b01
        fld st(0) // 00c33b05
        fsubp st(2), st(0) // 00c33b07
        fxch st(1) // 00c33b09
        fmulp st(2), st(0) // 00c33b0b
        faddp st(1), st(0) // 00c33b0d
        fstp dword ptr [esp + 010h] // 00c33b0f
        fld dword ptr [esp + 010h] // 00c33b13
        fstp dword ptr [esi + ebp*4] // 00c33b17
        add dword ptr [ecx], 1 // 00c33b1a
        jmp l_00c33b21 // 00c33b1d
    l_00c33b1f:
        fstp st(0) // 00c33b1f
    l_00c33b21:
        cmp dword ptr [esp + 028h], 1 // 00c33b21
        jne l_00c33b51 // 00c33b26
        mov eax, dword ptr [ecx] // 00c33b28
        mov ebp, dword ptr [esp + 020h] // 00c33b2a
        mov ebp, dword ptr [ebp + 0ch] // 00c33b2e
        lea eax, [eax + eax*2] // 00c33b31
        mov dword ptr [esi + eax*4], ebp // 00c33b34
        mov ebp, dword ptr [esp + 020h] // 00c33b37
        mov ebp, dword ptr [ebp + 010h] // 00c33b3b
        lea eax, [esi + eax*4] // 00c33b3e
        mov dword ptr [eax + 4], ebp // 00c33b41
        mov ebp, dword ptr [esp + 020h] // 00c33b44
        mov ebp, dword ptr [ebp + 014h] // 00c33b48
        mov dword ptr [eax + 8], ebp // 00c33b4b
        add dword ptr [ecx], 1 // 00c33b4e
    l_00c33b51:
        mov eax, dword ptr [esp + 020h] // 00c33b51
        lea ebp, [eax + 0ch] // 00c33b55
        mov dword ptr [esp + 0ch], ebp // 00c33b58
        mov ebp, dword ptr [esp + 028h] // 00c33b5c
        add eax, 030h // 00c33b60
        add edx, 030h // 00c33b63
        sub dword ptr [esp + 014h], 1 // 00c33b66
        mov dword ptr [esp + 010h], ebp // 00c33b6b
        mov dword ptr [esp + 020h], eax // 00c33b6f
        jne l_00c33795 // 00c33b73
        mov eax, dword ptr [esp + 024h] // 00c33b79
    l_00c33b7d:
        mov ebp, dword ptr [esp + 018h] // 00c33b7d
        cmp ebp, eax // 00c33b81
        jge l_00c33c8a // 00c33b83
        sub eax, ebp // 00c33b89
        mov ebp, dword ptr [esp + 010h] // 00c33b8b
        mov dword ptr [esp + 024h], eax // 00c33b8f
    l_00c33b93:
        mov eax, dword ptr [esp + 02ch] // 00c33b93
        fld dword ptr [edx + eax*4] // 00c33b97
        fstp dword ptr [esp + 8] // 00c33b9a
        fld dword ptr [esp + 8] // 00c33b9e
        fcomi st(0), st(1) // 00c33ba2
        jbe l_00c33baa // 00c33ba4
        xor eax, eax // 00c33ba6
        jmp l_00c33baf // 00c33ba8
    l_00c33baa:
        mov eax, 1 // 00c33baa
    l_00c33baf:
        mov dword ptr [esp + 028h], eax // 00c33baf
        xor eax, ebp // 00c33bb3
        je l_00c33c50 // 00c33bb5
        mov ebp, dword ptr [esp + 02ch] // 00c33bbb
        mov eax, dword ptr [esp + 0ch] // 00c33bbf
        fld dword ptr [eax + ebp*4] // 00c33bc3
        mov eax, dword ptr [ecx] // 00c33bc6
        fstp dword ptr [esp + 020h] // 00c33bc8
        lea eax, [ebp + eax*2] // 00c33bcc
        fld st(1) // 00c33bd0
        add eax, dword ptr [ecx] // 00c33bd2
        fld dword ptr [esp + 020h] // 00c33bd4
        fld st(0) // 00c33bd8
        movss dword ptr [esi + eax*4], xmm0 // 00c33bda
        fsubp st(2), st(0) // 00c33bdf
        mov eax, dword ptr [esp + 0ch] // 00c33be1
        fsubp st(2), st(0) // 00c33be5
        fdivrp st(1), st(0) // 00c33be7
        fstp dword ptr [esp + 8] // 00c33be9
        fld dword ptr [eax + ebx*4] // 00c33bed
        fstp dword ptr [esp + 020h] // 00c33bf0
        fld dword ptr [edx + ebx*4] // 00c33bf4
        mov eax, dword ptr [ecx] // 00c33bf7
        fld dword ptr [esp + 020h] // 00c33bf9
        lea ebp, [ebx + eax*2] // 00c33bfd
        fld st(0) // 00c33c00
        add ebp, eax // 00c33c02
        fsubp st(2), st(0) // 00c33c04
        mov eax, dword ptr [esp + 0ch] // 00c33c06
        fld dword ptr [esp + 8] // 00c33c0a
        fld st(0) // 00c33c0e
        fmulp st(3), st(0) // 00c33c10
        fxch st(2) // 00c33c12
        faddp st(1), st(0) // 00c33c14
        fstp dword ptr [esp + 020h] // 00c33c16
        fld dword ptr [esp + 020h] // 00c33c1a
        fstp dword ptr [esi + ebp*4] // 00c33c1e
        fld dword ptr [eax + edi*4] // 00c33c21
        mov eax, dword ptr [ecx] // 00c33c24
        fstp dword ptr [esp + 020h] // 00c33c26
        lea ebp, [edi + eax*2] // 00c33c2a
        fld dword ptr [edx + edi*4] // 00c33c2d
        add ebp, eax // 00c33c30
        fld dword ptr [esp + 020h] // 00c33c32
        fld st(0) // 00c33c36
        fsubp st(2), st(0) // 00c33c38
        fxch st(1) // 00c33c3a
        fmulp st(2), st(0) // 00c33c3c
        faddp st(1), st(0) // 00c33c3e
        fstp dword ptr [esp + 020h] // 00c33c40
        fld dword ptr [esp + 020h] // 00c33c44
        fstp dword ptr [esi + ebp*4] // 00c33c48
        add dword ptr [ecx], 1 // 00c33c4b
        jmp l_00c33c52 // 00c33c4e
    l_00c33c50:
        fstp st(0) // 00c33c50
    l_00c33c52:
        cmp dword ptr [esp + 028h], 1 // 00c33c52
        jne l_00c33c74 // 00c33c57
        mov eax, dword ptr [ecx] // 00c33c59
        mov ebp, dword ptr [edx] // 00c33c5b
        lea eax, [eax + eax*2] // 00c33c5d
        lea eax, [esi + eax*4] // 00c33c60
        mov dword ptr [eax], ebp // 00c33c63
        mov ebp, dword ptr [edx + 4] // 00c33c65
        mov dword ptr [eax + 4], ebp // 00c33c68
        mov ebp, dword ptr [edx + 8] // 00c33c6b
        mov dword ptr [eax + 8], ebp // 00c33c6e
        add dword ptr [ecx], 1 // 00c33c71
    l_00c33c74:
        mov ebp, dword ptr [esp + 028h] // 00c33c74
        mov dword ptr [esp + 0ch], edx // 00c33c78
        add edx, 0ch // 00c33c7c
        sub dword ptr [esp + 024h], 1 // 00c33c7f
        jne l_00c33b93 // 00c33c84
    l_00c33c8a:
        pop edi // 00c33c8a
        fstp st(0) // 00c33c8b
        pop ebp // 00c33c8d
        add esp, 014h // 00c33c8e
        ret 010h // 00c33c91
    }
}
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void clip_33ca0_kernel(){
    __asm {
        sub esp, 014h // 00c33ca0
        mov eax, dword ptr [esp + 01ch] // 00c33ca3
        mov edx, dword ptr [esp + 018h] // 00c33ca7
        push ebp // 00c33cab
        mov ebp, dword ptr [esp + 028h] // 00c33cac
        push edi // 00c33cb0
        lea eax, [eax + eax*2] // 00c33cb1
        lea eax, [edx + eax*4 - 0ch] // 00c33cb4
        mov edi, 3 // 00c33cb8
        sub edi, ebx // 00c33cbd
        mov dword ptr [ecx], 0 // 00c33cbf
        fld dword ptr [eax + ebp*4] // 00c33cc5
        fld dword ptr [esp + 028h] // 00c33cc8
        sub edi, ebp // 00c33ccc
        fcomi st(0), st(1) // 00c33cce
        fstp st(1) // 00c33cd0
        mov dword ptr [esp + 0ch], eax // 00c33cd2
        mov dword ptr [esp + 010h], 0 // 00c33cd6
        ja l_00c33ce8 // 00c33cde
        mov dword ptr [esp + 010h], 1 // 00c33ce0
    l_00c33ce8:
        mov eax, dword ptr [esp + 024h] // 00c33ce8
        cmp eax, 4 // 00c33cec
        movss xmm0, dword ptr [esp + 028h] // 00c33cef
        mov dword ptr [esp + 018h], 0 // 00c33cf5
        jl l_00c3411f // 00c33cfd
        mov eax, edx // 00c33d03
        add eax, 018h // 00c33d05
        mov dword ptr [esp + 020h], eax // 00c33d08
        mov eax, dword ptr [esp + 024h] // 00c33d0c
        add eax, -4 // 00c33d10
        shr eax, 2 // 00c33d13
        add eax, 1 // 00c33d16
        mov dword ptr [esp + 014h], eax // 00c33d19
        add eax, eax // 00c33d1d
        add eax, eax // 00c33d1f
        mov dword ptr [esp + 018h], eax // 00c33d21
        jmp l_00c33d2b // 00c33d25
    l_00c33d27:
        mov ebp, dword ptr [esp + 02ch] // 00c33d27
    l_00c33d2b:
        fld dword ptr [edx + ebp*4] // 00c33d2b
        fstp dword ptr [esp + 8] // 00c33d2e
        fld dword ptr [esp + 8] // 00c33d32
        fxch st(1) // 00c33d36
        fcomi st(0), st(1) // 00c33d38
        jbe l_00c33d40 // 00c33d3a
        xor eax, eax // 00c33d3c
        jmp l_00c33d45 // 00c33d3e
    l_00c33d40:
        mov eax, 1 // 00c33d40
    l_00c33d45:
        mov dword ptr [esp + 028h], eax // 00c33d45
        xor eax, dword ptr [esp + 010h] // 00c33d49
        je l_00c33dea // 00c33d4d
        mov ebp, dword ptr [esp + 02ch] // 00c33d53
        mov eax, dword ptr [esp + 0ch] // 00c33d57
        fld dword ptr [eax + ebp*4] // 00c33d5b
        mov eax, dword ptr [ecx] // 00c33d5e
        fstp dword ptr [esp + 010h] // 00c33d60
        lea eax, [ebp + eax*2] // 00c33d64
        add eax, dword ptr [ecx] // 00c33d68
        fld st(0) // 00c33d6a
        fld dword ptr [esp + 010h] // 00c33d6c
        movss dword ptr [esi + eax*4], xmm0 // 00c33d70
        fld st(0) // 00c33d75
        mov eax, dword ptr [esp + 0ch] // 00c33d77
        fsubp st(2), st(0) // 00c33d7b
        fsubp st(3), st(0) // 00c33d7d
        fdivrp st(2), st(0) // 00c33d7f
        fxch st(1) // 00c33d81
        fstp dword ptr [esp + 8] // 00c33d83
        fld dword ptr [eax + ebx*4] // 00c33d87
        fstp dword ptr [esp + 010h] // 00c33d8a
        fld dword ptr [edx + ebx*4] // 00c33d8e
        mov eax, dword ptr [ecx] // 00c33d91
        fld dword ptr [esp + 010h] // 00c33d93
        lea ebp, [ebx + eax*2] // 00c33d97
        fld st(0) // 00c33d9a
        add ebp, eax // 00c33d9c
        fsubp st(2), st(0) // 00c33d9e
        mov eax, dword ptr [esp + 0ch] // 00c33da0
        fld dword ptr [esp + 8] // 00c33da4
        fld st(0) // 00c33da8
        fmulp st(3), st(0) // 00c33daa
        fxch st(2) // 00c33dac
        faddp st(1), st(0) // 00c33dae
        fstp dword ptr [esp + 010h] // 00c33db0
        fld dword ptr [esp + 010h] // 00c33db4
        fstp dword ptr [esi + ebp*4] // 00c33db8
        fld dword ptr [eax + edi*4] // 00c33dbb
        mov eax, dword ptr [ecx] // 00c33dbe
        fstp dword ptr [esp + 010h] // 00c33dc0
        lea ebp, [edi + eax*2] // 00c33dc4
        fld dword ptr [edx + edi*4] // 00c33dc7
        add ebp, eax // 00c33dca
        fld dword ptr [esp + 010h] // 00c33dcc
        fld st(0) // 00c33dd0
        fsubp st(2), st(0) // 00c33dd2
        fxch st(1) // 00c33dd4
        fmulp st(2), st(0) // 00c33dd6
        faddp st(1), st(0) // 00c33dd8
        fstp dword ptr [esp + 010h] // 00c33dda
        fld dword ptr [esp + 010h] // 00c33dde
        fstp dword ptr [esi + ebp*4] // 00c33de2
        add dword ptr [ecx], 1 // 00c33de5
        jmp l_00c33dec // 00c33de8
    l_00c33dea:
        fstp st(1) // 00c33dea
    l_00c33dec:
        cmp dword ptr [esp + 028h], 1 // 00c33dec
        jne l_00c33e0e // 00c33df1
        mov eax, dword ptr [ecx] // 00c33df3
        mov ebp, dword ptr [edx] // 00c33df5
        lea eax, [eax + eax*2] // 00c33df7
        lea eax, [esi + eax*4] // 00c33dfa
        mov dword ptr [eax], ebp // 00c33dfd
        mov ebp, dword ptr [edx + 4] // 00c33dff
        mov dword ptr [eax + 4], ebp // 00c33e02
        mov ebp, dword ptr [edx + 8] // 00c33e05
        mov dword ptr [eax + 8], ebp // 00c33e08
        add dword ptr [ecx], 1 // 00c33e0b
    l_00c33e0e:
        mov eax, dword ptr [esp + 02ch] // 00c33e0e
        fld dword ptr [edx + eax*4 + 0ch] // 00c33e12
        mov ebp, dword ptr [esp + 028h] // 00c33e16
        fstp dword ptr [esp + 028h] // 00c33e1a
        fld dword ptr [esp + 028h] // 00c33e1e
        fxch st(1) // 00c33e22
        fcomi st(0), st(1) // 00c33e24
        jbe l_00c33e2c // 00c33e26
        xor eax, eax // 00c33e28
        jmp l_00c33e31 // 00c33e2a
    l_00c33e2c:
        mov eax, 1 // 00c33e2c
    l_00c33e31:
        mov dword ptr [esp + 028h], eax // 00c33e31
        xor eax, ebp // 00c33e35
        je l_00c33eca // 00c33e37
        mov ebp, dword ptr [esp + 02ch] // 00c33e3d
        fld dword ptr [edx + ebp*4] // 00c33e41
        mov eax, dword ptr [ecx] // 00c33e44
        fstp dword ptr [esp + 8] // 00c33e46
        lea eax, [ebp + eax*2] // 00c33e4a
        add eax, dword ptr [ecx] // 00c33e4e
        fld st(0) // 00c33e50
        fld dword ptr [esp + 8] // 00c33e52
        movss dword ptr [esi + eax*4], xmm0 // 00c33e56
        fld st(0) // 00c33e5b
        mov eax, dword ptr [ecx] // 00c33e5d
        fsubp st(2), st(0) // 00c33e5f
        lea ebp, [ebx + eax*2] // 00c33e61
        add ebp, eax // 00c33e64
        fsubp st(3), st(0) // 00c33e66
        fdivrp st(2), st(0) // 00c33e68
        fxch st(1) // 00c33e6a
        fstp dword ptr [esp + 8] // 00c33e6c
        fld dword ptr [edx + ebx*4] // 00c33e70
        fstp dword ptr [esp + 010h] // 00c33e73
        fld dword ptr [edx + ebx*4 + 0ch] // 00c33e77
        fld dword ptr [esp + 010h] // 00c33e7b
        fld st(0) // 00c33e7f
        fsubp st(2), st(0) // 00c33e81
        fld dword ptr [esp + 8] // 00c33e83
        fld st(0) // 00c33e87
        fmulp st(3), st(0) // 00c33e89
        fxch st(2) // 00c33e8b
        faddp st(1), st(0) // 00c33e8d
        fstp dword ptr [esp + 010h] // 00c33e8f
        fld dword ptr [esp + 010h] // 00c33e93
        fstp dword ptr [esi + ebp*4] // 00c33e97
        mov eax, dword ptr [ecx] // 00c33e9a
        fld dword ptr [edx + edi*4] // 00c33e9c
        lea ebp, [edi + eax*2] // 00c33e9f
        fstp dword ptr [esp + 010h] // 00c33ea2
        add ebp, eax // 00c33ea6
        fld dword ptr [edx + edi*4 + 0ch] // 00c33ea8
        fld dword ptr [esp + 010h] // 00c33eac
        fld st(0) // 00c33eb0
        fsubp st(2), st(0) // 00c33eb2
        fxch st(1) // 00c33eb4
        fmulp st(2), st(0) // 00c33eb6
        faddp st(1), st(0) // 00c33eb8
        fstp dword ptr [esp + 010h] // 00c33eba
        fld dword ptr [esp + 010h] // 00c33ebe
        fstp dword ptr [esi + ebp*4] // 00c33ec2
        add dword ptr [ecx], 1 // 00c33ec5
        jmp l_00c33ecc // 00c33ec8
    l_00c33eca:
        fstp st(1) // 00c33eca
    l_00c33ecc:
        cmp dword ptr [esp + 028h], 1 // 00c33ecc
        jne l_00c33efc // 00c33ed1
        mov eax, dword ptr [ecx] // 00c33ed3
        mov ebp, dword ptr [esp + 020h] // 00c33ed5
        mov ebp, dword ptr [ebp - 0ch] // 00c33ed9
        lea eax, [eax + eax*2] // 00c33edc
        mov dword ptr [esi + eax*4], ebp // 00c33edf
        mov ebp, dword ptr [esp + 020h] // 00c33ee2
        mov ebp, dword ptr [ebp - 8] // 00c33ee6
        lea eax, [esi + eax*4] // 00c33ee9
        mov dword ptr [eax + 4], ebp // 00c33eec
        mov ebp, dword ptr [esp + 020h] // 00c33eef
        mov ebp, dword ptr [ebp - 4] // 00c33ef3
        mov dword ptr [eax + 8], ebp // 00c33ef6
        add dword ptr [ecx], 1 // 00c33ef9
    l_00c33efc:
        mov eax, dword ptr [esp + 02ch] // 00c33efc
        fld dword ptr [edx + eax*4 + 018h] // 00c33f00
        mov ebp, dword ptr [esp + 028h] // 00c33f04
        fstp dword ptr [esp + 028h] // 00c33f08
        fld dword ptr [esp + 028h] // 00c33f0c
        fxch st(1) // 00c33f10
        fcomi st(0), st(1) // 00c33f12
        jbe l_00c33f1a // 00c33f14
        xor eax, eax // 00c33f16
        jmp l_00c33f1f // 00c33f18
    l_00c33f1a:
        mov eax, 1 // 00c33f1a
    l_00c33f1f:
        mov dword ptr [esp + 028h], eax // 00c33f1f
        xor eax, ebp // 00c33f23
        je l_00c33fc7 // 00c33f25
        mov ebp, dword ptr [esp + 02ch] // 00c33f2b
        mov eax, dword ptr [esp + 020h] // 00c33f2f
        fld dword ptr [eax + ebp*4 - 0ch] // 00c33f33
        mov eax, dword ptr [ecx] // 00c33f37
        fstp dword ptr [esp + 010h] // 00c33f39
        lea eax, [ebp + eax*2] // 00c33f3d
        add eax, dword ptr [ecx] // 00c33f41
        fld st(0) // 00c33f43
        fld dword ptr [esp + 010h] // 00c33f45
        movss dword ptr [esi + eax*4], xmm0 // 00c33f49
        fld st(0) // 00c33f4e
        mov eax, dword ptr [esp + 020h] // 00c33f50
        fsubp st(2), st(0) // 00c33f54
        fsubp st(3), st(0) // 00c33f56
        fdivrp st(2), st(0) // 00c33f58
        fxch st(1) // 00c33f5a
        fstp dword ptr [esp + 8] // 00c33f5c
        fld dword ptr [eax + ebx*4 - 0ch] // 00c33f60
        fstp dword ptr [esp + 010h] // 00c33f64
        fld dword ptr [edx + ebx*4 + 018h] // 00c33f68
        mov eax, dword ptr [ecx] // 00c33f6c
        fld dword ptr [esp + 010h] // 00c33f6e
        lea ebp, [ebx + eax*2] // 00c33f72
        fld st(0) // 00c33f75
        add ebp, eax // 00c33f77
        fsubp st(2), st(0) // 00c33f79
        mov eax, dword ptr [esp + 020h] // 00c33f7b
        fld dword ptr [esp + 8] // 00c33f7f
        fld st(0) // 00c33f83
        fmulp st(3), st(0) // 00c33f85
        fxch st(2) // 00c33f87
        faddp st(1), st(0) // 00c33f89
        fstp dword ptr [esp + 010h] // 00c33f8b
        fld dword ptr [esp + 010h] // 00c33f8f
        fstp dword ptr [esi + ebp*4] // 00c33f93
        fld dword ptr [eax + edi*4 - 0ch] // 00c33f96
        mov eax, dword ptr [ecx] // 00c33f9a
        fstp dword ptr [esp + 010h] // 00c33f9c
        lea ebp, [edi + eax*2] // 00c33fa0
        fld dword ptr [edx + edi*4 + 018h] // 00c33fa3
        add ebp, eax // 00c33fa7
        fld dword ptr [esp + 010h] // 00c33fa9
        fld st(0) // 00c33fad
        fsubp st(2), st(0) // 00c33faf
        fxch st(1) // 00c33fb1
        fmulp st(2), st(0) // 00c33fb3
        faddp st(1), st(0) // 00c33fb5
        fstp dword ptr [esp + 010h] // 00c33fb7
        fld dword ptr [esp + 010h] // 00c33fbb
        fstp dword ptr [esi + ebp*4] // 00c33fbf
        add dword ptr [ecx], 1 // 00c33fc2
        jmp l_00c33fc9 // 00c33fc5
    l_00c33fc7:
        fstp st(1) // 00c33fc7
    l_00c33fc9:
        cmp dword ptr [esp + 028h], 1 // 00c33fc9
        jne l_00c33ff9 // 00c33fce
        mov eax, dword ptr [ecx] // 00c33fd0
        mov ebp, dword ptr [esp + 020h] // 00c33fd2
        mov ebp, dword ptr [ebp] // 00c33fd6
        lea eax, [eax + eax*2] // 00c33fd9
        mov dword ptr [esi + eax*4], ebp // 00c33fdc
        mov ebp, dword ptr [esp + 020h] // 00c33fdf
        mov ebp, dword ptr [ebp + 4] // 00c33fe3
        lea eax, [esi + eax*4] // 00c33fe6
        mov dword ptr [eax + 4], ebp // 00c33fe9
        mov ebp, dword ptr [esp + 020h] // 00c33fec
        mov ebp, dword ptr [ebp + 8] // 00c33ff0
        mov dword ptr [eax + 8], ebp // 00c33ff3
        add dword ptr [ecx], 1 // 00c33ff6
    l_00c33ff9:
        mov eax, dword ptr [esp + 02ch] // 00c33ff9
        fld dword ptr [edx + eax*4 + 024h] // 00c33ffd
        mov ebp, dword ptr [esp + 028h] // 00c34001
        fstp dword ptr [esp + 028h] // 00c34005
        fld dword ptr [esp + 028h] // 00c34009
        fxch st(1) // 00c3400d
        fcomi st(0), st(1) // 00c3400f
        jbe l_00c34017 // 00c34011
        xor eax, eax // 00c34013
        jmp l_00c3401c // 00c34015
    l_00c34017:
        mov eax, 1 // 00c34017
    l_00c3401c:
        mov dword ptr [esp + 028h], eax // 00c3401c
        xor eax, ebp // 00c34020
        je l_00c340c1 // 00c34022
        mov ebp, dword ptr [esp + 02ch] // 00c34028
        mov eax, dword ptr [esp + 020h] // 00c3402c
        fld dword ptr [eax + ebp*4] // 00c34030
        mov eax, dword ptr [ecx] // 00c34033
        fstp dword ptr [esp + 010h] // 00c34035
        lea eax, [ebp + eax*2] // 00c34039
        add eax, dword ptr [ecx] // 00c3403d
        fld st(0) // 00c3403f
        fld dword ptr [esp + 010h] // 00c34041
        movss dword ptr [esi + eax*4], xmm0 // 00c34045
        fld st(0) // 00c3404a
        mov eax, dword ptr [esp + 020h] // 00c3404c
        fsubp st(2), st(0) // 00c34050
        fsubp st(3), st(0) // 00c34052
        fdivrp st(2), st(0) // 00c34054
        fxch st(1) // 00c34056
        fstp dword ptr [esp + 8] // 00c34058
        fld dword ptr [eax + ebx*4] // 00c3405c
        fstp dword ptr [esp + 010h] // 00c3405f
        fld dword ptr [edx + ebx*4 + 024h] // 00c34063
        mov eax, dword ptr [ecx] // 00c34067
        fld dword ptr [esp + 010h] // 00c34069
        lea ebp, [ebx + eax*2] // 00c3406d
        fld st(0) // 00c34070
        add ebp, eax // 00c34072
        fsubp st(2), st(0) // 00c34074
        mov eax, dword ptr [esp + 020h] // 00c34076
        fld dword ptr [esp + 8] // 00c3407a
        fld st(0) // 00c3407e
        fmulp st(3), st(0) // 00c34080
        fxch st(2) // 00c34082
        faddp st(1), st(0) // 00c34084
        fstp dword ptr [esp + 010h] // 00c34086
        fld dword ptr [esp + 010h] // 00c3408a
        fstp dword ptr [esi + ebp*4] // 00c3408e
        fld dword ptr [eax + edi*4] // 00c34091
        mov eax, dword ptr [ecx] // 00c34094
        fstp dword ptr [esp + 010h] // 00c34096
        lea ebp, [edi + eax*2] // 00c3409a
        fld dword ptr [edx + edi*4 + 024h] // 00c3409d
        add ebp, eax // 00c340a1
        fld dword ptr [esp + 010h] // 00c340a3
        fld st(0) // 00c340a7
        fsubp st(2), st(0) // 00c340a9
        fxch st(1) // 00c340ab
        fmulp st(2), st(0) // 00c340ad
        faddp st(1), st(0) // 00c340af
        fstp dword ptr [esp + 010h] // 00c340b1
        fld dword ptr [esp + 010h] // 00c340b5
        fstp dword ptr [esi + ebp*4] // 00c340b9
        add dword ptr [ecx], 1 // 00c340bc
        jmp l_00c340c3 // 00c340bf
    l_00c340c1:
        fstp st(1) // 00c340c1
    l_00c340c3:
        cmp dword ptr [esp + 028h], 1 // 00c340c3
        jne l_00c340f3 // 00c340c8
        mov eax, dword ptr [ecx] // 00c340ca
        mov ebp, dword ptr [esp + 020h] // 00c340cc
        mov ebp, dword ptr [ebp + 0ch] // 00c340d0
        lea eax, [eax + eax*2] // 00c340d3
        mov dword ptr [esi + eax*4], ebp // 00c340d6
        mov ebp, dword ptr [esp + 020h] // 00c340d9
        mov ebp, dword ptr [ebp + 010h] // 00c340dd
        lea eax, [esi + eax*4] // 00c340e0
        mov dword ptr [eax + 4], ebp // 00c340e3
        mov ebp, dword ptr [esp + 020h] // 00c340e6
        mov ebp, dword ptr [ebp + 014h] // 00c340ea
        mov dword ptr [eax + 8], ebp // 00c340ed
        add dword ptr [ecx], 1 // 00c340f0
    l_00c340f3:
        mov eax, dword ptr [esp + 020h] // 00c340f3
        lea ebp, [eax + 0ch] // 00c340f7
        mov dword ptr [esp + 0ch], ebp // 00c340fa
        mov ebp, dword ptr [esp + 028h] // 00c340fe
        add eax, 030h // 00c34102
        add edx, 030h // 00c34105
        sub dword ptr [esp + 014h], 1 // 00c34108
        mov dword ptr [esp + 010h], ebp // 00c3410d
        mov dword ptr [esp + 020h], eax // 00c34111
        jne l_00c33d27 // 00c34115
        mov eax, dword ptr [esp + 024h] // 00c3411b
    l_00c3411f:
        mov ebp, dword ptr [esp + 018h] // 00c3411f
        cmp ebp, eax // 00c34123
        jge l_00c34230 // 00c34125
        sub eax, ebp // 00c3412b
        mov ebp, dword ptr [esp + 010h] // 00c3412d
        mov dword ptr [esp + 024h], eax // 00c34131
    l_00c34135:
        mov eax, dword ptr [esp + 02ch] // 00c34135
        fld dword ptr [edx + eax*4] // 00c34139
        fstp dword ptr [esp + 8] // 00c3413c
        fld dword ptr [esp + 8] // 00c34140
        fxch st(1) // 00c34144
        fcomi st(0), st(1) // 00c34146
        jbe l_00c3414e // 00c34148
        xor eax, eax // 00c3414a
        jmp l_00c34153 // 00c3414c
    l_00c3414e:
        mov eax, 1 // 00c3414e
    l_00c34153:
        mov dword ptr [esp + 028h], eax // 00c34153
        xor eax, ebp // 00c34157
        je l_00c341f6 // 00c34159
        mov ebp, dword ptr [esp + 02ch] // 00c3415f
        mov eax, dword ptr [esp + 0ch] // 00c34163
        fld dword ptr [eax + ebp*4] // 00c34167
        mov eax, dword ptr [ecx] // 00c3416a
        fstp dword ptr [esp + 020h] // 00c3416c
        lea eax, [ebp + eax*2] // 00c34170
        add eax, dword ptr [ecx] // 00c34174
        fld st(0) // 00c34176
        fld dword ptr [esp + 020h] // 00c34178
        movss dword ptr [esi + eax*4], xmm0 // 00c3417c
        fld st(0) // 00c34181
        mov eax, dword ptr [esp + 0ch] // 00c34183
        fsubp st(2), st(0) // 00c34187
        fsubp st(3), st(0) // 00c34189
        fdivrp st(2), st(0) // 00c3418b
        fxch st(1) // 00c3418d
        fstp dword ptr [esp + 8] // 00c3418f
        fld dword ptr [eax + ebx*4] // 00c34193
        fstp dword ptr [esp + 020h] // 00c34196
        fld dword ptr [edx + ebx*4] // 00c3419a
        mov eax, dword ptr [ecx] // 00c3419d
        fld dword ptr [esp + 020h] // 00c3419f
        lea ebp, [ebx + eax*2] // 00c341a3
        fld st(0) // 00c341a6
        add ebp, eax // 00c341a8
        fsubp st(2), st(0) // 00c341aa
        mov eax, dword ptr [esp + 0ch] // 00c341ac
        fld dword ptr [esp + 8] // 00c341b0
        fld st(0) // 00c341b4
        fmulp st(3), st(0) // 00c341b6
        fxch st(2) // 00c341b8
        faddp st(1), st(0) // 00c341ba
        fstp dword ptr [esp + 020h] // 00c341bc
        fld dword ptr [esp + 020h] // 00c341c0
        fstp dword ptr [esi + ebp*4] // 00c341c4
        fld dword ptr [eax + edi*4] // 00c341c7
        mov eax, dword ptr [ecx] // 00c341ca
        fstp dword ptr [esp + 020h] // 00c341cc
        lea ebp, [edi + eax*2] // 00c341d0
        fld dword ptr [edx + edi*4] // 00c341d3
        add ebp, eax // 00c341d6
        fld dword ptr [esp + 020h] // 00c341d8
        fld st(0) // 00c341dc
        fsubp st(2), st(0) // 00c341de
        fxch st(1) // 00c341e0
        fmulp st(2), st(0) // 00c341e2
        faddp st(1), st(0) // 00c341e4
        fstp dword ptr [esp + 020h] // 00c341e6
        fld dword ptr [esp + 020h] // 00c341ea
        fstp dword ptr [esi + ebp*4] // 00c341ee
        add dword ptr [ecx], 1 // 00c341f1
        jmp l_00c341f8 // 00c341f4
    l_00c341f6:
        fstp st(1) // 00c341f6
    l_00c341f8:
        cmp dword ptr [esp + 028h], 1 // 00c341f8
        jne l_00c3421a // 00c341fd
        mov eax, dword ptr [ecx] // 00c341ff
        mov ebp, dword ptr [edx] // 00c34201
        lea eax, [eax + eax*2] // 00c34203
        lea eax, [esi + eax*4] // 00c34206
        mov dword ptr [eax], ebp // 00c34209
        mov ebp, dword ptr [edx + 4] // 00c3420b
        mov dword ptr [eax + 4], ebp // 00c3420e
        mov ebp, dword ptr [edx + 8] // 00c34211
        mov dword ptr [eax + 8], ebp // 00c34214
        add dword ptr [ecx], 1 // 00c34217
    l_00c3421a:
        mov ebp, dword ptr [esp + 028h] // 00c3421a
        mov dword ptr [esp + 0ch], edx // 00c3421e
        add edx, 0ch // 00c34222
        sub dword ptr [esp + 024h], 1 // 00c34225
        jne l_00c34135 // 00c3422a
    l_00c34230:
        pop edi // 00c34230
        fstp st(0) // 00c34231
        pop ebp // 00c34233
        add esp, 014h // 00c34234
        ret 010h // 00c34237
    }
}
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void clip_34240_kernel(){
    __asm {
        sub esp, 014h // 00c34240
        mov eax, dword ptr [esp + 01ch] // 00c34243
        fld dword ptr [esp + 020h] // 00c34247
        mov edx, dword ptr [esp + 018h] // 00c3424b
        push ebp // 00c3424f
        mov ebp, dword ptr [esp + 028h] // 00c34250
        push edi // 00c34254
        lea eax, [eax + eax*2] // 00c34255
        lea eax, [edx + eax*4 - 0ch] // 00c34258
        mov edi, 3 // 00c3425c
        sub edi, ebp // 00c34261
        mov dword ptr [ecx], 0 // 00c34263
        fld dword ptr [eax + ebp*4] // 00c34269
        sub edi, ebx // 00c3426c
        fcomip st(0), st(1) // 00c3426e
        mov dword ptr [esp + 0ch], eax // 00c34270
        mov dword ptr [esp + 010h], 0 // 00c34274
        ja l_00c34286 // 00c3427c
        mov dword ptr [esp + 010h], 1 // 00c3427e
    l_00c34286:
        mov eax, dword ptr [esp + 024h] // 00c34286
        cmp eax, 4 // 00c3428a
        movss xmm0, dword ptr [esp + 028h] // 00c3428d
        mov dword ptr [esp + 018h], 0 // 00c34293
        jl l_00c346ad // 00c3429b
        mov eax, edx // 00c342a1
        add eax, 018h // 00c342a3
        mov dword ptr [esp + 020h], eax // 00c342a6
        mov eax, dword ptr [esp + 024h] // 00c342aa
        add eax, -4 // 00c342ae
        shr eax, 2 // 00c342b1
        add eax, 1 // 00c342b4
        mov dword ptr [esp + 014h], eax // 00c342b7
        add eax, eax // 00c342bb
        add eax, eax // 00c342bd
        mov dword ptr [esp + 018h], eax // 00c342bf
        jmp l_00c342c9 // 00c342c3
    l_00c342c5:
        mov ebp, dword ptr [esp + 02ch] // 00c342c5
    l_00c342c9:
        fld dword ptr [edx + ebp*4] // 00c342c9
        fstp dword ptr [esp + 8] // 00c342cc
        fld dword ptr [esp + 8] // 00c342d0
        fcomi st(0), st(1) // 00c342d4
        jbe l_00c342dc // 00c342d6
        xor eax, eax // 00c342d8
        jmp l_00c342e1 // 00c342da
    l_00c342dc:
        mov eax, 1 // 00c342dc
    l_00c342e1:
        mov dword ptr [esp + 028h], eax // 00c342e1
        xor eax, dword ptr [esp + 010h] // 00c342e5
        je l_00c34384 // 00c342e9
        mov ebp, dword ptr [esp + 02ch] // 00c342ef
        mov eax, dword ptr [esp + 0ch] // 00c342f3
        fld dword ptr [eax + ebp*4] // 00c342f7
        mov eax, dword ptr [ecx] // 00c342fa
        fstp dword ptr [esp + 010h] // 00c342fc
        lea eax, [ebp + eax*2] // 00c34300
        fld st(1) // 00c34304
        add eax, dword ptr [ecx] // 00c34306
        fld dword ptr [esp + 010h] // 00c34308
        fld st(0) // 00c3430c
        movss dword ptr [esi + eax*4], xmm0 // 00c3430e
        fsubp st(2), st(0) // 00c34313
        mov eax, dword ptr [esp + 0ch] // 00c34315
        fsubp st(2), st(0) // 00c34319
        fdivrp st(1), st(0) // 00c3431b
        fstp dword ptr [esp + 8] // 00c3431d
        fld dword ptr [eax + ebx*4] // 00c34321
        fstp dword ptr [esp + 010h] // 00c34324
        fld dword ptr [edx + ebx*4] // 00c34328
        mov eax, dword ptr [ecx] // 00c3432b
        fld dword ptr [esp + 010h] // 00c3432d
        lea ebp, [ebx + eax*2] // 00c34331
        fld st(0) // 00c34334
        add ebp, eax // 00c34336
        fsubp st(2), st(0) // 00c34338
        mov eax, dword ptr [esp + 0ch] // 00c3433a
        fld dword ptr [esp + 8] // 00c3433e
        fld st(0) // 00c34342
        fmulp st(3), st(0) // 00c34344
        fxch st(2) // 00c34346
        faddp st(1), st(0) // 00c34348
        fstp dword ptr [esp + 010h] // 00c3434a
        fld dword ptr [esp + 010h] // 00c3434e
        fstp dword ptr [esi + ebp*4] // 00c34352
        fld dword ptr [eax + edi*4] // 00c34355
        mov eax, dword ptr [ecx] // 00c34358
        fstp dword ptr [esp + 010h] // 00c3435a
        lea ebp, [edi + eax*2] // 00c3435e
        fld dword ptr [edx + edi*4] // 00c34361
        add ebp, eax // 00c34364
        fld dword ptr [esp + 010h] // 00c34366
        fld st(0) // 00c3436a
        fsubp st(2), st(0) // 00c3436c
        fxch st(1) // 00c3436e
        fmulp st(2), st(0) // 00c34370
        faddp st(1), st(0) // 00c34372
        fstp dword ptr [esp + 010h] // 00c34374
        fld dword ptr [esp + 010h] // 00c34378
        fstp dword ptr [esi + ebp*4] // 00c3437c
        add dword ptr [ecx], 1 // 00c3437f
        jmp l_00c34386 // 00c34382
    l_00c34384:
        fstp st(0) // 00c34384
    l_00c34386:
        cmp dword ptr [esp + 028h], 1 // 00c34386
        jne l_00c343a8 // 00c3438b
        mov eax, dword ptr [ecx] // 00c3438d
        mov ebp, dword ptr [edx] // 00c3438f
        lea eax, [eax + eax*2] // 00c34391
        lea eax, [esi + eax*4] // 00c34394
        mov dword ptr [eax], ebp // 00c34397
        mov ebp, dword ptr [edx + 4] // 00c34399
        mov dword ptr [eax + 4], ebp // 00c3439c
        mov ebp, dword ptr [edx + 8] // 00c3439f
        mov dword ptr [eax + 8], ebp // 00c343a2
        add dword ptr [ecx], 1 // 00c343a5
    l_00c343a8:
        mov eax, dword ptr [esp + 02ch] // 00c343a8
        fld dword ptr [edx + eax*4 + 0ch] // 00c343ac
        mov ebp, dword ptr [esp + 028h] // 00c343b0
        fstp dword ptr [esp + 028h] // 00c343b4
        fld dword ptr [esp + 028h] // 00c343b8
        fcomi st(0), st(1) // 00c343bc
        jbe l_00c343c4 // 00c343be
        xor eax, eax // 00c343c0
        jmp l_00c343c9 // 00c343c2
    l_00c343c4:
        mov eax, 1 // 00c343c4
    l_00c343c9:
        mov dword ptr [esp + 028h], eax // 00c343c9
        xor eax, ebp // 00c343cd
        je l_00c34460 // 00c343cf
        mov ebp, dword ptr [esp + 02ch] // 00c343d5
        fld dword ptr [edx + ebp*4] // 00c343d9
        mov eax, dword ptr [ecx] // 00c343dc
        fstp dword ptr [esp + 8] // 00c343de
        lea eax, [ebp + eax*2] // 00c343e2
        fld st(1) // 00c343e6
        add eax, dword ptr [ecx] // 00c343e8
        fld dword ptr [esp + 8] // 00c343ea
        fld st(0) // 00c343ee
        movss dword ptr [esi + eax*4], xmm0 // 00c343f0
        fsubp st(2), st(0) // 00c343f5
        mov eax, dword ptr [ecx] // 00c343f7
        lea ebp, [ebx + eax*2] // 00c343f9
        add ebp, eax // 00c343fc
        fsubp st(2), st(0) // 00c343fe
        fdivrp st(1), st(0) // 00c34400
        fstp dword ptr [esp + 8] // 00c34402
        fld dword ptr [edx + ebx*4] // 00c34406
        fstp dword ptr [esp + 010h] // 00c34409
        fld dword ptr [edx + ebx*4 + 0ch] // 00c3440d
        fld dword ptr [esp + 010h] // 00c34411
        fld st(0) // 00c34415
        fsubp st(2), st(0) // 00c34417
        fld dword ptr [esp + 8] // 00c34419
        fld st(0) // 00c3441d
        fmulp st(3), st(0) // 00c3441f
        fxch st(2) // 00c34421
        faddp st(1), st(0) // 00c34423
        fstp dword ptr [esp + 010h] // 00c34425
        fld dword ptr [esp + 010h] // 00c34429
        fstp dword ptr [esi + ebp*4] // 00c3442d
        mov eax, dword ptr [ecx] // 00c34430
        fld dword ptr [edx + edi*4] // 00c34432
        lea ebp, [edi + eax*2] // 00c34435
        fstp dword ptr [esp + 010h] // 00c34438
        add ebp, eax // 00c3443c
        fld dword ptr [edx + edi*4 + 0ch] // 00c3443e
        fld dword ptr [esp + 010h] // 00c34442
        fld st(0) // 00c34446
        fsubp st(2), st(0) // 00c34448
        fxch st(1) // 00c3444a
        fmulp st(2), st(0) // 00c3444c
        faddp st(1), st(0) // 00c3444e
        fstp dword ptr [esp + 010h] // 00c34450
        fld dword ptr [esp + 010h] // 00c34454
        fstp dword ptr [esi + ebp*4] // 00c34458
        add dword ptr [ecx], 1 // 00c3445b
        jmp l_00c34462 // 00c3445e
    l_00c34460:
        fstp st(0) // 00c34460
    l_00c34462:
        cmp dword ptr [esp + 028h], 1 // 00c34462
        jne l_00c34492 // 00c34467
        mov eax, dword ptr [ecx] // 00c34469
        mov ebp, dword ptr [esp + 020h] // 00c3446b
        mov ebp, dword ptr [ebp - 0ch] // 00c3446f
        lea eax, [eax + eax*2] // 00c34472
        mov dword ptr [esi + eax*4], ebp // 00c34475
        mov ebp, dword ptr [esp + 020h] // 00c34478
        mov ebp, dword ptr [ebp - 8] // 00c3447c
        lea eax, [esi + eax*4] // 00c3447f
        mov dword ptr [eax + 4], ebp // 00c34482
        mov ebp, dword ptr [esp + 020h] // 00c34485
        mov ebp, dword ptr [ebp - 4] // 00c34489
        mov dword ptr [eax + 8], ebp // 00c3448c
        add dword ptr [ecx], 1 // 00c3448f
    l_00c34492:
        mov eax, dword ptr [esp + 02ch] // 00c34492
        fld dword ptr [edx + eax*4 + 018h] // 00c34496
        mov ebp, dword ptr [esp + 028h] // 00c3449a
        fstp dword ptr [esp + 028h] // 00c3449e
        fld dword ptr [esp + 028h] // 00c344a2
        fcomi st(0), st(1) // 00c344a6
        jbe l_00c344ae // 00c344a8
        xor eax, eax // 00c344aa
        jmp l_00c344b3 // 00c344ac
    l_00c344ae:
        mov eax, 1 // 00c344ae
    l_00c344b3:
        mov dword ptr [esp + 028h], eax // 00c344b3
        xor eax, ebp // 00c344b7
        je l_00c34559 // 00c344b9
        mov ebp, dword ptr [esp + 02ch] // 00c344bf
        mov eax, dword ptr [esp + 020h] // 00c344c3
        fld dword ptr [eax + ebp*4 - 0ch] // 00c344c7
        mov eax, dword ptr [ecx] // 00c344cb
        fstp dword ptr [esp + 010h] // 00c344cd
        lea eax, [ebp + eax*2] // 00c344d1
        fld st(1) // 00c344d5
        add eax, dword ptr [ecx] // 00c344d7
        fld dword ptr [esp + 010h] // 00c344d9
        fld st(0) // 00c344dd
        movss dword ptr [esi + eax*4], xmm0 // 00c344df
        fsubp st(2), st(0) // 00c344e4
        mov eax, dword ptr [esp + 020h] // 00c344e6
        fsubp st(2), st(0) // 00c344ea
        fdivrp st(1), st(0) // 00c344ec
        fstp dword ptr [esp + 8] // 00c344ee
        fld dword ptr [eax + ebx*4 - 0ch] // 00c344f2
        fstp dword ptr [esp + 010h] // 00c344f6
        fld dword ptr [edx + ebx*4 + 018h] // 00c344fa
        mov eax, dword ptr [ecx] // 00c344fe
        fld dword ptr [esp + 010h] // 00c34500
        lea ebp, [ebx + eax*2] // 00c34504
        fld st(0) // 00c34507
        add ebp, eax // 00c34509
        fsubp st(2), st(0) // 00c3450b
        mov eax, dword ptr [esp + 020h] // 00c3450d
        fld dword ptr [esp + 8] // 00c34511
        fld st(0) // 00c34515
        fmulp st(3), st(0) // 00c34517
        fxch st(2) // 00c34519
        faddp st(1), st(0) // 00c3451b
        fstp dword ptr [esp + 010h] // 00c3451d
        fld dword ptr [esp + 010h] // 00c34521
        fstp dword ptr [esi + ebp*4] // 00c34525
        fld dword ptr [eax + edi*4 - 0ch] // 00c34528
        mov eax, dword ptr [ecx] // 00c3452c
        fstp dword ptr [esp + 010h] // 00c3452e
        lea ebp, [edi + eax*2] // 00c34532
        fld dword ptr [edx + edi*4 + 018h] // 00c34535
        add ebp, eax // 00c34539
        fld dword ptr [esp + 010h] // 00c3453b
        fld st(0) // 00c3453f
        fsubp st(2), st(0) // 00c34541
        fxch st(1) // 00c34543
        fmulp st(2), st(0) // 00c34545
        faddp st(1), st(0) // 00c34547
        fstp dword ptr [esp + 010h] // 00c34549
        fld dword ptr [esp + 010h] // 00c3454d
        fstp dword ptr [esi + ebp*4] // 00c34551
        add dword ptr [ecx], 1 // 00c34554
        jmp l_00c3455b // 00c34557
    l_00c34559:
        fstp st(0) // 00c34559
    l_00c3455b:
        cmp dword ptr [esp + 028h], 1 // 00c3455b
        jne l_00c3458b // 00c34560
        mov eax, dword ptr [ecx] // 00c34562
        mov ebp, dword ptr [esp + 020h] // 00c34564
        mov ebp, dword ptr [ebp] // 00c34568
        lea eax, [eax + eax*2] // 00c3456b
        mov dword ptr [esi + eax*4], ebp // 00c3456e
        mov ebp, dword ptr [esp + 020h] // 00c34571
        mov ebp, dword ptr [ebp + 4] // 00c34575
        lea eax, [esi + eax*4] // 00c34578
        mov dword ptr [eax + 4], ebp // 00c3457b
        mov ebp, dword ptr [esp + 020h] // 00c3457e
        mov ebp, dword ptr [ebp + 8] // 00c34582
        mov dword ptr [eax + 8], ebp // 00c34585
        add dword ptr [ecx], 1 // 00c34588
    l_00c3458b:
        mov eax, dword ptr [esp + 02ch] // 00c3458b
        fld dword ptr [edx + eax*4 + 024h] // 00c3458f
        mov ebp, dword ptr [esp + 028h] // 00c34593
        fstp dword ptr [esp + 028h] // 00c34597
        fld dword ptr [esp + 028h] // 00c3459b
        fcomi st(0), st(1) // 00c3459f
        jbe l_00c345a7 // 00c345a1
        xor eax, eax // 00c345a3
        jmp l_00c345ac // 00c345a5
    l_00c345a7:
        mov eax, 1 // 00c345a7
    l_00c345ac:
        mov dword ptr [esp + 028h], eax // 00c345ac
        xor eax, ebp // 00c345b0
        je l_00c3464f // 00c345b2
        mov ebp, dword ptr [esp + 02ch] // 00c345b8
        mov eax, dword ptr [esp + 020h] // 00c345bc
        fld dword ptr [eax + ebp*4] // 00c345c0
        mov eax, dword ptr [ecx] // 00c345c3
        fstp dword ptr [esp + 010h] // 00c345c5
        lea eax, [ebp + eax*2] // 00c345c9
        fld st(1) // 00c345cd
        add eax, dword ptr [ecx] // 00c345cf
        fld dword ptr [esp + 010h] // 00c345d1
        fld st(0) // 00c345d5
        movss dword ptr [esi + eax*4], xmm0 // 00c345d7
        fsubp st(2), st(0) // 00c345dc
        mov eax, dword ptr [esp + 020h] // 00c345de
        fsubp st(2), st(0) // 00c345e2
        fdivrp st(1), st(0) // 00c345e4
        fstp dword ptr [esp + 8] // 00c345e6
        fld dword ptr [eax + ebx*4] // 00c345ea
        fstp dword ptr [esp + 010h] // 00c345ed
        fld dword ptr [edx + ebx*4 + 024h] // 00c345f1
        mov eax, dword ptr [ecx] // 00c345f5
        fld dword ptr [esp + 010h] // 00c345f7
        lea ebp, [ebx + eax*2] // 00c345fb
        fld st(0) // 00c345fe
        add ebp, eax // 00c34600
        fsubp st(2), st(0) // 00c34602
        mov eax, dword ptr [esp + 020h] // 00c34604
        fld dword ptr [esp + 8] // 00c34608
        fld st(0) // 00c3460c
        fmulp st(3), st(0) // 00c3460e
        fxch st(2) // 00c34610
        faddp st(1), st(0) // 00c34612
        fstp dword ptr [esp + 010h] // 00c34614
        fld dword ptr [esp + 010h] // 00c34618
        fstp dword ptr [esi + ebp*4] // 00c3461c
        fld dword ptr [eax + edi*4] // 00c3461f
        mov eax, dword ptr [ecx] // 00c34622
        fstp dword ptr [esp + 010h] // 00c34624
        lea ebp, [edi + eax*2] // 00c34628
        fld dword ptr [edx + edi*4 + 024h] // 00c3462b
        add ebp, eax // 00c3462f
        fld dword ptr [esp + 010h] // 00c34631
        fld st(0) // 00c34635
        fsubp st(2), st(0) // 00c34637
        fxch st(1) // 00c34639
        fmulp st(2), st(0) // 00c3463b
        faddp st(1), st(0) // 00c3463d
        fstp dword ptr [esp + 010h] // 00c3463f
        fld dword ptr [esp + 010h] // 00c34643
        fstp dword ptr [esi + ebp*4] // 00c34647
        add dword ptr [ecx], 1 // 00c3464a
        jmp l_00c34651 // 00c3464d
    l_00c3464f:
        fstp st(0) // 00c3464f
    l_00c34651:
        cmp dword ptr [esp + 028h], 1 // 00c34651
        jne l_00c34681 // 00c34656
        mov eax, dword ptr [ecx] // 00c34658
        mov ebp, dword ptr [esp + 020h] // 00c3465a
        mov ebp, dword ptr [ebp + 0ch] // 00c3465e
        lea eax, [eax + eax*2] // 00c34661
        mov dword ptr [esi + eax*4], ebp // 00c34664
        mov ebp, dword ptr [esp + 020h] // 00c34667
        mov ebp, dword ptr [ebp + 010h] // 00c3466b
        lea eax, [esi + eax*4] // 00c3466e
        mov dword ptr [eax + 4], ebp // 00c34671
        mov ebp, dword ptr [esp + 020h] // 00c34674
        mov ebp, dword ptr [ebp + 014h] // 00c34678
        mov dword ptr [eax + 8], ebp // 00c3467b
        add dword ptr [ecx], 1 // 00c3467e
    l_00c34681:
        mov eax, dword ptr [esp + 020h] // 00c34681
        lea ebp, [eax + 0ch] // 00c34685
        mov dword ptr [esp + 0ch], ebp // 00c34688
        mov ebp, dword ptr [esp + 028h] // 00c3468c
        add eax, 030h // 00c34690
        add edx, 030h // 00c34693
        sub dword ptr [esp + 014h], 1 // 00c34696
        mov dword ptr [esp + 010h], ebp // 00c3469b
        mov dword ptr [esp + 020h], eax // 00c3469f
        jne l_00c342c5 // 00c346a3
        mov eax, dword ptr [esp + 024h] // 00c346a9
    l_00c346ad:
        mov ebp, dword ptr [esp + 018h] // 00c346ad
        cmp ebp, eax // 00c346b1
        jge l_00c347ba // 00c346b3
        sub eax, ebp // 00c346b9
        mov ebp, dword ptr [esp + 010h] // 00c346bb
        mov dword ptr [esp + 024h], eax // 00c346bf
    l_00c346c3:
        mov eax, dword ptr [esp + 02ch] // 00c346c3
        fld dword ptr [edx + eax*4] // 00c346c7
        fstp dword ptr [esp + 8] // 00c346ca
        fld dword ptr [esp + 8] // 00c346ce
        fcomi st(0), st(1) // 00c346d2
        jbe l_00c346da // 00c346d4
        xor eax, eax // 00c346d6
        jmp l_00c346df // 00c346d8
    l_00c346da:
        mov eax, 1 // 00c346da
    l_00c346df:
        mov dword ptr [esp + 028h], eax // 00c346df
        xor eax, ebp // 00c346e3
        je l_00c34780 // 00c346e5
        mov ebp, dword ptr [esp + 02ch] // 00c346eb
        mov eax, dword ptr [esp + 0ch] // 00c346ef
        fld dword ptr [eax + ebp*4] // 00c346f3
        mov eax, dword ptr [ecx] // 00c346f6
        fstp dword ptr [esp + 020h] // 00c346f8
        lea eax, [ebp + eax*2] // 00c346fc
        fld st(1) // 00c34700
        add eax, dword ptr [ecx] // 00c34702
        fld dword ptr [esp + 020h] // 00c34704
        fld st(0) // 00c34708
        movss dword ptr [esi + eax*4], xmm0 // 00c3470a
        fsubp st(2), st(0) // 00c3470f
        mov eax, dword ptr [esp + 0ch] // 00c34711
        fsubp st(2), st(0) // 00c34715
        fdivrp st(1), st(0) // 00c34717
        fstp dword ptr [esp + 8] // 00c34719
        fld dword ptr [eax + ebx*4] // 00c3471d
        fstp dword ptr [esp + 020h] // 00c34720
        fld dword ptr [edx + ebx*4] // 00c34724
        mov eax, dword ptr [ecx] // 00c34727
        fld dword ptr [esp + 020h] // 00c34729
        lea ebp, [ebx + eax*2] // 00c3472d
        fld st(0) // 00c34730
        add ebp, eax // 00c34732
        fsubp st(2), st(0) // 00c34734
        mov eax, dword ptr [esp + 0ch] // 00c34736
        fld dword ptr [esp + 8] // 00c3473a
        fld st(0) // 00c3473e
        fmulp st(3), st(0) // 00c34740
        fxch st(2) // 00c34742
        faddp st(1), st(0) // 00c34744
        fstp dword ptr [esp + 020h] // 00c34746
        fld dword ptr [esp + 020h] // 00c3474a
        fstp dword ptr [esi + ebp*4] // 00c3474e
        fld dword ptr [eax + edi*4] // 00c34751
        mov eax, dword ptr [ecx] // 00c34754
        fstp dword ptr [esp + 020h] // 00c34756
        lea ebp, [edi + eax*2] // 00c3475a
        fld dword ptr [edx + edi*4] // 00c3475d
        add ebp, eax // 00c34760
        fld dword ptr [esp + 020h] // 00c34762
        fld st(0) // 00c34766
        fsubp st(2), st(0) // 00c34768
        fxch st(1) // 00c3476a
        fmulp st(2), st(0) // 00c3476c
        faddp st(1), st(0) // 00c3476e
        fstp dword ptr [esp + 020h] // 00c34770
        fld dword ptr [esp + 020h] // 00c34774
        fstp dword ptr [esi + ebp*4] // 00c34778
        add dword ptr [ecx], 1 // 00c3477b
        jmp l_00c34782 // 00c3477e
    l_00c34780:
        fstp st(0) // 00c34780
    l_00c34782:
        cmp dword ptr [esp + 028h], 1 // 00c34782
        jne l_00c347a4 // 00c34787
        mov eax, dword ptr [ecx] // 00c34789
        mov ebp, dword ptr [edx] // 00c3478b
        lea eax, [eax + eax*2] // 00c3478d
        lea eax, [esi + eax*4] // 00c34790
        mov dword ptr [eax], ebp // 00c34793
        mov ebp, dword ptr [edx + 4] // 00c34795
        mov dword ptr [eax + 4], ebp // 00c34798
        mov ebp, dword ptr [edx + 8] // 00c3479b
        mov dword ptr [eax + 8], ebp // 00c3479e
        add dword ptr [ecx], 1 // 00c347a1
    l_00c347a4:
        mov ebp, dword ptr [esp + 028h] // 00c347a4
        mov dword ptr [esp + 0ch], edx // 00c347a8
        add edx, 0ch // 00c347ac
        sub dword ptr [esp + 024h], 1 // 00c347af
        jne l_00c346c3 // 00c347b4
    l_00c347ba:
        pop edi // 00c347ba
        fstp st(0) // 00c347bb
        pop ebp // 00c347bd
        add esp, 014h // 00c347be
        ret 010h // 00c347c1
    }
}
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void clip_347d0_kernel(){
    __asm {
        sub esp, 8 // 00c347d0
        push ebx // 00c347d3
        push ebp // 00c347d4
        mov ebp, dword ptr [esp + 01ch] // 00c347d5
        push esi // 00c347d9
        mov esi, dword ptr [esp + 018h] // 00c347da
        push edi // 00c347de
        mov edi, 3 // 00c347df
        sub edi, ebp // 00c347e4
        mov dword ptr [ecx], 0 // 00c347e6
        fld dword ptr [edx + ebp*4 + 024h] // 00c347ec
        fld dword ptr [esp + 020h] // 00c347f0
        mov ebx, eax // 00c347f4
        sub edi, ebx // 00c347f6
        fcomi st(0), st(1) // 00c347f8
        fstp st(1) // 00c347fa
        mov dword ptr [esp + 014h], 0 // 00c347fc
        ja l_00c3480e // 00c34804
        mov dword ptr [esp + 014h], 1 // 00c34806
    l_00c3480e:
        fld dword ptr [edx + ebp*4] // 00c3480e
        fstp dword ptr [esp + 010h] // 00c34811
        fld dword ptr [esp + 010h] // 00c34815
        fxch st(1) // 00c34819
        fcomi st(0), st(1) // 00c3481b
        jbe l_00c34823 // 00c3481d
        xor eax, eax // 00c3481f
        jmp l_00c34828 // 00c34821
    l_00c34823:
        mov eax, 1 // 00c34823
    l_00c34828:
        movss xmm0, dword ptr [esp + 020h] // 00c34828
        mov dword ptr [esp + 01ch], eax // 00c3482e
        xor eax, dword ptr [esp + 014h] // 00c34832
        je l_00c348c0 // 00c34836
        mov eax, ebp // 00c3483c
        fld dword ptr [edx + eax*4 + 024h] // 00c3483e
        movss dword ptr [esi + eax*4], xmm0 // 00c34842
        fstp dword ptr [esp + 020h] // 00c34847
        mov eax, dword ptr [ecx] // 00c3484b
        lea ebp, [ebx + eax*2] // 00c3484d
        fld st(0) // 00c34850
        add ebp, eax // 00c34852
        fld dword ptr [esp + 020h] // 00c34854
        fld st(0) // 00c34858
        fsubp st(2), st(0) // 00c3485a
        fsubp st(3), st(0) // 00c3485c
        fdivrp st(2), st(0) // 00c3485e
        fxch st(1) // 00c34860
        fstp dword ptr [esp + 020h] // 00c34862
        fld dword ptr [edx + ebx*4 + 024h] // 00c34866
        fstp dword ptr [esp + 014h] // 00c3486a
        fld dword ptr [edx + ebx*4] // 00c3486e
        fld dword ptr [esp + 014h] // 00c34871
        fld st(0) // 00c34875
        fsubp st(2), st(0) // 00c34877
        fld dword ptr [esp + 020h] // 00c34879
        fld st(0) // 00c3487d
        fmulp st(3), st(0) // 00c3487f
        fxch st(2) // 00c34881
        faddp st(1), st(0) // 00c34883
        fstp dword ptr [esp + 020h] // 00c34885
        fld dword ptr [esp + 020h] // 00c34889
        fstp dword ptr [esi + ebp*4] // 00c3488d
        mov eax, dword ptr [ecx] // 00c34890
        fld dword ptr [edx + edi*4 + 024h] // 00c34892
        lea ebp, [edi + eax*2] // 00c34896
        fstp dword ptr [esp + 020h] // 00c34899
        add ebp, eax // 00c3489d
        fld dword ptr [edx + edi*4] // 00c3489f
        fld dword ptr [esp + 020h] // 00c348a2
        fld st(0) // 00c348a6
        fsubp st(2), st(0) // 00c348a8
        fxch st(1) // 00c348aa
        fmulp st(2), st(0) // 00c348ac
        faddp st(1), st(0) // 00c348ae
        fstp dword ptr [esp + 020h] // 00c348b0
        fld dword ptr [esp + 020h] // 00c348b4
        fstp dword ptr [esi + ebp*4] // 00c348b8
        add dword ptr [ecx], 1 // 00c348bb
        jmp l_00c348c2 // 00c348be
    l_00c348c0:
        fstp st(1) // 00c348c0
    l_00c348c2:
        cmp dword ptr [esp + 01ch], 1 // 00c348c2
        jne l_00c348e4 // 00c348c7
        mov eax, dword ptr [ecx] // 00c348c9
        mov ebp, dword ptr [edx] // 00c348cb
        lea eax, [eax + eax*2] // 00c348cd
        lea eax, [esi + eax*4] // 00c348d0
        mov dword ptr [eax], ebp // 00c348d3
        mov ebp, dword ptr [edx + 4] // 00c348d5
        mov dword ptr [eax + 4], ebp // 00c348d8
        mov ebp, dword ptr [edx + 8] // 00c348db
        mov dword ptr [eax + 8], ebp // 00c348de
        add dword ptr [ecx], 1 // 00c348e1
    l_00c348e4:
        mov eax, dword ptr [esp + 024h] // 00c348e4
        fld dword ptr [edx + eax*4 + 0ch] // 00c348e8
        mov ebp, dword ptr [esp + 01ch] // 00c348ec
        fstp dword ptr [esp + 020h] // 00c348f0
        fld dword ptr [esp + 020h] // 00c348f4
        fxch st(1) // 00c348f8
        fcomi st(0), st(1) // 00c348fa
        jbe l_00c34902 // 00c348fc
        xor eax, eax // 00c348fe
        jmp l_00c34907 // 00c34900
    l_00c34902:
        mov eax, 1 // 00c34902
    l_00c34907:
        mov dword ptr [esp + 01ch], eax // 00c34907
        xor eax, ebp // 00c3490b
        je l_00c349a0 // 00c3490d
        mov ebp, dword ptr [esp + 024h] // 00c34913
        fld dword ptr [edx + ebp*4] // 00c34917
        mov eax, dword ptr [ecx] // 00c3491a
        fstp dword ptr [esp + 010h] // 00c3491c
        lea ebp, [ebp + eax*2] // 00c34920
        add ebp, eax // 00c34924
        fld st(0) // 00c34926
        movss dword ptr [esi + ebp*4], xmm0 // 00c34928
        fld dword ptr [esp + 010h] // 00c3492d
        mov eax, dword ptr [ecx] // 00c34931
        fld st(0) // 00c34933
        lea ebp, [ebx + eax*2] // 00c34935
        fsubp st(2), st(0) // 00c34938
        add ebp, eax // 00c3493a
        fsubp st(3), st(0) // 00c3493c
        fdivrp st(2), st(0) // 00c3493e
        fxch st(1) // 00c34940
        fstp dword ptr [esp + 020h] // 00c34942
        fld dword ptr [edx + ebx*4] // 00c34946
        fstp dword ptr [esp + 014h] // 00c34949
        fld dword ptr [edx + ebx*4 + 0ch] // 00c3494d
        fld dword ptr [esp + 014h] // 00c34951
        fld st(0) // 00c34955
        fsubp st(2), st(0) // 00c34957
        fld dword ptr [esp + 020h] // 00c34959
        fld st(0) // 00c3495d
        fmulp st(3), st(0) // 00c3495f
        fxch st(2) // 00c34961
        faddp st(1), st(0) // 00c34963
        fstp dword ptr [esp + 020h] // 00c34965
        fld dword ptr [esp + 020h] // 00c34969
        fstp dword ptr [esi + ebp*4] // 00c3496d
        mov eax, dword ptr [ecx] // 00c34970
        fld dword ptr [edx + edi*4] // 00c34972
        lea ebp, [edi + eax*2] // 00c34975
        fstp dword ptr [esp + 020h] // 00c34978
        add ebp, eax // 00c3497c
        fld dword ptr [edx + edi*4 + 0ch] // 00c3497e
        fld dword ptr [esp + 020h] // 00c34982
        fld st(0) // 00c34986
        fsubp st(2), st(0) // 00c34988
        fxch st(1) // 00c3498a
        fmulp st(2), st(0) // 00c3498c
        faddp st(1), st(0) // 00c3498e
        fstp dword ptr [esp + 020h] // 00c34990
        fld dword ptr [esp + 020h] // 00c34994
        fstp dword ptr [esi + ebp*4] // 00c34998
        add dword ptr [ecx], 1 // 00c3499b
        jmp l_00c349a2 // 00c3499e
    l_00c349a0:
        fstp st(1) // 00c349a0
    l_00c349a2:
        cmp dword ptr [esp + 01ch], 1 // 00c349a2
        jne l_00c349c5 // 00c349a7
        mov eax, dword ptr [ecx] // 00c349a9
        mov ebp, dword ptr [edx + 0ch] // 00c349ab
        lea eax, [eax + eax*2] // 00c349ae
        lea eax, [esi + eax*4] // 00c349b1
        mov dword ptr [eax], ebp // 00c349b4
        mov ebp, dword ptr [edx + 010h] // 00c349b6
        mov dword ptr [eax + 4], ebp // 00c349b9
        mov ebp, dword ptr [edx + 014h] // 00c349bc
        mov dword ptr [eax + 8], ebp // 00c349bf
        add dword ptr [ecx], 1 // 00c349c2
    l_00c349c5:
        mov eax, dword ptr [esp + 024h] // 00c349c5
        fld dword ptr [edx + eax*4 + 018h] // 00c349c9
        mov ebp, dword ptr [esp + 01ch] // 00c349cd
        fstp dword ptr [esp + 020h] // 00c349d1
        fld dword ptr [esp + 020h] // 00c349d5
        fxch st(1) // 00c349d9
        fcomi st(0), st(1) // 00c349db
        jbe l_00c349e3 // 00c349dd
        xor eax, eax // 00c349df
        jmp l_00c349e8 // 00c349e1
    l_00c349e3:
        mov eax, 1 // 00c349e3
    l_00c349e8:
        mov dword ptr [esp + 01ch], eax // 00c349e8
        xor eax, ebp // 00c349ec
        je l_00c34a84 // 00c349ee
        mov ebp, dword ptr [esp + 024h] // 00c349f4
        fld dword ptr [edx + ebp*4 + 0ch] // 00c349f8
        mov eax, dword ptr [ecx] // 00c349fc
        fstp dword ptr [esp + 020h] // 00c349fe
        lea ebp, [ebp + eax*2] // 00c34a02
        add ebp, eax // 00c34a06
        fld st(0) // 00c34a08
        movss dword ptr [esi + ebp*4], xmm0 // 00c34a0a
        fld dword ptr [esp + 020h] // 00c34a0f
        mov eax, dword ptr [ecx] // 00c34a13
        fld st(0) // 00c34a15
        lea ebp, [ebx + eax*2] // 00c34a17
        fsubp st(2), st(0) // 00c34a1a
        add ebp, eax // 00c34a1c
        fsubp st(3), st(0) // 00c34a1e
        fdivrp st(2), st(0) // 00c34a20
        fxch st(1) // 00c34a22
        fstp dword ptr [esp + 020h] // 00c34a24
        fld dword ptr [edx + ebx*4 + 0ch] // 00c34a28
        fstp dword ptr [esp + 010h] // 00c34a2c
        fld dword ptr [edx + ebx*4 + 018h] // 00c34a30
        fld dword ptr [esp + 010h] // 00c34a34
        fld st(0) // 00c34a38
        fsubp st(2), st(0) // 00c34a3a
        fld dword ptr [esp + 020h] // 00c34a3c
        fld st(0) // 00c34a40
        fmulp st(3), st(0) // 00c34a42
        fxch st(2) // 00c34a44
        faddp st(1), st(0) // 00c34a46
        fstp dword ptr [esp + 020h] // 00c34a48
        fld dword ptr [esp + 020h] // 00c34a4c
        fstp dword ptr [esi + ebp*4] // 00c34a50
        mov eax, dword ptr [ecx] // 00c34a53
        fld dword ptr [edx + edi*4 + 0ch] // 00c34a55
        lea ebp, [edi + eax*2] // 00c34a59
        fstp dword ptr [esp + 020h] // 00c34a5c
        add ebp, eax // 00c34a60
        fld dword ptr [edx + edi*4 + 018h] // 00c34a62
        fld dword ptr [esp + 020h] // 00c34a66
        fld st(0) // 00c34a6a
        fsubp st(2), st(0) // 00c34a6c
        fxch st(1) // 00c34a6e
        fmulp st(2), st(0) // 00c34a70
        faddp st(1), st(0) // 00c34a72
        fstp dword ptr [esp + 020h] // 00c34a74
        fld dword ptr [esp + 020h] // 00c34a78
        fstp dword ptr [esi + ebp*4] // 00c34a7c
        add dword ptr [ecx], 1 // 00c34a7f
        jmp l_00c34a86 // 00c34a82
    l_00c34a84:
        fstp st(1) // 00c34a84
    l_00c34a86:
        cmp dword ptr [esp + 01ch], 1 // 00c34a86
        jne l_00c34aa9 // 00c34a8b
        mov eax, dword ptr [ecx] // 00c34a8d
        mov ebp, dword ptr [edx + 018h] // 00c34a8f
        lea eax, [eax + eax*2] // 00c34a92
        lea eax, [esi + eax*4] // 00c34a95
        mov dword ptr [eax], ebp // 00c34a98
        mov ebp, dword ptr [edx + 01ch] // 00c34a9a
        mov dword ptr [eax + 4], ebp // 00c34a9d
        mov ebp, dword ptr [edx + 020h] // 00c34aa0
        mov dword ptr [eax + 8], ebp // 00c34aa3
        add dword ptr [ecx], 1 // 00c34aa6
    l_00c34aa9:
        mov eax, dword ptr [esp + 024h] // 00c34aa9
        fld dword ptr [edx + eax*4 + 024h] // 00c34aad
        mov ebp, dword ptr [esp + 01ch] // 00c34ab1
        fstp dword ptr [esp + 020h] // 00c34ab5
        fld dword ptr [esp + 020h] // 00c34ab9
        fxch st(1) // 00c34abd
        fcomi st(0), st(1) // 00c34abf
        jbe l_00c34ac7 // 00c34ac1
        xor eax, eax // 00c34ac3
        jmp l_00c34acc // 00c34ac5
    l_00c34ac7:
        mov eax, 1 // 00c34ac7
    l_00c34acc:
        mov dword ptr [esp + 01ch], eax // 00c34acc
        xor eax, ebp // 00c34ad0
        je l_00c34b64 // 00c34ad2
        mov ebp, dword ptr [esp + 024h] // 00c34ad8
        fld dword ptr [edx + ebp*4 + 018h] // 00c34adc
        mov eax, dword ptr [ecx] // 00c34ae0
        fstp dword ptr [esp + 020h] // 00c34ae2
        lea ebp, [ebp + eax*2] // 00c34ae6
        fld dword ptr [esp + 020h] // 00c34aea
        add ebp, eax // 00c34aee
        fld st(0) // 00c34af0
        movss dword ptr [esi + ebp*4], xmm0 // 00c34af2
        fsubp st(2), st(0) // 00c34af7
        mov eax, dword ptr [ecx] // 00c34af9
        fsubp st(2), st(0) // 00c34afb
        fdivrp st(1), st(0) // 00c34afd
        fstp dword ptr [esp + 020h] // 00c34aff
        fld dword ptr [edx + ebx*4 + 018h] // 00c34b03
        fstp dword ptr [esp + 010h] // 00c34b07
        fld dword ptr [edx + ebx*4 + 024h] // 00c34b0b
        fld dword ptr [esp + 010h] // 00c34b0f
        fld st(0) // 00c34b13
        lea ebx, [ebx + eax*2] // 00c34b15
        fsubp st(2), st(0) // 00c34b18
        add ebx, eax // 00c34b1a
        fld dword ptr [esp + 020h] // 00c34b1c
        fld st(0) // 00c34b20
        fmulp st(3), st(0) // 00c34b22
        fxch st(2) // 00c34b24
        faddp st(1), st(0) // 00c34b26
        fstp dword ptr [esp + 024h] // 00c34b28
        fld dword ptr [esp + 024h] // 00c34b2c
        fstp dword ptr [esi + ebx*4] // 00c34b30
        mov eax, dword ptr [ecx] // 00c34b33
        fld dword ptr [edx + edi*4 + 018h] // 00c34b35
        fstp dword ptr [esp + 020h] // 00c34b39
        fld dword ptr [edx + edi*4 + 024h] // 00c34b3d
        lea edi, [edi + eax*2] // 00c34b41
        fld dword ptr [esp + 020h] // 00c34b44
        add edi, eax // 00c34b48
        fld st(0) // 00c34b4a
        fsubp st(2), st(0) // 00c34b4c
        fxch st(1) // 00c34b4e
        fmulp st(2), st(0) // 00c34b50
        faddp st(1), st(0) // 00c34b52
        fstp dword ptr [esp + 024h] // 00c34b54
        fld dword ptr [esp + 024h] // 00c34b58
        fstp dword ptr [esi + edi*4] // 00c34b5c
        add dword ptr [ecx], 1 // 00c34b5f
        jmp l_00c34b68 // 00c34b62
    l_00c34b64:
        fstp st(0) // 00c34b64
        fstp st(0) // 00c34b66
    l_00c34b68:
        cmp dword ptr [esp + 01ch], 1 // 00c34b68
        jne l_00c34b8d // 00c34b6d
        mov eax, dword ptr [ecx] // 00c34b6f
        add edx, 024h // 00c34b71
        lea eax, [eax + eax*2] // 00c34b74
        lea esi, [esi + eax*4] // 00c34b77
        mov eax, dword ptr [edx] // 00c34b7a
        mov dword ptr [esi], eax // 00c34b7c
        mov eax, dword ptr [edx + 4] // 00c34b7e
        mov dword ptr [esi + 4], eax // 00c34b81
        mov edx, dword ptr [edx + 8] // 00c34b84
        mov dword ptr [esi + 8], edx // 00c34b87
        add dword ptr [ecx], 1 // 00c34b8a
    l_00c34b8d:
        pop edi // 00c34b8d
        pop esi // 00c34b8e
        pop ebp // 00c34b8f
        pop ebx // 00c34b90
        add esp, 8 // 00c34b91
        ret 0ch // 00c34b94
    }
}
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void rectangle_kernel(){
    __asm {
        movss xmm1, dword ptr constant_00d7a208 // 00c34ba0
        sub esp, 0f0h // 00c34ba8
        push ebp // 00c34bae
        mov ebp, dword ptr [esp + 0100h] // 00c34baf
        push esi // 00c34bb6
        mov esi, dword ptr [esp + 0100h] // 00c34bb7
        push ebp // 00c34bbe
        push ecx // 00c34bbf
        mov dword ptr [ecx], 0 // 00c34bc0
        movaps xmm0, xmm1 // 00c34bc6
        subss xmm0, dword ptr [esi] // 00c34bc9
        lea eax, [esp + 010h] // 00c34bcd
        movss dword ptr [esp], xmm0 // 00c34bd1
        push eax // 00c34bd6
        mov eax, edi // 00c34bd7
        call clip_347d0_kernel // 00c34bd9
        mov eax, dword ptr [ecx] // 00c34bde
        test eax, eax // 00c34be0
        je l_00c34c4e // 00c34be2
        fld dword ptr [esi] // 00c34be4
        mov esi, dword ptr [esp + 0fch] // 00c34be6
        push ebx // 00c34bed
        push ebp // 00c34bee
        push ecx // 00c34bef
        fstp dword ptr [esp] // 00c34bf0
        push eax // 00c34bf3
        lea edx, [esp + 018h] // 00c34bf4
        push edx // 00c34bf8
        mov ebx, edi // 00c34bf9
        call clip_34240_kernel // 00c34bfb
        mov eax, dword ptr [ecx] // 00c34c00
        test eax, eax // 00c34c02
        je l_00c34c4d // 00c34c04
        mov edx, dword ptr [esp + 0104h] // 00c34c06
        fld dword ptr [edx + 4] // 00c34c0d
        push edi // 00c34c10
        push ecx // 00c34c11
        fstp dword ptr [esp] // 00c34c12
        push eax // 00c34c15
        mov eax, esi // 00c34c16
        push eax // 00c34c18
        mov ebx, ebp // 00c34c19
        lea esi, [esp + 01ch] // 00c34c1b
        call clip_33710_kernel // 00c34c1f
        mov eax, dword ptr [ecx] // 00c34c24
        test eax, eax // 00c34c26
        je l_00c34c4d // 00c34c28
        mov edx, dword ptr [esp + 0104h] // 00c34c2a
        subss xmm1, dword ptr [edx + 4] // 00c34c31
        push edi // 00c34c36
        push ecx // 00c34c37
        movss dword ptr [esp], xmm1 // 00c34c38
        push eax // 00c34c3d
        mov eax, esi // 00c34c3e
        mov esi, dword ptr [esp + 010ch] // 00c34c40
        push eax // 00c34c47
        call clip_33ca0_kernel // 00c34c48
    l_00c34c4d:
        pop ebx // 00c34c4d
    l_00c34c4e:
        pop esi // 00c34c4e
        pop ebp // 00c34c4f
        add esp, 0f0h // 00c34c50
        ret 0ch // 00c34c56
    }
}
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void face_kernel(){
    __asm {
        sub esp, 011ch // 00c49230
        mov eax, dword ptr [esi + 017ch] // 00c49236
        lea ecx, [eax + eax*2 + 030h] // 00c4923c
        lea edx, [esi + ecx*4] // 00c49240
        mov ecx, 011h // 00c49243
        sub ecx, eax // 00c49248
        lea ecx, [ecx + ecx*2] // 00c4924a
        push ebx // 00c4924d
        mov dword ptr [esp + 050h], edx // 00c4924e
        lea edx, [esi + ecx*4] // 00c49252
        lea ebx, [eax + eax*2 + 6] // 00c49255
        mov ecx, 3 // 00c49259
        sub ecx, eax // 00c4925e
        lea eax, [ecx + ecx*2] // 00c49260
        mov ecx, dword ptr [esp + 0124h] // 00c49263
        shl ebx, 4 // 00c4926a
        shl eax, 4 // 00c4926d
        push ebp // 00c49270
        add ebx, esi // 00c49271
        add eax, esi // 00c49273
        sub ecx, 0 // 00c49275
        push edi // 00c49278
        mov dword ptr [esp + 020h], edx // 00c49279
        mov ebp, 2 // 00c4927d
        je l_00c492ac // 00c49282
        sub ecx, 1 // 00c49284
        je l_00c4929f // 00c49287
        sub ecx, 1 // 00c49289
        jne l_00c492bd // 00c4928c
        mov ecx, eax // 00c4928e
        add eax, 0ch // 00c49290
        xor edi, edi // 00c49293
        mov dword ptr [esp + 024h], 1 // 00c49295
        jmp l_00c492c9 // 00c4929d
    l_00c4929f:
        mov ecx, eax // 00c4929f
        add eax, 018h // 00c492a1
        xor edi, edi // 00c492a4
        mov dword ptr [esp + 024h], ebp // 00c492a6
        jmp l_00c492c9 // 00c492aa
    l_00c492ac:
        lea ecx, [eax + 0ch] // 00c492ac
        add eax, 018h // 00c492af
        mov edi, 1 // 00c492b2
        mov dword ptr [esp + 024h], ebp // 00c492b7
        jmp l_00c492c9 // 00c492bb
    l_00c492bd:
        mov ecx, dword ptr [esp + 010h] // 00c492bd
        mov eax, dword ptr [esp + 010h] // 00c492c1
        mov edi, dword ptr [esp + 010h] // 00c492c5
    l_00c492c9:
        mov edx, dword ptr [esi + 0dch] // 00c492c9
        sub edx, 0 // 00c492cf
        je l_00c492f0 // 00c492d2
        sub edx, 1 // 00c492d4
        je l_00c492e8 // 00c492d7
        sub edx, 1 // 00c492d9
        jne l_00c492fb // 00c492dc
        mov dword ptr [esp + 028h], 1 // 00c492de
        jmp l_00c492ff // 00c492e6
    l_00c492e8:
        xor edx, edx // 00c492e8
        mov dword ptr [esp + 028h], ebp // 00c492ea
        jmp l_00c492ff // 00c492ee
    l_00c492f0:
        mov edx, 1 // 00c492f0
        mov dword ptr [esp + 028h], ebp // 00c492f5
        jmp l_00c492ff // 00c492f9
    l_00c492fb:
        mov edx, dword ptr [esp + 010h] // 00c492fb
    l_00c492ff:
        mov ebp, dword ptr [esi + 0e0h] // 00c492ff
        mov dword ptr [esp + 098h], ebp // 00c49305
        mov ebp, dword ptr [esi + 0e4h] // 00c4930c
        mov dword ptr [esp + 09ch], ebp // 00c49312
        mov ebp, dword ptr [esi + 0e8h] // 00c49319
        mov dword ptr [esp + 0a0h], ebp // 00c4931f
        mov ebp, dword ptr [esp + 020h] // 00c49326
        fld dword ptr [ebp + edi*4] // 00c4932a
        fld qword ptr constant_00d7a308 // 00c4932e
        fmul st(1), st(0) // 00c49334
        fxch st(1) // 00c49336
        fstp dword ptr [esp + 020h] // 00c49338
        fld dword ptr [ecx] // 00c4933c
        fld dword ptr [esp + 020h] // 00c4933e
        fld st(0) // 00c49342
        fmulp st(2), st(0) // 00c49344
        fxch st(1) // 00c49346
        fstp dword ptr [esp + 010h] // 00c49348
        fld dword ptr [ecx + 4] // 00c4934c
        fmul st(0), st(1) // 00c4934f
        fstp dword ptr [esp + 014h] // 00c49351
        fmul dword ptr [ecx + 8] // 00c49355
        mov ecx, dword ptr [esp + 024h] // 00c49358
        fstp dword ptr [esp + 018h] // 00c4935c
        fld dword ptr [esi + 0e0h] // 00c49360
        fadd dword ptr [esp + 010h] // 00c49366
        fstp dword ptr [esp + 030h] // 00c4936a
        fld dword ptr [esi + 0e4h] // 00c4936e
        fadd dword ptr [esp + 014h] // 00c49374
        fstp dword ptr [esp + 034h] // 00c49378
        fld dword ptr [esi + 0e8h] // 00c4937c
        fadd dword ptr [esp + 018h] // 00c49382
        fstp dword ptr [esp + 038h] // 00c49386
        fmul dword ptr [ebp + ecx*4] // 00c4938a
        fstp dword ptr [esp + 020h] // 00c4938e
        fld dword ptr [esp + 020h] // 00c49392
        fst dword ptr [esp + 02ch] // 00c49396
        fld dword ptr [eax] // 00c4939a
        fstp dword ptr [esp + 020h] // 00c4939c
        fld dword ptr [esp + 020h] // 00c493a0
        fld st(0) // 00c493a4
        fld dword ptr [esp + 02ch] // 00c493a6
        fld st(0) // 00c493aa
        fmulp st(2), st(0) // 00c493ac
        fxch st(1) // 00c493ae
        fstp dword ptr [esp + 010h] // 00c493b0
        fld dword ptr [eax + 4] // 00c493b4
        fstp dword ptr [esp + 020h] // 00c493b7
        fld dword ptr [esp + 020h] // 00c493bb
        fld st(0) // 00c493bf
        fmul st(0), st(2) // 00c493c1
        fstp dword ptr [esp + 014h] // 00c493c3
        fld dword ptr [eax + 8] // 00c493c7
        fstp dword ptr [esp + 020h] // 00c493ca
        fld dword ptr [esp + 020h] // 00c493ce
        fld st(0) // 00c493d2
        fmulp st(3), st(0) // 00c493d4
        fxch st(2) // 00c493d6
        fstp dword ptr [esp + 018h] // 00c493d8
        fld dword ptr [esp + 010h] // 00c493dc
        fadd dword ptr [esp + 030h] // 00c493e0
        fstp dword ptr [esp + 03ch] // 00c493e4
        fld dword ptr [esp + 014h] // 00c493e8
        fadd dword ptr [esp + 034h] // 00c493ec
        fstp dword ptr [esp + 040h] // 00c493f0
        fld dword ptr [esp + 018h] // 00c493f4
        fadd dword ptr [esp + 038h] // 00c493f8
        fstp dword ptr [esp + 044h] // 00c493fc
        fxch st(3) // 00c49400
        fstp dword ptr [esp + 020h] // 00c49402
        fld dword ptr [esp + 020h] // 00c49406
        fld st(0) // 00c4940a
        fmulp st(3), st(0) // 00c4940c
        fxch st(2) // 00c4940e
        fstp dword ptr [esp + 010h] // 00c49410
        fld st(1) // 00c49414
        fmulp st(3), st(0) // 00c49416
        fxch st(2) // 00c49418
        fstp dword ptr [esp + 014h] // 00c4941a
        fmulp st(1), st(0) // 00c4941e
        fstp dword ptr [esp + 018h] // 00c49420
        fld dword ptr [esi + 0e0h] // 00c49424
        fadd dword ptr [esp + 010h] // 00c4942a
        fstp dword ptr [esp + 048h] // 00c4942e
        fld dword ptr [esi + 0e4h] // 00c49432
        fadd dword ptr [esp + 014h] // 00c49438
        fstp dword ptr [esp + 04ch] // 00c4943c
        fld dword ptr [esi + 0e8h] // 00c49440
        fadd dword ptr [esp + 018h] // 00c49446
        fstp dword ptr [esp + 050h] // 00c4944a
        fld dword ptr [esp + 098h] // 00c4944e
        fsub dword ptr [ebx + 024h] // 00c49455
        fstp dword ptr [esp + 010h] // 00c49458
        fld dword ptr [esp + 09ch] // 00c4945c
        fsub dword ptr [ebx + 028h] // 00c49463
        fstp dword ptr [esp + 014h] // 00c49466
        fld dword ptr [esp + 0a0h] // 00c4946a
        fsub dword ptr [ebx + 02ch] // 00c49471
        fstp dword ptr [esp + 018h] // 00c49474
        fld dword ptr [ebx + 4] // 00c49478
        fstp dword ptr [esp + 020h] // 00c4947b
        fld dword ptr [ebx] // 00c4947f
        fstp dword ptr [esp + 02ch] // 00c49481
        fld dword ptr [ebx + 8] // 00c49485
        fstp dword ptr [esp + 024h] // 00c49488
        fld dword ptr [esp + 020h] // 00c4948c
        fld st(0) // 00c49490
        fld dword ptr [esp + 014h] // 00c49492
        fld st(0) // 00c49496
        fmulp st(2), st(0) // 00c49498
        fld dword ptr [esp + 02ch] // 00c4949a
        fld st(0) // 00c4949e
        fld dword ptr [esp + 010h] // 00c494a0
        fld st(0) // 00c494a4
        fmulp st(2), st(0) // 00c494a6
        fxch st(4) // 00c494a8
        faddp st(1), st(0) // 00c494aa
        fld dword ptr [esp + 024h] // 00c494ac
        fld st(0) // 00c494b0
        fmul dword ptr [esp + 018h] // 00c494b2
        faddp st(2), st(0) // 00c494b6
        fxch st(1) // 00c494b8
        fstp dword ptr [esp + 098h] // 00c494ba
        fld dword ptr [ebx + 010h] // 00c494c1
        fmul st(0), st(3) // 00c494c4
        fld st(4) // 00c494c6
        fmul dword ptr [ebx + 0ch] // 00c494c8
        faddp st(1), st(0) // 00c494cb
        fld dword ptr [ebx + 014h] // 00c494cd
        fmul dword ptr [esp + 018h] // 00c494d0
        faddp st(1), st(0) // 00c494d4
        fstp dword ptr [esp + 09ch] // 00c494d6
        fld dword ptr [ebx + 01ch] // 00c494dd
        fmulp st(3), st(0) // 00c494e0
        fld dword ptr [ebx + 018h] // 00c494e2
        fmulp st(4), st(0) // 00c494e5
        fxch st(2) // 00c494e7
        faddp st(3), st(0) // 00c494e9
        fld dword ptr [ebx + 020h] // 00c494eb
        fmul dword ptr [esp + 018h] // 00c494ee
        faddp st(3), st(0) // 00c494f2
        fxch st(2) // 00c494f4
        fstp dword ptr [esp + 0a0h] // 00c494f6
        fld dword ptr [esp + 030h] // 00c494fd
        fsub dword ptr [ebx + 024h] // 00c49501
        fstp dword ptr [esp + 010h] // 00c49504
        fld dword ptr [esp + 034h] // 00c49508
        fsub dword ptr [ebx + 028h] // 00c4950c
        fstp dword ptr [esp + 014h] // 00c4950f
        fld dword ptr [esp + 038h] // 00c49513
        fsub dword ptr [ebx + 02ch] // 00c49517
        fstp dword ptr [esp + 018h] // 00c4951a
        fld st(2) // 00c4951e
        fld dword ptr [esp + 014h] // 00c49520
        fld st(0) // 00c49524
        fmulp st(2), st(0) // 00c49526
        fld st(3) // 00c49528
        fld dword ptr [esp + 010h] // 00c4952a
        fld st(0) // 00c4952e
        fmulp st(2), st(0) // 00c49530
        fxch st(3) // 00c49532
        faddp st(1), st(0) // 00c49534
        fld st(3) // 00c49536
        fmul dword ptr [esp + 018h] // 00c49538
        faddp st(1), st(0) // 00c4953c
        fstp dword ptr [esp + 0a4h] // 00c4953e
        fld dword ptr [ebx + 010h] // 00c49545
        fmul st(0), st(1) // 00c49548
        fld st(2) // 00c4954a
        fmul dword ptr [ebx + 0ch] // 00c4954c
        faddp st(1), st(0) // 00c4954f
        fld dword ptr [ebx + 014h] // 00c49551
        fmul dword ptr [esp + 018h] // 00c49554
        faddp st(1), st(0) // 00c49558
        fstp dword ptr [esp + 0a8h] // 00c4955a
        fmul dword ptr [ebx + 01ch] // 00c49561
        fld dword ptr [ebx + 018h] // 00c49564
        fmulp st(2), st(0) // 00c49567
        faddp st(1), st(0) // 00c49569
        fld dword ptr [ebx + 020h] // 00c4956b
        fmul dword ptr [esp + 018h] // 00c4956e
        faddp st(1), st(0) // 00c49572
        fstp dword ptr [esp + 0ach] // 00c49574
        fld dword ptr [esp + 03ch] // 00c4957b
        fsub dword ptr [ebx + 024h] // 00c4957f
        fstp dword ptr [esp + 010h] // 00c49582
        fld dword ptr [esp + 040h] // 00c49586
        fsub dword ptr [ebx + 028h] // 00c4958a
        fstp dword ptr [esp + 014h] // 00c4958d
        fld dword ptr [esp + 044h] // 00c49591
        fsub dword ptr [ebx + 02ch] // 00c49595
        fstp dword ptr [esp + 018h] // 00c49598
        fld st(2) // 00c4959c
        fld dword ptr [esp + 014h] // 00c4959e
        fld st(0) // 00c495a2
        fmulp st(2), st(0) // 00c495a4
        fld st(3) // 00c495a6
        fld dword ptr [esp + 010h] // 00c495a8
        fld st(0) // 00c495ac
        fmulp st(2), st(0) // 00c495ae
        fxch st(3) // 00c495b0
        faddp st(1), st(0) // 00c495b2
        fld st(3) // 00c495b4
        fmul dword ptr [esp + 018h] // 00c495b6
        faddp st(1), st(0) // 00c495ba
        fstp dword ptr [esp + 0b0h] // 00c495bc
        fld dword ptr [ebx + 010h] // 00c495c3
        fmul st(0), st(1) // 00c495c6
        fld st(2) // 00c495c8
        fmul dword ptr [ebx + 0ch] // 00c495ca
        faddp st(1), st(0) // 00c495cd
        fld dword ptr [ebx + 014h] // 00c495cf
        fmul dword ptr [esp + 018h] // 00c495d2
        faddp st(1), st(0) // 00c495d6
        fstp dword ptr [esp + 0b4h] // 00c495d8
        fmul dword ptr [ebx + 01ch] // 00c495df
        fld dword ptr [ebx + 018h] // 00c495e2
        fmulp st(2), st(0) // 00c495e5
        faddp st(1), st(0) // 00c495e7
        fld dword ptr [ebx + 020h] // 00c495e9
        fmul dword ptr [esp + 018h] // 00c495ec
        faddp st(1), st(0) // 00c495f0
        fstp dword ptr [esp + 0b8h] // 00c495f2
        fld dword ptr [esp + 048h] // 00c495f9
        fsub dword ptr [ebx + 024h] // 00c495fd
        fstp dword ptr [esp + 010h] // 00c49600
        fld dword ptr [esp + 04ch] // 00c49604
        fsub dword ptr [ebx + 028h] // 00c49608
        fstp dword ptr [esp + 014h] // 00c4960b
        fld dword ptr [esp + 050h] // 00c4960f
        fsub dword ptr [ebx + 02ch] // 00c49613
        fstp dword ptr [esp + 018h] // 00c49616
        fld dword ptr [esp + 014h] // 00c4961a
        fld st(0) // 00c4961e
        fmulp st(4), st(0) // 00c49620
        fld dword ptr [esp + 010h] // 00c49622
        fld st(0) // 00c49626
        fmulp st(4), st(0) // 00c49628
        fxch st(4) // 00c4962a
        faddp st(3), st(0) // 00c4962c
        fld dword ptr [esp + 018h] // 00c4962e
        fld st(0) // 00c49632
        fmulp st(3), st(0) // 00c49634
        fxch st(3) // 00c49636
        mov eax, dword ptr [esp + 058h] // 00c49638
        faddp st(2), st(0) // 00c4963c
        movss xmm0, dword ptr [eax + edx*4] // 00c4963e
        fxch st(1) // 00c49643
        mov edi, dword ptr [esp + 028h] // 00c49645
        push edx // 00c49649
        fstp dword ptr [esp + 0c0h] // 00c4964a
        movss dword ptr [esp + 014h], xmm0 // 00c49651
        fld dword ptr [ebx + 010h] // 00c49657
        movss xmm0, dword ptr [eax + edi*4] // 00c4965a
        fmul st(0), st(1) // 00c4965f
        lea edx, [esp + 014h] // 00c49661
        fld st(3) // 00c49665
        push edx // 00c49667
        fmul dword ptr [ebx + 0ch] // 00c49668
        lea eax, [esp + 0d0h] // 00c4966b
        lea ecx, [esi + 0164h] // 00c49672
        push eax // 00c49678
        faddp st(1), st(0) // 00c49679
        lea edx, [esp + 0a4h] // 00c4967b
        fld dword ptr [ebx + 014h] // 00c49682
        movss dword ptr [esp + 020h], xmm0 // 00c49685
        fmul st(0), st(3) // 00c4968b
        faddp st(1), st(0) // 00c4968d
        fstp dword ptr [esp + 0cch] // 00c4968f
        fmul dword ptr [ebx + 01ch] // 00c49696
        fld dword ptr [ebx + 018h] // 00c49699
        fmulp st(3), st(0) // 00c4969c
        faddp st(2), st(0) // 00c4969e
        fmul dword ptr [ebx + 020h] // 00c496a0
        faddp st(1), st(0) // 00c496a3
        fstp dword ptr [esp + 0d0h] // 00c496a5
        call rectangle_kernel // 00c496ac
        mov edx, dword ptr [esi + 0d8h] // 00c496b1
        xor eax, eax // 00c496b7
        mov dword ptr [edx], eax // 00c496b9
        mov edi, dword ptr [esi + 0d8h] // 00c496bb
        add edi, 4 // 00c496c1
        cmp dword ptr [ecx], eax // 00c496c4
        mov dword ptr [esp + 02ch], edi // 00c496c6
        mov dword ptr [esp + 020h], eax // 00c496ca
        jle l_00c49a16 // 00c496ce
        mov dword ptr [esp + 028h], eax // 00c496d4
        lea eax, [esp + 0d0h] // 00c496d8
        add edi, 014h // 00c496df
        mov dword ptr [esp + 024h], eax // 00c496e2
        lea ebp, [esi + 010ch] // 00c496e6
    l_00c496ec:
        mov ecx, dword ptr [esp + 058h] // 00c496ec
        mov eax, dword ptr [esi + 0dch] // 00c496f0
        fld dword ptr [ecx + eax*4] // 00c496f6
        mov edx, dword ptr [esp + 028h] // 00c496f9
        fstp qword ptr [esp + 010h] // 00c496fd
        add eax, edx // 00c49701
        fld dword ptr [esp + eax*4 + 0c8h] // 00c49703
        push ecx // 00c4970a
        fstp dword ptr [esp] // 00c4970b
        call abs_kernel // 00c4970e
        fsubr qword ptr [esp + 010h] // 00c49713
        mov eax, dword ptr [esp + 024h] // 00c49717
        fstp dword ptr [esp + 054h] // 00c4971b
        fld dword ptr [esp + 054h] // 00c4971f
        fld dword ptr constant_00d7a300 // 00c49723
        fcomip st(0), st(1) // 00c49729
        ja l_00c499ee // 00c4972b
        fld dword ptr [eax - 4] // 00c49731
        fstp dword ptr [esp + 024h] // 00c49734
        fld dword ptr [eax - 8] // 00c49738
        fstp dword ptr [esp + 054h] // 00c4973b
        fld dword ptr [eax] // 00c4973f
        fstp dword ptr [esp + 010h] // 00c49741
        fld dword ptr [esp + 054h] // 00c49745
        fld st(0) // 00c49749
        fmul dword ptr [ebx] // 00c4974b
        fld dword ptr [esp + 024h] // 00c4974d
        fld st(0) // 00c49751
        fmul dword ptr [ebx + 0ch] // 00c49753
        faddp st(2), st(0) // 00c49756
        fld dword ptr [esp + 010h] // 00c49758
        fld st(0) // 00c4975c
        fmul dword ptr [ebx + 018h] // 00c4975e
        faddp st(3), st(0) // 00c49761
        fld dword ptr [ebx + 024h] // 00c49763
        faddp st(3), st(0) // 00c49766
        fxch st(2) // 00c49768
        fstp dword ptr [ebp - 8] // 00c4976a
        fld dword ptr [ebx + 010h] // 00c4976d
        fmul st(0), st(1) // 00c49770
        fld st(3) // 00c49772
        fmul dword ptr [ebx + 4] // 00c49774
        faddp st(1), st(0) // 00c49777
        fld dword ptr [ebx + 01ch] // 00c49779
        fmul st(0), st(3) // 00c4977c
        faddp st(1), st(0) // 00c4977e
        fadd dword ptr [ebx + 028h] // 00c49780
        fstp dword ptr [ebp - 4] // 00c49783
        fmul dword ptr [ebx + 014h] // 00c49786
        fld dword ptr [ebx + 8] // 00c49789
        fmulp st(3), st(0) // 00c4978c
        faddp st(2), st(0) // 00c4978e
        fmul dword ptr [ebx + 020h] // 00c49790
        faddp st(1), st(0) // 00c49793
        fadd dword ptr [ebx + 02ch] // 00c49795
        fstp dword ptr [ebp] // 00c49798
        mov ecx, dword ptr [esi + 016ch] // 00c4979b
        mov dword ptr [edi + 4], ecx // 00c497a1
        mov edx, dword ptr [esi + 0170h] // 00c497a4
        mov dword ptr [edi + 8], edx // 00c497aa
        mov ecx, dword ptr [esi + 0174h] // 00c497ad
        mov dword ptr [edi + 0ch], ecx // 00c497b3
        fld dword ptr [esi + 0178h] // 00c497b6
        fld qword ptr constant_00d7a280 // 00c497bc
        fmul st(1), st(0) // 00c497c2
        fld st(2) // 00c497c4
        fmulp st(2), st(0) // 00c497c6
        fxch st(1) // 00c497c8
        fstp dword ptr [esp + 010h] // 00c497ca
        fld dword ptr [esi + 016ch] // 00c497ce
        fld dword ptr [esp + 010h] // 00c497d4
        fld st(0) // 00c497d8
        fmulp st(2), st(0) // 00c497da
        fxch st(1) // 00c497dc
        fstp dword ptr [esp + 048h] // 00c497de
        fld dword ptr [esi + 0170h] // 00c497e2
        fmul st(0), st(1) // 00c497e8
        fstp dword ptr [esp + 04ch] // 00c497ea
        fmul dword ptr [esi + 0174h] // 00c497ee
        fstp dword ptr [esp + 050h] // 00c497f4
        fld dword ptr [ebp - 8] // 00c497f8
        fsub dword ptr [esp + 048h] // 00c497fb
        fstp dword ptr [esp + 030h] // 00c497ff
        fld dword ptr [ebp - 4] // 00c49803
        fsub dword ptr [esp + 04ch] // 00c49806
        fstp dword ptr [esp + 034h] // 00c4980a
        fld dword ptr [ebp] // 00c4980e
        fsub dword ptr [esp + 050h] // 00c49811
        fstp dword ptr [esp + 038h] // 00c49815
        fmulp st(1), st(0) // 00c49819
        fstp dword ptr [esp + 010h] // 00c4981b
        fld dword ptr [esp + 010h] // 00c4981f
        fst dword ptr [esp + 010h] // 00c49823
        fld dword ptr [esp + 010h] // 00c49827
        fld st(0) // 00c4982b
        fmul dword ptr [esi + 016ch] // 00c4982d
        fstp dword ptr [esp + 03ch] // 00c49833
        fld dword ptr [esi + 0170h] // 00c49837
        fmul st(0), st(1) // 00c4983d
        mov ecx, dword ptr [esp + 02ch] // 00c4983f
        fstp dword ptr [esp + 040h] // 00c49843
        fmul dword ptr [esi + 0174h] // 00c49847
        fstp dword ptr [esp + 044h] // 00c4984d
        fld dword ptr [esp + 03ch] // 00c49851
        fld dword ptr [esp + 030h] // 00c49855
        fld st(0) // 00c49859
        faddp st(2), st(0) // 00c4985b
        fxch st(1) // 00c4985d
        fstp dword ptr [esp + 08ch] // 00c4985f
        fld dword ptr [esp + 040h] // 00c49866
        fadd dword ptr [esp + 034h] // 00c4986a
        fstp dword ptr [esp + 090h] // 00c4986e
        fld dword ptr [esp + 044h] // 00c49875
        fadd dword ptr [esp + 038h] // 00c49879
        fstp dword ptr [esp + 094h] // 00c4987d
        fld dword ptr [esp + 08ch] // 00c49884
        fsub dword ptr [esi + 024h] // 00c4988b
        fstp dword ptr [esp + 05ch] // 00c4988e
        fld dword ptr [esp + 090h] // 00c49892
        fsub dword ptr [esi + 028h] // 00c49899
        fstp dword ptr [esp + 060h] // 00c4989c
        fld dword ptr [esp + 094h] // 00c498a0
        fsub dword ptr [esi + 02ch] // 00c498a7
        fstp dword ptr [esp + 064h] // 00c498aa
        fld dword ptr [esi + 4] // 00c498ae
        fld dword ptr [esp + 060h] // 00c498b1
        fld st(0) // 00c498b5
        fmulp st(2), st(0) // 00c498b7
        fld dword ptr [esi] // 00c498b9
        fld dword ptr [esp + 05ch] // 00c498bb
        fld st(0) // 00c498bf
        fmulp st(2), st(0) // 00c498c1
        fxch st(3) // 00c498c3
        faddp st(1), st(0) // 00c498c5
        fld dword ptr [esi + 8] // 00c498c7
        fld dword ptr [esp + 064h] // 00c498ca
        fld st(0) // 00c498ce
        fmulp st(2), st(0) // 00c498d0
        fxch st(2) // 00c498d2
        faddp st(1), st(0) // 00c498d4
        fstp dword ptr [ecx] // 00c498d6
        fld dword ptr [esi + 010h] // 00c498d8
        fmul st(0), st(2) // 00c498db
        fld dword ptr [esi + 0ch] // 00c498dd
        fmul st(0), st(4) // 00c498e0
        faddp st(1), st(0) // 00c498e2
        fld dword ptr [esi + 014h] // 00c498e4
        fmul st(0), st(2) // 00c498e7
        faddp st(1), st(0) // 00c498e9
        fstp dword ptr [edi - 010h] // 00c498eb
        fld dword ptr [esi + 01ch] // 00c498ee
        fmulp st(2), st(0) // 00c498f1
        fld dword ptr [esi + 018h] // 00c498f3
        fmulp st(3), st(0) // 00c498f6
        fxch st(1) // 00c498f8
        faddp st(2), st(0) // 00c498fa
        fmul dword ptr [esi + 020h] // 00c498fc
        faddp st(1), st(0) // 00c498ff
        fstp dword ptr [edi - 0ch] // 00c49901
        fxch st(1) // 00c49904
        fstp dword ptr [esp + 010h] // 00c49906
        fld dword ptr [esi + 016ch] // 00c4990a
        fld dword ptr [esp + 010h] // 00c49910
        fld st(0) // 00c49914
        fmulp st(2), st(0) // 00c49916
        fxch st(1) // 00c49918
        fstp dword ptr [esp + 080h] // 00c4991a
        fld dword ptr [esi + 0170h] // 00c49921
        fmul st(0), st(1) // 00c49927
        fstp dword ptr [esp + 084h] // 00c49929
        fmul dword ptr [esi + 0174h] // 00c49930
        fstp dword ptr [esp + 088h] // 00c49936
        fsub dword ptr [esp + 080h] // 00c4993d
        fstp dword ptr [esp + 068h] // 00c49944
        fld dword ptr [esp + 034h] // 00c49948
        fsub dword ptr [esp + 084h] // 00c4994c
        fstp dword ptr [esp + 06ch] // 00c49953
        fld dword ptr [esp + 038h] // 00c49957
        fsub dword ptr [esp + 088h] // 00c4995b
        fstp dword ptr [esp + 070h] // 00c49962
        add ecx, 024h // 00c49966
        fld dword ptr [esp + 068h] // 00c49969
        mov dword ptr [esp + 02ch], ecx // 00c4996d
        fsub dword ptr [esi + 054h] // 00c49971
        add edi, 024h // 00c49974
        fstp dword ptr [esp + 074h] // 00c49977
        fld dword ptr [esp + 06ch] // 00c4997b
        fsub dword ptr [esi + 058h] // 00c4997f
        fstp dword ptr [esp + 078h] // 00c49982
        fld dword ptr [esp + 070h] // 00c49986
        fsub dword ptr [esi + 05ch] // 00c4998a
        fstp dword ptr [esp + 07ch] // 00c4998d
        fld dword ptr [esi + 034h] // 00c49991
        fld dword ptr [esp + 078h] // 00c49994
        fld st(0) // 00c49998
        fmulp st(2), st(0) // 00c4999a
        fld dword ptr [esp + 074h] // 00c4999c
        fld st(0) // 00c499a0
        fmul dword ptr [esi + 030h] // 00c499a2
        faddp st(3), st(0) // 00c499a5
        fld dword ptr [esi + 038h] // 00c499a7
        fld dword ptr [esp + 07ch] // 00c499aa
        fld st(0) // 00c499ae
        fmulp st(2), st(0) // 00c499b0
        fxch st(4) // 00c499b2
        faddp st(1), st(0) // 00c499b4
        fstp dword ptr [edi - 02ch] // 00c499b6
        fld dword ptr [esi + 040h] // 00c499b9
        fmul st(0), st(2) // 00c499bc
        fld st(1) // 00c499be
        fmul dword ptr [esi + 03ch] // 00c499c0
        faddp st(1), st(0) // 00c499c3
        fld dword ptr [esi + 044h] // 00c499c5
        fmul st(0), st(4) // 00c499c8
        faddp st(1), st(0) // 00c499ca
        fstp dword ptr [edi - 028h] // 00c499cc
        fld dword ptr [esi + 04ch] // 00c499cf
        fmulp st(2), st(0) // 00c499d2
        fmul dword ptr [esi + 048h] // 00c499d4
        faddp st(1), st(0) // 00c499d7
        fld dword ptr [esi + 050h] // 00c499d9
        fmulp st(2), st(0) // 00c499dc
        faddp st(1), st(0) // 00c499de
        fstp dword ptr [edi - 024h] // 00c499e0
        mov ecx, dword ptr [esi + 0d8h] // 00c499e3
        add dword ptr [ecx], 1 // 00c499e9
        jmp l_00c499f0 // 00c499ec
    l_00c499ee:
        fstp st(0) // 00c499ee
    l_00c499f0:
        mov ecx, dword ptr [esp + 020h] // 00c499f0
        add dword ptr [esp + 028h], 3 // 00c499f4
        add ecx, 1 // 00c499f9
        add eax, 0ch // 00c499fc
        add ebp, 0ch // 00c499ff
        cmp ecx, dword ptr [esi + 0164h] // 00c49a02
        mov dword ptr [esp + 020h], ecx // 00c49a08
        mov dword ptr [esp + 024h], eax // 00c49a0c
        jl l_00c496ec // 00c49a10
    l_00c49a16:
        pop edi // 00c49a16
        pop ebp // 00c49a17
        pop ebx // 00c49a18
        add esp, 011ch // 00c49a19
        ret 4 // 00c49a1f
    }
}
// Complete recovered instruction schedule; original private register ABI retained.
__declspec(naked) void box_box_kernel(){
    __asm {
        push ebp // 00c49a30
        mov ebp, esp // 00c49a31
        and esp, 0fffffff8h // 00c49a33
        sub esp, 022ch // 00c49a36
        mov eax, dword ptr [ebp + 0ch] // 00c49a3c
        fld dword ptr [eax + 038h] // 00c49a3f
        push ebx // 00c49a42
        fstp dword ptr [esp + 0ch] // 00c49a43
        push esi // 00c49a47
        mov esi, dword ptr [ebp + 010h] // 00c49a48
        fld dword ptr [esi + 0ch] // 00c49a4b
        push edi // 00c49a4e
        fstp dword ptr [esp + 044h] // 00c49a4f
        fld dword ptr [esi] // 00c49a53
        fstp dword ptr [esp + 03ch] // 00c49a55
        fld dword ptr [eax + 034h] // 00c49a59
        fstp dword ptr [esp + 010h] // 00c49a5c
        fld dword ptr [eax + 03ch] // 00c49a60
        fstp dword ptr [esp + 018h] // 00c49a63
        fld dword ptr [esi + 018h] // 00c49a67
        fstp dword ptr [esp + 040h] // 00c49a6a
        fld dword ptr [esp + 010h] // 00c49a6e
        fld st(0) // 00c49a72
        fld dword ptr [esp + 03ch] // 00c49a74
        fld st(0) // 00c49a78
        fmulp st(2), st(0) // 00c49a7a
        fld dword ptr [esp + 044h] // 00c49a7c
        fld st(0) // 00c49a80
        fld dword ptr [esp + 014h] // 00c49a82
        fld st(0) // 00c49a86
        fmulp st(2), st(0) // 00c49a88
        fxch st(4) // 00c49a8a
        faddp st(1), st(0) // 00c49a8c
        fld dword ptr [esp + 040h] // 00c49a8e
        fld st(0) // 00c49a92
        fmul dword ptr [esp + 018h] // 00c49a94
        faddp st(2), st(0) // 00c49a98
        fxch st(1) // 00c49a9a
        fstp dword ptr [esp + 0f0h] // 00c49a9c
        fld dword ptr [esi + 010h] // 00c49aa3
        fstp dword ptr [esp + 044h] // 00c49aa6
        fld dword ptr [esi + 4] // 00c49aaa
        fstp dword ptr [esp + 03ch] // 00c49aad
        fld dword ptr [esi + 01ch] // 00c49ab1
        fstp dword ptr [esp + 040h] // 00c49ab4
        fld dword ptr [esp + 03ch] // 00c49ab8
        fld st(0) // 00c49abc
        fmulp st(6), st(0) // 00c49abe
        fld dword ptr [esp + 044h] // 00c49ac0
        fld st(0) // 00c49ac4
        fmulp st(6), st(0) // 00c49ac6
        fxch st(6) // 00c49ac8
        faddp st(5), st(0) // 00c49aca
        fld dword ptr [esp + 040h] // 00c49acc
        fmul dword ptr [esp + 018h] // 00c49ad0
        faddp st(5), st(0) // 00c49ad4
        fxch st(4) // 00c49ad6
        fstp dword ptr [esp + 0f4h] // 00c49ad8
        fld dword ptr [esi + 014h] // 00c49adf
        fstp dword ptr [esp + 044h] // 00c49ae2
        fld dword ptr [esi + 8] // 00c49ae6
        fstp dword ptr [esp + 054h] // 00c49ae9
        fld dword ptr [esi + 020h] // 00c49aed
        fstp dword ptr [esp + 03ch] // 00c49af0
        fld dword ptr [esp + 054h] // 00c49af4
        fmul dword ptr [esp + 010h] // 00c49af8
        fld dword ptr [esp + 044h] // 00c49afc
        fmul dword ptr [esp + 014h] // 00c49b00
        faddp st(1), st(0) // 00c49b04
        fld dword ptr [esp + 03ch] // 00c49b06
        fmul dword ptr [esp + 018h] // 00c49b0a
        faddp st(1), st(0) // 00c49b0e
        fstp dword ptr [esp + 0f8h] // 00c49b10
        fld dword ptr [eax + 044h] // 00c49b17
        fstp dword ptr [esp + 014h] // 00c49b1a
        fld dword ptr [eax + 040h] // 00c49b1e
        fstp dword ptr [esp + 018h] // 00c49b21
        fld dword ptr [eax + 048h] // 00c49b25
        fstp dword ptr [esp + 010h] // 00c49b28
        fld dword ptr [esp + 018h] // 00c49b2c
        fmul st(0), st(3) // 00c49b30
        fld dword ptr [esp + 014h] // 00c49b32
        fmul st(0), st(3) // 00c49b36
        faddp st(1), st(0) // 00c49b38
        fld dword ptr [esp + 010h] // 00c49b3a
        fmul st(0), st(2) // 00c49b3e
        faddp st(1), st(0) // 00c49b40
        fstp dword ptr [esp + 0fch] // 00c49b42
        fld dword ptr [esp + 018h] // 00c49b49
        fmul st(0), st(4) // 00c49b4d
        fld dword ptr [esp + 014h] // 00c49b4f
        fmul st(0), st(6) // 00c49b53
        faddp st(1), st(0) // 00c49b55
        fld dword ptr [esp + 010h] // 00c49b57
        fmul dword ptr [esp + 040h] // 00c49b5b
        faddp st(1), st(0) // 00c49b5f
        fstp dword ptr [esp + 0100h] // 00c49b61
        fld dword ptr [esp + 018h] // 00c49b68
        fmul dword ptr [esp + 054h] // 00c49b6c
        fld dword ptr [esp + 014h] // 00c49b70
        fmul dword ptr [esp + 044h] // 00c49b74
        faddp st(1), st(0) // 00c49b78
        fld dword ptr [esp + 010h] // 00c49b7a
        fmul dword ptr [esp + 03ch] // 00c49b7e
        faddp st(1), st(0) // 00c49b82
        fstp dword ptr [esp + 0104h] // 00c49b84
        fld dword ptr [eax + 050h] // 00c49b8b
        fstp dword ptr [esp + 014h] // 00c49b8e
        fld dword ptr [eax + 04ch] // 00c49b92
        fstp dword ptr [esp + 010h] // 00c49b95
        fld dword ptr [eax + 054h] // 00c49b99
        fstp dword ptr [esp + 018h] // 00c49b9c
        fld dword ptr [esp + 010h] // 00c49ba0
        fmul st(0), st(3) // 00c49ba4
        fld dword ptr [esp + 014h] // 00c49ba6
        fmul st(0), st(3) // 00c49baa
        faddp st(1), st(0) // 00c49bac
        fld dword ptr [esp + 018h] // 00c49bae
        fmul st(0), st(2) // 00c49bb2
        faddp st(1), st(0) // 00c49bb4
        fstp dword ptr [esp + 0108h] // 00c49bb6
        fld dword ptr [esp + 010h] // 00c49bbd
        fmul st(0), st(4) // 00c49bc1
        fld dword ptr [esp + 014h] // 00c49bc3
        fmul st(0), st(6) // 00c49bc7
        faddp st(1), st(0) // 00c49bc9
        fld dword ptr [esp + 018h] // 00c49bcb
        fmul dword ptr [esp + 040h] // 00c49bcf
        faddp st(1), st(0) // 00c49bd3
        fstp dword ptr [esp + 010ch] // 00c49bd5
        fld dword ptr [esp + 010h] // 00c49bdc
        fmul dword ptr [esp + 054h] // 00c49be0
        fld dword ptr [esp + 014h] // 00c49be4
        fmul dword ptr [esp + 044h] // 00c49be8
        faddp st(1), st(0) // 00c49bec
        fld dword ptr [esp + 018h] // 00c49bee
        fmul dword ptr [esp + 03ch] // 00c49bf2
        faddp st(1), st(0) // 00c49bf6
        fstp dword ptr [esp + 0110h] // 00c49bf8
        fld dword ptr [eax + 05ch] // 00c49bff
        fstp dword ptr [esp + 014h] // 00c49c02
        fld dword ptr [eax + 058h] // 00c49c06
        fstp dword ptr [esp + 010h] // 00c49c09
        fld dword ptr [eax + 060h] // 00c49c0d
        fstp dword ptr [esp + 018h] // 00c49c10
        fld dword ptr [esp + 010h] // 00c49c14
        fld st(0) // 00c49c18
        fmulp st(4), st(0) // 00c49c1a
        fld dword ptr [esp + 014h] // 00c49c1c
        fld st(0) // 00c49c20
        fmulp st(4), st(0) // 00c49c22
        fxch st(4) // 00c49c24
        faddp st(3), st(0) // 00c49c26
        fld dword ptr [esp + 018h] // 00c49c28
        fld st(0) // 00c49c2c
        fmulp st(3), st(0) // 00c49c2e
        fxch st(3) // 00c49c30
        faddp st(2), st(0) // 00c49c32
        fld dword ptr [esi + 024h] // 00c49c34
        faddp st(2), st(0) // 00c49c37
        fxch st(1) // 00c49c39
        fstp dword ptr [esp + 0114h] // 00c49c3b
        fld st(0) // 00c49c42
        fmulp st(4), st(0) // 00c49c44
        fld st(2) // 00c49c46
        fmulp st(5), st(0) // 00c49c48
        fxch st(3) // 00c49c4a
        mov edi, dword ptr [ebp + 018h] // 00c49c4c
        faddp st(4), st(0) // 00c49c4f
        mov ecx, dword ptr [ebp + 014h] // 00c49c51
        fld st(0) // 00c49c54
        fmul dword ptr [esp + 040h] // 00c49c56
        faddp st(4), st(0) // 00c49c5a
        fld dword ptr [esi + 028h] // 00c49c5c
        faddp st(4), st(0) // 00c49c5f
        fxch st(3) // 00c49c61
        fstp dword ptr [esp + 0118h] // 00c49c63
        fld dword ptr [esp + 054h] // 00c49c6a
        fmulp st(2), st(0) // 00c49c6e
        fmul dword ptr [esp + 044h] // 00c49c70
        faddp st(1), st(0) // 00c49c74
        fld dword ptr [esp + 03ch] // 00c49c76
        fmulp st(2), st(0) // 00c49c7a
        faddp st(1), st(0) // 00c49c7c
        fadd dword ptr [esi + 02ch] // 00c49c7e
        fstp dword ptr [esp + 011ch] // 00c49c81
        fld dword ptr [ecx + 038h] // 00c49c88
        fstp dword ptr [esp + 044h] // 00c49c8b
        fld dword ptr [edi + 0ch] // 00c49c8f
        fstp dword ptr [esp + 03ch] // 00c49c92
        fld dword ptr [ecx + 034h] // 00c49c96
        fstp dword ptr [esp + 018h] // 00c49c99
        fld dword ptr [edi] // 00c49c9d
        fstp dword ptr [esp + 014h] // 00c49c9f
        fld dword ptr [ecx + 03ch] // 00c49ca3
        fstp dword ptr [esp + 040h] // 00c49ca6
        fld dword ptr [edi + 018h] // 00c49caa
        fstp dword ptr [esp + 010h] // 00c49cad
        fld dword ptr [esp + 014h] // 00c49cb1
        fld st(0) // 00c49cb5
        fld dword ptr [esp + 018h] // 00c49cb7
        fld st(0) // 00c49cbb
        fmulp st(2), st(0) // 00c49cbd
        fld dword ptr [esp + 03ch] // 00c49cbf
        fld st(0) // 00c49cc3
        fld dword ptr [esp + 044h] // 00c49cc5
        fld st(0) // 00c49cc9
        fmulp st(2), st(0) // 00c49ccb
        fxch st(4) // 00c49ccd
        faddp st(1), st(0) // 00c49ccf
        fld dword ptr [esp + 040h] // 00c49cd1
        fld st(0) // 00c49cd5
        fmul dword ptr [esp + 010h] // 00c49cd7
        faddp st(2), st(0) // 00c49cdb
        fxch st(1) // 00c49cdd
        fstp dword ptr [esp + 0120h] // 00c49cdf
        fld dword ptr [edi + 010h] // 00c49ce6
        fstp dword ptr [esp + 018h] // 00c49ce9
        fld dword ptr [edi + 4] // 00c49ced
        fstp dword ptr [esp + 014h] // 00c49cf0
        fld dword ptr [edi + 01ch] // 00c49cf4
        fstp dword ptr [esp + 03ch] // 00c49cf7
        fld st(2) // 00c49cfb
        fmul dword ptr [esp + 014h] // 00c49cfd
        fld st(4) // 00c49d01
        fmul dword ptr [esp + 018h] // 00c49d03
        faddp st(1), st(0) // 00c49d07
        fld st(1) // 00c49d09
        fmul dword ptr [esp + 03ch] // 00c49d0b
        faddp st(1), st(0) // 00c49d0f
        fstp dword ptr [esp + 0124h] // 00c49d11
        fld dword ptr [edi + 014h] // 00c49d18
        fstp dword ptr [esp + 044h] // 00c49d1b
        fld dword ptr [edi + 8] // 00c49d1f
        fstp dword ptr [esp + 054h] // 00c49d22
        fld dword ptr [edi + 020h] // 00c49d26
        fstp dword ptr [esp + 040h] // 00c49d29
        fld dword ptr [esp + 054h] // 00c49d2d
        fld st(0) // 00c49d31
        fmulp st(4), st(0) // 00c49d33
        fld dword ptr [esp + 044h] // 00c49d35
        fmulp st(5), st(0) // 00c49d39
        fxch st(3) // 00c49d3b
        faddp st(4), st(0) // 00c49d3d
        fmul dword ptr [esp + 040h] // 00c49d3f
        faddp st(3), st(0) // 00c49d43
        fxch st(2) // 00c49d45
        fstp dword ptr [esp + 0128h] // 00c49d47
        fld dword ptr [ecx + 044h] // 00c49d4e
        fstp dword ptr [esp + 080h] // 00c49d51
        fld dword ptr [ecx + 040h] // 00c49d58
        fstp dword ptr [esp + 054h] // 00c49d5b
        fld dword ptr [ecx + 048h] // 00c49d5f
        fstp dword ptr [esp + 028h] // 00c49d62
        fld st(2) // 00c49d66
        fld dword ptr [esp + 054h] // 00c49d68
        fld st(0) // 00c49d6c
        fmulp st(2), st(0) // 00c49d6e
        fld dword ptr [esp + 080h] // 00c49d70
        fld st(0) // 00c49d77
        fmulp st(5), st(0) // 00c49d79
        fxch st(2) // 00c49d7b
        faddp st(4), st(0) // 00c49d7d
        fld dword ptr [esp + 028h] // 00c49d7f
        fld st(0) // 00c49d83
        fmul dword ptr [esp + 010h] // 00c49d85
        faddp st(5), st(0) // 00c49d89
        fxch st(4) // 00c49d8b
        fstp dword ptr [esp + 012ch] // 00c49d8d
        fld st(0) // 00c49d94
        fmul dword ptr [esp + 014h] // 00c49d96
        fld st(2) // 00c49d9a
        fmul dword ptr [esp + 018h] // 00c49d9c
        faddp st(1), st(0) // 00c49da0
        fld st(4) // 00c49da2
        fmul dword ptr [esp + 03ch] // 00c49da4
        faddp st(1), st(0) // 00c49da8
        fstp dword ptr [esp + 0130h] // 00c49daa
        fmulp st(2), st(0) // 00c49db1
        fmul dword ptr [esp + 044h] // 00c49db3
        faddp st(1), st(0) // 00c49db7
        fld dword ptr [esp + 040h] // 00c49db9
        fmulp st(2), st(0) // 00c49dbd
        faddp st(1), st(0) // 00c49dbf
        fstp dword ptr [esp + 0134h] // 00c49dc1
        fld dword ptr [ecx + 050h] // 00c49dc8
        fstp dword ptr [esp + 080h] // 00c49dcb
        fld dword ptr [edi + 0ch] // 00c49dd2
        fstp dword ptr [esp + 028h] // 00c49dd5
        fld dword ptr [ecx + 04ch] // 00c49dd9
        fstp dword ptr [esp + 014h] // 00c49ddc
        fld dword ptr [ecx + 054h] // 00c49de0
        fstp dword ptr [esp + 018h] // 00c49de3
        fld dword ptr [edi + 018h] // 00c49de7
        fstp dword ptr [esp + 010h] // 00c49dea
        fld dword ptr [esp + 028h] // 00c49dee
        fld st(0) // 00c49df2
        fld dword ptr [esp + 080h] // 00c49df4
        fld st(0) // 00c49dfb
        fmulp st(2), st(0) // 00c49dfd
        fld st(3) // 00c49dff
        fld dword ptr [esp + 014h] // 00c49e01
        fld st(0) // 00c49e05
        fmulp st(2), st(0) // 00c49e07
        fxch st(3) // 00c49e09
        faddp st(1), st(0) // 00c49e0b
        fld dword ptr [esp + 010h] // 00c49e0d
        fld dword ptr [esp + 018h] // 00c49e11
        fld st(0) // 00c49e15
        fmulp st(2), st(0) // 00c49e17
        fxch st(2) // 00c49e19
        faddp st(1), st(0) // 00c49e1b
        fstp dword ptr [esp + 0138h] // 00c49e1d
        fld dword ptr [edi + 010h] // 00c49e24
        fstp dword ptr [esp + 018h] // 00c49e27
        fld dword ptr [edi + 4] // 00c49e2b
        fstp dword ptr [esp + 014h] // 00c49e2e
        fld dword ptr [edi + 01ch] // 00c49e32
        fstp dword ptr [esp + 03ch] // 00c49e35
        fld dword ptr [esp + 014h] // 00c49e39
        fmul st(0), st(3) // 00c49e3d
        fld dword ptr [esp + 018h] // 00c49e3f
        fmul st(0), st(3) // 00c49e43
        faddp st(1), st(0) // 00c49e45
        fld dword ptr [esp + 03ch] // 00c49e47
        fmul st(0), st(2) // 00c49e4b
        faddp st(1), st(0) // 00c49e4d
        fstp dword ptr [esp + 013ch] // 00c49e4f
        fld dword ptr [edi + 014h] // 00c49e56
        mov edx, dword ptr [eax + 0210h] // 00c49e59
        fstp dword ptr [esp + 080h] // 00c49e5f
        mov dword ptr [esp + 0150h], edx // 00c49e66
        fld dword ptr [edi + 8] // 00c49e6d
        mov edx, dword ptr [eax + 0214h] // 00c49e70
        fstp dword ptr [esp + 028h] // 00c49e76
        mov eax, dword ptr [eax + 0218h] // 00c49e7a
        fld dword ptr [edi + 020h] // 00c49e80
        mov dword ptr [esp + 0154h], edx // 00c49e83
        fstp dword ptr [esp + 044h] // 00c49e8a
        mov edx, dword ptr [ecx + 0210h] // 00c49e8e
        fld dword ptr [esp + 028h] // 00c49e94
        mov dword ptr [esp + 0158h], eax // 00c49e98
        fld st(0) // 00c49e9f
        mov eax, dword ptr [ecx + 0214h] // 00c49ea1
        fmulp st(4), st(0) // 00c49ea7
        mov dword ptr [esp + 015ch], edx // 00c49ea9
        fld dword ptr [esp + 080h] // 00c49eb0
        mov edx, dword ptr [ebp + 8] // 00c49eb7
        fld st(0) // 00c49eba
        mov dword ptr [esp + 0160h], eax // 00c49ebc
        fmulp st(4), st(0) // 00c49ec3
        mov dword ptr [esp + 0168h], edx // 00c49ec5
        fxch st(4) // 00c49ecc
        faddp st(3), st(0) // 00c49ece
        fld dword ptr [esp + 044h] // 00c49ed0
        fmulp st(2), st(0) // 00c49ed4
        fxch st(2) // 00c49ed6
        faddp st(1), st(0) // 00c49ed8
        fstp dword ptr [esp + 0140h] // 00c49eda
        fld dword ptr [ecx + 05ch] // 00c49ee1
        fstp dword ptr [esp + 028h] // 00c49ee4
        fld dword ptr [ecx + 058h] // 00c49ee8
        fstp dword ptr [esp + 080h] // 00c49eeb
        fld dword ptr [ecx + 060h] // 00c49ef2
        mov ecx, dword ptr [ecx + 0218h] // 00c49ef5
        fstp dword ptr [esp + 040h] // 00c49efb
        mov dword ptr [esp + 0164h], ecx // 00c49eff
        fld dword ptr [esp + 028h] // 00c49f06
        fld st(0) // 00c49f0a
        fmulp st(4), st(0) // 00c49f0c
        fld dword ptr [esp + 080h] // 00c49f0e
        fld st(0) // 00c49f15
        fmulp st(6), st(0) // 00c49f17
        fxch st(4) // 00c49f19
        faddp st(5), st(0) // 00c49f1b
        fld dword ptr [esp + 040h] // 00c49f1d
        fld st(0) // 00c49f21
        fmul dword ptr [esp + 010h] // 00c49f23
        faddp st(6), st(0) // 00c49f27
        fld dword ptr [edi + 024h] // 00c49f29
        faddp st(6), st(0) // 00c49f2c
        fxch st(5) // 00c49f2e
        fstp dword ptr [esp + 0144h] // 00c49f30
        fld st(3) // 00c49f37
        fmul dword ptr [esp + 014h] // 00c49f39
        fld st(1) // 00c49f3d
        fmul dword ptr [esp + 018h] // 00c49f3f
        faddp st(1), st(0) // 00c49f43
        fld st(5) // 00c49f45
        fmul dword ptr [esp + 03ch] // 00c49f47
        faddp st(1), st(0) // 00c49f4b
        fadd dword ptr [edi + 028h] // 00c49f4d
        fstp dword ptr [esp + 0148h] // 00c49f50
        fxch st(3) // 00c49f57
        fmulp st(1), st(0) // 00c49f59
        fxch st(2) // 00c49f5b
        fmulp st(1), st(0) // 00c49f5d
        faddp st(1), st(0) // 00c49f5f
        fld dword ptr [esp + 044h] // 00c49f61
        fmulp st(2), st(0) // 00c49f65
        faddp st(1), st(0) // 00c49f67
        fadd dword ptr [edi + 02ch] // 00c49f69
        fstp dword ptr [esp + 014ch] // 00c49f6c
        fld dword ptr [esp + 0120h] // 00c49f73
        fld st(0) // 00c49f7a
        fld dword ptr [esp + 0f0h] // 00c49f7c
        fld st(0) // 00c49f83
        fmulp st(2), st(0) // 00c49f85
        fld dword ptr [esp + 0124h] // 00c49f87
        fld st(0) // 00c49f8e
        fld dword ptr [esp + 0f4h] // 00c49f90
        fld st(0) // 00c49f97
        fmulp st(2), st(0) // 00c49f99
        fxch st(4) // 00c49f9b
        faddp st(1), st(0) // 00c49f9d
        fld dword ptr [esp + 0128h] // 00c49f9f
        fld dword ptr [esp + 0f8h] // 00c49fa6
        fld st(0) // 00c49fad
        fmulp st(2), st(0) // 00c49faf
        fxch st(2) // 00c49fb1
        faddp st(1), st(0) // 00c49fb3
        fstp dword ptr [esp + 0214h] // 00c49fb5
        fld st(4) // 00c49fbc
        fmul dword ptr [esp + 0fch] // 00c49fbe
        fld st(2) // 00c49fc5
        fmul dword ptr [esp + 0100h] // 00c49fc7
        faddp st(1), st(0) // 00c49fce
        fld dword ptr [esp + 0128h] // 00c49fd0
        fmul dword ptr [esp + 0104h] // 00c49fd7
        faddp st(1), st(0) // 00c49fde
        fstp dword ptr [esp + 0218h] // 00c49fe0
        fld st(1) // 00c49fe7
        fmul dword ptr [esp + 010ch] // 00c49fe9
        fld st(5) // 00c49ff0
        fmul dword ptr [esp + 0108h] // 00c49ff2
        faddp st(1), st(0) // 00c49ff9
        fld dword ptr [esp + 0128h] // 00c49ffb
        fmul dword ptr [esp + 0110h] // 00c4a002
        faddp st(1), st(0) // 00c4a009
        fstp dword ptr [esp + 021ch] // 00c4a00b
        fld dword ptr [esp + 012ch] // 00c4a012
        fmul st(0), st(3) // 00c4a019
        fld dword ptr [esp + 0130h] // 00c4a01b
        fmul st(0), st(5) // 00c4a022
        faddp st(1), st(0) // 00c4a024
        fld st(1) // 00c4a026
        fmul dword ptr [esp + 0134h] // 00c4a028
        faddp st(1), st(0) // 00c4a02f
        fstp dword ptr [esp + 0220h] // 00c4a031
        fld dword ptr [esp + 012ch] // 00c4a038
        fmul dword ptr [esp + 0fch] // 00c4a03f
        fld dword ptr [esp + 0130h] // 00c4a046
        fmul dword ptr [esp + 0100h] // 00c4a04d
        faddp st(1), st(0) // 00c4a054
        fld dword ptr [esp + 0104h] // 00c4a056
        fmul dword ptr [esp + 0134h] // 00c4a05d
        faddp st(1), st(0) // 00c4a064
        fstp dword ptr [esp + 0224h] // 00c4a066
        fld dword ptr [esp + 0130h] // 00c4a06d
        fmul dword ptr [esp + 010ch] // 00c4a074
        fld dword ptr [esp + 012ch] // 00c4a07b
        fmul dword ptr [esp + 0108h] // 00c4a082
        faddp st(1), st(0) // 00c4a089
        fld dword ptr [esp + 0110h] // 00c4a08b
        fmul dword ptr [esp + 0134h] // 00c4a092
        faddp st(1), st(0) // 00c4a099
        fstp dword ptr [esp + 0228h] // 00c4a09b
        fld st(2) // 00c4a0a2
        fmul dword ptr [esp + 0138h] // 00c4a0a4
        fld st(4) // 00c4a0ab
        fmul dword ptr [esp + 013ch] // 00c4a0ad
        faddp st(1), st(0) // 00c4a0b4
        fld st(1) // 00c4a0b6
        fmul dword ptr [esp + 0140h] // 00c4a0b8
        faddp st(1), st(0) // 00c4a0bf
        fstp dword ptr [esp + 022ch] // 00c4a0c1
        fld dword ptr [esp + 0fch] // 00c4a0c8
        fmul dword ptr [esp + 0138h] // 00c4a0cf
        fld dword ptr [esp + 0100h] // 00c4a0d6
        fmul dword ptr [esp + 013ch] // 00c4a0dd
        faddp st(1), st(0) // 00c4a0e4
        fld dword ptr [esp + 0104h] // 00c4a0e6
        fmul dword ptr [esp + 0140h] // 00c4a0ed
        faddp st(1), st(0) // 00c4a0f4
        fstp dword ptr [esp + 0230h] // 00c4a0f6
        fld dword ptr [esp + 010ch] // 00c4a0fd
        fmul dword ptr [esp + 013ch] // 00c4a104
        fld dword ptr [esp + 0108h] // 00c4a10b
        fmul dword ptr [esp + 0138h] // 00c4a112
        faddp st(1), st(0) // 00c4a119
        fld dword ptr [esp + 0110h] // 00c4a11b
        movss xmm0, dword ptr constant_00d7a244 // 00c4a122
        fmul dword ptr [esp + 0140h] // 00c4a12a
        lea eax, [esp + 0f0h] // 00c4a131
        movss dword ptr [esp + 054h], xmm0 // 00c4a138
        xor ebx, ebx // 00c4a13e
        faddp st(1), st(0) // 00c4a140
        mov dword ptr [esp + 010h], eax // 00c4a142
        fstp dword ptr [esp + 0234h] // 00c4a146
        fld dword ptr [esp + 0144h] // 00c4a14d
        fsub dword ptr [esp + 0114h] // 00c4a154
        fstp dword ptr [esp + 058h] // 00c4a15b
        fld dword ptr [esp + 0148h] // 00c4a15f
        fsub dword ptr [esp + 0118h] // 00c4a166
        fstp dword ptr [esp + 05ch] // 00c4a16d
        fld dword ptr [esp + 014ch] // 00c4a171
        fsub dword ptr [esp + 011ch] // 00c4a178
        fstp dword ptr [esp + 060h] // 00c4a17f
        fld dword ptr [esp + 058h] // 00c4a183
        fld st(0) // 00c4a187
        fmulp st(4), st(0) // 00c4a189
        fld dword ptr [esp + 05ch] // 00c4a18b
        fld st(0) // 00c4a18f
        fmulp st(6), st(0) // 00c4a191
        fxch st(4) // 00c4a193
        faddp st(5), st(0) // 00c4a195
        fld dword ptr [esp + 060h] // 00c4a197
        fld st(0) // 00c4a19b
        fmulp st(3), st(0) // 00c4a19d
        fxch st(5) // 00c4a19f
        faddp st(2), st(0) // 00c4a1a1
        fxch st(1) // 00c4a1a3
        fstp dword ptr [esp + 048h] // 00c4a1a5
        fld st(0) // 00c4a1a9
        fmul dword ptr [esp + 0fch] // 00c4a1ab
        fld st(3) // 00c4a1b2
        fmul dword ptr [esp + 0100h] // 00c4a1b4
        faddp st(1), st(0) // 00c4a1bb
        fld st(4) // 00c4a1bd
        fmul dword ptr [esp + 0104h] // 00c4a1bf
        faddp st(1), st(0) // 00c4a1c6
        fstp dword ptr [esp + 04ch] // 00c4a1c8
        fld st(0) // 00c4a1cc
        fmul dword ptr [esp + 0108h] // 00c4a1ce
        fld st(3) // 00c4a1d5
        fmul dword ptr [esp + 010ch] // 00c4a1d7
        faddp st(1), st(0) // 00c4a1de
        fld st(4) // 00c4a1e0
        fmul dword ptr [esp + 0110h] // 00c4a1e2
        faddp st(1), st(0) // 00c4a1e9
        fstp dword ptr [esp + 050h] // 00c4a1eb
        fld st(0) // 00c4a1ef
        fmulp st(5), st(0) // 00c4a1f1
        fld st(2) // 00c4a1f3
        fmulp st(2), st(0) // 00c4a1f5
        fxch st(4) // 00c4a1f7
        faddp st(1), st(0) // 00c4a1f9
        fld dword ptr [esp + 0128h] // 00c4a1fb
        fmul st(0), st(3) // 00c4a202
        faddp st(1), st(0) // 00c4a204
        fstp dword ptr [esp + 070h] // 00c4a206
        fld st(2) // 00c4a20a
        fmul dword ptr [esp + 012ch] // 00c4a20c
        fld dword ptr [esp + 0130h] // 00c4a213
        fmul st(0), st(2) // 00c4a21a
        faddp st(1), st(0) // 00c4a21c
        fld st(2) // 00c4a21e
        fmul dword ptr [esp + 0134h] // 00c4a220
        faddp st(1), st(0) // 00c4a227
        fstp dword ptr [esp + 074h] // 00c4a229
        fld dword ptr [esp + 0138h] // 00c4a22d
        fmulp st(3), st(0) // 00c4a234
        fmul dword ptr [esp + 013ch] // 00c4a236
        faddp st(2), st(0) // 00c4a23d
        fmul dword ptr [esp + 0140h] // 00c4a23f
        faddp st(1), st(0) // 00c4a246
        fstp dword ptr [esp + 078h] // 00c4a248
        _emit 141 // 00c4a24c native alignment lea esp, [esp]
        _emit 100 // 00c4a24c native alignment lea esp, [esp]
        _emit 36 // 00c4a24c native alignment lea esp, [esp]
        _emit 0 // 00c4a24c native alignment lea esp, [esp]
    l_00c4a250:
        fld dword ptr [esp + ebx*4 + 048h] // 00c4a250
        push ecx // 00c4a254
        fstp dword ptr [esp + 01ch] // 00c4a255
        fld dword ptr [esp + 01ch] // 00c4a259
        fstp dword ptr [esp] // 00c4a25d
        call abs_kernel // 00c4a260
        fstp dword ptr [esp + 080h] // 00c4a265
        push ecx // 00c4a26c
        fld dword ptr [esp + ebx*4 + 0224h] // 00c4a26d
        fstp dword ptr [esp] // 00c4a274
        call abs_kernel // 00c4a277
        fmul dword ptr [esp + 0160h] // 00c4a27c
        push ecx // 00c4a283
        fstp qword ptr [esp + 02ch] // 00c4a284
        fld dword ptr [esp + ebx*4 + 0218h] // 00c4a288
        fstp dword ptr [esp] // 00c4a28f
        call abs_kernel // 00c4a292
        fmul dword ptr [esp + 015ch] // 00c4a297
        push ecx // 00c4a29e
        fadd qword ptr [esp + 02ch] // 00c4a29f
        fstp qword ptr [esp + 02ch] // 00c4a2a3
        fld dword ptr [esp + ebx*4 + 0230h] // 00c4a2a7
        fstp dword ptr [esp] // 00c4a2ae
        call abs_kernel // 00c4a2b1
        fmul dword ptr [esp + 0164h] // 00c4a2b6
        fadd qword ptr [esp + 028h] // 00c4a2bd
        fstp dword ptr [esp + 028h] // 00c4a2c1
        fld dword ptr [esp + 028h] // 00c4a2c5
        fadd dword ptr [esp + ebx*4 + 0150h] // 00c4a2c9
        fsubr dword ptr [esp + 080h] // 00c4a2d0
        fstp dword ptr [esp + 014h] // 00c4a2d7
        fldz  // 00c4a2db
        fld dword ptr [esp + 014h] // 00c4a2dd
        fcomi st(0), st(1) // 00c4a2e1
        fstp st(1) // 00c4a2e3
        ja l_00c4a4d2 // 00c4a2e5
        fld dword ptr [esp + 054h] // 00c4a2eb
        mov eax, dword ptr [esp + 010h] // 00c4a2ef
        fxch st(1) // 00c4a2f3
        fcomip st(0), st(1) // 00c4a2f5
        fstp st(0) // 00c4a2f7
        jbe l_00c4a376 // 00c4a2f9
        movss xmm0, dword ptr [esp + 014h] // 00c4a2fb
        mov dword ptr [esp + 016ch], ebx // 00c4a301
        mov ecx, dword ptr [eax] // 00c4a308
        mov edx, dword ptr [eax + 4] // 00c4a30a
        movss dword ptr [esp + 054h], xmm0 // 00c4a30d
        xorps xmm0, xmm0 // 00c4a313
        comiss xmm0, dword ptr [esp + 018h] // 00c4a316
        mov dword ptr [esp + 064h], ecx // 00c4a31b
        mov ecx, dword ptr [eax + 8] // 00c4a31f
        mov dword ptr [esp + 068h], edx // 00c4a322
        mov dword ptr [esp + 06ch], ecx // 00c4a326
        jbe l_00c4a376 // 00c4a32a
        movss xmm0, dword ptr constant_00d7a208 // 00c4a32c
        movaps xmm1, xmm0 // 00c4a334
        subss xmm1, dword ptr [esp + 064h] // 00c4a337
        movss dword ptr [esp + 01ch], xmm1 // 00c4a33d
        mov edx, dword ptr [esp + 01ch] // 00c4a343
        movaps xmm1, xmm0 // 00c4a347
        subss xmm1, dword ptr [esp + 068h] // 00c4a34a
        subss xmm0, dword ptr [esp + 06ch] // 00c4a350
        movss dword ptr [esp + 020h], xmm1 // 00c4a356
        mov ecx, dword ptr [esp + 020h] // 00c4a35c
        movss dword ptr [esp + 024h], xmm0 // 00c4a360
        mov dword ptr [esp + 064h], edx // 00c4a366
        mov edx, dword ptr [esp + 024h] // 00c4a36a
        mov dword ptr [esp + 068h], ecx // 00c4a36e
        mov dword ptr [esp + 06ch], edx // 00c4a372
    l_00c4a376:
        add ebx, 1 // 00c4a376
        add eax, 0ch // 00c4a379
        cmp ebx, 3 // 00c4a37c
        mov dword ptr [esp + 010h], eax // 00c4a37f
        jl l_00c4a250 // 00c4a383
        xor ebx, ebx // 00c4a389
        mov dword ptr [esp + 010h], ebx // 00c4a38b
    l_00c4a38f:
        mov eax, dword ptr [esp + 010h] // 00c4a38f
        fld dword ptr [esp + eax*4 + 070h] // 00c4a393
        push ecx // 00c4a397
        fstp dword ptr [esp + 018h] // 00c4a398
        fld dword ptr [esp + 018h] // 00c4a39c
        fstp dword ptr [esp] // 00c4a3a0
        call abs_kernel // 00c4a3a3
        fstp dword ptr [esp + 080h] // 00c4a3a8
        push ecx // 00c4a3af
        fld dword ptr [esp + ebx + 021ch] // 00c4a3b0
        fstp dword ptr [esp] // 00c4a3b7
        call abs_kernel // 00c4a3ba
        fmul dword ptr [esp + 0154h] // 00c4a3bf
        push ecx // 00c4a3c6
        fstp qword ptr [esp + 02ch] // 00c4a3c7
        fld dword ptr [esp + ebx + 0218h] // 00c4a3cb
        fstp dword ptr [esp] // 00c4a3d2
        call abs_kernel // 00c4a3d5
        fmul dword ptr [esp + 0150h] // 00c4a3da
        push ecx // 00c4a3e1
        fadd qword ptr [esp + 02ch] // 00c4a3e2
        fstp qword ptr [esp + 02ch] // 00c4a3e6
        fld dword ptr [esp + ebx + 0220h] // 00c4a3ea
        fstp dword ptr [esp] // 00c4a3f1
        call abs_kernel // 00c4a3f4
        fmul dword ptr [esp + 0158h] // 00c4a3f9
        mov ecx, dword ptr [esp + 010h] // 00c4a400
        fadd qword ptr [esp + 028h] // 00c4a404
        fstp dword ptr [esp + 028h] // 00c4a408
        fld dword ptr [esp + 028h] // 00c4a40c
        fadd dword ptr [esp + ecx*4 + 015ch] // 00c4a410
        fsubr dword ptr [esp + 080h] // 00c4a417
        fstp dword ptr [esp + 010h] // 00c4a41e
        fldz  // 00c4a422
        fld dword ptr [esp + 010h] // 00c4a424
        fcomi st(0), st(1) // 00c4a428
        fstp st(1) // 00c4a42a
        ja l_00c4a4d2 // 00c4a42c
        fld dword ptr [esp + 054h] // 00c4a432
        fxch st(1) // 00c4a436
        fcomip st(0), st(1) // 00c4a438
        fstp st(0) // 00c4a43a
        jbe l_00c4a4df // 00c4a43c
        movss xmm0, dword ptr [esp + 010h] // 00c4a442
        lea edx, [ecx + 3] // 00c4a448
        mov dword ptr [esp + 016ch], edx // 00c4a44b
        mov eax, dword ptr [esp + ebx + 0120h] // 00c4a452
        mov edx, dword ptr [esp + ebx + 0124h] // 00c4a459
        movss dword ptr [esp + 054h], xmm0 // 00c4a460
        xorps xmm0, xmm0 // 00c4a466
        comiss xmm0, dword ptr [esp + 014h] // 00c4a469
        movss xmm0, dword ptr constant_00d7a208 // 00c4a46e
        mov dword ptr [esp + 064h], eax // 00c4a476
        mov eax, dword ptr [esp + ebx + 0128h] // 00c4a47a
        mov dword ptr [esp + 068h], edx // 00c4a481
        mov dword ptr [esp + 06ch], eax // 00c4a485
        jbe l_00c4a4e7 // 00c4a489
        movaps xmm1, xmm0 // 00c4a48b
        subss xmm1, dword ptr [esp + 064h] // 00c4a48e
        movss dword ptr [esp + 01ch], xmm1 // 00c4a494
        mov edx, dword ptr [esp + 01ch] // 00c4a49a
        movaps xmm1, xmm0 // 00c4a49e
        subss xmm1, dword ptr [esp + 068h] // 00c4a4a1
        movss dword ptr [esp + 020h], xmm1 // 00c4a4a7
        mov eax, dword ptr [esp + 020h] // 00c4a4ad
        movaps xmm1, xmm0 // 00c4a4b1
        subss xmm1, dword ptr [esp + 06ch] // 00c4a4b4
        movss dword ptr [esp + 024h], xmm1 // 00c4a4ba
        mov dword ptr [esp + 064h], edx // 00c4a4c0
        mov edx, dword ptr [esp + 024h] // 00c4a4c4
        mov dword ptr [esp + 068h], eax // 00c4a4c8
        mov dword ptr [esp + 06ch], edx // 00c4a4cc
        jmp l_00c4a4e7 // 00c4a4d0
    l_00c4a4d2:
        fstp st(0) // 00c4a4d2
    l_00c4a4d4:
        xor al, al // 00c4a4d4
        pop edi // 00c4a4d6
        pop esi // 00c4a4d7
        pop ebx // 00c4a4d8
        mov esp, ebp // 00c4a4d9
        pop ebp // 00c4a4db
        ret 24 // 00c4a4dc
    l_00c4a4df:
        movss xmm0, dword ptr constant_00d7a208 // 00c4a4df
    l_00c4a4e7:
        add ecx, 1 // 00c4a4e7
        add ebx, 0ch // 00c4a4ea
        cmp ebx, 024h // 00c4a4ed
        mov dword ptr [esp + 010h], ecx // 00c4a4f0
        jl l_00c4a38f // 00c4a4f4
        fld dword ptr [esp + 054h] // 00c4a4fa
        lea edx, [esp + 0f4h] // 00c4a4fe
        fadd qword ptr constant_00d7a318 // 00c4a505
        mov dword ptr [esp + 018h], 6 // 00c4a50b
        mov dword ptr [esp + 014h], edx // 00c4a513
        fstp dword ptr [esp + 054h] // 00c4a517
    l_00c4a51b:
        lea ecx, [esp + 0124h] // 00c4a51b
        xor ebx, ebx // 00c4a522
        mov dword ptr [esp + 010h], ecx // 00c4a524
        jmp l_00c4a530 // 00c4a528
        _emit 141 // 00c4a52a native alignment lea ebx, [ebx]
        _emit 155 // 00c4a52a native alignment lea ebx, [ebx]
        _emit 0 // 00c4a52a native alignment lea ebx, [ebx]
        _emit 0 // 00c4a52a native alignment lea ebx, [ebx]
        _emit 0 // 00c4a52a native alignment lea ebx, [ebx]
        _emit 0 // 00c4a52a native alignment lea ebx, [ebx]
    l_00c4a530:
        fld dword ptr [ecx + 4] // 00c4a530
        fstp dword ptr [esp + 080h] // 00c4a533
        fld dword ptr [edx] // 00c4a53a
        fstp dword ptr [esp + 028h] // 00c4a53c
        fld dword ptr [ecx] // 00c4a540
        fstp dword ptr [esp + 044h] // 00c4a542
        fld dword ptr [edx + 4] // 00c4a546
        fstp dword ptr [esp + 03ch] // 00c4a549
        fld dword ptr [esp + 028h] // 00c4a54d
        fld st(0) // 00c4a551
        fld dword ptr [esp + 080h] // 00c4a553
        fld st(0) // 00c4a55a
        fmulp st(2), st(0) // 00c4a55c
        fld dword ptr [esp + 03ch] // 00c4a55e
        fld st(0) // 00c4a562
        fld dword ptr [esp + 044h] // 00c4a564
        fld st(0) // 00c4a568
        fmulp st(2), st(0) // 00c4a56a
        fxch st(4) // 00c4a56c
        fsubrp st(1), st(0) // 00c4a56e
        fstp dword ptr [esp + 048h] // 00c4a570
        fld dword ptr [ecx - 4] // 00c4a574
        fstp dword ptr [esp + 028h] // 00c4a577
        fld dword ptr [edx - 4] // 00c4a57b
        fstp dword ptr [esp + 080h] // 00c4a57e
        fld dword ptr [esp + 028h] // 00c4a585
        fld st(0) // 00c4a589
        fmulp st(2), st(0) // 00c4a58b
        fld dword ptr [esp + 080h] // 00c4a58d
        fld st(0) // 00c4a594
        fmulp st(4), st(0) // 00c4a596
        fxch st(2) // 00c4a598
        fsubrp st(3), st(0) // 00c4a59a
        fxch st(2) // 00c4a59c
        fstp dword ptr [esp + 04ch] // 00c4a59e
        fmulp st(2), st(0) // 00c4a5a2
        fmulp st(2), st(0) // 00c4a5a4
        fsubrp st(1), st(0) // 00c4a5a6
        fstp dword ptr [esp + 050h] // 00c4a5a8
        fld dword ptr [esp + 048h] // 00c4a5ac
        fld st(0) // 00c4a5b0
        fld dword ptr [esp + 04ch] // 00c4a5b2
        fld st(0) // 00c4a5b6
        fld dword ptr [esp + 050h] // 00c4a5b8
        fld st(0) // 00c4a5bc
        fld st(2) // 00c4a5be
        fmulp st(3), st(0) // 00c4a5c0
        fld st(4) // 00c4a5c2
        fmulp st(5), st(0) // 00c4a5c4
        fxch st(2) // 00c4a5c6
        faddp st(4), st(0) // 00c4a5c8
        fld st(1) // 00c4a5ca
        fmulp st(2), st(0) // 00c4a5cc
        fxch st(3) // 00c4a5ce
        faddp st(1), st(0) // 00c4a5d0
        fstp dword ptr [esp + 03ch] // 00c4a5d2
        fld dword ptr [esp + 03ch] // 00c4a5d6
        fld dword ptr constant_00d7a310 // 00c4a5da
        fcomip st(0), st(1) // 00c4a5e0
        fstp st(0) // 00c4a5e2
        ja l_00c4a8d6 // 00c4a5e4
        fmul dword ptr [esp + 05ch] // 00c4a5ea
        push ecx // 00c4a5ee
        fld dword ptr [esp + 05ch] // 00c4a5ef
        fmulp st(3), st(0) // 00c4a5f3
        faddp st(2), st(0) // 00c4a5f5
        fmul dword ptr [esp + 064h] // 00c4a5f7
        faddp st(1), st(0) // 00c4a5fb
        fstp dword ptr [esp + 048h] // 00c4a5fd
        fld dword ptr [esp + 048h] // 00c4a601
        fstp dword ptr [esp] // 00c4a605
        call abs_kernel // 00c4a608
        fstp dword ptr [esp + 040h] // 00c4a60d
        push ecx // 00c4a611
        fld dword ptr [esp + 04ch] // 00c4a612
        fmul dword ptr [esp + 0f4h] // 00c4a616
        fld dword ptr [esp + 050h] // 00c4a61d
        fmul dword ptr [esp + 0f8h] // 00c4a621
        faddp st(1), st(0) // 00c4a628
        fld dword ptr [esp + 054h] // 00c4a62a
        fmul dword ptr [esp + 0fch] // 00c4a62e
        faddp st(1), st(0) // 00c4a635
        fstp dword ptr [esp + 02ch] // 00c4a637
        fld dword ptr [esp + 02ch] // 00c4a63b
        fstp dword ptr [esp] // 00c4a63f
        call abs_kernel // 00c4a642
        fmul dword ptr [esp + 0150h] // 00c4a647
        push ecx // 00c4a64e
        fstp qword ptr [esp + 084h] // 00c4a64f
        fld dword ptr [esp + 04ch] // 00c4a656
        fmul dword ptr [esp + 0100h] // 00c4a65a
        fld dword ptr [esp + 050h] // 00c4a661
        fmul dword ptr [esp + 0104h] // 00c4a665
        faddp st(1), st(0) // 00c4a66c
        fld dword ptr [esp + 054h] // 00c4a66e
        fmul dword ptr [esp + 0108h] // 00c4a672
        faddp st(1), st(0) // 00c4a679
        fstp dword ptr [esp + 02ch] // 00c4a67b
        fld dword ptr [esp + 02ch] // 00c4a67f
        fstp dword ptr [esp] // 00c4a683
        call abs_kernel // 00c4a686
        fmul dword ptr [esp + 0154h] // 00c4a68b
        push ecx // 00c4a692
        fadd qword ptr [esp + 084h] // 00c4a693
        fstp qword ptr [esp + 084h] // 00c4a69a
        fld dword ptr [esp + 04ch] // 00c4a6a1
        fmul dword ptr [esp + 010ch] // 00c4a6a5
        fld dword ptr [esp + 050h] // 00c4a6ac
        fmul dword ptr [esp + 0110h] // 00c4a6b0
        faddp st(1), st(0) // 00c4a6b7
        fld dword ptr [esp + 054h] // 00c4a6b9
        fmul dword ptr [esp + 0114h] // 00c4a6bd
        faddp st(1), st(0) // 00c4a6c4
        fstp dword ptr [esp + 02ch] // 00c4a6c6
        fld dword ptr [esp + 02ch] // 00c4a6ca
        fstp dword ptr [esp] // 00c4a6ce
        call abs_kernel // 00c4a6d1
        fmul dword ptr [esp + 0158h] // 00c4a6d6
        push ecx // 00c4a6dd
        fadd qword ptr [esp + 084h] // 00c4a6de
        fstp dword ptr [esp + 02ch] // 00c4a6e5
        fld dword ptr [esp + 02ch] // 00c4a6e9
        fstp dword ptr [esp + 084h] // 00c4a6ed
        fld dword ptr [esp + 04ch] // 00c4a6f4
        fmul dword ptr [esp + 0130h] // 00c4a6f8
        fld dword ptr [esp + 0134h] // 00c4a6ff
        fmul dword ptr [esp + 050h] // 00c4a706
        faddp st(1), st(0) // 00c4a70a
        fld dword ptr [esp + 054h] // 00c4a70c
        fmul dword ptr [esp + 0138h] // 00c4a710
        faddp st(1), st(0) // 00c4a717
        fstp dword ptr [esp + 02ch] // 00c4a719
        fld dword ptr [esp + 02ch] // 00c4a71d
        fstp dword ptr [esp] // 00c4a721
        call abs_kernel // 00c4a724
        fmul dword ptr [esp + 0160h] // 00c4a729
        fstp qword ptr [esp + 070h] // 00c4a730
        fld dword ptr [esp + 048h] // 00c4a734
        fmul dword ptr [esp + 0120h] // 00c4a738
        fld dword ptr [esp + 0124h] // 00c4a73f
        fmul dword ptr [esp + 04ch] // 00c4a746
        faddp st(1), st(0) // 00c4a74a
        fld dword ptr [esp + 0128h] // 00c4a74c
        push ecx // 00c4a753
        fmul dword ptr [esp + 054h] // 00c4a754
        faddp st(1), st(0) // 00c4a758
        fstp dword ptr [esp + 02ch] // 00c4a75a
        fld dword ptr [esp + 02ch] // 00c4a75e
        fstp dword ptr [esp] // 00c4a762
        call abs_kernel // 00c4a765
        fmul dword ptr [esp + 015ch] // 00c4a76a
        push ecx // 00c4a771
        fadd qword ptr [esp + 074h] // 00c4a772
        fstp qword ptr [esp + 074h] // 00c4a776
        fld dword ptr [esp + 04ch] // 00c4a77a
        fmul dword ptr [esp + 013ch] // 00c4a77e
        fld dword ptr [esp + 050h] // 00c4a785
        fmul dword ptr [esp + 0140h] // 00c4a789
        faddp st(1), st(0) // 00c4a790
        fld dword ptr [esp + 054h] // 00c4a792
        fmul dword ptr [esp + 0144h] // 00c4a796
        faddp st(1), st(0) // 00c4a79d
        fstp dword ptr [esp + 02ch] // 00c4a79f
        fld dword ptr [esp + 02ch] // 00c4a7a3
        fstp dword ptr [esp] // 00c4a7a7
        call abs_kernel // 00c4a7aa
        fmul dword ptr [esp + 0164h] // 00c4a7af
        fadd qword ptr [esp + 070h] // 00c4a7b6
        fstp dword ptr [esp + 028h] // 00c4a7ba
        fld dword ptr [esp + 028h] // 00c4a7be
        fadd dword ptr [esp + 080h] // 00c4a7c2
        fsubr dword ptr [esp + 040h] // 00c4a7c9
        fstp dword ptr [esp + 040h] // 00c4a7cd
        fldz  // 00c4a7d1
        fld dword ptr [esp + 040h] // 00c4a7d3
        fcomip st(0), st(1) // 00c4a7d7
        fstp st(0) // 00c4a7d9
        ja l_00c4a4d4 // 00c4a7db
        fld dword ptr [esp + 03ch] // 00c4a7e1
        push ecx // 00c4a7e5
        fstp dword ptr [esp] // 00c4a7e6
        push dword ptr [ebp+1ch] // Explicit borrowed CRT context.
        call sqrt_kernel // 00c4a7e9
        fstp dword ptr [esp + 03ch] // 00c4a7ee
        fld dword ptr [esp + 040h] // 00c4a7f2
        fld dword ptr [esp + 03ch] // 00c4a7f6
        fld st(0) // 00c4a7fa
        fdivp st(2), st(0) // 00c4a7fc
        fxch st(1) // 00c4a7fe
        fstp dword ptr [esp + 040h] // 00c4a800
        fld dword ptr [esp + 054h] // 00c4a804
        fld dword ptr [esp + 040h] // 00c4a808
        fcomip st(0), st(1) // 00c4a80c
        fstp st(0) // 00c4a80e
        jbe l_00c4a8c4 // 00c4a810
        fld dword ptr [esp + 048h] // 00c4a816
        mov eax, dword ptr [esp + 048h] // 00c4a81a
        fdiv st(0), st(1) // 00c4a81e
        mov ecx, dword ptr [esp + 04ch] // 00c4a820
        mov dword ptr [esp + 064h], eax // 00c4a824
        mov dword ptr [esp + 068h], ecx // 00c4a828
        mov edx, dword ptr [esp + 050h] // 00c4a82c
        mov dword ptr [esp + 06ch], edx // 00c4a830
        movss xmm0, dword ptr [esp + 040h] // 00c4a834
        movss dword ptr [esp + 054h], xmm0 // 00c4a83a
        xorps xmm0, xmm0 // 00c4a840
        comiss xmm0, dword ptr [esp + 044h] // 00c4a843
        movss xmm0, dword ptr constant_00d7a208 // 00c4a848
        fstp dword ptr [esp + 064h] // 00c4a850
        fld dword ptr [esp + 068h] // 00c4a854
        fdiv st(0), st(1) // 00c4a858
        fstp dword ptr [esp + 068h] // 00c4a85a
        fdivr dword ptr [esp + 06ch] // 00c4a85e
        fstp dword ptr [esp + 06ch] // 00c4a862
        jbe l_00c4a8ad // 00c4a866
        movaps xmm1, xmm0 // 00c4a868
        subss xmm1, dword ptr [esp + 064h] // 00c4a86b
        movss dword ptr [esp + 01ch], xmm1 // 00c4a871
        mov eax, dword ptr [esp + 01ch] // 00c4a877
        movaps xmm1, xmm0 // 00c4a87b
        subss xmm1, dword ptr [esp + 068h] // 00c4a87e
        movss dword ptr [esp + 020h], xmm1 // 00c4a884
        mov ecx, dword ptr [esp + 020h] // 00c4a88a
        movaps xmm1, xmm0 // 00c4a88e
        subss xmm1, dword ptr [esp + 06ch] // 00c4a891
        movss dword ptr [esp + 024h], xmm1 // 00c4a897
        mov edx, dword ptr [esp + 024h] // 00c4a89d
        mov dword ptr [esp + 064h], eax // 00c4a8a1
        mov dword ptr [esp + 068h], ecx // 00c4a8a5
        mov dword ptr [esp + 06ch], edx // 00c4a8a9
    l_00c4a8ad:
        mov eax, dword ptr [esp + 018h] // 00c4a8ad
        mov edx, dword ptr [esp + 014h] // 00c4a8b1
        mov ecx, dword ptr [esp + 010h] // 00c4a8b5
        add eax, ebx // 00c4a8b9
        mov dword ptr [esp + 016ch], eax // 00c4a8bb
        jmp l_00c4a8dc // 00c4a8c2
    l_00c4a8c4:
        movss xmm0, dword ptr constant_00d7a208 // 00c4a8c4
        mov edx, dword ptr [esp + 014h] // 00c4a8cc
        mov ecx, dword ptr [esp + 010h] // 00c4a8d0
        jmp l_00c4a8da // 00c4a8d4
    l_00c4a8d6:
        fstp st(2) // 00c4a8d6
        fstp st(1) // 00c4a8d8
    l_00c4a8da:
        fstp st(0) // 00c4a8da
    l_00c4a8dc:
        add ebx, 1 // 00c4a8dc
        add ecx, 0ch // 00c4a8df
        cmp ebx, 3 // 00c4a8e2
        mov dword ptr [esp + 010h], ecx // 00c4a8e5
        jl l_00c4a530 // 00c4a8e9
        mov eax, dword ptr [esp + 018h] // 00c4a8ef
        add eax, 3 // 00c4a8f3
        add edx, 0ch // 00c4a8f6
        cmp eax, 0fh // 00c4a8f9
        mov dword ptr [esp + 014h], edx // 00c4a8fc
        mov dword ptr [esp + 018h], eax // 00c4a900
        jl l_00c4a51b // 00c4a904
        mov ecx, dword ptr [esp + 016ch] // 00c4a90a
        cmp ecx, 5 // 00c4a911
        subss xmm0, dword ptr [esp + 054h] // 00c4a914
        movss dword ptr [esp + 054h], xmm0 // 00c4a91a
        jle l_00c4aff3 // 00c4a920
        fld dword ptr [esp + 064h] // 00c4a926
        movss xmm0, dword ptr constant_00d7a260 // 00c4a92a
        fld st(0) // 00c4a932
        movss xmm1, dword ptr constant_00d7a24c // 00c4a934
        fld dword ptr [esp + 0108h] // 00c4a93c
        fld st(0) // 00c4a943
        fmulp st(2), st(0) // 00c4a945
        fld dword ptr [esp + 068h] // 00c4a947
        fld st(0) // 00c4a94b
        fld dword ptr [esp + 010ch] // 00c4a94d
        fld st(0) // 00c4a954
        fmulp st(2), st(0) // 00c4a956
        fxch st(4) // 00c4a958
        faddp st(1), st(0) // 00c4a95a
        fld dword ptr [esp + 06ch] // 00c4a95c
        fld st(0) // 00c4a960
        fmul dword ptr [esp + 0110h] // 00c4a962
        faddp st(2), st(0) // 00c4a969
        fxch st(1) // 00c4a96b
        fstp dword ptr [esp + 028h] // 00c4a96d
        fldz  // 00c4a971
        fld dword ptr [esp + 028h] // 00c4a973
        fcomip st(0), st(1) // 00c4a977
        fstp st(0) // 00c4a979
        jbe l_00c4a985 // 00c4a97b
        movss dword ptr [esp + 010h], xmm1 // 00c4a97d
        jmp l_00c4a98b // 00c4a983
    l_00c4a985:
        movss dword ptr [esp + 010h], xmm0 // 00c4a985
    l_00c4a98b:
        fld dword ptr [esp + 010h] // 00c4a98b
        fmul dword ptr [esp + 0158h] // 00c4a98f
        fstp dword ptr [esp + 028h] // 00c4a996
        fld dword ptr [esp + 028h] // 00c4a99a
        fld st(0) // 00c4a99e
        fmulp st(4), st(0) // 00c4a9a0
        fxch st(3) // 00c4a9a2
        fstp dword ptr [esp + 070h] // 00c4a9a4
        fld st(2) // 00c4a9a8
        fmulp st(4), st(0) // 00c4a9aa
        fxch st(3) // 00c4a9ac
        fstp dword ptr [esp + 074h] // 00c4a9ae
        fld dword ptr [esp + 0110h] // 00c4a9b2
        fmulp st(2), st(0) // 00c4a9b9
        fxch st(1) // 00c4a9bb
        fstp dword ptr [esp + 078h] // 00c4a9bd
        fld st(2) // 00c4a9c1
        fld dword ptr [esp + 0fch] // 00c4a9c3
        fld st(0) // 00c4a9ca
        fmulp st(2), st(0) // 00c4a9cc
        fld st(2) // 00c4a9ce
        fld dword ptr [esp + 0100h] // 00c4a9d0
        fld st(0) // 00c4a9d7
        fmulp st(2), st(0) // 00c4a9d9
        fxch st(3) // 00c4a9db
        faddp st(1), st(0) // 00c4a9dd
        fld st(4) // 00c4a9df
        fmul dword ptr [esp + 0104h] // 00c4a9e1
        faddp st(1), st(0) // 00c4a9e8
        fstp dword ptr [esp + 028h] // 00c4a9ea
        fldz  // 00c4a9ee
        fld dword ptr [esp + 028h] // 00c4a9f0
        fcomip st(0), st(1) // 00c4a9f4
        fstp st(0) // 00c4a9f6
        jbe l_00c4aa02 // 00c4a9f8
        movss dword ptr [esp + 010h], xmm1 // 00c4a9fa
        jmp l_00c4aa08 // 00c4aa00
    l_00c4aa02:
        movss dword ptr [esp + 010h], xmm0 // 00c4aa02
    l_00c4aa08:
        fld dword ptr [esp + 010h] // 00c4aa08
        fmul dword ptr [esp + 0154h] // 00c4aa0c
        fstp dword ptr [esp + 028h] // 00c4aa13
        fld dword ptr [esp + 028h] // 00c4aa17
        fld st(0) // 00c4aa1b
        fmulp st(2), st(0) // 00c4aa1d
        fxch st(1) // 00c4aa1f
        fstp dword ptr [esp + 048h] // 00c4aa21
        fld st(0) // 00c4aa25
        fmulp st(2), st(0) // 00c4aa27
        fxch st(1) // 00c4aa29
        fstp dword ptr [esp + 04ch] // 00c4aa2b
        fmul dword ptr [esp + 0104h] // 00c4aa2f
        fstp dword ptr [esp + 050h] // 00c4aa36
        fld st(2) // 00c4aa3a
        fld dword ptr [esp + 0f0h] // 00c4aa3c
        fld st(0) // 00c4aa43
        fmulp st(2), st(0) // 00c4aa45
        fld st(2) // 00c4aa47
        fld dword ptr [esp + 0f4h] // 00c4aa49
        fld st(0) // 00c4aa50
        fmulp st(2), st(0) // 00c4aa52
        fxch st(3) // 00c4aa54
        faddp st(1), st(0) // 00c4aa56
        fld st(4) // 00c4aa58
        fmul dword ptr [esp + 0f8h] // 00c4aa5a
        faddp st(1), st(0) // 00c4aa61
        fstp dword ptr [esp + 028h] // 00c4aa63
        fldz  // 00c4aa67
        fld dword ptr [esp + 028h] // 00c4aa69
        fcomip st(0), st(1) // 00c4aa6d
        jbe l_00c4aa79 // 00c4aa6f
        movss dword ptr [esp + 010h], xmm1 // 00c4aa71
        jmp l_00c4aa7f // 00c4aa77
    l_00c4aa79:
        movss dword ptr [esp + 010h], xmm0 // 00c4aa79
    l_00c4aa7f:
        fld dword ptr [esp + 010h] // 00c4aa7f
        fmul dword ptr [esp + 0150h] // 00c4aa83
        fstp dword ptr [esp + 028h] // 00c4aa8a
        fld dword ptr [esp + 028h] // 00c4aa8e
        fld st(0) // 00c4aa92
        fmulp st(3), st(0) // 00c4aa94
        fxch st(2) // 00c4aa96
        fstp dword ptr [esp + 01ch] // 00c4aa98
        fld st(1) // 00c4aa9c
        fmulp st(3), st(0) // 00c4aa9e
        fxch st(2) // 00c4aaa0
        fstp dword ptr [esp + 020h] // 00c4aaa2
        fmul dword ptr [esp + 0f8h] // 00c4aaa6
        fstp dword ptr [esp + 024h] // 00c4aaad
        fld dword ptr [esp + 01ch] // 00c4aab1
        fadd dword ptr [esp + 0114h] // 00c4aab5
        fstp dword ptr [esp + 058h] // 00c4aabc
        fld dword ptr [esp + 020h] // 00c4aac0
        fadd dword ptr [esp + 0118h] // 00c4aac4
        fstp dword ptr [esp + 05ch] // 00c4aacb
        fld dword ptr [esp + 024h] // 00c4aacf
        fadd dword ptr [esp + 011ch] // 00c4aad3
        fstp dword ptr [esp + 060h] // 00c4aada
        fld dword ptr [esp + 058h] // 00c4aade
        fadd dword ptr [esp + 048h] // 00c4aae2
        fstp dword ptr [esp + 01ch] // 00c4aae6
        fld dword ptr [esp + 05ch] // 00c4aaea
        fadd dword ptr [esp + 04ch] // 00c4aaee
        fstp dword ptr [esp + 020h] // 00c4aaf2
        fld dword ptr [esp + 060h] // 00c4aaf6
        fadd dword ptr [esp + 050h] // 00c4aafa
        fstp dword ptr [esp + 024h] // 00c4aafe
        fld dword ptr [esp + 01ch] // 00c4ab02
        fadd dword ptr [esp + 070h] // 00c4ab06
        fstp dword ptr [esp + 048h] // 00c4ab0a
        fld dword ptr [esp + 020h] // 00c4ab0e
        fadd dword ptr [esp + 074h] // 00c4ab12
        fstp dword ptr [esp + 04ch] // 00c4ab16
        fld dword ptr [esp + 024h] // 00c4ab1a
        fadd dword ptr [esp + 078h] // 00c4ab1e
        fstp dword ptr [esp + 050h] // 00c4ab22
        fld st(3) // 00c4ab26
        fld dword ptr [esp + 0138h] // 00c4ab28
        fld st(0) // 00c4ab2f
        fmulp st(2), st(0) // 00c4ab31
        fld st(3) // 00c4ab33
        fmul dword ptr [esp + 013ch] // 00c4ab35
        faddp st(2), st(0) // 00c4ab3c
        fld st(4) // 00c4ab3e
        fmul dword ptr [esp + 0140h] // 00c4ab40
        faddp st(2), st(0) // 00c4ab47
        fxch st(1) // 00c4ab49
        fstp dword ptr [esp + 028h] // 00c4ab4b
        fld dword ptr [esp + 028h] // 00c4ab4f
        fcomip st(0), st(2) // 00c4ab53
        jbe l_00c4ab5f // 00c4ab55
        movss dword ptr [esp + 010h], xmm1 // 00c4ab57
        jmp l_00c4ab65 // 00c4ab5d
    l_00c4ab5f:
        movss dword ptr [esp + 010h], xmm0 // 00c4ab5f
    l_00c4ab65:
        fld dword ptr [esp + 010h] // 00c4ab65
        fmul dword ptr [esp + 0164h] // 00c4ab69
        fstp dword ptr [esp + 028h] // 00c4ab70
        fld dword ptr [esp + 028h] // 00c4ab74
        fld st(0) // 00c4ab78
        fmulp st(2), st(0) // 00c4ab7a
        fxch st(1) // 00c4ab7c
        fstp dword ptr [esp + 080h] // 00c4ab7e
        fld st(0) // 00c4ab85
        fmul dword ptr [esp + 013ch] // 00c4ab87
        fstp dword ptr [esp + 084h] // 00c4ab8e
        fmul dword ptr [esp + 0140h] // 00c4ab95
        fstp dword ptr [esp + 088h] // 00c4ab9c
        fld st(3) // 00c4aba3
        fld dword ptr [esp + 012ch] // 00c4aba5
        fld st(0) // 00c4abac
        fmulp st(2), st(0) // 00c4abae
        fld dword ptr [esp + 0130h] // 00c4abb0
        fmul st(0), st(4) // 00c4abb7
        faddp st(2), st(0) // 00c4abb9
        fld st(4) // 00c4abbb
        fmul dword ptr [esp + 0134h] // 00c4abbd
        faddp st(2), st(0) // 00c4abc4
        fxch st(1) // 00c4abc6
        fstp dword ptr [esp + 028h] // 00c4abc8
        fld dword ptr [esp + 028h] // 00c4abcc
        fcomip st(0), st(2) // 00c4abd0
        jbe l_00c4abdc // 00c4abd2
        movss dword ptr [esp + 010h], xmm1 // 00c4abd4
        jmp l_00c4abe2 // 00c4abda
    l_00c4abdc:
        movss dword ptr [esp + 010h], xmm0 // 00c4abdc
    l_00c4abe2:
        fld dword ptr [esp + 010h] // 00c4abe2
        fmul dword ptr [esp + 0160h] // 00c4abe6
        fstp dword ptr [esp + 028h] // 00c4abed
        fld dword ptr [esp + 028h] // 00c4abf1
        fld st(0) // 00c4abf5
        fmulp st(2), st(0) // 00c4abf7
        fxch st(1) // 00c4abf9
        fstp dword ptr [esp + 070h] // 00c4abfb
        fld st(0) // 00c4abff
        fmul dword ptr [esp + 0130h] // 00c4ac01
        fstp dword ptr [esp + 074h] // 00c4ac08
        fmul dword ptr [esp + 0134h] // 00c4ac0c
        fstp dword ptr [esp + 078h] // 00c4ac13
        fld dword ptr [esp + 0120h] // 00c4ac17
        fld st(0) // 00c4ac1e
        fmulp st(5), st(0) // 00c4ac20
        fld dword ptr [esp + 0124h] // 00c4ac22
        fld st(0) // 00c4ac29
        fmulp st(4), st(0) // 00c4ac2b
        fxch st(5) // 00c4ac2d
        faddp st(3), st(0) // 00c4ac2f
        fld dword ptr [esp + 0128h] // 00c4ac31
        fld st(0) // 00c4ac38
        fmulp st(5), st(0) // 00c4ac3a
        fxch st(3) // 00c4ac3c
        faddp st(4), st(0) // 00c4ac3e
        fxch st(3) // 00c4ac40
        fstp dword ptr [esp + 028h] // 00c4ac42
        fld dword ptr [esp + 028h] // 00c4ac46
        fcomip st(0), st(1) // 00c4ac4a
        fstp st(0) // 00c4ac4c
        jbe l_00c4ac58 // 00c4ac4e
        movss dword ptr [esp + 010h], xmm1 // 00c4ac50
        jmp l_00c4ac5e // 00c4ac56
    l_00c4ac58:
        movss dword ptr [esp + 010h], xmm0 // 00c4ac58
    l_00c4ac5e:
        fld dword ptr [esp + 010h] // 00c4ac5e
        lea eax, [ecx - 6] // 00c4ac62
        fmul dword ptr [esp + 015ch] // 00c4ac65
        cdq  // 00c4ac6c
        mov ecx, 3 // 00c4ac6d
        idiv ecx // 00c4ac72
        fstp dword ptr [esp + 028h] // 00c4ac74
        fld dword ptr [esp + 028h] // 00c4ac78
        fld st(0) // 00c4ac7c
        fmulp st(3), st(0) // 00c4ac7e
        fxch st(2) // 00c4ac80
        fstp dword ptr [esp + 01ch] // 00c4ac82
        fld st(1) // 00c4ac86
        fmulp st(3), st(0) // 00c4ac88
        fxch st(2) // 00c4ac8a
        fstp dword ptr [esp + 020h] // 00c4ac8c
        fmulp st(1), st(0) // 00c4ac90
        fstp dword ptr [esp + 024h] // 00c4ac92
        fld dword ptr [esp + 0144h] // 00c4ac96
        fsub dword ptr [esp + 01ch] // 00c4ac9d
        fstp dword ptr [esp + 058h] // 00c4aca1
        fld dword ptr [esp + 0148h] // 00c4aca5
        fsub dword ptr [esp + 020h] // 00c4acac
        fstp dword ptr [esp + 05ch] // 00c4acb0
        fld dword ptr [esp + 014ch] // 00c4acb4
        fsub dword ptr [esp + 024h] // 00c4acbb
        fstp dword ptr [esp + 060h] // 00c4acbf
        fld dword ptr [esp + 058h] // 00c4acc3
        fsub dword ptr [esp + 070h] // 00c4acc7
        fstp dword ptr [esp + 01ch] // 00c4accb
        fld dword ptr [esp + 05ch] // 00c4accf
        fsub dword ptr [esp + 074h] // 00c4acd3
        fstp dword ptr [esp + 020h] // 00c4acd7
        fld dword ptr [esp + 060h] // 00c4acdb
        fsub dword ptr [esp + 078h] // 00c4acdf
        fstp dword ptr [esp + 024h] // 00c4ace3
        lea eax, [eax + eax*2] // 00c4ace7
        fld dword ptr [esp + 01ch] // 00c4acea
        mov ecx, dword ptr [esp + eax*4 + 0f0h] // 00c4acee
        fsub dword ptr [esp + 080h] // 00c4acf5
        lea eax, [esp + eax*4 + 0f0h] // 00c4acfc
        mov dword ptr [esp + 028h], ecx // 00c4ad03
        mov ecx, dword ptr [eax + 4] // 00c4ad07
        fstp dword ptr [esp + 070h] // 00c4ad0a
        mov eax, dword ptr [eax + 8] // 00c4ad0e
        fld dword ptr [esp + 020h] // 00c4ad11
        mov dword ptr [esp + 030h], eax // 00c4ad15
        fsub dword ptr [esp + 084h] // 00c4ad19
        lea eax, [edx + edx*2] // 00c4ad20
        mov edx, dword ptr [esp + eax*4 + 0124h] // 00c4ad23
        lea eax, [esp + eax*4 + 0120h] // 00c4ad2a
        fstp dword ptr [esp + 074h] // 00c4ad31
        mov dword ptr [esp + 02ch], ecx // 00c4ad35
        fld dword ptr [esp + 024h] // 00c4ad39
        mov ecx, dword ptr [eax] // 00c4ad3d
        fsub dword ptr [esp + 088h] // 00c4ad3f
        mov eax, dword ptr [eax + 8] // 00c4ad46
        mov dword ptr [esp + 020h], edx // 00c4ad49
        mov dword ptr [esp + 01ch], ecx // 00c4ad4d
        fstp dword ptr [esp + 078h] // 00c4ad51
        mov dword ptr [esp + 024h], eax // 00c4ad55
        fld dword ptr [esp + 070h] // 00c4ad59
        fsub dword ptr [esp + 048h] // 00c4ad5d
        fstp dword ptr [esp + 058h] // 00c4ad61
        fld dword ptr [esp + 074h] // 00c4ad65
        fsub dword ptr [esp + 04ch] // 00c4ad69
        fstp dword ptr [esp + 05ch] // 00c4ad6d
        fld dword ptr [esp + 078h] // 00c4ad71
        fsub dword ptr [esp + 050h] // 00c4ad75
        fstp dword ptr [esp + 060h] // 00c4ad79
        fld dword ptr [esp + 02ch] // 00c4ad7d
        fld st(0) // 00c4ad81
        fld dword ptr [esp + 020h] // 00c4ad83
        fld st(0) // 00c4ad87
        fmulp st(2), st(0) // 00c4ad89
        fld dword ptr [esp + 01ch] // 00c4ad8b
        fld st(0) // 00c4ad8f
        fld dword ptr [esp + 028h] // 00c4ad91
        fld st(0) // 00c4ad95
        fmulp st(2), st(0) // 00c4ad97
        fxch st(4) // 00c4ad99
        faddp st(1), st(0) // 00c4ad9b
        fld dword ptr [esp + 030h] // 00c4ad9d
        fld st(0) // 00c4ada1
        fmul dword ptr [esp + 024h] // 00c4ada3
        faddp st(2), st(0) // 00c4ada7
        fxch st(1) // 00c4ada9
        fstp dword ptr [esp + 080h] // 00c4adab
        fld st(4) // 00c4adb2
        fmul dword ptr [esp + 05ch] // 00c4adb4
        fld dword ptr [esp + 058h] // 00c4adb8
        fmul st(0), st(5) // 00c4adbc
        faddp st(1), st(0) // 00c4adbe
        fld st(1) // 00c4adc0
        fmul dword ptr [esp + 060h] // 00c4adc2
        faddp st(1), st(0) // 00c4adc6
        fstp dword ptr [esp + 010h] // 00c4adc8
        fld st(2) // 00c4adcc
        fmul dword ptr [esp + 05ch] // 00c4adce
        fld dword ptr [esp + 058h] // 00c4add2
        fmul st(0), st(3) // 00c4add6
        faddp st(1), st(0) // 00c4add8
        fld dword ptr [esp + 024h] // 00c4adda
        fmul dword ptr [esp + 060h] // 00c4adde
        faddp st(1), st(0) // 00c4ade2
        fstp dword ptr [esp + 028h] // 00c4ade4
        fld dword ptr [esp + 028h] // 00c4ade8
        fchs  // 00c4adec
        fstp dword ptr [esp + 014h] // 00c4adee
        fld dword ptr [esp + 080h] // 00c4adf2
        fld st(0) // 00c4adf9
        fmul st(0), st(0) // 00c4adfb
        fld1  // 00c4adfd
        fsubrp st(1), st(0) // 00c4adff
        fstp dword ptr [esp + 040h] // 00c4ae01
        fld dword ptr [esp + 040h] // 00c4ae05
        fld qword ptr constant_00d7a268 // 00c4ae09
        fcomip st(0), st(1) // 00c4ae0f
        fstp st(0) // 00c4ae11
        jbe l_00c4ae28 // 00c4ae13
        xorps xmm0, xmm0 // 00c4ae15
        fstp st(0) // 00c4ae18
        movss dword ptr [esp + 018h], xmm0 // 00c4ae1a
        movss dword ptr [esp + 010h], xmm0 // 00c4ae20
        jmp l_00c4ae56 // 00c4ae26
    l_00c4ae28:
        fld dword ptr [esp + 040h] // 00c4ae28
        fld1  // 00c4ae2c
        fdivrp st(1), st(0) // 00c4ae2e
        fstp dword ptr [esp + 040h] // 00c4ae30
        fld dword ptr [esp + 014h] // 00c4ae34
        fmul st(0), st(1) // 00c4ae38
        fadd dword ptr [esp + 010h] // 00c4ae3a
        fmul dword ptr [esp + 040h] // 00c4ae3e
        fstp dword ptr [esp + 018h] // 00c4ae42
        fmul dword ptr [esp + 010h] // 00c4ae46
        fadd dword ptr [esp + 014h] // 00c4ae4a
        fmul dword ptr [esp + 040h] // 00c4ae4e
        fstp dword ptr [esp + 010h] // 00c4ae52
    l_00c4ae56:
        fld dword ptr [esp + 018h] // 00c4ae56
        mov eax, dword ptr [ebp + 8] // 00c4ae5a
        fld st(0) // 00c4ae5d
        mov ecx, dword ptr [esp + 064h] // 00c4ae5f
        fmulp st(5), st(0) // 00c4ae63
        mov edx, dword ptr [esp + 068h] // 00c4ae65
        fxch st(4) // 00c4ae69
        mov dword ptr [eax], 1 // 00c4ae6b
        mov dword ptr [eax + 01ch], ecx // 00c4ae71
        fstp dword ptr [esp + 058h] // 00c4ae74
        mov ecx, dword ptr [esp + 06ch] // 00c4ae78
        fld st(3) // 00c4ae7c
        mov dword ptr [eax + 020h], edx // 00c4ae7e
        fmulp st(5), st(0) // 00c4ae81
        mov dword ptr [eax + 024h], ecx // 00c4ae83
        fxch st(4) // 00c4ae86
        fstp dword ptr [esp + 05ch] // 00c4ae88
        fxch st(3) // 00c4ae8c
        fmulp st(2), st(0) // 00c4ae8e
        fxch st(1) // 00c4ae90
        fstp dword ptr [esp + 060h] // 00c4ae92
        fld dword ptr [esp + 048h] // 00c4ae96
        fadd dword ptr [esp + 058h] // 00c4ae9a
        fstp dword ptr [esp + 048h] // 00c4ae9e
        fld dword ptr [esp + 05ch] // 00c4aea2
        fadd dword ptr [esp + 04ch] // 00c4aea6
        fstp dword ptr [esp + 04ch] // 00c4aeaa
        fld dword ptr [esp + 060h] // 00c4aeae
        fadd dword ptr [esp + 050h] // 00c4aeb2
        fstp dword ptr [esp + 050h] // 00c4aeb6
        fld dword ptr [esp + 010h] // 00c4aeba
        fld st(0) // 00c4aebe
        fmulp st(3), st(0) // 00c4aec0
        fxch st(2) // 00c4aec2
        fstp dword ptr [esp + 058h] // 00c4aec4
        fmul st(0), st(1) // 00c4aec8
        fstp dword ptr [esp + 05ch] // 00c4aeca
        fmul dword ptr [esp + 024h] // 00c4aece
        fstp dword ptr [esp + 060h] // 00c4aed2
        fld dword ptr [esp + 058h] // 00c4aed6
        fadd dword ptr [esp + 070h] // 00c4aeda
        fstp dword ptr [esp + 070h] // 00c4aede
        fld dword ptr [esp + 05ch] // 00c4aee2
        fadd dword ptr [esp + 074h] // 00c4aee6
        fstp dword ptr [esp + 074h] // 00c4aeea
        fld dword ptr [esp + 060h] // 00c4aeee
        fadd dword ptr [esp + 078h] // 00c4aef2
        fstp dword ptr [esp + 078h] // 00c4aef6
        fld dword ptr [esp + 048h] // 00c4aefa
        fsub dword ptr [esi + 024h] // 00c4aefe
        fstp dword ptr [esp + 01ch] // 00c4af01
        fld dword ptr [esp + 04ch] // 00c4af05
        fsub dword ptr [esi + 028h] // 00c4af09
        fstp dword ptr [esp + 020h] // 00c4af0c
        fld dword ptr [esp + 050h] // 00c4af10
        fsub dword ptr [esi + 02ch] // 00c4af14
        fstp dword ptr [esp + 024h] // 00c4af17
        fld dword ptr [esi] // 00c4af1b
        fld dword ptr [esp + 01ch] // 00c4af1d
        fld st(0) // 00c4af21
        fmulp st(2), st(0) // 00c4af23
        fld dword ptr [esi + 4] // 00c4af25
        fld dword ptr [esp + 020h] // 00c4af28
        fld st(0) // 00c4af2c
        fmulp st(2), st(0) // 00c4af2e
        fxch st(3) // 00c4af30
        faddp st(1), st(0) // 00c4af32
        fld dword ptr [esp + 024h] // 00c4af34
        fld st(0) // 00c4af38
        fmul dword ptr [esi + 8] // 00c4af3a
        faddp st(2), st(0) // 00c4af3d
        fxch st(1) // 00c4af3f
        fstp dword ptr [eax + 4] // 00c4af41
        fld dword ptr [esi + 010h] // 00c4af44
        fmul st(0), st(3) // 00c4af47
        fld dword ptr [esi + 0ch] // 00c4af49
        fmul st(0), st(3) // 00c4af4c
        faddp st(1), st(0) // 00c4af4e
        fld dword ptr [esi + 014h] // 00c4af50
        fmul st(0), st(2) // 00c4af53
        faddp st(1), st(0) // 00c4af55
        fstp dword ptr [eax + 8] // 00c4af57
        fld dword ptr [esi + 01ch] // 00c4af5a
        fmulp st(3), st(0) // 00c4af5d
        fld dword ptr [esi + 018h] // 00c4af5f
        fmulp st(2), st(0) // 00c4af62
        fxch st(2) // 00c4af64
        faddp st(1), st(0) // 00c4af66
        fld dword ptr [esi + 020h] // 00c4af68
        fmulp st(2), st(0) // 00c4af6b
        faddp st(1), st(0) // 00c4af6d
        fstp dword ptr [eax + 0ch] // 00c4af6f
        fld dword ptr [esp + 070h] // 00c4af72
        fsub dword ptr [edi + 024h] // 00c4af76
        fstp dword ptr [esp + 01ch] // 00c4af79
        fld dword ptr [esp + 074h] // 00c4af7d
        fsub dword ptr [edi + 028h] // 00c4af81
        fstp dword ptr [esp + 020h] // 00c4af84
        fld dword ptr [esp + 078h] // 00c4af88
        fsub dword ptr [edi + 02ch] // 00c4af8c
        fstp dword ptr [esp + 024h] // 00c4af8f
        fld dword ptr [esp + 020h] // 00c4af93
        fld st(0) // 00c4af97
        fmul dword ptr [edi + 4] // 00c4af99
        fld dword ptr [edi] // 00c4af9c
        fld dword ptr [esp + 01ch] // 00c4af9e
        fld st(0) // 00c4afa2
        fmulp st(2), st(0) // 00c4afa4
        fxch st(2) // 00c4afa6
        faddp st(1), st(0) // 00c4afa8
        fld dword ptr [esp + 024h] // 00c4afaa
        fld st(0) // 00c4afae
        fmul dword ptr [edi + 8] // 00c4afb0
        faddp st(2), st(0) // 00c4afb3
        fxch st(1) // 00c4afb5
        fstp dword ptr [eax + 010h] // 00c4afb7
        fld dword ptr [edi + 010h] // 00c4afba
        fmul st(0), st(3) // 00c4afbd
        fld dword ptr [edi + 0ch] // 00c4afbf
        fmul st(0), st(3) // 00c4afc2
        faddp st(1), st(0) // 00c4afc4
        fld dword ptr [edi + 014h] // 00c4afc6
        fmul st(0), st(2) // 00c4afc9
        faddp st(1), st(0) // 00c4afcb
        fstp dword ptr [eax + 014h] // 00c4afcd
        fld dword ptr [edi + 01ch] // 00c4afd0
        fmulp st(3), st(0) // 00c4afd3
        fld dword ptr [edi + 018h] // 00c4afd5
        fmulp st(2), st(0) // 00c4afd8
        fxch st(2) // 00c4afda
        faddp st(1), st(0) // 00c4afdc
        fld dword ptr [edi + 020h] // 00c4afde
        fmulp st(2), st(0) // 00c4afe1
        faddp st(1), st(0) // 00c4afe3
        fstp dword ptr [eax + 018h] // 00c4afe5
        mov al, 1 // 00c4afe8
        pop edi // 00c4afea
        pop esi // 00c4afeb
        pop ebx // 00c4afec
        mov esp, ebp // 00c4afed
        pop ebp // 00c4afef
        ret 24 // 00c4aff0
    l_00c4aff3:
        cmp ecx, 3 // 00c4aff3
        fld dword ptr [esp + 054h] // 00c4aff6
        fadd qword ptr constant_00d7a318 // 00c4affa
        fstp dword ptr [esp + 054h] // 00c4b000
        jge l_00c4b012 // 00c4b004
        movss xmm0, dword ptr constant_00d7a260 // 00c4b006
        xor eax, eax // 00c4b00e
        jmp l_00c4b029 // 00c4b010
    l_00c4b012:
        movss xmm0, dword ptr constant_00d7a24c // 00c4b012
        sub ecx, 3 // 00c4b01a
        mov eax, 1 // 00c4b01d
        mov dword ptr [esp + 016ch], ecx // 00c4b022
    l_00c4b029:
        movss xmm1, dword ptr [esp + 054h] // 00c4b029
        fld dword ptr [esp + 068h] // 00c4b02f
        mov dword ptr [esp + 020ch], eax // 00c4b033
        fld st(0) // 00c4b03a
        lea edx, [eax + eax*2] // 00c4b03c
        lea eax, [eax + eax*2] // 00c4b03f
        add eax, eax // 00c4b042
        add eax, eax // 00c4b044
        mov ecx, eax // 00c4b046
        movss dword ptr [esp + 01f8h], xmm1 // 00c4b048
        movss dword ptr [esp + 0208h], xmm0 // 00c4b051
        shl edx, 4 // 00c4b05a
        lea eax, [esp + 015ch] // 00c4b05d
        sub eax, ecx // 00c4b064
        mov ecx, dword ptr [esp + 06ch] // 00c4b066
        mov dword ptr [esp + 0204h], ecx // 00c4b06a
        lea ebx, [esp + 0120h] // 00c4b071
        sub ebx, edx // 00c4b078
        mov edx, dword ptr [esp + 064h] // 00c4b07a
        mov dword ptr [esp + 01fch], edx // 00c4b07e
        mov dword ptr [esp + 018h], eax // 00c4b085
        mov eax, dword ptr [esp + 068h] // 00c4b089
        mov dword ptr [esp + 0200h], eax // 00c4b08d
        mov edx, dword ptr [ebx + 024h] // 00c4b094
        mov dword ptr [esp + 0170h], edx // 00c4b097
        mov eax, dword ptr [ebx + 028h] // 00c4b09e
        mov dword ptr [esp + 0174h], eax // 00c4b0a1
        mov ecx, dword ptr [ebx + 02ch] // 00c4b0a8
        mov dword ptr [esp + 0178h], ecx // 00c4b0ab
        fmul dword ptr [ebx + 4] // 00c4b0b2
        fld dword ptr [ebx] // 00c4b0b5
        push ecx // 00c4b0b7
        fld dword ptr [esp + 068h] // 00c4b0b8
        movss dword ptr [esp + 040h], xmm0 // 00c4b0bc
        fld st(0) // 00c4b0c2
        mov dword ptr [esp + 018h], 0 // 00c4b0c4
        fmulp st(2), st(0) // 00c4b0cc
        fxch st(2) // 00c4b0ce
        faddp st(1), st(0) // 00c4b0d0
        fld dword ptr [ebx + 8] // 00c4b0d2
        fld dword ptr [esp + 070h] // 00c4b0d5
        fld st(0) // 00c4b0d9
        fmulp st(2), st(0) // 00c4b0db
        fxch st(2) // 00c4b0dd
        faddp st(1), st(0) // 00c4b0df
        fstp dword ptr [esp + 05ch] // 00c4b0e1
        fld dword ptr [ebx + 010h] // 00c4b0e5
        fmul st(0), st(3) // 00c4b0e8
        fld st(2) // 00c4b0ea
        fmul dword ptr [ebx + 0ch] // 00c4b0ec
        faddp st(1), st(0) // 00c4b0ef
        fld dword ptr [ebx + 014h] // 00c4b0f1
        fmul st(0), st(2) // 00c4b0f4
        faddp st(1), st(0) // 00c4b0f6
        fstp dword ptr [esp + 060h] // 00c4b0f8
        fld dword ptr [ebx + 01ch] // 00c4b0fc
        fmulp st(3), st(0) // 00c4b0ff
        fld dword ptr [ebx + 018h] // 00c4b101
        fmulp st(2), st(0) // 00c4b104
        fxch st(2) // 00c4b106
        faddp st(1), st(0) // 00c4b108
        fld dword ptr [ebx + 020h] // 00c4b10a
        fmulp st(2), st(0) // 00c4b10d
        faddp st(1), st(0) // 00c4b10f
        fstp dword ptr [esp + 064h] // 00c4b111
        fld dword ptr [esp + 05ch] // 00c4b115
        fstp dword ptr [esp] // 00c4b119
        call abs_kernel // 00c4b11c
        fstp dword ptr [esp + 010h] // 00c4b121
        push ecx // 00c4b125
        fld dword ptr [esp + 060h] // 00c4b126
        fstp dword ptr [esp] // 00c4b12a
        call abs_kernel // 00c4b12d
        fstp dword ptr [esp + 044h] // 00c4b132
        fld dword ptr [esp + 010h] // 00c4b136
        fld dword ptr [esp + 044h] // 00c4b13a
        fcomip st(0), st(1) // 00c4b13e
        fstp st(0) // 00c4b140
        jbe l_00c4b158 // 00c4b142
        movss xmm0, dword ptr [esp + 044h] // 00c4b144
        movss dword ptr [esp + 010h], xmm0 // 00c4b14a
        mov dword ptr [esp + 014h], 1 // 00c4b150
    l_00c4b158:
        fld dword ptr [esp + 060h] // 00c4b158
        push ecx // 00c4b15c
        fstp dword ptr [esp] // 00c4b15d
        call abs_kernel // 00c4b160
        fld dword ptr [esp + 010h] // 00c4b165
        fxch st(1) // 00c4b169
        fcomip st(0), st(1) // 00c4b16b
        fstp st(0) // 00c4b16d
        jbe l_00c4b179 // 00c4b16f
        mov dword ptr [esp + 014h], 2 // 00c4b171
    l_00c4b179:
        mov eax, dword ptr [esp + 014h] // 00c4b179
        sub eax, 0 // 00c4b17d
        mov ecx, 0ch // 00c4b180
        lea edi, [esp + 090h] // 00c4b185
        rep movsd // 00c4b18c
        mov esi, dword ptr [ebp + 018h] // 00c4b18e
        mov ecx, 0ch // 00c4b191
        lea edi, [esp + 0c0h] // 00c4b196
        rep movsd // 00c4b19d
        je l_00c4b405 // 00c4b19f
        sub eax, 1 // 00c4b1a5
        je l_00c4b2de // 00c4b1a8
        sub eax, 1 // 00c4b1ae
        jne l_00c4b536 // 00c4b1b1
        mov eax, dword ptr [esp + 018h] // 00c4b1b7
        fld dword ptr [eax] // 00c4b1bb
        movss xmm0, dword ptr [esp + 060h] // 00c4b1bd
        comiss xmm0, dword ptr constant_00d7a218 // 00c4b1c3
        fstp dword ptr [esp + 028h] // 00c4b1ca
        fld dword ptr [ebx] // 00c4b1ce
        fld dword ptr [esp + 028h] // 00c4b1d0
        fld st(0) // 00c4b1d4
        fmulp st(2), st(0) // 00c4b1d6
        fxch st(1) // 00c4b1d8
        fstp dword ptr [esp + 064h] // 00c4b1da
        fld st(0) // 00c4b1de
        fmul dword ptr [ebx + 4] // 00c4b1e0
        fstp dword ptr [esp + 068h] // 00c4b1e3
        fmul dword ptr [ebx + 8] // 00c4b1e7
        fstp dword ptr [esp + 06ch] // 00c4b1ea
        fld dword ptr [eax + 4] // 00c4b1ee
        fstp dword ptr [esp + 028h] // 00c4b1f1
        fld dword ptr [ebx + 0ch] // 00c4b1f5
        fld dword ptr [esp + 028h] // 00c4b1f8
        fld st(0) // 00c4b1fc
        fmulp st(2), st(0) // 00c4b1fe
        fxch st(1) // 00c4b200
        fstp dword ptr [esp + 048h] // 00c4b202
        fld dword ptr [ebx + 010h] // 00c4b206
        fmul st(0), st(1) // 00c4b209
        fstp dword ptr [esp + 04ch] // 00c4b20b
        fmul dword ptr [ebx + 014h] // 00c4b20f
        fstp dword ptr [esp + 050h] // 00c4b212
        jbe l_00c4b222 // 00c4b216
        movss xmm0, dword ptr constant_00d7a24c // 00c4b218
        jmp l_00c4b22a // 00c4b220
    l_00c4b222:
        movss xmm0, dword ptr constant_00d7a260 // 00c4b222
    l_00c4b22a:
        fld dword ptr [eax + 8] // 00c4b22a
        movss dword ptr [esp + 010h], xmm0 // 00c4b22d
        fmul dword ptr [esp + 03ch] // 00c4b233
        push 2 // 00c4b237
        fmul dword ptr [esp + 014h] // 00c4b239
        fstp dword ptr [esp + 02ch] // 00c4b23d
        fld dword ptr [ebx + 018h] // 00c4b241
        fld dword ptr [esp + 02ch] // 00c4b244
        fld st(0) // 00c4b248
        fmulp st(2), st(0) // 00c4b24a
        fxch st(1) // 00c4b24c
        fstp dword ptr [esp + 020h] // 00c4b24e
        fld dword ptr [ebx + 01ch] // 00c4b252
        fmul st(0), st(1) // 00c4b255
        fstp dword ptr [esp + 024h] // 00c4b257
        fmul dword ptr [ebx + 020h] // 00c4b25b
        fstp dword ptr [esp + 028h] // 00c4b25e
        fld dword ptr [esp + 020h] // 00c4b262
        fsub dword ptr [esp + 04ch] // 00c4b266
        fstp dword ptr [esp + 05ch] // 00c4b26a
        fld dword ptr [esp + 024h] // 00c4b26e
        fsub dword ptr [esp + 050h] // 00c4b272
        fstp dword ptr [esp + 060h] // 00c4b276
        fld dword ptr [esp + 028h] // 00c4b27a
        fsub dword ptr [esp + 054h] // 00c4b27e
        fstp dword ptr [esp + 064h] // 00c4b282
        fld dword ptr [esp + 05ch] // 00c4b286
        fsub dword ptr [esp + 068h] // 00c4b28a
        fstp dword ptr [esp + 020h] // 00c4b28e
        fld dword ptr [esp + 060h] // 00c4b292
        fsub dword ptr [esp + 06ch] // 00c4b296
        fstp dword ptr [esp + 024h] // 00c4b29a
        fld dword ptr [esp + 064h] // 00c4b29e
        fsub dword ptr [esp + 070h] // 00c4b2a2
        fstp dword ptr [esp + 028h] // 00c4b2a6
        fld dword ptr [esp + 020h] // 00c4b2aa
        fadd dword ptr [esp + 0174h] // 00c4b2ae
        fstp dword ptr [esp + 0174h] // 00c4b2b5
        fld dword ptr [esp + 024h] // 00c4b2bc
        fadd dword ptr [esp + 0178h] // 00c4b2c0
        fstp dword ptr [esp + 0178h] // 00c4b2c7
        fld dword ptr [esp + 028h] // 00c4b2ce
        fadd dword ptr [esp + 017ch] // 00c4b2d2
        jmp l_00c4b523 // 00c4b2d9
    l_00c4b2de:
        mov eax, dword ptr [esp + 018h] // 00c4b2de
        fld dword ptr [eax + 8] // 00c4b2e2
        movss xmm0, dword ptr [esp + 05ch] // 00c4b2e5
        comiss xmm0, dword ptr constant_00d7a218 // 00c4b2eb
        fstp dword ptr [esp + 028h] // 00c4b2f2
        fld dword ptr [ebx + 018h] // 00c4b2f6
        fld dword ptr [esp + 028h] // 00c4b2f9
        fld st(0) // 00c4b2fd
        fmulp st(2), st(0) // 00c4b2ff
        fxch st(1) // 00c4b301
        fstp dword ptr [esp + 064h] // 00c4b303
        fld dword ptr [ebx + 01ch] // 00c4b307
        fmul st(0), st(1) // 00c4b30a
        fstp dword ptr [esp + 068h] // 00c4b30c
        fmul dword ptr [ebx + 020h] // 00c4b310
        fstp dword ptr [esp + 06ch] // 00c4b313
        fld dword ptr [eax] // 00c4b317
        fstp dword ptr [esp + 028h] // 00c4b319
        fld dword ptr [ebx] // 00c4b31d
        fld dword ptr [esp + 028h] // 00c4b31f
        fld st(0) // 00c4b323
        fmulp st(2), st(0) // 00c4b325
        fxch st(1) // 00c4b327
        fstp dword ptr [esp + 048h] // 00c4b329
        fld dword ptr [ebx + 4] // 00c4b32d
        fmul st(0), st(1) // 00c4b330
        fstp dword ptr [esp + 04ch] // 00c4b332
        fmul dword ptr [ebx + 8] // 00c4b336
        fstp dword ptr [esp + 050h] // 00c4b339
        jbe l_00c4b349 // 00c4b33d
        movss xmm0, dword ptr constant_00d7a24c // 00c4b33f
        jmp l_00c4b351 // 00c4b347
    l_00c4b349:
        movss xmm0, dword ptr constant_00d7a260 // 00c4b349
    l_00c4b351:
        fld dword ptr [eax + 4] // 00c4b351
        movss dword ptr [esp + 010h], xmm0 // 00c4b354
        fmul dword ptr [esp + 03ch] // 00c4b35a
        push 1 // 00c4b35e
        fmul dword ptr [esp + 014h] // 00c4b360
        fstp dword ptr [esp + 02ch] // 00c4b364
        fld dword ptr [ebx + 0ch] // 00c4b368
        fld dword ptr [esp + 02ch] // 00c4b36b
        fld st(0) // 00c4b36f
        fmulp st(2), st(0) // 00c4b371
        fxch st(1) // 00c4b373
        fstp dword ptr [esp + 020h] // 00c4b375
        fld dword ptr [ebx + 010h] // 00c4b379
        fmul st(0), st(1) // 00c4b37c
        fstp dword ptr [esp + 024h] // 00c4b37e
        fmul dword ptr [ebx + 014h] // 00c4b382
        fstp dword ptr [esp + 028h] // 00c4b385
        fld dword ptr [esp + 020h] // 00c4b389
        fsub dword ptr [esp + 04ch] // 00c4b38d
        fstp dword ptr [esp + 05ch] // 00c4b391
        fld dword ptr [esp + 024h] // 00c4b395
        fsub dword ptr [esp + 050h] // 00c4b399
        fstp dword ptr [esp + 060h] // 00c4b39d
        fld dword ptr [esp + 028h] // 00c4b3a1
        fsub dword ptr [esp + 054h] // 00c4b3a5
        fstp dword ptr [esp + 064h] // 00c4b3a9
        fld dword ptr [esp + 05ch] // 00c4b3ad
        fsub dword ptr [esp + 068h] // 00c4b3b1
        fstp dword ptr [esp + 020h] // 00c4b3b5
        fld dword ptr [esp + 060h] // 00c4b3b9
        fsub dword ptr [esp + 06ch] // 00c4b3bd
        fstp dword ptr [esp + 024h] // 00c4b3c1
        fld dword ptr [esp + 064h] // 00c4b3c5
        fsub dword ptr [esp + 070h] // 00c4b3c9
        fstp dword ptr [esp + 028h] // 00c4b3cd
        fld dword ptr [esp + 020h] // 00c4b3d1
        fadd dword ptr [esp + 0174h] // 00c4b3d5
        fstp dword ptr [esp + 0174h] // 00c4b3dc
        fld dword ptr [esp + 024h] // 00c4b3e3
        fadd dword ptr [esp + 0178h] // 00c4b3e7
        fstp dword ptr [esp + 0178h] // 00c4b3ee
        fld dword ptr [esp + 028h] // 00c4b3f5
        fadd dword ptr [esp + 017ch] // 00c4b3f9
        jmp l_00c4b523 // 00c4b400
    l_00c4b405:
        mov eax, dword ptr [esp + 018h] // 00c4b405
        fld dword ptr [eax + 8] // 00c4b409
        movss xmm0, dword ptr [esp + 058h] // 00c4b40c
        comiss xmm0, dword ptr constant_00d7a218 // 00c4b412
        fstp dword ptr [esp + 028h] // 00c4b419
        fld dword ptr [ebx + 018h] // 00c4b41d
        fld dword ptr [esp + 028h] // 00c4b420
        fld st(0) // 00c4b424
        fmulp st(2), st(0) // 00c4b426
        fxch st(1) // 00c4b428
        fstp dword ptr [esp + 064h] // 00c4b42a
        fld dword ptr [ebx + 01ch] // 00c4b42e
        fmul st(0), st(1) // 00c4b431
        fstp dword ptr [esp + 068h] // 00c4b433
        fmul dword ptr [ebx + 020h] // 00c4b437
        fstp dword ptr [esp + 06ch] // 00c4b43a
        fld dword ptr [eax + 4] // 00c4b43e
        fstp dword ptr [esp + 028h] // 00c4b441
        fld dword ptr [esp + 028h] // 00c4b445
        fld st(0) // 00c4b449
        fmul dword ptr [ebx + 0ch] // 00c4b44b
        fstp dword ptr [esp + 048h] // 00c4b44e
        fld dword ptr [ebx + 010h] // 00c4b452
        fmul st(0), st(1) // 00c4b455
        fstp dword ptr [esp + 04ch] // 00c4b457
        fmul dword ptr [ebx + 014h] // 00c4b45b
        fstp dword ptr [esp + 050h] // 00c4b45e
        jbe l_00c4b46e // 00c4b462
        movss xmm0, dword ptr constant_00d7a24c // 00c4b464
        jmp l_00c4b476 // 00c4b46c
    l_00c4b46e:
        movss xmm0, dword ptr constant_00d7a260 // 00c4b46e
    l_00c4b476:
        fld dword ptr [esp + 03ch] // 00c4b476
        movss dword ptr [esp + 010h], xmm0 // 00c4b47a
        fmul dword ptr [eax] // 00c4b480
        push 0 // 00c4b482
        fmul dword ptr [esp + 014h] // 00c4b484
        fstp dword ptr [esp + 02ch] // 00c4b488
        fld dword ptr [ebx] // 00c4b48c
        fld dword ptr [esp + 02ch] // 00c4b48e
        fld st(0) // 00c4b492
        fmulp st(2), st(0) // 00c4b494
        fxch st(1) // 00c4b496
        fstp dword ptr [esp + 020h] // 00c4b498
        fld dword ptr [ebx + 4] // 00c4b49c
        fmul st(0), st(1) // 00c4b49f
        fstp dword ptr [esp + 024h] // 00c4b4a1
        fmul dword ptr [ebx + 8] // 00c4b4a5
        fstp dword ptr [esp + 028h] // 00c4b4a8
        fld dword ptr [esp + 020h] // 00c4b4ac
        fsub dword ptr [esp + 04ch] // 00c4b4b0
        fstp dword ptr [esp + 05ch] // 00c4b4b4
        fld dword ptr [esp + 024h] // 00c4b4b8
        fsub dword ptr [esp + 050h] // 00c4b4bc
        fstp dword ptr [esp + 060h] // 00c4b4c0
        fld dword ptr [esp + 028h] // 00c4b4c4
        fsub dword ptr [esp + 054h] // 00c4b4c8
        fstp dword ptr [esp + 064h] // 00c4b4cc
        fld dword ptr [esp + 05ch] // 00c4b4d0
        fsub dword ptr [esp + 068h] // 00c4b4d4
        fstp dword ptr [esp + 020h] // 00c4b4d8
        fld dword ptr [esp + 060h] // 00c4b4dc
        fsub dword ptr [esp + 06ch] // 00c4b4e0
        fstp dword ptr [esp + 024h] // 00c4b4e4
        fld dword ptr [esp + 064h] // 00c4b4e8
        fsub dword ptr [esp + 070h] // 00c4b4ec
        fstp dword ptr [esp + 028h] // 00c4b4f0
        fld dword ptr [esp + 020h] // 00c4b4f4
        fadd dword ptr [esp + 0174h] // 00c4b4f8
        fstp dword ptr [esp + 0174h] // 00c4b4ff
        fld dword ptr [esp + 0178h] // 00c4b506
        fadd dword ptr [esp + 024h] // 00c4b50d
        fstp dword ptr [esp + 0178h] // 00c4b511
        fld dword ptr [esp + 017ch] // 00c4b518
        fadd dword ptr [esp + 028h] // 00c4b51f
    l_00c4b523:
        lea esi, [esp + 094h] // 00c4b523
        fstp dword ptr [esp + 017ch] // 00c4b52a
        call face_kernel // 00c4b531
    l_00c4b536:
        pop edi // 00c4b536
        pop esi // 00c4b537
        mov al, 1 // 00c4b538
        pop ebx // 00c4b53a
        mov esp, ebp // 00c4b53b
        pop ebp // 00c4b53d
        ret 24 // 00c4b53e
    }
}
} // namespace
bool intersect_native_dyn_boxes_00c49a30(void* result,const void* a,const float* ma,const void* b,const float* mb,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;unsigned char answer;
    __asm {
        push c
        push mb
        push b
        push ma
        push a
        push result
        call box_box_kernel
        mov answer,al
    }
    return answer!=0;
}
static_assert(std::is_standard_layout_v<NativeDynBoxBoxRuntime>);
NativeDynBoxBoxRuntime::NativeDynBoxBoxRuntime(const CameraAxesCrtAccess& crt) noexcept
    :methods_{reinterpret_cast<std::uintptr_t>(&intersect)},crt_(crt) {}
bool __fastcall NativeDynBoxBoxRuntime::intersect(void* owner,void*,void* result,const void* a,const float* ma,const void* b,const float* mb){
    const auto& storage=*static_cast<const DynStaticDispatchObjectStorage*>(owner);
    const auto& runtime=*static_cast<const NativeDynBoxBoxRuntime*>(storage.vtable);
    return intersect_native_dyn_boxes_00c49a30(result,a,ma,b,mb,runtime.crt_);
}
} // namespace bsp
