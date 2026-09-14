#include "bsp/native_structured_node_predicate.hpp"

#include <cstdint>

namespace bsp {
bool native_structured_node_has_remaining_00715bf0(const void* wrapper) noexcept {
    static_assert(sizeof(void*) == 4);
    const auto node = *static_cast<const volatile std::uint32_t*>(wrapper);
    return node != 0u &&
        *reinterpret_cast<const volatile std::uint32_t*>(node + 0x20u) != 0u;
}
} // namespace bsp
