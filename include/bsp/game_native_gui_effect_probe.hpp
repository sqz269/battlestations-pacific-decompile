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
class GameNativeSettingsProcess;
class GameInputRuntime;

enum class GameNativeGuiEffectProbeStage {
    prepared, shadow, sampler_parent, effect_parent, names, first_effect,
    second_effect, ready, retiring_references, retiring_names, retired,
    closing, closed, failed
};
struct GameNativeGuiEffectProbeResource {
    const char* requested_name{};
    NativeString native_name; // Stable actual8h header in retained heap metadata.
    bool name_entered{}, name_live{};
    std::size_t operation_index{};
    bool acquisition_entered{};
    void* caller_reference{}; // Exactly one returned owning credit until retired.
    void* returned_identity{}; // Opaque after retirement; never another owner.
    std::uint32_t profile{};
    std::int32_t references_on_return{}, references_before_release{};
    bool fallback_call_seen{}, release_entered{}, caller_released{};
    bool companion_present_after_release{};
    std::optional<std::int32_t> references_after_release;
};
struct GameNativeGuiEffectProbeObservation {
    GameNativeGuiEffectProbeStage stage{GameNativeGuiEffectProbeStage::prepared};
    GameNativeGuiEffectProbeStage failure_stage{GameNativeGuiEffectProbeStage::prepared};
    std::exception_ptr failure;
    std::array<GameNativeGuiEffectProbeResource, 2> resources;
};

// Opt-in actual-resource probe, not frontend publication/Text/draw. Construct
// and retain this heap metadata first, then call run() after genuine phase7.
// Parent graphs, actual names, acquisitions and failures cannot be abandoned.
// No fake MSG, stack-word capture, wrapper effect or extra native retain exists.
class GameNativeGuiEffectProbe final {
public:
    GameNativeGuiEffectProbe(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameSingletonHost&, GameNativeReadOnlyData&,
        GameNativeSettingsProcess&, GameInputRuntime&,
        NativePlatformLoadPublication actual_platform_0109cf04);
    // Borrows the caller's existing cell, including renderer's ready partial
    // focus-domain publication. Its receiver supports reached +41 load-pump
    // reads only; this does not admit a whole-platform owner or original ABI.
    ~GameNativeGuiEffectProbe() noexcept;
    GameNativeGuiEffectProbe(const GameNativeGuiEffectProbe&) = delete;
    GameNativeGuiEffectProbe& operator=(const GameNativeGuiEffectProbe&) = delete;
    // Actual GuiFont.mshd/GuiFontBilinear.mshd acquisitions. Returned observation
    // exposes exact raw caller credits still OWNED by these slots; callers may
    // inspect/borrow them but must not release/publish them independently.
    const GameNativeGuiEffectProbeObservation& run();
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
