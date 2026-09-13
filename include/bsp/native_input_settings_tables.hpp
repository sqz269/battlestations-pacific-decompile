#pragma once
#include "bsp/native_input_settings_startup.hpp"
#include "bsp/native_input_keyboard_apply.hpp"
#include <array>

namespace bsp {
// Required library-storage calls used by006A7BE0. Receivers, mapped results,
// iterators and keys are the ACTUAL native allocations. Numeric names identify
// original boundaries without inventing a complete STL implementation/provider.
// Existing keyboard-library calls share the same settings/device/tree storage.
struct NativeInputSettingsTableCalls {
    virtual ~NativeInputSettingsTableCalls() = default;
    virtual void call_006a7540(void* tree, void* subtree) = 0;
    virtual void call_006a0db0(void* header, std::uint32_t count, std::array<std::uint32_t,5> value) = 0;
    virtual void call_0049df50(void* header, std::uint32_t count, std::uint32_t value) = 0;
    virtual void* call_0055a9a0(void* tree, const NativeString* key) = 0;
    virtual void call_006a6350(void* header, std::uint32_t count, std::array<std::uint32_t,4> value) = 0;
    virtual void* call_0069fa40(void* tree, void* output, const std::int32_t* key) = 0;
    virtual void* call_006a1e70(void* tree, const NativeString* key) = 0;
    virtual void call_006a79a0(void* header, std::uint32_t count, std::array<std::uint32_t,4> value) = 0;
    virtual void call_0049e050(void* header, std::uint32_t count, std::array<std::uint32_t,2> value) = 0;
    virtual void call_006a4710(void* header, std::uint32_t count, std::array<std::uint32_t,4> value) = 0;
    virtual void call_00492210(void* header, std::uint32_t count, std::uint32_t value) = 0;
    virtual void* call_006a6900(void* tree, const NativeString* key) = 0;
    virtual void* call_006a1f80(void* tree, const std::int32_t* key) = 0;
};
struct NativeInputSettingsTableServices {
    NativeInputSettingsScriptServices& scripts;
    NativeInputKeyboardLibrary& trees;
    NativeInputSettingsTableCalls& containers;
    const bool& crt_sse2_conversion;
    const volatile float& one_00d7a24c;
    const volatile float& base_zero_replacement_00cf7fe8;
    std::uint32_t descriptor_flag_stack_preimage;
    std::uint32_t vector_opaque_stack_preimage;
};

// Complete normal006A7BE0 schedule: existing raw script prefix, native settings
// containers, real tracked Lua objects, KeyboardSetup/InputNames/Conflicts,
// DEVINPUTS and same-temporary-state ControllerInputNames, then reverse cleanup.
// Original ECX settings, no stack arguments, RET. This is a new explicit-service
// C++ ABI. Required library providers are not supplied or replaced by defaults.
void load_native_input_settings_tables_006a7be0(void* actual_settings,
    NativeInputSettingsTableServices&);

// Actual settings must already contain constructed native headers and owner78.
// The byte4 guard survives failures; byte5 is not changed. Full constructor/
// singleton/destructor wiring, malformed native storage, original FH3/private
// stack aliases, hardware-fault behavior and gameplay remain outside this API.
} // namespace bsp
