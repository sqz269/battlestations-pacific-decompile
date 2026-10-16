#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Whole 006A62D0: Native ECX opaque tree, EDX unused, node stacked, RET4.
// This Source API adds borrowed RawCtx (RET8) and returns semantic void.
// Descriptive names are hypotheses; fixed node nil21/pair+C is established.
void __fastcall destroy_native_input_controller_subtree_006a62d0(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node,
    NativeStringRawPoolContext& strings);

// Whole 006A6650: Native ECX tree, EDX unused, plain RET/incidental EAX.
// This Source API adds borrowed RawCtx (RET4), without taking head ownership.
void __fastcall clear_native_input_controller_tree_006a6650(
    void* actual_tree, std::uint32_t unused_edx,
    NativeStringRawPoolContext& strings);
} // namespace bsp
