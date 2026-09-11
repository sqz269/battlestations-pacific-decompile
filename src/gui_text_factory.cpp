#include "bsp/gui_text_factory.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error GUI Text pool operations require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 24);
static_assert(sizeof(NativeGuiTextPoolStorage) == 0x38);
static_assert(offsetof(NativeGuiTextPoolStorage, critical_section_0c) == 0x0c);
static_assert(offsetof(NativeGuiTextPoolStorage, recursion_24) == 0x24);
static_assert(offsetof(NativeGuiTextPoolStorage, slabs_28) == 0x28);
static_assert(offsetof(NativeGuiTextPoolStorage, slab_count_2c) == 0x2c);
static_assert(offsetof(NativeGuiTextPoolStorage, table_capacity_30) == 0x30);
static_assert(offsetof(NativeGuiTextPoolStorage, first_free_slab_34) == 0x34);

// Exact-width loads/stores preserve native field reloads and DWORD wrap. These
// do not validate malformed native memory or cache another view of its state.
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
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* base, std::uint32_t displacement) noexcept {
    return pointer(address(base) + displacement);
}
CRITICAL_SECTION* section(NativeGuiTextPoolStorage& storage) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(storage.critical_section_0c);
}
void replace_table(void* pool, std::uint32_t bytes) {
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
    put(pool, 0x28, address(replacement));
}
} // namespace

NativeGuiTextPool::NativeGuiTextPool(AllocatorListDomain& list,
    NativeGuiTextPoolStorage& storage) : allocator_list_(list), storage_(storage) {
    // Bind the actual recovered trim implementation before native list
    // publication can expose this owner to a reentrant allocation handler.
    list.bind_virtual0(storage.allocator_00,
        {native_vtable, native_virtual0, this, &invoke_trim});
}
NativeGuiTextPool::~NativeGuiTextPool() {
    allocator_list_.unbind_virtual0(storage_.allocator_00);
}

void NativeGuiTextPool::initialize_00ab77c0() {
    allocator_list_.prepend_base_element(storage_.allocator_00);
    storage_.allocator_00.native_vtable_00 = native_vtable;
    unsigned unwind_state = 0;
    // DEEC44 has -1/CB7E40,0/CB7E48,1/CB7E53. This C++ interface
    // reproduces the three effects; it is not a native MSVC EH-frame ABI.
    __try {
        InitializeCriticalSection(section(storage_));
        put(&storage_, 0x24, 0);
        unwind_state = 1;
        put(&storage_, 0x28, 0);
        put(&storage_, 0x2c, 0);
        put(&storage_, 0x30, 0);
        put(&storage_, 0x34, 0xffffffffu);
        unwind_state = 2;
        if (word(&storage_, 0x30) < 32u) {
            put(&storage_, 0x30, 32);
            replace_table(&storage_, 0x80);
        }
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_gui_text_pool_table_00ab70c0(at(&storage_, 0x28));
            if (unwind_state >= 1) destroy_critical_section_00402f70();
            allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
        }
    }
}

void* initialize_gui_text_pool_slab_00ab6f60(void* slab,
    std::uint32_t slab_index) noexcept {
    put_half(slab, 0x7e80, 64);
    auto* free_index = at(slab, 0x7e00);
    auto* slot_index = at(slab, 0x1f4);
    for (std::uint32_t index = 0; index < 64; ++index) {
        put_half(free_index, 0, static_cast<std::uint16_t>(63u - index));
        put(slot_index, 0, slab_index);
        free_index = at(free_index, 2);
        slot_index = at(slot_index, 0x1f8);
    }
    return slab;
}

void* NativeGuiTextPool::allocate_raw_slot_00ab78a0() {
    void* const pool = &storage_;
    auto* const critical_section = section(storage_);
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    // No native EH frame: allocation failure leaves the lock/depth and all
    // prior publications intact. There is deliberately no automatic unlock.
    if (word(pool, 0x34) == 0xffffffffu) {
        put(pool, 0x34, word(pool, 0x2c));
        void* const raw = singleton_lifetime_allocate({
            SingletonAllocationKind::object, slab_bytes, slab_bytes});
        void* const slab = raw ? initialize_gui_text_pool_slab_00ab6f60(
            raw, word(pool, 0x34)) : nullptr;
        const auto capacity = word(pool, 0x30);
        if (word(pool, 0x2c) == capacity) {
            const auto grown_capacity = capacity * 2u + 2u;
            const auto bytes = grown_capacity * 4u;
            put(pool, 0x30, grown_capacity);
            replace_table(pool, bytes);
        }
        auto* const destination = at(pointer(word(pool, 0x28)), word(pool, 0x2c) * 4u);
        if (destination) put(destination, 0, address(slab));
        put(pool, 0x2c, word(pool, 0x2c) + 1u);
    }
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), word(pool, 0x34) * 4u));
    put_half(slab, 0x7e80, static_cast<std::uint16_t>(half(slab, 0x7e80) - 1u));
    const auto remaining = half(slab, 0x7e80);
    const auto index = half(slab, 0x7e00u + static_cast<std::uint32_t>(remaining) * 2u);
    void* const slot = at(slab, static_cast<std::uint32_t>(index) * 0x1f8u);
    if (remaining == 0) {
        auto next = word(pool, 0x34) + 1u;
        const bool has_later_slab = next < word(pool, 0x2c);
        put(pool, 0x34, 0xffffffffu);
        if (has_later_slab) {
            auto* cursor = at(pointer(word(pool, 0x28)), next * 4u);
            for (;;) {
                if (half(pointer(word(cursor)), 0x7e80) != 0) {
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

void NativeGuiTextPool::return_raw_slot_00ab75a0(void* slot) {
    void* const pool = &storage_;
    auto* const critical_section = section(storage_);
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    const auto slab_index = word(slot, 0x1f4);
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), slab_index * 4u));
    const auto delta = static_cast<std::int32_t>(address(slot) - address(slab));
    // AB75C8..75D9 signed IMUL+SAR correction is division by1F8 toward zero.
    const auto index = static_cast<std::uint16_t>(delta / 0x1f8);
    const auto free_count = half(slab, 0x7e80);
    put_half(slab, 0x7e00u + static_cast<std::uint32_t>(free_count) * 2u, index);
    put_half(slab, 0x7e80, static_cast<std::uint16_t>(half(slab, 0x7e80) + 1u));
    if (slab_index < word(pool, 0x34)) put(pool, 0x34, slab_index);
    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(critical_section);
}

void NativeGuiTextPool::trim_empty_slabs_00ab7500() {
    void* const pool = &storage_;
    std::uint32_t index = 0;
    while (index < word(pool, 0x2c)) {
        auto* const slab = pointer(word(pointer(word(pool, 0x28)), index * 4u));
        if (half(slab, 0x7e80) != 64) { ++index; continue; }
        singleton_lifetime_free(slab);
        auto* const table = pointer(word(pool, 0x28));
        const auto count = word(pool, 0x2c);
        put(table, index * 4u, word(table, count * 4u - 4u));
        put(pool, 0x2c, word(pool, 0x2c) - 1u);
        if (index < word(pool, 0x2c)) {
            auto* id = at(pointer(word(pointer(word(pool, 0x28)), index * 4u)), 0x1f4);
            for (std::uint32_t slot = 0; slot < 64; ++slot) {
                put(id, 0, index);
                id = at(id, 0x1f8);
            }
        }
        // AB755D decrements, AB7560 increments: retry the replacement index.
    }
    put(pool, 0x34, 0xffffffffu);
    auto* cursor = pointer(word(pool, 0x28));
    for (std::uint32_t candidate = 0; candidate < word(pool, 0x2c); ++candidate) {
        if (half(pointer(word(cursor)), 0x7e80) != 0) {
            put(pool, 0x34, candidate);
            break;
        }
        cursor = at(cursor, 4);
    }
}
void NativeGuiTextPool::invoke_trim(void* context) {
    static_cast<NativeGuiTextPool*>(context)->trim_empty_slabs_00ab7500();
}

void free_gui_text_pool_table_00ab70c0(void* header) noexcept {
    void* const table = pointer(word(header));
    if (table) singleton_lifetime_free(table);
}
void NativeGuiTextPool::destroy_critical_section_00402f70() noexcept {
    while (static_cast<std::int32_t>(word(&storage_, 0x24)) > 0) {
        put(&storage_, 0x24, word(&storage_, 0x24) - 1u);
        LeaveCriticalSection(section(storage_));
    }
    DeleteCriticalSection(section(storage_));
}
void NativeGuiTextPool::destroy_00ab7420() {
    storage_.allocator_00.native_vtable_00 = native_vtable;
    for (std::uint32_t index = 0; index < word(&storage_, 0x2c); ++index)
        singleton_lifetime_free(pointer(word(pointer(word(&storage_, 0x28)), index * 4u)));
    free_gui_text_pool_table_00ab70c0(at(&storage_, 0x28));
    destroy_critical_section_00402f70();
    allocator_list_.unlink_base_element_00403970(storage_.allocator_00);
}

void* allocate_gui_text_raw_slot_00ab79e0(NativeGuiTextPool& pool) {
    return pool.allocate_raw_slot_00ab78a0();
}
void return_gui_text_failed_slot_00ab76f0(void* slot, NativeGuiTextPool& pool) {
    pool.return_raw_slot_00ab75a0(slot);
}
} // namespace bsp
