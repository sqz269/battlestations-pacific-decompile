#include "bsp/native_input_int_string_subtree.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
namespace {
using Word = std::uint32_t;

Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(base) + offset);
}

void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

bool is_nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(node) + 0x19) != 0;
}
} // namespace

void __fastcall destroy_native_input_int_string_subtree_006a0aa0(
    void* opaque_tree, std::uint32_t unused_edx, void* actual_node,
    NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    auto* current = actual_node;
    while (!is_nil(current)) {
        auto* const right = pointer(word(current, 8));
        destroy_native_input_int_string_subtree_006a0aa0(
            opaque_tree, 0u, right, strings);

        const auto data = word(current, 0x14);
        auto* const captured_left = pointer(word(current, 0));
        if (data != 0) {
            const auto returned_size = word(current, 0x10) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8,
                strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, pointer(data), returned_size,
                strings.actual_small_returns_disabled_01090aa4);
        }
        singleton_lifetime_free(current);
        current = captured_left;
    }
}

} // namespace bsp
