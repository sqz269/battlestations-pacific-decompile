#include "bsp/native_input_default_scalar_subtree.hpp"
#include "bsp/native_input_int_subtrees.hpp"

namespace bsp {
void __fastcall destroy_native_input_default_scalar_subtree_0069d3f0(
    void* opaque_tree, std::uint32_t, void* node) {
    destroy_native_input_int_scalar_subtree_00554f10(opaque_tree, 0, node);
}
} // namespace bsp
