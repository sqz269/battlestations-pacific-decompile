#include "bsp/native_nested_string_vector_wrappers.hpp"

#include "bsp/native_string_vector_copy_construct.hpp"

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));

void __fastcall placement_copy_native_nested_string_vector_00558640(
    void* slot, const void* source, NativeStringRawPoolContext& strings) {
    if (slot != nullptr) {
        // Native PUSH EDX does not change EDX before00557BC0 consumes ECX
        // and the stacked source. Its EDX word is preserved but unused.
        (void)construct_native_string_header_vector_copy_00557bc0(slot,
            reinterpret_cast<std::uintptr_t>(source), source, strings);
    }
    // Verified RET-only placement delete adds no cleanup on a child failure.
}

void __fastcall placement_copy_native_nested_string_vector_005590c0(
    void*, std::uint32_t unused_edx, void* slot, const void* source,
    NativeStringRawPoolContext& strings) {
    if (slot != nullptr) {
        (void)construct_native_string_header_vector_copy_00557bc0(
            slot, unused_edx, source, strings);
    }
}

void __fastcall tidy_native_nested_string_vector_00559dc0(void*,
    std::uint32_t, void* header, NativeStringRawPoolContext& strings) {
    // Full instruction/callee comparison establishes identical header and
    // allocator scheduling; only native entry/stack conventions differ.
    tidy_native_string_header_vector_00432440(header, strings);
}
} // namespace bsp
