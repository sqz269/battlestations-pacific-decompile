#include "bsp/native_wide_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint16_t) == 2);

template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}

template<class T> void write_header(void* header, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(header) + offset, &value, sizeof(value));
}

// Raw-context byte helpers: fence CURRENT header epochs, without typed fields.
std::uint32_t raw_wide_word(const void* header, std::uint32_t offset) noexcept {
    std::uint32_t value;
    const auto location = reinterpret_cast<std::uintptr_t>(header) + offset;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(location), sizeof(value));
    _ReadWriteBarrier();
    return value;
}

template<class T> void raw_wide_store(void* header, std::uint32_t offset, T value) noexcept {
    const auto location = reinterpret_cast<std::uintptr_t>(header) + offset;
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(location), &value, sizeof(value));
    _ReadWriteBarrier();
}
// End raw-context byte helpers.

} // namespace

void resize_native_wide_string_header_004c53e0(void* actual_header,
    NativeStringStorage& storage, std::uint32_t length, bool preserve) {
    const auto initial_length = read_header<std::uint32_t>(actual_header, 0);
    if (length == initial_length) return; // 004c53ec: do not read the pointer.

    if (length == 0) {
        auto* const old_data = read_header<char*>(actual_header, 4);
        if (old_data != nullptr) storage.release(old_data, initial_length * 2u + 2u);
        write_header<char*>(actual_header, 4, nullptr); // 004c5411
        write_header<std::uint32_t>(actual_header, 0, 0);
        return;
    }

    const auto byte_length = length * 2u;
    char* const block = storage.allocate(byte_length + 2u); // 004c5436
    if (preserve) {
        // Allocation may have changed either actual header field. Native uses
        // the unsigned minimum before doubling, with no capacity field.
        const auto current_length = read_header<std::uint32_t>(actual_header, 0);
        const auto copy_length = length > current_length ? current_length : length;
        const auto* const source = read_header<char*>(actual_header, 4);
        const auto copy_bytes = copy_length * 2u;
        if (copy_bytes != 0) std::memcpy(block, source, copy_bytes);
    }
    // Reread after memcpy: the destination can alias the actual header.
    auto* const old_data = read_header<char*>(actual_header, 4); // 004c5462
    if (old_data != nullptr)
        storage.release(old_data, read_header<std::uint32_t>(actual_header, 0) * 2u + 2u);
    write_header<char*>(actual_header, 4, block); // 004c547f
    write_header<std::uint32_t>(actual_header, 0, length);
    const std::uint16_t terminator = 0;
    std::memcpy(reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(block) + byte_length),
        &terminator, sizeof(terminator)); // 004c5484: store after both header writes.
}

void resize_native_wide_string_header_004c53e0(void* actual_header,
    NativeStringRawPoolContext& context, std::uint32_t length, bool preserve) {
    const auto initial_length = raw_wide_word(actual_header, 0u);
    if (length == initial_length) return; // no pointer/getter on equality

    if (length == 0u) {
        const auto old_data = raw_wide_word(actual_header, 4u);
        if (old_data != 0u) {
            const auto bytes = initial_length * 2u + 2u; // BEFORE getter
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                context.actual_published_01090aa8,
                context.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool,
                reinterpret_cast<void*>(old_data), bytes,
                context.actual_small_returns_disabled_01090aa4);
        }
        raw_wide_store(actual_header, 4u, std::uint32_t{0});
        raw_wide_store(actual_header, 0u, std::uint32_t{0});
        return;
    }

    auto* const allocation_pool = native_string_pool_get_or_create_00419cc0(
        context.actual_published_01090aa8,
        context.actual_manager_publication_01090aa0);
    const auto byte_length = length * 2u; // Native DWORD wrap, AFTER getter
    void* const block = allocate_native_string_pool_00bd1120(
        allocation_pool, byte_length + 2u);
    if (preserve) {
        const auto current_length = raw_wide_word(actual_header, 0u);
        const auto copy_length = length > current_length ? current_length : length;
        const auto source = raw_wide_word(actual_header, 4u);
        const auto copy_bytes = copy_length * 2u;
        // BF7680 admits overlap; retain CURRENT operand reads even at count0.
        if (copy_bytes != 0u)
            std::memmove(block, reinterpret_cast<const void*>(source), copy_bytes);
    }
    const auto old_data = raw_wide_word(actual_header, 4u); // AFTER copy
    if (old_data != 0u) {
        const auto bytes = raw_wide_word(actual_header, 0u) * 2u + 2u;
        auto* const return_pool = native_string_pool_get_or_create_00419cc0(
            context.actual_published_01090aa8,
            context.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(return_pool,
            reinterpret_cast<void*>(old_data), bytes,
            context.actual_small_returns_disabled_01090aa4);
    }
    raw_wide_store(actual_header, 4u,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(block)));
    raw_wide_store(actual_header, 0u, length);
    raw_wide_store(block, byte_length, std::uint16_t{0});
}

void* construct_native_wide_string_header_004c5e60(void* actual_header,
    const char* source, NativeStringStorage& storage) {
    write_header<std::uint32_t>(actual_header, 0, 0); // 004c5e6a
    write_header<char*>(actual_header, 4, nullptr); // 004c5e70; no release.

    const auto source_address = reinterpret_cast<std::uintptr_t>(source);
    auto scan = source_address;
    unsigned char byte;
    do {
        byte = *reinterpret_cast<const volatile unsigned char*>(scan);
        ++scan;
    } while (byte != 0); // 004c5e80: measure only after clearing the header.
    resize_native_wide_string_header_004c53e0(actual_header, storage,
        scan - (source_address + 1u), true);

    auto* const data = read_header<char*>(actual_header, 4); // 004c5e95
    if (data != nullptr) {
        auto input = source_address;
        auto output = reinterpret_cast<std::uintptr_t>(data);
        do {
            // Streaming read/store preserves overlap with source or header;
            // neither strlen's result nor the header length bounds this loop.
            byte = *reinterpret_cast<const volatile unsigned char*>(input);
            const auto unit = static_cast<std::uint16_t>(byte);
            std::memcpy(reinterpret_cast<void*>(output), &unit, sizeof(unit));
            output += 2u;
            ++input;
        } while (byte != 0);
    }
    return actual_header;
}

void destroy_native_wide_string_header_00436430(void* actual_header,
    NativeStringStorage& storage) noexcept {
    auto* const data = read_header<char*>(actual_header, 4);
    if (data != nullptr)
        storage.release(data, read_header<std::uint32_t>(actual_header, 0) * 2u + 2u);
}

void destroy_native_wide_string_header_00436430(void* actual_header,
    NativeStringRawPoolContext& context) {
    auto* const data = read_header<char*>(actual_header, 4);
    if (data != nullptr) {
        const auto size = read_header<std::uint32_t>(actual_header, 0) * 2u + 2u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            context.actual_published_01090aa8,
            context.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, data, size,
            context.actual_small_returns_disabled_01090aa4);
    }
}

void* copy_construct_native_wide_string_header_004c8dd0(void* actual_header,
    const void* actual_source, NativeStringStorage& storage) {
    auto* const destination = static_cast<volatile std::uint32_t*>(actual_header);
    const auto* const source = static_cast<const volatile std::uint32_t*>(actual_source);
    const bool same_header = actual_header == actual_source; // 004C8DD8
    destination[0] = 0; // 004C8DDA, INCLUDING self identity
    destination[1] = 0; // 004C8DE0; never return abandoned constructor preimage
    if (!same_header) {
        const auto length = source[0]; // 004C8DE9, request captured before resize
        resize_native_wide_string_header_004c53e0(actual_header, storage, length, true);
        if (source[0] != 0) { // 004C8DF3, CURRENT length after allocation callbacks
            std::uintptr_t input = source[1]; // 004C8DF8, capture data only now
            std::uintptr_t output = destination[1]; // 004C8DFB, after source data
            std::uint16_t unit;
            do {
                unit = *reinterpret_cast<const volatile std::uint16_t*>(input);
                *reinterpret_cast<volatile std::uint16_t*>(output) = unit;
                input += 2u;
                output += 2u;
            } while (unit != 0); // 004C8E00..004C8E0F includes NUL, no count bound
        }
    }
    return actual_header;
}

} // namespace bsp

namespace bsp {

void* assign_native_wide_string_header_004c5e20(void* actual_header,
    const void* actual_source, NativeStringStorage& storage) {
    if (actual_header == actual_source) return actual_header; // 004C5E28
    auto* const destination = static_cast<volatile std::uint32_t*>(actual_header);
    const auto* const source = static_cast<const volatile std::uint32_t*>(actual_source);
    const auto length = source[0]; // 004C5E2C, captured resize request
    resize_native_wide_string_header_004c53e0(actual_header, storage, length, true);
    if (source[0] != 0) { // 004C5E36, CURRENT after allocation/release callbacks
        std::uintptr_t input = source[1]; // 004C5E3B, only after nonzero length
        std::uintptr_t output = destination[1]; // 004C5E3E, after source data
        std::uint16_t unit;
        do {
            unit = *reinterpret_cast<const volatile std::uint16_t*>(input);
            *reinterpret_cast<volatile std::uint16_t*>(output) = unit;
            input += 2u;
            output += 2u;
        } while (unit != 0); // 004C5E41..004C5E50, includes NUL
    }
    return actual_header;
}

} // namespace bsp
