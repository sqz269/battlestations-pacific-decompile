#include "bsp/native_particle_emitter_update.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_tracer_update.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle emitter update requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(offsetof(NativeParticleEmitterUpdateBindings, cleanup) == 0);
static_assert(offsetof(NativeParticleEmitterUpdateBindings, retained_owners) == 4);
static_assert(offsetof(NativeParticleEmitterUpdateBindings, crt) == 8);
static_assert(offsetof(NativeParticleEmitterUpdateBindings, vtable_00d5df24) == 12);
// Verified immutable image words, with original operand widths/load sites.
alignas(8) const std::uint64_t constant_00d7a270 = 0x3fa99999a0000000ull;
alignas(8) const std::uint64_t constant_00d7a280 = 0x3fe0000000000000ull;
alignas(8) const std::uint64_t constant_00d7a318 = 0x3f50624de0000000ull;
const std::uint32_t constant_00ce7638 = 0x3d4ccccdu;
const std::uint32_t constant_00ce4970 = 0x501502f9u;
const std::uint32_t constant_00ce4adc = 0xd01502f9u;
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint8_t __fastcall cleanup_bridge(void* state,
    const NativeParticleEmitterUpdateBindings* access, std::uint32_t argument) {
    return cleanup_native_particle_emitter_state_00b04f00(state, argument, *access->cleanup);
}
void* __fastcall geometry_bridge(void* model, void*, std::uint32_t unused) noexcept {
    return gui_model_geometry_00b74640(*static_cast<NativeModelTailStorage*>(at(model, 0x174)), unused);
}
void* __fastcall element_bridge(void* geometry, void*, std::int32_t index) noexcept {
    return gui_geometry_element_00b732c0(geometry, index);
}
void __fastcall model_bounds_bridge(void* model, void*, const float* bounds) noexcept {
    set_native_generated_model_bounds_00b74390(model, bounds);
}
void __fastcall element_bounds_bridge(void* element, void*, const GuiWidgetBounds* bounds) noexcept {
    set_gui_element_bounds_00b855b0(element, *bounds);
}
void* __fastcall current_delete_bridge(NativeParticleEmitterContainer* container,
    const NativeParticleEmitterUpdateBindings* access, std::uint32_t flags,
    std::uint32_t captured_profile) {
    if (captured_profile != 0x00d5df24 || !access->vtable_00d5df24 ||
        access->vtable_00d5df24[1] != 0x00b057a0)
        throw std::logic_error("actual emitter container current deleting profile is unsupported");
    return delete_native_particle_emitter_container_00b057a0(container, flags, *access);
}
} // namespace

NativeParticleEmitterActualCleanupBindings::NativeParticleEmitterActualCleanupBindings(
    NativePointLightLinksRuntime& links, GeneratedModelLifetimeRuntime& nodes) noexcept
    : links_(links), nodes_(nodes) {}
void NativeParticleEmitterActualCleanupBindings::call_00b7c160(void* actual_light) {
    unlink_native_point_light_nodes_00b7c160(links_.light(actual_light));
}
void NativeParticleEmitterActualCleanupBindings::call_00b6dfa0(void* actual_node) {
    auto* const lifetime = nodes_.find_actual_node(reinterpret_cast<std::uint32_t>(actual_node));
    if (!lifetime) throw std::logic_error("actual particle light node has no canonical lifetime binding");
    unlink_and_release_render_model_00b6dfa0(*lifetime);
}

void destroy_native_particle_emitter_base_00b04fc0(
    NativeParticleEmitterContainer& storage, NativeParticleEmitterCleanupBindings& cleanup) {
    volatile auto& owner = storage;
    owner.native_vtable_00 = 0x00d5df20;
    try {
        for (std::int32_t index = 0; index < static_cast<std::int32_t>(owner.count_1c); ++index) {
            const auto id = owner.rows_0c.data_00[index].state_id_00;
            cleanup_native_particle_emitter_state_00b04f00(at(owner.states_14.data_00,
                static_cast<std::uint32_t>(id) * 0x6cu), 1, cleanup);
        }
    } catch (...) {
        destroy_native_particle_emitter_states_00b04a80(storage.states_14);
        destroy_native_particle_emitter_rows_00b04c40(storage.rows_0c);
        throw;
    }
    destroy_native_particle_emitter_states_00b04a80(storage.states_14);
    destroy_native_particle_emitter_rows_00b04c40(storage.rows_0c);
}
void destroy_native_particle_emitter_container_00b05730(
    NativeParticleEmitterContainer& storage, const NativeParticleEmitterUpdateBindings& access) {
    volatile auto& owner = storage;
    owner.native_vtable_00 = 0x00d5df24;
    try {
        if (const auto identity = owner.word_2c) {
            release_native_render_actual_owner(*access.retained_owners,
                reinterpret_cast<void*>(identity));
            owner.word_2c = 0;
        }
    } catch (...) {
        try { destroy_native_particle_emitter_base_00b04fc0(storage, *access.cleanup); }
        catch (...) { std::terminate(); }
        throw;
    }
    destroy_native_particle_emitter_base_00b04fc0(storage, *access.cleanup);
}
NativeParticleEmitterContainer* delete_native_particle_emitter_container_00b057a0(
    NativeParticleEmitterContainer* owner, std::uint32_t flags,
    const NativeParticleEmitterUpdateBindings& access) {
    destroy_native_particle_emitter_container_00b05730(*owner, access);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

// Original instruction kernels retain x87 stack/control behavior. EDX adds
// only borrowed per-call bindings in one extra stack DWORD where needed.
// Existing direct native-storage bridges preserve original stack cleanups.
__declspec(naked) void __fastcall rotate_native_particle_emitter_row_00b04990(NativeParticleEmitterRows*, void*, std::int32_t, std::int32_t) {
    __asm {
        sub esp,08h // 00b04990
        mov edx,dword ptr [esp + 0ch] // 00b04993
        mov eax,dword ptr [ecx] // 00b04997
        push ebx // 00b04999
        mov ebx,dword ptr [esp + 014h] // 00b0499a
        push esi // 00b0499e
        mov esi,dword ptr [eax + edx*08h] // 00b0499f
        mov eax,dword ptr [eax + edx*08h + 04h] // 00b049a2
        mov dword ptr [esp + 0ch],eax // 00b049a6
        mov eax,ebx // 00b049aa
        sub eax,edx // 00b049ac
        cmp eax,04h // 00b049ae
        push edi // 00b049b1
        mov dword ptr [esp + 0ch],esi // 00b049b2
        mov edi,edx // 00b049b6
        jl L_00b04a2d // 00b049b8
        mov esi,ebx // 00b049ba
        sub esi,edx // 00b049bc
        sub esi,04h // 00b049be
        shr esi,02h // 00b049c1
        add esi,01h // 00b049c4
        lea eax,[edx*08h + 010h] // 00b049c7
        lea edi,[edx + esi*04h] // 00b049ce
        push ebp // 00b049d1
    L_00b049d2:
        mov edx,dword ptr [ecx] // 00b049d2
        mov bp,word ptr [eax + edx*01h + -08h] // 00b049d4
        mov word ptr [eax + edx*01h + -010h],bp // 00b049d9
        fld dword ptr [eax + edx*01h + -04h] // 00b049de
        fstp dword ptr [eax + edx*01h + -0ch] // 00b049e2
        lea edx,[eax + edx*01h + -010h] // 00b049e6
        mov edx,dword ptr [ecx] // 00b049ea
        mov bp,word ptr [eax + edx*01h] // 00b049ec
        mov word ptr [eax + edx*01h + -08h],bp // 00b049f0
        fld dword ptr [eax + edx*01h + 04h] // 00b049f5
        fstp dword ptr [eax + edx*01h + -04h] // 00b049f9
        mov edx,dword ptr [ecx] // 00b049fd
        mov bp,word ptr [eax + edx*01h + 08h] // 00b049ff
        mov word ptr [eax + edx*01h],bp // 00b04a04
        fld dword ptr [eax + edx*01h + 0ch] // 00b04a08
        fstp dword ptr [eax + edx*01h + 04h] // 00b04a0c
        mov edx,dword ptr [ecx] // 00b04a10
        mov bp,word ptr [eax + edx*01h + 010h] // 00b04a12
        mov word ptr [eax + edx*01h + 08h],bp // 00b04a17
        fld dword ptr [eax + edx*01h + 014h] // 00b04a1c
        fstp dword ptr [eax + edx*01h + 0ch] // 00b04a20
        add eax,020h // 00b04a24
        sub esi,01h // 00b04a27
        jnz L_00b049d2 // 00b04a2a
        pop ebp // 00b04a2c
    L_00b04a2d:
        cmp edi,ebx // 00b04a2d
        jge L_00b04a4b // 00b04a2f
    L_00b04a31:
        mov eax,dword ptr [ecx] // 00b04a31
        mov dx,word ptr [eax + edi*08h + 08h] // 00b04a33
        lea eax,[eax + edi*08h] // 00b04a38
        mov word ptr [eax],dx // 00b04a3b
        fld dword ptr [eax + 0ch] // 00b04a3e
        add edi,01h // 00b04a41
        fstp dword ptr [eax + 04h] // 00b04a44
        cmp edi,ebx // 00b04a47
        jl L_00b04a31 // 00b04a49
    L_00b04a4b:
        mov eax,dword ptr [ecx] // 00b04a4b
        mov cx,word ptr [esp + 0ch] // 00b04a4d
        movss xmm0,dword ptr [esp + 010h] // 00b04a52
        lea ebx,[eax + ebx*08h] // 00b04a58
        pop edi // 00b04a5b
        pop esi // 00b04a5c
        mov word ptr [ebx],cx // 00b04a5d
        movss dword ptr [ebx + 04h],xmm0 // 00b04a60
        pop ebx // 00b04a65
        add esp,08h // 00b04a66
        ret 08h // 00b04a69
    }
}

__declspec(naked) void __fastcall set_native_particle_geometry_bounds_00b72650(void*, void*, const float*) {
    __asm {
        mov eax,dword ptr [esp + 04h] // 00b72650
        mov byte ptr [ecx + 080h],01h // 00b72654
        fld dword ptr [eax] // 00b7265b
        fstp dword ptr [ecx + 084h] // 00b7265d
        fld dword ptr [eax + 04h] // 00b72663
        fstp dword ptr [ecx + 088h] // 00b72666
        fld dword ptr [eax + 08h] // 00b7266c
        fstp dword ptr [ecx + 08ch] // 00b7266f
        fld dword ptr [eax + 0ch] // 00b72675
        fstp dword ptr [ecx + 090h] // 00b72678
        ret 04h // 00b7267e
    }
}

__declspec(naked) void __fastcall update_native_particle_submodel_00af1eb0(void*, const NativeParticleEmitterUpdateBindings*, float) {
    __asm {
        sub esp,040h // 00af1eb0
        mov dword ptr [esp+03ch],edx // borrowed bindings
        push ebx // 00af1eb3
        push ebp // 00af1eb4
        push esi // 00af1eb5
        mov esi,ecx // 00af1eb6
        mov ecx,dword ptr [esi + 0184h] // 00af1eb8
        cmp byte ptr [ecx + 04h],00h // 00af1ebe
        push edi // 00af1ec2
        jz L_00af1fd9 // 00af1ec3
        fld qword ptr [constant_00d7a270] // 00af1ec9
        fld dword ptr [esp + 054h] // 00af1ecf
        fcomip st(0),st(1) // 00af1ed3
        fstp st(0) // 00af1ed5
        jbe L_00af1ee7 // 00af1ed7
        movss xmm0,dword ptr [constant_00ce7638] // 00af1ed9
        movss dword ptr [esp + 054h],xmm0 // 00af1ee1
    L_00af1ee7:
        mov eax,dword ptr [ecx + 0ch] // 00af1ee7
        sub eax,dword ptr [ecx + 08h] // 00af1eea
        mov dword ptr [esp + 010h],eax // 00af1eed
        fild dword ptr [esp + 010h] // 00af1ef1
        fmul dword ptr [ecx + 010h] // 00af1ef5
        fmul dword ptr [esp + 054h] // 00af1ef8
        fadd dword ptr [esi + 01a0h] // 00af1efc
        fstp dword ptr [esp + 010h] // 00af1f02
        fld dword ptr [esp + 010h] // 00af1f06
        fst dword ptr [esi + 01a0h] // 00af1f0a
        cmp byte ptr [ecx + 05h],00h // 00af1f10
        jz L_00af1f8f // 00af1f14
        mov edx,dword ptr [ecx + 0ch] // 00af1f16
        add edx,01h // 00af1f19
        mov dword ptr [esp + 010h],edx // 00af1f1c
        fild dword ptr [esp + 010h] // 00af1f20
        fxch // 00af1f24
        fcomip st(0),st(1) // 00af1f26
        fstp st(0) // 00af1f28
        jc L_00af1f68 // 00af1f2a
        lea esp,[esp] // 00af1f2c
    L_00af1f30:
        mov eax,dword ptr [ecx + 0ch] // 00af1f30
        fld dword ptr [esi + 01a0h] // 00af1f33
        sub eax,dword ptr [ecx + 08h] // 00af1f39
        mov edx,ecx // 00af1f3c
        mov dword ptr [esp + 010h],eax // 00af1f3e
        fisub dword ptr [esp + 010h] // 00af1f42
        fstp dword ptr [esi + 01a0h] // 00af1f46
        mov eax,dword ptr [edx + 0ch] // 00af1f4c
        fld dword ptr [esi + 01a0h] // 00af1f4f
        add eax,01h // 00af1f55
        mov dword ptr [esp + 010h],eax // 00af1f58
        fild dword ptr [esp + 010h] // 00af1f5c
        fxch // 00af1f60
        fcomip st(0),st(1) // 00af1f62
        fstp st(0) // 00af1f64
        jnc L_00af1f30 // 00af1f66
    L_00af1f68:
        cvttss2si eax,dword ptr [esi + 01a0h] // 00af1f68
        mov dword ptr [esi + 01b0h],eax // 00af1f70
        add eax,01h // 00af1f76
        mov dword ptr [esi + 01b4h],eax // 00af1f79
        cmp eax,dword ptr [ecx + 0ch] // 00af1f7f
        jle L_00af1fc7 // 00af1f82
        mov ecx,dword ptr [ecx + 08h] // 00af1f84
        mov dword ptr [esi + 01b4h],ecx // 00af1f87
        jmp L_00af1fc7 // 00af1f8d
    L_00af1f8f:
        fild dword ptr [ecx + 0ch] // 00af1f8f
        fstp dword ptr [esp + 010h] // 00af1f92
        fld dword ptr [esp + 010h] // 00af1f96
        fxch // 00af1f9a
        fcomip st(0),st(1) // 00af1f9c
        jc L_00af1fae // 00af1f9e
        fsub qword ptr [constant_00d7a318] // 00af1fa0
        fstp dword ptr [esi + 01a0h] // 00af1fa6
        jmp L_00af1fb0 // 00af1fac
    L_00af1fae:
        fstp st(0) // 00af1fae
    L_00af1fb0:
        cvttss2si eax,dword ptr [esi + 01a0h] // 00af1fb0
        mov dword ptr [esi + 01b0h],eax // 00af1fb8
        add eax,01h // 00af1fbe
        mov dword ptr [esi + 01b4h],eax // 00af1fc1
    L_00af1fc7:
        fld dword ptr [esi + 01a0h] // 00af1fc7
        fisub dword ptr [esi + 01b0h] // 00af1fcd
        fstp dword ptr [esi + 01ach] // 00af1fd3
    L_00af1fd9:
        fld dword ptr [esp + 054h] // 00af1fd9
        mov ebx,dword ptr [esi + 0190h] // 00af1fdd
        fadd dword ptr [esi + 01a4h] // 00af1fe3
        push 00h // 00af1fe9
        mov ecx,esi // 00af1feb
        fstp dword ptr [esi + 01a4h] // 00af1fed
        call geometry_bridge // 00af1ff3
        test ebx,ebx // 00af1ff8
        movss xmm0,dword ptr [constant_00ce4970] // 00af1ffa
        mov ecx,dword ptr [esi + 018ch] // 00af2002
        mov dword ptr [esp + 014h],eax // 00af2008
        mov eax,dword ptr [esi + 0184h] // 00af200c
        mov edi,dword ptr [eax + 020h] // 00af2012
        movss dword ptr [esp + 024h],xmm0 // 00af2015
        movss dword ptr [esp + 028h],xmm0 // 00af201b
        movss dword ptr [esp + 02ch],xmm0 // 00af2021
        movss xmm0,dword ptr [constant_00ce4adc] // 00af2027
        movss dword ptr [esp + 030h],xmm0 // 00af202f
        movss dword ptr [esp + 034h],xmm0 // 00af2035
        movss dword ptr [esp + 038h],xmm0 // 00af203b
        movss xmm0,dword ptr [eax + 028h] // 00af2041
        mov eax,dword ptr [esi + 0188h] // 00af2046
        lea edx,[ecx + ecx*04h] // 00af204c
        lea ecx,[eax + edx*04h] // 00af204f
        lea edx,[edi + edi*04h] // 00af2052
        movss dword ptr [esp + 010h],xmm0 // 00af2055
        lea ebp,[eax + edx*04h] // 00af205b
        jle L_00af2191 // 00af205e
        fld dword ptr [esp + 010h] // 00af2064
        mov dword ptr [esp + 010h],ebx // 00af2068
        fld st(0) // 00af206c
        or ebx,0ffffffffh // 00af206e
        fldz // 00af2071
        fsub st(1),st(0) // 00af2073
        fxch // 00af2075
        fstp qword ptr [esp + 018h] // 00af2077
        fld dword ptr [esp + 054h] // 00af207b
    L_00af207f:
        fld dword ptr [ecx + 010h] // 00af207f
        fadd st(0),st(1) // 00af2082
        fstp dword ptr [esp + 054h] // 00af2084
        fld dword ptr [esp + 054h] // 00af2088
        fst dword ptr [ecx + 010h] // 00af208c
        mov eax,dword ptr [esi + 0184h] // 00af208f
        fld dword ptr [eax + 018h] // 00af2095
        fstp dword ptr [esp + 054h] // 00af2098
        fld dword ptr [eax + 01ch] // 00af209c
        fld dword ptr [esp + 054h] // 00af209f
        fld st(0) // 00af20a3
        fsubp st(2),st(0) // 00af20a5
        fxch // 00af20a7
        fdiv qword ptr [esp + 018h] // 00af20a9
        fld st(2) // 00af20ad
        fsub st(0),st(5) // 00af20af
        fmulp st(1),st(0) // 00af20b1
        faddp st(1),st(0) // 00af20b3
        fstp dword ptr [esp + 054h] // 00af20b5
        fld dword ptr [esp + 054h] // 00af20b9
        fmul st(0),st(2) // 00af20bd
        fadd dword ptr [ecx + 0ch] // 00af20bf
        fstp dword ptr [ecx + 0ch] // 00af20c2
        fcomip st(0),st(3) // 00af20c5
        jbe L_00af20e1 // 00af20c7
        mov eax,dword ptr [esi + 018ch] // 00af20c9
        add eax,01h // 00af20cf
        cdq // 00af20d2
        idiv edi // 00af20d3
        add dword ptr [esi + 0190h],ebx // 00af20d5
        mov dword ptr [esi + 018ch],edx // 00af20db
    L_00af20e1:
        fld dword ptr [ecx] // 00af20e1
        fstp dword ptr [esp + 054h] // 00af20e3
        fld dword ptr [esp + 054h] // 00af20e7
        movss xmm2,dword ptr [esp + 054h] // 00af20eb
        fld dword ptr [esp + 024h] // 00af20f1
        fcomip st(0),st(1) // 00af20f5
        jbe L_00af20ff // 00af20f7
        movss dword ptr [esp + 024h],xmm2 // 00af20f9
    L_00af20ff:
        fld dword ptr [ecx + 04h] // 00af20ff
        fstp dword ptr [esp + 054h] // 00af2102
        fld dword ptr [esp + 054h] // 00af2106
        movss xmm1,dword ptr [esp + 054h] // 00af210a
        fld dword ptr [esp + 028h] // 00af2110
        fcomip st(0),st(1) // 00af2114
        jbe L_00af211e // 00af2116
        movss dword ptr [esp + 028h],xmm1 // 00af2118
    L_00af211e:
        fld dword ptr [ecx + 08h] // 00af211e
        fstp dword ptr [esp + 054h] // 00af2121
        fld dword ptr [esp + 054h] // 00af2125
        movss xmm0,dword ptr [esp + 054h] // 00af2129
        fld dword ptr [esp + 02ch] // 00af212f
        fcomip st(0),st(1) // 00af2133
        jbe L_00af213d // 00af2135
        movss dword ptr [esp + 02ch],xmm0 // 00af2137
    L_00af213d:
        fld dword ptr [esp + 030h] // 00af213d
        fxch st(3) // 00af2141
        fcomip st(0),st(3) // 00af2143
        fstp st(2) // 00af2145
        jbe L_00af214f // 00af2147
        movss dword ptr [esp + 030h],xmm2 // 00af2149
    L_00af214f:
        fld dword ptr [esp + 034h] // 00af214f
        fxch // 00af2153
        fcomip st(0),st(1) // 00af2155
        fstp st(0) // 00af2157
        jbe L_00af2161 // 00af2159
        movss dword ptr [esp + 034h],xmm1 // 00af215b
    L_00af2161:
        fld dword ptr [esp + 038h] // 00af2161
        fxch // 00af2165
        fcomip st(0),st(1) // 00af2167
        fstp st(0) // 00af2169
        jbe L_00af2173 // 00af216b
        movss dword ptr [esp + 038h],xmm0 // 00af216d
    L_00af2173:
        add ecx,014h // 00af2173
        cmp ecx,ebp // 00af2176
        jnz L_00af2180 // 00af2178
        mov ecx,dword ptr [esi + 0188h] // 00af217a
    L_00af2180:
        sub dword ptr [esp + 010h],01h // 00af2180
        jnz L_00af207f // 00af2185
        fstp st(0) // 00af218b
        fstp st(1) // 00af218d
        fstp st(0) // 00af218f
    L_00af2191:
        fld dword ptr [esp + 024h] // 00af2191
        fld st(0) // 00af2195
        fld dword ptr [esp + 030h] // 00af2197
        fld st(0) // 00af219b
        fsubp st(2),st(0) // 00af219d
        fxch // 00af219f
        fstp dword ptr [esp + 018h] // 00af21a1
        fld dword ptr [esp + 028h] // 00af21a5
        fld st(0) // 00af21a9
        fld dword ptr [esp + 034h] // 00af21ab
        fld st(0) // 00af21af
        fsubp st(2),st(0) // 00af21b1
        fxch // 00af21b3
        fstp dword ptr [esp + 01ch] // 00af21b5
        fld dword ptr [esp + 02ch] // 00af21b9
        fld st(0) // 00af21bd
        fld dword ptr [esp + 038h] // 00af21bf
        fld st(0) // 00af21c3
        fsubp st(2),st(0) // 00af21c5
        fxch // 00af21c7
        fstp dword ptr [esp + 020h] // 00af21c9
        fxch st(4) // 00af21cd
        faddp st(5),st(0) // 00af21cf
        fxch st(4) // 00af21d1
        fstp dword ptr [esp + 030h] // 00af21d3
        faddp st(1),st(0) // 00af21d7
        fstp dword ptr [esp + 034h] // 00af21d9
        faddp st(1),st(0) // 00af21dd
        fstp dword ptr [esp + 038h] // 00af21df
        fld dword ptr [esp + 030h] // 00af21e3
        fld qword ptr [constant_00d7a280] // 00af21e7
        fmul st(1),st(0) // 00af21ed
        fxch // 00af21ef
        fstp dword ptr [esp + 024h] // 00af21f1
        fld dword ptr [esp + 034h] // 00af21f5
        fmul st(0),st(1) // 00af21f9
        fstp dword ptr [esp + 028h] // 00af21fb
        fmul dword ptr [esp + 038h] // 00af21ff
        fstp dword ptr [esp + 02ch] // 00af2203
        fld dword ptr [esp + 024h] // 00af2207
        fstp dword ptr [esp + 03ch] // 00af220b
        fld dword ptr [esp + 028h] // 00af220f
        fstp dword ptr [esp + 040h] // 00af2213
        fld dword ptr [esp + 02ch] // 00af2217
        fstp dword ptr [esp + 044h] // 00af221b
        fld dword ptr [esp + 018h] // 00af221f
        fld dword ptr [esp + 01ch] // 00af2223
        fld dword ptr [esp + 020h] // 00af2227
        fld st(1) // 00af222b
        fmulp st(2),st(0) // 00af222d
        fxch // 00af222f
        fstp dword ptr [esp + 054h] // 00af2231
        fld dword ptr [esp + 054h] // 00af2235
        fld st(2) // 00af2239
        fmulp st(3),st(0) // 00af223b
        fxch st(2) // 00af223d
        fstp dword ptr [esp + 054h] // 00af223f
        fld dword ptr [esp + 054h] // 00af2243
        faddp st(2),st(0) // 00af2247
        fmul st(0),st(0) // 00af2249
        fstp dword ptr [esp + 054h] // 00af224b
        fadd dword ptr [esp + 054h] // 00af224f
        fstp dword ptr [esp + 054h] // 00af2253
        fld dword ptr [esp + 054h] // 00af2257
        mov ecx,dword ptr [esp+04ch] // borrowed bindings
        mov ecx,dword ptr [ecx+8] // borrowed bindings
        call native_crt_sqrt_st0_00bf7030 // 00af225b
        fstp dword ptr [esp + 054h] // 00af2260
        fld dword ptr [esp + 054h] // 00af2264
        mov edi,dword ptr [esp + 014h] // 00af2268
        fmul qword ptr [constant_00d7a280] // 00af226c
        lea eax,[esp + 03ch] // 00af2272
        push eax // 00af2276
        mov ecx,edi // 00af2277
        fstp dword ptr [esp + 04ch] // 00af2279
        call set_native_particle_geometry_bounds_00b72650 // 00af227d
        lea ecx,[esp + 03ch] // 00af2282
        push ecx // 00af2286
        mov ecx,esi // 00af2287
        call model_bounds_bridge // 00af2289
        push 00h // 00af228e
        mov ecx,edi // 00af2290
        call element_bridge // 00af2292
        lea edx,[esp + 03ch] // 00af2297
        push edx // 00af229b
        mov ecx,eax // 00af229c
        call element_bounds_bridge // 00af229e
        pop edi // 00af22a3
        pop esi // 00af22a4
        pop ebp // 00af22a5
        pop ebx // 00af22a6
        add esp,040h // 00af22a7
        ret 04h // 00af22aa
    }
}

__declspec(naked) std::int32_t __fastcall update_native_particle_emitter_container_00b05110(NativeParticleEmitterContainer*, const NativeParticleEmitterUpdateBindings*, float) {
    __asm {
        push edx // borrowed bindings
        push ecx // 00b05110
        push ebp // 00b05111
        push edi // 00b05112
        mov edi,ecx // 00b05113
        xor ebp,ebp // 00b05115
        cmp dword ptr [edi + 01ch],ebp // 00b05117
        jle L_00b051fe // 00b0511a
        push ebx // 00b05120
        push esi // 00b05121
    L_00b05122:
        mov eax,dword ptr [edi + 0ch] // 00b05122
        fld dword ptr [esp + 01ch] // 00b05125
        movzx ebx,word ptr [eax + ebp*08h] // 00b05129
        mov esi,dword ptr [edi + 014h] // 00b0512d
        imul ebx,ebx,06ch // 00b05130
        fld dword ptr [esi + ebx*01h + 040h] // 00b05133
        fadd st(0),st(1) // 00b05137
        fstp dword ptr [esi + ebx*01h + 040h] // 00b05139
        mov ecx,dword ptr [esi + ebx*01h + 064h] // 00b0513d
        add esi,ebx // 00b05141
        cmp dword ptr [ecx + 010h],03h // 00b05143
        jnz L_00b05157 // 00b05147
        push ecx // 00b05149
        mov ecx,dword ptr [esi + 030h] // 00b0514a
        fstp dword ptr [esp] // 00b0514d
        mov edx,dword ptr [esp+018h] // borrowed bindings
        call update_native_particle_submodel_00af1eb0 // 00b05150
        jmp L_00b05159 // 00b05155
    L_00b05157:
        fstp st(0) // 00b05157
    L_00b05159:
        fld dword ptr [esi + 040h] // 00b05159
        mov edx,dword ptr [esi + 064h] // 00b0515c
        fstp dword ptr [esp + 010h] // 00b0515f
        fld dword ptr [esp + 010h] // 00b05163
        fld dword ptr [edx + 020h] // 00b05167
        fxch // 00b0516a
        fcomi st(0),st(1) // 00b0516c
        fstp st(1) // 00b0516e
        ja L_00b0517f // 00b05170
        fld dword ptr [esi + 044h] // 00b05172
        fxch // 00b05175
        fcomip st(0),st(1) // 00b05177
        fstp st(0) // 00b05179
        jbe L_00b051b8 // 00b0517b
        jmp L_00b05181 // 00b0517d
    L_00b0517f:
        fstp st(0) // 00b0517f
    L_00b05181:
        push 00h // 00b05181
        mov ecx,esi // 00b05183
        mov edx,dword ptr [esp+018h] // borrowed bindings
        call cleanup_bridge // 00b05185
        test al,al // 00b0518a
        jz L_00b051b8 // 00b0518c
        mov eax,dword ptr [edi + 01ch] // 00b0518e
        sub eax,01h // 00b05191
        cmp ebp,eax // 00b05194
        jge L_00b051b4 // 00b05196
        mov eax,dword ptr [edi + 010h] // 00b05198
        mov ecx,dword ptr [edi + 01ch] // 00b0519b
        sub eax,01h // 00b0519e
        cmp ecx,eax // 00b051a1
        jge L_00b051a7 // 00b051a3
        mov eax,ecx // 00b051a5
    L_00b051a7:
        push eax // 00b051a7
        push ebp // 00b051a8
        lea ecx,[edi + 0ch] // 00b051a9
        call rotate_native_particle_emitter_row_00b04990 // 00b051ac
        sub ebp,01h // 00b051b1
    L_00b051b4:
        add dword ptr [edi + 01ch],-01h // 00b051b4
    L_00b051b8:
        mov ecx,dword ptr [edi + 014h] // 00b051b8
        mov ebx,dword ptr [ebx + ecx*01h + 064h] // 00b051bb
        mov eax,dword ptr [ebx + 010h] // 00b051bf
        test eax,eax // 00b051c2
        jnz L_00b051e1 // 00b051c4
        cmp byte ptr [ebx + 065h],00h // 00b051c6
        mov eax,dword ptr [edi + 04h] // 00b051ca
        jz L_00b051d8 // 00b051cd
        add dword ptr [eax + 01f0h],01h // 00b051cf
        jmp L_00b051f0 // 00b051d6
    L_00b051d8:
        add dword ptr [eax + 01f4h],01h // 00b051d8
        jmp L_00b051f0 // 00b051df
    L_00b051e1:
        cmp eax,01h // 00b051e1
        jnz L_00b051f0 // 00b051e4
        mov eax,dword ptr [edi + 04h] // 00b051e6
        add dword ptr [eax + 01f8h],01h // 00b051e9
    L_00b051f0:
        add ebp,01h // 00b051f0
        cmp ebp,dword ptr [edi + 01ch] // 00b051f3
        jl L_00b05122 // 00b051f6
        pop esi // 00b051fc
        pop ebx // 00b051fd
    L_00b051fe:
        mov eax,dword ptr [edi + 01ch] // 00b051fe
        pop edi // 00b05201
        pop ebp // 00b05202
        pop ecx // 00b05203
        add esp,4 // borrowed bindings
        ret 04h // 00b05204
    }
}

__declspec(naked) std::int32_t __fastcall update_native_particle_emitter_00aff640(void*, const NativeParticleEmitterUpdateBindings*, float) {
    __asm {
        push edx // borrowed bindings
        push esi // 00aff640
        mov esi,ecx // 00aff641
        mov ecx,dword ptr [esi + 010h] // 00aff643
        test ecx,ecx // 00aff646
        jz L_00aff67c // 00aff648
        fld dword ptr [esp + 0ch] // 00aff64a
        push ecx // 00aff64e
        fstp dword ptr [esp] // 00aff64f
        mov edx,dword ptr [esp+8] // borrowed bindings
        call update_native_particle_emitter_container_00b05110 // 00aff652
        test eax,eax // 00aff657
        mov dword ptr [esi + 020h],eax // 00aff659
        jnz L_00aff686 // 00aff65c
        mov ecx,dword ptr [esi + 010h] // 00aff65e
        test ecx,ecx // 00aff661
        jz L_00aff686 // 00aff663
        mov eax,dword ptr [ecx] // 00aff665
        mov edx,dword ptr [esp+4] // 00aff667
        push eax // borrowed bindings
        push 01h // 00aff66a
        call current_delete_bridge // 00aff66c
        mov dword ptr [esi + 010h],00h // 00aff66e
        mov eax,dword ptr [esi + 020h] // 00aff675
        pop esi // 00aff678
        add esp,4 // borrowed bindings
        ret 04h // 00aff679
    L_00aff67c:
        mov dword ptr [esi + 020h],00h // 00aff67c
        mov eax,dword ptr [esi + 020h] // 00aff683
    L_00aff686:
        pop esi // 00aff686
        add esp,4 // borrowed bindings
        ret 04h // 00aff687
    }
}

} // namespace bsp
