#include "bsp/native_resource_pointer_array.hpp"

#include "bsp/singleton_lifetime.hpp"
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::int32_t) == 4);
using Word = std::uint32_t;

Word address(const void* pointer) noexcept {
    return reinterpret_cast<Word>(pointer);
}
volatile Word& word(void* header, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(address(header) + offset);
}
std::int32_t signed_bits(Word bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
std::int32_t signed_word(void* header, Word offset) noexcept {
    return signed_bits(word(header, offset));
}

void reserve_pointers(void* header, std::int32_t requested) {
    if (requested < 16) requested = 16;
    if (signed_word(header, 8) >= requested) return;
    const Word bytes = static_cast<Word>(requested) * 4u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    Word index = 0;
    Word destination = address(replacement);
    while (signed_bits(index) < signed_word(header, 4)) {
        if (destination != 0) {
            const Word source = word(header) + index * 4u;
            const Word value = word(reinterpret_cast<void*>(source));
            word(reinterpret_cast<void*>(destination)) = value;
        }
        index += 1u;
        destination += 4u;
    }
    singleton_lifetime_free(reinterpret_cast<void*>(word(header)));
    word(header) = address(replacement);
    word(header, 8) = static_cast<Word>(requested);
}

void resize_pointers(void* header, std::int32_t requested,
    void (*reserve)(void*, std::int32_t) = reserve_pointers) {
    if (requested > signed_word(header, 8)) reserve(header, requested);
    Word index = word(header, 4);
    while (signed_bits(index) < requested) {
        const Word destination = word(header) + index * 4u;
        if (destination != 0) word(reinterpret_cast<void*>(destination)) = 0;
        index += 1u;
    }
    while (requested < signed_word(header, 4)) {
        word(header, 4) = word(header, 4) - 1u;
    }
    word(header, 4) = static_cast<Word>(requested);
}

void destroy_pointers(void* header) {
    resize_pointers(header, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(word(header)));
}
} // namespace

__declspec(noinline) void reserve_native_resource_item_pointers_00b872f0(void* header,
    std::int32_t requested) {
    reserve_pointers(header, requested);
}
void reserve_native_hierarchy_item_pointers_00b87350(void* header,
    std::int32_t requested) {
    reserve_pointers(header, requested);
}
__declspec(noinline) void resize_native_resource_item_pointers_00b873c0(void* header,
    std::int32_t requested) {
    // bsp.py show 00B873C0: growth calls B872F0 on this actual header.
    resize_pointers(header, requested, reserve_native_resource_item_pointers_00b872f0);
}
// Keep the native resize boundary visible to the built call graph; this body
// now receives the live StructuredHierarchyItems triplet through its destructor.
__declspec(noinline) void resize_native_hierarchy_item_pointers_00b87410(void* header,
    std::int32_t requested) {
    resize_pointers(header, requested);
}
__declspec(noinline) void destroy_native_resource_item_pointers_00b87b20(void* header) {
    // bsp.py show 00B87B20: preserve resize-zero, then current-data free.
    resize_native_resource_item_pointers_00b873c0(header, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(word(header)));
}
void destroy_native_hierarchy_item_pointers_00b87b40(void* header) {
    // bsp.py show 00B87B40: call 00B87410(0), then free the current
    // backing pointer. Pass the same actual header, including signed
    // count/capacity and the resize-before-free behavior on negative capacity.
    resize_native_hierarchy_item_pointers_00b87410(header, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(word(header)));
}
} // namespace bsp
