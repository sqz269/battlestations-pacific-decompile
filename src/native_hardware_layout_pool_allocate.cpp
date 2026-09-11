#include "bsp/native_hardware_layout_pool_allocate.hpp"
#include "bsp/native_hardware_layout_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware layout pool allocation requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);

std::uint32_t word(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::uint16_t half(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint16_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ax, word ptr [eax + edx]
        mov result, ax
    }
    return result;
}
void put_half(void* base, std::uint32_t byte_offset, std::uint16_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov cx, value
        mov word ptr [eax + edx], cx
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}
void* at(const void* base, std::uint32_t offset) noexcept {
    return pointer(address(base) + offset);
}
} // namespace

void* initialize_native_hardware_layout_slab_00b5ff50(
    void* slab, std::uint32_t slab_index) noexcept {
    put_half(slab, 0x940, 32);
    auto* free_index = at(slab, 0x900);
    auto* slot_index = at(slab, 0x44);
    for (std::uint32_t index = 0; index < 32; ++index) {
        put_half(free_index, 0, static_cast<std::uint16_t>(31u - index));
        put(slot_index, 0, slab_index);
        free_index = at(free_index, 2);
        slot_index = at(slot_index, 0x48);
    }
    return slab;
}

void* allocate_native_hardware_layout_slot_00b605b0(void* pool) {
    auto* const critical_section = reinterpret_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);

    // The native body has no EH frame. A real allocation/new-handler exception
    // leaves the lock, recursion count, and preceding publications in place.
    if (word(pool, 0x34) == 0xffffffffu) {
        put(pool, 0x34, word(pool, 0x2c));
        void* const raw = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x944, 0x944});
        void* const slab = raw ? initialize_native_hardware_layout_slab_00b5ff50(
            raw, word(pool, 0x34)) : nullptr;

        const auto capacity = word(pool, 0x30);
        if (word(pool, 0x2c) == capacity) {
            const auto grown_capacity = capacity * 2u + 2u;
            const auto bytes = grown_capacity * 4u;
            put(pool, 0x30, grown_capacity);
            void* const replacement = singleton_lifetime_allocate({
                SingletonAllocationKind::pointer_slots, bytes, bytes});
            auto* destination = replacement;
            for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
                if (destination) {
                    const auto old_table = word(pool, 0x28);
                    put(destination, 0, word(pointer(old_table), index * 4u));
                }
                destination = at(destination, 4);
            }
            void* const old_table = pointer(word(pool, 0x28));
            if (old_table) singleton_lifetime_free(old_table);
            // B60646 is ADD ESP,4 after free, followed by this publication.
            put(pool, 0x28, address(replacement));
        }

        const auto index = word(pool, 0x2c);
        auto* const destination = at(pointer(word(pool, 0x28)), index * 4u);
        if (destination) put(destination, 0, address(slab));
        // Reload after the store: a caller-supplied table can alias this word.
        put(pool, 0x2c, word(pool, 0x2c) + 1u);
    }

    const auto slab_index = word(pool, 0x34);
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), slab_index * 4u));
    put_half(slab, 0x940, static_cast<std::uint16_t>(half(slab, 0x940) - 1u));
    const auto remaining = half(slab, 0x940);
    const auto index = half(slab, 0x900u + static_cast<std::uint32_t>(remaining) * 2u);
    void* const slot = at(slab, static_cast<std::uint32_t>(index) * 0x48u);

    if (remaining == 0) {
        auto next = word(pool, 0x34) + 1u;
        const bool has_later_slab = next < word(pool, 0x2c);
        put(pool, 0x34, 0xffffffffu);
        if (has_later_slab) {
            auto* cursor = at(pointer(word(pool, 0x28)), next * 4u);
            for (;;) {
                if (half(pointer(word(cursor)), 0x940) != 0) {
                    put(pool, 0x34, next);
                    break;
                }
                ++next;
                cursor = at(cursor, 4);
                if (next >= word(pool, 0x2c)) break;
            }
        }
    }

    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(critical_section);
    return slot;
}

void* allocate_native_hardware_layout_00b606f0(void* actual_pool_0108fe9c) {
    return allocate_native_hardware_layout_slot_00b605b0(actual_pool_0108fe9c);
}

void return_native_hardware_layout_00b60260(void* raw_slot, void* actual_pool_0108fe9c) {
    return_native_hardware_layout_slot_00b60110(actual_pool_0108fe9c, raw_slot);
}
} // namespace bsp
