#pragma once

#include <cmath>

namespace bsp {

// Packet cc13-torprun (re-scoped after rev2): a plane's vtable[38h] speed.
//
// On all nine plane vtables slot +38h is 007B8E60, FLD [ECX+B1Ch] / RET: the speed
// cached at unit+B1Ch (ctl+6Ch, ctl = unit+AB0h). 0080E0F0 -> 0092D730 is the ship
// slot, not the plane's. The only value writer is 007D8020
// PlaneFlightController_UpdateSpeedTerms (007D807C MOVSS [ctl+6Ch]), from the
// controller's world velocity ctl+18h/+1Ch/+20h, the host's plane_world_velocity
// (not motion.linear_velocity; they differ while a plane is deck-parented,
// docs/CC13_FIX18.md). 007DB2C0 ResetMotion stores 0.0 at 007DB382 and then
// refreshes through 007D9C80 at 007DB3D3.
//
// 007D8020's chain, main thread at 24-bit precision control, so each op rounds:
//   007D8024 FLD y; FLD x; FLD z
//   007D802D/2F x*x, 007D8031/33 y*y, 007D8035/37 FADDP y*y + x*x,
//   007D8039 FMUL ST0 z*z, 007D803B FADDP (y*y + x*x) + z*z, 007D803D FSTP float s2
//   007D8041-007D804F FCOMI against the double 1e-10 at 00CE3820, JBE
//     (s2 <= 1e-10 or unordered) -> 0.0f (007D8064 XORPS)
//   else 007D8051 00BF7030 sqrt of the float s2 (the CRT path keeps PC24),
//     007D8056 FSTP float: the correctly rounded float sqrt either way.
// Each product of two floats and each sum of two floats is formed in double and
// rounded once to float, which is the per-op PC24 rounding (the double result is
// exact, or rounds no differently from the float result, for these operands).
// Lifted from game_hosts_lua.cpp's plane_cached_speed_007b8e60 (cc13-fix12 /
// fix18), which now calls this helper.
//
// LABELLED deviation: the image reads the value 007D8020 cached at the last
// 007D9C10 RefreshBodyFrame / 007D9C80 BodyToWorldVelocity call; this recomputes
// from the current plane_world_velocity. The host's writers of that field were
// checked against the refresh points (CoreLaw's 007DC80E tail, 007D9E80
// SetForwardSpeed, 007C11E0, 007C5F60/007C6340 spawn, 007CA3F0, 007C6F50
// liftoff, 007C71E0 deck reparent): all are followed by a refresh in the image.
// The exception is carrier_deck_carry_in (game_hosts_units.cpp), which rotates
// deck_local_vel into plane_world_velocity at the head of the ground-roll arm,
// before pilot_think_and_commit's AI reads; assuming orthonormal carrier rows, a
// rotation keeps |v|, so the recompute differs from the cached value there only by
// the rotation's rounding.
inline float plane_cached_speed_b1c_007d8020(const float v[3]) {
    const float xx = static_cast<float>(static_cast<double>(v[0]) * v[0]);   // 007D802F
    const float yy = static_cast<float>(static_cast<double>(v[1]) * v[1]);   // 007D8033
    const float xy = static_cast<float>(static_cast<double>(yy) + xx);       // 007D8037
    const float zz = static_cast<float>(static_cast<double>(v[2]) * v[2]);   // 007D8039
    const float s2 = static_cast<float>(static_cast<double>(xy) + zz);       // 007D803B/3D
    if (!(static_cast<double>(s2) > 1e-10)) return 0.0f;                     // 007D804F JBE, 007D8064
    return static_cast<float>(std::sqrt(static_cast<double>(s2)));          // 007D8051/56
}

// kPlaneCachedSpeedB1cCc13Bound (OFF): the host's plane vtable[38h] readers take
// plane_cached_speed_b1c_007d8020(plane_world_velocity). OFF keeps each site's
// previous value: float(sqrt(x*x + y*y + z*z)) in double, rounded once, with no
// 1e-10 floor, over plane_world_velocity (the leader and follow-hold readers) or
// over motion.linear_velocity (the torpedo and dive-bomb readers).
// Sites (src/game_hosts_units.cpp, host line : image call):
//   torpedo_run_time_009d1360 (:11232), image 009D138C; called from the aim tick
//            (:35674, 009D19A4) and the done/prepare tick (:25619, 009D27D1)
//   leader_live_speed_007b8e60 (:7180), read at:
//     :9054   007F2920 squadron placement, 007F294A
//     :10948  009BEE30 follow law leader, 009BFC58/009BFCC3
//     :29522  009B5B00 level_bomb_release_enter, 009B5B2C
//     :29825  009B6D20 level-bomb moveto tick, 009B6D4F-009B6D5A
//     :29959  009B8100 level_bomb_prepare_arm, 009B8121
//     :29982  009B6670 level_bomb_prepare_tick in.live_speed_38, 009B6B56/009B6B59
//     :37743  009B1420 land-begin steer, 009B1A11 and 009B1A7A
//     (:30068 and :38045 are log lines only)
//   follow_hold_attitude (:10118), PlaneFollowHoldAttitude::speed own and leader
//            (:9842/:9843), 009BEE56 hold arm
//   TorpedoApproachBinding::unit_speed_vtable38 (:24812), image 009D3D01
//   dive-bomb aimdive tick tin.speed_vtable38 (:33847), image 009C5EDB-009C5EDE
// Not switched: game_hosts_lua.cpp GetEntitySpeed already uses the helper (live
// under kLuaGetEntitySpeedBound); moveto_target_speed_009c23b0 is class-dispatched
// (plane +B1Ch, squadron leader's +B1Ch, ship 0092D730) and is a separate packet;
// the avoid_len(plane_world_velocity) readers labelled vtable[38h] (vehicle and
// terrain avoidance, landing and take-off) stay with their own packets.
// Expected movers when ON: torpedo run times and release timing, wing station
// speeds and follow-law steering, dive-bomb pull-out, wherever a speed rounds
// differently or falls under the 1e-10 floor; and, for the torpedo and dive-bomb
// readers, every deck-parented plane (plane_world_velocity is deck-relative).
inline constexpr bool kPlaneCachedSpeedB1cCc13Bound = false;

}  // namespace bsp
