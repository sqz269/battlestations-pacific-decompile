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
//   &+11D4h at 009F27A5 and &+11D5h at 009F279E);
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
// any path-arm call now writes 11D4h / 11D5h, which gate 009E6640's side
// refusal, 009E7C72's goal-override arm and 009E6E94's -1000 standoff.
// The extra 009E3C00 calls also move the plan's follower fields and can
// advance its cursor (009E421F), as the image's do.
inline constexpr bool kShipAiArcCentrePlanTestCc13Bound = false;

}  // namespace bsp
