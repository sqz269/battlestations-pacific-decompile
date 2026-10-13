#pragma once
// Packet cc13-ver16: the flak terms the gunnery host does not model yet.
// Evidence: docs/CC13_VER_16.md. Every routine is pure and takes the fields
// the image reads; nothing here is wired into a host. The names are
// hypotheses, not recovered symbols.
//
// - 009030C0 AAFlakBot tick: the range gate on the 00901C20 lead distance
//   (0090332C-00903344) and the vertical adjust (00903297-009032E9).
// - 0070C370 flak round tick: the proximity fuse's arming test
//   (0070C3DA-0070C400). The fuse time itself, MinRange / V0, is
//   0070C0B0 and is already reconstructed as
//   WeaponClassFinaliseResult::flak_fuse_time (bullet_engagement_range.hpp).
#include <cstdint>

namespace bsp {

inline constexpr std::uint32_t kFlakBotRangeGateAddr = 0x0090332C;
inline constexpr std::uint32_t kFlakBotVerticalAdjustAddr = 0x00903297;
inline constexpr std::uint32_t kFlakFuseArmTestAddr = 0x0070C3DA;

// 00D7A320 (float BCA3D70Ah) and 00D7A2F8 (double 3F947AE140000000h, the
// float 0.02f widened), 00D7A280 (double 0.5).
inline constexpr float kFlakBotLowVertical = -0.0199999995529651641845703125f;
inline constexpr double kFlakBotVerticalBias = 0.0199999995529651641845703125;

// 0090332C-00903344. `lead_distance` is 00901C20's outDist (|lead point -
// muzzle|), which 009031F4 / 0090319A write over the tick's dt slot; the
// bounds are the current ammunition's class +58h (MinRange) and +60h
// (FlyTime * V0 for a Flak class). FCOMI + JBE twice: both comparisons are
// strict and an unordered (NaN) distance fails.
bool flak_bot_range_gate_0090332c(float lead_distance, float min_range,
                                  float max_range) noexcept;

// 00903297-009032E9. `vertical` is the solved elevation plus bot+5Ch (radians,
// the angle 0085ABA0 is then asked for). Only a negative value changes:
// against a target answering IsKindOf(0Fh) (a plane) it becomes 0; otherwise,
// below -0.02f, it becomes v - (v + 0.02) * 0.5 (x87, stored to a float).
float flak_bot_vertical_adjust_00903297(float vertical, bool target_is_plane) noexcept;

// 0070C3DA-0070C400. The proximity search runs only when the round's flight
// time (+1C4h) is strictly greater than the class fuse time (+D8h) and the
// tick's scaled step is not equal to 0.0f (00D7A218; UCOMISS / LAHF /
// TEST AH,44h / JNP).
bool flak_fuse_armed_0070c3da(float flight_time, float fuse_time,
                              float scaled_step) noexcept;

// 0072D5B0 is already reconstructed (gun_select_fire_mode_0072d5b0,
// gameplay_seg40_59_cc13.hpp). The fact the host misses is what reads the
// record it selects: Throw [gun+3F8h]+4h (007302DF), the reload draw
// U(+28h, +2Ch) on stream 1 (0072D55C-0072D579), the barrel delay +30h
// (00730A05) and the throw seat from [[gun+3F8h]+34h]+8h (0073031D). In
// mode 1 (008FF240, a kind-6 gun with an AI-held role slot) those are
// Bullet[2]'s fields. docs/CC13_VER_16.md D4.

}  // namespace bsp
