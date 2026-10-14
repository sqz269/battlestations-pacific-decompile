#pragma once
#include "bsp/native_material_effect_cache.hpp"
#include <exception>
#include <memory>

namespace bsp {
struct NativeMaterialDescriptorSamplersContext;
class NativeMaterialDescriptorSamplersCallFactory;
}
namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;

// Genuine caller inputs, borrowed unchanged. The root/mode calls have distinct
// native B3B280 entry/release evidence. Unknown B-16 residue stays unknown;
// existing helpers reject its consumption rather than manufacture stack bytes.
// All contexts, globals, providers and applicable evidence must survive every
// retained compiler frame and actual effect retirement. Serialize updates.
struct GameNativeGuiEffectApplicationBindings {
    NativeMaterialDescriptorSamplersContext& root_samplers;
    NativeMaterialDescriptorSamplersContext& mode_samplers;
    void* const volatile& material_manager_00f8bbf0;
    // F8BBF0 above is the existing shadow target's policy+0E publication;
    // "material_manager" is a historical descriptive hypothesis, not a new
    // owner type. This factory supplies genuine per-compiler shared stack state.
    NativeMaterialDescriptorSamplersCallFactory* sampler_calls{};
};

enum class GameNativeGuiEffectLoadPhase { fresh, running, complete, failed };
struct GameNativeGuiEffectLoadOperation final {
    GameNativeGuiEffectLoadPhase phase{GameNativeGuiEffectLoadPhase::fresh};
    // Stable borrowed actual8h header, retained by the caller through failure.
    const void* input_name{};
    void* captured_renderer{};
    NativeMaterialEffectCacheAcquired cache;
    // Normal return transfers the actual owning reference to the caller.
    // After that this is an opaque diagnostic identity, not another owner.
    void* result{};
    std::uint32_t result_profile{};
    std::int32_t result_references{}; // Observed actual+04, never a replacement counter.
    std::exception_ptr failure;
};

// Real numeric renderer48/cache/loader/program/compiler composition over the
// SAME ready renderer, VFS, string pool, actual effects/passes/shaders/states
// and shared record allocator. No semantic shader/material wrapper, copied
// registry or callable table containing original numeric addresses is built.
// B318B0's fixed0/1/1 route returns an actual held effect (or native null).
// It is the missing production supplier for GuiFont.mshd/GuiFontBilinear.mshd;
// constructing this graph does not yet attach the current sprite frontend.
//
// Actual B3B3C0, B3C3A0, B43B00, B46950, B2EBB0 and B318B0 bodies are composed
// from existing established helpers. No new raw register/x87 recovery or ABI
// claim is introduced. C++ failures retain their one-shot nested frames and
// prohibit replay/retarget. Preserve this graph with the application's process-
// retention policy on failure; its destructor never performs native rollback.
class GameNativeGuiEffectApplication final {
public:
    GameNativeGuiEffectApplication(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameNativeGuiEffectApplicationBindings);
    ~GameNativeGuiEffectApplication() noexcept;
    GameNativeGuiEffectApplication(const GameNativeGuiEffectApplication&) = delete;
    GameNativeGuiEffectApplication& operator=(const GameNativeGuiEffectApplication&) = delete;

    // Caller supplies initialized live actual8h name storage. A completed
    // return transfers exactly the native caller reference: no additional
    // retain. Retire it through existing actual-owner paths before shared drain.
    void* acquire_effect_00b318b0(const void* stable_actual_name_header);
    std::size_t operation_count() const noexcept;
    const GameNativeGuiEffectLoadOperation& operation(std::size_t) const;
    bool requires_process_retention() const noexcept;
    const std::exception_ptr& failure() const noexcept;
    // Keep this graph alive through renderer cache/effect/compiler retirement.
    // After successful renderer native drain this verifies that same drain and
    // permits metadata destruction. Never clears caches, releases caller refs,
    // discards failures or supplies a second teardown order. A never-entered
    // graph may be destroyed without this call.
    void after_native_renderer_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
