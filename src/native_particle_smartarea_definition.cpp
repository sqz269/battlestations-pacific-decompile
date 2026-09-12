#include "bsp/native_particle_smartarea_definition.hpp"
#include "bsp/native_particle_definition.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/material_effect_plane.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native smart-area definition requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, unit_random) == 0);
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, crt) == 4);
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, percent_limit_00ce3d08) == 8);
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, percent_00d7a220) == 12);
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, turn_00ce3828) == 16);
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, degrees_00cf1448) == 20);
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, transverse_00d7a258) == 24);
static_assert(offsetof(NativeParticleSmartareaDefinitionAccess, definition_virtual0c) == 40);
void* __fastcall direction_bridge(void* destination, const void* source,
    const void* matrix, std::uint32_t) {
    transform_effect_direction_0042d0d0_no_normalize(
        *static_cast<EffectPlaneVector*>(destination),
        *static_cast<const EffectPlaneVector*>(source),
        *static_cast<const CameraMatrix*>(matrix));
    return destination;
}
} // namespace

void* __fastcall create_native_particle_smartarea_record_00b01ce0(
    void* definition, const NativeParticleSmartareaDefinitionAccess* access,
    NativeNodeStorage* model, float time) {
    void* allocation = access->allocate_00bf681b(0x108); // 00b01cfe
    if (allocation == nullptr) return nullptr;
    try {
        float spilled;
        __asm {
            fld time // 00b01d16, after the allocator returns
            fstp spilled // 00b01d1f, preserve binary32/x87 input conversion
        }
        return construct_native_particle_record_00afe0a0(allocation,
            access->one_00d7a24c, model, definition, spilled); // 00b01d26
    } catch (...) {
        access->free_00bf65ac(allocation); // original CBB4D0 allocation unwind
        throw;
    }
}

// The extra saved EDX word leaves every native local/outgoing offset intact.
// Incoming argument slots (also reused as float scratch) move by four bytes.
__declspec(naked) void __fastcall generate_native_particle_smartarea_00b01ec0(
    void*, const NativeParticleSmartareaDefinitionAccess*, const void*, void*, void*) {
    __asm {
        push edx // added borrowed access
        SUB ESP,03ch // 00b01ec0
        PUSH EBX // 00b01ec3
        PUSH ESI // 00b01ec4
        MOV ESI,dword ptr [ESP + 04ch] // 00b01ec5
        FLD dword ptr [ESI + 03ch] // 00b01ec9
        MOV EBX,ECX // 00b01ecc
        FMUL dword ptr [ESI + 034h] // 00b01ece
        FSTP dword ptr [ESP + 04ch] // 00b01ed1
        FLD dword ptr [ESP + 04ch] // 00b01ed5
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+0ch] // current 00d7a220 address
        FLD qword ptr [EDX] // 00b01ed9
        FCOMIP st(0),st(1) // 00b01edf
        FSTP st(0) // 00b01ee1
        JBE label_00b01eed // 00b01ee3
        MOVSS XMM0,dword ptr [ESP + 04ch] // 00b01ee5
        JMP label_00b01ef5 // 00b01eeb
    label_00b01eed:
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+08h] // current 00ce3d08 address
        MOVSS XMM0,dword ptr [EDX] // 00b01eed
    label_00b01ef5:
        MOV ECX,dword ptr [EBX + 084h] // 00b01ef5
        MOVZX EAX,word ptr [ECX + 0ah] // 00b01efb
        TEST AX,AX // 00b01eff
        MOVSS dword ptr [ESP + 08h],XMM0 // 00b01f02
        FLD dword ptr [ESP + 08h] // 00b01f08
        JNZ label_00b01f1d // 00b01f0c
        MOVSS XMM0,dword ptr [ECX + 04h] // 00b01f0e
        FSTP st(0) // 00b01f13
        MOVSS dword ptr [ESP + 014h],XMM0 // 00b01f15
        JMP label_00b01f37 // 00b01f1b
    label_00b01f1d:
        CMP AX,01h // 00b01f1d
        PUSH ECX // 00b01f21
        FSTP dword ptr [ESP] // 00b01f22
        JNZ label_00b01f2e // 00b01f25
        CALL evaluate_native_particle_linear_curve_00affa70 // 00b01f27
        JMP label_00b01f33 // 00b01f2c
    label_00b01f2e:
        CALL evaluate_native_particle_cubic_curve_00affae0 // 00b01f2e
    label_00b01f33:
        FSTP dword ptr [ESP + 014h] // 00b01f33
    label_00b01f37:
        XOR ECX,ECX // 00b01f37
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx] // canonical unit random binding
        CALL native_particle_unit_random_00bd2f40 // 00b01f39
        FSTP dword ptr [ESP + 04ch] // 00b01f3e
        XOR ECX,ECX // 00b01f42
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx] // canonical unit random binding
        CALL native_particle_unit_random_00bd2f40 // 00b01f44
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+010h] // current 00ce3828 address
        FMUL qword ptr [EDX] // 00b01f49
        MOV ECX,dword ptr [EBX + 08ch] // 00b01f4f
        MOVZX EAX,word ptr [ECX + 0ah] // 00b01f55
        TEST AX,AX // 00b01f59
        FSTP dword ptr [ESP + 01ch] // 00b01f5c
        FLD dword ptr [ESP + 04ch] // 00b01f60
        JNZ label_00b01f75 // 00b01f64
        MOVSS XMM0,dword ptr [ECX + 04h] // 00b01f66
        FSTP st(0) // 00b01f6b
        MOVSS dword ptr [ESP + 0ch],XMM0 // 00b01f6d
        JMP label_00b01fa8 // 00b01f73
    label_00b01f75:
        CMP AX,01h // 00b01f75
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+0ch] // current 00d7a220 address
        FMUL qword ptr [EDX] // 00b01f79
        PUSH ECX // 00b01f7f
        JNZ label_00b01f94 // 00b01f80
        FSTP dword ptr [ESP + 01ch] // 00b01f82
        FLD dword ptr [ESP + 01ch] // 00b01f86
        FSTP dword ptr [ESP] // 00b01f8a
        CALL evaluate_native_particle_linear_curve_00affa70 // 00b01f8d
        JMP label_00b01fa4 // 00b01f92
    label_00b01f94:
        FSTP dword ptr [ESP + 01ch] // 00b01f94
        FLD dword ptr [ESP + 01ch] // 00b01f98
        FSTP dword ptr [ESP] // 00b01f9c
        CALL evaluate_native_particle_cubic_curve_00affae0 // 00b01f9f
    label_00b01fa4:
        FSTP dword ptr [ESP + 0ch] // 00b01fa4
    label_00b01fa8:
        FLD dword ptr [ESP + 0ch] // 00b01fa8
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+014h] // current 00cf1448 address
        FDIV qword ptr [EDX] // 00b01fac
        FSTP dword ptr [ESP + 018h] // 00b01fb2
        FLD dword ptr [ESP + 018h] // 00b01fb6
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+010h] // current 00ce3828 address
        FMUL qword ptr [EDX] // 00b01fba
        FSTP dword ptr [ESP + 0ch] // 00b01fc0
        FLD dword ptr [ESP + 01ch] // 00b01fc4
        FCOS // 00b01fc8
        FSTP dword ptr [ESP + 010h] // 00b01fca
        FLD dword ptr [ESP + 01ch] // 00b01fce
        FSIN // 00b01fd2
        FSTP dword ptr [ESP + 018h] // 00b01fd4
        FLD dword ptr [ESP + 0ch] // 00b01fd8
        FCOS // 00b01fdc
        FSTP dword ptr [ESP + 01ch] // 00b01fde
        MOVSS XMM0,dword ptr [ESP + 01ch] // 00b01fe2
        MOVSS dword ptr [ESP + 024h],XMM0 // 00b01fe8
        FLD dword ptr [ESP + 0ch] // 00b01fee
        FSIN // 00b01ff2
        FSTP dword ptr [ESP + 01ch] // 00b01ff4
        FLD dword ptr [ESP + 01ch] // 00b01ff8
        FMUL dword ptr [ESP + 018h] // 00b01ffc
        FSTP dword ptr [ESP + 028h] // 00b02000
        FLD dword ptr [ESP + 0ch] // 00b02004
        FSIN // 00b02008
        FSTP dword ptr [ESP + 01ch] // 00b0200a
        FLD dword ptr [ESP + 01ch] // 00b0200e
        MOV EAX,dword ptr [ESI + 0a0h] // 00b02012
        CMP dword ptr [EAX + 070h],00h // 00b02018
        FMUL dword ptr [ESP + 010h] // 00b0201c
        FSTP dword ptr [ESP + 020h] // 00b02020
        JZ label_00b0202b // 00b02024
        LEA EAX,[ESI + 060h] // 00b02026
        JMP label_00b02058 // 00b02029
    label_00b0202b:
        PUSH EDI // 00b0202b
        MOV EDI,dword ptr [ESI + 0a4h] // 00b0202c
        CMP byte ptr [EDI + 01b0h],00h // 00b02032
        JZ label_00b02044 // 00b02039
        MOV ECX,EDI // 00b0203b
        CALL get_native_node_local_matrix_00b6db60 // 00b0203d
        JMP label_00b02057 // 00b02042
    label_00b02044:
        TEST byte ptr [EDI + 05ch],02h // 00b02044
        JNZ label_00b02051 // 00b02048
        MOV ECX,EDI // 00b0204a
        CALL refresh_native_camera_world_00b6db70 // 00b0204c
    label_00b02051:
        LEA EAX,[EDI + 0f0h] // 00b02051
    label_00b02057:
        POP EDI // 00b02057
    label_00b02058:
        PUSH 00h // 00b02058
        PUSH EAX // 00b0205a
        LEA EDX,[ESP + 028h] // 00b0205b
        LEA ECX,[ESP + 040h] // 00b0205f
        CALL direction_bridge // 00b02063
        FLD dword ptr [ESP + 04ch] // 00b02068
        MOVSS XMM0,dword ptr [EAX] // 00b0206c
        MOVSS dword ptr [ESP + 020h],XMM0 // 00b02070
        MOVSS XMM0,dword ptr [EAX + 04h] // 00b02076
        MOVSS dword ptr [ESP + 024h],XMM0 // 00b0207b
        MOVSS XMM0,dword ptr [EAX + 08h] // 00b02081
        MOVSS dword ptr [ESP + 028h],XMM0 // 00b02086
        mov ecx,dword ptr [esp+044h] // borrowed access
        mov ecx,dword ptr [ecx+04h] // same actual CRT domain
        CALL native_crt_sqrt_st0_00bf7030 // 00b0208c
        FSTP dword ptr [ESP + 01ch] // 00b02091
        FLD dword ptr [ESP + 01ch] // 00b02095
        MOV EAX,dword ptr [ESP + 050h] // 00b02099
        FSTP dword ptr [ESP + 01ch] // 00b0209d
        MOV ECX,dword ptr [EBX + 088h] // 00b020a1
        FLD dword ptr [ESP + 010h] // 00b020a7
        FLD dword ptr [ESP + 014h] // 00b020ab
        FLD st(0) // 00b020af
        FMULP st(2),st(0) // 00b020b1
        FXCH // 00b020b3
        FSTP dword ptr [ESP + 02ch] // 00b020b5
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+018h] // current 00d7a258 address
        FLD qword ptr [EDX] // 00b020b9
        FMUL st(0),st(1) // 00b020bf
        FSTP dword ptr [ESP + 030h] // 00b020c1
        FMUL dword ptr [ESP + 018h] // 00b020c5
        FSTP dword ptr [ESP + 034h] // 00b020c9
        FLD dword ptr [ESP + 02ch] // 00b020cd
        FLD dword ptr [ESP + 01ch] // 00b020d1
        FLD st(0) // 00b020d5
        FMULP st(2),st(0) // 00b020d7
        FXCH // 00b020d9
        FSTP dword ptr [ESP + 038h] // 00b020db
        FLD dword ptr [ESP + 030h] // 00b020df
        FMUL st(0),st(1) // 00b020e3
        FSTP dword ptr [ESP + 03ch] // 00b020e5
        FMUL dword ptr [ESP + 034h] // 00b020e9
        FSTP dword ptr [ESP + 040h] // 00b020ed
        FLD dword ptr [ESI] // 00b020f1
        FADD dword ptr [ESP + 038h] // 00b020f3
        FSTP dword ptr [ESP + 02ch] // 00b020f7
        FLD dword ptr [ESI + 04h] // 00b020fb
        FADD dword ptr [ESP + 03ch] // 00b020fe
        FSTP dword ptr [ESP + 030h] // 00b02102
        FLD dword ptr [ESI + 08h] // 00b02106
        FADD dword ptr [ESP + 040h] // 00b02109
        FSTP dword ptr [ESP + 034h] // 00b0210d
        FLD dword ptr [ESP + 02ch] // 00b02111
        FSTP dword ptr [EAX] // 00b02115
        FLD dword ptr [ESP + 030h] // 00b02117
        FSTP dword ptr [EAX + 04h] // 00b0211b
        FLD dword ptr [ESP + 034h] // 00b0211e
        FSTP dword ptr [EAX + 08h] // 00b02122
        MOVZX EAX,word ptr [ECX + 0ah] // 00b02125
        TEST AX,AX // 00b02129
        JNZ label_00b0213b // 00b0212c
        MOVSS XMM0,dword ptr [ECX + 04h] // 00b0212e
        MOVSS dword ptr [ESP + 050h],XMM0 // 00b02133
        JMP label_00b02172 // 00b02139
    label_00b0213b:
        CMP AX,01h // 00b0213b
        FLD dword ptr [ESP + 04ch] // 00b0213f
        mov edx,dword ptr [esp+044h] // borrowed access
        mov edx,dword ptr [edx+0ch] // current 00d7a220 address
        FMUL qword ptr [EDX] // 00b02143
        PUSH ECX // 00b02149
        JNZ label_00b0215e // 00b0214a
        FSTP dword ptr [ESP + 050h] // 00b0214c
        FLD dword ptr [ESP + 050h] // 00b02150
        FSTP dword ptr [ESP] // 00b02154
        CALL evaluate_native_particle_linear_curve_00affa70 // 00b02157
        JMP label_00b0216e // 00b0215c
    label_00b0215e:
        FSTP dword ptr [ESP + 050h] // 00b0215e
        FLD dword ptr [ESP + 050h] // 00b02162
        FSTP dword ptr [ESP] // 00b02166
        CALL evaluate_native_particle_cubic_curve_00affae0 // 00b02169
    label_00b0216e:
        FSTP dword ptr [ESP + 050h] // 00b0216e
    label_00b02172:
        MOV ECX,dword ptr [EBX + 080h] // 00b02172
        MOVZX EAX,word ptr [ECX + 0ah] // 00b02178
        TEST AX,AX // 00b0217c
        POP ESI // 00b0217f
        POP EBX // 00b02180
        JNZ label_00b02190 // 00b02181
        MOVSS XMM0,dword ptr [ECX + 04h] // 00b02183
        MOVSS dword ptr [ESP + 044h],XMM0 // 00b02188
        JMP label_00b021ad // 00b0218e
    label_00b02190:
        CMP AX,01h // 00b02190
        FLD dword ptr [ESP] // 00b02194
        PUSH ECX // 00b02197
        FSTP dword ptr [ESP] // 00b02198
        JNZ label_00b021a4 // 00b0219b
        CALL evaluate_native_particle_linear_curve_00affa70 // 00b0219d
        JMP label_00b021a9 // 00b021a2
    label_00b021a4:
        CALL evaluate_native_particle_cubic_curve_00affae0 // 00b021a4
    label_00b021a9:
        FSTP dword ptr [ESP + 044h] // 00b021a9
    label_00b021ad:
        FLD dword ptr [ESP + 044h] // 00b021ad
        MOV EAX,dword ptr [ESP + 04ch] // 00b021b1
        FLD st(0) // 00b021b5
        FMUL dword ptr [ESP + 018h] // 00b021b7
        FSTP dword ptr [ESP + 030h] // 00b021bb
        FLD st(0) // 00b021bf
        FMUL dword ptr [ESP + 01ch] // 00b021c1
        FSTP dword ptr [ESP + 034h] // 00b021c5
        FMUL dword ptr [ESP + 020h] // 00b021c9
        FSTP dword ptr [ESP + 038h] // 00b021cd
        FLD dword ptr [ESP + 030h] // 00b021d1
        FLD dword ptr [ESP + 048h] // 00b021d5
        FLD st(0) // 00b021d9
        FMULP st(2),st(0) // 00b021db
        FXCH // 00b021dd
        FSTP dword ptr [ESP + 024h] // 00b021df
        FLD dword ptr [ESP + 034h] // 00b021e3
        FMUL st(0),st(1) // 00b021e7
        FSTP dword ptr [ESP + 028h] // 00b021e9
        FMUL dword ptr [ESP + 038h] // 00b021ed
        FSTP dword ptr [ESP + 02ch] // 00b021f1
        FLD dword ptr [ESP + 024h] // 00b021f5
        FSTP dword ptr [EAX] // 00b021f9
        FLD dword ptr [ESP + 028h] // 00b021fb
        FSTP dword ptr [EAX + 04h] // 00b021ff
        FLD dword ptr [ESP + 02ch] // 00b02202
        FSTP dword ptr [EAX + 08h] // 00b02206
        ADD ESP,03ch // 00b02209
        add esp,04h // added borrowed access
        RET 0ch // 00b0220c
    }
}

__declspec(naked) void __fastcall generate_native_particle_smartarea_with_matrix_00b01e20(
    void*, const NativeParticleSmartareaDefinitionAccess*, const void*, void*, void*, void*) {
    __asm {
        push edx // borrowed access; incoming stack words shift four bytes
        mov edx,dword ptr [esp+010h] // 00b01e20
        mov eax,dword ptr [ecx] // 00b01e24
        mov eax,dword ptr [eax+0ch] // 00b01e26, capture current target once
        push esi // 00b01e29
        mov esi,dword ptr [esp+0ch] // 00b01e2a
        push edx // 00b01e2e
        mov edx,dword ptr [esp+014h] // 00b01e2f
        push edx // 00b01e33
        push esi // 00b01e34
        mov edx,eax // exact captured target
        mov eax,dword ptr [esp+010h] // borrowed access
        call dword ptr [eax+028h] // 00b01e35, same actual definition/record
        mov ecx,dword ptr [esi+0a0h] // 00b01e37, post-callback reload
        cmp dword ptr [ecx+070h],0 // 00b01e3d
        je matrix_local_or_world // 00b01e41
        mov ecx,dword ptr [esp+018h] // 00b01e43
        lea eax,[esi+060h] // 00b01e47
        push eax // 00b01e4a
        call copy_native_camera_matrix_004134f0 // 00b01e4b
        pop esi // 00b01e50
        add esp,04h // borrowed access
        ret 010h // 00b01e51
    matrix_local_or_world:
        mov esi,dword ptr [esi+0a4h] // 00b01e54
        cmp byte ptr [esi+01b0h],0 // 00b01e5a
        je matrix_world // 00b01e61
        mov ecx,esi // 00b01e63
        call get_native_node_local_matrix_00b6db60 // 00b01e65
        mov ecx,dword ptr [esp+018h] // 00b01e6a
        push eax // 00b01e6e
        call copy_native_camera_matrix_004134f0 // 00b01e6f
        pop esi // 00b01e74
        add esp,04h // borrowed access
        ret 010h // 00b01e75
    matrix_world:
        test byte ptr [esi+05ch],2 // 00b01e78
        jne matrix_world_ready // 00b01e7c
        mov ecx,esi // 00b01e7e
        call refresh_native_camera_world_00b6db70 // 00b01e80
    matrix_world_ready:
        mov ecx,dword ptr [esp+018h] // 00b01e85
        lea eax,[esi+0f0h] // 00b01e89
        push eax // 00b01e8f
        call copy_native_camera_matrix_004134f0 // 00b01e90
        pop esi // 00b01e95
        add esp,04h // borrowed access
        ret 010h // 00b01e96
    }
}
} // namespace bsp
