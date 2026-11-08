#include "bsp/native_plane_session_message_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
static_assert(sizeof(seg42::PlaneChangeStateMessage) == 0x24);
static_assert(sizeof(seg42::UnitMessage) == 0x20);
static_assert(sizeof(seg42::PlaneDisappearMessage) == 0x24);

// Evidence: bsp.py show 00762920 / 00762B00 / 00762BC0 --asm, existing
// bsp.gpr /battlestationspacific.exe. All three bodies TEST [ESP+4],1,
// save ECX in ESI, store 00CE4974 at [ESI], conditionally call 00BF65AC,
// return ESI in EAX and RET4. No payload or other header byte is touched.
// Keep the existing correct Ghidra CG_scalar_deleting_dtor_* names; these
// C++ names identify their message profiles, not newly recovered symbols.
// The root profile is the existing callable translation of 00CE4974.
__declspec(noinline) seg42::PlaneChangeStateMessage*
delete_plane_change_state_message_00762920(seg42::PlaneChangeStateMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::UnitMessage*
delete_plane_touchdown_message_00762b00(seg42::UnitMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::PlaneDisappearMessage*
delete_plane_disappear_message_00762bc0(seg42::PlaneDisappearMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

// Chain 514 evidence: bsp.py show 007625C0 / 007625E0 --asm in existing
// bsp.gpr /battlestationspacific.exe. Both TEST byte [ESP+4],1; save ECX in
// ESI; store 00CE4974 at [ESI]; optionally call 00BF65AC; return ESI; RET4.
// Profiles 00D033C4 (98h) and 00D033D8 (9Ah) select these scalar deletes.
// Retain their correct Ghidra CG_scalar_deleting_dtor_* names. The callable
// root-profile translation and allocator are the same as the plane messages.
__declspec(noinline) seg42::UnitMessage*
delete_ship_wrecked_message_007625c0(seg42::UnitMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::UnitMessage*
delete_ship_explode_to_parts_message_007625e0(seg42::UnitMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

// Chain 626: bsp.py show 0075CA20 / 0075CA40 / 00760610 / 00760650 --asm,
// existing bsp.gpr /battlestationspacific.exe. Each TESTs the low flags byte's
// bit 0, stores 00CE4974 at [this], optionally calls 00BF65AC, and returns
// this in EAX with RET4. Retain their correct CG_scalar_deleting_dtor names
// in Ghidra. These typed interfaces use the existing callable root profile
// and the same malloc/free pair as the factory's OwnedDecodedMessage.
static_assert(sizeof(seg42::FormationMemberMessage) == 0x24);
static_assert(sizeof(seg42::VehicleValueMessage) == 0x20);
static_assert(sizeof(seg42::VehicleShipyardLaunchMessage) == 0x24);

__declspec(noinline) seg42::FormationMemberMessage*
delete_formation_join_message_0075ca20(seg42::FormationMemberMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::FormationMemberMessage*
delete_formation_leave_message_0075ca40(seg42::FormationMemberMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::VehicleValueMessage*
delete_vehicle_set_party_message_00760610(seg42::VehicleValueMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

// Goal 5 chain 775 evidence: bsp.py show 00760630 --asm in the existing
// bsp.gpr /battlestationspacific.exe. 00760630..0076064C tests flags bit 0,
// stamps 00CE4974, optionally calls 00BF65AC, and returns this with RET4.
// Preserve Ghidra's correct CG_scalar_deleting_dtor_00760630 library name.
__declspec(noinline) seg42::VehicleValueMessage*
delete_vehicle_set_race_message_00760630(seg42::VehicleValueMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) seg42::VehicleShipyardLaunchMessage*
delete_vehicle_shipyard_launch_message_00760650(seg42::VehicleShipyardLaunchMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

// Chain 686 evidence: bsp.py show 00758cd0 --asm, existing bsp.gpr,
// /battlestationspacific.exe. 00758CD0..00758CEC tests flags bit 0,
// saves ECX in ESI, stores 00CE4974 at [ESI], optionally calls 00BF65AC,
// and returns ESI in EAX with RET4. No payload member owns storage.
// Retain Ghidra's correct CG_scalar_deleting_dtor_00758cd0 name.
__declspec(noinline) ElevatorMoveSessionMessage*
delete_elevator_move_message_00758cd0(ElevatorMoveSessionMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->unit.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}
// Chain 690 evidence: bsp.py show 00464FE0 --asm and ghidra disasm 006EC100,
// existing bsp.gpr /battlestationspacific.exe. Both TEST byte [ESP+4],1;
// save this; stamp 00CE4974; conditionally call 00BF65AC; return this; RET4.
// Only +0 is written. The native 7Bh / 7Ch profiles (00CE53AC / 00CFAA0C)
// select these scalar deletes. These are new C++ interfaces, not ABI entries.
static_assert(sizeof(NativeVehicleUnitLaunchMessage) == 0x24);
static_assert(offsetof(NativeVehicleUnitLaunchMessage, heading_20) == 0x20);
static_assert(sizeof(NativeVehicleAddLaunchedMessage) == 0x24);
static_assert(offsetof(NativeVehicleAddLaunchedMessage, plane_id_20) == 0x20);

__declspec(noinline) NativeVehicleUnitLaunchMessage*
delete_vehicle_unit_launch_message_00464fe0(NativeVehicleUnitLaunchMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

__declspec(noinline) NativeVehicleAddLaunchedMessage*
delete_vehicle_add_launched_message_006ec100(NativeVehicleAddLaunchedMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->u.h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

} // namespace bsp
