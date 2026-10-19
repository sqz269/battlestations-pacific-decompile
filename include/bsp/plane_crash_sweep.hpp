#pragma once

// Packet cc9_lua53_kamikaze_impact (docs/SQUADRON_LAND_TASK.md 5gc): the
// plane's own swept collision test, 007C0910 BSP_Plane_SweepCollisionPoints.
//
// The image (read from the listing, 007C0910-007C0C5F, __thiscall, RET 10h;
// called once per plane fixed step from 007CE040 at 007CEF67):
//   - 007C091C..007C0937: return 0 while the cooldown unit+9DCh is positive
//     (decremented by the step, 007C0C43) or the byte unit+61h is set. The
//     cooldown is 3.0 (00CE3854) or, for a kind-45h base, 5.0 (00CE3850)
//     after 007C5F60 places the plane on a launch spot (007C605D / 007C60EA),
//     and 1.0f from the constructor (007CFE78 MOVSS [ESI+9DCh],XMM1, XMM1 =
//     [00D7A24C] from 007CFDEA). The decrement runs only on the steps the
//     caller sweeps: unit+5Ch set, 007BC5B0 true (+900h 7, 6 or 4, among
//     others), a positive step and unit+520h clear (007CEEC1..007CEEFA).
//   - 007CEF00..007CEF21 (the caller): skipped for a live plane that is the
//     player's controlled unit ([00E188A8]+5FCh -> +908h == unit+54h).
//   - per collision point (class+5BCh, stride 0Ch: the model's "collipos"
//     dummies, else four bbox points 007D57AD..007D586C: (0, 0.96 max.y,
//     0.96 min.z), (-0.49 w, 0, 0), (0.49 w, 0, 0), (0, 0.96 min.y,
//     0.96 max.z); 0.96 = 00D04310, 0.49 = 00CF5E58), x scaled by 0.75
//     (00CEC9D8) unless the shot record's vtable[38h]() answers true; the
//     segment runs from the point under the previous pose (unit+924h) to the
//     point under the current pose (unit+CCh).
//   - 007C0ABA..007C0ACE: both ends below -1.0 (00D7A260) -> return 2 at once.
//   - 007C0AD4..007C0B53: a current end below -2.0 (00CE7D7C) is pulled back
//     along the segment to y = -2: t = (prev.y + 2.0 (00D7A308)) /
//     (prev.y - cur.y), end = prev + t (cur - prev); the 0060C5B0 test with
//     unit+900h == 6 (on the water) skips both water tests.
//   - 007C0C0A: 0084BF00(list, unit+72Ch, unit, [unit+730h], extra, &out):
//     the projectile sweep and impact, with the plane as the shot and its
//     class's KamikazeBulletClass (class+210h, copied to unit+730h at
//     007D6289) as the shot's class. Every aircraft class carries one (this
//     installation: 21 "Kamikaze Plane Impact" blast 1400..1700 range 30,
//     98 "Okha Impact" 6000..9500 range 30, 99 "Normal Plane Impact" 350..500
//     range 20; no DamageMin/Max, so the damage is the blast). Type Kamikaze
//     is sub-type 11h, so 0084BF00's exemption skips a unit of the plane's
//     own squadron (+9D4h).
//   - the caller, 007CF02D..007CF0CC: unit+9E0h set drops an entity (1) or
//     plane (4) hit; any other non-zero mode routes the impact message
//     (0077C2A0 with 007BA990's record, 5, 0) and 0090EC40; the message is
//     0CAh, 007BBA10: the bullet's effect at the point and Kill(1) for modes
//     1, 3 and 4, the water arm 007D8180 for mode 2.
//
// The host (src/game_hosts_gunnery.cpp, run_plane_crash_sweeps_007c0910):
// planes in free flight only (+900h == 7 and not landed: LABELLED, the
// host's deck, taxi, take-off and landing poses are not the image's), the origin and the two wing
// points (the bbox fallback's +-0.49 w; the host carries no collipos dummies
// and no per-plane bbox height, so the nose/tail pair is not swept:
// LABELLED), through the gunnery host's const unit segment query (the same
// 0098ADD0 binding shells use). On a unit hit outside the plane's squadron:
// the KamikazeBulletClass blast through the gunless blast path (the shot
// record's weapon scale and attribution are not carried: LABELLED) and the
// plane's death. Landscape and water ends are counted, not acted on (the
// plane water law is its own packet).
//
// False: the sweep runs as a census only (no damage, no death, no counters
// that other summary lines read).
namespace bsp {

inline constexpr bool kPlaneCrashSweepBound = true;  // ON: USN19/USN18/USNOS pairs (docs/SQUADRON_LAND_TASK.md 5gc)

// 00CE3854 (float 3.0): 007C5F60's unit+9DCh for a launch spot (5.0, 00CE3850,
// for a kind-45h base; LABELLED: the host arms 3.0 for both). The host arms it
// on a plane's first free-flight step.
inline constexpr float kPlaneCrashLaunchCooldown = 3.0f;
// 00CE3850 (float 5.0): the same store for a kind-45h (AirField) base, 007C60EA.
inline constexpr float kPlaneCrashAirfieldLaunchCooldown = 5.0f;

// 00D7A24C (float 1.0): 007CFD20's unit+9DCh (007CFE78). A plane that is never
// placed on a launch spot keeps it, so an air-spawned plane's first 1.0 s of
// swept steps is skipped, not 3.0 s. Packet cc13-cool9dc.
inline constexpr float kPlaneCrashConstructCooldown = 1.0f;

// True: the gunnery host's first arm of a plane that has been in free flight
// on every step the sweep has seen (never grounded, so never through 007C5F60)
// takes kPlaneCrashConstructCooldown; every later arm, and the first arm of a
// plane seen off free flight before it, takes kPlaneCrashLaunchCooldown.
// False: every arm takes kPlaneCrashLaunchCooldown, as before.
inline constexpr bool kPlaneCrashConstructCooldownCc13Bound = false;

// 00CEC9D8 (double 0.75), 00CF5E58 (double 0.49), 00D04310 (double 0.96).
inline constexpr float kPlaneCrashFoldedWingScale = 0.75f;
inline constexpr float kPlaneCrashWingPointRatio = 0.49f;
inline constexpr float kPlaneCrashNosePointRatio = 0.96f;
// 00D7A260 (float -1.0): both ends below it end the sweep with mode 2.
inline constexpr float kPlaneCrashSubmergedY = -1.0f;
// 00CE7D7C (float -2.0), with 00D7A308 (double 2.0) as its negation: the
// surface pull-back.
inline constexpr float kPlaneCrashSurfaceCutY = -2.0f;

}  // namespace bsp
