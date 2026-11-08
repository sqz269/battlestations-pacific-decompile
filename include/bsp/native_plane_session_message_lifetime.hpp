#pragma once

#include "bsp/session_message_cc13_seg42.hpp"

namespace bsp {

// Complete scalar-deleting destructors for the C3h, C5h and C7h messages.
// Original ABI: ECX=this, stack flags byte, RET4, EAX=this (including after
// free). Stamp only +0 with the root profile; free storage iff flags bit 0.
// New typed C++ interfaces, not original-ABI entry points. Bit 0 requires an
// allocation from singleton_lifetime_allocate; flags=0 admits stack senders.
// Type names describe the recovered MT_* message roles, not original symbols.
seg42::PlaneChangeStateMessage* delete_plane_change_state_message_00762920(
    seg42::PlaneChangeStateMessage*, std::uint32_t flags);
seg42::UnitMessage* delete_plane_touchdown_message_00762b00(
    seg42::UnitMessage*, std::uint32_t flags);
seg42::PlaneDisappearMessage* delete_plane_disappear_message_00762bc0(
    seg42::PlaneDisappearMessage*, std::uint32_t flags);

// The same recovered ownership contract for ship-death messages 98h / 9Ah.
seg42::UnitMessage* delete_ship_wrecked_message_007625c0(
    seg42::UnitMessage*, std::uint32_t flags);
seg42::UnitMessage* delete_ship_explode_to_parts_message_007625e0(
    seg42::UnitMessage*, std::uint32_t flags);

// Local formation and vehicle messages use the same root stamp/free contract.
// Their factory allocations are 24h for formation and shipyard launch, 20h
// for SetParty. No payload member owns a reference or a separate allocation.
seg42::FormationMemberMessage* delete_formation_join_message_0075ca20(
    seg42::FormationMemberMessage*, std::uint32_t flags);
seg42::FormationMemberMessage* delete_formation_leave_message_0075ca40(
    seg42::FormationMemberMessage*, std::uint32_t flags);
seg42::VehicleValueMessage* delete_vehicle_set_party_message_00760610(
    seg42::VehicleValueMessage*, std::uint32_t flags);
seg42::VehicleShipyardLaunchMessage* delete_vehicle_shipyard_launch_message_00760650(
    seg42::VehicleShipyardLaunchMessage*, std::uint32_t flags);

// Carrier elevator message 87h: the factory allocates 24h, including the
// conditional plane id and its two presence/control bytes after UnitMessage.
struct ElevatorMoveSessionMessage {
    seg42::UnitMessage unit;
    std::uint16_t plane_id_20;
    std::uint8_t flag_22;
    std::uint8_t has_plane_23;
};
static_assert(sizeof(ElevatorMoveSessionMessage) == 0x24);
static_assert(offsetof(ElevatorMoveSessionMessage, plane_id_20) == 0x20);
static_assert(offsetof(ElevatorMoveSessionMessage, flag_22) == 0x22);
static_assert(offsetof(ElevatorMoveSessionMessage, has_plane_23) == 0x23);
ElevatorMoveSessionMessage* delete_elevator_move_message_00758cd0(
    ElevatorMoveSessionMessage*, std::uint32_t flags);

// The 7Bh / 7Ch factory records are each 24h bytes. Their decoded payloads
// remain live until projected to the launch receiver; neither owns a subobject.
struct NativeVehicleUnitLaunchMessage {
    seg42::UnitMessage u;
    float heading_20;
};
struct NativeVehicleAddLaunchedMessage {
    seg42::UnitMessage u;
    std::uint16_t plane_id_20;
    std::uint16_t pad_22;
};
NativeVehicleUnitLaunchMessage* delete_vehicle_unit_launch_message_00464fe0(
    NativeVehicleUnitLaunchMessage*, std::uint32_t flags);
NativeVehicleAddLaunchedMessage* delete_vehicle_add_launched_message_006ec100(
    NativeVehicleAddLaunchedMessage*, std::uint32_t flags);

} // namespace bsp
