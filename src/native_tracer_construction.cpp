#include "bsp/native_tracer_construction.hpp"
#include "bsp/native_tracer_lifetime.hpp"
#include "bsp/native_tracer_update.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tracer construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T load(const void* actual, std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(actual) + offset, sizeof result);
    return result;
}
template<class T> void store(void* actual, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(actual) + offset, &value, sizeof value);
}
void* at(void* actual, std::size_t offset) noexcept {
    return static_cast<std::byte*>(actual) + offset;
}
template<class T> T& required(T* actual) {
    if (!actual) throw std::logic_error("tracer constructor reached an invalid null native owner");
    return *actual;
}
void retain(void* actual) noexcept {
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(actual, 4)));
    count.fetch_add(1, std::memory_order_seq_cst);
}
// The native constructor's FLD/FST/FSTP duplicates one load, including the
// x87 conversion of signaling NaNs and the caller's floating environment.
void geometry_scalars(float* result, const volatile std::uint32_t* source) noexcept {
    __asm {
        mov eax, source
        mov edx, result
        fld dword ptr [eax]
        fst dword ptr [edx + 4]
        fstp dword ptr [edx]
    }
}
void copy_point(void* destination, const void* source) noexcept {
    // Ten ordered FLD32/FSTP32 pairs: a memcpy would preserve an sNaN that
    // BAC0BF..BAC0F6 quiets and would miss its x87 invalid/denormal flags.
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + 8]
        fld dword ptr [eax + 0ch]
        fstp dword ptr [edx + 0ch]
        fld dword ptr [eax + 10h]
        fstp dword ptr [edx + 10h]
        fld dword ptr [eax + 14h]
        fstp dword ptr [edx + 14h]
        fld dword ptr [eax + 18h]
        fstp dword ptr [edx + 18h]
        fld dword ptr [eax + 1ch]
        fstp dword ptr [edx + 1ch]
        fld dword ptr [eax + 20h]
        fstp dword ptr [edx + 20h]
        fld dword ptr [eax + 24h]
        fstp dword ptr [edx + 24h]
        mov ecx, [eax + 28h]
        mov [edx + 28h], ecx
        mov ecx, [eax + 2ch]
        mov [edx + 2ch], ecx
    }
}
NativeMaterialParameterStorage* parameter(void* actual,
    NativeTracerConstructionAccess& access, const char* text,
    std::size_t source_offset, std::uint32_t vectors, std::size_t result_offset = 0) {
    NativeString name;
    name.assign_0041e870(access.parameters.parameter_names, text);
    NativeMaterialParameterStorage* result;
    // States6..10 arm AFTER the string constructor; result788/78C is
    // published before the native string destructor may observe actual state.
    try {
        result = register_native_material_parameter_00b17e10(
            required(load<NativeMaterialStorage*>(actual, 0x1bc)), &name,
            at(actual, source_offset), vectors * 4u, 0, access.parameters);
        if (result_offset) store(actual, result_offset, result);
    } catch (...) {
        destroy_native_string_header_0041dd20(&name, access.parameters.parameter_names);
        throw;
    }
    destroy_native_string_header_0041dd20(&name, access.parameters.parameter_names);
    return result;
}
NativeMaterialStorage* material(void* actual_template, NativeTracerConstructionAccess& access) {
    void* raw = allocate_native_material_slot_00b18780(access.materials.material_slots);
    if (!raw) return nullptr;
    NativeMaterialStorage* result;
    try {
        result = initialize_native_material_00b18900(raw,
            load<void*>(actual_template, 0x14), access.materials.retained_owners);
    } catch (...) {
        access.materials.material_slots.return_slot_00b17a80(raw); // state5 B17D70
        throw;
    }
    access.callees.bind_material(*result);
    return result;
}
NativeMeshStorage* mesh(NativeTracerConstructionAccess& access) {
    void* raw = access.meshes.pool_0108fff8.allocate_slot_00b73a10();
    if (!raw) return nullptr;
    NativeMeshStorage* result;
    try { result = construct_native_mesh_00b73d70(raw, access.mesh_constants); }
    catch (...) { access.meshes.pool_0108fff8.return_slot_00b72da0(raw); throw; }
    access.callees.bind_mesh(*result);
    return result;
}
} // namespace

void reserve_native_tracer_points_00bac070(NativeTracerPointArrayStorage& array,
    std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (array.capacity_08 >= capacity) return;
    const auto bytes = static_cast<std::uint32_t>(capacity) * 0x30u;
    void* replacement = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    for (std::int32_t i = 0; i < array.count_04; ++i) {
        const auto offset = static_cast<std::uint32_t>(i) * 0x30u;
        const auto destination = reinterpret_cast<std::uintptr_t>(replacement) + offset;
        if (destination) copy_point(reinterpret_cast<void*>(destination),
            reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(array.data_00) + offset));
    }
    singleton_lifetime_free(array.data_00);
    array.data_00 = replacement; // BAC120 returning-free continuation
    array.capacity_08 = capacity; // BAC122; count remains current
}
void resize_native_tracer_points_00bac310(NativeTracerPointArrayStorage& array,
    std::int32_t count) {
    if (array.capacity_08 < count) reserve_native_tracer_points_00bac070(array, count);
    const auto initial_count = array.count_04;
    if (initial_count < count) {
        auto offset = static_cast<std::uint32_t>(initial_count) * 0x30u;
        auto remaining = static_cast<std::uint32_t>(count) - static_cast<std::uint32_t>(initial_count);
        do {
            const auto address = reinterpret_cast<std::uintptr_t>(array.data_00) + offset;
            if (address) {
                store(reinterpret_cast<void*>(address), 0x28, std::uint32_t{0});
                store(reinterpret_cast<void*>(address), 0x2c, std::uint32_t{0});
            }
            offset += 0x30u;
        } while (--remaining);
    }
    while (count < array.count_04) --array.count_04;
    array.count_04 = count;
}

RegisteredType4TracerView construct_native_tracer_00bad6f0(
    NativeTracerProfileBindings& profile, void* actual_source, void* unused_second,
    void* texture0, void* texture1, void* actual_template,
    NativeTracerConstructionAccess& access) {
    static_cast<void>(unused_second);
    auto& base = profile.model;
    auto& owners = access.materials.retained_owners;
    if (!actual_source || !actual_template || base.phase != NativeModelOwner::Phase::prepared ||
        &base.environment.retained_owners != &owners || &access.meshes.retained_owners != &owners ||
        &access.sections.retained_owners != &owners ||
        base.environment.actual_names != &access.parameters.parameter_names ||
        &access.materials.parameter_names != &access.parameters.parameter_names)
        throw std::invalid_argument("tracer requires a prepared actual slot and one canonical ownership domain");
    void* actual = &base.storage.node;
    NativeString name;
    name.resize_0041dd40(access.parameters.parameter_names, 0x11, true);
    constexpr char tracer_name[18] = "SkinedWaterTracer";
    if (name.data()) std::memcpy(name.data(), tracer_name, name.block_size());
    try { construct_native_model_00b75030(base, name); }
    catch (...) {
        destroy_native_string_header_0041dd20(&name, access.parameters.parameter_names);
        throw;
    }
    destroy_native_string_header_0041dd20(&name, access.parameters.parameter_names);
    publish_native_tracer_profile_00bad6f0(profile);
    store(actual, 0x190, std::uint32_t{0});
    auto& points = *::new (at(actual, 0x194)) NativeTracerPointArrayStorage;
    points = {nullptr, 0, 0};
    store(actual, 0x1a0, std::uint32_t{0});
    store(actual, 0x1a4, std::uint32_t{0});
    store(actual, 0x1a8, std::uint32_t{0});
    const auto& constants = access.constants;
    auto first = constants.scalar_00ce74f8;
    auto second = constants.scalar_00ce3d34;
    store(actual, 0x1f4, first);
    first = constants.scalar_00d6404c;
    store(actual, 0x1e0, second);
    second = constants.scalar_00ce380c;
    store(actual, 0x1ec, first);
    first = constants.scalar_00d0d918;
    store(actual, 0x1e4, second);
    second = constants.scalar_00ce81a4;
    store(actual, 0x244, actual_source);
    store(actual, 0x1f0, first);
    first = constants.one_00d7a24c;
    store(actual, 0x1e8, second);
    second = constants.scalar_00ce38b8;
    store(actual, 0x200, std::uint8_t{1});
    store(actual, 0x201, std::uint8_t{0});
    store(actual, 0x202, std::uint8_t{0});
    for (const auto offset : {0x254u,0x790u,0x794u,0x788u,0x78cu,0x204u,0x1f8u})
        store(actual, offset, std::uint32_t{0});
    store(actual, 0x1dc, first);
    store(actual, 0x214, std::uint32_t{0});
    store(actual, 0x1d8, second);
    store(actual, 0x1ac, std::uint32_t{0});
    store(actual, 0x1b8, std::uint32_t{0});
    store(actual, 0x1b4, std::uint32_t{0});
    store(actual, 0x18c, load<std::uint32_t>(actual_template, 8));
    store(actual, 0x1fc, first);
    store(actual, 0x224, std::uint32_t{0});
    store(actual, 0x228, std::uint32_t{0});
    store(actual, 0x22c, std::uint32_t{0});
    store(actual, 0x1c0, std::uint8_t{1});
    store(actual, 0x248, std::uint32_t{0});
    try {
        void* old = load<void*>(actual, 0x190);
        if (old != actual_template) {
            store(actual, 0x190, actual_template);
            retain(actual_template);
            if (old) release_native_render_actual_owner(owners, old);
        }
        resize_native_tracer_points_00bac310(points,
            load<std::int32_t>(load<void*>(actual, 0x190), 0x10));
        store(actual, 0x1bc, material(actual_template, access));
        set_native_material_parameter_owner_00b18a40(
            required(load<NativeMaterialStorage*>(actual, 0x1bc)), actual, 0, owners);
        set_native_material_texture_00b189f0(
            required(load<NativeMaterialStorage*>(actual, 0x1bc)), 0, texture0, owners);
        set_native_material_texture_00b189f0(
            required(load<NativeMaterialStorage*>(actual, 0x1bc)), 1, texture1, owners);
        void* effect_name = at(required(load<NativeMaterialStorage*>(actual, 0x1bc)).effect_7c, 0xb8);
        const char* text = load<const char*>(effect_name, 4);
        const char* match = text ? std::strstr(text, "static") : nullptr;
        if (match && reinterpret_cast<std::uintptr_t>(match) -
            reinterpret_cast<std::uintptr_t>(load<const char*>(effect_name, 4)) != 0xffffffffu) {
            parameter(actual, access, "cParams0", 0x258, 1);
        } else {
            parameter(actual, access, "BoneData0", 0x288, 0x28, 0x788);
            parameter(actual, access, "BoneData1", 0x508, 0x28, 0x78c);
            parameter(actual, access, "cZScale0", 0x268, 1);
            parameter(actual, access, "cZScale1", 0x278, 1);
        }
        auto* geometry = mesh(access);
        void* source_stream = native_mesh_vertex_stream_00b73260(
            required(load<NativeMeshStorage*>(actual_template, 0x0c)), 0);
        const auto first_argument = load<std::uint32_t>(actual_template, 0x30);
        void* renderer = access.renderer_00f8d394;
        const void* renderer_table = load<void*>(renderer, 0);
        void* descriptor = access.layouts.current_stream_descriptor_virtual24(source_stream);
        const auto entry = load<std::uint32_t>(renderer_table, 0x5c);
        void* stream = access.callees.renderer_virtual5c(renderer, entry, first_argument, 1, descriptor);
        store(actual, 0x248, stream);
        set_native_mesh_vertex_stream_00b73bb0(required(geometry), owners, 0, stream);
        set_native_mesh_index_stream_00b73b70(required(geometry), owners,
            load<void*>(load<void*>(actual_template, 0x0c), 0x60));
        auto& section = required(create_native_mesh_section_00533fa0(
            access.sections.pool_010901d4, access.mesh_constants.maximum_00ce4970));
        access.callees.bind_section(section);
        section.primitive_08 = 4;
        section.range_words_0c[0] = 0;
        section.range_words_0c[1] = load<std::uint32_t>(actual_template, 0x30);
        section.range_words_0c[2] = 0;
        section.range_words_0c[3] = load<std::uint32_t>(actual_template, 0x34);
        rebuild_native_mesh_section_vertex_layout_00b865a0(section, owners, geometry, access.layouts);
        set_native_mesh_section_material_00b864c0(section, owners, load<void*>(actual, 0x1bc));
        append_native_mesh_draw_section_00b73c60(required(geometry), &section);
        release_native_render_actual_owner(owners, &section);
        float scalars[2];
        geometry_scalars(scalars, &constants.unchanged_00d7a260);
        set_native_model_geometry_00b75170(base, 0, geometry, scalars[0], scalars[1]);
        propagate_native_node_root_00b6d890(base.environment.nodes, base.node.transform,
            load<RenderNodeRootList*>(actual_source, 0x14));
        set_native_generated_model_bounds_00b74390(actual,
            static_cast<const float*>(at(actual_template, 0x38)));
        if (geometry) release_native_render_actual_owner(owners, geometry);
        store(actual, 0x250, std::uint32_t{0});
        store(actual, 0x24c, std::uint8_t{1});
    } catch (...) {
        // State4 ->3 ->2 ->-1. No constructed material/template/stream/raw
        // mesh/section reference rollback is present in descriptor DFD694.
        try {
            destroy_native_tracer_pointer_array_00bac880(at(actual, 0x1a0));
            destroy_native_tracer_point_array_00bac860(at(actual, 0x194));
            restore_native_tracer_model_profile(profile);
            destroy_native_model_00b750c0(base);
            retire_failed_native_tracer_profile(profile);
        } catch (...) { std::terminate(); }
        throw;
    }
    return {actual};
}
} // namespace bsp
