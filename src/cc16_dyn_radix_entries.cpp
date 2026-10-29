#include "bsp/cc16_dyn_radix_entries.hpp"

namespace bsp::cc16_dyn_radix_entries {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered layout requires Win32 pointers.");

U32 address(const void* value) noexcept {
    return reinterpret_cast<U32>(value);
}

void* pointer(U32 value) noexcept {
    return reinterpret_cast<void*>(value);
}

// Integer address arithmetic retains x86 wrap, including a zero copy cursor.
// Volatile accesses retain the instruction-order reads around heap callbacks.
volatile U32& word(const void* base, U32 offset = 0) noexcept {
    return *static_cast<volatile U32*>(pointer(address(base) + offset));
}
} // namespace

void append_object_00c37180(void* manager, void* object, const Heap& heap) {
    const U32 old_count = word(manager, 0x08);
    const U32 next_count = old_count + 1u;
    if (next_count > old_count && next_count > word(manager, 0x0c)) {
        word(manager, 0x0c) = next_count;
        void* grown = heap.operator_new_00bf55be(next_count * 8u);
        U32 index = 0;
        U32 output = address(grown);
        while (index < word(manager, 0x08)) {
            if (output != 0) {
                const U32 input = word(manager, 0x04) + index * 8u;
                word(pointer(output)) = word(pointer(input));
                word(pointer(output), 4) = word(pointer(input), 4);
            }
            ++index;
            output += 8u;
        }
        void* old_storage = pointer(word(manager, 0x04));
        if (old_storage != nullptr) {
            heap.free_00bf6989(old_storage);
        }
        word(manager, 0x04) = address(grown);
    }
    word(manager, 0x08) = next_count;
    const U32 row_object = word(manager, 0x04) + old_count * 8u + 4u;
    word(pointer(row_object)) = address(object);
    *static_cast<volatile std::uint16_t*>(pointer(address(object) + 0x20u)) =
        static_cast<std::uint16_t>(old_count);
}

} // namespace bsp::cc16_dyn_radix_entries
