#pragma once
// A squadron whose ordnance is spent goes home: the B5 arm of the squadron
// command block's step 0084E010, packet cc9_squadron_spent_ordnance_rtb,
// docs/SQUADRON_LAND_TASK.md section 5ea (the read) and 5eb (the binding).
// Every descriptive name here is a hypothesis, not a recovered symbol.
//
// The image, every frame, for every squadron (0071F290's tail on the +348h
// block 0084D810 builds):
//  1. [+38h]->vtable[4] = 009F8470 -> 009F8160: when the current command is one
//     of the seven ordnance classes, no member can drop (007ED790(0) ->
//     007B9140), the class is not a fighter (vtable[18h](13h)) and
//     (sq+369h == 0 || [00E17BF2] == 0), the latch [+38h]+3Ch = 1 (009F8205).
//     009F7C90 sets the same latch when a non-fighter's rockets run out
//     (007ED920 -> 007B9400, kind 33h; 009F7CD4).
//  2. 009F7C90: latch set and 007EE6B0 (every member's top task answers its
//     break-off slot +1Ch) -> EndCommand(current, 1) (009F7CFC): stage 2.
//  3. vtable[7Ch] = 0084E010: queue stage +48h == 2, 0071BE60() < 2 and the
//     latch (B5), then (sq+369h == 0 || [00E17BF2] == 0) && sq+368h == 0 ->
//     007F16D0(&result); a non-null result clears the commands (0071D880) and
//     is issued (0071ECF0); +228h = 1.
#include <cstdint>

#include "bsp/attack_commands.hpp"

namespace bsp {

// Packet cc9_squadron_spent_ordnance_rtb. True: the three steps above run
// once per simulation step for every squadron, and B5 issues `returntobase`
// on the squadron, whose 007F16D0 resolution installs the land task (site
// arm) or the retreat task (retreat arm). False: nothing runs. With the
// environment variable BSP_SPENT_RTB_CENSUS=1 the steps run and are logged
// while the switch is false, but nothing is issued (a diagnostic only).
inline constexpr bool kSquadronSpentOrdnanceRtbBound = false;

// 009F8160's seven ordnance classes (009F818B-009F81C2).
inline bool spent_ordnance_command_class_009f8160(std::uint32_t command) noexcept {
    return command == kAttackCmdTorpedo || command == kAttackCmdDiveBomb ||
           command == kAttackCmdLevelBomb || command == kAttackCmdDropKamikaze ||
           command == kAttackCmdDepthCharge || command == kAttackCmdRocket ||
           command == kAttackCmdKamikaze;
}

}  // namespace bsp
