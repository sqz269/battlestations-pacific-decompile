#include "bsp/native_input_preset_tree_predecessor.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstddef>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word word(const void* base, std::size_t offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(base) + offset);
}
void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
void put(void* base, std::size_t offset, const void* value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::byte*>(base) + offset) =
        static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
bool nil(const void* node) {
    return *reinterpret_cast<const volatile std::uint8_t*>(
        static_cast<const std::byte*>(node) + 0x29) != 0;
}
} // namespace

void __fastcall decrement_native_input_preset_tree_iterator_0069aec0(
    NativeInputPresetTreeIterator* actual_iterator, std::uint32_t unused_edx) {
    static_cast<void>(unused_edx);
    if (word(actual_iterator, 0) == 0) {
        invoke_native_invalid_parameter_00bf6713(); // 0069AEC8; may return.
    }
    void* node = pointer(word(actual_iterator, 4)); // CURRENT after diagnostic.
    if (nil(node)) {
        void* right = pointer(word(node, 8));
        put(actual_iterator, 4, right); // Publish before captured-right nil test.
        if (nil(right)) {
            invoke_native_invalid_parameter_00bf6713(); // 0069AEE3 tail.
            return;
        }
        return;
    }

    void* candidate = pointer(word(node, 0));
    if (!nil(candidate)) {
        void* next = pointer(word(candidate, 8));
        while (!nil(next)) {
            candidate = next;
            next = pointer(word(candidate, 8));
        }
        put(actual_iterator, 4, candidate);
        return;
    }

    void* parent = pointer(word(node, 4));
    while (!nil(parent)) {
        void* current = pointer(word(actual_iterator, 4));
        if (current != pointer(word(parent, 0))) {
            break;
        }
        put(actual_iterator, 4, parent);
        parent = pointer(word(parent, 4)); // Read AFTER the publication.
    }
    void* current = pointer(word(actual_iterator, 4));
    if (nil(current)) {
        invoke_native_invalid_parameter_00bf6713(); // 0069AF3F tail.
        return;
    }
    put(actual_iterator, 4, parent);
}
} // namespace bsp
