#include "bsp/native_particle_model_construction.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_particle_model_pool_allocate.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-model construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T load(const void* actual, std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(actual) + offset, sizeof result);
    return result;
}
template<class T> T& required(T* actual) {
    if (!actual) throw std::logic_error("particle constructor reached an invalid null native owner");
    return *actual;
}
template<class T> T* place_preserving(void* slot) {
    if (!slot) throw std::invalid_argument("particle constructor requires actual aligned storage");
    std::array<std::byte, sizeof(T)> preimage;
    std::memcpy(preimage.data(), slot, preimage.size());
    T* result = ::new (slot) T;
    std::memcpy(slot, preimage.data(), preimage.size());
    return result;
}
void retain(void* actual) noexcept {
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(actual) + 4));
    count.fetch_add(1, std::memory_order_seq_cst);
}
void bits(float& target, std::uint32_t value) noexcept { std::memcpy(&target, &value, 4); }
float float_bits(std::uint32_t value) noexcept { float result; bits(result, value); return result; }
void copy_identity(CameraMatrix& matrix, const volatile std::uint32_t& one) {
    const auto diagonal = one;
    const std::uint32_t source[16] = {diagonal,0,0,0,0,diagonal,0,0,0,0,diagonal,0,0,0,0,diagonal};
    copy_native_camera_matrix_004134f0(&matrix, nullptr, source);
}
float reciprocal(std::int32_t divisor) noexcept {
    float result;
    __asm {
        fild divisor
        fld1
        fdivrp st(1), st(0)
        fstp result
    }
    return result;
}
std::uint32_t truncate_low_word(float source) noexcept {
    unsigned short old_control, truncate_control;
    __int64 integer;
    __asm { fnstcw old_control }
    truncate_control = static_cast<unsigned short>(old_control | 0x0c00u);
    __asm {
        fld source
        fldcw truncate_control
        fistp integer
        fldcw old_control
    }
    return static_cast<std::uint32_t>(integer);
}
void* allocate_object(std::size_t size) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
}
NativeMeshStorage* create_mesh(NativeParticleModelConstructionAccess& access) {
    void* raw = access.meshes.pool_0108fff8.allocate_slot_00b73a10();
    if (!raw) return nullptr;
    // B73D70 cannot throw in its supported constant/storage domain; states2/8
    // still identify the caller's raw-slot return if a native constructor fails.
    NativeMeshStorage* result;
    try { result = construct_native_mesh_00b73d70(raw, access.mesh_constants); }
    catch (...) { access.meshes.pool_0108fff8.return_slot_00b72da0(raw); throw; }
    access.callees.bind_mesh(*result);
    return result;
}
NativeMeshSectionStorage& create_section(NativeParticleModelConstructionAccess& access) {
    auto& section = required(create_native_mesh_section_00533fa0(
        access.sections.pool_010901d4, access.mesh_constants.maximum_00ce4970));
    access.callees.bind_section(section);
    section.primitive_08 = 4;
    section.range_words_0c[0] = 0;
    section.range_words_0c[2] = 0;
    section.range_words_0c[3] = 0;
    section.range_words_0c[1] = 0;
    return section;
}
NativeMaterialStorage* create_material(NativeParticleModelConstructionAccess& access,
    const void* captured_variant, bool second) {
    void* raw = allocate_native_material_slot_00b18780(access.materials.material_slots);
    if (!raw) return nullptr;
    NativeMaterialStorage* material;
    try {
        NativeMaterialStorage* source;
        if (second) source = access.callees.call_00af1120(access.resources_00f8c280);
        else {
            const auto flag79 = load<std::uint8_t>(captured_variant, 0x79);
            const auto flag78 = load<std::uint8_t>(captured_variant, 0x78);
            source = access.callees.call_00af10f0(access.resources_00f8c280, flag78, flag79);
        }
        material = clone_native_material_00b18b60(raw, required(source), access.materials.retained_owners);
    } catch (...) { access.materials.material_slots.return_slot_00b17a80(raw); throw; }
    access.callees.bind_material(*material);
    return material;
}
template<class Source> void register_parameter(NativeMaterialStorage& material, NativeParticleModelConstructionAccess& access,
    const char* text, std::uint32_t length, Source source,
    std::uint32_t count, std::uint8_t matrix) {
    NativeString name;
    name.resize_0041dd40(access.parameters.parameter_names, length, true);
    if (name.data()) std::memcpy(name.data(), text, name.block_size());
    // Native states4..7/10..11 arm only AFTER resize and memcpy.
    try {
        register_native_material_parameter_00b17e10(material, &name, source(), count, matrix, access.parameters);
    } catch (...) {
        destroy_native_string_header_0041dd20(&name, access.parameters.parameter_names);
        throw;
    }
    destroy_native_string_header_0041dd20(&name, access.parameters.parameter_names);
}
void register_matrices(NativeMaterialStorage& material, NativeParticleModelTailStorage& tail,
    NativeParticleModelConstructionAccess& access) {
    if (!tail.flag_1b0) {
        copy_identity(tail.custom_world_218, access.one_00d7a24c);
        copy_identity(tail.custom_inverse_world_258, access.one_00d7a24c);
    }
    register_parameter(material, access, "cCustWorldMat", 13, [&] { return &tail.custom_world_218; }, 16, 1);
    register_parameter(material, access, "cCustInvWorldMat", 16, [&] { return &tail.custom_inverse_world_258; }, 16, 1);
}
void finish_mesh(NativeMeshStorage*& current_mesh, NativeMeshSectionStorage& section,
    NativeParticleModelConstructionAccess& access) {
    // Native captures BOTH renderer and virtual entry before AF10B0.
    void* renderer = access.renderer_00f8d394;
    const void* table = load<void*>(renderer, 0);
    const auto entry = load<std::uint32_t>(table, 0x5c);
    void* descriptor = access.callees.call_00af10b0(access.resources_00f8c280);
    void* stream = access.callees.renderer_virtual5c(renderer, entry, 0, 0x1000, descriptor);
    auto& owners = access.materials.retained_owners;
    set_native_mesh_vertex_stream_00b73bb0(required(current_mesh), owners, 0, stream);
    if (stream) release_native_render_actual_owner(owners, stream);
    rebuild_native_mesh_section_vertex_layout_00b865a0(section, owners, current_mesh, access.layouts);
    append_native_mesh_draw_section_00b73c60(required(current_mesh), &section);
    release_native_render_actual_owner(owners, &section);
}
void destroy_emitter_backing(NativeRenderPointerArrayStorage& array) noexcept {
    // AF6B70 -> AF6180(0), then BF6989; raw cells are not released.
    while (array.count_04 > 0) --array.count_04;
    array.count_04 = 0;
    singleton_lifetime_free(array.data_00);
}
} // namespace

NativeParticleEmitterStorage* construct_native_particle_emitter_00aff5f0(
    void* raw, NativeNodeStorage& model, void* definition) {
    auto* emitter = place_preserving<NativeParticleEmitterStorage>(raw);
    emitter->model_08 = &model;
    emitter->definition_0c = definition;
    emitter->vtable_00 = 0x00ceb130;
    emitter->references_04.store(1, std::memory_order_relaxed);
    emitter->vtable_00 = 0x00d5dcb4;
    retain(definition);
    emitter->container_10 = nullptr;
    for (auto& word : emitter->words_14) word = 0;
    return emitter;
}

void reserve_native_particle_emitter_pointers_00af6120(
    NativeRenderPointerArrayStorage& array, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (array.capacity_08 >= capacity) return;
    const auto bytes = static_cast<std::uint32_t>(capacity) * 4u;
    auto** replacement = static_cast<void**>(singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes}));
    for (std::int32_t i = 0; i < array.count_04; ++i) {
        const auto address = reinterpret_cast<std::uintptr_t>(replacement) + static_cast<std::uint32_t>(i) * 4u;
        if (address) *reinterpret_cast<void**>(address) = array.data_00[i];
    }
    singleton_lifetime_free(array.data_00);
    array.data_00 = replacement; // verified6174; absent in stale pseudocode
    array.capacity_08 = capacity; // verified6176
}

NativeParticleModelArraysStorage* construct_native_particle_model_arrays_00afd2e0(
    void* raw, NativeNodeStorage& model, std::int32_t count,
    NativeParticleModelConstructionCallees& callees) {
    auto* arrays = place_preserving<NativeParticleModelArraysStorage>(raw);
    arrays->bytes_04.data_00 = nullptr;
    arrays->bytes_04.count_04 = 0;
    arrays->records_0c.data_00 = nullptr;
    arrays->records_0c.count_04 = 0;
    arrays->model_00 = &model;
    try {
        callees.call_00afd130(arrays->bytes_04, count);
        callees.call_00afd220(arrays->records_0c, count);
        for (std::int32_t i = 0; i < count; ++i)
            static_cast<std::uint8_t*>(arrays->bytes_04.data_00)[i] = static_cast<std::uint8_t>(i);
        arrays->word_14 = 0;
    } catch (...) {
        try {
            callees.call_00afd1e0(arrays->records_0c);
            callees.call_00afd0f0(arrays->bytes_04);
        } catch (...) { std::terminate(); }
        throw;
    }
    return arrays;
}

NativeNodeStorage* construct_native_particle_model_00af74a0(
    NativeModelOwner& base, void* variant, NativeParticleModelConstructionAccess& access) {
    if (!variant || base.phase != NativeModelOwner::Phase::prepared ||
        &base.environment.retained_owners != &access.materials.retained_owners ||
        &access.meshes.retained_owners != &access.materials.retained_owners ||
        &access.sections.retained_owners != &access.materials.retained_owners ||
        &access.materials.parameter_names != &access.parameters.parameter_names)
        throw std::invalid_argument("particle model requires a prepared actual slot and one canonical ownership domain");
    auto& node = base.storage.node;
    auto& tail = *place_preserving<NativeParticleModelTailStorage>(
        reinterpret_cast<std::byte*>(&node) + 0x184);
    // The native producer places its actual NativeString at variant+08.
    const auto& name = *std::launder(reinterpret_cast<const NativeString*>(static_cast<std::byte*>(variant) + 8));
    construct_native_model_00b75030(base, name);
    node.vtable_00 = 0x00d5da50;
    tail.emitters_194 = {nullptr, 0, 0};
    bits(tail.scalar_1dc, access.one_00d7a24c);
    tail.active_1a4 = 1;
    tail.initialized_1a5 = 0;
    bits(tail.scalar_1e4, 0);
    tail.variant_18c = variant;
    access.callees.bind_particle_profile(base);
    try {
        access.callees.call_00af40e0(variant);
        retain(variant);
        copy_identity(tail.point_matrix_298, access.one_00d7a24c);
        void* current_variant = tail.variant_18c;
        tail.flag_1b0 = load<std::uint8_t>(current_variant, 0x64);
        tail.flag_1b1 = load<std::uint8_t>(current_variant, 0x65);
        tail.mesh_1b4 = create_mesh(access);
        const auto unchanged = base.environment.constants.unchanged_00d7a260;
        set_native_model_geometry_00b75170(base, 0, tail.mesh_1b4, float_bits(unchanged), float_bits(unchanged));
        void* index = access.callees.call_00af10a0(access.resources_00f8c280);
        set_native_mesh_index_stream_00b73b70(required(tail.mesh_1b4), access.materials.retained_owners, index);
        auto& first_section = create_section(access);
        auto& first_material = required(create_material(access, variant, false));
        void* texture = access.callees.call_00b0d140(access.shadow_00f8d39c);
        set_native_material_texture_00b189f0(first_material, 1, texture, access.materials.retained_owners);
        texture = access.callees.call_00b0d130(access.shadow_00f8d39c);
        set_native_material_texture_00b189f0(first_material, 2, texture, access.materials.retained_owners);
        set_native_mesh_section_material_00b864c0(first_section, access.materials.retained_owners, &first_material);
        register_matrices(first_material, tail, access);
        register_parameter(first_material, access, "cColorBurn", 10,
            [&] { return static_cast<std::byte*>(tail.variant_18c) + 0x74; }, 1, 0);
        register_parameter(first_material, access, "cDepthSampleOffset", 18,
            [&] { return static_cast<std::byte*>(access.shadow_00f8d39c) + 4; }, 2, 0);
        release_native_render_actual_owner(access.materials.retained_owners, &first_material);
        finish_mesh(tail.mesh_1b4, first_section, access);

        tail.mesh_1c8 = create_mesh(access);
        index = access.callees.call_00af10a0(access.resources_00f8c280);
        set_native_mesh_index_stream_00b73b70(required(tail.mesh_1c8), access.materials.retained_owners, index);
        auto& second_section = create_section(access);
        auto& second_material = required(create_material(access, nullptr, true));
        texture = access.callees.call_00b0d130(access.shadow_00f8d39c);
        set_native_material_texture_00b189f0(second_material, 1, texture, access.materials.retained_owners);
        set_native_mesh_section_material_00b864c0(second_section, access.materials.retained_owners, &second_material);
        register_matrices(second_material, tail, access);
        release_native_render_actual_owner(access.materials.retained_owners, &second_material);
        finish_mesh(tail.mesh_1c8, second_section, access);

        current_variant = tail.variant_18c;
        bits(tail.scalar_184, 0);
        bits(tail.scalar_188, 0);
        const auto divisor = load<std::int32_t>(current_variant, 0x5c);
        bits(tail.scalar_1e0, 0);
        tail.reciprocal_1a0 = reciprocal(divisor);
        std::int32_t i = 0;
        if (load<std::int32_t>(current_variant, 0x54) > 0) do {
            void* definition = load<void*>(tail.variant_18c, 0x34 + static_cast<std::size_t>(i) * 4);
            void* raw = allocate_object(0x28);
            NativeParticleEmitterStorage* emitter = nullptr;
            if (raw) {
                try { emitter = construct_native_particle_emitter_00aff5f0(raw, node, definition); }
                catch (...) { singleton_lifetime_free(raw); throw; }
            }
            auto& array = tail.emitters_194;
            if (array.count_04 == array.capacity_08) {
                const auto doubled_bits = static_cast<std::uint32_t>(array.capacity_08) * 2u;
                std::int32_t doubled;
                std::memcpy(&doubled, &doubled_bits, 4);
                reserve_native_particle_emitter_pointers_00af6120(array, doubled > 1 ? doubled : 1);
            }
            const auto address = reinterpret_cast<std::uintptr_t>(array.data_00) + static_cast<std::uint32_t>(array.count_04) * 4u;
            if (address) *reinterpret_cast<void**>(address) = emitter;
            ++array.count_04;
            ++i;
        } while (i < load<std::int32_t>(tail.variant_18c, 0x54));

        void* raw = allocate_object(0x18);
        NativeParticleModelArraysStorage* arrays = nullptr;
        if (raw) {
            const auto count = load<std::int32_t>(tail.variant_18c, 0x58);
            try { arrays = construct_native_particle_model_arrays_00afd2e0(raw, node, count, access.callees); }
            catch (...) { singleton_lifetime_free(raw); throw; }
        }
        const auto random_max = access.random_max_00ce3d64;
        const auto one = access.one_00d7a24c;
        tail.arrays_190 = arrays;
        tail.words_1a8[0] = 0; tail.words_1a8[1] = 0;
        access.live_count_00f8d2c8 = access.live_count_00f8d2c8 + 1u;
        bits(tail.scalar_1d8, one);
        tail.flag_1d4 = 0;
        tail.words_1ec[0] = 0;
        bits(tail.scalar_1e8, one);
        tail.words_1ec[1] = 0; tail.words_1ec[2] = 0; tail.words_1ec[3] = 0;
        const float random_value = access.callees.call_00bd2f10(
            access.random, RandomStream::primary, 0.0f, float_bits(random_max));
        auto* child = reinterpret_cast<NativeNodeStorage*>(node.first_child_34);
        node.mask_48 = 0x1b;
        tail.random_1fc = truncate_low_word(random_value);
        while (child) {
            access.callees.call_007099c0(*child, 0x1b);
            child = reinterpret_cast<NativeNodeStorage*>(child->next_sibling_3c);
        }
        access.callees.call_00af0950(access.manager_00f8c274, node);
    } catch (...) {
        try {
            destroy_emitter_backing(tail.emitters_194);
            destroy_native_model_00b750c0(base);
        } catch (...) { std::terminate(); }
        access.callees.retire_failed_particle_profile(base);
        throw;
    }
    return &node;
}
} // namespace bsp
