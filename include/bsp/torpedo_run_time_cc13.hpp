#pragma once

// Packet cc13-torprun (defect fix for 009D1360 torpedo_run_time).
//
// kTorpedoRunTimeAxisSpeedCc13Bound (OFF): the unit speed 009D1360 reads at
// 009D138C is unit->vtable[38h], 0092D730: the dot of the body's linear
// velocity (00C31F40, body+4) with the body axis at 00C32000's +18h/+1Ch/+20h,
// evaluated as (v.y*a.y + v.x*a.x) + v.z*a.z on the x87 stack (0092D74E..0092D763)
// and stored to float (0092D765). ON evaluates that per op at 24-bit precision
// (the main thread's precision control) from the host's velocity and pose_row2,
// the axis GameUnitsHost::unit_forward_speed_0092d730 uses. OFF keeps |v|, the
// speed magnitude, which differs whenever the velocity is off the body axis.
// Site: src/game_hosts_units.cpp, GameUnitsHost::Impl::torpedo_run_time_009d1360.
// Expected movers when ON: torpedo run times (approach +98h), and with them
// torpedo release timing, hits and misses.

namespace bsp {

inline constexpr bool kTorpedoRunTimeAxisSpeedCc13Bound = false;

}  // namespace bsp
