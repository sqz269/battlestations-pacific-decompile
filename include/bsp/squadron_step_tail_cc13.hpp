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
// Membership: the image reaches 0084E010 only through 0071F290's vt[7Ch] call at
// 0071F39E. 0071F290 returns first unless the owner entity ([dir+34h]) has +5Ch set
// (0071F29F) and +5Dh, +60h, +5Eh clear (0071F2A9..0071F2C1), and unless
// [[00E188A8]+1FE4h] == 2 (0071F378). The host mirrors these the way 009F8470's
// host call (run_squadron_free_attack_009f8470) is gated: a registry record with a
// flight leader and at least one member the gunnery host does not count dead. On
// top of that, the squadron's own unit (the host fuses the squadron entity with wing
// 0, script_orders 007EDA91 note) must pass the live-entity test used at 0043F080 /
// 009AACD1 and 0074F09A: state->active (+5Ch), state->simulate (+5Dh),
// scene_pending_destroy_0060 (+60h), scene_destroyed_005e (+5Eh). LABELLED: that
// gate tests wing 0's state and scene flags, not gunnery death. A gunnery-dead but
// active wing 0 passes, and the moveto-self is then skipped (its issue requires a
// unit the gunnery host does not count dead). A destroyed wing 0 ends the tail for
// the whole squadron, whereas the image's separate squadron entity would continue.
// The session mode is != 2 in every host (0 or 1; single player), so 0071F378
// always passes.
//
// Take-off: 0084E010 has no take-off test of its own. The head arm needs only an
// empty head and queue and a non-null +3D0h; with +369h = 1, b4 goes to 0084E595
// and a squadron with an empty queue gets its moveto-self. A squadron registered at
// launch with an empty head would therefore take b4 during take-off unless
// 0071F290 holds it. A hangar member is a disabled entity: 007BC550 -> 00922F80
// clears its +5Ch, and 007EF010 sends only members whose +5Ch is clear. In the host,
// wing 0 is the squadron entity, so the +5Ch test above holds the tail until wing 0
// has been sent. LABELLED: this is the equivalent gate, not a cited one. Whether
// the image's separate squadron entity is disabled while its wings wait inside is
// unread, so if it stays enabled, the image issues the moveto-self during take-off
// and the host does not.
//
// The host (kSquadronMoveToSelfBound) runs that tail only for spent_rtb records
// whose command the retest ended (ended_stage2_command) or that latched.
//
// True: GameUnitsHost::run_spent_ordnance_rtb_0084e010 also runs the tail, from
// the leader's director (stage, override stage, filled slots, slot 0), for every
// other member squadron as defined above. A resolve is the existing
// issue_return_to_base_007f16d0 (returntobase to each member; 007F16D0's choice
// is record_return_to_base_007f16d0's). The moveto-self is the same issue the
// spent_rtb path makes. The tail sits after that function's early return, so it
// runs only because kSquadronSpentOrdnanceRtbBound is true
// (squadron_spent_ordnance.hpp:33) or the BSP_SPENT_RTB_CENSUS diagnostic is set.
// LABELLED:
//   - +369h is taken as 1 (007F2D09). The +369h == 0 path is not run: a flight
//     created with ReloadEnabled false (the power-up spawn request,
//     cc13_large61.hpp:329, a4) would resolve from the head arm, an empty head or
//     a head whose category is neither 1 nor 2 (0084E50D-0084E527, so a
//     category-0 head), which this host never does;
//   - +368h is taken as 0, and the latch as clear, because latched squadrons stay
//     on the spent_rtb path;
//   - a placed count of 0 stands for 007F16D0's null answer;
//   - once the tail issues, it waits for the head to change (the image re-issues
//     every step while its conditions hold). The guard is set even when nothing
//     was placed (the issue was refused), so the host never retries that head,
//     while the image retries every step;
//   - after a moveto-self completes, the image's re-issue re-arms the stage through
//     0071ECF0; the host does not re-issue, so the leader's director stays at
//     stage 2;
//   - the own-moveto test assumes the squadron-unit issue becomes the leader's
//     slot 0, the proxy the spent_rtb path already relies on (the host fuses the
//     squadron's command block with its flight leader's);
//   - the leader test is the gunnery host's death test, a proxy for 0084E043,
//     which tests +3D0h for null, not for death.
// False: only the spent_rtb path, unchanged.

namespace bsp::game {

inline constexpr bool kSquadronGeneralStepTailBound = true;   // ON: OCEAN_WAVE_FIELD 95 (cc9-physics23)

}  // namespace bsp::game
