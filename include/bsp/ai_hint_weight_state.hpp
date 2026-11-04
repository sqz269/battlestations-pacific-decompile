#pragma once

#include <array>
#include <cstdint>
#include <map>

namespace bsp {

// Source ownership for F8A740 and F8A750+i*0Ch, not VC9 tree/node layouts.
// Keys are this executable's existing Lua Ptr entity handles (index+1 for
// units). Both the script producer and planner reader use that same identity.
// The image clears all nine maps at A32441, on coordinator creation; ordinary
// script writes then survive across party thinks. One mission runs per process.
struct AiHintWeightState {
    std::map<std::uint32_t, float> global;
    std::array<std::map<std::uint32_t, float>, 8> parties;
};

inline AiHintWeightState& process_ai_hint_weights() {
    static AiHintWeightState state;
    return state;
}

} // namespace bsp
