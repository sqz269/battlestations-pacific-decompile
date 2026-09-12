#include "bsp/native_particle_sphere_definition.hpp"
#include "bsp/native_particle_definition.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_world.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle sphere definition requires MSVC Win32.
#endif
namespace bsp {
static_assert(offsetof(NativeParticleSphereDefinitionAccess, unit_random) == 0);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, crt) == 4);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, random_offset_00d7a210) == 8);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, two_pi_00ce3828) == 12);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, pi_00ce3d28) == 16);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, percent_00d7a220) == 20);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, percent_limit_00ce3d08) == 24);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, one_00d7a24c) == 28);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, allocate_00bf681b) == 32);
static_assert(offsetof(NativeParticleSphereDefinitionAccess, definition_virtual0c) == 36);

// Original normal allocation/leaf-construction schedule. The compiler-specific
// FS:[0] unwind frame is outside this new typed ABI; allocation still uses the
// required real domain and its actual exception behavior, with no fallback.
__declspec(naked) void* __fastcall create_native_particle_sphere_record_00b02bc0(
    void*, const NativeParticleSphereDefinitionAccess*, NativeNodeStorage*, float) {
    __asm {
        push esi
        push edi
        mov esi,ecx // definition captured before allocation
        mov edi,edx
        push 0108h // 00b02bd7
        call dword ptr [edi + 020h] // 00b02bde, actual00BF681B
        add esp,4 // 00b02be3
        test eax,eax // 00b02bea
        jz allocation_done // 00b02bf4
        fld dword ptr [esp + 010h] // 00b02bf6: incoming time
        push ecx // 00b02bfa
        fstp dword ptr [esp] // 00b02bff: original x87 float32 spill
        push esi // 00b02c02
        push dword ptr [esp + 014h] // 00b02c03: actual model
        mov ecx,eax // 00b02c04
        mov edx,dword ptr [edi + 01ch]
        call construct_native_particle_record_00afe0a0 // 00b02c06, RET0C
    allocation_done:
        pop edi
        pop esi
        ret 8 // 00b02c1a/00b02c2e
    }
}

// Keep original instruction ordering. Only incoming words shift for the extra
// access word; this includes the incoming parent slot reused as time scratch.
__declspec(naked) void __fastcall generate_native_particle_sphere_00b02ce0(
    void*, const NativeParticleSphereDefinitionAccess*, const void*, void*, void*) {
    __asm {
        push edx // borrowed access, outside original local frame
        sub esp,034h // 00b02ce0
        push esi // 00b02ce3
        push edi // 00b02ce4
        mov edi,ecx // 00b02ce5
        xor ecx,ecx // 00b02ce7
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx]
        call native_particle_unit_random_00bd2f40 // 00b02ce9
        fadd st(0),st(0) // 00b02cee
        xor ecx,ecx // 00b02cf0
        fstp dword ptr [esp + 014h] // 00b02cf2
        fld dword ptr [esp + 014h] // 00b02cf6
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx + 08h]
        fsub qword ptr [edx] // 00b02cfa
        fstp dword ptr [esp + 010h] // 00b02d00
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx]
        call native_particle_unit_random_00bd2f40 // 00b02d04
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx + 0ch]
        fmul qword ptr [edx] // 00b02d09
        mov esi,dword ptr [esp + 044h] // 00b02d0f
        fstp dword ptr [esp + 014h] // 00b02d13
        fld dword ptr [esp + 014h] // 00b02d17
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx + 010h]
        fsub qword ptr [edx] // 00b02d1b
        fstp dword ptr [esp + 08h] // 00b02d21
        fld dword ptr [esi + 03ch] // 00b02d25
        fmul dword ptr [esi + 034h] // 00b02d28
        fstp dword ptr [esp + 044h] // 00b02d2b
        fld dword ptr [esp + 044h] // 00b02d2f
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx + 014h]
        fld qword ptr [edx] // 00b02d33
        fcomip st(0),st(1) // 00b02d39
        fstp st(0) // 00b02d3b
        jbe l_00b02d47 // 00b02d3d
        movss xmm0,dword ptr [esp + 044h] // 00b02d3f
        jmp l_00b02d4f // 00b02d45
    L_00b02d47:
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx + 018h]
        movss xmm0,dword ptr [edx] // 00b02d47
    L_00b02d4f:
        fld dword ptr [esp + 010h] // 00b02d4f
        movss dword ptr [esp + 044h],xmm0 // 00b02d53
        fmul st(0),st(0) // 00b02d59
        fld1 // 00b02d5b
        fsubrp st(1),st(0) // 00b02d5d
        fstp dword ptr [esp + 014h] // 00b02d5f
        fld dword ptr [esp + 014h] // 00b02d63
        mov ecx,dword ptr [esp + 03ch]
        mov ecx,dword ptr [ecx + 4]
        call native_crt_sqrt_st0_00bf7030 // 00b02d67
        fstp dword ptr [esp + 014h] // 00b02d6c
        fld dword ptr [esp + 014h] // 00b02d70
        fstp dword ptr [esp + 014h] // 00b02d74
        fld dword ptr [esp + 08h] // 00b02d78
        fsin // 00b02d7c
        fstp dword ptr [esp + 0ch] // 00b02d7e
        fld dword ptr [esp + 0ch] // 00b02d82
        fmul dword ptr [esp + 014h] // 00b02d86
        fstp dword ptr [esp + 038h] // 00b02d8a
        fld dword ptr [esp + 08h] // 00b02d8e
        fcos // 00b02d92
        fstp dword ptr [esp + 0ch] // 00b02d94
        fld dword ptr [esp + 0ch] // 00b02d98
        mov ecx,dword ptr [edi + 084h] // 00b02d9c
        fmul dword ptr [esp + 014h] // 00b02da2
        movzx eax,word ptr [ecx + 0ah] // 00b02da6
        test ax,ax // 00b02daa
        fstp dword ptr [esp + 030h] // 00b02dad
        fld dword ptr [esp + 044h] // 00b02db1
        jnz l_00b02dc6 // 00b02db5
        movss xmm0,dword ptr [ecx + 04h] // 00b02db7
        fstp st(0) // 00b02dbc
        movss dword ptr [esp + 08h],xmm0 // 00b02dbe
        jmp l_00b02de0 // 00b02dc4
    L_00b02dc6:
        cmp ax,01h // 00b02dc6
        push ecx // 00b02dca
        fstp dword ptr [esp] // 00b02dcb
        jnz l_00b02dd7 // 00b02dce
        call evaluate_native_particle_linear_curve_00affa70 // 00b02dd0
        jmp l_00b02ddc // 00b02dd5
    L_00b02dd7:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b02dd7
    L_00b02ddc:
        fstp dword ptr [esp + 08h] // 00b02ddc
    L_00b02de0:
        mov ecx,dword ptr [edi + 088h] // 00b02de0
        movzx eax,word ptr [ecx + 0ah] // 00b02de6
        test ax,ax // 00b02dea
        jnz l_00b02dfc // 00b02ded
        movss xmm0,dword ptr [ecx + 04h] // 00b02def
        movss dword ptr [esp + 0ch],xmm0 // 00b02df4
        jmp l_00b02e1a // 00b02dfa
    L_00b02dfc:
        cmp ax,01h // 00b02dfc
        fld dword ptr [esp + 044h] // 00b02e00
        push ecx // 00b02e04
        fstp dword ptr [esp] // 00b02e05
        jnz l_00b02e11 // 00b02e08
        call evaluate_native_particle_linear_curve_00affa70 // 00b02e0a
        jmp l_00b02e16 // 00b02e0f
    L_00b02e11:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b02e11
    L_00b02e16:
        fstp dword ptr [esp + 0ch] // 00b02e16
    L_00b02e1a:
        fld dword ptr [esp + 0ch] // 00b02e1a
        fsub dword ptr [esp + 08h] // 00b02e1e
        fstp dword ptr [esp + 014h] // 00b02e22
        fldz // 00b02e26
        fld dword ptr [esp + 014h] // 00b02e28
        fucomip st(0),st(1) // 00b02e2c
        fstp st(0) // 00b02e2e
        lahf // 00b02e30
        test ah,044h // 00b02e31
        jp l_00b02e3a // 00b02e34
        fldz // 00b02e36
        jmp l_00b02e45 // 00b02e38
    L_00b02e3a:
        xor ecx,ecx // 00b02e3a
        mov edx,dword ptr [esp + 03ch]
        mov edx,dword ptr [edx]
        call native_particle_unit_random_00bd2f40 // 00b02e3c
        fmul dword ptr [esp + 014h] // 00b02e41
    L_00b02e45:
        fstp dword ptr [esp + 014h] // 00b02e45
        mov eax,dword ptr [esp + 048h] // 00b02e49
        fld dword ptr [esp + 014h] // 00b02e4d
        mov ecx,dword ptr [edi + 080h] // 00b02e51
        fadd dword ptr [esp + 08h] // 00b02e57
        pop edi // 00b02e5b
        fstp dword ptr [esp + 010h] // 00b02e5c
        fld dword ptr [esp + 010h] // 00b02e60
        fld st(0) // 00b02e64
        fmul dword ptr [esp + 02ch] // 00b02e66
        fstp dword ptr [esp + 014h] // 00b02e6a
        fld st(0) // 00b02e6e
        fmul dword ptr [esp + 0ch] // 00b02e70
        fstp dword ptr [esp + 018h] // 00b02e74
        fmul dword ptr [esp + 034h] // 00b02e78
        fstp dword ptr [esp + 01ch] // 00b02e7c
        fld dword ptr [esi] // 00b02e80
        fadd dword ptr [esp + 014h] // 00b02e82
        fstp dword ptr [esp + 020h] // 00b02e86
        fld dword ptr [esi + 04h] // 00b02e8a
        fadd dword ptr [esp + 018h] // 00b02e8d
        fstp dword ptr [esp + 024h] // 00b02e91
        fld dword ptr [esi + 08h] // 00b02e95
        pop esi // 00b02e98
        fadd dword ptr [esp + 018h] // 00b02e99
        fstp dword ptr [esp + 024h] // 00b02e9d
        fld dword ptr [esp + 01ch] // 00b02ea1
        fstp dword ptr [eax] // 00b02ea5
        fld dword ptr [esp + 020h] // 00b02ea7
        fstp dword ptr [eax + 04h] // 00b02eab
        fld dword ptr [esp + 024h] // 00b02eae
        fstp dword ptr [eax + 08h] // 00b02eb2
        movzx eax,word ptr [ecx + 0ah] // 00b02eb5
        test ax,ax // 00b02eb9
        jnz l_00b02ecb // 00b02ebc
        movss xmm0,dword ptr [ecx + 04h] // 00b02ebe
        movss dword ptr [esp + 03ch],xmm0 // 00b02ec3
        jmp l_00b02ee9 // 00b02ec9
    L_00b02ecb:
        cmp ax,01h // 00b02ecb
        fld dword ptr [esp + 03ch] // 00b02ecf
        push ecx // 00b02ed3
        fstp dword ptr [esp] // 00b02ed4
        jnz l_00b02ee0 // 00b02ed7
        call evaluate_native_particle_linear_curve_00affa70 // 00b02ed9
        jmp l_00b02ee5 // 00b02ede
    L_00b02ee0:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b02ee0
    L_00b02ee5:
        fstp dword ptr [esp + 03ch] // 00b02ee5
    L_00b02ee9:
        fld dword ptr [esp + 03ch] // 00b02ee9
        mov eax,dword ptr [esp + 044h] // 00b02eed
        fld st(0) // 00b02ef1
        fmul dword ptr [esp + 028h] // 00b02ef3
        fstp dword ptr [esp + 01ch] // 00b02ef7
        fld st(0) // 00b02efb
        fmul dword ptr [esp + 08h] // 00b02efd
        fstp dword ptr [esp + 020h] // 00b02f01
        fmul dword ptr [esp + 030h] // 00b02f05
        fstp dword ptr [esp + 024h] // 00b02f09
        fld dword ptr [esp + 01ch] // 00b02f0d
        fstp dword ptr [eax] // 00b02f11
        fld dword ptr [esp + 020h] // 00b02f13
        fstp dword ptr [eax + 04h] // 00b02f17
        fld dword ptr [esp + 024h] // 00b02f1a
        fstp dword ptr [eax + 08h] // 00b02f1e
        add esp,034h // 00b02f21
        lea esp,[esp + 4] // discard access without changing flags
        ret 0ch // 00b02f24
    }
}

__declspec(naked) void __fastcall generate_native_particle_sphere_matrix_00b02f30(
    void*, const NativeParticleSphereDefinitionAccess*, const void*, void*, void*, void*) {
    __asm {
        push edx // borrowed access
        mov edx,dword ptr [esp + 010h] // 00b02f30, velocity
        mov eax,dword ptr [ecx] // 00b02f34
        mov eax,dword ptr [eax + 0ch] // 00b02f36, captured target
        push esi // 00b02f39
        mov esi,dword ptr [esp + 0ch] // 00b02f3a, parent
        push edx // 00b02f3e
        mov edx,dword ptr [esp + 014h] // 00b02f3f, position
        push edx // 00b02f43
        push esi // 00b02f44
        mov edx,eax // dispatch the already captured target
        mov eax,dword ptr [esp + 010h]
        call dword ptr [eax + 024h] // 00b02f45, captured virtual0C, RET0C
        mov ecx,dword ptr [esi + 0a0h] // 00b02f47: post-callback reload
        cmp dword ptr [ecx + 070h],0 // 00b02f4d
        je model_matrix // 00b02f51
        mov ecx,dword ptr [esp + 018h] // 00b02f53
        lea eax,[esi + 060h] // 00b02f57
        push eax // 00b02f5a
        call copy_native_camera_matrix_004134f0 // 00b02f5b
        pop esi // 00b02f60
        lea esp,[esp + 4]
        ret 010h // 00b02f61
    model_matrix:
        mov esi,dword ptr [esi + 0a4h] // 00b02f64
        cmp byte ptr [esi + 01b0h],0 // 00b02f6a
        je world_matrix // 00b02f71
        mov ecx,esi // 00b02f73
        call get_native_node_local_matrix_00b6db60 // 00b02f75
        mov ecx,dword ptr [esp + 018h] // 00b02f7a
        push eax // 00b02f7e
        call copy_native_camera_matrix_004134f0 // 00b02f7f
        pop esi // 00b02f84
        lea esp,[esp + 4]
        ret 010h // 00b02f85
    world_matrix:
        test byte ptr [esi + 05ch],2 // 00b02f88
        jne copy_world // 00b02f8c
        mov ecx,esi // 00b02f8e
        call refresh_native_camera_world_00b6db70 // 00b02f90
    copy_world:
        mov ecx,dword ptr [esp + 018h] // 00b02f95
        lea eax,[esi + 0f0h] // 00b02f99
        push eax // 00b02f9f
        call copy_native_camera_matrix_004134f0 // 00b02fa0
        pop esi // 00b02fa5
        lea esp,[esp + 4]
        ret 010h // 00b02fa6
    }
}
} // namespace bsp
