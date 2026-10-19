#pragma once

namespace bsp {

// Packet cc13-arcplan (docs/CC13_ORBIT.md section 5). kShipAiArcCentrePlanTestCc13Bound:
// 009E46F0's arm test and path arm, as the image has them.
//
// 009E46F6 MOV ECX,[ESI+2F4h] / 009E46FC CMP dword [ECX+1Ch],0 / 009E4701 JNE
// 009E47D3: the path arm runs whenever the live plan's search_state (+1Ch) is
// non-zero. The path arm then:
// - stores 009D5AE0 into nested+11D4h and 009D5B90 into nested+11D5h
//   (009E47D3 / 009E47DC and 009E47E4 / 009E47F1; the caller 009F27C6 passes
//   &+11D4h at 009F27A5 and &+11D5h at 009F279E). Both walks start at
//   plan+20h, the head (009D5AE1), not at the cursor, and store 009D5930's
//   direction into each undecided node's +4h; 009E3C00's cursor walk keeps a
//   non-zero +4h, so they must run before it, as here;
// - calls 009E3C00 at 009E4813;
// - subtracts the unit position from whatever point 009E3C00 left in the
//   record, with no test of record+18h.
//
// OFF (the host as it was): the arm test is node_count (+34h, the follower
// cursor) != 0, the two bytes are never written, and the path arm refuses when
// record+18h is 0. 009E3C00 writes +18h only at 009E4243, from a corner
// node's anchor handle, so a straight leg always refuses. 11DCh then takes 0.
//
// ON reach: every ship whose live plan has a non-zero state while its cursor
// is 0 moves 11DCh (the ring-scan reference bearing, so 1D8h in mode 3), and
// any path-arm call now writes 11D4h / 11D5h. Every host reader of the pair:
// - 009F282D, the mode-2 11DCh gate (game_hosts_ship_ai.cpp, ApproachStep);
// - 009E8005, the clearance end (ship_ai_approach_update.cpp, both tests);
// - 009E6B5B, standoff 4 (approach_update, the both-clear arm);
// - 009E6E94, the -1000 standoff (approach_update, both set);
// - 009E6640, the slot probe's side refusal (approach_update and its twin
//   in ship_ai_ring_scan.cpp, the 009E7709 scoring pass);
// - 009E7C80, the goal-override arm under 1209h (approach_update and its
//   ring_scan.cpp twin at 009E7C72).
// The image's only other writer of the pair is 009E5530 (009E5633 /
// 009E5639), in the ring construct.
// The extra 009E3C00 calls also move the plan's follower fields and can
// advance its cursor (009E421F), as the image's do.
inline constexpr bool kShipAiArcCentrePlanTestCc13Bound = true;  // ON with the astern skip: LOMP10 path_arm 3952, command_arm 7, completes 736.49 s (SHIP_AI 314)

}  // namespace bsp
