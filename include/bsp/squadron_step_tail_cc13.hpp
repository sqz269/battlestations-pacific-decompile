#pragma once
// Packet cc13-steptail (cc9 -71 grant): 0084E010's tail for every squadron.
//
// The image runs the squadron command block step 0084E010 for every squadron.
// Its tail (model: src/airops_ver_cc13.cpp, docs/CC13_VER_10.md section 2):
//   - b4: the head arm 0084E047-0084E05C (+54h == 0, a leader at +3D0h, and
//     0071BE60() == 0), or a stage-2 head (+48h == 2) with at most one filled slot
//     that is not retreat / returntobase / land and has no spent-ordnance latch
//     (0084E4B1-0084E4E1);
//   - b5: such a head that is returning or latched (0084E4E8), or override stage
//     2 with the latch (0084E4ED-0084E4FA);
//   - the resolve gate 0084E4FF-0084E54B: with ReloadEnabled (+369h) set, only b5
//     resolves. A resolve calls 007F16D0 (0084E552) unless both reload bytes, or
//     +368h, are set. A non-null answer is issued (0071D880, 0071ECF0). A null
//     answer, a gated resolve and a bare b4 all go to 0084E595, a moveto on the
//     squadron itself when 0071BE60() <= (stage == 2).
//
// The host (kSquadronMoveToSelfBound) runs that tail only for spent_rtb records
// whose command the retest ended (ended_stage2_command) or that latched.
//
// True: GameUnitsHost::run_spent_ordnance_rtb_0084e010 also runs the tail, from
// the leader's director (stage, override stage, filled slots, slot 0), for every
// other squadron that has a live flight leader. A resolve is the existing
// issue_return_to_base_007f16d0 (returntobase to each member; 007F16D0's choice
// is record_return_to_base_007f16d0's). The moveto-self is the same issue the
// spent_rtb path makes. LABELLED:
//   - a placed count of 0 stands for 007F16D0's null answer;
//   - +369h is taken as 1 (007F2D09), +368h as 0, and the latch as clear, because
//     latched squadrons stay on the spent_rtb path;
//   - once the tail issues, it waits for the head to change (the image re-issues
//     every step while its conditions hold).
// False: only the spent_rtb path, unchanged.

namespace bsp::game {

inline constexpr bool kSquadronGeneralStepTailBound = false;

}  // namespace bsp::game
