#pragma once
#include "bsp/native_node_destruction.hpp"

namespace bsp {

// Dispatch only: hierarchy, scene/root slots and attachment backlinks remain in
// the existing bindings. Supply the initialized value at0109032C (cGroup), the
// actual current node virtual1C, and the actual attached-group virtual3C route.
// Callbacks may reenter; all traversed bindings must remain live through calls.
struct NativeNodeParentingRuntime {
    NativeNodeDestructionRuntime& nodes;
    const volatile std::uint32_t& group_type_0109032c;
    void (*set_attachment_virtual1c)(NativeNodeParentingRuntime&, CameraTransform&, void*);
    void (*notify_attached_group_virtual3c)(void* actual_group);
};

// Native ECX parent, stack child, RET4. Does not detach an unrelated old parent.
// Sets child.parent BEFORE root/scene callbacks; prepends AFTER those callbacks.
void prepend_native_node_child_00b6e010(NativeNodeDestructionRuntime&,
    CameraTransform& parent, CameraTransform& child);
// Full native ECX child, stack requested parent, RET4. Equal-parent is inert.
// Nonnull path preserves B6E010's first world callback and this body's second.
// No cycle validation, ownership increment, matrix conversion or fallback.
void set_native_node_parent_00b6e680(NativeNodeParentingRuntime&,
    CameraTransform& child, CameraTransform* requested_parent);

// Native ECX group, stack node, RET4. Uses the SAME existing backlink binding;
// vector storage is the established typed projection, not a raw178h overlay.
void register_native_node_attachment_00b8f460(NativeNodeParentingRuntime&,
    GeneratedModelAttachmentLinks&, CameraTransform&);
// Native node virtual1C. Assigns+A0 BEFORE registration (whose equality guard
// consequently skips insertion), then dispatches each live child's virtual1C.
void set_native_node_attachment_00b6d7b0(NativeNodeParentingRuntime&,
    CameraTransform&, void* actual_group);
// cGroup override: same assignment-before-registration, without child recursion.
void set_native_group_attachment_00b8f4f0(NativeNodeParentingRuntime&,
    CameraTransform&, void* actual_group);

} // namespace bsp
