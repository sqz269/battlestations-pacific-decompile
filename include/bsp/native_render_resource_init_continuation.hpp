#pragma once
#include "bsp/native_render_pass_companion.hpp"
#include <optional>
#include "bsp/native_render_resource_init.hpp"
#include "bsp/native_bloom_initializer.hpp"
#include "bsp/native_bright_pass_initializer.hpp"
#include "bsp/native_downscale_pass_initializers.hpp"
#include "bsp/native_luminance_owner.hpp"
#include <array>

namespace bsp {

// Complete B4E2B0 normal body (59 bytes): ECX post20, stacked frame, RET4.
// Publish/retain incoming before releasing captured old at zero. Borrow the
// actual D5E600 frame profile and current imports; no second count or registry.
void assign_native_post_effect_frame_00b4e2b0(void* actual_post_effect,
    NativeFrameTargetOwnerStorage* incoming, NativeRenderResourceInitEntryContext&,
    NativeTextureSurfaceReferenceIncrement const volatile& increment_iat_00ce221c);

// Complete four-byte B54CD0; ECX bloom owner, EAX borrowed holder+20, RET0.
// Native B54E70/B54F90 are its producer; this does not retain or validate it.
void* native_bloom_output_holder_00b54cd0(const void* actual_bloom) noexcept;

struct NativeRenderResourceInitContinuationContext {
    NativeRenderResourceInitEntryContext& entry;
    NativeRenderPassInitializationContext& passes;
    NativeDownscalePassInitializationContext& downscale;
    NativeLuminanceInitializationContext& luminance;
    NativeBrightPassInitializationContext& bright;
    NativeBloomInitializationContext& bloom;
    NativeRenderEffectLifetimeContext& effects_lifetime;
    // Same actual owner registry/effects domain. Stable through final native
    // pass release and external quiescence; binding adds no native credit.
    NativeRenderPassCompanionContext& pass_companions;
    const volatile std::uint32_t* actual_texture_profile_00d61948;
    const volatile std::uint32_t& actual_00ce6650;
    const volatile std::uint32_t& actual_00ce3854;
    // Original byte strings: D5E448, D5E41C, D5E3FC, D5E360.
    std::array<const char*,4> effect_names;
    // Original byte strings: D5E430,D5E40C,D5E3F0,D5E3E4,D5E3D0,
    // D5E3C0,D5E3B0,D5E430,D5E3A8,D5E3A0,D5E384,D5E374.
    std::array<const char*,12> parameter_names;
};

// Persistent, immovable host attempt storage. Actual owners and their current
// fields/refcounts remain authoritative. These records never acquire a second
// resource credit and do not free, unbind, roll back or replay failed work.
// Keep every child frame alive on failure, including partially prepared host
// companions. Existing child blocks require explicit external quiescence before
// destruction/reset. Normal return here is another FRONTIER, not full init.
struct NativeRenderResourceInitContinuationState final {
    enum class Phase { fresh, preparing, running, awaiting_b11599_continuation,
        post_running, awaiting_later_continuation, failed };
    NativeRenderResourceInitContinuationState() = default;
    NativeRenderResourceInitContinuationState(const NativeRenderResourceInitContinuationState&) = delete;
    NativeRenderResourceInitContinuationState& operator=(const NativeRenderResourceInitContinuationState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitEntryState* entry{};
    // The same context object and original entry remain alive through every
    // dependent stage. Identities are host metadata, never resource credits.
    const NativeRenderResourceInitContinuationContext* context_identity{};
    const NativeRenderPassCompanionContext* pass_companions_identity{};
    const void* post_identity{};
    std::uint32_t native_site{};
    int native_state{-1};
    std::uint32_t temporary_mask_esp10{};
    std::uint32_t spill_esp14{},spill_esp20{},aligned_width_esp58{};
    std::uint32_t half_aligned_height_esp1d8{};
    std::uint32_t quarter_width{},quarter_height{};
    std::uint32_t ebp_bits{},edi_bits{};
    NativeTextureSurfaceReferenceIncrement captured_increment_ebp{};
    std::array<void*,5> raw_holders{},returned_holders{};
    std::array<NativeRenderTextureSurfaceOwnerArguments,5> holder_arguments{};
    std::array<NativeRenderTextureSurfaceOwnerAcquired,5> holders;
    NativeRendererSurfaceFactoryAcquired target;
    NativeRuntimeTextureCreationArguments half_texture_arguments{};
    NativeRuntimeTextureCreationAcquired half_texture;
    NativeTextureSurfaceGetterAcquired half_surface;
    // +60,+64,+18,+1C,+20,+24,+28, in native creation order.
    std::array<void*,7> raw_passes{};
    // Canonical metadata for these service-visible owners only. Native
    // constructors establish all cleanup pointers before binding; every
    // nonnull owner is bound BEFORE publication/initializer callbacks. Keep
    // bound companions until native terminal retirement, then quiescence.
    // A failed emplace retains its completed raw owner without caller cleanup.
    std::array<std::optional<NativeRenderPassReference>,7> pass_references;
    std::array<bool,7> pass_binding_started{},pass_published{};
    std::array<std::uint32_t,7> pass_binding_publication_sites{};
    std::array<NativeRenderPassInitializationBlock,4> passes;
    NativeLuminanceInitializationBlock luminance;
    NativeBrightPassInitializationBlock bright;
    NativeBloomInitializationBlock bloom;
    // Direct B4E470 children at +68,+6C,+80,+70. Their canonical companions
    // are the existing blocks, in the SAME actual-owner registration domain.
    std::array<void*,4> raw_posts{},returned_posts{};
    std::array<NativePostEffect20ConstructionBlock,4> posts;
    std::array<NativeString,16> names;
    std::array<bool,16> name_constructed{},name_return_started{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
    bool field70_published{};
};

// B107F0 NORMAL-PATH CONTINUATION FRAGMENT [B109BC,B11599). Requires the
// actual published entry state at B109BC; consumes it once, preserves its
// service/argument identities, and retains all acquired child frames. Original
// three argument cells remain borrowed; word0 is re-read at its two native sites.
// End BEFORE B11599. +70 receives the actual constructor result (possibly null);
// later configuration and the remainder [B11599,B13029) are not executed.
// No full initialization/destruction admission, native FH3/SEH/private-stack
// aliasing, arbitrary profiles, app binding or binary ABI claim.
void continue_native_render_resource_init_00b109bc_fragment(
    NativeRenderResourceInitEntryState&, NativeRenderResourceInitContinuationContext&,
    NativeRenderResourceInitContinuationState&);

} // namespace bsp
