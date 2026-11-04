#include "bsp/gameplay_seg61_62_cc13.hpp"
#include "bsp/native_return_one_byte.hpp"
#include "bsp/wire_goal4_chain399_switches.hpp"

namespace bsp::gameplay_seg61_62_cc13 {

// Existing 00951F20 body moved from gameplay_seg61_62_cc13.cpp into bsp_core:
// the live ship calculation and the existing ship-motion/math consumers all
// need this completion helper. New C++ interface, not a native ABI replacement.
bool vehicle_class_derive_constants_default_00951f20() noexcept {
    if constexpr (kWireGoal4Chain399Bound) {
        // bsp.py show 00951F20 --asm: CALL 00876180; TEST AL,AL; SETNZ AL.
        // Only the reconstructed callee's byte result is part of this contract.
        return return_native_one_byte_00876180() != 0;
    } else {
    const std::uint8_t al = 1;  // 00876180: MOV AL, 1 ; RET
    return al != 0;
    }
}

} // namespace bsp::gameplay_seg61_62_cc13
