#pragma once
#include "bsp/native_model_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Actual matching CRT services for allocations made by the caller. Both
// defaults free real host allocations. Providers must return and not throw.
struct NativeTracelineMemory {
    void (*free_00bf6989)(void*) noexcept = singleton_lifetime_free;
    void (*free_00bf65ac)(void*) noexcept = singleton_lifetime_free;
};
struct NativeTracelineLifetimeAccess {
    // The SAME initialized38h F8C288 pool that allocated this1BCh slot.
    void* actual_pool_00f8c288;
    // Borrow current complete24-DWORD profiles. No fabricated/default table.
    const volatile std::uint32_t* vtable_00d0c928;
    const volatile std::uint32_t* vtable_00d0c8c8;
    NativeTracelineMemory memory{};
};

// Complete AF1980..AF19A3: ECX actual Traceline, RET via B750C0 tail jump.
// Capture188, publish D0C8C8, free nonnull captured array WITHOUT clearing188,
// then full canonical B750C0 including scene/hierarchy/retained-owner cleanup.
// The same live prepared-and-constructed NativeModelOwner is required.
void destroy_native_traceline_00af1980(NativeModelOwner&,
    const NativeTracelineMemory& = {});
// Complete858380..85839B: ECX slot, stack flags, original EAX, RET4.
// Successful destruction precedes AF1EA0 return to actual F8C288 iff flags&1.
// D0C8C8's858330/normal-model-pool path is deliberately a separate contract.
void* delete_native_traceline_00858380(NativeModelOwner&,
    NativeTracelineLifetimeAccess&, std::uint32_t flags);

// Complete86AD50..86ADDF: actual80h payload authored inline by B0B6A0.
// Publish D0D4A4; release CURRENT40 then CURRENT44 via captured actual+04 and
// current virtual0; clear each only after successful release. Free/clear4C,
// retaining50/54 and all other bytes. State0 unwind frees/clears CURRENT4C.
// No payload constructor or second reference domain is introduced. Ordinary
// C++ exceptions reproduce cleanup effects; original x86 SEH ABI is not used.
void destroy_native_traceline_payload_0086ad50(void* actual_payload80h,
    NativeRenderActualOwners&, const NativeTracelineMemory& = {});
// Complete86ADE0..86ADFD: ECX payload, stack flags, original EAX even after
// free, RET4. BF65AC is reached iff flags&1 after successful full destruction.
void* delete_native_traceline_payload_0086ade0(void* actual_payload80h,
    NativeRenderActualOwners&, std::uint32_t flags, const NativeTracelineMemory& = {});

class NativeTracelineReference;
struct NativeTracelineCompanionDisposal {
    void* context;
    // After actual destruction, physical return and runtime unbind. Remove
    // the same actual-owner registration and dispose companions if desired.
    // No access to reference/owner follows this callback.
    void (*retire)(void*, NativeTracelineReference&) noexcept;
};
// One canonical companion over the SAME existing NativeModelOwner, actual+04,
// node/scene binding and runtime. Bind after B0B6A0 publishes D0C928, before
// AF3440. Register this reference in base.environment.retained_owners.
// Replaces existing scene callbacks, preserving their NativeModelOwner context
// and all native bytes. No implicit retain or normal-model companion exists.
class NativeTracelineReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativeTracelineReference(NativeModelOwner&, NativeTracelineLifetimeAccess&,
        NativeTracelineCompanionDisposal);
    ~NativeTracelineReference() override;
    NativeTracelineReference(const NativeTracelineReference&) = delete;
    NativeTracelineReference& operator=(const NativeTracelineReference&) = delete;
    NativeModelOwner& model_owner() noexcept { return owner_; }
    NativeTracelineLifetimeAccess& lifetime_access() noexcept { return access_; }
    CameraTransform& transform() noexcept override { return owner_.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.node.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;
    // Host cleanup after real base destruction on construction failure only;
    // no physical pool return. Disposal must distinguish this path if needed.
    void retire_after_failed_construction() noexcept;
private:
    enum class Phase { bound, destroying, retired };
    NativeModelOwner& owner_;
    NativeTracelineLifetimeAccess& access_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativeTracelineCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    static NativeTracelineReference& from_binding(SceneNodeAttachment&);
    static bool is_type(SceneAttachmentRuntime&, SceneNodeAttachment&, std::uint32_t);
    static void attach_scene(SceneAttachmentRuntime&, SceneNodeAttachment&, SceneResource*, bool);
    static void remove_scene(SceneAttachmentRuntime&, SceneNodeAttachment&, SceneResource*, bool);
    static void world_changed(SceneAttachmentRuntime&, SceneNodeAttachment&);
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
    void require_slot(std::uint32_t offset, std::uint32_t expected, bool allow_base = false) const noexcept;
    void retire() noexcept;
};
} // namespace bsp
