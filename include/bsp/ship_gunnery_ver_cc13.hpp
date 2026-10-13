#pragma once
// Packet cc13-ver21: the AI ship surface gunnery terms where the image and the
// gunnery host part ways. Evidence: docs/CC13_VER_21.md. Every routine is pure
// and takes the fields the image reads; random draws come in as arguments, so
// nothing here touches 00BD2F10 (the image draws every term below from its one
// process-wide generator, stream index ECX = 1). Nothing here is wired into a
// host. The names are hypotheses, not recovered symbols.
//
// - 00864880 BSP_GunneryTargetRecord_Construct: the per-target ranging record's
//   seed, including the +-think-time velocity extrapolation of +50h and +60h.
// - 00862CD0 BSP_GunneryTargetRecord_Update: the error length rules, and the
//   bearing that 00862F62 never writes back.
// - 006DF833..006DF97D in 006DF520 BSP_GunBot_MuzzleSolutionAimTick: the
//   time-of-flight push-out, from the class's derived speed (+50h) and time
//   scale (+5Ch), with no vertical component.
// - 006E8592..006E85E4 in 006E8430: the artillery launch velocity's vertical bias.
// - 006E6750 / 006E6490: the flight step scaled by the class time scale.
// The artillery class's speed and time scale themselves are 006E98BC, already
// reconstructed as weapon_class_derive_engagement_range (bullet_engagement_range.hpp).
#include <cstdint>

namespace bsp {

inline constexpr std::uint32_t kRangingRecordConstructAddr = 0x00864880;
inline constexpr std::uint32_t kRangingRecordUpdateAddr = 0x00862CD0;
inline constexpr std::uint32_t kRangingBearingDiscardAddr = 0x00862F62;
inline constexpr std::uint32_t kArtilleryLeadPushOutAddr = 0x006DF833;
inline constexpr std::uint32_t kArtilleryLaunchBiasAddr = 0x006E85D2;
inline constexpr std::uint32_t kProjectileScaledStepAddr = 0x006E675A;

// 00CF9058 (double 9.8100004196167, the float 9.81f widened), 00CEB5A8 (float
// pi/4), 00D7A280 (double 0.5), 00D7A258 (double 0.0), 00D0DE84 (float 0.05,
// the fixed step).
inline constexpr double kShipGunneryGravity = 9.8100004196167;
inline constexpr float kShipGunneryQuarterPi = 0.7853981852531433f;
inline constexpr float kArtilleryLaunchBiasStep = 0.05000000074505806f;

// One ArtillerySubDirectorBot row, [00E19994] + 28h * level, at the offsets the
// two record routines read (robots.lua Hungarian names, docs/ARTILLERY_RANGING_ERROR.md).
struct ArtilleryRangingRow {
    float max_error_radius;  // +0Ch
    float error_range_mul;   // +10h
    float min_error_mul;     // +14h
    float approach_mul_min;  // +18h
    float approach_mul_max;  // +1Ch
    float deviation_mul;     // +20h
    float angle_change;      // +24h, radians
    float start_time;        // +28h, ErrorDistIncStartTime
    float full_time;         // +2Ch, ErrorDistIncFullTime
};

struct RangingVec3 {
    float x;
    float y;
    float z;
};

// The record fields both routines write.
struct ArtilleryRangingRecord {
    float error_length;       // +70h
    float bearing;            // +6Ch
    RangingVec3 offset;       // +74h..+7Ch, (0, 0, +70h) turned about the up axis by +6Ch
    RangingVec3 last_target;  // +50h..+58h, the translation row of the +20h matrix copy
    RangingVec3 predicted;    // +60h..+68h
    float t_update;           // +80h
    float t_shot;             // +84h
    bool fired_since;         // +88h
};

// 00864880. `distance` is |owner - target| (00864991's sqrt, 0 at or below
// 00CE3820); `error_u` is the draw U(err0 * MinErrorMul, err0) and `bearing_u`
// the draw U(0, 2pi) (00864A12, 00864A31). `think_time` is GlobalConfig+88h
// (WeaponDirectorThinkTime, 2 s in this installation) read at 00864A8D, and
// `target_velocity` is target->vtable[34h]() (00864AC0, 00864B0D). The +20h
// matrix copy's translation is moved BACK by velocity * think time
// (00864AE7..00864B02) and +60h is the target position moved FORWARD by it,
// with +64h then zeroed (00864B8A). The host's engagement_00864880 seeds both
// at the target position.
ArtilleryRangingRecord artillery_ranging_record_construct_00864880(
    const ArtilleryRangingRow& row, float distance, float error_u, float bearing_u,
    const RangingVec3& target_position, const RangingVec3& target_velocity,
    float think_time, float now) noexcept;

// The error arm of 00862CD0 (00862D44..00862FA7). `frac` is distance over the
// owner's +490h clamped to [0, 1]; `miss_length` is |+60h - target position|
// (00862E2F), used only on the fired-since arm; `angle_u` is the draw
// U(-AngleChange, AngleChange) at 00862F46, made whenever the error moved.
// The bearing is NOT changed: 00438AA0 is pure (RET 8, result in ST0) and
// 00862F62 pops its sum with FSTP ST0, so 00862F80 rotates by the old +6Ch.
// Returns true when the error moved (the draw was consumed). The extrapolation
// tail (00862FA9..008630EB) is not part of this fragment.
bool artillery_ranging_record_update_error_00862cd0(ArtilleryRangingRecord& rec,
    const ArtilleryRangingRow& row, float distance, float frac, float now,
    float miss_length, float angle_u) noexcept;

// (0, 0, length) turned about +Y by `bearing`. LABELLED: the sense of
// 0085C3F0's rotation was not re-read; the bearing is uniform over 2pi, so the
// sense does not change any distribution.
RangingVec3 artillery_ranging_offset(float length, float bearing) noexcept;

// 006DF833..006DF97D. `distance` is |aim point - gun+FCh|, `class_speed` the
// class +50h and `time_scale` its +5Ch (for artillery 006E98BC makes these
// sqrt(9.81 * Range) and V0 / that). Pitch = asin(distance * g / speed^2) / 2,
// or pi/4 when the ratio exceeds 1 (00CEB5A8); time of flight =
// distance / (speed * cos(pitch) * time_scale); the push-out is
// velocity * time with its Y term multiplied by 0.0 (00D7A258), so a target's
// vertical speed is never led.
struct ArtilleryLeadPushOut {
    float pitch;
    float flight_time;
    RangingVec3 push_out;
};
ArtilleryLeadPushOut artillery_lead_push_out_006df833(float distance, float class_speed,
    float time_scale, const RangingVec3& target_velocity) noexcept;

// 006E8592..006E85E4: launch velocity = direction * class +50h, and for class
// sub-types 4..7 (artillery) vy -= 0.05f * 9.81 (FSUBP of the single-precision
// product). The host passes sub-type 0 to projectile_launch_velocity_006e8430.
// Returns the vertical speed removed (0 for other sub-types).
float artillery_launch_vertical_bias_006e85d2(int class_sub_type) noexcept;

// 006E675A / 006E649B: the projectile's per-step dt is class +5Ch * step, for
// every class (1.0 except the artillery arm's V0 / sqrt(g * Range)).
float projectile_scaled_step_006e675a(float time_scale, float step) noexcept;

// The ground range the vertical bias costs on flat water: the shell's flight
// in the class frame is shortened by 2 * bias / g, so it lands
// speed * cos(pitch) * 2 * 0.05 short. A derived quantity, not an image routine.
float artillery_launch_bias_shortfall(float class_speed, float pitch) noexcept;

}  // namespace bsp
