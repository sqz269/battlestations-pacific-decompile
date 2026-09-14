#pragma once
// Gun dispersion: the two producers of miss that sit between a gun's aim and a
// shell's flight path. Addresses: 00730160, 00BD2E60, 00412E20, 0072F830,
// 006DEFF0, 006DF520, 00901610, 006DEE00, 008FB4E0. docs/GUN_DISPERSION.md.
//
// Every rule here is pure: the RNG is an explicit bsp::RandomState, the authored
// values are explicit arguments, and nothing reaches a host or a global.
#include <array>
#include <cstdint>

#include "bsp/bot_fire_target.hpp"  // GunAimAngles
#include "bsp/random.hpp"           // RandomState, random_next_u32_00ba2c20

namespace bsp {

// ---------------------------------------------------------------------------
// The shared draw
// ---------------------------------------------------------------------------
// 00BD2F10 -> 00BD2ED0 -> 00BD2E60. 00BD2ED0 picks the MT19937 state for the
// caller's ECX stream index and the current thread; both producers below pass
// stream 1. 00BD2E60 is `lo + u * (hi - lo)` with `u = (uint32)next / 2^32`.
// The native intermediates are x87 80-bit; this uses double, so the final
// rounding can differ by one ULP in cases a double cannot separate.
float gun_dispersion_uniform_00bd2e60(RandomState& state, float low,
                                      float high) noexcept;

// ---------------------------------------------------------------------------
// Producer 1: the per-shot throw cone, BSP_Gun_Fire 00730160
// ---------------------------------------------------------------------------

// The fields the throw magnitude is built from. `authored_throw` is the Lua
// `Throw` at `gun[+3F8h]+4h`, written by the fire-parameter reader 007313E0;
// it is a half-angle in radians (0.01 on the 133 mm turret).
struct GunThrowInputs {
    float authored_throw = 0.0f;
    // gunclass[+80h] (the weapon Function) is 1, 5 or 6.
    bool aa_function = false;
    // GameSettings+760h, the Lua global `TurnOffAAGunThrow`.
    bool turn_off_aa_gun_throw = false;
    // bot->vtable[24h](), the seat bot's authored `BulletThrowMul`. 1.0f when
    // no AI role holds the seat: the native then never reaches the FMUL.
    float bot_throw_multiplier = 1.0f;
};

// 0073031D..0073049C. The result is the cone half-angle in radians; a value at
// or below zero suppresses the whole draw (the `COMISS`/`JBE` at 00730525).
float gun_throw_magnitude_0073031d(const GunThrowInputs& inputs) noexcept;

// gun+400h `throwA` and gun+404h `throwB`, the two values the gun keeps and
// replicates. `angle` is the roll about the barrel axis in radians; `radius`
// is a tangent, so the off-axis angle it produces is `atan(radius)`.
struct GunThrowPolar {
    float angle = 0.0f;
    float radius = 0.0f;
};

// 00730540..0073058F, reached only when `useExplicitThrow == 0`:
//   angle  = U(0, 2*pi)
//   radius = tan(magnitude) * U(0, 1)
// Two draws from stream 1, angle first. The radius is uniform in radius, not
// in area, so the pattern is centre-weighted.
GunThrowPolar gun_throw_draw_00730540(RandomState& state,
                                      float magnitude) noexcept;

// 007305D6. For bullet sub-types 4, 5, 6 and 7 the radius alone is scaled by
// the owning unit's float at `unit+63Ch`. Not reached on the explicit-throw
// path, which jumps past it at 00730606.
GunThrowPolar gun_throw_unit_scale_007305d6(const GunThrowPolar& in,
                                            float unit_multiplier) noexcept;

// 0073060A..0073064A. `scale` is 00470440(7, unit), the gameplay-modifier
// product for category 7; it is 1.0f whenever no modifier record is registered.
// Both members are scaled, the angle included.
GunThrowPolar gun_throw_scale_0073062c(const GunThrowPolar& in,
                                       float scale) noexcept;

// 00730654..0073075E. `forward`, `axis_cos` and `axis_sin` are rows 2, 1 and 0
// of the barrel node's world matrix (`gun[+3CCh]+0F0h`). The result is
// `forward + radius * (cos(angle) * axis_cos + sin(angle) * axis_sin)` and is
// deliberately not renormalised: adding a tangent to a unit forward is what
// turns `radius` into an angle.
std::array<float, 3> gun_throw_apply_00730654(
    const std::array<float, 3>& forward, const std::array<float, 3>& axis_cos,
    const std::array<float, 3>& axis_sin, const GunThrowPolar& polar) noexcept;

// 007304A0..007304FE, bullet sub-type 0Ah only. A deterministic fan, not a
// draw: tube `index` of `tube_count` is offset by
// `magnitude * index / (tube_count - 1) - magnitude / 2`. Fewer than two tubes
// gives zero. The offset rotates the direction about the matrix's row-1 axis
// through 0085C3F0, and the random cone is skipped entirely.
float gun_torpedo_fan_offset_007304a0(float magnitude, int tube_count,
                                      int index) noexcept;

// 0072FEFB..00730035, inside BSP_Gun_SpawnShotAndEffects 0072F830. This is the
// multi-bullet ring, not dispersion: `bullet_index` of `bullet_count` sits at
// `2*pi*index/count` on a ring of radius `cone_angle`, with no RNG at all.
// `cone_angle` is the gun class `MultiBulletConeAngle` at `gunclass+0D0h` and
// `bullet_count` its `OneTimeBulletAmount` at `gunclass+0CCh`.
std::array<float, 3> gun_multi_bullet_ring_0072fefb(
    const std::array<float, 3>& direction, const std::array<float, 3>& axis_cos,
    const std::array<float, 3>& axis_sin, float cone_angle, int bullet_count,
    int bullet_index) noexcept;

// ---------------------------------------------------------------------------
// Producer 2: the gun bot's aim error pair, 006DEFF0
// ---------------------------------------------------------------------------

// Two floats of one ArtilleryGunnerBot skill level. Layout from the producer
// `read_ArtilleryGunnerBot_parameters_008FD370` (docs/ROBOT_CONFIG.md): the
// config is a 0Ch header followed by six 1Ch-byte levels, so level L's fields
// are at `config + 1Ch*L + 0Ch` and `+ 10h`.
struct ArtilleryGunnerAimErrorLevel {
    float max_angle_error = 0.0f;  // +0Ch, Lua `MaxAngleError`
    float power = 0.0f;            // +10h, Lua `Power`
};

// 006DEFF0, the reroll. Three draws from stream 1, in this order:
//   t   = U(0, 1)
//   r   = U(0, MaxAngleError) * t^Power        (t == 0 gives an exact 0)
//   phi = U(0, 2*pi)
// and the pair is `{horz = r*sin(phi), vert = r*cos(phi)}` in radians. Power
// above 1 pulls the magnitude towards zero; below 1 pushes it out.
GunAimAngles gun_bot_aim_error_draw_006deff0(
    RandomState& state, const ArtilleryGunnerAimErrorLevel& level) noexcept;

// 006DF5C6. The countdown the reroll installs: `U(3.0f, 8.0f)` seconds, a pair
// of hard constants at 00CE3854 and 00CE3918, not an authored field.
float gun_bot_aim_error_period_006df5c6(RandomState& state) noexcept;

// 006DEFF0's own tail, 006DF0E5..006DF124. The state the tick interpolates
// between: `target` is the freshly drawn pair (bot+6Ch/+70h) and `previous`
// the one it replaces (bot+64h/+68h).
struct ArtilleryGunnerAimErrorState {
    GunAimAngles previous{};
    GunAimAngles target{};
    GunAimAngles current{};
    float period = 0.0f;     // bot+78h
    float countdown = 0.0f;  // bot+74h
};

// 006DF59F..006DF5E9 plus 006DEFF0. Subtracts `dt`, and when the countdown has
// gone negative rerolls the pair, the period and the countdown together. The
// interpolation itself is gun_bot_muzzle_error_006df520 in bsp/gun_bot_ticks.hpp.
void gun_bot_aim_error_tick_006df5a0(ArtilleryGunnerAimErrorState& state,
                                     RandomState& rng,
                                     const ArtilleryGunnerAimErrorLevel& level,
                                     float dt) noexcept;

// 006DFB0B..006DFB54. The pair is added to the angles the gravity arc solved
// and each sum is wrapped into (-pi, pi] by BSP_Math_AddWrappedAngle 00438AA0
// before BSP_TurningGun_SetTargetAngles 0085ABA0 sees it.
GunAimAngles gun_bot_aim_error_apply_006dfb0b(const GunAimAngles& solved,
                                              const GunAimAngles& error) noexcept;

}  // namespace bsp
