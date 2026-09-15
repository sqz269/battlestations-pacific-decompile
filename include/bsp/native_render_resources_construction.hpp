#pragma once
#include "bsp/native_cockpit_helper_construction.hpp"
#include "bsp/native_cockpit_helper_lifetime.hpp"
#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_render_service_parameters.hpp"
#include "bsp/native_render_service_texture_construction.hpp"
#include <optional>

namespace bsp {

// Stable bindings to the actual current DWORD cells, not float defaults.
// Their values may alias owner bytes and are read at the original MOVSS sites.
struct NativeRenderResourcesInitialCells {
    const volatile std::uint32_t& actual_00ce54a0;
    const volatile std::uint32_t& actual_00ce3804;
    const volatile std::uint32_t& actual_00ce4bc4;
    const volatile std::uint32_t& actual_00ce74f8;
    const volatile std::uint32_t& actual_00ce8198;
    const volatile std::uint32_t& actual_00ce6650;
    const volatile std::uint32_t& actual_00d7a24c;
    const volatile std::uint32_t& actual_00ce3854;
    const volatile std::uint32_t& actual_00ce38b8;
    const volatile std::uint32_t& actual_00ce3800;
    const volatile std::uint32_t& actual_00ce3d08;
    const volatile std::uint32_t& actual_00d1f980;
    const volatile std::uint32_t& actual_00ce3d34;
    const volatile std::uint32_t& actual_00ce89d0;
    const volatile std::uint32_t& actual_00ce89cc;
    const volatile std::uint32_t& actual_00ce89c8;
    const volatile std::uint32_t& actual_00d7a238;
    const volatile std::uint32_t& actual_00ce3d30;
    const volatile std::uint32_t& actual_00ce7628;
    const volatile std::uint32_t& actual_00ce6a04;
};

// One actual renderer/publication/string domain, borrowed through the existing
// texture context; profiles/literals remain current borrowed storage. The node
// and texture contexts must share the raw string-pool/manager/gate bindings.
// Surface/texture contexts also share the renderer publication cell and actual
// string adapter. These source-domain requirements are checked before admission.
struct NativeRenderResourcesConstructionContext {
    NativeRenderServiceBaseContext& base;
    NativeRenderServiceTextureConstructionContext& textures;
    NativeFrameTargetOwnerContext& frame_targets;
    const NativeRenderResourcesInitialCells& cells;
    const NativeRenderServiceParameterConstants& parameters;
    NativeCameraEnvironment& camera;
    const NativeNodeRawConstants& node_constants;
    NativeViewportRegistry& viewport_registry;
    const NativeCockpitViewportReleaseContext& viewport_release;
    const volatile std::uint32_t* actual_helper_table_00d61854;
    const void* actual_literal_00d5e474;
    const void* actual_literal_00d5e468;
    const void* actual_literal_00d5e460;
};

// Persistent caller-owned host storage, prepared before the first native event.
// Not a native owner/refcount; never move, reset, replay, or destroy unresolved
// child cache/VFS attempts. The block and optional companions survive failure.
class NativeRenderResourcesConstructionAcquired final {
public:
    enum class Phase { fresh, preparing, running, failed, helper_binding_failed, complete };
    NativeRenderResourcesConstructionAcquired() = default;
    NativeRenderResourcesConstructionAcquired(const NativeRenderResourcesConstructionAcquired&) = delete;
    NativeRenderResourcesConstructionAcquired& operator=(const NativeRenderResourcesConstructionAcquired&) = delete;
    Phase phase{Phase::fresh};
    void* owner{};
    int unwind_state{-1};
    std::uint32_t native_site{};
    // Native ESP+10/+14 alias allocation spill and reusable raw8h string.
    alignas(4) unsigned char native_locals_10_17[8];
    NativeTextureCacheAcquired default_loads[2];
    NativeRenderServiceTextureConstructionAcquired texture_construction;
    NativeCockpitConstructionBlock cockpit;
    void* helper_allocation{}; // identity only after a native free
    bool helper_completed{};  // HOST fact, never a new native EH state
    bool helper_allocation_freed{};
    bool helper_retired{};
    NativeCockpitHelperOwner* helper_owner() noexcept;
    NativeCockpitHelperReference* helper_reference() noexcept;
    // After native final-zero retirement and every semantic borrow has ended.
    // Disposes host companions only; no native release, free, or retry.
    void forget_retired_helper_after_host_quiescence() noexcept;
private:
    friend void* construct_native_render_resources_00b14a10(void*,
        NativeRenderResourcesConstructionContext&, NativeRenderResourcesConstructionAcquired&);
    static void record_helper_retirement(void*, NativeCockpitHelperReference&) noexcept;
    std::optional<NativeCockpitHelperOwner> helper_owner_;
    std::optional<NativeCockpitHelperReference> helper_reference_;
};

// Complete B14A10..B14F5B normal body and nine-state C++ exception projection
// in the existing concrete provider domain. Original ECX actual6ACh receiver,
// zero stacked arguments, EAX original receiver, RET. New explicit context ABI.
// Leaves every unmentioned receiver byte as its preimage. Allocations request
// exactly40h/CCh/24h through the existing CRT domain. Null branches are retained;
// the ordinary allocator either returns nonnull or throws.
//
// Block admission and helper-companion storage precede B0F020 publication.
// Successful B3C800 binds owner/reference without allocation/retain, then writes
// receiver+0C. Post-success host binding failure preserves the completed helper
// and block, reports helper_binding_failed, and is NOT native state8 unwind.
// No automatic recovery for that unsupported boundary is claimed. Native
// helper failure frees its exact raw allocation while retaining surviving block
// records. Callers must explicitly retire them and establish host quiescence.
// Startup wiring, derived destruction, native FH3/SEH identity and gameplay
// remain separate integration dependencies. See docs/NATIVE_RENDER_RESOURCES_CS.md
// for BK source provenance, the current composition fixture and its limits.
void* construct_native_render_resources_00b14a10(void* actual_receiver,
    NativeRenderResourcesConstructionContext&, NativeRenderResourcesConstructionAcquired&);

} // namespace bsp
