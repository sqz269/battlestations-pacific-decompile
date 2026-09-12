#include "bsp/native_physical_provider_pool_lifecycle.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstdlib>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical provider pool lifecycle requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(AllocatorListElement) == 0x0c);
constexpr std::uint32_t pool_profile = 0x00d68e68;

void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
volatile std::uint16_t& half(void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint16_t*>(at(base, offset));
}
void* volatile& pointer(void* base, std::uint32_t offset) noexcept {
    return *static_cast<void* volatile*>(at(base, offset));
}
CRITICAL_SECTION* section(void* pool) noexcept {
    return static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
}
// Reviewed shared00402F70 operation over the actual section, also inlined in
// BF33A0. Use the real Win32 services, including signed depth and reloads.
void destroy_section(void* pool) noexcept {
    while (static_cast<std::int32_t>(word(pool, 0x24)) > 0) {
        word(pool, 0x24) = word(pool, 0x24) - 1u;
        LeaveCriticalSection(section(pool));
    }
    DeleteCriticalSection(section(pool));
}
void invoke_trim(void* pool) { trim_native_physical_provider_pool_00bf3430(pool); }

void* actual_canonical_pool;
AllocatorListDomain* actual_canonical_list;
} // namespace

void free_native_physical_provider_pool_table_00bf2e30(void* header) noexcept {
    void* const current = pointer(header, 0);
    if (current) singleton_lifetime_free(current);
}

void* initialize_native_physical_provider_pool_00bf3250(void* pool,
    AllocatorListDomain& list) {
    auto& element = *::new (pool) AllocatorListElement;
    list.prepend_base_element(element);
    unsigned unwind_state = 0;
    // E02794/E0277C: state0 base, state1 section, state2 table. The native
    // main body goes directly0->2 after CS and raw-header initialization.
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
            void* destination = replacement;
            for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
                if (destination)
                    pointer(destination, 0) = pointer(pointer(pool, 0x28), index * 4u);
                destination = at(destination, 4);
            }
            free_native_physical_provider_pool_table_00bf2e30(at(pool, 0x28));
            pointer(pool, 0x28) = replacement;
        }
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2)
                free_native_physical_provider_pool_table_00bf2e30(at(pool, 0x28));
            if (unwind_state >= 1) destroy_section(pool);
            list.unlink_base_element_00403970(element);
        }
    }
    return pool;
}

void trim_native_physical_provider_pool_00bf3430(void* pool) noexcept {
    for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
        void* const block = pointer(pointer(pool, 0x28), index * 4u);
        if (half(block, 0x1f0) != 8u) continue;
        singleton_lifetime_free(block);
        // BF3456..BF348C is a returning-free continuation, not an exit.
        void* const table = pointer(pool, 0x28);
        const auto last = word(pool, 0x2c) - 1u;
        pointer(table, index * 4u) = pointer(table, last * 4u);
        word(pool, 0x2c) = word(pool, 0x2c) - 1u;
        if (index < word(pool, 0x2c)) {
            void* slot_id = at(pointer(pointer(pool, 0x28), index * 4u), 0x38);
            for (std::uint32_t slot = 0; slot < 8u; ++slot) {
                word(slot_id, 0) = index;
                slot_id = at(slot_id, 0x3c);
            }
        }
        --index; // wrapping subtraction plus loop increment retries index0 too
    }
    const bool has_blocks = word(pool, 0x2c) != 0;
    word(pool, 0x34) = 0xffffffffu;
    if (has_blocks) {
        void* cursor = pointer(pool, 0x28); // captured once for this final scan
        std::uint32_t index = 0;
        do {
            if (half(pointer(cursor, 0), 0x1f0) != 0) {
                word(pool, 0x34) = index;
                break;
            }
            ++index;
            cursor = at(cursor, 4);
        } while (index < word(pool, 0x2c));
    }
}

void destroy_native_physical_provider_pool_00bf33a0(void* pool,
    AllocatorListDomain& list) noexcept {
    const bool has_blocks = word(pool, 0x2c) != 0;
    word(pool, 0) = pool_profile; // native count comparison precedes this store
    if (has_blocks) {
        std::uint32_t index = 0;
        do {
            singleton_lifetime_free(pointer(pointer(pool, 0x28), index * 4u));
            ++index;
        } while (index < word(pool, 0x2c));
    }
    free_native_physical_provider_pool_table_00bf2e30(at(pool, 0x28));
    destroy_section(pool);
    list.unlink_base_element_00403970(*static_cast<AllocatorListElement*>(pool));
}

void bind_native_physical_provider_pool_trim(void* pool, AllocatorListDomain& list) {
    list.bind_virtual0(*static_cast<AllocatorListElement*>(pool),
        {pool_profile, 0x00bf3430, pool, &invoke_trim});
}

void bind_static_native_physical_provider_pool_0109dbf0(void* pool,
    AllocatorListDomain& list) {
    bind_native_physical_provider_pool_trim(pool, list);
    actual_canonical_pool = pool;
    actual_canonical_list = &list;
}

int initialize_static_native_physical_provider_pool_00cd9010() {
    initialize_native_physical_provider_pool_00bf3250(actual_canonical_pool,
        *actual_canonical_list);
    return std::atexit(&destroy_static_native_physical_provider_pool_00ce10f0);
}

void destroy_static_native_physical_provider_pool_00ce10f0() noexcept {
    destroy_native_physical_provider_pool_00bf33a0(actual_canonical_pool,
        *actual_canonical_list);
}
} // namespace bsp
