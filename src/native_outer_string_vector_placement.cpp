#include "bsp/native_outer_string_vector_placement.hpp"

#include "bsp/native_outer_string_vector_copy_construct.hpp"

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));

void __fastcall placement_copy_native_outer_string_vector_006a6810(
    void* slot, const void* source, NativeStringRawPoolContext& strings) {
    if (slot != nullptr) {
        (void)construct_native_outer_string_vector_copy_0055b150(slot,
            reinterpret_cast<std::uintptr_t>(source), source, strings);
    }
    // Complete native placement unwind calls RET-only00401130, no cleanup.
}

void __fastcall placement_copy_native_outer_string_vector_006a6b60(
    void*, std::uint32_t unused_edx, void* slot, const void* source,
    NativeStringRawPoolContext& strings) {
    if (slot != nullptr) {
        (void)construct_native_outer_string_vector_copy_0055b150(
            slot, unused_edx, source, strings);
    }
}
} // namespace bsp
