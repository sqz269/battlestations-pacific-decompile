#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <climits>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
static_assert(CHAR_BIT == 8 && sizeof(std::uint32_t) == 4);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

std::uint32_t address_bits(const void* base, std::uint32_t offset) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(base)) + offset;
}

void* pointer_from_bits(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

template<class T>
T read_raw(const void* base, std::uint32_t offset) noexcept {
    static_assert(sizeof(T) == 1 || sizeof(T) == 4);
    T value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer_from_bits(address_bits(base, offset)), sizeof(value));
    _ReadWriteBarrier();
    return value;
}

template<class T>
void write_raw(void* base, std::uint32_t offset, T value) noexcept {
    static_assert(sizeof(T) == 1 || sizeof(T) == 4);
    _ReadWriteBarrier();
    std::memcpy(pointer_from_bits(address_bits(base, offset)), &value, sizeof(value));
    _ReadWriteBarrier();
}

NativeStringPoolStorage* current_pool(NativeStringRawPoolContext& context) {
    return native_string_pool_get_or_create_00419cc0(
        context.actual_published_01090aa8,
        context.actual_manager_publication_01090aa0);
}
} // namespace

void resize_native_string_header_0041dd40_exact(
    void* actual_header, NativeStringRawPoolContext& context,
    std::uint32_t request, const void* actual_preserve_byte_home,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    const auto initial_length = read_raw<std::uint32_t>(actual_header, 0u);
    if (request == initial_length) return;

    if (request == 0u) {
        const auto old_data = read_raw<std::uint32_t>(actual_header, 4u);
        if (old_data != 0u) {
            const std::uint32_t old_size = initial_length + 1u;
            auto* const pool = current_pool(context);
            return_native_string_pool_00bd1510(pool, pointer_from_bits(old_data),
                old_size, context.actual_small_returns_disabled_01090aa4);
        }
        write_raw<std::uint32_t>(actual_header, 4u, 0u);
        write_raw<std::uint32_t>(actual_header, 0u, 0u);
        return;
    }

    auto* const allocation_pool = current_pool(context);
    void* const new_block = allocate_native_string_pool_00bd1120(
        allocation_pool, request + 1u);
    const auto new_data = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(new_block));

    // DD95 reads this actual BYTE HOME after the real allocation returns.
    if (read_raw<std::uint8_t>(actual_preserve_byte_home, 0u) != 0u) {
        const auto current_length = read_raw<std::uint32_t>(actual_header, 0u);
        const auto copied = request > current_length ? current_length : request;
        const auto old_data = read_raw<std::uint32_t>(actual_header, 4u);
        // Native calls BF7680 even when copied==0. The real provider owns its
        // current feature/overlap/zero behavior; retain the captured allocation
        // independently of its returned EAX.
        copy_native_crt_bytes_00bf7680(new_block, pointer_from_bits(old_data),
            copied, actual_feature_word_0109eea4);
    }

    const auto old_data = read_raw<std::uint32_t>(actual_header, 4u);
    if (old_data != 0u) {
        const std::uint32_t old_size =
            read_raw<std::uint32_t>(actual_header, 0u) + 1u;
        auto* const pool = current_pool(context);
        return_native_string_pool_00bd1510(pool, pointer_from_bits(old_data),
            old_size, context.actual_small_returns_disabled_01090aa4);
    }

    write_raw<std::uint32_t>(actual_header, 4u, new_data);
    write_raw<std::uint32_t>(actual_header, 0u, request);
    write_raw<std::uint8_t>(new_block, request, 0u);
}
} // namespace bsp
