#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {

// The native MSVC string in these six routines, not the pooled NativeString.
// This is raw storage: destruction is explicit and the leading word is untouched.
struct NativeLegacySboStringStorage {
    std::uint32_t preserved_00;
    union Buffer {
        char inline_bytes[16];
        char* heap;
    } buffer_04;
    std::uint32_t length_14;
    std::uint32_t capacity_18;

    char* data() noexcept {
        return capacity_18 < 16 ? buffer_04.inline_bytes : buffer_04.heap;
    }
    const char* data() const noexcept {
        return capacity_18 < 16 ? buffer_04.inline_bytes : buffer_04.heap;
    }
};

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeLegacySboStringStorage) == 0x1C);
static_assert(std::is_trivially_default_constructible_v<NativeLegacySboStringStorage>);
static_assert(offsetof(NativeLegacySboStringStorage, buffer_04) == 0x04);
static_assert(offsetof(NativeLegacySboStringStorage, length_14) == 0x14);
static_assert(offsetof(NativeLegacySboStringStorage, capacity_18) == 0x18);

// New C++ interfaces: the original routines use ECX and callee-popped arguments.
NativeLegacySboStringStorage& native_legacy_sbo_string_assign_counted_00408720(
    NativeLegacySboStringStorage&, const char* source, std::uint32_t count);
NativeLegacySboStringStorage& native_legacy_sbo_string_assign_substring_00408120(
    NativeLegacySboStringStorage&, const NativeLegacySboStringStorage& source,
    std::uint32_t offset, std::uint32_t count);
void native_legacy_sbo_string_destroy_004072d0(NativeLegacySboStringStorage&) noexcept;
// preserve_count is the second native stack argument, usually the old length.
void native_legacy_sbo_string_grow_004089e0(NativeLegacySboStringStorage&,
    std::uint32_t requested_capacity, std::uint32_t preserve_count);
char* native_legacy_sbo_string_allocate_00408b60(std::uint32_t bytes);
NativeLegacySboStringStorage& native_legacy_sbo_string_erase_004087f0(
    NativeLegacySboStringStorage&, std::uint32_t offset, std::uint32_t count);

} // namespace bsp
