#pragma once
#include <cstdint>

// oct18-tA: image layout, new source interfaces. See docs/OCT18_TAIL_A.md.
// Names describe behavior; they are not recovered symbols or binary replacements.
namespace bsp::oct18_tail_a {
std::uint32_t air_ops_block_kind_006ca5b0() noexcept;
void air_ops_site_platform_tick_006cebe0(void* self, std::uint32_t argument) noexcept;
void air_ops_site_slot50_006cec10(void* self, std::uint32_t a, std::uint32_t b,
                                std::uint32_t c) noexcept;
std::uint8_t airfield_site_slot30_006cf3f0() noexcept;
void mothership_site_slot14_006cfe30(void* self, std::uint32_t argument) noexcept;
void* air_ops_block_member_006d0d20(void* self) noexcept;
void* airfield_weapon_director_006d0d30(const void* self) noexcept;
void unit_slot1a8_006d1640() noexcept;
std::uint32_t airfield_class_recon_category_006d1d20() noexcept;
std::uint8_t airfield_slotb4_006d1d40() noexcept;
void airfield_apply_sync_message_006d1d60(void* self, std::uint32_t argument) noexcept;
std::uint32_t air_ops_block_kind_006d1d70() noexcept;
std::uint32_t unit_slot190_006d1de0() noexcept;
std::uint32_t unit_slot1d4_006d1ee0() noexcept;
std::uint8_t unit_slot1e4_006d1f40(const volatile std::uint8_t& global_00e17bf2) noexcept;
void unit_slot1ec_006d1f90(void* self, std::uint32_t argument) noexcept;
const char* command_class_base_name_006f7f30() noexcept;
} // namespace bsp::oct18_tail_a
