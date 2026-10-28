#pragma once

// Packet cc13-aimaxis (substitution census S4).
namespace bsp {

// kAimLeadBodyAxisCc13Bound (OFF): the target velocity in the lead tail of
// GameUnitsHost::Impl::aim_point_009fada0 (009FADA0's tail calls 008120E0, which
// calls vtable[34h] = 00812090 on the target).
//
// 00812090 (00812090-008120CF, __thiscall(unit, float* out), RET 4):
//   s = 0092D730([unit+1018h]) (FSTP float 0081209F), then
//   out.x = +94h * s (FMULP 008120B3, FSTP float 008120B7),
//   out.y = +98h * s (FMUL 008120BF, FSTP float 008120C1),
//   out.z = s * +9Ch (FMUL 008120C4, FSTP float 008120CB).
//   +94h..+9Ch is row 2 of the unit's +74h matrix, the body forward axis. Each
//   component is one float product rounded once to float, so float * float in C++
//   is exact; there is no PC24 chain.
//
// ON: velocity = motion.pose_row2 * speed, the same axis the host already reads two
// lines above for 0092D730's speed. LABELLED: +74h is the unit's LOCAL matrix and
// pose_row2 is the host's WORLD row. The two are equal only for a root entity (no
// parent). For a child entity the two would differ by the parent's rotation.
// OFF: the level heading direction (sin h, 0, cos h) * speed, as before.
//
// Not switched: the other 00812090 stand-ins keep the level heading in
// game_hosts_ship_ai.cpp. These are the neighbour_world_velocity helper (~9665),
// the torpedo walk (~2342), the 009E2773 site (~4040) and the site at ~10092.
// Each has its own copy and none shares a helper with the aim lead.
//
// Movers when ON: the lead aim points of the shooters (ships, torpedoes and planes
// through aim_point_009fada0's callers). The targets are the hull-sampling classes,
// whose vtables carry 008120E0 at +48h. Against a pitching target the lead gains
// the forward axis's y component and loses the level direction's horizontal length.
// For a level hull, pose_row2 equals (sin h, 0, cos h) up to rounding, so the lead
// moves little.
//
// ON since cc9-nondet2 (2026-10-28): plausible on every pair (docs/USN21_NONDETERMINISM.md 7).
inline constexpr bool kAimLeadBodyAxisCc13Bound = true;

}  // namespace bsp
