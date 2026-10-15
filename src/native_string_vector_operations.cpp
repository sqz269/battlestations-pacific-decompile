#include "bsp/native_string_vector_operations.hpp"

#include "bsp/lua_cache_tree_helpers.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(std::uintptr_t) == 4);

bool __cdecl equal_native_string_range_00b6c670(const void* first,
    const void* last, const void* second_first) {
    std::uint32_t result[2] = {};
    const auto end = reinterpret_cast<std::uintptr_t>(last);
    const auto* mismatch = static_cast<const std::uint32_t*>(
        mismatch_native_string_range_00b6c480(result, first, last, second_first,
            end, 0u));
    return *mismatch == end;
}

void destroy_native_string_range_dispatch_00432360(void*,
    void* first, void* last, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_range_00432050(first, last, strings);
}

} // namespace bsp
