#include "bsp/native_gun_control_message_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

static_assert(sizeof(seg42::VehicleGunControlMessage) == 0x3c);

// Evidence: bsp.py ghidra disasm 007605F0, bsp.gpr /battlestationspacific.exe:
// TEST byte [ESP+4],1; save ECX; store CE4974 at [this]; conditional BF65AC;
// return this; RET 4. The payload has no owned subobjects. Keep Ghidra's
// correct CG_scalar_deleting_dtor name; the C++ name is descriptive.
__declspec(noinline) seg42::VehicleGunControlMessage*
delete_vehicle_gun_control_message_007605f0(seg42::VehicleGunControlMessage* message,
    std::uint32_t flags) {
    static_cast<volatile NativeSessionMessageStorage&>(message->h.base).profile_00 =
        native_session_message_root_profile_00ce4974();
    if ((flags & 1u) != 0) singleton_lifetime_free(message);
    return message;
}

} // namespace bsp
