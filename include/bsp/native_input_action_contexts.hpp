#pragma once
#include "bsp/native_input_action_configuration.hpp"
#include <cstdint>

namespace bsp {
// A93020: ECX actual24h owner, no stack args/result, RET. Capture header+10
// once, but reload active DWORD+20 and enabled byte+1C for each30h action.
// Activation may mutate storage; the endpoint is recomputed after each call.
void refresh_native_input_action_contexts_00a93020(void* actual_owner,
    NativeInputActionConfigurationContext&);

// A933F0: ECX actual24h owner, two stack DWORDs(index,level), RET8. Write the
// indexed actual context word, recompute the UNSIGNED maximum with floor1,
// then refresh every action even if the selected level did not change.
// No bounds validation, context growth or replacement storage is supplied.
void set_native_input_context_level_00a933f0(void* actual_owner,
    std::uint32_t index, std::uint32_t level, NativeInputActionConfigurationContext&);

// New explicit-service C++ ABI. Valid native storage and actual providers are
// required; exceptions retain earlier writes. No original stack-alias/FH3/SEH,
// arbitrary asynchronous mutation or application/gameplay compatibility claim.
} // namespace bsp
