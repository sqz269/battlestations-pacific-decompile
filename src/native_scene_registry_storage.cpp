#include "bsp/native_scene_registry_storage.hpp"
#include <new>

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + n);
}
Word read(const void* p, Word n = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        reinterpret_cast<Word>(p) + n);
}
void write(void* p, Word n, Word v) noexcept {
    *reinterpret_cast<volatile Word*>(at(p, n)) = v;
}
void* pointer(const void* p, Word n = 0) noexcept {
    return reinterpret_cast<void*>(read(p, n));
}
}

void* allocate_native_scene_registry_pairs_00b81b90(Word count,
    NativeSceneRegistryStorageBindings& bindings) {
    if (count > 0x1fffffffu) throw std::bad_alloc();
    return bindings.actual_00bf681b(count * 8u);
}

void clear_native_scene_registry_vector_00b82c60(void* vector,
    NativeSceneRegistryStorageBindings& bindings) {
    void* const begin = pointer(vector, 4);
    if (begin) bindings.actual_00bf65ac(begin);
    write(vector, 4, 0);
    write(vector, 8, 0);
    write(vector, 0x0c, 0);
}

void construct_native_scene_registry_vector_00b83220(void* vector,
    NativeSceneRegistryVectorArguments& arguments,
    NativeSceneRegistryStorageBindings& bindings) {
    const Word count = arguments.count_00; // B8323C, before vector clears
    write(vector, 4, 0);
    write(vector, 8, 0);
    write(vector, 0x0c, 0);
    if (count == 0) return;
    if (count > 0x1fffffffu) bindings.actual_00b82dd0();
    void* const allocated = allocate_native_scene_registry_pairs_00b81b90(
        count, bindings);
    // Exact one-byte write, preserving a binding's changes to the upper bytes.
    *reinterpret_cast<volatile unsigned char*>(&arguments.count_00) = 0;
    const Word slot4 = arguments.count_00; // B83274
    const Word slot3 = arguments.count_00; // B83277
    const Word end = reinterpret_cast<Word>(allocated) + count * 8u;
    write(vector, 0x0c, end); // B8327F before current source pointer capture
    const void* const pair = arguments.pair_source_04;
    write(vector, 4, reinterpret_cast<Word>(allocated));
    write(vector, 8, reinterpret_cast<Word>(allocated));
    try { // Native state0 covers only fill and the final end publication.
        fill_native_scene_registry_iterators_00b82570(allocated, count, pair,
            reinterpret_cast<Word>(vector), slot3, slot4);
        write(vector, 8, end);
    } catch (...) {
        clear_native_scene_registry_vector_00b82c60(vector, bindings);
        throw;
    }
}

void destroy_native_scene_registry_list_00b829d0(void* list,
    NativeSceneRegistryStorageBindings& bindings) {
    void* head = pointer(list, 4);
    void* node = pointer(head);
    write(head, 0, reinterpret_cast<Word>(head));
    head = pointer(list, 4);
    write(head, 4, reinterpret_cast<Word>(head));
    const bool empty = node == pointer(list, 4); // native CMP before count store
    write(list, 8, 0);
    if (!empty) {
        do {
            void* const next = pointer(node);
            bindings.actual_00bf65ac(node);
            const bool reached_head = next == pointer(list, 4);
            node = next;
            if (reached_head) break;
        } while (true);
    }
    bindings.actual_00bf65ac(pointer(list, 4));
    write(list, 4, 0);
}

void destroy_native_scene_registry_list_00b82b40(void* list,
    NativeSceneRegistryStorageBindings& bindings) {
    destroy_native_scene_registry_list_00b829d0(list, bindings);
}

void destroy_native_scene_registry_00b82e90(void* registry,
    NativeSceneRegistryStorageBindings& bindings) {
    void* const begin = pointer(registry, 0x14);
    if (begin) bindings.actual_00bf65ac(begin);
    write(registry, 0x14, 0);
    write(registry, 0x18, 0);
    write(registry, 0x1c, 0);
    destroy_native_scene_registry_list_00b829d0(at(registry, 4), bindings);
}

void* construct_native_scene_registry_00b83600(void* registry,
    const void* allocator_byte, const void*,
    NativeSceneRegistryConstructScratch& scratch,
    NativeSceneRegistryStorageBindings& bindings) {
    *static_cast<volatile unsigned char*>(registry) =
        *static_cast<const volatile unsigned char*>(allocator_byte);
    void* const list = at(registry, 4);
    void* const head = allocate_native_scene_registry_sentinel_00b82390(
        bindings.actual_00bf681b);
    write(list, 4, reinterpret_cast<Word>(head));
    write(list, 8, 0);
    try {
        const Word captured_head = read(list, 4); // B83642
        scratch.vector_arguments_08.pair_source_04 = scratch.pair_00;
        scratch.vector_arguments_08.count_00 = 9;
        write(scratch.pair_00, 4, captured_head); // B8364B
        write(scratch.pair_00, 0, reinterpret_cast<Word>(list)); // B8364F
        construct_native_scene_registry_vector_00b83220(at(registry, 0x10),
            scratch.vector_arguments_08, bindings);
    } catch (...) {
        // CC2370 reads current actual owner from the native frame. The source
        // interface captures this explicit registry input; no private EBP ABI.
        destroy_native_scene_registry_list_00b82b40(list, bindings);
        throw;
    }
    write(registry, 0x20, 1);
    write(registry, 0x24, 1);
    return registry;
}
} // namespace bsp
