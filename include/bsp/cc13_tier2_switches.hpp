#pragma once

// cc13 Tier 2 adoption switches (docs/CC13_TIER2.md). Each one is OFF: the
// host keeps its current behaviour until a pair run judges the ON arm.

namespace bsp {

// 0072CDD0, the AA line-of-fire predicate. The landscape query 0098ADD0 at
// 0072CE91 takes EBX, the caller's muzzle pointer ([ESP+28h], loaded at
// 0072CDF1), as its start: the muzzle as given, NOT raised. Only the local copy
// at [ESP+18h] gets the +5.0 (qword 00D7A370, 0072CE42..0072CE52), and that copy
// is the start of the unit walk 0098B130 at 0072CECE. The end point is the target
// raised by 5.0 and floored at 5.0f (00CE3850) for both queries.
// ON: game_hosts_gunnery.cpp's line_of_fire_blocked_0072cdd0 starts the land
// segment at the unraised muzzle, as the image does. OFF: it starts at muzzle +5,
// the same point as the unit walk. A land hit refuses the target, and the answer
// is cached per (gun, target) for the gun's life (0072F6E0), so an ON land
// refusal persists.
inline constexpr bool kLineOfFireLandFromMuzzleBound = false;

}  // namespace bsp
