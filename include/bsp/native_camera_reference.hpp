#pragma once
#include "bsp/native_camera_owner.hpp"

namespace bsp {
class NativeCameraReference;

// Explicit lifetime of the host companions. The callback runs AFTER native
// camera destruction, pool return and removal of the separate lifetime binding.
// It may dispose this adapter and its NativeCameraOwner. For stack/arena storage
// it records retirement; both companions then remain valid until scope/arena
// disposal. No native storage or adapter access occurs after the callback.
struct NativeCameraCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeCameraReference&) noexcept;
};

// One canonical stable adapter per live camera, bound in the SAME node lifetime
// runtime as its actual hierarchy. It neither retains nor initializes +04.
// The adapter, owner, environment and disposal contract must all survive logical
// release while queue references remain. Do not directly destroy the owner once
// this adapter is bound; its final zero callback owns that native terminal path.
// All terminal/runtime callbacks must be nonthrowing, as required by the shared
// queue interfaces. Unsupported current native profiles fail explicitly.
class NativeCameraReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativeCameraReference(NativeCameraOwner&, NativeCameraCompanionDisposal);
    // Existing live-owner/profile checks precede admitted host registration.
    // Consumes the lifetime-binding credit without retaining it or native +04.
    NativeCameraReference(NativeCameraOwner&, NativeCameraCompanionDisposal,
        GeneratedModelLifetimeRuntime::BindingAdmission&&);
    ~NativeCameraReference() override;
    NativeCameraReference(const NativeCameraReference&) = delete;
    NativeCameraReference& operator=(const NativeCameraReference&) = delete;

    NativeCameraOwner& camera_owner() noexcept { return owner_; }
    CameraTransform& transform() noexcept override { return owner_.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.node.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;

private:
    NativeCameraReference(NativeCameraOwner&, NativeCameraCompanionDisposal,
        GeneratedModelLifetimeRuntime::BindingAdmission*);
    enum class Phase { bound, destroying, retired };
    NativeCameraOwner& owner_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativeCameraCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
};
}
