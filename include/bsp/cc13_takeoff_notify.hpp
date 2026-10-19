#pragma once

// cc13-takeoff adoption switches (docs/CC13_TAKEOFF_NOTIFY.md). All three are OFF:
// the units host keeps its current behaviour until a pair run judges the ON arm.
// The first two draw nothing from the shared RNG 00BD2F10; the third
// (kPlaneShipyardLaunchSlowPushCc13Bound) does, through SlowTakeoff's enter 009CE1D0.

namespace bsp {

// 007C17D0 BSP_Plane_NotifyTakeoffStart, the tail of takeoff/Takeoff's enter
// 009CE270. Reached from the SlowTakeoff -> Takeoff switch (009CFC70) and the
// ground retakeoff push (0099B107 -> 009CF8E0 with DL = 0).
// The image (007C17EB..007C1869): nothing unless byte +904h is set. In single
// player [[00E188A8]+1FE4h] is 0, never 2, so it builds message C3h {+20h = the
// current +900h, +21h = 4} and routes it with 0077C2A0(plane, msg, 7, 0). Mode 0
// sets EBX = 1 (0077C333), so the router posts it to the loopback queue
// (0076E520). The pump drains it at row 9 of the same fixed step (00778542 ->
// 0076C600 -> 00780670), and 007CCFA0's requested-4 arm (007CD02C) runs:
//   current 7         -> 007CB5F0 (the touchdown authority arm);
//   +900h is 4 or 5   -> 007C7430: +904h = 0, +910h = 0, +908h = 0.0, and
//                        +900h = 4 when it is 5 and +BF8h is clear;
//   otherwise         -> 007CA3F0(0) (the ground state from Locked).
// ON: game_hosts_units.cpp runs this at both enters and delivers the message
// after the plane's own step (LABELLED: the image delivers at row 9).
// OFF: the host only records "Plane::takeoff_enter_007c17d0".
inline constexpr bool kPlaneTakeoffStartNotifyCc13Bound = true;

// unit+AA0h, the ShipYardLaunch delay. 007D5D20 stores 0.0 (007D614B), then for a
// kind-1 holder with a `ShipYardLaunch` bag key 5.0 when the key's byte is set,
// else -1.0 (007D645A / 007D6464); a kind-2 (network create) holder uses its
// +12Ah the same way (007D6355). 007B83F0, the plane's 7Ah handler, stores 5.0 or
// -1.0. Readers in single player: 009CF8E0 (DL = 0: SlowTakeoff when > 0.0),
// 009CFB60 (SlowTakeoff's leave test), 007BB9A0 (the local input gate) and
// 007BC5B0 (the water-contact gate).
// ON: the units host carries +AA0h per plane (5.0 for a plane created from a
// record with `ShipYardLaunch` set, else 0.0; -1.0 after 009CFB60's 60 m arm) and
// the four readers use it. OFF: the readers take 0.0.
inline constexpr bool kPlaneShipyardLaunchAa0Cc13Bound = true;

// Sub-switch of the above (shared RNG). 009CF8E0 with DL = 0 (the ground retakeoff
// push) starts in takeoff/SlowTakeoff when +AA0h > 0.0, and SlowTakeoff's enter
// 009CE1D0 draws U(-0.3, 0.3) from 00BD2F10. ON (with the parent ON): the push
// takes that arm. OFF: the push keeps takeoff/Takeoff and draws nothing.
inline constexpr bool kPlaneShipyardLaunchSlowPushCc13Bound = true;

}  // namespace bsp
