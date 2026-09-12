#include "bsp/native_particle_record_children.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle record children requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(NativeParticleModelArraysStorage) == 0x18);
static_assert(offsetof(NativeParticleRecordChildrenAccess, random) == 0);
static_assert(offsetof(NativeParticleRecordChildrenAccess, floor_00bf85b0) == 4);
static_assert(offsetof(NativeParticleRecordChildrenAccess, truncate_st0_00bf7420) == 8);
static_assert(offsetof(NativeParticleRecordChildrenAccess, zero_00d7a218) == 12);
static_assert(offsetof(NativeParticleRecordChildrenAccess, one_00d7a24c) == 16);
static_assert(offsetof(NativeParticleRecordChildrenAccess, signed_random_scale_00d5da30) == 20);
static_assert(offsetof(NativeParticleRecordChildrenAccess, random_offset_00d7a210) == 24);
static_assert(offsetof(NativeParticleRecordChildrenAccess, percent_00d7a220) == 28);
static_assert(offsetof(NativeParticleRecordChildrenAccess, definition_virtual10) == 32);
std::uint32_t __fastcall random_bridge(RandomStream stream, const NativeParticleRecordChildrenAccess* access) {
    return access->random->next_00bd2fc0(stream);
}
} // namespace
// Each kernel adds four bytes for borrowed access. Native outgoing argument
// and local offsets stay intact; incoming words shift only where indicated.
// The original interpolation's incoming matrix word doubles as float scratch.

__declspec(naked) void __fastcall initialize_native_particle_record_00afe1a0(void*, const volatile std::uint32_t*, NativeNodeStorage*, void*, float, const void*) {
    __asm {
        push edx // current one address
        xorps xmm0,xmm0 // 00afe1a0
        mov edx,dword ptr [esp + 08h] // 00afe1a3
        movss xmm1,dword ptr [esp + 010h] // 00afe1a7
        xor eax,eax // 00afe1ad
        mov dword ptr [ecx + 0b8h],eax // 00afe1af
        mov dword ptr [ecx + 0bch],eax // 00afe1b5
        mov dword ptr [ecx + 0c0h],eax // 00afe1bb
        mov dword ptr [ecx + 0c4h],eax // 00afe1c1
        mov dword ptr [ecx + 0c8h],eax // 00afe1c7
        mov dword ptr [ecx + 0cch],eax // 00afe1cd
        mov dword ptr [ecx + 0d0h],eax // 00afe1d3
        mov dword ptr [ecx + 0d4h],eax // 00afe1d9
        mov dword ptr [ecx + 0d8h],eax // 00afe1df
        mov dword ptr [ecx + 0dch],eax // 00afe1e5
        mov dword ptr [ecx + 0e0h],eax // 00afe1eb
        mov dword ptr [ecx + 0e4h],eax // 00afe1f1
        movss dword ptr [ecx + 0e8h],xmm0 // 00afe1f7
        movss dword ptr [ecx + 0ech],xmm0 // 00afe1ff
        movss dword ptr [ecx + 0f0h],xmm0 // 00afe207
        movss dword ptr [ecx + 0f4h],xmm0 // 00afe20f
        movss dword ptr [ecx + 0f8h],xmm0 // 00afe217
        movss dword ptr [ecx + 0fch],xmm0 // 00afe21f
        movss dword ptr [ecx + 0100h],xmm0 // 00afe227
        movss dword ptr [ecx + 0104h],xmm0 // 00afe22f
        mov dword ptr [ecx + 0a4h],edx // 00afe237
        mov edx,dword ptr [esp + 0ch] // 00afe23d
        movss dword ptr [ecx + 030h],xmm1 // 00afe241
        push eax // added binding
        mov eax,dword ptr [esp+04h] // added binding
        movss xmm1,dword ptr [eax] // 00afe246
        pop eax
        mov dword ptr [ecx + 040h],eax // 00afe24e
        mov dword ptr [ecx + 044h],eax // 00afe251
        mov eax,dword ptr [esp + 014h] // 00afe254
        mov dword ptr [ecx + 0a0h],edx // 00afe258
        movss dword ptr [ecx + 034h],xmm0 // 00afe25e
        movss dword ptr [ecx + 0b4h],xmm1 // 00afe263
        fld dword ptr [eax] // 00afe26b
        fstp dword ptr [ecx + 054h] // 00afe26d
        fld dword ptr [eax + 04h] // 00afe270
        fstp dword ptr [ecx + 058h] // 00afe273
        fld dword ptr [eax + 08h] // 00afe276
        fstp dword ptr [ecx + 05ch] // 00afe279
        movss dword ptr [ecx + 024h],xmm0 // 00afe27c
        movss dword ptr [ecx + 028h],xmm0 // 00afe281
        movss dword ptr [ecx + 02ch],xmm0 // 00afe286
        add esp,04h // added binding
        ret 010h // 00afe28b
    }
}

__declspec(naked) void __fastcall interpolate_native_particle_record_00afdbf0(const void*, const NativeParticleRecordChildrenAccess*, void*, void*, void*, std::int32_t, float) {
    __asm {
        sub esp,028h // 00afdbf0
        mov dword ptr [esp+024h],edx // borrowed access
        mov edx,dword ptr [esp + 034h] // 00afdbf3
        push esi // 00afdbf7
        push edi // 00afdbf8
        mov edi,dword ptr [esp + 034h] // 00afdbf9
        push edx // 00afdbfd
        mov edx,dword ptr [esp + 03ch] // 00afdbfe
        mov esi,ecx // 00afdc02
        mov ecx,dword ptr [esi + 0a0h] // 00afdc04
        mov eax,dword ptr [ecx] // 00afdc0a
        mov eax,dword ptr [eax + 010h] // 00afdc0c
        push edx // 00afdc0f
        push edi // 00afdc10
        push esi // 00afdc11
        mov edx,eax // already captured virtual10 // added binding
        mov eax,dword ptr [esp+03ch] // added binding
        call dword ptr [eax+020h] // 00afdc12
        fld dword ptr [esi] // 00afdc14
        fstp dword ptr [esp + 03ch] // 00afdc16
        fld dword ptr [esp + 03ch] // 00afdc1a
        fld st(0) // 00afdc1e
        fsub dword ptr [esi + 0ch] // 00afdc20
        fstp dword ptr [esp + 08h] // 00afdc23
        fld dword ptr [esi + 04h] // 00afdc27
        fstp dword ptr [esp + 03ch] // 00afdc2a
        fld dword ptr [esp + 03ch] // 00afdc2e
        fld st(0) // 00afdc32
        fsub dword ptr [esi + 010h] // 00afdc34
        fstp dword ptr [esp + 0ch] // 00afdc37
        fld dword ptr [esi + 08h] // 00afdc3b
        fstp dword ptr [esp + 03ch] // 00afdc3e
        fld dword ptr [esp + 03ch] // 00afdc42
        fld st(0) // 00afdc46
        fsub dword ptr [esi + 014h] // 00afdc48
        fstp dword ptr [esp + 010h] // 00afdc4b
        fld dword ptr [esp + 08h] // 00afdc4f
        fld dword ptr [esp + 044h] // 00afdc53
        fld st(0) // 00afdc57
        fmulp st(2),st(0) // 00afdc59
        fxch // 00afdc5b
        fstp dword ptr [esp + 020h] // 00afdc5d
        fld dword ptr [esp + 0ch] // 00afdc61
        fmul st(0),st(1) // 00afdc65
        fstp dword ptr [esp + 024h] // 00afdc67
        fmul dword ptr [esp + 010h] // 00afdc6b
        fstp dword ptr [esp + 028h] // 00afdc6f
        fld dword ptr [edi] // 00afdc73
        fsubrp st(3),st(0) // 00afdc75
        fxch st(2) // 00afdc77
        fstp dword ptr [esp + 08h] // 00afdc79
        fsubr dword ptr [edi + 04h] // 00afdc7d
        fstp dword ptr [esp + 0ch] // 00afdc80
        fsubr dword ptr [edi + 08h] // 00afdc84
        fstp dword ptr [esp + 010h] // 00afdc87
        fld dword ptr [esi + 0ch] // 00afdc8b
        fadd dword ptr [esp + 08h] // 00afdc8e
        fstp dword ptr [esp + 014h] // 00afdc92
        fld dword ptr [esi + 010h] // 00afdc96
        fadd dword ptr [esp + 0ch] // 00afdc99
        fstp dword ptr [esp + 018h] // 00afdc9d
        fld dword ptr [esi + 014h] // 00afdca1
        fadd dword ptr [esp + 010h] // 00afdca4
        fstp dword ptr [esp + 01ch] // 00afdca8
        fld dword ptr [esp + 014h] // 00afdcac
        fadd dword ptr [esp + 020h] // 00afdcb0
        fstp dword ptr [esp + 08h] // 00afdcb4
        fld dword ptr [esp + 018h] // 00afdcb8
        fadd dword ptr [esp + 024h] // 00afdcbc
        fstp dword ptr [esp + 0ch] // 00afdcc0
        fld dword ptr [esp + 01ch] // 00afdcc4
        fadd dword ptr [esp + 028h] // 00afdcc8
        fstp dword ptr [esp + 010h] // 00afdccc
        fld dword ptr [esp + 08h] // 00afdcd0
        fstp dword ptr [edi] // 00afdcd4
        fld dword ptr [esp + 0ch] // 00afdcd6
        fstp dword ptr [edi + 04h] // 00afdcda
        fld dword ptr [esp + 010h] // 00afdcdd
        fstp dword ptr [edi + 08h] // 00afdce1
        pop edi // 00afdce4
        pop esi // 00afdce5
        add esp,028h // 00afdce6
        ret 014h // 00afdce9
    }
}

__declspec(naked) std::int32_t __fastcall append_native_particle_record_children_00afd440(NativeParticleModelArraysStorage*, const NativeParticleRecordChildrenAccess*, NativeNodeStorage*, void*, const void*, float, float, float) {
    __asm {
        sub esp,094h // 00afd440
        mov dword ptr [esp+090h],edx // borrowed access
        fld dword ptr [esp + 0a4h] // 00afd446
        push ebx // 00afd44d
        sub esp,08h // 00afd44e
        fstp qword ptr [esp] // 00afd451
        mov ebx,ecx // 00afd454
        mov edx,dword ptr [esp+09ch] // added binding
        call dword ptr [edx+04h] // 00afd456
        fstp dword ptr [esp + 010h] // 00afd45b
        fld dword ptr [esp + 010h] // 00afd45f
        add esp,08h // 00afd463
        mov edx,dword ptr [esp+094h] // added binding
        call dword ptr [edx+08h] // 00afd466
        xor edx,edx // 00afd46b
        cmp eax,edx // 00afd46d
        mov dword ptr [esp + 0ch],eax // 00afd46f
        jg l_00afd481 // 00afd473
        xor eax,eax // 00afd475
        pop ebx // 00afd477
        add esp,094h // 00afd478
        ret 018h // 00afd47e
    L_00afd481:
        mov ecx,dword ptr [ebx + 08h] // 00afd481
        sub ecx,dword ptr [ebx + 014h] // 00afd484
        cmp ecx,eax // 00afd487
        jge l_00afd491 // 00afd489
        mov dword ptr [esp + 0ch],ecx // 00afd48b
        mov eax,ecx // 00afd48f
    L_00afd491:
        cmp eax,edx // 00afd491
        fild dword ptr [esp + 0ch] // 00afd493
        fld1 // 00afd497
        mov dword ptr [esp + 018h],edx // 00afd499
        fdivrp st(1),st(0) // 00afd49d
        mov dword ptr [esp + 010h],edx // 00afd49f
        fstp dword ptr [esp + 01ch] // 00afd4a3
        jle l_00afd785 // 00afd4a7
        push ebp // 00afd4ad
        mov ebp,dword ptr [esp + 0a0h] // 00afd4ae
        push edi // 00afd4b5
        mov edi,dword ptr [esp + 0a8h] // 00afd4b6
        push esi // 00afd4bd
        mov edi,edi // 00afd4be
    L_00afd4c0:
        mov eax,dword ptr [ebx + 014h] // 00afd4c0
        cmp eax,dword ptr [ebx + 08h] // 00afd4c3
        jge l_00afd782 // 00afd4c6
        mov ecx,dword ptr [ebx + 04h] // 00afd4cc
        fld dword ptr [esp + 0b8h] // 00afd4cf
        movzx esi,byte ptr [ecx + eax*01h] // 00afd4d6
        movss xmm0,dword ptr [ebp + 0200h] // 00afd4da
        imul esi,esi,0108h // 00afd4e2
        add esi,dword ptr [ebx + 0ch] // 00afd4e8
        lea edx,[esp + 03ch] // 00afd4eb
        push edx // 00afd4ef
        push ecx // 00afd4f0
        movss dword ptr [esp + 044h],xmm0 // 00afd4f1
        movss xmm0,dword ptr [ebp + 0204h] // 00afd4f7
        fstp dword ptr [esp] // 00afd4ff
        push edi // 00afd502
        movss dword ptr [esp + 04ch],xmm0 // 00afd503
        movss xmm0,dword ptr [ebp + 0208h] // 00afd509
        push ebp // 00afd511
        mov ecx,esi // 00afd512
        movss dword ptr [esp + 054h],xmm0 // 00afd514
        mov edx,dword ptr [esp+0b0h] // added binding
        mov edx,dword ptr [edx+010h] // added binding
        call initialize_native_particle_record_00afe1a0 // 00afd51a
        mov eax,dword ptr [edi + 020h] // 00afd51f
        movss xmm0,dword ptr [eax] // 00afd522
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+0ch] // added binding
        ucomiss xmm0,dword ptr [eax] // 00afd526
        pop eax
        lahf // 00afd52d
        test ah,044h // 00afd52e
        movss dword ptr [esp + 038h],xmm0 // 00afd531
        jp l_00afd549 // 00afd537
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+010h] // added binding
        movss xmm0,dword ptr [eax] // 00afd539
        pop eax
        movss dword ptr [esp + 014h],xmm0 // 00afd541
        jmp l_00afd56c // 00afd547
    L_00afd549:
        xor ecx,ecx // 00afd549
        mov edx,dword ptr [esp+0a0h] // added binding
        call random_bridge // 00afd54b
        mov dword ptr [esp + 014h],eax // 00afd550
        fild dword ptr [esp + 014h] // 00afd554
        fmul dword ptr [esp + 038h] // 00afd558
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+014h] // added binding
        fmul qword ptr [eax] // 00afd55c
        pop eax
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+018h] // added binding
        fadd qword ptr [eax] // 00afd562
        pop eax
        fstp dword ptr [esp + 014h] // 00afd568
    L_00afd56c:
        mov ecx,dword ptr [edi + 020h] // 00afd56c
        movzx eax,word ptr [ecx + 0ah] // 00afd56f
        test ax,ax // 00afd573
        jnz l_00afd585 // 00afd576
        movss xmm0,dword ptr [ecx + 04h] // 00afd578
        movss dword ptr [esp + 020h],xmm0 // 00afd57d
        jmp l_00afd5a1 // 00afd583
    L_00afd585:
        cmp ax,01h // 00afd585
        fldz // 00afd589
        push ecx // 00afd58b
        fstp dword ptr [esp] // 00afd58c
        jnz l_00afd598 // 00afd58f
        call evaluate_native_particle_linear_curve_00affa70 // 00afd591
        jmp l_00afd59d // 00afd596
    L_00afd598:
        call evaluate_native_particle_cubic_curve_00affae0 // 00afd598
    L_00afd59d:
        fstp dword ptr [esp + 020h] // 00afd59d
    L_00afd5a1:
        fld dword ptr [esp + 020h] // 00afd5a1
        push ecx // 00afd5a5
        fmul dword ptr [esp + 018h] // 00afd5a6
        mov ecx,dword ptr [esp + 020h] // 00afd5aa
        lea edx,[esp + 064h] // 00afd5ae
        lea eax,[esp + 04ch] // 00afd5b2
        fstp dword ptr [esp + 014h] // 00afd5b6
        fld dword ptr [esp + 014h] // 00afd5ba
        fst dword ptr [esi + 038h] // 00afd5be
        push eax // added binding
        mov eax,dword ptr [esp+0a8h] // added binding
        mov eax,dword ptr [eax+01ch] // added binding
        fdivr qword ptr [eax] // 00afd5c1
        pop eax
        fstp dword ptr [esi + 03ch] // 00afd5c7
        fild dword ptr [esp + 020h] // 00afd5ca
        fmul dword ptr [esp + 02ch] // 00afd5ce
        fstp dword ptr [esp + 014h] // 00afd5d2
        fld dword ptr [esp + 014h] // 00afd5d6
        fstp dword ptr [esp] // 00afd5da
        push ecx // 00afd5dd
        push edx // 00afd5de
        push eax // 00afd5df
        lea ecx,[esp + 064h] // 00afd5e0
        push ecx // 00afd5e4
        mov ecx,dword ptr [esp + 0c4h] // 00afd5e5
        mov edx,dword ptr [esp+0b4h] // added binding
        call interpolate_native_particle_record_00afdbf0 // 00afd5ec
        movss xmm0,dword ptr [esp + 054h] // 00afd5f1
        movss dword ptr [esi + 0ch],xmm0 // 00afd5f7
        fld dword ptr [esi + 0ch] // 00afd5fc
        movss xmm0,dword ptr [esp + 058h] // 00afd5ff
        movss dword ptr [esi + 010h],xmm0 // 00afd605
        movss xmm0,dword ptr [esp + 05ch] // 00afd60a
        movss dword ptr [esi + 014h],xmm0 // 00afd610
        fstp dword ptr [esi + 018h] // 00afd615
        fld dword ptr [esi + 010h] // 00afd618
        lea edx,[esp + 060h] // 00afd61b
        fstp dword ptr [esi + 01ch] // 00afd61f
        push edx // 00afd622
        fld dword ptr [esi + 014h] // 00afd623
        lea ecx,[esi + 060h] // 00afd626
        fstp dword ptr [esi + 020h] // 00afd629
        fld dword ptr [esi + 018h] // 00afd62c
        fstp dword ptr [esi] // 00afd62f
        fld dword ptr [esi + 01ch] // 00afd631
        fstp dword ptr [esi + 04h] // 00afd634
        fld dword ptr [esi + 020h] // 00afd637
        fstp dword ptr [esi + 08h] // 00afd63a
        movss xmm0,dword ptr [esp + 04ch] // 00afd63d
        movss dword ptr [esi + 048h],xmm0 // 00afd643
        movss xmm0,dword ptr [esp + 050h] // 00afd648
        movss dword ptr [esi + 04ch],xmm0 // 00afd64e
        movss xmm0,dword ptr [esp + 054h] // 00afd653
        movss dword ptr [esi + 050h],xmm0 // 00afd659
        call copy_native_camera_matrix_004134f0 // 00afd65e
        mov eax,dword ptr [edi + 02ch] // 00afd663
        movss xmm0,dword ptr [eax] // 00afd666
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+0ch] // added binding
        ucomiss xmm0,dword ptr [eax] // 00afd66a
        pop eax
        lahf // 00afd671
        test ah,044h // 00afd672
        movss dword ptr [esp + 034h],xmm0 // 00afd675
        jp l_00afd687 // 00afd67b
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+010h] // added binding
        movss xmm0,dword ptr [eax] // 00afd67d
        pop eax
        jmp l_00afd6b0 // 00afd685
    L_00afd687:
        xor ecx,ecx // 00afd687
        mov edx,dword ptr [esp+0a0h] // added binding
        call random_bridge // 00afd689
        mov dword ptr [esp + 010h],eax // 00afd68e
        fild dword ptr [esp + 010h] // 00afd692
        fmul dword ptr [esp + 034h] // 00afd696
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+014h] // added binding
        fmul qword ptr [eax] // 00afd69a
        pop eax
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+018h] // added binding
        fadd qword ptr [eax] // 00afd6a0
        pop eax
        fstp dword ptr [esp + 010h] // 00afd6a6
        movss xmm0,dword ptr [esp + 010h] // 00afd6aa
    L_00afd6b0:
        movss dword ptr [esi + 0a8h],xmm0 // 00afd6b0
        mov ecx,dword ptr [edi + 030h] // 00afd6b8
        movss xmm0,dword ptr [ecx] // 00afd6bb
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+0ch] // added binding
        ucomiss xmm0,dword ptr [eax] // 00afd6bf
        pop eax
        lahf // 00afd6c6
        test ah,044h // 00afd6c7
        movss dword ptr [esp + 030h],xmm0 // 00afd6ca
        jp l_00afd6dc // 00afd6d0
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+010h] // added binding
        movss xmm0,dword ptr [eax] // 00afd6d2
        pop eax
        jmp l_00afd705 // 00afd6da
    L_00afd6dc:
        xor ecx,ecx // 00afd6dc
        mov edx,dword ptr [esp+0a0h] // added binding
        call random_bridge // 00afd6de
        mov dword ptr [esp + 010h],eax // 00afd6e3
        fild dword ptr [esp + 010h] // 00afd6e7
        fmul dword ptr [esp + 030h] // 00afd6eb
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+014h] // added binding
        fmul qword ptr [eax] // 00afd6ef
        pop eax
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+018h] // added binding
        fadd qword ptr [eax] // 00afd6f5
        pop eax
        fstp dword ptr [esp + 010h] // 00afd6fb
        movss xmm0,dword ptr [esp + 010h] // 00afd6ff
    L_00afd705:
        movss dword ptr [esi + 0ach],xmm0 // 00afd705
        mov edx,dword ptr [edi + 038h] // 00afd70d
        movss xmm0,dword ptr [edx] // 00afd710
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+0ch] // added binding
        ucomiss xmm0,dword ptr [eax] // 00afd714
        pop eax
        lahf // 00afd71b
        test ah,044h // 00afd71c
        movss dword ptr [esp + 02ch],xmm0 // 00afd71f
        jp l_00afd731 // 00afd725
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+010h] // added binding
        movss xmm0,dword ptr [eax] // 00afd727
        pop eax
        jmp l_00afd75a // 00afd72f
    L_00afd731:
        xor ecx,ecx // 00afd731
        mov edx,dword ptr [esp+0a0h] // added binding
        call random_bridge // 00afd733
        mov dword ptr [esp + 010h],eax // 00afd738
        fild dword ptr [esp + 010h] // 00afd73c
        fmul dword ptr [esp + 02ch] // 00afd740
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+014h] // added binding
        fmul qword ptr [eax] // 00afd744
        pop eax
        push eax // added binding
        mov eax,dword ptr [esp+0a4h] // added binding
        mov eax,dword ptr [eax+018h] // added binding
        fadd qword ptr [eax] // 00afd74a
        pop eax
        fstp dword ptr [esp + 010h] // 00afd750
        movss xmm0,dword ptr [esp + 010h] // 00afd754
    L_00afd75a:
        mov eax,dword ptr [esp + 01ch] // 00afd75a
        mov ecx,01h // 00afd75e
        add dword ptr [esp + 024h],ecx // 00afd763
        add eax,ecx // 00afd767
        movss dword ptr [esi + 0b0h],xmm0 // 00afd769
        add dword ptr [ebx + 014h],ecx // 00afd771
        cmp eax,dword ptr [esp + 018h] // 00afd774
        mov dword ptr [esp + 01ch],eax // 00afd778
        jl l_00afd4c0 // 00afd77c
    L_00afd782:
        pop esi // 00afd782
        pop edi // 00afd783
        pop ebp // 00afd784
    L_00afd785:
        mov eax,dword ptr [esp + 018h] // 00afd785
        pop ebx // 00afd789
        add esp,094h // 00afd78a
        ret 018h // 00afd790
    }
}
} // namespace bsp
