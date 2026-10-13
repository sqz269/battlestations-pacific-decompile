// Packet cc13-ver13 (docs/CC13_VER_13.md): when a dying ship stops being a target.
// Names are hypotheses, not recovered symbols. Every answer here is read from the
// disk bytes of battlestationspacific.exe; the callers supply the entity bytes.
// 00835F60's release test already exists: director_release_fire_target_00835f60
// (include/bsp/gameplay_seg52_cc13.hpp).
#pragma once

namespace bsp {

// The `land` arm of 009F8160 BSP_Bot_RevalidateCurrentCommand (009F8295-009F82D9),
// reached when the squadron's current command (0071BE40 at 009F816A) is 00E08FA0.
struct SquadronLandTargetFacts {
    bool present{false};          // the resolved land target, ESI != 0 (009F829F)
    bool destroyed_5e{false};     // target+5Eh (009F82A1)
    bool torn_down_5d{false};     // target+5Dh (009F82A7), set at the kill step's flush
    // 00927C90(0) on [this+8]: [obj+188h] is 9 or one of 0..7. Its meaning is not
    // established (the other caller is 006C0840 BSP_AirOps_FindNearestLandingSite).
    bool slot0_valid_00927c90{false};
    int owner_side_54{0};         // [[this+8]+54h] (009F82C2)
    int target_side_54{0};        // target+54h (009F82C5)
};

// True when the arm sets this+3Ch = 1 (009F82D1), the latch 009F7C90 turns into every
// member's break-off: a null, removed (+5Eh) or dead (+5Dh) land target, or a live one
// that passes 00927C90(0) and belongs to another side. False keeps the command
// (009F8416).
bool squadron_land_target_latches_009f8295(const SquadronLandTargetFacts& f) noexcept;

}  // namespace bsp
