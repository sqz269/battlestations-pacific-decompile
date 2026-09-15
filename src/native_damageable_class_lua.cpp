#include "bsp/native_damageable_class_lua.hpp"
#include "bsp/entity_identity.hpp"

namespace bsp {
std::int32_t native_mesh_category_from_name_007149d0(
    const char* name, const char* const volatile* actual_categories_00e08138) {
    std::uint32_t index = 0;
    // 7149D3 tests the first cell, and7149E3 reloads it for the comparison.
    if (actual_categories_00e08138[0] == nullptr) return -1;
    for (;;) {
        if (compare_insensitive_00438e10(
                name, actual_categories_00e08138[index]) == 0) {
            return static_cast<std::int32_t>(index);
        }
        ++index; // Original32-bit register increment, without a fixed15 limit.
        // 7149F3 tests the next cell; the next comparison reloads that cell.
        if (actual_categories_00e08138[index] == nullptr) return -1;
    }
}
} // namespace bsp
