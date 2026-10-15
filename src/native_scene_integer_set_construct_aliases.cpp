#include "bsp/native_scene_integer_set_construct_aliases.hpp"
#include "bsp/native_scene_integer_set_head.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene integer-set construction requires MSVC Win32.
#endif

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word read_word(Word address) noexcept {
    return *reinterpret_cast<const volatile Word*>(address);
}
void write_word(Word address, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address) = value;
}

} // namespace

void* construct_native_scene_integer_set_head_00468940(void* actual_tree) {
    const Word tree = reinterpret_cast<Word>(actual_tree);
    void* const allocation = allocate_native_scene_integer_set_head_004682b0(); // 00468943
    const Word allocated_head = reinterpret_cast<Word>(allocation);
    write_word(tree + 4u, allocated_head);
    *reinterpret_cast<volatile unsigned char*>(allocated_head + 0x11u) = 1;
    Word head = read_word(tree + 4u); // 0046894F
    write_word(head + 4u, head);
    head = read_word(tree + 4u); // 00468955
    write_word(head, head);
    head = read_word(tree + 4u); // 0046895A
    write_word(head + 8u, head);
    write_word(tree + 8u, 0);
    return reinterpret_cast<void*>(head);
}

void* construct_native_scene_integer_set_head_0046cf10(void* actual_tree) {
    return construct_native_scene_integer_set_head_00468ea0(actual_tree);
}

} // namespace bsp
