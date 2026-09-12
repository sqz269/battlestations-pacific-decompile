#include "bsp/gameplay_loose_ends_2.hpp"

// Packet cc2_loose_ends_2. Evidence in docs/GAMEPLAY_LOOSE_ENDS_2.md.

namespace bsp {

PlaneControllerModeGates plane_controller_mode_gates(int mode) noexcept {
    PlaneControllerModeGates gates;
    switch (mode) {
        case 0:  // free flight, written at 007DC841
            gates.roll_rate_survives = true;
            gates.stores_ground_plane = false;
            gates.applies_wheel_height_lift = false;
            gates.core_law_block = 0x007DBE1Cu;   // 007DBE16 JNE not taken
            gates.factor_arm = 0x007DA6E6u;       // 007DA396 JE
            gates.enables_bank_yaw_coupling = true;  // 007DA700 MOV byte [EAX],1
            return gates;
        case 1:  // ground roll, written at 007DCD24
            gates.roll_rate_survives = false;     // 007DA8E5 MOVSS [ESP+18h],XMM2
            gates.stores_ground_plane = true;     // 007DB702-007DB743
            gates.applies_wheel_height_lift = true;  // 007DA2B7-007DA338
            gates.core_law_block = 0x007DBEB3u;   // 007DBEAD JNE not taken
            gates.factor_arm = 0x007DA542u;       // 007DA3A0 JE
            gates.enables_bank_yaw_coupling = false;  // 007DA6DE MOV byte [EAX],0
            return gates;
        case 2:  // water surface, written at 007DCDDC
            gates.roll_rate_survives = true;
            gates.stores_ground_plane = false;
            gates.applies_wheel_height_lift = false;
            gates.core_law_block = 0x007DC205u;   // 007DBEAD JNE taken
            gates.factor_arm = 0x007DA3D0u;       // 007DA3A9 JE
            gates.enables_bank_yaw_coupling = false;  // 007DA53A MOV byte [ECX],0
            return gates;
        default:
            // No writer stores a value above 2, so this row is unreachable in the shipped
            // image. The three readers that can see it still have an explicit arm: the core
            // law's `>= 2` branch and FUN_007DA380's tail, which publishes
            // kLooseEnds2ControllerFactorFallback in both float out-parameters.
            gates.roll_rate_survives = true;
            gates.stores_ground_plane = false;
            gates.applies_wheel_height_lift = false;
            gates.core_law_block = 0x007DC205u;
            gates.factor_arm = 0x007DA3ABu;
            gates.enables_bank_yaw_coupling = false;  // 007DA3BF MOV byte [EAX],0
            return gates;
    }
}

PlaneControllerModeGates plane_controller_mode_gates(PlaneControllerMode mode) noexcept {
    return plane_controller_mode_gates(static_cast<int>(mode));
}

bool any_weapon_category_accepts_target_008637d0(WeaponCategoryAvailabilityHost& host) {
    // 008637D5 tests the first entry before the loop and 0086381A tests the next one at the
    // bottom, so an entry at or above the terminator ends the walk without being examined.
    for (int index = 0;; ++index) {
        const int category = host.category_at(index);
        if (category >= kLooseEnds2WeaponCategoryTerminator) {
            return false;  // 0086382E XOR AL,AL
        }
        if (host.unit_has_category(category)            // 008637F2
            && host.gunnery_ai_accepts_category(category)  // 008637F9-00863802
            && host.category_accepts_target(category)) {   // 0086380E
            return true;  // 00863837 MOV AL,1
        }
    }
}

}  // namespace bsp
