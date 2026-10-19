#pragma once
// Packet cc13-stophead (cc9 -71 grant): 009F8470's Stop-head arm, applied.
//
// The image: when the squadron's current command is `stop` 00E08F88 (009F85E3
// CMP EBX,0E08F88), the free-attack tick first sends 0071D880 (009F85ED,
// MT_GAMEUNIT_CLEARCMD with index -1, every slot), then 0071ECF0(moveto
// 00E08F68, 00468560(00427EB0(squadron), 0.0)) (009F8615, a moveto to the
// squadron's own position), and only then 0071D980 with the dogfight class
// (009F8631). All three are routed through 0077C2A0 to the loopback queue and
// delivered in post order at the step's row-9 drain, so the override lands on a
// moveto head, not on `stop`. When the override ends (0071E610) the squadron's
// head is that moveto.
//
// The arena (src/squadron_free_attack.cpp) records the arm as
// SquadronFreeAttackDecision::clear_then_moveto_self.
//
// True: GameUnitsHost::run_squadron_free_attack_009f8470 issues, to each live
// member, a `moveto` to the squadron's position (the leader's, as the arena's
// 00427EB0) with flags 1 through issue_script_command. Its MT_COMMAND delivery
// (00816E30) posts 0081733E's 0071D880 every-slot clear and then 0071ECF0's
// SETCMD. The member's 0071D980 override send and its task install run as that
// chain's after-delivery, so the order is clear, moveto push, override.
// SUBSTITUTIONS (labelled at the site): the clear and the moveto go through the
// entity-order hop 0077D600 -> 00816E30 rather than straight from the director;
// the host's 0071D980 is synchronous, so it is deferred behind that delivery.
//
// False: the arm is counted only (free_attack_moveto_self) and `stop` stays the
// head, as before.
//
// Not covered here (cc9's 0084E010 tail): after the moveto arrives, the image's
// 0084E4B1..0084E5C6 resolves 007F16D0 (return to base / land / retreat) or
// issues 0084E595's moveto-self. The host's tail runs only for spent_rtb
// records whose command the retest ended, so with this switch on, the squadron
// idles at the moveto point.

namespace bsp::game {

inline constexpr bool kSquadronStopHeadMoveToSelfBound = false;

}  // namespace bsp::game
