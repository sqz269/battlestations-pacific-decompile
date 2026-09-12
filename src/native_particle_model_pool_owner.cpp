#include "bsp/native_particle_model_pool_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstdlib>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-model pool lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(AllocatorListElement) == 0x0c);
constexpr std::uint32_t pool_profile = 0x00d5da38;

void* at(void* base, std::uint32_t bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + bytes);
}
volatile std::uint32_t& word(void* base, std::uint32_t bytes) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, bytes));
}
volatile std::uint16_t& half(void* base, std::uint32_t bytes) noexcept {
    return *static_cast<volatile std::uint16_t*>(at(base, bytes));
}
void* volatile& pointer(void* base, std::uint32_t bytes) noexcept {
    return *static_cast<void* volatile*>(at(base, bytes));
}
CRITICAL_SECTION* section(void* pool) noexcept {
    return static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
}
void destroy_section(void* pool) noexcept {
    while (static_cast<std::int32_t>(word(pool, 0x24)) > 0) {
        word(pool, 0x24) = word(pool, 0x24) - 1u;
        LeaveCriticalSection(section(pool));
    }
    DeleteCriticalSection(section(pool));
}
void invoke_trim(void* actual_pool) { trim_native_particle_model_pool_00af6940(actual_pool); }

void* actual_canonical_pool;
AllocatorListDomain* actual_canonical_list;
} // namespace

void free_native_particle_model_pool_table_00af5d00(void* header) noexcept {
    auto* const current = pointer(header, 0);
    if (current) singleton_lifetime_free(current);
}

void* initialize_native_particle_model_pool_00af6860(void* pool, AllocatorListDomain& list) {
    auto& element = *::new (pool) AllocatorListElement;
    list.prepend_base_element(element);
    unsigned unwind_state = 0;
    // DF2BDC/DF2BC4: state0 base links, state1 section, state2 table.
    // Main body goes directly from0 to2 after section/metadata initialization.
    __try {
        word(pool, 0) = pool_profile;
        ::new (section(pool)) CRITICAL_SECTION;
        InitializeCriticalSection(section(pool));
        word(pool, 0x24) = 0;
        pointer(pool, 0x28) = nullptr;
        word(pool, 0x2c) = 0;
        word(pool, 0x30) = 0;
        word(pool, 0x34) = 0xffffffffu;
        unwind_state = 2;
        if (word(pool, 0x30) < 32u) {
            word(pool, 0x30) = 32;
            void* const replacement = singleton_lifetime_allocate(
                {SingletonAllocationKind::pointer_slots, 0x80, 0x80});
            auto* destination = replacement;
            for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
                if (destination)
                    pointer(destination, 0) = pointer(pointer(pool, 0x28), index * 4u);
                destination = at(destination, 4);
            }
            free_native_particle_model_pool_table_00af5d00(at(pool, 0x28));
            pointer(pool, 0x28) = replacement;
        }
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) free_native_particle_model_pool_table_00af5d00(at(pool, 0x28));
            if (unwind_state >= 1) destroy_section(pool);
            list.unlink_base_element_00403970(element);
        }
    }
    return pool;
}

void destroy_native_particle_model_pool_00af5ff0(void* pool, AllocatorListDomain& list) noexcept {
    const bool has_slabs = word(pool, 0x2c) != 0;
    word(pool, 0) = pool_profile; // native captures comparison before this store
    if (has_slabs) {
        std::uint32_t index = 0;
        do {
            singleton_lifetime_free(pointer(pointer(pool, 0x28), index * 4u));
            ++index;
        } while (index < word(pool, 0x2c));
    }
    free_native_particle_model_pool_table_00af5d00(at(pool, 0x28));
    destroy_section(pool);
    list.unlink_base_element_00403970(*static_cast<AllocatorListElement*>(pool));
}

void trim_native_particle_model_pool_00af6940(void* pool) noexcept {
    for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
        auto* const slab = pointer(pointer(pool, 0x28), index * 4u);
        if (half(slab, 0x5c40) != 32u) continue;
        singleton_lifetime_free(slab);
        // Reload table/count after free; move last pointer before decrementing.
        auto* const table = pointer(pool, 0x28);
        const auto last = word(pool, 0x2c) - 1u;
        pointer(table, index * 4u) = pointer(table, last * 4u);
        word(pool, 0x2c) = word(pool, 0x2c) - 1u;
        if (index < word(pool, 0x2c)) {
            auto* slot_id = at(pointer(pointer(pool, 0x28), index * 4u), 0x2dc);
            for (std::uint32_t slot = 0; slot < 32u; ++slot) {
                word(slot_id, 0) = index;
                slot_id = at(slot_id, 0x2e0);
            }
        }
        --index; // paired with loop increment: retry the moved slab, including0
    }
    const bool has_slabs = word(pool, 0x2c) != 0;
    word(pool, 0x34) = 0xffffffffu;
    if (has_slabs) {
        auto* cursor = pointer(pool, 0x28); // captured once for the final scan
        std::uint32_t index = 0;
        do {
            if (half(pointer(cursor, 0), 0x5c40) != 0) {
                word(pool, 0x34) = index;
                break;
            }
            ++index;
            cursor = at(cursor, 4);
        } while (index < word(pool, 0x2c));
    }
}

void bind_static_native_particle_model_pool_00f8d2d0(void* pool, AllocatorListDomain& list) {
    list.bind_virtual0(*static_cast<AllocatorListElement*>(pool),
        {pool_profile, 0x00af6940, pool, &invoke_trim});
    actual_canonical_pool = pool;
    actual_canonical_list = &list;
}

int initialize_static_native_particle_model_pool_00cd7830() {
    initialize_native_particle_model_pool_00af6860(actual_canonical_pool, *actual_canonical_list);
    return std::atexit(&destroy_static_native_particle_model_pool_00ce0b90);
}

void destroy_static_native_particle_model_pool_00ce0b90() noexcept {
    destroy_native_particle_model_pool_00af5ff0(actual_canonical_pool, *actual_canonical_list);
}

} // namespace bsp
