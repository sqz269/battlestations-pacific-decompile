// The external channel into the ship hull body. docs/UNIT_FORCE_CHANNEL.md.
//
// Routines read for this file:
//   0092BF30, 0092BF30..0092BF37 (two instructions, read in full)
//   00821E80, the switch head and cases 90h..95h; case 93h at 00822235..0082226C
//   0080FFD0, 0080FFD0..0081000D (the message's producer, read in full)
//   00762220, 00762220..00762275 (the stream decoder's constructor for the same class)
//   the complete xref sets of 00C35360, 00C35330, 00C37E50, 00C37E20, 00C32050, 00C37E70
//
// Nothing here is a binary-compatible replacement.

#include "bsp/unit_force_channel.hpp"

namespace bsp {

void unit_controller_add_torque_0092bf30(DynBody& body, const OceanVec3& torque) noexcept {
    // 0092BF30 MOV ECX,[ECX+2Ch] loads the hull body off the controller; 0092BF33 is a
    // tail JMP, so the helper adds nothing of its own and the callee's RET 4 cleans up.
    dyn_body_add_torque_00c35330(body, torque);
}

bool unit_handle_add_hull_torque_00822235(const UnitHullTorqueMessage& message,
                                          DynBody& controller_body) noexcept {
    // 00822235..00822251 copies msg+1Ch/+20h/+24h into a stack vec3 and pushes its
    // address; 0082224A loads the controller from unit+1018h into ECX. 0082225B returns
    // AL = 1 and 0082226C is RET 4.
    unit_controller_add_torque_0092bf30(controller_body, message.torque);
    return true;
}

}  // namespace bsp
