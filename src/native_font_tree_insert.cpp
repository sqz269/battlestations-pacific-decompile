#include "bsp/native_font_tree_insert.hpp"
#include "bsp/native_effect_handle_acquisition.hpp"
#include <cstddef>

namespace bsp {
static_assert(sizeof(NativeIntPointerTree18Iterator) == 8);
static_assert(offsetof(NativeIntPointerTree18Iterator, owner) == 0);
static_assert(offsetof(NativeIntPointerTree18Iterator, node) == 4);
static_assert(sizeof(NativeIntPointerTree18Pair) == 8);
static_assert(offsetof(NativeIntPointerTree18Pair, key_00) == 0);
static_assert(offsetof(NativeIntPointerTree18Pair, value_04) == 4);

NativeIntPointerTree18Iterator* __cdecl link_native_font_tree_node_00ad4600(
    void* actual_font_base, NativeIntPointerTree18Iterator* output8,
    std::uint8_t insert_left, void* parent,
    const NativeIntPointerTree18Pair* pair8) {
    return link_native_int_pointer_tree18_node_0086ebe0(
        actual_font_base, output8, insert_left, parent, pair8);
}
} // namespace bsp
