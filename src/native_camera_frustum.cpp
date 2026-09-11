#include "bsp/native_camera_frustum.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera frustum requires MSVC Win32 x87/SSE assembly.
#endif

namespace bsp {

// Full original 00b650b0; only explicit CRT/context integer plumbing differs.
__declspec(naked) void* __fastcall normalize_native_camera_plane_00b650b0(void*, const NativeCameraFrustumContext*) {
    __asm {
        push ecx // 00b650b0
        push esi // 00b650b1
        mov esi, ecx // 00b650b2
        call camera_vector_length_00419440 // 00b650b4
        fstp dword ptr [esp + 4] // 00b650b9
        fldz  // 00b650bd
        fld dword ptr [esp + 4] // 00b650bf
        fcomi st(0), st(1) // 00b650c3
        fstp st(1) // 00b650c5
        jbe L_00b650d3 // 00b650c7
        fld1  // 00b650c9
        fdivrp st(1), st(0) // 00b650cb
        fstp dword ptr [esp + 4] // 00b650cd
        jmp L_00b650de // 00b650d1
    L_00b650d3:
        xorps xmm0, xmm0 // 00b650d3
        fstp st(0) // 00b650d6
        movss dword ptr [esp + 4], xmm0 // 00b650d8
    L_00b650de:
        fld dword ptr [esi] // 00b650de
        mov eax, esi // 00b650e0
        fld dword ptr [esp + 4] // 00b650e2
        fld st(0) // 00b650e6
        fmulp st(2), st(0) // 00b650e8
        fxch st(1) // 00b650ea
        fstp dword ptr [esi] // 00b650ec
        fld dword ptr [esi + 4] // 00b650ee
        fmul st(0), st(1) // 00b650f1
        fstp dword ptr [esi + 4] // 00b650f3
        fld st(0) // 00b650f6
        fmul dword ptr [esi + 8] // 00b650f8
        fstp dword ptr [esi + 8] // 00b650fb
        fmul dword ptr [esi + 0ch] // 00b650fe
        fstp dword ptr [esi + 0ch] // 00b65101
        pop esi // 00b65104
        pop ecx // 00b65105
        ret  // 00b65106
    }
}

// Full original 00b653f0; only explicit CRT/context integer plumbing differs.
__declspec(naked) void* __fastcall extract_native_camera_frustum_00b653f0(void*, const NativeCameraFrustumContext*, const void*) {
    __asm {
        push edx // preserve concrete context outside original local frame
        mov eax, dword ptr [esp + 8] // 00b653f0
        fld dword ptr [eax] // 00b653f4
        push ebx // 00b653f6
        fadd dword ptr [eax + 0ch] // 00b653f7
        push ebp // 00b653fa
        push esi // 00b653fb
        mov esi, ecx // 00b653fc
        fstp dword ptr [esi] // 00b653fe
        push edi // 00b65400
        fld dword ptr [eax + 01ch] // 00b65401
        lea edi, [esi + 010h] // 00b65404
        fadd dword ptr [eax + 010h] // 00b65407
        lea ebx, [esi + 020h] // 00b6540a
        lea ebp, [esi + 030h] // 00b6540d
        fstp dword ptr [esi + 4] // 00b65410
        fld dword ptr [eax + 02ch] // 00b65413
        fadd dword ptr [eax + 020h] // 00b65416
        fstp dword ptr [esi + 8] // 00b65419
        fld dword ptr [eax + 03ch] // 00b6541c
        fadd dword ptr [eax + 030h] // 00b6541f
        fstp dword ptr [esi + 0ch] // 00b65422
        fld dword ptr [eax + 0ch] // 00b65425
        fsub dword ptr [eax] // 00b65428
        fstp dword ptr [edi] // 00b6542a
        fld dword ptr [eax + 01ch] // 00b6542c
        fsub dword ptr [eax + 010h] // 00b6542f
        fstp dword ptr [esi + 014h] // 00b65432
        fld dword ptr [eax + 02ch] // 00b65435
        fsub dword ptr [eax + 020h] // 00b65438
        fstp dword ptr [esi + 018h] // 00b6543b
        fld dword ptr [eax + 03ch] // 00b6543e
        fsub dword ptr [eax + 030h] // 00b65441
        fstp dword ptr [esi + 01ch] // 00b65444
        fld dword ptr [eax + 0ch] // 00b65447
        fsub dword ptr [eax + 4] // 00b6544a
        fstp dword ptr [ebx] // 00b6544d
        fld dword ptr [eax + 01ch] // 00b6544f
        fsub dword ptr [eax + 014h] // 00b65452
        fstp dword ptr [esi + 024h] // 00b65455
        fld dword ptr [eax + 02ch] // 00b65458
        fsub dword ptr [eax + 024h] // 00b6545b
        fstp dword ptr [esi + 028h] // 00b6545e
        fld dword ptr [eax + 03ch] // 00b65461
        fsub dword ptr [eax + 034h] // 00b65464
        fstp dword ptr [esi + 02ch] // 00b65467
        fld dword ptr [eax + 4] // 00b6546a
        fadd dword ptr [eax + 0ch] // 00b6546d
        fstp dword ptr [ebp] // 00b65470
        fld dword ptr [eax + 01ch] // 00b65473
        fadd dword ptr [eax + 014h] // 00b65476
        fstp dword ptr [esi + 034h] // 00b65479
        fld dword ptr [eax + 02ch] // 00b6547c
        fadd dword ptr [eax + 024h] // 00b6547f
        fstp dword ptr [esi + 038h] // 00b65482
        fld dword ptr [eax + 03ch] // 00b65485
        fadd dword ptr [eax + 034h] // 00b65488
        fstp dword ptr [esi + 03ch] // 00b6548b
        fld dword ptr [eax + 8] // 00b6548e
        fadd st(0), st(0) // 00b65491
        fstp dword ptr [esi + 040h] // 00b65493
        fld dword ptr [eax + 018h] // 00b65496
        fadd st(0), st(0) // 00b65499
        fstp dword ptr [esi + 044h] // 00b6549b
        fld dword ptr [eax + 028h] // 00b6549e
        fadd st(0), st(0) // 00b654a1
        fstp dword ptr [esi + 048h] // 00b654a3
        fld dword ptr [eax + 038h] // 00b654a6
        fadd st(0), st(0) // 00b654a9
        fstp dword ptr [esi + 04ch] // 00b654ab
        fld dword ptr [eax + 0ch] // 00b654ae
        fsub dword ptr [eax + 8] // 00b654b1
        fstp dword ptr [esi + 050h] // 00b654b4
        fld dword ptr [eax + 01ch] // 00b654b7
        fsub dword ptr [eax + 018h] // 00b654ba
        fstp dword ptr [esi + 054h] // 00b654bd
        fld dword ptr [eax + 02ch] // 00b654c0
        fsub dword ptr [eax + 028h] // 00b654c3
        fstp dword ptr [esi + 058h] // 00b654c6
        fld dword ptr [eax + 03ch] // 00b654c9
        fsub dword ptr [eax + 038h] // 00b654cc
        fstp dword ptr [esi + 05ch] // 00b654cf
        mov edx, dword ptr [esp + 10h]
        mov edx, dword ptr [edx + 8]
        movss xmm0, dword ptr [edx] // 00b654d2
        movaps xmm1, xmm0 // 00b654da
        subss xmm1, dword ptr [esi + 0ch] // 00b654dd
        movss dword ptr [esi + 0ch], xmm1 // 00b654e2
        movaps xmm1, xmm0 // 00b654e7
        subss xmm1, dword ptr [esi + 01ch] // 00b654ea
        movss dword ptr [esi + 01ch], xmm1 // 00b654ef
        movaps xmm1, xmm0 // 00b654f4
        subss xmm1, dword ptr [esi + 02ch] // 00b654f7
        movss dword ptr [esi + 02ch], xmm1 // 00b654fc
        movaps xmm1, xmm0 // 00b65501
        subss xmm1, dword ptr [esi + 03ch] // 00b65504
        movss dword ptr [esi + 03ch], xmm1 // 00b65509
        movaps xmm1, xmm0 // 00b6550e
        subss xmm1, dword ptr [esi + 04ch] // 00b65511
        subss xmm0, dword ptr [esi + 05ch] // 00b65516
        movss dword ptr [esi + 04ch], xmm1 // 00b6551b
        movss dword ptr [esi + 05ch], xmm0 // 00b65520
        mov edx, dword ptr [esp + 10h] // current saved context
        call normalize_native_camera_plane_00b650b0 // 00b65525
        mov ecx, edi // 00b6552a
        mov edx, dword ptr [esp + 10h] // current saved context
        call normalize_native_camera_plane_00b650b0 // 00b6552c
        mov ecx, ebx // 00b65531
        mov edx, dword ptr [esp + 10h] // current saved context
        call normalize_native_camera_plane_00b650b0 // 00b65533
        mov ecx, ebp // 00b65538
        mov edx, dword ptr [esp + 10h] // current saved context
        call normalize_native_camera_plane_00b650b0 // 00b6553a
        lea ecx, [esi + 040h] // 00b6553f
        mov edx, dword ptr [esp + 10h] // current saved context
        call normalize_native_camera_plane_00b650b0 // 00b65542
        lea ecx, [esi + 050h] // 00b65547
        mov edx, dword ptr [esp + 10h] // current saved context
        call normalize_native_camera_plane_00b650b0 // 00b6554a
        pop edi // 00b6554f
        mov eax, esi // 00b65550
        pop esi // 00b65552
        pop ebp // 00b65553
        pop ebx // 00b65554
        pop edx // discard added context word before original RET4
        ret 4 // 00b65555
    }
}

// Full original 00b658e0, including original load/store/flags order. No helper.
__declspec(naked) void __fastcall assign_native_camera_frustum_planes_00b658e0(void*, void*, const void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4] // 00b658e0
        fld dword ptr [eax] // 00b658e4
        mov edx, dword ptr [esp + 8] // 00b658e6
        fstp dword ptr [ecx] // 00b658ea
        fld dword ptr [eax + 4] // 00b658ec
        fstp dword ptr [ecx + 4] // 00b658ef
        fld dword ptr [eax + 8] // 00b658f2
        fstp dword ptr [ecx + 8] // 00b658f5
        fld dword ptr [eax + 0ch] // 00b658f8
        fstp dword ptr [ecx + 0ch] // 00b658fb
        mov dword ptr [ecx + 010h], edx // 00b658fe
        fld dword ptr [eax + 010h] // 00b65901
        fstp dword ptr [ecx + 014h] // 00b65904
        fld dword ptr [eax + 014h] // 00b65907
        fstp dword ptr [ecx + 018h] // 00b6590a
        fld dword ptr [eax + 018h] // 00b6590d
        fstp dword ptr [ecx + 01ch] // 00b65910
        fld dword ptr [eax + 01ch] // 00b65913
        fstp dword ptr [ecx + 020h] // 00b65916
        mov dword ptr [ecx + 024h], edx // 00b65919
        fld dword ptr [eax + 020h] // 00b6591c
        fstp dword ptr [ecx + 028h] // 00b6591f
        fld dword ptr [eax + 024h] // 00b65922
        fstp dword ptr [ecx + 02ch] // 00b65925
        fld dword ptr [eax + 028h] // 00b65928
        fstp dword ptr [ecx + 030h] // 00b6592b
        fld dword ptr [eax + 02ch] // 00b6592e
        fstp dword ptr [ecx + 034h] // 00b65931
        mov dword ptr [ecx + 038h], edx // 00b65934
        fld dword ptr [eax + 030h] // 00b65937
        fstp dword ptr [ecx + 03ch] // 00b6593a
        fld dword ptr [eax + 034h] // 00b6593d
        fstp dword ptr [ecx + 040h] // 00b65940
        fld dword ptr [eax + 038h] // 00b65943
        fstp dword ptr [ecx + 044h] // 00b65946
        fld dword ptr [eax + 03ch] // 00b65949
        fstp dword ptr [ecx + 048h] // 00b6594c
        mov dword ptr [ecx + 04ch], edx // 00b6594f
        fld dword ptr [eax + 040h] // 00b65952
        fstp dword ptr [ecx + 050h] // 00b65955
        fld dword ptr [eax + 044h] // 00b65958
        fstp dword ptr [ecx + 054h] // 00b6595b
        fld dword ptr [eax + 048h] // 00b6595e
        fstp dword ptr [ecx + 058h] // 00b65961
        fld dword ptr [eax + 04ch] // 00b65964
        fstp dword ptr [ecx + 05ch] // 00b65967
        mov dword ptr [ecx + 060h], edx // 00b6596a
        fld dword ptr [eax + 050h] // 00b6596d
        fstp dword ptr [ecx + 064h] // 00b65970
        fld dword ptr [eax + 054h] // 00b65973
        fstp dword ptr [ecx + 068h] // 00b65976
        fld dword ptr [eax + 058h] // 00b65979
        fstp dword ptr [ecx + 06ch] // 00b6597c
        fld dword ptr [eax + 05ch] // 00b6597f
        fstp dword ptr [ecx + 070h] // 00b65982
        mov dword ptr [ecx + 074h], edx // 00b65985
        ret 8 // 00b65988
    }
}
} // namespace bsp
