#pragma once

// Packet cc13-seg67b (cc9 adoption plan item 4, "seg67 parts of B"): switches for
// live seg67 plane-state mismatches the units host does not model yet.
// docs/CC13_SEG67B_ITEM4.md. Every switch starts OFF; cc9 flips.

namespace bsp {

// 009A76E0 BSP_BotStateDogfightAim_Tick_Provisional, the not-head-on arm
// (009A790B-009A794B): plan+2B0h = 1 (byte, MOV at 009A7932), plan+2D8h = 1,
// plan+2B4h = (approach+D4h - row+20Ch) + target vtable[38h]. Plan = approach+18h.
// The host's aim wrapper writes +2B4h and +2D8h but leaves +2B0h as the last writer
// set it, which is 0 from the per-think reseed: BSP_PilotBot_SeedPlanSlots stores
// DL = 0 (XOR EDX,EDX at 0099B46E) at 0099B53C, called by BSP_PilotBot_Update at
// 009998D2 before the task arm. +2B0h gates the pilot's target-speed-correction raise
// (0099D8CD -> 0099D924; host reader kPilotSpeedCorrDivisorBound). The reseed clears
// the byte at the top of every think that reaches the task arm. The one think path
// that skips it is the early-out at 009998A4-009998C6 (state 2 with [unit+9C2h] set
// to 009999B0, POP/POP/RET 4: the whole update returns). That think skips the
// reseed, the task arm and the reader BSP_PilotBot_PlanControls 0099D300 (whose one
// caller is the update) together, so a 1 left from the previous think is neither
// read nor changed there, and the next full think reseeds it before the reader runs.
// Hence, wherever +2B0h is read, the binary's 1 holds only for a think in which the
// not-head-on aim arm ran; it does not persist into maneuver or avoid. The host reseeds in the same order (kPilotPlanReseedBound).
// True: the not-head-on aim tick stores plan+2B0h = 1.
// False: unchanged host (+2B0h keeps its previous value, usually 0).
inline constexpr bool kSeg67bDogfightAimSpeedCorrOffBound = false;

// Packet cc13-para: the level bomber's paratrooper (MParatrooper, kind 31h) path.
// One switch for the whole mechanism, so the ordnance half (a) cannot run without
// the release half:
//  (a) 009B7CBF-009B7CF7: approach+CEh (has ordnance) also counts a rack holding a
//      31h round (007B9500 = 007B91C0(plane; 31h, 0));
//  (b) 009B7D6F-009B7E1F: with 31h, the predicted impact is 007BCCF0(unit) plus
//      velocity x approach+A8h (no +A4h fall time), then y = the aim point's y;
//  (c) 007C0E98-007C0EC9: at the issue each fired 31h rack gets 006E3F90:
//      rack+4F8h = 1, +4FCh = 007BCCF0(unit), +508h = the unit's velocity;
//  (d) 006E541F-006E5515: at each drop from a flagged 31h rack, the landing
//      target (U(-50, 50) + px, py, U(-50, 50) + pz) and the rack point's advance
//      by velocity x toRepeatTime (rack+494h); 006E55E7 clears the flag once the
//      rack's ammo is <= 0.
// NOT MODELLED (recorded gap): the MParatrooper round itself. The host's rack drop
// spawns nothing for a 31h rack (the gunnery bomb drop takes only a general-bomb
// gun), so there is no 007AB230 landing call, chute flight, soldier spawn
// (007AB590) or building capture, and none of 006E4D50's two stream-1 dispersion
// draws (006E4F91, 006E4FB1) or the round's 007AB8C0 stream-1 draw.
// False: unchanged host (no 31h ordnance, bomb-branch impact point).
inline constexpr bool kLevelBombParatrooperDropBound = true;  // ON: JM08 pair s64 (docs/CC13_PARATROOPER.md)
// 006E5479 and 006E549E: the landing target's two U(-50, 50) draws, x then z, on
// stream 0 of the shared 00BD2F10. False: no draw, the target is the rack point.
inline constexpr bool kLevelBombParatrooperDropDrawBound = true;  // ON: JM08 pair s64 (docs/CC13_PARATROOPER.md)
static_assert(!kLevelBombParatrooperDropDrawBound || kLevelBombParatrooperDropBound,
              "the paratrooper drop draws need the drop path");

// Packet cc13-para2 (option 2): the MParatrooper round itself
// (bsp/cc13_paratrooper_round.hpp). A level bomber's drop from a 31h rack
// releases the rack's hung round through the gunnery host's bomb release
// (006E0A70, 006E1F00: velocity, drift +348h, flying), 007AB230 takes the
// 006E541F landing point, and the round then runs every fixed step:
// 007AB590 (from the callback list), 007AC000 (descent, landing, slope and
// water checks, the 006F2C30 building search), 007AB360 (chute opening at
// +468h above ground, the drag/steer 007ABB30, the Damage - armour bleed into
// an enemy building's 006F1F20 -> 0095DA00, the same-side kill) and 007AB0B0
// (chute close, the CaptureDuration timer). A landed round sits in the
// building's capturer list +7DCh, which the ship AI host's capture tick reads
// as 006F6760 arm 3 (CapturePower per capturer to its side).
// False: the 31h drop spawns nothing (option 1 only).
inline constexpr bool kParatrooperRoundBound = true;  // ON: JM08 pair s64 (docs/CC13_PARATROOPER.md)
static_assert(!kParatrooperRoundBound || kLevelBombParatrooperDropBound,
              "the paratrooper round needs the drop path");
// 007AB94A in 007AB8C0: U(ParachuteOpenAltitudeMin, Max) on stream 1, once per
// round 006E3E70 hangs on a 31h rack. False: no draw; +468h is the midpoint.
inline constexpr bool kParatrooperConstructDrawBound = true;  // ON: JM08 pair s64 (docs/CC13_PARATROOPER.md)
static_assert(!kParatrooperConstructDrawBound ||
                  (kLevelBombParatrooperDropBound && kParatrooperRoundBound),
              "the paratrooper construct draw needs the round");
// 006E4F91, 006E4FB1, 006E513C, 006E51AC in 006E4D50: the four stream-1 cone and
// drift draws the bomb release takes, for a 31h round. False: no draw, no cone,
// no drift (the release velocity is 006E0A70's alone).
inline constexpr bool kParatrooperReleaseScatterDrawBound = true;  // ON: JM08 pair s64 (docs/CC13_PARATROOPER.md)
static_assert(!kParatrooperReleaseScatterDrawBound ||
                  (kLevelBombParatrooperDropBound && kParatrooperRoundBound),
              "the paratrooper release draws need the round");
// 004B102F in 004B0F80 (from 007AB6AC): the soldier animation's start phase,
// U(0, duration) on stream 0, once per soldier. False: no draw.
inline constexpr bool kParatrooperSoldierAnimDrawBound = true;  // ON: JM08 pair s64 (docs/CC13_PARATROOPER.md)
static_assert(!kParatrooperSoldierAnimDrawBound ||
                  (kLevelBombParatrooperDropBound && kParatrooperRoundBound),
              "the paratrooper soldier draw needs the round");

namespace level_bomb_paratrooper {
// 007BCCF0's divisor. The raw-memory reference model is
// plane_ordnance_release_point_world_007bccf0 (src/cc13_seg47.cpp, its own
// kReleaseHeightDivisor_00d05a38 there); the units host's
// paratrooper_release_point_007bccf0 works on slot fields, so it cannot take that
// Seg47Host signature and keeps this copy. The drop's U(-50, 50) bounds are
// seg46's kDropJitterLo_00ceca0c / kDropJitterHi_00ceb4d4.
inline constexpr double kReleaseHeightDivisor = 2.799999952316284;   // double [00D05A38]
inline constexpr int kParatrooperKind = 0x31;
// The class step scale +5Ch: the BombClass constructor's default (no Lua key).
inline constexpr float kParatrooperStepScale = 1.0f;                  // [00D7A24C]
}  // namespace level_bomb_paratrooper

}  // namespace bsp
