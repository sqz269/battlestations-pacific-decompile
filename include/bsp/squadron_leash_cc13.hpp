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
// Before that, the dogfight arm 009F8319..009F83A8 (current == 00E08F58): with
// a squadron target (00523560) whose command (vt+174h) is retreat 00E08F90 and
// no member at +3D0h (count +3CCh) inside the map (00427EB0 -> 0071C4F0), or,
// with no squadron, a plane target (0071EA00) outside the map whose +9D4h
// squadron is retreating (009F841E..009F8463), 009F83A8 runs 0071E430(current, 1)
// EndCommand. Either way it then falls into 009F83AD.
//
// True: GameUnitsHost::run_squadron_free_attack_009f8470 runs the dogfight arm
// and then the leash gates on the squadron leader's director, before its tick.
// The arm's end is GameUnitsHost::end_command_0071e430(member, current, true) on
// every live member. The out-of-map test is FreeAttackWorld::outside_map_0071c4f0
// (point_outside_world_map_0071c4f0). The leash raises stage 2 on every live
// member, since the host's members each hold the override. 009F7FE0 is
// cc13_seg68's body over the free-attack arena
// (squadron_within_guard_distance_009f7fe0); 0080DC70 is director_allows_fire_and_move.
// 009F8407's 009F7810 is not run. Its only output is the selector target at +34h.
// The leash fires only in mode 2 (0071BE40 answers the override there, so
// 009F83C4 can differ), and the same tick's 009F849F then jumps to 009F8640 on
// mode != 1, so that tick never reads +34h. 009F8487 clears it on the next tick
// before the 009F8520 read, and its interval store at +38h has no reader.
// LABELLED.
// False: neither arm, as before.
//
// Independent of kSquadronStopHeadMoveToSelfBound. With that switch on, the
// head under the override is the moveto to the squadron's own position at the
// pick (00427EB0([ESI+8]) at 009F85FE), so the circle is centred there. With it
// off the head is `stop`, whose descriptor gives the point, or no point (then
// the leash never fires).

namespace bsp::game {

inline constexpr bool kSquadronDogfightLeashBound = true;   // ON: OCEAN_WAVE_FIELD 95 (cc9-physics23)

// [00CF29A8] = 1.25f, the FLD dword at 009F83D4 pushed as 009F7FE0's scale.
inline constexpr float kSquadronLeashGuardScale = 1.25f;

}  // namespace bsp::game
