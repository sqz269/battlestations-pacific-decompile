#pragma once
#include "bsp/native_string.hpp"
#include "bsp/native_platform_load_messages.hpp"
#include <array>
#include <exception>
#include <memory>
#include <optional>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameNativeShadowTargetApplication;
class GameInputRuntime;

enum class GameNativeGuiEffectProbeStage {
    prepared, shadow, shader_cache, sampler_parent, effect_parent, names, first_effect,
    second_effect, shader_cache_release, ready, retiring_references, retiring_names, retired,
    closing, closed, failed, fakemotion_preload, fakemotion_warm_hit
};
struct GameNativeGuiEffectProbeCacheObservation {
    bool sampled{};
    void* owner{}; // Opaque after the normal cache close.
    std::uint32_t cursor{}, count{}; // Read only while this owner is live.
    std::uint8_t source_mode{}, load_variants{};
};
struct GameNativeGuiEffectProbeResource {
    const char* requested_name{};
    NativeString native_name; // Stable actual8h header in retained heap metadata.
    // Warm fakemotion borrows the cold slot's EXACT header; it owns no name.
    const NativeString* input_name{};
    bool name_entered{}, name_live{};
    std::size_t operation_index{};
    const void* retained_operation{};
    bool acquisition_entered{};
    void* caller_reference{}; // Exactly one returned owning credit until retired.
    void* returned_identity{}; // Opaque after retirement; never another owner.
    std::uint32_t profile{};
    std::int32_t references_on_return{}, references_before_release{};
    bool fallback_call_seen{}, release_entered{}, caller_released{};
    bool companion_present_after_release{};
    std::optional<std::int32_t> references_after_release;
    GameNativeGuiEffectProbeCacheObservation cache_before, cache_after;
    std::uint32_t cache_native_site{};
    bool cache_published{}, cold_loader_entered{};
};
struct GameNativeGuiEffectProbeSelectedSampler {
    void* descriptor{};
    void* sampler{};
    std::int32_t count{}, texture_source{}, index{}, type{};
    std::uint32_t mode_mask{};
    std::uint8_t vertex_sampler{};
    bool inspected{}, scene_texture_name{}, dummy_combiner{};
};
struct GameNativeGuiEffectProbeSamplerCall {
    void* builder{};
    void* pass{};
    void* mode_descriptor{};
    std::uint32_t following_root_site{}, following_mode_site{};
    std::int32_t mode_sampler_count{};
    GameNativeGuiEffectProbeSelectedSampler root;
};
struct GameNativeGuiEffectProbeObservation {
    GameNativeGuiEffectProbeStage stage{GameNativeGuiEffectProbeStage::prepared};
    GameNativeGuiEffectProbeStage failure_stage{GameNativeGuiEffectProbeStage::prepared};
    std::exception_ptr failure;
    std::array<GameNativeGuiEffectProbeResource, 2> resources;
    // Default-off extension: two distinct returned credits, one actual name.
    bool include_fakemotion{};
    const void* effect_application_identity{};
    GameNativeGuiEffectProbeResource fakemotion_preload, fakemotion_warm;
    GameNativeGuiEffectProbeSelectedSampler fakemotion_sampler;
    // Actual root/mode inputs at each compiler invocation. The observation hook
    // supplies NO entry/release evidence; the existing provider guard remains.
    std::array<GameNativeGuiEffectProbeSamplerCall, 2> fakemotion_sampler_calls;
    std::size_t fakemotion_sampler_call_count{};
    std::int32_t fakemotion_references_before_cache_release{},
        fakemotion_references_after_cache_release{};
    bool fakemotion_warm_verified{};
    // Schedule the existing renderer/process cache, without another owner or
    // gate override. Failed creation/release keeps its actual frame retained.
    bool cache_creation_entered{}, cache_creation_returned{};
    bool cache_release_entered{}, cache_release_returned{};
    void* cache_identity{}; // Opaque after release, never dereferenced then.
    std::uint8_t source_mode_on_create{}, load_variants_on_create{};
};

// Opt-in actual-resource probe, not frontend publication/Text/draw. Construct
// and retain this heap metadata first, then call run() after genuine phase7.
// Parent graphs, actual names, acquisitions and failures cannot be abandoned.
// No fake MSG, stack-word capture, wrapper effect or extra native retain exists.
class GameNativeGuiEffectProbe final {
public:
    GameNativeGuiEffectProbe(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameSingletonHost&, GameNativeReadOnlyData&,
        GameNativeShadowTargetApplication&, GameInputRuntime&,
        NativePlatformLoadPublication actual_platform_0109cf04);
    // Borrows the startup host's completed canonical shadow application through
    // shared drain. This probe never allocates or closes another shadow owner.
    // Borrows the caller's existing cell, including renderer's ready partial
    // focus-domain publication. Its receiver supports reached +41 load-pump
    // reads only; this does not admit a whole-platform owner or original ABI.
    ~GameNativeGuiEffectProbe() noexcept;
    GameNativeGuiEffectProbe(const GameNativeGuiEffectProbe&) = delete;
    GameNativeGuiEffectProbe& operator=(const GameNativeGuiEffectProbe&) = delete;
    // Actual GuiFont.mshd/GuiFontBilinear.mshd acquisitions. Optional fakemotion
    // runs afterward, before the SAME normal cache close, then verifies a warm
    // same-owner acquisition with cache publication null. No second graph/cache.
    // Returned observation
    // exposes exact raw caller credits still OWNED by these slots; callers may
    // inspect/borrow them but must not release/publish them independently.
    const GameNativeGuiEffectProbeObservation& run(bool include_fakemotion = false);
    // Reverse slot order, existing B32010 canonical/current-profile release,
    // then native input-name destruction before original string-pool drain.
    void retire_caller_references();
    // Caller performs original shared singleton/renderer drain while this and
    // every borrowed provider remain alive, then closes metadata here.
    void after_native_renderer_drain();
    const GameNativeGuiEffectProbeObservation& observation() const noexcept;
    bool requires_process_retention() const noexcept;
    // Precise retained cache/loader/descriptor/compiler/sampler states/sites.
    // Failed methods emit this before throwing; caller's no-unwind process-exit
    // policy must retain the heap probe and actual application service graph.
    void emit_failure_observations() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
