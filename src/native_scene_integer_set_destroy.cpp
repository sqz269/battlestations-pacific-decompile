#include "bsp/native_scene_integer_set_destroy.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene integer-set destruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* storage) noexcept {
    return reinterpret_cast<Word>(storage);
}
Word read(Word location, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(location + offset);
}
void write(Word location, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location + offset) = value;
}

void destroy_set(void* actual_tree, const NativeSceneIntegerSetDestroyFrame& frame) {
    const Word tree = address(actual_tree);
    const Word head = read(tree, 4);
    const Word minimum = read(head);
    const Word parameters = address(frame.range.actual_parameters20);
    write(parameters, 16, head);
    write(parameters, 12, tree);
    write(parameters, 8, minimum);
    write(parameters, 4, tree);
    write(parameters, 0, address(frame.actual_range_output8));
    (void)erase_native_scene_integer_set_range_0046b170(actual_tree, frame.range);
    void* const current_head = reinterpret_cast<void*>(read(tree, 4));
    singleton_lifetime_free(current_head);
    write(tree, 4, 0);
    write(tree, 8, 0);
}
} // namespace

void destroy_native_scene_integer_set_0046b530(void* actual_tree,
    const NativeSceneIntegerSetDestroyFrame& frame) {
    destroy_set(actual_tree, frame);
}
void destroy_native_scene_integer_set_0046c4c0(void* actual_tree,
    const NativeSceneIntegerSetDestroyFrame& frame) {
    destroy_set(actual_tree, frame);
}
void destroy_native_scene_integer_set_0046ce90(void* actual_tree,
    const NativeSceneIntegerSetDestroyFrame& frame) {
    destroy_set(actual_tree, frame);
}

} // namespace bsp
