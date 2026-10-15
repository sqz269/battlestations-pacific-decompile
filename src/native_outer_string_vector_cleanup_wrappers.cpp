#include "bsp/native_outer_string_vector_cleanup_wrappers.hpp"

#include "bsp/native_outer_string_vector_cleanup.hpp"

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));

void __fastcall destroy_native_outer_string_vector_range_dispatch_0055ac60(
    const void* receiver, std::uint32_t, void* first, void* last,
    NativeStringRawPoolContext& strings) {
    destroy_native_outer_string_vector_range_0055a3a0(first, last, receiver,
        reinterpret_cast<std::uint32_t>(last), strings);
}

void __fastcall tidy_native_outer_string_vector_0055b110(void* header,
    NativeStringRawPoolContext& strings) {
    // Complete native instruction/field/callee equivalence was checked;
    // only the two CALL relative displacements differ in the byte bodies.
    tidy_native_outer_string_vector_0055aff0(header, strings);
}
} // namespace bsp
