#pragma once
// Packet cc13-landc: adoption batch C, the land / park / re-takeoff chain. Each
// switch below is OFF; the host sites are in src/game_hosts_units.cpp.
// The names are hypotheses, not recovered symbols. Summary: docs/CC13_LANDC.md.

namespace bsp {

// 007C11E0 BSP_Plane_NotifyFlightState(0) at the batch's transition sites:
// 007C1680 (5 -> 4), 007CC7A0 (state 2), 007CC820 (state 1, and its copy in
// 007CC8B0's stock return), 007CB5F0 (the touchdown's authority arm, 7 -> 4),
// 007C5F60 (state 2 on the launch spot) and 007CA3F0 (through 007C1430,
// 4 or 5). Every one of them PUSHes 0 (007C16E8, 007CC7DE, 007CC853,
// 007CB72F, 007C6278, and 007C14B4 with EBX = 0).
// Left to the per-step poll on purpose: 007C16F0's call (007C176F PUSH 0 /
// 007C1771 CALL 007C11E0, 4 -> 5). From 4 to 5 the a/b targets do not change
// (both are 1 in 4 and in 5), 5 takes neither the zeroing nor +C0Ch, and
// 007BC550's enable arm runs only with +5Ch clear, which a state-4 plane is not.
// Body 007C11E0-007C1429, immediate == 0, with s = +900h after the store:
//   a = !(s == 6 || (s == 7 && !(5.0f >= +908h)))  (007C11E9-007C1208; JC, so a
//       NaN +908h gives a = 0)
//   b = s in {7, 6, 4, 5}                          (007C120A-007C1223)
//   [+DECh]+29h (the gear channel, host channel_b) = a and [+DECh]+45h (the
//   wing channel, host channel_a) = b, each setting its moving byte when it
//   changes, and either moving byte sets +11h (007C12E0-007C1314). The values
//   are not touched.
//   s none of 7/6/4/5/3 (007C1318-007C1345): +9F0h = 0.0f, the flight
//   controller's +3Ch..+44h and +48h..+50h = the vector at 00F87574, then
//   007D9C80, 007DDEA0 and 007EB380(0.0f) (007C134B-007C13C5).
//   Tail: s in {7, 6, 3} -> +C0Ch = 1 (007C13CB-007C140C; the 4 arm is not
//   reachable there); s == 1, s == 0 or +5Ch == 0 -> 007BC550 (007C1412-
//   007C1423), which for s 0/1 disables the member (00922F80(0), 00951F40(0))
//   and otherwise, with +5Ch clear, enables it (00922F30(0), 00951F40(1),
//   00779C60(1.0f)), both only with the Lua self +4A4h set.
// True: the host runs that body at those sites (GameUnitsHost::Impl::
// flight_state_notify_007c11e0). The per-step poll wing_fold_notify_007c11e0
// still covers the other call sites; a notify here records the state it saw,
// so the poll does not repeat it.
// ON effects beyond the actuator targets, from the 007BC550 tail:
//   007CC7A0 (the elevator take into state 2): the parked plane was disabled in
//   state 1, so the enable arm runs: state->active = 1, row.active = true, the
//   hit node re-attached (plane_hit_node_detached = false), 00779C60 recorded.
//   A plane coming down the elevator becomes active and hittable; OFF does
//   nothing there.
//   007CC8B0's copy of 007CC820 (the stock return, s = 1), before the Kill: the
//   disable arm (state->active = 0, row.active = false, hit node detached) and
//   +9F0h, the world and angular velocities zeroed. The stow (007B96C0 at the
//   elevator, 006D0738) only detaches the node, so under OFF the plane is still
//   active here. Readers of active / row.active after the Kill:
//   kill_unit_00926d90 -> the gunnery kill_unit, note_kill_cause_00926e02 and
//   target_gone_00925c40 read neither; the squadron counts (script orders
//   +3CCh, 007F4E19's member scan) do not see the plane, which the stock return
//   took out of member_units first; the liveness tests that also check +5Dh
//   are already false (kStockReturnDestroyBytesBound sets it). What changes:
//   the units host's per-step slot walk and the 00836920 director walk skip
//   the slot from that step on, and ship AI auto-targeting (its row->active
//   filter) drops it, and a ship whose locked fire target it is releases it at
//   once instead of keeping a dead row.
// UNCERTAIN: the retire draw timing (kPlaneRetireOnDestroyedDrawCc13Bound): the
// image draws at the later destroy flush (00926C80 queues onto the F899A8
// list), not right after the Kill. The gear channel_b is never initialised, so
// ON can leave moving_any set for good (no reader found). If the notify comes
// before the first per-step poll, the wing target snaps from the notify's state
// rather than the poll's, one step earlier.
// NOT FULLY MODELLED: 00922F30 (enable) and 00922F80 (disable) are carried only
// as +5Ch (state->active / row.active) and the hit node. In the image both
// return at once when +5Eh is set (00922F33 / 00922F83) or +5Ch already has the
// target value; both dispatch vtable[68h] / [6Ch] (0077D500 / 0077D580), which
// build message 52h and send it through 0077C7B0 (a no-op in single player);
// and both recurse over the child list (+48h first child, +44h next sibling,
// 00922F51-00922F6E / 00922FB4-00922FB9). The host does not test +5Eh. 007CA3F0
// runs 007C1430 (and so this notify) before its +5Dh test at 007CA4D8, so with
// kPlaneGroundStateDeadTailCc13Bound either way it can reach the enable arm on
// a flushed plane (+5Dh set). The image also enables it there while +5Eh is
// clear; with +5Eh set the image returns and the host would still set
// active / row.active and re-attach the hit node.
// False: the sites record 007C11E0 and the poll sets the wing target one fixed
// step later, as before.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 104).
inline constexpr bool kPlaneFlightStateNotifyCc13Bound = true;

// 007CC8B0's Kill(plane, 5) (007F1D57 -> 00926D90 -> 00926C80 Destroy), whose
// destroy flush dispatches vtable[7Ch] = 007BCAA0 BSP_Plane_OnDestroyed for a
// plane (00D19D28+7Ch). 007BCAA0 begins with 00BD2F10(ECX = 0, [00E18718],
// [00E18714]) at 007BCABB, stream 0, stored at +C14h (007BCAC0): the
// planepartclasses.lua DeadEngineFireDelay pair, which 004A9BD0 reads
// (004AA9BD / 004AAA57).
// True: the stock return takes that draw through the gunnery host's stream-0
// draw (death_mode_draw_00bd2f10(0, ...)). +C14h itself has no host field and
// no reader on a retired plane, so only the draw is carried.
// False: the retired plane draws nothing, as before.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 104).
inline constexpr bool kPlaneRetireOnDestroyedDrawCc13Bound = true;

// The same 007BCABB draw for a plane that dies through the death message
// chain (00877B90 SetHealth -> 007CA8A0 -> 007BBFA0 -> 007D0B80, whose tail
// 007D12FC calls vtable[70h](1)): the destroy flush that follows dispatches
// 007BCAA0. Stream-0 order per plane: 007BC3C7 (ExplosionExplosionDelay, only
// for a delayed explosion), then 007BCABB. Stream 1 (007CA914's mode draw,
// 007BC1F2, the 007CAAD0 debris draws) is untouched.
// True: the host's death-mode block (the plane's first step after the gunnery
// death) takes the draw right after its own flush. The image flushes at
// 009273A0 after every unit's motion, so with two planes dying in one step
// the shared stream-0 order is A delay, A 007BCABB, B delay, B 007BCABB here
// against A delay, B delay, A 007BCABB, B 007BCABB in the image; under
// BSP_GUNNERY_RNG_STREAMS=1 each plane has its own stream-0 key and the per-
// plane order is the image's.
// False: no draw, as before.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 104).
inline constexpr bool kPlaneCombatDeathRetireDrawCc13Bound = true;

// 007CA3F0 BSP_Plane_EnterGroundStateFromLocked: after 007C1430(4 + flag) and
// the stores +9F0h = 0, +9F4h = 1.0, +9ECh / +9E4h / +9E8h = 0 (007CA4D3-
// 007CA507), 007CA4D8 tests the plane's +5Dh and 007CA50F JNZ 007CA5D5 skips
// the rest: the site occupancy (vtable[24h] 006CED90), the controller
// +48h..+50h and 007D9C80, the takeoff task 0099A4A0, and the game+193Ch clear.
// The host's +5Dh is state->simulate, written by the death-flags flush
// (kPlaneDeathFlagsBound) and the stock-return destroy (kStockReturnDestroyBytesBound).
// True: ground_state_from_locked_007ca3f0 returns there for such a plane.
// False: the tail runs for it, as before.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 104).
inline constexpr bool kPlaneGroundStateDeadTailCc13Bound = true;

// A plane killed through 00926D90 BSP_MissionEntity_Kill: the retreat leave-map
// Kill(4) (007F3418 PUSH 4 / CALL 00926D90, and 007F31A0's other members) and
// any other funnel kill. 00926D90 sets +5Fh, stores the cause at +70h
// (00926E02) and dispatches through vtable[70h] (argument 1) to 0077D1A0 for a
// plane (00D19D28+70h = 00D19D98). 0077D1A0 builds message 4Eh only with
// [00E0AF20] set and game+1FE4h 0 or 1 (0077D1BB-0077D236; not modelled, as in
// the gunnery host's destroy_unit_0077d1a0), then 00926C80 Destroy (0077D243).
// No SetHealth, so no 007CA8A0: no stream-1 mode draw (007CA914), no 007BBFA0 timer or 007BC3C7
// delay draw, no 007D0B80 flags (C39h/C36h/C3Ah). The destroy flush then
// dispatches 007BCAA0: 007BCABB's stream-0 DeadEngineFireDelay draw and the
// 007F3970 squadron removal.
// The host's death-mode block takes every gunnery-dead plane not already
// removed, so today a retreat plane draws a mode (logged as "plane death mode",
// e.g. explosion_delayed with its stream-0 delay draw) one step after the Kill.
// The test is the host's kill cause: every cause other than 1 is stored only by
// a 00926D90 caller (note_kill_cause: the retreat arms 4, the stock return 5,
// the ship exit 4, the gunnery funnel's normalized Lua / ship AI causes).
// True: for such a plane the block skips the mode draw and the mode, does not
// set C3Ah, keeps the +60h / +5Dh flush and the squadron removal, takes the
// 007BCABB draw (whatever kPlaneCombatDeathRetireDrawCc13Bound says) and
// removes the plane. LABELLED: the draw lands one step after the Kill (the
// block's step), not at that step's 009273A0 flush. A cause-1 funnel kill (Lua
// Kill(x), 007CE3A7) cannot be told from a gunfire death and keeps the mode.
// False: the mode is drawn, as before.
// cc9-physics24 2026-10-19: ON by mechanism (OCEAN_WAVE_FIELD 104).
inline constexpr bool kPlaneFunnelKillDestroyOnlyCc13Bound = true;

// Packet cc13-killx. The cause cannot separate a cause-1 funnel kill from a
// gunfire death, in the image either: 008AC5C0 Lua Kill(x) passes 1 (008AC71B:
// the optional boolean makes it 2) to 00926D90 (008AC756, or 007ED3A3 per
// squadron member), as do the depth kill 007CE3A7 and the impact message 0CAh
// (007BBA10: 007BBA7F / 007BBAED), while SetHealth's chain reaches 00926D90
// only in 007D0B80's explosion arm (007D0CFD, +70h = 1), after 007CA8A0 drew
// the mode; that arm then falls into 007D12FC (007D0D02 sets +C10h, 007D0D12
// JMPs there), whose vtable[70h](1) is skipped once +5Dh is raised (007D1305).
// Its other arms destroy through 007D12FC's vtable[70h](1) and leave
// +70h at its constructor value, which the host takes as 1. What differs is
// the path: a 00926D90 Kill without SetHealth, against SetHealth. The host
// therefore tags every unit killed through a 00926D90 stand-in (note_kill_cause:
// kill_unit_00926d90's note_kill_cause_00926e02, the gunnery host's plane
// crash impact for 007BBA7F / 007BBAED, and the direct retreat / stock-return
// / ship-exit notes). The explosion arm is the death block's own mode (no
// separate host kill), so it stays untagged. The three host sites
// that call kill_unit_00926d90 for a direct vtable[70h](1) are not tagged: the
// water powerlost stand-ins for 007CB8DC and 007CBE02 (007BBFA0 -> 007D0B80 ->
// 007D12FC) and the submarine drowning (008553A1). 007EF078 (the hangar member
// Kill(1) with the base owner dead) is not modelled: the launch tick's gate
// (007F1F66-007F1F71) tests the same owner in the same step.
// LABELLED: the tag is written only while kKillFunnelCauseBound (gunnery host)
// calls note_kill_cause_00926e02; it is ON.
// True: the destroy-only test is the tag, for every cause (Lua Kill(x) and the
// depth kill join Kill(4); e.g. JM02's Kill(Mission.PlayerBetty)).
// False: kPlaneFunnelKillDestroyOnlyCc13Bound's cause != 1 test, as before.
inline constexpr bool kPlaneFunnelKillPathTagCc13Bound = true;
static_assert(!kPlaneFunnelKillPathTagCc13Bound || kPlaneFunnelKillDestroyOnlyCc13Bound,
              "kPlaneFunnelKillPathTagCc13Bound refines kPlaneFunnelKillDestroyOnlyCc13Bound");

}  // namespace bsp
