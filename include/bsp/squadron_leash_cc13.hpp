#pragma once
// Packet cc13-leash (cc9 -71 grant; cc9 released 009F8160's leash arm to cc13):
// 009F8160's dogfight leash, 009F83AD..009F8407.
//
// The image, on every 009F8470 tick (009F848A calls 009F8160 first):
//   009F83AD..009F83BF  the current command's category (vtable+0Ch) is 1 or 2;
//   009F83C4            slot 0 [director+54h] is not that command (an override);
//   009F83CB            0080DC70 AllowsFireAndMove answers false (+3Ch or +3Dh
//                       clear);
//   009F83D4..009F83E7  009F7FE0(director+18Ch, the override's descriptor, scale
//                       1.25f [00CF29A8]) answers false: the target is outside
//                       1.25 x GuardDist (tuning +594h) of 0071E4C0's point,
//                       horizontally (no point answers true);
// then 009F83F5 0071D9E0(2) ends the override, and 009F8407 runs 009F7810.
//
// True: GameUnitsHost::run_squadron_free_attack_009f8470 runs those gates on the
// squadron leader's director before its tick and, when they all hold, raises
// stage 2 on every live member (the host's members each hold the override).
// 009F7FE0 is cc13_seg68's body over the free-attack arena
// (squadron_within_guard_distance_009f7fe0); 0080DC70 is director_allows_fire_and_move.
// 009F8407's 009F7810 is not run: its only output is the selector target at
// +34h, which 009F8487 clears on the next tick before any read (the tick's
// 009F8520 read comes after that clear), and its interval store at +38h has no
// reader. LABELLED.
// False: no leash, as before.
//
// Independent of kSquadronStopHeadMoveToSelfBound. With that switch on, the
// head under the override is the moveto to the pick position, so the circle is
// centred there; with it off the head is `stop`, whose descriptor gives the
// point, or no point (then the leash never fires).

namespace bsp::game {

inline constexpr bool kSquadronDogfightLeashBound = false;

// [00CF29A8] = 1.25f, the FLD dword at 009F83D4 pushed as 009F7FE0's scale.
inline constexpr float kSquadronLeashGuardScale = 1.25f;

}  // namespace bsp::game
