#include "bsp/native_pooled_resource_path.hpp"

#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <type_traits>

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4, "The recovered headers and addresses are Win32.");

template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}
template<class T> void write_header(void* header, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(header) + offset, &value, sizeof(value));
}
char* byte_at(char* data, std::uint32_t index) noexcept {
    return reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data) + index);
}
bool signed_positive(std::uint32_t value) noexcept {
    return value != 0 && (value & 0x80000000u) == 0;
}

// The inline copy blocks in these two owners capture destination data before
// source data (00BEE738/3C and 00BEE7C6/CA). Do not cache either across resize.
template<class Storage>
void copy_current_nonempty_source(void* destination, const void* source) {
    if (read_header<std::uint32_t>(source, 0) != 0) {
        const auto count = read_header<std::uint32_t>(destination, 0);
        auto* data = read_header<char*>(destination, 4);
        const auto* source_data = read_header<const char*>(source, 4);
        if (count != 0) {
            if constexpr (std::is_same_v<Storage, NativeStringRawPoolContext>)
                std::memmove(data, source_data, count);
            else std::memcpy(data, source_data, count);
        }
    }
}

template<class Storage> struct RawUnwind {
    void* header;
    Storage& storage;
    bool& active;
    ~RawUnwind() noexcept {
        if constexpr (std::is_same_v<Storage, NativeStringRawPoolContext>) {
            if (active) {
                active = false;
                destroy_native_string_header_0041dd20(header, storage);
            }
        }
    }
};
} // namespace

template<class Storage>
static void normalize_with_storage(void* actual_header, Storage& storage) {
    lowercase_native_string_header_004bcc00(actual_header);
    const auto slash_length = read_header<std::uint32_t>(actual_header, 0);
    if (signed_positive(slash_length)) {
        for (std::uint32_t index = 0; index < slash_length; ++index) {
            auto* byte = byte_at(read_header<char*>(actual_header, 4), index);
            if (*byte == '\\') *byte = '/';
        }
    }

    auto end = read_header<std::uint32_t>(actual_header, 0);
    std::uint32_t start = 0;
    if (end != 0) {
        auto* const data = read_header<char*>(actual_header, 4);
        while (*byte_at(data, start) == ' ') {
            ++start;
            if (start >= end) break;
        }
    }
    --end; // Native DWORD wrap, including empty input.
    if (signed_positive(end)) {
        auto* const data = read_header<char*>(actual_header, 4);
        while (*byte_at(data, end) == ' ') {
            --end;
            if (!signed_positive(end)) break;
        }
    }

    alignas(4) char result[8]; // Constructed below; native stack fields start uninitialized.
    auto* const returned_result = construct_native_string_substring_00469840(
        actual_header, result, start, end - start + 1u, storage);
    bool result_armed = true;
    RawUnwind<Storage> unwind{result, storage, result_armed};
    // EH state 0 starts only after the substring helper returns (00BEE71B).
    try {
        if (actual_header != returned_result) {
            const auto requested = read_header<std::uint32_t>(returned_result, 0);
            resize_native_string_header_0041dd40(actual_header, storage, requested, true);
            copy_current_nonempty_source<Storage>(actual_header, returned_result);
        }
    } catch (...) {
        if constexpr (!std::is_same_v<Storage, NativeStringRawPoolContext>)
            destroy_native_string_header_0041dd20(result, storage); // CC75B0, current fields.
        throw;
    }
    // State -1 precedes normal return of the result buffer. A pool-getter/free
    // exception here does not reenter the result cleanup. Host release is noexcept.
    result_armed = false;
    destroy_native_string_header_0041dd20(result, storage);
}

template<class Storage>
static void* copy_path_with_storage(void* actual_output_header,
    const void* actual_source_header, Storage& storage) {
    const bool identical = actual_output_header == actual_source_header;
    write_header<std::uint32_t>(actual_output_header, 0, 0);
    write_header<char*>(actual_output_header, 4, nullptr);
    if (!identical) {
        const auto requested = read_header<std::uint32_t>(actual_source_header, 0);
        resize_native_string_header_0041dd40(actual_output_header, storage, requested, true);
        copy_current_nonempty_source<Storage>(actual_output_header, actual_source_header);
    }
    // Native flag bit 0 is armed only here (00BEE7DD), after initial copy.
    bool output_constructed = true;
    RawUnwind<Storage> unwind{actual_output_header, storage, output_constructed};
    try {
        normalize_with_storage(actual_output_header, storage);
    } catch (...) {
        if constexpr (!std::is_same_v<Storage, NativeStringRawPoolContext>) {
            if (output_constructed) {
                output_constructed = false; // CC75DC clears the flag before cleanup.
                destroy_native_string_header_0041dd20(actual_output_header, storage);
            }
        }
        throw;
    }
    output_constructed = false;
    return actual_output_header;
}


void normalize_native_resource_path_header_00bee690(void* header, NativeStringStorage& storage) {
    normalize_with_storage(header, storage);
}
void normalize_native_resource_path_header_00bee690(void* header, NativeStringRawPoolContext& storage) {
    normalize_with_storage(header, storage);
}
void* copy_construct_native_resource_path_header_00bee780(void* output, const void* source, NativeStringStorage& storage) {
    return copy_path_with_storage(output, source, storage);
}
void* copy_construct_native_resource_path_header_00bee780(void* output, const void* source, NativeStringRawPoolContext& storage) {
    return copy_path_with_storage(output, source, storage);
}

} // namespace bsp
