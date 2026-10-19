# cc13-landc: plane land / park / retire switches

Packet cc13-landc. All five switches are in `include/bsp/plane_land_chain_cc13.hpp` and are
OFF. The host sites are in `src/game_hosts_units.cpp`. The header comments carry the full
byte evidence; this page is the summary. Names are hypotheses, not recovered symbols.
Status: reconstructed, build-tested with every switch OFF and compile-checked ON. None has
had a game run with the switch ON.

## kPlaneFlightStateNotifyCc13Bound

007C11E0 BSP_Plane_NotifyFlightState(0) runs as `flight_state_notify_007c11e0` at these
call sites:

| Site | Caller | Transition |
|---|---|---|
| 007C16EA | 007C1680 | 5 -> 4 |
| 007CC7F4 | 007CC7A0 | state 2 (elevator take) |
| 007CC869 | 007CC820 | state 1, also the copy in 007CC8B0's stock return |
| 007CB733 | 007CB5F0 | touchdown authority arm, 7 -> 4 |
| 007C628A | 007C5F60 | state 2 on the launch spot |
| 007C14C5 | 007C1430 | 4 or 5, from 007CA3F0 |

The calls, as bytes: 007C16EA CALL 007C11E0, 007CC7F4 CALL 007C11E0, 007CC869 CALL 007C11E0,
007CB733 CALL 007C11E0, 007C628A CALL 007C11E0, 007C14C5 CALL 007C11E0.
One site is left to the per-step poll on purpose: 007C1771 CALL 007C11E0, where 007C16F0
does 4 -> 5. The a/b targets do not change between 4 and 5,
and 007BC550's enable arm only runs on a plane whose +5Ch is clear.

Effects that only happen ON, through 007BC550:
- 007CC7A0 re-enables the parked plane, so it becomes active and hittable.
- The stock return disables the plane and zeroes +9F0h and the velocities before the
  Kill. From that step on, the slot walk, the 00836920 director walk and ship AI
  targeting all skip the plane.

00922F30 / 00922F80 are only partly modelled. The host does not carry the +5Eh gate,
message 52h, or the recursion over the child list.

## kPlaneRetireOnDestroyedDrawCc13Bound and kPlaneCombatDeathRetireDrawCc13Bound

These take 007BCAA0 OnDestroyed's 00BD2F10 draw at 007BCABB: stream 0, the
DeadEngineFireDelay pair. The first switch covers the stock return's Kill(5). The second
covers a plane that dies through the death message chain. The image takes the draw at
its 009273A0 flush, but the host takes it at its own flush point.

## kPlaneGroundStateDeadTailCc13Bound

007CA3F0 tests +5Dh at 007CA4D8, and 007CA50F jumps past the rest of the tail for a
flushed plane. The host returns at the same point. This is reachable only rarely.

## kPlaneFunnelKillDestroyOnlyCc13Bound

The retreat leave-map Kill(4) is `007F3418 CALL 00926D90`. 00926D90 dispatches through
vtable[70h] to 0077D1A0, which then calls 00926C80 Destroy. SetHealth is never called,
so 007CA8A0 never runs. With the switch ON, a plane whose host kill cause is not 1:
- skips the mode draw and the mode;
- leaves C3Ah clear;
- keeps the flush and the squadron removal;
- takes the 007BCABB draw;
- logs `plane funnel kill destroy only: unit=... cause=N` instead of `plane death mode`.

The summary lines are "summary plane land chain", "summary plane combat death draw",
"summary plane ground state dead tail" and "summary plane funnel kill destroy only".
