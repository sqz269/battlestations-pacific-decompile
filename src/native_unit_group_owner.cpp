#include "bsp/native_unit_group_owner.hpp"
#include <cstdint>
#include <cstring>

namespace bsp {
static_assert(sizeof(void*) == 4, "Native unit/group pointer fields require Win32.");
bool native_unit_is_group_owner_00778890(const void* actual_unit) noexcept {
    const auto* const unit_bytes = static_cast<const std::uint8_t*>(actual_unit);
    const void* captured_group;
    std::memcpy(&captured_group, unit_bytes + 0x284, sizeof(captured_group));
    if (!captured_group) return false;
    const void* captured_owner;
    const auto* const group_bytes = static_cast<const std::uint8_t*>(captured_group);
    std::memcpy(&captured_owner, group_bytes + 0x14, sizeof(captured_owner));
    return captured_owner == actual_unit;
}
} // namespace bsp
