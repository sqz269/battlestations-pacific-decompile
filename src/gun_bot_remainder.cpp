#include "bsp/gun_bot_remainder.hpp"

#include <cmath>

namespace bsp {
namespace {

float dot3(const std::array<float, 3>& a, const std::array<float, 3>& b) noexcept {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// 00419010 with the argument order ship_ai_attackmove_substates.hpp records:
// (x0, y0, x1, y1, x), clamped to the endpoints outside [x0, x1].
float interpolate_clamped(float x0, float y0, float x1, float y1, float x) noexcept {
    if (x <= x0) return y0;
    if (x >= x1) return y1;
    const float span = x1 - x0;
    if (span == 0.0f) return y1;
    return y0 + (y1 - y0) * ((x - x0) / span);
}

float horizontal_distance_squared(const std::array<float, 3>& a,
                                  const std::array<float, 3>& b) noexcept {
    const float dx = a[0] - b[0];
    const float dz = a[2] - b[2];
    return dx * dx + dz * dz;
}

} // namespace

// ---------------------------------------------------------------------------
// 008FB8D0 and 008FBB00
// ---------------------------------------------------------------------------
TorpedoInterceptRoots torpedo_intercept_time_008fb8d0(const std::array<float, 3>& shooter,
                                                      const std::array<float, 3>& target,
                                                      float speed,
                                                      const std::array<float, 3>& target_velocity) noexcept {
    TorpedoInterceptRoots roots;

    // 008FB8D3..008FB8EF: d is the SHOOTER minus the TARGET, not the other way.
    const std::array<float, 3> d = {shooter[0] - target[0], shooter[1] - target[1],
                                    shooter[2] - target[2]};

    // 008FB933..008FB97D.
    const float a = dot3(target_velocity, target_velocity) - speed * speed;
    const float b = dot3(d, target_velocity);
    const float c = dot3(d, d);

    // 008FB980..008FB998: the quadratic arm runs while |a| is outside +/- 1e-4.
    if (a >= kInterceptCoefficientEpsilon || a <= -kInterceptCoefficientEpsilon) {
        // 008FBA02: b * b - 4 * a * c. The native builds the discriminant for a
        // linear coefficient of b, while the true intercept needs 2 * b.
        const float discriminant = b * b - kInterceptDiscriminantFactor * a * c;
        if (discriminant < 0.0f) return roots; // 008FBA1F

        const float root = std::sqrt(discriminant); // 00BF7030
        const float denominator = a + a;            // 008FBA40

        if (a > 0.0f) {
            // 008FBA44: the far root first when the target outruns the torpedo.
            roots.first = (b + root) / denominator;
            if (roots.first < 0.0f) return roots; // 008FBA6D
            roots.second = (b - root) / denominator;
        } else {
            // 008FBA95: the one positive root of a downward parabola.
            roots.first = (b - root) / denominator;
            if (roots.first < 0.0f) return roots; // 008FBABE
            roots.second = (b + root) / denominator;
        }
        roots.root_count = (roots.second >= 0.0f) ? 2 : 1; // 008FBAE4
        return roots;
    }

    // 008FB99C..008FB9F7: the degenerate arm, again with the missing factor of
    // two. A negative time is reported as zero rather than as no solution.
    if (b >= kInterceptCoefficientEpsilon || b <= -kInterceptCoefficientEpsilon) {
        const float t = c / b;
        roots.root_count = 1;
        roots.first = (t >= 0.0f) ? t : 0.0f;
        return roots;
    }
    return roots;
}

bool torpedo_intercept_point_008fbb00(const std::array<float, 3>& shooter,
                                      const std::array<float, 3>& target,
                                      float speed,
                                      const std::array<float, 3>& target_velocity,
                                      std::array<float, 3>& out) noexcept {
    const TorpedoInterceptRoots roots =
        torpedo_intercept_time_008fb8d0(shooter, target, speed, target_velocity);
    if (roots.root_count == 0) return false; // 008FBB28

    // 008FBB9E takes the first root, 008FBB3C the second.
    const float t = (roots.root_count == 1) ? roots.first : roots.second;
    out[0] = target[0] + t * target_velocity[0];
    out[1] = target[1] + t * target_velocity[1];
    out[2] = target[2] + t * target_velocity[2];
    return true;
}

float torpedo_intercept_time_exact(const std::array<float, 3>& shooter,
                                   const std::array<float, 3>& target,
                                   float speed,
                                   const std::array<float, 3>& target_velocity) noexcept {
    const std::array<float, 3> d = {shooter[0] - target[0], shooter[1] - target[1],
                                    shooter[2] - target[2]};
    const float a = dot3(target_velocity, target_velocity) - speed * speed;
    const float b = dot3(d, target_velocity);
    const float c = dot3(d, d);

    if (a >= kInterceptCoefficientEpsilon || a <= -kInterceptCoefficientEpsilon) {
        const float discriminant = b * b - a * c; // the half-b form
        if (discriminant < 0.0f) return -1.0f;
        const float root = std::sqrt(discriminant);
        const float lo = (b - root) / a;
        const float hi = (b + root) / a;
        if (lo >= 0.0f && (hi < 0.0f || lo <= hi)) return lo;
        if (hi >= 0.0f) return hi;
        return -1.0f;
    }
    if (b >= kInterceptCoefficientEpsilon || b <= -kInterceptCoefficientEpsilon) {
        const float t = c / (b + b);
        return (t >= 0.0f) ? t : -1.0f;
    }
    return -1.0f;
}

// ---------------------------------------------------------------------------
// Entity vtable slot 100h
// ---------------------------------------------------------------------------
std::array<float, 3> entity_lead_point_0042d810() noexcept {
    // 0042D814: three zeroes through the out pointer, every argument ignored.
    return {0.0f, 0.0f, 0.0f};
}

float ship_lead_length_taper_00816941(float box_z_draw) noexcept {
    const float magnitude = std::fabs(box_z_draw); // 008168FB, the sign test
    return interpolate_clamped(kShipLeadHullTaperStart, kShipLeadHullTaperEnd,
                               kShipLeadHullTaperEnd, kShipLeadHullTaperFloor, magnitude);
}

std::array<float, 3> ship_lead_point_00816650(const ShipLeadSections& sections,
                                              const ShipHullExtents& hull,
                                              const std::array<float, 3>& box,
                                              const std::array<float, 3>& origin,
                                              float section_chance,
                                              float engine_room_weight,
                                              float magazine_weight,
                                              float fuel_tank_weight,
                                              const ShipLeadRandomDraws& draws,
                                              bool engine_room_available,
                                              bool magazine_available,
                                              bool fuel_tank_available) noexcept {
    // 00816659 and 00816687: a chance of zero or less, or a roll the chance does
    // not beat, goes straight to the hull box. AAGunnerBot passes -1.0 here, so
    // that class never reaches a named section.
    bool use_sections = section_chance > 0.0f && section_chance > draws.section_roll;

    float weight_engine = 0.0f;
    float weight_magazine = 0.0f;
    float weight_fuel = 0.0f;
    if (use_sections) {
        // 0081668F, 008166D1 and 0081671D: each weight survives only when it is
        // positive, the section's own byte is set and 0093A570 did NOT find its
        // id in the ship's section vector.
        if (engine_room_weight > 0.0f && sections.engine_room.present && engine_room_available)
            weight_engine = engine_room_weight;
        if (magazine_weight > 0.0f && sections.magazine.present && magazine_available)
            weight_magazine = magazine_weight;
        if (fuel_tank_weight > 0.0f && sections.fuel_tank.present && fuel_tank_available)
            weight_fuel = fuel_tank_weight;

        // 00816769: a total that does not clear 1e-4 falls through as well.
        const float total = weight_engine + weight_magazine + weight_fuel;
        if (total <= kInterceptCoefficientEpsilon) use_sections = false;
    }

    if (use_sections) {
        // 0081679F and 008167C7: the running sums are compared against a draw
        // over [0, total - 1e-4).
        const float first = weight_engine;
        const float second = weight_engine + weight_magazine;
        if (first > draws.pick) return sections.engine_room.point;  // 008167AB
        if (second > draws.pick) return sections.magazine.point;    // 008167D1
        return sections.fuel_tank.point;                            // 008167EB
    }

    // 00816820: a point inside the hull box of [ship+538h]. The x offset is
    // tapered by how far out the z draw landed, the y draw is one sided.
    const float taper = ship_lead_length_taper_00816941(draws.box_z);
    std::array<float, 3> out{};
    out[0] = origin[0] + taper * draws.box_x * (kShipLeadHalfExtentScale * hull.length);
    out[1] = origin[1] + draws.box_y * (kShipLeadVerticalExtentScale * hull.height);
    out[2] = origin[2] + draws.box_z * (kShipLeadHalfExtentScale * hull.width);
    (void)box; // the box is what the caller drew `draws` over
    return out;
}

// ---------------------------------------------------------------------------
// 008FC080
// ---------------------------------------------------------------------------
float depth_charge_sink_time_008fc23e(float owner_height, float target_height,
                                      float sink_rate) noexcept {
    // 008FC23E..008FC270: clamped at zero by 00415550 BSP_Math_MaxFloatByRef.
    const float t = (owner_height - target_height - kDepthChargeSinkDepthBias) / sink_rate;
    return (t > 0.0f) ? t : 0.0f;
}

bool depth_charge_in_range_008fc354(const DepthChargeRangeInputs& in) noexcept {
    // 008FC2F4..008FC31E: both distances drop the height component.
    const float now = horizontal_distance_squared(in.owner_position, in.target_position);
    const float predicted = horizontal_distance_squared(in.owner_position, in.predicted_position);
    const float nearer = (predicted < now) ? predicted : now; // 00415510

    // 008FC322: the radius is squared, then floored at 10000 by 00415550.
    const float radius_squared = in.attack_dist * in.attack_dist;
    const float effective = (radius_squared > kDepthChargeRadiusFloorSquared)
                                ? radius_squared
                                : kDepthChargeRadiusFloorSquared;
    return effective > nearer; // 008FC350, a strict compare
}

bool depth_charge_fire_byte_008fc386(const DepthChargeFireInputs& in) noexcept {
    if (!in.in_range) return false;              // 008FC354
    if (in.fire_delay > 0.0f) return false;      // 008FC35B
    if (in.has_muzzle && in.muzzle_inhibited) return false; // 008FC377, bit 3
    return true;
}

void depth_charge_bot_tick_008fc080(DepthChargeBotHost& host, DepthChargeBotState& state,
                                    float dt) {
    // 008FC08A..008FC0D2: the shared five-part target check, then the clear.
    if (host.resolve_fire_target_00521ea0() == nullptr ||
        !gun_bot_target_still_valid_008ffa20(host.target_validity())) {
        host.clear_fire_target_slot38();
    }

    host.run_idle_rest_timer_008fbce0(dt); // 008FC0E2

    // 008FC0E7..008FC10C: the fire delay is decremented here only while it is
    // still positive; the firing arm decrements it otherwise, so it moves by dt
    // exactly once a frame either way.
    const bool delay_was_positive = state.fire_delay > 0.0f;
    if (delay_was_positive) state.fire_delay -= dt;

    if (!host.side_enabled_00927f10()) return; // 008FC11E
    if (!host.gun_present()) return;           // 008FC12B
    if (host.fire_target_entity_slot44() == nullptr) return; // 008FC13C

    // 008FC146: a 0.1 s think period, reset only when it expires.
    state.think_delay -= dt;
    if (state.think_delay > 0.0f) return;
    state.think_delay = kDepthChargeThinkPeriod; // 008FC166

    const float sink_rate = host.weapon_sink_rate();               // 008FC17F
    const std::array<float, 3> owner = host.owner_world_position(); // 008FC1A1
    const std::array<float, 3> target = host.target_world_position(); // 008FC1E6

    // 008FC204: a target whose world height is above -2.0 is not submerged and
    // the tick releases the trigger through the tail jump at 008FC232.
    if (target[1] > kDepthChargeTargetDepthCeiling) {
        host.set_trigger_slot1e8(false);
        return;
    }

    // 008FC23E..008FC2AB: predict where the target will be when a charge
    // dropped now has sunk to it.
    const float sink_time = depth_charge_sink_time_008fc23e(owner[1], target[1], sink_rate);
    const std::array<float, 3> velocity = host.target_world_velocity_slot34();
    const std::array<float, 3> predicted = {target[0] + velocity[0] * sink_time,
                                           target[1] + velocity[1] * sink_time,
                                           target[2] + velocity[2] * sink_time};

    const DepthChargeBotLevel level = host.level_parameters(); // 008FC2BD

    DepthChargeRangeInputs range;
    range.owner_position = owner;
    range.target_position = target;
    range.predicted_position = predicted;
    range.attack_dist = level.attack_dist;

    DepthChargeFireInputs fire;
    fire.in_range = depth_charge_in_range_008fc354(range);
    fire.fire_delay = state.fire_delay;
    fire.has_muzzle = true;
    fire.muzzle_inhibited = host.muzzle_fire_inhibited(); // 008FC377
    const bool held = depth_charge_fire_byte_008fc386(fire);

    host.set_trigger_slot1e8(held); // 008FC39A
    if (!held) return;              // 008FC39C

    // 008FC3A0: the frame that fires is the frame the delay was not positive,
    // so the decrement the prologue skipped happens here instead.
    if (!delay_was_positive) state.fire_delay -= dt;

    // 008FC3B1: the burst ends once the delay has run below ContinuousFireTime,
    // and a fresh delay is drawn from the FireDelay pair.
    if (level.continuous_fire_time > state.fire_delay) {
        state.fire_delay = host.random_range_00bd2f10(level.fire_delay_min, level.fire_delay_max);
    }
}

// ---------------------------------------------------------------------------
// 00902920's fire byte
// ---------------------------------------------------------------------------
bool aa_gunner_fire_byte_00902920(const AAGunnerFireByteInputs& in) noexcept {
    if (!in.weapon_mount_present) return false;      // 00902F83
    if (!in.weapon_descriptor_present) return false; // 00902F97
    if (!in.aim_accepted) return false;              // 00902FB5

    // 00902FCD: the range gate is nine tenths of the descriptor's own maximum.
    if (in.max_range * kAAGunnerBotRangeFraction <= in.distance) return false;

    // 00902FE8..00903052: the SUM of the two absolute deltas must be under five
    // degrees, not each of them.
    const float delta_horz = std::fabs(in.commanded_horz - in.gun_horz);
    const float delta_vert = std::fabs(in.commanded_vert - in.gun_vert);
    if (kAAGunnerFireAngleSum <= delta_horz + delta_vert) return false;

    // 00903066: bit 0 of the muzzle's +634h word inhibits this class.
    if (in.has_muzzle && in.muzzle_inhibited) return false;
    return true;
}

// ---------------------------------------------------------------------------
// 004F3730 and 008527E0
// ---------------------------------------------------------------------------
SegmentCrossingXZ segment_crossing_004f3730(const std::array<float, 2>& a0,
                                            const std::array<float, 2>& a1,
                                            const std::array<float, 2>& b0,
                                            const std::array<float, 2>& b1) noexcept {
    SegmentCrossingXZ result;

    // 004F3781: 004F3630(ECX &a0, EDX &da, b0, &db, &t, &u) does the work. Its
    // denominator is db.x * da.y - db.y * da.x and the two quotients it writes
    // are the parameters along a0->a1 and b0->b1 respectively.
    const std::array<float, 2> da = {a1[0] - a0[0], a1[1] - a0[1]};
    const std::array<float, 2> db = {b1[0] - b0[0], b1[1] - b0[1]};

    const float denominator = da[0] * db[1] - da[1] * db[0];
    // 004F365A rejects a parallel pair through 004F3560(denominator, 0), a
    // relative near-equality test whose tolerance rule was not read; this
    // projection uses exact equality instead and is marked partial for it.
    if (denominator == 0.0f) return result;

    const std::array<float, 2> delta = {b0[0] - a0[0], b0[1] - a0[1]};
    const float t = (delta[0] * db[1] - delta[1] * db[0]) / denominator;
    const float u = (delta[0] * da[1] - delta[1] * da[0]) / denominator;

    // 004F3774..004F378E: both parameters must land in [0, 1].
    if (t < 0.0f || t > kSegmentCrossingParameterMax) return result;
    if (u < 0.0f || u > kSegmentCrossingParameterMax) return result;

    result.crossed = true;
    result.point[0] = a0[0] + (a1[0] - a0[0]) * t;
    result.point[1] = a0[1] + t * (a1[1] - a0[1]);
    return result;
}

bool submarine_is_shallow_008527e0(float entity_height, float depth_reference) noexcept {
    // 008527FA: [entity+1204h] - 3.0 < [entity+100h].
    return depth_reference - kSubmarineSubmergedBias < entity_height;
}

} // namespace bsp
