#pragma once

#include "bsp/native_input_action_owner.hpp"
#include <cstdint>

namespace bsp {

// Required source adapter for the recognized signed-int -> float raw12h map
// subscript library contract. Native4D6900 takes ECX=actual header, a pointer
// to the signed key in one stack word, returns the writable float cell in EAX,
// and RET4. It searches/inserts into that same header; an absent key receives
// float+0 before return. Insertion, validation and their failures must be real.
// It also serves game+5C8; do not hard-code E18A7C or substitute std::map state.
class NativeInputActionDeadlineCalls {
public:
    virtual ~NativeInputActionDeadlineCalls() = default;
    virtual float* call_004d6900(void* actual_map_header,
        const std::int32_t* key) = 0;
};

struct NativeInputActionDeadlineContext {
    NativeInputActionOwnerContext& actions;
    void* const map_header_00e18a7c;
    const volatile double& delay_00ce65d0;
    NativeInputActionDeadlineCalls& calls;
};

// 4D8CD0: ECX=actual game, no stack inputs, plain RET. The borrowed game has
// its actual float clock at+64C; each 4BEC00 call uses the real current action
// owner and its+4 raw30h record array. Sixteen fixed indices, without count or
// null guards. Required valid storage includes those indices. Each reached
// deadline is rounded before subscript and stored through its returned cell.
// Provider failures propagate with earlier writes intact; no cleanup is added.
void record_native_input_action_deadlines_004d8cd0(void* actual_game,
    NativeInputActionDeadlineContext&);

// 6965A0: no native arguments. Captures actual E188A8 once, then tail-jumps
// 4D8CD0 with that receiver. No null-publication guard. Original bytes span
// 6965A0..6965AA, exclusive6965AB; Ghidra start was absent during recovery.
void invoke_native_input_action_deadline_callback_006965a0(
    void* volatile& game_publication_00e188a8,
    NativeInputActionDeadlineContext&);

// New C++ source ABI, complete within the actual-storage/provider domain.
// This does not construct game/action/map owners or provide map insertion.
// No original ABI, hardware-fault, asynchronous mutation or gameplay claim.
} // namespace bsp
