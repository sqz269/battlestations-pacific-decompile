#pragma once

// SquadronSetSpeed's per-plane velocity set, 007D9E80, as the image does it (packet
// cc13-setspeed). Header-only switch.

namespace bsp {

// kSetSpeedCtlAxisCc13Bound (OFF): 007D9E80 BSP_PlaneFlightController_SetForwardSpeed
// (__thiscall on the controller unit+AB0h, one float, RET 4), reached from Lua
// SquadronSetSpeed 0089F780 -> 0074E1E0 (game_hosts_lua.cpp, unchanged here), the plane
// vtable slot +3Ch on nine plane vtables, and 007D5D20's property bag (007D64E5,
// 007D654D; the host's spawn seed does not come through this function):
//   007D9E86..007D9E96: ctl+3Ch/+40h = 0, ctl+44h = speed (the body velocity);
//   007D9E9B..007D9EBD: ctl+48h..+50h = [00F87574..00F8757C] (the body angular rate);
//   007D9EC2 007D9C80: ctl+18h..+20h = 0042D0D0(ctl+3Ch, unit+74h, w 0), the body
//     velocity through the matrix rows, and ctl+24h..+2Ch likewise for the angular
//     rate; then a tail jump to 007D8020 (speed terms: ctl+6Ch = |ctl+18h|, ...);
//   007D9EC7..007D9ED6: ctl+30h..+38h = ctl+18h..+20h.
// ctl+30h..+38h (unit+AE0h) is the low-passed velocity that 007DC743..007DC80B
// filters at the core law's tail; the host carries it as plane_velocity_lp_30 under
// kPlaneVelocityLowpassBound. Its readers are 007C18B0 (unit+C84h flight-path angle)
// and 0099DA97..0099DAC8 (the throttle error correction).
// OFF, GameUnitsHost::set_plane_forward_speed_007d9e80 sets the velocity to
// speed * forward / |forward| with a 1e-6 length guard and leaves the low-pass alone.
// ON, each axis is f32(speed * row2[i]): the zero body components add nothing in
// 0042D0D0, so there is no normalisation and no guard. The low-pass is set to the new
// velocity (and marked valid). LABELLED: the host's pose rows stand for unit+74h, which
// is the local matrix; they agree for an unparented plane. 007D8020's terms are left to
// the host's per-step refresh.
// Movers when ON: after a script SquadronSetSpeed, the plane's low-passed velocity
// restarts at the new velocity. That moves the flight-path angle +C84h (pitch mode 2)
// and the throttle correction for the next steps. The velocity changes by rounding only.
inline constexpr bool kSetSpeedCtlAxisCc13Bound = false;

}  // namespace bsp
