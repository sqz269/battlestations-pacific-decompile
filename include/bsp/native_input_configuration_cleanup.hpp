#pragma once
#include "bsp/native_input_action_records.hpp"
#include <cstdint>

namespace bsp {
// Required checked-STL storage operations over the caller's actual10h headers.
// No parallel storage or default successful implementation is supplied. These
// library boundaries must preserve their captured ranges and returning CRT
// diagnostics. Header+0 is not an action ID and is not cleared by this packet.
class NativeInputConfigurationStorageCalls {
public:
    virtual ~NativeInputConfigurationStorageCalls() = default;
    // Native ECX destination, stack source, EAX destination, RET4. Deep assign
    // checked DWORD vector {untouched,begin,end,capacity}; source self-alias is
    // a no-op. Allocation/copy/erase details remain a required library contract.
    virtual void* call_00697bd0(void* actual_destination, const void* actual_source) = 0;
    // Native ECX vector; stack output8h iterator, first-owner, first-position,
    // last-owner, last-position; EAX output, RET14h. Validate owner pair, move
    // the current suffix into the captured first position, update end, and
    // write the returned iterator {first-owner, first-position}.
    virtual void* call_006977f0(void* actual_vector, void* actual_iterator_out,
        const void* first_owner, void* first_position,
        const void* last_owner, void* last_position) = 0;
};

struct NativeInputConfigurationCleanupContext {
    NativeInputActionOwnerContext& action_owner;
    NativeInputActionRecordsContext& records;
    NativeInputConfigurationStorageCalls& storage;
};

// A92260: native ECX actual24h owner, stack uint32 index, EAX exactly0/1,
// RET4. Unsigned count check precedes reading actual30h registered byte00.
std::uint32_t query_native_input_action_registered_00a92260(
    const void* actual_owner, std::uint32_t action_index) noexcept;

// A93880: native ECX actual30h record, RET, no result. Resize context/binding
// counts to0, retaining those arrays; then release current listener2C and clear
// registered00. Enabled01 and the action's previous/current values stay as-is.
void clear_native_input_action_registration_00a93880(
    void* actual_action, NativeInputActionRecordsContext&);

// A93920: native ECX actual24h owner, stack uint32 index, RET4. Computes the
// selected30h address and invokes A93880; native performs no bounds check.
void unregister_native_input_action_00a93920(void* actual_owner,
    std::uint32_t action_index, NativeInputActionRecordsContext&);

// 698730: ECX actual embedded input configuration; no stack args/result, RET.
// Repeatedly resolves the real shared action publication through004BEC00;
// checks all indices0..128h and obtains the owner AGAIN before unregistering.
// Cleans the nested checked vector at+4D0, then flat checked DWORD vectors at
// +500,+510,+4E0,+4F0. The 698680 constructor and698A10 parser establish these
// offsets; this API does not construct, own, or project the surrounding Lua
// object. Outer/flat capacities and opaque header+0 words remain retained.
void clear_native_input_configuration_00698730(
    void* actual_configuration, NativeInputConfigurationCleanupContext&);

// Complete game-body schedules with explicit library providers, not complete
// application/configuration ownership or original register/FH3/SEH replacement.
// Required providers operate on valid native allocations. Their exceptions and
// real CRT exceptions preserve preceding writes; no additional rollback exists.
} // namespace bsp
