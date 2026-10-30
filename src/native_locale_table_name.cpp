#include "bsp/native_locale_table_name.hpp"

#include "bsp/native_string_vector_append.hpp"

#include <cstdint>

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(std::uint32_t) == 4, "requires the Win32 address model");

void register_native_locale_table_name_00aa0d30(void* actual_manager,
    const void* actual_name_header8, NativeStringRawPoolContext& strings) {
    const auto manager = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_manager));
    void* const vector = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(manager + 4u));
    // The current Source append contract does not consume incoming EDX.
    append_native_string_header_vector_00450540(
        vector, 0u, actual_name_header8, strings);
}
} // namespace bsp
