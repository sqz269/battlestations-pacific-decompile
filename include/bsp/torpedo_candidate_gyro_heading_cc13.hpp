#pragma once

// The ship AI's torpedo heading as the image reads it, torpedo record+46Ch (packet
// cc13-torpgyro). Header-only switch.

namespace bsp {

// kTorpedoCandidateGyroHeadingCc13Bound (OFF): two ship-AI readers take a live torpedo's
// heading from torpedo+46Ch:
//   009DC060 (the track refresh after 009F0AD0's admission, 009EADC2): 009DC0F9 FLD
//     [torpedo+46Ch], 009DC102 FSTP track+20h;
//   008173E0 (the threat test 00814390 / 00814420 run): 00817535 FLD [torpedo+46Ch].
// +46Ch is the commanded heading: 00856637 installs the launch command's heading (the
// TorpedoBot run line plus AngleErr, or the player's order), else 10000.0 [00D0C310];
// 00857480 at 008574A0 replaces the sentinel with the hull's yaw on the first swim step
// (008574E8); 00857061 turns the swimming torpedo toward it. A homing torpedo (Fido) also
// rewrites it on each homing step: 00856E25 stores the target bearing in +46Ch even when
// the turn is skipped (the host does this at bomb_torpedo_tick.cpp:364). 0080DCA0
// (FLD [ECX+46Ch] / RET) is a getter for it with no referenced caller. OFF, the host's
// GameShipAiHost torpedo_candidate and torpedoes_threatening_00814390 use
// atan2(vx, vz) of the round's velocity (a LABELLED SUBSTITUTION). ON they read
// GameGunneryHost::LiveTorpedo::commanded_heading_46c, the round's commanded_heading,
// which torpedo_swim3d_step keeps as the image does (kTorpedoGyroHeadingBound,
// kTorpedoSwimDepthHomingBound), the 10000.0 sentinel included for a torpedo not yet
// swimming. LABELLED: under the kTorpedoSwimThrustBound arm (homing OFF) nothing seeds
// the sentinel.
// Movers when ON: ship AI torpedo evasion: the torpedo tracks' heading (track+20h) and the
// threat test's answer for torpedoes still turning onto their gyro heading, not yet
// swimming, or homing (their +46Ch is the bearing to their target), hence which torpedoes
// count as threats and the evasive turns they cause.
// ON since cc9-nondet2 (2026-10-28): plausible on every pair (docs/USN21_NONDETERMINISM.md 8).
inline constexpr bool kTorpedoCandidateGyroHeadingCc13Bound = true;

// Census (cc9-nondet1, log only), src/game_hosts_ship_ai.cpp: reads at the track
// (009DC0F9) and threat (00817535) sites, and those where +46Ch and atan2(velocity)
// differ by more than 1e-3 rad after wrapping (the 10000 sentinel always counts).
struct TorpedoGyroHeadingCensus {
    unsigned long long track_reads{0}, track_differs{0};
    unsigned long long threat_reads{0}, threat_differs{0};
};
TorpedoGyroHeadingCensus& torpedo_gyro_heading_census() noexcept;

}  // namespace bsp
