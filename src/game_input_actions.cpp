#include "bsp/game_input_actions.hpp"

namespace bsp::game {
struct GameInputActions::Impl final : NativeInputActionTickCalls {
    GameInputRuntime& runtime;
    GameInputActionServices bound;
    NativeInputActionBindingContext bindings;
    NativeInputActionConfigurationContext configuration;
    NativeInputActionTickContext tick;
    Impl(GameInputRuntime& input, GameInputActionServices services)
        : runtime(input), bound(services),
          bindings{input.devices(), services.crt, services.constants},
          configuration{input.records_context(), bindings, services.timing},
          tick{input.backend_context().global_00f8bbf4, services.callback_00f8bbfc,
              services.zero_00d7a218, services.timing, *this} {}
    void backend_vslot04(void* backend, std::uint32_t profile, float seconds) override {
        runtime.backend_update_vslot04(backend, profile, seconds);
    }
    void call_00a922a0(void* owner) override {
        rebind_all_native_input_actions_00a922a0(owner,
            runtime.backend_context().global_00f8bbf4);
    }
    void call_00a92370(void* action) override {
        poll_native_input_action_00a92370(action, bindings);
    }
    void post_tick_callback_00f8bbfc(std::uint32_t identity) override {
        bound.callbacks.post_tick(identity);
    }
};
GameInputActions::GameInputActions(GameInputRuntime& runtime, GameInputActionServices services)
    : impl_(std::make_unique<Impl>(runtime, services)) {}
GameInputActions::~GameInputActions() = default;
void GameInputActions::update(float seconds) {
    void* const owner = impl_->runtime.action_owner();
    update_native_input_action_owner_00a92c40(owner, seconds, impl_->tick);
}
void GameInputActions::configure(std::uint32_t index, const void* contexts,
    std::uint8_t replace_listener) {
    void* const owner = impl_->runtime.action_owner();
    configure_native_input_action_00a93c80(owner, index, contexts, replace_listener,
        impl_->configuration);
}
} // namespace bsp::game
