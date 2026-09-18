// The float block the ship AI's ring-slot scorer and standoff choice read
// through brain+0AB0h.
//
// Packet cc8_ship_ai_approach_slot_tune, worker agent/cc8-ship-approach-curves.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was READ-ONLY. Every descriptive name here is a hypothesis, not a recovered
// symbol. docs/SHIP_AI_APPROACH_SLOT_TUNE.md carries the evidence.
//
// Where it comes from. brain+0AB0h is a POINTER, not an inline block: 009E6EBD
// loads the brain from [nested+0h], 009E6EBF loads [brain+0AB0h] into EAX and
// 009E6EC5 reads [EAX+1Ch]. The only store to brain+0AB0h in the image is
// 009F11AA `MOV [ESI+0AB0h],EAX` inside BSP_ShipAi_BrainRecordConstruct, and
// its source is [unit+73Ch] (009F11A4), the field beside [unit+538h] that the
// same constructor copies to brain+0AACh.
//
// [unit+73Ch] is filled once, per unit, by BSP_UnitVehicleBase_Construct: the
// allocator at 0081F200 returns the block and 0081F214..0081F27D writes ten
// constants into it from .rdata, then 0081F28C stores the pointer. On the
// failed-allocation arm (0081F283 XOR EAX,EAX) the pointer is null and every
// reader would fault, so the image treats the block as always present.
//
// There is no Lua key and no string anywhere near it. The values are immediates
// compiled into the constructor, which is why no loader on main produces them
// and why every unit in a mission gets the same block.
//
// What is NOT established: whether any later writer changes a field through the
// pointer. This packet found the one producer of the pointer and the one
// producer of its contents; it did not enumerate writers through [unit+73Ch].

#ifndef BSP_SHIP_AI_APPROACH_TUNE_HPP
#define BSP_SHIP_AI_APPROACH_TUNE_HPP

#include <cstddef>
#include <cstdint>

namespace bsp {

// 009F1160, BSP_ShipAi_BrainRecordConstruct; the store is at 009F11AA.
inline constexpr std::uint32_t kShipAiTunePointerStoreAddress = 0x009F11AAu;
// 0081ED40, BSP_UnitVehicleBase_Construct; the fill is 0081F200-0081F28C.
inline constexpr std::uint32_t kShipAiTuneFillAddress = 0x0081F200u;
// unit+73Ch, the field the brain copies into brain+0AB0h.
inline constexpr std::size_t kUnitOffApproachTuneBlock = 0x73C;
inline constexpr std::size_t kBrainOffApproachTuneBlock = 0xAB0;

// The block as its readers use it. Only the offsets with a reader or a writer
// are members; the stores reach +28h and one of them is a byte at +21h, so the
// allocation is at least 2Ch bytes and this is not its whole shape.
struct ShipAiApproachTune {
    // +00h. 009E81FA, the divisor-normalised slot score's scale:
    // slot+2Ch = slot+18h / running_max * tune+0h.
    float slot_score_scale = 0.0f;
    // +04h. Read three times: 009E7489 as the scorer's weight, 009E784B as the
    // selection's reject penalty, and 009E5D8C where the approach constructor
    // forms 1.0 - tune+4h (SUBSS from the 1.0 at 00D7A208).
    float slot_weight = 0.0f;
    // +08h. 009E964E, the avoidance strength.
    float avoid_strength = 0.0f;
    // +0Ch. The avoidance span beside it.
    float avoid_span = 0.0f;
    // +10h. 009E75F2, the bearing term of the evade score.
    float evade_bearing = 0.0f;
    // +14h. The evade weight beside it.
    float evade_weight = 0.0f;
    // +18h. The evade span, a half-angle in radians.
    float evade_span = 0.0f;
    // +1Ch. 009E6EC5. When it is at or above zero the standoff choice takes it
    // verbatim and skips every other arm, including the 119-step curve scan;
    // the installed -1.0f is what keeps that scan alive.
    float range_override = 0.0f;
};

// The ten constants BSP_UnitVehicleBase_Construct writes, in its own order.
// Every one is an immediate from .rdata, addresses in the doc.
ShipAiApproachTune ship_ai_approach_tune_defaults_0081f200() noexcept;

}  // namespace bsp

#endif  // BSP_SHIP_AI_APPROACH_TUNE_HPP
