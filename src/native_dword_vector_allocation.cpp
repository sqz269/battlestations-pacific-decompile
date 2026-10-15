#include "bsp/native_dword_vector_allocation.hpp"

namespace bsp {

void* __fastcall allocate_native_dword_vector_00553260(
    std::uint32_t count, void* unused_edx) {
    // Whole native body matches: same unsigned count, same raw byte request.
    return native_singleton_pointer_allocate_00bcfeb0(count, unused_edx);
}

[[noreturn]] void __cdecl throw_native_dword_vector_length_error_00556c20() {
    // Identical native payload, message, state-arm point and cleanup schedule.
    // The full accepted service owns its temporary; do not add another guard.
    native_singleton_length_error_00bd0590();
}

void cleanup_native_dword_vector_length_message_00c6d790(
    NativeLegacySboStringStorage& completed_message) noexcept {
    native_legacy_sbo_string_destroy_004072d0(completed_message);
}

} // namespace bsp
