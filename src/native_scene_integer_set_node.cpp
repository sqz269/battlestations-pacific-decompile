#include "bsp/native_scene_integer_set_node.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene integer-set nodes require MSVC Win32.
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
unsigned char read_byte(Word location) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(location);
}
void write_word(Word location, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location) = value;
}
void write_byte(Word location, unsigned char value) noexcept {
    *reinterpret_cast<volatile unsigned char*>(location) = value;
}
} // namespace

void* allocate_native_scene_integer_set_node_004681b0(const void* actual_parameters20) {
    const Word parameters = address(actual_parameters20);
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x14, 0x14}); // 004681B2
    const Word node = address(allocation);
    if (node != 0) { // 004681BC: no input-cell reads on the null branch
        const Word left = read_word(parameters);
        const Word parent = read_word(parameters + 4u);
        write_word(node, left);
        const Word right = read_word(parameters + 8u);
        write_word(node + 4u, parent);
        const Word key_pointer = read_word(parameters + 0x0cu);
        write_word(node + 8u, right);
        const Word key = read_word(key_pointer);
        const auto color = read_byte(parameters + 0x10u);
        write_word(node + 0x0cu, key);
        write_byte(node + 0x10u, color);
        write_byte(node + 0x11u, 0);
    }
    return allocation;
}

} // namespace bsp
