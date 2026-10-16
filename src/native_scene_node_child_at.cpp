#include "bsp/native_scene_node_child_at.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(Word base, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(base + offset);
}
} // namespace

void* child_at_native_scene_node_list_00467f30(void* actual_list,
    const void* actual_index4) {
    Word remaining = read(address(actual_index4)); // 00467F30, before any list read.
    if ((remaining & 0x80000000u) != 0) return nullptr;
    const Word list = address(actual_list);
    const Word count = read(list, 8);
    // Index is nonnegative here, so this is the complete signed count<=index.
    if ((count & 0x80000000u) != 0 || count <= remaining) return nullptr;
    Word current = read(list);
    if (current == 0) return nullptr;
    while (remaining != 0) {
        current = read(current, 0x44u);
        remaining -= 1u;
        if (current == 0) return nullptr;
    }
    return pointer(current);
}

} // namespace bsp
