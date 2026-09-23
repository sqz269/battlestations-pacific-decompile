#include "bsp/dive_bomb_goaway_turn.hpp"

#include "bsp/dive_bomb_task.hpp"

namespace bsp {

// 009C4950-009C4A3C, the goaway enter. docs/DIVE_BOMB_GOAWAY_TURN.md section 2.
void dive_bomb_goaway_enter_009c4950(const DiveBombGoAwayEnterInputs& in,
                                     DiveBombGoAwayTurnState& state) noexcept {
    // 009C4950-009C497E: AND EAX,80000001h / JNS / DEC-OR-INC is the signed
    // remainder by 2; a zero remainder (JE at 009C4967) takes +1.0.
    state.side_18 = in.step_odd ? dive_bomb_goaway_turn::kSideOdd
                                : dive_bomb_goaway_turn::kSideEven;
    state.countdown_24 = dive_bomb_goaway_turn::kEnterCountdown;  // 009C498B
    // 009C4990-009C4999: FLD approach+B4h, FADD ST0,ST0, FSTP float.
    float standoff = in.attack_distance_b4 + in.attack_distance_b4;
    // 009C49B9-009C49E0: FLD extent, FLD +20h, FCOMIP, `76` JBE keeps the
    // extent unless +20h is strictly larger - the larger of the two.
    if (in.has_extent_target) {
        standoff = (standoff > in.target_extent) ? standoff : in.target_extent;
    }
    // 009C4A01-009C4A10: draw * +20h, rounded to float by the FSTP/FLD pair.
    standoff = in.standoff_jitter * standoff;
    // 009C4A16-009C4A2E: both flags set -> * 1.5 (qword), else the FST value.
    if (in.control_flag_369 && in.global_e17bf2) {
        standoff = static_cast<float>(
            static_cast<double>(standoff) *
            dive_bomb_goaway_turn::kOrderedStandoffScale);
    }
    state.standoff_20 = standoff;
}

// 009C4A6D-009C4ACD.
DiveBombGoAwayTimerReport dive_bomb_goaway_timers_009c4a6d(
    const DiveBombGoAwayTimerInputs& in, DiveBombGoAwayTurnState& state) noexcept {
    DiveBombGoAwayTimerReport out;
    // 009C4A6D-009C4A84: FLD approach+BCh, FLD +20h, FSUB qword 100.0, FCOMIP,
    // `76` JBE: count down only while the aircraft is more than 100 m inside
    // the standoff ring.
    if (static_cast<double>(state.standoff_20) - dive_bomb_goaway_turn::kRingSlack >
        static_cast<double>(in.planar_distance_bc)) {
        state.countdown_24 = state.countdown_24 - in.dt;  // 009C4A86-009C4A8B
        out.counted_down = true;
    }
    // 009C4A8E-009C4AA1: FADD +28h, FSTP float, FST +28h.
    state.clock_28 = in.dt + state.clock_28;
    // 009C4AA4 COMISS [00D7A24C], approach+C4h, `76` JBE: 1.0 > C4h only.
    if (dive_bomb_goaway_turn::kForceClockGate > in.approach_clock_c4) {
        // 009C4AAD-009C4ABC: +28h > +2Ch + 6.0 (qword), `76` JBE.
        if (static_cast<double>(state.clock_28) >
            static_cast<double>(state.window_2c) +
                dive_bomb_goaway_turn::kForceWindowPad) {
            state.countdown_24 = dive_bomb_goaway_turn::kForcedCountdown;
            out.forced = true;
        }
    }
    return out;
}

// 009C4CF1-009C4D9A.
bool dive_bomb_goaway_reroll_009c4cf1(const DiveBombGoAwayRerollInputs& in,
                                      DiveBombGoAwayTurnState& state) noexcept {
    // 009C4CF1 COMISS XMM0(=0), [ESI+24h]; `76` JBE out unless 0 > +24h.
    if (!(0.0f > state.countdown_24)) {
        return false;
    }
    state.window_2c = in.window_draw;  // 009C4D18
    // 009C4D38-009C4D3E: second draw FADD +2Ch, FSTP +24h.
    state.countdown_24 = in.countdown_draw + state.window_2c;
    // 009C4D54-009C4D8C: 00419010(100, 1.0, 400, 1.5, altitude), FMUL +24h.
    const float scale = dive_bomb_interpolate_clamped_00419010(
        dive_bomb_goaway_turn::kScaleLowAltitude, dive_bomb_goaway_turn::kScaleLow,
        dive_bomb_goaway_turn::kScaleHighAltitude, dive_bomb_goaway_turn::kScaleHigh,
        in.altitude);
    state.countdown_24 = scale * state.countdown_24;
    // 009C4D8F-009C4D9A: +28h = -(+2Ch * 0.5 qword). Exact at float width.
    state.clock_28 = static_cast<float>(
        -(static_cast<double>(state.window_2c) * dive_bomb_goaway_turn::kClockLeadIn));
    return true;
}

// 009C4D9D-009C4E1D.
DiveBombGoAwayTurnCommand dive_bomb_goaway_turn_split_009c4dad(
    const DiveBombGoAwayTurnState& state, float heading_1c) noexcept {
    DiveBombGoAwayTurnCommand out;
    // 009C4D9D-009C4DAD: FLD +28h (rounded through [ESP+18h]), FLD +2Ch, FCOMIP,
    // `76` JBE to the heading arm unless +2Ch is strictly larger.
    if (state.window_2c > state.clock_28) {
        out.bank_arm = true;
        // 009C4DAF-009C4DB4: FMUL +18h, FADD ST0,ST0, FSTP float.
        float bank = state.clock_28 * state.side_18;
        bank = bank + bank;
        // 009C4DBC-009C4DE5: low bound first (FCOMIP/JBE), then high (COMISS/JBE).
        if (dive_bomb_goaway_turn::kBankClampLo > bank) {
            bank = dive_bomb_goaway_turn::kBankClampLo;
        } else if (bank > dive_bomb_goaway_turn::kBankClampHi) {
            bank = dive_bomb_goaway_turn::kBankClampHi;
        }
        out.bank_target_2c4 = bank;   // 009C4DF6
        out.heading_mode_2cc = 1;     // 009C4DF0, EBX
        return out;
    }
    out.heading_2c0 = heading_1c;     // 009C4E0E-009C4E17
    out.heading_mode_2cc = 2;         // 009C4E1D
    return out;
}

}  // namespace bsp
