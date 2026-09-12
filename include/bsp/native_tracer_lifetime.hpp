#pragma once
#include "bsp/native_model_owner.hpp"
#include "bsp/native_tracer_pool.hpp"

namespace bsp {

// Stable dispatch companion over the SAME NativeModelOwner/NativeNodeBinding.
// actual_slot_bytes must cover the real7B0h tracer slot (ID at7AC). The model
// environment's188h pool is never used for tracer scalar deletion. No native
// word is initialized, no scene/lifetime is registered, and no retain occurs.
class NativeTracerProfileBindings final {
public:
    NativeTracerProfileBindings(NativeModelOwner&, std::size_t actual_slot_bytes,
        void* actual_pool_0109049c, const volatile std::uint32_t* actual_vtable_00d63fa0);
    NativeTracerProfileBindings(const NativeTracerProfileBindings&) = delete;
    NativeTracerProfileBindings& operator=(const NativeTracerProfileBindings&) = delete;
    NativeModelOwner& model;
    void* const actual_pool_0109049c;
    const volatile std::uint32_t* const vtable_00d63fa0; // at least23 current DWORDs
private:
    friend void publish_native_tracer_profile_00bad6f0(NativeTracerProfileBindings&) noexcept;
    friend void restore_native_tracer_model_profile(NativeTracerProfileBindings&) noexcept;
    SceneTypePredicate model_is_type_;
    SceneAttachOverride model_attach_;
    SceneAttachOverride model_remove_;
    void (*model_world_changed_)(SceneAttachmentRuntime&, SceneNodeAttachment&);
    void* model_context_;
};

// BAD780 profile publication plus host dispatch binding; also used by BAC990.
// Call only after successful B75030. Restoration changes only host callbacks;
// call it BEFORE B750C0, including constructor unwind. B750C0 publishes its own
// native profile. Neither helper changes the canonical transform or atomic.
void publish_native_tracer_profile_00bad6f0(NativeTracerProfileBindings&) noexcept;
void restore_native_tracer_model_profile(NativeTracerProfileBindings&) noexcept;
void retire_failed_native_tracer_profile(NativeTracerProfileBindings&) noexcept;

// Complete BAC130/BAC370 over actual0Ch pointer/count/capacity header at+1A0.
// Native ECX header, signed count stack word, RET4. Reserve copies pointer
// words only. Resize initializes new cells to null and destroys no pointee.
void reserve_native_tracer_pointer_array_00bac130(void* actual_header, std::int32_t capacity);
void resize_native_tracer_pointer_array_00bac370(void* actual_header, std::int32_t count);
// Complete member destructors, ECX actual header, RET. Resize0 then free the
// CURRENT backing. Preserve stale pointer/capacity.194 uses concrete BAC310.
void destroy_native_tracer_point_array_00bac860(void* actual_header_194);
void destroy_native_tracer_pointer_array_00bac880(void* actual_header_1a0);

// Complete BAC970 including returning-free continuation through BACA7D.
// ECX actual tracer, RET. Current248/1BC/190 releases, current254 logical
// unlink/release, pointer array, point array, full canonical model/node base.
// FH3 state2/1/0 member-to-base cleanup is retained for C++ exceptions. No
// physical return; no original EH/SEH ABI replacement is claimed.
void destroy_native_tracer_00bac970(NativeTracerProfileBindings&);
// Complete BACB90; ECX actual tracer, stack flags, EAX original slot, RET4.
// Return SAME7B0h slot to actual0109049C through BABF70 iff flags&1.
void* delete_native_tracer_00bacb90(NativeTracerProfileBindings&, std::uint32_t flags);

class NativeTracerReference;
struct NativeTracerCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeTracerReference&) noexcept;
};
// The canonical actual+04 companion for a successfully constructed BAD6F0
// tracer, replacing (never accompanying) NativeModelReference on that owner.
// Binds SAME GeneratedModelLifetimeRuntime; caller's existing actual-owner
// resolver must map the raw slot to this reference. No second ownership map,
// count, node or render model is introduced. Retirement follows physical return
// and runtime unbinding; its callback may destroy all host companions.
// Once bound, final zero release owns scalar deletion; do not directly destroy
// the model/tracer owner or return its physical slot while this binding is live.
class NativeTracerReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativeTracerReference(NativeTracerProfileBindings&, NativeTracerCompanionDisposal);
    ~NativeTracerReference() override;
    NativeTracerReference(const NativeTracerReference&) = delete;
    NativeTracerReference& operator=(const NativeTracerReference&) = delete;
    CameraTransform& transform() noexcept override { return profile_.model.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return profile_.model.node.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeTracerProfileBindings& profile_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativeTracerCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
};
// Descriptive names are hypotheses; new MSVC Win32 C++ interfaces.
} // namespace bsp
