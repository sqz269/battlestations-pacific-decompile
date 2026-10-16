#include "bsp/native_font_tree_iterator.hpp"
#include "bsp/native_effect_handle_acquisition.hpp"
#include <cstddef>

namespace bsp {
static_assert(sizeof(NativeIntPointerTree18Iterator) == 8);
static_assert(offsetof(NativeIntPointerTree18Iterator, owner) == 0);
static_assert(offsetof(NativeIntPointerTree18Iterator, node) == 4);

void __cdecl decrement_native_font_tree_iterator_00ad4040(
    NativeIntPointerTree18Iterator& actual_iterator) {
    decrement_native_int_pointer_tree18_00869990(actual_iterator);
}
} // namespace bsp
