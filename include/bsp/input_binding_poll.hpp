#pragma once

#include "bsp/input_action_classifier.hpp"

#include <cstdint>
#include <vector>

namespace bsp {

struct InputActionRecord;

// The remaining device and CRT calls of 00a92370/00a91d60. These are explicit
// external calls, not replacement device state or default implementations.
// The slot names are deliberately neutral: their implementations are outside
// this packet. Each device call has native ECX=device, stack=input code, RET4.
// Predicate results are AL bytes; slot20 must not be normalized to bool because
// required modifiers test !=0 whereas forbidden modifiers test ==1.
struct InputBindingPollHost {
    virtual ~InputBindingPollHost() = default;
    virtual std::uint8_t device_query_1c(InputDevice&, std::uint32_t code) = 0;
    virtual std::uint8_t device_query_20(InputDevice&, std::uint32_t code) = 0;
    // Native ST0 result is stored to binary32 immediately by 00a92370.
    virtual float device_value_24(InputDevice&, std::uint32_t code) = 0;
    // Actual library 00bf7030 takes/returns ST0. The caller loads a binary32
    // sum of squares and immediately spills the returned root to binary32.
    // The host owns the CRT/FPU/error policy; no CRT implementation is ported.
    virtual float crt_sqrt_00bf7030(float sum_of_squares) = 0;
};

// ECX=30h action, no stack arguments, AL result, RET. Scans every nonnull
// primary cached pointer, including unresolved bindings, and stops at the first
// nonzero slot1C result. Modifier state and +14h flags are not consulted.
bool any_input_binding_query_00a92090(
    const std::vector<InputActionBinding>&, InputBindingPollHost&);

// Native stack=(float primary, float paired), ST0 result, RET8. Retains x87
// intermediates and binary32 spills; only the actual CRT call remains external.
// The positive result is capped at +1; negative results are not capped at -1.
float adjust_input_binding_pair_00a91d60(float primary, float paired,
    InputBindingPollHost&);

// Full 00a92370: ECX=30h action, no stack arguments, RET. Performs the existing
// begin_action_frame shift exactly once, then scans and evaluates its bindings.
// No enabled check occurs here (00a92c40 owns that guard). All referenced
// devices and vectors must stay valid/stable across the host calls. Resolved
// primary/modifier cached pointers must be nonnull, as in native code.
void poll_input_action_bindings_00a92370(InputActionRecord&, InputBindingPollHost&);

// ECX=input singleton, no stack arguments, RET. Visits all action records,
// including disabled records, reusing the existing 00a91e80 implementation.
void rebind_all_input_actions_00a922a0(std::vector<InputActionRecord>&,
    const InputBindingDeviceGroups&) noexcept;

} // namespace bsp
