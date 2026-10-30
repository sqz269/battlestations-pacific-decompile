#pragma once
#include "bsp/native_model_pool.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_model_storage_lifetime.hpp"
#include <exception>

namespace bsp {
class ModelTypeBootstrap;

// Exactly model+174..183 in the actual 188h pool slot. No initializers erase
// allocation preimages; the pool's slab index at +184 is a separate object.
struct NativeModelTailStorage {
    void* retained_174;
    float scalar_178;
    float scalar_17c;
    void* geometry_180;
};
struct NativeModelStorageView {
    NativeNodeStorage& node;
    NativeModelTailStorage& model;
};
struct NativeModelConstants {
    const volatile std::uint32_t& minimum_00ce4adc;
    const volatile std::uint32_t& maximum_00ce4970;
    const volatile std::uint32_t& negative_zero_00d7a208;
    const volatile std::uint32_t& unchanged_00d7a260;
};
// Configure before the first Model in this SAME environment. Raw Model/node
// construction and cleanup use nodes' existing raw name pool. The borrowed
// adapter is only for existing source callers' NativeString temporaries; it
// must address that SAME actual pool/publication/manager domain. No new pool,
// string owner, Model environment or exception transport is created.
struct NativeModelRawNameServices {
    const NativeNodeRawConstants& constants;
    NativeStringStorage& temporary_names;
};
struct NativeModelEnvironment {
    NativeModelPool& pool_01090054;
    NativeNodeDestructionRuntime& nodes;
    ModelTypeBootstrap& types;
    NativeRenderActualOwners& retained_owners;
    NativeModelConstants constants;
    // Actual current profile views, at least23 and22 DWORDs respectively.
    // Unknown owner/table entries have no constructed-type/default fallback.
    const volatile std::uint32_t* vtable_00d62de8;
    const volatile std::uint32_t* vtable_00d62c88;
    // Bind the SAME native string storage used by the actual object's other
    // names. Null retains the older semantic pool interface for existing users.
    // This borrowed binding must remain stable through node destruction.
    NativeStringStorage* actual_names = nullptr;
    // Optional born-raw source route. Requires actual_names==nullptr and an
    // immutable raw node runtime. Keep services and their references stable
    // through every Model lifetime; never switch a live environment's mode.
    const NativeModelRawNameServices* raw_name_services = nullptr;
};

// HOST diagnostics/extent state, not a native field, reference or retry token.
// The sole stable Model record retains this with its actual raw frames through
// every callback, failure disposition and any flags0 physical-slot obligation.
struct NativeModelRawHostLifetime {
    bool typed_storage_ended{}, failed{};
    std::uint32_t stopped_site{};
    std::exception_ptr exception;
};
struct NativeModelRawLifetimeBinding {
    NativeModelStorageContext& storage;
    NativeModelStorageLifetimeFrame& terminal_frame;
    NativeModelStorageLifetimeAcquired& terminal;
    NativeModelRawHostLifetime& host;
};

class NativeModelOwner final {
public:
    // Prepare stable typed views over one unused actual pool slot and register
    // only the external scene association. Preserve all188h bytes. B75030 must
    // run before inspecting fields or acquiring references. No second native
    // transform, count, scene pointer, name or point-light array is created.
    NativeModelOwner(void* actual_slot, std::size_t slot_bytes, NativeModelEnvironment&);
    // Required immutable raw LIFETIME selection, separate from raw name mode.
    // Same renderer/VFS raw scene/tree/world/point-light/string providers and
    // fresh disjoint persistent terminal backing must precede construction.
    NativeModelOwner(void* actual_slot, std::size_t slot_bytes, NativeModelEnvironment&,
        NativeModelRawLifetimeBinding&);
    // Only an abandoned prepared owner is cleaned up implicitly. A live owner
    // must complete explicit native destruction before its companion is removed.
    ~NativeModelOwner();
    NativeModelOwner(const NativeModelOwner&) = delete;
    NativeModelOwner& operator=(const NativeModelOwner&) = delete;
    enum class Phase { prepared, constructing, live, dead };
    NativeModelStorageView storage;
    NativeModelEnvironment& environment;
    NativeNodeBinding node;
    Phase phase{Phase::prepared}; // host lifetime bookkeeping, not a native word
    NativeModelRawLifetimeBinding* const raw_lifetime;
private:
    NativeModelOwner(void*, std::size_t, NativeModelEnvironment&, NativeModelRawLifetimeBinding*);
};

// Original ECX raw slot, stack NativeString*, EAX same slot, RET4. Calls full
// node construction then exact model/tail/pose stores, including negative zero
// at +18/+1C/+20. On failure the caller still owns the physical pool slot.
void* construct_native_model_00b75030(NativeModelOwner&, const NativeString&);

// Forward the actual 8-byte name header to the raw node constructor unchanged.
// The persistent node runtime must select its raw pool and actual_names must
// remain null through destruction, so neither construction nor later direct
// cleanup crosses NativeStringStorage::release's noexcept adapter. Node/model
// bounds must borrow the same actual CE4970/CE4ADC cells. Mode/identity rejection
// leaves the owner prepared. Raw-node materialization/FH3 limits still apply;
// canonical reference callbacks retain their existing noexcept boundary.
void* construct_native_model_00b75030(NativeModelOwner&, const void* actual_name_header,
    const NativeNodeRawConstants&);

// Native ECX=model+178, RET. This new C++ interface receives the containing
// four-word tail. Release CURRENT raw+180 through actual+04/current virtual0,
// then clear it; preserve +174/+178/+17C. Complete member cleanup, including
// the action used by B750C0 state1 unwind.
void destroy_native_model_geometry_member_00b74f20(
    NativeModelTailStorage&, NativeRenderActualOwners&);
// Full direct ECX=model, RET. Release/clear174, reload/release/clear180, full
// node base. State1 unwind cleans CURRENT180 then base; state0 cleans base
// only. Callbacks may rebuild node hierarchy/scene state. No physical return.
void destroy_native_model_00b750c0(NativeModelOwner&);
// Original ECX=model, stack flags, EAX original address even after return,
// RET4. Return the actual188h slot to canonical pool01090054 iff flags&1.
void* delete_native_model_00b75290(NativeModelOwner&, std::uint32_t flags);
// Used by the SAME canonical raw current04 transport. The actual argument home
// is re-read AFTER raw destruction/typed extent completion, before pool return.
// On failure preserve its record/frames; flags0 keeps original slot custody.
void* delete_native_model_raw_00b75290(NativeModelOwner&,
    const volatile std::uint32_t& actual_flags_argument);
// Original ECX=model; stack unused DWORD, raw geometry, float178, float17C;
// RET10h. Publish/retain incoming before releasing captured old. Read sentinel
// once after callbacks; SSE unordered inputs are stored. No rollback on throw.
void set_native_model_geometry_00b75170(NativeModelOwner&, std::uint32_t unused,
    void* actual_geometry, float scalar_178, float scalar_17c);

class NativeModelReference;
struct NativeModelCompanionDisposal {
    void* context;
    // Runs after actual destruction, flags-selected pool return and lifetime
    // unbinding. Raw flags0 must retain its original physical-slot receipt in
    // the same retired record; failures never call this disposal. May dispose
    // the reference/owner companions; no access follows callback.
    void (*retire)(void*, NativeModelReference&) noexcept;
};
// One canonical companion per live native model. It borrows actual+04 without
// initializing/retaining it and uses the SAME node lifetime runtime. Once bound,
// its final zero callback owns scalar deletion; do not directly destroy owner.
// Native geometry is a raw actual owner, never a shared_ptr adapter address.
class NativeModelReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativeModelReference(NativeModelOwner&, NativeModelCompanionDisposal);
    ~NativeModelReference() override;
    NativeModelReference(const NativeModelReference&) = delete;
    NativeModelReference& operator=(const NativeModelReference&) = delete;
    NativeModelOwner& model_owner() noexcept { return owner_; }
    CameraTransform& transform() noexcept override { return owner_.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.node.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;
    // Normal Page raw calls deliberately bypass the legacy noexcept virtuals.
    // Each caller owns fresh disjoint persistent invocation/frame backing;
    // callbacks may retire this reference, so no post-body self access occurs.
    void release_raw_virtual18_00b6f310(std::uint32_t captured_target,
        NativeNodeTreeRetirementFrame&, NativeNodeTreeRetirementAcquired&);
    void remove_raw_scene_virtual54_00b6ee10(std::uint32_t captured_target,
        NativeNodeSceneAttachmentFrame&, NativeNodeSceneAttachmentAcquired&);
private:
    enum class Phase { bound, destroying, retired, failed };
    NativeModelOwner& owner_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativeModelCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
    class RawDeleteCalls;
    void delete_raw_scalar_00b75290(const volatile std::uint32_t&);
};
} // namespace bsp
