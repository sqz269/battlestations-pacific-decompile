#include "bsp/native_scene_record_map_subtree_destroy.hpp"

#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}

std::uint32_t word(std::uint32_t base, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(base + offset);
}

bool sentinel(std::uint32_t node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(node + 0x19u) != 0;
}

} // namespace

void destroy_native_scene_record_map_subtree_0046ad10(
    void* actual_root, NativeStringRawPoolContext& strings) {
    auto current = address(actual_root);
    while (!sentinel(current)) {
        const auto right = word(current, 8);
        destroy_native_scene_record_map_subtree_0046ad10(pointer(right), strings);

        const auto name_data = word(current, 0x10);
        const auto captured_left = word(current);
        if (name_data != 0) {
            const auto returned_size = word(current, 0x0c) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8,
                strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, pointer(name_data), returned_size,
                strings.actual_small_returns_disabled_01090aa4);
        }
        singleton_lifetime_free(pointer(current));
        current = captured_left;
    }
}

} // namespace bsp
