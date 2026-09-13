#include "bsp/native_mesh_clone.hpp"
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_string_vector.hpp"
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mesh clone requires MSVC Win32 x87 load/store semantics.
#endif

namespace bsp {
namespace {
void copy_x87_word(const void* source, void* destination, std::uint32_t byte_offset) noexcept {
    __asm {
        mov eax, source
        mov edx, destination
        mov ecx, byte_offset
        fld dword ptr [eax + ecx]
        fstp dword ptr [edx + ecx]
    }
}
// This is B73F50's concrete 4CDC20 call-site composition. The separately
// leased generic append entry is not replaced/annotated by this packet.
void append_weight_name(NativeMeshWeightNamesStorage& names,
    const NativeMeshWeightNameStorage& source, NativeStringStorage& strings) {
    if (names.count_04 == names.capacity_08) {
        auto capacity = names.capacity_08 * 2;
        if (capacity < 2) capacity = 1;
        reserve_native_string_vector_00426520(names, capacity, strings);
    }
    auto* const destination = names.data_00 + names.count_04;
    destination->length_00 = 0;
    destination->data_04 = nullptr;
    copy_native_string_header_00be0a30_fragment(destination, strings, &source);
    ++names.count_04;
}
}

void copy_native_mesh_lod_00b72bd0(
    NativeMeshStorage& destination, const NativeMeshStorage& source) noexcept {
    for (std::int32_t index = 0; index < source.lod_count_50; ++index) {
        auto& output = destination.lod_phases_10[index];
        const auto& input = source.lod_phases_10[index];
        output.minimum_00 = input.minimum_00;
        output.maximum_04 = input.maximum_04;
        output.untouched_08 = input.untouched_08;
        output.untouched_0c = input.untouched_0c;
    }
    destination.lod_count_50 = source.lod_count_50;
}

namespace {
void consume_stream(NativeStreamCloneAcquired& stream, NativeRenderActualOwners& owners) {
    auto* consumed = std::exchange(stream.creator, nullptr);
    stream.companion = nullptr;
    stream.canonical_registration = false;
    stream.phase = NativeStreamClonePhase::consumed;
    release_native_render_actual_owner(owners, consumed);
}
void copy_mesh(NativeMeshStorage& destination,
    NativeMeshStorage& source, GuiNativeGeometryOwners& geometry,
    NativeStringStorage& strings, NativeMaterialDestructionAccess& materials,
    const volatile std::uint32_t* material_profile, NativeMeshCloneAcquired& acquired,
    NativeStreamCloneServices* streams) {
    if (&source == &destination || acquired.section || acquired.material ||
        acquired.stream.creator || acquired.stream.companion ||
        (acquired.stream.phase != NativeStreamClonePhase::empty &&
         acquired.stream.phase != NativeStreamClonePhase::consumed))
        throw std::invalid_argument("native Text mesh clone requires distinct storage and no pending creators");
    auto& owners = geometry.actual_owners();
    copy_x87_word(&source, &destination, 0x0c);
    copy_native_mesh_lod_00b72bd0(destination, source);
    if (streams && source.index_stream_60) {
        auto* stream = clone_native_index_stream_00b729a0(source.index_stream_60, *streams, acquired.stream);
        acquired.stream.phase = NativeStreamClonePhase::mesh_publication;
        acquired.stream.native_site = 0x00b73f9e;
        set_native_mesh_index_stream_00b73b70(destination, owners, stream);
        consume_stream(acquired.stream, owners);
    } else {
        //3E null-source clears/releases current destination, as does this setter.
        set_native_mesh_index_stream_00b73b70(destination, owners, source.index_stream_60);
    }
    for (std::int32_t index = 0; index < source.vertex_stream_count_7c; ++index) {
        if (streams) {
            // Native3E has no null vertex-slot skip. Reached invalid storage fails.
            auto* stream = clone_native_vertex_stream_00b72a70(source.vertex_streams_64[index], *streams, acquired.stream);
            acquired.stream.phase = NativeStreamClonePhase::mesh_publication;
            acquired.stream.native_site = 0x00b7401c;
            set_native_mesh_vertex_stream_00b73bb0(destination, owners, index, stream);
            consume_stream(acquired.stream, owners);
        } else {
            set_native_mesh_vertex_stream_00b73bb0(destination, owners, index, source.vertex_streams_64[index]);
        }
    }
    for (std::int32_t index = 0; index < source.draw_sections_54.count_04; ++index) {
        const auto* const source_section = static_cast<NativeMeshSectionStorage*>(
            source.draw_sections_54.data_00[index]);
        if (!source_section) throw std::logic_error("native Text mesh has a null section");
        acquired.section = geometry.clone_section_00b85ef0(*source_section);
        const auto* const source_material = static_cast<NativeMaterialStorage*>(acquired.section->material_20);
        if (!source_material) throw std::logic_error("native Text section has a null material");
        acquired.material = geometry.clone_material_00b18b60(*source_material, materials, material_profile);
        set_native_mesh_section_material_00b864c0(*acquired.section, owners, acquired.material);
        auto* const consumed_material = std::exchange(acquired.material, nullptr);
        release_native_render_actual_owner(owners, consumed_material);
        append_native_mesh_draw_section_00b73c60(destination, acquired.section);
        auto* const consumed_section = std::exchange(acquired.section, nullptr);
        release_native_render_actual_owner(owners, consumed_section);
    }
    destination.flag_80 = source.flag_80;
    if (source.flag_80)
        for (std::uint32_t offset = 0x84; offset != 0x94; offset += 4)
            copy_x87_word(&source, &destination, offset);
    destination.flag_94 = source.flag_94;
    if (source.flag_94)
        for (std::uint32_t offset = 0x98; offset != 0xb0; offset += 4)
            copy_x87_word(&source, &destination, offset);
    resize_native_string_vector_00427110(destination.weight_names_b0, 0, strings);
    reserve_native_string_vector_00426520(
        destination.weight_names_b0, source.weight_names_b0.count_04, strings);
    for (std::int32_t index = 0; index < source.weight_names_b0.count_04; ++index)
        append_weight_name(destination.weight_names_b0, source.weight_names_b0.data_00[index], strings);
}
} // namespace
void copy_native_mesh_for_text_00b73f50(NativeMeshStorage& destination,
    NativeMeshStorage& source, GuiNativeGeometryOwners& geometry, NativeStringStorage& strings,
    NativeMaterialDestructionAccess& materials, const volatile std::uint32_t* profile,
    NativeMeshCloneAcquired& acquired) {
    copy_mesh(destination, source, geometry, strings, materials, profile, acquired, nullptr);
}
void copy_native_mesh_for_text_00b73f50_flags3e(NativeMeshStorage& destination,
    NativeMeshStorage& source, GuiNativeGeometryOwners& geometry, NativeStringStorage& strings,
    NativeMaterialDestructionAccess& materials, const volatile std::uint32_t* profile,
    NativeMeshCloneAcquired& acquired, NativeStreamCloneServices& streams) {
    if (&streams.geometry != &geometry)
        throw std::logic_error("mesh3E requires the same canonical stream/geometry owner");
    copy_mesh(destination, source, geometry, strings, materials, profile, acquired, &streams);
}
} // namespace bsp
