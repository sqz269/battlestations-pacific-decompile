#include "bsp/native_scene_integer_set_head.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene integer-set heads require MSVC Win32.
#endif

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* storage) noexcept {
    return reinterpret_cast<Word>(storage);
}
Word read_word(Word location) noexcept {
    return *reinterpret_cast<const volatile Word*>(location);
}
void write_word(Word location, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location) = value;
}
void write_byte(Word location, unsigned char value) noexcept {
    *reinterpret_cast<volatile unsigned char*>(location) = value;
}

} // namespace

void* allocate_native_scene_integer_set_head_004682b0() {
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x14, 0x14}); // 004682B2
    const Word head = address(allocation);
    if (head != 0) write_word(head, 0);
    const Word parent = head + 4u;
    if (parent != 0) write_word(parent, 0);
    const Word right = head + 8u;
    if (right != 0) write_word(right, 0);
    write_byte(head + 0x10u, 1);
    write_byte(head + 0x11u, 0);
    return allocation;
}

void* construct_native_scene_integer_set_head_00468ea0(void* actual_tree) {
    const Word tree = address(actual_tree);
    void* const allocation = allocate_native_scene_integer_set_head_004682b0(); // 00468EA3
    write_word(tree + 4u, address(allocation));
    write_byte(address(allocation) + 0x11u, 1);
    Word current_head = read_word(tree + 4u); // 00468EAF
    write_word(current_head + 4u, current_head);
    current_head = read_word(tree + 4u); // 00468EB5
    write_word(current_head, current_head);
    current_head = read_word(tree + 4u); // 00468EBA
    write_word(current_head + 8u, current_head);
    write_word(tree + 8u, 0);
    return actual_tree;
}

} // namespace bsp
