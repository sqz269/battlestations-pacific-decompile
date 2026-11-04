#pragma once

namespace bsp {

// Goal 4 chain 386: the gun bots' virtuals on the loaded Robots.lua rows in
// src/game_hosts_gunnery.cpp. ON: 0072F6E0 (the gun's line-of-fire record
// cache), 008FF200 (AAFlakBot SetFireTarget) with 008FDBE0 (its error roll),
// 008FB590 (DepthChargeBot accept) and 00730160's throw magnitude
// (gun_throw_magnitude_0073031d) run their reconstructed bodies.
// OFF: the host's inline stand-ins, unchanged.
inline constexpr bool kWireGoal4Chain386Bound = true;

} // namespace bsp
