#include "bsp/native_renderer_generated_model.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native generated model construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
void require(bool condition, const char* why) {
    if (!condition) throw std::invalid_argument(why);
}
void current_slot(void* renderer, const volatile std::uint32_t* profile,
    std::uint32_t byte_offset, std::uint32_t expected) {
    require(renderer && *static_cast<const volatile std::uint32_t*>(renderer)==0x00d5f0a8 &&
        profile && profile[byte_offset/4]==expected,
        "generated model requires the captured actual renderer and current native slot");
}
void sentinel_pair(const volatile std::uint32_t* source, float* pair) noexcept {
    __asm {
        mov eax,source
        mov edx,pair
        fld dword ptr [eax]
        fst dword ptr [edx+4]
        fstp dword ptr [edx]
    }
}
template<class T> void consume(NativeRenderActualOwners& owners, T*& creator) {
    void* const actual=creator;
    creator=nullptr; // A throwing terminal must not be retried.
    release_native_render_actual_owner(owners,actual);
}
void store(void* target, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(target)+byte_offset)=value;
}

bool raw_model_names_match(NativeRendererGeneratedModelContext& c,
    const NativeRendererRawModelBinding& raw) {
    auto& models = c.model_and_geometry.models;
    if (models.actual_names || !models.nodes.uses_raw_name_pool()) return false;
    const auto& names = models.nodes.require_raw_name_pool();
    return &names.actual_published_01090aa8 == &raw.names.actual_published_01090aa8 &&
        &names.actual_small_returns_disabled_01090aa4 == &raw.names.actual_small_returns_disabled_01090aa4 &&
        &names.actual_manager_publication_01090aa0 == &raw.names.actual_manager_publication_01090aa0 &&
        &raw.names.actual_manager_publication_01090aa0 == &c.actual_manager_01090aa0;
}

void* create_generated_model(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext& c,
    NativeRendererGeneratedModelAcquired& acquired,
    const NativeRendererRawModelBinding* raw) {
    auto& a=c.model_and_geometry;
    auto& owners=a.geometry.actual_owners();
    require(!acquired.started && !acquired.model && !acquired.model_owner &&
        !acquired.model_reference && !acquired.mesh.creator && !acquired.declaration.reference &&
        !acquired.vertex.creator && !acquired.index.creator && !acquired.material.creator &&
        !acquired.section.creator, "generated model requires fresh caller acquisition");
    require(a.prepare_model && a.retire_failed_model && a.bind_completed_model &&
        &a.models.retained_owners==&owners && &a.streams.actual_owners==&owners &&
        &a.materials.retained_owners==&owners &&
        (raw ? raw_model_names_match(c,*raw) : a.models.actual_names==&a.strings) &&
        &a.materials.parameter_names==&a.strings && &c.declarations.strings==&a.strings &&
        &c.declarations.declarations.strings==&a.strings && &c.effects.strings==&a.strings &&
        &a.models.constants.unchanged_00d7a260==&a.unchanged_00d7a260 &&
        static_cast<const volatile void*>(&a.streams.actual_renderer_00f8d394)==
            static_cast<const volatile void*>(&c.actual_renderer_00f8d394) &&
        &a.streams.actual_renderer_00f8d394==&c.indices.lifetime.actual_renderer_00f8d394 &&
        &a.streams.actual_physical==&c.indices.lifetime.actual_physical &&
        &a.streams.actual_synchronization_0108d6dc==&c.indices.lifetime.actual_synchronization_0108d6dc &&
        &a.streams.actual_synchronization_0108d6dc==&c.effects.synchronization_0108d6dc &&
        a.streams.actual_physical.actual_lifetime_01090aa0.borrows_same_domain(c.actual_manager_01090aa0) &&
        a.streams.actual_renderer_profile_00d5f0a8==c.indices.actual_renderer_profile_00d5f0a8,
        "generated model requires shared actual renderer, AA0, strings, pools and canonical owner contexts");
    acquired.started=true;
    acquired.active_call_site=0x00b4c725;
    void* const raw_model=a.models.pool_01090054.allocate_raw_slot_00b74d00();
    if (!raw_model) throw std::bad_alloc(); // Native later null dereference is outside source domain.
    acquired.model=raw_model;
    try {
        acquired.model_owner=&a.prepare_model(a.companion_context,raw_model,a.models);
        auto& model=*acquired.model_owner;
        require(&model.storage.node==raw_model && &model.environment==&a.models &&
            model.phase==NativeModelOwner::Phase::prepared,
            "generated model preparation must bind the same unused actual model");
        acquired.active_call_site=0x00b4c73b;
        if (raw)
            construct_native_model_00b75030(model,static_cast<const void*>(&model_name),raw->constants);
        else
            construct_native_model_00b75030(model,model_name);
    } catch (...) {
        // Native state0 CBFA60 -> B748C0 -> canonical model pool return.
        if (auto* model=acquired.model_owner) {
            acquired.model_owner=nullptr;
            a.retire_failed_model(a.companion_context,*model);
        }
        acquired.model=nullptr;
        a.models.pool_01090054.return_raw_slot_00b74750(raw_model);
        throw;
    }
    // Model allocation state is disarmed; later mesh failure must retain it.
    acquired.model_reference=&a.bind_completed_model(a.companion_context,*acquired.model_owner);
    require(&acquired.model_reference->model_owner()==acquired.model_owner &&
        &owners.resolve_actual(raw_model)==static_cast<RenderCommandReference*>(acquired.model_reference),
        "generated model must have its canonical actual reference");
    acquired.active_call_site=0x00b4c756;
    auto* const mesh=a.geometry.create_mesh(acquired.mesh);
    float scalars[2];
    sentinel_pair(&a.unchanged_00d7a260,scalars);
    acquired.active_call_site=0x00b4c792;
    set_native_model_geometry_00b75170(*acquired.model_owner,0,mesh,scalars[0],scalars[1]);

    acquired.active_call_site=0x00b4c7a3;
    void* renderer=c.actual_renderer_00f8d394;
    current_slot(renderer,a.streams.actual_renderer_profile_00d5f0a8,0x38,0x00b317e0);
    acquired.declaration.reference=load_native_renderer_vertex_declaration_00b317e0(
        renderer,&layout_name,c.declarations);
    if (!acquired.declaration.reference) throw std::bad_alloc();
    a.geometry.register_native_declaration_reference(acquired.declaration,c.declarations.declarations);
    acquired.active_call_site=0x00b4c7c6;
    renderer=c.actual_renderer_00f8d394;
    current_slot(renderer,a.streams.actual_renderer_profile_00d5f0a8,0x5c,0x00b287c0);
    acquired.vertex.vertex=true;
    acquired.vertex.phase=NativeStreamClonePhase::factory;
    (void)create_native_registered_vertex_stream_00b287c0(renderer,vertex_count,
        vertex_count ? 1u : 0x1000u,acquired.declaration.reference,a.streams,&acquired.vertex.creator);
    if (!acquired.vertex.creator) throw std::bad_alloc();
    acquired.vertex.phase=NativeStreamClonePhase::registration;
    a.geometry.register_stream_clone_creator(acquired.vertex,a.streams,c.indices);
    acquired.vertex.phase=NativeStreamClonePhase::complete;
    acquired.active_call_site=0x00b4c7ce;
    consume(owners,acquired.declaration.reference);
    acquired.active_call_site=0x00b4c7e5;
    set_native_mesh_vertex_stream_00b73bb0(*mesh,owners,0,acquired.vertex.creator);
    acquired.active_call_site=0x00b4c7ee;
    consume(owners,acquired.vertex.creator);
    acquired.vertex.phase=NativeStreamClonePhase::consumed;

    acquired.active_call_site=0x00b4c804;
    NativeMaterialFactoryRawContext material_context{c.effects,acquired.effect,
        a.streams.actual_renderer_profile_00d5f0a8};
    a.geometry.create_material_for_effect_00535320(effect_name,c.actual_renderer_00f8d394,
        a.materials,a.material_vtable_00d5e520,material_context,acquired.material);
    if (index_count) {
        acquired.active_call_site=0x00b4c82f;
        renderer=c.actual_renderer_00f8d394;
        current_slot(renderer,c.indices.actual_renderer_profile_00d5f0a8,0x60,0x00b288b0);
        acquired.index.phase=NativeStreamClonePhase::factory;
        (void)create_native_registered_index_stream_00b288b0(renderer,index_count,1,
            vertex_count>0xffffu ? 0x66u : 0x65u,c.indices,&acquired.index.creator);
        if (!acquired.index.creator) throw std::bad_alloc();
        acquired.index.phase=NativeStreamClonePhase::registration;
        a.geometry.register_stream_clone_creator(acquired.index,a.streams,c.indices);
        acquired.index.phase=NativeStreamClonePhase::complete;
        acquired.active_call_site=0x00b4c834;
        set_native_mesh_index_stream_00b73b70(*mesh,owners,acquired.index.creator);
        // No matching creator decrement occurs anywhere in native B4C700.
    }
    acquired.active_call_site=0x00b4c839;
    auto* const section=a.geometry.create_section(acquired.section);
    store(section,0x08,section_kind);
    store(section,0x0c,0);
    store(section,0x10,vertex_count);
    store(section,0x14,0);
    store(section,0x18,primitive_count);
    acquired.active_call_site=0x00b4c85c;
    rebuild_native_mesh_section_vertex_layout_00b865a0(*section,owners,mesh,c.layouts);
    acquired.active_call_site=0x00b4c864;
    set_native_mesh_section_material_00b864c0(*section,owners,acquired.material.creator);
    acquired.active_call_site=0x00b4c86c;
    append_native_mesh_draw_section_00b73c60(*mesh,section);
    acquired.active_call_site=0x00b4c87f;
    if (acquired.material.creator) consume(owners,acquired.material.creator);
    acquired.active_call_site=0x00b4c891;
    consume(owners,acquired.section.creator);
    acquired.active_call_site=0x00b4c8a7;
    if (acquired.mesh.creator) consume(owners,acquired.mesh.creator);
    acquired.active_call_site=0x00b4c8b9;
    acquired.complete=true;
    return raw_model;
}
} // namespace

void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext& c,
    NativeRendererGeneratedModelAcquired& acquired) {
    return create_generated_model(model_name,layout_name,effect_name,section_kind,
        vertex_count,primitive_count,index_count,c,acquired,nullptr);
}

void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext& c,
    NativeRendererGeneratedModelAcquired& acquired, const NativeRendererRawModelBinding& raw) {
    return create_generated_model(model_name,layout_name,effect_name,section_kind,
        vertex_count,primitive_count,index_count,c,acquired,&raw);
}
} // namespace bsp
