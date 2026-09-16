#include "bsp/native_resource_extra_item_profile.hpp"

#include "bsp/native_resource_instance_bones.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource extra item profiles require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
}

NativeResourceExtraItemProfileCalls::NativeResourceExtraItemProfileCalls(
    NativeResourceInstancePublicationCalls& other,
    NativeResourceExtraItemProfileStorage storage) noexcept
    : other_(other), storage_(storage) {}

Word NativeResourceExtraItemProfileCalls::item_slot(Word profile, Word offset) noexcept {
    // Current actual profile contents remain authoritative, including changed
    // entries that the downstream supplied provider may know how to execute.
    if (offset <= 0x20u && (offset & 3u) == 0) {
        if (profile == 0x00d6328cu)
            return storage_.animation_profile_00d6328c[offset / 4u];
        if (profile == 0x00d632b8u)
            return storage_.bone_profile_00d632b8[offset / 4u];
    }
    return other_.item_slot(profile, offset);
}

Word NativeResourceExtraItemProfileCalls::item_type_token(Word entry, void* item) {
    switch (entry) {
    case 0x00b8a060u:
        return read_native_mesh_binding_type_00b931b0(storage_.animation_cells_01090268[0]);
    case 0x00b8a070u:
        return read_native_mesh_binding_type_00b931b0(storage_.animation_cells_01090268[3]);
    case 0x00b8a140u:
        return read_native_mesh_binding_type_00b931b0(storage_.bone_cells_01090278[0]);
    case 0x00b8a150u:
        return read_native_mesh_binding_type_00b931b0(storage_.bone_cells_01090278[3]);
    default:
        return other_.item_type_token(entry, item);
    }
}

std::uint8_t NativeResourceExtraItemProfileCalls::matches_type(
    std::uintptr_t entry, void* item, Word token) {
    if (entry == 0x00b8a730u)
        return matches_native_fallback_type_00b86950(token, storage_.animation_cells_01090268);
    if (entry == 0x00b8a870u)
        return matches_native_fallback_type_00b86950(token, storage_.bone_cells_01090278);
    return other_.matches_type(entry, item, token);
}

void attach_native_extra_item_00b8a080(void* item, void* instance,
    void*, void* node, Word, NativeResourceExtraItemAttachContext& context) {
    const Word profile = *static_cast<const volatile Word*>(instance);
    if (profile == 0x00d63244u && context.instance_profile_00d63244 != nullptr &&
        context.instance_profile_00d63244[2] == 0x00b89e90u) {
        publish_native_resource_instance_item_00b89e90(instance, &context.publication, item, node);
        return;
    }
    if (profile == 0x00cfd8e0u && context.game_instance_profile_00cfd8e0 != nullptr &&
        context.game_instance_profile_00cfd8e0[2] == 0x0071b710u) {
        publish_native_game_resource_instance_item_0071b710(instance, &context.publication, item, node);
        return;
    }
    throw std::runtime_error("Extra item attachment reached an unsupported current instance slot08");
}
} // namespace bsp
