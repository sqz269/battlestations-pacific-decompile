#include "bsp/native_physical_provider_pool.hpp"

#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical provider pools require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
volatile std::uint16_t& half(void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint16_t*>(at(base, offset));
}
void* pointer(void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
} // namespace

void* initialize_native_physical_provider_block_00bf2d50(void* block,
    std::uint32_t block_index) noexcept {
    half(block, 0x1f0) = 8;
    for (std::uint32_t slot = 0; slot != 8; ++slot) {
        half(block, 0x1e0 + slot * 2u) = static_cast<std::uint16_t>(7u - slot);
        word(block, 0x38 + slot * 0x3cu) = block_index;
    }
    return block;
}

void* acquire_native_physical_provider_slot_00bf34d0(
    NativePhysicalProviderPoolContext& context) {
    void* const pool = context.actual_initialized_pool_0109dbf0;
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    if (word(pool, 0x34) == 0xffffffffu) {
        word(pool, 0x34) = word(pool, 0x2c);
        void* block = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x1f4, 0x1f4});
        if (block) {
            const auto current_index = word(pool, 0x34);
            block = initialize_native_physical_provider_block_00bf2d50(block, current_index);
        }
        const auto capacity = word(pool, 0x30);
        if (word(pool, 0x2c) == capacity) {
            const auto new_capacity = capacity + capacity + 2u;
            const auto bytes = new_capacity * 4u;
            word(pool, 0x30) = new_capacity;
            void* const replacement = singleton_lifetime_allocate({
                SingletonAllocationKind::object, bytes, bytes});
            std::uint32_t index = 0;
            void* destination = replacement;
            while (index < word(pool, 0x2c)) {
                if (destination) {
                    void* const current_data = pointer(pool, 0x28);
                    word(destination) = word(current_data, index * 4u);
                }
                ++index;
                destination = at(destination, 4);
            }
            void* const old_data = pointer(pool, 0x28);
            if (old_data) singleton_lifetime_free(old_data);
            // BF3566 ADD ESP,4 precedes this publication; the old saved
            // no-return override incorrectly hides that continuation.
            word(pool, 0x28) = reinterpret_cast<std::uint32_t>(replacement);
        }
        const auto count = word(pool, 0x2c);
        void* const current_data = pointer(pool, 0x28);
        void* const destination = at(current_data, count * 4u);
        if (destination) word(destination) = reinterpret_cast<std::uint32_t>(block);
        word(pool, 0x2c) = word(pool, 0x2c) + 1u;
    }
    const auto block_index = word(pool, 0x34);
    void* const data = pointer(pool, 0x28);
    void* const block = pointer(data, block_index * 4u);
    half(block, 0x1f0) = static_cast<std::uint16_t>(half(block, 0x1f0) - 1u);
    const auto free_count = half(block, 0x1f0);
    const auto slot_index = half(block, 0x1e0 + static_cast<std::uint32_t>(free_count) * 2u);
    void* const slot = at(block, static_cast<std::uint32_t>(slot_index) * 0x3cu);
    if (free_count == 0) {
        auto index = word(pool, 0x34) + 1u;
        const bool has_remaining = index < word(pool, 0x2c);
        word(pool, 0x34) = 0xffffffffu;
        if (has_remaining) {
            void* entry = at(pointer(pool, 0x28), index * 4u);
            for (;;) {
                if (half(pointer(entry), 0x1f0) != 0) {
                    word(pool, 0x34) = index;
                    break;
                }
                ++index;
                entry = at(entry, 4);
                if (index >= word(pool, 0x2c)) break;
            }
        }
    }
    word(section, 0x18) = word(section, 0x18) - 1u;
    LeaveCriticalSection(section);
    return slot;
}

void return_native_physical_provider_slot_00bf2fc0(
    NativePhysicalProviderPoolContext& context, void* slot) {
    void* const pool = context.actual_initialized_pool_0109dbf0;
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    const auto block_index = word(slot, 0x38);
    void* const block = pointer(pointer(pool, 0x28), block_index * 4u);
    const auto difference = reinterpret_cast<std::uint32_t>(slot)
        - reinterpret_cast<std::uint32_t>(block);
    const auto slot_index = static_cast<std::int32_t>(difference) / 0x3c;
    const auto free_count = half(block, 0x1f0);
    half(block, 0x1e0 + static_cast<std::uint32_t>(free_count) * 2u) =
        static_cast<std::uint16_t>(slot_index);
    half(block, 0x1f0) = static_cast<std::uint16_t>(half(block, 0x1f0) + 1u);
    if (block_index < word(pool, 0x34)) word(pool, 0x34) = block_index;
    word(section, 0x18) = word(section, 0x18) - 1u;
    LeaveCriticalSection(section);
}

void return_native_physical_provider_slot_00bf3200(void* slot,
    NativePhysicalProviderPoolContext& context) {
    return_native_physical_provider_slot_00bf2fc0(context, slot);
}

} // namespace bsp
