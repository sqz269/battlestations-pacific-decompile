#include "bsp/native_string_allocate_storage.hpp"

#include "bsp/native_string_pool_storage.hpp"

namespace bsp {

char* allocate_native_string_storage_0041cb50(std::uint32_t length,
    NativeStringRawPoolContext& context) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        context.actual_published_01090aa8,
        context.actual_manager_publication_01090aa0); // 0041CB53
    const std::uint32_t size = length + 1u; // 0041CB5A, after getter
    return static_cast<char*>(allocate_native_string_pool_00bd1120(pool, size)); // 0041CB60
}

} // namespace bsp
