#pragma once

// Packet cc13-flag5d (census S6).

namespace bsp {

// kTargetDisabledByte5dCc13Bound (OFF): the level-bomb and strafe tasks read the latched
// target's +5Dh byte, not a liveness test.
//   009B88F0 (level-bomb transition): 009B8968 and 009B8AF4, CMP byte [[task+440h]+5Dh],0;
//     set means the forced mode-2 attack does not engage. 009B8D80 (break-off, run from
//     009B8B50): 009B8DA4, set (or no target) breaks off.
//   009CCED0 (strafe approach): 009CCF37, CMP byte [target+5Dh],0 with target = task+54h,
//     else task+70h; set (or no target) jumps to 009CD015 as with no target.
// Polarity: set = out of action. Image writers of entity+5Dh: 00925E14 clears it at
// creation; 0092639B (00926390, with +60h) sets it when FlushPending 009273A0 runs the
// destroy list, so a unit killed in step N reads set from the end of step N; 009274CE (the
// kill list, with +5Eh/+5Fh) sets it again; 009272EF sets it at spawn for a kind-3 Lua
// descriptor whose _entity.deadMeat is true (no shipped script authors it; save loads).
// Host mirror GameUnitSlot::state->simulate (GameUnitsHost::unit_flag_005d): cleared at
// creation, set at the destroy (kSunkShipFlushBound ships, kPlaneDeathFlagsBound planes,
// the stock-return destroy) and by store_scene_node_flags at the kill list. Not mirrored:
// the deadMeat spawn write; 00922FE1 in SceneNode_Kill (in practice it only re-sets +5Dh
// on nodes whose +5Eh is already set, or at teardown); 009263F4 in 009263C0 (no callers;
// its inline copy is 009274CE); and the 00925A73 CancelPendingDestroy clear (reached only
// from Gun_ApplyClearDestroyedFailure, on guns, which are not host slots).
// OFF: both sites take !df_slot_live(target) (+5Ch set and +5Dh/+5Eh/+60h clear) as
// "disabled". ON: the +5Dh byte alone, in game_hosts_units.cpp.
// Movers: a latched target with +5Ch clear (scene node inactive) or +60h set before its
// +5Dh store now counts as engageable, as in the image, so level bombers keep the forced
// attack and do not break off, and strafers keep their approach, on such targets.
inline constexpr bool kTargetDisabledByte5dCc13Bound = false;

}  // namespace bsp
