#pragma once
#include "bsp/native_input_device_runtime.hpp"
#include "bsp/native_input_action_records.hpp"
#include "bsp/native_input_cursor.hpp"
#include <memory>

namespace bsp { struct XLiveManagerOwner; }
namespace bsp::game {

using GameRawInputDeviceLookup = void* (*)(void*, std::uint32_t, std::uint32_t);

// Source application composition; every publication is borrowed from the same
// application. This facade adds no native manager, backend, action table or
// device projection. Services and publication cells survive raw manager drain.
struct GameInputRuntimeBindings {
    NativeInputDeviceRuntimeServices devices;
    SoundLifetimeAccess lifetime;
    void* volatile& backend_00f8bbf4;
    void* volatile& actions_00f8bbf8;
    const volatile std::uint32_t& binding_one_bits_00d7a24c;
    // A source provider publication, not a native field or substitute listener.
    // Required only when an actual record releases a nonnull listener to zero.
    NativeInputActionRecordCalls* volatile& listener_calls;
    PlatformCursorGlobals cursor_globals;
    XLiveManagerOwner* volatile& online_00f8abe8;
    const volatile float& loading_step_00d7a2f0;
    NativeInputShowCursorCall const& show_cursor;
    GameRawInputDeviceLookup lookup_device_004ba6d0;
};

class GameInputRuntime final {
public:
    explicit GameInputRuntime(GameInputRuntimeBindings);
    ~GameInputRuntime();
    GameInputRuntime(const GameInputRuntime&) = delete;
    GameInputRuntime& operator=(const GameInputRuntime&) = delete;
    NativeInputBackendOwnerContext& backend_context() noexcept;
    NativeInputActionOwnerContext& action_context() noexcept;
    NativeInputDeviceRuntime& devices() noexcept;
    // Native allocation/constructor, publication reload/callback write/reset.
    void startup();
    void update_cursor(bool loading);
    void update_backend(float seconds);
    void* action_owner();
    // Call after the shared raw manager drain while window and DLLs still live.
    // Does not initiate a second raw drain or repair surviving native state.
    void release_sdk_after_native_drain();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
