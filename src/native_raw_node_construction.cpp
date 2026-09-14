#include "bsp/native_raw_node_construction.hpp"
#include "bsp/native_raw_name_reader.hpp"
#include "bsp/native_raw_scalar_reader.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native raw child construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const unsigned char*>(owner) + offset);
}
void put(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(owner) + offset) = value;
}
void* pointer(const void* owner, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(word(owner, offset)));
}
void* raw_path_header(void* reader, std::uint32_t index) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(reader)
        + 0x10u + index * 8u;
    return reinterpret_cast<void*>(address);
}
void return_captured_temporary(void* data, std::uint32_t length,
    NativeStringRawPoolContext& strings) {
    if (!data) return;
    // Native arguments are pending before the no-argument getter call.
    const auto size = length + 1u;
    auto* pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,
        strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, size,
        strings.actual_small_returns_disabled_01090aa4);
}
} // namespace

void* construct_native_raw_child_node_00bea250(void* node, void* parent,
    NativeRawNameReaderContext& context) {
    put(node, 0, 0x00ceb130u); // BD30F0 base identity; state0 is now armed.
    volatile unsigned state = 0;
    volatile bool normal_return = false;
    std::uint32_t temporary[2];
    temporary[1] = 0; // BEA296 initializes only the data word before BF0510.
    __try {
        put(node, 4, 1);
        put(node, 0, 0x00d68bb4u);
        put(node, 8, word(parent, 8));
        put(node, 0x0c, static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(parent)));
        put(node, 0x10, 0);
        put(node, 0x14, 0);
        state = 1; // CC7158 destroys node name, then CC7150 stamps base.
        put(node, 0x18, word(parent, 0x18) + 1u);

        void* const returned = read_native_raw_name_and_debit_00bf0510(
            pointer(node, 8), temporary,
            reinterpret_cast<std::uint32_t*>(
                static_cast<unsigned char*>(pointer(node, 0x0c)) + 0x20),
            context);
        state = 2; // CC7163 destroys the actual local temporary first.
        void* const name = static_cast<unsigned char*>(node) + 0x10;
        if (name != returned) {
            resize_native_string_header_0041dd40(
                name, context.strings, word(returned), true);
            if (word(returned) != 0) {
                const auto length = word(name);
                if (length != 0)
                    std::memmove(pointer(name, 4), pointer(returned, 4), length);
            }
        }

        void* const temporary_data = pointer(temporary, 4);
        const auto temporary_length = word(temporary);
        state = 1; // BEA2F8 disarms state2 BEFORE pool getter/return.
        return_captured_temporary(temporary_data, temporary_length, context.strings);

        const auto payload = read_native_raw_dword_and_debit_00bf0280(
            pointer(node, 8),
            reinterpret_cast<std::uint32_t*>(
                static_cast<unsigned char*>(pointer(node, 0x0c)) + 0x20),
            context.streams);
        put(node, 0x1c, payload);
        put(node, 0x20, payload);

        void* const reader = pointer(node, 8);
        const auto index = word(reader, 0x60);
        void* const path = raw_path_header(reader, index);
        put(reader, 0x60, index + 1u); // BEA33C publishes before resize.
        if (path != name) {
            resize_native_string_header_0041dd40(
                path, context.strings, word(name), true);
            if (word(name) != 0) {
                const auto length = word(path);
                if (length != 0)
                    std::memmove(pointer(path, 4), pointer(name, 4), length);
            }
        }
        normal_return = true;
    } __finally {
        if (!normal_return) {
            __try {
                if (state == 2)
                    destroy_native_string_header_0041dd20(temporary, context.strings);
            } __finally {
                __try {
                    if (state >= 1)
                        destroy_native_string_header_0041dd20(
                            static_cast<unsigned char*>(node) + 0x10,
                            context.strings);
                } __finally {
                    destroy_native_ref_counted_base_00bd30f0(node);
                }
            }
        }
    }
    return node;
}
} // namespace bsp
