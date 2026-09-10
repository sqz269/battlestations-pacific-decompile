#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native legacy SBO string reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t maximum_length = 0xFFFFFFFEU;
constexpr std::uint32_t all_remaining = 0xFFFFFFFFU;

std::uintptr_t address(const void* pointer) noexcept {
    return reinterpret_cast<std::uintptr_t>(pointer);
}
template<class T> T* offset_pointer(T* pointer, std::uint32_t offset) noexcept {
    return reinterpret_cast<T*>(address(pointer) + offset);
}
[[noreturn]] void length_error() {
    // 00BF5695: native length_error throw metadata 00D83F98, message 00D69274.
    throw std::length_error("string too long");
}
[[noreturn]] void range_error() {
    // 00BF56D4: native out_of_range throw metadata 00D863A8, message 00D69284.
    throw std::out_of_range("invalid string position");
}
void finish_growth(NativeLegacySboStringStorage& string, char* replacement,
    std::uint32_t capacity, std::uint32_t preserve_count) {
    if (preserve_count != 0) {
        (void)memcpy_s(replacement, capacity + 1U, string.data(), preserve_count);
    }
    if (string.capacity_18 >= 16) {
        singleton_lifetime_free(string.buffer_04.heap);
    }
    string.buffer_04.inline_bytes[0] = '\0';
    string.buffer_04.heap = replacement;
    string.capacity_18 = capacity;
    string.length_14 = preserve_count;
    // Direct calls can retain the native inline selection for a small capacity.
    *offset_pointer(string.data(), preserve_count) = '\0';
}
} // namespace

char* native_legacy_sbo_string_allocate_00408b60(std::uint32_t bytes) {
    // Native DIV computes UINT_MAX / bytes; for a one-byte element, its
    // quotient < 1 bad_alloc branch is unreachable for every nonzero uint32.
    // Zero still calls operator new(0). The concrete service retries through
    // _callnewh and throws bad_alloc when allocation is exhausted (00BF681B).
    return static_cast<char*>(singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes}));
}

void native_legacy_sbo_string_destroy_004072d0(
    NativeLegacySboStringStorage& string) noexcept {
    if (string.capacity_18 >= 16) {
        singleton_lifetime_free(string.buffer_04.heap);
    }
    string.capacity_18 = 15;
    string.length_14 = 0;
    string.buffer_04.inline_bytes[0] = '\0';
}

void native_legacy_sbo_string_grow_004089e0(NativeLegacySboStringStorage& string,
    std::uint32_t requested_capacity, std::uint32_t preserve_count) {
    std::uint32_t capacity = requested_capacity | 15U;
    if (capacity > maximum_length) {
        capacity = requested_capacity;
    } else {
        const auto previous_capacity = string.capacity_18;
        const auto half = previous_capacity >> 1U;
        if (capacity / 3U < half && previous_capacity <= maximum_length - half) {
            capacity = previous_capacity + half;
        }
    }
    std::uint32_t retry_value = requested_capacity;
    char* fallback_replacement;
    try {
        auto* const replacement = native_legacy_sbo_string_allocate_00408b60(capacity + 1U);
        // The native state stays 0, including the following memcpy_s. A4B
        // overwrites the original request stack slot with this pointer first.
        retry_value = static_cast<std::uint32_t>(address(replacement));
        finish_growth(string, replacement, capacity, preserve_count);
        return;
    } catch (...) {
        // 00408A50..00408A77: ordinary allocation failure retries the request.
        // A throwing memcpy_s handler instead leaves replacement-pointer bits
        // in that same stack slot. Preserve this unusual native continuation.
        capacity = retry_value;
        try {
            fallback_replacement = native_legacy_sbo_string_allocate_00408b60(capacity + 1U);
        } catch (...) {
            // 00408AE0..00408B0F: dispose/reset even the previous string before
            // CxxThrowException(0, 0) rethrows the second exception.
            native_legacy_sbo_string_destroy_004072d0(string);
            throw;
        }
    }
    // 00408A71 returns through the EH dispatcher to 00408A72. Although the
    // stored state remains 2, the nested catch context has ended. A throwing
    // copy handler here propagates without the second-allocation reset.
    finish_growth(string, fallback_replacement, capacity, preserve_count);
}

NativeLegacySboStringStorage& native_legacy_sbo_string_erase_004087f0(
    NativeLegacySboStringStorage& string, std::uint32_t offset, std::uint32_t count) {
    if (string.length_14 < offset) {
        range_error();
    }
    const auto remaining = string.length_14 - offset;
    if (remaining < count) {
        count = remaining;
    }
    if (count != 0) {
        auto* const data = string.data();
        (void)memmove_s(offset_pointer(data, offset), string.capacity_18 - offset,
            offset_pointer(data, offset + count), remaining - count);
        // Reload after a returning CRT handler, matching 00408851..00408865.
        string.length_14 -= count;
        *offset_pointer(string.data(), string.length_14) = '\0';
    }
    return string;
}

NativeLegacySboStringStorage& native_legacy_sbo_string_assign_substring_00408120(
    NativeLegacySboStringStorage& destination,
    const NativeLegacySboStringStorage& source,
    std::uint32_t offset, std::uint32_t count) {
    if (source.length_14 < offset) {
        range_error();
    }
    const auto remaining = source.length_14 - offset;
    if (remaining < count) {
        count = remaining;
    }
    if (&destination == &source) {
        native_legacy_sbo_string_erase_004087f0(destination, offset + count, all_remaining);
        return native_legacy_sbo_string_erase_004087f0(destination, 0, offset);
    }
    if (count > maximum_length) {
        length_error();
    }
    if (destination.capacity_18 < count) {
        native_legacy_sbo_string_grow_004089e0(destination, count, destination.length_14);
    } else if (count == 0) {
        destination.length_14 = 0;
        *destination.data() = '\0';
        return destination;
    }
    if (count != 0) {
        // Both views are deliberately resolved after possible allocation callbacks.
        const auto* const first = offset_pointer(source.data(), offset);
        (void)memcpy_s(destination.data(), destination.capacity_18, first, count);
        destination.length_14 = count;
        *offset_pointer(destination.data(), count) = '\0';
    }
    return destination;
}

NativeLegacySboStringStorage& native_legacy_sbo_string_assign_counted_00408720(
    NativeLegacySboStringStorage& string, const char* source, std::uint32_t count) {
    const auto first = address(string.data());
    const auto source_address = address(source);
    // Native unsigned pointer comparisons precede the maximum-length check.
    // The terminator at data + length is outside this self-alias interval.
    if (source_address >= first && source_address < first + string.length_14) {
        return native_legacy_sbo_string_assign_substring_00408120(string, string,
            static_cast<std::uint32_t>(source_address - first), count);
    }
    if (count > maximum_length) {
        length_error();
    }
    if (string.capacity_18 < count) {
        native_legacy_sbo_string_grow_004089e0(string, count, string.length_14);
    } else if (count == 0) {
        string.length_14 = 0;
        *string.data() = '\0';
        return string;
    }
    if (count != 0) {
        (void)memcpy_s(string.data(), string.capacity_18, source, count);
        string.length_14 = count;
        *offset_pointer(string.data(), count) = '\0';
    }
    return string;
}

} // namespace bsp
