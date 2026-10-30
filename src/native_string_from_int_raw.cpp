#include "bsp/native_string_from_int_raw.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdio>
#include <cstring>
#include <intrin.h>
#include <stdexcept>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4, "004260B0 raw storage requires Win32");

std::uint32_t raw_word(const void* header, std::size_t offset) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, static_cast<const unsigned char*>(header) + offset,
        sizeof(value));
    _ReadWriteBarrier();
    return value;
}

void raw_put(void* header, std::size_t offset, std::uint32_t value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(static_cast<unsigned char*>(header) + offset, &value,
        sizeof(value));
    _ReadWriteBarrier();
}

char* raw_data(const void* header) noexcept {
    return reinterpret_cast<char*>(static_cast<std::uintptr_t>(raw_word(header, 4)));
}

class TemporaryUnwind final {
public:
    TemporaryUnwind(void* header, NativeStringRawPoolContext& strings) noexcept
        : header_(header), strings_(strings) {}

    ~TemporaryUnwind() noexcept {
        if (armed_) {
            armed_ = false;
            destroy_native_string_header_0041dd20(header_, strings_);
        }
    }

    TemporaryUnwind(const TemporaryUnwind&) = delete;
    TemporaryUnwind& operator=(const TemporaryUnwind&) = delete;

    void disarm() noexcept { armed_ = false; }

private:
    void* const header_;
    NativeStringRawPoolContext& strings_;
    bool armed_{true};
};

} // namespace

void* construct_native_string_from_int_004260b0(void* actual_output8,
    std::int32_t value, const NativeStringFromIntRawContext& context) {
    if (!context.actual_format_00ce3a34 || context.actual_format_bytes != 3 ||
        context.actual_format_00ce3a34[0] != '%' ||
        context.actual_format_00ce3a34[1] != 'd' ||
        context.actual_format_00ce3a34[2] != '\0')
        throw std::invalid_argument("004260B0 requires the actual CE3A34 format span");

    raw_put(actual_output8, 0, 0);
    raw_put(actual_output8, 4, 0);
    char buffer[52];
    (void)std::snprintf(buffer, sizeof(buffer), context.actual_format_00ce3a34,
        static_cast<int>(value));

    alignas(4) unsigned char temporary[8];
    raw_put(temporary, 0, 0);
    raw_put(temporary, 4, 0);
    const auto buffer_length = static_cast<std::uint32_t>(std::strlen(buffer));
    resize_native_string_header_0041dd40(temporary, context.strings,
        buffer_length, true);
    char* const captured_data = raw_data(temporary);
    const auto captured_length = raw_word(temporary, 0);
    if (captured_data) {
        const auto bytes = captured_length + std::uint32_t{1};
        if (bytes != 0)
            std::memmove(captured_data, buffer, bytes);
    }

    const bool same_header = actual_output8 == static_cast<void*>(temporary);
    TemporaryUnwind unwind(temporary, context.strings);
    if (!same_header) {
        resize_native_string_header_0041dd40(actual_output8, context.strings,
            captured_length, true);
        if (captured_length != 0) {
            const auto current_length = raw_word(actual_output8, 0);
            char* const current_data = raw_data(actual_output8);
            if (current_length != 0)
                std::memmove(current_data, captured_data, current_length);
        }
    }

    unwind.disarm();
    if (captured_data) {
        const auto bytes = captured_length + std::uint32_t{1};
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            context.strings.actual_published_01090aa8,
            context.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, captured_data, bytes,
            context.strings.actual_small_returns_disabled_01090aa4);
    }
    return actual_output8;
}

} // namespace bsp
