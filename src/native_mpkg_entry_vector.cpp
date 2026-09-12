#include "bsp/native_mpkg_entry_vector.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
template<class T> T read(const void* base, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
template<class T> void put(void* base, std::uint32_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(reinterpret_cast<std::uintptr_t>(base) + offset) = value;
}
std::int32_t signed_bits(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
void* entry_at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
void release_name(void* entry, NativeStringStorage& strings) noexcept {
    auto* const data = read<char*>(entry, 4);
    if (data) {
        const auto bytes = read<std::uint32_t>(entry) + 1u;
        strings.release(data, bytes); // Captured pointer/size precede419CC0/BD1510.
    }
}
} // namespace

void* copy_native_mpkg_entry_00bb9330(void* destination, const void* source,
    NativeStringStorage& strings) {
    put<std::uint32_t>(destination, 0, 0);
    put<std::uint32_t>(destination, 4, 0);
    if (destination != source) {
        resize_native_string_header_0041dd40(destination, strings,
            read<std::uint32_t>(source), true);
        if (read<std::uint32_t>(source) != 0) {
            const auto bytes = read<std::uint32_t>(destination);
            auto* const from = read<const void*>(source, 4);
            auto* const to = read<void*>(destination, 4);
            if (bytes) std::memmove(to, from, bytes); // BF7680 supports overlap.
        }
    }
    put(destination, 8, read<std::uint32_t>(source, 8));
    put(destination, 0xc, read<std::uint8_t>(source, 0xc));
    put(destination, 0x10, read<std::uint32_t>(source, 0x10));
    put(destination, 0x14, read<std::uint16_t>(source, 0x14));
    put(destination, 0x18, read<std::uint32_t>(source, 0x18));
    put(destination, 0x1c, read<std::uint32_t>(source, 0x1c));
    put(destination, 0x20, read<std::uint32_t>(source, 0x20));
    return destination;
}

void reserve_native_mpkg_entries_00bb93a0(void* vector, std::int32_t requested,
    NativeStringStorage& strings) {
    volatile auto& current_request = requested;
    if (current_request < 1) current_request = 1;
    if (signed_bits(read<std::uint32_t>(vector, 8)) >= current_request) return;
    const auto bytes = static_cast<std::uint32_t>(current_request) * 0x24u;
    void* const replacement = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    std::uint32_t index = 0;
    while (signed_bits(index) < signed_bits(read<std::uint32_t>(vector, 4))) {
        const auto offset = index * 0x24u;
        auto* const destination = entry_at(replacement, offset);
        if (destination) {
            auto* const source = entry_at(read<void*>(vector), offset);
            copy_native_mpkg_entry_00bb9330(destination, source, strings);
        }
        // State0 CC47C0 only computes captured-local arguments to RET401130.
        // No new-array free or completed-prefix cleanup is added on failure.
        ++index;
    }
    index = 0;
    std::uint32_t offset = 0;
    while (signed_bits(index) < signed_bits(read<std::uint32_t>(vector, 4))) {
        release_name(entry_at(read<void*>(vector), offset), strings);
        ++index;
        offset += 0x24u;
    }
    singleton_lifetime_free(read<void*>(vector));
    const auto capacity = static_cast<std::uint32_t>(current_request); // BB9474, AFTER free.
    put(vector, 0, replacement);
    put(vector, 8, capacity);
}

void resize_native_mpkg_entries_00bb94a0(void* vector, std::int32_t requested,
    NativeStringStorage& strings) {
    if (requested > signed_bits(read<std::uint32_t>(vector, 8)))
        reserve_native_mpkg_entries_00bb93a0(vector, requested, strings);
    const auto count = read<std::uint32_t>(vector, 4);
    if (signed_bits(count) < requested) {
        auto offset = count * 0x24u;
        auto remaining = static_cast<std::uint32_t>(requested) - count;
        do {
            auto* const entry = entry_at(read<void*>(vector), offset);
            if (entry) {
                put<std::uint32_t>(entry, 0, 0);
                put<std::uint32_t>(entry, 4, 0);
            }
            offset += 0x24u;
            --remaining;
        } while (remaining != 0);
    }
    while (requested < signed_bits(read<std::uint32_t>(vector, 4))) {
        put(vector, 4, read<std::uint32_t>(vector, 4) - 1u);
        const auto offset = read<std::uint32_t>(vector, 4) * 0x24u;
        release_name(entry_at(read<void*>(vector), offset), strings);
    }
    put(vector, 4, static_cast<std::uint32_t>(requested));
}

void append_native_mpkg_entry_00bb9520(void* vector, const void* entry,
    NativeStringStorage& strings) {
    const auto capacity = read<std::uint32_t>(vector, 8);
    if (read<std::uint32_t>(vector, 4) == capacity) {
        auto doubled = capacity + capacity;
        if (signed_bits(doubled) <= 1) doubled = 1;
        reserve_native_mpkg_entries_00bb93a0(vector, signed_bits(doubled), strings);
    }
    const auto offset = read<std::uint32_t>(vector, 4) * 0x24u;
    auto* const destination = entry_at(read<void*>(vector), offset);
    try {
        if (destination) copy_native_mpkg_entry_00bb9330(destination, entry, strings);
    } catch (...) {
        // CC47F0 reloads current count then current base before RET401130.
        const auto current_offset = read<std::uint32_t>(vector, 4) * 0x24u;
        (void)entry_at(read<void*>(vector), current_offset);
        throw;
    }
    put(vector, 4, read<std::uint32_t>(vector, 4) + 1u);
}
void destroy_native_mpkg_entries_00bb9900(void* vector, NativeStringStorage& strings) {
    resize_native_mpkg_entries_00bb94a0(vector, 0, strings);
    singleton_lifetime_free(read<void*>(vector));
}
} // namespace bsp
