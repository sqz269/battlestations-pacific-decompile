#include "bsp/native_renderer_generated_model.hpp"

#include "bsp/native_mesh_pool.hpp"
#include "bsp/native_model_pool.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"

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

void require_current_slot(void* renderer, const volatile std::uint32_t* profile,
    std::uint32_t byte_offset, std::uint32_t expected) {
    require(renderer &&
        *static_cast<const volatile std::uint32_t*>(renderer) == 0x00d5f0a8u &&
        profile && profile[byte_offset / 4] == expected,
        "generated model requires the captured actual renderer and current native slot");
}

void sentinel_pair(const volatile std::uint32_t* source, float* pair) noexcept {
    __asm {
        mov eax, source
        mov edx, pair
        fld dword ptr [eax]
        fst dword ptr [edx + 4]
        fstp dword ptr [edx]
    }
}

template<class T>
void consume(NativeRenderActualOwners& owners, T*& creator) {
    void* const actual = creator;
    creator = nullptr; // A throwing terminal must not be retried.
    release_native_render_actual_owner(owners, actual);
}

void store(void* target, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(target) + byte_offset) = value;
}

bool raw_model_names_match(NativeRendererGeneratedModelContext& context,
    const NativeRendererRawModelBinding& raw) {
    auto& models = context.model_and_geometry.models;
    if (models.actual_names || !models.nodes.uses_raw_name_pool()) return false;
    const auto& names = models.nodes.require_raw_name_pool();
    return &names.actual_published_01090aa8 == &raw.names.actual_published_01090aa8 &&
        &names.actual_small_returns_disabled_01090aa4 ==
            &raw.names.actual_small_returns_disabled_01090aa4 &&
        &names.actual_manager_publication_01090aa0 ==
            &raw.names.actual_manager_publication_01090aa0 &&
        &raw.names.actual_manager_publication_01090aa0 ==
            &context.actual_manager_01090aa0;
}

void* create_generated_model(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext& context,
    NativeRendererGeneratedModelAcquired& acquired,
    const NativeRendererRawModelBinding* raw) {
    auto& access = context.model_and_geometry;
    auto& owners = access.geometry.actual_owners();
    require(!acquired.started && !acquired.raw_model && !acquired.model_owner &&
        !acquired.model_reference && !acquired.mesh.started &&
        !acquired.declaration.reference && !acquired.vertex.creator &&
        acquired.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        !acquired.index.creator && !acquired.section.started,
        "generated model requires a fresh caller acquisition");
    require(access.prepare_model && access.retire_failed_model &&
        access.bind_completed_model &&
        &access.models.retained_owners == &owners &&
        &access.streams.actual_owners == &owners &&
        &access.materials.retained_owners == &owners &&
        (raw ? raw_model_names_match(context, *raw) :
            access.models.actual_names == &access.strings) &&
        &access.materials.parameter_names == &access.strings &&
        &context.declarations.strings == &access.strings &&
        &context.declarations.declarations.strings == &access.strings &&
        &context.effects.strings == &access.strings &&
        &access.models.constants.unchanged_00d7a260 == &access.unchanged_00d7a260 &&
        static_cast<const volatile void*>(&access.streams.actual_renderer_00f8d394) ==
            static_cast<const volatile void*>(&context.actual_renderer_00f8d394) &&
        static_cast<const volatile void*>(
            &context.effects.effects.construction.current_renderer_00f8d394) ==
            static_cast<const volatile void*>(&context.actual_renderer_00f8d394) &&
        &access.streams.actual_renderer_00f8d394 ==
            &context.indices.lifetime.actual_renderer_00f8d394 &&
        &access.streams.actual_physical == &context.indices.lifetime.actual_physical &&
        &access.streams.actual_synchronization_0108d6dc ==
            &context.indices.lifetime.actual_synchronization_0108d6dc &&
        &access.streams.actual_synchronization_0108d6dc ==
            &context.effects.synchronization_0108d6dc &&
        access.streams.actual_physical.actual_lifetime_01090aa0.borrows_same_domain(
            context.actual_manager_01090aa0) &&
        access.streams.actual_renderer_profile_00d5f0a8 ==
            context.indices.actual_renderer_profile_00d5f0a8 &&
        access.streams.actual_renderer_profile_00d5f0a8 ==
            context.effects.effects.renderer_profile_00d5f0a8,
        "generated model requires shared actual renderer, AA0, strings, pools and owners");

    acquired.started = true;
    acquired.active_call_site = 0x00b4c725;
    void* const raw_model = allocate_native_model_slot_00b74eb0();
    if (!raw_model) throw std::bad_alloc();
    acquired.raw_model = raw_model;
    try {
        acquired.model_owner = &access.prepare_model(
            access.companion_context, raw_model, access.models);
        auto& model = *acquired.model_owner;
        require(&model.storage.node == raw_model && &model.environment == &access.models &&
            model.phase == NativeModelOwner::Phase::prepared,
            "generated model preparation must bind the same unused actual model");
        acquired.active_call_site = 0x00b4c73b;
        if (raw) {
            construct_native_model_00b75030(
                model, static_cast<const void*>(&model_name), raw->constants);
        } else {
            construct_native_model_00b75030(model, model_name);
        }
    } catch (...) {
        if (auto* model = acquired.model_owner) {
            acquired.model_owner = nullptr;
            access.retire_failed_model(access.companion_context, *model);
        }
        acquired.raw_model = nullptr;
        return_native_model_slot_00b748c0(raw_model);
        throw;
    }
    acquired.model_reference = &access.bind_completed_model(
        access.companion_context, *acquired.model_owner);
    require(&acquired.model_reference->model_owner() == acquired.model_owner &&
        &owners.resolve_actual(raw_model) ==
            static_cast<RenderCommandReference*>(acquired.model_reference),
        "generated model must have its canonical actual reference");

    acquired.active_call_site = 0x00b4c756;
    auto* const mesh = access.geometry.create_native_mesh_00b73b60(acquired.mesh);
    require(mesh != nullptr,
        "native B4C700 mesh allocation returned null before its later dereference");
    float scalars[2];
    sentinel_pair(&access.unchanged_00d7a260, scalars);
    acquired.active_call_site = 0x00b4c792;
    set_native_model_geometry_00b75170(
        *acquired.model_owner, 0, mesh, scalars[0], scalars[1]);

    acquired.active_call_site = 0x00b4c7a3;
    void* renderer = context.actual_renderer_00f8d394;
    require_current_slot(renderer, access.streams.actual_renderer_profile_00d5f0a8,
        0x38, 0x00b317e0);
    acquired.declaration.reference = load_native_renderer_vertex_declaration_00b317e0(
        renderer, &layout_name, context.declarations);
    if (!acquired.declaration.reference) throw std::bad_alloc();
    access.geometry.register_native_declaration_reference(
        acquired.declaration, context.declarations.declarations);

    acquired.active_call_site = 0x00b4c7c6;
    renderer = context.actual_renderer_00f8d394;
    require_current_slot(renderer, access.streams.actual_renderer_profile_00d5f0a8,
        0x5c, 0x00b287c0);
    acquired.vertex.vertex = true;
    acquired.vertex.phase = NativeStreamClonePhase::factory;
    create_native_registered_vertex_stream_00b287c0(renderer, vertex_count,
        vertex_count ? 1u : 0x1000u, acquired.declaration.reference,
        access.streams, &acquired.vertex.creator);
    if (!acquired.vertex.creator) throw std::bad_alloc();
    acquired.vertex.phase = NativeStreamClonePhase::registration;
    access.geometry.register_stream_clone_creator(
        acquired.vertex, access.streams, context.indices);
    acquired.vertex.phase = NativeStreamClonePhase::complete;

    acquired.active_call_site = 0x00b4c7ce;
    consume(owners, acquired.declaration.reference);
    acquired.active_call_site = 0x00b4c7e5;
    set_native_mesh_vertex_stream_00b73bb0(
        *mesh, owners, 0, acquired.vertex.creator);
    acquired.active_call_site = 0x00b4c7ee;
    consume(owners, acquired.vertex.creator);
    acquired.vertex.phase = NativeStreamClonePhase::consumed;

    acquired.active_call_site = 0x00b4c804;
    create_native_material_from_effect_cache_00535320(effect_name,
        access.materials.material_slots, owners, context.effects, acquired.material);
    if (!acquired.material.material) throw std::bad_alloc();
    access.geometry.register_native_material_creator(acquired.material,
        access.materials, access.material_vtable_00d5e520);

    if (index_count) {
        acquired.active_call_site = 0x00b4c82f;
        renderer = context.actual_renderer_00f8d394;
        require_current_slot(renderer, context.indices.actual_renderer_profile_00d5f0a8,
            0x60, 0x00b288b0);
        acquired.index.phase = NativeStreamClonePhase::factory;
        create_native_registered_index_stream_00b288b0(renderer, index_count, 1,
            vertex_count > 0xffffu ? 0x66u : 0x65u, context.indices,
            &acquired.index.creator);
        if (!acquired.index.creator) throw std::bad_alloc();
        acquired.index.phase = NativeStreamClonePhase::registration;
        access.geometry.register_stream_clone_creator(
            acquired.index, access.streams, context.indices);
        acquired.index.phase = NativeStreamClonePhase::complete;
        acquired.active_call_site = 0x00b4c834;
        set_native_mesh_index_stream_00b73b70(*mesh, owners, acquired.index.creator);
        // Native B4C700 performs no balancing creator decrement.
    }

    acquired.active_call_site = 0x00b4c839;
    auto* const section =
        access.geometry.create_native_section_00533fa0(acquired.section);
    require(section != nullptr,
        "native B4C700 section allocation returned null before its field stores");
    store(section, 0x08, section_kind);
    store(section, 0x0c, 0);
    store(section, 0x10, vertex_count);
    store(section, 0x14, 0);
    store(section, 0x18, primitive_count);
    acquired.active_call_site = 0x00b4c85c;
    rebuild_native_mesh_section_vertex_layout_00b865a0(
        *section, owners, mesh, context.layouts);
    acquired.active_call_site = 0x00b4c864;
    set_native_mesh_section_material_00b864c0(
        *section, owners, acquired.material.material);
    acquired.active_call_site = 0x00b4c86c;
    append_native_mesh_draw_section_00b73c60(*mesh, section);

    acquired.active_call_site = 0x00b4c87f;
    consume(owners, acquired.material.material);
    acquired.active_call_site = 0x00b4c891;
    consume(owners, acquired.section.creator);
    acquired.active_call_site = 0x00b4c8a7;
    consume(owners, acquired.mesh.creator);
    acquired.active_call_site = 0x00b4c8b9;
    acquired.complete = true;
    return raw_model;
}

} // namespace

void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext& context,
    NativeRendererGeneratedModelAcquired& acquired) {
    return create_generated_model(model_name, layout_name, effect_name, section_kind,
        vertex_count, primitive_count, index_count, context, acquired, nullptr);
}

void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext& context,
    NativeRendererGeneratedModelAcquired& acquired,
    const NativeRendererRawModelBinding& raw) {
    return create_generated_model(model_name, layout_name, effect_name, section_kind,
        vertex_count, primitive_count, index_count, context, acquired, &raw);
}

} // namespace bsp
