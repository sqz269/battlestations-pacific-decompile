#pragma once

#include <cstddef>

// Packet cc13-leak92 (STUB census row 22). docs/CC13_LEAK92.md.
//
// kLeakMessageRouterDeliveryCc13Bound: the wreck's leak rates arrive one step late.
// The ship's wreck handler 00824B60 runs from the fan-out row 15 flush (009273A0
// -> 00926390 -> vt[7Ch]). Its sink block calls 0074EC50 at 00824FE5:
//   0074EC50  outside lobby mode 2 ([00E188A8]+1FE4h), draws U(0, 1) per leak
//             from 00BD2F10 (ECX = 1, 0074ECA5) and computes the new rates into a 92h
//             message (MT_SHIP_SINK, 0075B430(92h)); the leak manager itself is
//             not written;
//   0074ED7B  0077C2A0(msg, 7, 0) with ECX = the unit (manager+10h) routes it;
//             a local session copies it to the loopback queue (0076E520).
// The next session pump (row 9, 00875E91 -> 00778450 -> 0076C600, or the
// extra step 00874D00 at 00874D68) delivers it to the ship's handler 00821E80.
// Arm 92h (jump table 00822394 index 0Dh) is 0082220D: 0074E860(&unit+10D4h,
// msg+1Ch count, msg+20h), which overwrites the rate array (+18h) with the
// message's copy.
//
// OFF (today): the host writes the rates at the post. ON: the units host keeps
// the computed rates and the fixed-step host's pump delivers them by the same
// overwrite. The draws stay where they are in both cases, at the post.
//
// OFF is identical: the ON-only storage is never filled, and no call or log
// line is added on the OFF path. Not yet game-validated.

namespace bsp {

inline constexpr bool kLeakMessageRouterDeliveryCc13Bound = true;  // ON: JM08 pair, same-time wrecks within 0.03% water (SHIP_AI 311)

}  // namespace bsp

namespace bsp::game {

// 0076C600's delivery of the queued 92h messages, in post order (0082220D ->
// 0074E860). Returns how many it delivered; 0 with the switch OFF.
std::size_t units_deliver_ship_sink_92h_0082220d();

}  // namespace bsp::game
