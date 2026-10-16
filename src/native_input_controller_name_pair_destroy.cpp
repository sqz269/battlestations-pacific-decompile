#include "bsp/native_input_controller_name_pair_destroy.hpp"
#include "bsp/native_input_int_string_tree_range.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(base) + offset);
}

void put(void* base, std::size_t offset, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::byte*>(base) + offset) = value;
}

void* pointer(Word bits) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

struct PairKeyCleanup {
    void* actual_pair;
    NativeStringRawPoolContext& strings;
    bool armed;

    ~PairKeyCleanup() noexcept(false) {
        if (armed)
            cleanup_native_input_controller_name_pair_key_00c7f200(actual_pair, strings);
    }

    void disarm() noexcept { armed = false; }
};
} // namespace

void __fastcall cleanup_native_input_controller_name_pair_key_00c7f200(
    void* actual_pair, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(actual_pair, strings);
}

void __fastcall destroy_native_input_controller_name_pair_006a2bb0(
    void* actual_pair, std::uint32_t unused_edx,
    NativeStringRawPoolContext& strings) {
    static_cast<void>(unused_edx);
    Word captured_data;
    {
        auto* const initial_head = pointer(word(actual_pair, 0x0c));
        auto* const captured_begin = pointer(word(initial_head, 0));
        auto* const embedded_tree = static_cast<std::byte*>(actual_pair) + 8;
        NativeInputIntStringTreeIterator ignored_output;
        const NativeInputIntStringTreeIterator first{embedded_tree, captured_begin};
        const NativeInputIntStringTreeIterator last{embedded_tree, initial_head};

        // Original state0 starts after captures, immediately before the range.
        PairKeyCleanup cleanup{actual_pair, strings, true};
        erase_native_input_int_string_tree_range_006a1b70(
            embedded_tree, 0u, &ignored_output, first, last, strings);
        auto* const current_head = pointer(word(embedded_tree, 4));
        singleton_lifetime_free(current_head);
        put(embedded_tree, 4, 0u);
        put(embedded_tree, 8, 0u);
        captured_data = word(actual_pair, 4);
        cleanup.disarm();
    }

    // Guard lifetime/EH scope has ended. A normal getter failure must never
    // retry key cleanup or reread the already captured data pointer.
    if (captured_data != 0) {
        const auto returned_size = word(actual_pair, 0) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(captured_data), returned_size,
            strings.actual_small_returns_disabled_01090aa4);
    }
}
} // namespace bsp
