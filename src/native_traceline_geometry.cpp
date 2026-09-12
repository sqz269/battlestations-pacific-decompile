#include "bsp/native_traceline_geometry.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/random_threads.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Traceline geometry requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p)+offset);
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p)+offset)=value;
}
struct SectionGuard {
    TrackedCriticalSection* captured;
    explicit SectionGuard(TrackedCriticalSection* section):captured(section) {
        if (captured) {
            EnterCriticalSection(&captured->native);
            write(captured,0x18,read<std::uint32_t>(captured,0x18)+1u);
        }
    }
    ~SectionGuard() {
        if (captured) {
            write(captured,0x18,read<std::uint32_t>(captured,0x18)-1u);
            LeaveCriticalSection(&captured->native);
        }
    }
};
// Preserve the original FLD/FST/FSTP conversion and both argument bit patterns.
void sentinel_pair(const volatile float* source, float* values) noexcept {
    __asm {
        mov eax,source
        mov edx,values
        fld dword ptr [eax]
        fst dword ptr [edx+4]
        fstp dword ptr [edx]
    }
}
}

void attach_native_traceline_geometry_00af3440(NativeModelOwner& model,
    void* payload, RenderNodeRootList* root, NativeTracelineGeometryAccess& a) {
    auto& owners=a.geometry.actual_owners();
    if (model.phase!=NativeModelOwner::Phase::live ||
        &model.environment.retained_owners!=&owners ||
        &a.material_lifetime.retained_owners!=&owners ||
        model.environment.actual_names!=&a.strings ||
        &a.parameters.parameter_names!=&a.strings ||
        &a.material_lifetime.parameter_names!=&a.strings ||
        static_cast<const volatile void*>(&a.unchanged_00d7a260)!=
            static_cast<const volatile void*>(&model.environment.constants.unchanged_00d7a260) ||
        static_cast<NativeMaterialParameterSlots*>(&a.parameters.parameter_slots)!=
            &a.material_lifetime.parameter_slots || !a.allocate_array_00bf55be)
        throw std::invalid_argument("Traceline geometry requires the same live native owner and string domains");
    auto* node=static_cast<void*>(&model.storage.node);
    const SectionGuard guard(a.lock_00f8c284); // Capture once; release captured lock.
    write(node,0x184,payload);
    if (read<std::uint8_t>(payload,4)!=0) {
        const std::int32_t value=read<std::int32_t>(payload,8);
        float converted;
        __asm {
            cvtsi2ss xmm0,value
            movss converted,xmm0
        }
        write(node,0x1a0,converted);
    }
    const auto count=read<std::uint32_t>(payload,0x20);
    const auto bytes=count>std::numeric_limits<std::uint32_t>::max()/0x14u
        ? std::numeric_limits<std::uint32_t>::max() : count*0x14u;
    write(node,0x188,a.allocate_array_00bf55be(bytes));
    write<std::uint32_t>(node,0x190,0);
    write<std::uint32_t>(node,0x18c,0);
    auto* mesh=a.geometry.create_mesh(); // Same B73B60 pool and B73D70 constructor.
    float scalars[2];
    sentinel_pair(&a.unchanged_00d7a260,scalars);
    set_native_model_geometry_00b75170(model,0,mesh,scalars[0],scalars[1]);
    void* declaration=read<void*>(payload,0x40);
    const auto vertices=read<std::uint32_t>(payload,0x20)*2u+2u;
    void* renderer=a.renderer_00f8d394;
    const auto* table=read<const volatile std::uintptr_t*>(renderer,0);
    using CreateStream=void* (__thiscall*)(void*,std::uint32_t,std::uint32_t,void*);
    const auto factory=reinterpret_cast<CreateStream>(table[0x5c/4]);
    void* stream=factory(renderer,vertices,0x1000,declaration);
    set_native_mesh_vertex_stream_00b73bb0(*mesh,owners,0,stream);
    release_native_render_actual_owner(owners,stream);
    auto* section=a.geometry.create_section();
    // AF3569..AF35F8 selects three different stack temporaries but the SAME
    // D5D934 string and 535320 factory in all branches. Keep its live flag read.
    static_cast<void>(read<std::uint32_t>(payload,0x48));
    NativeString effect_name;
    effect_name.assign_0041e870(a.strings,"traceline.mshd");
    NativeMaterialStorage* material;
    try {
        material=a.geometry.create_material_for_effect_00535320(effect_name,
            a.renderer_00f8d394,a.material_lifetime,a.material_vtable_00d5e520);
    } catch (...) { destroy_native_string_header_0041dd20(&effect_name,a.strings); throw; }
    destroy_native_string_header_0041dd20(&effect_name,a.strings);
    NativeString parameter_name;
    resize_native_string_header_0041dd40(&parameter_name,a.strings,8,true);
    if (auto* data=read<void*>(&parameter_name,4))
        std::memcpy(data,"AnimFrac",read<std::uint32_t>(&parameter_name,0)+1u);
    try {
        register_native_material_float_00b18b20(*material,&parameter_name,
            static_cast<std::byte*>(node)+0x1ac,a.parameters);
    } catch (...) { destroy_native_string_header_0041dd20(&parameter_name,a.strings); throw; }
    destroy_native_string_header_0041dd20(&parameter_name,a.strings);
    void* items=read<void*>(payload,0x4c);
    void* first=read<void*>(items,0);
    set_native_material_texture_00b189f0(*material,0,read<void*>(first,8),owners);
    set_native_mesh_section_material_00b864c0(*section,owners,material);
    release_native_render_actual_owner(owners,material);
    write<std::uint32_t>(section,8,5);
    write<std::uint32_t>(section,0xc,0);
    write<std::uint32_t>(section,0x14,0);
    write<std::uint32_t>(section,0x10,0);
    write<std::uint32_t>(section,0x18,0);
    stream=native_mesh_vertex_stream_00b73260(*mesh,0);
    append_native_mesh_section_vertex_stream_00b85b80(*section,stream);
    set_native_mesh_section_vertex_layout_00b86650(*section,owners,read<void*>(payload,0x44));
    append_native_mesh_draw_section_00b73c60(*mesh,section);
    release_native_render_actual_owner(owners,section);
    release_native_render_actual_owner(owners,mesh);
    propagate_native_node_root_00b6d890(model.environment.nodes,model.node.transform,root);
}
} // namespace bsp
