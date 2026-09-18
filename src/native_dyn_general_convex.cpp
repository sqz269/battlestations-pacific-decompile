#include "bsp/native_dyn_general_convex.hpp"
#include "bsp/native_dyn_convex_simplex.hpp"
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native general convex intersection requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
void intersect_kernel();
alignas(8) const std::uint64_t constant_00d7a278=0x47efffffe0000000ULL;
alignas(8) const std::uint64_t constant_00d7a280=0x3fe0000000000000ULL;
alignas(8) const std::uint64_t constant_00d7a290=0x3f1a36e2eb1c432dULL;
alignas(8) const std::uint64_t constant_00d7a2b8=0x3eb0c6f7a0b5ed8dULL;
alignas(8) const std::uint64_t constant_00d7a2c0=0xbeb0c6f7a0b5ed8dULL;
alignas(8) const std::uint64_t constant_00d7a2d0=0x3ec0c6f7a0000000ULL;
alignas(8) const std::uint64_t constant_00d7a2f8=0x3f947ae140000000ULL;
alignas(8) const std::uint64_t constant_00d7a330=0x3ee4f8b580000000ULL;
alignas(8) const std::uint64_t constant_00d7a338=0xbf1a36e2eb1c432dULL;
alignas(8) const std::uint64_t constant_00d7a340=0xc12e847fc0000000ULL;
alignas(8) const std::uint64_t constant_00d7a348=0x3fd0000000000000ULL;
alignas(8) const std::uint64_t constant_00d7a350=0x3e45798ee2308c3aULL;
alignas(8) const std::uint64_t constant_00d7a358=0x3f847ae140000000ULL;
// The extra final CRT argument leaves original locals/arguments in place.
__declspec(naked) void sqrt_shim(){
    __asm {
        push ecx
        mov ecx,dword ptr [esp+8]
        call native_crt_sqrt_st0_00bf7030
        pop ecx
        ret 4
    }
}
// The extra final CRT argument leaves original locals/arguments in place.
__declspec(naked) void intersect_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+14h]
        push dword ptr [esp+14h]
        push dword ptr [esp+14h]
        call intersect_kernel
        ret 10h
    }
}
// Complete recovered instruction schedule; address comments are PE starts.
__declspec(naked) void result_kernel(){
    __asm {
        sub esp, 028h // 00c48be0
        mov eax, dword ptr [esi + 078h] // 00c48be3
        fld qword ptr constant_00d7a2f8 // 00c48be6
        mov dword ptr [eax], 1 // 00c48bec
        fstp qword ptr [esi + 070h] // 00c48bf2
        fld qword ptr [esi + 088h] // 00c48bf5
        push edi // 00c48bfb
        fld qword ptr [esi + 080h] // 00c48bfc
        mov edi, dword ptr [esi + 078h] // 00c48c02
        fld qword ptr [esi + 090h] // 00c48c05
        add edi, 4 // 00c48c0b
        fld st(1) // 00c48c0e
        fmulp st(2), st(0) // 00c48c10
        fld st(2) // 00c48c12
        fmulp st(3), st(0) // 00c48c14
        fxch st(1) // 00c48c16
        faddp st(2), st(0) // 00c48c18
        fmul st(0), st(0) // 00c48c1a
        faddp st(1), st(0) // 00c48c1c
        push dword ptr [esp+48] // Borrowed CRT access.
        call sqrt_shim // 00c48c1e
        fld1  // 00c48c23
        fdivrp st(1), st(0) // 00c48c25
        fld st(0) // 00c48c27
        fmul qword ptr [esi + 080h] // 00c48c29
        fstp qword ptr [esi + 080h] // 00c48c2f
        fld st(0) // 00c48c35
        fmul qword ptr [esi + 088h] // 00c48c37
        fstp qword ptr [esi + 088h] // 00c48c3d
        fmul qword ptr [esi + 090h] // 00c48c43
        fstp qword ptr [esi + 090h] // 00c48c49
        fld qword ptr [esi + 01c0h] // 00c48c4f
        fsub qword ptr [esi + 01d8h] // 00c48c55
        fld qword ptr [esi + 01c8h] // 00c48c5b
        fsub qword ptr [esi + 01e0h] // 00c48c61
        fld qword ptr [esi + 01d0h] // 00c48c67
        fsub qword ptr [esi + 01e8h] // 00c48c6d
        fld qword ptr [esi + 088h] // 00c48c73
        fmulp st(2), st(0) // 00c48c79
        fld qword ptr [esi + 080h] // 00c48c7b
        fmulp st(3), st(0) // 00c48c81
        fxch st(1) // 00c48c83
        faddp st(2), st(0) // 00c48c85
        fmul qword ptr [esi + 090h] // 00c48c87
        faddp st(1), st(0) // 00c48c8d
        fld qword ptr [esi + 080h] // 00c48c8f
        fmul st(0), st(1) // 00c48c95
        fld qword ptr [esi + 088h] // 00c48c97
        fmul st(0), st(2) // 00c48c9d
        fld qword ptr [esi + 090h] // 00c48c9f
        fmulp st(3), st(0) // 00c48ca5
        fld qword ptr [esi + 01d8h] // 00c48ca7
        faddp st(2), st(0) // 00c48cad
        fadd qword ptr [esi + 01e0h] // 00c48caf
        fld qword ptr [esi + 01e8h] // 00c48cb5
        faddp st(3), st(0) // 00c48cbb
        fxch st(1) // 00c48cbd
        fstp qword ptr [esi + 01c0h] // 00c48cbf
        fstp qword ptr [esi + 01c8h] // 00c48cc5
        fstp qword ptr [esi + 01d0h] // 00c48ccb
        mov ecx, dword ptr [esi] // 00c48cd1
        fld qword ptr [esi + 01c0h] // 00c48cd3
        mov eax, dword ptr [ecx + 4] // 00c48cd9
        fstp dword ptr [esp + 8] // 00c48cdc
        add eax, 8 // 00c48ce0
        fld qword ptr [esi + 01c8h] // 00c48ce3
        fstp dword ptr [esp + 0ch] // 00c48ce9
        fld qword ptr [esi + 01d0h] // 00c48ced
        fstp dword ptr [esp + 010h] // 00c48cf3
        fld qword ptr [esi + 01d8h] // 00c48cf7
        fstp dword ptr [esp + 020h] // 00c48cfd
        fld qword ptr [esi + 01e0h] // 00c48d01
        fstp dword ptr [esp + 024h] // 00c48d07
        fld qword ptr [esi + 01e8h] // 00c48d0b
        fstp dword ptr [esp + 028h] // 00c48d11
        fld dword ptr [esp + 8] // 00c48d15
        fsub dword ptr [eax + 024h] // 00c48d19
        fstp dword ptr [esp + 014h] // 00c48d1c
        fld dword ptr [esp + 0ch] // 00c48d20
        fsub dword ptr [eax + 028h] // 00c48d24
        fstp dword ptr [esp + 018h] // 00c48d27
        fld dword ptr [esp + 010h] // 00c48d2b
        fsub dword ptr [eax + 02ch] // 00c48d2f
        fstp dword ptr [esp + 01ch] // 00c48d32
        fld dword ptr [eax + 4] // 00c48d36
        fld dword ptr [esp + 018h] // 00c48d39
        fld st(0) // 00c48d3d
        fmulp st(2), st(0) // 00c48d3f
        fld dword ptr [eax] // 00c48d41
        fld dword ptr [esp + 014h] // 00c48d43
        fld st(0) // 00c48d47
        fmulp st(2), st(0) // 00c48d49
        fxch st(3) // 00c48d4b
        faddp st(1), st(0) // 00c48d4d
        fld dword ptr [eax + 8] // 00c48d4f
        fld dword ptr [esp + 01ch] // 00c48d52
        fld st(0) // 00c48d56
        fmulp st(2), st(0) // 00c48d58
        fxch st(2) // 00c48d5a
        faddp st(1), st(0) // 00c48d5c
        fstp dword ptr [edi] // 00c48d5e
        fld dword ptr [eax + 010h] // 00c48d60
        fmul st(0), st(2) // 00c48d63
        fld dword ptr [eax + 0ch] // 00c48d65
        fmul st(0), st(4) // 00c48d68
        faddp st(1), st(0) // 00c48d6a
        fld dword ptr [eax + 014h] // 00c48d6c
        fmul st(0), st(2) // 00c48d6f
        faddp st(1), st(0) // 00c48d71
        fstp dword ptr [edi + 4] // 00c48d73
        fld dword ptr [eax + 01ch] // 00c48d76
        fmulp st(2), st(0) // 00c48d79
        fld dword ptr [eax + 018h] // 00c48d7b
        fmulp st(3), st(0) // 00c48d7e
        fxch st(1) // 00c48d80
        faddp st(2), st(0) // 00c48d82
        fmul dword ptr [eax + 020h] // 00c48d84
        faddp st(1), st(0) // 00c48d87
        fstp dword ptr [edi + 8] // 00c48d89
        mov edx, dword ptr [esi + 4] // 00c48d8c
        mov eax, dword ptr [edx + 4] // 00c48d8f
        fld dword ptr [esp + 020h] // 00c48d92
        fsub dword ptr [eax + 02ch] // 00c48d96
        add eax, 8 // 00c48d99
        fstp dword ptr [esp + 014h] // 00c48d9c
        fld dword ptr [esp + 024h] // 00c48da0
        fsub dword ptr [eax + 028h] // 00c48da4
        fstp dword ptr [esp + 018h] // 00c48da7
        fld dword ptr [esp + 028h] // 00c48dab
        fsub dword ptr [eax + 02ch] // 00c48daf
        fstp dword ptr [esp + 01ch] // 00c48db2
        fld dword ptr [eax + 4] // 00c48db6
        fld dword ptr [esp + 018h] // 00c48db9
        fld st(0) // 00c48dbd
        fmulp st(2), st(0) // 00c48dbf
        fld dword ptr [eax] // 00c48dc1
        fld dword ptr [esp + 014h] // 00c48dc3
        fld st(0) // 00c48dc7
        fmulp st(2), st(0) // 00c48dc9
        fxch st(3) // 00c48dcb
        faddp st(1), st(0) // 00c48dcd
        fld dword ptr [eax + 8] // 00c48dcf
        fld dword ptr [esp + 01ch] // 00c48dd2
        fld st(0) // 00c48dd6
        fmulp st(2), st(0) // 00c48dd8
        fxch st(2) // 00c48dda
        faddp st(1), st(0) // 00c48ddc
        fstp dword ptr [edi + 0ch] // 00c48dde
        fld dword ptr [eax + 010h] // 00c48de1
        fmul st(0), st(2) // 00c48de4
        fld dword ptr [eax + 0ch] // 00c48de6
        fmul st(0), st(4) // 00c48de9
        faddp st(1), st(0) // 00c48deb
        fld dword ptr [eax + 014h] // 00c48ded
        fmul st(0), st(2) // 00c48df0
        faddp st(1), st(0) // 00c48df2
        fstp dword ptr [edi + 010h] // 00c48df4
        fld dword ptr [eax + 01ch] // 00c48df7
        fmulp st(2), st(0) // 00c48dfa
        fld dword ptr [eax + 018h] // 00c48dfc
        fmulp st(3), st(0) // 00c48dff
        fxch st(1) // 00c48e01
        faddp st(2), st(0) // 00c48e03
        fmul dword ptr [eax + 020h] // 00c48e05
        faddp st(1), st(0) // 00c48e08
        fstp dword ptr [edi + 014h] // 00c48e0a
        fld qword ptr [esi + 080h] // 00c48e0d
        fstp dword ptr [edi + 018h] // 00c48e13
        fld qword ptr [esi + 088h] // 00c48e16
        fstp dword ptr [edi + 01ch] // 00c48e1c
        fld qword ptr [esi + 090h] // 00c48e1f
        fstp dword ptr [edi + 020h] // 00c48e25
        pop edi // 00c48e28
        add esp, 028h // 00c48e29
        ret 4 // 00c48e2c
    }
}
// Complete recovered instruction schedule; address comments are PE starts.
__declspec(naked) void direction_kernel(){
    __asm {
        sub esp, 074h // 00c51c20
        mov eax, dword ptr [edi] // 00c51c23
        fld dword ptr [eax + 018h] // 00c51c25
        push ebx // 00c51c28
        fadd dword ptr [eax + 0ch] // 00c51c29
        push esi // 00c51c2c
        fstp dword ptr [esp + 8] // 00c51c2d
        fld dword ptr [eax + 01ch] // 00c51c31
        fadd dword ptr [eax + 010h] // 00c51c34
        fstp dword ptr [esp + 0ch] // 00c51c37
        fld dword ptr [eax + 020h] // 00c51c3b
        fadd dword ptr [eax + 014h] // 00c51c3e
        mov eax, dword ptr [eax + 4] // 00c51c41
        add eax, 8 // 00c51c44
        fstp dword ptr [esp + 010h] // 00c51c47
        fld dword ptr [esp + 8] // 00c51c4b
        fld qword ptr constant_00d7a280 // 00c51c4f
        fmul st(1), st(0) // 00c51c55
        fxch st(1) // 00c51c57
        fstp dword ptr [esp + 018h] // 00c51c59
        fld dword ptr [esp + 0ch] // 00c51c5d
        fmul st(0), st(1) // 00c51c61
        fstp dword ptr [esp + 01ch] // 00c51c63
        fld dword ptr [esp + 010h] // 00c51c67
        fmul st(0), st(1) // 00c51c6b
        fstp dword ptr [esp + 020h] // 00c51c6d
        fld dword ptr [eax + 0ch] // 00c51c71
        fld dword ptr [esp + 01ch] // 00c51c74
        fld st(0) // 00c51c78
        fmulp st(2), st(0) // 00c51c7a
        fld dword ptr [eax] // 00c51c7c
        fld dword ptr [esp + 018h] // 00c51c7e
        fld st(0) // 00c51c82
        fmulp st(2), st(0) // 00c51c84
        fxch st(3) // 00c51c86
        faddp st(1), st(0) // 00c51c88
        fld dword ptr [eax + 018h] // 00c51c8a
        fld dword ptr [esp + 020h] // 00c51c8d
        fld st(0) // 00c51c91
        fmulp st(2), st(0) // 00c51c93
        fxch st(2) // 00c51c95
        faddp st(1), st(0) // 00c51c97
        fadd dword ptr [eax + 024h] // 00c51c99
        fstp dword ptr [esp + 030h] // 00c51c9c
        fld dword ptr [eax + 010h] // 00c51ca0
        fmul st(0), st(2) // 00c51ca3
        fld dword ptr [eax + 4] // 00c51ca5
        fmul st(0), st(4) // 00c51ca8
        faddp st(1), st(0) // 00c51caa
        fld dword ptr [eax + 01ch] // 00c51cac
        fmul st(0), st(2) // 00c51caf
        faddp st(1), st(0) // 00c51cb1
        fadd dword ptr [eax + 028h] // 00c51cb3
        fstp dword ptr [esp + 034h] // 00c51cb6
        fld dword ptr [eax + 014h] // 00c51cba
        fmulp st(2), st(0) // 00c51cbd
        fld dword ptr [eax + 8] // 00c51cbf
        fmulp st(3), st(0) // 00c51cc2
        fxch st(1) // 00c51cc4
        faddp st(2), st(0) // 00c51cc6
        fmul dword ptr [eax + 020h] // 00c51cc8
        faddp st(1), st(0) // 00c51ccb
        fadd dword ptr [eax + 02ch] // 00c51ccd
        mov eax, dword ptr [edi + 4] // 00c51cd0
        fstp dword ptr [esp + 038h] // 00c51cd3
        fld dword ptr [eax + 0ch] // 00c51cd7
        fadd dword ptr [eax + 018h] // 00c51cda
        fstp dword ptr [esp + 018h] // 00c51cdd
        fld dword ptr [eax + 01ch] // 00c51ce1
        fadd dword ptr [eax + 010h] // 00c51ce4
        fstp dword ptr [esp + 01ch] // 00c51ce7
        fld dword ptr [eax + 020h] // 00c51ceb
        fadd dword ptr [eax + 014h] // 00c51cee
        fstp dword ptr [esp + 020h] // 00c51cf1
        fld dword ptr [esp + 018h] // 00c51cf5
        fmul st(0), st(1) // 00c51cf9
        fstp dword ptr [esp + 8] // 00c51cfb
        fld dword ptr [esp + 01ch] // 00c51cff
        fmul st(0), st(1) // 00c51d03
        fstp dword ptr [esp + 0ch] // 00c51d05
        fmul dword ptr [esp + 020h] // 00c51d09
        fstp dword ptr [esp + 010h] // 00c51d0d
        mov eax, dword ptr [eax + 4] // 00c51d11
        fld dword ptr [eax + 014h] // 00c51d14
        add eax, 8 // 00c51d17
        fld dword ptr [esp + 0ch] // 00c51d1a
        xor ebx, ebx // 00c51d1e
        fld st(0) // 00c51d20
        fmulp st(2), st(0) // 00c51d22
        fld dword ptr [eax] // 00c51d24
        fld dword ptr [esp + 8] // 00c51d26
        fld st(0) // 00c51d2a
        fmulp st(2), st(0) // 00c51d2c
        fxch st(3) // 00c51d2e
        faddp st(1), st(0) // 00c51d30
        fld dword ptr [eax + 018h] // 00c51d32
        fld dword ptr [esp + 010h] // 00c51d35
        fld st(0) // 00c51d39
        fmulp st(2), st(0) // 00c51d3b
        fxch st(2) // 00c51d3d
        faddp st(1), st(0) // 00c51d3f
        fadd dword ptr [eax + 024h] // 00c51d41
        fstp dword ptr [esp + 018h] // 00c51d44
        fld dword ptr [eax + 010h] // 00c51d48
        fmul st(0), st(2) // 00c51d4b
        fld dword ptr [eax + 4] // 00c51d4d
        fmul st(0), st(4) // 00c51d50
        faddp st(1), st(0) // 00c51d52
        fld dword ptr [eax + 01ch] // 00c51d54
        fmul st(0), st(2) // 00c51d57
        faddp st(1), st(0) // 00c51d59
        fadd dword ptr [eax + 028h] // 00c51d5b
        fstp dword ptr [esp + 01ch] // 00c51d5e
        fld dword ptr [eax + 014h] // 00c51d62
        fmulp st(2), st(0) // 00c51d65
        fld dword ptr [eax + 8] // 00c51d67
        fmulp st(3), st(0) // 00c51d6a
        fxch st(1) // 00c51d6c
        faddp st(2), st(0) // 00c51d6e
        fmul dword ptr [eax + 020h] // 00c51d70
        faddp st(1), st(0) // 00c51d73
        fadd dword ptr [eax + 02ch] // 00c51d75
        fstp dword ptr [esp + 020h] // 00c51d78
        fld dword ptr [esp + 018h] // 00c51d7c
        fsub dword ptr [esp + 030h] // 00c51d80
        fstp dword ptr [esp + 8] // 00c51d84
        fld dword ptr [esp + 01ch] // 00c51d88
        fsub dword ptr [esp + 034h] // 00c51d8c
        fstp dword ptr [esp + 0ch] // 00c51d90
        fld dword ptr [esp + 020h] // 00c51d94
        fsub dword ptr [esp + 038h] // 00c51d98
        fstp dword ptr [esp + 010h] // 00c51d9c
        fld dword ptr [esp + 8] // 00c51da0
        fstp qword ptr [esp + 030h] // 00c51da4
        fld dword ptr [esp + 0ch] // 00c51da8
        fstp qword ptr [esp + 038h] // 00c51dac
        fld dword ptr [esp + 010h] // 00c51db0
        fstp qword ptr [esp + 040h] // 00c51db4
        fld qword ptr constant_00d7a278 // 00c51db8
        fstp qword ptr [esp + 8] // 00c51dbe
    l_00c51dc2:
        mov esi, dword ptr [edi + 01f0h] // 00c51dc2
        fld qword ptr [esi + ebx + 8] // 00c51dc8
        add esi, ebx // 00c51dcc
        fmul qword ptr [esp + 038h] // 00c51dce
        fld qword ptr [esi] // 00c51dd2
        fmul qword ptr [esp + 030h] // 00c51dd4
        faddp st(1), st(0) // 00c51dd8
        fld qword ptr [esi + 010h] // 00c51dda
        fmul qword ptr [esp + 040h] // 00c51ddd
        faddp st(1), st(0) // 00c51de1
        fld qword ptr constant_00d7a330 // 00c51de3
        fcomip st(0), st(1) // 00c51de9
        fstp st(0) // 00c51deb
        ja l_00c51ed2 // 00c51ded
        mov ecx, dword ptr [edi] // 00c51df3
        mov eax, dword ptr [ecx] // 00c51df5
        mov eax, dword ptr [eax + 0ch] // 00c51df7
        lea edx, [edi + 0ch] // 00c51dfa
        push edx // 00c51dfd
        push esi // 00c51dfe
        lea edx, [esp + 020h] // 00c51dff
        push edx // 00c51e03
        call eax // 00c51e04
        fld qword ptr [esi] // 00c51e06
        mov ecx, dword ptr [edi + 4] // 00c51e08
        fchs  // 00c51e0b
        fstp qword ptr [esp + 060h] // 00c51e0d
        lea eax, [edi + 03ch] // 00c51e11
        fld qword ptr [esi + 8] // 00c51e14
        push eax // 00c51e17
        fchs  // 00c51e18
        lea eax, [esp + 064h] // 00c51e1a
        fstp qword ptr [esp + 06ch] // 00c51e1e
        push eax // 00c51e22
        fld qword ptr [esi + 010h] // 00c51e23
        lea eax, [esp + 050h] // 00c51e26
        fchs  // 00c51e2a
        push eax // 00c51e2c
        fstp qword ptr [esp + 07ch] // 00c51e2d
        mov edx, dword ptr [ecx] // 00c51e31
        mov edx, dword ptr [edx + 0ch] // 00c51e33
        call edx // 00c51e36
        fld qword ptr [esp + 018h] // 00c51e38
        fld st(0) // 00c51e3c
        fld qword ptr [esp + 048h] // 00c51e3e
        fsub st(1), st(0) // 00c51e42
        fld qword ptr [esp + 020h] // 00c51e44
        fld st(0) // 00c51e48
        fld qword ptr [esp + 050h] // 00c51e4a
        fsub st(1), st(0) // 00c51e4e
        fld qword ptr [esp + 028h] // 00c51e50
        fsub qword ptr [esp + 058h] // 00c51e54
        fld qword ptr [esi + 8] // 00c51e58
        fmulp st(3), st(0) // 00c51e5b
        fld qword ptr [esi] // 00c51e5d
        fmulp st(6), st(0) // 00c51e5f
        fxch st(2) // 00c51e61
        faddp st(5), st(0) // 00c51e63
        fld qword ptr [esi + 010h] // 00c51e65
        fmulp st(2), st(0) // 00c51e68
        fxch st(4) // 00c51e6a
        faddp st(1), st(0) // 00c51e6c
        fld qword ptr [esp + 8] // 00c51e6e
        fcomip st(0), st(1) // 00c51e72
        jbe l_00c51ec8 // 00c51e74
        fstp qword ptr [esp + 8] // 00c51e76
        fld qword ptr [esi] // 00c51e7a
        fstp qword ptr [edi + 080h] // 00c51e7c
        fld qword ptr [esi + 8] // 00c51e82
        fstp qword ptr [edi + 088h] // 00c51e85
        fld qword ptr [esi + 010h] // 00c51e8b
        fstp qword ptr [edi + 090h] // 00c51e8e
        fxch st(3) // 00c51e94
        fstp qword ptr [edi + 01c0h] // 00c51e96
        fxch st(2) // 00c51e9c
        fstp qword ptr [edi + 01c8h] // 00c51e9e
        fld qword ptr [esp + 028h] // 00c51ea4
        fstp qword ptr [edi + 01d0h] // 00c51ea8
        fxch st(1) // 00c51eae
        fstp qword ptr [edi + 01d8h] // 00c51eb0
        fstp qword ptr [edi + 01e0h] // 00c51eb6
        fld qword ptr [esp + 058h] // 00c51ebc
        fstp qword ptr [edi + 01e8h] // 00c51ec0
        jmp l_00c51ed2 // 00c51ec6
    l_00c51ec8:
        fstp st(0) // 00c51ec8
        fstp st(3) // 00c51eca
        fstp st(0) // 00c51ecc
        fstp st(1) // 00c51ece
        fstp st(0) // 00c51ed0
    l_00c51ed2:
        add ebx, 018h // 00c51ed2
        cmp ebx, 0270h // 00c51ed5
        jl l_00c51dc2 // 00c51edb
        pop esi // 00c51ee1
        pop ebx // 00c51ee2
        add esp, 074h // 00c51ee3
        ret  // 00c51ee6
    }
}
// Complete recovered instruction schedule; address comments are PE starts.
__declspec(naked) void search_kernel(){
    __asm {
        sub esp, 064h // 00c51ef0
        push ebx // 00c51ef3
        push ebp // 00c51ef4
        push esi // 00c51ef5
        mov esi, ecx // 00c51ef6
        mov eax, dword ptr [esi] // 00c51ef8
        fld dword ptr [eax + 018h] // 00c51efa
        add eax, 0ch // 00c51efd
        fadd dword ptr [eax] // 00c51f00
        lea ebx, [esi + 0ch] // 00c51f02
        push edi // 00c51f05
        fstp dword ptr [esp + 028h] // 00c51f06
        fld dword ptr [eax + 010h] // 00c51f0a
        fadd dword ptr [eax + 4] // 00c51f0d
        fstp dword ptr [esp + 02ch] // 00c51f10
        fld dword ptr [eax + 014h] // 00c51f14
        fadd dword ptr [eax + 8] // 00c51f17
        mov eax, dword ptr [esi + 4] // 00c51f1a
        add eax, 0ch // 00c51f1d
        fstp dword ptr [esp + 030h] // 00c51f20
        fld dword ptr [esp + 028h] // 00c51f24
        fld qword ptr constant_00d7a280 // 00c51f28
        fmul st(1), st(0) // 00c51f2e
        fxch st(1) // 00c51f30
        fstp dword ptr [esp + 010h] // 00c51f32
        fld dword ptr [esp + 02ch] // 00c51f36
        fmul st(0), st(1) // 00c51f3a
        fstp dword ptr [esp + 014h] // 00c51f3c
        fld dword ptr [esp + 030h] // 00c51f40
        fmul st(0), st(1) // 00c51f44
        fstp dword ptr [esp + 018h] // 00c51f46
        fld dword ptr [ebx + 0ch] // 00c51f4a
        fld dword ptr [esp + 014h] // 00c51f4d
        fld st(0) // 00c51f51
        fmulp st(2), st(0) // 00c51f53
        fld dword ptr [esp + 010h] // 00c51f55
        fld st(0) // 00c51f59
        fmul dword ptr [ebx] // 00c51f5b
        faddp st(3), st(0) // 00c51f5d
        fld dword ptr [ebx + 018h] // 00c51f5f
        fld dword ptr [esp + 018h] // 00c51f62
        fld st(0) // 00c51f66
        fmulp st(2), st(0) // 00c51f68
        fxch st(4) // 00c51f6a
        faddp st(1), st(0) // 00c51f6c
        fadd dword ptr [ebx + 024h] // 00c51f6e
        fstp dword ptr [esp + 028h] // 00c51f71
        fld dword ptr [ebx + 010h] // 00c51f75
        fmul st(0), st(2) // 00c51f78
        fld dword ptr [ebx + 4] // 00c51f7a
        fmul st(0), st(2) // 00c51f7d
        faddp st(1), st(0) // 00c51f7f
        fld dword ptr [ebx + 01ch] // 00c51f81
        fmul st(0), st(4) // 00c51f84
        faddp st(1), st(0) // 00c51f86
        fadd dword ptr [ebx + 028h] // 00c51f88
        fstp dword ptr [esp + 02ch] // 00c51f8b
        fld dword ptr [ebx + 014h] // 00c51f8f
        fmulp st(2), st(0) // 00c51f92
        fmul dword ptr [ebx + 8] // 00c51f94
        faddp st(1), st(0) // 00c51f97
        fld dword ptr [ebx + 020h] // 00c51f99
        fmulp st(2), st(0) // 00c51f9c
        faddp st(1), st(0) // 00c51f9e
        fadd dword ptr [ebx + 02ch] // 00c51fa0
        fstp dword ptr [esp + 030h] // 00c51fa3
        fld dword ptr [esp + 028h] // 00c51fa7
        fld dword ptr [esp + 02ch] // 00c51fab
        fld dword ptr [esp + 030h] // 00c51faf
        fstp qword ptr [esp + 068h] // 00c51fb3
        fld dword ptr [eax + 0ch] // 00c51fb7
        fadd dword ptr [eax] // 00c51fba
        fstp dword ptr [esp + 028h] // 00c51fbc
        fld dword ptr [eax + 010h] // 00c51fc0
        fadd dword ptr [eax + 4] // 00c51fc3
        fstp dword ptr [esp + 02ch] // 00c51fc6
        fld dword ptr [eax + 014h] // 00c51fca
        fadd dword ptr [eax + 8] // 00c51fcd
        fstp dword ptr [esp + 030h] // 00c51fd0
        fld dword ptr [esp + 028h] // 00c51fd4
        fmul st(0), st(3) // 00c51fd8
        fstp dword ptr [esp + 010h] // 00c51fda
        fld dword ptr [esp + 02ch] // 00c51fde
        lea ebp, [esi + 03ch] // 00c51fe2
        fmul st(0), st(3) // 00c51fe5
        lea edi, [esi + 080h] // 00c51fe7
        fstp dword ptr [esp + 014h] // 00c51fed
        fld dword ptr [esp + 030h] // 00c51ff1
        fmulp st(3), st(0) // 00c51ff5
        fxch st(2) // 00c51ff7
        fstp dword ptr [esp + 018h] // 00c51ff9
        fld dword ptr [ebp + 0ch] // 00c51ffd
        fld dword ptr [esp + 014h] // 00c52000
        fld st(0) // 00c52004
        fmulp st(2), st(0) // 00c52006
        fld dword ptr [ebp] // 00c52008
        fld dword ptr [esp + 010h] // 00c5200b
        fld st(0) // 00c5200f
        fmulp st(2), st(0) // 00c52011
        fxch st(3) // 00c52013
        faddp st(1), st(0) // 00c52015
        fld dword ptr [ebp + 018h] // 00c52017
        fld dword ptr [esp + 018h] // 00c5201a
        fld st(0) // 00c5201e
        fmulp st(2), st(0) // 00c52020
        fxch st(2) // 00c52022
        faddp st(1), st(0) // 00c52024
        fadd dword ptr [ebp + 024h] // 00c52026
        fstp dword ptr [esp + 028h] // 00c52029
        fld dword ptr [ebp + 010h] // 00c5202d
        fmul st(0), st(2) // 00c52030
        fld dword ptr [ebp + 4] // 00c52032
        fmul st(0), st(4) // 00c52035
        faddp st(1), st(0) // 00c52037
        fld dword ptr [ebp + 01ch] // 00c52039
        fmul st(0), st(2) // 00c5203c
        faddp st(1), st(0) // 00c5203e
        fadd dword ptr [ebp + 028h] // 00c52040
        fstp dword ptr [esp + 02ch] // 00c52043
        fld dword ptr [ebp + 014h] // 00c52047
        fmulp st(2), st(0) // 00c5204a
        fld dword ptr [ebp + 8] // 00c5204c
        fmulp st(3), st(0) // 00c5204f
        fxch st(1) // 00c52051
        faddp st(2), st(0) // 00c52053
        fmul dword ptr [ebp + 020h] // 00c52055
        faddp st(1), st(0) // 00c52058
        fadd dword ptr [ebp + 02ch] // 00c5205a
        mov dword ptr [esi + 01b8h], 0 // 00c5205d
        fstp dword ptr [esp + 030h] // 00c52067
        fld dword ptr [esp + 028h] // 00c5206b
        fld dword ptr [esp + 02ch] // 00c5206f
        fld dword ptr [esp + 030h] // 00c52073
        fxch st(2) // 00c52077
        fsubrp st(3), st(0) // 00c52079
        fsubrp st(3), st(0) // 00c5207b
        fsub qword ptr [esp + 068h] // 00c5207d
        fxch st(1) // 00c52081
        fstp qword ptr [edi] // 00c52083
        fxch st(1) // 00c52085
        fstp qword ptr [edi + 8] // 00c52087
        fstp qword ptr [edi + 010h] // 00c5208a
        fld qword ptr [edi + 8] // 00c5208d
        fld qword ptr [edi] // 00c52090
        fld qword ptr [edi + 010h] // 00c52092
        fld st(1) // 00c52095
        fmulp st(2), st(0) // 00c52097
        fld st(2) // 00c52099
        fmulp st(3), st(0) // 00c5209b
        fxch st(1) // 00c5209d
        faddp st(2), st(0) // 00c5209f
        fmul st(0), st(0) // 00c520a1
        faddp st(1), st(0) // 00c520a3
        push dword ptr [esp+120] // Borrowed CRT access.
        call sqrt_shim // 00c520a5
        fld1  // 00c520aa
        fdivrp st(1), st(0) // 00c520ac
        fld qword ptr [edi] // 00c520ae
        fmul st(0), st(1) // 00c520b0
        fstp qword ptr [edi] // 00c520b2
        fld st(0) // 00c520b4
        fmul qword ptr [edi + 8] // 00c520b6
        fstp qword ptr [edi + 8] // 00c520b9
        fmul qword ptr [edi + 010h] // 00c520bc
        fstp qword ptr [edi + 010h] // 00c520bf
        mov dword ptr [esi + 01bch], 0 // 00c520c2
        fld qword ptr constant_00d7a340 // 00c520cc
        fstp qword ptr [esp + 010h] // 00c520d2
    l_00c520d6:
        mov ecx, dword ptr [esi] // 00c520d6
        mov eax, dword ptr [ecx] // 00c520d8
        mov eax, dword ptr [eax + 0ch] // 00c520da
        push ebx // 00c520dd
        push edi // 00c520de
        lea edx, [esp + 048h] // 00c520df
        push edx // 00c520e3
        call eax // 00c520e4
        fld qword ptr [edi] // 00c520e6
        mov ecx, dword ptr [esi + 4] // 00c520e8
        fchs  // 00c520eb
        fstp qword ptr [esp + 028h] // 00c520ed
        push ebp // 00c520f1
        fld qword ptr [edi + 8] // 00c520f2
        lea eax, [esp + 02ch] // 00c520f5
        fchs  // 00c520f9
        push eax // 00c520fb
        fstp qword ptr [esp + 038h] // 00c520fc
        lea eax, [esp + 060h] // 00c52100
        fld qword ptr [edi + 010h] // 00c52104
        push eax // 00c52107
        fchs  // 00c52108
        fstp qword ptr [esp + 044h] // 00c5210a
        mov edx, dword ptr [ecx] // 00c5210e
        mov edx, dword ptr [edx + 0ch] // 00c52110
        call edx // 00c52113
        fld qword ptr [esp + 040h] // 00c52115
        fsub qword ptr [esp + 058h] // 00c52119
        fld qword ptr [esp + 048h] // 00c5211d
        fsub qword ptr [esp + 060h] // 00c52121
        fld qword ptr [esp + 050h] // 00c52125
        fsub qword ptr [esp + 068h] // 00c52129
        fld qword ptr [edi + 8] // 00c5212d
        fmul st(0), st(2) // 00c52130
        fld st(3) // 00c52132
        fmul qword ptr [edi] // 00c52134
        faddp st(1), st(0) // 00c52136
        fld qword ptr [edi + 010h] // 00c52138
        fmul st(0), st(2) // 00c5213b
        faddp st(1), st(0) // 00c5213d
        fst qword ptr [esp + 028h] // 00c5213f
        fld qword ptr [esi + 070h] // 00c52143
        fchs  // 00c52146
        fcomip st(0), st(1) // 00c52148
        ja l_00c52504 // 00c5214a
        fld qword ptr [esp + 010h] // 00c52150
        fadd qword ptr constant_00d7a290 // 00c52154
        fcomip st(0), st(1) // 00c5215a
        ja l_00c529f6 // 00c5215c
        xor ecx, ecx // 00c52162
        fstp st(0) // 00c52164
        cmp dword ptr [esi + 01b8h], ecx // 00c52166
        jle l_00c521b6 // 00c5216c
        fld qword ptr constant_00d7a2d0 // 00c5216e
        lea edx, [esi + 0a8h] // 00c52174
    l_00c5217a:
        fld qword ptr [edx - 010h] // 00c5217a
        fsub st(0), st(4) // 00c5217d
        fld qword ptr [edx - 8] // 00c5217f
        fsub st(0), st(4) // 00c52182
        fld qword ptr [edx] // 00c52184
        fsub st(0), st(4) // 00c52186
        fld st(2) // 00c52188
        fmulp st(3), st(0) // 00c5218a
        fld st(1) // 00c5218c
        fmulp st(2), st(0) // 00c5218e
        fxch st(2) // 00c52190
        faddp st(1), st(0) // 00c52192
        fld st(1) // 00c52194
        fmulp st(2), st(0) // 00c52196
        faddp st(1), st(0) // 00c52198
        fxch st(1) // 00c5219a
        fcomi st(0), st(1) // 00c5219c
        fstp st(1) // 00c5219e
        ja l_00c529e8 // 00c521a0
        add ecx, 1 // 00c521a6
        add edx, 018h // 00c521a9
        cmp ecx, dword ptr [esi + 01b8h] // 00c521ac
        jl l_00c5217a // 00c521b2
        fstp st(0) // 00c521b4
    l_00c521b6:
        mov eax, dword ptr [esi + 01b8h] // 00c521b6
        fld qword ptr [esp + 040h] // 00c521bc
        lea eax, [eax + eax*2] // 00c521c0
        fstp qword ptr [esi + eax*8 + 0f8h] // 00c521c3
        lea eax, [esi + eax*8 + 0f8h] // 00c521ca
        fld qword ptr [esp + 048h] // 00c521d1
        fstp qword ptr [eax + 8] // 00c521d5
        fld qword ptr [esp + 050h] // 00c521d8
        fstp qword ptr [eax + 010h] // 00c521dc
        mov eax, dword ptr [esi + 01b8h] // 00c521df
        fld qword ptr [esp + 058h] // 00c521e5
        lea ecx, [eax + eax*2] // 00c521e9
        fstp qword ptr [esi + ecx*8 + 0158h] // 00c521ec
        lea eax, [esi + ecx*8 + 0158h] // 00c521f3
        fld qword ptr [esp + 060h] // 00c521fa
        fstp qword ptr [eax + 8] // 00c521fe
        fld qword ptr [esp + 068h] // 00c52201
        fstp qword ptr [eax + 010h] // 00c52205
        mov eax, dword ptr [esi + 01b8h] // 00c52208
        lea edx, [eax + eax*2] // 00c5220e
        fxch st(2) // 00c52211
        lea ecx, [esi + edx*8 + 098h] // 00c52213
        add eax, 1 // 00c5221a
        mov dword ptr [esi + 01b8h], eax // 00c5221d
        fstp qword ptr [ecx] // 00c52223
        fstp qword ptr [ecx + 8] // 00c52225
        fstp qword ptr [ecx + 010h] // 00c52228
        mov ecx,esi // Explicit interface to the complete R139 reducer.
        call reduce_native_dyn_convex_simplex_00c3cc30 // 00c5222b
        cmp dword ptr [esi + 01b8h], 4 // 00c52230
        je l_00c52a0a // 00c52237
        fld qword ptr [edi + 8] // 00c5223d
        fld qword ptr [edi] // 00c52240
        fld qword ptr [edi + 010h] // 00c52242
        fld st(1) // 00c52245
        fmulp st(2), st(0) // 00c52247
        fld st(2) // 00c52249
        fmulp st(3), st(0) // 00c5224b
        fxch st(1) // 00c5224d
        faddp st(2), st(0) // 00c5224f
        fmul st(0), st(0) // 00c52251
        faddp st(1), st(0) // 00c52253
        fld qword ptr constant_00d7a2b8 // 00c52255
        fcomip st(0), st(1) // 00c5225b
        fstp st(0) // 00c5225d
        ja l_00c52516 // 00c5225f
        fld qword ptr [edi + 8] // 00c52265
        fld qword ptr [edi] // 00c52268
        fld qword ptr [edi + 010h] // 00c5226a
        fld st(1) // 00c5226d
        fmulp st(2), st(0) // 00c5226f
        fld st(2) // 00c52271
        fmulp st(3), st(0) // 00c52273
        fxch st(1) // 00c52275
        faddp st(2), st(0) // 00c52277
        fmul st(0), st(0) // 00c52279
        faddp st(1), st(0) // 00c5227b
        push dword ptr [esp+120] // Borrowed CRT access.
        call sqrt_shim // 00c5227d
        fld1  // 00c52282
        fdivrp st(1), st(0) // 00c52284
        fld st(0) // 00c52286
        fmul qword ptr [edi] // 00c52288
        fstp qword ptr [edi] // 00c5228a
        fld st(0) // 00c5228c
        fmul qword ptr [edi + 8] // 00c5228e
        fstp qword ptr [edi + 8] // 00c52291
        fmul qword ptr [edi + 010h] // 00c52294
        fstp qword ptr [edi + 010h] // 00c52297
        fld qword ptr [esi + 0a0h] // 00c5229a
        fmul qword ptr [edi + 8] // 00c522a0
        fld qword ptr [esi + 098h] // 00c522a3
        fmul qword ptr [edi] // 00c522a9
        faddp st(1), st(0) // 00c522ab
        fld qword ptr [esi + 0a8h] // 00c522ad
        fmul qword ptr [edi + 010h] // 00c522b3
        faddp st(1), st(0) // 00c522b6
        fld qword ptr [esp + 010h] // 00c522b8
        fcomip st(0), st(1) // 00c522bc
        ja l_00c522c6 // 00c522be
        fstp qword ptr [esp + 010h] // 00c522c0
        jmp l_00c522c8 // 00c522c4
    l_00c522c6:
        fstp st(0) // 00c522c6
    l_00c522c8:
        add dword ptr [esi + 01bch], 1 // 00c522c8
        cmp dword ptr [esi + 01bch], 0ah // 00c522cf
        jl l_00c520d6 // 00c522d6
        mov eax, dword ptr [esi + 01b8h] // 00c522dc
        sub eax, 1 // 00c522e2
        je l_00c52fb9 // 00c522e5
        sub eax, 1 // 00c522eb
        je l_00c52eba // 00c522ee
        sub eax, 1 // 00c522f4
        jne l_00c53001 // 00c522f7
        fld qword ptr [esi + 0b0h] // 00c522fd
        fsub qword ptr [esi + 098h] // 00c52303
        fst qword ptr [esp + 058h] // 00c52309
        fld qword ptr [esi + 0b8h] // 00c5230d
        fsub qword ptr [esi + 0a0h] // 00c52313
        fst qword ptr [esp + 060h] // 00c52319
        fld qword ptr [esi + 0c0h] // 00c5231d
        fsub qword ptr [esi + 0a8h] // 00c52323
        fst qword ptr [esp + 068h] // 00c52329
        fld qword ptr [esi + 0c8h] // 00c5232d
        fsub qword ptr [esi + 098h] // 00c52333
        fst qword ptr [esp + 010h] // 00c52339
        fld qword ptr [esi + 0d0h] // 00c5233d
        fsub qword ptr [esi + 0a0h] // 00c52343
        fst qword ptr [esp + 018h] // 00c52349
        fld qword ptr [esi + 0d8h] // 00c5234d
        fsub qword ptr [esi + 0a8h] // 00c52353
        fstp qword ptr [esp + 020h] // 00c52359
        fld qword ptr [esi + 098h] // 00c5235d
        fchs  // 00c52363
        fld qword ptr [esi + 0a0h] // 00c52365
        fchs  // 00c5236b
        fld qword ptr [esi + 0a8h] // 00c5236d
        fchs  // 00c52373
        fxch st(4) // 00c52375
        fmul st(0), st(7) // 00c52377
        fxch st(3) // 00c52379
        fmul st(0), st(6) // 00c5237b
        faddp st(3), st(0) // 00c5237d
        fld qword ptr [esp + 020h] // 00c5237f
        fmul st(0), st(5) // 00c52383
        faddp st(3), st(0) // 00c52385
        fxch st(2) // 00c52387
        fst qword ptr [esp + 028h] // 00c52389
        fld st(6) // 00c5238d
        fmulp st(7), st(0) // 00c5238f
        fld st(5) // 00c52391
        fmulp st(6), st(0) // 00c52393
        fxch st(6) // 00c52395
        faddp st(5), st(0) // 00c52397
        fld st(3) // 00c52399
        fmulp st(4), st(0) // 00c5239b
        fxch st(4) // 00c5239d
        faddp st(3), st(0) // 00c5239f
        fld qword ptr [esp + 010h] // 00c523a1
        fmul st(0), st(0) // 00c523a5
        fld qword ptr [esp + 018h] // 00c523a7
        fmul st(0), st(0) // 00c523ab
        faddp st(1), st(0) // 00c523ad
        fld qword ptr [esp + 020h] // 00c523af
        fmul st(0), st(0) // 00c523b3
        faddp st(1), st(0) // 00c523b5
        fmul st(0), st(3) // 00c523b7
        fld qword ptr [esp + 028h] // 00c523b9
        fmul st(0), st(0) // 00c523bd
        fsubp st(1), st(0) // 00c523bf
        fst qword ptr [esp + 028h] // 00c523c1
        fld qword ptr constant_00d7a2c0 // 00c523c5
        fxch st(1) // 00c523cb
        fcomi st(0), st(1) // 00c523cd
        fstp st(1) // 00c523cf
        jbe l_00c52df5 // 00c523d1
        fld qword ptr constant_00d7a2b8 // 00c523d7
        fcomip st(0), st(1) // 00c523dd
        fstp st(0) // 00c523df
        jbe l_00c52df7 // 00c523e1
        fstp st(3) // 00c523e7
        mov eax, 1 // 00c523e9
        fstp st(3) // 00c523ee
        fstp st(0) // 00c523f0
        fstp st(0) // 00c523f2
        fstp st(0) // 00c523f4
        fld qword ptr [esi + 098h] // 00c523f6
        fsub qword ptr [esi + 0b0h] // 00c523fc
        fld qword ptr [esi + 0a0h] // 00c52402
        fsub qword ptr [esi + 0b8h] // 00c52408
        fld qword ptr [esi + 0a8h] // 00c5240e
        fsub qword ptr [esi + 0c0h] // 00c52414
        fld qword ptr [esi + 0a0h] // 00c5241a
        fmul st(0), st(2) // 00c52420
        fld qword ptr [esi + 098h] // 00c52422
        fmul st(0), st(4) // 00c52428
        faddp st(1), st(0) // 00c5242a
        fld qword ptr [esi + 0a8h] // 00c5242c
        fmul st(0), st(2) // 00c52432
        faddp st(1), st(0) // 00c52434
        fld st(3) // 00c52436
        fmulp st(4), st(0) // 00c52438
        fld st(2) // 00c5243a
        fmulp st(3), st(0) // 00c5243c
        fxch st(3) // 00c5243e
        faddp st(2), st(0) // 00c52440
        fmul st(0), st(0) // 00c52442
        faddp st(1), st(0) // 00c52444
        fdivp st(1), st(0) // 00c52446
        fld qword ptr [esi + 0110h] // 00c52448
        fsub qword ptr [esi + 0f8h] // 00c5244e
        fld qword ptr [esi + 0118h] // 00c52454
        fsub qword ptr [esi + 0100h] // 00c5245a
        fld qword ptr [esi + 0120h] // 00c52460
        fsub qword ptr [esi + 0108h] // 00c52466
        fxch st(2) // 00c5246c
        fmul st(0), st(3) // 00c5246e
        fxch st(1) // 00c52470
        fmul st(0), st(3) // 00c52472
        fxch st(2) // 00c52474
        fmul st(0), st(3) // 00c52476
        fxch st(1) // 00c52478
        fadd qword ptr [esi + 0f8h] // 00c5247a
        fld qword ptr [esi + 0100h] // 00c52480
        faddp st(3), st(0) // 00c52486
        fld qword ptr [esi + 0108h] // 00c52488
        faddp st(2), st(0) // 00c5248e
        fstp qword ptr [esi + 01c0h] // 00c52490
        fxch st(1) // 00c52496
        fstp qword ptr [esi + 01c8h] // 00c52498
        fstp qword ptr [esi + 01d0h] // 00c5249e
        fld qword ptr [esi + 0170h] // 00c524a4
        fsub qword ptr [esi + 0158h] // 00c524aa
        fld qword ptr [esi + 0178h] // 00c524b0
        fsub qword ptr [esi + 0160h] // 00c524b6
        fld qword ptr [esi + 0180h] // 00c524bc
        fsub qword ptr [esi + 0168h] // 00c524c2
        fxch st(2) // 00c524c8
        fmul st(0), st(3) // 00c524ca
        fxch st(1) // 00c524cc
        fmul st(0), st(3) // 00c524ce
        fxch st(2) // 00c524d0
        fmulp st(3), st(0) // 00c524d2
        fadd qword ptr [esi + 0158h] // 00c524d4
        fld qword ptr [esi + 0160h] // 00c524da
        faddp st(2), st(0) // 00c524e0
        fld qword ptr [esi + 0168h] // 00c524e2
        faddp st(3), st(0) // 00c524e8
        fstp qword ptr [esi + 01d8h] // 00c524ea
        fstp qword ptr [esi + 01e0h] // 00c524f0
        fstp qword ptr [esi + 01e8h] // 00c524f6
        pop edi // 00c524fc
        pop esi // 00c524fd
        pop ebp // 00c524fe
        pop ebx // 00c524ff
        add esp, 064h // 00c52500
        ret 4 // 00c52503
    l_00c52504:
        fstp st(0) // 00c52504
        xor eax, eax // 00c52506
        fstp st(2) // 00c52508
        fstp st(0) // 00c5250a
        fstp st(0) // 00c5250c
        pop edi // 00c5250e
        pop esi // 00c5250f
        pop ebp // 00c52510
        pop ebx // 00c52511
        add esp, 064h // 00c52512
        ret 4 // 00c52515
    l_00c52516:
        fld qword ptr constant_00d7a2c0 // 00c52516
        fld qword ptr [esp + 028h] // 00c5251c
        fcomip st(0), st(1) // 00c52520
        fstp st(0) // 00c52522
        ja l_00c52a0a // 00c52524
        mov eax, dword ptr [esi + 01b8h] // 00c5252a
        sub eax, 1 // 00c52530
        je l_00c52fb9 // 00c52533
        sub eax, 1 // 00c52539
        je l_00c528d5 // 00c5253c
        sub eax, 1 // 00c52542
        jne l_00c53001 // 00c52545
        fld qword ptr [esi + 0b0h] // 00c5254b
        fsub qword ptr [esi + 098h] // 00c52551
        fst qword ptr [esp + 058h] // 00c52557
        fld qword ptr [esi + 0b8h] // 00c5255b
        fsub qword ptr [esi + 0a0h] // 00c52561
        fst qword ptr [esp + 060h] // 00c52567
        fld qword ptr [esi + 0c0h] // 00c5256b
        fsub qword ptr [esi + 0a8h] // 00c52571
        fst qword ptr [esp + 068h] // 00c52577
        fld qword ptr [esi + 0c8h] // 00c5257b
        fsub qword ptr [esi + 098h] // 00c52581
        fst qword ptr [esp + 010h] // 00c52587
        fld qword ptr [esi + 0d0h] // 00c5258b
        fsub qword ptr [esi + 0a0h] // 00c52591
        fst qword ptr [esp + 018h] // 00c52597
        fld qword ptr [esi + 0d8h] // 00c5259b
        fsub qword ptr [esi + 0a8h] // 00c525a1
        fstp qword ptr [esp + 020h] // 00c525a7
        fld qword ptr [esi + 098h] // 00c525ab
        fchs  // 00c525b1
        fld qword ptr [esi + 0a0h] // 00c525b3
        fchs  // 00c525b9
        fld qword ptr [esi + 0a8h] // 00c525bb
        fchs  // 00c525c1
        fxch st(4) // 00c525c3
        fmul st(0), st(7) // 00c525c5
        fxch st(3) // 00c525c7
        fmul st(0), st(6) // 00c525c9
        faddp st(3), st(0) // 00c525cb
        fld qword ptr [esp + 020h] // 00c525cd
        fmul st(0), st(5) // 00c525d1
        faddp st(3), st(0) // 00c525d3
        fxch st(2) // 00c525d5
        fst qword ptr [esp + 028h] // 00c525d7
        fld st(6) // 00c525db
        fmulp st(7), st(0) // 00c525dd
        fld st(5) // 00c525df
        fmulp st(6), st(0) // 00c525e1
        fxch st(6) // 00c525e3
        faddp st(5), st(0) // 00c525e5
        fld st(3) // 00c525e7
        fmulp st(4), st(0) // 00c525e9
        fxch st(4) // 00c525eb
        faddp st(3), st(0) // 00c525ed
        fld qword ptr [esp + 010h] // 00c525ef
        fmul st(0), st(0) // 00c525f3
        fld qword ptr [esp + 018h] // 00c525f5
        fmul st(0), st(0) // 00c525f9
        faddp st(1), st(0) // 00c525fb
        fld qword ptr [esp + 020h] // 00c525fd
        fmul st(0), st(0) // 00c52601
        faddp st(1), st(0) // 00c52603
        fmul st(0), st(3) // 00c52605
        fld qword ptr [esp + 028h] // 00c52607
        fmul st(0), st(0) // 00c5260b
        fsubp st(1), st(0) // 00c5260d
        fst qword ptr [esp + 028h] // 00c5260f
        fld qword ptr constant_00d7a2c0 // 00c52613
        fxch st(1) // 00c52619
        fcomi st(0), st(1) // 00c5261b
        fstp st(1) // 00c5261d
        jbe l_00c52752 // 00c5261f
        fld qword ptr constant_00d7a2b8 // 00c52625
        fcomip st(0), st(1) // 00c5262b
        fstp st(0) // 00c5262d
        jbe l_00c52754 // 00c5262f
        fstp st(3) // 00c52635
        mov eax, 1 // 00c52637
        fstp st(3) // 00c5263c
        fstp st(0) // 00c5263e
        fstp st(0) // 00c52640
        fstp st(0) // 00c52642
        fld qword ptr [esi + 098h] // 00c52644
        fsub qword ptr [esi + 0b0h] // 00c5264a
        fld qword ptr [esi + 0a0h] // 00c52650
        fsub qword ptr [esi + 0b8h] // 00c52656
        fld qword ptr [esi + 0a8h] // 00c5265c
        fsub qword ptr [esi + 0c0h] // 00c52662
        fld qword ptr [esi + 0a0h] // 00c52668
        fmul st(0), st(2) // 00c5266e
        fld st(3) // 00c52670
        fmul qword ptr [esi + 098h] // 00c52672
        faddp st(1), st(0) // 00c52678
        fld qword ptr [esi + 0a8h] // 00c5267a
        fmul st(0), st(2) // 00c52680
        faddp st(1), st(0) // 00c52682
        fld st(3) // 00c52684
        fmulp st(4), st(0) // 00c52686
        fld st(2) // 00c52688
        fmulp st(3), st(0) // 00c5268a
        fxch st(3) // 00c5268c
        faddp st(2), st(0) // 00c5268e
        fmul st(0), st(0) // 00c52690
        faddp st(1), st(0) // 00c52692
        fdivp st(1), st(0) // 00c52694
        fld qword ptr [esi + 0110h] // 00c52696
        fsub qword ptr [esi + 0f8h] // 00c5269c
        fld qword ptr [esi + 0118h] // 00c526a2
        fsub qword ptr [esi + 0100h] // 00c526a8
        fld qword ptr [esi + 0120h] // 00c526ae
        fsub qword ptr [esi + 0108h] // 00c526b4
        fxch st(2) // 00c526ba
        fmul st(0), st(3) // 00c526bc
        fld st(3) // 00c526be
        fmulp st(2), st(0) // 00c526c0
        fld st(3) // 00c526c2
        fmulp st(3), st(0) // 00c526c4
        fadd qword ptr [esi + 0f8h] // 00c526c6
        fld qword ptr [esi + 0100h] // 00c526cc
        faddp st(2), st(0) // 00c526d2
        fld qword ptr [esi + 0108h] // 00c526d4
        faddp st(3), st(0) // 00c526da
        fstp qword ptr [esi + 01c0h] // 00c526dc
        fstp qword ptr [esi + 01c8h] // 00c526e2
        fstp qword ptr [esi + 01d0h] // 00c526e8
        fld qword ptr [esi + 0170h] // 00c526ee
        fsub qword ptr [esi + 0158h] // 00c526f4
        fld qword ptr [esi + 0178h] // 00c526fa
        fsub qword ptr [esi + 0160h] // 00c52700
        fld qword ptr [esi + 0180h] // 00c52706
        fsub qword ptr [esi + 0168h] // 00c5270c
        fxch st(2) // 00c52712
        fmul st(0), st(3) // 00c52714
        fld st(3) // 00c52716
        fmulp st(2), st(0) // 00c52718
        fxch st(3) // 00c5271a
        fmulp st(2), st(0) // 00c5271c
        fxch st(2) // 00c5271e
        fadd qword ptr [esi + 0158h] // 00c52720
        fld qword ptr [esi + 0160h] // 00c52726
        faddp st(3), st(0) // 00c5272c
        fld qword ptr [esi + 0168h] // 00c5272e
        faddp st(2), st(0) // 00c52734
        fstp qword ptr [esi + 01d8h] // 00c52736
        fxch st(1) // 00c5273c
        fstp qword ptr [esi + 01e0h] // 00c5273e
        fstp qword ptr [esi + 01e8h] // 00c52744
        pop edi // 00c5274a
        pop esi // 00c5274b
        pop ebp // 00c5274c
        pop ebx // 00c5274d
        add esp, 064h // 00c5274e
        ret 4 // 00c52751
    l_00c52752:
        fstp st(0) // 00c52752
    l_00c52754:
        fld st(3) // 00c52754
        fmul qword ptr [esp + 058h] // 00c52756
        fld st(1) // 00c5275a
        fmul qword ptr [esp + 060h] // 00c5275c
        faddp st(1), st(0) // 00c52760
        fld st(2) // 00c52762
        fmul qword ptr [esp + 068h] // 00c52764
        faddp st(1), st(0) // 00c52768
        fxch st(4) // 00c5276a
        fmul qword ptr [esp + 010h] // 00c5276c
        fxch st(1) // 00c52770
        fmul qword ptr [esp + 018h] // 00c52772
        faddp st(1), st(0) // 00c52776
        fxch st(1) // 00c52778
        fmul qword ptr [esp + 020h] // 00c5277a
        faddp st(1), st(0) // 00c5277e
        fmul st(0), st(1) // 00c52780
        fld st(2) // 00c52782
        fmul st(0), st(4) // 00c52784
        fsubp st(1), st(0) // 00c52786
        fdiv qword ptr [esp + 028h] // 00c52788
        fld st(0) // 00c5278c
        fmulp st(4), st(0) // 00c5278e
        fxch st(2) // 00c52790
        fsubrp st(3), st(0) // 00c52792
        fdivp st(2), st(0) // 00c52794
        fld qword ptr [esi + 0128h] // 00c52796
        fsub qword ptr [esi + 0f8h] // 00c5279c
        fld qword ptr [esi + 0130h] // 00c527a2
        fsub qword ptr [esi + 0100h] // 00c527a8
        fld qword ptr [esi + 0138h] // 00c527ae
        fsub qword ptr [esi + 0108h] // 00c527b4
        fxch st(2) // 00c527ba
        fmul st(0), st(3) // 00c527bc
        fld st(3) // 00c527be
        fmulp st(2), st(0) // 00c527c0
        fld st(3) // 00c527c2
        fmulp st(3), st(0) // 00c527c4
        fld qword ptr [esi + 0110h] // 00c527c6
        fsub qword ptr [esi + 0f8h] // 00c527cc
        fld qword ptr [esi + 0118h] // 00c527d2
        fsub qword ptr [esi + 0100h] // 00c527d8
        fld qword ptr [esi + 0120h] // 00c527de
        fsub qword ptr [esi + 0108h] // 00c527e4
        fxch st(2) // 00c527ea
        fmul st(0), st(7) // 00c527ec
        fstp qword ptr [esp + 058h] // 00c527ee
        fmul st(0), st(6) // 00c527f2
        fld st(6) // 00c527f4
        fmulp st(2), st(0) // 00c527f6
        fld qword ptr [esp + 058h] // 00c527f8
        fadd qword ptr [esi + 0f8h] // 00c527fc
        fstp qword ptr [esp + 058h] // 00c52802
        fadd qword ptr [esi + 0100h] // 00c52806
        fld qword ptr [esi + 0108h] // 00c5280c
        faddp st(2), st(0) // 00c52812
        fld qword ptr [esp + 058h] // 00c52814
        faddp st(3), st(0) // 00c52818
        faddp st(3), st(0) // 00c5281a
        faddp st(3), st(0) // 00c5281c
        fstp qword ptr [esi + 01c0h] // 00c5281e
        fstp qword ptr [esi + 01c8h] // 00c52824
        fstp qword ptr [esi + 01d0h] // 00c5282a
        fld qword ptr [esi + 0188h] // 00c52830
        fsub qword ptr [esi + 0158h] // 00c52836
        fld qword ptr [esi + 0190h] // 00c5283c
        fsub qword ptr [esi + 0160h] // 00c52842
        fld qword ptr [esi + 0198h] // 00c52848
        fsub qword ptr [esi + 0168h] // 00c5284e
        fxch st(2) // 00c52854
        fmul st(0), st(3) // 00c52856
        fld st(3) // 00c52858
        fmulp st(2), st(0) // 00c5285a
        fxch st(3) // 00c5285c
        fmulp st(2), st(0) // 00c5285e
        fld qword ptr [esi + 0170h] // 00c52860
        fsub qword ptr [esi + 0158h] // 00c52866
        fld qword ptr [esi + 0178h] // 00c5286c
        fsub qword ptr [esi + 0160h] // 00c52872
        fld qword ptr [esi + 0180h] // 00c52878
        fsub qword ptr [esi + 0168h] // 00c5287e
        fxch st(2) // 00c52884
        mov eax, 1 // 00c52886
        fmul st(0), st(6) // 00c5288b
        fld st(6) // 00c5288d
        fmulp st(2), st(0) // 00c5288f
        fxch st(6) // 00c52891
        fmulp st(2), st(0) // 00c52893
        fxch st(5) // 00c52895
        fadd qword ptr [esi + 0158h] // 00c52897
        fld qword ptr [esi + 0160h] // 00c5289d
        faddp st(6), st(0) // 00c528a3
        fld qword ptr [esi + 0168h] // 00c528a5
        faddp st(2), st(0) // 00c528ab
        faddp st(4), st(0) // 00c528ad
        fxch st(4) // 00c528af
        faddp st(1), st(0) // 00c528b1
        fxch st(3) // 00c528b3
        faddp st(1), st(0) // 00c528b5
        fxch st(1) // 00c528b7
        fstp qword ptr [esi + 01d8h] // 00c528b9
        fxch st(1) // 00c528bf
        fstp qword ptr [esi + 01e0h] // 00c528c1
        fstp qword ptr [esi + 01e8h] // 00c528c7
        pop edi // 00c528cd
        pop esi // 00c528ce
        pop ebp // 00c528cf
        pop ebx // 00c528d0
        add esp, 064h // 00c528d1
        ret 4 // 00c528d4
    l_00c528d5:
        fld qword ptr [esi + 098h] // 00c528d5
        fsub qword ptr [esi + 0b0h] // 00c528db
        fld qword ptr [esi + 0a0h] // 00c528e1
        fsub qword ptr [esi + 0b8h] // 00c528e7
        fld qword ptr [esi + 0a8h] // 00c528ed
        fsub qword ptr [esi + 0c0h] // 00c528f3
        fld qword ptr [esi + 0a0h] // 00c528f9
        fmul st(0), st(2) // 00c528ff
        fld st(3) // 00c52901
        fmul qword ptr [esi + 098h] // 00c52903
        faddp st(1), st(0) // 00c52909
        fld qword ptr [esi + 0a8h] // 00c5290b
        fmul st(0), st(2) // 00c52911
        faddp st(1), st(0) // 00c52913
        fld st(3) // 00c52915
        fmulp st(4), st(0) // 00c52917
        fld st(2) // 00c52919
        fmulp st(3), st(0) // 00c5291b
        fxch st(3) // 00c5291d
        faddp st(2), st(0) // 00c5291f
        fmul st(0), st(0) // 00c52921
        faddp st(1), st(0) // 00c52923
        fdivp st(1), st(0) // 00c52925
        fld qword ptr [esi + 0110h] // 00c52927
        fsub qword ptr [esi + 0f8h] // 00c5292d
        fld qword ptr [esi + 0118h] // 00c52933
        fsub qword ptr [esi + 0100h] // 00c52939
        fld qword ptr [esi + 0120h] // 00c5293f
        fsub qword ptr [esi + 0108h] // 00c52945
        fxch st(2) // 00c5294b
        fmul st(0), st(3) // 00c5294d
        fld st(3) // 00c5294f
        fmulp st(2), st(0) // 00c52951
        fld st(3) // 00c52953
        fmulp st(3), st(0) // 00c52955
        fadd qword ptr [esi + 0f8h] // 00c52957
        fld qword ptr [esi + 0100h] // 00c5295d
        faddp st(2), st(0) // 00c52963
        fld qword ptr [esi + 0108h] // 00c52965
        faddp st(3), st(0) // 00c5296b
        fstp qword ptr [esi + 01c0h] // 00c5296d
        fstp qword ptr [esi + 01c8h] // 00c52973
        fstp qword ptr [esi + 01d0h] // 00c52979
        fld qword ptr [esi + 0170h] // 00c5297f
        fsub qword ptr [esi + 0158h] // 00c52985
        fld qword ptr [esi + 0178h] // 00c5298b
        fsub qword ptr [esi + 0160h] // 00c52991
        fld qword ptr [esi + 0180h] // 00c52997
        fsub qword ptr [esi + 0168h] // 00c5299d
        fxch st(2) // 00c529a3
        fmul st(0), st(3) // 00c529a5
        fld st(3) // 00c529a7
        fmulp st(2), st(0) // 00c529a9
        fxch st(3) // 00c529ab
        fmulp st(2), st(0) // 00c529ad
        fld qword ptr [esi + 0158h] // 00c529af
        faddp st(3), st(0) // 00c529b5
        fadd qword ptr [esi + 0160h] // 00c529b7
        fld qword ptr [esi + 0168h] // 00c529bd
    l_00c529c3:
        faddp st(2), st(0) // 00c529c3
        fxch st(2) // 00c529c5
        fstp qword ptr [esi + 01d8h] // 00c529c7
        fxch st(1) // 00c529cd
    l_00c529cf:
        fstp qword ptr [esi + 01e0h] // 00c529cf
        mov eax, 1 // 00c529d5
        fstp qword ptr [esi + 01e8h] // 00c529da
        pop edi // 00c529e0
        pop esi // 00c529e1
        pop ebp // 00c529e2
        pop ebx // 00c529e3
        add esp, 064h // 00c529e4
        ret 4 // 00c529e7
    l_00c529e8:
        fstp st(3) // 00c529e8
        fstp st(1) // 00c529ea
        fstp st(0) // 00c529ec
        fstp st(0) // 00c529ee
        fld qword ptr [esp + 028h] // 00c529f0
        jmp l_00c529fc // 00c529f4
    l_00c529f6:
        fstp st(3) // 00c529f6
        fstp st(1) // 00c529f8
        fstp st(0) // 00c529fa
    l_00c529fc:
        fld qword ptr constant_00d7a338 // 00c529fc
        fxch st(1) // 00c52a02
        fcomip st(0), st(1) // 00c52a04
        fstp st(0) // 00c52a06
        jbe l_00c52a17 // 00c52a08
    l_00c52a0a:
        mov eax, 2 // 00c52a0a
        pop edi // 00c52a0f
        pop esi // 00c52a10
        pop ebp // 00c52a11
        pop ebx // 00c52a12
        add esp, 064h // 00c52a13
        ret 4 // 00c52a16
    l_00c52a17:
        mov eax, dword ptr [esi + 01b8h] // 00c52a17
        sub eax, 1 // 00c52a1d
        je l_00c52fb9 // 00c52a20
        sub eax, 1 // 00c52a26
        je l_00c52d9e // 00c52a29
        sub eax, 1 // 00c52a2f
        jne l_00c53001 // 00c52a32
        fld qword ptr [esi + 0b0h] // 00c52a38
        fsub qword ptr [esi + 098h] // 00c52a3e
        fst qword ptr [esp + 058h] // 00c52a44
        fld qword ptr [esi + 0b8h] // 00c52a48
        fsub qword ptr [esi + 0a0h] // 00c52a4e
        fst qword ptr [esp + 060h] // 00c52a54
        fld qword ptr [esi + 0c0h] // 00c52a58
        fsub qword ptr [esi + 0a8h] // 00c52a5e
        fst qword ptr [esp + 068h] // 00c52a64
        fld qword ptr [esi + 0c8h] // 00c52a68
        fsub qword ptr [esi + 098h] // 00c52a6e
        fst qword ptr [esp + 010h] // 00c52a74
        fld qword ptr [esi + 0d0h] // 00c52a78
        fsub qword ptr [esi + 0a0h] // 00c52a7e
        fst qword ptr [esp + 018h] // 00c52a84
        fld qword ptr [esi + 0d8h] // 00c52a88
        fsub qword ptr [esi + 0a8h] // 00c52a8e
        fstp qword ptr [esp + 020h] // 00c52a94
        fld qword ptr [esi + 098h] // 00c52a98
        fchs  // 00c52a9e
        fld qword ptr [esi + 0a0h] // 00c52aa0
        fchs  // 00c52aa6
        fld qword ptr [esi + 0a8h] // 00c52aa8
        fchs  // 00c52aae
        fxch st(4) // 00c52ab0
        fmul st(0), st(7) // 00c52ab2
        fxch st(3) // 00c52ab4
        fmul st(0), st(6) // 00c52ab6
        faddp st(3), st(0) // 00c52ab8
        fld qword ptr [esp + 020h] // 00c52aba
        fmul st(0), st(5) // 00c52abe
        faddp st(3), st(0) // 00c52ac0
        fxch st(2) // 00c52ac2
        fst qword ptr [esp + 028h] // 00c52ac4
        fld st(6) // 00c52ac8
        fmulp st(7), st(0) // 00c52aca
        fld st(5) // 00c52acc
        fmulp st(6), st(0) // 00c52ace
        fxch st(6) // 00c52ad0
        faddp st(5), st(0) // 00c52ad2
        fld st(3) // 00c52ad4
        fmulp st(4), st(0) // 00c52ad6
        fxch st(4) // 00c52ad8
        faddp st(3), st(0) // 00c52ada
        fld qword ptr [esp + 010h] // 00c52adc
        fmul st(0), st(0) // 00c52ae0
        fld qword ptr [esp + 018h] // 00c52ae2
        fmul st(0), st(0) // 00c52ae6
        faddp st(1), st(0) // 00c52ae8
        fld qword ptr [esp + 020h] // 00c52aea
        fmul st(0), st(0) // 00c52aee
        faddp st(1), st(0) // 00c52af0
        fmul st(0), st(3) // 00c52af2
        fld qword ptr [esp + 028h] // 00c52af4
        fmul st(0), st(0) // 00c52af8
        fsubp st(1), st(0) // 00c52afa
        fst qword ptr [esp + 028h] // 00c52afc
        fld qword ptr constant_00d7a2c0 // 00c52b00
        fxch st(1) // 00c52b06
        fcomi st(0), st(1) // 00c52b08
        fstp st(1) // 00c52b0a
        jbe l_00c52c3f // 00c52b0c
        fld qword ptr constant_00d7a2b8 // 00c52b12
        fcomip st(0), st(1) // 00c52b18
        fstp st(0) // 00c52b1a
        jbe l_00c52c41 // 00c52b1c
        fstp st(3) // 00c52b22
        fstp st(3) // 00c52b24
        fstp st(0) // 00c52b26
        fstp st(0) // 00c52b28
        fstp st(0) // 00c52b2a
        fld qword ptr [esi + 098h] // 00c52b2c
        fsub qword ptr [esi + 0b0h] // 00c52b32
        fld qword ptr [esi + 0a0h] // 00c52b38
        fsub qword ptr [esi + 0b8h] // 00c52b3e
        fld qword ptr [esi + 0a8h] // 00c52b44
        fsub qword ptr [esi + 0c0h] // 00c52b4a
        fld qword ptr [esi + 0a0h] // 00c52b50
        fmul st(0), st(2) // 00c52b56
        fld st(3) // 00c52b58
        fmul qword ptr [esi + 098h] // 00c52b5a
        faddp st(1), st(0) // 00c52b60
        fld qword ptr [esi + 0a8h] // 00c52b62
        fmul st(0), st(2) // 00c52b68
        faddp st(1), st(0) // 00c52b6a
        fld st(3) // 00c52b6c
        fmulp st(4), st(0) // 00c52b6e
        fld st(2) // 00c52b70
        fmulp st(3), st(0) // 00c52b72
        fxch st(3) // 00c52b74
        faddp st(2), st(0) // 00c52b76
        fmul st(0), st(0) // 00c52b78
        faddp st(1), st(0) // 00c52b7a
        fdivp st(1), st(0) // 00c52b7c
    l_00c52b7e:
        fld qword ptr [esi + 0110h] // 00c52b7e
        mov eax, 1 // 00c52b84
        fsub qword ptr [esi + 0f8h] // 00c52b89
        fld qword ptr [esi + 0118h] // 00c52b8f
        fsub qword ptr [esi + 0100h] // 00c52b95
        fld qword ptr [esi + 0120h] // 00c52b9b
        fsub qword ptr [esi + 0108h] // 00c52ba1
        fxch st(2) // 00c52ba7
        fmul st(0), st(3) // 00c52ba9
        fxch st(1) // 00c52bab
        fmul st(0), st(3) // 00c52bad
        fxch st(2) // 00c52baf
        fmul st(0), st(3) // 00c52bb1
        fxch st(1) // 00c52bb3
        fadd qword ptr [esi + 0f8h] // 00c52bb5
        fld qword ptr [esi + 0100h] // 00c52bbb
        faddp st(3), st(0) // 00c52bc1
        fld qword ptr [esi + 0108h] // 00c52bc3
        faddp st(2), st(0) // 00c52bc9
        fstp qword ptr [esi + 01c0h] // 00c52bcb
        fxch st(1) // 00c52bd1
        fstp qword ptr [esi + 01c8h] // 00c52bd3
        fstp qword ptr [esi + 01d0h] // 00c52bd9
        fld qword ptr [esi + 0170h] // 00c52bdf
        fsub qword ptr [esi + 0158h] // 00c52be5
        fld qword ptr [esi + 0178h] // 00c52beb
        fsub qword ptr [esi + 0160h] // 00c52bf1
        fld qword ptr [esi + 0180h] // 00c52bf7
        fsub qword ptr [esi + 0168h] // 00c52bfd
        fxch st(2) // 00c52c03
        fmul st(0), st(3) // 00c52c05
        fxch st(1) // 00c52c07
        fmul st(0), st(3) // 00c52c09
        fxch st(2) // 00c52c0b
        fmulp st(3), st(0) // 00c52c0d
        fadd qword ptr [esi + 0158h] // 00c52c0f
        fld qword ptr [esi + 0160h] // 00c52c15
        faddp st(2), st(0) // 00c52c1b
        fld qword ptr [esi + 0168h] // 00c52c1d
        faddp st(3), st(0) // 00c52c23
        fstp qword ptr [esi + 01d8h] // 00c52c25
        fstp qword ptr [esi + 01e0h] // 00c52c2b
        fstp qword ptr [esi + 01e8h] // 00c52c31
        pop edi // 00c52c37
        pop esi // 00c52c38
        pop ebp // 00c52c39
        pop ebx // 00c52c3a
        add esp, 064h // 00c52c3b
        ret 4 // 00c52c3e
    l_00c52c3f:
        fstp st(0) // 00c52c3f
    l_00c52c41:
        fld st(3) // 00c52c41
        fmul qword ptr [esp + 058h] // 00c52c43
        fld st(1) // 00c52c47
        fmul qword ptr [esp + 060h] // 00c52c49
        faddp st(1), st(0) // 00c52c4d
        fld st(2) // 00c52c4f
        fmul qword ptr [esp + 068h] // 00c52c51
        faddp st(1), st(0) // 00c52c55
        fxch st(4) // 00c52c57
        fmul qword ptr [esp + 010h] // 00c52c59
        fxch st(1) // 00c52c5d
        fmul qword ptr [esp + 018h] // 00c52c5f
        faddp st(1), st(0) // 00c52c63
        fxch st(1) // 00c52c65
        fmul qword ptr [esp + 020h] // 00c52c67
        faddp st(1), st(0) // 00c52c6b
        fmul st(0), st(1) // 00c52c6d
        fld st(3) // 00c52c6f
        fmul st(0), st(3) // 00c52c71
        fsubp st(1), st(0) // 00c52c73
        fdiv qword ptr [esp + 028h] // 00c52c75
        fmul st(3), st(0) // 00c52c79
        fxch st(2) // 00c52c7b
        fsubrp st(3), st(0) // 00c52c7d
        fdivp st(2), st(0) // 00c52c7f
        fld qword ptr [esi + 0128h] // 00c52c81
        fsub qword ptr [esi + 0f8h] // 00c52c87
        fld qword ptr [esi + 0130h] // 00c52c8d
        fsub qword ptr [esi + 0100h] // 00c52c93
        fld qword ptr [esi + 0138h] // 00c52c99
        fsub qword ptr [esi + 0108h] // 00c52c9f
        fxch st(2) // 00c52ca5
        fmul st(0), st(3) // 00c52ca7
        fxch st(1) // 00c52ca9
        fmul st(0), st(3) // 00c52cab
        fxch st(2) // 00c52cad
        fmul st(0), st(3) // 00c52caf
        fld qword ptr [esi + 0110h] // 00c52cb1
        fsub qword ptr [esi + 0f8h] // 00c52cb7
        fld qword ptr [esi + 0118h] // 00c52cbd
        fsub qword ptr [esi + 0100h] // 00c52cc3
        fld qword ptr [esi + 0120h] // 00c52cc9
        fsub qword ptr [esi + 0108h] // 00c52ccf
        fxch st(2) // 00c52cd5
        fmul st(0), st(7) // 00c52cd7
        fxch st(1) // 00c52cd9
        fmul st(0), st(7) // 00c52cdb
        fxch st(2) // 00c52cdd
        fmul st(0), st(7) // 00c52cdf
        fxch st(1) // 00c52ce1
        fadd qword ptr [esi + 0f8h] // 00c52ce3
        fstp qword ptr [esp + 058h] // 00c52ce9
        fld qword ptr [esi + 0100h] // 00c52ced
        faddp st(2), st(0) // 00c52cf3
        fadd qword ptr [esi + 0108h] // 00c52cf5
        fld qword ptr [esp + 058h] // 00c52cfb
    l_00c52cff:
        faddp st(4), st(0) // 00c52cff
        fxch st(1) // 00c52d01
        faddp st(4), st(0) // 00c52d03
        faddp st(1), st(0) // 00c52d05
        fxch st(1) // 00c52d07
        fstp qword ptr [esi + 01c0h] // 00c52d09
        fxch st(1) // 00c52d0f
        fstp qword ptr [esi + 01c8h] // 00c52d11
        fstp qword ptr [esi + 01d0h] // 00c52d17
        fld qword ptr [esi + 0188h] // 00c52d1d
        fsub qword ptr [esi + 0158h] // 00c52d23
        fld qword ptr [esi + 0190h] // 00c52d29
        fsub qword ptr [esi + 0160h] // 00c52d2f
        fld qword ptr [esi + 0198h] // 00c52d35
        fsub qword ptr [esi + 0168h] // 00c52d3b
        fxch st(2) // 00c52d41
        fmul st(0), st(3) // 00c52d43
        fxch st(1) // 00c52d45
        fmul st(0), st(3) // 00c52d47
        fxch st(2) // 00c52d49
        fmulp st(3), st(0) // 00c52d4b
        fld qword ptr [esi + 0170h] // 00c52d4d
        fsub qword ptr [esi + 0158h] // 00c52d53
        fld qword ptr [esi + 0178h] // 00c52d59
        fsub qword ptr [esi + 0160h] // 00c52d5f
        fld qword ptr [esi + 0180h] // 00c52d65
        fsub qword ptr [esi + 0168h] // 00c52d6b
        fxch st(2) // 00c52d71
        fmul st(0), st(6) // 00c52d73
        fxch st(1) // 00c52d75
        fmul st(0), st(6) // 00c52d77
        fxch st(2) // 00c52d79
        fmulp st(6), st(0) // 00c52d7b
        fadd qword ptr [esi + 0158h] // 00c52d7d
        fld qword ptr [esi + 0160h] // 00c52d83
        faddp st(2), st(0) // 00c52d89
        fld qword ptr [esi + 0168h] // 00c52d8b
        faddp st(6), st(0) // 00c52d91
        faddp st(2), st(0) // 00c52d93
        faddp st(2), st(0) // 00c52d95
        fxch st(3) // 00c52d97
        jmp l_00c529c3 // 00c52d99
    l_00c52d9e:
        fld qword ptr [esi + 098h] // 00c52d9e
        fsub qword ptr [esi + 0b0h] // 00c52da4
        fld qword ptr [esi + 0a0h] // 00c52daa
        fsub qword ptr [esi + 0b8h] // 00c52db0
        fld qword ptr [esi + 0a8h] // 00c52db6
        fsub qword ptr [esi + 0c0h] // 00c52dbc
        fld qword ptr [esi + 0a0h] // 00c52dc2
        fmul st(0), st(2) // 00c52dc8
        fld st(3) // 00c52dca
        fmul qword ptr [esi + 098h] // 00c52dcc
        faddp st(1), st(0) // 00c52dd2
        fld qword ptr [esi + 0a8h] // 00c52dd4
        fmul st(0), st(2) // 00c52dda
        faddp st(1), st(0) // 00c52ddc
        fld st(2) // 00c52dde
        fmulp st(3), st(0) // 00c52de0
        fld st(3) // 00c52de2
        fmulp st(4), st(0) // 00c52de4
        fxch st(2) // 00c52de6
        faddp st(3), st(0) // 00c52de8
        fmul st(0), st(0) // 00c52dea
        faddp st(2), st(0) // 00c52dec
        fdivrp st(1), st(0) // 00c52dee
        jmp l_00c52b7e // 00c52df0
    l_00c52df5:
        fstp st(0) // 00c52df5
    l_00c52df7:
        fld st(3) // 00c52df7
        fmul qword ptr [esp + 058h] // 00c52df9
        fld st(1) // 00c52dfd
        fmul qword ptr [esp + 060h] // 00c52dff
        faddp st(1), st(0) // 00c52e03
        fld st(2) // 00c52e05
        fmul qword ptr [esp + 068h] // 00c52e07
        faddp st(1), st(0) // 00c52e0b
        fxch st(4) // 00c52e0d
        fmul qword ptr [esp + 010h] // 00c52e0f
        fxch st(1) // 00c52e13
        fmul qword ptr [esp + 018h] // 00c52e15
        faddp st(1), st(0) // 00c52e19
        fxch st(1) // 00c52e1b
        fmul qword ptr [esp + 020h] // 00c52e1d
        faddp st(1), st(0) // 00c52e21
        fmul st(0), st(1) // 00c52e23
        fld st(2) // 00c52e25
        fmul st(0), st(4) // 00c52e27
        fsubp st(1), st(0) // 00c52e29
        fdiv qword ptr [esp + 028h] // 00c52e2b
        fmul st(3), st(0) // 00c52e2f
        fxch st(2) // 00c52e31
        fsubrp st(3), st(0) // 00c52e33
        fdivp st(2), st(0) // 00c52e35
        fld qword ptr [esi + 0128h] // 00c52e37
        fsub qword ptr [esi + 0f8h] // 00c52e3d
        fld qword ptr [esi + 0130h] // 00c52e43
        fsub qword ptr [esi + 0100h] // 00c52e49
        fld qword ptr [esi + 0138h] // 00c52e4f
        fsub qword ptr [esi + 0108h] // 00c52e55
        fxch st(2) // 00c52e5b
        fmul st(0), st(3) // 00c52e5d
        fxch st(1) // 00c52e5f
        fmul st(0), st(3) // 00c52e61
        fxch st(2) // 00c52e63
        fmul st(0), st(3) // 00c52e65
        fld qword ptr [esi + 0110h] // 00c52e67
        fsub qword ptr [esi + 0f8h] // 00c52e6d
        fld qword ptr [esi + 0118h] // 00c52e73
        fsub qword ptr [esi + 0100h] // 00c52e79
        fld qword ptr [esi + 0120h] // 00c52e7f
        fsub qword ptr [esi + 0108h] // 00c52e85
        fxch st(2) // 00c52e8b
        fmul st(0), st(7) // 00c52e8d
        fxch st(1) // 00c52e8f
        fmul st(0), st(7) // 00c52e91
        fxch st(2) // 00c52e93
        fmul st(0), st(7) // 00c52e95
        fstp qword ptr [esp + 068h] // 00c52e97
        fadd qword ptr [esi + 0f8h] // 00c52e9b
        fld qword ptr [esi + 0100h] // 00c52ea1
        faddp st(2), st(0) // 00c52ea7
        fld qword ptr [esi + 0108h] // 00c52ea9
        fadd qword ptr [esp + 068h] // 00c52eaf
        fxch st(1) // 00c52eb3
        jmp l_00c52cff // 00c52eb5
    l_00c52eba:
        fld qword ptr [esi + 098h] // 00c52eba
        fsub qword ptr [esi + 0b0h] // 00c52ec0
        fld qword ptr [esi + 0a0h] // 00c52ec6
        fsub qword ptr [esi + 0b8h] // 00c52ecc
        fld qword ptr [esi + 0a8h] // 00c52ed2
        fsub qword ptr [esi + 0c0h] // 00c52ed8
        fld qword ptr [esi + 0a0h] // 00c52ede
        fmul st(0), st(2) // 00c52ee4
        fld st(3) // 00c52ee6
        fmul qword ptr [esi + 098h] // 00c52ee8
        faddp st(1), st(0) // 00c52eee
        fld qword ptr [esi + 0a8h] // 00c52ef0
        fmul st(0), st(2) // 00c52ef6
        faddp st(1), st(0) // 00c52ef8
        fld st(3) // 00c52efa
        fmulp st(4), st(0) // 00c52efc
        fld st(2) // 00c52efe
        fmulp st(3), st(0) // 00c52f00
        fxch st(3) // 00c52f02
        faddp st(2), st(0) // 00c52f04
        fmul st(0), st(0) // 00c52f06
        faddp st(1), st(0) // 00c52f08
        fdivp st(1), st(0) // 00c52f0a
        fld qword ptr [esi + 0110h] // 00c52f0c
        fsub qword ptr [esi + 0f8h] // 00c52f12
        fld qword ptr [esi + 0118h] // 00c52f18
        fsub qword ptr [esi + 0100h] // 00c52f1e
        fld qword ptr [esi + 0120h] // 00c52f24
        fsub qword ptr [esi + 0108h] // 00c52f2a
        fxch st(2) // 00c52f30
        fmul st(0), st(3) // 00c52f32
        fxch st(1) // 00c52f34
        fmul st(0), st(3) // 00c52f36
        fxch st(2) // 00c52f38
        fmul st(0), st(3) // 00c52f3a
        fxch st(1) // 00c52f3c
        fadd qword ptr [esi + 0f8h] // 00c52f3e
        fld qword ptr [esi + 0100h] // 00c52f44
        faddp st(3), st(0) // 00c52f4a
        fld qword ptr [esi + 0108h] // 00c52f4c
        faddp st(2), st(0) // 00c52f52
        fstp qword ptr [esi + 01c0h] // 00c52f54
        fxch st(1) // 00c52f5a
        fstp qword ptr [esi + 01c8h] // 00c52f5c
        fstp qword ptr [esi + 01d0h] // 00c52f62
        fld qword ptr [esi + 0170h] // 00c52f68
        fsub qword ptr [esi + 0158h] // 00c52f6e
        fld qword ptr [esi + 0178h] // 00c52f74
        fsub qword ptr [esi + 0160h] // 00c52f7a
        fld qword ptr [esi + 0180h] // 00c52f80
        fsub qword ptr [esi + 0168h] // 00c52f86
        fxch st(2) // 00c52f8c
        fmul st(0), st(3) // 00c52f8e
        fxch st(1) // 00c52f90
        fmul st(0), st(3) // 00c52f92
        fxch st(2) // 00c52f94
        fmulp st(3), st(0) // 00c52f96
        fadd qword ptr [esi + 0158h] // 00c52f98
        fld qword ptr [esi + 0160h] // 00c52f9e
        faddp st(2), st(0) // 00c52fa4
        fld qword ptr [esi + 0168h] // 00c52fa6
        faddp st(3), st(0) // 00c52fac
        fstp qword ptr [esi + 01d8h] // 00c52fae
        jmp l_00c529cf // 00c52fb4
    l_00c52fb9:
        fld qword ptr [esi + 0f8h] // 00c52fb9
        fstp qword ptr [esi + 01c0h] // 00c52fbf
        fld qword ptr [esi + 0100h] // 00c52fc5
        fstp qword ptr [esi + 01c8h] // 00c52fcb
        fld qword ptr [esi + 0108h] // 00c52fd1
        fstp qword ptr [esi + 01d0h] // 00c52fd7
        fld qword ptr [esi + 0158h] // 00c52fdd
        fstp qword ptr [esi + 01d8h] // 00c52fe3
        fld qword ptr [esi + 0160h] // 00c52fe9
        fstp qword ptr [esi + 01e0h] // 00c52fef
        fld qword ptr [esi + 0168h] // 00c52ff5
        fstp qword ptr [esi + 01e8h] // 00c52ffb
    l_00c53001:
        pop edi // 00c53001
        pop esi // 00c53002
        pop ebp // 00c53003
        mov eax, 1 // 00c53004
        pop ebx // 00c53009
        add esp, 064h // 00c5300a
        ret 4 // 00c5300d
    }
}
// Complete recovered instruction schedule; address comments are PE starts.
__declspec(naked) void intersect_kernel(){
    __asm {
        sub esp, 030h // 00c53010
        push esi // 00c53013
        mov esi, dword ptr [esp + 038h] // 00c53014
        mov dword ptr [esi + 4], edx // 00c53018
        push edi // 00c5301b
        mov edi, dword ptr [esp + 044h] // 00c5301c
        mov dword ptr [esi], edi // 00c53020
        fld dword ptr [edi + 038h] // 00c53022
        fmul dword ptr [ecx + 0ch] // 00c53025
        fld dword ptr [edi + 034h] // 00c53028
        fmul dword ptr [ecx] // 00c5302b
        faddp st(1), st(0) // 00c5302d
        fld dword ptr [edi + 03ch] // 00c5302f
        fmul dword ptr [ecx + 018h] // 00c53032
        faddp st(1), st(0) // 00c53035
        fstp dword ptr [esi + 0ch] // 00c53037
        fld dword ptr [ecx + 4] // 00c5303a
        fmul dword ptr [edi + 034h] // 00c5303d
        fld dword ptr [ecx + 010h] // 00c53040
        fmul dword ptr [edi + 038h] // 00c53043
        faddp st(1), st(0) // 00c53046
        fld dword ptr [edi + 03ch] // 00c53048
        fmul dword ptr [ecx + 01ch] // 00c5304b
        faddp st(1), st(0) // 00c5304e
        fstp dword ptr [esi + 010h] // 00c53050
        fld dword ptr [ecx + 8] // 00c53053
        fmul dword ptr [edi + 034h] // 00c53056
        fld dword ptr [ecx + 014h] // 00c53059
        fmul dword ptr [edi + 038h] // 00c5305c
        faddp st(1), st(0) // 00c5305f
        fld dword ptr [edi + 03ch] // 00c53061
        fmul dword ptr [ecx + 020h] // 00c53064
        faddp st(1), st(0) // 00c53067
        fstp dword ptr [esi + 014h] // 00c53069
        fld dword ptr [edi + 040h] // 00c5306c
        fmul dword ptr [ecx] // 00c5306f
        fld dword ptr [edi + 044h] // 00c53071
        fmul dword ptr [ecx + 0ch] // 00c53074
        faddp st(1), st(0) // 00c53077
        fld dword ptr [ecx + 018h] // 00c53079
        fmul dword ptr [edi + 048h] // 00c5307c
        faddp st(1), st(0) // 00c5307f
        fstp dword ptr [esi + 018h] // 00c53081
        fld dword ptr [edi + 044h] // 00c53084
        fmul dword ptr [ecx + 010h] // 00c53087
        fld dword ptr [edi + 040h] // 00c5308a
        fmul dword ptr [ecx + 4] // 00c5308d
        faddp st(1), st(0) // 00c53090
        fld dword ptr [edi + 048h] // 00c53092
        fmul dword ptr [ecx + 01ch] // 00c53095
        faddp st(1), st(0) // 00c53098
        fstp dword ptr [esi + 01ch] // 00c5309a
        fld dword ptr [edi + 044h] // 00c5309d
        fmul dword ptr [ecx + 014h] // 00c530a0
        fld dword ptr [edi + 040h] // 00c530a3
        fmul dword ptr [ecx + 8] // 00c530a6
        faddp st(1), st(0) // 00c530a9
        fld dword ptr [edi + 048h] // 00c530ab
        fmul dword ptr [ecx + 020h] // 00c530ae
        faddp st(1), st(0) // 00c530b1
        fstp dword ptr [esi + 020h] // 00c530b3
        fld dword ptr [edi + 04ch] // 00c530b6
        fmul dword ptr [ecx] // 00c530b9
        fld dword ptr [edi + 050h] // 00c530bb
        fmul dword ptr [ecx + 0ch] // 00c530be
        faddp st(1), st(0) // 00c530c1
        fld dword ptr [ecx + 018h] // 00c530c3
        fmul dword ptr [edi + 054h] // 00c530c6
        faddp st(1), st(0) // 00c530c9
        fstp dword ptr [esi + 024h] // 00c530cb
        fld dword ptr [edi + 050h] // 00c530ce
        fmul dword ptr [ecx + 010h] // 00c530d1
        fld dword ptr [edi + 04ch] // 00c530d4
        fmul dword ptr [ecx + 4] // 00c530d7
        faddp st(1), st(0) // 00c530da
        fld dword ptr [edi + 054h] // 00c530dc
        fmul dword ptr [ecx + 01ch] // 00c530df
        faddp st(1), st(0) // 00c530e2
        fstp dword ptr [esi + 028h] // 00c530e4
        fld dword ptr [edi + 050h] // 00c530e7
        fmul dword ptr [ecx + 014h] // 00c530ea
        fld dword ptr [edi + 04ch] // 00c530ed
        fmul dword ptr [ecx + 8] // 00c530f0
        faddp st(1), st(0) // 00c530f3
        fld dword ptr [edi + 054h] // 00c530f5
        fmul dword ptr [ecx + 020h] // 00c530f8
        faddp st(1), st(0) // 00c530fb
        fstp dword ptr [esi + 02ch] // 00c530fd
        fld dword ptr [edi + 058h] // 00c53100
        fmul dword ptr [ecx] // 00c53103
        fld dword ptr [edi + 05ch] // 00c53105
        fmul dword ptr [ecx + 0ch] // 00c53108
        faddp st(1), st(0) // 00c5310b
        fld dword ptr [ecx + 018h] // 00c5310d
        fmul dword ptr [edi + 060h] // 00c53110
        faddp st(1), st(0) // 00c53113
        fadd dword ptr [ecx + 024h] // 00c53115
        fstp dword ptr [esi + 030h] // 00c53118
        fld dword ptr [edi + 05ch] // 00c5311b
        fmul dword ptr [ecx + 010h] // 00c5311e
        fld dword ptr [edi + 058h] // 00c53121
        fmul dword ptr [ecx + 4] // 00c53124
        faddp st(1), st(0) // 00c53127
        fld dword ptr [edi + 060h] // 00c53129
        fmul dword ptr [ecx + 01ch] // 00c5312c
        faddp st(1), st(0) // 00c5312f
        fadd dword ptr [ecx + 028h] // 00c53131
        fstp dword ptr [esi + 034h] // 00c53134
        fld dword ptr [edi + 05ch] // 00c53137
        fmul dword ptr [ecx + 014h] // 00c5313a
        fld dword ptr [edi + 058h] // 00c5313d
        fmul dword ptr [ecx + 8] // 00c53140
        faddp st(1), st(0) // 00c53143
        fld dword ptr [edi + 060h] // 00c53145
        fmul dword ptr [ecx + 020h] // 00c53148
        faddp st(1), st(0) // 00c5314b
        fadd dword ptr [ecx + 02ch] // 00c5314d
        fstp dword ptr [esi + 038h] // 00c53150
        fld dword ptr [edx + 038h] // 00c53153
        fmul dword ptr [eax + 0ch] // 00c53156
        fld dword ptr [edx + 034h] // 00c53159
        fmul dword ptr [eax] // 00c5315c
        faddp st(1), st(0) // 00c5315e
        fld dword ptr [edx + 03ch] // 00c53160
        fmul dword ptr [eax + 018h] // 00c53163
        faddp st(1), st(0) // 00c53166
        fstp dword ptr [esi + 03ch] // 00c53168
        fld dword ptr [eax + 4] // 00c5316b
        fmul dword ptr [edx + 034h] // 00c5316e
        fld dword ptr [eax + 010h] // 00c53171
        fmul dword ptr [edx + 038h] // 00c53174
        faddp st(1), st(0) // 00c53177
        fld dword ptr [edx + 03ch] // 00c53179
        fmul dword ptr [eax + 01ch] // 00c5317c
        faddp st(1), st(0) // 00c5317f
        fstp dword ptr [esi + 040h] // 00c53181
        fld dword ptr [eax + 8] // 00c53184
        fmul dword ptr [edx + 034h] // 00c53187
        fld dword ptr [eax + 014h] // 00c5318a
        fmul dword ptr [edx + 038h] // 00c5318d
        faddp st(1), st(0) // 00c53190
        fld dword ptr [edx + 03ch] // 00c53192
        fmul dword ptr [eax + 020h] // 00c53195
        faddp st(1), st(0) // 00c53198
        fstp dword ptr [esi + 044h] // 00c5319a
        fld dword ptr [edx + 040h] // 00c5319d
        fmul dword ptr [eax] // 00c531a0
        fld dword ptr [edx + 044h] // 00c531a2
        fmul dword ptr [eax + 0ch] // 00c531a5
        faddp st(1), st(0) // 00c531a8
        fld dword ptr [eax + 018h] // 00c531aa
        fmul dword ptr [edx + 048h] // 00c531ad
        faddp st(1), st(0) // 00c531b0
        fstp dword ptr [esi + 048h] // 00c531b2
        fld dword ptr [edx + 044h] // 00c531b5
        fmul dword ptr [eax + 010h] // 00c531b8
        fld dword ptr [edx + 040h] // 00c531bb
        fmul dword ptr [eax + 4] // 00c531be
        faddp st(1), st(0) // 00c531c1
        fld dword ptr [edx + 048h] // 00c531c3
        fmul dword ptr [eax + 01ch] // 00c531c6
        faddp st(1), st(0) // 00c531c9
        fstp dword ptr [esi + 04ch] // 00c531cb
        mov ecx, esi // 00c531ce
        fld dword ptr [edx + 044h] // 00c531d0
        fmul dword ptr [eax + 014h] // 00c531d3
        fld dword ptr [edx + 040h] // 00c531d6
        fmul dword ptr [eax + 8] // 00c531d9
        faddp st(1), st(0) // 00c531dc
        fld dword ptr [edx + 048h] // 00c531de
        fmul dword ptr [eax + 020h] // 00c531e1
        faddp st(1), st(0) // 00c531e4
        fstp dword ptr [esi + 050h] // 00c531e6
        fld dword ptr [edx + 04ch] // 00c531e9
        fmul dword ptr [eax] // 00c531ec
        fld dword ptr [edx + 050h] // 00c531ee
        fmul dword ptr [eax + 0ch] // 00c531f1
        faddp st(1), st(0) // 00c531f4
        fld dword ptr [eax + 018h] // 00c531f6
        fmul dword ptr [edx + 054h] // 00c531f9
        faddp st(1), st(0) // 00c531fc
        fstp dword ptr [esi + 054h] // 00c531fe
        fld dword ptr [edx + 050h] // 00c53201
        fmul dword ptr [eax + 010h] // 00c53204
        fld dword ptr [edx + 04ch] // 00c53207
        fmul dword ptr [eax + 4] // 00c5320a
        faddp st(1), st(0) // 00c5320d
        fld dword ptr [edx + 054h] // 00c5320f
        fmul dword ptr [eax + 01ch] // 00c53212
        faddp st(1), st(0) // 00c53215
        fstp dword ptr [esi + 058h] // 00c53217
        fld dword ptr [edx + 050h] // 00c5321a
        fmul dword ptr [eax + 014h] // 00c5321d
        fld dword ptr [edx + 04ch] // 00c53220
        fmul dword ptr [eax + 8] // 00c53223
        faddp st(1), st(0) // 00c53226
        fld dword ptr [edx + 054h] // 00c53228
        fmul dword ptr [eax + 020h] // 00c5322b
        faddp st(1), st(0) // 00c5322e
        fstp dword ptr [esi + 05ch] // 00c53230
        fld dword ptr [edx + 058h] // 00c53233
        fmul dword ptr [eax] // 00c53236
        fld dword ptr [edx + 05ch] // 00c53238
        fmul dword ptr [eax + 0ch] // 00c5323b
        faddp st(1), st(0) // 00c5323e
        fld dword ptr [eax + 018h] // 00c53240
        fmul dword ptr [edx + 060h] // 00c53243
        faddp st(1), st(0) // 00c53246
        fadd dword ptr [eax + 024h] // 00c53248
        fstp dword ptr [esi + 060h] // 00c5324b
        fld dword ptr [edx + 05ch] // 00c5324e
        fmul dword ptr [eax + 010h] // 00c53251
        fld dword ptr [edx + 058h] // 00c53254
        fmul dword ptr [eax + 4] // 00c53257
        faddp st(1), st(0) // 00c5325a
        fld dword ptr [edx + 060h] // 00c5325c
        fmul dword ptr [eax + 01ch] // 00c5325f
        faddp st(1), st(0) // 00c53262
        fadd dword ptr [eax + 028h] // 00c53264
        fstp dword ptr [esi + 064h] // 00c53267
        fld dword ptr [edx + 05ch] // 00c5326a
        fmul dword ptr [eax + 014h] // 00c5326d
        fld dword ptr [edx + 058h] // 00c53270
        fmul dword ptr [eax + 8] // 00c53273
        faddp st(1), st(0) // 00c53276
        fld dword ptr [edx + 060h] // 00c53278
        fmul dword ptr [eax + 020h] // 00c5327b
        faddp st(1), st(0) // 00c5327e
        fadd dword ptr [eax + 02ch] // 00c53280
        mov eax, dword ptr [esp + 040h] // 00c53283
        fstp dword ptr [esi + 068h] // 00c53287
        mov dword ptr [esi + 078h], eax // 00c5328a
        fld dword ptr [esi + 030h] // 00c5328d
        fadd qword ptr constant_00d7a358 // 00c53290
        fstp dword ptr [esi + 030h] // 00c53296
        fld qword ptr constant_00d7a2f8 // 00c53299
        fstp qword ptr [esi + 070h] // 00c5329f
        push dword ptr [esp+72] // Borrowed CRT access.
        call search_kernel // 00c532a2
        test eax, eax // 00c532a7
        jne l_00c532b5 // 00c532a9
        xor al, al // 00c532ab
        pop edi // 00c532ad
        pop esi // 00c532ae
        add esp, 030h // 00c532af
        ret 16 // 00c532b2
    l_00c532b5:
        cmp eax, 1 // 00c532b5
        jne l_00c53326 // 00c532b8
        fld qword ptr [esi + 01d8h] // 00c532ba
        fsub qword ptr [esi + 01c0h] // 00c532c0
        fld qword ptr [esi + 01e0h] // 00c532c6
        fsub qword ptr [esi + 01c8h] // 00c532cc
        fld qword ptr [esi + 01e8h] // 00c532d2
        fsub qword ptr [esi + 01d0h] // 00c532d8
        fxch st(2) // 00c532de
        fstp qword ptr [esi + 080h] // 00c532e0
        fstp qword ptr [esi + 088h] // 00c532e6
        fstp qword ptr [esi + 090h] // 00c532ec
        fld qword ptr [esi + 088h] // 00c532f2
        fld qword ptr [esi + 080h] // 00c532f8
        fld qword ptr [esi + 090h] // 00c532fe
        fld st(1) // 00c53304
        fmulp st(2), st(0) // 00c53306
        fld st(2) // 00c53308
        fmulp st(3), st(0) // 00c5330a
        fxch st(1) // 00c5330c
        faddp st(2), st(0) // 00c5330e
        fmul st(0), st(0) // 00c53310
        faddp st(1), st(0) // 00c53312
        fld qword ptr constant_00d7a350 // 00c53314
        fxch st(1) // 00c5331a
        fcomip st(0), st(1) // 00c5331c
        fstp st(0) // 00c5331e
        ja l_00c535c3 // 00c53320
    l_00c53326:
        mov edi, esi // 00c53326
        call direction_kernel // 00c53328
        fld qword ptr [esi + 01c0h] // 00c5332d
        fsub qword ptr [esi + 01d8h] // 00c53333
        fld qword ptr [esi + 01c8h] // 00c53339
        fsub qword ptr [esi + 01e0h] // 00c5333f
        fld qword ptr [esi + 01d0h] // 00c53345
        fsub qword ptr [esi + 01e8h] // 00c5334b
        fld qword ptr [esi + 088h] // 00c53351
        fmulp st(2), st(0) // 00c53357
        fxch st(2) // 00c53359
        fmul qword ptr [esi + 080h] // 00c5335b
        faddp st(1), st(0) // 00c53361
        fld qword ptr [esi + 090h] // 00c53363
        fmulp st(2), st(0) // 00c53369
        faddp st(1), st(0) // 00c5336b
        fldz  // 00c5336d
        fxch st(1) // 00c5336f
        fcomi st(0), st(1) // 00c53371
        jbe l_00c53379 // 00c53373
        fstp st(1) // 00c53375
        jmp l_00c5337b // 00c53377
    l_00c53379:
        fstp st(0) // 00c53379
    l_00c5337b:
        fadd qword ptr constant_00d7a348 // 00c5337b
        mov ecx, esi // 00c53381
        fld st(0) // 00c53383
        fmul qword ptr [esi + 080h] // 00c53385
        fst qword ptr [esp + 8] // 00c5338b
        fld qword ptr [esi + 088h] // 00c5338f
        fmul st(0), st(2) // 00c53395
        fst qword ptr [esp + 010h] // 00c53397
        fld qword ptr [esi + 090h] // 00c5339b
        fmulp st(3), st(0) // 00c533a1
        fxch st(2) // 00c533a3
        fst qword ptr [esp + 018h] // 00c533a5
        fxch st(1) // 00c533a9
        fstp dword ptr [esp + 044h] // 00c533ab
        fld dword ptr [esp + 044h] // 00c533af
        fadd dword ptr [esi + 060h] // 00c533b3
        fstp dword ptr [esi + 060h] // 00c533b6
        fxch st(1) // 00c533b9
        fstp dword ptr [esp + 044h] // 00c533bb
        fld dword ptr [esp + 044h] // 00c533bf
        fadd dword ptr [esi + 064h] // 00c533c3
        fstp dword ptr [esi + 064h] // 00c533c6
        fstp dword ptr [esp + 044h] // 00c533c9
        fld dword ptr [esp + 044h] // 00c533cd
        fadd dword ptr [esi + 068h] // 00c533d1
        fstp dword ptr [esi + 068h] // 00c533d4
        fld qword ptr constant_00d7a278 // 00c533d7
        fstp qword ptr [esi + 070h] // 00c533dd
        push dword ptr [esp+72] // Borrowed CRT access.
        call search_kernel // 00c533e0
        fld qword ptr [esi + 01d8h] // 00c533e5
        fsub qword ptr [esi + 01c0h] // 00c533eb
        fld qword ptr [esi + 01e0h] // 00c533f1
        fsub qword ptr [esi + 01c8h] // 00c533f7
        fld qword ptr [esi + 01e8h] // 00c533fd
        fsub qword ptr [esi + 01d0h] // 00c53403
        fxch st(2) // 00c53409
        fstp qword ptr [esi + 080h] // 00c5340b
        fstp qword ptr [esi + 088h] // 00c53411
        fstp qword ptr [esi + 090h] // 00c53417
        fld qword ptr [esi + 01d8h] // 00c5341d
        fsub qword ptr [esp + 8] // 00c53423
        fstp qword ptr [esi + 01d8h] // 00c53427
        fld qword ptr [esi + 01e0h] // 00c5342d
        fsub qword ptr [esp + 010h] // 00c53433
        fstp qword ptr [esi + 01e0h] // 00c53437
        fld qword ptr [esi + 01e8h] // 00c5343d
        fsub qword ptr [esp + 018h] // 00c53443
        fstp qword ptr [esi + 01e8h] // 00c53447
        fld qword ptr [esi + 088h] // 00c5344d
        fld qword ptr [esi + 080h] // 00c53453
        fld qword ptr [esi + 090h] // 00c53459
        fld st(1) // 00c5345f
        fmulp st(2), st(0) // 00c53461
        fld st(2) // 00c53463
        fmulp st(3), st(0) // 00c53465
        fxch st(1) // 00c53467
        faddp st(2), st(0) // 00c53469
        fmul st(0), st(0) // 00c5346b
        faddp st(1), st(0) // 00c5346d
        push dword ptr [esp+72] // Borrowed CRT access.
        call sqrt_shim // 00c5346f
        fld1  // 00c53474
        fdivrp st(1), st(0) // 00c53476
        fld qword ptr [esi + 080h] // 00c53478
        fmul st(0), st(1) // 00c5347e
        fstp qword ptr [esi + 080h] // 00c53480
        fld st(0) // 00c53486
        fmul qword ptr [esi + 088h] // 00c53488
        fstp qword ptr [esi + 088h] // 00c5348e
        fmul qword ptr [esi + 090h] // 00c53494
        fstp qword ptr [esi + 090h] // 00c5349a
        fld qword ptr [esi + 01c0h] // 00c534a0
        fsub qword ptr [esi + 01d8h] // 00c534a6
        fld qword ptr [esi + 01c8h] // 00c534ac
        fsub qword ptr [esi + 01e0h] // 00c534b2
        fld qword ptr [esi + 01d0h] // 00c534b8
        fsub qword ptr [esi + 01e8h] // 00c534be
        fld qword ptr [esi + 088h] // 00c534c4
        fmulp st(2), st(0) // 00c534ca
        fxch st(2) // 00c534cc
        fmul qword ptr [esi + 080h] // 00c534ce
        faddp st(1), st(0) // 00c534d4
        fld qword ptr [esi + 090h] // 00c534d6
        fmulp st(2), st(0) // 00c534dc
        faddp st(1), st(0) // 00c534de
        fldz  // 00c534e0
        fxch st(1) // 00c534e2
        fcomi st(0), st(1) // 00c534e4
        jbe l_00c534ec // 00c534e6
        fstp st(1) // 00c534e8
        jmp l_00c534ee // 00c534ea
    l_00c534ec:
        fstp st(0) // 00c534ec
    l_00c534ee:
        fadd qword ptr constant_00d7a348 // 00c534ee
        mov ecx, esi // 00c534f4
        fld st(0) // 00c534f6
        fmul qword ptr [esi + 080h] // 00c534f8
        fst qword ptr [esp + 020h] // 00c534fe
        fld qword ptr [esi + 088h] // 00c53502
        fmul st(0), st(2) // 00c53508
        fst qword ptr [esp + 028h] // 00c5350a
        fld qword ptr [esi + 090h] // 00c5350e
        fmulp st(3), st(0) // 00c53514
        fxch st(2) // 00c53516
        fst qword ptr [esp + 030h] // 00c53518
        fxch st(1) // 00c5351c
        fsub qword ptr [esp + 8] // 00c5351e
        fstp dword ptr [esp + 044h] // 00c53522
        fld dword ptr [esp + 044h] // 00c53526
        fadd dword ptr [esi + 060h] // 00c5352a
        fstp dword ptr [esi + 060h] // 00c5352d
        fxch st(1) // 00c53530
        fsub qword ptr [esp + 010h] // 00c53532
        fstp dword ptr [esp + 044h] // 00c53536
        fld dword ptr [esp + 044h] // 00c5353a
        fadd dword ptr [esi + 064h] // 00c5353e
        fstp dword ptr [esi + 064h] // 00c53541
        fsub qword ptr [esp + 018h] // 00c53544
        fstp dword ptr [esp + 044h] // 00c53548
        fld dword ptr [esp + 044h] // 00c5354c
        fadd dword ptr [esi + 068h] // 00c53550
        fstp dword ptr [esi + 068h] // 00c53553
        push dword ptr [esp+72] // Borrowed CRT access.
        call search_kernel // 00c53556
        fld qword ptr [esi + 01d8h] // 00c5355b
        fsub qword ptr [esi + 01c0h] // 00c53561
        fld qword ptr [esi + 01e0h] // 00c53567
        fsub qword ptr [esi + 01c8h] // 00c5356d
        fld qword ptr [esi + 01e8h] // 00c53573
        fsub qword ptr [esi + 01d0h] // 00c53579
        fxch st(2) // 00c5357f
        fstp qword ptr [esi + 080h] // 00c53581
        fstp qword ptr [esi + 088h] // 00c53587
        fstp qword ptr [esi + 090h] // 00c5358d
        fld qword ptr [esi + 01d8h] // 00c53593
        fsub qword ptr [esp + 020h] // 00c53599
        fstp qword ptr [esi + 01d8h] // 00c5359d
        fld qword ptr [esi + 01e0h] // 00c535a3
        fsub qword ptr [esp + 028h] // 00c535a9
        fstp qword ptr [esi + 01e0h] // 00c535ad
        fld qword ptr [esi + 01e8h] // 00c535b3
        fsub qword ptr [esp + 030h] // 00c535b9
        fstp qword ptr [esi + 01e8h] // 00c535bd
    l_00c535c3:
        push dword ptr [esp+72] // Borrowed CRT access.
        call result_kernel // 00c535c3
        pop edi // 00c535c8
        mov al, 1 // 00c535c9
        pop esi // 00c535cb
        add esp, 030h // 00c535cc
        ret 16 // 00c535cf
    }
}
// Complete recovered instruction schedule; address comments are PE starts.
__declspec(naked) void dispatch_kernel(){
    __asm {
        push ebp // 00c535e0
        mov ebp, esp // 00c535e1
        and esp, 0fffffff8h // 00c535e3
        sub esp, 01f8h // 00c535e6
        mov edx, dword ptr [ebp + 8] // 00c535ec
        lea eax, [ecx + 8] // 00c535ef
        add ecx, 0278h // 00c535f2
        mov dword ptr [esp + 01f4h], ecx // 00c535f8
        mov ecx, dword ptr [ebp + 0ch] // 00c535ff
        push ecx // 00c53602
        mov ecx, dword ptr [ebp + 010h] // 00c53603
        mov dword ptr [esp + 01f4h], eax // 00c53606
        push edx // 00c5360d
        mov edx, dword ptr [ebp + 014h] // 00c5360e
        lea eax, [esp + 8] // 00c53611
        push eax // 00c53615
        mov eax, dword ptr [ebp + 018h] // 00c53616
        push dword ptr [ebp+1ch] // Borrowed CRT access.
        call intersect_shim // 00c53619
        mov esp, ebp // 00c5361e
        pop ebp // 00c53620
        ret 24 // 00c53621
    }
}
} // namespace
std::int32_t search_native_dyn_convex_00c51ef0(void* work,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;std::int32_t result;
    __asm {
        mov ecx,work
        push c
        call search_kernel
        mov result,eax
    }
    return result;
}
void select_native_dyn_convex_direction_00c51c20(void* work) noexcept {
    __asm {
        push edi
        mov edi,work
        call direction_kernel
        pop edi
    }
}
void project_native_dyn_convex_contact_00c48be0(void* work,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;
    __asm {
        push esi
        mov esi,work
        push c
        call result_kernel
        pop esi
    }
}
bool intersect_native_dyn_convex_00c53010(void* work,void* result,void* a,const float* ma,void* b,const float* mb,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;unsigned char answer;
    __asm {
        push c
        push a
        push result
        push work
        mov ecx,ma
        mov edx,b
        mov eax,mb
        call intersect_kernel
        mov answer,al
    }
    return answer!=0;
}
bool dispatch_native_dyn_general_convex_00c535e0(DynGeneralConvexIntersectStorage& storage,void* result,void* a,const float* ma,void* b,const float* mb,const CameraAxesCrtAccess& crt){
    const auto* c=&crt;auto* owner=&storage;unsigned char answer;
    __asm {
        push c
        push mb
        push b
        push ma
        push a
        push result
        mov ecx,owner
        call dispatch_kernel
        mov answer,al
    }
    return answer!=0;
}
static_assert(std::is_standard_layout_v<NativeDynGeneralConvexRuntime>);
NativeDynGeneralConvexRuntime::NativeDynGeneralConvexRuntime(const CameraAxesCrtAccess& crt) noexcept
    :methods_{reinterpret_cast<std::uintptr_t>(&intersect)},crt_(crt) {}
bool __fastcall NativeDynGeneralConvexRuntime::intersect(void* p,void*,void* result,void* a,const float* ma,void* b,const float* mb){
    auto& owner=*static_cast<DynGeneralConvexIntersectStorage*>(p);
    const auto& runtime=*static_cast<const NativeDynGeneralConvexRuntime*>(owner.vtable_00);
    return dispatch_native_dyn_general_convex_00c535e0(owner,result,a,ma,b,mb,runtime.crt_);
}
} // namespace bsp
