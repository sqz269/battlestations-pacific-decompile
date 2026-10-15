#include "bsp/native_input_scalar_tree_iterator.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/native_int_pointer_tree18_erase.hpp"

namespace bsp {
void __fastcall increment_native_input_scalar_tree_iterator_005531f0(
    void* iterator, void*) {
    increment_native_int_pointer_tree18_00869a20(iterator);
}

void* __fastcall erase_native_input_scalar_tree_iterator_00556dc0(
    void* tree, void* unused_edx, void* output, void* input_owner, void* input_node) {
    return erase_native_int_pointer_tree18_iterator_0086e8a0(
        tree, unused_edx, output, input_owner, input_node);
}
} // namespace bsp
