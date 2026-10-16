#include "bsp/native_traffic_vector_failure_children.hpp"

namespace bsp {

void* __fastcall allocate_native_traffic_vector_0047e970(std::uint32_t count) {
    // Actual EDX is not an Original input. The canonical Source interface's
    // reserved slot is likewise unused; no value requirement is introduced.
    return native_singleton_pointer_allocate_00bcfeb0(count, nullptr);
}

[[noreturn]] void __cdecl throw_native_traffic_vector_length_00490240() {
    // Whole Native equivalence includes the state0 -> -1 cleanup of [EBP-50h].
    // Preserve its actual owning message/payload/cleanup through the complete
    // canonical Source operation, under its existing current-EH boundary.
    native_singleton_length_error_00bd0590();
}

} // namespace bsp
