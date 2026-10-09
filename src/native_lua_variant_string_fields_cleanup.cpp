#include "bsp/native_lua_variant_string_fields_cleanup.hpp"

#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

void return_native_lua_variant_string_fields_006ee020(
    volatile std::uint32_t& actual_length_04,
    void* volatile& actual_buffer_08,
    NativeStringPoolStorage* volatile& actual_published_01090aa8,
    void* volatile& actual_manager_publication_01090aa0,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4) {
    void* const captured_block = actual_buffer_08; // 006EE020, before length
    if (captured_block == nullptr) return;        // 006EE023..006EE025

    const std::uint32_t captured_length = actual_length_04; // 006EE027
    const std::uint32_t captured_size = captured_length + std::uint32_t{1}; // 006EE02C
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        actual_published_01090aa8, actual_manager_publication_01090aa0); // 006EE031
    return_native_string_pool_00bd1510(pool, captured_block, captured_size,
        actual_small_returns_disabled_01090aa4); // 006EE038; no unused-word argument
}

} // namespace bsp
