#include "bsp/native_hardware_layout_pool_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware-layout pool lifetime requires MSVC Win32.
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <new>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(AllocatorListElement) == 0x0c);

void* volatile& pointer(void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& word(void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::int32_t& recursion(void* pool) noexcept {
    return *reinterpret_cast<volatile std::int32_t*>(static_cast<unsigned char*>(pool) + 0x24);
}
CRITICAL_SECTION* section(void* pool) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(static_cast<unsigned char*>(pool) + 0x0c);
}
void destroy_section_00402f70(void* pool) noexcept {
    while (recursion(pool) > 0) {
        recursion(pool) = recursion(pool) - 1;
        LeaveCriticalSection(section(pool));
    }
    DeleteCriticalSection(section(pool));
}

} // namespace

void free_native_hardware_layout_pool_table_00b60020(void* header) noexcept {
    auto* const current = pointer(header, 0);
    if (current) singleton_lifetime_free(current);
}

void* initialize_native_hardware_layout_pool_00b604d0(
    void* pool, AllocatorListDomain& list) {
    auto& node = *static_cast<AllocatorListElement*>(pool);
    list.prepend_base_element(node);
    unsigned unwind_state = 0;
    // Native FH3 map DFA040 runs table -> section -> base. A Win32 finally
    // preserves native cleanup on actual unwind without a catch/rethrow edge.
    __try {
        word(pool, 0) = 0x00d62af0;
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
            auto* const replacement = singleton_lifetime_allocate(
                {SingletonAllocationKind::pointer_slots, 0x80, 0x80});
            for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
                auto* const destination = static_cast<unsigned char*>(replacement) + index * 4u;
                if (destination) pointer(destination, 0) = pointer(pointer(pool, 0x28), index * 4u);
            }
            free_native_hardware_layout_pool_table_00b60020(
                static_cast<unsigned char*>(pool) + 0x28);
            pointer(pool, 0x28) = replacement;
        }
    } __finally {
        if (AbnormalTermination()) {
            if (unwind_state >= 2) {
                free_native_hardware_layout_pool_table_00b60020(
                    static_cast<unsigned char*>(pool) + 0x28);
                destroy_section_00402f70(pool);
            }
            list.unlink_base_element_00403970(node);
        }
    }
    return pool;
}

void destroy_native_hardware_layout_pool_00b60270(
    void* pool, AllocatorListDomain& list) noexcept {
    std::uint32_t index = 0;
    const bool has_slabs = word(pool, 0x2c) != 0;
    word(pool, 0) = 0x00d62af0;
    if (has_slabs) {
        do {
            singleton_lifetime_free(pointer(pointer(pool, 0x28), index * 4u));
            ++index;
        } while (index < word(pool, 0x2c));
    }
    free_native_hardware_layout_pool_table_00b60020(
        static_cast<unsigned char*>(pool) + 0x28);
    destroy_section_00402f70(pool);
    list.unlink_base_element_00403970(*static_cast<AllocatorListElement*>(pool));
}

} // namespace bsp
