#include "bsp/native_particle_cone_definition.hpp"
#include "bsp/native_particle_unit_random.hpp"
#include "bsp/native_particle_definition.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/material_effect_plane.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cone definition requires MSVC Win32 x87/SSE assembly.
#endif
namespace bsp {
namespace {
static_assert(offsetof(NativeParticleConeDefinitionAccess, operator_new_00bf681b) == 0);
static_assert(offsetof(NativeParticleConeDefinitionAccess, unit_random) == 4);
static_assert(offsetof(NativeParticleConeDefinitionAccess, emission) == 8);
static_assert(offsetof(NativeParticleConeDefinitionAccess, percent_00d7a220) == 12);
static_assert(offsetof(NativeParticleConeDefinitionAccess, maximum_time_00ce3d08) == 16);
static_assert(offsetof(NativeParticleConeDefinitionAccess, angle_scale_00d5daf8) == 20);
static_assert(offsetof(NativeParticleConeDefinitionAccess, azimuth_scale_00ce3828) == 24);
static_assert(offsetof(NativeParticleConeDefinitionAccess, zero_00d7a218) == 28);
static_assert(offsetof(NativeParticleConeDefinitionAccess, free_00bf65ac) == 32);
static_assert(offsetof(NativeParticleConeDefinitionAccess, one_00d7a24c) == 36);
static_assert(offsetof(NativeParticleEmissionSpawnAccess, definition_virtual0c) == 8);
// The caller always supplies normalize=0. Adapt the existing exact branch
// kernel's C++ interface to original ECX/EDX/two-stack-word/RET8/EAX behavior.
float* __fastcall direction_bridge(float* destination, const float* source,
    const float* matrix, std::uint32_t) {
    transform_effect_direction_0042d0d0_no_normalize(
        *reinterpret_cast<EffectPlaneVector*>(destination),
        *reinterpret_cast<const EffectPlaneVector*>(source),
        *reinterpret_cast<const CameraMatrix*>(matrix));
    return destination;
}
} // namespace

void* __fastcall allocate_native_particle_cone_record_00b03970(void* definition,
    const NativeParticleConeDefinitionAccess* access, NativeNodeStorage* model, float time) {
    void* record = access->operator_new_00bf681b(0x108); // B0398E; ADD ESP,4
    if (!record) return nullptr;
    try {
        // B039A6/B039AF deliberately round the forwarded stack time through
        // x87 before the three-word thiscall constructor invocation.
        float forwarded_time;
        __asm { fld dword ptr [time] }
        __asm { fstp dword ptr [forwarded_time] }
        return construct_native_particle_record_00afe0a0(record, access->one_00d7a24c,
            model, definition, forwarded_time); // B039B6; callee RET0C
    } catch (...) {
        // Original CBB67B EH handler's state0 cleanup returns this allocation
        // to the same real allocation domain before propagating the exception.
        access->free_00bf65ac(record);
        throw;
    }
}

// Added callee-saved EBP holds access; only incoming stack words move by four.
// The original stack arguments still double as native float scratch.
__declspec(naked) void __fastcall generate_native_particle_cone_vectors_00b03b60(
    void*, const NativeParticleConeDefinitionAccess*, const void*, float*, float*) {
    __asm {
        push ebp
        mov ebp,edx
        sub esp,0x40 // 00b03b60
        push ebx // 00b03b63
        push edi // 00b03b64
        mov edi,dword ptr [esp + 0x50] // 00b03b65
        fld dword ptr [edi + 0x3c] // 00b03b69
        mov ebx,ecx // 00b03b6c
        fmul dword ptr [edi + 0x34] // 00b03b6e
        fstp dword ptr [esp + 0x50] // 00b03b71
        fld dword ptr [esp + 0x50] // 00b03b75
        mov edx,dword ptr [ebp+12] // borrowed current scalar
        fld qword ptr [edx] // 00b03b79
        fcomip st(0),st(1) // 00b03b7f
        fstp st(0) // 00b03b81
        jbe l_00b03b8d // 00b03b83
        movss xmm0,dword ptr [esp + 0x50] // 00b03b85
        jmp l_00b03b95 // 00b03b8b
    l_00b03b8d:
        mov edx,dword ptr [ebp+16] // borrowed current scalar
        movss xmm0,dword ptr [edx] // 00b03b8d
    l_00b03b95:
        mov ecx,dword ptr [ebx + 0x88] // 00b03b95
        movzx eax,word ptr [ecx + 0xa] // 00b03b9b
        test ax,ax // 00b03b9f
        movss dword ptr [esp + 0x50],xmm0 // 00b03ba2
        fld dword ptr [esp + 0x50] // 00b03ba8
        jnz l_00b03bbd // 00b03bac
        movss xmm0,dword ptr [ecx + 0x4] // 00b03bae
        fstp st(0) // 00b03bb3
        movss dword ptr [esp + 0x10],xmm0 // 00b03bb5
        jmp l_00b03bd7 // 00b03bbb
    l_00b03bbd:
        cmp ax,0x1 // 00b03bbd
        push ecx // 00b03bc1
        fstp dword ptr [esp] // 00b03bc2
        jnz l_00b03bce // 00b03bc5
        call evaluate_native_particle_linear_curve_00affa70 // 00b03bc7
        jmp l_00b03bd3 // 00b03bcc
    l_00b03bce:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b03bce
    l_00b03bd3:
        fstp dword ptr [esp + 0x10] // 00b03bd3
    l_00b03bd7:
        fld dword ptr [esp + 0x10] // 00b03bd7
        xor ecx,ecx // 00b03bdb
        mov edx,dword ptr [ebp+20] // borrowed current scalar
        fmul qword ptr [edx] // 00b03bdd
        fstp dword ptr [esp + 0x10] // 00b03be3
        mov edx,dword ptr [ebp+4] // canonical shared random access
        call native_particle_unit_random_00bd2f40 // 00b03be7
        mov edx,dword ptr [ebp+24] // borrowed current scalar
        fmul qword ptr [edx] // 00b03bec
        fstp dword ptr [esp + 0x18] // 00b03bf2
        fldz // 00b03bf6
        fld dword ptr [esp + 0x10] // 00b03bf8
        fucomip st(0),st(1) // 00b03bfc
        fstp st(0) // 00b03bfe
        lahf // 00b03c00
        test ah,0x44 // 00b03c01
        jp l_00b03c0a // 00b03c04
        fldz // 00b03c06
        jmp l_00b03c15 // 00b03c08
    l_00b03c0a:
        xor ecx,ecx // 00b03c0a
        mov edx,dword ptr [ebp+4] // canonical shared random access
        call native_particle_unit_random_00bd2f40 // 00b03c0c
        fmul dword ptr [esp + 0x10] // 00b03c11
    l_00b03c15:
        fstp dword ptr [esp + 0x8] // 00b03c15
        fld dword ptr [esp + 0x8] // 00b03c19
        fcos // 00b03c1d
        fstp dword ptr [esp + 0xc] // 00b03c1f
        movss xmm0,dword ptr [esp + 0xc] // 00b03c23
        movss dword ptr [esp + 0x28],xmm0 // 00b03c29
        fld dword ptr [esp + 0x8] // 00b03c2f
        fsin // 00b03c33
        fstp dword ptr [esp + 0xc] // 00b03c35
        fld dword ptr [esp + 0xc] // 00b03c39
        fstp dword ptr [esp + 0x20] // 00b03c3d
        fld dword ptr [esp + 0x18] // 00b03c41
        fsin // 00b03c45
        fstp dword ptr [esp + 0x14] // 00b03c47
        fld dword ptr [esp + 0x14] // 00b03c4b
        fmul dword ptr [esp + 0x20] // 00b03c4f
        fstp dword ptr [esp + 0x24] // 00b03c53
        fld dword ptr [esp + 0x8] // 00b03c57
        fsin // 00b03c5b
        fstp dword ptr [esp + 0xc] // 00b03c5d
        fld dword ptr [esp + 0xc] // 00b03c61
        fstp dword ptr [esp + 0x20] // 00b03c65
        fld dword ptr [esp + 0x18] // 00b03c69
        fcos // 00b03c6d
        fstp dword ptr [esp + 0x14] // 00b03c6f
        fld dword ptr [esp + 0x14] // 00b03c73
        mov eax,dword ptr [edi + 0xa0] // 00b03c77
        cmp dword ptr [eax + 0x70],0x0 // 00b03c7d
        fmul dword ptr [esp + 0x20] // 00b03c81
        fstp dword ptr [esp + 0x2c] // 00b03c85
        jz l_00b03c90 // 00b03c89
        lea eax,[edi + 0x60] // 00b03c8b
        jmp l_00b03cbd // 00b03c8e
    l_00b03c90:
        push esi // 00b03c90
        mov esi,dword ptr [edi + 0xa4] // 00b03c91
        cmp byte ptr [esi + 0x1b0],0x0 // 00b03c97
        jz l_00b03ca9 // 00b03c9e
        mov ecx,esi // 00b03ca0
        call get_native_node_local_matrix_00b6db60 // 00b03ca2
        jmp l_00b03cbc // 00b03ca7
    l_00b03ca9:
        test byte ptr [esi + 0x5c],0x2 // 00b03ca9
        jnz l_00b03cb6 // 00b03cad
        mov ecx,esi // 00b03caf
        call refresh_native_camera_world_00b6db70 // 00b03cb1
    l_00b03cb6:
        lea eax,[esi + 0xf0] // 00b03cb6
    l_00b03cbc:
        pop esi // 00b03cbc
    l_00b03cbd:
        push 0x0 // 00b03cbd
        push eax // 00b03cbf
        lea edx,[esp + 0x2c] // 00b03cc0
        lea ecx,[esp + 0x44] // 00b03cc4
        call direction_bridge // 00b03cc8
        movss xmm0,dword ptr [eax] // 00b03ccd
        movss dword ptr [esp + 0x24],xmm0 // 00b03cd1
        movss xmm0,dword ptr [eax + 0x4] // 00b03cd7
        movss dword ptr [esp + 0x28],xmm0 // 00b03cdc
        movss xmm0,dword ptr [eax + 0x8] // 00b03ce2
        movss dword ptr [esp + 0x2c],xmm0 // 00b03ce7
        movss xmm0,dword ptr [esp + 0x10] // 00b03ced
        mov edx,dword ptr [ebp+28] // borrowed current scalar
        ucomiss xmm0,dword ptr [edx] // 00b03cf3
        lahf // 00b03cfa
        test ah,0x44 // 00b03cfb
        jnp l_00b03d0c // 00b03cfe
        fld dword ptr [esp + 0x8] // 00b03d00
        fdiv dword ptr [esp + 0x10] // 00b03d04
        fstp dword ptr [esp + 0x8] // 00b03d08
    l_00b03d0c:
        mov ecx,dword ptr [ebx + 0x90] // 00b03d0c
        movzx eax,word ptr [ecx + 0xa] // 00b03d12
        test ax,ax // 00b03d16
        jnz l_00b03d28 // 00b03d19
        movss xmm0,dword ptr [ecx + 0x4] // 00b03d1b
        movss dword ptr [esp + 0x10],xmm0 // 00b03d20
        jmp l_00b03d46 // 00b03d26
    l_00b03d28:
        cmp ax,0x1 // 00b03d28
        fld dword ptr [esp + 0x50] // 00b03d2c
        push ecx // 00b03d30
        fstp dword ptr [esp] // 00b03d31
        jnz l_00b03d3d // 00b03d34
        call evaluate_native_particle_linear_curve_00affa70 // 00b03d36
        jmp l_00b03d42 // 00b03d3b
    l_00b03d3d:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b03d3d
    l_00b03d42:
        fstp dword ptr [esp + 0x10] // 00b03d42
    l_00b03d46:
        mov ecx,dword ptr [ebx + 0x8c] // 00b03d46
        movzx eax,word ptr [ecx + 0xa] // 00b03d4c
        test ax,ax // 00b03d50
        jnz l_00b03d62 // 00b03d53
        movss xmm0,dword ptr [ecx + 0x4] // 00b03d55
        movss dword ptr [esp + 0xc],xmm0 // 00b03d5a
        jmp l_00b03d80 // 00b03d60
    l_00b03d62:
        cmp ax,0x1 // 00b03d62
        fld dword ptr [esp + 0x50] // 00b03d66
        push ecx // 00b03d6a
        fstp dword ptr [esp] // 00b03d6b
        jnz l_00b03d77 // 00b03d6e
        call evaluate_native_particle_linear_curve_00affa70 // 00b03d70
        jmp l_00b03d7c // 00b03d75
    l_00b03d77:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b03d77
    l_00b03d7c:
        fstp dword ptr [esp + 0xc] // 00b03d7c
    l_00b03d80:
        fld dword ptr [esp + 0x8] // 00b03d80
        mov eax,dword ptr [esp + 0x54] // 00b03d84
        fld st(0) // 00b03d88
        mov ecx,dword ptr [ebx + 0x84] // 00b03d8a
        fld1 // 00b03d90
        fsubrp st(1),st(0) // 00b03d92
        fst qword ptr [esp + 0x18] // 00b03d94
        fld dword ptr [esp + 0x10] // 00b03d98
        fmulp st(2),st(0) // 00b03d9c
        fmul dword ptr [esp + 0xc] // 00b03d9e
        faddp st(1),st(0) // 00b03da2
        fstp dword ptr [esp + 0x20] // 00b03da4
        fld dword ptr [esp + 0x20] // 00b03da8
        fld st(0) // 00b03dac
        fmul dword ptr [esp + 0x24] // 00b03dae
        fstp dword ptr [esp + 0x30] // 00b03db2
        fld st(0) // 00b03db6
        fmul dword ptr [esp + 0x28] // 00b03db8
        fstp dword ptr [esp + 0x34] // 00b03dbc
        fmul dword ptr [esp + 0x2c] // 00b03dc0
        fstp dword ptr [esp + 0x38] // 00b03dc4
        fld dword ptr [edi] // 00b03dc8
        fadd dword ptr [esp + 0x30] // 00b03dca
        fstp dword ptr [esp + 0x3c] // 00b03dce
        fld dword ptr [edi + 0x4] // 00b03dd2
        fadd dword ptr [esp + 0x34] // 00b03dd5
        fstp dword ptr [esp + 0x40] // 00b03dd9
        fld dword ptr [edi + 0x8] // 00b03ddd
        fadd dword ptr [esp + 0x38] // 00b03de0
        fstp dword ptr [esp + 0x44] // 00b03de4
        fld dword ptr [esp + 0x3c] // 00b03de8
        fstp dword ptr [eax] // 00b03dec
        fld dword ptr [esp + 0x40] // 00b03dee
        fstp dword ptr [eax + 0x4] // 00b03df2
        fld dword ptr [esp + 0x44] // 00b03df5
        fstp dword ptr [eax + 0x8] // 00b03df9
        movzx eax,word ptr [ecx + 0xa] // 00b03dfc
        test ax,ax // 00b03e00
        jnz l_00b03e12 // 00b03e03
        movss xmm0,dword ptr [ecx + 0x4] // 00b03e05
        movss dword ptr [esp + 0x54],xmm0 // 00b03e0a
        jmp l_00b03e30 // 00b03e10
    l_00b03e12:
        cmp ax,0x1 // 00b03e12
        fld dword ptr [esp + 0x50] // 00b03e16
        push ecx // 00b03e1a
        fstp dword ptr [esp] // 00b03e1b
        jnz l_00b03e27 // 00b03e1e
        call evaluate_native_particle_linear_curve_00affa70 // 00b03e20
        jmp l_00b03e2c // 00b03e25
    l_00b03e27:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b03e27
    l_00b03e2c:
        fstp dword ptr [esp + 0x54] // 00b03e2c
    l_00b03e30:
        mov ecx,dword ptr [ebx + 0x80] // 00b03e30
        movzx eax,word ptr [ecx + 0xa] // 00b03e36
        test ax,ax // 00b03e3a
        pop edi // 00b03e3d
        pop ebx // 00b03e3e
        jnz l_00b03e4e // 00b03e3f
        movss xmm0,dword ptr [ecx + 0x4] // 00b03e41
        movss dword ptr [esp + 0x48],xmm0 // 00b03e46
        jmp l_00b03e6c // 00b03e4c
    l_00b03e4e:
        cmp ax,0x1 // 00b03e4e
        fld dword ptr [esp + 0x48] // 00b03e52
        push ecx // 00b03e56
        fstp dword ptr [esp] // 00b03e57
        jnz l_00b03e63 // 00b03e5a
        call evaluate_native_particle_linear_curve_00affa70 // 00b03e5c
        jmp l_00b03e68 // 00b03e61
    l_00b03e63:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b03e63
    l_00b03e68:
        fstp dword ptr [esp + 0x48] // 00b03e68
    l_00b03e6c:
        fld dword ptr [esp + 0x4c] // 00b03e6c
        mov eax,dword ptr [esp + 0x50] // 00b03e70
        fmul dword ptr [esp] // 00b03e74
        fld dword ptr [esp + 0x48] // 00b03e77
        fmul qword ptr [esp + 0x10] // 00b03e7b
        faddp st(1),st(0) // 00b03e7f
        fstp dword ptr [esp + 0x48] // 00b03e81
        fld dword ptr [esp + 0x48] // 00b03e85
        fld st(0) // 00b03e89
        fmul dword ptr [esp + 0x1c] // 00b03e8b
        fstp dword ptr [esp + 0x34] // 00b03e8f
        fld st(0) // 00b03e93
        fmul dword ptr [esp + 0x20] // 00b03e95
        fstp dword ptr [esp + 0x38] // 00b03e99
        fmul dword ptr [esp + 0x24] // 00b03e9d
        fstp dword ptr [esp + 0x3c] // 00b03ea1
        fld dword ptr [esp + 0x34] // 00b03ea5
        fstp dword ptr [eax] // 00b03ea9
        fld dword ptr [esp + 0x38] // 00b03eab
        fstp dword ptr [eax + 0x4] // 00b03eaf
        fld dword ptr [esp + 0x3c] // 00b03eb2
        fstp dword ptr [eax + 0x8] // 00b03eb6
        add esp,0x40 // 00b03eb9
        pop ebp
        ret 0xc // 00b03ebc
    }
}

__declspec(naked) void __fastcall generate_native_particle_cone_child_00b03ac0(
    void*, const NativeParticleConeDefinitionAccess*, const void*, float*, float*, void*) {
    __asm {
        push ebp
        mov ebp,edx
        mov edx,dword ptr [esp+0x10] // B03AC0 velocity
        mov eax,dword ptr [ecx] // B03AC4 current table
        mov eax,dword ptr [eax+0xc] // B03AC6 capture once
        push esi
        mov esi,dword ptr [esp+0xc] // B03ACA actual record
        push edx
        mov edx,dword ptr [esp+0x14] // B03ACF position
        push edx
        push esi
        mov edx,eax // captured native target
        mov eax,dword ptr [ebp+8]
        call dword ptr [eax+8] // B03AD5 canonical current virtual0C
        mov ecx,dword ptr [esi+0xa0] // B03AD7 reload after callback
        cmp dword ptr [ecx+0x70],0
        je child_model
        mov ecx,dword ptr [esp+0x18]
        lea eax,[esi+0x60]
        push eax
        call copy_native_camera_matrix_004134f0 // B03AEB
        pop esi
        pop ebp
        ret 0x10
    child_model:
        mov esi,dword ptr [esi+0xa4] // B03AF4
        cmp byte ptr [esi+0x1b0],0
        je child_world
        mov ecx,esi
        call get_native_node_local_matrix_00b6db60 // B03B05
        mov ecx,dword ptr [esp+0x18]
        push eax
        call copy_native_camera_matrix_004134f0 // B03B0F
        pop esi
        pop ebp
        ret 0x10
    child_world:
        test byte ptr [esi+0x5c],2 // B03B18
        jne child_copy_world
        mov ecx,esi
        call refresh_native_camera_world_00b6db70 // B03B20
    child_copy_world:
        mov ecx,dword ptr [esp+0x18]
        lea eax,[esi+0xf0]
        push eax
        call copy_native_camera_matrix_004134f0 // B03B30
        pop esi
        pop ebp
        ret 0x10 // B03B36
    }
}
} // namespace bsp
