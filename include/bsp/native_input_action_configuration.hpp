#pragma once
#include "bsp/native_input_action_binding_runtime.hpp"
#include "bsp/native_input_action_records.hpp"
#include "bsp/native_input_action_tick.hpp"
#include <cstdint>

namespace bsp {
// All services borrow the same actual action/binding/listener allocations.
// The timing cells and device services must remain valid through activation.
// No alternate action owner, settings representation, or default device exists.
struct NativeInputActionConfigurationContext {
    NativeInputActionRecordsContext& records;
    NativeInputActionBindingContext& bindings;
    NativeInputActionTimingGlobals timing;
};

// A92D40: native ECX actual30h action; stack actual12h owner-context header,
// DWORD enabled (low byte), DWORD active context value; RET0Ch, no result.
// A matching context or -1 enables and immediately polls the action. Disabled
// actions clear their values and actual listener flags/timers. The enabled arm
// captures the listener AFTER polling, updates it with delta1, then copies the
// current value/latch to previous using the original FLD/FSTP and byte order.
void activate_native_input_action_context_00a92d40(void* actual_action,
    const void* actual_context_header, std::uint8_t context_enabled,
    std::int32_t active_context, NativeInputActionConfigurationContext&);

// A93C80: native ECX actual24h owner; stack uint32 action index, actual12h
// context source header, DWORD replace-listener (low byte); RET0Ch, no result.
// Grows the actual30h array, sets registered, optionally replaces the actual24h
// listener, copies contexts, grows/zeros the owner's context words, then calls
// A92D40. Source contexts are reread after listener/storage callbacks; the action
// pointer is captured after the first resize and retained for the remainder.
void configure_native_input_action_00a93c80(void* actual_owner,
    std::uint32_t action_index, const void* actual_context_source,
    std::uint8_t replace_listener, NativeInputActionConfigurationContext&);

// New C++ interfaces, not original register/stack/FH3/SEH replacements. Valid
// native ranges and reached finite providers are required. Exceptions retain
// completed mutations. No extra rollback, reference acquisition or validation.
// This supplies the two record producers called by00698A10, not that routine's
// unreconstructed raw settings/Lua cleanup, parser, binding install, or tail.
} // namespace bsp
