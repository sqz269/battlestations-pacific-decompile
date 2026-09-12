// 009E4330, the ship AI navigation control block's constructor.
// See include/bsp/ship_ai_nav_block_ctor.hpp and
// docs/SHIP_AI_NAV_BLOCK_CTOR.md.  Every name is a hypothesis.
//
// Float shape: every arithmetic step below is x87 in the native routine, so
// each 80-bit intermediate is kept as `double` here and rounded to `float`
// exactly where the listing has an `FSTP`/`FST` to a dword or a `MOVSS` store.
#include "bsp/ship_ai_nav_block_ctor.hpp"

namespace bsp {
namespace {

// Rounds an 80-bit intermediate the way the native `FSTP dword ptr` does.
float store_float(double value) noexcept {
    return static_cast<float>(value);
}

} // namespace

ShipAiNavBlockSteeringDefaults ship_ai_nav_block_steering_defaults_009dfcb0() noexcept {
    // 009DFE50..009DFEA8.  Every member's initialiser is the constant the
    // matching MOVSS loads; nothing here depends on the argument.
    return ShipAiNavBlockSteeringDefaults{};
}

float ship_class_turn_circle_radius_0082e960(const ShipAiNavBlockClassInputs& ship_class,
                                             float throttle_fraction,
                                             UnitRudderCurveHost& host) {
    // 0082E96B, then 0082E970 FMUL float ptr [ESI+520h], then 0082E977 FSTP
    // to a dword before the reload at 0082E97B, so the product is rounded once.
    const double multiplier = unit_rudder_denominator_0082e890(throttle_fraction, host);
    return store_float(multiplier * static_cast<double>(ship_class.turn_radius_0520));
}

float ship_ai_nav_block_stop_radius_009e4537(float max_speed_0500,
                                             float hull_length_09c8) noexcept {
    // 009E44ED..009E44FD: the speed term is stored to a dword before the
    // compare.  009E4501..009E450D: so is the hull term.
    const float speed_term =
        store_float(static_cast<double>(max_speed_0500) * kShipAiNavBlockStopSeconds);
    const float hull_term =
        store_float(static_cast<double>(hull_length_09c8) * kShipAiNavBlockStopHullShare);
    // 009E4519 FCOMIP(hull_term, speed_term) then 009E451D JBE to the speed
    // term.  An unordered compare sets CF and ZF, so JBE is taken: a NaN hull
    // length yields the speed term, never the NaN.
    return (hull_term > speed_term) ? hull_term : speed_term;
}

float ship_ai_nav_block_yaw_rate_009e45a9(float max_rot_angle_04f8) noexcept {
    // 009E458D FCOMIP(double floor, MaxRotAngle) then 009E4591 JBE to
    // 009E459D, which keeps MaxRotAngle.  Unordered takes the same branch, so
    // a NaN MaxRotAngle is stored unchanged.
    if (kShipAiNavBlockYawFloorCompare > static_cast<double>(max_rot_angle_04f8)) {
        return kShipAiNavBlockYawFloor;
    }
    return max_rot_angle_04f8;
}

float ship_ai_nav_block_look_ahead_009e4648(float twice_turn_circle_full) noexcept {
    // 009E4625 FCOMIP(double 250.0, value) then 009E4629 JBE to 009E4635,
    // which keeps the value.  Same unordered behaviour as above.
    if (kShipAiNavBlockLookAheadFloorCompare > static_cast<double>(twice_turn_circle_full)) {
        return kShipAiNavBlockLookAheadFloor;
    }
    return twice_turn_circle_full;
}

ShipAiNavBlockFields ship_ai_nav_block_ctor_009e4330(const ShipAiNavBlockUnitInputs& unit,
                                                     ShipAiNavBlockCtorHost& host) {
    ShipAiNavBlockFields fields{};

    // 009E4354..009E4379.  The two flag bytes are written before the memsets
    // that clear the ranges ending just under them, so they survive.  The
    // struct's member initialisers already carry both the cleared ranges and
    // the two flags.

    // 009E4381..009E43C4, the constant seeds.
    fields.deadline_158 = kShipAiNavBlockDeadlineSentinel;
    fields.deadline_15c = kShipAiNavBlockDeadlineSentinel;

    // 009E43CC, ECX = blk+1C4h.
    const ShipAiNavBlockSteeringDefaults defaults = host.seed_steering_009dfcb0();
    fields.turn_circle_full_3c8 = defaults.turn_circle_full_3c8;
    fields.turn_circle_cruise_3cc = defaults.turn_circle_cruise_3cc;
    fields.yaw_rate_3d0 = defaults.yaw_rate_3d0;
    fields.stop_radius_3d4 = defaults.stop_radius_3d4;
    fields.start_radius_3d8 = defaults.start_radius_3d8;
    fields.hull_scale_3e4 = defaults.hull_scale_3e4;

    // 009E43D5..009E43FF, the three bytes and the twelve sector records.  The
    // struct's member initialisers carry both.

    // 009E4401..009E4477, the three memo records, the A18h triple and A90h.
    fields.latch_a20 = kShipAiNavBlockLatchClear;

    // 009E447D, then the class test at 009E448E and the store at 009E44A2.
    fields.owner_3fc = unit.present ? unit.handle : 0u;
    const bool submarine = unit.present
        && host.unit_answers_class_5c(kShipAiNavBlockSubmarineClassId);
    fields.submarine_3f8 = submarine ? fields.owner_3fc : 0u;

    // 009E44A8..009E44B4.  Unconditional: see the header's uncertainty note.
    fields.class_reference_168 = unit.ship_class.reference_0570;

    // 009E44C4, throttle 1.0f; 009E44CF stores the result.
    fields.turn_circle_full_3c8 =
        host.class_turn_circle_radius_0082e960(kShipAiNavBlockThrottleFull);

    // 009E44D5..009E44E1.
    fields.hull_scale_3e4 = store_float(
        static_cast<double>(unit.hull_length_09c8) * kShipAiNavBlockHullScale);

    // 009E44E7..009E453F, the hysteresis pair.  The same 1.5 multiplies the
    // class MaxSpeed at 009E44F9 and the chosen radius at 009E4533.
    fields.stop_radius_3d4 = ship_ai_nav_block_stop_radius_009e4537(
        unit.ship_class.max_speed_0500, unit.hull_length_09c8);
    fields.start_radius_3d8 = store_float(
        kShipAiNavBlockStopSeconds * static_cast<double>(fields.stop_radius_3d4));

    // 009E4555, throttle 0.9f; 009E4568 stores the result without popping.
    fields.turn_circle_cruise_3cc =
        host.class_turn_circle_radius_0082e960(kShipAiNavBlockThrottleCruise);

    // 009E4574..009E45A9.
    fields.yaw_rate_3d0 = ship_ai_nav_block_yaw_rate_009e45a9(unit.ship_class.max_rot_angle_04f8);

    // 009E45A3..009E45BB, FADD ST0,ST0 then a dword store.
    const float twice_full = store_float(static_cast<double>(fields.turn_circle_full_3c8)
                                         + static_cast<double>(fields.turn_circle_full_3c8));
    fields.look_ahead_340 = twice_full;

    // 009E45C1..009E45D1.  The divisor stays in the x87 register, so it is not
    // rounded to float before the divide.  No zero guard.
    fields.shoulder_angle_1b4 = store_float(
        static_cast<double>(fields.hull_scale_3e4)
        / (static_cast<double>(fields.turn_circle_cruise_3cc) * kShipAiNavBlockStopSeconds));

    // 009E45D7..009E4606.  No domain guard on the square root.
    const double cruise = static_cast<double>(fields.turn_circle_cruise_3cc);
    const double hull = static_cast<double>(unit.hull_length_09c8);
    const float radicand =
        store_float(cruise * cruise - hull * hull * kShipAiNavBlockShoulderHullShare);
    fields.shoulder_offset_1b8 = host.sqrt_00bf7030(radicand);

    // 009E4600 and 009E460C, the owner again.
    fields.owner_260 = fields.owner_3fc;
    fields.owner_2c8 = fields.owner_3fc;

    // 009E4612..009E4648.
    fields.look_ahead_floor_318 = ship_ai_nav_block_look_ahead_009e4648(twice_full);

    // 009E4653 and 009E4659: the neighbour list starts empty.
    fields.value_400 = 0u;
    fields.neighbour_count_604 = 0;

    // 009E463B..009E4669, FLDZ and FLD1 pushed in that order, then FCHS.
    fields.random_phase_148 = -host.uniform_float_00bd2f10(0.0f, 1.0f);

    // 009E4673..009E46A3.
    fields.value_a84 = 0.0f;
    fields.value_a88 = 0.0f;
    fields.value_a8c = 0.0f;
    fields.plan_state_3f0 = kShipAiNavBlockPlanStateInitial;
    fields.flag_3ec = true;
    fields.flag_3f4 = true;
    fields.early_out_3f5 = false;

    // 009E46A9, the last call.  It rewrites blk+168h, blk+3C4h and the twelve
    // sector shapes, so it runs after everything above.
    host.build_sector_shapes_009e0270(fields, kShipAiNavBlockSectorBuildArgumentRaw);

    return fields;
}

} // namespace bsp
