#pragma once

// Packet cc13-jm08 (read-only audit of JM08's leaderless landing ships, cc9 request).
//
namespace bsp {

// True: after a destroyed ship leaves its formation group (0077BD70, behind
// kDeadMemberLeavesGroupBound), the units host republishes the 007788B0 / 007788D0
// pair to the commands host for the leaving unit and every remaining member. In the
// image both predicates read unit+284h and group+14h live, so a leader hand-over is
// seen at once by 00836920.
//
// What the image does at a LEADER's death (corrected after review of 8fb2ae456):
// - 0077BDDE..0077BE02, before 0070D0C0 at 0077BE40: the dying unit's director
//   (vt114h = 0080E150, [unit+738h]) gets vt5Ch(successor) = 0071C150. While the
//   dying unit is still group+14h (00778890), 0071C150 calls the successor's
//   director vt4Ch(dying director) = 00836680, which runs 00720450: allowFire /
//   allowMove (+3Ch/+3Dh), the previous-command record (+16Ch..+184h), the mode
//   (+30h), the override cleared (0071E610), the ten command records copied (for a
//   kind-6 owner a `land` 00E08FA0 becomes `attackmove` 00E08F78, 0072058A..
//   00720607), stages reset; then 00836240 when src vt48h(2) and src+23Ch.
//   So the NEW LEADER INHERITS THE DEAD LEADER'S QUEUE in place of its own
//   `follow`. In JM08 that queue is Missouri's scene `cruise`, so the successor
//   begins a cruise (its own heading and ordered throttle, 00835E17) rather than
//   stopping. That copy is NOT modelled by this switch (see the report: the host's
//   director state lacks part of the copied record).
// - Members: the follow arm 00836ADC..00836B3B ends each member's `follow` on the
//   dead leader (00836B1A, leader != stored target), and the idle tail 00836DC9
//   re-issues `follow` 00E08F60 on the new leader (00836E13..00836E38).
// - The idle tail's cruise/stop pair (00836E3F) applies only to a leader whose
//   queue empties. Its +28h test follows 00835BF0(1): +28h is reset to -1.0f only
//   when it is >= 0 and int(clock) - +28h > 1; then >= 0.0f issues `cruise`
//   00E08F70, else `stop` 00E08F88.
// False: the commands host keeps the pair pushed at the join (set_unit_formation
// from 0077FA8D), so after a hand-over every member, the new leader included, keeps
// a `follow` aimed at the dead leader, as before.
// No 00BD2F10 draw on any of these paths.
inline constexpr bool kDeadLeaderRepublishesFollowerPairBound = false;

// True: ship_ai_wake_decompose_00811180 (src/ship_ai_wake_trail.cpp) returns the
// image's along-track distance. 00811180 picks the segment through the nearest
// sample k (00811482..00811584: k == 0 takes head/k1, k == 27h takes k-1/k, else
// the newer neighbour when its float squared 3D distance is <= the older one's),
// stores the projection dot(sample[b] - point, normalize(sample[b] - sample[a]))
// at 00811658 into [ESP+48h], and the leg loops 0081166A..00811721 add the b+1
// legs of slots head..head-b onto that same slot. So a member on a trail sample
// gets the full arc length (LST 01 in JM08: 1000, not 950), and a member ahead of
// the leader's head gets a NEGATIVE along (Helena: -500), which 00810630's
// head branch (along <= 0) extrapolates ahead of the leader.
// False: along is the sum of the first `k` legs from the head with no projection
// term, as before: on-sample stations sit one leg (50 m) short and every station
// ahead of the head collapses onto the leader's own point (JM08 Helena's station
// becomes Missouri, which stalls the column at about 120-180 s).
// The across convention (0070D290's left normal from the base at `along`) is
// unchanged. Main-thread x87 (24-bit PC) is modelled as float ops in the image's
// order. No 00BD2F10 draw.
inline constexpr bool kWakeDecomposeSegmentProjectionBound = false;

}  // namespace bsp
