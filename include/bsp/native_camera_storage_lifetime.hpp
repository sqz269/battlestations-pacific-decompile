#pragma once
#include "bsp/native_camera_pool.hpp"
#include "bsp/native_node_base_destruction.hpp"
#include <cstddef>

namespace bsp {
struct NativeNodeStorage;
class NativeCameraRetainedLifetime {
public:
    virtual ~NativeCameraRetainedLifetime() = default;
    // Pure lookup of the captured current numeric profile into its borrowed
    // actual table. No native callback, mutation, synthetic profile or fallback.
    virtual const volatile std::uint32_t* resolve_profile(std::uint32_t) const = 0;
    // Required genuine exact current0 on the captured438 owner. The binding
    // admits its concrete FAMILY/EXTENT/allocation and SAME actual+4 observed
    // zero; it must not decrement again or assume this is a node/atomic count.
    // Prepare persistent nested frames/acquisitions before any native call.
    virtual void invoke_virtual0(std::uint32_t target, void* actual_owner) = 0;
};
struct NativeCameraStorageLifetimeContext {
    NativeNodeBaseDestructionContext& node;
    NativeCameraPool& camera_pool_0108ffb0;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    const volatile std::uint32_t* const camera_profile_00d62cf0;
    const volatile std::uint32_t* const viewport_profile_00d5e5f8;
    const volatile std::uint32_t* const fog_profile_00d63180;
    NativeCameraRetainedLifetime& retained;
    // SAME node scenes/trees registry and current import cells. Tables and
    // metadata stay alive through callbacks; payloads may die at a terminal.
    // Viewport34h/fog94h use their real volatile-long count, never an invented
    // atomic/RenderCommandReference companion. Current0 then fresh current4
    // must remain the evidenced BD30E0 and family-specific deleting leaves.
};
struct NativeCameraStorageDestroyFrame {
    NativeNodeBaseDestructionFrame node;
};
struct NativeCameraStorageAcquired {
    bool started{}, complete{}, exception_cleanup_started{}, pool_returned{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    bool viewport_cleared{}, fog_cleared{}, retained_cleared{};
    NativeNodeBaseDestructionAcquired node;
    // Fresh disjoint diagnostics, no ownership credit or rollback. Keep this
    // and the initialized address-stable frame through failure disposition.
};

// CompleteB71990[65]: capture incoming word BEFOREold180; identity skip;
// publish/retain current import; capturedold/current decrement/current0.
void set_native_camera_viewport_storage_00b71990(void* actual_camera,
    const volatile std::uint32_t& incoming_argument,
    NativeCameraStorageLifetimeContext&);
// Complete605FD0[41]: captured nonnull owner, FRESH decrement/current0,
// then clear actual original cell. Null skips import read and the store.
void clear_native_camera_retained_storage_00605fd0(
    volatile std::uint32_t& actual_cell, NativeCameraStorageLifetimeContext&);
// CompleteB71F10[194]: capture180 THEN capture one decrement epoch for all
// normal180/184/438 releases. EH438 uses605FD0 with a FRESH import. Consume
// cleanup states before calls; raw node-base cleanup and diagnostics persist.
void destroy_native_camera_storage_00b71f10(void* actual_camera,
    NativeCameraStorageDestroyFrame&, NativeCameraStorageLifetimeContext&,
    NativeCameraStorageAcquired&);
// CompleteB71FE0[32]: late LOW BYTE flags AFTER full destructor; SAME actual
// camera pool return ifbit0, then return original address without payload read.
void* delete_native_camera_storage_00b71fe0(void* actual_camera,
    const volatile std::uint32_t& flags_argument,
    NativeCameraStorageDestroyFrame&, NativeCameraStorageLifetimeContext&,
    NativeCameraStorageAcquired&);

// Optional postconstruction metadata only. Actual prefix must ALREADY contain
// a live atomic+4 established by raw node construction; caller supplies the
// real physical45Ch pool slot (458h payload plus slab index). Admission checks
// SAME actual pool membership/free-index state and current camera0/4. No native
// stores, reference increments, legacy conversion or new payload lifetime.
// FullB71A80 camera construction is an external precondition, not provided here.
// After admission all deletion must go through this canonical companion.
// External synchronization excludes slot reuse until metadata unbinding;
// context/frame/acquired/companion survive final retirement and quiescence.
class NativeCameraStorageReference final : public RenderCommandReference {
public:
    NativeCameraStorageReference(NativeNodeStorage& actual_live_prefix,
        std::size_t actual_slot_bytes, NativeCameraStorageLifetimeContext&,
        NativeCameraStorageDestroyFrame&, NativeCameraStorageAcquired&);
    ~NativeCameraStorageReference() override;
    NativeCameraStorageReference(const NativeCameraStorageReference&) = delete;
    NativeCameraStorageReference& operator=(const NativeCameraStorageReference&) = delete;
    void release_zero_references() noexcept override;
    void* delete_scalar_00b71fe0(const volatile std::uint32_t& flags_argument);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const actual_;
    NativeCameraStorageLifetimeContext& context_;
    NativeCameraStorageDestroyFrame& frame_;
    NativeCameraStorageAcquired& acquired_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
// Valid raw storage/finite hierarchy/owned-array domains from node base apply.
//438 remains polymorphic: only null/nonterminal or explicitly admitted genuine
// terminal families. No arbitrary node extent/legacy terminal assumptions.
// Source C++ cleanup is not nativeFH3/SEH, private-stack ABI or game proof.
} // namespace bsp
