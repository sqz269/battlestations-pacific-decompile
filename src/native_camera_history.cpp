#include "bsp/native_camera_history.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_projection.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera history requires MSVC Win32 x87.
#endif
namespace bsp {
static_assert(offsetof(NativeCameraHistoryContext, crt)==0);
static_assert(offsetof(NativeCameraHistoryContext, reset_distance_00ce3930)==4);
static_assert(offsetof(NativeCameraHistoryContext, one_00d7a24c)==8);

// Complete original 005803E0. Keep original public-slot overwrite and every
// x87 spill; no float-vector snapshot or C++ arithmetic substitution.
__declspec(naked) void* __fastcall lerp_native_float3_005803e0(
    void*, const void*, const void*, const void*) {
    __asm {
        sub esp,01ch // 005803e0
        fld dword ptr [edx] // 005803e3
        mov eax,ecx // 005803e5
        fstp dword ptr [esp] // 005803e7
        mov ecx,dword ptr [esp + 020h] // 005803ea
        fld dword ptr [ecx] // 005803ee
        fld dword ptr [esp] // 005803f0
        fld st(0) // 005803f3
        fsubp st(2),st(0) // 005803f5
        fxch // 005803f7
        fstp dword ptr [esp + 04h] // 005803f9
        fld dword ptr [edx + 04h] // 005803fd
        fstp dword ptr [esp + 020h] // 00580400
        fld dword ptr [ecx + 04h] // 00580404
        fld dword ptr [esp + 020h] // 00580407
        fld st(0) // 0058040b
        fsubp st(2),st(0) // 0058040d
        fxch // 0058040f
        fstp dword ptr [esp + 08h] // 00580411
        fld dword ptr [edx + 08h] // 00580415
        fstp dword ptr [esp + 020h] // 00580418
        fld dword ptr [ecx + 08h] // 0058041c
        mov ecx,dword ptr [esp + 024h] // 0058041f
        fld dword ptr [esp + 020h] // 00580423
        fld st(0) // 00580427
        fsubp st(2),st(0) // 00580429
        fxch // 0058042b
        fstp dword ptr [esp + 0ch] // 0058042d
        fld dword ptr [ecx] // 00580431
        fstp dword ptr [esp + 020h] // 00580433
        fld dword ptr [esp + 04h] // 00580437
        fld dword ptr [esp + 020h] // 0058043b
        fld st(0) // 0058043f
        fmulp st(2), st(0) // 00580441
        fxch // 00580443
        fstp dword ptr [esp + 010h] // 00580445
        fld dword ptr [esp + 08h] // 00580449
        fmul st(0), st(1) // 0058044d
        fstp dword ptr [esp + 014h] // 0058044f
        fmul dword ptr [esp + 0ch] // 00580453
        fstp dword ptr [esp + 018h] // 00580457
        fld dword ptr [esp + 010h] // 0058045b
        faddp st(3),st(0) // 0058045f
        fxch st(2) // 00580461
        fstp dword ptr [esp + 04h] // 00580463
        fadd dword ptr [esp + 014h] // 00580467
        fstp dword ptr [esp + 08h] // 0058046b
        fadd dword ptr [esp + 018h] // 0058046f
        fstp dword ptr [esp + 0ch] // 00580473
        fld dword ptr [esp + 04h] // 00580477
        fstp dword ptr [eax] // 0058047b
        fld dword ptr [esp + 08h] // 0058047d
        fstp dword ptr [eax + 04h] // 00580481
        fld dword ptr [esp + 0ch] // 00580484
        fstp dword ptr [eax + 08h] // 00580488
        add esp,01ch // 0058048b
        ret 08h // 0058048e
    }
}

// Complete original B0DB00. EBP holds only the borrowed source context;
// the native inner search saves/restores EBP around its own endpoint pointer.
// Extra saved EBP shifts the two public arguments by4, while every native
// scratch offset is unchanged. Context reads add no FP operation or spill.
__declspec(naked) void __fastcall update_native_camera_history_00b0db00(
    void*, const NativeCameraHistoryContext*, std::uint32_t, void*) {
    __asm {
        push ebp
        mov ebp, edx
        sub esp, 017ch
        push esi // 00b0db06
        push edi // 00b0db07
        mov edi, dword ptr [esp + 0190h] // extra saved EBP shifts public camera only
        test byte ptr [edi + 05ch],02h // 00b0db0f
        mov esi,ecx // 00b0db13
        jnz loc_00b0db1e // 00b0db15
        mov ecx,edi // 00b0db17
        call refresh_native_camera_world_00b6db70
    loc_00b0db1e:
        fld dword ptr [edi + 0120h] // 00b0db1e
        lea ecx,[esp + 044h] // 00b0db24
        fstp dword ptr [esp + 01ch] // 00b0db28
        fld dword ptr [edi + 0124h] // 00b0db2c
        fstp dword ptr [esp + 020h] // 00b0db32
        fld dword ptr [edi + 0128h] // 00b0db36
        fstp dword ptr [esp + 024h] // 00b0db3c
        fld dword ptr [esi + 0288h] // 00b0db40
        fsub dword ptr [esp + 01ch] // 00b0db46
        fstp dword ptr [esp + 044h] // 00b0db4a
        fld dword ptr [esi + 028ch] // 00b0db4e
        fsub dword ptr [esp + 020h] // 00b0db54
        fstp dword ptr [esp + 048h] // 00b0db58
        fld dword ptr [esi + 0290h] // 00b0db5c
        fsub dword ptr [esp + 024h] // 00b0db62
        fstp dword ptr [esp + 04ch] // 00b0db66
        mov edx, dword ptr [ebp]
        call camera_vector_length_00419440
        push eax
        mov eax, dword ptr [ebp + 4]
        fld dword ptr [eax]
        pop eax
        fxch // 00b0db75
        fcomip st(0),st(1) // 00b0db77
        fstp st(0) // 00b0db79
        jbe loc_00b0db84 // 00b0db7b
        mov dword ptr [esi + 04h],00h // 00b0db7d
    loc_00b0db84:
        cmp dword ptr [esi + 04h],08h // 00b0db84
        jnz loc_00b0e0ff // 00b0db88
        fld dword ptr [esi + 050h] // 00b0db8e
        fstp dword ptr [esi + 08h] // 00b0db91
        fld dword ptr [esi + 054h] // 00b0db94
        fstp dword ptr [esi + 0ch] // 00b0db97
        fld dword ptr [esi + 058h] // 00b0db9a
        fstp dword ptr [esi + 010h] // 00b0db9d
        fld dword ptr [esi + 05ch] // 00b0dba0
        fstp dword ptr [esi + 014h] // 00b0dba3
        fld dword ptr [esi + 060h] // 00b0dba6
        fstp dword ptr [esi + 018h] // 00b0dba9
        fld dword ptr [esi + 064h] // 00b0dbac
        fstp dword ptr [esi + 01ch] // 00b0dbaf
        fld dword ptr [esi + 068h] // 00b0dbb2
        fstp dword ptr [esi + 020h] // 00b0dbb5
        fld dword ptr [esi + 06ch] // 00b0dbb8
        fstp dword ptr [esi + 024h] // 00b0dbbb
        fld dword ptr [esi + 070h] // 00b0dbbe
        fstp dword ptr [esi + 028h] // 00b0dbc1
        fld dword ptr [esi + 074h] // 00b0dbc4
        fstp dword ptr [esi + 02ch] // 00b0dbc7
        fld dword ptr [esi + 078h] // 00b0dbca
        fstp dword ptr [esi + 030h] // 00b0dbcd
        fld dword ptr [esi + 07ch] // 00b0dbd0
        fstp dword ptr [esi + 034h] // 00b0dbd3
        fld dword ptr [esi + 080h] // 00b0dbd6
        fstp dword ptr [esi + 038h] // 00b0dbdc
        fld dword ptr [esi + 084h] // 00b0dbdf
        fstp dword ptr [esi + 03ch] // 00b0dbe5
        fld dword ptr [esi + 088h] // 00b0dbe8
        fstp dword ptr [esi + 040h] // 00b0dbee
        fld dword ptr [esi + 08ch] // 00b0dbf1
        fstp dword ptr [esi + 044h] // 00b0dbf7
        fld dword ptr [esi + 090h] // 00b0dbfa
        fstp dword ptr [esi + 048h] // 00b0dc00
        fld dword ptr [esi + 094h] // 00b0dc03
        fstp dword ptr [esi + 04ch] // 00b0dc09
        fld dword ptr [esi + 098h] // 00b0dc0c
        fstp dword ptr [esi + 050h] // 00b0dc12
        fld dword ptr [esi + 09ch] // 00b0dc15
        fstp dword ptr [esi + 054h] // 00b0dc1b
        fld dword ptr [esi + 0a0h] // 00b0dc1e
        fstp dword ptr [esi + 058h] // 00b0dc24
        fld dword ptr [esi + 0a4h] // 00b0dc27
        fstp dword ptr [esi + 05ch] // 00b0dc2d
        fld dword ptr [esi + 0a8h] // 00b0dc30
        fstp dword ptr [esi + 060h] // 00b0dc36
        fld dword ptr [esi + 0ach] // 00b0dc39
        fstp dword ptr [esi + 064h] // 00b0dc3f
        fld dword ptr [esi + 0b0h] // 00b0dc42
        fstp dword ptr [esi + 068h] // 00b0dc48
        fld dword ptr [esi + 0b4h] // 00b0dc4b
        fstp dword ptr [esi + 06ch] // 00b0dc51
        fld dword ptr [esi + 0b8h] // 00b0dc54
        fstp dword ptr [esi + 070h] // 00b0dc5a
        fld dword ptr [esi + 0bch] // 00b0dc5d
        fstp dword ptr [esi + 074h] // 00b0dc63
        fld dword ptr [esi + 0c0h] // 00b0dc66
        fstp dword ptr [esi + 078h] // 00b0dc6c
        fld dword ptr [esi + 0c4h] // 00b0dc6f
        fstp dword ptr [esi + 07ch] // 00b0dc75
        fld dword ptr [esi + 0c8h] // 00b0dc78
        fstp dword ptr [esi + 080h] // 00b0dc7e
        fld dword ptr [esi + 0cch] // 00b0dc84
        fstp dword ptr [esi + 084h] // 00b0dc8a
        fld dword ptr [esi + 0d0h] // 00b0dc90
        fstp dword ptr [esi + 088h] // 00b0dc96
        fld dword ptr [esi + 0d4h] // 00b0dc9c
        fstp dword ptr [esi + 08ch] // 00b0dca2
        fld dword ptr [esi + 0d8h] // 00b0dca8
        fstp dword ptr [esi + 090h] // 00b0dcae
        fld dword ptr [esi + 0dch] // 00b0dcb4
        fstp dword ptr [esi + 094h] // 00b0dcba
        fld dword ptr [esi + 0e0h] // 00b0dcc0
        fstp dword ptr [esi + 098h] // 00b0dcc6
        fld dword ptr [esi + 0e4h] // 00b0dccc
        fstp dword ptr [esi + 09ch] // 00b0dcd2
        fld dword ptr [esi + 0e8h] // 00b0dcd8
        fstp dword ptr [esi + 0a0h] // 00b0dcde
        fld dword ptr [esi + 0ech] // 00b0dce4
        fstp dword ptr [esi + 0a4h] // 00b0dcea
        fld dword ptr [esi + 0f0h] // 00b0dcf0
        fstp dword ptr [esi + 0a8h] // 00b0dcf6
        fld dword ptr [esi + 0f4h] // 00b0dcfc
        fstp dword ptr [esi + 0ach] // 00b0dd02
        fld dword ptr [esi + 0f8h] // 00b0dd08
        fstp dword ptr [esi + 0b0h] // 00b0dd0e
        fld dword ptr [esi + 0fch] // 00b0dd14
        fstp dword ptr [esi + 0b4h] // 00b0dd1a
        fld dword ptr [esi + 0100h] // 00b0dd20
        fstp dword ptr [esi + 0b8h] // 00b0dd26
        fld dword ptr [esi + 0104h] // 00b0dd2c
        fstp dword ptr [esi + 0bch] // 00b0dd32
        fld dword ptr [esi + 0108h] // 00b0dd38
        fstp dword ptr [esi + 0c0h] // 00b0dd3e
        fld dword ptr [esi + 010ch] // 00b0dd44
        fstp dword ptr [esi + 0c4h] // 00b0dd4a
        fld dword ptr [esi + 0110h] // 00b0dd50
        fstp dword ptr [esi + 0c8h] // 00b0dd56
        fld dword ptr [esi + 0114h] // 00b0dd5c
        fstp dword ptr [esi + 0cch] // 00b0dd62
        fld dword ptr [esi + 0118h] // 00b0dd68
        fstp dword ptr [esi + 0d0h] // 00b0dd6e
        fld dword ptr [esi + 011ch] // 00b0dd74
        fstp dword ptr [esi + 0d4h] // 00b0dd7a
        fld dword ptr [esi + 0120h] // 00b0dd80
        fstp dword ptr [esi + 0d8h] // 00b0dd86
        fld dword ptr [esi + 0124h] // 00b0dd8c
        fstp dword ptr [esi + 0dch] // 00b0dd92
        fld dword ptr [esi + 0128h] // 00b0dd98
        fstp dword ptr [esi + 0e0h] // 00b0dd9e
        fld dword ptr [esi + 012ch] // 00b0dda4
        fstp dword ptr [esi + 0e4h] // 00b0ddaa
        fld dword ptr [esi + 0130h] // 00b0ddb0
        fstp dword ptr [esi + 0e8h] // 00b0ddb6
        fld dword ptr [esi + 0134h] // 00b0ddbc
        fstp dword ptr [esi + 0ech] // 00b0ddc2
        fld dword ptr [esi + 0138h] // 00b0ddc8
        fstp dword ptr [esi + 0f0h] // 00b0ddce
        fld dword ptr [esi + 013ch] // 00b0ddd4
        fstp dword ptr [esi + 0f4h] // 00b0ddda
        fld dword ptr [esi + 0140h] // 00b0dde0
        fstp dword ptr [esi + 0f8h] // 00b0dde6
        fld dword ptr [esi + 0144h] // 00b0ddec
        fstp dword ptr [esi + 0fch] // 00b0ddf2
        fld dword ptr [esi + 0148h] // 00b0ddf8
        fstp dword ptr [esi + 0100h] // 00b0ddfe
        fld dword ptr [esi + 014ch] // 00b0de04
        fstp dword ptr [esi + 0104h] // 00b0de0a
        fld dword ptr [esi + 0150h] // 00b0de10
        fstp dword ptr [esi + 0108h] // 00b0de16
        fld dword ptr [esi + 0154h] // 00b0de1c
        fstp dword ptr [esi + 010ch] // 00b0de22
        fld dword ptr [esi + 0158h] // 00b0de28
        fstp dword ptr [esi + 0110h] // 00b0de2e
        fld dword ptr [esi + 015ch] // 00b0de34
        fstp dword ptr [esi + 0114h] // 00b0de3a
        fld dword ptr [esi + 0160h] // 00b0de40
        fstp dword ptr [esi + 0118h] // 00b0de46
        fld dword ptr [esi + 0164h] // 00b0de4c
        fstp dword ptr [esi + 011ch] // 00b0de52
        fld dword ptr [esi + 0168h] // 00b0de58
        fstp dword ptr [esi + 0120h] // 00b0de5e
        fld dword ptr [esi + 016ch] // 00b0de64
        fstp dword ptr [esi + 0124h] // 00b0de6a
        fld dword ptr [esi + 0170h] // 00b0de70
        fstp dword ptr [esi + 0128h] // 00b0de76
        fld dword ptr [esi + 0174h] // 00b0de7c
        fstp dword ptr [esi + 012ch] // 00b0de82
        fld dword ptr [esi + 0178h] // 00b0de88
        fstp dword ptr [esi + 0130h] // 00b0de8e
        fld dword ptr [esi + 017ch] // 00b0de94
        fstp dword ptr [esi + 0134h] // 00b0de9a
        fld dword ptr [esi + 0180h] // 00b0dea0
        fstp dword ptr [esi + 0138h] // 00b0dea6
        fld dword ptr [esi + 0184h] // 00b0deac
        fstp dword ptr [esi + 013ch] // 00b0deb2
        fld dword ptr [esi + 0188h] // 00b0deb8
        fstp dword ptr [esi + 0140h] // 00b0debe
        fld dword ptr [esi + 018ch] // 00b0dec4
        fstp dword ptr [esi + 0144h] // 00b0deca
        fld dword ptr [esi + 0190h] // 00b0ded0
        fstp dword ptr [esi + 0148h] // 00b0ded6
        fld dword ptr [esi + 0194h] // 00b0dedc
        fstp dword ptr [esi + 014ch] // 00b0dee2
        fld dword ptr [esi + 0198h] // 00b0dee8
        fstp dword ptr [esi + 0150h] // 00b0deee
        fld dword ptr [esi + 019ch] // 00b0def4
        fstp dword ptr [esi + 0154h] // 00b0defa
        fld dword ptr [esi + 01a0h] // 00b0df00
        fstp dword ptr [esi + 0158h] // 00b0df06
        fld dword ptr [esi + 01a4h] // 00b0df0c
        fstp dword ptr [esi + 015ch] // 00b0df12
        fld dword ptr [esi + 01a8h] // 00b0df18
        fstp dword ptr [esi + 0160h] // 00b0df1e
        fld dword ptr [esi + 01ach] // 00b0df24
        fstp dword ptr [esi + 0164h] // 00b0df2a
        fld dword ptr [esi + 01b0h] // 00b0df30
        fstp dword ptr [esi + 0168h] // 00b0df36
        fld dword ptr [esi + 01b4h] // 00b0df3c
        fstp dword ptr [esi + 016ch] // 00b0df42
        fld dword ptr [esi + 01b8h] // 00b0df48
        fstp dword ptr [esi + 0170h] // 00b0df4e
        fld dword ptr [esi + 01bch] // 00b0df54
        fstp dword ptr [esi + 0174h] // 00b0df5a
        fld dword ptr [esi + 01c0h] // 00b0df60
        fstp dword ptr [esi + 0178h] // 00b0df66
        fld dword ptr [esi + 01c4h] // 00b0df6c
        fstp dword ptr [esi + 017ch] // 00b0df72
        fld dword ptr [esi + 01c8h] // 00b0df78
        fstp dword ptr [esi + 0180h] // 00b0df7e
        fld dword ptr [esi + 01cch] // 00b0df84
        fstp dword ptr [esi + 0184h] // 00b0df8a
        fld dword ptr [esi + 01d0h] // 00b0df90
        fstp dword ptr [esi + 0188h] // 00b0df96
        fld dword ptr [esi + 01d4h] // 00b0df9c
        fstp dword ptr [esi + 018ch] // 00b0dfa2
        fld dword ptr [esi + 01d8h] // 00b0dfa8
        fstp dword ptr [esi + 0190h] // 00b0dfae
        fld dword ptr [esi + 01dch] // 00b0dfb4
        fstp dword ptr [esi + 0194h] // 00b0dfba
        fld dword ptr [esi + 01e0h] // 00b0dfc0
        fstp dword ptr [esi + 0198h] // 00b0dfc6
        fld dword ptr [esi + 01e4h] // 00b0dfcc
        fstp dword ptr [esi + 019ch] // 00b0dfd2
        fld dword ptr [esi + 01e8h] // 00b0dfd8
        fstp dword ptr [esi + 01a0h] // 00b0dfde
        fld dword ptr [esi + 01ech] // 00b0dfe4
        fstp dword ptr [esi + 01a4h] // 00b0dfea
        fld dword ptr [esi + 01f0h] // 00b0dff0
        fstp dword ptr [esi + 01a8h] // 00b0dff6
        fld dword ptr [esi + 01f4h] // 00b0dffc
        fstp dword ptr [esi + 01ach] // 00b0e002
        fld dword ptr [esi + 01f8h] // 00b0e008
        fstp dword ptr [esi + 01b0h] // 00b0e00e
        fld dword ptr [esi + 01fch] // 00b0e014
        fstp dword ptr [esi + 01b4h] // 00b0e01a
        fld dword ptr [esi + 0200h] // 00b0e020
        fstp dword ptr [esi + 01b8h] // 00b0e026
        fld dword ptr [esi + 0204h] // 00b0e02c
        fstp dword ptr [esi + 01bch] // 00b0e032
        fld dword ptr [esi + 0208h] // 00b0e038
        fstp dword ptr [esi + 01c0h] // 00b0e03e
        fld dword ptr [esi + 020ch] // 00b0e044
        fstp dword ptr [esi + 01c4h] // 00b0e04a
        fld dword ptr [esi + 0210h] // 00b0e050
        fstp dword ptr [esi + 01c8h] // 00b0e056
        fld dword ptr [esi + 0214h] // 00b0e05c
        fstp dword ptr [esi + 01cch] // 00b0e062
        fld dword ptr [esi + 0218h] // 00b0e068
        fstp dword ptr [esi + 01d0h] // 00b0e06e
        fld dword ptr [esi + 021ch] // 00b0e074
        fstp dword ptr [esi + 01d4h] // 00b0e07a
        fld dword ptr [esi + 0220h] // 00b0e080
        fstp dword ptr [esi + 01d8h] // 00b0e086
        fld dword ptr [esi + 0224h] // 00b0e08c
        fstp dword ptr [esi + 01dch] // 00b0e092
        fld dword ptr [esi + 0228h] // 00b0e098
        fstp dword ptr [esi + 01e0h] // 00b0e09e
        fld dword ptr [esi + 022ch] // 00b0e0a4
        fstp dword ptr [esi + 01e4h] // 00b0e0aa
        fld dword ptr [esi + 0230h] // 00b0e0b0
        fstp dword ptr [esi + 01e8h] // 00b0e0b6
        fld dword ptr [esi + 0234h] // 00b0e0bc
        fstp dword ptr [esi + 01ech] // 00b0e0c2
        fld dword ptr [esi + 0238h] // 00b0e0c8
        fstp dword ptr [esi + 01f0h] // 00b0e0ce
        fld dword ptr [esi + 023ch] // 00b0e0d4
        fstp dword ptr [esi + 01f4h] // 00b0e0da
        fld dword ptr [esi + 0240h] // 00b0e0e0
        fstp dword ptr [esi + 01f8h] // 00b0e0e6
        fld dword ptr [esi + 0244h] // 00b0e0ec
        fstp dword ptr [esi + 01fch] // 00b0e0f2
        mov dword ptr [esi + 04h],07h // 00b0e0f8
    loc_00b0e0ff:
        test byte ptr [edi + 05ch],02h // 00b0e0ff
        jnz loc_00b0e10c // 00b0e103
        mov ecx,edi // 00b0e105
        call refresh_native_camera_world_00b6db70
    loc_00b0e10c:
        mov eax,dword ptr [esi + 04h] // 00b0e10c
        fld dword ptr [edi + 0f0h] // 00b0e10f
        lea eax,[eax + eax*08h] // 00b0e115
        fstp dword ptr [esi + eax*08h + 08h] // 00b0e118
        lea eax,[esi + eax*08h + 08h] // 00b0e11c
        fld dword ptr [edi + 0f4h] // 00b0e120
        movss xmm0, dword ptr [esp + 018ch] // original time slot +188
        fstp dword ptr [eax + 04h] // 00b0e12f
        fld dword ptr [edi + 0f8h] // 00b0e132
        fstp dword ptr [eax + 08h] // 00b0e138
        fld dword ptr [edi + 0fch] // 00b0e13b
        fstp dword ptr [eax + 0ch] // 00b0e141
        fld dword ptr [edi + 0100h] // 00b0e144
        fstp dword ptr [eax + 010h] // 00b0e14a
        fld dword ptr [edi + 0104h] // 00b0e14d
        fstp dword ptr [eax + 014h] // 00b0e153
        fld dword ptr [edi + 0108h] // 00b0e156
        fstp dword ptr [eax + 018h] // 00b0e15c
        fld dword ptr [edi + 010ch] // 00b0e15f
        fstp dword ptr [eax + 01ch] // 00b0e165
        fld dword ptr [edi + 0110h] // 00b0e168
        fstp dword ptr [eax + 020h] // 00b0e16e
        fld dword ptr [edi + 0114h] // 00b0e171
        fstp dword ptr [eax + 024h] // 00b0e177
        fld dword ptr [edi + 0118h] // 00b0e17a
        fstp dword ptr [eax + 028h] // 00b0e180
        fld dword ptr [edi + 011ch] // 00b0e183
        fstp dword ptr [eax + 02ch] // 00b0e189
        fld dword ptr [edi + 0120h] // 00b0e18c
        fstp dword ptr [eax + 030h] // 00b0e192
        fld dword ptr [edi + 0124h] // 00b0e195
        fstp dword ptr [eax + 034h] // 00b0e19b
        fld dword ptr [edi + 0128h] // 00b0e19e
        fstp dword ptr [eax + 038h] // 00b0e1a4
        fld dword ptr [edi + 012ch] // 00b0e1a7
        fstp dword ptr [eax + 03ch] // 00b0e1ad
        mov eax,dword ptr [esi + 04h] // 00b0e1b0
        fld dword ptr [edi + 01c4h] // 00b0e1b3
        add eax,01h // 00b0e1b9
        lea ecx,[eax + eax*08h] // 00b0e1bc
        fstp dword ptr [esi + ecx*08h] // 00b0e1bf
        mov eax,dword ptr [esi + 04h] // 00b0e1c2
        fld dword ptr [esp + 018ch]
        lea edx,[eax + eax*08h] // 00b0e1cc
        movss dword ptr [esi + edx*08h + 04ch],xmm0 // 00b0e1cf
        fsub dword ptr [esi] // 00b0e1d5
        mov ecx,dword ptr [esi + 04h] // 00b0e1d7
        test ecx,ecx // 00b0e1da
        fstp dword ptr [esp + 0ch] // 00b0e1dc
        lea eax,[ecx + 01h] // 00b0e1e0
        fld dword ptr [esp + 0ch] // 00b0e1e3
        mov edx,ecx // 00b0e1e7
        mov dword ptr [esi + 04h],eax // 00b0e1e9
        jbe loc_00b0e226 // 00b0e1ec
        push ebx // 00b0e1ee
        sub edx,01h // 00b0e1ef
        lea eax,[edx + edx*08h] // 00b0e1f2
        push ebp // 00b0e1f5
        lea ebp,[esi + eax*08h + 04ch] // 00b0e1f6
        lea eax,[ecx + ecx*08h] // 00b0e1fa
        lea ebx,[esi + eax*08h + 04ch] // 00b0e1fd
    loc_00b0e201:
        fld dword ptr [ebx] // 00b0e201
        fxch // 00b0e203
        fcomi st(0),st(1) // 00b0e205
        fstp st(1) // 00b0e207
        jbe loc_00b0e224 // 00b0e209
        fld dword ptr [ebp] // 00b0e20b
        fcomip st(0),st(1) // 00b0e20e
        jc loc_00b0e224 // 00b0e210
        test edx,edx // 00b0e212
        jbe loc_00b0e224 // 00b0e214
        sub edx,01h // 00b0e216
        sub ebp,048h // 00b0e219
        sub ecx,01h // 00b0e21c
        sub ebx,048h // 00b0e21f
        jmp loc_00b0e201 // 00b0e222
    loc_00b0e224:
        pop ebp // 00b0e224
        pop ebx // 00b0e225
    loc_00b0e226:
        lea edx,[edx + edx*08h] // 00b0e226
        fld dword ptr [esi + edx*08h + 04ch] // 00b0e229
        lea edx,[esi + edx*08h] // 00b0e22d
        fstp dword ptr [esp + 0ch] // 00b0e230
        lea eax,[ecx + ecx*08h] // 00b0e234
        fld dword ptr [esp + 0ch] // 00b0e237
        lea ecx,[esi + eax*08h] // 00b0e23b
        fld st(0) // 00b0e23e
        push eax
        mov eax, dword ptr [ebp + 8]
        movss xmm1,dword ptr [eax]
        pop eax
        fsubp st(2),st(0) // 00b0e248
        fsubr dword ptr [ecx + 04ch] // 00b0e24a
        fdivp st(1), st(0) // 00b0e24d
        fstp dword ptr [esp + 018h] // 00b0e24f
        fld dword ptr [esp + 018h] // 00b0e253
        fldz // 00b0e257
        fcomip st(0),st(1) // 00b0e259
        fstp st(0) // 00b0e25b
        jbe loc_00b0e40e // 00b0e25d
        xorps xmm0,xmm0 // 00b0e263
    loc_00b0e266:
        movss dword ptr [esp + 018h],xmm0 // 00b0e266
    loc_00b0e26c:
        movss xmm0,dword ptr [edx + 038h] // 00b0e26c
        movss dword ptr [esp + 01ch],xmm0 // 00b0e271
        movss xmm0,dword ptr [edx + 03ch] // 00b0e277
        movss dword ptr [esp + 020h],xmm0 // 00b0e27c
        movss xmm0,dword ptr [edx + 040h] // 00b0e282
        movss dword ptr [esp + 024h],xmm0 // 00b0e287
        movss xmm0,dword ptr [ecx + 038h] // 00b0e28d
        movss dword ptr [esp + 044h],xmm0 // 00b0e292
        movss xmm0,dword ptr [ecx + 03ch] // 00b0e298
        movss dword ptr [esp + 048h],xmm0 // 00b0e29d
        movss xmm0,dword ptr [ecx + 040h] // 00b0e2a3
        movss dword ptr [esp + 04ch],xmm0 // 00b0e2a8
        movss xmm0,dword ptr [edx + 010h] // 00b0e2ae
        movss dword ptr [esp + 038h],xmm0 // 00b0e2b3
        movss xmm0,dword ptr [edx + 020h] // 00b0e2b9
        movss dword ptr [esp + 03ch],xmm0 // 00b0e2be
        movss xmm0,dword ptr [edx + 030h] // 00b0e2c4
        movss dword ptr [esp + 040h],xmm0 // 00b0e2c9
        movss xmm0,dword ptr [ecx + 010h] // 00b0e2cf
        movss dword ptr [esp + 028h],xmm0 // 00b0e2d4
        movss xmm0,dword ptr [ecx + 020h] // 00b0e2da
        movss dword ptr [esp + 02ch],xmm0 // 00b0e2df
        movss xmm0,dword ptr [ecx + 030h] // 00b0e2e5
        movss dword ptr [esp + 030h],xmm0 // 00b0e2ea
        movss xmm0,dword ptr [edx + 0ch] // 00b0e2f0
        movss dword ptr [esp + 06ch],xmm0 // 00b0e2f5
        movss xmm0,dword ptr [edx + 01ch] // 00b0e2fb
        movss dword ptr [esp + 070h],xmm0 // 00b0e300
        movss xmm0,dword ptr [edx + 02ch] // 00b0e306
        movss dword ptr [esp + 074h],xmm0 // 00b0e30b
        movss xmm0,dword ptr [ecx + 0ch] // 00b0e311
        movss dword ptr [esp + 060h],xmm0 // 00b0e316
        movss xmm0,dword ptr [ecx + 01ch] // 00b0e31c
        movss dword ptr [esp + 064h],xmm0 // 00b0e321
        movss xmm0,dword ptr [ecx + 02ch] // 00b0e327
        movss dword ptr [esp + 068h],xmm0 // 00b0e32c
        movss xmm0,dword ptr [edx + 048h] // 00b0e332
        movss dword ptr [esp + 058h],xmm0 // 00b0e337
        movss xmm0,dword ptr [ecx + 048h] // 00b0e33d
        lea ecx,[esp + 018h] // 00b0e342
        push ecx // 00b0e346
        lea edx,[esp + 048h] // 00b0e347
        push edx // 00b0e34b
        lea edx,[esp + 024h] // 00b0e34c
        lea ecx,[esp + 080h] // 00b0e350
        movss dword ptr [esp + 064h],xmm0 // 00b0e357
        call lerp_native_float3_005803e0
        lea eax,[esp + 018h] // 00b0e362
        push eax // 00b0e366
        lea ecx,[esp + 02ch] // 00b0e367
        push ecx // 00b0e36b
        lea edx,[esp + 040h] // 00b0e36c
        lea ecx,[esp + 04ch] // 00b0e370
        call lerp_native_float3_005803e0
        movss xmm0,dword ptr [eax] // 00b0e379
        movss dword ptr [esp + 050h],xmm0 // 00b0e37d
        movss dword ptr [esp + 054h],xmm0 // 00b0e383
        movss xmm0,dword ptr [eax + 04h] // 00b0e389
        movss dword ptr [esp + 0ch],xmm0 // 00b0e38e
        fld dword ptr [esp + 0ch] // 00b0e394
        movss dword ptr [esp + 034h],xmm0 // 00b0e398
        movss xmm0,dword ptr [eax + 08h] // 00b0e39e
        fld dword ptr [esp + 050h] // 00b0e3a3
        movss dword ptr [esp + 014h],xmm0 // 00b0e3a7
        fld dword ptr [esp + 014h] // 00b0e3ad
        movss dword ptr [esp + 010h],xmm0 // 00b0e3b1
        fld st(1) // 00b0e3b7
        fmulp st(2), st(0) // 00b0e3b9
        fxch // 00b0e3bb
        fstp dword ptr [esp + 014h] // 00b0e3bd
        fld dword ptr [esp + 014h] // 00b0e3c1
        fld st(2) // 00b0e3c5
        fmulp st(3), st(0) // 00b0e3c7
        fxch st(2) // 00b0e3c9
        fstp dword ptr [esp + 014h] // 00b0e3cb
        fld dword ptr [esp + 014h] // 00b0e3cf
        faddp st(2),st(0) // 00b0e3d3
        fmul st(0), st(0) // 00b0e3d5
        fstp dword ptr [esp + 014h] // 00b0e3d7
        fadd dword ptr [esp + 014h] // 00b0e3db
        fstp dword ptr [esp + 014h] // 00b0e3df
        fld dword ptr [esp + 014h] // 00b0e3e3
        mov ecx, dword ptr [ebp]
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp + 014h] // 00b0e3ec
        fld dword ptr [esp + 014h] // 00b0e3f0
        fstp dword ptr [esp + 014h] // 00b0e3f4
        fldz // 00b0e3f8
        fld dword ptr [esp + 014h] // 00b0e3fa
        fcomi st(0),st(1) // 00b0e3fe
        fstp st(1) // 00b0e400
        jbe loc_00b0e428 // 00b0e402
        fld1 // 00b0e404
        fdivrp st(1), st(0) // 00b0e406
        fstp dword ptr [esp + 0ch] // 00b0e408
        jmp loc_00b0e433 // 00b0e40c
    loc_00b0e40e:
        movss xmm0,dword ptr [esp + 018h] // 00b0e40e
        comiss xmm0,xmm1 // 00b0e414
        jbe loc_00b0e266 // 00b0e417
        movss dword ptr [esp + 018h],xmm1 // 00b0e41d
        jmp loc_00b0e26c // 00b0e423
    loc_00b0e428:
        xorps xmm0,xmm0 // 00b0e428
        fstp st(0) // 00b0e42b
        movss dword ptr [esp + 0ch],xmm0 // 00b0e42d
    loc_00b0e433:
        fld dword ptr [esp + 0ch] // 00b0e433
        lea edx,[esp + 018h] // 00b0e437
        fld st(0) // 00b0e43b
        push edx // 00b0e43d
        fmul dword ptr [esp + 058h] // 00b0e43e
        lea eax,[esp + 064h] // 00b0e442
        push eax // 00b0e446
        lea edx,[esp + 074h] // 00b0e447
        fstp dword ptr [esp + 040h] // 00b0e44b
        lea ecx,[esp + 04ch] // 00b0e44f
        fld st(0) // 00b0e453
        fmul dword ptr [esp + 03ch] // 00b0e455
        fstp dword ptr [esp + 044h] // 00b0e459
        fmul dword ptr [esp + 018h] // 00b0e45d
        fstp dword ptr [esp + 048h] // 00b0e461
        call lerp_native_float3_005803e0
        movss xmm0,dword ptr [eax] // 00b0e46a
        movss dword ptr [esp + 034h],xmm0 // 00b0e46e
        movss dword ptr [esp + 014h],xmm0 // 00b0e474
        movss xmm0,dword ptr [eax + 04h] // 00b0e47a
        movss dword ptr [esp + 010h],xmm0 // 00b0e47f
        fld dword ptr [esp + 010h] // 00b0e485
        movss dword ptr [esp + 050h],xmm0 // 00b0e489
        movss xmm0,dword ptr [eax + 08h] // 00b0e48f
        fld dword ptr [esp + 034h] // 00b0e494
        movss dword ptr [esp + 054h],xmm0 // 00b0e498
        fld dword ptr [esp + 054h] // 00b0e49e
        movss dword ptr [esp + 08h],xmm0 // 00b0e4a2
        fld st(1) // 00b0e4a8
        fmulp st(2), st(0) // 00b0e4aa
        fxch // 00b0e4ac
        fstp dword ptr [esp + 010h] // 00b0e4ae
        fld dword ptr [esp + 010h] // 00b0e4b2
        fld st(2) // 00b0e4b6
        fmulp st(3), st(0) // 00b0e4b8
        fxch st(2) // 00b0e4ba
        fstp dword ptr [esp + 010h] // 00b0e4bc
        fld dword ptr [esp + 010h] // 00b0e4c0
        faddp st(2),st(0) // 00b0e4c4
        fmul st(0), st(0) // 00b0e4c6
        fstp dword ptr [esp + 010h] // 00b0e4c8
        fadd dword ptr [esp + 010h] // 00b0e4cc
        fstp dword ptr [esp + 010h] // 00b0e4d0
        fld dword ptr [esp + 010h] // 00b0e4d4
        mov ecx, dword ptr [ebp]
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp + 010h] // 00b0e4dd
        fld dword ptr [esp + 010h] // 00b0e4e1
        fstp dword ptr [esp + 010h] // 00b0e4e5
        fldz // 00b0e4e9
        fld dword ptr [esp + 010h] // 00b0e4eb
        fcomi st(0),st(1) // 00b0e4ef
        fstp st(1) // 00b0e4f1
        jbe loc_00b0e4ff // 00b0e4f3
        fld1 // 00b0e4f5
        fdivrp st(1), st(0) // 00b0e4f7
        fstp dword ptr [esp + 0ch] // 00b0e4f9
        jmp loc_00b0e50a // 00b0e4fd
    loc_00b0e4ff:
        xorps xmm0,xmm0 // 00b0e4ff
        fstp st(0) // 00b0e502
        movss dword ptr [esp + 0ch],xmm0 // 00b0e504
    loc_00b0e50a:
        fld dword ptr [esp + 0ch] // 00b0e50a
        fld st(0) // 00b0e50e
        fmul dword ptr [esp + 014h] // 00b0e510
        fstp dword ptr [esp + 01ch] // 00b0e514
        fld st(0) // 00b0e518
        fmul dword ptr [esp + 050h] // 00b0e51a
        fstp dword ptr [esp + 020h] // 00b0e51e
        fmul dword ptr [esp + 08h] // 00b0e522
        fstp dword ptr [esp + 024h] // 00b0e526
        fld dword ptr [esp + 040h] // 00b0e52a
        fld st(0) // 00b0e52e
        fld dword ptr [esp + 020h] // 00b0e530
        fld st(0) // 00b0e534
        fmulp st(2), st(0) // 00b0e536
        fld dword ptr [esp + 03ch] // 00b0e538
        fld st(0) // 00b0e53c
        fld dword ptr [esp + 024h] // 00b0e53e
        fld st(0) // 00b0e542
        fmulp st(2), st(0) // 00b0e544
        fxch st(4) // 00b0e546
        fsubrp st(1), st(0) // 00b0e548
        fstp dword ptr [esp + 028h] // 00b0e54a
        fld dword ptr [esp + 038h] // 00b0e54e
        fld st(0) // 00b0e552
        fmulp st(4), st(0) // 00b0e554
        fld dword ptr [esp + 01ch] // 00b0e556
        fld st(0) // 00b0e55a
        fmulp st(6), st(0) // 00b0e55c
        fxch st(4) // 00b0e55e
        fsubrp st(5),st(0) // 00b0e560
        fxch st(4) // 00b0e562
        fstp dword ptr [esp + 02ch] // 00b0e564
        fmulp st(2), st(0) // 00b0e568
        fmulp st(2), st(0) // 00b0e56a
        fsubrp st(1), st(0) // 00b0e56c
        fstp dword ptr [esp + 030h] // 00b0e56e
        fld dword ptr [esp + 02ch] // 00b0e572
        fld dword ptr [esp + 028h] // 00b0e576
        fld dword ptr [esp + 030h] // 00b0e57a
        fld st(2) // 00b0e57e
        fmulp st(3), st(0) // 00b0e580
        fxch st(2) // 00b0e582
        fstp dword ptr [esp + 08h] // 00b0e584
        fld dword ptr [esp + 08h] // 00b0e588
        fld st(1) // 00b0e58c
        fmulp st(2), st(0) // 00b0e58e
        fxch // 00b0e590
        fstp dword ptr [esp + 08h] // 00b0e592
        fadd dword ptr [esp + 08h] // 00b0e596
        fld st(1) // 00b0e59a
        fmulp st(2), st(0) // 00b0e59c
        fxch // 00b0e59e
        fstp dword ptr [esp + 08h] // 00b0e5a0
        fadd dword ptr [esp + 08h] // 00b0e5a4
        fstp dword ptr [esp + 08h] // 00b0e5a8
        fld dword ptr [esp + 08h] // 00b0e5ac
        mov ecx, dword ptr [ebp]
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp + 08h] // 00b0e5b5
        fld dword ptr [esp + 08h] // 00b0e5b9
        fstp dword ptr [esp + 08h] // 00b0e5bd
        fldz // 00b0e5c1
        fld dword ptr [esp + 08h] // 00b0e5c3
        fcomi st(0),st(1) // 00b0e5c7
        fstp st(1) // 00b0e5c9
        jbe loc_00b0e5d7 // 00b0e5cb
        fld1 // 00b0e5cd
        fdivrp st(1), st(0) // 00b0e5cf
        fstp dword ptr [esp + 0ch] // 00b0e5d1
        jmp loc_00b0e5e2 // 00b0e5d5
    loc_00b0e5d7:
        xorps xmm0,xmm0 // 00b0e5d7
        fstp st(0) // 00b0e5da
        movss dword ptr [esp + 0ch],xmm0 // 00b0e5dc
    loc_00b0e5e2:
        fld dword ptr [esp + 028h] // 00b0e5e2
        fld dword ptr [esp + 0ch] // 00b0e5e6
        fld st(0) // 00b0e5ea
        fmulp st(2), st(0) // 00b0e5ec
        fxch // 00b0e5ee
        fstp dword ptr [esp + 01ch] // 00b0e5f0
        fld dword ptr [esp + 02ch] // 00b0e5f4
        fmul st(0), st(1) // 00b0e5f8
        fstp dword ptr [esp + 020h] // 00b0e5fa
        fmul dword ptr [esp + 030h] // 00b0e5fe
        fstp dword ptr [esp + 024h] // 00b0e602
        fld dword ptr [esp + 03ch] // 00b0e606
        fld st(0) // 00b0e60a
        fld dword ptr [esp + 024h] // 00b0e60c
        fld st(0) // 00b0e610
        fmulp st(2), st(0) // 00b0e612
        fld dword ptr [esp + 040h] // 00b0e614
        fld st(0) // 00b0e618
        fld dword ptr [esp + 020h] // 00b0e61a
        fld st(0) // 00b0e61e
        fmulp st(2), st(0) // 00b0e620
        fxch st(4) // 00b0e622
        fsubrp st(1), st(0) // 00b0e624
        fstp dword ptr [esp + 028h] // 00b0e626
        fld dword ptr [esp + 01ch] // 00b0e62a
        fld st(0) // 00b0e62e
        fmulp st(2), st(0) // 00b0e630
        fld dword ptr [esp + 038h] // 00b0e632
        fld st(0) // 00b0e636
        fmulp st(4), st(0) // 00b0e638
        fxch st(2) // 00b0e63a
        fsubrp st(3),st(0) // 00b0e63c
        fxch st(2) // 00b0e63e
        fstp dword ptr [esp + 02ch] // 00b0e640
        fmulp st(2), st(0) // 00b0e644
        fmulp st(2), st(0) // 00b0e646
        fsubrp st(1), st(0) // 00b0e648
        fstp dword ptr [esp + 030h] // 00b0e64a
        fld dword ptr [esp + 02ch] // 00b0e64e
        fld dword ptr [esp + 028h] // 00b0e652
        fld dword ptr [esp + 030h] // 00b0e656
        fld st(2) // 00b0e65a
        fmulp st(3), st(0) // 00b0e65c
        fxch st(2) // 00b0e65e
        fstp dword ptr [esp + 08h] // 00b0e660
        fld dword ptr [esp + 08h] // 00b0e664
        fld st(1) // 00b0e668
        fmulp st(2), st(0) // 00b0e66a
        fxch // 00b0e66c
        fstp dword ptr [esp + 08h] // 00b0e66e
        fadd dword ptr [esp + 08h] // 00b0e672
        fld st(1) // 00b0e676
        fmulp st(2), st(0) // 00b0e678
        fxch // 00b0e67a
        fstp dword ptr [esp + 08h] // 00b0e67c
        fadd dword ptr [esp + 08h] // 00b0e680
        fstp dword ptr [esp + 08h] // 00b0e684
        fld dword ptr [esp + 08h] // 00b0e688
        mov ecx, dword ptr [ebp]
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp + 08h] // 00b0e691
        fld dword ptr [esp + 08h] // 00b0e695
        xorps xmm0,xmm0 // 00b0e699
        fstp dword ptr [esp + 08h] // 00b0e69c
        fldz // 00b0e6a0
        fld dword ptr [esp + 08h] // 00b0e6a2
        fcomi st(0),st(1) // 00b0e6a6
        fstp st(1) // 00b0e6a8
        jbe loc_00b0e6b6 // 00b0e6aa
        fld1 // 00b0e6ac
        fdivrp st(1), st(0) // 00b0e6ae
        fstp dword ptr [esp + 0ch] // 00b0e6b0
        jmp loc_00b0e6be // 00b0e6b4
    loc_00b0e6b6:
        fstp st(0) // 00b0e6b6
        movss dword ptr [esp + 0ch],xmm0 // 00b0e6b8
    loc_00b0e6be:
        fld dword ptr [esp + 028h] // 00b0e6be
        movss dword ptr [esp + 090h],xmm0 // 00b0e6c2
        fld dword ptr [esp + 0ch] // 00b0e6cb
        movss dword ptr [esp + 0a0h],xmm0 // 00b0e6cf
        fld st(0) // 00b0e6d8
        movss dword ptr [esp + 0b0h],xmm0 // 00b0e6da
        fmulp st(2), st(0) // 00b0e6e3
        push eax
        mov eax, dword ptr [ebp + 8]
        movss xmm0,dword ptr [eax]
        pop eax
        fxch // 00b0e6ed
        sub esp,010h // 00b0e6ef
        movss dword ptr [esp + 0d0h],xmm0 // 00b0e6f2
        fstp dword ptr [esp + 054h] // 00b0e6fb
        movss xmm0,dword ptr [esp + 02ch] // 00b0e6ff
        fld dword ptr [esp + 03ch] // 00b0e705
        movss dword ptr [esp + 094h],xmm0 // 00b0e709
        fmul st(0), st(1) // 00b0e712
        movss xmm0,dword ptr [esp + 030h] // 00b0e714
        movss dword ptr [esp + 0a4h],xmm0 // 00b0e71a
        movss xmm0,dword ptr [esp + 034h] // 00b0e723
        fstp dword ptr [esp + 058h] // 00b0e729
        movss dword ptr [esp + 0b4h],xmm0 // 00b0e72d
        movss xmm0,dword ptr [esp + 054h] // 00b0e736
        fmul dword ptr [esp + 040h] // 00b0e73c
        movss dword ptr [esp + 098h],xmm0 // 00b0e740
        movss xmm0,dword ptr [esp + 058h] // 00b0e749
        movss dword ptr [esp + 0a8h],xmm0 // 00b0e74f
        fstp dword ptr [esp + 05ch] // 00b0e758
        movss xmm0,dword ptr [esp + 048h] // 00b0e75c
        fld dword ptr [esp + 05ch] // 00b0e762
        movss dword ptr [esp + 09ch],xmm0 // 00b0e766
        fstp dword ptr [esp + 0b8h] // 00b0e76f
        movss xmm0,dword ptr [esp + 04ch] // 00b0e776
        fld dword ptr [edi + 01d8h] // 00b0e77c
        movss dword ptr [esp + 0ach],xmm0 // 00b0e782
        fstp dword ptr [esp + 018h] // 00b0e78b
        movss xmm0,dword ptr [esp + 050h] // 00b0e78f
        fld dword ptr [edi + 01d4h] // 00b0e795
        movss dword ptr [esp + 0bch],xmm0 // 00b0e79b
        fstp dword ptr [esp + 020h] // 00b0e7a4
        movss xmm0,dword ptr [esp + 088h] // 00b0e7a8
        fld dword ptr [edi + 01c8h] // 00b0e7b1
        movss dword ptr [esp + 0c4h],xmm0 // 00b0e7b7
        fstp dword ptr [esp + 044h] // 00b0e7c0
        movss xmm0,dword ptr [esp + 08ch] // 00b0e7c4
        fld dword ptr [esp + 018h] // 00b0e7cd
        movss dword ptr [esp + 0c8h],xmm0 // 00b0e7d1
        fstp dword ptr [esp + 0ch] // 00b0e7da
        movss xmm0,dword ptr [esp + 090h] // 00b0e7de
        fld dword ptr [esp + 020h] // 00b0e7e7
        lea ecx,[esp + 0114h] // 00b0e7eb
        fstp dword ptr [esp + 08h] // 00b0e7f2
        movss dword ptr [esp + 0cch],xmm0 // 00b0e7f6
        fld dword ptr [esp + 044h] // 00b0e7ff
        fstp dword ptr [esp + 04h] // 00b0e803
        fld dword ptr [esp + 06ch] // 00b0e807
        fld dword ptr [esp + 068h] // 00b0e80b
        fld st(0) // 00b0e80f
        fsubp st(2),st(0) // 00b0e811
        fld dword ptr [esp + 028h] // 00b0e813
        fmulp st(2), st(0) // 00b0e817
        faddp st(1), st(0) // 00b0e819
        fstp dword ptr [esp + 068h] // 00b0e81b
        fld dword ptr [esp + 068h] // 00b0e81f
        fstp dword ptr [esp] // 00b0e823
        lea edx, [esp]
        call build_native_camera_projection_00b642f0
        add esp, 010h
        push eax // 00b0e82b
        lea ecx,[esp + 0c8h] // 00b0e82c
        push ecx // 00b0e833
        lea edx,[esp + 08ch] // 00b0e834
        lea ecx,[esp + 014ch] // 00b0e83b
        call invert_native_camera_scaled_affine_00b63b30
        mov ecx,eax // 00b0e847
        call multiply_native_camera_matrices_00413920
        fld dword ptr [eax] // 00b0e84e
        fstp dword ptr [esi + 0248h] // 00b0e850
        fld dword ptr [eax + 04h] // 00b0e856
        fstp dword ptr [esi + 024ch] // 00b0e859
        fld dword ptr [eax + 08h] // 00b0e85f
        fstp dword ptr [esi + 0250h] // 00b0e862
        fld dword ptr [eax + 0ch] // 00b0e868
        fstp dword ptr [esi + 0254h] // 00b0e86b
        fld dword ptr [eax + 010h] // 00b0e871
        fstp dword ptr [esi + 0258h] // 00b0e874
        fld dword ptr [eax + 014h] // 00b0e87a
        fstp dword ptr [esi + 025ch] // 00b0e87d
        fld dword ptr [eax + 018h] // 00b0e883
        fstp dword ptr [esi + 0260h] // 00b0e886
        fld dword ptr [eax + 01ch] // 00b0e88c
        fstp dword ptr [esi + 0264h] // 00b0e88f
        fld dword ptr [eax + 020h] // 00b0e895
        fstp dword ptr [esi + 0268h] // 00b0e898
        fld dword ptr [eax + 024h] // 00b0e89e
        fstp dword ptr [esi + 026ch] // 00b0e8a1
        fld dword ptr [eax + 028h] // 00b0e8a7
        fstp dword ptr [esi + 0270h] // 00b0e8aa
        fld dword ptr [eax + 02ch] // 00b0e8b0
        fstp dword ptr [esi + 0274h] // 00b0e8b3
        fld dword ptr [eax + 030h] // 00b0e8b9
        fstp dword ptr [esi + 0278h] // 00b0e8bc
        fld dword ptr [eax + 034h] // 00b0e8c2
        fstp dword ptr [esi + 027ch] // 00b0e8c5
        fld dword ptr [eax + 038h] // 00b0e8cb
        fstp dword ptr [esi + 0280h] // 00b0e8ce
        fld dword ptr [eax + 03ch] // 00b0e8d4
        fstp dword ptr [esi + 0284h] // 00b0e8d7
        test byte ptr [edi + 05ch],02h // 00b0e8dd
        jnz loc_00b0e8ea // 00b0e8e1
        mov ecx,edi // 00b0e8e3
        call refresh_native_camera_world_00b6db70
    loc_00b0e8ea:
        movss xmm0,dword ptr [edi + 0120h] // 00b0e8ea
        movss xmm1,dword ptr [edi + 0124h] // 00b0e8f2
        movss xmm2,dword ptr [edi + 0128h] // 00b0e8fa
        pop edi // 00b0e902
        movss dword ptr [esi + 0288h],xmm0 // 00b0e903
        movss dword ptr [esi + 028ch],xmm1 // 00b0e90b
        movss dword ptr [esi + 0290h],xmm2 // 00b0e913
        pop esi // 00b0e91b
        add esp,017ch // 00b0e91c
        pop ebp
        ret 8
    }
}
} // namespace bsp
