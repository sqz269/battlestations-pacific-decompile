#include "bsp/native_gui_state_vector_access.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual GUI state-vector access requires MSVC Win32.
#endif

namespace bsp {
void* native_gui_state_vector_record_00acec40(
    const void* actual_vector, std::uint32_t index) {
    static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
    const auto* const bytes = static_cast<const std::byte*>(actual_vector);
    const std::uint32_t begin =
        *reinterpret_cast<const volatile std::uint32_t*>(bytes + 4);
    bool in_range = false;
    if (begin != 0) {
        const std::uint32_t end =
            *reinterpret_cast<const volatile std::uint32_t*>(bytes + 8);
        const std::uint32_t difference = end - begin;
        std::int32_t signed_difference;
        std::memcpy(&signed_difference, &difference, sizeof(signed_difference));
        in_range = index < static_cast<std::uint32_t>(signed_difference / 0x2c);
    }
    if (!in_range)
        invoke_native_invalid_parameter_00bf6713();
    const std::uint32_t current_begin =
        *reinterpret_cast<const volatile std::uint32_t*>(bytes + 4);
    const std::uint32_t address = current_begin + index * 0x2cu;
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(address));
}
} // namespace bsp
