#ifndef BSP_SHIP_TORPEDO_VER_CC13_HPP
#define BSP_SHIP_TORPEDO_VER_CC13_HPP

// Packet cc13-ver23: the ship and submarine torpedo launch decision, read from
// the bytes for the host-versus-image verification (docs/CC13_VER_23.md).
//
// Every name is a hypothesis, not a recovered symbol. The structs are not
// binary layouts; the offsets in the comments are the native ones. These are
// pure models: nothing in the host calls them. The callers belong in
// src/game_hosts_gunnery.cpp, which cc9 owns, so wiring them is an adoption
// note.
//
// The TorpedoBot tick's think timer, range gates A and B and no-solution exit
// are modelled by gameplay_seg40_59_cc13::torpedo_bot_tick_gates_008fff20
// (docs/CC13_GPR_40.md 1.2), and its 00901BA0 attack range by
// torpedo_bot_attack_range_00901ba0 with an injected 008387B0. This file binds
// that injection to the reconstructed 008387B0 and adds the gates that file
// does not have.

#include "bsp/avoid_zone_geometry.hpp"
#include "bsp/gameplay_seg40_59_cc13.hpp"
#include "bsp/gameplay_settings_tail.hpp"

#include <array>
#include <functional>

namespace bsp {
namespace ship_torpedo_ver_cc13 {

// ---------------------------------------------------------------------------
// 00901BA0 bound to the real 008387B0 / 00838530.
// 00901BA0, __thiscall(bot)(float size, bool fire_target), RET 8:
//   00901BAD  CALL 00731020([bot+58h]+3F4h)   the tube class range, classDesc+60h,
//                                             WaterTravelSpeed * FlyTime * 0.6 (00855A90)
//   00901BB6  CMP byte [ESP+10h],0            the flag
//   00901BC0  [[00E1998C] + (bot+34h + 1)*14h]   FireTargetAccuracy (row +8h)
//   00901BD3  [[00E1998C] + bot+34h*14h + 18h]   AnyTargetAccuracy  (row +0Ch)
//   00901C0B  CALL 008387B0(7, size, range, accuracy) with ECX = 00424C40()
// 008387B0 kind 7 takes the Torpedo profile (+2F0h, shipglobals.lua
// WeaponHitAccuracy.Torpedo) and returns 00838530(profile, size, accuracy) *
// range, one float store.
// ---------------------------------------------------------------------------
float torpedo_attack_range_00901ba0(const WeaponHitAccuracyProfile& torpedo_profile,
    const TorpedoBotParameters& level_row, float target_size,
    bool fire_target_06c, float tube_class_range_00731020) noexcept;

// ---------------------------------------------------------------------------
// The attack range cache bot+68h and the fire-target flag bot+6Ch, and the
// order the gun updates them in.
// 00727F10 BSP_Gun_SetBotFireTarget(target, flag), for the TorpedoBot slot
// gun+39Ch:
//   00727F44  bot->vt[38h](target)   = 009035A0
//   00727F56  bot->vt[3Ch](flag)     = 00903580
// 009035A0 (slot 38h): changed = (00521EA0(bot+38h) != target); 006DF170 stores
//   the target; when changed, 00903420 recomputes bot+68h with the CURRENT
//   bot+6Ch (00903554 MOVZX ECX,[EBX+6Ch]).
// 00903580 (slot 3Ch): when [bot+6Ch] != flag, CALL 00903420 at 0090358D and
//   only then MOV [ESI+6Ch],BL at 00903592. The recompute therefore uses the
//   OLD flag, and the cache keeps the old flag's range until the next target
//   change. Whether the designers meant this is not established; it is what the
//   bytes do.
// 008FF260 seeds bot+68h = 10000.0f (00CE3D64); 008FF2B0 (ClearFireTarget,
//   slot 30h) writes bot+6Ch = 0 at 008FF2DF and does not recompute.
// ---------------------------------------------------------------------------
inline constexpr float kTorpedoBotInitialAttackRange = 10000.0f;  // 00CE3D64

struct TorpedoBotRangeCache {
    float attack_range_068 = kTorpedoBotInitialAttackRange;
    bool fire_target_06c = false;
    unsigned recomputes = 0;  // calls of 00903420, for a host census
};

// range_for_flag(f) is 00903420 for the current target with bot+6Ch == f,
// that is 00901BA0(00903420's size, f).
void torpedo_gun_set_bot_fire_target_00727f10(TorpedoBotRangeCache& cache, bool target_changed,
    bool flag, const std::function<float(bool fire_target_06c)>& range_for_flag);

// ---------------------------------------------------------------------------
// 008FFF20 008FFFB2..0090002A: a submarine target is dropped for a surface
// shooter that cannot reach it.
//   008FFFB2..008FFFC6  target answers IsKindOf(8), else skip
//   008FFFC8..008FFFE4  walk [gun+3Ch] parents to the first IsKindOf(5): the owner
//   008FFFF0..00900005  target still present, IsKindOf(8), owner found
//   00900007..00900014  owner IsKindOf(8)   -> keep (a submarine shooter)
//   00900018            00852820(target)    -> keep when the boat is above
//                       ([+1204h] + [+1200h]) / 3.0 (00D7A2B0, double)
//   00900021..0090002A  bot->vt[38h](0): the target is dropped, and the tick
//                       goes on to the role gate at 0090002C with no target
// target_above_00852820 is 00852820's answer (gameplay_seg53_cc13::
// submarine_is_above_depth_charge_depth_00852820); NaN answers false (JBE).
// ---------------------------------------------------------------------------
struct SubmergedTargetInputs {
    bool has_target = false;
    bool target_is_submarine_8 = false;
    bool owner_found_5 = false;         // the parent walk found an IsKindOf(5)
    bool owner_is_submarine_8 = false;
    bool target_above_00852820 = false;
};
bool torpedo_bot_drops_submerged_target_008fffb2(const SubmergedTargetInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 00416CD0, __thiscall(zone)(const float2* p0, const float2* p1) -> bool in AL,
// RET 8, body 00416CD0-00416DAC. True means the segment keeps out of the zone.
//   00416CE9  004F2B00 -> 0085C910(zone+14h min, +1Ch max, p0, p1): no box
//             contact -> true (00416D99)
//   00416CF9  00416B50(zone, p0) inside -> false (00416DA4)
//   00416D09  00416B50(zone, p1) inside -> false
//   00416D16..00416D97  for each corner c from the first, with prev starting
//             at the LAST corner: 004F3730(ECX = prev, EDX = c, p0, p1, &out)
//             accepts -> false; prev = c
//   00416D99  true
// A zone with no corners reads base[-1] at 00416D1B before its empty-loop exit;
// the projection skips the read and answers true as the exit does.
// ---------------------------------------------------------------------------
bool avoid_zone_segment_clear_00416cd0(const AvoidZonePolygon& zone,
    const std::array<float, 2>& p0, const std::array<float, 2>& p1) noexcept;

// 00417940, __thiscall(group)(p0, p1) -> bool, RET 8: every zone of the group
// answers 00416CD0 true (00417964 / 0041796B: the first false returns false).
bool avoid_zone_group_segment_clear_00417940(const AvoidZoneLayerGroup& group,
    const std::array<float, 2>& p0, const std::array<float, 2>& p1) noexcept;

// ---------------------------------------------------------------------------
// 008FFF20 0090088E..009008EB, after the jitter and the +634h inhibit and
// before the launch command at 009008F1:
//   0090088E  004218E0()                    the avoid zone manager
//   00900893  004120D0(5)                   the group for layer 5 or below
//   0090089E  no group -> launch
//   009008A0..009008DA  p1 = the lead point (x, z) [ESP+70h], [ESP+60h] (the
//             008FBB00 answer plus the unit+6D4h spread, 009002AB..009002BF);
//             p0 = the gun position (x, z) [ESP+3Ch], [ESP+44h]
//   009008DF  00417940(group, &p0, &p1)
//   009008E6..009008EB  false -> 00900974, gun->vt[1E8h](0): the trigger drops
// The line is the gun to the aim point, not the 1000 m run line the friendly
// scan uses (0090058A).
// ---------------------------------------------------------------------------
inline constexpr std::int32_t kTorpedoLaunchZoneLayer = 5;  // 00900893 PUSH 5

bool torpedo_launch_zone_clear_0090088e(const AvoidZoneTable* table,
    const std::array<float, 2>& gun_xz, const std::array<float, 2>& lead_xz) noexcept;

}  // namespace ship_torpedo_ver_cc13
}  // namespace bsp

#endif  // BSP_SHIP_TORPEDO_VER_CC13_HPP
