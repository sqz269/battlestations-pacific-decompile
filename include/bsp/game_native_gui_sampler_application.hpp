#pragma once
#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/native_material_descriptor_samplers.hpp"
#include <optional>

namespace bsp::game {
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameInputRuntime;

// Evidence for THIS identified compiler invocation only. Null entry means
// unknown, not zero. Release arrays are distinct root/mode captures, borrowed
// through completion/failure. Never apply one captured word universally.
struct GameNativeGuiSamplerCallEvidence {
    std::optional<std::uint32_t> root_entry_word;
    const NativeMaterialDescriptorSamplerReleaseScratch* root_release{};
    std::size_t root_release_count{};
    const NativeMaterialDescriptorSamplerReleaseScratch* mode_release{};
    std::size_t mode_release_count{};
};
using GameNativeGuiSamplerEvidenceLookup = GameNativeGuiSamplerCallEvidence (*)(void*,
    NativeMaterialProgramBuilderStorage&, NativeMaterialPassStorage&,
    NativeShaderDescriptorStorage& root, NativeShaderDescriptorStorage& mode);
struct GameNativeGuiSamplerApplicationBindings {
    GameInputRuntime& input;
    NativePlatformLoadPublication actual_platform_0109cf04;
    NativePlatformLoadMessagePreimage native_message_preimage{};
    // Optional qualified capture; empty keeps Original MSG output bytes unwritten.
    void* evidence_context{};
    GameNativeGuiSamplerEvidenceLookup evidence{}; // Null retains unknown root entry.
};

// Canonical sampler parent. Borrows existing AA0/F8D41C/F8D420 cells, real VFS,
// input/online/XLive message services and numeric factory/cache profiles. No
// native manager/clock/registry/MSG default is created. Actual procedural creator
// companions register into the renderer's SAME owner registry without a retain.
// Root+mode share one retained per-compiler W=P-30h and its knowledge; separate
// calls never reuse a previous invocation's residue. Unknown consumption fails.
// Hold this parent, optional captured message/evidence and providers through failed frames
// and the original shared drain. No automatic native rollback is supplied.
class GameNativeGuiSamplerApplication final {
public:
    GameNativeGuiSamplerApplication(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameSingletonHost&, GameNativeReadOnlyData&,
        GameNativeGuiSamplerApplicationBindings);
    ~GameNativeGuiSamplerApplication() noexcept;
    GameNativeGuiSamplerApplication(const GameNativeGuiSamplerApplication&) = delete;
    GameNativeGuiSamplerApplication& operator=(const GameNativeGuiSamplerApplication&) = delete;
    // Real effect bindings over this parent's exact domains and the canonical
    // F8BBF0 shadow-target policy publication, never a separate material manager.
    GameNativeGuiEffectApplicationBindings effect_bindings();
    std::size_t live_call_scopes() const noexcept;
    std::size_t live_procedural_creators() const noexcept;
    bool requires_process_retention() const noexcept;
    // Only after same renderer/native singleton drain and caller effect/failed
    // frame closure. Verifies metadata quiescence, never releases owners itself.
    void after_native_renderer_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
