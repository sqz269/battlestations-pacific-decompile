#include "bsp/native_tracer_pool.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstdlib>
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tracer pool lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(AllocatorListElement) == 0x0c);
constexpr std::uint32_t pool_profile = 0x00d63f90;

void* owner_at(void* base, std::uint32_t bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + bytes);
}
volatile std::uint32_t& owner_word(void* base, std::uint32_t bytes) noexcept {
    return *static_cast<volatile std::uint32_t*>(owner_at(base, bytes));
}
volatile std::uint16_t& owner_half(void* base, std::uint32_t bytes) noexcept {
    return *static_cast<volatile std::uint16_t*>(owner_at(base, bytes));
}
void* volatile& owner_pointer(void* base, std::uint32_t bytes) noexcept {
    return *static_cast<void* volatile*>(owner_at(base, bytes));
}
CRITICAL_SECTION* section(void* pool) noexcept {
    return static_cast<CRITICAL_SECTION*>(owner_at(pool, 0x0c));
}
void destroy_section(void* pool) noexcept {
    while (static_cast<std::int32_t>(owner_word(pool, 0x24)) > 0) {
        owner_word(pool, 0x24) = owner_word(pool, 0x24) - 1u;
        LeaveCriticalSection(section(pool));
    }
    DeleteCriticalSection(section(pool));
}
void invoke_trim(void* actual_pool) { trim_native_tracer_pool_00babed0(actual_pool); }

void* actual_canonical_pool;
AllocatorListDomain* actual_canonical_list;
} // namespace

void free_native_tracer_pool_table_00ba9c80(void* header) noexcept {
    auto* const current = owner_pointer(header, 0);
    if (current) singleton_lifetime_free(current);
}

void* initialize_native_tracer_pool_00bac3c0(void* pool, AllocatorListDomain& list) {
    auto& element = *::new (pool) AllocatorListElement;
    list.prepend_base_element(element);
    unsigned unwind_state = 0;
    // DFD530/DFD518: state0 base links, state1 section, state2 table.
    // Main body goes directly from0 to2 after section/metadata initialization.
    __try {
        owner_word(pool, 0) = pool_profile;
        ::new (section(pool)) CRITICAL_SECTION;
        InitializeCriticalSection(section(pool));
        owner_word(pool, 0x24) = 0;
        owner_pointer(pool, 0x28) = nullptr;
        owner_word(pool, 0x2c) = 0;
        owner_word(pool, 0x30) = 0;
        owner_word(pool, 0x34) = 0xffffffffu;
        unwind_state = 2;
        if (owner_word(pool, 0x30) < 32u) {
            owner_word(pool, 0x30) = 32;
            void* const replacement = singleton_lifetime_allocate(
                {SingletonAllocationKind::pointer_slots, 0x80, 0x80});
            auto* destination = replacement;
            for (std::uint32_t index = 0; index < owner_word(pool, 0x2c); ++index) {
                if (destination)
                    owner_pointer(destination, 0) = owner_pointer(owner_pointer(pool, 0x28), index * 4u);
                destination = owner_at(destination, 4);
            }
            free_native_tracer_pool_table_00ba9c80(owner_at(pool, 0x28));
            owner_pointer(pool, 0x28) = replacement;
        }
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_native_tracer_pool_table_00ba9c80(owner_at(pool, 0x28));
            if (unwind_state >= 1) destroy_section(pool);
            list.unlink_base_element_00403970(element);
        }
    }
    return pool;
}

void destroy_native_tracer_pool_00babdd0(void* pool, AllocatorListDomain& list) noexcept {
    const bool has_slabs = owner_word(pool, 0x2c) != 0;
    owner_word(pool, 0) = pool_profile; // native captures comparison before this store
    if (has_slabs) {
        std::uint32_t index = 0;
        do {
            singleton_lifetime_free(owner_pointer(owner_pointer(pool, 0x28), index * 4u));
            ++index;
        } while (index < owner_word(pool, 0x2c));
    }
    free_native_tracer_pool_table_00ba9c80(owner_at(pool, 0x28));
    destroy_section(pool);
    list.unlink_base_element_00403970(*static_cast<AllocatorListElement*>(pool));
}

void trim_native_tracer_pool_00babed0(void* pool) noexcept {
    for (std::uint32_t index = 0; index < owner_word(pool, 0x2c); ++index) {
        auto* const slab = owner_pointer(owner_pointer(pool, 0x28), index * 4u);
        if (owner_half(slab, 0x3d90) != 8u) continue;
        singleton_lifetime_free(slab);
        // Reload table/count after free; move last pointer before decrementing.
        auto* const table = owner_pointer(pool, 0x28);
        const auto last = owner_word(pool, 0x2c) - 1u;
        owner_pointer(table, index * 4u) = owner_pointer(table, last * 4u);
        owner_word(pool, 0x2c) = owner_word(pool, 0x2c) - 1u;
        if (index < owner_word(pool, 0x2c)) {
            auto* slot_id = owner_at(owner_pointer(owner_pointer(pool, 0x28), index * 4u), 0x7ac);
            for (std::uint32_t slot = 0; slot < 8u; ++slot) {
                owner_word(slot_id, 0) = index;
                slot_id = owner_at(slot_id, 0x7b0);
            }
        }
        --index; // paired with loop increment: retry the moved slab, including0
    }
    const bool has_slabs = owner_word(pool, 0x2c) != 0;
    owner_word(pool, 0x34) = 0xffffffffu;
    if (has_slabs) {
        auto* cursor = owner_pointer(pool, 0x28); // captured once for the final scan
        std::uint32_t index = 0;
        do {
            if (owner_half(owner_pointer(cursor, 0), 0x3d90) != 0) {
                owner_word(pool, 0x34) = index;
                break;
            }
            ++index;
            cursor = owner_at(cursor, 4);
        } while (index < owner_word(pool, 0x2c));
    }
}

void bind_static_native_tracer_pool_0109049c(void* pool, AllocatorListDomain& list) {
    list.bind_virtual0(*static_cast<AllocatorListElement*>(pool),
        {pool_profile, 0x00babed0, pool, &invoke_trim});
    actual_canonical_pool = pool;
    actual_canonical_list = &list;
}

int initialize_static_native_tracer_pool_00cd88d0() {
    initialize_native_tracer_pool_00bac3c0(actual_canonical_pool, *actual_canonical_list);
    return std::atexit(&destroy_static_native_tracer_pool_00ce0f90);
}

void destroy_static_native_tracer_pool_00ce0f90() noexcept {
    destroy_native_tracer_pool_00babdd0(actual_canonical_pool, *actual_canonical_list);
}

} // namespace bsp

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);

std::uint32_t raw_word(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void raw_put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
std::uint16_t raw_half(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint16_t value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ax, word ptr [eax + edx] }
    __asm { mov value, ax }
    return value;
}
void raw_put_half(void* base, std::uint32_t byte_offset, std::uint16_t value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov cx, value }
    __asm { mov word ptr [eax + edx], cx }
}
std::uint32_t raw_address(const void* raw_pointer) noexcept {
    return reinterpret_cast<std::uintptr_t>(raw_pointer);
}
void* raw_pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
void* raw_at(const void* base, std::uint32_t offset) noexcept {
    return raw_pointer(raw_address(base) + offset);
}
} // namespace

void* initialize_native_tracer_slab_00ba9b50(
    void* slab, std::uint32_t slab_index) noexcept {
    raw_put_half(slab, 0x3d90, 8);
    auto* free_index = raw_at(slab, 0x3d80);
    auto* slot_index = raw_at(slab, 0x7ac);
    for (std::uint32_t index = 0; index < 8; ++index) {
        raw_put_half(free_index, 0, static_cast<std::uint16_t>(7u - index));
        raw_put(slot_index, 0, slab_index);
        free_index = raw_at(free_index, 2);
        slot_index = raw_at(slot_index, 0x7b0);
    }
    return slab;
}

void* allocate_native_tracer_slot_00bac4a0(void* pool) {
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(raw_at(pool, 0x0c));
    EnterCriticalSection(section);
    raw_put(pool, 0x24, raw_word(pool, 0x24) + 1u);

    // Native has no EH frame: allocation failure retains the actual lock,
    // current depth, and every publication preceding the throwing call.
    if (raw_word(pool, 0x34) == 0xffffffffu) {
        raw_put(pool, 0x34, raw_word(pool, 0x2c));
        void* const raw = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x3d94, 0x3d94});
        void* const slab = raw ? initialize_native_tracer_slab_00ba9b50(
            raw, raw_word(pool, 0x34)) : nullptr;

        const auto capacity = raw_word(pool, 0x30);
        if (raw_word(pool, 0x2c) == capacity) {
            const auto grown_capacity = capacity * 2u + 2u;
            const auto bytes = grown_capacity * 4u;
            raw_put(pool, 0x30, grown_capacity);
            void* const replacement = singleton_lifetime_allocate({
                SingletonAllocationKind::pointer_slots, bytes, bytes});
            auto* destination = replacement;
            for (std::uint32_t index = 0; index < raw_word(pool, 0x2c); ++index) {
                if (destination)
                    raw_put(destination, 0, raw_word(raw_pointer(raw_word(pool, 0x28)), index * 4u));
                destination = raw_at(destination, 4);
            }
            void* const old_table = raw_pointer(raw_word(pool, 0x28));
            if (old_table) singleton_lifetime_free(old_table);
            // BAC536 ADD ESP,4 follows returning free; BAC539 publishes.
            // The saved Ghidra flow omits those three bytes and falsely exits.
            raw_put(pool, 0x28, raw_address(replacement));
        }

        const auto insertion_index = raw_word(pool, 0x2c);
        auto* const destination = raw_at(raw_pointer(raw_word(pool, 0x28)), insertion_index * 4u);
        if (destination) raw_put(destination, 0, raw_address(slab));
        raw_put(pool, 0x2c, raw_word(pool, 0x2c) + 1u);
    }

    auto* const table = raw_pointer(raw_word(pool, 0x28));
    const auto selected_index = raw_word(pool, 0x34);
    auto* const slab = raw_pointer(raw_word(table, selected_index * 4u));
    raw_put_half(slab, 0x3d90, static_cast<std::uint16_t>(raw_half(slab, 0x3d90) - 1u));
    const auto remaining = raw_half(slab, 0x3d90);
    const auto index = raw_half(slab, 0x3d80u + static_cast<std::uint32_t>(remaining) * 2u);
    void* const slot = raw_at(slab, static_cast<std::uint32_t>(index) * 0x7b0u);

    if (remaining == 0) {
        auto next = raw_word(pool, 0x34) + 1u;
        const bool has_later_slab = next < raw_word(pool, 0x2c);
        raw_put(pool, 0x34, 0xffffffffu);
        if (has_later_slab) {
            auto* cursor = raw_at(raw_pointer(raw_word(pool, 0x28)), next * 4u);
            for (;;) {
                if (raw_half(raw_pointer(raw_word(cursor)), 0x3d90) != 0) {
                    raw_put(pool, 0x34, next);
                    break;
                }
                ++next;
                cursor = raw_at(cursor, 4);
                if (next >= raw_word(pool, 0x2c)) break;
            }
        }
    }

    raw_put(pool, 0x24, raw_word(pool, 0x24) - 1u);
    LeaveCriticalSection(section);
    return slot;
}

void return_native_tracer_slot_00babf70(void* pool, void* slot) {
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(raw_at(pool, 0x0c));
    EnterCriticalSection(section);
    raw_put(pool, 0x24, raw_word(pool, 0x24) + 1u);
    const auto slab_index = raw_word(slot, 0x7ac);
    auto* const slab = raw_pointer(raw_word(raw_pointer(raw_word(pool, 0x28)), slab_index * 4u));
    const auto displacement_bits = raw_address(slot) - raw_address(slab);
    std::int32_t displacement;
    std::memcpy(&displacement, &displacement_bits, sizeof(displacement));
    const auto slot_index = static_cast<std::uint16_t>(displacement / 0x7b0);
    const auto old_count = raw_half(slab, 0x3d90);
    raw_put_half(slab, 0x3d80u + static_cast<std::uint32_t>(old_count) * 2u, slot_index);
    raw_put_half(slab, 0x3d90, static_cast<std::uint16_t>(raw_half(slab, 0x3d90) + 1u));
    if (slab_index < raw_word(pool, 0x34)) raw_put(pool, 0x34, slab_index);
    raw_put(pool, 0x24, raw_word(pool, 0x24) - 1u);
    LeaveCriticalSection(section);
}

void* allocate_native_tracer_00bac660(void* actual_pool_0109049c) {
    return allocate_native_tracer_slot_00bac4a0(actual_pool_0109049c);
}

void return_native_tracer_00bac2b0(void* slot, void* actual_pool_0109049c) {
    return_native_tracer_slot_00babf70(actual_pool_0109049c, slot);
}
} // namespace bsp
