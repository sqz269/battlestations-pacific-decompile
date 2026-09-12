#include "bsp/unit_death_sink.hpp"

// docs/UNIT_DEATH_MESSAGE_AND_SINK.md, reports/unit_death_sink.json.
// Read-only analysis; descriptive names are hypotheses.

namespace bsp {

bool death_message_kills_00814560(float kamikaze_damage,
                                  float kamikaze_blast_damage) noexcept {
    // 00814574 COMISS [desc+510h], 0 / JA 00814591, then the same on +514h with
    // JBE to the return. Both tests are strict, so a class that leaves both
    // fields at the loader default 0.0f returns without touching the kill list.
    return kamikaze_damage > 0.0f || kamikaze_blast_damage > 0.0f;
}

bool sends_death_message_00827ada(float class_breakup_selector) noexcept {
    // 00827ACA FLD [desc+B0h]; 00827AD0 FLD double 100.0; FCOMIP ST0,ST1 compares
    // 100.0 against the field and 00827ADA JBE takes the breakup branch, so the
    // 9Ah message goes out when the field is below 100.0.
    return static_cast<double>(class_breakup_selector) < kBreakupSelectorThreshold;
}

bool sink_refused_008110f0(bool released, float invincibility) noexcept {
    // 008110F3 CMP byte [unit+5Dh],0 / JNZ, then COMISS [unit+150h], 0.0f / JA.
    return released || invincibility > 0.0f;
}

WreckSettleState wreck_settle_00824fe5(const std::array<float, 3>& hull_inertia) noexcept {
    // 00825006..00825044 read from the instruction bytes: DC C9 is
    // ST1 = I.y * 2.0, D8 C9 is I.z * 2.0 and D8 08 is 2.0 * I.x, stored in
    // component order at [ESP+4Ch], [ESP+50h], [ESP+54h].
    WreckSettleState state{};
    state.inertia[0] = hull_inertia[0] * kWreckInertiaScale;
    state.inertia[1] = hull_inertia[1] * kWreckInertiaScale;
    state.inertia[2] = hull_inertia[2] * kWreckInertiaScale;
    state.angular_damping = kWreckAngularDamping;
    state.linear_damping = kWreckLinearDamping;
    state.sink_time = 0.0f;
    state.sink_time_companion = 0.0f;
    return state;
}

WreckAnchorRanges wreck_anchor_ranges_008250f0(const ShipHullExtents& extents) noexcept {
    // 0082510D..008251BC. The lateral pair is symmetric about zero and divided
    // by the double 2.0 at 00CE3DE0; the vertical low bound is FLDZ and the high
    // bound is the height over the double 3.0 at 00D7A2B0.
    const double half_x = static_cast<double>(extents.width) / kWreckAnchorLateralDivisor;
    const double half_z = static_cast<double>(extents.length) / kWreckAnchorLateralDivisor;
    const double top_y = static_cast<double>(extents.height) / kWreckAnchorVerticalDivisor;

    WreckAnchorRanges ranges{};
    ranges.x_lo = static_cast<float>(-half_x);
    ranges.x_hi = static_cast<float>(half_x);
    ranges.y_lo = 0.0f;
    ranges.y_hi = static_cast<float>(top_y);
    ranges.z_lo = static_cast<float>(-half_z);
    ranges.z_hi = static_cast<float>(half_z);
    return ranges;
}

std::array<float, 3> wreck_anchor_point(const WreckAnchorRanges& ranges,
                                        const std::array<float, 3>& unit_samples) noexcept {
    // Each slot is zeroed at 008250F9..00825103 and then incremented once by the
    // draw, so the stored value is the draw itself. 00BD2F10's own mapping from
    // (lo, hi) to a value is a contract; this models it as a linear blend.
    std::array<float, 3> point{};
    point[0] = ranges.x_lo + (ranges.x_hi - ranges.x_lo) * unit_samples[0];
    point[1] = ranges.y_lo + (ranges.y_hi - ranges.y_lo) * unit_samples[1];
    point[2] = ranges.z_lo + (ranges.z_hi - ranges.z_lo) * unit_samples[2];
    return point;
}

MemberMessagePlan member_message_plan_00780090(const MemberMessageFields& fields) noexcept {
    // 00780095, 007800BD and 007800D7. The first arm returns after its dispatch,
    // so it excludes the other two; the second falls through into the third.
    MemberMessagePlan plan{};
    if (fields.flag_20h) {
        plan.call_slot_144h = true;
        return plan;
    }
    plan.call_slot_148h = fields.flag_30h;
    plan.call_slot_150h = true;
    plan.slot_150h_second_arg = (fields.value_2Ch == 1) ? fields.arg_28h : 8u;
    return plan;
}

void on_death_message_00814560(UnitDeathMessageHost& host, const DyingUnit& unit) {
    // 00814563/00814569: the parts object first, unconditionally.
    host.detach_all_live_parts(unit.parts_object);

    if (!death_message_kills_00814560(unit.kamikaze_damage, unit.kamikaze_blast_damage)) {
        return;  // 008145A6
    }

    // 00814595: the only path by which a damage death reaches the kill list, and
    // so the only path by which it reaches on-killed slot 80h.
    host.kill_entity(unit.unit, kUnitDestroyCauseQueued);
    // 008145A1, a tail jump.
    host.clear_hull_shape_fields(unit.motion_controller);
}

void on_breakup_message_00814520(UnitDeathMessageHost& host, const DyingUnit& unit) {
    // 00814529..00814547.
    if (host.class_has_breakup_pieces(unit.class_descriptor)) {
        const EntityRef manager = host.wreck_manager();
        if (manager != 0) {
            host.register_wreck(manager, unit.unit);
        }
    }
    // 00814553, a tail jump; it runs whether or not the registration happened.
    host.breakup_parts(unit.motion_controller);
}

void on_wrecked_00824fe5(WreckPhysicsHost& host,
                         const WreckedUnit& wreck,
                         const WreckBubbleSettings& settings) {
    host.reset_leak_manager(wreck.unit);

    const WreckSettleState settled = wreck_settle_00824fe5(host.read_body_inertia(wreck.hull_body));
    host.set_body_inertia(wreck.hull_body, settled.inertia);
    host.set_angular_damping(wreck.hull_body, settled.angular_damping);
    host.set_linear_damping(wreck.hull_body, settled.linear_damping);
    host.store_sink_fields(wreck.unit, settled.sink_time, settled.sink_time_companion);
    host.release_attached_effects(wreck.unit);

    // 00825093: the rest is the wreck's effect state and runs only for a queued
    // destroy. A unit whose cause is anything else keeps the physics change and
    // gets no bubble timer and no anchors.
    if (wreck.destroy_cause != kUnitDestroyCauseQueued) {
        return;
    }

    // 008250BD: the seed order is (cfg+64Ch, cfg+650h). The reload in
    // BSP_UnitInstance_Update passes the same two floats the other way round;
    // 00BD2F10's body settles which one is the low bound and is unread.
    host.store_bubble_timer(wreck.unit, host.random_range(settings.low, settings.high));

    if (!wreck.anchors_enabled) {
        return;  // 008250DD
    }

    // 008250F0..008251CD: five slots, drawn in the order z, x, y.
    const WreckAnchorRanges ranges = wreck_anchor_ranges_008250f0(wreck.extents);
    for (int index = 0; index < kWreckAnchorCount; ++index) {
        std::array<float, 3> point{};
        point[2] = host.random_range(ranges.z_lo, ranges.z_hi);
        point[0] = host.random_range(ranges.x_lo, ranges.x_hi);
        point[1] = host.random_range(ranges.y_lo, ranges.y_hi);
        host.store_anchor_point(wreck.unit, index, point);
    }
}

}  // namespace bsp
