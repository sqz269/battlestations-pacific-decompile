#pragma once
#include "bsp/instance_geometry.hpp"
#include "bsp/render_command_queue.hpp"
#include "bsp/scene_attachment.hpp"
#include "bsp/building_instance.hpp"

namespace bsp {
class GeneratedModelLifetime;

// Native point-light+1E0 is a borrowed reverse-link array. The same model+164
// light list supplies +1EC position/radius and +184 color to00B55780/00B42350.
// identity is the actual borrowed light owner; it and this companion must
// outlive linked models. This is separate from owned geometry+180.
struct GeneratedModelPointLightLinks {
    const void* identity;
    BuildingInstancePointLight values;
    std::vector<CameraTransform*> models;
};
// Native attached object+178 is another borrowed node array. identity is the
// same actual +A0 value projected by CameraTransform.notification_context.
struct GeneratedModelAttachmentLinks {
    void* identity;
    std::vector<CameraTransform*> models;
};

class GeneratedModelNodeLifetime : public RenderCommandModelLifetime {
public:
    virtual SceneNodeAttachment& scene_attachment() noexcept = 0;
    virtual void remove_scene_virtual54(SceneResource*, bool recurse) noexcept = 0;
};

// Dispatch associations only: hierarchy and scene state remain in the shared
// CameraTransform/SceneNodeAttachment. Every traversed child needs its actual
// virtual18/54 implementation; every nonnull+A0 identity needs an attachment
// binding. Bindings and scene runtime must outlive their registered models.
class GeneratedModelLifetimeRuntime {
public:
    explicit GeneratedModelLifetimeRuntime(SceneAttachmentRuntime& scenes) noexcept : scenes(scenes) {}
    void bind(GeneratedModelNodeLifetime&);
    void unbind(GeneratedModelNodeLifetime&) noexcept;
    GeneratedModelNodeLifetime& resolve(CameraTransform&) const noexcept;
    void bind_attachment(GeneratedModelAttachmentLinks&);
    void unbind_attachment(GeneratedModelAttachmentLinks&) noexcept;
    GeneratedModelAttachmentLinks& attachment(void* identity) const noexcept;
    SceneAttachmentRuntime& scenes;
private:
    std::vector<GeneratedModelNodeLifetime*> nodes_;
    std::vector<GeneratedModelAttachmentLinks*> attachments_;
};

// Borrowed access to the SAME native +164/+168/+16C list or diagnostic vector.
// All callbacks are required and nonthrowing. Each query reads the current
// count or current begin/element; it must not cache a copied list. Counts use
// the native unsigned comparison domain, with an actual valid array extent.
// Native signed descriptors must be validated nonnegative by their adapter.
// shrink_to_zero implements B6EC70(0), retaining backing pointer/capacity and
// releasing no pointed light owner. It must not perform the final array free.
struct NodePointLightReleaseView {
    void* context;
    std::uint32_t (*live_count)(void*) noexcept;
    GeneratedModelPointLightLinks& (*live_element)(void*, std::uint32_t) noexcept;
    void (*shrink_to_zero)(void*) noexcept;
};

// Every reference describes one actual node. Construction only binds fields;
// it does not copy/reset the released byte, acquire a self reference, or own
// list storage. The runtime, dispatch bindings and backing must remain live
// through callbacks. Terminal self release may end all of those lifetimes.
struct NodeLogicalReleaseState {
    GeneratedModelLifetimeRuntime& runtime;
    CameraTransform& transform;
    std::uint8_t& released_byte_44;
    RenderCommandReference& self;
    NodePointLightReleaseView point_lights;
};

// Native ECX node, RET0 or tail current virtual+00 after decrement to zero.
// Shared B6F310 body for borrowed native state and the diagnostic model owner.
// Traversal precedes the released-byte gate on EVERY call. Child count +38 is
// unchanged. This operation does not read state after terminal self release.
void release_node_logical_00b6f310(NodeLogicalReleaseState) noexcept;

// A real retained geometry owner with the existing stream/material lifetime.
// Native field+180 points to a refcounted geometry; this typed adapter retains
// the supplied shared object without pretending its C++ layout is intrusive.
class GeneratedModelGeometryReference final : public RenderCommandReference {
public:
    explicit GeneratedModelGeometryReference(std::shared_ptr<GeneratedInstanceGeometry>);
    std::shared_ptr<GeneratedInstanceGeometry> geometry;
    void release_zero_references() noexcept override;
};

// Only the native physical allocation/pool policy remains external after all
// recovered owner/link/scene cleanup. This operation must actually dispose or
// return the model storage; no default no-op is supplied. It may delete the
// lifetime object. No state is accessed after this call.
class GeneratedModelStorageOwner {
public:
    virtual ~GeneratedModelStorageOwner() = default;
    virtual void dispose_model_storage(GeneratedModelLifetime&) noexcept = 0;
};

// Explicit observed construction state. No native constructor values are
// inferred from zero-initialized host storage. The pointer fields transfer one
// already-acquired reference each, and point_lights transfer borrowed array
// storage. An absent diagnostic owner means the actual native string is empty.
struct GeneratedModelConstructionState {
    std::int32_t references;
    std::uint8_t released_byte_44;
    RenderCommandReference* retained_174;
    GeneratedModelGeometryReference* geometry_180;
    RenderCommandReference* retained_130;
    std::vector<GeneratedModelPointLightLinks*> point_lights_164;
    std::unique_ptr<RenderCommandDiagnosticString> name_54_58;
};

class GeneratedModelLifetime final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    GeneratedModelLifetime(GeneratedModelLifetimeRuntime&, SceneNodeAttachment&,
        GeneratedModelConstructionState, GeneratedModelStorageOwner&);
    CameraTransform& transform() noexcept override { return attachment_state.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return attachment_state; }
    void release_model_virtual18_00b6f310() noexcept override;
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_zero_references() noexcept override;

    GeneratedModelLifetimeRuntime& runtime;
    SceneNodeAttachment& attachment_state;
    GeneratedModelStorageOwner& storage_owner;
    std::uint8_t released_byte_44;
    RenderCommandReference* retained_174;
    GeneratedModelGeometryReference* geometry_180;
    RenderCommandReference* retained_130;
    std::vector<GeneratedModelPointLightLinks*> point_lights_164;
    std::unique_ptr<RenderCommandDiagnosticString> name_54_58;
};

// Native ECX pointer-array; stack address of node pointer; AL success; RET4.
// Removes first identical pointer by replacing it with last, then decrements
// count. Does not release/delete nodes, preserve order, or reduce capacity.
bool erase_generated_model_pointer_00b7bed0(std::vector<CameraTransform*>&,
    CameraTransform*) noexcept;
// Native ECX point-light, stack model, RET4: applies erase to point-light+1E0.
void remove_point_light_model_link_00b7c1a0(GeneratedModelPointLightLinks&,
    CameraTransform&) noexcept;
// Native ECX attached object, stack model, RET4. Only matching model+A0 removes
// the backlink and clears+A0. Clears the host notification callback alongside
// its context so the shared transform correctly projects no attached object.
void unregister_generated_model_attachment_00b8f4c0(GeneratedModelAttachmentLinks&,
    CameraTransform&) noexcept;
void reserve_generated_model_point_lights_00b6e500(
    std::vector<GeneratedModelPointLightLinks*>&, std::int32_t capacity);
void resize_generated_model_point_lights_00b6ec70(
    std::vector<GeneratedModelPointLightLinks*>&, std::int32_t count);

// Native ECX node, stack scene/recurse, RET8. Identity-conditional remove;
// reload scene after registry callback, publish null BEFORE releasing current
// scene. Does not clear a scene installed by that terminal callback. Then walk
// live children via actual virtual54 with next_sibling reloaded after each call.
void remove_generated_model_scene_00b6ee10(GeneratedModelLifetimeRuntime&,
    GeneratedModelNodeLifetime&, SceneResource* expected_scene, bool recurse) noexcept;

// Generated-model adapter for the shared native virtual18 at00B6F310.
// Reverse-link removals and child
// releases precede the byte44 gate even on repeated calls. First release clears
// hierarchy/root links, sets byte44, unregisters+A0, then releases one self ref.
// Scene/geometry ownership may remain live when that reference is not the last.
void release_generated_model_00b6f310(GeneratedModelLifetime&) noexcept;

// Terminal00B750C0 ->00B6F440 on the virtual18 path: release174,180,130; remove
// actual scene; free link-array/name storage, unbind host associations and return
// physical storage. It requires released byte44 and a cleared parent/root/child
// hierarchy, as established by00B6F310. Terminal resource callbacks must not
// rebuild that dying hierarchy or reattach its scene. General direct destruction
// before virtual18, native EH and physical pool bookkeeping are not claimed.
void destroy_generated_model_after_release_00b750c0(GeneratedModelLifetime&) noexcept;
}
