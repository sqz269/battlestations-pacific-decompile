// The rocket's proximity fuze, the tail of the air step 0080A000 (0080A55D-0080A9F8;
// packet cc13-rocketfuze, after cc13-homing1's motion part in rocket_homing_cc13.hpp).
// Names are hypotheses, not recovered symbols.
//
// 0080A55D..0080A592, the gate: (class +E4h or the byte [00F874FD]) and record+3BCh
// (the shooter entity) non-null and [00E188A8+1FE4h] != 2. It is reached on every step:
// the not-ignited exit 0080A155 and the homing exits 0080A264/0080A279 all jump to
// 0080A55D.
//   - class +E4h is the bulletclasses "AntiAir" bool: BSP_RocketClass_ReadLuaFields
//     00809CB0 pushes "AntiAir" (00D09030) at 00809D79..00809D9F, calls 00B662F0
//     GetBooleanOrDefault(0) and stores AL at [esi+E4h]. In this installation only the
//     R4M rows (bullets 11 and 116, Type "Rocket") set it.
//   - [00F874FD] is written by 008C1620 SetRocketAirGroundTypeDifferent(flag) as
//     !flag, zeroed by 004DA7C4 (teardown) and 004DFBE5 (scene load), and stored from
//     CL by the setter 004B4060 (MOV [00F874FD],CL; RET), which has no caller. Single player
//     calls the binding only in JM13 and PRCPJM13, with true, so it stays 0; the
//     multiplayer scripts pass false. The host keeps it as a bool, false, and does not
//     host the binding (LABELLED).
//   - [00E188A8+1FE4h] == 2 is the network client; never true in the host.
// 0080A5A8..0080A843, the scan: the shooter's side recon list
// [008053C0(shooter+54h)]+DE8h, IsKindOf(0Fh) only. For each plane, with the rocket's
// world translation p (+FCh) and velocity v (+318h, after the homing write) and the
// target's +FCh and vtable[34h] velocity: dv = v - tv, the closest-approach time
// t = -(dv . (p - tp)) / |dv|^2 clamped to [0, dt] (0 when |dv|^2 == 0), and the miss
// distance squared at t. The smallest wins (strictly smaller; start 00D7A248 = FLT_MAX).
// 0080A843..0080A87D, the fire test: best < (class +70h BlastRange)^2 and t < dt, both
// strict, so a t clamped to dt waits for the next step.
// 0080A883..0080A9F2, the detonation: when t != 0 the rocket moves to p + v t
// (006E8040); a zeroed hit record (no hit entity) goes to 0084BC60(ECX = record+310h,
// DL = 1; 1, &record, 3, &zero vector 00F87574), whose step 7 is the class Blast
// through 0084BAD0 at the rocket's point (ignoreFalloff = class+8h == 10h, false for a
// rocket); then the point effect (+338h, presentation) and Kill(1).
// RNG: the fuze draws nothing; the blast draws as any impact blast does.
#pragma once

#include <array>
#include <cstddef>

#include "bsp/rocket_homing_cc13.hpp"

namespace bsp {

// OFF: a host rocket bursts only on contact (the sweep impact) or expires.
// ON: run_projectiles runs rocket_proximity_fuze_0080a55d after the air step on every
// rocket whose class is AntiAir and bursts it through apply_impact_blast.
// Movers predicted: none on any current row; no row releases an R4M (GUNNERY 202).
inline constexpr bool kRocketProximityFuzeCc13Bound = false;
static_assert(!kRocketProximityFuzeCc13Bound || kRocketHomingBound,
              "the fuze sits in the rocket air step that cc13-homing1 hosts");

// [00F874FD]; see above.
inline constexpr bool kRocketAirGroundTypeSameCc13 = false;

// One IsKindOf(0Fh) entry of the shooter side's contact list.
struct RocketFuzeTarget {
    std::array<float, 3> position{};   // target+FCh
    std::array<float, 3> velocity{};   // target->vtable[34h]()
};

struct RocketFuzeResult {
    bool fire{false};                   // 0080A86A and 0080A87D both passed
    bool moved{false};                  // 0080A894: t != 0, 006E8040 ran
    float best_d2{0.0f};                // [ESP+20h] after the scan
    float best_t{0.0f};                 // [ESP+10h] after the scan
    std::array<float, 3> point{};       // the burst point: p + v t, or p when t == 0
};

// 0080A5A8..0080A9F2 without the gate and the calls. Per-op 24-bit rounding in image
// order (main thread PC24, docs/CC13_FPU_PRECISION.md); every FSTP dword is a float
// store. `blast_range` is class +70h.
RocketFuzeResult rocket_proximity_fuze_0080a55d(const std::array<float, 3>& position,
                                                const std::array<float, 3>& velocity,
                                                float dt, const RocketFuzeTarget* targets,
                                                std::size_t count, float blast_range) noexcept;

}  // namespace bsp
