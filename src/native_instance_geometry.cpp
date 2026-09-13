#include "bsp/native_instance_geometry.hpp"
#include "bsp/gui_text_content.hpp"
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native instance geometry requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p)+offset);
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p)+offset)=value;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
void sentinel_pair(const volatile std::uint32_t* source, float* values) noexcept {
    // B4C94C/B4C959/B4C961: one actual FLD, FST second argument, FSTP first.
    __asm {
        mov eax,source
        mov edx,values
        fld dword ptr [eax]
        fst dword ptr [edx+4]
        fstp dword ptr [edx]
    }
}
template<class T> void consume(NativeRenderActualOwners& owners, T*& creator) {
    void* actual=creator;
    creator=nullptr;
    release_native_render_actual_owner(owners,actual);
}
}

void* native_instance_generator_declaration_00b556b0(const void* generator) noexcept {
    return read<void*>(generator,0x10);
}
void* native_instance_generator_layout_00b556c0(const void* generator) noexcept {
    return read<void*>(generator,0x14);
}

void* create_native_instance_geometry_00b4c8d0(const NativeString& name,
    const void* generator, const NativeMeshStorage& source,
    const NativeMeshSectionStorage& selected, NativeInstanceGeometryAccess& a,
    NativeInstanceGeometryAcquired& acquired) {
    auto& owners=a.geometry.actual_owners();
    require(!acquired.model_owner && !acquired.model_reference && !acquired.mesh &&
        !acquired.stream && !acquired.material && !acquired.section,
        "instance geometry requires an empty acquired record");
    require(a.prepare_model && a.retire_failed_model && a.bind_completed_model &&
        a.bind_completed_stream && &a.models.retained_owners==&owners &&
        &a.streams.actual_owners==&owners && &a.materials.retained_owners==&owners &&
        a.models.actual_names==&a.strings && &a.materials.parameter_names==&a.strings &&
        &a.models.constants.unchanged_00d7a260==&a.unchanged_00d7a260,
        "instance geometry requires the same actual owner, string and sentinel domains");

    // B74EB0 tail-enters this SAME canonical01090054 pool, ignoring size184h.
    void* const raw_model=a.models.pool_01090054.allocate_raw_slot_00b74d00();
    if (!raw_model) throw std::bad_alloc();
    NativeModelOwner* model=nullptr;
    try {
        model=&a.prepare_model(a.companion_context,raw_model,a.models);
        require(&model->storage.node==raw_model && &model->environment==&a.models &&
            model->phase==NativeModelOwner::Phase::prepared,
            "instance geometry preparation must bind the same unused actual model");
        construct_native_model_00b75030(*model,name);
    } catch (...) {
        if (model) a.retire_failed_model(a.companion_context,*model);
        // B4C8D0 state0 allocation cleanup; never destroy a completed model here.
        a.models.pool_01090054.return_raw_slot_00b74750(raw_model);
        throw;
    }
    acquired.model_owner=model;
    auto& model_reference=a.bind_completed_model(a.companion_context,*model);
    require(&model_reference.model_owner()==model &&
        &owners.resolve_actual(raw_model)==static_cast<RenderCommandReference*>(&model_reference),
        "instance geometry model must have one canonical actual reference");
    acquired.model_reference=&model_reference;

    acquired.mesh=a.geometry.create_mesh();
    auto* const mesh=acquired.mesh;
    float scalars[2];
    sentinel_pair(&a.unchanged_00d7a260,scalars);
    set_native_model_geometry_00b75170(*model,0,mesh,scalars[0],scalars[1]);

    // Capture renderer and CURRENT table before the declaration getter, exactly
    // as B4C96B/B4C971. Numeric native targets are dispatched through their real
    // reconstructed bodies; no assumption that a native address is host code.
    void* const renderer=const_cast<void*>(a.streams.actual_renderer_00f8d394);
    const auto renderer_table=read<std::uint32_t>(renderer,0);
    const auto* const profile=a.streams.actual_renderer_profile_00d5f0a8;
    void* const declaration=native_instance_generator_declaration_00b556b0(generator);
    require(renderer_table==0x00d5f0a8 && profile && profile[0x5c/4]==0x00b287c0,
        "instance geometry requires current native renderer5C B287C0");
    acquired.stream=create_native_registered_vertex_stream_00b287c0(
        renderer,0,0x1000,declaration,a.streams);
    if (!acquired.stream) throw std::bad_alloc();
    auto& stream_reference=a.bind_completed_stream(a.companion_context,acquired.stream,a.streams);
    require(stream_reference.storage()==acquired.stream &&
        &owners.resolve_actual(acquired.stream)==static_cast<RenderCommandReference*>(&stream_reference),
        "instance geometry stream must have one canonical actual reference");
    set_native_mesh_vertex_stream_00b73bb0(*mesh,owners,0,read<void*>(&selected,0x3c));
    set_native_mesh_vertex_stream_00b73bb0(*mesh,owners,1,acquired.stream);
    write<std::uint32_t>(acquired.stream,0x54,0x80000000);
    consume(owners,acquired.stream);

    const auto* const source_material=read<NativeMaterialStorage*>(&selected,0x20);
    acquired.material=a.geometry.clone_material_00b18b60(
        *source_material,a.materials,a.material_vtable_00d5e520);
    set_native_mesh_index_stream_00b73b70(*mesh,owners,read<void*>(&source,0x60));
    acquired.section=a.geometry.create_section();
    auto* const section=acquired.section;
    write(section,0x08,read<std::uint32_t>(&selected,0x08));
    write(section,0x0c,read<std::uint32_t>(&selected,0x0c));
    write(section,0x14,read<std::uint32_t>(&selected,0x14));
    write(section,0x18,read<std::uint32_t>(&selected,0x18));
    write(section,0x10,read<std::uint32_t>(&selected,0x10));
    set_native_mesh_section_material_00b864c0(*section,owners,acquired.material);
    append_native_mesh_draw_section_00b73c60(*mesh,section);
    append_native_mesh_section_vertex_stream_00b85b80(*section,
        native_mesh_vertex_stream_00b73260(*mesh,0));
    append_native_mesh_section_vertex_stream_00b85b80(*section,
        native_mesh_vertex_stream_00b73260(*mesh,1));
    set_native_mesh_section_vertex_layout_00b86650(*section,owners,
        native_instance_generator_layout_00b556c0(generator));
    consume(owners,acquired.material);
    consume(owners,acquired.section);
    consume(owners,acquired.mesh);
    return raw_model;
}
} // namespace bsp
