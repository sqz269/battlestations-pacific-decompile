// Packet cc_cruise_speed_setting. See docs/CRUISE_SPEED_SETTING.md for the
// addresses, the evidence and the uncertainty behind every rule here.
//
// Names are hypotheses, not recovered symbols.
#include "bsp/cruise_speed_setting.hpp"

namespace bsp {
namespace {

// The native reads the record's value slot twice over, once as an int and once
// as a float, and picks with a single compare against zero at 0082359C. Types
// other than Int take the float load, which is the fall-through at 008235A5,
// not a check for ScenePropertyType::Float.
float record_value(const SceneStartSpeedProperty& record) noexcept
{
    if (record.type == ScenePropertyType::Int) {
        // 0082359E CVTSI2SS XMM0,dword ptr [EAX + 0xc]
        return static_cast<float>(record.value_int);
    }
    // 008235A5 MOVSS XMM0,dword ptr [EAX + 0xc]
    return record.value_float;
}

// 008235BF FDIVR double ptr [ESP+20h] then 008235CA FSTP float ptr [ESP+18h].
// The numerator reaches the x87 as a double (008235B0 FLD float / 008235B6 FSTP
// double) and the divisor is 0080FC30's float32 result; the quotient is rounded
// back to float32 on the store. An x87 mantissa of 53 or 64 bits is wide enough
// that this is the correctly rounded float32 quotient, so a double divide and a
// single narrowing cast reproduce it.
float divide_to_float32(float numerator, float denominator) noexcept
{
    return static_cast<float>(static_cast<double>(numerator) / static_cast<double>(denominator));
}

// 008235E1 FMUL float ptr [ESP+14h] then 008235EC FSTP float ptr [ESP+18h], the
// same shape in the other direction.
float multiply_to_float32(float lhs, float rhs) noexcept
{
    return static_cast<float>(static_cast<double>(lhs) * static_cast<double>(rhs));
}

} // namespace

float scene_start_speed_value_00823599(const SceneStartSpeedProperty& record) noexcept
{
    if (!record.present) {
        return 0.0f;
    }
    return record_value(record);
}

SceneStartSpeedSeed scene_start_speed_seed_008235b0(float start_speed,
                                                    float reference_speed,
                                                    float reference_speed_second) noexcept
{
    SceneStartSpeedSeed seed{};
    seed.authored = true;
    seed.start_speed = start_speed;
    seed.reference_speed = reference_speed;
    seed.ring_throttle = divide_to_float32(start_speed, reference_speed);
    // The second value is the ratio multiplied back up by the reference speed,
    // not the authored speed: 008235C3's PUSH shifts ESP by four, so 008235CA
    // stores the ratio into the slot 008235AA had written and 008235E1 reads it
    // back from there after 0080D9B0's RET 4 restores ESP.
    seed.axial_speed = multiply_to_float32(reference_speed_second, seed.ring_throttle);
    return seed;
}

SceneStartSpeedSeed run_start_speed_arm_0082356c(CruiseSpeedSettingHost& host,
                                                 std::uint32_t unit,
                                                 std::uint32_t bag,
                                                 int bag_kind) noexcept
{
    SceneStartSpeedSeed seed{};
    if (bag_kind != kSceneEntityBagRefKindPropertyBag) {
        // 0082353D takes this arm only for kind 1; kind 2 goes to 00823548 and
        // anything else jumps clear of the whole block at 00823542.
        return seed;
    }

    // 0082356C..0082357F; the call itself is 00823576.
    seed.shipyard_launch = host.find_shipyard_launch_00823576(bag, kSceneUnitShipYardLaunchKey);

    // 00823582..00823597.
    const SceneStartSpeedProperty record =
        host.find_start_speed_00823590(bag, kSceneUnitStartSpeedKey);
    if (!record.present) {
        // 00823597 JZ 008235FC: the whole seed is skipped, both 0080FC30 calls
        // included.
        return seed;
    }

    const float start_speed = record_value(record);

    // 008235BA and 008235DC are two separate calls to the same callee.
    const float reference_speed = host.unit_reference_speed_0080fc30(unit);
    const float ring_throttle = divide_to_float32(start_speed, reference_speed);

    // 008235D5 CALL 0080D9B0, this = unit+838h.
    host.set_order_ring_throttle_0080d9b0(unit, ring_throttle);

    const float reference_speed_second = host.unit_reference_speed_0080fc30(unit);
    const float axial_speed = multiply_to_float32(reference_speed_second, ring_throttle);

    // 008235F7 CALL 0092D770, this = [unit+1018h].
    host.set_controller_axial_speed_0092d770(unit, axial_speed);

    seed.authored = true;
    seed.start_speed = start_speed;
    seed.reference_speed = reference_speed;
    seed.ring_throttle = ring_throttle;
    seed.axial_speed = axial_speed;
    return seed;
}

CruiseSpeedSettingOutcome scene_cruise_ship_speed(const SceneStartSpeedProperty& record,
                                                  UnitOrderRing& ring,
                                                  float reference_speed,
                                                  float heading_radians,
                                                  float body_axis_speed,
                                                  const CruiseSpeedSetting& speed_setting) noexcept
{
    CruiseSpeedSettingOutcome out{};

    if (record.present) {
        out.seed = scene_start_speed_seed_008235b0(record_value(record), reference_speed,
                                                   reference_speed);
        // 008235D5: the seed reaches the ring through the immediate setter, which
        // fills the pending span and writes the live throttle at +148h.
        set_unit_order_ring_param_a_0080d9b0(ring, out.seed.ring_throttle);
    }

    // 00835E17..00835E58, when the scene's queued `Cruise` becomes current. The
    // ring's rudder was never seeded, so |rudder| < 0.01f and the latch keeps the
    // heading rather than a rudder.
    out.latched = cruise_command_begin_00835e17(ring, heading_radians);

    // 009E1265..009E13B1, every AI step.
    out.ordered = cruise_ordered_values_009e1170(out.latched, speed_setting, reference_speed,
                                                 body_axis_speed);
    return out;
}

} // namespace bsp
