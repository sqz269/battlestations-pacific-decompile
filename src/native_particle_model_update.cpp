#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_tracer_update.hpp"
#include "bsp/random_threads.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle model update requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeNodeStorage) == 0x174);
static_assert(offsetof(NativeParticleModelUpdateAccess, options) == 0);
static_assert(offsetof(NativeParticleModelUpdateAccess, random) == 4);
static_assert(offsetof(NativeParticleModelUpdateAccess, services) == 8);
static_assert(offsetof(NativeParticleModelUpdateAccess, floor_00bf85b0) == 12);
static_assert(offsetof(NativeParticleModelUpdateAccess, free_00bf65ac) == 16);
// Verified immutable image words. Keep original widths and load sites.
const std::uint32_t constant_00d7a218 = 0x00000000u;
const std::uint32_t constant_00d7a24c = 0x3f800000u;
alignas(8) const std::uint64_t constant_00d5da30 = 0x3e00000000000000ull;
alignas(8) const std::uint64_t constant_00d7a210 = 0x3ff0000000000000ull;
alignas(8) const std::uint64_t constant_00d7a220 = 0x4059000000000000ull;
alignas(8) const std::uint64_t constant_00d7a268 = 0x3f1a36e2e0000000ull;
const std::uint32_t constant_00d7a310 = 0x3727c5acu;

const volatile void* __fastcall options_bridge(void*, const NativeParticleModelUpdateAccess* access) {
    return access->options->call_0051f6b0();
}
std::uint32_t __fastcall random_bridge(RandomStream stream, const NativeParticleModelUpdateAccess* access) {
    return access->random->next_00bd2fc0(stream);
}
void* __fastcall definition_bridge(void* definition, const NativeParticleModelUpdateAccess* access,
    std::uint32_t captured_target, NativeNodeStorage* model, float time) {
    return access->services->definition_virtual08(definition, captured_target, *model, time);
}
void __fastcall append_bridge(void* owner, const NativeParticleModelUpdateAccess* access,
    const void* temporary) {
    access->services->call_00afd410(owner, temporary);
}
std::int32_t __fastcall emitter_bridge(void* emitter, const NativeParticleModelUpdateAccess* access,
    float delta) {
    return access->services->call_00aff640(emitter, delta);
}
std::int32_t __fastcall step_bridge(void* owner, const NativeParticleModelUpdateAccess* access,
    float time, std::uint8_t mode) {
    return access->services->call_00afd7a0(owner, time, mode);
}
void __fastcall bounds_bridge(void* model, void*, const float* words) noexcept {
    set_native_generated_model_bounds_00b74390(model, words);
}
} // namespace

// The extra four stack bytes contain only this invocation's access pointer.
// Original locals retain their offsets; incoming delta/mode are shifted by4.
// MOV-only access loads add no FP spills or flags. Native call stack cleanups
// are retained, except virtual08 gains the captured-target word for its bridge.
// B6E0D0 reuses B6FCB0's identical world/inverse/copy/flags instruction kernel.
__declspec(naked) void __fastcall update_native_particle_model_00af6dd0(
    NativeNodeStorage*, const NativeParticleModelUpdateAccess*, float, std::uint8_t) {
    __asm {
        sub esp,034h // 00af6dd0
        mov dword ptr [esp+030h],edx // borrowed per-call access
        push esi // 00af6dd3
        mov esi,ecx // 00af6dd4
        cmp byte ptr [esi + 01b0h],00h // 00af6dd6
        jz l_00af6e0f // 00af6ddd
        test byte ptr [esi + 05ch],02h // 00af6ddf
        jnz l_00af6dea // 00af6de3
        call refresh_native_camera_world_00b6db70 // 00af6de5
    L_00af6dea:
        lea eax,[esi + 0f0h] // 00af6dea
        push eax // 00af6df0
        lea ecx,[esi + 0218h] // 00af6df1
        call copy_native_camera_matrix_004134f0 // 00af6df7
        mov ecx,esi // 00af6dfc
        call get_native_camera_view_00b6fcb0 // 00af6dfe
        push eax // 00af6e03
        lea ecx,[esi + 0258h] // 00af6e04
        call copy_native_camera_matrix_004134f0 // 00af6e0a
    L_00af6e0f:
        mov edx,dword ptr [esp+034h] // per-call access; no FP or flag operation
        call options_bridge // 00af6e0f
        cmp byte ptr [eax + 04h],00h // 00af6e14
        jnz l_00af738b // 00af6e18
        push ebx // 00af6e1e
        xor ebx,ebx // 00af6e1f
        cmp byte ptr [esi + 01a5h],bl // 00af6e21
        push edi // 00af6e27
        jnz l_00af7072 // 00af6e28
        mov ecx,dword ptr [esi + 018ch] // 00af6e2e
        mov byte ptr [esi + 01a5h],01h // 00af6e34
        cmp dword ptr [ecx + 030h],ebx // 00af6e3b
        mov dword ptr [esp + 014h],ebx // 00af6e3e
        jle l_00af7072 // 00af6e42
        mov ebx,010h // 00af6e48
        push ebp // 00af6e4d
        mov edi,edi // 00af6e4e
    L_00af6e50:
        mov edx,dword ptr [esi + 018ch] // 00af6e50
        fld dword ptr [esi + 0184h] // 00af6e56
        mov ebp,dword ptr [ebx + edx*01h] // 00af6e5c
        mov eax,dword ptr [ebp] // 00af6e5f
        mov edx,dword ptr [eax + 08h] // 00af6e62
        push ecx // 00af6e65
        fstp dword ptr [esp] // 00af6e66
        push esi // 00af6e69
        mov ecx,ebp // 00af6e6a
        push edx // captured current native virtual08
        mov edx,dword ptr [esp+04ch]
        call definition_bridge // 00af6e6c
        mov edi,eax // 00af6e6e
        mov eax,dword ptr [ebp + 020h] // 00af6e70
        movss xmm0,dword ptr [eax] // 00af6e73
        ucomiss xmm0,dword ptr [constant_00d7a218] // 00af6e77
        lahf // 00af6e7e
        test ah,044h // 00af6e7f
        movss dword ptr [esp + 01ch],xmm0 // 00af6e82
        jp l_00af6e9a // 00af6e88
        movss xmm0,dword ptr [constant_00d7a24c] // 00af6e8a
        movss dword ptr [esp + 014h],xmm0 // 00af6e92
        jmp l_00af6ebd // 00af6e98
    L_00af6e9a:
        xor ecx,ecx // 00af6e9a
        mov edx,dword ptr [esp+040h] // per-call access; no FP or flag operation
        call random_bridge // 00af6e9c
        mov dword ptr [esp + 014h],eax // 00af6ea1
        fild dword ptr [esp + 014h] // 00af6ea5
        fmul dword ptr [esp + 01ch] // 00af6ea9
        fmul qword ptr [constant_00d5da30] // 00af6ead
        fadd qword ptr [constant_00d7a210] // 00af6eb3
        fstp dword ptr [esp + 014h] // 00af6eb9
    L_00af6ebd:
        mov ecx,dword ptr [ebp + 020h] // 00af6ebd
        movzx eax,word ptr [ecx + 0ah] // 00af6ec0
        test ax,ax // 00af6ec4
        jnz l_00af6ed6 // 00af6ec7
        movss xmm0,dword ptr [ecx + 04h] // 00af6ec9
        movss dword ptr [esp + 010h],xmm0 // 00af6ece
        jmp l_00af6ef2 // 00af6ed4
    L_00af6ed6:
        cmp ax,01h // 00af6ed6
        fldz // 00af6eda
        push ecx // 00af6edc
        fstp dword ptr [esp] // 00af6edd
        jnz l_00af6ee9 // 00af6ee0
        call evaluate_native_particle_linear_curve_00affa70 // 00af6ee2
        jmp l_00af6eee // 00af6ee7
    L_00af6ee9:
        call evaluate_native_particle_cubic_curve_00affae0 // 00af6ee9
    L_00af6eee:
        fstp dword ptr [esp + 010h] // 00af6eee
    L_00af6ef2:
        fld dword ptr [esp + 010h] // 00af6ef2
        fmul dword ptr [esp + 014h] // 00af6ef6
        fstp dword ptr [esp + 020h] // 00af6efa
        fld dword ptr [esp + 020h] // 00af6efe
        fst dword ptr [edi + 038h] // 00af6f02
        fdivr qword ptr [constant_00d7a220] // 00af6f05
        fstp dword ptr [edi + 03ch] // 00af6f0b
        cmp byte ptr [esi + 01b0h],00h // 00af6f0e
        jz l_00af6f76 // 00af6f15
        lea ecx,[esp + 030h] // 00af6f17
        push ecx // 00af6f1b
        mov ecx,esi // 00af6f1c
        call copy_native_node_local_position_00b6e0a0 // 00af6f1e
        fld dword ptr [eax] // 00af6f23
        fstp dword ptr [edi + 0ch] // 00af6f25
        xorps xmm0,xmm0 // 00af6f28
        fld dword ptr [eax + 04h] // 00af6f2b
        mov ecx,esi // 00af6f2e
        fstp dword ptr [edi + 010h] // 00af6f30
        fld dword ptr [eax + 08h] // 00af6f33
        fstp dword ptr [edi + 014h] // 00af6f36
        fld dword ptr [edi + 0ch] // 00af6f39
        fstp dword ptr [edi + 018h] // 00af6f3c
        fld dword ptr [edi + 010h] // 00af6f3f
        fstp dword ptr [edi + 01ch] // 00af6f42
        fld dword ptr [edi + 014h] // 00af6f45
        fstp dword ptr [edi + 020h] // 00af6f48
        fld dword ptr [edi + 018h] // 00af6f4b
        fstp dword ptr [edi] // 00af6f4e
        fld dword ptr [edi + 01ch] // 00af6f50
        fstp dword ptr [edi + 04h] // 00af6f53
        fld dword ptr [edi + 020h] // 00af6f56
        fstp dword ptr [edi + 08h] // 00af6f59
        movss dword ptr [edi + 048h],xmm0 // 00af6f5c
        movss dword ptr [edi + 04ch],xmm0 // 00af6f61
        movss dword ptr [edi + 050h],xmm0 // 00af6f66
        call get_native_node_local_matrix_00b6db60 // 00af6f6b
        push eax // 00af6f70
        jmp l_00af6ff1 // 00af6f71
    L_00af6f76:
        test byte ptr [esi + 05ch],02h // 00af6f76
        jnz l_00af6f83 // 00af6f7a
        mov ecx,esi // 00af6f7c
        call refresh_native_camera_world_00b6db70 // 00af6f7e
    L_00af6f83:
        movss xmm0,dword ptr [esi + 0120h] // 00af6f83
        movss xmm1,dword ptr [esi + 0124h] // 00af6f8b
        movss xmm2,dword ptr [esi + 0128h] // 00af6f93
        movss dword ptr [edi + 0ch],xmm0 // 00af6f9b
        movss dword ptr [edi + 010h],xmm1 // 00af6fa0
        movss dword ptr [edi + 014h],xmm2 // 00af6fa5
        movss dword ptr [edi + 018h],xmm0 // 00af6faa
        fld dword ptr [edi + 010h] // 00af6faf
        fstp dword ptr [edi + 01ch] // 00af6fb2
        fld dword ptr [edi + 014h] // 00af6fb5
        fstp dword ptr [edi + 020h] // 00af6fb8
        movss dword ptr [edi],xmm0 // 00af6fbb
        fld dword ptr [edi + 01ch] // 00af6fbf
        xorps xmm0,xmm0 // 00af6fc2
        fstp dword ptr [edi + 04h] // 00af6fc5
        fld dword ptr [edi + 020h] // 00af6fc8
        fstp dword ptr [edi + 08h] // 00af6fcb
        movss dword ptr [edi + 048h],xmm0 // 00af6fce
        movss dword ptr [edi + 04ch],xmm0 // 00af6fd3
        movss dword ptr [edi + 050h],xmm0 // 00af6fd8
        test byte ptr [esi + 05ch],02h // 00af6fdd
        jnz l_00af6fea // 00af6fe1
        mov ecx,esi // 00af6fe3
        call refresh_native_camera_world_00b6db70 // 00af6fe5
    L_00af6fea:
        lea edx,[esi + 0f0h] // 00af6fea
        push edx // 00af6ff0
    L_00af6ff1:
        lea ecx,[edi + 060h] // 00af6ff1
        call copy_native_camera_matrix_004134f0 // 00af6ff4
        test byte ptr [esi + 05ch],02h // 00af6ff9
        jnz l_00af7006 // 00af6ffd
        mov ecx,esi // 00af6fff
        call refresh_native_camera_world_00b6db70 // 00af7001
    L_00af7006:
        movss xmm0,dword ptr [esi + 0120h] // 00af7006
        movss xmm1,dword ptr [esi + 0124h] // 00af700e
        movss xmm2,dword ptr [esi + 0128h] // 00af7016
        movss dword ptr [esi + 020ch],xmm0 // 00af701e
        movss dword ptr [esi + 0210h],xmm1 // 00af7026
        movss dword ptr [esi + 0214h],xmm2 // 00af702e
        mov ecx,dword ptr [esi + 0190h] // 00af7036
        push edi // 00af703c
        mov edx,dword ptr [esp+044h] // per-call access; no FP or flag operation
        call append_bridge // 00af703d
        mov ecx,edi // 00af7042
        call destroy_native_particle_temporary_00afd9f0 // 00af7044
        push edi // 00af7049
        mov edx,dword ptr [esp+044h] // per-call access; no FP or flag operation
        call dword ptr [edx+010h] // 00af704a
        mov eax,dword ptr [esp + 01ch] // 00af704f
        mov ecx,dword ptr [esi + 018ch] // 00af7053
        add eax,01h // 00af7059
        add esp,04h // 00af705c
        add ebx,04h // 00af705f
        cmp eax,dword ptr [ecx + 030h] // 00af7062
        mov dword ptr [esp + 018h],eax // 00af7065
        jl l_00af6e50 // 00af7069
        xor ebx,ebx // 00af706f
        pop ebp // 00af7071
    L_00af7072:
        movss xmm0,dword ptr [esp + 044h] // 00af7072
        comiss xmm0,dword ptr [constant_00d7a218] // 00af7078
        jbe l_00af7205 // 00af707f
        cmp byte ptr [esi + 01b0h],00h // 00af7085
        jz l_00af7131 // 00af708c
        lea edx,[esp + 02ch] // 00af7092
        push edx // 00af7096
        mov ecx,esi // 00af7097
        call copy_native_node_local_position_00b6e0a0 // 00af7099
        fld dword ptr [eax] // 00af709e
        fsub dword ptr [esi + 020ch] // 00af70a0
        mov ecx,esi // 00af70a6
        fstp dword ptr [esp + 020h] // 00af70a8
        fld dword ptr [eax + 04h] // 00af70ac
        fsub dword ptr [esi + 0210h] // 00af70af
        fstp dword ptr [esp + 024h] // 00af70b5
        fld dword ptr [eax + 08h] // 00af70b9
        lea eax,[esp + 02ch] // 00af70bc
        fsub dword ptr [esi + 0214h] // 00af70c0
        push eax // 00af70c6
        fstp dword ptr [esp + 02ch] // 00af70c7
        fld dword ptr [esp + 024h] // 00af70cb
        fld dword ptr [esp + 048h] // 00af70cf
        fld st(0) // 00af70d3
        fdivp st(2),st(0) // 00af70d5
        fxch // 00af70d7
        fstp dword ptr [esp + 030h] // 00af70d9
        fld dword ptr [esp + 028h] // 00af70dd
        fdiv st(0),st(1) // 00af70e1
        fstp dword ptr [esp + 034h] // 00af70e3
        fdivr dword ptr [esp + 02ch] // 00af70e7
        fstp dword ptr [esp + 038h] // 00af70eb
        fld dword ptr [esp + 030h] // 00af70ef
        fstp dword ptr [esi + 0200h] // 00af70f3
        fld dword ptr [esp + 034h] // 00af70f9
        fstp dword ptr [esi + 0204h] // 00af70fd
        fld dword ptr [esp + 038h] // 00af7103
        fstp dword ptr [esi + 0208h] // 00af7107
        call copy_native_node_local_position_00b6e0a0 // 00af710d
        fld dword ptr [eax] // 00af7112
        fstp dword ptr [esi + 020ch] // 00af7114
        fld dword ptr [eax + 04h] // 00af711a
        fstp dword ptr [esi + 0210h] // 00af711d
        fld dword ptr [eax + 08h] // 00af7123
        fstp dword ptr [esi + 0214h] // 00af7126
        jmp l_00af7205 // 00af712c
    L_00af7131:
        test byte ptr [esi + 05ch],02h // 00af7131
        jnz l_00af713e // 00af7135
        mov ecx,esi // 00af7137
        call refresh_native_camera_world_00b6db70 // 00af7139
    L_00af713e:
        fld dword ptr [esi + 0120h] // 00af713e
        fstp dword ptr [esp + 02ch] // 00af7144
        fld dword ptr [esi + 0124h] // 00af7148
        fstp dword ptr [esp + 030h] // 00af714e
        fld dword ptr [esi + 0128h] // 00af7152
        fstp dword ptr [esp + 034h] // 00af7158
        fld dword ptr [esp + 02ch] // 00af715c
        fsub dword ptr [esi + 020ch] // 00af7160
        fstp dword ptr [esp + 020h] // 00af7166
        fld dword ptr [esp + 030h] // 00af716a
        fsub dword ptr [esi + 0210h] // 00af716e
        fstp dword ptr [esp + 024h] // 00af7174
        fld dword ptr [esp + 034h] // 00af7178
        fsub dword ptr [esi + 0214h] // 00af717c
        fstp dword ptr [esp + 028h] // 00af7182
        fld dword ptr [esp + 020h] // 00af7186
        fld dword ptr [esp + 044h] // 00af718a
        fld st(0) // 00af718e
        fdivp st(2),st(0) // 00af7190
        fxch // 00af7192
        fstp dword ptr [esp + 02ch] // 00af7194
        fld dword ptr [esp + 024h] // 00af7198
        fdiv st(0),st(1) // 00af719c
        fstp dword ptr [esp + 030h] // 00af719e
        fdivr dword ptr [esp + 028h] // 00af71a2
        fstp dword ptr [esp + 034h] // 00af71a6
        fld dword ptr [esp + 02ch] // 00af71aa
        fstp dword ptr [esi + 0200h] // 00af71ae
        fld dword ptr [esp + 030h] // 00af71b4
        fstp dword ptr [esi + 0204h] // 00af71b8
        fld dword ptr [esp + 034h] // 00af71be
        fstp dword ptr [esi + 0208h] // 00af71c2
        test byte ptr [esi + 05ch],02h // 00af71c8
        jnz l_00af71d5 // 00af71cc
        mov ecx,esi // 00af71ce
        call refresh_native_camera_world_00b6db70 // 00af71d0
    L_00af71d5:
        movss xmm0,dword ptr [esi + 0120h] // 00af71d5
        movss xmm1,dword ptr [esi + 0124h] // 00af71dd
        movss xmm2,dword ptr [esi + 0128h] // 00af71e5
        movss dword ptr [esi + 020ch],xmm0 // 00af71ed
        movss dword ptr [esi + 0210h],xmm1 // 00af71f5
        movss dword ptr [esi + 0214h],xmm2 // 00af71fd
    L_00af7205:
        fld dword ptr [esi + 0188h] // 00af7205
        sub esp,08h // 00af720b
        fadd dword ptr [esp + 04ch] // 00af720e
        fstp dword ptr [esp + 01ch] // 00af7212
        fld dword ptr [esi + 01a0h] // 00af7216
        fstp dword ptr [esp + 018h] // 00af721c
        fld dword ptr [esp + 01ch] // 00af7220
        fdiv dword ptr [esp + 018h] // 00af7224
        fstp dword ptr [esp + 024h] // 00af7228
        fld dword ptr [esp + 024h] // 00af722c
        fstp qword ptr [esp] // 00af7230
        mov edx,dword ptr [esp+044h] // per-call access; no FP or flag operation
        call dword ptr [edx+0ch] // 00af7233
        fstp dword ptr [esp + 024h] // 00af7238
        fld dword ptr [esp + 024h] // 00af723c
        add esp,08h // 00af7240
        fstp dword ptr [esp + 0ch] // 00af7243
        xor edi,edi // 00af7247
        cmp dword ptr [esi + 0198h],ebx // 00af7249
        fld dword ptr [esp + 010h] // 00af724f
        fld dword ptr [esp + 0ch] // 00af7253
        mov dword ptr [esi + 01f0h],ebx // 00af7257
        fld st(0) // 00af725d
        mov dword ptr [esi + 01f4h],ebx // 00af725f
        fmulp st(2),st(0) // 00af7265
        mov dword ptr [esi + 01f8h],ebx // 00af7267
        fxch // 00af726d
        fsubr dword ptr [esp + 014h] // 00af726f
        fstp dword ptr [esi + 0188h] // 00af7273
        jle l_00af72a2 // 00af7279
        fstp st(0) // 00af727b
    L_00af727d:
        fld dword ptr [esp + 044h] // 00af727d
        push ecx // 00af7281
        mov ecx,dword ptr [esi + 0194h] // 00af7282
        fstp dword ptr [esp] // 00af7288
        mov ecx,dword ptr [ecx + edi*04h] // 00af728b
        mov edx,dword ptr [esp+040h] // per-call access; no FP or flag operation
        call emitter_bridge // 00af728e
        add edi,01h // 00af7293
        cmp edi,dword ptr [esi + 0198h] // 00af7296
        jl l_00af727d // 00af729c
        fld dword ptr [esp + 0ch] // 00af729e
    L_00af72a2:
        fld qword ptr [constant_00d7a268] // 00af72a2
        fxch // 00af72a8
        fcomi st(0),st(1) // 00af72aa
        fstp st(1) // 00af72ac
        jbe l_00af7352 // 00af72ae
        fldz // 00af72b4
        xor edi,edi // 00af72b6
        fxch // 00af72b8
        fcomi st(0),st(1) // 00af72ba
        fstp st(1) // 00af72bc
        jbe l_00af7324 // 00af72be
        xorps xmm0,xmm0 // 00af72c0
        fstp st(0) // 00af72c3
        mov bl,byte ptr [esp + 048h] // 00af72c5
        movss dword ptr [esp + 044h],xmm0 // 00af72c9
        fld dword ptr [esp + 044h] // 00af72cf
        jmp l_00af72d7 // 00af72d3
    L_00af72d5:
        fstp st(0) // 00af72d5
    L_00af72d7:
        mov edx,dword ptr [esi + 018ch] // 00af72d7
        fmul dword ptr [esi + 01a0h] // 00af72dd
        cmp bl,byte ptr [edx + 070h] // 00af72e3
        fadd dword ptr [esi + 0184h] // 00af72e6
        setz al // 00af72ec
        fstp dword ptr [esp + 048h] // 00af72ef
        fld dword ptr [esp + 048h] // 00af72f3
        push eax // 00af72f7
        push ecx // 00af72f8
        mov ecx,dword ptr [esi + 0190h] // 00af72f9
        fstp dword ptr [esp] // 00af72ff
        mov edx,dword ptr [esp+044h] // per-call access; no FP or flag operation
        call step_bridge // 00af7302
        add edi,01h // 00af7307
        mov dword ptr [esp + 048h],edi // 00af730a
        fild dword ptr [esp + 048h] // 00af730e
        fstp dword ptr [esp + 044h] // 00af7312
        fld dword ptr [esp + 044h] // 00af7316
        fld dword ptr [esp + 0ch] // 00af731a
        fcomi st(0),st(1) // 00af731e
        ja l_00af72d5 // 00af7320
        fstp st(1) // 00af7322
    L_00af7324:
        movss xmm0,dword ptr [esp + 0ch] // 00af7324
        ucomiss xmm0,dword ptr [constant_00d7a218] // 00af732a
        lahf // 00af7331
        test ah,044h // 00af7332
        jnp l_00af733e // 00af7335
        mov byte ptr [esi + 01d4h],00h // 00af7337
    L_00af733e:
        fmul dword ptr [esi + 01a0h] // 00af733e
        fadd dword ptr [esi + 0184h] // 00af7344
        fstp dword ptr [esi + 0184h] // 00af734a
        jmp l_00af736a // 00af7350
    L_00af7352:
        movss xmm0,dword ptr [constant_00d7a310] // 00af7352
        fstp st(0) // 00af735a
        comiss xmm0,dword ptr [esp + 044h] // 00af735c
        jbe l_00af736a // 00af7361
        mov byte ptr [esi + 01d4h],00h // 00af7363
    L_00af736a:
        lea ecx,[esp + 02ch] // 00af736a
        push ecx // 00af736e
        mov ecx,dword ptr [esi + 018ch] // 00af736f
        call copy_native_particle_definition_bounds_00af3f50 // 00af7375
        push eax // 00af737a
        mov ecx,esi // 00af737b
        call bounds_bridge // 00af737d
        pop edi // 00af7382
        mov byte ptr [esi + 01d4h],00h // 00af7383
        pop ebx // 00af738a
    L_00af738b:
        pop esi // 00af738b
        add esp,034h // 00af738c
        ret 08h // 00af738f
    }
}

__declspec(naked) void* __fastcall copy_native_node_local_position_00b6e0a0(const void*, void*, void*) {
    __asm {
        mov eax,dword ptr [esp + 04h] // 00b6e0a0
        fld dword ptr [ecx + 0e0h] // 00b6e0a4
        fstp dword ptr [eax] // 00b6e0aa
        fld dword ptr [ecx + 0e4h] // 00b6e0ac
        fstp dword ptr [eax + 04h] // 00b6e0b2
        fld dword ptr [ecx + 0e8h] // 00b6e0b5
        fstp dword ptr [eax + 08h] // 00b6e0bb
        ret 04h // 00b6e0be
    }
}

__declspec(naked) const void* __fastcall get_native_node_local_matrix_00b6db60(const void*) {
    __asm {
        lea eax,[ecx + 0b0h] // 00b6db60
        ret // 00b6db66
    }
}

__declspec(naked) void* __fastcall copy_native_particle_definition_bounds_00af3f50(const void*, void*, void*) {
    __asm {
        mov eax,dword ptr [esp + 04h] // 00af3f50
        fld dword ptr [ecx + 07ch] // 00af3f54
        fstp dword ptr [eax] // 00af3f57
        fld dword ptr [ecx + 080h] // 00af3f59
        fstp dword ptr [eax + 04h] // 00af3f5f
        fld dword ptr [ecx + 084h] // 00af3f62
        fstp dword ptr [eax + 08h] // 00af3f68
        fld dword ptr [ecx + 08ch] // 00af3f6b
        fadd dword ptr [ecx + 088h] // 00af3f71
        fstp dword ptr [eax + 0ch] // 00af3f77
        ret 04h // 00af3f7a
    }
}

__declspec(naked) void __fastcall destroy_native_particle_temporary_00afd9f0(void*) {
    __asm {
        ret // 00afd9f0
    }
}

__declspec(naked) float __fastcall evaluate_native_particle_linear_curve_00affa70(const void*, void*, float) {
    __asm {
        fld dword ptr [esp + 04h] // 00affa70
        mov ecx,dword ptr [ecx + 04h] // 00affa74
        fld st(0) // 00affa77
        fld dword ptr [ecx + 04h] // 00affa79
        fxch // 00affa7c
        fcomi st(0),st(1) // 00affa7e
        fstp st(1) // 00affa80
        jbe l_00affa92 // 00affa82
    L_00affa84:
        fld dword ptr [ecx + 018h] // 00affa84
        add ecx,014h // 00affa87
        fxch // 00affa8a
        fcomi st(0),st(1) // 00affa8c
        fstp st(1) // 00affa8e
        ja l_00affa84 // 00affa90
    L_00affa92:
        fstp st(0) // 00affa92
        fld dword ptr [ecx] // 00affa94
        fstp dword ptr [esp + 04h] // 00affa96
        fld dword ptr [esp + 04h] // 00affa9a
        fld st(0) // 00affa9e
        fld st(2) // 00affaa0
        fxch // 00affaa2
        fucomip st(0),st(1) // 00affaa4
        fstp st(0) // 00affaa6
        lahf // 00affaa8
        test ah,044h // 00affaa9
        jp l_00affab8 // 00affaac
        fstp st(0) // 00affaae
        fstp st(0) // 00affab0
        fld dword ptr [ecx + 08h] // 00affab2
        ret 04h // 00affab5
    L_00affab8:
        fsubp st(1),st(0) // 00affab8
        fstp dword ptr [esp + 04h] // 00affaba
        fld dword ptr [esp + 04h] // 00affabe
        fmul dword ptr [ecx + 0ch] // 00affac2
        fadd dword ptr [ecx + 08h] // 00affac5
        fstp dword ptr [esp + 04h] // 00affac8
        fld dword ptr [esp + 04h] // 00affacc
        ret 04h // 00affad0
    }
}

__declspec(naked) float __fastcall evaluate_native_particle_cubic_curve_00affae0(const void*, void*, float) {
    __asm {
        fld dword ptr [esp + 04h] // 00affae0
        mov ecx,dword ptr [ecx + 04h] // 00affae4
        fld st(0) // 00affae7
        fld dword ptr [ecx + 04h] // 00affae9
        fxch // 00affaec
        fcomi st(0),st(1) // 00affaee
        fstp st(1) // 00affaf0
        jbe l_00affb02 // 00affaf2
    L_00affaf4:
        fld dword ptr [ecx + 020h] // 00affaf4
        add ecx,01ch // 00affaf7
        fxch // 00affafa
        fcomi st(0),st(1) // 00affafc
        fstp st(1) // 00affafe
        ja l_00affaf4 // 00affb00
    L_00affb02:
        fstp st(0) // 00affb02
        fld dword ptr [ecx] // 00affb04
        fstp dword ptr [esp + 04h] // 00affb06
        fld dword ptr [esp + 04h] // 00affb0a
        fld st(0) // 00affb0e
        fld st(2) // 00affb10
        fxch // 00affb12
        fucomip st(0),st(1) // 00affb14
        fstp st(0) // 00affb16
        lahf // 00affb18
        test ah,044h // 00affb19
        jp l_00affb28 // 00affb1c
        fstp st(1) // 00affb1e
        fstp st(0) // 00affb20
        fld dword ptr [ecx + 08h] // 00affb22
        ret 04h // 00affb25
    L_00affb28:
        fsubp st(1),st(0) // 00affb28
        fstp dword ptr [esp + 04h] // 00affb2a
        fld dword ptr [ecx + 0ch] // 00affb2e
        fld dword ptr [esp + 04h] // 00affb31
        fld st(0) // 00affb35
        fmulp st(2),st(0) // 00affb37
        fld dword ptr [ecx + 010h] // 00affb39
        faddp st(2),st(0) // 00affb3c
        fld st(0) // 00affb3e
        fmulp st(2),st(0) // 00affb40
        fld dword ptr [ecx + 014h] // 00affb42
        faddp st(2),st(0) // 00affb45
        fmulp st(1),st(0) // 00affb47
        fadd dword ptr [ecx + 018h] // 00affb49
        fstp dword ptr [esp + 04h] // 00affb4c
        fld dword ptr [esp + 04h] // 00affb50
        ret 04h // 00affb54
    }
}
} // namespace bsp
