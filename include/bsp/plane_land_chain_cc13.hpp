#pragma once
// Packet cc13-landc: adoption batch C, the land / park / re-takeoff chain. Each
// switch below is OFF; the host sites are in src/game_hosts_units.cpp.
// The names are hypotheses, not recovered symbols.

namespace bsp {

// 007C11E0 BSP_Plane_NotifyFlightState(0) at the batch's transition sites:
// 007C1680 (5 -> 4), 007CC7A0 (state 2), 007CC820 (state 1, and its copy in
// 007CC8B0's stock return), 007CB5F0 (the touchdown's authority arm, 7 -> 4),
// 007C5F60 (state 2 on the launch spot) and 007CA3F0 (through 007C1430,
// 4 or 5). Every one of them PUSHes 0 (007C16E8, 007CC7DE, 007CC853,
// 007CB72F, 007C6278, and 007C14B4 with EBX = 0).
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
// False: the sites record 007C11E0 and the poll sets the wing target one fixed
// step later, as before.
inline constexpr bool kPlaneFlightStateNotifyCc13Bound = false;

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
inline constexpr bool kPlaneRetireOnDestroyedDrawCc13Bound = false;

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
inline constexpr bool kPlaneCombatDeathRetireDrawCc13Bound = false;

// 007CA3F0 BSP_Plane_EnterGroundStateFromLocked: after 007C1430(4 + flag) and
// the stores +9F0h = 0, +9F4h = 1.0, +9ECh / +9E4h / +9E8h = 0 (007CA4D3-
// 007CA507), 007CA4D8 tests the plane's +5Dh and 007CA50F JNZ 007CA5D5 skips
// the rest: the site occupancy (vtable[24h] 006CED90), the controller
// +48h..+50h and 007D9C80, the takeoff task 0099A4A0, and the game+193Ch clear.
// The host's +5Dh is state->simulate, written by the death-flags flush
// (kPlaneDeathFlagsBound) and the stock-return destroy (kStockReturnDestroyBytesBound).
// True: ground_state_from_locked_007ca3f0 returns there for such a plane.
// False: the tail runs for it, as before.
inline constexpr bool kPlaneGroundStateDeadTailCc13Bound = false;

}  // namespace bsp
