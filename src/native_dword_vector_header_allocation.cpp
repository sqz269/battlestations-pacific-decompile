#include "bsp/native_dword_vector_header_allocation.hpp"

namespace bsp {

void* __fastcall allocate_native_dword_vector_header_storage_00696890(
    std::uint32_t count, void* unused_edx) {
    return allocate_native_nested_string_vector_00553300(count, unused_edx);
}

[[noreturn]] void __cdecl
throw_native_dword_vector_header_length_error_006978d0() {
    throw_native_nested_string_vector_length_error_00556c90();
}

void cleanup_native_dword_vector_header_length_message_00c7eba0(
    NativeLegacySboStringStorage& completed_message) noexcept {
    native_legacy_sbo_string_destroy_004072d0(completed_message);
}

} // namespace bsp
