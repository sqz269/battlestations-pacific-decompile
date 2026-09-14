#pragma once
#include "bsp/native_input_keyboard_storage.hpp"
#include "bsp/native_input_settings_lifetime.hpp"
#include <cstdint>

namespace bsp::game {

// Native 006A7BE0 has separate private-stack words whose bytes are not wholly
// initialized. These values are an explicit source policy, not recovered image
// constants or a claim that the native stack contained these exact bits.
struct GameInputSettingsStackPolicy {
    std::uint32_t descriptor_flag_stack_preimage;
    std::uint32_t vector_opaque_stack_preimage;
    std::uint32_t sensitivity_default_stack_preimage;
};

struct GameInputSettingsRuntimeBindings {
    void* volatile& settings_publication_00e198e8;
    void* volatile& manager_publication_01090aa0;
    NativeStringStorage& strings;
    const NativeLuaBootstrapInputs& bootstrap;
    const NativeLuaFileServices& files;
    const bool& crt_sse2_conversion;
    const volatile float& one_00d7a24c;
    const volatile float& base_zero_replacement_00cf7fe8;
    GameInputSettingsStackPolicy stack_policy;
};

// Stable-address owner of the concrete keyboard provider and the service
// contexts required by 005547D0/006A7BE0. Construction does not allocate,
// publish or register native settings. The host must bind context() to the
// canonical raw manager's CF81CC deletion dispatch before calling get().
// Every borrowed service and this object must outlive that manager's drain.
class GameInputSettingsRuntime final {
public:
    explicit GameInputSettingsRuntime(GameInputSettingsRuntimeBindings bindings) noexcept;
    ~GameInputSettingsRuntime() = default;
    GameInputSettingsRuntime(const GameInputSettingsRuntime&) = delete;
    GameInputSettingsRuntime& operator=(const GameInputSettingsRuntime&) = delete;
    GameInputSettingsRuntime(GameInputSettingsRuntime&&) = delete;
    GameInputSettingsRuntime& operator=(GameInputSettingsRuntime&&) = delete;

    NativeInputSettingsLifetimeContext& context() noexcept { return lifetime_; }
    void* get();

private:
    NativeInputSettingsScriptServices scripts_;
    NativeInputKeyboardStorage keyboard_;
    NativeInputSettingsTableServices tables_;
    NativeInputSettingsLifetimeContext lifetime_;
};
} // namespace bsp::game
