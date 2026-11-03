#pragma once

// Goal 2, chain 179 (slot g2-w2): segment 54 [008742A0-00878350] range sweep.
// OFF keeps every host stand-in byte for byte. ON routes:
//   00875A80 fixed_step_callback_register_00875a80 from the paratrooper round's
//            RoundAdapter::register_fixed_step_once (007AC095, src/cc13_paratrooper_round.cpp):
//            the round's node goes on a fixed-step callback list that 00874DE0 walks at the
//            head of the round step, in place of the host's callback_registered bool.
// Runtime-unvalidated; flip in a batch across missions (JM08 for the paratrooper round).

namespace bsp {

inline constexpr bool kWireGoal2Chain179Bound = false;

}  // namespace bsp
