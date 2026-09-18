#include "bsp/ship_ai_rudder_hop.hpp"

#include <cmath>

namespace bsp {

float ship_hull_heading_00826c3b(float forward_x, float forward_z) noexcept {
    // 00826C3B FLD [EBP+20h] pushes the row-2 x first, 00826C3E FLD [EBX+28h]
    // the row-2 z second, so at the call ST1 is x and ST0 is z. 00BF701A is the
    // x87 entry of the CRT atan2, which takes ST1 as the numerator and ST0 as
    // the denominator, so the argument order is atan2(row2.x, row2.z).
    return std::atan2(forward_x, forward_z);
}

ShipMotionTailStep ship_motion_tail_00826c34(const ShipMotionTailInputs& in,
                                             ShipMotionTailHost& host) {
    ShipMotionTailStep out{};

    // 00826C3B..00826C56. The heading is recomputed from the pose the physics
    // step left, and stored into unit+1050h before anything reads it back.
    out.hull_heading = ship_hull_heading_00826c3b(in.forward_x, in.forward_z);

    // 00826C4E MOV ECX,[EDI+0D08h] is unit+1018h, the controller; 00826C54
    // PUSH 0 is the one argument. The store to unit+1050h at 00826C56 is
    // scheduled between the two but is ordered before the call by the FSTP.
    host.controller_0092e5b0(0);

    // 00826C61..00826C81. The ordered rudder through the rudder curve. 00811940
    // is the same two instructions packaged as a getter, which is how
    // 009EB428 and 00835E34 reach this value.
    out.yaw_rate = host.yaw_rate_from_rudder_00811890(in.ordered_rudder);

    // 00826C7A..00826CC5. The world matrix is refreshed before its translation
    // row is handed to 00810190 below.
    host.refresh_world_matrix();

    // 00826CCC..00826CDB. CALL [[ESI]+50h], no arguments, ST0 back. For a unit
    // that is 006DFD60, which returns the unit+1050h just written, so on an
    // unmodified vtable heading_read_back == hull_heading.
    out.heading_read_back = host.unit_heading_vtable50();

    // 00826CDD..00826CEE. The float at [ESP] that 00826CD8 stored before the
    // virtual call is NOT that call's argument: 00810190 is RET 0Ch and takes
    // three stack arguments, and the slot 00826CD5's PUSH ECX reserved is the
    // third of them. See docs/SHIP_AI_RUDDER_HOP.md.
    host.append_wake_sample_00810190(in.world_position, out.heading_read_back,
                                     out.yaw_rate);
    out.wake_sampled = true;

    out.occupant_timer_left = in.occupant_timer;

    // 00826CF3..00826CFC.
    if (host.occupant_owns_unit_00778890()) {
        out.occupant_timer_ran = true;
        // 00826CFE FLD [EDI+0E48h]; 00826D04 FSUB [ESP+0C4h]; 00826D13 FST
        // (not FSTP) writes unit+1158h back and keeps the value for the compare.
        float left = in.occupant_timer - in.dt;
        // 00826D19 FLDZ; 00826D1B FCOMIP st(1) compares 0 against the new value
        // and 00826D1F JBE skips when 0 <= left, so the block runs on left < 0.
        if (left < 0.0f) {
            left += host.occupant_timer_refill_00424c40_430();
            host.occupant_tick_0070db60();
            out.occupant_timer_expired = true;
        }
        out.occupant_timer_left = left;
    }

    // 00826D43..00826D5A. UCOMISS / LAHF / TEST AH,44h / JNP is MSVC's float
    // "!=": the call runs when the two differ, and an unordered compare (a NaN
    // on either side) sets both ZF and PF and also runs it.
    const bool equal = (in.field_308 == in.field_308_sentinel);
    if (!equal) {
        host.field_308_crossed_0077a650();
        out.field_308_check_ran = true;
    }

    return out;
}

} // namespace bsp
