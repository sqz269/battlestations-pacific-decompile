#include "bsp/native_cube_texture_pool_allocate.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cube texture pool allocation requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);

std::uint32_t word(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
std::uint16_t half(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint16_t value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ax, word ptr [eax + edx] }
    __asm { mov value, ax }
    return value;
}
void put_half(void* base, std::uint32_t byte_offset, std::uint16_t value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov cx, value }
    __asm { mov word ptr [eax + edx], cx }
}
std::uint32_t address(const void* pointer) noexcept {
    return reinterpret_cast<std::uintptr_t>(pointer);
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(const void* base, std::uint32_t offset) noexcept {
    return pointer(address(base) + offset);
}
} // namespace

void* initialize_native_cube_texture_slab_00b3d2a0(
    void* slab, std::uint32_t slab_index) noexcept {
    put_half(slab, 0x6c0, 32);
    auto* free_index = at(slab, 0x680);
    auto* slot_index = at(slab, 0x30);
    for (std::uint32_t index = 0; index < 32; ++index) {
        put_half(free_index, 0, static_cast<std::uint16_t>(31u - index));
        put(slot_index, 0, slab_index);
        free_index = at(free_index, 2);
        slot_index = at(slot_index, 0x34);
    }
    return slab;
}

void* allocate_native_cube_texture_slot_00b3f170(void* pool) {
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    put(pool, 0x24, word(pool, 0x24) + 1u);

    // Native has no EH frame: allocation failure retains the actual lock,
    // current depth, and every publication preceding the throwing call.
    if (word(pool, 0x34) == 0xffffffffu) {
        put(pool, 0x34, word(pool, 0x2c));
        void* const raw = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x6c4, 0x6c4});
        void* const slab = raw ? initialize_native_cube_texture_slab_00b3d2a0(
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
                if (destination)
                    put(destination, 0, word(pointer(word(pool, 0x28)), index * 4u));
                destination = at(destination, 4);
            }
            void* const old_table = pointer(word(pool, 0x28));
            if (old_table) singleton_lifetime_free(old_table);
            // Full B3F206 ADD ESP,4 returns from free before B3F209 publishes.
            put(pool, 0x28, address(replacement));
        }

        const auto insertion_index = word(pool, 0x2c);
        auto* const destination = at(pointer(word(pool, 0x28)), insertion_index * 4u);
        if (destination) put(destination, 0, address(slab));
        put(pool, 0x2c, word(pool, 0x2c) + 1u);
    }

    auto* const table = pointer(word(pool, 0x28));
    const auto selected_index = word(pool, 0x34);
    auto* const slab = pointer(word(table, selected_index * 4u));
    put_half(slab, 0x6c0, static_cast<std::uint16_t>(half(slab, 0x6c0) - 1u));
    const auto remaining = half(slab, 0x6c0);
    const auto index = half(slab, 0x680u + static_cast<std::uint32_t>(remaining) * 2u);
    void* const slot = at(slab, static_cast<std::uint32_t>(index) * 0x34u);

    if (remaining == 0) {
        auto next = word(pool, 0x34) + 1u;
        const bool has_later_slab = next < word(pool, 0x2c);
        put(pool, 0x34, 0xffffffffu);
        if (has_later_slab) {
            auto* cursor = at(pointer(word(pool, 0x28)), next * 4u);
            for (;;) {
                if (half(pointer(word(cursor)), 0x6c0) != 0) {
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
    LeaveCriticalSection(section);
    return slot;
}

void return_native_cube_texture_slot_00b3d940(void* pool, void* slot) {
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    const auto slab_index = word(slot, 0x30);
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), slab_index * 4u));
    const auto displacement_bits = address(slot) - address(slab);
    std::int32_t displacement;
    std::memcpy(&displacement, &displacement_bits, sizeof(displacement));
    const auto slot_index = static_cast<std::uint16_t>(displacement / 0x34);
    const auto old_count = half(slab, 0x6c0);
    put_half(slab, 0x680u + static_cast<std::uint32_t>(old_count) * 2u, slot_index);
    put_half(slab, 0x6c0, static_cast<std::uint16_t>(half(slab, 0x6c0) + 1u));
    if (slab_index < word(pool, 0x34)) put(pool, 0x34, slab_index);
    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(section);
}

void* allocate_native_cube_texture_00b3f2c0(void* actual_pool_0108db70) {
    return allocate_native_cube_texture_slot_00b3f170(actual_pool_0108db70);
}

void return_native_cube_texture_00b3dce0(void* slot, void* actual_pool_0108db70) {
    return_native_cube_texture_slot_00b3d940(actual_pool_0108db70, slot);
}
} // namespace bsp
