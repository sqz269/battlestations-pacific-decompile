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
//
// OMITTED (labelled): of that chain the host models only 00929800's two Lua
// writes. Not modelled, with either switch (the code points here from
// units.cpp kill_convoy and land_convoy_flush_009273a0):
//   - 0077D1A0: the session broadcast 0077C7B0 / 0077C980 (message 4Eh, gated
//     on [00E0AF20] and [[00E188A8]+1FE4h] in {0, 1});
//   - 0077D1A0: 00986480 BSP_WarningManager_FireEntityKilled at 0077D253, the
//     "entityKilled" mission event, gated on [00F8A0C4]. GAMEPLAY-VISIBLE.
//     cc9's note: only 10 scripts in this installation mention entityKilled:
//     jm01.lua (COTP-IJN, PRCPIJN, ijn/JM) and seven training-grounds scripts.
//     None is a convoy row (JM05/PRCP08) or a WINS row;
//   - 00926C80: the +48h child loop, vt+78h / vt+70h on each child
//     (00926CFF..00926D31);
//   - 00926390: the 00925C90 observer notify;
//   - 0077D270: [[00E188A8]+193Ch] = 0 and the 00803BA0 recon-dirty loop over
//     the three +1F0h records (0077D295..0077D2C4);
//   - 00929B60: the 00927F60 scoring stamp and the 00928100 table mirror, which
//     run before 00929800 (named in the chain above, not modelled).

#include "bsp/land_convoy_wiring_cc13.hpp"

namespace bsp::game {

inline constexpr bool kLandConvoyDeadBound = false;
inline constexpr bool kLandConvoyDeadWired = kLandConvoyBodiesBound && kLandConvoyDeadBound;

}  // namespace bsp::game
