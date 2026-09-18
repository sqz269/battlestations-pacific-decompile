#include "bsp/native_dyn_primitive_dispatch.hpp"
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native primitive dispatch requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
alignas(8) const std::uint32_t constant_00d7a208=0x80000000U;
alignas(8) const std::uint32_t constant_00d7a24c=0x3f800000U;
alignas(8) const std::uint64_t constant_00d7a258=0x0000000000000000ULL;
alignas(8) const std::uint64_t constant_00d7a280=0x3fe0000000000000ULL;
alignas(8) const std::uint64_t constant_00d7a328=0x4010000000000000ULL;
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
// Complete original instruction schedule; address comments are native starts.
__declspec(naked) void sphere_sphere_kernel(){
    __asm {
        push ebp // 00c518d0
        mov ebp, esp // 00c518d1
        and esp, 0fffffff8h // 00c518d3
        sub esp, 04ch // 00c518d6
        mov ecx, dword ptr [ebp + 0ch] // 00c518d9
        mov eax, dword ptr [ebp + 014h] // 00c518dc
        fld dword ptr [ecx + 0210h] // 00c518df
        fadd dword ptr [eax + 0210h] // 00c518e5
        push ebx // 00c518eb
        push esi // 00c518ec
        mov esi, dword ptr [ebp + 010h] // 00c518ed
        fstp dword ptr [esp + 014h] // 00c518f0
        push edi // 00c518f4
        fld dword ptr [ecx + 05ch] // 00c518f5
        mov edi, dword ptr [ebp + 018h] // 00c518f8
        fstp dword ptr [esp + 010h] // 00c518fb
        fld dword ptr [ecx + 058h] // 00c518ff
        fstp dword ptr [esp + 0ch] // 00c51902
        fld dword ptr [ecx + 060h] // 00c51906
        fstp dword ptr [esp + 014h] // 00c51909
        fld dword ptr [esp + 0ch] // 00c5190d
        fld st(0) // 00c51911
        fmul dword ptr [esi] // 00c51913
        fld dword ptr [esp + 010h] // 00c51915
        fld st(0) // 00c51919
        fmul dword ptr [esi + 0ch] // 00c5191b
        faddp st(2), st(0) // 00c5191e
        fld dword ptr [esp + 014h] // 00c51920
        fld st(0) // 00c51924
        fmul dword ptr [esi + 018h] // 00c51926
        faddp st(3), st(0) // 00c51929
        fld dword ptr [esi + 024h] // 00c5192b
        faddp st(3), st(0) // 00c5192e
        fxch st(2) // 00c51930
        fstp dword ptr [esp + 028h] // 00c51932
        fld dword ptr [esi + 010h] // 00c51936
        fmul st(0), st(1) // 00c51939
        fld st(3) // 00c5193b
        fmul dword ptr [esi + 4] // 00c5193d
        faddp st(1), st(0) // 00c51940
        fld dword ptr [esi + 01ch] // 00c51942
        fmul st(0), st(3) // 00c51945
        faddp st(1), st(0) // 00c51947
        fadd dword ptr [esi + 028h] // 00c51949
        fstp dword ptr [esp + 02ch] // 00c5194c
        fmul dword ptr [esi + 014h] // 00c51950
        fld dword ptr [esi + 8] // 00c51953
        fmulp st(3), st(0) // 00c51956
        faddp st(2), st(0) // 00c51958
        fmul dword ptr [esi + 020h] // 00c5195a
        faddp st(1), st(0) // 00c5195d
        fadd dword ptr [esi + 02ch] // 00c5195f
        fstp dword ptr [esp + 030h] // 00c51962
        fld dword ptr [eax + 05ch] // 00c51966
        fstp dword ptr [esp + 010h] // 00c51969
        fld dword ptr [eax + 058h] // 00c5196d
        fstp dword ptr [esp + 014h] // 00c51970
        fld dword ptr [eax + 060h] // 00c51974
        fstp dword ptr [esp + 0ch] // 00c51977
        fld dword ptr [esp + 014h] // 00c5197b
        fld st(0) // 00c5197f
        fmul dword ptr [edi] // 00c51981
        fld dword ptr [esp + 010h] // 00c51983
        fld st(0) // 00c51987
        fmul dword ptr [edi + 0ch] // 00c51989
        faddp st(2), st(0) // 00c5198c
        fld dword ptr [esp + 0ch] // 00c5198e
        fld st(0) // 00c51992
        fmul dword ptr [edi + 018h] // 00c51994
        faddp st(3), st(0) // 00c51997
        fld dword ptr [edi + 024h] // 00c51999
        faddp st(3), st(0) // 00c5199c
        fxch st(2) // 00c5199e
        fstp dword ptr [esp + 034h] // 00c519a0
        fld dword ptr [edi + 010h] // 00c519a4
        fmul st(0), st(1) // 00c519a7
        fld st(3) // 00c519a9
        fmul dword ptr [edi + 4] // 00c519ab
        faddp st(1), st(0) // 00c519ae
        fld dword ptr [edi + 01ch] // 00c519b0
        fmul st(0), st(3) // 00c519b3
        faddp st(1), st(0) // 00c519b5
        fadd dword ptr [edi + 028h] // 00c519b7
        fstp dword ptr [esp + 038h] // 00c519ba
        fmul dword ptr [edi + 014h] // 00c519be
        fld dword ptr [edi + 8] // 00c519c1
        fmulp st(3), st(0) // 00c519c4
        faddp st(2), st(0) // 00c519c6
        fmul dword ptr [edi + 020h] // 00c519c8
        faddp st(1), st(0) // 00c519cb
        fadd dword ptr [edi + 02ch] // 00c519cd
        fstp dword ptr [esp + 03ch] // 00c519d0
        fld dword ptr [esp + 034h] // 00c519d4
        fsub dword ptr [esp + 028h] // 00c519d8
        fstp dword ptr [esp + 01ch] // 00c519dc
        fld dword ptr [esp + 038h] // 00c519e0
        fsub dword ptr [esp + 02ch] // 00c519e4
        fstp dword ptr [esp + 020h] // 00c519e8
        fld dword ptr [esp + 03ch] // 00c519ec
        fsub dword ptr [esp + 030h] // 00c519f0
        fstp dword ptr [esp + 024h] // 00c519f4
        fld dword ptr [esp + 020h] // 00c519f8
        fld dword ptr [esp + 01ch] // 00c519fc
        fld dword ptr [esp + 024h] // 00c51a00
        fld st(1) // 00c51a04
        fmulp st(2), st(0) // 00c51a06
        fld st(2) // 00c51a08
        fmulp st(3), st(0) // 00c51a0a
        fxch st(1) // 00c51a0c
        faddp st(2), st(0) // 00c51a0e
        fmul st(0), st(0) // 00c51a10
        faddp st(1), st(0) // 00c51a12
        fstp dword ptr [esp + 014h] // 00c51a14
        fld dword ptr [esp + 018h] // 00c51a18
        fld dword ptr [esp + 014h] // 00c51a1c
        fld st(1) // 00c51a20
        fmulp st(2), st(0) // 00c51a22
        fcomi st(0), st(1) // 00c51a24
        fstp st(1) // 00c51a26
        jb l_00c51a37 // 00c51a28
        fstp st(0) // 00c51a2a
        xor al, al // 00c51a2c
        pop edi // 00c51a2e
        pop esi // 00c51a2f
        pop ebx // 00c51a30
        mov esp, ebp // 00c51a31
        pop ebp // 00c51a33
        ret 24 // 00c51a34
    l_00c51a37:
        mov ebx, dword ptr [ebp + 8] // 00c51a37
        push ecx // 00c51a3a
        fstp dword ptr [esp] // 00c51a3b
        mov dword ptr [ebx], 1 // 00c51a3e
        push dword ptr [ebp+28] // Borrowed CRT context.
        call sqrt_kernel // 00c51a44
        fstp dword ptr [esp + 018h] // 00c51a49
        fld dword ptr [esp + 01ch] // 00c51a4d
        fld dword ptr [esp + 018h] // 00c51a51
        fld st(0) // 00c51a55
        fdivp st(2), st(0) // 00c51a57
        fxch st(1) // 00c51a59
        fstp dword ptr [esp + 01ch] // 00c51a5b
        fld dword ptr [esp + 020h] // 00c51a5f
        fdiv st(0), st(1) // 00c51a63
        mov eax, dword ptr [esp + 01ch] // 00c51a65
        mov dword ptr [ebx + 01ch], eax // 00c51a69
        mov eax, dword ptr [ebp + 0ch] // 00c51a6c
        fstp dword ptr [esp + 020h] // 00c51a6f
        mov ecx, dword ptr [esp + 020h] // 00c51a73
        mov dword ptr [ebx + 020h], ecx // 00c51a77
        fdivr dword ptr [esp + 024h] // 00c51a7a
        fstp dword ptr [esp + 024h] // 00c51a7e
        mov edx, dword ptr [esp + 024h] // 00c51a82
        mov dword ptr [ebx + 024h], edx // 00c51a86
        fld dword ptr [eax + 0210h] // 00c51a89
        fstp dword ptr [esp + 018h] // 00c51a8f
        fld dword ptr [esp + 01ch] // 00c51a93
        fld st(0) // 00c51a97
        fld dword ptr [esp + 018h] // 00c51a99
        fld st(0) // 00c51a9d
        fmulp st(2), st(0) // 00c51a9f
        fxch st(1) // 00c51aa1
        fstp dword ptr [esp + 040h] // 00c51aa3
        fld dword ptr [esp + 020h] // 00c51aa7
        fld st(0) // 00c51aab
        fmul st(0), st(2) // 00c51aad
        fstp dword ptr [esp + 044h] // 00c51aaf
        fld dword ptr [esp + 024h] // 00c51ab3
        fmulp st(2), st(0) // 00c51ab7
        fxch st(1) // 00c51ab9
        fstp dword ptr [esp + 048h] // 00c51abb
        fld dword ptr [esp + 040h] // 00c51abf
        fadd dword ptr [esp + 028h] // 00c51ac3
        fstp dword ptr [esp + 04ch] // 00c51ac7
        fld dword ptr [esp + 044h] // 00c51acb
        fadd dword ptr [esp + 02ch] // 00c51acf
        fstp dword ptr [esp + 050h] // 00c51ad3
        fld dword ptr [esp + 048h] // 00c51ad7
        fadd dword ptr [esp + 030h] // 00c51adb
        fstp dword ptr [esp + 054h] // 00c51adf
        fld dword ptr [esp + 04ch] // 00c51ae3
        fsub dword ptr [esi + 024h] // 00c51ae7
        fstp dword ptr [esp + 040h] // 00c51aea
        fld dword ptr [esp + 050h] // 00c51aee
        fsub dword ptr [esi + 028h] // 00c51af2
        fstp dword ptr [esp + 044h] // 00c51af5
        fld dword ptr [esp + 054h] // 00c51af9
        fsub dword ptr [esi + 02ch] // 00c51afd
        fstp dword ptr [esp + 048h] // 00c51b00
        fld dword ptr [esi] // 00c51b04
        fld dword ptr [esp + 040h] // 00c51b06
        fld st(0) // 00c51b0a
        fmulp st(2), st(0) // 00c51b0c
        fld dword ptr [esp + 044h] // 00c51b0e
        fld st(0) // 00c51b12
        fmul dword ptr [esi + 4] // 00c51b14
        faddp st(3), st(0) // 00c51b17
        fld dword ptr [esp + 048h] // 00c51b19
        fld st(0) // 00c51b1d
        fmul dword ptr [esi + 8] // 00c51b1f
        faddp st(4), st(0) // 00c51b22
        fxch st(3) // 00c51b24
        fstp dword ptr [ebx + 4] // 00c51b26
        fld dword ptr [esi + 010h] // 00c51b29
        fmul st(0), st(1) // 00c51b2c
        fld st(2) // 00c51b2e
        fmul dword ptr [esi + 0ch] // 00c51b30
        faddp st(1), st(0) // 00c51b33
        fld dword ptr [esi + 014h] // 00c51b35
        fmul st(0), st(4) // 00c51b38
        faddp st(1), st(0) // 00c51b3a
        fstp dword ptr [ebx + 8] // 00c51b3c
        mov ecx, dword ptr [ebp + 014h] // 00c51b3f
        mov al, 1 // 00c51b42
        fmul dword ptr [esi + 01ch] // 00c51b44
        fld dword ptr [esi + 018h] // 00c51b47
        fmulp st(2), st(0) // 00c51b4a
        faddp st(1), st(0) // 00c51b4c
        fld dword ptr [esi + 020h] // 00c51b4e
        fmulp st(2), st(0) // 00c51b51
        faddp st(1), st(0) // 00c51b53
        fstp dword ptr [ebx + 0ch] // 00c51b55
        fld dword ptr [ecx + 0210h] // 00c51b58
        fstp dword ptr [esp + 018h] // 00c51b5e
        fld dword ptr [esp + 018h] // 00c51b62
        fld st(0) // 00c51b66
        fmulp st(3), st(0) // 00c51b68
        fxch st(2) // 00c51b6a
        fstp dword ptr [esp + 04ch] // 00c51b6c
        fmul st(0), st(1) // 00c51b70
        fstp dword ptr [esp + 050h] // 00c51b72
        fmul dword ptr [esp + 024h] // 00c51b76
        fstp dword ptr [esp + 054h] // 00c51b7a
        fld dword ptr [esp + 034h] // 00c51b7e
        fsub dword ptr [esp + 04ch] // 00c51b82
        fstp dword ptr [esp + 040h] // 00c51b86
        fld dword ptr [esp + 038h] // 00c51b8a
        fsub dword ptr [esp + 050h] // 00c51b8e
        fstp dword ptr [esp + 044h] // 00c51b92
        fld dword ptr [esp + 03ch] // 00c51b96
        fsub dword ptr [esp + 054h] // 00c51b9a
        fstp dword ptr [esp + 048h] // 00c51b9e
        fld dword ptr [esp + 040h] // 00c51ba2
        fsub dword ptr [edi + 024h] // 00c51ba6
        fstp dword ptr [esp + 04ch] // 00c51ba9
        fld dword ptr [esp + 044h] // 00c51bad
        fsub dword ptr [edi + 028h] // 00c51bb1
        fstp dword ptr [esp + 050h] // 00c51bb4
        fld dword ptr [esp + 048h] // 00c51bb8
        fsub dword ptr [edi + 02ch] // 00c51bbc
        fstp dword ptr [esp + 054h] // 00c51bbf
        fld dword ptr [esp + 050h] // 00c51bc3
        fld st(0) // 00c51bc7
        fmul dword ptr [edi + 4] // 00c51bc9
        fld dword ptr [esp + 04ch] // 00c51bcc
        fld st(0) // 00c51bd0
        fmul dword ptr [edi] // 00c51bd2
        faddp st(2), st(0) // 00c51bd4
        fld dword ptr [esp + 054h] // 00c51bd6
        fld st(0) // 00c51bda
        fmul dword ptr [edi + 8] // 00c51bdc
        faddp st(3), st(0) // 00c51bdf
        fxch st(2) // 00c51be1
        fstp dword ptr [ebx + 010h] // 00c51be3
        fld dword ptr [edi + 010h] // 00c51be6
        fmul st(0), st(3) // 00c51be9
        fld dword ptr [edi + 0ch] // 00c51beb
        fmul st(0), st(2) // 00c51bee
        faddp st(1), st(0) // 00c51bf0
        fld dword ptr [edi + 014h] // 00c51bf2
        fmul st(0), st(3) // 00c51bf5
        faddp st(1), st(0) // 00c51bf7
        fstp dword ptr [ebx + 014h] // 00c51bf9
        fld dword ptr [edi + 01ch] // 00c51bfc
        fmulp st(3), st(0) // 00c51bff
        fmul dword ptr [edi + 018h] // 00c51c01
        faddp st(2), st(0) // 00c51c04
        fmul dword ptr [edi + 020h] // 00c51c06
        pop edi // 00c51c09
        pop esi // 00c51c0a
        faddp st(1), st(0) // 00c51c0b
        fstp dword ptr [ebx + 018h] // 00c51c0d
        pop ebx // 00c51c10
        mov esp, ebp // 00c51c11
        pop ebp // 00c51c13
        ret 24 // 00c51c14
    }
}
// Complete original instruction schedule; address comments are native starts.
__declspec(naked) void box_sphere_kernel(){
    __asm {
        push ebp // 00c48330
        mov ebp, esp // 00c48331
        and esp, 0fffffff8h // 00c48333
        mov eax, dword ptr [ebp + 0ch] // 00c48336
        sub esp, 07ch // 00c48339
        push ebx // 00c4833c
        mov edx, 1 // 00c4833d
        cmp dword ptr [eax + 8], edx // 00c48342
        push esi // 00c48345
        push edi // 00c48346
        jne l_00c4835e // 00c48347
        mov esi, dword ptr [ebp + 010h] // 00c48349
        mov edi, dword ptr [ebp + 018h] // 00c4834c
        mov ecx, eax // 00c4834f
        mov eax, dword ptr [ebp + 014h] // 00c48351
        mov dword ptr [esp + 030h], 0 // 00c48354
        jmp l_00c4836b // 00c4835c
    l_00c4835e:
        mov ecx, dword ptr [ebp + 014h] // 00c4835e
        mov esi, dword ptr [ebp + 018h] // 00c48361
        mov edi, dword ptr [ebp + 010h] // 00c48364
        mov dword ptr [esp + 030h], edx // 00c48367
    l_00c4836b:
        fld dword ptr [ecx + 038h] // 00c4836b
        movss xmm0, dword ptr [eax + 0210h] // 00c4836e
        fstp dword ptr [esp + 024h] // 00c48376
        movss dword ptr [esp + 02ch], xmm0 // 00c4837a
        fld dword ptr [esi + 0ch] // 00c48380
        fstp dword ptr [esp + 018h] // 00c48383
        fld dword ptr [esi] // 00c48387
        fstp dword ptr [esp + 01ch] // 00c48389
        fld dword ptr [ecx + 034h] // 00c4838d
        fstp dword ptr [esp + 028h] // 00c48390
        fld dword ptr [ecx + 03ch] // 00c48394
        fstp dword ptr [esp + 020h] // 00c48397
        fld dword ptr [esi + 018h] // 00c4839b
        fstp dword ptr [esp + 014h] // 00c4839e
        fld dword ptr [esp + 028h] // 00c483a2
        fld st(0) // 00c483a6
        fld dword ptr [esp + 01ch] // 00c483a8
        fld st(0) // 00c483ac
        fmulp st(2), st(0) // 00c483ae
        fld dword ptr [esp + 024h] // 00c483b0
        fld st(0) // 00c483b4
        fld dword ptr [esp + 018h] // 00c483b6
        fld st(0) // 00c483ba
        fmulp st(2), st(0) // 00c483bc
        fxch st(4) // 00c483be
        faddp st(1), st(0) // 00c483c0
        fld dword ptr [esp + 020h] // 00c483c2
        fld dword ptr [esp + 014h] // 00c483c6
        fld st(0) // 00c483ca
        fmulp st(2), st(0) // 00c483cc
        fxch st(2) // 00c483ce
        faddp st(1), st(0) // 00c483d0
        fstp dword ptr [esp + 058h] // 00c483d2
        fld dword ptr [esi + 010h] // 00c483d6
        fstp dword ptr [esp + 018h] // 00c483d9
        fld dword ptr [esi + 4] // 00c483dd
        fstp dword ptr [esp + 01ch] // 00c483e0
        fld dword ptr [esi + 01ch] // 00c483e4
        fstp dword ptr [esp + 010h] // 00c483e7
        fld dword ptr [esp + 01ch] // 00c483eb
        fld st(0) // 00c483ef
        fmulp st(6), st(0) // 00c483f1
        fld dword ptr [esp + 018h] // 00c483f3
        fld st(0) // 00c483f7
        fmulp st(4), st(0) // 00c483f9
        fxch st(6) // 00c483fb
        faddp st(3), st(0) // 00c483fd
        fld dword ptr [esp + 020h] // 00c483ff
        fmul dword ptr [esp + 010h] // 00c48403
        faddp st(3), st(0) // 00c48407
        fxch st(2) // 00c48409
        fstp dword ptr [esp + 05ch] // 00c4840b
        fld dword ptr [esi + 014h] // 00c4840f
        fstp dword ptr [esp + 018h] // 00c48412
        fld dword ptr [esi + 8] // 00c48416
        fstp dword ptr [esp + 014h] // 00c48419
        fld dword ptr [esi + 020h] // 00c4841d
        fstp dword ptr [esp + 01ch] // 00c48420
        fld dword ptr [esp + 028h] // 00c48424
        fmul dword ptr [esp + 014h] // 00c48428
        fld dword ptr [esp + 024h] // 00c4842c
        fmul dword ptr [esp + 018h] // 00c48430
        faddp st(1), st(0) // 00c48434
        fld dword ptr [esp + 020h] // 00c48436
        fmul dword ptr [esp + 01ch] // 00c4843a
        faddp st(1), st(0) // 00c4843e
        fstp dword ptr [esp + 060h] // 00c48440
        fld dword ptr [ecx + 044h] // 00c48444
        fstp dword ptr [esp + 024h] // 00c48447
        fld dword ptr [ecx + 040h] // 00c4844b
        fstp dword ptr [esp + 020h] // 00c4844e
        fld dword ptr [ecx + 048h] // 00c48452
        fstp dword ptr [esp + 028h] // 00c48455
        fld dword ptr [esp + 020h] // 00c48459
        fmul st(0), st(3) // 00c4845d
        fld dword ptr [esp + 024h] // 00c4845f
        fmul st(0), st(5) // 00c48463
        faddp st(1), st(0) // 00c48465
        fld dword ptr [esp + 028h] // 00c48467
        fmul st(0), st(2) // 00c4846b
        faddp st(1), st(0) // 00c4846d
        fstp dword ptr [esp + 064h] // 00c4846f
        fld dword ptr [esp + 020h] // 00c48473
        fmul st(0), st(2) // 00c48477
        fld dword ptr [esp + 024h] // 00c48479
        fmul st(0), st(6) // 00c4847d
        faddp st(1), st(0) // 00c4847f
        fld dword ptr [esp + 028h] // 00c48481
        fmul dword ptr [esp + 010h] // 00c48485
        faddp st(1), st(0) // 00c48489
        fstp dword ptr [esp + 068h] // 00c4848b
        fld dword ptr [esp + 020h] // 00c4848f
        fmul dword ptr [esp + 014h] // 00c48493
        fld dword ptr [esp + 024h] // 00c48497
        fmul dword ptr [esp + 018h] // 00c4849b
        faddp st(1), st(0) // 00c4849f
        fld dword ptr [esp + 028h] // 00c484a1
        fmul dword ptr [esp + 01ch] // 00c484a5
        faddp st(1), st(0) // 00c484a9
        fstp dword ptr [esp + 06ch] // 00c484ab
        fld dword ptr [ecx + 050h] // 00c484af
        fstp dword ptr [esp + 024h] // 00c484b2
        fld dword ptr [ecx + 04ch] // 00c484b6
        fstp dword ptr [esp + 028h] // 00c484b9
        fld dword ptr [ecx + 054h] // 00c484bd
        fstp dword ptr [esp + 020h] // 00c484c0
        fld dword ptr [esp + 028h] // 00c484c4
        fmul st(0), st(3) // 00c484c8
        fld dword ptr [esp + 024h] // 00c484ca
        fmul st(0), st(5) // 00c484ce
        faddp st(1), st(0) // 00c484d0
        fld dword ptr [esp + 020h] // 00c484d2
        fmul st(0), st(2) // 00c484d6
        faddp st(1), st(0) // 00c484d8
        fstp dword ptr [esp + 070h] // 00c484da
        fld dword ptr [esp + 028h] // 00c484de
        fmul st(0), st(2) // 00c484e2
        fld dword ptr [esp + 024h] // 00c484e4
        fmul st(0), st(6) // 00c484e8
        faddp st(1), st(0) // 00c484ea
        fld dword ptr [esp + 020h] // 00c484ec
        fmul dword ptr [esp + 010h] // 00c484f0
        faddp st(1), st(0) // 00c484f4
        fstp dword ptr [esp + 074h] // 00c484f6
        fld dword ptr [esp + 028h] // 00c484fa
        fmul dword ptr [esp + 014h] // 00c484fe
        fld dword ptr [esp + 024h] // 00c48502
        fmul dword ptr [esp + 018h] // 00c48506
        faddp st(1), st(0) // 00c4850a
        fld dword ptr [esp + 020h] // 00c4850c
        fmul dword ptr [esp + 01ch] // 00c48510
        faddp st(1), st(0) // 00c48514
        fstp dword ptr [esp + 078h] // 00c48516
        fld dword ptr [ecx + 05ch] // 00c4851a
        fstp dword ptr [esp + 024h] // 00c4851d
        fld dword ptr [ecx + 058h] // 00c48521
        fstp dword ptr [esp + 028h] // 00c48524
        fld dword ptr [ecx + 060h] // 00c48528
        fstp dword ptr [esp + 020h] // 00c4852b
        fld dword ptr [esp + 028h] // 00c4852f
        fld st(0) // 00c48533
        fmulp st(4), st(0) // 00c48535
        fld dword ptr [esp + 024h] // 00c48537
        fld st(0) // 00c4853b
        fmulp st(6), st(0) // 00c4853d
        fxch st(4) // 00c4853f
        faddp st(5), st(0) // 00c48541
        fld dword ptr [esp + 020h] // 00c48543
        fld st(0) // 00c48547
        fmulp st(3), st(0) // 00c48549
        fxch st(5) // 00c4854b
        faddp st(2), st(0) // 00c4854d
        fld dword ptr [esi + 024h] // 00c4854f
        faddp st(2), st(0) // 00c48552
        fxch st(1) // 00c48554
        fstp dword ptr [esp + 07ch] // 00c48556
        fld st(0) // 00c4855a
        fmulp st(2), st(0) // 00c4855c
        fld st(2) // 00c4855e
        fmulp st(5), st(0) // 00c48560
        fxch st(1) // 00c48562
        faddp st(4), st(0) // 00c48564
        fld dword ptr [esp + 010h] // 00c48566
        fmul st(0), st(3) // 00c4856a
        faddp st(4), st(0) // 00c4856c
        fld dword ptr [esi + 028h] // 00c4856e
        faddp st(4), st(0) // 00c48571
        fxch st(3) // 00c48573
        fstp dword ptr [esp + 080h] // 00c48575
        fld dword ptr [esp + 014h] // 00c4857c
        fmulp st(3), st(0) // 00c48580
        fmul dword ptr [esp + 018h] // 00c48582
        faddp st(2), st(0) // 00c48586
        fmul dword ptr [esp + 01ch] // 00c48588
        faddp st(1), st(0) // 00c4858c
        fadd dword ptr [esi + 02ch] // 00c4858e
        fstp dword ptr [esp + 084h] // 00c48591
        fld dword ptr [eax + 05ch] // 00c48598
        fstp dword ptr [esp + 024h] // 00c4859b
        fld dword ptr [eax + 058h] // 00c4859f
        fstp dword ptr [esp + 028h] // 00c485a2
        fld dword ptr [eax + 060h] // 00c485a6
        fstp dword ptr [esp + 020h] // 00c485a9
        fld dword ptr [edi] // 00c485ad
        fld dword ptr [esp + 028h] // 00c485af
        fld st(0) // 00c485b3
        fmulp st(2), st(0) // 00c485b5
        fld dword ptr [esp + 024h] // 00c485b7
        fld st(0) // 00c485bb
        fmul dword ptr [edi + 0ch] // 00c485bd
        faddp st(3), st(0) // 00c485c0
        fld dword ptr [esp + 020h] // 00c485c2
        fld st(0) // 00c485c6
        fmul dword ptr [edi + 018h] // 00c485c8
        faddp st(4), st(0) // 00c485cb
        fld dword ptr [edi + 024h] // 00c485cd
        faddp st(4), st(0) // 00c485d0
        fxch st(3) // 00c485d2
        fstp dword ptr [esp + 04ch] // 00c485d4
        fld dword ptr [edi + 010h] // 00c485d8
        fmul st(0), st(1) // 00c485db
        fld st(2) // 00c485dd
        fmul dword ptr [edi + 4] // 00c485df
        faddp st(1), st(0) // 00c485e2
        fld dword ptr [edi + 01ch] // 00c485e4
        fmul st(0), st(4) // 00c485e7
        faddp st(1), st(0) // 00c485e9
        fadd dword ptr [edi + 028h] // 00c485eb
        fstp dword ptr [esp + 050h] // 00c485ee
        fmul dword ptr [edi + 014h] // 00c485f2
        fld dword ptr [edi + 8] // 00c485f5
        fmulp st(2), st(0) // 00c485f8
        faddp st(1), st(0) // 00c485fa
        fld dword ptr [edi + 020h] // 00c485fc
        fmulp st(2), st(0) // 00c485ff
        faddp st(1), st(0) // 00c48601
        fadd dword ptr [edi + 02ch] // 00c48603
        fstp dword ptr [esp + 054h] // 00c48606
        fld dword ptr [esp + 04ch] // 00c4860a
        fsub dword ptr [esp + 07ch] // 00c4860e
        fstp dword ptr [esp + 040h] // 00c48612
        fld dword ptr [esp + 050h] // 00c48616
        fsub dword ptr [esp + 080h] // 00c4861a
        fstp dword ptr [esp + 044h] // 00c48621
        fld dword ptr [esp + 054h] // 00c48625
        fsub dword ptr [esp + 084h] // 00c48629
        fstp dword ptr [esp + 048h] // 00c48630
        fld dword ptr [esp + 044h] // 00c48634
        fld st(0) // 00c48638
        fmul dword ptr [esp + 05ch] // 00c4863a
        fld dword ptr [esp + 040h] // 00c4863e
        fld st(0) // 00c48642
        fmul dword ptr [esp + 058h] // 00c48644
        faddp st(2), st(0) // 00c48648
        fld dword ptr [esp + 048h] // 00c4864a
        fld st(0) // 00c4864e
        fmul dword ptr [esp + 060h] // 00c48650
        faddp st(3), st(0) // 00c48654
        fxch st(2) // 00c48656
        fstp dword ptr [esp + 040h] // 00c48658
        fld dword ptr [esp + 064h] // 00c4865c
        fmul st(0), st(1) // 00c48660
        fld st(3) // 00c48662
        fmul dword ptr [esp + 068h] // 00c48664
        xorps xmm0, xmm0 // 00c48668
        movss dword ptr [esp + 034h], xmm0 // 00c4866b
        movss dword ptr [esp + 038h], xmm0 // 00c48671
        faddp st(1), st(0) // 00c48677
        movss dword ptr [esp + 03ch], xmm0 // 00c48679
        fld st(2) // 00c4867f
        movss dword ptr [esp + 010h], xmm0 // 00c48681
        fmul dword ptr [esp + 06ch] // 00c48687
        faddp st(1), st(0) // 00c4868b
        fstp dword ptr [esp + 044h] // 00c4868d
        fmul dword ptr [esp + 070h] // 00c48691
        fld dword ptr [esp + 074h] // 00c48695
        fmulp st(3), st(0) // 00c48699
        faddp st(2), st(0) // 00c4869b
        fmul dword ptr [esp + 078h] // 00c4869d
        faddp st(1), st(0) // 00c486a1
        fstp dword ptr [esp + 048h] // 00c486a3
        fld dword ptr [ecx + 0210h] // 00c486a7
        fstp dword ptr [esp + 028h] // 00c486ad
        fld dword ptr [esp + 040h] // 00c486b1
        fld dword ptr [esp + 028h] // 00c486b5
        fld st(0) // 00c486b9
        fchs  // 00c486bb
        fcomip st(0), st(2) // 00c486bd
        jbe l_00c486db // 00c486bf
        fld st(0) // 00c486c1
        fadd st(0), st(2) // 00c486c3
        fstp dword ptr [esp + 034h] // 00c486c5
        fld dword ptr [esp + 034h] // 00c486c9
        fmul st(0), st(0) // 00c486cd
        fadd qword ptr constant_00d7a258 // 00c486cf
        fstp dword ptr [esp + 010h] // 00c486d5
        jmp l_00c486fb // 00c486d9
    l_00c486db:
        fxch st(1) // 00c486db
        fcomi st(0), st(1) // 00c486dd
        jbe l_00c486f9 // 00c486df
        fld st(0) // 00c486e1
        fsub st(0), st(2) // 00c486e3
        fstp dword ptr [esp + 034h] // 00c486e5
        fld dword ptr [esp + 034h] // 00c486e9
        fmul st(0), st(0) // 00c486ed
        fadd qword ptr constant_00d7a258 // 00c486ef
        fstp dword ptr [esp + 010h] // 00c486f5
    l_00c486f9:
        fxch st(1) // 00c486f9
    l_00c486fb:
        fld dword ptr [ecx + 0214h] // 00c486fb
        fstp dword ptr [esp + 028h] // 00c48701
        fld dword ptr [esp + 044h] // 00c48705
        fld dword ptr [esp + 028h] // 00c48709
        fld st(0) // 00c4870d
        fchs  // 00c4870f
        fcomip st(0), st(2) // 00c48711
        jbe l_00c48719 // 00c48713
        fadd st(0), st(1) // 00c48715
        jmp l_00c48725 // 00c48717
    l_00c48719:
        fxch st(1) // 00c48719
        fcomi st(0), st(1) // 00c4871b
        jbe l_00c48739 // 00c4871d
        fld st(0) // 00c4871f
        fsubrp st(2), st(0) // 00c48721
        fxch st(1) // 00c48723
    l_00c48725:
        fstp dword ptr [esp + 038h] // 00c48725
        fld dword ptr [esp + 038h] // 00c48729
        fmul st(0), st(0) // 00c4872d
        fadd dword ptr [esp + 010h] // 00c4872f
        fstp dword ptr [esp + 010h] // 00c48733
        jmp l_00c4873b // 00c48737
    l_00c48739:
        fstp st(1) // 00c48739
    l_00c4873b:
        fld dword ptr [ecx + 0218h] // 00c4873b
        fstp dword ptr [esp + 024h] // 00c48741
        fld dword ptr [esp + 048h] // 00c48745
        fld dword ptr [esp + 024h] // 00c48749
        fld st(0) // 00c4874d
        fchs  // 00c4874f
        fcomip st(0), st(2) // 00c48751
        jbe l_00c4876d // 00c48753
        fld st(1) // 00c48755
        fadd st(0), st(1) // 00c48757
        fstp dword ptr [esp + 03ch] // 00c48759
        fld dword ptr [esp + 03ch] // 00c4875d
        fmul st(0), st(0) // 00c48761
        fadd dword ptr [esp + 010h] // 00c48763
        fstp dword ptr [esp + 010h] // 00c48767
        jmp l_00c4878b // 00c4876b
    l_00c4876d:
        fxch st(1) // 00c4876d
        fcomi st(0), st(1) // 00c4876f
        jbe l_00c48789 // 00c48771
        fld st(0) // 00c48773
        fsub st(0), st(2) // 00c48775
        fstp dword ptr [esp + 03ch] // 00c48777
        fld dword ptr [esp + 03ch] // 00c4877b
        fmul st(0), st(0) // 00c4877f
        fadd dword ptr [esp + 010h] // 00c48781
        fstp dword ptr [esp + 010h] // 00c48785
    l_00c48789:
        fxch st(1) // 00c48789
    l_00c4878b:
        fld dword ptr [esp + 02ch] // 00c4878b
        fld dword ptr [esp + 010h] // 00c4878f
        fld st(1) // 00c48793
        fmulp st(2), st(0) // 00c48795
        fcomip st(0), st(1) // 00c48797
        fstp st(0) // 00c48799
        jbe l_00c487b2 // 00c4879b
        fstp st(4) // 00c4879d
        xor al, al // 00c4879f
        fstp st(2) // 00c487a1
        fstp st(1) // 00c487a3
        fstp st(1) // 00c487a5
        fstp st(0) // 00c487a7
        pop edi // 00c487a9
        pop esi // 00c487aa
        pop ebx // 00c487ab
        mov esp, ebp // 00c487ac
        pop ebp // 00c487ae
        ret 24 // 00c487af
    l_00c487b2:
        movss xmm1, dword ptr [esp + 010h] // 00c487b2
        ucomiss xmm1, xmm0 // 00c487b8
        lahf  // 00c487bb
        test ah, 044h // 00c487bc
        jp l_00c488b5 // 00c487bf
        fxch st(3) // 00c487c5
        fsubrp st(4), st(0) // 00c487c7
        fxch st(3) // 00c487c9
        fstp dword ptr [esp + 040h] // 00c487cb
        mov eax, dword ptr [esp + 040h] // 00c487cf
        mov dword ptr [esp + 034h], eax // 00c487d3
        fsubr dword ptr [esp + 028h] // 00c487d7
        fstp dword ptr [esp + 044h] // 00c487db
        mov ecx, dword ptr [esp + 044h] // 00c487df
        push ecx // 00c487e3
        fsubrp st(1), st(0) // 00c487e4
        mov dword ptr [esp + 03ch], ecx // 00c487e6
        fstp dword ptr [esp + 04ch] // 00c487ea
        mov edx, dword ptr [esp + 04ch] // 00c487ee
        fld dword ptr [esp + 048h] // 00c487f2
        mov dword ptr [esp + 040h], edx // 00c487f6
        fstp dword ptr [esp] // 00c487fa
        call abs_kernel // 00c487fd
        fstp dword ptr [esp + 028h] // 00c48802
        push ecx // 00c48806
        fld dword ptr [esp + 04ch] // 00c48807
        fstp dword ptr [esp] // 00c4880b
        call abs_kernel // 00c4880e
        fld dword ptr [esp + 028h] // 00c48813
        lea ebx, [esp + 038h] // 00c48817
        fxch st(1) // 00c4881b
        fcomip st(0), st(1) // 00c4881d
        fstp st(0) // 00c4881f
        ja l_00c48827 // 00c48821
        lea ebx, [esp + 03ch] // 00c48823
    l_00c48827:
        fld dword ptr [esp + 040h] // 00c48827
        push ecx // 00c4882b
        fstp dword ptr [esp] // 00c4882c
        call abs_kernel // 00c4882f
        fstp dword ptr [esp + 028h] // 00c48834
        push ecx // 00c48838
        fld dword ptr [ebx] // 00c48839
        fstp dword ptr [esp] // 00c4883b
        call abs_kernel // 00c4883e
        fld dword ptr [esp + 028h] // 00c48843
        fxch st(1) // 00c48847
        fcomip st(0), st(1) // 00c48849
        fstp st(0) // 00c4884b
        jbe l_00c48859 // 00c4884d
        lea ecx, [esp + 034h] // 00c4884f
        mov dword ptr [esp + 028h], ecx // 00c48853
        jmp l_00c4885f // 00c48857
    l_00c48859:
        mov ecx, ebx // 00c48859
        mov dword ptr [esp + 028h], ebx // 00c4885b
    l_00c4885f:
        lea eax, [esp + 034h] // 00c4885f
        sub ecx, eax // 00c48863
        sar ecx, 2 // 00c48865
        lea eax, [ecx + 1] // 00c48868
        cdq  // 00c4886b
        mov ebx, 3 // 00c4886c
        idiv ebx // 00c48871
        xorps xmm0, xmm0 // 00c48873
        push ecx // 00c48876
        movss dword ptr [esp + edx*4 + 038h], xmm0 // 00c48877
        lea eax, [edx + 1] // 00c4887d
        cdq  // 00c48880
        idiv ebx // 00c48881
        movss dword ptr [esp + edx*4 + 038h], xmm0 // 00c48883
        fld dword ptr [esp + ecx*4 + 038h] // 00c48889
        fstp dword ptr [esp] // 00c4888d
        call abs_kernel // 00c48890
        mov eax, dword ptr [esp + 028h] // 00c48895
        fstp dword ptr [esp + 010h] // 00c48899
        fld dword ptr [eax] // 00c4889d
        fld dword ptr [esp + 010h] // 00c4889f
        fld st(0) // 00c488a3
        fdivp st(2), st(0) // 00c488a5
        fxch st(1) // 00c488a7
        fstp dword ptr [eax] // 00c488a9
        fld dword ptr [esp + 02ch] // 00c488ab
        fld st(0) // 00c488af
        faddp st(2), st(0) // 00c488b1
        jmp l_00c4891e // 00c488b3
    l_00c488b5:
        fstp st(4) // 00c488b5
        push ecx // 00c488b7
        fstp st(2) // 00c488b8
        fstp st(1) // 00c488ba
        fstp st(1) // 00c488bc
        fstp st(0) // 00c488be
        fld dword ptr [esp + 038h] // 00c488c0
        fld dword ptr [esp + 03ch] // 00c488c4
        fld dword ptr [esp + 040h] // 00c488c8
        fld st(1) // 00c488cc
        fmulp st(2), st(0) // 00c488ce
        fld st(2) // 00c488d0
        fmulp st(3), st(0) // 00c488d2
        fxch st(1) // 00c488d4
        faddp st(2), st(0) // 00c488d6
        fmul st(0), st(0) // 00c488d8
        faddp st(1), st(0) // 00c488da
        fstp dword ptr [esp + 02ch] // 00c488dc
        fld dword ptr [esp + 02ch] // 00c488e0
        fstp dword ptr [esp] // 00c488e4
        push dword ptr [ebp+28] // Borrowed CRT context.
        call sqrt_kernel // 00c488e7
        fstp dword ptr [esp + 010h] // 00c488ec
        fld dword ptr [esp + 034h] // 00c488f0
        fld dword ptr [esp + 010h] // 00c488f4
        fld st(0) // 00c488f8
        fdivp st(2), st(0) // 00c488fa
        fxch st(1) // 00c488fc
        fstp dword ptr [esp + 034h] // 00c488fe
        fld dword ptr [esp + 038h] // 00c48902
        fdiv st(0), st(1) // 00c48906
        fstp dword ptr [esp + 038h] // 00c48908
        fld dword ptr [esp + 03ch] // 00c4890c
        fdiv st(0), st(1) // 00c48910
        fstp dword ptr [esp + 03ch] // 00c48912
        fld dword ptr [esp + 02ch] // 00c48916
        fld st(0) // 00c4891a
        fsubrp st(2), st(0) // 00c4891c
    l_00c4891e:
        mov eax, dword ptr [ebp + 8] // 00c4891e
        fxch st(1) // 00c48921
        fstp dword ptr [esp + 010h] // 00c48923
        mov dword ptr [eax], 1 // 00c48927
        fld dword ptr [esp + 064h] // 00c4892d
        fld dword ptr [esp + 038h] // 00c48931
        fld st(0) // 00c48935
        fmulp st(2), st(0) // 00c48937
        fld dword ptr [esp + 034h] // 00c48939
        fld st(0) // 00c4893d
        fmul dword ptr [esp + 058h] // 00c4893f
        faddp st(3), st(0) // 00c48943
        fld dword ptr [esp + 070h] // 00c48945
        fld dword ptr [esp + 03ch] // 00c48949
        fld st(0) // 00c4894d
        fmulp st(2), st(0) // 00c4894f
        fxch st(4) // 00c48951
        faddp st(1), st(0) // 00c48953
        fstp dword ptr [eax + 01ch] // 00c48955
        fld dword ptr [esp + 068h] // 00c48958
        fmul st(0), st(2) // 00c4895c
        fld dword ptr [esp + 05ch] // 00c4895e
        fmul st(0), st(2) // 00c48962
        faddp st(1), st(0) // 00c48964
        fld dword ptr [esp + 074h] // 00c48966
        fmul st(0), st(4) // 00c4896a
        faddp st(1), st(0) // 00c4896c
        fstp dword ptr [eax + 020h] // 00c4896e
        fmul dword ptr [esp + 060h] // 00c48971
        fld dword ptr [esp + 06ch] // 00c48975
        fmulp st(2), st(0) // 00c48979
        faddp st(1), st(0) // 00c4897b
        fld dword ptr [esp + 078h] // 00c4897d
        fmulp st(2), st(0) // 00c48981
        faddp st(1), st(0) // 00c48983
        fstp dword ptr [eax + 024h] // 00c48985
        fld dword ptr [esp + 010h] // 00c48988
        fmul qword ptr constant_00d7a280 // 00c4898c
        fld st(0) // 00c48992
        fsubrp st(2), st(0) // 00c48994
        fxch st(1) // 00c48996
        fstp dword ptr [esp + 02ch] // 00c48998
        fld dword ptr [eax + 01ch] // 00c4899c
        fld dword ptr [esp + 02ch] // 00c4899f
        fld st(0) // 00c489a3
        fmulp st(2), st(0) // 00c489a5
        fxch st(1) // 00c489a7
        fstp dword ptr [esp + 034h] // 00c489a9
        fld dword ptr [eax + 020h] // 00c489ad
        fmul st(0), st(1) // 00c489b0
        fstp dword ptr [esp + 038h] // 00c489b2
        fmul dword ptr [eax + 024h] // 00c489b6
        fstp dword ptr [esp + 03ch] // 00c489b9
        fld dword ptr [esp + 034h] // 00c489bd
        fadd dword ptr [esp + 04ch] // 00c489c1
        fstp dword ptr [esp + 040h] // 00c489c5
        fld dword ptr [esp + 038h] // 00c489c9
        fadd dword ptr [esp + 050h] // 00c489cd
        fstp dword ptr [esp + 044h] // 00c489d1
        fld dword ptr [esp + 03ch] // 00c489d5
        fadd dword ptr [esp + 054h] // 00c489d9
        fstp dword ptr [esp + 048h] // 00c489dd
        fstp dword ptr [esp + 02ch] // 00c489e1
        fld dword ptr [esp + 02ch] // 00c489e5
        fst dword ptr [esp + 02ch] // 00c489e9
        fld dword ptr [eax + 01ch] // 00c489ed
        fld dword ptr [esp + 02ch] // 00c489f0
        fld st(0) // 00c489f4
        fmulp st(2), st(0) // 00c489f6
        fxch st(1) // 00c489f8
        fstp dword ptr [esp + 04ch] // 00c489fa
        fld dword ptr [eax + 020h] // 00c489fe
        fmul st(0), st(1) // 00c48a01
        fstp dword ptr [esp + 050h] // 00c48a03
        fmul dword ptr [eax + 024h] // 00c48a07
        fstp dword ptr [esp + 054h] // 00c48a0a
        fld dword ptr [esp + 04ch] // 00c48a0e
        fld dword ptr [esp + 040h] // 00c48a12
        fld st(0) // 00c48a16
        faddp st(2), st(0) // 00c48a18
        fxch st(1) // 00c48a1a
        fstp dword ptr [esp + 034h] // 00c48a1c
        fld dword ptr [esp + 050h] // 00c48a20
        fadd dword ptr [esp + 044h] // 00c48a24
        fstp dword ptr [esp + 038h] // 00c48a28
        fld dword ptr [esp + 054h] // 00c48a2c
        fadd dword ptr [esp + 048h] // 00c48a30
        fstp dword ptr [esp + 03ch] // 00c48a34
        fld dword ptr [esp + 034h] // 00c48a38
        fsub dword ptr [esi + 024h] // 00c48a3c
        fstp dword ptr [esp + 04ch] // 00c48a3f
        fld dword ptr [esp + 038h] // 00c48a43
        fsub dword ptr [esi + 028h] // 00c48a47
        fstp dword ptr [esp + 050h] // 00c48a4a
        fld dword ptr [esp + 03ch] // 00c48a4e
        fsub dword ptr [esi + 02ch] // 00c48a52
        fstp dword ptr [esp + 054h] // 00c48a55
        fld dword ptr [esp + 04ch] // 00c48a59
        fld st(0) // 00c48a5d
        fmul dword ptr [esi] // 00c48a5f
        fld dword ptr [esp + 050h] // 00c48a61
        fld st(0) // 00c48a65
        fmul dword ptr [esi + 4] // 00c48a67
        faddp st(2), st(0) // 00c48a6a
        fld dword ptr [esi + 8] // 00c48a6c
        fld dword ptr [esp + 054h] // 00c48a6f
        fld st(0) // 00c48a73
        fmulp st(2), st(0) // 00c48a75
        fxch st(3) // 00c48a77
        faddp st(1), st(0) // 00c48a79
        fstp dword ptr [eax + 4] // 00c48a7b
        fld dword ptr [esi + 010h] // 00c48a7e
        fmul st(0), st(1) // 00c48a81
        fld dword ptr [esi + 0ch] // 00c48a83
        fmul st(0), st(4) // 00c48a86
        faddp st(1), st(0) // 00c48a88
        fld dword ptr [esi + 014h] // 00c48a8a
        fmul st(0), st(3) // 00c48a8d
        faddp st(1), st(0) // 00c48a8f
        fstp dword ptr [eax + 8] // 00c48a91
        fmul dword ptr [esi + 01ch] // 00c48a94
        fld dword ptr [esi + 018h] // 00c48a97
        fmulp st(3), st(0) // 00c48a9a
        faddp st(2), st(0) // 00c48a9c
        fmul dword ptr [esi + 020h] // 00c48a9e
        faddp st(1), st(0) // 00c48aa1
        fstp dword ptr [eax + 0ch] // 00c48aa3
        fxch st(1) // 00c48aa6
        fstp dword ptr [esp + 02ch] // 00c48aa8
        fld dword ptr [eax + 01ch] // 00c48aac
        fld dword ptr [esp + 02ch] // 00c48aaf
        fld st(0) // 00c48ab3
        fmulp st(2), st(0) // 00c48ab5
        fxch st(1) // 00c48ab7
        fstp dword ptr [esp + 04ch] // 00c48ab9
        fld dword ptr [eax + 020h] // 00c48abd
        fmul st(0), st(1) // 00c48ac0
        fstp dword ptr [esp + 050h] // 00c48ac2
        fmul dword ptr [eax + 024h] // 00c48ac6
        fstp dword ptr [esp + 054h] // 00c48ac9
        fsub dword ptr [esp + 04ch] // 00c48acd
        fstp dword ptr [esp + 034h] // 00c48ad1
        fld dword ptr [esp + 044h] // 00c48ad5
        fsub dword ptr [esp + 050h] // 00c48ad9
        fstp dword ptr [esp + 038h] // 00c48add
        fld dword ptr [esp + 048h] // 00c48ae1
        fsub dword ptr [esp + 054h] // 00c48ae5
        fstp dword ptr [esp + 03ch] // 00c48ae9
        fld dword ptr [esp + 034h] // 00c48aed
        fsub dword ptr [edi + 024h] // 00c48af1
        fstp dword ptr [esp + 04ch] // 00c48af4
        fld dword ptr [esp + 038h] // 00c48af8
        fsub dword ptr [edi + 028h] // 00c48afc
        fstp dword ptr [esp + 050h] // 00c48aff
        fld dword ptr [esp + 03ch] // 00c48b03
        fsub dword ptr [edi + 02ch] // 00c48b07
        fstp dword ptr [esp + 054h] // 00c48b0a
        fld dword ptr [esp + 04ch] // 00c48b0e
        fld st(0) // 00c48b12
        fmul dword ptr [edi] // 00c48b14
        fld dword ptr [esp + 050h] // 00c48b16
        fld st(0) // 00c48b1a
        cmp dword ptr [esp + 030h], 0 // 00c48b1c
        fmul dword ptr [edi + 4] // 00c48b21
        faddp st(2), st(0) // 00c48b24
        fld dword ptr [edi + 8] // 00c48b26
        fld dword ptr [esp + 054h] // 00c48b29
        fld st(0) // 00c48b2d
        fmulp st(2), st(0) // 00c48b2f
        fxch st(3) // 00c48b31
        faddp st(1), st(0) // 00c48b33
        fstp dword ptr [eax + 010h] // 00c48b35
        fld dword ptr [edi + 010h] // 00c48b38
        fmul st(0), st(1) // 00c48b3b
        fld st(3) // 00c48b3d
        fmul dword ptr [edi + 0ch] // 00c48b3f
        faddp st(1), st(0) // 00c48b42
        fld dword ptr [edi + 014h] // 00c48b44
        fmul st(0), st(3) // 00c48b47
        faddp st(1), st(0) // 00c48b49
        fstp dword ptr [eax + 014h] // 00c48b4b
        fmul dword ptr [edi + 01ch] // 00c48b4e
        fld dword ptr [edi + 018h] // 00c48b51
        fmulp st(3), st(0) // 00c48b54
        faddp st(2), st(0) // 00c48b56
        fmul dword ptr [edi + 020h] // 00c48b58
        faddp st(1), st(0) // 00c48b5b
        fstp dword ptr [eax + 018h] // 00c48b5d
        je l_00c48bca // 00c48b60
        movss xmm0, dword ptr constant_00d7a208 // 00c48b62
        mov esi, dword ptr [eax + 0ch] // 00c48b6a
        movaps xmm1, xmm0 // 00c48b6d
        subss xmm1, dword ptr [eax + 01ch] // 00c48b70
        movss dword ptr [esp + 04ch], xmm1 // 00c48b75
        mov ecx, dword ptr [esp + 04ch] // 00c48b7b
        movaps xmm1, xmm0 // 00c48b7f
        subss xmm1, dword ptr [eax + 020h] // 00c48b82
        subss xmm0, dword ptr [eax + 024h] // 00c48b87
        mov dword ptr [eax + 01ch], ecx // 00c48b8c
        movss dword ptr [esp + 050h], xmm1 // 00c48b8f
        mov edx, dword ptr [esp + 050h] // 00c48b95
        mov dword ptr [eax + 020h], edx // 00c48b99
        mov edx, dword ptr [eax + 8] // 00c48b9c
        movss dword ptr [esp + 054h], xmm0 // 00c48b9f
        mov ecx, dword ptr [esp + 054h] // 00c48ba5
        mov dword ptr [eax + 024h], ecx // 00c48ba9
        mov edi, dword ptr [eax + 010h] // 00c48bac
        mov ecx, dword ptr [eax + 4] // 00c48baf
        mov dword ptr [eax + 4], edi // 00c48bb2
        mov edi, dword ptr [eax + 014h] // 00c48bb5
        mov dword ptr [eax + 8], edi // 00c48bb8
        mov edi, dword ptr [eax + 018h] // 00c48bbb
        mov dword ptr [eax + 0ch], edi // 00c48bbe
        mov dword ptr [eax + 010h], ecx // 00c48bc1
        mov dword ptr [eax + 014h], edx // 00c48bc4
        mov dword ptr [eax + 018h], esi // 00c48bc7
    l_00c48bca:
        pop edi // 00c48bca
        pop esi // 00c48bcb
        mov al, 1 // 00c48bcc
        pop ebx // 00c48bce
        mov esp, ebp // 00c48bcf
        pop ebp // 00c48bd1
        ret 24 // 00c48bd2
    }
}
// Complete original instruction schedule; address comments are native starts.
__declspec(naked) void sphere_ray_kernel(){
    __asm {
        push ebp // 00c50740
        mov ebp, esp // 00c50741
        and esp, 0fffffff8h // 00c50743
        sub esp, 074h // 00c50746
        mov ecx, dword ptr [ebp + 0ch] // 00c50749
        mov eax, dword ptr [ecx + 4] // 00c5074c
        fld dword ptr [ecx + 038h] // 00c5074f
        fstp dword ptr [esp + 018h] // 00c50752
        add eax, 8 // 00c50756
        fld dword ptr [eax + 0ch] // 00c50759
        push ebx // 00c5075c
        fstp dword ptr [esp + 010h] // 00c5075d
        push esi // 00c50761
        fld dword ptr [eax] // 00c50762
        push edi // 00c50764
        fstp dword ptr [esp + 01ch] // 00c50765
        fld dword ptr [ecx + 034h] // 00c50769
        fstp dword ptr [esp + 028h] // 00c5076c
        fld dword ptr [ecx + 03ch] // 00c50770
        fstp dword ptr [esp + 020h] // 00c50773
        fld dword ptr [eax + 018h] // 00c50777
        fstp dword ptr [esp + 014h] // 00c5077a
        fld dword ptr [esp + 028h] // 00c5077e
        fld st(0) // 00c50782
        fld dword ptr [esp + 01ch] // 00c50784
        fld st(0) // 00c50788
        fmulp st(2), st(0) // 00c5078a
        fld dword ptr [esp + 018h] // 00c5078c
        fld st(0) // 00c50790
        fld dword ptr [esp + 024h] // 00c50792
        fld st(0) // 00c50796
        fmulp st(2), st(0) // 00c50798
        fxch st(4) // 00c5079a
        faddp st(1), st(0) // 00c5079c
        fld dword ptr [esp + 014h] // 00c5079e
        fld st(0) // 00c507a2
        fmul dword ptr [esp + 020h] // 00c507a4
        faddp st(2), st(0) // 00c507a8
        fxch st(1) // 00c507aa
        fstp dword ptr [esp + 050h] // 00c507ac
        fld dword ptr [eax + 010h] // 00c507b0
        fstp dword ptr [esp + 018h] // 00c507b3
        fld dword ptr [eax + 4] // 00c507b7
        fstp dword ptr [esp + 01ch] // 00c507ba
        fld dword ptr [eax + 01ch] // 00c507be
        fstp dword ptr [esp + 010h] // 00c507c1
        fld dword ptr [esp + 01ch] // 00c507c5
        fld st(0) // 00c507c9
        fmulp st(6), st(0) // 00c507cb
        fld dword ptr [esp + 018h] // 00c507cd
        fld st(0) // 00c507d1
        fmulp st(6), st(0) // 00c507d3
        fxch st(6) // 00c507d5
        faddp st(5), st(0) // 00c507d7
        fld dword ptr [esp + 010h] // 00c507d9
        fmul dword ptr [esp + 020h] // 00c507dd
        faddp st(5), st(0) // 00c507e1
        fxch st(4) // 00c507e3
        fstp dword ptr [esp + 054h] // 00c507e5
        fld dword ptr [eax + 014h] // 00c507e9
        fstp dword ptr [esp + 018h] // 00c507ec
        fld dword ptr [eax + 8] // 00c507f0
        fstp dword ptr [esp + 014h] // 00c507f3
        fld dword ptr [eax + 020h] // 00c507f7
        fstp dword ptr [esp + 01ch] // 00c507fa
        fld dword ptr [esp + 014h] // 00c507fe
        fmul dword ptr [esp + 028h] // 00c50802
        fld dword ptr [esp + 018h] // 00c50806
        fmul dword ptr [esp + 024h] // 00c5080a
        faddp st(1), st(0) // 00c5080e
        fld dword ptr [esp + 01ch] // 00c50810
        fmul dword ptr [esp + 020h] // 00c50814
        faddp st(1), st(0) // 00c50818
        fstp dword ptr [esp + 058h] // 00c5081a
        fld dword ptr [ecx + 044h] // 00c5081e
        fstp dword ptr [esp + 024h] // 00c50821
        fld dword ptr [ecx + 040h] // 00c50825
        fstp dword ptr [esp + 020h] // 00c50828
        fld dword ptr [ecx + 048h] // 00c5082c
        fstp dword ptr [esp + 028h] // 00c5082f
        fld dword ptr [esp + 020h] // 00c50833
        fmul st(0), st(3) // 00c50837
        fld dword ptr [esp + 024h] // 00c50839
        fmul st(0), st(3) // 00c5083d
        faddp st(1), st(0) // 00c5083f
        fld dword ptr [esp + 028h] // 00c50841
        fmul st(0), st(2) // 00c50845
        faddp st(1), st(0) // 00c50847
        fstp dword ptr [esp + 05ch] // 00c50849
        fld dword ptr [esp + 020h] // 00c5084d
        fmul st(0), st(4) // 00c50851
        fld dword ptr [esp + 024h] // 00c50853
        fmul st(0), st(6) // 00c50857
        faddp st(1), st(0) // 00c50859
        fld dword ptr [esp + 028h] // 00c5085b
        fmul dword ptr [esp + 010h] // 00c5085f
        faddp st(1), st(0) // 00c50863
        fstp dword ptr [esp + 060h] // 00c50865
        fld dword ptr [esp + 020h] // 00c50869
        fmul dword ptr [esp + 014h] // 00c5086d
        fld dword ptr [esp + 024h] // 00c50871
        fmul dword ptr [esp + 018h] // 00c50875
        faddp st(1), st(0) // 00c50879
        fld dword ptr [esp + 028h] // 00c5087b
        fmul dword ptr [esp + 01ch] // 00c5087f
        faddp st(1), st(0) // 00c50883
        fstp dword ptr [esp + 064h] // 00c50885
        fld dword ptr [ecx + 050h] // 00c50889
        fstp dword ptr [esp + 024h] // 00c5088c
        fld dword ptr [ecx + 04ch] // 00c50890
        fstp dword ptr [esp + 028h] // 00c50893
        fld dword ptr [ecx + 054h] // 00c50897
        fstp dword ptr [esp + 020h] // 00c5089a
        fld dword ptr [esp + 028h] // 00c5089e
        fmul st(0), st(3) // 00c508a2
        fld dword ptr [esp + 024h] // 00c508a4
        fmul st(0), st(3) // 00c508a8
        faddp st(1), st(0) // 00c508aa
        fld dword ptr [esp + 020h] // 00c508ac
        fmul st(0), st(2) // 00c508b0
        faddp st(1), st(0) // 00c508b2
        fstp dword ptr [esp + 068h] // 00c508b4
        fld dword ptr [esp + 028h] // 00c508b8
        fmul st(0), st(4) // 00c508bc
        fld dword ptr [esp + 024h] // 00c508be
        fmul st(0), st(6) // 00c508c2
        faddp st(1), st(0) // 00c508c4
        fld dword ptr [esp + 020h] // 00c508c6
        fmul dword ptr [esp + 010h] // 00c508ca
        faddp st(1), st(0) // 00c508ce
        fstp dword ptr [esp + 06ch] // 00c508d0
        fld dword ptr [esp + 028h] // 00c508d4
        fmul dword ptr [esp + 014h] // 00c508d8
        fld dword ptr [esp + 024h] // 00c508dc
        fmul dword ptr [esp + 018h] // 00c508e0
        faddp st(1), st(0) // 00c508e4
        fld dword ptr [esp + 020h] // 00c508e6
        fmul dword ptr [esp + 01ch] // 00c508ea
        faddp st(1), st(0) // 00c508ee
        fstp dword ptr [esp + 070h] // 00c508f0
        fld dword ptr [ecx + 05ch] // 00c508f4
        fstp dword ptr [esp + 024h] // 00c508f7
        fld dword ptr [ecx + 058h] // 00c508fb
        fstp dword ptr [esp + 028h] // 00c508fe
        fld dword ptr [ecx + 060h] // 00c50902
        fstp dword ptr [esp + 020h] // 00c50905
        fld dword ptr [esp + 028h] // 00c50909
        fld st(0) // 00c5090d
        fmulp st(4), st(0) // 00c5090f
        fld dword ptr [esp + 024h] // 00c50911
        fld st(0) // 00c50915
        fmulp st(4), st(0) // 00c50917
        fxch st(4) // 00c50919
        faddp st(3), st(0) // 00c5091b
        fld dword ptr [esp + 020h] // 00c5091d
        fld st(0) // 00c50921
        fmulp st(3), st(0) // 00c50923
        fxch st(3) // 00c50925
        faddp st(2), st(0) // 00c50927
        fld dword ptr [eax + 024h] // 00c50929
        faddp st(2), st(0) // 00c5092c
        fxch st(1) // 00c5092e
        fstp dword ptr [esp + 074h] // 00c50930
        fld st(0) // 00c50934
        fmulp st(4), st(0) // 00c50936
        fld st(2) // 00c50938
        fmulp st(5), st(0) // 00c5093a
        mov edi, dword ptr [ebp + 010h] // 00c5093c
        fxch st(3) // 00c5093f
        mov ebx, dword ptr [ebp + 014h] // 00c50941
        faddp st(4), st(0) // 00c50944
        fld st(0) // 00c50946
        fmul dword ptr [esp + 010h] // 00c50948
        faddp st(4), st(0) // 00c5094c
        fld dword ptr [eax + 028h] // 00c5094e
        faddp st(4), st(0) // 00c50951
        fxch st(3) // 00c50953
        fstp dword ptr [esp + 078h] // 00c50955
        fld dword ptr [esp + 014h] // 00c50959
        fmulp st(2), st(0) // 00c5095d
        fmul dword ptr [esp + 018h] // 00c5095f
        faddp st(1), st(0) // 00c50963
        fld dword ptr [esp + 01ch] // 00c50965
        fmulp st(2), st(0) // 00c50969
        faddp st(1), st(0) // 00c5096b
        fadd dword ptr [eax + 02ch] // 00c5096d
        fstp dword ptr [esp + 07ch] // 00c50970
        fld dword ptr [edi] // 00c50974
        fstp dword ptr [esp + 018h] // 00c50976
        fld dword ptr [esp + 018h] // 00c5097a
        fld dword ptr [esp + 074h] // 00c5097e
        fld st(0) // 00c50982
        fsubp st(2), st(0) // 00c50984
        fxch st(1) // 00c50986
        fstp dword ptr [esp + 02ch] // 00c50988
        fld dword ptr [edi + 4] // 00c5098c
        fld dword ptr [esp + 078h] // 00c5098f
        fld st(0) // 00c50993
        fsubp st(2), st(0) // 00c50995
        fxch st(1) // 00c50997
        fstp dword ptr [esp + 030h] // 00c50999
        fld dword ptr [edi + 8] // 00c5099d
        fsub dword ptr [esp + 07ch] // 00c509a0
        fstp dword ptr [esp + 034h] // 00c509a4
        fld dword ptr [esp + 02ch] // 00c509a8
        fld st(0) // 00c509ac
        fmul dword ptr [esp + 050h] // 00c509ae
        fld dword ptr [esp + 030h] // 00c509b2
        fld st(0) // 00c509b6
        fmul dword ptr [esp + 054h] // 00c509b8
        faddp st(2), st(0) // 00c509bc
        fld dword ptr [esp + 034h] // 00c509be
        fld st(0) // 00c509c2
        fmul dword ptr [esp + 058h] // 00c509c4
        faddp st(3), st(0) // 00c509c8
        fxch st(2) // 00c509ca
        fstp dword ptr [esp + 044h] // 00c509cc
        fld dword ptr [esp + 05ch] // 00c509d0
        fmul st(0), st(3) // 00c509d4
        fld st(1) // 00c509d6
        fmul dword ptr [esp + 060h] // 00c509d8
        faddp st(1), st(0) // 00c509dc
        fld st(2) // 00c509de
        fmul dword ptr [esp + 064h] // 00c509e0
        faddp st(1), st(0) // 00c509e4
        fstp dword ptr [esp + 048h] // 00c509e6
        fld dword ptr [esp + 068h] // 00c509ea
        fld st(0) // 00c509ee
        fmulp st(4), st(0) // 00c509f0
        fld dword ptr [esp + 06ch] // 00c509f2
        fld st(0) // 00c509f6
        fmulp st(3), st(0) // 00c509f8
        fxch st(4) // 00c509fa
        faddp st(2), st(0) // 00c509fc
        fld dword ptr [esp + 070h] // 00c509fe
        fmulp st(3), st(0) // 00c50a02
        fxch st(1) // 00c50a04
        faddp st(2), st(0) // 00c50a06
        fxch st(1) // 00c50a08
        fstp dword ptr [esp + 04ch] // 00c50a0a
        fld dword ptr [ebx] // 00c50a0e
        fstp dword ptr [esp + 01ch] // 00c50a10
        fld dword ptr [esp + 01ch] // 00c50a14
        fsubrp st(4), st(0) // 00c50a18
        fxch st(3) // 00c50a1a
        fstp dword ptr [esp + 02ch] // 00c50a1c
        fld dword ptr [ebx + 4] // 00c50a20
        fstp dword ptr [esp + 014h] // 00c50a23
        fld dword ptr [esp + 014h] // 00c50a27
        fsubrp st(2), st(0) // 00c50a2b
        fxch st(1) // 00c50a2d
        fstp dword ptr [esp + 030h] // 00c50a2f
        fld dword ptr [ebx + 8] // 00c50a33
        fstp dword ptr [esp + 010h] // 00c50a36
        fld dword ptr [esp + 010h] // 00c50a3a
        fsub dword ptr [esp + 07ch] // 00c50a3e
        fstp dword ptr [esp + 034h] // 00c50a42
        fld dword ptr [esp + 02ch] // 00c50a46
        fld st(0) // 00c50a4a
        fmul dword ptr [esp + 050h] // 00c50a4c
        fld dword ptr [esp + 030h] // 00c50a50
        fld st(0) // 00c50a54
        fmul dword ptr [esp + 054h] // 00c50a56
        faddp st(2), st(0) // 00c50a5a
        fld dword ptr [esp + 034h] // 00c50a5c
        fld st(0) // 00c50a60
        fmul dword ptr [esp + 058h] // 00c50a62
        faddp st(3), st(0) // 00c50a66
        fxch st(2) // 00c50a68
        fstp dword ptr [esp + 038h] // 00c50a6a
        fld dword ptr [esp + 05ch] // 00c50a6e
        fmul st(0), st(3) // 00c50a72
        fld st(1) // 00c50a74
        fmul dword ptr [esp + 060h] // 00c50a76
        faddp st(1), st(0) // 00c50a7a
        fld st(2) // 00c50a7c
        fmul dword ptr [esp + 064h] // 00c50a7e
        faddp st(1), st(0) // 00c50a82
        fstp dword ptr [esp + 03ch] // 00c50a84
        fxch st(4) // 00c50a88
        fmulp st(2), st(0) // 00c50a8a
        fxch st(3) // 00c50a8c
        fmulp st(2), st(0) // 00c50a8e
        faddp st(1), st(0) // 00c50a90
        fld dword ptr [esp + 070h] // 00c50a92
        fmulp st(2), st(0) // 00c50a96
        faddp st(1), st(0) // 00c50a98
        fstp dword ptr [esp + 040h] // 00c50a9a
        fld dword ptr [esp + 038h] // 00c50a9e
        fld dword ptr [esp + 044h] // 00c50aa2
        fld st(0) // 00c50aa6
        fsubp st(2), st(0) // 00c50aa8
        fxch st(1) // 00c50aaa
        fstp dword ptr [esp + 02ch] // 00c50aac
        fld dword ptr [esp + 03ch] // 00c50ab0
        fld dword ptr [esp + 048h] // 00c50ab4
        fld st(0) // 00c50ab8
        fsubp st(2), st(0) // 00c50aba
        fxch st(1) // 00c50abc
        fstp dword ptr [esp + 030h] // 00c50abe
        fld dword ptr [esp + 040h] // 00c50ac2
        fld dword ptr [esp + 04ch] // 00c50ac6
        fld st(0) // 00c50aca
        fsubp st(2), st(0) // 00c50acc
        fxch st(1) // 00c50ace
        fstp dword ptr [esp + 034h] // 00c50ad0
        fld dword ptr [esp + 02ch] // 00c50ad4
        fld st(0) // 00c50ad8
        fld dword ptr [esp + 030h] // 00c50ada
        fld dword ptr [esp + 034h] // 00c50ade
        fld st(1) // 00c50ae2
        fmulp st(2), st(0) // 00c50ae4
        fld st(2) // 00c50ae6
        fmulp st(3), st(0) // 00c50ae8
        fxch st(1) // 00c50aea
        faddp st(2), st(0) // 00c50aec
        fmul st(0), st(0) // 00c50aee
        faddp st(1), st(0) // 00c50af0
        fstp dword ptr [esp + 024h] // 00c50af2
        fmul st(0), st(3) // 00c50af6
        fld dword ptr [esp + 030h] // 00c50af8
        fmul st(0), st(3) // 00c50afc
        faddp st(1), st(0) // 00c50afe
        fld dword ptr [esp + 034h] // 00c50b00
        fmul st(0), st(2) // 00c50b04
        faddp st(1), st(0) // 00c50b06
        fstp dword ptr [esp + 028h] // 00c50b08
        fld dword ptr [esp + 028h] // 00c50b0c
        fadd st(0), st(0) // 00c50b10
        fstp dword ptr [esp + 020h] // 00c50b12
        fld dword ptr [esp + 020h] // 00c50b16
        fld dword ptr [ecx + 0210h] // 00c50b1a
        fld st(1) // 00c50b20
        fmulp st(2), st(0) // 00c50b22
        fld st(3) // 00c50b24
        fmulp st(4), st(0) // 00c50b26
        fld st(4) // 00c50b28
        fmulp st(5), st(0) // 00c50b2a
        fxch st(3) // 00c50b2c
        faddp st(4), st(0) // 00c50b2e
        fld st(1) // 00c50b30
        fmulp st(2), st(0) // 00c50b32
        fxch st(3) // 00c50b34
        faddp st(1), st(0) // 00c50b36
        fstp dword ptr [esp + 028h] // 00c50b38
        fld st(0) // 00c50b3c
        fmulp st(1), st(0) // 00c50b3e
        fsubr dword ptr [esp + 028h] // 00c50b40
        fstp dword ptr [esp + 028h] // 00c50b44
        fld dword ptr [esp + 028h] // 00c50b48
        fld dword ptr [esp + 024h] // 00c50b4c
        fmul qword ptr constant_00d7a328 // 00c50b50
        fmulp st(1), st(0) // 00c50b56
        fsubp st(1), st(0) // 00c50b58
        fstp dword ptr [esp + 028h] // 00c50b5a
        fld dword ptr [esp + 028h] // 00c50b5e
        fldz  // 00c50b62
        fcomip st(0), st(1) // 00c50b64
        ja l_00c50ba5 // 00c50b66
        push ecx // 00c50b68
        fstp dword ptr [esp] // 00c50b69
        push dword ptr [ebp+24] // Borrowed CRT context.
        call sqrt_kernel // 00c50b6c
        fstp dword ptr [esp + 028h] // 00c50b71
        fld dword ptr [esp + 024h] // 00c50b75
        fadd st(0), st(0) // 00c50b79
        fld dword ptr [esp + 020h] // 00c50b7b
        fld st(0) // 00c50b7f
        fchs  // 00c50b81
        fld dword ptr [esp + 028h] // 00c50b83
        fld st(0) // 00c50b87
        fsubp st(2), st(0) // 00c50b89
        fxch st(1) // 00c50b8b
        fdiv st(0), st(3) // 00c50b8d
        fstp dword ptr [esp + 028h] // 00c50b8f
        fld1  // 00c50b93
        fld dword ptr [esp + 028h] // 00c50b95
        fcomi st(0), st(1) // 00c50b99
        fstp st(1) // 00c50b9b
        jbe l_00c50bb2 // 00c50b9d
        fstp st(0) // 00c50b9f
        fstp st(2) // 00c50ba1
        fstp st(1) // 00c50ba3
    l_00c50ba5:
        fstp st(0) // 00c50ba5
    l_00c50ba7:
        xor al, al // 00c50ba7
        pop edi // 00c50ba9
        pop esi // 00c50baa
        pop ebx // 00c50bab
        mov esp, ebp // 00c50bac
        pop ebp // 00c50bae
        ret 20 // 00c50baf
    l_00c50bb2:
        xorps xmm0, xmm0 // 00c50bb2
        comiss xmm0, dword ptr [esp + 028h] // 00c50bb5
        jbe l_00c50c8c // 00c50bba
        fstp st(0) // 00c50bc0
        fsubrp st(1), st(0) // 00c50bc2
        fdivrp st(1), st(0) // 00c50bc4
        fstp dword ptr [esp + 028h] // 00c50bc6
        fld dword ptr [esp + 028h] // 00c50bca
        fldz  // 00c50bce
        fcomip st(0), st(1) // 00c50bd0
        fstp st(0) // 00c50bd2
        ja l_00c50ba7 // 00c50bd4
        mov esi, dword ptr [ebp + 8] // 00c50bd6
        mov eax, dword ptr [edi] // 00c50bd9
        mov dword ptr [esi], eax // 00c50bdb
        mov ecx, dword ptr [edi + 4] // 00c50bdd
        mov dword ptr [esi + 4], ecx // 00c50be0
        mov edx, dword ptr [edi + 8] // 00c50be3
        mov dword ptr [esi + 8], edx // 00c50be6
        fld dword ptr [ebx] // 00c50be9
        fsub dword ptr [edi] // 00c50beb
        fstp dword ptr [esp + 044h] // 00c50bed
        mov eax, dword ptr [esp + 044h] // 00c50bf1
        fld dword ptr [ebx + 4] // 00c50bf5
        fsub dword ptr [edi + 4] // 00c50bf8
        fstp dword ptr [esp + 048h] // 00c50bfb
        mov ecx, dword ptr [esp + 048h] // 00c50bff
        fld dword ptr [ebx + 8] // 00c50c03
        push ecx // 00c50c06
        fsub dword ptr [edi + 8] // 00c50c07
        mov dword ptr [esi + 0ch], eax // 00c50c0a
        mov dword ptr [esi + 010h], ecx // 00c50c0d
        fstp dword ptr [esp + 050h] // 00c50c10
        mov edx, dword ptr [esp + 050h] // 00c50c14
        mov dword ptr [esi + 014h], edx // 00c50c18
        fld dword ptr [esi + 010h] // 00c50c1b
        fstp dword ptr [esp + 028h] // 00c50c1e
        fld dword ptr [esp + 028h] // 00c50c22
        fld dword ptr [esi + 0ch] // 00c50c26
        fstp dword ptr [esp + 02ch] // 00c50c29
        fld dword ptr [esp + 02ch] // 00c50c2d
        fld dword ptr [esi + 014h] // 00c50c31
        fstp dword ptr [esp + 024h] // 00c50c34
        fld dword ptr [esp + 024h] // 00c50c38
        fld st(1) // 00c50c3c
        fmulp st(2), st(0) // 00c50c3e
        fld st(2) // 00c50c40
        fmulp st(3), st(0) // 00c50c42
        fxch st(1) // 00c50c44
        faddp st(2), st(0) // 00c50c46
        fmul st(0), st(0) // 00c50c48
        faddp st(1), st(0) // 00c50c4a
        fstp dword ptr [esp + 020h] // 00c50c4c
        fld dword ptr [esp + 020h] // 00c50c50
        fstp dword ptr [esp] // 00c50c54
        push dword ptr [ebp+24] // Borrowed CRT context.
        call sqrt_kernel // 00c50c57
        fstp dword ptr [esp + 01ch] // 00c50c5c
        mov al, 1 // 00c50c60
        fld dword ptr [esp + 028h] // 00c50c62
        fld dword ptr [esp + 01ch] // 00c50c66
        fld st(0) // 00c50c6a
        fdivp st(2), st(0) // 00c50c6c
        fxch st(1) // 00c50c6e
        fstp dword ptr [esi + 0ch] // 00c50c70
        fld dword ptr [esp + 024h] // 00c50c73
        fdiv st(0), st(1) // 00c50c77
        fstp dword ptr [esi + 010h] // 00c50c79
        fdivr dword ptr [esp + 020h] // 00c50c7c
        fstp dword ptr [esi + 014h] // 00c50c80
        pop edi // 00c50c83
        pop esi // 00c50c84
        pop ebx // 00c50c85
        mov esp, ebp // 00c50c86
        pop ebp // 00c50c88
        ret 20 // 00c50c89
    l_00c50c8c:
        fstp st(3) // 00c50c8c
        mov esi, dword ptr [ebp + 8] // 00c50c8e
        fstp st(0) // 00c50c91
        fstp st(0) // 00c50c93
        fld dword ptr [esp + 01ch] // 00c50c95
        fld dword ptr [esp + 018h] // 00c50c99
        fld st(0) // 00c50c9d
        fsubp st(2), st(0) // 00c50c9f
        fxch st(1) // 00c50ca1
        fstp dword ptr [esp + 044h] // 00c50ca3
        fld dword ptr [edi + 4] // 00c50ca7
        fstp dword ptr [esp + 028h] // 00c50caa
        fld dword ptr [esp + 014h] // 00c50cae
        fld dword ptr [esp + 028h] // 00c50cb2
        fld st(0) // 00c50cb6
        fsubp st(2), st(0) // 00c50cb8
        fxch st(1) // 00c50cba
        fstp dword ptr [esp + 048h] // 00c50cbc
        fld dword ptr [edi + 8] // 00c50cc0
        fstp dword ptr [esp + 028h] // 00c50cc3
        fld dword ptr [esp + 010h] // 00c50cc7
        fld dword ptr [esp + 028h] // 00c50ccb
        fld st(0) // 00c50ccf
        fsubp st(2), st(0) // 00c50cd1
        fxch st(1) // 00c50cd3
        fstp dword ptr [esp + 04ch] // 00c50cd5
        fld dword ptr [esp + 044h] // 00c50cd9
        fmul st(0), st(4) // 00c50cdd
        fstp dword ptr [esp + 038h] // 00c50cdf
        fld dword ptr [esp + 048h] // 00c50ce3
        fmul st(0), st(4) // 00c50ce7
        fstp dword ptr [esp + 03ch] // 00c50ce9
        fld dword ptr [esp + 04ch] // 00c50ced
        fmulp st(4), st(0) // 00c50cf1
        fxch st(3) // 00c50cf3
        fstp dword ptr [esp + 040h] // 00c50cf5
        fld dword ptr [esp + 038h] // 00c50cf9
        faddp st(2), st(0) // 00c50cfd
        fxch st(1) // 00c50cff
        fstp dword ptr [esp + 044h] // 00c50d01
        mov eax, dword ptr [esp + 044h] // 00c50d05
        mov dword ptr [esi], eax // 00c50d09
        fadd dword ptr [esp + 03ch] // 00c50d0b
        fstp dword ptr [esp + 048h] // 00c50d0f
        mov ecx, dword ptr [esp + 048h] // 00c50d13
        mov dword ptr [esi + 4], ecx // 00c50d17
        fadd dword ptr [esp + 040h] // 00c50d1a
        fstp dword ptr [esp + 04ch] // 00c50d1e
        mov edx, dword ptr [esp + 04ch] // 00c50d22
        mov dword ptr [esi + 8], edx // 00c50d26
        fld dword ptr [esi] // 00c50d29
        fsub dword ptr [esp + 074h] // 00c50d2b
        fstp dword ptr [esp + 044h] // 00c50d2f
        mov eax, dword ptr [esp + 044h] // 00c50d33
        fld dword ptr [esi + 4] // 00c50d37
        mov dword ptr [esi + 0ch], eax // 00c50d3a
        fsub dword ptr [esp + 078h] // 00c50d3d
        fstp dword ptr [esp + 048h] // 00c50d41
        mov ecx, dword ptr [esp + 048h] // 00c50d45
        fld dword ptr [esi + 8] // 00c50d49
        mov dword ptr [esi + 010h], ecx // 00c50d4c
        fsub dword ptr [esp + 07ch] // 00c50d4f
        fstp dword ptr [esp + 04ch] // 00c50d53
        mov edx, dword ptr [esp + 04ch] // 00c50d57
        mov dword ptr [esi + 014h], edx // 00c50d5b
        fld dword ptr [esi + 010h] // 00c50d5e
        fstp dword ptr [esp + 024h] // 00c50d61
        fld dword ptr [esp + 024h] // 00c50d65
        fld dword ptr [esi + 0ch] // 00c50d69
        fstp dword ptr [esp + 028h] // 00c50d6c
        fld dword ptr [esp + 028h] // 00c50d70
        fld dword ptr [esi + 014h] // 00c50d74
        fstp dword ptr [esp + 020h] // 00c50d77
        fld dword ptr [esp + 020h] // 00c50d7b
        fld st(1) // 00c50d7f
        fmulp st(2), st(0) // 00c50d81
        fld st(2) // 00c50d83
        fmulp st(3), st(0) // 00c50d85
        fxch st(1) // 00c50d87
        faddp st(2), st(0) // 00c50d89
        fmul st(0), st(0) // 00c50d8b
        faddp st(1), st(0) // 00c50d8d
        push ecx // 00c50d8f
        fstp dword ptr [esp + 020h] // 00c50d90
        fld dword ptr [esp + 020h] // 00c50d94
        fstp dword ptr [esp] // 00c50d98
        push dword ptr [ebp+24] // Borrowed CRT context.
        call sqrt_kernel // 00c50d9b
        fstp dword ptr [esp + 01ch] // 00c50da0
        pop edi // 00c50da4
        fld dword ptr [esp + 024h] // 00c50da5
        mov al, 1 // 00c50da9
        fld dword ptr [esp + 018h] // 00c50dab
        fld st(0) // 00c50daf
        fdivp st(2), st(0) // 00c50db1
        fxch st(1) // 00c50db3
        fstp dword ptr [esi + 0ch] // 00c50db5
        fld dword ptr [esp + 020h] // 00c50db8
        fdiv st(0), st(1) // 00c50dbc
        fstp dword ptr [esi + 010h] // 00c50dbe
        fdivr dword ptr [esp + 01ch] // 00c50dc1
        fstp dword ptr [esi + 014h] // 00c50dc5
        pop esi // 00c50dc8
        pop ebx // 00c50dc9
        mov esp, ebp // 00c50dca
        pop ebp // 00c50dcc
        ret 20 // 00c50dcd
    }
}
// Complete original instruction schedule; address comments are native starts.
__declspec(naked) void box_ray_kernel(){
    __asm {
        push ebp // 00c50dd0
        mov ebp, esp // 00c50dd1
        and esp, 0fffffff8h // 00c50dd3
        sub esp, 070h // 00c50dd6
        mov ecx, dword ptr [ebp + 0ch] // 00c50dd9
        mov eax, dword ptr [ecx + 4] // 00c50ddc
        fld dword ptr [ecx + 038h] // 00c50ddf
        fstp dword ptr [esp + 014h] // 00c50de2
        add eax, 8 // 00c50de6
        fld dword ptr [eax + 0ch] // 00c50de9
        push esi // 00c50dec
        fstp dword ptr [esp + 0ch] // 00c50ded
        push edi // 00c50df1
        fld dword ptr [ecx + 034h] // 00c50df2
        fstp dword ptr [esp + 020h] // 00c50df5
        fld dword ptr [eax] // 00c50df9
        fstp dword ptr [esp + 014h] // 00c50dfb
        fld dword ptr [ecx + 03ch] // 00c50dff
        fstp dword ptr [esp + 018h] // 00c50e02
        fld dword ptr [eax + 018h] // 00c50e06
        fstp dword ptr [esp + 0ch] // 00c50e09
        fld dword ptr [esp + 014h] // 00c50e0d
        fld st(0) // 00c50e11
        fld dword ptr [esp + 020h] // 00c50e13
        fld st(0) // 00c50e17
        fmulp st(2), st(0) // 00c50e19
        fld dword ptr [esp + 010h] // 00c50e1b
        fld st(0) // 00c50e1f
        fld dword ptr [esp + 01ch] // 00c50e21
        fld st(0) // 00c50e25
        fmulp st(2), st(0) // 00c50e27
        fxch st(4) // 00c50e29
        faddp st(1), st(0) // 00c50e2b
        fld dword ptr [esp + 0ch] // 00c50e2d
        fld st(0) // 00c50e31
        fmul dword ptr [esp + 018h] // 00c50e33
        faddp st(2), st(0) // 00c50e37
        fxch st(1) // 00c50e39
        fstp dword ptr [esp + 048h] // 00c50e3b
        fld dword ptr [eax + 010h] // 00c50e3f
        fstp dword ptr [esp + 010h] // 00c50e42
        fld dword ptr [eax + 4] // 00c50e46
        fstp dword ptr [esp + 014h] // 00c50e49
        fld dword ptr [eax + 01ch] // 00c50e4d
        fstp dword ptr [esp + 8] // 00c50e50
        fld dword ptr [esp + 014h] // 00c50e54
        fld st(0) // 00c50e58
        fmulp st(4), st(0) // 00c50e5a
        fld dword ptr [esp + 010h] // 00c50e5c
        fld st(0) // 00c50e60
        fmulp st(6), st(0) // 00c50e62
        fxch st(4) // 00c50e64
        faddp st(5), st(0) // 00c50e66
        fld dword ptr [esp + 8] // 00c50e68
        fmul dword ptr [esp + 018h] // 00c50e6c
        faddp st(5), st(0) // 00c50e70
        fxch st(4) // 00c50e72
        fstp dword ptr [esp + 04ch] // 00c50e74
        fld dword ptr [eax + 014h] // 00c50e78
        fstp dword ptr [esp + 010h] // 00c50e7b
        fld dword ptr [eax + 8] // 00c50e7f
        fstp dword ptr [esp + 0ch] // 00c50e82
        fld dword ptr [eax + 020h] // 00c50e86
        fstp dword ptr [esp + 014h] // 00c50e89
        fld dword ptr [esp + 0ch] // 00c50e8d
        fmul dword ptr [esp + 020h] // 00c50e91
        fld dword ptr [esp + 010h] // 00c50e95
        fmul dword ptr [esp + 01ch] // 00c50e99
        faddp st(1), st(0) // 00c50e9d
        fld dword ptr [esp + 014h] // 00c50e9f
        fmul dword ptr [esp + 018h] // 00c50ea3
        faddp st(1), st(0) // 00c50ea7
        fstp dword ptr [esp + 050h] // 00c50ea9
        fld dword ptr [ecx + 044h] // 00c50ead
        fstp dword ptr [esp + 01ch] // 00c50eb0
        fld dword ptr [ecx + 040h] // 00c50eb4
        fstp dword ptr [esp + 018h] // 00c50eb7
        fld dword ptr [ecx + 048h] // 00c50ebb
        fstp dword ptr [esp + 020h] // 00c50ebe
        fld dword ptr [esp + 018h] // 00c50ec2
        fmul st(0), st(5) // 00c50ec6
        fld dword ptr [esp + 01ch] // 00c50ec8
        fmul st(0), st(3) // 00c50ecc
        faddp st(1), st(0) // 00c50ece
        fld dword ptr [esp + 020h] // 00c50ed0
        fmul st(0), st(2) // 00c50ed4
        faddp st(1), st(0) // 00c50ed6
        fstp dword ptr [esp + 054h] // 00c50ed8
        fld dword ptr [esp + 018h] // 00c50edc
        fmul st(0), st(4) // 00c50ee0
        fld dword ptr [esp + 01ch] // 00c50ee2
        fmul st(0), st(4) // 00c50ee6
        faddp st(1), st(0) // 00c50ee8
        fld dword ptr [esp + 020h] // 00c50eea
        fmul dword ptr [esp + 8] // 00c50eee
        faddp st(1), st(0) // 00c50ef2
        fstp dword ptr [esp + 058h] // 00c50ef4
        fld dword ptr [esp + 018h] // 00c50ef8
        fmul dword ptr [esp + 0ch] // 00c50efc
        fld dword ptr [esp + 01ch] // 00c50f00
        fmul dword ptr [esp + 010h] // 00c50f04
        faddp st(1), st(0) // 00c50f08
        fld dword ptr [esp + 020h] // 00c50f0a
        fmul dword ptr [esp + 014h] // 00c50f0e
        faddp st(1), st(0) // 00c50f12
        fstp dword ptr [esp + 05ch] // 00c50f14
        fld dword ptr [ecx + 050h] // 00c50f18
        fstp dword ptr [esp + 01ch] // 00c50f1b
        fld dword ptr [ecx + 04ch] // 00c50f1f
        fstp dword ptr [esp + 020h] // 00c50f22
        fld dword ptr [ecx + 054h] // 00c50f26
        fstp dword ptr [esp + 018h] // 00c50f29
        fld dword ptr [esp + 020h] // 00c50f2d
        fmul st(0), st(5) // 00c50f31
        fld dword ptr [esp + 01ch] // 00c50f33
        fmul st(0), st(3) // 00c50f37
        faddp st(1), st(0) // 00c50f39
        fld dword ptr [esp + 018h] // 00c50f3b
        fmul st(0), st(2) // 00c50f3f
        faddp st(1), st(0) // 00c50f41
        fstp dword ptr [esp + 060h] // 00c50f43
        fld dword ptr [esp + 020h] // 00c50f47
        fmul st(0), st(4) // 00c50f4b
        fld dword ptr [esp + 01ch] // 00c50f4d
        fmul st(0), st(4) // 00c50f51
        faddp st(1), st(0) // 00c50f53
        fld dword ptr [esp + 018h] // 00c50f55
        fmul dword ptr [esp + 8] // 00c50f59
        faddp st(1), st(0) // 00c50f5d
        fstp dword ptr [esp + 064h] // 00c50f5f
        fld dword ptr [esp + 020h] // 00c50f63
        fmul dword ptr [esp + 0ch] // 00c50f67
        fld dword ptr [esp + 01ch] // 00c50f6b
        fmul dword ptr [esp + 010h] // 00c50f6f
        faddp st(1), st(0) // 00c50f73
        fld dword ptr [esp + 018h] // 00c50f75
        fmul dword ptr [esp + 014h] // 00c50f79
        faddp st(1), st(0) // 00c50f7d
        fstp dword ptr [esp + 068h] // 00c50f7f
        fld dword ptr [ecx + 05ch] // 00c50f83
        fstp dword ptr [esp + 01ch] // 00c50f86
        fld dword ptr [ecx + 058h] // 00c50f8a
        fstp dword ptr [esp + 020h] // 00c50f8d
        fld dword ptr [ecx + 060h] // 00c50f91
        fstp dword ptr [esp + 018h] // 00c50f94
        fld dword ptr [esp + 020h] // 00c50f98
        fld st(0) // 00c50f9c
        fmulp st(6), st(0) // 00c50f9e
        fld dword ptr [esp + 01ch] // 00c50fa0
        fld st(0) // 00c50fa4
        fmulp st(4), st(0) // 00c50fa6
        fxch st(6) // 00c50fa8
        faddp st(3), st(0) // 00c50faa
        fld dword ptr [esp + 018h] // 00c50fac
        fld st(0) // 00c50fb0
        fmulp st(3), st(0) // 00c50fb2
        fxch st(3) // 00c50fb4
        faddp st(2), st(0) // 00c50fb6
        fld dword ptr [eax + 024h] // 00c50fb8
        faddp st(2), st(0) // 00c50fbb
        fxch st(1) // 00c50fbd
        fstp dword ptr [esp + 06ch] // 00c50fbf
        fld st(0) // 00c50fc3
        fmulp st(4), st(0) // 00c50fc5
        fld st(4) // 00c50fc7
        fmulp st(3), st(0) // 00c50fc9
        fxch st(3) // 00c50fcb
        faddp st(2), st(0) // 00c50fcd
        fld st(0) // 00c50fcf
        fmul dword ptr [esp + 8] // 00c50fd1
        faddp st(2), st(0) // 00c50fd5
        fld dword ptr [eax + 028h] // 00c50fd7
        faddp st(2), st(0) // 00c50fda
        fxch st(1) // 00c50fdc
        fstp dword ptr [esp + 070h] // 00c50fde
        fld dword ptr [esp + 0ch] // 00c50fe2
        fmulp st(2), st(0) // 00c50fe6
        fld dword ptr [esp + 010h] // 00c50fe8
        fmulp st(3), st(0) // 00c50fec
        fxch st(1) // 00c50fee
        faddp st(2), st(0) // 00c50ff0
        fmul dword ptr [esp + 014h] // 00c50ff2
        faddp st(1), st(0) // 00c50ff6
        fadd dword ptr [eax + 02ch] // 00c50ff8
        mov eax, dword ptr [ebp + 010h] // 00c50ffb
        fstp dword ptr [esp + 074h] // 00c50ffe
        fld dword ptr [eax] // 00c51002
        fsub dword ptr [esp + 06ch] // 00c51004
        fstp dword ptr [esp + 030h] // 00c51008
        fld dword ptr [eax + 4] // 00c5100c
        fsub dword ptr [esp + 070h] // 00c5100f
        fstp dword ptr [esp + 034h] // 00c51013
        fld dword ptr [eax + 8] // 00c51017
        mov eax, dword ptr [ebp + 014h] // 00c5101a
        fsub dword ptr [esp + 074h] // 00c5101d
        fstp dword ptr [esp + 038h] // 00c51021
        fld dword ptr [esp + 034h] // 00c51025
        fld st(0) // 00c51029
        fld dword ptr [esp + 04ch] // 00c5102b
        fld st(0) // 00c5102f
        fmulp st(2), st(0) // 00c51031
        fld dword ptr [esp + 030h] // 00c51033
        fld st(0) // 00c51037
        fld dword ptr [esp + 048h] // 00c51039
        fld st(0) // 00c5103d
        fmulp st(2), st(0) // 00c5103f
        fxch st(4) // 00c51041
        faddp st(1), st(0) // 00c51043
        fld dword ptr [esp + 038h] // 00c51045
        fld st(0) // 00c51049
        fmul dword ptr [esp + 050h] // 00c5104b
        faddp st(2), st(0) // 00c5104f
        fxch st(1) // 00c51051
        fstp dword ptr [esp + 030h] // 00c51053
        fld st(1) // 00c51057
        fmul dword ptr [esp + 054h] // 00c51059
        fld st(5) // 00c5105d
        fmul dword ptr [esp + 058h] // 00c5105f
        faddp st(1), st(0) // 00c51063
        fld st(1) // 00c51065
        fmul dword ptr [esp + 05ch] // 00c51067
        faddp st(1), st(0) // 00c5106b
        fstp dword ptr [esp + 034h] // 00c5106d
        fld dword ptr [esp + 060h] // 00c51071
        fmulp st(2), st(0) // 00c51075
        fld dword ptr [esp + 064h] // 00c51077
        fmulp st(5), st(0) // 00c5107b
        fxch st(1) // 00c5107d
        faddp st(4), st(0) // 00c5107f
        fmul dword ptr [esp + 068h] // 00c51081
        faddp st(3), st(0) // 00c51085
        fxch st(2) // 00c51087
        fstp dword ptr [esp + 038h] // 00c51089
        fld dword ptr [eax] // 00c5108d
        fsub dword ptr [esp + 06ch] // 00c5108f
        fstp dword ptr [esp + 024h] // 00c51093
        fld dword ptr [eax + 4] // 00c51097
        fsub dword ptr [esp + 070h] // 00c5109a
        fstp dword ptr [esp + 028h] // 00c5109e
        fld dword ptr [eax + 8] // 00c510a2
        fsub dword ptr [esp + 074h] // 00c510a5
        fstp dword ptr [esp + 02ch] // 00c510a9
        fld dword ptr [esp + 028h] // 00c510ad
        fld st(0) // 00c510b1
        fmul st(0), st(3) // 00c510b3
        fld dword ptr [esp + 024h] // 00c510b5
        fld st(0) // 00c510b9
        fmul st(0), st(4) // 00c510bb
        xorps xmm0, xmm0 // 00c510bd
        movss dword ptr [esp + 018h], xmm0 // 00c510c0
        movss xmm0, dword ptr constant_00d7a24c // 00c510c6
        faddp st(2), st(0) // 00c510ce
        lea edx, [ecx + 0210h] // 00c510d0
        fld dword ptr [esp + 02ch] // 00c510d6
        xor edi, edi // 00c510da
        fld st(0) // 00c510dc
        movss dword ptr [esp + 014h], xmm0 // 00c510de
        fmul dword ptr [esp + 050h] // 00c510e4
        xor ecx, ecx // 00c510e8
        faddp st(3), st(0) // 00c510ea
        fxch st(2) // 00c510ec
        fstp dword ptr [esp + 024h] // 00c510ee
        fld st(0) // 00c510f2
        fmul dword ptr [esp + 054h] // 00c510f4
        fld st(3) // 00c510f8
        fmul dword ptr [esp + 058h] // 00c510fa
        faddp st(1), st(0) // 00c510fe
        fld st(2) // 00c51100
        fmul dword ptr [esp + 05ch] // 00c51102
        faddp st(1), st(0) // 00c51106
        fstp dword ptr [esp + 028h] // 00c51108
        fmul dword ptr [esp + 060h] // 00c5110c
        fld dword ptr [esp + 064h] // 00c51110
        fmulp st(3), st(0) // 00c51114
        faddp st(2), st(0) // 00c51116
        fmul dword ptr [esp + 068h] // 00c51118
        faddp st(1), st(0) // 00c5111c
        fstp dword ptr [esp + 02ch] // 00c5111e
        fld dword ptr [esp + 018h] // 00c51122
    l_00c51126:
        fld dword ptr [esp + ecx*4 + 024h] // 00c51126
        fstp dword ptr [esp + 01ch] // 00c5112a
        fld dword ptr [esp + ecx*4 + 030h] // 00c5112e
        fstp dword ptr [esp + 020h] // 00c51132
        fld dword ptr [esp + 01ch] // 00c51136
        fld st(0) // 00c5113a
        fld dword ptr [esp + 020h] // 00c5113c
        fld st(0) // 00c51140
        fsubp st(2), st(0) // 00c51142
        fxch st(1) // 00c51144
        fstp dword ptr [esp + 010h] // 00c51146
        fxch st(1) // 00c5114a
        fcomi st(0), st(1) // 00c5114c
        fld dword ptr [edx] // 00c5114e
        fstp dword ptr [esp + 020h] // 00c51150
        fld dword ptr [esp + 020h] // 00c51154
        jbe l_00c511b0 // 00c51158
        fxch st(2) // 00c5115a
        fcomi st(0), st(2) // 00c5115c
        ja l_00c51434 // 00c5115e
        fld st(2) // 00c51164
        fchs  // 00c51166
        fstp dword ptr [esp + 020h] // 00c51168
        fld dword ptr [esp + 020h] // 00c5116c
        fcomi st(0), st(2) // 00c51170
        fstp st(2) // 00c51172
        ja l_00c51434 // 00c51174
        fxch st(1) // 00c5117a
        fcomi st(0), st(1) // 00c5117c
        fld dword ptr [esp + 010h] // 00c5117e
        jbe l_00c5118e // 00c51182
        fld st(2) // 00c51184
        fsubp st(2), st(0) // 00c51186
        fld st(0) // 00c51188
        fdivp st(2), st(0) // 00c5118a
        jmp l_00c51194 // 00c5118c
    l_00c5118e:
        fstp st(1) // 00c5118e
        fldz  // 00c51190
        fxch st(1) // 00c51192
    l_00c51194:
        fxch st(1) // 00c51194
        fstp dword ptr [esp + 020h] // 00c51196
        fld dword ptr [esp + 01ch] // 00c5119a
        fcomip st(0), st(3) // 00c5119e
        jbe l_00c511aa // 00c511a0
        fxch st(2) // 00c511a2
        fsubrp st(1), st(0) // 00c511a4
        fdivrp st(1), st(0) // 00c511a6
        jmp l_00c51208 // 00c511a8
    l_00c511aa:
        fstp st(1) // 00c511aa
        fstp st(1) // 00c511ac
        jmp l_00c51204 // 00c511ae
    l_00c511b0:
        fxch st(1) // 00c511b0
        fcomip st(0), st(1) // 00c511b2
        ja l_00c5144a // 00c511b4
        fld st(0) // 00c511ba
        fchs  // 00c511bc
        fstp dword ptr [esp + 020h] // 00c511be
        fld dword ptr [esp + 020h] // 00c511c2
        fcomi st(0), st(2) // 00c511c6
        ja l_00c5145e // 00c511c8
        fxch st(2) // 00c511ce
        fcomi st(0), st(1) // 00c511d0
        fld dword ptr [esp + 010h] // 00c511d2
        jbe l_00c511e2 // 00c511d6
        fld st(1) // 00c511d8
        fsubp st(3), st(0) // 00c511da
        fld st(0) // 00c511dc
        fdivp st(3), st(0) // 00c511de
        jmp l_00c511e8 // 00c511e0
    l_00c511e2:
        fstp st(2) // 00c511e2
        fldz  // 00c511e4
        fxch st(2) // 00c511e6
    l_00c511e8:
        fxch st(2) // 00c511e8
        fstp dword ptr [esp + 020h] // 00c511ea
        fld dword ptr [esp + 01ch] // 00c511ee
        fxch st(3) // 00c511f2
        fcomi st(0), st(3) // 00c511f4
        fstp st(3) // 00c511f6
        jbe l_00c51200 // 00c511f8
        fsubp st(2), st(0) // 00c511fa
        fdivp st(1), st(0) // 00c511fc
        jmp l_00c51208 // 00c511fe
    l_00c51200:
        fstp st(0) // 00c51200
        fstp st(0) // 00c51202
    l_00c51204:
        fstp st(0) // 00c51204
        fld1  // 00c51206
    l_00c51208:
        fstp dword ptr [esp + 01ch] // 00c51208
        fld dword ptr [esp + 020h] // 00c5120c
        fcomip st(0), st(1) // 00c51210
        jbe l_00c51228 // 00c51212
        movss xmm0, dword ptr [esp + 020h] // 00c51214
        fstp st(0) // 00c5121a
        movss dword ptr [esp + 018h], xmm0 // 00c5121c
        fld dword ptr [esp + 018h] // 00c51222
        mov edi, ecx // 00c51226
    l_00c51228:
        fld dword ptr [esp + 01ch] // 00c51228
        fld dword ptr [esp + 014h] // 00c5122c
        fcomip st(0), st(1) // 00c51230
        fstp st(0) // 00c51232
        jbe l_00c51242 // 00c51234
        movss xmm0, dword ptr [esp + 01ch] // 00c51236
        movss dword ptr [esp + 014h], xmm0 // 00c5123c
    l_00c51242:
        fld dword ptr [esp + 014h] // 00c51242
        fxch st(1) // 00c51246
        fcomi st(0), st(1) // 00c51248
        fstp st(1) // 00c5124a
        ja l_00c5143a // 00c5124c
        add ecx, 1 // 00c51252
        add edx, 4 // 00c51255
        cmp ecx, 3 // 00c51258
        jl l_00c51126 // 00c5125b
        fld dword ptr [esp + 024h] // 00c51261
        mov esi, dword ptr [ebp + 8] // 00c51265
        fld dword ptr [esp + 030h] // 00c51268
        fld st(0) // 00c5126c
        fsubp st(2), st(0) // 00c5126e
        fxch st(1) // 00c51270
        fstp dword ptr [esp + 03ch] // 00c51272
        fld dword ptr [esp + 028h] // 00c51276
        fld dword ptr [esp + 034h] // 00c5127a
        fld st(0) // 00c5127e
        fsubp st(2), st(0) // 00c51280
        fxch st(1) // 00c51282
        fstp dword ptr [esp + 040h] // 00c51284
        fld dword ptr [esp + 02ch] // 00c51288
        fld dword ptr [esp + 038h] // 00c5128c
        fld st(0) // 00c51290
        fsubp st(2), st(0) // 00c51292
        fxch st(1) // 00c51294
        fstp dword ptr [esp + 044h] // 00c51296
        fld dword ptr [esp + 03ch] // 00c5129a
        fmul st(0), st(4) // 00c5129e
        fstp dword ptr [esp + 024h] // 00c512a0
        fld dword ptr [esp + 040h] // 00c512a4
        fmul st(0), st(4) // 00c512a8
        fstp dword ptr [esp + 028h] // 00c512aa
        fld dword ptr [esp + 044h] // 00c512ae
        fmulp st(4), st(0) // 00c512b2
        fxch st(3) // 00c512b4
        fstp dword ptr [esp + 02ch] // 00c512b6
        fld dword ptr [esp + 024h] // 00c512ba
        faddp st(2), st(0) // 00c512be
        fxch st(1) // 00c512c0
        fstp dword ptr [esp + 030h] // 00c512c2
        fadd dword ptr [esp + 028h] // 00c512c6
        fstp dword ptr [esp + 034h] // 00c512ca
        fadd dword ptr [esp + 02ch] // 00c512ce
        fstp dword ptr [esp + 038h] // 00c512d2
        fld dword ptr [esp + 034h] // 00c512d6
        fld st(0) // 00c512da
        fmul dword ptr [esp + 054h] // 00c512dc
        fld dword ptr [esp + 030h] // 00c512e0
        fld st(0) // 00c512e4
        fmulp st(4), st(0) // 00c512e6
        fxch st(1) // 00c512e8
        faddp st(3), st(0) // 00c512ea
        fld dword ptr [esp + 038h] // 00c512ec
        fld st(0) // 00c512f0
        fmul dword ptr [esp + 060h] // 00c512f2
        faddp st(4), st(0) // 00c512f6
        fld dword ptr [esp + 06ch] // 00c512f8
        faddp st(4), st(0) // 00c512fc
        fxch st(3) // 00c512fe
        fstp dword ptr [esp + 03ch] // 00c51300
        mov eax, dword ptr [esp + 03ch] // 00c51304
        mov dword ptr [esi], eax // 00c51308
        fld st(0) // 00c5130a
        fmulp st(4), st(0) // 00c5130c
        fld st(1) // 00c5130e
        fmul dword ptr [esp + 058h] // 00c51310
        faddp st(4), st(0) // 00c51314
        fld st(2) // 00c51316
        fmul dword ptr [esp + 064h] // 00c51318
        faddp st(4), st(0) // 00c5131c
        fld dword ptr [esp + 070h] // 00c5131e
        faddp st(4), st(0) // 00c51322
        fxch st(3) // 00c51324
        fstp dword ptr [esp + 040h] // 00c51326
        mov ecx, dword ptr [esp + 040h] // 00c5132a
        fld dword ptr [esp + 050h] // 00c5132e
        mov dword ptr [esi + 4], ecx // 00c51332
        fmulp st(3), st(0) // 00c51335
        fmul dword ptr [esp + 05ch] // 00c51337
        faddp st(2), st(0) // 00c5133b
        fmul dword ptr [esp + 068h] // 00c5133d
        faddp st(1), st(0) // 00c51341
        fadd dword ptr [esp + 074h] // 00c51343
        fstp dword ptr [esp + 044h] // 00c51347
        mov edx, dword ptr [esp + 044h] // 00c5134b
        fldz  // 00c5134f
        mov dword ptr [esi + 8], edx // 00c51351
        fst dword ptr [esp + 024h] // 00c51354
        fst dword ptr [esp + 028h] // 00c51358
        push ecx // 00c5135c
        fstp dword ptr [esp + 030h] // 00c5135d
        fld dword ptr [esp + edi*4 + 034h] // 00c51361
        fstp dword ptr [esp + edi*4 + 028h] // 00c51365
        fld dword ptr [esp + 02ch] // 00c51369
        fld dword ptr [esp + 028h] // 00c5136d
        fld dword ptr [esp + 030h] // 00c51371
        fld st(1) // 00c51375
        fmulp st(2), st(0) // 00c51377
        fld st(2) // 00c51379
        fmulp st(3), st(0) // 00c5137b
        fxch st(1) // 00c5137d
        faddp st(2), st(0) // 00c5137f
        fmul st(0), st(0) // 00c51381
        faddp st(1), st(0) // 00c51383
        fstp dword ptr [esp + 024h] // 00c51385
        fld dword ptr [esp + 024h] // 00c51389
        fstp dword ptr [esp] // 00c5138d
        push dword ptr [ebp+24] // Borrowed CRT context.
        call sqrt_kernel // 00c51390
        fstp dword ptr [esp + 020h] // 00c51395
        fld dword ptr [esp + 024h] // 00c51399
        fld dword ptr [esp + 020h] // 00c5139d
        fld st(0) // 00c513a1
        fdivp st(2), st(0) // 00c513a3
        fxch st(1) // 00c513a5
        fstp dword ptr [esp + 024h] // 00c513a7
        fld dword ptr [esp + 028h] // 00c513ab
        fdiv st(0), st(1) // 00c513af
        fstp dword ptr [esp + 028h] // 00c513b1
        fdivr dword ptr [esp + 02ch] // 00c513b5
        fstp dword ptr [esp + 02ch] // 00c513b9
        fld dword ptr [esp + 028h] // 00c513bd
        fld st(0) // 00c513c1
        fmul dword ptr [esp + 054h] // 00c513c3
        fld dword ptr [esp + 024h] // 00c513c7
        fld st(0) // 00c513cb
        fmul dword ptr [esp + 048h] // 00c513cd
        faddp st(2), st(0) // 00c513d1
        fld dword ptr [esp + 02ch] // 00c513d3
        fld st(0) // 00c513d7
        fmul dword ptr [esp + 060h] // 00c513d9
        faddp st(3), st(0) // 00c513dd
        fxch st(2) // 00c513df
        fstp dword ptr [esp + 03ch] // 00c513e1
        mov eax, dword ptr [esp + 03ch] // 00c513e5
        mov dword ptr [esi + 0ch], eax // 00c513e9
        fld st(0) // 00c513ec
        mov al, 1 // 00c513ee
        fmul dword ptr [esp + 04ch] // 00c513f0
        fld st(3) // 00c513f4
        fmul dword ptr [esp + 058h] // 00c513f6
        faddp st(1), st(0) // 00c513fa
        fld st(2) // 00c513fc
        fmul dword ptr [esp + 064h] // 00c513fe
        faddp st(1), st(0) // 00c51402
        fstp dword ptr [esp + 040h] // 00c51404
        mov ecx, dword ptr [esp + 040h] // 00c51408
        mov dword ptr [esi + 010h], ecx // 00c5140c
        fmul dword ptr [esp + 050h] // 00c5140f
        fld dword ptr [esp + 05ch] // 00c51413
        fmulp st(3), st(0) // 00c51417
        faddp st(2), st(0) // 00c51419
        fmul dword ptr [esp + 068h] // 00c5141b
        faddp st(1), st(0) // 00c5141f
        fstp dword ptr [esp + 044h] // 00c51421
        mov edx, dword ptr [esp + 044h] // 00c51425
        mov dword ptr [esi + 014h], edx // 00c51429
        pop edi // 00c5142c
        pop esi // 00c5142d
        mov esp, ebp // 00c5142e
        pop ebp // 00c51430
        ret 20 // 00c51431
    l_00c51434:
        fstp st(0) // 00c51434
        fstp st(0) // 00c51436
        fstp st(0) // 00c51438
    l_00c5143a:
        fstp st(2) // 00c5143a
        xor al, al // 00c5143c
        fstp st(0) // 00c5143e
        fstp st(0) // 00c51440
        pop edi // 00c51442
        pop esi // 00c51443
        mov esp, ebp // 00c51444
        pop ebp // 00c51446
        ret 20 // 00c51447
    l_00c5144a:
        fstp st(1) // 00c5144a
        xor al, al // 00c5144c
        fstp st(0) // 00c5144e
        fstp st(2) // 00c51450
        fstp st(0) // 00c51452
        fstp st(0) // 00c51454
        pop edi // 00c51456
        pop esi // 00c51457
        mov esp, ebp // 00c51458
        pop ebp // 00c5145a
        ret 20 // 00c5145b
    l_00c5145e:
        fstp st(2) // 00c5145e
        pop edi // 00c51460
        fstp st(0) // 00c51461
        xor al, al // 00c51463
        fstp st(3) // 00c51465
        pop esi // 00c51467
        fstp st(1) // 00c51468
        fstp st(0) // 00c5146a
        fstp st(0) // 00c5146c
        mov esp, ebp // 00c5146e
        pop ebp // 00c51470
        ret 20 // 00c51471
    }
}
} // namespace
float sqrt_native_dyn_float_004011d0(float value,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;float result;
    __asm {
        push value
        push c
        call sqrt_kernel
        fstp result
    }
    return result;
}
bool intersect_native_dyn_spheres_00c518d0(void* result,const void* a,const float* ma,const void* b,const float* mb,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;unsigned char answer;
    __asm {
        push c
        push mb
        push b
        push ma
        push a
        push result
        call sphere_sphere_kernel
        mov answer,al
    }
    return answer!=0;
}
bool intersect_native_dyn_box_sphere_00c48330(void* result,const void* a,const float* ma,const void* b,const float* mb,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;unsigned char answer;
    __asm {
        push c
        push mb
        push b
        push ma
        push a
        push result
        call box_sphere_kernel
        mov answer,al
    }
    return answer!=0;
}
bool intersect_native_dyn_sphere_ray_00c50740(void* result,const void* shape,const float* start,const float* end,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;unsigned char answer;
    __asm {
        push c
        push end
        push start
        push shape
        push result
        call sphere_ray_kernel
        mov answer,al
    }
    return answer!=0;
}
bool intersect_native_dyn_box_ray_00c50dd0(void* result,const void* shape,const float* start,const float* end,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;unsigned char answer;
    __asm {
        push c
        push end
        push start
        push shape
        push result
        call box_ray_kernel
        mov answer,al
    }
    return answer!=0;
}
NativeDynPrimitiveDispatchRuntime::NativeDynPrimitiveDispatchRuntime(const CameraAxesCrtAccess& crt) noexcept
    :crt_(crt),sphere_sphere_{reinterpret_cast<std::uintptr_t>(&sphere_sphere),&crt_},
     box_sphere_{reinterpret_cast<std::uintptr_t>(&box_sphere),&crt_},
     sphere_ray_{reinterpret_cast<std::uintptr_t>(&sphere_ray),&crt_},
     box_ray_{reinterpret_cast<std::uintptr_t>(&box_ray),&crt_} {}
const CameraAxesCrtAccess& NativeDynPrimitiveDispatchRuntime::context(const void* p) noexcept {
    const auto& owner=*static_cast<const DynStaticDispatchObjectStorage*>(p);
    return *static_cast<const Entry*>(owner.vtable)->crt;
}
bool __fastcall NativeDynPrimitiveDispatchRuntime::sphere_sphere(void* owner,void*,void* result,const void* a,const float* ma,const void* b,const float* mb){
    return intersect_native_dyn_spheres_00c518d0(result,a,ma,b,mb,context(owner));
}
bool __fastcall NativeDynPrimitiveDispatchRuntime::box_sphere(void* owner,void*,void* result,const void* a,const float* ma,const void* b,const float* mb){
    return intersect_native_dyn_box_sphere_00c48330(result,a,ma,b,mb,context(owner));
}
bool __fastcall NativeDynPrimitiveDispatchRuntime::sphere_ray(void* owner,void*,void* result,const void* shape,const float* start,const float* end){
    return intersect_native_dyn_sphere_ray_00c50740(result,shape,start,end,context(owner));
}
bool __fastcall NativeDynPrimitiveDispatchRuntime::box_ray(void* owner,void*,void* result,const void* shape,const float* start,const float* end){
    return intersect_native_dyn_box_ray_00c50dd0(result,shape,start,end,context(owner));
}
} // namespace bsp
