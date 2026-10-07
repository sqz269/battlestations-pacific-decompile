#include "bsp/native_unit_role_permission.hpp"
#include <cstring>

namespace bsp {
std::uint32_t native_unit_role_open_to_slot_0059bbd0(const void* actual_unit,
    std::uint32_t role, std::uint32_t candidate_slot) noexcept {
    std::uint32_t policy;
    const auto* const bytes = static_cast<const std::uint8_t*>(actual_unit);
    std::memcpy(&policy, bytes + 0x188u + role * 4u, sizeof(policy));
    return policy == 9u || policy == candidate_slot ? 1u : 0u;
}
} // namespace bsp
