#pragma once

// Packet cc13-mp5: the session-1 power-up messages 67 / 68. Names are hypotheses,
// not recovered symbols.

namespace bsp {

// kPowerupSessionBroadcastCc13Bound (OFF). When [game+1FE4h] == 1 the image builds
// message 67 (008E6E00, MT_POWERUP 43h) or 68 (008E6FB0, MT_POWERUP_EFFECT 44h) on
// the stack, hands it to 0076A9F0(ECX = game+1EF0h; &msg, 0) and destroys it.
// 0076A9F0 is MOV ECX,[ECX+188h]; JMP 00784790, and 00784790
// BSP_SessionTransport_BroadcastExcept walks the transport's list at +8h from
// ++begin(): 0078479A..007847AA take the first node and step past it, so the first
// entry (presumably the local one) is never sent to. Each later entry gets EnterCS,
// 00783C80 serialize, LeaveCS. On an empty list the begin() check at 007847A3 calls
// the _SECURE_SCL trap 00BF6713. There is no local or loopback arm, so with no
// entries after the first ("zero peers" below) the send changes nothing. Every send
// arm rejoins the path that already did the local work:
//   008E8CE6 (008E8C30 expire, 68 op 2)   -> 008E8D4E, the list erase
//   008EA466 (008EA250 insert, 68 op 0)   -> 008EA4AE, the epilogue
//   008ED5AB (008ED4B0 grant, 67 op 0)    -> 008ED5F1, the passive use
//   008E8AC0 (008E8A30 remove, 67 op 2)   -> 008E8B37, the active-item sweep
// OFF: require_unhosted_session throws at those four sites (game_hosts_powerups.cpp),
// and PowerupManagerBinding::send_powerup_message67 throws (game_hosts_mission_frame.cpp;
// it also serves 008EDFB7 GrantItem and 008EA06B PrepareFromLua in cc13_powerup.cpp).
// ON: each site calls send_powerup_message67 / send_powerup_message68 and counts it;
// the binding records the 0076A9F0 send to a list with no entries after the first
// and returns.
// Movers: none on game state; a session-1 run no longer throws here.
// Dependency: the binding's session_word_1fe4() (game_hosts_mission_frame.cpp) returns
// 0 until MP2a's SessionContext supplies the session mode, so the ON arms compile but
// are unreachable until then.
// For MP2a: the other senders route through 0077C2A0, not 0076A9F0. In mode 1 an
// explicit route override of 4 (building_pads.cpp:336, 0074B0A0) goes to peers only
// and is never delivered locally (0077C3C6 peer walk, no 0077C43A enqueue); with the
// default [00E0AF1C] = 1 the message is only enqueued locally through 0076E520.
inline constexpr bool kPowerupSessionBroadcastCc13Bound = true;  // cc9-sweep2b: prune flip, the OFF arm throws wherever this is reached (session 1 only)

}  // namespace bsp
