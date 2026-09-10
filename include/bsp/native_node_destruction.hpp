#pragma once
#include "bsp/generated_model_lifetime.hpp"
#include "bsp/native_node_construction.hpp"

namespace bsp {

// The existing B6EC70(0) path over actual +164/+168/+16C storage. Decrement
// count to zero without touching elements, pointer, capacity or light owners.
// Final array free belongs to node destruction, after logical release.
void shrink_native_node_point_lights_to_zero_00b6ec70(NativeNodePointLightArray&) noexcept;

// Association with the actual object stored at node+130. The count is a view
// of that object's native +04 word, not another host reference count. Its real
// virtual+00 may destroy the binding and reenter node/scene operations.
struct NativeNodeRetainedOwnerBinding {
    void* identity;
    std::atomic<std::int32_t>& references_04;
    void (*destroy_on_zero)(void* actual_owner);
};

// Reuses the existing scene dispatch and actual attachment backlink bindings.
// It stores no node map, hierarchy, scene pointer, root list or backlink copy.
// All supplied bindings/allocators must remain live while a call uses them.
class NativeNodeDestructionRuntime final {
public:
    NativeNodeDestructionRuntime(SceneAttachmentRuntime&,
        GeneratedModelLifetimeRuntime& actual_attachment_bindings,
        SizedStoragePool& actual_string_pool, SceneTypePredicate actual_node_virtual_0c);
    void bind_retained_owner(NativeNodeRetainedOwnerBinding&);
    void unbind_retained_owner(NativeNodeRetainedOwnerBinding&) noexcept;
    void release_retained_owner(void* captured_owner);

    SceneAttachmentRuntime& scenes;
    GeneratedModelLifetimeRuntime& attachments;
    SizedStoragePool& strings;
    SceneTypePredicate node_virtual_0c; // actual B6F570 with initialized node/root tokens
private:
    std::vector<NativeNodeRetainedOwnerBinding*> retained_owners_;
};

// Native ECX parent, stack child, RET4. Clears child.parent before relinking;
// leaves that child's previous/next words intact and does not release it.
void unlink_native_node_child_00b6d940(CameraTransform& parent,
    CameraTransform& child) noexcept;
// Native ECX root, stack node, RET4; prepend only, no ownership changes.
void prepend_native_node_root_00b721f0(RenderNodeRootList&, CameraTransform&) noexcept;
// Native ECX node, RET. Unregister current A0, then recurse through live children.
void unregister_native_node_attachments_00b6d850(NativeNodeDestructionRuntime&,
    CameraTransform&);
// Native ECX node, stack requested root, RET4. Current virtual50 and actual
// root+1C scene access; recursion keeps requested root and reloads child next.
void propagate_native_node_root_00b6d890(NativeNodeDestructionRuntime&,
    CameraTransform&, RenderNodeRootList* requested_root);
// ONLY the requested-parent-null path of native B6E680, including its equal-null
// early return. Nonnull reparent/B6E010 is deliberately not exposed here.
void set_native_node_parent_null_00b6e680(NativeNodeDestructionRuntime&, CameraTransform&);

// Universal node virtual54 adapter, native ECX node, stack expected scene and
// recursion byte, RET8. Uses the SAME current SceneNodeAttachment.remove_scene
// dispatch for children; terminal scene callbacks may install another scene.
void remove_native_node_scene_00b6ee10(SceneAttachmentRuntime&,
    SceneNodeAttachment&, SceneResource* expected_scene, bool recurse);
void native_node_world_changed_00b6dbe0(SceneAttachmentRuntime&, SceneNodeAttachment&);

// Complete direct B6F440 body, ECX owner, RET. Switches the actual dying owner
// to node-vtable phase, detaches the parent/root, releases +130, removes scenes,
// frees the actual +164 array and pooled +54 name, then ends the prefix lifetime.
// Child hierarchy/scene callbacks may reenter; there is no precleared-hierarchy
// restriction. Does NOT return the 0x1F0 slot or unbind host associations. The
// caller owns that distinct final disposal step and must not use dead bindings.
void destroy_native_node_00b6f440(NativeNodeDestructionRuntime&, NativeNodeBinding&);

} // namespace bsp
