#pragma once
// Goal 4, chain 260 (slot g4-w3): session-message loopback round trips.
//
// 0077C2A0 in a local session hands every routed message to 0076E520, which
// writes it into a 400h-byte stack stream (message vtable[4]), rebuilds a
// second message from those bits with the factory 00768530 (type byte read
// and rewound, then the case's constructor and vtable[8] read), and queues
// that copy. HostSessionMessages runs this round trip for the message types
// the hosts route; this chain adds:
//   76h MT_FORMATION_JOIN  (0077C8D0 at 0077C964): 0075C7C0 write, factory
//       case 76h 0075A2E0 ConstructForRead, 0075C810 read; the copy's 12-bit
//       member id is the leader the delivery joins.
//   77h MT_FORMATION_LEAVE (0077C980): 0075C860 write, 0075A330, 0075C8B0
//       read; the copy's member id (null target, 0) is the leave's other.
//   9Fh MT_SHIPREPAIR_BODYREPAIR (008AD330 RepairEnable at 008AD4CD):
//       00760A50 constructor (the sender's inlined stores, and the factory
//       case 9Fh), 00760AE0 write, 00760B30 read; the copy's +1Ch bool is
//       the flag 00939FD0 receives.
// ON: the hosts read the copy's fields. OFF: the posted values, as before.
namespace bsp {
inline constexpr bool kWireGoal4Chain260Bound = true;
}  // namespace bsp
