// 009E2020, the kamikaze_attack state's step. See the header for the evidence.
#include "bsp/ship_ai_kamikaze_attack.hpp"

namespace bsp {

bool ship_ai_kamikaze_attack_step_009e2020(ShipAiAttackMoveEngageState& state,
                                           ShipAiKamikazeAttackHost& host) {
    if (host.brain_target_0b20() == 0) { // 009E2032, JZ 009E2329
        // 009E2334: the heading is fetched before the mode test; 009E233A..
        // 009E237A are 009DFF40's body with that heading (mode 1, the two
        // timers, 009DA4E0, blk+1D8h, 00605070, blk+1CCh = 0).
        const float heading = host.unit_heading_vtable_0050();
        host.set_heading_and_drop_path_009dff40(heading);
        host.set_brain_speed_scale_0af0(1.0f); // 009E2394, 00D7A24C
        return state.attack_run_08;
    }
    // 009E203A..009E2326: 009E23B0's target arm; its null test is not reached.
    return ship_ai_attackmove_engage_step_009e23b0(state, host);
}

} // namespace bsp
