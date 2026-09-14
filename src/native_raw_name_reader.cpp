#include "bsp/native_raw_name_reader.hpp"
#include "bsp/native_raw_scalar_reader.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw counted-name reader requires MSVC Win32.
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
std::uint32_t current_slot(void* stream, std::uint32_t offset,
    NativeRawScalarReaderContext& context) {
    const auto profile = word(stream);
    if (profile == 0x00d642c0u)
        return context.memory->actual_stream_profile_00d642c0[offset / 4u];
    if (profile == 0x00d691b0u)
        return word(reinterpret_cast<const void*>(profile), offset);
    throw std::invalid_argument("Unimplemented current raw name stream profile");
}
void qualify_slot(void* stream, std::uint32_t offset, std::uint32_t expected,
    NativeRawScalarReaderContext& context) {
    if (current_slot(stream, offset, context) != expected)
        throw std::invalid_argument("Unimplemented current raw name stream slot");
}
void read_bytes(void* stream, void* destination, std::uint32_t requested,
    std::uint32_t* actual, NativeRawScalarReaderContext& context) {
    const auto profile = word(stream);
    if (profile == 0x00d642c0u) {
        if (context.memory->actual_stream_profile_00d642c0[0x24 / 4] != 0x00bef590u)
            throw std::invalid_argument("Unimplemented memory name read slot");
        native_memory_stream_read_00bef590(stream, nullptr, destination, requested, actual);
    } else if (profile == 0x00d691b0u) {
        if (word(reinterpret_cast<const void*>(profile), 0x24) != 0x00bf5030u)
            throw std::invalid_argument("Unimplemented physical name read slot");
        read_native_physical_stream_00bf5030(stream, destination, requested, actual,
            *context.physical);
    } else {
        throw std::invalid_argument("Unimplemented current name read profile");
    }
}
void return_captured_temporary(void* data, std::uint32_t length,
    NativeStringRawPoolContext& strings) {
    if (!data) return;
    const auto size = length + 1u;
    auto* pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, size,
        strings.actual_small_returns_disabled_01090aa4);
}
} // namespace

std::uint32_t read_native_raw_name_length_00be4300(void* stream,
    std::uint32_t* actual, NativeRawScalarReaderContext& context) {
    // Complete native26-byte bodies are identical. Keep pointer-slot seeding
    // in the existing naked source body; do not zero a new DWORD destination.
    return read_native_raw_dword_00be42e0(stream, actual, context);
}

void* read_native_raw_name_00be4620(void* stream, void* output,
    std::uint32_t* actual, NativeRawNameReaderContext& context) {
    std::uint32_t header_actual = 0;
    qualify_slot(stream, 0x38, 0x00be4300, context.streams);
    const auto declared = read_native_raw_name_length_00be4300(
        stream, &header_actual, context.streams);
    std::uint32_t temporary[2] = {0, 0};
    volatile bool result_constructed = false;
    volatile unsigned cleanup_state = 1;
    volatile bool normal_return = false;
    __try {
        if (declared == 0) {
            if (actual) *actual = header_actual;
            put(output, 0, 0);
            put(output, 4, 0);
            if (output != temporary)
                resize_native_string_header_0041dd40(output, context.strings, 0, true);
        } else {
            resize_native_string_header_0041dd40(temporary, context.strings, declared, true);
            const auto captured_length = word(temporary);
            void* const captured_data = pointer(temporary, 4);
            if (captured_length != 0)
                std::memset(captured_data, 0x20, captured_length);
            std::uint32_t payload_actual = 0;
            void* const destination = captured_data ? captured_data
                : reinterpret_cast<void*>(0x0109db64);
            read_bytes(stream, destination, declared, &payload_actual, context.streams);
            if (actual) *actual = header_actual + payload_actual;
            put(output, 0, 0);
            put(output, 4, 0);
            if (output != temporary) {
                resize_native_string_header_0041dd40(
                    output, context.strings, captured_length, true);
                if (captured_length != 0) {
                    const auto copied = word(output);
                    void* const output_data = pointer(output, 4);
                    // BF7680 admits overlap. Zero copy omitted as in the
                    // existing raw string helper, avoiding standard-library UB.
                    if (copied != 0) std::memmove(output_data, captured_data, copied);
                }
            }
            result_constructed = true;
            cleanup_state = 0; // BE4736: disarm temporary BEFORE its normal return.
            return_captured_temporary(captured_data, captured_length, context.strings);
        }
        normal_return = true;
    } __finally {
        if (!normal_return) {
            __try {
                if (cleanup_state == 1)
                    destroy_native_string_header_0041dd20(temporary, context.strings);
            } __finally {
                if (result_constructed) {
                    result_constructed = false;
                    destroy_native_string_header_0041dd20(output, context.strings);
                }
            }
        }
    }
    return output;
}

void* read_native_raw_name_and_debit_00bf0510(void* reader, void* output,
    std::uint32_t* budget, NativeRawNameReaderContext& context) {
    void* const stream = pointer(reader);
    qualify_slot(stream, 0x48, 0x00be4620, context.streams);
    // Native reuses its incoming output argument slot as the count output.
    // This source count retains its initial pointer word, with a new frame.
    auto actual = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(output));
    read_native_raw_name_00be4620(stream, output, &actual, context);
    *reinterpret_cast<volatile std::uint32_t*>(budget) = word(budget) - actual;
    return output;
}
void* read_native_raw_node_name_00be9fe0(void* node, void* output,
    NativeRawNameReaderContext& context) {
    auto* const budget = reinterpret_cast<std::uint32_t*>(
        static_cast<unsigned char*>(node) + 0x20);
    void* const reader = pointer(node, 8);
    read_native_raw_name_and_debit_00bf0510(reader, output, budget, context);
    return output;
}
void* read_native_raw_node_handle_name_00bea010(const void* wrapper, void* output,
    NativeRawNameReaderContext& context) {
    read_native_raw_node_name_00be9fe0(pointer(wrapper), output, context);
    return output;
}
} // namespace bsp
