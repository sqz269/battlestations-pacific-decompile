// The producer of the rudder curve settings block, and the gameplay-modifier product.
// docs/UNIT_RUDDER_CURVE.md carries the addresses, the ABI and the uncertainty.

#include "bsp/unit_rudder_curve.hpp"

namespace bsp {

UnitRudderCurveSettings unit_rudder_curve_settings(const UnitTurnMultiplierTable& table) noexcept {
    // The loader's mapping, store by store:
    //   0083CEDE  +444h = MinSpeed[1]      0083CF4D  +440h = MinSpeed[2]
    //   0083CFBC  +44Ch = MedSpeed[1]      0083D02B  +448h = MedSpeed[2]
    //   0083D09A  +43Ch = MaxSpeed[1]      0083D109  +438h = MaxSpeed[2]
    // 0082E890 then reads the first point as (+444h, +440h), the split as (+44Ch, +448h)
    // and the last as (+43Ch, +438h), so index 1 is the speed coordinate throughout.
    UnitRudderCurveSettings settings{};
    settings.speed_0444 = table.min_speed.throttle;
    settings.value_0440 = table.min_speed.turn_circle_multiplier;
    settings.speed_044c = table.med_speed.throttle;
    settings.value_0448 = table.med_speed.turn_circle_multiplier;
    settings.speed_043c = table.max_speed.throttle;
    settings.value_0438 = table.max_speed.turn_circle_multiplier;
    return settings;
}

UnitRudderCurveSettings unit_rudder_curve_load_0083ce56(UnitRudderCurveLoaderHost& host) {
    // 0083CE56: the previous temporary is released, the "TurnMultipliers" sub-table is
    // fetched from the current table and assigned back into the current-table slot.
    const int parent = host.current_table();
    host.release_temporary_00b67700(parent);
    const int sub_table = host.get_by_name_00b67800(parent, kTurnMultipliersTableKey);
    host.assign_current_table_00b67690(sub_table);

    // Then six identical blocks: release the previous temporary, look the key up on the
    // current table, index it, convert to float. The stores alternate index 1 then index 2
    // for each of the three keys.
    UnitTurnMultiplierTable table{};
    struct Row {
        const char* key;
        UnitTurnMultiplierPoint* point;
    };
    const Row rows[3] = {
        {kTurnMultiplierMinSpeedKey, &table.min_speed},
        {kTurnMultiplierMedSpeedKey, &table.med_speed},
        {kTurnMultiplierMaxSpeedKey, &table.max_speed},
    };
    for (const Row& row : rows) {
        for (int index = 1; index <= 2; ++index) {
            const int current = host.current_table();
            host.release_temporary_00b67700(current);
            const int array = host.get_by_name_00b67800(current, row.key);
            const int element = host.get_by_index_00b67720(array, index);
            const float value = host.get_number_00b66270(element);
            if (index == 1) {
                row.point->throttle = value;
            } else {
                row.point->turn_circle_multiplier = value;
            }
        }
    }
    return unit_rudder_curve_settings(table);
}

float gameplay_modifier_product_008e6430(const GameplayModifierEntry* entries, int count,
                                         GameplayModifierHost& host) {
    // 008E6435/008E6456: the accumulator starts at the 1.0f at 00D7A24C, so an empty or
    // fully filtered list returns exactly 1.0f.
    float product = 1.0f;
    for (int i = 0; i < count; ++i) {
        // 008E6470: the filter runs on node+8h with the unit; only a non-zero AL counts.
        if (host.entry_matches_008e4680(entries[i])) {
            // 008E647C: product = node[+1Ch] * product, in that operand order.
            product = entries[i].factor * product;
        }
    }
    return product;
}

}  // namespace bsp
