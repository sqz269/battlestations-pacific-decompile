#pragma once
#include "bsp/game_input_runtime.hpp"
#include "bsp/native_input_action_binding_runtime.hpp"
#include "bsp/native_input_action_configuration.hpp"
#include "bsp/native_input_action_contexts.hpp"
#include "bsp/native_input_action_deadlines.hpp"
#include "bsp/native_input_action_tick.hpp"
#include <memory>

namespace bsp::game {
// Required application dispatch for the captured F8BBFC identity. The known
// writer installs6965A0; that callback must use its real game clock and raw map.
// This composition does not install or substitute a callback.
struct GameInputActionCallbacks {
    virtual ~GameInputActionCallbacks() = default;
    virtual void post_tick(std::uint32_t captured_identity) = 0;
};
struct GameInputActionServices {
    const CameraAxesCrtAccess& crt;
    NativeInputActionBindingConstants constants;
    NativeInputActionTimingGlobals timing;
    const volatile float& zero_00d7a218;
    const volatile std::uint32_t& callback_00f8bbfc;
    GameInputActionCallbacks& callbacks;
};

// Finite binding for the real6965A0 callback, over the existing runtime's raw
// action publication and the caller's sole game/map storage. It borrows the
// actual game publication (game clock at+64C) and a real map-subscript service.
class GameInputDeadlineCallback final : public GameInputActionCallbacks {
public:
    GameInputDeadlineCallback(GameInputRuntime&, void* volatile& game_00e188a8,
        void* actual_map_00e18a7c, const volatile double& delay_00ce65d0,
        NativeInputActionDeadlineCalls&);
    void post_tick(std::uint32_t captured_identity) override;
private:
    void* volatile& game_;
    NativeInputActionDeadlineContext deadlines_;
};

// Source composition over the existing runtime's publications, raw records and
// finite devices. It owns only service bindings; runtime and all borrowed cells
// must outlive it. Application setup must populate actions at the native stage.
class GameInputActions final {
public:
    GameInputActions(GameInputRuntime&, GameInputActionServices);
    ~GameInputActions();
    GameInputActions(const GameInputActions&) = delete;
    GameInputActions& operator=(const GameInputActions&) = delete;
    // Native caller sequence: lazy4BEC00 getter then actual A92C40 frame tick.
    // Requires the constructed backend and valid actual action storage. Reached
    // callbacks use the required provider and may propagate their exceptions.
    void update(float seconds);
    // A93C80 uses the same raw owner/record/listener services as frame updates.
    // Contexts is an actual12h DWORD-vector header, borrowed through the call.
    void configure(std::uint32_t action_index, const void* contexts, std::uint8_t replace_listener);
    void set_context_level(std::uint32_t context_index, std::uint32_t level);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
