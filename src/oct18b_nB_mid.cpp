#include "bsp/oct18b_nB_mid.hpp"

namespace bsp::oct18b_nB_mid {
void read_mothership_create_007593b0(void* m, void* s, MotherShipCreateReadHooks& h) {
    h.ship_create_read_00813bd0(m, s);
    h.air_ops_stock_read_006d4eb0(static_cast<std::uint8_t*>(m) + 0x12c, s);
}
void projectile_set_air_mode_thunk_007b9070(cc13_seg33a::Seg33aHost& h, void* shot, void* unused) {
    cc13_seg33a::projectile_shot_set_air_mode_006e6410(h, shot, unused);
}
void decrement_session_byte12b_0076a710(void* owner) noexcept {
    auto& value = static_cast<std::uint8_t*>(owner)[0x12b];
    value = static_cast<std::uint8_t>(value - 1u);
}
void* descriptor_identity_00780800(void* receiver, std::uint32_t) noexcept { return receiver; }
void* descriptor_identity_0078eaa0(void* receiver) noexcept { return receiver; }
bool session_false_slot_00781080(std::uint32_t) noexcept { return false; }
}
