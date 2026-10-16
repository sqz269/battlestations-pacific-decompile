#include "bsp/native_input_default_scalar_tree_range.hpp"
#include "bsp/native_input_scalar_tree_range.hpp"

namespace bsp {
void* __fastcall erase_native_input_default_scalar_tree_range_0069fe70(
    void* tree, void* unused_edx, void* output, void* first_owner, void* first_node,
    void* last_owner, void* last_node) {
    return erase_native_input_scalar_tree_range_005572f0(
        tree, unused_edx, output, first_owner, first_node, last_owner, last_node);
}
} // namespace bsp
