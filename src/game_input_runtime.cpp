#include "bsp/game_input_runtime.hpp"
#include "bsp/native_input_backend_startup.hpp"
#include "bsp/native_input_backend_bindings.hpp"
#include "bsp/xlive_manager_owner.hpp"
#include <stdexcept>

namespace bsp::game {
struct GameInputRuntime::Impl final : NativeInputBackendBindingsCalls,
    NativeInputBackendSlotActivation, NativeInputCursorCalls,
    NativeInputActionRecordCalls {
    GameInputRuntimeBindings bound;
    NativeInputDeviceRuntime device_runtime;
    NativeInputBackendDirectInput direct_input;
    NativeInputBackendOwnerContext backend;
    NativeInputBackendStartupDeviceRuntime device_calls;
    NativeInputBackendBindingsContext bindings;
    NativeInputActionRecordsContext records;
    NativeInputActionStorageCalls record_storage;
    NativeInputActionOwnerContext actions;
    NativeInputCursorContext cursor;
    bool started{};

    explicit Impl(GameInputRuntimeBindings b)
        : bound(b), device_runtime(b.devices),
          backend{b.lifetime, b.backend_00f8bbf4, device_runtime, direct_input},
          device_calls(device_runtime), bindings{*this},
          records{{b.binding_one_bits_00d7a24c}, *this}, record_storage(records),
          actions{b.actions_00f8bbf8, b.lifetime, record_storage},
          cursor{b.backend_00f8bbf4, actions, b.cursor_globals,
              b.loading_step_00d7a2f0, b.show_cursor, *this} {
        if (!b.lookup_device_004ba6d0 || !b.show_cursor)
            throw std::invalid_argument("input runtime requires its real lookup and ShowCursor providers");
    }
    std::uint32_t device_class_vslot08(void* p, std::uint32_t profile) override {
        return device_runtime.device_class_vslot08(p, profile);
    }
    std::int32_t identifier_vslot34(void* p, std::uint32_t profile) override {
        return device_runtime.identifier_vslot34(p, profile);
    }
    void delete_vslot04(void* p, std::uint32_t profile, std::uint32_t flags) override {
        device_runtime.delete_vslot04(p, profile, flags);
    }
    void enumerate_devices_vslot10(void* input, std::uint32_t type,
        void* p, std::uint32_t flags) override {
        device_runtime.enumerate_devices_vslot10(input, type, p, flags);
    }
    void devices_changed_d8(std::uint32_t identity, std::uint32_t type,
        std::int32_t index) override {
        invoke_native_input_startup_callback_004b4630(identity, type, index);
    }
    void activate_slot_00a91620(void* p, std::uint32_t type, std::uint32_t slot) override {
        activate_native_input_device_slot_00a91620(p, type, slot, bindings);
    }
    PlatformManagerFlags* current_platform_manager_00f8abe8() noexcept override {
        auto* const online = bound.online_00f8abe8;
        return online ? &online->context.flags : nullptr;
    }
    void pump_platform_manager_00a409f0(PlatformManagerFlags& flags) override {
        auto* const online = bound.online_00f8abe8;
        if (!online || &online->context.flags != &flags)
            throw std::logic_error("input cursor requires its current published online owner");
        pump_xlive_system_00a409f0(online->context);
    }
    void* call_004ba6d0(void* p, std::int32_t type, std::uint32_t index) override {
        return bound.lookup_device_004ba6d0(p, static_cast<std::uint32_t>(type), index);
    }
    void call_00a90ee0(void* p, void* device) override {
        remove_active_native_input_device_00a90ee0(p, device, bindings);
    }
    void call_00bebf30(void* p, std::int32_t type) override {
        delete_native_input_device_class_00bebf30(p, static_cast<std::uint32_t>(type), bindings);
    }
    void call_00a91620(void* p, std::int32_t type, std::int32_t slot) override {
        activate_slot_00a91620(p, static_cast<std::uint32_t>(type), static_cast<std::uint32_t>(slot));
    }
    void backend_vslot0c(void* p, std::uint32_t profile) override {
        // Groups profile +0C is A90ED0 (C3). Its pure slot is +10,
        // reached by the tick prepass, not by enumeration at +0C.
        if (profile == 0x00d5b5f8) return;
        if (profile != 0x00d5b72c)
            throw std::invalid_argument("input enumeration requires the concrete DirectInput backend profile");
        enumerate_native_input_devices_00a983c0(p, bindings);
    }
    void backend_vslot04(void* p, std::uint32_t profile, float seconds) override {
        invoke_native_input_backend_update_vslot04(p, profile, seconds, device_calls, *this);
    }
    void call_00a9a140(void* p, std::uint32_t flags) override {
        device_runtime.set_mouse_cooperative_level_00a9a140(p, flags);
    }
    void call_listener_slot0(void* p, std::uint32_t profile) override {
        auto* const calls = bound.listener_calls;
        if (!calls)
            throw std::logic_error("input action listener release reached an unbound application provider");
        calls->call_listener_slot0(p, profile);
    }
};

GameInputRuntime::GameInputRuntime(GameInputRuntimeBindings b) : impl_(std::make_unique<Impl>(b)) {}
GameInputRuntime::~GameInputRuntime() = default;
NativeInputBackendOwnerContext& GameInputRuntime::backend_context() noexcept { return impl_->backend; }
NativeInputActionOwnerContext& GameInputRuntime::action_context() noexcept { return impl_->actions; }
NativeInputDeviceRuntime& GameInputRuntime::devices() noexcept { return impl_->device_runtime; }
void GameInputRuntime::startup() {
    if (impl_->started || impl_->bound.backend_00f8bbf4)
        throw std::logic_error("input backend startup requires its unconstructed publication");
    create_and_reset_native_input_backend(impl_->backend, impl_->device_calls);
    impl_->started = true;
}
void GameInputRuntime::update_cursor(bool loading) {
    auto* const platform = impl_->bound.devices.platform_0109cf04;
    if (!platform) throw std::logic_error("input cursor requires the canonical platform publication");
    update_native_input_cursor_00becb20(*platform, loading, impl_->cursor);
}
void GameInputRuntime::update_backend(float seconds) {
    void* const backend = impl_->bound.backend_00f8bbf4;
    if (!backend) throw std::logic_error("input update requires the actual published backend");
    const auto profile = *static_cast<const volatile std::uint32_t*>(backend);
    impl_->backend_vslot04(backend, profile, seconds);
}
void* GameInputRuntime::action_owner() { return get_native_input_action_owner_004bec00(impl_->actions); }
void GameInputRuntime::release_sdk_after_native_drain() {
    if (impl_->bound.backend_00f8bbf4 || impl_->bound.actions_00f8bbf8)
        throw std::logic_error("input SDK release requires completed native backend/action drain");
    impl_->bound.devices.sdk.release_tracked_references();
    impl_->direct_input.release_tracked_references();
}
} // namespace bsp::game
