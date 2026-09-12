#include "bsp/native_particle_definition.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_weak_owner.hpp"
#include <Windows.h>
#include <cstring>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4, "Original particle storage requires Win32");
namespace {
void* offset(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
const void* offset(const void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
template<class T> T load(const void* p, std::uint32_t n = 0) noexcept {
    T v; std::memcpy(&v, offset(p,n), sizeof v); return v;
}
template<class T> void store(void* p, std::uint32_t n, T v) noexcept {
    std::memcpy(offset(p,n), &v, sizeof v);
}
void profile(void* p, const volatile std::uint32_t* v) noexcept {
    store(p, 0, reinterpret_cast<std::uintptr_t>(v));
}
void destroy_string_and_reference(void* p, NativeParticleDefinitionBindings& a) noexcept {
    destroy_native_string_header_0041dd20(offset(p,8), a.strings);
    profile(p,a.reference_profile_00ceb130);
}
void dispose_parameter(void* p, std::uint32_t n, bool clear,
    NativeParticleDefinitionBindings& a) {
    void* value = load<void*>(p,n); // capture before both outgoing calls
    if (value) {
        destroy_native_particle_parameter_00affdf0(value,a);
        return_native_particle_parameter_00b00090(value,a);
        if (clear) store<std::uint32_t>(p,n,0);
    }
}
void release_rows(void* p, std::uint32_t first, std::uint32_t count,
    NativeParticleDefinitionBindings& a) {
    std::uint32_t index=0;
    while (static_cast<std::int32_t>(index) <
        *static_cast<volatile std::int32_t*>(offset(p,count))) {
        void* member=load<void*>(p, first+index*4u);
        if (::InterlockedDecrement(static_cast<volatile LONG*>(offset(member,4)))==0) {
            const void* table=load<const void*>(member);
            const auto target=load<std::uint32_t>(table);
            if (!a.member_virtual00) throw std::logic_error("particle definition requires current member virtual00");
            a.member_virtual00(a.context,member,target);
        }
        ++index;
    }
}
}

void* construct_native_particle_definition_00afa280(void* p, const void* name,
    std::uint32_t word10, std::uint32_t word70, std::uint32_t flag14,
    NativeParticleDefinitionBindings& a) {
    profile(p,a.reference_profile_00ceb130);
    store<std::uint32_t>(p,4,1);
    profile(p,a.base_profile_00d5dbc4);
    store<std::uint32_t>(p,8,0); store<std::uint32_t>(p,12,0);
    store<std::uint32_t>(p,0x4c,0); store<std::uint32_t>(p,0x68,0);
    store<std::uint8_t>(p,0x7c,1);
    try {
        void* destination=offset(p,8);
        if (destination != name) {
            resize_native_string_header_0041dd40(destination,a.strings,
                load<std::uint32_t>(name),true);
            if (load<std::uint32_t>(name)!=0) {
                const auto bytes=load<std::uint32_t>(destination);
                const void* source=load<const void*>(name,4);
                void* target=load<void*>(destination,4);
                // BF7680 selects backward copying for overlapping src<dst.
                if (bytes) std::memmove(target,source,bytes);
            }
        }
    } catch (...) { destroy_string_and_reference(p,a); throw; }
    store<std::uint8_t>(p,0x14,static_cast<std::uint8_t>(flag14));
    store(p,0x10,word10);
    store<std::uint8_t>(p,0x15,0); store<std::uint8_t>(p,0x1d,0);
    for (const auto n : {0x24u,0x28u,0x2cu,0x20u,0x34u}) store<std::uint32_t>(p,n,0);
    store(p,0x70,word70); store<std::uint8_t>(p,0x1c,1);
    store<std::uint32_t>(p,0x6c,0); store<std::uint32_t>(p,0x50,0);
    return p;
}
void* construct_native_particle_cone_definition_00b03940(void* p,const void* n,
    std::uint32_t x,std::uint32_t y,std::uint32_t f,NativeParticleDefinitionBindings& a) {
    construct_native_particle_definition_00afa280(p,n,x,y,f,a);
    profile(p,a.cone_profile_00d5debc); return p;
}
void* construct_native_particle_sphere_definition_00b02b90(void* p,const void* n,
    std::uint32_t x,std::uint32_t y,std::uint32_t f,NativeParticleDefinitionBindings& a) {
    construct_native_particle_definition_00afa280(p,n,x,y,f,a);
    profile(p,a.sphere_profile_00d5de88); return p;
}
void* construct_native_particle_smartarea_definition_00b01cb0(void* p,const void* n,
    std::uint32_t x,std::uint32_t y,std::uint32_t f,NativeParticleDefinitionBindings& a) {
    construct_native_particle_definition_00afa280(p,n,x,y,f,a);
    profile(p,a.smartarea_profile_00d5de48); return p;
}
void* create_native_particle_definition_00af9fb0(const void* kind,const void* name,
    std::uint32_t x,std::uint32_t y,void* text,NativeParticleDefinitionBindings& a) {
    using Construct=void* (*)(void*,const void*,std::uint32_t,std::uint32_t,std::uint32_t,NativeParticleDefinitionBindings&);
    Construct construct=nullptr; std::size_t bytes=0;
    const char* data=load<const char*>(kind,4);
    if (data && _stricmp(data,"ConeEmitter")==0) {bytes=0x94; construct=&construct_native_particle_cone_definition_00b03940;}
    else if (equal_native_string_header_00425850(kind,"SphereEmitter")) {bytes=0x8c; construct=&construct_native_particle_sphere_definition_00b02b90;}
    else if (equal_native_string_header_00425850(kind,"SmartAreaEmitter")) {bytes=0x90; construct=&construct_native_particle_smartarea_definition_00b01cb0;}
    void* owner=reinterpret_cast<void*>(y);
    if (construct) {
        owner=a.allocate_00bf681b(bytes);
        if (owner) {
            try { construct(owner,name,x,y,0,a); }
            catch (...) {a.free_00bf65ac(owner); throw;}
        }
    }
    const void* table=load<const void*>(owner);
    const auto target=load<std::uint32_t>(table,0x14);
    if (!a.parser_virtual14) throw std::logic_error("particle definition requires current parser virtual14");
    a.parser_virtual14(a.context,owner,target,text);
    return owner;
}
void destroy_native_particle_parameter_00affdf0(void* p,NativeParticleDefinitionBindings& a) noexcept {
    const auto kind=load<std::uint16_t>(p,0x0a);
    if (kind==1 || kind==2) {
        void* block=load<void*>(p,4);
        if (block) {a.free_array_00bf6989(block); store<std::uint32_t>(p,4,0);}
    }
}
void return_native_particle_parameter_00b00090(void* p,NativeParticleDefinitionBindings& a) {
    a.parameter_pool_00f8d344.return_raw_slot_00924420(p);
}
void destroy_native_particle_definition_00afa100(void* p,NativeParticleDefinitionBindings& a) {
    profile(p,a.base_profile_00d5dbc4);
    try {
        for (const auto n : {0x24u,0x28u,0x2cu,0x20u,0x30u,0x34u,0x38u}) dispose_parameter(p,n,true,a);
        release_rows(p,0x3c,0x4c,a); release_rows(p,0x54,0x68,a);
    } catch (...) {destroy_string_and_reference(p,a); throw;}
    destroy_string_and_reference(p,a);
}
void destroy_native_particle_cone_definition_00b039f0(void* p,NativeParticleDefinitionBindings& a) {
    profile(p,a.cone_profile_00d5debc);
    try { for (const auto n : {0x80u,0x84u,0x88u,0x8cu,0x90u}) dispose_parameter(p,n,false,a); }
    catch (...) {destroy_native_particle_definition_00afa100(p,a); throw;}
    destroy_native_particle_definition_00afa100(p,a);
}
void destroy_native_particle_sphere_definition_00b02c40(void* p,NativeParticleDefinitionBindings& a) {
    profile(p,a.sphere_profile_00d5de88);
    try { for (const auto n : {0x80u,0x84u,0x88u}) dispose_parameter(p,n,false,a); }
    catch (...) {destroy_native_particle_definition_00afa100(p,a); throw;}
    destroy_native_particle_definition_00afa100(p,a);
}
void destroy_native_particle_smartarea_definition_00b01d60(void* p,NativeParticleDefinitionBindings& a) {
    profile(p,a.smartarea_profile_00d5de48);
    try {
        dispose_parameter(p,0x80,false,a); dispose_parameter(p,0x84,false,a);
        dispose_parameter(p,0x88,true,a); dispose_parameter(p,0x8c,true,a);
    } catch (...) {destroy_native_particle_definition_00afa100(p,a); throw;}
    destroy_native_particle_definition_00afa100(p,a);
}
void* delete_native_particle_definition_00afa350(void* p,std::uint32_t f,NativeParticleDefinitionBindings& a) {
    destroy_native_particle_definition_00afa100(p,a); if (f&1) a.free_00bf65ac(p); return p;
}
void* delete_native_particle_cone_definition_00b03b40(void* p,std::uint32_t f,NativeParticleDefinitionBindings& a) {
    destroy_native_particle_cone_definition_00b039f0(p,a); if (f&1) a.free_00bf65ac(p); return p;
}
void* delete_native_particle_sphere_definition_00b02fb0(void* p,std::uint32_t f,NativeParticleDefinitionBindings& a) {
    destroy_native_particle_sphere_definition_00b02c40(p,a); if (f&1) a.free_00bf65ac(p); return p;
}
void* delete_native_particle_smartarea_definition_00b01ea0(void* p,std::uint32_t f,NativeParticleDefinitionBindings& a) {
    destroy_native_particle_smartarea_definition_00b01d60(p,a); if (f&1) a.free_00bf65ac(p); return p;
}

__declspec(naked) void* __fastcall construct_native_particle_record_00afe0a0(
    void*, const volatile std::uint32_t*, NativeNodeStorage*, void*, float) {
    __asm {
        movss xmm0,dword ptr [edx] // AFE0A0: borrowed current D7A24C
        mov eax,ecx
        mov ecx,dword ptr [esp+4]
        mov edx,dword ptr [esp+8]
        movss dword ptr [eax+0b4h],xmm0
        movss dword ptr [eax+0b0h],xmm0
        movss dword ptr [eax+0ach],xmm0
        movss dword ptr [eax+0a8h],xmm0
        movss xmm0,dword ptr [esp+0ch]
        mov dword ptr [eax+0a4h],ecx
        movss dword ptr [eax+30h],xmm0
        xorps xmm0,xmm0
        mov dword ptr [eax+0a0h],edx
        movss dword ptr [eax+34h],xmm0
        xor ecx,ecx
        mov dword ptr [eax+40h],ecx
        mov dword ptr [eax+44h],ecx
        mov dword ptr [eax+0b8h],ecx
        mov dword ptr [eax+0bch],ecx
        mov dword ptr [eax+0c0h],ecx
        mov dword ptr [eax+0c4h],ecx
        mov dword ptr [eax+0c8h],ecx
        mov dword ptr [eax+0cch],ecx
        mov dword ptr [eax+0d0h],ecx
        mov dword ptr [eax+0d4h],ecx
        mov dword ptr [eax+0d8h],ecx
        mov dword ptr [eax+0dch],ecx
        mov dword ptr [eax+0e0h],ecx
        mov dword ptr [eax+0e4h],ecx
        movss dword ptr [eax+0e8h],xmm0
        movss dword ptr [eax+0ech],xmm0
        movss dword ptr [eax+0f0h],xmm0
        movss dword ptr [eax+0f4h],xmm0
        movss dword ptr [eax+0f8h],xmm0
        movss dword ptr [eax+0fch],xmm0
        movss dword ptr [eax+100h],xmm0
        movss dword ptr [eax+104h],xmm0
        movss dword ptr [eax+24h],xmm0
        movss dword ptr [eax+28h],xmm0
        movss dword ptr [eax+2ch],xmm0
        ret 0ch
    }
}
} // namespace bsp
