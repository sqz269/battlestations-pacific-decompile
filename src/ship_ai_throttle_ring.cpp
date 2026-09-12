// The ship AI's throttle and steering on their way into the order ring.
//
// Packet cc_ai_throttle_ring. Every routine below carries the address it comes
// from, the original ABI and its coverage. Ghidra was read-only for the code
// reading; names are hypotheses, not recovered symbols. See
// docs/SHIP_AI_THROTTLE_TO_RING.md and docs/SHIP_AI_ORDER_CONSUMER.md.

#include "bsp/ship_ai_throttle_ring.hpp"

#include <cmath>

namespace bsp {

// ---------------------------------------------------------------------------
// 00415620, float __fastcall(float* value, const float* low, const float* high)
// RET 4, body 00415620-0041565E, complete.
// ---------------------------------------------------------------------------
// 00415635 FCOMI ST0,ST1 with ST0 = *low and ST1 = *value, then JA: the low
// bound wins only on a strict greater-than, which an unordered compare does not
// take. 0041564B FCOMI with ST0 = *value and ST1 = *high, then JBE: the value
// is kept only when it is less than or equal, so a NaN falls through to *high.
float clamp_float_by_ref_00415620(float value, float low, float high) noexcept {
    if (low > value) {
        return low;
    }
    if (value <= high) {
        return value;
    }
    return high;
}

// ---------------------------------------------------------------------------
// 006BC0C0, float* __thiscall(float* out)(float heading)
// RET 4, body 006BC0C0-006BC111, complete.
// ---------------------------------------------------------------------------
// 006BC0CB FSUBR from 00CE3830, 006BC0D8 JNC over the FADD from 00CE3828, then
// fcos into out[0] and fsin into out[1]. Both literals are doubles, so the
// subtraction and the wrap happen in the x87 stack's precision and only the two
// results are stored as float32.
std::array<float, 2> heading_to_direction_006bc0c0(float heading) noexcept {
    double angle = kHeadingBasisQuarterTurn - static_cast<double>(heading);
    if (angle < 0.0) {
        angle += kHeadingBasisFullTurn;
    }
    return std::array<float, 2>{static_cast<float>(std::cos(angle)),
                                static_cast<float>(std::sin(angle))};
}

// ---------------------------------------------------------------------------
// 0080E170 and 0080E190, both void __thiscall(unit)(float)
// RET 4, bodies 0080E170-0080E18A and 0080E190-0080E1AA, complete.
// ---------------------------------------------------------------------------
// Five instructions each. `SHL EAX,5` is the 20h slot stride and the
// displacement is 838h for param_a and 83Ch for param_b, so the index is the
// ring's write cursor at unit+97Ch = ring+144h. No bound is consulted and the
// live pair at ring+148h / +14Ch is untouched.
void ship_ai_ring_set_write_slot_throttle_0080e170(UnitOrderRing& ring, float value) noexcept {
    ring.slot[ring.write_cursor].param_a = value;
}

void ship_ai_ring_set_write_slot_rudder_0080e190(UnitOrderRing& ring, float value) noexcept {
    ring.slot[ring.write_cursor].param_b = value;
}

float ship_ai_ring_write_slot_throttle(const UnitOrderRing& ring) noexcept {
    return ring.slot[ring.write_cursor].param_a;
}

float ship_ai_ring_write_slot_rudder(const UnitOrderRing& ring) noexcept {
    return ring.slot[ring.write_cursor].param_b;
}

// ---------------------------------------------------------------------------
// 009DA250, float10 __thiscall(blk)(float heading_error)
// RET 4, body 009DA250-009DA3A4, complete.
// ---------------------------------------------------------------------------
// 009DA258 FCHS negates the incoming error. 009DA25C loads [blk+3FCh],
// 009DA262 [unit+538h] and 009DA268 the class float at +524h; 009DA276
// multiplies it by 00CEC160 = 1.2 and 009DA280 divides. 009DA28C and 009DA2A8
// clamp to [-1,+1] against 00D7A260 and 00D7A24C. 009DA2B0 tests blk+35Ch
// against 2 and negates through the 00D7A208 = -0.0f subtraction at 009DA2C7.
// Then 0092D730 at 009DA2D7 gives the signed forward
// speed: below 00CE65D0 = 0.4 in magnitude the routine returns 0.0f outright,
// and above it the demand is scaled by |speed| - 0.4 clamped to [0,1] and
// negated once more when the latched direction disagrees with the sign of the
// speed. Every float expression here is float32 in the image except the two
// doubles named above, which are folded in x87 before the store.
float ship_ai_rudder_from_heading_error_009da250(ShipAiThrottleDirection latched_direction,
                                                 float heading_error,
                                                 float ship_class_yaw_authority_0524,
                                                 ShipAiRudderLawHost& host) {
    const float authority =
        static_cast<float>(static_cast<double>(ship_class_yaw_authority_0524) *
                           kShipAiRudderAuthorityScale);
    const float raw = -heading_error / authority;

    float demand = kShipAiRudderLow;
    if (kShipAiRudderLow <= raw) {
        demand = raw;
        if (kShipAiRudderHigh < raw) {
            demand = kShipAiRudderHigh;
        }
    }
    if (latched_direction == ShipAiThrottleDirection::Astern) {
        demand = -demand;
    }

    const float speed = host.unit_body_axis_speed_0092d730();
    const float magnitude = std::fabs(speed);
    if (static_cast<double>(magnitude) < kShipAiRudderSpeedFloor) {
        return 0.0f;
    }

    float ramp = magnitude - static_cast<float>(kShipAiRudderSpeedFloor);
    if (ramp < 0.0f) {
        ramp = 0.0f;
    } else if (ramp > kShipAiRudderHigh) {
        ramp = kShipAiRudderHigh;
    }
    demand = ramp * demand;

    const bool astern_but_moving_ahead =
        latched_direction == ShipAiThrottleDirection::Astern && speed > 0.0f;
    const bool ahead_but_moving_astern =
        latched_direction == ShipAiThrottleDirection::Ahead && speed < 0.0f;
    if (astern_but_moving_ahead || ahead_but_moving_astern) {
        demand = -demand;
    }
    return demand;
}

// ---------------------------------------------------------------------------
// 00828F20, bool __thiscall(descriptor)(void)
// RET at 00828F79, body 00828F20-00828F79, complete.
// ---------------------------------------------------------------------------
// 00828F23 MOVSS XMM0,[ECX+4FCh]; 00828F2B XORPS XMM1,XMM1; 00828F2E COMISS
// XMM1,XMM0; 00828F37 JNC 00828F74. The fall-through needs CF = 1, which an
// ordered COMISS sets only for 0 < value but which an unordered pair also sets,
// so a NaN key passes the gate in the image. 00828F39..00828F49 repeats the
// test on [ECX+4F8h]. Both failures reach 00828F74 XOR AL,AL and return without
// storing anything.
//
// The arithmetic is one x87 stack:
//   00828F4B FLD    [ESP]            ; MaxRotAngle, spilled at 00828F44
//   00828F4E FLD    ST0              ; a second copy
//   00828F50 FDIV   [ESP+4]          ; ST0 = MaxRotAngle / MaxRotAngleChangeRatio
//   00828F54 FMUL   double 00D7A280  ; * 0.5
//   00828F5A FSTP   [ECX+524h]       ; stores and pops, leaving MaxRotAngle
//   00828F60 FDIVR  [ECX+500h]       ; ST0 = MaxSpeed / MaxRotAngle
//   00828F66 FSTP   [ECX+520h]
// FDIVR divides the memory operand by ST0, not the other way round. Both
// quotients are computed at the x87 working precision and only the two stores
// round to float32, which is what the doubles below reproduce. 00828F6F tail
// jumps to 00951F20, a two-line `return 00876180() != 0`, so the returned bool
// is that call's result, not the derivation's; 0096515D ignores it either way.
ShipClassAiDerivedMotion ship_class_ai_derived_motion_00828f20(
    float max_rot_angle_04f8, float max_rot_angle_change_ratio_04fc, float max_speed_0500) {
    ShipClassAiDerivedMotion out{};

    // COMISS + JNC: continue on `0 < value`, and on an unordered pair.
    const auto passes_gate = [](float value) {
        return (0.0f < value) || std::isnan(value);
    };
    if (!passes_gate(max_rot_angle_change_ratio_04fc) || !passes_gate(max_rot_angle_04f8)) {
        return out;
    }

    const double max_rot_angle = static_cast<double>(max_rot_angle_04f8);
    out.yaw_authority_0524 = static_cast<float>(
        (max_rot_angle / static_cast<double>(max_rot_angle_change_ratio_04fc)) *
        kShipClassYawAuthorityHalf);
    out.turn_radius_0520 =
        static_cast<float>(static_cast<double>(max_speed_0500) / max_rot_angle);
    out.derived = true;
    return out;
}

// ---------------------------------------------------------------------------
// 009F4C0E..009F4C78 and 009F4C86..009F4CDA, the slew
// ---------------------------------------------------------------------------
// Both copies are the same six-instruction shape. The compare is
// `step <= |previous - desired|` (009F4C4E FCOMI, JBE at 009F4C52), so a
// desired value already within one step passes through untouched.
float ship_ai_slew_toward_ring_009f4c0e(float previous, float desired, float step) noexcept {
    if (step <= std::fabs(previous - desired)) {
        return (desired <= previous) ? previous - step : previous + step;
    }
    return desired;
}

// ---------------------------------------------------------------------------
// 009F4B99..009F4D04, the tail of 009F3F80
// void __thiscall(blk)(float dt), RET 4 at 009F4D04, body 009F3F80-009F4D06.
// Coverage: complete for 009F4B99..009F4D04; the rest of 009F3F80 is not
// projected here (see the doc's coverage table).
// ---------------------------------------------------------------------------
// 009F4BA7..009F4BC6: |blk+1D0h| compared against the double 00D7A270 = 0.05.
// Only when the throttle is inside that deadband is 0092D730 called at all
// (009F4BD4), and only when the speed magnitude is also under 00D7A24C = 1.0f
// is blk+1D4h zeroed (009F4BFC). 009F4C12 multiplies dt by the double
// 00CE3D78 = 1.5 once and both slews use the product. The rudder setter runs
// first (009F4CE8) and the throttle setter second (009F4CFB); both take
// ECX = [blk+3FCh], the unit.
ShipAiRingHop ship_ai_order_ring_hop_009f4b99(ShipAiControlBlock& blk,
                                              float previous_ring_throttle,
                                              float previous_ring_rudder, float dt,
                                              ShipAiRingHopHost& host) {
    ShipAiRingHop out{};

    if (static_cast<double>(std::fabs(blk.desired_throttle)) < kShipAiRingHopThrottleDeadband) {
        const float speed = host.unit_body_axis_speed_0092d730();
        if (std::fabs(speed) < kShipAiRingHopSpeedDeadband) {
            blk.desired_rudder = 0.0f;
            out.rudder_zeroed = true;
        }
    }

    const float step = static_cast<float>(static_cast<double>(dt) * kShipAiOrderSlewRate);
    out.ring_throttle =
        ship_ai_slew_toward_ring_009f4c0e(previous_ring_throttle, blk.desired_throttle, step);
    out.ring_rudder =
        ship_ai_slew_toward_ring_009f4c0e(previous_ring_rudder, blk.desired_rudder, step);

    host.set_ring_write_slot_rudder_0080e190(out.ring_rudder);
    host.set_ring_write_slot_throttle_0080e170(out.ring_throttle);
    return out;
}

// ---------------------------------------------------------------------------
// 00815F30, void __thiscall(record)(const float* xz, int dir, float lo, float hi)
// RET 10h, body 00815F30-008160AC, complete.
// ---------------------------------------------------------------------------
// 00815F43 parks 00CE3958 = 2.0f in record+04h. 00815F5B..00815F63 build the
// squared distance from the stored sub_a position to the argument and compare
// it against the double 00CE3D90 = 400.0; beyond that the seven fields of
// sub_a are copied into sub_b and sub_a is reset with field_08 = 00CE3804 =
// 1000.0f. Then sub_a is restated for the new position. The direction arm at
// 00815FD1 keeps the bounds as given for direction 1 and negates both
// otherwise, which swaps their order, and the clamp that follows is written out
// twice in the image with the operands in the swapped order.
void unit_ai_order_push_turn_limit_00815f30(UnitAiOrderRecord& record,
                                            const std::array<float, 2>& position_xz,
                                            int direction, float low, float high) noexcept {
    record.timer_04 = kUnitAiOrderTurnLimitTimer;

    if (record.sub_a.flag_0c) {
        const float dx = record.sub_a.field_10 - position_xz[0];
        const float dz = record.sub_a.field_14 - position_xz[1];
        if (static_cast<double>(dx * dx + dz * dz) > kUnitAiOrderTurnLimitRadiusSq) {
            record.sub_b = record.sub_a;
            record.sub_a.field_04 = 0.0f;
            record.sub_a.field_08 = kUnitAiOrderSubRecordReset;
            record.sub_a.value_00 = 0.0f;
            record.sub_a.flag_0c = false;
            record.sub_a.field_18 = 0;
        }
    }

    record.sub_a.flag_0c = true;
    record.sub_a.field_10 = position_xz[0];
    record.sub_a.field_14 = position_xz[1];
    record.sub_a.field_18 = static_cast<std::uint32_t>(direction);

    float bound_low = low;
    float bound_high = high;
    if (direction != 1) {
        bound_low = -low;
        bound_high = -high;
        record.sub_a.field_04 = bound_low;
        record.sub_a.field_08 = bound_high;
        const float held = record.sub_a.value_00;
        if (bound_high <= held) {
            record.sub_a.value_00 = (held <= bound_low) ? held : bound_low;
        } else {
            record.sub_a.value_00 = bound_high;
        }
        return;
    }

    record.sub_a.field_04 = bound_low;
    record.sub_a.field_08 = bound_high;
    const float held = record.sub_a.value_00;
    if (held < bound_low) {
        record.sub_a.value_00 = bound_low;
    } else if (held <= bound_high) {
        record.sub_a.value_00 = held;
    } else {
        record.sub_a.value_00 = bound_high;
    }
}

// ---------------------------------------------------------------------------
// 00811D80, float10 __thiscall(record)(const float* xz)
// RET 4, body 00811D80-00811E7C, complete.
// ---------------------------------------------------------------------------
// 00811D83 loads the default 00E0E304 = 30.0f. Each sub-record is gated on its
// flag_0c and on the same squared radius the writer uses, 00CE3D90 = 400.0.
// The clamped value is always record+00h, the blended value 0080E000 maintains:
// ECX is never reloaded in sub_a's arm after 00811D8C MOV ESI,ECX, and sub_b's
// two arms reload it explicitly with MOV ECX,ESI at 00811E49 and 00811E66. The
// sign test is COMISS 0.0 against field_08 with JBE, so a non-negative high
// bound takes the [field_04, field_08] clamp and a negative one takes the
// swapped clamp followed by FCHS at 00811DEB / 00811E5B. sub_b's negative arm
// returns at 00811E5F without consulting anything else.
float unit_ai_order_turn_limit_at_00811d80(const UnitAiOrderRecord& record,
                                           const std::array<float, 2>& position_xz) noexcept {
    float result = kUnitAiOrderTurnLimitDefault;

    if (record.sub_a.flag_0c) {
        const float dx = record.sub_a.field_10 - position_xz[0];
        const float dz = record.sub_a.field_14 - position_xz[1];
        if (static_cast<double>(dx * dx + dz * dz) < kUnitAiOrderTurnLimitRadiusSq) {
            if (record.sub_a.field_08 >= 0.0f) {
                result = clamp_float_by_ref_00415620(record.blend_00, record.sub_a.field_04,
                                                     record.sub_a.field_08);
            } else {
                result = -clamp_float_by_ref_00415620(record.blend_00, record.sub_a.field_08,
                                                      record.sub_a.field_04);
            }
        }
    }

    if (record.sub_b.flag_0c) {
        const float dx = record.sub_b.field_10 - position_xz[0];
        const float dz = record.sub_b.field_14 - position_xz[1];
        if (static_cast<double>(dx * dx + dz * dz) < kUnitAiOrderTurnLimitRadiusSq) {
            if (record.sub_b.field_08 >= 0.0f) {
                result = clamp_float_by_ref_00415620(record.blend_00, record.sub_b.field_04,
                                                     record.sub_b.field_08);
            } else {
                return -clamp_float_by_ref_00415620(record.blend_00, record.sub_b.field_08,
                                                    record.sub_b.field_04);
            }
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// 009D8DCE..009D8F7C, the crossing geometry inside 009D8CE0
// void __thiscall(self)(void* candidate, void* other), RET 8 at 009D913F.
// Coverage: complete for this range.
// ---------------------------------------------------------------------------
// 009D8D63 and 009D8DCE both call 006BC0C0, on self's published heading
// (009D8D35 FLD [EDX+EAX+0ADCh], which is slot+44h) and on the other unit's
// (009D8D71 FLD [EBX+44h]). self's basis is (d.x, d.y) forward and
// (d.y, -d.x) across; 009D8E52 and 009D8EC2 are the two ABS masks that build
// the magnitudes the gate later uses. The intersection is the classic
// two-ray solve: the lateral offset of the other unit divided by the rate that
// offset closes gives the parameter along the other unit's ray.
ShipAiCrossingGeometry ship_ai_crossing_geometry_009d8dce(const ShipAiCrossingInput& in) noexcept {
    ShipAiCrossingGeometry out{};

    const std::array<float, 2> self_dir = heading_to_direction_006bc0c0(in.self_published_heading);
    const std::array<float, 2> other_dir =
        heading_to_direction_006bc0c0(in.other_published_heading);

    // The across-track axis of self: (dir.y, -dir.x).
    const float across_x = self_dir[1];
    const float across_z = -self_dir[0];

    const float to_other_x = in.other_position[0] - in.self_position[0];
    const float to_other_z = in.other_position[1] - in.self_position[1];
    out.lateral_offset = to_other_x * across_x + to_other_z * across_z;

    const float stepped_x = (other_dir[0] + in.other_position[0]) - in.self_position[0];
    const float stepped_z = (other_dir[1] + in.other_position[1]) - in.self_position[1];
    out.closing_lateral = out.lateral_offset - (stepped_x * across_x + stepped_z * across_z);

    if (std::fabs(out.closing_lateral) <= kShipAiCrossingParallelEpsilon) {
        return out; // 009D8ED6/009D8EDD: parallel tracks, 009D8CE0 exits at 009D9120
    }
    out.tracks_cross = true;

    const float t = out.lateral_offset / out.closing_lateral;
    const float cross_x = in.other_position[0] + t * other_dir[0];
    const float cross_z = in.other_position[1] + t * other_dir[1];

    out.self_range_to_cross = (cross_x - in.self_position[0]) * self_dir[0] +
                              (cross_z - in.self_position[1]) * self_dir[1];
    out.other_range_to_cross = (cross_x - in.other_position[0]) * other_dir[0] +
                               (cross_z - in.other_position[1]) * other_dir[1];
    return out;
}

// 009D8FD3 FLD [EDI+9CCh] with EDI = the other unit, FMUL double 00D7A328 = 4,
// 009D8FEC FCOMIP and 009D8FF0 JA: the offset magnitude at [ESP+0x34] was
// masked with 7FFFFFFF at 009D8EC2.
bool ship_ai_crossing_length_gate_009d8fd3(const ShipAiCrossingGeometry& geometry,
                                           const ShipAiCrossingInput& in) noexcept {
    const float bound =
        static_cast<float>(static_cast<double>(in.other_length_09cc) * kShipAiCrossingLengthScale);
    return std::fabs(geometry.lateral_offset) < bound;
}

// ---------------------------------------------------------------------------
// 009D8FE4..009D9136, the decision
// Coverage: partial. The two bounds the ranges are compared against are built
// at 009D8F80..009D8FCB; three of their four operands are resolved and the
// fourth, [ESP+3Ch], has no writer in the stored listing, so the whole
// 009D8FE4..009D9036 test arrives as `inside_gate`. Everything else is
// projected.
// ---------------------------------------------------------------------------
// 009D8FD3 FLD [EDI+9CCh] then FMUL double 00D7A328 = 4.0: the first disjunct
// of the gate is `|lateral_offset| < other_length * 4`. 009D9038..009D9044
// divides self's radius by the double 00D049A8 = 1.8 and 009D905A takes the max
// of that and a stack operand before it is added to the other unit's range.
// 009D90EC subtracts half of self's radius from self's range when self was
// already in SelfFirst, which is the hysteresis. 009D906D decides between the
// two states from the two ranges.
ShipAiCrossingState ship_ai_crossing_decide_009d8fe4(const ShipAiCrossingGeometry& geometry,
                                                     const ShipAiCrossingInput& in,
                                                     ShipAiCrossingState previous,
                                                     bool inside_gate) noexcept {
    if (!geometry.tracks_cross || !inside_gate) {
        return ShipAiCrossingState::None;
    }

    // 009D9038..009D9044: self's own radius divided by the double 1.8.
    const float margin =
        static_cast<float>(static_cast<double>(in.self_radius_09c8) / kShipAiCrossingRadiusDivisor);
    // 009D904C..009D905A: max(100.0f, self radius), added to the other unit's
    // range before it is tested against the path the other unit has left.
    const float reach = (kShipAiCrossingReachFloor > in.self_radius_09c8)
                            ? kShipAiCrossingReachFloor
                            : in.self_radius_09c8;

    if (geometry.other_range_to_cross + reach > in.other_published_path_left) {
        return ShipAiCrossingState::OtherFirst; // 009D9073
    }

    float self_range = geometry.self_range_to_cross;
    bool conflict = false;
    if (self_range <= 0.0f) {
        conflict = geometry.other_range_to_cross > 0.0f; // 009D908D / 009D90D3
    } else if (margin + self_range <= in.self_published_distance) {
        conflict = true; // 009D9099 falls through to 009D90DE
    } else {
        return ShipAiCrossingState::OtherFirst; // 009D90A1
    }
    if (!conflict) {
        return ShipAiCrossingState::None;
    }

    if (previous == ShipAiCrossingState::SelfFirst) {
        // 009D90EC: the hysteresis multiplies the divided radius, not the raw one.
        self_range =
            self_range - static_cast<float>(static_cast<double>(margin) * kShipAiCrossingHysteresis);
    }
    return (geometry.other_range_to_cross <= self_range) ? ShipAiCrossingState::OtherFirst
                                                         : ShipAiCrossingState::SelfFirst;
}

} // namespace bsp
