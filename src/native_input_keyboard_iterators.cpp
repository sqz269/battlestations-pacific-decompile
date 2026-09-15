#include "bsp/native_input_keyboard_iterators.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word read(const void* p, Word offset = 0) noexcept {
    Word result;
    std::memcpy(&result, static_cast<const unsigned char*>(p) + offset, 4);
    return result;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* link(const void* p, Word offset = 0) noexcept { return pointer(read(p, offset)); }
bool nil(const void* p, Word offset) noexcept {
    return static_cast<const unsigned char*>(p)[offset] != 0;
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    if (callbacks.invalid_parameter) callbacks.invalid_parameter(callbacks.context);
    else _invalid_parameter_noinfo();
}
void advance(NativeKeyboardTreeIterator* iterator, Word nil_offset,
             const SingletonLifetimeCallbacks& callbacks) {
    if (!iterator->owner) invalid(callbacks);
    void* node = iterator->node;
    if (nil(node, nil_offset)) {
        invalid(callbacks);
        return;
    }
    void* right = link(node, 8);
    if (nil(right, nil_offset)) {
        void* parent = link(node, 4);
        while (!nil(parent, nil_offset) && iterator->node == link(parent, 8)) {
            iterator->node = parent;
            parent = link(parent, 4);
        }
        iterator->node = parent;
    } else {
        void* left = link(right);
        while (!nil(left, nil_offset)) {
            right = left;
            left = link(left);
        }
        iterator->node = right;
    }
}
} // namespace

NativeKeyboardTreeIterator* find_native_input_hack_00546840(void* tree,
    NativeKeyboardTreeIterator* output, const std::int32_t* key) {
    void* candidate = link(tree, 4);
    void* node = link(candidate, 4);
    if (!nil(node, 0x11)) {
        const auto sought = *key;
        do {
            if (static_cast<std::int32_t>(read(node, 12)) < sought) node = link(node, 8);
            else { candidate = node; node = link(node); }
        } while (!nil(node, 0x11));
    }
    void* const head = link(tree, 4);
    if (candidate == head || *key < static_cast<std::int32_t>(read(candidate, 12))) candidate = head;
    output->owner = tree;
    output->node = candidate;
    return output;
}

void advance_native_input_sensitivity_00552770(NativeKeyboardTreeIterator* p) { advance(p, 0x29, {}); }
void advance_native_input_codes_00552d40(NativeKeyboardTreeIterator* p) { advance(p, 0x25, {}); }
void advance_native_input_device_005540c0(NativeKeyboardTreeIterator* p) { advance(p, 0x99, {}); }
void advance_native_checked_tree_iterator(NativeKeyboardTreeIterator* p, Word offset,
                                          const SingletonLifetimeCallbacks& callbacks) {
    advance(p, offset, callbacks);
}

NativeKeyboardBitIterator* advance_native_input_bit_0048d3b0(
    NativeKeyboardBitIterator* iterator, std::int32_t distance) {
    if (!distance) return iterator;
    if (!iterator->owner || !iterator->word) _invalid_parameter_noinfo();
    void* const owner = iterator->owner;
    const Word begin = read(owner, 8), initial_bit = iterator->bit;
    if (read(owner, 12) < begin) _invalid_parameter_noinfo();
    const Word offset = (static_cast<Word>(static_cast<std::int32_t>(address(iterator->word) - begin) >> 2) << 5) + initial_bit;
    const Word delta = static_cast<Word>(distance), magnitude = 0u - delta;
    if (distance < 0) {
        if (offset < magnitude) _invalid_parameter_noinfo();
    } else if (offset + delta > read(iterator->owner)) _invalid_parameter_noinfo();
    const Word sum = iterator->bit + delta;
    if (distance < 0 && iterator->bit < magnitude)
        iterator->word = pointer(address(iterator->word) - 4u - (((~sum) >> 5) << 2));
    else iterator->word = pointer(address(iterator->word) + ((sum >> 5) << 2));
    iterator->bit = sum & 31u;
    return iterator;
}
} // namespace bsp
