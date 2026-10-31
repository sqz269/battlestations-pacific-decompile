#include "bsp/native_lua_key_integer.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <climits>
#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {

static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4);

void* raw_address(const void* base, std::uint32_t offset) noexcept {
    const std::uint32_t bits =
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(base)) + offset;
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

std::uint32_t raw_word(const void* base, std::uint32_t offset) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, raw_address(base, offset), sizeof value);
    _ReadWriteBarrier();
    return value;
}

void raw_store(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(raw_address(base, offset), &value, sizeof value);
    _ReadWriteBarrier();
}

} // namespace

void set_native_lua_key_integer_007814e0(
    void* actual_key16, const void* actual_input_word_home,
    NativeStringRawPoolContext& actual_strings) {
    void* const captured_key = actual_key16;
    const std::uint32_t initial_length = raw_word(captured_key, 4u);
    if (initial_length == 0u) {
        const std::uint32_t input = raw_word(actual_input_word_home, 0u);
        raw_store(captured_key, 12u, input);
        raw_store(captured_key, 0u, 0u);
        return;
    }

    const std::uint32_t data_bits = raw_word(captured_key, 8u);
    if (data_bits != 0u) {
        void* const captured_block =
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(data_bits));
        const std::uint32_t captured_size = initial_length + std::uint32_t{1};
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            actual_strings.actual_published_01090aa8,
            actual_strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, captured_block, captured_size,
            actual_strings.actual_small_returns_disabled_01090aa4);
    }

    const std::uint32_t input = raw_word(actual_input_word_home, 0u);
    raw_store(captured_key, 8u, 0u);
    raw_store(captured_key, 4u, 0u);
    raw_store(captured_key, 12u, input);
    raw_store(captured_key, 0u, 0u);
}

} // namespace bsp
