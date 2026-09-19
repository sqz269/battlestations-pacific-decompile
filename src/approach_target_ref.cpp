#include "bsp/approach_target_ref.hpp"

#include <cmath>

// Packet cc8_hull_aim_point. See include/bsp/approach_target_ref.hpp for the
// addresses, the ABI and what is proved versus substituted.

namespace bsp {
namespace {

// 009FB2E7: the first timer value is drawn over [0, 2.0).
constexpr float kFirstTimerLo = 0.0f;

}  // namespace

ApproachTargetRefState approach_target_ref_construct_009fb200(
    float first_timer_draw) noexcept {
    ApproachTargetRefState state;
    // 009FB276/7B/80.
    state.spread_48 = {kApproachTargetRefSpreadX, kApproachTargetRefSpreadY,
                       kApproachTargetRefSpreadZ};
    // 009FB285-009FB2AA then 009FB2AF-009FB2D5: the offset, the bias and the
    // centre are all seeded from 00F87574/78/7C, copied twice. That address is
    // .data past raw size, so on disk it is zero; a runtime writer would show
    // up as a non-zero seed here.
    state.body_offset_28 = {0.0f, 0.0f, 0.0f};
    state.bias_34 = {0.0f, 0.0f, 0.0f};
    state.centre_54 = {0.0f, 0.0f, 0.0f};
    // 009FB25F.
    // sub+44h is 0.0; the tail it gates is not modelled.
    state.repick_timer_60 = first_timer_draw;  // 009FB2E7, Uniform(0, 2.0)
    state.section_chance_64 = kApproachTargetRefSectionChance;  // 009FB2F9
    state.weight_68 = kApproachTargetRefSectionWeight;          // 009FB2FE
    state.weight_6c = kApproachTargetRefSectionWeight;          // 009FB306
    state.weight_70 = kApproachTargetRefSectionWeight;          // 009FB30B
    state.dirty_41 = true;   // 009FB272
    state.tracking = true;
    state.frozen = false;
    (void)kFirstTimerLo;
    return state;
}

std::array<float, 3> approach_target_ref_pick_009fa260(
    ApproachTargetRefState& state, const LeadAimHullExtents& hull,
    const ShipLeadRandomDraws& draws, bool target_is_hull_sampler) noexcept {
    // 009FA266/009FA26B: with no target the body only clears the dirty byte.
    if (!state.tracking) {
        state.dirty_41 = false;  // 009FA2CE
        return state.body_offset_28;
    }

    std::array<float, 3> pick{{0.0f, 0.0f, 0.0f}};
    if (target_is_hull_sampler) {
        // 009FA2A0 CALL EAX with slot +100h == 00816650.
        //
        // The named-section path is unreachable under this seed
        // (section_chance = -1.0 fails 00816659), so the three section records
        // are passed empty and unavailable: nothing here can read them. They
        // stay in the call so the shape of the original is visible.
        const ShipLeadSections sections{};
        pick = ship_lead_point_00816650(
            sections, hull, state.spread_48, state.centre_54,
            state.section_chance_64, state.weight_68, state.weight_6c,
            state.weight_70, draws, false, false, false);
    } else {
        // slot +100h == 0042D810: the origin, whatever the arguments are.
        pick = entity_lead_point_0042d810();
    }

    // 009FA2A2-009FA2B0 stores the pick, then 009FA2B3-009FA2CB adds the bias.
    state.body_offset_28[0] = pick[0] + state.bias_34[0];
    state.body_offset_28[1] = pick[1] + state.bias_34[1];
    state.body_offset_28[2] = pick[2] + state.bias_34[2];
    state.dirty_41 = false;  // 009FA2CE
    return state.body_offset_28;
}

bool approach_target_ref_needs_pick_009fada0(ApproachTargetRefState& state,
                                             float dt, float rearm_draw,
                                             bool validity_still_holds) noexcept {
    // 009FAE1C: a frozen or untargeted sub-object does nothing further.
    if (!state.tracking) return false;

    bool pick = false;
    // 009FAE26-009FAE35: the timer counts UP by dt and is stored back.
    state.repick_timer_60 += dt;
    // 009FAE38-009FAE44 against 00CE3958 = 2.0.
    if (state.repick_timer_60 > kApproachTargetRefRepickPeriod) {
        // 009FAE61: re-armed to Uniform(-0.5, 0.5), so the next expiry is 1.5
        // to 2.5 s away.
        state.repick_timer_60 = rearm_draw;
        // 009FAE69-009FAEA3: slot +104h is handed the pick with the bias taken
        // back off, by value. 009FAEA5 JNZ skips the re-pick when it answers
        // true. Every class that samples a hull carries 0042BB20 there, which
        // always answers true, so this branch does not fire for them.
        if (!validity_still_holds) pick = true;  // 009FAEAB
    }
    // 009FAEB0-009FAEB8: the dirty byte forces a pick regardless.
    if (state.dirty_41) pick = true;
    return pick;
}

void approach_target_ref_store_world_point_009faeea(
    ApproachTargetRefState& state, const CameraMatrix& matrix) noexcept {
    // 009FAEDF CALL 004142E0, then the three stores at 009FAEEA/F5/00.
    std::array<float, 3> world{};
    transform_point_004142e0(state.body_offset_28, matrix, world);
    state.world_point_1c = world;
}

void approach_target_ref_freeze_009fadae(ApproachTargetRefState& state,
                                         const CameraMatrix& matrix) noexcept {
    // 009FADAE-009FAE18: one last sample through the same transform, then
    // sub+14h, sub+18h and the dirty byte are cleared, which freezes the point.
    approach_target_ref_store_world_point_009faeea(state, matrix);
    state.tracking = false;   // 009FAE11
    state.dirty_41 = false;   // 009FAE18
    state.frozen = true;
}

// ---------------------------------------------------------------------------
// The deterministic draw substitution
// ---------------------------------------------------------------------------
std::array<float, 4> approach_target_ref_unit_draws_substitute(
    std::uint32_t seed) noexcept {
    // LABELLED SUBSTITUTION for 00BD2F10 on stream ECX=1. A counter-based
    // integer hash, so the same approach gets the same hull point on every run
    // and the per-round census lines stay comparable. Not the image's sequence.
    std::array<float, 4> out{};
    for (std::uint32_t i = 0; i < 4U; ++i) {
        std::uint32_t h = seed * 0x9E3779B1U + i * 0x85EBCA6BU;
        h ^= h >> 16;
        h *= 0x7FEB352DU;
        h ^= h >> 15;
        h *= 0x846CA68BU;
        h ^= h >> 16;
        // [0,1) from the top 24 bits.
        out[i] = static_cast<float>(h >> 8) * (1.0f / 16777216.0f);
    }
    return out;
}

ShipLeadRandomDraws approach_target_ref_draws_from_unit(
    const std::array<float, 4>& unit,
    const std::array<float, 3>& spread) noexcept {
    ShipLeadRandomDraws draws;
    // The section path is dead under this seed, so the roll and the pick only
    // have to be well formed. 00816883/008168A0/008168D5 give the three ranges:
    // x and z are two sided over the spread, y is one sided from zero.
    draws.section_roll = unit[3];
    draws.pick = 0.0f;
    draws.box_x = (unit[0] * 2.0f - 1.0f) * spread[0];
    draws.box_y = unit[1] * spread[1];
    draws.box_z = (unit[2] * 2.0f - 1.0f) * spread[2];
    return draws;
}

}  // namespace bsp
