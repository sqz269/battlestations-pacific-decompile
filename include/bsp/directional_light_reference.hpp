#pragma once
#include "bsp/directional_light_owner.hpp"

namespace bsp {
class DirectionalLightReference;

// Borrow the actual immutable 22-word tables. They and every owner/runtime
// callback must outlive the last reference, including destruction reentry.
struct DirectionalLightReferenceProfiles {
    const volatile std::uint32_t* directional_00d62fb0;
    const volatile std::uint32_t* light_00d62f58;
    const volatile std::uint32_t* node_00d62c88;
};
struct DirectionalLightCompanionDisposal {
    void* context;
    // Called after native destruction, pool return and lifetime unbinding.
    // May delete both companions. No native/companion access follows this call.
    void (*retire)(void*, DirectionalLightReference&) noexcept;
};

// One canonical interface for the actual +04 counter and +30/+34/+3C/+170
// storage in DirectionalLightOwner. The caller first binds its SAME scene
// attachment in owner.runtime.scenes; construction adds only the lifetime
// association. It neither initializes nor increments the actual counter.
// Once bound, terminal reference release owns native deletion. Do not directly
// destroy the owner or either companion while references remain. All native
// terminal callbacks must be nonthrowing, as required by the queue interfaces.
class DirectionalLightReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    DirectionalLightReference(DirectionalLightOwner&, DirectionalLightReferenceProfiles,
        DirectionalLightCompanionDisposal);
    ~DirectionalLightReference() override;
    DirectionalLightReference(const DirectionalLightReference&) = delete;
    DirectionalLightReference& operator=(const DirectionalLightReference&) = delete;

    DirectionalLightOwner& light_owner() noexcept { return owner_; }
    CameraTransform& transform() noexcept override { return owner_.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.node.scene_attachment; }
    void release_model_virtual18_00b6f310() noexcept override;
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_zero_references() noexcept override;

private:
    enum class Phase { bound, destroying, retired };
    DirectionalLightOwner& owner_;
    GeneratedModelLifetimeRuntime& runtime_;
    DirectionalLightReferenceProfiles profiles_;
    DirectionalLightCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    const volatile std::uint32_t* current_table() const noexcept;
    static std::uint32_t light_count(void*) noexcept;
    static GeneratedModelPointLightLinks& light_element(void*, std::uint32_t) noexcept;
    static void shrink_lights(void*) noexcept;
};

// Actual shared services, with initialized directional/light/node descriptors
// behind the supplied predicates. No new pool, string allocator, type IDs,
// scene registry or shadow owner is constructed by this environment.
struct DirectionalLightReferenceEnvironment {
    DirectionalLightPool& pool_01090154;
    NativeNodeDestructionRuntime& nodes;
    SceneTypePredicate directional_virtual_0c;
    SceneTypePredicate light_virtual_0c;
    SystemShadowOwnerResolver& shadow_owners;
    DirectionalLightReferenceProfiles profiles;
};

// Host composition of B7BAC0 -> B7C6B0 and the canonical two runtime bindings.
// Returns the constructor's single self reference. Logical release may leave
// it live for queued references. Final release deletes both host companions
// after B7C820(1) returns this SAME actual 1F0-byte slot to its original pool.
// Host allocation/binding failure reclaims the unpublished fresh construction;
// this wrapper is a new C++ interface, not a recovered native factory ABI.
DirectionalLightReference* allocate_native_directional_light(
    DirectionalLightReferenceEnvironment&, const NativeString&);
}
