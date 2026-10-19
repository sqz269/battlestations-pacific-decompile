#include "bsp/oct18b_nc_high.hpp"

namespace bsp::oct18b_nc_high {
static_assert(sizeof(void*) == 4, "Native pointer-array layout requires Win32");
bool pointer_array_remove_unordered_00bbd7f0(void* array, const std::uint32_t* key) {
    auto* bytes = static_cast<std::uint8_t*>(array);
    auto* data = *reinterpret_cast<std::uint32_t**>(bytes);
    auto& count = *reinterpret_cast<std::uint32_t*>(bytes + 4);
    const auto original_count = count;
    // The native empty path does not dereference key.
    if (!original_count) return false;
    const auto sought = *key;
    for (std::uint32_t i = 0; i != original_count; ++i) {
        if (data[i] != sought) continue;
        if (i != original_count - 1) data[i] = data[original_count - 1];
        --count;
        return true;
    }
    return false;
}
__declspec(naked) void owner_default_slot1_00bd1800() { __asm { ret } }
__declspec(naked) void owner_default_slot2_00bd1810() { __asm { ret } }
__declspec(naked) void owner_default_slot3_00bd1820() { __asm { ret } }
} // namespace bsp::oct18b_nc_high
