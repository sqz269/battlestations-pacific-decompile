#pragma once

// Packet cc13-aimfreezekz: the target ref's death freeze (009FADA0's head 009FADAE..
// 009FAE18, 009FAF90's notify) for the kamikaze, rocket and depth-charge approaches, on
// cc13-aimfreeze3's latch (GameUnitsHost::Impl::aim_point_freeze_009fada0, the shooter's
// hull_aim_ref frozen / world_point_1c, tagged by aim_freeze_task: 4 kamikaze, 5 rocket,
// 6 depth charge). See include/bsp/approach_aim_freeze_cc13.hpp for the latch itself.
//
// Image.
//   Kamikaze 009AF0A0, sub at approach+68h: 009FADA0 at 009AF0CB; with +80h (sub+18h)
//   clear the body leaves at 009AF0DE (+54h, +60h, +ACh not updated). The rule 009AF310
//   (009AF34A-009AF370) never abandons a rocket-like plane (kind 17h, +C24h clear,
//   009AB94C), and a rocket-like leader's formation state is gotowards, so gotowards
//   009AC190 and aim 009AC910 keep steering at 009ABA30(sub+1Ch, +60h): the frozen
//   death point, y >= 1 plus the altitude interpolation.
//   Rocket 007B78F0, sub at approach+58h: 009FADA0 at 007B7902, then +50h / +54h from
//   approach->vtable[0] every tick with no early-out (007B7943-007B7A0A); the range test
//   that drops +4Ch on +70h == 0 runs once per +CCh = 1.0 s (007B7A33), so for up to 1 s
//   gotowards 007B4980, aim 007B5430 and the goaway bank / done / evasive point
//   (007B5AF0, 007B63CB) read the frozen point.
//   Depth charge 009A5F50, sub at approach+7Ch: the early-out on +94h (sub+18h) at
//   009A6005 comes before 009FADA0 (009A6043), so the death tick still takes +48h range,
//   +4Ch bearing and +75h from the frozen point (009A609D-009A6102); the goaway heading
//   009A3CF0 (009A3D2D), the goaway done 009A53EF and the follow heading 009A4B76 read it
//   afterwards.
//
// Host (OFF).
//   Kamikaze and rocket (kz_target_point, game_hosts_units.cpp): a target that fails
//   df_slot_live returns false before any point is written, so the caller's point stays
//   {0, 0, 0}: a rocket-like kamikaze whose target dies flies at the WORLD ORIGIN until it
//   hits something; a rocket plane's aim, distances and goaway are measured to the origin
//   for up to 1 s.
//   Depth charge (the Adapter's update_target_ref_009fada0): aim_point_009fada0 on the dead
//   target, the live pose WITH the lead, kept as the frozen point.
//
// ON. kz_target_point (task 4 for KzHost, 5 for RocketHostAdapter; DropKamikaze keeps 0
// and is unchanged: its rule 009AE610 leaves every attack state in the death tick, so its
// point is never read) and the depth-charge adapter (task 6) call the latch instead. On a
// target that fails df_slot_live they still answer false (has_target unchanged), with the
// point the latch answers: the frozen no-lead M(+CCh) x sub+28h once +5Dh is seen (009FAF90's
// double transform when +5Eh is seen first), else aim_point_009fada0's live point (+60h or
// +5Ch alone, where the image still tracks). {0, 0, 0} remains only with no target and no
// latch. LABELLED as in cc13-aimfreeze3: one ref per shooter, the notify order inferred.
//
// Evidence of reach (USNOS, J:\PROG\battlestations-pacific-decompile-cc9-ships44\local\
// s64_cen_os.log, current host OFF): all 14 kamikaze tasks are rocket_like=1 (Betty*_kz
// Ohkas). Gear14 died at 1794.93 s (kz03's blast) with kz04 (aim since 1785.57 s, min_dist
// 774.0) and kz06 (aim since 1789.17 s, min_dist 1683.6) still on it; Gear15 died at
// 1837.87 s (kz07) with kz08 (1229.8), kz09 (1822.6) and kz10 (2615.1) still on it. kz04's
// terrain avoid min_margin is -99.4.
//
// Expected movers when ON: USNOS first (and every Ohka mission): orphaned Ohkas steer to
// their dead target's death point instead of the world origin (impacts, near misses on
// neighbours, terrain hits); rocket planes for up to 1 s after a target's death (heading,
// a possible aim release); depth-charge goaway / follow headings by the lead offset (tens
// of metres). Census: "summary aim freeze kamikaze|rocket|depth_charge" (ON only).

namespace bsp {

inline constexpr bool kApproachRefFreezePlanesCc13Bound = false;

}  // namespace bsp
