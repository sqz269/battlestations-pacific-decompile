#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/native_gui_scene_fields.hpp"
#include "bsp/native_instance_group_upload.hpp"
#include "bsp/platform_renderer_activation.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const void* value, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(value) + offset);
}
void* pointer(const void* value, Word offset) noexcept {
    return reinterpret_cast<void*>(word(value, offset));
}
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void publish(void* value, Word offset, const void* identity) noexcept {
    *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(value) + offset) = address(identity);
}
const volatile Word* attachment_slot(void* node, NativeNodeSceneChildDispatch& dispatch) {
    const volatile Word* const profile = dispatch.resolve_profile(word(node));
    if (!profile) throw std::logic_error("root propagation requires its captured actual profile");
    return profile + 0x50 / sizeof(Word);
}
} // namespace

void propagate_native_node_root_storage_00b6d890(void* node,
    void* requested, NativeNodeSceneChildDispatch& dispatch) {
    void* const old_root = pointer(node, 0xa4); // B6D898, request already captured
    if (old_root == requested && pointer(node, 0x30)) return;
    if (old_root && !pointer(node, 0x30))
        unlink_native_raw_root_node_00b72220(old_root, node); // B6D8B7
    publish(node, 0xa4, requested); // B6D8BE
    if (requested) {
        void* const parent = pointer(node, 0x30); // B6D8C6
        if (!parent) {
            prepend_native_gui_scene_node_00b721f0(requested, node); // B6D8D0
            if (!pointer(node, 0x170)) {
                void* const current_root = pointer(node, 0xa4); // B6D8DE
                const volatile Word* const slot = attachment_slot(node, dispatch); // B6D8E5/E9
                // Existing genuine4B volatile load; nominal pointer type is
                // consumed only as opaque identity, never as a host object.
                void* const resource = native_scene_lighting_owner_00b72110(current_root);
                const Word target = *slot; // B6D8F2 AFTER getter
                dispatch.invoke_child(target, node, address(resource), 0); // B6D8F6
            }
        } else if (!pointer(node, 0x170)) {
            const volatile Word* const slot = attachment_slot(node, dispatch); // B6D904
            void* const resource = pointer(parent, 0x170); // B6D906
            const Word target = *slot; // B6D90C
            dispatch.invoke_child(target, node, address(resource), 0); // B6D914
        }
    }
    void* child = pointer(node, 0x34); // B6D916, CURRENT after attachment
    while (child) {
        propagate_native_node_root_storage_00b6d890(child, requested, dispatch); // B6D923
        child = pointer(child, 0x3c); // B6D928, CURRENT after recursive return
    }
}
} // namespace bsp
