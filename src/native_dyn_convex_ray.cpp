#include "bsp/native_dyn_convex_ray.hpp"
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native convex ray intersection requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
alignas(8) const std::uint64_t constant_00d7a250=0xbff0000000000000ULL;
alignas(8) const std::uint64_t constant_00d7a2a8=0x3d719799812dea11ULL;
alignas(8) const std::uint64_t constant_00d7a2b8=0x3eb0c6f7a0b5ed8dULL;
alignas(8) const std::uint64_t constant_00d7a2e0=0xbee4f8b588e368f1ULL;
alignas(8) const std::uint64_t constant_00d7a2e8=0x3e112e0be826d695ULL;
// Borrowed final argument leaves original locals and four stack arguments in place.
__declspec(naked) void sqrt_shim(){
    __asm {
        push ecx
        mov ecx,dword ptr [esp+8]
        call native_crt_sqrt_st0_00bf7030
        pop ecx
        ret 4
    }
}
// Consumed existing native math leaf; no new library reconstruction claim.
__declspec(naked) void cross_reference(){
    __asm {
        fld qword ptr [ecx + 010h] // 00401c20
        fmul qword ptr [edx + 8] // 00401c23
        fld qword ptr [edx + 010h] // 00401c26
        fmul qword ptr [ecx + 8] // 00401c29
        fsubp st(1), st(0) // 00401c2c
        fstp qword ptr [eax] // 00401c2e
        fld qword ptr [edx + 010h] // 00401c30
        fmul qword ptr [ecx] // 00401c33
        fld qword ptr [ecx + 010h] // 00401c35
        fmul qword ptr [edx] // 00401c38
        fsubp st(1), st(0) // 00401c3a
        fstp qword ptr [eax + 8] // 00401c3c
        fld qword ptr [ecx + 8] // 00401c3f
        fmul qword ptr [edx] // 00401c42
        fld qword ptr [ecx] // 00401c44
        fmul qword ptr [edx + 8] // 00401c46
        fsubp st(1), st(0) // 00401c49
        fstp qword ptr [eax + 010h] // 00401c4b
        ret  // 00401c4e
    }
}
// Consumed existing native math leaf; no new library reconstruction claim.
__declspec(naked) void subtract_reference(){
    __asm {
        fld qword ptr [edx] // 00401cb0
        fsub qword ptr [ecx] // 00401cb2
        fstp qword ptr [eax] // 00401cb4
        fld qword ptr [edx + 8] // 00401cb6
        fsub qword ptr [ecx + 8] // 00401cb9
        fstp qword ptr [eax + 8] // 00401cbc
        fld qword ptr [edx + 010h] // 00401cbf
        fsub qword ptr [ecx + 010h] // 00401cc2
        fstp qword ptr [eax + 010h] // 00401cc5
        ret  // 00401cc8
    }
}
// Consumed existing native math leaf; no new library reconstruction claim.
__declspec(naked) void scale_reference(){
    __asm {
        fld qword ptr [eax] // 00401cd0
        fld qword ptr [esp + 4] // 00401cd2
        fmul st(1), st(0) // 00401cd6
        fxch st(1) // 00401cd8
        fstp qword ptr [eax] // 00401cda
        fld qword ptr [eax + 8] // 00401cdc
        fmul st(0), st(1) // 00401cdf
        fstp qword ptr [eax + 8] // 00401ce1
        fmul qword ptr [eax + 010h] // 00401ce4
        fstp qword ptr [eax + 010h] // 00401ce7
        ret 8 // 00401cea
    }
}
// Complete recovered normal body; address comments are original PE starts.
__declspec(naked) void ray_kernel(){
    __asm {
        push ebp // 00c44780
        mov ebp, esp // 00c44781
        and esp, 0ffffffc0h // 00c44783
        sub esp, 07b4h // 00c44786
        mov eax, dword ptr [ebp + 010h] // 00c4478c
        fld dword ptr [eax] // 00c4478f
        push ebx // 00c44791
        fstp dword ptr [esp + 048h] // 00c44792
        push esi // 00c44796
        fld dword ptr [esp + 04ch] // 00c44797
        push edi // 00c4479b
        fld st(0) // 00c4479c
        mov edi, ecx // 00c4479e
        fst qword ptr [esp + 0268h] // 00c447a0
        mov ecx, dword ptr [ebp + 0ch] // 00c447a7
        fld dword ptr [eax + 4] // 00c447aa
        fstp dword ptr [esp + 050h] // 00c447ad
        fld dword ptr [esp + 050h] // 00c447b1
        fld st(0) // 00c447b5
        fst qword ptr [esp + 0270h] // 00c447b7
        fld dword ptr [eax + 8] // 00c447be
        mov eax, dword ptr [ebp + 014h] // 00c447c1
        fstp dword ptr [esp + 050h] // 00c447c4
        fld dword ptr [esp + 050h] // 00c447c8
        fld st(0) // 00c447cc
        fst qword ptr [esp + 0278h] // 00c447ce
        fld dword ptr [eax] // 00c447d5
        fsubrp st(6), st(0) // 00c447d7
        fxch st(5) // 00c447d9
        fstp dword ptr [esp + 070h] // 00c447db
        fld dword ptr [eax + 4] // 00c447df
        fsubrp st(3), st(0) // 00c447e2
        fxch st(2) // 00c447e4
        fstp dword ptr [esp + 074h] // 00c447e6
        fld dword ptr [eax + 8] // 00c447ea
        mov eax, dword ptr [ecx + 4] // 00c447ed
        fsubrp st(2), st(0) // 00c447f0
        add eax, 8 // 00c447f2
        fxch st(1) // 00c447f5
        fstp dword ptr [esp + 078h] // 00c447f7
        fld dword ptr [esp + 070h] // 00c447fb
        fstp qword ptr [esp + 0178h] // 00c447ff
        fld dword ptr [esp + 074h] // 00c44806
        fstp qword ptr [esp + 0180h] // 00c4480a
        fld dword ptr [esp + 078h] // 00c44811
        fstp qword ptr [esp + 0188h] // 00c44815
        fld dword ptr [ecx + 038h] // 00c4481c
        fstp dword ptr [esp + 080h] // 00c4481f
        fld dword ptr [ecx + 034h] // 00c44826
        fstp dword ptr [esp + 070h] // 00c44829
        fld dword ptr [ecx + 03ch] // 00c4482d
        fstp dword ptr [esp + 050h] // 00c44830
        fld dword ptr [esp + 080h] // 00c44834
        fld st(0) // 00c4483b
        fmul dword ptr [eax + 0ch] // 00c4483d
        fld dword ptr [eax] // 00c44840
        fld dword ptr [esp + 070h] // 00c44842
        fld st(0) // 00c44846
        fmulp st(2), st(0) // 00c44848
        fxch st(2) // 00c4484a
        faddp st(1), st(0) // 00c4484c
        fld dword ptr [eax + 018h] // 00c4484e
        fmul dword ptr [esp + 050h] // 00c44851
        faddp st(1), st(0) // 00c44855
        fstp dword ptr [esp + 0148h] // 00c44857
        fld dword ptr [eax + 4] // 00c4485e
        fmul st(0), st(1) // 00c44861
        fld st(2) // 00c44863
        fmul dword ptr [eax + 010h] // 00c44865
        faddp st(1), st(0) // 00c44868
        fld dword ptr [eax + 01ch] // 00c4486a
        fmul dword ptr [esp + 050h] // 00c4486d
        faddp st(1), st(0) // 00c44871
        fstp dword ptr [esp + 014ch] // 00c44873
        fld dword ptr [eax + 014h] // 00c4487a
        fmulp st(2), st(0) // 00c4487d
        fmul dword ptr [eax + 8] // 00c4487f
        faddp st(1), st(0) // 00c44882
        fld dword ptr [eax + 020h] // 00c44884
        fmul dword ptr [esp + 050h] // 00c44887
        faddp st(1), st(0) // 00c4488b
        fstp dword ptr [esp + 0150h] // 00c4488d
        fld dword ptr [ecx + 044h] // 00c44894
        fstp dword ptr [esp + 070h] // 00c44897
        fld dword ptr [ecx + 040h] // 00c4489b
        fstp dword ptr [esp + 080h] // 00c4489e
        fld dword ptr [ecx + 048h] // 00c448a5
        fstp dword ptr [esp + 050h] // 00c448a8
        fld dword ptr [esp + 070h] // 00c448ac
        fld st(0) // 00c448b0
        fmul dword ptr [eax + 0ch] // 00c448b2
        fld dword ptr [eax] // 00c448b5
        fld dword ptr [esp + 080h] // 00c448b7
        fld st(0) // 00c448be
        fmulp st(2), st(0) // 00c448c0
        fxch st(2) // 00c448c2
        faddp st(1), st(0) // 00c448c4
        fld dword ptr [eax + 018h] // 00c448c6
        fmul dword ptr [esp + 050h] // 00c448c9
        faddp st(1), st(0) // 00c448cd
        fstp dword ptr [esp + 0154h] // 00c448cf
        fld dword ptr [eax + 4] // 00c448d6
        fmul st(0), st(1) // 00c448d9
        fld st(2) // 00c448db
        fmul dword ptr [eax + 010h] // 00c448dd
        faddp st(1), st(0) // 00c448e0
        fld dword ptr [eax + 01ch] // 00c448e2
        fmul dword ptr [esp + 050h] // 00c448e5
        faddp st(1), st(0) // 00c448e9
        fstp dword ptr [esp + 0158h] // 00c448eb
        fld dword ptr [eax + 014h] // 00c448f2
        fmulp st(2), st(0) // 00c448f5
        fmul dword ptr [eax + 8] // 00c448f7
        faddp st(1), st(0) // 00c448fa
        fld dword ptr [eax + 020h] // 00c448fc
        fmul dword ptr [esp + 050h] // 00c448ff
        faddp st(1), st(0) // 00c44903
        fstp dword ptr [esp + 015ch] // 00c44905
        fld dword ptr [ecx + 050h] // 00c4490c
        fstp dword ptr [esp + 070h] // 00c4490f
        fld dword ptr [ecx + 04ch] // 00c44913
        fstp dword ptr [esp + 080h] // 00c44916
        fld dword ptr [ecx + 054h] // 00c4491d
        fstp dword ptr [esp + 050h] // 00c44920
        fld dword ptr [esp + 070h] // 00c44924
        fld st(0) // 00c44928
        fmul dword ptr [eax + 0ch] // 00c4492a
        fld dword ptr [eax] // 00c4492d
        fld dword ptr [esp + 080h] // 00c4492f
        fld st(0) // 00c44936
        fmulp st(2), st(0) // 00c44938
        fxch st(2) // 00c4493a
        faddp st(1), st(0) // 00c4493c
        fld dword ptr [eax + 018h] // 00c4493e
        fmul dword ptr [esp + 050h] // 00c44941
        faddp st(1), st(0) // 00c44945
        fstp dword ptr [esp + 0160h] // 00c44947
        fld dword ptr [eax + 4] // 00c4494e
        fmul st(0), st(1) // 00c44951
        fld st(2) // 00c44953
        fmul dword ptr [eax + 010h] // 00c44955
        faddp st(1), st(0) // 00c44958
        fld dword ptr [eax + 01ch] // 00c4495a
        fmul dword ptr [esp + 050h] // 00c4495d
        faddp st(1), st(0) // 00c44961
        fstp dword ptr [esp + 0164h] // 00c44963
        fld dword ptr [eax + 014h] // 00c4496a
        fmulp st(2), st(0) // 00c4496d
        fmul dword ptr [eax + 8] // 00c4496f
        faddp st(1), st(0) // 00c44972
        fld dword ptr [eax + 020h] // 00c44974
        fmul dword ptr [esp + 050h] // 00c44977
        faddp st(1), st(0) // 00c4497b
        fstp dword ptr [esp + 0168h] // 00c4497d
        fld dword ptr [ecx + 05ch] // 00c44984
        fstp dword ptr [esp + 080h] // 00c44987
        fld dword ptr [ecx + 058h] // 00c4498e
        fstp dword ptr [esp + 070h] // 00c44991
        fld dword ptr [ecx + 060h] // 00c44995
        fstp dword ptr [esp + 050h] // 00c44998
        fld dword ptr [eax] // 00c4499c
        fld dword ptr [esp + 070h] // 00c4499e
        fld st(0) // 00c449a2
        fmulp st(2), st(0) // 00c449a4
        fld dword ptr [esp + 080h] // 00c449a6
        fld st(0) // 00c449ad
        fmul dword ptr [eax + 0ch] // 00c449af
        faddp st(3), st(0) // 00c449b2
        fld dword ptr [esp + 050h] // 00c449b4
        fmul dword ptr [eax + 018h] // 00c449b8
        faddp st(3), st(0) // 00c449bb
        fld dword ptr [eax + 024h] // 00c449bd
        faddp st(3), st(0) // 00c449c0
        fxch st(2) // 00c449c2
        fstp dword ptr [esp + 016ch] // 00c449c4
        fld st(0) // 00c449cb
        fmul dword ptr [eax + 4] // 00c449cd
        fld st(2) // 00c449d0
        fmul dword ptr [eax + 010h] // 00c449d2
        faddp st(1), st(0) // 00c449d5
        fld dword ptr [eax + 01ch] // 00c449d7
        fmul dword ptr [esp + 050h] // 00c449da
        faddp st(1), st(0) // 00c449de
        fadd dword ptr [eax + 028h] // 00c449e0
        fstp dword ptr [esp + 0170h] // 00c449e3
        fmul dword ptr [eax + 8] // 00c449ea
        fld dword ptr [eax + 014h] // 00c449ed
        fmulp st(2), st(0) // 00c449f0
        faddp st(1), st(0) // 00c449f2
        fld dword ptr [esp + 050h] // 00c449f4
        fmul dword ptr [eax + 020h] // 00c449f8
        faddp st(1), st(0) // 00c449fb
        fadd dword ptr [eax + 02ch] // 00c449fd
        mov dword ptr [edi + 068h], 0 // 00c44a00
        fstp dword ptr [esp + 0174h] // 00c44a07
        fxch st(1) // 00c44a0e
        fst qword ptr [edi + 070h] // 00c44a10
        fxch st(1) // 00c44a13
        fst qword ptr [edi + 078h] // 00c44a15
        fxch st(2) // 00c44a18
        fst qword ptr [edi + 080h] // 00c44a1a
        fldz  // 00c44a20
        fst qword ptr [esp + 0d0h] // 00c44a22
        fst qword ptr [esp + 0d8h] // 00c44a29
        fst qword ptr [esp + 0e0h] // 00c44a30
        fld dword ptr [esp + 016ch] // 00c44a37
        fld dword ptr [esp + 0170h] // 00c44a3e
        fld dword ptr [esp + 0174h] // 00c44a45
        fxch st(5) // 00c44a4c
        fsubrp st(2), st(0) // 00c44a4e
        fsubp st(5), st(0) // 00c44a50
        fxch st(2) // 00c44a52
        fsubrp st(3), st(0) // 00c44a54
        fxch st(1) // 00c44a56
        fstp qword ptr [edi + 088h] // 00c44a58
        fxch st(2) // 00c44a5e
        fstp qword ptr [edi + 090h] // 00c44a60
        fstp qword ptr [edi + 098h] // 00c44a66
        cmp dword ptr [edi + 068h], 4 // 00c44a6c
        fstp qword ptr [esp + 070h] // 00c44a70
        fld qword ptr constant_00d7a250 // 00c44a74
        fstp qword ptr [esp + 080h] // 00c44a7a
        jge l_00c47a0a // 00c44a81
    l_00c44a87:
        fld qword ptr [edi + 090h] // 00c44a87
        lea esi, [edi + 088h] // 00c44a8d
        fld qword ptr [esi] // 00c44a93
        fld qword ptr [esi + 010h] // 00c44a95
        fld st(1) // 00c44a98
        fmulp st(2), st(0) // 00c44a9a
        fld st(2) // 00c44a9c
        fmulp st(3), st(0) // 00c44a9e
        fxch st(1) // 00c44aa0
        faddp st(2), st(0) // 00c44aa2
        fmul st(0), st(0) // 00c44aa4
        faddp st(1), st(0) // 00c44aa6
        fld qword ptr constant_00d7a2e8 // 00c44aa8
        fxch st(1) // 00c44aae
        fcomip st(0), st(1) // 00c44ab0
        fstp st(0) // 00c44ab2
        jbe l_00c47a0a // 00c44ab4
        fld qword ptr [esp + 080h] // 00c44aba
        fadd qword ptr constant_00d7a2b8 // 00c44ac1
        fld qword ptr [esp + 070h] // 00c44ac7
        fcomip st(0), st(1) // 00c44acb
        fstp st(0) // 00c44acd
        jbe l_00c47a0a // 00c44acf
        fld qword ptr [esi + 8] // 00c44ad5
        fld qword ptr [esi] // 00c44ad8
        fld qword ptr [esi + 010h] // 00c44ada
        fld st(1) // 00c44add
        fmulp st(2), st(0) // 00c44adf
        fld st(2) // 00c44ae1
        fmulp st(3), st(0) // 00c44ae3
        fxch st(1) // 00c44ae5
        faddp st(2), st(0) // 00c44ae7
        fmul st(0), st(0) // 00c44ae9
        faddp st(1), st(0) // 00c44aeb
        push dword ptr [ebp+18h] // Borrowed CRT service.
        call sqrt_shim // 00c44aed
        fld1  // 00c44af2
        fdivrp st(1), st(0) // 00c44af4
        mov ecx, dword ptr [ebp + 0ch] // 00c44af6
        mov eax, dword ptr [ecx] // 00c44af9
        mov eax, dword ptr [eax + 0ch] // 00c44afb
        lea edx, [esp + 0148h] // 00c44afe
        push edx // 00c44b05
        push esi // 00c44b06
        lea edx, [esp + 0c0h] // 00c44b07
        push edx // 00c44b0e
        fld st(0) // 00c44b0f
        fmul qword ptr [esi] // 00c44b11
        fstp qword ptr [esi] // 00c44b13
        fld qword ptr [esi + 8] // 00c44b15
        fmul st(0), st(1) // 00c44b18
        fstp qword ptr [esi + 8] // 00c44b1a
        fmul qword ptr [esi + 010h] // 00c44b1d
        fstp qword ptr [esi + 010h] // 00c44b20
        call eax // 00c44b23
        fld qword ptr [edi + 070h] // 00c44b25
        fld qword ptr [esp + 0b8h] // 00c44b28
        fsub st(1), st(0) // 00c44b2f
        fld qword ptr [edi + 078h] // 00c44b31
        fld qword ptr [esp + 0c0h] // 00c44b34
        fsub st(1), st(0) // 00c44b3b
        fld qword ptr [edi + 080h] // 00c44b3d
        fld qword ptr [esp + 0c8h] // 00c44b43
        fsub st(1), st(0) // 00c44b4a
        fld qword ptr [esi + 8] // 00c44b4c
        fmulp st(4), st(0) // 00c44b4f
        fld qword ptr [esi] // 00c44b51
        fmulp st(6), st(0) // 00c44b53
        fxch st(3) // 00c44b55
        faddp st(5), st(0) // 00c44b57
        fmul qword ptr [esi + 010h] // 00c44b59
        faddp st(4), st(0) // 00c44b5c
        fldz  // 00c44b5e
        fxch st(4) // 00c44b60
        fcomi st(0), st(4) // 00c44b62
        jbe l_00c44c28 // 00c44b64
        fld qword ptr [esi + 8] // 00c44b6a
        fmul qword ptr [esp + 0180h] // 00c44b6d
        fld qword ptr [esi] // 00c44b74
        fmul qword ptr [esp + 0178h] // 00c44b76
        faddp st(1), st(0) // 00c44b7d
        fld qword ptr [esi + 010h] // 00c44b7f
        fld qword ptr [esp + 0188h] // 00c44b82
        fmul st(1), st(0) // 00c44b89
        fxch st(2) // 00c44b8b
        faddp st(1), st(0) // 00c44b8d
        fst qword ptr [esp + 050h] // 00c44b8f
        fld qword ptr constant_00d7a2e0 // 00c44b93
        fxch st(1) // 00c44b99
        fcomip st(0), st(1) // 00c44b9b
        fstp st(0) // 00c44b9d
        ja l_00c47a4a // 00c44b9f
        fld qword ptr [esp + 070h] // 00c44ba5
        fst qword ptr [esp + 080h] // 00c44ba9
        fxch st(2) // 00c44bb0
        fdiv qword ptr [esp + 050h] // 00c44bb2
        fsubp st(2), st(0) // 00c44bb6
        fxch st(1) // 00c44bb8
        fst qword ptr [esp + 070h] // 00c44bba
        fld1  // 00c44bbe
        fxch st(1) // 00c44bc0
        fcomi st(0), st(1) // 00c44bc2
        fstp st(1) // 00c44bc4
        ja l_00c47a61 // 00c44bc6
        fld qword ptr [esp + 0178h] // 00c44bcc
        fmul st(0), st(1) // 00c44bd3
        fld qword ptr [esp + 0180h] // 00c44bd5
        fmul st(0), st(2) // 00c44bdc
        fxch st(3) // 00c44bde
        fmulp st(2), st(0) // 00c44be0
        fadd qword ptr [esp + 0268h] // 00c44be2
        fld qword ptr [esp + 0270h] // 00c44be9
        faddp st(3), st(0) // 00c44bf0
        fld qword ptr [esp + 0278h] // 00c44bf2
        faddp st(2), st(0) // 00c44bf9
        fstp qword ptr [edi + 070h] // 00c44bfb
        fxch st(1) // 00c44bfe
        fstp qword ptr [edi + 078h] // 00c44c00
        fstp qword ptr [edi + 080h] // 00c44c03
        fld qword ptr [esi] // 00c44c09
        fstp qword ptr [esp + 0d0h] // 00c44c0b
        fld qword ptr [esi + 8] // 00c44c12
        fstp qword ptr [esp + 0d8h] // 00c44c15
        fld qword ptr [esi + 010h] // 00c44c1c
        fstp qword ptr [esp + 0e0h] // 00c44c1f
        jmp l_00c44c2a // 00c44c26
    l_00c44c28:
        fstp st(0) // 00c44c28
    l_00c44c2a:
        mov esi, dword ptr [edi + 068h] // 00c44c2a
        xor ecx, ecx // 00c44c2d
        test esi, esi // 00c44c2f
        lea ebx, [ecx + 1] // 00c44c31
        jle l_00c44c6d // 00c44c34
        lea edx, [edi + 018h] // 00c44c36
    l_00c44c39:
        fld qword ptr [edx - 010h] // 00c44c39
        fsub st(0), st(3) // 00c44c3c
        fld qword ptr [edx - 8] // 00c44c3e
        fsub st(0), st(2) // 00c44c41
        fld qword ptr [edx] // 00c44c43
        fsub st(0), st(4) // 00c44c45
        fld st(1) // 00c44c47
        fmulp st(2), st(0) // 00c44c49
        fld st(2) // 00c44c4b
        fmulp st(3), st(0) // 00c44c4d
        fxch st(1) // 00c44c4f
        faddp st(2), st(0) // 00c44c51
        fmul st(0), st(0) // 00c44c53
        faddp st(1), st(0) // 00c44c55
        fld qword ptr constant_00d7a2a8 // 00c44c57
        fcomip st(0), st(1) // 00c44c5d
        fstp st(0) // 00c44c5f
        ja l_00c44cab // 00c44c61
        add ecx, ebx // 00c44c63
        add edx, 018h // 00c44c65
        cmp ecx, dword ptr [edi + 068h] // 00c44c68
        jl l_00c44c39 // 00c44c6b
    l_00c44c6d:
        fstp st(1) // 00c44c6d
        lea ecx, [esi + esi*2] // 00c44c6f
        fstp st(0) // 00c44c72
        lea eax, [edi + ecx*8 + 8] // 00c44c74
        add esi, 1 // 00c44c78
        mov dword ptr [edi + 068h], esi // 00c44c7b
        fstp qword ptr [eax] // 00c44c7e
        fld qword ptr [esp + 0c0h] // 00c44c80
        fstp qword ptr [eax + 8] // 00c44c87
        fld qword ptr [esp + 0c8h] // 00c44c8a
        fstp qword ptr [eax + 010h] // 00c44c91
        fld qword ptr [esp + 0c8h] // 00c44c94
        fld qword ptr [esp + 0c0h] // 00c44c9b
        fld qword ptr [esp + 0b8h] // 00c44ca2
        jmp l_00c44caf // 00c44ca9
    l_00c44cab:
        fxch st(1) // 00c44cab
        fxch st(2) // 00c44cad
    l_00c44caf:
        mov eax, dword ptr [edi + 068h] // 00c44caf
        cmp eax, 4 // 00c44cb2
        ja l_00c47a78 // 00c44cb5
        // Five verified switch targets; preserve incoming flags and all registers.
        pushfd
        cmp eax,0
        je switch_restore_0
        cmp eax,1
        je switch_restore_1
        cmp eax,2
        je switch_restore_2
        cmp eax,3
        je switch_restore_3
        jmp switch_restore_4
    switch_restore_0:
        popfd
        jmp l_00c44cc2
    switch_restore_1:
        popfd
        jmp l_00c44cd9
    switch_restore_2:
        popfd
        jmp l_00c44d0f
    switch_restore_3:
        popfd
        jmp l_00c44ec2
    switch_restore_4:
        popfd
        jmp l_00c4577d
    l_00c44cc2:
        fstp st(3) // 00c44cc2
        fld qword ptr [edi + 070h] // 00c44cc4
        fsubrp st(3), st(0) // 00c44cc7
        fsubr qword ptr [edi + 078h] // 00c44cc9
        fld qword ptr [edi + 080h] // 00c44ccc
        fsubrp st(2), st(0) // 00c44cd2
        jmp l_00c44ea7 // 00c44cd4
    l_00c44cd9:
        fstp st(3) // 00c44cd9
        fstp st(2) // 00c44cdb
        fstp st(0) // 00c44cdd
        fstp st(0) // 00c44cdf
        fld qword ptr [edi + 070h] // 00c44ce1
        fsub qword ptr [edi + 8] // 00c44ce4
        fld qword ptr [edi + 078h] // 00c44ce7
        fsub qword ptr [edi + 010h] // 00c44cea
        fld qword ptr [edi + 080h] // 00c44ced
        fsub qword ptr [edi + 018h] // 00c44cf3
        fxch st(2) // 00c44cf6
        fstp qword ptr [edi + 088h] // 00c44cf8
        fstp qword ptr [edi + 090h] // 00c44cfe
        fstp qword ptr [edi + 098h] // 00c44d04
        jmp l_00c47a00 // 00c44d0a
    l_00c44d0f:
        fstp st(0) // 00c44d0f
        fstp st(1) // 00c44d11
        fstp st(0) // 00c44d13
        fld qword ptr [edi + 020h] // 00c44d15
        fsub qword ptr [edi + 8] // 00c44d18
        fld qword ptr [edi + 028h] // 00c44d1b
        fsub qword ptr [edi + 010h] // 00c44d1e
        fld qword ptr [edi + 030h] // 00c44d21
        fsub qword ptr [edi + 018h] // 00c44d24
        fld qword ptr [edi + 8] // 00c44d27
        fsub qword ptr [edi + 070h] // 00c44d2a
        fld qword ptr [edi + 010h] // 00c44d2d
        fsub qword ptr [edi + 078h] // 00c44d30
        fld qword ptr [edi + 018h] // 00c44d33
        fsub qword ptr [edi + 080h] // 00c44d36
        fxch st(1) // 00c44d3c
        fmulp st(4), st(0) // 00c44d3e
        fxch st(1) // 00c44d40
        fmulp st(4), st(0) // 00c44d42
        fxch st(2) // 00c44d44
        faddp st(3), st(0) // 00c44d46
        fmulp st(1), st(0) // 00c44d48
        faddp st(1), st(0) // 00c44d4a
        fcomip st(0), st(1) // 00c44d4c
        jb l_00c44d95 // 00c44d4e
    l_00c44d50:
        fstp st(0) // 00c44d50
        fld qword ptr [edi + 8] // 00c44d52
        fstp qword ptr [edi + 8] // 00c44d55
        fld qword ptr [edi + 010h] // 00c44d58
        fstp qword ptr [edi + 010h] // 00c44d5b
        fld qword ptr [edi + 018h] // 00c44d5e
    l_00c44d61:
        fstp qword ptr [edi + 018h] // 00c44d61
        fld qword ptr [edi + 070h] // 00c44d64
        fsub qword ptr [edi + 8] // 00c44d67
        fld qword ptr [edi + 078h] // 00c44d6a
        fsub qword ptr [edi + 010h] // 00c44d6d
        fld qword ptr [edi + 080h] // 00c44d70
        fsub qword ptr [edi + 018h] // 00c44d76
        fxch st(2) // 00c44d79
        fstp qword ptr [edi + 088h] // 00c44d7b
        fstp qword ptr [edi + 090h] // 00c44d81
        fstp qword ptr [edi + 098h] // 00c44d87
        mov dword ptr [edi + 068h], ebx // 00c44d8d
        jmp l_00c47a00 // 00c44d90
    l_00c44d95:
        fld qword ptr [edi + 8] // 00c44d95
        fsub qword ptr [edi + 020h] // 00c44d98
        fld qword ptr [edi + 010h] // 00c44d9b
        fsub qword ptr [edi + 028h] // 00c44d9e
        fld qword ptr [edi + 018h] // 00c44da1
        fsub qword ptr [edi + 030h] // 00c44da4
        fld qword ptr [edi + 020h] // 00c44da7
        fsub qword ptr [edi + 070h] // 00c44daa
        fld qword ptr [edi + 028h] // 00c44dad
        fsub qword ptr [edi + 078h] // 00c44db0
        fld qword ptr [edi + 030h] // 00c44db3
        fsub qword ptr [edi + 080h] // 00c44db6
        fxch st(1) // 00c44dbc
        fmulp st(4), st(0) // 00c44dbe
        fxch st(1) // 00c44dc0
        fmulp st(4), st(0) // 00c44dc2
        fxch st(2) // 00c44dc4
        faddp st(3), st(0) // 00c44dc6
        fmulp st(1), st(0) // 00c44dc8
        faddp st(1), st(0) // 00c44dca
        fcomip st(0), st(1) // 00c44dcc
        fstp st(0) // 00c44dce
        jb l_00c44de6 // 00c44dd0
        fld qword ptr [edi + 020h] // 00c44dd2
        fstp qword ptr [edi + 8] // 00c44dd5
        fld qword ptr [edi + 028h] // 00c44dd8
        fstp qword ptr [edi + 010h] // 00c44ddb
        fld qword ptr [edi + 030h] // 00c44dde
        jmp l_00c44d61 // 00c44de1
    l_00c44de6:
        fld qword ptr [edi + 8] // 00c44de6
        fsub qword ptr [edi + 020h] // 00c44de9
        fld qword ptr [edi + 010h] // 00c44dec
        fsub qword ptr [edi + 028h] // 00c44def
        fld qword ptr [edi + 018h] // 00c44df2
        fsub qword ptr [edi + 030h] // 00c44df5
        fld qword ptr [edi + 8] // 00c44df8
        fsub qword ptr [edi + 070h] // 00c44dfb
        fld qword ptr [edi + 010h] // 00c44dfe
        fsub qword ptr [edi + 078h] // 00c44e01
        fld qword ptr [edi + 018h] // 00c44e04
        fsub qword ptr [edi + 080h] // 00c44e07
        fst qword ptr [esp + 0740h] // 00c44e0d
        fld qword ptr [edi + 020h] // 00c44e14
        fsub qword ptr [edi + 8] // 00c44e17
        fst qword ptr [esp + 0190h] // 00c44e1a
        fld qword ptr [edi + 028h] // 00c44e21
        fsub qword ptr [edi + 010h] // 00c44e24
        fstp qword ptr [esp + 0198h] // 00c44e27
        fld qword ptr [edi + 030h] // 00c44e2e
        fsub qword ptr [edi + 018h] // 00c44e31
        fstp qword ptr [esp + 01a0h] // 00c44e34
        fld qword ptr [esp + 0198h] // 00c44e3b
        fmulp st(2), st(0) // 00c44e42
        fld qword ptr [esp + 01a0h] // 00c44e44
        fmul st(0), st(3) // 00c44e4b
        fsubp st(2), st(0) // 00c44e4d
        fxch st(1) // 00c44e4f
        fstp qword ptr [esp + 0418h] // 00c44e51
        fld st(2) // 00c44e58
        fmul qword ptr [esp + 01a0h] // 00c44e5a
        fxch st(1) // 00c44e61
        fmul qword ptr [esp + 0740h] // 00c44e63
        fsubp st(1), st(0) // 00c44e6a
        fld qword ptr [esp + 0190h] // 00c44e6c
        fmulp st(2), st(0) // 00c44e73
        fxch st(2) // 00c44e75
        fmul qword ptr [esp + 0198h] // 00c44e77
        fsubp st(1), st(0) // 00c44e7e
        fld st(1) // 00c44e80
        fmul st(0), st(3) // 00c44e82
        fld st(1) // 00c44e84
        fmul st(0), st(5) // 00c44e86
        fsubp st(1), st(0) // 00c44e88
        fld st(5) // 00c44e8a
        fmulp st(2), st(0) // 00c44e8c
        fld qword ptr [esp + 0418h] // 00c44e8e
    l_00c44e95:
        fld st(0) // 00c44e95
        fmulp st(5), st(0) // 00c44e97
        fxch st(2) // 00c44e99
        fsubrp st(4), st(0) // 00c44e9b
        fxch st(1) // 00c44e9d
        fmulp st(4), st(0) // 00c44e9f
        fxch st(4) // 00c44ea1
        fmulp st(1), st(0) // 00c44ea3
        fsubp st(2), st(0) // 00c44ea5
    l_00c44ea7:
        fxch st(2) // 00c44ea7
        fstp qword ptr [edi + 088h] // 00c44ea9
        fxch st(1) // 00c44eaf
        fstp qword ptr [edi + 090h] // 00c44eb1
        fstp qword ptr [edi + 098h] // 00c44eb7
        jmp l_00c47a00 // 00c44ebd
    l_00c44ec2:
        fstp st(0) // 00c44ec2
        fstp st(1) // 00c44ec4
        fstp st(0) // 00c44ec6
        fld qword ptr [edi + 020h] // 00c44ec8
        fsub qword ptr [edi + 8] // 00c44ecb
        fld qword ptr [edi + 028h] // 00c44ece
        fsub qword ptr [edi + 010h] // 00c44ed1
        fld qword ptr [edi + 030h] // 00c44ed4
        fsub qword ptr [edi + 018h] // 00c44ed7
        fld qword ptr [edi + 8] // 00c44eda
        fsub qword ptr [edi + 070h] // 00c44edd
        fld qword ptr [edi + 010h] // 00c44ee0
        fsub qword ptr [edi + 078h] // 00c44ee3
        fld qword ptr [edi + 018h] // 00c44ee6
        fsub qword ptr [edi + 080h] // 00c44ee9
        fxch st(2) // 00c44eef
        fmulp st(5), st(0) // 00c44ef1
        fmulp st(3), st(0) // 00c44ef3
        fxch st(3) // 00c44ef5
        faddp st(2), st(0) // 00c44ef7
        fmulp st(2), st(0) // 00c44ef9
        faddp st(1), st(0) // 00c44efb
        fcomip st(0), st(1) // 00c44efd
        jb l_00c44f3e // 00c44eff
        fld qword ptr [edi + 038h] // 00c44f01
        fsub qword ptr [edi + 8] // 00c44f04
        fld qword ptr [edi + 040h] // 00c44f07
        fsub qword ptr [edi + 010h] // 00c44f0a
        fld qword ptr [edi + 048h] // 00c44f0d
        fsub qword ptr [edi + 018h] // 00c44f10
        fld qword ptr [edi + 8] // 00c44f13
        fsub qword ptr [edi + 070h] // 00c44f16
        fld qword ptr [edi + 010h] // 00c44f19
        fsub qword ptr [edi + 078h] // 00c44f1c
        fld qword ptr [edi + 018h] // 00c44f1f
        fsub qword ptr [edi + 080h] // 00c44f22
        fxch st(2) // 00c44f28
        fmulp st(5), st(0) // 00c44f2a
        fmulp st(3), st(0) // 00c44f2c
        fxch st(3) // 00c44f2e
        faddp st(2), st(0) // 00c44f30
        fmulp st(2), st(0) // 00c44f32
        faddp st(1), st(0) // 00c44f34
        fcomip st(0), st(1) // 00c44f36
        jae l_00c44d50 // 00c44f38
    l_00c44f3e:
        fld qword ptr [edi + 8] // 00c44f3e
        fsub qword ptr [edi + 020h] // 00c44f41
        fld qword ptr [edi + 010h] // 00c44f44
        fsub qword ptr [edi + 028h] // 00c44f47
        fld qword ptr [edi + 018h] // 00c44f4a
        fsub qword ptr [edi + 030h] // 00c44f4d
        fld qword ptr [edi + 020h] // 00c44f50
        fsub qword ptr [edi + 070h] // 00c44f53
        fld qword ptr [edi + 028h] // 00c44f56
        fsub qword ptr [edi + 078h] // 00c44f59
        fld qword ptr [edi + 030h] // 00c44f5c
        fsub qword ptr [edi + 080h] // 00c44f5f
        fxch st(1) // 00c44f65
        fmulp st(4), st(0) // 00c44f67
        fxch st(1) // 00c44f69
        fmulp st(4), st(0) // 00c44f6b
        fxch st(2) // 00c44f6d
        faddp st(3), st(0) // 00c44f6f
        fmulp st(1), st(0) // 00c44f71
        faddp st(1), st(0) // 00c44f73
        fcomip st(0), st(1) // 00c44f75
        jb l_00c44fca // 00c44f77
        fld qword ptr [edi + 038h] // 00c44f79
        fsub qword ptr [edi + 020h] // 00c44f7c
        fld qword ptr [edi + 040h] // 00c44f7f
        fsub qword ptr [edi + 028h] // 00c44f82
        fld qword ptr [edi + 048h] // 00c44f85
        fsub qword ptr [edi + 030h] // 00c44f88
        fld qword ptr [edi + 020h] // 00c44f8b
        fsub qword ptr [edi + 070h] // 00c44f8e
        fld qword ptr [edi + 028h] // 00c44f91
        fsub qword ptr [edi + 078h] // 00c44f94
        fld qword ptr [edi + 030h] // 00c44f97
        fsub qword ptr [edi + 080h] // 00c44f9a
        fxch st(1) // 00c44fa0
        fmulp st(4), st(0) // 00c44fa2
        fxch st(1) // 00c44fa4
        fmulp st(4), st(0) // 00c44fa6
        fxch st(2) // 00c44fa8
        faddp st(3), st(0) // 00c44faa
        fmulp st(1), st(0) // 00c44fac
        faddp st(1), st(0) // 00c44fae
        fcomip st(0), st(1) // 00c44fb0
        jb l_00c44fca // 00c44fb2
        fstp st(0) // 00c44fb4
        fld qword ptr [edi + 020h] // 00c44fb6
        fstp qword ptr [edi + 8] // 00c44fb9
        fld qword ptr [edi + 028h] // 00c44fbc
        fstp qword ptr [edi + 010h] // 00c44fbf
        fld qword ptr [edi + 030h] // 00c44fc2
        jmp l_00c44d61 // 00c44fc5
    l_00c44fca:
        fld qword ptr [edi + 8] // 00c44fca
        fsub qword ptr [edi + 038h] // 00c44fcd
        fld qword ptr [edi + 010h] // 00c44fd0
        fsub qword ptr [edi + 040h] // 00c44fd3
        fld qword ptr [edi + 018h] // 00c44fd6
        fsub qword ptr [edi + 048h] // 00c44fd9
        fld qword ptr [edi + 038h] // 00c44fdc
        fsub qword ptr [edi + 070h] // 00c44fdf
        fld qword ptr [edi + 040h] // 00c44fe2
        fsub qword ptr [edi + 078h] // 00c44fe5
        fld qword ptr [edi + 048h] // 00c44fe8
        fsub qword ptr [edi + 080h] // 00c44feb
        fxch st(1) // 00c44ff1
        fmulp st(4), st(0) // 00c44ff3
        fxch st(1) // 00c44ff5
        fmulp st(4), st(0) // 00c44ff7
        fxch st(2) // 00c44ff9
        faddp st(3), st(0) // 00c44ffb
        fmulp st(1), st(0) // 00c44ffd
        faddp st(1), st(0) // 00c44fff
        fcomip st(0), st(1) // 00c45001
        jb l_00c45056 // 00c45003
        fld qword ptr [edi + 020h] // 00c45005
        fsub qword ptr [edi + 038h] // 00c45008
        fld qword ptr [edi + 028h] // 00c4500b
        fsub qword ptr [edi + 040h] // 00c4500e
        fld qword ptr [edi + 030h] // 00c45011
        fsub qword ptr [edi + 048h] // 00c45014
        fld qword ptr [edi + 038h] // 00c45017
        fsub qword ptr [edi + 070h] // 00c4501a
        fld qword ptr [edi + 040h] // 00c4501d
        fsub qword ptr [edi + 078h] // 00c45020
        fld qword ptr [edi + 048h] // 00c45023
        fsub qword ptr [edi + 080h] // 00c45026
        fxch st(1) // 00c4502c
        fmulp st(4), st(0) // 00c4502e
        fxch st(1) // 00c45030
        fmulp st(4), st(0) // 00c45032
        fxch st(2) // 00c45034
        faddp st(3), st(0) // 00c45036
        fmulp st(1), st(0) // 00c45038
        faddp st(1), st(0) // 00c4503a
        fcomip st(0), st(1) // 00c4503c
        jb l_00c45056 // 00c4503e
        fstp st(0) // 00c45040
        fld qword ptr [edi + 038h] // 00c45042
        fstp qword ptr [edi + 8] // 00c45045
        fld qword ptr [edi + 040h] // 00c45048
        fstp qword ptr [edi + 010h] // 00c4504b
        fld qword ptr [edi + 048h] // 00c4504e
        jmp l_00c44d61 // 00c45051
    l_00c45056:
        fld qword ptr [edi + 020h] // 00c45056
        fsub qword ptr [edi + 8] // 00c45059
        fld qword ptr [edi + 028h] // 00c4505c
        fsub qword ptr [edi + 010h] // 00c4505f
        fld qword ptr [edi + 030h] // 00c45062
        fsub qword ptr [edi + 018h] // 00c45065
        fld qword ptr [edi + 8] // 00c45068
        fsub qword ptr [edi + 070h] // 00c4506b
        fld qword ptr [edi + 010h] // 00c4506e
        fsub qword ptr [edi + 078h] // 00c45071
        fld qword ptr [edi + 018h] // 00c45074
        fsub qword ptr [edi + 080h] // 00c45077
        fxch st(1) // 00c4507d
        fmulp st(4), st(0) // 00c4507f
        fxch st(1) // 00c45081
        fmulp st(4), st(0) // 00c45083
        fxch st(2) // 00c45085
        faddp st(3), st(0) // 00c45087
        fmulp st(1), st(0) // 00c45089
        faddp st(1), st(0) // 00c4508b
        fxch st(1) // 00c4508d
        fcomi st(0), st(1) // 00c4508f
        fstp st(1) // 00c45091
        jb l_00c45284 // 00c45093
        fld qword ptr [edi + 8] // 00c45099
        fsub qword ptr [edi + 020h] // 00c4509c
        fld qword ptr [edi + 010h] // 00c4509f
        fsub qword ptr [edi + 028h] // 00c450a2
        fld qword ptr [edi + 018h] // 00c450a5
        fsub qword ptr [edi + 030h] // 00c450a8
        fld qword ptr [edi + 020h] // 00c450ab
        fsub qword ptr [edi + 070h] // 00c450ae
        fld qword ptr [edi + 028h] // 00c450b1
        fsub qword ptr [edi + 078h] // 00c450b4
        fld qword ptr [edi + 030h] // 00c450b7
        fsub qword ptr [edi + 080h] // 00c450ba
        fxch st(1) // 00c450c0
        fmulp st(4), st(0) // 00c450c2
        fxch st(1) // 00c450c4
        fmulp st(4), st(0) // 00c450c6
        fxch st(2) // 00c450c8
        faddp st(3), st(0) // 00c450ca
        fmulp st(1), st(0) // 00c450cc
        faddp st(1), st(0) // 00c450ce
        fxch st(1) // 00c450d0
        fcomi st(0), st(1) // 00c450d2
        fstp st(1) // 00c450d4
        jb l_00c45284 // 00c450d6
        fld qword ptr [edi + 038h] // 00c450dc
        fsub qword ptr [edi + 8] // 00c450df
        fld qword ptr [edi + 040h] // 00c450e2
        fsub qword ptr [edi + 010h] // 00c450e5
        fld qword ptr [edi + 048h] // 00c450e8
        fsub qword ptr [edi + 018h] // 00c450eb
        fld qword ptr [edi + 020h] // 00c450ee
        fsub qword ptr [edi + 8] // 00c450f1
        fld qword ptr [edi + 028h] // 00c450f4
        fsub qword ptr [edi + 010h] // 00c450f7
        fld qword ptr [edi + 030h] // 00c450fa
        fsub qword ptr [edi + 018h] // 00c450fd
        fst qword ptr [esp + 0770h] // 00c45100
        fld st(1) // 00c45107
        fmul st(0), st(4) // 00c45109
        fxch st(1) // 00c4510b
        fmul st(0), st(5) // 00c4510d
        fsubp st(1), st(0) // 00c4510f
        fstp qword ptr [esp + 0790h] // 00c45111
        fld st(4) // 00c45118
        fmul qword ptr [esp + 0770h] // 00c4511a
        fld st(2) // 00c45121
        fmulp st(4), st(0) // 00c45123
        fsubrp st(3), st(0) // 00c45125
        fxch st(1) // 00c45127
        fmulp st(3), st(0) // 00c45129
        fmulp st(3), st(0) // 00c4512b
        fxch st(1) // 00c4512d
        fsubrp st(2), st(0) // 00c4512f
        fld qword ptr [edi + 020h] // 00c45131
        fsub qword ptr [edi + 8] // 00c45134
        fld qword ptr [edi + 028h] // 00c45137
        fsub qword ptr [edi + 010h] // 00c4513a
        fld qword ptr [edi + 030h] // 00c4513d
        fsub qword ptr [edi + 018h] // 00c45140
        fld st(1) // 00c45143
        fmul st(0), st(5) // 00c45145
        fld st(1) // 00c45147
        fmul st(0), st(5) // 00c45149
        fsubp st(1), st(0) // 00c4514b
        fstp qword ptr [esp + 0748h] // 00c4514d
        fld qword ptr [esp + 0790h] // 00c45154
        fld st(0) // 00c4515b
        fmulp st(2), st(0) // 00c4515d
        fld st(3) // 00c4515f
        fmulp st(6), st(0) // 00c45161
        fxch st(1) // 00c45163
        fsubrp st(5), st(0) // 00c45165
        fxch st(2) // 00c45167
        fmulp st(3), st(0) // 00c45169
        fmulp st(1), st(0) // 00c4516b
        fsubp st(1), st(0) // 00c4516d
        fld qword ptr [edi + 8] // 00c4516f
        fsub qword ptr [edi + 070h] // 00c45172
        fld qword ptr [edi + 010h] // 00c45175
        fsub qword ptr [edi + 078h] // 00c45178
        fld qword ptr [edi + 018h] // 00c4517b
        fsub qword ptr [edi + 080h] // 00c4517e
        fxch st(2) // 00c45184
        fmul qword ptr [esp + 0748h] // 00c45186
        fxch st(1) // 00c4518d
        fmulp st(4), st(0) // 00c4518f
        faddp st(3), st(0) // 00c45191
        fmulp st(1), st(0) // 00c45193
        faddp st(1), st(0) // 00c45195
        fxch st(1) // 00c45197
        fcomi st(0), st(1) // 00c45199
        fstp st(1) // 00c4519b
        jbe l_00c45284 // 00c4519d
        fstp st(0) // 00c451a3
        fld qword ptr [edi + 8] // 00c451a5
        fstp qword ptr [edi + 8] // 00c451a8
        fld qword ptr [edi + 010h] // 00c451ab
        fstp qword ptr [edi + 010h] // 00c451ae
        fld qword ptr [edi + 018h] // 00c451b1
        fstp qword ptr [edi + 018h] // 00c451b4
        fld qword ptr [edi + 020h] // 00c451b7
        fstp qword ptr [edi + 020h] // 00c451ba
        fld qword ptr [edi + 028h] // 00c451bd
        fstp qword ptr [edi + 028h] // 00c451c0
        fld qword ptr [edi + 030h] // 00c451c3
        fstp qword ptr [edi + 030h] // 00c451c6
        mov dword ptr [edi + 068h], 2 // 00c451c9
        fld qword ptr [edi + 8] // 00c451d0
        fsub qword ptr [edi + 020h] // 00c451d3
        fld qword ptr [edi + 010h] // 00c451d6
        fsub qword ptr [edi + 028h] // 00c451d9
        fld qword ptr [edi + 018h] // 00c451dc
        fsub qword ptr [edi + 030h] // 00c451df
        fld qword ptr [edi + 8] // 00c451e2
        fsub qword ptr [edi + 070h] // 00c451e5
        fld qword ptr [edi + 010h] // 00c451e8
        fsub qword ptr [edi + 078h] // 00c451eb
        fld qword ptr [edi + 018h] // 00c451ee
        fsub qword ptr [edi + 080h] // 00c451f1
        fst qword ptr [esp + 06e0h] // 00c451f7
        fld qword ptr [edi + 020h] // 00c451fe
        fsub qword ptr [edi + 8] // 00c45201
        fst qword ptr [esp + 0118h] // 00c45204
        fld qword ptr [edi + 028h] // 00c4520b
        fsub qword ptr [edi + 010h] // 00c4520e
        fstp qword ptr [esp + 0120h] // 00c45211
        fld qword ptr [edi + 030h] // 00c45218
        fsub qword ptr [edi + 018h] // 00c4521b
        fstp qword ptr [esp + 0128h] // 00c4521e
        fld qword ptr [esp + 0120h] // 00c45225
        fmulp st(2), st(0) // 00c4522c
        fld qword ptr [esp + 0128h] // 00c4522e
        fmul st(0), st(3) // 00c45235
        fsubp st(2), st(0) // 00c45237
        fxch st(1) // 00c45239
        fstp qword ptr [esp + 0460h] // 00c4523b
        fld st(2) // 00c45242
        fmul qword ptr [esp + 0128h] // 00c45244
        fxch st(1) // 00c4524b
        fmul qword ptr [esp + 06e0h] // 00c4524d
        fsubp st(1), st(0) // 00c45254
        fld qword ptr [esp + 0118h] // 00c45256
        fmulp st(2), st(0) // 00c4525d
        fxch st(2) // 00c4525f
        fmul qword ptr [esp + 0120h] // 00c45261
        fsubp st(1), st(0) // 00c45268
        fld st(1) // 00c4526a
        fmul st(0), st(3) // 00c4526c
        fld st(1) // 00c4526e
        fmul st(0), st(5) // 00c45270
        fsubp st(1), st(0) // 00c45272
        fld st(5) // 00c45274
        fmulp st(2), st(0) // 00c45276
        fld qword ptr [esp + 0460h] // 00c45278
        jmp l_00c44e95 // 00c4527f
    l_00c45284:
        fld qword ptr [edi + 038h] // 00c45284
        fsub qword ptr [edi + 8] // 00c45287
        fld qword ptr [edi + 040h] // 00c4528a
        fsub qword ptr [edi + 010h] // 00c4528d
        fld qword ptr [edi + 048h] // 00c45290
        fsub qword ptr [edi + 018h] // 00c45293
        fld qword ptr [edi + 8] // 00c45296
        fsub qword ptr [edi + 070h] // 00c45299
        fld qword ptr [edi + 010h] // 00c4529c
        fsub qword ptr [edi + 078h] // 00c4529f
        fld qword ptr [edi + 018h] // 00c452a2
        fsub qword ptr [edi + 080h] // 00c452a5
        fxch st(1) // 00c452ab
        fmulp st(4), st(0) // 00c452ad
        fxch st(1) // 00c452af
        fmulp st(4), st(0) // 00c452b1
        fxch st(2) // 00c452b3
        faddp st(3), st(0) // 00c452b5
        fmulp st(1), st(0) // 00c452b7
        faddp st(1), st(0) // 00c452b9
        fxch st(1) // 00c452bb
        fcomi st(0), st(1) // 00c452bd
        fstp st(1) // 00c452bf
        jb l_00c454b0 // 00c452c1
        fld qword ptr [edi + 8] // 00c452c7
        fsub qword ptr [edi + 038h] // 00c452ca
        fld qword ptr [edi + 010h] // 00c452cd
        fsub qword ptr [edi + 040h] // 00c452d0
        fld qword ptr [edi + 018h] // 00c452d3
        fsub qword ptr [edi + 048h] // 00c452d6
        fld qword ptr [edi + 038h] // 00c452d9
        fsub qword ptr [edi + 070h] // 00c452dc
        fld qword ptr [edi + 040h] // 00c452df
        fsub qword ptr [edi + 078h] // 00c452e2
        fld qword ptr [edi + 048h] // 00c452e5
        fsub qword ptr [edi + 080h] // 00c452e8
        fxch st(1) // 00c452ee
        fmulp st(4), st(0) // 00c452f0
        fxch st(1) // 00c452f2
        fmulp st(4), st(0) // 00c452f4
        fxch st(2) // 00c452f6
        faddp st(3), st(0) // 00c452f8
        fmulp st(1), st(0) // 00c452fa
        faddp st(1), st(0) // 00c452fc
        fxch st(1) // 00c452fe
        fcomi st(0), st(1) // 00c45300
        fstp st(1) // 00c45302
        jb l_00c454b0 // 00c45304
        fld qword ptr [edi + 020h] // 00c4530a
        fsub qword ptr [edi + 8] // 00c4530d
        fld qword ptr [edi + 028h] // 00c45310
        fsub qword ptr [edi + 010h] // 00c45313
        fld qword ptr [edi + 030h] // 00c45316
        fsub qword ptr [edi + 018h] // 00c45319
        fld qword ptr [edi + 038h] // 00c4531c
        fsub qword ptr [edi + 8] // 00c4531f
        fld qword ptr [edi + 040h] // 00c45322
        fsub qword ptr [edi + 010h] // 00c45325
        fld qword ptr [edi + 048h] // 00c45328
        fsub qword ptr [edi + 018h] // 00c4532b
        fst qword ptr [esp + 0620h] // 00c4532e
        fld st(1) // 00c45335
        fmul st(0), st(4) // 00c45337
        fxch st(1) // 00c45339
        fmul st(0), st(5) // 00c4533b
        fsubp st(1), st(0) // 00c4533d
        fld qword ptr [esp + 0620h] // 00c4533f
        fmul st(0), st(6) // 00c45346
        fxch st(4) // 00c45348
        fmul st(0), st(3) // 00c4534a
        fsubp st(4), st(0) // 00c4534c
        fxch st(4) // 00c4534e
        fmulp st(2), st(0) // 00c45350
        fmulp st(4), st(0) // 00c45352
        fsubrp st(3), st(0) // 00c45354
        fld qword ptr [edi + 038h] // 00c45356
        fsub qword ptr [edi + 8] // 00c45359
        fld qword ptr [edi + 040h] // 00c4535c
        fsub qword ptr [edi + 010h] // 00c4535f
        fld qword ptr [edi + 048h] // 00c45362
        fsub qword ptr [edi + 018h] // 00c45365
        fst qword ptr [esp + 04a0h] // 00c45368
        fld st(1) // 00c4536f
        fmul st(0), st(6) // 00c45371
        fxch st(1) // 00c45373
        fmul st(0), st(4) // 00c45375
        fsubp st(1), st(0) // 00c45377
        fld qword ptr [esp + 04a0h] // 00c45379
        fmul st(0), st(5) // 00c45380
        fxch st(6) // 00c45382
        fmul st(0), st(3) // 00c45384
        fsubp st(6), st(0) // 00c45386
        fxch st(3) // 00c45388
        fmulp st(2), st(0) // 00c4538a
        fmulp st(3), st(0) // 00c4538c
        fsubrp st(2), st(0) // 00c4538e
        fld qword ptr [edi + 8] // 00c45390
        fsub qword ptr [edi + 070h] // 00c45393
        fld qword ptr [edi + 010h] // 00c45396
        fsub qword ptr [edi + 078h] // 00c45399
        fld qword ptr [edi + 018h] // 00c4539c
        fsub qword ptr [edi + 080h] // 00c4539f
        fxch st(1) // 00c453a5
        fmulp st(5), st(0) // 00c453a7
        fxch st(1) // 00c453a9
        fmulp st(2), st(0) // 00c453ab
        fxch st(3) // 00c453ad
        faddp st(1), st(0) // 00c453af
        fxch st(2) // 00c453b1
        fmulp st(1), st(0) // 00c453b3
        faddp st(1), st(0) // 00c453b5
        fxch st(1) // 00c453b7
        fcomi st(0), st(1) // 00c453b9
        fstp st(1) // 00c453bb
        jbe l_00c454b0 // 00c453bd
        fstp st(0) // 00c453c3
        fld qword ptr [edi + 8] // 00c453c5
        fstp qword ptr [edi + 8] // 00c453c8
        fld qword ptr [edi + 010h] // 00c453cb
        fstp qword ptr [edi + 010h] // 00c453ce
        fld qword ptr [edi + 018h] // 00c453d1
        fstp qword ptr [edi + 018h] // 00c453d4
        fld qword ptr [edi + 038h] // 00c453d7
        fstp qword ptr [edi + 020h] // 00c453da
        fld qword ptr [edi + 040h] // 00c453dd
        fstp qword ptr [edi + 028h] // 00c453e0
        fld qword ptr [edi + 048h] // 00c453e3
        fstp qword ptr [edi + 030h] // 00c453e6
        mov dword ptr [edi + 068h], 2 // 00c453e9
        fld qword ptr [edi + 8] // 00c453f0
        fsub qword ptr [edi + 020h] // 00c453f3
        fld qword ptr [edi + 010h] // 00c453f6
        fsub qword ptr [edi + 028h] // 00c453f9
        fld qword ptr [edi + 018h] // 00c453fc
        fsub qword ptr [edi + 030h] // 00c453ff
        fld qword ptr [edi + 8] // 00c45402
        fsub qword ptr [edi + 070h] // 00c45405
        fld qword ptr [edi + 010h] // 00c45408
        fsub qword ptr [edi + 078h] // 00c4540b
        fld qword ptr [edi + 018h] // 00c4540e
        fsub qword ptr [edi + 080h] // 00c45411
        fld qword ptr [edi + 020h] // 00c45417
        fsub qword ptr [edi + 8] // 00c4541a
        fstp qword ptr [esp + 0298h] // 00c4541d
        fld qword ptr [edi + 028h] // 00c45424
        fsub qword ptr [edi + 010h] // 00c45427
        fst qword ptr [esp + 02a0h] // 00c4542a
        fld qword ptr [edi + 030h] // 00c45431
        fsub qword ptr [edi + 018h] // 00c45434
        fst qword ptr [esp + 02a8h] // 00c45437
        fxch st(1) // 00c4543e
        fmul st(0), st(2) // 00c45440
        fxch st(1) // 00c45442
        fmul st(0), st(3) // 00c45444
        fsubp st(1), st(0) // 00c45446
        fstp qword ptr [esp + 06e8h] // 00c45448
        fld qword ptr [esp + 02a8h] // 00c4544f
        fmul st(0), st(3) // 00c45456
        fld qword ptr [esp + 0298h] // 00c45458
        fmul st(2), st(0) // 00c4545f
        fxch st(1) // 00c45461
        fsubrp st(2), st(0) // 00c45463
        fmulp st(2), st(0) // 00c45465
        fld qword ptr [esp + 02a0h] // 00c45467
        fmulp st(3), st(0) // 00c4546e
        fxch st(1) // 00c45470
        fsubrp st(2), st(0) // 00c45472
        fld st(0) // 00c45474
        fmul st(0), st(3) // 00c45476
        fld st(2) // 00c45478
        fmul st(0), st(5) // 00c4547a
        fsubp st(1), st(0) // 00c4547c
        fxch st(2) // 00c4547e
        fmul st(0), st(5) // 00c45480
        fld qword ptr [esp + 06e8h] // 00c45482
        fmul st(4), st(0) // 00c45489
        fxch st(1) // 00c4548b
        fsubrp st(4), st(0) // 00c4548d
        fmulp st(4), st(0) // 00c4548f
        fmulp st(4), st(0) // 00c45491
        fxch st(2) // 00c45493
        fsubrp st(3), st(0) // 00c45495
        fxch st(1) // 00c45497
        fstp qword ptr [edi + 088h] // 00c45499
        fstp qword ptr [edi + 090h] // 00c4549f
        fstp qword ptr [edi + 098h] // 00c454a5
        jmp l_00c47a00 // 00c454ab
    l_00c454b0:
        fld qword ptr [edi + 038h] // 00c454b0
        fsub qword ptr [edi + 020h] // 00c454b3
        fld qword ptr [edi + 040h] // 00c454b6
        fsub qword ptr [edi + 028h] // 00c454b9
        fld qword ptr [edi + 048h] // 00c454bc
        fsub qword ptr [edi + 030h] // 00c454bf
        fld qword ptr [edi + 020h] // 00c454c2
        fsub qword ptr [edi + 070h] // 00c454c5
        fld qword ptr [edi + 028h] // 00c454c8
        fsub qword ptr [edi + 078h] // 00c454cb
        fld qword ptr [edi + 030h] // 00c454ce
        fsub qword ptr [edi + 080h] // 00c454d1
        fxch st(1) // 00c454d7
        fmulp st(4), st(0) // 00c454d9
        fxch st(1) // 00c454db
        fmulp st(4), st(0) // 00c454dd
        fxch st(2) // 00c454df
        faddp st(3), st(0) // 00c454e1
        fmulp st(1), st(0) // 00c454e3
        faddp st(1), st(0) // 00c454e5
        fxch st(1) // 00c454e7
        fcomi st(0), st(1) // 00c454e9
        fstp st(1) // 00c454eb
        jb l_00c456dc // 00c454ed
        fld qword ptr [edi + 020h] // 00c454f3
        fsub qword ptr [edi + 038h] // 00c454f6
        fld qword ptr [edi + 028h] // 00c454f9
        fsub qword ptr [edi + 040h] // 00c454fc
        fld qword ptr [edi + 030h] // 00c454ff
        fsub qword ptr [edi + 048h] // 00c45502
        fld qword ptr [edi + 038h] // 00c45505
        fsub qword ptr [edi + 070h] // 00c45508
        fld qword ptr [edi + 040h] // 00c4550b
        fsub qword ptr [edi + 078h] // 00c4550e
        fld qword ptr [edi + 048h] // 00c45511
        fsub qword ptr [edi + 080h] // 00c45514
        fxch st(1) // 00c4551a
        fmulp st(4), st(0) // 00c4551c
        fxch st(1) // 00c4551e
        fmulp st(4), st(0) // 00c45520
        fxch st(2) // 00c45522
        faddp st(3), st(0) // 00c45524
        fmulp st(1), st(0) // 00c45526
        faddp st(1), st(0) // 00c45528
        fxch st(1) // 00c4552a
        fcomi st(0), st(1) // 00c4552c
        fstp st(1) // 00c4552e
        jb l_00c456dc // 00c45530
        fld qword ptr [edi + 8] // 00c45536
        fsub qword ptr [edi + 020h] // 00c45539
        fld qword ptr [edi + 010h] // 00c4553c
        fsub qword ptr [edi + 028h] // 00c4553f
        fld qword ptr [edi + 018h] // 00c45542
        fsub qword ptr [edi + 030h] // 00c45545
        fld qword ptr [edi + 038h] // 00c45548
        fsub qword ptr [edi + 020h] // 00c4554b
        fld qword ptr [edi + 040h] // 00c4554e
        fsub qword ptr [edi + 028h] // 00c45551
        fld qword ptr [edi + 048h] // 00c45554
        fsub qword ptr [edi + 030h] // 00c45557
        fst qword ptr [esp + 04d0h] // 00c4555a
        fld st(1) // 00c45561
        fmul st(0), st(4) // 00c45563
        fxch st(1) // 00c45565
        fmul st(0), st(5) // 00c45567
        fsubp st(1), st(0) // 00c45569
        fstp qword ptr [esp + 0640h] // 00c4556b
        fld st(4) // 00c45572
        fmul qword ptr [esp + 04d0h] // 00c45574
        fld st(2) // 00c4557b
        fmulp st(4), st(0) // 00c4557d
        fsubrp st(3), st(0) // 00c4557f
        fxch st(1) // 00c45581
        fmulp st(3), st(0) // 00c45583
        fmulp st(3), st(0) // 00c45585
        fxch st(1) // 00c45587
        fsubrp st(2), st(0) // 00c45589
        fld qword ptr [edi + 038h] // 00c4558b
        fsub qword ptr [edi + 020h] // 00c4558e
        fld qword ptr [edi + 040h] // 00c45591
        fsub qword ptr [edi + 028h] // 00c45594
        fld qword ptr [edi + 048h] // 00c45597
        fsub qword ptr [edi + 030h] // 00c4559a
        fld st(1) // 00c4559d
        fmul st(0), st(5) // 00c4559f
        fld st(1) // 00c455a1
        fmul st(0), st(5) // 00c455a3
        fsubp st(1), st(0) // 00c455a5
        fstp qword ptr [esp + 04f0h] // 00c455a7
        fld qword ptr [esp + 0640h] // 00c455ae
        fld st(0) // 00c455b5
        fmulp st(2), st(0) // 00c455b7
        fld st(3) // 00c455b9
        fmulp st(6), st(0) // 00c455bb
        fxch st(1) // 00c455bd
        fsubrp st(5), st(0) // 00c455bf
        fxch st(2) // 00c455c1
        fmulp st(3), st(0) // 00c455c3
        fmulp st(1), st(0) // 00c455c5
        fsubp st(1), st(0) // 00c455c7
        fld qword ptr [edi + 020h] // 00c455c9
        fsub qword ptr [edi + 070h] // 00c455cc
        fld qword ptr [edi + 028h] // 00c455cf
        fsub qword ptr [edi + 078h] // 00c455d2
        fld qword ptr [edi + 030h] // 00c455d5
        fsub qword ptr [edi + 080h] // 00c455d8
        fxch st(2) // 00c455de
        fmul qword ptr [esp + 04f0h] // 00c455e0
        fxch st(1) // 00c455e7
        fmulp st(4), st(0) // 00c455e9
        faddp st(3), st(0) // 00c455eb
        fmulp st(1), st(0) // 00c455ed
        faddp st(1), st(0) // 00c455ef
        fxch st(1) // 00c455f1
        fcomip st(0), st(1) // 00c455f3
        fstp st(0) // 00c455f5
        jbe l_00c456de // 00c455f7
        fld qword ptr [edi + 020h] // 00c455fd
        fstp qword ptr [edi + 8] // 00c45600
        fld qword ptr [edi + 028h] // 00c45603
        fstp qword ptr [edi + 010h] // 00c45606
        fld qword ptr [edi + 030h] // 00c45609
        fstp qword ptr [edi + 018h] // 00c4560c
        fld qword ptr [edi + 038h] // 00c4560f
        fstp qword ptr [edi + 020h] // 00c45612
        fld qword ptr [edi + 040h] // 00c45615
        fstp qword ptr [edi + 028h] // 00c45618
        fld qword ptr [edi + 048h] // 00c4561b
        fstp qword ptr [edi + 030h] // 00c4561e
        mov dword ptr [edi + 068h], 2 // 00c45621
        fld qword ptr [edi + 8] // 00c45628
        fsub qword ptr [edi + 020h] // 00c4562b
        fld qword ptr [edi + 010h] // 00c4562e
        fsub qword ptr [edi + 028h] // 00c45631
        fld qword ptr [edi + 018h] // 00c45634
        fsub qword ptr [edi + 030h] // 00c45637
        fld qword ptr [edi + 8] // 00c4563a
        fsub qword ptr [edi + 070h] // 00c4563d
        fld qword ptr [edi + 010h] // 00c45640
        fsub qword ptr [edi + 078h] // 00c45643
        fld qword ptr [edi + 018h] // 00c45646
        fsub qword ptr [edi + 080h] // 00c45649
        fst qword ptr [esp + 0710h] // 00c4564f
        fld qword ptr [edi + 020h] // 00c45656
        fsub qword ptr [edi + 8] // 00c45659
        fst qword ptr [esp + 0130h] // 00c4565c
        fld qword ptr [edi + 028h] // 00c45663
        fsub qword ptr [edi + 010h] // 00c45666
        fstp qword ptr [esp + 0138h] // 00c45669
        fld qword ptr [edi + 030h] // 00c45670
        fsub qword ptr [edi + 018h] // 00c45673
        fstp qword ptr [esp + 0140h] // 00c45676
        fld qword ptr [esp + 0138h] // 00c4567d
        fmulp st(2), st(0) // 00c45684
        fld qword ptr [esp + 0140h] // 00c45686
        fmul st(0), st(3) // 00c4568d
        fsubp st(2), st(0) // 00c4568f
        fxch st(1) // 00c45691
        fstp qword ptr [esp + 0520h] // 00c45693
        fld st(2) // 00c4569a
        fmul qword ptr [esp + 0140h] // 00c4569c
        fxch st(1) // 00c456a3
        fmul qword ptr [esp + 0710h] // 00c456a5
        fsubp st(1), st(0) // 00c456ac
        fld qword ptr [esp + 0130h] // 00c456ae
        fmulp st(2), st(0) // 00c456b5
        fxch st(2) // 00c456b7
        fmul qword ptr [esp + 0138h] // 00c456b9
        fsubp st(1), st(0) // 00c456c0
        fld st(1) // 00c456c2
        fmul st(0), st(3) // 00c456c4
        fld st(1) // 00c456c6
        fmul st(0), st(5) // 00c456c8
        fsubp st(1), st(0) // 00c456ca
        fld st(5) // 00c456cc
        fmulp st(2), st(0) // 00c456ce
        fld qword ptr [esp + 0520h] // 00c456d0
        jmp l_00c44e95 // 00c456d7
    l_00c456dc:
        fstp st(0) // 00c456dc
    l_00c456de:
        fld qword ptr [edi + 020h] // 00c456de
        lea ecx, [esp + 058h] // 00c456e1
        fsub qword ptr [edi + 8] // 00c456e5
        lea edx, [esp + 038h] // 00c456e8
        lea eax, [esp + 07a8h] // 00c456ec
        fstp qword ptr [esp + 058h] // 00c456f3
        fld qword ptr [edi + 028h] // 00c456f7
        fsub qword ptr [edi + 010h] // 00c456fa
        fstp qword ptr [esp + 060h] // 00c456fd
        fld qword ptr [edi + 030h] // 00c45701
        fsub qword ptr [edi + 018h] // 00c45704
        fstp qword ptr [esp + 068h] // 00c45707
        fld qword ptr [edi + 038h] // 00c4570b
        fsub qword ptr [edi + 8] // 00c4570e
        fstp qword ptr [esp + 038h] // 00c45711
        fld qword ptr [edi + 040h] // 00c45715
        fsub qword ptr [edi + 010h] // 00c45718
        fstp qword ptr [esp + 040h] // 00c4571b
        fld qword ptr [edi + 048h] // 00c4571f
        fsub qword ptr [edi + 018h] // 00c45722
        fstp qword ptr [esp + 048h] // 00c45725
        call cross_reference // 00c45729
        fld qword ptr [eax] // 00c4572e
        lea ecx, [edi + 088h] // 00c45730
        fstp qword ptr [ecx] // 00c45736
        sub esp, 8 // 00c45738
        fld qword ptr [eax + 8] // 00c4573b
        fstp qword ptr [ecx + 8] // 00c4573e
        fld qword ptr [eax + 010h] // 00c45741
        mov eax, ecx // 00c45744
        fstp qword ptr [ecx + 010h] // 00c45746
        fld qword ptr [edi + 070h] // 00c45749
        fsub qword ptr [edi + 8] // 00c4574c
        fld qword ptr [edi + 078h] // 00c4574f
        fsub qword ptr [edi + 010h] // 00c45752
        fld qword ptr [edi + 080h] // 00c45755
        fsub qword ptr [edi + 018h] // 00c4575b
        fld qword ptr [ecx + 8] // 00c4575e
        fmulp st(2), st(0) // 00c45761
        fxch st(2) // 00c45763
        fmul qword ptr [ecx] // 00c45765
        faddp st(1), st(0) // 00c45767
        fld qword ptr [ecx + 010h] // 00c45769
        fmulp st(2), st(0) // 00c4576c
        faddp st(1), st(0) // 00c4576e
        fstp qword ptr [esp] // 00c45770
        call scale_reference // 00c45773
        jmp l_00c47a00 // 00c45778
    l_00c4577d:
        fstp st(0) // 00c4577d
        lea esi, [edi + 8] // 00c4577f
        fstp st(1) // 00c45782
        lea ebx, [edi + 020h] // 00c45784
        fstp st(0) // 00c45787
        fld qword ptr [ebx] // 00c45789
        fsub qword ptr [esi] // 00c4578b
        fld qword ptr [ebx + 8] // 00c4578d
        fsub qword ptr [esi + 8] // 00c45790
        fld qword ptr [ebx + 010h] // 00c45793
        fsub qword ptr [esi + 010h] // 00c45796
        fld qword ptr [esi] // 00c45799
        fsub qword ptr [edi + 070h] // 00c4579b
        fld qword ptr [esi + 8] // 00c4579e
        fsub qword ptr [edi + 078h] // 00c457a1
        fld qword ptr [esi + 010h] // 00c457a4
        fsub qword ptr [edi + 080h] // 00c457a7
        fxch st(1) // 00c457ad
        fmulp st(4), st(0) // 00c457af
        fxch st(1) // 00c457b1
        fmulp st(4), st(0) // 00c457b3
        fxch st(2) // 00c457b5
        faddp st(3), st(0) // 00c457b7
        fmulp st(1), st(0) // 00c457b9
        faddp st(1), st(0) // 00c457bb
        fcomip st(0), st(1) // 00c457bd
        jb l_00c4587d // 00c457bf
        fld qword ptr [edi + 038h] // 00c457c5
        fsub qword ptr [esi] // 00c457c8
        fld qword ptr [edi + 040h] // 00c457ca
        fsub qword ptr [esi + 8] // 00c457cd
        fld qword ptr [edi + 048h] // 00c457d0
        fsub qword ptr [esi + 010h] // 00c457d3
        fld qword ptr [esi] // 00c457d6
        fsub qword ptr [edi + 070h] // 00c457d8
        fld qword ptr [esi + 8] // 00c457db
        fsub qword ptr [edi + 078h] // 00c457de
        fld qword ptr [esi + 010h] // 00c457e1
        fsub qword ptr [edi + 080h] // 00c457e4
        fxch st(1) // 00c457ea
        fmulp st(4), st(0) // 00c457ec
        fxch st(1) // 00c457ee
        fmulp st(4), st(0) // 00c457f0
        fxch st(2) // 00c457f2
        faddp st(3), st(0) // 00c457f4
        fmulp st(1), st(0) // 00c457f6
        faddp st(1), st(0) // 00c457f8
        fcomip st(0), st(1) // 00c457fa
        jb l_00c4587d // 00c457fc
        fld qword ptr [edi + 050h] // 00c457fe
        fsub qword ptr [esi] // 00c45801
        fld qword ptr [edi + 058h] // 00c45803
        fsub qword ptr [esi + 8] // 00c45806
        fld qword ptr [edi + 060h] // 00c45809
        fsub qword ptr [esi + 010h] // 00c4580c
        fld qword ptr [esi] // 00c4580f
        fsub qword ptr [edi + 070h] // 00c45811
        fld qword ptr [esi + 8] // 00c45814
        fsub qword ptr [edi + 078h] // 00c45817
        fld qword ptr [esi + 010h] // 00c4581a
        fsub qword ptr [edi + 080h] // 00c4581d
        fxch st(1) // 00c45823
        fmulp st(4), st(0) // 00c45825
        fxch st(1) // 00c45827
        fmulp st(4), st(0) // 00c45829
        fxch st(2) // 00c4582b
        faddp st(3), st(0) // 00c4582d
        fmulp st(1), st(0) // 00c4582f
        faddp st(1), st(0) // 00c45831
        fcomip st(0), st(1) // 00c45833
        jb l_00c4587d // 00c45835
        fstp st(0) // 00c45837
        fld qword ptr [esi] // 00c45839
        fstp qword ptr [esi] // 00c4583b
        fld qword ptr [esi + 8] // 00c4583d
        fstp qword ptr [esi + 8] // 00c45840
        fld qword ptr [esi + 010h] // 00c45843
        fstp qword ptr [esi + 010h] // 00c45846
        fld qword ptr [edi + 070h] // 00c45849
        fsub qword ptr [esi] // 00c4584c
        fld qword ptr [edi + 078h] // 00c4584e
        fsub qword ptr [esi + 8] // 00c45851
        fld qword ptr [edi + 080h] // 00c45854
        fsub qword ptr [esi + 010h] // 00c4585a
        fxch st(2) // 00c4585d
        fstp qword ptr [edi + 088h] // 00c4585f
        fstp qword ptr [edi + 090h] // 00c45865
        fstp qword ptr [edi + 098h] // 00c4586b
        mov dword ptr [edi + 068h], 1 // 00c45871
        jmp l_00c47a00 // 00c45878
    l_00c4587d:
        fld qword ptr [esi] // 00c4587d
        fsub qword ptr [ebx] // 00c4587f
        fld qword ptr [esi + 8] // 00c45881
        fsub qword ptr [ebx + 8] // 00c45884
        fld qword ptr [esi + 010h] // 00c45887
        fsub qword ptr [ebx + 010h] // 00c4588a
        fld qword ptr [ebx] // 00c4588d
        fsub qword ptr [edi + 070h] // 00c4588f
        fld qword ptr [ebx + 8] // 00c45892
        fsub qword ptr [edi + 078h] // 00c45895
        fld qword ptr [ebx + 010h] // 00c45898
        fsub qword ptr [edi + 080h] // 00c4589b
        fxch st(1) // 00c458a1
        fmulp st(4), st(0) // 00c458a3
        fxch st(1) // 00c458a5
        fmulp st(4), st(0) // 00c458a7
        fxch st(2) // 00c458a9
        faddp st(3), st(0) // 00c458ab
        fmulp st(1), st(0) // 00c458ad
        faddp st(1), st(0) // 00c458af
        fcomip st(0), st(1) // 00c458b1
        jb l_00c45971 // 00c458b3
        fld qword ptr [edi + 038h] // 00c458b9
        fsub qword ptr [ebx] // 00c458bc
        fld qword ptr [edi + 040h] // 00c458be
        fsub qword ptr [ebx + 8] // 00c458c1
        fld qword ptr [edi + 048h] // 00c458c4
        fsub qword ptr [ebx + 010h] // 00c458c7
        fld qword ptr [ebx] // 00c458ca
        fsub qword ptr [edi + 070h] // 00c458cc
        fld qword ptr [ebx + 8] // 00c458cf
        fsub qword ptr [edi + 078h] // 00c458d2
        fld qword ptr [ebx + 010h] // 00c458d5
        fsub qword ptr [edi + 080h] // 00c458d8
        fxch st(1) // 00c458de
        fmulp st(4), st(0) // 00c458e0
        fxch st(1) // 00c458e2
        fmulp st(4), st(0) // 00c458e4
        fxch st(2) // 00c458e6
        faddp st(3), st(0) // 00c458e8
        fmulp st(1), st(0) // 00c458ea
        faddp st(1), st(0) // 00c458ec
        fcomip st(0), st(1) // 00c458ee
        jb l_00c45971 // 00c458f0
        fld qword ptr [edi + 050h] // 00c458f2
        fsub qword ptr [ebx] // 00c458f5
        fld qword ptr [edi + 058h] // 00c458f7
        fsub qword ptr [ebx + 8] // 00c458fa
        fld qword ptr [edi + 060h] // 00c458fd
        fsub qword ptr [ebx + 010h] // 00c45900
        fld qword ptr [ebx] // 00c45903
        fsub qword ptr [edi + 070h] // 00c45905
        fld qword ptr [ebx + 8] // 00c45908
        fsub qword ptr [edi + 078h] // 00c4590b
        fld qword ptr [ebx + 010h] // 00c4590e
        fsub qword ptr [edi + 080h] // 00c45911
        fxch st(1) // 00c45917
        fmulp st(4), st(0) // 00c45919
        fxch st(1) // 00c4591b
        fmulp st(4), st(0) // 00c4591d
        fxch st(2) // 00c4591f
        faddp st(3), st(0) // 00c45921
        fmulp st(1), st(0) // 00c45923
        faddp st(1), st(0) // 00c45925
        fcomip st(0), st(1) // 00c45927
        jb l_00c45971 // 00c45929
        fstp st(0) // 00c4592b
        fld qword ptr [ebx] // 00c4592d
        fstp qword ptr [esi] // 00c4592f
        fld qword ptr [ebx + 8] // 00c45931
        fstp qword ptr [esi + 8] // 00c45934
        fld qword ptr [ebx + 010h] // 00c45937
        fstp qword ptr [esi + 010h] // 00c4593a
        fld qword ptr [edi + 070h] // 00c4593d
        fsub qword ptr [esi] // 00c45940
        fld qword ptr [edi + 078h] // 00c45942
        fsub qword ptr [esi + 8] // 00c45945
        fld qword ptr [edi + 080h] // 00c45948
        fsub qword ptr [esi + 010h] // 00c4594e
        fxch st(2) // 00c45951
        fstp qword ptr [edi + 088h] // 00c45953
        fstp qword ptr [edi + 090h] // 00c45959
        fstp qword ptr [edi + 098h] // 00c4595f
        mov dword ptr [edi + 068h], 1 // 00c45965
        jmp l_00c47a00 // 00c4596c
    l_00c45971:
        fld qword ptr [esi] // 00c45971
        fsub qword ptr [edi + 038h] // 00c45973
        fld qword ptr [esi + 8] // 00c45976
        fsub qword ptr [edi + 040h] // 00c45979
        fld qword ptr [esi + 010h] // 00c4597c
        fsub qword ptr [edi + 048h] // 00c4597f
        fld qword ptr [edi + 038h] // 00c45982
        fsub qword ptr [edi + 070h] // 00c45985
        fld qword ptr [edi + 040h] // 00c45988
        fsub qword ptr [edi + 078h] // 00c4598b
        fld qword ptr [edi + 048h] // 00c4598e
        fsub qword ptr [edi + 080h] // 00c45991
        fxch st(1) // 00c45997
        fmulp st(4), st(0) // 00c45999
        fxch st(1) // 00c4599b
        fmulp st(4), st(0) // 00c4599d
        fxch st(2) // 00c4599f
        faddp st(3), st(0) // 00c459a1
        fmulp st(1), st(0) // 00c459a3
        faddp st(1), st(0) // 00c459a5
        fcomip st(0), st(1) // 00c459a7
        jb l_00c45a6f // 00c459a9
        fld qword ptr [ebx] // 00c459af
        fsub qword ptr [edi + 038h] // 00c459b1
        fld qword ptr [ebx + 8] // 00c459b4
        fsub qword ptr [edi + 040h] // 00c459b7
        fld qword ptr [ebx + 010h] // 00c459ba
        fsub qword ptr [edi + 048h] // 00c459bd
        fld qword ptr [edi + 038h] // 00c459c0
        fsub qword ptr [edi + 070h] // 00c459c3
        fld qword ptr [edi + 040h] // 00c459c6
        fsub qword ptr [edi + 078h] // 00c459c9
        fld qword ptr [edi + 048h] // 00c459cc
        fsub qword ptr [edi + 080h] // 00c459cf
        fxch st(1) // 00c459d5
        fmulp st(4), st(0) // 00c459d7
        fxch st(1) // 00c459d9
        fmulp st(4), st(0) // 00c459db
        fxch st(2) // 00c459dd
        faddp st(3), st(0) // 00c459df
        fmulp st(1), st(0) // 00c459e1
        faddp st(1), st(0) // 00c459e3
        fcomip st(0), st(1) // 00c459e5
        jb l_00c45a6f // 00c459e7
        fld qword ptr [edi + 050h] // 00c459ed
        fsub qword ptr [edi + 038h] // 00c459f0
        fld qword ptr [edi + 058h] // 00c459f3
        fsub qword ptr [edi + 040h] // 00c459f6
        fld qword ptr [edi + 060h] // 00c459f9
        fsub qword ptr [edi + 048h] // 00c459fc
        fld qword ptr [edi + 038h] // 00c459ff
        fsub qword ptr [edi + 070h] // 00c45a02
        fld qword ptr [edi + 040h] // 00c45a05
        fsub qword ptr [edi + 078h] // 00c45a08
        fld qword ptr [edi + 048h] // 00c45a0b
        fsub qword ptr [edi + 080h] // 00c45a0e
        fxch st(1) // 00c45a14
        fmulp st(4), st(0) // 00c45a16
        fxch st(1) // 00c45a18
        fmulp st(4), st(0) // 00c45a1a
        fxch st(2) // 00c45a1c
        faddp st(3), st(0) // 00c45a1e
        fmulp st(1), st(0) // 00c45a20
        faddp st(1), st(0) // 00c45a22
        fcomip st(0), st(1) // 00c45a24
        jb l_00c45a6f // 00c45a26
        fstp st(0) // 00c45a28
        fld qword ptr [edi + 038h] // 00c45a2a
        fstp qword ptr [esi] // 00c45a2d
        fld qword ptr [edi + 040h] // 00c45a2f
        fstp qword ptr [esi + 8] // 00c45a32
        fld qword ptr [edi + 048h] // 00c45a35
        fstp qword ptr [esi + 010h] // 00c45a38
        fld qword ptr [edi + 070h] // 00c45a3b
        fsub qword ptr [esi] // 00c45a3e
        fld qword ptr [edi + 078h] // 00c45a40
        fsub qword ptr [esi + 8] // 00c45a43
        fld qword ptr [edi + 080h] // 00c45a46
        fsub qword ptr [esi + 010h] // 00c45a4c
        fxch st(2) // 00c45a4f
        fstp qword ptr [edi + 088h] // 00c45a51
        fstp qword ptr [edi + 090h] // 00c45a57
        fstp qword ptr [edi + 098h] // 00c45a5d
        mov dword ptr [edi + 068h], 1 // 00c45a63
        jmp l_00c47a00 // 00c45a6a
    l_00c45a6f:
        fld qword ptr [esi] // 00c45a6f
        fsub qword ptr [edi + 050h] // 00c45a71
        fld qword ptr [esi + 8] // 00c45a74
        fsub qword ptr [edi + 058h] // 00c45a77
        fld qword ptr [esi + 010h] // 00c45a7a
        fsub qword ptr [edi + 060h] // 00c45a7d
        fld qword ptr [edi + 050h] // 00c45a80
        fsub qword ptr [edi + 070h] // 00c45a83
        fld qword ptr [edi + 058h] // 00c45a86
        fsub qword ptr [edi + 078h] // 00c45a89
        fld qword ptr [edi + 060h] // 00c45a8c
        fsub qword ptr [edi + 080h] // 00c45a8f
        fxch st(1) // 00c45a95
        fmulp st(4), st(0) // 00c45a97
        fxch st(1) // 00c45a99
        fmulp st(4), st(0) // 00c45a9b
        fxch st(2) // 00c45a9d
        faddp st(3), st(0) // 00c45a9f
        fmulp st(1), st(0) // 00c45aa1
        faddp st(1), st(0) // 00c45aa3
        fcomip st(0), st(1) // 00c45aa5
        jb l_00c45b6d // 00c45aa7
        fld qword ptr [ebx] // 00c45aad
        fsub qword ptr [edi + 050h] // 00c45aaf
        fld qword ptr [ebx + 8] // 00c45ab2
        fsub qword ptr [edi + 058h] // 00c45ab5
        fld qword ptr [ebx + 010h] // 00c45ab8
        fsub qword ptr [edi + 060h] // 00c45abb
        fld qword ptr [edi + 050h] // 00c45abe
        fsub qword ptr [edi + 070h] // 00c45ac1
        fld qword ptr [edi + 058h] // 00c45ac4
        fsub qword ptr [edi + 078h] // 00c45ac7
        fld qword ptr [edi + 060h] // 00c45aca
        fsub qword ptr [edi + 080h] // 00c45acd
        fxch st(1) // 00c45ad3
        fmulp st(4), st(0) // 00c45ad5
        fxch st(1) // 00c45ad7
        fmulp st(4), st(0) // 00c45ad9
        fxch st(2) // 00c45adb
        faddp st(3), st(0) // 00c45add
        fmulp st(1), st(0) // 00c45adf
        faddp st(1), st(0) // 00c45ae1
        fcomip st(0), st(1) // 00c45ae3
        jb l_00c45b6d // 00c45ae5
        fld qword ptr [edi + 038h] // 00c45aeb
        fsub qword ptr [edi + 050h] // 00c45aee
        fld qword ptr [edi + 040h] // 00c45af1
        fsub qword ptr [edi + 058h] // 00c45af4
        fld qword ptr [edi + 048h] // 00c45af7
        fsub qword ptr [edi + 060h] // 00c45afa
        fld qword ptr [edi + 050h] // 00c45afd
        fsub qword ptr [edi + 070h] // 00c45b00
        fld qword ptr [edi + 058h] // 00c45b03
        fsub qword ptr [edi + 078h] // 00c45b06
        fld qword ptr [edi + 060h] // 00c45b09
        fsub qword ptr [edi + 080h] // 00c45b0c
        fxch st(1) // 00c45b12
        fmulp st(4), st(0) // 00c45b14
        fxch st(1) // 00c45b16
        fmulp st(4), st(0) // 00c45b18
        fxch st(2) // 00c45b1a
        faddp st(3), st(0) // 00c45b1c
        fmulp st(1), st(0) // 00c45b1e
        faddp st(1), st(0) // 00c45b20
        fcomip st(0), st(1) // 00c45b22
        jb l_00c45b6d // 00c45b24
        fstp st(0) // 00c45b26
        fld qword ptr [edi + 050h] // 00c45b28
        fstp qword ptr [esi] // 00c45b2b
        fld qword ptr [edi + 058h] // 00c45b2d
        fstp qword ptr [esi + 8] // 00c45b30
        fld qword ptr [edi + 060h] // 00c45b33
        fstp qword ptr [esi + 010h] // 00c45b36
        fld qword ptr [edi + 070h] // 00c45b39
        fsub qword ptr [esi] // 00c45b3c
        fld qword ptr [edi + 078h] // 00c45b3e
        fsub qword ptr [esi + 8] // 00c45b41
        fld qword ptr [edi + 080h] // 00c45b44
        fsub qword ptr [esi + 010h] // 00c45b4a
        fxch st(2) // 00c45b4d
        fstp qword ptr [edi + 088h] // 00c45b4f
        fstp qword ptr [edi + 090h] // 00c45b55
        fstp qword ptr [edi + 098h] // 00c45b5b
        mov dword ptr [edi + 068h], 1 // 00c45b61
        jmp l_00c47a00 // 00c45b68
    l_00c45b6d:
        fld qword ptr [edi + 038h] // 00c45b6d
        fsub qword ptr [esi] // 00c45b70
        fld qword ptr [edi + 040h] // 00c45b72
        fsub qword ptr [esi + 8] // 00c45b75
        fld qword ptr [edi + 048h] // 00c45b78
        fsub qword ptr [esi + 010h] // 00c45b7b
        fld qword ptr [ebx] // 00c45b7e
        fsub qword ptr [esi] // 00c45b80
        fld qword ptr [ebx + 8] // 00c45b82
        fsub qword ptr [esi + 8] // 00c45b85
        fld qword ptr [ebx + 010h] // 00c45b88
        fsub qword ptr [esi + 010h] // 00c45b8b
        fst qword ptr [esp + 0680h] // 00c45b8e
        fld st(1) // 00c45b95
        fmul st(0), st(4) // 00c45b97
        fxch st(1) // 00c45b99
        fmul st(0), st(5) // 00c45b9b
        fsubp st(1), st(0) // 00c45b9d
        fstp qword ptr [esp + 0100h] // 00c45b9f
        fld st(4) // 00c45ba6
        fmul qword ptr [esp + 0680h] // 00c45ba8
        fld st(2) // 00c45baf
        fmulp st(4), st(0) // 00c45bb1
        fsubrp st(3), st(0) // 00c45bb3
        fxch st(2) // 00c45bb5
        fstp qword ptr [esp + 0108h] // 00c45bb7
        fmulp st(2), st(0) // 00c45bbe
        fmulp st(2), st(0) // 00c45bc0
        fsubrp st(1), st(0) // 00c45bc2
        fstp qword ptr [esp + 0110h] // 00c45bc4
        fld qword ptr [ebx] // 00c45bcb
        fsub qword ptr [esi] // 00c45bcd
        fld qword ptr [ebx + 8] // 00c45bcf
        fsub qword ptr [esi + 8] // 00c45bd2
        fld qword ptr [ebx + 010h] // 00c45bd5
        fsub qword ptr [esi + 010h] // 00c45bd8
        fld qword ptr [edi + 050h] // 00c45bdb
        fsub qword ptr [esi] // 00c45bde
        fld qword ptr [edi + 058h] // 00c45be0
        fsub qword ptr [esi + 8] // 00c45be3
        fld qword ptr [edi + 060h] // 00c45be6
        fsub qword ptr [esi + 010h] // 00c45be9
        fst qword ptr [esp + 0560h] // 00c45bec
        fld st(1) // 00c45bf3
        fmul st(0), st(4) // 00c45bf5
        fxch st(1) // 00c45bf7
        fmul st(0), st(5) // 00c45bf9
        fsubp st(1), st(0) // 00c45bfb
        fstp qword ptr [esp + 03a0h] // 00c45bfd
        fld st(4) // 00c45c04
        fmul qword ptr [esp + 0560h] // 00c45c06
        fld st(2) // 00c45c0d
        fmulp st(4), st(0) // 00c45c0f
        fsubrp st(3), st(0) // 00c45c11
        fxch st(1) // 00c45c13
        fmulp st(3), st(0) // 00c45c15
        fmulp st(3), st(0) // 00c45c17
        fxch st(1) // 00c45c19
        fsubrp st(2), st(0) // 00c45c1b
        fxch st(1) // 00c45c1d
        fstp qword ptr [esp + 03b0h] // 00c45c1f
        fld qword ptr [ebx] // 00c45c26
        fsub qword ptr [esi] // 00c45c28
        fld qword ptr [ebx + 8] // 00c45c2a
        fsub qword ptr [esi + 8] // 00c45c2d
        fld qword ptr [ebx + 010h] // 00c45c30
        fsub qword ptr [esi + 010h] // 00c45c33
        fld qword ptr [esi] // 00c45c36
        fsub qword ptr [edi + 070h] // 00c45c38
        fld qword ptr [esi + 8] // 00c45c3b
        fsub qword ptr [edi + 078h] // 00c45c3e
        fld qword ptr [esi + 010h] // 00c45c41
        fsub qword ptr [edi + 080h] // 00c45c44
        fxch st(1) // 00c45c4a
        fmulp st(4), st(0) // 00c45c4c
        fxch st(1) // 00c45c4e
        fmulp st(4), st(0) // 00c45c50
        fxch st(2) // 00c45c52
        faddp st(3), st(0) // 00c45c54
        fmulp st(1), st(0) // 00c45c56
        faddp st(1), st(0) // 00c45c58
        fxch st(2) // 00c45c5a
        fcomi st(0), st(2) // 00c45c5c
        fstp st(2) // 00c45c5e
        jb l_00c45e7a // 00c45c60
        fld qword ptr [esi] // 00c45c66
        fsub qword ptr [ebx] // 00c45c68
        fld qword ptr [esi + 8] // 00c45c6a
        fsub qword ptr [ebx + 8] // 00c45c6d
        fld qword ptr [esi + 010h] // 00c45c70
        fsub qword ptr [ebx + 010h] // 00c45c73
        fld qword ptr [ebx] // 00c45c76
        fsub qword ptr [edi + 070h] // 00c45c78
        fld qword ptr [ebx + 8] // 00c45c7b
        fsub qword ptr [edi + 078h] // 00c45c7e
        fld qword ptr [ebx + 010h] // 00c45c81
        fsub qword ptr [edi + 080h] // 00c45c84
        fxch st(1) // 00c45c8a
        fmulp st(4), st(0) // 00c45c8c
        fxch st(1) // 00c45c8e
        fmulp st(4), st(0) // 00c45c90
        fxch st(2) // 00c45c92
        faddp st(3), st(0) // 00c45c94
        fmulp st(1), st(0) // 00c45c96
        faddp st(1), st(0) // 00c45c98
        fxch st(2) // 00c45c9a
        fcomi st(0), st(2) // 00c45c9c
        fstp st(2) // 00c45c9e
        jb l_00c45e7a // 00c45ca0
        fld qword ptr [ebx] // 00c45ca6
        fsub qword ptr [esi] // 00c45ca8
        fld qword ptr [ebx + 8] // 00c45caa
        fsub qword ptr [esi + 8] // 00c45cad
        fld qword ptr [ebx + 010h] // 00c45cb0
        fsub qword ptr [esi + 010h] // 00c45cb3
        fld st(1) // 00c45cb6
        fmul qword ptr [esp + 0110h] // 00c45cb8
        fld st(1) // 00c45cbf
        fmul qword ptr [esp + 0108h] // 00c45cc1
        fsubp st(1), st(0) // 00c45cc8
        fld qword ptr [esp + 0100h] // 00c45cca
        fld st(0) // 00c45cd1
        fmulp st(3), st(0) // 00c45cd3
        fld st(4) // 00c45cd5
        fmul qword ptr [esp + 0110h] // 00c45cd7
        fsubp st(3), st(0) // 00c45cde
        fxch st(4) // 00c45ce0
        fmul qword ptr [esp + 0108h] // 00c45ce2
        fxch st(4) // 00c45ce9
        fmulp st(3), st(0) // 00c45ceb
        fxch st(3) // 00c45ced
        fsubrp st(2), st(0) // 00c45cef
        fld qword ptr [esi] // 00c45cf1
        fsub qword ptr [edi + 070h] // 00c45cf3
        fld qword ptr [esi + 8] // 00c45cf6
        fsub qword ptr [edi + 078h] // 00c45cf9
        fld qword ptr [esi + 010h] // 00c45cfc
        fsub qword ptr [edi + 080h] // 00c45cff
        fxch st(1) // 00c45d05
        fmulp st(3), st(0) // 00c45d07
        fxch st(1) // 00c45d09
        fmulp st(4), st(0) // 00c45d0b
        fxch st(1) // 00c45d0d
        faddp st(3), st(0) // 00c45d0f
        fmulp st(1), st(0) // 00c45d11
        faddp st(1), st(0) // 00c45d13
        fxch st(2) // 00c45d15
        fcomi st(0), st(2) // 00c45d17
        fstp st(2) // 00c45d19
        jb l_00c45e7a // 00c45d1b
        fld qword ptr [ebx] // 00c45d21
        fsub qword ptr [esi] // 00c45d23
        fld qword ptr [ebx + 8] // 00c45d25
        fsub qword ptr [esi + 8] // 00c45d28
        fld qword ptr [ebx + 010h] // 00c45d2b
        fsub qword ptr [esi + 010h] // 00c45d2e
        fld st(0) // 00c45d31
        fmul st(0), st(4) // 00c45d33
        fld st(2) // 00c45d35
        fld qword ptr [esp + 03b0h] // 00c45d37
        fmul st(1), st(0) // 00c45d3e
        fxch st(2) // 00c45d40
        fsubrp st(1), st(0) // 00c45d42
        fld st(4) // 00c45d44
        fmulp st(2), st(0) // 00c45d46
        fld qword ptr [esp + 03a0h] // 00c45d48
        fmulp st(3), st(0) // 00c45d4f
        fxch st(1) // 00c45d51
        fsubrp st(2), st(0) // 00c45d53
        fld qword ptr [esp + 03a0h] // 00c45d55
        fmulp st(3), st(0) // 00c45d5c
        fxch st(3) // 00c45d5e
        fmulp st(4), st(0) // 00c45d60
        fxch st(1) // 00c45d62
        fsubrp st(3), st(0) // 00c45d64
        fld qword ptr [esi] // 00c45d66
        fsub qword ptr [edi + 070h] // 00c45d68
        fld qword ptr [esi + 8] // 00c45d6b
        fsub qword ptr [edi + 078h] // 00c45d6e
        fld qword ptr [esi + 010h] // 00c45d71
        fsub qword ptr [edi + 080h] // 00c45d74
        fxch st(2) // 00c45d7a
        fmulp st(4), st(0) // 00c45d7c
        fmulp st(2), st(0) // 00c45d7e
        fxch st(2) // 00c45d80
        faddp st(1), st(0) // 00c45d82
        fxch st(1) // 00c45d84
        fmulp st(2), st(0) // 00c45d86
        faddp st(1), st(0) // 00c45d88
        fxch st(1) // 00c45d8a
        fcomi st(0), st(1) // 00c45d8c
        fstp st(1) // 00c45d8e
        jb l_00c45e7c // 00c45d90
        fstp st(0) // 00c45d96
        fld qword ptr [esi] // 00c45d98
        fstp qword ptr [esi] // 00c45d9a
        fld qword ptr [esi + 8] // 00c45d9c
        fstp qword ptr [esi + 8] // 00c45d9f
        fld qword ptr [esi + 010h] // 00c45da2
        fstp qword ptr [esi + 010h] // 00c45da5
        fld qword ptr [ebx] // 00c45da8
        fstp qword ptr [ebx] // 00c45daa
        fld qword ptr [ebx + 8] // 00c45dac
        fstp qword ptr [ebx + 8] // 00c45daf
        fld qword ptr [ebx + 010h] // 00c45db2
        fstp qword ptr [ebx + 010h] // 00c45db5
        fld qword ptr [esi] // 00c45db8
        fsub qword ptr [ebx] // 00c45dba
        fld qword ptr [esi + 8] // 00c45dbc
        fsub qword ptr [ebx + 8] // 00c45dbf
        fld qword ptr [esi + 010h] // 00c45dc2
        fsub qword ptr [ebx + 010h] // 00c45dc5
        fld qword ptr [esi] // 00c45dc8
        fsub qword ptr [edi + 070h] // 00c45dca
        fld qword ptr [esi + 8] // 00c45dcd
        fsub qword ptr [edi + 078h] // 00c45dd0
        fld qword ptr [esi + 010h] // 00c45dd3
        fsub qword ptr [edi + 080h] // 00c45dd6
        fld qword ptr [ebx] // 00c45ddc
        fsub qword ptr [esi] // 00c45dde
        fstp qword ptr [esp + 02c8h] // 00c45de0
        fld qword ptr [ebx + 8] // 00c45de7
        fsub qword ptr [esi + 8] // 00c45dea
        fst qword ptr [esp + 02d0h] // 00c45ded
        fld qword ptr [ebx + 010h] // 00c45df4
        fsub qword ptr [esi + 010h] // 00c45df7
        fst qword ptr [esp + 02d8h] // 00c45dfa
        fxch st(1) // 00c45e01
        fmul st(0), st(2) // 00c45e03
        fxch st(1) // 00c45e05
        fmul st(0), st(3) // 00c45e07
        fsubp st(1), st(0) // 00c45e09
        fstp qword ptr [esp + 0718h] // 00c45e0b
        fld qword ptr [esp + 02d8h] // 00c45e12
        fmul st(0), st(3) // 00c45e19
        fld qword ptr [esp + 02c8h] // 00c45e1b
        fmul st(2), st(0) // 00c45e22
        fxch st(1) // 00c45e24
        fsubrp st(2), st(0) // 00c45e26
        fmulp st(2), st(0) // 00c45e28
        fld qword ptr [esp + 02d0h] // 00c45e2a
        fmulp st(3), st(0) // 00c45e31
        fxch st(1) // 00c45e33
        fsubrp st(2), st(0) // 00c45e35
        fld st(0) // 00c45e37
        fmul st(0), st(3) // 00c45e39
        fld st(2) // 00c45e3b
        fmul st(0), st(5) // 00c45e3d
        fsubp st(1), st(0) // 00c45e3f
        fld st(5) // 00c45e41
        fmulp st(3), st(0) // 00c45e43
        fld qword ptr [esp + 0718h] // 00c45e45
        fmul st(4), st(0) // 00c45e4c
        fxch st(3) // 00c45e4e
        fsubrp st(4), st(0) // 00c45e50
        fxch st(4) // 00c45e52
        fmulp st(2), st(0) // 00c45e54
        fmulp st(4), st(0) // 00c45e56
        fsubrp st(3), st(0) // 00c45e58
        fxch st(1) // 00c45e5a
        fstp qword ptr [edi + 088h] // 00c45e5c
        fstp qword ptr [edi + 090h] // 00c45e62
        fstp qword ptr [edi + 098h] // 00c45e68
        mov dword ptr [edi + 068h], 2 // 00c45e6e
        jmp l_00c47a00 // 00c45e75
    l_00c45e7a:
        fstp st(0) // 00c45e7a
    l_00c45e7c:
        fld qword ptr [ebx] // 00c45e7c
        fsub qword ptr [esi] // 00c45e7e
        fld qword ptr [ebx + 8] // 00c45e80
        fsub qword ptr [esi + 8] // 00c45e83
        fld qword ptr [ebx + 010h] // 00c45e86
        fsub qword ptr [esi + 010h] // 00c45e89
        fld qword ptr [edi + 038h] // 00c45e8c
        fsub qword ptr [esi] // 00c45e8f
        fld qword ptr [edi + 040h] // 00c45e91
        fsub qword ptr [esi + 8] // 00c45e94
        fld qword ptr [edi + 048h] // 00c45e97
        fsub qword ptr [esi + 010h] // 00c45e9a
        fst qword ptr [esp + 0590h] // 00c45e9d
        fld st(1) // 00c45ea4
        fmul st(0), st(4) // 00c45ea6
        fxch st(1) // 00c45ea8
        fmul st(0), st(5) // 00c45eaa
        fsubp st(1), st(0) // 00c45eac
        fstp qword ptr [esp + 02f8h] // 00c45eae
        fld qword ptr [esp + 0590h] // 00c45eb5
        fmul st(0), st(5) // 00c45ebc
        fxch st(3) // 00c45ebe
        fmul st(0), st(2) // 00c45ec0
        fsubp st(3), st(0) // 00c45ec2
        fxch st(2) // 00c45ec4
        fstp qword ptr [esp + 0300h] // 00c45ec6
        fmulp st(2), st(0) // 00c45ecd
        fmulp st(2), st(0) // 00c45ecf
        fsubrp st(1), st(0) // 00c45ed1
        fstp qword ptr [esp + 0308h] // 00c45ed3
        fld qword ptr [edi + 038h] // 00c45eda
        fsub qword ptr [esi] // 00c45edd
        fld qword ptr [edi + 040h] // 00c45edf
        fsub qword ptr [esi + 8] // 00c45ee2
        fld qword ptr [edi + 048h] // 00c45ee5
        fsub qword ptr [esi + 010h] // 00c45ee8
        fld qword ptr [edi + 050h] // 00c45eeb
        fsub qword ptr [esi] // 00c45eee
        fld qword ptr [edi + 058h] // 00c45ef0
        fsub qword ptr [esi + 8] // 00c45ef3
        fld qword ptr [edi + 060h] // 00c45ef6
        fsub qword ptr [esi + 010h] // 00c45ef9
        fst qword ptr [esp + 06b0h] // 00c45efc
        fld st(1) // 00c45f03
        fmul st(0), st(4) // 00c45f05
        fxch st(1) // 00c45f07
        fmul st(0), st(5) // 00c45f09
        fsubp st(1), st(0) // 00c45f0b
        fstp qword ptr [esp + 058h] // 00c45f0d
        fld qword ptr [esp + 06b0h] // 00c45f11
        fmul st(0), st(5) // 00c45f18
        fxch st(3) // 00c45f1a
        fmul st(0), st(2) // 00c45f1c
        fsubp st(3), st(0) // 00c45f1e
        fxch st(2) // 00c45f20
        fstp qword ptr [esp + 060h] // 00c45f22
        fmulp st(2), st(0) // 00c45f26
        fmulp st(2), st(0) // 00c45f28
        fsubrp st(1), st(0) // 00c45f2a
        fstp qword ptr [esp + 068h] // 00c45f2c
        fld qword ptr [edi + 038h] // 00c45f30
        fsub qword ptr [esi] // 00c45f33
        fld qword ptr [edi + 040h] // 00c45f35
        fsub qword ptr [esi + 8] // 00c45f38
        fld qword ptr [edi + 048h] // 00c45f3b
        fsub qword ptr [esi + 010h] // 00c45f3e
        fld qword ptr [esi] // 00c45f41
        fsub qword ptr [edi + 070h] // 00c45f43
        fld qword ptr [esi + 8] // 00c45f46
        fsub qword ptr [edi + 078h] // 00c45f49
        fld qword ptr [esi + 010h] // 00c45f4c
        fsub qword ptr [edi + 080h] // 00c45f4f
        fxch st(2) // 00c45f55
        fmulp st(5), st(0) // 00c45f57
        fmulp st(3), st(0) // 00c45f59
        fxch st(3) // 00c45f5b
        faddp st(2), st(0) // 00c45f5d
        fmulp st(2), st(0) // 00c45f5f
        faddp st(1), st(0) // 00c45f61
        fxch st(1) // 00c45f63
        fcomi st(0), st(1) // 00c45f65
        fstp st(1) // 00c45f67
        jb l_00c46173 // 00c45f69
        fld qword ptr [esi] // 00c45f6f
        fsub qword ptr [edi + 038h] // 00c45f71
        fld qword ptr [esi + 8] // 00c45f74
        fsub qword ptr [edi + 040h] // 00c45f77
        fld qword ptr [esi + 010h] // 00c45f7a
        fsub qword ptr [edi + 048h] // 00c45f7d
        fld qword ptr [edi + 038h] // 00c45f80
        fsub qword ptr [edi + 070h] // 00c45f83
        fld qword ptr [edi + 040h] // 00c45f86
        fsub qword ptr [edi + 078h] // 00c45f89
        fld qword ptr [edi + 048h] // 00c45f8c
        fsub qword ptr [edi + 080h] // 00c45f8f
        fxch st(2) // 00c45f95
        fmulp st(5), st(0) // 00c45f97
        fmulp st(3), st(0) // 00c45f99
        fxch st(3) // 00c45f9b
        faddp st(2), st(0) // 00c45f9d
        fmulp st(2), st(0) // 00c45f9f
        faddp st(1), st(0) // 00c45fa1
        fxch st(1) // 00c45fa3
        fcomi st(0), st(1) // 00c45fa5
        fstp st(1) // 00c45fa7
        jb l_00c46173 // 00c45fa9
        fld qword ptr [edi + 038h] // 00c45faf
        fsub qword ptr [esi] // 00c45fb2
        fld qword ptr [edi + 040h] // 00c45fb4
        fsub qword ptr [esi + 8] // 00c45fb7
        fld qword ptr [edi + 048h] // 00c45fba
        fsub qword ptr [esi + 010h] // 00c45fbd
        fld st(1) // 00c45fc0
        fld qword ptr [esp + 0308h] // 00c45fc2
        fmul st(1), st(0) // 00c45fc9
        fld st(2) // 00c45fcb
        fld qword ptr [esp + 0300h] // 00c45fcd
        fmul st(1), st(0) // 00c45fd4
        fxch st(3) // 00c45fd6
        fsubrp st(1), st(0) // 00c45fd8
        fld qword ptr [esp + 02f8h] // 00c45fda
        fmul st(4), st(0) // 00c45fe1
        fxch st(2) // 00c45fe3
        fmul st(0), st(6) // 00c45fe5
        fsubp st(4), st(0) // 00c45fe7
        fxch st(2) // 00c45fe9
        fmulp st(5), st(0) // 00c45feb
        fmulp st(3), st(0) // 00c45fed
        fxch st(3) // 00c45fef
        fsubrp st(2), st(0) // 00c45ff1
        fld qword ptr [esi] // 00c45ff3
        fsub qword ptr [edi + 070h] // 00c45ff5
        fld qword ptr [esi + 8] // 00c45ff8
        fsub qword ptr [edi + 078h] // 00c45ffb
        fld qword ptr [esi + 010h] // 00c45ffe
        fsub qword ptr [edi + 080h] // 00c46001
        fxch st(2) // 00c46007
        fmulp st(5), st(0) // 00c46009
        fmulp st(2), st(0) // 00c4600b
        fxch st(3) // 00c4600d
        faddp st(1), st(0) // 00c4600f
        fxch st(2) // 00c46011
        fmulp st(1), st(0) // 00c46013
        faddp st(1), st(0) // 00c46015
        fxch st(1) // 00c46017
        fcomi st(0), st(1) // 00c46019
        fstp st(1) // 00c4601b
        jb l_00c46173 // 00c4601d
        fstp st(0) // 00c46023
        lea ecx, [esp + 038h] // 00c46025
        fld qword ptr [edi + 038h] // 00c46029
        lea edx, [esp + 058h] // 00c4602c
        fsub qword ptr [esi] // 00c46030
        lea eax, [esp + 0e8h] // 00c46032
        fstp qword ptr [esp + 038h] // 00c46039
        fld qword ptr [edi + 040h] // 00c4603d
        fsub qword ptr [esi + 8] // 00c46040
        fstp qword ptr [esp + 040h] // 00c46043
        fld qword ptr [edi + 048h] // 00c46047
        fsub qword ptr [esi + 010h] // 00c4604a
        fstp qword ptr [esp + 048h] // 00c4604d
        fld qword ptr [esi] // 00c46051
        fsub qword ptr [edi + 070h] // 00c46053
        fstp qword ptr [esp + 0328h] // 00c46056
        fld qword ptr [esi + 8] // 00c4605d
        fsub qword ptr [edi + 078h] // 00c46060
        fstp qword ptr [esp + 0330h] // 00c46063
        fld qword ptr [esi + 010h] // 00c4606a
        fsub qword ptr [edi + 080h] // 00c4606d
        fstp qword ptr [esp + 0338h] // 00c46073
        call cross_reference // 00c4607a
        fld qword ptr [eax + 8] // 00c4607f
        fmul qword ptr [esp + 0330h] // 00c46082
        fld qword ptr [esp + 0328h] // 00c46089
        fmul qword ptr [eax] // 00c46090
        faddp st(1), st(0) // 00c46092
        fld qword ptr [eax + 010h] // 00c46094
        fmul qword ptr [esp + 0338h] // 00c46097
        faddp st(1), st(0) // 00c4609e
        fldz  // 00c460a0
        fcomi st(0), st(1) // 00c460a2
        fstp st(1) // 00c460a4
        jb l_00c46173 // 00c460a6
        fstp st(0) // 00c460ac
        lea ecx, [esp + 038h] // 00c460ae
        fld qword ptr [esi] // 00c460b2
        lea edx, [esp + 058h] // 00c460b4
        fstp qword ptr [esi] // 00c460b8
        lea eax, [esp + 0a0h] // 00c460ba
        fld qword ptr [esi + 8] // 00c460c1
        fstp qword ptr [esi + 8] // 00c460c4
        fld qword ptr [esi + 010h] // 00c460c7
        fstp qword ptr [esi + 010h] // 00c460ca
        fld qword ptr [edi + 038h] // 00c460cd
        fstp qword ptr [ebx] // 00c460d0
        fld qword ptr [edi + 040h] // 00c460d2
        fstp qword ptr [ebx + 8] // 00c460d5
        fld qword ptr [edi + 048h] // 00c460d8
        fstp qword ptr [ebx + 010h] // 00c460db
        fld qword ptr [esi] // 00c460de
        fsub qword ptr [ebx] // 00c460e0
        fstp qword ptr [esp + 020h] // 00c460e2
        fld qword ptr [esi + 8] // 00c460e6
        fsub qword ptr [ebx + 8] // 00c460e9
        fstp qword ptr [esp + 028h] // 00c460ec
        fld qword ptr [esi + 010h] // 00c460f0
        fsub qword ptr [ebx + 010h] // 00c460f3
        fstp qword ptr [esp + 030h] // 00c460f6
        fld qword ptr [esi] // 00c460fa
        fsub qword ptr [edi + 070h] // 00c460fc
        fstp qword ptr [esp + 038h] // 00c460ff
        fld qword ptr [esi + 8] // 00c46103
        fsub qword ptr [edi + 078h] // 00c46106
        fstp qword ptr [esp + 040h] // 00c46109
        fld qword ptr [esi + 010h] // 00c4610d
        fsub qword ptr [edi + 080h] // 00c46110
        fstp qword ptr [esp + 048h] // 00c46116
        fld qword ptr [ebx] // 00c4611a
        fsub qword ptr [esi] // 00c4611c
        fstp qword ptr [esp + 058h] // 00c4611e
        fld qword ptr [ebx + 8] // 00c46122
        fsub qword ptr [esi + 8] // 00c46125
        fstp qword ptr [esp + 060h] // 00c46128
        fld qword ptr [ebx + 010h] // 00c4612c
        fsub qword ptr [esi + 010h] // 00c4612f
        fstp qword ptr [esp + 068h] // 00c46132
        call cross_reference // 00c46136
        mov edx, eax // 00c4613b
        lea ecx, [esp + 020h] // 00c4613d
        lea eax, [esp + 088h] // 00c46141
    l_00c46148:
        call cross_reference // 00c46148
        fld qword ptr [eax] // 00c4614d
        fstp qword ptr [edi + 088h] // 00c4614f
        fld qword ptr [eax + 8] // 00c46155
        fstp qword ptr [edi + 090h] // 00c46158
        fld qword ptr [eax + 010h] // 00c4615e
        fstp qword ptr [edi + 098h] // 00c46161
        mov dword ptr [edi + 068h], 2 // 00c46167
        jmp l_00c47a00 // 00c4616e
    l_00c46173:
        fld qword ptr [esi] // 00c46173
        fsub qword ptr [ebx] // 00c46175
        fld qword ptr [esi + 8] // 00c46177
        fsub qword ptr [ebx + 8] // 00c4617a
        fld qword ptr [esi + 010h] // 00c4617d
        fsub qword ptr [ebx + 010h] // 00c46180
        fld qword ptr [edi + 038h] // 00c46183
        fsub qword ptr [ebx] // 00c46186
        fld qword ptr [edi + 040h] // 00c46188
        fsub qword ptr [ebx + 8] // 00c4618b
        fld qword ptr [edi + 048h] // 00c4618e
        fsub qword ptr [ebx + 010h] // 00c46191
        fst qword ptr [esp + 05c0h] // 00c46194
        fld st(1) // 00c4619b
        fmul st(0), st(4) // 00c4619d
        fxch st(1) // 00c4619f
        fmul st(0), st(5) // 00c461a1
        fsubp st(1), st(0) // 00c461a3
        fstp qword ptr [esp + 01c0h] // 00c461a5
        fld qword ptr [esp + 05c0h] // 00c461ac
        fmul st(0), st(5) // 00c461b3
        fxch st(3) // 00c461b5
        fmul st(0), st(2) // 00c461b7
        fsubp st(3), st(0) // 00c461b9
        fxch st(2) // 00c461bb
        fstp qword ptr [esp + 01c8h] // 00c461bd
        fmulp st(2), st(0) // 00c461c4
        fmulp st(2), st(0) // 00c461c6
        fsubrp st(1), st(0) // 00c461c8
        fstp qword ptr [esp + 01d0h] // 00c461ca
        fld qword ptr [edi + 038h] // 00c461d1
        fsub qword ptr [ebx] // 00c461d4
        fld qword ptr [edi + 040h] // 00c461d6
        fsub qword ptr [ebx + 8] // 00c461d9
        fld qword ptr [edi + 048h] // 00c461dc
        fsub qword ptr [ebx + 010h] // 00c461df
        fld qword ptr [edi + 050h] // 00c461e2
        fsub qword ptr [ebx] // 00c461e5
        fld qword ptr [edi + 058h] // 00c461e7
        fsub qword ptr [ebx + 8] // 00c461ea
        fld qword ptr [edi + 060h] // 00c461ed
        fsub qword ptr [ebx + 010h] // 00c461f0
        fst qword ptr [esp + 0788h] // 00c461f3
        fld st(1) // 00c461fa
        fmul st(0), st(4) // 00c461fc
        fxch st(1) // 00c461fe
        fmul st(0), st(5) // 00c46200
        fsubp st(1), st(0) // 00c46202
        fstp qword ptr [esp + 038h] // 00c46204
        fld st(4) // 00c46208
        fmul qword ptr [esp + 0788h] // 00c4620a
        fld st(2) // 00c46211
        fmulp st(4), st(0) // 00c46213
        fsubrp st(3), st(0) // 00c46215
        fxch st(2) // 00c46217
        fstp qword ptr [esp + 040h] // 00c46219
        fmulp st(2), st(0) // 00c4621d
        fmulp st(2), st(0) // 00c4621f
        fsubrp st(1), st(0) // 00c46221
        fstp qword ptr [esp + 048h] // 00c46223
        fld qword ptr [edi + 038h] // 00c46227
        fsub qword ptr [ebx] // 00c4622a
        fld qword ptr [edi + 040h] // 00c4622c
        fsub qword ptr [ebx + 8] // 00c4622f
        fld qword ptr [edi + 048h] // 00c46232
        fsub qword ptr [ebx + 010h] // 00c46235
        fld qword ptr [ebx] // 00c46238
        fsub qword ptr [edi + 070h] // 00c4623a
        fld qword ptr [ebx + 8] // 00c4623d
        fsub qword ptr [edi + 078h] // 00c46240
        fld qword ptr [ebx + 010h] // 00c46243
        fsub qword ptr [edi + 080h] // 00c46246
        fxch st(2) // 00c4624c
        fmulp st(5), st(0) // 00c4624e
        fmulp st(3), st(0) // 00c46250
        fxch st(3) // 00c46252
        faddp st(2), st(0) // 00c46254
        fmulp st(2), st(0) // 00c46256
        faddp st(1), st(0) // 00c46258
        fxch st(1) // 00c4625a
        fcomi st(0), st(1) // 00c4625c
        fstp st(1) // 00c4625e
        jb l_00c46447 // 00c46260
        fld qword ptr [ebx] // 00c46266
        fsub qword ptr [edi + 038h] // 00c46268
        fld qword ptr [ebx + 8] // 00c4626b
        fsub qword ptr [edi + 040h] // 00c4626e
        fld qword ptr [ebx + 010h] // 00c46271
        fsub qword ptr [edi + 048h] // 00c46274
        fld qword ptr [edi + 038h] // 00c46277
        fsub qword ptr [edi + 070h] // 00c4627a
        fld qword ptr [edi + 040h] // 00c4627d
        fsub qword ptr [edi + 078h] // 00c46280
        fld qword ptr [edi + 048h] // 00c46283
        fsub qword ptr [edi + 080h] // 00c46286
        fxch st(2) // 00c4628c
        fmulp st(5), st(0) // 00c4628e
        fmulp st(3), st(0) // 00c46290
        fxch st(3) // 00c46292
        faddp st(2), st(0) // 00c46294
        fmulp st(2), st(0) // 00c46296
        faddp st(1), st(0) // 00c46298
        fxch st(1) // 00c4629a
        fcomi st(0), st(1) // 00c4629c
        fstp st(1) // 00c4629e
        jb l_00c46447 // 00c462a0
        fld qword ptr [edi + 038h] // 00c462a6
        fsub qword ptr [ebx] // 00c462a9
        fld qword ptr [edi + 040h] // 00c462ab
        fsub qword ptr [ebx + 8] // 00c462ae
        fld qword ptr [edi + 048h] // 00c462b1
        fsub qword ptr [ebx + 010h] // 00c462b4
        fld st(1) // 00c462b7
        fld qword ptr [esp + 01d0h] // 00c462b9
        fmul st(1), st(0) // 00c462c0
        fld st(2) // 00c462c2
        fmul qword ptr [esp + 01c8h] // 00c462c4
        fsubp st(2), st(0) // 00c462cb
        fld qword ptr [esp + 01c0h] // 00c462cd
        fmul st(3), st(0) // 00c462d4
        fld st(5) // 00c462d6
        fmulp st(2), st(0) // 00c462d8
        fxch st(3) // 00c462da
        fsubrp st(1), st(0) // 00c462dc
        fxch st(4) // 00c462de
        fmul qword ptr [esp + 01c8h] // 00c462e0
        fxch st(3) // 00c462e7
        fmulp st(2), st(0) // 00c462e9
        fxch st(2) // 00c462eb
        fsubrp st(1), st(0) // 00c462ed
        fld qword ptr [ebx] // 00c462ef
        fsub qword ptr [edi + 070h] // 00c462f1
        fld qword ptr [ebx + 8] // 00c462f4
        fsub qword ptr [edi + 078h] // 00c462f7
        fld qword ptr [ebx + 010h] // 00c462fa
        fsub qword ptr [edi + 080h] // 00c462fd
        fxch st(2) // 00c46303
        fmulp st(4), st(0) // 00c46305
        fmulp st(4), st(0) // 00c46307
        fxch st(2) // 00c46309
        faddp st(3), st(0) // 00c4630b
        fmulp st(1), st(0) // 00c4630d
        faddp st(1), st(0) // 00c4630f
        fxch st(1) // 00c46311
        fcomi st(0), st(1) // 00c46313
        fstp st(1) // 00c46315
        jb l_00c46447 // 00c46317
        fstp st(0) // 00c4631d
        lea ecx, [esp + 020h] // 00c4631f
        fld qword ptr [edi + 038h] // 00c46323
        lea edx, [esp + 038h] // 00c46326
        fsub qword ptr [ebx] // 00c4632a
        lea eax, [esp + 088h] // 00c4632c
        fstp qword ptr [esp + 020h] // 00c46333
        fld qword ptr [edi + 040h] // 00c46337
        fsub qword ptr [ebx + 8] // 00c4633a
        fstp qword ptr [esp + 028h] // 00c4633d
        fld qword ptr [edi + 048h] // 00c46341
        fsub qword ptr [ebx + 010h] // 00c46344
        fstp qword ptr [esp + 030h] // 00c46347
        fld qword ptr [ebx] // 00c4634b
        fsub qword ptr [edi + 070h] // 00c4634d
        fstp qword ptr [esp + 0358h] // 00c46350
        fld qword ptr [ebx + 8] // 00c46357
        fsub qword ptr [edi + 078h] // 00c4635a
        fstp qword ptr [esp + 0360h] // 00c4635d
        fld qword ptr [ebx + 010h] // 00c46364
        fsub qword ptr [edi + 080h] // 00c46367
        fstp qword ptr [esp + 0368h] // 00c4636d
        call cross_reference // 00c46374
        fld qword ptr [eax + 8] // 00c46379
        fmul qword ptr [esp + 0360h] // 00c4637c
        fld qword ptr [esp + 0358h] // 00c46383
        fmul qword ptr [eax] // 00c4638a
        faddp st(1), st(0) // 00c4638c
        fld qword ptr [eax + 010h] // 00c4638e
        fmul qword ptr [esp + 0368h] // 00c46391
        faddp st(1), st(0) // 00c46398
        fldz  // 00c4639a
        fcomi st(0), st(1) // 00c4639c
        fstp st(1) // 00c4639e
        jb l_00c46447 // 00c463a0
        fstp st(0) // 00c463a6
        lea ecx, [esp + 020h] // 00c463a8
        fld qword ptr [ebx] // 00c463ac
        lea edx, [esp + 038h] // 00c463ae
        fstp qword ptr [esi] // 00c463b2
        lea eax, [esp + 0a0h] // 00c463b4
        fld qword ptr [ebx + 8] // 00c463bb
        fstp qword ptr [esi + 8] // 00c463be
        fld qword ptr [ebx + 010h] // 00c463c1
        fstp qword ptr [esi + 010h] // 00c463c4
        fld qword ptr [edi + 038h] // 00c463c7
        fstp qword ptr [ebx] // 00c463ca
        fld qword ptr [edi + 040h] // 00c463cc
        fstp qword ptr [ebx + 8] // 00c463cf
        fld qword ptr [edi + 048h] // 00c463d2
        fstp qword ptr [ebx + 010h] // 00c463d5
        fld qword ptr [esi] // 00c463d8
        fsub qword ptr [ebx] // 00c463da
        fstp qword ptr [esp + 058h] // 00c463dc
        fld qword ptr [esi + 8] // 00c463e0
        fsub qword ptr [ebx + 8] // 00c463e3
        fstp qword ptr [esp + 060h] // 00c463e6
        fld qword ptr [esi + 010h] // 00c463ea
        fsub qword ptr [ebx + 010h] // 00c463ed
        fstp qword ptr [esp + 068h] // 00c463f0
        fld qword ptr [esi] // 00c463f4
        fsub qword ptr [edi + 070h] // 00c463f6
        fstp qword ptr [esp + 020h] // 00c463f9
        fld qword ptr [esi + 8] // 00c463fd
        fsub qword ptr [edi + 078h] // 00c46400
        fstp qword ptr [esp + 028h] // 00c46403
        fld qword ptr [esi + 010h] // 00c46407
        fsub qword ptr [edi + 080h] // 00c4640a
        fstp qword ptr [esp + 030h] // 00c46410
        fld qword ptr [ebx] // 00c46414
        fsub qword ptr [esi] // 00c46416
        fstp qword ptr [esp + 038h] // 00c46418
        fld qword ptr [ebx + 8] // 00c4641c
        fsub qword ptr [esi + 8] // 00c4641f
        fstp qword ptr [esp + 040h] // 00c46422
        fld qword ptr [ebx + 010h] // 00c46426
        fsub qword ptr [esi + 010h] // 00c46429
        fstp qword ptr [esp + 048h] // 00c4642c
        call cross_reference // 00c46430
        mov edx, eax // 00c46435
        lea ecx, [esp + 058h] // 00c46437
        lea eax, [esp + 0e8h] // 00c4643b
        jmp l_00c46148 // 00c46442
    l_00c46447:
        fld qword ptr [ebx] // 00c46447
        fsub qword ptr [esi] // 00c46449
        fld qword ptr [ebx + 8] // 00c4644b
        fsub qword ptr [esi + 8] // 00c4644e
        fld qword ptr [ebx + 010h] // 00c46451
        fsub qword ptr [esi + 010h] // 00c46454
        fld qword ptr [edi + 050h] // 00c46457
        fsub qword ptr [esi] // 00c4645a
        fld qword ptr [edi + 058h] // 00c4645c
        fsub qword ptr [esi + 8] // 00c4645f
        fld qword ptr [edi + 060h] // 00c46462
        fsub qword ptr [esi + 010h] // 00c46465
        fst qword ptr [esp + 05f0h] // 00c46468
        fld st(1) // 00c4646f
        fmul st(0), st(4) // 00c46471
        fxch st(1) // 00c46473
        fmul st(0), st(5) // 00c46475
        fsubp st(1), st(0) // 00c46477
        fstp qword ptr [esp + 01d8h] // 00c46479
        fld st(4) // 00c46480
        fmul qword ptr [esp + 05f0h] // 00c46482
        fld st(2) // 00c46489
        fmulp st(4), st(0) // 00c4648b
        fsubrp st(3), st(0) // 00c4648d
        fxch st(2) // 00c4648f
        fstp qword ptr [esp + 01e0h] // 00c46491
        fmulp st(2), st(0) // 00c46498
        fmulp st(2), st(0) // 00c4649a
        fsubrp st(1), st(0) // 00c4649c
        fstp qword ptr [esp + 01e8h] // 00c4649e
        fld qword ptr [edi + 050h] // 00c464a5
        fsub qword ptr [esi] // 00c464a8
        fld qword ptr [edi + 058h] // 00c464aa
        fsub qword ptr [esi + 8] // 00c464ad
        fld qword ptr [edi + 060h] // 00c464b0
        fsub qword ptr [esi + 010h] // 00c464b3
        fld qword ptr [edi + 038h] // 00c464b6
        fsub qword ptr [esi] // 00c464b9
        fld qword ptr [edi + 040h] // 00c464bb
        fsub qword ptr [esi + 8] // 00c464be
        fld qword ptr [edi + 048h] // 00c464c1
        fsub qword ptr [esi + 010h] // 00c464c4
        fst qword ptr [esp + 0440h] // 00c464c7
        fld st(1) // 00c464ce
        fmul st(0), st(4) // 00c464d0
        fxch st(1) // 00c464d2
        fmul st(0), st(5) // 00c464d4
        fsubp st(1), st(0) // 00c464d6
        fstp qword ptr [esp + 038h] // 00c464d8
        fld st(4) // 00c464dc
        fmul qword ptr [esp + 0440h] // 00c464de
        fld st(2) // 00c464e5
        fmulp st(4), st(0) // 00c464e7
        fsubrp st(3), st(0) // 00c464e9
        fxch st(2) // 00c464eb
        fstp qword ptr [esp + 040h] // 00c464ed
        fmulp st(2), st(0) // 00c464f1
        fmulp st(2), st(0) // 00c464f3
        fsubrp st(1), st(0) // 00c464f5
        fstp qword ptr [esp + 048h] // 00c464f7
        fld qword ptr [edi + 050h] // 00c464fb
        fsub qword ptr [esi] // 00c464fe
        fld qword ptr [edi + 058h] // 00c46500
        fsub qword ptr [esi + 8] // 00c46503
        fld qword ptr [edi + 060h] // 00c46506
        fsub qword ptr [esi + 010h] // 00c46509
        fld qword ptr [esi] // 00c4650c
        fsub qword ptr [edi + 070h] // 00c4650e
        fld qword ptr [esi + 8] // 00c46511
        fsub qword ptr [edi + 078h] // 00c46514
        fld qword ptr [esi + 010h] // 00c46517
        fsub qword ptr [edi + 080h] // 00c4651a
        fxch st(1) // 00c46520
        fmulp st(4), st(0) // 00c46522
        fxch st(1) // 00c46524
        fmulp st(4), st(0) // 00c46526
        fxch st(2) // 00c46528
        faddp st(3), st(0) // 00c4652a
        fmulp st(1), st(0) // 00c4652c
        faddp st(1), st(0) // 00c4652e
        fxch st(1) // 00c46530
        fcomi st(0), st(1) // 00c46532
        fstp st(1) // 00c46534
        jb l_00c46725 // 00c46536
        fld qword ptr [esi] // 00c4653c
        fsub qword ptr [edi + 050h] // 00c4653e
        fld qword ptr [esi + 8] // 00c46541
        fsub qword ptr [edi + 058h] // 00c46544
        fld qword ptr [esi + 010h] // 00c46547
        fsub qword ptr [edi + 060h] // 00c4654a
        fld qword ptr [edi + 050h] // 00c4654d
        fsub qword ptr [edi + 070h] // 00c46550
        fld qword ptr [edi + 058h] // 00c46553
        fsub qword ptr [edi + 078h] // 00c46556
        fld qword ptr [edi + 060h] // 00c46559
        fsub qword ptr [edi + 080h] // 00c4655c
        fxch st(1) // 00c46562
        fmulp st(4), st(0) // 00c46564
        fxch st(1) // 00c46566
        fmulp st(4), st(0) // 00c46568
        fxch st(2) // 00c4656a
        faddp st(3), st(0) // 00c4656c
        fmulp st(1), st(0) // 00c4656e
        faddp st(1), st(0) // 00c46570
        fxch st(1) // 00c46572
        fcomi st(0), st(1) // 00c46574
        fstp st(1) // 00c46576
        jb l_00c46725 // 00c46578
        fld qword ptr [edi + 050h] // 00c4657e
        fsub qword ptr [esi] // 00c46581
        fld qword ptr [edi + 058h] // 00c46583
        fsub qword ptr [esi + 8] // 00c46586
        fld qword ptr [edi + 060h] // 00c46589
        fsub qword ptr [esi + 010h] // 00c4658c
        fld st(1) // 00c4658f
        fld qword ptr [esp + 01e8h] // 00c46591
        fmul st(1), st(0) // 00c46598
        fld st(2) // 00c4659a
        fmul qword ptr [esp + 01e0h] // 00c4659c
        fsubp st(2), st(0) // 00c465a3
        fld qword ptr [esp + 01d8h] // 00c465a5
        fld st(0) // 00c465ac
        fmulp st(4), st(0) // 00c465ae
        fld st(5) // 00c465b0
        fmulp st(2), st(0) // 00c465b2
        fxch st(3) // 00c465b4
        fsubrp st(1), st(0) // 00c465b6
        fxch st(4) // 00c465b8
        fmul qword ptr [esp + 01e0h] // 00c465ba
        fxch st(2) // 00c465c1
        fmulp st(3), st(0) // 00c465c3
        fxch st(1) // 00c465c5
        fsubrp st(2), st(0) // 00c465c7
        fld qword ptr [esi] // 00c465c9
        fsub qword ptr [edi + 070h] // 00c465cb
        fld qword ptr [esi + 8] // 00c465ce
        fsub qword ptr [edi + 078h] // 00c465d1
        fld qword ptr [esi + 010h] // 00c465d4
        fsub qword ptr [edi + 080h] // 00c465d7
        fxch st(1) // 00c465dd
        fmulp st(5), st(0) // 00c465df
        fxch st(1) // 00c465e1
        fmulp st(2), st(0) // 00c465e3
        fxch st(3) // 00c465e5
        faddp st(1), st(0) // 00c465e7
        fxch st(2) // 00c465e9
        fmulp st(1), st(0) // 00c465eb
        faddp st(1), st(0) // 00c465ed
        fxch st(1) // 00c465ef
        fcomi st(0), st(1) // 00c465f1
        fstp st(1) // 00c465f3
        jb l_00c46725 // 00c465f5
        fstp st(0) // 00c465fb
        lea ecx, [esp + 020h] // 00c465fd
        fld qword ptr [edi + 050h] // 00c46601
        lea edx, [esp + 038h] // 00c46604
        fsub qword ptr [esi] // 00c46608
        lea eax, [esp + 088h] // 00c4660a
        fstp qword ptr [esp + 020h] // 00c46611
        fld qword ptr [edi + 058h] // 00c46615
        fsub qword ptr [esi + 8] // 00c46618
        fstp qword ptr [esp + 028h] // 00c4661b
        fld qword ptr [edi + 060h] // 00c4661f
        fsub qword ptr [esi + 010h] // 00c46622
        fstp qword ptr [esp + 030h] // 00c46625
        fld qword ptr [esi] // 00c46629
        fsub qword ptr [edi + 070h] // 00c4662b
        fstp qword ptr [esp + 0388h] // 00c4662e
        fld qword ptr [esi + 8] // 00c46635
        fsub qword ptr [edi + 078h] // 00c46638
        fstp qword ptr [esp + 0390h] // 00c4663b
        fld qword ptr [esi + 010h] // 00c46642
        fsub qword ptr [edi + 080h] // 00c46645
        fstp qword ptr [esp + 0398h] // 00c4664b
        call cross_reference // 00c46652
        fld qword ptr [eax + 8] // 00c46657
        fmul qword ptr [esp + 0390h] // 00c4665a
        fld qword ptr [eax] // 00c46661
        fmul qword ptr [esp + 0388h] // 00c46663
        faddp st(1), st(0) // 00c4666a
        fld qword ptr [eax + 010h] // 00c4666c
        fmul qword ptr [esp + 0398h] // 00c4666f
        faddp st(1), st(0) // 00c46676
        fldz  // 00c46678
        fcomi st(0), st(1) // 00c4667a
        fstp st(1) // 00c4667c
        jb l_00c46725 // 00c4667e
        fstp st(0) // 00c46684
        lea ecx, [esp + 020h] // 00c46686
        fld qword ptr [esi] // 00c4668a
        lea edx, [esp + 038h] // 00c4668c
        fstp qword ptr [esi] // 00c46690
        lea eax, [esp + 0a0h] // 00c46692
        fld qword ptr [esi + 8] // 00c46699
        fstp qword ptr [esi + 8] // 00c4669c
        fld qword ptr [esi + 010h] // 00c4669f
        fstp qword ptr [esi + 010h] // 00c466a2
        fld qword ptr [edi + 050h] // 00c466a5
        fstp qword ptr [ebx] // 00c466a8
        fld qword ptr [edi + 058h] // 00c466aa
        fstp qword ptr [ebx + 8] // 00c466ad
        fld qword ptr [edi + 060h] // 00c466b0
        fstp qword ptr [ebx + 010h] // 00c466b3
        fld qword ptr [esi] // 00c466b6
        fsub qword ptr [ebx] // 00c466b8
        fstp qword ptr [esp + 058h] // 00c466ba
        fld qword ptr [esi + 8] // 00c466be
        fsub qword ptr [ebx + 8] // 00c466c1
        fstp qword ptr [esp + 060h] // 00c466c4
        fld qword ptr [esi + 010h] // 00c466c8
        fsub qword ptr [ebx + 010h] // 00c466cb
        fstp qword ptr [esp + 068h] // 00c466ce
        fld qword ptr [esi] // 00c466d2
        fsub qword ptr [edi + 070h] // 00c466d4
        fstp qword ptr [esp + 020h] // 00c466d7
        fld qword ptr [esi + 8] // 00c466db
        fsub qword ptr [edi + 078h] // 00c466de
        fstp qword ptr [esp + 028h] // 00c466e1
        fld qword ptr [esi + 010h] // 00c466e5
        fsub qword ptr [edi + 080h] // 00c466e8
        fstp qword ptr [esp + 030h] // 00c466ee
        fld qword ptr [ebx] // 00c466f2
        fsub qword ptr [esi] // 00c466f4
        fstp qword ptr [esp + 038h] // 00c466f6
        fld qword ptr [ebx + 8] // 00c466fa
        fsub qword ptr [esi + 8] // 00c466fd
        fstp qword ptr [esp + 040h] // 00c46700
        fld qword ptr [ebx + 010h] // 00c46704
        fsub qword ptr [esi + 010h] // 00c46707
        fstp qword ptr [esp + 048h] // 00c4670a
        call cross_reference // 00c4670e
        mov edx, eax // 00c46713
        lea ecx, [esp + 058h] // 00c46715
        lea eax, [esp + 0e8h] // 00c46719
        jmp l_00c46148 // 00c46720
    l_00c46725:
        fld qword ptr [esi] // 00c46725
        fsub qword ptr [ebx] // 00c46727
        fld qword ptr [esi + 8] // 00c46729
        fsub qword ptr [ebx + 8] // 00c4672c
        fld qword ptr [esi + 010h] // 00c4672f
        fsub qword ptr [ebx + 010h] // 00c46732
        fld qword ptr [edi + 050h] // 00c46735
        fsub qword ptr [ebx] // 00c46738
        fld qword ptr [edi + 058h] // 00c4673a
        fsub qword ptr [ebx + 8] // 00c4673d
        fld qword ptr [edi + 060h] // 00c46740
        fsub qword ptr [ebx + 010h] // 00c46743
        fst qword ptr [esp + 0458h] // 00c46746
        fld st(1) // 00c4674d
        fmul st(0), st(4) // 00c4674f
        fxch st(1) // 00c46751
        fmul st(0), st(5) // 00c46753
        fsubp st(1), st(0) // 00c46755
        fstp qword ptr [esp + 01a8h] // 00c46757
        fld st(4) // 00c4675e
        fmul qword ptr [esp + 0458h] // 00c46760
        fld st(2) // 00c46767
        fmulp st(4), st(0) // 00c46769
        fsubrp st(3), st(0) // 00c4676b
        fxch st(2) // 00c4676d
        fstp qword ptr [esp + 01b0h] // 00c4676f
        fmulp st(2), st(0) // 00c46776
        fmulp st(2), st(0) // 00c46778
        fsubrp st(1), st(0) // 00c4677a
        fstp qword ptr [esp + 01b8h] // 00c4677c
        fld qword ptr [edi + 050h] // 00c46783
        fsub qword ptr [ebx] // 00c46786
        fld qword ptr [edi + 058h] // 00c46788
        fsub qword ptr [ebx + 8] // 00c4678b
        fld qword ptr [edi + 060h] // 00c4678e
        fsub qword ptr [ebx + 010h] // 00c46791
        fld qword ptr [edi + 038h] // 00c46794
        fsub qword ptr [ebx] // 00c46797
        fld qword ptr [edi + 040h] // 00c46799
        fsub qword ptr [ebx + 8] // 00c4679c
        fld qword ptr [edi + 048h] // 00c4679f
        fsub qword ptr [ebx + 010h] // 00c467a2
        fst qword ptr [esp + 0488h] // 00c467a5
        fld st(1) // 00c467ac
        fmul st(0), st(4) // 00c467ae
        fxch st(1) // 00c467b0
        fmul st(0), st(5) // 00c467b2
        fsubp st(1), st(0) // 00c467b4
        fstp qword ptr [esp + 038h] // 00c467b6
        fld st(4) // 00c467ba
        fmul qword ptr [esp + 0488h] // 00c467bc
        fld st(2) // 00c467c3
        fmulp st(4), st(0) // 00c467c5
        fsubrp st(3), st(0) // 00c467c7
        fxch st(2) // 00c467c9
        fstp qword ptr [esp + 040h] // 00c467cb
        fmulp st(2), st(0) // 00c467cf
        fmulp st(2), st(0) // 00c467d1
        fsubrp st(1), st(0) // 00c467d3
        fstp qword ptr [esp + 048h] // 00c467d5
        fld qword ptr [edi + 050h] // 00c467d9
        fsub qword ptr [ebx] // 00c467dc
        fld qword ptr [edi + 058h] // 00c467de
        fsub qword ptr [ebx + 8] // 00c467e1
        fld qword ptr [edi + 060h] // 00c467e4
        fsub qword ptr [ebx + 010h] // 00c467e7
        fld qword ptr [ebx] // 00c467ea
        fsub qword ptr [edi + 070h] // 00c467ec
        fld qword ptr [ebx + 8] // 00c467ef
        fsub qword ptr [edi + 078h] // 00c467f2
        fld qword ptr [ebx + 010h] // 00c467f5
        fsub qword ptr [edi + 080h] // 00c467f8
        fxch st(2) // 00c467fe
        fmulp st(5), st(0) // 00c46800
        fmulp st(3), st(0) // 00c46802
        fxch st(3) // 00c46804
        faddp st(2), st(0) // 00c46806
        fmulp st(2), st(0) // 00c46808
        faddp st(1), st(0) // 00c4680a
        fxch st(1) // 00c4680c
        fcomi st(0), st(1) // 00c4680e
        fstp st(1) // 00c46810
        jb l_00c469fb // 00c46812
        fld qword ptr [ebx] // 00c46818
        fsub qword ptr [edi + 050h] // 00c4681a
        fld qword ptr [ebx + 8] // 00c4681d
        fsub qword ptr [edi + 058h] // 00c46820
        fld qword ptr [ebx + 010h] // 00c46823
        fsub qword ptr [edi + 060h] // 00c46826
        fld qword ptr [edi + 050h] // 00c46829
        fsub qword ptr [edi + 070h] // 00c4682c
        fld qword ptr [edi + 058h] // 00c4682f
        fsub qword ptr [edi + 078h] // 00c46832
        fld qword ptr [edi + 060h] // 00c46835
        fsub qword ptr [edi + 080h] // 00c46838
        fxch st(2) // 00c4683e
        fmulp st(5), st(0) // 00c46840
        fmulp st(3), st(0) // 00c46842
        fxch st(3) // 00c46844
        faddp st(2), st(0) // 00c46846
        fmulp st(2), st(0) // 00c46848
        faddp st(1), st(0) // 00c4684a
        fxch st(1) // 00c4684c
        fcomi st(0), st(1) // 00c4684e
        fstp st(1) // 00c46850
        jb l_00c469fb // 00c46852
        fld qword ptr [edi + 050h] // 00c46858
        fsub qword ptr [ebx] // 00c4685b
        fld qword ptr [edi + 058h] // 00c4685d
        fsub qword ptr [ebx + 8] // 00c46860
        fld qword ptr [edi + 060h] // 00c46863
        fsub qword ptr [ebx + 010h] // 00c46866
        fld st(1) // 00c46869
        fld qword ptr [esp + 01b8h] // 00c4686b
        fmul st(1), st(0) // 00c46872
        fld st(2) // 00c46874
        fmul qword ptr [esp + 01b0h] // 00c46876
        fsubp st(2), st(0) // 00c4687d
        fld qword ptr [esp + 01a8h] // 00c4687f
        fld st(0) // 00c46886
        fmulp st(4), st(0) // 00c46888
        fld st(5) // 00c4688a
        fmulp st(2), st(0) // 00c4688c
        fxch st(3) // 00c4688e
        fsubrp st(1), st(0) // 00c46890
        fxch st(4) // 00c46892
        fmul qword ptr [esp + 01b0h] // 00c46894
        fxch st(2) // 00c4689b
        fmulp st(3), st(0) // 00c4689d
        fxch st(1) // 00c4689f
        fsubrp st(2), st(0) // 00c468a1
        fld qword ptr [ebx] // 00c468a3
        fsub qword ptr [edi + 070h] // 00c468a5
        fld qword ptr [ebx + 8] // 00c468a8
        fsub qword ptr [edi + 078h] // 00c468ab
        fld qword ptr [ebx + 010h] // 00c468ae
        fsub qword ptr [edi + 080h] // 00c468b1
        fxch st(2) // 00c468b7
        fmulp st(3), st(0) // 00c468b9
        fmulp st(4), st(0) // 00c468bb
        fxch st(1) // 00c468bd
        faddp st(3), st(0) // 00c468bf
        fmulp st(1), st(0) // 00c468c1
        faddp st(1), st(0) // 00c468c3
        fxch st(1) // 00c468c5
        fcomi st(0), st(1) // 00c468c7
        fstp st(1) // 00c468c9
        jb l_00c469fb // 00c468cb
        fstp st(0) // 00c468d1
        lea ecx, [esp + 020h] // 00c468d3
        fld qword ptr [edi + 050h] // 00c468d7
        lea edx, [esp + 038h] // 00c468da
        fsub qword ptr [ebx] // 00c468de
        lea eax, [esp + 088h] // 00c468e0
        fstp qword ptr [esp + 020h] // 00c468e7
        fld qword ptr [edi + 058h] // 00c468eb
        fsub qword ptr [ebx + 8] // 00c468ee
        fstp qword ptr [esp + 028h] // 00c468f1
        fld qword ptr [edi + 060h] // 00c468f5
        fsub qword ptr [ebx + 010h] // 00c468f8
        fstp qword ptr [esp + 030h] // 00c468fb
        fld qword ptr [ebx] // 00c468ff
        fsub qword ptr [edi + 070h] // 00c46901
        fstp qword ptr [esp + 0220h] // 00c46904
        fld qword ptr [ebx + 8] // 00c4690b
        fsub qword ptr [edi + 078h] // 00c4690e
        fstp qword ptr [esp + 0228h] // 00c46911
        fld qword ptr [ebx + 010h] // 00c46918
        fsub qword ptr [edi + 080h] // 00c4691b
        fstp qword ptr [esp + 0230h] // 00c46921
        call cross_reference // 00c46928
        fld qword ptr [eax + 8] // 00c4692d
        fmul qword ptr [esp + 0228h] // 00c46930
        fld qword ptr [esp + 0220h] // 00c46937
        fmul qword ptr [eax] // 00c4693e
        faddp st(1), st(0) // 00c46940
        fld qword ptr [eax + 010h] // 00c46942
        fmul qword ptr [esp + 0230h] // 00c46945
        faddp st(1), st(0) // 00c4694c
        fldz  // 00c4694e
        fcomi st(0), st(1) // 00c46950
        fstp st(1) // 00c46952
        jb l_00c469fb // 00c46954
        fstp st(0) // 00c4695a
        lea ecx, [esp + 020h] // 00c4695c
        fld qword ptr [ebx] // 00c46960
        lea edx, [esp + 038h] // 00c46962
        fstp qword ptr [esi] // 00c46966
        lea eax, [esp + 0a0h] // 00c46968
        fld qword ptr [ebx + 8] // 00c4696f
        fstp qword ptr [esi + 8] // 00c46972
        fld qword ptr [ebx + 010h] // 00c46975
        fstp qword ptr [esi + 010h] // 00c46978
        fld qword ptr [edi + 050h] // 00c4697b
        fstp qword ptr [ebx] // 00c4697e
        fld qword ptr [edi + 058h] // 00c46980
        fstp qword ptr [ebx + 8] // 00c46983
        fld qword ptr [edi + 060h] // 00c46986
        fstp qword ptr [ebx + 010h] // 00c46989
        fld qword ptr [esi] // 00c4698c
        fsub qword ptr [ebx] // 00c4698e
        fstp qword ptr [esp + 058h] // 00c46990
        fld qword ptr [esi + 8] // 00c46994
        fsub qword ptr [ebx + 8] // 00c46997
        fstp qword ptr [esp + 060h] // 00c4699a
        fld qword ptr [esi + 010h] // 00c4699e
        fsub qword ptr [ebx + 010h] // 00c469a1
        fstp qword ptr [esp + 068h] // 00c469a4
        fld qword ptr [esi] // 00c469a8
        fsub qword ptr [edi + 070h] // 00c469aa
        fstp qword ptr [esp + 020h] // 00c469ad
        fld qword ptr [esi + 8] // 00c469b1
        fsub qword ptr [edi + 078h] // 00c469b4
        fstp qword ptr [esp + 028h] // 00c469b7
        fld qword ptr [esi + 010h] // 00c469bb
        fsub qword ptr [edi + 080h] // 00c469be
        fstp qword ptr [esp + 030h] // 00c469c4
        fld qword ptr [ebx] // 00c469c8
        fsub qword ptr [esi] // 00c469ca
        fstp qword ptr [esp + 038h] // 00c469cc
        fld qword ptr [ebx + 8] // 00c469d0
        fsub qword ptr [esi + 8] // 00c469d3
        fstp qword ptr [esp + 040h] // 00c469d6
        fld qword ptr [ebx + 010h] // 00c469da
        fsub qword ptr [esi + 010h] // 00c469dd
        fstp qword ptr [esp + 048h] // 00c469e0
        call cross_reference // 00c469e4
        mov edx, eax // 00c469e9
        lea ecx, [esp + 058h] // 00c469eb
        lea eax, [esp + 0e8h] // 00c469ef
        jmp l_00c46148 // 00c469f6
    l_00c469fb:
        fld qword ptr [esi] // 00c469fb
        fsub qword ptr [edi + 038h] // 00c469fd
        fld qword ptr [esi + 8] // 00c46a00
        fsub qword ptr [edi + 040h] // 00c46a03
        fld qword ptr [esi + 010h] // 00c46a06
        fsub qword ptr [edi + 048h] // 00c46a09
        fld qword ptr [edi + 050h] // 00c46a0c
        fsub qword ptr [edi + 038h] // 00c46a0f
        fld qword ptr [edi + 058h] // 00c46a12
        fsub qword ptr [edi + 040h] // 00c46a15
        fld qword ptr [edi + 060h] // 00c46a18
        fsub qword ptr [edi + 048h] // 00c46a1b
        fst qword ptr [esp + 04b8h] // 00c46a1e
        fld st(1) // 00c46a25
        fmul st(0), st(4) // 00c46a27
        fxch st(1) // 00c46a29
        fmul st(0), st(5) // 00c46a2b
        fsubp st(1), st(0) // 00c46a2d
        fstp qword ptr [esp + 01f0h] // 00c46a2f
        fld qword ptr [esp + 04b8h] // 00c46a36
        fmul st(0), st(5) // 00c46a3d
        fxch st(3) // 00c46a3f
        fmul st(0), st(2) // 00c46a41
        fsubp st(3), st(0) // 00c46a43
        fxch st(2) // 00c46a45
        fstp qword ptr [esp + 01f8h] // 00c46a47
        fmulp st(2), st(0) // 00c46a4e
        fmulp st(2), st(0) // 00c46a50
        fsubrp st(1), st(0) // 00c46a52
        fstp qword ptr [esp + 0200h] // 00c46a54
        fld qword ptr [edi + 050h] // 00c46a5b
        fsub qword ptr [edi + 038h] // 00c46a5e
        fld qword ptr [edi + 058h] // 00c46a61
        fsub qword ptr [edi + 040h] // 00c46a64
        fld qword ptr [edi + 060h] // 00c46a67
        fsub qword ptr [edi + 048h] // 00c46a6a
        fld qword ptr [ebx] // 00c46a6d
        fsub qword ptr [edi + 038h] // 00c46a6f
        fld qword ptr [ebx + 8] // 00c46a72
        fsub qword ptr [edi + 040h] // 00c46a75
        fld qword ptr [ebx + 010h] // 00c46a78
        fsub qword ptr [edi + 048h] // 00c46a7b
        fst qword ptr [esp + 04e8h] // 00c46a7e
        fld st(1) // 00c46a85
        fmul st(0), st(4) // 00c46a87
        fxch st(1) // 00c46a89
        fmul st(0), st(5) // 00c46a8b
        fsubp st(1), st(0) // 00c46a8d
        fstp qword ptr [esp + 038h] // 00c46a8f
        fld qword ptr [esp + 04e8h] // 00c46a93
        fmul st(0), st(5) // 00c46a9a
        fxch st(3) // 00c46a9c
        fmul st(0), st(2) // 00c46a9e
        fsubp st(3), st(0) // 00c46aa0
        fxch st(2) // 00c46aa2
        fstp qword ptr [esp + 040h] // 00c46aa4
        fmulp st(2), st(0) // 00c46aa8
        fmulp st(2), st(0) // 00c46aaa
        fsubrp st(1), st(0) // 00c46aac
        fstp qword ptr [esp + 048h] // 00c46aae
        fld qword ptr [edi + 050h] // 00c46ab2
        fsub qword ptr [edi + 038h] // 00c46ab5
        fld qword ptr [edi + 058h] // 00c46ab8
        fsub qword ptr [edi + 040h] // 00c46abb
        fld qword ptr [edi + 060h] // 00c46abe
        fsub qword ptr [edi + 048h] // 00c46ac1
        fld qword ptr [edi + 038h] // 00c46ac4
        fsub qword ptr [edi + 070h] // 00c46ac7
        fld qword ptr [edi + 040h] // 00c46aca
        fsub qword ptr [edi + 078h] // 00c46acd
        fld qword ptr [edi + 048h] // 00c46ad0
        fsub qword ptr [edi + 080h] // 00c46ad3
        fxch st(2) // 00c46ad9
        fmulp st(5), st(0) // 00c46adb
        fmulp st(3), st(0) // 00c46add
        fxch st(3) // 00c46adf
        faddp st(2), st(0) // 00c46ae1
        fmulp st(2), st(0) // 00c46ae3
        faddp st(1), st(0) // 00c46ae5
        fxch st(1) // 00c46ae7
        fcomi st(0), st(1) // 00c46ae9
        fstp st(1) // 00c46aeb
        jb l_00c46cfd // 00c46aed
        fld qword ptr [edi + 038h] // 00c46af3
        fsub qword ptr [edi + 050h] // 00c46af6
        fld qword ptr [edi + 040h] // 00c46af9
        fsub qword ptr [edi + 058h] // 00c46afc
        fld qword ptr [edi + 048h] // 00c46aff
        fsub qword ptr [edi + 060h] // 00c46b02
        fld qword ptr [edi + 050h] // 00c46b05
        fsub qword ptr [edi + 070h] // 00c46b08
        fld qword ptr [edi + 058h] // 00c46b0b
        fsub qword ptr [edi + 078h] // 00c46b0e
        fld qword ptr [edi + 060h] // 00c46b11
        fsub qword ptr [edi + 080h] // 00c46b14
        fxch st(2) // 00c46b1a
        fmulp st(5), st(0) // 00c46b1c
        fmulp st(3), st(0) // 00c46b1e
        fxch st(3) // 00c46b20
        faddp st(2), st(0) // 00c46b22
        fmulp st(2), st(0) // 00c46b24
        faddp st(1), st(0) // 00c46b26
        fxch st(1) // 00c46b28
        fcomi st(0), st(1) // 00c46b2a
        fstp st(1) // 00c46b2c
        jb l_00c46cfd // 00c46b2e
        fld qword ptr [edi + 050h] // 00c46b34
        fsub qword ptr [edi + 038h] // 00c46b37
        fld qword ptr [edi + 058h] // 00c46b3a
        fsub qword ptr [edi + 040h] // 00c46b3d
        fld qword ptr [edi + 060h] // 00c46b40
        fsub qword ptr [edi + 048h] // 00c46b43
        fld st(1) // 00c46b46
        fld qword ptr [esp + 0200h] // 00c46b48
        fmul st(1), st(0) // 00c46b4f
        fld st(2) // 00c46b51
        fld qword ptr [esp + 01f8h] // 00c46b53
        fmul st(1), st(0) // 00c46b5a
        fxch st(3) // 00c46b5c
        fsubrp st(1), st(0) // 00c46b5e
        fld qword ptr [esp + 01f0h] // 00c46b60
        fmul st(4), st(0) // 00c46b67
        fxch st(2) // 00c46b69
        fmul st(0), st(6) // 00c46b6b
        fsubp st(4), st(0) // 00c46b6d
        fxch st(2) // 00c46b6f
        fmulp st(5), st(0) // 00c46b71
        fmulp st(3), st(0) // 00c46b73
        fxch st(3) // 00c46b75
        fsubrp st(2), st(0) // 00c46b77
        fld qword ptr [edi + 038h] // 00c46b79
        fsub qword ptr [edi + 070h] // 00c46b7c
        fld qword ptr [edi + 040h] // 00c46b7f
        fsub qword ptr [edi + 078h] // 00c46b82
        fld qword ptr [edi + 048h] // 00c46b85
        fsub qword ptr [edi + 080h] // 00c46b88
        fxch st(2) // 00c46b8e
        fmulp st(5), st(0) // 00c46b90
        fmulp st(2), st(0) // 00c46b92
        fxch st(3) // 00c46b94
        faddp st(1), st(0) // 00c46b96
        fxch st(2) // 00c46b98
        fmulp st(1), st(0) // 00c46b9a
        faddp st(1), st(0) // 00c46b9c
        fxch st(1) // 00c46b9e
        fcomi st(0), st(1) // 00c46ba0
        fstp st(1) // 00c46ba2
        jb l_00c46cfd // 00c46ba4
        fstp st(0) // 00c46baa
        lea ecx, [esp + 020h] // 00c46bac
        fld qword ptr [edi + 050h] // 00c46bb0
        lea edx, [esp + 038h] // 00c46bb3
        fsub qword ptr [edi + 038h] // 00c46bb7
        lea eax, [esp + 088h] // 00c46bba
        fstp qword ptr [esp + 020h] // 00c46bc1
        fld qword ptr [edi + 058h] // 00c46bc5
        fsub qword ptr [edi + 040h] // 00c46bc8
        fstp qword ptr [esp + 028h] // 00c46bcb
        fld qword ptr [edi + 060h] // 00c46bcf
        fsub qword ptr [edi + 048h] // 00c46bd2
        fstp qword ptr [esp + 030h] // 00c46bd5
        fld qword ptr [edi + 038h] // 00c46bd9
        fsub qword ptr [edi + 070h] // 00c46bdc
        fstp qword ptr [esp + 02e0h] // 00c46bdf
        fld qword ptr [edi + 040h] // 00c46be6
        fsub qword ptr [edi + 078h] // 00c46be9
        fstp qword ptr [esp + 02e8h] // 00c46bec
        fld qword ptr [edi + 048h] // 00c46bf3
        fsub qword ptr [edi + 080h] // 00c46bf6
        fstp qword ptr [esp + 02f0h] // 00c46bfc
        call cross_reference // 00c46c03
        fld qword ptr [eax + 8] // 00c46c08
        fmul qword ptr [esp + 02e8h] // 00c46c0b
        fld qword ptr [eax] // 00c46c12
        fmul qword ptr [esp + 02e0h] // 00c46c14
        faddp st(1), st(0) // 00c46c1b
        fld qword ptr [eax + 010h] // 00c46c1d
        fmul qword ptr [esp + 02f0h] // 00c46c20
        faddp st(1), st(0) // 00c46c27
        fldz  // 00c46c29
        fcomi st(0), st(1) // 00c46c2b
        fstp st(1) // 00c46c2d
        jb l_00c46cfd // 00c46c2f
        fstp st(0) // 00c46c35
        lea ecx, [esp + 020h] // 00c46c37
        fld qword ptr [edi + 038h] // 00c46c3b
        lea edx, [esp + 038h] // 00c46c3e
        fstp qword ptr [esi] // 00c46c42
        lea eax, [esp + 0a0h] // 00c46c44
        fld qword ptr [edi + 040h] // 00c46c4b
        fstp qword ptr [esi + 8] // 00c46c4e
        fld qword ptr [edi + 048h] // 00c46c51
        fstp qword ptr [esi + 010h] // 00c46c54
        fld qword ptr [edi + 050h] // 00c46c57
        fstp qword ptr [ebx] // 00c46c5a
        fld qword ptr [edi + 058h] // 00c46c5c
        fstp qword ptr [ebx + 8] // 00c46c5f
        fld qword ptr [edi + 060h] // 00c46c62
        fstp qword ptr [ebx + 010h] // 00c46c65
        fld qword ptr [esi] // 00c46c68
        fsub qword ptr [ebx] // 00c46c6a
        fstp qword ptr [esp + 058h] // 00c46c6c
        fld qword ptr [esi + 8] // 00c46c70
        fsub qword ptr [ebx + 8] // 00c46c73
        fstp qword ptr [esp + 060h] // 00c46c76
        fld qword ptr [esi + 010h] // 00c46c7a
        fsub qword ptr [ebx + 010h] // 00c46c7d
        fstp qword ptr [esp + 068h] // 00c46c80
        fld qword ptr [esi] // 00c46c84
        fsub qword ptr [edi + 070h] // 00c46c86
        fstp qword ptr [esp + 020h] // 00c46c89
        fld qword ptr [esi + 8] // 00c46c8d
        fsub qword ptr [edi + 078h] // 00c46c90
        fstp qword ptr [esp + 028h] // 00c46c93
        fld qword ptr [esi + 010h] // 00c46c97
        fsub qword ptr [edi + 080h] // 00c46c9a
        fstp qword ptr [esp + 030h] // 00c46ca0
        fld qword ptr [ebx] // 00c46ca4
        fsub qword ptr [esi] // 00c46ca6
        fstp qword ptr [esp + 038h] // 00c46ca8
        fld qword ptr [ebx + 8] // 00c46cac
        fsub qword ptr [esi + 8] // 00c46caf
        fstp qword ptr [esp + 040h] // 00c46cb2
        fld qword ptr [ebx + 010h] // 00c46cb6
        fsub qword ptr [esi + 010h] // 00c46cb9
        fstp qword ptr [esp + 048h] // 00c46cbc
        call cross_reference // 00c46cc0
        mov edx, eax // 00c46cc5
        lea ecx, [esp + 058h] // 00c46cc7
        lea eax, [esp + 0e8h] // 00c46ccb
        call cross_reference // 00c46cd2
        fld qword ptr [eax] // 00c46cd7
        fstp qword ptr [edi + 088h] // 00c46cd9
        fld qword ptr [eax + 8] // 00c46cdf
        fstp qword ptr [edi + 090h] // 00c46ce2
        fld qword ptr [eax + 010h] // 00c46ce8
        fstp qword ptr [edi + 098h] // 00c46ceb
        mov dword ptr [edi + 068h], 2 // 00c46cf1
        jmp l_00c44a87 // 00c46cf8
    l_00c46cfd:
        fld qword ptr [edi + 038h] // 00c46cfd
        fsub qword ptr [esi] // 00c46d00
        fld qword ptr [edi + 040h] // 00c46d02
        fsub qword ptr [esi + 8] // 00c46d05
        fld qword ptr [edi + 048h] // 00c46d08
        fsub qword ptr [esi + 010h] // 00c46d0b
        fld qword ptr [ebx] // 00c46d0e
        fsub qword ptr [esi] // 00c46d10
        fld qword ptr [ebx + 8] // 00c46d12
        fsub qword ptr [esi + 8] // 00c46d15
        fld qword ptr [ebx + 010h] // 00c46d18
        fsub qword ptr [esi + 010h] // 00c46d1b
        fst qword ptr [esp + 0518h] // 00c46d1e
        fld st(1) // 00c46d25
        fmul st(0), st(4) // 00c46d27
        fxch st(1) // 00c46d29
        fmul st(0), st(5) // 00c46d2b
        fsubp st(1), st(0) // 00c46d2d
        fst qword ptr [esp + 020h] // 00c46d2f
        fld qword ptr [esp + 0518h] // 00c46d33
        fmul st(0), st(6) // 00c46d3a
        fxch st(4) // 00c46d3c
        fmul st(0), st(3) // 00c46d3e
        fsubp st(4), st(0) // 00c46d40
        fxch st(3) // 00c46d42
        fst qword ptr [esp + 028h] // 00c46d44
        fxch st(4) // 00c46d48
        fmulp st(2), st(0) // 00c46d4a
        fmulp st(4), st(0) // 00c46d4c
        fsubrp st(3), st(0) // 00c46d4e
        fxch st(2) // 00c46d50
        fst qword ptr [esp + 030h] // 00c46d52
        fld qword ptr [esi] // 00c46d56
        fsub qword ptr [edi + 070h] // 00c46d58
        fld qword ptr [esi + 8] // 00c46d5b
        fsub qword ptr [edi + 078h] // 00c46d5e
        fld qword ptr [esi + 010h] // 00c46d61
        fsub qword ptr [edi + 080h] // 00c46d64
        fld qword ptr [edi + 050h] // 00c46d6a
        fsub qword ptr [esi] // 00c46d6d
        fstp qword ptr [esp + 03e8h] // 00c46d6f
        fld qword ptr [edi + 058h] // 00c46d76
        fsub qword ptr [esi + 8] // 00c46d79
        fstp qword ptr [esp + 03f0h] // 00c46d7c
        fld qword ptr [edi + 060h] // 00c46d83
        fsub qword ptr [esi + 010h] // 00c46d86
        fxch st(2) // 00c46d89
        fmul st(0), st(5) // 00c46d8b
        fxch st(3) // 00c46d8d
        fmul st(0), st(6) // 00c46d8f
        faddp st(3), st(0) // 00c46d91
        fmul st(0), st(3) // 00c46d93
        faddp st(2), st(0) // 00c46d95
        fld qword ptr [esp + 03f0h] // 00c46d97
        fmul st(0), st(4) // 00c46d9e
        fld qword ptr [esp + 03e8h] // 00c46da0
        fmul st(0), st(6) // 00c46da7
        faddp st(1), st(0) // 00c46da9
        fxch st(1) // 00c46dab
        fmul st(0), st(3) // 00c46dad
        faddp st(1), st(0) // 00c46daf
        fmulp st(1), st(0) // 00c46db1
        fxch st(4) // 00c46db3
        fcomi st(0), st(4) // 00c46db5
        fstp st(4) // 00c46db7
        ja l_00c47059 // 00c46db9
        fld qword ptr [ebx] // 00c46dbf
        fsub qword ptr [esi] // 00c46dc1
        fld qword ptr [ebx + 8] // 00c46dc3
        fsub qword ptr [esi + 8] // 00c46dc6
        fld qword ptr [ebx + 010h] // 00c46dc9
        fsub qword ptr [esi + 010h] // 00c46dcc
        fst qword ptr [esp + 0548h] // 00c46dcf
        fld st(1) // 00c46dd6
        fmul st(0), st(4) // 00c46dd8
        fxch st(1) // 00c46dda
        fmul st(0), st(5) // 00c46ddc
        fsubp st(1), st(0) // 00c46dde
        fld qword ptr [esp + 0548h] // 00c46de0
        fmul st(0), st(6) // 00c46de7
        fxch st(4) // 00c46de9
        fmul st(0), st(3) // 00c46deb
        fsubp st(4), st(0) // 00c46ded
        fxch st(4) // 00c46def
        fmulp st(2), st(0) // 00c46df1
        fmulp st(4), st(0) // 00c46df3
        fsubrp st(3), st(0) // 00c46df5
        fld qword ptr [esi] // 00c46df7
        fsub qword ptr [edi + 070h] // 00c46df9
        fld qword ptr [esi + 8] // 00c46dfc
        fsub qword ptr [edi + 078h] // 00c46dff
        fld qword ptr [esi + 010h] // 00c46e02
        fsub qword ptr [edi + 080h] // 00c46e05
        fxch st(1) // 00c46e0b
        fmulp st(3), st(0) // 00c46e0d
        fxch st(1) // 00c46e0f
        fmulp st(3), st(0) // 00c46e11
        fxch st(1) // 00c46e13
        faddp st(2), st(0) // 00c46e15
        fmulp st(2), st(0) // 00c46e17
        faddp st(1), st(0) // 00c46e19
        fxch st(1) // 00c46e1b
        fcomi st(0), st(1) // 00c46e1d
        fstp st(1) // 00c46e1f
        ja l_00c4705f // 00c46e21
        fstp st(0) // 00c46e27
        lea ecx, [esp + 020h] // 00c46e29
        fld qword ptr [esi] // 00c46e2d
        lea edx, [esp + 038h] // 00c46e2f
        fsub qword ptr [edi + 038h] // 00c46e33
        lea eax, [esp + 088h] // 00c46e36
        fstp qword ptr [esp + 038h] // 00c46e3d
        fld qword ptr [esi + 8] // 00c46e41
        fsub qword ptr [edi + 040h] // 00c46e44
        fstp qword ptr [esp + 040h] // 00c46e47
        fld qword ptr [esi + 010h] // 00c46e4b
        fsub qword ptr [edi + 048h] // 00c46e4e
        fstp qword ptr [esp + 048h] // 00c46e51
        fld qword ptr [esi] // 00c46e55
        fsub qword ptr [edi + 070h] // 00c46e57
        fstp qword ptr [esp + 0250h] // 00c46e5a
        fld qword ptr [esi + 8] // 00c46e61
        fsub qword ptr [edi + 078h] // 00c46e64
        fstp qword ptr [esp + 0258h] // 00c46e67
        fld qword ptr [esi + 010h] // 00c46e6e
        fsub qword ptr [edi + 080h] // 00c46e71
        fstp qword ptr [esp + 0260h] // 00c46e77
        call cross_reference // 00c46e7e
        fld qword ptr [eax + 8] // 00c46e83
        fmul qword ptr [esp + 0258h] // 00c46e86
        fld qword ptr [eax] // 00c46e8d
        fmul qword ptr [esp + 0250h] // 00c46e8f
        faddp st(1), st(0) // 00c46e96
        fld qword ptr [eax + 010h] // 00c46e98
        fmul qword ptr [esp + 0260h] // 00c46e9b
        faddp st(1), st(0) // 00c46ea2
        fldz  // 00c46ea4
        fcomi st(0), st(1) // 00c46ea6
        fstp st(1) // 00c46ea8
        ja l_00c4705f // 00c46eaa
        fstp st(0) // 00c46eb0
        lea ecx, [esp + 020h] // 00c46eb2
        fld qword ptr [esi] // 00c46eb6
        lea edx, [esp + 038h] // 00c46eb8
        fsub qword ptr [ebx] // 00c46ebc
        lea eax, [esp + 058h] // 00c46ebe
        fstp qword ptr [esp + 020h] // 00c46ec2
        fld qword ptr [esi + 8] // 00c46ec6
        fsub qword ptr [ebx + 8] // 00c46ec9
        fstp qword ptr [esp + 028h] // 00c46ecc
        fld qword ptr [esi + 010h] // 00c46ed0
        fsub qword ptr [ebx + 010h] // 00c46ed3
        fstp qword ptr [esp + 030h] // 00c46ed6
        fld qword ptr [edi + 038h] // 00c46eda
        fsub qword ptr [ebx] // 00c46edd
        fstp qword ptr [esp + 038h] // 00c46edf
        fld qword ptr [edi + 040h] // 00c46ee3
        fsub qword ptr [ebx + 8] // 00c46ee6
        fstp qword ptr [esp + 040h] // 00c46ee9
        fld qword ptr [edi + 048h] // 00c46eed
        fsub qword ptr [ebx + 010h] // 00c46ef0
        fstp qword ptr [esp + 048h] // 00c46ef3
        call cross_reference // 00c46ef7
        fld qword ptr [edi + 038h] // 00c46efc
        lea ecx, [esp + 058h] // 00c46eff
        fsub qword ptr [ebx] // 00c46f03
        lea edx, [esp + 020h] // 00c46f05
        lea eax, [esp + 0a0h] // 00c46f09
        fstp qword ptr [esp + 020h] // 00c46f10
        fld qword ptr [edi + 040h] // 00c46f14
        fsub qword ptr [ebx + 8] // 00c46f17
        fstp qword ptr [esp + 028h] // 00c46f1a
        fld qword ptr [edi + 048h] // 00c46f1e
        fsub qword ptr [ebx + 010h] // 00c46f21
        fstp qword ptr [esp + 030h] // 00c46f24
        fld qword ptr [ebx] // 00c46f28
        fsub qword ptr [edi + 070h] // 00c46f2a
        fstp qword ptr [esp + 0340h] // 00c46f2d
        fld qword ptr [ebx + 8] // 00c46f34
        fsub qword ptr [edi + 078h] // 00c46f37
        fstp qword ptr [esp + 0348h] // 00c46f3a
        fld qword ptr [ebx + 010h] // 00c46f41
        fsub qword ptr [edi + 080h] // 00c46f44
        fstp qword ptr [esp + 0350h] // 00c46f4a
        call cross_reference // 00c46f51
        fld qword ptr [eax + 8] // 00c46f56
        fmul qword ptr [esp + 0348h] // 00c46f59
        fld qword ptr [eax] // 00c46f60
        fmul qword ptr [esp + 0340h] // 00c46f62
        faddp st(1), st(0) // 00c46f69
        fld qword ptr [eax + 010h] // 00c46f6b
        fmul qword ptr [esp + 0350h] // 00c46f6e
        faddp st(1), st(0) // 00c46f75
        fldz  // 00c46f77
        fcomi st(0), st(1) // 00c46f79
        fstp st(1) // 00c46f7b
        ja l_00c4705f // 00c46f7d
        fstp st(0) // 00c46f83
        lea ecx, [esp + 020h] // 00c46f85
        fld qword ptr [esi] // 00c46f89
        lea edx, [esp + 038h] // 00c46f8b
        fstp qword ptr [esi] // 00c46f8f
        lea eax, [esp + 0e8h] // 00c46f91
        fld qword ptr [esi + 8] // 00c46f98
        fstp qword ptr [esi + 8] // 00c46f9b
        fld qword ptr [esi + 010h] // 00c46f9e
        fstp qword ptr [esi + 010h] // 00c46fa1
        fld qword ptr [ebx] // 00c46fa4
        fstp qword ptr [ebx] // 00c46fa6
        fld qword ptr [ebx + 8] // 00c46fa8
        fstp qword ptr [ebx + 8] // 00c46fab
        fld qword ptr [ebx + 010h] // 00c46fae
        fstp qword ptr [ebx + 010h] // 00c46fb1
        fld qword ptr [edi + 038h] // 00c46fb4
        fstp qword ptr [edi + 038h] // 00c46fb7
        fld qword ptr [edi + 040h] // 00c46fba
        fstp qword ptr [edi + 040h] // 00c46fbd
        fld qword ptr [edi + 048h] // 00c46fc0
        fstp qword ptr [edi + 048h] // 00c46fc3
        fld qword ptr [ebx] // 00c46fc6
        fsub qword ptr [esi] // 00c46fc8
        fstp qword ptr [esp + 020h] // 00c46fca
        fld qword ptr [ebx + 8] // 00c46fce
        fsub qword ptr [esi + 8] // 00c46fd1
        fstp qword ptr [esp + 028h] // 00c46fd4
        fld qword ptr [ebx + 010h] // 00c46fd8
        fsub qword ptr [esi + 010h] // 00c46fdb
        fstp qword ptr [esp + 030h] // 00c46fde
        fld qword ptr [edi + 038h] // 00c46fe2
        fsub qword ptr [esi] // 00c46fe5
        fstp qword ptr [esp + 038h] // 00c46fe7
        fld qword ptr [edi + 040h] // 00c46feb
        fsub qword ptr [esi + 8] // 00c46fee
        fstp qword ptr [esp + 040h] // 00c46ff1
        fld qword ptr [edi + 048h] // 00c46ff5
        fsub qword ptr [esi + 010h] // 00c46ff8
        fstp qword ptr [esp + 048h] // 00c46ffb
        call cross_reference // 00c46fff
        fld qword ptr [eax] // 00c47004
        lea ecx, [edi + 088h] // 00c47006
        fstp qword ptr [ecx] // 00c4700c
        sub esp, 8 // 00c4700e
        fld qword ptr [eax + 8] // 00c47011
        fstp qword ptr [ecx + 8] // 00c47014
        fld qword ptr [eax + 010h] // 00c47017
        mov eax, ecx // 00c4701a
        fstp qword ptr [ecx + 010h] // 00c4701c
        fld qword ptr [edi + 070h] // 00c4701f
        fsub qword ptr [esi] // 00c47022
        fld qword ptr [edi + 078h] // 00c47024
        fsub qword ptr [esi + 8] // 00c47027
        fld qword ptr [edi + 080h] // 00c4702a
        fsub qword ptr [esi + 010h] // 00c47030
        fld qword ptr [ecx + 8] // 00c47033
        fmulp st(2), st(0) // 00c47036
        fxch st(2) // 00c47038
        fmul qword ptr [ecx] // 00c4703a
        faddp st(1), st(0) // 00c4703c
        fld qword ptr [ecx + 010h] // 00c4703e
        fmulp st(2), st(0) // 00c47041
        faddp st(1), st(0) // 00c47043
        fstp qword ptr [esp] // 00c47045
        call scale_reference // 00c47048
        mov dword ptr [edi + 068h], 3 // 00c4704d
        jmp l_00c44a87 // 00c47054
    l_00c47059:
        fstp st(2) // 00c47059
        fstp st(0) // 00c4705b
        fstp st(0) // 00c4705d
    l_00c4705f:
        fld qword ptr [edi + 050h] // 00c4705f
        fsub qword ptr [esi] // 00c47062
        fld qword ptr [edi + 058h] // 00c47064
        fsub qword ptr [esi + 8] // 00c47067
        fld qword ptr [edi + 060h] // 00c4706a
        fsub qword ptr [esi + 010h] // 00c4706d
        fld qword ptr [ebx] // 00c47070
        fsub qword ptr [esi] // 00c47072
        fld qword ptr [ebx + 8] // 00c47074
        fsub qword ptr [esi + 8] // 00c47077
        fst qword ptr [esp + 0570h] // 00c4707a
        fld qword ptr [ebx + 010h] // 00c47081
        fsub qword ptr [esi + 010h] // 00c47084
        fxch st(1) // 00c47087
        fmul st(0), st(3) // 00c47089
        fld st(1) // 00c4708b
        fmul st(0), st(5) // 00c4708d
        fsubp st(1), st(0) // 00c4708f
        fst qword ptr [esp + 020h] // 00c47091
        fld st(5) // 00c47095
        fmulp st(2), st(0) // 00c47097
        fld st(2) // 00c47099
        fmulp st(4), st(0) // 00c4709b
        fxch st(1) // 00c4709d
        fsubrp st(3), st(0) // 00c4709f
        fxch st(2) // 00c470a1
        fst qword ptr [esp + 028h] // 00c470a3
        fxch st(1) // 00c470a7
        fmulp st(3), st(0) // 00c470a9
        fxch st(3) // 00c470ab
        fmul qword ptr [esp + 0570h] // 00c470ad
        fsubp st(2), st(0) // 00c470b4
        fxch st(1) // 00c470b6
        fst qword ptr [esp + 030h] // 00c470b8
        fld qword ptr [esi] // 00c470bc
        fsub qword ptr [edi + 070h] // 00c470be
        fld qword ptr [esi + 8] // 00c470c1
        fsub qword ptr [edi + 078h] // 00c470c4
        fld qword ptr [esi + 010h] // 00c470c7
        fsub qword ptr [edi + 080h] // 00c470ca
        fld qword ptr [edi + 038h] // 00c470d0
        fsub qword ptr [esi] // 00c470d3
        fstp qword ptr [esp + 03b8h] // 00c470d5
        fld qword ptr [edi + 040h] // 00c470dc
        fsub qword ptr [esi + 8] // 00c470df
        fstp qword ptr [esp + 03c0h] // 00c470e2
        fld qword ptr [edi + 048h] // 00c470e9
        fsub qword ptr [esi + 010h] // 00c470ec
        fxch st(3) // 00c470ef
        fmul st(0), st(5) // 00c470f1
        fxch st(2) // 00c470f3
        fmul st(0), st(6) // 00c470f5
        faddp st(2), st(0) // 00c470f7
        fmul st(0), st(3) // 00c470f9
        faddp st(1), st(0) // 00c470fb
        fld qword ptr [esp + 03b8h] // 00c470fd
        fmul st(0), st(4) // 00c47104
        fld qword ptr [esp + 03c0h] // 00c47106
        fmul st(0), st(6) // 00c4710d
        faddp st(1), st(0) // 00c4710f
        fxch st(2) // 00c47111
        fmul st(0), st(3) // 00c47113
        faddp st(2), st(0) // 00c47115
        fmulp st(1), st(0) // 00c47117
        fxch st(4) // 00c47119
        fcomi st(0), st(4) // 00c4711b
        fstp st(4) // 00c4711d
        ja l_00c473b5 // 00c4711f
        fld qword ptr [ebx] // 00c47125
        fsub qword ptr [esi] // 00c47127
        fld qword ptr [ebx + 8] // 00c47129
        fsub qword ptr [esi + 8] // 00c4712c
        fld qword ptr [ebx + 010h] // 00c4712f
        fsub qword ptr [esi + 010h] // 00c47132
        fst qword ptr [esp + 05a8h] // 00c47135
        fld st(1) // 00c4713c
        fmul st(0), st(4) // 00c4713e
        fxch st(1) // 00c47140
        fmul st(0), st(6) // 00c47142
        fsubp st(1), st(0) // 00c47144
        fstp qword ptr [esp + 05c8h] // 00c47146
        fld st(3) // 00c4714d
        fmul qword ptr [esp + 05a8h] // 00c4714f
        fld st(2) // 00c47156
        fmulp st(4), st(0) // 00c47158
        fsubrp st(3), st(0) // 00c4715a
        fxch st(1) // 00c4715c
        fmulp st(4), st(0) // 00c4715e
        fmulp st(2), st(0) // 00c47160
        fxch st(2) // 00c47162
        fsubrp st(1), st(0) // 00c47164
        fld qword ptr [esi] // 00c47166
        fsub qword ptr [edi + 070h] // 00c47168
        fld qword ptr [esi + 8] // 00c4716b
        fsub qword ptr [edi + 078h] // 00c4716e
        fld qword ptr [esi + 010h] // 00c47171
        fsub qword ptr [edi + 080h] // 00c47174
        fxch st(2) // 00c4717a
        fmul qword ptr [esp + 05c8h] // 00c4717c
        fxch st(1) // 00c47183
        fmulp st(4), st(0) // 00c47185
        faddp st(3), st(0) // 00c47187
        fmulp st(1), st(0) // 00c47189
        faddp st(1), st(0) // 00c4718b
        fxch st(1) // 00c4718d
        fcomi st(0), st(1) // 00c4718f
        fstp st(1) // 00c47191
        ja l_00c473bb // 00c47193
        fstp st(0) // 00c47199
        lea ecx, [esp + 020h] // 00c4719b
        fld qword ptr [esi] // 00c4719f
        lea edx, [esp + 038h] // 00c471a1
        fsub qword ptr [edi + 050h] // 00c471a5
        lea eax, [esp + 088h] // 00c471a8
        fstp qword ptr [esp + 038h] // 00c471af
        fld qword ptr [esi + 8] // 00c471b3
        fsub qword ptr [edi + 058h] // 00c471b6
        fstp qword ptr [esp + 040h] // 00c471b9
        fld qword ptr [esi + 010h] // 00c471bd
        fsub qword ptr [edi + 060h] // 00c471c0
        fstp qword ptr [esp + 048h] // 00c471c3
        fld qword ptr [esi] // 00c471c7
        fsub qword ptr [edi + 070h] // 00c471c9
        fstp qword ptr [esp + 0280h] // 00c471cc
        fld qword ptr [esi + 8] // 00c471d3
        fsub qword ptr [edi + 078h] // 00c471d6
        fstp qword ptr [esp + 0288h] // 00c471d9
        fld qword ptr [esi + 010h] // 00c471e0
        fsub qword ptr [edi + 080h] // 00c471e3
        fstp qword ptr [esp + 0290h] // 00c471e9
        call cross_reference // 00c471f0
        fld qword ptr [eax + 8] // 00c471f5
        fmul qword ptr [esp + 0288h] // 00c471f8
        fld qword ptr [esp + 0280h] // 00c471ff
        fmul qword ptr [eax] // 00c47206
        faddp st(1), st(0) // 00c47208
        fld qword ptr [eax + 010h] // 00c4720a
        fmul qword ptr [esp + 0290h] // 00c4720d
        faddp st(1), st(0) // 00c47214
        fldz  // 00c47216
        fcomi st(0), st(1) // 00c47218
        fstp st(1) // 00c4721a
        ja l_00c473bb // 00c4721c
        fstp st(0) // 00c47222
        lea ecx, [esp + 020h] // 00c47224
        fld qword ptr [esi] // 00c47228
        lea edx, [esp + 038h] // 00c4722a
        fsub qword ptr [ebx] // 00c4722e
        lea eax, [esp + 058h] // 00c47230
        fstp qword ptr [esp + 020h] // 00c47234
        fld qword ptr [esi + 8] // 00c47238
        fsub qword ptr [ebx + 8] // 00c4723b
        fstp qword ptr [esp + 028h] // 00c4723e
        fld qword ptr [esi + 010h] // 00c47242
        fsub qword ptr [ebx + 010h] // 00c47245
        fstp qword ptr [esp + 030h] // 00c47248
        fld qword ptr [edi + 050h] // 00c4724c
        fsub qword ptr [ebx] // 00c4724f
        fstp qword ptr [esp + 038h] // 00c47251
        fld qword ptr [edi + 058h] // 00c47255
        fsub qword ptr [ebx + 8] // 00c47258
        fstp qword ptr [esp + 040h] // 00c4725b
        fld qword ptr [edi + 060h] // 00c4725f
        fsub qword ptr [ebx + 010h] // 00c47262
        fstp qword ptr [esp + 048h] // 00c47265
        call cross_reference // 00c47269
        fld qword ptr [edi + 050h] // 00c4726e
        lea ecx, [esp + 058h] // 00c47271
        fsub qword ptr [ebx] // 00c47275
        lea edx, [esp + 020h] // 00c47277
        lea eax, [esp + 0a0h] // 00c4727b
        fstp qword ptr [esp + 020h] // 00c47282
        fld qword ptr [edi + 058h] // 00c47286
        fsub qword ptr [ebx + 8] // 00c47289
        fstp qword ptr [esp + 028h] // 00c4728c
        fld qword ptr [edi + 060h] // 00c47290
        fsub qword ptr [ebx + 010h] // 00c47293
        fstp qword ptr [esp + 030h] // 00c47296
        fld qword ptr [ebx] // 00c4729a
        fsub qword ptr [edi + 070h] // 00c4729c
        fstp qword ptr [esp + 0310h] // 00c4729f
        fld qword ptr [ebx + 8] // 00c472a6
        fsub qword ptr [edi + 078h] // 00c472a9
        fstp qword ptr [esp + 0318h] // 00c472ac
        fld qword ptr [ebx + 010h] // 00c472b3
        fsub qword ptr [edi + 080h] // 00c472b6
        fstp qword ptr [esp + 0320h] // 00c472bc
        call cross_reference // 00c472c3
        fld qword ptr [eax + 8] // 00c472c8
        fmul qword ptr [esp + 0318h] // 00c472cb
        fld qword ptr [esp + 0310h] // 00c472d2
        fmul qword ptr [eax] // 00c472d9
        faddp st(1), st(0) // 00c472db
        fld qword ptr [eax + 010h] // 00c472dd
        fmul qword ptr [esp + 0320h] // 00c472e0
        faddp st(1), st(0) // 00c472e7
        fldz  // 00c472e9
        fcomi st(0), st(1) // 00c472eb
        fstp st(1) // 00c472ed
        ja l_00c473bb // 00c472ef
        fstp st(0) // 00c472f5
        lea edx, [edi + 038h] // 00c472f7
        fld qword ptr [esi] // 00c472fa
        mov ecx, esi // 00c472fc
        fstp qword ptr [esi] // 00c472fe
        lea eax, [esp + 0e8h] // 00c47300
        fld qword ptr [esi + 8] // 00c47307
        fstp qword ptr [esi + 8] // 00c4730a
        fld qword ptr [esi + 010h] // 00c4730d
        fstp qword ptr [esi + 010h] // 00c47310
        fld qword ptr [ebx] // 00c47313
        fstp qword ptr [ebx] // 00c47315
        fld qword ptr [ebx + 8] // 00c47317
        fstp qword ptr [ebx + 8] // 00c4731a
        fld qword ptr [ebx + 010h] // 00c4731d
        fstp qword ptr [ebx + 010h] // 00c47320
        fld qword ptr [edi + 050h] // 00c47323
        fstp qword ptr [edx] // 00c47326
        fld qword ptr [edi + 058h] // 00c47328
        fstp qword ptr [edx + 8] // 00c4732b
        fld qword ptr [edi + 060h] // 00c4732e
        fstp qword ptr [edx + 010h] // 00c47331
        fld qword ptr [ebx] // 00c47334
        fsub qword ptr [esi] // 00c47336
        fstp qword ptr [esp + 020h] // 00c47338
        fld qword ptr [ebx + 8] // 00c4733c
        fsub qword ptr [esi + 8] // 00c4733f
        fstp qword ptr [esp + 028h] // 00c47342
        fld qword ptr [ebx + 010h] // 00c47346
        fsub qword ptr [esi + 010h] // 00c47349
        fstp qword ptr [esp + 030h] // 00c4734c
        call subtract_reference // 00c47350
        mov edx, eax // 00c47355
        lea ecx, [esp + 020h] // 00c47357
        lea eax, [esp + 038h] // 00c4735b
        call cross_reference // 00c4735f
        fld qword ptr [eax] // 00c47364
        lea ebx, [edi + 088h] // 00c47366
        fstp qword ptr [ebx] // 00c4736c
        fld qword ptr [eax + 8] // 00c4736e
        fstp qword ptr [ebx + 8] // 00c47371
        fld qword ptr [eax + 010h] // 00c47374
        lea eax, [esp + 020h] // 00c47377
    l_00c4737b:
        mov ecx, esi // 00c4737b
        fstp qword ptr [ebx + 010h] // 00c4737d
        lea edx, [edi + 070h] // 00c47380
        call subtract_reference // 00c47383
        fld qword ptr [eax + 8] // 00c47388
        fmul qword ptr [ebx + 8] // 00c4738b
        fld qword ptr [eax] // 00c4738e
        fmul qword ptr [ebx] // 00c47390
    l_00c47392:
        faddp st(1), st(0) // 00c47392
        sub esp, 8 // 00c47394
        fld qword ptr [eax + 010h] // 00c47397
        mov eax, ebx // 00c4739a
        fmul qword ptr [ebx + 010h] // 00c4739c
        faddp st(1), st(0) // 00c4739f
        fstp qword ptr [esp] // 00c473a1
        call scale_reference // 00c473a4
        mov dword ptr [edi + 068h], 3 // 00c473a9
        jmp l_00c44a87 // 00c473b0
    l_00c473b5:
        fstp st(1) // 00c473b5
        fstp st(1) // 00c473b7
        fstp st(0) // 00c473b9
    l_00c473bb:
        fld qword ptr [edi + 050h] // 00c473bb
        fsub qword ptr [esi] // 00c473be
        fld qword ptr [edi + 058h] // 00c473c0
        fsub qword ptr [esi + 8] // 00c473c3
        fld qword ptr [edi + 060h] // 00c473c6
        fsub qword ptr [esi + 010h] // 00c473c9
        fld qword ptr [edi + 038h] // 00c473cc
        fsub qword ptr [esi] // 00c473cf
        fld qword ptr [edi + 040h] // 00c473d1
        fsub qword ptr [esi + 8] // 00c473d4
        fst qword ptr [esp + 0600h] // 00c473d7
        fld qword ptr [edi + 048h] // 00c473de
        fsub qword ptr [esi + 010h] // 00c473e1
        fxch st(1) // 00c473e4
        fmul st(0), st(3) // 00c473e6
        fld st(1) // 00c473e8
        fmul st(0), st(5) // 00c473ea
        fsubp st(1), st(0) // 00c473ec
        fst qword ptr [esp + 020h] // 00c473ee
        fld st(5) // 00c473f2
        fmulp st(2), st(0) // 00c473f4
        fld st(2) // 00c473f6
        fmulp st(4), st(0) // 00c473f8
        fxch st(1) // 00c473fa
        fsubrp st(3), st(0) // 00c473fc
        fxch st(2) // 00c473fe
        fst qword ptr [esp + 028h] // 00c47400
        fxch st(1) // 00c47404
        fmulp st(3), st(0) // 00c47406
        fxch st(3) // 00c47408
        fmul qword ptr [esp + 0600h] // 00c4740a
        fsubp st(2), st(0) // 00c47411
        fxch st(1) // 00c47413
        fst qword ptr [esp + 030h] // 00c47415
        fld qword ptr [esi] // 00c47419
        fsub qword ptr [edi + 070h] // 00c4741b
        fld qword ptr [esi + 8] // 00c4741e
        fsub qword ptr [edi + 078h] // 00c47421
        fld qword ptr [esi + 010h] // 00c47424
        fsub qword ptr [edi + 080h] // 00c47427
        fld qword ptr [ebx] // 00c4742d
        fsub qword ptr [esi] // 00c4742f
        fstp qword ptr [esp + 03d0h] // 00c47431
        fld qword ptr [ebx + 8] // 00c47438
        fsub qword ptr [esi + 8] // 00c4743b
        fstp qword ptr [esp + 03d8h] // 00c4743e
        fld qword ptr [ebx + 010h] // 00c47445
        fsub qword ptr [esi + 010h] // 00c47448
        fxch st(3) // 00c4744b
        fmul st(0), st(5) // 00c4744d
        fxch st(2) // 00c4744f
        fmul st(0), st(6) // 00c47451
        faddp st(2), st(0) // 00c47453
        fmul st(0), st(3) // 00c47455
        faddp st(1), st(0) // 00c47457
        fld qword ptr [esp + 03d0h] // 00c47459
        fmul st(0), st(4) // 00c47460
        fld qword ptr [esp + 03d8h] // 00c47462
        fmul st(0), st(6) // 00c47469
        faddp st(1), st(0) // 00c4746b
        fxch st(2) // 00c4746d
        fmul st(0), st(3) // 00c4746f
        faddp st(2), st(0) // 00c47471
        fmulp st(1), st(0) // 00c47473
        fxch st(4) // 00c47475
        fcomi st(0), st(4) // 00c47477
        fstp st(4) // 00c47479
        ja l_00c476ef // 00c4747b
        fld qword ptr [edi + 038h] // 00c47481
        fsub qword ptr [esi] // 00c47484
        fld qword ptr [edi + 040h] // 00c47486
        fsub qword ptr [esi + 8] // 00c47489
        fld qword ptr [edi + 048h] // 00c4748c
        fsub qword ptr [esi + 010h] // 00c4748f
        fst qword ptr [esp + 0638h] // 00c47492
        fld st(1) // 00c47499
        fmul st(0), st(4) // 00c4749b
        fxch st(1) // 00c4749d
        fmul st(0), st(6) // 00c4749f
        fsubp st(1), st(0) // 00c474a1
        fstp qword ptr [esp + 0658h] // 00c474a3
        fld st(3) // 00c474aa
        fmul qword ptr [esp + 0638h] // 00c474ac
        fld st(2) // 00c474b3
        fmulp st(4), st(0) // 00c474b5
        fsubrp st(3), st(0) // 00c474b7
        fxch st(1) // 00c474b9
        fmulp st(4), st(0) // 00c474bb
        fmulp st(2), st(0) // 00c474bd
        fxch st(2) // 00c474bf
        fsubrp st(1), st(0) // 00c474c1
        fld qword ptr [esi] // 00c474c3
        fsub qword ptr [edi + 070h] // 00c474c5
        fld qword ptr [esi + 8] // 00c474c8
        fsub qword ptr [edi + 078h] // 00c474cb
        fld qword ptr [esi + 010h] // 00c474ce
        fsub qword ptr [edi + 080h] // 00c474d1
        fxch st(2) // 00c474d7
        fmul qword ptr [esp + 0658h] // 00c474d9
        fxch st(1) // 00c474e0
        fmulp st(4), st(0) // 00c474e2
        faddp st(3), st(0) // 00c474e4
        fmulp st(1), st(0) // 00c474e6
        faddp st(1), st(0) // 00c474e8
        fxch st(1) // 00c474ea
        fcomi st(0), st(1) // 00c474ec
        fstp st(1) // 00c474ee
        ja l_00c476f5 // 00c474f0
        fstp st(0) // 00c474f6
        lea ecx, [esp + 020h] // 00c474f8
        fld qword ptr [esi] // 00c474fc
        lea edx, [esp + 038h] // 00c474fe
        fsub qword ptr [edi + 050h] // 00c47502
        lea eax, [esp + 088h] // 00c47505
        fstp qword ptr [esp + 038h] // 00c4750c
        fld qword ptr [esi + 8] // 00c47510
        fsub qword ptr [edi + 058h] // 00c47513
        fstp qword ptr [esp + 040h] // 00c47516
        fld qword ptr [esi + 010h] // 00c4751a
        fsub qword ptr [edi + 060h] // 00c4751d
        fstp qword ptr [esp + 048h] // 00c47520
        fld qword ptr [esi] // 00c47524
        fsub qword ptr [edi + 070h] // 00c47526
        fstp qword ptr [esp + 02b0h] // 00c47529
        fld qword ptr [esi + 8] // 00c47530
        fsub qword ptr [edi + 078h] // 00c47533
        fstp qword ptr [esp + 02b8h] // 00c47536
        fld qword ptr [esi + 010h] // 00c4753d
        fsub qword ptr [edi + 080h] // 00c47540
        fstp qword ptr [esp + 02c0h] // 00c47546
        call cross_reference // 00c4754d
        fld qword ptr [eax + 8] // 00c47552
        fmul qword ptr [esp + 02b8h] // 00c47555
        fld qword ptr [esp + 02b0h] // 00c4755c
        fmul qword ptr [eax] // 00c47563
        faddp st(1), st(0) // 00c47565
        fld qword ptr [eax + 010h] // 00c47567
        fmul qword ptr [esp + 02c0h] // 00c4756a
        faddp st(1), st(0) // 00c47571
        fldz  // 00c47573
        fcomi st(0), st(1) // 00c47575
        fstp st(1) // 00c47577
        ja l_00c476f5 // 00c47579
        fstp st(0) // 00c4757f
        lea ecx, [esp + 020h] // 00c47581
        fld qword ptr [esi] // 00c47585
        lea edx, [esp + 038h] // 00c47587
        fsub qword ptr [edi + 038h] // 00c4758b
        lea eax, [esp + 058h] // 00c4758e
        fstp qword ptr [esp + 020h] // 00c47592
        fld qword ptr [esi + 8] // 00c47596
        fsub qword ptr [edi + 040h] // 00c47599
        fstp qword ptr [esp + 028h] // 00c4759c
        fld qword ptr [esi + 010h] // 00c475a0
        fsub qword ptr [edi + 048h] // 00c475a3
        fstp qword ptr [esp + 030h] // 00c475a6
        fld qword ptr [edi + 050h] // 00c475aa
        fsub qword ptr [edi + 038h] // 00c475ad
        fstp qword ptr [esp + 038h] // 00c475b0
        fld qword ptr [edi + 058h] // 00c475b4
        fsub qword ptr [edi + 040h] // 00c475b7
        fstp qword ptr [esp + 040h] // 00c475ba
        fld qword ptr [edi + 060h] // 00c475be
        fsub qword ptr [edi + 048h] // 00c475c1
        fstp qword ptr [esp + 048h] // 00c475c4
        call cross_reference // 00c475c8
        fld qword ptr [edi + 050h] // 00c475cd
        lea ecx, [esp + 058h] // 00c475d0
        fsub qword ptr [edi + 038h] // 00c475d4
        lea edx, [esp + 020h] // 00c475d7
        lea eax, [esp + 0a0h] // 00c475db
        fstp qword ptr [esp + 020h] // 00c475e2
        fld qword ptr [edi + 058h] // 00c475e6
        fsub qword ptr [edi + 040h] // 00c475e9
        fstp qword ptr [esp + 028h] // 00c475ec
        fld qword ptr [edi + 060h] // 00c475f0
        fsub qword ptr [edi + 048h] // 00c475f3
        fstp qword ptr [esp + 030h] // 00c475f6
        fld qword ptr [edi + 038h] // 00c475fa
        fsub qword ptr [edi + 070h] // 00c475fd
        fstp qword ptr [esp + 0370h] // 00c47600
        fld qword ptr [edi + 040h] // 00c47607
        fsub qword ptr [edi + 078h] // 00c4760a
        fstp qword ptr [esp + 0378h] // 00c4760d
        fld qword ptr [edi + 048h] // 00c47614
        fsub qword ptr [edi + 080h] // 00c47617
        fstp qword ptr [esp + 0380h] // 00c4761d
        call cross_reference // 00c47624
        fld qword ptr [eax + 8] // 00c47629
        fmul qword ptr [esp + 0378h] // 00c4762c
        fld qword ptr [esp + 0370h] // 00c47633
        fmul qword ptr [eax] // 00c4763a
        faddp st(1), st(0) // 00c4763c
        fld qword ptr [eax + 010h] // 00c4763e
        fmul qword ptr [esp + 0380h] // 00c47641
        faddp st(1), st(0) // 00c47648
        fldz  // 00c4764a
        fcomi st(0), st(1) // 00c4764c
        fstp st(1) // 00c4764e
        ja l_00c476f5 // 00c47650
        fstp st(0) // 00c47656
        mov ecx, esi // 00c47658
        fld qword ptr [esi] // 00c4765a
        lea eax, [esp + 0e8h] // 00c4765c
        fstp qword ptr [esi] // 00c47663
        mov edx, ebx // 00c47665
        fld qword ptr [esi + 8] // 00c47667
        fstp qword ptr [esi + 8] // 00c4766a
        fld qword ptr [esi + 010h] // 00c4766d
        fstp qword ptr [esi + 010h] // 00c47670
        fld qword ptr [edi + 038h] // 00c47673
        fstp qword ptr [ebx] // 00c47676
        fld qword ptr [edi + 040h] // 00c47678
        fstp qword ptr [ebx + 8] // 00c4767b
        fld qword ptr [edi + 048h] // 00c4767e
        fstp qword ptr [ebx + 010h] // 00c47681
        fld qword ptr [edi + 050h] // 00c47684
        fstp qword ptr [edi + 038h] // 00c47687
        fld qword ptr [edi + 058h] // 00c4768a
        fstp qword ptr [edi + 040h] // 00c4768d
        fld qword ptr [edi + 060h] // 00c47690
        fstp qword ptr [edi + 048h] // 00c47693
        call subtract_reference // 00c47696
        mov ebx, eax // 00c4769b
        mov ecx, esi // 00c4769d
        lea eax, [esp + 020h] // 00c4769f
        lea edx, [edi + 038h] // 00c476a3
        call subtract_reference // 00c476a6
        mov edx, eax // 00c476ab
        mov ecx, ebx // 00c476ad
        lea eax, [esp + 038h] // 00c476af
        call cross_reference // 00c476b3
        fld qword ptr [eax] // 00c476b8
        fstp qword ptr [edi + 088h] // 00c476ba
        lea ebx, [edi + 088h] // 00c476c0
        fld qword ptr [eax + 8] // 00c476c6
        mov ecx, esi // 00c476c9
        fstp qword ptr [ebx + 8] // 00c476cb
        lea edx, [edi + 070h] // 00c476ce
        fld qword ptr [eax + 010h] // 00c476d1
        lea eax, [esp + 058h] // 00c476d4
        fstp qword ptr [ebx + 010h] // 00c476d8
        call subtract_reference // 00c476db
        fld qword ptr [eax + 8] // 00c476e0
        fmul qword ptr [ebx + 8] // 00c476e3
        fld qword ptr [ebx] // 00c476e6
        fmul qword ptr [eax] // 00c476e8
        jmp l_00c47392 // 00c476ea
    l_00c476ef:
        fstp st(1) // 00c476ef
        fstp st(1) // 00c476f1
        fstp st(0) // 00c476f3
    l_00c476f5:
        fld qword ptr [edi + 050h] // 00c476f5
        fsub qword ptr [ebx] // 00c476f8
        fld qword ptr [edi + 058h] // 00c476fa
        fsub qword ptr [ebx + 8] // 00c476fd
        fld qword ptr [edi + 060h] // 00c47700
        fsub qword ptr [ebx + 010h] // 00c47703
        fld qword ptr [edi + 038h] // 00c47706
        fsub qword ptr [ebx] // 00c47709
        fld qword ptr [edi + 040h] // 00c4770b
        fsub qword ptr [ebx + 8] // 00c4770e
        fld qword ptr [edi + 048h] // 00c47711
        fsub qword ptr [ebx + 010h] // 00c47714
        fst qword ptr [esp + 0698h] // 00c47717
        fld st(1) // 00c4771e
        fmul st(0), st(4) // 00c47720
        fxch st(1) // 00c47722
        fmul st(0), st(5) // 00c47724
        fsubp st(1), st(0) // 00c47726
        fst qword ptr [esp + 020h] // 00c47728
        fld qword ptr [esp + 0698h] // 00c4772c
        fmul st(0), st(6) // 00c47733
        fxch st(4) // 00c47735
        fmul st(0), st(3) // 00c47737
        fsubp st(4), st(0) // 00c47739
        fxch st(3) // 00c4773b
        fst qword ptr [esp + 028h] // 00c4773d
        fxch st(4) // 00c47741
        fmulp st(2), st(0) // 00c47743
        fmulp st(4), st(0) // 00c47745
        fsubrp st(3), st(0) // 00c47747
        fxch st(2) // 00c47749
        fst qword ptr [esp + 030h] // 00c4774b
        fld qword ptr [ebx] // 00c4774f
        fsub qword ptr [edi + 070h] // 00c47751
        fld qword ptr [ebx + 8] // 00c47754
        fsub qword ptr [edi + 078h] // 00c47757
        fld qword ptr [ebx + 010h] // 00c4775a
        fsub qword ptr [edi + 080h] // 00c4775d
        fld qword ptr [esi] // 00c47763
        fsub qword ptr [ebx] // 00c47765
        fstp qword ptr [esp + 0400h] // 00c47767
        fld qword ptr [esi + 8] // 00c4776e
        fsub qword ptr [ebx + 8] // 00c47771
        fstp qword ptr [esp + 0408h] // 00c47774
        fld qword ptr [esi + 010h] // 00c4777b
        fsub qword ptr [ebx + 010h] // 00c4777e
        fxch st(2) // 00c47781
        fmul st(0), st(5) // 00c47783
        fxch st(3) // 00c47785
        fmul st(0), st(6) // 00c47787
        faddp st(3), st(0) // 00c47789
        fmul st(0), st(3) // 00c4778b
        faddp st(2), st(0) // 00c4778d
        fld qword ptr [esp + 0408h] // 00c4778f
        fmul st(0), st(4) // 00c47796
        fld qword ptr [esp + 0400h] // 00c47798
        fmul st(0), st(6) // 00c4779f
        faddp st(1), st(0) // 00c477a1
        fxch st(1) // 00c477a3
        fmul st(0), st(3) // 00c477a5
        faddp st(1), st(0) // 00c477a7
        fmulp st(1), st(0) // 00c477a9
        fxch st(4) // 00c477ab
        fcomi st(0), st(4) // 00c477ad
        fstp st(4) // 00c477af
        ja l_00c479f8 // 00c477b1
        fld qword ptr [edi + 038h] // 00c477b7
        fsub qword ptr [ebx] // 00c477ba
        fld qword ptr [edi + 040h] // 00c477bc
        fsub qword ptr [ebx + 8] // 00c477bf
        fld qword ptr [edi + 048h] // 00c477c2
        fsub qword ptr [ebx + 010h] // 00c477c5
        fst qword ptr [esp + 06c8h] // 00c477c8
        fld st(1) // 00c477cf
        fmul st(0), st(4) // 00c477d1
        fxch st(1) // 00c477d3
        fmul st(0), st(5) // 00c477d5
        fsubp st(1), st(0) // 00c477d7
        fld qword ptr [esp + 06c8h] // 00c477d9
        fmul st(0), st(6) // 00c477e0
        fxch st(4) // 00c477e2
        fmul st(0), st(3) // 00c477e4
        fsubp st(4), st(0) // 00c477e6
        fxch st(4) // 00c477e8
        fmulp st(2), st(0) // 00c477ea
        fmulp st(4), st(0) // 00c477ec
        fsubrp st(3), st(0) // 00c477ee
        fld qword ptr [ebx] // 00c477f0
        fsub qword ptr [edi + 070h] // 00c477f2
        fld qword ptr [ebx + 8] // 00c477f5
        fsub qword ptr [edi + 078h] // 00c477f8
        fld qword ptr [ebx + 010h] // 00c477fb
        fsub qword ptr [edi + 080h] // 00c477fe
        fxch st(1) // 00c47804
        fmulp st(3), st(0) // 00c47806
        fxch st(1) // 00c47808
        fmulp st(3), st(0) // 00c4780a
        fxch st(1) // 00c4780c
        faddp st(2), st(0) // 00c4780e
        fmulp st(2), st(0) // 00c47810
        faddp st(1), st(0) // 00c47812
        fxch st(1) // 00c47814
        fcomip st(0), st(1) // 00c47816
        fstp st(0) // 00c47818
        ja l_00c47a00 // 00c4781a
        fld qword ptr [ebx] // 00c47820
        lea ecx, [esp + 020h] // 00c47822
        fsub qword ptr [edi + 050h] // 00c47826
        lea edx, [esp + 038h] // 00c47829
        lea eax, [esp + 088h] // 00c4782d
        fstp qword ptr [esp + 038h] // 00c47834
        fld qword ptr [ebx + 8] // 00c47838
        fsub qword ptr [edi + 058h] // 00c4783b
        fstp qword ptr [esp + 040h] // 00c4783e
        fld qword ptr [ebx + 010h] // 00c47842
        fsub qword ptr [edi + 060h] // 00c47845
        fstp qword ptr [esp + 048h] // 00c47848
        fld qword ptr [ebx] // 00c4784c
        fsub qword ptr [edi + 070h] // 00c4784e
        fstp qword ptr [esp + 0208h] // 00c47851
        fld qword ptr [ebx + 8] // 00c47858
        fsub qword ptr [edi + 078h] // 00c4785b
        fstp qword ptr [esp + 0210h] // 00c4785e
        fld qword ptr [ebx + 010h] // 00c47865
        fsub qword ptr [edi + 080h] // 00c47868
        fstp qword ptr [esp + 0218h] // 00c4786e
        call cross_reference // 00c47875
        fld qword ptr [eax + 8] // 00c4787a
        fmul qword ptr [esp + 0210h] // 00c4787d
        fld qword ptr [eax] // 00c47884
        fmul qword ptr [esp + 0208h] // 00c47886
        faddp st(1), st(0) // 00c4788d
        fld qword ptr [eax + 010h] // 00c4788f
        fmul qword ptr [esp + 0218h] // 00c47892
        faddp st(1), st(0) // 00c47899
        fldz  // 00c4789b
        fcomip st(0), st(1) // 00c4789d
        fstp st(0) // 00c4789f
        ja l_00c47a00 // 00c478a1
        fld qword ptr [ebx] // 00c478a7
        lea ecx, [esp + 020h] // 00c478a9
        fsub qword ptr [edi + 038h] // 00c478ad
        lea edx, [esp + 038h] // 00c478b0
        lea eax, [esp + 058h] // 00c478b4
        fstp qword ptr [esp + 020h] // 00c478b8
        fld qword ptr [ebx + 8] // 00c478bc
        fsub qword ptr [edi + 040h] // 00c478bf
        fstp qword ptr [esp + 028h] // 00c478c2
        fld qword ptr [ebx + 010h] // 00c478c6
        fsub qword ptr [edi + 048h] // 00c478c9
        fstp qword ptr [esp + 030h] // 00c478cc
        fld qword ptr [edi + 050h] // 00c478d0
        fsub qword ptr [edi + 038h] // 00c478d3
        fstp qword ptr [esp + 038h] // 00c478d6
        fld qword ptr [edi + 058h] // 00c478da
        fsub qword ptr [edi + 040h] // 00c478dd
        fstp qword ptr [esp + 040h] // 00c478e0
        fld qword ptr [edi + 060h] // 00c478e4
        fsub qword ptr [edi + 048h] // 00c478e7
        fstp qword ptr [esp + 048h] // 00c478ea
        call cross_reference // 00c478ee
        fld qword ptr [edi + 050h] // 00c478f3
        lea ecx, [esp + 058h] // 00c478f6
        fsub qword ptr [edi + 038h] // 00c478fa
        lea edx, [esp + 020h] // 00c478fd
        lea eax, [esp + 0a0h] // 00c47901
        fstp qword ptr [esp + 020h] // 00c47908
        fld qword ptr [edi + 058h] // 00c4790c
        fsub qword ptr [edi + 040h] // 00c4790f
        fstp qword ptr [esp + 028h] // 00c47912
        fld qword ptr [edi + 060h] // 00c47916
        fsub qword ptr [edi + 048h] // 00c47919
        fstp qword ptr [esp + 030h] // 00c4791c
        fld qword ptr [edi + 038h] // 00c47920
        fsub qword ptr [edi + 070h] // 00c47923
        fstp qword ptr [esp + 0238h] // 00c47926
        fld qword ptr [edi + 040h] // 00c4792d
        fsub qword ptr [edi + 078h] // 00c47930
        fstp qword ptr [esp + 0240h] // 00c47933
        fld qword ptr [edi + 048h] // 00c4793a
        fsub qword ptr [edi + 080h] // 00c4793d
        fstp qword ptr [esp + 0248h] // 00c47943
        call cross_reference // 00c4794a
        fld qword ptr [eax + 8] // 00c4794f
        fmul qword ptr [esp + 0240h] // 00c47952
        fld qword ptr [eax] // 00c47959
        fmul qword ptr [esp + 0238h] // 00c4795b
        faddp st(1), st(0) // 00c47962
        fld qword ptr [eax + 010h] // 00c47964
        fmul qword ptr [esp + 0248h] // 00c47967
        faddp st(1), st(0) // 00c4796e
        fldz  // 00c47970
        fcomip st(0), st(1) // 00c47972
        fstp st(0) // 00c47974
        ja l_00c47a00 // 00c47976
        fld qword ptr [ebx] // 00c4797c
        mov ecx, esi // 00c4797e
        fstp qword ptr [esi] // 00c47980
        lea eax, [esp + 0e8h] // 00c47982
        fld qword ptr [ebx + 8] // 00c47989
        mov edx, ebx // 00c4798c
        fstp qword ptr [esi + 8] // 00c4798e
        fld qword ptr [ebx + 010h] // 00c47991
        fstp qword ptr [esi + 010h] // 00c47994
        fld qword ptr [edi + 038h] // 00c47997
        fstp qword ptr [ebx] // 00c4799a
        fld qword ptr [edi + 040h] // 00c4799c
        fstp qword ptr [ebx + 8] // 00c4799f
        fld qword ptr [edi + 048h] // 00c479a2
        fstp qword ptr [ebx + 010h] // 00c479a5
        fld qword ptr [edi + 050h] // 00c479a8
        fstp qword ptr [edi + 038h] // 00c479ab
        fld qword ptr [edi + 058h] // 00c479ae
        fstp qword ptr [edi + 040h] // 00c479b1
        fld qword ptr [edi + 060h] // 00c479b4
        fstp qword ptr [edi + 048h] // 00c479b7
        call subtract_reference // 00c479ba
        mov ebx, eax // 00c479bf
        mov ecx, esi // 00c479c1
        lea eax, [esp + 020h] // 00c479c3
        lea edx, [edi + 038h] // 00c479c7
        call subtract_reference // 00c479ca
        mov edx, eax // 00c479cf
        mov ecx, ebx // 00c479d1
        lea eax, [esp + 038h] // 00c479d3
        call cross_reference // 00c479d7
        fld qword ptr [eax] // 00c479dc
        lea ebx, [edi + 088h] // 00c479de
        fstp qword ptr [ebx] // 00c479e4
        fld qword ptr [eax + 8] // 00c479e6
        fstp qword ptr [ebx + 8] // 00c479e9
        fld qword ptr [eax + 010h] // 00c479ec
        lea eax, [esp + 058h] // 00c479ef
        jmp l_00c4737b // 00c479f3
    l_00c479f8:
        fstp st(3) // 00c479f8
        fstp st(1) // 00c479fa
        fstp st(0) // 00c479fc
        fstp st(0) // 00c479fe
    l_00c47a00:
        cmp dword ptr [edi + 068h], 4 // 00c47a00
        jl l_00c44a87 // 00c47a04
    l_00c47a0a:
        mov eax, dword ptr [ebp + 8] // 00c47a0a
        fld qword ptr [esp + 0d0h] // 00c47a0d
        fstp dword ptr [eax + 0ch] // 00c47a14
        fld qword ptr [esp + 0d8h] // 00c47a17
        fstp dword ptr [eax + 010h] // 00c47a1e
        fld qword ptr [esp + 0e0h] // 00c47a21
        fstp dword ptr [eax + 014h] // 00c47a28
        fld qword ptr [edi + 070h] // 00c47a2b
        fstp dword ptr [eax] // 00c47a2e
        fld qword ptr [edi + 078h] // 00c47a30
        fstp dword ptr [eax + 4] // 00c47a33
        fld qword ptr [edi + 080h] // 00c47a36
        fstp dword ptr [eax + 8] // 00c47a3c
        mov al, 1 // 00c47a3f
        pop edi // 00c47a41
        pop esi // 00c47a42
        pop ebx // 00c47a43
        mov esp, ebp // 00c47a44
        pop ebp // 00c47a46
        ret 14h // 00c47a47
    l_00c47a4a:
        fstp st(5) // 00c47a4a
        xor al, al // 00c47a4c
        fstp st(0) // 00c47a4e
        fstp st(2) // 00c47a50
        fstp st(0) // 00c47a52
        fstp st(0) // 00c47a54
        fstp st(0) // 00c47a56
        pop edi // 00c47a58
        pop esi // 00c47a59
        pop ebx // 00c47a5a
        mov esp, ebp // 00c47a5b
        pop ebp // 00c47a5d
        ret 14h // 00c47a5e
    l_00c47a61:
        fstp st(5) // 00c47a61
        xor al, al // 00c47a63
        fstp st(4) // 00c47a65
        fstp st(2) // 00c47a67
        fstp st(0) // 00c47a69
        fstp st(0) // 00c47a6b
        fstp st(0) // 00c47a6d
        pop edi // 00c47a6f
        pop esi // 00c47a70
        pop ebx // 00c47a71
        mov esp, ebp // 00c47a72
        pop ebp // 00c47a74
        ret 14h // 00c47a75
    l_00c47a78:
        fstp st(3) // 00c47a78
        pop edi // 00c47a7a
        fstp st(2) // 00c47a7b
        pop esi // 00c47a7d
        fstp st(0) // 00c47a7e
        xor al, al // 00c47a80
        fstp st(0) // 00c47a82
        pop ebx // 00c47a84
        mov esp, ebp // 00c47a85
        pop ebp // 00c47a87
        ret 14h // 00c47a88
    }
}
} // namespace
bool intersect_native_dyn_convex_ray_00c44780(DynConvexRayIntersectionStorage& storage,
    void* result,const void* shape,const float* start,const float* end,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;auto* owner=&storage;unsigned char answer;
    __asm {
        push c
        push end
        push start
        push shape
        push result
        mov ecx,owner
        call ray_kernel
        mov answer,al
    }
    return answer!=0;
}
static_assert(std::is_standard_layout_v<NativeDynConvexRayRuntime>);
NativeDynConvexRayRuntime::NativeDynConvexRayRuntime(const CameraAxesCrtAccess& crt) noexcept
    :methods_{reinterpret_cast<std::uintptr_t>(&intersect)},crt_(crt) {}
bool __fastcall NativeDynConvexRayRuntime::intersect(void* p,void*,void* result,const void* shape,const float* start,const float* end){
    auto& owner=*static_cast<DynConvexRayIntersectionStorage*>(p);
    const auto& runtime=*static_cast<const NativeDynConvexRayRuntime*>(owner.vtable);
    return intersect_native_dyn_convex_ray_00c44780(owner,result,shape,start,end,runtime.crt_);
}
} // namespace bsp
