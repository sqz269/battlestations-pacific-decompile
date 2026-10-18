#pragma once
// Packet cc13-dead: the Lua `Dead` / `KillReason` write on a land convoy's self
// table when the convoy is killed (007422A0's zero-count 00926D90(convoy, 2)).
//
// The image's writer chain for a LandConvoy (vtable 00CEA570):
//   00926D90 Kill: +5Fh, +70h = cause (00926E02), then vtable+70h(1) at 00926E05,
//     which for the convoy is 0077D1A0; 0077D1A0 calls 00926C80 Destroy at
//     0077D243, which sets +60h and queues the convoy on the destroy list
//     00F899A8 (00926D49 / 00926D55).
//   009273A0 flush, the same pass that runs the kill list's +080h (00742180):
//     the destroy list first (vtable+74h at 0092747F, before 0092751F), so
//     00926390 -> JMP vtable+7Ch = 0077D270 (the LastBanto post needs cause 1,
//     the convoy's is 2) -> 00929B60 at 0077D3CA -> 00927F60, 00928100, then
//     00929800: when +178h (the Lua self slot) is set, `Dead` = true at 00929896
//     (00B673A0, key 00CF829C) and `KillReason` = 00E0CF04[+70h] at 009298C2
//     ("soft" for cause 2).
// 00742180's tail 00779AF0 -> 00928C80 does not write `Dead` (it nulls `Ptr`,
// docs/MISSION_ENTITY_LUA_ATTACH.md).
//
// True (and kLandConvoyBodiesBound true): when the host runs a queued convoy's
// 00742180 in the 009273A0 loop, it first writes `Dead` = true and `KillReason`
// on the convoy marker's self table (GameMissionLuaHost::
// publish_land_convoy_dead_00929800). False: a convoy's `Dead` stays the false
// its attach seeded (00928BC7), as before.

#include "bsp/land_convoy_wiring_cc13.hpp"

namespace bsp::game {

inline constexpr bool kLandConvoyDeadBound = false;
inline constexpr bool kLandConvoyDeadWired = kLandConvoyBodiesBound && kLandConvoyDeadBound;

}  // namespace bsp::game
