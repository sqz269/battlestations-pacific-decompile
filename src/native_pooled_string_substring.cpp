#include "bsp/native_pooled_string_substring.hpp"

#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4, "The recovered headers and address arithmetic are Win32.");

template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}
template<class T> void write_header(void* header, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(header) + offset, &value, sizeof(value));
}
void clear_header(void* header) noexcept {
    write_header<std::uint32_t>(header, 0, 0);
    write_header<char*>(header, 4, nullptr);
}

} // namespace

void* copy_construct_native_string_header_00426060(void* actual_destination_header,
    const void* actual_source_header, NativeStringStorage& storage) {
    const bool identical = actual_destination_header == actual_source_header;
    clear_header(actual_destination_header); // 0042606A/70, before the identity branch.
    if (!identical) {
        const auto requested = read_header<std::uint32_t>(actual_source_header, 0);
        resize_native_string_header_0041dd40(actual_destination_header, storage, requested, true);
        if (read_header<std::uint32_t>(actual_source_header, 0) != 0) {
            const auto copied = read_header<std::uint32_t>(actual_destination_header, 0);
            const auto* data = read_header<const char*>(actual_source_header, 4);
            auto* output = read_header<char*>(actual_destination_header, 4);
            // Keep the existing raw-header helpers' zero-copy host policy.
            if (copied != 0) std::memcpy(output, data, copied);
        }
    }
    return actual_destination_header;
}

void* construct_native_string_substring_00469840(const void* actual_source_header,
    void* actual_output_header, std::uint32_t start, std::uint32_t count,
    NativeStringStorage& storage) {
    alignas(4) char temporary[8]{}; // Only the observed length/data pair.
    bool output_constructed = false; // Native [frame-18h] bit 0.
    bool temporary_cleanup_armed = true; // Native EH state 1, before any source reads.
    try {
        if ((start & 0x80000000u) != 0) { // Signed start < 0, 0046987A.
            count += start;
            start = 0;
        } else {
            const auto length = read_header<std::uint32_t>(actual_source_header, 0);
            if (start > length) start = length;
        }
        const auto* source_data = read_header<const char*>(actual_source_header, 4);
        if (source_data != nullptr) {
            const auto length = read_header<std::uint32_t>(actual_source_header, 0);
            if (length != 0 && start < length) {
                if (start + count > length) count = length - start;
                const auto* source_begin = reinterpret_cast<const char*>(
                    reinterpret_cast<std::uintptr_t>(source_data) + start);
                resize_native_string_header_0041dd40(temporary, storage, count, true);
                auto* const captured_temporary_data = read_header<char*>(temporary, 4);
                // The secure CRT replacement changes this recovered operation.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
                if (count != 0) std::strncpy(captured_temporary_data, source_begin, count);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
                copy_construct_native_string_header_00426060(actual_output_header, temporary, storage);
                output_constructed = true; // Only after 00426060 returns, 004698E4.
                temporary_cleanup_armed = false; // State 0, 004698EC.
                if (captured_temporary_data != nullptr)
                    storage.release(captured_temporary_data,
                        read_header<std::uint32_t>(temporary, 0) + 1u);
                return actual_output_header;
            }
        }
        // The empty branch is also a constructor: an old output is abandoned.
        const bool identical = actual_output_header == temporary;
        clear_header(actual_output_header);
        if (!identical)
            resize_native_string_header_0041dd40(actual_output_header, storage, 0, true);
        return actual_output_header;
    } catch (...) {
        // D891D0: state 1 -> 0 destroys CURRENT temporary fields (C61B50).
        if (temporary_cleanup_armed)
            destroy_native_string_header_0041dd20(temporary, storage);
        // State 0 -> -1 tests and clears the output flag (C61B58). The normal
        // post-construction pool return cannot throw through this host storage's
        // noexcept release, but the recovered flag is intentionally retained.
        if (output_constructed) {
            output_constructed = false;
            destroy_native_string_header_0041dd20(actual_output_header, storage);
        }
        throw;
    }
}

} // namespace bsp
