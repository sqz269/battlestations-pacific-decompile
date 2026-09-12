// 009EEAAB..009EF226, the tail of the ship AI's navigation arm.
// See include/bsp/ship_ai_navigation_arm_tail.hpp and
// docs/SHIP_AI_NAVIGATION_ARM_TAIL.md.  Every name is a hypothesis.
#include "bsp/ship_ai_navigation_arm_tail.hpp"

#include <cmath>

#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/vector_helpers.hpp"

namespace bsp {
namespace {

// 00415550 BSP_Math_MaxFloatByRef: one FCOMIP of `a` against `b` with JBE
// returning `b`, so an unordered compare (either input NaN) also returns `b`.
float max_by_ref_00415550(float a, float b) noexcept {
    return (a > b) ? a : b;
}

// 00415510 BSP_Math_MinFloatByRef: FCOMIP of `b` against `a` with JBE
// returning `b`, so an unordered compare returns `b` here too.
float min_by_ref_00415510(float a, float b) noexcept {
    return (b < a) ? b : a;
}

// The walk at 009EEB80..009EEEEC has no iteration bound of its own: it leaves
// only when a whole pass finds nothing blocking or when the accumulated
// setback reaches blk+330h.  This limit is a reconstruction guard and is NOT
// in the native routine; ShipAiTrafficSetback::steps reports what it saw.
constexpr int kShipAiArmTailWalkGuard = 1024;

// 009EF188 and 009EF18E, the doubles at 00CE3D28 and 00CE3D20.  The first is
// pi already rounded through a float, which is what the image stores.
constexpr double kShipAiArmTailPi = 3.1415927410125732;
constexpr double kShipAiArmTailHalfCircleDeg = 180.0;

}  // namespace

// ---------------------------------------------------------------------------
// 009D5240, the direction-latch setter every latch write in 009ED6B0 inlines.
// ---------------------------------------------------------------------------
bool ship_ai_set_direction_latch_009d5240(ShipAiControlBlock& blk,
                                          ShipAiThrottleDirection direction,
                                          bool force) noexcept {
    // 009D5244: already there, nothing happens.
    if (direction == blk.direction) {
        return false;
    }
    // 009D524F COMISS XMM0(0.0f),[ECX+360h] with JA taken when 0 > timer, so
    // the write needs a timer that has already run past zero, or `force`.
    const bool cooldown_expired = (0.0f > blk.timer_360);
    if (!cooldown_expired && !force) {
        return false;
    }
    blk.direction = direction;                          // 009D5269
    blk.direction_value_374 = kShipAiArmTailLatchMark;  // 009D526F, -1.0f
    blk.direction_counter_384 = 0;                      // 009D5277
    // 009D527F JZ: Stopped reloads the timer with zero, the other two with
    // 1.0f, so a stop can be undone on the next tick and a gear change cannot.
    blk.timer_360 =
        (direction == ShipAiThrottleDirection::Stopped) ? 0.0f : kShipAiArmTailCeilingCap;
    return true;
}

// ---------------------------------------------------------------------------
// 009EECDB..009EED50, the approach throttle ceiling.
// ---------------------------------------------------------------------------
float ship_ai_approach_ceiling_009eecdb(float remaining_330, float reference_speed_3c4,
                                        float class_deceleration_508) noexcept {
    // 009EED1A FDIV, 009EED37 FMULP, 009EED39 FMUL 0.5, then 009EED3F stores
    // the stopping distance as a float before 009EED43 divides by it.
    const double speed = static_cast<double>(reference_speed_3c4);
    const float stopping_distance = static_cast<float>(
        (speed / static_cast<double>(class_deceleration_508)) * speed * kShipAiArmTailHalf);
    const float ratio = static_cast<float>(static_cast<double>(remaining_330) /
                                           static_cast<double>(stopping_distance));
    // 009EED4B, 00415620(&ratio, &0.25f)(&1.0f).
    return clamp_float_by_ref_00415620(ratio, kShipAiArmTailCeilingFloor,
                                       kShipAiArmTailCeilingCap);
}

// ---------------------------------------------------------------------------
// 009EEC15..009EEC37, the clearance radius for one neighbour.
// ---------------------------------------------------------------------------
float ship_ai_traffic_clearance_009eec15(float allowance, float walked,
                                         float own_hull_radius,
                                         float other_hull_radius) noexcept {
    // 009EEBE0..009EEC07: (other + own) * 0.5, kept on the x87 stack.
    const float half_radii = static_cast<float>(
        (static_cast<double>(other_hull_radius) + static_cast<double>(own_hull_radius)) *
        kShipAiArmTailHalf);
    // 009EEC15..009EEC21: max(0.0f, allowance - walked).
    const float remaining_allowance = max_by_ref_00415550(0.0f, allowance - walked);
    // 009EEC32: max(60.0f, half_radii), then 009EEC37 adds the two.
    const float base = max_by_ref_00415550(kShipAiArmTailClearanceFloor, half_radii);
    return base + remaining_allowance;
}

// ---------------------------------------------------------------------------
// 009EED62..009EEEDC, one step of the walk back from the goal.
// ---------------------------------------------------------------------------
ShipAiSetbackStep ship_ai_traffic_setback_step_009eed62(float walked,
                                                        float point_x, float point_z,
                                                        float dir_x, float dir_z,
                                                        float other_x, float other_z,
                                                        float clearance) noexcept {
    // 009EED62..009EED96: project the neighbour onto the ray.
    const float rel_x = other_x - point_x;
    const float rel_z = other_z - point_z;
    const float along = rel_x * dir_x + rel_z * dir_z;

    // 009EED9E..009EEDAC: the walk moves against `dir`, so the projection is
    // subtracted rather than added.
    float out_walked = walked - along;

    // 009EEDB0..009EEDDC: the closest point on the ray.
    float proj_x = point_x + along * dir_x;
    float proj_z = point_z + along * dir_z;

    // 009EEE00, 00414C60 on the perpendicular offset.  Below 1e-10 squared it
    // answers +0.0f rather than calling the CRT square root.
    const std::array<float, 2> perp_vec{{proj_x - other_x, proj_z - other_z}};
    const float perp = length_2d_00414c60(perp_vec);

    // 009EEE11 FCOMI with JBE: only an ordered "perp < clearance" backs off.
    if (perp < clearance) {
        // 009EEE15..009EEE25, 00BF7030: the half chord inside the circle.
        const float back = static_cast<float>(
            std::sqrt(static_cast<double>(clearance) * static_cast<double>(clearance) -
                      static_cast<double>(perp) * static_cast<double>(perp)));
        out_walked += back;                 // 009EEE3C
        proj_x -= back * dir_x;             // 009EEE5C..009EEE67
        proj_z -= back * dir_z;             // 009EEE6B..009EEE76
    }

    // 009EEE80..009EEED4: one fixed 20.0 step further back.
    const float step = static_cast<float>(kShipAiArmTailWalkStep);
    ShipAiSetbackStep out;
    out.point_x = proj_x - step * dir_x;
    out.point_z = proj_z - step * dir_z;
    out.walked = out_walked + step;
    return out;
}

// ---------------------------------------------------------------------------
// 009EF0A8..009EF1D4, ahead or astern.
// ---------------------------------------------------------------------------
ShipAiThrottleDirection ship_ai_astern_choice_009ef0a8(float heading_error_abs,
                                                       float remaining_330,
                                                       float turn_radius,
                                                       float hull_radius,
                                                       ShipAiThrottleDirection latched) noexcept {
    const bool astern = (latched == ShipAiThrottleDirection::Astern);

    // 009EF112..009EF13B: max(hull * 3.0, turn_radius * 2).
    const float hull_term =
        static_cast<float>(static_cast<double>(hull_radius) * kShipAiArmTailHullRadiusScale);
    float threshold = max_by_ref_00415550(hull_term, turn_radius + turn_radius);

    // 009EF14A..009EF159: an already-astern ship keeps 80.0 more before it is
    // allowed to call itself far from the goal again.
    if (astern) {
        threshold = static_cast<float>(static_cast<double>(threshold) +
                                       kShipAiArmTailAsternHysteresis);
    }

    // 009EF167 FCOMIP with JBE: far enough to drive ahead.
    if (!(threshold > remaining_330)) {
        return ShipAiThrottleDirection::Ahead;
    }

    // 009EF17A..009EF18E: 120 degrees while astern, 130 otherwise.
    const int degrees = astern ? kShipAiArmTailAsternAngleAsternDeg
                               : kShipAiArmTailAsternAngleAheadDeg;
    const float limit = static_cast<float>(
        (static_cast<double>(degrees) * kShipAiArmTailPi) / kShipAiArmTailHalfCircleDeg);
    // 009EF196 FCOMIP with JBE: only an ordered "error > limit" reverses.
    return (heading_error_abs > limit) ? ShipAiThrottleDirection::Astern
                                       : ShipAiThrottleDirection::Ahead;
}

// ---------------------------------------------------------------------------
// 009EEAAB..009EF226 as one routine.
// ---------------------------------------------------------------------------
ShipAiArmTailResult ship_ai_navigation_arm_tail_009eeaab(
    ShipAiControlBlock& blk,
    ShipAiArmTailState& tail,
    const ShipAiNavState& nav,
    const ShipAiArmTailTuning& tuning,
    const ShipAiNavWaypoint& waypoint,
    const ShipAiNavPose& goal,
    const ShipAiNavPose& pose,
    bool goal_is_destination,
    float seconds,
    ShipAiArmTailHost& host) {
    ShipAiArmTailResult result;

    // -- 009EEAA8 / 009EEABB: the setback starts at zero on every path in.
    float setback = 0.0f;

    // -- 009EEAB7..009EEAF8, the four-way gate.
    const bool gate_open =
        tuning.keep_clear_of_traffic_21 &&              // [[blk+3FCh]+73Ch]+21h
        waypoint.more_path &&                           // record+20h
        tuning.neighbour_count_604 > 0 &&               // blk+604h
        static_cast<double>(blk.distance_32c) < kShipAiArmTailTrafficRange;
    result.setback.scanned = gate_open;

    if (gate_open) {
        // 009EEAFE..009EEB26: distance to the path point times the turn lead
        // the output block applied, the arc that lead sweeps.
        const float allowance = blk.distance_32c * std::fabs(nav.turn_lead_328);

        // 009EEB2A..009EEB68: the walk starts at the goal and runs back along
        // the unit direction from the pose to the goal.
        float point_x = goal.x;
        float point_z = goal.z;
        const ShipAiNavPose dir =
            host.normalize_004192e0_009eeb63(goal.x - pose.x, goal.z - pose.z);

        const ShipAiArmTailEntity own_unit = host.own_unit_009eebd2();  // 009EEBD2
        const float own_radius = host.entity_hull_radius_009eebe0(own_unit);

        bool walking = true;
        while (walking && result.setback.steps < kShipAiArmTailWalkGuard) {
            walking = false;
            // 009EEB80..009EEB92: the list length is re-read on every pass.
            const int count = host.neighbour_list_count_009eeb8b();
            for (int index = 0; index < count; ++index) {
                const ShipAiArmTailEntity entity = host.list_element_009dbbc0(index);
                if (entity == nullptr) {                            // 009EEBB9
                    continue;
                }
                if (!host.entity_is_kind_009eebc8(entity, kShipAiArmTailNeighbourKind)) {
                    continue;                                       // 009EEBCC
                }
                if (entity == own_unit) {                           // 009EEBDA
                    continue;
                }
                const float clearance = ship_ai_traffic_clearance_009eec15(
                    allowance, setback, own_radius,
                    host.entity_hull_radius_009eebe0(entity));
                const ShipAiNavPose other =
                    host.entity_position_00427eb0_009eec41(entity);

                // 009EEC59..009EECBF: squared distance against the radius.
                const float dx = point_x - other.x;
                const float dz = point_z - other.z;
                if (dx * dx + dz * dz < clearance * clearance) {
                    const ShipAiSetbackStep step = ship_ai_traffic_setback_step_009eed62(
                        setback, point_x, point_z, dir.x, dir.z, other.x, other.z, clearance);
                    setback = step.walked;
                    point_x = step.point_x;
                    point_z = step.point_z;
                    ++result.setback.steps;
                    // 009EEED8..009EEEEC: give up once the walk has eaten the
                    // whole remaining path, otherwise rescan from the top.
                    walking = !(blk.distance_330 <= setback);
                    break;
                }
            }
        }
    }
    result.setback.distance = setback;

    // -- 009EECDB..009EEEF8, the approach ceiling.
    float ceiling = kShipAiArmTailCeilingCap;
    if (goal_is_destination) {
        ceiling = ship_ai_approach_ceiling_009eecdb(blk.distance_330, tuning.reference_speed_3c4,
                                                    tuning.class_deceleration_508);
    }
    // 009EEF0A: the store happens on both paths.
    tail.throttle_ceiling_344 = ceiling;
    result.throttle_ceiling = ceiling;

    bool release_stop = false;  // BL, 009EEF02 / 009EEF3E
    bool changed = false;       // the byte at [ESP+37h], 009EDA2F cleared it

    if (blk.direction == ShipAiThrottleDirection::Stopped) {  // 009EEF04
        // 009EEF14..009EEF38: leave the stop only once the goal is further
        // away than the start radius plus whatever traffic is in the way.
        if (waypoint.steer_enabled &&
            (tuning.start_radius_3d8 + setback) < blk.distance_330) {
            release_stop = true;
            changed = true;  // 009EF002
        }
    } else if (!blk.flag_3a5) {  // 009EEF45, station keeping never stops here
        bool stop = false;
        if (!waypoint.steer_enabled) {          // 009EEF52
            stop = true;
        } else if (blk.distance_330 < (tuning.stop_radius_3d4 + setback)) {
            stop = true;                        // 009EEF5C..009EEF70, arrived
        } else if (tail.parked_2fd && waypoint.more_path) {
            // 009EEF8D..009EEFD1: a parked ship also stops when its look-ahead
            // radius has collapsed below the smaller of a tenth of the longest
            // path and a quarter of the class turn distance.
            const float path_share = static_cast<float>(
                static_cast<double>(nav.longest_path_1f0) * kShipAiArmTailPathLengthShare);
            const float turn_share = static_cast<float>(
                static_cast<double>(tuning.turn_distance_3cc) * kShipAiArmTailTurnDistanceShare);
            const float floor_value = min_by_ref_00415510(turn_share, path_share);
            if (static_cast<double>(nav.look_ahead_340) < static_cast<double>(floor_value)) {
                stop = true;
            }
        }
        if (stop) {  // 009EEFD3
            release_stop = false;
            ship_ai_set_direction_latch_009d5240(blk, ShipAiThrottleDirection::Stopped, false);
            changed = true;  // 009EF002
            result.request_stop = true;
        }
    }
    result.release_stop = release_stop;

    // -- 009EF007, the latch itself.
    const ShipAiThrottleDirection latched = blk.direction;
    if (latched == ShipAiThrottleDirection::Stopped && !release_stop) {
        // 009EF015..009EF04C: the ship is stopped and staying stopped, so it
        // publishes the two arrival bytes.
        if (waypoint.steer_enabled && !tail.plan_reset_2fc) {
            tail.goal_reached_2fe = true;  // 009EF034
            if (tail.plan_search_state_1c > kShipAiArmTailPlanStateFloor) {
                tail.parked_2fd = true;    // 009EF045
            }
        }
    } else if (blk.requested_direction != ShipAiThrottleDirection::Stopped) {
        // 009EF051..009EF0A3: an outside order owns the gear.  blk+1CCh == 1
        // means ahead, any other non-zero value means astern.
        const ShipAiThrottleDirection wanted =
            (blk.requested_direction == ShipAiThrottleDirection::Ahead)
                ? ShipAiThrottleDirection::Ahead
                : ShipAiThrottleDirection::Astern;
        // This arm does not set the result byte, so an ordered gear change
        // does not zero brain+0B14h at 009F5215.
        ship_ai_set_direction_latch_009d5240(blk, wanted, false);
    } else if (!(blk.flag_3a5 && !tail.leader_snapshot_3a6) &&  // 009EF0A8..009EF0B8
               (0.0f > blk.timer_360)) {                        // 009EF0BE
        // 009EF0D6..009EF109: the heading error against the arm's own target.
        const float heading = host.unit_heading_vtable_0050_009ef0d6();
        const float error =
            std::fabs(wrapped_angle_subtract_00438b10(blk.heading_target_324, heading));
        const float turn_radius = host.ship_class_turn_radius_0082e850_009ef112();
        const ShipAiThrottleDirection wanted = ship_ai_astern_choice_009ef0a8(
            error, blk.distance_330, turn_radius, tuning.hull_radius_9c8, latched);
        if (wanted != latched) {
            // 009EF1AB / 009EF1CD: the byte is set before the second timer
            // test, so a refused write still reports a wanted change.
            changed = true;
            ship_ai_set_direction_latch_009d5240(blk, wanted, false);
        }
    }

    result.direction = blk.direction;
    result.direction_change_requested = changed;

    // -- 009EF206..009EF213: every arm of 009ED6B0 ends here.
    host.after_arm_009de5b0(seconds);
    return result;
}

}  // namespace bsp
