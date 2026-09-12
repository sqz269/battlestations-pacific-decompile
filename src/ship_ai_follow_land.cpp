// Reconstruction of the ship AI `follow` and `land` state steps. See
// include/bsp/ship_ai_follow_land.hpp and docs/SHIP_AI_FOLLOW_LAND.md. Every
// routine is a projection of the listing named in its comment; the descriptive
// names are hypotheses, not recovered symbols. These are semantic interfaces,
// not binary replacements: the native routines work on the brain and the block
// directly and take their arguments in ECX and on the x87 stack.
#include "bsp/ship_ai_follow_land.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"  // clamped_interpolate_00419010, 00419010

namespace bsp {

// ---------------------------------------------------------------------------
// 009E1610, body 009E1610-009E18C2, complete.
// __thiscall(state)(float seconds), RET 4 at 009E18C0. The one stack argument
// is proved by the RET 4; no instruction in the body reads [ESP+...] for it.
// ---------------------------------------------------------------------------

// 009E164C..009E1684. Two arms of one latch on the **leader's** speed: the
// enter test at 009E167C compares the speed against zero (FLDZ, FXCH, FCOMIP,
// JC on speed < 0) and the leave test at 009E1669 compares it against the
// float32 at 00D7A260.
bool ship_ai_follow_making_way_latch(bool latched, float leader_speed) {
    if (!latched) {
        // 009E1675..009E1684: set on a speed at or above zero.
        if (leader_speed >= 0.0f) return true;
        return false;
    }
    // 009E165E..009E166F: clear at or below -1.0f, so only a leader actually
    // backing down flips it; a leader merely stopped keeps the formation ahead.
    if (leader_speed <= kShipAiFollowAsternSpeed) return false;
    return true;
}

// 009E16F0..009E172D. `r` is the turn radius scaled by the double at 00CE3DE0
// and rounded to float32 at 009E16FF before it is squared at 009E1707.
bool ship_ai_follow_station_latch(bool latched, float distance_sq, float turn_radius) {
    const float scaled =
        static_cast<float>(static_cast<double>(turn_radius) * kShipAiFollowStationRadiusScale);
    const float radius_sq = scaled * scaled;  // 009E1703..009E1709
    if (!latched) {
        // 009E1723..009E172D: FADD ST0,ST0 doubles the squared radius, so the
        // ship leaves station at sqrt(2) * 2.5 turn radii.
        return distance_sq > radius_sq + radius_sq;
    }
    // 009E1717..009E171D: it regains station inside 2.5 turn radii.
    return !(distance_sq < radius_sq);
}

// 009E174C..009E1773. The deltas are station point minus unit position, in that
// order (009E174C FLD [EDI] then FSUB the saved position).
float ship_ai_follow_station_dot(const ShipAiFollowLandXZ& station_point,
                                 const ShipAiFollowLandXZ& slot_direction,
                                 const ShipAiFollowLandXZ& unit_position) {
    const float dx = station_point.x - unit_position.x;  // 009E174E
    const float dz = station_point.z - unit_position.z;  // 009E1759
    // 009E1761..009E1773, one x87 chain with a single store: z term first.
    return dz * slot_direction.z + dx * slot_direction.x;
}

// 009E17CE..009E182A.
ShipAiFollowLandXZ ship_ai_follow_back_off(const ShipAiFollowLandXZ& station_point,
                                           const ShipAiFollowLandXZ& slot_direction,
                                           const ShipAiFollowLandXZ& unit_position,
                                           float offset) {
    const float dx = station_point.x - unit_position.x;
    const float dz = station_point.z - unit_position.z;
    // 009E17C4..009E17C8: the normal is (-dz, dx) of the slot direction.
    const float normal_x = -slot_direction.z;
    const float normal_z = slot_direction.x;
    // 009E17D4..009E17E8, one x87 chain: the cross product of the normal with
    // the ship-to-station delta.
    const float cross = normal_x * dx + normal_z * dz;
    const float step_x = normal_x * offset;  // 009E17FC, 009E1800
    const float step_z = normal_z * offset;  // 009E1804, 009E1806
    ShipAiFollowLandXZ moved = station_point;
    if (cross < 0.0f) {                      // 009E17F2 FCOMIP, 009E180C JC
        moved.x += step_x;                   // 009E181D
        moved.z += step_z;                   // 009E1826
    } else {
        moved.x -= step_x;                   // 009E180E
        moved.z -= step_z;                   // 009E1817
    }
    return moved;
}

// 009E183C..009E18A5. The two zeroed dwords at the head of the request are the
// only fields the out-of-station arm leaves untouched: 009E184C..009E185E zero
// request+8h..+14h and 009E1844/009E1848 set the two bytes, but nothing writes
// request+0h or +4h unless the in-station arm runs. They are consumed only
// behind `suppress == false` (docs/SHIP_AI_GOAL_VECTOR.md, the 009EDA34 /
// 009EDA41 gate), so the stale pair is never read; this projection publishes
// zeros there rather than reproducing the uninitialised stack read.
ShipAiStationRequest ship_ai_follow_build_request(const ShipAiFollowState& state) {
    ShipAiStationRequest request{};
    request.making_way = true;                       // 009E1844, BL = 1
    request.enable = true;                           // 009E1848, BL = 1
    request.heading = 0.0f;                          // 009E184C
    request.leader_speed = 0.0f;                     // 009E1852
    request.unused_10 = 0.0f;                        // 009E1858
    request.radius = 0.0f;                           // 009E185E
    request.suppress = state.out_of_station_08;      // 009E1864
    if (!state.out_of_station_08) {                  // 009E1842 TEST, 009E1869 JNZ
        request.direction_x = state.slot_direction_1c.x;  // 009E186B
        request.direction_z = state.slot_direction_1c.z;  // 009E1882
        request.heading = state.slot_heading_24;          // 009E1891
        request.leader_speed = state.leader_speed_28;     // 009E189C
        request.radius = static_cast<float>(static_cast<double>(state.radius_30) *
                                           kShipAiFollowRequestRadiusScale);  // 009E1873
        request.making_way = state.making_way_2c;         // 009E1879, 009E18A1
    }
    return request;
}

// 009DA3B0, body 009DA3B0-009DA408, complete. Six dwords then three bytes, in
// this order, with no test of any kind.
void ship_ai_publish_station_request_009da3b0(const ShipAiStationRequest& request,
                                              ShipAiStationRequest& blk_038c) {
    blk_038c.direction_x = request.direction_x;    // 009DA3B6, blk+38Ch
    blk_038c.direction_z = request.direction_z;    // 009DA3BF, blk+390h
    blk_038c.heading = request.heading;            // 009DA3C8, blk+394h
    blk_038c.leader_speed = request.leader_speed;  // 009DA3D1, blk+398h
    blk_038c.unused_10 = request.unused_10;        // 009DA3DA, blk+39Ch
    blk_038c.radius = request.radius;              // 009DA3E3, blk+3A0h
    blk_038c.making_way = request.making_way;      // 009DA3ED, blk+3A4h
    blk_038c.enable = request.enable;              // 009DA3F7, blk+3A5h
    blk_038c.suppress = request.suppress;          // 009DA400, blk+3A6h
}

void ship_ai_follow_step_009e1610(ShipAiFollowState& state, ShipAiFollowStepHost& host) {
    // 009E1616..009E1646. Three gates, all leaving through the same epilogue:
    // the formation at unit+284h, its leader at +14h, and the leader's kind.
    if (!host.leader_matches_kind_5c(kShipAiFollowLeaderKind)) return;

    // 009E164C..009E1684, on the leader's controller at leader+1018h.
    state.making_way_2c =
        ship_ai_follow_making_way_latch(state.making_way_2c, host.leader_body_axis_speed_0092d730());

    // 009E1689. Everything the rest of the step reads off the state except the
    // two latches is written here.
    host.update_formation_point_009df2d0(state);

    // 009E1697..009E16BA. The refresh is guarded by the byte at unit+0C8h,
    // which the host owns.
    host.refresh_world_pose_00414db0();
    const ShipAiFollowLandXZ position = host.unit_position();

    // 009E16C7..009E16EC, one x87 chain with a single store at 009E16EC.
    const float dx = position.x - state.station_point_14.x;
    const float dz = position.z - state.station_point_14.z;
    const float distance_sq = dx * dx + dz * dz;

    // 009E16F0, ECX = brain+0AACh.
    const float turn_radius = host.ship_class_turn_radius_0082e850();
    state.out_of_station_08 =
        ship_ai_follow_station_latch(state.out_of_station_08, distance_sq, turn_radius);

    if (state.out_of_station_08) {  // 009E1730
        const float dot =
            ship_ai_follow_station_dot(state.station_point_14, state.slot_direction_1c, position);
        if (dot < 0.0f) {  // 009E177D FCOMIP against zero, 009E1781 JBE
            // 009E1790: a second call, not the value from 009E16F0.
            const float radius = host.ship_class_turn_radius_0082e850();
            // 009E1799 SUBSS with the negative zero at 00D7A208, so the first
            // argument is exactly -dot.
            const float span = host.min_float_by_ref_00415510(kShipAiFollowBackOffBias - dot, radius);
            const float offset =
                static_cast<float>(static_cast<double>(span) * kShipAiFollowBackOffScale);
            state.station_point_14 = ship_ai_follow_back_off(state.station_point_14,
                                                            state.slot_direction_1c, position, offset);
        }
    }

    // 009E1837, keep_mode = 0 and final_leg = 1: a follower never treats its
    // station as a waypoint the path planner may keep, and always treats it as
    // the last leg.
    host.set_navigation_goal_009de050(state.station_point_14, false, true);

    // 009E18B6, ECX = brain+8h.
    host.publish_station_request_009da3b0(ship_ai_follow_build_request(state));
}

// ---------------------------------------------------------------------------
// 009DA610, body 009DA610-009DA671, complete.
// __thiscall(blk)(const float* goal2d), RET 4 at 009DA660 and 009DA66E. The one
// stack argument is proved by the RET 4 and the load at 009DA61C.
// ---------------------------------------------------------------------------
bool ship_ai_nav_goal_already_reached_009da610(ShipAiGoalPlan& blk,
                                               const ShipAiFollowLandXZ& goal) {
    if (!blk.flag_2fd) return false;           // 009DA613, JZ 009DA66A
    const float dx = goal.x - blk.goal_x_1dc;  // 009DA622
    const float dz = goal.z - blk.goal_z_1e0;  // 009DA62E
    // 009DA638..009DA647 one x87 chain, then 009DA64F: the same float32 at
    // 00D20278 that 009DE050 compares the goal jump against.
    if (dx * dx + dz * dz < kShipAiGoalMoveResetDistanceSq) return true;  // 009DA655, JBE
    blk.flag_2fd = false;  // 009DA663
    return false;
}

// ---------------------------------------------------------------------------
// 006AC5D0, body 006AC5D0-006ACB37. The per-call arm only, 006AC927 onwards;
// everything before it is the cache refresh, which casts two rays through the
// avoid-zone group and stays on the host. Coverage: partial.
// ---------------------------------------------------------------------------
ShipAiFollowLandXZ ship_ai_land_pad_approach_point_006ac5d0(const ShipAiLandPadLine& line,
                                                            const ShipAiFollowLandXZ& facing,
                                                            const ShipAiFollowLandXZ& from,
                                                            float half_width) {
    const float dx = line.origin_x_20c - from.x;  // 006AC927
    const float dz = line.origin_z_214 - from.z;  // 006AC947
    // The along-track and the absolute cross-track distance, in the pad's own
    // frame. The cross term uses the same negative zero at 00D7A208 the follow
    // step uses, so its first factor is exactly -facing.z.
    const float along = dx * facing.x + facing.z * dz;
    const float across = std::fabs((kShipAiFollowBackOffBias - facing.z) * dx + facing.x * dz);

    float offset;
    if (across <= half_width && along <= half_width && -along <= line.ahead_run_21c) {
        // Inside the corridor: a carrot ahead of the ship's own projection on
        // the approach axis, tapering to the axis as the lateral error grows.
        // 006ACA2C: interp(0.5, 1.0, 1.0, 0.0, across / half_width).
        const float blend = clamped_interpolate_00419010(0.5f, 1.0f, 1.0f, 0.0f, across / half_width);
        offset = blend * half_width - along;
        // 006ACA3C: never further back than the astern clearance.
        if (offset < -line.astern_clearance_218) offset = -line.astern_clearance_218;
        return ShipAiFollowLandXZ{line.origin_x_20c + offset * facing.x,
                                  line.origin_z_214 + offset * facing.z};
    }
    // Outside it: a hold-off point one clearance behind the pad, so a ship that
    // has missed the line is sent back to the entry rather than across it.
    offset = line.astern_clearance_218 < half_width ? line.astern_clearance_218 : half_width;
    return ShipAiFollowLandXZ{line.origin_x_20c - offset * facing.x,
                              line.origin_z_214 - offset * facing.z};
}

// 009E1E0E..009E1E31.
float ship_ai_land_clamp_throttle(float value) {
    if (!(kShipAiLandSpeedRampEnd <= value)) return kShipAiLandSpeedRampEnd;  // 009E1E18
    if (value > kShipAiLandSpeedRampFull) return kShipAiLandSpeedRampFull;    // 009E1E2F
    return value;
}

// ---------------------------------------------------------------------------
// 009E1950, body 009E1950-009E201A, complete.
// __thiscall(state)(float seconds), RET 4 at 009E1E54, 009E1FFB and 009E2018.
// The stack argument is read at 009E1A1B and 009E1FBA, both as a countdown.
// ---------------------------------------------------------------------------
void ship_ai_land_step_009e1950(ShipAiLandState& state, ShipAiLandStepHost& host, float seconds) {
    // 009E195B, 009E1963: the fallback landing point is the AI goal vector, used
    // unchanged whenever no pad is held.
    ShipAiFollowLandXZ goal = host.brain_goal_vector_0b2c();

    const int unit = host.brain_unit_0aa8();     // 009E1978
    host.refresh_world_pose_00414db0(unit);      // 009E1989, guarded on unit+0C8h
    const ShipAiFollowLandXZ position = host.unit_position();  // 009E198E, 009E199F

    // 009E19AF..009E19DC. Only a kind-0Ch unit has a pad; on anything else both
    // handles stay null and the step falls through to the final arm.
    int self = 0;
    int pad = 0;
    if (unit != 0 && host.entity_matches_kind_5c(unit, kShipAiLandUnitKind)) {
        self = unit;
        pad = host.unit_landing_pad_1200(unit);  // 009E19CA
    }

    // 009E19DE..009E1A02. A pad some other unit occupies is dropped and the
    // timer forced negative, so the re-pick below runs on this same tick.
    if (pad != 0 && host.pad_occupant_006ac220(pad) != 0 &&
        host.pad_occupant_006ac220(pad) != self) {
        pad = 0;
        state.rescan_timer_0c = kShipAiLandSpeedRampEnd;  // 009E1A02, the -1.0f at 00D7A260
    }

    // 009E1A07..009E1AB3. Once the final flag is up the ship stops looking.
    if (self != 0 && !state.final_10) {
        state.rescan_timer_0c -= seconds;  // 009E1A1B, stored at 009E1A27
        if (state.rescan_timer_0c < 0.0f || pad == 0) {  // 009E1A30 JA, 009E1A34 JNZ
            state.rescan_timer_0c =
                host.random_uniform_00bd2f10(kShipAiLandRescanMin, kShipAiLandRescanMax);
            int base = 0;
            if (pad != 0) {
                host.refresh_world_pose_00414db0(pad);  // 009E1A67
                base = host.pad_owner_220(pad);         // 009E1A6C
            }
            bool have_base = base != 0;  // 009E1A72
            if (!have_base) {
                base = host.brain_landing_base_0b20();  // 009E1A79
                have_base = base != 0 && host.entity_matches_kind_5c(base, kShipAiLandBaseKind);
            }
            if (have_base) {
                // 009E1A97, second argument 0: 006F2E60 may hand back a pad this
                // unit already occupies instead of only free ones.
                const int picked = host.pick_landing_pad_006f2e60(base, self, false);
                if (picked != 0 && picked != pad) {  // 009E1AA0, 009E1AA4
                    host.assign_landing_pad_006f2fb0(base, self, picked);  // 009E1AAE
                    pad = picked;
                }
            }
        }
    }

    host.set_blk_field_300(kShipAiLandThrottleHigh);  // 009E1AC2, unconditional

    if (pad != 0) {  // 009E1AB5
        host.refresh_world_pose_00414db0(unit);  // 009E1AEC
        goal = host.pad_approach_point_006ac5d0(pad, position, host.ship_class_zone_group_570(),
                                                kShipAiLandCorridorHalfWidth);  // 009E1B0A
        state.hold_heading_11 = false;  // 009E1B24
    }
    // 009E1B28..009E1B41. The middle field is stored as zero, not as a y.
    state.goal_x_18 = goal.x;
    state.goal_y_1c = 0.0f;
    state.goal_z_20 = goal.z;

    if (!state.final_10) {  // 009E1B46
        const float dx = goal.x - position.x;  // 009E1B50
        const float dz = goal.z - position.z;  // 009E1B5C
        const float distance_sq = dx * dx + dz * dz;  // 009E1B68..009E1B7E
        // 009E1B8C, the float32 sum widened against the double at 00D21860.
        if (static_cast<double>(distance_sq) < kShipAiLandHeadingTestDistanceSq) {
            const float heading = host.heading_from_delta(dx, dz);  // 009E1B9D..009E1BC4
            // 009E1BD7 then 009E1BDD; the AND 7FFFFFFFh at 009E1BEA is the abs.
            const float error =
                std::fabs(host.subtract_wrapped_angle_00438b10(host.unit_heading_vtable_50(), heading));
            if (static_cast<double>(error) < kShipAiLandFinalHeadingError) {
                state.final_10 = true;  // 009E1C04
            }
        }

        if (!state.final_10) {  // 009E1C0E, JZ 009E1E57
            // The approach arm, 009E1E57..009E1FFB.
            const int zone_set = host.nav_zone_set_for_class_0082adc0();  // 009E1E66
            goal = host.push_point_out_of_zones_00417b10(zone_set, goal, kShipAiLandZoneMargin,
                                                         true);  // 009E1E83
            host.set_blk_flag_3f4(true);                          // 009E1EA2
            // 009E1EB6: keep_mode = 0 and final_leg = 0, the opposite of the
            // follow step's call, so the planner is free to route around the
            // approach and this leg is not the last one.
            host.set_navigation_goal_009de050(goal, false, false);
            host.set_blk_field_3f0(3);  // 009E1EC2
            state.goal_x_18 = goal.x;   // 009E1ED6
            state.goal_y_1c = 0.0f;     // 009E1EDB
            state.goal_z_20 = goal.z;   // 009E1EF5

            // 009E1EBB..009E1F18: position minus goal this time, same sum.
            const float ax = position.x - goal.x;
            const float az = position.z - goal.z;
            const float arrival_sq = ax * ax + az * az;

            float radius = host.turn_circle_radius_00811a30(1.0f);  // 009E1F21
            radius = radius + radius;                                // 009E1F26 FADD ST0,ST0
            if (static_cast<double>(radius) < kShipAiLandArrivalRadiusFloorCompare) {
                radius = kShipAiLandArrivalRadiusFloor;  // 009E1F3C
            }
            if (arrival_sq < radius * radius) {  // 009E1F60, JBE 009E1F9F
                if (host.state_reached_vtable_2c(goal) ||          // 009E1F72
                    host.goal_already_reached_009da610(goal) ||    // 009E1F83
                    arrival_sq < kShipAiLandArrivalDistanceSq) {   // 009E1F94
                    state.final_10 = true;  // 009E1F9B
                }
            }

            const float timer = state.speed_ramp_08;  // 009E1F9F
            if (timer <= kShipAiLandSpeedRampEnd) {   // 009E1FA4 COMISS, 009E1FB1 JBE
                host.set_brain_speed_scale_0af0(kShipAiLandSpeedRampFull);  // 009E200C
                return;
            }
            const float next = timer - seconds;  // 009E1FBA
            state.speed_ramp_08 = next;          // 009E1FC6
            // 009E1FE6: interp(-1, 1, 0, 0, next), so the scale climbs from 0 to
            // 1 over the second the timer takes to reach -1 and stays there.
            host.set_brain_speed_scale_0af0(host.interpolate_clamped_00419010(
                kShipAiLandSpeedRampEnd, kShipAiLandSpeedRampFull, 0.0f, 0.0f, next));
            return;
        }
    }

    // The final arm, 009E1C18..009E1E54. Reached both by falling out of the
    // tests above and directly from 009E1B4A when the flag was already up.
    state.rescan_timer_0c = kShipAiLandRescanParked;  // 009E1C23
    host.set_blk_flag_3f4(false);                     // 009E1C28
    host.set_blk_field_3f0(-1);                       // 009E1C32

    if (!state.hold_heading_11) {  // 009E1C3C
        const float dx = goal.x - position.x;  // 009E1C46
        const float dz = goal.z - position.z;  // 009E1C52
        state.hold_heading_14 = host.heading_from_delta(dx, dz);  // 009E1C66..009E1CA3
        const float distance_sq = dx * dx + dz * dz;              // 009E1C95..009E1CB0
        // 009E1CBE: the latch goes up **inside** 100 units, not outside it, so
        // the ship aims at the pad until it is close and then drives straight.
        state.hold_heading_11 = distance_sq < kShipAiLandHoldHeadingDistanceSq;
    } else {
        // 009E1CD5..009E1D31: a carrot 100 units ahead on the held heading.
        const ShipAiFollowLandXZ direction =
            host.heading_to_direction_006bc0c0(state.hold_heading_14);
        goal.x = position.x +
                 static_cast<float>(static_cast<double>(direction.x) * kShipAiLandHoldCarrotDistance);
        goal.z = position.z +
                 static_cast<float>(kShipAiLandHoldCarrotDistance * static_cast<double>(direction.z));
        state.goal_x_18 = goal.x;   // 009E1D2E
        state.goal_y_1c = 0.0f;     // 009E1CFB
        state.goal_z_20 = goal.z;   // 009E1D31
    }

    // 009E1D37..009E1D64, the same pair of calls as the approach arm's test.
    const float error = std::fabs(
        host.subtract_wrapped_angle_00438b10(host.unit_heading_vtable_50(), state.hold_heading_14));

    if (host.blk_steering_mode() != ShipAiSteeringMode::Heading) {  // 009E1D72
        host.enter_heading_mode();  // 009E1D84..009E1D97, also zeroes +360h/+368h
    }
    host.clear_path_plan_009da4e0();                               // 009E1D9F
    host.set_blk_desired_heading_1d8(state.hold_heading_14);       // 009E1DB0
    host.wrap_blk_desired_heading_00605070();                      // 009E1DB4
    host.set_blk_requested_direction_1cc(ShipAiThrottleDirection::Stopped);  // 009E1DCC

    // 009E1DEF: full ahead while the heading error is under pi/12, half at pi/4
    // and beyond, linear between.
    const float ramp = host.interpolate_clamped_00419010(
        kShipAiLandThrottleErrorLow, kShipAiLandThrottleHigh, kShipAiLandThrottleErrorHigh,
        kShipAiLandThrottleLow, error);

    // 009E1E12..009E1E48: the final arm writes the four block fields 009DBF90
    // owns, inline, rather than calling the setter.
    host.set_blk_throttle_hold_1c8(0);                                       // 009E1E12
    host.set_blk_requested_direction_1cc(ShipAiThrottleDirection::Stopped);  // 009E1E34
    host.set_blk_desired_throttle_1d0(ship_ai_land_clamp_throttle(ramp));    // 009E1E3B
    host.set_brain_speed_scale_0af0(ramp);                                   // 009E1E48
}

}  // namespace bsp
