#include "bsp/native_enum_node_pool.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum node pool requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
// Actual DWORD loads/stores preserve native reloads and address arithmetic.
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
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(const void* base, std::uint32_t offset) noexcept {
    return pointer(address(base) + offset);
}

CRITICAL_SECTION* section(void* pool) noexcept {
    return static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
}
void invoke_trim(void* pool) { trim_native_enum_node_pool_00410cd0(pool); }
} // namespace

void bind_native_enum_node_pool_virtual0_00ce37a4(void* pool, AllocatorListDomain& list) {
    list.bind_virtual0(*static_cast<AllocatorListElement*>(pool),
        {native_enum_node_pool_profile, 0x00410cd0, pool, &invoke_trim});
}

void* initialize_native_enum_node_pool_00411050(void* pool, AllocatorListDomain& list) {
    auto& element = *::new (pool) AllocatorListElement;
    list.prepend_base_element(element);
    put(pool, 0, native_enum_node_pool_profile);
    ::new (section(pool)) CRITICAL_SECTION;
    InitializeCriticalSection(section(pool));
    put(pool, 0x24, 0);
    put(pool, 0x28, 0);
    put(pool, 0x2c, 0);
    put(pool, 0x30, 0);
    put(pool, 0x34, 0xffffffffu);
    // Complete ordinary path. Original FH3 failure cleanup remains external.
    if (word(pool, 0x30) < 32u) {
        put(pool, 0x30, 32);
        void* const replacement = singleton_lifetime_allocate(
            {SingletonAllocationKind::pointer_slots, 0x80, 0x80});
        auto* destination = replacement;
        for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
            if (destination)
                put(destination, 0, word(pointer(word(pool, 0x28)), index * 4u));
            destination = at(destination, 4);
        }
        void* const old_table = pointer(word(pool, 0x28));
        if (old_table) singleton_lifetime_free(old_table);
        put(pool, 0x28, address(replacement));
    }
    return pool;
}

void* initialize_native_enum_node_page_004e6370(void* page, std::uint32_t page_index) noexcept {
    put_half(page, 0x580, 64);
    auto* free_index = at(page, 0x500);
    auto* slot_index = at(page, 0x10);
    for (std::uint32_t index = 0; index < 64; ++index) {
        put_half(free_index, 0, static_cast<std::uint16_t>(63u - index));
        put(slot_index, 0, page_index);
        free_index = at(free_index, 2);
        slot_index = at(slot_index, 0x14);
    }
    return page;
}

void* allocate_native_enum_node_004e7c00(void* pool) {
    auto* const critical_section = reinterpret_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);

    // No added rollback/guard. Allocation failure and exceptional cleanup are
    // outside the admitted successful domain.
    if (word(pool, 0x34) == 0xffffffffu) {
        put(pool, 0x34, word(pool, 0x2c));
        void* const raw = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x584, 0x584});
        void* const slab = raw ? initialize_native_enum_node_page_004e6370(
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
            // 4E7C96 is ADD ESP,4 after returning CRT free, not a function exit.
            put(pool, 0x28, address(replacement));
        }

        const auto index = word(pool, 0x2c);
        auto* const destination = at(pointer(word(pool, 0x28)), index * 4u);
        if (destination) put(destination, 0, address(slab));
        put(pool, 0x2c, word(pool, 0x2c) + 1u);
    }

    const auto slab_index = word(pool, 0x34);
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), slab_index * 4u));
    put_half(slab, 0x580, static_cast<std::uint16_t>(half(slab, 0x580) - 1u));
    const auto remaining = half(slab, 0x580);
    const auto index = half(slab, 0x500u + static_cast<std::uint32_t>(remaining) * 2u);
    void* const slot = at(slab, static_cast<std::uint32_t>(index) * 0x14u);
    if (remaining == 0) {
        auto next = word(pool, 0x34) + 1u;
        const bool has_later_slab = next < word(pool, 0x2c);
        put(pool, 0x34, 0xffffffffu);
        if (has_later_slab) {
            auto* cursor = at(pointer(word(pool, 0x28)), next * 4u);
            for (;;) {
                if (half(pointer(word(cursor)), 0x580) != 0) {
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


void return_native_enum_node_0043b0a0(void* pool, void* slot) {
    auto* const critical_section = section(pool);
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    const auto page_index = word(slot, 0x10);
    auto* const page = pointer(word(pointer(word(pool, 0x28)), page_index * 4u));
    const auto offset_bits = address(slot) - address(page);
    std::int32_t signed_offset;
    std::memcpy(&signed_offset, &offset_bits, sizeof signed_offset);
    // Native signed IMUL/SAR/SHR implements division toward zero by 20.
    const auto slot_index = static_cast<std::uint16_t>(signed_offset / 20);
    put_half(page, 0x500u + static_cast<std::uint32_t>(half(page, 0x580)) * 2u, slot_index);
    put_half(page, 0x580, static_cast<std::uint16_t>(half(page, 0x580) + 1u));
    if (page_index < word(pool, 0x34)) put(pool, 0x34, page_index);
    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(critical_section);
}

void trim_native_enum_node_pool_00410cd0(void* pool) noexcept {
    for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
        auto* const page = pointer(word(pointer(word(pool, 0x28)), index * 4u));
        if (half(page, 0x580) != 64u) continue;
        singleton_lifetime_free(page);
        auto* const table = pointer(word(pool, 0x28));
        const auto last = word(pool, 0x2c) - 1u;
        put(table, index * 4u, word(table, last * 4u));
        put(pool, 0x2c, word(pool, 0x2c) - 1u);
        if (index < word(pool, 0x2c)) {
            auto* slot_id = at(pointer(word(pointer(word(pool, 0x28)), index * 4u)), 0x10);
            for (std::uint32_t slot = 0; slot < 64; ++slot) {
                put(slot_id, 0, index);
                slot_id = at(slot_id, 0x14);
            }
        }
        --index; // retry the moved page, including a hole at zero
    }
    const bool has_pages = word(pool, 0x2c) != 0;
    put(pool, 0x34, 0xffffffffu);
    if (has_pages) {
        auto* cursor = pointer(word(pool, 0x28));
        std::uint32_t index = 0;
        do {
            if (half(pointer(word(cursor)), 0x580) != 0) {
                put(pool, 0x34, index);
                break;
            }
            ++index;
            cursor = at(cursor, 4);
        } while (index < word(pool, 0x2c));
    }
}

void destroy_native_enum_node_pool_00410a60(void* pool, AllocatorListDomain& list) noexcept {
    const bool has_pages = word(pool, 0x2c) != 0;
    put(pool, 0, native_enum_node_pool_profile);
    if (has_pages) {
        std::uint32_t index = 0;
        do {
            singleton_lifetime_free(pointer(word(pointer(word(pool, 0x28)), index * 4u)));
            ++index;
        } while (index < word(pool, 0x2c));
    }
    void* const table = pointer(word(pool, 0x28));
    if (table) singleton_lifetime_free(table);
    while (static_cast<std::int32_t>(word(pool, 0x24)) > 0) {
        put(pool, 0x24, word(pool, 0x24) - 1u);
        LeaveCriticalSection(section(pool));
    }
    DeleteCriticalSection(section(pool));
    list.unlink_base_element_00403970(*static_cast<AllocatorListElement*>(pool));
}
} // namespace bsp
