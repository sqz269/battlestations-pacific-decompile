#pragma once

#include <cstdint>

#include "bsp/unit_rudder.hpp"

namespace bsp {
// How the ship AI's control block reaches the unit, and what `unit+61h`,
// `unit+0FC4h` and `unit+0FDCh` actually select. Semantic interfaces, not
// native object layouts. Names are hypotheses, not recovered symbols.
// Native addresses, evidence and uncertainty: docs/UNIT_AUTOPILOT_PAIR.md.

// ---------------------------------------------------------------------------
// The double-buffered AI order slot inside the unit
// ---------------------------------------------------------------------------

// 009F4D10 addresses its destination as `unit + 0AECh - 54h * [unit+0B40h]`
// (009F4D2F IMUL 0x54, 009F4D38 SUB, 009F4D3B ADD 0AECh), so the two slots are
// `unit+0AECh` (index 0) and `unit+0A98h` (index 1), 54h = 84 bytes apart.
// The base is the unit and not the AI object: 0081F1EC in
// BSP_UnitVehicleBase_Construct stores the unit's own pointer at `unit+0B3Ch`,
// which is slot 0's `+50h`, and 0081EFC8 initialises the index at `unit+0B40h`.
inline constexpr std::uint32_t kUnitAiOrderSlot0 = 0x0AECu;
inline constexpr std::uint32_t kUnitAiOrderSlotStride = 0x54u;
inline constexpr std::uint32_t kUnitAiOrderIndexField = 0x0B40u;

// The fields of one slot this packet read. +44h and +4Ch are shared with
// UnitHeadingTargetState in bsp/unit_rudder.hpp, which is what 00811960 writes;
// this struct names the two neighbours 009F4D10 fills in the same pass.
struct UnitAiOrderSlot {
    float distance_40{0.0f};   // +40h, from blk+32Ch
    float heading_44{0.0f};    // +44h, written by 00811960 from blk+324h
    float distance_48{0.0f};   // +48h, from blk+330h
    bool valid_4c{false};      // +4Ch, set to 1 twice by 009F4D10
};

struct ShipAiPublishHost {
    virtual ~ShipAiPublishHost() = default;
    // 009F4D2F, [unit+0B40h]: which of the two slots 009F4D10 writes.
    virtual int order_slot_index_0b40() = 0;
    // 009F4D48, 00811960 on that slot with blk+324h. Its reconstruction is
    // unit_set_heading_target_00811960 in bsp/unit_rudder.hpp; the host owns
    // the unit lookups that routine makes.
    virtual void set_heading_target_00811960(UnitHeadingTargetState& state,
                                            float desired_heading) = 0;
    // 009F4D71, 009F4D78 and 009F4D87: 009F0100(blk, seconds), 009EF350(blk)
    // and 009EF910(blk, seconds). Bodies unread: contract unread.
    virtual void tail_009f0100(float seconds) = 0;
    virtual void tail_009ef350() = 0;
    virtual void tail_009ef910(float seconds) = 0;
};

// 009F4D10: __thiscall(blk)(float), RET 4, body 009F4D10-009F4D90, complete
// apart from the three unread tail callees. `blk+33Ch` is set to -1.0f
// (00D7A260, loaded at 009F4D10 and stored at 009F4D27) before anything else;
// that field is returned so a caller can carry it without this module owning
// the whole control block.
struct ShipAiPublishResult {
    int slot_index{0};
    UnitAiOrderSlot slot{};
    float blk_33c{0.0f};
};
ShipAiPublishResult ship_ai_publish_order_009f4d10(float heading_target_324,
                                                   float distance_32c,
                                                   float distance_330,
                                                   float seconds,
                                                   ShipAiPublishHost& host);

// 00825F2C..00825F7C, the head of BSP_UnitInstance_UpdateShipMotion. When the
// current slot carries a valid order it clears the flag, flips the index
// (`1 - index`, 00825F51/00825F56) and calls 00811D10 with the slot it just
// left as the source and the new current slot as `this`.
//
// Correction (packet cc_ai_order_hop): that copy does NOT carry the order.
// 00811D10's body runs 00811D14..00811D71 over the record's +00h..+3Fh and
// stops, so +40h, +44h, +48h and +4Ch - every field of UnitAiOrderSlot - keep
// whatever the destination already held. After the flip the published order is
// the slot at `unit + 0A98h + 54h * index`, which is what its readers address.
// unit_ai_order_copy_00811d10 in bsp/ship_ai_navigation.hpp projects the copy.
// This routine therefore leaves both slots' order fields alone.
struct UnitAiOrderPromotion {
    int index{0};
    UnitAiOrderSlot slots[2]{};
    bool promoted{false};
};
void unit_promote_ai_order_00825f2c(UnitAiOrderPromotion& state);

// ---------------------------------------------------------------------------
// What unit+61h selects
// ---------------------------------------------------------------------------

// The pair the unit's ordered throttle and rudder are refilled from each ship
// motion step. 008266C1 tests `unit+61h`; when it is set, 00826708 and
// 0082674C overwrite every order-ring slot and `unit+980h`/`unit+984h` from
// `unit+0FC4h`/`unit+0FDCh`. 008350BD makes the same choice for the propellers.
enum class UnitOrderedPairSource : int {
    OrderRing = 0,     // unit+980h / unit+984h keep whatever the ring holds
    AutopilotPair = 1, // unit+0FC4h / unit+0FDCh overwrite the ring
};
UnitOrderedPairSource unit_ordered_pair_source_008266c1(bool unit_flag_0061) noexcept;

// 009F50FC: the same byte is the ship AI controller's third bail-out gate, so
// the two are mutually exclusive. The AI runs only while the byte is clear, and
// the autopilot pair is read only while it is set.
bool ship_ai_runs_with_autopilot_pair() noexcept;

}  // namespace bsp
