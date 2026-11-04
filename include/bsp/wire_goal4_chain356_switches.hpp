#pragma once
// Goal 4 chain 356 (agent/g4-w1). Two single-player session messages that
// 0077C2A0 routes to the local queue (0076E520) now travel through their
// rebuilt copies, as previous chains did for 5Ah, 76h/77h, 7Fh, 88h, 93h and
// the others in game_hosts_session_messages.cpp:
// - MT_GUN_FIRE (ADh): 0072D130 / 0084C5B0 send it with flags 1; the copy is
//   built by 006E3940 / 006E39B0 / 006E39E0 and delivered through the gun's
//   message switch 0072D830, whose ADh arm is FireIfReady (vtable[1DCh]).
// - MT_PLANE_DISAPPEAR (C7h): the plane's fixed step (007CE6FE..007CE74F)
//   sends it with flags 5; the copy (00762B20 / 00762BE0 / 00762C30) carries
//   the holder owner's id, from which the receiver (007CCFA0 case C7h ->
//   007BA890 -> 007CC8B0) resolves the holder whose stock the plane returns to.
// OFF keeps the direct deliveries (the sent flag and the deck index).
namespace bsp {
inline constexpr bool kWireGoal4Chain356Bound = true;
}  // namespace bsp
