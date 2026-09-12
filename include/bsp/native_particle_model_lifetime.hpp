#pragma once
#include "bsp/native_particle_model_construction.hpp"
#include "bsp/native_particle_model_pool_allocate.hpp"

namespace bsp {

// Borrow the actual five DWORDs produced by CD7850; never initialize/copy a
// second type domain. The first four are type IDs; the last is a native address.
struct NativeParticleModelTypeDescriptor {
    std::uint32_t own_id, model_id, node_id, root_id, native_name_address;
};
static_assert(sizeof(NativeParticleModelTypeDescriptor) == 0x14);
std::uint32_t native_particle_model_type_id_00af5af0(
    const volatile NativeParticleModelTypeDescriptor&) noexcept;
std::uint32_t native_particle_model_type_name_00af5b00(
    const volatile NativeParticleModelTypeDescriptor&) noexcept;
bool native_particle_model_is_type_00af6da0(
    const volatile NativeParticleModelTypeDescriptor&, std::uint32_t token) noexcept;

// Complete AF6180..AF61CF, ECX actual pointer header, stack signed count, RET4.
// Growth uses the canonical AF6120 reserve; shrinking never releases cells.
void resize_native_particle_emitter_pointers_00af6180(
    NativeRenderPointerArrayStorage&, std::int32_t count);
// Complete AF6B70..AF6B86. Shrink to zero, free CURRENT backing, retain stale
// data/capacity. This is member cleanup, not emitter object destruction.
void destroy_native_particle_emitter_pointers_00af6b70(
    NativeRenderPointerArrayStorage&);

// Complete native member destruction, actual cookie arrays from AFD130/AFD220.
// Native byte destructor AFCDA0 and record destructor AFD9F0 are literal RETs.
// Cookie counts, not descriptor counts, govern reverse element destruction.
// Valid native allocation spans and signed nonnegative cookies are required.
void destroy_native_particle_array_byte_00afcda0(void*) noexcept;
void destroy_native_particle_model_byte_array_00afd0f0(NativeParticleArrayStorage&) noexcept;
void destroy_native_particle_model_record_array_00afd1e0(NativeParticleArrayStorage&) noexcept;
// Complete AFD370..AFD405, ECX actual18h owner, RET. Destroy records then bytes;
// preserve borrowed model00 and word14. Does not free the18h allocation.
void destroy_native_particle_model_arrays_00afd370(NativeParticleModelArraysStorage&) noexcept;

struct NativeParticleModelLifetimeAccess {
    // SAME initialized F8D2D0 owner used for allocation; never model pool01090054.
    void* actual_pool_00f8d2d0;
    void* const volatile& manager_00f8c274;
    volatile std::uint32_t& live_count_00f8d2c8;
    // Borrow current complete23-DWORD D5DA50 profile and canonical type words.
    const volatile std::uint32_t* vtable_00d5da50;
    const volatile NativeParticleModelTypeDescriptor& types_00f8d308;
};

// Complete AF6C50..AF6D91 THROUGH REQUIRED existing canonical actual-owner
// bindings for every reached zero-reference emitter/variant/mesh. Uses exactly
// base.environment.retained_owners, its actual atomics, SAME node/model owner,
// hierarchy, scene and lifetime runtime. Missing actual bindings are errors.
// Ordinary sequence and two native unwind states are preserved; native SEH ABI
// is not reproduced. Arrays190 remains dangling; emitter data/capacity remain.
void destroy_native_particle_model_00af6c50(NativeModelOwner&, NativeParticleModelLifetimeAccess&);
// Complete AF7F40..AF7F5F: ECX model, stack flags, same EAX, RET4. Return2E0h
// slot to actual F8D2D0 iff flags bit0, after successful derived/base destruction.
void* delete_native_particle_model_00af7f40(
    NativeModelOwner&, NativeParticleModelLifetimeAccess&, std::uint32_t flags);

class NativeParticleModelReference;
struct NativeParticleModelCompanionDisposal {
    void* context;
    // After physical return and runtime unbind: remove this SAME canonical
    // actual-owner binding and dispose companions if desired. No access follows.
    void (*retire)(void*, NativeParticleModelReference&) noexcept;
};

// One companion over the existing NativeModelOwner, its node binding and sole
// native+04. No NativeModelReference, second owner domain, retain or storage.
// Construct from AF74A0's bind_particle_profile hook after D5DA50 is published,
// and register this object in the SAME NativeRenderActualOwners used by base.
// The constructor replaces callbacks on that existing node binding; it leaves
// its context, transforms and all native bytes unchanged. Bind/access lifetime
// must cover constructor callbacks and successful lifetime. Terminal providers
// are nonthrowing; the canonical actual emitter/variant implementations remain
// required and are not substituted by successful no-op releases.
class NativeParticleModelReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativeParticleModelReference(NativeModelOwner&, NativeParticleModelLifetimeAccess&,
        NativeParticleModelCompanionDisposal);
    ~NativeParticleModelReference() override;
    NativeParticleModelReference(const NativeParticleModelReference&) = delete;
    NativeParticleModelReference& operator=(const NativeParticleModelReference&) = delete;
    NativeModelOwner& model_owner() noexcept { return owner_; }
    CameraTransform& transform() noexcept override { return owner_.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.node.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;
    // AF74A0's retire_failed_particle_profile hook calls this AFTER real base
    // unwind. Unbind host bookkeeping only; caller still owns the raw slot.
    // Invokes disposal.retire, which must also handle this no-pool-return path.
    void retire_after_failed_construction() noexcept;
private:
    enum class Phase { bound, destroying, retired };
    NativeModelOwner& owner_;
    NativeParticleModelLifetimeAccess& access_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativeParticleModelCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    static NativeParticleModelReference& from_binding(SceneNodeAttachment&);
    static bool is_type(SceneAttachmentRuntime&, SceneNodeAttachment&, std::uint32_t);
    static void attach_scene(SceneAttachmentRuntime&, SceneNodeAttachment&, SceneResource*, bool);
    static void remove_scene(SceneAttachmentRuntime&, SceneNodeAttachment&, SceneResource*, bool);
    static void world_changed(SceneAttachmentRuntime&, SceneNodeAttachment&);
    static std::uint32_t light_count(void*) noexcept;
    static GeneratedModelPointLightLinks& light_element(void*, std::uint32_t) noexcept;
    static void shrink_lights(void*) noexcept;
    void require_slot(std::uint32_t offset, std::uint32_t expected, bool allow_base = false) const noexcept;
    void retire() noexcept;
};
} // namespace bsp
