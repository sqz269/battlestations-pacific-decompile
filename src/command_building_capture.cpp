// The CommandBuilding capture. Packet cc9_command_building_capture_bind; see
// bsp/command_building_capture.hpp and docs/SHIP_AI_OPEN_ITEMS.md sections 78
// and 80. Descriptive names are hypotheses.
#include "bsp/command_building_capture.hpp"

namespace bsp {

bool command_building_capture_countdown_006f75ed(CommandBuildingCaptureState& state,
                                                 float step) {
    // 006F755D: CMP [EDI-2BCh],2 (unit+54h); JE 006F75ED.
    if (state.party_54 != 2) return false;
    // 006F75ED..006F7601: FLD countdown, FLD step, FCOMI, JB 006F763C.
    if (step < state.countdown_7c0) {
        state.countdown_7c0 = state.countdown_7c0 - step;   // 006F763C FSUBP
        return false;
    }
    // 006F7603 FSUBR [+7BCh] (period - step), 006F760F FADDP (+ countdown).
    state.countdown_7c0 = (kCommandBuildingCapturePeriod - step) + state.countdown_7c0;
    return true;   // 006F7617 CALL 006F6760
}

std::int32_t command_building_add_capture_power_006f69d2(std::int32_t sum, float power,
                                                         float modifier) {
    // FLD [class+804h]; FMUL modifier; FIADD [sum]; CALL 00BF7420 (__ftol, truncation).
    const double value = static_cast<double>(power) * static_cast<double>(modifier)
        + static_cast<double>(sum);
    return static_cast<std::int32_t>(value);
}

CommandBuildingTickOutcome command_building_capture_tick_006f6760(
    CommandBuildingCaptureState& state, std::int32_t strength0, std::int32_t strength1,
    float fallback) {
    CommandBuildingTickOutcome out;
    const float old = state.progress_7a8;
    const std::int32_t s0 = strength0;
    const std::int32_t s1 = strength1;
    // local_174: 0 when s0 > s1, else (s1 <= s0) + 1.
    out.side = s1 < s0 ? 0 : (s1 <= s0 ? 2 : 1);
    float& p = state.progress_7a8;
    bool done = false;
    if (s0 > 0) {
        if (s1 > 0) {
            p = static_cast<float>(s0 - s1) + p;
            done = true;
        } else if (s1 == 0) {
            if (0.0f <= p) {
                p = static_cast<float>(s0) + p;
            } else {
                // Party 1's progress first decays by the fallback power.
                const float t = fallback + p;
                p = t;
                p = (0.0f <= t) ? static_cast<float>(s0) + 0.0f : static_cast<float>(s0) + t;
            }
            done = true;
        }
    }
    if (!done) {
        if (s0 == 0 && s1 > 0) {
            if (p <= 0.0f) {
                p = p - static_cast<float>(s1);
            } else {
                const float t = p - fallback;
                p = t;
                p = (t <= 0.0f) ? 0.0f - static_cast<float>(s1) : t - static_cast<float>(s1);
            }
        } else {
            // Nobody in range: decay toward 0 by the fallback power, clamped at 0.
            if (p <= 0.0f) {
                if (0.0f <= p) {
                    p = 0.0f;
                } else {
                    const float t = fallback + p;
                    p = (0.0f < t || t == 0.0f) ? 0.0f : t;
                }
            } else {
                const float t = p - fallback;
                p = (t < 0.0f || t == 0.0f) ? 0.0f : t;
            }
        }
    }
    // 006F6FC8: |new - old| (DAT_00D7A208 is -0.0f) > 0 and the building is
    // neutral (it always is here) -> D5h {progress, CaptureValue, side}.
    float delta = p - old;
    if (delta <= 0.0f) delta = -0.0f - delta;
    out.progress_changed = 0.0f < delta;
    float magnitude = p;
    if (magnitude <= 0.0f) magnitude = -0.0f - magnitude;
    if (magnitude < static_cast<float>(state.capture_value_7a4)) {
        state.side_7ac = out.side;
        return out;
    }
    out.completed = true;
    p = 0.0f;                      // 006F7207
    state.side_7ac = out.side;     // 006F732E
    return out;
}

bool command_building_neutralize_006f3270(CommandBuildingCaptureState& state,
                                          std::int32_t prior_slot_2d8) {
    // 006F327C: a neutral building returns (006F32A0 RET).
    if (state.party_54 == 2) return false;
    state.prior_party_7b0 = state.party_54;   // 006F3284
    state.prior_slot_7b4 = prior_slot_2d8;    // 006F3290
    state.progress_7a8 = 0.0f;                // 006F2962
    // 006F296C: D3h slot 9 -> 006F4D10. +7A8h = 0 again (006F4D3A), +528h
    // takes 9 the first time (006F4D48), and the slot-9 arm (006F5088..)
    // leaves +7B0h (already the old party) and sets the party to 2 (006F50CA).
    if (state.first_slot_528 == -1) state.first_slot_528 = 9;
    state.party_54 = 2;
    return true;
}

CommandBuildingFlipOutcome command_building_flip_006f4d10(CommandBuildingCaptureState& state,
    std::int32_t slot, std::int32_t slot_party, std::int32_t prior_slot_party) {
    CommandBuildingFlipOutcome out;
    state.progress_7a8 = 0.0f;                                  // 006F4D3A
    if (state.first_slot_528 == -1) state.first_slot_528 = slot;  // 006F4D48
    if (slot == 8 || slot == 9) {
        // 006F509F..006F50CA: SetPartyRace(2, race). (Slot 9's +7B0h record at
        // 006F5099 is game-mode 2 only.)
        out.new_party = 2;
        state.party_54 = 2;
        return out;
    }
    out.new_party = slot_party;
    state.party_54 = slot_party;                                // 006F4FB8 SetPartyRace
    // 006F4FBA..006F5017: the old owner retaking it skips the repair.
    const bool retake = prior_slot_party >= 0 && prior_slot_party == state.prior_party_7b0
        && state.prior_party_7b0 == slot_party;
    out.repair = state.prior_party_7b0 == 2 || !retake;
    return out;
}

}  // namespace bsp
