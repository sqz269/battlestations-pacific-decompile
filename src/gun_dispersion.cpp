// Gun dispersion. See include/bsp/gun_dispersion.hpp and docs/GUN_DISPERSION.md.
//
// Every routine mirrors the native rounding: the x87 bodies round to float32 at
// each FSTP to a `float ptr` slot, so each such store is an explicit narrowing
// here. The one place this cannot be exact is x87's 80-bit working precision,
// which Win32 MSVC cannot express; doubles are used instead.
#include "bsp/gun_dispersion.hpp"

#include <cmath>
#include <cstddef>

#include "bsp/unit_rudder.hpp"  // wrapped_angle_add_00438aa0

namespace bsp {
namespace {

// Every `FSTP float ptr` in the native listings.
float narrow(double value) noexcept { return static_cast<float>(value); }

// 00CE3D9C, the 2*pi both producers draw their roll angle from.
constexpr float kTwoPi = 6.2831855f;
// 00CE3854 and 00CE3918, the reroll period bounds.
constexpr float kAimErrorPeriodMin = 3.0f;
constexpr float kAimErrorPeriodMax = 8.0f;
// 00D7A280, the halving constant of the torpedo fan.
constexpr double kHalf = 0.5;

}  // namespace

float gun_dispersion_uniform_00bd2e60(RandomState& state, float low,
                                      float high) noexcept {
    // 00BD2E91..00BD2EAF: FILD of the signed word, +2^32 (00CE3978) when it
    // reads negative, times 2^-32 (00D63B80), then FSTP to a float slot.
    const std::uint32_t word = random_next_u32_00ba2c20(state);
    const float unit = narrow(static_cast<double>(word) * 0x1p-32);
    // 00BD2EAF..00BD2EC6: lo + u * (hi - lo), one FSTP float at the end.
    const double lo = low;
    return narrow(lo + static_cast<double>(unit) *
                           (static_cast<double>(high) - lo));
}

float gun_throw_magnitude_0073031d(const GunThrowInputs& inputs) noexcept {
    // 007302DF: the magnitude starts as the authored `Throw`.
    float magnitude = inputs.authored_throw;
    // 007302F0..00730317: weapon Function 1, 5 or 6 under GameSettings+760h.
    if (inputs.aa_function && inputs.turn_off_aa_gun_throw) {
        magnitude = 0.0f;
    }
    // 00730498: FMUL by the seat bot's BulletThrowMul, then FSTP float.
    return narrow(static_cast<double>(inputs.bot_throw_multiplier) *
                  static_cast<double>(magnitude));
}

GunThrowPolar gun_throw_draw_00730540(RandomState& state,
                                      float magnitude) noexcept {
    GunThrowPolar polar;
    // 00730540..0073055C: the roll angle first.
    polar.angle = gun_dispersion_uniform_00bd2e60(state, 0.0f, kTwoPi);
    // 00730562..0073057A: the radial fraction, kept as a double by the native.
    const double fraction = gun_dispersion_uniform_00bd2e60(state, 0.0f, 1.0f);
    // 00730586: BSP_Math_TangentX87Float 00412E20, FSINCOS then FDIVP, with a
    // float32 spill and reload around the result.
    const float tangent = narrow(std::tan(static_cast<double>(magnitude)));
    // 0073058B..0073059F: FMUL then FSTP float.
    polar.radius = narrow(static_cast<double>(tangent) * fraction);
    return polar;
}

GunThrowPolar gun_throw_unit_scale_007305d6(const GunThrowPolar& in,
                                            float unit_multiplier) noexcept {
    GunThrowPolar out = in;
    out.radius = narrow(static_cast<double>(in.radius) *
                        static_cast<double>(unit_multiplier));
    return out;
}

GunThrowPolar gun_throw_scale_0073062c(const GunThrowPolar& in,
                                       float scale) noexcept {
    GunThrowPolar out;
    const double factor = scale;
    // 0073062C and 00730646: the angle is scaled as well as the radius.
    out.angle = narrow(static_cast<double>(in.angle) * factor);
    out.radius = narrow(static_cast<double>(in.radius) * factor);
    return out;
}

std::array<float, 3> gun_throw_apply_00730654(
    const std::array<float, 3>& forward, const std::array<float, 3>& axis_cos,
    const std::array<float, 3>& axis_sin, const GunThrowPolar& polar) noexcept {
    std::array<float, 3> out = forward;
    // 00730658 and 007306E6: FCOS and FSIN, each spilled to a float slot.
    const float cosine = narrow(std::cos(static_cast<double>(polar.angle)));
    const float sine = narrow(std::sin(static_cast<double>(polar.angle)));
    const double radius = polar.radius;
    for (std::size_t i = 0; i < 3; ++i) {
        // 0073065E..007306DE: axis_cos scaled by cos, then by the radius, then
        // accumulated, with an FSTP float at every step.
        const float scaled_cos = narrow(static_cast<double>(cosine) *
                                        static_cast<double>(axis_cos[i]));
        const float term_cos = narrow(static_cast<double>(scaled_cos) * radius);
        out[i] = narrow(static_cast<double>(term_cos) +
                        static_cast<double>(out[i]));
    }
    for (std::size_t i = 0; i < 3; ++i) {
        // 007306EC..0073075E: the same for axis_sin scaled by sin.
        const float scaled_sin = narrow(static_cast<double>(sine) *
                                        static_cast<double>(axis_sin[i]));
        const float term_sin = narrow(static_cast<double>(scaled_sin) * radius);
        out[i] = narrow(static_cast<double>(term_sin) +
                        static_cast<double>(out[i]));
    }
    return out;
}

float gun_torpedo_fan_offset_007304a0(float magnitude, int tube_count,
                                      int index) noexcept {
    // 007304BC: fewer than two tubes zeroes the magnitude outright.
    if (tube_count < 2) {
        return 0.0f;
    }
    // 007304D2..007304EA: the magnitude is widened to a double, divided by
    // tube_count - 1 and multiplied by the tube index.
    const double spread = static_cast<double>(magnitude) /
                          static_cast<double>(tube_count - 1) *
                          static_cast<double>(index);
    // 007304F0..007304FE: minus half the magnitude, then FSTP float.
    return narrow(spread - static_cast<double>(magnitude) * kHalf);
}

std::array<float, 3> gun_multi_bullet_ring_0072fefb(
    const std::array<float, 3>& direction, const std::array<float, 3>& axis_cos,
    const std::array<float, 3>& axis_sin, float cone_angle, int bullet_count,
    int bullet_index) noexcept {
    std::array<float, 3> out = direction;
    if (bullet_count <= 0) {
        return out;
    }
    // 0072FEFB..0072FF11: FILD index, FMUL 2*pi (00CE3828, a double), FIDIV by
    // the count, FSTP float.
    const float angle = narrow(static_cast<double>(bullet_index) *
                               6.283185307179586 /
                               static_cast<double>(bullet_count));
    const float sine = narrow(std::sin(static_cast<double>(angle)));
    const float cosine = narrow(std::cos(static_cast<double>(angle)));
    const double radius = cone_angle;
    for (std::size_t i = 0; i < 3; ++i) {
        const float scaled_cos = narrow(static_cast<double>(cosine) *
                                        static_cast<double>(axis_cos[i]));
        const float term_cos = narrow(static_cast<double>(scaled_cos) * radius);
        const float scaled_sin = narrow(static_cast<double>(sine) *
                                        static_cast<double>(axis_sin[i]));
        const float term_sin = narrow(static_cast<double>(scaled_sin) * radius);
        // 0072FFD9..00730019: the cos term joins the direction first.
        const float partial = narrow(static_cast<double>(out[i]) +
                                     static_cast<double>(term_cos));
        out[i] = narrow(static_cast<double>(partial) +
                        static_cast<double>(term_sin));
    }
    return out;
}

GunAimAngles gun_bot_aim_error_draw_006deff0(
    RandomState& state, const ArtilleryGunnerAimErrorLevel& level) noexcept {
    // 006DEFF3..006DF00E: the exponent's base.
    const float base = gun_dispersion_uniform_00bd2e60(state, 0.0f, 1.0f);
    // 006DF030..006DF043: an exact zero short-circuits the power to zero; the
    // 006DF053 negative-base fixup cannot fire for a draw in [0, 1).
    float weight = 0.0f;
    if (base != 0.0f) {
        // 006DF065..006DF085: FYL2X then F2XM1/FSCALE, that is base^Power.
        weight = narrow(std::exp2(static_cast<double>(level.power) *
                                  std::log2(static_cast<double>(base))));
    }
    // 006DF08B..006DF0BD: U(0, MaxAngleError) scaled by the weight.
    const float span =
        gun_dispersion_uniform_00bd2e60(state, 0.0f, level.max_angle_error);
    const float radius =
        narrow(static_cast<double>(span) * static_cast<double>(weight));
    // 006DF0CD..006DF0E1: the roll angle.
    const float roll = gun_dispersion_uniform_00bd2e60(state, 0.0f, kTwoPi);
    // 006DF0FD..006DF124: sine into horz, cosine into vert, each through a
    // float32 spill before the multiply.
    const float sine = narrow(std::sin(static_cast<double>(roll)));
    const float cosine = narrow(std::cos(static_cast<double>(roll)));
    GunAimAngles pair;
    pair.horz = narrow(static_cast<double>(sine) * static_cast<double>(radius));
    pair.vert = narrow(static_cast<double>(cosine) * static_cast<double>(radius));
    return pair;
}

float gun_bot_aim_error_period_006df5c6(RandomState& state) noexcept {
    return gun_dispersion_uniform_00bd2e60(state, kAimErrorPeriodMin,
                                           kAimErrorPeriodMax);
}

void gun_bot_aim_error_tick_006df5a0(ArtilleryGunnerAimErrorState& state,
                                     RandomState& rng,
                                     const ArtilleryGunnerAimErrorLevel& level,
                                     float dt) noexcept {
    // 006DF59F..006DF5AE: the countdown falls by dt, through a float slot.
    state.countdown = narrow(static_cast<double>(state.countdown) -
                             static_cast<double>(dt));
    // 006DF5B1..006DF5B7: a countdown still at or above zero keeps the pair.
    if (state.countdown >= 0.0f) {
        return;
    }
    // 006DF5B9..006DF5E6: one period serves as both the reset and the span.
    const float period = gun_bot_aim_error_period_006df5c6(rng);
    state.period = period;
    state.countdown = period;
    // 006DF0E5..006DF0FA: the old target becomes the previous pair and the
    // current pair is re-seeded from it before the fresh draw lands.
    state.previous = state.target;
    state.current = state.previous;
    state.target = gun_bot_aim_error_draw_006deff0(rng, level);
}

GunAimAngles gun_bot_aim_error_apply_006dfb0b(
    const GunAimAngles& solved, const GunAimAngles& error) noexcept {
    GunAimAngles out;
    out.horz = wrapped_angle_add_00438aa0(solved.horz, error.horz);
    out.vert = wrapped_angle_add_00438aa0(solved.vert, error.vert);
    return out;
}

}  // namespace bsp
