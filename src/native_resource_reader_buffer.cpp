#include "bsp/native_resource_reader_buffer.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource reader buffers require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::uint32_t bits(const void* value) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
std::int32_t signed_word(const void* base, std::uint32_t byte_offset) noexcept {
    return static_cast<std::int32_t>(word(base, byte_offset));
}

void copy_entry(void* destination, const void* source, NativeStringRawPoolContext& pool) {
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (destination != source) {
        resize_native_string_header_0041dd40(destination, pool, word(source), true);
        if (word(source) != 0) {
            const auto size = word(destination);
            const auto source_data = word(source, 4);
            const auto destination_data = word(destination, 4);
            // BF7680 supports overlapping ranges. Omit its zero-byte call,
            // following the existing actual-header string helper's policy.
            if (size != 0) std::memmove(pointer(destination_data), pointer(source_data), size);
        }
    }
    put(destination, 8, word(source, 8));
}
} // namespace

void reserve_native_resource_reader_buffer_00bf05d0(void* header,
    std::int32_t requested_capacity, NativeStringRawPoolContext& pool) {
    if (requested_capacity < 1) requested_capacity = 1;
    if (signed_word(header, 8) >= requested_capacity) return;
    const auto requested = static_cast<std::uint32_t>(requested_capacity);
    const auto byte_count = requested * 0x0cu;
    auto* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, byte_count, byte_count});
    for (std::uint32_t index = 0;
         static_cast<std::int32_t>(index) < signed_word(header, 4); ++index) {
        const auto displacement = index * 0x0cu;
        auto* const destination = pointer(bits(replacement) + displacement);
        if (destination) {
            auto* const source = pointer(word(header) + displacement);
            // CC7790's sole state0 action is placement delete401130, a RET.
            // No owner guard or compensating cleanup belongs around this copy.
            copy_entry(destination, source, pool);
        }
    }
    std::uint32_t displacement = 0;
    for (std::uint32_t index = 0;
         static_cast<std::int32_t>(index) < signed_word(header, 4); ++index) {
        auto* const entry = pointer(word(header) + displacement);
        destroy_native_string_header_0041dd20(entry, pool);
        displacement += 0x0cu;
    }
    singleton_lifetime_free(pointer(word(header)));
    put(header, 0, bits(replacement));
    put(header, 8, requested);
}

void resize_native_resource_reader_buffer_00bf0700(void* header,
    std::int32_t requested_count, NativeStringRawPoolContext& pool) {
    if (requested_count > signed_word(header, 8))
        reserve_native_resource_reader_buffer_00bf05d0(header, requested_count, pool);
    const auto requested = static_cast<std::uint32_t>(requested_count);
    const auto old_count = word(header, 4);
    if (static_cast<std::int32_t>(old_count) < requested_count) {
        auto displacement = old_count * 0x0cu;
        auto remaining = requested - old_count;
        do {
            auto* const entry = pointer(word(header) + displacement);
            if (entry) {
                put(entry, 0, 0);
                put(entry, 4, 0);
            }
            displacement += 0x0cu;
        } while (--remaining != 0);
    }
    while (requested_count < signed_word(header, 4)) {
        put(header, 4, word(header, 4) - 1u);
        const auto current_count = word(header, 4);
        auto* const entry = pointer(word(header) + current_count * 0x0cu);
        destroy_native_string_header_0041dd20(entry, pool);
    }
    put(header, 4, requested);
}

} // namespace bsp
