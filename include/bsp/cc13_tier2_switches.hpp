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
// ON (cc9-gunnery53) by mechanism. 0072CDF1 MOV EBX,[ESP+28h] and 0072CE8E PUSH EBX
// were read from the bytes. Pairs on aa5aaab54, reference env, 30000 frames
// (SHIP_AI_OPEN_ITEMS 300.3):
//   - JM08: 34 blocks either way, gameplay identical.
//   - USN07: no queries.
//   - USN13: blocks 420 -> 426. The first moved death is a plane at 415 s; the last
//     player plane dies at 655.76 s instead of 832.57 s, so "Game Over" comes at
//     659.96 s instead of 834.92 s.
inline constexpr bool kLineOfFireLandFromMuzzleBound = true;

// 009C7F00, the dive-bomb goaway completion test. The image stores three values
// to float: the term (+20h * 0.9 qword [00D7A390]) at 009C7F16, the reload arm's
// second 0.9 product at 009C7F7D, and ceiling - 100.0 (qword [00D7A220]) at
// 009C7FB8. The compares (009C7F8D, 009C7FC2) read those floats back.
// ON: dive_bomb_goaway_complete_009c7f00 rounds each value to float where the
// image stores it. OFF: the host keeps all three in double.
// Difference: at most 1 ULP at a compare, which moves goaway completion by at
// most one tick at a rounding boundary.
// Caveat: the cast rounds the double product a second time, where the x87
// under 24-bit precision control rounds the exact product once. The two can
// differ only on a double-rounding tie.
inline constexpr bool kDiveBombGoawayFloatStoresBound = true;   // ON: OCEAN_WAVE_FIELD 98 (cc9-physics23)

}  // namespace bsp
