#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_node_construction.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle preparation requires MSVC Win32.
#endif
namespace bsp {
namespace {
template<class T> T read(const void* owner, std::uint32_t offset) noexcept {
    T value;
    std::memcpy(&value, reinterpret_cast<const void*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset), sizeof value);
    return value;
}
void* at(void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
std::int32_t live_count(const void* owner, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::int32_t*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* live_pointer(const void* owner, std::uint32_t offset) noexcept {
    return *reinterpret_cast<void* const volatile*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset);
}
}
void prepare_native_particle_variant_00af40e0(void* variant,
    const NativeParticlePreparationDispatch& dispatch) {
    std::uint32_t index = 0;
    void* row = at(variant, 0x10);
    while (static_cast<std::int32_t>(index) < live_count(variant, 0x30)) {
        prepare_native_particle_definition_00af9f50(live_pointer(row, 0), dispatch);
        ++index;
        row = at(row, 4);
    }
}
void prepare_native_particle_definition_00af9f50(void* definition,
    const NativeParticlePreparationDispatch& dispatch) {
    std::uint32_t index = 0;
    void* row = at(definition, 0x3c);
    while (static_cast<std::int32_t>(index) < live_count(definition, 0x4c)) {
        prepare_native_particle_definition_00af9f50(live_pointer(row, 0), dispatch);
        ++index;
        row = at(row, 4);
    }
    index = 0;
    row = at(definition, 0x54);
    while (static_cast<std::int32_t>(index) < live_count(definition, 0x68)) {
        void* member = live_pointer(row, 0);
        void* table = live_pointer(member, 0);
        auto target = read<std::uint32_t>(table, 0x14);
        if (!dispatch.member_virtual14)
            throw std::logic_error("particle preparation requires actual current member virtual14 dispatch");
        dispatch.member_virtual14(dispatch.context, member, target);
        member = live_pointer(row, 0); // AF9F89: do not reuse the earlier owner/table.
        table = live_pointer(member, 0);
        target = read<std::uint32_t>(table, 0x0c);
        if (!dispatch.member_virtual0c)
            throw std::logic_error("particle preparation requires actual current member virtual0C dispatch");
        dispatch.member_virtual0c(dispatch.context, member, target);
        ++index;
        row = at(row, 4);
    }
}
void* __fastcall native_particle_index_stream_00af10a0(const void* owner) {
    return read<void*>(owner, 4);
}
void* __fastcall native_particle_vertex_descriptor_00af10b0(const void* owner) {
    return read<void*>(owner, 0x1c);
}
NativeMaterialStorage* __fastcall native_particle_secondary_material_00af1120(const void* owner) {
    return read<NativeMaterialStorage*>(owner, 0x18);
}
NativeMaterialStorage* __fastcall native_particle_material_variant_00af10f0(
    const void* owner, void*, std::uint32_t first, std::uint32_t second) {
    const auto offset = 8u + (static_cast<std::uint8_t>(first) ? 8u : 0u) +
        (static_cast<std::uint8_t>(second) ? 4u : 0u);
    return read<NativeMaterialStorage*>(owner, offset);
}
void* __fastcall native_shadow_texture_holder_00b4d170(const void* owner) {
    return read<void*>(owner, 0x0c);
}
void* __fastcall native_shadow_holder_texture_00b4cb10(const void* owner) {
    return read<void*>(owner, 8);
}
void* __fastcall native_shadow_owner_texture_00b0d130(const void* owner) {
    return native_shadow_holder_texture_00b4cb10(read<void*>(owner, 0x3c));
}
void* __fastcall native_shadow_owner_map_texture_00b0d140(const void* owner) {
    return native_shadow_holder_texture_00b4cb10(
        native_shadow_texture_holder_00b4d170(read<void*>(owner, 0x60)));
}
void __fastcall set_native_node_hierarchy_mask_007099c0(NativeNodeStorage* node,
    void*, std::uint32_t mask) {
    auto* child = reinterpret_cast<NativeNodeStorage*>(node->first_child_34);
    *reinterpret_cast<volatile std::uint32_t*>(at(node, 0x48)) = mask;
    while (child) {
        set_native_node_hierarchy_mask_007099c0(child, nullptr, mask);
        child = reinterpret_cast<NativeNodeStorage*>(child->next_sibling_3c);
    }
}
} // namespace bsp
