#pragma once

#include "bsp/session_message_cc13_seg42.hpp"

#include <cstdint>

namespace bsp {

// 007605F0, __thiscall(ECX = 79h message, flags), EAX = this, RET 4.
// Reset only the root profile; bit 0 releases the message's own allocation.
// The explicit C++ interface does not preserve the original register ABI.
seg42::VehicleGunControlMessage* delete_vehicle_gun_control_message_007605f0(
    seg42::VehicleGunControlMessage*, std::uint32_t flags);

} // namespace bsp
