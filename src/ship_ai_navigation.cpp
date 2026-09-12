// The navigation arm of 009ED6B0 and the 84-byte record the ship AI publishes
// into the unit. Every routine carries its native address, ABI and coverage in
// bsp/ship_ai_navigation.hpp, docs/SHIP_AI_NAVIGATION_ARM.md and
// docs/UNIT_AI_ORDER_SLOT_READER.md. Names are hypotheses, not recovered
// symbols.

#include "bsp/ship_ai_navigation.hpp"

#include <array>
#include <cmath>

#include "bsp/geometry_helpers.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/vector_helpers.hpp"

namespace bsp {
namespace {

// 00D7A264, the image's float pi. 009EE8DC pushes it into 00438AA0.
constexpr float kNavPi = 3.1415927410125732421875f;

// 00415510: __fastcall(const float* a, const float* b) -> float, body
// 00415510-0041554E. ST0 = b, ST1 = a, FCOMIP b against a, JBE returns b. An
// unordered compare sets CF and takes the JBE, so a NaN in either operand
// yields b.
float native_min_00415510(float a, float b) noexcept {
    return (b <= a) ? b : a;
}

// 00415550, the mirror: ST0 = a, ST1 = b, FCOMIP a against b, JBE returns b.
float native_max_00415550(float a, float b) noexcept {
    return (a <= b) ? b : a;
}

// 00415620: __fastcall(const float* a, const float* b, const float* c) -> float,
// RET 4, body 00415620-0041565C. FCOMI b against a with JA returning b, then
// FCOMI a against c with JBE returning a. That is clamp(a, low = b, high = c)
// with both unordered paths landing on the first operand of their compare.
float native_clamp_00415620(float a, float low, float high) noexcept {
    if (low > a) {        // 00415635/00415637: FCOMI b,a; JA
        return low;
    }
    if (a <= high) {      // 0041564B/0041564D: FCOMI a,c; JBE
        return a;
    }
    return high;
}

}  // namespace

// ---------------------------------------------------------------------------
// The record and its two addresses
// ---------------------------------------------------------------------------

std::uint32_t unit_ai_order_current_offset(int index) noexcept {
    // 009F4D2F..009F4D3B and 00825F38/00825F6E: IMUL 54h, SUB, ADD 0AECh.
    return static_cast<std::uint32_t>(kUnitAiOrderSlot0) -
           static_cast<std::uint32_t>(kUnitAiOrderRecordSize) * static_cast<std::uint32_t>(index);
}

std::uint32_t unit_ai_order_published_offset(int index) noexcept {
    // 00825F64/00825F67 and 009D8D32/009D8D44: IMUL 54h, LEA +0A98h.
    return kUnitAiOrderPublishedSlot0 +
           static_cast<std::uint32_t>(kUnitAiOrderRecordSize) * static_cast<std::uint32_t>(index);
}

void unit_ai_order_copy_00811d10(UnitAiOrderRecord& dst, const UnitAiOrderRecord& src) noexcept {
    // 00811D14..00811D71, sixteen copies over +00h..+3Fh and nothing past it.
    dst.blend_00 = src.blend_00; // 00811D14
    dst.timer_04 = src.timer_04; // 00811D18
    dst.sub_a = src.sub_a;       // 00811D1E..00811D46
    dst.sub_b = src.sub_b;       // 00811D49..00811D71
    // +40h, +44h, +48h, +4Ch and +50h are deliberately not copied. The routine
    // returns at 00811D74 with RET 4.
}

void unit_ai_order_slot_step_0080e000(UnitAiOrderRecord& record, float seconds) noexcept {
    // 0080E011/0080E019: COMISS timer, 0 with JC skipping the countdown, so an
    // ordered timer at or above zero counts down and a negative or NaN timer
    // does not.
    if (!(record.timer_04 < 0.0f)) {
        const float next = record.timer_04 - seconds; // 0080E01E, one float store
        record.timer_04 = next;                       // 0080E028
        // 0080E02D/0080E031: FCOMI 0 against the new timer, JBE skips. The
        // reset therefore runs only on the step that drives it below zero.
        if (next < 0.0f) {
            record.sub_a = UnitAiOrderSubRecord{}; // 0080E03D..0080E04F
            record.sub_b = UnitAiOrderSubRecord{}; // 0080E052..0080E064
            record.sub_a.field_08 = kUnitAiOrderSubRecordReset; // 0080E042
            record.sub_b.field_08 = kUnitAiOrderSubRecordReset; // 0080E057
        }
    }

    // 0080E069: FMUL against the double 50.0 at 00CE3938, then one float store.
    const float step = static_cast<float>(static_cast<double>(seconds) * kUnitAiOrderBlendRate);
    const float target = record.sub_a.value_00; // 0080E072
    const float current = record.blend_00;      // 0080E079
    const float diff = target - current;        // 0080E089, stored at 0080E08D
    // 0080E095/0080E099: FCOMI diff against zero, JC takes the negative arm.
    if (!(diff < 0.0f)) {
        // 0080E0A0/0080E0A2: FCOMIP diff against step, JBE snaps.
        record.blend_00 = (diff <= step) ? target : (current + step); // 0080E0A4, 0080E0B6
    } else {
        // 0080E0C4 negates the difference before the same compare.
        const float magnitude = -diff;
        record.blend_00 = (magnitude <= step) ? target : (current - step); // 0080E0CF, 0080E0E1
    }
}

// ---------------------------------------------------------------------------
// The turn lead, 009EE848..009EE9D8
// ---------------------------------------------------------------------------

ShipAiTurnLead ship_ai_turn_lead_009ee848(float bearing, float distance, float unit_heading,
                                          const ShipAiNavState& nav,
                                          ShipAiThrottleDirection direction) noexcept {
    // 009EE856: the longest path length seen, times the double 1.5 at 00CE3D78,
    // rounded once into the stack slot at 009EE869.
    const float scaled_path =
        static_cast<float>(static_cast<double>(nav.longest_path_1f0) * kShipAiNavPathLengthScale);
    // 009EE87B: min(1000.0f, that). 009EE877 stored the distance as a double, so
    // 009EE880's FDIVR divides the double by the x87 value of the minimum.
    const float denominator = native_min_00415510(kShipAiNavTurnLeadCap, scaled_path);
    const float ratio = static_cast<float>(static_cast<double>(distance) /
                                           static_cast<double>(denominator)); // 009EE891
    // 009EE895: clamp(ratio, 0.0f, 1.0f); 009EE89A: FLD1/FSUBRP.
    const float clamped = native_clamp_00415620(ratio, 0.0f, 1.0f);
    const float taper = static_cast<float>(1.0 - static_cast<double>(clamped)); // 009EE8A9

    // 009EE8B1..009EE8C3, one 80-bit chain with a single float store:
    // (pi/2 - blk+3D0h) * taper * taper.
    const double window = static_cast<double>(nav.turn_window_3d0);
    const float lead = static_cast<float>((kShipAiNavQuarterTurn - window) *
                                          (static_cast<double>(taper) * static_cast<double>(taper)));

    // 009EE8C7: the unit's own heading through vtable slot 50h; 009EE8EA flips
    // it by the image's pi while the latched direction is astern.
    float heading = unit_heading;
    if (direction == ShipAiThrottleDirection::Astern) { // 009EE8C9
        heading = wrapped_angle_add_00438aa0(heading, kNavPi);
    }
    float delta = wrapped_angle_subtract_00438b10(heading, bearing); // 009EE907

    // 009EE916..009EE936: the path point one-sides the error. An unordered
    // compare takes the JBE on both arms and leaves the error alone.
    if (nav.side_304 == ShipAiNavTurnSide::ClampNonNegative) {
        if (delta < 0.0f) {
            delta = 0.0f;
        }
    } else if (nav.side_304 == ShipAiNavTurnSide::ClampNonPositive) {
        if (delta > 0.0f) {
            delta = 0.0f;
        }
    }

    // 009EE95E/009EE962/009EE964: |error| against blk+3D0h, JBE leaves the
    // bearing untouched.
    if (!(std::fabs(delta) > nav.turn_window_3d0)) {
        return ShipAiTurnLead{};
    }
    ShipAiTurnLead out{};
    out.applied = true;
    // 009EE96A: COMISS error, 0 with JBE taking the second arm.
    if (delta > 0.0f) {
        out.correction = native_min_00415510(delta - nav.turn_window_3d0, lead); // 009EE96F
    } else {
        out.correction = native_max_00415550(nav.turn_window_3d0 + delta, -lead); // 009EE9AA
    }
    return out;
}

// ---------------------------------------------------------------------------
// The output block, 009EE671..009EEAA2
// ---------------------------------------------------------------------------

ShipAiNavResult ship_ai_navigation_arm_009ee671(ShipAiControlBlock& blk,
                                                ShipAiNavState& nav,
                                                const ShipAiNavWaypoint& waypoint,
                                                const ShipAiNavPose& pose,
                                                ShipAiNavHost& host) {
    ShipAiNavResult result{};

    // 009EE67A..009EE69D: the goal is republished on the block every frame, and
    // the middle component is always stored as zero.
    nav.goal_valid_0a90 = true;
    nav.goal_x_0a94 = waypoint.x;
    nav.goal_y_0a98 = 0.0f;
    nav.goal_z_0a9c = waypoint.z;

    // 009EE6AC: the remaining path length, and 009EE6B5..009EE6CB keeps the
    // largest one the block has seen. FCOMPI/JBE, so an unordered compare does
    // not raise it.
    const float path_length = host.remaining_path_length_009d9e50();
    if (path_length > nav.longest_path_1f0) {
        nav.longest_path_1f0 = path_length;
    }

    // 009EE6F1..009EE712: the planar offset from the unit to the path point.
    const std::array<float, 2> to_waypoint{waypoint.x - pose.x, waypoint.z - pose.z};

    nav.side_304 = waypoint.side;                              // 009EE765
    blk.distance_32c = length_2d_00414c60(to_waypoint);        // 009EE76B, 009EE770
    nav.last_leg_338 = !waypoint.more_path;                    // 009EE776..009EE783

    if (!waypoint.more_path) {
        // 009EE78B..009EE7E4: the next leg's bearing, but only when that leg is
        // longer than the double 100.0 at 00D7A220 in squared length.
        const float ex = waypoint.next_x - waypoint.x;
        const float ez = waypoint.next_z - waypoint.z;
        const float length2 = ex * ex + ez * ez; // 009EE7B7..009EE7BF, one float store
        if (static_cast<double>(length2) > kShipAiNavNextLegMinLength2) {
            nav.next_leg_heading_334 = heading_angle_00414eb0(std::array<float, 2>{ex, ez});
        } else {
            nav.last_leg_338 = false; // 009EE7E4
        }
    }

    // 009EE7EB..009EE809: the bearing is only taken while the unit is far
    // enough out and the path point allows steering.
    if (static_cast<double>(blk.distance_32c) > kShipAiNavBearingDeadzone &&
        waypoint.steer_enabled) {
        result.bearing_taken = true;
        blk.heading_target_324 = heading_angle_00414eb0(to_waypoint); // 009EE813, 009EE818
        // 009EE83A/009EE842: the turn lead only runs inside 1000.0.
        if (static_cast<double>(blk.distance_32c) < kShipAiNavTurnLeadRange) {
            const ShipAiTurnLead lead =
                ship_ai_turn_lead_009ee848(blk.heading_target_324, blk.distance_32c,
                                           host.unit_heading_vtable_0050(), nav, blk.direction);
            if (lead.applied) {
                result.turn_lead_applied = true;
                blk.heading_target_324 = wrapped_angle_add_00438aa0(blk.heading_target_324,
                                                                    lead.correction); // 009EE9C7
                nav.turn_lead_328 = lead.correction;                                  // 009EE9D8
            }
        }
    }

    blk.distance_330 = path_length; // 009EEA00

    // 009EEA08..009EEAA2: the look-ahead radius shrinks with the cosine of the
    // angle between the heading target and the block's own axis vector, and is
    // capped by blk+3C8h.
    if (blk.distance_32c < nav.look_ahead_340 + nav.look_ahead_340) {
        const float axis_bearing = heading_angle_00414eb0(nav.hull_axis_19c); // 009EEA1A
        const float offset =
            wrapped_angle_subtract_00438b10(blk.heading_target_324, axis_bearing); // 009EEA37
        // 009EEA44: FCOS on the float-rounded offset, one float store after it.
        const float cosine = static_cast<float>(std::cos(static_cast<double>(offset)));
        const float floored = native_max_00415550(kShipAiNavCosineFloor, std::fabs(cosine));
        // 009EEA89/009EEA95: the doubled floor divides the distance, which
        // 009EEA5C had stored as a double.
        const float radius = static_cast<float>(static_cast<double>(blk.distance_32c) /
                                                (static_cast<double>(floored) +
                                                 static_cast<double>(floored)));
        nav.look_ahead_340 = native_min_00415510(nav.look_ahead_max_3c8, radius); // 009EEA9D
    }

    result.distance_to_waypoint = blk.distance_32c;
    result.heading_target = blk.heading_target_324;
    result.path_length = blk.distance_330;
    return result;
}

}  // namespace bsp
