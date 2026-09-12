#include "bsp/native_vfs_open_logging.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS open logging requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* pointer(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
std::uint8_t byte(const void* p, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(p) + offset);
}
void clear_header(void* p) noexcept {
    *static_cast<volatile std::uint32_t*>(p) = 0;
    *static_cast<volatile std::uint32_t*>(at(p, 4)) = 0;
}
void release_capture(void* data, std::uint32_t length, NativeStringStorage& strings) noexcept {
    if (data) strings.release(static_cast<char*>(data), length + 1u);
}
} // namespace

void* construct_native_log_builder_004264a0(void* builder, NativeStringStorage& strings) {
    clear_header(builder);
    try {
        construct_native_string_cstring_0041e870(at(builder, 8), "%d", strings);
    } catch (...) {
        destroy_native_string_header_0041dd20(builder, strings);
        throw;
    }
    try {
        construct_native_string_cstring_0041e870(at(builder, 0x10), "%.3f", strings);
    } catch (...) {
        destroy_native_string_header_0041dd20(at(builder, 8), strings);
        destroy_native_string_header_0041dd20(builder, strings);
        throw;
    }
    return builder;
}

void* construct_native_log_builder_00426500(void* builder, NativeStringStorage& strings) {
    construct_native_log_builder_004264a0(builder, strings);
    return builder;
}

void* append_native_log_header_00bd1a20(void* builder, const void* source,
    NativeStringStorage& strings) {
    const auto length = word(source);
    if (length) {
        const auto old_length = word(builder);
        resize_native_string_header_0041dd40(builder, strings, old_length + length, true);
        const auto* input = pointer(source, 4);
        auto* output = pointer(builder, 4);
        std::memcpy(at(output, old_length), input, length);
    }
    return builder;
}

void* append_native_log_cstring_00bd1a60(void* builder, const char* source,
    NativeStringStorage& strings) {
    if (!source) return builder;
    std::uint32_t temporary[2];
    construct_native_string_cstring_0041e870(temporary, source, strings);
    const auto length = word(temporary);
    auto* data = pointer(temporary, 4);
    try {
        if (length) {
            const auto old_length = word(builder);
            resize_native_string_header_0041dd40(builder, strings, old_length + length, true);
            std::memcpy(at(pointer(builder, 4), old_length), data, length);
        }
    } catch (...) {
        // CC54D0's state0 unwind rereads the actual local header. The normal
        // return instead uses EBP/EDI captured before the destination resize.
        destroy_native_string_header_0041dd20(temporary, strings);
        throw;
    }
    release_capture(data, length, strings);
    return builder;
}

void destroy_native_log_builder_00425f80(void* builder, NativeStringStorage& strings) noexcept {
    destroy_native_string_header_0041dd20(at(builder, 0x10), strings);
    destroy_native_string_header_0041dd20(at(builder, 8), strings);
    destroy_native_string_header_0041dd20(builder, strings);
}

void log_native_vfs_opened_resource_00bde9c0(void* manager, const void* name,
    void* unused_stream, std::uint32_t mount_byte, NativeVfsOpenLoggingContext& context) {
    (void)unused_stream;
    auto& strings = context.strings;
    bool suppress = false;
    std::uint32_t temporary[2] = {};
    resize_native_string_header_0041dd40(temporary, strings, 1, true);
    auto* dot_data = pointer(temporary, 4);
    const auto dot_length = word(temporary);
    if (dot_data) std::memcpy(dot_data, ".", dot_length + 1u);
    const auto dot = reverse_find_native_string_header_00467cf0(name, temporary, 0x7fffffffu);
    release_capture(dot_data, dot_length, strings);
    if (dot > 0) {
        // The preceding release deliberately leaves the dangling header;
        // 41E870 clears it before constructing the underscore temporary.
        construct_native_string_cstring_0041e870(temporary, "_", strings);
        const auto underscore = reverse_find_native_string_header_00467cf0(
            name, temporary, static_cast<std::uint32_t>(dot));
        destroy_native_string_header_0041dd20(temporary, strings);
        if (underscore > 0 && static_cast<std::uint32_t>(dot - underscore) == 8u) {
            suppress = true;
            auto position = static_cast<std::uint32_t>(underscore) + 1u;
            while (static_cast<std::int32_t>(position) < dot && suppress) {
                const auto value = byte(pointer(name, 4), position);
                suppress = value >= '0' && value <= '9';
                ++position;
            }
        }
    }
    if (!context.file_log_0109cee8 || !byte(manager, 0x79) ||
        static_cast<std::uint8_t>(mount_byte) == 0 || suppress) return;

    std::uint32_t builder[6];
    construct_native_log_builder_00426500(builder, strings);
    try {
        auto* current = append_native_log_cstring_00bd1a60(builder, "<FILE><", strings);
        current = append_native_log_header_00bd1a20(current, name, strings);
        append_native_log_cstring_00bd1a60(current, ">", strings);
    } catch (...) {
        destroy_native_log_builder_00425f80(builder, strings);
        throw;
    }
    destroy_native_log_builder_00425f80(builder, strings);
}
} // namespace bsp
