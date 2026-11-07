#include "bsp/game_input_runtime.hpp"
#include "bsp/band_004b_cc13.hpp"
#include "bsp/cc13_front2.hpp"
#include "bsp/native_input_backend_startup.hpp"
#include "bsp/native_input_backend_bindings.hpp"
#include "bsp/native_input_class_configuration.hpp"
#include "bsp/native_input_action_listener_owner.hpp"
#include "bsp/native_gamepad_rumble.hpp"
#include "bsp/render_tail.hpp"
#include <optional>
#include <cstring>
#include <stdexcept>

namespace bsp::game {
struct GameInputRuntime::Impl final : NativeInputBackendBindingsCalls,
    NativeInputBackendSlotActivation, NativeInputCursorDeviceCalls,
    NativeInputActionRecordCalls, NativeGamepadRumbleOutput, DynamicInputButtonQueryHost {
    GameInputRuntimeBindings bound;
    NativeInputDeviceRuntime device_runtime;
    NativeInputBackendDirectInput direct_input;
    NativeInputBackendOwnerContext backend;
    NativeInputBackendStartupDeviceRuntime device_calls;
    NativeInputBackendBindingsContext bindings;
    NativeInputActionListenerCalls native_listeners;
    NativeInputActionRecordsContext records;
    NativeInputActionStorageCalls record_storage;
    NativeInputActionOwnerContext actions;
    NativeInputCursorContext cursor;
    NativePlatformLoadMessagesContext platform_load;
    std::optional<NativePlatformLoadMessagesOperation> cursor_operation{std::in_place};
    NativeGamepadRumbleContext rumble;
    bool started{};

    explicit Impl(GameInputRuntimeBindings b)
        : bound(b), device_runtime(b.devices),
          backend{b.lifetime, b.backend_00f8bbf4, device_runtime, direct_input},
          device_calls(device_runtime), bindings{*this},
          records{{b.binding_one_bits_00d7a24c}, *this}, record_storage(records),
          actions{b.actions_00f8bbf8, b.lifetime, record_storage},
          cursor{b.backend_00f8bbf4, actions, b.cursor_globals,
              b.loading_step_00d7a2f0, b.show_cursor, *this},
          platform_load{b.online_00f8abe8, b.backend_00f8bbf4,
              b.online_pump, &cursor, b.xlive},
          rumble{b.backend_00f8bbf4, b.rumble_enabled_00e12f2c, device_runtime, *this} {
        if (!b.lookup_device_004ba6d0 || !b.show_cursor)
            throw std::invalid_argument("input runtime requires its real lookup and ShowCursor providers");
        if (&b.rumble_enabled_00e12f2c != &b.devices.xinput_tables.rumble_enabled)
            throw std::invalid_argument("input rumble setter and devices must share the same mutable word");
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
        if (calls) calls->call_listener_slot0(p, profile);
        else native_listeners.call_listener_slot0(p, profile);
    }
    void set_force_vslot38(void* p, std::uint32_t profile, std::uint32_t channel,
        float value) override {
        device_runtime.set_force_vslot38(p, profile, channel, value);
    }
    void invalid_parameter_00bf6713() override {
        device_runtime.invalid_parameter_00bf6713();
    }
    bool input_device_vcall_2c(void* device) override {
        // Dispatch the current native profile on the same vector element.
        return device_runtime.buttons_active_vslot2c(device) != 0;
    }
};

GameInputRuntime::GameInputRuntime(GameInputRuntimeBindings b) : impl_(std::make_unique<Impl>(b)) {}
GameInputRuntime::~GameInputRuntime() = default;
NativeInputBackendOwnerContext& GameInputRuntime::backend_context() noexcept { return impl_->backend; }
NativeInputActionOwnerContext& GameInputRuntime::action_context() noexcept { return impl_->actions; }
NativeInputActionRecordsContext& GameInputRuntime::records_context() noexcept { return impl_->records; }
NativeInputCursorContext& GameInputRuntime::cursor_context() noexcept { return impl_->cursor; }
NativePlatformLoadMessagesContext& GameInputRuntime::platform_load_context() noexcept {
    impl_->platform_load.online = impl_->bound.online_pump;
    return impl_->platform_load;
}
NativeInputDeviceRuntime& GameInputRuntime::devices() noexcept { return impl_->device_runtime; }
void GameInputRuntime::startup() {
    if (impl_->started || impl_->bound.backend_00f8bbf4)
        throw std::logic_error("input backend startup requires its unconstructed publication");
    create_and_reset_native_input_backend(impl_->backend, impl_->device_calls);
    impl_->started = true;
}
void GameInputRuntime::initialize_classes_004dd6a8() {
    for (std::uint32_t type = 0; type != 3; ++type) {
        void* const backend = impl_->bound.backend_00f8bbf4;
        if (!backend) throw std::logic_error("input class initialization requires the current backend");
        configure_native_input_class_00a917e0(backend, type, 1, nullptr);
    }
}
void GameInputRuntime::set_rumble_enabled_00a94c50(bool enabled) {
    set_native_gamepad_rumble_enabled_00a94c50(enabled, impl_->rumble);
}
void GameInputRuntime::update_cursor(bool loading) {
    auto* const platform = impl_->bound.devices.platform_0109cf04;
    if (!platform) throw std::logic_error("input cursor requires the canonical platform publication");
    update_cursor(*platform, loading);
}
void GameInputRuntime::update_cursor(Win32PlatformState& platform, bool loading) {
    using Phase = NativePlatformLoadMessagesOperation::Phase;
    if (impl_->cursor_operation->phase == Phase::complete)
        impl_->cursor_operation.emplace();
    else if (impl_->cursor_operation->phase != Phase::fresh)
        throw std::logic_error("input cursor cannot replay an interrupted native operation");
    // Cursor-only entry never reads MSG. Preserve its untouched caller preimage.
    update_native_platform_load_cursor_00becb20(&platform.native_window_focus(),
        static_cast<std::uint8_t>(loading), platform_load_context(), *impl_->cursor_operation);
}
void GameInputRuntime::update_backend(float seconds) {
    void* const backend = impl_->bound.backend_00f8bbf4;
    if (!backend) throw std::logic_error("input update requires the actual published backend");
    const auto profile = *static_cast<const volatile std::uint32_t*>(backend);
    impl_->backend_vslot04(backend, profile, seconds);
}
void GameInputRuntime::backend_update_vslot04(void* backend, std::uint32_t profile, float seconds) {
    impl_->backend_vslot04(backend, profile, seconds);
}
bool GameInputRuntime::poll_dynamic_button_edge() {
    // Normal-frame producer: GameFrameHost::game_on_move calls update_backend
    // with its incoming delta before dispatching to mission device-wait. Keep
    // this query read-only apart from the shared latch, so additional consumers
    // cannot resample hardware or advance device timers a second time.
    auto* const backend = static_cast<std::uint8_t*>(impl_->bound.backend_00f8bbf4);
    if (!backend) throw std::logic_error("input button edge requires the actual published backend");
    // Evidence: bsp.py ghidra disasm 0067D35B, MOV ECX,[F8BBF4] then
    // CALL 004B43B0; bsp.py show 004B43B0 --asm keeps that receiver in ESI
    // across 00A91020 and reads/writes ESI+DD only AFTER the query. The native
    // groups constructor 00A91570 already initializes this byte to zero.
    // Borrow its real checked device vectors and existing latch, without a
    // second device table, state snapshot, synthetic sample or latch owner.
    const auto down = static_cast<std::uint8_t>(
        input_device_table_any_dynamic_device_button_down_00a91020(backend, *impl_));
    return cc13_band004b::press_start_poll_button_edge_004b43b0(backend[0xdd], down);
}
bool GameInputRuntime::select_sound_device_active_00941140() {
    // 0094119B / 009411D1 MOV ESI,[F8BBF4]: the same published backend and the
    // same +2Ch dispatch 00A91020 uses above, without a second device table.
    const auto* const backend = static_cast<const std::uint8_t*>(impl_->bound.backend_00f8bbf4);
    if (!backend) throw std::logic_error("00941140 slot-1 test requires the actual published backend");
    return sound_request_select_device_active_00941140(backend, *impl_);
}
void* GameInputRuntime::action_owner() { return get_native_input_action_owner_004bec00(impl_->actions); }
void* GameInputRuntime::input_device(std::uint32_t device_class, std::uint32_t index) {
    void* const backend = impl_->bound.backend_00f8bbf4;
    if (!backend) throw std::logic_error("input device lookup requires the published native backend");
    return impl_->bound.lookup_device_004ba6d0(backend, device_class, index);
}
float GameInputRuntime::frontend_scroll_axis() {
    // bsp.py show 00683820 --asm: 0068383A/5C each call 004BEC00, then
    // load [owner+4]+36B4/3714. Those are action 123h/125h's +24 value
    // lanes in the actual 30h records produced by 00698A10.
    const auto read_axis = [this](std::size_t offset) {
        const auto* owner = static_cast<const std::byte*>(action_owner());
        const std::byte* records;
        std::uint32_t count;
        std::memcpy(&records, owner + 4, sizeof records);
        std::memcpy(&count, owner + 8, sizeof count);
        if (!records || count <= offset / 0x30)
            throw std::logic_error("front-end scroll requires the configured native action records");
        float axis;
        std::memcpy(&axis, records + offset, sizeof axis);
        return axis;
    };
    const float axis = read_axis(0x36b4);
    return axis == 0.0f ? read_axis(0x3714) : axis;
}
bool GameInputRuntime::frontend_scroll_pad_axis(std::uint32_t code, double& value) {
    // 00683885..A1 captures the current backend and returned device, then
    // invokes its current slot24. Use the application's own finite provider.
    void* const backend = impl_->bound.backend_00f8bbf4;
    if (!backend) throw std::logic_error("front-end scroll requires the published native backend");
    void* const pad = impl_->bound.lookup_device_004ba6d0(backend, 1, 0);
    if (!pad) return false;
    value = impl_->device_runtime.value_24(pad, code);
    return true;
}
void GameInputRuntime::release_sdk_after_native_drain() {
    if (impl_->bound.backend_00f8bbf4 || impl_->bound.actions_00f8bbf8)
        throw std::logic_error("input SDK release requires completed native backend/action drain");
    impl_->bound.devices.sdk.release_tracked_references();
    impl_->direct_input.release_tracked_references();
}
} // namespace bsp::game
