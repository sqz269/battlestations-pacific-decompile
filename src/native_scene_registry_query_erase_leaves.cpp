#include "bsp/native_scene_registry_query_erase_leaves.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene registry leaves require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& cell(Word address, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(address + offset);
}
Word identity(void* value) noexcept { return reinterpret_cast<Word>(value); }
void require_valid(bool condition) {
    if (!condition) throw std::invalid_argument("raw scene registry outside valid iterator domain");
}
std::int32_t signed_bits(Word value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
Word sar3(Word value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}
void check_boundary(Word registry, Word captured_begin, Word bucket) {
    require_valid(captured_begin != 0 && bucket < sar3(cell(registry, 0x18) - captured_begin));
}
Word bucket_for(Word registry, Word key) noexcept {
    // C03DBE: signed CDQ/IDIV returns quotient EAX and remainder EDX. With the
    // reached positive divisor its negative/positive-remainder repair cannot
    // be reached. Unsigned products/subtraction preserve native DWORD wrap.
    const auto dividend = signed_bits(key ^ 0xdeadbeefu);
    const Word quotient = static_cast<Word>(dividend / 0x1f31d);
    const Word remainder = static_cast<Word>(dividend % 0x1f31d);
    Word hash = remainder * 0x41a7u - quotient * 0xb14u;
    if (signed_bits(hash) < 0) hash += 0x7fffffffu;
    const Word mask = cell(registry, 0x20);
    Word bucket = mask & hash;
    if (cell(registry, 0x24) <= bucket) bucket += 0xffffffffu - (mask >> 1);
    return bucket;
}
}

void* equal_range_native_scene_registry_00b82650(void* actual,
    NativeSceneRegistryEqualRangeArguments& arguments,
    NativeSceneRegistryEqualRangeScratch& scratch) {
    const Word initial_key = arguments.key; // B82650
    const Word registry = identity(actual);
    Word bucket = bucket_for(registry, cell(initial_key));
    check_boundary(registry, cell(registry, 0x14), bucket);
    Word node = cell(cell(registry, 0x14), bucket * 8u + 4u);
    const Word owner = registry + 4u;
    require_valid(owner != 0);
    ++bucket;
    for (;;) {
        check_boundary(registry, cell(registry, 0x14), bucket);
        if (cell(cell(registry, 0x14), bucket * 8u + 4u) == node) break;
        require_valid(owner != 0);
        require_valid(node != cell(owner, 4));
        const Word node_key = cell(node, 8); // B82708 precedes current key cell
        const Word key_address = arguments.key;
        if (node_key >= cell(key_address)) {
            scratch.first_08 = node; // B82721; no initialization on other paths
            for (;;) {
                check_boundary(registry, cell(registry, 0x14), bucket);
                if (cell(cell(registry, 0x14), bucket * 8u + 4u) == node) break;
                require_valid(node != cell(owner, 4));
                const Word current_key_address = arguments.key;
                const Word current_key = cell(current_key_address); // B82754
                if (current_key < cell(node, 8)) break;
                require_valid(node != cell(owner, 4));
                node = cell(node);
            }
            require_valid(owner != 0);
            const Word first = scratch.first_08;
            if (first != node) {
                const Word output = arguments.output;
                cell(output, 0x0c) = node;
                cell(output) = owner;
                cell(output, 8) = owner;
                cell(output, 4) = first;
                return reinterpret_cast<void*>(output);
            }
            break;
        }
        require_valid(node != cell(owner, 4));
        node = cell(node);
    }
    const Word output = arguments.output; // B82799 before current sentinel
    const Word sentinel = cell(owner, 4);
    cell(output, 4) = sentinel;
    cell(output) = owner;
    cell(output, 8) = owner;
    cell(output, 0x0c) = sentinel;
    return reinterpret_cast<void*>(output);
}

void* erase_native_scene_registry_entry_00b827c0(void* actual,
    NativeSceneRegistryEraseArguments& arguments, NativeSceneRegistryStorageBindings& bindings) {
    const Word initial_owner = arguments.owner;
    const Word registry = identity(actual);
    require_valid(initial_owner != 0);
    Word node = arguments.node;
    require_valid(node != cell(initial_owner, 4));
    Word bucket = bucket_for(registry, cell(node, 8));
    for (;;) {
        check_boundary(registry, cell(registry, 0x14), bucket);
        const Word captured_begin = cell(registry, 0x14);
        if (cell(captured_begin, bucket * 8u + 4u) != node) break;
        check_boundary(registry, captured_begin, bucket);
        const Word boundary_node = cell(cell(registry, 0x14), bucket * 8u + 4u);
        const Word registry_owner = registry + 4u;
        require_valid(registry_owner != 0);
        require_valid(boundary_node != cell(registry_owner, 4));
        const Word before_next_begin = cell(registry, 0x14); // B8287C
        const Word next_boundary = cell(boundary_node); // B82881
        check_boundary(registry, before_next_begin, bucket);
        const Word current_begin = cell(registry, 0x14);
        cell(current_begin, bucket * 8u) = registry_owner;
        node = arguments.node; // B8289F: AFTER boundary owner, before node store
        cell(current_begin, bucket * 8u + 4u) = next_boundary;
        if (bucket == 0) break;
        --bucket;
    }
    const Word owner = arguments.owner; // B828B1; stays captured across free
    const Word owner_head = cell(owner, 4);
    arguments.node = node; // B828B8 even before native invalid-parameter branch
    require_valid(node != owner_head);
    const Word registry_head = cell(registry, 8);
    const Word next = cell(node); // B828C6 follows head comparison
    if (node != registry_head) {
        const Word previous = cell(node, 4);
        cell(previous) = next;
        const Word current_next = cell(node);
        const Word current_previous = cell(node, 4);
        cell(current_next, 4) = current_previous;
        const auto actual_free = bindings.actual_00bf65ac;
        if (!actual_free) throw std::logic_error("missing actual BF65AC free binding");
        actual_free(reinterpret_cast<void*>(node)); // B828D8
        cell(registry, 0x0c) = cell(registry, 0x0c) - 1u; // corrected B828E0
    }
    const Word output = arguments.output; // B828E4 AFTER free/current count
    cell(output, 4) = next;
    cell(output) = owner;
    return reinterpret_cast<void*>(output);
}

void count_native_scene_registry_range_00b820b0(NativeSceneRegistryCountArguments& arguments) {
    Word node = arguments.first_node;
    const Word last = arguments.last_node;
    const Word owner = arguments.first_owner;
    const Word counter = arguments.count;
    for (;;) {
        require_valid(owner != 0 && owner == arguments.last_owner);
        if (node == last) return;
        cell(counter) = cell(counter) + 1u;
        require_valid(owner != 0);
        require_valid(node != cell(owner, 4));
        node = cell(node);
    }
}
} // namespace bsp
