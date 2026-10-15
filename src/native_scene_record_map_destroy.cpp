#include "bsp/native_scene_record_map_destroy.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene-record map destruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* storage) noexcept {
    return reinterpret_cast<Word>(storage);
}
Word read_word(Word location, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(location + offset);
}
void write_word(Word location, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location + offset) = value;
}

void destroy_map(void* actual_tree, const NativeSceneRecordMapDestroyFrame& frame,
    NativeStringRawPoolContext& strings) {
    const Word tree = address(actual_tree);
    const Word head = read_word(tree, 4);
    const Word minimum = read_word(head);
    const Word parameters = address(frame.range.actual_parameters20);
    write_word(parameters, 16, head);
    write_word(parameters, 12, tree);
    write_word(parameters, 8, minimum);
    write_word(parameters, 4, tree);
    write_word(parameters, 0, address(frame.actual_range_output8));
    (void)erase_native_scene_record_map_range_0046b570(actual_tree, frame.range, strings);
    void* const current_head = reinterpret_cast<void*>(read_word(tree, 4));
    singleton_lifetime_free(current_head);
    write_word(tree, 4, 0);
    write_word(tree, 8, 0);
}
} // namespace

void destroy_native_scene_record_map_0046c500(void* actual_tree,
    const NativeSceneRecordMapDestroyFrame& frame, NativeStringRawPoolContext& strings) {
    destroy_map(actual_tree, frame, strings);
}
void destroy_native_scene_record_map_0046ced0(void* actual_tree,
    const NativeSceneRecordMapDestroyFrame& frame, NativeStringRawPoolContext& strings) {
    destroy_map(actual_tree, frame, strings);
}
void destroy_native_scene_record_map_0046ef70(void* actual_tree,
    const NativeSceneRecordMapDestroyFrame& frame, NativeStringRawPoolContext& strings) {
    destroy_map(actual_tree, frame, strings);
}

} // namespace bsp
