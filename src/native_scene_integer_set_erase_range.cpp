#include "bsp/native_scene_integer_set_erase_range.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_integer_set_clear.hpp"
#include "bsp/native_scene_integer_set_erase.hpp"
#include "bsp/native_scene_integer_set_increment.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene integer-set range erasure requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* storage) noexcept { return reinterpret_cast<Word>(storage); }
Word read(Word location, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(location + offset);
}
void write(Word location, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location + offset) = value;
}
} // namespace

void* erase_native_scene_integer_set_range_0046b170(void* actual_tree,
    const NativeSceneIntegerSetEraseRangeFrame& frame) {
    const Word parameters = address(frame.actual_parameters20);
    Word owner = read(parameters, 4);
    const Word tree = address(actual_tree);
    const Word initial_head = read(tree, 4);
    const Word initial_minimum = read(initial_head);
    if (owner == 0 || owner != tree) invoke_native_invalid_parameter_00bf6713();
    Word node = read(parameters, 8);
    if (node == initial_minimum) {
        const Word last_owner = read(parameters, 12);
        const Word captured_head = read(tree, 4);
        if (last_owner == 0 || last_owner != tree) invoke_native_invalid_parameter_00bf6713();
        if (read(parameters, 16) == captured_head) {
            clear_native_scene_integer_set_00468980(actual_tree, 0);
            const Word head = read(tree, 4);
            const Word minimum = read(head);
            const Word output = read(parameters);
            write(output, 0, tree);
            write(output, 4, minimum);
            return reinterpret_cast<void*>(output);
        }
    }
    for (;;) {
        if (owner == 0 || owner != read(parameters, 12))
            invoke_native_invalid_parameter_00bf6713();
        if (node == read(parameters, 16)) break;
        increment_native_scene_integer_set_iterator_00466ae0(
            reinterpret_cast<void*>(parameters + 4u));
        // Do not inspect any child metadata before this reached return.
        const auto& child = *frame.child_frame;
        const Word child_arguments = address(child.actual_parameters12);
        write(child_arguments, 8, node);
        write(child_arguments, 4, owner);
        write(child_arguments, 0, address(frame.actual_private_output8));
        (void)erase_native_scene_integer_set_iterator_0046a740(actual_tree, child);
        node = read(parameters, 8);
        owner = read(parameters, 4);
    }
    const Word output = read(parameters);
    write(output, 0, owner);
    write(output, 4, node);
    return reinterpret_cast<void*>(output);
}

} // namespace bsp
