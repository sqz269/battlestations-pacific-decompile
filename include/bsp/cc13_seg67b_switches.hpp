#pragma once

// Packet cc13-seg67b (cc9 adoption plan item 4, "seg67 parts of B"): switches for
// live seg67 plane-state mismatches the units host does not model yet.
// docs/CC13_SEG67B_ITEM4.md. Every switch starts OFF; cc9 flips.

namespace bsp {

// 009A76E0 BSP_BotStateDogfightAim_Tick_Provisional, the not-head-on arm
// (009A790B-009A794B): plan+2B0h = 1 (byte, MOV at 009A7932), plan+2D8h = 1,
// plan+2B4h = (approach+D4h - row+20Ch) + target vtable[38h]. Plan = approach+18h.
// The host's aim wrapper writes +2B4h and +2D8h but leaves +2B0h as the last writer
// set it, which is 0 from the dogfight follow (009C1C0D). +2B0h gates the pilot's
// target-speed-correction raise (0099D8CD -> 0099D924; host reader
// kPilotSpeedCorrDivisorBound). No dogfight state (009A7000-009AB000) writes +2B0h
// = 0, so the binary's 1 also persists through maneuver and avoid until a 0-writer
// (the follow, another task's state) runs.
// True: the not-head-on aim tick stores plan+2B0h = 1.
// False: unchanged host (+2B0h keeps its previous value, usually 0).
inline constexpr bool kSeg67bDogfightAimSpeedCorrOffBound = false;

}  // namespace bsp
