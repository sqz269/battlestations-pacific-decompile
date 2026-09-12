// How the ship AI control block reaches the unit, and what unit+61h selects.
// Addresses, ABI, coverage and uncertainty: docs/UNIT_AUTOPILOT_PAIR.md.
// Names are hypotheses, not recovered symbols.

#include "bsp/unit_autopilot_pair.hpp"

namespace bsp {

ShipAiPublishResult ship_ai_publish_order_009f4d10(float heading_target_324,
                                                   float distance_32c,
                                                   float distance_330,
                                                   float seconds,
                                                   ShipAiPublishHost& host) {
    ShipAiPublishResult out{};
    out.blk_33c = -1.0f;                                // 009F4D10 loads 00D7A260
                                                        // (-1.0f), 009F4D27 stores it
    out.slot_index = host.order_slot_index_0b40();      // 009F4D2F
    UnitHeadingTargetState heading{};
    host.set_heading_target_00811960(heading, heading_target_324); // 009F4D48
    out.slot.heading_44 = heading.target_heading;
    out.slot.distance_40 = distance_32c;                // 009F4D55
    out.slot.valid_4c = true;                           // 009F4D58
    out.slot.distance_48 = distance_330;                // 009F4D62
    out.slot.valid_4c = true;                           // 009F4D6B, written twice
    host.tail_009f0100(seconds);                        // 009F4D71
    host.tail_009ef350();                               // 009F4D78
    host.tail_009ef910(seconds);                        // 009F4D87
    return out;
}

void unit_promote_ai_order_00825f2c(UnitAiOrderPromotion& state) {
    const int index = state.index;                      // 00825F2C
    state.promoted = state.slots[index].valid_4c;       // 00825F3F
    if (!state.promoted) {
        return;                                         // 00825F4C
    }
    state.slots[index].valid_4c = false;                // 00825F4E
    const int next = 1 - index;                         // 00825F51, 00825F56
    state.index = next;                                 // 00825F5E
    state.slots[next] = state.slots[index];             // 00825F77, 00811D10
}

UnitOrderedPairSource unit_ordered_pair_source_008266c1(bool unit_flag_0061) noexcept {
    return unit_flag_0061 ? UnitOrderedPairSource::AutopilotPair
                          : UnitOrderedPairSource::OrderRing;
}

bool ship_ai_runs_with_autopilot_pair() noexcept {
    // 009F50FC bails out of the whole controller update when the byte is set;
    // 008266C1 reads the autopilot pair only when it is set.
    return false;
}

}  // namespace bsp
