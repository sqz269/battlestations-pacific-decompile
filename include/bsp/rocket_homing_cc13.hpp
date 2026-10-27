// The rocket's air step 0080A000 and its homing branch (packet cc13-homing1,
// docs/CC13_ROCKET_HOMING.md). Names are hypotheses, not recovered symbols.
//
// 0080A000 is MRocket's vtable+19Ch slot (00D09284 in the table at 00D090E8),
// the air-mode velocity step that the bomb-family tick 006E1300 calls with the
// class-scaled fixed step (docs/BOMB_FAMILY_TICK.md). __thiscall(record, float
// dt), RET 4. This file reconstructs its motion part, 0080A000-0080A559:
//   - the ignition timer and its gravity bookkeeping (0080A024-0080A148);
//   - the thrust along the world forward row (0080A14E-0080A250);
//   - the homing branch (0080A25C-0080A559), which no host ran before.
// NOT reconstructed: the proximity fuze 0080A55D onwards (class+E4h or
// [00F874FD], against IsKindOf(0Fh) in the recon slot's list, radius class+70h),
// and the ignition notice record+310h->vtable[3Ch] (effects).
//
// The homing block is written only by 0080AA20 (BSP_Rocket_SetHomingTarget),
// whose two callers are the plane AI: the rocket task's aim tick 007B56F6
// (robots.lua StrikeHomingAngle, row+114h) and the opportunistic rocket
// sub-object 009FD4FB (Rocket_SmallPlane/LargePlane/Ship/Landfort HomingAngle,
// row+174h/1A8h/1DCh/210h). A byte scan of .text for E8/E9 rel32 and of every
// section for the absolute address finds exactly those two (the scan finds
// 0080A000's own vtable entry, so the pattern is known to occur). A rocket a
// ship's or a gun's fire spawns therefore never homes: the constructor at
// 0080ABB5 zeroes +47Ch.
#pragma once

#include <array>
#include <cstddef>

#include "bsp/camera_projection.hpp"

namespace bsp {

// ON by mechanism (GUNNERY 202, IJN23 pair): the gunnery host's rocket spawn fills the
// homing block; the glue is docs/CC13_ROCKET_HOMING.md section 6 as landed.
inline constexpr bool kRocketHomingBound = true;

namespace rocket_homing {
// The homing angle must be strictly positive (0080A272 COMISS, float).
inline constexpr float kAngleFloor = 0.0f;  // 00D7A218
// Both forward ranges must exceed it (0080A2DF, 0080A39B FLD qword).
inline constexpr double kMinForwardRange = 50.0;  // 00CE3938
// The lateral divisor is half the larger of range and VMax (0080A3C7 FMUL qword).
inline constexpr double kDivisorHalf = 0.5;  // 00D7A280
// The per-step cap is 0.05 x HomingAngle (0080A3E2 FLD dword 3D4CCCCDh). It is the
// tick element's fixed step written as a literal, not dt.
inline constexpr float kStepCapScale = 0.05f;  // 00D0DE84
// The low bound is -0.0f - cap (0080A3E8 MOVSS dword 80000000h).
inline constexpr float kNegativeZero = -0.0f;  // 00D7A208
// The float 9.81 widened (0080A12A FLD qword).
inline constexpr double kGravity = 9.8100004196167;  // 00CF9058
}  // namespace rocket_homing

// record+468h, the block 0080AA20 fills. The image keeps an observer-registered
// pointer at +47Ch; the observer callback 004C2D90 (block vtable 00CE75CC slot 1)
// nulls it when the target notifies, which is the only loss of lock. The host
// keeps a one-based unit index and must clear it the same way.
struct RocketHomingBlock {
    std::size_t target_plus_one{0};          // +47Ch (+14h of the block)
    std::array<float, 3> body_point{};       // +480h..+488h, in the target's frame
    float angle{0.0f};                       // +48Ch, radians (robots.lua DEG())
};

// The record fields 0080A000 reads and writes.
struct RocketFlight {
    // +74h, the local pose: rows right, up, forward, translation (4x4 row-major,
    // the layout 0042D5E0 and 0085DC80 address). A released rocket has no parent,
    // so the host treats it as the world matrix +CCh as well (labelled).
    CameraMatrix pose{};
    std::array<float, 3> velocity{};         // +318h..+320h
    float ignition_timer_490{0.0f};          // +490h
    bool ignited_494{false};                 // +494h
    RocketHomingBlock homing{};              // +468h block
};

// [record+314h] class fields.
struct RocketClassParams {
    float ignition_delay_e0{0.0f};           // "IgnitionDelay"
    float vmax_d8{0.0f};                     // "VMax"
    float accel_dc{0.0f};                    // "Acceleration"
};

// What 0080A000 asks of the homing target.
struct RocketHomingTargetView {
    CameraMatrix world{};                    // target+CCh (refreshed by 00414DB0)
    std::array<float, 3> velocity{};         // target->vtable[34h]()
};

enum class RocketHomingExit {
    kNotIgnited,        // 0080A155: +494h still clear
    kNoTarget,          // 0080A264: +47Ch null
    kAngleNotPositive,  // 0080A279: +48Ch <= 0.0f
    kBodyPointNear,     // 0080A311: the body point's local z <= 50
    kLeadPointNear,     // 0080A3B4: the lead point's local z <= 50
    kSteered,           // 0080A501..0080A553 ran
};

struct RocketAirStepResult {
    bool ignited_this_step{false};           // 0080A0EF set +494h (and fired vt[3Ch])
    float axial_speed{0.0f};                 // [ESP+24h] after the thrust block
    RocketHomingExit homing{RocketHomingExit::kNotIgnited};
    float flight_time{0.0f};                 // 00809E10 on the body point's range
    std::array<float, 2> steer{};            // the clamped lateral terms (x', y')
};

// 0080A024..0080A148: the timer and gravity bookkeeping. Before ignition the
// timer accumulates and gravity acts; after ignition the timer drains and the
// same gravity is given back, so a lit rocket recovers the vertical speed it
// lost while falling. Returns true when +494h was set on this step.
bool rocket_ignition_step_0080a024(RocketFlight& r, const RocketClassParams& c, float dt) noexcept;

// 0080A15B..0080A250: thrust along the WORLD forward row (+ECh). Returns the
// axial speed [ESP+24h] the homing branch uses.
float rocket_thrust_step_0080a15b(RocketFlight& r, const RocketClassParams& c, float dt) noexcept;

// 0080A25C..0080A559: the homing branch. `target` is null when +47Ch is null.
// `axial_speed` is the thrust block's output. Writes the forward row (then
// 0085DC80) and the velocity.
RocketHomingExit rocket_homing_step_0080a25c(RocketFlight& r, const RocketClassParams& c,
                                             const RocketHomingTargetView* target,
                                             float axial_speed, float* flight_time_out,
                                             std::array<float, 2>* steer_out) noexcept;

// 0080A000's motion part in image order. The proximity fuze is not included.
RocketAirStepResult rocket_air_step_0080a000(RocketFlight& r, const RocketClassParams& c,
                                             const RocketHomingTargetView* target,
                                             float dt) noexcept;

// 0080AA20 (BSP_Rocket_SetHomingTarget), __thiscall(block = record+468h; target,
// point*, angle), RET 0Ch. A target that answers IsKindOf(5) is registered with
// the point and the angle; anything else clears the block to the zero vector
// [00F87574] and angle 0. `target_is_kind_5` is the caller's vtable[5Ch](5).
void rocket_set_homing_target_0080aa20(RocketHomingBlock& block, std::size_t target_plus_one,
                                       bool target_is_kind_5,
                                       const std::array<float, 3>& point, float angle) noexcept;

// 0042AE80, __thiscall(float v[3]; float s), RET 4: v /= s, each a float store.
void vector3_divide_in_place_0042ae80(std::array<float, 3>& v, float s) noexcept;
// The rocket air step (via game_hosts_gunnery) runs on the main thread at 24-bit precision
// control, where each FDIV rounds to 24 bits with the extended exponent before the FSTP
// DWORD: a denormal quotient is rounded twice (cc13-fixture9, docs/CC13_FIXTURE_PASS_9.md).
// vector3_divide_in_place_pc24_0042ae80 models that and is fixture-tested;
// vector3_divide_in_place_0042ae80 takes it once this is flipped.
inline constexpr bool kVector3DivideInPlacePc24Cc13Bound = false;
void vector3_divide_in_place_pc24_0042ae80(std::array<float, 3>& v, float s) noexcept;

}  // namespace bsp
