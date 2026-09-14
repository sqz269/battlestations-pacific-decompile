#include "bsp/native_unit_part_attachment.hpp"
#include "bsp/native_spatial_index_publication.hpp"
#include <cstddef>

namespace bsp {
namespace {
template<class T> T& field(void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::byte*>(object) + offset);
}
std::uint32_t slot(void* owner, std::size_t offset) noexcept {
    return field<volatile std::uint32_t>(field<void* volatile>(owner, 0), offset);
}
std::uint8_t query(void* part, std::uint32_t kind,
    const NativeUnitPartAttachmentAccess& access) {
    void* const owner = field<void* volatile>(part, 0x164);
    return access.query_kind_5c(owner, slot(owner, 0x5c), kind);
}
} // namespace

void* __fastcall find_native_unit_part_spatial_parent_0070f7d0(void* part,
    const NativeUnitPartAttachmentAccess* access) {
    if (!query(part, 0x1e, *access)) return nullptr;
    void* const owner = field<void* volatile>(part, 0x164);
    void* parent = field<void* volatile>(owner, 0x3c);
    while (parent) {
        if (access->collision_node_b0(parent, slot(parent, 0xb0))) {
            void* const node = access->collision_node_b0(parent, slot(parent, 0xb0));
            if (field<volatile std::uint8_t>(node, 0x158)) return node;
        }
        parent = field<void* volatile>(parent, 0x3c);
    }
    return nullptr;
}

void __fastcall attach_native_unit_part_00710ad0(void* part,
    const NativeUnitPartAttachmentAccess* access) {
    if (field<volatile std::uint8_t>(part, 0x184)) return;
    const std::uint32_t is_static = query(part, 0x1c, *access)
        || query(part, 0x1b, *access) || query(part, 0x36, *access)
        || query(part, 0x44, *access);
    void* const owner = field<void* volatile>(part, 0x164);
    if (!field<volatile std::uint8_t>(owner, 0xc8)) {
        refresh_pose_00414db0(access->spatial->poses->resolve_pose(owner));
    }
    const void* const matrix = static_cast<std::byte*>(owner) + 0xcc;
    void* const index = get_native_spatial_index_0042e630(
        *access->manager_publication_01090aa0, *access->index_publication_00f8a0d8);
    void* const parent = find_native_unit_part_spatial_parent_0070f7d0(part, access);
    attach_native_spatial_node_0098ba10(index, access->spatial, part, parent,
        matrix, is_static);
    field<volatile std::uint8_t>(part, 0x184) = 1;
}

NativeUnitPartAttachmentBindings::NativeUnitPartAttachmentBindings(
    NativeUnitPartCollisionGlobals globals, NativeUnitPartCollisionCallbacks callbacks,
    const NativeUnitPartAttachmentAccess& access) noexcept
    : NativeUnitPartStorageBindings(globals, callbacks), access_(access) {}

void NativeUnitPartAttachmentBindings::call_00710ad0(void* part) {
    attach_native_unit_part_00710ad0(part, &access_);
}
} // namespace bsp
