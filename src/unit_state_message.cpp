#include "bsp/unit_state_message.hpp"

namespace bsp {
namespace {

// The wrap the native code performs with CMP EAX,0Ah / XOR EAX,EAX, i.e. the
// index is incremented and reset to zero once it reaches ten. It is never a
// modulus of an arbitrary value.
int advance_ring_index(int index) noexcept
{
    const int next = index + 1;
    return (next < static_cast<int>(kUnitOrderRingSlotCount)) ? next : 0;
}

bool ring_index_in_range(int index) noexcept
{
    return index >= 0 && index < static_cast<int>(kUnitOrderRingSlotCount);
}

} // namespace

void construct_unit_order_ring_00812d40(UnitOrderRing& ring) noexcept
{
    // 00812D42..00812F48: ten unrolled copies of the same six stores. The two
    // parameters are zeroed, the predicted byte is set, both upper bounds take
    // 00CE3958 and both lower bounds 00CE7D7C, and the kind byte is cleared.
    for (std::size_t i = 0; i < kUnitOrderRingSlotCount; ++i) {
        ring.slot[i].param_a = 0.0f;
        ring.slot[i].param_b = 0.0f;
        ring.slot[i].predicted = true;                      // 00812D62, MOV byte, DL = 1
        ring.slot[i].param_a_high = kUnitOrderRingBoundHigh; // 00812D6A
        ring.slot[i].param_a_low = kUnitOrderRingBoundLow;   // 00812D74
        ring.slot[i].param_b_high = kUnitOrderRingBoundHigh; // 00812D65
        ring.slot[i].param_b_low = kUnitOrderRingBoundLow;   // 00812D6F
        ring.slot[i].kind = 0;                               // 00812D79
    }

    ring.read_cursor = 0;                                // 00812F5A, ECX is still zero
    ring.current_param_b = 0.0f;                         // 00812F66
    ring.current_param_a = 0.0f;                         // 00812F6E
    ring.write_cursor = kUnitOrderRingSyncLagTicks;      // 00812F7E, from DAT_00E0B51C
    ring.slew_a = kUnitOrderRingSlewADefault;            // 00812F84
    ring.slew_b = kUnitOrderRingSlewBDefault;            // 00812F8C
}

void backfill_unit_order_ring_00812fa0(UnitOrderRing& ring, float param_a, float param_b,
                                       std::uint8_t kind, int age_ticks,
                                       int sync_lag_ticks) noexcept
{
    // 00812FAA/00812FB4: CMOVL caps the age at DAT_00E0B51C.
    int age = age_ticks;
    if (sync_lag_ticks < age) {
        age = sync_lag_ticks;
    }

    const int write_cursor = ring.write_cursor; // 00812FAE
    if (!ring_index_in_range(write_cursor)) {
        return; // the native code has no bound check; this projection needs one
    }

    // 00812FB9..00812FBD: a negative start wraps by adding ten, once.
    int index = write_cursor - age;
    if (index < 0) {
        index += static_cast<int>(kUnitOrderRingSlotCount);
    }
    if (!ring_index_in_range(index)) {
        return; // only reachable for an age outside [0, 10); native would fault
    }

    // 00812FD4..00812FFB, then the same four stores once more for the cursor
    // slot itself at 00813002..0081300E.
    for (;;) {
        UnitOrderRingSlot& slot = ring.slot[static_cast<std::size_t>(index)];
        slot.param_a = param_a;   // 00812FE1
        slot.param_b = param_b;   // 00812FE5
        slot.kind = kind;         // 00812FEA
        slot.predicted = false;   // 00812FED, stores a zero byte
        if (index == write_cursor) {
            break;
        }
        index = advance_ring_index(index);
    }
}

void set_unit_order_ring_param_a_0080d9b0(UnitOrderRing& ring, float value) noexcept
{
    // 0080D9B0..0080D9E8. The loop runs from the read cursor to the write
    // cursor, then the live field is written unconditionally.
    int index = ring.read_cursor;
    if (ring_index_in_range(index) && ring_index_in_range(ring.write_cursor)) {
        while (index != ring.write_cursor) {
            ring.slot[static_cast<std::size_t>(index)].param_a = value; // 0080D9CF
            index = advance_ring_index(index);
        }
        ring.slot[static_cast<std::size_t>(index)].param_a = value; // 0080D9E3
    }
    ring.current_param_a = value; // 0080D9E8, the store to unit+980h
}

void set_unit_order_ring_param_b_0080da00(UnitOrderRing& ring, float value) noexcept
{
    // 0080DA00..0080DA3A, the same shape on slot+04h and the live rudder.
    int index = ring.read_cursor;
    if (ring_index_in_range(index) && ring_index_in_range(ring.write_cursor)) {
        while (index != ring.write_cursor) {
            ring.slot[static_cast<std::size_t>(index)].param_b = value; // 0080DA1F
            index = advance_ring_index(index);
        }
        ring.slot[static_cast<std::size_t>(index)].param_b = value; // 0080DA34
    }
    ring.current_param_b = value; // 0080DA3A, the store to unit+984h
}

float clamp_unit_order_ring_slot_00813020(float value, float low, float high) noexcept
{
    // 00813040..00813067 for param_a and 00813084..008130C2 for param_b. The
    // native shape tests the low bound first with FCOMIP/JBE, so an unordered
    // comparison takes the arm that yields the low bound.
    if (low <= value) {
        if (high < value) {
            return high;
        }
        return value;
    }
    return low;
}

void tick_unit_order_ring_00813020(UnitOrderRing& ring, float dt, int session_mode) noexcept
{
    const int read_cursor = ring.read_cursor;
    const int write_cursor = ring.write_cursor;
    if (!ring_index_in_range(read_cursor) || !ring_index_in_range(write_cursor)) {
        return; // the native code has no bound check; this projection needs one
    }
    const std::size_t read_index = static_cast<std::size_t>(read_cursor);
    const UnitOrderRingSlot& current = ring.slot[read_index];

    // 0081302D..008130C2: both targets are clamped against the slot's own bounds.
    const float target_a =
        clamp_unit_order_ring_slot_00813020(current.param_a, current.param_a_low,
                                            current.param_a_high);
    const float target_b =
        clamp_unit_order_ring_slot_00813020(current.param_b, current.param_b_low,
                                            current.param_b_high);

    // 008130C2..0081311F. ECX is LEA ring+148h then LEA ring+14Ch, so these two
    // calls are the writes to unit+980h and unit+984h.
    ring.current_param_a =
        unit_step_towards_0042ac60(ring.current_param_a, target_a, ring.slew_a * dt);
    ring.current_param_b =
        unit_step_towards_0042ac60(ring.current_param_b, target_b, ring.slew_b * dt);

    // 00813124..00813152. The kind always follows; the confirmed triple only
    // advances on an authoritative slot, and it is copied from the live pair,
    // not from the slot.
    const std::uint8_t kind = current.kind;
    ring.current_kind = kind; // 00813134
    if (!current.predicted) { // 0081313A, CMP byte [EAX+8],0
        ring.confirmed_param_a = ring.current_param_a; // 00813144
        ring.confirmed_kind = kind;                    // 0081314A
        ring.confirmed_param_b = ring.current_param_b; // 00813152
    }

    // 00813158..00813186: the write slot is copied forward as eight dwords and
    // the copy is marked predicted.
    const int next_write = advance_ring_index(write_cursor);
    ring.slot[static_cast<std::size_t>(next_write)] = ring.slot[static_cast<std::size_t>(write_cursor)];
    ring.slot[static_cast<std::size_t>(next_write)].predicted = true; // 00813186

    // 0081318C..008131C9. A networked client keeps the lag; everyone else
    // consumes what was just written.
    if (session_mode == kUnitOrderRingClientSessionMode) {
        ring.read_cursor = advance_ring_index(read_cursor);
    } else {
        ring.read_cursor = write_cursor;
    }
    ring.write_cursor = next_write;
}

UnitNetworkMotionState dead_reckon_ship_sync_00816c80(const UnitStateMessage& message,
                                                      int age_ticks,
                                                      float wrapped_heading) noexcept
{
    // 00816CEF..00816D05: the age is converted with FILD and scaled by 00D0DE84.
    const float seconds = static_cast<float>(age_ticks) * kUnitStateMessageTickSeconds;

    UnitNetworkMotionState state;
    state.velocity_x = message.velocity_x; // 00816D2A
    state.velocity_z = message.velocity_z; // 00816D5C
    // 00816D6A/00816D72 and 00816D76/00816D7E: the product is computed first and
    // stored to a stack float, then reloaded and added to the transmitted
    // position, so both operands reach the addition as floats.
    state.position_x = message.velocity_x * seconds + message.position_x;
    state.position_z = message.velocity_z * seconds + message.position_z;
    state.heading = wrapped_heading; // 00816DA1, the 00438AA0 result
    state.yaw_rate = message.yaw_rate; // 00816DB0
    state.reserved_18 = 0.0f;          // 00816CF9
    state.reserved_1c = 0.0f;          // 00816CF3
    state.valid = 1;                   // 00816DB6
    return state;
}

void apply_ship_sync_message_00816c80(ShipSyncApplyHost& host, UnitOrderRing& ring,
                                      const UnitStateMessage& message, int current_tick)
{
    // 00816C90..00816CAA: the gun sync shares this virtual and returns early.
    if (host.message_is_category_0075a660(kSessionMessageShipGunsSync)) {
        host.apply_gun_sync_00813950();
        return;
    }

    // 00816CB8..00816CEB: the signed difference is formed first and negated
    // after the gate, so the value handed on is the age in ticks.
    const int age_ticks = current_tick - message.send_tick;

    // 00816CAD..00816CC7: the collision group is mirrored before the gate.
    if (message.collision_group != host.controller_collision_group()) {
        host.set_controller_collision_group_0092bd70(message.collision_group);
    }

    // 00816CCC: COMISS against 1.0f with JA, so an unordered compare falls
    // through and a NaN gate value applies the message.
    if (host.apply_gate_value() > kUnitStateMessageApplyGate) {
        return;
    }

    const float seconds = static_cast<float>(age_ticks) * kUnitStateMessageTickSeconds;
    const float heading =
        host.add_wrapped_angle_00438aa0(message.heading, message.yaw_rate * seconds);
    host.apply_network_motion_state_0092f2e0(
        dead_reckon_ship_sync_00816c80(message, age_ticks, heading));

    // 00816DC0..00816DE9: the order pair is back-filled only for a unit the
    // local player is not occupying in role 1.
    if (!host.is_local_player_role_00927f30(kUnitStateMessageLocalRole)) {
        backfill_unit_order_ring_00812fa0(ring, message.ordered_throttle, message.ordered_rudder,
                                          message.class_byte, age_ticks);
    }

    if (message.flooded != 0) { // 00816DEE
        host.clear_unit_10d0(); // 00816DF7
    }

    if (host.unit_class_query() == 2) {          // 00816E0D/00816E0F
        host.set_unit_class_byte(message.class_byte); // 00816E17
    }
}

} // namespace bsp
