#include "bsp/native_resource_registry_node_leaves.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw resource registry node leaves require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
// Actual DWORD loads/stores without inventing a C++ aggregate or pointer-object
// lifetime over caller scratch. These also accept existing typed iterator cells.
Word read(const void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, base
        add eax, byte_offset
        mov ecx, [eax]
        mov result, ecx
    }
    return result;
}
void write(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        add eax, byte_offset
        mov ecx, value
        mov [eax], ecx
    }
}
bool nil(Word node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(node + 0x19u) != 0;
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
}

Word compare_native_resource_registry_iterators_00b19530(
    const void* left, const void* right, const SingletonLifetimeCallbacks& callbacks) {
    const Word owner = read(left);
    if (owner == 0 || owner != read(right)) invalid(callbacks); // B19542
    const Word node = read(left, 4);
    const bool equal = node == read(right, 4);
    return (node & 0xffffff00u) | static_cast<Word>(equal);
}

void decrement_native_resource_registry_iterator_00b1ab40(
    void* iterator, const SingletonLifetimeCallbacks& callbacks) {
    if (read(iterator) == 0) invalid(callbacks); // B1AB48
    const Word node = read(iterator, 4);
    if (nil(node)) {
        const Word selected = read(pointer(node), 8);
        write(iterator, 4, selected);
        if (nil(selected)) invalid(callbacks); // B1AB63 returning tail
        return;
    }
    Word selected = read(pointer(node));
    if (!nil(selected)) {
        Word next = read(pointer(selected), 8);
        while (!nil(next)) {
            selected = next;
            next = read(pointer(selected), 8);
        }
        write(iterator, 4, selected);
        return;
    }
    selected = read(pointer(node), 4);
    while (!nil(selected)) {
        const Word current = read(iterator, 4);
        if (current != read(pointer(selected))) break;
        write(iterator, 4, selected);
        selected = read(pointer(selected), 4);
    }
    if (nil(read(iterator, 4))) {
        invalid(callbacks); // B1ABBF returning tail, no later publication
        return;
    }
    write(iterator, 4, selected);
}

void* construct_native_resource_registry_node_00b1aca0(void* node,
    const NativeResourceRegistryNodeArguments& arguments, NativeStringRawPoolContext& strings) {
    const Word left = arguments.left;
    const Word right = arguments.right;
    const Word pair = arguments.pair;
    const Word parent = arguments.parent;
    void* const header = pointer(address(node) + 0x0cu);
    const bool same = address(header) == pair;
    write(node, 0, left);
    write(node, 4, parent);
    write(node, 8, right);
    write(header, 0, 0);
    write(header, 4, 0);
    if (!same) {
        const Word requested = read(pointer(pair));
        resize_native_string_header_0041dd40(header, strings, requested, true); // B1ACD9
        if (read(pointer(pair)) != 0) {
            const Word count = read(header);
            const Word source = read(pointer(pair), 4);
            const Word destination = read(header, 4);
            if (count != 0) std::memmove(pointer(destination), pointer(source), count); // B1ACEF
        }
    }
    const Word value = read(pointer(pair), 8);
    const auto color = *reinterpret_cast<const volatile unsigned char*>(&arguments.color);
    write(header, 8, value);
    auto* const bytes = static_cast<volatile unsigned char*>(node);
    bytes[0x18] = color;
    bytes[0x19] = 0;
    return node;
}

void* allocate_native_resource_registry_node_00b1ad10(
    const NativeResourceRegistryNodeAllocationFrame& frame, NativeStringRawPoolContext& strings,
    NativeResourceRegistryNodeAcquired& acquired) {
    if (acquired.started) throw std::logic_error("resource registry node acquisition is not fresh");
    acquired.started = true;
    void* allocation;
    try {
        allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c}); // B1AD33
    } catch (...) {
        acquired.source_failed = true;
        throw;
    }
    acquired.captured_allocation = allocation;
    acquired.allocation_completed = true;
    frame.allocation_cleanup = address(allocation);
    acquired.native_eh_state = 0;
    frame.placement_cleanup = address(allocation);
    acquired.native_eh_state = 1;
    try {
        if (allocation) {
            const Word color = frame.incoming.color;
            const Word pair = frame.incoming.pair;
            const Word right = frame.incoming.right;
            frame.constructor.color = color;
            const Word parent = frame.incoming.parent;
            frame.constructor.pair = pair;
            const Word left = frame.incoming.left;
            frame.constructor.right = right;
            frame.constructor.parent = parent;
            frame.constructor.left = left;
            construct_native_resource_registry_node_00b1aca0(allocation, frame.constructor, strings); // B1AD68
            acquired.constructor_completed = true;
        }
    } catch (...) {
        acquired.source_failed = true;
        // CBC6D0: current EBP-14 then EBP-18, passed to actual RET-only401130.
        // The two reads survive; no string/node destructor is hidden here.
        const Word placement_argument = frame.allocation_cleanup;
        const Word placement_destination = frame.placement_cleanup;
        (void)placement_argument;
        (void)placement_destination;
        acquired.native_eh_state = 0;
        acquired.placement_cleanup_completed = true;
        acquired.native_eh_state = 2; // catch-all entered after state1 unwind
        void* const current_allocation = pointer(frame.allocation_cleanup);
        acquired.catch_freed_address = current_allocation;
        singleton_lifetime_free(current_allocation); // B1AD86
        acquired.catch_free_completed = true;
        throw; // B1AD92 BF6885(0,0), source transport only
    }
    return allocation;
}
} // namespace bsp
