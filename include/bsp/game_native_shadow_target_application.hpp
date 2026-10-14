#pragma once
#include "bsp/native_shadow_depth_target_owner.hpp"
#include <exception>
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameNativeSettingsProcess;

struct GameNativeShadowTargetObservation {
    enum class Stage {
        prepared, settings_captured, allocating, allocation_captured,
        constructing, ready, failed, closed
    };
    Stage stage{Stage::prepared};
    Stage failure_stage{Stage::prepared};
    std::uint8_t hires_shadow{}; // Host metadata; assigned from actual +1Dh once.
    NativeShadowDepthTargetDimensions dimensions{};
    void* allocated{};
    NativeShadowDepthTargetStorage* returned{};
    bool constructor_entered{};
    std::size_t constructor_operation_index{};
    std::exception_ptr failure;
};

// Source application parent over the existing renderer's AA0/F8BBF0/texture/
// terminal domain. Create and retain this metadata BEFORE calling construct().
// A8FE30 itself publishes/registers the sole genuine2Ch owner. This parent adds
// neither a native owner reference nor an enable/create/resource call.
// This is not a replacement for the register ABI/whole body of 0073D410.
class GameNativeShadowTargetApplication final {
public:
    GameNativeShadowTargetApplication(GameHostLog&, GameNativeRendererApplication&,
        GameNativeSettingsProcess&);
    ~GameNativeShadowTargetApplication() noexcept;
    GameNativeShadowTargetApplication(const GameNativeShadowTargetApplication&) = delete;
    GameNativeShadowTargetApplication& operator=(const GameNativeShadowTargetApplication&) = delete;
    // One-shot normal producer: capture canonical settings+1Dh, save square
    // extent, allocate raw2Ch, call established A8FE30 only for nonnull storage.
    // Entered failure keeps allocation/publication/section/nested operation;
    // caller must preserve this heap graph and its providers until process exit.
    void construct();
    const GameNativeShadowTargetObservation& observation() const noexcept;
    const NativeShadowDepthTargetOperation* constructor_operation() const noexcept;
    // Borrow only a still-published completed owner. Null is preserved if the
    // allocator returned null; never construct a default target for that branch.
    NativeShadowDepthTargetStorage* target() const;
    bool requires_process_retention() const noexcept;
    // Original native singleton drain must retire target while renderer/context
    // are alive. This verifies closure and retires metadata only; no deletion.
    void after_native_renderer_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
