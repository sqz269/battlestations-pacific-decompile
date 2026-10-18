#include "bsp/oct18_tail_a.hpp"
#include <cstring>

namespace bsp::oct18_tail_a {
// Original: MOV EAX,4; RET. No register/stack input is read.
std::uint32_t air_ops_block_kind_006ca5b0() noexcept { return 4; }
// Original: RET 4; ECX and one stack word are unobserved.
void air_ops_site_platform_tick_006cebe0(void*, std::uint32_t) noexcept {}
// Original: RET 0Ch; ECX and three stack words are unobserved.
void air_ops_site_slot50_006cec10(void*, std::uint32_t, std::uint32_t,
                                std::uint32_t) noexcept {}
// Original: MOV AL,1; RET. Upper EAX is unspecified.
std::uint8_t airfield_site_slot30_006cf3f0() noexcept { return 1; }
// Original: RET 4, unobserved ECX and stack word.
void mothership_site_slot14_006cfe30(void*, std::uint32_t) noexcept {}
// Original: LEA EAX,[ECX+15Ch]; RET. Interior pointer, not a load.
void* air_ops_block_member_006d0d20(void* self) noexcept {
    return static_cast<std::uint8_t*>(self) + 0x15c;
}
// Original: MOV EAX,[ECX+8C8h]; RET. Pointer-valued field, original Win32 layout.
void* airfield_weapon_director_006d0d30(const void* self) noexcept {
    void* result;
    std::memcpy(&result, static_cast<const std::uint8_t*>(self) + 0x8c8, sizeof(result));
    return result;
}
// Original: RET; no arguments are observed. Virtual slot's semantic type remains unknown.
void unit_slot1a8_006d1640() noexcept {}
// Original: MOV EAX,10h; RET.
std::uint32_t airfield_class_recon_category_006d1d20() noexcept { return 0x10; }
// Original: MOV AL,1; RET; upper EAX unspecified.
std::uint8_t airfield_slotb4_006d1d40() noexcept { return 1; }
// Original: RET 4; the incoming sync message is ignored.
void airfield_apply_sync_message_006d1d60(void*, std::uint32_t) noexcept {}
// Original: MOV EAX,14h; RET.
std::uint32_t air_ops_block_kind_006d1d70() noexcept { return 0x14; }
// Original: MOV EAX,6; RET.
std::uint32_t unit_slot190_006d1de0() noexcept { return 6; }
// Original: XOR EAX,EAX; RET.
std::uint32_t unit_slot1d4_006d1ee0() noexcept { return 0; }
// Original: MOV AL,[00E17BF2]; RET. Explicit borrowed global, not a cached value.
std::uint8_t unit_slot1e4_006d1f40(const volatile std::uint8_t& flag) noexcept { return flag; }
// Original: RET 4; ECX and one stack word unobserved.
void unit_slot1ec_006d1f90(void*, std::uint32_t) noexcept {}
// Original: MOV EAX,00CFB38C; RET, returning this exact text in image storage.
const char* command_class_base_name_006f7f30() noexcept { return "undefined command name"; }
} // namespace bsp::oct18_tail_a
