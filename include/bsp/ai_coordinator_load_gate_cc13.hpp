#pragma once

// Packet cc13-mp4: the AI coordinator's load gate, 004E17FD..004E1838 in
// 004DFB70 BSP_Game_LoadMissionScene, computed from the session context instead of
// the single-player constant it replaces (kAiCoordinatorCreatedAtLoad = false).
//
//   004E17FD  CMP [game+1FE4h], 1 / JE 004E180F     a hosted session
//   004E1806  CMP BYTE [game+61Ch], 0 / JE 004E183D  else the forced-mode byte
//   004E180F..004E1834  for i in 0..7: EAX = [game+18CCh + 4i] (no null test),
//             +8h, +9h and +0Ah all non-zero (the 004B5510 IsActiveAi predicate,
//             inlined) -> 004E1838 CALL 00A32350; none -> 004E183D, no coordinator.
//
// Campaign identity: a campaign run never writes bsp::session_context() (only a
// --skirmish run under kSkirmishSessionCc13Bound publishes one), so the context is
// campaign_session_context(): session word 0 and forced byte false. The first test
// then fails before any slot is read, and the answer is false for every campaign load,
// whatever the slot records hold. Names are hypotheses.

#include <array>
#include <cstddef>

#include "bsp/session_context_cc13.hpp"

namespace bsp {

// One [game+18CCh + 4i] record's three bytes. +0Ah is written with +8h and +9h by
// 004BB160 for the scene's mission records (004BB21C..004BB224) and rewritten per slot
// by 0076CB90 (the lobby's 005D5340); the caller reads all three from the host's
// participant pools (packet cc13-mp3).
struct AiCoordinatorLoadSlot {
    bool occupied_08{false};
    bool ai_09{false};
    bool active_0a{false};
};

inline bool ai_coordinator_created_at_load_004e17fd(
    const SessionContext& session,
    const std::array<AiCoordinatorLoadSlot, 8>& slots) noexcept {
    if (!(session.session_word_1fe4 == 1 || session.forced_61c)) return false;  // 004E17FD..004E180D
    for (const AiCoordinatorLoadSlot& slot : slots) {                            // 004E180F..004E1834
        if (slot.occupied_08 && slot.ai_09 && slot.active_0a) return true;       // 004E1838
    }
    return false;                                                                // 004E183D
}

}  // namespace bsp
