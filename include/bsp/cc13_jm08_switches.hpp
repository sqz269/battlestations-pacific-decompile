#pragma once

// Packet cc13-jm08 (read-only audit of JM08's leaderless landing ships, cc9 request).
//
// True: after a destroyed ship leaves its formation group (0077BD70, behind
// kDeadMemberLeavesGroupBound), the units host republishes the 007788B0 / 007788D0
// pair to the commands host for the leaving unit and every remaining member. In the
// image both predicates read unit+284h and group+14h live, so a leader hand-over is
// seen at once by 00836920:
// - the follow arm 00836ADC..00836B3B ends the new leader's `follow` (00836B03,
//   007788D0(unit) == unit) and every member's `follow` on the dead leader
//   (00836B1A, leader != stored target);
// - the idle tail 00836DC9 then issues `stop` 00E08F88 to the new leader (00836E3F,
//   not a follower; +184h clear; *(unit+73Ch)+28h < 0.0f after 00835BF0's reset)
//   and `follow` 00E08F60 on the new leader to each member (00836E13..00836E38).
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
