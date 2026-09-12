#include "bsp/native_particle_emission_spawn.hpp"
#include "bsp/native_particle_emission_state.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle emission spawn requires MSVC Win32.
#endif

namespace bsp {
static_assert(offsetof(NativeParticleEmissionSpawnAccess, floor_00bf85b0) == 0);
static_assert(offsetof(NativeParticleEmissionSpawnAccess, truncate_st0_00bf7420) == 4);
static_assert(offsetof(NativeParticleEmissionSpawnAccess, definition_virtual0c) == 8);
static_assert(offsetof(NativeParticleEmissionSpawnAccess, state) == 12);
// Original instruction kernels retain each x87 lifetime/spill and current
// actual-storage reload. One added local DWORD saves borrowed EDX access.
// MOV-only service loads add no floating-point operations or flag changes.

__declspec(naked) std::int32_t __fastcall spawn_native_particle_emission_00b04c80(
    NativeParticleEmitterContainer*, const NativeParticleEmissionSpawnAccess*,
    void*, const void*, float, float, float) {
    __asm {
        sub esp,028h // 00b04c80
        mov dword ptr [esp + 024h],edx // additional local only
        fld dword ptr [esp + 034h] // 00b04c83
        push edi // 00b04c87
        sub esp,08h // 00b04c88
        fstp qword ptr [esp] // 00b04c8b
        mov edi,ecx // 00b04c8e
        mov eax,dword ptr [esp + 030h] // borrowed access
        call dword ptr [eax + 00h] // 00b04c90
        fstp dword ptr [esp + 040h] // 00b04c95
        fld dword ptr [esp + 040h] // 00b04c99
        add esp,08h // 00b04c9d
        mov eax,dword ptr [esp + 028h] // borrowed access
        call dword ptr [eax + 04h] // 00b04ca0
        mov ecx,eax // 00b04ca5
        test ecx,ecx // 00b04ca7
        mov dword ptr [esp + 038h],ecx // 00b04ca9
        jg L_00b04cb8 // 00b04cad
        xor eax,eax // 00b04caf
        pop edi // 00b04cb1
        add esp,028h // 00b04cb2
        ret 014h // 00b04cb5
    L_00b04cb8:
        mov eax,dword ptr [edi + 010h] // 00b04cb8
        sub eax,dword ptr [edi + 01ch] // 00b04cbb
        cmp eax,ecx // 00b04cbe
        jge L_00b04cc6 // 00b04cc0
        mov dword ptr [esp + 038h],eax // 00b04cc2
    L_00b04cc6:
        fild dword ptr [esp + 038h] // 00b04cc6
        xor eax,eax // 00b04cca
        cmp dword ptr [esp + 038h],eax // 00b04ccc
        fld1 // 00b04cd0
        fdivrp st(1),st(0) // 00b04cd2
        mov dword ptr [esp + 08h],eax // 00b04cd4
        fstp dword ptr [esp + 04h] // 00b04cd8
        fld dword ptr [esp + 04h] // 00b04cdc
        fmul dword ptr [esp + 040h] // 00b04ce0
        mov dword ptr [esp + 040h],00h // 00b04ce4
        fstp dword ptr [esp + 0ch] // 00b04cec
        jle L_00b04da4 // 00b04cf0
        push ebx // 00b04cf6
        mov ebx,dword ptr [esp + 038h] // 00b04cf7
        push ebp // 00b04cfb
        push esi // 00b04cfc
        lea ecx,[ecx] // 00b04cfd
    L_00b04d00:
        mov ecx,dword ptr [edi + 01ch] // 00b04d00
        cmp ecx,dword ptr [edi + 010h] // 00b04d03
        jge L_00b04da1 // 00b04d06
        mov edx,dword ptr [edi + 0ch] // 00b04d0c
        movzx esi,word ptr [edx + ecx*08h] // 00b04d0f
        lea ebp,[eax + 01h] // 00b04d13
        imul esi,esi,06ch // 00b04d16
        add esi,dword ptr [edi + 014h] // 00b04d19
        mov dword ptr [esp + 040h],ebp // 00b04d1c
        push ecx // 00b04d20
        lea ecx,[esp + 02ch] // 00b04d21
        fild dword ptr [esp + 044h] // 00b04d25
        fmul dword ptr [esp + 014h] // 00b04d29
        fstp dword ptr [esp + 044h] // 00b04d2d
        fld dword ptr [esp + 044h] // 00b04d31
        fstp dword ptr [esp] // 00b04d35
        push eax // 00b04d38
        lea eax,[esp + 024h] // 00b04d39
        push eax // 00b04d3d
        push ecx // 00b04d3e
        mov ecx,ebx // 00b04d3f
        mov edx,dword ptr [esp + 044h] // borrowed access
        call generate_native_particle_spawn_vectors_00afdaf0 // 00b04d41
        fild dword ptr [esp + 014h] // 00b04d46
        mov ecx,dword ptr [edi + 08h] // 00b04d4a
        push ebx // 00b04d4d
        fmul dword ptr [esp + 01ch] // 00b04d4e
        lea edx,[esp + 020h] // 00b04d52
        push edx // 00b04d56
        mov edx,dword ptr [esp + 044h] // 00b04d57
        fadd dword ptr [esp + 050h] // 00b04d5b
        lea eax,[esp + 030h] // 00b04d5f
        push eax // 00b04d63
        push ecx // 00b04d64
        fstp dword ptr [esp + 050h] // 00b04d65
        push ecx // 00b04d69
        fld dword ptr [esp + 054h] // 00b04d6a
        mov ecx,esi // 00b04d6e
        fstp dword ptr [esp] // 00b04d70
        push edx // 00b04d73
        mov eax,dword ptr [esp + 04ch] // borrowed access
        mov edx,dword ptr [eax + 0ch] // borrowed concrete initializer access
        call initialize_native_particle_emission_state_00b0ca40 // 00b04d74
        fld dword ptr [ebx + 0b4h] // 00b04d79
        fmul dword ptr [esi + 05ch] // 00b04d7f
        mov eax,01h // 00b04d82
        add dword ptr [esp + 04ch],eax // 00b04d87
        mov dword ptr [esp + 014h],ebp // 00b04d8b
        fstp dword ptr [esi + 05ch] // 00b04d8f
        add dword ptr [edi + 01ch],eax // 00b04d92
        mov eax,ebp // 00b04d95
        cmp eax,dword ptr [esp + 044h] // 00b04d97
        jl L_00b04d00 // 00b04d9b
    L_00b04da1:
        pop esi // 00b04da1
        pop ebp // 00b04da2
        pop ebx // 00b04da3
    L_00b04da4:
        mov eax,dword ptr [esp + 030h] // 00b04da4
        cmp byte ptr [eax + 065h],00h // 00b04da8
        jz L_00b04dc0 // 00b04dac
        mov ecx,dword ptr [edi + 08h] // 00b04dae
        mov eax,dword ptr [ecx + 08h] // 00b04db1
        mov ecx,dword ptr [esp + 040h] // 00b04db4
        add dword ptr [eax + 01f0h],ecx // 00b04db8
        jmp L_00b04dc4 // 00b04dbe
    L_00b04dc0:
        mov ecx,dword ptr [esp + 040h] // 00b04dc0
    L_00b04dc4:
        mov edx,dword ptr [edi + 08h] // 00b04dc4
        mov eax,dword ptr [edx + 08h] // 00b04dc7
        add dword ptr [eax + 01f4h],ecx // 00b04dca
        mov eax,dword ptr [edi + 08h] // 00b04dd0
        mov eax,dword ptr [eax + 08h] // 00b04dd3
        add dword ptr [eax + 01f8h],ecx // 00b04dd6
        mov eax,dword ptr [esp + 038h] // 00b04ddc
        pop edi // 00b04de0
        add esp,028h // 00b04de1
        ret 014h // 00b04de4
    }
}

__declspec(naked) void __fastcall generate_native_particle_spawn_vectors_00afdaf0(
    const void*, const NativeParticleEmissionSpawnAccess*, float*, float*, std::int32_t, float) {
    __asm {
        sub esp,028h // 00afdaf0
        mov dword ptr [esp + 024h],edx // additional local only
        mov edx,dword ptr [esp + 030h] // 00afdaf3
        push esi // 00afdaf7
        push edi // 00afdaf8
        mov edi,dword ptr [esp + 034h] // 00afdaf9
        mov esi,ecx // 00afdafd
        mov ecx,dword ptr [esi + 0a0h] // 00afdaff
        mov eax,dword ptr [ecx] // 00afdb05
        mov eax,dword ptr [eax + 0ch] // 00afdb07
        push edx // 00afdb0a
        push edi // 00afdb0b
        push esi // 00afdb0c
        mov edx,eax // already captured actual definition virtual0C
        mov eax,dword ptr [esp + 038h] // borrowed access
        call dword ptr [eax + 08h] // 00afdb0d
        fld dword ptr [esi] // 00afdb0f
        fstp dword ptr [esp + 038h] // 00afdb11
        fld dword ptr [esp + 038h] // 00afdb15
        fld st(0) // 00afdb19
        fsub dword ptr [esi + 0ch] // 00afdb1b
        fstp dword ptr [esp + 08h] // 00afdb1e
        fld dword ptr [esi + 04h] // 00afdb22
        fstp dword ptr [esp + 038h] // 00afdb25
        fld dword ptr [esp + 038h] // 00afdb29
        fld st(0) // 00afdb2d
        fsub dword ptr [esi + 010h] // 00afdb2f
        fstp dword ptr [esp + 0ch] // 00afdb32
        fld dword ptr [esi + 08h] // 00afdb36
        fstp dword ptr [esp + 038h] // 00afdb39
        fld dword ptr [esp + 038h] // 00afdb3d
        fld st(0) // 00afdb41
        fsub dword ptr [esi + 014h] // 00afdb43
        fstp dword ptr [esp + 010h] // 00afdb46
        fld dword ptr [esp + 08h] // 00afdb4a
        fld dword ptr [esp + 040h] // 00afdb4e
        fld st(0) // 00afdb52
        fmulp st(2),st(0) // 00afdb54
        fxch // 00afdb56
        fstp dword ptr [esp + 020h] // 00afdb58
        fld dword ptr [esp + 0ch] // 00afdb5c
        fmul st(0),st(1) // 00afdb60
        fstp dword ptr [esp + 024h] // 00afdb62
        fmul dword ptr [esp + 010h] // 00afdb66
        fstp dword ptr [esp + 028h] // 00afdb6a
        fld dword ptr [edi] // 00afdb6e
        fsubrp st(3),st(0) // 00afdb70
        fxch st(2) // 00afdb72
        fstp dword ptr [esp + 08h] // 00afdb74
        fsubr dword ptr [edi + 04h] // 00afdb78
        fstp dword ptr [esp + 0ch] // 00afdb7b
        fsubr dword ptr [edi + 08h] // 00afdb7f
        fstp dword ptr [esp + 010h] // 00afdb82
        fld dword ptr [esi + 0ch] // 00afdb86
        fadd dword ptr [esp + 08h] // 00afdb89
        fstp dword ptr [esp + 014h] // 00afdb8d
        fld dword ptr [esi + 010h] // 00afdb91
        fadd dword ptr [esp + 0ch] // 00afdb94
        fstp dword ptr [esp + 018h] // 00afdb98
        fld dword ptr [esi + 014h] // 00afdb9c
        fadd dword ptr [esp + 010h] // 00afdb9f
        fstp dword ptr [esp + 01ch] // 00afdba3
        fld dword ptr [esp + 014h] // 00afdba7
        fadd dword ptr [esp + 020h] // 00afdbab
        fstp dword ptr [esp + 08h] // 00afdbaf
        fld dword ptr [esp + 018h] // 00afdbb3
        fadd dword ptr [esp + 024h] // 00afdbb7
        fstp dword ptr [esp + 0ch] // 00afdbbb
        fld dword ptr [esp + 01ch] // 00afdbbf
        fadd dword ptr [esp + 028h] // 00afdbc3
        fstp dword ptr [esp + 010h] // 00afdbc7
        fld dword ptr [esp + 08h] // 00afdbcb
        fstp dword ptr [edi] // 00afdbcf
        fld dword ptr [esp + 0ch] // 00afdbd1
        fstp dword ptr [edi + 04h] // 00afdbd5
        fld dword ptr [esp + 010h] // 00afdbd8
        fstp dword ptr [edi + 08h] // 00afdbdc
        pop edi // 00afdbdf
        pop esi // 00afdbe0
        add esp,028h // 00afdbe1
        ret 010h // 00afdbe4
    }
}

} // namespace bsp
