#include "bsp/native_locale_pool_table.hpp"
#include "bsp/native_locale_node_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale pool tables require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word word(Word base, Word byte_offset = 0u) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put_word(Word base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void increment_count(Word base) noexcept {
    __asm {
        mov eax, base
        add dword ptr [eax + 4], 1
    }
}
} // namespace

// bsp.py show A9EA60/A9EE40 and A9F0B0: the latter inlines the same
// growth/append instructions on its actual +28h triple. These are the sole
// Source bodies for the two helper addresses, replacing detached GUI-table
// descriptors. Native instruction ordering and wrapping DWORD addresses are
// retained, including allocator/free callback changes to the borrowed triple.
__declspec(noinline) void grow_native_string_map_pool_table_00a9ea60(
    NativeLocalePoolSlabTable table, NativeLocalePoolTableTrace trace) {
    const Word actual = address(table.storage);
    const Word capacity = word(actual, 8u) * 2u + 2u;
    const Word bytes = capacity * 4u;
    put_word(actual, 8u, capacity);
    if (trace.progress) {
        trace.progress->capacity_published = true;
        trace.progress->table_allocation_bytes = bytes;
        trace.progress->table_allocation_entered = true;
    }
    void* const replacement = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
    if (trace.progress) {
        trace.progress->table_allocation = replacement;
        trace.progress->table_allocation_returned = true;
        if (trace.require_entered) trace.require_entered(trace.context);
    }
    Word destination = address(replacement);
    for (Word i = 0u; i < word(actual, 4u); ++i, destination += 4u) {
        if (destination != 0u)
            put_word(destination, 0u, word(word(actual), i * 4u));
    }
    const Word previous = word(actual);
    if (trace.progress) trace.progress->old_table = pointer(previous);
    if (previous != 0u) {
        if (trace.progress) trace.progress->old_table_free_entered = true;
        singleton_lifetime_free(pointer(previous));
        if (trace.progress) {
            trace.progress->old_table_free_returned = true;
            if (trace.require_entered) trace.require_entered(trace.context);
        }
    }
    put_word(actual, 0u, address(replacement));
    if (trace.progress) trace.progress->table_published = true;
}

__declspec(noinline) void push_native_string_map_pool_slab_00a9ee40(
    NativeLocalePoolSlabTable table, const void* pointer_cell,
    NativeLocalePoolTableTrace trace) {
    const Word actual = address(table.storage);
    if (word(actual, 4u) == word(actual, 8u))
        grow_native_string_map_pool_table_00a9ea60(table, trace);
    const Word destination = word(actual) + word(actual, 4u) * 4u;
    if (destination != 0u) {
        put_word(destination, 0u, word(address(pointer_cell)));
        if (trace.progress) trace.progress->slab_store_performed = true;
    }
    increment_count(actual);
    if (trace.progress) trace.progress->slab_count_incremented = true;
}

} // namespace bsp
