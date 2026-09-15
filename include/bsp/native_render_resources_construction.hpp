#pragma once

#include "bsp/native_cockpit_helper_construction.hpp"
#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_render_service_parameters.hpp"
#include "bsp/native_render_service_texture_construction.hpp"

namespace bsp {

// Current raw MOVSS inputs. Bindings stay fixed and outside the actual6ACh
// receiver; referents may alias its fields. No float conversions or defaults.
struct NativeRenderResourcesConstants {
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

// One actual manager/publication/string/cache/camera domain. All providers and
// profile/literal/constant storage outlive the attempt and surviving resources.
// Surface and texture services must borrow the same renderer cell/string adapter;
// base publication and raw string operations share the manager cell.
// Texture context borrows the current renderer at00F8D394 and D5F0A8 profile.
// Allocator pair supplies the original BF681B/BF65AC allocation family, including
// its nullable result and exception policy. No allocation-size substitution.
struct NativeRenderResourcesConstructionContext {
    NativeRenderServiceBaseContext& base;
    const NativeRenderResourcesConstants& constants;
    const NativeRenderServiceParameterConstants& parameters;
    NativeFrameTargetOwnerContext& frame_targets;
    NativeRenderServiceTextureConstructionContext& textures;
    const void* black_00d5e474;
    const void* noise_00d5e468;
    const void* marker_00d5e460;
    const volatile std::uint32_t& cockpit_near_00d7a2f0;
    const NativeCockpitViewportReleaseContext& cockpit_release;
    NativeCockpitConstructionBlock::Admission& cockpit_admission;
    void* (*allocate_00bf681b)(std::size_t);
    void (*free_00bf65ac)(void*) noexcept;
};

// Persistent operation, not another native owner. Only reached stores initialize
// the raw local8h name/allocation region. Child cache frames and the cockpit
// construction block must survive failures according to their own contracts.
struct NativeRenderResourcesConstructionAcquired {
    enum class Phase { fresh, running, complete, failed };
    NativeRenderResourcesConstructionAcquired() = default;
    NativeRenderResourcesConstructionAcquired(const NativeRenderResourcesConstructionAcquired&) = delete;
    NativeRenderResourcesConstructionAcquired& operator=(const NativeRenderResourcesConstructionAcquired&) = delete;
    Phase phase{Phase::fresh};
    void* owner{};
    int unwind_state{-1};
    std::uint32_t native_site{};
    std::uint32_t failure_site{};
    alignas(4) unsigned char native_local_14[8];
    NativeTextureCacheAcquired black;
    NativeTextureCacheAcquired noise;
    NativeRenderServiceTextureConstructionAcquired texture_helper;
};

// Full B14A10..B14F5B,1356 bytes: ECX actual6ACh allocation, no arguments,
// EAX original receiver, RET. Publish/register base before initializing fields.
// Preserve interleaved raw constant reads, untouched bytes, current renderer
// reloads, frame-target setters, texture publications BEFORE name cleanup,
// helper allocations and the nine-state compiler cleanup schedule.
// Numeric D5F0A8 slots128/12C/64 dispatch B24DC0/B20090/B319B0 only.
// New source API; original calling ABI/FH3/SEH and full game are not replaced.
// Completed resource fields are NOT rolled back by constructor unwind. A
// failed child's unresolved metadata remains caller-owned; never retry/reset.
void* construct_native_render_resources_00b14a10(void* actual_owner,
    NativeRenderResourcesConstructionContext&,
    NativeRenderResourcesConstructionAcquired&);
} // namespace bsp
