#include "bsp/native_wide_string.hpp"

#include <cstring>

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

} // namespace bsp
